// -*- C++ -*-
// Copyright 2006 PreXion 
// ALL RIGHTS RESERVED
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
// Filename:	AqCore_API.h
// Author:		David Guigonis
// Created:		Friday, July 28, 2006 at 11:20:38 AM
//

#ifndef _AQCORE_API_H_
#define _AQCORE_API_H_
//////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4251)

#ifdef AQCORE_EXPORTS
#  define AQCORE_API __declspec(dllexport)
//#  error export
#else
#  ifndef STATIC_LINK
#    define AQCORE_API __declspec(dllimport)
//#    error import
#  else
#    define AQCORE_API
#    error nothing
#  endif
#endif


//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQCORE_API_H_
