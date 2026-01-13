#pragma warning (disable: 4503)

#include "PxNetDB.h"


#include "AddRequest.h"
#include "PxQueue.h"

CAddRequest::CAddRequest(void)
{
	m_AE = "testAE1";
	m_priority = CPxQueueEntry::PxQueuePriority_Default;
	m_loopInterval = 400;

	m_MyID = 1;
}

CAddRequest::~CAddRequest(void)
{
}
int	CAddRequest::Process(void)
{
	srand(time(0));

	doInitDB();

	CPxWorkQueue QueueProc;

	int runCount = 0;
	while(!TerminationRequested())
	{	
		printf(" CAddRequest %d \n",runCount);

		addReq();

		runCount++;

		if( ::rand()%3 == 0)
		{ 
			::Sleep(m_loopInterval*30);
		}
		::Sleep(m_loopInterval);
	}

	return 0;
}
 
void CAddRequest::addReq()
{
	char _str_buff[128];
	 
	sprintf(_str_buff,"121.322.222.%d.%d",m_MyID,m_countNN++);

	CPxWorkQueue QueueProc;
//	QueueProc.setupDBName(SQLITE_DB_FILTE);
	QueueProc.initRes();
 


	CPxQueueEntry new_entry;
	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Series;
	new_entry.m_StudyInstanceUID = " ";
	new_entry.m_SeriesInstanceUID = _str_buff;
	new_entry.m_DestinationAE = m_AE;
	new_entry.m_Priority = m_priority;
	new_entry.m_Status = 0;
	new_entry.m_CreateTime = time(0);
	new_entry.m_AccessTime = time(0);

	QueueProc.createJobID(new_entry);

	QueueProc.addQueue(new_entry);
}