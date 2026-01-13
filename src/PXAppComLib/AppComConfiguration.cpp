/*************************************************************************
 *
 *---------------------------------------------------------------------
 *	Copyright (c) PreXion , 2001. All rights reserved.
 *
 *	PURPOSE:
 *		Member of the AppComConfiguration Class 
 *
 *
 ************************************************************************/
#pragma warning (disable: 4786)
#include "AppComConfiguration.h"
#include "PxDB.h"
#include "RTVRegistry.h"

//-----------------------------------------------------------------------------
const std::string AppComConfiguration::gkRAIDType = "RAID";
const std::string AppComConfiguration::gkDVDType  = "DVD";


static const HKEY kDefaultKey       = HKEY_LOCAL_MACHINE;
//static const char* kDefaultLocation = "Software\\TeraRecon\\Aquarius\\AQNet\\1.0"; //#46
//static const char* kDefaultAQCommonLocation = "Software\\TeraRecon\\Aquarius\\Common";//#46
 static const char* kDefaultSNMPMonitorLocation = "SYSTEM\\CurrentControlSet\\Services\\AqSNMPMonitor";

static int kMaxNumberOfDICOMDataLocations = 16;
static int kMaxNumberOfArchiveDrives      = 16;


//#7 2012/03/08 K.Ko
		
static std::string _Home_folder;

void AppComConfiguration::setHomeFolder(const std::string &home_dir){
	_Home_folder = home_dir;
};

//-----------------------------------------------------------------------------
void AppComDevice::Clear ()
{
    m_label = "";
    m_type  = "";
    m_path  = "";

    m_pathToCacheOnDevice = "";
    m_pathToOriginalDICOMDataOnDevice = "";
}

//-----------------------------------------------------------------------------
// Get the Product Type String from the Registry
int AppComConfiguration::GetProductType (std::string& oType)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

	oType = "";
	int status = rtvr.GetRegistryKey ("ProductType", oType);

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value
		oType = "AQNetDevelopment";
	}

	return status;
}

//-----------------------------------------------------------------------------
// Get the Product Version string from the Registry
int AppComConfiguration::GetProductVersion (std::string& oVersion)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

	oVersion = "";
	int status = rtvr.GetRegistryKey ("ProductVersion", oVersion);

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value
		oVersion = "0.99";
	}

	return status;
}

//-----------------------------------------------------------------------------
//
int AppComConfiguration::SetSNMPMonitorStartupType (int iValue)
{
	RTVRegistry rtvr (kDefaultKey, kDefaultSNMPMonitorLocation);

 	int status = rtvr.SetRegistryKey ("Start", iValue);
 
	return status;
}

 //-----------------------------------------------------------------------------
//
 int AppComConfiguration::GetSNMPMonitorStartupType (int& oValue)
 {
	RTVRegistry rtvr (kDefaultKey, kDefaultSNMPMonitorLocation);
	int value = 0;
    
	int status = rtvr.GetRegistryKey ("Start", value);

	if (status != RTVRegistry::kSuccess)
	{
		 value = 0;
	}
 
    oValue = value;
	return status;
 }


/* GL 2006-4-7 change to watermark per disk in DB

//-----------------------------------------------------------------------------
// Get the DICOM data location from the Registry
int AppComConfiguration::GetDiskHighWaterMark (int& oSpaceInMB)
{
	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);

    int spaceInMB = 10000;

	int status = rtvr.GetRegistryKey ("DiskHighWaterMark", spaceInMB);

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value = 10 GB
		spaceInMB = 10000;
	}
 

    oSpaceInMB = spaceInMB;
	return status;
}

//-----------------------------------------------------------------------------
// Get the DICOM data location from the Registry
int AppComConfiguration::SetDiskHighWaterMark (int iSpaceInMB)
{
	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);

 	int status = rtvr.SetRegistryKey ("DiskHighWaterMark", iSpaceInMB);
 
	return status;
}

//-----------------------------------------------------------------------------
// Get the DICOM data location from the Registry
int AppComConfiguration::GetDiskLowWaterMark (int& oSpaceInMB)
{
	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);

    int spaceInMB = 500;

	int status = rtvr.GetRegistryKey ("DiskLowWaterMark", spaceInMB);

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value = 500 MB
		spaceInMB = 500;
	}
 

    oSpaceInMB = spaceInMB;
	return status;
}

//-----------------------------------------------------------------------------
// Get the DICOM data location from the Registry
int AppComConfiguration::SetDiskLowWaterMark (int iSpaceInMB)
{
	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);

	int status = rtvr.SetRegistryKey ("DiskLowWaterMark", iSpaceInMB);
 
	return status;
}
*/

