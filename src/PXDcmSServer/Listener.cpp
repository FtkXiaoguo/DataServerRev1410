/***********************************************************************
 * Listener.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2011, All rights reserved.
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

#include "Listener.h"

#include "Globals.h"
#include "rtvPoolAccess.h"
#include "PxDicomutil.h"
#include "AssociationHandler.h"
#include "AppComConfiguration.h"

#include "PxNetDB.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "IDcmLibDefUID.h"

using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"

#endif

//#8 2012/03/16 K.KO
//ServiceList指定仕組みの追加
#include "PxDicomMessage.h"
#include "PxDicomimage.h"
#include "CMove.h"

//
// for Memory Dump
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>



const int kTwentyFourHours = 60 * 60 * 24;

 
 

extern TRLogger gDcmApiLogger; // #4 2012/02/21 K.Ko

//-----------------------------------------------------------------------------
//
Listener& Listener::theListener()
{
	static Listener p; // the signle Listener object
	return p;

}

//-----------------------------------------------------------------------------
//
Listener::Listener()
{
m_processorName = "Listener";

 m_firstCheckFlag = true;
 m_checkedLicenseStatus = false;
}

//-----------------------------------------------------------------------------
//
#define M_ADD_SOP_CLASS(sop_cls) { \
	if(!TRDICOMUtil::addSOPClassUID((sop_cls),m_serviceListName)) \
	{\
		gLogger.LogMessage("ERROR: Listener::DICOM_Initialization TRDICOMUtil::addSOPClassUID [%s] \n", (sop_cls));\
		gLogger.FlushLog();\
		return false;\
	}\
}
bool Listener::DICOM_Initialization()
{
	int status;

	gLogger.LogMessage("DICOM_Initialization m_dicomLogLevel[%d]\n", gConfig.m_dicomLogLevel);
	gLogger.FlushLog();
 

//change the logger
// #4 2012/02/21 K.Ko
	if(!TRDICOMUtil::InitialDICOM(
		"PXDcmAPI_License", 
		gConfig.m_dicomLogLevel, 
		&gDcmApiLogger,
		gConfig.m_logMaxSize, 
		(gConfig.m_networkCapture)?gConfig.m_mergeCap:0, 
		(gConfig.m_networkCapture)?gConfig.m_numberOfNetworkCapture:0)
		)

	{
		gLogger.LogMessage("%s: MergeCOM Init failed\n", gConfig.m_serverNameStr);
		gLogger.FlushLog();
		return false;
	}

	//	-- - 03/14/02 - Turns off Reverse hostname lookup
	status = MC_Set_Bool_Config_Value(ACCEPT_ANY_HOSTNAME, 1);
	if (status != MC_NORMAL_COMPLETION)
	{
		gLogger.LogMessage("%s: Unable to turn off reverse hostname lookup: status = %d\n", gConfig.m_serverNameStr, status);
		gLogger.FlushLog();
	}

	MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, gConfig.m_assocationReplyTimeout);
	MC_Set_Int_Config_Value(CONNECT_TIMEOUT, gConfig.m_connectTimeout);
	MC_Set_Int_Config_Value(WRITE_TIMEOUT, gConfig.m_writeTimeout);
	MC_Set_Int_Config_Value(RELEASE_TIMEOUT, gConfig.m_releaseTimeout);
	MC_Set_Int_Config_Value(INACTIVITY_TIMEOUT, gConfig.m_inactivityTimeout);	

    m_port = gConfig.m_port;
	status = MC_Set_Int_Config_Value(TCPIP_LISTEN_PORT, m_port);
    if (status != MC_NORMAL_COMPLETION)
    {
        gLogger.LogMessage("ERROR: RTVDiCOMListener::Process() - unable to set listen port: DcmLib error code = %d\n", status);
		gLogger.FlushLog();
		return false;
    }

	SetRejectAllAssociations(gConfig.m_rejectAllAssociations);

	////////////////////////////////////////////////////////////////
	//
	//	 
	//
	////////////////////////////////////////////////////////////////

 
	if (1)//gAqNETDental)
	{
		SetServiceList("AqNETDental_Service_List");
	}
	 
	else
 
	{
		switch(gConfig.m_forceTransferSyntax)
		{
		case 1:
			SetServiceList("TIDICOMServer_Implicit_LE_Service_List");
			break;
		case 2:
			SetServiceList("TIDICOMServer_Explicit_LE_Service_List");
			break;
		case 3:
			SetServiceList("TIDICOMServer_Explicit_BE_Service_List");
			break;
		case 0:
		default:
			SetServiceList("TIDICOMServer_Service_List");
			break;
		}
	}

 

	//	char* proposedServiceList = 0;
	//	This is some test code for creating service lists on the fly.  I'm leaving it here because we may want to do this later.
		//		For now, I'm using the merge config files.

/*		const int cSCU_ROLE_NO = 0;
		const int cSCP_ROLE_YES = 1;

		//	Create Service List for forcing CT ImplicitLittleEndian 
		status = MC_NewServiceFromName("CT_ImplicitLittleEndian", "STANDARD_CT", "IMPLICIT_L_E_Syntax_List", cSCU_ROLE_NO, cSCP_ROLE_YES);
		if (status != MC_NORMAL_COMPLETION)
		{
			gLogger.LogMessage("ERROR: Listener::DICOM_Initialization(): MC_NewServiceFromName returned DcmLib error code = (%d,%s)\n", status, MC_Error_Message((MC_STATUS)status));
			gLogger.FlushLog();
			return false;
		}

		char* serviceNameArray[2];
		serviceNameArray[0] = "CT_ImplicitLittleEndian";
		serviceNameArray[1] = 0;

		status = MC_NewProposedServiceList("TIDICOMServer_Implicit_LE_Service_List", serviceNameArray);
		if (status != MC_NORMAL_COMPLETION)
		{
			gLogger.LogMessage("ERROR: Listener::DICOM_Initialization(): MC_NewProposedServiceList returned DcmLib error code = (%d,%s)\n", status, MC_Error_Message((MC_STATUS)status));
			gLogger.FlushLog();
			return false;
		}


	}

*/	 

