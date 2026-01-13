/***********************************************************************
 * rtvPoolAccess.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon, Inc 2002, All rights reserved.
 *
 *	PURPOSE:
 *		This class is served as a template for objects pool access 
 *      control in a multip threads environment.
 *
 *	AUTHOR(S):  Gang Li	05-03-2002
 *   
 *-------------------------------------------------------------------
 */

//-----------------------------------------------------------------------------

#if !defined(RTVPoolAccess_H)
#define RTVPoolAccess_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include "AqCore/TRCriticalsection.h"

#include "IntervalProcessor.h"

// RTVMapAccess is a thread safe map class wraper
//
template <class K, class O, class _Pr = std::less<K> >
class RTVMapAccess
{
public:	
	typedef std::map<K, O, _Pr> _Myt;
	typedef	typename _Myt::iterator iterator;

	RTVMapAccess() {};
	~RTVMapAccess(){}; // no virtual destructor, do not derive from it
	//virtual ~RTVMapAccess(){};

	// public member variables to provid possiblly direct map access
	// use it carefully.
	TRCriticalSection m_cs; //access control resource
	_Myt m_map;

	bool Add(const K& key, const O& obj, bool overwrite=false)
	{
		TRCSLock fplock(&m_cs); // lock map, unlock when function return
		if(!overwrite)
		{
			_Myt::iterator iter = m_map.find(key);
			if (iter != m_map.end() ) // key does not exist, insert ok
				return false;
		}
		m_map[key] = obj;
		return true;
	};

	int Size()
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		return m_map.size();
	};
	

	O Get(const K& key)
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		_Myt::iterator iter = m_map.find(key);
		return (iter != m_map.end())?iter->second:NULL;
	};
	
	bool Has(const K& key)
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		_Myt::iterator iter = m_map.find(key);
		return (iter != m_map.end())?true:false;
	};

	const K* ItemKey(const O& obj)
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		_Myt::iterator iter;
		for (iter = m_map.begin(); iter != m_map.end();)
		{
			if (iter->second == obj ) // found one
			{
				return &(iter->first); 
			}
		}
		return 0; 
	};

	int Remove(const K& key)
	{
		int removed = 0;
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		_Myt::iterator iter = m_map.find(key);
		if (iter != m_map.end() ) // find key
		{
			m_map.erase(key);
			removed++;
		}
		return removed;
	};
	
	int Remove(const O& obj)
	{
		int removed = 0;
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		_Myt::iterator iter;
		for (iter = m_map.begin(); iter != m_map.end();)
		{
			if (iter->second == obj ) // found one
			{
				iter = m_map.erase(iter);
				removed++;
			}
			else
			{
				++iter;
			}
		}
		return removed;
	};
	
	void Clear()
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		m_map.clear();
	};

	void Walk(bool pf(iterator&))
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		iterator iter;
		for (iter = m_map.begin(); iter != m_map.end(); iter++)
		{
			if(!(*pf)(iter))
				break;
		}
	};


};

// RTVLockObject is a class warpper for a resource with a critical section
// and a lock counter.
// It is used to put object in RTVPoolAccess
template <class O>
class RTVLockObject
{
//friend class RTVObjectAutoLock<K, O>;
public:
	RTVLockObject(O obj, void (*pOnClose)(O&)):m_obj(obj),m_pOnClose(pOnClose),m_nLock(0){};
	~RTVLockObject(){if(m_pOnClose) (*m_pOnClose)(m_obj);}; 
	void IncLock() {InterlockedIncrement(&m_nLock);}; // this one has to be called before Grab
	// it is safe to give out reference, after grabbed no one can delete this object
	O& Grab() {m_lock.Enter(); return m_obj;} ; 
	void Release() {m_lock.Leave(); InterlockedDecrement(&m_nLock);};
	int  NLock() {return m_nLock;};

protected:
	O m_obj;
	void (*m_pOnClose)(O& obj); // ther callback function for deleting action
	TRCriticalSection m_lock; 
	long m_nLock; // out standing locks counter
};

// RTVPoolAccess is a thread safe pool class. The objects in the poll
// can be reserved through Reserve call, then lock it as will. 
//
template <class K, class O>
class RTVPoolAccess
{
public:	
	typedef std::map<K, RTVLockObject<O>*> POOL;

	RTVPoolAccess() {};
	virtual ~RTVPoolAccess();

	bool Create(const K& key, const O& obj, void (*pOnClose)(O& obj)=0);
	bool Has(const K& key);
	RTVLockObject<O>* Reserve(const K& key);
	bool Delete(const K&  key);

protected:
	TRCriticalSection m_cs; //access control resource
	POOL m_pool; // objects pool
};


