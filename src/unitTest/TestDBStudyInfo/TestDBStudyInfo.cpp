/***********************************************************************
 * TestDBDaemon.cpp
 
 *
 *-------------------------------------------------------------------
 */
#define _WIN32_DCOM


#pragma warning (disable: 4503)
#include <io.h>

#include "rtvsocket.h"
#include "rtvPoolAccess.h"
//#include "ArchiveManager.h"
//#include "AQNetConfiguration.h"
 
#include "PxDBSqlite.h"
#include "globals.h"
 
//#include "FilenameList.h"
#include "AqCore/TRLogger.h"
#include "TestDBStudyInfo.h"
//#include "TestDBStudyInfoConfigFile.h"
//#include "RTVMiniDump.h"
 
#include "TestDb.h"

std::vector<TestDBProessBase*>  g_proc_list;
std::vector<iRTVThread*> g_thread_list;
 
//	Global config & logging
//TestDBStudyInfoConfigFile gConfig;
TRLogger gLogger;
LARGE_INTEGER gCountsPerSecond;


int gHASPFeatureID;

TRLogger gRoutingLogger;

/***************************************************************************
 *
 *   main accepts the following options
 *
 *      -install  install the server as a service (manual start)
 *      -auto     install the server as a service (auto start)
 *
 *      -remove   remove the service
 *      -debug    run the server as a console program
 *	    -start	  installs and runs the server as a service
 *		-port n   which port to listen to
 ***************************************************************************/

int ServerMain(int argc, char** argv);
 
int main(int argc, char *argv[])
{
	ServerMain(argc, argv);
    return 0;
}



void LogVersionInfo(void);



//extern AqObject dicomServerObject;
//extern ApplicationEntity outboundLocalAE;

int ServerMain(int argc, char** argv)
{
#if 0
	gConfig.Init();
	gConfig.InitKVPMap();
	gConfig.Load(gConfig.m_configFilename);
	gConfig.ValidateAndFixConfiguration();

	const char* configFile = gConfig.m_configFilename;

#endif



	// Read From the registry
	// Vikram 01/30/02
	std::string reportDir;
	std::string logDir;

	std::string mergeLog;
	std::string logFile;

	// These calls return a standard location even if the registry does not have any setting
	//
//	AQNetConfiguration::GetInteractiveReportLocation(reportDir);
//	AQNetConfiguration::GetLogFilesLocation(logDir);

	logFile = logDir;
#if 0
 	logFile += gConfig.m_logFilename1;
	mergeLog = logDir;
 	mergeLog += gConfig.m_mergeLog1;
	strcpy(gConfig.m_logFilename, logFile.c_str());
	strcpy(gConfig.m_mergeLog, mergeLog.c_str());
	strcpy(gConfig.m_reportPathname, reportDir.c_str());

	gLogger.SetLogFile(gConfig.m_logFilename);
	gLogger.SetLogLevel(gConfig.m_logLevel);
#endif
	// set up common library error log
	SetAqLogger(&gLogger);

 

//	gLogger.LogMessage("---- start thread %d 1.0.0 ---- \n", gConfig.m_testThreadNum);
	gLogger.FlushLog();

	//TestDBProessBase::LANG_DEF g_LangID = TestDBProessBase::LANG_ID_Unknown;
	//TestDBProessBase::LANG_DEF g_LangID = TestDBProessBase::LANG_ID_RUSSIAN;
	//TestDBProessBase::LANG_DEF g_LangID = TestDBProessBase::LANG_ID_CHINESE;
	//TestDBProessBase::LANG_DEF g_LangID = TestDBProessBase::LANG_ID_LATIN1;
	TestDBProessBase::LANG_DEF g_LangID = TestDBProessBase::LANG_ID_JAPANESE;
	if(1){
		TestDBProessBase * new_proc = TestDBProcessAddStudy::createInstace();
		new_proc->m_LangID = g_LangID;
		new_proc->setPatientName("test");
		new_proc->PreProcess();
		new_proc->Process();
		//delete all
		new_proc->destroy();
	}

	{
		TestDBProessBase * new_proc = TestDBProcessSearchStudy::createInstace();
		new_proc->m_LangID = g_LangID;
		new_proc->PreProcess();
		new_proc->Process();
		//delete all
		new_proc->destroy();
	}

	gLogger.LogMessage("---- all end ---- \n" );
	gLogger.FlushLog();




	return 0;
}
  
#include "compression.h"

#if 0
void TestDBStudyInfoConfigFile::LogVersionInfo(void)
{
	char buf[256];

	buf[sizeof buf - 1] = '\0';
	gLogger.LogMessage(buf);
}
#endif
