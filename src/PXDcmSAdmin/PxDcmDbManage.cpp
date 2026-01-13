


#include "QtHelper.h"

#include "PxDcmDbManage.h"

#include "aqCore/TRCryptor.h"

#include "PxNetDB.h"
#include "AppComUtil.h"
#include "PxDICOMUtil.h"
///
#include <functional>
#include <algorithm>
#include <vector>
#include <iostream>

#include <QtCore/qstring.h>
#include <QChar>
CPxDcmDbManage::CPxDcmDbManage(void)
{
	 
}
CPxDcmDbManage::~CPxDcmDbManage(void)
{
	 
}
bool CPxDcmDbManage::initDB()
{
	int ret_b = false;
 
		{
#if 1
 
			TCHAR  MyComputerName[256];
			unsigned long nSize = sizeof(MyComputerName)-1 ;
			::GetComputerName(MyComputerName, &nSize);
			std::wstring dbServerNameTemp = MyComputerName;
			dbServerNameTemp = dbServerNameTemp;// + _T("\\SQLEXPRESS");

			 
			QString qstr_temp = WStr2QString(dbServerNameTemp) ;
		 
	 
			std::string dest  = QString2Str(qstr_temp) + "\\SQLEXPRESS"; ;
			CPxDcmDB::InitDBServerName(dest.c_str());
#else
			CPxDcmDB::InitDBServerName("MONE\\SQLEXPRESS");
#endif

			ret_b = CPxDcmDB::InitDatabaseInfo();
			
		} 

	 
	 
	return ret_b;
}

#include "JISToSJISMS.h"
void CPxDcmDbManage::ReformatJapaneseDicom(const std::string  &iOrg, std::string  &oConv )
{
	oConv = iOrg;
	 
	{
		CJISToSJISMS::ConvertJISToSJIS( iOrg, oConv );
		CJISToSJISMS::ReformatPatientName( oConv, cPNStandard );
	}
	return;
}

void
CPxDcmDbManage::ConvertSJToJCodeOnly(const std::string  &iOrg, std::string  &oConv )
{
	oConv = iOrg;
	//if ( gEM.IsJapaneseLocale())
	{
		CJISToSJISMS::ConvertSJToJCodeOnly( iOrg, oConv );
	}
	return;
}

//#62 2013/07/30
void
CPxDcmDbManage::ConvertSJToJCodeOnlyForSQL(const std::string  &iOrg, std::string  &oConv )
{
	oConv = iOrg;
	//if ( gEM.IsJapaneseLocale())
	{
		CJISToSJISMS::ConvertSJToJCodeOnlyForSQL( iOrg, oConv );
	}
	return;
}


void CPxDcmDbManage::initDispSpaceManager()
{
	AppComUtil::startDiskSpaceManger();
}
void CPxDcmDbManage::initDcmtk()
{
	TRDICOMUtil::InitialDICOM("mydcmtk");
}

std::string CPxDcmDbManage::getDBSeriesFolder(const std::string &studyUID,const std::string &seriesUID,const char *SOPInstanceUID)
{
	char *iPattern = 0;
	char spec[80];

	if(SOPInstanceUID){
		 
		sprintf(spec, "*%s.dcm", SOPInstanceUID);
		spec[sizeof spec - 1] = '\0';
		//
		iPattern = spec;
	}

	std::string fileName = AppComUtil::getSeriesFolder(studyUID ,seriesUID,iPattern);

	return fileName;

}

int	CPxDcmDbManage::DeletePrivateSeries(const char* seriesUID)
{
	 
	CPxDB pxDb;
	SQA sqa;

	std::vector<std::string> sops;
	int ret = kOK;

	sqa.FormatCommandText("DELETE PrivateData WHERE AuxSeriesUID='%s'", seriesUID);
	
	ret = pxDb.SQLExecuteBegin(sqa);
	 
	pxDb.SQLExecuteEnd(sqa);

	return ret  ;
}


void CPxDcmDbManage::RemoveSeriesFromDisk(const char* iSeriesUID, const char* iStudyUID, std::string& originalDir)
{
	originalDir = getDBSeriesFolder(iStudyUID,iSeriesUID);

	AppComUtil::PxRemoveDirectory(originalDir);
 
}

void CPxDcmDbManage::RemoveAllDiskFiles(std::vector<std::string>& iAllSeries, const char* studyUID, int iKeepOrphaned)
{
	AppComUtil::PxRemoveAllDiskFiles(iAllSeries, studyUID, iKeepOrphaned);
}

bool CPxDcmDbManage::verifyUserAccount(const std::string &userName, const std::string &passwd)
{
 
	bool ret_b =false;

	CPxDB pxDb;

	UserAccount  oUserAccount;
	int status  = pxDb.GetAqNETUserAccount(userName.c_str(),  oUserAccount);
 
	if(kOK !=status){
		return false;
	}

	if (oUserAccount.m_username[0] == '\0'){
		return false;
	}
	if (oUserAccount.m_password[0] == '\0'){
		return false;
	}
 

	if ( _stricmp(oUserAccount.m_status,"DISABLED") == 0){
		return false;
	}
		 

	if (pxDb.IsPasswordExpired(oUserAccount.m_accountID)){
		return false;
	}
		 

	TRCryptor encryptor;
	
	const char* encryptedPassword = encryptor.EncryptPassword(passwd.c_str());
	if(encryptedPassword==0){
		return false;
	}
	if(strlen(encryptedPassword)<1){
		return false;
	}
 
	std::string db_passwd = std::string(oUserAccount.m_password);
	
	ret_b = db_passwd == std::string(encryptedPassword);
	 
	return ret_b;
}

 
bool CPxDcmDbManage::getMediaPointInfo(std::vector<MediaPointInfo> &MpList)
{
	std::vector<UtilMediaPointInfo>  Util_MpVector;
	Util_MpVector.clear();
	if(AppComUtil::getMediaPointInfo(Util_MpVector)){
		MediaPointInfo new_item;
		int size = Util_MpVector.size();
		for(int i=0;i<size;i++){
			new_item.m_name		= Util_MpVector[i].m_name;
			new_item.m_total	= Util_MpVector[i].m_total;
			new_item.m_free		= Util_MpVector[i].m_free;
			//
			MpList.push_back(new_item);
		}
	}

	return true;
}

std::string CPxDcmDbManage::getTagName(unsigned long iTag)//#49
{
	 
	std::string tagName;
	TRDICOMUtil::GetTagName( iTag, tagName);
 

	return tagName;
}
