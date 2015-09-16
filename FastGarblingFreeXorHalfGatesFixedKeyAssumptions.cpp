#include "FastGarblingFreeXorHalfGatesFixedKeyAssumptions.h"
#include "GarbledGate.h"
#include <string.h>
#include <iostream>
//#include "TedKrovetzAesNiWrapperC.h"
#include "intrinsic.h"
#include "ecb.h"
#ifdef _WIN32
#include "StdAfx.h"
#else
#include "Compat.h"
#endif

using namespace std;


FastGarblingFreeXorHalfGatesFixedKeyAssumptions::FastGarblingFreeXorHalfGatesFixedKeyAssumptions(void)
{
}
FastGarblingFreeXorHalfGatesFixedKeyAssumptions::~FastGarblingFreeXorHalfGatesFixedKeyAssumptions(void)
{
	//release memory allocated in this class
	if (deltaFreeXor != NULL)
		_aligned_free(deltaFreeXor);

	if (garbledWires!= NULL){
		garbledWires--;
		_aligned_free(garbledWires);
	}
	if (encryptedChunkKeys != NULL)
		_aligned_free(encryptedChunkKeys);

	if (indexArray != NULL)
		_aligned_free(indexArray);

}

FastGarblingFreeXorHalfGatesFixedKeyAssumptions::FastGarblingFreeXorHalfGatesFixedKeyAssumptions(const char* fileName)
{
	//create the needed memory for this circuit
	createCircuitMemory(fileName);
}

void FastGarblingFreeXorHalfGatesFixedKeyAssumptions::createCircuitMemory(const char* fileName) {

	//call the base class to create circuit function
	createCircuit(fileName, true);

	//create this memory and initialize it in construction time to gain performance
	deltaFreeXor = (block *)_aligned_malloc(sizeof(block), 16);
	if (deltaFreeXor == NULL) {
		cout << "deltaFreeXor could not be allocated";
		exit(0);
	}
	garbledTables = (block *)_aligned_malloc(sizeof(block) * (numberOfGates - numOfXorGates - numOfNotGates) * 2, 16);

	if (garbledTables == NULL) {
		cout << "garbled tables could not be allocated";
		exit(0);
	}
	memset(garbledTables, 0, (sizeof(block) * (numberOfGates - numOfXorGates - numOfNotGates) * 2));

	garbledWires = (block *)_aligned_malloc(sizeof(block) * ((lastWireIndex + 1) + 1), 16);

	if (garbledWires == NULL) {
		cout << "garbledWires could not be allocated";
		exit(0);
	}
	memset(garbledWires, 0, sizeof(block) * ((lastWireIndex + 1) + 1));
	garbledWires++;

	encryptedChunkKeys = (block *)_aligned_malloc(sizeof(block) * (numberOfInputs), 16);
	if (encryptedChunkKeys == NULL) {
		cout << "encryptedChunkKeys could not be allocated";
		exit(0);
	}
	memset(encryptedChunkKeys, 0, sizeof(block) * (numberOfInputs));
	indexArray = (block *)_aligned_malloc(sizeof(block) * (numberOfInputs), 16);
	if (indexArray == NULL) {
		cout << "indexArray could not be allocated";
		exit(0);
	}

	//we put the indices ahead of time to encrypt the whole chunk in one call.
	for (int i = 0; i < numberOfInputs; i++){

		indexArray[i] = _mm_set_epi32(0, 0, 0, i);

	}

}






