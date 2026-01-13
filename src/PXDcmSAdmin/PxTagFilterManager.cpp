
#include "PxTagFilterManager.h"

//#include "PxNetDB.h"


///
#include <functional>
#include <algorithm>
#include <vector>
#include <iostream>

#include <QtCore/qstring.h>
#include <QChar>
CPxTagFilterManager::CPxTagFilterManager(void)
{
	 
}
CPxTagFilterManager::~CPxTagFilterManager(void)
{
	 
}
int CPxTagFilterManager::getIDFromList(const DB_ID_NameList &list, const std::string &name)
{
	int ret_id = -1;
	int size = list.size();
	for(int i=0;i<size;i++){
		if(list[i].m_name == name){
			ret_id =  list[i].m_ID;
			break;
		}
	}
	return ret_id ;
}
std::string CPxTagFilterManager::getNameFromList(const DB_ID_NameList &list, int id)
{
	std::string  ret_str ;
	int size = list.size();
	for(int i=0;i<size;i++){
		if(list[i].m_ID == id){
			ret_str =  list[i].m_name;
			break;
		}
	}
	return ret_str ;
}

bool CPxTagFilterManager::getCompareatorList(DB_ID_NameList &list)
{
	PxDBUtil::CPxDBUtil DbUtil;
	 
	return DbUtil.getIDListName(PxDBUtil::IdList_Comparator,list);
		 
}
bool CPxTagFilterManager::getTagNameList(DB_ID_NameList &list)
{
	PxDBUtil::CPxDBUtil DbUtil;
	 
	return DbUtil.getIDListName(PxDBUtil::IdList_DicomTag,list);
}
bool CPxTagFilterManager::getTagNameListEx(DICOM_TAG_NameList &list)
{
	PxDBUtil::CPxDBUtil DbUtil;
	 
	return DbUtil.getTagListName( list);
}
bool CPxTagFilterManager::addTagName(unsigned long iTag,const std::string &name)//#49
{
	PxDBUtil::CPxDBUtil DbUtil;
	 
	return DbUtil.addTagName( iTag, name);
}
bool CPxTagFilterManager::deleteTagName(unsigned long iTag )//#49
{
	PxDBUtil::CPxDBUtil DbUtil;
	return DbUtil.deleteTagName(iTag );
}

bool CPxTagFilterManager::getTagFilterNameList(DB_ID_NameList &list)
{
	PxDBUtil::CPxDBUtil DbUtil;
	 
	return DbUtil.getIDListName(PxDBUtil::IdList_TagFilter,list);
}
bool CPxTagFilterManager::getRoutingPattern(DB_ID_NameList &list)
{
	PxDBUtil::CPxDBUtil DbUtil;
	 
	return DbUtil.getIDListName(PxDBUtil::IdList_RoutingPattern,list);
}

bool CPxTagFilterManager::getTagFilterRules(std::vector<PxDBUtil::TagFilterRuleTagName> &oVal, int TagFilterID)
{
	PxDBUtil::CPxDBUtil DbUtil;
	AqString whereFilter;
	whereFilter.Format("WHERE F.TagFilterID = %d",TagFilterID);
	 
	if(kOK!=DbUtil.getTagFilterRulesWithTagName( oVal ,whereFilter)){
		return false;
	};
	int size = oVal.size();
	
	return true;
}

bool CPxTagFilterManager::modifyRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries)
{
	PxDBUtil::CPxDBUtil DbUtil;
 
	return DbUtil.modifyRoutingPattern(name, entries);
 
}
    //--------------------------------------------------------------------------
    //
bool CPxTagFilterManager::insertRoutingPattern(const std::string &name,const std::vector<RoutingPatternEntry> &entries)
{
	PxDBUtil::CPxDBUtil DbUtil;
 
	return DbUtil.insertRoutingPattern(name, entries);
 
}
    
bool CPxTagFilterManager::getRoutingPattern(const std::string &name,std::vector<RoutingPatternEntry> &entries)
{
	PxDBUtil::CPxDBUtil DbUtil;
 
	return DbUtil.getRoutingPattern(name,entries);
}
bool CPxTagFilterManager::addRoutingSchedule(const std::string &name,long startDate,long endDate)
{
	PxDBUtil::CPxDBUtil DbUtil;
 
	return DbUtil.addRoutingSchedule(name,startDate, endDate);
}
bool CPxTagFilterManager::deleteRoutingSchedule(const std::string &name)
{
	PxDBUtil::CPxDBUtil DbUtil;
 
	return DbUtil.deleteRoutingSchedule(name);
}
bool CPxTagFilterManager::isRoutingOnSchedule(const std::string &name)
{
	PxDBUtil::CPxDBUtil DbUtil;
 
	return DbUtil.isRoutingOnSchedule(name);
}