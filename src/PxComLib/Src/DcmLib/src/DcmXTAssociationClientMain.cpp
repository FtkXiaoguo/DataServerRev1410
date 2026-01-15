// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4189)
#include<WinSock2.h>
#include "DcmXTAssociationClientMain.h"

//////////////////
#include "DcmXTDataSetMain.h"
#include "DcmXTDicomMessageMain.h"
 
#include "AssociationHelp.h"

#include "CheckMemoryLeak.h"
#include "FXDcmLibLogger.h"

using namespace std ;

DcmXTAssociationClientMain::DcmXTAssociationClientMain()
{
	m_association = 0;//2012/02/13 K.Ko
	m_ASC_NetWork = 0;
	m_ASC_Params = 0;
	m_ProposeServiceList = 0;
	m_clientHelper = new AssociationHelpClient ;

	m_clientHelper->setupLogLevel(DcmXTUtilMain::getLogLevel());
}
DcmXTAssociationClientMain::~DcmXTAssociationClientMain()
{
	delete m_clientHelper;
}

bool DcmXTAssociationClientMain::destroy()
{

	if(m_association){
		 
		ASC_destroyAssociation(&m_association);
		m_association = 0;
	}
	return true;
}

bool DcmXTAssociationClientMain::close()
{
 
	if(m_association){
		ASC_releaseAssociation(m_association);
	
	}
	if(m_ASC_NetWork){
		
			ASC_dropNetwork(&m_ASC_NetWork);
			m_ASC_NetWork = 0;
		
	}
	return destroy();
	 
}
bool DcmXTAssociationClientMain::abort()
{
	if(m_association){ //2012/02/13 K.Ko
		ASC_abortAssociation(m_association);
		return destroy();
	}

	return DcmXTAssociationMain::abort();
}
 
void DcmXTAssociationClientMain::Delete()
{
	delete this;
}

const char* my_dcmShortSCUStorageSOPClassUIDs[] = {
    /* This list *must* be limited to 64 SOP classes or less (currently: 63).
     * If we have more than 64 storage transfer syntaxes, tools
     * such as storescu will fail because they attempt to negotiate two
     * presentation contexts for each SOP class, and there is a total limit of
     * 128 contexts for one association.
     * Because of this limitation, all draft and retired storage SOP classes
     * (except for Ultrasound 1993) and the "Standalone" SOP classes
     * are removed from this list. We have also removed support for Stored Print
     * (which will most likely be retired) and the Video objects (which most
     * likely use MPEG2 and, therefore, need a specific association negotiation
     * configuration file anyway.
     * UID_MediaStorageDirectoryStorage should not be present in this list.
     */
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
//  UID_HardcopyColorImageStorage,
//  UID_HardcopyGrayscaleImageStorage,
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
    UID_StereometricRelationshipStorage,
//  UID_StoredPrintStorage,
    UID_TwelveLeadECGWaveformStorage,
    UID_UltrasoundImageStorage,
    UID_UltrasoundMultiframeImageStorage,
    UID_VLEndoscopicImageStorage,
    UID_VLMicroscopicImageStorage,
    UID_VLPhotographicImageStorage,
    UID_VLSlideCoordinatesMicroscopicImageStorage,
//  UID_VideoEndoscopicImageStorage,
//  UID_VideoMicroscopicImageStorage,
//  UID_VideoPhotographicImageStorage,

#endif
    UID_XRayAngiographicImageStorage,
//  UID_XRayFluoroscopyImageStorage,
	UID_XRayRadiofluoroscopicImageStorage,
//   UID_XRayRadiationDoseSR,
	UID_XRayRadiationDoseSRStorage,
	//	
	UID_SecondaryCaptureImageStorage,
	//
	UID_FINDPatientRootQueryRetrieveInformationModel,
	UID_FINDStudyRootQueryRetrieveInformationModel,
	UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,

	//
	UID_MOVEPatientRootQueryRetrieveInformationModel,
	UID_MOVEStudyRootQueryRetrieveInformationModel,
    NULL
};
const int my_numberOfDcmShortSCUStorageSOPClassUIDs = (sizeof(my_dcmShortSCUStorageSOPClassUIDs) / sizeof(const char*)) - 1;