//#8 2012/03/09 K.KO
//ServiceList指定仕組みの追加
	
	if(!TRDICOMUtil::initServiceList(m_serviceListName))
	{
		gLogger.LogMessage("ERROR: Listener::DICOM_Initialization TRDICOMUtil::initServiceList [%s] \n", m_serviceListName);
		gLogger.FlushLog();
		return false;
	}
	
	/////////////////////////////
	M_ADD_SOP_CLASS(UID_CTImageStorage);
	M_ADD_SOP_CLASS(UID_XRayAngiographicImageStorage);
	//
	M_ADD_SOP_CLASS(UID_XRayAngiographicImageStorage);
	M_ADD_SOP_CLASS(UID_XRayRadiofluoroscopicImageStorage);
	M_ADD_SOP_CLASS(UID_XRayRadiationDoseSRStorage);
	M_ADD_SOP_CLASS(UID_SecondaryCaptureImageStorage);

	//
//	M_ADD_SOP_CLASS(UID_FINDModalityWorklistInformationModel);
#if 1
	M_ADD_SOP_CLASS(UID_FINDPatientRootQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_FINDStudyRootQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_GETPatientRootQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel); 
	M_ADD_SOP_CLASS(UID_GETStudyRootQueryRetrieveInformationModel);


	M_ADD_SOP_CLASS(UID_MOVEPatientRootQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_MOVEStudyRootQueryRetrieveInformationModel);
	M_ADD_SOP_CLASS(UID_FINDGeneralPurposeWorklistInformationModel);
	//
	M_ADD_SOP_CLASS(UID_DigitalIntraOralXRayImageStorageForPresentation);
	//
	M_ADD_SOP_CLASS(UID_DigitalXRayImageStorageForPresentation);// #23 2012/07/07 K.KO for DX
	M_ADD_SOP_CLASS(UID_DigitalXRayImageStorageForProcessing);// #23 2012/07/07 K.KO for DX
	 
#endif


//#8 2012/03/16 K.KO
//ServiceList指定仕組みの追加
	 CMove::initCMoveServiceList();

	return true;
}

//-----------------------------------------------------------------------------
//
void Listener::DICOM_Release()
{
	TRDICOMUtil::ReleaseDICOM();
}

//-----------------------------------------------------------------------------
//
char *ToUpper(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = toupper(*s);
	return ret;
}

//-----------------------------------------------------------------------------
//
static char* GetMyName(void)
{
	static char myName[128];

	if (myName[0])
		return myName;
	
	char *p;
	gethostname(myName, sizeof(myName)-1);
	if ( ( p = strchr(myName,'.')))
		*p = '\0';
	return myName;
}


int Listener::Process(void)
{
	CPxDcmDB db;
	db.ActiveLocalAE(GetMyName(), true);
	
	int rcd = RTVDiCOMListener::Process();

	db.ActiveLocalAE(GetMyName(), false);

	return rcd;
}

