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
//#include "ArchiveManager.h"
#include "AppComConfiguration.h"
#include "DiskSpaceManager.h"
#include "Listener.h"
#include "RTVDiCOMStore.h"
#include "PxNetDB.h"
#include "globals.h"
//#include "SMTPSendMessage.h"
//#include "FilenameList.h"
#include "AqCore/TRLogger.h"
#include "PXDcmSServer.h"
//#include "RTVMiniDump.h"
#include "PxDicomutil.h"

 #include "AutoRoutingMonitor.h"
#include "PxQueue.h"

//The define is commented to enable controlling the cache writing using the gConfig.m_writeCache flag
 

 
//#9 2012/03/19 K.Ko for New Hasp
//#include "PXLicenseManagerIf.h"
#include "ThreadedLicenseChecker.h"
 

//	Global config & logging
TIDICOMServerConfigFile gConfig;
TRLogger gLogger;
TRLogger gDcmApiLogger; // #4 2012/02/21 K.Ko

LARGE_INTEGER gCountsPerSecond;
 

//GATE_ONLY
//TIDICOMServiceProcessor processor("PXDicomGate");
 
TIDICOMServiceProcessor processor("PXDcmSServer");
 

RTVDaemon service(&processor);

//const char* gEmailConfigFile = "C:/AQNetConfig/emailConfig.cfg";
//const char* gEmailRecipientsFile = "C:/AQNetConfig/highWatermarkAlertEmails.cfg";
//SMTPSendMessage gEmailer;
int gHASPFeatureID;
int gHASPFeatureID2;//#47

TRLogger gRoutingLogger;

bool dicom_server_checkLicense();

/*
*  home folder 
*  c:\PxSDcmServerHome\    
*/
static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};
 
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
 

void getHomeFromModulePath();
void setupHomePath(const std::string &folder);
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

