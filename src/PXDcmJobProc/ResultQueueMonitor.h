/***********************************************************************
 * StoreSCUSeriesDirMonitor.h
 *---------------------------------------------------------------------
 *	
 *  
 *-------------------------------------------------------------------
 */
#ifndef RESULT_QUEUE_MONITOR_H
#define RESULT_QUEUE_MONITOR_H

#include "PxNetDB.h"
#include "rtvPoolAccess.h"
//#include "RTVDiCOMService.h"
#include "IntervalProcessor.h"
 
class CPxQueueProc;
class CResultQueueMonitor : public CIntervalProcessor //iRTVThreadProcess//InactiveHandler
{
public:
	static CResultQueueMonitor &theResultQueueMonitor();
	CResultQueueMonitor() ;
	
	~CResultQueueMonitor() ;

	void setupSQLiteDBDir(const std::string &folderName){ m_SQLiteDB_Dir = folderName;};
#if 0

	int Process(void);

	virtual void Kick(void) {m_lastActiveTime = GetTickCount();};
	virtual bool IsTimeOver(DWORD TickCount);
	virtual void ForceTimeOut();
#endif
 
	
/////////////
	static void selfTest();
	
	 
protected:
	//////////////////////
	
	virtual int doProcess();
  
 unsigned long	m_lastActiveTime;

 CPxQueueProc *m_ReusltQueue;
 std::string m_SQLiteDB_Dir;
};

#endif // STORESCU_SERIES_DIR_MONITOR_H

