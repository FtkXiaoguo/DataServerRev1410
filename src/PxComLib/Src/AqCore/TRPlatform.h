/***********************************************************************
 * TRPlatform.h
 *---------------------------------------------------------------------
 *
 *   
 *-------------------------------------------------------------------
 */

#ifndef TRPLATFORM_H_
#define TRPLATFORM_H_

#ifdef _WIN32
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

//#include ".h"

#	include <tchar.h>
#	include <io.h>
#	include <direct.h>
#	include <windows.h>
#	include <sys/types.h>
#	include <winsock.h>

#	ifndef snprintf
//#	define	snprintf	_snprintf
#	endif

#	ifndef strcasecmp
#	define	strcasecmp	_stricmp
#	endif
#else
#	include <unistd.h>
#endif

#include <sys/stat.h>
#include <time.h>

//#138 2021/02/12 N.Furutsuki
#define STRNCAT_S(dest,str,strLen) strncat_s(dest,sizeof(dest),str,strLen)


struct TRRAM
{
	// all memory is given in MB
	int		m_totalPhysical;		// total available physical memory
	int		m_availablePhysical;	// available physical memory
	int		m_totalVirtual;			// total virtual memory
	int		m_availableVirtual;		// available virtual memory
};

template <class T> inline T  Min(T a, T b) 
{ 
	  return a < b ? a:b;
}


#ifdef _WIN32
//#include "rtvmap.h"
#include <map>
#include <list>
#include <vector>


/**********************************************************************
 * TRFile has most of the information of interest 
 **********************************************************************/
class  TRFile
{
public:
	enum { kCreateTime, kAccessTime, kModificationTime};
	TRFile () { m_name[0] = '\0';}
	TRFile(const TRFile& iFile)		{ *this = iFile;}
	TRFile(const WIN32_FIND_DATA& iData)	{ *this = iData;}

	TRFile(const WIN32_FIND_DATA& iData, const char* iDir)
	{ 
		CopyFromOSData(iData,iDir);
	}

	~TRFile() {};

	const char *GetName(void) const { return m_name;}
	TRFile& operator=(const WIN32_FIND_DATA& iData)
	{
	    CopyFromOSData(iData,0);
		return *this;
	}

	// age in days
	int		GetAge(int iWhat=kCreateTime, int* oHour=0, int* oSec=0) const;
	int		GetAgeInSeconds(int iWhat=kCreateTime) const;
	int		GetSizeInKiloBytes(void) const { return m_size;}

	// member access
	const FILETIME& GetCreationTime(void) const { return m_cTime; }
	void	SetSizeInKiloBytes(int kbytes) { m_size = kbytes;}
	void	AddSizeInKiloBytes(int kbytes) { m_size += kbytes;}

	bool	IsDirectory(void) const
	{
		return (m_attributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
	}

	void	CopyFromOSData(const WIN32_FIND_DATA& iData, const char* iDir)
	{
		if (!iDir || !*iDir)
		{
			strncpy(m_name, iData.cFileName, sizeof m_name);
		}
		else
		{
			int n = strlen(iDir);
			int hasSlash=(iDir[n-1]=='/' || iDir[n-1]=='\\');
			_snprintf(m_name,sizeof m_name, "%s%s%s", iDir,hasSlash?"":"/",iData.cFileName);
		}

		m_name[sizeof m_name - 1] = '\0';

		m_cTime = iData.ftCreationTime;
		m_mTime = iData.ftLastWriteTime;
		m_aTime = iData.ftLastAccessTime;
		m_size	= int((iData.nFileSizeHigh * (__int64(MAXDWORD)+1) + iData.nFileSizeLow)/1024);
		m_attributes = iData.dwFileAttributes;
		
	}
private:
	char			m_name[MAX_PATH];
	FILETIME		m_cTime;
	FILETIME		m_aTime;
	FILETIME		m_mTime;
	int				m_size;	// in kBytes;
	unsigned int	m_attributes;
};

//------------------------------------------------------------------------------
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

class  TRFileATimeSort
{
public:
	TRFileATimeSort(void) {}
	bool operator() ( const TRFile& A, const TRFile& B) const
	{
		return A.GetAgeInSeconds(TRFile::kAccessTime) > B.GetAgeInSeconds(TRFile::kAccessTime);
	}
};

//-----------------------------------------------------------------------------

class TRFileMTimeSort
{
public:
	TRFileMTimeSort(void) {}
	bool operator() ( const TRFile& A, const TRFile& B) const
	{
		return A.GetAgeInSeconds(TRFile::kModificationTime) > B.GetAgeInSeconds(TRFile::kModificationTime);
	}
};


//-----------------------------------------------------------------------------
// Sort based on a weighted average of CreationTime and AccessTime
class TRFileCATimeSort
{
public:
	TRFileCATimeSort(void) {}
	bool operator() ( const TRFile& A, const TRFile& B) const
	{
		unsigned long aT=A.GetAgeInSeconds(TRFile::kAccessTime)/8 + A.GetAgeInSeconds(TRFile::kCreateTime);
		unsigned long bT=B.GetAgeInSeconds(TRFile::kAccessTime)/8 + B.GetAgeInSeconds(TRFile::kCreateTime);
		return aT > bT;
	}
};
/**************************************************************************
 *
 * Filename only
 * specially designed for std::map<> use on seriesUID caches
 **************************************************************************/

class TRFileName
{
public:
	TRFileName(const TRFileName& iFile)	{ *this = iFile;}
	TRFileName(const WIN32_FIND_DATA& iData)	{ *this = iData;}
	~TRFileName() {};
	TRFileName(const char *s=0) 
	{ 
		if (s)	strcpy(m_name, s);
		else	m_name[0]='\0'; 
		m_len = strlen(m_name);
	}

