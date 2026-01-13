/***********************************************************************
 * CFind.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2011, All rights reserved.
 *
 *	PURPOSE:
 *		Processes CFind Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "CFind.h"

#include <sys/stat.h>
#include <direct.h>
#include <assert.h>
#include <vector>

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#include "PxDicomMessage.h"
#include "PxDicomImage.h"

#else
#include "rtvMergeToolKit.h"
#include "VLIDicomMessage.h"
#endif

#include "AqCore/TRPlatform.h"
#include "Globals.h"
#include "SeriesDirMonitor.h"
//#include "TRDICOMUtil.h"



// The commeted out constants have no filter build on

// DICOMData
const __int64 kPatientsName =					(1 << 1);
const __int64 kPatientID =						(1 << 2);
const __int64 kPatientsBirthDate =				(1 << 3);
//const __int64 kPatientsBirthDate2 =			(1 << 4);
const __int64 kPatientsSex =					(1 << 5);
const __int64 kPatientsAge =					(1 << 6);
const __int64 kStudyDate =						(1 << 7);
const __int64 kStudyTime =						(1 << 8);
//const __int64 kStudyDate2 =					(1 << 9);
//const __int64 kStudyTime2 =					(1 << 10);
const __int64 kAccessionNumber =				(1 << 11);
const __int64 kStudyID =						(1 << 12);
const __int64 kStudyInstanceUID =				(1 << 13);
const __int64 kModalitiesInStudy =				(1 << 14);
const __int64 kReferringPhysiciansName =		(1 << 15);
const __int64 kStudyDescription =				(1 << 16);
const __int64 kNumberOfStudyRelatedSeries =		(1 << 17);
const __int64 kNumberOfStudyRelatedInstances =	(1 << 18);
//const __int64 kStationName =					(1 << 19);
//const __int64 kOfflineFlag =					(1 << 20);
const __int64 kSeriesInstanceUID =				(1 << 21);
const __int64 kSeriesNumber =					(1 << 22);
const __int64 kSeriesDescription =				(1 << 23);
const __int64 kModality =						(1 << 24);
const __int64 kBodyPartExamined =				(1 << 25);
const __int64 kViewPosition =					(1 << 26);
const __int64 kNumberOfSeriesRelatedInstances =	(1 << 27);
//const __int64 kSavePath =						(1 << 28);
const __int64 kSOPInstanceUID =					(1 << 29);
const __int64 kSOPClassUID =					(1 << 30);
const __int64 kInstanceNumber =					(1 << 31);

//
// #76
// 32bit以内ように調整
// 注意
// 32bit以内、使用されていないものを流用
//
const __int64 kSeriesDate						(1 << 4);//(1 << 32);
const __int64 kSeriesTime						(1 << 9);//(1 << 33);
//const __int64 kRows =							(1 << 32);
//const __int64 kColumns =						(1 << 33);
const __int64 kImageType =						(1 << 10);//(1 << 34);
//const __int64 kkBitsAllocated =				(1 << 35);
//const __int64 kBitsStored =					(1 << 36);
//const __int64 kPixelRepresentation =			(1 << 37);
//const __int64 kPatientOrientation =			(1 << 38);
//const __int64 kInsertDate =					(1 << 39);
//const __int64 kInsertTime =					(1 << 40);
//const __int64 kFileSize =						(1 << 41);
//const __int64 kFileName =						(1 << 42);
//const __int64 kSubSeriesNumber =				(1 << 43);
//const __int64 kImagePosition =				(1 << 44);
//const __int64 kImagePositionX =				(1 << 45);
//const __int64 kImagePositionY =				(1 << 46);
//const __int64 kImagePositionZ =				(1 << 47);
//const __int64 kImageOrientationPatientXX =	(1 << 48);
//const __int64 kImageOrientationPatientXY =	(1 << 49);
//const __int64 kImageOrientationPatientXZ =	(1 << 50);
//const __int64 kImageOrientationPatientYX =	(1 << 51);
//const __int64 kImageOrientationPatientYY =	(1 << 52);
//const __int64 kImageOrientationPatientYZ =	(1 << 53);
//const __int64 kMediaLabel =					(1 << 54);
const __int64 kCharacterSet =					(1 << 19);//(1 << 55);
//const __int64 kStudyRowStatus =				(1 << 56);
//const __int64 kSeriesRowStatus =				(1 << 57);
//const __int64 kInstanceRowStatus =			(1 << 58);

typedef std::map<long, MC_STATUS> statMap;


//-----------------------------------------------------------------------------------------------------
//
void SetMasks(statMap& iStats, long& oInputMask, long& oOutputMask)
{
	statMap::iterator si;

	oInputMask = oOutputMask = 0;
	for (si = iStats.begin(); si != iStats.end(); si++)
	{
		if (si->second == MC_NORMAL_COMPLETION)
		{
			oInputMask += si->first;
		}

		if (si->second == MC_NORMAL_COMPLETION || si->second == MC_NULL_VALUE)
		{
			oOutputMask += si->first;
		}
	}
	
	return;
}

//-----------------------------------------------------------------------------------------------------
//
CFind::CFind(DiCOMConnectionInfo& connectInfo, int iMessageID):
		RTVDiCOMService(connectInfo, iMessageID)
{
	m_processorName = "CFind";
	m_cancel = false;
	m_db.InitDatabaseInfo();
	m_numberOfMatches = 0;
	m_rspID = -1;
	
	memset(&m_filter, 0, sizeof m_filter);
}

//-----------------------------------------------------------------------------------------------------
//
int CFind::ThreadFunction(void* data)
{
	int status = -1;
try{ // #11  2012/03/23 K.Ko
		 
	// prepare COM for ado ADO database call, because it will run in a different thread
	AqCOMThreadInit comInitGuard;

	switch(m_queryLevel)
	{
		case kPatientLevel:
			status = m_db.GetPatientList(m_patientData, &m_filter);	
			break;
		case kStudyLevel:
			{//#60 2013/07/03
				//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8_return_query
				bool bAlwaysUTF8 = gConfig.m_listFromSqlAlwaysUTF8 != 0;
				if(gConfig.m_SqlOrderByStudyDate == 0){
					status = m_db.GetStudyList(m_studyData, &m_filter, gConfig.m_SqlTopNumber/*#60*/, false/*iSort*/, bAlwaysUTF8);
				}else{
					status = m_db.GetStudyListEx(m_studyData, &m_filter, gConfig.m_SqlTopNumber, gConfig.m_SqlOrderByStudyDate, bAlwaysUTF8);
				}
				//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8_return_query
				for (int i = 0; i < m_studyData.size(); i++){
					strncpy(m_studyData[i].m_characterSet, "ISO_IR 192", sizeof(m_studyData[i].m_characterSet));
				}
			}
			break;
		case kSeriesLevel:
		{
			//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8_return_query
			bool bAlwaysUTF8 = gConfig.m_listFromSqlAlwaysUTF8 != 0;
			status = m_db.GetSeriesList(m_seriesData, &m_filter, false/*iSort*/, bAlwaysUTF8);
			if (bAlwaysUTF8){
				//set  characterSet in BuildResponseMessage
			}
		}
			break;
		case kInstanceLevel:
			status = m_db.GetInstanceList(m_imageData, &m_filter, m_filter.m_seriesInstanceUID);	
			break;
	}
}catch(...) // #11  2012/03/23 K.Ko
{
	 
	LogMessage("ERROR:[C%08d]  : [Exception] CFind::ThreadFunction \n", 
			DicomServError_Exception);
	FlushLog();
	status = MC_ERROR;
}
	return status;
}

