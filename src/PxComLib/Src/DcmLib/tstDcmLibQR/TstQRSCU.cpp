// TstStoreSCU.cpp: CTstQRSCU クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <ctime>

#include "TstQRSCU.h"

 
#ifdef USE_NEW_LIB

#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "IDcmLibDefUID.h"

using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
	ServiceInfo    m_servInfo;
	AssocInfo      m_asscInfo;
///

static char _str_buff[1024];

static char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr;
    
    switch (A_syntax)
    {
    case IMPLICIT_LITTLE_ENDIAN: ptr = "Implicit VR Little Endian"; break;
    case EXPLICIT_LITTLE_ENDIAN: ptr = "Explicit VR Little Endian"; break;
    case EXPLICIT_BIG_ENDIAN:    ptr = "Explicit VR Big Endian"; break;
    case IMPLICIT_BIG_ENDIAN:    ptr = "Implicit VR Big Endian"; break;
    case DEFLATED_EXPLICIT_LITTLE_ENDIAN: ptr = "Deflated Explicit VR Little Endian"; break;
    case RLE:                    ptr = "RLE"; break;
    case JPEG_BASELINE:          ptr = "JPEG Baseline (Process 1)"; break;
    case JPEG_EXTENDED_2_4:      ptr = "JPEG Extended (Process 2 & 4)"; break;
    case JPEG_EXTENDED_3_5:      ptr = "JPEG Extended (Process 3 & 5)"; break;
    case JPEG_SPEC_NON_HIER_6_8: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)"; break;
    case JPEG_SPEC_NON_HIER_7_9: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)"; break;
    case JPEG_FULL_PROG_NON_HIER_10_12: ptr = "JPEG Full Progression, Non-Hierarchical (Process 10 & 12)"; break;
    case JPEG_FULL_PROG_NON_HIER_11_13: ptr = "JPEG Full Progression, Non-Hierarchical (Process 11 & 13)"; break;
    case JPEG_LOSSLESS_NON_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical (Process 14)"; break;
    case JPEG_LOSSLESS_NON_HIER_15: ptr = "JPEG Lossless, Non-Hierarchical (Process 15)"; break;
    case JPEG_EXTENDED_HIER_16_18: ptr = "JPEG Extended, Hierarchical (Process 16 & 18)"; break;
    case JPEG_EXTENDED_HIER_17_19: ptr = "JPEG Extended, Hierarchical (Process 17 & 19)"; break;
    case JPEG_SPEC_HIER_20_22: ptr = "JPEG Spectral Selection Hierarchical (Process 20 & 22)"; break;
    case JPEG_SPEC_HIER_21_23: ptr = "JPEG Spectral Selection Hierarchical (Process 21 & 23)"; break;
    case JPEG_FULL_PROG_HIER_24_26: ptr = "JPEG Full Progression, Hierarchical (Process 24 & 26)"; break;
    case JPEG_FULL_PROG_HIER_25_27: ptr = "JPEG Full Progression, Hierarchical (Process 25 & 27)"; break;
    case JPEG_LOSSLESS_HIER_28: ptr = "JPEG Lossless, Hierarchical (Process 28)"; break;
    case JPEG_LOSSLESS_HIER_29: ptr = "JPEG Lossless, Hierarchical (Process 29)"; break;
    case JPEG_LOSSLESS_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical, First-Order Prediction"; break;
	case JPEG_2000_LOSSLESS_ONLY: ptr = "JPEG 2000 Image Compression (Lossless Only)"; break;
	case JPEG_2000:				ptr = "JPEG 2000 Image Compression"; break;
    case PRIVATE_SYNTAX_1:      ptr = "Private Syntax 1"; break;
    case PRIVATE_SYNTAX_2:      ptr = "Private Syntax 2"; break;
    }
    return ptr;
}
///
CTstQRSCU::CTstQRSCU()
{

	m_sendLoopNN = 0;
	m_LoopInterval = 2000;//mSec



	m_PatientName = "*";
	m_PatientID	= "";
	m_StudyID	= "";
	m_SeriesID  = "";
	m_StudyDate = "";

	m_applicationID = 0;

	m_MyAE = "TestQR";

#if 1
 
	m_DicomSvrAE = "MONE_AE";
	m_DicomSvrHost = "172.17.3.72";
	m_DicomSvrPort = 105;
#else
	m_DicomSvrAE = "FCOLDMOON_AE ";
	m_DicomSvrHost = "172.17.23.21";
	m_DicomSvrPort = 105;
#endif

//
	m_CMoveDestAE = "OctNote-AQN";
 
//
 
//
	m_Modality = 0;
	 
//

//
	m_runCount = 0;
 
	m_RequestCount = 0;
 



	m_associationID = 0;

	m_ReopenAssociation = 1;
///
	m_sendQRStudyCmd	= 1;
	m_sendQRSeriesCmd	= 1;
	m_sendQRImagesCmd	= 1;
	m_sendCMoveCmd		= 1;

}