void FastGarblingFreeXorHalfGatesFixedKeyAssumptions::garble(block *emptyBothInputKeys, block *emptyBothOutputKeys, unsigned char *emptyTranslationTable, block seed){

	*(this->seed) = seed;

	//init the aes encryptions of the seed and the fixed key. Fill the input wires
	initAesEncryptionsAndInputKeys(emptyBothInputKeys);

	int nonXorIndex = 0;
	ROUND_KEYS* KEY = (ROUND_KEYS *)_aligned_malloc(4 * 256, 16);



	for (int i = 0; i<numberOfGates; i++){

		if (garbledGates[i].truthTable == XOR_GATE){
			//create the 0-key by xoring the two 0-keys of the input
			garbledWires[garbledGates[i].output] = _mm_xor_si128(garbledWires[garbledGates[i].input0], garbledWires[garbledGates[i].input1]);
			continue;
		}
		else if (garbledGates[i].truthTable == XOR_NOT_GATE){
			//create the 0-key by xoring the two 0-keys of the input and xoring that with the delta.
			garbledWires[garbledGates[i].output] = _mm_xor_si128(_mm_xor_si128(garbledWires[garbledGates[i].input0], garbledWires[garbledGates[i].input1]), *deltaFreeXor);


		}
		else{

			//get the keys from the input wires of the gate
			block keys[4];
			keys[0] = garbledWires[garbledGates[i].input0];
			keys[1] = _mm_xor_si128(keys[0], *deltaFreeXor);

			keys[2] = garbledWires[garbledGates[i].input1];
			keys[3] = _mm_xor_si128(keys[2], *deltaFreeXor);

			//Get the signal bits of the input keys computed.
			int wire0SignalBit = getSignalBitOf(keys[0]);
			int wire1SignalBit = getSignalBitOf(keys[2]);

			//generate the keys array as well as the encryptedKeys array
			block plainText[4];
			block cipherText[4];
			
			plainText[0] = _mm_set_epi32(0, 0, 0, i);
			plainText[1] = _mm_set_epi32(0, 0, 0, i);
			plainText[2] = _mm_set_epi32(0, 0, 0, i + numberOfGates);
			plainText[3] = _mm_set_epi32(0, 0, 0, i + numberOfGates);

			//create the plaintext which is the ks4_ec4
			intrin_sequential_ks4_enc4((const unsigned char*)plainText, (unsigned char*)cipherText, 4, (unsigned char*) KEY, (unsigned char*)keys, NULL);

			//for more information, go to the pseudocode of the paper, page 9
			if (wire1SignalBit == 0){
				garbledTables[2 * nonXorIndex] = _mm_xor_si128(cipherText[0], cipherText[1]);
			}
			else{//signal bit is 1
				garbledTables[2 * nonXorIndex] = _mm_xor_si128(_mm_xor_si128(cipherText[0], cipherText[1]), *deltaFreeXor);
			}

			block tempK0, tempK1;

			if (wire0SignalBit == 0){
				tempK0 = cipherText[0];
			}
			else{
				tempK0 = _mm_xor_si128(cipherText[0], garbledTables[2 * nonXorIndex]);
			}

			garbledTables[2 * nonXorIndex + 1] = _mm_xor_si128(_mm_xor_si128(cipherText[2], cipherText[3]), keys[0]);

			if (wire1SignalBit == 0){
				tempK1 = cipherText[2];
			}
			else{
				tempK1 = _mm_xor_si128(_mm_xor_si128(cipherText[2], garbledTables[2 * nonXorIndex + 1]), keys[0]);
			}

			//set the garbled output to be the XOR of the two temp keys
			garbledWires[garbledGates[i].output] = _mm_xor_si128(tempK0, tempK1);

			nonXorIndex++;
		}

	}


	//copy the output keys to get back to the caller of the function as well as filling the translation table.
	//The input keys were already filled in the initialization of the function.
	for (int i = 0; i < numberOfOutputs; i++) {
		emptyBothOutputKeys[2 * i] = garbledWires[outputIndices[i]];
		emptyBothOutputKeys[2 * i + 1] = _mm_xor_si128(emptyBothOutputKeys[2 * i], *deltaFreeXor);

		translationTable[i] = emptyTranslationTable[i] = getSignalBitOf(emptyBothOutputKeys[2 * i]);
	}

	_aligned_free(KEY);


}