bool DcmXTAssociationClientMain::open(const char *RemoteApplicationTitle,
						int          RemoteHostPortNumber,
						const char *RemoteHostTCPIPName,
						const char *ServiceList,
						DcmXtError &errorCode)
{

	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::open \n");

	errorCode = DcmXtErr_Normal;

try{
	if(!initNetwork()){
		DCMLIB_LOG_ERROR("DcmXTAssociationClientMain::open initNetwork error \n");
		throw(0);//return false;
	}

	/* initialize network, i.e. create an instance of T_ASC_Network*. */
	 m_opt_acse_timeout = 1;

	 /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded()) {
        DCMLIB_LOG_WARN("Warning: no data dictionary loaded, check environment variable: %s \n",DCM_DICT_ENVIRONMENT_VARIABLE);
    }

	OFCondition cond = ASC_initializeNetwork(NET_REQUESTOR, 0, m_opt_acse_timeout, &m_ASC_NetWork);
    if (cond.bad()) {
        //DimseCondition::dump(cond);
		{
			DCMLIB_LOG_ERROR("ASC_initializeNetwork error: \n");
			 OFString strTemp;
			 DimseCondition::dump(strTemp, cond);
			 strTemp += OFString("\n");
			 DCMLIB_LOG_ERROR(strTemp.c_str());
		}
        throw(0);//return false;
    }

	OFCmdUnsignedInt opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
	if(m_ProposeServiceList){
		if(m_ProposeServiceList->maxPDUSize>=4*1024){
			opt_maxReceivePDULength = m_ProposeServiceList->maxPDUSize;
		}
	}

	m_ASC_Params = 0;
	/* initialize asscociation parameters, i.e. create an instance of T_ASC_Parameters*. */
    cond = ASC_createAssociationParameters(&m_ASC_Params, opt_maxReceivePDULength);
    if (cond.bad()) {
        //DimseCondition::dump(cond);
		{
			DCMLIB_LOG_ERROR("ASC_createAssociationParameters error: \n");
			 OFString strTemp;
			 DimseCondition::dump(strTemp, cond);
			 strTemp += OFString("\n");
			 DCMLIB_LOG_ERROR(strTemp.c_str());
		}
        throw(1);//return false;
    }

	{//#10 2012/03/22 K.Ko
		char uidBuffer[128];
		uidBuffer[0] = 0;
		if(IDcmLibApi::Get_String_Config_Value(IMPLEMENTATION_CLASS_UID, sizeof(uidBuffer), uidBuffer)){
			if(strlen(uidBuffer)>1){
				strncpy(m_ASC_Params->ourImplementationClassUID,uidBuffer,sizeof(m_ASC_Params->ourImplementationClassUID)-1);
				 strcpy(m_ASC_Params->DULparams.callingImplementationClassUID,m_ASC_Params->ourImplementationClassUID);
			}
			//
		}
		uidBuffer[0] = 0;
		if(IDcmLibApi::Get_String_Config_Value(IMPLEMENTATION_VERSION, sizeof(uidBuffer), uidBuffer)){
			if(strlen(uidBuffer)>1){
				strncpy(m_ASC_Params->ourImplementationVersionName,uidBuffer,sizeof(m_ASC_Params->ourImplementationVersionName)-1);
				 strcpy(m_ASC_Params->DULparams.callingImplementationVersionName,m_ASC_Params->ourImplementationVersionName);
			}
			//
		}

	}


	/* sets this application's title and the called application's title in the params */
    /* structure. The default values to be set here are "STORESCU" and "ANY-SCP". */
    ASC_setAPTitles(	m_ASC_Params, 
						stringTochar(m_LocalAE) /*opt_ourTitle*/, 
						RemoteApplicationTitle,//stringTochar(RemoteApplicationTitle) /*opt_peerTitle*/, 
						NULL);


	 

	OFBool opt_secureConnection = OFFalse; /* default: no secure connection */

    /* Set the transport layer type (type of network connection) in the params */
    /* strucutre. The default is an insecure connection; where OpenSSL is  */
    /* available the user is able to request an encrypted,secure connection. */
    cond = ASC_setTransportLayerType(m_ASC_Params, opt_secureConnection);
    if (cond.bad()) {
        //DimseCondition::dump(cond);
		{
			DCMLIB_LOG_ERROR("ASC_setTransportLayerType error: \n");
			 OFString strTemp;
			 DimseCondition::dump(strTemp, cond);
			 strTemp += OFString("\n");
			 DCMLIB_LOG_ERROR(strTemp.c_str());
		}
        throw(2);//return false;
    }

	DIC_NODENAME localHost;
    DIC_NODENAME peerHost;
	 /* Figure out the presentation addresses and copy the */
    /* corresponding values into the association parameters.*/
    gethostname(localHost, sizeof(localHost) - 1);
    sprintf(peerHost, "%s:%d", RemoteHostTCPIPName, (int)RemoteHostPortNumber);
    ASC_setPresentationAddresses(m_ASC_Params, localHost,  peerHost );


	OFList<OFString> sopClassUIDList;

	//OFString sopClassUID = stringTochar(ServiceList);
//	sopClassUIDList.push_back(sopClassUID);

	


	if(m_ProposeServiceList){
		if(EXS_Unknown != m_ProposeServiceList->networkTransferSyntax){
			m_clientHelper->setNetworkTransferSyntax(m_ProposeServiceList->networkTransferSyntax);
		}
	//	int numberOfSOP = m_ProposeServiceList->SOPClassUIDs.size();
		int numberOfSOP = m_ProposeServiceList->SOPClassUIDsSize;
		for(int i=0;i<numberOfSOP;i++){
	//		sopClassUIDList.push_back(m_ProposeServiceList->SOPClassUIDs[i].c_str());
			sopClassUIDList.push_back(m_ProposeServiceList->SOPClassUIDs[i]);
		}
	}else{
		for(int i=0;i<my_numberOfDcmShortSCUStorageSOPClassUIDs;i++){
			sopClassUIDList.push_back(my_dcmShortSCUStorageSOPClassUIDs[i]);
		}
	}

	/* Set the presentation contexts which will be negotiated */
    /* when the network connection will be established */
#if 1
     cond = m_clientHelper->addStoragePresentationContexts(m_ASC_Params, sopClassUIDList);

	if (cond.bad()) {
        //DimseCondition::dump(cond);
		{
			DCMLIB_LOG_ERROR("addStoragePresentationContexts error: \n");
			 OFString strTemp;
			 DimseCondition::dump(strTemp, cond);
			 strTemp += OFString("\n");
			 DCMLIB_LOG_ERROR(strTemp.c_str());
		}
        throw(2);//return false;
    }
#endif
 
	  /* dump presentation contexts if required */
//    if (opt_showPresentationContexts || opt_debug) {
	if(m_clientHelper->opt_verbose){
        DCMLIB_LOG_DEBUG("Request Parameters: \n");
		std::stringstream sting_temp;
        ASC_dumpParameters(m_ASC_Params, sting_temp);
		DCMLIB_LOG_DEBUG((sting_temp.str()+ ("\n")).c_str());

    }

	/* create association, i.e. try to establish a network connection to another */
    /* DICOM application. This call creates an instance of T_ASC_Association*. */
    
	if (m_clientHelper->opt_verbose){
		DCMLIB_LOG_DEBUG(">>>>>>> Requesting Association \n");
	}

	int reTryNN = 10;
	m_association = 0;

	//2012/06/01 K.KO add openAssociation retry number
	int try_open_number = 0;
	if(IDcmLibApi::Get_Int_Config_Value(RETRY_NUMBER_OPEN_ASSOC,&try_open_number)){
		if(try_open_number>0){
			reTryNN = try_open_number;
		}
	}
	int open_assoc_timeout  =0;
	if(IDcmLibApi::Get_Int_Config_Value(OPEN_ASSOC_TIMEOUT,&open_assoc_timeout)){
		 
	}
	for(int run_i=0;run_i<reTryNN;run_i++){
		DCMLIB_LOG_TRACE("...");
		errorCode = DcmXtErr_Normal;
		void *net_key = m_ASC_NetWork->network;

		int save_pre_timeout = -1 ;
		if(open_assoc_timeout>0){
			save_pre_timeout = dcmConnectionTimeout.get();
			dcmConnectionTimeout.set(open_assoc_timeout);
		}
		cond = ASC_requestAssociation(m_ASC_NetWork, m_ASC_Params, &m_association);

		if(open_assoc_timeout>0){
			dcmConnectionTimeout.set(save_pre_timeout);
		}

		DCMLIB_LOG_TRACE("|");
		if (cond.bad()) {
			
			if (cond == DUL_ASSOCIATIONREJECTED) {
				T_ASC_RejectParameters rej;

				ASC_getRejectParameters(m_ASC_Params, &rej);
		  //      errmsg("Association Rejected:");
				ASC_printRejectParameters(stderr, &rej);

				errorCode = DcmXtErr_AssociatioinRejected;
				throw(3);//return 1;
			} else {
				 
				errorCode = DcmXtErr_Timeout;
				 
		//        errmsg("Association Request Failed:");
				DimseCondition::dump(cond);
				if(run_i>=(reTryNN-1)){
					throw(3);//return 1;
				}else{
					if(m_association){
						//Donot use ASC_destroyAssociation(&m_association);!
						// just delete m_association;
						
						free(m_association);
						m_association = 0;
						continue;
					}
				}
			}
		}else{
			if (m_clientHelper->opt_verbose){
				DCMLIB_LOG_DEBUG(">>>>>>> Requesting Association OK  \n");
			}
			break;
		}
	}
	

	//
	/* dump the connection parameters if in debug mode*/
#if 0
    if (m_clientHelper->opt_debug)
    {
        ostream& out = ofConsole.lockCout();     
        ASC_dumpConnectionParameters(m_association, out);
        ofConsole.unlockCout();
    }
#endif

    /* dump the presentation contexts which have been accepted/refused */
    if (m_clientHelper->opt_showPresentationContexts || m_clientHelper->opt_debug) {
        DCMLIB_LOG_DEBUG("Association Parameters Negotiated: \n");
		std::stringstream sting_temp;
        ASC_dumpParameters(m_ASC_Params, sting_temp);
		DCMLIB_LOG_DEBUG((sting_temp.str()+ ("\n")).c_str());
    }

    /* count the presentation contexts which have been accepted by the SCP */
    /* If there are none, finish the execution */
    if (ASC_countAcceptedPresentationContexts(m_ASC_Params) == 0) {
		DCMLIB_LOG_ERROR("ASC_countAcceptedPresentationContexts(m_ASC_Params) == 0 \n");
  //      errmsg("No Acceptable Presentation Contexts");
        throw(4);//return 1;
    }

    /* dump general information concerning the establishment of the network connection if required */
    if (m_clientHelper->opt_verbose) {
        DCMLIB_LOG_DEBUG("Association Accepted (Max Send PDV: %d) \n", m_association->sendPDVLength );
    }

    /* do the real work, i.e. for all files which were specified in the */
    /* command line, transmit the encapsulated DICOM objects to the SCP. */
    cond = EC_Normal;

#if 0
     OFListIterator(OFString) iter = fileNameList.begin();
     OFListIterator(OFString) enditer = fileNameList.end();

    while ((iter != enditer) && (cond == EC_Normal)) // compare with EC_Normal since DUL_PEERREQUESTEDRELEASE is also good()
    {
        cond = cstore(assoc, *iter);
        ++iter;
    }

#endif
	}catch(int error_code)
	{
		if(errorCode == DcmXtErr_Normal){
			errorCode = DcmXtErr_Unknown;
		}else{
			;;//errorCode is setted already
		}

		if (m_clientHelper->opt_verbose){
			DCMLIB_LOG_ERROR(">>>>>**  error_code  %d \n",error_code);
		}

		bool ret_flag = true;
		switch(error_code){
			case 0:
			ret_flag = false;
			break;

			case 1: 
			case 2:
				if(m_ASC_NetWork){
					ASC_dropNetwork(&m_ASC_NetWork);
					m_ASC_NetWork = 0;
				}
			if(m_ASC_Params){
				ASC_destroyAssociationParameters(&m_ASC_Params);
				m_ASC_Params = 0;
			}
			ret_flag = false;
			break;

			case 3:
			case 4:
				//
				//m_ASC_Paramsはm_associationに設定され、destroyしない。
				//
				if(m_ASC_NetWork){
					ASC_dropNetwork(&m_ASC_NetWork);
					m_ASC_NetWork = 0;
				}
				if(m_association){
					ASC_destroyAssociation(&m_association);
					m_association = 0;
				}
			ret_flag = false;
			break;

		}

		DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::open return erro %d \n",ret_flag);

		return ret_flag;
		
	}
	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::open return normal  \n");
	return true;
}
 

	//

