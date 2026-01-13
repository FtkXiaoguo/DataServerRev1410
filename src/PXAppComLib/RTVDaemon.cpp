/***********************************************************************
 * RTVDaemon.cpp
 *---------------------------------------------------------------------
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#ifndef RTVDAEMON_H_
#include "RTVDaemon.h"
#endif

//-------------------------------------------------------------------
#include <vector>
class ServiceStateGuard
{
public:
	ServiceStateGuard(void):m_scLock(0) {}
	~ServiceStateGuard(void) { Cleanup(); }

	void AddServieHandle(SC_HANDLE h)
	{
		m_scHandles.push_back(h);
	}
	
	void Cleanup()
	{
		try
		{
			if (m_scLock)
			{
				UnlockServiceDatabase(m_scLock); 
				m_scLock = 0;
			}
			
			int i, N;
			
			for ( N = m_scHandles.size(), i = 0; i < N; ++i)
			{
				if (m_scHandles[i])
				{
					CloseServiceHandle(m_scHandles[i]);
					m_scHandles[i] = 0;
				}
			}
			
			
		} 
		catch (...) {}
		
		m_scHandles.clear();
	}

	
	SC_LOCK					m_scLock; 
	std::vector<SC_HANDLE>	m_scHandles;
};

//-----------------------------------------------------------------------------
/* required C interface to the service routines */
static VOID WINAPI service_ctrl(DWORD dwCtrlCode);
static VOID WINAPI service_main(DWORD dwArgc, char **lpszArgv);

static RTVDaemonProcessor*	sProcessor;
static RTVDaemon*			sThis;

std::string RTVDaemon::m_action;
//----------------------------------------------------------------------------
RTVDaemon::RTVDaemon(RTVDaemonProcessor* iP)
{
	m_processor = iP;
	sThis = this;
	m_debug = 0;
	m_error = 0;
	m_action = "restart";
	memset(&m_sStatus, 0, sizeof(m_sStatus));
}

//----------------------------------------------------------------------------
static VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
    // Handle the requested control code.
    //
    switch(dwCtrlCode)
    {
        case SERVICE_CONTROL_STOP:
            sThis->ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            sProcessor->Stop();
			sThis->ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            return;

        case SERVICE_CONTROL_INTERROGATE:
            break;

		case SERVICE_CONTROL_SHUTDOWN:
			sProcessor->OnSystemShutDown();
			break;

        /* ignore the unknown control code. */
        default:
            break;

    }

    sThis->ReportStatusToSCMgr(sThis->m_sStatus.dwCurrentState, 
		                       NO_ERROR, 0);
}

