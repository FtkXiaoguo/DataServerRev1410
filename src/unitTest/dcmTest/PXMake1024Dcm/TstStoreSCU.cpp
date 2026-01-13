// TstStoreSCU.cpp: CTstStoreSCU クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <ctime>

#include "TstStoreSCU.h"

#ifdef USE_NEW_LIB
#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "PxDicomImage.h"
using namespace XTDcmLib;

#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif

#include "rtvloadoption.h"

#include "AqCore/trplatform.h"

#include "TstVLIDicomImage.h"

#include "genData\OctDatBase.h"

#include "TstSrcData.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
	ServiceInfo    m_servInfo;
	AssocInfo      m_asscInfo;
///

static char _str_buff[1024];

#ifndef USE_NEW_LIB
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
#endif
///
CTstStoreSCU::CTstStoreSCU()
{

	m_rootUID = "1.2.392.200036.9163"  ;  //PreXion DICOM root UID
	m_useTimeTickCount = 1;
	//
	/*
	* use Rescale:
	*  PixelVale:  [0,0xffff]  -- unsigned short
	*  HU = Slope*PixelValue + intercept   [-1024,0xffff-1024]
	*     
	*
	*/
	m_useRescale = 1;
	m_RescaleIntercept = -1024.0;
	m_RescaleSlope = 1.0;


	m_EmbeddingCheckPattern = 1;

	m_myTestUID = "1";
	m_PatientName = "XtTestSCU--5";
	m_PatientID	= "1111118";
	m_StudyID	= "2222228";
	m_SeriesID  = "3333338";

	m_applicationID = 0;

	m_MyAE = "TestSCU1";

 
	m_DicomSvrAE = "MONE_AE";
	m_DicomSvrHost = "172.17.3.72";
	m_DicomSvrPort = 105;

//
	m_sizeX = 128;
	m_sizeY = 128;
	m_sizeZ = 128;
//
	m_pitchX	= 0.1;
	m_pitchY	= 0.1;
	m_pitchZ	= 0.1;
//
	m_Modality = 0;
			
	m_testDataType = 3;		 
//
	m_sendLoopNN  = 1;
	m_seriesNN = 1;
    m_seriesNN_mode = 0; //const mode
    m_studyNN = 1;
    m_studyNN_mode = 0; //const mode

//
	m_ImageCount = 0;
	m_PatientCount = 0;
	m_StudyCount = 0;
	m_SeriesCount = 0;
//
	m_sourceDataType = 0;
	m_setPixelValue  = 0;
 
	m_TstSrcData = 0;

	m_associationID = 0;

	m_ReopenAssociation = 1;
//
	m_ReferringPhysician	= "";
	m_StudyDescription		= "";
}

CTstStoreSCU::~CTstStoreSCU()
{
	if(!m_TstSrcData) delete m_TstSrcData;
	
}
static char _DicomSvrAE[128]={0,};
			 
static char _DicomSvrHost[512]={0,};
			 
static char _MyAE[512]={0,};
		 

static char _myTestUID[512]={0,};
			 
static char _PatientName[512]={0,};
			 
static char _PatientID[512]={0,};
			 
static char _StudyID[512]={0,};
			 
static char _SeriesID[512]={0,};
		 
static char _sourceDataFolder[1024]={0,};
		 
//
static char _ReferringPhysician[512]={0,};
		 
static char _StudyDescription[512]={0,};
		 
//
static char _logFileName[512]={0,};
		 
static char _rootUID_temp[128]={0,};

