#pragma once

 
class AEItemData;

class CAddPushDicomRequest  
{
public:
	CAddPushDicomRequest(void);
	~CAddPushDicomRequest(void);
 
 
 
void setMyID( int id) { m_MyID = id;};
void setPriority(int p) { m_priority= p;};
void setLoopInterval(int intval){ m_loopInterval = intval;};

#if 0
bool pushDICOMStudy(const std::string AETitle,const std::string &studyUID);
bool pushDICOMSeries(const std::string AETitle,const std::string &studyUID);
bool pushDICOMSeries(const std::string AETitle,const std::string &studyUID,const std::string &seriesUID);
#else
//#48
bool pushDICOMStudy(const AEItemData *AETitle,const std::string &studyUID);
//bool pushDICOMSeries(const AEItemData *AETitle,const std::string &studyUID);
bool pushDICOMSeries(const AEItemData *AETitle,const std::string &studyUID,const std::string &seriesUID);
#endif
	
protected:
	 
	int m_priority;
	int m_loopInterval;
	 
	int m_MyID;
	
	std::string m_DBQueueName;
};
