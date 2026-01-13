
#ifndef _FXDCMLIB_LOGGER__H_
#define _FXDCMLIB_LOGGER__H_
//
#include "dcmtk/oflog/logger.h"
#include "dcmtk/oflog/fileap.h"


#include "DcmXTUtilMain.h"


#if 0
extern log4cplus::Logger _FxDcmLibLogger_  ;

#define DCMLIB_LOG_ERROR(msg)	if(DcmXTUtilMain::isEnableAPILog()) LOG4CPLUS_ERROR(_FxDcmLibLogger_,	DcmXTUtilMain::getLogPrefix().c_str()<<msg  );
#define DCMLIB_LOG_WARN(msg)	if(DcmXTUtilMain::isEnableAPILog()) LOG4CPLUS_WARN(_FxDcmLibLogger_,	DcmXTUtilMain::getLogPrefix().c_str()<<msg  );
#define DCMLIB_LOG_INFO(msg)	if(DcmXTUtilMain::isEnableAPILog()) LOG4CPLUS_INFO(_FxDcmLibLogger_,	DcmXTUtilMain::getLogPrefix().c_str()<<msg  );
#define DCMLIB_LOG_DEBUG(msg)	if(DcmXTUtilMain::isEnableAPILog()) LOG4CPLUS_DEBUG(_FxDcmLibLogger_,	DcmXTUtilMain::getLogPrefix().c_str()<<msg  ); 
#define DCMLIB_LOG_TRACE(msg)	if(DcmXTUtilMain::isEnableAPILog()) LOG4CPLUS_TRACE(_FxDcmLibLogger_,	DcmXTUtilMain::getLogPrefix().c_str()<<msg  ); 
#else

#endif

#if 0
#define DCMLIB_LOG_ERROR  DcmXTUtilMain::LoggerError
#define DCMLIB_LOG_WARN   DcmXTUtilMain::LoggerWarn 
#define DCMLIB_LOG_DEBUG  DcmXTUtilMain::LoggerDebug 
#define DCMLIB_LOG_TRACE  DcmXTUtilMain::LoggerTrace 
#else
#define DCMLIB_LOG_ERROR  DcmXTUtilMain::getApiLogger()->LoggerError
#define DCMLIB_LOG_WARN   DcmXTUtilMain::getApiLogger()->LoggerWarn
#define DCMLIB_LOG_DEBUG  DcmXTUtilMain::getApiLogger()->LoggerDebug
#define DCMLIB_LOG_TRACE  DcmXTUtilMain::getApiLogger()->LoggerTrace

#endif


#endif //_FXDCMLIB_LOGGER__H_