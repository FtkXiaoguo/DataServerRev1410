/***********************************************************************
 * CResultQueueMonitor.cpp
 *---------------------------------------------------------------------
 *	
 *
 *	
 *  
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "ResultQueueMonitor.h"

 
/////
  #include "PxQueue.h"

 
#include "AqCore/TRPlatform.h"
//#include "TerareconCacheWriter.h"
//#include "TRCompressedCacheWriter.h"
//#include "JobControl.h"
//#include "JobInfo.h"
#include "AqCore/AqString.h"

#include "Globals.h"
//The define is commented to enable controlling the cache writing using the gConfig.m_writeCache flag
// Murali - 2006.01.05
//#ifdef _EX_CACHE_WRITER
//#include "RTVCacheWriter.h"
//#endif

CResultQueueMonitor &CResultQueueMonitor::theResultQueueMonitor()
{
	static CResultQueueMonitor p; // the signle PxQueueWatchProc object
	return p; 
}

   
//-----------------------------------------------------------------------------------------------------
//
CResultQueueMonitor::CResultQueueMonitor()
{
 
	

	m_ReusltQueue = 0;
	 
}

//-----------------------------------------------------------------------------------------------------
//
CResultQueueMonitor::~CResultQueueMonitor()
{
 
	if(m_ReusltQueue) delete m_ReusltQueue;
	 
}



int CResultQueueMonitor::doProcess()
{
	gLogger.LogMessage(kDebug,"CResultQueueMonitor::doProcess -- start  \n");
	gLogger.FlushLog();
 
	 
	if(!m_ReusltQueue){
		m_ReusltQueue = new CPxResultQueue;
	//	std::string db_file_name = m_SQLiteDB_Dir + "\\" + SQLITE_DB_FILTE;
	//	m_ReusltQueue->setupDBName(db_file_name.c_str());
		 
		m_ReusltQueue->initRes();
	}
	((CPxResultQueue*)m_ReusltQueue)->deleteFinishedEntryWithMaxLen(gConfig.m_resultQueueMaxSize);

	gLogger.LogMessage(kDebug,"CResultQueueMonitor::doProcess -- end  \n");
	gLogger.FlushLog();

	return 1;
}
