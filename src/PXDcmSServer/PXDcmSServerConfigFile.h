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
#include "PXDcmSServer.h"
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

class TIDICOMServerConfigFile : public iRTVOptions // RTVConfigFile
{
public:
	TIDICOMServerConfigFile();
	~TIDICOMServerConfigFile() {}

	void Init();
	void InitKVPMap();
	void ValidateAndFixConfiguration();

//	ConfigWatcher m_configWatcher;

	//	Override rtvbase
	//void LogMessage(int verbose, int location, const char *, ...);

	char m_DB_FolderName[1024];

	char m_configFilename[256];
	int m_port;
	char m_logFilename[256];
	char m_mergeLog[256]; 
	char m_reportPathname[256];
	int m_maxAssociations;
	int m_maxThreads;
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
	int m_postDicomResponse; //2010/03/16 k.ko #660

	//	-- - 06/20/02 - Need control of these timeouts
	int m_assocationReplyTimeout;
	int m_connectTimeout;
	int m_releaseTimeout;
	int m_writeTimeout;
	int m_inactivityTimeout;
	int m_numberOfRetries;

	//Archive Manager 11/08/01 JWU
	// Start
	char m_xtenderDrive [256];
	char m_raidDrive[256];
	int m_raidWatermark;		//500 MB
	int m_extraFreeSpaceOnDVD; //MB
	int m_DVDConnected;
	int m_raidMonitorInactiveTime; 
	//
	int m_traceDicomLevel;  // 2010/04/27 K.Ko
	                        // 0: nothing, 1: association, 2: message
	// End

	std::vector <AppComDevice> m_raidDevices;

	//	Japanese PACS don't use this response, so we need to turn it off
	int m_sendErrorForDuplicateUID;

	int m_changeDuplicateUIDs;
	char m_rootUID[65];

 

	//	Used for simulating non-compliant DICOM behavior
	int m_excludeQRLevelFromCFindRSP;
	int m_dontSendCMoveRSP;
	int m_requireCEchoBeforeCStoreRq;
	int m_sendCMovePendingWithZeroRemaining;

	time_t m_lastCleaned;
	int m_maxSeriesPusher;
	int m_pusherQueueSize;

	int m_maxCompressThreads;
	int	m_fixSpaceInUID;
	int	m_doCompressedCache;
	int m_dbConnectionPool;

	int m_useModalityLUT;  

	
	void LogVersionInfo(void);

	
	int m_networkCapture;
	int m_numberOfNetworkCapture;
	int m_messageDump;

	//	Very dangerous, but necessary for now 
	//		will override m_changeDuplicateUIDs
	int m_overwriteDuplicates;

	//	- Force MR, CT, SC to use a specific transfer syntax
	//		This is only valid for non-encapsulated transfer syntaxes:
	//		0 = do NOT force transfer syntax
	//		1 = Implicit Little Endian
	//		2 = Explicit Little Endian
	//		3 = Explicit Big Endian
	int	m_forceTransferSyntax;
	int m_enableMiniDumpTrigger;
	
	int m_enableAutoClean;

	//	
	//	New configs for DICOM Network Gateway product

	//	0 -> no local storage
	//	1 -> local storage (DEFAULT)
	int m_localStorage;

	//	N = 0 -> lossless
	//	N = 1 -> no compression
	//	N > 1 -> N:1
	int m_compressionRatio;

	//	Default - same as input
	char m_mediaStorageTransferSyntaxUID[65];

	//	Default - same as input
	char m_autoRouteTransferSyntaxUID[65];

	//	0 -> No cache validation
	//	1 -> New (non-cacheReader) validation
	//	2 -> Old (cacheReader) validation
	int m_validateCache;
	int m_keepBadCache;

	int m_autoRouteRetries;
	int m_autoRouteTimeout;

	int m_licenseCheckSleepSeconds;
	int m_licenseCheckInitialState;

	int m_decompressOnForceILE;
	int m_alsoDecompressLossyOnForceILE;

	int m_queueSendJobs;

	int m_bypassDatabaseQuery;
	int m_bypassDatabaseInsert;


	//YZ. Centralize server name. 
	char m_serverNameStr[32];
//	char* m_logFilename1;
//	char* m_mergeLog1; 
	char m_mergeCap[256]; 

	int m_requireHigherLevelKeys;

	int m_queryRIS;

	// 
	// 0 -> Do not create cache for CR image
	// 1 -> Create cache for CR image
	int m_writeCacheForCRImage;

	//	where to send data if no rules apply
	char m_defaultRoutingTargetAE[16 + 1];
	char m_defaultRoutingTargetIP[32];
	int m_defaultRoutingTargetPort;
	int m_defaultRoutingCompressionMethod;
	int m_defaultRoutingCompressionFactor;

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
	int m_hangCMove;

	int m_autoBumpLogLevel;
	int m_useStandardCodecs;

	int m_numOfRetriesGettingIPAddress; // 
	//AutoRoutingの(Block)チェックタイミング
	int m_seriesCompleteAutoRoutingTimeout;
	//
	int m_AutoRoutingBlockSize;
	int	m_AutoRoutingTrig; //#21 2012/05/29 K.Ko
#define AutoRouringTirg_Timeout 1
#define AutoRouringTirg_AssociationClose 2
#define AutoRouringTirg_BlockSize 3
	int m_useDentalCT_AE ;
	int m_useJPEGGateway_AE;//#70
	//
	int m_safetyCheck;////#27 2012/06/14 K.Ko

	//#60 2013/07/03
	int m_SqlTopNumber; //default 0: none
	int m_SqlOrderByStudyDate;//default 0 none,1: ASC,2: DESC
	//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
	int m_listFromSqlAlwaysUTF8;//default 0: not 1: yes

};

#endif // AUTO_VOX_CONFIG_FILE_H