/***********************************************************************
 * PXLicenseManagerIf.h
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */
#ifndef PX_LICENSE_MANAGER_IF_H
#define PX_LICENSE_MANAGER_IF_H


 
#  if defined(MAKE_PXLICENSE_DLL)
#     define PXLICENSE_DLL __declspec( dllexport )
#  else
#     define PXLICENSE_DLL __declspec( dllimport )
#  endif

#if defined(PXLICENSE_STATIC_LINK)
	#undef  PXLICENSE_DLL
	#define PXLICENSE_DLL
#endif
 
#define FALSE 0
#define TRUE 1

#include <string>
 

//#include "rtvthread.h"
 

class CPrexionHasp;
struct _FeatureApp;

enum
{
	ePREXION_PXDICOMSERVER = 1,				// feature1
	ePREXION_PXADMINSERVER,					// feature2
 ///
 	ePREXION_FINECUBE_DICOMSERVER,			// feature3
	ePREXION_FINECUBE_IMAGESERVER,			// feature4
	ePREXION_CDMAKER,						// feature5
	ePREXION_PXGATEWAY,						// feature6
 	ePREXION_GPUVR,							// feature7
	ePREXION_PXDICOMSERVER3,				// feature8 //#1401 2012/03/19 DataServerV3Å@K.Ko

	ePREXION_VIWER_PLUGIN1,					// feature9
	ePREXION_VIWER_PLUGIN2,
	ePREXION_VIWER_PLUGIN3,
	ePREXION_VIWER_PLUGIN4,
	ePREXION_VIWER_PLUGIN5,
	ePREXION_VIWER_PLUGIN6,
	ePREXION_VIWER_PLUGIN7,
	ePREXION_VIWER_PLUGIN8,

///
	ePREXION_PX2ConsoleBasic,				// feature17	
#if 0
	ePREXION_PX2ConsoleCephalo,				
	ePREXION_PX2Console1024,					
	ePREXION_PX2DES,	
	ePREXION_PX2MAR,	
	ePREXION_PX2PMS,
	ePREXION_PX2ENT,
//
	ePREXION_PX2CTMode_TMJ,					// feature24
	ePREXION_PX2CTMode_CADCAM,
	ePREXION_PX2EXT_FOV,	
//
	ePREXION_PX2CTMode_Standard,				// feature27
	ePREXION_PX2CTMode_Rapid,
	ePREXION_PX2CTMode_HightDef,
	ePREXION_PX2CTMode_UltraHightDef,
//
	ePREXION_PX2_FOV50_50,					// feature31		 
	ePREXION_PX2_FOV100_50,			 
	ePREXION_PX2_FOV100_78,			 
	
//
#endif
	
	eEndHASP
};

//---------------------------------------------------------------------
//
class PXLICENSE_DLL LicenseManager
{
public:
 

	static void closeAll();
	static LicenseManager& theLicenseManager();

//	virtual ~LicenseManager(void);

	static void selfTest();

	//	CheckFeature() and CheckLicense() will return one of these
	//	Also, DetermineProductCode() sets oValidLicense to one of these
	enum
	{
		kLMLicenseNoHASP = -6,
		kLMHaspException = -5,
		kLMAmbiguousLicense = -4,
		kLMNoLicense = -3,
		kLMLicenseDisuse = -2,
		kLMLicenseDisabled = -1,
		kLMUndefined = 0,
		kLMOk = 1,
		kLMLicenseExpired = 2,
		kLMLicenseValid = 3,
		kLMLicenseWillExpire = 4
	};


	//	Convenience functions
	static int IsValid(int iStatus) { return iStatus >= kLMLicenseValid; }
	static int FeatureExists(int iStatus) { return iStatus >= kLMLicenseExpired; }

	//2016/07/01
	virtual int GetHaspInfo(char* strBuff,int size) = 0;
	//2016/05/10
	virtual int readBinary(int offset, unsigned char *buffer, int size, bool RomFlag=true) = 0;
	//
	//	Call this one if you don't want to add it to the map (for periodic checking)
	virtual int CheckFeature(int iFeature, bool ignoreBypass = false) = 0;
	virtual int CheckFeature(int iFeature, int& oDaysToExpire, bool ignoreBypass = false) = 0;