bool CTstStoreSCU::loadOption(const char *fileName)
{
	iRTVOptions cstore_opt;

	cstore_opt.Add("myTestUID",_myTestUID,sizeof(_myTestUID));
	cstore_opt.Add("DcmServerAE",_DicomSvrAE ,		sizeof(_DicomSvrAE));
	cstore_opt.Add("DcmServerHost",_DicomSvrHost , sizeof(_DicomSvrHost));
	cstore_opt.Add("DcmServerPort",&m_DicomSvrPort , sizeof(m_DicomSvrPort));
	cstore_opt.Add("MyAE",_MyAE , sizeof(_MyAE));
//
	cstore_opt.Add("PatientName",_PatientName , sizeof(_PatientName));
	cstore_opt.Add("PatientID",_PatientID , sizeof(_PatientID));
	cstore_opt.Add("StudyID",_StudyID , sizeof(_StudyID));
	cstore_opt.Add("SeriesID",_SeriesID , sizeof(_SeriesID));
//
	cstore_opt.Add("VolumeSizeX",&m_sizeX,sizeof(m_sizeX));
	cstore_opt.Add("VolumeSizeY",&m_sizeY,sizeof(m_sizeY));
	cstore_opt.Add("VolumeSizeZ",&m_sizeZ,sizeof(m_sizeZ));
//
	cstore_opt.Add("TestDataType",&m_testDataType, sizeof(m_testDataType));

	cstore_opt.Add("SendLoopNN",&m_sendLoopNN,sizeof(m_sendLoopNN));
	cstore_opt.Add("SeriesNN",&m_seriesNN,sizeof(m_seriesNN));
	cstore_opt.Add("SeriesNN_mode",&m_seriesNN_mode,sizeof(m_seriesNN_mode));
	cstore_opt.Add("StudyNN",&m_studyNN,sizeof(m_studyNN));
	cstore_opt.Add("StudyNN_mode",&m_studyNN_mode,sizeof(m_studyNN_mode));
//
	cstore_opt.Add("VolumePitchX",&m_pitchX,sizeof(m_pitchX));
	cstore_opt.Add("VolumePitchY",&m_pitchY,sizeof(m_pitchY));
	cstore_opt.Add("VolumePitchZ",&m_pitchZ,sizeof(m_pitchZ));
 //
	cstore_opt.Add("Modality",&m_Modality,sizeof(m_Modality));
	
 //

	cstore_opt.Add("SourceDataType",&m_sourceDataType,sizeof(m_sourceDataType));
	cstore_opt.Add("SourceFolderName",_sourceDataFolder,sizeof(_sourceDataFolder));

	//
	cstore_opt.Add("setPixelValue",&m_setPixelValue,sizeof(m_setPixelValue));
	 
//
	cstore_opt.Add("ReferringPhysician",_ReferringPhysician,sizeof(_ReferringPhysician));
	cstore_opt.Add("StudyDescription",_StudyDescription,sizeof(_StudyDescription));
		
//
	cstore_opt.Add("LogFileName",_logFileName,sizeof(_logFileName));
	
	cstore_opt.Add("EmbeddingCheckPattern",&m_EmbeddingCheckPattern,sizeof(m_EmbeddingCheckPattern));

	cstore_opt.Add("useRescale",&m_useRescale,sizeof(m_useRescale));
	cstore_opt.Add("RescaleIntercept",&m_RescaleIntercept,sizeof(m_RescaleIntercept));
	cstore_opt.Add("RescaleSlope",&m_RescaleSlope,sizeof(m_RescaleSlope));

	cstore_opt.Add("RootUID",_rootUID_temp,sizeof(_rootUID_temp));
	cstore_opt.Add("useTimeTickCount",&m_useTimeTickCount,sizeof(m_useTimeTickCount));
	 

	if(!cstore_opt.Load(fileName)){
		return false;
	}

	m_logFileName = _logFileName;
	
	bool none_log_file = false;
	 
	if( m_logFileName.size()>1 )
	{
		none_log_file = (m_logFileName != "null");
	}
	if(!none_log_file){
#if 0
		char _str_buff[512];
		WORD procID = ::GetCurrentProcessId();
		sprintf(_str_buff,"TstStoreSCU_%x.log",procID);
		string use_logFileName = _str_buff;
		if(!m_logFileName.empty())
		{
			use_logFileName = m_logFileName;
		}
		string full_log_name = "C:\\AQNetLog\\"+use_logFileName;
		m_Logger.SetLogFile(full_log_name.c_str());
#endif
	}

	

	m_DicomSvrAE	= _DicomSvrAE;
	m_DicomSvrHost	= _DicomSvrHost;
	m_MyAE			= _MyAE;
 //
	m_myTestUID   = _myTestUID;
	m_PatientName = m_myTestUID+_PatientName;
//	m_PatientID	= _PatientID+m_myTestUID;
	m_PatientID	= _PatientID;
	if(strlen(_StudyID)<1){
		m_StudyID	=  m_myTestUID + "00000";
	}else{
		m_StudyID	= _StudyID+m_myTestUID;
	}

	if(strlen(_SeriesID)<1){
		m_SeriesID	=  m_myTestUID + "000000";
	}else{
		m_SeriesID  = _SeriesID+m_myTestUID;
	}
//
	m_sourceDataFolder =  _sourceDataFolder;
 
	m_ReferringPhysician	= _ReferringPhysician;
	m_StudyDescription		= _StudyDescription;;

	if(strlen(_rootUID_temp)>1){
		m_rootUID = _rootUID_temp;
	}
	//
	{
		m_Logger.LogMessage(">>>> C-Store <<<\n");
		m_Logger.LogMessage(" DICOM Server \n");
		m_Logger.LogMessage("             HOST:%s \n",m_DicomSvrHost.c_str());
		m_Logger.LogMessage("               AE:%s \n",m_DicomSvrAE.c_str());
		m_Logger.LogMessage("              Port:%d \n",m_DicomSvrPort);
		m_Logger.LogMessage("             MyAE:%s \n",m_MyAE.c_str());

		if(m_EmbeddingCheckPattern == 0){
			m_Logger.LogMessage("     -- None EmbeddingCheckPattern \n");
		}else{
			m_Logger.LogMessage("     -- EmbeddingCheckPattern \n");
		}

		m_Logger.LogMessage("   \n");
		m_Logger.FlushLog();
	}
	return true;
}
bool CTstStoreSCU::openAssociation()
{
	printf(" *** >> CTstStoreSCU::openAssociation   *** \n");


	MC_STATUS mcStatus;

 	closeAssociation();

	 

	if(m_applicationID){
		mcStatus = MC_Release_Application(&m_applicationID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
		  ;
		}
	}

	printf(" *** >>MC_Register_Application   *** \n");

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

	m_Logger.LogMessage("Open Association  DicomServer host IP %s , AE %s, Port %d \n",
								m_DicomSvrHost.c_str(),m_DicomSvrAE.c_str(),m_DicomSvrPort);
	m_Logger.FlushLog();

	printf(" *** >> to Open Association   *** \n");

	strcpy(m_serviceList,"test teset");
	mcStatus = MC_Open_Association(
						m_applicationID, 
						&m_associationID, 
						m_DicomSvrAE.c_str(),
						&m_DicomSvrPort, 
						(char*)m_DicomSvrHost.c_str(), 
						"TIDICOM_SCU_Service_List"//m_serviceList
						);
 
	if( mcStatus == MC_ASSOCIATION_REJECTED){
		printf(" *** Open Association rejected *** \n");
        m_Logger.LogMessage("ERROR *** Open Association rejected ***\n");
		m_Logger.FlushLog();
        return false;
	}else
	if (mcStatus != MC_NORMAL_COMPLETION)
    {
		printf(" *** Open Association faied *** \n");
        m_Logger.LogMessage("ERROR *** Open Association faied ***\n");
		m_Logger.FlushLog();
        return false;
    }
 
	bool debug_asssociation = false;
	int val_temp = rand() ;
	if( (val_temp % 3) == 0 ){
		debug_asssociation = true;
	}
	if(debug_asssociation){
		MC_Abort_Association(&m_associationID);
		closeAssociation();

		printf(" *** abort_association end  *** \n");
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
#if 1
        mcStatus = MC_Get_First_Acceptable_Service(m_associationID,&m_servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
#ifdef USE_NEW_LIB
				printf("  %-30s: %s\n",m_servInfo.ServiceName, 
						 GetSyntaxDescription(m_servInfo.SyntaxType) );
#else
            printf("  %-30s: %s\n",m_servInfo.ServiceName, 
                              GetSyntaxDescription(m_servInfo.SyntaxType));
#endif
            
        
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
bool CTstStoreSCU::closeAssociation()
{
	if(m_associationID<1) return false;

	m_Logger.LogMessage("Close Association   \n");
	m_Logger.FlushLog();

	  MC_STATUS       mcStatus;
	/*
     * A failure on close has no real recovery.  Abort the association
     * and continue on.
     */
    mcStatus = MC_Close_Association(&m_associationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        
 //       MC_Abort_Association(&m_associationID);
		return false;
    }
	return true;

}
bool CTstStoreSCU::SendImage(unsigned int dicomMsgID)
{
	unsigned long start_time = ::GetTickCount();

    MC_STATUS       mcStatus;

   char          affectedSOPinstance[64+2];
  char   SOPClassUID[64+2];    /* SOP Class UID of the file */
    char   serviceName[48];             /* MergeCOM-3 service name for SOP Class */
    char   SOPInstanceUID[64+2]; /* SOP Instance UID of the file */
    
	mcStatus = MC_Get_Value_To_String(dicomMsgID, 
                        MC_ATT_SOP_CLASS_UID,
                        sizeof(SOPClassUID),
                        SOPClassUID);

    /* Get the SOP class UID and set the service */
    mcStatus = MC_Get_MergeCOM_Service(SOPClassUID, serviceName, sizeof(serviceName));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return false;
    }            
                 
    mcStatus = MC_Set_Service_Command(dicomMsgID, serviceName, C_STORE_RQ);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
       return false;
    }
 
           
	//  Get affected SOP Instance UID 
	mcStatus = MC_Get_Value_To_String(dicomMsgID,
					MC_ATT_SOP_INSTANCE_UID,
					sizeof(affectedSOPinstance),
					affectedSOPinstance);

	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		return false;
	}
    /* set affected SOP Instance UID */
    mcStatus = MC_Set_Value_From_String(dicomMsgID, 
                      MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                      affectedSOPinstance);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return false;
    }
            
 
    /*
     *  Send the message
     */
  

    mcStatus = MC_Send_Request_Message(m_associationID, dicomMsgID);
    if (mcStatus == MC_ASSOCIATION_ABORTED
     || mcStatus == MC_SYSTEM_ERROR)
    {
        /*
         * At this point, the association has been dropped, or we should
         * drop it in the case of MC_SYSTEM_ERROR.
         */
		m_Logger.LogMessage("*** CTstStoreSCU::SendImage MC_Send_Request_Message error [%d]\n",mcStatus );
		m_Logger.FlushLog();
       return false;
    }
    else if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * This is a failure condition we can continue with
         */
		m_Logger.LogMessage("*** CTstStoreSCU::SendImage MC_Send_Request_Message error [%d]\n",mcStatus );
		m_Logger.FlushLog();
        return false;
    }
    
	//
	/*
     *  Wait for response
     */
	int               A_timeout = 30; /*Sec*/
	int             responseMessageID;
	char*           responseService;
    MC_COMMAND      responseCommand;
    mcStatus = MC_Read_Message(m_associationID, A_timeout, &responseMessageID,
                 &responseService, &responseCommand);
    if (mcStatus == MC_TIMEOUT){
		m_Logger.LogMessage("*** CTstStoreSCU::SendImage MC_Read_Message error MC_TIMEOUT [%d]\n",mcStatus );
		m_Logger.FlushLog();
		return false;
	}
      
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
		m_Logger.LogMessage("*** CTstStoreSCU::SendImage MC_Read_Message error  [%d]\n",mcStatus );
        m_Logger.FlushLog();
		return false;
    }
    
	unsigned int    ret_dicomMsgID;

    mcStatus = MC_Get_Value_To_UInt(responseMessageID, 
                    MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO,
                    &ret_dicomMsgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
		MC_Free_Message(&responseMessageID);
        return false;
    }

	if((ret_dicomMsgID&(0x0000ffff)) !=  (dicomMsgID&(0x0000ffff))){
 		MC_Free_Message(&responseMessageID);
		m_Logger.LogMessage("*** CTstStoreSCU::SendImage ret_dicomMsgID[%d] !=  dicomMsgID[%d]\n",ret_dicomMsgID ,dicomMsgID);
 		m_Logger.FlushLog();
		return false;
	}

	unsigned int cStoreResponse ;
	 mcStatus = MC_Get_Value_To_UInt(responseMessageID, MC_ATT_STATUS, &cStoreResponse);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        MC_Free_Message(&responseMessageID);
        return false;  
    }
	//
	char *StoreResponseStr;
	if (cStoreResponse & 0xA000 || 
		cStoreResponse & 0xC000 || 
		cStoreResponse == 0x0110 ||		//  Processing failure
		cStoreResponse == 0x0122)		//	Refused: SOP Class not supported
	{
	 
		StoreResponseStr = "*** failure";
		
	}
	else if (cStoreResponse & 0xB000 || 
		cStoreResponse == 0x0111 ||		//	Refused: Duplicate SOP
		cStoreResponse == 0xD000)		//	Some vendors use as Refused: Duplicate SOP
	{
		StoreResponseStr = "### Refused";
	}
	else if (cStoreResponse == 0x0000)
	{
		;;
	}
	else
	{
		StoreResponseStr = "*** Unknown"; 
	}
	///
	if(cStoreResponse != 0){
		printf(" *** CStorResponse: %d %s\n",cStoreResponse,StoreResponseStr);
		m_Logger.LogMessage("*** CTstStoreSCU::SendImage CStorResponse: %d %s\n",cStoreResponse,StoreResponseStr);
		m_Logger.FlushLog();
	}


	mcStatus = MC_Free_Message(&responseMessageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
       return false;
    }

	unsigned long end_Time = ::GetTickCount();

	m_spentTimeSum = (end_Time-start_time)/1000.0;

    return (cStoreResponse == 0x0000);
}    
bool CTstStoreSCU::beginNewStudy()
{
	m_StudyCount++;

	if(m_ReopenAssociation!=0){
		printf(">>>==== re-open the association ====<<< \n");
		if(!openAssociation()){
			return false;
		}
	}

	if(m_CurStudyCount++>=m_CurStudyNN){
		beginNewPatient() ;
		m_CurStudyCount = 0;
	}
	printf("###NewStudy %d \n",m_StudyCount);
	m_Logger.LogMessage("###NewStudy %d \n",m_StudyCount);
	m_Logger.FlushLog();

	genCurSeriesNN();
	getCurStudyDate();

	genStudyInstanceUID();

	return true;
}
void CTstStoreSCU::beginNewPatient()
{
	m_PatientCount++;
	printf("#NewPatient %d \n",m_PatientCount);

	genCurStudyNN();
}
void CTstStoreSCU::beginNewSeries()
{
	getCurSeriesTime();
 
	genSeriesInstanceUID();

}
void CTstStoreSCU::beginNewImage()
{
	if(m_testDataType<0){
		float rnd_dd = rand()/(float)RAND_MAX;
		m_CurTestDataType = (int)( rnd_dd*4);
	}
}

