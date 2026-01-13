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

#include "PxNetDB.h"
 
#include <string>
#include <codecvt>
#include <locale>
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
		m_LangID = LANG_ID_Unknown;
		m_loopInterval = 200;
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
		::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // for database to work in multi-thread mode


		gLogger.LogMessage("doInitDB %s \n" ,m_processorName);
		gLogger.FlushLog();

		if(!g_db.InitDatabaseInfo())
		{
			gLogger.LogMessage("** DB::InitDatabaseInfo failed %s \n",m_processorName);
			gLogger.FlushLog();
			return 0;
		}
		CPxDcmDB::InitCharacterLib();
	}
	m_initDBFlag = true;

	return true;
}

  
int TestDBProessBase::Process(void)
{
	doInitDB();

 
	int timeoutCounts = 0;

	 m_run_count = 0;
//	while(!TerminationRequested())
	{
		::Sleep(m_loopInterval);
	 
//		gLogger.LogMessage("TestDBProessBase: run_count %d [%s]\n",m_run_count,m_processorName);
//		gLogger.FlushLog();

		doDBProc();
		m_run_count++;
	}

	return 0;
}

bool TestDBProessBase::doSearchBitData(void)
{
	SQA sqa;
	sqa.FormatCommandText("SELECT TOP 10000  *  FROM dbo.testBigDb  WHERE intDat1>200 and intDat1>0500 and bigIn1='yyy' and bigIn2='ggg' ORDER BY intDat2 DESC ");

	DWORD start_time = ::GetTickCount();

	int retcd = g_db.SQLExecuteBegin(sqa);
	if (retcd != kOK) {
		gLogger.LogMessage("   dispLastData error \n");
		gLogger.FlushLog();
		return  false;
	}

	DWORD end_time = ::GetTickCount();
	float spent_time = end_time / 1000.0 - start_time / 1000.0f;
	printf("search DB %.2f Sec \n", spent_time);

	int size = sqa.GetRecordCount(); if (size < 1) return false;
	retcd = sqa.MoveFirst();
	if (retcd != kOK){
		gLogger.LogMessage("   dispLastData MoveFirst error \n");
		gLogger.FlushLog();
		return  false;
	}
	int data2;

	SQL_GET_INT(data2, sqa);

	g_db.SQLExecuteEnd(sqa);

	//


	gLogger.FlushLog();


	return true;
}

