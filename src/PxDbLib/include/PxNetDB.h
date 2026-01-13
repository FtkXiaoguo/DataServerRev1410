/***********************************************************************
 * PxNetDB.h 
 *
 *-------------------------------------------------------------------
 */

#ifndef	__PX_FXNetDB_h__
#define	__PX_FXNetDB_h__

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxDB.h"
#include "rtvsprotocol.h"
#include "PxDcmDataLib/include/PxDICOMInfo.h"

#define USE_HEADER_DIRECT  // do not use dll

/////////////////////////////////////////////////////////////////////
#ifdef USE_HEADER_DIRECT
#define FxDefDllCls_FxDbLib
#else
#ifdef MakeDll_FxDbLib
	#define FxDefDllCls_FxDbLib __declspec(dllexport)
#else 
	#define FxDefDllCls_FxDbLib __declspec(dllimport)
#endif
#endif
/////////////////////////////////////////////////////////////////////

class FxDefDllCls_FxDbLib CPxDcmDB : public CPxDB
{

public:
	enum { kLoadSortInfoFromDisk = 1};
	static int AuxDataTypeToRTVMask(int iType);
	static int DBReadStatusToRTVS(int iReadstatus);
	static int RTVSReadStatusToDB(int iReadstatus);
	static bool GetAvailableMedia(int reserveSpace, std::string& oRootDir);
	static bool CheckMediaSpace(int reserveSpace, const char* iDir, bool checkLow=false);
	static bool MakeMediaSpace(int reserveSpace, const char* iRootDir);
	static bool CheckSeriesPath(int reserveSpace, const char* iStudyUID, const char* iSeriesUID, std::string& oiCacheDir);
	static bool MakeCacheWriteDir(const char* iStudyUID, const char* iSeriesUID, std::string& oCacheDir);
	// #88 2016/09/26 by N.Furutsuki
	static bool InitDatabaseInfo(bool iRedo = false, int iretry = 0);

	CPxDcmDB();
	virtual ~CPxDcmDB();

	int	OpenUserStudy(SQA& iSqa, int iGroupID, int iUserID, const pRTVSDicomInformation& iFilter);
	int	GetUserStudy(SQA& iSqa, pRTVSDicomInformation& oVal, const pRTVSDICOMDevice& source);
	void CloseIt(SQA& iSqa) {SQLExecuteEnd(iSqa);};

	int	GetUserStudies(int iGroupID, int iUserID, std::vector<pRTVSDicomInformation>& oVal, const pRTVSDicomInformation& iFilter);
	int	GetUserSeries(int iGroupID, int iUserID, std::vector<pRTVSDicomInformation>& oVal, const pRTVSDicomInformation& iFilter);
	int	GetUserSeriesAuxStauts(int iUserID, pRTVSDicomInformation* ioVal, int iSize);
	int GetSeriesSortInfo(const char* iSeriesUID, std::vector<AqDICOMImageInfo>& oVal, int iNInstace=0);

	int UpdateStudyAccessTime(const char* iStudyUID);

	int GetNumberOfInstancesInSeries(const char* iSeriesUID);

	static void	SetDBOption(unsigned int iOption)
	{
		m_sDBOption = iOption;
	}


	static int		m_sDBOption;
};

#endif	/* __PX_FXNetDB_h__ */
