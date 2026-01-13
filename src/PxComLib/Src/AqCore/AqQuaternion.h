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
 **** File       : AqQuaternion.h								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface for the AqQuaternion class.		  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqQuaternion.h: interface for the AqQuaternion class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AQ_QUATERNION_H_
#define _AQ_QUATERNION_H_ "_AQ_QUATERNION_H_"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "AqVector.h"
#include "AqMatrix4x4.h"

class AqQuaternion
{
public:
	AqQuaternion();
	virtual ~AqQuaternion();

	  // axis of rotation and the angle of rotation in degrees
	  void initAxisAngle(AqVector *qVec);

	  // axis of rotation and the angle of rotation in degrees
	  void initAxisAngle(AqVector *vIn, double sIn);

	  // axis of rotation and the angle of rotation in degrees
	  void initAxisAngle(double vX, double vY, double vZ, double sIn);

	  void init(AqVector *vIn, double wIn);

	  void init(double vX, double vY, double vZ, double wIn);

	  void init(AqMatrix4x4 *mIn); 

	  void getMatrix(AqMatrix4x4 *mOut); 

	  // get angle of rotation in radians
	  void getAngle(double *angleOut); 

	  // get angle of rotation in degrees
	  void getAngleDeg(double *sOut); 

	  void getAxis(AqVector *axisOut); 
	
	  void getScalar(double *sOut); 

	  void getVector(AqVector *vOut); 

	  void normalize(); 

	  // to aggregate rotations
	  void multiply(AqQuaternion *qIn); 
	
	  void multiply(AqQuaternion *qIn, AqQuaternion *qOut); 

	  void invert(); 

	  void invert(AqQuaternion *qOut); 	  

	  void conjugate(AqVector *vOut); 

	  static  void invert(AqQuaternion *qIn, AqQuaternion *qOut);

	  static   void conjugate(AqQuaternion *qIn, AqVector *vOut);

private:
	double _scalar;	
	AqVector _vector;
	double _normal;

	// tem variables to store intermediate results
	AqVector _tempVector, _vIn, _tempVec1;
	double _tempScalar, _scIn;
	AqVector _axis;

	double _sin_a, _cos_a, _angle;

	void _computeNormal();

	void _quatToMat(AqMatrix4x4 *mOut);	
	void _matToQuat(AqMatrix4x4 *mIn);
	
	
};

#endif // !defined(AFX_TRQUATERNION_H__CE42F093_9AC2_4355_89EF_486D843AF004__INCLUDED_)
