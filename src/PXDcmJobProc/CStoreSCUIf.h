/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_STORESCU_INTERFACE_H
#define C_STORESCU_INTERFACE_H

 
//-----------------------------------------------------------------------------
class CStoreSCUIf
{
public:
	 
	virtual bool tryToClose(int seriesCompleteTimeout/*sec*/) = 0;
	virtual bool sendStudy(const std::string &AE,const std::string &StudyUID) = 0;
	virtual bool sendSeries(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID) = 0;
	virtual bool sendImage(const std::string &AE,const std::string &StudyUID,const std::string &SeriesUID,const std::string &SOPInstanceUID) = 0;
	virtual bool openCStoreAssociation(const std::string &AE,const std::string &SeriesUID) = 0;
	virtual bool closeCStoreAssociation(void) = 0;
	virtual bool initCStoreSCU() = 0;
	//
	virtual bool sendDicomFile(const std::string &fileName) = 0;
	
	//
	virtual std::string getAE() const = 0;
	virtual std::string getSeriesUID() const = 0;
	//
	virtual void destroy() = 0;
	virtual time_t getLastTime() = 0;
	virtual void setLastTime(time_t t) = 0;
	//
//	virtual DICOMStudy *getDicomStudyInfo() = 0;
//	virtual void setDicomStudyInfo(const DICOMStudy *info) = 0;
	virtual void setDicomStudyInfo(const std::string &patientNameRef,const std::string &patientIDRef,const std::string &patientBirthDateRef) = 0;
	//
	//１ブロック（Ｉｍａｇｅ）使用中、削除しないようにする
	virtual void openSession() = 0;
	virtual void closeSession() = 0;
	
protected:
 
///
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_STORESCU_INTERFACE_H
