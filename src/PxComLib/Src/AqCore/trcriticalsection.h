/*---------------------------------------------------------------------
 * TRCriticalsection.h
 *---------------------------------------------------------------------
 *	
 *
 *--------------------------------------------------------------------*/

#ifndef TRCRITICAL_SECTION_H_
#define TRCRITICAL_SECTION_H_

/*
 * APIS - both UNIX and NT implementation derive
 * from the abstract API class. 
 */

class TRCriticalSectionAPI
{
public:
	TRCriticalSectionAPI(void) {}
	virtual ~TRCriticalSectionAPI(void) {}
	virtual void Enter(void) const = 0;
	virtual void Leave(void) const = 0;
	virtual int	 Enter(unsigned mSec) const = 0 ; //true if entered the critical section
private:

};

class TRSemaphoreAPI
{
public:
	enum {kError = -1, kOK = 0, kTimeOut};
	TRSemaphoreAPI(void) {}
	virtual ~TRSemaphoreAPI(void) {}

	// function returns kError/kOK/kTimeOut. Use negative time
	// to request infinite wait
	virtual int Wait(long iMillieSeconds) = 0;

	// function returns the counter
	virtual int	Post(int iN) = 0;
};



/****************************************************************************
 * Win32 critical section and semaphore class
 ****************************************************************************/
#if defined(_WIN32)

#include <process.h>
#include <windows.h>

#pragma warning (disable: 4786)
#pragma warning (disable: 4616)
 

class TRCriticalSection : public TRCriticalSectionAPI
{
public:
	TRCriticalSection(void)   { InitializeCriticalSection(&m_cs);}

	virtual~TRCriticalSection(void)  { DeleteCriticalSection(&m_cs);}
	

	// both Enter and Leave are not really const methods but by making
	// them so helps make other API use critical section much better.
	inline void Enter(void) const {EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));}
	inline void Leave(void) const {LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));}
	int			Enter(unsigned int mSec) const;
private:
	//GL 2005-9-14 private copy constructor and assign operator to forbid use it
	TRCriticalSection(const TRCriticalSection& iCS); //{ InitializeCriticalSection(&m_cs);}
	const TRCriticalSection& operator=(const TRCriticalSection& iCS);// {return *this;};

    CRITICAL_SECTION m_cs;
};

//---------------------------------------------------------------------------
#include <assert.h>
// a kind of count-up semaphore
class TRSemaphore : public TRSemaphoreAPI
{
public:
	TRSemaphore(int iInitVal=0,const char *iName=0, int iMax=50)
	{
		m_handle = CreateSemaphore(0, iInitVal, iMax, iName);
	}

	virtual ~TRSemaphore(void)
	{
		CloseHandle(m_handle);
	}

	int   Wait(long iMilliSecond)
	{
		if (iMilliSecond == 0)
		{
		   assert(iMilliSecond); // tcz 2006.08.02 changed to catch old 0. Should remove later
		}
	
		if (iMilliSecond < 0)
		  iMilliSecond= INFINITE;
		DWORD status = WaitForSingleObject(m_handle,iMilliSecond);
		return (status == WAIT_OBJECT_0) ? kOK:(status==WAIT_TIMEOUT ?kTimeOut:kError);
	}

	inline int  Post(int n=1)
	{
		LONG lcount;
		ReleaseSemaphore(m_handle, n, &lcount);
		return int(lcount);
	}
private:
	TRSemaphore (const TRSemaphore& iS);
	const TRSemaphore& operator=(const TRSemaphore& iS);

	HANDLE m_handle;
};

//-----------------------------------------------------------------
class TRMutex
{
public:
	enum { kError = -1, kOK = 0, kTimeOut};
	TRMutex(const char* iName, int iAutoLock=0);
	virtual ~TRMutex(void);

	/* Request the exclusive lock. iMilliSec specifies
	 * how long to wait before giving up. 0 mean infinity
	 * Check function return for the status
	 */
	int		Acquire(long iMilliSec=0) const;

	/* release lock */
	int		Release(void) const;
private:
	TRMutex (const TRMutex& iS);
	const TRMutex& operator=(const TRMutex& iS);

	HANDLE	m_mutex;
};

//----------------------------------------------------------------
class TRWaitableCS : public TRCriticalSectionAPI
{
public:
	TRWaitableCS(const char *iName=0) : m_mutex(iName) {}

	int	Enter(unsigned int imSec) const
	{
		return m_mutex.Acquire(imSec)==TRMutex::kOK;
	}

	void Leave(void) const
	{
		m_mutex.Release();
	}

	void Enter(void) const
	{
		(void)Enter(0);
	}
private:
	TRWaitableCS(const TRWaitableCS&);
	const TRWaitableCS& operator=(const TRWaitableCS&);
	TRMutex m_mutex;
};

#else
/**************************************************************************
 * Critical section and semaphore under Posix threads (not tested)
 *************************************************************************/
#include <pthread.h>

class TRCriticalSection : public TRCriticalSectionAPI
{
public:
	TRCriticalSection(void)   { pthread_mutex_init(&m_mutex, 0);}
	~TRCriticalSection(void)  { pthread_mutex_destroy(&m_mutex);}
	// both Enter and Leave are not really const methods but by making
	// them so helps make other API use critical section much better.
	inline void Enter(void) const {pthread_mutex_lock(const_cast<pthread_mutex_t*>(&m_mutex));}
	inline void Leave(void) const {pthread_mutex_unlock(const_cast<pthread_mutex_t*>(&m_mutex));}
private:
    pthread_mutex_t m_mutex;
};

