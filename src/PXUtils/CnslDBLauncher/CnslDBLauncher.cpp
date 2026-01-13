// CephLauncher.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "CnslDBLauncher.h"
#include "CnslDBLauncherDlg.h"
 
 
#include "PMSDB.h"

#include "AqCore/TRLogger.h"
TRLogger g_pxLogger_;


#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CMyCommandLineInfo : public CCommandLineInfo
{
public:
	// Sets default values
	CMyCommandLineInfo(){
		m_R_Flag = false;
		m_R_Val = 0;
	};

	// plain char* version on UNICODE for source-code backwards compatibility
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
	{
		 
		if(bFlag == TRUE){
			if( CString(pszParam) == "R"){
				m_R_Flag = true;
			}
		}else{
			if(m_R_Flag){
				m_R_Val = atoi(pszParam);
				m_R_Flag = false;
			}else{
				m_PatientID = pszParam;
			}

		}
	}

///
	CString m_PatientID;
		
	int  m_R_Val;
protected:
	bool m_R_Flag;
	
};

// CCnslDBLauncherApp

BEGIN_MESSAGE_MAP(CCnslDBLauncherApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCnslDBLauncherApp コンストラクション

CCnslDBLauncherApp::CCnslDBLauncherApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


char gAppHomeDirBuff[2*MAX_PATH+1]={0,};
void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	size_t pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);
#if 1
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
#endif
	strcpy_s(gAppHomeDirBuff,2*MAX_PATH,sub_str_temp.c_str());

  }else{
	  gAppHomeDirBuff[0] = 0;
  }
}

// 唯一の CCnslDBLauncherApp オブジェクトです。

CCnslDBLauncherApp theApp;


int g_theApp_return_code = 0;

// CCnslDBLauncherApp 初期化

void runCmd(const char *cmdLine,DWORD &nExitCode,bool wait);
void runCmdCephScan(const CString &fileName,DWORD &nExitCode);


bool importCnslDB(const std::string &PatientID,CPMSDB::EXE_CODE &error_code,std::string &msg);

void runCnslApp();
static CMyCommandLineInfo cmdInfo;
BOOL CCnslDBLauncherApp::InitInstance()
{
	
	getHomeFromModulePath();

	CString logFileName;
	logFileName.Format("%slog\\CnslDBLauncher.log",gAppHomeDirBuff);
	g_pxLogger_.SetLogFile((LPCTSTR)logFileName);


	//#17 2012/09/26 
	
	ParseCommandLine(cmdInfo);


	CString start_msg = "CCnslDBLauncherApp::InitInstance  ";
	if(!cmdInfo.m_PatientID.IsEmpty()){
		start_msg = start_msg + CString("ID: [ ")+ cmdInfo.m_PatientID+ CString("]");
	}

	CString return2CnslApp;
	return2CnslApp.Format("Return to CnslApp [%d] \n ",cmdInfo.m_R_Val);
	start_msg = start_msg + return2CnslApp;
	
	g_pxLogger_.SetLogLevel(4);
	//////////
	g_pxLogger_.LogMessage("\n\n========================================\n");
	g_pxLogger_.LogMessage((LPCTSTR)start_msg);
	g_pxLogger_.FlushLog();

#if 1
	const CString strMutex = "_CnslDBLauncherApp_xxx_MUTEX";
	HANDLE hMutex = ::CreateMutex(NULL, TRUE, strMutex);
	if(hMutex==NULL)
	{
		return FALSE;
	}
	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		 
		g_pxLogger_.LogMessage("error CnslDBLauncherApp already exists \n");
		g_pxLogger_.FlushLog();
		return FALSE;
	}
#endif

	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	SetRegistryKey(_T("PreXion"));

	
 

	DWORD nExitCode = 1;

 
	std::string patientID = (LPCTSTR)cmdInfo.m_PatientID;
	 
	CPMSDB::EXE_CODE error_code;
	std::string exe_msg;
	if(!importCnslDB(patientID,error_code,exe_msg)){
		 
		g_pxLogger_.LogMessage(kErrorOnly,"importCnslDB failed   \n" );
		g_pxLogger_.FlushLog();

#if 0
		if(error_code == CPMSDB::ExeCode_ID_Existing){
			//エラー処理済み
		}else{
			CDBLauncherMsgDlg dlg;
			dlg.m_errorCode = error_code;
			dlg.m_exeMsg = exe_msg;
			if(dlg.DoModal()!= IDOK){
				return FALSE;
			}
		}
	
#endif
	}else{
		g_pxLogger_.LogMessage(kTrace,"importCnslDB OK   \n" );
		g_pxLogger_.FlushLog();
	}
	 
	CDBLauncherMsgDlg dlg;
	dlg.m_errorCode = error_code;
	dlg.m_exeMsg = exe_msg;
	dlg.DoModal();

	{
		g_pxLogger_.LogMessage("CCnslDBLauncherApp Launch CnslApp  \n " );
		g_pxLogger_.FlushLog();

		runCnslApp();

	}
#if 0
	bool loop_flag = true;
	int loop_run_nn = 0;
	while(loop_flag){

//		runCmdCephScan(cmdInfo.m_DicomInfoFile,nExitCode);

		g_pxLogger_.LogMessage("runCmd Exitcode %d \n",nExitCode);
		g_pxLogger_.FlushLog();

		g_theApp_return_code = nExitCode;

#if 0
		switch(nExitCode){
			case CephAppExit_Success:
			case CephAppExit_Fatal:
			case CephAppExit_Already_Exists:
				//終了
				loop_flag = false;
				break;
			case CephAppExit_Error :
				//継続可能なエラー
				//CephApp再起動
				loop_flag = true;
				::Sleep(1000);
				break;
		}
#endif
	}

