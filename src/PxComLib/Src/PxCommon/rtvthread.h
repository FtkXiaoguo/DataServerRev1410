/*
 *  rtvthread.h
 *---------------------------------------------------------------------
 *		Copyright (c) TeraRecon, Inc. 2000-2002. All rights reserved
 *
 *	PURPOSE:
 *		Threads, Critical Sections, Semaphores and Thread processors.
 *		Platform independent APIs.
 *
 *	AUTHOR(S):  T.C. Zhao, June 2000
 *
 */

#ifndef RTVTHREAD_H_
#define RTVTHREAD_H_

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)
#include <assert.h>
#include "rtvbase.h"
#include "AqCore/TRAtomic.h"

//-----------------------------------------------------------------------------
class iRTVThreadID;
class iRTVThread;

//
// In most cases, iRTVThreadedProcess is the right class to use as it
// handles the destruction sequence properly.
//

/********************************************************************
 * iRTVThreadProcess is a class meant to be a base
 * of processors for thread. Upon startup of a thread,
 * the function iRTVThreadProcess::Process(void) will be
 * called. When Process() function ends, the thread ends.
 *
 * PreProcess() is called before the start of a thread
 ********************************************************************/
class iRTVThreadProcess : public iRTVBase
{
public:
	/* threaded process status */
	enum 
	{ 
		kToBeStarted		= -100, 
		kNoError			=  0,
		kRunning			=  1, 
		kProcessTerminated	=  2,
		kFailed				= -1, 
		kKilled				= -2,
		kHang				= -3,
		kPreProcessing		= -4,
		kThreadJustCreated	= -5,		// just before creating thread
		kThreadFailed		= -6,		// failed to create thread
		kNoManager			= -7,
		/* aliases */
		kProcessEnded		=  kProcessTerminated
	};
	
	iRTVThreadProcess(const char *iProcessorName=0);
	virtual ~iRTVThreadProcess(void);
	
	/* the PreProcess() is called in the main thread. If Preprocess()
	* returns a negative value, the thread will NOT run Process()
	*/
	virtual  int	PreProcess(void) { m_processStatus = kToBeStarted; return 0;}
	
	/* The main entry for the threaded execution. When Process() ends, the
	* thread terminates.
	*	 The Process() function must check for termination request (TerminationRequested())
	*/
	virtual  int	Process(void) = 0;
	
	/*
	* check the status of the thread
	*  Potential returns: kToBeStarted, kRunning, kProcessEnded, kFailed
	*/
	inline int			GetProcessStatus(void) const 
	{ 
		return int(m_processStatus);
	}

	inline int			GetProcessReturnCode(void) const 
	{ 
		return int(m_processReturnCode);
	}

	virtual void	LogProcessStatus(void) {};
	
	inline bool			IsRunning(void) const 
	{ 
		return (m_processStatus == kRunning			||
			m_processStatus == kPreProcessing		||
			m_processStatus == kThreadJustCreated);
	}

	/* Check if the threadProcessor has terminated or not */
	inline bool			HasTerminated(void) const
	{
		return (m_processStatus == kProcessTerminated);
	}
	
	/* actually inside the Process() */
	inline bool			InsideThreadProcess(void) const
	{
		return (m_processStatus == kRunning);
	}
	
	/* 
	* Request the termination of the thread - the process() method
	* must check the settings of TerminationRequested()
	*/
	
	virtual  void	RequestTermination(int iFlag=1);
	
	/*
	* Check if a request to terminate has been made.
	* Process() method should return if true.
	*/
	virtual  int	TerminationRequested(void) const { return m_ended;}
	void	DeleteThread(void);
	
	
	/* Wait for the thread to terminate. Typically make a request, then use
	* this function to wait.
	*    
	*/
	virtual  int	WaitForProcessToEnd(int iMsec=8000, int iDelta=10);
	
	inline unsigned int	GetThreadID(void) const 
	{ 
		return m_threadID;
	}
	
	inline iRTVThread*		GetTheTread() 
	{
		return m_theThread;
	};

	inline const char*		Name(void)  const 
	{ 
		return m_processorName;
	}
	
