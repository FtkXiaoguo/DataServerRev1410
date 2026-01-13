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
 **** File       : AqBaseTypes.h								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Definition for base data types.			      ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 Prashant Chopra 2005-12-16
 Include AqCore.h. 

 ********************************************************************/

#ifndef AQ_BASE_TYPES_H
#define AQ_BASE_TYPES_H "AQ_BASE_TYPES_H"

#include "AqCore.h"


// ************************** Section I *****************************/
//
//		Polymorphic data types AND architecture specific definition
//
// ********************** CHANGE HERE FOR UPGRADES *******************/


#if defined(WIN32)
	// 32-bit platform specific types here	
	typedef __int64						AqINT64;
	typedef unsigned __int64			AqUINT64;

	
	// <long 64 bit>	:	[8 words	| 64bits]
	typedef AqINT64						AqLONG64;	
	typedef AqUINT64					AqULONG64;
	

	// <size_t>			:	[4 words	| 32bits]
	typedef unsigned int				AqSIZE_T;

	// <ptrdiff_t>		:	[4 words	| 32bits]
	typedef int							AqPTR_DIFF;

	// <time_t>			:	[4 words	| 32bits]
	typedef long						AqTIME_IN_SECS;

	// <WPARAM>			:	[4 words	| 32bits]
	typedef unsigned int				AqWPARAM;

	// <LPARAM>			:	[4 words	| 32bits] long
	typedef long						AqLPARAM;

	// <LRESULT>		:	[4 words	| 32bits] long
	typedef long						AqLRESULT;	

#else  	
	// 64-bit platform specific types here

	typedef __int3264					AqINT64;
	typedef unsigned __int3264			AqUINT64;

	
	// <long 64 bit>	:	[8 words	| 64bits]
	typedef AqINT64						AqLONG64;	
	typedef AqUINT64					AqULONG64;
	

	// <size_t>			:	[8 words	| 64bits]
	typedef SIZE_T						AqSIZE_T;

	// <ptrdiff_t>		:	[8 words	| 64bits]
	typedef AqINT64						AqPTR_DIFF;

	// <time_t>			:	[8 words	| 64bits]
	typedef AqINT64						AqTIME_IN_SECS;

	// <generic pointer>:	[8 words	| 64bits]
	typedef AqINT64						AqGENERIC_POINTER;

	// <WPARAM>			:	[8 words	| 64bits]
	typedef WPARAM						AqWPARAM;

	// <LPARAM>			:	[8 words	| 64bits] long
	typedef LPARAM						AqLPARAM;

	// <LRESULT>		:	[8 words	| 64bits] long
	typedef LRESULT						AqLRESULT;	
#endif

// ************************** End Section I


#endif