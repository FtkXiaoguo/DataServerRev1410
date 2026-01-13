/***********************************************************************
 * TRLogger.cpp
 *---------------------------------------------------------------------
 *	
 *
 *   
 *-------------------------------------------------------------------
 */

#include "TRLogger.h"

#include "TRCriticalsection.h"


#include "TRPlatform.h"

//#include <string.h>

int TRLogger::sLogLevel = 0;
//-----------------------------------------------------------------------------

#if defined(_WIN32) && !defined (vsnprintf)
#define vsnprintf _vsnprintf
#endif


TRLogger::TRLogger(void):m_cs(0)
{
 	m_logFP = 0;
	m_logFile[0] = '\0';
	m_prefix[0] = '\0';
	m_maxLogFileSize = 3*(1024*1024); // default 3MBytes
	m_logLevel = sLogLevel;
	UseMutex("Logger", 0);
}

//-----------------------------------------------------------------------
// This must be the first call into this object, if Mutex is to be used for locking in stead of CriticalSection.
void TRLogger::UseMutex(const char *name, int iYeaNo)
{
	if (m_cs)
	{
		delete m_cs, m_cs = 0;
	}
	
    m_useMutex = iYeaNo;
	
	if (m_useMutex)
		m_cs = new TRWaitableCS(name);
	else
		m_cs = new TRCriticalSection();
}

//-----------------------------------------------------------------------
void TRLogger::SetLogPrefix(const char* iPrefix)
{
	strncpy(m_prefix, iPrefix ? iPrefix : "", sizeof(m_prefix) );
	m_prefix[sizeof m_prefix - 1] = '\0';
}

//-----------------------------------------------------------------------

TRLogger::~TRLogger(void)
{
//	TRCSLock L(m_cs);

	m_cs->Enter();
	if (m_logFP && m_logFP != stdout)
		fclose(m_logFP);
	m_cs->Leave();
 	delete m_cs;
}

//-----------------------------------------------------------------------
void TRLogger::SetMaxLogFileSize(int iMBytes)
{
	if (iMBytes > 0 && iMBytes < 50)
	{
		m_maxLogFileSize = iMBytes * (1024*1024);
	}
}

//-----------------------------------------------------------------------
void TRLogger::SetLogFile(const char *iFile, const char *iMode)
{
	TRCSLock L(m_cs);

	if (strcmp(m_logFile, iFile) == 0 && m_logFP)
		return;
	
	if (m_logFP && m_logFP != stdout)
		fclose(m_logFP);

	if (stricmp(iFile,"stdout") == 0)
	{
		m_logFP = stdout;
		return;
	}

	char pathname[512+4], *p;
	snprintf(pathname, sizeof pathname, "%s", iFile);
	pathname[sizeof(pathname)-1] = 0;
	if ( (p = strrchr(pathname,'/')) || ( p = strrchr(pathname,'\\')))
	{
		*p = '\0';
	}

	TRPlatform::MakeDirIfNeedTo(pathname);
	m_logFP = fopen(iFile, iMode);
	
	if(!m_logFP)
	{
		m_logFP = stdout;
	}
	else
	{
		if (iFile != m_logFile)
		{
			strncpy(m_logFile, iFile, sizeof m_logFile);
			m_logFile[sizeof m_logFile-1] = 0;
		}
	}
}

//-----------------------------------------------------------------------
void TRLogger::CloseLog(void)
{
	TRCSLock L(m_cs);

	if (m_logFP)
	{
		FILE *fp = m_logFP;
		m_logFP = 0;
		fclose(fp);
	}
}

//-----------------------------------------------------------------------
void TRLogger::RotateLog(unsigned iMaxLogSize)
{
	TRCSLock L(m_cs);
	
	if (!m_logFP || m_logFP == stdout)
		return;

	unsigned maxLogFileSize = (iMaxLogSize == 0)?m_maxLogFileSize : iMaxLogSize;

	unsigned int size = TRPlatform::GetFileSize(m_logFile);
	// We rotate the log if it has grown to more than 2Mbytes
	if (size > maxLogFileSize)
	{
		char backupLog[512];
		if((sLogLevel & (0xffff0000)) !=0){ //2010/03/17 K.Ko
			sprintf(backupLog,"%s_%s",m_logFile,TRPlatform::YYYYMMDDTimeStamp(1));
		}else{
			strcat(strcpy(backupLog,m_logFile),"-old");
		}
		CloseLog();
		TRPlatform::remove(backupLog);
		TRPlatform::rename(m_logFile, backupLog);
		SetLogFile(m_logFile);
	}

}