//-----------------------------------------------------------------------------------------------------
//
void CFind::BuildResponseMessage(int i, int iMsgID, long iMask)
{
try{ // #11  2012/03/23 K.Ko
	
	switch(m_queryLevel)
	{
		case kPatientLevel:
			if (iMask & kPatientsName)					 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENTS_NAME, m_patientData[i].m_patientsName);
			if (iMask & kPatientID)						 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENT_ID, m_patientData[i].m_patientID);
			if (iMask & kPatientsBirthDate)				 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENTS_BIRTH_DATE, m_patientData[i].m_patientsBirthDate);
			if (iMask & kPatientsSex)					 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENTS_SEX, m_patientData[i].m_patientsSex);
			break;
		case kStudyLevel:
			if (iMask & kPatientsName)					 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENTS_NAME, m_studyData[i].m_patientsName);
			if (iMask & kPatientID)						 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENT_ID, m_studyData[i].m_patientID);
			if (iMask & kPatientsBirthDate)				 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENTS_BIRTH_DATE, m_studyData[i].m_patientsBirthDate);
			if (iMask & kPatientsSex)					 MC_Set_Value_From_String (iMsgID, MC_ATT_PATIENTS_SEX, m_studyData[i].m_patientsSex);
			if (iMask & kStudyDate)						 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_DATE, m_studyData[i].m_studyDate);
			if (iMask & kStudyTime)						 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_TIME, m_studyData[i].m_studyTime);
			if (iMask & kAccessionNumber)				 MC_Set_Value_From_String (iMsgID, MC_ATT_ACCESSION_NUMBER, m_studyData[i].m_accessionNumber);
			if (iMask & kStudyID)						 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_ID,	m_studyData[i].m_studyID);
			if (iMask & kStudyInstanceUID)				 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_INSTANCE_UID, m_studyData[i].m_studyInstanceUID);
			if (iMask & kModalitiesInStudy)				 MC_Set_Value_From_String (iMsgID, MC_ATT_MODALITIES_IN_STUDY, m_studyData[i].m_modalitiesInStudy);
			if (iMask & kReferringPhysiciansName)		 MC_Set_Value_From_String (iMsgID, MC_ATT_REFERRING_PHYSICIANS_NAME, m_studyData[i].m_referringPhysiciansName);
			if (iMask & kStudyDescription)				 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_DESCRIPTION, m_studyData[i].m_studyDescription);
			if (iMask & kNumberOfStudyRelatedSeries)	 MC_Set_Value_From_LongInt(iMsgID, MC_ATT_NUMBER_OF_STUDY_RELATED_SERIES, m_studyData[i].m_numberOfStudyRelatedSeries);
			if (iMask & kNumberOfStudyRelatedInstances)  MC_Set_Value_From_LongInt(iMsgID, MC_ATT_NUMBER_OF_STUDY_RELATED_INSTANCES, m_studyData[i].m_numberOfStudyRelatedInstances);
			if (iMask & kCharacterSet)					 MC_Set_Value_From_String (iMsgID, MC_ATT_SPECIFIC_CHARACTER_SET, m_studyData[i].m_characterSet);
			break;
		case kSeriesLevel:
			if (iMask & kStudyInstanceUID)				 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_INSTANCE_UID, m_seriesData[i].m_studyInstanceUID);
			if (iMask & kSeriesInstanceUID)				 MC_Set_Value_From_String (iMsgID, MC_ATT_SERIES_INSTANCE_UID, m_seriesData[i].m_seriesInstanceUID);
			if (iMask & kSeriesNumber)					 MC_Set_Value_From_LongInt (iMsgID, MC_ATT_SERIES_NUMBER, m_seriesData[i].m_seriesNumber);
			if (iMask & kSeriesDescription)				 MC_Set_Value_From_String (iMsgID, MC_ATT_SERIES_DESCRIPTION, m_seriesData[i].m_seriesDescription);
			if (iMask & kModality)						 MC_Set_Value_From_String (iMsgID, MC_ATT_MODALITY, m_seriesData[i].m_modality);
			if (iMask & kNumberOfSeriesRelatedInstances) MC_Set_Value_From_LongInt(iMsgID, MC_ATT_NUMBER_OF_SERIES_RELATED_INSTANCES, m_seriesData[i].m_numberOfSeriesRelatedInstances);
			// 06/04/2003 added bodypartExamined
			if (iMask & kBodyPartExamined)				 MC_Set_Value_From_String( iMsgID, MC_ATT_BODY_PART_EXAMINED,m_seriesData[i].m_bodyPartExamined);
			//long	m_NumberOfSeriesRelatedInstances;
			//char	m_ViewPosition[ cSizeCS+1 ];				// 32
			if (iMask & kSeriesDate)					 MC_Set_Value_From_String(iMsgID, MC_ATT_SERIES_DATE, m_seriesData[i].m_seriesDate);
			if (iMask & kSeriesTime)					 MC_Set_Value_From_String(iMsgID, MC_ATT_SERIES_TIME, m_seriesData[i].m_seriesTime);
			if (gConfig.m_listFromSqlAlwaysUTF8!=0){//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8_return_query
				MC_Set_Value_From_String(iMsgID, MC_ATT_SPECIFIC_CHARACTER_SET, "ISO_IR 192");
			}
			break;
		case kInstanceLevel:
			if (iMask & kStudyInstanceUID)				 MC_Set_Value_From_String (iMsgID, MC_ATT_STUDY_INSTANCE_UID, m_imageData[i].m_studyInstanceUID);
			if (iMask & kSeriesInstanceUID)				 MC_Set_Value_From_String (iMsgID, MC_ATT_SERIES_INSTANCE_UID, m_imageData[i].m_seriesInstanceUID);
			if (iMask & kSOPInstanceUID)				 MC_Set_Value_From_String (iMsgID, MC_ATT_SOP_INSTANCE_UID, m_imageData[i].m_SOPInstanceUID);
			//char	m_sOPClassUID[ cSizeUI+1 ];					// 80
			if (iMask & kInstanceNumber)				 MC_Set_Value_From_LongInt (iMsgID, MC_ATT_INSTANCE_NUMBER, m_imageData[i].m_instanceNumber);
			//char	m_ImageType[ (cSizeCS*4)+1 ];				// 80
			break;
	}
}catch(...) // #11  2012/03/23 K.Ko
{
	 
	LogMessage("ERROR:[C%08d]  : [Exception] CFind::BuildResponseMessage \n", 
			DicomServError_Exception);
	FlushLog();
 
}
	return;
}

