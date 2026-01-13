
// testdcmInfoDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "testdcmInfo.h"
#include "testdcmInfoDlg.h"
#include "afxdialogex.h"
#include "testdcmInfoAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CtestdcmInfoDlg ダイアログ



CtestdcmInfoDlg::CtestdcmInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtestdcmInfoDlg::IDD, pParent)
	, m_InputStr(_T(""))
	, m_DispStr(_T(""))
	, m_dicmFileName(_T("test_data.dcm"))
	, m_StrDispHex(_T(""))
{
#ifdef UNICODE
	m_StrAppMode = _T("Unicode");
#else
	m_StrAppMode = _T("MultiByte");
#endif
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestdcmInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_INPUT, m_InputStr);
	DDX_Text(pDX, IDC_EDIT_DISP, m_DispStr);
	DDX_Control(pDX, IDC_EDIT_TAG_DISP, m_CtrlTagDisp);
	DDX_Text(pDX, IDC_EDIT_DCM_FILE, m_dicmFileName);
	DDX_Text(pDX, IDC_STATIC_MODE, m_StrAppMode);
	DDX_Text(pDX, IDC_EDIT_TAG_DISP_HEX, m_StrDispHex);
	DDX_Control(pDX, IDC_STATIC_DRAW, m_CtrlDraw);
}

BEGIN_MESSAGE_MAP(CtestdcmInfoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_SEL_FILE, &CtestdcmInfoDlg::OnBnClickedBtSelFile)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CtestdcmInfoDlg::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CtestdcmInfoDlg::OnBnClickedButtonTest)
END_MESSAGE_MAP()


// CtestdcmInfoDlg メッセージ ハンドラー

BOOL CtestdcmInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CtestdcmInfoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CtestdcmInfoDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CtestdcmInfoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


#define DEF_EXT  _T("*.*")
#define FILTER   _T("")

void CtestdcmInfoDlg::OnBnClickedBtSelFile()
{
	CFileDialog dlg(TRUE, DEF_EXT, NULL, 0, FILTER);
	if (dlg.DoModal() == IDOK)
	{
		m_dicmFileName = dlg.GetPathName();
		this->UpdateData(FALSE);
	}
}

void CtestdcmInfoDlg::OnBnClickedButtonLoad()
{
	UpdateData();

	CtestdcmInfoAPI loadDcmAPI;
	if (loadDcmAPI.loadDicomFile(m_dicmFileName)){
		
#ifdef UNICODE
	 	m_CtrlTagDisp.SetWindowText(loadDcmAPI.m_PatientNameW.c_str());
		m_CtrlDraw.m_drawStr = loadDcmAPI.m_PatientNameW.c_str();
#else
		m_CtrlTagDisp.SetWindowText(loadDcmAPI.m_PatientName.c_str());
		m_CtrlDraw.m_drawStr = loadDcmAPI.m_PatientName.c_str();
		
#endif
		m_CtrlDraw.Invalidate();

		const char* strP = loadDcmAPI.m_PatientName.c_str();
		int strSize = loadDcmAPI.m_PatientName.size();
		m_StrDispHex.Empty();
		CString str_fmt_temp;
		for (int i = 0; i < strSize; i++){
			str_fmt_temp.Format(_T(",%02x"), (unsigned char)strP[i]);
			m_StrDispHex = m_StrDispHex + str_fmt_temp;
		}
		this->UpdateData(FALSE);
	};
}

char _big5_buffer[] = {
	0xA4, 0xA4,
	0xA4, 0xA5,
	0xaa, 0xf4, 0xa4, 0xeb, 0xbc, 0x62,
	0,
};
void CtestdcmInfoDlg::OnBnClickedButtonTest()
{
	UpdateData();
	CtestdcmInfoAPI tester;
	 
	std::wstring strBuffer;
//	tester.WStringToChar(strBuffer,inStr );

	tester.Big5ToWString(strBuffer, _big5_buffer);

	m_DispStr = m_InputStr;
	m_DispStr = _big5_buffer;
	UpdateData(FALSE);
	m_CtrlDraw.m_drawStr = m_DispStr;
	m_CtrlDraw.Invalidate();
}
