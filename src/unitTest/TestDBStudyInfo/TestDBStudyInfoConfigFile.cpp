/***********************************************************************
 * TestDBStudyInfoConfigFile.cpp
 *---------------------------------------------------------------------
 *   
 *-------------------------------------------------------------------
 */

#include "AqCore/TRPlatform.h"
#include "Globals.h"

#include "TestDBStudyInfoConfigFile.h"

template <class T> T iClamp(T a, T amin, T amax)
{
	return a < amin ? amin: ( a > amax ? amax:a);
}

TestDBStudyInfoConfigFile::TestDBStudyInfoConfigFile()
{
}

void TestDBStudyInfoConfigFile::Init()
{
		
	{
		m_backupInterval = 10;
		m_testThreadNum = 1;
		m_testBigDbLen = 100;

		for(int i=0;i<TestDB_ThreadMax;i++){
			m_patientName[i][0] = 0;
			//
			m_testThreadType[i] = 0;
		}
	}


	m_port = 105;
	
//	strcpy(gConfig.m_cachePath, "c:/AQNetHome/DICOMCache/");
	strcpy(gConfig.m_reportPathname, "c:/AQNetInteractiveReport/");

	m_maxAssociations = 10;
	m_seriesCompleteTimeout = 30;
	m_maxThreads = 180;
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
	m_writeCache = 1;
	m_threadWrite = 1;
	m_inProcessSeriesCompletion = 3;

	m_inactivityTimeout = 7200;
	m_assocationReplyTimeout = 10;
	m_connectTimeout = 10;
	m_releaseTimeout = 10;
	m_writeTimeout = 300;
	m_numberOfRetries = 30;

	m_addDefaultOrientation = 0;

	// Start
	strcpy(m_xtenderDrive , "C:/");
	strcpy(m_raidDrive , "C:/");
	m_raidWatermark = 500;		//500 MB
	m_extraFreeSpaceOnDVD = 100; //MB
	m_DVDConnected	= 0;
	m_raidMonitorInactiveTime = 60; //mins
	// End

	m_sendErrorForDuplicateUID = 0;
	m_changeDuplicateUIDs = 0;
	strcpy(m_rootUID, "2.16.840.1.113669.632.21");

	//	Rob Lewis - 4/10/02
	strcpy(m_mergeLicense,"");

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
	m_excludeQRLevelFromCFindRSP = 0;
	m_dontSendCMoveRSP = 0;
	m_requireCEchoBeforeCStoreRq  = 0;
	m_sendCMovePendingWithZeroRemaining = 0;
	m_maxSeriesPusher = 1;
	m_maxCompressThreads = 5;
	m_doCompressedCache = 0;
	m_dbConnectionPool = 1; // enable for test


	m_fixSpaceInUID = 1;
	m_useModalityLUT = 1;

	m_networkCapture = 0;
	m_messageDump = 0;

	m_overwriteDuplicates = 0;
	m_forceTransferSyntax = 0;
	m_enableMiniDumpTrigger = 0;

 
	m_enableAutoClean = 0;
	std::string configRootPath = "", configFile = ""; 
	AQNetConfiguration::GetConfigRootPath(configRootPath);
	configFile = configRootPath+"TestDBStudyInfo.cfg";
	strcpy(gConfig.m_serverNameStr, "AQNetDICOMServer");
	strcpy(gConfig.m_logFilename,   "c:/AQNetLog/TestDBStudyInfo.log");
	gConfig.m_logFilename1 = gConfig.m_logFilename + 11;
	strcpy(gConfig.m_mergeLog,      "c:/AQNetLog/AQNetDICOMServerMerge.log");
	gConfig.m_mergeLog1 = gConfig.m_mergeLog + 11;
	
	strcpy(gConfig.m_configFilename, configFile.c_str() );
	strcpy(gConfig.m_mergeCap,"c:/AQNetLog/AQNetDICOMServerMerge.cap");
 

    AQNetConfiguration::GetArchiveDevices(AQNetConfiguration::gkRAIDType,
                                          m_raidDevices);

	m_localStorage = 1;

	//	N = 1 -> no compression
	m_compressionRatio = 1;

	//	Default is 0 -> same syntax as input
	memset(m_mediaStorageTransferSyntaxUID, 0, 65);
	memset(m_autoRouteTransferSyntaxUID, 0, 65);

	m_pusherQueueSize = 1;	//	Default to single threaded

	m_validateCache = 1;	//	Default to NON-CacheReader based cache validation
	m_keepBadCache = 0;
	
	m_autoRouteRetries = 1;
	m_autoRouteTimeout = 20;

	m_licenseCheckSleepSeconds = 3600;	//	Check license 1x / hr
	m_licenseCheckInitialState = 1;		//	Initial state is valid license

	m_decompressOnForceILE = 1;
	m_alsoDecompressLossyOnForceILE = 1;

	m_queueSendJobs = 1;

	m_bypassDatabaseQuery = 0;
	m_bypassDatabaseInsert = 0;
	m_bypassHASP[0] = 0;

	m_requireHigherLevelKeys = 1;

	m_queryRIS = 0;

	// added by shiying, 2005.3.17
	m_writeCacheForCRImage = 1;

	//	where to send data if no rules apply
	strcpy(m_defaultRoutingTargetAE, "NONE");
	strcpy(m_defaultRoutingTargetIP, "0.0.0.0");
	m_defaultRoutingTargetPort = 0;
	m_defaultRoutingCompressionMethod = 0;
	m_defaultRoutingCompressionFactor = 0;
	memset(m_routingLogFile, 0, sizeof m_routingLogFile);

	m_enableStorageCommitment = 0;

	m_earlyHighwatermarkEmail = 0;
	m_reportProgress = 0;
	m_reportProgressFrequency = 20;

	m_rejectAllAssociations = 0;
	m_failStoreOneImage = 0;
	m_failStoreOneImagePermanently = 0;

	m_numberOfNetworkCapture = 0;
	m_hangCMove = 0;
	m_autoBumpLogLevel = 0;
	m_useStandardCodecs = 1;

	m_numOfRetriesGettingIPAddress = 5;
}



