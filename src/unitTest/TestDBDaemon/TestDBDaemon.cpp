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
#include "AQNetConfiguration.h"
 
#include "PXNetDB.h"
#include "globals.h"
 
//#include "FilenameList.h"
#include "AqCore/TRLogger.h"
#include "TestDBDaemon.h"
//#include "RTVMiniDump.h"
 
#include "TestDb.h"

std::vector<TestDBProessBase*>  g_proc_list;
std::vector<iRTVThread*> g_thread_list;
 
//	Global config & logging
TestDBDaemonConfigFile gConfig;
TRLogger gLogger;
LARGE_INTEGER gCountsPerSecond;
 

 
TIDICOMServiceProcessor processor("TestDBDaemon");
 

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



extern AqObject dicomServerObject;
extern ApplicationEntity outboundLocalAE;

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

	logFile = logDir;
	logFile += gConfig.m_logFilename1;
	mergeLog = logDir;
	mergeLog += gConfig.m_mergeLog1;
	strcpy(gConfig.m_logFilename, logFile.c_str());
	strcpy(gConfig.m_mergeLog, mergeLog.c_str());
	strcpy(gConfig.m_reportPathname, reportDir.c_str());

	gLogger.SetLogFile(gConfig.m_logFilename);
	gLogger.SetLogLevel(gConfig.m_logLevel);

	// set up common library error log
	SetAqLogger(&gLogger);

	//	Set up routing logger (for Aorticare)
	if (strlen(gConfig.m_routingLogFile) > 0)
	{
		gRoutingLogger.SetLogFile(gConfig.m_routingLogFile);
	}
  
  
	gLogger.LogMessage("---- start thread %d 1.0.0 ---- \n",gConfig.m_testThreadNum );
	gLogger.FlushLog();
 
#if 0
 //::CoInitialize( NULL);
 
::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // for database to work in multi-thread mode

	
		{
//K.Ko  2010/02/22  #617
			TCHAR  MyComputerName[256];
			unsigned long nSize = sizeof(MyComputerName)-1 ;
			GetComputerName(MyComputerName, &nSize);
			std::string dbServerNameTemp = MyComputerName;
#if 1 // #813: SQL Server 2000‚Æ2005‚É—¼‘Î‰ž‚·‚é
			dbServerNameTemp = dbServerNameTemp + DBCore::GetSQLServerDBNameExt();
#else 
			dbServerNameTemp = dbServerNameTemp + "\\SQLEXPRESS";
#endif
			AQNetDB::InitDBServerName(dbServerNameTemp.c_str());
		} 
	
#endif

	if(gConfig.m_testThreadNum<1) gConfig.m_testThreadNum  = 1;


 
	for(int th_i = 0;th_i<gConfig.m_testThreadNum ;th_i++){
		TestDBProessBase *new_proc;
		switch(gConfig.m_testThreadType[th_i]){
			case 0:
			new_proc = TestDBProcessList::createInstace();
			break;
			//
			case 1:
			new_proc = TestDBProcessUpdate::createInstace();
			break;
			//
			case 2:
			TestDBProcessBigDb *new_proc_temp = TestDBProcessBigDb::createInstace();
			new_proc_temp->setBigDbLen(gConfig.m_testBigDbLen);
			new_proc = new_proc_temp;

			break;
		}
	 
		//
		g_proc_list.push_back(new_proc);
		//
		if(strlen(gConfig.m_patientName[th_i])>0){
			new_proc->setPatientName(gConfig.m_patientName[th_i]);
		}

		if(th_i >0){
			iRTVThread *new_thread = new iRTVThread(g_proc_list[th_i]);
			g_thread_list.push_back(new_thread);  
		}
	}
 
	//the first  thread
	g_proc_list[0]->PreProcess();
	g_proc_list[0]->Process();

#if 0
	TestDBProcessUpdate &db_proc_update = TestDBProcessUpdate::theTestDBProcess();
	if(strlen(gConfig.m_patientName1)>1){
 		db_proc_update.setPatientName(gConfig.m_patientName1);
	}
	iRTVThread theDBProcThread(&db_proc_update);

 
	TestDBProcessList &db_proc_list  = TestDBProcessList::theTestDBProcess();
	if(strlen(gConfig.m_patientName2)>1){
 		db_proc_list.setPatientName(gConfig.m_patientName2);
	}

	db_proc_list.PreProcess();
	db_proc_list.Process();
#endif


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

