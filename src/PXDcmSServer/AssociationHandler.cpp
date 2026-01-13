/***********************************************************************
 * AssociationHandler.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Accepts images over an open DICOM association.  If the incoming
 *		images belong to a new series, a SeriesDirMonitor thread is 
 *		kicked off.
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "AssociationHandler.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif


#include "PxDicomutil.h"

#include "AppComConfiguration.h"
#include "Globals.h"
#include "diskspacemanager.h"
#include "Listener.h"
#include "RTVDiCOMCEcho.h"
#include "CStore.h"
#include "CMove.h"
#include "CFind.h"
#include "AqStorageCommitment.h"

 
//#9 2012/03/19 K.Ko for New Hasp
#include "PXLicenseManagerIf.h"
#include "ThreadedLicenseChecker.h"
 

#include "TICache.h"
 
 
extern int gHASPFeatureID;
extern const char* kStorageCommitmentServiceName;

//#define UseMemoryState
//-----------------------------------------------------------------------------
//
AssociationHandler::AssociationHandler(DiCOMConnectionInfo& connectInfo) : RTVAssociationHandler(connectInfo)
{ 
	m_processorName = "Association Handler";

	m_ThreadPriority = iRTVThread::LOWEST;//3;// 既存
}
AssociationHandler::~AssociationHandler() 
{
 //	GetAqLogger()->LogMessage(" :::: delete AssociationHandler \n");
 //	GetAqLogger()->FlushLog();
};
//-----------------------------------------------------------------------------
//

int AssociationHandler::SendLicenseWillExpireEmail(const char* iSubj, const char* iMsg)
{
	static int lastDaySent;

#if 0
	time_t t = time(0);
	struct tm tm = *localtime(&t);
	int day = tm.tm_mday;

	if (day == lastDaySent)
		return 0;

	lastDaySent = day;

//	if (!gEmailer.IsConfigured())
//		return -1;

	std::string subj = "";
	std::string msg = "";

	std::string errString = gEmailer.SendMessage(iSubj, iMsg);
	if (errString.size() > 0)
	{
		GetAqLogger()->LogMessage("ERROR: failed to send email: (%s)\n", errString.c_str());
		GetAqLogger()->LogMessage("       subj = %s", iSubj);
		GetAqLogger()->LogMessage("       msg = %s", iMsg);
		GetAqLogger()->LogMessage("       to = %s", gEmailer.GetTo().c_str());
		GetAqLogger()->FlushLog();
		return -1;
	}

#endif
	return 0;
}

//-----------------------------------------------------------------------------
//
int AssociationHandler::CheckLicenseStatus(void)
{
	int daysToExpire = 0;
 
//#9 2012/03/19 K.Ko for New Hasp
	return PXLicenseManager::GetCachedStatus(gHASPFeatureID, daysToExpire);
 
}

//-----------------------------------------------------------------------------
//
int AssociationHandler::setupThreadPriority()
{ 
	return GetTheTread()->SetPriority(m_ThreadPriority);
//	HANDLE cur_thread = ::GetCurrentThread();
//	return ::SetThreadPriority(cur_thread,m_ThreadPriority);
  
};/// 2010/03/15 K.Ko

extern TRLogger gDcmApiLogger; // #4 2012/02/21 K.Ko
int AssociationHandler::PreProcess(void)
{
	m_maxThreadsPerAssociation = gConfig.m_maxThreadsPerAssociation;
	m_inactivityTimeout = gConfig.m_inactivityTimeout;

	std::string calling_AE = m_connectInfo.RemoteApplicationTitle;

	std::string HighestPrioAE = "consoleAE";
	if(calling_AE == HighestPrioAE) { /// 2010/03/15 K.Ko
		m_ThreadPriority = iRTVThread::REALTIME;//THREAD_PRIORITY_TIME_CRITICAL;
		//ここはスレッド起動前
//		::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
//		::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
		GetAqLogger()->LogMessage(kInfo,"INFO: Association set THREAD_PRIORITY to REALTIME  for AE: %s \n", calling_AE.c_str());
 
	}
	m_processStatus = kToBeStarted;

	//	Check log to see if it's time to rotate
	struct _stat statBuf;
	long fileSize = -1;
	if (_stat(gConfig.m_logFilename, &statBuf))
	{
		//	No point logging - we can't stat the log file.
		return 0;
	} 
	else
	{
		fileSize = statBuf.st_size;
	}	
	
	if (fileSize > (gConfig.m_logMaxSize * 1024) || fileSize < 0)
	{
		gLogger.RotateLog();
	}    

	{
		// #4 2012/02/21 K.Ko
		if (_stat(gConfig.m_mergeLog, &statBuf))
		{
			//	No point logging - we can't stat the log file.
			return 0;
		} 
		else
		{
			fileSize = statBuf.st_size;
		}	
		
		if (fileSize > (gConfig.m_logMaxSize * 1024) || fileSize < 0)
		{
			gDcmApiLogger.RotateLog();
		}    


	}
	return 0;
}


//-----------------------------------------------------------------------------
//
bool AssociationHandler::ProcessMessages()
{
	std::string rootDir;
    std::string cacheDir;

try{ // #11  2012/03/23 K.Ko
	// try_catch for ProcessMessages all

	//
	//	We will always print this - that way, when errors reference AssociationID, the
	//		Log record can be traced to a particular host.
	if(gConfig.m_traceDicomLevel >=1){ //#693
		GetAqLogger()->LogMessage("INFO:[C%08d] Association %d: Accepted from %s on [%s,%s]\n",
				DicomServInfor_DicomAssociationInfo,
				m_connectInfo.AssociationID, m_connectInfo.RemoteApplicationTitle, 
				m_connectInfo.RemoteHostName, m_connectInfo.RemoteIPAddress);
	}else{
		GetAqLogger()->LogMessage(kInfo,"INFO:[C%08d] Association %d: Accepted from %s on [%s,%s]\n",
				DicomServInfor_DicomAssociationInfo,
				m_connectInfo.AssociationID, m_connectInfo.RemoteApplicationTitle, 
				m_connectInfo.RemoteHostName, m_connectInfo.RemoteIPAddress);
	}

	FlushLog();

	// prepare COM for ado ADO database call
	AqCOMThreadInit comInitGuard;

	// break big m_inactivityTimeout (3600s) to small piecses
	//int tmp_count, s_timeout = 5;
	int tmp_count, s_timeout = 1;
	int n_timeout = m_inactivityTimeout/s_timeout;
	int status = MC_TIMEOUT; tmp_count = 0;
	while(tmp_count < n_timeout && status == MC_TIMEOUT && !TerminationRequested())
	{
		m_state = kMC_Read_Message;
		status = MC_Read_Message(m_connectInfo.AssociationID, s_timeout, &m_messageID, &m_serviceName, &m_command);
		tmp_count++;
	}
#ifdef UseMemoryState
	IDcmLibApi::CheckMemory();
#endif

    // Get the Root Dir.
    // 
    // We need to get the the Root Dir only if the association is C_STORE_RQ and
    // we need to do this only once per association.
	const int reserveSpace = kSeriesReserveSpace; // 1.6 GB

    if (m_command == C_STORE_RQ)
	{
		// 
		// Need to check status and close the associaltion if this call fails
		// If this call fails it means we are out of resources.
		//
		// There is also a problem if a single series is sent over
		// multiple C_STORE associations. We need to add some 
		// logic to make sure that once a series is stored it
		// if it come again on the next association the cache is built in the
		// right place. We definitely do not want the cache to be split up
		// over 2 disks. This is remote possibility but .....
		int pathStatus = RTVDiskSpaceManager::GetAvailableMedia(reserveSpace, rootDir, cacheDir);
		if (pathStatus != RTVDiskSpaceManager::kOK)
		{
			GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): AssociationHandler::Process() - GetAvailableMedia failed for %dMB.\n",
								DicomServError_DiskSpaceError,m_connectInfo.AssociationID,reserveSpace);
			GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): Reject connection for not enough resource.\n", 
								DicomServError_DiskSpaceError,m_connectInfo.AssociationID);
			SendResponseMessage(C_STORE_FAILURE_REFUSED_NO_RESOURCES, C_STORE_RSP);
			MC_Close_Association(&m_connectInfo.AssociationID);
			m_connectInfo.AssociationID = -1;
			return false;
		}
	}

#ifdef _TIMING_ASSOC
		LARGE_INTEGER startTime0;
		LARGE_INTEGER endTime;
#endif
	BOOL threadWrite;
	while(!TerminationRequested())
	{
		//
		//	Read failed
		//
		if (status != MC_NORMAL_COMPLETION)
		{
			if(status == MC_ASSOCIATION_CLOSED){
				if(gConfig.m_traceDicomLevel >=1){ //#693
					GetAqLogger()->LogMessage("INFO:[C%08d] Association %d: closed\n", 
						DicomServInfor_DicomAssociationInfo, m_connectInfo.AssociationID);
				}else{
					GetAqLogger()->LogMessage(kInfo,"INFO: Association %d: closed\n", m_connectInfo.AssociationID);
				}
			}else{
				GetAqLogger()->LogMessage(kWarning,"WARNING: associationHandler::Process() - Association %d: DcmLib error (%d,%s) on MC_Read_Message() from host %s\n",
					m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS)status), m_connectInfo.RemoteHostName);
			}
			break;
		}
		m_active = TRUE;
		
		if (gConfig.m_messageDump)
		{
			TRDICOMUtil::DumpMessage(m_messageID, m_serviceName, m_command);
		}

		// 2012/02/14 K.Ko
		bool proc_commitent = false;
		if(m_serviceName){
			if (!strcmp(m_serviceName, kStorageCommitmentServiceName)){
				proc_commitent = true;
			}
		}

	//	if (!strcmp(m_serviceName, kStorageCommitmentServiceName))
		if(proc_commitent)
		{
			m_state = kStorageCommitmentProcess;
			AqStorageCommitment* pStoreCommit = new AqStorageCommitment(m_connectInfo, m_messageID);
			pStoreCommit->PreProcess();
			pStoreCommit->Process();
			delete pStoreCommit, pStoreCommit =0;
		//	Handover(pStoreCommit);
		}
		else if (m_command == C_ECHO_RQ)
		{
			m_state = kCEchoProcess;

			/*
			*  have responsed  API inside.
			*
			*/