CTstQRSCU::~CTstQRSCU()
{

}
#if 0
static char _DicomSvrAE[128]={0,};
static char _DicomSvrHost[512]={0,};
static char _MyAE[512]={0,};

static char _StudyDate[512]={0,};
static char _PatientName[512]={0,};
static char _PatientID[512]={0,};
static char _StudyID[512]={0,};
static char _SeriesID[512]={0,};
static char _CMoveDestAE[512]={0,};
#endif

bool CTstQRSCU::openAssociation()
{
	MC_STATUS mcStatus;

	closeAssociation();

	if(m_applicationID){
		mcStatus = MC_Release_Application(&m_applicationID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
		  ;
		}
	}
	/*
     *  Register this DICOM application
     */
	mcStatus = MC_Register_Application(&m_applicationID, m_MyAE.c_str());

	if (mcStatus != MC_NORMAL_COMPLETION)
    {
        
        return false;
    }


 
	printf(" Open Association  DicomServer host IP %s , AE %s, Port %d \n",
								m_DicomSvrHost.c_str(),m_DicomSvrAE.c_str(),m_DicomSvrPort);

	strcpy(m_serviceList,"test teset");
	mcStatus = MC_Open_Association(
						m_applicationID, 
						&m_associationID, 
						m_DicomSvrAE.c_str(),
						&m_DicomSvrPort, 
						(char*)m_DicomSvrHost.c_str(), 
						MY_SERVICE_LIST//m_serviceList
						);
 
	if (mcStatus != MC_NORMAL_COMPLETION)
    {
		printf(" *** Open Association faied *** \n");
        
        return false;
    }
 
	mcStatus = MC_Get_Association_Info( m_associationID, &m_asscInfo); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        ;
    }

    if (1)
    {
        printf("Connecting to Remote Application:\n");
        printf("  Remote AE Title:          %s\n", m_asscInfo.RemoteApplicationTitle);
        printf("  Local AE Title:           %s\n", m_asscInfo.LocalApplicationTitle);
        printf("  Host name:                %s\n", m_asscInfo.RemoteHostName);
        printf("  IP Address:               %s\n", m_asscInfo.RemoteIPAddress);
        printf("  Local Max PDU Size:       %d\n", m_asscInfo.LocalMaximumPDUSize);
        printf("  Remote Max PDU Size:      %d\n", m_asscInfo.RemoteMaximumPDUSize);
        printf("  Max operations invoked:   %d\n", m_asscInfo.MaxOperationsInvoked);
        printf("  Max operations performed: %d\n", m_asscInfo.MaxOperationsPerformed);
        printf("  Implementation Version:   %s\n", m_asscInfo.RemoteImplementationVersion);
        printf("  Implementation Class UID: %s\n\n\n", m_asscInfo.RemoteImplementationClassUID);
        
        printf("Services and transfer syntaxes negotiated:\n");
        
        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are 
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
#if 0
        mcStatus = MC_Get_First_Acceptable_Service(m_associationID,&m_servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
            printf("  %-30s: %s\n",m_servInfo.ServiceName, 
                              GetSyntaxDescription(m_servInfo.SyntaxType));
            
        
            mcStatus = MC_Get_Next_Acceptable_Service(m_associationID,&m_servInfo);
        }
            
        if (mcStatus != MC_END_OF_LIST)
        {
            ;
        }
#endif
        
        printf("\n");
    }
    else
	{
        printf("Connected to remote system [%s]\n ",m_DicomSvrAE.c_str() );
	}

	return true;
}
bool CTstQRSCU::closeAssociation()
{
	if(m_associationID<1) return false;

	  MC_STATUS       mcStatus;
	/*
     * A failure on close has no real recovery.  Abort the association
     * and continue on.
     */
    mcStatus = MC_Close_Association(&m_associationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        
        MC_Abort_Association(&m_associationID);
		return false;
    }
	return true;

}


