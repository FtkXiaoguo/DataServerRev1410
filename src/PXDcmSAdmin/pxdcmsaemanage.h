#pragma once

#include "PxDcmDbManage.h"
 
class AEItemData
{
public:
	AEItemData(){
		m_AEID = 0;
		m_PortNum = 0;
		m_CanPushData = 0;
		m_CanQR = 0;
		m_CanQRFromMe = 0;
	};
	int m_AEID;
	//
	std::string m_AEName;
	std::string m_AETitle;
	std::string m_HostName;
	std::string m_IP;
	int m_PortNum;
	//
	int	m_CanPushData;
	int m_CanQR;
	int m_CanQRFromMe;
	//

};

typedef std::map<std::string , AEItemData>  AEsMap;

class CPxDcmsAEManage : public CPxDcmDbManage
{
public:
	CPxDcmsAEManage(void);
virtual ~CPxDcmsAEManage(void);

	bool readLocalAEs();
	//
	 


	virtual bool addAE(const AEItemData &ae,bool modify=false) = 0;
	virtual bool deleteAE(const std::string &aeName) = 0;
	//
	virtual bool queryAEs(AEsMap &map,bool storageOnly=false )=0;
protected:
	bool	m_isLocalAE;
//	AEsMap m_MyAEList;
//	AEsMap m_RemoteAEList;
};

class CPxDcmsAEManageLocal : public CPxDcmsAEManage
{
public :
CPxDcmsAEManageLocal(void);
virtual ~CPxDcmsAEManageLocal(void);

	virtual bool addAE(const AEItemData &ae,bool modify=false);
	virtual bool deleteAE(const std::string &aeName);

virtual bool queryAEs(AEsMap &map,bool storageOnly=false );
};

class CPxDcmsAEManageRemote : public CPxDcmsAEManage
{
public :
CPxDcmsAEManageRemote(void);
virtual ~CPxDcmsAEManageRemote(void);
	virtual bool queryAEs(AEsMap &map,bool storageOnly=false );
///
	virtual bool addAE(const AEItemData &ae,bool modify=false);
	virtual bool deleteAE(const std::string &aeName);

protected:
	bool queryStorageAEs(AEsMap &map);
};