//-----------------------------------------------------------------------------------------------------
//
void CFind::Print(int i, int iMsgID, long iMask)
{
	
	switch(m_queryLevel)
	{
		case kPatientLevel:
			if (iMask & kPatientsName)					 fprintf(stderr,"kPatientsName = %s\n", m_patientData[i].m_patientsName);
			if (iMask & kPatientID)						 fprintf(stderr,"kPatientID = %s\n", m_patientData[i].m_patientID);
			if (iMask & kPatientsBirthDate)				 fprintf(stderr,"kPatientsBirthDate = %s\n", m_patientData[i].m_patientsBirthDate);
			if (iMask & kPatientsSex)					 fprintf(stderr,"kPatientsSex = %s\n", m_patientData[i].m_patientsSex);
			break;
		case kStudyLevel:
			{
			DICOMStudy& thisStudy = m_studyData[i];
			if (iMask & kPatientsName)					 fprintf(stderr,"kPatientsName = %s\n", thisStudy.m_patientsName);
			if (iMask & kPatientID)						 fprintf(stderr,"kPatientID = %s\n", thisStudy.m_patientID);
			if (iMask & kPatientsBirthDate)				 fprintf(stderr,"kPatientsBirthDate = %s\n", thisStudy.m_patientsBirthDate);
			if (iMask & kPatientsSex)					 fprintf(stderr,"kPatientsSex = %s\n", thisStudy.m_patientsSex);
			if (iMask & kStudyDate)						 fprintf(stderr,"kStudyDate = %s\n", thisStudy.m_studyDate);
			if (iMask & kStudyTime)						 fprintf(stderr,"kStudyTime = %s\n", thisStudy.m_studyTime);
			if (iMask & kAccessionNumber)				 fprintf(stderr,"kAccessionNumber = %s\n", thisStudy.m_accessionNumber);
			if (iMask & kStudyID)						 fprintf(stderr,"kStudyID = %s\n", thisStudy.m_studyID);
			if (iMask & kStudyInstanceUID)				 fprintf(stderr,"kStudyInstanceUID = %s\n", thisStudy.m_studyInstanceUID);
			if (iMask & kModalitiesInStudy)				 fprintf(stderr,"kModalitiesInStudy = %s\n", thisStudy.m_modalitiesInStudy);
			if (iMask & kReferringPhysiciansName)		 fprintf(stderr,"kReferringPhysiciansName = %s\n", thisStudy.m_referringPhysiciansName);
			if (iMask & kStudyDescription)				 fprintf(stderr,"kStudyDescription = %s\n", thisStudy.m_studyDescription);
			if (iMask & kNumberOfStudyRelatedSeries)	 fprintf(stderr,"kNumberOfStudyRelatedSeries = %d\n", thisStudy.m_numberOfStudyRelatedSeries);
			if (iMask & kNumberOfStudyRelatedInstances)  fprintf(stderr,"kNumberOfStudyRelatedInstances = %d\n", thisStudy.m_numberOfStudyRelatedInstances);
			if (iMask & kCharacterSet)					 fprintf(stderr,"kCharacterSet = %s\n", thisStudy.m_characterSet);
			}
			break;
		case kSeriesLevel:
			if (iMask & kStudyInstanceUID)				 fprintf(stderr,"kStudyInstanceUID = %s\n", m_seriesData[i].m_studyInstanceUID);
			if (iMask & kSeriesInstanceUID)				 fprintf(stderr,"kSeriesInstanceUID = %s\n", m_seriesData[i].m_seriesInstanceUID);
			if (iMask & kSeriesNumber)					 fprintf(stderr,"kSeriesNumber = %d\n", m_seriesData[i].m_seriesNumber);
			if (iMask & kSeriesDescription)				 fprintf(stderr,"kSeriesDescription = %s\n", m_seriesData[i].m_seriesDescription);
			if (iMask & kModality)						 fprintf(stderr,"kModality = %s\n", m_seriesData[i].m_modality);
			if (iMask & kNumberOfSeriesRelatedInstances) fprintf(stderr,"kNumberOfSeriesRelatedInstances = %d\n", m_seriesData[i].m_numberOfSeriesRelatedInstances);
			// 06/04/2003 added bodypartExamined
			if (iMask & kBodyPartExamined)				 fprintf(stderr,"kBodyPartExamined = %s\n",m_seriesData[i].m_bodyPartExamined);
			//long	m_NumberOfSeriesRelatedInstances;
			//char	m_ViewPosition[ cSizeCS+1 ];				// 32
			if (iMask & kSeriesDate)					 fprintf(stderr,"kSeriesDate = %s\n", m_seriesData[i].m_seriesDate);
			break;
		case kInstanceLevel:
			if (iMask & kStudyInstanceUID)				 fprintf(stderr,"kStudyInstanceUID = %s\n", m_imageData[i].m_studyInstanceUID);
			if (iMask & kSeriesInstanceUID)				 fprintf(stderr,"kSeriesInstanceUID = %s\n", m_imageData[i].m_seriesInstanceUID);
			if (iMask & kSOPInstanceUID)				 fprintf(stderr,"kSOPInstanceUID = %s\n", m_imageData[i].m_SOPInstanceUID);
			//char	m_sOPClassUID[ cSizeUI+1 ];					// 80
			if (iMask & kInstanceNumber)				 fprintf(stderr,"kInstanceNumber = %d\n", m_imageData[i].m_instanceNumber);
			//char	m_ImageType[ (cSizeCS*4)+1 ];				// 80
			break;
	}
}

