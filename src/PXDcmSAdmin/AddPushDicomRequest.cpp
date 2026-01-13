#pragma warning (disable: 4503)

#include "PxNetDB.h"

 #include <QCoreApplication>

#include "AddPushDicomRequest.h"
#include "PxQueue.h"

#include "QtHelper.h"

//#25 20012/06/11 K.Ko
#include "JISToSJISMS.h"
void ReformatJapaneseDicom( CPxQueueEntry &entry )
{
	std::string str_temp;
	//m_PatientName
//#142_NoPatientName_NoComment
#if 0
	if(entry.m_extInfo.m_PatientName.size()>0){
		CJISToSJISMS::ConvertJISToSJIS( entry.m_extInfo.m_PatientName, str_temp );
		CJISToSJISMS::ReformatPatientName( str_temp, cPNStandard );
		entry.m_extInfo.m_PatientName	= str_temp;
	}
	//m_Comment
	if(entry.m_extInfo.m_Comment.size()>0){
		CJISToSJISMS::ConvertJISToSJIS( entry.m_extInfo.m_Comment, str_temp );
		CJISToSJISMS::ReformatPatientName( str_temp, cPNStandard );
		entry.m_extInfo.m_Comment		= str_temp;
	}
#endif
}

CAddPushDicomRequest::CAddPushDicomRequest(void)
{
	 
	m_priority = CPxQueueEntry::PxQueuePriority_Default;
	m_loopInterval = 400;

	m_MyID = 1;

	m_DBQueueName = QString2Str(QCoreApplication::applicationDirPath());
	int pos = m_DBQueueName.find_last_of("/");
	m_DBQueueName = m_DBQueueName.substr(0,pos);
	m_DBQueueName = m_DBQueueName +"/data/" ;
	std::string queue_entry_folder = m_DBQueueName +"entry/";
	m_DBQueueName = m_DBQueueName + SQLITE_DB_FILTE;

	//
	CPxQueueProc::setupQueueDBName(m_DBQueueName);
	
	CPxQueueProc::setupQueueEntryFolder(queue_entry_folder);
	
}

CAddPushDicomRequest::~CAddPushDicomRequest(void)
{
}
 
#include "AppComConfiguration.h"
#include "PxDcmsAEManage.h"
inline bool isOutJPEG(const AEItemData *AETitle){
	bool ret_b = false;
	if(!AppComConfiguration::GetJpegGatewayFlag()){
		return false;
	};//#47
	if( (AETitle->m_PortNum == 0)	&&
		(AETitle->m_IP == "0.0.0.0") ){
		ret_b = true;
	}
	return ret_b;
}
//bool CAddPushDicomRequest::pushDICOMStudy(const std::string AETitle,const std::string &studyUID)
//#48
bool CAddPushDicomRequest::pushDICOMStudy(const AEItemData *AETitle,const std::string &studyUID)
{
	 
	CPxWorkQueue QueueProc;
//	QueueProc.setupDBName(m_DBQueueName);
	QueueProc.initRes();

	CPxQueueEntry new_entry;
	//#48
	new_entry.m_cmdID	= isOutJPEG(AETitle) ? CPxQueueEntry::PxQueueCmd_JPEG : CPxQueueEntry::PxQueueCmd_DICOM;
	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Study;
	new_entry.m_StudyInstanceUID	= studyUID;
//	new_entry.m_SeriesInstanceUID	= " ";
	new_entry.m_DestinationAE		= AETitle->m_AETitle;
	new_entry.m_Priority			= m_priority;
	new_entry.m_Status				= 0;//don't care here
	new_entry.m_CreateTime			= time(0);
	new_entry.m_AccessTime			= time(0);

	QueueProc.createJobID(new_entry); //#19 2012/05/21 K.Ko

	///////////
	//extInfo
	{
		CPxDcmDB pxDb;
 
		std::vector<DICOMStudy> oVal;
		DICOMData seriesFilter;

		strcpy(seriesFilter.m_studyInstanceUID,		studyUID.c_str());
		 
		int status = pxDb.GetStudyList(oVal, &seriesFilter);
		if (status != kOK){
			return false;
		}
		if(oVal.size()<1){
			return false;
		}
//		new_entry.m_extInfo.m_PatientName = "";//2023_03_16 oVal[0].m_patientsName;
		new_entry.m_extInfo.m_PatientID		= oVal[0].m_patientID;
	 	new_entry.m_extInfo.m_Date			= oVal[0].m_studyDate;
		if(strlen(oVal[0].m_studyTime)>0){//#79 2014/08/21 K.Ko
			new_entry.m_extInfo.m_Date		= new_entry.m_extInfo.m_Date + std::string(" ") + oVal[0].m_studyTime ;
		}
		new_entry.m_extInfo.m_BirthDate		= oVal[0].m_patientsBirthDate; //0212/06/15 added K.Ko
//		new_entry.m_extInfo.m_Comment		= oVal[0].m_studyDescription;
	}

	ReformatJapaneseDicom(new_entry);//#25 20012/06/11 K.Ko
	QueueProc.addQueue(new_entry);
	return true;
}
 
