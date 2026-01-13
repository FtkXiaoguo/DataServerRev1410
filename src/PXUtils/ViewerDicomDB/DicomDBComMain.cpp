#include "stdafx.h"
#include "DicomDBComMain.h"

//#include "SearchFolder.h"

#include "PxNetDB.h"
#include "PxDicomutil.h"
#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "IDcmLibDefUID.h"
#include "AuxData.h"

#include "AqCore\TRPlatform.h"

#if 0
#include "DicomDB.h"
#include "DBCore.h"
#include "AqnetDB.h"
#include "ImptDicomFile.h"
#endif

#include "MyDicomFile.h"
using namespace DicomDBComAPI;

//CDicomDB _dicomDB;
CPxDcmDB  _dicomDB;

static	std::string g_getStudyFolderName(const std::string &topFolder, const std::string &studyUID)
{
	//		return m_mediapoint + std::string("\\") + studyUID;
	return topFolder + std::string("\\") + studyUID;
};

static std::string g_getSeriesFolderName(const std::string &studyFolder, const std::string &seriesUID)
{
	return studyFolder + std::string("\\") + seriesUID;
}

DicomDBComMain::DicomDBComMain()
{
	 
}
DicomDBComMain::~DicomDBComMain()
{
	
}
void DicomDBComMain::destroy(void)
{
	delete this;
}
bool DicomDBComMain::initMain(void)
{
//	CImptDicomFile::initDcmTK();
	if (!TRDICOMUtil::InitialDICOM(
		"PXDcmAPI_License")){
		return false;
	}
	_dicomDB.InitDatabaseInfo();
//	return _dicomDB.init();
	return true;
 
}
void DicomDBComMain::initDBCom(void)
{
 //	CDicomDB::initDBCom();
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // for database to work in multi-thread mode

}

#define MyStrNCpy(dest,src) strncpy(dest,src.c_str(),sizeof(dest))

bool DicomDBComMain::getMediaPointList(char **MediaPointListBuf, int &size)
{
	
	std::vector<MediaPoint>	vData;

	CPxDB::InitMediaPoints();
	vData = CPxDB::GetMediaPoints();

	int num = vData.size();
	if (num<1) return false;
	bool ret_b = false;

	int ret_size = 0;
	for (int i = 0; i<num; i++)
	{
		if (i == 0)
		{
			//			 gHWaterMark = vData[i].m_highWaterMark;
			//			 gLWaterMark = vData[i].m_lowWaterMark;
		}
		// add mounted media only
		MediaPoint mediapPointTemp = vData[i];
		if (mediapPointTemp.m_mediaPoint[0] != 0){
			std::string mediapoint_str = std::string(mediapPointTemp.m_mediaPoint) +
				std::string(mediapPointTemp.m_mediaLabel);

			strcpy(MediaPointListBuf[ret_size++], mediapoint_str.c_str());
			ret_b = true;

			if (ret_size >= size){
				break;
			}
		}
		//		addMediaPoint(vData[i].m_mediaPoint, vData[i].m_mediaType, vData[i].m_mediaLabel, vData[i].m_highWaterMark, vData[i].m_lowWaterMark );
	}
	size = ret_size;
	return ret_b;
}

 
bool DicomDBComMain::getStudyDate(const char *DicomFile, SimpleDicomInfoDef *dcmInfo)
{
	CMyDicomFile dicomFile;
	SimpleDicomInfo dicomInfoTemp;

	if (kNormalCompletion != dicomFile.loadDicomHeader(DicomFile)){
		return false;
	}
	dicomFile.getSimpleDicomInfo(dicomInfoTemp);

//	CImptDicomFile ImportDicomFile;



//	ImportDicomFile.getStudyDate(DicomFile, &dicomInfoTemp);
	if (dcmInfo){
		MyStrNCpy(dcmInfo->m_patientID, dicomInfoTemp.m_patientID);
		MyStrNCpy(dcmInfo->m_patientName, dicomInfoTemp.m_patientName);
		MyStrNCpy(dcmInfo->m_studyUID, dicomInfoTemp.m_studyUID);
		MyStrNCpy(dcmInfo->m_seriesUID, dicomInfoTemp.m_seriesUID);
		dcmInfo->m_studyDay = dicomInfoTemp.m_studyDay;
		dcmInfo->m_studyMon = dicomInfoTemp.m_studyMon;
		dcmInfo->m_studyYear = dicomInfoTemp.m_studyYear;
	}
	return true;
}
#include "AqCore/TRAtomic.h"
TRCriticalSection	g_threadMapCS;
bool DicomDBComMain::doImportDicomfile(const char *DicomFile)
{
	bool ret_b = true;
	CMyDicomFile dicomFile;
	if (kNormalCompletion != dicomFile.loadDicomHeader(DicomFile)){
		return false;
	}
	DICOMData dicom_data;
	dicomFile.FillSortInfo(dicom_data, false/* bFillPixelInfo */);
	dicom_data.m_pixelOffset = dicomFile.m_pixelOffset;
	dicom_data.m_dataSize = dicomFile.m_pixelSize;

	int try_count = 0;
	while(try_count < 5){
		if ( _dicomDB.SaveDICOMData(dicom_data) != kOK)
		{
			ret_b = false;
			::Sleep(100);

		}
		else{
			break;
		}
	}
	if (!ret_b){
		return false;
	}
	//
	// HandleTerareconSpecific
	//
	AuxData auxData;
	//	Do we have any aux data?
	if (auxData.Init(dicomFile.GetID(), dicomFile.GetStudyInstanceUID(), dicomFile.GetSeriesInstanceUID(), dicomFile.GetSOPInstanceUID()) != 0)
	{	
			
	}
	else{
		//
		int dbstat = kDBTimeout;
		int try_count = 0;
		while (try_count < 5){
			if ((dbstat = _dicomDB.SaveAuxDataInfo(auxData.m_auxData, auxData.m_auxReferencs)) != kOK)
			{
				ret_b = false;
				::Sleep(100);

			}
			else{
				break;
			}

		}
	}

	return ret_b;

#if 0
	CImptDicomFile ImportDicomFile;

	ImportDicomFile.resetResource();
	ImportDicomFile.setDB(_dicomDB.getDB());

	if (ImportDicomFile.scanDicomFile(DicomFile, true/*importDB*/, &g_threadMapCS)){

	}
	else{
	}

	return true;
#endif
}