void CTstStoreSCU::genCurSeriesNN()
{
	if(m_seriesNN_mode==0){
		m_CurSeriesNN = m_seriesNN;
	}else{
		float rnd_dd = rand()/(float)RAND_MAX;
		m_CurSeriesNN = (int)( rnd_dd*m_seriesNN+1.5);
	}
	
}
void CTstStoreSCU::genCurStudyNN()
{
	if(m_studyNN_mode==0){
		m_CurStudyNN = m_studyNN;
	}else{
		float rnd_dd = rand()/(float)RAND_MAX;
		m_CurStudyNN = (int)(rnd_dd*m_studyNN+1.5);
	}
}

void CTstStoreSCU::initCount()
{
	genCurSeriesNN();
	genCurStudyNN();

	m_ImageCount		= 0;
	m_SeriesCount	= -1;
	m_StudyCount	= -1;
	m_PatientCount	= -1;

	m_CurStudyCount = 0;
	m_CurSeriesCount = 0;


	if(m_testDataType<0){
		m_CurTestDataType = 0;
	}else{
		m_CurTestDataType = m_testDataType;
	}

	beginNewPatient();
	beginNewStudy();

	doSeriesCount();

	if(m_TstSrcData) delete m_TstSrcData;
	if(m_sourceDataType!=0){
		m_TstSrcData = new CTstSrcData;
		m_TstSrcData->openStudy(m_sourceDataFolder);
	}

	getCurSeriesTime();
	getCurStudyDate();
}
bool CTstStoreSCU::doSeriesCount()
{
 
 

	m_SeriesCount++;

	beginNewSeries();
	if(m_CurSeriesCount++ >=m_CurSeriesNN ){
		if(!beginNewStudy()) {
			return false;
		}
		m_CurSeriesCount = 1;
	}
	setupUID();

	return true;
}
void CTstStoreSCU::setupUID()
{
static char _char_buff[128];
//Patient
	sprintf(_char_buff,"%d",m_PatientCount+1);
	m_CurPatientName	= m_PatientName + _char_buff;

	
	m_CurPatientID		= m_PatientID + _char_buff;
//StudyID
	sprintf(_char_buff,"%d",m_StudyCount+1);
	m_CurStudyID		= m_StudyID + _char_buff;
//SeriesID
	sprintf(_char_buff,"%d",m_SeriesCount+1);
	m_CurSeriesID		= m_SeriesID + _char_buff;

	unsigned long p_id_int = atol(m_PatientID.c_str());
	unsigned hig_dig = atoi(m_myTestUID.c_str());

	p_id_int += (hig_dig*10000000);
	sprintf(_char_buff,"%08d",p_id_int+m_PatientCount);

	m_CurPatientID = _char_buff;

}
bool CTstStoreSCU::sendStudyData(int sizeX,int sizeY,int sizeZ)
{
 
	if(sizeX<1) sizeX=m_sizeX;
	if(sizeY<1) sizeY=m_sizeY;
	if(sizeZ<1) sizeZ=m_sizeZ;

	float data_range = (0x7ff);
	if(sizeZ>1){
		data_range/=(float)(sizeZ-1.0);
	}

	{
		printf(" [%d] sizeX: %d, sizeY: %d, sizeZ: %d, type: %d \n",
			m_CurSeriesCount,
			sizeX,sizeY,sizeZ,m_CurTestDataType);
	}
	COctDatBase *DataGenPtr = new COctDatBase;
	DataGenPtr->setDim(sizeX,sizeY,sizeZ);

	if(m_useRescale){
		if(m_CurTestDataType == 10){
			DataGenPtr->genData(DataType_USHORT,10,-1024-m_setPixelValue,0);
			
		}else{
			DataGenPtr->genData(DataType_USHORT,m_CurTestDataType,-1024-m_RescaleIntercept,(1000.0-m_RescaleIntercept)/m_RescaleSlope);
		}
			 
	}else{
		if(m_CurTestDataType == 10){
			DataGenPtr->genData(DataType_SHORT,10,m_setPixelValue,0);
		}else{
			DataGenPtr->genData(DataType_SHORT,m_CurTestDataType,-1024,1000);
		}
		 
	}
	

	m_spentTimeSum = 0.0f;

	printf("send: \n");
	unsigned long start_time = ::GetTickCount();

	bool error_flag =false;
	for(int z_i=0;z_i<sizeZ;z_i++){
		printf(".");
		CTstVLIDicomImage dicomImage;
		dicomImage.openNewDicom();
#ifdef USE_NEW_LIB
		CPxDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
#else
		VLIDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
#endif
		if(!dicomImage_dicom) {
			error_flag = true;
			break;
		} 

		 
		dicomImage.setPatientName(m_CurPatientName);
		dicomImage.setPatientID(m_CurPatientID);
		dicomImage.setStudyID(m_CurStudyID);

		dicomImage.setStudyInstanceUID(m_curStudyInstanceUID);
	//	dicomImage.setSeriesInstanceUID(m_CurSeriesID);
		dicomImage.setSeriesInstanceUID(m_curSeriesInstanceUID);


		dicomImage.setupPitch(m_pitchX,m_pitchY,m_pitchZ);

		if(m_useRescale){
			dicomImage.setupRescale(m_RescaleIntercept,m_RescaleSlope,true);
			dicomImage.setBits(12,11,16);
		}else{
			dicomImage.setupRescale(0.0,1.0,false);
			dicomImage.setBits(16,15,16);
		}

		{
		 
//			unsigned short *data = new unsigned short[sizeX*sizeY];
			unsigned short *data = (unsigned short *)(DataGenPtr->getDataPtr(z_i));
			for(int y_i = 0 ;y_i <sizeY; y_i++){
				for(int x_i = 0 ;x_i <sizeX; x_i++){
					if( (x_i<10) && (y_i<10)){
		//				data[y_i*sizeX + x_i] = data_range *z_i;
					}else{
//						data[y_i*sizeX + x_i] = x_i;
					}
				}
			}
			if(m_EmbeddingCheckPattern){
	//			dicomImage.embedTestPattern(m_CurSeriesCount,m_CurTestDataType,z_i+1,(unsigned char*)data,sizeX*sizeY*2);
				dicomImage.embedTestPattern(m_SeriesCount,m_CurTestDataType,z_i+1,(unsigned char*)data,sizeX*sizeY*2);
			}

		
 			dicomImage.setupImage(sizeX,sizeY,z_i,(unsigned char*)data);

 
//			delete [] data;
		}

	// 	dicomImage.saveDicom("dbg_dicom.dcm");


		dicomImage.setImageNumber(z_i +1);

	//	dicomImage.setupSOPInstanceUID();
		genSOPInstanceUID();
		dicomImage.setupSOPInstanceUID(m_curSOPInstanceUID);

 

		if(m_EmbeddingCheckPattern){
			sprintf(_str_buff,dicomImage.getTestDataPatternFromStudyDesString().c_str(),m_CurStudyCount);
		}else{
			if(!m_StudyDescription.empty()){
				sprintf(_str_buff,"%s", m_StudyDescription.c_str());
			}else{
				sprintf(_str_buff,"StudyDes日本語 [%d]",m_CurStudyCount);
			}
		}
		dicomImage.setStudyDescription(_str_buff);


		if(m_EmbeddingCheckPattern){
			sprintf(_str_buff,dicomImage.getTestDataPatternFromSereisDesString().c_str(),
				sizeZ,
	//			m_CurSeriesCount,
				m_SeriesCount,
				m_CurTestDataType);
		}else{
			sprintf(_str_buff,"SeriesDes[%d] Data[%d]",m_CurSeriesCount,m_CurTestDataType);
		}
		
		dicomImage.setSeriesDescription(_str_buff);
		//

		dicomImage.setModality(m_Modality);

	

		if(!m_ReferringPhysician.empty()){
			dicomImage.setReferringPhysician(m_ReferringPhysician.c_str());
		}

		dicomImage.setupCurDate(m_curStudyDate,m_curSeriesTime);


		dicomImage.prepareDICOM();

 // 	dicomImage.saveDicom("dbg_dicom1.dcm");

	// 	::Sleep(10000);
		if(!SendImage(dicomImage_dicom->GetID())){
			error_flag = true;
			break;
		} 
		
		m_ImageCount++;
	}
	unsigned long end_time = ::GetTickCount();
	printf("\n <<< spent time %.2f >>> \n",(end_time - start_time)/1000.0f);

	delete DataGenPtr;

	return (!error_flag);
}
void CTstStoreSCU::sendDataContinue()
{
	int run_i =0;
	bool success_flag = true ;
	while(true){
		if(success_flag){
			run_i++;
		}
		if(m_sendLoopNN>0){
			if(run_i>m_sendLoopNN) break;
		}
 
	//	for(int i=0;i<this->m_sendLoopNN;i++){

		if(!success_flag){
		//途中,送信中断された場合
			printf("try to re-open association ... \n");
			if(openAssociation())
			{
				success_flag = true;
			}else{
			//	TRPlatform::sleep(50);
				::Sleep(1000);
				success_flag = false;
				continue;
			}
		}
		//
		if(m_sourceDataType == 0){
			if(success_flag){
				 beginNewImage() ;
				 
#if 1
				if(!sendStudyData()){
					::Sleep(10000);
					success_flag = false;
					continue;
				}else{
					success_flag = true;
				}
#else
				 success_flag = true;
#endif

			}
			if(!doSeriesCount()){ // and try to re-openAossiation
				::Sleep(3000);
				success_flag = false;
				continue;
			}else{
				success_flag = true;
			}
		}else{
			
			if(!sendStudyDataFromSrc()){ // and try to re-openAossiation
				::Sleep(10000);
				success_flag = false;
				continue;
			}else{
				success_flag = true;
			}
			
		}
	}
}
#include "AqCore/TRPlatform.h"
bool CTstStoreSCU::procStudy(string studyFolder)
{
	if(!m_TstSrcData) return false;

	SrcDataSeriesList src_data_series_list = m_TstSrcData->getSeriesList();

	if(src_data_series_list.size()<1) return true;

	if(!beginNewStudy()){
		return false;
	}

	m_CurSeriesCount = 0;
	 

	SrcDataImageIter ImageIter;
	SrcDataSeriesIter SeriesIter;
	m_spentTimeSum = 0.0f;
	unsigned long start_time = ::GetTickCount();
	printf("send >\n");
	for (SeriesIter = src_data_series_list.begin(); SeriesIter < src_data_series_list.end(); SeriesIter++)
	{
		beginNewSeries();

		int image_size = (*SeriesIter)->size();
		for(ImageIter = (*SeriesIter)->begin();ImageIter<(*SeriesIter)->end();ImageIter++){
			beginNewImage();
			setupUID();
			if(!procDicomImage(*ImageIter)){
				return false;
			}
			printf(".");
		}
		printf(".");
		m_SeriesCount++;
		//Next Series

	}

//	printf("\n <<< spent time %.2f >>> \n",m_spentTimeSum);
	unsigned long end_time = ::GetTickCount();
	printf("\n <<< spent time %.2f >>> \n",(end_time - start_time)/1000.0f);

	//Next Study
	
 
#if 0
	std::vector<TRFileName> SeriesList;
	std::vector<TRFileName>::iterator iter;

	std::string seriesPath;
	int tmpStatus = TRPlatform::iGetDirectoryList(studyFolder.c_str(), "*", SeriesList);
	if (tmpStatus < 0 || SeriesList.size() < 3){
		return false;
	}

	for (iter = SeriesList.begin(); iter < SeriesList.end(); iter++)
	{
		if (!strcmp(iter->GetName(),".") || !strcmp(iter->GetName(), ".."))
			continue;
		seriesPath = studyFolder + std::string("/") + iter->GetName();
		if (!TRPlatform::IsDirectory(seriesPath.c_str()))
			continue;
		if(!procSeries(seriesPath)){
			return false;
		}
	}

#endif

	return true;
}
bool CTstStoreSCU::procSeries(string SeriesFolder)
{

	std::vector<TRFileName> DicomFilList;
	std::vector<TRFileName>::iterator iter;
	std::string DicomFileName;

	int tmpStatus = TRPlatform::iGetDirectoryList(SeriesFolder.c_str(), "*", DicomFilList);
	if (tmpStatus < 0 || DicomFilList.size() < 3){
		return false;
	}

	int imageNum = 0;
	m_spentTimeSum = 0.0f;
	unsigned long start_time = ::GetTickCount();
	printf("send >\n");
	for (iter = DicomFilList.begin(); iter < DicomFilList.end(); iter++)
	{
		if (!strcmp(iter->GetName(),".") || !strcmp(iter->GetName(), ".."))
			continue;
		DicomFileName = SeriesFolder + std::string("/") + iter->GetName();
		if(!procDicomFile(DicomFileName,imageNum++)){
			return false;
		}
		printf(".");
		;;
	}

//	printf("\n <<< spent time %.2f >>> \n",m_spentTimeSum);

	unsigned long end_time = ::GetTickCount();
	printf("\n <<< spent time %.2f >>> \n",(end_time - start_time)/1000.0f);

	return true;
}
bool CTstStoreSCU::procDicomImage(CTstVLIDicomImage *src_image,int ImageNumber)
{
	 
#ifdef USE_NEW_LIB
	CPxDicomImage *dicomImage_dicom = src_image->getDicomImage();
#else
	VLIDicomImage *dicomImage_dicom = src_image->getDicomImage();
#endif
	if(!dicomImage_dicom) return false;

	if(ImageNumber>=0){
		src_image->setImageNumber(ImageNumber);
	}
	src_image->setPatientName(m_CurPatientName);
	src_image->setPatientID(m_CurPatientID);
	src_image->setStudyID(m_CurStudyID);

	src_image->setStudyInstanceUID(m_curStudyInstanceUID);
//	src_image->setSeriesInstanceUID(m_CurSeriesID);
	src_image->setSeriesInstanceUID(m_curSeriesInstanceUID);

 

//	src_image->saveDicom("dbg_dicom.dcm");



//	src_image->setupSOPInstanceUID();
	genSOPInstanceUID();
	src_image->setupSOPInstanceUID(m_curSOPInstanceUID);



	sprintf(_str_buff,"StudyDes日本語 [%d]",m_CurStudyCount);
	src_image->setStudyDescription(_str_buff);

	sprintf(_str_buff,"SeriesDes日本語[%d] Data[%d]",m_CurSeriesCount,m_CurTestDataType);
	src_image->setSeriesDescription(_str_buff);


	if(!m_StudyDescription.empty()){
		src_image->setStudyDescription(m_StudyDescription.c_str());
	}

	if(!m_ReferringPhysician.empty()){
		src_image->setReferringPhysician(m_ReferringPhysician.c_str());
	}
	src_image->prepareDICOM();

	src_image->setupCurDate(m_curStudyDate,m_curSeriesTime);

// 	src_image->saveDicom("dbg_dicom.dcm");
	
	if(!SendImage(dicomImage_dicom->GetID())){
		return  false;
	}
	m_ImageCount++;
	 
	return true;
}
bool CTstStoreSCU::procDicomFile(string DicomFile,int ImageNumber)
{
	CTstVLIDicomImage dicomImage;
	dicomImage.loadDicom(DicomFile.c_str());

#ifdef USE_NEW_LIB
	CPxDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
#else
	VLIDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
#endif
	if(!dicomImage_dicom) return false;

	dicomImage.setImageNumber(ImageNumber);
	dicomImage.setPatientName(m_CurPatientName);
	dicomImage.setPatientID(m_CurPatientID);
	dicomImage.setStudyID(m_CurStudyID);
	dicomImage.setStudyInstanceUID(m_curStudyInstanceUID);
//	dicomImage.setSeriesInstanceUID(m_CurSeriesID);
	dicomImage.setSeriesInstanceUID(m_curSeriesInstanceUID);

 

//	dicomImage.saveDicom("dbg_dicom.dcm");



//	dicomImage.setupSOPInstanceUID();
	genSOPInstanceUID();
	dicomImage.setupSOPInstanceUID(m_curSOPInstanceUID);



	sprintf(_str_buff,"StudyDes日本語 [%d]",m_CurStudyCount);
	dicomImage.setStudyDescription(_str_buff);

	sprintf(_str_buff,"SeriesDes日本語[%d] Data[%d]",m_CurSeriesCount,m_CurTestDataType);
	dicomImage.setSeriesDescription(_str_buff);


	if(!m_StudyDescription.empty()){
		dicomImage.setStudyDescription(m_StudyDescription.c_str());
	}

	if(!m_ReferringPhysician.empty()){
		dicomImage.setReferringPhysician(m_ReferringPhysician.c_str());
	}

	dicomImage.prepareDICOM();

 //	dicomImage.saveDicom("dbg_dicom.dcm");
	
	if(!SendImage(dicomImage_dicom->GetID())){
		return  false;
	}
	m_ImageCount++;
	 
	
	return true;
}
bool CTstStoreSCU::sendStudyDataFromSrc()
{
	return procStudy(m_sourceDataFolder);
}
void CTstStoreSCU::getCurStudyDate()
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
void CTstStoreSCU::getCurSeriesTime()
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

