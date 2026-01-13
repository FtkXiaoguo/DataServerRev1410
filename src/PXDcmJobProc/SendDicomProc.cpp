/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "SendDicomProc.h"
#include "CStoreSCU.h"

#include <assert.h>
#include <sys/timeb.h>
#include "rtvpoolaccess.h"
#include "AppComUtil.h"
//#include "TRDICOMUtil.h"
#include "Globals.h"

#include "PxQueueWatchProc.h"


#include "StoreSCUSeriesDirMonitor.h"

#include "rtvsutil.h"

#ifdef _PROFILE
#include "ScopeTimer.h"
#endif

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif
 
 

//-----------------------------------------------------------------------------
CSendDicomProc::CSendDicomProc ()
{
	m_cancelFlag = false;//#82 2014/09/29 K.Ko
	 sprintf(m_connectInfo.LocalApplicationTitle,"DcmQueueAE");
	 m_pDcmDB = new CPxDcmDB;
}

//-----------------------------------------------------------------------------
CSendDicomProc::~CSendDicomProc()
{
	 delete m_pDcmDB;
 

}
 
bool CSendDicomProc::initDICOM()
{
	int status;
	if(!CStoreSCU::initDICOM( ))
	{
		gLogger.LogMessage("ERROR: InitialDICOM failed\n");
		gLogger.FlushLog();
		return false;
	}
 

	return true;
}

bool CSendDicomProc::sendStudy(const CPxQueueEntry *entry)
{
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;

	gLogger.LogMessage(kDebug,"CSendDicomProc::sendStudy to [%s], [%s]\n", AE.c_str(),StudyUID.c_str());
	gLogger.FlushLog();

	std::vector<DICOMSeries>  series_list;

	if(!querySeries( StudyUID,  series_list)){
		gLogger.LogMessage(kDebug,"CSendDicomProc::sendStudy querySeries is failed\n");
		gLogger.FlushLog();
	}
	int series_size = series_list.size();
	if(series_size<1){
		gLogger.LogMessage(kDebug,"CSendDicomProc::sendStudy series_list is null\n");
		gLogger.FlushLog();
		return false;
	}
	bool ret_b= true;

	CPxQueueEntry entry_temp = *entry;
	for(int run_i=0;run_i<series_size;run_i++){

		if(m_cancelFlag){//#82 2014/09/29 K.Ko
			gLogger.LogMessage("CSendDicomProc::sendStudy querySeries is canceled\n");
			gLogger.FlushLog();
			break;
		} 

		DICOMSeries item = series_list[run_i];
		std::string SeriesUID = item.m_seriesInstanceUID;
		entry_temp.m_SeriesInstanceUID = SeriesUID;
	//	if(!sendSeries(AE,StudyUID, SeriesUID,run_i==0/*getStudyInfo*/)){
		if(!sendSeries(&entry_temp,run_i==0/*getStudyInfo*/)){
			gLogger.LogMessage(kErrorOnly,"CSendDicomProc::sendStudy series_list[%d] is failed\n",run_i);
			gLogger.FlushLog();
			ret_b = false;
			break;
		}
	}
	return ret_b;
}
//bool CSendDicomProc::sendSeries(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,bool getStudyInfo)

