/***********************************************************************
 * PXDcmSServer.cpp
 *---------------------------------------------------------------------
 *    Copyright, PreXion 2011, All rights reserved.
 *
 *		 PX DICOM  SERVER
 *
 *
 *
 *-------------------------------------------------------------------
 */

#pragma warning (disable: 4503)
#include <io.h>

#include "rtvsocket.h"
#include "rtvPoolAccess.h"

#include "AppComConfiguration.h"
#include "DiskSpaceManager.h"
#include "PxQueueWatchProc.h"

#include "RTVDiCOMStore.h"
#include "PxNetDB.h"
#include "globals.h"

#include "AqCore/TRLogger.h"
#include "PXDcmJobProc.h"

#include "PxDicomutil.h"
#include "StoreSCUSeriesDirMonitor.h"
#include "ResultQueueMonitor.h"

 

 
//#9 2012/03/19 K.Ko for New Hasp
#include "PXLicenseManagerIf.h"
#include "ThreadedLicenseChecker.h"
 
#include "AutoDelStudyProc.h"//#52

//	Global config & logging
JobProcServerConfigFile gConfig;
TRLogger gLogger;
TRLogger gDcmApiLogger; // #4 2012/02/21 K.Ko

LARGE_INTEGER gCountsPerSecond;
 
 
PXDcmJobProcServiceProcessor processor("PXDcmJobProc");
 

RTVDaemon service(&processor);

//const char* gEmailConfigFile = "C:/AQNetConfig/emailConfig.cfg";
//const char* gEmailRecipientsFile = "C:/AQNetConfig/highWatermarkAlertEmails.cfg";
//SMTPSendMessage gEmailer;
int gHASPFeatureID;
int gHASPFeatureID2;//#47

TRLogger gRoutingLogger;

bool dicom_server_checkLicense();

bool checkExportJPEG(bool dispLog, bool *cancelFlag = 0);//#48
/*
*  home folder 
*  c:\PxSDcmServerHome\    
*/
static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};
 
////////////////////////////
static PxQueueWatchProc *g_QueueWatchProcMain = 0;
static PxQueueWatchProc *g_QueueWatchProcSub = 0;
static iRTVThread *g_QueueWatchProcSubThread = 0;

static  CAutoDelStudyProc *g_AutoDelStudyProc = 0;//#52
static	iRTVThread *g_AutoDelStudyProcThread = 0;//#52

static iRTVThread *g_QueueWatchMainThread = 0; //#82 2014/09/29 K.Ko change main thread
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
//#undef DBG_APP_MODE

void getHomeFromModulePath();
void setupHomePath(const std::string &folder);

void doSelfTest();
int main(int argc, char *argv[])
{
	 

	getHomeFromModulePath();
	if(argc>1){
		std::string arg_str_temp = argv[argc-1];//最後の引数をConfigファイルに
		int pos = arg_str_temp.find("-");
		if(pos>=0){
			// -debug, -install ...
		}else{
			setupHomePath(arg_str_temp);
		}
	}
	AppComConfiguration::setHomeFolder(gPxDcmServerHomeBuff) ;
 	service.Start(argc, argv);
 
    return 0;
}
 
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

void setupHomePath(const std::string &folder)
{
	if(folder.size()>1){
		std::string sub_str_temp = folder;
		int pos = sub_str_temp.find_last_of("/\\");
		int all_size = folder.size();
		if(pos < (all_size-1)){
			sub_str_temp += "/";
		}
		strcpy(gPxDcmServerHomeBuff,sub_str_temp.c_str());
	}
}

//-----------------------------------------------------------------------------
//
int InitEmail(void)
{
#if 0
	//	Load email config file
	gEmailer.LoadConfig(gEmailConfigFile);
	if (gEmailer.GetSMTPServer().size() <= 0)
	{
		return -1;
	}


	//	FROM
	std::string senderAddress = std::string("AQNetServer@") + std::string(TRPlatform::GetIPAddressString());
	gEmailer.SetFrom(senderAddress);

	//	TO
	FilenameList fnList;
	int status = fnList.Load(gEmailRecipientsFile, 1 /* CLEAR_LIST */, 0 /* NO_CHECK_EXISTS */);
	if (status != kFNListFileOK)
	{
		return -1;
	}

	std::string recipients;
	std::vector<std::string> recipientV = fnList.GetList();
	for(int i = 0; i < recipientV.size(); i++)
	{
		recipients += recipientV[i];
		
		if ((i+1) < recipientV.size())
		{
			recipients += ",";
		}
	}
	gEmailer.SetTo(recipients);
	
#endif
	return 0;
}


