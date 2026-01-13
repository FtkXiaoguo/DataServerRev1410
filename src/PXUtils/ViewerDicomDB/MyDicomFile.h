// MyDicomFile.h: CMyDicomFile クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYDICOMFILE_H__C6CB6447_C8AF_4283_B52C_A34B32363978__INCLUDED_)
#define AFX_MYDICOMFILE_H__C6CB6447_C8AF_4283_B52C_A34B32363978__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PxDicomimage.h"

//#include "SearchFolder.h"

class MyMediaCBinfo
{
public:

	MyMediaCBinfo() { init();}	// 09/26/2002 T.C. Zhao
 
	~MyMediaCBinfo() { if (fp) { fclose(fp); fp = 0; }}
	//
    FILE*         fp;
   // char          buffer[16*1024];
	char          buffer[4*1024];
    unsigned long bytesRead;
	unsigned long dataSize;
	int			  errCode;

	bool		  cancelFlag;
 
	void init() {
		fp = 0; dataSize = 0; errCode = 0;
		cancelFlag = false;
	}
 
}  ;

class SimpleDicomInfo
{
public:
	std::string m_studyUID;
	std::string m_patientName;
	std::string m_patientID;
	//
	std::string m_seriesUID;
	int	m_studyYear;
	int m_studyMon;
	int m_studyDay;
};

class CMyDicomFile : public CPxDicomImage
{
public:
	CMyDicomFile();
	virtual ~CMyDicomFile();

	int loadDicomHeader(const char *iFilePath);
	unsigned long m_pixelOffset;
	unsigned long  m_pixelSize ;
	MyMediaCBinfo & getMediaCBinfo() { return m_myMediaCBinfo;};
	bool initRegisterApp();

	bool getSimpleDicomInfo(SimpleDicomInfo & dicomInfo);
protected:
	int LoadHeaderEx(const char* iFilePath, int iKeepAsFile);
	int LoadHeaderEx(const char* iFilePath, unsigned long& oOffset, unsigned long& oSize);

	MyMediaCBinfo m_myMediaCBinfo;
};

#endif // !defined(AFX_MYDICOMFILE_H__C6CB6447_C8AF_4283_B52C_A34B32363978__INCLUDED_)