//-----------------------------------------------------------------------------------------------------
//
int CFind::Process()
{
	MC_STATUS	mcStatus;

	//	Used for filters and output masks
	statMap		stats;
	long		inMask = 0;
	long		outMask = 0;

	//	C-FIND association
	int			associationID = m_connectInfo.AssociationID;

	//	Message ID's for Merge Message objects
	int			msgID = m_messageID;			//	Inbound C-FIND-RQ message we are servicing
	int			cFindCancelMessageID = -1;			//	Used to check for inbound C-CANCEL-FIND-RQ messages

	char		queryLevelStr[64];
	std::vector<DICOMSeries> tmpSeriesData;

	 

	//	Message to be used in responses
	mcStatus = MC_Open_Empty_Message(&m_rspID);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
 		return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "on MC_Open_Empty_Message()");
	}

	//	Get m_serviceName so we can send it out in response messages
	mcStatus = MC_Get_Message_Service(msgID, &m_serviceName, &m_command);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		LogMessage("ERROR: (%d) - CFind::Process() - DcmLib error (%d,%s) - Couldn't get Message Service\n", associationID, mcStatus, MC_Error_Message(mcStatus));
		return mcStatus;
	}

	//	See if it's a local request
	int remoteHostPortNumber = 104; 
	int localRequest = !strcmp(TRPlatform::GetIPAddressString(), m_connectInfo.RemoteIPAddress);
	char  remoteIPAddress[66];
	strcpy(remoteIPAddress, m_connectInfo.RemoteIPAddress);

	//	It's not local - need to verify permission
	if (!localRequest)
	{	
		ApplicationEntity AEInfo;
		ASTRNCPY(AEInfo.m_AETitle, m_connectInfo.RemoteApplicationTitle);
		ASTRNCPY(AEInfo.m_IPAddress, remoteIPAddress);
		AEInfo.m_port = gConfig.m_port;

		//	-- 08/06/02	Not checking the port for now, because it creates a dependancy
		//		with the listening port setting in the config file
		if(!m_db.IsRetrieveAE (AEInfo))
		{
			char msg[64];
			_snprintf(msg, sizeof msg, "Invalid remote AE = %s", m_connectInfo.RemoteApplicationTitle);
 			return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, msg);
		}
	}

	//	PATIENT || STUDY || SERIES || IMAGE
	mcStatus = MC_Get_Value_To_String(msgID, MC_ATT_QUERY_RETRIEVE_LEVEL, 64, queryLevelStr);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
 		return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "Couldn't get Query Level");
	}
	LogMessage(kDebug, "TRANS: (%d) - CFind::Process() - Query Level = %s\n", associationID, queryLevelStr);


	//#69 2013/09/04 K.Ko
	/*
	*  xxx\xxx\xxx　を正しく読むため
	*/
	CPxDicomImage dicom_image_temp(msgID,0);//#91 2017/01/12 N.Furutsuki
	dicom_image_temp.HandoverID();//iMsgID に対して、MC_Free_Message しない為。


	//	Extract the inbound attributes.  Attributes with a value represent constraints.  
	//	Those that are null are requests for return info.
	if (!strcmp(queryLevelStr, "PATIENT"))
	{
	//	stats[kPatientsName] =						MC_Get_Value_To_String (msgID, MC_ATT_PATIENTS_NAME, kVR_PN, m_filter.m_patientsName);
		//#69 2013/09/04 K.Ko
		stats[kPatientsName] =						(MC_STATUS)dicom_image_temp.GetValue(msgID, MC_ATT_PATIENTS_NAME, m_filter.m_patientsName, kVR_PN);

		stats[kPatientID] =							MC_Get_Value_To_String (msgID, MC_ATT_PATIENT_ID, kVR_LO, m_filter.m_patientID);
		stats[kPatientsBirthDate] =					MC_Get_Value_To_String (msgID, MC_ATT_PATIENTS_BIRTH_DATE, kVR_DA, m_filter.m_patientsBirthDate);
		stats[kPatientsSex] =						MC_Get_Value_To_String (msgID, MC_ATT_PATIENTS_SEX, kVR_CS, m_filter.m_patientsSex);

		SetMasks(stats, inMask, outMask); 
		m_queryLevel = kPatientLevel;
	} else if (!strcmp(queryLevelStr, "STUDY"))
	{
//		stats[kPatientsName] =						MC_Get_Value_To_String (msgID, MC_ATT_PATIENTS_NAME, kVR_LO, m_filter.m_patientsName);
		//#69 2013/09/04 K.Ko
		stats[kPatientsName] =						(MC_STATUS)dicom_image_temp.GetValue(msgID, MC_ATT_PATIENTS_NAME, m_filter.m_patientsName,kVR_PN);
		
		stats[kPatientsSex] =						MC_Get_Value_To_String (msgID, MC_ATT_PATIENTS_SEX, kVR_LO, m_filter.m_patientsSex);
		stats[kPatientsBirthDate] =					MC_Get_Value_To_String (msgID, MC_ATT_PATIENTS_BIRTH_DATE, kVR_LO, m_filter.m_patientsBirthDate);
		stats[kPatientID] =							MC_Get_Value_To_String (msgID, MC_ATT_PATIENT_ID, kVR_LO, m_filter.m_patientID);
		stats[kStudyDate] =							MC_Get_Value_To_String (msgID, MC_ATT_STUDY_DATE, kVR_DA, m_filter.m_studyDate);
		stats[kStudyTime] =							MC_Get_Value_To_String (msgID, MC_ATT_STUDY_TIME, kVR_TM, m_filter.m_studyTime);
		stats[kAccessionNumber] =					MC_Get_Value_To_String (msgID, MC_ATT_ACCESSION_NUMBER, kVR_SH, m_filter.m_accessionNumber);
		stats[kStudyID] =							MC_Get_Value_To_String (msgID, MC_ATT_STUDY_ID, kVR_SH,	m_filter.m_studyID);
		stats[kStudyInstanceUID] =					MC_Get_Value_To_String (msgID, MC_ATT_STUDY_INSTANCE_UID, kVR_UI, m_filter.m_studyInstanceUID);
		stats[kModalitiesInStudy] =					MC_Get_Value_To_String (msgID, MC_ATT_MODALITIES_IN_STUDY, kVR_CS, m_filter.m_modalitiesInStudy);
		stats[kReferringPhysiciansName] =			MC_Get_Value_To_String (msgID, MC_ATT_REFERRING_PHYSICIANS_NAME, kVR_PN, m_filter.m_referringPhysiciansName);
	//	stats[kStudyDescription] =					MC_Get_Value_To_String (msgID, MC_ATT_STUDY_DESCRIPTION, kVR_LO, m_filter.m_studyDescription);
		//#69 2013/09/04 K.Ko
		stats[kStudyDescription] =					(MC_STATUS)dicom_image_temp.GetValue(msgID, MC_ATT_STUDY_DESCRIPTION, m_filter.m_studyDescription, kVR_LO);

		stats[kNumberOfStudyRelatedSeries] =		MC_Get_Value_To_LongInt(msgID, MC_ATT_NUMBER_OF_STUDY_RELATED_SERIES, &m_filter.m_numberOfStudyRelatedSeries);
		stats[kNumberOfStudyRelatedInstances] =		MC_Get_Value_To_LongInt(msgID, MC_ATT_NUMBER_OF_STUDY_RELATED_INSTANCES, &m_filter.m_numberOfStudyRelatedInstances);
		stats[kPatientsAge] =						MC_Get_Value_To_LongInt(msgID, MC_ATT_PATIENTS_AGE, &m_filter.m_patientsAge);
#if 0
		stats[kCharacterSet] =						MC_Get_Value_To_String (msgID, MC_ATT_SPECIFIC_CHARACTER_SET, kVR_CS, m_filter.m_characterSet);
#else
		{//#137 2021/01/12 N.Furutsuki
			std::string get_char_sets_temp = CPxDicomMessage::GetCharacterSets(msgID);
			stats[kCharacterSet] = (get_char_sets_temp.size() < 1) ? MC_ERROR : MC_NORMAL_COMPLETION;
			if (get_char_sets_temp.size() < 1){
				ASTRNCPY(m_filter.m_characterSet, "");
			}
			else{
				ASTRNCPY(m_filter.m_characterSet, get_char_sets_temp.c_str());
			}
		}
#endif

		SetMasks(stats, inMask, outMask);
		m_queryLevel = kStudyLevel;

		if (m_filter.m_numberOfStudyRelatedSeries < 0)
		{
			inMask &= ~kNumberOfStudyRelatedSeries;
		}
		if (m_filter.m_numberOfStudyRelatedInstances < 0)
		{
			inMask &= ~kNumberOfStudyRelatedInstances;
		}
	}
	else if (!strcmp(queryLevelStr, "SERIES"))
	{
		stats[kStudyInstanceUID] = MC_Get_Value_To_String(msgID, MC_ATT_STUDY_INSTANCE_UID, kVR_UI, m_filter.m_studyInstanceUID);
		if (gConfig.m_requireHigherLevelKeys && stats[kStudyInstanceUID] != MC_NORMAL_COMPLETION)
			return HandleError(mcStatus, C_FIND_FAILURE_INVALID_DATASET, "SERIES Level Query must include StudyInstanceUID");

		stats[kModality] = MC_Get_Value_To_String(msgID, MC_ATT_MODALITY, kVR_CS, m_filter.m_modality);
		stats[kSeriesNumber] = MC_Get_Value_To_LongInt(msgID, MC_ATT_SERIES_NUMBER, &m_filter.m_seriesNumber);
		//	stats[kSeriesDescription] =					MC_Get_Value_To_String (msgID, MC_ATT_SERIES_DESCRIPTION, kVR_LO, m_filter.m_seriesDescription);
		//#69 2013/09/04 K.Ko
		stats[kSeriesDescription] = (MC_STATUS)dicom_image_temp.GetValue(msgID, MC_ATT_SERIES_DESCRIPTION, m_filter.m_seriesDescription, kVR_LO);

		stats[kSeriesInstanceUID] = MC_Get_Value_To_String(msgID, MC_ATT_SERIES_INSTANCE_UID, kVR_UI, m_filter.m_seriesInstanceUID);
#if 0
		stats[kCharacterSet] =						MC_Get_Value_To_String (msgID, MC_ATT_SPECIFIC_CHARACTER_SET, kVR_CS, m_filter.m_characterSet);
#else
		{//#137 2021/01/12 N.Furutsuki
			std::string get_char_sets_temp = CPxDicomMessage::GetCharacterSets(msgID);
			stats[kCharacterSet] = (get_char_sets_temp.size() < 1) ? MC_ERROR : MC_NORMAL_COMPLETION;
			if (get_char_sets_temp.size() < 1){
				ASTRNCPY(m_filter.m_characterSet, "");
			}
			else{
				ASTRNCPY(m_filter.m_characterSet, get_char_sets_temp.c_str());
			}
		}
#endif
		stats[kNumberOfSeriesRelatedInstances] =	MC_Get_Value_To_LongInt (msgID, MC_ATT_NUMBER_OF_SERIES_RELATED_INSTANCES, &m_filter.m_numberOfSeriesRelatedInstances);
		stats[kBodyPartExamined]			=		MC_Get_Value_To_String (msgID, MC_ATT_BODY_PART_EXAMINED, sizeof  m_filter.m_bodyPartExamined,m_filter.m_bodyPartExamined);
		// 2006.02.01 for series date&time
		stats[kSeriesDate]						=	MC_Get_Value_To_String (msgID, MC_ATT_SERIES_DATE, kVR_DA, m_filter.m_seriesDate);
		stats[kSeriesTime] =						MC_Get_Value_To_String (msgID, MC_ATT_SERIES_TIME, kVR_TM, m_filter.m_studyTime);
		SetMasks(stats, inMask, outMask);
		m_queryLevel = kSeriesLevel;

		if (m_filter.m_numberOfSeriesRelatedInstances < 0)
		{
			inMask &= ~kNumberOfSeriesRelatedInstances;
		}
	} else if (!strcmp(queryLevelStr, "IMAGE"))
	{
		stats[kStudyInstanceUID] =					MC_Get_Value_To_String (msgID, MC_ATT_STUDY_INSTANCE_UID, kVR_UI, m_filter.m_studyInstanceUID);
		stats[kSeriesInstanceUID] =					MC_Get_Value_To_String (msgID, MC_ATT_SERIES_INSTANCE_UID, kVR_UI, m_filter.m_seriesInstanceUID);

		if (gConfig.m_requireHigherLevelKeys && (stats[kStudyInstanceUID] != MC_NORMAL_COMPLETION || stats[kSeriesInstanceUID] != MC_NORMAL_COMPLETION))
			return HandleError(mcStatus, C_FIND_FAILURE_INVALID_DATASET, "IMAGE Level Query must include both StudyInstanceUID and SeriesInstanceUID");

		stats[kSOPInstanceUID] =					MC_Get_Value_To_String (msgID, MC_ATT_SOP_INSTANCE_UID, kVR_UI, m_filter.m_SOPInstanceUID);
		long tmpInt = 0L;
		stats[kInstanceNumber] =					MC_Get_Value_To_LongInt (msgID, MC_ATT_INSTANCE_NUMBER, &tmpInt);
		m_filter.m_instanceNumber = tmpInt;

		SetMasks(stats, inMask, outMask);
		m_queryLevel = kInstanceLevel;
	}
	
	//m_filter.m_mask = inMask;

	//	Do the lookup
	iRTVThreadRunClassFunction lookupProcess(this);
	iRTVThread lookupThread(&lookupProcess);

	//	Wait until the lookup is completed or cancelled by an incoming C_CANCEL_FIND_RQ message
	int processStatus = lookupProcess.GetProcessStatus();
	char* tmpServiceName;
	unsigned short	tmpCommand;
	while(processStatus != iRTVThreadProcess::kProcessTerminated && !m_cancel)
	{
		//	Poll for C-CANCEL-FIND message - no timeout
		mcStatus = MC_Read_Message (associationID, 0, &cFindCancelMessageID, &tmpServiceName, &tmpCommand);
		MC_Free_Message (&cFindCancelMessageID);
		if (mcStatus == MC_NORMAL_COMPLETION && tmpCommand == C_CANCEL_FIND_RQ)
		{

			if (gConfig.m_messageDump)
			{
				TRDICOMUtil::DumpMessage(cFindCancelMessageID, tmpServiceName, tmpCommand);
			}

			m_cancel = true;
			MC_Send_Response_Message(associationID, C_FIND_CANCEL_REQUEST_RECEIVED, m_rspID);
			if(gConfig.m_traceDicomLevel >=1){ //#693
				LogMessage("WARNING:[C%08d] (%d) - CFind::Process() - Received C-CANCEL-FIND-RQ\n",DicomServError_CFindError, associationID);
			}else{
				LogMessage(kDebug,  "DEBUG:[C%08d] (%d) - CFind::Process() - Received C-CANCEL-FIND-RQ\n",DicomServError_CFindError, associationID);
			}
			FlushLog();
			// Do db cancel to let CFindDBLookup return
			m_db.SetCancelFlag(1);
			lookupProcess.RequestTermination(1);
			lookupProcess.WaitForProcessToEnd(4000);

			return mcStatus;
		}

		//	See if db lookup is done yet
		processStatus = lookupProcess.GetProcessStatus();

		//	If this delay is much longer, normal queries are held up.
		Sleep(100);
	}

	if(m_cancel)
		return 0;
	
	//	The database lookup completed - now compose and send response a response message for each match