//-----------------------------------------------------------------------------
//
void Listener::OnAETitlesChanged(bool first)
{
	gLogger.LogMessage( "***AE Config file changed\n");

	//#93　2017/02/13 N.Furutsuki
	//#94
	bool isLocalBackup = AppComConfiguration::GetLocalBackupFlag() == 1;

	
	CPxDcmDB db;
	
	// if database initialization fails, Listener will call this function again
	if(db.InitDatabaseInfo(false, first?20:0) == false)
	{
		gLogger.LogMessage("ERROR:[C%08d] -Listener::OnAETitlesChanged, - failed to initialize DB: %s\n",DicomServError_DBInitError, db.GetServerName());
		Sleep(1000);
		if(first)
			Sleep(1000);
		return;
	}
	else if(first)
	{
		gLogger.LogMessage(kInfo,"INFO: -Listener::OnAETitlesChanged, Database initialization success\n");
		const char* queryStr = "select count(*) from serieslevel where status = -2";
		int oldSeriesNum = 0;
	//	db.SQLGetInt(queryStr, oldSeriesNum);
		if(oldSeriesNum > 0 && 0)  // TCZ&GangLi 2006.06.30 disable for v1.7.2
		{
			_flushall();
			STARTUPINFO si;
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi;
			ZeroMemory( &pi, sizeof(pi) );
			CreateProcess(NULL,
							"CPxDcmDBChecker.exe -v",	// Command line. 
							NULL,	// Process handle not inheritable. 
							NULL,	// Thread handle not inheritable. 
							FALSE,	// Set handle inheritance to FALSE. 
							DETACHED_PROCESS,
							NULL,	// Use parent's environment block. 
							NULL,	// Use parent's starting directory. 
							&si,	// Pointer to STARTUPINFO structure.
							&pi );	// Pointer to PROCESS_INFORMATION structure.
			// Close process and thread handles. 
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}

	}

	if(gConfig.m_dbConnectionPool)
		db.ConnectionPooling(true);

	AE_ApplicationID_Map userAE;	
	AE_ApplicationID_Map::iterator app_iter, user_iter;
	// get user AE for this machine from database
	
	if(db.GetAllLocalAE(m_port, userAE, db.InCluster()?GetMyName():0) != kOK){
		gLogger.LogMessage("ERROR:[C%08d] GetLocalAE failed in Listener::OnAETitlesChanged\n",DicomServError_InvalidDicomAE);
	}

	gLogger.LogMessage("------\n");
	gLogger.LogMessage(">>db.InCluster() %d, GetMyName() %s\n", db.InCluster(),GetMyName());
	gLogger.LogMessage("------\n");
	for (user_iter = userAE.begin(); user_iter != userAE.end(); user_iter++)
	{
		gLogger.LogMessage(">>user AE: %s\n", user_iter->first.c_str());
	}
	gLogger.LogMessage("------\n");

	//
	//	Add default AE Title to the map
	//

	AE_ApplicationID_Map reset_locale_userAE_list;	//#70 その他ローカルAE対応　2013/09/18
	AE_ApplicationID_Map locale_userAE_temp;	
	{
		AE_ApplicationID_Map::iterator it_temp;
		it_temp = userAE.begin();
		while(it_temp != userAE.end()){
			locale_userAE_temp[it_temp->first] = it_temp->second;
			reset_locale_userAE_list[it_temp->first] = 0;//#70 その他ローカルAE対応　2013/09/18
			it_temp++;
		}
	}
	

	//	-- - 09/26/02 - AUTOVOX should always be defined
	userAE[kDefaultLocalAETitle] = 1;

#ifdef _DEBUG
	for (user_iter = userAE.begin(); user_iter != userAE.end(); user_iter++)
		printf("user AE: %s\n", user_iter->first.c_str());

	userAE["SHUTDOWN"] = 1;
#else
	for (user_iter = userAE.begin(); user_iter != userAE.end(); user_iter++)
		gLogger.LogMessage("user AE: %s\n", user_iter->first.c_str());

#endif
	userAE["CLEANUP_MEMORY"] = 1;
	userAE["RESET_TOOLKIT"] = 1;
	userAE["SYS_INFO"] = 1;
	userAE["AE_UPDATE"] = 1;
	userAE["WHL_UPDATE"] = 1;
	if(gConfig.m_enableMiniDumpTrigger)
		userAE["MINIDUMP"] = 1;
	
	//	Always respond to HOSTNAME_AE
//	char hostDefaultAE[17];
	char hostDefaultAE[20];
	//strncpy(hostDefaultAE, ToUpper(GetMyName()), sizeof hostDefaultAE);
	ASTRNCPY(hostDefaultAE, ToUpper(GetMyName()));
	STRNCAT_S(hostDefaultAE, "_AE", sizeof hostDefaultAE);

	//	-- - 2004.12.02 - truncate AE at 16 chars
	hostDefaultAE[16] = 0;

	userAE[hostDefaultAE] = 1;

	//	BEGIN: -- - 06/25/03 - Add default local AE's into database.  
	//	If they're already there, no insert happens, so no harm done
	UserGroup adminGroup;
	//int aeStatus = db.GetUserGroup("scan", adminGroup);
	int aeStatus = db.GetUserGroup(db.GetAdmUserGroupID(), adminGroup);
	if (aeStatus != kOK)
	{
		gLogger.LogMessage("Could not get group ID for scan group\n");
	} 
	else
	{
	 
		// Junnan Wu 05/19/04
		// We should also populate hostname_AE and AUTOVOX as the default local AE
		// if hostDefaultAE.m_AEName existing, try to update it so problem (such as IP address changed) will be gone
		// otherwise, try to add it 
		
		const char* loopbackIP = "127.0.0.1";
		// tc zhao 2004.04.22
		// the api for addlocalAE apparently is changed

		// tcz 2006.10.18  this takes care of the case where dicomserver starts
		// before the network subsystem is fully initialized. The best
		// way is to add the loopback first, then retry.
		const char* myIPAddress = TRPlatform::GetIPAddressString();
		int nretries = gConfig.m_numOfRetriesGettingIPAddress, cr = 0;
		while (cr < nretries && (!myIPAddress || strcmp(myIPAddress, loopbackIP) == 0))
		{
			Sleep(1000);
			myIPAddress = TRPlatform::GetIPAddressString();
			gLogger.LogMessage(kWarning,"IPAddress=%s\n", myIPAddress);
			++cr;
		}

		if (!myIPAddress  || strcmp(myIPAddress, loopbackIP) == 0)
		{
			gLogger.LogMessage("Could not get local IP Address. Using loopback %s\n",myIPAddress);
		}

#if 0 // #687 2010/04/20 K.Ko	
		ApplicationEntity autovoxAE;
		std::string tmpStr = "AUTOVOX_";
		tmpStr += GetMyName();
		ASTRNCPY(autovoxAE.m_AEName, tmpStr.c_str());
		ASTRNCPY(autovoxAE.m_AETitle, "AUTOVOX");
		ASTRNCPY(autovoxAE.m_hostName, GetMyName());
		ASTRNCPY(autovoxAE.m_IPAddress, TRPlatform::GetIPAddressString());
		ASTRNCPY(autovoxAE.m_description, "Default local AE");
		autovoxAE.m_priority = 3;
		autovoxAE.m_port = gConfig.m_port;
		autovoxAE.m_level = 1;
		aeStatus = db.InitDefaultLocalAE(autovoxAE);
		if (aeStatus != kOK)
		{
			gLogger.LogMessage("Failed to insert default local ae title AUTOVOX\n");
		}
#endif
	
		reset_locale_userAE_list[hostDefaultAE] = 1;	//#70 その他ローカルAE対応　2013/09/18
		ApplicationEntity hostAE;
		ASTRNCPY(hostAE.m_AEName, hostDefaultAE);
		ASTRNCPY(hostAE.m_AETitle, hostDefaultAE);
		ASTRNCPY(hostAE.m_hostName, GetMyName());
		ASTRNCPY(hostAE.m_IPAddress, TRPlatform::GetIPAddressString());
		ASTRNCPY(hostAE.m_description, "Default local AE");
		hostAE.m_port = gConfig.m_port;
		hostAE.m_priority = 3;
		hostAE.m_level = 1;
		aeStatus = db.InitDefaultLocalAE(hostAE);
		if (aeStatus != kOK)
		{
			gLogger.LogMessage("ERROR:[C%08d] Failed to insert default local ae title %s\n",DicomServError_InvalidDicomAE, hostDefaultAE);
		}

#if 1 // #687 2010/04/20 K.Ko
		// update AqNET_Local_00 as well

		//  AqNET_Local_00 
		//  use this AE Title for Anonymiz 
		//
		reset_locale_userAE_list["DICOM_Local_00"] = 1;	//#70 その他ローカルAE対応　2013/09/18
		ApplicationEntity importAE;
//		ASTRNCPY(importAE.m_AEName, "AqNET local import AE");
//		ASTRNCPY(importAE.m_AETitle, "AqNET_Local_00");
		ASTRNCPY(importAE.m_AEName, "DataServer local import AE");
		ASTRNCPY(importAE.m_AETitle, "DICOM_Local_00");
		ASTRNCPY(importAE.m_hostName, GetMyName());
		ASTRNCPY(importAE.m_IPAddress, TRPlatform::GetIPAddressString());
		ASTRNCPY(importAE.m_description, "Default local import AE title");
		importAE.m_port = gConfig.m_port;
		importAE.m_priority = 3;
		importAE.m_level = 1;
		aeStatus = db.InitDefaultLocalAE(importAE);
		if (aeStatus != kOK)
		{
			gLogger.LogMessage("ERROR:[C%08d] Failed to insert default local ae title %s\n",DicomServError_InvalidDicomAE, importAE.m_AETitle);
		}
#endif

		 
		if(gConfig.m_useDentalCT_AE !=0 ){
			 ApplicationEntity cnsl_importAE;
 
			reset_locale_userAE_list["DENTALCT_AE"] = 1;	//#70 その他ローカルAE対応　2013/09/18
			ASTRNCPY(cnsl_importAE.m_AEName, "DENTALCT_AE");
			ASTRNCPY(cnsl_importAE.m_AETitle, "DENTALCT_AE");
			ASTRNCPY(cnsl_importAE.m_hostName, GetMyName());
			ASTRNCPY(cnsl_importAE.m_IPAddress, TRPlatform::GetIPAddressString());
			ASTRNCPY(cnsl_importAE.m_description, "console local import AE title");
			cnsl_importAE.m_port = gConfig.m_port;
			cnsl_importAE.m_priority = 3;
			cnsl_importAE.m_level = 1;
			aeStatus = db.InitDefaultLocalAE(cnsl_importAE);
			if (aeStatus != kOK)
			{
				gLogger.LogMessage("ERROR:[C%08d] Failed to insert default console local import AE title %s\n",DicomServError_InvalidDicomAE, cnsl_importAE.m_AETitle);
			}
		}
		//
		if(gConfig.m_useJPEGGateway_AE !=0 ){//#70
			 ApplicationEntity jpeggateway_AE;
 
			reset_locale_userAE_list["JPEGGateway_AE"] = 1;	//#70 その他ローカルAE対応　2013/09/18
			ASTRNCPY(jpeggateway_AE.m_AEName, "JPEGGateway_AE");
			ASTRNCPY(jpeggateway_AE.m_AETitle, "JPEGGateway_AE");
			ASTRNCPY(jpeggateway_AE.m_hostName, GetMyName());
			ASTRNCPY(jpeggateway_AE.m_IPAddress, TRPlatform::GetIPAddressString());
			ASTRNCPY(jpeggateway_AE.m_description, "console local import AE title");
			jpeggateway_AE.m_port = gConfig.m_port;
			jpeggateway_AE.m_priority = 3;
			jpeggateway_AE.m_level = 1;
			aeStatus = db.InitDefaultLocalAE(jpeggateway_AE);
			if (aeStatus != kOK)
			{
				gLogger.LogMessage("ERROR:[C%08d] Failed to insert default console local import AE title %s\n",DicomServError_InvalidDicomAE, jpeggateway_AE.m_AETitle);
			}
		}

		if (isLocalBackup){//#93　2017/02/13 N.Furutsuki
			ApplicationEntity bakcup_AE;

			reset_locale_userAE_list["DcmBackup_AE"] = 1;	//#70 その他ローカルAE対応　2013/09/18
			ASTRNCPY(bakcup_AE.m_AEName, "DcmBackup_AE");
			ASTRNCPY(bakcup_AE.m_AETitle, "DcmBackup_AE");
			ASTRNCPY(bakcup_AE.m_hostName, GetMyName());
			gLogger.LogMessage("set-1 DcmBackup_AE: 127.0.0.1");
			ASTRNCPY(bakcup_AE.m_IPAddress, "127.0.0.1");// TRPlatform::GetIPAddressString());
			ASTRNCPY(bakcup_AE.m_description, "local backup AE title");
			bakcup_AE.m_port = gConfig.m_port;
			bakcup_AE.m_priority = 3;
			bakcup_AE.m_level = 1;
			aeStatus = db.InitDefaultLocalAE(bakcup_AE);
			if (aeStatus != kOK)
			{
				gLogger.LogMessage("ERROR:[C%08d] Failed to insert default console local import AE title %s\n", DicomServError_InvalidDicomAE, bakcup_AE.m_AETitle);
			}
			//#141 register AE : DcmBackup_AE
			userAE[bakcup_AE.m_AETitle] = 1;
		}

		///全てのローカルＡＥを再設定
		 
		{//#70 その他ローカルAE対応　2013/09/18
			AE_ApplicationID_Map::iterator it_temp;
			it_temp = locale_userAE_temp.begin();
			while(it_temp != locale_userAE_temp.end()){
				std::string ae_title = it_temp->first;
				if(reset_locale_userAE_list[ae_title] !=1){
					ApplicationEntity other_AE;
 
					 
					ASTRNCPY(other_AE.m_AEName, ae_title.c_str());
					ASTRNCPY(other_AE.m_AETitle, ae_title.c_str());
					ASTRNCPY(other_AE.m_hostName, GetMyName());
					if (isLocalBackup && strcmp(other_AE.m_AEName, "DcmBackup_AE") == 0){
						gLogger.LogMessage("set-2 DcmBackup_AE: 127.0.0.1 \n");
						ASTRNCPY(other_AE.m_IPAddress, "127.0.0.1");
					}
					else{
						ASTRNCPY(other_AE.m_IPAddress, TRPlatform::GetIPAddressString());
					}
					ASTRNCPY(other_AE.m_description, "console local import AE title");
					other_AE.m_port = gConfig.m_port;
					other_AE.m_priority = 3;
					other_AE.m_level = 1;
					aeStatus = db.InitDefaultLocalAE(other_AE);
					if (aeStatus != kOK)
					{
						gLogger.LogMessage("ERROR:[C%08d] Failed to insert default console local import AE title %s\n",DicomServError_InvalidDicomAE, other_AE.m_AETitle);
					}

				}
				 
				it_temp++;
			}
		}
		

 
	}
	//	END: 06/25/03 - Add default local AE's into database.  

	if(first) 
	{
		m_AEApplicationIDMap.clear();
	}

	MC_STATUS status;
	//	UnRegister AETitles that are no longer in AE Config map
	for (app_iter = m_AEApplicationIDMap.begin(); app_iter != m_AEApplicationIDMap.end();)
	{
		//	This AETitle is no longer allowed to listen
		if ((user_iter = userAE.find(app_iter->first)) == userAE.end())
		{
			if (app_iter->second > 0)
			{
				status = MC_Release_Application(&app_iter->second);
				if (status != MC_NORMAL_COMPLETION)
				{
					gLogger.LogMessage("ERROR:[C%08d]  Listener::OnAETitlesChanged - DcmLib error %d - failed to release applicationID %d for AE %s\n",DicomServError_DicomLibInitError, status, app_iter->second, app_iter->first);
					gLogger.FlushLog();
					++app_iter;
				}
				else
				{
					app_iter = m_AEApplicationIDMap.erase(app_iter);
				}
			}
		}
		else
		{
			++app_iter;
		}
	}
	
	int applicationID;
	//	Register AE Titles that are allowed to listen
	gLogger.LogMessage(kInfo, "--------- AE Title Assignments ---------\n");
	for (user_iter = userAE.begin(); user_iter != userAE.end(); user_iter++)
	{
		gLogger.LogMessage( "register AE: %s\n", user_iter->first.c_str());
		//	This AETitle is not yet registered, but should be
		if ((app_iter = m_AEApplicationIDMap.find(user_iter->first)) == m_AEApplicationIDMap.end())
		{
			gLogger.LogMessage( "register AE -> : %s\n", user_iter->first.c_str());
			status = MC_Register_Application(&applicationID, user_iter->first.c_str());
			if (status != MC_NORMAL_COMPLETION)
			{
				gLogger.LogMessage("ERROR:[C%08d] Listener::OnAETitlesChanged - DcmLib error %d - failed to register AE %s\n",DicomServError_DicomLibInitError, status, user_iter->first.c_str());
				gLogger.FlushLog();
			}

			m_AEApplicationIDMap[user_iter->first] = applicationID;
		}
	}

	if (isLocalBackup){//#93　2017/02/13 N.Furutsuki
						//#94
		setupLocalBackupAE();

	}

	gLogger.LogMessage(kInfo,  "------------------------------------\n");
	gLogger.FlushLog();

}

