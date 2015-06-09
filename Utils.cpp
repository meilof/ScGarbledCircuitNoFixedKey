/**
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
* Copyright (c) 2012 - SCAPI (http://crypto.biu.ac.il/scapi)
* This file is part of the SCAPI project.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
* We request that any publication and/or code referring to and/or based on SCAPI contain an appropriate citation to SCAPI, including a reference to
* http://crypto.biu.ac.il/SCAPI.
* 
* SCAPI uses Crypto++, Miracl, NTL and Bouncy Castle. Please see these projects for any further licensing issues.
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
*/
#include "stdafx.h"
#include "Utils.h"
#include <iostream>
#include "NTL/GF2XFactoring.h"
#include "Config.h"



Utils::Utils(int fieldSize)
{

	this->fieldSize = fieldSize;
}

void Utils::generateRandomIrredPolynomial()
{

	GF2X irredPoly = BuildSparseIrred_GF2X(fieldSize);

	//init the field with the newly generated polynomial.
	GF2E::init(irredPoly);

}

GF2E Utils::generateFieldElement(unsigned char* inputByteElement)
{
	//first create a GF2X
	GF2X polynomialElement;

	//translate the bytes into a GF2X element
	GF2XFromBytes(polynomialElement, inputByteElement, fieldSize/8);


	//convert the GF2X to GF2E
	return to_GF2E(polynomialElement);

}

GF2EX* Utils::interpolateFromNTL(block * xVector, block * yVector, int len)
{

	vec_GF2E xVectorNTL; //the x coordinates
	vec_GF2E yVectorNTL; //the y coordinates

	xVectorNTL.SetLength(len);
	yVectorNTL.SetLength(len);

	for (int i = 0; i < len; i++){

		//put the first point in the coordinates arrays.
		xVectorNTL[i] = generateFieldElement((unsigned char *)&(xVector[i]));
		yVectorNTL[i] = generateFieldElement((unsigned char *)&(yVector[i]));

	}

	
	GF2EX* polynomial = new GF2EX;
	//interpolate the points, put the result polynomial in the created polynomial and return it.
	interpolate(*polynomial, xVectorNTL, yVectorNTL);

	return polynomial;
}


block Utils::evalPoint(GF2EX* polynomial, block xValue ){

	block yValue;
	//generate a field element
	GF2E xFieldValue = generateFieldElement((unsigned char *)&xValue);

	GF2E yFieldValue = eval(*polynomial, xFieldValue);

	BytesFromGF2X((unsigned char *)&yValue, rep(yFieldValue), fieldSize / 8);

	return yValue;


}