bool CTstQRSCU::beginNewRequest()
{
	m_RequestCount++;

	if(m_ReopenAssociation!=0){
		printf(">>>==== re-open the association ====<<< \n");
		if(!openAssociation()){
			return false;
		}
	}


	printf("###NewRequest %d \n",m_RequestCount);

	return true;

}



void CTstQRSCU::initParam()
{


	m_runCount		= 0;
 
	m_RequestCount	= 0;
 



	changeParam();

	
	MC_STATUS mcStatus;
  
 
	mcStatus = MC_Library_Initialization ( NULL, NULL, NULL ); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to initialize library", mcStatus);
         ;
    }

		
	{
		DcmXTUtil	*pDcmXtUtil = IDcmLibApi::getDcmXTUtil();
		pDcmXtUtil->createServiceList(true/*isPropose*/,MY_SERVICE_LIST);
#if 0
		pDcmXtUtil->setNetworkTransferSyntax(true/*isPropose*/,DcmXT_EXS_LittleEndianExplicit,MY_SERVICE_LIST);
		 

		pDcmXtUtil->clearServiceList(true/*isPropose*/,MY_SERVICE_LIST);
		pDcmXtUtil->addSOPClassUID(true/*isPropose*/,UID_FINDPatientRootQueryRetrieveInformationModel,MY_SERVICE_LIST);
		//
		pDcmXtUtil->addSOPClassUID(true/*isPropose*/,UID_FINDStudyRootQueryRetrieveInformationModel,MY_SERVICE_LIST);
		pDcmXtUtil->addSOPClassUID(true/*isPropose*/,UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,MY_SERVICE_LIST);
		pDcmXtUtil->addSOPClassUID(true/*isPropose*/,UID_MOVEStudyRootQueryRetrieveInformationModel,MY_SERVICE_LIST);

		pDcmXtUtil->setMaxPDUSize(true/*isPropose*/,5*1024,MY_SERVICE_LIST);
#else
		//use default
#endif

	}
		 

//	IDcmLibApi::openLogger("dcmlibAPI.log",IDcmLib::LOGLEVEL_TRACE );
	//IDcmLibApi::openLogger("dcmlibAPI.log",IDcmLib::LOGLEVEL_ERROR );

}
void CTstQRSCU::changeParam()
{
 
	if(m_runCount++<1){
	 
	 
	//
		return;
	} 
	//

 
}
 


void CTstQRSCU::sendDataContinue()
{
	int run_i =0;
	while(true){
		run_i++;
		if(m_sendLoopNN>0){
			if(run_i>m_sendLoopNN) break;
		}
 
	//	for(int i=0;i<this->m_sendLoopNN;i++){

		if(! beginNewRequest() ){
	//		::Sleep(40000);
			continue;
		}
			 
			m_curStudyInstanceUID.empty();

	//		sendStudyData();

			if(m_sendQRStudyCmd==0){
				continue;
			}

			updateStudyData();

			if(!sendQRCmd()){
//				::Sleep(4000);
				continue;
			}

			 
			if(!receiveQRRSP()){
	//			::Sleep(4000);
				continue;
			}


			changeParam();

	//		::Sleep(300);

			if(m_curStudyInstanceUID.size()!=0){
				printf(" C-MOVE From %s %s to %s ...\n",
					m_DicomSvrHost.c_str(),m_DicomSvrAE.c_str(),
					m_CMoveDestAE.c_str());

				if(m_sendQRSeriesCmd==0){
					continue;
				}
				sendQRSeriesCmd();
				receiveQRRSP();

				if(m_sendQRImagesCmd==0){
					continue;
				}
				//
				sendQRImagesCmd();
				receiveQRRSP();

				if(m_sendCMoveCmd==0){
					continue;
				}

				if(sendCMoveCmd()){
					waitCMoveRSP();
				}
			}

//			::Sleep(m_LoopInterval);
	 
	}
}