	inline const char*	GetName(void) const { return m_name;}
	inline const char*	Name(void)	  const { return m_name;}
	inline int			GetLength(void) const { return m_len;}
	inline int			Length(void)	const { return m_len;}

	/* in yyyy-mm-dd format */
	inline const char* GetCreationDate(void) const { return m_date;}

	bool operator < (const TRFileName& iFile) const
	{
		//return strncmp(m_name, iFile.Name(), Min(iFile.Length(), this->Length())) < 0;
		return strcmp(m_name, iFile.Name()) < 0;
	}

	TRFileName& operator=(const WIN32_FIND_DATA& iData)
	{
		strncpy(m_name, iData.cFileName, sizeof m_name);
		m_name[sizeof m_name - 1] = '\0';
		m_len = strlen(m_name);
		_SYSTEMTIME sysTime;
		FileTimeToSystemTime(&iData.ftCreationTime, &sysTime);
		sprintf(m_date,"%04d-%02d-%02d",sysTime.wYear,sysTime.wMonth,sysTime.wDay);
		return *this;
	}

private:
	char		m_name[MAX_PATH];
	char		m_date[12];
	int			m_len;
};

#endif



class FileNameCmp // complet name
{
	FileNameCmp(void) {}
	inline bool operator() ( const TRFileName& f1, const TRFileName& f2) const
	{
		return strcmp(f1.Name(), f2.Name()) < 0;
	}
};

#include <string>
// most of the method in TRPlatform return 0 for success except
// methods that have a boolean name, such IsDirectory() etc, that
// return 1 for success.
class TRPlatform 
{
public:
	TRPlatform(void);
	virtual ~TRPlatform(void);
	
	//
	// SysInfo
	//
	int			GetTotalPhysicalMemory(void);
	int			GetTotalVirtualMemory(void);
	int			GetProcessorCount(void) ;
	static int	GetAvailableDiskSpace(const char* iDir); // in MBytes
	int			GetTotalDiskSpace(const char *iDir);	// in MBytes;

	static const char*  GetMyName(void);
	static const char*  GetMyNameLowercase(void);
	static const char*  GetMyNameUppercase(void);

	static unsigned		GetIPAddress(void);
	static const char*	GetIPAddressString(void);
	
	static const TRRAM*	GetRAM(void);				

	// 
	// System shutdown
	//
	static int			Shutdown(int iReboot=0);
	static int			Reboot(void) { return Shutdown(1);}


	static const char*		GetSysErrorText(char *oMesgBuf, int iBufLen);

	//
	// process related
	//

	enum
	{
		kLow,
		kNormal,
		kHigh
	};

