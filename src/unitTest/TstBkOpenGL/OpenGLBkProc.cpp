

#include "OpenGLBkProc.h"

 
#include <winioctl.h>
#include <stdlib.h>
#include <process.h>
#include <ntsecapi.h>
#include <locale.h>
#include <time.h>
#include <atlsecurity.h>

#include "AqCore/TRLogger.h"
extern TRLogger gLogger;


COpenGLBkProc::COpenGLBkProc()
{
	 
}
COpenGLBkProc::~COpenGLBkProc()
{
	
}
static char _cmd_line_[2048];
int COpenGLBkProc::CreateProcsseSession( OpenGLProc_COM* p, int newSessionId, const char* full_arg)//, LOGGING* log )
{
	int retcd = -1;
#if 0
	TCHAR  szUserName[] = TEXT("test");
	TCHAR  szPassword[] = TEXT("test");
	

	if (!LogonUser(szUserName, NULL, szPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &p->g_token))
//	if (!LogonUser(szUserName, NULL, szPassword, LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, &p->g_token))
	{
		log->error( "vpsoft_exec_vpsw: can not logon '%s' process.\n", szUserName );		
		return -1;
	}
#endif
#if 0
	DWORD nRetLen=0;
  	GetTokenInformation(p->g_token,TokenStatistics,NULL,NULL,&nRetLen);
  	log->info( "TokenSessionId size=%d, hToken=0x%x", nRetLen, p->g_token);
  	PLUID pTokenID=(PLUID)GlobalAlloc(GPTR,nRetLen);
  	if( !GetTokenInformation( p->g_token,TokenSessionId,pTokenID,nRetLen,&nRetLen))
  	{
		return -1;
	}
	int newSessionId = *(int*)(pTokenID);
	GlobalFree( pTokenID );
#endif

#if 0
	DWORD dwLength=0;
	NTSTATUS                     ns;
	PSECURITY_LOGON_SESSION_DATA pLogonSessionData;

	GetTokenInformation(p->g_token, TokenStatistics, NULL, 0, &dwLength);

	PTOKEN_STATISTICS pTokenStatistics = (PTOKEN_STATISTICS)LocalAlloc(LPTR, dwLength);
	
	GetTokenInformation(p->g_token, TokenStatistics, pTokenStatistics, dwLength, &dwLength);

	ns = LsaGetLogonSessionData(&pTokenStatistics->AuthenticationId, &pLogonSessionData);
	if (LsaNtStatusToWinError(ns) != ERROR_SUCCESS) {
		log->info( "Failded loggon session \n" );
		LocalFree(pTokenStatistics);
		CloseHandle(p->g_token);
		return -1;
	}
	
	newSessionId = pLogonSessionData->Session;

  	log->info( "newSessionId %d\n", newSessionId );
	
	LsaFreeReturnBuffer(pLogonSessionData);
	LocalFree(pTokenStatistics);

//	CloseHandle(hToken);
#endif

#if 0
	DWORD nRetLen=0;
  	GetTokenInformation(p->g_token,TokenSessionId,NULL,NULL,&nRetLen);
  	log->info( "TokenSessionId size=%d, hToken=0x%x", nRetLen, p->g_token);
  	PLUID pTokenID=(PLUID)GlobalAlloc(GPTR,nRetLen);
  	if( !GetTokenInformation( p->g_token,TokenSessionId,pTokenID,nRetLen,&nRetLen))
  	{
		return -1;
	}
	int newSessionId = *(int*)(pTokenID);
	GlobalFree( pTokenID );
#endif
  		
	do
    {
	//	DWORD sessionId = 0;
	    HANDLE svctoken = NULL;
	    HANDLE provtoken = NULL;

	    DWORD dwProcessID = GetCurrentProcessId();

		gLogger.LogMessage("CreateProcsseSession: dwProcessID %d \n",dwProcessID);
		gLogger.FlushLog();

		if ( 0 == ProcessIdToSessionId(dwProcessID,&m_sessionId) )
		{
			break;
		}
        // Create token for new process by duplicating our own token
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &svctoken) == 0)
        {
			gLogger.LogMessage("OpenProcessToken: break \n");
			gLogger.FlushLog();
            break;
        }


        DuplicateTokenEx(svctoken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &provtoken);
        CloseHandle(svctoken);
        svctoken = NULL;


		if ( 0 == m_sessionId  )
		{
			gLogger.LogMessage(" 0 == sessionId \n" );
			gLogger.FlushLog();

	        // Move the new token to the target session
	        if (! SetTokenInformation(provtoken, TokenSessionId, &newSessionId, sizeof(DWORD)))
	        {
				gLogger.LogMessage("SetTokenInformation: break \n");
				gLogger.FlushLog();
	            break;
	        }
			sprintf(_cmd_line_,"%s 0",full_arg);
		}else{
			gLogger.LogMessage(" 0 != sessionId  go on CreateProcessAsUser \n" );
			gLogger.FlushLog();
			sprintf(_cmd_line_,"%s",full_arg);
		}

		