//---------------------------------------------------------------------
static VOID WINAPI service_main(DWORD dwArgc, char **lpszArgv)
{
       // register our service control handler:
    //
    sThis->m_statusHandle = RegisterServiceCtrlHandler(sProcessor->GetName(),
		                                               service_ctrl);

    if (!sThis->m_statusHandle)
        goto cleanup;

    // SERVICE_STATUS members that don't change in example
    //
    sThis->m_sStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    sThis->m_sStatus.dwServiceSpecificExitCode = 0;

    // report the status to the service control manager.
    //
    if (!sThis->ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        goto cleanup;

	if(sProcessor->PreProcess() != NO_ERROR)
	    goto cleanup;

	
	if (!sThis->ReportStatusToSCMgr(
        SERVICE_RUNNING,       // service state
        NO_ERROR,              // exit code
        0))                    // wait hint
        goto cleanup;

    sProcessor->Process( dwArgc, lpszArgv );

cleanup:

	if (sThis->m_statusHandle)
		(VOID)sThis->ReportStatusToSCMgr(
				SERVICE_STOP_PENDING,
				NO_ERROR,//sThis->m_error,
				0);

	sProcessor->PostProcess();

    // try to report the stopped status to the service control manager.
    //
    if (sThis->m_statusHandle)
        (VOID)sThis->ReportStatusToSCMgr(
				SERVICE_STOPPED,
				NO_ERROR,//sThis->m_error,
				0);

    return;
}

//---------------------------------------------------------------------
int RTVDaemon::ReportStatusToSCMgr(DWORD dwCurrentState,
                                   DWORD dwWin32ExitCode,
                                   DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;
	
	
    if ( !m_debug ) // we don't report to the SCM if running as a console program
    {
        if (dwCurrentState == SERVICE_START_PENDING)
            m_sStatus.dwControlsAccepted = 0;
        else
            m_sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
		
        m_sStatus.dwCurrentState = dwCurrentState;
        m_sStatus.dwWin32ExitCode = dwWin32ExitCode;
        m_sStatus.dwWaitHint = dwWaitHint;
		
        if ( ( dwCurrentState == SERVICE_RUNNING ) ||
			( dwCurrentState == SERVICE_STOPPED ) )
            m_sStatus.dwCheckPoint = 0;
        else
            m_sStatus.dwCheckPoint = dwCheckPoint++;
		
		
        // Report the status of the service to the service control manager.
        //
        if (!(fResult = SetServiceStatus( m_statusHandle, &m_sStatus))) {
            AddToMessageLog(TEXT("SetServiceStatus"));
        }
    }
    return fResult;
}

//---------------------------------------------------------------------
char* RTVDaemon:: GetLastErrorText( char* lpszBuf,long dwSize )
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

//---------------------------------------------------------------------
char* RTVDaemon::GetLastErrorText(void)
{
	static char bufs[8][512];
	static int iBuf;
	char *buf;
	
	buf = bufs[iBuf = ((iBuf+1)&7)];
	
	return GetLastErrorText(buf, 512);
}

//---------------------------------------------------------------------
void RTVDaemon::HandleFailureAction(SC_HANDLE shandle, const char* action)
{
	// handle failure actions
	SERVICE_FAILURE_ACTIONS failure;
	SC_ACTION act;
	act.Type = SC_ACTION_NONE;
	
	failure.lpCommand = 0;
	failure.lpRebootMsg = 0;
	failure.dwResetPeriod = 120*(1000);
	failure.cActions = 1;
	failure.lpsaActions = &act;
	
	if (*action == 0 || _stricmp(action,"restart") ==0)
	{
		act.Type = SC_ACTION_RESTART;
		act.Delay = 5*(1000); // 5 seconds
	}
		
	if (ChangeServiceConfig2(shandle,SERVICE_CONFIG_FAILURE_ACTIONS,&failure) == 0)
	{
		_tprintf(TEXT("ChangeServiceConfig2 failed - %s\n"), GetLastErrorText());		
	}
}

//---------------------------------------------------------------------
void RTVDaemon::CmdInstallService(int iAutoStart)
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
	
    TCHAR szPath[512];
	
    if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
    {
        _tprintf(TEXT("Unable to install %s - %s\n"), m_processor->GetName(), 
			         GetLastErrorText());
        return;
    }
	
    schSCManager = OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);

	//	Need to convert to double-null terminated array of null-terminated strings
	char dependencies[512];
	char *dp = dependencies;
	for(int i = 0; i < m_dependencies.size(); i++)
	{
		strncpy(dp, m_dependencies[i].c_str(), sizeof(dependencies) - (dp - dependencies));
		dependencies[sizeof(dependencies)-1] = 0;

		dp += m_dependencies[i].size();
		*dp = 0;	//	NULL terminator for each service name
		dp++;
	}
	*dp = 0;	//	Double NULL terminator (sentinel for entire array)

    if ( schSCManager )
    {
        schService = CreateService(
            schSCManager,               // SCManager database
            m_processor->GetName(),        // name of service
            m_processor->GetDisplayName(), // name to display
            SERVICE_ALL_ACCESS,         // desired access
            SERVICE_WIN32_OWN_PROCESS,  // service type
            iAutoStart ? SERVICE_AUTO_START: SERVICE_DEMAND_START, // start type
            SERVICE_ERROR_NORMAL,       // error control type
            szPath,                     // service's binary
            NULL,                       // no load ordering group
            NULL,                       // no tag identifier
            dependencies,				// dependencies
            NULL,                       // LocalSystem account
            NULL);                      // no password
		
        if ( schService )
        {
            _tprintf(TEXT("%s installed (start: %s recovery: %s).\n"), m_processor->GetDisplayName(),
				          iAutoStart ?"auto":"manual",m_action.c_str());
        }
        else
        {
            _tprintf(TEXT("CreateService failed - %s\n"), GetLastErrorText());
        }


		HandleFailureAction(schService,m_action.c_str());

		CloseServiceHandle(schService);
		
        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText());
}