//bool CAddPushDicomRequest::pushDICOMSeries(const std::string AETitle,const std::string &studyUID,const std::string &seriesUID)
//#48
bool CAddPushDicomRequest::pushDICOMSeries(const AEItemData *AETitle,const std::string &studyUID,const std::string &seriesUID)
{
	CPxWorkQueue QueueProc;
//	QueueProc.setupDBName(m_DBQueueName);
	QueueProc.initRes();

	CPxQueueEntry new_entry;
	//#48
	new_entry.m_cmdID	= isOutJPEG(AETitle) ? CPxQueueEntry::PxQueueCmd_JPEG : CPxQueueEntry::PxQueueCmd_DICOM;

	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Series;
	new_entry.m_StudyInstanceUID	= studyUID;
	new_entry.m_SeriesInstanceUID	= seriesUID;
	new_entry.m_DestinationAE		= AETitle->m_AETitle;
	new_entry.m_Priority			= m_priority;
	new_entry.m_Status				= 0;//don't care here
	new_entry.m_CreateTime			= time(0);
	new_entry.m_AccessTime			= time(0);

	QueueProc.createJobID(new_entry); //#19 2012/05/21 K.Ko

	///////////
	//extInfo
	{
		CPxDcmDB pxDb;
	 
	 	std::vector<SeriesDisplayInfo> oVal;
 
		DICOMData seriesFilter;

		strcpy(seriesFilter.m_studyInstanceUID,		studyUID.c_str());
		strcpy(seriesFilter.m_seriesInstanceUID,	seriesUID.c_str());
		 
	 
		int status = pxDb.GetSeriesDisplayInfoOnServer(oVal, &seriesFilter);
	 
		if (status != kOK){
			return false;
		}
		if(oVal.size()<1){
			return false;
		}
//		new_entry.m_extInfo.m_PatientName	= oVal[0].m_patientsName;
		new_entry.m_extInfo.m_PatientID		= oVal[0].m_patientID;
//		new_entry.m_extInfo.m_Date			= oVal[0].m_seriesDate + std::string(" ") + oVal[0].m_seriesTime;
//#79 2014/08/21 K.Ko
		new_entry.m_extInfo.m_Date			=	  std::string(strlen(oVal[0].m_seriesDate)==0 ? oVal[0].m_studyDate :  oVal[0].m_seriesDate)
												+ std::string(" ") 
												+ std::string(strlen(oVal[0].m_seriesTime)==0 ? oVal[0].m_studyTime : oVal[0].m_seriesTime);
		new_entry.m_extInfo.m_BirthDate		= oVal[0].m_patientsBirthDate; //0212/06/15 added K.Ko
		new_entry.m_extInfo.m_ImagesInSeries= oVal[0].m_numberOfSeriesRelatedInstances;
		new_entry.m_extInfo.m_SeriesNumber	= oVal[0].m_seriesNumber;
	//	new_entry.m_extInfo.m_Comment		= oVal[0].m_seriesDescription;
	}

	ReformatJapaneseDicom(new_entry); //#25 20012/06/11 K.Ko
	QueueProc.addQueue(new_entry);
	return true;
}
	
