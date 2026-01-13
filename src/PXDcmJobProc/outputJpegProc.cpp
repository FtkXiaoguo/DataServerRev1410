/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "outputJpegProc.h"
//#include "CStoreSCU.h"

#include <assert.h>
#include <sys/timeb.h>
#include "rtvpoolaccess.h"
#include "AppComUtil.h"
//#include "TRDICOMUtil.h"
#include "Globals.h"

#include "PxQueueWatchProc.h"
#include <time.h>

//#include "StoreSCUSeriesDirMonitor.h"

#include "rtvsutil.h"

#include "AqCore/TRPlatform.h"
//////////

 
//#include "PxDicomMessage.h"
#include "PxDicomImage.h"

#include "Conversion.h"
 
#include "DicomImageDataProc.h"
 
#include "HistogramData.h"

//#97 2017/07/10 N.Furutsuki
#include "PxFolderLock.h"
////////////

#ifdef _PROFILE
#include "ScopeTimer.h"
#endif
 
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
#include <memory>
 
///////////////////////////////////////////////

inline const char* GetNextLine(const char* iBytes, char* buf, int bufLen )
{
	if (!iBytes) 
	{
		*buf = '\0';
		return 0;
	}

	for (int i = 0; *iBytes && (*iBytes != '\n' && *iBytes != '\r') && i < bufLen; ++i)
	{
		*buf++ = *iBytes++;
	}

	*buf = '\0';
	return *iBytes ? iBytes+1 : 0;
} 
//#include <ctype.h>
 inline char* DeSpaceDe(char *s, int replaceInPlace=1)
{
	if(*s == 0)
		return s;
	char * p = s + strlen(s) -1, *sh = s;
	

	for (; *s && p >= s && iswspace(*p); --p )
		;
	*++p = '\0';
	
	for ( ; *s && s < p && iswspace(*s); ++s)
		;
	return replaceInPlace ? strcpy(sh,s):s;
}

//--------------------------------------------------------------------------------
inline int Split(const char *buf, char *name, char *value, int n, int seperator)
{
	char *sep;
	int str_len = strlen(buf);
	if(str_len<1) return 0;

	char *_str_buf = new char[str_len+1];
	strcpy(_str_buf,buf);
	if (!(sep = strchr(_str_buf,seperator))){
		delete [] _str_buf;
		return 0;
	}

	*sep = '\0';
	snprintf(name, n, "%s", _str_buf);
	snprintf(value, n, "%s", sep+1);
	name[ n - 1] = value[n - 1] = '\0';
	DeSpaceDe(name);
	DeSpaceDe(value);

	delete [] _str_buf;
	return 1;
}

inline int getImageNumber(const char *lineBuff)
{

	if(lineBuff==0){
		return -1;
	}

	int ret_i = -1;
	AqString str_temp = lineBuff;

	if(str_temp.GetLength()<3){
		return -1;
	}

	int pos1 = str_temp.Find("[");
	if(pos1<0) {
		return -1;
	}
	
	int pos2 = str_temp.Find("]");
	if(pos2<0) {
		return -1;
	}

	int len = pos2-pos1-1;
	if(len<1){
		return -1;
	}
 
	str_temp = str_temp.Mid(pos1+1,len);
	str_temp = str_temp.TrimLeft();str_temp = str_temp.TrimRight();
	if(	(str_temp[0]<='9') && (str_temp[0]>='0')){
		ret_i = atoi(str_temp);
	}

	return ret_i;

}

//#71 Chen {[(
//////////////////////////////////////////////////////////////////////////////////////
//JpegTagWriter

JpegTagWriter::JpegTagWriter(std::string strModualPath)
{
	m_ActionGateForE2 = true;//#73 2013/11/26 K.Ko

	if (strModualPath.empty()){
		m_strYTWexe = "C:\\Actiongate\\YTW29.exe";
//		m_strOutputFolder = "C:\\actionGATE\\DEMO\\Data";
	}else{
		m_strYTWexe = strModualPath;
//		m_strOutputFolder = "C:\\actionGATE\\DEMO\\Data";
	}
	int index = m_strYTWexe.find_last_of("\\");
	if (index < 0) index = m_strYTWexe.find_last_of("/");
	if (index > 0) 	m_strWorkingFolder = m_strYTWexe.substr(0,index);
}
JpegTagWriter::~JpegTagWriter()
{
}

bool JpegTagWriter::IsModualExisted(std::string strModualPath)
{
	return (access(strModualPath.c_str(), 0) == 0);
}

bool JpegTagWriter::AddUserTag(const UserTag& tag, const std::string& strSrcJpg, std::string& strDstjpg, std::string& strCmd)
{
	bool bRtn = false;
	std::string strDst = m_strOutputFolder + "\\" + tag.strPatientID + "\\FCE2" + strSrcJpg.substr(strSrcJpg.find_last_of("\\"), strSrcJpg.size()-1);
	::DeleteFile(strDst.c_str());
	char szImageType[256] = "";
	//std::string strDataTime("20000000000000");
	std::string strDataTime(tag.strDataTime);
	if (strDataTime.size() > 14) strDataTime.erase(strDataTime.begin()+14, strDataTime.end());
	else{
		while(strDataTime.size()<14) strDataTime += "0";
	}
	std::string strPatientID = tag.strPatientID;
	if (strPatientID.size() != 9){
		gLogger.LogMessage(kWarning,"patient id = %s, size must be 9\n", strPatientID.c_str());
	}
	//while(strPatientID.size() < 9) strPatientID = "0" + strPatientID;
	//assert(strPatientID.size() == 9);
	//if (strDataTime.size() > 14) strDataTime = strDataTime.substr(0, 13);
	//else while()
	
	//if (_access(strDst.c_str(), 0) != 0)
	{
		if(m_ActionGateForE2){//#73 2013/11/26 K.Ko
		strCmd = m_strYTWexe + 
			" -CURRDIR"       + m_strWorkingFolder +
			" -FCE2SAVE"      + strSrcJpg + 
			" -FCE2IAD"       + strDataTime +
			" -FCE2PID"       + tag.strPatientID +
			" -FCE2STID"      + tag.strStudyUID +
			" -FCE2SEID"      + tag.strSeriesUID +
			" -FCE2ITP"       + std::string(itoa(tag.eImageType, szImageType, 10)) +
			" -FCE2SRCPATH"   + tag.strOriginalFolderPath +
			" -FCE2SRCFILE"   + tag.strOriginalFolderName;
		}else{
			//#73 2013/11/26 K.Ko
		strCmd = m_strYTWexe + 
			" -CURRDIR"       + m_strWorkingFolder +
			" -FCSAVE"      + strSrcJpg + 
			" -FCIAD"       + strDataTime +
			" -FCPID"       + tag.strPatientID +
			" -FCSTID"      + tag.strStudyUID +
			" -FCSEID"      + tag.strSeriesUID +
			" -FCITP"       + std::string(itoa(tag.eImageType, szImageType, 10)) +
			" -FCSRCPATH"   + tag.strOriginalFolderPath +
			" -FCSRCFILE"   + tag.strOriginalFolderName;
		}

		//strCmd = "C:\\Actiongate\\YTW29.exe -CURRDIRC:\\Actiongate -FCE2SAVEC:\\00000\\write-exif.jpg -FCE2PID000000002 -FCE2IAD20130520130220 -FCE2STID2.16.840.1.113669.632.21.23688796.715862132.858993553.7100739801 -FCE2SEID2.16.840.1.113669.632.21.23688796.715862132.858993553.3109477706 -FCE2ITP1";
		static CriticalSection s_CS;
		s_CS.Enter();		
		int nRtn = system(strCmd.c_str());
		s_CS.Leave();

		bRtn = (0 == nRtn)?true:false;
	}


	return bRtn;
}


////////////////
//#71 Chen )]}

//-----------------------------------------------------------------------------
COutputJpegProc::COutputJpegProc ()
{
	m_cancelFlag = false;//#82 2014/09/29 K.Ko
	 m_cs = new TRCriticalSection ;
	 m_pDcmDB = new CPxDcmDB;
	 //
 
	//---------
	 //ImgType定義
	 m_TypeTable[CDeltaViewInfoEntry::ImageType_O] = "O";
	 m_TypeTable[CDeltaViewInfoEntry::ImageType_D] = "D";
//	 m_TypeTable[CDeltaViewInfoEntry::ImageType_P] = "P";
	 m_TypeTable[CDeltaViewInfoEntry::ImageType_V] = "V";
	 ////
	 m_TypeTable[CDeltaViewInfoEntry::ImageType_3D] = "3D";	//CT
	 m_TypeTable[CDeltaViewInfoEntry::ImageType_P]	= "P";	//Panorama
	 m_TypeTable[CDeltaViewInfoEntry::ImageType_C]	= "C";	//Cephalo
 
	 //---------
	 //ImgIDの頭文字
	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_O] = "C";
	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_D] = "D";
