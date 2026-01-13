// ImptDicomFile.cpp: CImptDicomFile クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#include "AqCore/TRLogger.h"

#include "ImptDicomFile.h"

#include "VLIDicomImage.h"
#include "MyDicomFile.h"

#include "AQNetDB.h"
#include "AuxData.h"

#include "rtvMergeToolKit.h "

#include "libmtime.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

#ifdef MakeDicomDBComAPI
#include "DicomDBComIf.h"
extern DicomDBComAPI::DicomDBComLogger *u_gLogger ;
#define U_logger u_gLogger
#else
extern TRLogger *p_gLogger ;
#define U_logger p_gLogger
#endif

CImptDicomFile::CImptDicomFile()
{
#ifdef USE_LOCAL_DICOMMESSAGE
	m_pImage = new  CMyDicomFile;
#else
 
 m_pImage = new VLIDicomImage;
#endif
 //
 m_db = 0 ;
m_dbData = new DICOMData;
resetTimeCounter();
 
}

CImptDicomFile::~CImptDicomFile()
{
delete m_pImage;
 
delete m_dbData;
}

void CImptDicomFile::resetResource()
{
	 ;
}
bool CImptDicomFile::initDcmTK()
{
	if(!TRDICOMUtil::InitialDICOM("DBA6-5B453") )
		return false;

	/*
	*  VLIDicomMessage::LoadHeader
	* MC_Register_Application(&msgApplicationID, "VLIDicomMessage");
	*  for multi thread !
	*/
 
#if 0

#if 0
 	VLIDicomMessage dicom_temp;
		
 	int status = dicom_temp.Load("temp.dcm",1 /*iHeaderOnly*/);
 
	if (status != kNormalCompletion){
		printf(" dicom_temp.Load error \n");
	}
#else

	CMyDicomFile dicom_temp;
	int status = dicom_temp.loadDicomHeader("temp.dcm");
	if (status != kNormalCompletion){
		printf(" dicom_temp.Load error \n");
	}

#endif

#else
	CMyDicomFile dicom_temp;
	bool ret_b = dicom_temp.initRegisterApp();
	if(!ret_b){
		printf( " initRegisterApp error \n");
	}

#endif
	 
	return true;
}