	// run an external command in either sync or async mode.
	// returns 0 for sucess. -1 for error. In the case of async,
	// the returned status is the creation status, not the termination
	// status. In the case sync mode, the returned status is the exit status 
	// of the command.
	// Kind of run and forget kind of thing
	static int System(const char* iCommand, int iAsync=1, int iPriority=kNormal);

	// old, not sure working
	static int SystemCommand(const char *sysCommand, const char* commandName, int iSync=1);
	static int WaitSystemCommand(int iPid);


	// find basic info about a file/dir: existence/readable/writable etc.
	// access() return 0 for success
	enum
	{
		R_OK = 04,	// readable
		W_OK = 02,	// writable
		F_OK = 00	// exisitence
	};

	// recursively remove a directory
	static int RemoveDirectory(const char* dir);
	static int RemoveParentDirectoryIfEmpty(const char* iPath);

	static inline int			access(const char *iPath, int iMode)
	{
		return iPath ? ::access(iPath, iMode) : -1;
	}
	
	//----------------------------------------------------------------------
	// create a directory
	static inline int			mkdir(const char *iPath)
	{
		return iPath ? ::mkdir(iPath) : -1;
	}

	//----------------------------------------------------------------------
	// check if a given path is a directory
	static  inline int			IsDirectory(const char* iPath)
	{
		struct stat statBuf;
		
		if (!iPath || !*iPath)
			return 0;
		
		if (::stat(iPath, &statBuf) != 0)
			return 0;
		
		return (statBuf.st_mode & _S_IFDIR);
	}

	//----------------------------------------------------------------------
    static inline bool IsSlash(int c) { return c=='/' || c=='\\';}

	//----------------------------------------------------------------------
	// create a directory if not already exists - recursively: 0 success
	static	inline int			MakeDirIfNeedTo(const char *iDir)
	{
		if (!iDir || !*iDir)
			return -1;
		
		if (IsDirectory(iDir))
			return 0;
		
		char temp[512], *p, *p1;
	
		strncpy(temp,iDir,sizeof temp);
		temp[sizeof(temp)-1] = 0;
		
		// remove all the trailing slashes
		for (p = temp+strlen(temp)-1 ; p >= temp && IsSlash(*p); p--)
		{
			*p = '\0';
		}
		
		// check again 
		if (IsDirectory(temp))
			return 0;
		
		// tcz 2005.06.06
		// for UNC, better to try make dir directly as shared point
		// does not work right with stat, which could make this function fail.
		
		if (mkdir(temp) == 0)
			return 0;
		
		// save original
		char dir0[512];
		strcpy(dir0, temp);
		
		p = strrchr(temp,'/');
		p1 = strrchr(temp,'\\');
		// get upper directory
		if ( p || p1)
		{
			if (p > p1) 
				*p = '\0';
			else 
				*p1 = '\0';
		}
		else
		{
			// reached root probably
			return 0;
		}

		int status = 0;
		
		if ((status = MakeDirIfNeedTo(temp)) != 0)
			return status;
		
		status = mkdir(dir0);
		
		if(status < 0 && IsDirectory(dir0))
			return 0;
		
		return status;
	}

	//---------------------------------------------------------------------

	static inline bool IsSystemFile(const char* iFileName)
	{
		static const int sBypassSystemFileCheck = 0;
	
		if (sBypassSystemFileCheck)
			return false;

		if (iFileName && access(iFileName,F_OK) == 0)
		{
			DWORD fileAttrib = GetFileAttributes(iFileName);
			if (fileAttrib & FILE_ATTRIBUTE_SYSTEM)
			{
				// on XP, ".pref is considered system file. AQNet saves user 
				// preference with .pref filename
				return strstr(iFileName,".pref") == 0; 
			}
		}

		return false;
	}
	

	//----------------------------------------------------------------------
	static inline int			strcasecmp(const char *s1, const char* s2)
	{
		return ::strcasecmp(s1,s2);
	}

	//----------------------------------------------------------------------
	// delete a file
	static inline int			remove(const char *fileName)
	{
		if (IsSystemFile(fileName))
			return -1;
		return fileName ? ::remove(fileName) : -1;
	}

	//----------------------------------------------------------------------
	static inline int			rename(const char *oldName, const char *newName)
	{
        if (IsSystemFile(oldName) || IsSystemFile(newName))
			return -1;
		return ::rename(oldName,newName);
	}