//	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_P] = "P";
	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_V] = "S";
	 ////
	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_3D] = "CT";	//CT
	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_P]	= "P";	//Panorama
	 m_TypeFirstCharTable[CDeltaViewInfoEntry::ImageType_C]	= "C";	//Cephalo

	 //---------
	 //ImgExt
	 m_ExtTable[CDeltaViewInfoEntry::ImageType_O] = "SC";
	 m_ExtTable[CDeltaViewInfoEntry::ImageType_D] = "CRIO";
//	 m_ExtTable[CDeltaViewInfoEntry::ImageType_P] = "PANO";
	 m_ExtTable[CDeltaViewInfoEntry::ImageType_V] = "STV";
	 ////
	 m_ExtTable[CDeltaViewInfoEntry::ImageType_3D] = "dcm";	//CT
	 m_ExtTable[CDeltaViewInfoEntry::ImageType_P]	= "dcm";	//Panorama
	 m_ExtTable[CDeltaViewInfoEntry::ImageType_C]	= "dcm";	//Cephalo
	 
}

//-----------------------------------------------------------------------------
COutputJpegProc::~COutputJpegProc()
{
	delete m_cs;
	delete m_pDcmDB;
 
}
 


bool COutputJpegProc::sendStudy(const CPxQueueEntry *entry)
{
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;

	gLogger.LogMessage(kDebug,"COutputJpegProc::sendStudy to [%s], [%s]\n", AE.c_str(),StudyUID.c_str());
	gLogger.FlushLog();

	std::vector<DICOMSeries>  series_list;

	if(!querySeries( StudyUID,  series_list)){
		gLogger.LogMessage(kDebug,"COutputJpegProc::sendStudy querySeries is failed\n");
		gLogger.FlushLog();
	}
	int series_size = series_list.size();
	if(series_size<1){
		gLogger.LogMessage(kDebug,"COutputJpegProc::sendStudy series_list is null\n");
		gLogger.FlushLog();
		return false;
	}
	bool ret_b= true;

	CPxQueueEntry entry_temp = *entry;
	for(int run_i=0;run_i<series_size;run_i++){
		if(m_cancelFlag){//#82 2014/09/29 K.Ko
			gLogger.LogMessage("CSendDicomProc::sendSeries  is canceled\n");
			gLogger.FlushLog();
			break;
		} 
		DICOMSeries item = series_list[run_i];
		std::string SeriesUID = item.m_seriesInstanceUID;
		entry_temp.m_SeriesInstanceUID = SeriesUID;
	//	if(!sendSeries(AE,StudyUID, SeriesUID,run_i==0/*getStudyInfo*/)){
		if(!sendSeries(&entry_temp,run_i==0/*getStudyInfo*/)){
			gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendStudy series_list[%d] is failed\n",run_i);
			gLogger.FlushLog();
			ret_b = false;
			break;
		}
	}
	return ret_b;
}
//bool COutputJpegProc::sendSeries(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,bool getStudyInfo)

bool COutputJpegProc::sendSeries(const CPxQueueEntry *entry,bool getStudyInfo)
{
	TRCSLock fplock(m_cs);

	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;
	const std::string SeriesUID	= entry->m_SeriesInstanceUID;



	gLogger.LogMessage(kDebug,"COutputJpegProc::sendSeries to [%s], [%s] [%s]\n", AE.c_str(),StudyUID.c_str(),SeriesUID.c_str());
	gLogger.FlushLog();

	bool ret_b = true;

 
	try{
	//
		std::vector<AppComDevice> raids;
		AppComConfiguration::GetArchiveDevices(AppComConfiguration::gkRAIDType,raids);

	//	std::string series_folder = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom(SeriesUID, StudyUID, 0);
		std::string SOPInstanceUID ;
		int imageNumber;
		std::string series_folder = AppComUtil::getSeriesFolder(StudyUID, SeriesUID,"*.dcm");
		if(series_folder.size()<1){
			gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendSeries getSeriesFolder error [%s] [%s]\n",
				StudyUID.c_str(),SeriesUID.c_str());
			gLogger.FlushLog();
			throw(-1);//return false;
		}
 
	 
		////////////
 
//		pCStoreSCU_hdr->setDicomStudyInfo(entry->m_extInfo.m_PatientName,entry->m_extInfo.m_PatientID,entry->m_extInfo.m_BirthDate);
 
	 
		
		std::vector<DICOMInstance>  image_list;
		queryImages(StudyUID,SeriesUID,image_list);

		std::string dicom_file_name;

		int image_size = image_list.size();
		if(image_size<1){
			gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendSeries image_list is null\n");
			gLogger.FlushLog();
			throw(-1);//return false;
		}
		
	 	if(image_size>1){
	 
		//	if(image_size<512){
			//#103 2019/08/23 N.Furutsuki
			if (image_size<gConfig.m_checkCTImageSize){
				gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendSeries writeOutDICOM  is ignored. slice number [%d]\n",image_size);
				gLogger.FlushLog();
				throw(1);//skip it
			}
			//CT thumbnail
		//	DICOMInstance dicom_infor = image_list[image_size/2];

			int ThumbnailNum = gConfig.m_CTDataThumbnailIndex;
			if (ThumbnailNum >= (image_list.size() - 2)){
			//added 2023_0316
				ThumbnailNum = image_list.size() / 2;
			}
			DICOMInstance  dicom_infor = image_list[ThumbnailNum];

			dicom_file_name = AppComUtil::getDicomFileName(series_folder,
											dicom_infor.m_SOPInstanceUID,
											dicom_infor.m_instanceNumber);

			if( !writeOutDICOM(dicom_file_name,true/*is3DVolume=*/)){
				gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendSeries writeOutDICOM to %s Failed\n",dicom_file_name.c_str());
				gLogger.FlushLog();
				throw(-1);//return false;
			};
		}else{
			DICOMInstance dicom_infor = image_list[0];
			dicom_file_name = AppComUtil::getDicomFileName(series_folder,
											dicom_infor.m_SOPInstanceUID,
											dicom_infor.m_instanceNumber);

			if( !writeOutDICOM(dicom_file_name,false/*is3DVolume=*/)){
				gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendSeries writeOutDICOM to %s Failed\n",dicom_file_name.c_str());
				gLogger.FlushLog();
				throw(-1);//return false;
			};
		}
		////
	 
		//
		 
		 
		 
	}catch(int error_code)
	{
		if(error_code<0){
			ret_b = false;
		}
	}
	catch(...)
	{
		ret_b = false;
	}

 	//#142_NoPatientName_NoComment
	//gLogger.LogMessage("INFO:[C%08d] send Series [%s] ID[%s] [%d] [%s] to [%s]  - %s\n",DicomJobProcInfor_CMoveInfo,
	//	entry->m_extInfo.m_PatientName.c_str(),
	gLogger.LogMessage("INFO:[C%08d] send Series ID[%s] [%d] to [%s]  - %s\n", DicomJobProcInfor_CMoveInfo,
		entry->m_extInfo.m_PatientID.c_str(),
		entry->m_extInfo.m_SeriesNumber,
	// entry->m_extInfo.m_Comment.c_str(),
							AE.c_str(), 
							ret_b?"Success":"Failed");
	gLogger.FlushLog();

	return ret_b;
}
//bool COutputJpegProc::sendImage(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,const std::string &SOPInstanceUID)

