/***********************************************************************
 * TRPlatform.cpp
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#include "TRPlatform.h"

#include "TRAtomic.h"

int TRPlatform::m_netInitialized;

//-----------------------------------------------------------------------------
TRPlatform::TRPlatform(void)
{
	m_osName[0] = '\0';
	m_totalPhysical = m_totalVirtual = 0;
	m_processorCount = 0;
}

//-----------------------------------------------------------------------------
TRPlatform::~TRPlatform(void)
{
}

//-----------------------------------------------------------------------------

int TRPlatform::GetTotalPhysicalMemory(void)
{
	if (!m_totalPhysical)
	{
		m_totalPhysical = GetRAM()->m_totalPhysical;
	}

	return m_totalPhysical;
}

//--------------------------------------------------------------------------------
int TRPlatform::GetTotalVirtualMemory(void)
{
	if (!m_totalVirtual)
	{
		m_totalVirtual = GetRAM()->m_totalVirtual;
	}

	return m_totalVirtual;
}

//-------------------------------------------------------------
int TRPlatform::GetProcessorCount(void)
{
	if (m_processorCount)
		return m_processorCount;

	static SYSTEM_INFO sInfo;
	GetSystemInfo(&sInfo);

	m_processorCount = sInfo.dwNumberOfProcessors;

	return m_processorCount;
}

//---------------------------------------------------------------------
const char* TRPlatform::GetSysErrorText(char* lpszBuf,int dwSize)
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;
	int err = GetLastError();

	if (err == 0)
		return "";
	
    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		err,
		LANG_NEUTRAL,
		(LPTSTR)&lpszTemp,
		0,
		NULL );
	
    // supplied buffer is not long enough
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        _stprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }
	
    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );
	
    return lpszBuf;
}

//-------------------------------------------------------------------

int	TRPlatform::GetAvailableDiskSpace(const char *iDir)
{
	__int64 freeSpace, availableSpace, totalSpace;

	if (GetDiskFreeSpaceEx(iDir, (PULARGE_INTEGER)& availableSpace, (PULARGE_INTEGER)& totalSpace, (PULARGE_INTEGER)& freeSpace) == 0)
		return -1;

	int available = int(availableSpace / (1024*1024));

#ifdef _DEBUG
	int total= int(totalSpace  / ( 1024*1024));
	fprintf(stderr,"AvailabelSpace: %d MBytes Total: %d MBytes\n", available,total);
#endif

	return available; 
}

//------------------------------------------------------------------
int	TRPlatform::GetTotalDiskSpace(const char *iDir)
{
	__int64 freeSpace, availableSpace, totalSpace;

	if (GetDiskFreeSpaceEx(iDir, (PULARGE_INTEGER)& availableSpace, (PULARGE_INTEGER)& totalSpace, (PULARGE_INTEGER)& freeSpace) == 0)
		return -1;

	int total= totalSpace  / ( 1024*1024);

	return total; 
}

//-------------------------------------------------------------------
const TRRAM* TRPlatform::GetRAM(void)
{
	static int ibuf;
	static TRRAM ramBuf[8];
	TRRAM *ram = ramBuf + (++ibuf & 7);

#if 0
	MEMORYSTATUSEX mstat;

	mstat.dwLength = sizeof (mstat);
	GlobalMemoryStatusEx (&mstat);

	ram->m_totalPhysical =     mstat.ullTotalPhys /(1024*1024);
	ram->m_availablePhysical = mstat.ullAvailPhys /(1024*1024);
	ram->m_totalVirtual      = mstat.ullTotalVirtual/(1024*1024);
	ram->m_availableVirtual  = mstat.ullAvailVirtual/(1024*1024);

#else
	static MEMORYSTATUS mstat;

	GlobalMemoryStatus(&mstat);
	ram->m_totalPhysical =     mstat.dwTotalPhys /(1024*1024);
	ram->m_availablePhysical = mstat.dwAvailPhys /(1024*1024);
	ram->m_totalVirtual      = mstat.dwTotalVirtual/(1024*1024);
	ram->m_availableVirtual  = mstat.dwAvailVirtual/(1024*1024);

#endif

	return ram;
}

//-------------------------------------------------------------------
const char* TRPlatform::GetOSName(int iLongFormat)
{
	OSVERSIONINFOEX osv;
	BOOL isEx;
	static char osVersion[64];

	if (osVersion[0])
		return osVersion;

	memset(&osv, 0, sizeof(osv));
	osv.dwOSVersionInfoSize = sizeof(osv);

	if (!(isEx = GetVersionEx((OSVERSIONINFO*)&osv)))
	{
		osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osv))
		{
			strcpy(osVersion,"Win/NT(?)");
			return osVersion;
		}
	}

	switch(osv.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if (iLongFormat)
			_snprintf(osVersion, sizeof(osVersion), "%s %d.%d %s",
			osv.dwMajorVersion==5 ? "Win2K":"WinNT", osv.dwMajorVersion, osv.dwMinorVersion, osv.szCSDVersion);
		else
			_snprintf(osVersion, sizeof(osVersion), "%s %d.%d",
			osv.dwMajorVersion==5 ? "Win2K":"WinNT", osv.dwMajorVersion, osv.dwMinorVersion);

#if 0 // seems osv.wProductType only available in W2K/SDK ?
		if (isEx)
		{
			if (osv.wProductType==VER_NT_WORKSTATION)
				strcat(osVersion," Pro");
			if (osv.wProductType==VER_NT_SERVER)
				strcat(osVersion," Server");
		}
		else
			
		{
			HKEY hKey;
            char szProductType[80];
            DWORD dwBufLen;
			
            RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey );
            RegQueryValueEx( hKey, "ProductType", NULL, NULL,
				(LPBYTE) szProductType, &dwBufLen);
            RegCloseKey( hKey );
            if ( lstrcmpi( "WINNT", szProductType) == 0 )
				strcat(osVersion, "Workstation" );
            if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				strcat(osVersion, "Server" );
		}	
#endif
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if ((osv.dwMajorVersion > 4) || ((osv.dwMajorVersion == 4) && (osv.dwMinorVersion > 0)))
			strcpy (osVersion,"Windows98");
		else 
			strcpy(osVersion, "Windows95");
		break;
	default:
		strcpy(osVersion,"Windows32");
		break;
	}
	
	
	// Get language
	char lang[24];
	int sz = (sizeof osVersion ) - 1; 
	lang[0] = '\0';
	::GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_SENGLANGUAGE , lang, sizeof lang);
	
	STRNCAT_S(osVersion, lang, sz);
	osVersion[sz] = '\0';
	
	return osVersion;
}

#include <windows.h>
//------------------------------------------------------------------------------
int	TRPlatform::System(const char* iCommand, int Async, int iPriority)
{
	if (!iCommand || !*iCommand)
		return -1;
	
	//	_flushall();
	static STARTUPINFO si;
	static PROCESS_INFORMATION pi;
	si.cb = sizeof(si);
	
	DWORD createMode = NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW|DETACHED_PROCESS;

#if (AQ_USING_SDK == 1)
	if (iPriority==kLow)
		createMode = BELOW_NORMAL_PRIORITY_CLASS;
	else if (iPriority == kHigh)
		createMode = ABOVE_NORMAL_PRIORITY_CLASS;
#endif
	
#ifdef _DEBUG
	createMode &= ~CREATE_NO_WINDOW;
#endif

	char command[1024];

	strncpy(command,iCommand, sizeof command);
	command[sizeof command - 1] = '\0';
	
	int status = CreateProcess(NULL,command,
		NULL,	// Process handle not inheritable. 
		NULL,	// Thread handle not inheritable.
		FALSE,	// Set handle inheritance to FALSE.
		createMode,
		NULL,   // inherit  parent environment
		NULL,	// current directory
		&si,
		&pi);

	// CreateProcess returns none-zero for success
	if (status == 0)
		status = -1;
	else 
		status = 0;

	if (!Async)
	{
		unsigned long exitStatus = 0;
		WaitForSingleObject( pi.hProcess, INFINITE );
		::GetExitCodeProcess(pi.hProcess,&exitStatus);
		status = exitStatus;
	}
	
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	
	return status;
}

//----------------------------------------------------------------------------
// excute a command synchronously
int TRPlatform::SystemCommand(const char *cmd, const char* cmdName, int sync)
{
	if (!cmd || !cmd)
		return -1;

	if (!cmdName || !*cmdName)
	{
		cmdName = cmd;
	}

	return _spawnlp(sync ? _P_WAIT:_P_NOWAIT, "cmd.exe",cmdName,"/c",cmd,0);
}

//-------------------------------------------------------------
int TRPlatform::WaitSystemCommand(int iPid)
{
	return _cwait(0, iPid, 0);
}


//----------------------------------------------------------------------
// Recursively remove a directory
int TRPlatform::RemoveDirectory(const char* dir)
{
	if( strlen(dir) < 2 ) 
		return -1;

	std::vector<TRFileName> fileList;

	/* dir==0 not exactly an error but could be very dangerous */

	if (!dir || !*dir)
		return -1;

	int status = TRPlatform::iGetDirectoryList(dir, "*", fileList);

	if ( status < 0 && fileList.size() < 2)
	{
#ifdef _DEBUG
		fprintf(stderr,"Can't get file list in %s\n", dir ? dir :"null");
#endif
		return status;
	}


	std::vector<TRFileName>::iterator p;
	std::string name;
	
	for ( p = fileList.begin(); p < fileList.end(); p++)
	{
		if (!strcmp(p->GetName(), ".") || !strcmp(p->GetName(),".."))
			continue;
		
		name = dir;
		name += "/";
		name += p->GetName();
		
		if (TRPlatform::IsDirectory(name.data()) > 0)
		{
			status = RemoveDirectory(name.data());
#if defined(_DEBUG) && 0
			fprintf(stderr,"status=%d remove dir %s\n", status, name.data());
#endif
		}
		else
		{
			status = TRPlatform::remove(name.data());
#if defined(_DEBUG) && 0
			fprintf(stderr,"status=%d remove file %s\n", status, name.data());
#endif
			
		}
	}

	return _rmdir(dir);
}