//        STARTUPINFO si = { 0 };
#if 1
        memset( &p->g_si, 0, sizeof( p->g_si ));
        p->g_si.cb = sizeof(p->g_si);
        
//        LPSTR dsktp = "WinSta0\\Default";
//        p->g_si.lpDesktop = dsktp;
#endif
//        wchar_t *dsktp = L"WinSta0\\Default";

        //si.cb = sizeof(si);
        ///si.lpDesktop = dsktp;

		// Spawn new process in target session
	//	retcd = CreateProcessAsUser(provtoken, NULL, (LPSTR)full_arg, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &p->g_si, &p->g_pi );
		retcd = CreateProcessAsUser(provtoken, NULL, (LPSTR)_cmd_line_, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &p->g_si, &p->g_pi );
 

	
//		log->info( "vpsoft_exec_vpsw: session id '%d' <- '%d'\n", newSessionId, sessionId );

		if (provtoken)
		{
			CloseHandle(provtoken);
			provtoken = NULL;
		}
	}
	while(0);
	
	return retcd;
}

PROCESS_INFORMATION _processInfo;
int COpenGLBkProc::startMyProc(OpenGLProc_COM* p,  const char* full_arg)
{
	// Windows Version 判定
	OSVERSIONINFO os;
	memset( &os, 0, sizeof( os ) );
	os.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if ( !GetVersionEx( &os ) )
	{
	}
	gLogger.LogMessage( "startMyProc: os P%d MA%d \n", os.dwPlatformId, os.dwMajorVersion );
	gLogger.FlushLog();
	

	ZeroMemory( &p->g_si, sizeof( p->g_si ) );
	p->g_si.cb = sizeof(p->g_si);
	ZeroMemory( &p->g_pi, sizeof( p->g_pi ) );
 
	// XP か？
	int retcd = 0;
	const DWORD dwPlatformXp = 2;
	const DWORD dwMajorXp    = 5;
	
	
	if ( os.dwPlatformId <= dwPlatformXp && os.dwMajorVersion <= dwMajorXp)
	{
		gLogger.LogMessage( "startMyProc: CreateProcess %s \n" ,full_arg );
		gLogger.FlushLog();
		retcd = CreateProcess (
#if 0
						NULL,
						(LPSTR)full_arg,	// Command line. 
						NULL,	// Process handle not inheritable. 
						NULL,	// Thread handle not inheritable. 
						FALSE,	// Set handle inheritance to FALSE. 
						0,		// No creation flags. 
						NULL,	// Use parent's environment block. 
						NULL,	// Use parent's starting directory. 
 
						&p->g_si,// Pointer to STARTUPINFO structure.
						&p->g_pi // Pointer to PROCESS_INFORMATION structure.
 
#else
 						NULL, (LPSTR)full_arg, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &p->g_si, &p->g_pi
#endif
						 );
	}
	else{

		int newSessionId = 1;
			newSessionId = WTSGetActiveConsoleSessionId();

		gLogger.LogMessage("startMyProc: newSessionId %d \n",newSessionId);
		gLogger.FlushLog();
		retcd = CreateProcsseSession( p, newSessionId, full_arg );
	}
	return retcd;
}



#if 0

#include "vpsoft_dr_com.h"
#include "logvpsw.h"

