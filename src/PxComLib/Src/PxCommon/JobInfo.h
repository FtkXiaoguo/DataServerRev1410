/***********************************************************************
 * JobInfo.h
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

#ifndef JOB_INFO_H
#define JOB_INFO_H
#include <string>
#include <map>
#include <vector>
#include "AqCore/TRCommandLine.h"

typedef std::map<std::string, std::string> KVP_MAP;

//	NOTE: for internal implementation purposes...not for general use
class JobInfoPublisherInterface;
class JobInfoSubscriberInterface;


//----------------------------------------------------------
//
class TRProgress
{
public:
	TRProgress() : m_total(0), m_completed(0) {};
	virtual ~TRProgress() {};

	virtual void SetProgress(int iCompleted, int iTotal) {};
	virtual bool AmICancelled(void) { return false; };
	
	int GetCompleted(void) const { return m_completed; }
	int GetTotal(void) const { return m_total; }

protected:
	virtual void SetCompleted(int iCompleted) { m_completed = iCompleted; }
	virtual void SetTotal(int iTotal) { m_total = iTotal; }

	int m_completed;
	int m_total;
};


//----------------------------------------------------------
//
class JobInfoPublisher : public TRProgress
{
public:
	JobInfoPublisher(int iID = 0);
	virtual ~JobInfoPublisher();
	virtual int  GetJobID(void);

	virtual void SetProgress(int iCompleted, int iTotal);
	virtual bool AmICancelled(void);

	virtual int SetInfo(KVP_MAP& iKVP);
	virtual int SetStatus(int iStatus);

	virtual bool NotifyOnlineDataReady();
	virtual bool NotifyOnlineDataFinished();


	void SetCompleted(int iCompleted) { m_completed = iCompleted; }
	void SetTotal(int iTotal) { m_total = iTotal; }

private:
	JobInfoPublisherInterface* m_pPublisher;
};

//----------------------------------------------------------
//
class JobInfoSubscriber
{
public:
	JobInfoSubscriber(int iJobID, const char* iProcessName);
	JobInfoSubscriber(int iJobID, int iUserID);

	//	NOTE: only use this one if you really don't know userID or processName.  It could be very slow!!
	JobInfoSubscriber(int iID = 0);
	virtual ~JobInfoSubscriber();

	static int QueryJobs(KVP_MAP iConstraints, std::vector<int>& oJobIDs);

	static int QueryJobs(int iUserID, std::vector<int>& oJobIDs);

	static int QueryJobs(const char* iProcessName, std::vector<int>& oJobIDs);

	virtual int GetInfo(KVP_MAP& ioKVP);
	virtual int GetStatus(int& oStatus);
	virtual int GetProgress(int& oCompleted, int& oTotal);
	virtual int Cancel(void);
	virtual int Hide(void);
	virtual void Clean(void);

	//	use this to get the job's process name.  If it has a display name defined you'll get that
	//		otherwise, you'll get the target process name
	virtual std::string GetName(void);

	virtual std::string GetValue(const char* iKey);
	std::string GetInfo(std::vector<std::string> &oInfo);
	virtual bool IsOnlineDataReady();
	virtual bool IsOnlineDataFinished();

private:
	JobInfoSubscriberInterface* m_pSubscriber;
};

#endif // JOB_INFO_H
