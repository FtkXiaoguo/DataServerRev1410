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
 **** File       : AqTransform.h								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface for the AqTransform class.			  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqTransform.h: interface for the AqTransform class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AQTRANSFORM_H_
#define _AQTRANSFORM_H_ "_AQTRANSFORM_H_"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// whether to use quaternions when computing aggregate rotations
#define TR_USE_QUATERNIONS


#include "AqMatrix4x4.h"


typedef enum {kAqAffine=0, kAqFreeform=1} eAqTransformType;

class AqTransform  
{
public:
	AqTransform();
	virtual ~AqTransform();
	
	virtual void transformPoint(double in[4]);
	virtual void transformPoint(const double in[4], double out[4]);
	virtual int invert();
	virtual int update();    
	virtual unsigned short getDoFCount();	
	virtual unsigned short getActiveDoFCount();

	//#ifdef TR_DEBUG
	//	inline int setLogger(TRLogger *logger) {			
	//			return TRObject::setLogger(logger);
	//	}
	//#endif


	eAqTransformType type;	
private:

};

#endif 
