// tstEventLockDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "tstEventLock.h"
#include "tstEventLockDlg.h"

#include "TestDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報で使われている CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// メッセージ ハンドラがありません。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstEventLockDlg ダイアログ

CTstEventLockDlg::CTstEventLockDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTstEventLockDlg::IDD, pParent)
	, m_isGlobal(FALSE)
{
	//{{AFX_DATA_INIT(CTstEventLockDlg)
		// メモ: この位置に ClassWizard によってメンバの初期化が追加されます。
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTstEventLockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTstEventLockDlg)
	// メモ: この場所には ClassWizard によって DDX と DDV の呼び出しが追加されます。
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK_GLOBAL, m_CtrlChkGlobal);
	DDX_Check(pDX, IDC_CHECK_GLOBAL, m_isGlobal);
}

BEGIN_MESSAGE_MAP(CTstEventLockDlg, CDialog)
	//{{AFX_MSG_MAP(CTstEventLockDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOCK_DB, OnLockDb)
	ON_BN_CLICKED(IDC_UNLOCK_DB, OnUnlockDb)
	ON_BN_CLICKED(IDC_QUEUE_EVENT, OnQueueEvent)
	ON_BN_CLICKED(IDC_WAIT_QUEUE, OnWaitQueue)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_INIT_DB, &CTstEventLockDlg::OnBnClickedInitDb)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstEventLockDlg メッセージ ハンドラ

BOOL CTstEventLockDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
 

	

	// "バージョン情報..." メニュー項目をシステム メニューへ追加します。

	// IDM_ABOUTBOX はコマンド メニューの範囲でなければなりません。
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

	// このダイアログ用のアイコンを設定します。フレームワークはアプリケーションのメイン
	// ウィンドウがダイアログでない時は自動的に設定しません。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定
	
	// TODO: 特別な初期化を行う時はこの場所に追加してください。
	
	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

void CTstEventLockDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CTstEventLockDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR CTstEventLockDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

CTestDB _tstDB_;
void CTstEventLockDlg::OnLockDb() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	_tstDB_.lockDB();
}

void CTstEventLockDlg::OnUnlockDb() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	_tstDB_.unlockDB();
}

void CTstEventLockDlg::OnQueueEvent() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	_tstDB_.addQueueEvent();
}
void CTstEventLockDlg::OnWaitQueue()
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	_tstDB_.waitQueueEvent();
}
void CTstEventLockDlg::OnBnClickedInitDb()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	UpdateData();

	CTestDB::initTestDB(m_isGlobal);
}