	//	These are used by the threaded license checker
	virtual int GetCachedStatus(int iFeature, int& oDaysToExpire) = 0;
	virtual int CheckLicense(int iFeature, int& oDaysToExpire) = 0;
	virtual void UpdateLicenseCache(void) = 0;

	//	Use this one.  It returns immediately, and initially, it will return 0.  When the
	//		threaded license checker updates the cache, this will return the correct value.
	virtual int GetNumberOfVolumePro(void) = 0;

	//	The threaded licence checker uses this one.  It is expensive.
	virtual int CacheNumberOfVolumePro(void) = 0;
	//	Will set oProductCode to one of the above values.
	virtual int DetermineProduct(int& oHaspFeatureID, int& oProductCode, int& oLicenseStatus) = 0;	
//
 	virtual void BypassHASP(int iYesNo) = 0;  

	//#1253 2011/10/03 K.Ko 
	virtual char *getCheckCode(int no)=0;
	virtual int	 getCheckID() = 0;
	enum
	{
		kAqUnknown		=		0,
		kPXDicomServer	=		(1 << 0),
		kPXAdminServer	=		(1 << 1),
		kPXFineCubeDicomServer =(1 << 2),
		kPXFineCubeImageServer =(1 << 3),
		kAqNET			=		(1 << 4),
		kPXCDMaker		=		(1 << 5),
		kPXGayway		=		(1 << 6),
		kGpuVR			=		(1 << 7),
	 
	};

protected:
//	LicenseManager(void);


	//////
	virtual void _getHaspDriverVersionStr( char *str_buff) = 0;
public:
	static void _convProductCodeToStr(int iProductCode , char *str_buff);
// 2014.6.9 chen: modified for VC2012, #1710
	bool GetHaspDriverVersionString(char* szBuffer, int length)
	{
		std::string VerStr;
 
		char _str_buff[256];
		_getHaspDriverVersionStr(_str_buff);

		VerStr = _str_buff;
	 
		if (VerStr.size() > static_cast<size_t>(length)+1){
			strcpy(szBuffer, VerStr.c_str());
			return true;
		}else{
			return false;
		}
	}
};


namespace LicenseManagerUtility
{
	//	Convert LicenseStatus to string
	static std::string ConvertErrorCodeToString(int iStatus)
	{
		std::string statusStr;
		switch(iStatus)
		{

// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMLicenseNoHASP:
			statusStr = "No HASP";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMHaspException:
			statusStr = "HASP Exception";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMLicenseDisuse:
			statusStr = "Disuse";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMAmbiguousLicense:
			statusStr = "Ambiguous License";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMNoLicense:
			statusStr = "No License";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMLicenseDisabled:
			statusStr = "Disabled";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMLicenseExpired:
			statusStr = "Expired";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMLicenseValid:
			statusStr = "Valid";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMLicenseWillExpire:
			statusStr = "WillExpire";
			break;
// 2014.6.9 chen: modified for VC2012, #1710
		case LicenseManager::kLMUndefined:
		default:
			statusStr = "Unknown Status";
			break;
		};
		return statusStr;
	}

	static std::string ConvertProductCodeToString(int iProductCode)
	{
		std::string productStr;
 
		char _str_buff[256];
// 2014.6.9 chen: modified for VC2012, #1710
		LicenseManager::_convProductCodeToStr(iProductCode , _str_buff);

		productStr = _str_buff;
	 
		return productStr;
	}

};



