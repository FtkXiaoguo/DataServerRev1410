/***********************************************************************
 * JobInfo.cpp
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2006, All rights reserved.
 *
 *	PURPOSE: abstract interface for job info
 *		
 *
 *	AUTHOR(S):  Rob Lewis, March 2006
 *  
 *-------------------------------------------------------------------
 */


#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "JobInfo.h"
#include "JobInfoImplInterface.h"
#include  "AqCore/TRCommandLine.h"
#include "Job.h"

//----------------------------------------------------------
//
void JobInfoPublisher::SetProgress(int iCompleted, int iTotal)
{
	SetCompleted(iCompleted);
	SetTotal(iTotal);
	m_pPublisher->SetProgress(iCompleted, iTotal);
}

//----------------------------------------------------------
//
bool JobInfoPublisher::AmICancelled(void)
{
	return m_pPublisher->AmICancelled();
}

//----------------------------------------------------------
//
int JobInfoPublisher::SetInfo(KVP_MAP& iKVP)
{
	return m_pPublisher->SetInfo(iKVP);
}

//----------------------------------------------------------
//
int JobInfoPublisher::SetStatus(int iStatus)
{
	return m_pPublisher->SetStatus(iStatus);
}


//----------------------------------------------------------
//
bool JobInfoPublisher::NotifyOnlineDataReady()
{
	return m_pPublisher->NotifyOnlineDataReady();
}

//----------------------------------------------------------
//
bool JobInfoPublisher::NotifyOnlineDataFinished()
{
	return m_pPublisher->NotifyOnlineDataFinished();
}

//----------------------------------------------------------
//
int JobInfoPublisher::GetJobID()
{
	return m_pPublisher->GetJobID();
}

//----------------------------------------------------------
//
int JobInfoSubscriber::GetInfo(KVP_MAP& ioKVP)
{
	return m_pSubscriber->GetInfo(ioKVP);
}

//----------------------------------------------------------
//
int JobInfoSubscriber::GetStatus(int& oStatus)
{
	return m_pSubscriber->GetStatus(oStatus);
}

//----------------------------------------------------------
//
int JobInfoSubscriber::GetProgress(int& oCompleted, int& oTotal)
{
	return m_pSubscriber->GetProgress(oCompleted, oTotal);
}

//----------------------------------------------------------
//
int JobInfoSubscriber::Cancel(void)
{
	return m_pSubscriber->Cancel();
}

//----------------------------------------------------------
//
int JobInfoSubscriber::Hide(void)
{
	return m_pSubscriber->Hide();
}

//----------------------------------------------------------
//
void JobInfoSubscriber::Clean(void)
{
	m_pSubscriber->Clean();
}

//----------------------------------------------------------
//
std::string JobInfoSubscriber::GetName()
{	
	std::string ret = GetValue(kJOBKEYdisplayName);
	return (ret.size() > 0) ? ret : GetValue(kJOBKEYtargetProcessName);
}

//----------------------------------------------------------
//
std::string JobInfoSubscriber::GetValue(const char* iKey)
{	
	std::string ret;
	
	if (!iKey || !*iKey)
		return ret;
	
	KVP_MAP c;
	c[iKey] = "";
	GetInfo(c);
	KVP_MAP::iterator it;
	if ((it = c.find(iKey)) != c.end())
	{
		return it->second;
	}
	
	return ret;
	
}

//----------------------------------------------------------
//
std::string JobInfoSubscriber::GetInfo(std::vector<std::string> &oInfo)
{
	KVP_MAP c;
	GetInfo(c);
	KVP_MAP::iterator it;
	std::string s, all;

	for (it = c.begin(); it != c.end(); it++)
	{
		s = it->first + " = " + it->second;
		oInfo.push_back(s);
		all += s + "\n";
	}

	return all;
}
	
//----------------------------------------------------------
//
bool JobInfoSubscriber::IsOnlineDataReady()
{
	return m_pSubscriber->IsOnlineDataReady();
}

//----------------------------------------------------------
//
bool JobInfoSubscriber::IsOnlineDataFinished()
{
	return m_pSubscriber->IsOnlineDataFinished();
}