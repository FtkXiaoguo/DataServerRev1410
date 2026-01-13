//  
//
//////////////////////////////////////////////////////////////////////

#include "DcmXTUtilMain.h"

//////////////////
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"
//#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcistrmz.h"    /* for dcmZlibExpectRFC1950Encoding */

 
#include "CheckMemoryLeak.h"

IDcmLib::LOG_LEVEL DcmXTUtilMain::m_logLevel = IDcmLib::LOGLEVEL_OFF_LOG; 
 
DcmXTUtilMain::DcmXTUtilMain()
{
	
}
DcmXTUtilMain::~DcmXTUtilMain()
{
}
void DcmXTUtilMain::Delete()
{
	destroy();
	delete this;
}
void  DcmXTUtilMain::destroy()
{
	DcmServiceListMap::iterator it = m_ProposeServiceListMap.begin();
	while(it!=m_ProposeServiceListMap.end()){
		delete (*it).second ;
		it++;
	}
 
	//
	it = m_AcceptServiceListMap.begin();
	while(it!=m_AcceptServiceListMap.end()){
		delete (*it).second ;
		it++;
	}

}
E_TransferSyntax  DcmXTUtilMain::DcmXT2E_TransferSyntax(DcmXT_TransferSyntax in)
{
	return (E_TransferSyntax) in;
}
DcmXT_TransferSyntax  DcmXTUtilMain::E2DcmXT_TransferSyntax(E_TransferSyntax in)
{
	return (DcmXT_TransferSyntax) in;
}


bool DcmXTUtilMain::getTansferSyntaxByName(const char *xferName,	DcmXTUtil::XferNames &ret) 
{
	DcmXfer xfer( xferName);

	strcpy(ret.xferID, xfer.getXferID());
    strcpy(ret.xferName	,xfer.getXferName());
    ret.xfer		= E2DcmXT_TransferSyntax(xfer.getXfer());
	return true;
}

bool DcmXTUtilMain::getTansferSyntaxBySyntax(const DcmXT_TransferSyntax dcmxt_xfer,	DcmXTUtil::XferNames &ret) 
{
	DcmXfer xfer(DcmXT2E_TransferSyntax(dcmxt_xfer));

	strcpy(ret.xferID, xfer.getXferID());
    strcpy(ret.xferName,xfer.getXferName());
    ret.xfer		= E2DcmXT_TransferSyntax(xfer.getXfer());
	
	return true;
}


 #include "dcmtk/ofstd/ofthread.h"

DcmXTMutexMain::DcmXTMutexMain()
{
	m_mutex = new OFMutex;
}
DcmXTMutexMain::~DcmXTMutexMain()
{
	if(m_mutex){
		delete m_mutex;
	}
}
 
int DcmXTMutexMain::lock()
{
	if(!m_mutex) return -1;
	return m_mutex->lock();
}
int DcmXTMutexMain::unlock()
{
  	if(!m_mutex) return -1;
	return m_mutex->unlock();
}
 

struct UIDNameMapLL {
     char* uid;
     char* nameDcmTK;
	 char* nameMC;
};

//
// It is very important that the names of the UIDs may not use the following
// characters: space  (  )  [  ], =  <  >