bool CSendDicomProc::sendSeries(const CPxQueueEntry *entry,bool getStudyInfo)
{
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;
	const std::string SeriesUID	= entry->m_SeriesInstanceUID;



	gLogger.LogMessage(kDebug,"CSendDicomProc::sendSeries to [%s], [%s] [%s]\n", AE.c_str(),StudyUID.c_str(),SeriesUID.c_str());
	gLogger.FlushLog();

	bool ret_b = true;
	try{
	//
		std::vector<AppComDevice> raids;
		AppComConfiguration::GetArchiveDevices(AppComConfiguration::gkRAIDType,raids);

	//	std::string series_folder = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom(SeriesUID, StudyUID, 0);
		std::string SOPInstanceUID ;
		int imageNumber;
		std::string series_folder = AppComUtil::getSeriesFolder(StudyUID, SeriesUID,"*.dcm");
		if(series_folder.size()<1){
			gLogger.LogMessage(kErrorOnly,"CSendDicomProc::sendSeries getSeriesFolder error [%s] [%s]\n",
				StudyUID.c_str(),SeriesUID.c_str());
			gLogger.FlushLog();
			throw(-1);//return false;
		}

		CStoreSCUIf *pCStoreSCU_hdr = openCStoreAssociation(AE,SeriesUID);
		if(!pCStoreSCU_hdr){
			gLogger.LogMessage(kDebug,"CSendDicomProc::sendSeries openCStoreAssociation error\n");
			gLogger.FlushLog();
			throw(-1);//return false;
		}
	 
		////////////
	 
#if 1
		pCStoreSCU_hdr->setDicomStudyInfo("",
			//#142_NoPatientName_NoComment
			//entry->m_extInfo.m_PatientName,
			entry->m_extInfo.m_PatientID,entry->m_extInfo.m_BirthDate);
#else
	//	if(gConfig.m_safetyCheck !=0 ){
		if(1){ //for logger the Study information
			//#27 2012/06/14 K.Ko
	 		if(getStudyInfo){
				m_curStudy.clear();
	 			if(StudyUID.size()>0){
					DICOMData tmpFilter;
					ASTRNCPY(tmpFilter.m_studyInstanceUID, StudyUID.c_str());
					 
			 		m_pDcmDB->GetStudyList( m_curStudy, &tmpFilter);
				
	 			}
	 		}

			if(m_curStudy.size()>0){
				DICOMStudy &pCurStudyInfo =  m_curStudy[0] ;
				pCStoreSCU_hdr->setDicomStudyInfo(&pCurStudyInfo);
			} 
		}
#endif
	 
		
		std::vector<DICOMInstance>  image_list;
		queryImages(StudyUID,SeriesUID,image_list);

		int image_size = image_list.size();
		if(image_size<1){
			gLogger.LogMessage(kErrorOnly,"CSendDicomProc::sendSeries image_list is null\n");
			gLogger.FlushLog();
			throw(-1);//return false;
		}
		
		////
		std::string dicom_file_name;
		for(int i=0;i<image_size;i++){
			if(m_cancelFlag){//#82 2014/09/29 K.Ko
				gLogger.LogMessage("CSendDicomProc::sendSeries  is canceled\n");
				gLogger.FlushLog();
				break;
			} 

			DICOMInstance dicom_infor = image_list[i];
			dicom_file_name = AppComUtil::getDicomFileName(series_folder,
											dicom_infor.m_SOPInstanceUID,
											dicom_infor.m_instanceNumber);
			if(!pCStoreSCU_hdr->sendDicomFile(dicom_file_name)){
				gLogger.LogMessage(kDebug,"CSendDicomProc::sendSeries sendDicomFile[%s] error\n",dicom_file_name.c_str());
				gLogger.FlushLog();
				throw(-1);//return false;
			}
		}
		//
		 
		if(!closeCStoreAssociation(pCStoreSCU_hdr)){
			gLogger.LogMessage(kDebug,"CSendDicomProc::sendSeries closeCStoreAssociation error\n");
			gLogger.FlushLog();
			throw(-1);//eturn false;
		}
		 
	}catch(int error_code)
	{
		ret_b = false;
	}
	catch(...)
	{
		ret_b = false;
	}
	//#142_NoPatientName_NoComment
	//gLogger.LogMessage("INFO:[C%08d] send Series [%s] ID[%s] [%d] [%s] to [%s]  - %s\n",DicomJobProcInfor_CMoveInfo,
	//	entry->m_extInfo.m_PatientName.c_str(),entry->m_extInfo.m_PatientID.c_str(),
	gLogger.LogMessage("INFO:[C%08d] send Series ID[%s] [%d] to [%s]  - %s\n", DicomJobProcInfor_CMoveInfo,
		entry->m_extInfo.m_PatientID.c_str(),
	 	entry->m_extInfo.m_SeriesNumber,
	//entry->m_extInfo.m_Comment.c_str(),
							AE.c_str(), 
							ret_b?"Success":"Failed");
	gLogger.FlushLog();

	return ret_b;
}
//bool CSendDicomProc::sendImage(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,const std::string &SOPInstanceUID)

