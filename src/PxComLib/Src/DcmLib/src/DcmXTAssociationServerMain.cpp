// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include<WinSock2.h>
#include "DICTION.H"
#include "DcmXTAssociationServerMain.h"

#include "DcmXTApplicationInstanceMain.h"

#include "DcmXTDicomMessageMain.h"
//////////////////
 
#include "dcmtk/ofstd/ofstdinc.h"

#include "AssociationHelp.h"

#include "DcmXTUtilMain.h"

#include "CheckMemoryLeak.h"
#include "FXDcmLibLogger.h"

/** an array of const strings containing all known Storage SOP Classes
 *  that fit into the conventional PATIENT-STUDY-SERIES-INSTANCE information
 *  model, i.e. everything a Storage SCP might want to store in a PACS.
 *  Special cases such as hanging protocol storage or the Storage SOP Class
 *  are not included in this list.
 *
 *  THIS LIST CONTAINS ALL STORAGE SOP CLASSES INCLUDING RETIRED ONES
 *  AND IS LARGER THAN 64 ENTRIES.
 */

const char* my_dcmAllStorageSOPClassUIDs[] =
{
#if 0
    UID_AmbulatoryECGWaveformStorage,
    UID_BasicTextSR,
    UID_BasicVoiceAudioWaveformStorage,
    UID_BlendingSoftcopyPresentationStateStorage,
#endif
    UID_CTImageStorage,
#if 0
    UID_CardiacElectrophysiologyWaveformStorage,
    UID_ChestCADSR,
    UID_ColorSoftcopyPresentationStateStorage,
    UID_ComprehensiveSR,
    UID_ComputedRadiographyImageStorage,
    UID_DRAFT_SRAudioStorage,
    UID_DRAFT_SRComprehensiveStorage,
    UID_DRAFT_SRDetailStorage,
    UID_DRAFT_SRTextStorage,
    UID_DRAFT_WaveformStorage,
    UID_DigitalIntraOralXRayImageStorageForPresentation,
    UID_DigitalIntraOralXRayImageStorageForProcessing,
    UID_DigitalMammographyXRayImageStorageForPresentation,
    UID_DigitalMammographyXRayImageStorageForProcessing,
    UID_DigitalXRayImageStorageForPresentation,
    UID_DigitalXRayImageStorageForProcessing,
    UID_EncapsulatedPDFStorage,
    UID_EnhancedCTImageStorage,
    UID_EnhancedMRImageStorage,
    UID_EnhancedSR,
    UID_EnhancedXAImageStorage,
    UID_EnhancedXRFImageStorage,
    UID_GeneralECGWaveformStorage,
    UID_GrayscaleSoftcopyPresentationStateStorage,
    UID_HardcopyColorImageStorage,
    UID_HardcopyGrayscaleImageStorage,
    UID_HemodynamicWaveformStorage,
    UID_KeyObjectSelectionDocument,
    UID_MRImageStorage,
    UID_MRSpectroscopyStorage,
    UID_MammographyCADSR,
    UID_MultiframeGrayscaleByteSecondaryCaptureImageStorage,
    UID_MultiframeGrayscaleWordSecondaryCaptureImageStorage,
    UID_MultiframeSingleBitSecondaryCaptureImageStorage,
    UID_MultiframeTrueColorSecondaryCaptureImageStorage,
    UID_NuclearMedicineImageStorage,
    UID_OphthalmicPhotography16BitImageStorage,
    UID_OphthalmicPhotography8BitImageStorage,
    UID_PETCurveStorage,
    UID_PETImageStorage,
    UID_ProcedureLogStorage,
    UID_PseudoColorSoftcopyPresentationStateStorage,
    UID_RETIRED_NuclearMedicineImageStorage,
    UID_RETIRED_UltrasoundImageStorage,
    UID_RETIRED_UltrasoundMultiframeImageStorage,
    UID_RETIRED_VLImageStorage,
    UID_RETIRED_VLMultiFrameImageStorage,
    UID_RETIRED_XRayAngiographicBiPlaneImageStorage,
    UID_RTBeamsTreatmentRecordStorage,
    UID_RTBrachyTreatmentRecordStorage,
    UID_RTDoseStorage,
    UID_RTImageStorage,
    UID_RTPlanStorage,
    UID_RTStructureSetStorage,
    UID_RTTreatmentSummaryRecordStorage,
    UID_RawDataStorage,
    UID_RealWorldValueMappingStorage,
    UID_SecondaryCaptureImageStorage,
    UID_SpatialFiducialsStorage,
    UID_SpatialRegistrationStorage,
    UID_StandaloneCurveStorage,
    UID_StandaloneModalityLUTStorage,
    UID_StandaloneOverlayStorage,
    UID_StandaloneVOILUTStorage,
    UID_StereometricRelationshipStorage,
    UID_StoredPrintStorage,
    UID_TwelveLeadECGWaveformStorage,
    UID_UltrasoundImageStorage,
    UID_UltrasoundMultiframeImageStorage,
    UID_VLEndoscopicImageStorage,
    UID_VLMicroscopicImageStorage,
    UID_VLPhotographicImageStorage,
    UID_VLSlideCoordinatesMicroscopicImageStorage,
    UID_VideoEndoscopicImageStorage,
    UID_VideoMicroscopicImageStorage,
    UID_VideoPhotographicImageStorage,
#endif
    UID_XRayAngiographicImageStorage,
//  UID_XRayFluoroscopyImageStorage,
	UID_XRayRadiofluoroscopicImageStorage,
//   UID_XRayRadiationDoseSR,
	UID_XRayRadiationDoseSRStorage,
//
UID_SecondaryCaptureImageStorage,

///////
 UID_FINDModalityWorklistInformationModel,              
 UID_FINDPatientRootQueryRetrieveInformationModel,      
 UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel, 
 UID_FINDStudyRootQueryRetrieveInformationModel,        
 UID_GETPatientRootQueryRetrieveInformationModel,       
 UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel,  
 UID_GETStudyRootQueryRetrieveInformationModel,         
 UID_MOVEPatientRootQueryRetrieveInformationModel,      
 UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel, 
 UID_MOVEStudyRootQueryRetrieveInformationModel,        
 //UID_FINDGeneralPurposeWorklistInformationModel,     
 //
 UID_DigitalIntraOralXRayImageStorageForPresentation,
/////////
    NULL
};