void CTstQRSCU::getCurStudyDate()
{
	char char_buff[64];
	struct tm cur_time;
	 _getsystime(&cur_time);
  	sprintf(char_buff,"%04d%02d%02d",
					cur_time.tm_year+1900,
					cur_time.tm_mon+1,
					cur_time.tm_mday
					);
	m_curStudyDate = char_buff;
}
void CTstQRSCU::getCurSeriesTime()
{
	char char_buff[64];
	struct tm cur_time;
	 _getsystime(&cur_time);
  	sprintf(char_buff,"%02d%02d%02d",
					cur_time.tm_hour,
					cur_time.tm_min,
					cur_time.tm_sec
					);
	m_curSeriesTime = char_buff;
}

#if 1
bool CTstQRSCU::SetValue ( int A_messageid, unsigned long A_tag,
                            const char *A_value, const char *A_default, 
                            bool A_required )
{
    MC_STATUS      status;
    static char    S_prefix[] = "SetValue";

    if ( strlen(A_value) <= (size_t)0 )
    {
        /*
         * The tag we were gonna set, was not given a value
         */
        if ( A_required == false )
        {
            /*
             * It's not a required tag, so we can set it to NULL
             */
            status = MC_Set_Value_To_NULL ( A_messageid, A_tag );
            if ( status != MC_NORMAL_COMPLETION )
            {
              
                return false;
            }
            return true;
        }
        else if ( A_required == true )
        {
            /*
             * The tag is required so we must check to see if there is a
             *  default since we were not given a value for it.
             */
            if ( strncmp( A_default, NULL, sizeof(NULL) ) )
            {
                status = MC_Set_Value_From_String ( A_messageid, A_tag,
                    A_default );
                if ( status != MC_NORMAL_COMPLETION )
                {
                   
                    return ( false );
                }
                return ( true );
            } 
            else
            {
                /*
                 * This is a required tag and no value was given for it.  Ther
                 *  is no default value, so we cannot set it.  Its an error.
                 */
                printf("%s:%lX, Required Parameter not set.\n", S_prefix,A_tag);
                return ( false );
            }
        }
    }

    /*
     * This is just a usual tag that is being set, since we have a value for it
     */
    status = MC_Set_Value_From_String ( A_messageid, A_tag, A_value ); 
    if ( status != MC_NORMAL_COMPLETION )
    {
      
        printf("***          Tag: %lX\n", A_tag);
        return ( false );
    }
    return ( true );

} /* SetValue() */

#endif

bool CTstQRSCU::setupStudyLevel(int A_messageid)
{
#if 1
	/* 
     * Fields needed for STUDY_LEVEL (All Models) 
     */
	printf("Search StudyLevel: ");
    {
		 /* 
         * Fields needed by Study Root Model, and other Models   
         */
        {
		 
			printf("PatientName %s ",m_PatientName.c_str());
            if ( SetValue ( A_messageid, MC_ATT_PATIENTS_NAME,
                m_PatientName.c_str(), NULL,false) == false )
                return ( false );

			printf(" [%s] ",m_PatientID.c_str());
            if ( SetValue ( A_messageid, MC_ATT_PATIENT_ID,
                m_PatientID.c_str(), "*", false) == false )
                return ( false );
        } 

		printf("StudyDate [%s] ",m_StudyDate.c_str());
        if ( SetValue ( A_messageid, MC_ATT_STUDY_DATE,
            m_StudyDate.c_str(), NULL,false) == false )
                return ( false );

#if 0
        if ( SetValue ( A_messageid, MC_ATT_STUDY_TIME,
            "", NULL,false) == false )
                return ( false );
#else
		printf("StudyTime [%s] ",m_StudyTime.c_str());
		if ( SetValue ( A_messageid, MC_ATT_STUDY_TIME,
			m_StudyTime.c_str(), NULL,false) == false )
                return ( false );
#endif

        if ( SetValue ( A_messageid, MC_ATT_ACCESSION_NUMBER,
            "", NULL,false) == false )
                return ( false );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_ID,
            "", NULL,false) == false )
                return ( false );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            "", NULL,false) == false )
                return ( false );

		printf("Modality[CT]");
		if ( SetValue ( A_messageid, MC_ATT_MODALITIES_IN_STUDY,
            "CT", NULL,false) == false )
                return ( false );

		

        printf("\n");
 

    } /* end of STUDY_LEVEL */

