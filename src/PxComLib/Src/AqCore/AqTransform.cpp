
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
 **** File       : AqTransform.cpp								  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Implementation of the AqTransform class.		  ****
 **** 															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 Prashant Chopra 2005-12-16
 Removed redundant inclusion of stdafx.h. 

 ********************************************************************/


#include "AqTransform.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AqTransform::AqTransform()
{
	
}

AqTransform::~AqTransform()
{

}

void AqTransform::transformPoint(const double in[4], double out[4]) {
	int res = kAqFailure;
	// throw an error here since this method should be overridden

	//#ifdef TR_DEBUG
		// log the event
		//TR_LOG(this->logptr(), "ERROR(AqTransform::transformPoint:<%d>): Abstract definition. Shouldnt be invoked!", res);
		//this->error(res);
	//#endif
	return;
}

void AqTransform::transformPoint(double in[4]) {
	int res = kAqFailure;
	// throw an error here since this method should be overridden

	//#ifdef TR_DEBUG
		// log the event
	//	TR_LOG(this->logptr(), "ERROR(AqTransform::transformPoint:<%d>): Abstract definition. Shouldnt be invoked!", res);
	//	this->error(res);				
	//#endif
	//TR_EXITNOW(res);
	return;
}


int AqTransform::invert() { 
	int res = kAqFailure;
	// throw an error here since this method should be overridden

	//#ifdef TR_DEBUG
		// log the event
	//	TR_LOG(this->logptr(), "(AqTransform::invert:<%d>): Abstract definition. Shouldnt be invoked!", res);
	//	this->warning(TR_IGNORE);				
	//#endif
	//return res;	
	return res;
}


int AqTransform::update() { 
	int res = kAqFailure;
	// throw an error here since this method should be overridden	
	// log the event
	//#ifdef TR_DEBUG
	//	TR_LOG(this->logptr(), "ERROR(AqTransform::update:<%d>): Abstract definition. Shouldnt be invoked!", res);
	//	this->error(res);					
	//#endif	
	//TR_EXITNOW(res);
	return res;	
}

unsigned short AqTransform::getDoFCount() {
	int res = kAqFailure;
	// throw an error here since this method should be overridden
	
	// log the event
	//#ifdef TR_DEBUG		
	//	TR_LOG(this->logptr(), "ERROR(AqTransform::getDoFCount:<6>): Abstract definition. Shouldnt be invoked!");
	//	this->error(res);						
	//#endif
	//TR_EXITNOW(res);
	return res;	
}

unsigned short AqTransform::getActiveDoFCount() {
	int res = kAqFailure;
	// throw an error here since this method should be overridden
	
	// log the event
	//#ifdef TR_DEBUG	
		//TR_LOG(this->logptr(),"ERROR(AqTransform::getActiveDoFCount:<6>): Abstract definition. Shouldnt be invoked!");
		//this->error(res);	
	//#endif
	//TR_EXITNOW(res);
	return res;	
}