enum 
{ 
	kManual = 1, 
	kAuto	= 2, 
	kRemove = 4, 
	kConsole= 8, 
	kStart	= 16,
	kStop	= 32,
	kDebug	= kConsole
};

//----------------------------------------------------------------------------
static void Help(const char *cmd)
{
	_tprintf(TEXT("Usage: %s [-install|-auto|-debug|-remove|-run|-help] [-depends <svcName> ...]\n"), cmd);
	_tprintf(TEXT("   -install  Installs the service manual start\n"));
	_tprintf(TEXT("   -auto     Installs the service auto start\n"));
	_tprintf(TEXT("   -remove   Removes a previously installed service\n"));
	_tprintf(TEXT("   -console  Runs the service as a console application\n"));
	_tprintf(TEXT("   -start    Installs and re-starts the service\n"));
	_tprintf(TEXT("   -stop     stop the service without removing the service\n"));
	_tprintf(TEXT("   -action  [none|restart] what to do when service fails\n"));
	_tprintf(TEXT("   -depends  indicate which services this one depends on\n"));
}

//----------------------------------------------------------------------------
int RTVDaemon::Start(int argc, char **argv)
{		
	SERVICE_TABLE_ENTRY dispatchTable[]=
    {
        { (char *)m_processor->GetName(), service_main },
        { NULL, NULL }
    };


	sProcessor = m_processor;

	unsigned int options = 0;

	for ( int i = 1; i < argc; i++)
	{
		if (_stricmp(argv[i],"-install")==0)
			options |= kManual;
		else if (_stricmp(argv[i],"-stop")==0)
			options |= kStop;
		else if (_stricmp(argv[i],"-auto")==0)
			options |= kAuto;
		else if (_stricmp(argv[i],"-remove") == 0)
			options |= kRemove;
		else if (_stricmp(argv[i],"-debug") == 0 || _stricmp(argv[i],"-console")==0)
			options = kDebug; // override all other options
		else if (_stricmp(argv[i],"-run") == 0 || _stricmp(argv[i],"-start")==0)
			options |= kStart;
		else if (_stricmp(argv[i],"-action") == 0)
		{
			m_action = (argc > i+1) ? argv[++i]:"none";
		}
		else if (_stricmp(argv[i],"-depends") == 0)
		{
			//	Not sure if spaces are allowed, but let's assume they are and handle it
			std::string svcName;
			while (argc > i + 1)			
			{
				++i;
				if (argv[i][0] == '-')
				{
					--i;
					break;
				}

				svcName += argv[i];
				if (argc > i + 1 && argv[i+1][0] != '-')
					svcName += " ";
			} 

			if (svcName.size() > 0)
				m_dependencies.push_back(svcName);
		}
		else if (_stricmp(argv[i],"-h") == 0 || _stricmp(argv[i],"-help") == 0)
		{
			 Help(argv[0]);
			 exit(0);
		}
		else
		{
			_tprintf(TEXT("Application option %s - ignored\n"), argv[i]);
        }
    }


	if ((options & kDebug))
	{
		CmdDebugService(argc, argv);
	    return 0;
	}

	if ((options & kStop))
	{
		CmdStopService();
	}

	if ((options & kRemove))
	{
		CmdRemoveService();
	}

	if ((options & kManual) && (options&kAuto))
	{
		_tprintf(TEXT("Conflict options -install and -auto. Using -auto"));
		options &= ~kManual;
	}

	if ((options&kManual) || (options&kAuto))
	{
		CmdInstallService((options&kAuto));
	}

	if ((options & kStart))
	{
		if (!(options&kManual) && !(options&kAuto))
		{
			options |= kAuto;
			CmdInstallService((options&kAuto));
		}

		ReStartService(m_processor->GetName());
	}

	if (options != 0)
		exit(0);

	if (!StartServiceCtrlDispatcher(dispatchTable))
	{
		 fprintf(stderr,"Can't run the service");
         return -1;
	}

	return 0;
}

