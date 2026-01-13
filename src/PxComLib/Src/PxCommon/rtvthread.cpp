/*************************************************************************
 *---------------------------------------------------------------------
 *---------------------------------------------------------------------
 */

#if !defined( _WIN32_WINNT)
#define _WIN32_WINNT   0x0500
#include "rtvthread.h"
#undef _WIN32_WINNT
#else
#include "rtvthread.h"
#endif

void iRTVThreadProcess::DeleteThread(void) 
{ 
	if (m_theThread) 
		delete m_theThread, m_theThread=0;
}

/******************************************************************
 * ThreadProcess is a processor for threads
 ******************************************************************/

bool iRTVThreadProcess::Stop(int imSec)
{
	if (!m_theThread)
		return true;
	
	m_theThread->Terminate(imSec, false);
	return IsRunning();
}

//------------------------------------------------------------------------------
void iRTVThreadProcess::ForceStop(int imSec)
{
	if (!m_theThread)
		return;
	
	m_theThread->Terminate(imSec, true);
}

//---------------------------------------------------------------------------
iRTVThreadProcess::iRTVThreadProcess(const char* iProcessorName)
{
	m_processorName = (iProcessorName && *iProcessorName)?  iProcessorName : "Unknown";
    m_ended = 0;
	m_threadID = 0;
	m_processStatus = kToBeStarted;
	m_theThread = 0;
	m_processReturnCode = -kToBeStarted; //mark as process has not statred
}

//------------------------------------------------------------------------------
void iRTVThreadProcess::RequestTermination(int iFlag)
{
    m_ended = iFlag;
	if (m_ended == 0 && !IsRunning())
		m_processStatus = kToBeStarted;	// reset
}

//------------------------------------------------------------------------------
int iRTVThreadProcess::WaitForProcessToEnd(int imSec, int iDelta)
{
	if (iDelta <= 0 || iDelta > imSec)
		iDelta = imSec;

	int delta = (imSec > iDelta ? iDelta : imSec), t;

	if (imSec/delta > 1024) delta = (imSec >> 10);
	
	for ( t = 0; t < imSec && IsRunning(); t += delta)
		Sleep(delta);

	return IsRunning() ? kHang:kNoError;
}

//------------------------------------------------------------------------------
iRTVThreadProcess::~iRTVThreadProcess(void)
{
	if (!TerminationRequested())
		RequestTermination(1);
	
	WaitForProcessToEnd(5000);
	
	if (IsRunning())
		ErrMessage("ThreadProcessor (%s) hang in ~iRTVThreadProcess\n", m_processorName);
}


/*******************************************************************
 * Threads
 *******************************************************************/

/*
	kBadProcessor		= -1,	// bad argument to Create()
	kFailedToCreate		= -2,	// can't create the thread	
	kAlreadyCreated		= -3,	// already running
	kPreprocessFailed	= -4	// preprocess() failed
*/

const char* iRTVThread::GetStatusText(int iStatus)
{
	
	switch(iStatus)
	{
	case kOK:
		return "OK";
	case kFailedToCreate:
		return "FailedToCreate";
	case kBadProcessor:
		return "BadProcessor";
	case kPreprocessFailed:
		return "PreProcessFaileD";
	default:
		return "Unknown";
	}
	
	return "Unknown";
}

long iRTVThread::Total = 0;

#if !defined(_WIN32)
/*****   UNIX pthread implementation. UNTESTED!!! *******/

#include "rtviostream.h"
#include <pthread.h>
#include <signal.h>


class iRTVThreadID
{
public:
    iRTVThreadID(void) { m_threadID = 0;}
    pthread_t m_threadID;
};


iRTVThread::iRTVThread(void)
{
    m_process = 0;
    m_thread = new iRTVThreadID;
}

//------------------------------------------------------------------------------
static void *ThreadProcess(void *data)
{
    iRTVThreadProcess *proc = (iRTVThreadProcess *)data;

	if (!proc)
		return;
	
	proc->m_processStatus = iRTVThreadProcess::kRunning;
	
    proc->Message("Thread(0x%x, %s) processor started\n", proc->GetThreadID(), proc->Name());

    proc->m_processReturnCode = proc->Process();
	
	proc->m_processStatus = iRTVThreadProcess::kProcessEnded;

    proc->Message("-Thread (0x%x, %s) processor ended\n",proc->GetThreadID(), proc->Name());

	proc->m_theThread->ProcessDone();
    return 0;
}


