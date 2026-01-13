//////////////////////////////////////////////////////////////////////
// CMove.cpp
//
//	Copyright, PreXion 2011, All rights reserved.
//
//	PURPOSE:
//		Processes CMove Requests
//
//	
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxDicomutil.h"

#include "AqCore/TRPlatform.h"
#include "RTVSUtil.h"
#include "Globals.h"
#include "Compression.h"
#include "PxDicomMessage.h"
#include "PxDicomimage.h"
#include "CMove.h"
#include "Listener.h"


#include "AutoRoutingAEMan.h"  


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
#define M_ADD_SOP_CLASS2CMOVE(sop_cls) { \
	if(!TRDICOMUtil::addSOPClassUID((sop_cls),MOVE_SERVICE_NAME,true/*isPropose*/)) \
	{\
		gLogger.LogMessage("ERROR: CMove::initCMoveServiceList TRDICOMUtil::addSOPClassUID [%s] \n", (sop_cls));\
		gLogger.FlushLog();\
		return false;\
	}\
}
bool CMove::initCMoveServiceList()
{
	if(!TRDICOMUtil::initServiceList(MOVE_SERVICE_NAME,true/*isPropose*/))
	{
		gLogger.LogMessage("ERROR: CMove::initCMoveServiceList TRDICOMUtil::initServiceList [%s] \n", MOVE_SERVICE_NAME);
		gLogger.FlushLog();
		return false;
	}

	/*
	* CMove時にMove先に対して、Proposeする
	* 調整してください。
	*/
	//
	M_ADD_SOP_CLASS2CMOVE(UID_CTImageStorage);
	//
	M_ADD_SOP_CLASS2CMOVE(UID_XRayAngiographicImageStorage );
	M_ADD_SOP_CLASS2CMOVE(UID_XRayRadiofluoroscopicImageStorage );
	M_ADD_SOP_CLASS2CMOVE(UID_XRayRadiationDoseSRStorage );
	M_ADD_SOP_CLASS2CMOVE(UID_SecondaryCaptureImageStorage );
	M_ADD_SOP_CLASS2CMOVE(UID_FINDPatientRootQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CMOVE(UID_FINDStudyRootQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CMOVE(UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CMOVE(UID_MOVEPatientRootQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CMOVE(UID_MOVEStudyRootQueryRetrieveInformationModel );
 
	//
	M_ADD_SOP_CLASS2CMOVE(UID_DigitalIntraOralXRayImageStorageForPresentation);
	//
	//
	M_ADD_SOP_CLASS2CMOVE(UID_DigitalXRayImageStorageForPresentation);// #23 2012/07/07 K.KO for DX
	M_ADD_SOP_CLASS2CMOVE(UID_DigitalXRayImageStorageForProcessing);// #23 2012/07/07 K.KO for DX

	return true;
}
//--------------------------------------------------------------------
//
CMove::	CMove(DiCOMConnectionInfo& connectInfo, int iMessageID):
		RTVDiCOMService(connectInfo, iMessageID)
{

	m_remainingCStoreSubOperations = 0;
	m_successfulCStoreSubOperations = 0; 
 	m_failedCStoreSubOperations = 0; 
 	m_warnedCStoreSubOperations = 0; 
 	m_totalCStoreSubOperations = 0; 
	m_instanceCount = 0;
	m_db.InitDatabaseInfo();
	m_abort = false;
	m_currentCStoreSubOperationIndex = 0;
	m_studyLevelObjectID = 0;
	m_seriesLevelObjectID = 0;
    memset(m_targetAETitle, 0, sizeof m_targetAETitle);
	memset(m_targetIPAddress, 0, sizeof m_targetIPAddress);
	m_targetPort = 104;

	//	Initialize final C-MOVE response msg so it can be used to contain a list of all failed SOP instance UIDs
	MC_Open_Empty_Message(&m_finalCMoveRespMSG);

}

//--------------------------------------------------------------------
// 
CMove::~CMove()
{
	try{
		MC_Free_Message(&m_finalCMoveRespMSG);

		if (gConfig.m_decompressOnForceILE)
			m_ILEServiceMap.Clear();
	}catch(...)
	{
		LogMessage("ERROR:  CMove::~CMove exception \n");
		 
		;//sometime it's error
	}
}

//--------------------------------------------------------------------
//
int CMove::HandleFinalError(const char* iMsg, int iResponseCode)
{
	int status;

	LogMessage("ERROR:[C%08d] (%d) CMove failed - %s - sending final response status %d\n", 
		DicomServError_CMoveError,
		m_connectInfo.AssociationID, iMsg, iResponseCode);
	FlushLog();
	status = SendCMoveResponse(iResponseCode);
	if(status != MC_NORMAL_COMPLETION ) 
	{
		LogMessage("ERROR:(%d) CMove::Process() - Failed in HandleFinalError with (%d,%s)\n",m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
	}

	return status;
}

//--------------------------------------------------------------------
//
// Process a C-MOVE request. Only for one seriesInstanceUID or one SOP Instance UID now
// m_messageID object from main application need to be freed in main application.
// Will deal with mult. seriesInstanceUIDs or mult.SOP Instance UIDs  later.
//
int CMove::Process() 
{
	int status;

	int			associationID = m_connectInfo.AssociationID;

 
	int			msgID = m_messageID;			//	Inbound C-FIND-RQ message we are servicing
	status = MC_Get_Message_Service(msgID, &m_serviceName, &m_command);

 
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("ERROR: (%d) - CMove::Process() - DcmLib error (%d ) - Couldn't get Message Service\n", associationID, status   );
		return status;
	}

//	status = MC_Set_Service_Command(m_finalCMoveRespMSG, "STUDY_ROOT_QR_MOVE", C_MOVE_RSP);
	status = MC_Set_Service_Command(m_finalCMoveRespMSG, m_serviceName, C_MOVE_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - Failed on (%d,%s) to MC_Set_Service_Command() for serviceName = %s\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), m_serviceName);
		MC_Free_Message(&m_finalCMoveRespMSG);
		return status;  
	}

	status = ParseCMoveRQMessage();
	if (status != MC_NORMAL_COMPLETION) 
	{
		return status;
	}

	//	Set this now to the actual service name as parsed from the inbound request
	status = MC_Set_Service_Command(m_finalCMoveRespMSG, m_serviceName, C_MOVE_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - Failed on (%d,%s) to MC_Set_Service_Command() for serviceName = %s\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), m_serviceName);
		MC_Free_Message(&m_finalCMoveRespMSG);
		return status;  
	}

	status = GetSOPInstanceList();
	if (status != MC_NORMAL_COMPLETION) 
	{
		return status;
	}

	m_cStoreAssociationStatus = OpenCStoreAssociation();
	if(m_cStoreAssociationStatus != MC_NORMAL_COMPLETION) 
	{
		return m_cStoreAssociationStatus;
	}

	//	-- - 1/18/02: Used to simulate condition where DICOM server tries C-ECHO before sending C-STORE requests
	if (gConfig.m_requireCEchoBeforeCStoreRq)
	{
		status = CECHOProcess();
		if(status != MC_NORMAL_COMPLETION ) 
		{
			LogMessage("ERROR:[C%08d] (%d) CMove::Process() - Failed on (%d,%s) to do CECHOProcess().\n",DicomServError_CMoveError,m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
			FlushLog();
		}
	}

	for(int i = 0; i < m_seriesInstanceUIDs.size(); i++)
	{
		status = AuditLogSendSeries(m_seriesInstanceUIDs[i].c_str());
		if (status != kOK)
		{
			LogMessage("ERROR:[C%08d] CMove::Process() - Failed on AuditLogSendSeries for AETitle: %s and SeriesUID: %s\n",DicomServError_CMoveError, m_targetAETitle, m_seriesInstanceUIDs[i].c_str());
			FlushLog();
		}
	}

	//	Send all instances to the SCU
	SendInstances();

	if (m_abort)
	{
		if (m_cStoreAssociationID > 0)
			MC_Close_Association(&m_cStoreAssociationID);

		return MC_NORMAL_COMPLETION;
	}

	//	NOTE: actually, we don't know here if there were any failures or not - that will be determined inside the function
	//		  so this code is just so it knows this is a final response.
	SendCMoveResponse(C_MOVE_SUCCESS_NO_FAILURES);

	//	C-STORE association should be closed by now.  If not, close it.
	if (m_cStoreAssociationStatus != MC_ASSOCIATION_CLOSED)
	{
		MC_Close_Association(&m_cStoreAssociationID);
	}

	return  MC_NORMAL_COMPLETION;
}
//------------------------------------------------------------------------------------------------
std::string UIDFromFilename(const char* iFileName)
{
	char fileName[MAX_PATH];

	ASTRNCPY(fileName, iFileName);

	//	Format is Instance#_SOPInstanceUID.dcm
	//	Skip past '_' char
	char* uid = strchr(fileName, '_');
	if (!uid || !*uid)
		return "";

	++uid;
	
	//	Remove trailing ".dcm"
	char* dot = strrchr(uid, '.');
	if (!dot || !*dot)
		return "";

	*dot = 0;

	return uid;
}

//------------------------------------------------------------------------------------------------
//	This one does *not* uses db.  Instead, it assumes the file structure on disk
//	 
int	CMove::GetPathToInstanceList(std::vector<std::string>& oPaths, 
								std::vector<std::string>& oInstanceUIDs, 
								int iLevel)
{
	//!!! must change to have series UID
	int inStudyUIDs  = m_studyInstanceUIDs.size();
	int inSeriesUIDs = m_seriesInstanceUIDs.size();
	int inSOPUIDs    = m_SOPInstanceUIDs.size();
	std::string seriesUID, studyUID;

	if(gConfig.m_safetyCheck !=0 ){
	//#27 2012/06/14 K.Ko
		m_curStudy.clear();
		if(m_studyInstanceUIDs.size()>0){
			DICOMData tmpFilter;
			ASTRNCPY(tmpFilter.m_studyInstanceUID, m_studyInstanceUIDs[0].c_str());
		//	std::vector<DICOMStudy>  oVal;
			
			m_db.GetStudyList( m_curStudy, &tmpFilter);
		}

	}
	switch(iLevel)
	{
		case kStudyLevel:
			if (inStudyUIDs < 1)
			{
				GetAqLogger()->LogMessage("ERROR: AQDB::GetPathToInstanceList() - STUDY level must include at least one StudyInstanceUID\n");
				return kParameterError;
			}
			break;
		case kSeriesLevel:
			if (inStudyUIDs != 1)
			{
				GetAqLogger()->LogMessage("ERROR: AQDB::GetPathToInstanceList() - SERIES level must include a StudyInstanceUID\n");
				return kParameterError;
			}
			break;
		case kInstanceLevel:
			if (inStudyUIDs != 1 || inSeriesUIDs != 1)
			{
				GetAqLogger()->LogMessage("ERROR: AQDB::GetPathToInstanceList() - INSTANCE level must include a StudyInstanceUID and SeriesInstanceUID\n");
				return kParameterError;
			}
			break;
	};

	std::vector<std::string> seriesList;
	std::string seriesPath;
	std::vector<TRFileName> instanceList;
	char fileName[MAX_PATH];

	int nStudyUIDs  = m_studyInstanceUIDs.size();
	int nSeriesUIDs = 0;
	int nSOPUIDs    = 0;

	for(int i=0; i<nStudyUIDs; i++)
	{
		studyUID = m_studyInstanceUIDs[i];

		m_db.GetSeries(m_studyInstanceUIDs[i].c_str(), seriesList);
		nSeriesUIDs = seriesList.size();
		std::string uid;
		
		std::map<std::string, int> seriesUIDmap;
		for ( int sm = m_seriesInstanceUIDs.size(); --sm>=0;)
			seriesUIDmap[m_seriesInstanceUIDs[sm]] = 1;

		for(int j=0; j<nSeriesUIDs; j++)
		{
			if (iLevel == kSeriesLevel)
			{
				if (seriesUIDmap.find(seriesList[j]) == seriesUIDmap.end())
				{
					continue;
				}
			}

			seriesUID = seriesList[j];
			m_db.GetSeriesPath(m_studyInstanceUIDs[i].c_str(), seriesList[j].c_str(), seriesPath);
			if (seriesPath.length() < 3)
			{
				GetAqLogger()->LogMessage("ERROR: AQDB::GetPathToInstanceList() - GetSeriesPath failed - returned: %s for series = %s, study = %s\n",
					seriesPath.c_str(), seriesUID.c_str(), studyUID.c_str());
				return kNoResult;
			}

			TRPlatform::iGetDirectoryList(seriesPath.c_str(), "*.dcm", instanceList);
			if (instanceList.size() < 1)
				return kNoResult;

			std::map<std::string,int> sopMap;
			if (iLevel == kInstanceLevel)
			{
				for ( int N = m_SOPInstanceUIDs.size(); --N>=0;)
					sopMap[m_SOPInstanceUIDs[N]] = 1;
			}

			for(int k = 0; k < instanceList.size(); k++)
			{
				strncpy(fileName, instanceList[k].GetName(), sizeof fileName);
				fileName[sizeof(fileName)-1] = 0;

				uid = UIDFromFilename(fileName);

				//	Is it one of the ones that was asked for?
				if (iLevel == kInstanceLevel && sopMap.find(uid) == sopMap.end())
					continue;

				//	It is - so give it back
				oInstanceUIDs.push_back(uid);

				std::string fullPath = seriesPath + "\\" + instanceList[k].GetName();
				oPaths.push_back(fullPath);
			}
		}
	}

	return kOK;
}

//--------------------------------------------------------------------
// Get the SeriesPath And FileNames contained in CDBdate.
//  
int CMove::GetSOPInstanceList()
{
	DICOMData	 iKey;
	int 		 status;
	
	//	-- - 10/31/02 - Get the paths instead of just the instanceUID's - avoid db lookups in send loop

	status = GetPathToInstanceList(m_pathV, m_instanceV, m_level);
	if (status != 0)
	{
		return HandleFinalError("GetSOPInstanceList - GetPathToInstanceList() failed", C_MOVE_FAILURE_UNABLE_TO_PROCESS);
	}

	if ((m_totalCStoreSubOperations = m_instanceV.size()) < 1)
	{
		return HandleFinalError("GetSOPInstanceList - GetPathToInstanceList() returned 0 results", C_MOVE_FAILURE_UNABLE_TO_PROCESS);
	}

	return MC_NORMAL_COMPLETION;
}

//#define LCOAL_PROFILE	1

//--------------------------------------------------------------------
//  
unsigned int CMove::TransferSyntaxToMask(int iTransferSyntax)
{
	unsigned int index = iTransferSyntax - IMPLICIT_LITTLE_ENDIAN;
	return 1 << index;
}

//--------------------------------------------------------------------
//  
int CMove::IsAccepted(unsigned int iMask, int iTransferSyntax)
{
	return (iMask & TransferSyntaxToMask(iTransferSyntax));
}

//--------------------------------------------------------------------
// Send the instances to the SCU
//  
void CMove::SendInstances()
{
	int		status, rcode;
	int		cMoveRespStatus;
	unsigned int	cStoreResp;
	std::string		sopUID;
	std::string		nameAndPath;

try{ // #11  2012/03/23 K.Ko

	//	Set up some members so we can remember how many operations failed, etc
	m_instanceCount = m_totalCStoreSubOperations;
	m_remainingCStoreSubOperations = m_totalCStoreSubOperations;
	m_successfulCStoreSubOperations = 0; 
 	m_failedCStoreSubOperations = 0; 
 	m_warnedCStoreSubOperations = 0; 
	m_failedSOPListCount = 0;

	int messageID = -1;
	MessageFreeGuard guard(messageID);
	int ntries = 0;
#ifdef LCOAL_PROFILE
	LARGE_INTEGER CountsPerSecond, cStart, tcounter1, tcounter2, tcounter;
	QueryPerformanceFrequency(&CountsPerSecond); 
	QueryPerformanceCounter(&cStart); 
	tcounter.QuadPart = 0;
	long double elapsedMilliseconds;
#endif
	int cTime, checkTime = time(0);

	//	For testing only
	if (gConfig.m_hangCMove)
	{
		while(!Listener::theListener().TerminationRequested())
			Sleep(1000);
		
		m_abort = 1;
	}

	SendCMoveResponse(C_MOVE_PENDING_MORE_SUB_OPERATIONS);

typedef std::map<std::string ,int > LOGStatusMap;
typedef std::pair<std::string ,int> theLOGStatusPair;

LOGStatusMap _logStatusMap;

	while (!m_abort && m_instanceCount>0)
	{
		m_currentCStoreSubOperationIndex = m_totalCStoreSubOperations - m_instanceCount;
		//	Check for cancel
		cTime = time(0);
		if(cTime-checkTime>1)
		{
			status = CheckCMoveCancellation(m_currentCStoreSubOperationIndex, cMoveRespStatus);
			if (cMoveRespStatus == C_MOVE_CANCEL_REQUEST_RECEIVED || TRDICOMUtil::AssociationIsDead((MC_STATUS) status))
			{
				m_abort = true;
				return;
			}
			checkTime = cTime;
		}

		if(messageID < 0)
		{
			messageID = BuildSendMessage(m_pathV[m_currentCStoreSubOperationIndex].c_str());
			//	Which instance are we going to send in this iteration of the loop?
			sopUID.assign(m_instanceV[m_currentCStoreSubOperationIndex].c_str());
		}
		
		if(messageID < 0)
		{
			HandleFailedInstance(sopUID);
			continue;
		}

#ifdef LCOAL_PROFILE
		QueryPerformanceCounter(&tcounter1); 
#endif

		// Send it off 
		ntries = 0;
		do
		{
			if (gConfig.m_decompressOnForceILE)
			{
				CPxDicomImage* pImage = 0;
				pImage = new CPxDicomImage;
				DecompressPixelsIfNeedTo(messageID, pImage);
				pImage->ReleaseImagePixels();
				pImage->HandoverID();
				delete pImage, pImage = 0;
			}

			if(gConfig.m_traceDicomLevel >=2){ //#693
				char studyIDTemp[128];
				studyIDTemp[0] = 0;//2017/01/12
				 MC_Get_Value_To_String(messageID, 
                                                MC_ATT_STUDY_ID,
                                                sizeof(studyIDTemp),
                                                studyIDTemp);
				 char patineIDTemp[128];
				 MC_Get_Value_To_String(messageID, 
                                                MC_ATT_PATIENT_ID,
                                                sizeof(patineIDTemp),
                                                patineIDTemp);
 
				 char patineNameTemp[128];
				 patineNameTemp[0] = 0;//2017/01/12
#if 0
				 MC_Get_Value_To_String(messageID, 
                                                MC_ATT_PATIENTS_NAME,
                                                sizeof(patineNameTemp),
                                                patineNameTemp);
#else
				 //患者名のカタカナのある場合
				 {
					 CPxDicomImage dicomImg_temp;
					dicomImg_temp.GetValue(messageID,MC_ATT_PATIENTS_NAME,patineNameTemp,sizeof(patineNameTemp));
				}
#endif
				 
				 char sereisUID[128];
				 sereisUID[0] = 0;//2017/01/12
				 MC_Get_Value_To_String(messageID, 
                                                MC_ATT_SERIES_INSTANCE_UID,
                                                sizeof(sereisUID),
                                                sereisUID);
				 std::string LogSereisUID = sereisUID;

				 LOGStatusMap::iterator logoutIterator = _logStatusMap.find (LogSereisUID);
				 if (logoutIterator == _logStatusMap.end ()) {
				// just do once for same studyID
					 _logStatusMap.insert(theLOGStatusPair(LogSereisUID,1));
					 //
					 std::string str_temp;
					 AutoRoutingAEMan::ReformatJapaneseDicom(patineNameTemp, str_temp);

					LogMessage("INFO:[C%08d]  SendSeries to AE: %s , PatientName:%s, id:%s, StudypID:%s, SeriesUID:%s \n",
						DicomServInfor_CMoveInfo, m_targetAETitle, 
						str_temp.c_str(),//patineNameTemp,
						patineIDTemp,studyIDTemp,sereisUID);
					FlushLog();
				 }
	 


			}
			status = MC_Send_Request_Message(m_cStoreAssociationID, messageID);
			if(status == kTimeout)
			{
				rcode = CheckCMoveCancellation(m_currentCStoreSubOperationIndex, cMoveRespStatus);
				if (cMoveRespStatus == C_MOVE_CANCEL_REQUEST_RECEIVED || TRDICOMUtil::AssociationIsDead(rcode))
				{
					m_abort = true;
					return;
				}
			}
			//   log the failures. Could potentially generate one entry
			// per failed SOP.
			else if (status != kNormalCompletion)
			{
				LogMessage("ERROR:(%d) CMove::SendInstance failed. Status=%s. SOP=%s\n",m_cStoreAssociationID, MC_Error_Message((MC_STATUS)status),sopUID.c_str());
			}
		} while (status == kTimeout && ntries++ < gConfig.m_numberOfRetries );
		
#ifdef LCOAL_PROFILE
		QueryPerformanceCounter(&tcounter2); 
		tcounter.QuadPart += (tcounter2.QuadPart - tcounter1.QuadPart);
#endif

		// read in disk file before get responce
		MC_Free_Message(&messageID);
		if(m_instanceCount > 1 )
			messageID = BuildSendMessage(m_pathV[m_currentCStoreSubOperationIndex+1].c_str());
		else
			messageID = -1;

		if (status != MC_NORMAL_COMPLETION) 
		{
		 	if(status == MC_ASSOCIATION_ABORTED || status == MC_ASSOCIATION_CLOSED ||  status == MC_INVALID_ASSOC_ID)
			{
				m_cStoreAssociationStatus = MC_ASSOCIATION_CLOSED;
			}
			//	Which instance are we going to send in this iteration of the loop?
			sopUID.assign(m_instanceV[m_currentCStoreSubOperationIndex].c_str());
			HandleFailedInstance(sopUID);
			continue;
		}

		// after read disk file and send more pending send, CStore responce should arrive
		// no need to do CheckCMoveCancellation, because ReceiveAndParseCStoreRspMsg will do it
		//	Read the SCU's C-STORE-RSP message
#ifdef LCOAL_PROFILE_REP
		QueryPerformanceCounter(&tcounter1); 

		status = ReceiveAndParseCStoreRspMsg(cStoreResp);
		
		QueryPerformanceCounter(&tcounter2); 
		tcounter.QuadPart += (tcounter2.QuadPart - tcounter1.QuadPart);
#else
		status = ReceiveAndParseCStoreRspMsg(cStoreResp);
#endif

		//	We didn't get a valid response - consider this one a failure and move on
		if (status != MC_NORMAL_COMPLETION) 
		{
			if (TRDICOMUtil::AssociationIsDead((MC_STATUS) status))
			{
				m_abort = true;
				return;
			}
			sopUID.assign(m_instanceV[m_currentCStoreSubOperationIndex].c_str());
			HandleFailedInstance(sopUID);
			continue;
		}
	 
  		m_remainingCStoreSubOperations--;
		m_instanceCount--;
		
		//	What was the response from the SCU?
		if (cStoreResp == C_STORE_SUCCESS)
		{
			m_successfulCStoreSubOperations++;
		}
		else
		{
			if(cStoreResp == C_STORE_WARNING_ELEMENT_COERCION ||  
			   cStoreResp == C_STORE_WARNING_INVALID_DATASET || 
			   cStoreResp == C_STORE_WARNING_ELEMENTS_DISCARDED)
			{
				m_warnedCStoreSubOperations++;
			}
			else
			{
				m_failedCStoreSubOperations++;

				//	-- - 06/02/03 - Moved from below - do not count warnings as failures
				AddToFailedSOPList((char*) m_instanceV[m_currentCStoreSubOperationIndex].c_str());
			}

			//  AddToFailedSOPList((char*) m_instanceV[m_currentCStoreSubOperationIndex].c_str());
		}

		// send more pening send before get responce
		if(m_instanceCount > 0)
			SendCMoveResponse(C_MOVE_PENDING_MORE_SUB_OPERATIONS);
		
	}
#ifdef LCOAL_PROFILE
		elapsedMilliseconds = (long double)(tcounter.QuadPart * 1000) / (long double)CountsPerSecond.QuadPart;
		printf("\n!!! message spent: %.2lf\n", elapsedMilliseconds);
		QueryPerformanceCounter(&tcounter2); 
		tcounter.QuadPart = (tcounter2.QuadPart - cStart.QuadPart);
		elapsedMilliseconds = (long double)(tcounter.QuadPart * 1000) / (long double)CountsPerSecond.QuadPart;
		printf("\n!!! total spent: %.2lf\n", elapsedMilliseconds);
#endif

}catch(...) // #11  2012/03/23 K.Ko
{
	 
	LogMessage("ERROR:[C%08d]  : [Exception] CMove::SendInstances \n", 
			DicomServError_Exception);
	FlushLog();
 
}

}

//--------------------------------------------------------------------
//
int CMove::DecompressPixelsIfNeedTo(int iMessageID, CPxDicomImage* pImage)
{
	int status = MC_NORMAL_COMPLETION;

	char SOPClassUID[65];
    status = MC_Get_Value_To_String(iMessageID, MC_ATT_SOP_CLASS_UID, sizeof(SOPClassUID), SOPClassUID);
    if (status != MC_NORMAL_COMPLETION)
    {  
        return status;
    }
       
	//	What is the MergeCOM service name for this SOP Class UID?
	char serviceName[65];
    status = MC_Get_MergeCOM_Service(SOPClassUID, serviceName, sizeof(serviceName));
    if (status != MC_NORMAL_COMPLETION)
    {
        return status;
    }

	//	ASSERT: The other guy will only take ILE for this service.
	//		Let's see if we already have ILE.
	TRANSFER_SYNTAX storedSyntax = INVALID_TRANSFER_SYNTAX;
	status = MC_Get_Message_Transfer_Syntax(iMessageID, &storedSyntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	//	What's on disk will go fine - don't mess with it
	int acceptedTransferSyntaxes = m_ILEServiceMap.Get(serviceName);
	if (IsAccepted(acceptedTransferSyntaxes, storedSyntax))
		return MC_NORMAL_COMPLETION;

	bool needToDecompress = Compression::IsCompressed(storedSyntax);
	if (!needToDecompress || 
		(Compression::IsLossyCompressed(storedSyntax) && !gConfig.m_alsoDecompressLossyOnForceILE))
	{
		//	Don't need to decompress - so leave it alone; nothing to do
		return MC_NORMAL_COMPLETION;
	}

	//	ASSERT: we need to decompress
	pImage->PopulateFromMessage(iMessageID);
	status = (MC_STATUS) pImage->GetStatus();
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;  			
	}

	int bitsStored = pImage->GetBitsStored();
	int bytesPerPixel = (bitsStored+7)/8;
	int outBytes = pImage->GetNumberOfBytesOfPixelData();

	if (outBytes == 0 || pImage->GetNumberOfFrames() > 1)
		return status;

	Compression compr;
	unsigned char* pixels = 0;
	uint8* frameBuffer = 0;
	int comprStat;

	//	We don't have them; get them from the Image object
	pixels = pImage->GetImagePixels();
	unsigned long pixelDataSize = pImage->GetPixelDataSize();
	if (!pixels || !pixelDataSize)
	{
		return MC_SYSTEM_ERROR;
	}

	compr.SetUseStandardCodecs(gConfig.m_useStandardCodecs);
	comprStat = compr.SetPixels(pImage, pixels, pixelDataSize, outBytes, pImage->GetBitsAllocated(), storedSyntax, pImage->GetSamplesPerPixel());
	if (comprStat != kSuccess)
	{
		if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		return MC_COMPRESSION_FAILURE;
	}

	comprStat = compr.DecodeNextFrame(frameBuffer);
	if (comprStat != kSuccess || !frameBuffer)
	{
		if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		return MC_COMPRESSION_FAILURE;
	} 

	//	Replace existing buffer with transcoded buffer
	status = (MC_STATUS) pImage->SetValue(kVLIPixelData, (unsigned char*) frameBuffer, outBytes);
	if (status != MC_NORMAL_COMPLETION)
	{
		if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		return status;
	}

	status = MC_Set_Message_Transfer_Syntax(iMessageID, IMPLICIT_LITTLE_ENDIAN);
	if (status != MC_NORMAL_COMPLETION)
	{
		if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		return status;
	}

	if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;

	//	Set Lossy Compressed flag
	if (Compression::IsLossyCompressed(storedSyntax))
	{
		pImage->SetValue(kVLILossyImageCompression,	"01");
	}

	return MC_NORMAL_COMPLETION;
}

//--------------------------------------------------------------------
//
void CMove::HandleFailedInstance(std::string iSopUID)
{
	m_remainingCStoreSubOperations--;
	m_failedCStoreSubOperations++;
	m_instanceCount--; 

	// Add to failed SOP Instance UID list
	AddToFailedSOPList((char*) iSopUID.c_str());
	SendCMoveResponse(C_MOVE_PENDING_MORE_SUB_OPERATIONS);
}

//--------------------------------------------------------------------
//
int CMove::ReceiveAndParseCStoreRspMsg(unsigned int& oCStoreResponse)
{
	int		status;
	int				cStoreResponseMessageID;
 	unsigned int	cStoreResponse;
	char*			cStoreServiceName;

	int ntries = 0;
	int cMoveRespStatus;
	int readTimeout = 1;

try{ // #11  2012/03/23 K.Ko

	do
	{
		status = MC_Read_Message(m_cStoreAssociationID, readTimeout, &cStoreResponseMessageID, &cStoreServiceName, &m_command);
		if(status == kTimeout)
		{
			status = CheckCMoveCancellation(m_currentCStoreSubOperationIndex, cMoveRespStatus);
			if (cMoveRespStatus == C_MOVE_CANCEL_REQUEST_RECEIVED || TRDICOMUtil::AssociationIsDead((MC_STATUS) status))
			{
				m_abort = true;
				return status;
			}
		}
		readTimeout += 2;
	} while (ntries++ < gConfig.m_numberOfRetries && status == kTimeout);
	
	//	Failure to read message
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - ReceiveAndParseCStoreRspMsg() - Failed on (%d,%s) to MC_Read_Message() for cStoreAssociationID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), m_cStoreAssociationID);
        return status; 
    }

	MessageFreeGuard guard(cStoreResponseMessageID);

	if (gConfig.m_messageDump)
	{
		TRDICOMUtil::DumpMessage(cStoreResponseMessageID, cStoreServiceName, m_command);
	}

	//	What was the response code?
    status = MC_Get_Value_To_UInt(cStoreResponseMessageID, MC_ATT_STATUS, &cStoreResponse);
    if (status != MC_NORMAL_COMPLETION)
    {
        LogMessage("ERROR:(%d) CMove::Process() - ReceiveAndParseCStoreRspMsg() - Failed on (%d,%s) to get CStore Response in MC_Get_Value_To_UInt() for cStoreResponseMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), cStoreResponseMessageID);
		return status;  
    }

	//	The response code gets checked by the caller
	oCStoreResponse = cStoreResponse;
	
}catch(...) // #11  2012/03/23 K.Ko
{
	 
	LogMessage("ERROR:[C%08d]  : [Exception] CMove::ReceiveAndParseCStoreRspMsg \n", 
			DicomServError_Exception);
	FlushLog();
	status = MC_ERROR;
}
	return status;  
}