#endif 

// A critical section counter for lazy people: useful
// for complicated control flow that needs to go in/out of a critical
// section or complicated returns from a function or block. 
// NOT itself thread-safe!
class TRCSLock
{
public:
	TRCSLock(const TRCriticalSectionAPI* cs, bool lockit=true):m_locked(0) 
	{ 
		if ((m_cs = cs)&&lockit) 
		{
			m_cs->Enter();
			m_locked=1;
		}
	}

	~TRCSLock(void)				  
	{ 
		if (IsLocked())
			m_cs->Leave();
	}

	inline bool		IsLocked(void)	  { return ( m_cs != 0 && m_locked != 0);		}
	inline void		Unlock(void)	  { if (IsLocked())			{  m_cs->Leave();m_locked = 0;}}
	inline void		Lock(void)		  { if (!m_locked && m_cs)	{  m_cs->Enter();m_locked = 1;}}

private:
	TRCSLock (const TRCSLock& iC);
	const TRCSLock& operator=(const TRCSLock& iC);

	const TRCriticalSectionAPI*	m_cs;
	int								m_locked;
};

// -- 2006.08.09
// Similar to TRCSLock, but TRTimeCSLock times out
class TRCSTimedLock
{
public:
	TRCSTimedLock(const TRCriticalSectionAPI* cs, int iTimeInMs=2000, bool lockit=true ):m_locked(0) 
	{ 
		if ((m_cs = cs)&&lockit) 
		{
			m_locked = m_cs->Enter(iTimeInMs);
		}
	}

	~TRCSTimedLock(void)				  
	{ 
		if (IsLocked())
			m_cs->Leave();
	}

	inline bool		IsLocked(void)	  { return ( m_cs != 0 && m_locked != 0);		}
	inline void		Unlock(void)	  { if (IsLocked())	{  m_cs->Leave();m_locked = 0;}}

	inline void		Lock(int iTimeInMs=2000)		 
	{ 
		if (!m_locked && m_cs)
		{ 
			m_locked =	m_cs->Enter(iTimeInMs);
		}
	}

private:
	TRCSTimedLock (const TRCSTimedLock& iC);
	const TRCSTimedLock& operator=(const TRCSTimedLock& iC);

	const TRCriticalSectionAPI*	m_cs;
	int							m_locked;
};


//------------------------------------------------------------------
// a fake critical section
class TRFunnyCS : public TRCriticalSectionAPI
{
public:
	TRFunnyCS(void) {}
	~TRFunnyCS(void) {}
	inline void Enter(void) const { }
	inline void Leave(void) const { }
	inline int	Enter(unsigned int mSec) const { return 1;}
};


/*
 * Critical section based on a string. 
 *
 *
 *   TRNamedCSLock lock("string");
 *      criticalSection code
 *
 *   The lock is released in the destructor of TRNamedCSLock()
 */

#define USE_MUTEX_FOR_NAMEDCS

#ifdef USE_MUTEX_FOR_NAMEDCS
/* Lock a critical section within a block:
 */
class TRNamedCSLock
{
public:
#define kCSLockTimeOut (5*60*1000)
	TRNamedCSLock(const char* iName, bool iInitialLocked=true, 
		             unsigned itimeout=kCSLockTimeOut)
		            : m_mutex(iName), m_lockCount(0)
	{ 
		if (iInitialLocked) Lock(itimeout);
	}

	~TRNamedCSLock(void)			{ for (; m_lockCount > 0;) Unlock();}

	void		Lock(unsigned timeOut=kCSLockTimeOut)			
	{ 
		if (m_mutex.Acquire(timeOut) == TRMutex::kOK)
			++m_lockCount;
	}

	void		Unlock(void)		{ if (m_lockCount > 0 ) {m_mutex.Release(); --m_lockCount;}}
	static int	Size(void)			{ return 0;}
	static void ClearAll(void)		{ }
private:
	TRNamedCSLock (const TRNamedCSLock& iC);
	const TRNamedCSLock& operator=(const TRNamedCSLock& iC);
	
	TRMutex	m_mutex;
	int			m_lockCount;
};

#else
/**********************************************************************
 * A critical section based on a string. A cheaper alternative to mutex ?
 *********************************************************************/
class TRCountedCS : public TRCriticalSection
{
public:
	TRCountedCS(void) { m_refCount = 1;}
    ~TRCountedCS(void) {};
	int	m_refCount;
};

typedef std::map<std::string, TRCountedCS*> TRNamedCSMap;

class TRNamedCSLock 
{
public:
	TRNamedCSLock(const char* iName, bool iInitialLocked=true):m_lockCount(0)
	{
		m_name = iName ? iName:	"anonymous";
		if (iInitialLocked) 	Lock();
		
	}
	
	~TRNamedCSLock(void)		{ for (; m_lockCount > 0;) Unlock();}	
	void Lock(void);
	void Unlock(void);
	static int	Size(void)		{ return m_allCS.size();}
	static void ClearAll(void)	{ m_allCS.clear();}
	
private:
	TRNamedCSLock (const TRNamedCSLock& iC);
	const TRNamedCSLock& operator=(const TRNamedCSLock& iC);

	std::string					m_name;
	int							m_lockCount;
	static TRCriticalSection	m_cs;
	static TRNamedCSMap			m_allCS;
};

#endif /* use MUTEX for named CriticalSection */

#endif /* critical section */