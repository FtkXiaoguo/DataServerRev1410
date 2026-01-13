// modifyDicomPA.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "modifyDicomPA.h"
#include "modifyDicomPADlg.h"

#include "AqCore/TRLogger.h"
#include "AqCore/TRPlatform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


TRLogger gLogger;
/////////////////////////////////////////////////////////////////////////////
// CTstCDRomApp

BEGIN_MESSAGE_MAP(CTstCDRomApp, CWinApp)
	//{{AFX_MSG_MAP(CTstCDRomApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstCDRomApp クラスの構築

CTstCDRomApp::CTstCDRomApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CTstCDRomApp オブジェクト

CTstCDRomApp theApp;

char gAppHomeDirBuff[2*MAX_PATH+1]={0,};
void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	size_t pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);
#if 0
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
#endif
	strcpy(gAppHomeDirBuff,sub_str_temp.c_str());

  }else{
	  gAppHomeDirBuff[0] = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CTstCDRomApp クラスの初期化

BOOL CTstCDRomApp::InitInstance()
{
	AfxEnableControlContainer();

	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さくしたけ
	//  れば以下の特定の初期化ルーチンの中から不必要なものを削除して
	//  ください。

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

	getHomeFromModulePath();

	CString logFileDir;
	logFileDir.Format("%s\\log",gAppHomeDirBuff);
	TRPlatform::MakeDirIfNeedTo(logFileDir);

	CString logFileName;
	logFileName.Format("%s\\log\\MoidfyDicom.log",gAppHomeDirBuff);
	gLogger.SetLogFile((LPCTSTR)logFileName);

	gLogger.LogMessage("---- Start Dicom Modify -----\n");
	gLogger.FlushLog();

	CTstCDRomDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ダイアログが <OK> で消された時のコードを
		//       記述してください。
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ダイアログが <ｷｬﾝｾﾙ> で消された時のコードを
		//       記述してください。
	}

	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}