#include <Psapi.h>
// this one need Psapi.lib
//-----------------------------------------------------------------------------
//
DWORD ProcessWorkset()
{
	PROCESS_MEMORY_COUNTERS psmemCounters;
	if( !GetProcessMemoryInfo(GetCurrentProcess(),&psmemCounters, sizeof(psmemCounters)) )
		return 0;
	return psmemCounters.WorkingSetSize;
}

/*
DWORD CountMemUsage()
{
 MEMORY_BASIC_INFORMATION mbi;
 DWORD      dwMemUsed = 0;
 PVOID      pvAddress = 0;

 memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
 while(VirtualQuery(pvAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
    {
    if(mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE)
         dwMemUsed += mbi.RegionSize;
    pvAddress = ((BYTE*)mbi.BaseAddress) + mbi.RegionSize;
    } return dwMemUsed;
 }
*/

//-----------------------------------------------------------------------------
//
int	Listener::PreProcess(void)
{ 
	m_maxAssociations = gConfig.m_maxAssociations;
	m_maxThreads = gConfig.m_maxThreads;
	
	m_processStatus = kToBeStarted;
	// the first incoming association will make it 4 times big, so times 5 to avoid false MC release
	m_start_mem = ProcessWorkset()*5; 
	gLogger.LogMessage("DICOM start on mem*5=%d\n", m_start_mem);

	gLogger.LogMessage("postDicomResponse: %d \n",gConfig.m_postDicomResponse);//2010/03/16 k.ko #660
	
	 checkLicense();
	 //

	 


	return 0;

}