bool TestDBProessBase::doSearchStudy()
{
DICOMData studyInfo;
pRTVSDicomInformation  iConstraints;
memset(&iConstraints,0,sizeof(iConstraints));
std::vector<pRTVSDicomInformation>  StudyOut;
 
 m_iGroupID = 3;
 m_iUserID = 2;

strcpy(iConstraints.m_patientName,m_PatientName.c_str());
//int status = g_db.GetUserSeries(iGroupID,iUserID,oOut,iConstraints);
	 
try {
 

	int status = g_db.GetUserStudies(m_iGroupID,m_iUserID,StudyOut,iConstraints);

	int nn = StudyOut.size();

	if(m_run_count%20 == 0){
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
	   
//	 doSearchStudy();

	 doSearchBitData();

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
	m_bakcupInterval = 10;//Sec
	m_testBigDBLen = 10;
	m_processorName = "TestDBProcessBigDb";

	m_runBackupFlag = false;
}
int	 TestDBProcessBigDb::PreProcess(void)
{
	return 0;
}
bool TestDBProcessBigDb::doCreateTestBigDB()
{
	if(m_creatBigDBFlag) return true;

	int MBytes = ((float)m_testBigDBLen * STR_MAX_LEN * STR_FILED_LEN)/(1024.0*1000) +0.5;

	if(this->m_runBackupFlag){
			gLogger.LogMessage("  ====== doCreateTestBigDB  -- runBackupFlag interval %d mSec, %d Sec\n",
				m_loopInterval,m_bakcupInterval );
		gLogger.FlushLog();

	}else{
		gLogger.LogMessage("  ====== doCreateTestBigDB %d about %d MB\n",m_testBigDBLen,MBytes);
		gLogger.FlushLog();
	}

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
	
	if(m_runBackupFlag){
		int interval_temp =  m_bakcupInterval*1000.0/m_loopInterval ;
		if( (m_run_count % interval_temp ) == 1 ){
			return runBackup();
		}
	}

	try {

 	doCreateTestBigDB();



	int index = (rand()  + rand() + rand() + rand() )/(4.0*RAND_MAX) * m_testBigDBLen + 1;

	char _str_buff[STR_MAX_LEN];

	sprintf(_str_buff,"[%d],data%d ::",index, m_run_count);
	for(int ii =strlen(_str_buff);ii<STR_MAX_LEN;ii++){
		_str_buff[ii] = 'a'+ii%20 ;
	}
	_str_buff[STR_MAX_LEN-2] = 0;

	int field_index = rand() %(STR_FILED_LEN);

	if(!doInsertBigDB( index)){

		gLogger.LogMessage("  ====== doInsertBigDB %d error\n",index);
 		gLogger.FlushLog();

		return false;
	}

	//
	 time_t timeReturn = time(NULL);
 
	 int data2 = timeReturn ;//&(0xffff) ;
	 int data3 = (timeReturn &(0xffff0000)) >> 16 ;
	 

   
	SQA sqa;
	sqa.FormatCommandText("Update dbo.testBigDb set intDat2=%d, intDat3=%d, bitDat%d = '%s' WHERE intDat1 = '%d' ", 
		data2,
		data3,
		field_index+1,
		_str_buff,
		index );		
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);

	int retcd = g_db.SQLExecuteBegin(sqa);
		if(retcd != kOK)  return  false;
		g_db.SQLExecuteEnd(sqa);


	if ((m_run_count % 3) == 0) {
		m_deleteIDList.push_back(index);
	}
	//
	if((m_run_count%40) ==0) {
		doCheckBigDB(index, field_index);
	}


	if (m_deleteIDList.size() > 5){
		doDeleteDB(m_deleteIDList[0]);
		m_deleteIDList.erase(m_deleteIDList.begin());

	}
	// rember the last date
	//
		m_lastDate = data2;
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
 		
		sqa.FormatCommandText("IF NOT EXISTS (SELECT BigDbID FROM dbo.testBigDb  WHERE intDat1=%d ) "
			"INSERT dbo.testBigDb (intDat1, intDat2,intDat3, intDat4,bigIn1,bigIn2,bitDat1,bitDat2,bitDat3) "
			"VALUES (%d, %d, %d, %d,'yyy ','ggg ',' ',' ',' ')", 
			index ,
			index, 2,3,4);
 		
		int retcd = g_db.SQLExecuteBegin(sqa);
		if(retcd != kOK)  return  false;
		g_db.SQLExecuteEnd(sqa);

		return true;

}

bool TestDBProcessBigDb::doDeleteDB(int index)
{
	SQA sqa;

	sqa.FormatCommandText("DELETE FROM dbo.testBigDb  WHERE intDat1=%d ", index);
		 
	int retcd = g_db.SQLExecuteBegin(sqa);
	if (retcd != kOK)  return  false;
	g_db.SQLExecuteEnd(sqa);

	return true;
}

bool TestDBProcessBigDb::dispLastData()
{
 
	SQA sqa;
	sqa.FormatCommandText("SELECT intDat2  FROM dbo.testBigDb  ORDER BY intDat2 DESC" );		
 

	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		gLogger.LogMessage("   dispLastData error \n");
		gLogger.FlushLog();
		return  false;
	}
	int size = sqa.GetRecordCount(); if(size < 1) return false;
	retcd = sqa.MoveFirst();
	if(retcd != kOK){
		gLogger.LogMessage("   dispLastData MoveFirst error \n");
		gLogger.FlushLog();
		return  false;
	}
	int data2;
 
	SQL_GET_INT(data2, sqa);
 
	g_db.SQLExecuteEnd(sqa);

	//
	int reg_data = TestDBProcessBigDb::ReadKey(DefLastDbDate );

	time_t timeReturn  = data2;//(long)(data3<<16) + data2;
	struct tm*    timePtr = localtime(&timeReturn);

	//
	time_t timeReg  = reg_data; 
	struct tm*    RegtimePtr = localtime(&timeReg);


	gLogger.LogMessage(" >>== dispLastData   %04d/%02d/%02d,%02d:%02d  --",
	 
		timePtr->tm_year + 1900,
		timePtr->tm_mon + 1,
		timePtr->tm_mday,
		//
		timePtr->tm_hour,
        timePtr->tm_min,
        timePtr->tm_sec 
		);
	

	if(reg_data != data2){
		gLogger.LogMessage("### Diff %04d/%02d/%02d,%02d:%02d  \n",
			RegtimePtr->tm_year + 1900,
			RegtimePtr->tm_mon + 1,
			RegtimePtr->tm_mday,
			//
			RegtimePtr->tm_hour,
			RegtimePtr->tm_min,
			RegtimePtr->tm_sec
	 
			);
	}else{
		gLogger.LogMessage("### Diff OK \n");
	}
 

	gLogger.FlushLog();

	return true;

}
bool TestDBProcessBigDb::doCheckBigDB(int id,int field_index)
{

	char _str_buff[STR_MAX_LEN];

	SQA sqa;
	sqa.FormatCommandText("SELECT intDat2, intDat3, bitDat%d FROM dbo.testBigDb  WHERE intDat1=%d ", 
		field_index+1,
		id );		


	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		gLogger.LogMessage("   doCheckBigDB error \n");
		gLogger.FlushLog();
		return  false;
	}
	int size = sqa.GetRecordCount(); if(size < 1) return false;
	retcd = sqa.MoveFirst();
	if(retcd != kOK){
		gLogger.LogMessage("   doCheckBigDB MoveFirst error \n");
		gLogger.FlushLog();
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
		id,field_index+1,_str_buff,
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

#include "RTVRegistry.h"
static const HKEY kDefaultKey       = HKEY_LOCAL_MACHINE;
static const char* kDefaultLocation = "Software\\PreXion\\AQServer";


void TestDBProcessBigDb::WriteKey(std::string keyName,int val)
{
	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);
	rtvr.SetRegistryKey (keyName.c_str(), val);
}
int TestDBProcessBigDb::ReadKey(std::string keyName)
{
	int ret_val = 0;


	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);

	 
	int status = rtvr.GetRegistryKey (keyName.c_str(), ret_val);

	return ret_val;
}
void TestDBProcessBigDb::onShutdown()
{
   TestDBProcessBigDb::WriteKey(DefLastDbDate, m_lastDate);
	return  ;
}

