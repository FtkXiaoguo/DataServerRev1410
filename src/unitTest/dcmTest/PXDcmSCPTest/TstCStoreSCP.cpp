// TstStoreSCU.cpp: CTstCStoreSCP クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

#include <ctime>

#include "TstCStoreSCP.h"
#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#include "IDcmLibApi.h "
using namespace XTDcmLib;
#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif

#include "AqCore/trplatform.h"

#include "rtvloadoption.h"
#include "TstVLIDicomImage.h"

 
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
CTstCStoreSCP::CTstCStoreSCP()
{

	m_sendLoopNN = 0;
	m_LoopInterval = 2000;//mSec


	m_calledApplicationID = 0;

	m_MyAE = "MONE_AE";

 


	m_ListenPort = 105;
//

//
 
//
	m_Modality = 0;
	 
//

//
	m_runCount = 0;
 
	m_RequestCount = 0;
 



	m_waitStoreAssociationID = 0;

	m_ReopenAssociation = 1;

	m_checkPixelData = 0;
	m_writeFileFlag = 0;
}

CTstCStoreSCP::~CTstCStoreSCP()
{

}
static char _DicomSvrAE[128]={0,};
static char _DicomSvrHost[512]={0,};
static char _MyAE[512]={0,};

static char _StudyDate[512]={0,};
static char _PatientName[512]={0,};
static char _PatientID[512]={0,};
static char _StudyID[512]={0,};
static char _SeriesID[512]={0,};
static char _CMoveDestAE[512]={0,};
static char _logFileName[512]={0,};
bool CTstCStoreSCP::loadOption(const char *fileName)
{
	iRTVOptions cstore_opt;

	
	cstore_opt.Add("ListenPort",&m_ListenPort , sizeof(m_ListenPort));
	cstore_opt.Add("MyAE",_MyAE , sizeof(_MyAE));
//

	cstore_opt.Add("LogFileName",_logFileName,sizeof(_logFileName));
	
 ;
//

	cstore_opt.Add("SendLoopNN",&m_sendLoopNN,sizeof(m_sendLoopNN));
	cstore_opt.Add("LoopInterval",&m_LoopInterval,sizeof(m_LoopInterval));
 

	cstore_opt.Add("checkPixelData",&m_checkPixelData,sizeof(m_checkPixelData));
	//
	cstore_opt.Add("writeFileFlag",&m_writeFileFlag,sizeof(m_writeFileFlag));

 //
	cstore_opt.Add("Modality",&m_Modality,sizeof(m_Modality));
	
 //

//
	if(!cstore_opt.Load(fileName)){
		return false;
	}
	//


	m_logFileName = _logFileName;
 
	bool none_log_file = false;
	 
	if( m_logFileName.size()>1 )
	{
		none_log_file = (m_logFileName != "null");
	}
	if(!none_log_file){
		char _str_buff[512];
		WORD procID = ::GetCurrentProcessId();
		sprintf(_str_buff,"TstStoreSCP_%x.log",procID);
		string use_logFileName = _str_buff;
		if(!m_logFileName.empty())
		{
			use_logFileName = m_logFileName;
		}
		string full_log_name = "C:\\AQNetLog\\"+use_logFileName;
		m_Logger.SetLogFile(full_log_name.c_str());
	}else{
		string full_log_name;
		if(m_homeFolder.size()){
			full_log_name = m_homeFolder+m_logFileName;
		}else{
			full_log_name = string("C:\\AQNetLog\\")+m_logFileName;
		}
		m_Logger.SetLogFile(full_log_name.c_str());
	}
//	m_Logger.SetLogLevel(kTrace);
	m_Logger.LogMessage("start log ... \n");


	m_DicomSvrAE	= _DicomSvrAE;
	m_DicomSvrHost	= _DicomSvrHost;
	m_MyAE			= _MyAE;
 //

	return true;
}
bool CTstCStoreSCP::initCStoreSCP()
{
	MC_STATUS mcStatus;



	if(m_calledApplicationID){
		mcStatus = MC_Release_Application(&m_calledApplicationID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
		  ;
		}
	}

 	MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, 20);
	MC_Set_Int_Config_Value(CONNECT_TIMEOUT, 20);
 	MC_Set_Int_Config_Value(WRITE_TIMEOUT, 20);
 	MC_Set_Int_Config_Value(RELEASE_TIMEOUT, 20);
 	MC_Set_Int_Config_Value(INACTIVITY_TIMEOUT, 20);	

	/*
     *  Register this DICOM application
     */
	mcStatus = MC_Register_Application(&m_calledApplicationID, m_MyAE.c_str());

	if (mcStatus != MC_NORMAL_COMPLETION)
    {
        
        return false;
    }

 	mcStatus = MC_Set_Int_Config_Value( TCPIP_LISTEN_PORT,m_ListenPort );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return false;
    }

		 
	char start_title[256];
	
	sprintf(start_title,"#### C-Store SCP AE:%s, port: %d \n",m_MyAE.c_str(),m_ListenPort);