//----------------------------------------------------------------------
void TRLogger::WriteLogMessage(const char* iFmt, va_list args, const char* iPrefix)
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
	
#if 0
	vsnprintf(mbuf, sizeof mbuf, iFmt, args);
	mbuf[sizeof mbuf - 1] = '\0';
#else
	unsigned long threadID = ::GetCurrentThreadId();
	sprintf(mbuf," [%04x] ",threadID);
	int used_str_len = strlen(mbuf);//-1;
	char *mbuf_ptr = mbuf + used_str_len;
	vsnprintf(mbuf_ptr, sizeof(mbuf)-used_str_len, iFmt, args);
	mbuf[sizeof mbuf - 1] = '\0';
#endif

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

	if(iPrefix)
		fprintf(m_logFP,"%s %s%s %s", TRPlatform::ctimeN(), m_prefix,iPrefix, mbuf);
	else
		fprintf(m_logFP,"%s %s %s", TRPlatform::ctimeN(), m_prefix,mbuf);
}

//----------------------------------------------------------------------
// LogMessage always writes the log entry
void TRLogger::LogMessage(const char *fmt, ...)
{
	if (!m_logFP) return; // short cut before do lock
	
	if(!fmt || !*fmt)
    {
		fputs("Bad error message format", m_logFP);
		return;
    }
	
	TRCSLock L(m_cs);
	
	if (!m_logFP)
		return;
	
	try
	{
		va_list args;
		va_start(args, fmt);
		WriteLogMessage(fmt, args);
		va_end(args);
	}
	catch (...)
	{
		fprintf(m_logFP,"** ERROR in log format detected (%s)\n", fmt);
#ifdef _DEBUG
		if (m_logFP != stderr && m_logFP != stdout)
			fprintf(stderr,"** ERROR in log format detected (%s)\n", fmt);
#endif
	}
}

//----------------------------------------------------------------------
// LogMessage always writes the log entry
void TRLogger::LogMessageWithSysError(const char *fmt, ...)
{
	if (!m_logFP) return; // short cut before do lock
	
	if(!fmt || !*fmt)
    {
		fputs("Bad error message format", m_logFP);
		return;
    }
	
	TRCSLock L(m_cs);
	
	if (!m_logFP)
		return;
	
	try
	{
		char bug[1040] = "ERROR:  ";
		
		TRPlatform::GetSysErrorText(bug+8,1024);

		va_list args;
		va_start(args, fmt);
		WriteLogMessage(fmt, args, bug);
		va_end(args);
	}
	catch (...)
	{
		fprintf(m_logFP,"** ERROR in log format detected (%s)\n", fmt);
#ifdef _DEBUG
		if (m_logFP != stderr && m_logFP != stdout)
			fprintf(stderr,"** ERROR in log format detected (%s)\n", fmt);
#endif
	}
}

//-----------------------------------------------------------------------
void TRLogger::LogMessage(int iLevel, const char* fmt, ...)
{
	if (iLevel > m_logLevel || !fmt || !*fmt)
		return;
	
	if (!m_logFP) return; // short cut before do lock
	TRCSLock L(m_cs);
	
	if (!m_logFP)
		return;
	
	try
	{
		va_list args;
		va_start(args, fmt);
		WriteLogMessage(fmt, args);
		va_end(args);
	}
	catch (...)
	{
		fprintf(m_logFP,"** ERROR in log format detected (%s)\n", fmt);
#ifdef _DEBUG
		if (m_logFP != stderr && m_logFP != stdout)
			fprintf(stderr,"** ERROR in log format detected (%s)\n", fmt);
#endif
	}
}

//-----------------------------------------------------------------------
void TRLogger::FlushLog(void)
{
	if (!m_logFP) return; // short cut before do lock
	TRCSLock L(m_cs);

	if (m_logFP)
		fflush(m_logFP);
}