	//----------------------------------------------------------------------
	static inline const char*	ctime(const time_t *iT=0)
	{
		time_t t;
		t = iT ? *iT : time(&t);
		return ::ctime(&t);

	}

	//----------------------------------------------------------------------
	static inline const char*	ctimeN(const time_t *iT=0)
	{
		char *s = (char *) TRPlatform::ctime(iT);
		*(s + 19) = '\0'; // don't need the newline and year
		return s + 4;
	}

	//----------------------------------------------------------------------
	// Generates same UID for the same input iUID if it not 0. If iUID is 0,
	// a unique UID will be generated each time this function is called.
	static std::string GenerateUID(const char* iUID=0, const char* rootUID=0);

	static unsigned int GetChecksum(const char *iStr, int iSeq=0);

	// similar to ctimeN() except with millisecond precision
	static const char* TimeStamp(void);
	static const char* YYYYMMDDTimeStamp(int includeHour=1);
	static const char* YYYYMMDDHHMMSSUUUTimeStamp(int includeSeparator=0);

	// get file size, in bytes
	static inline unsigned int	GetFileSize(const char *iFile)
	{
		if (iFile)
		{
			struct _stat statBuf;
			statBuf.st_size = 0;
			::_stat(iFile, &statBuf);
			return statBuf.st_size;
		}

		return 0;
	}

	//----------------------------------------------------------------------
	static inline unsigned int GetFileModificationTime(const char *iFile)
	{
		if (iFile)
		{
			struct _stat statBuf;
			statBuf.st_mtime = 0;
			::_stat(iFile,&statBuf);
			return statBuf.st_mtime;
		}
		return 0;
	}

	//----------------------------------------------------------------------
	static inline unsigned int GetFileAccessTime(const char* iFile)
	{
		if (iFile)
		{
			struct _stat statBuf;
			statBuf.st_atime = 0;
			::_stat(iFile,&statBuf);
			return statBuf.st_atime;
		}
		return 0;
	}

	//----------------------------------------------------------------------
	static inline unsigned int GetFileCreationTime(const char *iFile)
	{
		if (iFile)
		{
			struct _stat statBuf;
			statBuf.st_mtime = 0;
			::_stat(iFile,&statBuf);
			return statBuf.st_ctime;
		}
		return 0;
	}

	static int m_netInitialized;

	static void NetInit(void)
	{

#ifdef _WIN32
	if(!m_netInitialized)
	{
		WSADATA wsaData;
		if (WSAStartup(0x202, &wsaData)!= 0) 
		{
			WSACleanup();
			return;
		}
		m_netInitialized = 1;
	}
#endif
	}


	static void NetShutdown(void)
	{
		if (m_netInitialized)
		{
			WSACleanup();
			m_netInitialized = 0;
		}
	}

	static inline void			sleep(int sec)
	{
#ifdef _WIN32
		Sleep(sec*1000);
#else
		::sleep(sec);
#endif
	}



	static inline void			msleep(int msec)
	{
#ifdef _WIN32
		Sleep(msec);
#else
		ulseep(msec*1000);
#endif
	}

	static const char*			GetOSName(int iLongFormat=0);

	
#ifdef _WIN32


	template <class T> static  int iGetDirectoryList(const char *iDir, const char *iRE, std::vector<T, std::allocator<T> >& oOut, int iHowMany=0)
	{
		if(!iDir || !*iDir)
			return oOut.size();

		char fileSpec[MAX_PATH];
		WIN32_FIND_DATA FindFileData;
		HANDLE handle;
		
		oOut.clear();
		
		if (!iRE || !*iRE)
			iRE = "*";
				
		snprintf(fileSpec, sizeof(fileSpec),"%s/%s", iDir,iRE);
		fileSpec[sizeof(fileSpec) - 1] = '\0';
		
		handle = FindFirstFile(fileSpec, &FindFileData);
		if (handle == INVALID_HANDLE_VALUE)
		{
			return -1;
		}
		
		oOut.push_back( T(FindFileData));
		
		for ( int n = 1;FindNextFile(handle,&FindFileData) && (iHowMany <= 0 || n < iHowMany);n++)
			oOut.push_back( T(FindFileData));
		
		FindClose(handle);
		
		return oOut.size();
	}