void LogVersionInfo(void);

#if 0
bool CallBackMiniDump(const struct _EXCEPTION_POINTERS *pExceptionInfo, const char* dumpFile)
{
	if(dumpFile && dumpFile[0])
	{
		char msg[1024];
		snprintf(msg, 1024, "**Global exception trigger a mini dump:%s\n", dumpFile);
		gLogger.LogMessage(msg);
		gConfig.CloseLog();
	}
	else
	{
		gLogger.LogMessage("**Global exception happend ***");
	}
	return true;
}
#endif

extern AqObject dicomServerObject;
extern ApplicationEntity outboundLocalAE;

bool g_terminationFlag = false;
int ServerMain (int argc, char** argv)
{
	gConfig.Init();
	gConfig.InitKVPMap();
	gConfig.Load(gConfig.m_configFilename);
	gConfig.ValidateAndFixConfiguration();
	

	CPxQueueProc::Global_init();

	// doSelfTest();

	// for release version do mini dump when crash
#if !defined(_DEBUG)

#if 0
	char dumpDir[MAX_PATH];
	sprintf(dumpDir, "%s/AQNetMiniDump", gConfig.m_raidDevices[0].GetPathToDevice().c_str());

	RTVMiniDumpInit(kDICOMServerVersion,
					  kDICOMServerRevision,
					  kDICOMServerTweaks,
					  kDICOMServerBuild,
					  dumpDir, &gLogger);
#endif 

#endif
	//RTVMiniDump::Init("C:/AQNetLog", 0, 0, 0, CallBackMiniDump);
	const char* configFile = gConfig.m_configFilename;
	QueryPerformanceFrequency(&gCountsPerSecond);



	RTVDiskSpaceManager::SetEarlyHighwatermarkEmail(gConfig.m_earlyHighwatermarkEmail);




	// Read From the registry
	// Vikram 01/30/02
	std::string reportDir;
	std::string logDir;

	std::string mergeLog;
	std::string logFile;

	// These calls return a standard location even if the registry does not have any setting
	//
	AppComConfiguration::GetInteractiveReportLocation(reportDir);
	AppComConfiguration::GetLogFilesLocation(logDir);

#if 0
	//#7 2012/03/08 K.Ko
 
	logFile = logDir;
	logFile += gConfig.m_logFilename1;
	mergeLog = logDir;
	mergeLog += gConfig.m_mergeLog1;
	strcpy(gConfig.m_logFilename, logFile.c_str());
	strcpy(gConfig.m_mergeLog, mergeLog.c_str());
	strcpy(gConfig.m_reportPathname, reportDir.c_str());

#endif


	gLogger.SetLogFile(gConfig.m_logFilename);
	gLogger.SetLogLevel(gConfig.m_logLevel);
	//
	// #4 2012/02/21 K.Ko
	gDcmApiLogger.SetLogFile(gConfig.m_mergeLog);
	gDcmApiLogger.SetLogLevel(gConfig.m_dicomLogLevel);

//	gLogger.SetDefaultLogLevel(0xffff0000);//change old logfile to with timestamp

	// set up common library error log
	SetAqLogger(&gLogger);

	CPxDB::SetLogger(&gLogger);
	 
	//	Set up routing logger (for Aorticare)
	if (strlen(gConfig.m_routingLogFile) > 0)
	{
		gRoutingLogger.SetLogFile(gConfig.m_routingLogFile);
	}

	///////////////////////
	// start logger
	
	gConfig.LogVersionInfo();

	gConfig.Save(stdout);


	RTVDiCOMStore::c_maxStoreThread = gConfig.m_maxStoreThreads;
	RTVDiCOMStore::c_maxCompressedThread = gConfig.m_maxCompressedThreads;
 
 
#if 0
	//	Just in case...
	if (strcmp(gConfig.m_bypassHASP,":D9)p<") == 0)
	{
		LicenseManager::theLicenseManager().BypassHASP(1);
	}
#endif

	bool isDICOMGateWay		= AppComConfiguration::GetGatewayFlag()==1;
	bool isJPEGGateWay = AppComConfiguration::GetJpegGatewayFlag() == 1;
	//#93　2017/02/13 N.Furutsuki
	//#94
	bool isLocalBackup = AppComConfiguration::GetLocalBackupFlag() == 1;

	

	gLogger.LogMessage("==== GateWay [%d] ==========\n", isDICOMGateWay ? 1 : 0);
	gLogger.LogMessage("==== JPEG GateWay [%d] ==========\n", isJPEGGateWay ? 1 : 0);
	gLogger.LogMessage("==== LocalBackup [%d] ==========\n", isLocalBackup ? 1 : 0);
	
	//////////////////////////////////
	if (isLocalBackup){//#94　2017/02/13 N.Furutsuki
		gConfig.m_EnableAutoDeleteStudy = 0;
		gLogger.LogMessage("==== set EnableAutoDeleteStudy to 0 for LocalBackup ==========\n");

	}


	if(isDICOMGateWay || isJPEGGateWay){
		gLogger.LogMessage("==========================================\n  EnableAutoDeleteStudy[%d] \n  AutoDeleteStudyInterval[%d] \n  AutoDeleteKeepDays[%d] \n",
				gConfig.m_EnableAutoDeleteStudy,
				gConfig.m_AutoDeleteStudyInterval,
				gConfig.m_AutoDeleteKeepDays);
	}

	if(isJPEGGateWay){
		gLogger.LogMessage("==========================================\n  For [%s] \n  ExportJPEGFolder[%s] \n  ExportJPEGFolderSub[%s] \n  OutputJpegQuality[%d] \n  CTDataThumbnailIndex[%d] \n checkCTImageSize[%d] \n",
			(gConfig.m_ActionGate == 1) ? "ActionGate" : "DELTA View",
			gConfig.m_ExportJPEG_FolderName,
			gConfig.m_ExportJPEG_FolderSubName,//#98 2017/07/10 N.Furutsuki
			gConfig.m_OutputJpegQuality,
			gConfig.m_CTDataThumbnailIndex, //#48
			gConfig.m_checkCTImageSize);//#103 2019/08/23 N.Furutsuki
		if(gConfig.m_ActionGate==1){
			gLogger.LogMessage("ActionGate YWExe [%s] \n",gConfig.m_ActionGateYTWExePath);
		}
	}
	 
	gLogger.FlushLog();
	 

	checkExportJPEG(true, &g_terminationFlag);//#48
	

 
//#9 2012/03/19 K.Ko for New Hasp
gHASPFeatureID = ePREXION_PXDICOMSERVER3;		//#9 2012/03/19 K.Ko for New Hasp
gHASPFeatureID2 = ePREXION_FINECUBE_IMAGESERVER;	//#47
dicom_server_checkLicense(); // K.Ko 2010/05/21

 

	//	Is it AqNET Sonic?
	bool ignoreBypass = true;
//	gAqNETSonic = LicenseManager::FeatureExists(LicenseManager::theLicenseManager().CheckFeature(eAQUARIUS_SONIC, ignoreBypass));



	//	Start the license checker thread
//	ThreadedLicenseChecker& theThreadedLicenseChecker = ThreadedLicenseChecker::theThreadedLicenseChecker();
//	iRTVThread lmThread(&theThreadedLicenseChecker);
	
	RTVDiCOMStore::c_maxStoreThread = gConfig.m_maxStoreThreads;
	RTVDiCOMStore::c_maxCompressedThread = gConfig.m_maxCompressedThreads;

 
	AqCOMThreadInit comInitGuard;
	
	//set up server name, currently comment to use default setup from registry
	//CPxDcmDB::InitDBServerName(gConfig.m_dbServerName);

	
  
		{
//K.Ko  2010/02/22  #617
			TCHAR  MyComputerName[256];
			unsigned long nSize = sizeof(MyComputerName)-1 ;
			GetComputerName(MyComputerName, &nSize);
			std::string dbServerNameTemp = MyComputerName;
			dbServerNameTemp = dbServerNameTemp + "\\SQLEXPRESS";
			CPxDcmDB::InitDBServerName(dbServerNameTemp.c_str());
		} 

		if(gConfig.m_writeCache != 0){
			gLogger.LogMessage(kWarning,"WARNING: *** m_writeCache !=0 , reset the writeCahe to 0  *** !\n");
			gLogger.FlushLog();

			gConfig.m_writeCache  = 0;
		}

		if(gConfig.m_threadWrite != 0){
			gLogger.LogMessage(kWarning,"WARNING: *** m_threadWrite !=0 , reset the threadWrite to 0  *** !\n");
			gLogger.FlushLog();

			gConfig.m_threadWrite  = 0;
		}
 
	///////////////////// 
	//moved to here #53
	struct tm when;
	time_t now;

	time( &now );
	when = *localtime( &now );
	AppComConfiguration::GetAutoWakeupHour(when.tm_hour);
	gConfig.m_lastCleaned = mktime( &when );

	gLogger.LogMessage(kDebug,">> start threads \n");
	gLogger.FlushLog();

	std::string queue_db_name = std::string(gConfig.m_DB_FolderName) + "\\" + SQLITE_DB_FILTE;
	CPxQueueProc::setupQueueDBName(queue_db_name);
	std::string queue_entry_folder = std::string(gConfig.m_DB_FolderName) + "\\entry\\";
	CPxQueueProc::setupQueueEntryFolder(queue_entry_folder);
	/////////////////////

	
	g_QueueWatchProcMain = new PxQueueWatchProc(isJPEGGateWay);
	g_QueueWatchProcMain->setupSQLiteDBDir(gConfig.m_DB_FolderName);
	///
	if(!g_QueueWatchProcMain->DICOM_Initialization()){
		gLogger.LogMessage("ERROR:[C%08d] DICOM_Initialization error  !\n",DicomJobProcError_DicomLibInitError);
		gLogger.FlushLog();
		return 1;
	}

	////////////////
	PxQueueWatchProc::setupQueueLog();

	if(gConfig.m_useSubThreadForLowPriority !=0){
		//setup main thread WatchPriority
		g_QueueWatchProcMain->setWatchPriority(CPxQueueEntry::PxQueuePriority_Default); // >= PxQueuePriority_Default

		g_QueueWatchProcSub = new PxQueueWatchProc(isJPEGGateWay);
		g_QueueWatchProcSub->setupSQLiteDBDir(gConfig.m_DB_FolderName);

		if(!g_QueueWatchProcSub->DICOM_Initialization()){
			gLogger.LogMessage("ERROR:[C%08d] DICOM_Initialization error  !\n",DicomJobProcError_DicomLibInitError);
			gLogger.FlushLog();
			return 1;
		}

		//setup sub thread WatchPriority
		g_QueueWatchProcSub->setWatchPriority(CPxQueueEntry::PxQueuePriority_Low);  

		g_QueueWatchProcSubThread = new iRTVThread(g_QueueWatchProcSub); //start sub thread 
	}
	//////////////////

	if(isDICOMGateWay || isJPEGGateWay){
		if(gConfig.m_EnableAutoDeleteStudy){
			gLogger.LogMessage(" === Start  AutoDelStudyProc === \n");
			gLogger.FlushLog();
			g_AutoDelStudyProc = new CAutoDelStudyProc();//#52
			g_AutoDelStudyProcThread = new iRTVThread(g_AutoDelStudyProc); //#52
		}
	}

	
 

//	char AEConfigFilename[256];

 
#if 0
try
{
#endif

//	strncpy(AEConfigFilename, gConfig.m_AEConfigPathname, 256);
//	STRNCAT_S(AEConfigFilename, gConfig.m_AEConfigFilename, 256);
	
	//ArchiveManager am(gConfig.m_raidDrive, &AEmon);
	
	// start working threads
	RTVDiskSpaceManager::StartupAll();
	//
	 
	
	/*
	* for send dicom 
	*/

	CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor().setIntervalTime(gConfig.m_seriesCompleteTimeout*1000);
	iRTVThread CstoreSCUSeriesDirThread(&CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor());

	/*
	*  delete the resulte queue
	*/
	CResultQueueMonitor::theResultQueueMonitor().setupSQLiteDBDir(gConfig.m_DB_FolderName);
	CResultQueueMonitor::theResultQueueMonitor().setIntervalTime(gConfig.m_resultQueueMonitorInterval*1000);
	iRTVThread ResultQueueMonitorThread(&CResultQueueMonitor::theResultQueueMonitor());

	///
	


	// The define is replaced with gConfig.m_writeCache flag to enable 
	// controlling the cache writing using the external cache writer.
	// Murali - 2006.01.05
 
	
//	if (gConfig.m_doCompressedCache)
//		RTVOneShotThreadManager::theManager().SetMaxRunThreads(gConfig.m_maxCompressThreads, "TRCompressedCacheWriter");

#if 0 //moved #53
	struct tm when;
	time_t now;

	time( &now );
	when = *localtime( &now );
	AppComConfiguration::GetAutoWakeupHour(when.tm_hour);
	gConfig.m_lastCleaned = mktime( &when );

	gLogger.LogMessage(kDebug,"run  QueueWatchProc Main\n");
	gLogger.FlushLog();

	std::string queue_db_name = std::string(gConfig.m_DB_FolderName) + "\\" + SQLITE_DB_FILTE;
	CPxQueueProc::setupQueueDBName(queue_db_name);
	std::string queue_entry_folder = std::string(gConfig.m_DB_FolderName) + "\\entry\\";
	CPxQueueProc::setupQueueEntryFolder(queue_entry_folder);
#endif
	
	//////////////////
#if 0
 	g_QueueWatchProcMain->PreProcess();
   	g_QueueWatchProcMain->Process();
#else
	g_QueueWatchMainThread = new iRTVThread(g_QueueWatchProcMain);  

	//#82 2014/09/29 K.Ko change main thread
	while(!g_terminationFlag){
		int imSec = 5000;
		
		processor.waitEvent( imSec);
	}
#endif
	
	gLogger.LogMessage(kDebug,"ServerMain: stop all  \n");
	gLogger.FlushLog();

	// shutdown procedures
 
//	RTVOneShotThreadManager::theManager().RequestTermination(1);
// 	RTVInactiveManager::theManager().RequestTermination(1);
 ///////
	CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor().RequestTermination(1);
	CResultQueueMonitor::theResultQueueMonitor().RequestTermination(1);

	if(g_QueueWatchProcMain){//#82 2014/09/29 K.Ko change main thread
		g_QueueWatchProcMain->RequestTermination(1); //  
	}


	if(g_QueueWatchProcSub){
		g_QueueWatchProcSub->RequestTermination(1); //  
	}

	if(g_AutoDelStudyProc){//#52
		g_AutoDelStudyProc->RequestTermination(1); //  
	}

	gLogger.LogMessage(kDebug,"wait all threads  \n");
	gLogger.FlushLog();

	// make sure all stopped
//	RTVOneShotThreadManager::theManager().ForceStop(18000);
//  RTVInactiveManager::theManager().ForceStop(18000);
 ///////
	CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor().ForceStop(18000);

	gLogger.LogMessage(kDebug,"wait CResultQueueMonitor threads  \n");
	gLogger.FlushLog();

	CResultQueueMonitor::theResultQueueMonitor().ForceStop(18000);
 
		
	gLogger.LogMessage(kDebug,"wait g_QueueWatchMainThread threads  \n");
	gLogger.FlushLog();

	if(g_QueueWatchProcMain){//#82 2014/09/29 K.Ko change main thread
		
		g_QueueWatchProcMain->ForceStop(18000); //  
	}


	gLogger.LogMessage(kDebug,"wait g_QueueWatchProcSub threads  \n");
	gLogger.FlushLog();

	if(g_QueueWatchProcSub){
		
		g_QueueWatchProcSub->ForceStop(18000); //  
	}

	if(g_AutoDelStudyProc){//#52
		g_AutoDelStudyProc->ForceStop(18000); //  
	}
	 

	gLogger.LogMessage(kDebug,"wait threads -end  \n");
	gLogger.FlushLog();

	PxQueueWatchProc::DICOM_Release(); //MC_Library_Release();
	RTVDiskSpaceManager::ShutdownAll();

	//	If we ever get here, we should clean up
	//Sleep(2000);

	
	gLogger.LogMessage(kDebug,"all -end  \n");
	gLogger.FlushLog();


	gLogger.CloseLog();

	if (strlen(gConfig.m_routingLogFile) > 0)
	{
		gRoutingLogger.CloseLog();
	}



//#if !defined(_DEBUG)
#if 0
}
catch (...)
{
	gLogger.LogMessage("**Global exception in main\n");
	gLogger.CloseLog();
	return -1;
}
#endif
	return 0;
}
void   PXDcmJobProcServiceProcessor::waitEvent(int imSec) //#82 2014/09/29 K.Ko change main thread
{
	DWORD dwWaitStatus = WaitForSingleObject(m_wakeupEvent, imSec);
}
int	PXDcmJobProcServiceProcessor::PreProcess(void)
{
	return 0;
}

