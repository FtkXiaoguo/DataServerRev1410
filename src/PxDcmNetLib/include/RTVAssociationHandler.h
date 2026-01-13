/***********************************************************************
 * RTVAssociationHandler.h
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
#ifndef RTVASSOCIATION_HANDLER_H
#define RTVASSOCIATION_HANDLER_H

#include "RTVDiCOMService.h"

#define kIdleTime					   3600
// set threads per association to 10 to limit memory usage. Should use memory manager to do it

//#define _TIMING_ASSOC	1

class RTVAssociationHandler : public RTVDiCOMService, public RTVThreadManager<RTVDiCOMService>
{
public:
	enum ProcessState 
	{ 
		kInitialized, 
		kEnterProcess, 
		kMC_Read_Message,
		kCEchoProcess,
		kCStoreProcess,
		kCFindProcess,
		kCMoveProcess,
		kStorageCommitmentProcess,
		kInCleanStopped,
		kInStopAll,
		kWaitforNextWorker,
		kLeaveProcess,
		kEnterDestructor, 
		kLeaveDestructor,
	};


	RTVAssociationHandler(DiCOMConnectionInfo& connectInfo);
	virtual ~RTVAssociationHandler();

	int Process(void);
	virtual void LogProcessStatus(void);
	BOOL GetActive() {return m_active;};
	
	virtual int CheckLicenseStatus(void);

protected:
#ifdef _TIMING_ASSOC
	long double m_waitTime;
	long double m_checkTime;
	long double m_handoverTime;
#endif

	virtual void Close();
	virtual bool ProcessMessages() = 0;
	virtual int SendLicenseWillExpireEmail(const char* iSubj, const char* iMsg) { return 0; }

	int m_maxThreadsPerAssociation;
	int m_inactivityTimeout;
	ProcessState m_state;
	BOOL m_active;
};

#endif // RTVASSOCIATION_HANDLER_H
