/***********************************************************************
 * TRLogger.h
 *---------------------------------------------------------------------
 *  
 * 
 */

#ifndef TRLOGGER_H_
#define TRLOGGER_H_

#include "AqCore.h"
#include <stdio.h>

class TRCriticalSectionAPI;

//------------------------------------------------------
class TRLogger : public AqLoggerInterface
{
public:

	TRLogger(void);
	virtual ~TRLogger(void);

	static void	SetDefaultLogLevel(int iV) 
	{
		sLogLevel = iV;
	}

	void UseMutex(const char* iName, int iYN=1);
	
	virtual void SetLogLevel(int iFlag) {m_logLevel = iFlag;};
	virtual int	GetLogLevel() {return m_logLevel;};

	virtual void LogMessage(const char *fmt, ...);
	virtual void LogMessage(int iLevel, const char *fmt, ...);
	virtual void LogMessageWithSysError(const char *fmt, ...);

	virtual void WriteLogMessage(const char* ifmt, va_list arguments, const char* iPrefix=0);
	virtual void FlushLog(void);
	virtual void RotateLog(unsigned iMaxLogSize=0);

	/* set the logfile where LogMessage() go to
	*
	* if no LogFile set, LogMessage() write to stderr 
	*/

	void SetLogFile(const char *iFile, const char *iMode="a+");
	void SetLogPrefix(const char* iPrefix);
	void SetMaxLogFileSize(int iMbytes);
	void CloseLog(void);

protected:
	static int	sLogLevel;

	int			m_logLevel;
	int			m_useMutex;
	unsigned	m_maxLogFileSize;
	char		m_logFile[512];
	char		m_prefix[64];
	FILE*		m_logFP;
	TRCriticalSectionAPI*	m_cs;
};

#endif
