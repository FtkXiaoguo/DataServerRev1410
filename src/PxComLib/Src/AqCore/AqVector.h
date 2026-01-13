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
 **** File       : AqVector.h									  ****
 **** Author     : Prashant Chopra | Keiji Ito (for part)		  ****
 **** Comments   : Interface for AqVector class.				  ****
 **** 			   Part of the implementation was borrowed from	  ****
 ****			   a class written by Keiji I...mostly for vector ****
 ****			   products (see include\kmath.h in AQWS		  ****
 ****			   codebase)									  ****
 *********************************************************************
 ********************************************************************/
// AqVector.h: interface for the AqVector class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AQVECTOR_H_
#define _AQVECTOR_H_ "_AQVECTOR_H_"

#include "AqTransform.h"

#include <math.h>

class AqVector 
{
public:
	AqVector();
	virtual ~AqVector();

	inline double distance(const AqVector *in) {	
		return AqVector::distance(this->tuple, in->tuple);	
	}

	static inline double distance(const double in[4], const double out[4]) {
		double dist = 0.0;
		for (unsigned int i=0; i<3; i++) {
			dist+= ((out[i] - in[i]) * (out[i] - in[i]));
		}
		dist = sqrt(dist);
		return dist;
	}	
	
	static inline double distance(const AqVector *in, const AqVector *out) {
		return AqVector::distance(in->tuple, out->tuple);
	}	
	

	inline void copy(const AqVector *src) {
		AqVector::copy(src->tuple, this->tuple);
	}
	
	static inline void copy(const AqVector *src, AqVector *dst) {
		AqVector::copy(src->tuple, dst->tuple);
	}

	static inline void copy(const double src[4], double dst[4]) {
		for (unsigned int i=0; i<3; i++) {
			dst[i] = src[i];
		}
		dst[3] = 1.0;
	}

	inline void randomize () {
		AqVector::randomize(this->tuple);
	}

	static inline void randomize(double src[4]) {
		for (unsigned int i=0; i<3; i++) {
			src[i] = (double)rand();
		}
		src[3] = 1.0;
	}

	inline void reverse() {
		for (unsigned int i=0; i<3; i++) {
			this->tuple[i] *= -1.0;
		}		
	}

	inline void init(const double x, const double y, const double z) {
		AqVector::init(this->tuple, x, y, z);
	}

	static inline void init(double dst[4], double x, double y, double z) {
		dst[0] = x;
		dst[1] = y;
		dst[2] = z;
		dst[3] = 1.0;
	}

	inline void init(const double src[4]) {
		AqVector::init(this->tuple, src[0], src[1], src[2]);
	}

	inline void init(AqVector *src) {		
		AqVector::copy(src->tuple, this->tuple);
	}

	
	inline void subtract(AqVector *other) {
		AqVector::subtract(this, other, this);
	}

	inline void subtract(AqVector *other, AqVector *out) {
		AqVector::subtract(this, other, out);
	}

	static inline void subtract(AqVector *from, AqVector *other, AqVector *out) {
		for (unsigned int i=0; i<3; i++) {
			out->tuple[i] = from->tuple[i] - other->tuple[i];
		}
	}

	inline void add(AqVector *other) {
		AqVector::add(this, other, this);
	}

	inline void add(AqVector *other, AqVector *out) {
		AqVector::add(this, other, out);
	}

	static inline void add(AqVector *with, AqVector *other, AqVector *out) {
		for (unsigned int i=0; i<3; i++) {
			out->tuple[i] = with->tuple[i] + other->tuple[i];
		}
	}

	// asserts only that the vector tips are closer than the tolerance
	inline bool closePoint(AqVector *in) {
		return AqVector::closePoint(this->tuple, in->tuple);
	}

	static inline bool closePoint(const double src[4], const double dst[4]) {		
		double dist = AqVector::distance(src, dst);
		return (dist<kAqEpsilon);
	}
	
	inline bool equalPoint(AqVector *in) {
		return AqVector::equalPoint(this->tuple, in->tuple);
	}

	static inline bool equalPoint(const double src[4], const double dst[4]) {		
		double dist = AqVector::distance(src, dst);
		return (dist==0.0);
	}

	// asserts that the vec directions are same
	inline bool equalVec(AqVector *in) {
		return AqVector::equalVec(this->tuple, in->tuple);
	}