#if 0
	//	For instance level queries, need to get a series list to obtain the StudyInstanceUID (PS3.4 - Annex C.4.1.3.1.1)
	int dbSeriesListStatus = 0;
	if (m_queryLevel == kInstanceLevel)
	{
		DICOMData tmpFilter;

		ASTRNCPY(tmpFilter.m_seriesInstanceUID, m_filter.m_seriesInstanceUID);
		dbSeriesListStatus = m_db.GetSeriesList(tmpSeriesData, &tmpFilter);
		if (dbSeriesListStatus != 0)
		{
 			return HandleError(mcStatus, C_FIND_FAILURE_REFUSED_NO_RESOURCES, "at Database lookup for IMAGE LEVEL GetSeriesList()");
		}
	}
#endif

	LogMessage(kDebug,  "DEBUG: (%d) - CFind::Process() - Inbound C-FIND Request contains:\n", associationID);

	//	How many matches - i.e. how many response messages to send?
	switch(m_queryLevel)
	{
		case kPatientLevel:
			m_numberOfMatches = m_patientData.size();	
			break;
		case kStudyLevel:
			m_numberOfMatches = m_studyData.size();		
			break;
		case kSeriesLevel:
			m_numberOfMatches = m_seriesData.size();		
			break;
		case kInstanceLevel:
			m_numberOfMatches = m_imageData.size();		
			break;
	}

	//	Prepare and send one response for each match
	int i;
	char* serviceName;
	unsigned short	command;

	for(i=0; i<m_numberOfMatches; i++)
	{	
		//GL database will set the study UID
		//	ASSERT: No cancel request was received - proceed with transmission of query results	
		//	Copy the StudyInstanceUID we got earlier if the query is at the IMAGE level
		//if (m_queryLevel == kInstanceLevel && dbSeriesListStatus == 0)
		//{
		//	ASTRNCPY(m_imageData[i].m_studyInstanceUID, tmpSeriesData[0].m_studyInstanceUID);
		//}

		//	Simulate non-compliant DICOM behavior.
		if (!gConfig.m_excludeQRLevelFromCFindRSP)
		{
			mcStatus = MC_Set_Value_From_String(m_rspID, MC_ATT_QUERY_RETRIEVE_LEVEL, queryLevelStr);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
 				return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "Couldn't set Query Level");
			}
		}

		//	-- - 2006.04.20 - Was set to AUTOVOX...this was not right...in response to bug reported by japan team.
		mcStatus = MC_Set_Value_From_String(m_rspID, MC_ATT_RETRIEVE_AE_TITLE, TRDICOMUtil::CalculateInboundLocalAETitle().c_str());
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
 			return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "Couldn't set Retrieve AE Title");
		}

		//	Populate response message with results from the database lookup
  		BuildResponseMessage(i, m_rspID, outMask);

		//	Print out the contents of the response message for debugging
		#ifdef _DEBUG
