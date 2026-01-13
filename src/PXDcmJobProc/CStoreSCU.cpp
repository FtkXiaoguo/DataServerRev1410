/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "CStoreSCU.h"

#include <assert.h>
#include <sys/timeb.h>
#include "rtvpoolaccess.h"
//#include "AppComUtil.h"
//#include "TRDICOMUtil.h"
#include "Globals.h"
#include "DiskSpaceManager.h"
//#include "SeriesDirMonitor.h"
#include "Compression.h"
#include "NMObject.h"
#include "AuxData.h"
#include "AppComCacheWriter.h"
#include "Conversion.h"


#include "StoreSCUSeriesDirMonitor.h"

#include "rtvsutil.h"

#ifdef _PROFILE
#include "ScopeTimer.h"
#endif

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "IDcmLibDefUID.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif
 

#define SERVICENAME_LEN		64
 
enum
{
	kECSuccess = 0,
	kECWarning,
	kECFailure,
	kECUnknown
};

//#25 20012/06/11 K.Ko
#include "JISToSJISMS.h"
void CStoreSCU::ReformatJapaneseDicom( const std::string &org, std::string &conv)
{
	CJISToSJISMS::ConvertJISToSJIS( org, conv );
	CJISToSJISMS::ReformatPatientName( conv, cPNStandard );
}

//-----------------------------------------------------------------------------
CStoreSCU::CStoreSCU (const DiCOMConnectionInfo &connectionInfo)
:m_sessionOpened(false)
{
	m_cs = new TRCriticalSection ;
	m_connectInfo = connectionInfo;
//	 sprintf(m_connectInfo.LocalApplicationTitle,"DcmQueueAE");
	m_hasDicomStudyInfo = false;
}

 
//-----------------------------------------------------------------------------
CStoreSCU::~CStoreSCU()
{
	delete m_cs;
	int x =0;
 

}
void CStoreSCU::destroy()
{
	m_hasDicomStudyInfo = false;
	m_connectInfo.AssociationID = -1;
	m_AE.empty();
	m_SeriesUID.empty();
	//
	int status = MC_Release_Application(&m_connectInfo.ApplicationID);
	if (status != MC_NORMAL_COMPLETION)
	{
		gLogger.LogMessage("ERROR:  error %d - failed to Release AE %s\n", status);
		return  ;
	}

}
int gConnectTimeout = 20;
bool CStoreSCU::initDICOM()
{
	int status;
	if(!TRDICOMUtil::InitialDICOM("mydcmtk" ))
	{
		gLogger.LogMessage("ERROR: InitialDICOM failed\n");
		gLogger.FlushLog();
		return false;
	}

	if(!TRDICOMUtil::initServiceList(Q_CSTORE_SERVICE_NAME,true/*isPropose*/))
	{
		gLogger.LogMessage("ERROR: CStoreSCU::initDICOM   [%s] \n", Q_CSTORE_SERVICE_NAME);
		gLogger.FlushLog();
		return false;
	}

	return true;
}
bool CStoreSCU::initCStoreSCU()
{
	TRCSLock fplock(m_cs);

	int status;
	//	Register the localAETitle  
	status = MC_Register_Application(&m_connectInfo.ApplicationID, m_connectInfo.LocalApplicationTitle);
	if (status != MC_NORMAL_COMPLETION)
	{
		gLogger.LogMessage("ERROR: error %d - failed to register AE %s\n", status, m_connectInfo.LocalApplicationTitle);
		return -status;
	}

	MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, gConnectTimeout);
	MC_Set_Int_Config_Value(CONNECT_TIMEOUT, gConnectTimeout);


	

	return true;
}