//-----------------------------------------------------------------------------
//
int TRPlatform::RemoveParentDirectoryIfEmpty(const char* iPath)
{

	//	Make a copy so we can manipulate it
	char d[MAX_PATH];
	strcpy(d, iPath);
	if (!d)
	{
		return -1;
	}

	//	Find the end and work backwards from there
	int i = strlen(d) - 1;
	bool found = false;

	//	Remove trailing slash, if there is one
	if (d[i] == '/' || d[i] == '\\') d[i] = 32;

	//	Remove the last path component
	for(;i>0;i--)
	{
		if (d[i] == '/' || d[i] == '\\')
		{
			found = true;
			d[i] = '\0';
			break;
		}
	}

	//	There weren't any more slashes - we're done
	if (!found)
		return -1;

	//	If there are more than 2 entries in the directory, it is not empty
	std::vector<TRFileName> fileList;
	int status = TRPlatform::iGetDirectoryList(d, "*", fileList);
	if (fileList.size() != 2)
	{
		return 0;
	}

	//	If the 2 entries are . and .., then it is ok to delete the directory
	std::vector<TRFileName>::iterator p;
	std::string name;
	for ( p = fileList.begin(); p < fileList.end(); p++)
	{
		if (!strcmp(p->GetName(), ".") || !strcmp(p->GetName(),".."))
			continue;
		
		return 0;
	}

	//	Go ahead and delete it
	status = TRPlatform::RemoveDirectory(d);
	return status;
}


