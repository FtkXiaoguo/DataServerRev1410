#pragma once

#include "PxDcmDbManage.h"
 
class StudyQuery 
{
public:
	std::string m_patientID;
	std::string m_patientName;
 	std::string m_studyDate;
	std::string m_studyDescription;
};
////////
class PxDicomInfor
{
public:
	 
	std::string		m_patientName;
	std::string		m_patientID;
	std::string		m_patientSex;
	std::string		m_patientBirthDate;
	std::string		m_studyCount;

	std::string		m_studyUID;
	std::string		m_studyID;	// SH 16 char max
	std::string		m_studyDate;	// can contain ranges yyyymmdd-yyyymmdd
	std::string		m_studyTime;
	std::string		m_accessionNumber;   // SH 16 max
	std::string		m_radiologistName;
	std::string		m_physicianName;
	std::string		m_modalitiesInStudy;
	std::string		m_studyDescription;
	std::string		m_seriesCount;
	std::string		m_imagesInStudy;
 
	std::string		m_seriesUID;
	std::string		m_seriesDate;	// yyyymmdd
	std::string		m_seriesTime;	// hhmmss+6
	std::string		m_seriesNumber;	// IS 12 max
	std::string		m_modality;
	std::string		m_imageCount;
	std::string		m_seriesDescription;

	std::string		m_bodyPartExamined;
	std::string		m_source;

	//
	std::string		m_KVP;
	std::string		m_ExposureTime;
	std::string		m_XRayTubeCurrent;
	//
	std::string		m_ManufacturerModelName;
	std::string		m_StationName;
	std::string		m_InstitutionName;
};
typedef std::vector<PxDicomInfor> PxDicomInforList;

class PxImageInfor
{
 
public:
std::string		m_studyInstanceUID;
std::string		m_seriesInstanceUID;
std::string		m_SOPInstanceUID;				
std::string		m_SOPClassUID;				
int				m_instanceNumber;			
std::string		m_imageTypeTokens;					

};

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
typedef std::vector<PxImageInfor> PxImageInforList;


class AEItemData;

class CPxDicomImage;
struct pRTVSDicomInformation;
class CPxDcmStudysManage : public CPxDcmDbManage
{
public:
	enum SaftyCheckTag {
		SaftyCheckTag_None = 0,
		SaftyCheckTag_PatientName  ,
		SaftyCheckTag_PatientID  ,
		SaftyCheckTag_PatientBirthDate  ,
	};
	CPxDcmStudysManage(const std::string &charSet);
virtual ~CPxDcmStudysManage(void);
 
	static void InitCharacterLib(void);
	 
	virtual bool queryStudys(const StudyQuery &query,PxDicomInforList &objs );

	virtual bool querySeries(const std::string &studyUID,PxDicomInforList &objs);

	virtual bool queryImages(const std::string &studyUID,const std::string &seriesUID,PxImageInforList &objs);

	virtual bool loadImage(const PxImageInfor &Image,PxImagePixel &pixelData,int &ret_saftyCheck,PxDicomInfor *dicom_info=0/*for safty check*/,bool rawData=false);//#27 2012/06/14
	virtual bool loadImage(const std::string dicomFile,PxImagePixel &pixelData,bool rawData=false,PxDicomInfor *dicom_info=0 ); 
	
	virtual std::string getImageFileName(const PxImageInfor &Image) const;

	virtual std::string getSeriesFolder(const std::string &studyUID,const std::string &seriesUID ) const;
	//
	virtual bool delSeries(const std::string &studyUID,const std::string &seriesUID);
	virtual bool delStudy(const std::string &studyUID);
	//
#if 0
	virtual bool pushDICOMSeries(const std::string AETitle,const std::string &studyUID,const std::string &seriesUID);
	virtual bool pushDICOMStudy(const std::string AETitle,const std::string &studyUID);
#else
	//#48
	//AEItemData
	virtual bool pushDICOMSeries(const AEItemData *AETitle,const std::string &studyUID,const std::string &seriesUID);
	virtual bool pushDICOMStudy(const AEItemData *AETitle,const std::string &studyUID);
#endif
protected:
	std::string m_CharacterSet;//#140_search_Japanese_JIS_UTF8
	bool setupImagePixel(CPxDicomImage *dicom,PxImagePixel &pixelData,bool rawData=false);
	void setupQueryFilter(const StudyQuery &query,pRTVSDicomInformation *filerQuery);
};
 