int CreateProcsseSession( VPSW_COM* p, int newSessionId, const char* full_arg, LOGGING* log )
{
	int retcd = -1;
#if 0
	TCHAR  szUserName[] = TEXT("test");
	TCHAR  szPassword[] = TEXT("test");
	

	if (!LogonUser(szUserName, NULL, szPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &p->g_token))
//	if (!LogonUser(szUserName, NULL, szPassword, LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, &p->g_token))
	{
		log->error( "vpsoft_exec_vpsw: can not logon '%s' process.\n", szUserName );		
		return -1;
	}
#endif
#if 0
	DWORD nRetLen=0;
  	GetTokenInformation(p->g_token,TokenStatistics,NULL,NULL,&nRetLen);
  	log->info( "TokenSessionId size=%d, hToken=0x%x", nRetLen, p->g_token);
  	PLUID pTokenID=(PLUID)GlobalAlloc(GPTR,nRetLen);
  	if( !GetTokenInformation( p->g_token,TokenSessionId,pTokenID,nRetLen,&nRetLen))
  	{
		return -1;
	}
	int newSessionId = *(int*)(pTokenID);
	GlobalFree( pTokenID );
#endif

#if 0
	DWORD dwLength=0;
	NTSTATUS                     ns;
	PSECURITY_LOGON_SESSION_DATA pLogonSessionData;

	GetTokenInformation(p->g_token, TokenStatistics, NULL, 0, &dwLength);

	PTOKEN_STATISTICS pTokenStatistics = (PTOKEN_STATISTICS)LocalAlloc(LPTR, dwLength);
	
	GetTokenInformation(p->g_token, TokenStatistics, pTokenStatistics, dwLength, &dwLength);

	ns = LsaGetLogonSessionData(&pTokenStatistics->AuthenticationId, &pLogonSessionData);
	if (LsaNtStatusToWinError(ns) != ERROR_SUCCESS) {
		log->info( "Failded loggon session \n" );
		LocalFree(pTokenStatistics);
		CloseHandle(p->g_token);
		return -1;
	}
	
	newSessionId = pLogonSessionData->Session;

  	log->info( "newSessionId %d\n", newSessionId );
	
	LsaFreeReturnBuffer(pLogonSessionData);
	LocalFree(pTokenStatistics);

//	CloseHandle(hToken);
#endif

#if 0
	DWORD nRetLen=0;
  	GetTokenInformation(p->g_token,TokenSessionId,NULL,NULL,&nRetLen);
  	log->info( "TokenSessionId size=%d, hToken=0x%x", nRetLen, p->g_token);
  	PLUID pTokenID=(PLUID)GlobalAlloc(GPTR,nRetLen);
  	if( !GetTokenInformation( p->g_token,TokenSessionId,pTokenID,nRetLen,&nRetLen))
  	{
		return -1;
	}
	int newSessionId = *(int*)(pTokenID);
	GlobalFree( pTokenID );
#endif
  		
	do
    {
		DWORD sessionId = 0;
	    HANDLE svctoken = NULL;
	    HANDLE provtoken = NULL;

	    DWORD dwProcessID = GetCurrentProcessId();

		if ( 0 == ProcessIdToSessionId(dwProcessID,&sessionId) )
		{
			break;
		}
        // Create token for new process by duplicating our own token
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &svctoken) == 0)
        {
            break;
        }
        DuplicateTokenEx(svctoken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &provtoken);
        CloseHandle(svctoken);
        svctoken = NULL;


		if ( 0 == sessionId  )
		{
	        // Move the new token to the target session
	        if (! SetTokenInformation(provtoken, TokenSessionId, &newSessionId, sizeof(DWORD)))
	        {
	            break;
	        }
	    }

		
//        STARTUPINFO si = { 0 };
#if 1
        memset( &p->g_si, 0, sizeof( p->g_si ));
        p->g_si.cb = sizeof(p->g_si);
        
//        LPSTR dsktp = "WinSta0\\Default";
//        p->g_si.lpDesktop = dsktp;
#endif
//        wchar_t *dsktp = L"WinSta0\\Default";

        //si.cb = sizeof(si);
        ///si.lpDesktop = dsktp;

		// Spawn new process in target session
		retcd = CreateProcessAsUser(provtoken, NULL, (LPSTR)full_arg, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &p->g_si, &p->g_pi );