	/*
	* old obsolete API
	*/
	virtual  int	HasEnded(void) const { return TerminationRequested();}
	virtual  void	SetEnded(int iFlag)  { RequestTermination(iFlag);}
	bool Stop(int iMsec);
	virtual void ForceStop(int iMsec);//#82 2014/09/29 K.Ko change to virtual
	
	friend	class	iRTVThread;
	friend void __cdecl ThreadProcess( void *data);
	
protected:
	iRTVThread*		m_theThread;
	int				m_processStatus;
	unsigned int	m_threadID;
	int				m_ended;
	const char*		m_processorName;
	int				m_processReturnCode;
	
};

//------------------------------------------------------------------------
class iRTVThreadFunction
{
public:
	friend class iRTVThreadRunClassFunction;
	iRTVThreadFunction(){};
	virtual ~iRTVThreadFunction(){};

protected:
	 virtual int ThreadFunction(void* data) = 0;


protected:
	iRTVThreadProcess* m_pobj;
	void* m_data;
};


//------------------------------------------------------------------------
class iRTVThreadRunClassFunction : public iRTVThreadProcess
{
public:
	iRTVThreadRunClassFunction(iRTVThreadFunction* pobj, void* data=0): m_pobj(pobj), m_data(data){};
	virtual ~iRTVThreadRunClassFunction(){};
	void SetData(void* data){m_data = data;};
	void* GetData(){return m_data;};
	virtual int Process(void) {return m_pobj->ThreadFunction(m_data);};

protected:
	iRTVThreadFunction* m_pobj;
	void* m_data;
};

/***********************************************************************
 * iRTVThread takes a threadProcessor and runs the processor
 * in a seperate thread. Not all functions supported on all platforms
 ***********************************************************************/
class iRTVThread : public iRTVBase
{
public:
	static long Total; // 32 bits value is read/write atomic

	/* thread priority class */
	enum PriorityClass
	{
		 kDefault,kIdle, kLow, kNormal, kHigh
	};

	/* thread priorities within the class. Some of these are
	 * combinations of priority class and prioity. For example, 
	 * IDLE priority is really (kIdle, NORMAL)
	 */
	enum 
	{ 
		IDLE, LOWEST, LOW, NORMAL, HIGH, REALTIME
	};
	

	/* status from Create() */
	enum 
	{ 
		kOK = 0, 
		kBadProcessor		= -1,	// bad argument to Create()
		kFailedToCreate		= -2,	// can't create the thread	
		kAlreadyCreated		= -3,	// already running
		kPreprocessFailed	= -4	// preprocess() failed
	};

	/* Initialize the class. No thread is actually created if iProcess
	 * is null.
	 */
	iRTVThread(iRTVThreadProcess *iProcess=0, int iPriority=NORMAL);
	
	/* 
	 * create the thread. returns kOK if successful. Returning ok
	 * does not mean the iRTVThreadProcess::Process() is running
	 */
	int		Create(iRTVThreadProcess *iProcess, int iPriority=NORMAL, PriorityClass iPriorityClass=kDefault);

	/*
	 * Create the thread, and waits until the execution enters 
	 * iRTVThreadProcess::Process() or iMsec expires
	 */
	int	    CreateAndRunProcess(iRTVThreadProcess *iProcess, int iMsec=100, int iPriority=NORMAL);

	/*
	 * De-couple the thread with processor after waiting for iMsec milli-second
	 */
		
	int		Terminate(int iMsec=2000, int iForceKill=0);

	 /* 
	  * forcibly kill a thread after waiting for iMsec millisecond - dangerous
	  */

    inline void	Kill(int iMsec=2000) 
	{ 
		Terminate(iMsec,1);
	}
	  

	inline int		GetStatus(void) const 
	{ 
		return int(m_status);
	}

	const char*	GetStatusText(int iStatus);

	void	ProcessDone(void);
	
	// not all platform supports priority and suspend/resume
	int		SetPriority(int inPriority, PriorityClass ipc = kDefault);
	int		Suspend(int iMsec=0);
	int		Resume(void);
	virtual ~iRTVThread(void);

#if defined( __DENTAL )
	HANDLE GetThreadHandle( void );
#endif

private:
	iRTVThread(const iRTVThread&);
	iRTVThreadProcess*	m_process;
	iRTVThreadID*		m_thread;
	int					m_status;
	TRCriticalSection	m_cs;
};


