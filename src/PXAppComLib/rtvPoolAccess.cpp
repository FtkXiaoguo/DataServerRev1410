/***********************************************************************
 * rtvPoolAccess.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is served as a template for objects pool access 
 *      control in a multip threads environment.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

//-----------------------------------------------------------------------------
#include "AqCore/TRPlatform.h"
#include "rtvPoolAccess.h"
#include "PxDB.h"
#include "AqCore/AqCore.h"

void InactiveHandler::KeepIt(void* iLocker, bool iKeep)
{
	if(iKeep)
		m_keepMap.Add(iLocker, 1, true);
	else 
		m_keepMap.Remove(iLocker);
}


RTVInactiveManager& RTVInactiveManager::theManager()
{
	static RTVInactiveManager p; // the signle manager object
	return p;
}

RTVInactiveManager::RTVInactiveManager():m_napTime(3000)
{
	m_inProcess = 0;
}

// timeout all managed objects then cleanup
RTVInactiveManager::~RTVInactiveManager()
{
	if (!TerminationRequested())
	RequestTermination(1);
	
	if(WaitForProcessToEnd(4000) == kNoError)
	{

		TRCSLock fplock(&m_cs);
		InactiveHandlerMap::iterator iter;
		for (iter=m_inactiveHandlerMap.begin(); iter != m_inactiveHandlerMap.end(); ++iter)
		{
			iter->second->ForceTimeOut();
			//delete handler;      //we owe the object by porinter, so delete it when finished using
		}
		m_inactiveHandlerMap.clear();
	}

}


bool RTVInactiveManager::Has(const char* id) 
{
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	InactiveHandlerMap::iterator iter = m_inactiveHandlerMap.find(id);
	return (iter != m_inactiveHandlerMap.end())?true:false;
}

// take the ownershih of the object, and let managed object updates it's status
void RTVInactiveManager::Handover(const char* id, InactiveHandler* ph)
{
	if(m_processStatus != kRunning)
	{
		// no process, can not take any
		delete ph;
		return;
	}

	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	InactiveHandlerMap::iterator iter = m_inactiveHandlerMap.find(id);
	if (iter != m_inactiveHandlerMap.end())
	{
		if(ph != iter->second)
		{
			delete ph;
			ph = iter->second;
		}
	}
	else
	{
		m_inactiveHandlerMap[id] = ph;
	}
	ph->Kick(); // there is a watched one, just kick it
}

InactiveHandler* RTVInactiveManager::LockHandler(const char* id, void* iLocker, bool lockIt)
{
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	InactiveHandlerMap::iterator iter = m_inactiveHandlerMap.find(id);

	if(iter != m_inactiveHandlerMap.end()) 
	{
		InactiveHandler* p = iter->second;
		p->Kick();
		p->KeepIt(iLocker, lockIt);
		return p;

	}
	return 0;

}


// let managed object updates it's status
bool RTVInactiveManager::Kick(const char* id) 
{
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	InactiveHandlerMap::iterator iter = m_inactiveHandlerMap.find(id);

	if(iter != m_inactiveHandlerMap.end()) 
	{
		iter->second->Kick();
		return true;
	}
	return false;
}


int RTVInactiveManager::Size()
{
	TRCSLock fplock(&m_cs);
	int size = m_inactiveHandlerMap.size();
	if (size == 0 && m_inProcess != 0)
		size = 1;
	return size;
}

//long running thread to give every managed objects a chance to check time out status
int RTVInactiveManager::Process(void)
{
	InactiveHandlerMap::iterator iter;
	bool fired = false;
	InactiveHandler* handler;
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
		fplock.Lock();
		TickCount = GetTickCount();

		for (iter=m_inactiveHandlerMap.begin(); iter != m_inactiveHandlerMap.end(); iter++)
		{
			handler = iter->second;
			if(!handler)
				continue;

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
		}

		if(!fired) 
		{
			fplock.Unlock();
		}
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


