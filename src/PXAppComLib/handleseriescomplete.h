/***********************************************************************
 * $Id: HandleSeriesComplete.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *
 *
 *	
 *  
 *-------------------------------------------------------------------
 */

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#ifndef HANDLE_COMPLETED_SERIES_H
#define HANDLE_COMPLETED_SERIES_H

#include <vector>
#include "PxDB.h"
#include "RTVDiCOMService.h"
#include "rtvthread.h"


typedef std::vector<std::string> SOPINSTANCE_LIST;
typedef std::map<int,SOPINSTANCE_LIST> SOPINSTANCE_BY_GROUP;

struct StudySeries
{
	char m_studyInstanceUID[65];
	char m_seriesInstanceUID[65];
	char m_date[64];
};


class HandleSeriesComplete : public iRTVThreadProcess
{
public:

	HandleSeriesComplete()
	{
		m_routingLogFile[0] = 0;
		m_processorName = "HandleSeriesComplete";
		m_logLevel = 0;
		m_networkCapture = 0;
		m_autoBumpLogLevel = 0;
	}

	virtual ~HandleSeriesComplete(){}

	int Process(void);
	int HandleDataFromFile(int* CBdataSizePtr, void** CBdataBufferPtr, int CBisFirst, int* CBisLastPtr);

	///******************************
	DiCOMConnectionInfo m_connectInfo;
	char m_watchedDir[MAX_PATH];
	char m_cacheDir[MAX_PATH];
	DicomObjectType m_objectType;
	int m_doCompressedCacheWrite;
	int m_applicationID;
	char m_seriesInstanceUID[68];
	bool m_isDeltaReport;
	char m_studyInstanceUID[68];
	int m_seriesLevelObjectID;
	int m_studyLevelObjectID;
	unsigned long   m_CBfileSize;
	char  m_CBfileName[MAX_PATH];
	char* m_CBfileBuf;
	DWORD m_lastActiveTime;
	int m_queryRIS;
	char m_defaultRoutingTargetAE[68];
	char m_routingLogFile[MAX_PATH];
	int m_validateCache;
	char  m_defaultRoutingTargetIP[68];
	int m_defaultRoutingTargetPort;
	int m_defaultRoutingCompressionMethod;
	int m_seriesCompleteTimeout;
	int m_defaultRoutingCompressionFactor;
	int m_queueSendJobs;
	int m_dicomServerLogLevel;
	int m_isGate;
	int m_logLevel;
	int m_networkCapture;
	int m_autoBumpLogLevel;

private:

	void ExtractCaScore();
	
	int GetListOfApplicableTagFilterIDs(std::map<int, int>& oFilterIDMap, std::vector<std::string>& oRoutingLogInfo);
	int GetDuplicateReportInfos(std::vector<StudySeries>& oVal);
	int CreateAndSubmitPrefetchJob(KVP_MAP& iPrefetchArgs, std::vector<KVP_MAP>& iRemoteAEs);
	int CreateAndSubmitPrintJob(std::string iPrinterName, std::string iDisplayMode, int iskipN);
	int AuditLogReceivedSeries(void);
	int AuditLogSendSeries(ApplicationEntity& iRemoteAE);
	int AuditLogAssignSeries(const char* iSeriesUID, int iGroupID);
	int MakeDicomServerObject(void);
	int MakeRemoteAEObject(ApplicationEntity& iRemoteAE);
	int MakeOutboundAEObject(void);
	int MakeLocalAEObject(void);

	

	void ExecuteReportGrabber();
	int MakeGroupObject(int groupID, UserGroup& oGroup);
	void Kick(void) {m_lastActiveTime = GetTickCount();};

	void PraseTagFiltersForDataProcessingPatterns(std::map<int, int> &iIDsOfFiltersThatPassed);
	int CreateAndSubmitPEJob(std::string& iPEName, std::string& iStudyName, std::string& iSeriesName, std::vector<std::string>& iSopInstanceList);
	int CreateAndSubmitPEJob(std::string& iPEName, std::string& iStudyInstanceUId, std::string& iSeriesInstanceUID);
	void GetStudyUID(std::string &oStudyUID);
	void GetSOPInstanceListByGroup(SOPINSTANCE_BY_GROUP &iSopInstanceByGroup);
	BOOL DetermineIfJobShouldBeCreated(std::vector<std::string> &iPEList, std::string iPEName);
	
	
	CPxDB m_db;
	std::map<std::string,std::string> m_progressInfoMap;

	bool CheckCache(int& oCached, int& onFrames);
};

#endif