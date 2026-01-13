/***********************************************************************
 * PxNetDB.h 
 *
 *-------------------------------------------------------------------
 */

#ifndef	__PX_DBUTIL_h__
#define	__PX_DBUTIL_h__

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxDB.h"
#include "rtvsprotocol.h"
#include "PxDcmDataLib/include/PxDICOMInfo.h"
 
#define AUTO_ROUTING_NAME "AutoRoutingPat"
/////////////////////////////////////////////////////////////////////
namespace PxDBUtil
{
enum IdListName {
	IdList_RemoteAE = -1,
    IdList_StoreTargetAE = 1,
    IdList_QRSourceAE = 2,
    IdList_LocalAE = 3,
    IdList_LocalAEGroupAssignment = 4,
    IdList_QRSourceAEGroupAssignment = 5,
    IdList_RoutingTargetGroup = 6,
    IdList_RoutingPattern = 7,
    IdList_Schedule = 8,
    IdList_UserGroup = 9,
    IdList_TagFilter =10,
    IdList_DicomTag = 11,
    IdList_Comparator = 12,
    IdList_TagFilterGroupAssignment= 13,
    IdList_RoutingTarget = 14,
    IdList_Printer = 15,
    IdList_Organization = 16,
 //   IdList_Department = 17,
    IdList_Domain = 18,
    IdList_DataProcessJob = 19,
    IdList_DataProcessor = 20
};
/////////////////////////////////////////////////////////////////////

class DB_ID_Name {
public:
	int m_ID;
	std::string m_name;
};
class DB_ID_NameEx {
public:
	int m_ID;
	unsigned long m_TAG;
	std::string m_name;
};
class TagFilterRuleTagName
{
public:
	int m_filterID;
	std::string m_tagName;
	int m_comparatorID;
	std::string m_valStr;

};
class  CPxDBUtil : public CPxDB
{

public:
	 
	CPxDBUtil();
	virtual ~CPxDBUtil();
	bool getIDListName(IdListName TableName,std::vector<DB_ID_Name> &idname_list);
	bool getTagListName(std::vector<DB_ID_NameEx> &idname_list);//#49
	bool addTagName(unsigned long iTag,const std::string &name);//#49
	bool deleteTagName(unsigned long iTag );//#49

	bool insertIDListName(IdListName TableName,std::string nameVal);
	bool deleteIDListName(IdListName TableName,std::string nameVal);
	 
	bool getTagFilterRulesWithTagName(std::vector<TagFilterRuleTagName>& oVal, const char* iWhereFilter);
 
	//
	bool MakeTagFilter(const std::string &name, const std::string &description,const std::vector<TagRule> &rules,bool isAddNew);
	//
	bool deleteTagFilter(const std::string &name);
	//--------
	bool modifyRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries);
    //
	bool insertRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries);
	bool getRoutingPattern(const std::string &name,std::vector<RoutingPatternEntry> &entries);
	//
	bool addRoutingSchedule(const std::string &name,long startDate=-1,long endDate=-1);
	bool deleteRoutingSchedule(const std::string &name);
	bool isRoutingOnSchedule(const std::string &name);
};

}
#endif	/* __PX_DBUTIL_h__ */
