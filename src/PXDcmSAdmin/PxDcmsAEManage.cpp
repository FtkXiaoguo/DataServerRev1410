
#include "PxDcmsAEManage.h"

#include "PxNetDB.h"


///
#include <functional>
#include <algorithm>
#include <vector>
#include <iostream>

#include <QtCore/qstring.h>
#include <QChar>
CPxDcmsAEManage::CPxDcmsAEManage(void)
{
	m_isLocalAE = false;
}
CPxDcmsAEManage::~CPxDcmsAEManage(void)
{
	 
}

bool CPxDcmsAEManage::readLocalAEs()
{
	return true;
}
	 

static bool hasAE_inside(const std::string &aeName,const std::vector<ApplicationEntity> &cont)
{
	bool ret_val = false;
	int size = cont.size();
	for(int i=0;i<size;i++){
		std::string cur_name = cont[i].m_AEName;
		if(cur_name == aeName) {
			ret_val =true;
			break;
		}
	}
	return ret_val;
}


CPxDcmsAEManageLocal::CPxDcmsAEManageLocal(void)
{
	m_isLocalAE = true;
}
CPxDcmsAEManageLocal::~CPxDcmsAEManageLocal(void)
{
}

bool CPxDcmsAEManageLocal::queryAEs(AEsMap &map_ret ,bool storageOnly/*not used*/)
{
	CPxDB pxDb;
 
	AqString whereFilter;
 	 
	std::vector<ApplicationEntity> tmpData;
	int ret = pxDb.QueryApplicationEntity(CPxDB::kLocalAE, tmpData,0);
	if(ret != kOK) return false;

	map_ret.clear();
	int aes_size = tmpData.size();
	for(int i=0;i<aes_size; i++){
		std::pair<std::string,AEItemData> new_item;
		 
		std::string AEname = tmpData[i].m_AEName;
	//	AEItemData AEtemp;
		new_item.second.m_AEID			= tmpData[i].m_AEID;
		new_item.second.m_AEName		= AEname;
		new_item.second.m_AETitle		= tmpData[i].m_AETitle;
		new_item.second.m_HostName		= tmpData[i].m_hostName;
		new_item.second.m_IP			= tmpData[i].m_IPAddress;
		new_item.second.m_PortNum		= tmpData[i].GetPort();
		 

		new_item.first	= AEname;
		map_ret.insert(new_item);
	}
 
	return true;
}

bool CPxDcmsAEManageLocal::addAE(const AEItemData &ae,bool modify)
{
#if 0
	CPxDB pxDb;

	ApplicationEntity ae_temp;
	
	ae_temp.m_AEID = ae.m_AEID;//for modify

	strcpy(ae_temp.m_AEName,	ae.m_AEName.c_str());
	strcpy(ae_temp.m_AETitle,	ae.m_AETile.c_str());

	strcpy(ae_temp.m_hostName,	ae.m_HostName.c_str());
	strcpy(ae_temp.m_IPAddress,	ae.m_IP.c_str());
	//
	ae_temp.m_port		= ae.m_PortNum;
	ae_temp.m_level		= 1;
	ae_temp.m_priority	= 3;
	strcpy(ae_temp.m_description, "ae test");
	 
	std::vector<int> iGroupIDs;
	 
	int ret;
	if(modify){
		ret= pxDb.ModifyRemoteAE(ae_temp,iGroupIDs,
				ae.m_CanQRFromMe,//iQRAllowed, 
				ae.m_CanQR,//iQRSource, 
				ae.m_CanPushData//iStoreAE
				);
	}else{
		ret= pxDb.AddRemoteAE(ae_temp,iGroupIDs,
				ae.m_CanQRFromMe,//iQRAllowed, 
				ae.m_CanQR,//iQRSource, 
				ae.m_CanPushData//iStoreAE
				);
	}
	if(ret != kOK) return false;
#endif

	CPxDB pxDb;

	AqString sql;

	if(modify){
		sql.Format("UPDATE LocalAE "
				" SET AETitle='%s' ,"
				" HostName='%s' ,"
				" IPAddress='%s', "
				" Port = %d "
				" WHERE AEName = '%s' "
				,
				ae.m_AETitle.c_str(), ae.m_HostName.c_str(), ae.m_IP.c_str(), ae.m_PortNum,
				ae.m_AEName.c_str()
				 
				);
	}else{
		sql.Format("IF NOT EXISTS (SELECT id FROM LocalAE WHERE AEName='%s' OR ( AETitle='%s' AND HostName='%s' "
				" AND IPAddress='%s' AND Port = %d)) INSERT INTO LocalAE (AEName,AETitle, HostName, IPAddress, Port, Level, Priority, Description) "\
				" VALUES('%s','%s','%s','%s',%d, %d, %d,'%s')", 
				ae.m_AEName.c_str(),ae.m_AETitle.c_str(), ae.m_HostName.c_str(), ae.m_IP.c_str(), ae.m_PortNum,
				ae.m_AEName.c_str(), ae.m_AETitle.c_str(), ae.m_HostName.c_str(), ae.m_IP.c_str(), ae.m_PortNum, 
				1 ,//ae.m_level, 
				2 ,//ae.m_priority, 
				" " //ae.m_description); 
				);
 
	}

	SQA sqa;
	sqa.SetCommandText(sql);

	int retcd = pxDb.SQLExecuteBegin(sqa);

	pxDb.SQLExecuteEnd(sqa);

	if(retcd != kOK) {
		 
		return false;
	}


	return true;
}
 