//-----------------------------------------------------------------------------
// Get the ArchiveDrives from the Registry
int AppComConfiguration::GetArchiveDevices (const std::string& iType, std::vector<AppComDevice>& oDevices)
{
    std::string value;
    AppComDevice device;
	bool pathSet = false;

    oDevices.clear ();
	
	CPxDB::InitMediaPoints(true);
	// -- 2007.05.07 Reference is not safe as c_mediaPoints can be reset by other threads
//	const std::vector<MediaPoint>& mpList = CPxDB::GetMediaPoints();
	const std::vector<MediaPoint> mpList = CPxDB::GetMediaPoints();
	int mpSize = mpList.size();
	if(mpSize < 1)
	{
		// Error
		return -1;
	}

	for (int i = 0; i < kMaxNumberOfArchiveDrives; i++)
    {
        device.Clear ();
		pathSet = false;

		if(i < mpSize)
		{
			device.m_type = mpList[i].m_mediaType;
			device.m_path = mpList[i].m_mediaPoint;
			device.m_label = mpList[i].m_mediaLabel;
            if (!device.m_path.empty() && device.m_type == iType)
            {
                // Path is valid so fill in the other entries
                device.m_pathToOriginalDICOMDataOnDevice = device.m_path + device.m_label;
                device.m_pathToOriginalDICOMDataOnDevice += "/"; // Add a trailing slash

                device.m_pathToCacheOnDevice = device.m_path;
                device.m_pathToCacheOnDevice += "AQNetCache/";

                oDevices.push_back (device);
            }

		}
        
	
		if (pathSet)
        {
            if (device.m_type == iType)
            {
                // Path is valid so fill in the other entries
                device.m_pathToOriginalDICOMDataOnDevice = device.m_path + device.m_label;
                device.m_pathToOriginalDICOMDataOnDevice += "/"; // Add a trailing slash

                device.m_pathToCacheOnDevice = device.m_path;
                device.m_pathToCacheOnDevice += "AQNetCache/";

                oDevices.push_back (device);
            }
        }
        
    }

	return RTVRegistry::kSuccess;
 
#if 0
    int status = RTVRegistry::kSuccess;
    

	char loc[64];
    for (int i = 0; i < kMaxNumberOfArchiveDrives; i++)
    {
        device.Clear ();
        pathSet = false;

        value = "";
        sprintf (loc, "%s\\DriveList\\%02X", kDefaultAQCommonLocation, i);
      
        RTVRegistry rtvr (kDefaultKey, loc);
        
	    status = rtvr.GetRegistryKey ("Type", value);
        if (status == RTVRegistry::kSuccess)
        {
            // Only push the string if it does have a valid value.
            if (!value.empty())
            {
                device.m_type = value;
            }
        }
        
        value = "";
        status = rtvr.GetRegistryKey ("Path", value);
        if (status == RTVRegistry::kSuccess)
        {
            // Only push the string if it does have a valid value.
            if (!value.empty())
            {
                device.m_path = value;
                pathSet = true;

                // Make sure you always have a trailing slash
                if (strncmp (&device.m_path[device.m_path.length()-1], "/", 1) != 0)
                    device.m_path += "/";
            }
        } 

        value = "";
        status = rtvr.GetRegistryKey ("Label", value);
        if (status == RTVRegistry::kSuccess)
        {
            // Only push the string if it does have a valid value.
            if (!value.empty())
            {
                device.m_label = value;
            }
        } 


        value = "";
        status = rtvr.GetRegistryKey ("Drive", value);
        if (status == RTVRegistry::kSuccess)
        {
            // Only push the string if it does have a valid value.
            if (!value.empty())
            {
                device.m_drive = value[0];
            }
        } 
        
		
		if (pathSet)
        {
            if (device.m_type == iType)
            {
                // Path is valid so fill in the other entries
                device.m_pathToOriginalDICOMDataOnDevice = device.m_path + device.m_label;
                device.m_pathToOriginalDICOMDataOnDevice += "/"; // Add a trailing slash

                device.m_pathToCacheOnDevice = device.m_path;
                device.m_pathToCacheOnDevice += "AQNetCache/";

				device.m_directPathToCacheOnDevice = device.m_drive;
				device.m_directPathToCacheOnDevice += ":/AQNetCache/";

				device.m_directPathToOriginalDICOMDataOnDevice = device.m_drive;
				device.m_directPathToOriginalDICOMDataOnDevice += ":/" + device.m_label;
				device.m_directPathToOriginalDICOMDataOnDevice += "/";

                oDevices.push_back (device);
            }
        }
        
    }

	return RTVRegistry::kSuccess;
#endif
}