bool COutputJpegProc::sendImage(const CPxQueueEntry *entry)
{
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;
	const std::string SeriesUID	= entry->m_SeriesInstanceUID;
	const std::string SOPInstanceUID	= entry->m_SOPInstanceUID;
 
	gLogger.LogMessage(kDebug,"COutputJpegProc::sendImage to [%s] \n   [%s]   [%s]   [%s]\n", AE.c_str(),
				StudyUID.c_str(),SeriesUID.c_str(),SOPInstanceUID.c_str());
	gLogger.FlushLog();

	std::string series_folder = AppComUtil::getSeriesFolder(StudyUID, SeriesUID,"*.dcm");
	if(series_folder.size()<1){
		gLogger.LogMessage(kErrorOnly,"COutputJpegProc::sendImage getSeriesFolder error [%s] [%s]\n",
			StudyUID.c_str(),SeriesUID.c_str());
		gLogger.FlushLog();
		return false;
	}
 
#if 0
	////
	CStoreSCUIf *pCStoreSCU_hdr = openCStoreAssociation(AE,SeriesUID);
	if(!pCStoreSCU_hdr){
		gLogger.LogMessage(kDebug,"COutputJpegProc::sendImage openCStoreAssociation error\n");
		gLogger.FlushLog();
		return false;
	}

//	CPxDcmDB pxDb;

 
		pCStoreSCU_hdr->setDicomStudyInfo(entry->m_extInfo.m_PatientName,entry->m_extInfo.m_PatientID,entry->m_extInfo.m_BirthDate);
 

	std::string dicom_file_name;
	std::vector<DICOMInstance> dicom_info;
	DICOMData  iFilter;
	iFilter.Clear();
	strcpy(iFilter.m_SOPInstanceUID, SOPInstanceUID.c_str());

	int status = m_pDcmDB->GetInstanceList( dicom_info, &iFilter, SeriesUID.c_str());
 
	if (status == kOK)
	{
		dicom_file_name = AppComUtil::getDicomFileName(series_folder,
										SOPInstanceUID,
										dicom_info[0].m_instanceNumber);
	}

	if(dicom_file_name.size()<1){
		gLogger.LogMessage(kDebug,"COutputJpegProc::sendImage dicom_file_name is null\n");
		gLogger.FlushLog();
		return false;
	}
	///
	if(!pCStoreSCU_hdr->sendDicomFile(dicom_file_name)){
		gLogger.LogMessage(kDebug,"COutputJpegProc::sendImage sendDicomFile[%s] error\n",dicom_file_name.c_str());
		gLogger.FlushLog();
		return false;
	}

	pCStoreSCU_hdr->closeSession(); //１ Ｉｍａｇｅ 使用終了
	////////
	/// do not close association heere
	 
#endif

	return true;
}

//bool COutputJpegProc::sendEntryFile(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,
//						const std::string &seriesFolder,const std::vector<std::string> &ImageFileList)
bool COutputJpegProc::sendEntryFile(const CPxQueueEntry *entry,const std::string &seriesFolder,const std::vector<std::string> &ImageFileList)
{
//	const int queueID			= entry->m_QueueID;
//	const std::string JobID		= entry->m_JobID;
//	const std::string SOPID		= entry->m_SOPInstanceUID;
	const std::string AE		= entry->m_DestinationAE;
	const std::string StudyUID	= entry->m_StudyInstanceUID;
	const std::string SeriesUID	= entry->m_SeriesInstanceUID;

 
	gLogger.LogMessage(kDebug,"COutputJpegProc::sendSeries to [%s], ( ID[%d] Job[%s] No.[%s] ) [%s] [%s]\n", 
					AE.c_str(),
					entry->m_QueueID,
					entry->m_JobID.c_str(),
					entry->m_SOPInstanceUID.c_str(),
					StudyUID.c_str(),SeriesUID.c_str());
	gLogger.FlushLog();

#if 0
	////
	CStoreSCUIf *pCStoreSCU_hdr = openCStoreAssociation(AE,SeriesUID);
	if(!pCStoreSCU_hdr){
		gLogger.LogMessage(kDebug,"COutputJpegProc::sendEntryFile openCStoreAssociation error\n");
		gLogger.FlushLog();
		return false;
	}
 
		pCStoreSCU_hdr->setDicomStudyInfo(entry->m_extInfo.m_PatientName,entry->m_extInfo.m_PatientID,entry->m_extInfo.m_BirthDate);
 


	int image_size = ImageFileList.size();
	std::string dicom_file_name;
	std::string my_series_folder = seriesFolder + "\\";
	for(int i=0;i<image_size;i++){
		dicom_file_name = my_series_folder + ImageFileList[i];
		if(!pCStoreSCU_hdr->sendDicomFile(dicom_file_name)){
			gLogger.LogMessage(kDebug,"COutputJpegProc::sendEntryFile sendDicomFile[%s] error\n",dicom_file_name.c_str());
			gLogger.FlushLog();
			return false;
		}
	}
	//

	pCStoreSCU_hdr->closeSession(); //１ブロック  使用終了
	 
	 ////////
	/// do not close association heere

#endif
	return true;
}

class PxImageInforInstanceNumberSort
{
public:
    bool operator()( const DICOMInstance& lhs, const DICOMInstance& rhs ) const
    {
        return lhs.m_instanceNumber  < rhs.m_instanceNumber;
    }
};
bool COutputJpegProc::queryImages(const std::string studyUID,const std::string &seriesUID,std::vector<DICOMInstance> &image_list)
{
	//CPxDcmDB pxDb;

	DICOMData  Filter;

	Filter.Clear();
	;

	DICOMStudy  iStudy;
	strcpy(Filter.m_studyInstanceUID, studyUID.c_str());
	strcpy(Filter.m_seriesInstanceUID, seriesUID.c_str());

	image_list.clear();

	int status = m_pDcmDB->GetInstanceList(image_list,&Filter,seriesUID.c_str());

 
	if (status != kOK)
	{
		gLogger.LogMessage(kErrorOnly," ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		gLogger.FlushLog();
		return false;
	}
	//
	
	std::sort(image_list.begin(),image_list.end(),PxImageInforInstanceNumberSort());

	 
	return true;
}

bool COutputJpegProc::querySeries(const std::string studyUID, std::vector<DICOMSeries> &series_list)
{
//	CPxDcmDB pxDb;

	DICOMData studyFilter;

	studyFilter.Clear();
	 
	strcpy(studyFilter.m_studyInstanceUID, studyUID.c_str());

	series_list.clear();

 	int status = m_pDcmDB->GetSeriesList(series_list,&studyFilter);
 

	 
	if (status != kOK)
	{
		gLogger.LogMessage(kErrorOnly," ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		gLogger.FlushLog();
		return false;
	}
	//
	return true;
}
 
 
bool COutputJpegProc::init( )
{
	TRCSLock fplock(m_cs);

	m_OutputJPEGHome = gConfig.m_ExportJPEG_FolderName;

	m_JpegTempFolder = gConfig.m_JpegTempFolder;//#71

	if(gConfig.m_ActionGate == 1){ //#71 K.Ko
	}else{
		if (strlen(gConfig.m_ExportJPEG_FolderSubName) > 1){
			//#98 2017/07/10 N.Furutsuki
			m_OutputJPEGHome = std::string(gConfig.m_ExportJPEG_FolderSubName) + "\\";
		}
		else{
			m_OutputJPEGHome = m_OutputJPEGHome + "\\DeltaImage\\";
		}
	}
  
	if(!CDicomImageDataProc::init()){
		gLogger.LogMessage("ERROR: CDicomImageDataProc::init failed\n");
		gLogger.FlushLog();
	}
	
	return initDICOM();

//	return true;
}

bool COutputJpegProc::initDICOM()
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

	//writeOutDICOM("C:/Test/20130809181205_Orlando^John_000000003/09786043/32201924/00002DCM",false);
	return true;
}
bool checkExportJPEG(bool dispLog,bool *cancelFlag=0);
bool COutputJpegProc::writeOutDICOM(const std::string &dicomFileName,bool is3DVolume )
{
	if(!checkExportJPEG(false,&m_cancelFlag)) return false;

	bool ret_b = true;
	int status;
	 
	char _str_buff[256];

//	CPxDicomMessage *dcm_msg = new CPxDicomMessage;
	std::auto_ptr<CPxDicomImage > pDicom = std::auto_ptr<CPxDicomImage > (new CPxDicomImage);
	
	unsigned char *ImageDataBuff = 0;

	try {
		status = pDicom->Load(dicomFileName.c_str());

		if(status != kNormalCompletion){
			throw(-1);
		}
		int iMessageID = pDicom->GetID();
 
		bool skip_proc = false;
		/////////////
		//SOPClassUID
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_SOP_CLASS_UID, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
			throw(-1);
		}
		std::string SOPCLASSUID = _str_buff;


		/////////////
		//ImageType
		std::string ImageType;
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_IMAGE_TYPE, _str_buff, sizeof(_str_buff));
		if(status == MC_NORMAL_COMPLETION) ImageType = _str_buff;

		while (status == MC_NORMAL_COMPLETION){
			_str_buff[0]=0;
			status = pDicom->GetNextValue(MC_ATT_IMAGE_TYPE, _str_buff, sizeof(_str_buff));
			std::string str_temp = _str_buff;
			if(str_temp.size()<1){
				break;
			}else{
				ImageType = ImageType + std::string("\\")+ _str_buff;
			}
		}
		 

		std::string disp_image_type = "--";
		if(SOPCLASSUID == "1.2.840.10008.5.1.4.1.1.7"){//SecondaryCaptureImageStorage
			int pos;
			if((pos=ImageType.find("TERARECON"))>=0){//シーン保存
				skip_proc = true;
				disp_image_type = "Scene saving";
			}else
			if((pos=ImageType.find("AQNETSC"))>=0){//画像保存
				skip_proc = true;
				disp_image_type = "Image saving";
			}
		}

 
		if(skip_proc){
			gLogger.LogMessage(kWarning," SOPCLASSUID is secondary capture. [%s] is ignored \n",disp_image_type.c_str());
			gLogger.FlushLog();
			throw(1);
		}
 
		/////////////
		//PatientName
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_PATIENTS_NAME, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
			throw(-1);
		}

		/////////////
		//PatientID
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_PATIENT_ID, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
			throw(-1);
		}
		std::string patientID = _str_buff;


		/////////////
		//Date
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_SERIES_DATE, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
		//	throw(-1);
			//try StudyDate
			status = pDicom->GetValue(MC_ATT_STUDY_DATE, _str_buff, sizeof(_str_buff));
			if (status != MC_NORMAL_COMPLETION){
				throw(-1);
			}
		}
		std::string Date = _str_buff;

		/////////////
		//Time
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_SERIES_TIME, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
		//	throw(-1);
			//try StudyDate
			status = pDicom->GetValue(MC_ATT_STUDY_DATE, _str_buff, sizeof(_str_buff));
			if (status != MC_NORMAL_COMPLETION){
				throw(-1);
			}
		}
		std::string Time = _str_buff;


		/////////////
		//StudyUID
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_STUDY_INSTANCE_UID, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
			throw(-1);
		}
		std::string study_uid = _str_buff;

		/////////////
		//SeriesUID
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_SERIES_INSTANCE_UID, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
			throw(-1);
		}
		std::string series_uid = _str_buff;


		/////////////
		//Modality
		_str_buff[0]=0;
		status = pDicom->GetValue(MC_ATT_MODALITY, _str_buff, sizeof(_str_buff));
		if (status != MC_NORMAL_COMPLETION){
			throw(-1);
		}
		std::string modality = _str_buff;

		CDeltaViewInfoEntry::ImageType new_imageType = CDeltaViewInfoEntry::ImageType_P;

		if(modality == "DX"){
			new_imageType = CDeltaViewInfoEntry::ImageType_C;
		}

		if(is3DVolume){
			new_imageType = CDeltaViewInfoEntry::ImageType_3D;
		}

		PxImagePixel pixelData;
		if(!setupImagePixel(pDicom.get(),pixelData,true/* rawData*/)){
			gLogger.LogMessage(kErrorOnly," writeOutDICOM setupImagePixel error \n");
			gLogger.FlushLog();
			throw(-1);
		}

		//#83 2014/10/30 N.Furutsuki
		bool use_recommended_wwwl = false;
		if(new_imageType==CDeltaViewInfoEntry::ImageType_P){
		// just for pano only
			use_recommended_wwwl =		((pixelData.m_wl*pixelData.m_wl)>0.01) &&
										((pixelData.m_ww*pixelData.m_ww)>0.01) ;
		}
	 
		
		PxImagePixel pixelDataOut;
		CHistogramData makeHist;

		if(!use_recommended_wwwl){//#83 2014/10/30 N.Furutsuki
			if(!calImageHist(&pixelData,&makeHist)){
				gLogger.LogMessage(kErrorOnly," writeOutDICOM calImageHist error \n");
				gLogger.FlushLog();
				throw(-1);
			}
		}

		