bool CPxDcmsAEManageLocal::deleteAE(const std::string &aeName)
{
 
	CPxDB pxDb;

	AqUString ttt  ;
	 ttt.ConvertUTF8(  aeName.c_str());
	 
 	int ret = pxDb.DeleteLocalAE( aeName.c_str());

	if(ret != kOK) return false;
 
	return true;
}

/////////////
// remote AE
/////////////
CPxDcmsAEManageRemote::CPxDcmsAEManageRemote(void)
{
	m_isLocalAE = false;
}
CPxDcmsAEManageRemote::~CPxDcmsAEManageRemote(void)
{
}

bool CPxDcmsAEManageRemote::queryAEs(AEsMap &map_ret ,bool storageOnly  )
{
	if(storageOnly){
		return queryStorageAEs(map_ret);
	}

	int ret;
	CPxDB pxDb;
 
	AqString whereFilter;
	whereFilter.Format(" WHERE AEName= '%s'", "*");

	std::vector<ApplicationEntity> tmpData;
 
 	 ret = pxDb.QueryApplicationEntity(CPxDB::kRemoteAE, tmpData,0);
 	if(ret != kOK) return false;


	//StoreTarget
	std::vector<ApplicationEntity> tmpData_StoreTarget;
	ret = pxDb.QueryApplicationEntity(CPxDB::kStoreTargetAE, tmpData_StoreTarget,0);
//	if(ret != kOK) return false;

	//QRAllowed
	std::vector<ApplicationEntity> tmpData_QRAllowed;
	ret = pxDb.QueryApplicationEntity(CPxDB::kQRAllowedAE, tmpData_QRAllowed,0);
//	if(ret != kOK) return false;

	//QRSouce
	std::vector<ApplicationEntity> tmpData_QRSource;
	ret = pxDb.QueryApplicationEntity(CPxDB::kQRSourceAE, tmpData_QRSource,0);
//	if(ret != kOK) return false;

 
	map_ret.clear();
	int aes_size = tmpData.size();
	for(int i=0;i<aes_size; i++){
		std::pair<std::string,AEItemData> new_item;
		 
		std::string AEname = tmpData[i].m_AEName;
	//	AEItemData AEtemp;
		new_item.second.m_AEID			= tmpData[i].m_AEID;
		new_item.second.m_AEName		= AEname;
		new_item.second.m_AETitle		= tmpData[i].m_AETitle;
		new_item.second.m_HostName		= tmpData[i].m_hostName;
		new_item.second.m_IP			= tmpData[i].m_IPAddress;
		new_item.second.m_PortNum		= tmpData[i].GetPort();
		//
		new_item.second.m_CanPushData	= hasAE_inside(AEname,tmpData_StoreTarget)	? 1 : 0;
		new_item.second.m_CanQRFromMe	= hasAE_inside(AEname,tmpData_QRAllowed)	? 1 : 0;
		new_item.second.m_CanQR			= hasAE_inside(AEname,tmpData_QRSource)	? 1 : 0;
		//

		new_item.first	= AEname;
		map_ret.insert(new_item);
	}
	int ret_size = map_ret.size();
 
	return true;
}

