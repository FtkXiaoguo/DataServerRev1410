// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

 
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
 

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "api_logger.h"

#include<stdarg.h>

void MyLogger::LoggerError(const char *format,...){
	 	va_list args;
		va_start(args, format);
		LoggerOutput("Error",format, args);
		va_end(args);
	}
void MyLogger::LoggerWarn(const char *format,...){
		va_list args;
		va_start(args, format);
		LoggerOutput("Warn",format, args);
		va_end(args);
	}
void MyLogger::LoggerDebug(const char *format,...){
		va_list args;
		va_start(args, format);
		LoggerOutput("Debug",format, args);
		va_end(args);
	}
void MyLogger::LoggerTrace(const char *format,...){
		va_list args;
		va_start(args, format);
		LoggerOutput("Trace",format, args);
		va_end(args);
	}
void MyLogger::LoggerOutput(char *type, const char* ifmt, va_list arguments )
	{
		printf("[%s]: ",type);
		vprintf(ifmt, arguments);
		printf("\n");
		 
	}
 
