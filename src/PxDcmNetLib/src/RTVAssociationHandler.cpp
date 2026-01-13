/***********************************************************************
 * RTVAssociationHandler.cpp
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

#include "RTVAssociationHandler.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"

#include "AQNetLicenseManager.h"

#endif

//-----------------------------------------------------------------------------
#ifdef _TIMING_ASSOC
static long double aCountsPerSecond = 0;
#endif

RTVAssociationHandler::RTVAssociationHandler(DiCOMConnectionInfo& connectInfo):
	RTVDiCOMService(connectInfo, -1)
{ 
//	m_maxThreadsPerAssociation = 25;
	m_inactivityTimeout = 3600;

	m_state = kInitialized;
	m_active = FALSE;

#ifdef _TIMING_ASSOC
	m_waitTime = 0;
	m_checkTime = 0;
	m_handoverTime = 0;
	if(aCountsPerSecond == 0)
	{
		LARGE_INTEGER countsPerSecond;
		QueryPerformanceFrequency(&countsPerSecond);
		aCountsPerSecond = countsPerSecond.QuadPart;
	}
#endif


}


//-----------------------------------------------------------------------------
void RTVAssociationHandler::LogProcessStatus(void)
{
	char *pState = "Unknown";
	switch(m_state)
	{
		case kInitialized:
			pState = "Initialized";
			break;
		case kEnterProcess:
			pState = "Enter Process";
			break;
		case kMC_Read_Message:
			pState = "Read_Message";
			break;
		case kCEchoProcess:
			pState = "CEcho Process";
			break;
		case kCStoreProcess:
			pState = "CStore Process";
			break;
		case kCFindProcess:
			pState = "CFind Process";
			break;
		case kCMoveProcess:
			pState = "CMove Process";
			break;
		case kStorageCommitmentProcess:
			pState = "Storage Commitment Process";
			break;
		case kInCleanStopped:
			pState = "In CleanStopped loop";
			break;
		case kInStopAll:
			pState = "In StopAll call";
			break;
		case kWaitforNextWorker:
			pState = "In WaitforNextWorker";
			break;
		case kLeaveProcess:
			pState = "Leave Process";
			break;
		case kEnterDestructor:
			pState = "Enter Destructor";
			break;
		case kLeaveDestructor:
			pState = "Leave Destructor";
			break;
		default:
			pState = "Unknown";
			break;

	}
	LogMessage("STATE_INFO: %s ID=%d, spawned threads=%d, state: %s\n", 
		m_processorName, m_connectInfo.AssociationID, Threads(), pState);
	
	// ask all spawned threads to log status
	// the map access has no threads access control here, it may cause data corruption
	// However we can not put the threads control because we want to report status even thread
	// is in lock up state. This function is designed to report state in lock up situation.
	ThreadsMap::iterator iter;
	TRCSLock fplock(&m_threadMapCS);
	RTVDiCOMService* pRTVDiCOMService;
	for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
	{
		pRTVDiCOMService = iter->first;
		pRTVDiCOMService->LogProcessStatus();
	}
	fplock.Unlock();

}

//-----------------------------------------------------------------------------
int RTVAssociationHandler::CheckLicenseStatus(void)
{
//	return LicenseManager::kLMLicenseValid;
	return 0;
}

//-----------------------------------------------------------------------------

int RTVAssociationHandler::Process()
{
	MC_STATUS	status;
	m_state = kEnterProcess;

#ifdef _TIMING_ASSOC
	LARGE_INTEGER startTime0;
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&startTime0); 
#endif

	//
	//	Debug messages
	//

#if 0
	//	Check license	
	std::string msg = "WARNING: License will expire soon.  Please contact TeraRecon for an updated license\n";
	int licenseStatus = CheckLicenseStatus();
	switch(licenseStatus)
	{
	case LicenseManager::kLMLicenseNoHASP:
	case LicenseManager::kLMHaspException:
		LogMessage("ERROR: No valid license - HASP key is not attached, or is invalid\n");
		FlushLog();
		return 0;
	case LicenseManager::kLMLicenseDisuse:
		LogMessage("ERROR: No valid license - Feature is not licensed\n");
		FlushLog();
		return 0;
	case LicenseManager::kLMLicenseDisabled:
		LogMessage("ERROR: No valid license - Feature is disabled\n");
		FlushLog();
		return 0;
	case LicenseManager::kLMLicenseExpired:
		LogMessage("ERROR: No valid license - Feature has expired\n");
		FlushLog();
		return 0;
	case LicenseManager::kLMLicenseWillExpire:
		LogMessage(msg.c_str());
		FlushLog();
		SendLicenseWillExpireEmail("License will expire soon", msg.c_str());
		break;
	case LicenseManager::kLMLicenseValid:
		break;
	};

#endif
	//
	//	Accept Association
	//
	status = MC_Accept_Association(m_connectInfo.AssociationID);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Reject_Association(m_connectInfo.AssociationID, TRANSIENT_NO_REASON_GIVEN);
		LogMessage("ERROR: RTVAssociationHandler::Process() - Association %d: rejected - DcmLib error: %d on MC_Accept_Association()\n", m_connectInfo.AssociationID, status);
		FlushLog();
		m_state = kLeaveProcess;
		return 0;
	}

	//	Added by Vikram 11/06/01 - We need to do this in order to avoid dropping response messages during C-FIND.
	//	Otherwise, any more than 3 query replies causes hang on the remote host.
	int t = 1;
	setsockopt(m_connectInfo.Tcp_socket,IPPROTO_TCP, TCP_NODELAY, (const char*)&t, sizeof(t));
	ProcessMessages();
	m_active = FALSE;

#ifdef _TIMING_ASSOC
	QueryPerformanceCounter(&endTime);
	long double seconds ;
	seconds = (endTime.QuadPart - startTime0.QuadPart)/aCountsPerSecond;
	LogMessage("ASSOC %d, process time %lf s\n", m_connectInfo.AssociationID, seconds);

	LogMessage("ASSOC %d, wait time %lf s\n", m_connectInfo.AssociationID, m_waitTime/aCountsPerSecond);
	LogMessage("ASSOC %d, check time %lf s\n", m_connectInfo.AssociationID, m_checkTime / aCountsPerSecond);
	LogMessage("ASSOC %d, handover time %lf s\n", m_connectInfo.AssociationID, m_handoverTime / aCountsPerSecond);
	
#endif

	Close();
	m_state = kLeaveProcess;
	return 0;
}

//-----------------------------------------------------------------------------
void RTVAssociationHandler::Close()
{
	// make sure all active thread finished
	int t = 0;
	while(Threads() && t < 18000 && !TerminationRequested())
	{
		m_state = kInCleanStopped;
		Sleep(1000);
		t++;
		CleanStopped();
	}

	if(m_connectInfo.AssociationID >= 0)
	{
		MC_Close_Association(&m_connectInfo.AssociationID);
		m_connectInfo.AssociationID = -1;
	}
	if(Threads())
	{
		m_state = kInStopAll;
		LogMessage("force to stop %d CStore threads\n", Threads());
		StopAll(18000);
	}
	FlushLog();
	MC_Free_Message(&m_messageID);
}

//-----------------------------------------------------------------------------

RTVAssociationHandler::~RTVAssociationHandler() 
{	
	Close();
}

//-----------------------------------------------------------------------------