//		retcd = CreateProcessAsUser(p->g_token, NULL, (LPSTR)full_arg, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &p->g_si, &p->g_pi );
//		retcd = CreateProcessAsUser(provtoken, NULL, (LPSTR)full_arg, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &p->g_pi );
		//memcpy( &p->g_si, &si, sizeof( si ) );
		log->info( "vpsoft_exec_vpsw: session id '%d' <- '%d'\n", newSessionId, sessionId );

		if (provtoken)
		{
			CloseHandle(provtoken);
			provtoken = NULL;
		}
	}
	while(0);
	
	return retcd;
}

#if 1

#include <windows.h>
#include <Tchar.h>

#define DESKTOP_ALL (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | \
DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | \
DESKTOP_JOURNALPLAYBACK | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | \
DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED)

#define WINSTA_ALL (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | \
WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | \
WINSTA_WRITEATTRIBUTES | WINSTA_ACCESSGLOBALATOMS | \
WINSTA_EXITWINDOWS | WINSTA_ENUMERATE | WINSTA_READSCREEN | \
STANDARD_RIGHTS_REQUIRED)

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | \
GENERIC_EXECUTE | GENERIC_ALL)

BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);

BOOL AddAceToDesktop(HDESK hdesk, PSID psid);

BOOL GetLogonSID (HANDLE hToken, PSID *ppsid);

VOID FreeLogonSID (PSID *ppsid);

//int vpsoft_make_vpsw_exe( const char* full, void* ptr, DWORD sz );


