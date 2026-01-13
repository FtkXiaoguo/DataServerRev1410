
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
 **** File       : AqMatrix4x4.cpp								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Implementation of the AqMatrix4x4 class.		  ****
 **** 															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 Prashant Chopra 2005-12-16
 Removed redundant inclusion of stdafx.h. 

 ********************************************************************/


#include "AqMatrix4x4.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AqMatrix4x4::AqMatrix4x4()
{
	AqMatrix4x4::Identity(*this->TheArray); 
}

AqMatrix4x4::~AqMatrix4x4()
{

}

/*
void AqMatrix4x4::PrintElements(TRLoggingLevel loglevel) {
#ifdef TR_DEBUG
	if (loglevel >= this->getLogger()->logLevel()) {
		unsigned int i, j;
		if (this->openlog()) {
			TR_LOG(this->logptr(),"\n\nThe matrix (%ld):\n", this); 
			this->textlogfast();
			for (i=0; i<4; i++) {
				TR_LOG(this->logptr(),"\n");
				this->textlogfast();
				for (j=0; j<4; j++) {
					TR_LOG(this->logptr()," %lf:", this->TheArray[i][j]);
					this->textlogfast();
				}
			}	
			this->closelog();
		}
	}
#endif
}
*/

void AqMatrix4x4::SetElement(unsigned int i, unsigned int j, double value) {	
	this->TheArray[i][j] = value;
}


void AqMatrix4x4::InitToZero(double Elements[16]) {
  Ptr4x4 elements  = (Ptr4x4)Elements;
  unsigned int i,j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
		elements[i][j] = 0.0;
    }
  }
}



void AqMatrix4x4::Identity(double Elements[16]) {
	Elements[0] = Elements[5] = Elements[10] = Elements[15] = 1.0;
	Elements[1] = Elements[2] = Elements[3] = Elements[4] = 
		Elements[6] = Elements[7] = Elements[8] = Elements[9] = 
		Elements[11] = Elements[12] = Elements[13] = Elements[14] = 0.0;
}

// Multiply a homogeneous point (x,y,z,w) by this matrix, 
// p' = A*p.
// The in[4] and out[4] can be the same array.
template<class Class1, class Class2, class Class3>
static inline void TRMatrixMultiplyPoint(Class1 arrayIn[16], Class2 pIn[4], Class3 pOut[4]) {
  Class3 e1 = pIn[0];
  Class3 e2 = pIn[1];
  Class3 e3 = pIn[2];
  Class3 e4 = pIn[3];

  pOut[0] = e1*arrayIn[0]  + e2*arrayIn[1]  + e3*arrayIn[2]  + e4*arrayIn[3];
  pOut[1] = e1*arrayIn[4]  + e2*arrayIn[5]  + e3*arrayIn[6]  + e4*arrayIn[7];
  pOut[2] = e1*arrayIn[8]  + e2*arrayIn[9]  + e3*arrayIn[10] + e4*arrayIn[11];
  pOut[3] = e1*arrayIn[12] + e2*arrayIn[13] + e3*arrayIn[14] + e4*arrayIn[15];
}  


//void AqMatrix4x4::MultiplyPoint(const double arrayIn[16], 
                                 //const float pIn[4], float pOut[4]) {
  //TRMatrixMultiplyPoint(arrayIn, pIn, pOut);
//}


void AqMatrix4x4::MultiplyPoint(const double arrayIn[16], 
                                 const double pIn[4], double pOut[4]) {
  TRMatrixMultiplyPoint(arrayIn, pIn, pOut);
}

void AqMatrix4x4::MultiplyMatrix(const double a[16], const double b[16], double c[16]) {
  Ptr4x4 aMatrix = (Ptr4x4) a;
  Ptr4x4 bMatrix = (Ptr4x4) b;
  Ptr4x4 cMatrix = (Ptr4x4) c;
  unsigned int i, k;
  double temp[4][4];

  for (i = 0; i < 4; i++) {
    for (k = 0; k < 4; k++)  {
      temp[i][k] =  aMatrix[i][0] * bMatrix[0][k] +
                    aMatrix[i][1] * bMatrix[1][k] +
                    aMatrix[i][2] * bMatrix[2][k] +
                    aMatrix[i][3] * bMatrix[3][k];
	}
  }

  for (i = 0; i < 4; i++) {
    cMatrix[i][0] = temp[i][0];
    cMatrix[i][1] = temp[i][1];
    cMatrix[i][2] = temp[i][2];
    cMatrix[i][3] = temp[i][3];
    }
}


float AqMatrix4x4::TRDeterminant3x3(const float c1[3], 
                                     const float c2[3], 
                                     const float c3[3]) {
  return c1[0]*c2[1]*c3[2] + c2[0]*c3[1]*c1[2] + c3[0]*c1[1]*c2[2] -
         c1[0]*c3[1]*c2[2] - c2[0]*c1[1]*c3[2] - c3[0]*c2[1]*c1[2];
}

double AqMatrix4x4::TRDeterminant3x3(double a1, double a2, double a3, 
                                      double b1, double b2, double b3, 
                                      double c1, double c2, double c3) {
    return ( a1 * TRDeterminant2x2( b2, b3, c2, c3 )
           - b1 * TRDeterminant2x2( a2, a3, c2, c3 )
           + c1 * TRDeterminant2x2( a2, a3, b2, b3 ) );
}


