/***********************************************************************
 * CAutoDelStudyProc.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2011, All rights reserved.
 *
 *	PURPOSE:
 *		 
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "AutoDelStudyProc.h"
 

#include "Globals.h"
#include "rtvPoolAccess.h"
#include "PxDicomutil.h"
//#include "AssociationHandler.h"
#include "AppComConfiguration.h"

#include "PxNetDB.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "IDcmLibDefUID.h"

using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"

#endif

//#8 2012/03/16 K.KO
//ServiceList指定仕組みの追加
#include "PxDicomMessage.h"
#include "PxDicomimage.h"
 


#include "SendDicomProc.h"
#include "CStoreSCU.h"

//
// for Memory Dump
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>



const int kTwentyFourHours = 60 * 60 * 24;

 

extern TRLogger gDcmApiLogger; // #4 2012/02/21 K.Ko

 
 
//-----------------------------------------------------------------------------
//
CAutoDelStudyProc::CAutoDelStudyProc():
m_countNN(0)
//
,m_firstCheckFlag(true)
,m_checkedLicenseStatus(false)
{
	 
m_processorName = "CAutoDelStudyProc";

 
 //
 

 
}

CAutoDelStudyProc::~CAutoDelStudyProc()
{
 
}

//-----------------------------------------------------------------------------
//
#define M_ADD_SOP_CLASS(sop_cls) { \
	if(!TRDICOMUtil::addSOPClassUID((sop_cls),m_serviceListName)) \
	{\
		gLogger.LogMessage("ERROR: CAutoDelStudyProc::DICOM_Initialization TRDICOMUtil::addSOPClassUID [%s] \n", (sop_cls));\
		gLogger.FlushLog();\
		return false;\
	}\
}
bool CAutoDelStudyProc::DICOM_Initialization()
{
	int status;

	gLogger.LogMessage("DICOM_Initialization m_dicomLogLevel[%d]\n", gConfig.m_dicomLogLevel);
	gLogger.FlushLog();
 
 
 
//#8 2012/03/16 K.KO
//ServiceList指定仕組みの追加

//ここは不要
//	CStoreSCU::initCMoveServiceList();
 
	return true;
}

//-----------------------------------------------------------------------------
//
void CAutoDelStudyProc::DICOM_Release()
{
	TRDICOMUtil::ReleaseDICOM();
}

//-----------------------------------------------------------------------------
//
 
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


void CAutoDelStudyProc::RequestTermination(int iFlag)
{
	gLogger.LogMessage(kDebug,"INFO:  CAutoDelStudyProc::RequestTermination [%d]\n",iFlag);
	gLogger.FlushLog();

	if(iFlag>0){
		 
	}
	CIntervalProcessor::RequestTermination(iFlag);
}
int CAutoDelStudyProc::Process(void)
{
	gLogger.LogMessage(kDebug,"INFO:  CAutoDelStudyProc::Process\n");
	gLogger.FlushLog();
	
	AqCOMThreadInit comInitGuard; // for init DB COM
	

	if(!doInitDB()){
		return -1;
	}

	gLogger.LogMessage(kDebug,"INFO:  CAutoDelStudyProc::Process doInitDB OK \n");
	gLogger.FlushLog();
	
	/*
	* Open Association のリトライ回数
	*/
	 

	/*
	* Open Association のtimeout
	* default -1 
	*/
 
 

int runCount = 0;
	while(!TerminationRequested())
	{
	 
		gLogger.LogMessage(kDebug,"CProcQueue %d at priority[%d]\n",runCount,m_watchPriority);
		gLogger.FlushLog();

	 
		if(!checkLicense())
		{
			WaitForProcessToEnd(20000);//
			continue;

		}
 
		runCount++;


		gLogger.LogMessage(kDebug,">>>WaitForSingleObject -- begin \n");
		gLogger.FlushLog();

		DWORD dwWaitStatus;
		int timeout = 1000;
	 	timeout = gConfig.m_AutoDeleteStudyInterval*1000*60;
	//	if(m_watchPriority == CPxQueueEntry::PxQueuePriority_Low){
		if(1){
		//subThread for PxQueuePriority_Low
			dwWaitStatus = WaitForSingleObject(m_wakeupEvent, timeout);
			if (dwWaitStatus == WAIT_OBJECT_0)
			{
				//wakeup event
				::ResetEvent(m_wakeupEvent);
			}
			else
			{
				//timeout
				 
			}
			if(!TerminationRequested()){
				if(!doDeleteStudy()){
					gLogger.LogMessage(kErrorOnly,"doDeleteStudy error \n" );
					gLogger.FlushLog();
				}
			}
		} 

		gLogger.LogMessage(kDebug,"<<<WaitForSingleObject [%d]-- end  \n",dwWaitStatus);
		gLogger.FlushLog();
	}

	//#82 2014/09/29 K.Ko
	m_processStatus = iRTVThreadProcess::kProcessTerminated;

	gLogger.LogMessage(kDebug," CAutoDelStudyProc::Process -- end  \n" );
	gLogger.FlushLog();

	return 0;
}



