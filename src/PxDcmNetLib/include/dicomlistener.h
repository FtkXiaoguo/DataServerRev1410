/***********************************************************************
 * DiCOMListener.h
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
#ifndef DiCOMLISTENER_H
#define DiCOMLISTENER_H

#include "RTVDiCOMListener.h"
#include "PxDicomStatus.h"
#include "PxDicomServer.h"

struct DiCOMConfig
{
	DiCOMConfig();

	char localAETitle[kMaxLocalAELen];
	int	 dicomLogLevel;
	char license[32];
	char dicomLogFile[256];
	int  maxLogFileSize;
	int listenPort;

	//	-- 06/06/03 For capturing DICOM packets straight from the tcp buffer
	int	networkCapture;

 	//	-- 06/11/03 An attempt to fix #3729 - CMove failing because SerialKey lookup fails
 	int	testMoveOriginator;

	int  storesOriginals;
	int	 m_enableMiniDumpTrigger;
	
	int m_forceTransferSyntax;
};

class DiCOMListener : public RTVDiCOMListener
{
public:

	static DiCOMListener& theListener();
	~DiCOMListener(){};

	void Config(DiCOMConfig& dconfig) {m_config = dconfig;};
	PxDicomStatus DICOM_Initialization();
	void DICOM_Release();
	int	 PreProcess(void);
	void CleanupMemory(void);
	void DoSysInfo(void);
	void IdleProcess(void);
	DiCOMConfig m_config;

private:
	DiCOMListener();
	void HandleAssociation(DiCOMConnectionInfo& connectInfo);
	//DWORD m_start_mem;
	
};

#endif // DiCOMLISTENER_H

