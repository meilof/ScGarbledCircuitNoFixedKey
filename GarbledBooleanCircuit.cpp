#include "GarbledBooleanCircuit.h"
#include "GarbledGate.h"
#include "Config.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include "mmintrin.h"
#include "TedKrovetzAesNiWrapperC.h"
#include <string.h>

#ifdef _WIN32
#include "StdAfx.h"
#else
#include "Compat.h"
#endif

using namespace std;

GarbledBooleanCircuit::GarbledBooleanCircuit(void)
{
}


GarbledBooleanCircuit::~GarbledBooleanCircuit(void)
{

	if(inputIndices != NULL)
		delete[] inputIndices;
	
	if(outputIndices != NULL)
		delete[] outputIndices;

	
	if (garbledGates != NULL){
		delete[] garbledGates;
	}
		

	if(translationTable != NULL)
		delete[] translationTable;

	if (numOfInputsForEachParty != NULL)
		delete[] numOfInputsForEachParty;


	if(garbledTables != NULL)
			_aligned_free(garbledTables);


	if (computedWires != NULL){
		computedWires--;
		_aligned_free(computedWires);
	}

	if(seed != NULL)
		_aligned_free(seed);

	if (aesSeedKey != NULL)
		_aligned_free(aesSeedKey);


}

void GarbledBooleanCircuit::createCircuit(const char* fileName, bool isFreeXor){
	//clock_t start, stop;


	//start = clock();
	this->isFreeXor = isFreeXor;

	//init all the variable to either null or 0 for integers.
	lastWireIndex = 0;
	numberOfGates = 0;
	numOfXorGates = 0;	
	numOfNotGates = 0;
	numberOfParties = 0;
	numOfInputsForEachParty = NULL;
	numberOfInputs = 0;
	numberOfOutputs = 0;
	

	inputIndices = NULL;
	outputIndices = NULL;
	garbledTables = NULL;
	garbledGates = NULL;
	translationTable = NULL;
	garbledWires = NULL;
	computedWires = NULL;
	seed = NULL;
	aesSeedKey = NULL;

	//read the file and fill the gates, number of parties, input indices, output indices and so on.
	readCircuitFromFile(fileName);

	seed = (block *)_aligned_malloc(sizeof(block), 16);

	aesSeedKey = (AES_KEY *)_aligned_malloc(sizeof(AES_KEY), 16);

	computedWires = (block *) _aligned_malloc(sizeof(block) * ((lastWireIndex +1) + 1),16);//the wires that have been already computed. It is assumed that when a gate is handled the

	if (computedWires== NULL) {
		cout<<"computedWires could not be allocated";
		exit(0);
	}
	memset(computedWires, 0, sizeof(block) * ((lastWireIndex + 1)+1));
	computedWires++;

	//allocate memory for the translation table
	translationTable = new unsigned char[numberOfOutputs]; 
}


int* GarbledBooleanCircuit::readInputsFromFile(char* fileName){

	ifstream myfile;
	int numberOfInputs;
	int* inputs = NULL;
	myfile.open (fileName);
	if (myfile.is_open())
	{
		//get the number of inputs
		myfile >> numberOfInputs;//get the number of inputs
		inputs = new int[numberOfInputs];

		//fille the an int array with the bits of the inputs read from the file
		for(int i=0; i<numberOfInputs; i++){
			myfile >> inputs[i];
		}
	}
		
	return inputs;
}


int GarbledBooleanCircuit::binaryTodecimal(int n){
	
	int output = 0;
	int pow = 1;

	//turns the string of the truth table that was taken as a decimal number into a number between 0 and 15 which represents the truth table
	//0 means the truth table of 0000 and 8 means 1000 and so on. The functions returns the decimal representation of the thruth table.
	for(int i=0; n > 0; i++) {
		
		if(n % 10 == 1) {
			
			output += pow;
		}
		n /= 10;

		pow = pow*2;
	}
	return output;
}



