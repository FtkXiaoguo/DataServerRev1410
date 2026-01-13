/***********************************************************************
 * CStoreSCUSeriesDirMonitor.cpp
 *---------------------------------------------------------------------
 *	
 *
 *	
 *  
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "StoreSCUSeriesDirMonitor.h"


/////
#include "DataMonitor.h"

#include "HandleSeriesComplete.h"
#include "AqCore/TRPlatform.h"
#include "AppComCacheWriter.h"
#include "TRCompressedCacheWriter.h"
#include "JobControl.h"
#include "JobInfo.h"
#include "AqCore/AqString.h"

#include "Globals.h"
//The define is commented to enable controlling the cache writing using the gConfig.m_writeCache flag
// Murali - 2006.01.05
//#ifdef _EX_CACHE_WRITER
#include "RTVCacheWriter.h"
//#endif


#include "CStoreSCU.h"

enum
{
	kValidateCacheWithoutCacheReader = 1,
	kValidateCacheWithCacheReader = 2,
};

const char* kAQNetCacheWriter = "AQNetCacheWriter";


//-----------------------------------------------------------------------------------------------------
//
CStoreSCUSeriesDirMonitor::CStoreSCUSeriesDirMonitor(const DiCOMConnectionInfo& iCInfo, const char* iPatientID, const char* dirName,
		 const char* cacheDir, DicomObjectType iType, const char* iSeriesUID, bool doCompress)
{
	

	m_lastActiveTime = 0;
	m_connectInfo = iCInfo;
	m_isDeltaReport = false;
	 
 
	
	//m_processorName = "Series Dir Monitor";	
	m_doCompressedCacheWrite = doCompress;
 

	// add prefix to patient ID for classify out from series UID
	m_fixedPatientID[0] = 0;

#if 0
	//#13 
	//DataMonitor 起動しない
	if(iPatientID && iPatientID[0])
	{
		AqString aString = iPatientID;
		aString.TrimRight(); aString.TrimLeft();
		
		snprintf(m_fixedPatientID, 125, "_~!@PatientID@%s", aString);
		RTVInactiveManager& imanger=RTVInactiveManager::theManager();

		if(imanger.LockHandler(m_fixedPatientID, this, true) == 0) // no data for this patient
		{
			//Launch Data Monitor and hook up data monitor for this one
			DataMonitor* pDataMonitor = new DataMonitor(iCInfo, iPatientID, m_seriesInstanceUID);
			
			//let RTVInactiveManager owns monitor now
			imanger.Handover(m_fixedPatientID, pDataMonitor);
			imanger.LockHandler(m_fixedPatientID, this, true);
		}
		else
		{
			GetAqLogger()->LogMessage(kInfo,"INFO:(%d|%X) lock DataMonitor(%s) on: %s for series %s from %s\n", 
				m_connectInfo.AssociationID, this, m_fixedPatientID, iPatientID, m_seriesInstanceUID, m_connectInfo.RemoteHostName);
		}
	}
#endif

#ifdef USE_JOBPUBLISH
	m_pPublisher = 0;
#endif
	m_SereisCompleteProcessed = false;
}

//-----------------------------------------------------------------------------------------------------
//
void CStoreSCUSeriesDirMonitor::CreateJobInfoPublisher(int iNumberOfInstances)
{
 
}

//-----------------------------------------------------------------------------------------------------
//
CStoreSCUSeriesDirMonitor::~CStoreSCUSeriesDirMonitor()
{
	if(m_fixedPatientID[0] != 0)
	{
		// let data monitor off the hook for this one	
		RTVInactiveManager& imanger=RTVInactiveManager::theManager();
		imanger.LockHandler(m_fixedPatientID, this, false);
	}
	
	 
}


//-----------------------------------------------------------------------------------------------------
//


//-----------------------------------------------------------------------------------------------------
//
 


#if 0
//-----------------------------------------------------------------------------------------------------
//
bool CStoreSCUSeriesDirMonitor::IsTimeOver(DWORD TickCount)
{
#if 1
	long overtime = (TickCount-m_lastActiveTime)/1000 - gConfig.m_seriesCompleteTimeout;

#if 0
	if (overtime > 3600)
	{
 		m_keepMap.Clear(); // clear locks for 1 hour old lock
		return true;
	}
	else
		return (overtime > 0);
#else
	return (overtime > 0);
#endif

#else
	return true;
#endif


}

//-----------------------------------------------------------------------------------------------------
//
void CStoreSCUSeriesDirMonitor::ForceTimeOut(void)
{	
	 
}
 
//-----------------------------------------------------------------------------------------------------
//
int CStoreSCUSeriesDirMonitor::Process(void)
{	
	int recode = 1;
	while(!TerminationRequested())
	{
	//	IsTimeOver(GetTickCount());
		::Sleep(gConfig.m_seriesCompleteTimeout*1000);
		//
		m_lastActiveTime = GetTickCount();
		int recode = 1;
	  
		printf(">>>CStoreSCUSeriesDirMonitor::Process \n");

		closeFinishedSeries();

	}
	return recode;
}
 

#endif

//-------------------------------------------------------------------
//
static inline bool IsLineBreak(int c) { return c=='\r' || c=='\n';}
int GetValidLine( FILE* fp, char* oBuf, int ioBufLen)
{
	int i, c;

	for (ioBufLen--, i = 0; (c = getc(fp)) != EOF && !IsLineBreak(c) && i < ioBufLen;)
		oBuf[i++] = c;
	oBuf[i] = '\0';
	return c;
}

//-----------------------------------------------------------------------------------------------------
//
int HowManySOPsInCache(FILE* fp)
{
	char lineBuf[200];

	int sopCountInCache = 0;
	while (GetValidLine(fp, lineBuf, sizeof lineBuf) != EOF)
	{
		lineBuf[strlen("SOPInstanceUID")] = 0;
		if (!strcmp(lineBuf,"SOPInstanceUID"))
			 sopCountInCache++;
	}
	
	return sopCountInCache;
}

//-----------------------------------------------------------------------------------------------------
//
 
 

 
int CStoreSCUSeriesDirMonitor::doProcess()
{

	gLogger.LogMessage(kDebug,"CStoreSCUSeriesDirMonitor::doProcess -- start  \n");
	gLogger.FlushLog();

	closeFinishedSeries();

	gLogger.LogMessage(kDebug,"CStoreSCUSeriesDirMonitor::doProcess -- end  \n");
	gLogger.FlushLog();
	return 1;
}


 

//////////////////////
void CStoreSCUSeriesDirMonitor::selfTest()
{
	CStoreSCUSeriesDirMonitor AEList;

	std::string testAE1= "tt";
#if 0
	StoreSCUSeriesMap *se_map= AEList.getStoreSCUSeriesMap(testAE1);
	if(!se_map){
		se_map = new StoreSCUSeriesMap;
		se_map->clear();
	
		AEList.insertStoreSCUSeriesMap(testAE1,se_map);
	}
#endif
	DiCOMConnectionInfo connectionInfo;
	CStoreSCUIf *new_se = new CStoreSCU(connectionInfo);
//	(*se_map)["12.0"] = new_se;
	AEList.getStoreSCUSeriesMap(testAE1)["12.0"] = new_se;
	new_se->setLastTime(time(0));

//	se_map= AEList.getStoreSCUSeriesMap(testAE1);

	CStoreSCUIf *sup_series = AEList.getSotreSCU(testAE1,"12.0");

	 
	sup_series = AEList.getSotreSCU(testAE1,"12.0.0");

	sup_series = AEList.getSotreSCU("tt1","12.0");
	int xx=0;

}

CStoreSCUSeriesDirMonitor &CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor()
{
	static CStoreSCUSeriesDirMonitor p; // the signle PxQueueWatchProc object
	return p; 
}

#if 1
StoreSCUSeriesMap & CStoreSCUSeriesDirMonitor::getStoreSCUSeriesMap(const std::string &AE)
{
	TRCSLock fplock(&m_AESeriesListMapAccess.m_cs);

#if 0
	StoreSCUSeriesMap *ret_ptr = 0;
	std::map<std::string,StoreSCUSeriesMap*>::iterator it;

	it = m_AESeriesMap.begin();
	while(it!=m_AESeriesMap.end()){
		if(it->first == AE){
			ret_ptr = it->second;
			break;
		}
		it++;
	}
 
	
	return ret_ptr;
#else
	return m_AESeriesMap[AE];
#endif
}
#endif

#if 0
void CStoreSCUSeriesDirMonitor::insertStoreSCUSeriesMap(const std::string &AE,const StoreSCUSeriesMap *map)
{
	TRCSLock fplock(&m_AESeriesListMapAccess.m_cs);
	m_AESeriesMap[AE] = map;

}
#endif

CStoreSCUIf *CStoreSCUSeriesDirMonitor::getSotreSCU(const std::string &AE,const std::string &seriesUID)
{
	TRCSLock fplock(&m_AESeriesListMapAccess.m_cs);

	StoreSCUSeriesMap &seriesMap = getStoreSCUSeriesMap(AE);
	if(seriesMap.size()<1){
		return 0;
	}

#if 0
	CStoreSCUIf *ret_ptr = (*seriesMap)[seriesUID];
#else
	CStoreSCUIf *ret_ptr =  getCStoreSCUFromStoreSCUSeriesMap(seriesUID,seriesMap);

#endif
	//

	return ret_ptr;
}
#if 0
void CStoreSCUSeriesDirMonitor::lock()
{
	m_AESeriesListMapAccess.m_cs.Enter();
}
void CStoreSCUSeriesDirMonitor::unlock()
{
	m_AESeriesListMapAccess.m_cs.Leave();
}
#endif
CStoreSCUIf *CStoreSCUSeriesDirMonitor::getCStoreSCUFromStoreSCUSeriesMap(const std::string &seriesUID,  StoreSCUSeriesMap &seriesMap)
{
	CStoreSCUIf *ret_ptr = 0;
	StoreSCUSeriesMap::iterator it;

	it = seriesMap.begin();
	while(it!=seriesMap.end()){
		if(it->first == seriesUID){
			ret_ptr = it->second;
			break;
		}
		it++;
		 
	}

	return ret_ptr;
}
void CStoreSCUSeriesDirMonitor::insertCSotreSCU(const std::string &AE,const std::string &seriesUID,CStoreSCUIf *storeSCU)
{
	TRCSLock fplock(&m_AESeriesListMapAccess.m_cs);

#if 0
	StoreSCUSeriesMap *seriesMap = getStoreSCUSeriesMap(AE);
	if(!seriesMap){
		 
		seriesMap = new StoreSCUSeriesMap;
		seriesMap->clear();
		insertStoreSCUSeriesMap(AE,seriesMap);
	}

	(*seriesMap)[seriesUID] = storeSCU;
#else
	m_AESeriesMap[AE][seriesUID]  = storeSCU;
#endif

}
CStoreSCUIf *CStoreSCUSeriesDirMonitor::deleteCSotreSCU(const std::string &AE,const std::string &seriesUID)
{
	TRCSLock fplock(&m_AESeriesListMapAccess.m_cs);
	
	CStoreSCUIf *ret_ptr = 0;
	StoreSCUSeriesMap &seriesMap = getStoreSCUSeriesMap(AE);
	if(seriesMap.size()<1){
		;//
	}else{
		ret_ptr =  getCStoreSCUFromStoreSCUSeriesMap(seriesUID,seriesMap);
		if(!ret_ptr){
			;//
		}else{
			seriesMap.erase(seriesUID);
		}
		 
	}

	return ret_ptr;

}

void CStoreSCUSeriesDirMonitor::closeFinishedSeries()
{
	TRCSLock fplock(&m_AESeriesListMapAccess.m_cs);
	
//	StoreSCUSeriesMap *ret_ptr = 0;
	std::map<std::string,StoreSCUSeriesMap>::iterator ae_it;

	ae_it = m_AESeriesMap.begin();
	while(ae_it!=m_AESeriesMap.end()){
		 
		StoreSCUSeriesMap &seriesMap = ae_it->second;
		 
		StoreSCUSeriesMap::iterator series_it;

		series_it = seriesMap.begin();

		{
			bool del_cstoreSCU_flag = false;
			CStoreSCUIf *CStoreSCUPtr=0;
			while(series_it!=seriesMap.end()){
				 
				CStoreSCUPtr = series_it->second;
				if(CStoreSCUPtr->tryToClose(gConfig.m_seriesCompleteTimeout)){
					del_cstoreSCU_flag = true;
					//deleteを行い、series_itのリセットが必要、一旦終了
					break;
				}
				series_it++;
				 
			}
			if(del_cstoreSCU_flag){
				CStoreSCUIf *CStoreSCUP_temp = deleteCSotreSCU(CStoreSCUPtr->getAE(),CStoreSCUPtr->getSeriesUID());
				if(CStoreSCUP_temp){
					delete CStoreSCUP_temp;
				}
				break;
				//at_itのリセットが必要、一旦終了
			}
		}

		ae_it++;
	}

}