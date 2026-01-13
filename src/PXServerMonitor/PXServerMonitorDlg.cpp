// PXServerMonitorDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PXServerMonitor.h"
#include "PXServerMonitorDlg.h"
#include <io.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum MY_WM{
	WM_NOTIFY_DATA = WM_USER + 1,
	WM_MENU_ITEM_EXIT
};


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPXServerMonitorDlg ダイアログ




CPXServerMonitorDlg::CPXServerMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPXServerMonitorDlg::IDD, pParent),
	m_nTimerElapse(2000)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPXServerMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPXServerMonitorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_NOTIFY_DATA, OnNotifyIcon)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_EXCUTE_DCOMSADMIN, &CPXServerMonitorDlg::OnBnClickedButtonExcuteDcomsadmin)
END_MESSAGE_MAP()


// CPXServerMonitorDlg メッセージ ハンドラ

BOOL CPXServerMonitorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。
	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText(_T(""));
	GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText(_T(""));
	GetDlgItem(IDC_BUTTON_EXCUTE_DCOMSADMIN)->ShowWindow(SW_HIDE);
	BOOL bRtn = theApp.m_singleApp.SetProp(m_hWnd);
	ASSERT(bRtn);

	bRtn = InitializeNotifyData();
	ASSERT(bRtn);

	Shell_NotifyIcon(NIM_ADD,&m_NotifyData);

	ShowWindow(SW_SHOWNORMAL);
	SetForegroundWindow();

	MonitorReg();
	SetTimer(1, m_nTimerElapse, NULL);
	SetTimer(2, 300, NULL);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CPXServerMonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CPXServerMonitorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CPXServerMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CPXServerMonitorDlg::InitializeNotifyData(void)
{
	m_NotifyData.cbSize=sizeof(m_NotifyData);
	m_NotifyData.hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_NotifyData.hWnd=m_hWnd;
	lstrcpy(m_NotifyData.szTip,theApp.m_singleApp.m_strAppName);
	m_NotifyData.uCallbackMessage=WM_NOTIFY_DATA;
	m_NotifyData.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP;
	//Shell_NotifyIcon(nShellState,&m_NotifyData);
	return TRUE;
}

afx_msg LRESULT CPXServerMonitorDlg::OnMenuItem(WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_MENU_ITEM_EXIT){
		CDialog::OnOK();
	}
	return 0;
}

afx_msg LRESULT CPXServerMonitorDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	CString str;
	if(wParam == m_NotifyData.uID){
		if(lParam == WM_LBUTTONDOWN){
		}else if(lParam == WM_RBUTTONDOWN){
		}else if (lParam == WM_RBUTTONUP){
			POINT point;
			::GetCursorPos(&point); 
			CMenu menu;
			menu.CreatePopupMenu();  
			menu.AppendMenu(MF_STRING, WM_MENU_ITEM_EXIT, _T("Exit"));
			menu.TrackPopupMenu(TPM_LEFTALIGN, point.x ,point.y, this);
			HMENU hmenu = menu.Detach();
			menu.DestroyMenu();
		}else if(lParam == WM_LBUTTONDBLCLK ){
			::OutputDebugString("lParam == WM_LBUTTONDBLCLK\n");
			ShowWindow(SW_SHOW);
		}
	}

	return 0;
}
void CPXServerMonitorDlg::OnDestroy()
{
	CDialog::OnDestroy();

	theApp.m_singleApp.RemoveProp(m_hWnd);
	Shell_NotifyIcon(NIM_DELETE,&m_NotifyData);
	KillTimer(1);
	KillTimer(2);
}

void CPXServerMonitorDlg::OnClose()
{
	ShowWindow(SW_HIDE);
	Shell_NotifyIcon(NIM_MODIFY,&m_NotifyData);
	//CDialog::OnClose();
}

BOOL CPXServerMonitorDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam){
		case IDOK:
		case IDCANCEL:
			return FALSE;
		case WM_MENU_ITEM_EXIT:
			CDialog::OnOK();
			break;
		default:
			break;
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CPXServerMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1){
		MonitorReg();
	}else if (nIDEvent == 2){
		FlashBackGround();
	}
	CDialog::OnTimer(nIDEvent);
}


BOOL IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            // handle error
            AfxMessageBox("IsWow64 error!");
        }
    }
    return bIsWow64;
}



