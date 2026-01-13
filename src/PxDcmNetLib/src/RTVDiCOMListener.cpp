/***********************************************************************
 * RTVDiCOMListener.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
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

#include "RTVDICOMDef.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#include "PxDBData.h"

#else
#include "rtvMergeToolKit.h"
#include "CPxDBData.h"
#endif

#include "RTVDiCOMListener.h"
#include "RTVAssociationHandler.h"

//-------------------------------------------------------------------------
//
RTVDiCOMListener::RTVDiCOMListener()
{
	m_port = 0;
	m_requestMemoryCleanup = false;
	m_requestToolkitReset = false;
	m_requestSysInfo =false;
	m_maxAssociations = 10;
	m_maxThreads = 180;
	m_newAE = 0;

	ASTRNCPY(m_serviceListName, "TIDICOMServer_Service_List");

}

//-------------------------------------------------------------------------
//
int RTVDiCOMListener::GetActiveThreads(int* opRealInactuiveThreads/*=0*/)
{
	ThreadsMap::iterator iter;
	int nActive = 0;
	int realInactuiveThreads = 0;

	RTVAssociationHandler* pAssoc;
	TRCSLock fplock(&m_threadMapCS);
	// Get active thread number
	for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
	{
		pAssoc = (RTVAssociationHandler *)(iter->first);
		if(pAssoc->GetActive())
		{
			nActive++;
		}
		else if(opRealInactuiveThreads)
		{
			if(pAssoc->GetManagedThreads() > 1)
				realInactuiveThreads++;	
		}
	}
	fplock.Unlock();
	*opRealInactuiveThreads = realInactuiveThreads;
	return nActive;

}

//-------------------------------------------------------------------------
//
void RTVDiCOMListener::SetServiceList(char* iServiceListName)
{
	ASTRNCPY(m_serviceListName, iServiceListName);
}