//		if(!dispImagePixel(pixelData,  pixelDataOut, &makeHist)){
		//#83 2014/10/30 N.Furutsuki
		if(!dispImagePixel(pixelData,  pixelDataOut, use_recommended_wwwl? 0 : &makeHist)){
			gLogger.LogMessage(kErrorOnly," writeOutDICOM dispImagePixel error \n");
			gLogger.FlushLog();
			throw(-1);
		}

		ImageDataBuff = pixelDataOut.m_pixelData;

		int sizeX = pixelDataOut.m_sizeX;
		int sizeY = pixelDataOut.m_sizeY;
		int line_alloc_sizeX = pixelDataOut.m_line_alloc_sizeX;
		int writeSizeX = pixelDataOut.m_sizeX;
		int writeSizeY = pixelDataOut.m_sizeY;
 
		 	
//#71 Chen {[(
		//{
		//	JpegTagWriter::UserTag tag;
		//	JpegTagWriter writer;
		//	std::string src, dst;
		//	tag.strDataTime = "20130520130220";
		//	tag.strPatientID = "100000002";
		//	tag.strStudyUID = "2.16.840.1.113669.632.21.23688796.715862132.858993553.7100739801";
		//	tag.strSeriesUID = "2.16.840.1.113669.632.21.23688796.715862132.858993553.3109477706";
		//	tag.strImageType = "3"; //0 dental
		//	//1 panoroma
		//	tag.strOriginalFolderName = "tmp";
		//	tag.strOriginalFolderPath = "c:\\tmp";
		//	src = "C:\\00000\\write-exif.jpg";
		//	bool bRtn = writer.AddUserTag(tag, src, dst);
		//}
		
		if(gConfig.m_ActionGate){
			

			if (JpegTagWriter::IsModualExisted(gConfig.m_ActionGateYTWExePath) == false){
				gLogger.LogMessage(kErrorOnly," JpegTagWriter Modual is not existed \n");
				return false;
			} 

			std::auto_ptr<JpegTagWriter::UserTag> jpeg_tag  = std::auto_ptr<JpegTagWriter::UserTag>(new JpegTagWriter::UserTag);
			jpeg_tag->strDataTime = Date+Time; //Please check format ! K.Ko
			jpeg_tag->strSoftware = "";	
			jpeg_tag->strPatientID = patientID;
			jpeg_tag->strSeriesUID = series_uid;
			int index = dicomFileName.find_last_of("\\");
			if (index < 0) index = dicomFileName.find_last_of("/");
			assert(index > 0);
			jpeg_tag->strOriginalFolderName = dicomFileName.substr(0, index);
			jpeg_tag->strOriginalFolderPath = dicomFileName.substr(index+1, dicomFileName.size()-1);
			jpeg_tag->eImageType = JpegTagWriter::ImageTypeDental; //0 DENTAL

			//
			//今は、以下の３種類 

			switch(new_imageType){
				case CDeltaViewInfoEntry::ImageType_P:
					jpeg_tag->eImageType = JpegTagWriter::ImageTypePanoroma;
					break;
				case CDeltaViewInfoEntry::ImageType_C:
					jpeg_tag->eImageType = JpegTagWriter::ImageTypeCephalo;
					break;
				case CDeltaViewInfoEntry::ImageType_3D:
					jpeg_tag->eImageType = JpegTagWriter::ImageType3DImage;
					break;
			}

			jpeg_tag->strStudyUID = study_uid;
			 
			//

			CDeltaViewInfoEntry new_entry;/*output*/
			if(!addDeltaViewEntry(new_entry,new_imageType,study_uid,series_uid,Date,true/*useSeriesUID*/)){
				throw(-1);
			}
			OutJPEG_Folder_Info ID_Folder_Info;

			bool dbg_jpeg = false;
			if(dbg_jpeg){
				//for debug write JPEG 
				ID_Folder_Info.m_Exp_FolderName = "c:\\temp\\";
			}else{
				if(!checkPatientID(patientID,ID_Folder_Info)){
					throw(-1);
				}
			}

			{//write out JPEG for Action Gate

				//write Expansion JPEG
			//	if(new_imageType != CDeltaViewInfoEntry::ImageType_3D){
					std::string ext_jpeg_file_name = new_entry.m_ImgID;
				//	ext_jpeg_file_name = ID_Folder_Info.m_Exp_FolderName + std::string("\\") + ext_jpeg_file_name ;
					ext_jpeg_file_name = ext_jpeg_file_name + std::string(".jpg");
				 
					if(!writeJPEG(ext_jpeg_file_name,ImageDataBuff, line_alloc_sizeX, sizeY, writeSizeX, writeSizeY,jpeg_tag.get())){
						gLogger.LogMessage(kErrorOnly," writeJPEG [%s] error \n",ext_jpeg_file_name);
						gLogger.FlushLog();
						throw(-1);
					}
				 
				
			//	}

				//write Thumbnail JPEG
				//自動生成
				
			}

			return true;//終了
		}
