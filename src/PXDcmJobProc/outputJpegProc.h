/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_OUTPUT_JPEG_PROC_H
#define C_OUTPUT_JPEG_PROC_H

#include "SendDicomProc.h"

//#71 Chen {[(
struct JpegTagWriter
{
	enum ImageType{
		ImageTypeDental = 0,
		ImageTypePanoroma,
		ImageTypeCephalo,
		ImageTypeMouthImage,
		ImageType3DImage,
		ImageTypeTW_FMS,
		ImageTypeOther
	};

	struct UserTag{
		std::string strDataTime;
		std::string strSoftware;
		std::string strPatientID;
		std::string strStudyUID;
		std::string strSeriesUID;
		std::string strOriginalFolderName;
		std::string strOriginalFolderPath;
		ImageType   eImageType;
	};

	JpegTagWriter(std::string strModualPath);
	virtual ~JpegTagWriter();
	static bool IsModualExisted(std::string strModualPath);
	bool AddUserTag(const UserTag& tag, const std::string& strSrcJpg, std::string& strDstjpg, std::string& strCmd);
	std::string m_strYTWexe;
	std::string m_strWorkingFolder;
	std::string m_strOutputFolder;
	bool m_ActionGateForE2;//#73 2013/11/26 K.Ko
private:
	struct CriticalSection{
		CriticalSection(){
			::InitializeCriticalSection(&m_cs);
		}
		virtual ~CriticalSection(){
			::DeleteCriticalSection(&m_cs);
		}
		void Enter(){
			::EnterCriticalSection(&m_cs);
		}
		void Leave(){
			::LeaveCriticalSection(&m_cs);
		}
		CRITICAL_SECTION m_cs;
	};
	//
	
};
 
//#71 Chen )]}

//-----------------------------------------------------------------------------
class OutJPEG_Folder_Info
{
public:
	std::string m_ID;
	std::string m_ID_FolderName;
	std::string m_Exp_FolderName;
	std::string m_Thum_FolderName;
	std::string m_DaxInfo_FileName;
};
 
class CDeltaViewInfoEntry
{
public:
	enum ImageType {
		ImageType_O = 0	,	
		ImageType_D		,	
//		ImageType_P		,	
		ImageType_V		,	
		////
		ImageType_3D	,	//CT
		ImageType_P		,	//Panorama
		ImageType_C		,	//Cephalo
	};
#define DeltaViewInfoEntryDef_ImgID		"ImgID" 
#define DeltaViewInfoEntryDef_StudyUID	"ImgStudyUID"
#define DeltaViewInfoEntryDef_SeriesUID	"ImgSeriesUID" 
#define DeltaViewInfoEntryDef_ImgType	"ImgType" 
#define DeltaViewInfoEntryDef_ImgBui	"ImgBui" 
#define DeltaViewInfoEntryDef_ImgHani	"ImgHani"
#define DeltaViewInfoEntryDef_ImgDate	"ImgDate"
#define DeltaViewInfoEntryDef_ImgExt	"ImgExt"
 
	std::string m_ImgID;
	std::string m_StudyUID;
	std::string m_SeriesUID;
	std::string m_ImgType;
	std::string m_ImgBui;
	std::string m_ImgHani;
	std::string m_ImgDate;
	std::string m_ImgExt;

};

 
class TypeImageCount
{
public:
	TypeImageCount(){
		m_ImageCount = 0;
	}
	int m_ImageCount;
};
class CDeltaViewInfo
{
#define DeltaViewInfoEntryDef_KanjaNo	"KanjaNo"
#define DeltaViewInfoEntryDef_ImgCount	"ImgCount"
public:
	CDeltaViewInfo() {
		m_ImgCount = 0;
	 
	}
	~CDeltaViewInfo() {
		destroy();
	}
	void destroy(){
		m_ImageCountPerTypeRecord.clear();
		m_EntryList.clear();
		m_ImgCount = 0;
	}
	//
	std::map<std::string , TypeImageCount >	m_ImageCountPerTypeRecord;

	std::string m_KanjaNo;
 	int m_ImgCount;
	std::map<std::string/*ImgID*/,CDeltaViewInfoEntry> m_EntryList;
};