bool TestDBProcessBigDb::runBackup()
{
	 gLogger.LogMessage("  ====== TestDBProcessBigDb::runBackup %d  \n", m_run_count );
	 gLogger.FlushLog();

 
	char _str_buff_[1024];

	sprintf(_str_buff_,"\"C:\\Program Files\\AQNet\\bin\\testBackupCmd.bat\"");
	system(_str_buff_);

	return true;
}

////////////////////////////
TestDBProcessAddStudy::TestDBProcessAddStudy()
{
	m_studyInfo = new DICOMData;
}
 
TestDBProcessAddStudy::~TestDBProcessAddStudy()
{
	delete m_studyInfo;
}
TestDBProcessAddStudy *TestDBProcessAddStudy::createInstace()
{
	TestDBProcessAddStudy *instance = new TestDBProcessAddStudy;


	return instance;
}
	//	static TestDBProcessUpdate& theTestDBProcess();
 

int	 TestDBProcessAddStudy::PreProcess(void)
{
	memset(m_studyInfo, 0, sizeof(DICOMData));

//	char	m_studyInstanceUID[kVR_UI];
	strcpy(m_studyInfo->m_patientsName, "testData^Huge");
	strcpy(m_studyInfo->m_patientID, "101010");

	strcpy(m_studyInfo->m_patientsBirthDate, "20000101");
	strcpy(m_studyInfo->m_patientsSex, "M");
 
	strcpy(m_studyInfo->m_studyDate, "20170602");
	strcpy(m_studyInfo->m_studyTime, "12:00:00");
 
	strcpy(m_studyInfo->m_studyID, "111");
//	char	m_radiologistName[kVR_PN];
//	char	m_referringPhysiciansName[kVR_PN];
	strcpy(m_studyInfo->m_modalitiesInStudy, "CT");
	strcpy(m_studyInfo->m_studyDescription, "studyDescription");
//	long	m_numberOfStudyRelatedSeries;
//	long	m_numberOfStudyRelatedInstances;
	strcpy(m_studyInfo->m_characterSet, "ISO_IR 100\ISO 2022 IR 13\ISO 2022 IR 87");
//	long	m_accessTime;

//	char	m_seriesInstanceUID[kVR_UI];
	m_studyInfo->m_seriesNumber = 100;
	strcpy(m_studyInfo->m_seriesDescription, "seriesDescription");
	strcpy(m_studyInfo->m_modality, "CT");
//	char	m_bodyPartExamined[kVR_CS];
//	char	m_viewPosition[kVR_CS];
//	long	m_numberOfSeriesRelatedInstances;
	strcpy(m_studyInfo->m_stationName, "stationName");
//	int		m_offlineFlag;
//	int		m_IsQRData;
//	long	m_seriesModifyTime;
//	long	m_seriesHoldToDate;
	strcpy(m_studyInfo->m_seriesDate, "20170602");
	strcpy(m_studyInfo->m_seriesTime, "12:00:00");


	// instance level
//	char m_SOPInstanceUID[kVR_UI];
	strcpy(m_studyInfo->m_SOPClassUID, "1.2.840.10008.5.1.4.1.1.1.1");

//	int	m_transferSyntax;

	m_studyInfo->m_instanceNumber = 1;
	m_studyInfo->m_rows = 512;
	m_studyInfo->m_columns = 512;
	m_studyInfo->m_numberOfFrames = 1;

//	char m_imageTypeTokens[kVR_UI];
	m_studyInfo->m_bitsAllocated = 16;
	m_studyInfo->m_bitsStored = 12;
	m_studyInfo->m_highBit = 11;
	m_studyInfo->m_pixelRepresentation = 1;
//	m_studyInfo->m_photometricInterpretation; // (use enum)
//	short m_planarConfiguration;

	m_studyInfo->m_windowWidth = 2000.0f;
	m_studyInfo->m_windowCenter = 1500.0f;
//	int m_smallestPixelValue;
//	int m_largestPixelValue;
	m_studyInfo->m_samplesPerPixel = 1;

	m_studyInfo->m_pixelSpacing[0] = 0.1f;
	m_studyInfo->m_pixelSpacing[1] = 0.1f;
//	float m_aspectRatio;
	m_studyInfo->m_rescaleSlope = 0;
	m_studyInfo->m_rescaleIntercept = 1;
//	char m_patientOrientation[kVR_CS];
//	float m_slicePosition;
	m_studyInfo->m_sliceThickness = 0.1;
//	float m_imagePosition[3];
//	double m_imageOrientation[6];

	m_studyInfo->m_pixelOffset = 100;
	m_studyInfo->m_dataSize = 100;

	
	m_studyCount = 0;
	m_seriesCount = 0;;
	m_imageCount = 0;;

	beginNewStudy();
	beginNewSeries();
	return 0;
}

