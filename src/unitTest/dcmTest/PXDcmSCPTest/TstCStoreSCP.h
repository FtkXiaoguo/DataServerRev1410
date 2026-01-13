// TstStoreSCU.h: CTstStoreSCU クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
#define AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstDicomBase.h"

#include "rtvthread.h"

#include "AssociationHandler.h"


#include "XSTRING"

using namespace std;


 
class CTstVLIDicomImage;
class CTstCStoreSCP   : public CTstDicomBase, public RTVThreadManager<CAssociationHandler>
{
public:
	CTstCStoreSCP();
	virtual ~CTstCStoreSCP();

//	void setupAE();
	void setupHomeFolder(const std::string &folder) { m_homeFolder = folder;};

	void setupPatientInfo(const char*PatientName, const char*PatientID);
	void setupDataInfo(const char*StudyID, const char*SeriesID);

	void receiveDataContinue();

	bool initCStoreSCP();

	bool ProcessCSTOREAssociation();
	bool WaitCSTOREAssociation(bool &runFlag);

	bool loadOption(const char *fileName);

	bool closeAll();
	bool waitAll();
protected:	
  

	void cleanup();

	ThreadsMap	m_threadMap;

//	bool setupStudyLevel(int A_messageid);


	void getCurStudyDate();
	void getCurSeriesTime();
	//



 
//
	string m_DicomSvrHost;
	int m_DicomSvrPort;
	string m_DicomSvrAE;

	//for C-MOVE Destination
	string m_CMoveDestAE;
 
//	int			   m_associationID;

	/////
	char  m_serviceList[100];
//
	int     m_calledApplicationID;
	int		m_waitStoreAssociationID;
	 int   m_storeMessageID;

//
	string m_MyAE;
	int m_ListenPort;
 
	string m_logFileName;
//
	int m_Modality; // 0: CT, 1: PX
//

    int m_sendLoopNN;
	int m_LoopInterval;
	int m_UseCMoveFlag;
///

	int m_checkPixelData;
	//
	int	m_writeFileFlag;
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
	bool m_runFlag;

	std::string m_homeFolder;
};

#endif // !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