///////////////////

class PxImagePixel
{
public:
	PxImagePixel() {
		m_sizeX = m_sizeY = 0;
		m_pixelData = 0;
		m_samplesPerPixel = 0;
	}
virtual ~PxImagePixel() {
		if(m_pixelData) delete [] m_pixelData;
		m_pixelData = 0;
	}
	int m_line_alloc_sizeX;
	int m_sizeX;
	int m_sizeY;
	unsigned char *m_pixelData;
	int m_samplesPerPixel;
	int m_bits;
	float m_rescaleIntercept;
	float m_rescaleSlope;
	//
	float m_ww;
	float m_wl;
	//
	int m_pixelRepresentation; //0: unsigned short, 1: singned short
	float m_hu_offset;//-1024 when modality = CT
};
///////////////////
class CHistogramData;
class CDicomImageDataProc;
class COutputJpegProc   : public CSendDicomProcIf
{
public:
	COutputJpegProc ();
	
	~COutputJpegProc();
	

	bool sendStudy(const CPxQueueEntry *entry);
	bool sendSeries(const CPxQueueEntry *entry,bool getStudyInfo=true);
	bool sendImage(const CPxQueueEntry *entry);
	bool sendEntryFile(const CPxQueueEntry *entry,const std::string &seriesFolder,const std::vector<std::string> &ImageFileList);

	bool init();
	 
	virtual void cancel(void) { m_cancelFlag = true;};//#82 2014/09/29 K.Ko
protected:
	bool checkSeriesUID(const std::string &seriesUID);
	bool addDeltaViewEntry(CDeltaViewInfoEntry &new_entry/*output*/,CDeltaViewInfoEntry::ImageType type,const std::string &StudyUID,const std::string &SeriesUID,const std::string &Date,bool useSeriesUID=false);

	bool readDeltaViewInfo(const std::string &IniFileName);
	bool writeDeltaViewInfo(const std::string &IniFileName);
	static bool ParseEntry(const char *buff,CDeltaViewInfo &outInof);
	static bool updateImageCount(CDeltaViewInfo &outInfo,const std::string &imageID);
	static std::string genNewTypeID(CDeltaViewInfo &outInfo,const std::string &typeFirstChar);

	bool initDICOM();

	bool writeOutDICOM(const std::string &dicomFileName,bool is3DVolume=false);
	bool writeJPEG(const std::string &jpegFileName,const unsigned char *imageData,int sizeX,int sizeY,int writeSizeX,int writeSizeY);

	//#71 Chen
	bool writeJPEG(const std::string &jpegFileName,const unsigned char *imageData,int sizeX,int sizeY,int writeSizeX,int writeSizeY, const JpegTagWriter::UserTag* pTag);
	 
	bool queryImages(const std::string studyUID,const std::string &seriesUID,std::vector<DICOMInstance> &image_list);
	bool querySeries(const std::string studyUID, std::vector<DICOMSeries> &series_list);
	//

	bool checkPatientID(const std::string &ID,OutJPEG_Folder_Info &Info);
 	 
	//
	bool setupImagePixel(CPxDicomImage *pDicom,PxImagePixel &pixelData,bool rawData);
	bool dispImagePixel(const PxImagePixel &pixelDataIn, PxImagePixel &pixelDataOut,CHistogramData *makeHist=0);
	bool calImageHist(const PxImagePixel *image,CHistogramData *makeHist );

	//
	std::vector<DICOMStudy> m_curStudy;
	//
	CPxDcmDB *m_pDcmDB;

	std::string m_OutputJPEGHome;
	//
	CDeltaViewInfo m_DeltaViewInfo;
	//
	std::map<int,std::string> m_TypeFirstCharTable;
	std::map<int,std::string> m_TypeTable;
	std::map<int,std::string> m_ExtTable;
	//
	TRCriticalSection *m_cs;
	///
	CDicomImageDataProc *m_ImageProcH;
	//
	std::string m_JpegTempFolder;//#71

	bool m_cancelFlag;//#82 2014/09/29 K.Ko
///
};



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_OUTPUT_JPEG_PROC_H