#include "nmobject.h"

 
char __searchPatientEntry[TestDB_ThreadMax][64];
char __testDBTypeEntry[TestDB_ThreadMax][64];

void TestDBStudyInfoConfigFile::InitKVPMap()
{
	if (GetCount() == 0)
	{
			
		///////////////////
	Add("TestDBThreadNum",&m_testThreadNum);
	Add("TestBigDbLen",&m_testBigDbLen);
	Add("backupInterval",&m_backupInterval);

	{
		for(int i=0;i<TestDB_ThreadMax;i++){
			sprintf(__searchPatientEntry[i],"searchPatient%d",i+1);

			Add(__searchPatientEntry[i], m_patientName[i]);
			//
			sprintf(__testDBTypeEntry[i],"testDBType%d",i+1);
			Add(__testDBTypeEntry[i], &m_testThreadType[i]);
		}
	}
 
 
	////////////////////



		//	These should appear in the default config file and be displayable / editable
		//	from the web interface
		Add("port", &m_port);
		Add("verbose", &m_logLevel);
		Add("DicomLogLevel", &m_dicomLogLevel);
		
		//Add("logLocation", &m_logLocation);
		Add("logMaxSize", &m_logMaxSize);
//		Add("aeConfigPathname", m_AEConfigPathname);
//		Add("aeConfigFilename", m_AEConfigFilename);
//		Add("defaultLocalAETitle", m_defaultLocalAETitle);
		Add("maxAssociations", &m_maxAssociations);
		Add("seriesCompleteTimeout", &m_seriesCompleteTimeout);
		Add("maxThreads", &m_maxThreads);
		Add("maxStoreThreads", &m_maxStoreThreads);
		Add("maxCompressedThreads", &m_maxCompressedThreads);
		Add("maxThreadsPerAssociation", &m_maxThreadsPerAssociation);
		Add("changeDuplicateUIDs", &m_changeDuplicateUIDs);
		Add("overwriteDuplicates", &m_overwriteDuplicates);

		Add("inactivityTimeout", &m_inactivityTimeout);
		Add("assocationReplyTimeout", &m_assocationReplyTimeout);
		Add("connectTimeout", &m_connectTimeout);
		Add("releaseTimeout", &m_releaseTimeout);
		Add("writeTimeout", &m_writeTimeout);
		Add("numberOfRetries", &m_numberOfRetries);

		//	These are supported, and can be added into the config file, but should not be
		//	displayable or editable from the web interface
		Add("databaseServerName", m_dbServerName);
		Add("writeCache", &m_writeCache);
		Add("inProcessSeriesCompletion", &m_inProcessSeriesCompletion);
		Add("threadWrite", &m_threadWrite);
		Add("sendErrorForDuplicateUID", &m_sendErrorForDuplicateUID);
		Add("rootUID", m_rootUID);
		Add("mergeLicense", m_mergeLicense);
		Add("excludeQRLevelFromCFindRSP", &m_excludeQRLevelFromCFindRSP);
		Add("dontSendCMoveRSP", &m_dontSendCMoveRSP);
		Add("requireCEchoBeforeCStoreRq", &m_requireCEchoBeforeCStoreRq);
		Add("sendCMovePendingWithZeroRemaining", &m_sendCMovePendingWithZeroRemaining);
		Add("maxSeriesPusher", &m_maxSeriesPusher);
		Add("maxCompressThreads", &m_maxCompressThreads);
		Add("fixSpaceInUID", &m_fixSpaceInUID);
		Add("doCompressedCache", &m_doCompressedCache);
		Add("DBConnectionPool", &m_dbConnectionPool);

		Add("networkCapture", &m_networkCapture);
		Add("messageDump", &m_messageDump);

		Add("UseModalityLUT", &m_useModalityLUT);
		Add("forceTransferSyntax", &m_forceTransferSyntax);
		Add("enableMiniDumpTrigger", &m_enableMiniDumpTrigger);
		Add("enableAutoClean", &m_enableAutoClean);

//		Add("localStorage", &m_localStorage);
//		Add("compressionRatio", &m_compressionRatio);

		Add("validateCache", &m_validateCache);
		Add("keepBadCache", &m_keepBadCache);

		Add("autoRouteRetries", &m_autoRouteRetries);
		Add("autoRouteTimeout", &m_autoRouteTimeout);

//		Add("licenseCheckSleepSeconds", &m_licenseCheckSleepSeconds);
//		Add("licenseCheckInitialState", &m_licenseCheckInitialState);
		
		Add("decompressOnForceILE", &m_decompressOnForceILE);
		Add("alsoDecompressLossyOnForceILE", &m_alsoDecompressLossyOnForceILE);

		Add("queueSendJobs", &m_queueSendJobs);

		Add("bypassDatabaseQuery", &m_bypassDatabaseQuery);
		Add("bypassDatabaseInsert", &m_bypassDatabaseInsert);
		Add("[s@gt$29", m_bypassHASP);

		Add("requireHigherLevelKeys", &m_requireHigherLevelKeys);
		Add("queryRIS", &m_queryRIS);

		Add("FlipPETSliceSpacing", &CNMObject::m_sFlipSliceSpacing);
	//	Turn off threaded sending for now
//		Add("pusherQueueSize", &m_pusherQueueSize);

//		Add("mediaStorageTransferSyntaxUID", m_mediaStorageTransferSyntaxUID);
//		Add("m_autoRouteTransferSyntaxUID", m_autoRouteTransferSyntaxUID);

	/*	These will be stored in the registry - ignore them in config file
		Add("logFilename", m_logFilename);
		Add("mergeLog", m_mergeLog);
		Add("cachePath", m_cachePath);
		Add("reportPathname", m_reportPathname);
   
	These will not be supported in Release 1.0
		Add("dicomForward", &m_dicomForward);
		Add("dicomForwardHost", m_dicomForwardHost);
		Add("dicomForwardAETitle", m_dicomForwardAETitle);
		Add("dicomForwardPort", &m_dicomForwardPort);
		Add("dicomForwardSerially", &m_dicomForwardSerially);
		Add("deleteDicomFiles", &m_deleteDicomFiles);

		Add("xtenderDrive", m_xtenderDrive);
		Add("raidDrive", m_raidDrive);
		Add("raidWatermark", &m_raidWatermark);
		Add("extraFreeSpaceOnDVD", &m_extraFreeSpaceOnDVD);
		Add("DVDConnected", &m_DVDConnected);
		Add("raidMonitorInactiveTime", &m_raidMonitorInactiveTime);

		Add("epsilon", &m_epsilon);
*/
		// added by shiying hu, 2005.3.17
		Add("writeCacheForCRImage", &m_writeCacheForCRImage);

		Add("defaultRoutingTargetAE", m_defaultRoutingTargetAE);
		Add("defaultRoutingTargetIP", m_defaultRoutingTargetIP);
		Add("defaultRoutingTargetPort", &m_defaultRoutingTargetPort);

		Add("defaultRoutingCompressionMethod", &m_defaultRoutingCompressionMethod);
		Add("defaultRoutingCompressionFactor", &m_defaultRoutingCompressionFactor);

		Add("routingLogFile", m_routingLogFile);

		// T.C. Zhao 2005.05.19 Kumamato workaround
		Add("AddNMDefaultOrientation", &CNMObject::m_sAddInitialOrientation );
		Add("FlipNMSliceSpacing", &CNMObject::m_sFlipSliceSpacing);

		Add("enableStorageCommitment", &m_enableStorageCommitment);


		// T.C. Zhao 2006.06.30 restore old highmark email behavior
		Add("EarlyhighwatermarkEmail", &m_earlyHighwatermarkEmail);

		Add("rejectAllAssociations", &m_rejectAllAssociations);
		Add("failStoreOneImage", &m_failStoreOneImage);
		Add("failStoreOneImagePermanently", &m_failStoreOneImagePermanently);
		Add("numberOfNetworkCapture", &m_numberOfNetworkCapture);
		Add("hangCMove", &m_hangCMove);

		Add("autoBumpLogLevel", &m_autoBumpLogLevel);
		Add("useStandardCodecs", &m_useStandardCodecs);
		Add("numOfRetriesGettingIPAddress", &m_numOfRetriesGettingIPAddress);

#ifndef AQUARIUS_GATE_ONLY	
		Add("reportProgress", &m_reportProgress);
		Add("reportProgressFrequency", &m_reportProgressFrequency);
#endif
	}
}