bool CSendDicomProc::sendImage(const CPxQueueEntry *entry)
{
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;
	const std::string SeriesUID	= entry->m_SeriesInstanceUID;
	const std::string SOPInstanceUID	= entry->m_SOPInstanceUID;
 
	gLogger.LogMessage(kDebug,"CSendDicomProc::sendImage to [%s] \n   [%s]   [%s]   [%s]\n", AE.c_str(),
				StudyUID.c_str(),SeriesUID.c_str(),SOPInstanceUID.c_str());
	gLogger.FlushLog();

	std::string series_folder = AppComUtil::getSeriesFolder(StudyUID, SeriesUID,"*.dcm");
	if(series_folder.size()<1){
		gLogger.LogMessage(kErrorOnly,"CSendDicomProc::sendImage getSeriesFolder error [%s] [%s]\n",
			StudyUID.c_str(),SeriesUID.c_str());
		gLogger.FlushLog();
		return false;
	}
 
	////
	CStoreSCUIf *pCStoreSCU_hdr = openCStoreAssociation(AE,SeriesUID);
	if(!pCStoreSCU_hdr){
		gLogger.LogMessage(kDebug,"CSendDicomProc::sendImage openCStoreAssociation error\n");
		gLogger.FlushLog();
		return false;
	}

//	CPxDcmDB pxDb;

#if 1
		pCStoreSCU_hdr->setDicomStudyInfo("",
		//#142_NoPatientName_NoComment
		//	entry->m_extInfo.m_PatientName,
			entry->m_extInfo.m_PatientID,entry->m_extInfo.m_BirthDate);
#else
//	if(gConfig.m_safetyCheck !=0 ){
	if(1){ //for logger the Study information
		//#27 2012/06/14 K.Ko
		
		if(pCStoreSCU_hdr->getDicomStudyInfo() == 0){
			m_curStudy.clear();
			if(StudyUID.size()>0){
				DICOMData tmpFilter;
				ASTRNCPY(tmpFilter.m_studyInstanceUID, StudyUID.c_str());
			//	std::vector<DICOMStudy>  oVal;
				
				m_pDcmDB->GetStudyList( m_curStudy, &tmpFilter);
			 
			}
		}

		if(m_curStudy.size()>0){
			pCStoreSCU_hdr->setDicomStudyInfo(&(m_curStudy[0]));
		}
	}
#endif

	std::string dicom_file_name;
	std::vector<DICOMInstance> dicom_info;
	DICOMData  iFilter;
	iFilter.Clear();
	strcpy(iFilter.m_SOPInstanceUID, SOPInstanceUID.c_str());

	int status = m_pDcmDB->GetInstanceList( dicom_info, &iFilter, SeriesUID.c_str());
 
	if (status == kOK)
	{
		dicom_file_name = AppComUtil::getDicomFileName(series_folder,
										SOPInstanceUID,
										dicom_info[0].m_instanceNumber);
	}

	if(dicom_file_name.size()<1){
		gLogger.LogMessage(kDebug,"CSendDicomProc::sendImage dicom_file_name is null\n");
		gLogger.FlushLog();
		return false;
	}
	///
	if(!pCStoreSCU_hdr->sendDicomFile(dicom_file_name)){
		gLogger.LogMessage(kDebug,"CSendDicomProc::sendImage sendDicomFile[%s] error\n",dicom_file_name.c_str());
		gLogger.FlushLog();
		return false;
	}

	pCStoreSCU_hdr->closeSession(); //１ Ｉｍａｇｅ 使用終了
	////////
	/// do not close association heere
	 

	return true;
}

