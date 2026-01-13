// -*- C++ -*-
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF TERARECON, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART,
// IS STRICTLY PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN
// PERMISSION OF TERARECON, INC.
//
// Filename:	AqMathCore.h
// Author:		David Guigonis
// Created:		Wednesday, July 12, 2006 at 3:03:33 PM
//

#ifndef _AQMATHCORE_H_
#define _AQMATHCORE_H_
//////////////////////////////////////////////////////////////////////////
#include "AqMathConst.h"

#ifndef max
#  define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#  define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

namespace AqMathCore
{
	/* IsZero     */
	template<class T> inline bool IsZero(const T& a) { return (fabs(a) < AqMathConst::EPSILON); }

	/* Abs        */
	template<class T> inline T Abs(const T& a) { return a < 0 ? -a:a; }

	/* To Radians */
	template<class T> inline T ToRadians(const T& a) { return a * AqMathConst::DEG; }
	/* To Degrees */
	template<class T> inline T ToDegrees(const T& a) { return a * AqMathConst::RAD; }

	/* Sqrt        */
	template<class T> inline T Sqrt(const T& a) { return sqrt(a); }
	/* Square      */
	template<class T> inline T Square(const T& a) { return a*a; }
};


//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQMATHCORE_H_