//	printf("%s",start_title);
	m_Logger.LogMessage(start_title);


	string check_pixeldata_str ;
	if(m_checkPixelData){
		check_pixeldata_str = "###  check Pixel data mode ### \n";
	}else{
		check_pixeldata_str = "###  none check Pixel data mode ### \n";
	}
//	printf("%s",check_pixeldata_str.c_str());
	m_Logger.LogMessage(check_pixeldata_str.c_str());
	m_Logger.FlushLog();
 
	string write_file_flag_str ;
	if(m_writeFileFlag == 1){
		write_file_flag_str = "###  write dicom file ### \n";
	}else{
		write_file_flag_str = "###  not write dicom file ### \n";
	}
	m_Logger.LogMessage(write_file_flag_str.c_str());
	m_Logger.FlushLog();
	return true;
}





void CTstCStoreSCP::receiveDataContinue()
{
 
	int run_i =0;
	m_runFlag = true;
	while(m_runFlag){
		run_i++;
		if(m_sendLoopNN>0){
			if(run_i>m_sendLoopNN) break;
		}
  
		if(WaitCSTOREAssociation(m_runFlag)){

			ProcessCSTOREAssociation();
		//	::Sleep(100000);
		}else{
			//check memory leak;
#ifdef USE_NEW_LIB
			bool dbg_chk_flag = false;
			if(dbg_chk_flag){
				IDcmLibApi::CheckMemory(1);
			}
#endif
		}
 		cleanup();


//		::Sleep(m_LoopInterval);
	 
	}

	CleanStopped();
}

void CTstCStoreSCP::getCurStudyDate()
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
void CTstCStoreSCP::getCurSeriesTime()
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



