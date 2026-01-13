/***********************************************************************
 * globals.h
 *-------------------------------------------------------------------
 */
#ifndef GLOBALS_H
#define GLOBALS_H
#pragma warning (disable: 4786)

#include "tstEventLockDaemonConfigFile.h"
#include "AqCore/TRLogger.h"
extern TestEvtLockDaemonConfigFile gConfig;
extern TRLogger gLogger;

class TRCriticalSection;

#include "rtvtime.h"
//	Used for performance profiling
extern LARGE_INTEGER gCountsPerSecond;
extern TRCriticalSection gDBCS;

#endif // GLOBALS_H