//#71 Chen )]}
		OutJPEG_Folder_Info ID_Folder_Info;

		CPxFolderLocker lock_folder;//#97 2017/07/10 N.Furutsuki

		bool dbg_jpeg = false;
		if(dbg_jpeg){
			//for debug write JPEG 
			ID_Folder_Info.m_Exp_FolderName = "c:\\temp\\";
		}else{
			if(!checkPatientID(patientID,ID_Folder_Info)){
				throw(-1);
			}
			//#97 2017/07/10 N.Furutsuki
			lock_folder.setup(ID_Folder_Info.m_ID_FolderName);
			lock_folder.setupCancelFlag(&m_cancelFlag);
			lock_folder.lock();

			if(!readDeltaViewInfo(ID_Folder_Info.m_DaxInfo_FileName)){
				;//new file//throw(-1);
				m_DeltaViewInfo.m_KanjaNo = ID_Folder_Info.m_ID;
			}else{
				if(m_DeltaViewInfo.m_KanjaNo != ID_Folder_Info.m_ID){
					gLogger.LogMessage(kErrorOnly," ERROR: invalid PatientID  [%s] != [%s] \n",m_DeltaViewInfo.m_KanjaNo.c_str(),ID_Folder_Info.m_ID.c_str());
					gLogger.FlushLog();
					throw(-1);
				}
			}
			if(!checkSeriesUID(series_uid)){
				gLogger.LogMessage(kErrorOnly," SeriesUID [%s] already exists \n",series_uid.c_str());
				gLogger.FlushLog();
				throw(1);
			};
		}

		//moved

		CDeltaViewInfoEntry new_entry;/*output*/
		if(!addDeltaViewEntry(new_entry,new_imageType,study_uid,series_uid,Date)){
			throw(-1);
		}
		
		
		{//write out JPEG

			//moved

			//write Expansion JPEG
			if(new_imageType != CDeltaViewInfoEntry::ImageType_3D){
				std::string ext_jpeg_file_name = new_entry.m_ImgID;
				ext_jpeg_file_name = ID_Folder_Info.m_Exp_FolderName + std::string("\\") + ext_jpeg_file_name ;
				ext_jpeg_file_name = ext_jpeg_file_name + std::string(".jpg");
			 
				if(!writeJPEG(ext_jpeg_file_name,ImageDataBuff, line_alloc_sizeX, sizeY, writeSizeX, writeSizeY)){
					gLogger.LogMessage(kErrorOnly," writeJPEG [%s] error \n",ext_jpeg_file_name);
					gLogger.FlushLog();
					throw(-1);
				}
		 
			 
			}

			//write Thumbnail JPEG
			{

				writeSizeX = 200;
				writeSizeY = (int)((float)writeSizeX/sizeX*sizeY+0.5f);

				int new_line_alloc_sizeX = (writeSizeX/8+1)*8;
				unsigned char *Thum_ImageDataBuff = new unsigned char[new_line_alloc_sizeX*writeSizeY];

				bool write_b=true;
 
				if(!CDicomImageDataProc::resizeImage(/* input */ ImageDataBuff,  sizeX, sizeY,
							line_alloc_sizeX,
							/* output*/ Thum_ImageDataBuff,writeSizeX,writeSizeY,
							new_line_alloc_sizeX)){
						gLogger.LogMessage(kErrorOnly," resizeImage error \n" );
						gLogger.FlushLog();
						write_b = false;
				}else{
 

					std::string ext_jpeg_file_name = new_entry.m_ImgID;
					ext_jpeg_file_name = ID_Folder_Info.m_Thum_FolderName + std::string("\\") + ext_jpeg_file_name ;
					ext_jpeg_file_name = ext_jpeg_file_name + std::string(".jpg");
				 
			 
					if(!writeJPEG(ext_jpeg_file_name,Thum_ImageDataBuff, new_line_alloc_sizeX, writeSizeY, writeSizeX, writeSizeY)){
						gLogger.LogMessage(kErrorOnly," writeJPEG [%s] error \n",ext_jpeg_file_name);
						gLogger.FlushLog();
						write_b = false;
					}

				}

				delete [] Thum_ImageDataBuff;

				if(!write_b){
					
					throw(-1);
				}
		 
				 
			}
			
		}

		//update DxaInfo.ini
		if(!writeDeltaViewInfo(ID_Folder_Info.m_DaxInfo_FileName)){
			throw(-1);
		}
		//#97 2017/07/10 N.Furutsuki
		lock_folder.unlock();


	}catch(int err_no){
		if(err_no<0){
			ret_b = false;
		}else{
			ret_b = true;
		}
	}catch(...){
		ret_b = false;
	}
//	delete pDicom;

 // 	if(ImageDataBuff) delete [] ImageDataBuff;

	return ret_b;
 
}

bool COutputJpegProc::checkPatientID(const std::string &ID,OutJPEG_Folder_Info &Info)
{
	char _format_str[64];

#if 0//#54 
	//ID桁数まで 0埋め
	sprintf(_format_str,"%s%ds", "%0",gConfig.m_IDJpegFolderMaxLen);
#else
	strcpy(_format_str,"%s");
#endif
	char folder_name_str[128];
	sprintf(folder_name_str,_format_str,ID.c_str());

	Info.m_ID = folder_name_str;
	Info.m_ID_FolderName= m_OutputJPEGHome + folder_name_str;

	int status;


	if(gConfig.m_ActionGate == 1){//#71 K.Ko

	}else{
		//ID folder
		if (TRPlatform::IsDirectory(Info.m_ID_FolderName.c_str()) <=0)
		{
			 
			status = TRPlatform::MakeDirIfNeedTo(Info.m_ID_FolderName.c_str());
			if (status < 0){
				return false;
			}
		 
		}

		//Expansion folder
		Info.m_Exp_FolderName = Info.m_ID_FolderName + std::string("\\Expansion");
		if (TRPlatform::IsDirectory(Info.m_Exp_FolderName.c_str()) <=0)
		{
			status = TRPlatform::MakeDirIfNeedTo(Info.m_Exp_FolderName.c_str());
			if (status < 0){
				return false;
			}
			  ;
		}
		//Thumbnail folder
		Info.m_Thum_FolderName = Info.m_ID_FolderName + std::string("\\Thumbnail");
		if (TRPlatform::IsDirectory(Info.m_Thum_FolderName.c_str()) <=0)
		{
			status = TRPlatform::MakeDirIfNeedTo(Info.m_Thum_FolderName.c_str());
			if (status < 0){
				return false;
			}
		}

		Info.m_DaxInfo_FileName = Info.m_ID_FolderName + "\\DxaInfo.ini";

	}
 
	return true;
}


bool COutputJpegProc::readDeltaViewInfo(const std::string &IniFileName)
{
	bool ret_b = true;


	m_DeltaViewInfo.destroy();

	char *read_buff = 0;
	FILE *fp = 0;
 
	try{
		struct stat statBuf;
		if(::stat(IniFileName.c_str(), &statBuf)) 
		{
	 
			return false;
		}
		if(statBuf.st_size<1) {
			return false;
		}

		///
		fp = fopen(IniFileName.c_str(),"rb");
		if(!fp){
			throw(-1);
		}
		read_buff = new char[statBuf.st_size+4];
		int read_n = fread(read_buff,1,statBuf.st_size,fp);
		if(read_n < statBuf.st_size){
			throw(-1);
		}
		ret_b = ParseEntry(read_buff,m_DeltaViewInfo);

//		CStdioFile file;
	}catch(int){
		ret_b = false;
	}catch(...){
		ret_b = false;
	}
	if(fp){
		fclose(fp);
	}
	if(read_buff){
		delete [] read_buff;
	}

	m_DeltaViewInfo.m_ImgCount;
	int image_count = m_DeltaViewInfo.m_EntryList.size();

	return ret_b;
}

bool COutputJpegProc::ParseEntry(const char *buff,CDeltaViewInfo &outInof)
{

	char line_buf[1024], name[1024], value[1024];
	const char* head = buff;
	int n;
	
	if (!buff || !buff[0])
	{
		 
		return false;
	}

 	bool do_flag = true;
	int line_nn = 0;

	CDeltaViewInfoEntry entry_item;

	bool read_header = true;
	outInof.m_ImgCount = -1;

	int cur_image_id = -1;
	while ( do_flag)
	{
		head = GetNextLine(head,line_buf, sizeof(line_buf));
		if(!head){
			do_flag = false;
		}else
		if(head[0] == 00){
			do_flag = false;
		}
		if(strlen(line_buf)<1){
			continue;
		}else{
			line_nn++;
		}
		
		int image_num =  getImageNumber(line_buf);
		if(image_num>=0){
			if(entry_item.m_ImgID.size()>0){
				outInof.m_EntryList[entry_item.m_ImgID] = entry_item;

				updateImageCount(outInof,entry_item.m_ImgID);
				CDeltaViewInfoEntry new_null;
				entry_item = new_null;
			}
			cur_image_id = image_num;
			continue;
		} 

		if (!Split(line_buf,name, value, sizeof(name), '='))
		{
			continue;
		}
		 
		std::string name_str = name;
		if(read_header){
			if(name_str == DeltaViewInfoEntryDef_KanjaNo){
	 			outInof.m_KanjaNo = value;
				gLogger.LogMessage(kInfo," read header m_KanjaNo= [%s] \n",value );
				gLogger.FlushLog();

			}else
			if(name_str == DeltaViewInfoEntryDef_ImgCount){
	 			outInof.m_ImgCount = atoi(value);
			}
			if( (outInof.m_ImgCount>0) && 
		//		(outInof.m_KanjaNo.size()>1)
				(outInof.m_KanjaNo.size()>0) //#100  2017/09/20 N.Furutsuki
				){
				read_header = false;
				 
			}
		}else{
			
			
			if(strlen(value)<1) {
				continue;
			}
			/////
			if(name_str == DeltaViewInfoEntryDef_ImgID){
	 			entry_item.m_ImgID		= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_StudyUID){
	 			entry_item.m_StudyUID	= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_SeriesUID){
				entry_item.m_SeriesUID	= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_ImgType){
				entry_item.m_ImgType	= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_ImgBui){
				entry_item.m_ImgBui	= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_ImgHani){
				entry_item.m_ImgHani	= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_ImgDate){
				entry_item.m_ImgDate	= value;
			}else
			if(name_str == DeltaViewInfoEntryDef_ImgExt){
				entry_item.m_ImgExt	= value;
			}
		}
	}
	//last
	if(entry_item.m_ImgID.size()>0){
		outInof.m_EntryList[entry_item.m_ImgID] = entry_item;

		updateImageCount(outInof,entry_item.m_ImgID);
		CDeltaViewInfoEntry new_null;
		entry_item = new_null;
	}


	return true;
}

