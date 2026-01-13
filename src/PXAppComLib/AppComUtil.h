/***********************************************************************
 *  AppComUtil.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2005, All rights reserved.
 *
 *	PURPOSE:
 *		Shared routines
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef APPCOM_UTIL_H
#define APPCOM_UTIL_H

#include <string>
#include <vector>

class UtilMediaPointInfo
{
public:
	std::string m_name;
	float m_total;
	float m_free;
};

class CPxDB;
class AppComUtil
{
public:
	static void startDiskSpaceManger();
	static int DeleteSeries(CPxDB* PxDB, const char* iStudyInstanceUID, const char* iSeriesInstanceUID);
	static std::string getSeriesFolder(const std::string &iStudyInstanceUID,const std::string &iSeriesInstanceUID,const char* iSOP);
	static std::string getStudyFolder(const std::string &iStudyInstanceUID,const std::string &iSeriesInstanceUID,const char* iSOP);

	static std::string getDicomFileName(const std::string &seriesFolder,
										const std::string &iSOPInstanceUID,
										int imageNumber);
	 

	static void PxRemoveDirectory(const std::string folder);
	static void PxRemoveAllDiskFiles(std::vector<std::string>& iAllSeries, const char* studyUID, int iKeepOrphaned);

	static bool getMediaPointInfo(std::vector<UtilMediaPointInfo> &MpVector);
	//
	//	Convert LicenseStatus to string
	// 2012/06/11
	static std::string ConvertErrorCodeToString(int iStatus);
	//
	//#33 HIFチェックの追加 2012/09/06 K.Ko
#define ChkHifDrv_OK (0)
#define ChkHifDrv_NoneDll_Error (-1)
#define ChkHifDrv_ChkDrv_Error (-2)
#define ChkHifDrv_Verify_Error (-3)
	static int ChkHifDrv(char *msg,int size);
	 
};

#endif // APPCOM_UTIL_H