bool DcmXTAssociationClientMain::getInfo(DcmXTAssociationInfo &info)
{
	if(!m_association) return false;
	if(!m_association->params) return false;

	memset(&info,0,sizeof(DcmXTAssociationInfo));
	{
#if 0
		ASInfoStrCpy_RemoteApplicationTitle(info.RemoteApplicationTitle,	m_association->params->DULparams.callingAPTitle) ;
		ASInfoStrCpy_LocalApplicationTitle(info.LocalApplicationTitle,		m_association->params->DULparams.calledAPTitle);
#else	// #81 2014/08/22 K.Ko
		ASInfoStrCpy_RemoteApplicationTitle(info.RemoteApplicationTitle,	m_association->params->DULparams.calledAPTitle); 
		ASInfoStrCpy_LocalApplicationTitle(info.LocalApplicationTitle,		m_association->params->DULparams.callingAPTitle);
#endif
		//
		ASInfoStrCpy_RemoteHostName(info.RemoteHostName,					m_association->params->DULparams.calledPresentationAddress);
 		ASInfoStrCpy_RemoteIPAddress(info.RemoteIPAddress,					m_association->params->DULparams.calledPresentationAddress);
		ASInfoStrCpy_RemoteImplementationClassUID(info.RemoteImplementationClassUID,	m_association->params->theirImplementationClassUID);
		ASInfoStrCpy_RemoteImplementationVersion(info.RemoteImplementationVersion,		m_association->params->theirImplementationVersionName);
		//
 
		info.LocalMaximumPDUSize			= m_association->params->ourMaxPDUReceiveSize;   
 		info.RemoteMaximumPDUSize			= m_association->params->theirMaxPDUReceiveSize;
	} 
	
	return true;
	
}
 
