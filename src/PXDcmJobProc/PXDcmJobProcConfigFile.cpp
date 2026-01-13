/***********************************************************************
 * PXDcmSServerConfigFile.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2011, All rights reserved.
 *
 *	PURPOSE:
 *		Read DICOMServer configuration file 
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

#include "AqCore/TRPlatform.h"
#include "Globals.h"

#include "PXDcmJobProcConfigFile.h"


//#7 2012/03/08 K.Ko
 

template <class T> T iClamp(T a, T amin, T amax)
{
	return a < amin ? amin: ( a > amax ? amax:a);
}

JobProcServerConfigFile::JobProcServerConfigFile()
{
	m_ExportJPEG_FolderName[0] = 0; //#48
	m_ExportJPEG_FolderSubName[0] = 0; //#98 2017/07/10 N.Furutsuki
}

void JobProcServerConfigFile::Init()
{
	
	m_ActionGateForE2 = 1;//#73 2013/11/26 K.Ko
	m_ActionGate = 0; //#71 K.Ko
	strcpy(m_ActionGateYTWExePath,"C:\\Actiongate\\YTW29.exe");

	m_OutputJpegQuality = 90;//#48
	m_IDJpegFolderMaxLen = 10;//#48
	m_CTDataThumbnailIndex = 180;//#48

	//#103 2019/08/23 N.Furutsuki
	m_checkCTImageSize = 16;

	m_resultQueueMonitorInterval = 20; // Sec
	m_resultQueueMaxSize = 100;
	m_useSubThreadForLowPriority = 1;
	//
	m_retrySendInterval = 10; //Sec
	m_retrySendCount = 5;
	m_retrySendIntervalMax = 30 ;//Sec // #80 2014/08/14 K.Ko

	m_traceDicomLevel = 1;
	//m_port = 105;
	
//	strcpy(gConfig.m_cachePath, "c:/AQNetHome/DICOMCache/");
	strcpy(gConfig.m_reportPathname, "c:/AQNetInteractiveReport/");


//	m_maxAssociations = 10;
	m_seriesCompleteTimeout = 5; //JobProc
 
	m_maxStoreThreads = 30;
	m_maxCompressedThreads = 8;
	m_maxThreadsPerAssociation = 25;
	m_epsilon = 0.1f;
	strcpy(m_AEConfigPathname, "c:/AQNetConfig/ae/");
	strcpy(m_AEConfigFilename, "user_ae.cfg");
//	strcpy(m_defaultLocalAETitle, kDefaultLocalAETitle);
	m_dicomForward = 0;
	strcpy(m_dicomForwardHost,"");
	strcpy(m_dicomForwardAETitle,"");
	m_dicomForwardPort = 0;
	m_dicomForwardSerially = 0;
	m_deleteDicomFiles = 1;
	m_logLevel = 0;
	//m_logLocation = kLogToFile;
	m_logMaxSize = 1024;
	m_dicomLogLevel = 0;
#if 0
	m_writeCache = 1;
	m_threadWrite = 1;
#else
	m_writeCache = 0;
	m_threadWrite = 0;
#endif
	m_inProcessSeriesCompletion = 3;

	m_assocationReplyTimeout = 10;
	m_connectTimeout = 10;
	m_releaseTimeout = 10;
 
	m_numberOfRetries = 30;

	m_addDefaultOrientation = 0;

	 

	m_sendErrorForDuplicateUID = 0;
 
	strcpy(m_rootUID, "2.16.840.1.113669.632.21");

	 
 

	//	Make default databaseServerName upper(hostname)
	TRPlatform::NetInit();
	char buf[128];
	gethostname(buf, sizeof(buf)-1);

	//	Ignore domain
	char* p = strchr(buf, '.');
	if (p != NULL)
		*p = '\0';

	//	Make upper case for SQL Server 
	for(int i = 0; i < sizeof(buf) && buf[i] != '\0'; i++ )
	{
	  if(islower(buf[i]))
		 buf[i] = _toupper(buf[i]);
	}

	strncpy(m_dbServerName,buf,sizeof(buf));
 
	 
	 
	m_messageDump = 0;

 

	std::string configRootPath = "", configFile = ""; 
//	AppComConfiguration::GetConfigRootPath(configRootPath);
	//#7 2012/03/08 K.Ko
	AppComConfiguration::GetConfigRootPath(configRootPath) ;
 

	configFile = configRootPath+"PXDcmJobProc.cfg";
	strcpy(gConfig.m_serverNameStr, "PXDcmJobProc");

//	strcpy(gConfig.m_logFilename,   "c:/AQNetLog/PXDcmSServer.log");
	//#7 2012/03/08 K.Ko

	strcpy(gConfig.m_configFilename, configFile.c_str() );

	//-----------------------------
	std::string dataRootPath = "";
	AppComConfiguration::GetDataRootPath(dataRootPath);
	strcpy(gConfig.m_DB_FolderName,dataRootPath.c_str());
	//-----------------------------

	std::string LogRootPath ;
	AppComConfiguration::GetLogFilesLocation(LogRootPath);
	TRPlatform::MakeDirIfNeedTo(LogRootPath.c_str());

	strcpy(gConfig.m_logFilename,   (LogRootPath+"PXDcmJobProc.log").c_str());
 

	strcpy(gConfig.m_mergeLog,      (LogRootPath+"PXDcmJobProcDcmAPI.log").c_str() );
	///

    AppComConfiguration::GetArchiveDevices(AppComConfiguration::gkRAIDType,
                                          m_raidDevices);

 
	{//#71 K.Ko
		std::string MyHomePath ;
		AppComConfiguration::GetAQNetHome(MyHomePath);
		// Make sure you always have a trailing slash
		int homepath_size = MyHomePath.size();
		while(homepath_size >1){
			homepath_size = MyHomePath.size();

			if( ('/' ==  MyHomePath[homepath_size-1] ) ||
				('\\' == MyHomePath[homepath_size-1] )
				){
					MyHomePath = MyHomePath.substr(0,homepath_size-1);
			}else{
				break;
			}
		}
		strcpy(gConfig.m_JpegTempFolder,   (MyHomePath+"\\TempJpeg").c_str());
	}

	//	Default is 0 -> same syntax as input
	memset(m_mediaStorageTransferSyntaxUID, 0, 65);
	memset(m_autoRouteTransferSyntaxUID, 0, 65);

 

	m_validateCache = 1;	//	Default to NON-CacheReader based cache validation
	m_keepBadCache = 0;
	


	m_licenseCheckSleepSeconds = 3600;	//	Check license 1x / hr
	m_licenseCheckInitialState = 1;		//	Initial state is valid license


 

	memset(m_routingLogFile, 0, sizeof m_routingLogFile);

	m_enableStorageCommitment = 0;

	m_earlyHighwatermarkEmail = 0;
	m_reportProgress = 0;
	m_reportProgressFrequency = 20;

	m_rejectAllAssociations = 0;
	m_failStoreOneImage = 0;
	m_failStoreOneImagePermanently = 0;

	m_useStandardCodecs = 1;

	m_numOfRetriesGettingIPAddress = 5;

 
	m_safetyCheck = 1;////#27 2012/06/14 K.Ko

	m_EnableAutoDeleteStudy		= 0;	//#52
	m_AutoDeleteStudyInterval	= 10;	//Minute//#52
	m_AutoDeleteKeepDays		= 90;
	m_DeleteStudyNumberOneTime  = 3; //#52 一度削除する最大データ数

	//#55
	m_LogonComputerName[0] = 0;
	m_LogonUser[MAX_PATH] = 0;
	m_LogonPassword[MAX_PATH] = 0;
	m_LogonRetry = 10;
	m_checkShardeFolderRetry = 120; //120Sec
	//
	//#99 2017/08/08
	//Exception of TDO Screen Capture 
	strcpy(m_NoCheckSeriesUIDImageType1,"SC");
	m_NoCheckSeriesUIDImageType2[0] = 0;
	m_NoCheckSeriesUIDImageType3[0] = 0;
	m_NoCheckSeriesUIDImageType4[0] = 0;
}



#include "nmobject.h"
void JobProcServerConfigFile::InitKVPMap()
{
	if (GetCount() == 0)
	{
		Add("EnableAutoDeleteStudy", &m_EnableAutoDeleteStudy);//#52
		Add("AutoDeleteStudyInterval", &m_AutoDeleteStudyInterval);//#52
		Add("AutoDeleteKeepDays", &m_AutoDeleteKeepDays);//#52
		Add("DeleteStudyNumberOneTime", &m_DeleteStudyNumberOneTime);//#52

		Add("ExportJPEGFolder", m_ExportJPEG_FolderName);//#48
		Add("OutputJpegQuality", &m_OutputJpegQuality);//#48
		Add("CTDataThumbnailIndex", &m_CTDataThumbnailIndex);//#48

		//#103 2019/08/23 N.Furutsuki
		Add("CheckCTImageSize", &m_checkCTImageSize);//#48
	 

		Add("ExportJPEGFolderSub", m_ExportJPEG_FolderSubName);////#98 2017/07/10 N.Furutsuki

		Add("ActionGate", &m_ActionGate);//#71 K.Ko1
		Add("ActionGateYTWExe",m_ActionGateYTWExePath);
	 
		Add("ActionGateForE2", &m_ActionGateForE2);//#73 2013/11/26 K.Ko

		//	These should appear in the default config file and be displayable / editable
		//	from the web interface
		//Add("port", &m_port);
		Add("verbose", &m_logLevel);
		Add("DicomLogLevel", &m_dicomLogLevel);
		
		//Add("logLocation", &m_logLocation);
		Add("logMaxSize", &m_logMaxSize);
//		Add("aeConfigPathname", m_AEConfigPathname);
//		Add("aeConfigFilename", m_AEConfigFilename);
//		Add("defaultLocalAETitle", m_defaultLocalAETitle);
//		Add("maxAssociations", &m_maxAssociations);
		Add("seriesCompleteTimeout", &m_seriesCompleteTimeout);
 
		Add("maxStoreThreads", &m_maxStoreThreads);
		Add("maxCompressedThreads", &m_maxCompressedThreads);
		Add("maxThreadsPerAssociation", &m_maxThreadsPerAssociation);
	 

	 
		Add("assocationReplyTimeout", &m_assocationReplyTimeout);
		Add("connectTimeout", &m_connectTimeout);
		Add("releaseTimeout", &m_releaseTimeout);
	 
		Add("numberOfRetries", &m_numberOfRetries);

		//	These are supported, and can be added into the config file, but should not be
		//	displayable or editable from the web interface
		Add("databaseServerName", m_dbServerName);
		Add("writeCache", &m_writeCache);
		Add("inProcessSeriesCompletion", &m_inProcessSeriesCompletion);
		Add("threadWrite", &m_threadWrite);
		Add("sendErrorForDuplicateUID", &m_sendErrorForDuplicateUID);
		Add("rootUID", m_rootUID);
	 
	
		Add("messageDump", &m_messageDump);


 

 
		Add("validateCache", &m_validateCache);
		Add("keepBadCache", &m_keepBadCache);



		Add("routingLogFile", m_routingLogFile);
#if 0
		Add("FlipPETSliceSpacing", &CNMObject::m_sFlipSliceSpacing);
		//  workaround
		
		Add("AddNMDefaultOrientation", &CNMObject::m_sAddInitialOrientation );
		Add("FlipNMSliceSpacing", &CNMObject::m_sFlipSliceSpacing);
#endif

		Add("enableStorageCommitment", &m_enableStorageCommitment);


		// -- 2006.06.30 restore old highmark email behavior
		Add("EarlyhighwatermarkEmail", &m_earlyHighwatermarkEmail);

		Add("rejectAllAssociations", &m_rejectAllAssociations);
		Add("failStoreOneImage", &m_failStoreOneImage);
		Add("failStoreOneImagePermanently", &m_failStoreOneImagePermanently);
	

		Add("useStandardCodecs", &m_useStandardCodecs);
		Add("numOfRetriesGettingIPAddress", &m_numOfRetriesGettingIPAddress);

 
		Add("reportProgress", &m_reportProgress);
		Add("reportProgressFrequency", &m_reportProgressFrequency);
 

	 
		Add("traceDicomLevel",&m_traceDicomLevel);// 2010/04/27 K.Ko
		//
		Add("resultQueueMaxSize",&m_resultQueueMaxSize);
		Add("useSubThreadForLowPriority",&m_useSubThreadForLowPriority);
		
		//#27 2012/06/14 K.Ko
		Add("safetyCheck",&m_safetyCheck );
		//
		Add("retrySendInterval",&m_retrySendInterval );
		Add("retrySendCount",&m_retrySendCount );

		Add("retrySendIntervalMax",&m_retrySendIntervalMax );// #80 2014/08/14 K.Ko
		
		////////////
		//#55
		Add("LogonComputerName",m_LogonComputerName);
		Add("LogonUser",m_LogonUser);
		Add("LogonPassword",m_LogonPassword);
		Add("LogonRetry",&m_LogonRetry);
		Add("checkShardeFolderRetry",&m_checkShardeFolderRetry);
	 
		//#99 2017/08/08
		//Exception of TDO Screen Capture 
		Add("NoCheckSeriesUIDImageType1", m_NoCheckSeriesUIDImageType1);
		Add("NoCheckSeriesUIDImageType2", m_NoCheckSeriesUIDImageType2);
		Add("NoCheckSeriesUIDImageType3", m_NoCheckSeriesUIDImageType3);
		Add("NoCheckSeriesUIDImageType4", m_NoCheckSeriesUIDImageType4);
	}
}

extern TRLogger gRoutingLoggerHCS;
void JobProcServerConfigFile::ValidateAndFixConfiguration()
{
	//m_port					= iClamp(m_port, 1, 99999);
 
	m_seriesCompleteTimeout	= iClamp(m_seriesCompleteTimeout, 0, 36000);
 
	m_maxStoreThreads		= iClamp(m_maxStoreThreads, 5, 500);
	m_maxCompressedThreads	= iClamp(m_maxCompressedThreads, 3, 100);
	m_maxThreadsPerAssociation = iClamp(m_maxThreadsPerAssociation, 1, 1000);
	m_epsilon				= iClamp(m_epsilon, .01f, 0.5f);
	m_dicomForward			= iClamp(m_dicomForward, 0, 1);
	m_dicomForwardSerially	= iClamp(m_dicomForwardSerially, 0, 1);
	m_dicomForwardPort		= iClamp(m_dicomForwardPort, 104, 99999);
	m_deleteDicomFiles		= iClamp(m_deleteDicomFiles, 0, 1);
	//m_logLocation			= iClamp(m_logLocation, 0, 2);
	m_logMaxSize			= iClamp(m_logMaxSize, 10, 1048576);
 

	m_sendErrorForDuplicateUID = iClamp(m_sendErrorForDuplicateUID, 0, 1);
	m_writeCache			= iClamp(m_writeCache, 0, 3);				    //  Could be 0,1,2,3 : 0-DoNotWrite, 1-InProcess,2-OutOfProcess, 3- Using Queue-Manager
	m_threadWrite			= iClamp(m_threadWrite, 0, 1);
 
 
	m_assocationReplyTimeout= iClamp(m_assocationReplyTimeout, 0, 600);
	m_connectTimeout		= iClamp(m_connectTimeout, 0, 600);
	m_releaseTimeout		= iClamp(m_releaseTimeout, 0, 600);
 
	m_numberOfRetries		= iClamp(m_numberOfRetries, 1, 100);
 

	m_messageDump			= iClamp(m_messageDump, 0, 1);
 



	m_numOfRetriesGettingIPAddress = iClamp(m_numOfRetriesGettingIPAddress, 1, 240);



	if (m_reportPathname[strlen(m_reportPathname)-1] != '/')
	{
		strcat(m_reportPathname, "/");
	}

	if (m_AEConfigPathname[strlen(m_AEConfigPathname)-1] != '/')
	{
		strcat(m_AEConfigPathname, "/");
	}

	char *p;
	if ( ( p = strchr(m_dbServerName,'.')))
		*p = '\0';

	 

	
	m_validateCache = iClamp(m_validateCache, 0, 2);
	m_keepBadCache = iClamp(m_keepBadCache, 0, 1);


	m_licenseCheckSleepSeconds = iClamp(m_licenseCheckSleepSeconds, 1, 7200); 
	m_licenseCheckInitialState = iClamp(m_licenseCheckInitialState, 0, 1); 

 

	if (strlen(m_routingLogFile) > 0 && strchr(m_routingLogFile, '/') == 0 && strchr(m_routingLogFile, '\\') == 0)
	{
		std::string tmpLog = m_routingLogFile;
		strncpy(m_routingLogFile, "C:/AQNetLog/", sizeof m_routingLogFile);
		STRNCAT_S(m_routingLogFile, tmpLog.c_str(), sizeof m_routingLogFile);
		gRoutingLoggerHCS.SetLogFile(m_routingLogFile);
	}
 

	m_enableStorageCommitment = iClamp(m_enableStorageCommitment, 0, 1);	
	m_reportProgress = iClamp(m_reportProgress, 0, 1);
	m_reportProgressFrequency = iClamp(m_reportProgressFrequency, 1, 100);

	m_rejectAllAssociations = iClamp(m_rejectAllAssociations, 0, 1);
	m_failStoreOneImage = iClamp(m_failStoreOneImage, 0, 1);
	m_failStoreOneImagePermanently = iClamp(m_failStoreOneImagePermanently, 0, 1);


	m_useStandardCodecs = iClamp(m_useStandardCodecs, 0, 1);
}
