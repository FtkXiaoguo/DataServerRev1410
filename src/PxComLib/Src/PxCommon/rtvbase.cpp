/***********************************************************************
 * rtvbase.cpp
 *---------------------------------------------------------------------
 *
 *   
 *-------------------------------------------------------------------
 */

#include "rtvbase.h"

#include <stdio.h>   // for vsprintf()
#include <stdarg.h>

//-----------------------------------------------------------------------------

int iRTVBase::sVerbose = 0;


#if defined(_WIN32) && !defined (vsnprintf)
#define vsnprintf _vsnprintf
#endif

//GL 2005-9-14 move TRCriticalSection from class member to singlton here
// to real lock out stderr
#ifdef _DEBUG
TRCriticalSection iRTVBase::m_stderrcs; // changed to class member to avoid cleanup problems (tcz 2005.09.22)
#endif

iRTVBase::iRTVBase(void)
{
    m_verbose = iRTVBase::sVerbose;
}

//-----------------------------------------------------------------------

iRTVBase::~iRTVBase(void)
{
}

//-----------------------------------------------------------------------
void iRTVBase::Message(const char *fmt, ...)
{	
#ifdef _DEBUG
	if( m_verbose <= 0 || !fmt || !*fmt)
		return;
	
	char mbuf[2048];
	va_list args;
	
	va_start(args, fmt);
	vsnprintf(mbuf, sizeof mbuf - 1, fmt, args);
	va_end(args);
	
	//GL  09/14/2005 lock globle stderr
	TRCSLock L(&m_stderrcs);
	
	fputs(mbuf, stderr);
#endif
}


//-----------------------------------------------------------------------
void iRTVBase::Message0(const char *fmt, ...)
{
#ifdef _DEBUG
	if (!fmt || !*fmt || m_verbose < 0)
		return;

	char mbuf[2048];
    va_list args;
	
    va_start(args, fmt);
    vsnprintf(mbuf,sizeof(mbuf), fmt, args);
    va_end(args);
	
	//GL  09/14/2005 lock globle stderr
	TRCSLock L(&m_stderrcs);

    fputs(mbuf, stderr);
#endif
}

//-----------------------------------------------------------------------
void iRTVBase::Echo(const char *fmt, ...)
{
#ifdef _DEBUG
    if(!fmt || !*fmt)
		return;
	
    char mbuf[2048];
    va_list args;
	
    va_start(args, fmt);
    vsnprintf(mbuf, sizeof(mbuf), fmt, args);
    va_end(args);
	
	//GL  09/14/2005 lock globle stderr
	TRCSLock L(&m_stderrcs);

    fputs(mbuf, stderr);
#endif
}

//-----------------------------------------------------------------------
void iRTVBase::Message2(const char *fmt, ...)
{
#ifdef _DEBUG
    if(m_verbose < 2 || !fmt || !*fmt)
		return;
	
    char mbuf[2048];
    va_list args;
	
    va_start(args, fmt);
    vsnprintf(mbuf, sizeof(mbuf), fmt, args);
	mbuf[sizeof mbuf - 1] = '\0';
    va_end(args);
	
	//GL  09/14/2005 lock globle stderr
	TRCSLock L(&m_stderrcs);

    fputs(mbuf, stderr);
#endif
}

//-----------------------------------------------------------------------
void iRTVBase::ErrMessage(const char *fmt, ...)
{
#ifdef _DEBUG
    if(!fmt || !*fmt || m_verbose < 0)
    {
		fputs("bad error message", stderr);
		return;
    }
	
    char mbuf[2048];
    va_list args;
	
    va_start(args, fmt);
    vsnprintf(mbuf, sizeof(mbuf), fmt, args);
	mbuf[sizeof mbuf - 1] = '\0';
    va_end(args);
	
	//GL  09/14/2005 lock globle stderr
	TRCSLock L(&m_stderrcs);

	fputs(mbuf, stderr);
#endif
}