#if 0
			RTVDiCOMCEcho cecho(m_connectInfo, m_messageID);
			cecho.PreProcess();
			status = (MC_STATUS)cecho.Process();
			if (status != MC_NORMAL_COMPLETION)
			{
				GetAqLogger()->LogMessage("ERROR:[C%08d] AssociationHandler::Process() - DcmLib error %d on return from CEcho::Process() from host %s - Association %d: closed\n",
					DicomServError_DicomAssociationError, status, m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			}
#endif
			MC_Free_Message(&m_messageID); // free mssage early to let cleanup work

			if(gConfig.m_traceDicomLevel >=2){ //#693
					GetAqLogger()->LogMessage("INFO:[C%08d]  C_ECHO_RQ - Received %s request from host %s over association %d \n", 
						DicomServInfor_DicomAssociationInfo,m_connectInfo.LocalApplicationTitle,m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			}else{
					GetAqLogger()->LogMessage(kInfo,"INFO: C_ECHO_RQ - Received %s  request from host %s over association %d \n", 
						m_connectInfo.LocalApplicationTitle, m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			}

			if (!strcmp(m_connectInfo.LocalApplicationTitle, "SHUTDOWN"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				 
				GetAqLogger()->LogMessage(kInfo,"INFO: AssociationHandler::Process() - Received Shutdown request from host %s over association %d - shutting down server\n", 
						m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			 
				Listener::theListener().RequestTermination(1);
				return false;
			} 
			else if(!strcmp(m_connectInfo.LocalApplicationTitle, "CLEANUP_MEMORY"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				Listener::theListener().RequestCleanupMemory(true);
				return false;
			} 
			else if(!strcmp(m_connectInfo.LocalApplicationTitle, "RESET_TOOLKIT"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				Listener::theListener().RequestToolkitReset(true);
				return false;
			} 
			else if (!strcmp(m_connectInfo.LocalApplicationTitle, "SYS_INFO"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				Listener::theListener().RequestSysInfo(true); // do print sys info after this thread finished
				GetAqLogger()->FlushLog();
				return false;
			}
			else if (!strcmp(m_connectInfo.LocalApplicationTitle, "AE_UPDATE"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				Listener::theListener().AETitlesChanged(); // AE title changed in database
				return false;
			}
			else if (!strcmp(m_connectInfo.LocalApplicationTitle, "WHL_UPDATE"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				RTVDiskSpaceManager::UpdateAll(); // check water makers changes
				return false;
			}
			else if (!strcmp(m_connectInfo.LocalApplicationTitle, "MINIDUMP"))
			{
				MC_Close_Association(&m_connectInfo.AssociationID);
				m_connectInfo.AssociationID = -1;
				//trigger a exception
				char* ptr = 0;
				*ptr = 3;
				return false;
			}
		}
		else if (m_command == C_STORE_RQ)
		{	
try{ // #11  2012/03/23 K.Ko
	// try_catch for C_STORE_RQ
 
			m_state = kCStoreProcess;
#ifdef _TIMING_ASSOC
			QueryPerformanceCounter(&startTime0); 
#endif
			if(gConfig.m_traceDicomLevel >=2){ //#693
					GetAqLogger()->LogMessage("INFO:[C%08d]  C_STORE_RQ - Received %s request from host %s over association %d \n", 
						DicomServInfor_DicomAssociationInfo,m_connectInfo.LocalApplicationTitle,m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			} 
#ifdef UseMemoryState
			_CrtMemState Before_CStore;
			_CrtMemCheckpoint(&Before_CStore);
#endif

			CStore* pCStore = new CStore(m_connectInfo, m_messageID);

#if 0 // test do nothing for debug
 //			pCStore->dbg_dumyProc(2);
 			pCStore->dbg_stepByStepProc(0);

			delete pCStore;
			pCStore = 0;
			m_messageID = -1;
#else
	/// {[( do real process
			int preProcessStatus ;
		 
			bool postResponseFlag = (gConfig.m_postDicomResponse != 0);
			if(postResponseFlag){   //2010/03/16 k.ko #660
				preProcessStatus = pCStore->PreProcessNoneResponce() ;
			}else{
				preProcessStatus = pCStore->PreProcess() ;
		 
			}
			if(preProcessStatus== MC_NORMAL_COMPLETION)
			{
#ifdef _TIMING_ASSOC
				QueryPerformanceCounter(&endTime); 
				m_checkTime += endTime.QuadPart - startTime0.QuadPart;
				startTime0 = endTime;

#endif
				threadWrite = gConfig.m_threadWrite;
				if(threadWrite && Threads() >= m_maxThreadsPerAssociation)
				{
					threadWrite = FALSE;
				}
				
				if(threadWrite && RTVDiCOMStore::c_totalInProcessing > RTVDiCOMStore::c_maxStoreThread)
				{
					threadWrite = FALSE;
				}

				if(threadWrite && pCStore->IsCompressed() &&
					RTVDiCOMStore::c_totalCompressedThread > RTVDiCOMStore::c_maxCompressedThread)
				{
					threadWrite = FALSE;
				}

				if (threadWrite)
				{
					Handover(pCStore);

					if(postResponseFlag){   //2010/03/16 k.ko #660
						pCStore->SuccessResponce();//not best
					}
				}
				else
				{
					bool error = true;
					if(postResponseFlag){   //2010/03/16 k.ko #660
						error = (pCStore->Process()!=MC_NORMAL_COMPLETION);
						pCStore->SuccessResponce(error);
					}else{
			//			pCStore->PreProcess();
						if(pCStore->Process()==MC_NORMAL_COMPLETION){
							error = false;
						}else{
							error = true;
						} 
					}
					if(!error){ //#21 2012/05/29 K.Ko
						m_seriesUID = pCStore->getSeriesUID();
					}
					delete pCStore;
				}
				pCStore = 0;
				//
#ifdef UseMemoryState
				_CrtMemState After_CStore,diff_CStore;
				_CrtMemCheckpoint(&After_CStore);
				if(_CrtMemDifference(&diff_CStore,&Before_CStore,&After_CStore)){
					bool dump_flag = false;
					if(dump_flag){
						_CrtMemDumpAllObjectsSince(&Before_CStore);

						_CrtDumpMemoryLeaks();
					}
					  _CrtMemDumpStatistics( &diff_CStore );
				}
#endif
				//
				
			}
			else
			{
				int st = pCStore->GetResponseStatus();
				// delete will send out CStore error message
				delete pCStore, pCStore = 0;
				if(st == C_STORE_FAILURE_REFUSED_NO_RESOURCES)
				{
					GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): Reject connection for not enough resource.\n",
						DicomServError_DicomAssociationError, m_connectInfo.AssociationID);
					MC_Close_Association(&m_connectInfo.AssociationID);
					m_connectInfo.AssociationID = -1;
	
					 return false; // can not continue receiving data, break the link
				}
			}
	
			m_messageID = -1; //turnover message ownership
#ifdef _TIMING_ASSOC
			QueryPerformanceCounter(&endTime);
			m_handoverTime += (endTime.QuadPart - startTime0.QuadPart);
#endif
			/// )]}  end - do real process
#endif
	
}catch(...) // #11  2012/03/23 K.Ko
{
	// try_catch for C_STORE_RQ
	GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): [Exception] C_STORE_RQ \n", 
			DicomServError_Exception,m_connectInfo.AssociationID);
	GetAqLogger()->FlushLog();
	RequestTermination(1);
}
		} 
		else if (m_command == C_FIND_RQ)
		{
try{ // #11  2012/03/23 K.Ko
	// try_catch for C_FIND_RQ
			if(gConfig.m_traceDicomLevel >=2){ //#693
				GetAqLogger()->LogMessage("INFO:[C%08d]  C_FIND_RQ - Received %s request from host %s over association %d\n", 
					DicomServInfor_DicomAssociationInfo,m_connectInfo.LocalApplicationTitle,m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			} 

			m_state = kCFindProcess;
			CFind cfind(m_connectInfo, m_messageID);
			cfind.PreProcess();
			status = (MC_STATUS) cfind.Process();
			if (status != MC_NORMAL_COMPLETION)
			{
				GetAqLogger()->LogMessage(kWarning,"WARNING:[C%08d] AssociationHandler::Process() - DcmLib error %d on return from CFind::Process() from host %s - Association %d: closed\n", 
					DicomServError_DicomAssociationError, status, m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			}
}catch(...) // #11  2012/03/23 K.Ko
{
	// try_catch for C_FIND_RQ
	GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): [Exception] C_STORE_RQ \n", 
			DicomServError_Exception,m_connectInfo.AssociationID);
	GetAqLogger()->FlushLog();
	RequestTermination(1);
}
		} 
		else if (m_command == C_MOVE_RQ)
		{
try{ // #11  2012/03/23 K.Ko
	// try_catch for C_MOVE_RQ
			if(gConfig.m_traceDicomLevel >=2){ //#693
				GetAqLogger()->LogMessage("INFO:[C%08d]  C_MOVE_RQ - Received %s request from host %s over association %d \n", 
					DicomServInfor_DicomAssociationInfo,m_connectInfo.LocalApplicationTitle,m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			} 

			m_state = kCMoveProcess;
			CMove cmove(m_connectInfo, m_messageID);
			cmove.PreProcess();
			status = (MC_STATUS) cmove.Process();
			if (status != MC_NORMAL_COMPLETION)
			{
				GetAqLogger()->LogMessage("ERROR:[C%08d] AssociationHandler::Process() - DcmLib error %d on return from CMove::Process() from host %s - Association %d: closed\n",
					DicomServError_DicomAssociationError,status, m_connectInfo.RemoteHostName, m_connectInfo.AssociationID);
			}
}catch(...) // #11  2012/03/23 K.Ko
{
	// try_catch for C_MOVE_RQ
	GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): [Exception] C_STORE_RQ \n", 
			DicomServError_Exception,m_connectInfo.AssociationID);
	GetAqLogger()->FlushLog();
	RequestTermination(1);
}
		}
		
		MC_Free_Message(&m_messageID); // free message before get new one

#ifdef _TIMING_ASSOC1
		LARGE_INTEGER startTime1;
		QueryPerformanceCounter(&startTime1); 
#endif
		// break big timeout (3600s) to small piecses
		status = MC_TIMEOUT; tmp_count = 0;
		while(tmp_count < n_timeout && status == MC_TIMEOUT && !TerminationRequested())
		{
			m_state = kMC_Read_Message;
			CleanStopped();
			status = MC_Read_Message(m_connectInfo.AssociationID, s_timeout, &m_messageID, &m_serviceName, &m_command);
			tmp_count++;
			if(tmp_count > 1)
				m_active = FALSE;
		}

#ifdef _TIMING_ASSOC1
		QueryPerformanceCounter(&endTime);
		m_waitTime += endTime.QuadPart - startTime1.QuadPart;
#endif


	}

}catch(...) // #11  2012/03/23 K.Ko
{
	// try_catch for ProcessMessages all
	GetAqLogger()->LogMessage("ERROR:[C%08d] (%d): [Exception] ProcessMessages \n", 
			DicomServError_Exception,m_connectInfo.AssociationID);
	GetAqLogger()->FlushLog();
	return false;
}
	return true;
}

//#21 2012/05/29 K.Ko
//Association終了時の処理の追加
#include "DiskSpaceManager.h"
#include "SeriesDirMonitor.h"
#include "AutoRoutingAEMan.h"
void AssociationHandler::Close()
{
	RTVAssociationHandler::Close();

	//
	if(gConfig.m_AutoRoutingTrig != AutoRouringTirg_AssociationClose){
		return ;
	}

	bool ret_b = true;
	RTVInactiveManager& imanger=RTVInactiveManager::theManager();

	if(	(m_command == C_STORE_RQ ) && 
		(m_seriesUID.size()>0)){
		//lock
		SeriesDirMonitor* pSeriesDirMonitor = (SeriesDirMonitor*)imanger.LockHandler(m_seriesUID.c_str(), this, true);
		if(pSeriesDirMonitor == 0)
		{
			LogMessage("ERROR:[C%08d]  : [Exception] AssociationHandler::Close  SeriesDirMonitor == 0\n", 
				DicomServError_AutoRoutingError);
			FlushLog();
			ret_b = false;
			return;
		}

		pSeriesDirMonitor->forceComplte();

		//Unlock
		imanger.LockHandler(m_seriesUID.c_str(), this, false);
	}
	 
	
}