int GarbledBooleanCircuit::getRowTruthTableResult(int i, int j, unsigned char truthTable){

	//get the row of the table starting from 0
	int rowNumber = 2*i + j;

	//return the result of row i,j.
	return truthTable & integerPow(3-rowNumber);
}


int GarbledBooleanCircuit::integerPow(int p) {

	switch( p ) {
      case(0):
		return 1;
	  case(1):
		return 2;
      case(2):
		return 4;
      default:
		return 8;
	
    }

}


void GarbledBooleanCircuit::translate(block *outputKeys, unsigned char* answer){

	
	for(int i=0; i<numberOfOutputs;i++){

		//The answer of i'th position is the signal bit of the XOr between the related translation table location and the related outputKey array position
		answer[i] = getSignalBitOf(outputKeys[i]) ^ translationTable[i];
		//cout<<(int)answer[i];

	}

	//cout<<"\n";

}


unsigned char* GarbledBooleanCircuit::getTranslationTable(){

	return translationTable;
}

void GarbledBooleanCircuit::setTranslationTable(unsigned char* translationTable){

	
	if(this->translationTable!=NULL)
		delete[] this->translationTable;

	this->translationTable = translationTable;
}
	

void GarbledBooleanCircuit::setGarbledTables(block* garbledTables){

	if(this->garbledTables!=NULL)
		free(this->garbledTables);

	this->garbledTables = garbledTables;
}



int *GarbledBooleanCircuit::getNumOfInputsForEachParty(){
	return numOfInputsForEachParty;
}


bool GarbledBooleanCircuit::verify(block *bothInputKeys){

	block *emptyBothWireOutputKeys = (block *) _aligned_malloc(sizeof(block)  * numberOfOutputs*2, 16); 

	//Call the internal internalVerify function that verifies all the gates but does not check the translation table.
	bool isVerified = internalVerify(bothInputKeys,emptyBothWireOutputKeys);

	//Check that the results of the internal verify comply with the translation table.
	if(isVerified==true){
		isVerified = verifyTranslationTable(emptyBothWireOutputKeys);

	}

	//Free the localy allocated memory
	_aligned_free(emptyBothWireOutputKeys);
	return isVerified;

}


bool  GarbledBooleanCircuit::equalBlocks(block a, block b)
{ 
	//A function that checks if two blocks are equal by casting to double size long array and check each half of a block
	long *ap = (long*) &a;
	long *bp = (long*) &b;
	if ((ap[0] == bp[0]) && (ap[1] == bp[1]))
		return 1;
	else{
		return 0;
	}
}


bool GarbledBooleanCircuit::verifyTranslationTable(block * bothWireOutputKeys)
{
	bool isVerified = true;
	//go over the output key results and make sure that they comply with the translation table
	for (int i=0; i<numberOfOutputs;i++) {
		block zeroBlock = bothWireOutputKeys[2*i];
		block oneBlock = bothWireOutputKeys[2*i+1];

		unsigned char translatedZeroValue = translationTable[i] ^ getSignalBitOf(zeroBlock);
		unsigned char translatedOneValue = translationTable[i] ^ getSignalBitOf(oneBlock);

		//Verify that the translatedZeroValue is actually 0 and that translatedOneValue is indeed 1
		if (translatedZeroValue != 0 || translatedOneValue != 1) {
			isVerified = false;
			break;
		}
	}	return isVerified;
}