//-----------------------------------------------------------------------------
//
int	CAutoDelStudyProc::PreProcess(void)
{ 


	return 0;

}

 

//-----------------------------------------------------------------------------
//



bool dicom_server_checkLicense()  ;
 
bool CAutoDelStudyProc::checkLicense( ) // K.Ko 2010/05/21
{
	
	bool do_flag = true;

	SYSTEMTIME stime_cur;
	;	
	FILETIME	ftime_cur;
 

	if(m_firstCheckFlag )
	{ 
		do_flag = true;
		m_firstCheckFlag = false;

		GetLocalTime(&stime_cur);
		SystemTimeToFileTime(&stime_cur,&ftime_cur);
	}else{// interval
#if 1
		do_flag = false;  //起動時一回チェックのみ
#else
		GetLocalTime(&stime_cur);
		SystemTimeToFileTime(&stime_cur,&ftime_cur);

		int time_diff = (*((__int64*)&ftime_cur)-*((__int64*)&m_lastCheckLicenseTime) )/10000000.0 ;
		int check_interval = 60; //1 Minute
		if(m_checkedLicenseStatus){
			//normal 
			check_interval = 60*10; //5 Minute
		}
		if(time_diff  < check_interval){
			do_flag = false;
		}else{
			do_flag = true;
			
		}
#endif
	}
	//
	if(!do_flag){
		return m_checkedLicenseStatus;
	}else{
		m_lastCheckLicenseTime = ftime_cur;//for next interval
		m_checkedLicenseStatus = dicom_server_checkLicense();
	 
	}

	return m_checkedLicenseStatus;
}

 


bool CAutoDelStudyProc::doInitDB()
{
 //	AqCOMThreadInit comInitGuard; // for CoInitializeEx

	CPxDcmDB g_db;
 
	 
		{
 
			/*
			* for access PxDcmDB
			*/
 
			gLogger.LogMessage(kDebug," DB sever name %s \n",g_db.GetServerName());
			gLogger.FlushLog();
			
 
 
			if(!g_db.InitDatabaseInfo()){
				gLogger.LogMessage("ERROR:[C%08d] InitDatabaseInfo error\n",DicomJobProcError_DBInitError);
				gLogger.FlushLog();
			}
 
 

		} 
   
	
	{
		AqString strSQL;
		strSQL.Format("SELECT MajorVersion FROM dbinfo" );

		int oVal;
		g_db.SQLGetInt(strSQL, oVal);
	}
 
	return true;
}
void CAutoDelStudyProc::setupQueueLog()
{
	 
	
}
bool CAutoDelStudyProc::doDeleteStudy()
{
	bool ret_b = true;
	gLogger.LogMessage(kDebug,"doDeleteStudy  \n" );
	gLogger.FlushLog();
			
	std::vector<std::string> studyList_temp;
	std::vector<std::string> studyList;

	if(!searchStudy(studyList_temp)){
		ret_b = false;
	}else{
		int org_study_list_size = studyList_temp.size();
		if(org_study_list_size>0){
			std::vector<std::string> resultQueue_studyList;
			getResultQueueList(resultQueue_studyList,true/*failedOnly*/);
			//
			std::vector<std::string> sendQueue_studyList;
			getSendQueueList(sendQueue_studyList);
			for(int i=0;i<org_study_list_size;i++){
				std::string study_uid = studyList_temp[i];
				if( (std::find(resultQueue_studyList.begin(), resultQueue_studyList.end(), study_uid)!=resultQueue_studyList.end()) ||
					(std::find(sendQueue_studyList.begin(), sendQueue_studyList.end(), study_uid)!=sendQueue_studyList.end())
					){
						//再送信可能性のあるデータは削除しない。
						;//skip
				}else{
					studyList.push_back(study_uid);
				}
			}
		}
 
	};
	//終了時を考慮
	if(TerminationRequested()){
		return true;
	}
	int study_list_size = studyList.size();
	
	if(study_list_size>0){
		int delete_nn = study_list_size;
		if(delete_nn>gConfig.m_DeleteStudyNumberOneTime){
			delete_nn = gConfig.m_DeleteStudyNumberOneTime;
		}
		gLogger.LogMessage(kInfo,"delete Study [%d]/[%d]\n",delete_nn,study_list_size);
		gLogger.FlushLog();
		for(int i=0;i<delete_nn;i++){
			if(!deleteStudy(studyList[i])){
				gLogger.LogMessage(kErrorOnly,"delete Study [%s] failed\n",studyList[i].c_str());
				gLogger.FlushLog();
			}
			//終了時を考慮
			if(TerminationRequested()){
				break;
			}
		}
	}

	return ret_b;
}
 