//------------------------------------------------------------------------
/*
 * A threaded processor
 */
class iRTVThreadedProcess : public iRTVThreadProcess
{
public:
	iRTVThreadedProcess(const char* iName);
	~iRTVThreadedProcess(void);

	// Start the threaded process. The parameter sets if and how long
	// to wait for the thread the start.
	int Start(int iWaitForThreadInMs=0, 
		      int iPriority=iRTVThread::NORMAL, 
			  iRTVThread::PriorityClass iPC=iRTVThread::kDefault);

	int Stop(int iWait=0);

private:
	iRTVThread m_theThread;
};


//------------------------------------------------------------------------
#pragma warning (disable: 4786)
#pragma warning (disable: 4616)
#include <map>
#include <string>

struct iRTVThreadAndStatus
{
	iRTVThreadAndStatus(iRTVThread* T=0, int iDeleteWhenDone=1)
	{
		m_theThread = T;
		m_deleteWhenDone = iDeleteWhenDone;
	}

	operator iRTVThread*(void) const { return m_theThread;}

	iRTVThread*	m_theThread;
	int			m_deleteWhenDone;
};

// single thread mode Thread Manager
//iRTVThreadProcess
template <class T>
class RTVThreadManager
{
public:	
	typedef	std::map<T*, iRTVThreadAndStatus> ThreadsMap;
	RTVThreadManager():m_managedThreads(0) {m_threadMap.clear();} ;
	virtual	~RTVThreadManager() {StopAll();};
	
	// take ownership of threaded process object, may start it in a thread depends on auto_start flag
	void	Handover(T* processor, bool auto_start=true, int iDeleteWhenDone=1)
	{
		ThreadsMap::iterator iter;
		TRCSLock fplock(&m_threadMapCS);
		iter = m_threadMap.find(processor);
	
		if (iter != m_threadMap.end())
		{
			// can't not hand over more than once
			return;
		}
		
		iRTVThread* pthread = 0;
		if(auto_start)
		{
			pthread = new iRTVThread(processor);
		}
	
		m_threadMap[processor] = iRTVThreadAndStatus(pthread,iDeleteWhenDone);
		m_managedThreads++;
	};


	int GetManagedThreads() {return m_managedThreads;};

	// user may call this function to start process object in a thread, if it is not started
	bool	Start(T* processor)
	{
		ThreadsMap::iterator iter;
		TRCSLock fplock(&m_threadMapCS);
		iter = m_threadMap.find(processor);
		if (iter == m_threadMap.end() ) // not handed over to me yet, fail it
			return false;
		
		if((iter->second) != NULL) // already started, just return
			return true;
		
		m_threadMap[processor] = iRTVThreadAndStatus(new iRTVThread(processor),iter->second.m_deleteWhenDone);
		
		return true;
	}

	int		Threads() 
	{ 
		TRCSLock fplock(&m_threadMapCS); 
		return m_threadMap.size();
	}

	int		CleanStopped()
	{
		iRTVThread* pthread = NULL;
		ThreadsMap::iterator iter;
		TRCSLock fplock(&m_threadMapCS);
		T* process;
		int closed = 0;
		int stat;
		for (iter=m_threadMap.begin(); iter != m_threadMap.end(); )
		{
			try
			{
				process =(iter->first);
				stat = process->GetProcessStatus();
			}
			catch(...)
			{
				assert(0);
				iter = m_threadMap.erase(iter);
				continue;
			}

			if (stat == iRTVThreadProcess::kProcessTerminated)
			{
				//pthread = process->m_theThread;
				process->Message("deleting Thread(0x%x, %s) processor thread\n", process->GetThreadID(), process->Name());
				process->DeleteThread();
				//we owe the object by porinter, so delete it when finished using
				process->Message("deleting Thread(0x%x, %s) processor itself\n", process->GetThreadID(), process->Name());
				
				if (iter->second.m_deleteWhenDone)
					delete process;  
			
				//delete pthread;  
				iter = m_threadMap.erase(iter);
				closed++;
			}
			else
				++iter;
		}
		return closed;
	}
	