//----------------------------------------------------------------------------
static SC_HANDLE GetServiceHandle(const char *iServiceName, SC_HANDLE& schSCManager)
{
	SC_HANDLE   schService;

	schSCManager = OpenSCManager(
				NULL,                   // machine (NULL == local)
				NULL,                   // database (NULL == default)
				SC_MANAGER_ALL_ACCESS   // access required
				);

	if (!schSCManager)
	{		
		_tprintf(TEXT("OpenSCManager failed - %s\n"), RTVDaemon::GetLastErrorText());
		 return 0;
	}

	schService = OpenService(schSCManager, iServiceName, SERVICE_ALL_ACCESS);

	if (!schService)
	{
		_tprintf(TEXT("OpenService (%s) failed - %s\n"), iServiceName, RTVDaemon::GetLastErrorText());
		CloseServiceHandle(schSCManager);
		schSCManager = 0;
	}

	return schService;
}

//----------------------------------------------------------------------------
int RTVDaemon::StopService(const char* iServiceName)
{
	SERVICE_STATUS	sStatus;
	SC_HANDLE		schService;
	SC_HANDLE		schSCManager;
	int ntries;
    
	if (!(schService = GetServiceHandle(iServiceName, schSCManager)))
		return 0;

	if ( ControlService( schService, SERVICE_CONTROL_STOP,&sStatus ))
	{
		_tprintf(TEXT("Stopping %s."), iServiceName);
		Sleep( 1000 );
		
		for ( ntries =0; ntries < 5 &&QueryServiceStatus(schService, &sStatus); ntries++)
		{
			if ( sStatus.dwCurrentState == SERVICE_STOP_PENDING )
			{
				_tprintf(TEXT("."));
				Sleep( 1000 );
			}
			else
				break;
		}
		
		if ( sStatus.dwCurrentState == SERVICE_STOPPED )
			_tprintf(TEXT("\n%s stopped.\n"), iServiceName );
		else
			_tprintf(TEXT("\n%s failed to stop.\n"), iServiceName);	
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return (sStatus.dwCurrentState == SERVICE_STOPPED);
}

//----------------------------------------------------------------------------
BOOL	RTVDaemon::MyStartService(const char *iServiceName)
{
 
	SERVICE_STATUS	sStatus;
	SC_HANDLE		schService;
	SC_HANDLE		schSCManager;
	int ntries;
	static int outCounter;
    
	if (!(schService = GetServiceHandle(iServiceName,schSCManager)))
		return FALSE;
	
	QueryServiceStatus(schService, &sStatus);
	if (sStatus.dwCurrentState == SERVICE_RUNNING)
	{
#ifdef _DEBUG
		_tprintf(TEXT("%s already running\n"), iServiceName)
#endif
			;
	}
	else
	{
		// start the service
		if (::StartService(schService, 0, 0))
		{
			for (ntries =0; ntries < 5 && QueryServiceStatus(schService, &sStatus); ntries++)
			{
				if ( sStatus.dwCurrentState == SERVICE_START_PENDING )
				{
					_tprintf(TEXT("."));
					Sleep( 1000 );
				}
				else
					break;
			}
			
			if ((outCounter++%5)==0)
			{
				if ( sStatus.dwCurrentState == SERVICE_RUNNING )
					_tprintf(TEXT("\n%s running.\n"), iServiceName );
				else
					_tprintf(TEXT("\n%s failed to start.\n"), iServiceName);
			}
		}
		else
		{
			if ((outCounter++%15) == 0)
				_tprintf(TEXT("StartService failed - %s\n"), GetLastErrorText());
		}
	}
	
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	
	return sStatus.dwCurrentState == SERVICE_RUNNING;
 
}

//----------------------------------------------------------------------------
// return non-zero for success
BOOL	RTVDaemon::ReStartService(const char *iServiceName)
{
	SERVICE_STATUS	sStatus;
	SC_HANDLE		schService;
	SC_HANDLE		schSCManager;
	int ntries;
    
	if (!(schService = GetServiceHandle(iServiceName,schSCManager)))
		return FALSE;

	QueryServiceStatus(schService, &sStatus);

	if (sStatus.dwCurrentState == SERVICE_RUNNING)
	{
		if ( ControlService( schService, SERVICE_CONTROL_STOP,&sStatus ))
		{
			_tprintf(TEXT("Stopping %s."), iServiceName);
			Sleep( 1000 );
			
			for ( ntries = 0; ntries < 5 && QueryServiceStatus(schService, &sStatus); ntries++)
			{
				if ( sStatus.dwCurrentState == SERVICE_STOP_PENDING )
				{
					_tprintf(TEXT("."));
					Sleep( 1000 );
				}
				else
					break;
			}
			
			if ( sStatus.dwCurrentState == SERVICE_STOPPED )
				_tprintf(TEXT("\n%s stopped.\n"), iServiceName );
			else
				_tprintf(TEXT("\n%s failed to stop.\n"), iServiceName);	
		}
	}

	HandleFailureAction(schService,m_action.c_str());

	// start the service
	if (::StartService(schService, 0, 0))
	{
		for (ntries =0; ntries < 5 && QueryServiceStatus(schService, &sStatus); ntries++)
		{
			if ( sStatus.dwCurrentState == SERVICE_START_PENDING )
			{
				_tprintf(TEXT("."));
				Sleep( 1000 );
			}
			else
				break;
		}

		if ( sStatus.dwCurrentState == SERVICE_RUNNING )
			_tprintf(TEXT("\n%s running.\n"), iServiceName );
		else
			_tprintf(TEXT("\n%s failed to start.\n"), iServiceName);
	}
	else
		_tprintf(TEXT("StartService failed - %s\n"), GetLastErrorText());

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return sStatus.dwCurrentState == SERVICE_RUNNING;
}

//--------------------------------------------------------------------
void RTVDaemon::CmdStopService(void)
{
	StopService(m_processor->GetName());
}

//--------------------------------------------------------------------
void RTVDaemon::CmdRemoveService(void)
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
	int ntries;
	
    schSCManager = OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, m_processor->GetName(), SERVICE_ALL_ACCESS);
		
        if (schService)
        {
            // try to stop the service
            if ( ControlService( schService, 
				                 SERVICE_CONTROL_STOP,
				                 &m_sStatus ) )
            {
                _tprintf(TEXT("Stopping %s."), m_processor->GetDisplayName());
                Sleep( 1000 );
				
				for (ntries = 0; ntries < 5 && QueryServiceStatus( schService, &m_sStatus ); ntries++)
                {
                    if ( m_sStatus.dwCurrentState == SERVICE_STOP_PENDING )
                    {
                        _tprintf(TEXT("."));
                        Sleep( 1000 );
                    }
                    else
                        break;
                }
				
                if ( m_sStatus.dwCurrentState == SERVICE_STOPPED )
                    _tprintf(TEXT("\n%s stopped.\n"), m_processor->GetDisplayName() );
                else
                    _tprintf(TEXT("\n%s failed to stop.\n"), m_processor->GetDisplayName());
				
            }
			
            // now remove the service
            if( DeleteService(schService) )
                _tprintf(TEXT("%s removed.\n"), m_processor->GetDisplayName() );
            else
                _tprintf(TEXT("DeleteService failed - %s\n"), GetLastErrorText());
			
			
            CloseServiceHandle(schService);
        }
        else
            _tprintf(TEXT("OpenService (%s) failed - %s\n"), m_processor->GetName(), GetLastErrorText());
		
        CloseServiceHandle(schSCManager);
    }
    else
	{
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText());
	}
}