//------------------------------------------------------------------------------
int iRTVThread::Create(iRTVThreadProcess *process, int priority, PriorityClass iPC)
{
    int err;

	if (!m_thread)
		return kAlloc;
	
    m_process = process;
	
#if 0
    pthread_attr_t attrib;
    pthread_attr_init(&attrib);
    err = pthread_create(&m_thread->m_threadID, &attrib, 
		ThreadProcess, (void *)m_process);
#else
    err = pthread_create(&m_thread->m_threadID, 0,
		ThreadProcess, (void *)m_process);
#endif
	
	m_process->m_threadID = unsigned int(m_thread->m_threadID);
	m_process->m_theThread = this;

    if(err)
    {
        perror("ThreadCreation");
        ErrMessage("Thread Creation failed\n");
    }
    else
    {
		Message2("Thread (0x%0x) created\n", m_thread->m_threadID);
    }
	
    return err;
}

//------------------------------------------------------------------------------
iRTVThread::iRTVThread(iRTVThreadProcess *process, int iPriority)
{
    m_thread = new iRTVThreadID;
    this->Create(process,iPriority);
}

iRTVThread::~iRTVThread(void)
{
	if (m_thread)
	{
		Terminate();
		delete m_thread;
		m_thread = 0;
	}
}

//------------------------------------------------------------------------------
int iRTVThread::Suspend(int imsec)
{
    return 1;
}

int iRTVThread::Resume(void)
{
    return 0;
}

int iRTVThread::SetPriority(int iFlag)
{
    return 0;
}

int iRTVThread::Terminate(void)
{
	if (!m_thread)
		return kFailed;

    if(m_thread->m_threadID)
    {
		pthread_kill(m_thread->m_threadID, SIGTERM);
    }
	
    m_thread->m_threadID = 0;
	
    return 0;
}

#else
/********************************************************************
* WIN32 threads implementation
*******************************************************************/
#include <process.h>
#include <windows.h>

class iRTVThreadID
{
public:
    iRTVThreadID(void)  { m_threadID = 0;}
	~iRTVThreadID(void) {  m_threadID = 0;}
    HANDLE	m_threadID;
};

iRTVThread::iRTVThread(iRTVThreadProcess *process,int iPriority)
{
	m_thread = new iRTVThreadID;
	m_process = 0;
	this->Create(process,iPriority);
}

//------------------------------------------------------------------------------
static void __cdecl ThreadProcess( void *data)
{
    iRTVThreadProcess *proc = (iRTVThreadProcess *)data;

	if (!proc)
		return;
	
	try
	{	
		proc->m_processStatus = iRTVThreadProcess::kRunning;
		
		proc->Message("Thread(0x%x, %s) processor started\n", proc->GetThreadID(), proc->Name());
		
		InterlockedIncrement(&iRTVThread::Total);	
		proc->m_processReturnCode = proc->Process();
		
		proc->Message("-Thread (0x%x, %s) processor ended\n",proc->GetThreadID(), proc->Name());
		proc->m_threadID = 0;
		
		InterlockedDecrement(&iRTVThread::Total);
		
		proc->Message("Thread(0x%x, %s) processor terminated\n", proc->GetThreadID(), proc->Name());
		
		proc->m_processStatus = iRTVThreadProcess::kProcessTerminated;
		
	} 
	catch (...)
	{
		proc->Message("**Global exception in ThreadProcess in %s\n", proc->Name());
		assert(0);
	}
	
}

//------------------------------------------------------------------------------
void iRTVThread::ProcessDone(void)
{ 
	if(m_thread)
		m_thread->m_threadID = 0;
}

#if defined ( __DENTAL )
HANDLE
iRTVThread::GetThreadHandle( void )
{return m_thread->m_threadID;}
#endif
//------------------------------------------------------------------------------
int iRTVThread::Create(iRTVThreadProcess *process, int iPriority, PriorityClass iPC)
{
	if (!m_thread)
		return m_status = kFailedToCreate;

	m_status = kOK;

	if (!process)
	{
		m_process = 0;
		return m_status = kBadProcessor;
	}

	if (m_thread->m_threadID && process->IsRunning())
	{
		return m_status = kAlreadyCreated;
	}

    m_process = process;

	m_process->m_processStatus = iRTVThreadProcess::kPreProcessing;

	if (m_process->PreProcess() < 0)
	{
		m_process->m_processStatus = iRTVThreadProcess::kFailed;
		return m_status = kPreprocessFailed;
	}

	m_process->m_processStatus = iRTVThreadProcess::kThreadJustCreated;
	
    m_thread->m_threadID = (HANDLE)_beginthread(ThreadProcess, 0,(void *)m_process);
	
    if(m_thread->m_threadID == 0 || long(m_thread->m_threadID)== -1)
    {
        ErrMessage("Thread Creation failed\n");
		m_thread->m_threadID = 0;
		m_process->m_processStatus = iRTVThreadProcess::kThreadFailed;
    }
    else
    {	
		Message2("Thread (0x%0x, %s) created\n",  m_thread->m_threadID,  process->Name());
		SetPriority(iPriority,iPC);
    }

	m_process->m_threadID = unsigned int(m_thread->m_threadID);
	m_process->m_theThread = this;

    return m_status = (m_thread->m_threadID == 0? kFailedToCreate:kOK);
}