	// we could use iGetDirectoryList() to get all kind of directory entries with
	// various information associated with the entry in a sorted fashion.
	// For example, define
	// T	=	TRFile
	// C	=	TRFileCTimeSort
	// we could get CreationTime sorted file lists
	
	template <class T,class C> static  int iGetDirectoryList(const char *iDir,  std::map<T, int, C>& oOut, int iHowMany=0)
	{
		if(!iDir || !*iDir)
			return oOut.size();

		char fileSpec[MAX_PATH];
		WIN32_FIND_DATA FindFileData;
		HANDLE handle;
		
		oOut.clear();
		
		snprintf(fileSpec, sizeof(fileSpec),"%s/*", iDir);
		fileSpec[sizeof(fileSpec) - 1] = '\0';

		handle = FindFirstFile(fileSpec, &FindFileData);
		if (handle == INVALID_HANDLE_VALUE)
		{
			return -1;
		}
		
		if(FindFileData.cFileName[0]!='.' || (FindFileData.cFileName[1]!='.' && FindFileData.cFileName[1]!='\0'))
			oOut[T(FindFileData)] = 1;
		
		for ( int n = 1;FindNextFile(handle,&FindFileData) && (iHowMany <= 0 || n < iHowMany);n++)
		{
			if (FindFileData.cFileName[0]=='.' && (FindFileData.cFileName[1]=='.' ||FindFileData.cFileName[1]=='\0'))
				continue;
			else
				oOut[T(FindFileData)] = 1;
		}
		
		FindClose(handle);
		
		return oOut.size();
	}

#endif
	
private:
	char		m_osName[32];
	int			m_totalPhysical;
	int			m_totalVirtual;
	int			m_processorCount;
};

//---------------------------------------------------------------
// same as iGetDirectoryList(), except it's recursive. Also
// the filename are full paths
template <class T,class C> int iGetDirectoryListR(const char *iDir,  std::map<T, int, C>& oOut, int iHowMany=0)
{
	
	if(!iDir || !*iDir)
		return oOut.size();
	
	char fileSpec[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE handle;
	
	snprintf(fileSpec, sizeof(fileSpec),"%s/*", iDir);
	fileSpec[sizeof(fileSpec) - 1] = '\0';
	
	handle = FindFirstFile(fileSpec, &FindFileData);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	
	int n = 0;
	do
	{
		if (FindFileData.cFileName[0]=='.' && (FindFileData.cFileName[1]=='.' ||FindFileData.cFileName[1]=='\0'))
			continue;
		
		oOut[T(FindFileData)] = 1;
		
		if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
		{
			;
		}
		else
		{
			std::string subDir = std::string(iDir) ;
			
			subDir += "/" + std::string(FindFileData.cFileName);
			iGetDirectoryListR(subDir.c_str(),oOut,iHowMany);
		}
		
		// check if we have limit
		n = oOut.size();
		if ( n >= iHowMany && iHowMany > 0)
			break;
		
	}
	while (FindNextFile(handle,&FindFileData));
	
	FindClose(handle);
	
	return oOut.size();
}

//---------------------------------------------------------------
// same as iGetDirectoryList(), except it's recursive and vector argument
template <class T> int iGetDirectoryListR(const char *iDir,  std::vector<T>& oOut, int iHowMany=0)
{
	
	if(!iDir || !*iDir)
		return oOut.size();
	
	char fileSpec[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE handle;

	snprintf(fileSpec, sizeof(fileSpec),"%s/*", iDir);
	fileSpec[sizeof(fileSpec) - 1] = '\0';

	handle = FindFirstFile(fileSpec, &FindFileData);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	
	int n = 0;
	do
	{
		if (FindFileData.cFileName[0]=='.' && (FindFileData.cFileName[1]=='.' ||FindFileData.cFileName[1]=='\0'))
			continue;
		
		oOut.push_back(T(FindFileData,iDir));
		
		if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
		{
			;
		}
		else
		{
			std::string subDir = std::string(iDir) ;
			subDir += "/" + std::string(FindFileData.cFileName);
			iGetDirectoryListR(subDir.c_str(),oOut,iHowMany);
		}
		
		// check if we have limit
		n = oOut.size();
		if ( n >= iHowMany && iHowMany > 0)
			break;
		
	}
	while (FindNextFile(handle,&FindFileData));
	
	FindClose(handle);
	
	return oOut.size();
}

#endif