void FastGarblingFreeXorHalfGatesFixedKeyAssumptions::initAesEncryptionsAndInputKeys(block* emptyBothInputKeys){

	//create the aes with the seed as the key. This will be used for encrypting the input keys
	AES_set_encrypt_key((const unsigned char *)seed, 128, aesSeedKey);

	*deltaFreeXor = ZERO_BLOCK;
	AES_ecb_encrypt(deltaFreeXor, aesSeedKey);
	AES_ecb_encrypt(deltaFreeXor, aesSeedKey);

	//set the last bit of the first char to 1
	*((unsigned char *)(deltaFreeXor)) |= 1;


	AES_ECB_encrypt((const unsigned char *)indexArray,
		(unsigned char *)encryptedChunkKeys,
		SIZE_OF_BLOCK * (numberOfInputs),
		(const unsigned char *)aesSeedKey->rd_key,
		aesSeedKey->rounds);


	//create the input keys. We encrypt using the aes with the seed as index and encrypt the index of the input wire,
	for (int i = 0; i<numberOfInputs; i++){
		garbledWires[inputIndices[i]] = emptyBothInputKeys[2 * i] = encryptedChunkKeys[i];
		
		emptyBothInputKeys[2 * i + 1] = _mm_xor_si128(encryptedChunkKeys[i], *deltaFreeXor);
	}


	//set the fixed -1 wire to delta, this way we turn a not gate into a xor gate.
	garbledWires[-1] = *deltaFreeXor;

}

void  FastGarblingFreeXorHalfGatesFixedKeyAssumptions::compute(block * singleWiresInputKeys, block * Output)
{

	ROUND_KEYS* KEY = (ROUND_KEYS *)_aligned_malloc(4 * 256, 16);
	int nonXorIndex = 0;
	for (int i = 0; i<numberOfInputs; i++){

		//get the input keys into the computed wires array
		computedWires[inputIndices[i]] = singleWiresInputKeys[i];
	}

	for (int i = 0; i<numberOfGates; i++){

		if (garbledGates[i].truthTable == XOR_GATE || garbledGates[i].truthTable == XOR_NOT_GATE){
			//create the output key by xoring the computed keys if the first input wire and the second input wire
			computedWires[garbledGates[i].output] = _mm_xor_si128(computedWires[garbledGates[i].input0], computedWires[garbledGates[i].input1]);
			continue;

		}

		else{

			block keys[2];
			//get the keys from the already calculated wires
			keys[0] = computedWires[garbledGates[i].input0];
			keys[1] = computedWires[garbledGates[i].input1];

			//Get the signal bits of A and B which are the input keys computed.
			int wire0SignalBit = getSignalBitOf(keys[0]);
			int wire1SignalBit = getSignalBitOf(keys[1]);

			//create the ciphertext
			block ciphertext[2];
			block plaintext[2];
			plaintext[0] = _mm_set_epi32(0, 0, 0, i);
			plaintext[1] = _mm_set_epi32(0, 0, 0, i + numberOfGates);


			intrin_sequential_ks2_enc2((const unsigned char*)plaintext, (unsigned char*)ciphertext, 2, (unsigned char*)KEY, (unsigned char*)keys, NULL);

			
			//check the pseudo code of the paper, page 9.
			block tempK0, tempK1;

			if (wire0SignalBit == 0){
				tempK0 = ciphertext[0];
			}
			else{
				tempK0 = _mm_xor_si128(ciphertext[0], garbledTables[2 * nonXorIndex]);
			}

			if (wire1SignalBit == 0){
				tempK1 = ciphertext[1];
			}
			else{
				tempK1 = _mm_xor_si128(_mm_xor_si128(ciphertext[1], garbledTables[2 * nonXorIndex + 1]), keys[0]);
			}

			computedWires[garbledGates[i].output] = _mm_xor_si128(tempK0, tempK1);

			//increment the nonXor gates number only for the non-xor (not XOR or XOR_NOT) gates. For circuits
			//That do not use FreeXor optimization it will be incremented for every gate
			nonXorIndex++;
		}

	}

	//copy the output wire keys which are the result the user is interested in.
	for (int i = 0; i < numberOfOutputs; i++) {
		Output[i] = computedWires[outputIndices[i]];

	}

	_aligned_free(KEY);

}