//------------------------------------------------------------------------------
int iRTVThread::CreateAndRunProcess(iRTVThreadProcess *process, int iMsec, int iPriority)
{
	int status;
	if ((status = Create(process,iPriority)) == kOK)
	{
		int delta = iMsec > 2 ? 2: iMsec;
		for (int t = 0;!m_process->InsideThreadProcess() && !m_process->HasTerminated() && t<iMsec;t += delta) 
			Sleep(delta);
	}
	
	return m_status;	
}


//------------------------------------------------------------------------------
iRTVThread::~iRTVThread(void)
{
	m_cs.Enter();
	if (m_thread)
	{
		Terminate();
		delete m_thread;
		m_thread = 0;
	}
	m_cs.Leave();
}

int iRTVThread::Suspend(int imsec)
{
    return 1;
}

int iRTVThread::Resume(void)
{
    return 0;
}

//------------------------------------------------------------------------------
int iRTVThread::SetPriority(int iPriority, iRTVThread::PriorityClass iPC)
{
    int p = THREAD_PRIORITY_NORMAL;
	
	
    if(iPriority == LOW)
		p = THREAD_PRIORITY_BELOW_NORMAL;
	else if (iPriority == LOWEST)
		p = THREAD_PRIORITY_LOWEST;
    else if (iPriority == HIGH)
		p = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (iPriority == REALTIME)
		p = THREAD_PRIORITY_TIME_CRITICAL;
	else if (iPriority == IDLE)
		p = THREAD_PRIORITY_IDLE;
    else 
		p = THREAD_PRIORITY_NORMAL;

	if (iPC != kDefault)
	{
		int pc = NORMAL_PRIORITY_CLASS;

		if (iPC == kLow)
			pc = BELOW_NORMAL_PRIORITY_CLASS;
		else if (iPC == kHigh)
			pc = HIGH_PRIORITY_CLASS;
		else if (iPC == kIdle)
			pc = IDLE_PRIORITY_CLASS;
		else
			pc = NORMAL_PRIORITY_CLASS;

		::SetPriorityClass(m_thread->m_threadID, pc);
	}
	
    int st = SetThreadPriority(m_thread->m_threadID, p);
//	assert(st);
	return !st;
}


//------------------------------------------------------------------------------
// called by the destructor
int iRTVThread::Terminate(int iMsec, int forceKill)
{
	TRCSLock L(&m_cs);

	if (m_process && m_thread && m_thread->m_threadID )
	{
		if (!m_process->TerminationRequested())
			m_process->RequestTermination(1);

		m_process->WaitForProcessToEnd(iMsec);
		
		if (m_process->IsRunning())
		{
			if (forceKill)
			{
				Message("Thread (0x%x) %s hang and killed in Terminate()\n", m_thread->m_threadID, m_process->Name());
				TerminateThread(m_thread->m_threadID,0);
				m_process->m_processStatus = iRTVThreadProcess::kKilled;
			}
			else
			{
				Message("Thread (0x%x) %s hang in Terminate()\n", m_thread->m_threadID, m_process->Name());
				m_process->m_processStatus = iRTVThreadProcess::kHang;
			}	 
		}
	}
	
	m_process = 0;
	
    return 0;
}

#endif

//-----------------------------------------------------------------------------
//
RTVOneShotThreadManager& RTVOneShotThreadManager::theManager()
{
	static RTVOneShotThreadManager p; // the signle Listener object
	return p;
}

//-----------------------------------------------------------------------------
//
RTVOneShotThreadManager::RTVOneShotThreadManager()
{
	m_processorName = "RTVOneShotThreadManager";
	m_finishIt = false;
}

//-----------------------------------------------------------------------------
//
RTVOneShotThreadManager::~RTVOneShotThreadManager()
{
	m_processorName = "RTVOneShotThreadManager";
	TRCSLock fplock(&m_threadMapCS);
	while(!m_requests.empty())
	{
		iRTVThreadProcess* p = m_requests.front();
		if (m_cleanup[p])
		{
			m_cleanup.erase(p);
			delete p;
		}
		m_requests.pop_front();
	}
}

