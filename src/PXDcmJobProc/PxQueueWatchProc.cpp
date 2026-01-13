/***********************************************************************
 * PxQueueWatchProc.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2011, All rights reserved.
 *
 *	PURPOSE:
 *		Listens for DICOM C-STORE requests on a port in its own thread.
 *		Each new association request kicks off a new AssociationHandler
 *		thread.
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxQueueWatchProc.h"
#include "outputJpegProc.h"

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

///////////////////
class CMyPxQueueProc : public CPxWorkQueue
{
public:
	CMyPxQueueProc(void){
		m_owner = 0;
	};
	virtual ~CMyPxQueueProc(void)
	{
		int dbg_p = 0;
	};

	PxQueueWatchProc *m_owner;
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
			
			bool ret_b = false;
			try{
				ret_b = m_owner->doQueueWork( entry);
			}catch(...)
			{
				ret_b = false;
			}
			return ret_b;
		}
		return true;
	}
};
		//////////////
//-----------------------------------------------------------------------------
//
#if 0
PxQueueWatchProc& PxQueueWatchProc::thePxQueueWatchProc()
{
	static PxQueueWatchProc p; // the signle PxQueueWatchProc object
	return p;

}
#endif

//-----------------------------------------------------------------------------
//
PxQueueWatchProc::PxQueueWatchProc(bool isOutJpegGateWay):
m_countNN(0)
//
,m_firstCheckFlag(true)
,m_checkedLicenseStatus(false)
{
	m_watchPriority = CPxQueueEntry::PxQueuePriority_All;
m_processorName = "PxQueueWatchProc";

 
 //
 

 m_CSendDicomProc_hdr = new CSendDicomProc;
 m_QueueProc = new CMyPxQueueProc;
 ((CMyPxQueueProc*)m_QueueProc)->m_owner = this;

 if(isOutJpegGateWay){//#48
	 m_outputJPEGProc_hdr = new COutputJpegProc;
 }else{
	 m_outputJPEGProc_hdr = 0;
 }
}

PxQueueWatchProc::~PxQueueWatchProc()
{
	delete m_QueueProc;
	delete m_CSendDicomProc_hdr;

	if(m_outputJPEGProc_hdr) {//#48
		delete m_outputJPEGProc_hdr;
		m_outputJPEGProc_hdr = 0;
	}
}

//-----------------------------------------------------------------------------
//
#define M_ADD_SOP_CLASS(sop_cls) { \
	if(!TRDICOMUtil::addSOPClassUID((sop_cls),m_serviceListName)) \
	{\
		gLogger.LogMessage("ERROR: PxQueueWatchProc::DICOM_Initialization TRDICOMUtil::addSOPClassUID [%s] \n", (sop_cls));\
		gLogger.FlushLog();\
		return false;\
	}\
}
bool PxQueueWatchProc::DICOM_Initialization()
{
	int status;

	gLogger.LogMessage("DICOM_Initialization m_dicomLogLevel[%d]\n", gConfig.m_dicomLogLevel);
	gLogger.FlushLog();
 

	if(!m_CSendDicomProc_hdr->init()){
		gLogger.LogMessage(" PxQueueWatchProc::init() failed\n");
		gLogger.FlushLog();
		return false;
	};

	if(m_outputJPEGProc_hdr){
		if(!m_outputJPEGProc_hdr->init()){
			gLogger.LogMessage(" PxQueueWatchProc::init() -- m_outputJPEGProc_hdr failed\n");
			gLogger.FlushLog();
			return false;
		};
	}
 
//#8 2012/03/16 K.KO
//ServiceList指定仕組みの追加
	 CStoreSCU::initCMoveServiceList();
 
	return true;
}

//-----------------------------------------------------------------------------
//
void PxQueueWatchProc::DICOM_Release()
{
	TRDICOMUtil::ReleaseDICOM();
}

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


void PxQueueWatchProc::RequestTermination(int iFlag)
{
	if(iFlag>0){
		if(m_QueueProc){
			m_QueueProc->setBreakFlag(true);
		};
		////#82 2014/09/29 K.Ko
		if(m_outputJPEGProc_hdr){
			m_outputJPEGProc_hdr->cancel();
		}
		if(m_CSendDicomProc_hdr){
			m_CSendDicomProc_hdr->cancel();
		}
		
	}
	CIntervalProcessor::RequestTermination(iFlag);
}
int PxQueueWatchProc::Process(void)
{
	gLogger.LogMessage(kDebug,"INFO:  PxQueueWatchProc::Process\n");
	gLogger.FlushLog();
	
	AqCOMThreadInit comInitGuard; // for init DB COM
	

	if(!doInitDB()){
		return -1;
	}

	gLogger.LogMessage(kDebug,"INFO:  PxQueueWatchProc::Process doInitDB OK \n");
	gLogger.FlushLog();
	
	/*
	* Open Association のリトライ回数
	*/
	if(m_watchPriority == CPxQueueEntry::PxQueuePriority_Low){
		//subThread for PxQueuePriority_Low
		MC_Set_Int_Config_Value(RETRY_NUMBER_OPEN_ASSOC,(int)2);
		//for thread Sub
		m_QueueProc->setRetryMax(gConfig.m_retrySendCount);
		m_QueueProc->setRetryInterval(gConfig.m_retrySendInterval);
		m_QueueProc->setRetryIntervalMax(gConfig.m_retrySendIntervalMax);// #80 2014/08/14 K.Ko
	}else{
		

		MC_Set_Int_Config_Value(RETRY_NUMBER_OPEN_ASSOC,(int)1);
	} 

	if(gConfig.m_useSubThreadForLowPriority ==0){
		// one thread only
		m_QueueProc->setRetryMax(gConfig.m_retrySendCount);
		m_QueueProc->setRetryInterval(gConfig.m_retrySendInterval);
		m_QueueProc->setRetryIntervalMax(gConfig.m_retrySendIntervalMax);// #80 2014/08/14 K.Ko
	}

	/*
	* Open Association のtimeout
	* default -1 
	*/
	MC_Set_Int_Config_Value(OPEN_ASSOC_TIMEOUT,(int)3); //3Sec

	int rcd = 0;



