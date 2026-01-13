/***********************************************************************
 * globals.h
 *-------------------------------------------------------------------
 */
#ifndef GLOBALS_H
#define GLOBALS_H
#pragma warning (disable: 4786)

#include "TstBkOpenGLDaemonConfigFile.h"
#include "AqCore/TRLogger.h"
extern TestDBDaemonConfigFile gConfig;
extern TRLogger gLogger;

class TRCriticalSection;

#include "rtvtime.h"
//	Used for performance profiling
extern LARGE_INTEGER gCountsPerSecond;
extern TRCriticalSection gDBCS;

#endif // GLOBALS_H