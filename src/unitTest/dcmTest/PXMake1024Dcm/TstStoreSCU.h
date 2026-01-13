// TstStoreSCU.h: CTstStoreSCU クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
#define AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstDicomBase.h"

#include "XSTRING"

using namespace std;

class CTstSrcData;
 
class CTstVLIDicomImage;
class CTstStoreSCU   : public CTstDicomBase
{
public:
	CTstStoreSCU();
	virtual ~CTstStoreSCU();
	static 	bool initClient();
 
	void setupAE();

	void setupPatientInfo(const char*PatientName, const char*PatientID);
	void setupDataInfo(const char*StudyID, const char*SeriesID);

	void initCount();
	bool doSeriesCount();
	bool sendStudyData(int sizeX=0,int sizeY=0,int sizeZ=0);
	bool sendStudyDataFromSrc();

	void sendDataContinue();

	bool openAssociation();
	bool closeAssociation();

	bool closeALL();

	bool SendImage(unsigned int dicomMsgID);

	bool loadOption(const char *fileName);

	bool procStudy(string studyFolder);
	bool procSeries(string SeriesFolder);
	bool procDicomFile(string DicomFile,int ImageNumber);
	bool procDicomImage(CTstVLIDicomImage *image,int ImageNumber=-1);
	//
	bool testReadWriteDicom();
protected:
	 
	void getCurStudyDate();
	void getCurSeriesTime();
	//
	void setupUID();
	bool beginNewStudy();
	void beginNewPatient();
	void beginNewSeries();
	void beginNewImage();

	void genStudyInstanceUID();
	void genSeriesInstanceUID();
	void genSOPInstanceUID();

	void genCurSeriesNN();
	void genCurStudyNN();

	string m_myTestUID; //for different UID
	string m_PatientName ;
	string m_PatientID ;
	string m_StudyID ;
	string m_SeriesID ;
//
	string m_DicomSvrHost;
	int m_DicomSvrPort;
	string m_DicomSvrAE;
	string m_MyAE;
	char  m_serviceList[100];
//
	int            m_applicationID;
	int			   m_associationID;
//
	int m_sizeX;
	int m_sizeY;
	int m_sizeZ;
//
	float m_pitchX;
	float m_pitchY;
	float m_pitchZ;
//
	int m_useRescale;
	float m_RescaleIntercept ;
	float m_RescaleSlope ;
//
	int m_Modality; // 0: CT, 1: PX
//
	string m_ReferringPhysician;
	string m_StudyDescription;
//
	int m_testDataType;
    int m_sendLoopNN;
    int m_seriesNN;
    int m_seriesNN_mode;
    int m_studyNN;
    int m_studyNN_mode;
///
	int m_sourceDataType;
	string m_sourceDataFolder;
	//
	int m_setPixelValue ;
///

	string m_CurPatientName ;
	string m_CurPatientID ;
	string m_CurStudyID ;
	string m_CurSeriesID ;
	unsigned long m_ImageCount;
	unsigned long m_PatientCount;
	unsigned long m_StudyCount;
	unsigned long m_SeriesCount;
//
	unsigned long m_CurStudyCount;
	unsigned long m_CurSeriesCount;

	//
	int m_ReopenAssociation;
//
	int m_CurSeriesNN;
    int m_CurStudyNN;
	int m_CurTestDataType;


//
		//
	string m_curStudyDate;
	string m_curSeriesTime;

///
CTstSrcData *m_TstSrcData;
//
	string m_logFileName;
//
	int m_EmbeddingCheckPattern;
/////
	string m_rootUID;
	string m_curStudyInstanceUID;
	string m_curSeriesInstanceUID;
	string m_curSOPInstanceUID;
	int m_useTimeTickCount ;
	//
	float m_spentTimeSum;
};

#endif // !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
