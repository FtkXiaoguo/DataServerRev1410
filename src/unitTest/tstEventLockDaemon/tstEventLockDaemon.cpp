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
 
//#include "PXNetDB.h"
#include "globals.h"
 
//#include "FilenameList.h"
#include "AqCore/TRLogger.h"
#include "tstEventLockDaemon.h"
//#include "RTVMiniDump.h"
 
#include "TestDB.h"
 
 
std::vector<iRTVThread*> g_thread_list;
 
//	Global config & logging
TestEvtLockDaemonConfigFile gConfig;
TRLogger gLogger;
LARGE_INTEGER gCountsPerSecond;
 

 
TstEvtLockServiceProcessor processor("tstEventLockDaemon");
 

RTVDaemon service(&processor);

 

TRLogger gRoutingLogger;
static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};

void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	int pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
	strcpy(gPxDcmServerHomeBuff,sub_str_temp.c_str());

  }else{
	  gPxDcmServerHomeBuff[0] = 0;
  }
}
 

bool g_runFlag = true;
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

int main(int argc, char *argv[])
{
	getHomeFromModulePath();

	AQNetConfiguration::setHomeFolder(gPxDcmServerHomeBuff) ;
	service.Start(argc, argv);
    return 0;
}



void LogVersionInfo(void);



int ServerMain (int argc, char** argv)
{
	gConfig.Init();
	gConfig.InitKVPMap();
	gConfig.Load(gConfig.m_configFilename);
	gConfig.ValidateAndFixConfiguration();
	
  
	const char* configFile = gConfig.m_configFilename;
 



	// Read From the registry
	// Vikram 01/30/02
	std::string reportDir;
	std::string logDir;

	std::string mergeLog;
	std::string logFile;

	// These calls return a standard location even if the registry does not have any setting
	//
	AQNetConfiguration::GetInteractiveReportLocation(reportDir);
	AQNetConfiguration::GetLogFilesLocation(logDir);

	 

	gLogger.SetLogFile(gConfig.m_logFilename);
	gLogger.SetLogLevel(gConfig.m_logLevel);

	// set up common library error log
	SetAqLogger(&gLogger);

	//	Set up routing logger (for Aorticare)
	if (strlen(gConfig.m_routingLogFile) > 0)
	{
		gRoutingLogger.SetLogFile(gConfig.m_routingLogFile);
	}
  
  
	gLogger.LogMessage("---- start   ---- \n" );
	gLogger.FlushLog();
 
	CTestDB::initTestDB(gConfig.m_GlobalResourceName==1);

	int run_count = 0;

	CTestDB testQueueDB;
	while(g_runFlag){
		gLogger.LogMessage("---- run %d \n",run_count++ );
		gLogger.FlushLog();

		gLogger.LogMessage(">>> waitQueueEvent -- start \n" );
		gLogger.FlushLog();

		if(!testQueueDB.waitQueueEvent()){
			gLogger.LogMessage(">>> waitQueueEvent XXX error\n" );
			gLogger.FlushLog();
		}else{

			gLogger.LogMessage(">>> waitQueueEvent -- end \n" );
			gLogger.FlushLog();
		}
		::Sleep(1000);
		//
		gLogger.LogMessage("**** lockDB -- start \n" );
		gLogger.FlushLog();

		testQueueDB.lockDB() ;
		 
		gLogger.LogMessage("**** lockDB -- OK    \n" );
		gLogger.FlushLog();
		 
		int sleep_nn = 10*1000;
		::Sleep(sleep_nn);
		//
		testQueueDB.unlockDB() ;
		gLogger.LogMessage("**** unlockDB -- after %d Sec\n",sleep_nn/1000);
		gLogger.FlushLog();

	}


	gLogger.LogMessage("---- all end ---- \n" );
	gLogger.FlushLog();

	

	return 0;
}

int	TstEvtLockServiceProcessor::PreProcess(void)
{
	return 0;
}

int TstEvtLockServiceProcessor::Process(int argc, char **argv)
{
	m_stop = 0;
	int rcode = ServerMain(argc, argv);
	m_stop = 1;
	return rcode;
}

void TstEvtLockServiceProcessor::Stop(void)
{
//	Listener::theListener().RequestTermination(1); // trigger main loop to break

	g_runFlag = false;
 
	Sleep(400);
	 
}