#endif

	return FALSE;
 
}


int CCnslDBLauncherApp::ExitInstance()  // return app exit code
{
 
	g_pxLogger_.LogMessage("CCnslDBLauncherApp::ExitInstance %d \n ",g_theApp_return_code);
	g_pxLogger_.FlushLog();
	g_pxLogger_.LogMessage("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n ",g_theApp_return_code);
	g_pxLogger_.FlushLog();

	return g_theApp_return_code;
}



#ifdef _DEBUG
const char *FilePath = "CephScanD.exe";
#else
const char *FilePath = "CephScan.exe";
#endif


PROCESS_INFORMATION _processInfo;
void runCmd(const char *cmdLine, DWORD &nExitCode,bool wait)
{


	//command line
	char cmdlineBuff[MAX_PATH*16];
#if 0
	if(fileName.IsEmpty()){
		sprintf(cmdline,"%sbin\\%s",gAppHomeDirBuff,FilePath);
	}else{
	//#17 2012/09/26 
		sprintf(cmdline,"%sbin\\%s \"%s\" ",gAppHomeDirBuff,FilePath,(LPCTSTR)fileName);
	}
 
	g_pxLogger_.LogMessage("run [%s] \n",cmdline);
	g_pxLogger_.FlushLog();
#else
	sprintf_s(cmdlineBuff,MAX_PATH*16,"%s",cmdLine);
#endif
	////////////

	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	 
	
 
	//sprintf(cmdline, "%s -ROT_%s", m_FilePath, m_szItemName);
	BOOL status = CreateProcess(NULL,
						cmdlineBuff,	// Command line. 
						NULL,	// Process handle not inheritable. 
						NULL,	// Thread handle not inheritable. 
						FALSE,	// Set handle inheritance to FALSE. 
						0,		// No creation flags. 
						NULL,	// Use parent's environment block. 
						NULL,	// Use parent's starting directory. 
						&si,	// Pointer to STARTUPINFO structure.
						&_processInfo );	// Pointer to PROCESS_INFORMATION structure.



	if(status)
	{
	//	CloseHandle(_processInfo.hTread);
		// 終了まで待つ間、画面の再描画のみ行う

		if(wait){
		while (WaitForSingleObject(_processInfo.hProcess, 500)        == WAIT_TIMEOUT)    {
#if 0
			MSG msg;
			GetMessage(&Msg, NULL, WM_PAINT, WM_PAINT);
			if (Msg.message == WM_PAINT)
			{
				// 再描画処理をここに記述する。        
			}
#endif
		}

		}
		

		// 戻り値を取得する
		GetExitCodeProcess(_processInfo.hProcess, &nExitCode);

		CloseHandle(_processInfo.hThread);
		CloseHandle(_processInfo.hProcess);

	}else{
		g_pxLogger_.LogMessage("CreateProcess faile [%s] \n ",cmdlineBuff);
		g_pxLogger_.FlushLog();
	}

}

void runCmdCephScan(const CString &fileName,DWORD &nExitCode)
{
	//command line
	char cmdline[MAX_PATH*16];

	if(fileName.IsEmpty()){
		sprintf_s(cmdline,MAX_PATH*16,"%sbin\\%s",gAppHomeDirBuff,FilePath);
	}else{
	//#17 2012/09/26 
		sprintf_s(cmdline,MAX_PATH*16,"%sbin\\%s \"%s\" ",gAppHomeDirBuff,FilePath,(LPCTSTR)fileName);
	}
 
	g_pxLogger_.LogMessage("run [%s] \n",cmdline);
	g_pxLogger_.FlushLog();

	runCmd(cmdline, nExitCode,true /* wait*/);
}


#include "rtvregistry.h"
void runCnslApp()
{

	RTVRegistry rtvr (HKEY_CURRENT_USER, "Software\\PreXion\\CephScan");

#if 0
	CString CnslApp_Name = RegUtil::Read( HKEY_CURRENT_USER,
			"Software\\PreXion\\CephScan",
			"LaunchAppOnExit",CString(""));
#else
	std::string CnslApp_Name = "";
	int status = rtvr.GetRegistryKey ("LaunchAppOnExit", CnslApp_Name);

	if (status != RTVRegistry::kSuccess)
	{
		return ;
	}

#endif
	if(CnslApp_Name.size()<1){
		g_pxLogger_.LogMessage("runCnslApp [ %s ] \n ", CnslApp_Name.c_str());
		g_pxLogger_.FlushLog();
	}

	DWORD nExitCode;
	runCmd(CnslApp_Name.c_str(), nExitCode,false /* wait*/);
															 
}

class CMyPMSDB : public CPMSDB
{
public:
	protected:
	virtual bool confirmErrorCode(const EXE_CODE &error_code,const std::string &msg){
#if 0
		CDBLauncherMsgDlg dlg;
		dlg.m_errorCode = error_code;
		dlg.m_exeMsg = msg;
		if(dlg.DoModal()!= IDOK){
			return false;
		}
#endif
		return true;
	};
};
bool importCnslDB(const std::string &PatientID,CPMSDB::EXE_CODE &error_code,std::string &msg)
{
	bool ret_b = false;

	CMyPMSDB importDB;
	ret_b = importDB.importDB(PatientID,error_code,msg);
	
	return ret_b;
}