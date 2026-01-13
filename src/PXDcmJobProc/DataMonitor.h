/***********************************************************************
 * DataMonitor.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2001-2006, All rights reserved.
 *
 *	PURPOSE:
 *		Monitor incoming DICOM data to detect when all DICOM data for 
 *		the specified patient are completed.
 *
 *	AUTHOR(S):  Gang Li, April 2006
 *  
 *-------------------------------------------------------------------
 */
#ifndef DATA_MONITOR_H
#define DATA_MONITOR_H

#include "AqCore/AqString.h"
#include "rtvPoolAccess.h"
#include "RTVDiCOMService.h"


class DataMonitor : public InactiveHandler
{
public:
	DataMonitor(const DiCOMConnectionInfo& iCInfo, const char* iPatientID, const char* iSeriesUID);
	~DataMonitor();
	int Process(void);

	virtual void Kick(void) {m_lastActiveTime = GetTickCount();};
	virtual bool IsTimeOver(DWORD TickCount);
	virtual void ForceTimeOut();
	const char* GetPatientID() const { return m_patientID.GetString(); }	
	const DiCOMConnectionInfo& GetConnectionInfo(void) const { return m_connectInfo; }
	double GetStartDBTime() {return m_startDBTime;}


protected:
	
	AqString	m_patientID;
	AqString	m_seriesUID;
	DWORD	m_lastActiveTime;
	double	m_startDBTime;
	double	m_endDBTime;
	
	long	m_waitTime;

	DiCOMConnectionInfo m_connectInfo;

};

#endif // DATA_MONITOR_H