bool CImptDicomFile::scanDicomFile(const std::string &fileName, bool importDB,TRCriticalSection *cs)
{
//	p_gLogger->LogMessage(kDebug,"read Dicomfile \n");
//	p_gLogger->FlushLog();

	MTIME tm1 ;

	MTIME tm2;
	m_fileName = fileName;

	tm1.update();

 
#ifdef USE_LOCAL_DICOMMESSAGE
	int status = m_pImage->loadDicomHeader(fileName.c_str());
#else
	int status = m_pImage->Load(fileName.c_str(),1 /*iHeaderOnly*/);

#endif

	if(status != kNormalCompletion ){
//		p_gLogger->LogMessage(kErrorOnly,"load DICOM file  error %s \n",fileName.c_str() );
//		p_gLogger->FlushLog();
		return false;
	}

	tm2.update();
	int read_time = elapse( tm1, tm2);
	m_readDicomTimeSum += read_time;

 

	if(importDB){
		if (cs){
			cs->Enter();
		}
		if(!doImportDB()){
			if (U_logger){
				U_logger->LogMessage(kErrorOnly, "doImportDB error  \n");
				U_logger->FlushLog();
			}
		}
		if (cs){
			cs->Leave();
		}
	}

	tm1.update();
	int saveDB_time = elapse( tm2, tm1);
	m_saveDBTimeSum += saveDB_time;
	
	m_runCount += 1.0;


	int int_count = m_runCount;
	if( (int_count) %50 == 0){
		reportTime();
	//	if(m_runCount > 1000){
			resetTimeCounter();
	//	}

	}

	return true;
}
bool CImptDicomFile::doImportDB()
{
	if(!CoerceSOPInstanceUID()){
	
		return true;
	}
	//
	if(!theProcess()){
		return false;
	}

	return true;
}
void CImptDicomFile::resetTimeCounter()
{
	m_readDicomTimeSum = 0.0;
	m_saveDBTimeSum = 0.0;
	m_runCount =0.0;
}
void CImptDicomFile::reportTime()
{
	if(m_runCount <1.0) return;
	double read_time = m_readDicomTimeSum/m_runCount;
	double write_time = m_saveDBTimeSum/m_runCount;

//	p_gLogger->LogMessage(kDebug,"readDICOM time %08.2f , writDB time %010.2f mSec/[%.0f]\n",read_time,write_time,m_runCount);
//	p_gLogger->FlushLog();

}
bool CImptDicomFile::theProcess()// see CStore
{
	m_pImage->FillSortInfo(*m_dbData,false/* bFillPixelInfo */);

	/*
	*  2011/04/05 再登録したパノラマーが表示できない
	*  m_pixelOffset, m_dataSize  0 となっている。
	*/
	m_dbData->m_pixelOffset = m_pImage->m_pixelOffset;
	m_dbData->m_dataSize = m_pImage->m_pixelSize;
 

//	p_gLogger->LogMessage(kDebug,"SaveDICOMData \n");
//	p_gLogger->FlushLog();
	int dbstat = kDBTimeout;
	

	int try_count = 0;
	while(try_count < 5){
		if ((dbstat =  m_db->SaveDICOMData(*m_dbData) ) != kOK)
		{
			if (U_logger){
				U_logger->LogMessage("SaveDICOMData re_try %d \n", try_count);
				U_logger->FlushLog();
			}
			::Sleep(100);
				 
		}else{
			break;
		}
	}
	if(dbstat != kOK){
		if (U_logger){
			U_logger->LogMessage(kErrorOnly, "Save DICOMData to DB error  \n");
			U_logger->LogMessage(kErrorOnly, "   --  m_patientID %s \n", m_dbData->m_patientID);
			U_logger->LogMessage(kErrorOnly, "   --  m_studyDate %s %s \n", m_dbData->m_studyDate, m_dbData->m_studyTime);
			U_logger->FlushLog();
		}
		return false;
	}

	if(!HandleTerareconSpecific()){
		return false;
	}
	return true;
}
bool CImptDicomFile::CoerceSOPInstanceUID()
{
	const char* studyUID = m_pImage->GetStudyInstanceUID();
	const char* seriesUID= m_pImage->GetSeriesInstanceUID() ;
	const char* instanceUID = m_pImage->GetSOPInstanceUID();

	bool old_data_flag = m_db->HasThisInstance(instanceUID,seriesUID);
	char *data_flag = old_data_flag? "exist data":"new data";
	//	Transaction Log
//	p_gLogger->LogMessage(kDebug,"TRANS: CoerceSOPInstanceUID() - Attempting to store SOPInstanceUID = %s [%s]\n",instanceUID,data_flag);
//	p_gLogger->FlushLog();

	if(old_data_flag){
//		p_gLogger->LogMessage(kTrace,"HasThisInstance %s - ignored   \n",instanceUID );
//		p_gLogger->FlushLog();
		return false;
	}

	return true;
}
bool CImptDicomFile::HandleTerareconSpecific()
{
	AuxData auxData;
	//	Do we have any aux data?
	if (auxData.Init(m_pImage->GetID(), m_pImage->GetStudyInstanceUID(), m_pImage->GetSeriesInstanceUID(), m_pImage->GetSOPInstanceUID()) != 0)
	{	
		return true;	
	}
	//
	int dbstat = kDBTimeout;
	int try_count = 0;
	while(try_count < 5){
		if ((dbstat = m_db->SaveAuxDataInfo(auxData.m_auxData, auxData.m_auxReferencs)) != kOK)
		{
			if (U_logger){
				U_logger->LogMessage(kErrorOnly, "SaveDICOMData re_try %d \n", try_count);
				U_logger->FlushLog();
			}
			::Sleep(100);
				 
		}else{
			break;
		}

		if(dbstat != kOK){
			if (U_logger){
				U_logger->LogMessage(kErrorOnly, "SaveAuxDataInfo to DB error  \n");
				U_logger->LogMessage(kErrorOnly, "   --  m_patientID %s \n", m_dbData->m_patientID);
				U_logger->LogMessage(kErrorOnly, "   --  m_studyDate %s %s \n", m_dbData->m_studyDate, m_dbData->m_studyTime);
				U_logger->FlushLog();
			}
			return false;
		}
	}

	return true;
}

bool CImptDicomFile::getStudyDate(const std::string &DicomFile,SimpleDicomInfo *dcmInfo)
{
	if(!dcmInfo){
		return false;
	}
	int status = m_pImage->loadDicomHeader(DicomFile.c_str());
 
	if(status != kNormalCompletion ){
//		p_gLogger->LogMessage(kErrorOnly,"load DICOM file  error %s \n",fileName.c_str() );
//		p_gLogger->FlushLog();
		return false;
	}
 
//	SimpleDicomInfo  dicomInfo;
	m_pImage->getSimpleDicomInfo(*dcmInfo);
//	year	= dicomInfo.m_studyYear;
//	month	= dicomInfo.m_studyMon;
//	day		= dicomInfo.m_studyDay;
 
	return true;
}