static  UIDNameMapLL uidNameMap[] = 
{
{ "1.2.840.10008.5.1.4.1.1.9.1.3", "AmbulatoryECGWaveformStorage", "STANDARD_WAVEFORM_AMBULATORY_ECG" },
{ "1.2.840.10008.5.1.4.1.1.88.11", "BasicTextSRStorage", "STANDARD_BASIC_TEXT_SR" },
{ "1.2.840.10008.5.1.4.1.1.9.4.1", "BasicVoiceAudioWaveformStorage", "STANDARD_WAVEFORM_BASIC_VOICE_AU" },
{ "1.2.840.10008.5.1.4.1.1.9.3.1", "CardiacElectrophysiologyWaveformStorage", "STANDARD_WAVEFORM_CARDIAC_EP" },
{ "1.2.840.10008.5.1.4.1.1.88.65", "ChestCADSRStorage", "CHEST_CAD_SR" },
{ "1.2.840.10008.5.1.4.1.1.88.33", "ComprehensiveSRStorage", "STANDARD_COMPREHENSIVE_SR" },
{ "1.2.840.10008.5.1.4.1.1.1", "ComputedRadiographyImageStorage", "STANDARD_CR" },
{ "1.2.840.10008.5.1.4.1.1.2", "CTImageStorage", "STANDARD_CT" },
{ "1.2.840.10008.5.1.4.1.1.1.3", "DigitalIntraOralXRayImageStorageForPresentation", "STANDARD_IO_PRESENT" },
{ "1.2.840.10008.5.1.4.1.1.1.3.1", "DigitalIntraOralXRayImageStorageForProcessing", "STANDARD_IO_PROCESS" },
{ "1.2.840.10008.5.1.4.1.1.1.2", "DigitalMammographyXRayImageStorageForPresentation", "STANDARD_MG_PRESENT" },
{ "1.2.840.10008.5.1.4.1.1.1.2.1", "DigitalMammographyXRayImageStorageForProcessing", "STANDARD_MG_PROCESS" },
{ "1.2.840.10008.5.1.4.1.1.1.1", "DigitalXRayImageStorageForPresentation", "STANDARD_DX_PRESENT" },
{ "1.2.840.10008.5.1.4.1.1.1.1.1", "DigitalXRayImageStorageForProcessing", "STANDARD_DX_PROCESS" },
{ "1.2.840.10008.5.1.4.1.1.2.1", "EnhancedCTImageStorage", "ENHANCED_CT_IMAGE" },
{ "1.2.840.10008.5.1.4.1.1.4.1", "EnhancedMRImageStorage", "ENHANCED_MR_IMAGE" },
{ "1.2.840.10008.5.1.4.1.1.88.22", "EnhancedSRStorage", "STANDARD_ENHANCED_SR" },
{ "1.2.840.10008.5.1.4.1.1.9.1.2", "GeneralECGWaveformStorage", "STANDARD_WAVEFORM_GENERAL_ECG" },
{ "1.2.840.10008.5.1.4.1.1.11.1", "GrayscaleSoftcopyPresentationStateStorage", "STANDARD_GRAYSCALE_SOFTCOPY_PS" },
{ "1.2.840.10008.5.1.4.38.1", "HangingProtocolStorage", "HANGING_PROTOCOL" },
{ "1.2.840.10008.5.1.4.1.1.9.2.1", "HemodynamicWaveformStorage", "STANDARD_WAVEFORM_HEMODYNAMIC" },
{ "1.2.840.10008.5.1.4.1.1.88.59", "KeyObjectSelectionDocumentStorage", "KEY_OBJECT_SELECTION_DOC" },
{ "1.2.840.10008.5.1.4.1.1.88.50", "MammographyCADSRStorage", "MAMMOGRAPHY_CAD_SR" },
{ "1.2.840.10008.1.3.10", "MediaStorageDirectoryStorage", "DICOMDIR" },
{ "1.2.840.10008.5.1.4.1.1.4", "MRImageStorage", "STANDARD_MR" },
{ "1.2.840.10008.5.1.4.1.1.4.2", "MRSpectroscopyStorage", "MR_SPECTROSCOPY" },
{ "1.2.840.10008.5.1.4.1.1.7.2", "MultiframeGrayscaleByteSecondaryCaptureImageStorage", "SC_MULTIFRAME_GRAYSCALE_BYTE" },
{ "1.2.840.10008.5.1.4.1.1.7.3", "MultiframeGrayscaleWordSecondaryCaptureImageStorage", "SC_MULTIFRAME_GRAYSCALE_WORD" },
{ "1.2.840.10008.5.1.4.1.1.7.1", "MultiframeSingleBitSecondaryCaptureImageStorage", "SC_MULTIFRAME_SINGLE_BIT" },
{ "1.2.840.10008.5.1.4.1.1.7.4", "MultiframeTrueColorSecondaryCaptureImageStorage", "SC_MULTIFRAME_TRUE_COLOR" },
{ "1.2.840.10008.5.1.4.1.1.20", "NuclearMedicineImageStorage", "STANDARD_NM" },
{ "1.2.840.10008.5.1.4.1.1.77.1.5.2", "OphthalmicPhotography16BitImageStorage", "STANDARD_OPHTHALMIC_16_BIT" },
{ "1.2.840.10008.5.1.4.1.1.77.1.5.1", "OphthalmicPhotography8BitImageStorage", "STANDARD_OPHTHALMIC_8_BIT" },
{ "1.2.840.10008.5.1.4.1.1.128", "PositronEmissionTomographyImageStorage", "STANDARD_PET" },
{ "1.2.840.10008.5.1.4.1.1.88.40", "ProcedureLogStorage", "PROCEDURE_LOG" },
{ "1.2.840.10008.5.1.4.1.1.66", "RawDataStorage", "RAW_DATA" },
{ "1.2.840.10008.5.1.4.1.1.481.4", "RTBeamsTreatmentRecordStorage", "STANDARD_RT_BEAMS_TREAT" },
{ "1.2.840.10008.5.1.4.1.1.481.6", "RTBrachyTreatmentRecordStorage", "STANDARD_RT_BRACHY_TREAT" },
{ "1.2.840.10008.5.1.4.1.1.481.2", "RTDoseStorage", "STANDARD_RT_DOSE" },
{ "1.2.840.10008.5.1.4.1.1.481.1", "RTImageStorage", "STANDARD_RT_IMAGE" },
{ "1.2.840.10008.5.1.4.1.1.481.5", "RTPlanStorage", "STANDARD_RT_PLAN" },
{ "1.2.840.10008.5.1.4.1.1.481.3", "RTStructureSetStorage", "STANDARD_RT_STRUCTURE_SET" },
{ "1.2.840.10008.5.1.4.1.1.481.7", "RTTreatmentSummaryRecordStorage", "STANDARD_RT_TREAT_SUM" },
{ "1.2.840.10008.5.1.4.1.1.7", "SecondaryCaptureImageStorage", "STANDARD_SEC_CAPTURE" },
{ "1.2.840.10008.5.1.4.1.1.66.2", "SpatialFiducialsStorage", "SPATIAL_FIDUCIALS" },
{ "1.2.840.10008.5.1.4.1.1.66.1", "SpatialRegistrationStorage", "SPATIAL_REGISTRATION" },
{ "1.2.840.10008.5.1.4.1.1.77.1.5.3", "StereometricRelationshipStorage", "STEREOMETRIC_RELATIONSHIP" },
{ "1.2.840.10008.5.1.4.1.1.9.1.1", "TwelveLeadECGWaveformStorage", "STANDARD_WAVEFORM_12_LEAD_ECG" },
{ "1.2.840.10008.5.1.4.1.1.6.1", "UltrasoundImageStorage", "STANDARD_US" },
{ "1.2.840.10008.5.1.4.1.1.3.1", "UltrasoundMultiframeImageStorage", "STANDARD_US_MF" },
{ "1.2.840.10008.5.1.4.1.1.77.1.1.1", "VideoEndoscopicImageStorage", "STANDARD_VIDEO_ENDOSCOPIC" },
{ "1.2.840.10008.5.1.4.1.1.77.1.2.1", "VideoMicroscopicImageStorage", "STANDARD_VIDEO_MICROSCOPIC" },
{ "1.2.840.10008.5.1.4.1.1.77.1.4.1", "VideoPhotographicImageStorage", "STANDARD_VIDEO_PHOTOGRAPHIC" },
{ "1.2.840.10008.5.1.4.1.1.77.1.1", "VLEndoscopicImageStorage", "STANDARD_VL_ENDOSCOPIC" },
{ "1.2.840.10008.5.1.4.1.1.77.1.2", "VLMicroscopicImageStorage", "STANDARD_VL_MICROSCOPIC" },
{ "1.2.840.10008.5.1.4.1.1.77.1.4", "VLPhotographicImageStorage", "STANDARD_VL_PHOTOGRAPHIC" },
{ "1.2.840.10008.5.1.4.1.1.77.1.3", "VLSlideCoordinatesMicroscopicImageStorage", "STANDARD_VL_SLIDE_MICROSCOPIC" },
{ "1.2.840.10008.5.1.4.1.1.12.1", "XRayAngiographicImageStorage", "STANDARD_XRAY_ANGIO" },
{ "1.2.840.10008.5.1.4.1.1.12.2", "XRayRadiofluoroscopicImageStorage", "STANDARD_XRAY_RF" },
{ "1.2.840.10008.5.1.1.30", "RETIRED_HardcopyColorImageStorage", "STANDARD_HARDCOPY_COLOR" },
{ "1.2.840.10008.5.1.1.29", "RETIRED_HardcopyGrayscaleImageStorage", "STANDARD_HARDCOPY_GRAYSCALE" },
{ "1.2.840.10008.5.1.4.1.1.5", "RETIRED_NuclearMedicineImageStorage", "STANDARD_NM_RETIRED" },
{ "1.2.840.10008.5.1.4.1.1.9", "RETIRED_StandaloneCurveStorage", "STANDARD_CURVE" },
{ "1.2.840.10008.5.1.4.1.1.10", "RETIRED_StandaloneModalityLUTStorage", "STANDARD_MODALITY_LUT" },
{ "1.2.840.10008.5.1.4.1.1.8", "RETIRED_StandaloneOverlayStorage", "STANDARD_OVERLAY" },
{ "1.2.840.10008.5.1.4.1.1.129", "RETIRED_StandalonePETCurveStorage", "STANDARD_PET_CURVE" },
{ "1.2.840.10008.5.1.4.1.1.11", "RETIRED_StandaloneVOILUTStorage", "STANDARD_VOI_LUT" },
{ "1.2.840.10008.5.1.1.27", "RETIRED_StoredPrintStorage", "STANDARD_PRINT_STORAGE" },
{ "1.2.840.10008.5.1.4.1.1.6", "RETIRED_UltrasoundImageStorage", "STANDARD_US_RETIRED" },
{ "1.2.840.10008.5.1.4.1.1.3", "RETIRED_UltrasoundMultiframeImageStorage", "STANDARD_US_MF_RETIRED" },
{ "1.2.840.10008.5.1.4.1.1.12.3", "RETIRED_XRayAngiographicBiPlaneImageStorage", "STANDARD_XRAY_ANGIO_BIPLANE" },
{ "1.2.840.10008.5.1.4.1.2.1.1", "FINDPatientRootQueryRetrieveInformationModel", "PATIENT_ROOT_QR_FIND" },
{ "1.2.840.10008.5.1.4.1.2.2.1", "FINDStudyRootQueryRetrieveInformationModel", "STUDY_ROOT_QR_FIND" },
{ "1.2.840.10008.5.1.4.1.2.1.3", "GETPatientRootQueryRetrieveInformationModel", "PATIENT_ROOT_QR_GET" },
{ "1.2.840.10008.5.1.4.1.2.2.3", "GETStudyRootQueryRetrieveInformationModel", "STUDY_ROOT_QR_GET" },
{ "1.2.840.10008.5.1.4.1.2.1.2", "MOVEPatientRootQueryRetrieveInformationModel", "PATIENT_ROOT_QR_MOVE" },
{ "1.2.840.10008.5.1.4.1.2.2.2", "MOVEStudyRootQueryRetrieveInformationModel", "STUDY_ROOT_QR_MOVE" },
{ "1.2.840.10008.5.1.4.1.2.3.1", "RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel", "PATIENT_STUDY_ONLY_QR_FIND" },
{ "1.2.840.10008.5.1.4.1.2.3.3", "RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel", "PATIENT_STUDY_ONLY_QR_GET" },
{ "1.2.840.10008.5.1.4.1.2.3.2", "RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel", "PATIENT_STUDY_ONLY_QR_MOVE" },
{ "1.2.840.10008.5.1.4.32.1", "FINDGeneralPurposeWorklistInformationModel", "G_P_WORKLIST" },
{ "1.2.840.10008.5.1.4.31", "FINDModalityWorklistInformationModel", "MODALITY_WORKLIST_FIND" },
{ "1.2.840.10008.5.1.4.32.3", "GeneralPurposePerformedProcedureStepSOPClass", "G_P_PERFORMED_PROCEDURE_STEP" },
{ "1.2.840.10008.5.1.4.32.2", "GeneralPurposeScheduledProcedureStepSOPClass", "G_P_SCHEDULED_PROCEDURE_STEP" },
{ "1.2.840.10008.5.1.4.32", "GeneralPurposeWorklistManagementMetaSOPClass", "G_P_WORKLIST_MANAGEMENT_META" },
{ "1.2.840.10008.3.1.2.3.5", "ModalityPerformedProcedureStepNotificationSOPClass", "PERFORMED_PROCEDURE_STEP_NOTIFY" },
{ "1.2.840.10008.3.1.2.3.4", "ModalityPerformedProcedureStepRetrieveSOPClass", "PERFORMED_PROCEDURE_STEP_RETR" },
{ "1.2.840.10008.3.1.2.3.3", "ModalityPerformedProcedureStepSOPClass", "PERFORMED_PROCEDURE_STEP" },
{ "1.2.840.10008.1.20.2", "RETIRED_StorageCommitmentPullModelSOPClass", "STORAGE_COMMITMENT_PULL" },
{ "1.2.840.10008.1.20.1", "StorageCommitmentPushModelSOPClass", "STORAGE_COMMITMENT_PUSH" },
{ "1.2.840.10008.5.1.4.38.2", "FINDHangingProtocolInformationModel", "HANGING_PROTOCOL_FIND" },
{ "1.2.840.10008.5.1.4.38.3", "MOVEHangingProtocolInformationModel", "HANGING_PROTOCOL_MOVE" },
{ "1.2.840.10008.5.1.4.37.2", "BreastImagingRelevantPatientInformationQuery", "BREAST_IMAGING_RPI_QUERY" },
{ "1.2.840.10008.5.1.4.37.3", "CardiacRelevantPatientInformationQuery", "CARDIAC_RPI_QUERY" },
{ "1.2.840.10008.5.1.4.37.1", "GeneralRelevantPatientInformationQuery", "GENERAL_RPI_QUERY" },
{ "1.2.840.10008.5.1.1.15", "BasicAnnotationBoxSOPClass", "BASIC_ANNOTATION_BOX" },
{ "1.2.840.10008.5.1.1.4.1", "BasicColorImageBoxSOPClass", "BASIC_COLOR_IMAGE_BOX" },
{ "1.2.840.10008.5.1.1.18", "BasicColorPrintManagementMetaSOPClass", "BASIC_COLOR_PRINT_MANAGEMENT" },
{ "1.2.840.10008.5.1.1.2", "BasicFilmBoxSOPClass", "BASIC_FILM_BOX" },
{ "1.2.840.10008.5.1.1.1", "BasicFilmSessionSOPClass", "BASIC_FILM_SESSION" },
{ "1.2.840.10008.5.1.1.4", "BasicGrayscaleImageBoxSOPClass", "BASIC_GRAYSCALE_IMAGE_BOX" },
{ "1.2.840.10008.5.1.1.9", "BasicGrayscalePrintManagementMetaSOPClass", "BASIC_GRAYSCALE_PRINT_MANAGEMENT" },
{ "1.2.840.10008.5.1.1.23", "PresentationLUTSOPClass", "PRESENTATION_LUT" },
{ "1.2.840.10008.5.1.1.14", "PrintJobSOPClass", "PRINT_JOB" },
{ "1.2.840.10008.5.1.1.16.376", "PrinterConfigurationRetrievalSOPClass", "PRINTER_CONFIGURATION" },
{ "1.2.840.10008.5.1.1.16", "PrinterSOPClass", "PRINTER" },
{ "1.2.840.10008.5.1.1.24.1", "RETIRED_BasicPrintImageOverlayBoxSOPClass", "BASIC_PRINT_IMAGE_OVERLAY_BOX" },
{ "1.2.840.10008.5.1.1.24", "RETIRED_ImageOverlayBoxSOPClass", "IMAGE_OVERLAY_BOX_RETIRED" },
{ "1.2.840.10008.5.1.1.26", "RETIRED_PrintQueueManagementSOPClass", "PRINT_QUEUE_MANAGEMENT" },
{ "1.2.840.10008.5.1.1.31", "RETIRED_PullPrintRequestSOPClass", "PULL_PRINT_REQUEST" },
{ "1.2.840.10008.5.1.1.32", "RETIRED_PullStoredPrintManagementMetaSOPClass", "PULL_STORED_PRINT_MANAGEMENT" },
{ "1.2.840.10008.5.1.1.18.1", "RETIRED_ReferencedColorPrintManagementMetaSOPClass", "REF_COLOR_PRINT_MANAGEMENT" },
{ "1.2.840.10008.5.1.1.9.1", "RETIRED_ReferencedGrayscalePrintManagementMetaSOPClass", "REF_GRAYSCALE_PRINT_MANAGEMENT" },
{ "1.2.840.10008.5.1.1.4.2", "RETIRED_ReferencedImageBoxSOPClass", "REFERENCED_IMAGE_BOX" },
{ "1.2.840.10008.5.1.1.22", "VOILUTBoxSOPClass", "VOI_LUT_BOX" },
{ "1.2.840.10008.3.1.2.6.1", "RETIRED_DetachedInterpretationManagementSOPClass", "DETACHED_INTERP_MANAGEMENT" },
{ "1.2.840.10008.3.1.2.1.4", "RETIRED_DetachedPatientManagementMetaSOPClass", "DETACHED_PATIENT_MANAGEMENT_META" },
{ "1.2.840.10008.3.1.2.1.1", "RETIRED_DetachedPatientManagementSOPClass", "DETACHED_PATIENT_MANAGEMENT" },
{ "1.2.840.10008.3.1.2.5.4", "RETIRED_DetachedResultsManagementMetaSOPClass", "DETACHED_RESULTS_MANAGEMENT_META" },
{ "1.2.840.10008.3.1.2.5.1", "RETIRED_DetachedResultsManagementSOPClass", "DETACHED_RESULTS_MANAGEMENT" },
{ "1.2.840.10008.3.1.2.5.5", "RETIRED_DetachedStudyManagementMetaSOPClass", "STUDY_MANAGEMENT" },
{ "1.2.840.10008.3.1.2.3.1", "RETIRED_DetachedStudyManagementSOPClass", "DETACHED_STUDY_MANAGEMENT" },
{ "1.2.840.10008.3.1.2.2.1", "RETIRED_DetachedVisitManagementSOPClass", "DETACHED_VISIT_MANAGEMENT" },
{ "1.2.840.10008.1.40", "ProceduralEventLoggingSOPClass", "PROCEDURAL_EVENT_LOGGING" },
{ "1.2.840.10008.5.1.1.33", "MediaCreationManagementSOPClass", "MEDIA_CREATION_MANAGEMENT" },
{ "1.2.840.10008.5.1.4.33", "InstanceAvailabilityNotificationSOPClass", "INSTANCE_AVAIL_NOTIFICATION" },
{ "1.2.840.10008.1.9", "RETIRED_BasicStudyContentNotificationSOPClass", "STUDY_CONTENT_NOTIFICATION" },
{ "1.2.840.10008.3.1.2.3.2", "RETIRED_StudyComponentManagementSOPClass", "STUDY_COMPONENT_MANAGEMENT" },
{ "1.2.840.10008.1.1", "VerificationSOPClass", "STANDARD_ECHO" },

{ NULL, NULL,NULL }

};
static const int uidNameMap_size = ( sizeof(uidNameMap) / sizeof(UIDNameMapLL) );

 char * DcmXTUtilMain::getNameOfUID(const char* uid)
{
	int i = 0;
    if (uid == NULL) return 0;
    for (i=0; i<uidNameMap_size; i++) {
      if (uidNameMap[i].uid != NULL && strcmp(uid, uidNameMap[i].uid) == 0) {
        return uidNameMap[i].nameMC;
      }
    }
    return 0;

	
//	return dcmFindNameOfUID( uid);
}


 char *DcmXTUtilMain::getUIDOfName(const char* name)
{
	int i = 0;
    if (name == NULL) return 0;
    for (i=0; i<uidNameMap_size; i++) {
      if (uidNameMap[i].nameMC != NULL && strcmp(name, uidNameMap[i].nameMC) == 0) {
        return uidNameMap[i].uid;
      }
    }
    return 0;
}

		
//#include <log4cplus/logger.h>
//#include <log4cplus/configurator.h>
//#include <iomanip>
#include "dcmtk/oflog/logger.h"
#include "dcmtk/oflog/fileap.h"
 
 #include<fstream>   
