/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_SENDDICOM_PROC_H
#define C_SENDDICOM_PROC_H

#include "PxNetDB.h"
#include "RTVDiCOMStore.h"

 

#define Q_CSTORE_SERVICE_NAME "TIDICOM_QUEUE_SCU_Service_List"
//-----------------------------------------------------------------------------

class CPxQueueEntry;

class CSendDicomProcIf  
{
public:
	 
	virtual bool sendStudy(const CPxQueueEntry *entry) = 0;
	virtual bool sendSeries(const CPxQueueEntry *entry,bool getStudyInfo=true)= 0;
	virtual bool sendImage(const CPxQueueEntry *entry)= 0;
	virtual bool sendEntryFile(const CPxQueueEntry *entry,const std::string &seriesFolder,const std::vector<std::string> &ImageFileList)= 0;

	virtual void cancel(void) = 0;//#82 2014/09/29 K.Ko
};

class CStoreSCUIf;

class CSendDicomProc  : public CSendDicomProcIf
{
public:
	CSendDicomProc ();
	
	~CSendDicomProc();
	
	virtual void cancel(void) { m_cancelFlag = true;};//#82 2014/09/29 K.Ko
#if 0
	
	bool sendStudy(const std::string &AE,const std::string &StudyUID);
	bool sendSeries(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,bool getStudyInfo=true);
	bool sendImage(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,const std::string &SOPInstanceUID);
	bool sendEntryFile(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,
						const std::string &seriesFolder,const std::vector<std::string> &ImageFileList);
#else
	bool sendStudy(const CPxQueueEntry *entry);
	bool sendSeries(const CPxQueueEntry *entry,bool getStudyInfo=true);
	bool sendImage(const CPxQueueEntry *entry);
	bool sendEntryFile(const CPxQueueEntry *entry,const std::string &seriesFolder,const std::vector<std::string> &ImageFileList);

#endif

	CStoreSCUIf *openCStoreAssociation(const std::string &AE,const std::string &SeriesUID);
	bool closeCStoreAssociation(CStoreSCUIf *cstoreSCU);

	bool init();
	void setLocaleAE(const std::string &localeAE);
protected:
	bool initDICOM();
	 
	bool queryImages(const std::string studyUID,const std::string &seriesUID,std::vector<DICOMInstance> &image_list);
	bool querySeries(const std::string studyUID, std::vector<DICOMSeries> &series_list);
	//
	DiCOMConnectionInfo  m_connectInfo;
	//
	std::vector<DICOMStudy> m_curStudy;
	//
	CPxDcmDB *m_pDcmDB;
	bool m_cancelFlag;//#82 2014/09/29 K.Ko
///
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_SENDDICOM_PROC_H
