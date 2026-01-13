/***********************************************************************
 
 *-------------------------------------------------------------------
 */
#ifndef TEST_DB_DAEMON_CONFIG_FILE_H
#define TEST_DB_DAEMON_CONFIG_FILE_H

#pragma warning (disable: 4503)

#include "rtvloadoption.h"
#include "TestDBDaemon.h"
#include "AQNetConfiguration.h"

 
#define	kMaxAssociationThreads	300

class CDBTools;
//class ConfigWatcher; 

class TestDBDaemonConfigFile : public iRTVOptions // RTVConfigFile
{
public:
	TestDBDaemonConfigFile();
	~TestDBDaemonConfigFile() {}

	void Init();
	void InitKVPMap();
	void ValidateAndFixConfiguration();

	///////////////////
#define TestDB_ThreadMax (16)
	int m_testThreadNum;

	char m_patientName[TestDB_ThreadMax][256];
	int m_testThreadType[TestDB_ThreadMax]; //0: list, 1: update
 
	int m_testBigDbLen;
	////////////////////

//	ConfigWatcher m_configWatcher;

	//	Override rtvbase
	//void LogMessage(int verbose, int location, const char *, ...);

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
	int m_logLocation;
	int m_logMaxSize;
	int m_writeCache;
	int m_inProcessSeriesCompletion;
	int m_threadWrite;

	//	Rob Lewis - 06/20/02 - Need control of these timeouts
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
	// End

	std::vector <AQNetDevice> m_raidDevices;

	//	Japanese PACS don't use this response, so we need to turn it off
	int m_sendErrorForDuplicateUID;

	int m_changeDuplicateUIDs;
	char m_rootUID[65];

	//	Rob Lewis - 4/10/02
	char m_mergeLicense[32];

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

	int m_useModalityLUT; // T.C. Zhao 2003-03-18

	//	T.C. Zhao 2003-02-13
	void LogVersionInfo(void);

	//	Rob Lewis - 3/13/03
	int m_networkCapture;
	int m_numberOfNetworkCapture;
	int m_messageDump;

	//	Rob Lewis - 3/27/03 - Very dangerous, but necessary for now (bug #3496)
	//		will override m_changeDuplicateUIDs
	int m_overwriteDuplicates;

	//	Rob Lewis - 04/011/03 - Force MR, CT, SC to use a specific transfer syntax
	//		This is only valid for non-encapsulated transfer syntaxes:
	//		0 = do NOT force transfer syntax
	//		1 = Implicit Little Endian
	//		2 = Explicit Little Endian
	//		3 = Explicit Big Endian
	int	m_forceTransferSyntax;
	int m_enableMiniDumpTrigger;
	
	int m_enableAutoClean;

	//	Rob Lewis - 12/09/03
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

	char m_bypassHASP[65];

	//YZ. Centralize server name. 
	char m_serverNameStr[32];
	char* m_logFilename1;
	char* m_mergeLog1; 
	char m_mergeCap[256]; 

	int m_requireHigherLevelKeys;

	int m_queryRIS;

	// added by shiying hu, 2005.3.17
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

	// T.C. Zhao 2005.05.19
	// Kumamato workaround
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

	int m_numOfRetriesGettingIPAddress; // T.C. Zhao 2008.07.08
};

#endif // AUTO_VOX_CONFIG_FILE_H