//--------------------------------------------------------------------
//    
int CMove::CheckCMoveCancellation(int iSOPInstanceIndex, int &oRespStatus)
{
	int	status;
	int			cMoveCancelMessageID = -1;
   	char*		serviceName;
	MC_COMMAND	command;
	int			m_instanceCount = -1;
	
	oRespStatus = 0;
	MessageFreeGuard guard(cMoveCancelMessageID);
	//	Read message from C_MOVE  RQ 
	status = MC_Read_Message (m_connectInfo.AssociationID, 0, &cMoveCancelMessageID, &serviceName, &command);
	if (status == MC_TIMEOUT) 
	{
		return MC_TIMEOUT;  
	}
	else if (status != MC_NORMAL_COMPLETION) 
	{
		LogMessage("ERROR:(%d) CMove::Process() - CheckCMoveCancellation() - C_MOVE_CANCELLATION check failed (%d,%s) on MC_Read_Message() for cMoveAssociationID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), m_connectInfo.AssociationID);
		oRespStatus = C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
        return status;  
    }


	if (gConfig.m_messageDump)
	{
		TRDICOMUtil::DumpMessage(cMoveCancelMessageID, serviceName, command);
	}

	//	We received a message.  Was it a cancel?
	if (command != C_CANCEL_MOVE_RQ)
	{
		return MC_NORMAL_COMPLETION;  
	}

	m_abort = true;
	LogMessage(kDebug, "DEBUG:(%d) CMove::Process() - CheckCMoveCancellation() - Received a C_CANCEL_MOVE_RQ\n", m_connectInfo.AssociationID);
	oRespStatus = C_MOVE_CANCEL_REQUEST_RECEIVED;

	for( m_instanceCount = iSOPInstanceIndex; m_instanceCount<m_totalCStoreSubOperations;m_instanceCount++)
	{
		AddToFailedSOPList((char*) m_instanceV[m_instanceCount].c_str());
	}

	SendCMoveResponse(C_MOVE_CANCEL_REQUEST_RECEIVED);

	return status;  
}

//--------------------------------------------------------------------
// Estabish an association to destination AETitle for C-STORE sub-operations 
//
int CMove::OpenCStoreAssociation()
{
	int cMoveRespStatus;
	int status;
try{ // #11  2012/03/23 K.Ko
	
	/* 
	 *  Added code to detect local request so we don't need to have
	 *  the local host in the application entity table 
	 */

	// local request port is configurable in image server.  If no entry is in the db, then use the default
	int localRequest = !strcmp(TRPlatform::GetIPAddressString(), m_connectInfo.RemoteIPAddress);
	ASTRNCPY(m_targetIPAddress, m_connectInfo.RemoteIPAddress);
	
	ApplicationEntity AEInfo;
	ASTRNCPY(AEInfo.m_AETitle,   m_connectInfo.RemoteApplicationTitle);
	ASTRNCPY(AEInfo.m_IPAddress, m_connectInfo.RemoteIPAddress);
	AEInfo.m_port =				 m_targetPort;

	//	Not checking the port for now, because it creates a dependancy
	//		with the listening port setting in the config file
	//
	//		If localRequest, don't need to check IsRetrieveAE()
	if(!localRequest && !m_db.IsRetrieveAE(AEInfo))
	{
		char msg[128];
		sprintf(msg, "OpenCStoreAssociation - AE %s not allowed to retrieve", m_connectInfo.RemoteApplicationTitle);
		 HandleFinalError(msg, C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN);
		return C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN;
	}

	//	Local requests *do* need to look up in table.  If it's there, use that
	//		port (imageServer must have configured to a different port (NetStation, for example).
	//		Otherwise, just use the default (104).
	if(m_db.IsStoreTargetAE(m_targetAETitle, AEInfo))
	{
		ASTRNCPY(m_targetIPAddress, AEInfo.m_IPAddress);
		m_targetPort = AEInfo.m_port;
	}
	else if( !localRequest )
	{
		char msg[128];
		sprintf(msg, "OpenCStoreAssociation - Failed to get remote AE information for %s", m_targetAETitle);
		 HandleFinalError(msg, C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN);
		 return C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN;
	}


	LogMessage(kDebug,"C-STORE SCP: AE=%s IP=%s port=%d\n", AEInfo.m_AETitle,AEInfo.m_IPAddress,AEInfo.m_port);
	FlushLog();
#ifdef _DEBUG	
	printf("AETitle  = %s\n", m_targetAETitle);
	printf("Port     = %d\n", m_targetPort);
#endif	

	//	Open the C-STORE Association
 	int ntries = 0;
	do
	{
		status = CheckCMoveCancellation(m_currentCStoreSubOperationIndex, cMoveRespStatus);
		if (cMoveRespStatus == C_MOVE_CANCEL_REQUEST_RECEIVED || TRDICOMUtil::AssociationIsDead((MC_STATUS) status))
		{
			m_abort = true;
			return status;
		}

		status = MC_Open_Association(m_connectInfo.ApplicationID, &m_cStoreAssociationID, m_targetAETitle, &m_targetPort,
	//								 m_targetIPAddress, "TIDICOM_SCU_Service_List");	
									m_targetIPAddress, MOVE_SERVICE_NAME);
		
	} while (ntries++ < gConfig.m_numberOfRetries && status == kTimeout);

	// 
	// Log all related info so we can spot configuration errors
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("ERROR:[C%08d] (%d): OpenCStoreAssociation(%s,%s,%d) failed. Status=%s\n",
			DicomServError_CMoveError,
			m_connectInfo.AssociationID,
			m_targetAETitle, m_targetIPAddress, m_targetPort, MC_Error_Message((MC_STATUS)status));
	}
	// End of modification

    if (status == MC_ASSOCIATION_REJECTED || status == MC_INVALID_APPLICATION_TITLE || status == MC_INVALID_SERVICE_LIST_NAME ||
		status == MC_INVALID_HOST_NAME || status == MC_CONFIG_INFO_MISSING || status == MC_CONFIG_INFO_ERROR || status == MC_INVALID_PORT_NUMBER)
    {
		  HandleFinalError("OpenCStoreAssociation() - Failed to open C_STORE Associationon", C_MOVE_FAILURE_REFUSED_CANT_PERFORM);
		return status; //2010/04/30 K.Ko
    }
    else if (status != MC_NORMAL_COMPLETION)
    {
		 HandleFinalError("OpenCStoreAssociation() - Failed to open C_STORE Associationon", C_MOVE_FAILURE_REFUSED_CANT_PERFORM);
		return status;
	}

	if (gConfig.m_decompressOnForceILE)
	{
		ServiceInfo info;
		bool overwrite = true;
		status = MC_Get_First_Acceptable_Service(m_cStoreAssociationID, &info);
		if (status == MC_NORMAL_COMPLETION)
		{
			//	Keep going until we get a status of MC_END_OF_LIST or an error
			for(;status == MC_NORMAL_COMPLETION;)
			{
				unsigned int acceptedTransferSyntaxes = 0;
				if (m_ILEServiceMap.Has(info.ServiceName))
				{
					acceptedTransferSyntaxes = m_ILEServiceMap.Get(info.ServiceName);
				}

				acceptedTransferSyntaxes |= TransferSyntaxToMask(info.SyntaxType);
				m_ILEServiceMap.Add(info.ServiceName, acceptedTransferSyntaxes, overwrite);
				status = MC_Get_Next_Acceptable_Service(m_cStoreAssociationID, &info);
			}
		}
	}

	int t = 1;
	AssocInfo ai;
	status = MC_Get_Association_Info(m_cStoreAssociationID, &ai);
	setsockopt(ai.Tcp_socket,IPPROTO_TCP, TCP_NODELAY, (const char*)&t, sizeof(t));
}catch(...) // #11  2012/03/23 K.Ko
{
	 
	LogMessage("ERROR:[C%08d]  : [Exception] CMove::OpenCStoreAssociation \n", 
			DicomServError_Exception);
	FlushLog();
	return MC_ERROR;
}

	return MC_NORMAL_COMPLETION;  
}