//-------------------------------------------------------------------------
//
int RTVDiCOMListener::Process()
{
	//
	//	Set listen port
	//
	MC_STATUS status;
	int activeA;
	int realInactuiveThreads;
	
	int calledApplicationID;
	int associationID;
	AssocInfo ai;
	DiCOMConnectionInfo connectionIfno;

	int timeoutCounts = 0;
	while(!TerminationRequested())
	{	
		//	Autoclean wakeup check 
		DoAutoClean();

		//
		//	Wait for a C-STORE association request for uncompressed CT and MR only
		//	     need to make this a configurable parameter
		//
		//status = MC_Wait_For_Association("TIDICOMServer_Service_List", 1, &calledApplicationID, &associationID);
		status = MC_Wait_For_Association(m_serviceListName, 1, &calledApplicationID, &associationID);
		CleanStopped();
		
		activeA = GetActiveThreads(&realInactuiveThreads);
		if (realInactuiveThreads > 5) m_requestSysInfo = true;

		if (m_requestSysInfo)
		{
			m_requestSysInfo = false;
			DoSysInfo();
		}

		if(m_requestMemoryCleanup)
		{
			m_requestMemoryCleanup = false;
			CleanupMemory();
		} 

		if (m_requestToolkitReset)
		{
			m_requestToolkitReset = false;
			ResetToolkit();
		}

		if ( status == MC_NO_APPLICATIONS_REGISTERED)  // -- 08/08/2002
		{
			OnAETitlesChanged(true);   // treat this as first time as we never cleared appID
			GetAqLogger()->LogMessage(kWarning,"WARNING: NO_APPLICATIONS_REGISTERED: Will retry database\n");
			continue;
		}
	
		//printf("DICOM start on mem=%d\n", ProcessWorkset());
		if (status == MC_TIMEOUT)
		{
			timeoutCounts++;
			// process application titles change
			if(m_newAE) OnAETitlesChanged(), m_newAE = 0;
			
			//	Proceed only if there are no active threads
			if (timeoutCounts > 2 && Threads() == 0) // had 3 continue time out, process is in idle
			{
				timeoutCounts = 0;
				IdleProcess();
			}
			continue;
		}
		else
		{
			timeoutCounts = 0;
		}

		if (status != MC_NORMAL_COMPLETION)
		{
			GetAqLogger()->LogMessage("ERROR: RTVDiCOMListener::Process() - unable to process association request: DcmLib error code = (%d,%s)\n", status, MC_Error_Message(status));
			GetAqLogger()->FlushLog();
			Sleep(2000);
			continue;
		}

		//
		//	Process the association
		//
		status = MC_Get_Association_Info(associationID, &ai);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Reject_Association(associationID, TRANSIENT_NO_REASON_GIVEN);
			GetAqLogger()->LogMessage("ERROR: RTVDiCOMListener::Process() - Association %d: rejected - DcmLib error: (%d, %s) on MC_Get_Association_Info()\n", associationID, status, MC_Error_Message(status));
			GetAqLogger()->FlushLog();
			continue;
		}	

		//2017/02/14
		if (calledApplicationID < 0){
		
			GetAqLogger()->LogMessage("WARNING: Unregistered Local AE(called) %s \n", ai.LocalApplicationTitle);
			GetAqLogger()->FlushLog();
		}

		if (!checkLicense())
		{
			GetAqLogger()->LogMessage("ERROR: RTVDiCOMListener::Process() -   check license error \n");
			GetAqLogger()->FlushLog();
			MC_Reject_Association(associationID, TRANSIENT_LOCAL_LIMIT_EXCEEDED);
			continue;
		}

		if ( activeA >= m_maxAssociations)
		{
			GetAqLogger()->LogMessage(kWarning,"WARNING: RTVDiCOMListener::Process() - max associations %d already reached - rejecting association request from %s %s (%s)\n", 
					   m_maxAssociations, ai.RemoteIPAddress, ai.RemoteHostName, ai.RemoteApplicationTitle);
			GetAqLogger()->FlushLog();
			MC_Reject_Association(associationID, TRANSIENT_LOCAL_LIMIT_EXCEEDED);
			continue;
		}

		if (iRTVThread::Total >= m_maxThreads)
		{
			GetAqLogger()->LogMessage(kWarning,"WARNING: RTVDiCOMListener::Process() - Cmax threads %d already reached - rejecting association request from %s %s (%s)\n", 
					   m_maxThreads, ai.RemoteIPAddress, ai.RemoteHostName, ai.RemoteApplicationTitle);
			GetAqLogger()->FlushLog();
			MC_Reject_Association(associationID, TRANSIENT_LOCAL_LIMIT_EXCEEDED);
			continue;
		}

		if (m_rejectAllAssociations)
		{
			GetAqLogger()->LogMessage(kInfo,"INFO: RTVDiCOMListener::Process() - RejectAll mode for testing - rejecting association request from %s %s (%s)\n", 
					   ai.RemoteIPAddress, ai.RemoteHostName, ai.RemoteApplicationTitle);
			GetAqLogger()->FlushLog();
			MC_Reject_Association(associationID, PERMANENT_NO_REASON_GIVEN);
			continue;
		}

		//pAssociationHandler = new AssociationHandler(associationID);
		//pAssociationHandler->Process();
		//Handover(pAssociationHandler);
		connectionIfno.ApplicationID = calledApplicationID;
		connectionIfno.AssociationID = associationID;
		strcpy(connectionIfno.RemoteApplicationTitle, ai.RemoteApplicationTitle);
		strcpy(connectionIfno.RemoteHostName, ai.RemoteHostName);
		connectionIfno.Tcp_socket = ai.Tcp_socket;
		strcpy(connectionIfno.RemoteIPAddress, ai.RemoteIPAddress);
		strcpy(connectionIfno.LocalApplicationTitle, ai.LocalApplicationTitle);
		HandleAssociation(connectionIfno); 
	}

	GetAqLogger()->LogMessage(kInfo, "INFO: RTVDiCOMListener::Process(): shutting down\n");
	DoSysInfo();
	GetAqLogger()->FlushLog();
	StopAll(18000); // force all working thread to stop in 180 seconds
	GetAqLogger()->LogMessage(kInfo, "INFO: RTVDiCOMListener::Process(): exit\n");
	GetAqLogger()->FlushLog();
	return 0;
}
