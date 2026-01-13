/***********************************************************************
 * rtvtime.cpp
 *---------------------------------------------------------------------
 *
 *	
 * 
 *-------------------------------------------------------------------
 */
#ifndef RTVTIME_H_
#include "rtvtime.h"
#endif

//-----------------------------------------------------------------------------

RTVTime::RTVTime(int iOffset)
{
	m_offset = iOffset;
    m_start = GetSysmTime();
}

//------------------------------------------------------------------------------
int RTVTime::GetTime(void) const
{
	return GetSysmTime()/1000;
}

//------------------------------------------------------------------------------
void RTVTime::Reset(void)
{
	m_start = GetSysmTime();
}

//------------------------------------------------------------------------------
int RTVTime::GetStartTime(void) const
{
	return m_start/1000;
}

//------------------------------------------------------------------------------
void RTVTime::SetElapsed(int iSeconds)
{
     m_start = (GetSysmTime() - iSeconds * 1000);
}

#if defined(_WIN32)

//extern "C" { unsigned long __stdcall GetTickCount(void); }

//------------------------------------------------------------------------------
int RTVTime::GetSysmTime(void) const
{
	return GetTickCount() - m_offset*1000;
}
#else

int RTVTime::GetSysmTime(void) const
{
	struct timeval tv;
	gettimeofday(&tv,0);
	return (tv.tv_sec-m_offset)*1000 + tv.tv_usec/1000;
}

#endif
