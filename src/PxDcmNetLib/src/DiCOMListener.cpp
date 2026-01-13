/***********************************************************************
 * DiCOMListener.cpp
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		Listens for DICOM C-STORE requests on a port in its own thread.
 *		Each new association request kicks off a new AssociationHandler
 *		thread.
 *
 *	AUTHOR(S):  Gang Li, July 2002
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4503)

#include "rtvMergeToolKit.h"

//#include "nvr_server.h"
#include "VLIDicom.h"
#include "TRDICOMUtil.h"
#include "DiCOMAssociation.h"

#include "DiCOMListener.h"

#ifndef STANDALONE_SCU
#include "nvr_server.h"
#else
#include "AqCore/TRLogger.h"
#endif

extern TRLogger gLogger;

extern MC_STATUS GetTagOBOW(int messageID,unsigned long tag,void* userInfo,CALLBACK_TYPE CBtype, unsigned long* dataSizePtr,void** dataBufferPtr,int isFirst,int* isLastPtr);

//-----------------------------------------------------------------------------------------
//
DiCOMConfig::DiCOMConfig()
{
	localAETitle[0] = 0;
	dicomLogLevel = 0;
	license[0] = 0;
	dicomLogFile[0] = 0;
	maxLogFileSize = 0;
    listenPort = 104; 
	networkCapture = 0;

	//	Rob Lewis 06/11/03 An attempt to fix #3729 - CMove failing because SerialKey lookup fails
	testMoveOriginator = 0;

	storesOriginals = 0;

	m_forceTransferSyntax = 0;
}

//-----------------------------------------------------------------------------------------
//
DiCOMListener& DiCOMListener::theListener()
{
	static DiCOMListener p; // the signle Listener object
	return p;

}

//-----------------------------------------------------------------------------------------
//
DiCOMListener::DiCOMListener()
{
	m_processorName = "DiCOMListener";
}

extern int VLIDicomApplicationID;
extern char VLIDicomLocalAETitle[kMaxLocalAELen];

//-----------------------------------------------------------------------------------------
//
VLIDicomStatus DiCOMListener::DICOM_Initialization()
{
	VLIDicomStatus status;
	
	status =  (VLIDicomStatus) MC_Library_Initialization(MC_Config_Values, MC_Dictionary_Values, NULL);
	if (status != kNormalCompletion)
		return status;
	
	TRDICOMUtil::SetLogBits(m_config.dicomLogLevel);
	
	//	Enable Merge Network Capture file
	if (m_config.networkCapture)
	{
		TRDICOMUtil::EnableNetworkCapture("C:/AQNetLog/AQNetImageServerMerge.cap");
	}
	
	//	Set the Merge License - Rob Lewis 4/10/02
	status = (VLIDicomStatus) MC_Set_String_Config_Value(LICENSE, m_config.license);
	if (status != kNormalCompletion)
		return status;

	//	Set the Merge Log File for the Server
	status = (VLIDicomStatus) MC_Set_String_Config_Value(LOG_FILE, m_config.dicomLogFile);
	if (status != kNormalCompletion)
		return status;
	MC_Set_Int_Config_Value(NUM_HISTORICAL_LOG_FILES, 2);
	MC_Set_Int_Config_Value(LOG_FILE_SIZE, (m_config.maxLogFileSize * 1024) / 80);
	//MC_Set_Int_Config_Value(WORK_BUFFER_SIZE, 250000);
	
	int applicationID;
	status = (VLIDicomStatus) MC_Register_Application(&applicationID, m_config.localAETitle);
	if (status != kNormalCompletion)
	{
		return status;
	}
	
	status = (VLIDicomStatus) MC_Register_Callback_Function(applicationID, MC_ATT_PIXEL_DATA, 0, GetTagOBOW);
	if (status != kNormalCompletion)
	{
		return status;
	}

	// copy to VLIDiCOM, should let VLIDiCOM reference this
	VLIDicomApplicationID = applicationID;
	strcpy(VLIDicomLocalAETitle, m_config.localAETitle);

	//	Rob Lewis 06/11/03 An attempt to fix #3729 - CMove failing because SerialKey lookup fails
 	VLIDicom::m_testMoveOriginator = m_config.testMoveOriginator;
	
	// register special AE titles
	int tmp;
	MC_Register_Application(&tmp,"SYS_INFO" );
	MC_Register_Application(&tmp,"WHL_UPDATE");
	MC_Register_Application(&tmp,"CLEANUP_MEMORY");
	MC_Register_Application(&tmp,"RESET_TOOLKIT");
	if(m_config.m_enableMiniDumpTrigger)
		MC_Register_Application(&tmp,"MINIDUMP");

//#ifdef _DEBUG
//#endif
	
	//	Rob Lewis - 03/14/02 - Turns off Reverse hostname lookup
	MC_Set_Bool_Config_Value(ACCEPT_ANY_HOSTNAME, 1);
	
	// Rob Lewis - 06/03/02 - Changed from 6 to 10 so most assoc can complete and we can still
	//		cancel requests in a reasonable time
	MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, 10);
	MC_Set_Int_Config_Value(CONNECT_TIMEOUT, 10);
	
	//	Don't want these to time out too soon
	MC_Set_Int_Config_Value(INACTIVITY_TIMEOUT, 30);
	MC_Set_Int_Config_Value(WRITE_TIMEOUT, 10);
	MC_Set_Int_Config_Value(RELEASE_TIMEOUT, 10);
	MC_Set_Int_Config_Value(ARTIM_TIMEOUT, 10);
	
	m_port = m_config.listenPort;
	status = (VLIDicomStatus) MC_Set_Int_Config_Value(TCPIP_LISTEN_PORT, m_port);
    if (status != MC_NORMAL_COMPLETION)
    {
        gLogger.LogMessage("ERROR: DiCOMListener::DICOM_Initialization() - unable to set listen port: Merge error code = %d\n", status);
		gLogger.FlushLog();
		return status;
    }

	if (m_config.m_forceTransferSyntax == 1)
	{
		SetServiceList("TIDICOMServer_Implicit_LE_Service_List");
	}
	else if (m_config.m_forceTransferSyntax == 2)
	{
		SetServiceList("TIDICOMServer_Explicit_LE_Service_List");
	}
	else if (m_config.m_forceTransferSyntax == 3)
	{
		SetServiceList("TIDICOMServer_Explicit_BE_Service_List");
	}
	else // (m_config.m_forceTransferSyntax == 0)
	{
		SetServiceList("TIDICOMServer_Service_List");
	}

	// T.C. Zhao 2005.09.22
	// check the validity of the Merge license (may not be the best way)
	int emptyID = -1;
    status = (VLIDicomStatus)MC_Open_Empty_Message(&emptyID);
	if (status == kNormalCompletion && emptyID > 0)
	{
		MC_Free_Message(&emptyID);
	}
	return status;
}

//-----------------------------------------------------------------------------------------
//
void DiCOMListener::DICOM_Release()
{
	MC_Library_Release();
}


#include <Psapi.h>
// this one need Psapi.lib
DWORD ProcessWorkset()
{
	PROCESS_MEMORY_COUNTERS psmemCounters;
	if( !GetProcessMemoryInfo(GetCurrentProcess(),&psmemCounters, sizeof(psmemCounters)) )
		return 0;
	return psmemCounters.WorkingSetSize;
}

//-----------------------------------------------------------------------------------------
//
int	DiCOMListener::PreProcess(void)
{ 
	m_maxAssociations = 10;
	m_maxThreads = 500;
	
	m_processStatus = kToBeStarted;
	// the first incoming association will make it 4 times big, so times 5 to avoid false MC release
	//m_start_mem = ProcessWorkset()*5; 
	return 0;

}

//-----------------------------------------------------------------------------------------
//
void DiCOMListener::CleanupMemory(void)
{
	MC_Cleanup_Memory(10);
}

//-----------------------------------------------------------------------------------------
//
void DiCOMListener::DoSysInfo(void)
{
	gLogger.LogMessage("SYS_INFO: Total threads=%d, Association threads=%d\n",iRTVThread::Total, Threads());
	// ask all spawned threads to log status
	// the map access has no threads access control here, it may cause data corruption
	// However we can not put the threads control because we want to report status even thread
	// is in lock up state. This function is designed to report state in lock up situation.
	ThreadsMap::iterator iter;
	TRCSLock fplock(&m_threadMapCS);
	RTVAssociationHandler* pAssociationHandler;
	for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
	{
		pAssociationHandler = iter->first;
		pAssociationHandler->LogProcessStatus();
	}
	fplock.Unlock();

	gLogger.LogMessage("\nSYS_INFO: Memory usage = %d\n", ProcessWorkset());

	//gLogger.LogMessage("SYS_INFO: RTVInactiveManager m_state = %d\n", RTVInactiveManager::theManger().GetState());
#ifdef _TRACE_MEMORY
	gLogger.LogMessage("SYS_INFO: open messages  = %d\n", MergeToolKit::GetOpenMessageCount());
	gLogger.LogMessage("SYS_INFO: open files     = %d\n", MergeToolKit::GetOpenFileCount());
	gLogger.LogMessage("SYS_INFO: open items     = %d\n", MergeToolKit::GetOpenItemCount());
	MergeToolKit::PrintMergeIDCounts();
#endif
	gLogger.FlushLog();
}

//-----------------------------------------------------------------------------------------
//
void DiCOMListener::IdleProcess(void)
{
/*
#ifndef _TRACE_MEMORY
	// if no active associations and no outstanding serial monitors,
	// restart merge tool kit to regain leaked memory.
	if(Threads() == 0 )
	{
		
		if(ProcessWorkset() < m_start_mem*1.587) // about the memory watermark go on
			return;	
		gLogger.LogMessage("DICOM restart on mem=%d\n", ProcessWorkset());
		
		DICOM_Release();
		if(!DICOM_Initialization())
		{
			gLogger.LogMessage("ERROR: Listener::Process() - DICOM_Initialization() failed\n");
			gLogger.FlushLog();
			RequestTermination(1);
			return;
		}
		
		//MC_Cleanup_Memory(10); // there is memory leak in our side, don't use it yet
		
		m_start_mem = ProcessWorkset(); // adjust memory watermark
	}
#endif				
*/	
}

//-----------------------------------------------------------------------------------------
//
void DiCOMListener::HandleAssociation(DiCOMConnectionInfo& connectInfo)
{
		DiCOMAssociation* pAssociation = new DiCOMAssociation(connectInfo);
		Handover(pAssociation);
}