//-----------------------------------------------------------------------------
// Get the InteractiveReportLocation from the Registry
int AppComConfiguration::GetInteractiveReportLocation (std::string& oLocation)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

	oLocation = "";
	int status = rtvr.GetRegistryKey ("InteractiveReportLocation", oLocation);

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value
		oLocation = "c:/AQNetInteractiveReport/";
	}

	// Make sure you always have a trailing slash
	if (strncmp (&oLocation[oLocation.length()-1], "/", 1) != 0)
		oLocation += "/";

	return status;
}

//-----------------------------------------------------------------------------
// Get the Log file location from the Registry
int AppComConfiguration::GetLogFilesLocation (std::string& oLocation)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

	oLocation = "";
#if 0 //#7 2012/03/08 K.Ko
	int status = rtvr.GetRegistryKey ("LogFilesLocation", oLocation);
#else
	oLocation = _Home_folder + "log/";
	int status = RTVRegistry::kSuccess;
#endif

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value
		oLocation = "c:/AQNetLog/";
	}
	// Make sure you always have a trailing slash
	if (strncmp (&oLocation[oLocation.length()-1], "/", 1) != 0)
		oLocation += "/";

	return status;
}

//-----------------------------------------------------------------------------
// Get the Log file location from the Registry
int AppComConfiguration::GetAQNetHome (std::string& oLocation)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

	oLocation = "";
#if 0 //#7 2012/03/08 K.Ko
	int status = rtvr.GetRegistryKey ("AQNetHome", oLocation);
#else
	oLocation = _Home_folder ;
	int status = RTVRegistry::kSuccess;
#endif
	if (status != RTVRegistry::kSuccess)
	{
		// Default Value
		oLocation = "c:/AQNetHome/";
	}
	// Make sure you always have a trailing slash
	if (strncmp (&oLocation[oLocation.length()-1], "/", 1) != 0)
		oLocation += "/";

	return status;
}

//-----------------------------------------------------------------------------
// Get the AutoDeleteAge from the Registry (moved from HKEY_LOCAL_MACHINE\SOFTWARE\TeraRecon\Aquarius\AQNet\1.0
// to HKEY_LOCAL_MACHINE\SOFTWARE\TeraRecon\Aquarius\Common to support APS (Junnan, 09/11/06) 
// 
int AppComConfiguration::GetAutoDeleteAge (int& oAge)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppCommonRegistry());

 
    int age = 0;	//	Default is to delete nothing
 

	int status = rtvr.GetRegistryKey ("AutoDeleteAge", age);

	if (status != RTVRegistry::kSuccess)
	{
 
		age = 0;	//	Default is to delete nothing
 
	}
 
    oAge = age;
	return status;
}


//-----------------------------------------------------------------------------
//	Is the product configured as AqNETSonic (ultrasound only)?
int AppComConfiguration::GetAqNETSonicMode (int& oIsSonic)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

    int isSonic = 0;	//	Default is normal operating mode
	int status = rtvr.GetRegistryKey ("AqNETSonic", isSonic);
	if (status != RTVRegistry::kSuccess)
	{
		isSonic = 0;
	}
 
    oIsSonic = isSonic;
	return status;
}

//-----------------------------------------------------------------------------
// Get the AutoDeleteAge from the Registry
int AppComConfiguration::GetAutoWakeupHour (int& oHour)
{
	RTVRegistry rtvr (kDefaultKey, CPxDB::getAppDefaultRegistry());

    int hour = 3;	//	Default is to wake up at 4AM

	int status = rtvr.GetRegistryKey ("AutoDeleteWakeupHour", hour);

	if (status != RTVRegistry::kSuccess)
	{
		// Default is to wake up at 4AM
		hour = 3;
	}
 
    oHour = hour;
	return status;
}



//---------------------------------------------------------------------------
//-- 10/02/2002
int AppComConfiguration::GetValue(const char* key, int& oValue, int iDefault)
{
	return GetValue(CPxDB::getAppDefaultRegistry(), key, oValue, iDefault);
/*	RTVRegistry rtvr (kDefaultKey, kDefaultLocation);
	int ret;

	int status = rtvr.GetRegistryKey (key, ret);

	if (status != RTVRegistry::kSuccess)
		ret = iDefault;
	
	oValue = ret;
	return status;
	*/
}

//-----------------------------------------------------------------------------
// Jwu 03/31/2003
int AppComConfiguration::SetValue (const char* key,int iValue)
{
	
	return SetValue(CPxDB::getAppDefaultRegistry(), key, iValue);
	/*RTVRegistry rtvr (kDefaultKey, kDefaultLocation);

 	int status = rtvr.SetRegistryKey (key, iValue);
	return status;
	*/
}

//-----------------------------------------------------------------------------
//  
int AppComConfiguration::GetValue(const char* iLocation, const char* key, int& oValue, int iDefault)
{
	RTVRegistry rtvr (kDefaultKey, iLocation);
	int ret;

	int status = rtvr.GetRegistryKey (key, ret);

	if (status != RTVRegistry::kSuccess)
		ret = iDefault;
	
	oValue = ret;
	return status;
}