bool DcmXTAssociationClientMain::getFirstAcceptableService(DcmXTAssociationServiceInfo &serviceInfo,bool &endList)
{
	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::getFirstAcceptableService \n");

	if(!m_association) {
		DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::getFirstAcceptableService m_association==0 \n");
		return false;
	}
	if(!m_association->params) {
		DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::getFirstAcceptableService m_association->params==0 \n");
		return false;
	}

	endList = false;
	{
		m_AcceptableServiceCounter = ASC_countPresentationContexts(m_association->params);
		m_AcceptableServiceIterator = 0;
		if(m_AcceptableServiceCounter<1) {
			DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::getFirstAcceptableService m_AcceptableServiceCounter<1 \n");
			return false;
		}
		
		T_ASC_PresentationContext pc;
		bool have_data = false;
		while(m_AcceptableServiceIterator<m_AcceptableServiceCounter){
			ASC_getPresentationContext(m_association->params, m_AcceptableServiceIterator++, &pc);
			if(ASC_P_ACCEPTANCE == pc.resultReason){
				have_data = true;
				break;
			}
		}
		if(!have_data) {
			endList = true;
			return false;
		}
		serviceInfo.PresentationContextID	= pc.presentationContextID;

		ASServiceInfo_ServiceName(serviceInfo.ServiceName		,dcmFindNameOfUID(pc.abstractSyntax));
		ASServiceInfo_SyntaxTypeUID(serviceInfo.SyntaxTypeUID	,pc.acceptedTransferSyntax);


		const char* l_as = dcmFindNameOfUID(pc.abstractSyntax);
		const char* ts = dcmFindNameOfUID(pc.acceptedTransferSyntax);
		DcmXfer disp_xfer = DcmXfer(pc.acceptedTransferSyntax);

		{
			const char *xfername_temp = disp_xfer.getXferName();
			if(!xfername_temp) xfername_temp = " ";
			const char *xferID_temp = disp_xfer.getXferID();
			if(!xferID_temp) xferID_temp = " ";
			DCMLIB_LOG_TRACE(" %d, %s, %s  \n",disp_xfer.getXfer(),xfername_temp,xferID_temp);
		}

	}

	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::getFirstAcceptableService return normal \n");
	return true;

}
bool DcmXTAssociationClientMain::getNextAcceptableService(DcmXTAssociationServiceInfo &serviceInfo,bool &endList)
{
	 
	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::getNextAcceptableService \n");

	if(!m_association) {
		DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::getNextAcceptableService m_association==0 \n");
		return false;
	}
	if(!m_association->params) {
		DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::getNextAcceptableService m_association->params==0 \n");
		return false;
	}

	endList = false;
	{
		 
	 
		T_ASC_PresentationContext pc;
		bool have_data = false;
		while(m_AcceptableServiceIterator<m_AcceptableServiceCounter){
			ASC_getPresentationContext(m_association->params, m_AcceptableServiceIterator++, &pc);
			if(ASC_P_ACCEPTANCE == pc.resultReason){
				have_data = true;
				break;
			}
		}
		if(!have_data) {
			endList = true;
			return false;
		}
		serviceInfo.PresentationContextID	= pc.presentationContextID;

		ASServiceInfo_ServiceName(serviceInfo.ServiceName		,dcmFindNameOfUID(pc.abstractSyntax));
		ASServiceInfo_SyntaxTypeUID(serviceInfo.SyntaxTypeUID	,pc.acceptedTransferSyntax);



		const char* l_as = dcmFindNameOfUID(pc.abstractSyntax);
		const char* ts = dcmFindNameOfUID(pc.acceptedTransferSyntax);
		DcmXfer disp_xfer = DcmXfer(pc.acceptedTransferSyntax);

		{
			const char *xfername_temp = disp_xfer.getXferName();
			if(!xfername_temp) xfername_temp = " ";
			const char *xferID_temp = disp_xfer.getXferID();
			if(!xferID_temp) xferID_temp = " ";
			DCMLIB_LOG_TRACE(" %d, %s, %s  \n",disp_xfer.getXfer(),xfername_temp,xferID_temp);
		}

	}

	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::getNextAcceptableService return normal \n");
	return true;

}

 bool DcmXTAssociationClientMain::sendRequestMessage( DcmXTDicomMessage &message)
{
	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage \n");

	if(!m_association) {
		DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::sendRequestMessage m_association==0 \n");
		return false;
	}

//	m_ResponseParam->clear();

//	DIMSE_debug(0);


	OFCondition cond;

	DIC_UI sopClass;
    DIC_UI sopInstance;
 
	m_ResponseParam->m_statusDetail = NULL;

// 2012/03/23  unused
//	int lastStatusCode = STATUS_Success;
//	OFBool unsuccessfulStoreEncountered = OFTrue; // assumption

	/*
	/*
	* HowToUseMessageID
	* TBD:
	*  message.getID(m_sentOrgMsgID); 
	*  を使うべき
	*/
	DIC_US msgId = m_association->nextMsgID++;
  
	// DcmXTDataSet *dataset = message.getDcmXTDataSet();

	 DcmXTDataSetMain *datasetInstance = (DcmXTDataSetMain *)message.getDcmXTDataSet();
	 DcmDataset * dataset  = (DcmDataset *)(datasetInstance->getDcmDataPtr());


	 dcm_string  AserviceUID;
	 MC_COMMAND serviceCmd;
	 //message.Get_ServiceName( AserviceUID,  serviceCmd)  ;
	 {
		 char serviceUID_buff[512];
		 serviceUID_buff[0]=0;
		message.Get_ServiceName(serviceUID_buff,512,serviceCmd);
		AserviceUID = serviceUID_buff;
	 }
	 
	 //////////////////

 
 	// dataset->print(COUT);

	 /* figure out which SOP class and SOP instance is encapsulated in the file */
    if (!DU_findSOPClassAndInstanceInDataSet(dataset,
        sopClass,sizeof(sopClass), sopInstance, sizeof(sopInstance),m_clientHelper->opt_correctUIDPadding)) {
      

		switch (serviceCmd)
		  {
			case DIMSE_C_ECHO_RQ:
			  DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage DIMSE_C_ECHO_RQ  \n");
			  strcpy(sopClass,AserviceUID.c_str());//#32 2012/08/06 for Echo-SCU
			  break;
			case DIMSE_C_STORE_RQ:
			  DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage DIMSE_C_STORE_RQ  \n");
			  break;
			case DIMSE_C_FIND_RQ:
			  DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage DIMSE_C_FIND_RQ  \n");
		//	  strcpy(sopClass,UID_FINDPatientRootQueryRetrieveInformationModel);  
				strcpy(sopClass,AserviceUID.c_str());  
			 break;
			case DIMSE_C_MOVE_RQ:
				{
					DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage DIMSE_C_MOVE_RQ  \n");
			//	strcpy(sopClass,	 UID_MOVEStudyRootQueryRetrieveInformationModel);
					strcpy(sopClass,	AserviceUID.c_str());
				}
			 break;
			case DIMSE_C_CANCEL_RQ:
				{
					DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage DIMSE_C_CANCEL_RQ  \n");
					cond = DIMSE_sendCancelRequest(m_association,
		 						m_ResponseParam->m_presIdCmd, m_ResponseParam->m_find_request.MessageID);
					  return cond.good();
				}
			 break;
			default:
			  DCMLIB_LOG_DEBUG("DcmXTAssociationClientMain::sendRequestMessage unknown serviceCmd  \n");
			  return false;
			  break;
		  }
         
	}

	 
	/* figure out which of the accepted presentation contexts should be used */
    DcmXfer filexfer(dataset->getOriginalXfer());

	if(filexfer.getXfer() != EXS_Unknown)
	{
		m_ResponseParam->m_presIdCmd = ASC_findAcceptedPresentationContextID(m_association, sopClass, filexfer.getXferID());
	}else{
		m_ResponseParam->m_presIdCmd = ASC_findAcceptedPresentationContextID(m_association, sopClass);
	}
    if (m_ResponseParam->m_presIdCmd == 0) {
        const char *modalityName = dcmSOPClassUIDToModality(sopClass);
        if (!modalityName) modalityName = dcmFindNameOfUID(sopClass);
        if (!modalityName) modalityName = "unknown SOP class";
        DCMLIB_LOG_ERROR("No presentation context for: ( %s), %s  \n",modalityName,sopClass);
        return false;
    }

	 /* if required, dump general information concerning transfer syntaxes */
    if (m_clientHelper->opt_verbose) {
        DcmXfer fileTransfer(dataset->getOriginalXfer());
        T_ASC_PresentationContext pc;
        ASC_findAcceptedPresentationContext(m_association->params, m_ResponseParam->m_presIdCmd, &pc);
        DcmXfer netTransfer(pc.acceptedTransferSyntax);

		const char *fileTf_temp = dcmFindNameOfUID(fileTransfer.getXferID());
		if(!fileTf_temp) fileTf_temp = "";
		const char *netTf_temp = dcmFindNameOfUID(netTransfer.getXferID());
		if(!netTf_temp) netTf_temp = "";
        DCMLIB_LOG_TRACE("Transfer: %s -> %s \n",fileTf_temp ,netTf_temp);
    }

	/* prepare the transmission of data */
  
    

    /* if required, dump some more general information */
    if (m_clientHelper->opt_verbose) {
		const char *sopClass_temp = dcmSOPClassUIDToModality(sopClass);
		if(!sopClass_temp) sopClass_temp = "***";
        DCMLIB_LOG_DEBUG("send RQ: MsgID %d ( %s ) \n" ,msgId,sopClass_temp);
    }

	
	AssociationHelpClient::setProgressCallbackVerbose(m_clientHelper->opt_verbose);

	
	/* finally conduct transmission of data */
    

	switch (serviceCmd)
      {
		case DIMSE_C_ECHO_RQ:
			{ //#32 2012/08/06 for Echo-SCU
				memset((char*)&(m_ResponseParam->m_echo_request), 0,sizeof(m_ResponseParam->m_echo_request));
				memset((char*)&(m_ResponseParam->m_echo_response),0, sizeof(m_ResponseParam->m_echo_response));
			 

				m_ResponseParam->m_echo_request.MessageID = msgId;
				cond = AssociationHelpClient::My_DIMSE_echoUser(m_association, m_ResponseParam);

			}
		  break;
		case DIMSE_C_STORE_RQ:
			{
				  
				memset((char*)&(m_ResponseParam->m_store_request),0, sizeof(m_ResponseParam->m_store_request));

				strcpy(m_ResponseParam->m_store_request.AffectedSOPClassUID, sopClass);
				strcpy(m_ResponseParam->m_store_request.AffectedSOPInstanceUID, sopInstance);
				m_ResponseParam->m_store_request.MessageID = msgId;
				m_ResponseParam->m_store_request.DataSetType = DIMSE_DATASET_PRESENT;
				m_ResponseParam->m_store_request.Priority = DIMSE_PRIORITY_LOW;

			  cond = AssociationHelpClient::My_DIMSE_storeUser(m_association, 
				m_ResponseParam,
		//		NULL, 
				dataset, 
				AssociationHelpClient::StoreProgressCallback,NULL,
				NULL, 64);
			  //
#if 0 // 2012/03/23  unused
				/*
				 * If store command completed normally, with a status
				 * of success or some warning then the image was accepted.
				 */
				if (cond == EC_Normal && (m_ResponseParam->m_store_response.DimseStatus == STATUS_Success || 
					DICOM_WARNING_STATUS(m_ResponseParam->m_store_response.DimseStatus))) {
					unsuccessfulStoreEncountered = OFFalse;
				}

				/* remember the response's status for later transmissions of data */
				lastStatusCode = m_ResponseParam->m_store_response.DimseStatus;
#endif

			}
		  break;
		case DIMSE_C_FIND_RQ:
			{
				memset((char*)&(m_ResponseParam->m_find_request), 0,sizeof(m_ResponseParam->m_find_request));

				strcpy(m_ResponseParam->m_find_request.AffectedSOPClassUID, sopClass);
				
				m_ResponseParam->m_find_request.MessageID = msgId;
				m_ResponseParam->m_find_request.DataSetType = DIMSE_DATASET_PRESENT;
				m_ResponseParam->m_find_request.Priority = DIMSE_PRIORITY_LOW;

			  cond = AssociationHelpClient::My_DIMSE_findUser(m_association, 
				m_ResponseParam,
				dataset, 
	//			AssociationHelpClient::FindProgressCallback,NULL,
				NULL);
			   //
#if 0 // 2012/03/23  unused
				/*
				 * If store command completed normally, with a status
				 * of success or some warning then the image was accepted.
				 */
				if (cond == EC_Normal && (m_ResponseParam->m_find_response.DimseStatus == STATUS_Success || 
					DICOM_WARNING_STATUS(m_ResponseParam->m_find_response.DimseStatus))) {
					unsuccessfulStoreEncountered = OFFalse;
				}

				/* remember the response's status for later transmissions of data */
				lastStatusCode = m_ResponseParam->m_find_response.DimseStatus;
#endif

			}
		 break;
		case DIMSE_C_MOVE_RQ:
			{
				memset((char*)&(m_ResponseParam->m_move_request), 0,sizeof(m_ResponseParam->m_move_request));

				strcpy(m_ResponseParam->m_move_request.AffectedSOPClassUID, sopClass);
				
				m_ResponseParam->m_move_request.MessageID = msgId;
				m_ResponseParam->m_move_request.DataSetType = DIMSE_DATASET_PRESENT;
				m_ResponseParam->m_move_request.Priority = DIMSE_PRIORITY_LOW;

				//set the destination
				{
					// change string to char * 2012/02/16 K.Ko
#if 0
			 		dllString str_temp;
					message.Get_Value(MC_ATT_MOVE_DESTINATION,str_temp);
					
					if(str_temp.size()<1){
						DCMLIB_LOG_ERROR(" no destination AE \n");
						return false;
					}
					strcpy(m_ResponseParam->m_move_request.MoveDestination,str_temp.c_str());
#else
					char _str_buff[128];
					_str_buff[0]=0;
					message.Get_Value(MC_ATT_MOVE_DESTINATION,_str_buff,128);
					if(_str_buff[0]==0){
						DCMLIB_LOG_ERROR(" no destination AE \n");
						return false;
					}
					strcpy(m_ResponseParam->m_move_request.MoveDestination,_str_buff);
#endif
				}
			  cond = AssociationHelpClient::My_DIMSE_moveUser(m_association, 
				m_ResponseParam,
				dataset);
			   //
#if 0 // 2012/03/23  unused
				/*
				 * If store command completed normally, with a status
				 * of success or some warning then the image was accepted.
				 */
				if (cond == EC_Normal && (m_ResponseParam->m_find_response.DimseStatus == STATUS_Success || 
					DICOM_WARNING_STATUS(m_ResponseParam->m_find_response.DimseStatus))) {
					unsuccessfulStoreEncountered = OFFalse;
				}

				/* remember the response's status for later transmissions of data */
				lastStatusCode = m_ResponseParam->m_find_response.DimseStatus;
#endif 
			}
		 break;
		 
		case DIMSE_C_CANCEL_RQ:
			{
		 ;;
			}
		 break;
		default:
          return false;
          break;
      }

	message.getID(m_sentOrgMsgID);

	DCMLIB_LOG_TRACE("DcmXTAssociationClientMain::sendRequestMessage return %d \n",cond.good());
	return cond.good();
}


