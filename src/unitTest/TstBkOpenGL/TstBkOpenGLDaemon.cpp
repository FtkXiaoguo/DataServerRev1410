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
 
#include "PXNetDB.h"
#include "globals.h"
 
//#include "FilenameList.h"
#include "AqCore/TRLogger.h"
#include "TstBkOpenGLDaemon.h"
//#include "RTVMiniDump.h"
 
#include "TstBkOpenGL.h"

std::vector<TestOpenGLBase*>  g_proc_list;
std::vector<iRTVThread*> g_thread_list;
 
//	Global config & logging
TestDBDaemonConfigFile gConfig;
TRLogger gLogger;
LARGE_INTEGER gCountsPerSecond;
 

 
TIDICOMServiceProcessor processor("TstBkOpenGLDaemon");
 

RTVDaemon service(&processor);

 
 
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

int main(int argc, char *argv[])
{
	service.Start(argc, argv);
    return 0;
}



void LogVersionInfo(void);

static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};

static char gExeFolder[2*MAX_PATH+1];
void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	int pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);

	strcpy(gExeFolder,sub_str_temp.c_str());
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
	strcpy(gPxDcmServerHomeBuff,sub_str_temp.c_str());

  }else{
	  gPxDcmServerHomeBuff[0] = 0;
  }
}


extern AqObject dicomServerObject;
extern ApplicationEntity outboundLocalAE;

char g_run_bk_exe_path[2048];
int ServerMain (int argc, char** argv)
{
	getHomeFromModulePath();

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
//	AQNetConfiguration::GetInteractiveReportLocation(reportDir);
//	AQNetConfiguration::GetLogFilesLocation(logDir);

#if 1
	logFile = logDir;
//	logFile += gConfig.m_logFilename1;
//	mergeLog = logDir;
//	mergeLog += gConfig.m_mergeLog1;
//	strcpy(gConfig.m_logFilename, logFile.c_str());
//	strcpy(gConfig.m_mergeLog, mergeLog.c_str());
//	strcpy(gConfig.m_reportPathname, reportDir.c_str());
gLogger.SetLogFile(gConfig.m_logFilename);
#endif
	
	std::string log_file = gPxDcmServerHomeBuff + std::string("log\\TstBkOpenGL.log");
	gLogger.SetLogFile(log_file.c_str());
 

	gLogger.SetLogLevel(gConfig.m_logLevel);

	// set up common library error log
	SetAqLogger(&gLogger);

	//	Set up routing logger (for Aorticare)
	if (strlen(gConfig.m_routingLogFile) > 0)
	{
		gRoutingLogger.SetLogFile(gConfig.m_routingLogFile);
	}
  
	{
		std::string bk_exe_file = gExeFolder + std::string("\\PxOpenGLBK.exe");
		strcpy(g_run_bk_exe_path,bk_exe_file.c_str());
		 
	}
  
	gLogger.LogMessage("---- start thread %d 1.0.0 ---- \n",gConfig.m_testThreadNum );
	gLogger.FlushLog();
 
 

	if(gConfig.m_testThreadNum<1) gConfig.m_testThreadNum  = 1;


 
	 
		TestOpenGLBase *new_proc  = new TestOpenGLBase;
		 
		//
		g_proc_list.push_back(new_proc);
		//
		 
 
	//the first  thread
	g_proc_list[0]->PreProcess();
	g_proc_list[0]->Process();

 

	gLogger.LogMessage("---- all end ---- \n" );
	gLogger.FlushLog();

	char AEConfigFilename[256];
 
	 
 
	//delete all
	{
		int thread_num = g_proc_list.size();
		for(int i=0;i<thread_num;i++){
			if(i>0){
				delete g_thread_list[i];
			}
			 g_proc_list[i]->destroy(); ;
		}
	}

	return 0;
}

int	TIDICOMServiceProcessor::PreProcess(void)
{
	return 0;
}

int TIDICOMServiceProcessor::Process(int argc, char **argv)
{
	m_stop = 0;
	int rcode = ServerMain(argc, argv);
	m_stop = 1;
	return rcode;
}

void TIDICOMServiceProcessor::Stop(void)
{
//	Listener::theListener().RequestTermination(1); // trigger main loop to break

	int thread_num = g_proc_list.size();
	for(int i=0;i<thread_num;i++){
		g_proc_list[i]->RequestTermination(1);
	}
#if 0
	TestDBProcessList::theTestDBProcess().RequestTermination(1);
	
	TestDBProcessUpdate::theTestDBProcess().RequestTermination(1);
#endif

	// while wating main thread to shutdown, keep pump stop pending message to service manager
	int count = 0;
	while(!m_stop && count < 200)
	{
		service.ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
		Sleep(0); // yield to other threads
		Sleep(400);
		count++;
	}
}

#include "compression.h"
//---------------------------------------------------------------------
// T.C. Zhao  2003-02-13
//
void TestDBDaemonConfigFile::LogVersionInfo(void)
{
	char buf[256];

 

	buf[sizeof buf - 1] = '\0';
	gLogger.LogMessage(buf);
}

