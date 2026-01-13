#pragma once
#include "vlidicom.h"

#include "PxDicommessage.h"

#include "PxDicomStatus.h"

class QueryStudyInfo
{
public:
	std::string		m_patientName;

	std::string		m_patientID;
	std::string		m_patientSex;
	std::string		m_patientBirthDate;
	std::string		m_studyCount;

	std::string		m_studyUID;
	std::string		m_studyID;
	std::string		m_studyDate;
	std::string		m_studyTime;
	std::string		m_accessionNumber;
	std::string		m_modalitiesInStudy;
	std::string		m_studyDescription;
	std::string		m_referringPhysiciansName;

};
//
class QuerySeriesInfo
{
public:
	std::string		m_studyUID;
	std::string		m_seriesUID;
	std::string		m_seriesDate;	// yyyymmdd
	std::string		m_seriesTime;	// hhmmss+6
	std::string		m_seriesNumber;	// IS 12 max
	std::string		m_modality ;
	std::string		m_imageCount;
	std::string		m_seriesDescription;
	std::string		m_bodyPartExamined;

};
//
class QueryImageInfo
{
public:
	std::string		m_studyUID;
	std::string		m_seriesUID;
	std::string		m_SOPInstanceUID;
	
	std::string		m_instanceNumber;
};
//

class CDcmServTst :
	public VLIDicom
{
public:
	CDcmServTst(void);
	~CDcmServTst(void);
	void Init(void);
	 
	void dotst(int loopNN=1);

	void RetrieveImages(std::string studyUID, std::string seriesUID);
protected:
	void setupStudyQuery(const QueryStudyInfo &query);
	void setupSeriesQuery(const QuerySeriesInfo &query);
	void setupImageQuery(const QueryImageInfo &query);
	std::string m_localAETitle;
	//
	CPxDicomMessage			m_lastQuery;
	 
};