//---------------------------------------------------------------------
int TRFile::GetAge(int iWhat, int* oHour, int* oSec) const
{
	int			seconds, days, hours;
	static const int	SecondsInADay = (3600*24);
	
	seconds = GetAgeInSeconds(iWhat);
	
	days = seconds / SecondsInADay;
	hours = (seconds - days*SecondsInADay)/3600;
	
	if (oHour)
		*oHour = hours;
	
	if (oSec)
		*oSec = (seconds - days*SecondsInADay - hours * 3600);	
	
	return  days;
}

//-------------------------------------------------------------------
// simulation of ctime() with subsecond precision
#include <sys/timeb.h>
const char* TRPlatform::TimeStamp(void)
{
	static char out[16][36];
	char ctimeOut[32];
	static long sK;
	int k = InterlockedIncrement(&sK);
	char *buf = out[k&15];
	struct _timeb timebuf;

	_ftime(&timebuf);
	strcpy(ctimeOut, ctime(&timebuf.time));
	ctimeOut[24] = '\0';
	_snprintf(buf,sizeof out[0], "%.19s.%03d %.4s", ctimeOut,timebuf.millitm,ctimeOut+20);
	return buf;
}

//-------------------------------------------------------------------
// output yyyymmdd-hhmm
const char* TRPlatform::YYYYMMDDTimeStamp(int hhmm)
{
	static char out[16][36];
	static TRAtomicVar<int> sK;
	int k = ++sK;
	char *buf = out[k&15];
	time_t t = time(0);
	struct tm tm = *localtime(&t);

	tm.tm_mon += 1;
	tm.tm_year += 1900;

	if (hhmm)
	{	
		snprintf(buf, sizeof out[0], "%d%02d%02d%02d%02d", tm.tm_year,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min);
	}
	else
	{
		snprintf(buf, sizeof out[0], "%d%02d%02d", tm.tm_year,tm.tm_mon,tm.tm_mday);
	}
		
	return buf;
}