#endif

	return true;
}

bool CTstQRSCU::setupSeriesLevel(int A_messageid)
{
#if 1
	/* 
     * Fields needed for STUDY_LEVEL (All Models) 
     */
	printf("Search SeriesLevel: ");
    {
		 /* 
         * Fields needed by Study Root Model, and other Models   
         */
        {
		 
			printf("StudUID %s ",m_curStudyInstanceUID.c_str());
            if ( SetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
                m_curStudyInstanceUID.c_str(), NULL,false) == false )
                return ( false );

			/*
			* 検索結果のフィールドとなる。
			*/
            if ( SetValue ( A_messageid, MC_ATT_SERIES_INSTANCE_UID,
                "", "*", false) == false )
                return ( false );
			//
			 if ( SetValue ( A_messageid, MC_ATT_SERIES_NUMBER,
                "", "*", false) == false )
                return ( false );
			//
			 if ( SetValue ( A_messageid, MC_ATT_SERIES_DESCRIPTION,
                "", "*", false) == false )
                return ( false );
			//
			 if ( SetValue ( A_messageid, MC_ATT_MODALITY,
                "", "*", false) == false )
                return ( false );
			//
			 if ( SetValue ( A_messageid, MC_ATT_NUMBER_OF_SERIES_RELATED_INSTANCES,
                "", "*", false) == false )
                return ( false );
			 //
			 if ( SetValue ( A_messageid, MC_ATT_BODY_PART_EXAMINED,
                "", "*", false) == false )
                return ( false );
			 //
			 if ( SetValue ( A_messageid, MC_ATT_SERIES_DATE,
                "", "*", false) == false )
                return ( false );
			 //
			 if ( SetValue ( A_messageid, MC_ATT_SERIES_TIME,
                "", "*", false) == false )
                return ( false );
        } 

		

        printf("\n");
 

    } /* end of STUDY_LEVEL */

#endif

	return true;
}
bool CTstQRSCU::sendQRCmd()
{
	MC_STATUS            status;
 //   int                  messageID;
     int                  responseMessageID;

	char                 *model = "STUDY_ROOT_QR_FIND" ;

	int	  ReqMessageID;

#if 1 //2010/06/10
	status =  MC_Open_Empty_Message(&ReqMessageID);
	if (status != MC_NORMAL_COMPLETION)
	{
	 
		return false;
	}

	//	Added by RL & VS 12/20/01 - We think this should be here.because Open_Empty_Message doesn't set it?
	status =  MC_Set_Value_From_Int(ReqMessageID, MC_ATT_MESSAGE_ID, ReqMessageID);
	if (status != MC_NORMAL_COMPLETION)
	{
		return false;
	}
	//
	status =  MC_Set_Service_Command(ReqMessageID, "STUDY_ROOT_QR_FIND", C_FIND_RQ); 
	if (status != MC_NORMAL_COMPLETION)
	{
		return false;
	}
#else
	status = MC_Open_Message ( &messageID, model, C_FIND_RQ );     
    if ( status != MC_NORMAL_COMPLETION )
    {
         
        return false;
    }
	status = MC_Set_Message_Transfer_Syntax(messageID,IMPLICIT_LITTLE_ENDIAN);
	if ( status != MC_NORMAL_COMPLETION )
    {
         
        return false;
    }
	
#endif

#if 0
	status = MC_Set_Value_From_String (messageID,
        MC_ATT_QUERY_RETRIEVE_LEVEL, "PATIENT");//"STUDY" );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("QUERY_RETRIEVE_LEVEL, MC_Set_Value_From_String %d", status );
		MC_Free_Message ( &messageID );
        return false;
    }


	/* 
     * Fields needed for PATIENT_LEVEL (Patient Root, Patient/Study Root) 
     */
    {
		 
        if ( SetValue ( messageID, MC_ATT_PATIENTS_NAME, 
            m_PatientName.c_str(), NULL, false) == false )
		{
			MC_Free_Message ( &messageID );
            return ( false );
		}

        if ( SetValue ( messageID, MC_ATT_PATIENT_ID, 
            m_PatientID.c_str(), "*", false) == false )
		{
			MC_Free_Message ( &messageID );
            return ( false );
		}
             
    } /* end of PATIENT_LEVEL */