bool TestDBProcessAddStudy::doDBProc()
{
	switch (m_LangID){
	case LANG_ID_CHINESE:
		doAddStudyCn();
		break;
	case LANG_ID_RUSSIAN:
		doAddStudyRu();
		break;
	case LANG_ID_LATIN1:
		doAddStudyL1();
		break;
	case LANG_ID_JAPANESE:
		doAddStudyJp();
		break;
	default:
		doAddStudy();
		break;
	}
 	
//	doMyAddStudy();
	return true;
}
bool TestDBProcessAddStudy::doMyAddStudy()
{
	beginNewImage();
	const DICOMData& dData = *m_studyInfo;

	std::wstring patID = L"1234555888";
//	std::wstring patName = L"赵^之龙";
 	std::wstring patName = L"日本語^渓流";
//	std::wstring patName = L"test^Prexion";
	std::wstring studyUID = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(dData.m_studyInstanceUID);
	std::wstring studyBirthDay = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(dData.m_patientsBirthDate);
 
 
	{
		SQA sqa;
		int retcd;

		sqa.FormatCommandText(L"IF NOT EXISTS (SELECT StudyInstanceUID FROM StudyLevel WHERE StudyInstanceUID= '%s') "\
			L"INSERT StudyLevel (StudyInstanceUID,PatientsName, PatientID, PatientsBirthDate) VALUES ('%s', N'%s','%s','%s')", 
			studyUID.c_str(),
			studyUID.c_str(), patName.c_str(), patID.c_str(), studyBirthDay.c_str());

	//	sqa.m_options = 0;
		 
		sqa.setOptions(kDBLockExecute | kDBExecuteNoRecords);
		retcd = g_db.SQLExecuteBegin(sqa);
		// try one more time in case make SOPClassID clashed
		if (retcd != kOK)
		{
			g_db.SQLExecuteEnd(sqa, false);
			GetAqLogger()->LogMessage("INFO: -CPxDB::SaveDICOMData try insert instance again when first attempt fail\n");
			retcd = g_db.SQLExecuteBegin(sqa);
		}
		if (retcd != kOK)
			return retcd;

		g_db.SQLExecuteEnd(sqa);
		 
	}
	return true;
}
bool TestDBProcessAddStudy::doAddStudy()
{
	return doAddStudyRu();

	beginNewImage();
	strcpy(m_studyInfo->m_patientID, "1234555888");
 	strcpy(m_studyInfo->m_patientsName, "赵^子龙");
	strcpy(m_studyInfo->m_referringPhysiciansName, "武^则天");
//	wcscpy((wchar_t*)m_studyInfo->m_patientsNameWStr, L"赵^之龙");
//	wcscpy((wchar_t*)m_studyInfo->m_patientsNameWStr, L"日本語^渓流");
//	m_pImage->FillPatientInfo(studyInfo);
	strcpy(m_studyInfo->m_studyDescription, "检查描述");
	strcpy(m_studyInfo->m_seriesDescription, "序列描述");

	strcpy(m_studyInfo->m_characterSet, "ISO_IR 100\\GB18030");
	if (g_db.SaveDICOMData(*m_studyInfo) != kOK){
		gLogger.LogMessage("UpdateStudyInfo error \n");
		gLogger.FlushLog();
	}

	checkNext();

	return true;
}

