/***********************************************************************
 * ThreadedLicenseChecker.h
 *---------------------------------------------------------------------
 // #963 2010/12/16 K.Ko for New Hasp êVãKçÏê¨Å@
 // AQNetLicenseManagerÅ@Ç©ÇÁï™ó£
 *-------------------------------------------------------------------
 */
#ifndef THREAD_LICENSE_CHECKER_H
#define THREAD_LICENSE_CHECKER_H

#define FALSE 0
#define TRUE 1
#include <string>
#include <map>
#include "rtvthread.h"
 


//---------------------------------------------------------------------
//
class ThreadedLicenseChecker : public iRTVThreadProcess
{
public:
	static ThreadedLicenseChecker& theThreadedLicenseChecker();
	virtual ~ThreadedLicenseChecker(void) {}

	int Process();

private:
	ThreadedLicenseChecker();
};

#endif THREAD_LICENSE_CHECKER_H