//--------------------------------------------------------------------
//
//	oState can be:
//
//		SERVICE_STOPPED
//		SERVICE_STOP_PENDING
//		SERVICE_START_PENDING
//		SERVICE_RUNNING 
//		SERVICE_CONTINUE_PENDING
//		SERVICE_PAUSE_PENDING
//		SERVICE_PAUSED 
//
BOOL RTVDaemon::GetServiceState(const char *iServiceName, unsigned long& oState)
{
    SERVICE_STATUS ssStatus; 
    SC_HANDLE schSCManager;

	oState = 0;
 
    schSCManager = OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);

    // Open a handle to the service. 
    SC_HANDLE schService = OpenService( 
        schSCManager,
        iServiceName,
        SERVICE_ALL_ACCESS);
	
    if (schService == NULL) 
    {
		_tprintf(TEXT("OpenService failed %d"), GetLastErrorText());
 		CloseServiceHandle(schSCManager);
		return FALSE;
    }
 
    if (!QueryServiceStatus(schService,				// handle to service 
					    &ssStatus) )			// address of status info 
    {
        printf("ControlService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
        return FALSE;
    }

 
	oState = ssStatus.dwCurrentState;
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);	
	
	return TRUE;
}


//--------------------------------------------------------------------
BOOL RTVDaemon::GetServiceStartType(const char* iServiceName, int& oStartType)
{
   
    BOOL bSuccess=TRUE;
	DWORD dwBytesNeeded; 

    // Need to acquire database lock before reconfiguring. 
    SC_HANDLE schSCManager;
    schSCManager = OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);

    // Open a handle to the service. 
    SC_HANDLE schService = OpenService( 
        schSCManager,           // SCManager database 
        iServiceName,           // name of service 
        SERVICE_QUERY_CONFIG); // need CHANGE access 
   
	if (schService == NULL) 
    { 
		oStartType = -1; // special case of service is not installed at all
		CloseServiceHandle(schSCManager);	
        return TRUE;
    }
 
	LPQUERY_SERVICE_CONFIG lpqscBuf; 
	lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc( 
        LPTR, 4096); 
    if (lpqscBuf == NULL) {
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);	
		return FALSE;
	}

	bSuccess = QueryServiceConfig( 
        schService, 
        lpqscBuf, 
        4096, 
        &dwBytesNeeded);


	if (bSuccess) 
    {
		oStartType = lpqscBuf->dwStartType;
    }
 
	LocalFree(lpqscBuf); 
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);	
  
    return bSuccess;
}


