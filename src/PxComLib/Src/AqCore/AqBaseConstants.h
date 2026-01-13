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
 **** File       : AqBaseConstants.h							  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Definition for base constants			      ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 Prashant Chopra 2005-12-16
 Include stdafx.h. 

 ********************************************************************/

#ifndef AQ_BASE_CONSTANTS_H
#define AQ_BASE_CONSTANTS_H "AQ_BASE_CONSTANTS_H"

#include "AqBaseTypes.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <memory.h>

//GL 12-16-2005 should never include stdafx in header file
//#include "stdafx.h"

// ************************** Section I: 
#define kMaxValueLengthInString 512
#define kMaximumStudyIdLength kMaxValueLengthInString
#define kMaximumSeriesIdLength kMaxValueLengthInString
#define kAqMinutesPerHour 60

typedef enum {		
			kAqUnknownError=-1,			// Unknown error
			kAqFailure=0,				// Failure
			kAqSuccess=1,				// Success. Expected behaviour
			kAqErrBadInputParameters=2	// Bad input parameters	
} eAqReturnType;


typedef enum {
			kAqVolumeDataTypeUnknown=-1, 
			kAqVolumeDataTypeUnsignedChar=0, 
			kAqVolumeDataTypeChar=2, 
			kAqVolumeDataTypeUnsignedBpp12=3, 
			kAqVolumeDataTypeBpp12=4, 
			kAqVolumeDataTypeUnsignedBpp16=5, 
			kAqVolumeDataTypeBpp16=6, 
			kAqVolumeDataTypeUnsignedShort=7, 
			kAqVolumeDataTypeShort=8
} eAqVolumeDataType;


typedef enum 
{
			kAqCT = 0,
			kAqMR,
			kAqXA,
			kAqXF,
			kAqRF,
			kAqUS,
			kAqCR,
			kAqDR,
			kAqDX,
			kAqSC,
			kAqNM,
			kAqPT,
			kAqUnsupportedModality
} eAqDicomModality;


typedef enum {
			kAqUnknownStatus=-1, 
			kAqRunning=0, 
			kAqHungup=1, 
			kAqNotEnoughMemory=3, 
			kAqDone=4
} eAqProcessStatus;


// ************************** End Section I

// ************************** Begin Section II

// common math macros
#ifndef kAqEpsilon		
#define kAqEpsilon	0.0001
#endif

#define kAqPI 3.1415926535



#define AQ_ISZERO(_v) (fabs(_v) <= kAqEpsilon)
#define AQ_SIGNOF(a_) (((a_)<0.0)?-1:1)
#define AQ_ROUND(a_) ((a_)>0?(int)((a_)+0.5): -(int)(0.5-(a_)))
#define AQ_INBETWEEN(l_,h_,a_) ((a_)>=(l_) && (a_)<=(h_))
#define AQ_SWAP(a_,b_) { a_^=b_; b_^=a_; a_^=b_; }
#define AQ_CLAMP(_l, _h, _v) (((_v)<(_l))?(_l):(((_v)>(_h))?(_h):(_v)))
#ifdef AQ_MIN
#undef AQ_MIN
#endif
#define AQ_MIN(_a, _b) (((_a) < (_b))?(_a):(_b))
#ifdef AQ_MAX
#undef AQ_MAX
#endif
#define AQ_MAX(_a, _b) (((_a) > (_b))?(_a):(_b))

// ************************** End Section V

#endif