/*********************************************************************
 *********************************************************************
 **************											**************
 **************	This document contains information that	**************
 **************	   is proprietary to PreXion 	**************
 **************		     All rights reserved.			**************
 **************			Copyright PreXion 		**************
 **************				 2005-2006					**************
 **************											**************
 *********************************************************************
 *********************************************************************
 *********************************************************************
 ****															  ****
 **** File       : AqMatrix4x4.h								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface for the AqMatrix4x4 class.			  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqMatrix4x4.h: interface for the AqMatrix4x4 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AQ_MATRIX4x4_H_
#define _AQ_MATRIX4x4_H_ "_AQ_MATRIX4x4_H_"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "AqBaseConstants.h"


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// To access a double[16] as a double[4][4] array
typedef double (*Ptr4x4)[4];

class AqMatrix4x4 
{
public:
	AqMatrix4x4();
	virtual ~AqMatrix4x4();

	double TheArray[4][4];  

	// Copy operations	
	void Copy(AqMatrix4x4 *source) {
		AqMatrix4x4::Copy(*this->TheArray,source); 		
	}

	static void Copy(double Elements[16], AqMatrix4x4 *source) {
		AqMatrix4x4::Copy(Elements,*source->TheArray); 
	}

	static void Copy(double Elements[16], const double newElements[16]);

    void Copy(const double Elements[16]) {
		this->Copy(*this->TheArray,Elements); 		
	}

	// Comparisons
	bool IsEqual(AqMatrix4x4 *other) {
		return AqMatrix4x4::IsEqual(*this->TheArray,other); 		
	}

	static bool IsEqual(double Elements[16], AqMatrix4x4 *other) {
		return AqMatrix4x4::IsEqual(Elements,*other->TheArray); 
	}

	static bool IsEqual(double Elements[16], const double otherElements[16]);

    bool IsEqual(const double Elements[16]) {
		return this->IsEqual(*this->TheArray,Elements);
	}

	bool IsIdentity() {
		return AqMatrix4x4::IsIdentity(this, kAqEpsilon);
	}

	bool IsIdentity(double tolerance) {
		return AqMatrix4x4::IsIdentity(this, tolerance);
	}

	static bool IsIdentity(AqMatrix4x4 *other, double tolerance);
	

	// Initialization operations	
	void InitToZero() { 
		AqMatrix4x4::InitToZero(*this->TheArray); 		
	}

	static void InitToZero(double Elements[16]);

	inline void Randomize() {
		this->Identity();
		this->SetElement(0,3,rand());
		this->SetElement(1,3,rand());
		this->SetElement(2,3,rand());

		#ifdef TR_DEBUG
			TR_LOG(this->logptr(), "\nMatrix after randomize: ");
			this->dump();
			this->PrintElements(TR_DUMP_PLUS);
		#endif
	}
    

    void Identity() { 
		AqMatrix4x4::Identity(*this->TheArray);
	}

	static void Identity(double Elements[16]);


	// Inversion operations	
	int Invert(AqMatrix4x4 *in, AqMatrix4x4 *out) {
		return AqMatrix4x4::Invert(*in->TheArray,*out->TheArray); 		
	}

    int Invert() {
		return AqMatrix4x4::Invert(this,this); 
	}

	
	int Invert(AqMatrix4x4 &in,AqMatrix4x4 &out) {
		return this->Invert(&in,&out);
	}

    int Invert(const double inElements[16], double outElements[16]);	
	
	void Adjoint(AqMatrix4x4 *in, AqMatrix4x4 *out) {
		AqMatrix4x4::Adjoint(*in->TheArray,*out->TheArray);
	}

	static void Adjoint(const double inArray[16], double outArray[16]);

	double Determinant() {
		return AqMatrix4x4::Determinant(*this->TheArray);
	}

	static double Determinant(const double inArray[16]);

	inline static float TRDeterminant2x2(const float c1[2], const float c2[2]) {
		return (c1[0]*c2[1] - c2[0]*c1[1]);
	};

  
	inline static double TRDeterminant2x2(double a, double b, double c, double d) {
		return (a * d - b * c);
	};


	static float TRDeterminant3x3(const float c1[3], 
                                     const float c2[3], 
                                     const float c3[3]);

	static double TRDeterminant3x3(double a1, double a2, double a3, 
                                      double b1, double b2, double b3, 
                                      double c1, double c2, double c3);

    // Multiplication operations

	// Multiply a homogeneous point (x,y,z,w) by this matrix, 
	// p' = A*p.
	// The in[4] and out[4] can be the same array.
	//void MultiplyPoint(const float in[4], float out[4]) {		
	//		AqMatrix4x4::MultiplyPoint(*this->TheArray,in,out); 		
	//}

	void MultiplyPoint(const double in[4], double out[4]) {		
			AqMatrix4x4::MultiplyPoint(*this->TheArray,in,out); 		
	}

	//static void MultiplyPoint(const double Elements[16], 
                            //const float in[4], float out[4]);
	static void MultiplyPoint(const double Elements[16], 
                            const double in[4], double out[4]);

	// Multiplies matrices a and b and stores the result in c.
	static void MultiplyMatrix(AqMatrix4x4 *a, AqMatrix4x4 *b, AqMatrix4x4 *c) {		
			AqMatrix4x4::MultiplyMatrix(*a->TheArray,*b->TheArray,*c->TheArray); 		
	};

	static void MultiplyMatrix(const double a[16], const double b[16], 
                          double c[16]);

	
	// Transpose operations
    static void Transpose(AqMatrix4x4 *in, AqMatrix4x4 *out) {
		AqMatrix4x4::Transpose(*in->TheArray,*out->TheArray); 
		//this matrix was modified; see if any events will be invoked 
	}

	void Transpose() { 
		AqMatrix4x4::Transpose(this,this); 
	}

	
	void Transpose(AqMatrix4x4 &in,AqMatrix4x4 &out) {
		this->Transpose(&in,&out);
	}

	static void Transpose(const double inElements[16], double outElements[16]);

	
	// Element access operations
	void SetElement(unsigned int i, unsigned int j, double value);

	double GetElement(unsigned int i, unsigned int j) const {
		return this->TheArray[i][j];
	}

	double *operator[](const unsigned int i) {
		return &(this->TheArray[i][0]);
	}
  
	const double *operator[](unsigned int i) const{ 
		return &(this->TheArray[i][0]); 
	}  
	
	//void PrintElements(TRLoggingLevel loglevel=TR_DUMP_PLUS);

protected:
  
  
private:
  
};

#endif // !defined(AFX_TRMATRIX4X4_H__4AF59A75_C02B_4641_B6EF_3CDBF3127021__INCLUDED_)
