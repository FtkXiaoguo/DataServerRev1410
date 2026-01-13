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
 **** File       : AqQuaternion.cpp								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Implementation of the AqQuaternion class.	  ****
 **** 															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 Prashant Chopra 2005-12-16
 Removed redundant inclusion of stdafx.h. 

 ********************************************************************/


#include "AqQuaternion.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AqQuaternion::AqQuaternion()
{
	_scalar = 0.0;			
	_vector.init(0, 0, 0);
}

AqQuaternion::~AqQuaternion()
{

}
 
// axis of rotation and the angle of rotation in degrees
void AqQuaternion::initAxisAngle(AqVector *qVec) {
	this->initAxisAngle(qVec->tuple[0], qVec->tuple[1], qVec->tuple[2], qVec->tuple[3]);	
}

// axis of rotation and the angle of rotation in degrees
void AqQuaternion::initAxisAngle(AqVector *vIn, double sIn) {
	// assumes the scalar component is in degrees
	this->initAxisAngle(vIn->tuple[0], vIn->tuple[1], vIn->tuple[2], sIn);		
}

// axis of rotation and the angle of rotation in degrees
void AqQuaternion::initAxisAngle(double vX, double vY, double vZ, double sIn) {
	// assumes the scalar component is in degrees		
	_angle = sIn*kAqPI/180.0;

	_axis.init(vX, vY, vZ);
	_axis.normalize();

	_sin_a = sin( 0.5*_angle );
	_cos_a = cos( 0.5*_angle );				
		
	_vector.init(&_axis);
	_vector.multiply(_sin_a);
	_scalar = _cos_a;
	this->normalize();
}

void AqQuaternion::init(AqVector *vIn, double wIn) {
	_scalar = wIn;
	_vector.copy(vIn);
}

void AqQuaternion::init(double vX, double vY, double vZ, double wIn) {
	_scalar = wIn;
	_vector.init(vX, vY, vZ);
}

void AqQuaternion::init(AqMatrix4x4 *mIn) {
	this->_matToQuat(mIn);		
	this->normalize();
}

void AqQuaternion::getMatrix(AqMatrix4x4 *mOut) {
	this->_quatToMat(mOut);
}

// get angle of rotation in radians
void AqQuaternion::getAngle(double *angleOut) {		
	*angleOut = acos( _scalar ) * 2.0;
}

// get angle of rotation in degrees
void AqQuaternion::getAngleDeg(double *sOut) {
	this->getAngle(sOut);
	*sOut = 180.0* ((*sOut)/kAqPI);
}

void AqQuaternion::getAxis(AqVector *axisOut) {	
	this->normalize();
		
	_cos_a = _vector.tuple[2];
		
	_sin_a = sqrt( 1.0 - _cos_a * _cos_a );
	if ( fabs( _sin_a ) < kAqEpsilon ) {				
		_sin_a = 1;
	}
	_sin_a = 1.0/_sin_a;
		
	axisOut->init(&_vector);
	axisOut->multiply(_sin_a);	
	axisOut->normalize();
}
	
void AqQuaternion::getScalar(double *sOut) {
	*sOut = _scalar;
}

void AqQuaternion::getVector(AqVector *vOut) {
	vOut->copy(&(_vector));
}

void AqQuaternion::normalize() {
	_computeNormal();
	if(_normal>0.0) {
		for(unsigned char i=0;i<3;i++)
			_vector.tuple[i]/= _normal;
		_scalar/= _normal;
	}
}

// to aggregate rotations
void AqQuaternion::multiply(AqQuaternion *qIn) {
	this->multiply(qIn, this);
}
	
void AqQuaternion::multiply(AqQuaternion *qIn, AqQuaternion *qOut) {
	// qq´ = [ww´ - v · v´, v x v´ + wv´ +w´v] 		
		
	this->normalize();
	qIn->normalize();

	qIn->getScalar(&_scIn);
	qIn->getVector(&_vIn);

	_tempScalar = _scalar*_scIn - _vector.dot(&_vIn);	
			
	_vector.cross(&_vIn, &_tempVector);		

	_vIn.multiply(_scalar, &_tempVec1);
	_tempVector.add(&_tempVec1);

	_vector.multiply(_scIn, &_tempVec1);		
	_tempVector.add(&_tempVec1);

	// copy to outgoing param
	qOut->init(&_tempVector, _tempScalar);
	qOut->normalize();
}

void AqQuaternion::invert() {
	AqQuaternion::invert(this, this);
}

