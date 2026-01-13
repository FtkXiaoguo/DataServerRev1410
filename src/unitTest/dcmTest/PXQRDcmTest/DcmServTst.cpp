#include "DcmServTst.h"

 
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
 

extern int VLIDicomApplicationID;
CDcmServTst::CDcmServTst(void)
{
	m_localAETitle = "MyAE";
}

CDcmServTst::~CDcmServTst(void)
{
}
void CDcmServTst::Init(void)
{
	PxDicomStatus status;
		
	int applicationID;
	status = (PxDicomStatus) MC_Register_Application(&applicationID, m_localAETitle.c_str());
	if (status != kNormalCompletion)
	{
		return  ;
	}
	
	 
	// copy to VLIDiCOM, should let VLIDiCOM reference this
	VLIDicomApplicationID = applicationID;
}

void CDcmServTst::setupStudyQuery(const QueryStudyInfo &query)
{
	m_lastQuery.ClearAllValues();
	//
	if(query.m_patientName.size()<1){
		m_lastQuery.SetValueNull(kVLIPatientsName);
	}else{
		m_lastQuery.SetValue(kVLIPatientsName, query.m_patientName.c_str());
	}

	if(query.m_patientID.size()<1){
		m_lastQuery.SetValueNull(kVLIPatientId);
	}else{
		m_lastQuery.SetValue(kVLIPatientId, query.m_patientID.c_str());
	}
	//
	if(query.m_patientSex.size()<1){
		m_lastQuery.SetValueNull(kVLIPatientsSex);
	}else{
		m_lastQuery.SetValue(kVLIPatientsSex, query.m_patientSex.c_str());
	}
	//
	if(query.m_patientBirthDate.size()<1){
		m_lastQuery.SetValueNull(kVLIPatientsBirthDate);
	}else{
		m_lastQuery.SetValue(kVLIPatientsBirthDate, query.m_patientBirthDate.c_str());
	}
	//
	if(query.m_studyCount.size()<1){
		m_lastQuery.SetValueNull(kVLINumberOfPatientRelatedStudies);
	}else{
		m_lastQuery.SetValue(kVLINumberOfPatientRelatedStudies, query.m_studyCount.c_str());
	}
	//
	if(query.m_studyUID.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyInstanceUid);
	}else{
		m_lastQuery.SetValue(kVLIStudyInstanceUid, query.m_studyUID.c_str());
	}
	//
	if(query.m_studyID.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyId);
	}else{
		m_lastQuery.SetValue(kVLIStudyId, query.m_studyID.c_str());
	}
	//
	if(query.m_studyDate.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyDate);
	}else{
		m_lastQuery.SetValue(kVLIStudyDate, query.m_studyDate.c_str());
	}
	//
	if(query.m_studyTime.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyTime);
	}else{
		m_lastQuery.SetValue(kVLIStudyTime, query.m_studyTime.c_str());
	}
	//
	if(query.m_accessionNumber.size()<1){
		m_lastQuery.SetValueNull(kVLIAccessionNumber);
	}else{
		m_lastQuery.SetValue(kVLIAccessionNumber, query.m_accessionNumber.c_str());
	}
	//
	if(query.m_modalitiesInStudy.size()<1){
		m_lastQuery.SetValueNull(kVLIModalitiesInStudy);
	}else{
		m_lastQuery.SetValue(kVLIModalitiesInStudy, query.m_modalitiesInStudy.c_str() );
	}
	//
	if(query.m_studyDescription.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyDescription);
	}else{
		m_lastQuery.SetValue(kVLIStudyDescription, query.m_studyDescription.c_str());
	}
	//
	if(query.m_referringPhysiciansName.size()<1){
		m_lastQuery.SetValueNull(kVLIReferringPhysiciansName);
	}else{
		m_lastQuery.SetValue(kVLIReferringPhysiciansName, query.m_referringPhysiciansName.c_str());
	}
	//
	 
}
void CDcmServTst::setupSeriesQuery(const QuerySeriesInfo &query)
{
	m_lastQuery.ClearAllValues();
	//
	if(query.m_studyUID.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyInstanceUid);
	}else{
		m_lastQuery.SetValue(kVLIStudyInstanceUid, query.m_studyUID.c_str());
	}
	//
	if(query.m_seriesUID.size()<1){
		m_lastQuery.SetValueNull(kVLISeriesInstanceUid);
	}else{
		m_lastQuery.SetValue(kVLISeriesInstanceUid, query.m_seriesUID.c_str());
	}
	//
	if(query.m_seriesDate.size()<1){
		m_lastQuery.SetValueNull(kVLISeriesDate);
	}else{
		m_lastQuery.SetValue(kVLISeriesDate, query.m_seriesDate.c_str());
	}
	//
	if(query.m_seriesTime.size()<1){
		m_lastQuery.SetValueNull(kVLISeriesTime);
	}else{
		m_lastQuery.SetValue(kVLISeriesTime, query.m_seriesTime.c_str());
	}
	//
	if(query.m_seriesNumber.size()<1){
		m_lastQuery.SetValueNull(kVLISeriesNumber);
	}else{
		m_lastQuery.SetValue(kVLISeriesNumber, query.m_seriesNumber.c_str());
	}
	//
	if(query.m_modality.size()<1){
		m_lastQuery.SetValueNull(kVLIModality);
	}else{
		m_lastQuery.SetValue(kVLIModality, query.m_modality.c_str());
	}
	//
	if(query.m_imageCount.size()<1){
		m_lastQuery.SetValueNull(kVLINumberOfSeriesRelatedInstances);
	}else{
		m_lastQuery.SetValue(kVLINumberOfSeriesRelatedInstances, query.m_imageCount.c_str());
	}
	//
	if(query.m_seriesDescription.size()<1){
		m_lastQuery.SetValueNull(kVLISeriesDescription);
	}else{
		m_lastQuery.SetValue(kVLISeriesDescription, query.m_seriesDescription.c_str());
	}
	//
	if(query.m_bodyPartExamined.size()<1){
		m_lastQuery.SetValueNull(kVLIBodyPartExamined);
	}else{
		m_lastQuery.SetValue(kVLIBodyPartExamined, query.m_bodyPartExamined.c_str());
	}
}
//
void CDcmServTst::setupImageQuery(const QueryImageInfo &query)
{
	m_lastQuery.ClearAllValues();
	//
	if(query.m_studyUID.size()<1){
		m_lastQuery.SetValueNull(kVLIStudyInstanceUid);
	}else{
		m_lastQuery.SetValue(kVLIStudyInstanceUid, query.m_studyUID.c_str());
	}
	//
	if(query.m_seriesUID.size()<1){
		m_lastQuery.SetValueNull(kVLISeriesInstanceUid);
	}else{
		m_lastQuery.SetValue(kVLISeriesInstanceUid, query.m_seriesUID.c_str());
	}
	//
	if(query.m_SOPInstanceUID.size()<1){
		m_lastQuery.SetValueNull(kVLISopInstanceUid);
	}else{
		m_lastQuery.SetValue(kVLISopInstanceUid, query.m_SOPInstanceUID.c_str());
	}
	//
	if(query.m_instanceNumber.size()<1){
		m_lastQuery.SetValueNull(kVLIInstanceNumber);
	}else{
		m_lastQuery.SetValue(kVLIInstanceNumber, query.m_instanceNumber.c_str());
	}

}
 
