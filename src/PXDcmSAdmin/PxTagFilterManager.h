#pragma once

#include "PxDcmDbManage.h"
 #include "PxDBUtil.h"

typedef std::vector<PxDBUtil::DB_ID_Name> DB_ID_NameList;
typedef std::vector<PxDBUtil::DB_ID_NameEx> DICOM_TAG_NameList;
 
#if 0
class RoutingPatternEntry
{
public:
	int m_routingPatternID  ;
    std::string m_storeTargetName  ;
    int m_storeTargetID  ;
    //private String m_localAEName = "";
    //private int m_localAEID = -1;
	std::string m_tagFilterName  ;
    int m_tagFilterID  ;
    int m_compressionMethod;
    int m_compressionFactor;
};
#endif

class CPxTagFilterManager : public CPxDcmDbManage
{
public:
	CPxTagFilterManager(void);
virtual ~CPxTagFilterManager(void);

static int getIDFromList(const DB_ID_NameList &list, const std::string &name);
static std::string getNameFromList(const DB_ID_NameList &list, int id);

	bool deleteTagFilter(const std::string &name);

	bool getCompareatorList(DB_ID_NameList &list);
	bool getTagNameList(DB_ID_NameList &list);
	bool getTagNameListEx(DICOM_TAG_NameList &list);//#49
	bool addTagName(unsigned long iTag,const std::string &name);//#49
	bool deleteTagName(unsigned long iTag );//#49

	bool getTagFilterNameList(DB_ID_NameList &list);
	bool getRoutingPattern(DB_ID_NameList &list);
//

	bool getTagFilterRules(std::vector<PxDBUtil::TagFilterRuleTagName> &oVal, int TagFilterID);
	
	//
	bool modifyRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries);
	bool insertRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries);
	bool getRoutingPattern(const std::string &name,std::vector<RoutingPatternEntry> &entries);
	//
	bool addRoutingSchedule(const std::string &name,long startDate=-1,long endDate=-1);
	bool deleteRoutingSchedule(const std::string &name);
	bool isRoutingOnSchedule(const std::string &name);
 
protected:
	 
};

 