#define TimeStrFormat "%04d/%02d/%02d %02d:%02d:%02d"
#define DateStrFormat "%04d/%02d/%02d"
static std::string getTimeStringFromTM(const struct tm &tm_temp)
{
	char _str_buf[64];
 
	sprintf(_str_buf, 
			TimeStrFormat,
			tm_temp.tm_year + 1900,
			tm_temp.tm_mon + 1,
			tm_temp.tm_mday,
			tm_temp.tm_hour,
			tm_temp.tm_min,
			tm_temp.tm_sec
			);
	return _str_buf;
}
static std::string getDateStringFromTM(const struct tm &tm_temp)
{
	char _str_buf[64];
 
	sprintf(_str_buf, 
			DateStrFormat,
			tm_temp.tm_year + 1900,
			tm_temp.tm_mon + 1,
			tm_temp.tm_mday
			);
	return _str_buf;
}
bool CAutoDelStudyProc::searchStudy(std::vector<std::string> &studyList)
{
	CPxDB PxDb;
	AqString strSQL;
	 
	time_t cur_t = time(0);

	time_t t1 = cur_t-60*60*24*gConfig.m_AutoDeleteKeepDays;

	struct tm  *tt1 = localtime(&t1);
	int year = tt1->tm_year + 1900;
	int month = tt1->tm_mon + 1;

	std::string time = getDateStringFromTM(*tt1);
	strSQL.Format("SELECT StudyInstanceUID FROM  StudyLevel WHERE AccessTime < '%s' ORDER BY AccessTime ASC",time.c_str());

	char _study_uid_buff[128];
	SQA sqa;
	sqa.SetCommandText(strSQL);
	int retcd = PxDb.SQLExecuteBegin(sqa);
	if(retcd != kOK){
	}else{
		int size = sqa.GetRecordCount(); 
		if(size < 1) {
		}else{
	//		studyList.resize(size);
			int index = 0;
			retcd = sqa.MoveFirst(); 
			 
			while( retcd == kOK && index < size ){
				_study_uid_buff[0] = 0;
				SQL_GET_STR(_study_uid_buff,sqa);
				if(strlen(_study_uid_buff)>0){
					studyList.push_back(_study_uid_buff);
				}
				retcd = sqa.MoveNext();
			}
		}

		 
		PxDb.SQLExecuteEnd(sqa);
	}

	return true;
}

#include "AppComUtil.h"
bool CAutoDelStudyProc::deleteStudy(const std::string &studyUID)
{
try{
	CPxDcmDB pxDb;

	//
	std::vector<std::string>  oSeries;

	if(kOK!=pxDb.GetSeries(studyUID.c_str(), oSeries)){
		return false;
	};

	if( kOK!=pxDb.DeleteStudy( studyUID.c_str()))
	{
		return false;
	}
	AppComUtil::PxRemoveAllDiskFiles(oSeries, studyUID.c_str(), 0/*iKeepOrphaned*/);
}catch(...){
	return false;
}
return true;
}
#include "PxQueue.h"
//-----------------------------------------------------------------------------
bool CAutoDelStudyProc::getResultQueueList(std::vector<std::string> &studyList,bool failedOnly)
{
	studyList.clear();
	CPxResultQueue ResultQueueProc;
	ResultQueueProc.initRes();
	if(failedOnly){
		ResultQueueProc.readQueueExt(CPxQueueEntry::PxQueuePriority_All,
								CPxQueueEntry::PxQueueStatus_Failed);
	}else{
		ResultQueueProc.readQueueExt(CPxQueueEntry::PxQueuePriority_All,
								CPxQueueEntry::PxQueueStatus_Finished,
								CPxQueueEntry::PxQueueStatus_Failed);
	}
	std::vector<CPxQueueEntry> &entry_list = ResultQueueProc.getEntryList();
	int size = entry_list.size();
	if(size>0){
		studyList.resize(size);
		for(int i=0;i<size;i++){
			studyList[i] = entry_list[i].m_StudyInstanceUID;
		}
	}

	return true;
}
 
bool CAutoDelStudyProc::getSendQueueList(std::vector<std::string> &studyList)
{
	char _str_num_buff[64];
	studyList.clear();

	CPxWorkQueue QueueProc;
	QueueProc.initRes();
//	QueueProc.readQueueExt(CPxQueueEntry::PxQueuePriority_All);
	QueueProc.readQueueAll();

	std::vector<CPxQueueEntry> &entry_list = QueueProc.getEntryList();
	int size = entry_list.size();
	if(size>0){
		studyList.resize(size);
		for(int i=0;i<size;i++){
			studyList[i] = entry_list[i].m_StudyInstanceUID;
		}
	}

	return true;
}
//
