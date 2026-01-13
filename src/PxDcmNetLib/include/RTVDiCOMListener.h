/***********************************************************************
 * RTVDiCOMListener.h
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
#ifndef RTVDiCOMLISTENER_H
#define RTVDiCOMLISTENER_H

#include "rtvthread.h"
#include "AqCore/TRPlatform.h"
#include "RTVDiCOMService.h"
#include "RTVAssociationHandler.h"

#define kIdleTime				3600
#define kDefaultLocalAETitle	"AUTOVOX"

class RTVDiCOMListener : public iRTVThreadProcess, public RTVThreadManager<RTVAssociationHandler>
{
public:
	RTVDiCOMListener();
	~RTVDiCOMListener() {StopAll();};

	int Port() { return m_port; };
	virtual int Process(void);
	void RequestCleanupMemory(bool request) {m_requestMemoryCleanup = request;};
	void RequestToolkitReset(bool request) {m_requestToolkitReset = request;};
	int GetActiveThreads(int* opRealInactuiveThreads=0);
	virtual void CleanupMemory(void) {m_requestMemoryCleanup = false;};
	virtual void ResetToolkit(void) {m_requestToolkitReset = false;}
	void RequestSysInfo(bool request) {m_requestSysInfo = request;};
	virtual void DoSysInfo(void) {m_requestSysInfo = false;};
	virtual void IdleProcess(void) {};
	void AETitlesChanged() {m_newAE = 1;};
	virtual	void OnAETitlesChanged(bool first=false) {};
	virtual void DoAutoClean(void) {};

protected:
	virtual bool checkLicense() { return true;};// K.Ko 2010/05/21
	virtual void HandleAssociation(DiCOMConnectionInfo& connectInfo) = 0;
	void SetServiceList(char* iServiceListName);
	void SetRejectAllAssociations(int iYesNo) { m_rejectAllAssociations = iYesNo; }

	int m_port;
	bool m_requestMemoryCleanup;
	bool m_requestToolkitReset;
	bool m_requestSysInfo;
	int m_maxAssociations;
	int m_maxThreads;
	int m_newAE;

	char m_serviceListName[64];

	//	For testing only
	int m_rejectAllAssociations;
};

#endif // RTVDiCOMLISTENER_H

