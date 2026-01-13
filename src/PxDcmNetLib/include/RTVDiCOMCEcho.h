/***********************************************************************
 * $Id: RTVDiCOMCEcho.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes RTVDiCOMCEcho Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef RTVDiCOMCEcho_H
#define RTVDiCOMCEcho_H

#include "RTVDiCOMService.h"

class RTVDiCOMCEcho : public RTVDiCOMService
{
public:
	RTVDiCOMCEcho(DiCOMConnectionInfo& connectInfo, int iMessageID);
	~RTVDiCOMCEcho() {}
	
	virtual void LogProcessStatus(void) {};
	virtual int Process();

private:
};

#endif // RTVDiCOMCEcho_H
