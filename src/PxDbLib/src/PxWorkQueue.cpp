#include "PxWorkQueue.h"
#include "PxDB.h"
#include "AqCore/TRPlatform.h"

#include "PxResultQueue.h"

CPxWorkQueue::CPxWorkQueue(void)
{
	m_mutex_sqliteDB = m_mutex_sqliteDB_send;
	m_resultQueue = new CPxResultQueue ;
	m_QurueRetryMax = 5;
	m_QurueRetryInterval = 10;//Sec
	m_QurueRetryIntervalMax = 30;//Sec// #80 2014/08/14 K.Ko
}

CPxWorkQueue::~CPxWorkQueue(void)
{
	delete m_resultQueue;
}

void CPxWorkQueue::initRes()
{
	CPxQueueProc::initRes();

	if(m_resultQueue){
#ifndef USE_MSSQL_SERVER_QUEUE
//		m_resultQueue->setupDBName(m_DBName);
#endif
		m_resultQueue->initRes();

	}
}
const char *CPxWorkQueue::getQueueTableName() const
{
	return "sendQueue";
}
const char *CPxWorkQueue::getChgQueueStatusProcName() const 
{
	return "ChgReadQueueStatus";
}
 
bool CPxWorkQueue::watchQueueWithPrioriy(int priority)
{
	std::vector<CPxQueueEntry> queueTemp;
	//update list
	if(!readQueueSimpleEntry(
			queueTemp,
			priority,//CPxQueueEntry::PxQueuePriority_All,
			CPxQueueEntry::PxQueueStatus_Standby,
			CPxQueueEntry::PxQueueStatus_Failed)){
		LogMessage(kErrorOnly," CPxWorkQueue::watchQueueWithPrioriy readQueueSimpleEntry failed \n");
		return false;
	}
	//process
	m_doQueueWorkCount = 0;

	int queue_size = queueTemp.size();
	if(queue_size>0){
		
		//at first
		//do priority > PxQueuePriority_Low
		int do_priority_count = 0;
		for(int i=0;i<queue_size;i++){
			bool do_first_flag = true;
			if(priority == CPxQueueEntry::PxQueuePriority_All){
			//one thread  for all priority
				do_first_flag =  (queueTemp[i].m_Priority > CPxQueueEntry::PxQueuePriority_Low);
			}
			if(do_first_flag){
				if(m_breakFlag) break;

				m_doQueueWork_flag = false;
				if(watchFilter(queueTemp[i]) ){
				 
					if(!procQueue(queueTemp[i])){
						LogMessage(kErrorOnly," procQueue failed");
					}
				}
				if(m_doQueueWork_flag){
					m_doQueueWorkCount++;
				}
				do_priority_count++;
			}
		}
		if(	(priority == CPxQueueEntry::PxQueuePriority_All)&& //one thread  for all priority
			(do_priority_count < queue_size))
		{
		//then 
		//do priority == PxQueuePriority_Low
			for(int i=0;i<queue_size;i++){
				if(queueTemp[i].m_Priority == CPxQueueEntry::PxQueuePriority_Low){
				
					if(m_breakFlag) break;

					m_doQueueWork_flag = false;
					if(watchFilter(queueTemp[i]) ){
					 
						if(!procQueue(queueTemp[i])){
							LogMessage(kErrorOnly," procQueue failed");
						}
					}
					if(m_doQueueWork_flag){
						m_doQueueWorkCount++;
					}
				}
			}
		}

	}
	m_watchedQueueSize = queue_size;
	return true;
}
bool CPxWorkQueue::watchQueue(int priority)
{
	m_runCount++;

	int ret_b = true;
 
	ret_b = watchQueueWithPrioriy(priority);
 

	return ret_b;
}
bool CPxWorkQueue::procQueue(const CPxQueueEntry &entry)
{
	LogMessage(kInfo," procQueue %d",entry.m_QueueID);

	 
	 
	SQA *iSqa= 0;
	
 	CPxQueueEntry entry_temp;

	int ret_b = false;
	int break_flag = false;
	try {
 
#if 0
		/*
		* Full Entry を読み込む
		*/
		if(!readOneEntry(entry.m_QueueID,entry_temp)){
			LogMessage(kErrorOnly," readOneEntry error");
			throw(-1);
			 
		}
		int status =entry_temp.m_Status;
#else
		int status =entry.m_Status;
#endif

		bool do_flag = false;
		//at first set status as PxQueueStatus_Processing
		if(	(status  == CPxQueueEntry::PxQueueStatus_Standby)){
			do_flag = true;
		}else if (status == CPxQueueEntry::PxQueueStatus_Failed){
			time_t cur_time = time(0) ;
			int diff_time = cur_time - entry.m_AccessTime;
			if(diff_time<0)  diff_time=-diff_time;

			int interval_time = m_QurueRetryInterval*entry.m_RetryCount;
			if(interval_time>m_QurueRetryIntervalMax){ // #80 2014/08/14 K.Ko 
				interval_time = m_QurueRetryIntervalMax ;
			}

			if(diff_time>=interval_time){ //m_QurueRetryInterval){
				do_flag = true;
			}else{
				do_flag = false;
				 
#if 0
				//back the entry;
				if(!updateQueue(entry.m_QueueID,entry)){
					LogMessage(kErrorOnly," updateQueue error");
					throw(-1);
				}
#endif
			}
		//	if(m_retryInterval
		} 

		 

		if(do_flag){
			///////////////////
			/*
			* Full Entry を読み込む
			*/
			bool read_f = false;
			for(int try_i=0;try_i<m_dbEexRetryNN;try_i++){
				read_f = readOneEntry(entry.m_QueueID,entry_temp);
				if(read_f) break;
			}
			if(!read_f){
				LogMessage(kErrorOnly," readOneEntry error");
				throw(-1);
				 
			}
			 
			status =entry_temp.m_Status;
			///////////////////
			if(	(status  == CPxQueueEntry::PxQueueStatus_Standby) ||
				(status  == CPxQueueEntry::PxQueueStatus_Failed )
				)
			{
				; // do it
			}else{
			//途中変更されたケース
				throw(1);
			}

			if(false ==changeStatus(entry.m_QueueID,
									CPxQueueEntry::PxQueueStatus_Processing,
									status
										)){
				//maybe , other process is doing it.
				// do not re-try here, just return. 
				throw(1);
				 
			};
		}else{
			//alread processed
			throw(1);
			 
		}
	}catch(int errorID){
		break_flag = true;
		ret_b = (errorID>=0);
	}
	
	if(break_flag){
		return ret_b;
	}
	//////////////

	if(m_breakFlag) {//#82 2014/09/29 K.Ko
 		return ret_b;
	}
	//do it ...
//	m_doQueueWork_flag =  doQueueWork(entry)  ;
	m_doQueueWork_flag =  doQueueWork(entry_temp);
	
	
//	if(!finishQueue( entry,!do_sucess_flag/*failedFlag*/)){
	//use entry_temp !
	if(!finishQueue( entry_temp,!m_doQueueWork_flag/*failedFlag*/)){
	
		return false;
	}

	return true;
}
  