void TestDBProcessAddStudy::beginNewStudy()
{
	m_seriesCount = 0;
	m_seriesNum = 1 + 3*rand()/(float)RAND_MAX;
	//
	DWORD curTime = ::GetTickCount();
	sprintf(m_studyInfo->m_studyInstanceUID, "1.2.392.200036.9163.31.114.%d%d", curTime,m_studyCount++);
	strcpy(m_studyInfo->m_patientsName, "testData^Huge");

	if ((m_studyCount % 10) == 0){
		gLogger.LogMessage("m_studyCount %d \n", m_studyCount);
		gLogger.FlushLog();
	}
}
void TestDBProcessAddStudy::beginNewSeries(void)
{
	m_imageCount = 0;
	m_imageNum = 1 + 5 * rand() / (float)RAND_MAX;

	sprintf(m_studyInfo->m_seriesInstanceUID, "%s.%d", m_studyInfo->m_studyInstanceUID, m_seriesCount++);
}
void TestDBProcessAddStudy::beginNewImage(void)
{
	sprintf(m_studyInfo->m_SOPInstanceUID, "%s.%d", m_studyInfo->m_seriesInstanceUID, m_imageCount++);
}

void TestDBProcessAddStudy::checkNext(void)
{
	if (m_imageCount >= m_imageNum){
		beginNewSeries();
	}
	if (m_seriesCount >= m_seriesNum){
		beginNewStudy();
	}
}

////////////

////////////////////////////
TestDBProcessSearchStudy::TestDBProcessSearchStudy()
{
	m_studyInfo = new DICOMData;
}

