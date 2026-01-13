/***********************************************************************
 * TestDBDaemon.cpp
 
 *
 *-------------------------------------------------------------------
 */
#define _WIN32_DCOM


#pragma warning (disable: 4503)
#include <io.h>

 
#include "PxNetDB.h"
 #include "AqCore/TRLogger.h"
 TRLogger gLogger;
#include "AppComConfiguration.h"

 #include "PxQueue.h"
 
#include "ProcQueue.h"
#include "AddRequest.h"

 #include "PxODBC.h"

int ServerMain (int argc, char** argv);
std::string g_db_file_name;
int main(int argc, char *argv[])
{

 // 	CPxODBC pxODBC;
 // 	pxODBC.doTest();

	if(argc>1){
		g_db_file_name = argv[1];
	}
	ServerMain ( argc, argv);
    return 0;
}

void simpleTest();

void g_initDB()
{
	printf(" test DB InitDatabaseInfo ...\n");
//::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // for database to work in multi-thread mode
	AqCOMThreadInit comInitGuard; // for CoInitializeEx
 
	CPxDcmDB g_db;
 
 
	g_db.setupGlobalDBType(kDBType_SQLite);

 //	CPxDcmDB::InitDBServerName("PxSendQueue.db");

	 
#if 0
	if(!g_db.InitDatabaseInfo())
	{
		gLogger.FlushLog();
		printf("** DB::InitDatabaseInfo failed   \n" );
	 
		return ;
	}else{
		gLogger.FlushLog();
		printf("** DB::InitDatabaseInfo OK   \n" );
	}
#endif

 
	
	{
		g_db.setupLocaleDBType(kDBType_SQLite);
		printf(" test DB \n");
		AqString strSQL;
		strSQL.Format("SELECT MajorVersion FROM dbinfo" );

		int oVal;
		int ret_c = g_db.SQLGetInt(strSQL, oVal);
		printf(" test DB oVal %d, ret_c %d\n",oVal,ret_c);
	}
 
}
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
std::string queue_db_fileName;

bool isQueueWatcher = false;
std::string AE_Str = "TestAE" ;
int g_priority = CPxQueueEntry::PxQueuePriority_Default;//PxQueuePriority_High;
int g_interval = 500;
int ServerMain (int argc, char** argv)
{
	 
 gLogger.SetLogFile(".\\tstQueueDB.log");
 gLogger.SetLogLevel(4);
  
 gLogger.LogMessage("---- start ServerMain ---- \n" );
 gLogger.FlushLog();
 
 SetAqLogger(&gLogger);
 //::CoInitialize( NULL);
 
 if(argc>1){
	  
	 int opt_index = 0;
	 for(int i=1;i<argc;i++){
		 if(argv[i][0] == '-'){
			 
			 switch(argv[i][1]){
				 case 'a':
				 {
					AE_Str = argv[i+1];
				 }
				 break;
				 case 'p':
				 {
					g_priority = atoi(argv[i+1]);
				 }
				 break;
				 case 'i':
				 {
					g_interval = atoi(argv[i+1]);
				 }
				 break;
			}
			opt_index = i+1;
			i++;
		 }
	 }
	 std::string run_mode;
	 if(opt_index < (argc-1)){
		 run_mode = argv[opt_index+1];
	 }
	 if(run_mode == "Watcher"){
		 isQueueWatcher = true;
	 } 
 }

 getHomeFromModulePath();
 AppComConfiguration::setHomeFolder(gPxDcmServerHomeBuff) ;
 std::string dataRootPath = "";
AppComConfiguration::GetDataRootPath(dataRootPath);

queue_db_fileName = dataRootPath + "\\" + SQLITE_DB_FILTE;
 CPxQueueProc::setupQueueDBName(queue_db_fileName);
 std::string queueEntryFolder = dataRootPath + "\\entry\\";
 CPxQueueProc::setupQueueEntryFolder(queueEntryFolder);

 //simpleTest(); return 0;
#if 1	
	g_initDB();


 	CPxWorkQueue queue;
//	CPxResultQueue queue;
//	queue.setupDBName(SQLITE_DB_FILTE);
	queue.initRes();
	
 	queue.selTest();

	//
	CPxResultQueue wk_queue;
//	wk_queue.setupDBName(SQLITE_DB_FILTE);
	wk_queue.initRes();
	wk_queue.selTest();
#if 0

	queue.readQueue(2);

	CPxQueueEntry new_entry;
	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Series;
	new_entry.m_StudyInstanceUID = " ";
	new_entry.m_SeriesInstanceUID = "121.322.222";
	new_entry.m_DestinationAE = "testAE";
	new_entry.m_Priority = 1;
	new_entry.m_Status = 0;
	new_entry.m_CreateTime = time(0);
	new_entry.m_AccessTime = time(0);

	queue.createJobID(new_entry);
	queue.addQueue(new_entry);
	 
	queue.watchQueue();

	queue.recycleQueue(10);
#endif

#else


#if 1
	//AddRequest-1
	CAddRequest AddRequest1;
	AddRequest1.setPriority(CPxQueueEntry::PxQueuePriority_Low);
	AddRequest1.setMyID(1);
 	iRTVThread InaMangerThread1(&AddRequest1);
#if 0
	//AddRequest-2
	CAddRequest AddRequest2;
	AddRequest2.setAE("tesAE1");
	AddRequest2.setPriority(CPxQueueEntry::PxQueuePriority_Default);
	AddRequest2.setLoopInterval(400);
	AddRequest2.setMyID(2);
 	iRTVThread InaMangerThread2(&AddRequest2);

	//AddRequest-3
	CAddRequest AddRequest3;
	AddRequest3.setAE("tesAE3");
	AddRequest3.setPriority(CPxQueueEntry::PxQueuePriority_High);
	AddRequest3.setLoopInterval(500);
 	iRTVThread InaMangerThread3(&AddRequest3);
#endif
	//
	CProcQueue ProcQueue;
	ProcQueue.Process();


#else
 CProcQueue ProcQueue;
 CAddRequest AddRequest;
	
 if(isQueueWatcher){
	ProcQueue.Process();
 }else{
	 AddRequest.setAE(AE_Str);
	 AddRequest.setPriority(g_priority );
	AddRequest.setLoopInterval(g_interval);
	 AddRequest.Process();
 }
#endif
#endif
	return 0;
}
