#ifndef _API_LOGGER_H
#define _API_LOGGER_H

#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;

class MyLogger : public DcmLibApiLogger
{
public:
	//
	virtual void LoggerError(const char *format,...);
	virtual void LoggerWarn(const char *format,...);
	virtual void LoggerDebug(const char *format,...);
	virtual void LoggerTrace(const char *format,...);
protected:
	void LoggerOutput(char *type, const char* ifmt, va_list arguments );
};

#endif //_API_LOGGER_H
 
 