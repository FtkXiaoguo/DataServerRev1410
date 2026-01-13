/*************************************************************************
 *
 *---------------------------------------------------------------------
 *	Copyright (c) PreXion , 2001. All rights reserved.
 *
 *	PURPOSE:
 *		This class sets and get configuration information 
 *
 *
 *
 ************************************************************************/

#if !defined(APPCOM_CONFIGURATION_H)
#define APPCOM_CONFIGURATION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// This is the utility class that describes the different device AQNetServer can
// read and write to.
class AppComDevice
{
    public:
        enum
        {
            kDVD = 0,
            kRAID,

        };


        // Get the Label on the device. This is actually the directory in which 
        // the original DICOM data files are stored
        const std::string& GetLabelOnTheDevice () {return m_label;}

        // Get the path to the device
        const std::string& GetTypeOfDevice () {return m_type;}
       
        
        // Get the path to the device
        const std::string GetPathToDevice () {return m_path;}
        
        // Get path to the location of the Cache on the device (C:/AQNet/RAID0/AQNetCache/)
        const std::string& GetPathToCacheOnDevice () {return m_pathToCacheOnDevice;}

        // Get path to the location of the original DICOM data on the device (C:/AQNet/RAID0/AQNetDICOM_R01/)
        const std::string& GetPathToOriginalDICOMDataOnDevice () {return m_pathToOriginalDICOMDataOnDevice;}

		 // Get path to the location of the Cache on the device (D:/AQNetCache/)
        const std::string& GetDirectPathToCacheOnDevice () {return m_pathToCacheOnDevice;}

        // Get path to the location of the original DICOM data on the device (D:/AQNetDICOM_R01/)
        const std::string& GetDirectPathToOriginalDICOMDataOnDevice () {return m_pathToOriginalDICOMDataOnDevice;}
  
        friend class AppComConfiguration;

        // Clear the contents;
        void Clear ();

    private:
        std::string m_label;
        std::string m_type;
        std::string m_path;

        std::string m_pathToCacheOnDevice;
        std::string m_pathToOriginalDICOMDataOnDevice;
};

//-----------------------------------------------------------------------------
class AppComConfiguration 
{
	public:
		//#7 2012/03/08 K.Ko
		static void setHomeFolder(const std::string &home_dir) ;
        // Strings to be used in for the GetArchiveDevices member function
        static const std::string gkRAIDType;
        static const std::string gkDVDType;

		// Get the Product Type String from the Registry
		static int GetProductType    (std::string& oType);

		// Get the Product Version string from the Registry
		static int GetProductVersion (std::string& oVersion);


		// Get the list of devices in the 
		// TCZ changed first parameter from std::string to const std::string& to fix
		// a global parameter initialization problem for gkRAIDType
        static int GetArchiveDevices (const std::string& iType, std::vector<AppComDevice>& oDevices);

		// return c, d, e, not flopy disk drive, cd_rom drive
		static int GetAllUseableDrives    (std::vector<std::string>& oDrives);
		static int GetRAIDName (const std::string iDriveLetter,std::string& oRaidName);

		// Get the  location of the Home directory from the Registry
		static int GetAQNetHome         (std::string& oLocation);

		// Get the InteractiveReportLocation from the Registry
		static int GetInteractiveReportLocation (std::string& oLocation);

		// Get the Log file location from the Registry
		static int GetLogFilesLocation          (std::string& oLocation);

		/* GL 2006-4-7 change to watermark per disk in DB
        // Get the High and low water marks for the disk
        static int GetDiskHighWaterMark (int& oSpaceInMB);
        static int GetDiskLowWaterMark (int& oSpaceInMB);
		static int SetDiskHighWaterMark (int iSpaceInMB);
		static int SetDiskLowWaterMark (int iSpaceInMB);
		*/

		// jwu quick way to get/set registry key/value, need use different key location:kDefaultSNMPMonitorLocation
		static int SetSNMPMonitorStartupType (int iValue);
		static int GetSNMPMonitorStartupType (int& oValue);

		// Get the time to wake up (oHour) and delete original images older than (oAge) days 
		static int AppComConfiguration::GetAutoDeleteAge (int& oAge);
		static int AppComConfiguration::GetAutoWakeupHour (int& oHour);

		// Is the product configured as AqNETSonic (ultrasound only)?
		static int AppComConfiguration::GetAqNETSonicMode (int& oIsSonic);

		// -- - quick hack to do some things before HASP
	    static int GetValue(const char* key, int& oValue, int iDefault=0);
		static int GetValue(const char* iLocation, const char* key, int& oValue, int iDefault=0);

		static int SetValue (const char* key,int iValue);		
		static int SetValue (const char* iLocation, const char* key, int iValue);

		static int GetConfigRootPath(std::string& oConfigRoothPath);
		static int GetDataRootPath(std::string& oDataRoothPath);

		static int GetJpegGatewayFlag();//#47
		static int GetGatewayFlag();//#47
		static int GetLocalBackupFlag(); //#93
};
//-----------------------------------------------------------------------------

#endif // !defined(APPCOM_CONFIGURATION_H)
