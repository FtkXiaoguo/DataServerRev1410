#pragma once
#include "pxqueueproc.h"

class ResultQueueSeriesEntry
{
public:
	ResultQueueSeriesEntry(){
		m_AccessTime = 0x7fffffff;
		m_imageCount = 0;
	}
	std::string m_JobID;
 	std::string m_StudyInstanceUID ; 
 	std::string m_SeriesInstanceUID; 
	std::string m_DestinationAE;
	long m_AccessTime ;
	int m_imageCount;
};

class CPxResultQueue :
	public CPxQueueProc
{
public:
	CPxResultQueue(void);
	~CPxResultQueue(void);

	virtual void selTest();

	bool addQueue(const CPxQueueEntry &entry);
	bool deleteFinishedEntryWithTime(int beforMinutes ,int beforHours , int beforDays);
 
	bool deleteFinishedEntryWithMaxLen(int maxLen);
protected:
	bool deleteResultQueueSeriesEntry(const ResultQueueSeriesEntry& entry);
	virtual const char *getQueueTableName() const  ;
	virtual const char *getChgQueueStatusProcName() const  ;
};