#include<process.h>   
#include<iostream>


//static log4cplus::FileAppender *_dcmAPIErr_fileAp_=0;
void  DcmXTUtilMain::DcmAPIError(const char* name)
{

#if 1
	getApiLogger()->LoggerError("DcmAPI not suppurt : %s \n",name);
#else
	log4cplus::Logger logger = log4cplus::Logger::getInstance("dcmAPIErr.log");
//	logger.forcedLog(OFLogger::ERROR_LOG_LEVEL,name) ;
//
	if(!_dcmAPIErr_fileAp_){
		_dcmAPIErr_fileAp_ = new log4cplus::FileAppender("dcmAPIErr.log",
	 	//	LOG4CPLUS_FSTREAM_NAMESPACE::ios::_Nocreate |
			LOG4CPLUS_FSTREAM_NAMESPACE::ios::app 
			);

		log4cplus::SharedAppenderPtr tt(_dcmAPIErr_fileAp_);
		 
		logger.addAppender(tt);
	}
	
	{
 

		LOG4CPLUS_ERROR(logger,   " "
							   << name
							   << std::endl  ); 
	 


	}
#endif
	 
}

#if 0
static log4cplus::FileAppender *_DcmLib_fileAp_=0;
log4cplus::Logger _FxDcmLibLogger_ ;
void DcmXTUtilMain::openLogger(const char *fileName,IDcmLib::LOG_LEVEL log_level)
{
 //   FxDcmLibLogger = log4cplus::Logger::getInstance("dcmAPIErr1.log");
	_FxDcmLibLogger_ = log4cplus::Logger::getInstance("DcmLibLogger");//fileName);
	
	if(!_DcmLib_fileAp_){
		 
	//	_DcmLib_fileAp_ = new log4cplus::FileAppender("dcmAPIErr1.log",
		_DcmLib_fileAp_ = new log4cplus::FileAppender(fileName,
	//		LOG4CPLUS_FSTREAM_NAMESPACE::ios::_Nocreate |
			LOG4CPLUS_FSTREAM_NAMESPACE::ios::app 
			);

		log4cplus::SharedAppenderPtr tt(_DcmLib_fileAp_);
		 
		_FxDcmLibLogger_.addAppender(tt);

 
		switch(log_level){
			case IDcmLib::LOGLEVEL_OFF_LOG:
				_FxDcmLibLogger_.setLogLevel(log4cplus::OFF_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_FALTAL:
				_FxDcmLibLogger_.setLogLevel(log4cplus::FATAL_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_ERROR:
				_FxDcmLibLogger_.setLogLevel(log4cplus::ERROR_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_WARN:
				_FxDcmLibLogger_.setLogLevel(log4cplus::WARN_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_INFO:
				_FxDcmLibLogger_.setLogLevel(log4cplus::INFO_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_DEBUG:
				_FxDcmLibLogger_.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_TRACE:
				_FxDcmLibLogger_.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_ALL:
				_FxDcmLibLogger_.setLogLevel(log4cplus::ALL_LOG_LEVEL);
				break;
			case IDcmLib::LOGLEVEL_NOT_SET:
				_FxDcmLibLogger_.setLogLevel(log4cplus::NOT_SET_LOG_LEVEL);
				break;
				 
		}
		m_logLevel = log_level;
 
		LOG4CPLUS_ERROR(_FxDcmLibLogger_,   " "
							   << " ---------start DcmAPI ------------ \n"
							   << std::endl  ); 
	}
}
#endif



class DcmLibApiLoggerNull : public DcmLibApiLogger {
public:
	DcmLibApiLoggerNull() { };
	~DcmLibApiLoggerNull() { };
	//
//	virtual void Logger(int id,const char *str) = 0;
	virtual void LoggerError(const char *format,...){ };
	virtual void LoggerWarn(const char *format,...){ };
	virtual void LoggerDebug(const char *format,...){ };
	virtual void LoggerTrace(const char *format,...){ };
	int		getLoggerLevel(){ return IDcmLib::LOGLEVEL_OFF_LOG;};
};
static DcmLibApiLoggerNull __DcmLibLogger_instance_null;
static XTDcmLib::DcmLibApiLogger *_DcmLibLogger_instance_ = &__DcmLibLogger_instance_null;
//IDcmLib::LOG_LEVEL _DcmLibLogger_level = IDcmLib::LOGLEVEL_OFF_LOG;

void DcmXTUtilMain::setupApiLogger(XTDcmLib::DcmLibApiLogger *logger,IDcmLib::LOG_LEVEL log_level)
{
	_DcmLibLogger_instance_ = logger;
//	_DcmLibLogger_level = log_level;
	m_logLevel = log_level;
}
XTDcmLib::DcmLibApiLogger *DcmXTUtilMain::getApiLogger()
{
	return _DcmLibLogger_instance_;
}

//#18 2012/05/17 K.Ko
int g_WriteDicomFileBufferLen = 64*1024;
void DcmXTUtilMain::setupWriteBufferLen(int len)
{
	g_WriteDicomFileBufferLen = len;
}
int g_ReadDicomFileBufferLen = 64*1024;
void DcmXTUtilMain::setupReadeBufferLen(int len)
{
	g_ReadDicomFileBufferLen = len;
}
int DcmXTUtilMain::getWriteBufferLen()
{
	return g_WriteDicomFileBufferLen;
}
int DcmXTUtilMain::getReadeBufferLen(){
	return g_ReadDicomFileBufferLen;
}

#if 0
bool DcmXTUtilMain::isEnableAPILog()
{
//	return (_DcmLib_fileAp_!=0);
	return (_DcmLibLogger_instance_!=0);
}

void LoggerOutput(int id, const char* ifmt, va_list arguments, const char* iPrefix=0)
{
	if(!_DcmLibLogger_instance_) return;

	char mbuf[4096*4];
	vsprintf(mbuf, ifmt, arguments);
	_DcmLibLogger_instance_->Logger(id,mbuf);
}

void DcmXTUtilMain::LoggerError(const char *format,...)
{
	try
	{
		va_list args;
		va_start(args, format);
		LoggerOutput(IDcmLib::LOGLEVEL_ERROR,format, args);
		va_end(args);
	}
	catch (...)
	{
	}
 
}
void DcmXTUtilMain::LoggerWarn(const char *format,...)
{
	try
	{
		va_list args;
		va_start(args, format);
		LoggerOutput(IDcmLib::LOGLEVEL_WARN,format, args);
		va_end(args);
	}
	catch (...)
	{
	}
}
void DcmXTUtilMain::LoggerDebug(const char *format,...)
{
	try
	{
		va_list args;
		va_start(args, format);
		LoggerOutput(IDcmLib::LOGLEVEL_DEBUG,format, args);
		va_end(args);
	}
	catch (...)
	{
	}
}
void DcmXTUtilMain::LoggerTrace(const char *format,...)
{
	try
	{
		va_list args;
		va_start(args, format);
		LoggerOutput(IDcmLib::LOGLEVEL_TRACE,format, args);
		va_end(args);
	}
	catch (...)
	{
	}
}
#endif

#if 0
#include <sys/timeb.h>
const char* my_YYYYMMDDHHMMSSTimeStamp()
{
#define STR_MAX_LEN (36)
	static char my_out[64][STR_MAX_LEN];
//	static  TRAtomicVar<int> my_sK;
	static int my_sK =0;
	try {
		int k = my_sK++;
		char *buf = my_out[k&63];
		struct _timeb timebuf;
		_ftime(&timebuf);
		struct tm tm = *localtime(&timebuf.time);

		tm.tm_mon += 1;
		tm.tm_year += 1900;
		const char* format = "%4d/%02d/%02d %02d:%02d:%02d";
		
		sprintf(buf, format, tm.tm_year,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min, tm.tm_sec, timebuf.millitm);
		buf[STR_MAX_LEN-1] = '\0';
		++k;
		return buf;
	}catch(...){
		return "";
	}

}

std::string DcmXTUtilMain::getLogPrefix()
{
	char _char_buff[2048];
	unsigned long threadID = ::GetCurrentThreadId();
	sprintf(_char_buff,"%s[%d]",my_YYYYMMDDHHMMSSTimeStamp(),threadID);
	return _char_buff;
}
#endif
//////////////

typedef struct {
	MC_VR mc_vr;
	DcmEVR dcmEvr;
} MC_DCM_EVR_Entry;

MC_DCM_EVR_Entry MC_DCM_VR_Table[]={
	AE	, EVR_AE,
	AS, EVR_AS,
	CS, EVR_CS,
	DA, EVR_DA,
	DS, EVR_DS,
	DT, EVR_DT,
	IS, EVR_IS,
	LO, EVR_LO,
	LT, EVR_LT,
	PN, EVR_PN,
	SH, EVR_SH,
	ST, EVR_ST,
	TM, EVR_TM,
	UT, EVR_UT,
	UI, EVR_UI,
	SS, EVR_SS,
	US, EVR_US,
	AT, EVR_AT,
	SL, EVR_SL,
	UL, EVR_UL,
	FL, EVR_FL,
	FD, EVR_FD,
	UNKNOWN_VR, EVR_UNKNOWN,
	OB, EVR_OB,
	OW, EVR_OW,
//	OL, EVR_OL,
	OF, EVR_OF,
	SQ, EVR_SQ
};
/*
    AS, CS, DA, DS, DT, IS, LO, LT, PN, SH, ST, TM, UT,
     UI, SS, US, AT, SL, UL, FL, FD, UNKNOWN_VR, OB, OW, OL, OF, SQ
	 */
 /*

 */
DcmEVR DcmXTUtilMain::ConvtMCVr2DcmEVr(MC_VR mc_vr)
{
	DcmEVR ret_vr=EVR_UNKNOWN;

	int table_size = sizeof(MC_DCM_VR_Table)/sizeof(MC_DCM_EVR_Entry);
	for(int i=0;i<table_size;i++){
		if(MC_DCM_VR_Table[i].mc_vr == mc_vr){
			ret_vr = MC_DCM_VR_Table[i].dcmEvr;
			break;
		}
	}

	return ret_vr;
}
MC_VR DcmXTUtilMain::ConvtDcmEVr2MCVr( DcmEVR vr)
{
	MC_VR ret_vr = UNKNOWN_VR;
	int table_size = sizeof(MC_DCM_VR_Table)/sizeof(MC_DCM_EVR_Entry);
	for(int i=0;i<table_size;i++){
		if(MC_DCM_VR_Table[i].dcmEvr == vr){
			ret_vr = MC_DCM_VR_Table[i].mc_vr;
			break;
		}
	}
	return ret_vr;
}

bool DcmXTUtilMain::AddPrivateCreator2GDict(unsigned short g,const char*    AprivateCode,unsigned short e )
{
	
	DcmDataDictionary &globalDataDict = dcmDataDict.wrlock();

	const DcmDictEntry *find_entry=0 ;

	 
	//
	DcmTagKey tagKey(g,e);
	if(!(find_entry=globalDataDict.findEntry(tagKey,AprivateCode))){
  		DcmDictEntry *new_entry = new DcmDictEntry(g,  e,  EVR_LO, "PrivateCreator",
										1/*vmMin*/,-1/*vmMax*/,
										0/*vers*/,OFTrue,
										AprivateCode);
		globalDataDict.addEntry(new_entry);
		//
		find_entry = globalDataDict.findEntry(tagKey,AprivateCode);
		 
	}
	dcmDataDict.unlock();

	return  (find_entry != 0);

}
bool DcmXTUtilMain::Add_PrivateTag(unsigned short g,unsigned short e, MC_VR Avr,const char* AprivateCode)
{
	return AddPrivateTag2GDict(g,e,Avr,AprivateCode);
}



bool DcmXTUtilMain::AddPrivateTag2GDict(unsigned short g,unsigned short e, MC_VR Avr,const char* AprivateCode)
{
	DcmDataDictionary &globalDataDict = dcmDataDict.wrlock();

	const DcmDictEntry *find_entry=0 ;

	 
	//
	DcmTagKey tagKey(g,e);
	if(!(find_entry=globalDataDict.findEntry(tagKey,AprivateCode))){
		DcmDictEntry *new_entry = new DcmDictEntry(g,  e,  DcmXTUtilMain::ConvtMCVr2DcmEVr(Avr), "pname",
										1/*vmMin*/,-1/*vmMax*/,
										0/*vers*/,OFTrue,
										AprivateCode);
		globalDataDict.addEntry(new_entry);
		//
		find_entry = globalDataDict.findEntry(tagKey,AprivateCode);
	}
	
	
	dcmDataDict.unlock();

	return  find_entry!=0;
}

void DcmXTUtilMain::initDefaultServiceList(const std::string &newName,bool Propose)
{
	DcmServiceListEntry *new_list = new DcmServiceListEntry;
	new_list->networkTransferSyntax = EXS_LittleEndianImplicit;
	new_list->maxPDUSize = 0;

	
#if 1
	new_list->SOPClassUIDsSize = 0;

	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_CTImageStorage);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_XRayAngiographicImageStorage);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_XRayRadiofluoroscopicImageStorage);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_XRayRadiationDoseSRStorage);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_SecondaryCaptureImageStorage);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_FINDPatientRootQueryRetrieveInformationModel);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_FINDStudyRootQueryRetrieveInformationModel);
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel);
	
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_MOVEStudyRootQueryRetrieveInformationModel);

	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_DigitalXRayImageStorageForPresentation); // #23 2012/07/07 K.KO for DX
	strcpy(new_list->SOPClassUIDs[new_list->SOPClassUIDsSize++],UID_DigitalXRayImageStorageForProcessing);   // #23 2012/07/07 K.KO for DX
 