const int my_numberOfAllDcmStorageSOPClassUIDs = (sizeof(my_dcmAllStorageSOPClassUIDs) / sizeof(const char*)) - 1;



DcmXTAssociationServerMain::DcmXTAssociationServerMain(T_ASC_Association *aso )
{
 m_association = aso;
 m_ApplicationInstanceMain = 0;

 m_serverHelper = new AssociationHelpServer;
 m_serverHelper->setupLogLevel(DcmXTUtilMain::getLogLevel());
 
// m_CurStorageSOPClassUID = 0;
 //m_CurStorageSOPClassUIDSize = 0;

 m_AcceptServiceList = 0;
}
DcmXTAssociationServerMain::~DcmXTAssociationServerMain()
{
	int x= 0;
 
	delete m_serverHelper;
//	destroyCurStorageSOPClassUID();
}


	//
void DcmXTAssociationServerMain::Delete()
{
	destroy();
	delete this;
}
bool DcmXTAssociationServerMain::abort()
{
	if(m_association){ //2012/02/13 K.Ko
		ASC_abortAssociation(m_association);
		return destroy();
	}
	return DcmXTAssociationMain::abort();
	 
}
 void DcmXTAssociationServerMain::setAcceptServiceList(const DcmServiceListEntry *seriveEntry)
{
	
	if(!seriveEntry) {
		DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::setAcceptServiceList seriveEntry==null \n");
		return ;
	}

	DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::setAcceptServiceList \n");

#if 0
	destroyCurStorageSOPClassUID();
	m_CurStorageSOPClassUIDSize = seriveEntry->SOPClassUIDs.size();

	m_CurStorageSOPClassUID = new const char*[m_CurStorageSOPClassUIDSize];
	for(int i=0;i<m_CurStorageSOPClassUIDSize;i++){
		std::string str_item = seriveEntry->SOPClassUIDs[i];
		m_CurStorageSOPClassUID[i] = str_item.c_str();

		DCMLIB_LOG_TRACE("m_CurStorageSOPClassUID[%d] %s \n",i,str_item.c_str());
	}
#else
	m_AcceptServiceList = seriveEntry;
#endif
	//
	if(EXS_Unknown != seriveEntry->networkTransferSyntax){
		m_serverHelper->setNetworkTransferSyntax(seriveEntry->networkTransferSyntax);
	}
	if(seriveEntry->maxPDUSize>=4*1024){
		m_serverHelper->setMaxPDUSize( seriveEntry->maxPDUSize);
	}
	 
}
#if 0
 void DcmXTAssociationServerMain::destroyCurStorageSOPClassUID() 
{
	DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::destroyCurStorageSOPClassUID \n");

	if(m_CurStorageSOPClassUIDSize<1) return;

	// m_CurStorageSOPClassUID[i] is reference point
	delete [] m_CurStorageSOPClassUID;
	m_CurStorageSOPClassUID = 0;
	m_CurStorageSOPClassUIDSize  =0 ;

}
#endif

