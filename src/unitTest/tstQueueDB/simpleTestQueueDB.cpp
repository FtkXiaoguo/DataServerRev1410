/***********************************************************************
 *
 */
 #include "PxQueue.h"

#include "Aqcore/trcriticalsection.h"
#include "AqCore/TRPlatform.h"

void initQueueDB();

void simpleTest1();
void testUpdateQueueDB(int id);
void testReadQueueDB(int id);
void simpleTest()
{
	{
 //		TRMutex *tt = new TRMutex("tttt");
	//	delete tt;
	}
	initQueueDB();
	for(int i=0;i<10;i++){

  		simpleTest1();

#if 0
		int id = 3;
	//	testUpdateQueueDB(id);
		testReadQueueDB(id);
#endif
	}
	int xx= 0;
}
void simpleTest1()
{
CPxWorkQueue queue;
//	CPxResultQueue queue;
 
	queue.initRes();
	

	queue.readQueueExt(1, CPxQueueEntry::PxQueueStatus_Standby, CPxQueueEntry::PxQueueStatus_Unknown);

	
#if 1
 // 	queue.selTest();	
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

	queue.createJobID(new_entry);

	printf(" **** CPxWorkQueue::selTest addQueue(new_entry) \n");
 
#if 1
  queue.addQueue(new_entry);
#else
	std::vector<int> recyceIDs;

	if(queue.getRecycleIDs(recyceIDs)){
		CPxQueueEntry entry_temp = new_entry;

		
		entry_temp.m_Status =  CPxQueueEntry::PxQueueStatus_Standby;
		
#if 1
		if(recyceIDs.size()>0){
 
			queue.changeStatus(recyceIDs[0],
					CPxQueueEntry::PxQueueStatus_Empty/* new status*/,
					CPxQueueEntry::PxQueueStatus_Empty/* current*/);
				 
//			queue.updateQueue( recyceIDs[0],  entry_temp, CPxQueueEntry::PxQueueStatus_Empty);
		}
#endif

	
	}
#endif

			
 
#endif

}

#include "PxDB.h"
extern std::string queue_db_fileName;
CPxDB gQueueDB;
void initQueueDB()
{
	gQueueDB.setupLocaleDBType(kDBType_SQLite);
	AqUString dbname_str;
	dbname_str.Format(L"%S",queue_db_fileName.c_str());
	gQueueDB.SetMyDBName(dbname_str);
	
}
void testUpdateQueueDB(int id )
{
	AqString	strSQL;

	AqString	strSQL_Update_Status;
	//
	time_t cur_time = time(0);
	struct tm tm_cur_temp = *localtime(&cur_time);

//	int id = 112;

	strSQL_Update_Status.Format(
//					"BEGIN TRANSACTION;" ///<- use TRANSACTION
					"UPDATE sendQueue SET "
					" Status = 2"
					

					" WHERE QueueID = %d"
//						"commit;"			///<- use TRANSACTION
					,
			
					//////////
					id);

	SQA sqa(gQueueDB.getDBType());
	sqa.SetCommandText(strSQL_Update_Status);
	int retcd = gQueueDB.SQLExecuteBegin(sqa);

	gQueueDB.SQLExecuteEnd(sqa);

}
void testReadQueueDB(int id)
{

	AqString	strSQL;
#if 1
	strSQL.Format(	"SELECT Status from sendQueue "
						" WHERE QueueID = %d",
						id );
#else
	strSQL.Format(	"SELECT Status from sendQueue ");
#endif

	AqString	strSQL_Update_Status;
	//
	{
		time_t cur_time = time(0);
		struct tm tm_cur_temp = *localtime(&cur_time);

	//	int id = 112;

		strSQL_Update_Status.Format(
	//					"BEGIN TRANSACTION;" ///<- use TRANSACTION
						"UPDATE sendQueue SET "
						" Status = 2"
						

						" WHERE QueueID = %d"
	//						"commit;"			///<- use TRANSACTION
						,
				
						//////////
						id);
	}

	SQA sqa(gQueueDB.getDBType());
		sqa.SetCommandText(strSQL);
		int retcd = gQueueDB.SQLExecuteBegin(sqa);
		//from here , DB will be locked until SQLExecuteEnd

		
		bool doFlag = false;

		if(retcd == kOK) {
			if(sqa.GetRecordCount()>0){
				int DB_cur_status = -1;
				retcd = sqa.MoveFirst(); 
				if(retcd == kOK){
					SQL_GET_INT(DB_cur_status, sqa);
 
					{
						doFlag = true;
					}
				}
			}
		}
		gQueueDB.SQLExecuteEnd(sqa);
						//now do it
	#if 1
		sqa.SetCommandText(strSQL_Update_Status);
		retcd = gQueueDB.SQLExecuteBegin(sqa);
		if(retcd == kOK) {
			 
		}
		gQueueDB.SQLExecuteEnd(sqa);
	#endif
				 

		

}