int PXDcmJobProcServiceProcessor::Process(int argc, char **argv)
{
	m_wakeupEvent = CreateEvent(0, TRUE, FALSE, 0);//#82 2014/09/29 K.Ko change main thread
	m_stop = 0;
	int rcode = ServerMain(argc, argv);
	m_stop = 1;

	return rcode;
}

void PXDcmJobProcServiceProcessor::Stop(void)
{
	g_terminationFlag = true;
	
#if 0
	if(g_QueueWatchProcMain){
		g_QueueWatchProcMain->RequestTermination(1);
	}
#else
	//#82 2014/09/29 K.Ko change main thread
	SetEvent(m_wakeupEvent);
#endif
	// while wating main thread to shutdown, keep pump stop pending message to service manager
	int count = 0;
#if 0
//	while(!m_stop && count < 200)
//	while(!m_stop && count < 2)//#30 2012/07/03
	{

//		service.ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
		Sleep(0); // yield to other threads
		Sleep(400);
		count++;
	}
#endif

}

#include "compression.h"
//---------------------------------------------------------------------
// 
//
void JobProcServerConfigFile::LogVersionInfo(void)
{
	char buf[256];

 
	 

	std::string awareVersion = Compression::GetJ2KToolkitVersionString();
#if 0
 	std::string haspVersion  = LicenseManager::theLicenseManager().GetHaspDriverVersionString();
#else
	std::string haspVersion = "Unknown";
#endif

 
	snprintf(buf, sizeof buf, 
		"------------------ PXDcmJobProc Starts ------------\n  PXDcmJobProc V%d.%d.%d.%d (built on %s) [DcmLib: %s] [Hasp API: %s] Started\n",
	//	sonicStr.c_str(),
	//	sonicStr.c_str(),
		kDICOMServerVersion,
		kDICOMServerRevision,
		kDICOMServerTweaks,
		kDICOMServerBuild,
		__DATE__,
		TRDICOMUtil::GetVersionString(),
	 
		haspVersion.c_str());
 

	buf[sizeof buf - 1] = '\0';
	gLogger.LogMessage(buf);
	gLogger.FlushLog();
}