//-----------------------------------------------------------------------------
//
void Listener::CleanupMemory(void)
{
	MC_Cleanup_Memory(10);
}

//-----------------------------------------------------------------------------
//
void Listener::DoSysInfo(void)
{
	gLogger.LogMessage("SYS_INFO: Total threads=%d, Association threads=%d, Serial monitors=%d\n"
		,iRTVThread::Total, Threads(), RTVInactiveManager::theManager().Size());
	// ask all spawned threads to log status
	// the map access has no threads access control here, it may cause data corruption
	// However we can not put the threads control because we want to report status even thread
	// is in lock up state. This function is designed to report state in lock up situation.
	ThreadsMap::iterator iter;
	RTVAssociationHandler* pAssociationHandler;
	TRCSLock fplock(&m_threadMapCS);
	for (iter=m_threadMap.begin(); iter != m_threadMap.end(); ++iter)
	{
		pAssociationHandler = iter->first;
		pAssociationHandler->LogProcessStatus();
	}
	fplock.Unlock();
	gLogger.LogMessage("SYS_INFO: RTVInactiveManager m_state = %d\n", RTVInactiveManager::theManager().GetState());

	RTVOneShotThreadManager::MaxRunThreadsMap runningThreadsMap;
	RTVOneShotThreadManager::MaxRunThreadsMap::iterator oiter;
	int oneShotThreads = RTVOneShotThreadManager::theManager().GetRunningThreads(runningThreadsMap);
	gLogger.LogMessage("SYS_INFO: RTVOneShotThreadManager running threads = %d\n\n", oneShotThreads);
	for (oiter=runningThreadsMap.begin(); oiter != runningThreadsMap.end(); ++oiter)
	{
		gLogger.LogMessage("SYS_INFO: %s threads = %d\n", oiter->first.c_str(), oiter->second);
	}

	gLogger.LogMessage("\nSYS_INFO: Memory usage = %d\n", ProcessWorkset());

#ifdef _TRACE_MEMORY
	gLogger.LogMessage("SYS_INFO: open messages  = %d\n", MergeToolKit::GetOpenMessageCount());
	gLogger.LogMessage("SYS_INFO: open files     = %d\n", MergeToolKit::GetOpenFileCount());
	gLogger.LogMessage("SYS_INFO: open items     = %d\n", MergeToolKit::GetOpenItemCount());
	MergeToolKit::PrintMergeIDCounts();
#endif
	gLogger.FlushLog();
}