bool CPxWorkQueue::finishQueue(const CPxQueueEntry &entry,bool failedFlag)
{
	bool ret_b = false;
	//////////////
#ifndef USE_RECYECLEQUEUE
	CPxQueueEntry::PxQueueStatus theFinishedStatus = CPxQueueEntry::PxQueueStatus_Empty;
#else
	CPxQueueEntry::PxQueueStatus theFinishedStatus = CPxQueueEntry::PxQueueStatus_Finished;
#endif
	if(failedFlag){
		if(entry.m_RetryCount >= m_QurueRetryMax){
			//move to resultQueue with faile
			if(m_resultQueue){
				CPxQueueEntry entry_temp = entry;
 
				entry_temp.m_Status = CPxQueueEntry::PxQueueStatus_Failed;
#if 0
				if(!m_resultQueue->addQueue(entry_temp)){
					ret_b = false;
				}else{

					ret_b = changeStatus(entry.m_QueueID,theFinishedStatus);
				}
#endif
				///
				for(int i=0;i<m_dbEexRetryNN;i++){
					if(!m_resultQueue->addQueue(entry_temp)){
						ret_b = false;
					}else{
						ret_b = true;
						break;
					}
				}
			 
				if(ret_b){
					for(int i=0;i<m_dbEexRetryNN;i++){
	 					ret_b = changeStatus(entry.m_QueueID,theFinishedStatus);
						if(ret_b) break;
					}
				}

				//if (ret_b)
				{
					TCHAR szBuffer[1024*4] = "";
					HKEY hKey = NULL;
					const TCHAR *subkey = _T("SOFTWARE\\PreXion\\PXDataServer");
					const TCHAR* pQueryKey   = _T("QueueProcError");
					DWORD dwDisposition = 0;
					const INT value = 0x01;
					long result=RegCreateKeyEx(HKEY_LOCAL_MACHINE,subkey,0, NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey, &dwDisposition);
					if(result==ERROR_SUCCESS){
						result=RegSetValueEx(hKey,pQueryKey, 0,REG_DWORD_LITTLE_ENDIAN,(CONST BYTE *)&value,(DWORD)sizeof(value));
						RegCloseKey(hKey);
					}
					_stprintf(szBuffer, _T("%s: RegSetValueEx=%s\\%s=%d, %s"), __FUNCTION__, subkey, pQueryKey, value, result==0?("SUCCESS"):("FAILED"));
					OutputDebugString(szBuffer);
				}
			}else{
				ret_b = false;
			}
		}else{
			//retry count
			CPxQueueEntry entry_temp = entry;
			entry_temp.m_RetryCount++;
			
			entry_temp.m_Status		= CPxQueueEntry::PxQueueStatus_Failed;
			entry_temp.m_Priority	= CPxQueueEntry::PxQueuePriority_Low ; //失敗したものに対して、Priority をLowにする。
			entry_temp.m_AccessTime = time(0);
			 
			for(int i=0;i<m_dbEexRetryNN;i++){
				ret_b = updateQueue(entry.m_QueueID,entry_temp);
				if(ret_b) break;
			}
		}
	}else{
		//move to resultQueue with sucessed
		if(m_resultQueue){

			CPxQueueEntry entry_temp = entry;
 
			entry_temp.m_Status = CPxQueueEntry::PxQueueStatus_Finished;

			for(int i=0;i<m_dbEexRetryNN;i++){
				if(!m_resultQueue->addQueue(entry_temp)){
					ret_b = false;
				}else{
					ret_b = true;
					break;
				}
			}
		 
			if(ret_b){
				for(int i=0;i<m_dbEexRetryNN;i++){
	 				ret_b = changeStatus(entry.m_QueueID,theFinishedStatus);
					if(ret_b) break;
				}
			}
 
			if(ret_b){
				if(entry.m_SendLevel == CPxQueueEntry::PxQueuelevel_EntryFile){
					deleteEntryFile(entry);
				}
			}
		}else{
			ret_b = false;
		}
	}

	return ret_b;
}

