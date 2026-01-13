/***********************************************************************
 * DataMonitor.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Monitor incoming DICOM data to detect when all DICOM data for 
 *		the specified patient are completed.
 *
 *	
 *  
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "DataMonitor.h"
#include "Globals.h"
#include "PxDB.h"


//-----------------------------------------------------------------------------------------------------
//
DataMonitor::DataMonitor(const DiCOMConnectionInfo& iCInfo, const char* iPatientID, const char* iSeriesUID)
{
	m_lastActiveTime = 0;
	m_connectInfo = iCInfo;

	m_patientID = iPatientID?iPatientID:"";
	m_seriesUID = iSeriesUID?iSeriesUID:"";

	gLogger.LogMessage(kInfo,"INFO:(%d|%X) new DataMonitor on: %s for series: %s from %s\n", m_connectInfo.AssociationID, 
		this, m_patientID.GetString(), m_seriesUID.GetString(), m_connectInfo.RemoteHostName);

	CPxDB db;
	db.InitDatabaseInfo();
	m_startDBTime = db.GetDBDateTime();

	if(gConfig.m_seriesCompleteTimeout < 15 )
	{
		m_waitTime = 45;
	}
	else
	{
		m_waitTime = gConfig.m_seriesCompleteTimeout * 2;
	}
		
}

//-----------------------------------------------------------------------------------------------------
//
DataMonitor::~DataMonitor()
{
	if(m_lastActiveTime > 0)
	{
		gLogger.LogMessage(kInfo,"INFO:(%d|%X) close DataMonitor on:%s, from %s\n", 
			m_connectInfo.AssociationID, this, m_patientID.GetString(), m_connectInfo.RemoteHostName);
	}
	else
	{
		gLogger.LogMessage(kInfo,"INFO:(%d|%X) close UNUSED DataMonitor on:%s, from %s\n", 
			m_connectInfo.AssociationID, this, m_patientID.GetString(), m_connectInfo.RemoteHostName);

	}
}

//-----------------------------------------------------------------------------------------------------
//
bool DataMonitor::IsTimeOver(DWORD TickCount)
{
	long overtime = (TickCount-m_lastActiveTime)/1000 - m_waitTime;

	if (overtime > 3600)
	{
		m_keepMap.Clear(); // clear locks for 1 hour old lock
		return true;
	}
	else
		return (overtime > 0);

}

//-----------------------------------------------------------------------------------------------------
//
void DataMonitor::ForceTimeOut(void)
{	
	Process();
}


//-----------------------------------------------------------------------------------------------------
//
int DataMonitor::Process(void)
{	
	
	CPxDB db;
	db.InitDatabaseInfo();
	
	//chetan, 6/6/2006 - if APS is not enabled, just exit
	if (!db.IsAPSEnableded())
	{
		return 0;
	}

	gLogger.LogMessage(kInfo,"INFO:(%d|%X)  DataMonitor process on:%s, from %s;\n", 
		m_connectInfo.AssociationID, this, m_patientID.GetString(), m_connectInfo.RemoteHostName);

	m_endDBTime = db.GetDBDateTime();

	char cmdline[1025]; cmdline[1024] = 0;

	AqString time1, time2;
	AqBinToHex(&m_startDBTime, sizeof(double), time1);
	AqBinToHex(&m_endDBTime,sizeof(double), time2);
	

	//use following code to get back double
	//AqString strBuffer;
	//void * pValue = strBuffer.HexToBin(keyString.c_str());
	//oKeyValue = *((double*)pValue);

	strcpy(cmdline, "\"C:\\Program Files\\AQAPS\\Bin\\WorkflowCreator.exe\" \"");
	//strcpy(cmdline, "C:\\Program Files\\AQUARIUS_APS\\WorkflowCreator.exe \"");
	strcat(cmdline, m_patientID.GetString()); strcat(cmdline, "\" \"");
	strcat(cmdline, m_seriesUID.GetString()); strcat(cmdline, "\" ");
	strcat(cmdline, time1.GetString()); strcat(cmdline, " ");
	strcat(cmdline, time2.GetString()); strcat(cmdline, " ");
	char buffer[8]; itoa(gConfig.m_dicomLogLevel, buffer, 10);
	strcat(cmdline, buffer);



	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof(pi) );
	int retcode =   CreateProcess(NULL,
					cmdline,	// Command line. 
					NULL,	// Process handle not inheritable. 
					NULL,	// Thread handle not inheritable. 
					FALSE,	// Set handle inheritance to FALSE. 
					DETACHED_PROCESS,
					NULL,	// Use parent's environment block. 
					NULL,	// Use parent's starting directory. 
					&si,	// Pointer to STARTUPINFO structure.
					&pi );	// Pointer to PROCESS_INFORMATION structure.

	if (retcode == 0)
	{
		int errorStatus = GetLastError();
		gLogger.LogMessage(kInfo,"INFO: Failed to run workflow creator.exe with the following error status = %d\n", errorStatus);
	}

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );


	return 0;
}


//-----------------------------------------------------------------------------