#if 0
		printf("\n--------------\n");
		printf("Query Level = %s\n\n", queryLevelStr);
		Print(i, m_rspID, outMask);
		printf("--------------\n");
#endif
		#endif
		
		//	Send out the fully populated response
		mcStatus = MC_Set_Service_Command(m_rspID, m_serviceName, C_FIND_RSP);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
 			return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "on MC_Set_Service_Command()");
		}

		if (gConfig.m_messageDump)
		{
			TRDICOMUtil::DumpMessage(m_rspID, m_serviceName, C_FIND_RSP);
		}

		mcStatus = MC_Send_Response_Message(associationID, C_FIND_PENDING, m_rspID);	
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
 			return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "on MC_Send_Response_Message()");
		}

		//	Reset the Merge message object for re-use
		mcStatus = MC_Empty_Message(m_rspID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
 			return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "on MC_Empty_Message()");
		}

#if 1  // K.Ko
		if (!(i % 15))
		{
			//	Poll for C-CANCEL-FIND message
			mcStatus = MC_Read_Message (associationID, 0, &cFindCancelMessageID, &serviceName, &command);

			//	Some error happened while reading
			if (mcStatus != MC_NORMAL_COMPLETION && mcStatus != MC_TIMEOUT)	
			{
				return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "while polling for C-CANCEL-FIND-RQ");
			} 
			//	We got a message
			else if (mcStatus == MC_NORMAL_COMPLETION)	
			{
				if (gConfig.m_messageDump)
				{
					TRDICOMUtil::DumpMessage(cFindCancelMessageID, serviceName, command);
				}

				//	It was a cancel request
				if (m_command == C_CANCEL_FIND_RQ)		
				{
 					return HandleError(mcStatus, C_FIND_CANCEL_REQUEST_RECEIVED, "Received C-CANCEL-FIND-RQ", kDebug);
				}
			}
		}
