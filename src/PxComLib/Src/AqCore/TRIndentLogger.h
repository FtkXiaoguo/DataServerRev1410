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
// Filename:	TRIndentLogger.h
// Author:		David Guigonis
// Created:		Tuesday, May 02, 2006 at 3:58:20 PM
//

#ifndef _TRINDENTLOGGER_H_
#define _TRINDENTLOGGER_H_
//////////////////////////////////////////////////////////////////////////
#include "TRLogger.h"
#include "TRPerformanceTimer.h"
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// Same as TRLogger but support indenting
// If log message starts with '{' then it indents the text to the right
// If log message starts with '}' then it indents back the text to the left

class TRIndentLogger : public TRLogger
{
public:
	TRIndentLogger();
	TRIndentLogger(const std::string& indent);
	virtual ~TRIndentLogger() {}

	virtual void WriteLogMessage(const char* ifmt, va_list arguments, const char* iPrefix=0);

protected:
	std::string m_indent;
	std::string m_currentIndent;
};


//////////////////////////////////////////////////////////////////////////
// Same as TRIndentLogger but support timing
// If log message starts with '{' then a timer is started
// If log message starts with '}' then the timer is stopped, and print the elapsed time

class TRTimerIndentLogger : public TRIndentLogger
{
public:
	TRTimerIndentLogger();
	TRTimerIndentLogger(const std::string& indent);
	virtual ~TRTimerIndentLogger() {}

	virtual void WriteLogMessage(const char* ifmt, va_list arguments, const char* iPrefix=0);

protected:
	std::vector<TRPerformanceTimer> m_timers;
};

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _TRINDENTLOGGER_H_