bool FastGarblingFreeXorHalfGatesFixedKeyAssumptions::internalVerify(block *bothInputKeys, block *emptyBothWireOutputKeys){

	int nonXorIndex = 0;
	ROUND_KEYS* KEY = (ROUND_KEYS *)_aligned_malloc(4 * 256, 16);

	//set the delta to be the xor between the first 2 inputs
	*deltaFreeXor = _mm_xor_si128(bothInputKeys[0], bothInputKeys[1]);



	for (int i = 0; i<numberOfGates; i++){

		if (garbledGates[i].truthTable == XOR_GATE){
			//create the 0-key by xoring the two 0-keys of the input
			garbledWires[garbledGates[i].output] = _mm_xor_si128(garbledWires[garbledGates[i].input0], garbledWires[garbledGates[i].input1]);
			continue;
		}
		else if (garbledGates[i].truthTable == XOR_NOT_GATE){
			//create the 0-key by xoring the two 0-keys of the input and xoring that with the delta.
			garbledWires[garbledGates[i].output] = _mm_xor_si128(_mm_xor_si128(garbledWires[garbledGates[i].input0], garbledWires[garbledGates[i].input1]), *deltaFreeXor);


		}
		else{


			//get the keys from the input wires of the gate
			block keys[4];
			keys[0] = garbledWires[garbledGates[i].input0];
			keys[1] = _mm_xor_si128(keys[0], *deltaFreeXor);

			keys[2] = garbledWires[garbledGates[i].input1];
			keys[3] = _mm_xor_si128(keys[2], *deltaFreeXor);

			//An array of signal bits the 0-wire. This prevents from calling the function getSignalBitOf more than
			//once for each 0-wire in the for loop below
			int wire0signalBitsArray[2];
			wire0signalBitsArray[0] = getSignalBitOf(keys[0]);
			wire0signalBitsArray[1] = 1 - wire0signalBitsArray[0];


			//An array of signal bits the 0-wire. This prevents from calling the function getSignalBitOf more than
			//once for each 0-wire in the for loop below
			int wire1signalBitsArray[2];
			wire1signalBitsArray[0] = getSignalBitOf(keys[2]);
			wire1signalBitsArray[1] = 1 - wire1signalBitsArray[0];


			//generate the keys array as well as the encryptedKeys array
			block plainText[4];
			block cipherText[4];
		

			plainText[0] = _mm_set_epi32(0, 0, 0, i);
			plainText[1] = _mm_set_epi32(0, 0, 0, i);
			plainText[2] = _mm_set_epi32(0, 0, 0, i + numberOfGates);
			plainText[3] = _mm_set_epi32(0, 0, 0, i + numberOfGates);

			//create the plaintext which is the ks4_ec4
			intrin_sequential_ks4_enc4((const unsigned char*)plainText, (unsigned char*)cipherText, 4, (unsigned char*)KEY, (unsigned char*)keys, NULL);

			//T0 = AES(a0, i) XOR AES(a0, i) XOR lsb(a0)deltaFreeXor

			if (wire1signalBitsArray[0] == 0){
				if (!equalBlocks(garbledTables[2 * nonXorIndex], _mm_xor_si128(cipherText[0], cipherText[1])))
					return false;
			}
			else{//signal bit is 1
				if (!equalBlocks(garbledTables[2 * nonXorIndex], _mm_xor_si128(_mm_xor_si128(cipherText[0], cipherText[1]), *deltaFreeXor)))
					return false;
			}

			block tempK0, tempK1;

			if (wire0signalBitsArray[0] == 0){
				tempK0 = cipherText[0];
			}
			else{
				tempK0 = _mm_xor_si128(cipherText[0], garbledTables[2 * nonXorIndex]);
			}

			if (!equalBlocks(garbledTables[2 * nonXorIndex + 1], _mm_xor_si128(_mm_xor_si128(cipherText[2], cipherText[3]), keys[0])))
				return false;

			if (wire1signalBitsArray[0] == 0){
				tempK1 = cipherText[2];
			}
			else{
				tempK1 = _mm_xor_si128(_mm_xor_si128(cipherText[2], garbledTables[2 * nonXorIndex + 1]), keys[0]);
			}

			//set the garbled output to be the XOR of the two temp keys
			garbledWires[garbledGates[i].output] = _mm_xor_si128(tempK0, tempK1);

			nonXorIndex++;
		}

	}

	//copy the output keys to return to the caller of the function
	for (int i = 0; i < numberOfOutputs; i++) {
		emptyBothWireOutputKeys[2 * i] = garbledWires[outputIndices[i]];
		emptyBothWireOutputKeys[2 * i + 1] = _mm_xor_si128(emptyBothWireOutputKeys[2 * i], *deltaFreeXor);

	}

	return true;

}