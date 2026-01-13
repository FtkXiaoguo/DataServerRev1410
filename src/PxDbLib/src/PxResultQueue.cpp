#include "PxResultQueue.h"

#include "PxDB.h"

bool ExeSQLExecute(SQA &sqa,CPxDB *pDb,int retryNN);
CPxResultQueue::CPxResultQueue(void)
{
	m_needQueuEvent = false;//do not event queue
	m_mutex_sqliteDB = m_mutex_sqliteDB_result;
}

CPxResultQueue::~CPxResultQueue(void)
{
}
const char *CPxResultQueue::getQueueTableName() const
{
	return "resultQueue";
}
const char *CPxResultQueue::getChgQueueStatusProcName() const 
{
	return "ChgResultQueueStatus";
}

bool CPxResultQueue::addQueue(const CPxQueueEntry &entry)
{
	return addEntryToQueue( entry);
}
bool CPxResultQueue::deleteFinishedEntryWithTime(int  beforMinutes ,int beforHours , int beforDays)
{
	/*
	* delete the result record,
	* the record can be recycle used.
	*/
	time_t time_span = beforMinutes*60 + beforHours*60*60 + beforDays*24*60*60;
	time_t sel_time = time(0) - time_span;
	struct tm tm_temp = *localtime(&sel_time);

	
	AqString	strSQL;


	strSQL.Format( "UPDATE %s SET "
					" Status = %d"
					" WHERE AccessTime < '%04d-%02d-%02d %02d:%02d:%02d' " 
					" AND Status = %d ",
					getQueueTableName(),
					CPxQueueEntry::PxQueueStatus_Empty,
					///
					tm_temp.tm_year + 1900 ,
					tm_temp.tm_mon+1,
					tm_temp.tm_mday,
					tm_temp.tm_hour,
					tm_temp.tm_min,
					tm_temp.tm_sec,
					///
					CPxQueueEntry::PxQueueStatus_Finished
					);

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);
	int retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);

	return (retcd == kOK) ;
}
bool CPxResultQueue::deleteFinishedEntryWithMaxLen(int maxLen)
{
	std::vector<CPxQueueEntry> queueTemp;
	//update list
 
#if 0
	if(!readQueueSimpleEntry(queueTemp,CPxQueueEntry::PxQueuePriority_All,
				CPxQueueEntry::PxQueueStatus_Finished, CPxQueueEntry::PxQueueStatus_Failed)){
#else
	//Ž¸”s‚µ‚½Entry‚Ííœ‚µ‚È‚¢
	if(!readQueueSimpleEntry(queueTemp,CPxQueueEntry::PxQueuePriority_All,
				CPxQueueEntry::PxQueueStatus_Finished )){
#endif
		LogMessage(kErrorOnly," CPxResultQueue::deleteFinishedEntryWithMaxLen readQueueSimpleEntry failed");
		return false;
	}
	//process
	int queue_size = queueTemp.size();
	char _buff[64];
	if(queue_size>0){
		std::map<std::string,ResultQueueSeriesEntry> JobList;
		for(int i=0;i<queue_size;i++){
			const CPxQueueEntry &queue_entry = queueTemp[i];

			sprintf(_buff,"T%04d_",i);//ƒ\[ƒg‚³‚ê‚é
			std::string job_uid = CPxQueueProc::genJobUID(queue_entry);
			job_uid = _buff+job_uid ;
			if(JobList[job_uid].m_AccessTime > queue_entry.m_AccessTime){
				JobList[job_uid].m_AccessTime = queue_entry.m_AccessTime;
			}
			JobList[job_uid].m_JobID				= queue_entry.m_JobID;
			JobList[job_uid].m_imageCount++;
			JobList[job_uid].m_DestinationAE		= queue_entry.m_DestinationAE;
			JobList[job_uid].m_SeriesInstanceUID	= queue_entry.m_SeriesInstanceUID;
			JobList[job_uid].m_StudyInstanceUID		= queue_entry.m_StudyInstanceUID;
		}
		int job_list_size = JobList.size();

		LogMessage(kDebug,"job_list_size %d \n",job_list_size);
		 

		int del_size = job_list_size-maxLen;
		if(del_size>0){
			int del_count = 0;
			std::map<std::string,ResultQueueSeriesEntry>::iterator it = JobList.begin();
			while(del_count <del_size){
				const ResultQueueSeriesEntry &del_entry = it->second;
				deleteResultQueueSeriesEntry(del_entry);
				it++; del_count++;
			}
		}
		 
	}
	return true;
}
bool CPxResultQueue::deleteResultQueueSeriesEntry(const ResultQueueSeriesEntry& entry)
{
//	std::string AE = CPxQueueProc::getAEFromJobUID(entry.m_JobUID);
	AqString	strSQL;


	strSQL.Format( "UPDATE %s SET "
					" Status = %d"
					" ,StudyInstanceUID = ' '"
					" ,SeriesInstanceUID = ' '"
					" ,SOPInstanceUID = ' '"
					" WHERE JobID = '%s' " 
					" AND DestinationAE = '%s' "
					" AND SeriesInstanceUID = '%s' " ,
					getQueueTableName(),
					CPxQueueEntry::PxQueueStatus_Empty,
					///
					entry.m_JobID.c_str(),
					entry.m_DestinationAE.c_str(),
					entry.m_SeriesInstanceUID.c_str()
			
					);

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);
	int retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);


	return true;
}
void CPxResultQueue::selTest()
{
//	changeStatus(79,CPxQueueEntry::PxQueueStatus_Processing,CPxQueueEntry::PxQueueStatus_Empty);
//	deleteFinishedEntryWithTime(3,0,0);
	deleteFinishedEntryWithMaxLen(10);
}