#else
	new_list->SOPClassUIDs.push_back( UID_CTImageStorage );

 
	new_list->SOPClassUIDs.push_back( UID_XRayAngiographicImageStorage );
	new_list->SOPClassUIDs.push_back( UID_XRayRadiofluoroscopicImageStorage );
	new_list->SOPClassUIDs.push_back( UID_XRayRadiationDoseSRStorage );
	new_list->SOPClassUIDs.push_back( UID_XRayRadiationDoseSRStorage );
	new_list->SOPClassUIDs.push_back( UID_SecondaryCaptureImageStorage );
	new_list->SOPClassUIDs.push_back( UID_FINDPatientRootQueryRetrieveInformationModel );
	new_list->SOPClassUIDs.push_back( UID_FINDStudyRootQueryRetrieveInformationModel );
	new_list->SOPClassUIDs.push_back( UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel );
#endif
 
	if(Propose){
		m_ProposeServiceListMap[newName] = new_list;
	}else{
		m_AcceptServiceListMap[newName] = new_list;
	}
}
bool DcmXTUtilMain::clearServiceList(bool isPropose,const char *ServiceName)
{
	DcmServiceListEntry *serviceList = DcmXTUtilMain::getServiceList( ServiceName, isPropose);

	if(!serviceList) return false;
	for(int i=0;i<SOPClassUIDsMax;i++){
		serviceList->SOPClassUIDs[i][0] = 0;
	}
	serviceList->SOPClassUIDsSize = 0;
	return true;
}
bool DcmXTUtilMain::createServiceList(bool isPropose,const char *name )
{
	 
	initDefaultServiceList(name,isPropose/*propose*/);
	return true;
}
#if 0
bool DcmXTUtilMain::createAcceptServiceList(const char *name )
{
	 
	initDefaultServiceList(name,false/*propose*/);
	return true;
}