#include "AppComUtil.h"
bool dicom_server_checkLicense() // K.Ko 2010/05/21
{


	int isJpegGateway = AppComConfiguration::GetJpegGatewayFlag();//#47
 
	if (0 != AppComConfiguration::GetLocalBackupFlag()){
		isJpegGateway = 1;//#94
	}

//#9 2012/03/19 K.Ko for New Hasp
	bool ret_val = false;
	//notice : use the gHASPFeatureID

	int daysToExpire = 0;
//	int dicomServerHaspStatus = LicenseManager::theLicenseManager().CheckFeature(gHASPFeatureID,daysToExpire);
	int dicomServerHaspStatus = PXLicenseManager::CheckLicense(gHASPFeatureID, daysToExpire);

//	dicomServerHaspStatus = 3;//k.ko Debug
	//	std::string hasp_status_str = LicenseManager::ConvertErrorCodeToString(dicomServerHaspStatus); //error ???
	std::string hasp_status_str = AppComUtil::ConvertErrorCodeToString(dicomServerHaspStatus);
	gLogger.LogMessage("HASP: License Status = %s\n", hasp_status_str.c_str());
	if (LicenseManager::IsValid(dicomServerHaspStatus))
	{
		ret_val = true;//OK
	
	}else{
		bool checkHifDevice = true;
		if(isJpegGateway !=0){ //#47
			int GPUHaspStatus = PXLicenseManager::CheckLicense(gHASPFeatureID2, daysToExpire);
			std::string hasp_status_str = AppComUtil::ConvertErrorCodeToString(GPUHaspStatus);
			gLogger.LogMessage("HASP: License Status (From ImageServer) = %s\n", hasp_status_str.c_str());
			if (LicenseManager::IsValid(GPUHaspStatus)){
				ret_val = true;//OK
				checkHifDevice = false;
			}


		}
		if(checkHifDevice){//#47
			char chkHifMsg[256];
			chkHifMsg[0] = 0;
			 
			if(AppComUtil::ChkHifDrv(chkHifMsg,256) == ChkHifDrv_OK ){ //#33 HIFチェックの追加 2012/09/06 K.Ko 2012/09/28 修正漏れ
				ret_val = true;
				gLogger.LogMessage(" ### Found HIF License %s ### \n",chkHifMsg);
			}else{
				ret_val = false;
				gLogger.LogMessage("ERROR:[C%08d] No valid license - either HASP key is not attached, or license file is expired\n",DicomJobProcError_LicenceInvalid);
				 
			}
		}
	
	}


	//	Add the feature to the internal map for periodic checking by the license checker thread
//	int daysToExpire = 0;
//	LicenseManager::theLicenseManager().CheckLicense(gHASPFeatureID, daysToExpire);

	return ret_val;
 
}