//-----------------------------------------------------------------------------
//  
int AppComConfiguration::SetValue (const char* iLocation, const char* key,int iValue)
{
	RTVRegistry rtvr (kDefaultKey, iLocation);

 	int status = rtvr.SetRegistryKey (key, iValue);
	return status;
}

 
//-----------------------------------------------------------------------------
int AppComConfiguration::GetAllUseableDrives (std::vector<std::string>& oDrives)
{
 
	char root_path[4] = {0};
 
	DWORD drives;
	char ch;
 
	// Get the drives.The return value is a bitmask representing the currently available disk drives
	drives = GetLogicalDrives();

	if(drives == 0)
	{
	// Error
		return -1;
	}
	std::string letter;
	for(ch = 'A'; ch <= 'Z'; ch++, drives = (drives >> 1))
	{
		if((drives & 1) == 1) // check if bit 0 contains a drive
		{
			 sprintf(root_path, "%c:", ch);

			 UINT drive_type = GetDriveType(root_path); // Get the drive type
			 // Check if it's a fixed disk  
			 if(drive_type == DRIVE_FIXED )
			 {
				 letter = root_path;
				 letter.resize(1);
				 oDrives.push_back (letter);	   
			 }
		}
	}
	return RTVRegistry::kSuccess;
}


//-----------------------------------------------------------------------------
//
int AppComConfiguration::GetRAIDName (const std::string iDriveLetter, std::string& oRaidName)
{
	char loc[64];
	std::string value;
	oRaidName = "";
	for (int i = 0; i < kMaxNumberOfArchiveDrives; i++)
    {
		sprintf (loc, "%s\\DriveList\\%02X", CPxDB::getAppCommonRegistry(), i);
		RTVRegistry rtvr (kDefaultKey, loc);
        
		int status = rtvr.GetRegistryKey ("Drive", value);
        if (status == RTVRegistry::kSuccess)
        {
            // Only push the string if it does have a valid value.
            if (!value.empty() && !(value.compare(iDriveLetter)))
            {

				status = rtvr.GetRegistryKey ("Path", value);
				if( status == RTVRegistry::kSuccess)
				{
					if(!value.empty()) //C:/AQNet/RAID0/
					{
						value.resize(value.size()-1); // remove laset slash
						oRaidName = value.substr(value.find_last_of("/")+1);
					}
				}
				break; 
			}
        } 
	}
 
	return RTVRegistry::kSuccess;

}

//-----------------------------------------------------------------------------
// Always return path with tailing slash
//
int AppComConfiguration::GetConfigRootPath(std::string& oConfigRoothPath)
{
	RTVRegistry rtvr ( kDefaultKey,  CPxDB::getAppCommonRegistry());
 #if 0 //#7 2012/03/08 K.Ko
	int status = rtvr.GetRegistryKey ("ConfigRoot", oConfigRoothPath);
#else
	oConfigRoothPath = _Home_folder + "config/";
	int status = RTVRegistry::kSuccess;
#endif

	if (status != RTVRegistry::kSuccess)
	{
		// Default Value
		oConfigRoothPath = "c:/AQNetConfig/";
	}
	else {
		 
		std::string c = oConfigRoothPath.substr(oConfigRoothPath.length()-1);
		if(c.compare("/") != 0 && c.compare("\\") != 0 ) oConfigRoothPath += "/";
	}

	return RTVRegistry::kSuccess;
}

int AppComConfiguration::GetDataRootPath(std::string& oDataRoothPath)
{
 
	oDataRoothPath = _Home_folder + "data";
	int status = RTVRegistry::kSuccess;
 

	 

	return RTVRegistry::kSuccess;
}

int AppComConfiguration::GetJpegGatewayFlag()//#47
{
	int isGateway = 0;
	RTVRegistry rtvr ( kDefaultKey,  "SOFTWARE\\PreXion\\PXDataServer");
	int status = rtvr.GetRegistryKey ("JpegGateway", isGateway);
	return isGateway;
}
int AppComConfiguration::GetGatewayFlag()//#47
{
	int isGateway = 0;
	RTVRegistry rtvr ( kDefaultKey,  "SOFTWARE\\PreXion\\PXDataServer");
	int status = rtvr.GetRegistryKey ("GatewayFlag", isGateway);
	return isGateway;
}

int AppComConfiguration::GetLocalBackupFlag() //#93,#94
{
	int isLocalBackup = 0;
	RTVRegistry rtvr(kDefaultKey, "SOFTWARE\\PreXion\\PXDataServer");
	int status = rtvr.GetRegistryKey("LocalBackup", isLocalBackup);
	return isLocalBackup;
}