double AqMatrix4x4::Determinant(const double inArray[16]) {
  Ptr4x4 elements = (Ptr4x4)inArray;

  double a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4, detResult;
  
  a1 = elements[0][0]; b1 = elements[0][1]; 
  c1 = elements[0][2]; d1 = elements[0][3];

  a2 = elements[1][0]; b2 = elements[1][1]; 
  c2 = elements[1][2]; d2 = elements[1][3];

  a3 = elements[2][0]; b3 = elements[2][1]; 
  c3 = elements[2][2]; d3 = elements[2][3];

  a4 = elements[3][0]; b4 = elements[3][1]; 
  c4 = elements[3][2]; d4 = elements[3][3];

  detResult = a1 * TRDeterminant3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
       - b1 * TRDeterminant3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
       + c1 * TRDeterminant3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
       - d1 * TRDeterminant3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
  return detResult;
}


void AqMatrix4x4::Adjoint(const double inArray[16], double outArray[16])
{
  Ptr4x4 inData = (Ptr4x4) inArray;
  Ptr4x4 outData = (Ptr4x4) outArray;

  double a1, a2, a3, a4, b1, b2, b3, b4;
  double c1, c2, c3, c4, d1, d2, d3, d4;
  
  a1 = inData[0][0]; b1 = inData[0][1]; 
  c1 = inData[0][2]; d1 = inData[0][3];

  a2 = inData[1][0]; b2 = inData[1][1]; 
  c2 = inData[1][2]; d2 = inData[1][3];

  a3 = inData[2][0]; b3 = inData[2][1];
  c3 = inData[2][2]; d3 = inData[2][3];

  a4 = inData[3][0]; b4 = inData[3][1]; 
  c4 = inData[3][2]; d4 = inData[3][3];


  // reversal of row column labels indicates that
  // rows and columns have been transposed

  outData[0][0]  =   
    TRDeterminant3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
  outData[1][0]  = 
    - TRDeterminant3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
  outData[2][0]  =   
    TRDeterminant3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
  outData[3][0]  = 
    - TRDeterminant3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);

  outData[0][1]  = 
    - TRDeterminant3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
  outData[1][1]  =   
    TRDeterminant3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
  outData[2][1]  = 
    - TRDeterminant3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
  outData[3][1]  =   
    TRDeterminant3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
        
  outData[0][2]  =   
    TRDeterminant3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
  outData[1][2]  = 
    - TRDeterminant3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
  outData[2][2]  =   
    TRDeterminant3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
  outData[3][2]  = 
    - TRDeterminant3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
        
  outData[0][3]  = 
    - TRDeterminant3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
  outData[1][3]  =   
    TRDeterminant3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
  outData[2][3]  = 
    - TRDeterminant3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
  outData[3][3]  =   
    TRDeterminant3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
}


int AqMatrix4x4::Invert(const double inArray[16], double outArray[16]) {
  int res = kAqSuccess;

  Ptr4x4 outMatrix = (Ptr4x4)outArray;
  unsigned int i, j;
  double determinant;

  
  //      -1     
  //     A    = adjoint (A) / determinant (A)
  // 

  determinant = AqMatrix4x4::Determinant(inArray);

  // if the determinent is zero, then the inverse matrix is not unique and is singular OR non-invertible.
  if ( fabs(determinant) < kAqEpsilon )  {
		#ifdef TR_DEBUG
			TR_LOG(this->logptr(), "\nMatrix in Invert: ");
			this->dump();
			this->PrintElements(TR_DUMP_PLUS);
		#endif
		res = kAqErrBadInputParameters;
		// log the event
		#ifdef TR_DEBUG
			TR_LOG(this->logptr(), "(AqMatrix4x4::Invert:<%d>): Singular matrix with determinant %lf. Cannot invert!", res, determinant);
			this->warning(TR_IGNORE);	
		#endif

		return res;
  }

  // calculate the adjoint matrix
  AqMatrix4x4::Adjoint(inArray, outArray);

  // scale the adjoint matrix to get the inverse
  for (i=0; i<4; i++) {
    for(j=0; j<4; j++) {
      outMatrix[i][j] = outMatrix[i][j] / determinant;
    }
  }
  return res;
}


void AqMatrix4x4::Copy(double destArray[16], const double sourceArray[16])
{
  for (unsigned int i = 0; i < 16; i++) {
    destArray[i] = sourceArray[i];
    }
}



bool AqMatrix4x4::IsEqual(double destArray[16], const double sourceArray[16])
{
  bool res = 1;
  for (unsigned int i = 0; i < 16; i++) {
	  if( fabs(destArray[i] - sourceArray[i]) > kAqEpsilon) {
		res = 0;
		break;
	  }
    }
  return res;
}


bool AqMatrix4x4::IsIdentity(AqMatrix4x4 *other, double tolerance) 
{
	unsigned int i, j;
	bool outcome = 1;	
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			if (i==j) {
				if (  fabs(other->GetElement(i, j) - 1.0) > tolerance) {
					outcome = 0;
					break;
				};
			} else {
				if (  fabs(other->GetElement(i, j) - 0.0) > tolerance) {
					outcome = 0;
					break;
				};

			}
		}
	}	
	return outcome;
}


void AqMatrix4x4::Transpose(const double inArray[16], double outArray[16])
{
  Ptr4x4 inMatrix = (Ptr4x4)inArray;
  Ptr4x4 outMatrix = (Ptr4x4)outArray;
  unsigned int i, j;
  double temp;

  for (i=0; i<4; i++) {
    for(j=i; j<4; j++) {
      temp = inMatrix[i][j];
      outMatrix[i][j] = inMatrix[j][i];
      outMatrix[j][i] = temp;
    }
  }
}