bool remoteLogon()//#55
{
	///////////////
	bool ret_b = false;
	if(strlen(gConfig.m_LogonComputerName)>1){
		ret_b = true;
		 
		gLogger.LogMessage("remote logon %s user:%s pw:%s \n",
			gConfig.m_LogonComputerName,
			gConfig.m_LogonUser,
			"xxxxxx");//gConfig.m_LogonPassword);
		gLogger.FlushLog();

		NETRESOURCE netResource;
		ZeroMemory(&netResource, sizeof(NETRESOURCE));
		netResource.lpRemoteName = (LPSTR)gConfig.m_LogonComputerName;

		DWORD dwRet = NO_ERROR;
	
		for( int n = 0; n <gConfig.m_LogonRetry; n++ )
		{
			if(g_terminationFlag) return false;//サービス終了
			
			dwRet = WNetAddConnection2( &netResource, gConfig.m_LogonUser , gConfig.m_LogonPassword, 0 );
			
			if ( dwRet == NO_ERROR ){
				break;
			}
			bool break_flag = false;
			char *error_code = "Unknown";
			switch(dwRet){
				case ERROR_ACCESS_DENIED: error_code = "ERROR_ACCESS_DENIED"; break;
				case ERROR_ALREADY_ASSIGNED: error_code = "ERROR_ALREADY_ASSIGNED"; break;
				case ERROR_BAD_DEV_TYPE: error_code = "ERROR_BAD_DEV_TYPE"; break;
				case ERROR_BAD_DEVICE: error_code = "ERROR_BAD_DEVICE"; break;
				case ERROR_BAD_NET_NAME: error_code = "ERROR_BAD_NET_NAME"; break;
				case ERROR_BAD_PROFILE: error_code = "ERROR_BAD_PROFILE"; break;
				case ERROR_BAD_PROVIDER: error_code = "ERROR_BAD_PROVIDER"; break;
				case ERROR_BUSY: error_code = "ERROR_BUSY"; break;
				case ERROR_CANCELLED: error_code = "ERROR_CANCELLED"; break;
				case ERROR_CANNOT_OPEN_PROFILE: error_code = "ERROR_CANNOT_OPEN_PROFILE"; break;
				case ERROR_DEVICE_ALREADY_REMEMBERED: error_code = "ERROR_DEVICE_ALREADY_REMEMBERED"; break;
				case ERROR_EXTENDED_ERROR: error_code = "ERROR_EXTENDED_ERROR"; break;
				case ERROR_INVALID_PASSWORD: error_code = "ERROR_INVALID_PASSWORD"; break;
				case ERROR_NO_NET_OR_BAD_PATH: error_code = "ERROR_NO_NET_OR_BAD_PATH"; break;
				case ERROR_NO_NETWORK: error_code = "ERROR_NO_NETWORK"; break;
				case ERROR_SESSION_CREDENTIAL_CONFLICT: 
					error_code = "ERROR_SESSION_CREDENTIAL_CONFLICT"; 
					break_flag = true;
					break;
			}
			
			gLogger.LogMessage("-%d:%s-",dwRet,error_code);
			gLogger.FlushLog();

			if(break_flag){
				break;
			}

			::Sleep(300);
		}
		gLogger.LogMessage("\n");
		 

		if(dwRet == NO_ERROR){
			gLogger.LogMessage("remote logon  OK \n");
		}else{
			gLogger.LogMessage("remote logon  ERROR \n");
		}
		gLogger.FlushLog();
	}
	return ret_b;
}