#endif

	status = MC_Set_Value_From_String (ReqMessageID,
        MC_ATT_QUERY_RETRIEVE_LEVEL,  "STUDY" );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("QUERY_RETRIEVE_LEVEL, MC_Set_Value_From_String %d", status );
		MC_Free_Message ( &ReqMessageID );
        return false;
    }

	if(!setupStudyLevel(ReqMessageID))
	{
		printf("setupStudyLevel erro" );
		MC_Free_Message ( &ReqMessageID );
        return false;
	}
	

	if(0){
//		CTstVLIDicomImage dicmImageTemp;
//		dicmImageTemp.createFromMessasgeID(messageID);
//		dicmImageTemp.saveDicom("RQMsg.dcm");
	}
	/*
     * Send off the message 
     */
    status = MC_Send_Request_Message ( m_associationID, ReqMessageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &ReqMessageID );
         
        return false;
    }

    status = MC_Free_Message ( &ReqMessageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
         
        return ( false );
    }

	return true;
}
bool CTstQRSCU::GetValue ( int A_messageid, unsigned long A_tag, 
                            char *A_value, int A_size, char *A_default )
{
    MC_STATUS      status;
   
    status = MC_Get_Value_To_String ( A_messageid, A_tag, A_size,
                                      A_value );
    if ( status == MC_NULL_VALUE || status == MC_EMPTY_VALUE  ||
         status == MC_INVALID_TAG )
    {
        if (!A_default)
        {
            A_value[0] = '\0';
            return ( false );
        }
        strcpy ( A_value, A_default );
    }
    else if ( status != MC_NORMAL_COMPLETION )
    {
        printf("MC_Get_Value_To_String %d ", status );
        printf("***          Tag:  %lX\n", A_tag);
        return ( false );
    }
    return ( true );
} /* GetValue() */


