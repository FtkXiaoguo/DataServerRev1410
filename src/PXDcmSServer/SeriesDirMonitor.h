/***********************************************************************
 * SeriesDirMonitor.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Monitor incoming DICOM series and convert to Vox format when 
 *		timeout has elapsed with no images being added to the series 
 *		directory.
 *
 *	
 *  
 *-------------------------------------------------------------------
 */
#ifndef SERIES_DIR_MONITOR_H
#define SERIES_DIR_MONITOR_H

#include "PxNetDB.h"
#include "rtvPoolAccess.h"
#include "RTVDiCOMService.h"


class JobInfoPublisher;
class  AutoRoutingAEMan ;//#17 2012/05/11 K.Ko

class SeriesDirMonitor : public InactiveHandler
{
public:
	SeriesDirMonitor(const DiCOMConnectionInfo& iCInfo, const char* iPatientID, const char* dirName, 
		const char* cacheDir, DicomObjectType iType, const char* iSeriesUID, bool doCompress);
	~SeriesDirMonitor();
	int Process(void);

	void forceComplte();//#21 2012/05/29 K.Ko

	virtual void Kick(void) {m_lastActiveTime = GetTickCount();};
	virtual bool IsTimeOver(DWORD TickCount);
	virtual void ForceTimeOut();
	const char* GetSeriesInstanceUID() const { return m_seriesInstanceUID; }	
	const char* GetWatchDir() {return m_watchedDirectory;}
 	const char* GetCacheDir() {return m_cacheDir;}
	const DiCOMConnectionInfo& GetConnectionInfo(void) const { return m_connectInfo; }
//	bool CheckCache(int& oCached, int& onFrames);
//	int CreateAndSubmitCacheWriteJob(const char* pDicomDir);
	
	enum
	{
		kInProcessSeriesCompletion =1,
		kOutOfProcessSeriesCompletion,
		kInProcessSeriesCompletionThreaded
	};

	void InitInstanceMap(void);
	bool AddInstanceUID(const char* iUID);
	void RemoveInstanceUID(const char* iUID) {m_processInstanceMap.Remove(iUID);}
	void ClearInstanceMap(void) {m_processInstanceMap.Clear();}
	
	//#17 2012/05/11 K.Ko
	AutoRoutingAEMan *getAutoRoutingMan() const { return m_AutoRoutingMan ;};
	
	//#20 SeriesñàÇ…AutoRoutingÇçsÇ§Å@2012/05/23Å@K.KO
	bool tryAutoRoutingOnSeriesComplete();
protected:
	
	void DoInProcessSeriesCompletion();
	void DoOutOfProcessSeriesCompletion();
	void CreateJobInfoPublisher(int iNumberOfInstances);
	int  HandleCache(void);

	int m_seriesLevelObjectID;
	int m_studyLevelObjectID;
	DWORD	m_lastActiveTime;	
	
	char m_fixedPatientID[126];
	char m_watchedDirectory[MAX_PATH];
	char m_cacheDir[MAX_PATH];
	char m_seriesInstanceUID[kVR_UI];
	DicomObjectType m_objectType;

	char  m_CBfileName[MAX_PATH];
	char* m_CBfileBuf;
	unsigned long   m_CBfileSize;
	bool  m_doCompressedCacheWrite;

	CPxDcmDB m_db;

	bool m_isDeltaReport;
	std::map<std::string,std::string> m_progressInfoMap;
	DiCOMConnectionInfo m_connectInfo;
	
	RTVMapAccess<std::string, int> m_processInstanceMap;
	long m_instanceMapInitFlag;
#ifdef USE_JOBPUBLISH
	JobInfoPublisher* m_pPublisher;
#endif


	bool m_SereisCompleteProcessed;

	AutoRoutingAEMan *m_AutoRoutingMan;//#17 2012/05/11 K.Ko

	bool m_forceComplteFlag;//#21 2012/05/29 K.Ko
};

#endif // SERIES_DIR_MONITOR_H

