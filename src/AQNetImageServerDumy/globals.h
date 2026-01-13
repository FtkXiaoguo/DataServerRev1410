/***********************************************************************
 * globals.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		Centralize global variables
 *
 *	AUTHOR(S):  Rob Lewis, Sept 2001
 *
 *-------------------------------------------------------------------
 */
#ifndef GLOBALS_H
#define GLOBALS_H
#pragma warning (disable: 4786)

#include "PXDcmJobProcConfigFile.h"
#include "AqCore/TRLogger.h"
extern JobProcServerConfigFile gConfig;
extern TRLogger gLogger;

class TRCriticalSection;

#include "rtvtime.h"
//	Used for performance profiling
extern LARGE_INTEGER gCountsPerSecond;
extern TRCriticalSection gDBCS;

#endif // GLOBALS_H