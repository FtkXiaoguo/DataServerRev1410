// TstCPxDicomImage.h: CTstVLIDicomImage クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTVLIDICOMIMAGE_H__210FAFE6_350B_4BFD_8A17_4DCF74BC69FC__INCLUDED_)
#define AFX_TSTVLIDICOMIMAGE_H__210FAFE6_350B_4BFD_8A17_4DCF74BC69FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstDicomBase.h"
#include "XSTRING"
using namespace std;

#ifdef USE_NEW_LIB
class CPxDicomImage;
#else
class VLIDicomImage;
#endif
class CTstVLIDicomImage  : public CTstDicomBase
{
class test_data_struct 
{
public:
	int uid;
	int dataType;
	int imageNumber;
	int dumy;
};
public:
	CTstVLIDicomImage();
	virtual ~CTstVLIDicomImage();

	static string getTestDataPatternFromStudyDesString() { return "Stduy[%d]" ;};
	static string getTestDataPatternFromSereisDesString() { return "Frame[%d] SeriesDes[%d] Data[%d]" ;};
 
	void embedTestPattern(int uid, int dataType, int imageNumber, unsigned char *Pixel, int PixelSize);
	bool createFromMessasgeID(int messageID);
	unsigned char*	GetImagePixels();
	bool checkImageData(int z_no,int &ref_dataType, int & ref_seriesUID,int &totalFrame);
	//
	bool loadDicom(const char *filename,bool bHeaderOnly=0 );

	void outputTAG(const char *outputFile);

	void openNewDicom();
	void saveDicom(const char *filename);
	void ConvertToFile(const char *filename);

	void setupImage(int sizeX,int sizeY, int FrameNo,unsigned char* iBuf,int bufSize=0);
	void prepareDICOM();
	//
//	int GetID() const ;
	//
#ifdef USE_NEW_LIB
	CPxDicomImage *getDicomImage() const ;
#else
	VLIDicomImage *getDicomImage() const ;
#endif

	void setPatientName(const string &name);
	void setPatientID(const string &id);
	void setStudyID(const string & studyID);
	void setStudyInstanceUID(const string &studyInstanceUID);
	void setSeriesInstanceUID(const string &seriesInstanceUID);
	void setImageNumber(int no);

	void setupSOPInstanceUID(const string &uid="");


	void setupCurDate(string studyDate,string seriesTime);

	void setupPitch(float x, float y,float z){m_pitchX=x;m_pitchY=y;m_pitchZ=z;};
	void setupRescale(float Intercept,float Slope, bool useRescale=true){
		m_RescaleIntercept = Intercept;
		m_RescaleSlope = Slope;
		m_useRescale = useRescale;
	}

	void setBits(int BitsStored,int HighBit,int BitsAllocated){
		m_BitsStored = BitsStored;
		m_HighBit = HighBit;
		m_BitsAllocated = BitsAllocated;
	}


	void setSeriesDescription(const char *des);
	void setStudyDescription(const char *des);
	void setReferringPhysician(const char *name);
	void setModality(int modality) { m_Modality = modality;};
protected:
	

	string m_InstanceCreatorUID;
	string m_SeriesInstanceID ;
	int		m_ImageNumber;
#ifdef USE_NEW_LIB
 
	CPxDicomImage *m_DicomImage;
#else
	VLIDicomImage *m_DicomImage;
 
#endif


	void setupDefault();
	float	m_pitchX;
	float	m_pitchY;
	float	m_pitchZ;
	//
	float m_RescaleIntercept ;
	float m_RescaleSlope  ;
	bool m_useRescale ;
	//
	int m_BitsAllocated;
	int m_BitsStored ;
	int m_HighBit;

	char	m_SeriesDescription[128];
	char	m_StudyDescription[128];
	char	m_ReferringPhysician[128];
	//
	int		m_Modality; // 0: CT, 1: PX
 

	//
	 
};

#endif // !defined(AFX_TSTVLIDICOMIMAGE_H__210FAFE6_350B_4BFD_8A17_4DCF74BC69FC__INCLUDED_)
