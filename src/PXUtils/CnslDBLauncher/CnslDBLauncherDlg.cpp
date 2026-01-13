// CephLauncherDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "CnslDBLauncher.h"
#include "CnslDBLauncherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CDBLauncherMsgDlg ダイアログ




CDBLauncherMsgDlg::CDBLauncherMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDBLauncherMsgDlg::IDD, pParent)
	, m_LableMsg(_T(""))
	, m_DispMsg(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDBLauncherMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_MSG, m_LableMsg);
	DDX_Text(pDX, IDC_EDIT_MSG, m_DispMsg);
}

BEGIN_MESSAGE_MAP(CDBLauncherMsgDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CDBLauncherMsgDlg メッセージ ハンドラ

BOOL CDBLauncherMsgDlg::OnInitDialog()
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

	char *line1_pre = "    ";
	char *line2_pre = "    ";
	switch(m_errorCode){
		case CPMSDB::ExeCode_No_Error:
			m_DispMsg.Format(" PatientID [%s] \r\n%s患者情報を登録しました。",
							m_exeMsg.c_str(),line1_pre);
			break;

		case CPMSDB::ExeCode_PatientInfo_Existing:
			m_DispMsg.Format(" PatientID [%s] \r\n%s患者情報は既に登録してあります。",
							m_exeMsg.c_str(),line1_pre);
			break;

		case CPMSDB::ExeCode_ID_Existing:
			m_DispMsg.Format(" PatientID [%s] \r\n%s患者情報は既に登録してありますが、ID以外の患者情報が異なります。\r\n%s患者情報編集にて御確認ください。",
							m_exeMsg.c_str(),line1_pre,line2_pre);
			break;
		case CPMSDB::ExeCode_InitDB_Error:
			m_DispMsg.Format(" DB初期化エラー ");
			break;
		case CPMSDB::ExeCode_InsertDB_Error:
			m_DispMsg.Format(" PatientID [%s]　\r\n%s患者情報の登録が失敗しました。",
							m_exeMsg.c_str(),line1_pre);
			break;

		case CPMSDB::ExeCode_GetPatientInfo_Error:
			m_DispMsg.Format(" %s \r\n%s患者情報のロードが失敗しました。",
							m_exeMsg.c_str(),line1_pre);
			break;
		case CPMSDB::ExeCode_PatientInfo_Invalid:
			m_DispMsg.Format(" PatientID [%s] \r\n%s患者情報が正しくないため、登録できません。",
							m_exeMsg.c_str(),line1_pre);
			break;
		default:
			m_DispMsg.Format(" PatientID [%s] \r\n%s不明なエラーが発生しました。患者情報登録ができません。",
							m_exeMsg.c_str(),line1_pre);
			break;

	}
	
	UpdateData(FALSE);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CDBLauncherMsgDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDBLauncherMsgDlg::OnPaint()
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
HCURSOR CDBLauncherMsgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