//--------------------------------------------------------------------
BOOL RTVDaemon::SetServiceStartType(const char* iServiceName, int iStartType)
{
    SC_LOCK sclLock; 
    LPQUERY_SERVICE_LOCK_STATUS lpqslsBuf;
    DWORD dwBytesNeeded, dwStartType; 
    BOOL bSuccess=TRUE;

	ServiceStateGuard guard;
 
    // Need to acquire database lock before reconfiguring. 
    SC_HANDLE schSCManager;
    schSCManager = OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);

	guard.AddServieHandle(schSCManager);

    // If the database cannot be locked, report the details. 
	sclLock = LockServiceDatabase(schSCManager); 
    if (sclLock == NULL) 
    { 
        // Exit if the database is not locked by another process. 
		if (GetLastError() != ERROR_SERVICE_DATABASE_LOCKED) 
        {
            _tprintf(TEXT("LockServiceDatabase failed (%d)\n"), GetLastErrorText());
            return FALSE;
        }
		
        // Allocate a buffer to get details about the lock. 
        lpqslsBuf = (LPQUERY_SERVICE_LOCK_STATUS) LocalAlloc( 
            LPTR, sizeof(QUERY_SERVICE_LOCK_STATUS)+256); 
        if (lpqslsBuf == NULL) 
        {
            _tprintf(TEXT("LocalAlloc failed (%d)\n"), GetLastErrorText()); 
            return FALSE;
        }
		
        // Get and print the lock status information. 
        if (!QueryServiceLockStatus( 
            schSCManager, 
            lpqslsBuf, 
            sizeof(QUERY_SERVICE_LOCK_STATUS)+256, 
            &dwBytesNeeded) ) 
        {
            _tprintf(TEXT("QueryServiceLockStatus failed (%d)"), GetLastErrorText()); 
            return FALSE;
        }
        LocalFree(lpqslsBuf); 
    } 

	guard.m_scLock = sclLock;

    // The database is locked, so it is safe to make changes. 
	
    // Open a handle to the service. 
    SC_HANDLE schService = OpenService( 
        schSCManager,           // SCManager database 
        iServiceName,           // name of service 
        SERVICE_CHANGE_CONFIG); // need CHANGE access 
   
	if (schService == NULL) 
    {
        _tprintf(TEXT("OpenService failed (%d)\n"), GetLastErrorText()); 
        return FALSE;
    }

	guard.AddServieHandle(schService);

	switch (iStartType)
	{
	case kDaemonAutoStart:
		dwStartType = SERVICE_AUTO_START;
		break;
	case kDaemonManualStart:
		dwStartType = SERVICE_DEMAND_START;
		break;
	case kDaemonDisabled:
		dwStartType = SERVICE_DISABLED;
		break;
	default:
        _tprintf(TEXT("Invalid Service Start Type (%d)\n"), iStartType); 
		return FALSE;
		break;
	};
	
    // Make the changes. 
    if (! ChangeServiceConfig( 
        schService,        // handle of service 
        SERVICE_NO_CHANGE, // service type: no change 
        dwStartType,       // change service start type 
        SERVICE_NO_CHANGE, // error control: no change 
        NULL,              // binary path: no change 
        NULL,              // load order group: no change 
        NULL,              // tag ID: no change 
        NULL,              // dependencies: no change 
        NULL,              // account name: no change 
        NULL,              // password: no change 
        NULL) )            // display name: no change
    {
        _tprintf(TEXT("ChangeServiceConfig failed (%d)\n"), GetLastErrorText()); 
        bSuccess = FALSE;
    }
	
    return bSuccess;
}