bool CPxWorkQueue::resendQueue(int result_id)
{
	if(!m_resultQueue) return false;

	CPxQueueEntry entry_temp ;
	if(!m_resultQueue->readOneEntry(result_id,entry_temp)){
		return false;
	}
	m_resultQueue->changeStatus(result_id,CPxQueueEntry::PxQueueStatus_Empty,entry_temp.m_Status);
	
	entry_temp.m_Status		= CPxQueueEntry::PxQueueStatus_Standby;
	entry_temp.m_Priority	= CPxQueueEntry::PxQueuePriority_Default ;  
	entry_temp.m_RetryCount = 0;
	entry_temp.m_AccessTime	= time(0);

	addQueue(entry_temp);
	//
	
	return true;
}

void CPxWorkQueue::selTest()
{
	printf(" **** CPxWorkQueue::selTest -- start \n");
	char _str_buff[128];
	 
	sprintf(_str_buff,"121.322.222.%d.%d",2,3);

	 
	CPxQueueEntry new_entry;
	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Series;
	new_entry.m_StudyInstanceUID = "";
	new_entry.m_SeriesInstanceUID = _str_buff;
	new_entry.m_DestinationAE = "testAE";
	new_entry.m_Priority = 1;
	new_entry.m_Status = 0;
	new_entry.m_CreateTime = time(0);
	new_entry.m_AccessTime = time(0);

	createJobID(new_entry);

	printf(" **** CPxWorkQueue::selTest addQueue(new_entry) \n");
 	addQueue(new_entry);

	{
		{
			CPxQueueBinFile queue_bin_file;
			 
			queue_bin_file.m_seriesFolder = "test\\tttt";
			 
			queue_bin_file.m_imageFileList.resize(2);
			queue_bin_file.m_imageFileList[0] = "image1";
			queue_bin_file.m_imageFileList[1] = "image2";
			queue_bin_file.writeQueueBinFile("test.entry.bin");
		}
		//
		{
			CPxQueueBinFile queue_bin_file;
			 
			queue_bin_file.readQueueBinFile("test.entry.bin");
			 
			int xx = queue_bin_file.m_imageFileList.size();;
			 
		}
		//
	}
	printf(" **** CPxWorkQueue::selTest watchAllQueue\n");
	watchQueue(CPxQueueEntry::PxQueuePriority_All);
	printf(" **** CPxWorkQueue::selTest -- end \n");
}