bool CStoreSCU::sendStudy(const std::string &AE,const std::string &StudyUID)
{
	TRCSLock fplock(m_cs);

	gLogger.LogMessage(kDebug,"CStoreSCU::sendStudy to [%s], [%s]\n", AE.c_str(),StudyUID.c_str());
	gLogger.FlushLog();

	return true;
}
bool CStoreSCU::sendSeries(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID)
{
	TRCSLock fplock(m_cs);

	gLogger.LogMessage(kDebug,"CStoreSCU::sendSeries to [%s], [%s] [%s]\n", AE.c_str(),StudyUID.c_str(),SeriesUID.c_str());
	gLogger.FlushLog();

#if 0
	//
	std::vector<AQNetDevice> raids;
	AQNetConfiguration::GetArchiveDevices(AQNetConfiguration::gkRAIDType,raids);

//	std::string series_folder = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom(SeriesUID, StudyUID, 0);
	std::string SOPInstanceUID ;
	int imageNumber;
	std::string series_folder = AqNETUtil::getSeriesFolder(StudyUID, SeriesUID,"*%s.dcm");
 

	std::vector<DICOMInstance>  image_list;
	queryImages(StudyUID,SeriesUID,image_list);

	int image_size = image_list.size();
	if(image_size<1){
		return false;
	}
	////
	if(!openCStoreAssociation(AE,SeriesUID)){
	}
	////
	std::string dicom_file_name;
	for(int i=0;i<image_size;i++){
		DICOMInstance dicom_infor = image_list[i];
		dicom_file_name = AqNETUtil::getDicomFileName(series_folder,
										dicom_infor.m_SOPInstanceUID,
										dicom_infor.m_instanceNumber);
	}
#endif
	return true;
}
bool CStoreSCU::sendImage(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,const std::string &SOPInstanceUID)
{
	TRCSLock fplock(m_cs);

	gLogger.LogMessage(kDebug,"CStoreSCU::sendSeries to [%s] \n   [%s]   [%s]   [%s]\n", AE.c_str(),
				StudyUID.c_str(),SeriesUID.c_str(),SOPInstanceUID.c_str());
	gLogger.FlushLog();

	return true;
}