void GarbledBooleanCircuit::readCircuitFromFile(const char* fileName)
{

	int inFan, outFan, input0, input1, output, type, typeBin, numOfinputsForParty;
	int currentPartyNumber;
	ifstream myfile;

	
	myfile.open(fileName);
	

	int **partiesInputs;

	int numOfNonXorGate = 0;


	if (myfile.is_open())
	{
		
		myfile >> numberOfGates;//get the gates
		myfile >> numberOfParties;

		numOfInputsForEachParty = new int[numberOfParties];
		partiesInputs = new int*[numberOfParties];

		for(int j=0 ; j<numberOfParties; j++){
			myfile >> currentPartyNumber;

			myfile >> numOfinputsForParty;
			numOfInputsForEachParty[currentPartyNumber-1] = numOfinputsForParty;

			partiesInputs[currentPartyNumber-1] = new int[numOfInputsForEachParty[currentPartyNumber-1]];

			for(int i = 0; i<numOfInputsForEachParty[currentPartyNumber-1]; i++){
				myfile >>partiesInputs[currentPartyNumber-1][i];
			}
		}


		//get the number of outputs
		myfile >> numberOfOutputs;

		//allocate memory for the output number of wires and get each wire number into the array of outputs indices
		outputIndices = new int[numberOfOutputs];

		for(int i=0;i < numberOfOutputs;i++){
			myfile >> outputIndices[i];
		}


		//calculate the total number of inputs
		for(int i=0; i<numberOfParties;i++){
			numberOfInputs+=numOfInputsForEachParty[i];
		}

		//allocate memory for the gates, We add one gate for the all-one gate whose output is always 1 and thus have a wire who is always 1 without the 
		//involvement of the user. This will be useful to turn a NOT gate into a XORGate
		garbledGates = new GarbledGate[numberOfGates];


		//write the inputs to the inputs array of the garbled circuit
		inputIndices = new int[numberOfInputs];

		int index = 0;
		for(int i=0;i <numberOfParties; i++){
			for(int j=0; j< numOfInputsForEachParty[i]; j++){

				inputIndices[index] = partiesInputs[i][j];
				index++;
			}
		}

		//create a one-gate for the NOT gates
		
		//garbledGates[0].truthTable = ONE_GATE;
		//garbledGates[0].input0 = inputIndices[0];
		//garbledGates[0].input1 = inputIndices[0];
		//garbledGates[0].output = -1;//the outputWire is defined to be -1. 

		//Increment the garbled gates pointer so the one gate will be in -1 location and the other gates will start with 0.
		//We will do the same when allocating wires.
		//garbledGates = garbledGates + 1;


		//create a gate whose output is always 1. This gate number will be -1 and we will move the poiter one place 
		//go over the file and create gate by gate
		for(int i=0; i<numberOfGates;i++)
		{

			//get  each row that represents a gate
			myfile >> inFan;
			myfile >> outFan;
			myfile >> input0;

			if (inFan != 1)//a 2 input 1 output gate - regualr gate, else we have a not gate
			{
				myfile >> input1;					
			}

			
			myfile >> output;
			myfile >> typeBin;


			if(lastWireIndex < output){
				lastWireIndex = output;
			}

			if (inFan == 1)//NOT gate
			{
				input1 = -1;
				type = XOR_GATE;

				garbledGates[i].truthTable = type;
			}
			else{
				type = binaryTodecimal(typeBin);

				garbledGates[i].truthTable = type;
			}


			//Just garbled require that the first input number would be less than the second one. If this is the case, we need to switch between bit2 and bit3 in order
			//to switch the labels and still get the required truth table


			//transform the binary string to a decimal number between 0-15. That is if the truth table string was "0110", typeBin gets the value 110 in decimal since it 
			//is an int. This function transforms it to the decimal number 6 (XOR_GATE).
			

			//we build the truth table in a way that we only need to get a specific row instead of doing a lot of shifts
			garbledGates[i].truthTableBits[0] = getRowTruthTableResultShifts(0, type);
			garbledGates[i].truthTableBits[1] = getRowTruthTableResultShifts(1, type);
			garbledGates[i].truthTableBits[2] = getRowTruthTableResultShifts(2, type);
			garbledGates[i].truthTableBits[3] = getRowTruthTableResultShifts(3, type);

			garbledGates[i].input0 = input0;
			garbledGates[i].input1 = input1;
			garbledGates[i].output = output;

			if (type == XOR_GATE || type == XOR_NOT_GATE){
				if (garbledGates[i].input1 == -1){
					numOfNotGates++;
				}
				else{
					numOfXorGates++;
				}
			}

		}

		for (int i = 0; i < numberOfParties; ++i)
			delete[] partiesInputs[i];
		delete[] partiesInputs;

	}
	myfile.close();
}