DcmXTDicomMessage * DcmXTAssociationClientMain::readMessage(char *serviceNameBuff,int buffLen,unsigned short &command,DcmXtError &errorCode,int timeout/*sec*/)
{

	errorCode = DcmXtErr_Unknown;
	DcmXTDicomMessage *ret_message=0;

	if(!m_association) return 0;

	dcm_string serviceName;

//	DcmXTDataSetMain *new_dataset = new DcmXTDataSetMain;
//	DcmDataset *dataset_ptr =   new_dataset->getDcmDataPtr() ;

	DcmXTDicomMessageMain *message_temp  = new DcmXTDicomMessageMain;

	message_temp->open();
	DcmDataset *dataset_ptr =   ((DcmXTDataSetMain *)(message_temp->getDcmXTDataSet()))->getDcmDataPtr();

  

	/*
	m_clientHelper->opt_blockMode, m_clientHelper->opt_dimse_timeout,
	*/

	OFCondition cond = EC_IllegalParameter;

	command = m_ResponseParam->m_reqMsg.CommandField;

	message_temp->setResponseMsg(m_sentOrgMsgID,0);//m_ResponseParam->m_response.opts);
	 switch (command)
      {
		case DIMSE_C_ECHO_RQ:
			{
				//#32 2012/08/06 for Echo-SCU
				int echo_rsp_command;
				cond = AssociationHelpClient::My_DIMSE_readEchoResponse(m_association,m_ReceivedMsg,&dataset_ptr,
					  m_ResponseParam,
					  m_clientHelper->opt_blockMode, 
					  timeout,
					  echo_rsp_command);
				if (cond == EC_Normal)
				{
					/*
					* HowToUseMessageID
					* TBD  use m_ResponseParam->m_echo_response.MessageIDBeingRespondedTo
					*/
					message_temp->setResponseMsg(
						m_sentOrgMsgID,
						m_ResponseParam->m_echo_response.DimseStatus);//m_ResponseParam->m_response.opts);
					
					serviceName = m_ResponseParam->m_echo_request.AffectedSOPClassUID;
					 if (m_clientHelper->opt_verbose) {
						 DIMSE_printCEchoRSP(stdout, &(m_ResponseParam->m_echo_response));
					}

					 command = echo_rsp_command;
				}
 
			}
		  break;
		case DIMSE_C_STORE_RQ:
			{
				 cond = AssociationHelpClient::My_DIMSE_readCStoreResponse(m_association,
															m_ReceivedMsg,&dataset_ptr,
															m_ResponseParam,
															m_clientHelper->opt_blockMode, 
															timeout,//m_clientHelper->opt_dimse_timeout,
															NULL);
					  
				if (cond == EC_Normal)
				{
					serviceName = m_ResponseParam->m_store_request.AffectedSOPClassUID;
					 if (m_clientHelper->opt_verbose) {
						 DIMSE_printCStoreRSP(stdout, &(m_ResponseParam->m_store_response));
					}
					command = DIMSE_C_STORE_RSP;//#81 2014/08/22 K.Ko
				}
			}
		  break;
		case DIMSE_C_FIND_RQ:
			{
				 cond = AssociationHelpClient::My_DIMSE_readCFindResponse(m_association,
															m_ReceivedMsg,&dataset_ptr,
															m_ResponseParam,
															m_clientHelper->opt_blockMode, 
															timeout,//m_clientHelper->opt_dimse_timeout,
															NULL);
				if (cond == EC_Normal)
				{

					/*
					* HowToUseMessageID
					* TBD  use m_ResponseParam->m_echo_response.MessageIDBeingRespondedTo
					*/
					message_temp->setResponseMsg(m_sentOrgMsgID,m_ResponseParam->m_find_response.DimseStatus);//m_ResponseParam->m_response.opts);
					serviceName = m_ResponseParam->m_find_request.AffectedSOPClassUID;
					 if (m_clientHelper->opt_verbose) {
						 DIMSE_printCFindRSP(stdout, &(m_ResponseParam->m_find_response));
					}
					command = DIMSE_C_FIND_RSP;//#81 2014/08/22 K.Ko
				}
			}
		 break;
		case DIMSE_C_MOVE_RQ:
			{
				cond = AssociationHelpClient::My_DIMSE_readCMoveResponse(m_association,
															m_ReceivedMsg,
															m_ResponseParam,
			//												NULL/*callback*/, NULL,
															m_clientHelper->opt_blockMode, 
															timeout,//m_clientHelper->opt_dimse_timeout,
															m_ASC_NetWork,
			//												NULL/*subOpCallback*/,NULL/*subOpCallbackData*/,
															&dataset_ptr,
															true);
				if (cond == EC_Normal)
				{

					/*
					* HowToUseMessageID
					* TBD  use m_ResponseParam->m_echo_response.MessageIDBeingRespondedTo
					*/
					message_temp->setResponseMsg(m_sentOrgMsgID,m_ResponseParam->m_move_response.DimseStatus);//m_ResponseParam->m_response.opts);
					serviceName = m_ResponseParam->m_move_response.AffectedSOPClassUID;
					 if (m_clientHelper->opt_verbose) {
						 DIMSE_printCMoveRSP(stdout, &(m_ResponseParam->m_move_response));
					}
					command = DIMSE_C_MOVE_RSP;//#81 2014/08/22 K.Ko
				}
				 
			}
		 break;
		default:
          return false;
          break;
      }



	
	if(cond.bad()) {
//		new_dataset->Delete();
		message_temp->Delete();
		return 0;
	}
	///
	if (cond != EC_Normal)
    {
	 	printf("Store : " );
        DimseCondition::dump(cond);
    }

    /* dump status detail information if there is some */
    if (m_ResponseParam->m_statusDetail  != NULL) {
   //     printf("  Status Detail:\n");
  //      m_ResponseParam->m_statusDetail->print(COUT);
        delete m_ResponseParam->m_statusDetail;
    }
///
	
	static bool dbg_write_dicm_flag = false;
	if(dbg_write_dicm_flag){
		DcmFileFormat dfileTemp(dataset_ptr);
		dfileTemp.saveFile("dbg_RecedCmd.dcm");
	}

	 
	
///	message_temp->makeFromDataset(new_dataset);


	ret_message = message_temp;

	strncpy(serviceNameBuff,serviceName.c_str(),buffLen);

	 
	errorCode = DcmXtErr_Normal;
	return ret_message;
}


 