//-----------------------------------------------------------------------------
//
void RTVOneShotThreadManager::SetMaxRunThreads(int maxRunThreads, const char* threadType)
{
	TRCSLock fplock(&m_threadMapCS);
	m_maxRunthreadsMap[threadType] = maxRunThreads;
}

//-----------------------------------------------------------------------------
//
int RTVOneShotThreadManager::AddRequest(iRTVThreadProcess* pRequest, int iDeleteWhenDone)
{
	// comment out to let thread start later
	/*if(m_processStatus != kRunning)
	{
		// no process, can not take any
		return iRTVThreadProcess::kNoManager;
	}*/

	TRCSLock fplock(&m_threadMapCS);
	m_requests.push_back(pRequest);
	m_cleanup[pRequest] = iDeleteWhenDone;
	m_newRequestSemaphore.Post();
	return 0;
}

//------------------------------------------------------------------------------
void RTVOneShotThreadManager::SetDeleteWhenDone(iRTVThreadProcess* pRequest, int iDeleteWhenDone)
{
	TRCSLock fplock(&m_threadMapCS);

	ThreadsMap::iterator iter = m_threadMap.find(pRequest);

	if (iter != m_threadMap.end())
	{
		iter->second.m_deleteWhenDone = iDeleteWhenDone;
	}
	else if (m_cleanup.find(pRequest) != m_cleanup.end())
	{
		m_cleanup[pRequest] = iDeleteWhenDone;
	}
	else
	{
		assert(0);
	}
}