bool CPxDcmsAEManageRemote::queryStorageAEs(AEsMap &map_ret)
{
	CPxDB pxDb;
	
	//StoreTarget
	std::vector<ApplicationEntity> tmpData_StoreTarget;
	int ret = pxDb.QueryApplicationEntity(CPxDB::kStoreTargetAE, tmpData_StoreTarget,0);
	if(ret != kOK) return false;


	map_ret.clear();
	int aes_size = tmpData_StoreTarget.size();
	for(int i=0;i<aes_size; i++){
		std::pair<std::string,AEItemData> new_item;
		 
		std::string AEname				= tmpData_StoreTarget[i].m_AEName;
	//	AEItemData AEtemp;
		new_item.second.m_AEID			= tmpData_StoreTarget[i].m_AEID;
		new_item.second.m_AEName		= AEname;
		new_item.second.m_AETitle		= tmpData_StoreTarget[i].m_AETitle;
		new_item.second.m_HostName		= tmpData_StoreTarget[i].m_hostName;
		new_item.second.m_IP			= tmpData_StoreTarget[i].m_IPAddress;
		new_item.second.m_PortNum		= tmpData_StoreTarget[i].GetPort();
		//
		new_item.second.m_CanPushData	=  1;
		new_item.second.m_CanQRFromMe	=  0;
		new_item.second.m_CanQR			=  0;
		//

		new_item.first	= AEname;
		map_ret.insert(new_item);
	}
	int ret_size = map_ret.size();

	return true;
}


bool CPxDcmsAEManageRemote::addAE(const AEItemData &ae,bool modify)
{
	CPxDB pxDb;

	ApplicationEntity ae_temp;
	
	ae_temp.m_AEID = ae.m_AEID;//for modify

	strcpy(ae_temp.m_AEName,	ae.m_AEName.c_str());
	strcpy(ae_temp.m_AETitle,	ae.m_AETitle.c_str());

	strcpy(ae_temp.m_hostName,	ae.m_HostName.c_str());
	strcpy(ae_temp.m_IPAddress,	ae.m_IP.c_str());
	//
	ae_temp.m_port		= ae.m_PortNum;
	ae_temp.m_level		= 1;
	ae_temp.m_priority	= 3;
	strcpy(ae_temp.m_description, "ae test");
	 
	std::vector<int> iGroupIDs;
	 
	int ret;
	if(modify){
		ret= pxDb.ModifyRemoteAE(ae_temp,iGroupIDs,
				ae.m_CanQRFromMe,//iQRAllowed, 
				ae.m_CanQR,//iQRSource, 
				ae.m_CanPushData//iStoreAE
				);
	}else{
		ret= pxDb.AddRemoteAE(ae_temp,iGroupIDs,
				ae.m_CanQRFromMe,//iQRAllowed, 
				ae.m_CanQR,//iQRSource, 
				ae.m_CanPushData//iStoreAE
				);
	}
	if(ret != kOK) return false;

	return true;
}
 
bool CPxDcmsAEManageRemote::deleteAE(const std::string &aeName)
{
	CPxDB pxDb;

	AqUString ttt  ;
	 ttt.ConvertUTF8(  aeName.c_str());
	 
 	int ret = pxDb.DeleteRemoteAE( aeName.c_str());

	if(ret != kOK) return false;
 
	return true;
}