//--------------------------------------------------------------------
static BOOL WINAPI ControlHandler ( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
	case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
	case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
		_tprintf(TEXT("Stopping %s"), sProcessor->GetDisplayName());
		sProcessor->Stop();
		_tprintf(TEXT("%s Stopped"),sProcessor->GetDisplayName());
		return TRUE;
		break;
		
    }
    return FALSE;
}

//--------------------------------------------------------------------
void RTVDaemon::CmdDebugService(int argc, char ** argv)
{
    DWORD dwArgc;
    LPTSTR *lpszArgv;
	
#ifdef UNICODE
    lpszArgv = CommandLineToArgvW(GetCommandLineW(), &(dwArgc) );
#else
    dwArgc   = (DWORD) argc;
    lpszArgv = argv;
#endif
	
    _tprintf(TEXT("Running %s as console application [%s]\n"), m_processor->GetDisplayName(), m_action.c_str());
	
    SetConsoleCtrlHandler( ControlHandler, TRUE );

	if (m_processor->PreProcess() == NO_ERROR)	
        m_processor->Process( dwArgc, lpszArgv );

	m_processor->PostProcess();
}

//----------------------------------------------------------------------------
VOID RTVDaemon::AddToMessageLog(LPTSTR lpszMsg)
{
    TCHAR   szMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];
	
	
    if ( !m_debug )
    {
        m_error = GetLastError();
		
        // Use event logging to log the error.
        //
        hEventSource = RegisterEventSource(NULL, m_processor->GetName());
		
        _stprintf(szMsg, TEXT("%s error: %d"), m_processor->GetName(), m_error);
        lpszStrings[0] = szMsg;
        lpszStrings[1] = lpszMsg;
		
        if (hEventSource != NULL) {
            ReportEvent(hEventSource, // handle of event source
                EVENTLOG_ERROR_TYPE,  // event type
                0,                    // event category
                0,                    // event ID
                NULL,                 // current user's SID
                2,                    // strings in lpszStrings
                0,                    // no bytes of raw data
                (const char **)lpszStrings,          // array of error strings
                NULL);                // no raw data
			
            (VOID) DeregisterEventSource(hEventSource);
        }
    }
}

//----------------------------------------------------------------------------
// this is freaky
int RTVDaemonProcessor::OnConsole(void)
{
	return sThis ? sThis->OnConsole():0;
}