extern TRLogger gRoutingLoggerHCS;
void TestDBStudyInfoConfigFile::ValidateAndFixConfiguration()
{
	m_port					= iClamp(m_port, 1, 99999);
	m_maxAssociations		= iClamp(m_maxAssociations, 1, 25);
	m_seriesCompleteTimeout	= iClamp(m_seriesCompleteTimeout, 0, 36000);
	m_maxThreads			= iClamp(m_maxThreads, 25, 1000);
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
	m_changeDuplicateUIDs	= iClamp(m_changeDuplicateUIDs, 0, 1);
	m_overwriteDuplicates	= iClamp(m_overwriteDuplicates, 0, 10000);		//	Could be 0,1 but making this a code for security
	m_sendErrorForDuplicateUID = iClamp(m_sendErrorForDuplicateUID, 0, 1);
	m_writeCache			= iClamp(m_writeCache, 0, 3);				    //  Could be 0,1,2,3 : 0-DoNotWrite, 1-InProcess,2-OutOfProcess, 3- Using Queue-Manager
	m_threadWrite			= iClamp(m_threadWrite, 0, 1);
	m_doCompressedCache		= iClamp(m_doCompressedCache, 0, 1);
	m_dbConnectionPool		= iClamp(m_dbConnectionPool, 0, 1);
	m_inactivityTimeout		= iClamp(m_inactivityTimeout, 0, 36000);
	m_assocationReplyTimeout= iClamp(m_assocationReplyTimeout, 0, 600);
	m_connectTimeout		= iClamp(m_connectTimeout, 0, 600);
	m_releaseTimeout		= iClamp(m_releaseTimeout, 0, 600);
	m_writeTimeout			= iClamp(m_writeTimeout, 0, 600);
	m_numberOfRetries		= iClamp(m_numberOfRetries, 1, 100);
	m_maxSeriesPusher		= iClamp(m_maxSeriesPusher, 1, 20);
	m_maxCompressThreads	= iClamp(m_maxCompressThreads, 1, 20);

	m_fixSpaceInUID			= iClamp(m_fixSpaceInUID, 0, 1);
	m_networkCapture		= iClamp(m_networkCapture, 0, 1);
	m_messageDump			= iClamp(m_messageDump, 0, 1);
	m_forceTransferSyntax	= iClamp(m_forceTransferSyntax, 0, 3);
	m_enableMiniDumpTrigger = iClamp(m_enableMiniDumpTrigger, 0, 1);
	m_enableAutoClean		= iClamp(m_enableAutoClean, 0, 1);

	m_numOfRetriesGettingIPAddress = iClamp(m_numOfRetriesGettingIPAddress, 1, 240);
	/*

	TerareconCacheWriter::SetUseModalityLUT(m_useModalityLUT);

/*
	if (m_cachePath[strlen(m_cachePath)-1] != '/')
	{
		strcat(m_cachePath, "/");
	}
*/
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

	//Archive Manager 11/08/01 JWU
	// Start
	if (m_xtenderDrive[strlen(m_xtenderDrive)-1] != '/')
	{
		strcat(m_xtenderDrive, "/");
	}
	if (m_raidDrive[strlen(m_raidDrive)-1] != '/')
	{
		strcat(m_raidDrive, "/");
	}

	m_excludeQRLevelFromCFindRSP	= iClamp(m_excludeQRLevelFromCFindRSP, 0, 1);
	m_dontSendCMoveRSP				= iClamp(m_dontSendCMoveRSP, 0, 1);
	m_requireCEchoBeforeCStoreRq	= iClamp(m_requireCEchoBeforeCStoreRq, 0, 1);	
	m_sendCMovePendingWithZeroRemaining = iClamp(m_sendCMovePendingWithZeroRemaining, 0, 1);
	// End

	m_localStorage = iClamp(m_localStorage, 0, 1);
	m_compressionRatio = iClamp(m_compressionRatio, 0, 1);

	m_pusherQueueSize = iClamp(m_pusherQueueSize, 1, 50);
	m_validateCache = iClamp(m_validateCache, 0, 2);
	m_keepBadCache = iClamp(m_keepBadCache, 0, 1);

	m_autoRouteRetries = iClamp(m_autoRouteRetries, 0, 10);
	m_autoRouteTimeout = iClamp(m_autoRouteTimeout, 3, 600);

	m_licenseCheckSleepSeconds = iClamp(m_licenseCheckSleepSeconds, 1, 7200); 
	m_licenseCheckInitialState = iClamp(m_licenseCheckInitialState, 0, 1); 

	m_decompressOnForceILE = iClamp(m_decompressOnForceILE, 0, 1); 
	m_alsoDecompressLossyOnForceILE = iClamp(m_alsoDecompressLossyOnForceILE, 0, 1); 

	m_queueSendJobs = iClamp(m_queueSendJobs, 0, 1); 

	m_bypassDatabaseQuery = iClamp(m_bypassDatabaseQuery, 0, 1); 
	m_bypassDatabaseInsert = iClamp(m_bypassDatabaseInsert, 0, 1); 

	m_requireHigherLevelKeys = iClamp(m_requireHigherLevelKeys, 0, 1); 

	m_queryRIS = iClamp(m_queryRIS, 0, 1); 

	// added by shiying hu, 2005.3.17
	m_writeCacheForCRImage			= iClamp(m_writeCacheForCRImage, 0, 1);

	m_defaultRoutingCompressionMethod = iClamp(m_defaultRoutingCompressionMethod, 0, 1);;
	m_defaultRoutingCompressionFactor = iClamp(m_defaultRoutingCompressionFactor, 0, 1);;
	m_defaultRoutingTargetPort = iClamp(m_defaultRoutingTargetPort, 0, 99999);

	if (strlen(m_routingLogFile) > 0 && strchr(m_routingLogFile, '/') == 0 && strchr(m_routingLogFile, '\\') == 0)
	{
		std::string tmpLog = m_routingLogFile;
		strncpy(m_routingLogFile, "C:/AQNetLog/", sizeof m_routingLogFile);
		strncat(m_routingLogFile, tmpLog.c_str(), sizeof m_routingLogFile);
		gRoutingLoggerHCS.SetLogFile(m_routingLogFile);
	}

	m_enableStorageCommitment = iClamp(m_enableStorageCommitment, 0, 1);	
	m_reportProgress = iClamp(m_reportProgress, 0, 1);
	m_reportProgressFrequency = iClamp(m_reportProgressFrequency, 1, 100);

	m_rejectAllAssociations = iClamp(m_rejectAllAssociations, 0, 1);
	m_failStoreOneImage = iClamp(m_failStoreOneImage, 0, 1);
	m_failStoreOneImagePermanently = iClamp(m_failStoreOneImagePermanently, 0, 1);
	m_numberOfNetworkCapture = iClamp(m_numberOfNetworkCapture, 0, 30);
	m_hangCMove = iClamp(m_hangCMove, 0, 1);

	m_autoBumpLogLevel = iClamp(m_autoBumpLogLevel, 0, 1);
	m_useStandardCodecs = iClamp(m_useStandardCodecs, 0, 1);
}