//-----------------------------------------------------------------------------
//
void Listener::ResetToolkit(void)
{
	DICOM_Release();
	if(!DICOM_Initialization())
	{
		gLogger.LogMessage("ERROR: Listener::Process() - DICOM_Initialization() failed\n");
		gLogger.FlushLog();
		RequestTermination(1);
		return;
	}
	
	gLogger.LogMessage("DICOM Memory after DICOM_Release = %d\n", ProcessWorkset());
	gLogger.FlushLog();
	
	OnAETitlesChanged(true); // intialize AE titiles	
}

//-----------------------------------------------------------------------------
//
void Listener::IdleProcess(void)
{
	// all accosiation finished and no pending working threads, 
	// force inactive monitor Manager to cleanup monitors
#if 0
	if(RTVInactiveManager::theManger().Size() != 0)
	{
		Sleep(1000);
		if(RTVInactiveManager::theManger().Size() != 0)
			RTVInactiveManager::theManger().Cleanup();
	}
#endif
#ifndef _TRACE_MEMORY
	// if no active associations and no outstanding serial monitors,
	// restart merge tool kit to regain leaked memory.
	if(RTVInactiveManager::theManager().Size() == 0)
	{
		int current_mem = ProcessWorkset();
		if(current_mem >= m_start_mem*1.587) // about the memory watermark go on
		{
			gLogger.LogMessage("DICOM clean up memory start at = %d\n", ProcessWorkset());
			gLogger.FlushLog();
			

			MC_STATUS status = MC_Cleanup_Memory(10); // there is memory leak in our side, don't use it yet
			if (status != MC_NORMAL_COMPLETION)
			{
				gLogger.LogMessage(kWarning,"WARNING: Listener::Process() - MC_Cleanup_Memory returned (%d,%s)\n", status, MC_Error_Message(status));
			}

			gLogger.LogMessage("DICOM clean up memory end with = %d\n", ProcessWorkset());
			gLogger.FlushLog();

/*			DICOM_Release();
			if(!DICOM_Initialization())
			{
				gLogger.LogMessage("ERROR: Listener::Process() - DICOM_Initialization() failed\n");
				gLogger.FlushLog();
				RequestTermination(1);
				return;
			}
			
			gLogger.LogMessage("DICOM Memory after DICOM_Release = %d\n", ProcessWorkset());
			gLogger.FlushLog();

			OnAETitlesChanged(true); // intialize AE titiles	
		*/

			m_start_mem = ProcessWorkset(); // adjust memory watermark
		}
		else if(current_mem < m_start_mem)
		{
			m_start_mem = current_mem;
		}
	}
#endif				

	//	Check log to see if it's time to rotate
	struct _stat statBuf;
	long fileSize = -1;
	if (_stat(gConfig.m_logFilename, &statBuf))
	{
		//	No point logging - we can't stat the log file.
		return;
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
			return  ;
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

	CheckMemory();//2012/03/27 K.Ko

//	-- - 1/21/03 - Move this to main listener loop
//	DoAutoClean();

}

//-----------------------------------------------------------------------------
//
void Listener::HandleAssociation(DiCOMConnectionInfo& connectInfo)
{
	AssociationHandler* pAssociationHandler = new AssociationHandler(connectInfo);
	//pAssociationHandler->Process();
	Handover(pAssociationHandler);

	pAssociationHandler->setupThreadPriority();// 2010/03/15 K.Ko

	
}

//-----------------------------------------------------------------------------
//
void Listener::DoAutoClean()
{
	//	Web scheduled task should do autoclean.  If that doesn't work, this one can be turned on.
	if (!gConfig.m_enableAutoClean)
		return;
#if 0
	int wakeupHour;
	AQNetConfiguration::GetAutoWakeupHour(wakeupHour);

	SYSTEMTIME now;
	static SYSTEMTIME timeAutoCleanLastRan;		//	So it only wakes up once per day

	//	The first time through, this should be set to yesterday at the wakeup time
	if (timeAutoCleanLastRan.wDay == 0)
	{
		GetLocalTime(&timeAutoCleanLastRan);
		timeAutoCleanLastRan.wDay -= 1;
		timeAutoCleanLastRan.wHour = wakeupHour;
	}

	GetLocalTime(&now);	
	RTVSystemTime currentTime(now);
	RTVSystemTime lastRanTime(timeAutoCleanLastRan);
	int daysSinceLastRan = currentTime - lastRanTime;

	if (daysSinceLastRan >= 1 && abs(now.wHour - wakeupHour) <= 1)
	{
		timeAutoCleanLastRan = now;
		int autoDeleteAge = 0;
		AQNetConfiguration::GetAutoDeleteAge(autoDeleteAge);
		if (autoDeleteAge < 1)
		{
			return;
		}

		gLogger.LogMessage(kInfo, "INFO: AssociationHandler::DoAutoClean() - Woke up - autoDeleteAge = %d\n", autoDeleteAge);

		AutoClean* cleaner = new AutoClean(autoDeleteAge, m_pLogger);
		RTVOneShotThreadManager::theManager().AddRequest(cleaner);
	}
#endif
}
bool dicom_server_checkLicense()  ;
 
bool Listener::checkLicense( ) // K.Ko 2010/05/21
{
	
	bool do_flag = true;

	SYSTEMTIME stime_cur;
	;	
	FILETIME	ftime_cur;
 

	if(m_firstCheckFlag )
	{ 
		do_flag = true;
		m_firstCheckFlag = false;

		GetLocalTime(&stime_cur);
		SystemTimeToFileTime(&stime_cur,&ftime_cur);
	}else{// interval
#if 1
		do_flag = false;  //起動時一回チェックのみ
#else
		GetLocalTime(&stime_cur);
		SystemTimeToFileTime(&stime_cur,&ftime_cur);

		int time_diff = (*((__int64*)&ftime_cur)-*((__int64*)&m_lastCheckLicenseTime) )/10000000.0 ;
		int check_interval = 60; //1 Minute
		if(m_checkedLicenseStatus){
			//normal 
			check_interval = 60*10; //5 Minute
		}
		if(time_diff  < check_interval){
			do_flag = false;
		}else{
			do_flag = true;
			
		}
#endif
	}
	//
	if(!do_flag){
		return m_checkedLicenseStatus;
	}else{
		m_lastCheckLicenseTime = ftime_cur;//for next interval
		m_checkedLicenseStatus = dicom_server_checkLicense();
	 
	}

	return m_checkedLicenseStatus;
}


void Listener::CheckMemory(void)  //2012/03/27 K.Ko
{
 
	bool dbg_chk_flag = false;
	if(dbg_chk_flag){
		IDcmLibApi::CheckMemory(1);
	}
 
}

//#93　2017/02/13 N.Furutsuki
bool Listener::setupLocalBackupAE()
{
	gLogger.LogMessage("Setup Remote AE for LocalBackup\n");
	gLogger.FlushLog();

	std::string local_QR_AE = TRDICOMUtil::CalculateLocalAETitle();

	std::string local_AE = TRDICOMUtil::CalculateInboundLocalAETitle();

	//////////////
	/// XXXX-AQNET
	ApplicationEntity QR_AE_temp;
	//
	strcpy(QR_AE_temp.m_AEName, local_QR_AE.c_str());
	strcpy(QR_AE_temp.m_AETitle, local_QR_AE.c_str());

	strcpy(QR_AE_temp.m_hostName, "localhost");
	strcpy(QR_AE_temp.m_IPAddress, "127.0.0.1");
	//
	QR_AE_temp.m_port = 104;
	QR_AE_temp.m_level = 1;
	QR_AE_temp.m_priority = 3;
	strcpy(QR_AE_temp.m_description, "Viewer Local AE");

	//////////////
	/// XXXX_AE
	ApplicationEntity Local_AE_temp;
	//
	strcpy(Local_AE_temp.m_AEName, local_AE.c_str());
	strcpy(Local_AE_temp.m_AETitle, local_AE.c_str());

	strcpy(Local_AE_temp.m_hostName, "localhost");
	strcpy(Local_AE_temp.m_IPAddress, "127.0.0.1");
	//
	Local_AE_temp.m_port = 105;
	Local_AE_temp.m_level = 1;
	Local_AE_temp.m_priority = 3;
	strcpy(Local_AE_temp.m_description, "Viewer Local AE");

	std::vector<int> iGroupIDs;
	

	CPxDcmDB db;

	std::vector<ApplicationEntity> QR_oVal;
	AqString whereFilter;
	whereFilter.Format(" WHERE AETitle= '%s'", local_QR_AE.c_str());

	db.QueryApplicationEntity(CPxDB::kRemoteAE, QR_oVal, whereFilter);
	if (QR_oVal.size() > 0){
		db.DeleteRemoteAE(local_QR_AE.c_str());
	}
	{
		db.AddRemoteAE(QR_AE_temp, iGroupIDs,
			1,//ae.m_CanQRFromMe,//iQRAllowed, 
			1,//ae.m_CanQR,//iQRSource, 
			1//ae.m_CanPushData//iStoreAE
			);
	}
	
	//
	std::vector<ApplicationEntity> AE_oVal;
	 
	whereFilter.Format(" WHERE AETitle= '%s'", local_AE.c_str());

	db.QueryApplicationEntity(CPxDB::kRemoteAE, AE_oVal, whereFilter);
	if (QR_oVal.size() > 0){
		db.DeleteRemoteAE(local_AE.c_str());
	}
	{
		db.AddRemoteAE(Local_AE_temp, iGroupIDs,
			0,//ae.m_CanQRFromMe,//iQRAllowed, 
			0,//ae.m_CanQR,//iQRSource, 
			1//ae.m_CanPushData//iStoreAE
			);
	}
	
	return true;
}