//bool CSendDicomProc::sendEntryFile(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,
//						const std::string &seriesFolder,const std::vector<std::string> &ImageFileList)
bool CSendDicomProc::sendEntryFile(const CPxQueueEntry *entry,const std::string &seriesFolder,const std::vector<std::string> &ImageFileList)
{
//	const int queueID			= entry->m_QueueID;
//	const std::string JobID		= entry->m_JobID;
//	const std::string SOPID		= entry->m_SOPInstanceUID;
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;
	const std::string SeriesUID	= entry->m_SeriesInstanceUID;

 
	gLogger.LogMessage(kDebug,"CSendDicomProc::sendSeries to [%s], ( ID[%d] Job[%s] No.[%s] ) [%s] [%s]\n", 
					AE.c_str(),
					entry->m_QueueID,
					entry->m_JobID.c_str(),
					entry->m_SOPInstanceUID.c_str(),
					StudyUID.c_str(),SeriesUID.c_str());
	gLogger.FlushLog();

	////
	CStoreSCUIf *pCStoreSCU_hdr = openCStoreAssociation(AE,SeriesUID);
	if(!pCStoreSCU_hdr){
		gLogger.LogMessage(kDebug,"CSendDicomProc::sendEntryFile openCStoreAssociation error\n");
		gLogger.FlushLog();
		return false;
	}
#if 1
		pCStoreSCU_hdr->setDicomStudyInfo("",
			//#142_NoPatientName_NoComment
			//entry->m_extInfo.m_PatientName,
			entry->m_extInfo.m_PatientID,entry->m_extInfo.m_BirthDate);
#else
	//if(gConfig.m_safetyCheck !=0 ){
	if(1){ //for logger the Study information
		//#27 2012/06/14 K.Ko
	//	CPxDcmDB pxDb;
		if(pCStoreSCU_hdr->getDicomStudyInfo() == 0){
			m_curStudy.clear();
			if(StudyUID.size()>0){
				DICOMData tmpFilter;
				ASTRNCPY(tmpFilter.m_studyInstanceUID, StudyUID.c_str());
			//	std::vector<DICOMStudy>  oVal;
				
				m_pDcmDB->GetStudyList( m_curStudy, &tmpFilter);
			 
			}
		}

		if(m_curStudy.size()>0){
			pCStoreSCU_hdr->setDicomStudyInfo(&(m_curStudy[0]));
		}
	}
#endif


	int image_size = ImageFileList.size();
	std::string dicom_file_name;
	std::string my_series_folder = seriesFolder + "\\";
	for(int i=0;i<image_size;i++){
		dicom_file_name = my_series_folder + ImageFileList[i];
		if(!pCStoreSCU_hdr->sendDicomFile(dicom_file_name)){
			gLogger.LogMessage(kDebug,"CSendDicomProc::sendEntryFile sendDicomFile[%s] error\n",dicom_file_name.c_str());
			gLogger.FlushLog();
			return false;
		}
	}
	//

	pCStoreSCU_hdr->closeSession(); //１ブロック  使用終了
	 
	 ////////
	/// do not close association heere


	return true;
}
bool CSendDicomProc::queryImages(const std::string studyUID,const std::string &seriesUID,std::vector<DICOMInstance> &image_list)
{
	//CPxDcmDB pxDb;

	DICOMData  Filter;

	Filter.Clear();
	;

	DICOMStudy  iStudy;
	strcpy(Filter.m_studyInstanceUID, studyUID.c_str());
	strcpy(Filter.m_seriesInstanceUID, seriesUID.c_str());

	image_list.clear();

	int status = m_pDcmDB->GetInstanceList(image_list,&Filter,seriesUID.c_str());

 
	if (status != kOK)
	{
		gLogger.LogMessage(kErrorOnly," ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		gLogger.FlushLog();
		return false;
	}
	//
	
	 
	return true;
}

bool CSendDicomProc::querySeries(const std::string studyUID, std::vector<DICOMSeries> &series_list)
{
//	CPxDcmDB pxDb;

	DICOMData studyFilter;

	studyFilter.Clear();
	 
	strcpy(studyFilter.m_studyInstanceUID, studyUID.c_str());

	series_list.clear();

 	int status = m_pDcmDB->GetSeriesList(series_list,&studyFilter);
 

	 
	if (status != kOK)
	{
		gLogger.LogMessage(kErrorOnly," ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		gLogger.FlushLog();
		return false;
	}
	//
	return true;
}
CStoreSCUIf *CSendDicomProc::openCStoreAssociation(const std::string &AE,const std::string &SeriesUID)
{
	 
	CStoreSCUIf *pStoreSCU = 0;
 
	CStoreSCUSeriesDirMonitor::CLockSeriesDirMonitor lock(CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor());
 
	{ 
		pStoreSCU = CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor().getSotreSCU(AE,SeriesUID);
		if(!pStoreSCU){
			CStoreSCU *new_cstoreSUC = new CStoreSCU(m_connectInfo);
			 
			new_cstoreSUC->initCStoreSCU();

			if(!new_cstoreSUC->openCStoreAssociation(AE,SeriesUID)){
				delete new_cstoreSUC;
		//		return 0;
	 
			}else{
			 
 				CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor().insertCSotreSCU(AE,SeriesUID,new_cstoreSUC);
				pStoreSCU = new_cstoreSUC;
			 
			}
			 
		}
		if(pStoreSCU){
			pStoreSCU->openSession(); //１ブロック（Ｉｍａｇｅ）使用中、削除しないようにする
		}
	 
	}

 

	return pStoreSCU;
  
}
bool CSendDicomProc::closeCStoreAssociation(CStoreSCUIf *cstoreSCU)
{
	CStoreSCUSeriesDirMonitor::CLockSeriesDirMonitor lock(CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor());
 
	CStoreSCUIf *StoreSCU = CStoreSCUSeriesDirMonitor::theStoreSCUSeriesDirMonitor().deleteCSotreSCU(
		cstoreSCU->getAE(),cstoreSCU->getSeriesUID());
	if(!StoreSCU){
		return false;
	}else{
		StoreSCU->closeCStoreAssociation();
		delete StoreSCU;
	}

	return true;
	
}
void CSendDicomProc::setLocaleAE(const std::string &localeAE)
{
	strncpy(m_connectInfo.LocalApplicationTitle,localeAE.c_str(),20);
}
bool CSendDicomProc::init( )
{
	return initDICOM();
}