int ServerMain (int argc, char** argv)
{
	gConfig.Init();
	gConfig.InitKVPMap();
	gConfig.Load(gConfig.m_configFilename);
	gConfig.ValidateAndFixConfiguration();
	

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

	RTVDiCOMStore::c_maxStoreThread = gConfig.m_maxStoreThreads;
	RTVDiCOMStore::c_maxCompressedThread = gConfig.m_maxCompressedThreads;

	//	 - Check to see if someone is already listening on this port - (maybe another instance of us)
	{
		RTVSocket sock;
		if (sock.Listen(gConfig.m_port) == RTVSocket::BUSY)
		{
			gLogger.LogMessage("ERROR:[C%08d] socket port %d is BUSY\n",DicomServError_SoketBusy,gConfig.m_port);
			gLogger.FlushLog();
			return 0;
		}
	}

 
 
//#9 2012/03/19 K.Ko for New Hasp
#if 0
gHASPFeatureID = ePREXION_PXDICOMSERVER3;		//#9 2012/03/19 K.Ko for New Hasp
gHASPFeatureID2 = ePREXION_FINECUBE_IMAGESERVER;	//#47
dicom_server_checkLicense(); // K.Ko 2010/05/21
#endif
 

	//	Is it AqNET Sonic?
	bool ignoreBypass = true;
 
 

	//	Start the license checker thread
//	ThreadedLicenseChecker& theThreadedLicenseChecker = ThreadedLicenseChecker::theThreadedLicenseChecker();
//	iRTVThread lmThread(&theThreadedLicenseChecker);
	
	RTVDiCOMStore::c_maxStoreThread = gConfig.m_maxStoreThreads;
	RTVDiCOMStore::c_maxCompressedThread = gConfig.m_maxCompressedThreads;

	AqCOMThreadInit comInitGuard;
	
	//set up server name, currently comment to use default setup from registry
	//CPxDcmDB::InitDBServerName(gConfig.m_dbServerName);

	
 
		{
			// #88 2016/09/26 by N.Furutsuki
			gLogger.LogMessage(" DB sever name %s,  user: %s, pw: %s \n",
				DBCore::GetSQLServerDBNameExt(),
				DBCore::GetSQLServerUser(),
				DBCore::GetSQLServerPassword());
			gLogger.FlushLog();

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
 
		
	/*
	* setup Queue DB
	*/
	std::string queue_db_name = std::string(gConfig.m_DB_FolderName) + "\\" + SQLITE_DB_FILTE;
	CPxQueueProc::setupQueueDBName(queue_db_name);
	
 	std::string queue_entry_folder = std::string(gConfig.m_DB_FolderName) + "\\entry\\";
 	CPxQueueProc::setupQueueEntryFolder(queue_entry_folder);

	//
	//	Initialize MergeCOM DICOM library
	//
	Listener& theListener = Listener::theListener();
	if(!theListener.DICOM_Initialization()){
		gLogger.LogMessage("ERROR:[C%08d] DICOM_Initialization error  !\n",DicomServError_DicomLibInitError);
		gLogger.FlushLog();
		return 1;
	}

	//#140_search_Japanese_JIS_UTF8
	if (CPxDB::InitCharacterLib()<0){
		gLogger.LogMessage("*** InitCharacterLib failed \n");
	};
	///
	CPxQueueProc::Global_init();

	/*  
	 * Emit the version ID of the server
	 */

	gConfig.LogVersionInfo();

	gConfig.Save(stdout);
	
	theListener.OnAETitlesChanged(true);

	char AEConfigFilename[256];

// In release build, we do not want JIT debugging. Hope the try/catch disables it
//#if !defined(_DEBUG)
#if 0
try
{
#endif

	strncpy(AEConfigFilename, gConfig.m_AEConfigPathname, 256);
	STRNCAT_S(AEConfigFilename, gConfig.m_AEConfigFilename, 256);
	
	//ArchiveManager am(gConfig.m_raidDrive, &AEmon);
	
	// start working threads
	RTVDiskSpaceManager::StartupAll();
 	iRTVThread InaMangerThread(&RTVInactiveManager::theManager());

	//#13 2012/03/28
	//これを起動する必要があるか？
  	iRTVThread OneShotThreadManagerThread(&RTVOneShotThreadManager::theManager());

	// run max 3 AQnetsendimage threads concuerrently only
	RTVOneShotThreadManager::theManager().SetMaxRunThreads(gConfig.m_maxSeriesPusher, "SeriesPusher");
	// SeriesPusher run insdie HandleSeriesComplete, use m_maxSeriesPusher for HandleSeriesComplete
	RTVOneShotThreadManager::theManager().SetMaxRunThreads(gConfig.m_maxSeriesPusher, "HandleSeriesComplete");

	// The define is replaced with gConfig.m_writeCache flag to enable 
	// controlling the cache writing using the external cache writer.
	// Murali - 2006.01.05
//#ifdef _EX_CACHE_WRITER
#if 0
	if(gConfig.m_writeCache == 2)
	{
		// run max 3 RTVCacheWriter threads concuerrently only
		RTVOneShotThreadManager::theManager().SetMaxRunThreads(3, "RTVCacheWriter");
		//RTVCacheWriter::c_logLevel = gConfig.GetVerbose();
		RTVCacheWriter::c_doCompress = (gConfig.m_doCompressedCache)?true:false;
	}
#endif
//#endif
	/////////////////////////////
	//AutoRouringTirg_BlockSize　以外の場合、起動しても、スレッド直ぐ終わる
	// ref: CAutoRoutingMonitor::Process
	///
	CAutoRoutingMonitor::theAutoRoutingMonitor().NapTime(1);
	iRTVThread AutoRoutingMonitorThread(&CAutoRoutingMonitor::theAutoRoutingMonitor());
 
	/////////////////////////////

	
	if (gConfig.m_doCompressedCache)
		RTVOneShotThreadManager::theManager().SetMaxRunThreads(gConfig.m_maxCompressThreads, "TRCompressedCacheWriter");

	struct tm when;
	time_t now;

	time( &now );
	when = *localtime( &now );
	AppComConfiguration::GetAutoWakeupHour(when.tm_hour);
	gConfig.m_lastCleaned = mktime( &when );

	theListener.PreProcess();
	theListener.Process();
	
	// shutdown procedures
	RTVOneShotThreadManager::theManager().RequestTermination(1);
 	RTVInactiveManager::theManager().RequestTermination(1);
	
	////////////////
 
	CAutoRoutingMonitor::theAutoRoutingMonitor().RequestTermination(1);
 
	////////////////
	// make sure all stopped
	RTVOneShotThreadManager::theManager().ForceStop(18000);
 	RTVInactiveManager::theManager().ForceStop(18000);
	
	theListener.DICOM_Release(); //MC_Library_Release();
	RTVDiskSpaceManager::ShutdownAll();

	//	If we ever get here, we should clean up
	//Sleep(2000);

	
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
	Listener::theListener().RequestTermination(1); // trigger main loop to break
	// while wating main thread to shutdown, keep pump stop pending message to service manager
	int count = 0;
#if 0
// 	while(!m_stop && count < 200)
 	while(!m_stop && count < 2)//#30 2012/07/03
	{
	//	service.ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
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
void TIDICOMServerConfigFile::LogVersionInfo(void)
{
	char buf[256];

 
	std::string awareVersion = Compression::GetJ2KToolkitVersionString();
#if 0
 	std::string haspVersion  = LicenseManager::theLicenseManager().GetHaspDriverVersionString();
#else
	std::string haspVersion = "Unknown";
#endif


	snprintf(buf, sizeof buf, 
		"------------------ DICOMServer Starts ------------\n  DICOMServer V%d.%d.%d.%d (built on %s) [DcmLib: %s][Hasp API: %s] Started\n",
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
	//
	//#93　2017/02/13 N.Furutsuki
	//#94
	bool isLocalBackup = AppComConfiguration::GetLocalBackupFlag() == 1;
	 
	gLogger.LogMessage("==== LocalBackup [%d] ==========\n", isLocalBackup ? 1 : 0);


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

#if 1
	return true; // always OK
#else
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
			 
			if(AppComUtil::ChkHifDrv(chkHifMsg,256) == ChkHifDrv_OK ){ //#33 HIFチェックの追加 2012/09/06 K.Ko
				ret_val = true;
				gLogger.LogMessage(" ### Found HIF License %s ### \n",chkHifMsg);
			}else{
			 
				ret_val = false;
				gLogger.LogMessage("ERROR:[C%08d] No valid license - either HASP key is not attached, or license file is expired\n",DicomServError_LicenceInvalid);
			}
		 
		}
	}

#endif
	//	Add the feature to the internal map for periodic checking by the license checker thread
//	int daysToExpire = 0;
//	LicenseManager::theLicenseManager().CheckLicense(gHASPFeatureID, daysToExpire);

	return ret_val;
 
}