//--------------------------------------------------------------------
// m_messageID will not be freed here, It will be freed in main application.
// Get the destination AE title Retrieve root, 
// level and instance UID based on different level.
//
//	Setup m_filter	
//
int CMove::ParseCMoveRQMessage()
{
	int status;
    char retrieveLevelStr[kVR_LO];

	// Get the destination AE title so we know where to send the C-STORE messages
	status = MC_Get_Value_To_String (m_messageID, MC_ATT_MOVE_DESTINATION, sizeof(m_targetAETitle), m_targetAETitle);
    if (status != MC_NORMAL_COMPLETION)
    {
		return HandleFinalError("ParseCMoveRQMessage - Missing MoveDestination", C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN); 
    }	

	//	2005.02.17 - -- - Remove white space to fix Q/R problem with EFilm
	iRTVDeSpaceDe(m_targetAETitle);
	LogMessage(	kDebug, "DEBUG:(%d) CMove::Process() - ParseCMoveRQMessage() - destination AETitle = %s\n", m_connectInfo.AssociationID, m_targetAETitle );

	// Get the information model                
    status = MC_Get_Message_Service(m_messageID, &m_serviceName, &m_command);
    if (status != MC_NORMAL_COMPLETION)
    {
		return HandleFinalError("ParseCMoveRQMessage - MC_Get_Message_Service() failed", C_MOVE_FAILURE_INVALID_DATASET);  
    }

	//	-- - 11/01/02 - made this handle lists of UID's, and also handle instance, series and study levels.
	//	TODO: Patient Level
	if (!strncmp(m_serviceName, "PATIENT_ROOT", 16) && !strncmp (m_serviceName,  "STUDY_ROOT", 16))
    {
		return HandleFinalError("ParseCMoveRQMessage - Patient / Study model not supported", C_MOVE_FAILURE_UNABLE_TO_PROCESS);  
	}

	//	Get the QR level
	status = MC_Get_Value_To_String(m_messageID, MC_ATT_QUERY_RETRIEVE_LEVEL, sizeof(retrieveLevelStr), retrieveLevelStr);
	if (status != MC_NORMAL_COMPLETION)
	{
		return HandleFinalError("ParseCMoveRQMessage - Missing QueryRetrieveLevel", C_MOVE_FAILURE_INVALID_DATASET);
	}

	LogMessage(	kDebug, "DEBUG:(%d) CMove - retrieve level = %s\n",m_connectInfo.AssociationID, retrieveLevelStr);

	//	Hanedle the retrieve levels
	char uid[kVR_UI];
	if (!strcmp(retrieveLevelStr, "PATIENT"))
	{
		return HandleFinalError("ParseCMoveRQMessage - Patient Level Retrieve not supported", C_MOVE_FAILURE_UNABLE_TO_PROCESS);
	} 
	else if (!strcmp(retrieveLevelStr, "STUDY"))
	{
		m_level = kStudyLevel;
		return BuildInstanceFilter(MC_ATT_STUDY_INSTANCE_UID, m_studyInstanceUIDs);	
	} 
	else if (!strcmp(retrieveLevelStr, "SERIES"))
	{
		status = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_INSTANCE_UID, sizeof uid, uid);
		if (status != MC_NORMAL_COMPLETION)
		{
			return HandleFinalError("BuildInstanceFilter - Missing unique key in identifier", C_MOVE_FAILURE_INVALID_DATASET);
		}
		m_studyInstanceUIDs.push_back(uid);

		m_level = kSeriesLevel;
		return BuildInstanceFilter(MC_ATT_SERIES_INSTANCE_UID, m_seriesInstanceUIDs);	
	} 
	else if (!strcmp(retrieveLevelStr, "IMAGE"))
	{
		status = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_INSTANCE_UID, sizeof uid, uid);
		if (status != MC_NORMAL_COMPLETION)
		{
			return HandleFinalError("BuildInstanceFilter - Missing unique key in identifier", C_MOVE_FAILURE_INVALID_DATASET);
		}
		m_studyInstanceUIDs.push_back(uid);

		status = MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_INSTANCE_UID, sizeof uid, uid);
		if (status != MC_NORMAL_COMPLETION)
		{
			return HandleFinalError("BuildInstanceFilter - Missing unique key in identifier", C_MOVE_FAILURE_INVALID_DATASET);
		}
		m_seriesInstanceUIDs.push_back(uid);

		m_level = kInstanceLevel;
		return BuildInstanceFilter(MC_ATT_SOP_INSTANCE_UID, m_SOPInstanceUIDs);	
	} 
	else
	{
		char msg[256];
		_snprintf(msg, sizeof msg, "ParseCMoveRQMessage - invalid Retrieve Level: %s", retrieveLevelStr);
		return HandleFinalError(msg, C_MOVE_FAILURE_UNABLE_TO_PROCESS);
	}

	return MC_SYSTEM_ERROR; 
}