#endif

	}

	// Prepare and send final response message with status C_FIND_SUCCESS

/*
 *	 Commented this out because it violates the DICOM standard.
 *		Only supposed to send it for status == PENDING.  Didn't find out about it 
 *		until PSP complained about it.
 *
	mcStatus = MC_Set_Value_From_String(m_rspID, MC_ATT_QUERY_RETRIEVE_LEVEL, queryLevelStr);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "Couldn't set Query Level");
	}
*/

	mcStatus = MC_Set_Service_Command(m_rspID, m_serviceName, C_FIND_RSP);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
 		return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "on MC_Set_Service_Command()");
	}

	if (gConfig.m_messageDump)
	{
		TRDICOMUtil::DumpMessage(m_rspID, m_serviceName, C_FIND_RSP);
	}

	mcStatus = MC_Send_Response_Message(associationID, C_FIND_SUCCESS, m_rspID);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
 		return HandleError(mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS, "on MC_Send_Response_Message()");
	}

	return mcStatus;
}

//-----------------------------------------------------------------------------------------------------
//
int CFind::HandleError(MC_STATUS iErrorCode, RESP_STATUS iResponseCode, const char* iErrorMsg, int iDebugLevel)
{
	int	associationID = m_connectInfo.AssociationID;

	LogMessage(kWarning,"WARNING:[C%08d] (%d) - CFind::Process() - DcmLib error (%d,%s) - %s\n",DicomServError_CFindError, associationID, iErrorCode, MC_Error_Message(iErrorCode), iErrorMsg);
	MC_Set_Service_Command(m_rspID, m_serviceName, C_FIND_RSP);
	FlushLog();
	MC_Send_Response_Message(associationID, iResponseCode, m_rspID);
	return iErrorCode;
}

//-----------------------------------------------------------------------------------------------------
//
CFind::~CFind()
{
	MC_Free_Message(&m_rspID);
}