//	QueueProc.initDB();;//<- 重要 for access PxQueueDB
	m_QueueProc->initRes();

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

	 	if(!m_QueueProc->watchQueue(m_watchPriority)){
//		if(!m_QueueProc->watchQueue(CPxQueueEntry::PxQueuePriority_All)){
			gLogger.LogMessage(kErrorOnly,"QueueProc.watchQueue error\n");
			gLogger.FlushLog();
			::Sleep(500);
		}
		 
 
//		if(m_QueueProc->getWatchedQueueSize()>0){
		if(m_QueueProc->getDoQueueWorkCount()>0){
			continue;
		}
 

		if(runCount%10 == 0){
			m_QueueProc->recycleQueue(1);
		}
 
		runCount++;


		gLogger.LogMessage(kDebug,">>>WaitForSingleObject -- begin \n");
		gLogger.FlushLog();

		DWORD dwWaitStatus;
		int timeout = 5000;
		if(m_watchPriority == CPxQueueEntry::PxQueuePriority_Low){
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
		}else{

			HANDLE sendQueu_h = m_QueueProc->getSendQueuEvent();

			
			HANDLE waitedObjects[2];
			waitedObjects[0] = m_wakeupEvent;   //locale event
			waitedObjects[1] = sendQueu_h;		//Global Queu event 

			
			dwWaitStatus = WaitForMultipleObjects(2, waitedObjects, FALSE, timeout);
			if (dwWaitStatus == WAIT_OBJECT_0)
			{
				//wakeup event
				::ResetEvent(m_wakeupEvent);
			}
			else if (dwWaitStatus == WAIT_OBJECT_0 + 1)
			{
				// queue event
				::ResetEvent(sendQueu_h);
			}
			else
			{
				//timeout
				 
			}
		} 

		gLogger.LogMessage(kDebug,"<<<WaitForSingleObject [%d]-- end  \n",dwWaitStatus);
		gLogger.FlushLog();
	}
	//#82 2014/09/29 K.Ko
	m_processStatus = iRTVThreadProcess::kProcessTerminated;

	gLogger.LogMessage(kDebug," PxQueueWatchProc::Process -- end [%s] \n",
							(m_watchPriority == CPxQueueEntry::PxQueuePriority_Low) ? "Sub" : "Main");
	gLogger.FlushLog();

	return rcd;
}



