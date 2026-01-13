#include "ProcQueue.h"

#include "PxQueue.h"
 #include "AqCore/TRLogger.h"

class CMyPxQueueProc : public CPxWorkQueue
{
public:
	CMyPxQueueProc(void){
		m_owner = 0;
	};
	~CMyPxQueueProc(void){
	};

	CProcQueue *m_owner;
protected:
	virtual bool watchFilter(const CPxQueueEntry &entry)
	{ 
		if(m_owner){
			return m_owner->watchFilter( entry);
		}
		return true;
	};
	virtual bool doQueueWork(const CPxQueueEntry &entry)
	{
		if(m_owner){
			return m_owner->doQueueWork( entry);
		}
		return true;
	}
};
CProcQueue::CProcQueue(void)
{
	m_watchOnAE = "testAE1";
	m_Logger = new TRLogger;
	//
	
}

CProcQueue::~CProcQueue(void)
{
	if(m_Logger) delete m_Logger;
}

bool CProcQueue::watchFilter(const CPxQueueEntry &entry)
{
	if(entry.m_DestinationAE == m_watchOnAE){
		return true;
	}else{
		return false;
	}
}
bool CProcQueue::doQueueWork(const CPxQueueEntry &entry)
{
	m_Logger->LogMessage("%d, %s, %s, %s\n",m_countNN,
		(entry.m_StudyInstanceUID.size()>0) ? entry.m_StudyInstanceUID.c_str() : " ",
		(entry.m_SeriesInstanceUID.size()>0) ? entry.m_SeriesInstanceUID.c_str() : " ",
		(entry.m_SOPInstanceUID.size()>0) ? entry.m_SOPInstanceUID.c_str() : " ");
	m_Logger->FlushLog();

	m_countNN++;

	static bool ret_b = true; //fot debug
	return ret_b;
}
int	CProcQueue::Process(void)
{
	char str_buff[256];
	sprintf(str_buff,"./proc_%s.log",m_watchOnAE.c_str());
	m_Logger->SetLogFile(str_buff);
	m_Logger->SetLogLevel(3);

	m_Logger->LogMessage("No., study, series, instance\n");
	m_Logger->FlushLog();

	doInitDB();

	CMyPxQueueProc QueueProc;
#ifndef USE_MSSQL_SERVER_QUEUE
//	QueueProc.setupDBName(SQLITE_DB_FILTE);
#endif
	QueueProc.initRes();
 
	
	QueueProc.m_owner = this;

	int runCount = 0;
	while(!TerminationRequested())
	{	
		printf(" CProcQueue %d \n",runCount);

		QueueProc.watchQueue();

		if(runCount%10 == 0){
			QueueProc.recycleQueue(1);
		}
		runCount++;
		if(QueueProc.getQueueSize()<1){
			printf(" CProcQueue idle... \n");
			::Sleep(400);
		}
	}

	return 0;
}