//--------------------------------------------------------------------
//
int CMove::BuildInstanceFilter(unsigned long iTag, std::vector<std::string>& iFilter)
{
	int status;
	char uid[kVR_UI];
	int count = 0;

 	status = MC_Get_Value_Count(m_messageID, iTag, &count);
	if (status != MC_NORMAL_COMPLETION || count < 1)
	{
		return HandleFinalError("BuildInstanceFilter - Missing unique key in identifier", C_MOVE_FAILURE_INVALID_DATASET);
	}

	status = MC_Get_Value_To_String(m_messageID, iTag, sizeof uid, uid);
	if (status != MC_NORMAL_COMPLETION)
	{
		return HandleFinalError("BuildInstanceFilter - Missing unique key in identifier", C_MOVE_FAILURE_INVALID_DATASET);
	}
	iFilter.push_back(uid);

	int i;
	for(i=1; i<count; i++)
	{
		status = MC_Get_Next_Value_To_String(m_messageID, iTag, sizeof uid, uid);
		if (status != MC_NORMAL_COMPLETION)
		{
			char msg[256];
			_snprintf(msg, sizeof msg, "BuildInstanceFilter - Error (%d,%s) during MC_Get_Next_Value_To_String()\n", status, MC_Error_Message((MC_STATUS)status));
			return HandleFinalError(msg, C_MOVE_FAILURE_UNABLE_TO_PROCESS);
		}
		iFilter.push_back(uid);
	}

	return MC_NORMAL_COMPLETION;  
}