//-----------------------------------------------------------------------------
//
int	PxQueueWatchProc::PreProcess(void)
{ 
	gLogger.LogMessage("PxQueueWatchProc::PreProcess [%s] \n",
		(m_watchPriority == CPxQueueEntry::PxQueuePriority_Low) ? "Sub" : "Main");
	gLogger.FlushLog();

 
	return 0;

}

 

//-----------------------------------------------------------------------------
//



bool dicom_server_checkLicense()  ;
 
bool PxQueueWatchProc::checkLicense( ) // K.Ko 2010/05/21
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

 

bool PxQueueWatchProc::watchFilter(const CPxQueueEntry &entry)
{
#if 0
	if(entry.m_DestinationAE == m_watchOnAE){
		return true;
	}else{
		return false;
	}
#else
	return true;
#endif

}



bool PxQueueWatchProc::doQueueWork(const CPxQueueEntry &entry)
{
	CSendDicomProcIf *pSendDicomProcOp = 0;

	if(entry.m_cmdID == CPxQueueEntry::PxQueueCmd_JPEG){//#48
	//	return doQueueOutputJPeg( entry); 
		gLogger.LogMessage("doQueueOutputJPeg [(%s)] %d, %s, %s, %s , %s \n",
		m_watchPriority!=CPxQueueEntry::PxQueuePriority_Low ? "Main":"Sub",
		m_countNN,
		(entry.m_StudyInstanceUID.size()>0) ? entry.m_StudyInstanceUID.c_str() : " ",
		(entry.m_SeriesInstanceUID.size()>0) ? entry.m_SeriesInstanceUID.c_str() : " ",
		(entry.m_SOPInstanceUID.size()>0) ? entry.m_SOPInstanceUID.c_str() : " ",
	//#142_NoPatientName_NoComment
	//	(entry.m_extInfo.m_Comment.size()>0) ? entry.m_extInfo.m_Comment.c_str() : " "
		" "
		);

		pSendDicomProcOp = m_outputJPEGProc_hdr;
	}else{

//	gLogger.LogMessage(kDebug,"doQueueWork [(%s)] %d, %s, %s, %s , %s \n",
		gLogger.LogMessage("doQueueWork [(%s)] %d, %s, %s, %s , %s \n",
			m_watchPriority!=CPxQueueEntry::PxQueuePriority_Low ? "Main":"Sub",
			m_countNN,
			(entry.m_StudyInstanceUID.size()>0) ? entry.m_StudyInstanceUID.c_str() : " ",
			(entry.m_SeriesInstanceUID.size()>0) ? entry.m_SeriesInstanceUID.c_str() : " ",
			(entry.m_SOPInstanceUID.size()>0) ? entry.m_SOPInstanceUID.c_str() : " ",
		//#142_NoPatientName_NoComment
		//	(entry.m_extInfo.m_Comment.size()>0) ? entry.m_extInfo.m_Comment.c_str() : " "
			" "
			);
		pSendDicomProcOp = m_CSendDicomProc_hdr;
	}
	gLogger.FlushLog();

	if(!pSendDicomProcOp){//#48
		gLogger.LogMessage("*** ERROR: pSendDicomProcOp == NULL ***\n");
		gLogger.FlushLog();
		return false;
	}

 	m_countNN++;

	
	bool ret_b = false; //fot debug

#if 1
	switch(entry.m_SendLevel){
		case CPxQueueEntry::PxQueueLevel_Unknown:
		break;
		case CPxQueueEntry::PxQueueLevel_Study:
 
	//		ret_b = m_CSendDicomProc_hdr->sendStudy(&entry);
			ret_b = pSendDicomProcOp->sendStudy(&entry);
		break;
		case CPxQueueEntry::PxQueueLevel_Series:
	 
	//		ret_b = m_CSendDicomProc_hdr->sendSeries(&entry);
			ret_b = pSendDicomProcOp->sendSeries(&entry);
		break;
		case CPxQueueEntry::PxQueueLevel_Image:
 
	//		ret_b = m_CSendDicomProc_hdr->sendImage(&entry);
			ret_b = pSendDicomProcOp->sendImage(&entry);
		break;
		case CPxQueueEntry::PxQueuelevel_EntryFile:
		{
			std::string seriesFolder;
			std::vector<std::string> ImageFileList;
			if(!getImageFileListFromEntryFile(entry, seriesFolder ,ImageFileList)){
				ret_b =false;
			}else{
	 
	//			ret_b = m_CSendDicomProc_hdr->sendEntryFile(&entry,seriesFolder,ImageFileList);
				ret_b = pSendDicomProcOp->sendEntryFile(&entry,seriesFolder,ImageFileList);
			}
		
		}
		break;
	}
#else
	// just test the queue processing
	ret_b = true;
#endif
		 
	gLogger.LogMessage(kDebug,"doQueueWork [(%s)] %d, -- end \n",
		m_watchPriority!=CPxQueueEntry::PxQueuePriority_Low ? "Main":"Sub",
		ret_b);
	gLogger.FlushLog();


	return ret_b;
}
bool PxQueueWatchProc::getImageFileListFromEntryFile(const CPxQueueEntry &entry, std::string &seriesFolder ,std::vector<std::string> &ImageFileList)
{
	CPxQueueBinFile queue_bin_file;

	std::string entry_file_name = CPxQueueProc::getQueueEntryFileName(entry);//m_SQLiteDB_Dir + "\\entry\\" + entry.m_JobID;
	if(!queue_bin_file.readQueueBinFileEx(entry_file_name,seriesFolder,ImageFileList)){
		return false;
	}
	
	return true;
}