//-------------------------------------------------------------------
// output yyyymmdd:hhmmss.uuu
const char* TRPlatform::YYYYMMDDHHMMSSUUUTimeStamp(int iSep)
{
	static char out[64][36];
	static TRAtomicVar<int> sK;
	int k = ++sK;
	char *buf = out[k&63];
	struct _timeb timebuf;
	_ftime(&timebuf);
	struct tm tm = *localtime(&timebuf.time);

	tm.tm_mon += 1;
	tm.tm_year += 1900;
	const char* format = "%4d%02d%02d%02d%02d%02d%03d";
	
	if (iSep)
		format = "%4d%02d%02d:%02d%02d%02d.%03d";
	
	snprintf(buf, sizeof out[0], format, tm.tm_year,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min, tm.tm_sec, timebuf.millitm);
	buf[sizeof out[0] - 1] = '\0';
	++k;
	return buf;
}

#if 0
//--------------------------------------------------------------------
int TRPlatform::FromYYYYMMDDHHMMSSUUUTimeStamp(const char* iStamp, unsigned int& sec, unsigned int& ms)
{
    int u, sep = strchr(iStamp,".");
	struct tm;

	const char* fmt = sep ? "%4d%02d%02d:%02d%02d%02d.%03d":"%4d%02d%02d%02d%02d%02d%03d";

	sscanf(iStamp,fmt,&tm.tm_year,&tm.tm_mon,&tm.tm_mday,&tm.tm_hour,&tm.tm_min,&tm.tm_sec, &u);

	sec = _mktime(&tm);
	us = u;

	return 0;
}
#endif

//---------------------------------------------------------------------
int TRFile::GetAgeInSeconds(int iWhat) const
{
	SYSTEMTIME	stime;	
	FILETIME	ftime;
	__int64		diff;

	GetSystemTime(&stime);
	SystemTimeToFileTime(&stime,&ftime);
	
	if (iWhat == kAccessTime)
	{
		diff = *(__int64*)&ftime - *(__int64*)&m_aTime;
	}
	else if (iWhat == kModificationTime)
	{
		diff = *(__int64*)&ftime - *(__int64*)&m_mTime;
	}
	else
	{
		diff = *(__int64*)&ftime - *(__int64*)&m_cTime;
	}
	
	/* the stupid thing is in 100-nano seconds */
	return int(diff/10000000);
}

//---------------------------------------------------------------------
#include <winsock.h>
const char* TRPlatform::GetIPAddressString(void)
{
	/* semi-thread safe hack */
	static char sRet[10][32];
	static int ind;
	char* ret =  sRet[ind];
	in_addr addr;
	addr.s_addr = GetIPAddress();
	strcpy(ret, inet_ntoa(addr));

#if 0
	char hostname[128];

	struct hostent *hostent;
	struct sockaddr_in server;
	
	gethostname(hostname, sizeof hostname);
	
    if(!(hostent = gethostbyname(hostname)))
	{
		return "";
	}
	
    memcpy((char *)&(server.sin_addr.s_addr),*hostent->h_addr_list, hostent->h_length);
	strcpy(ret, inet_ntoa(server.sin_addr));
#endif	

	ind = (ind + 1) % 10;
	return ret;
}

