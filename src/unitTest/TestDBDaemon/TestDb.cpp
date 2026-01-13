/***********************************************************************
 
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#define _WIN32_DCOM

#include "TestDb.h"

#include "Globals.h"
#include "rtvPoolAccess.h"

//#include "AQNetDB.h"
 

 CPxDcmDB	g_db;
 
//-----------------------------------------------------------------------------
//
char *ToUpper(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = toupper(*s);
	return ret;
}

//-----------------------------------------------------------------------------
//
static char* GetMyName(void)
{
	static char myName[128];

	if (myName[0])
		return myName;
	
	char *p;
	gethostname(myName, sizeof(myName)-1);
	if ( ( p = strchr(myName,'.')))
		*p = '\0';
	return myName;
}


TestDBProessBase::TestDBProessBase() 
{
		m_initDBFlag = false;
		m_PatientName = "testDb**";
	
}
void TestDBProessBase::destroy()
{
	delete this;
}
bool TestDBProessBase::doInitDB()
{
	if(m_initDBFlag){
		return true;
	}

	{
	bool testSQLite = true;
	if(testSQLite){
		g_db.setupDBType(kDBType_SQLite);

		CPxDcmDB::InitDBServerName(".\\test.db");

		if(!g_db.InitDatabaseInfo())
		{
			gLogger.FlushLog();
			printf("** DB::InitDatabaseInfo failed   \n" );
		 
			return 0;
		}else{
			gLogger.FlushLog();
			printf("** DB::InitDatabaseInfo OK   \n" );
		}
	}else{
		::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // for database to work in multi-thread mode


		gLogger.LogMessage("doInitDB %s \n" ,m_processorName);
		gLogger.FlushLog();

		if(!g_db.InitDatabaseInfo())
		{
			gLogger.LogMessage("** DB::InitDatabaseInfo failed %s \n",m_processorName);
			gLogger.FlushLog();
			return 0;
		}
	}
	}
	m_initDBFlag = true;

	return true;
}

  
int TestDBProessBase::Process(void)
{
	doInitDB();

 
	int timeoutCounts = 0;

	 m_run_count = 0;
	while(!TerminationRequested())
	{
		::Sleep(200);
	 
//		gLogger.LogMessage("TestDBProessBase: run_count %d [%s]\n",m_run_count,m_processorName);
//		gLogger.FlushLog();

		doDBProc();
		m_run_count++;
	}

	return 0;
}


bool TestDBProessBase::doSearchStudy()
{
DICOMData studyInfo;
pRTVSDicomInformation  iConstraints;
memset(&iConstraints,0,sizeof(iConstraints));
std::vector<pRTVSDicomInformation>  StudyOut;
 
 m_iGroupID = 1;//3;
 m_iUserID = 2;

strcpy(iConstraints.m_patientName,m_PatientName.c_str());
//int status = g_db.GetUserSeries(iGroupID,iUserID,oOut,iConstraints);
	 
try {
 

	int status = g_db.GetUserStudies(m_iGroupID,m_iUserID,StudyOut,iConstraints);

	int nn = StudyOut.size();

	if(m_run_count%10 == 0){
		gLogger.LogMessage("[%d]: TestDBProessBase::doSearchStudy(%s) %d is found  by %s \n", m_run_count ,m_PatientName.c_str(),nn,m_processorName);
		gLogger.FlushLog();
	}


		 
	 float rnd_temp = 	rand()/(float)RAND_MAX;
	 int rnd_i = rnd_temp * nn + 0.5;
 
	 for(int i=0;i<nn;i++){
		 if(i !=  rnd_i) continue;
		 doSearchSeries(StudyOut[i].m_studyUID);
	 }
	 

	 StudyOut.clear();
}catch(...)
{
	int error =1;
}

	return true;
}

void TestDBProessBase::setupStudyDescription(std::string studyUID,std::string des)
{
	 
		
	SQA sqa;
	sqa.FormatCommandText("Update dbo.StudyLevel set StudyDescription = '%s' WHERE StudyInstanceUID = '%s' ", des.c_str(),studyUID.c_str());		
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	
	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd != kOK)  return  ;
	g_db.SQLExecuteEnd(sqa);


}
bool TestDBProessBase::doSearchSeries(std::string study_uid)
{
	pRTVSDicomInformation  iConstraints;
	memset(&iConstraints,0,sizeof(iConstraints));

	std::vector<pRTVSDicomInformation>  SeriesOut;
 
	strcpy(iConstraints.m_studyUID,study_uid.c_str());
	int status = g_db.GetUserSeries(m_iGroupID,m_iUserID,SeriesOut,iConstraints);

	int nn = SeriesOut.size();
 
	 for(int i=0;i<nn;i++){
		 //doSearchSeries(StudyOut[i].m_studyUID);
		 int instance_nn = g_db.GetNumberOfInstancesInSeries(SeriesOut[i].m_seriesUID);
		
	 }
	 procSeries(SeriesOut);

	 SeriesOut.clear();

	return true;
}

bool TestDBProessBase::procSeries(std::vector<pRTVSDicomInformation> & SeriesOut)
{
	std::vector<DICOMInstanceX>  oVal;
	DICOMData  iFilter;

	int nn = SeriesOut.size();
 
	 for(int i=0;i<nn;i++){
		 //doSearchSeries(StudyOut[i].m_studyUID);
	//	 int instance_nn = g_db.GetNumberOfInstancesInSeries(SeriesOut[i].m_seriesUID);
		
		
		memset(&iFilter,0,sizeof(iFilter));
		strcpy(iFilter.m_seriesInstanceUID,SeriesOut[i].m_seriesUID);

		const char* iSeriesInstanceUID = SeriesOut[0].m_seriesUID;
		int status =g_db.GetInstanceList(oVal, &iFilter,iSeriesInstanceUID);

		int imageNum = oVal.size();
		{
			for(int ii=0;ii<imageNum;ii++){
				const char* iSopInstanceUID = "**";// oVal[ii].m_SOPInstanceUID;
	//			printf(" [%d]: (%d, %d) \n",ii,oVal[ii].m_columns,oVal[ii].m_rows);
				int	sss ;
	//			 g_db.GetInstanceStatus(iSeriesInstanceUID, iSopInstanceUID, sss);
	//			g_db.SetInstanceStatus(iSeriesInstanceUID, iSopInstanceUID, sss);

			}
		}
	 
	 }


 

	
#if 0
	 int	GetInstanceList( std::vector<DICOMInstance>& oVal, const DICOMData*  iFilter, const char* iSeriesInstanceUID);
	int	GetInstanceList( std::vector<DICOMInstanceX>& oVal, const DICOMData*  iFilter, const char* iSeriesInstanceUID, int iNInstace=0);
	int	GetInstanceStatus(const char* iSeriesUID, const char* iSopInstanceUID, int& iStatus);
	int	SetInstanceStatus(const char* iSeriesUID, const char* iSopInstanceUID, int iStatus);
#endif
	return true;
}

//-----------------------------------------------------------------------------
//
#if 0
TestDBProcessList& TestDBProcessList::theTestDBProcess()
{
static TestDBProcessList p;
  
	 // the signle TestDBProcessList object
	return p;

}
#endif

TestDBProcessList* TestDBProcessList::createInstace()
{
	TestDBProcessList *p = new TestDBProcessList ;
  
	 // the signle TestDBProcessList object
	return p;

}

//-----------------------------------------------------------------------------
//
TestDBProcessList::TestDBProcessList()
{
	m_processorName = "TestDBProcessList";
}

//-----------------------------------------------------------------------------
//
int	TestDBProcessList::PreProcess(void)
{ 
 
 
	return 0;

}
 
 bool TestDBProcessList::doDBProc()
 {
	   
	 doSearchStudy();
	return true;
 }

//////////////
//-----------------------------------------------------------------------------
//
#if 0
TestDBProcessUpdate& TestDBProcessUpdate::theTestDBProcess()
{
	static TestDBProcessUpdate p; // the signle TestDBProcessList object
	return p;

}
#endif

TestDBProcessUpdate* TestDBProcessUpdate::createInstace()
{
	TestDBProcessUpdate *p = new TestDBProcessUpdate;
  
	 // the signle TestDBProcessList object
	return p;

}

//-----------------------------------------------------------------------------
//
TestDBProcessUpdate::TestDBProcessUpdate()
{
	m_processorName = "TestDBProcessUpdate";
}


//-----------------------------------------------------------------------------
//
int	TestDBProcessUpdate::PreProcess(void)
{ 
 

	return 0;

}
 
 bool TestDBProcessUpdate::doDBProc()
 {
	 doSearchStudy();

	 //doUpdate();

	 return true;
 }
 

bool TestDBProcessUpdate::doSearchSeries(std::string study_uid)
{
	char _str_buff[512];
	sprintf(_str_buff,"doSS%d",m_run_count);
	_str_buff[63] = 0;

	g_db.UpdateStudyAccessTime(study_uid.c_str());
	
	setupStudyDescription(study_uid,_str_buff);

	return TestDBProessBase::doSearchSeries( study_uid);

 
}
 bool TestDBProcessUpdate::procSeries(std::vector<pRTVSDicomInformation> & SeriesOut)
 {
	 TestDBProessBase::procSeries( SeriesOut);

	 /////////////
	 {
		std::vector<DICOMInstanceX>  oVal;
		DICOMData  iFilter;

		int nn = SeriesOut.size();
 
		 for(int i=0;i<nn;i++)
		 {
 		
	 		 g_db.AddNewSeries(SeriesOut[i].m_seriesUID);
		 
	//		 g_db.MarkBadSeries(SeriesOut[i].m_seriesUID);

			 CPxDB::eSeriesStatus stus = g_db.GetSeriesStatus(SeriesOut[i].m_seriesUID);
			 //
			 g_db.MarkBadSeries(SeriesOut[i].m_seriesUID);
			 //
			 g_db.SetSeriesStatus(SeriesOut[i].m_seriesUID,stus);

		 }

	 }

 
 
	 return true;
	 /////////////
 }

 bool TestDBProcessUpdate::doUpdate()
 {
	 DICOMData studyInfo;

	 std::string studyUID;
	 if (g_db.UpdateStudyInfo(studyInfo) != kOK)
	{
		gLogger.LogMessage("  Update StudyInfo failed in Overwrite duplicates. studyUID=%s\n",studyUID.c_str());
		gLogger.FlushLog();
	}

//	 CPxDB::AddNewSeries(const char*  iSeriesUID);

	 return true;
 }

 ///////////////
 
TestDBProcessBigDb* TestDBProcessBigDb::createInstace()
{
	TestDBProcessBigDb *p = new TestDBProcessBigDb;
  
	 // the signle TestDBProcessList object
	return p;

}

//-----------------------------------------------------------------------------
//
bool TestDBProcessBigDb::m_creatBigDBFlag = false;
TestDBProcessBigDb::TestDBProcessBigDb()
{
	m_testBigDBLen = 10;
	m_processorName = "TestDBProcessBigDb";
}
int	 TestDBProcessBigDb::PreProcess(void)
{
	return 0;
}
bool TestDBProcessBigDb::doCreateTestBigDB()
{
	if(m_creatBigDBFlag) return true;

	int MBytes = ((float)m_testBigDBLen * STR_MAX_LEN * STR_FILED_LEN)/(1024.0*1000) +0.5;

	gLogger.LogMessage("  ====== doCreateTestBigDB %d about %d MB\n",m_testBigDBLen,MBytes);
	gLogger.FlushLog();

	dispLastData();

#if 0
	for(int run_i=0;run_i<m_testBigDBLen;run_i++){
		SQA sqa;
 		
		sqa.FormatCommandText("IF NOT EXISTS (SELECT BigDbID FROM dbo.testBigDb  WHERE intDat1=%d ) "
			"INSERT dbo.testBigDb (intDat1, intDat2,intDat3, intDat4,bitDat1,bitDat2,bitDat3) "
			"VALUES (%d, %d, %d, %d,' ',' ',' ')", 
			run_i+1,
			run_i, 2,3,4);
 		
		int retcd = g_db.SQLExecuteBegin(sqa);
		if(retcd != kOK)  return  false;
		g_db.SQLExecuteEnd(sqa);

	}

#endif

	gLogger.LogMessage("  ====== doCreateTestBigDB END ===\n" );
	gLogger.FlushLog();

	m_creatBigDBFlag = true;
	return true;
}
bool TestDBProcessBigDb::doDBProc()
{

	try {

 	doCreateTestBigDB();

	int op_mode = rand()%4;


	int index = (rand()  + rand() + rand() + rand() )/(4.0*RAND_MAX) * m_testBigDBLen + 1;

	int field_index = rand() %(STR_FILED_LEN);
	switch(op_mode){
	case 0: // insert
		{
			if(!doInsertBigDB( index)){

				gLogger.LogMessage("  ====== doInsertBigDB %d error\n",index);
 				gLogger.FlushLog();

				return false;
			}else{
				gLogger.LogMessage("  ====== doInsertBigDB %d OK\n",index);
 				gLogger.FlushLog();
			}
		}
		break;
	case 1: // update
		{
			if(!doUpdateBigDB( index,field_index)){
				gLogger.LogMessage("  ====== doUpdateBigDB %d error\n",index);
 				gLogger.FlushLog();

				return false;
			}else{
				gLogger.LogMessage("  ====== doUpdateBigDB %d OK\n",index);
 				gLogger.FlushLog();
			}
		}
		break;
	//
	case 2: // check
		{
			if((m_run_count%20) ==0) {
				
				if(!doCheckBigDB(index, field_index)){
					gLogger.LogMessage("  ====== doCheckBigDB %d error\n",index);
 					gLogger.FlushLog();

					return false;
				}else{
					gLogger.LogMessage("  ====== doCheckBigDB %d OK\n",index);
 					gLogger.FlushLog();
				}
			}
		}
		break;
	case 3: // delete
		{
			if(!doDeleteBigDB(index)){
				gLogger.LogMessage("  ====== doDeleteBigDB %d error\n",index);
				gLogger.FlushLog();

				return false;
			}else{
				gLogger.LogMessage("  ====== doDeleteBigDB %d OK\n",index);
				gLogger.FlushLog();
			}
			 
		}
		break;

	}

	}catch(...){
		return false;
	}
	return true;
}

bool TestDBProcessBigDb::doInsertBigDB(int index)
{
//	gLogger.LogMessage("  ====== doCreateTestBigDB %d about %d MB\n",m_testBigDBLen,MBytes);
//	gLogger.FlushLog();

	 
		SQA sqa;
 		
#if 0
		sqa.FormatCommandText("IF NOT EXISTS (SELECT BigDbID FROM testBigDb  WHERE intDat1=%d ) "
			"INSERT INTO testBigDb (intDat1, intDat2,intDat3, intDat4,bitDat1,bitDat2,bitDat3) "
			"VALUES (%d, %d, %d, %d,' ',' ',' ')", 
			index ,
			index, 2,3,4);
		int retcd = g_db.SQLExecuteBegin(sqa);
		if(retcd != kOK)  return  false;
		g_db.SQLExecuteEnd(sqa);

#else
		if(findBigDB(index)) return true;
		

		//
		sqa.FormatCommandText("INSERT INTO testBigDb (intDat1, intDat2,intDat3, intDat4,bitDat1,bitDat2,bitDat3) "
			"VALUES (%d, %d, %d, %d,' ',' ',' ')", 
			index ,
			index, 2,3,4);

		sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);

		int retcd = kFailedUnknown;
		for(int run_i=0;run_i<20;run_i++){
			  retcd = g_db.SQLExecuteBegin(sqa);
			 g_db.SQLExecuteEnd(sqa);
			 if(retcd == kOK) {
				 break;
			 }else{
				 printf(" run_i %d \n",run_i);
			 }
		}
		if(retcd != kOK)  return  false;
		
		 
#endif
 		
		
		return true;

}
bool TestDBProcessBigDb::dispLastData()
{
 
	SQA sqa;
	sqa.FormatCommandText("SELECT intDat2  FROM testBigDb  ORDER BY intDat2 DESC" );		
 
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);

	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		gLogger.LogMessage("   dispLastData error \n");
		gLogger.FlushLog();
		g_db.SQLExecuteEnd(sqa);

		return  false;
	}
	int size = sqa.GetRecordCount(); if(size < 1) return false;
	retcd = sqa.MoveFirst();
	if(retcd != kOK){
		gLogger.LogMessage("   dispLastData MoveFirst error \n");
		gLogger.FlushLog();
		g_db.SQLExecuteEnd(sqa);

		return  false;
	}
	int data2;
 
	SQL_GET_INT(data2, sqa);
 
	g_db.SQLExecuteEnd(sqa);


	time_t timeReturn  = data2;//(long)(data3<<16) + data2;
 
	 struct tm*    timePtr = localtime(&timeReturn);

 
	gLogger.LogMessage(" >>== dispLastData   %04d/%02d/%02d,%02d:%02d\n",
	 
		timePtr->tm_year + 1900,
		timePtr->tm_mon + 1,
		timePtr->tm_mday,
		//
		timePtr->tm_hour,
        timePtr->tm_min,
        timePtr->tm_sec
		);
	gLogger.FlushLog();
	

	return true;

}
bool TestDBProcessBigDb::doCheckBigDB(int index,int field_index)
{

	if(!findBigDB(index)) return true;
		
	char _str_buff[STR_MAX_LEN];

	SQA sqa;
	sqa.FormatCommandText("SELECT intDat2, intDat3, bitDat%d FROM testBigDb  WHERE intDat1=%d ", 
		field_index+1,
		index );		
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);

	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		gLogger.LogMessage("   doCheckBigDB error \n");
		gLogger.FlushLog();
		g_db.SQLExecuteEnd(sqa);
		return  false;
	}
	int size = sqa.GetRecordCount(); if(size < 1) return false;
	retcd = sqa.MoveFirst();
	if(retcd != kOK){
		gLogger.LogMessage("   doCheckBigDB MoveFirst error \n");
		gLogger.FlushLog();
		g_db.SQLExecuteEnd(sqa);
		return  false;
	}
	int data2;
	int data3;
	SQL_GET_INT(data2, sqa);
	SQL_GET_INT(data3, sqa);
	SQL_GET_STR(_str_buff, sqa);

	_str_buff[16] = 0;
	g_db.SQLExecuteEnd(sqa);


	time_t timeReturn  = data2;//(long)(data3<<16) + data2;
 
	 struct tm*    timePtr = localtime(&timeReturn);

	
	gLogger.LogMessage(" doCheckBigDB rec[%d], filed[%d] [%s] date: %04d/%02d/%02d,%02d:%02d\n",
		index,field_index+1,_str_buff,
		timePtr->tm_year + 1900,
		timePtr->tm_mon + 1,
		timePtr->tm_mday,
		//
		timePtr->tm_hour,
        timePtr->tm_min,
        timePtr->tm_sec
		);
	gLogger.FlushLog();
	
	return true;
}
bool TestDBProcessBigDb::doDeleteBigDB(int index)
{
	if(!findBigDB(index)) return true;
		

	SQA sqa;
	sqa.FormatCommandText("DELETE FROM testBigDb WHERE intDat1=%d ", index);		

	 sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);

	int retcd = g_db.SQLExecuteBegin(sqa);
		
	g_db.SQLExecuteEnd(sqa);
	if(retcd != kOK)  return  false;
		

	return true;
}
bool TestDBProcessBigDb::doUpdateBigDB(int index,int field_index)
{

	if(!findBigDB(index)) return true;
		
	char _str_buff[STR_MAX_LEN];

	sprintf(_str_buff,"[%d],data%d ::",index, m_run_count);
	for(int ii =strlen(_str_buff);ii<STR_MAX_LEN;ii++){
		_str_buff[ii] = 'a'+ii%20 ;
	}
	_str_buff[STR_MAX_LEN-2] = 0;

	  

	//
	 time_t timeReturn = time(NULL);
 
	 int data2 = timeReturn ;//&(0xffff) ;
	 int data3 = (timeReturn &(0xffff0000)) >> 16 ;
	 

   
	SQA sqa;
	sqa.FormatCommandText("Update testBigDb set intDat2=%d, intDat3=%d, bitDat%d = '%s' WHERE intDat1 = '%d' ", 
		data2,
		data3,
		field_index+1,
		_str_buff,
		index );		
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);

	int retcd = g_db.SQLExecuteBegin(sqa);
		
	g_db.SQLExecuteEnd(sqa);

	if(retcd != kOK)  return  false;
	//
			
		
		return true;
}

bool TestDBProcessBigDb::findBigDB(int index)
{
	SQA sqa;
	int size = 0;
	sqa.FormatCommandText("SELECT BigDbID FROM testBigDb WHERE intDat1=%d ",index);
	 
	//sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd == kOK){
		size = sqa.GetRecordCount();
	}
	g_db.SQLExecuteEnd(sqa);
	return (size>0) ;
}