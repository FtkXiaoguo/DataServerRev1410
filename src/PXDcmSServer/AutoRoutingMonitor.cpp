/***********************************************************************
 * CAutoRoutingMonitor.cpp
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
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "AutoRoutingMonitor.h"
 

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

#include "AutoRoutingAEMan.h" //#17 2012/05/11 K.Ko

 
CAutoRoutingMonitor& CAutoRoutingMonitor::theAutoRoutingMonitor()
{
	static CAutoRoutingMonitor p; // the signle manager object
	return p;
}
//-----------------------------------------------------------------------------------------------------
//
CAutoRoutingMonitor::CAutoRoutingMonitor()
{
	 
	  
}

//-----------------------------------------------------------------------------------------------------
//
 

//-----------------------------------------------------------------------------------------------------
//
CAutoRoutingMonitor::~CAutoRoutingMonitor()
{ 

}

 
AutoRoutingAEMan * CAutoRoutingMonitor::getInactiveHandler(const std::string &id)
{
	AutoRoutingAEMan *ret_v = 0;

	InactiveHandlerMap::iterator iter;

	for (iter=m_inactiveHandlerMap.begin(); iter != m_inactiveHandlerMap.end(); iter++)
	{
		if(iter->first == id){
			ret_v = (AutoRoutingAEMan*)(iter->second);
			break;
		}
		iter++;
	}
	return ret_v;
}
//-----------------------------------------------------------------------------------------------------
//
int CAutoRoutingMonitor::Process(void)
{
	if(gConfig.m_AutoRoutingTrig != AutoRouringTirg_BlockSize){
	//それ以外は実行しない。
		return 0;
	}

	InactiveHandlerMap::iterator iter;
	bool fired = false;
	AutoRoutingAEMan* handler;
	std::string id;
	DWORD TickCount;

	// initialize COM for ADO 
	AqCOMThreadInit comInitGuard;

	ResetEvent(m_wakeupEvent);
	CIntervalProcessor::setIntervalTime(m_napTime);

	bool removeIt = false;
	TRCSLock fplock(&m_cs, false);
	while(!TerminationRequested())
	{	
		m_inProcess = 0;
		fired = false;
	//	fplock.Lock();
		TickCount = GetTickCount();

		for (iter=m_inactiveHandlerMap.begin(); iter != m_inactiveHandlerMap.end(); iter++)
		{
			handler = (AutoRoutingAEMan*)(iter->second);
			if(!handler)
				continue;

			if(handler->IsTimeOver(TickCount))
			{
				handler->Process();
				fired = true;
			}
			if(handler->getStopFlag( ))
			{
				 handler->doLast( );
				
				id = iter->first;

				fplock.Lock();
				m_inactiveHandlerMap.erase(id);
				fplock.Unlock();  
				delete handler;
				fired = true;
				break;
			}
#if 0
			//printf("m_amap[handler] = %d\n", m_amap[handler]);
			//Sleep(1000);
			if(handler->CanStop(TickCount))
			{
				id = iter->first;
				
				fplock.Unlock(); // give others a chance to kick, the iterator may invalid

				m_inProcess = 1;
				handler->m_inProcess = 1;
				removeIt = false;
				if(handler->Process() == 0)
				{
					fplock.Lock();
					//check if it is kicked when in processing
					if(handler->CanStop(TickCount))
					{
						m_inactiveHandlerMap.erase(id);
						removeIt = true;
					}
					fplock.Unlock(); // give others a chance to kick, the iterator may invalid
					
					// we owe the object by porinter, so delete it when finished using
					if(removeIt)
						delete handler, handler=0; 
					
				}
				else
				{
					// keep the handle in case process fail
					handler->m_inProcess = 0;
				}
				m_inProcess = 0;
				if(handler)
					handler->m_inProcess = 0;
				fired = true;
				break;
			}
#endif
		}

	 
//		fplock.Unlock();
		 
		Sleep(0); // yield to other threads
		// no one timed out, take a nap
		if(!fired) 
		{
	//		Sleep((int)m_napTime);
			WaitForProcessToEnd(m_intervalTime);
		}
		
	}
	return 0;
 }

int CAutoRoutingMonitor::doProcess()
{
	printf(">>>CResultQueueMonitor::doProcess \n");

 
//	 tryAutoRouting();

	return 1;
}

 
#if 0

//Series毎にAutoRouting　2012/05/23　K.KO
bool CAutoRoutingMonitor::tryAutoRouting()
{

	 
	if(!m_AutoRoutingMan) {
		return false;
	}
	return  m_AutoRoutingMan->tryAutoRouting();
	 
}
#endif