#endif
DcmServiceListEntry *DcmXTUtilMain::getServiceList(std::string name,bool Propose)
{
	DcmServiceListEntry *ret_entry = 0;

	if(Propose){
		DcmServiceListMap::iterator it = m_ProposeServiceListMap.begin();
		while(it!=m_ProposeServiceListMap.end()){
			if((*it).first == name){
				ret_entry = (*it).second;
			}
			it++;
		}
	}else{
		DcmServiceListMap::iterator it = m_AcceptServiceListMap.begin();
		while(it!=m_AcceptServiceListMap.end()){
			if((*it).first == name){
				ret_entry = (*it).second;
			}
			it++;
		}
	}
	return ret_entry;
}

bool DcmXTUtilMain::setNetworkTransferSyntax(bool isPropose,DcmXT_TransferSyntax xfer,const char *ServiceName)
{
	DcmServiceListEntry *sericeList = getServiceList(ServiceName,isPropose/*propose*/);
	if(sericeList){
		sericeList->networkTransferSyntax =  DcmXT2E_TransferSyntax(xfer) ;
		return true;
	}else{
		return false;
	}
}
bool DcmXTUtilMain::setMaxPDUSize(bool isPropose,int size,const char *ServiceName) //2012/03/09 K.Ko
{
	DcmServiceListEntry *sericeList = getServiceList(ServiceName,isPropose/*propose*/);
	if(sericeList){
		sericeList->maxPDUSize =  size  ;
		return true;
	}else{
		return false;
	}
}
bool DcmXTUtilMain::addSOPClassUID(bool isPropose,const char * sopUID,const char *ServiceName)
{
	DcmServiceListEntry *sericeList = getServiceList(ServiceName,isPropose/*propose*/);
	if(sericeList){
//		sericeList->SOPClassUIDs.push_back(sopUID);  
		strcpy(sericeList->SOPClassUIDs[sericeList->SOPClassUIDsSize++],sopUID);

		return true;
	}else{
		return false;
	}
}
const char *DcmXTUtilMain::getTagName(unsigned long  tag) //2012/03/09 K.Ko 
{
	const char * ret_str = 0;

	
	DcmDataDictionary &globalDataDict = dcmDataDict.wrlock();

	const DcmDictEntry *find_entry=0 ;

	 
	//
	Uint16 tag_g = (tag&(0xffff0000) ) >>16;
	Uint16 tag_e = (tag&(0x0000ffff) );
	DcmTagKey tagKey(tag_g,tag_e);
	if((find_entry=globalDataDict.findEntry(tagKey,0))){
		if(find_entry){
			ret_str = find_entry->getTagName();
		}
		 
	}
	dcmDataDict.unlock();

	return ret_str;
}
#if 0
bool DcmXTUtilMain::addAcceptSOPClassUID(const char * sopUID,const char *ServiceName)
{
	DcmServiceListEntry *sericeList = getServiceList(ServiceName,false/*propose*/);
	if(sericeList){
		sericeList->SOPClassUIDs.push_back(sopUID);  
		return true;
	}else{
		return false;
	}
}
#endif


DcmServiceListEntry::DcmServiceListEntry()
{
	SOPClassUIDs = new char*[SOPClassUIDsMax];
	for(int i=0;i<SOPClassUIDsMax;i++){
		SOPClassUIDs[i] = new char[65];
		SOPClassUIDs[i][0] = 0;
	}
	SOPClassUIDsSize = 0;
	//
	networkTransferSyntax = EXS_Unknown;
}
DcmServiceListEntry::~DcmServiceListEntry()
{
	for(int i=0;i<SOPClassUIDsMax;i++){
		delete [] SOPClassUIDs[i];
	}
	delete [] SOPClassUIDs;
	SOPClassUIDsSize = 0;
}