void CDcmServTst::dotst(int loopNN)
{
	PxDicomStatus status;


	iRTVThread OneShotThreadManagerThread(&RTVOneShotThreadManager::theManager());
#if 1
	// AddServer(const char* iAETitle, const char* iHostname, int iPort, int iConnect = 1);
	AddServer("MONE_AE","mone",105);
	//
#endif

	DicomDataSource  iSource;
	std::vector<CPxDicomMessage*>  oReply;
//	status = QueryListOfStudies  (  iSource,  oReply, const char* iPatientID);

	for(int i=0;i<loopNN;i++){

	// Query Study
	//
	QueryStudyInfo query;
	query.m_patientName = "*";
	setupStudyQuery(query);

#if 1
	bool searchOne = true;
	status = QueryAllServers(m_lastQuery, oReply, searchOne); 
#else
	status = Query(m_lastQuery, oReply);
#endif

	std::string cur_study_uid;
	if (status == kNormalCompletion || status == kFindAborted )
	{	int i;
		char str_buff[256];
		
		int replySize = oReply.size();
		
		for( i=0;i<replySize;i++){
			printf("--------------------------\n");
			printf("No. %d \n",i);
			oReply[i]->GetValue(kVLIPatientsName,str_buff);
			printf("PatientName: %s \n",str_buff);
			//
			oReply[i]->GetValue(kVLIPatientId,str_buff);
			printf("PatientId: %s \n",str_buff);
			//
			oReply[i]->GetValue(kVLIStudyDate,str_buff);
			printf("StudyDate: %s \n",str_buff);
			//
			oReply[i]->GetValue(kVLIStudyTime,str_buff);
			printf("StudyTime: %s \n",str_buff);
			//
			//
			oReply[i]->GetValue(kVLIStudyInstanceUid,str_buff);
			printf("StudyUID: %s \n",str_buff);
			cur_study_uid = str_buff;
		}
		// delete 
		for( i=0;i<replySize;i++){
			delete oReply[i];
		}
		oReply.clear();
	}

	//
	// Query Series
	//
	QuerySeriesInfo series_query;
	series_query.m_studyUID = cur_study_uid;
	setupSeriesQuery(series_query);

	status = QueryAllServers(m_lastQuery, oReply, searchOne); 

	std::string cur_series_uid;
	if (status == kNormalCompletion || status == kFindAborted )
	{	
		int i;
		char str_buff[256];
		
		int replySize = oReply.size();
		
		for( i=0;i<replySize;i++){
			printf("--------------------------\n");
			printf("No. %d \n",i);
			oReply[i]->GetValue(kVLISeriesInstanceUid,str_buff);
			printf("SeriesUid: %s \n",str_buff);
			cur_series_uid = str_buff;
		}
		// delete 
		for( i=0;i<replySize;i++){
			delete oReply[i];
		}
		oReply.clear();
	}

	//
	// Query Image
	//
	QueryImageInfo image_query;
	image_query.m_studyUID = cur_study_uid;
	image_query.m_seriesUID = cur_series_uid;
	setupImageQuery(image_query);

	status = QueryAllServers(m_lastQuery, oReply, searchOne); 

	std::string cur_sopInstance_uid;
	if (status == kNormalCompletion || status == kFindAborted )
	{	
		int i;
		char str_buff[256];
		
		int replySize = oReply.size();
		
		for( i=0;i<replySize;i++){
			printf("--------------------------\n");
			printf("No. %d \n",i);
			oReply[i]->GetValue(kVLISopInstanceUid,str_buff);
			printf("SopInstanceUid: %s \n",str_buff);
			cur_sopInstance_uid = str_buff;
		}
		// delete 
		for( i=0;i<replySize;i++){
			delete oReply[i];
		}
		oReply.clear();
	}
	//
	// local C-Store SCP •K—v
//	RetrieveImages(cur_study_uid,cur_series_uid);

	} // loopNN

}

void CDcmServTst::RetrieveImages(std::string studyUID, std::string seriesUID)
{
	DicomDataSource iSource;
	if(!m_serverList.empty()){
		iSource.SetServer(*(m_serverList.begin()));
	}
	 
	std::vector<CPxDicomImage*>  oImages;
	//
	std::vector<std::string> seriesV, studyV, instanceV;
	std::vector<DicomDataSource> dsv;
	std::string modality;

	oImages.clear();
	studyV.clear();
	seriesV.clear();
	instanceV.clear();

	seriesV.push_back(seriesUID);
	studyV.push_back(studyUID);

	Retrieve(oImages, studyV, seriesV, instanceV, "SERIES", &iSource, 0, false);

	//
}