//-------------------------------------------------------------------
unsigned TRPlatform::GetIPAddress(void)
{
	struct hostent *hostent;
	struct sockaddr_in server;
	char hostname[128];
	static unsigned int IPAddress;
	static unsigned int sLoopbackIP = inet_addr("127.0.0.1");

	if (IPAddress && IPAddress != sLoopbackIP)
		return IPAddress;
	
	gethostname(hostname, sizeof hostname);
	
    if(!(hostent = gethostbyname(hostname)))
	{
		return sLoopbackIP;
	}
	
	memcpy((char *)&(server.sin_addr.s_addr),*hostent->h_addr_list, hostent->h_length);
	
	return  IPAddress = server.sin_addr.s_addr;
}

//------------------------------------------------------------------------
BOOL TRPlatform::Shutdown(int iReboot)
{
	/* reboot or shutdown the system forcibly - moved from the LCDCommand to 
	 * platform so Web can use it (-- 06/11/02)
	 */
	HANDLE hToken;              // handle to process token 
	TOKEN_PRIVILEGES tkp;       // pointer to token structure 
	
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
		return FALSE;
	}
	
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,  &tkp.Privileges[0].Luid); 
	
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	
	// Get shutdown privilege for this process. 
	
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0); 
	
	// Cannot test the return value of AdjustTokenPrivileges. 
	
	if (GetLastError() != ERROR_SUCCESS) 
        return FALSE;
	
	BOOL ret;
	
	if (iReboot)
		ret = ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
	else
		ret = ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE | EWX_POWEROFF, 0);

	return ret;
}

#define  sRTVRootUID	   "2.16.840.1.113669.632.21"

//-----------------------------------------------------------------------
unsigned int TRPlatform::GetChecksum(const char *iUID, int iRandom)
{
	if (!iUID) 
		return 0;

	unsigned int len = strlen(iUID);
	unsigned int c = 0,i;


	const char * p = iUID + len;
	
	if (iRandom < 0)
	{
		for ( c = len;  --p >= iUID; )
		{
			c +=  unsigned(*p + 1 - '0');
		}
	}
	else if (iRandom == 0)
	{
		for ( c = len;  --p >= iUID; )
			c = (c << 4)^(c >> 28)^*p;
		
	}
	else
	{
		static unsigned int weights[32] = 
		{ 
			1,2,2,1,5,6,7,8,9,7,5,4,3,2,5,2,
			1,3,4,8,2,9,4,3,0,4,3,2,1,8,2,5
		};
		
		for ( i = 0; --p >= iUID; ++i)
		{
			c +=  unsigned(*p + 1 - '0') * weights[(iRandom+i)&31];
		}
	}


	return c;
}

//-----------------------------------------------------------------------
std::string TRPlatform::GenerateUID(const char* iUID, const char* rootUID)
{
	char uniq[64+4], UID[64+4];

	memset(uniq,0,sizeof(uniq));
	memset(UID,0,sizeof(UID));

	static TRAtomicVar<unsigned int> sSeq;
	static int pID = _getpid();
	static unsigned timeOffset = unsigned(34*(365*24*60*60)); // 34 years
	
	if (!rootUID || !*rootUID)
		rootUID = sRTVRootUID;
	
	if (!iUID || !*iUID)
	{
		unsigned int seq = ++sSeq;
		struct _timeb timebuf;
		_ftime(&timebuf);
		_snprintf(uniq, sizeof uniq,"%u.%s.%u.%u.%u.%u", TRPlatform::GetIPAddress(),
			YYYYMMDDTimeStamp(0),timebuf.time - timeOffset,seq,unsigned(pID),timebuf.millitm);	
	}
	else
	{	
		int len = strlen(iUID);
		int offset = (len > 15 ? len-15: len);

		if ( offset != len )
		{
			if ( memchr(iUID+offset,'.',1) )
				offset++;

			//	-- - 8/25/05 - removed '.' between %u and %s to avoid possibility
			//		of '.0' in UID - voilation of encoding rules.
			_snprintf(uniq,sizeof uniq,"%u%u%u%s", 
				GetChecksum(iUID,0),GetChecksum(iUID,-1),GetChecksum(iUID,1),iUID+offset);
		}
		else
		{
			_snprintf(uniq,sizeof uniq,"%u%u%u", 
				GetChecksum(iUID,0),GetChecksum(iUID,-1),GetChecksum(iUID,1));
		}
	}
	
	uniq[sizeof uniq - 1] = '\0';		
	_snprintf(UID, sizeof UID - 1, "%s.%s", rootUID,uniq);	


	// added by shiying hu, to avoid "." at the end
	if ( UID[63] == '.' )
		UID[63] = UID[64];

	UID[64] = '\0';

	if (strlen(UID)&1)
		strcat(UID,"1");
	
	return std::string(UID);
}