bool CTstQRSCU::receiveQRRSP(int ReqMessageID)
{
	MC_STATUS            status;
 
    int                  responseMessageID;

	MC_COMMAND           command;
	char                 *serviceName;

	unsigned int         response;
	/* 
     * A single response message is sent for each
     * match to the find request.  Wait for all of these
     * response messages here.  This loop is exited when
     * the status contained in a response message
     * is equal to a failure, or C_FIND_SUCCESS.
     */
	int timeOut = 20; /* Sec*/
	bool done = false;

	int itemCount = 0;

	printf("C-FIND -----------------------------------------\n");

	int rand_sel_index = (int)( rand()/(float)RAND_MAX * 20.0 -10.0);

	int timeout_counter = 0;
    while (!done  )
    {
        status = MC_Read_Message ( m_associationID,
                                   timeOut,
                                   &responseMessageID,
                                   &serviceName,
                                   &command );
        if ( status == MC_TIMEOUT )
        {
            printf("Timed out in MC_Read_Message.  Calling again.\n");
			if(timeout_counter++ >2 ) return false;
            continue;
        }
        else if ( status != MC_NORMAL_COMPLETION )
        {
            printf("MC_Read_Message %d NULL",status );
            return false;
        }

        /* 
         * The status of the find response is contained in
         * the DICOM group 0 elements.  Retreive this status
         * and examine it here.
         */
        status = MC_Get_Value_To_UInt ( responseMessageID,
                                        MC_ATT_STATUS,
                                        &response );
        if ( status != MC_NORMAL_COMPLETION )
        {
             
            return ( false );
        }                                             

        /* 
         * If you are thinking that this error section could be better handled
         *  using a switch statement, you are right.  However under the OS9000
         *  compiler it did not work as a switch.  The solution was to write
         *  it as an if-else.
         */
        if ( response == C_FIND_SUCCESS )
        {
            /*
             * If we get a success, we are finished 
             */
			printf ( "  received itemCount %d \n",itemCount);
            printf ( "  Response is C_FIND_SUCCESS\n" );
           
            done = true;
        }
        else if ( response == C_FIND_CANCEL_REQUEST_RECEIVED ) 
        {
            /* 
             * If we get a cancel,  we are finished 
             */
            printf (" Response C_FIND_CANCEL_REQUEST_RECEIVED\n" );
           
            done = true;
        }
        else if ( response == C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT || 
                  response == C_FIND_PENDING )
        {
            if ( response == C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT )
            {
                printf ( " C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT\n");
                printf ( "\t\twas received...\n" );
            }
			itemCount ++;
            /*
             * This means we got a real message with data 
             */
    
            /*
             * Hold off on Validation of CFIND until we know it is not  
             *   a C_FIND_SUCCESS since C_FIND_SUCCESS will not validate 
             *   because it is an empty message.                         
             */
#if 0
      //      ValMessage      (responseMessageID, "C-FIND-RSP");
       //     WriteToListFile (responseMessageID, "cfindrsp.msg");

            qrStatus = ReadCFINDMessage(&data, A_data, responseMessageID, 
                A_myConfig); 
            if ( qrStatus == QR_FAILURE )
            {
                MC_Free_Message(&responseMessageID);
                return ( QR_FAILURE );
            }

            if(A_list->numData < A_myConfig->maxQueryResponses )
            {
                /* 
                 * Add it to the list of responses to the query
                 */
                qrStatus = AddToList ( A_data->level, A_list, &data );
                if ( qrStatus == QR_FAILURE )
                {
                    MC_Free_Message(&responseMessageID);
                    return ( QR_FAILURE );
                }
            }
            else if ( S_once != TRUE ) 
            {  
                /* 
                 * We have filled up our buffer of C-FIND-RSP messages we
                 * can receive, we now send a cancel to the server to stop
                 * sending these response messages.  Note that because of
                 * a delay in when the SCP receives the C-CANCEL, several
                 * responses may continue to come in.
                 */
                if (CancelCFINDRQ(&A_myConfig->associationID, model) ==
                    QR_FAILURE)
                    printf("ERROR: sending C-FIND-RSP,  C-CANCEL\n");
                S_once = TRUE;
            }
#endif
			{
#if 0
				char patient_name[64];
				if ( GetValue ( responseMessageID, MC_ATT_PATIENTS_NAME,
					patient_name,
					sizeof ( patient_name),
					"" ) == false )
					return ( false );

				char patient_id[64];
				if ( GetValue ( responseMessageID, MC_ATT_PATIENT_ID, 
					patient_id,
					sizeof ( patient_id ),
					"" ) == false )
					return ( false );

				printf(" PatientName: %s, ID: %s \n",patient_name,patient_id);
#else
				bool selectToCMoveFlag = false;
				if(m_sendCMoveCmd){
					selectToCMoveFlag = itemCount==rand_sel_index;
				}

				selectToCMoveFlag = true;
				readStudyLevelRSP(responseMessageID,selectToCMoveFlag);
#endif
				bool cancel_flag = false;
				if(cancel_flag){
					if(ReqMessageID>0){
						sendQRCancel(ReqMessageID);
					}
					break;
				}
     
			}
        }
        else
        {
            /* 
             * Some other kind of error message 
             */
            printf(" Response is Unknown Error:  %X.\n", response);
            
            done = true;
        }

        /*
         * Free the valid response message and continue 
         * waiting for another response.
         */
        status = MC_Free_Message ( &responseMessageID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            
            return ( false );
        }
    } /* while(done != TRUE) */


  printf("------------------------------------------------\n");
    return ( true );
} /* SendCFINDMessage() */