void AqQuaternion::invert(AqQuaternion *qOut) {
	AqQuaternion::invert(this, qOut);
}

void AqQuaternion::invert(AqQuaternion *qIn, AqQuaternion *qOut) {
	AqVector _tempVec;
	double _tempSc;
	AqQuaternion::conjugate(qIn, &_tempVec);
	qIn->getScalar(&_tempSc);
	qOut->init(&_tempVec, _tempSc);
	qOut->normalize();
}

void AqQuaternion::conjugate(AqVector *vOut) {
	AqQuaternion::conjugate(this, vOut);
}

void AqQuaternion::conjugate(AqQuaternion *qIn, AqVector *vOut) {		
	AqVector _tempVec;
	qIn->getVector(&_tempVec);
	vOut->init(-_tempVec.tuple[0], -_tempVec.tuple[1], -_tempVec.tuple[2]);
}

/*********************************** private methods **********************************************/
void AqQuaternion::_matToQuat(AqMatrix4x4 *mIn) {	
	double  tr, s, q[4];
    unsigned short i, j, k;              

    short nxt[3] = {1, 2, 0};


    tr = mIn->TheArray[0][0] + mIn->TheArray[1][1] + mIn->TheArray[2][2];


    // check the diagonal
    if (tr > 0.0) {
          s = sqrt (tr + 1.0);
          _scalar = s / 2.0;
          s = 0.5 / s;
          _vector.init((mIn->TheArray[1][2] - mIn->TheArray[2][1]) * s,  (mIn->TheArray[2][0] - mIn->TheArray[0][2]) * s, (mIn->TheArray[0][1] - mIn->TheArray[1][0]) * s);
    } else {                
          // diagonal is negative
          i = 0;
          if (mIn->TheArray[1][1] > mIn->TheArray[0][0]) i = 1;
          if (mIn->TheArray[2][2] > mIn->TheArray[i][i]) i = 2;
          j = nxt[i];
          k = nxt[j];


          s = sqrt ((mIn->TheArray[i][i] - (mIn->TheArray[j][j] + mIn->TheArray[k][k])) + 1.0);                
          q[i] = s * 0.5;                      
          if (s != 0.0) s = 0.5 / s;
          q[3] = (mIn->TheArray[j][k] - mIn->TheArray[k][j]) * s;
          q[j] = (mIn->TheArray[i][j] + mIn->TheArray[j][i]) * s;
          q[k] = (mIn->TheArray[i][k] + mIn->TheArray[k][i]) * s;

		  _scalar = q[3];
		  _vector.init(q);          
          
     }// end if
}



void AqQuaternion::_quatToMat(AqMatrix4x4 *mOut) {	
	double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
 
    // calculate coefficients
    x2 = _vector.tuple[0] + _vector.tuple[0]; y2 = _vector.tuple[1] + _vector.tuple[1]; 
    z2 = _vector.tuple[2] + _vector.tuple[2];
    xx = _vector.tuple[0] * x2;   xy = _vector.tuple[0] * y2;   xz = _vector.tuple[0] * z2;
    yy = _vector.tuple[1] * y2;   yz = _vector.tuple[1] * z2;   zz = _vector.tuple[2] * z2;
    wx = _scalar * x2;   wy = _scalar * y2;   wz = _scalar * z2;

    mOut->TheArray[0][0] = 1.0 - (yy + zz);    mOut->TheArray[0][1] = xy + wz;
    mOut->TheArray[0][2] = xz - wy;            mOut->TheArray[0][3] = 0.0;
           
    mOut->TheArray[1][0] = xy - wz;            mOut->TheArray[1][1] = 1.0 - (xx + zz);
    mOut->TheArray[1][2] = yz + wx;            mOut->TheArray[1][3] = 0.0;

    mOut->TheArray[2][0] = xz + wy;            mOut->TheArray[2][1] = yz - wx;
    mOut->TheArray[2][2] = 1.0 - (xx + yy);    mOut->TheArray[2][3] = 0.0;

    mOut->TheArray[3][0] = 0;                  mOut->TheArray[3][1] = 0;
    mOut->TheArray[3][2] = 0;                  mOut->TheArray[3][3] = 1;        
}


void AqQuaternion::_computeNormal() {
		_normal = 0.0; 
		for(unsigned char i=0;i<3;i++)
			_normal += _vector.tuple[i]*_vector.tuple[i];
		_normal+= _scalar*_scalar;
		_normal = sqrt(_normal);
}