bool COutputJpegProc::updateImageCount(CDeltaViewInfo &outInfo,const std::string &imageID)
{
	const char *pStr = imageID.c_str();
	//find number;
	int pos = -1;
	for(int i=0;i<imageID.size();i++){
		if((pStr[i] >= '0')&&(pStr[i] <= '9')){
			pos = i;
			break;
		} 
	}
	if(pos<0) return false;

	std::string first_Char = imageID.substr(0,pos);
	std::string num_str = imageID.substr(pos);
	int image_num = atoi(num_str.c_str());
	if(outInfo.m_ImageCountPerTypeRecord[first_Char].m_ImageCount < image_num){
		outInfo.m_ImageCountPerTypeRecord[first_Char].m_ImageCount = image_num;
	}

	return true;
}

bool COutputJpegProc::addDeltaViewEntry(CDeltaViewInfoEntry &new_entry/*output*/,CDeltaViewInfoEntry::ImageType type,const std::string &StudyUID,const std::string &SeriesUID,const std::string &Date,bool useSeriesUID)
{
//	CDeltaViewInfoEntry new_entry;

	if(useSeriesUID){
		new_entry.m_ImgID		= SeriesUID;
	}else{
 		new_entry.m_ImgID		= genNewTypeID(m_DeltaViewInfo,m_TypeFirstCharTable[type]);
	}
	new_entry.m_ImgType		= m_TypeTable[type];
	new_entry.m_ImgDate		= Date;
	new_entry.m_ImgExt		= m_ExtTable[type];
	new_entry.m_StudyUID	= StudyUID;
	new_entry.m_SeriesUID	= SeriesUID;

	m_DeltaViewInfo.m_EntryList[new_entry.m_ImgID] = new_entry;

	return true;

}

//#99 2017/08/08
//Exception of TDO Screen Capture 
inline bool ExceptionCheckSeriesUID(const std::string &ImgType)
{
	bool ret_b = false;
	if (gConfig.m_NoCheckSeriesUIDImageType1[0]){
		if (ImgType == gConfig.m_NoCheckSeriesUIDImageType1){
			ret_b = true;
		}
	}
	if (gConfig.m_NoCheckSeriesUIDImageType2[0]){
		if (ImgType == gConfig.m_NoCheckSeriesUIDImageType2){
			ret_b = true;
		}
	}
	if (gConfig.m_NoCheckSeriesUIDImageType3[0]){
		if (ImgType == gConfig.m_NoCheckSeriesUIDImageType3){
			ret_b = true;
		}
	}
	if (gConfig.m_NoCheckSeriesUIDImageType4[0]){
		if (ImgType == gConfig.m_NoCheckSeriesUIDImageType4){
			ret_b = true;
		}
	}
	return ret_b;
}
//
//
bool COutputJpegProc::checkSeriesUID(const std::string &seriesUID)
{
	bool ret_b = true;
	std::map<std::string/*ImgID*/,CDeltaViewInfoEntry>::iterator it = m_DeltaViewInfo.m_EntryList.begin();
		 
	while(it!=m_DeltaViewInfo.m_EntryList.end()){
		if(seriesUID == it->second.m_SeriesUID){
		//既にSeriesUID存在
			if (ExceptionCheckSeriesUID(it->second.m_ImgType )){ //"SC"){
			//#99 2017/08/08
			//Exception of TDO Screen Capture 
			}else{
				ret_b = false;
				break;
			}
		}
		it++;
	}
			
	return ret_b;
}
std::string COutputJpegProc::genNewTypeID(CDeltaViewInfo &outInfo,const std::string &typeFirstChar)
{
	std::string ret_imageType;

	int cur_image = outInfo.m_ImageCountPerTypeRecord[typeFirstChar].m_ImageCount;
	cur_image++;
	char _char_buff[128];
	sprintf(_char_buff,"%d",cur_image);

	ret_imageType = _char_buff;
	ret_imageType = typeFirstChar+ret_imageType;
	outInfo.m_ImageCountPerTypeRecord[typeFirstChar].m_ImageCount = cur_image;

	return ret_imageType;
}
 
inline void printOut(FILE *fp,const char *keyName,const std::string &keyValue)
{
	if(keyValue.size()<1) {
		fprintf(fp,"%s=\n",	keyName );
	}else{
		fprintf(fp,"%s=%s\n",	keyName,	keyValue.c_str());
	}
}
bool COutputJpegProc::writeDeltaViewInfo(const std::string &IniFileName)
{
	int ret_b = true;

	int imageCount = m_DeltaViewInfo.m_EntryList.size();
	
	try{
		FILE *fp = fopen(IniFileName.c_str(),"wt");
		//header
		fprintf(fp,"[Info]\n");
		fprintf(fp,"KanjaNo=%s\n",m_DeltaViewInfo.m_KanjaNo.c_str());
		fprintf(fp,"ImgCount=%d\n",imageCount);
		//
		std::map<std::string/*ImgID*/,CDeltaViewInfoEntry>::iterator it = m_DeltaViewInfo.m_EntryList.begin();
		int run_i = 0;
		while(it!=m_DeltaViewInfo.m_EntryList.end()){
			fprintf(fp,"[%d]\n",run_i+1);
			printOut(fp,	DeltaViewInfoEntryDef_ImgID,		it->second.m_ImgID );
			printOut(fp,	DeltaViewInfoEntryDef_ImgType,		it->second.m_ImgType);
			printOut(fp,	DeltaViewInfoEntryDef_StudyUID,		it->second.m_StudyUID);
			printOut(fp,	DeltaViewInfoEntryDef_SeriesUID,	it->second.m_SeriesUID);
			printOut(fp,	DeltaViewInfoEntryDef_ImgBui,		it->second.m_ImgBui);
			printOut(fp,	DeltaViewInfoEntryDef_ImgHani,		it->second.m_ImgHani);
			printOut(fp,	DeltaViewInfoEntryDef_ImgDate,		it->second.m_ImgDate);
			printOut(fp,	DeltaViewInfoEntryDef_ImgExt,		it->second.m_ImgExt);

			//
			it++;
			run_i++;

		}
		fclose(fp);
	}catch(...){
		ret_b = false;
	}
	return ret_b;
}

bool COutputJpegProc::writeJPEG(const std::string &jpegFileName,const unsigned char *imageData,int sizeX,int sizeY,int writeSizeX,int writeSizeY)
{ 
	return CDicomImageDataProc::writeJPEG(jpegFileName,imageData, sizeX, sizeY, writeSizeX, writeSizeY,gConfig.m_OutputJpegQuality);
}

