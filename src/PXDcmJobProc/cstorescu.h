/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_STORESCU_H
#define C_STORESCU_H

#include "PxNetDB.h"
#include "RTVDiCOMStore.h"

#include "CStoreSCUIf.h"

#define Q_CSTORE_SERVICE_NAME "TIDICOM_QUEUE_SCU_Service_List"
//-----------------------------------------------------------------------------
class CStoreSCU  : public CStoreSCUIf
{
public:
	CStoreSCU (const DiCOMConnectionInfo &connectionInfo);
	
	~CStoreSCU();

	static void trimLeftRight(const std::string &str_in,  std::string &str_out);
	static void ReformatJapaneseDicom( const std::string &org, std::string &conv);
 

	//#8 2012/03/16 K.KO
	//ServiceList指定仕組みの追加
	static bool initCMoveServiceList();
	
	virtual bool tryToClose(int seriesCompleteTimeout/*sec*/);

	virtual bool sendStudy(const std::string &AE,const std::string &StudyUID);
	virtual bool sendSeries(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID);
	virtual bool sendImage(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,const std::string &SOPInstanceUID);
	virtual bool openCStoreAssociation(const std::string &AE,const std::string &SeriesUID);
	virtual bool closeCStoreAssociation(void);
	virtual bool initCStoreSCU();
	//
	virtual bool sendDicomFile(const std::string &fileName);

	virtual std::string getAE() const { return m_AE;} ;
	virtual std::string getSeriesUID() const { return m_SeriesUID;} ;
	//
	virtual void destroy() ;
	virtual time_t getLastTime(){ return m_lastTime;};
	virtual void setLastTime(time_t t){ m_lastTime = t;};

	//
//	virtual DICOMStudy *getDicomStudyInfo();
//	virtual void setDicomStudyInfo(const DICOMStudy *info) ;
	virtual void setDicomStudyInfo(const std::string &patientNameRef,const std::string &patientIDRef,const std::string &patientBirthDateRef);
 
	//
	//１ブロック（Ｉｍａｇｅ）使用中、削除しないようにする
	virtual void openSession() ;
	virtual void closeSession() ;
	///
	static bool initDICOM();
	
	
protected:
	 
	bool ReadResponse(int iMessageID,bool& oMessageWasSent);
	bool queryImages(const std::string studyUID,const std::string &seriesUID,std::vector<DICOMInstance> &image_list);
	DiCOMConnectionInfo  m_connectInfo;

	time_t m_lastTime;
	//
	std::string m_AE;
	std::string m_SeriesUID;
///
//	DICOMStudy m_DicomStudyInfo;
//	std::string m_patientNameRef;	//#142_NoPatientName_NoComment
	std::string m_patientIDRef;
	std::string m_patientBirthDateRef;
	bool	m_hasDicomStudyInfo;
	//
	bool m_sessionOpened;
	//
	TRCriticalSection *m_cs;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_STORE_H