//--------------------------------------------------------------------
//
int CMove::AttemptSendResponse(int iResponseMsg, int iResponseCode)
{
	int status;

	status = MC_Set_Service_Command(iResponseMsg, m_serviceName, C_MOVE_RSP);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("ERROR:(%d) CMove::Process() - SendCMoveResponse() - Could not Set service and command to ack. Progress Msg (%d,%s) on MC_Set_Service_Command()\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
		return status;  
	}

	//	According to DICOM PS3.4 - C4.2.1.6
	if (iResponseCode == C_MOVE_PENDING_MORE_SUB_OPERATIONS || iResponseCode == C_MOVE_CANCEL_REQUEST_RECEIVED)
	{
		//	RL - 1/18/02 - Added this condition to test bug from Toshiba
		unsigned int nr = (gConfig.m_sendCMovePendingWithZeroRemaining) ? 0 : m_remainingCStoreSubOperations;

		//	Set MC_ATT_NUMBER_OF_REMAINING_SUBOPERATIONS
		status = MC_Set_Value_From_UInt(iResponseMsg, MC_ATT_NUMBER_OF_REMAINING_SUBOPERATIONS, nr);
		if (status != MC_NORMAL_COMPLETION)
		{
			LogMessage("ERROR:(%d) CMove::Process() - SendCMoveResponse() - Faild on (%d,%s) to set MC_ATT_NUMBER_OF_REMAINING_SUBOPERATIONS to 0\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
			return status;  
		}

		//	Set MC_ATT_NUMBER_OF_REMAINING_SUBOPERATIONS
		status = MC_Set_Value_From_UInt(iResponseMsg, MC_ATT_NUMBER_OF_COMPLETED_SUBOPERATIONS, m_successfulCStoreSubOperations);
		if (status != MC_NORMAL_COMPLETION)
		{
			LogMessage("ERROR:(%d) CMove::Process() - SendCMoveResponse() - Set # of completed suboperations failed (%d,%s) on MC_Set_Value_From_String()\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
			return status;  
		}

		//	Set MC_ATT_NUMBER_OF_FAILED_SUBOPERATIONS
		status = MC_Set_Value_From_UInt(iResponseMsg, MC_ATT_NUMBER_OF_FAILED_SUBOPERATIONS, m_failedCStoreSubOperations);
		if (status != MC_NORMAL_COMPLETION)
		{
			LogMessage("ERROR:(%d) CMove::Process() - SendCMoveResponse() - Set # of failed suboperations failed (%d,%s) on MC_Set_Value_From_String()\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
			return status;  
		}	
			
		//	Set MC_ATT_NUMBER_OF_WARNING_SUBOPERATIONS
		status = MC_Set_Value_From_UInt(iResponseMsg, MC_ATT_NUMBER_OF_WARNING_SUBOPERATIONS, m_warnedCStoreSubOperations);
		if (status != MC_NORMAL_COMPLETION)
		{
			LogMessage("ERROR:(%d) CMove::Process() - SendCMoveResponse() - Set # of warning suboperations failed (%d,%s) on MC_Set_Value_From_String()\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
			return status;  
		}	
	}
	else if (iResponseCode == C_MOVE_SUCCESS_NO_FAILURES)
	{
		if (m_failedCStoreSubOperations > 0 || m_warnedCStoreSubOperations > 0)
			iResponseCode = C_MOVE_WARNING_ONE_OR_MORE_FAILURES;
	}
	
	//	TODO: Don't know if this should be a retry loop or not - too risky to take it out right now - -- - 10/11/02
	int ntries = 0;
	int cMoveRespStatus;
	do
	{
		status = CheckCMoveCancellation(m_currentCStoreSubOperationIndex, cMoveRespStatus);
		if (cMoveRespStatus == C_MOVE_CANCEL_REQUEST_RECEIVED || TRDICOMUtil::AssociationIsDead((MC_STATUS) status))
		{
			m_abort = true;
			return status;
		}

		status = MC_Send_Response_Message(m_connectInfo.AssociationID, iResponseCode, iResponseMsg);
	} while (ntries++ < gConfig.m_numberOfRetries && status == kTimeout);

	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("ERROR:(%d) CMove::Process() - SendCMoveResponse() - Failed to send the response message to C_MOVE SCU (%d,%s) on MC_Send_Response_Message()\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));
		return status;  
	}
 
	return MC_NORMAL_COMPLETION;  
}

int CMove::BuildSendMessage(const char *iFilePath)
{
	int		status;
	MediaCBinfo		callbackInfo;
	char			SOPClassUID[kVR_UI];
	char			serviceName[32];
 	char			transferSyntaxUID[kVR_UI];
	int syntax;
	int oMID;
  
	//	Create a file object to accept the DICOM file from disk
	status = MC_Create_Empty_File(&oMID, iFilePath);
	if (status != MC_NORMAL_COMPLETION)
    {
       	LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to MC_Create_Empty_File() with file path =%s\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), iFilePath);
        return -1;
    }
	MessageFreeGuard guard(oMID, true);

	//	Read the file in from disk.  Assume it is in part 10 format.
	//	TODO: Do we need to register a different applicationID here to avoid GetPixelData callbacks?
	status = MC_Open_File(m_connectInfo.ApplicationID, oMID, &callbackInfo, AqMediaToFileObj);
	if (status != MC_NORMAL_COMPLETION)
    {
       	LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to MC_Open_File() with file path =%s\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) , iFilePath);

		//	Attempt to get more info about the reason the file couldn't be opened
		struct _stat statBuf;
		int stat = _stat(iFilePath, &statBuf);
		if (stat)
		{
			LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Could not stat file %s\n", m_connectInfo.AssociationID, iFilePath);
		} 
		else
		{
			//	Able to stat file, how about open
			LogMessage(kInfo,"INFO:(%d) CMove::Process() - BuildSendMessage() - was able to stat file %s, trying open...\n", m_connectInfo.AssociationID, iFilePath);
			FILE* fptr = fopen(iFilePath, "rb");
			if (!fptr)
			{
				LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Could not open file %s\n", m_connectInfo.AssociationID, iFilePath);
			} 
			else
			{
				fclose(fptr);
				LogMessage(kInfo,"INFO:(%d) CMove::Process() - BuildSendMessage() - was able to open file %s, reason for failed MC_Open_File unknown\n", m_connectInfo.AssociationID, iFilePath);
			}
		}
		return -1;
    }

	//	Get the Transfer Syntax from the file object
	status =  MC_Get_Value_To_String(oMID, MC_ATT_TRANSFER_SYNTAX_UID, sizeof transferSyntaxUID, transferSyntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(transferSyntaxUID, (TRANSFER_SYNTAX*)&syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
      		LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed to get Transfer Syntax on (%d,%s) MC_Get_Enum_From_Transfer_Syntax ()\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status));	 
			return -1;
		}
	}
 
	//	Convert the file to a message object, so we can send it to the C-STORE SCP
	status = MC_File_To_Message(oMID); 
	if (status != MC_NORMAL_COMPLETION)
    {
       	LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to MC_File_To_Message()  with file path =%s\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), iFilePath);
        return status; 
    }

	guard.SetFileType(false); // convertd to message ID now

	////////
	if(gConfig.m_safetyCheck !=0 ){
		//#27 2012/06/14 K.Ko

		std::string str_temp;
		char _str_buff[256];
		
		//ref:
		//CPxDicomImage::FillSortInfo
		if(m_curStudy.size()>0){
			DICOMStudy &ref_dicom_info = m_curStudy[0];

			std::string ref_str_temp;
			std::string str_temp;
			//PatientName
			_str_buff[0]=0;

			CPxDicomMessage dicom_temp;
		//	 status = MC_Get_Value_To_String(oMID, MC_ATT_PATIENTS_NAME, sizeof(_str_buff), _str_buff);
			status = dicom_temp.GetValue(oMID, MC_ATT_PATIENTS_NAME, _str_buff, sizeof(_str_buff));
			if (status != MC_NORMAL_COMPLETION)
			{  
				LogMessage(kDebug,"ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s)  MC_Get_Value_To_String MC_ATT_PATIENTS_NAME   \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) );
				//return -1; //do not return error
			}else{
				ref_str_temp	= ref_dicom_info.m_patientsName;
				if(ref_str_temp.size()>0){
					str_temp		= 	_str_buff;
					if(str_temp != ref_str_temp){
						//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
						//skip it
				//		LogMessage(kErrorOnly,"ERROR:[C%08d](%d) CMove::BuildSendMessage() - Failed on check patientName  %s != %s \n",DicomServInfor_SafetyCheckPatientInfo,m_connectInfo.AssociationID,ref_str_temp.c_str(),str_temp.c_str() );
				//		return -1;
					}
				}
			}

			//PatientID
			_str_buff[0]=0;
			 status = MC_Get_Value_To_String(oMID, MC_ATT_PATIENT_ID, sizeof(_str_buff), _str_buff);
			if (status != MC_NORMAL_COMPLETION)
			{  
				LogMessage(kDebug,"ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s)  MC_Get_Value_To_String MC_ATT_PATIENT_ID   \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) );
				//return -1;//do not return error
			}else{
				ref_str_temp	= ref_dicom_info.m_patientID;
				if(ref_str_temp.size()>0){
					str_temp		= 	_str_buff;
					if(str_temp != ref_str_temp){
						LogMessage(kErrorOnly,"ERROR:[C%08d](%d) CMove::BuildSendMessage() - Failed on check PatientID  %s != %s \n",DicomServInfor_SafetyCheckPatientInfo,m_connectInfo.AssociationID,ref_str_temp.c_str(),str_temp.c_str() );
						return -1;
					}
				}
			}

			//BirthDate
			_str_buff[0]=0;
			 status = MC_Get_Value_To_String(oMID, MC_ATT_PATIENTS_BIRTH_DATE, sizeof(_str_buff), _str_buff);
			if (status != MC_NORMAL_COMPLETION)
			{  
				LogMessage(kDebug,"ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s)  MC_Get_Value_To_String MC_ATT_PATIENTS_BIRTH_DATE   \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) );
				//return -1;//do not return error
			}else{
				ref_str_temp	= ref_dicom_info.m_patientsBirthDate;
				if(ref_str_temp.size()>0){
					str_temp		= 	_str_buff;
					if(str_temp != ref_str_temp){
						LogMessage(kErrorOnly,"ERROR:[C%08d](%d) CMove::BuildSendMessage() - Failed on check BirthDate  %s != %s \n",DicomServInfor_SafetyCheckPatientInfo,m_connectInfo.AssociationID,ref_str_temp.c_str(),str_temp.c_str() );
						return -1;
					}
				}
			}
		}

	}

	//
	//	Populate the message with appropriate values
	//

	//	Get the SOP Class UID - tells us what kind of object it is
    status = MC_Get_Value_To_String(oMID, MC_ATT_SOP_CLASS_UID, sizeof(SOPClassUID), SOPClassUID);
    if (status != MC_NORMAL_COMPLETION)
    {  
		LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to get SOP_CLASS_UID in MC_Get_Value_To_String() for  cStoreMessageID = %d \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), oMID);
        return -1;
    }
       
	//	What is the MergeCOM service name for this SOP Class UID?
    status = MC_Get_MergeCOM_Service(SOPClassUID, serviceName, sizeof(serviceName));
    if (status != MC_NORMAL_COMPLETION)
    {
        LogMessage("ERROR:(%d) CMove::Process() -BuildSendMessage() - Failed on (%d,%s) in MC_Get_MergeCOM_Service() for SOPClassUID = %d\n",  m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), SOPClassUID);
        return -1;
    }

	//	This is needed in preparation for sending the message over the network
    status = MC_Set_Service_Command(oMID, serviceName, C_STORE_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) in MC_Set_Service_Command() for cStoreMessageID = %d\n",  m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), oMID);
		return -1;
    }
  
    //  Get affected SOP Instance UID 
    char affectedSOPinstance[kVR_UI];   
    status = MC_Get_Value_To_String(oMID, MC_ATT_SOP_INSTANCE_UID, sizeof(affectedSOPinstance), affectedSOPinstance);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() -BuildSendMessage() - Failed on (%d,%s) to get Affected SOP Instance UID in MC_Get_Value_To_String() for cStoreMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), oMID);
        return -1;
    }
 
    // Set the affected SOP Instance UID 
    status = MC_Set_Value_From_String (oMID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, affectedSOPinstance);
    if (status != MC_NORMAL_COMPLETION)
    {
        LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to Set Affected SOP Instance UID = %d in MC_Set_Value_From_String () for cStoreMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), affectedSOPinstance,  oMID);
		return -1;
    }

    // Set the move originator application entity title 
    status = MC_Set_Value_From_String(oMID, MC_ATT_MOVE_ORIGINATOR_APPLICATION_ENTITY_TITLE, m_connectInfo.RemoteApplicationTitle);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to Set Originator Application Entity Title in MC_Set_Value_From_String() for cStoreMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), oMID);	 
        return -1;
    }
     
    // Get the move originator message ID (dicomMessageID).  The dicomMessageID is different from the message ID 
	// which is contained in the C-MOVE-RQ message's group 0 elements (and usually handled automatically by the toolkit) 
	unsigned int dicomMessageID;
    status = MC_Get_Value_To_UInt(m_messageID, MC_ATT_MESSAGE_ID, &dicomMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to get dicomMessageID in MC_Get_Value_To_UInt() for  cMoveMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), m_messageID);
        return -1;
    }
   
    // Set the move originator message ID 
    status = MC_Set_Value_From_UInt(oMID, MC_ATT_MOVE_ORIGINATOR_MESSAGE_ID, dicomMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
		LogMessage("ERROR:(%d) CMove::Process() - BuildSendMessage() - Failed on (%d,%s) to set Move Originator Message ID in MC_Get_Value_To_UInt() for cStoreMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), oMID);
        return -1;
    }

	//	Set the Message Transfer Syntax to what was in the file.  Only do this for non-encapsulated syntaxes
	//  otherwise, Merge will translate automatically for us.
	if (syntax != IMPLICIT_LITTLE_ENDIAN && syntax != EXPLICIT_LITTLE_ENDIAN && syntax != EXPLICIT_BIG_ENDIAN)
	{
		status = MC_Set_Message_Transfer_Syntax(oMID, (TRANSFER_SYNTAX)syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			LogMessage("ERROR:(%d) CMove::Process() - BuildAndSendCStoreRequestMsg() - Failed on (%d,%s) to set Transfer Syntax in MC_Get_Value_To_UInt() for cStoreMessageID = %d\n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status), oMID);
			return -1;
		}
	}
	guard.Release(); // ready to give out message ID
	return oMID;

}

//--------------------------------------------------------------------
//  Send a progress response message to C-MOVE SCU with the number of 
//  Successful,Failed, Warning and remaining C-STORE sub-operations.  
//
int CMove::SendCMoveResponse(int iResponseCode)
{
	int status;
	int responseMsg = -1;

	if (iResponseCode == C_MOVE_PENDING_MORE_SUB_OPERATIONS)
	{
		if (gConfig.m_dontSendCMoveRSP)
			return MC_NORMAL_COMPLETION;

		MC_Open_Empty_Message(&responseMsg);
	}
	else
	{
		responseMsg = m_finalCMoveRespMSG;
	}

	status = AttemptSendResponse(responseMsg, iResponseCode);
	if (iResponseCode == C_MOVE_PENDING_MORE_SUB_OPERATIONS)
	{
		MC_Free_Message(&responseMsg);
	} 

	return status;
}

//--------------------------------------------------------------------
//
int CMove::AddToFailedSOPList(char* iSOPInstanceUID)
{
	int status;

	//	-- - 10/24/02 - Pad to even length if needed (See PS3.5 Table 6.2-1 Encoding rules for VR of UI).
	int len = strlen(iSOPInstanceUID);
	if (len % 2 != 0)
	{
		iSOPInstanceUID[len] = 32;
		iSOPInstanceUID[len+1] = 0;
	}

	if(m_failedSOPListCount == 0)
	{
		m_failedSOPListCount++;
		status = MC_Set_Value_From_String (m_finalCMoveRespMSG, MC_ATT_FAILED_SOP_INSTANCE_UID_LIST, iSOPInstanceUID);
	}
	else
	{
		status = MC_Set_Next_Value_From_String (m_finalCMoveRespMSG, MC_ATT_FAILED_SOP_INSTANCE_UID_LIST, iSOPInstanceUID);
	}

	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage(kDebug, "DEBUG:(%d) CMove::Process() - AddToFailedSOPList() failed...\n", m_connectInfo.AssociationID);
	}

	return status;
}