bool DcmXTAssociationServerMain::destroy()
{

	if(m_association){
		 
		ASC_dropSCPAssociation(m_association);
				
		ASC_destroyAssociation(&m_association);
		m_association = 0;
	}
	return true;
}



#include "DcmXTDataSetMain.h"
DcmXTDicomMessage * DcmXTAssociationServerMain::readMessage(char *serviceNameBuff,int buffLen,unsigned short &command,DcmXtError &errorCode,int timeout/*sec*/)
{
	DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::readMessage \n");

	dcm_string serviceName;
	
	errorCode = DcmXtErr_Unknown;

	DcmXTDicomMessage *ret_message=0;

	if(!m_association) {
		DCMLIB_LOG_ERROR("DcmXTAssociationServerMain::readMessage m_association==null \n");
		return 0;
	}


//	DcmXTDataSetMain *new_dataset = new DcmXTDataSetMain;
//	DcmDataset *dataset_ptr =   new_dataset->getDcmDataPtr() ;

	
	 
	DcmXTSCPResponseParam backup_Param ;
	memcpy(&backup_Param,m_ResponseParam,sizeof(DcmXTSCPResponseParam));
 
	
	/*
	*  2012/03/23 K.Ko 処理変更
	*  先にwaitCommandsを発行して、来るMsgがあれば、続いて、processCommands
	*
	*/
 
	T_ASC_PresentationContextID presID_temp = 0;

	/*
	*  step -0 wait commnad
	*/
	OFCondition cond = m_serverHelper->waitCommands(m_association,m_ReceivedMsg,presID_temp,timeout);
	
	if(cond.bad()) {
		if (cond == DUL_PEERREQUESTEDRELEASE)
		{
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage DUL_PEERREQUESTEDRELEASE \n");

	//	if (opt_verbose) printf("Association Release\n");
			//closeAssociation(ASC_releaseAssociation) is called by Client 
			cond = ASC_acknowledgeRelease(m_association);
			errorCode = DcmXtErr_AssociatioinClosed;
			destroy();;
		}
		else if (cond == DUL_PEERABORTEDASSOCIATION)
		{
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage DUL_PEERABORTEDASSOCIATION \n");

	//	if (opt_verbose) printf("Association Aborted\n");
			errorCode = DcmXtErr_AssociatioinAborted;
			destroy();
		}
		else if(cond == DIMSE_NODATAAVAILABLE){
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage DIMSE_NODATAAVAILABLE \n");
			
			errorCode = DcmXtErr_Timeout;
		}
		else 
		{
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage  (aborting association) \n");
		 
		/* some kind of error so abort the association */
			errorCode = DcmXtErr_AssociatioinAborted;
			cond = ASC_abortAssociation(m_association);
			destroy();
		}
		/*
		* Timeoutの場合、終了
		* 
		*/
		return 0;
	}
	//////////

	m_ResponseParam->clear();
	m_ResponseParam->m_presIdCmd = presID_temp;

	DcmXTDicomMessageMain *message_temp  = new DcmXTDicomMessageMain;
	message_temp->open();
	DcmDataset *dataset_ptr =   ((DcmXTDataSetMain *)(message_temp->getDcmXTDataSet()))->getDcmDataPtr();

	/*
	*  step -1 read commnad
	*/
	cond = m_serverHelper->processCommands(m_association,m_ReceivedMsg,dataset_ptr,m_ResponseParam);
	if(cond.bad()) {
		if (cond == DUL_PEERREQUESTEDRELEASE)
		{
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage DUL_PEERREQUESTEDRELEASE \n");

	//	if (opt_verbose) printf("Association Release\n");
			//closeAssociation(ASC_releaseAssociation) is called by Client 
			cond = ASC_acknowledgeRelease(m_association);
			errorCode = DcmXtErr_AssociatioinClosed;
			destroy();;
		}
		else if (cond == DUL_PEERABORTEDASSOCIATION)
		{
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage DUL_PEERABORTEDASSOCIATION \n");

	//	if (opt_verbose) printf("Association Aborted\n");
			errorCode = DcmXtErr_AssociatioinAborted;
			destroy();
		}
		else if(cond == DIMSE_NODATAAVAILABLE){
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage DIMSE_NODATAAVAILABLE \n");
			
			errorCode = DcmXtErr_Timeout;
		}
		else 
		{
			DCMLIB_LOG_DEBUG("DcmXTAssociationServerMain::readMessage  (aborting association) \n");
		 
		/* some kind of error so abort the association */
			errorCode = DcmXtErr_AssociatioinAborted;
			cond = ASC_abortAssociation(m_association);
			destroy();
		}

	//	new_dataset->Delete();
		message_temp->Delete();
		return 0;
	}
	
	command = m_ReceivedMsg->CommandField ;//m_ResponseParam->m_request.MessageID ;

	/*
	*  step -2 process command
	*/
	switch (command)
      {
		case DIMSE_C_ECHO_RQ:
		  // process C-ECHO-Request
			DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::readMessage DIMSE_C_ECHO_RQ \n");
			serviceName = m_ResponseParam->m_echo_request.AffectedSOPClassUID;  
		  break;
		case DIMSE_C_STORE_RQ:
			DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::readMessage DIMSE_C_STORE_RQ \n");
			serviceName = m_ResponseParam->m_store_request.AffectedSOPClassUID;  
		  //
			message_temp->changeMetalInfo();

		  break;
		case DIMSE_C_FIND_RQ:
			DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::readMessage DIMSE_C_FIND_RQ \n");
			serviceName = m_ResponseParam->m_find_request.AffectedSOPClassUID;  
		 break;
		case DIMSE_C_CANCEL_RQ:
			DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::readMessage DIMSE_C_CANCEL_RQ \n");
			serviceName = backup_Param.m_find_request.AffectedSOPClassUID; 
			
		break;
		case DIMSE_C_MOVE_RQ:
			{
				DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::readMessage DIMSE_C_MOVE_RQ \n");
				//
				// we need the move originator message ID (dicomMessageID).  The dicomMessageID is different from the message ID 
				// which is contained in the C-MOVE-RQ message's group 0 elements  
				//
				message_temp->Set_Value(MC_ATT_MESSAGE_ID,m_ReceivedMsg->msg.CMoveRQ.MessageID);
				//
				message_temp->Set_Value( MC_ATT_MOVE_DESTINATION, m_ResponseParam->m_move_request.MoveDestination );
				serviceName = m_ResponseParam->m_move_request.AffectedSOPClassUID;  
			}
		 break;
		default:
          
          break;
      }
	 
	strncpy(serviceNameBuff,serviceName.c_str(),buffLen);

	DCMLIB_LOG_DEBUG(" %s   command: %d \n", serviceNameBuff, command  ); 
	
	message_temp->setupCommand(serviceNameBuff,command);

#if 0
	/*
	DIMSE_C_STORE_RQ の場合
	DataSetからMetaInfoに移す.

#define DCM_FileMetaInformationGroupLength       DcmTagKey(0x0002, 0x0000)
#define DCM_FileMetaInformationVersion           DcmTagKey(0x0002, 0x0001)
#define DCM_MediaStorageSOPClassUID              DcmTagKey(0x0002, 0x0002)
#define DCM_MediaStorageSOPInstanceUID           DcmTagKey(0x0002, 0x0003)
#define DCM_TransferSyntaxUID                    DcmTagKey(0x0002, 0x0010)
#define DCM_ImplementationClassUID               DcmTagKey(0x0002, 0x0012)
#define DCM_ImplementationVersionName            DcmTagKey(0x0002, 0x0013)
#define DCM_SourceApplicationEntityTitle         DcmTagKey(0x0002, 0x0016)
#define DCM_PrivateInformationCreatorUID         DcmTagKey(0x0002, 0x0100)
#define DCM_PrivateInformation                   DcmTagKey(0x0002, 0x0102)
	
	*/

#endif

	bool dbg_write_dicm_flag = false;
	if(dbg_write_dicm_flag)
	{
		char uibuffer[128];
		message_temp->Get_Value(0x00020012,uibuffer,sizeof(uibuffer)-1);

		DcmFileFormat dfileTemp(dataset_ptr);

	// 	 DcmMetaInfo *metainfo = dfileTemp.getMetaInfo();
	// 	 metainfo->Get_ValueString();
		 

		dfileTemp.saveFile("dbg_RecedCmd.dcm");
	}

	
//	DcmXTDicomMessageMain *message_temp  = new DcmXTDicomMessageMain;
//	message_temp->makeFromDataset(new_dataset);

	{
		/* check if this is a valid presentation context */
	 
		T_ASC_PresentationContext pc;
		char *ts = NULL;

		/* figure out if is this a valid presentation context */
		OFCondition cond = ASC_findAcceptedPresentationContext(m_association->params, m_ResponseParam->m_presIdCmd, &pc);
		if(cond.bad())
		{
			DCMLIB_LOG_ERROR("DcmXTAssociationServerMain::readMessage ASC_findAcceptedPresentationContext failed \n");
			;
		}else{

			/* determine the transfer syntax which is specified in the presentation context */
			ts = pc.acceptedTransferSyntax;
	    
			/* create a DcmXfer object on the basis of the transfer syntax which was determined above */
			DcmXfer xfer(ts);

			message_temp->Set_TransferSyntax(DcmXTUtilMain::E2DcmXT_TransferSyntax(xfer.getXfer()));
		}
		/* check if the transfer syntax is supported by dcmtk */
	//	*xferSyntax = xfer.getXfer();


	}
	errorCode = DcmXtErr_Normal;
	ret_message = message_temp;
	return ret_message;
}

 
bool DcmXTAssociationServerMain::sendResponseMessage(RESP_STATUS    ResponseStatus,const DcmXTDicomMessage *message)
{
 
 

	OFCondition cond;


	 
	T_DIMSE_Command command =  m_ReceivedMsg->CommandField ;//m_ResponseParam->m_request.MessageID ;

	const DcmXTDicomMessageMain *dicomMsgmain = (DcmXTDicomMessageMain *)message;

	MC_COMMAND serviceCmd;
	dcm_string  AserviceUID;
//	dicomMsgmain->Get_ServiceName(AserviceUID, serviceCmd);
	{
		 char serviceUID_buff[512];
		 serviceUID_buff[0]=0;
		dicomMsgmain->Get_ServiceName(serviceUID_buff,512,serviceCmd);
		AserviceUID = serviceUID_buff;
	 }

	bool check_flag = serviceCmd == command;

	switch (command)
      {
        case DIMSE_C_ECHO_RQ:
          //
          break;
        case DIMSE_C_STORE_RQ:
		{
			//Status
			m_ResponseParam->m_store_response.DimseStatus = ResponseStatus;
			cond = AssociationHelpServer::My_DIMSE_sendStoreResponse( m_association,m_ResponseParam) ;
	
		 }
          break;
	   case DIMSE_C_FIND_RQ:
		 {
			 //Status
			 m_ResponseParam->m_find_response.DimseStatus = ResponseStatus;

			 //DataSet
			 DcmXTDataSetMain *dataSetMain = (DcmXTDataSetMain*)(message->getDcmXTDataSet());
			 DcmDataset * dataset  = (DcmDataset *)(dataSetMain->getDcmDataPtr());
			 {
				 bool dbg_write_dicm_flag = false;
				if(dbg_write_dicm_flag){
					DcmFileFormat dfileTemp(dataset);
					dfileTemp.saveFile("dbg_RespCmd.dcm",EXS_LittleEndianImplicit);

				
				}
			 }
			 if(ResponseStatus == C_FIND_SUCCESS){
			 //send final response message with status C_FIND_SUCCESS
			 //no dataset
				 m_ResponseParam->m_response_datas = 0;
			 }else{
				m_ResponseParam->m_response_datas = dataset;
			 }
			cond = AssociationHelpServer::My_DIMSE_sendFindResponse( m_association,m_ResponseParam) ;
	
		 }
		 break;
	  case DIMSE_C_CANCEL_RQ:
		 {
			 /*
			 *  after C_CANCEL_RQ ...
			 *  
			 */
			 //Status
			 m_ResponseParam->m_find_response.DimseStatus = ResponseStatus;

			 //DataSet
			 DcmXTDataSetMain *dataSetMain = (DcmXTDataSetMain*)(message->getDcmXTDataSet());
			 DcmDataset * dataset  = (DcmDataset *)(dataSetMain->getDcmDataPtr());
			 m_ResponseParam->m_response_datas = dataset;
			cond = AssociationHelpServer::My_DIMSE_sendFindResponse( m_association,m_ResponseParam) ;
		 }
		 break;
	   case DIMSE_C_MOVE_RQ:
		 {
			 //Status
			 m_ResponseParam->m_move_response.DimseStatus = ResponseStatus;

			 //DataSet
			 DcmXTDataSetMain *dataSetMain = (DcmXTDataSetMain*)(message->getDcmXTDataSet());
			 DcmDataset * dataset  = (DcmDataset *)(dataSetMain->getDcmDataPtr());
			 m_ResponseParam->m_response_datas = dataset;

			switch (m_ResponseParam->m_move_response.DimseStatus) {
			case STATUS_Success:
			case STATUS_Pending:
				{
					/*
					* for update progress etc..
					*/
					unsigned short s_temp;
					if(MC_NORMAL_COMPLETION == dataSetMain->Get_Value(MC_ATT_NUMBER_OF_REMAINING_SUBOPERATIONS,s_temp)){
						m_ResponseParam->m_move_response.NumberOfRemainingSubOperations = s_temp;
					}
					if(MC_NORMAL_COMPLETION == dataSetMain->Get_Value(MC_ATT_NUMBER_OF_COMPLETED_SUBOPERATIONS,s_temp)){
						m_ResponseParam->m_move_response.NumberOfCompletedSubOperations = s_temp;
					}
					if(MC_NORMAL_COMPLETION == dataSetMain->Get_Value(MC_ATT_NUMBER_OF_FAILED_SUBOPERATIONS,s_temp)){
						m_ResponseParam->m_move_response.NumberOfFailedSubOperations = s_temp;
					}
					if(MC_NORMAL_COMPLETION == dataSetMain->Get_Value(MC_ATT_NUMBER_OF_WARNING_SUBOPERATIONS,s_temp)){
						m_ResponseParam->m_move_response.NumberOfWarningSubOperations = s_temp;
					}
				 
				}
				break;
			default:
				 
				break;
			}

			cond = AssociationHelpServer::My_DIMSE_sendMoveResponse( m_association,m_ResponseParam) ;
	
		 }
		 break;

        default:
          
          break;
      }

 	 
	 
	 
	return cond.good();
}

