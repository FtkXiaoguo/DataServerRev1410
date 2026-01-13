// TstStoreSCU.cpp: CTstQRSCUApp クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

#include <ctime>

#include "TstQRSCUApp.h"

#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#include "IDcmLibApi.h "
using namespace XTDcmLib;
#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif

#include "AqCore/trplatform.h"

#include "TstVLIDicomImage.h"

#include "rtvloadoption.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
extern ServiceInfo    m_servInfo;
extern AssocInfo      m_asscInfo;
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
CTstQRSCUApp::CTstQRSCUApp()
{
	//
	m_beforeTime = 0;
	m_timeSpan = 0;

}

CTstQRSCUApp::~CTstQRSCUApp()
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

bool CTstQRSCUApp::loadOption(const char *fileName)
{
	iRTVOptions cstore_opt;

	
	cstore_opt.Add("DcmServerAE",_DicomSvrAE ,		sizeof(_DicomSvrAE));
	cstore_opt.Add("DcmServerHost",_DicomSvrHost , sizeof(_DicomSvrHost));
	cstore_opt.Add("DcmServerPort",&m_DicomSvrPort , sizeof(m_DicomSvrPort));
	cstore_opt.Add("MyAE",_MyAE , sizeof(_MyAE));
//
	cstore_opt.Add("PatientName",_PatientName , sizeof(_PatientName));
	cstore_opt.Add("PatientID",_PatientID , sizeof(_PatientID));
	cstore_opt.Add("StudyID",_StudyID , sizeof(_StudyID));
	cstore_opt.Add("SeriesID",_SeriesID , sizeof(_SeriesID));
	cstore_opt.Add("StudyDate",_StudyDate,sizeof(_StudyDate));
//
 ;
//

	cstore_opt.Add("SendLoopNN",&m_sendLoopNN,sizeof(m_sendLoopNN));
	cstore_opt.Add("LoopInterval",&m_LoopInterval,sizeof(m_LoopInterval));
 

//
	cstore_opt.Add("UseCMoveFlag",&m_sendCMoveCmd, sizeof(m_sendCMoveCmd));
	cstore_opt.Add("CMoveDestAE",_CMoveDestAE,sizeof(_CMoveDestAE));
 //
	cstore_opt.Add("Modality",&m_Modality,sizeof(m_Modality));
	
 //
	cstore_opt.Add("BeforeTime",&m_beforeTime,sizeof(m_beforeTime));
	cstore_opt.Add("TimeSpan",&m_timeSpan,sizeof(m_timeSpan));
 

//
	if(!cstore_opt.Load(fileName)){
		return false;
	}

	m_DicomSvrAE	= _DicomSvrAE;
	m_DicomSvrHost	= _DicomSvrHost;
	m_MyAE			= _MyAE;
 //
	m_StudyDate   = _StudyDate;
	m_PatientName = _PatientName;
//	m_PatientID	= _PatientID+m_myTestUID;
	m_PatientID	= _PatientID;
	m_StudyID	= _StudyID;
	m_SeriesID  = _SeriesID;
//
	m_CMoveDestAE = _CMoveDestAE;
	
	return true;
}


bool CTstQRSCUApp::closeAll()
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

///
// 検索のStudyDate,StudyTimeを更新する
//
void CTstQRSCUApp::updateStudyData()
{
	if(m_timeSpan == 0) return;
	time_t now_time = time(0);

	time_t from_time	= now_time -m_beforeTime*60;
	time_t to_time		= from_time +m_timeSpan*60;

	struct tm tm_from = *localtime(&from_time);
	struct tm tm_to = *localtime(&to_time);

	char _char_buff[128];
	//make StudyDate
	sprintf(_char_buff, "%04d%02d%02d-%04d%02d%02d",
		tm_from.tm_year+1900	,tm_from.tm_mon+1	,tm_from.tm_mday,
		tm_to.tm_year+1900		,tm_to.tm_mon+1		,tm_to.tm_mday );
	m_StudyDate = _char_buff;

	//make StudyTime
	sprintf(_char_buff, "%02d%02d%02d-%02d%02d%02d",
		tm_from.tm_hour	,tm_from.tm_min	,tm_from.tm_sec,
		tm_to.tm_hour	,tm_to.tm_min	,tm_to.tm_sec );
	m_StudyTime = _char_buff;

}