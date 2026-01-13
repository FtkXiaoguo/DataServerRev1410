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
// Filename:	TRIndentLogger.cpp
// Author:		David Guigonis
// Created:		Tuesday, May 02, 2006 at 4:02:47 PM
//

// 5/4/06 Modified by David G: Use 3 decimal places for millisec display

#include "StdAfx.h"
#include "TRIndentLogger.h"
#include "TRPlatform.h"
#include "TRCriticalsection.h"
#include <math.h>

#if defined(_WIN32) && !defined (vsnprintf)
#  define vsnprintf _vsnprintf
#endif

//////////////////////////////////////////////////////////////////////////
// TRIndentLogger

TRIndentLogger::TRIndentLogger()
: m_indent("  "), m_currentIndent(std::string(""))
{
}

TRIndentLogger::TRIndentLogger(const std::string& indent)
: m_indent(indent), m_currentIndent(std::string(""))
{
}

void TRIndentLogger::WriteLogMessage(const char* iFmt, va_list args, const char* iPrefix /* = 0 */)
{
	if (!m_logFP) return; // short cut before do lock
	
	TRCSLock L(m_cs);
	
	if (!m_logFP)
		return;

	if(!iFmt || !*iFmt)
    {
		fputs("Bad error message format", m_logFP);
		return;
    }

	char mbuf[4096];
	
	vsnprintf(mbuf, sizeof mbuf, iFmt, args);
	mbuf[sizeof mbuf - 1] = '\0';

	if(iPrefix == 0)
		iPrefix = "";

#ifdef _DEBUG
	if (m_logFP != stderr && m_logFP != stdout)
	{
		if(iPrefix[0])
			fputs(iPrefix, stderr);
		fputs(mbuf, stderr);
	}
#endif

	char c =  ( strlen(mbuf) == 0 ) ? char(0) : mbuf[0];
	if( c == '{' )
	{
		m_currentIndent += m_indent;
	}

	if(iPrefix)
		fprintf(m_logFP,"%s %s %s %s", TRPlatform::ctimeN(), m_currentIndent.c_str(), iPrefix, mbuf);
	else
		fprintf(m_logFP,"%s %s %s", TRPlatform::ctimeN(), m_currentIndent.c_str(), mbuf);

	if ( c == '}' )
	{
		int l = m_currentIndent.length();
		if ( l >= m_indent.length() )
		{
			m_currentIndent.erase(l-m_indent.length());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// TRTimerIndentLogger

TRTimerIndentLogger::TRTimerIndentLogger()
: TRIndentLogger()
{
}

TRTimerIndentLogger::TRTimerIndentLogger(const std::string& indent)
: TRIndentLogger(indent)
{
}

void TRTimerIndentLogger::WriteLogMessage(const char* iFmt, va_list args, const char* iPrefix /* = 0 */)
{
	if (!m_logFP) return; // short cut before do lock
	
	TRCSLock L(m_cs);
	
	if (!m_logFP)
		return;

	if(!iFmt || !*iFmt)
    {
		fputs("Bad error message format", m_logFP);
		return;
    }

	char mbuf[4096];
	
	vsnprintf(mbuf, sizeof mbuf, iFmt, args);
	mbuf[sizeof mbuf - 1] = '\0';

	if(iPrefix == 0)
		iPrefix = "";

#ifdef _DEBUG
	if (m_logFP != stderr && m_logFP != stdout)
	{
		if(iPrefix[0])
			fputs(iPrefix, stderr);
		fputs(mbuf, stderr);
	}
#endif

	char tbuf[32] = { 0 };
	char c =  ( strlen(mbuf) == 0 ) ? char(0) : mbuf[0];

	if( c == '{' )
	{
		m_currentIndent += m_indent;

		m_timers.push_back( TRPerformanceTimer() );
		m_timers.back().start();

		sprintf(tbuf, "% 19s", "");
	}
	else if( c == '}' )
	{
		TRPerformanceTimer& timer = m_timers.back();
		timer.end();

		double    s_d  = timer.elapse_s();
		int       s_i  = int(floor(s_d));

		double    ms_d = (s_d-s_i)*1000.;
		int       ms_i = int(floor(ms_d));

		double    us_d = (ms_d-ms_i)*1000.;
		int       us_i = int(floor(us_d));

		sprintf(tbuf, "%7d:%03d:%03d sec", s_i, ms_i, us_i);
	}
	else
	{
		sprintf(tbuf, "% 19s", "");
	}

	if(iPrefix)
		fprintf(m_logFP,"%s %s %s %s %s", TRPlatform::ctimeN(), tbuf, m_currentIndent.c_str(), iPrefix, mbuf);
	else
		fprintf(m_logFP,"%s %s %s %s", TRPlatform::ctimeN(), tbuf, m_currentIndent.c_str(), mbuf);

	if ( c == '}' )
	{
		int l = m_currentIndent.length();
		if ( l >= m_indent.length() )
		{
			m_currentIndent.erase(l-m_indent.length());
		}

		m_timers.pop_back();
	}
}