BOOL CPXServerMonitorDlg::MonitorReg(void)
{
	HKEY hKey = 0;
	DWORD dwType = 0;
	CHAR lpData[1024] = "";
	DWORD dwDataSize = 0;
	LONG lRet = 0;
	DWORD dwRet = 0;

	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\PreXion\\PXDataServer"),
		NULL,
		KEY_QUERY_VALUE,
		&hKey);

    if (lRet != ERROR_SUCCESS) {
        DebugLog("RegOpenKeyEx error\n");
        return FALSE;
    }

    dwDataSize = sizeof(lpData)/sizeof(lpData[0]);

    lRet = RegQueryValueEx(
      hKey,
      _T("QueueProcError"),
      0,
      &dwType,
      (LPBYTE)lpData,
      &dwDataSize);

    if (lRet != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        DebugLog("RegQueryValueEx error\n");
        return FALSE;
    }
	//DebugLog(_T("%s\n"), lpData);
	UINT* pValue = (UINT*)lpData;

	switch(dwType)
	{
	case REG_NONE: // No value type
        DebugLog(_T("REG_NONE\n")); 
        break;
    case REG_SZ: // Unicode nul terminated string
        DebugLog(_T("REG_SZ\n")); 
        break;
    case REG_EXPAND_SZ: // Unicode nul terminated string  (with environment variable references)
        DebugLog(_T("REG_EXPAND_SZ\n")); 
        break;
    case REG_BINARY: // Free form binary
        DebugLog(_T("REG_BINARY\n")); 
        break;
    case REG_DWORD_LITTLE_ENDIAN: // 32-bit number
		{
			DebugLog("REG_DWORD_LITTLE_ENDIAN: %d\n", *pValue);
			CheckRegValue(*pValue);
		}
        break;
    case REG_DWORD_BIG_ENDIAN: // 32-bit number
        DebugLog(_T("REG_DWORD_BIG_ENDIAN\n")); 
        break;
    case REG_LINK: // Symbolic Link (unicode)
        DebugLog(_T("REG_LINK\n")); 
        break;
    case REG_MULTI_SZ: // Multiple Unicode strings
        DebugLog(_T("REG_MULTI_SZ\n")); 
        break;
    case REG_RESOURCE_LIST: // Resource list in the resource map
        DebugLog(_T("REG_RESOURCE_LIST\n")); 
        break;
    case REG_FULL_RESOURCE_DESCRIPTOR:  // Resource list in the hardware description
        DebugLog(_T("REG_FULL_RESOURCE_DESCRIPTOR\n")); 
        break;
    case REG_RESOURCE_REQUIREMENTS_LIST: 
        DebugLog(_T("REG_RESOURCE_REQUIREMENTS_LIST\n")); 
        break;
    case REG_QWORD: // 64-bit number
        DebugLog(_T("REG_QWORD\n"));
        break;
    default: 
        DebugLog(_T("Unknown\n")); 
        break;
    }

    RegCloseKey(hKey);
	return TRUE;
}

void CPXServerMonitorDlg::CheckRegValue(UINT value)
{
	CString str(_T(""));
	if (value == 0){
		str.LoadString(IDS_STRING_DICOM_SEND_OK);
		GetDlgItem(IDC_BUTTON_EXCUTE_DCOMSADMIN)->ShowWindow(SW_HIDE);
	}else{		
		str.LoadString(IDS_STRING_DICOM_SEND_ERROR);
		GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText(str);
		ShowWindow(SW_SHOW);
		ShowWindow(SW_SHOWNORMAL);
		SetForegroundWindow();
		GetDlgItem(IDC_BUTTON_EXCUTE_DCOMSADMIN)->ShowWindow(SW_SHOW);
	}
	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->SetWindowText(str);
	Invalidate();
}

void CPXServerMonitorDlg::FlashBackGround(void)
{
	static int counter = 0;
	static TCHAR szBuffer[1024*8] = "";
	CWnd* pWnd = GetDlgItem(IDC_STATIC_SHOW_STATUS);
	//pWnd->GetWindowText(szBuffer, sizeof(szBuffer));
	if (counter > 8){
		counter = 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		pWnd->RedrawWindow();
	}
	strcat(szBuffer, "*");
	char szText[1024*8];
	sprintf(szText, "Checking DICOM Data Send:%s", szBuffer);;

	CClientDC dc(pWnd);
	COLORREF col = dc.SetBkColor(RGB(212, 208, 200));
	dc.TextOut(0, 0, szText);
	dc.SetBkColor(col);

	GetDlgItem(IDC_STATIC_SHOW_MESSAGE)->RedrawWindow();
	counter ++;
}

void CPXServerMonitorDlg::OnBnClickedButtonExcuteDcomsadmin()
{
	GetDlgItem(IDC_BUTTON_EXCUTE_DCOMSADMIN)->EnableWindow(FALSE);

	TCHAR szName[1024*8] = _T(""); 
	::GetModuleFileName(NULL, szName, sizeof(szName));
	CString str(szName);
	int nIndex = str.ReverseFind(_T('\\'));
	CString strPXDcmSAdmin = str.Left(nIndex);
	strPXDcmSAdmin += _T("\\PXDcmSAdmin.exe");

	CString strWndName(_T("PXDcmSAdmin"));
	HWND hWnd = ::FindWindow(NULL, strWndName);
	if (hWnd){
		::ShowWindow(hWnd, SW_SHOW);
		::ShowWindow(hWnd, SW_NORMAL);
		::SetForegroundWindow(hWnd);
	}else if (_taccess(strPXDcmSAdmin, 0) == 0){
		PROCESS_INFORMATION ProInfo;
		STARTUPINFO    StartInfo;
		ZeroMemory ( &StartInfo, sizeof(StartInfo));
  		CString strCmdLine = strPXDcmSAdmin + _T(" -d");
        ZeroMemory (&ProInfo, sizeof(ProInfo));
		DebugLog("excute cmd line = %s\n", strCmdLine);
		if (!CreateProcess ( NULL,    
			strCmdLine.GetBuffer(strCmdLine.GetLength()),                  
            NULL,                      
            NULL,                    
            FALSE,                   
            0,                         
            NULL,                    
            NULL,                 
            &StartInfo,                   
            &ProInfo)          
        ){
            DebugLog(_T("CreateProcess failed : %d\n"), GetLastError());
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(ProInfo.hProcess, INFINITE)){
			DebugLog("%s Exited\n", strPXDcmSAdmin);
		}
		if (!CloseHandle(ProInfo.hThread)) {
			DebugLog("CloseHandle(hThread) Failed\n");
		}
	}else{
	}
	GetDlgItem(IDC_BUTTON_EXCUTE_DCOMSADMIN)->EnableWindow(TRUE);
}

void CPXServerMonitorDlg:: MsgPump()
{
	MSG msg;
	for (int i=0; i<1000; i++){
		while(::PeekMessage(&msg, m_hWnd, 0, 0, PM_NOREMOVE)){
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}