//#71 Chen
bool COutputJpegProc::writeJPEG(const std::string &jpegFileName,const unsigned char *imageData,int sizeX,int sizeY,int writeSizeX,int writeSizeY, const JpegTagWriter::UserTag* pTag)
{	
	bool bRtn = false;
	if (pTag == NULL){
		bRtn = this->writeJPEG(jpegFileName,imageData, sizeX, sizeY, writeSizeX,writeSizeY);
	}else{
		JpegTagWriter writer(gConfig.m_ActionGateYTWExePath);
		if (writer.IsModualExisted(gConfig.m_ActionGateYTWExePath) == false){
			gLogger.LogMessage(kErrorOnly," JpegTagWriter is not valid @COutputJpegProc::writeJPEG\n");	
		}else{
			writer.m_ActionGateForE2 = (gConfig.m_ActionGateForE2==1);//#73 2013/11/26 K.Ko

			struct tm *tm;
			time_t t = time (NULL);
			tm = localtime (&t);
			char szTm[1024] = "";

			_snprintf (szTm, sizeof(szTm),
				"%04i%02i%02i_%02i_%02i_%02i_%d",
				tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec, GetTickCount());

			int idx = jpegFileName.find_last_of("\\");
			if (idx < 0) idx = jpegFileName.find_last_of("/");
			std::string strTemp = jpegFileName.substr(idx+1, jpegFileName.size()-1);
//#error (szTempFolder should be initialized from configure)
			const std::string sTempFolder = m_JpegTempFolder;//"c:\\TempJpeg";
			std::string strJpgFolder = sTempFolder + "\\" + szTm;
#if 0
			::CreateDirectory(sTempFolder, NULL);
			::CreateDirectory(strJpgFolder.c_str(), NULL);
#else
			TRPlatform::MakeDirIfNeedTo(strJpgFolder.c_str());
#endif

			strTemp = std::string(strJpgFolder) + "\\" + jpegFileName;

			bRtn = CDicomImageDataProc::writeJPEG(strTemp,imageData, sizeX, sizeY, writeSizeX, writeSizeY,gConfig.m_OutputJpegQuality);
			if (bRtn){
				std::string strDst, strCmd;
				bRtn = writer.AddUserTag(*pTag, strTemp, strDst, strCmd);
				if (bRtn){
					//if (strDst == jpegFileName){
					//	bRtn = true;
					//	//the file is alread in specified position
					//}else{
					//	bRtn = ::CopyFile(strDst.c_str(), jpegFileName.c_str(), FALSE)?true:false;
					//	::DeleteFile(strDst.c_str());	
					//	if (!bRtn) gLogger.LogMessage(kErrorOnly," Copy Jpeg File [src =%s, dst =%s] error \n",strDst, jpegFileName);
					//}
				}else{
					gLogger.LogMessage(kErrorOnly," JpegTagWriterAddUserTag [%s] error \n",jpegFileName.c_str());
					gLogger.LogMessage(kErrorOnly, strCmd.c_str());
					gLogger.FlushLog();
				}
			}
			std::string strRmdirCmd = "RD /S /Q " + strJpgFolder;
			system(strRmdirCmd.c_str());
		}
	}

	return bRtn;
}

bool COutputJpegProc::calImageHist(const PxImagePixel *image,CHistogramData *makeHist )
{
 

	if(image->m_samplesPerPixel == 3){
		//
	}else{
		if(image->m_bits==8){
			if(makeHist){
				makeHist->createHist(256);
			}
			unsigned char *p_pixel_src = image->m_pixelData;

			if(makeHist){
				makeHist->m_totalPixels = image->m_sizeX*image->m_sizeY;//m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY;

				if(image->m_samplesPerPixel == 4){
					for(int y_i=0;y_i<image->m_sizeY;y_i++){
						for(int x_i=0;x_i<image->m_sizeX;x_i++){
							int index =  (y_i*image->m_sizeX + x_i);
							unsigned char data;
							data = p_pixel_src[4*index + 0];
							makeHist->m_hist[data] ++;
						 	data = p_pixel_src[4*index + 1];
						 	makeHist->m_hist[data] ++;
						 	data = p_pixel_src[4*index + 2];
						 	makeHist->m_hist[data] ++;
						//	 
							 
						}
					}

				}else{
					for(int y_i=0;y_i<image->m_sizeY;y_i++){
						for(int x_i=0;x_i<image->m_sizeX;x_i++){
							int index =  (y_i*image->m_sizeX + x_i);
							unsigned char data = p_pixel_src[index];
							makeHist->m_hist[data] ++;
							 
							 
						}
					}
				}
			}
		}else
		if(image->m_bits>8)
		{
			if(image->m_pixelRepresentation == 1){
				int used_bits_max  = (1<<image->m_bits) ;
				
				if(image->m_hu_offset>0.0){
				//means: 12Bit CT
					used_bits_max  = (1<<12) ;
				}
				float gain = 1.0/(float)used_bits_max;
				short *p_pixel_src = ( short *)image->m_pixelData;
				//
				 
				float offset_val = image->m_rescaleIntercept*gain ;
				offset_val += image->m_hu_offset*gain;;
				//
				gain *=image->m_rescaleSlope;
				
				if(makeHist){
					makeHist->createHist(used_bits_max);
					makeHist->m_totalPixels = image->m_sizeX*image->m_sizeY;
				}
				for(int y_i=0;y_i<image->m_sizeY;y_i++){
					for(int x_i=0;x_i<image->m_sizeX;x_i++){
						int index =  (y_i*image->m_sizeX + x_i);
						float hist_val =  p_pixel_src[index]*gain + offset_val;
						 
						if(makeHist){
							int index_no = hist_val*used_bits_max;
							if(index_no<0) index_no = 0;
							if(index_no>=used_bits_max) index_no = used_bits_max-1;
							makeHist->m_hist[index_no] ++;
						}
					}
				}

			}else{
				int used_bits_max  = (1<<image->m_bits) ;
				
				float gain = 1.0/(float)used_bits_max;
				short *p_pixel_src = ( short *)image->m_pixelData;

				float offset_val = image->m_rescaleIntercept*gain ;
				offset_val += image->m_hu_offset*gain;;
				//
				gain *=image->m_rescaleSlope;
				
				if(makeHist){
					makeHist->createHist(used_bits_max);
					makeHist->m_totalPixels = image->m_sizeX*image->m_sizeY;
				}

				for(int y_i=0;y_i<image->m_sizeY;y_i++){
					for(int x_i=0;x_i<image->m_sizeX;x_i++){
						int index =  (y_i*image->m_sizeX + x_i);
						float hist_val =  p_pixel_src[index]*gain + offset_val;

						if(makeHist){
							int index_no = hist_val*used_bits_max;
							if(index_no<0) index_no = 0;
							if(index_no>=used_bits_max) index_no = used_bits_max-1;
							makeHist->m_hist[index_no] ++;
						}
						 
					}
				}
			}
			

		}
	}
	return true;
}
bool COutputJpegProc::dispImagePixel(const PxImagePixel &pixelDataIn, PxImagePixel &pixelDataOut,CHistogramData *makeHist)
{
	int sizeX = pixelDataIn.m_sizeX;
    int sizeY = pixelDataIn.m_sizeY;

	if(pixelDataOut.m_pixelData){
		delete [] pixelDataOut.m_pixelData;
	}
	pixelDataOut.m_bits = 8;
 

	pixelDataOut = pixelDataIn;
	pixelDataOut.m_pixelData = 0;

	pixelDataOut.m_line_alloc_sizeX = (sizeX/8+1)*8;

	int sliceSize = sizeX*sizeY;
	int alloc_sliceSize = pixelDataOut.m_line_alloc_sizeX*sizeY;


	 
	int adjSamplePerPixel = pixelDataIn.m_samplesPerPixel;;
	int alloc_pixel_buff_len = alloc_sliceSize*adjSamplePerPixel ;
	//pixelDataOut.m_pixelData = (unsigned char*)(new unsigned short[alloc_pixel_buff_len/2+1]);
	pixelDataOut.m_pixelData = (unsigned char*)(new unsigned char[alloc_pixel_buff_len +1]);

	if(pixelDataIn.m_samplesPerPixel == 3){
		//
	}else{
		if(pixelDataIn.m_bits==8){
			
			unsigned char *p_pixel_src	= pixelDataIn.m_pixelData;
			unsigned char *p_pixel_dest = pixelDataOut.m_pixelData;

 
			int all_size = pixelDataIn.m_sizeX*pixelDataIn.m_sizeY;
			for(int i=0;i<all_size;i++){
				p_pixel_dest[ i ] =  p_pixel_src[i] ;
			}
		}else
		if(pixelDataIn.m_bits>8)
		{

			float wl = pixelDataIn.m_wl;
			float ww = pixelDataIn.m_ww;

			if(makeHist){
				int pixel_min,pixel_max;
				makeHist->findRange(pixel_min,pixel_max);
				pixel_min  = pixel_min-pixelDataIn.m_hu_offset;
				pixel_max  = pixel_max-pixelDataIn.m_hu_offset;

				ww = pixel_max-pixel_min;
				wl = ww/2 + pixel_min;

			}
			

			float RescaleIntercept	= pixelDataIn.m_rescaleIntercept; 
			float  RescaleSlope		= pixelDataIn.m_rescaleSlope;

			if(wl == 0){
				wl = 1200;
			}
			if(ww == 0){
				ww = 3000;
			}
			float offset =   RescaleIntercept -(wl-ww/2.0)  ;
		//	offset += pixelDataIn.m_hu_offset;
			float gain = 256.0/ww;
			gain *=pixelDataIn.m_rescaleSlope;

			offset *= gain;

			if(pixelDataIn.m_pixelRepresentation == 1){

				short *p_pixel_src				= ( short *)pixelDataIn.m_pixelData;
				unsigned char *p_pixel_dest		= ( unsigned char *)pixelDataOut.m_pixelData;
				//
		///////////////
				
				for(int y_i=0;y_i<pixelDataIn.m_sizeY;y_i++){
					for(int x_i=0;x_i<pixelDataIn.m_sizeX;x_i++){
						int index_src	=  (y_i*pixelDataIn.m_sizeX + x_i);
						int index_dest	=  (y_i*pixelDataOut.m_line_alloc_sizeX + x_i);
						float f_temp = p_pixel_src[index_src]*gain + offset ;
						if(f_temp <0.0f) f_temp = 0.0f;
						if(f_temp >255.0f) f_temp = 255.0f;
						p_pixel_dest[ index_dest ] =  (unsigned char)(f_temp+0.5f);
						 
					}
				}

	//			m_Pixel_Gain	= 1.0f/gain ;
	//			m_Pixel_Offset  = -offset_val*m_Pixel_Gain;

			}else{
				
				unsigned short *p_pixel_src		= ( unsigned short *)pixelDataIn.m_pixelData;
				unsigned char *p_pixel_dest		= ( unsigned char *)pixelDataOut.m_pixelData;

			
				for(int y_i=0;y_i<pixelDataIn.m_sizeY;y_i++){
					for(int x_i=0;x_i<pixelDataIn.m_sizeX;x_i++){
						int index_src	=  (y_i*pixelDataIn.m_sizeX + x_i);
						int index_dest	=  (y_i*pixelDataOut.m_line_alloc_sizeX + x_i);

						float f_temp = p_pixel_src[index_src]*gain + offset;
					
						if(f_temp <0.0f) f_temp = 0.0f;
						if(f_temp >255.0f) f_temp = 255.0f;
						p_pixel_dest[ index_dest ] =  (unsigned char)(f_temp+0.5f);

						 
					}
				}
	//			m_Pixel_Gain	= 1.0f/gain ;
	//			m_Pixel_Offset  = -offset_val*m_Pixel_Gain-image->m_hu_offset;
			}
		}

	}
#if 0
	unsigned char *data_ptr = (unsigned char *)image_pixels;
		for(int y_i=0;y_i<pixelData.m_sizeY;y_i++){
				 
			unsigned char *org_data_line_ptr = data_ptr + y_i*pixelData.m_sizeX;
			unsigned char *dest_data_line_ptr = pixelData.m_pixelData + y_i*pixelData.m_line_alloc_sizeX;
			memcpy(dest_data_line_ptr,org_data_line_ptr,pixelData.m_sizeX*bpp);
			 
		}
#endif
	return true;
}