bool DcmXTAssociationServerMain::getInfo(DcmXTAssociationInfo &info)
{
	if(!m_association) return false;
	if(!m_association->params) return false;

	memset(&info,0,sizeof(DcmXTAssociationInfo));
	{
 
		ASInfoStrCpy_RemoteApplicationTitle(info.RemoteApplicationTitle,	m_association->params->DULparams.callingAPTitle) ;
		ASInfoStrCpy_LocalApplicationTitle(info.LocalApplicationTitle,		m_association->params->DULparams.calledAPTitle);
		//
		ASInfoStrCpy_RemoteHostName(info.RemoteHostName,					m_association->params->DULparams.callingPresentationAddress);//m_association->params->DULparams.calledPresentationAddress);
 		ASInfoStrCpy_RemoteIPAddress(info.RemoteIPAddress,					m_association->params->DULparams.callingPresentationAddress);
		ASInfoStrCpy_RemoteImplementationClassUID(info.RemoteImplementationClassUID,	m_association->params->theirImplementationClassUID);
		ASInfoStrCpy_RemoteImplementationVersion(info.RemoteImplementationVersion,		m_association->params->theirImplementationVersionName);
		//
 

	 	info.NumberOfProposedServices		= ASC_countPresentationContexts(m_association->params);
		info.NumberOfAcceptableServices		= ASC_countAcceptedPresentationContexts(m_association->params);
		//
 		info.LocalMaximumPDUSize			= m_association->params->ourMaxPDUReceiveSize;   
 		info.RemoteMaximumPDUSize			= m_association->params->theirMaxPDUReceiveSize;
 		info.MaxOperationsInvoked			= m_association->params->DULparams.maximumOperationsInvoked;   
 		info.MaxOperationsPerformed			= m_association->params->DULparams.maximumOperationsPerformed;   
		 
		 
	}
	
	LPHOSTENT host_info = gethostbyname(info.RemoteHostName);

	dcm_string RemoteIPAddressTemp = info.RemoteIPAddress;
	bool need_get_ipadds = false;
	if(RemoteIPAddressTemp.size()<0){
		need_get_ipadds = true;
	}else{
		if(RemoteIPAddressTemp.find(".") == std::string::npos)
		{
			need_get_ipadds = true;
		}
	}
	//#50 
	//host_info == NULLの場合がある。
//	if(need_get_ipadds){
	if(need_get_ipadds && (host_info!=NULL)){
		char _str_buff[128];
		sprintf(_str_buff,"%d.%d.%d.%d" ,
			(unsigned char)*((host_info->h_addr_list[0])) ,
			(unsigned char)*((host_info->h_addr_list[0]) + 1) ,
			(unsigned char)*((host_info->h_addr_list[0]) + 2) ,
			(unsigned char)*((host_info->h_addr_list[0]) + 3)
			);
		RemoteIPAddressTemp = _str_buff;
	}

	strncpy(info.RemoteIPAddress,RemoteIPAddressTemp.c_str(),128);
	return true;
	
}
bool DcmXTAssociationServerMain::reject( DcmXT_Reject_Reason reason)
{
	T_ASC_RejectParameters rej_param =
    {
      ASC_RESULT_REJECTEDPERMANENT,
      ASC_SOURCE_SERVICEUSER,
      ASC_REASON_SU_NOREASON
    };
	switch(reason){
	case XTDcmLib::DcmXt_PERMANENT_NO_REASON_GIVEN:
		rej_param.result	= ASC_RESULT_REJECTEDPERMANENT;
		rej_param.source	= ASC_SOURCE_SERVICEUSER;
		rej_param.reason	= ASC_REASON_SU_NOREASON;
		break;
	case XTDcmLib::DcmXt_TRANSIENT_NO_REASON_GIVEN:
		rej_param.result	= ASC_RESULT_REJECTEDTRANSIENT;
		rej_param.source	= ASC_SOURCE_SERVICEUSER;
		rej_param.reason	= ASC_REASON_SU_NOREASON;
		break;
	case XTDcmLib::DcmXt_PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED:
		rej_param.result	= ASC_RESULT_REJECTEDPERMANENT;
		rej_param.source	= ASC_SOURCE_SERVICEUSER;
		rej_param.reason	= ASC_REASON_SU_CALLINGAETITLENOTRECOGNIZED;
		break;
	case XTDcmLib::DcmXt_TRANSIENT_TEMPORARY_CONGESTION:
		rej_param.result	= ASC_RESULT_REJECTEDTRANSIENT;
		rej_param.source	= ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
		rej_param.reason	= ASC_REASON_SP_PRES_TEMPORARYCONGESTION;
		break;
	case XTDcmLib::DcmXt_TRANSIENT_LOCAL_LIMIT_EXCEEDED:
		rej_param.result	= ASC_RESULT_REJECTEDTRANSIENT;
		rej_param.source	= ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
		rej_param.reason	= ASC_REASON_SP_PRES_LOCALLIMITEXCEEDED;
		break;
	}

	OFCondition cond = ASC_rejectAssociation(m_association, &rej_param);
    if (cond.bad())
    {
		DCMLIB_LOG_ERROR("Association Reject Failed \n");
		//DimseCondition::dump(cond);
		{
			 OFString strTemp;
			 DimseCondition::dump(strTemp, cond);
			 strTemp += OFString("\n");
			 DCMLIB_LOG_ERROR(strTemp.c_str());
		}
    }

	destroy();;

	return cond.good();
}
bool DcmXTAssociationServerMain::accept()
{
#if 0
	if(m_CurStorageSOPClassUID){
	m_serverHelper->setAcceptStorageSOPClassUIDs(m_CurStorageSOPClassUID,m_CurStorageSOPClassUIDSize);
#else
	bool haveSOPClassUIDList = false;
	if(m_AcceptServiceList){
		if(m_AcceptServiceList->SOPClassUIDsSize>0){
			haveSOPClassUIDList = true;
		}
	}
	if(haveSOPClassUIDList){
		m_serverHelper->setAcceptStorageSOPClassUIDs((const char **)m_AcceptServiceList->SOPClassUIDs,m_AcceptServiceList->SOPClassUIDsSize);
#endif
	}else{
		m_serverHelper->setAcceptStorageSOPClassUIDs(my_dcmAllStorageSOPClassUIDs,my_numberOfAllDcmStorageSOPClassUIDs);
	}

	OFCondition cond = m_serverHelper->acceptAssociation(m_association);

	return cond.good();
}