template <class K, class O>
RTVPoolAccess<K, O>::~RTVPoolAccess()
{
	POOL::iterator iter;
	RTVLockObject<O>* p_obj = NULL;
	
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	for (iter=m_pool.begin(); iter != m_pool.end(); iter++)
	{
		// We assume no one grab object at exit time
		// so clear all objects in the pool
		p_obj = iter->second; 
		delete p_obj;
	}
	m_pool.clear();

}

// Create a RTVLockObject obecjt for a resource to put in the pool
template <class K, class O>
bool RTVPoolAccess<K, O>::Create(const K& key, const O& obj, void (*pOnClose)(O&))
{
	POOL::iterator iter;
	bool status = false;
	RTVLockObject<O>* pLockObject;
	
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	iter = m_pool.find(key);
    if (iter == m_pool.end() ) // key does not exist, insert ok
	{
		pLockObject = new RTVLockObject<O>(obj, pOnClose);
		if(pLockObject)
		{
			m_pool[key] = pLockObject;
			status = true;
		}
	}
	return status;
}

// Query a resource existion, it may disapear after return.
template <class K, class O>
bool RTVPoolAccess<K, O>::Has(const K& key)
{
	//event we found it, it may disapear after return
	TRCSLock fplock(&m_cs); 
	return (m_pool.find(key) != m_pool.end());
}

// Get out a resource by key and increase the object lock counter to reserve it
template <class K, class O>
RTVLockObject<O>* RTVPoolAccess<K, O>::Reserve(const K& key)
{
	POOL::iterator iter;
	RTVLockObject<O>* pLockObject = 0;
	
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	iter = m_pool.find(key);
    if (iter != m_pool.end() ) 
	{
		pLockObject = iter->second; // found the key
		pLockObject->IncLock();
	}
	return pLockObject;
}

//-----------------------------------------------------------------------------
// Delete entry from the pool. The call may fail if others grabbed the entry.
template <class K, class O>
bool RTVPoolAccess<K, O>::Delete(const K& key)
{
	POOL::iterator iter;
	TRCSLock fplock(&m_cs); // lock pool, unlock when function return
	iter = m_pool.find(key);
    if (iter == m_pool.end() ) 
		return true; // the object is not in the pool, delete noting success

	RTVLockObject<O>* pLockObject = iter->second; // found it

	if(pLockObject->NLock() > 0) // make sure no one grabbed it
		return false;

	m_pool.erase(iter);
	delete pLockObject;
	return true;
}

//-----------------------------------------------------------------------------
// Help class to provid RTVPoolAccess object grab
template <class K, class O>
class RTVObjectAutoLock
{

public:
	RTVObjectAutoLock( RTVPoolAccess<K, O>* pc ) : m_pobj(0), m_pc(pc) {};
	virtual ~RTVObjectAutoLock() {Release();};
	O Grab(const K& key)
	{
		if(m_pc ==  0) return 0;
		m_pobj = m_pc->Reserve(key);  // find the resource and increase lock counter
		if(!m_pobj) return 0;
		return m_pobj->Grab(); //try to lock it
	}

	void Release()
	{
		if(!m_pobj) return;

		m_pobj->Release(); // release the lock and decrease the lock counter.
		m_pobj = 0;
	}

private:
	RTVPoolAccess<K, O>* m_pc;
	RTVLockObject<O>* m_pobj;
};

//-----------------------------------------------------------------------------
#include "rtvthread.h"
class RTVInactiveManager;

class InactiveHandler : public iRTVBase
{
public:
	friend class RTVInactiveManager;

	InactiveHandler() : m_inProcess(0){};

	virtual int Process(void) = 0;
	virtual void Kick(void) = 0;
	virtual bool IsTimeOver(DWORD TickCount) = 0;
	virtual void ForceTimeOut() = 0;
	bool CanStop(DWORD TickCount) {return (IsTimeOver(TickCount) && m_keepMap.Size() == 0);};
	

protected:
	void KeepIt(void* iLocker, bool iKeep);

	RTVMapAccess<void*, int> m_keepMap;
	long m_inProcess;
};


class RTVInactiveManager : public CIntervalProcessor//iRTVThreadProcess 
{
public:	
	typedef std::map<std::string, InactiveHandler*> InactiveHandlerMap;
	static RTVInactiveManager& theManager();
	~RTVInactiveManager();
	void Handover(const char* id, InactiveHandler* ph);
	InactiveHandler* LockHandler(const char* id, void* iLocker, bool lockIt);
	bool Has(const char* id);
	bool Kick(const char* id);
	void NapTime(int t) {m_napTime = t;};
	int NapTime() { return m_napTime;};
	virtual int Process(void);
	int Size();
	int  GetState() {return m_inProcess;};

//private:
protected:
	RTVInactiveManager();

	int    m_napTime;
	InactiveHandlerMap m_inactiveHandlerMap;
	TRCriticalSection m_cs; //access control resource
	int m_inProcess;
	
	
};


#endif // !defined(RTVPoolAccess_H)