int StartProcessLogon(
	const char * userid, const char* password, const char* full_arg
	, LOGGING* log )
{
	PROCESS_INFORMATION pi = {0}; 
	STARTUPINFO si;

	HANDLE hUser=NULL;
	HANDLE hToken=NULL;

	setlocale(LC_ALL, "");

	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);
	si.lpDesktop = "winsta0\\default";

	int retcd = 0;
	HANDLE hProcSelf = NULL;
	HANDLE hTokenSelf = NULL;
	LUID luid;
	TOKEN_PRIVILEGES tp;
	HWINSTA hwinstaSave = NULL;
	HWINSTA hwinsta = NULL;
	HDESK hdesk = NULL;
	PSID pSid = NULL;


	// 別ユーザーでのログイン(トークン取得)
	BOOL	bRet;
	bRet = LogonUser(userid, NULL, password,
		LOGON32_LOGON_INTERACTIVE,
		LOGON32_PROVIDER_DEFAULT, &hUser);
	if ( false == bRet )
	{
		log->error( "LogonUser failed %s\n", userid );
		retcd = -1;
		goto END;
	}


	// このプロセスに、偽装特権を有効化
	hProcSelf = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	if ( NULL == hProcSelf )
	{
		log->error( "OpenProcess failed\n" );
		retcd = -1;
		goto END;
	}
	
	bRet = OpenProcessToken(hProcSelf, TOKEN_ADJUST_PRIVILEGES, &hTokenSelf);
	if ( false == bRet )
	{
		log->error( "OpenProcessToken failed\n" );
		retcd = -1;
		goto END;
	}
	bRet = LookupPrivilegeValue(NULL, SE_IMPERSONATE_NAME, &luid);
	if ( false == bRet )
	{
		log->error( "LookupPrivilegeValue failed\n" );
		retcd = -1;
		goto END;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	bRet = AdjustTokenPrivileges(hTokenSelf, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if ( false == bRet )
	{
		log->error( "AdjustTokenPrivileges failed\n" );
		retcd = -1;
		goto END;
	}

	// ウィンドウステーションのオープン
	hwinstaSave = GetProcessWindowStation();
	if ( NULL == hwinstaSave )
	{
		log->error( "GetProcessWindowStation failed\n" );
		retcd = -1;
		goto END;
	}
	hwinsta = OpenWindowStation( "winsta0", FALSE, READ_CONTROL | WRITE_DAC);
	if ( NULL == hwinsta )
	{
		log->error( "OpenWindowStation failed\n" );
		retcd = -1;
		goto END;
	}
	hdesk = OpenDesktop(
    	"default", // interactive window station 
		0, // no interaction with other desktop processes
		FALSE, // handle is not inheritable
		READ_CONTROL |
		WRITE_DAC | 
		DESKTOP_WRITEOBJECTS | 
		DESKTOP_READOBJECTS);
	if ( NULL == hdesk )
	{
		log->error( "OpenDesktop failed\n" );
		retcd = -1;
		goto END;
	}
  	bRet = SetProcessWindowStation(hwinsta);
  	if ( false == bRet )
  	{
		log->error( "SetProcessWindowStation failed\n" );
		retcd = -1;
		goto END;
	}

  // ウィンドウステーション/ デスクトップへの DACL 設定
  	bRet = GetLogonSID(hUser, &pSid); // <- 下記参照
  	if ( bRet == false )
  	{
		log->error( "SetProcessWindowStation failed\n" );
		retcd = -1;
		goto END;
	}
	bRet = AddAceToWindowStation(hwinsta, pSid); // <- 下記参照
  	if ( bRet == false )
  	{
		log->error( "AddAceToWindowStation failed\n" );
		retcd = -1;
		goto END;
	}
	bRet = AddAceToDesktop(hdesk, pSid); // <- 下記参照
	if ( bRet == false )
	{
		log->error( "AddAceToDesktop failed\n" );
		retcd = -1;
		goto END;
	}

	// 偽装実行
	bRet = ImpersonateLoggedOnUser(hUser);
	if ( bRet == false )
	{
		log->error( "ImpersonateLoggedOnUser failed %d\n", GetLastError() );
		retcd = -1;
		goto END;
	}

	// File 作成
	// 	vpsoft_make_vpsw_exe( full_arg, ptr, sz );

	// CreateProcessAsUser !! (メモ帳を起動)
//	retcd = CreateProcessAsUser(provtoken, NULL, (LPSTR)full_arg, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &p->g_si, &p->g_pi );
	bRet = CreateProcessAsUser(hUser, NULL, (LPSTR)full_arg,
		NULL, NULL,FALSE, NORMAL_PRIORITY_CLASS | 
		CREATE_NEW_CONSOLE /*DETACHED_PROCESS*/, NULL, NULL, &si, &pi);
	if ( bRet == false )
	{
		log->error( "CreateProcessAsUser failed %d\n", GetLastError() );
		retcd = -1;
	}
	
	// 偽装を戻す
	RevertToSelf();

END:
  if (hwinstaSave)
    SetProcessWindowStation(hwinstaSave);
  if(pi.hProcess)
    CloseHandle(pi.hProcess);
  if(hUser!=NULL)
    CloseHandle(hUser);
  if(hTokenSelf)
    CloseHandle(hTokenSelf);
  if(hwinsta)
    CloseWindowStation(hwinsta);
  if(hdesk)
    CloseDesktop(hdesk);

	if ( retcd )
	{
		return 0;
	}
	else
	{
		return 1;
	}
  }


#include <windows.h>
#pragma comment(lib, "advapi32.lib")

BOOL GetLogonSID (HANDLE hToken, PSID *ppsid) 
{
   BOOL bSuccess = FALSE;
   DWORD dwIndex;
   DWORD dwLength = 0;
   PTOKEN_GROUPS ptg = NULL;

// Verify the parameter passed in is not NULL.
    if (NULL == ppsid)
        goto Cleanup;

// Get required buffer size and allocate the TOKEN_GROUPS buffer.

   if (!GetTokenInformation(
         hToken,         // handle to the access token
         TokenGroups,    // get information about the token's groups 
         (LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
         0,              // size of buffer
         &dwLength       // receives required buffer size
      )) 
   {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
         goto Cleanup;

      ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),
         HEAP_ZERO_MEMORY, dwLength);

      if (ptg == NULL)
         goto Cleanup;
   }

// Get the token group information from the access token.

   if (!GetTokenInformation(
         hToken,         // handle to the access token
         TokenGroups,    // get information about the token's groups 
         (LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
         dwLength,       // size of buffer
         &dwLength       // receives required buffer size
         )) 
   {
      goto Cleanup;
   }

// Loop through the groups to find the logon SID.

   for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++) 
      if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID)
             ==  SE_GROUP_LOGON_ID) 
      {
      // Found the logon SID; make a copy of it.

         dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
         *ppsid = (PSID) HeapAlloc(GetProcessHeap(),
                     HEAP_ZERO_MEMORY, dwLength);
         if (*ppsid == NULL)
             goto Cleanup;
         if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid)) 
         {
             HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
             goto Cleanup;
         }
         break;
      }

   bSuccess = TRUE;

