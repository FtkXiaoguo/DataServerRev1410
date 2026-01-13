/*---------------------------------------------------------------------
 * TRCriticalSection.cpp
 *---------------------------------------------------------------------
 *
 *
 *--------------------------------------------------------------------*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include "TRCriticalSection.h"
#undef _WIN32_WINNT
#else
#include "TRCriticalSection.h"
#endif

#ifdef _DEBUG
#include <stdio.h>
#endif


//---------------------------------------------------------------------------
int TRCriticalSection::Enter(unsigned int mSec) const
{ 
	int own = 1;

	if (mSec == 0)
	{
		Enter();
	}
	else
	{
		unsigned delta = (mSec > 100 ? 100:mSec), t;	
		for ( t = 0; t < mSec && !(own = TryEnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs))); t += delta)
		{
			Sleep(delta);
		}
	}
	
	return own;
}

//----------------------------------------------------------------------------
/* most of the code are in the header file */

TRMutex::TRMutex(const char* iName, int iAutoLock)
{
	DWORD err = 0;
	m_mutex = ::CreateMutex(0, iAutoLock, iName);

	if (m_mutex == 0)
	{
		err = ::GetLastError();
		if(ERROR_ACCESS_DENIED == err)
		{
			m_mutex = ::OpenMutex(0, TRUE, iName);
		}
	}
#ifdef _DEBUG
	if (m_mutex == 0)
	{
		err = ::GetLastError();
		fprintf(stderr,"Failed to create mutax %s with error(%d).\n", iName ? iName:"anonymous", err);
	}
#endif
}


//-----------------------------------------------------------------------------
TRMutex::~TRMutex(void)
{
	if (m_mutex)
	{
		CloseHandle(m_mutex);
	}

	m_mutex = 0;
}

//-----------------------------------------------------------------------------
int	TRMutex::Acquire(long iMilliSec) const
{
	if (!m_mutex)
		return kError;
		
	if (iMilliSec <= 0)
		iMilliSec= INFINITE;

	DWORD status = WaitForSingleObject(m_mutex,iMilliSec);
	return (status == WAIT_OBJECT_0) ? kOK:(status==WAIT_TIMEOUT ?kTimeOut:kError);
}

//-----------------------------------------------------------------------------
int TRMutex::Release(void) const
{
	return ReleaseMutex(m_mutex) ? kOK:kError;
}

//-------------------------------------------------------------------------

#ifndef USE_MUTEX_FOR_NAMEDCS

TRCriticalSection	TRNamedCSLock::m_cs;
TRNamedCSMap		TRNamedCSLock::m_allCS;

void TRNamedCSLock::Lock(void) 
{	
	TRCSLock localLock(&m_cs);
	++m_lockCount;
	if (( p = m_allCS.find(m_name)) == m_allCS.end())
	{
		CountedCS *cs = new CountedCS();
		m_allCS[m_name] = cs;
		localLock.Unlock();
		cs->Enter();
	}
	else
	{
		p->second->m_refCount++;
		localLock.Unlock();
		p->second->Enter();
	}	
}

//----------------------------------------------------------------------------
void TRNamedCSLock::Unlock(void) 
{
	TRNamedCSMap::iterator p,e;
	
	TRCSLock localLock(&m_cs);
	p = m_allCS.find(m_name);
	e = m_allCS.end();
	--m_lockCount;
	if (p != e && p->second)
	{		
		localLock.Unlock();
		p->second->Leave();
		if (p->second->m_refCount > 0)
		{
			p->second->m_refCount--;
			localLock.Lock();
			if (p->second && int(p->second->m_refCount) == 0)
			{		
				delete p->second;
				m_allCS.erase(p->first);
			}
		}
	}	
}

#endif