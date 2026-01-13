#include "AppComUtil.h"

#include "AqCore/TRPlatform.h"
#include "PxNetDB.h"
#include "diskspacemanager.h"

#include "RTVRegistry.h"
//-----------------------------------------------------------------------------
//	Delete a series
int AppComUtil::DeleteSeries(CPxDB* ipDB, const char* iStudyInstanceUID, const char* iSeriesInstanceUID)
{
	int status1, status2;
	char strSQL[512];

	if (!ipDB || !iStudyInstanceUID || !iSeriesInstanceUID || !*iStudyInstanceUID || !*iSeriesInstanceUID)
		return -1;

	//	Delete the cache
	RTVDiskSpaceManager::DeleteCache(iSeriesInstanceUID, iStudyInstanceUID);

	//	Delete the original files
    std::string origDir = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom(iSeriesInstanceUID, iStudyInstanceUID);
	if (!origDir.empty() && origDir.length() >= 10)
	{
		TRPlatform::RemoveDirectory(origDir.c_str());
	}

	sprintf(strSQL, " DELETE PrivateData WHERE AuxSeriesUID = '%s'", iSeriesInstanceUID);
	status1 = ipDB->SQLExecute(strSQL);

	//	Delete database entries for the series.  Just need to delete seriesLevel.
	//	Stored procedure takes care of the rest.
	sprintf(strSQL, " DELETE SeriesLevel WHERE seriesInstanceUID = '%s'", iSeriesInstanceUID);
	status2 = ipDB->SQLExecute(strSQL);

	if (status1 != kOK)
		return status1;

	if (status2 != kOK)
		return status2;

	return kOK;
}


std::string AppComUtil::getSeriesFolder(const std::string & iStudyInstanceUID, const std::string & iSeriesInstanceUID,const char* iSOP)
{
	 
	std::string ret_str = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom(iSeriesInstanceUID, iStudyInstanceUID,iSOP);
	 

	return ret_str;
}
std::string AppComUtil::getStudyFolder(const std::string &iStudyInstanceUID,const std::string &iSeriesInstanceUID,const char* iSOP)
{
	std::string ret_str = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom("", iStudyInstanceUID);
	 

	return ret_str;
}

std::string AppComUtil::getDicomFileName(const std::string &seriesFolder,
										const std::string &iSOPInstanceUID,
										int imageNumber)
{
	char str_buff[120];
	 
	 
	std::string fileName  ;

	if(seriesFolder.size()>0){
		sprintf(str_buff,"%05d_%s.dcm", imageNumber ,iSOPInstanceUID.c_str());
		fileName = seriesFolder + str_buff;
	}else{
		;
	}

	return fileName;
}

void AppComUtil::startDiskSpaceManger()
{
	RTVDiskSpaceManager::StartupAll();
}

void AppComUtil::PxRemoveDirectory(const std::string folder)
{
	if (!folder.empty())
	{
		TRPlatform::RemoveDirectory(folder.c_str());
	}
}

void AppComUtil::PxRemoveAllDiskFiles(std::vector<std::string>& iAllSeries, const char* studyUID, int iKeepOrphaned)
{
	std::string origDir,file;
	int series_size = iAllSeries.size();
	for(int i=0;i<series_size;i++){
		std::string series_folder = getSeriesFolder(studyUID, iAllSeries[i],0);
 
		PxRemoveDirectory(series_folder);
		/* check if we have any orphaned files */
		//??
	}
	/* try remove the study folder */
	origDir = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom("", studyUID);
	_rmdir(origDir.c_str());
//	origDir = gServer.GetDirectoryToReadCacheFrom("",studyUID);
//	_rmdir(origDir.c_str());
}

static inline char *ToUpper(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = toupper(*s);
	return ret;
}

using namespace std;
bool AppComUtil::getMediaPointInfo(std::vector<UtilMediaPointInfo> &MpVector)
{
	MpVector.clear();

	 
	std::vector<AppComDevice> raidDevices;
	int rcode = AppComConfiguration::GetArchiveDevices (AppComConfiguration::gkRAIDType, raidDevices);
	if(rcode != RTVRegistry::kSuccess || raidDevices.size() < 1){
			return false;
	}

#if 1
	UtilMediaPointInfo new_Mp_item;

	__int64 freeSpace, availableSpace, totalSpace;

	char dev_name_buff[128+1];

	std::vector< string> driveNameList;
	int dev_count = 0;
	for (int i = 0; i < raidDevices.size(); i++)
	{
		string path = raidDevices[i].GetDirectPathToOriginalDICOMDataOnDevice();

		//ignore same drive
		{
			string dev_name = raidDevices[i].GetPathToDevice();
			ASTRNCPY(dev_name_buff,dev_name.c_str());
			{
				int str_len = strlen(dev_name_buff);
				if(str_len>=128) str_len= 128;
				for(int c_i=0;c_i<str_len;c_i++){
					if(dev_name_buff[c_i] == ':'){
						dev_name_buff[c_i+1] = 0;
						break;
					}
				}
			}
			dev_name = ToUpper(dev_name_buff);

			bool used_dev_name_flag= false;
			for(int j=0;j<driveNameList.size();j++){
				if(dev_name == driveNameList[j]){
					used_dev_name_flag = true;
					break;
				}
			}
			if(used_dev_name_flag) continue;
			driveNameList.push_back(dev_name);
	 
		}
		 
		if (GetDiskFreeSpaceEx(path.c_str(), (PULARGE_INTEGER)& availableSpace, (PULARGE_INTEGER)& totalSpace, (PULARGE_INTEGER)& freeSpace) == 0)
		return false;
	
		//OK
 		
		new_Mp_item.m_name	= dev_name_buff;
		new_Mp_item.m_total	= totalSpace  / ( 1024*1024); //MByte
		new_Mp_item.m_free	= freeSpace  / ( 1024*1024);
		MpVector.push_back(new_Mp_item);
		dev_count++;

	}	
 
 
#endif
	return true;
}