bool CTstCStoreSCP::WaitCSTOREAssociation(bool &runFlag)
{
	MC_STATUS             status;

	bool toDoAssociation = false;
	for(int run_i = 0;run_i < 10;run_i++){
		runFlag = false;
		toDoAssociation = false;
		int             this_calledApplicationID;
		status = MC_Wait_For_Association ( "AqNETDental_Service_List",
						5,//1,//5,//-1,//5,
						&this_calledApplicationID,
						&m_waitStoreAssociationID );
		//
		CleanStopped();
		switch ( status )
		{
			case MC_NORMAL_COMPLETION:
				{
						AssocInfo   asscInfo;
	        
					printf ( " Accept_Association ########################################## \n" );

	     
					status = MC_Get_Association_Info( m_waitStoreAssociationID, &asscInfo); 
					if (status != MC_NORMAL_COMPLETION)
					{
						printf("MC_Get_Association_Info failed", status);
					}
					else
					{
	       
						{
							printf("Connection from Remote Application:\n");
							printf("   AE Title:                 %s\n", asscInfo.RemoteApplicationTitle);
							printf("   Host name:                %s\n", asscInfo.RemoteHostName);
							printf("   IP Address:               %s\n", asscInfo.RemoteIPAddress);
							printf("   Implementation Version:   %s\n", asscInfo.RemoteImplementationVersion);
							printf("   Implementation Class UID: %s\n", asscInfo.RemoteImplementationClassUID);
							printf("   Requested AE Title:       %s\n\n\n", asscInfo.LocalApplicationTitle);
						}
					}

					bool debug_flag=false;
					int val_temp = rand() ;
					if( (val_temp % 5) == 0 ){
			//			debug_flag = true;
					}
					if(debug_flag){
						status = MC_Reject_Association(m_waitStoreAssociationID,TRANSIENT_NO_REASON_GIVEN);
					}else{
						status = MC_Accept_Association ( m_waitStoreAssociationID );
					}
					if ( status != MC_NORMAL_COMPLETION )
					{
						printf("MC_Accept_Association  error %d \n",status );
						return ( false );
					}
					runFlag = true;
					toDoAssociation = true;
				}
				break;

			case MC_NEGOTIATION_ABORTED:
				printf("Association aborted during negotiation, continuing\n");

				runFlag = true;
				toDoAssociation = false;
				break;
	            
			case MC_TIMEOUT:
					
				runFlag = true;
				toDoAssociation = false;
				break;
			default:
				 printf( "MC_Wait_For_Association  error  %d \n",status);
				runFlag = false;
				toDoAssociation = false;

	           
		} /* switch status of MC_Wait_For_Association for store association */

		if(toDoAssociation){
			if (this_calledApplicationID != m_calledApplicationID){
				runFlag = true;
				toDoAssociation = false;
			}
		}

		if(!runFlag) break;
		if(toDoAssociation) break;
	}
	return toDoAssociation;
}

bool CTstCStoreSCP::ProcessCSTOREAssociation()
{
	// not use it here!

//	m_Logger.LogMessage(" create new AssociationHandler\n");
//	m_Logger.FlushLog();

	//別スレッド起動
	printf(" new CAssociationHandler \n");
	CAssociationHandler* pAssociationHandler = new CAssociationHandler(m_waitStoreAssociationID,&m_Logger);

 
	pAssociationHandler->setCheckPixelData(m_checkPixelData);
	//
	pAssociationHandler->setWriteFile(m_writeFileFlag!=0);
	pAssociationHandler->setupHomeFolder(m_homeFolder);
	///
	pAssociationHandler->setProImgFlag((m_checkPixelData != 0) || (m_writeFileFlag!=0));
	
	Handover(pAssociationHandler);

	return true;
}


void CTstCStoreSCP::cleanup()
{
	
	CleanStopped();
	printf(" ## CAssociationHandler count %d \n",m_threadMap.size());

}
bool CTstCStoreSCP::closeAll()
{
	waitAll();

	StopAll();

	MC_STATUS             status;

	if(m_calledApplicationID){
		status = MC_Release_Application(&m_calledApplicationID);
		if (status != MC_NORMAL_COMPLETION)
		{
		  ;
		}
	}
	return true;
}

bool CTstCStoreSCP::waitAll()
{
	iRTVThread* pthread = NULL;
	ThreadsMap::iterator iter;
	TRCSLock fplock(&m_threadMapCS);
	CAssociationHandler* process;
	int closed = 0;
	int stat;

	bool all_closed = false;
	while(!all_closed){
		all_closed = true;
		for (iter=m_threadMap.begin(); iter != m_threadMap.end(); )
		{
			try
			{
				process =(iter->first);
				stat = process->GetProcessStatus();
			}
			catch(...)
			{
				assert(0);
				iter = m_threadMap.erase(iter);
				continue;
			}

			if (stat != iRTVThreadProcess::kProcessTerminated)
			{
				 all_closed  = false;
				 break;
			}
			iter++;
			 
		}
		if(!all_closed){
			::Sleep(500);
		}
	}

	return true;
}
 