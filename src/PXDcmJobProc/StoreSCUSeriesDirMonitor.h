/***********************************************************************
 * StoreSCUSeriesDirMonitor.h
 *---------------------------------------------------------------------
 *	
 *  
 *-------------------------------------------------------------------
 */
#ifndef STORESCU_SERIES_DIR_MONITOR_H
#define STORESCU_SERIES_DIR_MONITOR_H

#include "PxNetDB.h"
#include "rtvPoolAccess.h"
//#include "JobProcBgProcessor.h"
#include "IntervalProcessor.h"
#include "RTVDiCOMService.h"

#include "CStoreSCUIf.h"

#if 0
class CStoreSCUSeries
{
public:
	void clear(){
		m_associationID = -1;
	};
	 
	time_t m_lastTime;
	DiCOMConnectionInfo  m_connectInfo;
	int m_associationID;
};
#endif

typedef std::map<std::string /*seriesUID*/,CStoreSCUIf*> StoreSCUSeriesMap;

class CStoreSCUSeriesDirMonitor : public CIntervalProcessor//iRTVThreadProcess// InactiveHandler
{
public:
	class CLockSeriesDirMonitor
	{
	public:
		CLockSeriesDirMonitor(CStoreSCUSeriesDirMonitor &h){
			m_target = &h;
			m_target->m_AESeriesListMapAccess.m_cs.Enter();
		}
		~CLockSeriesDirMonitor(){
			 
			m_target->m_AESeriesListMapAccess.m_cs.Leave();
		}
	protected:
		CStoreSCUSeriesDirMonitor *m_target;
	};
	static CStoreSCUSeriesDirMonitor &theStoreSCUSeriesDirMonitor();
	CStoreSCUSeriesDirMonitor(){};
	~CStoreSCUSeriesDirMonitor();

	CStoreSCUSeriesDirMonitor(const DiCOMConnectionInfo& iCInfo, const char* iPatientID, const char* dirName, 
		const char* cacheDir, DicomObjectType iType, const char* iSeriesUID, bool doCompress);
	
#if 0
	int Process(void);

	virtual void Kick(void) {m_lastActiveTime = GetTickCount();};
	virtual bool IsTimeOver(DWORD TickCount);
	virtual void ForceTimeOut();
#endif
	
//	void lock();
//	void unlock();

	const DiCOMConnectionInfo& GetConnectionInfo(void) const { return m_connectInfo; }
//	bool CheckCache(int& oCached, int& onFrames);
//	int CreateAndSubmitCacheWriteJob(const char* pDicomDir);
	
	enum
	{
		kInProcessSeriesCompletion =1,
		kOutOfProcessSeriesCompletion,
		kInProcessSeriesCompletionThreaded
	};

 
	
/////////////
	static void selfTest();
	
	CStoreSCUIf *getSotreSCU(const std::string &AE,const std::string &seriesUID);
	void insertCSotreSCU(const std::string &AE,const std::string &seriesUID,CStoreSCUIf *storeSCU);
	CStoreSCUIf *deleteCSotreSCU(const std::string &AE,const std::string &seriesUID);
protected:


	virtual int doProcess();

	//////////////////////
 	StoreSCUSeriesMap & getStoreSCUSeriesMap(const std::string &AE);
	CStoreSCUIf *getCStoreSCUFromStoreSCUSeriesMap(const std::string &seriesUID, StoreSCUSeriesMap &map);
//	void insertStoreSCUSeriesMap(const std::string &AE,StoreSCUSeriesMap *map);

	void closeFinishedSeries();
	///////////////////////
	void DoInProcessSeriesCompletion();
	void DoOutOfProcessSeriesCompletion();
	void CreateJobInfoPublisher(int iNumberOfInstances);
	int  HandleCache(void);

	int m_seriesLevelObjectID;
	int m_studyLevelObjectID;
	DWORD	m_lastActiveTime;	
	
	char m_fixedPatientID[126];


	char  m_CBfileName[MAX_PATH];
	char* m_CBfileBuf;
	unsigned long   m_CBfileSize;
	bool  m_doCompressedCacheWrite;

	CPxDcmDB m_db;

	bool m_isDeltaReport;
	std::map<std::string,std::string> m_progressInfoMap;
	DiCOMConnectionInfo m_connectInfo;
	

	bool m_SereisCompleteProcessed;

////////////
	RTVMapAccess<std::string, int> m_AESeriesListMapAccess;
	std::map<std::string /*AE*/,StoreSCUSeriesMap> m_AESeriesMap;


};

#endif // STORESCU_SERIES_DIR_MONITOR_H

