#pragma once
#include "pxqueueproc.h"

class CPxResultQueue;
class CPxWorkQueue :
	public CPxQueueProc
{
public:
	CPxWorkQueue(void);
	virtual ~CPxWorkQueue(void);
	virtual void initRes();
	void selTest(void);
	//
	void setRetryMax(int max){ m_QurueRetryMax = max;};
	void setRetryInterval(int iv /*Sec*/){  m_QurueRetryInterval = iv;};
	void setRetryIntervalMax(int ivMax /*Sec*/){  m_QurueRetryIntervalMax = ivMax;}; // #80 2014/08/14 K.Ko
	/*
	*  watch the queue and process
	*/
 	bool watchQueue(int priority=CPxQueueEntry::PxQueuePriority_All);
	int getWatchedQueueSize() const { return m_watchedQueueSize;};
	int getDoQueueWorkCount () const { return m_doQueueWorkCount;};
	//
	bool resendQueue(int result_id);
	//
//	bool deleteQueue(int QueueID);
protected:
	//
	virtual const char *getQueueTableName()  const ;
	virtual const char *getChgQueueStatusProcName() const ;
		 
	virtual bool watchFilter(const CPxQueueEntry &entry) const { return true;};
	bool watchQueueWithPrioriy(int priority);
//	bool watchAllQueue();
	bool procQueue(const CPxQueueEntry &entry);
virtual bool doQueueWork(const CPxQueueEntry &entry){ return true;};
bool finishQueue(const CPxQueueEntry &entry,bool failedFlag);
//	void deleteEntryFile(const CPxQueueEntry &entry);
	//
	int m_QurueRetryMax;
	int m_QurueRetryInterval;//Sec
	int m_QurueRetryIntervalMax;//Sec// #80 2014/08/14 K.Ko
	//
	CPxResultQueue *m_resultQueue;
	//
	bool m_doQueueWork_flag;
	int m_doQueueWorkCount;
	int m_watchedQueueSize;
};
