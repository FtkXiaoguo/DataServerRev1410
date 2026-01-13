/***********************************************************************
 * PXDcmSServerConfigFile.h
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
#ifndef AUTO_VOX_CONFIG_FILE_H
#define AUTO_VOX_CONFIG_FILE_H

#pragma warning (disable: 4503)

#include "rtvloadoption.h"
#include "PXDcmJobProc.h"
#include "AppComConfiguration.h"

//#define kLogToScreen 0
//#define kLogToFile 1
//#define kLogToBoth 2

//#define kErrorMsg 0
//#define kWarningMsg 1
//#define kInfoMsg 2
//#define kTransMsg 3
//#define kDebugMsg 4
#define	kMaxAssociationThreads	300

class CDBTools;
//class ConfigWatcher; 

class JobProcServerConfigFile : public iRTVOptions // RTVConfigFile
{
public:
	JobProcServerConfigFile();
	~JobProcServerConfigFile() {}

	void Init();
	void InitKVPMap();
	void ValidateAndFixConfiguration();

//	ConfigWatcher m_configWatcher;

	//	Override rtvbase
	//void LogMessage(int verbose, int location, const char *, ...);

	char m_ExportJPEG_FolderName[1024];//#48
	char m_ExportJPEG_FolderSubName[1024];//#98 2017/07/10 N.Furutsuki
	int  m_OutputJpegQuality;//#48
	int m_IDJpegFolderMaxLen;
	int m_CTDataThumbnailIndex;
	//#103 2019/08/23 N.Furutsuki
	int m_checkCTImageSize;

	char m_DB_FolderName[1024];

	char m_configFilename[256];
//	int m_port;
	char m_logFilename[256];
	char m_mergeLog[256]; 
	char m_reportPathname[256];
 
 
	int m_maxStoreThreads;
	int m_maxCompressedThreads;
	int m_maxThreadsPerAssociation;
	int m_seriesCompleteTimeout;
	char m_AEConfigPathname[256];
	char m_AEConfigFilename[256];
//	char m_defaultLocalAETitle[256];
	char m_dbServerName[64];
	long m_dbType;
	float m_epsilon;
	int m_dicomForward;
	char m_dicomForwardHost[64];
	char m_dicomForwardAETitle[17];
	int m_dicomForwardPort;
	int m_dicomForwardSerially;
	int m_deleteDicomFiles;
	//int m_verbose;
	int	m_logLevel;
	int	m_dicomLogLevel;
//	int m_logLocation;
	int m_logMaxSize;
	int m_writeCache;
	int m_inProcessSeriesCompletion;
	int m_threadWrite;
	 
	//	-- - 06/20/02 - Need control of these timeouts
	int m_assocationReplyTimeout;
	int m_connectTimeout;
	int m_releaseTimeout;
 

	int m_numberOfRetries;

	//Archive Manager 11/08/01 JWU
	// Start
 
	int m_traceDicomLevel;  // 2010/04/27 K.Ko
	                        // 0: nothing, 1: association, 2: message
	// End

	std::vector <AppComDevice> m_raidDevices;

	//	Japanese PACS don't use this response, so we need to turn it off
	int m_sendErrorForDuplicateUID;

 
	char m_rootUID[65];

	 

 
	time_t m_lastCleaned;
 

 

	
	void LogVersionInfo(void);

	
	int m_messageDump;



 
	//	Default - same as input
	char m_mediaStorageTransferSyntaxUID[65];

	//	Default - same as input
	char m_autoRouteTransferSyntaxUID[65];

	//	0 -> No cache validation
	//	1 -> New (non-cacheReader) validation
	//	2 -> Old (cacheReader) validation
	int m_validateCache;
	int m_keepBadCache;


	int m_licenseCheckSleepSeconds;
	int m_licenseCheckInitialState;



	char m_serverNameStr[32];


	char m_routingLogFile[MAX_PATH];

	// 
	// workaround
	int	m_addDefaultOrientation;

	//	For beta testing
	int m_enableStorageCommitment;


	int m_earlyHighwatermarkEmail;
	int m_reportProgress;
	int m_reportProgressFrequency;

	//	For testing only
	int m_rejectAllAssociations;
	int m_failStoreOneImage;
	int m_failStoreOneImagePermanently;

	int m_useStandardCodecs;

	int m_numOfRetriesGettingIPAddress; // 
	///
	int m_resultQueueMonitorInterval; // Sec
	int m_resultQueueMaxSize ;
	//
	int	m_useSubThreadForLowPriority;

	//
	int m_safetyCheck;////#27 2012/06/14 K.Ko
	//
	int m_retrySendInterval; //Sec
	int m_retrySendCount;
	int	m_retrySendIntervalMax;//Sec // #80 2014/08/14 K.Ko
	//
	int	m_EnableAutoDeleteStudy;	//#52
	int m_AutoDeleteStudyInterval;	//Minute//#52
	int m_AutoDeleteKeepDays;		//#52
	int m_DeleteStudyNumberOneTime; //#52 一度削除する最大データ数
	//#55
	char m_LogonComputerName[MAX_PATH];
	char m_LogonUser[MAX_PATH];
	char m_LogonPassword[MAX_PATH];
	int  m_LogonRetry;
	int	 m_checkShardeFolderRetry; // x 1Sec
	//#71 K.Ko
	int  m_ActionGate; //
	char m_ActionGateYTWExePath[MAX_PATH*4];
	char m_JpegTempFolder[MAX_PATH];
	int	 m_ActionGateForE2;//#73 2013/11/26 K.Ko
	//#99 2017/08/08
	//Exception of TDO Screen Capture 
	char m_NoCheckSeriesUIDImageType1[16];
	char m_NoCheckSeriesUIDImageType2[16];
	char m_NoCheckSeriesUIDImageType3[16];
	char m_NoCheckSeriesUIDImageType4[16];
};

#endif // AUTO_VOX_CONFIG_FILE_H