//------------------------------------------------------------------------------
bool RTVOneShotThreadManager::RemoveRequest(iRTVThreadProcess* pRequest, int iMsec)
{
	TRCSLock fplock(&m_threadMapCS);
	bool stopped = false;
	// check running thread map
	iRTVThread* pThread=0;
	ThreadsMap::iterator iter;
	iter = m_threadMap.find(pRequest);

	if (iter != m_threadMap.end())
	{
		// take it out of map to avoid other thread delete it
		pThread = iter->second;
		iRTVThreadAndStatus tt = iter->second;
		m_threadMap.erase(iter);
		stopped = pRequest->Stop(iMsec);
		m_threadMap[pRequest] = tt;//pThread; // put back to let cleanup clean it
		return stopped;
	}
	else
	{
		RTVThreadQueue::iterator qiter;
		for (qiter=m_requests.begin(); qiter<m_requests.end(); qiter++)
		{
			if(*qiter == pRequest)
			{
				m_requests.erase(qiter);
				assert(m_cleanup.find(pRequest) != m_cleanup.end());
				int cleanit = m_cleanup[pRequest];
				m_cleanup.erase(pRequest);
				if (cleanit)
					delete pRequest;		
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
int	RTVOneShotThreadManager::GetMaxRunThreads(const char* threadType)
{
	MaxRunThreadsMap::iterator iter;

	TRCSLock fplock(&m_threadMapCS);
	iter = m_maxRunthreadsMap.find(threadType);

	return (iter != m_maxRunthreadsMap.end())?iter->second:-1;
}

//------------------------------------------------------------------------------
int	RTVOneShotThreadManager::GetRunningThreads(const char* threadType)
{
	int runningThreads = 0;
	ThreadsMap::iterator iter;
	iRTVThreadProcess* process;

	TRCSLock fplock(&m_threadMapCS);

	for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
	{
		process = (iter->first);
		if (threadType == process->Name())
		{
			runningThreads++;
		}
	}
	return runningThreads;

}

int	RTVOneShotThreadManager::GetRunningThreads(MaxRunThreadsMap& oMap)
{
	int runningThreads = 0;
	ThreadsMap::iterator iter;
	MaxRunThreadsMap::iterator oIter;
	
	TRCSLock fplock(&m_threadMapCS);
	oMap.clear();
	oMap["Total queued"] = m_requests.size();
	const char* procName;

	for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
	{
		procName = (iter->first)->Name();
		
		oIter = oMap.find(procName);
		if(oIter == oMap.end())
			oMap[procName] = 1;
		else
			oMap[procName] += 1;
		runningThreads++;
	}
	return runningThreads;

}

//------------------------------------------------------------------------------
void RTVOneShotThreadManager::RequestTermination(int iFlag)
{
	iRTVThreadProcess::RequestTermination(iFlag);
	m_newRequestSemaphore.Post();
}


//------------------------------------------------------------------------------
int RTVOneShotThreadManager::WaitFinish()
{
	if(m_processStatus != kRunning) // will run process in main thread
	{
		m_finishIt = true;
		return TRSemaphoreAPI::kOK;
	}

	m_finishIt = true;
	m_newRequestSemaphore.Post();
	int stat;
	while(!TerminationRequested())
	{
		stat = m_finishSemaphore.Wait(5000);
		if(stat == TRSemaphoreAPI::kOK)
		{
			RequestTermination(1);
			break;
		}
		else if(stat == TRSemaphoreAPI::kError)
			break;
	}
	return stat;
}

//-----------------------------------------------------------------------------
//
int	RTVOneShotThreadManager::Process()
{
	int runningThreads;
	int maxRunThreads;
	iRTVThreadProcess* proc;
	bool waitCount;

	TRCSLock fplock(&m_threadMapCS, false);
	while(!TerminationRequested())
	{
		CleanStopped();
		Sleep(10); // give other thread a chance to add request
		fplock.Lock();
		waitCount = false;
		if(!m_requests.empty())
		{
			proc = m_requests.front();
			maxRunThreads = GetMaxRunThreads(proc->Name());
			runningThreads = GetRunningThreads(proc->Name());
			// maxRunThreads < 0 means unlimited
			if( runningThreads < maxRunThreads || maxRunThreads < 0)
			{
				Handover(proc,true,m_cleanup[proc]);
				m_requests.pop_front();
				m_cleanup.erase(proc);
			}
			else
				waitCount = true;
		}

		runningThreads = m_threadMap.size(); // flag not for wait for m_newRequestSemaphore
		fplock.Unlock();
		
		if(runningThreads == 0)
		{
			if(m_finishIt)
			{
				if(m_processStatus != kRunning) //running in the main thread, just return 
					break;
				else
					m_finishSemaphore.Post();
				
			}
			else
				m_newRequestSemaphore.Wait(5000);//(INFINITE);
		}
		else if(waitCount)
			Sleep(500);

	}
	return 0;
}

#if 0
RTVMessageManager():m_pPipe(0), m_maxMessage(100), m_maxSendingMessage(5)
{};
	virtual	~RTVMessageManager() {if(m_pPipe) delete m_pPipe, m_pPipe=0;};

int	RTVMessageManager::Process()
{
	int runningThreads;
	int maxRunThreads;
	iRTVThreadProcess* proc;

	TRCSLock fplock(&m_cs, false);
	while(!TerminationRequested())
	{
		fplock.Lock();
		Sleep(10); // give other thread a chance to add request
		fplock.Lock();
		if(!m_requests.empty())
		{
			proc = m_requests.front();
			maxRunThreads = GetMaxRunThreads(proc->Name());
			runningThreads = GetRunningThreads(proc->Name());
			// maxRunThreads < 0 means unlimited
			if( runningThreads < maxRunThreads || maxRunThreads < 0)
			{
				Handover(proc);
				m_requests.pop_front();
			}
		}

		runningThreads = m_threadMap.size(); // flag not for wait for m_newRequestSemaphore
		fplock.Unlock();

		if(runningThreads == 0)
			m_newRequestSemaphore.Wait(INFINITE);

	}
	return 0;
}

int		PassMessagePip(RTVMessagePipe* ipPipe);
	
	bool	Post();
	void	Purge();

protected:
	RTVMessagePipe*		m_pPipe;
	int					m_maxMessage;
	int					m_maxSendingMessage;
	RTVMessageQueue		m_messages;
	TRCriticalSection	m_cs;
	TRSemaphore		m_newMessageSemaphore;
	
};
#endif

////////////////////////////////////////////////////////////////////////////////////
// -- 2006.08.01
// Finally got a chance to get this implemented
iRTVThreadedProcess::iRTVThreadedProcess(const char* iName) : iRTVThreadProcess(iName)
{



}

//---------------------------------------------------------------------------
iRTVThreadedProcess::~iRTVThreadedProcess()
{



}

//---------------------------------------------------------------------------
int iRTVThreadedProcess::Start(int iWaitForThread, int iPriority, iRTVThread::PriorityClass iPC)
{
	int status = m_theThread.Create(this, iPriority, iPC);

	if (iWaitForThread)
	{
		int delta = 20, t;
		for ( t = 0; !IsRunning() && t < iWaitForThread; t += delta)
		{
			Sleep(delta);
		}
	}

	return status;

}

//---------------------------------------------------------------------------
int iRTVThreadedProcess::Stop(int iWait)
{
	int status = 0;

	RequestTermination(1);

	if (iWait)
	{
		WaitForProcessToEnd(iWait);
		status = (IsRunning() ? -1:0);
	}
	
	return status;
}