bool COutputJpegProc::setupImagePixel(CPxDicomImage *pDicom,PxImagePixel &pixelData,bool rawData)
{
	int sizeY = pDicom->GetNumberOfRows();
    int sizeX = pDicom->GetNumberOfColumns();
    int frames = pDicom->GetNumberOfFrames();

	//
	double RescaleIntercept = pDicom->GetRescaleIntercept();
	double RescaleSlope = pDicom->GetRescaleSlope();

	//
	double wl = pDicom->GetWindowCenter();
	double ww = pDicom->GetWindowWidth();
	//
	int BitsAllocated = pDicom->GetBitsAllocated();
    int BitsStored = pDicom->GetBitsStored();
    int HighBit = pDicom->GetHighBit();
    int SamplesPerPixel = pDicom->GetSamplesPerPixel();

 

	unsigned char*	image_pixels = pDicom->GetImagePixels();
	if(!image_pixels){
		return false;
	}
	// 
	int sliceSize = sizeX*sizeY;
	if(sliceSize <1) return false;
	pixelData.m_sizeX = sizeX;
	pixelData.m_sizeY = sizeY;
	int adjSamplePerPixel = SamplesPerPixel;
	if(adjSamplePerPixel == 3){
		adjSamplePerPixel = 4;
	}
	pixelData.m_samplesPerPixel = adjSamplePerPixel ;

	if(rawData){
		pixelData.m_bits				= BitsStored;
		pixelData.m_rescaleIntercept	= RescaleIntercept;
		pixelData.m_rescaleSlope		= RescaleSlope;
 
		pixelData.m_ww					= ww;
		pixelData.m_wl					= wl;
	 
		pixelData.m_pixelRepresentation	= pDicom->GetPixelRepresentation(); //0: unsigned short, 1: singned short
		if(kCT == pDicom->GetModality()){
			pixelData.m_hu_offset			= 1024.0f;
		}else{
			pixelData.m_hu_offset			= 0.0f;
		}
	}else{
		pixelData.m_bits				= 8;
		pixelData.m_rescaleIntercept	= 0.0f;
		pixelData.m_rescaleSlope		= 1.0f;
		pixelData.m_ww					= 255;
		pixelData.m_wl					= 128;
		//
		pixelData.m_pixelRepresentation	= 0; //0: unsigned short, 1: singned short
		pixelData.m_hu_offset			= 0.0f;
	}
	if(pixelData.m_pixelData){
		delete [] pixelData.m_pixelData;
		
	}
	int bpp = 1;
	if(pixelData.m_bits>8){
		bpp = 2;
	}
	int pixel_buff_len = sliceSize*adjSamplePerPixel*bpp;
 	pixelData.m_pixelData = new unsigned char[pixel_buff_len+2];
//	pixelData.m_pixelData = (unsigned char*)(new unsigned short[pixel_buff_len/2+1]);
	
	
	if(rawData){
		if(adjSamplePerPixel == 4){
			
			pixelData.m_hu_offset			= 0.0f;
			unsigned char *data_ptr = (unsigned char *)image_pixels;

			for(int i=0;i<sliceSize;i++){
					 
				int index = 3*i;
				int des_index = adjSamplePerPixel*i;
				 
				pixelData.m_pixelData[des_index + 2] = data_ptr[index + 0];
				 
				//+1
				 
				pixelData.m_pixelData[des_index + 1] = data_ptr[index + 1];
				 
				//+2
				 
				pixelData.m_pixelData[des_index + 0] = data_ptr[index + 2];
				 
				//+3
			// 	pixelData.m_pixelData[des_index + 3]  = sum_pixel/3;
			}
		}else{
			memcpy(pixelData.m_pixelData,image_pixels,pixel_buff_len);
			
		}
	}else{

		double offset;
		double gain ;
		if(BitsAllocated == 16){
			if(wl == 0){
				wl = 1200;
			}
			if(ww == 0){
				ww = 3000;
			}
			 offset =   RescaleIntercept -(wl-ww/2.0)  ;
			 gain = 256.0/ww;

			short *data_ptr = (short*)image_pixels;
			for(int i=0;i<sliceSize;i++){
				short s_temp = (RescaleSlope*data_ptr[i] + offset)*gain  ;
				if(s_temp<0) s_temp = 0;
				if(s_temp>255) s_temp = 255;
				pixelData.m_pixelData[i] = s_temp;
			}
			
		}else 
		if(BitsAllocated == 8){
			if(wl == 0){
				wl = 128;
			}
			if(ww == 0){
				ww = 256;
			}

			offset =   RescaleIntercept -(wl-ww/2.0)  ;
			 gain = 256.0/ww;

			unsigned char *data_ptr = (unsigned char *)image_pixels;
			 
			 
			if(adjSamplePerPixel == 4){
				
				for(int i=0;i<sliceSize;i++){
					 
					int index = 3*i;
					int des_index = adjSamplePerPixel*i;
					short s_temp = (RescaleSlope*data_ptr[index + 0] + offset)*gain  ;
					if(s_temp<0) s_temp = 0;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[des_index + 2] = s_temp;
					 
					//+1
					s_temp = (RescaleSlope*data_ptr[index + 1] + offset)*gain  ;
					if(s_temp<0) s_temp = 1;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[des_index + 1] = s_temp;
					 
					//+2
					s_temp = (RescaleSlope*data_ptr[index + 2] + offset)*gain  ;
					if(s_temp<0) s_temp = 1;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[des_index + 0] = s_temp;
					 
					//+3
				// 	pixelData.m_pixelData[des_index + 3]  = sum_pixel/3;
				}
			}else{
				for(int i=0;i<sliceSize;i++){
					short s_temp = (RescaleSlope*data_ptr[i] + offset)*gain  ;
					if(s_temp<0) s_temp = 0;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[i] = s_temp;
				}
			}
		}

	}
	return true;
}


