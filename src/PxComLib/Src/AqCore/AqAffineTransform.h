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
 **** File       : AqAffineTransform.h							  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface for the AqAffineTransform class.	  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqAffineTransform.h: interface for the AqAffineTransform class.
//
//////////////////////////////////////////////////////////////////////

// always try to use set dof parameters when composing matrices
// matrices shouldnt be directly copied for a transform
// special care should be taken when:
//		- inverting the transform
//		- pre/post applying a transform
//
//		Ideally, matrix and the dof parameters should correspond one to one.
//

#ifndef _AQ_AFFINETRANSFORM_H_
#define _AQ_AFFINETRANSFORM_H_ "_AQ_AFFINETRANSFORM_H_"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AqTransform.h"

#ifdef TR_USE_QUATERNIONS
#include "AqQuaternion.h"
#endif

#define kAqDoFCount 12 // 3 (rot) + 3 (trans) + 3 (scale) + 3 (shear)
typedef enum {
	kAqTransX=0,
	kAqTransY=1,
	kAqTransZ=2,
	kAqRotX=3,
	kAqRotY=4,
	kAqRotZ=5,
	kAqScaleX=6,
	kAqScaleY=7,
	kAqScaleZ=8,
	kAqShearXY=9,	
	kAqShearYZ=10,	
	kAqShearXZ=11
} eAqTagDoF;



// simple, amoeba
static double kAqDoFWeight[2][kAqDoFCount] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

static double kAqIdentityDoFValues[kAqDoFCount] = {
	0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0
};

// Assuming a row major representation
// which means that if a 16 element array is given, and it is
// accessed as a 4x4 matrix, the elements are arranged in a 'row major'
// format. i.e. for an element A(i,j), i would be the row index, and j would
// be the column index.

class AqAffineTransform : public AqTransform
{
public:
	AqAffineTransform();
	virtual ~AqAffineTransform();	
	
	
	// to not to do that, call the blahNow versions
	virtual int update();



	virtual unsigned short getDoFCount();	
	virtual unsigned short getActiveDoFCount();	
	virtual int invert();		

#ifdef TR_DEBUG
	int setLogger(TRLogger *logger);
#endif
	// set/get major data
	void copy(AqAffineTransform *from);
	void copyMatrix(AqMatrix4x4 *from);
	void copyMatrix(AqAffineTransform *from);	
	void getMatrix(AqMatrix4x4 *mat);
	
	

	// set/get activity flags
	void setShearFlagDoF(unsigned char inflag);
	void setTranslationFlagDoF(unsigned char inflag);
	void setRotationFlagDoF(unsigned char inflag);
	void setScalingFlagDoF(unsigned char inflag);
	int setFlagDoF(unsigned int, unsigned char);			
	
	void deactivateAllDoF(void);
	void activateAllDoF(void);
	
	bool isActiveShearDoF();
	bool isActiveTranslationDoF();
	bool isActiveRotationDoF();
	bool isActiveScalingDoF();
	bool isActiveDoF(unsigned int);
	bool isActiveDoF(eAqTagDoF);

	unsigned char firstActiveDegreeIndex(char from=-1);
	
	// get set dof activity flags
	int setDoFParameters(double parameters[kAqDoFCount]);  
	int setDoFParameter(unsigned int, double);	
	int setDoFParameter(eAqTagDoF, double);
	
	void getDoFParameters(double *out);	
	double getDoFParameter(unsigned int);
	double getDoFParameter(eAqTagDoF);



	// operations
	/*					PRE				*/

	// apply the incoming transform before the existing matrix
	// i.e. if existing matrix is A, and incoming transform is T,
	// a pre-transform would mean 
	//		A = A.T
	void preApplyTransform(AqMatrix4x4 *);
	void preApplyTransform(AqAffineTransform *other);
	
	// translation
	void preTranslate(double x, double y, double z);	
	void preTranslate(const double x[3]);
	void preTranslate(const float x[3]);	
	
	/* rotation assumes the angle values in degrees*/	
	void preRotate(double x, double y, double z);	
	void preRotate(const double x[3]);
	void preRotate(const float x[3]);

	// scaling
	void preScale(double x, double y, double z);	
	void preScale(const double x[3]);
	void preScale(const float x[3]);


	/*					POST				*/
	// apply the incoming transform *after* the existing matrix
	// i.e. if existing matrix is A, and incoming transform is T,
	// a post-transform would mean 
	//		A = T.A
	void postApplyTransform(AqMatrix4x4 *);	
	void postApplyTransform(AqAffineTransform *other);

	void postTranslate(double x, double y, double z);	
	void postTranslate(const double x[3]);
	void postTranslate(const float x[3]);

	/* rotation assumes the angle values in degrees*/
	void postRotate(const double x[3]);
	void postRotate(const float x[3]);	
	void postRotate(double x, double y, double z);	

	// scale
	void postScale(double x, double y, double z);	
	void postScale(const double x[3]);
	void postScale(const float x[3]);


	// point transformations
	//void transformPoint(const float in[4], float out[4]);
	void transformPoint(double in[4]);
	void transformPoint(const double in[4], double out[4]);

	// set origin for rotation/scaling/schear
	void setOrigin(AqVector *tIn);
	void getOrigin(AqVector *tOut);


	// equality
	bool isEqual(AqAffineTransform *other);
	bool isEqual(AqAffineTransform *other1, AqAffineTransform *other2);	

	//identity
	void identity();
	void randomize();
 
	// dump	
//	void print(TRLoggingLevel loglevel=TR_DUMP_PLUS);
//	void printFlags(TRLoggingLevel loglevel=TR_DUMP_PLUS);
//	void printDOFparameters(TRLoggingLevel loglevel=TR_DUMP_PLUS);
	//void printDOFparameter(unsigned int, TRLoggingLevel loglevel=TR_DUMP_PLUS);
private:
	AqMatrix4x4 *Matrix;	
	AqMatrix4x4 *tempMatrix1;
	AqMatrix4x4 *tempMatrix2;	

	AqVector *tOrigin;// for rotation, shear, and scaling

	double _dofParameters[kAqDoFCount];
	unsigned char activeFlagsDoF[kAqDoFCount];

	// 0 -  tX  - kAqTransX  
	// 1 -  tY  - kAqTransY  
	// 2 -  tZ  - kAqTransZ  
	// 3 -  rx  - kAqRotX  
	// 4 -  ry	- kAqRotY  
	// 5 -  rz	- kAqRotZ  
	// 6 -  sx	- kAqScaleX  
	// 7 -  sy	- kAqScaleY  
	// 8 -	sz	- kAqScaleZ 
	// 9 -  sxy - kAqShearXY	
	// 10 - syz - kAqShearYZ	
	// 11 - sxz - kAqShearXZ

#ifdef TR_USE_QUATERNIONS
	AqQuaternion _quatX;
	AqQuaternion _quatY;
	AqQuaternion _quatZ;
#endif

	void _getTranslationMatrix(double, double, double, AqMatrix4x4 *);
	void _getRotationMatrix(double, double, double, AqMatrix4x4 *);
	void _getScalingMatrix(double, double, double, AqMatrix4x4 *);
	
	void _getShearingMatrix(double, double, double, AqMatrix4x4 *);
	void _getShearingMatrixX(double, AqMatrix4x4 *);
	void _getShearingMatrixY(double, AqMatrix4x4 *);
	void _getShearingMatrixZ(double, AqMatrix4x4 *);

};

#endif // !defined(AFX_TRAFFINETRANSFORM_H__61FD8FE7_562F_4CD0_8CC2_A3EAAAB5B9FF__INCLUDED_)