bool CStoreSCU::queryImages(const std::string studyUID,const std::string &seriesUID,std::vector<DICOMInstance> &image_list)
{
	CPxDcmDB pxDb;

	DICOMData  Filter;

	Filter.Clear();
	;

	DICOMStudy  iStudy;
	strcpy(Filter.m_studyInstanceUID, studyUID.c_str());
	strcpy(Filter.m_seriesInstanceUID, seriesUID.c_str());

	image_list.clear();

	int status = pxDb.GetInstanceList(image_list,&Filter,seriesUID.c_str());

 
	if (status != kOK)
	{
		printf(" ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		return false;
	}
	//
	
	 
	return true;
}

bool CStoreSCU::openCStoreAssociation(const std::string &AE,const std::string &SeriesUID)
{
 
	TRCSLock fplock(m_cs);

	m_AE = AE;
	m_SeriesUID = SeriesUID;

	m_lastTime = time(0);

	int associationID = -1;

	CPxDcmDB	g_db;

	AqString whereFilter;
	whereFilter.Format(" WHERE AETitle= '%s'", AE.c_str());

	 

	//StoreTarget
	std::vector<ApplicationEntity> tmpData_StoreTarget;
	int ret = g_db.QueryApplicationEntity(CPxDB::kStoreTargetAE, tmpData_StoreTarget,whereFilter);
 	if(ret != kOK) return false;

	if(tmpData_StoreTarget.size()<1){
		return false;
	}

	ApplicationEntity ae = tmpData_StoreTarget[0];
 
	int port_number = ae.GetPort();
	char host_tcpip[256];
	sprintf(host_tcpip,"%s",ae.GetAddress());
	int status = MC_Open_Association(m_connectInfo.ApplicationID, &associationID, 
									ae.m_AETitle, &port_number,host_tcpip,
 
									Q_CSTORE_SERVICE_NAME);

	if(status == MC_NORMAL_COMPLETION){
		m_connectInfo.AssociationID = associationID;
	}else{
		return false;
	}
	//printf("send dicom \n");
	return true;
}
bool CStoreSCU::closeCStoreAssociation(void)
{
	TRCSLock fplock(m_cs);

	if(m_connectInfo.AssociationID <0){
		return false;
	}
	MC_Close_Association(&m_connectInfo.AssociationID);
	m_connectInfo.AssociationID = -1;

	printf("\n");

	return true;
}
bool CStoreSCU::sendDicomFile(const std::string &fileName)
{
	TRCSLock fplock(m_cs);

	bool ret_b = false;

	m_lastTime = time(0);

	int status;
	printf(".");
	CPxDicomMessage *dcm_msg = new CPxDicomMessage;

	try {
		status = dcm_msg->Load(fileName.c_str());

		if(status != kNormalCompletion){
			throw(-1);
		}
		int iMessageID = dcm_msg->GetID();
		char SOPClassUID[kVR_UI];
		char serviceName2[SERVICENAME_LEN];

		/*
		* 
		*/
		if(gConfig.m_safetyCheck !=0 ){
		//#27 2012/06/14 K.Ko

			std::string str_temp;
			char _str_buff[256];
			
			//ref:
			//CPxDicomImage::FillSortInfo
			if(m_hasDicomStudyInfo){
				 
				std::string ref_str_temp;
				std::string str_temp;
				//PatientName
				_str_buff[0]=0;

				 
			//	 status = MC_Get_Value_To_String(oMID, MC_ATT_PATIENTS_NAME, sizeof(_str_buff), _str_buff);
				status = dcm_msg->GetValue(MC_ATT_PATIENTS_NAME, _str_buff, sizeof(_str_buff));
				if (status != MC_NORMAL_COMPLETION)
				{  
					gLogger.LogMessage(kDebug,"ERROR:(%d) CStoreSCU::sendDicomFile - Failed on (%d,%s)  MC_Get_Value_To_String MC_ATT_PATIENTS_NAME   \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) );
					gLogger.FlushLog();
					//return -1; //do not return error
				}else{
					ref_str_temp = ""; //m_patientNameRef;	//#142_NoPatientName_NoComment
					if(ref_str_temp.size()>0){
						CStoreSCU::ReformatJapaneseDicom( _str_buff, str_temp);
						std::string str_temp_temp = str_temp;
						CStoreSCU::trimLeftRight(str_temp_temp,str_temp);
			//			str_temp		= 	_str_buff;
						if(str_temp != ref_str_temp){
							gLogger.LogMessage(kErrorOnly,"ERROR:[C%08d](%d) CStoreSCU::sendDicomFile - Failed on check patientName  %s != %s \n",DicomJobProcInfor_SafetyCheckPatientInfo,m_connectInfo.AssociationID,ref_str_temp.c_str(),str_temp.c_str() );
							gLogger.FlushLog();
							return false;
						}
					}
				}

				//PatientID
				_str_buff[0]=0;
				 status = dcm_msg->GetValue(MC_ATT_PATIENT_ID, _str_buff, sizeof(_str_buff));
				if (status != MC_NORMAL_COMPLETION)
				{  
					gLogger.LogMessage(kDebug,"ERROR:(%d) CStoreSCU::sendDicomFile - Failed on (%d,%s)  MC_Get_Value_To_String MC_ATT_PATIENT_ID   \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) );
					gLogger.FlushLog();
					//return -1;//do not return error
				}else{
					ref_str_temp	= m_patientIDRef;
					if(ref_str_temp.size()>0){
					//	str_temp		= 	_str_buff;
						CStoreSCU::trimLeftRight(_str_buff,str_temp);
						if(str_temp != ref_str_temp){
							gLogger.LogMessage(kErrorOnly,"ERROR:[C%08d](%d) CStoreSCU::sendDicomFile - Failed on check PatientID  %s != %s \n",DicomJobProcInfor_SafetyCheckPatientInfo,m_connectInfo.AssociationID,ref_str_temp.c_str(),str_temp.c_str() );
							gLogger.FlushLog();
							return false;
						}
					}
				}

				//BirthDate
				_str_buff[0]=0;
				 status = dcm_msg->GetValue(MC_ATT_PATIENTS_BIRTH_DATE, _str_buff, sizeof(_str_buff));
				if (status != MC_NORMAL_COMPLETION)
				{  
					gLogger.LogMessage(kDebug,"ERROR:(%d) CStoreSCU::sendDicomFile) - Failed on (%d,%s)  MC_Get_Value_To_String MC_ATT_PATIENTS_BIRTH_DATE   \n", m_connectInfo.AssociationID, status, MC_Error_Message((MC_STATUS) status) );
					gLogger.FlushLog();
					//return -1;//do not return error
				}else{
					ref_str_temp	= m_patientBirthDateRef;
					if(ref_str_temp.size()>0){
						//str_temp		= 	_str_buff;
						CStoreSCU::trimLeftRight(_str_buff,str_temp);
						if(str_temp != ref_str_temp){
							gLogger.LogMessage(kErrorOnly,"ERROR:[C%08d](%d) CStoreSCU::sendDicomFile - Failed on check BirthDate  %s != %s \n",DicomJobProcInfor_SafetyCheckPatientInfo,m_connectInfo.AssociationID,ref_str_temp.c_str(),str_temp.c_str() );
							gLogger.FlushLog();
							return false;//-1;
						}
					}
				}
			}
		}

		 

		// Get the SOP class UID and set the service  
		status = MC_Get_Value_To_String(iMessageID, MC_ATT_SOP_CLASS_UID, sizeof(SOPClassUID), SOPClassUID);
		if (status != MC_NORMAL_COMPLETION)
		{  
			gLogger.LogMessage(kErrorOnly,"ERROR: %d Failed to get SOP_CLASS_UID\n", status );
			gLogger.FlushLog();
			return status;  
		}
	       
		status = MC_Get_MergeCOM_Service(SOPClassUID, serviceName2, sizeof(serviceName2));
		if (status != MC_NORMAL_COMPLETION)
		{
			gLogger.LogMessage(kErrorOnly,"ERROR: %d Failed to get MC_Get_MergeCOM_Service \n", status );
			gLogger.FlushLog();
			return status;  
		}

		status = MC_Set_Service_Command(iMessageID, serviceName2, C_STORE_RQ);
		if (status != MC_NORMAL_COMPLETION)
		{
			gLogger.LogMessage(kErrorOnly,"ERROR: %d MC_Set_Service_Command failed\n", status);
			return status;  
		}

		///////
		// Send it off 
		status = MC_Send_Request_Message (m_connectInfo.AssociationID, iMessageID);
		if (status != MC_NORMAL_COMPLETION)
		{
			gLogger.LogMessage(kErrorOnly,"ERROR: %d Failed on sending C_STORE_RQ\n", status);
			gLogger.FlushLog();
			return false;  
		} 

		bool oMessageWasSent = false;
		 
		if(ReadResponse(iMessageID, oMessageWasSent))
		{
			if (!oMessageWasSent){
				ret_b = false;
			}else{
				ret_b = true;
			}
		}else{
			ret_b = false;
		}

		
	}catch(int err_no){
		ret_b = false;
	}catch(...){
		ret_b = false;
	}
	delete dcm_msg;

	return ret_b;
}

bool CStoreSCU::ReadResponse(int iMessageID,bool& oMessageWasSent)
{

	oMessageWasSent = false;

   	char*			serviceName;
	MC_COMMAND		command;
	MC_STATUS		status;
	int responseStatus; 
	 
	int timeout = 600;  
	int cStoreResponseMessageID; // need to be freed if created.
	unsigned int  cStoreResponse;

	// Wait for a reply 
    status = MC_Read_Message(m_connectInfo.AssociationID, timeout, &cStoreResponseMessageID, &serviceName, &command);			
    if (status != MC_NORMAL_COMPLETION)
    {
		gLogger.LogMessage(kErrorOnly,"ERROR: error %d Read_Message failed\n", status);
		gLogger.FlushLog();
		return false;  
         
    }

    status = MC_Get_Value_To_UInt(cStoreResponseMessageID, MC_ATT_STATUS, &cStoreResponse);
    if (status != MC_NORMAL_COMPLETION)
    {
        gLogger.LogMessage(kErrorOnly,"ERROR: error %d Failed to get C-STORE Response\n", status);
		gLogger.FlushLog();
        MC_Free_Message(&cStoreResponseMessageID);
		return false;  
    }
	 
	//	Rob Lewis - 2006.08.14 - not sure if doing it this way is precise enough.  However, the old way misses some failure codes (0xA800 from many vendors). 
	//		The problem is: that code is not explicitly defined anywhere in the DICOM standard.  Nor is the validity of masking 0xA000 to define failure.
	//		However, upon surveying the DICOM standard and some Conformance statements from the field, there were no apparent cases of any Axxx values being used 
	//		as anything	but failure.  This appears to be the safest way to classify the status codes.
	if (cStoreResponse & 0xA000 || 
		cStoreResponse & 0xC000 || 
		cStoreResponse == 0x0110 ||		//  Processing failure
		cStoreResponse == 0x0122)		//	Refused: SOP Class not supported
		responseStatus = kECFailure;
	else if (cStoreResponse & 0xB000 || 
		cStoreResponse == 0x0111 ||		//	Refused: Duplicate SOP
		cStoreResponse == 0xD000)		//	Some vendors use as Refused: Duplicate SOP
		responseStatus = kECWarning;
	else if (cStoreResponse == 0x0000)
		responseStatus = kECSuccess;
	else
		responseStatus = kECUnknown;

    // Check for a successful, failed, or Warning C-STORE   .  
    if (responseStatus == kECSuccess)
	{
//		m_successfulCStoreSubOperations++;
		oMessageWasSent = true;
	}
	else if (responseStatus == kECWarning)
	{
		gLogger.LogMessage(kWarning,"WARNING: C-STORE-RSP indicates warning \n");
		gLogger.FlushLog();
	//	m_warnedCStoreSubOperations++;
		oMessageWasSent = true;
	}
	else if (responseStatus == kECFailure)
	{
		gLogger.LogMessage(kErrorOnly,"ERROR: C-STORE-RSP indicates storage failure \n");
		gLogger.FlushLog();
		oMessageWasSent = false;
	}
	else // (responseStatus == kECUnknown)
	{
		//	NOTE: DICOM is somewhat ambiguous about these codes and their meaning.  
		gLogger.LogMessage(kErrorOnly,"ERROR: unknown C-STORE-RSP status code indicates potential storage failure.  Failed storage assumed  \n");
		gLogger.FlushLog();
		oMessageWasSent = false;
	}

	if (responseStatus != kECSuccess)
	{
 		gLogger.LogMessage("       (0000,0900) Status = 0x%04x\n", cStoreResponse);
		gLogger.FlushLog();
		//	Attempt to get the error comment, if available
		int errCmtStatus;
		char errorComment[kVR_LO];
		memset(errorComment, 0, sizeof(errorComment));
		errCmtStatus = MC_Get_Value_To_String(cStoreResponseMessageID, MC_ATT_ERROR_COMMENT, sizeof(errorComment), errorComment);
		if (errCmtStatus == MC_NORMAL_COMPLETION && strlen(errorComment) > 0){
			gLogger.LogMessage("       (0000,0902) Error Comment = %s\n",	errorComment);
			gLogger.FlushLog();
		}
	}

	MC_Free_Message(&cStoreResponseMessageID);

	if(status==MC_NORMAL_COMPLETION){
		return true;
	}else{
		return false;
	}
 
}

bool CStoreSCU::tryToClose(int seriesCompleteTimeout/*sec*/)
{
	TRCSLock fplock(m_cs);

	time_t cur_time = time(0);
	int time_diff = cur_time - m_lastTime;
	bool close_flag = (time_diff>(seriesCompleteTimeout ));

	if(m_sessionOpened){
	//session is opened
		close_flag = (time_diff>300);
	}

	if(close_flag){
		closeCStoreAssociation();
	}
	return close_flag;
}

//#8 2012/03/16 K.KO
//ServiceList指定仕組みの追加
#define M_ADD_SOP_CLASS2CSTORE(sop_cls) { \
	if(!TRDICOMUtil::addSOPClassUID((sop_cls),Q_CSTORE_SERVICE_NAME,true/*isPropose*/)) \
	{\
		gLogger.LogMessage("ERROR: CMove::initCMoveServiceList TRDICOMUtil::addSOPClassUID [%s] \n", (sop_cls));\
		gLogger.FlushLog();\
		return false;\
	}\
}
bool CStoreSCU::initCMoveServiceList()
{
	if(!TRDICOMUtil::initServiceList(Q_CSTORE_SERVICE_NAME,true/*isPropose*/))
	{
		gLogger.LogMessage("ERROR: CMove::initCMoveServiceList TRDICOMUtil::initServiceList [%s] \n", Q_CSTORE_SERVICE_NAME);
		gLogger.FlushLog();
		return false;
	}

	/*
	* CMove時にMove先に対して、Proposeする
	* 調整してください。
	*/
	//
	M_ADD_SOP_CLASS2CSTORE(UID_CTImageStorage);
	//
	M_ADD_SOP_CLASS2CSTORE(UID_XRayAngiographicImageStorage );
	M_ADD_SOP_CLASS2CSTORE(UID_XRayRadiofluoroscopicImageStorage );
	M_ADD_SOP_CLASS2CSTORE(UID_XRayRadiationDoseSRStorage );
	M_ADD_SOP_CLASS2CSTORE(UID_SecondaryCaptureImageStorage );
	M_ADD_SOP_CLASS2CSTORE(UID_FINDPatientRootQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CSTORE(UID_FINDStudyRootQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CSTORE(UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CSTORE(UID_MOVEPatientRootQueryRetrieveInformationModel );
	M_ADD_SOP_CLASS2CSTORE(UID_MOVEStudyRootQueryRetrieveInformationModel );
 
	//
	M_ADD_SOP_CLASS2CSTORE(UID_DigitalIntraOralXRayImageStorageForPresentation);
	//
	//
	M_ADD_SOP_CLASS2CSTORE(UID_DigitalXRayImageStorageForPresentation);// #23 2012/07/07 K.KO for DX
	M_ADD_SOP_CLASS2CSTORE(UID_DigitalXRayImageStorageForProcessing);// #23 2012/07/07 K.KO for DX

	return true;
}

#if 0
DICOMStudy *CStoreSCU::getDicomStudyInfo()
{
	if(!m_hasDicomStudyInfo) return 0;

	return &m_DicomStudyInfo;
}

void CStoreSCU::setDicomStudyInfo(const DICOMStudy *info)
{

	memcpy(&m_DicomStudyInfo ,info,sizeof(DICOMStudy));

	std::string str_temp;
	//just use patientName (JIS)
	CStoreSCU::ReformatJapaneseDicom( m_DicomStudyInfo.m_patientsName, str_temp);
	strcpy(m_DicomStudyInfo.m_patientsName,str_temp.c_str());
#else
#define IS_SPACE(c) ((c)==' ')
static inline char* trimRight(char* buf)
{
	int len = strlen(buf);
	char *p;

#if 0
	for ( p = buf + len-1; p >= buf && isspace(*p); )
		*p-- = 0;
#else
	char *start_p	= buf;
	char *end_p		= buf + len-1;
	for ( p = end_p; p >=start_p  ;p-- ){
		if(IS_SPACE(*p)){
			*p  = 0;
		}else{
			break;
		}
	}
#endif

	return buf;
}
static inline char* trimLeft(char* buf)
{
	int len = strlen(buf);
	char *p;

	char *start_p	= buf;
	char *end_p		= buf + len-1;
	for ( p = start_p; p <=end_p  ;p++ ){
		if(IS_SPACE(*p)){
			;
		}else{
			break;
		}
	}

	return p;
}
 void CStoreSCU::trimLeftRight(const std::string &str_in,  std::string &str_out)
{
	char _str_temp_in[256];
 
	strncpy(_str_temp_in,str_in.c_str(),256);
	
	char *char_temp = trimRight(_str_temp_in);
	char *char_temp1 = trimLeft(char_temp);
	str_out = char_temp1;
}
void CStoreSCU::setDicomStudyInfo(const std::string &patientNameRef,const std::string &patientIDRef,const std::string &patientBirthDateRef)
{
//trimLeft, trimRight
//Queue's item has space!
 
#if 0
	std::string test_out;
	std::string test_string = "   幹士 ni  ";
	trimLeftRight(test_string,test_out);
	test_string  = "   ss 幹 士 ni  ";
	trimLeftRight(test_string,test_out);
#endif

	//trimLeftRight(patientNameRef		,m_patientNameRef);	//#142_NoPatientName_NoComment
	trimLeftRight(patientIDRef			,m_patientIDRef);
	trimLeftRight(patientBirthDateRef	,m_patientBirthDateRef);
	//
	 
#endif
	m_hasDicomStudyInfo = true;
}

void CStoreSCU::openSession()
{
	m_sessionOpened = true;
}
void CStoreSCU::closeSession()
{
	m_sessionOpened = false;
}