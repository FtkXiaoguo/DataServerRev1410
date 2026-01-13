// TstStoreSCU.h: CTstStoreSCU クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
#define AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

 

#include "XSTRING"

using namespace std;


#define MY_SERVICE_LIST "TstQRSCUServiceList"
 
class CTstQRSCU    
{
public:
	CTstQRSCU();
	virtual ~CTstQRSCU();

//	void setupAE();
 
//	void setupPatientInfo(const char*PatientName, const char*PatientID);
//	void setupDataInfo(const char*StudyID, const char*SeriesID);

	void initParam();
	void changeParam();
	void sendDataContinue();

	bool openAssociation();
	bool closeAssociation();

	bool sendQRCmd();
	bool sendQRSeriesCmd();
	bool sendQRImagesCmd();
	bool receiveQRRSP(int ReqMessageID=0);
	//
	bool sendQRCancel(int iMsgID); 
	//
	bool sendCMoveCmd();
	bool waitCMoveRSP();
	bool ProcessCSTOREAssociation();
	
//	bool loadOption(const char *fileName);

	virtual void updateStudyData() { } ;
protected:	

static bool SetValue ( int A_messageid, unsigned long A_tag,
                            const char *A_value, const char *A_default, 
                            bool A_required );
static bool GetValue ( int A_messageid, unsigned long A_tag, 
                            char *A_value, int A_size, char *A_default );

	bool setupStudyLevel(int A_messageid);
	bool setupSeriesLevel(int A_messageid);
	bool setupImagesLevel(int A_messageid);

	bool readStudyLevelRSP(int A_messageid,bool setMoveInstance=false);


	void getCurStudyDate();
	void getCurSeriesTime();
	//
 
	bool beginNewRequest();



 
	//
	string m_curStudyInstanceUID;
	string m_curSerieInstanceUID;
	string m_curImageInstanceUID;
	//search field
	string m_PatientName ;
	string m_PatientID ;
	string m_StudyID ;
	string m_SeriesID ;
	string m_StudyDate;
	string m_StudyTime;
//
	string m_DicomSvrHost;
	int m_DicomSvrPort;
	string m_DicomSvrAE;
	string m_MyAE;
	//for C-MOVE Destination
	string m_CMoveDestAE;
 

	char  m_serviceList[100];
//
	int            m_applicationID;
	int			   m_associationID;
//
//	int	  m_ReqMessageID;
	 
 
//
	int m_Modality; // 0: CT, 1: PX
//

    int m_sendLoopNN;
	int m_LoopInterval;
 

///

 
 
	unsigned long m_runCount;
	
	unsigned long m_RequestCount;
 

	//
	int m_ReopenAssociation;
//

//
		//
	string m_curStudyDate;
	string m_curSeriesTime;

//
	int	m_sendQRStudyCmd;
	int	m_sendQRSeriesCmd;
	int	m_sendQRImagesCmd;
	int	m_sendCMoveCmd;
	
};

#endif // !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
