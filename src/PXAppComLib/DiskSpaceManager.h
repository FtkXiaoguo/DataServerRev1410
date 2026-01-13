/*****************************************************************
 * DiskSpaceManager.h
 *
 *
 *	Copyright(c) PreXion  2001, All rights reserved.
 *
 *	PURPOSE:
 *     Manage disk spaces by removing old files.
 *
 *	
 *
 *****************************************************************/

#ifndef RTVDISKSPACE_MANAGER_H_
#define RTVDISKSPACE_MANAGER_H_

#include "rtvthread.h"
#include "AqCore/TRPlatform.h"
#include "AqCore/TRAtomic.h"
#include "AppComConfiguration.h"

//-----------------------------------------------------------------------------
#if 0
class TRFileCTimeSort
{
public:
	TRFileCTimeSort(void) {}
	bool operator() ( const TRFile& f1, const TRFile& f2) const
	{
		return f1.GetAgeInSeconds(TRFile::kCreateTime) > f2.GetAgeInSeconds(TRFile::kCreateTime);
	}
};

//-----------------------------------------------------------------------------

class TRFileATimeSort
{
public:
	TRFileATimeSort(void) {}
	bool operator() ( const TRFile& A, const TRFile& B) const
	{
		return A.GetAgeInSeconds(TRFile::kAccessTime) > B.GetAgeInSeconds(TRFile::kAccessTime);
	}
};
#endif
//-----------------------------------------------------------------------------

class TRFileACTimeSort
{
public:
	TRFileACTimeSort(void) {}
	bool operator() ( const TRFile& A, const TRFile& B) const
	{
		unsigned long aT=A.GetAgeInSeconds(TRFile::kAccessTime) + A.GetAgeInSeconds(TRFile::kCreateTime)/8;
		unsigned long bT=B.GetAgeInSeconds(TRFile::kAccessTime) + B.GetAgeInSeconds(TRFile::kCreateTime)/8;
		return aT > bT;
	}
};

//-----------------------------------------------------------------------------

typedef std::map<TRFile, int, TRFileCTimeSort> CTimeSortedFile;
typedef std::map<TRFile, int, TRFileATimeSort> ATimeSortedFile;
typedef std::map<TRFile, int, TRFileACTimeSort> ACTimeSortedFile;

typedef ACTimeSortedFile tDSMFiles;
//just use access time sort, because on NT the access resultion is one hour
//typedef ATimeSortedFile tDSMFiles;

/* 
 * iRTVSGetSpace class removes the files/directories in
 * order to create more space.
 */
class RTVDiskSpaceManager;
class iRTVSGetSpace : public iRTVThreadProcess
{
public:
	iRTVSGetSpace(void);
	int			Process(void);
	void		Reset(void);
	void		SetRequestSize(int iReq) {m_MBytesToRemove = iReq;}
	int			FreedSpace(void) const { return m_freedSpace;}
	RTVDiskSpaceManager*	m_manager;
private:
	int			m_MBytesToRemove;
	int			m_freedSpace;
};
//-----------------------------------------------------------------------------

// the user of RTVDiskSpaceManager should make a global instance of RTVDiskSpaceManager
// and let other threads to request space from that RTVDiskSpaceManager
// Here we go to predefine a external global refrence.
typedef std::map<std::string, RTVDiskSpaceManager*> SpaceManagerGroup;

#define kMaxWorkerThreads 5

class RTVDiskSpaceManager : public iRTVThreadProcess
{
friend iRTVSGetSpace;
public:
	enum { kOK = 0, kFailed = -20, kCannotOpen, kErrWrite, kFileTooLarge, kBadDir};

	/*** space manager group start ***/
	static int StartupAll();
	static int UpdateAll();
	static void ShutdownAll();
	static int GetAvailableMedia(int reserveSpace, std::string& oOriginalRootDir, 
                                 std::string& oCacheRootDir);
	
	static bool MakeMediaSpace(int reserveSpace, std::string cacheDir);

    static std::string GetDirectoryToWriteCacheTo  (const std::string& iSeriesUID, 
                                             const std::string& iStudyUID);

	// -- 2005.05.04
	// for import and general scratch directory
	static std::string GetCacheDirectory(void);
	static std::string GetImportDirectory(void);

	static inline bool FileExists(const std::string& iDir, const std::string& iPat);
    static std::string GetDirectoryToReadCacheFrom (const std::string& iSeriesUID, 
				const std::string& iStudyUID, const char* iPattern="Cache.description");
    static std::string GetDirectoryToReadOriginalFrom (const std::string& iSeriesUID, 
                                                const std::string& iStudyUID,
												const char* iPattern=0);
	//	Added by -- - 12/18/02
	static int DeleteCache(const std::string& iSeriesUID, const std::string& iStudyUID);
	//-----------------------------------------------------------------------------
	//

	static int SetEarlyHighwatermarkEmail(int iYN)
	{
		int oldSetting = m_sEarlyHighWatermarkEmail;
		m_sEarlyHighWatermarkEmail = iYN;
		return oldSetting;
	}

private:

	RTVDiskSpaceManager(void);
	~RTVDiskSpaceManager(void);
	
// use group space managers to manage space on each Archive Devices
// Next version will make one space manager to manage multip devices
/*** space manager group start ***/
	static  SpaceManagerGroup c_SpaceManagers;
	static bool c_started;

	
    // Vikram 03/22/02 Change to support Multiple Drives
    static std::vector <AppComDevice> c_raidDevices;

/*** space manager group end ***/

	int			Start(const char* iDir);

	int			Process(void);

	// have enough space left
	int			SpaceAvailable();
	int			MakeSpace(int iMBytes);

	// how much space to reserve
	void		SetWaterMarks(int iHighMBytes, int iLowMBytes=0);

	void		SetDirectory(const char* iDir);

	const char* GetDirectory(void) const { return m_directory;}

	// Additional component in the directory tree:
	// Dir=C:/AQNetHome/DicomCache
	// Tail=/Cache
	// file/directory removed= Dir/SeriesUID/*Tail
	void		SetTail(const char* iTail);
	const char*	GetTail(void) const { return m_tail;}

	//template <class T,class C> int Update(const char* top, std::map<T, int, C>& ofiles, bool firstiime=true);
	int					Update(int iNumberOfDirectories=0); // 0 means all
	void				Print(tDSMFiles& files) const;
	char				m_directory[256];
	iRTVThread			m_theThread;
	iRTVThread			m_workerThread[kMaxWorkerThreads];
	iRTVSGetSpace 		m_getSpace[kMaxWorkerThreads];
	iRTVSGetSpace		m_demandSpace;
	TRSemaphore		m_semaphore;
	int					m_lowWaterMark;
	int					m_highWaterMark;
	char				m_tail[32];
	tDSMFiles			m_files;
	TRCriticalSection	m_cs;
	int					m_lastAlert;
	int					m_lastLowAlert;
	static int			m_sEarlyHighWatermarkEmail;  // -- 2006.06.30
};
//-----------------------------------------------------------------------------


#endif	/* RTVDISKSPACE_MANAGER_H_ */