	static inline bool equalVec(const double src[4], const double dst[4]) {				
		double tvec0[4], tvec1[4];
		AqVector::normalize(src, tvec0);
		AqVector::normalize(dst, tvec1);
		double dot = AqVector::dot(tvec0, tvec1);		
		return ( (fabs(dot)-1.0) < kAqEpsilon );
	}

	inline double length() {
		return AqVector::length(this->tuple);
	}
	
	static inline double length(const double src[4]) {
		double zvec[4] = {0, 0, 0, 1};
		return (AqVector::distance(src, zvec));
	}	

	inline int normalize() {
		int res = kAqSuccess;
		if((res=AqVector::normalize(this->tuple))!=kAqSuccess) {
			res = kAqFailure;
			//#ifdef TR_DEBUG
			//	TR_LOG(this->logptr(),"ERROR(AqVector::normalize:<3>): Possible div by zero!");
			//	this->error(res);
			//#endif
		}
		return res;
	}

	inline int normalize(AqVector *out) {
		int res = kAqSuccess;
		if((res=AqVector::normalize(this->tuple, out->tuple))!=kAqSuccess) {
			res = kAqFailure;
			//#ifdef TR_DEBUG
				//TR_LOG(this->logptr(),"ERROR(AqVector::normalize:<3>): Possible div by zero!");
				//this->error(res);
			//#endif
		}
		return res;
	}

	static inline int normalize(double src[4]) {
		int res = kAqSuccess;
		double vlen = AqVector::length(src);

		if (vlen<kAqEpsilon) {
			res = kAqFailure;
			return res;
		}

		for (unsigned int i=0; i<3; i++) {
			src[i] /= vlen;
		}
		src[3] = 1.0;
		return res;
	}

	static inline int normalize(const double src[4], double dst[4]) {
		int res = kAqSuccess;
		double vlen = AqVector::length(src);

		if (vlen<kAqEpsilon) {
			res = kAqFailure;	
			return res;
		}

		for (unsigned int i=0; i<3; i++) {
			dst[i] = src[i] / vlen;
		}
		dst[3] = 1.0;
		return res;
	}

	inline void multiply(double scIn) {
		AqVector::multiply(this, scIn, this);
	}

	inline void multiply(double scIn, AqVector *out) {
		AqVector::multiply(this, scIn, out);
	}

	static inline void multiply(AqVector *in, double scIn, AqVector *out) {
		for (int i=0; i<3; i++) {
			out->tuple[i] = in->tuple[i]*scIn;
		}
	}

	inline double dot(AqVector *in) {
		return AqVector::dot(this->tuple, in->tuple);
	}
	
	static inline double dot(AqVector *in1, AqVector *in2) {
		return AqVector::dot(in1->tuple, in2->tuple);
	}


	static inline double dot(const double v0[4], const double v1[4]) {
		return (v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2]);
	}

	// store in other
	inline void cross(AqVector *other) {
		AqVector::cross(this->tuple, other->tuple, other->tuple);
	}

	// store the result in out
	inline void cross(AqVector *other, AqVector *out) {
		AqVector::cross(this->tuple, other->tuple, out->tuple);
	}

	static inline void cross(const double in1[4], const double in2[4], double *out) {
		*out = in1[1] * in2[2] - in1[2] * in2[1];
		*(out+1) = in1[2] * in2[0] - in1[0] * in2[2];
		*(out+2) = in1[0] * in2[1] - in1[1] * in2[0];
		*(out+3) = 1.0;
	}

	inline void print() {
		//#ifdef TR_DEBUG
			//TR_LOG(this->logptr(), "\n\n\tAqVector::(%ld): (%lf, %lf, %lf, %lf)!", this, this->tuple[0], this->tuple[1], this->tuple[2], this->tuple[3]);
			//this->dump();
		//#endif
	}

	// will not employ translation since vectors shouldnt/cannot be translated
	inline void transformVector (AqTransform *trans) {
		this->tuple[3] = 0.0;
		AqVector::transform(trans, this, this);
		this->tuple[3] = 1.0;
	}

	inline void transform (AqTransform *trans) {
		AqVector::transform(trans, this, this);
	}

	inline void transform (AqTransform *trans, AqVector *out) {
		AqVector::transform(trans, this, out);
	}


	static inline void transform(AqTransform *trans, AqVector *in, AqVector *out) {
		trans->transformPoint(in->tuple, out->tuple);
	}


	double tuple[4];

private:
		

};

#endif 