//#48
bool checkExportJPEG(bool dispLog, bool *cancelFlag )
{
	
	/////////////
	bool isJPEGGateWay  = AppComConfiguration::GetJpegGatewayFlag();

	if(!isJPEGGateWay){
		return true;
	}

	if(g_terminationFlag) return false ;

//	if(strlen(gConfig.m_ExportJPEG_FolderName)<2){
	//#98 2017/07/10 N.Furutsuki
	if (strlen(gConfig.m_ExportJPEG_FolderName)<2 && strlen(gConfig.m_ExportJPEG_FolderSubName)<2){
		gLogger.LogMessage("ERROR: ExportJPEG Folder is NULL  \n");
		gLogger.FlushLog();
		return false;
	}

	int retry_nn = 0;
	bool access_success = false;
	//#98 2017/07/10 N.Furutsuki
	const char *check_folder_name = gConfig.m_ExportJPEG_FolderName;
	if (strlen(gConfig.m_ExportJPEG_FolderSubName) > 1){
		check_folder_name = gConfig.m_ExportJPEG_FolderSubName;
	}
//	for(int run_i=0;run_i<2;run_i++){
	for(int run_i=0;run_i<gConfig.m_checkShardeFolderRetry; run_i++ ){
		if (cancelFlag){ //#82 2017/07/10 N.Furutsuki
			if (*cancelFlag){
				gLogger.LogMessage("checkExportJPEG is canceled  \n");
				gLogger.FlushLog();
				return false;
			}
		}
		retry_nn = run_i;
		
//		if (TRPlatform::access(gConfig.m_ExportJPEG_FolderName,TRPlatform::W_OK) !=0)
		if (TRPlatform::access(check_folder_name, TRPlatform::W_OK) != 0)
		{
			bool sleep_flag = true;
			if(run_i==0){
			//一回目の失敗でLOGON情報を使う。
				if(remoteLogon()){
				//remoteLogon使用時,Sleepしない
				 sleep_flag = false;
				};
			}else{
			//二回目の失敗
		//		gLogger.LogMessage("ERROR: Can not access ExportJPEG Folder [%s] \n",gConfig.m_ExportJPEG_FolderName);
		//		gLogger.FlushLog();
		//		return false ;
			}
			if(sleep_flag){
				int time_base = 200;//mSec
				int sleep_nn = 1000/time_base;//Sec
				for(int sleep_i=0;sleep_i<sleep_nn;sleep_i++){
					if(g_terminationFlag) false ;//サービス終了
					::Sleep(time_base);
				}
			}
		}else{
			access_success  = true;
			retry_nn = run_i;
			break;
		}
	}
	 
	if(access_success){
		if(dispLog){
			gLogger.LogMessage("ExportJPEG Folder [%s] -- working [%d]\n", check_folder_name, retry_nn);
			gLogger.FlushLog();
		}
		return  true;
	}else{
		gLogger.LogMessage("ERROR: Can not access ExportJPEG Folder [%s] [%d]\n", check_folder_name, retry_nn);
		gLogger.FlushLog();
		return false ;
	}
}

#include "StoreSCUSeriesDirMonitor.h"
void doSelfTest()
{
	CStoreSCUSeriesDirMonitor::selfTest();

}