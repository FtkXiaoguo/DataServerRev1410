/***********************************************************************
 * Listener.h
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
#ifndef LISTENER_H
#define LISTENER_H

#include "RTVDiCOMListener.h"

class Listener : public RTVDiCOMListener
{
public:
	typedef std::map<std::string, int> AE_ApplicationID_Map;

	static Listener& theListener();
	~Listener(){};

	bool DICOM_Initialization();
	void DICOM_Release();
	int	 PreProcess(void);
	int Process(void);
	void CleanupMemory(void);
	void ResetToolkit(void);
	void DoSysInfo(void);
	void IdleProcess(void);
	void OnAETitlesChanged(bool first=false);
	void DoAutoClean(void);

	void CheckMemory(void); //2012/03/27 K.Ko
private:
	Listener();

	virtual bool setupLocalBackupAE();//#93Å@2017/02/13 N.Furutsuki
	virtual bool checkLicense();// K.Ko 2010/05/21
 
	void HandleAssociation(DiCOMConnectionInfo& connectInfo);

	int m_newAE;
	AE_ApplicationID_Map m_AEApplicationIDMap;
	DWORD m_start_mem;

	bool m_firstCheckFlag;
	bool m_checkedLicenseStatus;// K.Ko 2010/05/21
	FILETIME m_lastCheckLicenseTime;// K.Ko 2010/05/21
};

#endif // LISTENER_H