bool CTstQRSCU::readStudyLevelRSP(int A_messageid,bool setMoveInstance)
{
    /*
     * required fields for STUDY_LEVEL 
     */
    char str_buffer[256];

	printf(" >>>> ");
	{
            /* 
             * If it is not a STUDY_MODEL query, then the patient's name
             *  goes with the patient level structure
             */
            if ( GetValue ( A_messageid, MC_ATT_PATIENTS_NAME, 
                str_buffer,
				sizeof ( str_buffer ),
				"") == false )
				return ( false );
				printf("Name %s ",str_buffer);

            if ( GetValue ( A_messageid, MC_ATT_PATIENT_ID,
                str_buffer,
				sizeof ( str_buffer ),
				"") == false )
				return ( false );
				printf("[%s] ",str_buffer);
       }
	///
 
    {
        if ( GetValue ( A_messageid, MC_ATT_STUDY_DATE, 
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		printf("StudyDate %s  ",str_buffer);

        if ( GetValue ( A_messageid, MC_ATT_STUDY_TIME, 
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		printf("StudyTime %s ",str_buffer);

        if ( GetValue ( A_messageid, MC_ATT_ACCESSION_NUMBER, 
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
	//	printf("Study AccessionNumber %s \n",str_buffer);

     
		if(setMoveInstance){
			if ( GetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		
			m_curStudyInstanceUID = str_buffer;
			//
			if ( GetValue ( A_messageid, MC_ATT_SERIES_INSTANCE_UID,
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		
			m_curSerieInstanceUID = str_buffer;
			//
			if ( GetValue ( A_messageid, MC_ATT_SOP_INSTANCE_UID,
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		
			m_curImageInstanceUID = str_buffer;

			
			printf("\n ### setup C-MOVE StudyUID %d, \n     SeriesUID %s,\n     ImageUID %s \n",
				m_curStudyInstanceUID.c_str(),
				m_curSerieInstanceUID.c_str(),
				m_curImageInstanceUID.c_str());
		}
#if 0

        if ( GetValue ( A_messageid, MC_ATT_STUDY_ID,
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		printf("Study ID %s \n",str_buffer);

        if ( GetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            str_buffer,
            sizeof ( str_buffer ),
            "") == false )
            return ( false );
		printf("Study InstanceID %s \n",str_buffer);
#endif
       


    } /* End of STUDY_LEVEL */

	printf(" \n");

	return true;
}

bool CTstQRSCU::sendQRCancel(int iMsgID)
{
	MC_STATUS            status;
	// Prepare and Send the C-FIND-CANCEL request message to the SCP 
	int cancelMessageID = -1;
	status =  MC_Open_Empty_Message(&cancelMessageID);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("Create Empty Message in VLIDicom::DoCFindCancellation() %d", status);
		return status;
	}
 
	status = MC_Set_Service_Command(cancelMessageID, "STUDY_ROOT_QR_FIND", C_CANCEL_FIND_RQ);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("Set Model in VLIDicom::DoCFindCancellation() %d", status);
		MC_Free_Message(&cancelMessageID);
		return status;
	}

	//
	//	Set Message ID from the active Request message so we can cancel the correct message
	status = MC_Set_Value_From_Int(cancelMessageID, MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO, iMsgID);
 	if (status != MC_NORMAL_COMPLETION)
	{
		printf("Set Message ID in VLIDicom::DoCFindCancellation() %d", status);
		MC_Free_Message(&cancelMessageID);
		return status;
	}
 
	//	Send the cancel request
	status = MC_Send_Request_Message(m_associationID, cancelMessageID);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("Send C-FIND-CANCEL RQ in VLIDicom::DoCFindCancellation() %d", status);
		MC_Free_Message(&cancelMessageID);
		return status;
	}
	
	MC_Free_Message(&cancelMessageID);


	return true;
}