#if defined(_DEBUG) && 0

#include "rtvsutil.h" // for DeSpaceDe()

//-----------------------------------------------------------------------
// test the strength of checksums
static void TestChecksum(void)
{
	char *file = "C:/testdata/sop.txt";
	FILE *fp = fopen(file,"r");

	if (!fp)
	{
		assert(0);
		return;
	}

	typedef std::vector<std::string> tList;
	tList in, out;
	std::map<std::string,tList> uniq;
	std::map<std::string,tList>::iterator it;
	char line[128];
	std::string newUID;
	int i;

	for ( ; fgets(line,sizeof line, fp); )
	{
		line[sizeof line - 1] = '\0';
		iRTVDeSpaceDe(line);
		in.push_back(line);
	}
	fclose(fp);

	for (  i = in.size(); --i >= 0; )
	{
		newUID = TRPlatform::GenerateUID(in[i].c_str());
		out.push_back(newUID);

	
		if (uniq.find(newUID) == uniq.end())
		{
			uniq[newUID] = tList();
		}
		
		uniq.find(newUID)->second.push_back(in[i]);
	}

	if (uniq.size() != in.size())
	{
		fprintf(stderr,"Total  %d UID in. %d newUID out\n", in.size(), uniq.size());

		for ( it = uniq.begin(); it != uniq.end(); ++it)
		{
			if (it->second.size() > 1)
			{
				fprintf(stderr,"REPEAT\n");
				for ( int j = 0; j < it->second.size(); j++)
				{
					fprintf(stderr, "in=%s out=%s\n", it->second[j].c_str(), it->first.c_str());
				}
			}
		}
	}
}


#endif

static int IsUNCShare(const char* iPath)
{
	if (!iPath || !*iPath)
		return 0;

	if (iPath[0] != '\\' || iPath[1] != '\\')
		return 0;

	if (strchr(iPath+2,'\\'))
		return 0;

	return 1;

}

//-----------------------------------------------------------------------------
//
const char* TRPlatform::GetMyName()
{
	char *p;
	static char myName[128];

	if (myName[0])
		return myName;
	
	TRPlatform::NetInit();
	gethostname(myName, sizeof(myName)-1);
	if ( ( p = strchr(myName,'.')))
		*p = '\0';

	return myName;
}

//-----------------------------------------------------------------------------
//
static char *ToLower(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = tolower(*s);
	return ret;
}

//-----------------------------------------------------------------------------
//
static char *ToUpper(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = toupper(*s);
	return ret;
}

//-----------------------------------------------------------------------------
//
const char* TRPlatform::GetMyNameLowercase()
{
	static char myNameLower[128];

	if (myNameLower[0])
		return myNameLower;

	strncpy(myNameLower, GetMyName(), sizeof(myNameLower));
	myNameLower[sizeof(myNameLower)-1] = 0;
	
	return ToLower(myNameLower);
}

//-----------------------------------------------------------------------------
//
const char* TRPlatform::GetMyNameUppercase()
{
	static char myNameUpper[128];

	if (myNameUpper[0])
		return myNameUpper;

	strncpy(myNameUpper, GetMyName(), sizeof(myNameUpper));
	myNameUpper[sizeof(myNameUpper)-1] = 0;
	
	return ToUpper(myNameUpper);
}