/*
* LicenseManager::ConvertErrorCodeToString error ?
*/
//	Convert LicenseStatus to string
#include "pxlicensemanagerif.h"


std::string AppComUtil::ConvertErrorCodeToString(int iStatus)
	{
		std::string statusStr;
		switch(iStatus)
		{

		case LicenseManager::kLMLicenseNoHASP:
			statusStr = "No HASP";
			break;
		case LicenseManager::kLMHaspException:
			statusStr = "HASP Exception";
			break;
		case LicenseManager::kLMLicenseDisuse:
			statusStr = "Disuse";
			break;
		case LicenseManager::kLMAmbiguousLicense:
			statusStr = "Ambiguous License";
			break;
		case LicenseManager::kLMNoLicense:
			statusStr = "No License";
			break;
		case LicenseManager::kLMLicenseDisabled:
			statusStr = "Disabled";
			break;
		case LicenseManager::kLMLicenseExpired:
			statusStr = "Expired";
			break;
		case LicenseManager::kLMLicenseValid:
			statusStr = "Valid";
			break;
		case LicenseManager::kLMLicenseWillExpire:
			statusStr = "WillExpire";
			break;
		case LicenseManager::kLMUndefined:
		default:
			statusStr = "Unknown Status";
			break;
		};

		return statusStr;

	}

//#33 HIFチェックの追加 2012/09/06 K.Ko

extern "C"
{
typedef void (*GETAPPID_TFUNC)(unsigned int &data1,unsigned int &data2,bool update);
 
typedef bool (*CHK_TFUNC)(unsigned int &data1,unsigned int &data2,const char *buff,int buff_size);
 
}
int AppComUtil::ChkHifDrv(char *msg,int size)
{
	char str_buff[1024]={ 0,};
#ifdef _DEBUG
	char *dll_name = "HifCheckerD.dll";
#else
	char *dll_name = "HifChecker.dll";
#endif

	int ret_b = ChkHifDrv_NoneDll_Error;

	HMODULE dll_hd = LoadLibrary(dll_name);

	if(!dll_hd ){
		sprintf(msg,"LoadLibrary %s error",dll_name);
		return ret_b;
	}
	

	GETAPPID_TFUNC getAppID_func;
	CHK_TFUNC Chk_func;

	getAppID_func	= (GETAPPID_TFUNC)GetProcAddress( dll_hd, "getAppID" );
	Chk_func		= (CHK_TFUNC)GetProcAddress( dll_hd, "checkHifDrv" );
	if( (getAppID_func != NULL) && (Chk_func != NULL ) )
	{
		unsigned int org_data1;
		unsigned int org_data2;
		unsigned int verify_data1;
		unsigned int verify_data2;

		getAppID_func(org_data1, org_data2, true/*update*/);

		char msg_buff[64];
		bool chk_ret_b = Chk_func( verify_data1,verify_data2,msg_buff,64  );
		if(!chk_ret_b){
			ret_b = ChkHifDrv_ChkDrv_Error;
		//	printf("checkHifDrv error \n");
			sprintf(str_buff,"checkHifDrv error ");
		}else{
			ret_b = ChkHifDrv_OK;
			
			sprintf(str_buff,"checkHifDrv %s ",msg_buff);
			//printf("checkHifDrv ver %s \n",msg_buff);
		}

		if(ret_b == ChkHifDrv_OK){
			unsigned int verify_data1_temp;
			unsigned int verify_data2_temp;
			verify_data1_temp = ~org_data1;
			verify_data2_temp = verify_data1_temp | org_data2;
			if( (verify_data1_temp == verify_data1 ) &&
				(verify_data2_temp == verify_data2 ) ){

					ret_b = ChkHifDrv_OK;
					//printf("verify OK  \n");
			}else{
				ret_b = ChkHifDrv_Verify_Error;
				sprintf(str_buff,"checkHifDrv Verify Error" );
				 
				//printf("verify error  \n");
			}
		}
	}else{
		ret_b = ChkHifDrv_NoneDll_Error;
		//printf("GetProcAddress error \n");
		sprintf(str_buff,"GetProcAddress error");
	}

	strncpy(msg,str_buff,size);

	return ret_b;
}