Cleanup: 

// Free the buffer for the token groups.

   if (ptg != NULL)
      HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

   return bSuccess;
}

VOID FreeLogonSID (PSID *ppsid) 
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

BOOL StartInteractiveClientProcess (
    LPTSTR lpszUsername,    // client to log on
    LPTSTR lpszDomain,      // domain of client's account
    LPTSTR lpszPassword,    // client's password
    LPTSTR lpCommandLine    // command line to execute
) 
{
   HANDLE      hToken;
   HDESK       hdesk = NULL;
   HWINSTA     hwinsta = NULL, hwinstaSave = NULL;
   PROCESS_INFORMATION pi;
   PSID pSid = NULL;
   STARTUPINFO si;
   BOOL bResult = FALSE;

// Log the client on to the local computer.

   if (!LogonUser(
           lpszUsername,
           lpszDomain,
           lpszPassword,
           LOGON32_LOGON_INTERACTIVE,
           LOGON32_PROVIDER_DEFAULT,
           &hToken) ) 
   {
      goto Cleanup;
   }

// Save a handle to the caller's current window station.

   if ( (hwinstaSave = GetProcessWindowStation() ) == NULL)
      goto Cleanup;

// Get a handle to the interactive window station.

   hwinsta = OpenWindowStation(
       _T("winsta0"),                   // the interactive window station 
       FALSE,                       // handle is not inheritable
       READ_CONTROL | WRITE_DAC);   // rights to read/write the DACL

   if (hwinsta == NULL) 
      goto Cleanup;

// To get the correct default desktop, set the caller's 
// window station to the interactive window station.

   if (!SetProcessWindowStation(hwinsta))
      goto Cleanup;

// Get a handle to the interactive desktop.

   hdesk = OpenDesktop(
      _T("default"),     // the interactive window station 
      0,             // no interaction with other desktop processes
      FALSE,         // handle is not inheritable
      READ_CONTROL | // request the rights to read and write the DACL
      WRITE_DAC | 
      DESKTOP_WRITEOBJECTS | 
      DESKTOP_READOBJECTS);

// Restore the caller's window station.

   if (!SetProcessWindowStation(hwinstaSave)) 
      goto Cleanup;

   if (hdesk == NULL) 
      goto Cleanup;

// Get the SID for the client's logon session.

   if (!GetLogonSID(hToken, &pSid)) 
      goto Cleanup;

// Allow logon SID full access to interactive window station.

   if (! AddAceToWindowStation(hwinsta, pSid) ) 
      goto Cleanup;

// Allow logon SID full access to interactive desktop.

   if (! AddAceToDesktop(hdesk, pSid) ) 
      goto Cleanup;

// Impersonate client to ensure access to executable file.

   if (! ImpersonateLoggedOnUser(hToken) ) 
      goto Cleanup;

// Initialize the STARTUPINFO structure.
// Specify that the process runs in the interactive desktop.

   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb= sizeof(STARTUPINFO);
   si.lpDesktop = TEXT("winsta0\\default");

// Launch the process in the client's logon session.

   bResult = CreateProcessAsUser(
      hToken,            // client's access token
      NULL,              // file to execute
      lpCommandLine,     // command line
      NULL,              // pointer to process SECURITY_ATTRIBUTES
      NULL,              // pointer to thread SECURITY_ATTRIBUTES
      FALSE,             // handles are not inheritable
      NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,   // creation flags
      NULL,              // pointer to new environment block 
      NULL,              // name of current directory 
      &si,               // pointer to STARTUPINFO structure
      &pi                // receives information about new process
   ); 

// End impersonation of client.

   RevertToSelf();

   if (bResult && pi.hProcess != INVALID_HANDLE_VALUE) 
   { 
      WaitForSingleObject(pi.hProcess, INFINITE); 
      CloseHandle(pi.hProcess); 
   } 

   if (pi.hThread != INVALID_HANDLE_VALUE)
      CloseHandle(pi.hThread);  

Cleanup: 

   if (hwinstaSave != NULL)
      SetProcessWindowStation (hwinstaSave);

// Free the buffer for the logon SID.

   if (pSid)
      FreeLogonSID(&pSid);

// Close the handles to the interactive window station and desktop.

   if (hwinsta)
      CloseWindowStation(hwinsta);

   if (hdesk)
      CloseDesktop(hdesk);

// Close the handle to the client's access token.

   if (hToken != INVALID_HANDLE_VALUE)
      CloseHandle(hToken);  

   return bResult;
}

BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid)
{
   ACCESS_ALLOWED_ACE   *pace = NULL;
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bSuccess = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pacl;
   PACL                 pNewAcl = NULL;
   PSECURITY_DESCRIPTOR psd = NULL;
   PSECURITY_DESCRIPTOR psdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   __try
   {
      // Obtain the DACL for the window station.

      if (!GetUserObjectSecurity(
             hwinsta,
             &si,
             psd,
             dwSidSize,
             &dwSdSizeNeeded)
      )
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
         psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               dwSdSizeNeeded);

         if (psd == NULL)
            __leave;

         psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               dwSdSizeNeeded);

         if (psdNew == NULL)
            __leave;

         dwSidSize = dwSdSizeNeeded;

         if (!GetUserObjectSecurity(
               hwinsta,
               &si,
               psd,
               dwSidSize,
               &dwSdSizeNeeded)
         )
            __leave;
      }
      else
         __leave;

      // Create a new DACL.

      if (!InitializeSecurityDescriptor(
            psdNew,
            SECURITY_DESCRIPTOR_REVISION)
      )
         __leave;

      // Get the DACL from the security descriptor.

      if (!GetSecurityDescriptorDacl(
            psd,
            &bDaclPresent,
            &pacl,
            &bDaclExist)
      )
         __leave;

      // Initialize the ACL.

      ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
      aclSizeInfo.AclBytesInUse = sizeof(ACL);

      // Call only if the DACL is not NULL.

      if (pacl != NULL)
      {
         // get the file ACL size info
         if (!GetAclInformation(
               pacl,
               (LPVOID)&aclSizeInfo,
               sizeof(ACL_SIZE_INFORMATION),
               AclSizeInformation)
         )
            __leave;
      }

      // Compute the size of the new ACL.

      dwNewAclSize = aclSizeInfo.AclBytesInUse +
            (2*sizeof(ACCESS_ALLOWED_ACE)) + (2*GetLengthSid(psid)) -
            (2*sizeof(DWORD));

      // Allocate memory for the new ACL.

      pNewAcl = (PACL)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwNewAclSize);

      if (pNewAcl == NULL)
         __leave;

      // Initialize the new DACL.

      if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
         __leave;

      // If DACL is present, copy it to a new DACL.

      if (bDaclPresent)
      {
         // Copy the ACEs to the new ACL.
         if (aclSizeInfo.AceCount)
         {
            for (i=0; i < aclSizeInfo.AceCount; i++)
            {
               // Get an ACE.
               if (!GetAce(pacl, i, &pTempAce))
                  __leave;

               // Add the ACE to the new ACL.
               if (!AddAce(
                     pNewAcl,
                     ACL_REVISION,
                     MAXDWORD,
                     pTempAce,
                    ((PACE_HEADER)pTempAce)->AceSize)
               )
                  __leave;
            }
         }
      }

      // Add the first ACE to the window station.

      pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) -
                  sizeof(DWORD));

      if (pace == NULL)
         __leave;

      pace->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
      pace->Header.AceFlags = CONTAINER_INHERIT_ACE |
                   INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
      pace->Header.AceSize  = LOWORD(sizeof(ACCESS_ALLOWED_ACE) +
                   GetLengthSid(psid) - sizeof(DWORD));
      pace->Mask            = GENERIC_ACCESS;

      if (!CopySid(GetLengthSid(psid), &pace->SidStart, psid))
         __leave;

      if (!AddAce(
            pNewAcl,
            ACL_REVISION,
            MAXDWORD,
            (LPVOID)pace,
            pace->Header.AceSize)
      )
         __leave;

      // Add the second ACE to the window station.

      pace->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
      pace->Mask            = WINSTA_ALL;

      if (!AddAce(
            pNewAcl,
            ACL_REVISION,
            MAXDWORD,
            (LPVOID)pace,
            pace->Header.AceSize)
      )
         __leave;

      // Set a new DACL for the security descriptor.

      if (!SetSecurityDescriptorDacl(
            psdNew,
            TRUE,
            pNewAcl,
            FALSE)
      )
         __leave;

      // Set the new security descriptor for the window station.

      if (!SetUserObjectSecurity(hwinsta, &si, psdNew))
         __leave;

      // Indicate success.

      bSuccess = TRUE;
   }
   __finally
   {
      // Free the allocated buffers.

      if (pace != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pace);

      if (pNewAcl != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

      if (psd != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

      if (psdNew != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
   }

   return bSuccess;

}

BOOL AddAceToDesktop(HDESK hdesk, PSID psid)
{
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bSuccess = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pacl;
   PACL                 pNewAcl = NULL;
   PSECURITY_DESCRIPTOR psd = NULL;
   PSECURITY_DESCRIPTOR psdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   __try
   {
      // Obtain the security descriptor for the desktop object.

      if (!GetUserObjectSecurity(
            hdesk,
            &si,
            psd,
            dwSidSize,
            &dwSdSizeNeeded))
      {
         if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
         {
            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
                  GetProcessHeap(),
                  HEAP_ZERO_MEMORY,
                  dwSdSizeNeeded );

            if (psd == NULL)
               __leave;

            psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
                  GetProcessHeap(),
                  HEAP_ZERO_MEMORY,
                  dwSdSizeNeeded);

            if (psdNew == NULL)
               __leave;

            dwSidSize = dwSdSizeNeeded;

            if (!GetUserObjectSecurity(
                  hdesk,
                  &si,
                  psd,
                  dwSidSize,
                  &dwSdSizeNeeded)
            )
               __leave;
         }
         else
            __leave;
      }

      // Create a new security descriptor.

      if (!InitializeSecurityDescriptor(
            psdNew,
            SECURITY_DESCRIPTOR_REVISION)
      )
         __leave;

      // Obtain the DACL from the security descriptor.

      if (!GetSecurityDescriptorDacl(
            psd,
            &bDaclPresent,
            &pacl,
            &bDaclExist)
      )
         __leave;

      // Initialize.

      ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
      aclSizeInfo.AclBytesInUse = sizeof(ACL);

      // Call only if NULL DACL.

      if (pacl != NULL)
      {
         // Determine the size of the ACL information.

         if (!GetAclInformation(
               pacl,
               (LPVOID)&aclSizeInfo,
               sizeof(ACL_SIZE_INFORMATION),
               AclSizeInformation)
         )
            __leave;
      }

      // Compute the size of the new ACL.

      dwNewAclSize = aclSizeInfo.AclBytesInUse +
            sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid(psid) - sizeof(DWORD);

      // Allocate buffer for the new ACL.

      pNewAcl = (PACL)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwNewAclSize);

      if (pNewAcl == NULL)
         __leave;

      // Initialize the new ACL.

      if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
         __leave;

      // If DACL is present, copy it to a new DACL.

      if (bDaclPresent)
      {
         // Copy the ACEs to the new ACL.
         if (aclSizeInfo.AceCount)
         {
            for (i=0; i < aclSizeInfo.AceCount; i++)
            {
               // Get an ACE.
               if (!GetAce(pacl, i, &pTempAce))
                  __leave;

               // Add the ACE to the new ACL.
               if (!AddAce(
                  pNewAcl,
                  ACL_REVISION,
                  MAXDWORD,
                  pTempAce,
                  ((PACE_HEADER)pTempAce)->AceSize)
               )
                  __leave;
            }
         }
      }

      // Add ACE to the DACL.

      if (!AddAccessAllowedAce(
            pNewAcl,
            ACL_REVISION,
            DESKTOP_ALL,
            psid)
      )
         __leave;

      // Set new DACL to the new security descriptor.

      if (!SetSecurityDescriptorDacl(
            psdNew,
            TRUE,
            pNewAcl,
            FALSE)
      )
         __leave;

      // Set the new security descriptor for the desktop object.

      if (!SetUserObjectSecurity(hdesk, &si, psdNew))
         __leave;

      // Indicate success.

      bSuccess = TRUE;
   }
   __finally
   {
      // Free buffers.

      if (pNewAcl != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

      if (psd != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

      if (psdNew != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
   }

   return bSuccess;
}
#endif



#endif
