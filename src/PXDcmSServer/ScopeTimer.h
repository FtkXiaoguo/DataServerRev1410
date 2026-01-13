/***********************************************************************
 * $Id: ScopeTimer.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		Time how long it takes for this instance to go out of scope.
 *		Useful for timing functions and code blocks.
 *
 *	AUTHOR(S):  Rob Lewis, March 2002		
 *
 *-------------------------------------------------------------------
 */

#include <windows.h>
#include "globals.h"

class ScopeTimer 
{
public:
	ScopeTimer(const char* iBlockName, int iThreshold = 0)
	{ 
		QueryPerformanceCounter(&m_start); 
		m_blockName = iBlockName; 
		m_threshold = iThreshold;
	}

	virtual ~ScopeTimer()
	{ 
		QueryPerformanceCounter(&m_end); 
		long double elapsedMilliseconds = (long double)(((m_end.QuadPart - m_start.QuadPart) * 1000)) / (long double) (gCountsPerSecond.QuadPart);
	
		if (elapsedMilliseconds > (long double) m_threshold)
			gConfig.LogMessage(kErrorMsg, gConfig.m_logLocation, "PROFILE: %s - Elapsed Time = %.2lf\n", m_blockName, elapsedMilliseconds);
	}

protected:
	RTVTime m_timer;
	const char* m_blockName;
	int m_threshold;
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_end;
};