bool CTstStoreSCU::testReadWriteDicom()
{
  int sizeX=m_sizeX;
  int sizeY=m_sizeY;
 

	float data_range = (0x7ff);
 

	{
		printf(" [%d] sizeX: %d, sizeY: %d,  , type: %d \n",
			m_CurSeriesCount,
			sizeX,sizeY,m_CurTestDataType);
	}
	COctDatBase *DataGenPtr = new COctDatBase;
	DataGenPtr->setDim(sizeX,sizeY,1);
	DataGenPtr->genData(DataType_USHORT,m_CurTestDataType);

	bool error_flag =false;
 //
		CTstVLIDicomImage dicomImage;
		dicomImage.openNewDicom();
#ifdef USE_NEW_LIB
		CPxDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
#else
		VLIDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
#endif
		if(!dicomImage_dicom) {
			error_flag = true;
		    return false;
		} 

		 
		dicomImage.setPatientName(m_CurPatientName);
		dicomImage.setPatientID(m_CurPatientID);
		dicomImage.setStudyID(m_CurStudyID);
		dicomImage.setStudyInstanceUID(m_curStudyInstanceUID);

		//dicomImage.setSeriesInstanceUID(m_CurSeriesID);
		dicomImage.setSeriesInstanceUID(m_curSeriesInstanceUID);

		dicomImage.setupPitch(m_pitchX,m_pitchY,m_pitchZ);
		{
		 
//			unsigned short *data = new unsigned short[sizeX*sizeY];
			unsigned short *data = (unsigned short *)(DataGenPtr->getDataPtr(0));
			for(int y_i = 0 ;y_i <sizeY; y_i++){
				for(int x_i = 0 ;x_i <sizeX; x_i++){
					if( (x_i<10) && (y_i<10)){
		//				data[y_i*sizeX + x_i] = data_range *z_i;
					}else{
//						data[y_i*sizeX + x_i] = x_i;
					}
				}
			}
			if(m_EmbeddingCheckPattern){
	//			dicomImage.embedTestPattern(m_CurSeriesCount,m_CurTestDataType,1,(unsigned char*)data,sizeX*sizeY*2);
				dicomImage.embedTestPattern(m_SeriesCount,m_CurTestDataType,1,(unsigned char*)data,sizeX*sizeY*2);
			}

		
 			dicomImage.setupImage(sizeX,sizeY,0,(unsigned char*)data);

 
//			delete [] data;
		}

	// 	dicomImage.saveDicom("dbg_dicom.dcm");


		dicomImage.setImageNumber(1);

//		dicomImage.setupSOPInstanceUID();
		genSOPInstanceUID();
		dicomImage.setupSOPInstanceUID(m_curSOPInstanceUID);
 

		if(m_EmbeddingCheckPattern){
			sprintf(_str_buff,dicomImage.getTestDataPatternFromStudyDesString().c_str(),m_CurStudyCount);
		}else{
			if(!m_StudyDescription.empty()){
				sprintf(_str_buff,"%s", m_StudyDescription.c_str());
			}else{
				sprintf(_str_buff,"StudyDes日本語 [%d]",m_CurStudyCount);
			}
		}
		dicomImage.setStudyDescription(_str_buff);


		if(m_EmbeddingCheckPattern){
			sprintf(_str_buff,dicomImage.getTestDataPatternFromSereisDesString().c_str(),
				1,
//				m_CurSeriesCount,
				m_SeriesCount,
				m_CurTestDataType);
		}else{
			sprintf(_str_buff,"SeriesDes[%d] Data[%d]",m_CurSeriesCount,m_CurTestDataType);
		}
		
		dicomImage.setSeriesDescription(_str_buff);
		//

		dicomImage.setModality(m_Modality);

	

		if(!m_ReferringPhysician.empty()){
			dicomImage.setReferringPhysician(m_ReferringPhysician.c_str());
		}

		dicomImage.setupCurDate(m_curStudyDate,m_curSeriesTime);


		dicomImage.prepareDICOM();

  		dicomImage.saveDicom("dbg_dicom1.dcm");

	 
		m_ImageCount++;
	 

	delete DataGenPtr;

	return (!error_flag);
 
}
bool CTstStoreSCU::closeALL()
{
	MC_STATUS             status;

	if(m_applicationID){
		status = MC_Release_Application(&m_applicationID);
		if (status != MC_NORMAL_COMPLETION)
		{
		  return false;
		}
	}

	return true;
}

void CTstStoreSCU::genStudyInstanceUID()
{
	

	char _char_buff[128];

	if(m_useTimeTickCount){
		unsigned long tick_count = ::GetTickCount();
		sprintf(_char_buff,".%u.%u",tick_count,m_StudyCount+1);
	}else{
		sprintf(_char_buff,".%d",m_StudyCount+1);
	}
	m_curStudyInstanceUID = m_rootUID + "." + m_myTestUID +_char_buff;
}
void CTstStoreSCU::genSeriesInstanceUID()
{
	char _char_buff[128];
	sprintf(_char_buff,".%u",m_SeriesCount+1);
	m_curSeriesInstanceUID = m_curStudyInstanceUID  +_char_buff;
}
void CTstStoreSCU::genSOPInstanceUID()
{
	char _char_buff[128];
	sprintf(_char_buff,".%u",m_ImageCount+1);
	m_curSOPInstanceUID = m_curStudyInstanceUID  +_char_buff;
}

bool CTstStoreSCU::initClient()
{
	MC_Set_Int_Config_Value(RETRY_NUMBER_OPEN_ASSOC,(int)2);

	return true;
}