//--------------------------------------------------------------------
//
int CMove::CECHOProcess()
{
	int	status;
	MC_COMMAND	command;
	LogMessage(kDebug,  "DEBUG:(%d) CMove::Process() - Sending C-ECHO Request before C-STORE in C-MOVE (simulating behavior for test)\n",m_cStoreAssociationID);
	
	int echoMsg = -1;
	status = MC_Open_Empty_Message(&echoMsg);
	if(status != MC_NORMAL_COMPLETION) 
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed to open Empty Message for E-ECHO\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status)); 
		return status;
	}

	MessageFreeGuard guard1(echoMsg);

	status = MC_Set_Service_Command(echoMsg, "STANDARD_ECHO", C_ECHO_RQ);
	if(status != MC_NORMAL_COMPLETION) 
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed to set Service Command for E-ECHO\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status)); 
		return status;
	}

	//	Send the echo request
	status =  MC_Send_Request_Message(m_cStoreAssociationID, echoMsg);
	if(status != MC_NORMAL_COMPLETION) 
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed on attempt to send C-ECHO-RQ\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status)); 
		return status;
	}

	//	Read the C-ECHO-RSP message
	int echoRsp = -1;
	char*	serviceName;
	status =  MC_Read_Message(m_cStoreAssociationID, 10, &echoRsp, &serviceName, &command);
	if(status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed on attempt to Read C-ECHO-RSP\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status)); 
		return status;
	} 
	
	MessageFreeGuard guard2(echoRsp);

	if (gConfig.m_messageDump)
	{
		TRDICOMUtil::DumpMessage(echoRsp, serviceName, command);
	}

	if(command != C_ECHO_RSP)
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed on attempt to Read C-ECHO-RSP\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status)); 
		return status;
	}
	
	unsigned int echoRspStatus = -1;
	status = MC_Get_Value_To_UInt(echoRsp, MC_ATT_STATUS, &echoRspStatus);
	if(status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed on attempt to Read C-ECHO-RSP Response Status\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status)); 
		return status;
	} 

	if (echoRspStatus != C_ECHO_SUCCESS)
	{
		LogMessage( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Bad C-ECHO-RSP Response Status %d\n",m_cStoreAssociationID, status, MC_Error_Message((MC_STATUS) status), echoRspStatus); 
		return status;
	}

	LogMessage(kDebug,  "DEBUG:(%d) CMove::Process() - Received C-ECHO Reply\n",m_cStoreAssociationID);
	return status;
}