//#1253 2011/10/03 K.Ko -- begin
template < class T1  > class MyLicenseManager
{
 #define DefMyCheckCode  "IQaGxKs31t6THxfl1jgRVA5Cc2egC45eg/k4DSZbfE+xnCNRwHbKmt/T3GrMX/4CB37Rle7NeTVGL6bO" \
			"EX1poapb848oWlE84wV4uV7cw/siA5XenPnha9wkYdszVBoyMIgYHukYUS+5ekf7I+NAg2bP2HE4bI5b" \
			"5ooFhsTxUG1yogZ9KO4hxy/LWq2YpBSBmk/B/D0NcBqW4pzld58I6cRl6UiEXaacRE5AZtv1q1tw3HAb" \
			"Ryp7YqH02diJMONjItqVOuaOem9z88ur8ZJTo866m4nG+GIfj/JvaSFXVMKk9b8rzdeROqUU61WGVRSU" \
			"R/HmguRIxt99yIDDTyIdFn+ssZIfh7ZuuoYA2o/N/iSG6ZnELSbV07EhTmD6hSSgHmUr/tppISfW7lwC" \
			"ha67i17+xhcaeQPSANDxwugDadyfhmtCDijpUDQiSTfNr/guHiMvKIDn4OgKjhezF4h71kln1bPaLfV0" 

public:
	//2016/07/01
	static int GetHaspInfo(char *strBuff,int size)
	{
		 
		return T1::theLicenseManager().GetHaspInfo(strBuff, size);
	}

	//2016/05/10
	static int readBinary(unsigned char *buffer, int size,int offset=(2048-16-1), bool RomFlag=true)
	{
		if(!fistMyCheck()){
			
			return LicenseManager::kLMNoLicense;
		};
		return T1::theLicenseManager().readBinary( offset, buffer, size,RomFlag);
	}

	static int CheckFeature(int iFeature, bool ignoreBypass = false) 
	{
		if(!fistMyCheck()){
			
			return LicenseManager::kLMNoLicense;
		};
		return T1::theLicenseManager().CheckFeature( iFeature,  ignoreBypass);
	}
	static int CheckLicense(int iFeature, int& oDaysToExpire){
		 
		if(!fistMyCheck()){
			oDaysToExpire = -1;
			return LicenseManager::kLMNoLicense;
		};
		return T1::theLicenseManager().CheckLicense( iFeature,  oDaysToExpire);
	}
	static int GetCachedStatus(int iFeature, int& oDaysToExpire) {
		if(!fistMyCheck()){
			oDaysToExpire = -1;
			return LicenseManager::kLMNoLicense;
		};
		return T1::theLicenseManager().GetCachedStatus( iFeature,  oDaysToExpire);
	}
	static int DetermineProduct(int& oHaspFeatureID, int& oProductCode, int& oLicenseStatus)
	{
		if(!fistMyCheck()){
			oHaspFeatureID = -1;
			oProductCode = -1;
			oLicenseStatus = LicenseManager::kLMNoLicense;
			return LicenseManager::kLMHaspException;
		};
		return T1::theLicenseManager().DetermineProduct( oHaspFeatureID,  oProductCode, oLicenseStatus);
	}
//
		
	static char shiftCode(char code,int checkID) {
		int nn = checkID%8;
	 
		if(code-nn<0) return 0;
		return code -nn ;
		
	}
protected:
	static bool fistMyCheck(){
		int checkID = T1::theLicenseManager().getCheckID();
		int str_len ;
		const char *checkCode = T1::theLicenseManager().getCheckCode(checkID);
		const char *mycode =  DefMyCheckCode;
		str_len = strlen(mycode);
		if(!checkIDCode( checkCode,mycode, str_len-1,checkID)){
			return false;
		};
		return true;

	}
	
	static bool checkIDCode(const char *checkCode, const char *myCode,int checkSize,int checkID)
		{
	 
			
			for(int i=0;i<checkSize;i++){
				char my_c = myCode[i];
				my_c = shiftCode(my_c,checkID);
				if(my_c != checkCode[i]){
					return false;
				}
			}
			return true;
		}

};

typedef MyLicenseManager< LicenseManager> PXLicenseManager;  // Ç±ÇÍÇégÇ§ÅB

//#1253 2011/10/03 K.Ko -- end

 

#endif PX_LICENSE_MANAGER_IF_H