	void	StopAll(int iMsec=18000)
	{
		TRCSLock fplock(&m_threadMapCS);
		CleanStopped();
		
		if(!m_threadMap.size())
			return;
		
		ThreadsMap::iterator iter;
		T* process;
		
		// tell all threads to stop
		for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
		{
			process = (iter->first);
			process->RequestTermination(1);
		}
		Sleep(100); // yeild to other threads to terminate
		
		// make sure all threads stopped
		for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
		{
			process = (iter->first);
			process->ForceStop(iMsec);
#if 0
			delete process->m_theThread;  //we owe the object by porinter, so delete it when finished using
			process->m_theThread = 0;
			delete process;       //we owe the object by porinter, so delete it when finished using
#endif
		}
		
		CleanStopped();
		
		assert(m_threadMap.size() == 0);
		
		m_threadMap.clear();
	};
	
protected:
	ThreadsMap	m_threadMap;
	TRCriticalSection	m_threadMapCS;
	int			m_managedThreads;
};

#include <queue>
// singleton one shut Thread Manager (thread safe)
class RTVOneShotThreadManager : public iRTVThreadProcess, protected RTVThreadManager<iRTVThreadProcess>
{
public:	
	typedef	std::deque<iRTVThreadProcess*> RTVThreadQueue;
	typedef	std::map<std::string, int> MaxRunThreadsMap;
	static RTVOneShotThreadManager& theManager();

	virtual	~RTVOneShotThreadManager();
	int		Process();
	void	RequestTermination(int iFlag=1);
	int		AddRequest(iRTVThreadProcess* pRequest, int iDeleteWhenDone=1);
	void	SetDeleteWhenDone(iRTVThreadProcess* pRequest, int iDeleteWhenDone=1);
	bool	RemoveRequest(iRTVThreadProcess* pRequest, int iMsec);
	void	SetMaxRunThreads(int maxRunThreads, const char* threadType);
	int		GetMaxRunThreads(const char* threadType);
	int		GetRunningThreads(const char* threadType);
	int		GetRunningThreads(MaxRunThreadsMap& oMap);
	int		WaitFinish();

protected:
	MaxRunThreadsMap	m_maxRunthreadsMap;
	RTVThreadQueue		m_requests;
	TRSemaphore		m_newRequestSemaphore;
	TRSemaphore		m_finishSemaphore;
	std::map<iRTVThreadProcess*,int> m_cleanup;
	bool				m_finishIt;

private:
	RTVOneShotThreadManager();
};

//------------------------------------------------------------------------
class RTVMessage
{
public:
	RTVMessage():m_key(0), m_dataLen(0), m_pData(0){};
	virtual ~RTVMessage() {if(m_pData) delete[] m_pData, m_pData=0; };

	virtual bool	Merge() {return false;};
	long			GetKey() {return m_key;};
	long			GetDataLen() {return m_dataLen;};
	const char*		GetData() { return m_pData;};
protected:
	long	m_key;
	long	m_dataLen;
	char*	m_pData;
};

//------------------------------------------------------------------------
class RTVMessagePipe : public iRTVBase
{
public:
	RTVMessagePipe();
	virtual ~RTVMessagePipe();

	virtual int Send(RTVMessage& iMessage) = 0;
	virtual int GetOutstandardSendings() = 0;
};

//------------------------------------------------------------------------	
class RTVMessageManager : public iRTVThreadProcess
{
public:	
	typedef	std::deque<RTVMessage*> RTVMessageQueue;
	
	RTVMessageManager();
	virtual	~RTVMessageManager();
	int		Process();
	int		PassMessagePip(RTVMessagePipe* ipPipe);
	
	bool	Post();
	void	Purge();
	int		m_maxMessage;
	int		m_maxSendingMessage;

protected:
	RTVMessagePipe*		m_pPipe;
	RTVMessageQueue		m_messages;
	TRCriticalSection	m_cs;
	TRSemaphore		m_newMessageSemaphore;
	
};

#endif   /* RTVThread_H */