TestDBProcessSearchStudy::~TestDBProcessSearchStudy()
{
	delete m_studyInfo;
}
TestDBProcessSearchStudy *TestDBProcessSearchStudy::createInstace()
{
	TestDBProcessSearchStudy *instance = new TestDBProcessSearchStudy;


	return instance;
}
//	static TestDBProcessUpdate& theTestDBProcess();


int	 TestDBProcessSearchStudy::PreProcess(void)
{
	memset(m_studyInfo, 0, sizeof(DICOMData));
 
	return 0;
}

bool TestDBProcessSearchStudy::doDBProc()
{
	switch (m_LangID){
	case LANG_ID_CHINESE:
		doSearchStudyCn();
		break;
	case LANG_ID_RUSSIAN:
		doSearchStudyRu();
		break;
	case LANG_ID_LATIN1:
		doSearchStudyL1();
		break;
	case LANG_ID_JAPANESE:
		doSearchStudyJp();
		break;
		
	default:
		doSearchStudy();
		break;
	}
 
	return true;
}
bool TestDBProcessSearchStudy::doSearchStudy()
{
	int iGroupID = 1;
	int iUserID = 1;

	std::vector<pRTVSDicomInformation> oVal;
	pRTVSDicomInformation  iFilter;
	memset(&iFilter, 0, sizeof(pRTVSDicomInformation));
	//strcpy(iFilter.m_patientID, "123455*");
	   strcpy(iFilter.m_patientName, "*赵*");
	//strcpy(iFilter.m_patientName, "*test*");
	//strcpy(iFilter.m_patientName, "*日本語*");

	strcpy(iFilter.m_characterSet, "ISO_IR 100\\GB18030");
	int	retn = g_db.GetUserStudies(iGroupID, iUserID, oVal, iFilter);
	 
#if 0
	std::vector<DICOMPatient>  oVal;
	DICOMData iFilter;
	strcpy(iFilter.m_patientID, "123455*");
	int	retn = g_db.GetPatientList( oVal,&iFilter);
//	int	GetStudyList(std::vector<DICOMStudy>& oVal, DICOMData* iFilter, int TopN = 0, bool iSort = false);

#endif
	return true;
}
bool TestDBProcessSearchStudy::doMySearchStudy()
{
	 
	std::vector<DICOMPatient> oVal;
	DICOMData  iFilter;
 
	//strcpy(iConstraints.m_patientName, m_PatientName.c_str());
	//int status = g_db.GetUserSeries(iGroupID,iUserID,oOut,iConstraints);
	strcpy(iFilter.m_patientID, "123455*");
// 	strcpy(iFilter.m_patientsName, "*赵*");
 //	strcpy(iFilter.m_patientsName, "*test*");
	strcpy(iFilter.m_patientsName, "*日本語*");
	try {
		AqUString Ufilter  = L"";
		CPxDB::MakeUserStudiesFilterU(iFilter, Ufilter);
		
		 
		if (!Ufilter.IsEmpty())
		{
			Ufilter = L"WHERE " + Ufilter;
		}
		
		SQA sqa;

		sqa.FormatCommandText(L"SELECT DISTINCT PatientsName, PatientID, PatientsBirthDate,"\
			L"PatientsSex FROM StudyLevel %s", Ufilter);

//		sqa.m_options = kDBAsyncExecute;

		int retcd = g_db.SQLExecuteBegin(sqa); // do AsyncExecute
		oVal.clear(); if (retcd != kOK) return retcd;
		int size = sqa.GetRecordCount(); if (size < 1) return kNoResult;
		oVal.resize(size);

		DICOMPatient* pInfo;
		int index = 0;
		retcd = sqa.MoveFirst(); if (retcd != kOK)  return retcd;
		while (retcd == kOK && index < size)
		{
			pInfo = &(oVal[index++]);

			SQL_GET_STR(pInfo->m_patientsName, sqa);
			SQL_GET_STR(pInfo->m_patientID, sqa);
			SQL_GET_STR(pInfo->m_patientsBirthDate, sqa);
			SQL_GET_STR(pInfo->m_patientsSex, sqa);

			retcd = sqa.MoveNext();
		}
		g_db.SQLExecuteEnd(sqa);

	}
	catch (...)
	{
		int error = 1;
	}


	return true;
}

 