bool DicomDBComMain::findSeriesUIDFromDB(const char *seriesUID)
{

	bool ret_b = false;
	 
	if (1){
		std::string oSeriesPath;
		int status = _dicomDB.GetSeriesStatus(seriesUID);
		switch (status){
		case CPxDB::eCompLeted:
		case CPxDB::eInprogress:
			ret_b = true;
			break;
		default:
			ret_b = false;
			break;
		}

	}
	return ret_b;
}

inline bool g_checkStudyUID(const std::string &topFolder, const std::string &studyUID)
{
	printf("checkStudyUID %s \n", studyUID.c_str());

	std::string study_folder = g_getStudyFolderName(topFolder, studyUID);

	bool ret_b = (TRPlatform::IsDirectory(study_folder.c_str()) >0);

	return ret_b;
}

inline bool g_checkSeriesUID(const std::string &studyFolder, const std::string &seriesUID)
{
	printf("   checkSeriesUID %s, %s \n", studyFolder.c_str(), seriesUID.c_str());


	std::string series_folder = studyFolder + std::string("\\") + seriesUID;

	bool ret_b = (TRPlatform::IsDirectory(series_folder.c_str()) >0);

	return ret_b;
}


//#include "DbConform.h"
static bool _doConformSeries(const char* Top_folder, CDicomDBComSearchFolderIf *callback, const std::string &studyUID, bool &_toCancelFlag)
{
 
	 

	DICOMData filter;
	memset(&filter, 0, sizeof(DICOMData));

	ASTRNCPY(filter.m_studyInstanceUID, studyUID.c_str());
	//	ASTRNCPY(filter.m_seriesInstanceUID, "**");

	std::vector<DICOMSeries>  oVal;
	int status = _dicomDB.GetSeriesList(oVal, &filter);
	int size = oVal.size();
	//	int	GetSeriesList( std::vector<DICOMSeries>& oVal, const DICOMData*  iFilter, bool iSort=false);

	bool cancelFlag = false;
	std::string study_folder = g_getStudyFolderName(Top_folder, studyUID);
	for (int i = 0; i<size; i++){
		if (_toCancelFlag){
			cancelFlag = true;
			break;
		}

		if (strlen(oVal[i].m_studyInstanceUID) < 1){
			continue;
		}

		if (callback){
			if (!callback->procSeries(studyUID.c_str(), oVal[i].m_seriesInstanceUID
				)){
				cancelFlag = true;
				break;
			}
		}
		if (!g_checkSeriesUID(study_folder, oVal[i].m_seriesInstanceUID)){
			if (callback){
				callback->dispInfo((std::string(" removed series: ") + oVal[i].m_seriesInstanceUID).c_str());
			}
			printf(" removed series: %s \n", oVal[i].m_seriesInstanceUID);
			//delete this study
			status = _dicomDB.DeleteSeries(oVal[i].m_seriesInstanceUID);
			if (status != kOK){
				cancelFlag = true;
				break;
			}

		}
		//		printf("delete study [%s] - %s \n",oVal[i].m_patientID,oVal[i].m_studyInstanceUID);
		//		m_db->DeleteStudy( oVal[i].m_studyInstanceUID);
	}

	return !cancelFlag;
 
}
bool DicomDBComMain::doConform(const char* folder, CDicomDBComSearchFolderIf *callback, bool &_toCancelFlag)
{

 

#if 0
	CDbConformProcCallback  *_ProcCallback = (CDbConformProcCallback*)callback;

	AQNetDB *_db = _dicomDB.getDB();

	if (_db == 0) {
		if (_ProcCallback){
			_ProcCallback->procFinished(folder, 1);
		}
		return false;
	}
#endif

	_toCancelFlag = false;



	DICOMData filter;
	memset(&filter, 0, sizeof(DICOMData));

	ASTRNCPY(filter.m_patientsName, "**");

	std::vector<DICOMStudy>  oVal;
	int status = _dicomDB.GetStudyList(oVal, &filter);

	int size = oVal.size();
	;;
	bool cancelFlag = false;
	for (int i = 0; i<size; i++){
		if (_toCancelFlag){
			cancelFlag = true;
			break;
		}
		if (callback){
			if (!callback->procStudy(oVal[i].m_patientsName, oVal[i].m_patientID, oVal[i].m_studyInstanceUID)){
				cancelFlag = true;
				break;
			}
		}
		if (!g_checkStudyUID(folder, oVal[i].m_studyInstanceUID)){
			if (callback){
				callback->dispInfo((std::string(" removed study: ") + oVal[i].m_studyInstanceUID).c_str());
			}
			//delete this study
			status = _dicomDB.DeleteStudy(oVal[i].m_studyInstanceUID);
			if (status != kOK){
				cancelFlag = true;
				break;
			}
		}
		else{
			if (!_doConformSeries(folder, callback, oVal[i].m_studyInstanceUID, _toCancelFlag)){
				cancelFlag = true;
				break;
			}
		}
		//		printf("delete study [%s] - %s \n",oVal[i].m_patientID,oVal[i].m_studyInstanceUID);
		//		m_db->DeleteStudy( oVal[i].m_studyInstanceUID);
	}
	if (cancelFlag){

		//		return false;
	}
	if (callback){
		callback->procFinished(folder, cancelFlag);
	}
	return !cancelFlag;
 
}

void DicomDBComMain::getDBName(char *strBuff, int size)
{
	strncpy(strBuff, "PxDcmDB", size);

}
//
void DicomDBComMain::generatStudyFolder(const char *MediaPointTopFolder, const char *StudyUID, char *strBuff, int size)
{
	std::string studyFolderStr = g_getStudyFolderName(MediaPointTopFolder, StudyUID);
	strncpy(strBuff, studyFolderStr.c_str(), size);
}
void DicomDBComMain::generatSeriesFolder(const char *StudyFolder, const char *SeriesUID, char *strBuff, int size)
{
	std::string seriesFolderStr = g_getSeriesFolderName(StudyFolder, SeriesUID);
	strncpy(strBuff, seriesFolderStr.c_str(), size);
}
void DicomDBComMain::generatDicomFile(const char *SeriesFolder, const char *InstanceUID, int imageNum, char *strBuff, int size)
{
	char _str_buf[128];
	sprintf_s(_str_buf, sizeof(_str_buf), "%05d_", imageNum);
	std::string dicomFileStr = std::string(SeriesFolder) + std::string("\\") + std::string(_str_buf) + std::string(InstanceUID) + std::string(".dcm");

	strncpy(strBuff, dicomFileStr.c_str(), size);
}