bool PxQueueWatchProc::doInitDB()
{
 //	AqCOMThreadInit comInitGuard; // for CoInitializeEx

	CPxDcmDB g_db;
 
	 
		{
 
			/*
			* for access PxDcmDB
			*/
#if 0  //InitDBServerName 設定済み
			TCHAR  MyComputerName[256];
			unsigned long nSize = sizeof(MyComputerName)-1 ;
			GetComputerName(MyComputerName, &nSize);
			std::string dbServerNameTemp = MyComputerName;
 
			dbServerNameTemp = dbServerNameTemp + DBCore::GetSQLServerDBNameExt();
 

			gLogger.LogMessage(" DB sever name %s \n",dbServerNameTemp.c_str());
			gLogger.FlushLog();


			CPxDcmDB::InitDBServerName(dbServerNameTemp.c_str());
#else
			gLogger.LogMessage(kDebug," DB sever name %s \n",g_db.GetServerName());
			gLogger.FlushLog();
			

#endif

#if 0
			{
				AqUString strSQL;
		 
				strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;Integrated Security=SSPI;Initial Catalog=PxDcmDB;APPLICATION NAME=%S", 
					dbServerNameTemp.c_str(), GetCurrentProcessName());
			 
		 


				g_db.SetDBConectionInfo(strSQL);

				if(g_db.TestConnectionInfo()  != kOK )
				{
					
					gLogger.LogMessage("** DB::InitDatabaseInfo failed   \n" );
					gLogger.FlushLog();
				 
					return false;
				}
			}
#else
			if(!g_db.InitDatabaseInfo()){
				gLogger.LogMessage("ERROR:[C%08d] InitDatabaseInfo error\n",DicomJobProcError_DBInitError);
				gLogger.FlushLog();
			}
#endif
 
#if 0
//DICOM SERVER 起動しないと、Level = -1 となっているため、エラーになる。
//チェック不要
			{
 
				//locale AE
				std::vector<ApplicationEntity>  oAEList;
				g_db.GetEnabledLocalAE(oAEList);
				if(oAEList.size()<1){
					gLogger.LogMessage("ERROR:[C%08d] GetLocalAE failed in PxQueueWatchProc::doInitDB\n",DicomJobProcError_InvalidDicomAE);
					gLogger.FlushLog();
					return false;
				}else{
					 m_CSendDicomProc_hdr->setLocaleAE(oAEList[0].m_AETitle);
 
				}

			}
#endif


		} 
   
	
	{
		AqString strSQL;
		strSQL.Format("SELECT MajorVersion FROM dbinfo" );

		int oVal;
		g_db.SQLGetInt(strSQL, oVal);
	}
 
	return true;
}
void PxQueueWatchProc::setupQueueLog()
{
	CPxQueueProc::setLogger(&gLogger);
	
}
 
//-----------------------------------------------------------------------------
//