AqObject dicomServerObject;
ApplicationEntity outboundLocalAE;

//-----------------------------------------------------------------------------------------------------
//
int CMove::MakeDicomServerObject(void)
{	
	int status;

	if (!dicomServerObject.GetID())
	{
		dicomServerObject.m_Type = kTypeDICOMServer;
		ASTRNCPY(dicomServerObject.m_FullName, "AqNETDicomServer");
		ASTRNCPY(dicomServerObject.m_Description, "DICOM Server Process");
		ASTRNCPY(dicomServerObject.m_Hostname, TRPlatform::GetMyName());
		ASTRNCPY(dicomServerObject.m_Address, TRPlatform::GetIPAddressString());
		status = m_db.MakeAqObject(dicomServerObject);
		if (status != kOK)
		{
			LogMessage("ERROR:[C%08d] Failed to make DicomServerObject\n",DicomServError_CMoveError);
			FlushLog();
			return status;
		}
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
int CMove::MakeOutboundAEObject(void)
{	
	int status, objectID = 0;
	
	if (outboundLocalAE.GetID())
		return kOK;

	ASTRNCPY(outboundLocalAE.m_AETitle, TRDICOMUtil::CalculateLocalAETitle().c_str());
	ASTRNCPY(outboundLocalAE.m_hostName, TRPlatform::GetMyName());
	ASTRNCPY(outboundLocalAE.m_IPAddress, TRPlatform::GetIPAddressString());
	outboundLocalAE.m_IsLocalAE = 1;

	status = m_db.MakeAqObject(outboundLocalAE);
	if (status != kOK)
	{
		LogMessage("ERROR:[C%08d] Failed to make AE Object for remote AE [%s, %s, %s]\n",DicomServError_CMoveError, outboundLocalAE.m_AETitle, outboundLocalAE.m_hostName, outboundLocalAE.m_IPAddress);
		FlushLog();
		return status;
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
int CMove::MakeRemoteAEObject(ApplicationEntity& iRemoteAE)
{	
	int notLocal = 0, status, objectID = 0;
	status = m_db.MakeAqObject(iRemoteAE);
	if (status != kOK)
	{
		LogMessage("ERROR:[C%08d] Failed to make AE Object for remote AE [%s, %s, %s]\n", DicomServError_CMoveError,iRemoteAE.m_AETitle, iRemoteAE.m_hostName, iRemoteAE.m_IPAddress);
		FlushLog();
		return status;
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
int CMove::MakeLocalAEObject(void)
{	
	int isLocal = 1, status;
	if (!m_connectInfo.LocalAEAqObjectID)
	{
		int localAEID;
		std::string strSQL = "SELECT ID from LocalAE where AETitle = '" + std::string(m_connectInfo.LocalApplicationTitle) + "'";
		status = m_db.SQLGetInt(strSQL.c_str(), localAEID);
		if (status != kOK || localAEID <= 0)
		{
			LogMessage("ERROR:[C%08d] Failed to make AE Object for local AE %s\n",DicomServError_CMoveError, m_connectInfo.LocalApplicationTitle);
			FlushLog();
			return status;
		}

		status = m_db.MakeAEObject(localAEID, m_connectInfo.LocalAEAqObjectID, isLocal);
		if (status != kOK)
		{
			LogMessage("ERROR:[C%08d] Failed to make AE Object for local AE %s\n", DicomServError_CMoveError,m_connectInfo.LocalApplicationTitle);
			FlushLog();
			return status;
		}
	}

	return kOK;
}


//-----------------------------------------------------------------------------------------------------
//
int CMove::AuditLogSendSeries(const char* iSeriesInstanceUID)
{
//
	if(gConfig.m_traceDicomLevel >=2){ //#693  
		LogMessage("INFO:[C%08d] CMove::Process() SendSeries for AETitle: %s and SeriesUID: %s\n",
		DicomServInfor_CMoveInfo, m_targetAETitle, iSeriesInstanceUID);
		FlushLog();
	}
//
	if (!m_db.IsAuditTrailEnabled())
		return kOK;

	int status = MakeDicomServerObject();
	if (status != kOK)
	{
		return status;
	}

	ApplicationEntity targetAE;
	ASTRNCPY(targetAE.m_AETitle, m_targetAETitle);
	ASTRNCPY(targetAE.m_IPAddress, m_targetIPAddress);
	targetAE.m_port = m_targetPort;
	targetAE.m_IsLocalAE = 0;

	if (status = MakeRemoteAEObject(targetAE) != kOK)
	{
		return status;
	}

	ApplicationEntity originatorAE;
	ASTRNCPY(originatorAE.m_AETitle, m_connectInfo.RemoteApplicationTitle);
	ASTRNCPY(originatorAE.m_IPAddress, m_connectInfo.RemoteIPAddress);
	ASTRNCPY(originatorAE.m_hostName, m_connectInfo.RemoteHostName);
	originatorAE.m_port = 0;
	originatorAE.m_IsLocalAE = 0;
	if (status = MakeRemoteAEObject(originatorAE) != kOK)
	{
		return status;
	}
	m_connectInfo.RemoteAEAqObjectID = originatorAE.GetID();

	if (status = MakeOutboundAEObject() != kOK)
	{
		return status;
	}

	if (!m_seriesLevelObjectID)
	{
		status = m_db.MakePatientObject(iSeriesInstanceUID, m_studyLevelObjectID, m_seriesLevelObjectID, originatorAE.GetID() );
		if (status != kOK)
		{
			LogMessage("ERROR:[C%08d] Failed to make serisLevelObject for seriesUID: %s\n",DicomServError_CMoveError, iSeriesInstanceUID);
			FlushLog();
			return status;
		}
	}

	//	Make SeriesEvenLog entry
	EventLog event;
	event.m_Actor		= dicomServerObject.GetID();
	event.m_Activity	= m_db.GetActionID("Send");
	event.m_ActOn		= m_seriesLevelObjectID;
	event.m_Requestor	= originatorAE.GetID();
	event.m_ActionFrom	= outboundLocalAE.GetID();
	event.m_ActionAt	= targetAE.GetID();
	event.m_eventType	= kSeriesEventLog;

	m_db.LogEvent(event);

	return kOK;
}


#if 0

MC_STATUS AqFileObjToMedia( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    size_t     count;
    CBinfo*    cbInfo = (CBinfo*)A_userInfo;

 	//  
	// Added try/catch  for extra protection against bad data
	try  
	{
	   if (A_isFirst)
			cbInfo->fp = fopen(A_filename, "wb");
			
		
		//	Added check for datasize.  If we are being called with something that is
		//		obviously too big, just fail.  NOTE:  the caller should set datasize
		//		before invoking this callback, otherwise this check will not happen.
		if (!cbInfo->fp || (cbInfo->dataSize > 0 && A_dataSize > cbInfo->dataSize))
			return MC_CANNOT_COMPLY;

		if(A_dataSize > 0)
		{
			extern int errno;
			errno = 0;
			count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfo->fp);
			if (count != (size_t)A_dataSize)
			{
				printf("fwrite error: %d",errno);
				return MC_CANNOT_COMPLY;
			}
		}

		if (A_isLast)
		{
			/*
			 * NULL ->fp so that the routine calling MC_Write file knows
			 * not to close the stream.
			 */
			fclose(cbInfo->fp);
			cbInfo->fp = NULL;
		}

		return MC_NORMAL_COMPLETION;
	}
	catch (...)
	{
		assert(0);
		return MC_CANNOT_COMPLY;
	}

} /* FileObjToMedia() */

 

MC_STATUS FileObjToMemory( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    MemoryCBinfo*    cbInfo = (MemoryCBinfo*)A_userInfo;

	try  
	{
	   if (A_isFirst)
			cbInfo->bytesRead = 0;

		if ((cbInfo->dataSize > 0 && A_dataSize > cbInfo->dataSize))
			return MC_CANNOT_COMPLY;

		if(A_dataSize > 0)
		{
			unsigned long leftBufferSize = cbInfo->dataSize - cbInfo->bytesRead;
			if ( leftBufferSize < A_dataSize )
				return MC_CANNOT_COMPLY; // do not have enough buffer

			memcpy(cbInfo->memoryPointerForWrite+cbInfo->bytesRead,
				A_dataBuffer,A_dataSize);

			cbInfo->bytesRead += A_dataSize;

		}

		return MC_NORMAL_COMPLETION;
	}
	catch (...)
	{
		assert(0);
		return MC_CANNOT_COMPLY;
	}

} /* FileObjToMemory() */


//--------------------------------------------------------------------
//  Function    :   MediaToFileObj
//  Parameters  :   A_fileName   - Filename to open for reading
//                  A_userInfo   - Pointer to an object used to preserve
//                                 data between calls to this function.
//                  A_dataSize   - Number of bytes read
//                  A_dataBuffer - Pointer to buffer of data read
//                  A_isFirst    - Set to non-zero value on first call
//                  A_isLast     - Set to 1 when file has been completely 
//                                 read
//
//  Returns     :   MC_NORMAL_COMPLETION on success
//                  any other PxDicomStatus value on failure.
//
//  Description :   Callback function used by MC_Open_File to read a file
//                  in the DICOM Part 10 (media) format.
//
//--------------------------------------------------------------------
MC_STATUS AqMediaToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast)
{

    MediaCBinfo*    callbackInfo = (MediaCBinfo*)A_userInfo;
    size_t          bytes_read;

    if (!A_userInfo)
	{
		callbackInfo->errCode |= kCBERRBadArg;
        return  MC_CANNOT_COMPLY;
	}

    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
        callbackInfo->fp = fopen(A_filename, BINARY_READ);
    }
    
    if (!callbackInfo->fp)
	{
		callbackInfo->errCode |= kCBERROpenFile;
		return MC_CANNOT_COMPLY;
	}

    bytes_read = fread(callbackInfo->buffer, 1, sizeof(callbackInfo->buffer),
                       callbackInfo->fp);
    if (ferror(callbackInfo->fp))
	{
		callbackInfo->errCode |= kCBERRReadFile;
        return MC_CANNOT_COMPLY;
	}

    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytes_read;
    callbackInfo->bytesRead += bytes_read;
    return  MC_NORMAL_COMPLETION;
    
} /* MediaToFileObj() */


MC_STATUS  AqMemoryToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast)
{

    MemoryCBinfo*    callbackInfo = (MemoryCBinfo*)A_userInfo;

    if (!A_userInfo)
        return  MC_CANNOT_COMPLY;

    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
    }
    
	unsigned long leftBytes = callbackInfo->dataSize - callbackInfo->bytesRead;
	int bytesToCopy = (int)(leftBytes > sizeof(callbackInfo->buffer) ? sizeof(callbackInfo->buffer) : leftBytes);
	if ( bytesToCopy > 0 )
	{
		memcpy(callbackInfo->buffer,
			callbackInfo->memoryPointerForRead+callbackInfo->bytesRead,
			bytesToCopy);
	}
	callbackInfo->bytesRead += bytesToCopy;
	*A_dataBuffer = callbackInfo->buffer;
	*A_dataSize = bytesToCopy;

	if ( leftBytes <= sizeof(callbackInfo->buffer) )
	{
        *A_isLast = 1;
	}

    return  MC_NORMAL_COMPLETION;
    
} /* MemoryToFileObj() */

#endif