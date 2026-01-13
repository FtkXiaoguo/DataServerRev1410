// modifyDicomPADlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "modifyDicomPA.h"
#include "modifyDicomPADlg.h"
#include "AqCore/TRPlatform.h"
#include "DicomProc.h"

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
// CTstCDRomDlg ダイアログ

CTstCDRomDlg::CTstCDRomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTstCDRomDlg::IDD, pParent)
	, m_InputDir(_T("C:\\temp"))
	, m_OutputDir(_T("C:\\temp"))
	, m_RestStr2(_T(""))
	, m_NewUIDFlag(FALSE)
	, m_FlipAllFlag(FALSE)
{
	m_LogInfoFlag = true;//One Series only
	//{{AFX_DATA_INIT(CTstCDRomDlg)
	 
	 m_runningFlag  = false;
	m_Result = _T("");
	 
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTstCDRomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTstCDRomDlg)


	DDX_Text(pDX, IDC_REST, m_Result);

	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT_INPUT_DIR, m_InputDir);
	DDX_Text(pDX, IDC_EDIT_OUTPUT_DIR, m_OutputDir);
	DDX_Text(pDX, IDC_REST2, m_RestStr2);
	DDX_Check(pDX, IDC_CHECK_NEW_SERIES_UID, m_NewUIDFlag);
	DDX_Check(pDX, IDC_CHECK_FLIP_ALL, m_FlipAllFlag);
}

BEGIN_MESSAGE_MAP(CTstCDRomDlg, CDialog)
	//{{AFX_MSG_MAP(CTstCDRomDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	 
	ON_BN_CLICKED(IDC_BT_START, OnBtStart)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BT_SEL_OUTPUT_DIR, &CTstCDRomDlg::OnBnClickedBtSelOutputDir)
	ON_BN_CLICKED(IDC_BT_SEL_INPUT_DIR, &CTstCDRomDlg::OnBnClickedBtSelInputDir)
	ON_BN_CLICKED(IDOK, &CTstCDRomDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTstCDRomDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstCDRomDlg メッセージ ハンドラ

BOOL CTstCDRomDlg::OnInitDialog()
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
	
	CDicomProc::initDcmLib();
	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

void CTstCDRomDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTstCDRomDlg::OnPaint() 
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
HCURSOR CTstCDRomDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

 
void CTstCDRomDlg::OnBtStart() 
{
	CWaitCursor waitCursor;
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	UpdateData();

	 
	CString input_dir_temp = m_InputDir;
	input_dir_temp.MakeUpper();
	CString output_dir_temp = m_OutputDir;
	output_dir_temp.MakeUpper();
	if(input_dir_temp == output_dir_temp){
		MessageBox("InputDir == OutDir","",MB_ICONERROR);
		return;
	}
	if(output_dir_temp.Find(input_dir_temp)>=0){
		CString sub_str = output_dir_temp.Mid(input_dir_temp.GetLength(),1);
		if(sub_str == "\\"){
			MessageBox("InputDir <- OutDir","",MB_ICONERROR);
			return;
		}
	}

	m_toCancelFlag = false;
	m_runningFlag = true;

	if(searchInputDir((LPCTSTR)m_InputDir )){
		MessageBox("DICOM Flip completed successfully.","");
	}else{
		MessageBox("DICOM Flip failed!","",MB_ICONERROR);
	}

	UpdateData(FALSE);

	if(m_toCancelFlag){
		CDialog::OnCancel();
	}
	m_runningFlag  = false;
}

void CTstCDRomDlg::OnBnClickedBtSelOutputDir()
{
	UpdateData();

	BOOL		bRes;
	char		chPutFolder[MAX_PATH];
	LPITEMIDLIST	pidlRetFolder;
	BROWSEINFO	stBInfo;
	CString		cRetStr;

	//　構造体を初期化します。
	stBInfo.pidlRoot = NULL;	//ルートフォルダです。
	stBInfo.hwndOwner = this->GetSafeHwnd();	//表示するダイアログの親ウィンドウのハンドルです。
	stBInfo.pszDisplayName = chPutFolder;	//選択されているフォルダ名を受けます。
	stBInfo.lpszTitle = "Original Dir";	//説明の文字列です。
	stBInfo.ulFlags = BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;	//表示フラグです。
	stBInfo.lpfn = NULL;	//ダイアログプロシージャへのポインタです。
	stBInfo.lParam = 0L;	//プロシージャに送るパラメーターです。

	//　ダイアログボックスを表示します。
	pidlRetFolder = ::SHBrowseForFolder( &stBInfo );

 
	//　pidlRetFolderにはそのフォルダを表すアイテムＩＤリストへのポインタが
	//　入っています。chPutFolderには選択されたフォルダ名（非フルパス）だけ
	//　しか入っていないので、このポインタを利用します。

	if( pidlRetFolder != NULL )
	{
		//　フルパスを取得します。
		bRes = ::SHGetPathFromIDList( pidlRetFolder, chPutFolder );
		if( bRes != FALSE ){
			m_OutputDir = chPutFolder;
		}

		::CoTaskMemFree( pidlRetFolder );
	}
 
	
	UpdateData(FALSE);
}

void CTstCDRomDlg::OnBnClickedBtSelInputDir()
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	UpdateData();

	BOOL		bRes;
	char		chPutFolder[MAX_PATH];
	LPITEMIDLIST	pidlRetFolder;
	BROWSEINFO	stBInfo;
 

	//　構造体を初期化します。
	stBInfo.pidlRoot = NULL;	//ルートフォルダです。
	stBInfo.hwndOwner = this->GetSafeHwnd();	//表示するダイアログの親ウィンドウのハンドルです。
	stBInfo.pszDisplayName = chPutFolder;	//選択されているフォルダ名を受けます。
	stBInfo.lpszTitle = "Original Dir";	//説明の文字列です。
	stBInfo.ulFlags = BIF_RETURNONLYFSDIRS;	//表示フラグです。
	stBInfo.lpfn = NULL;	//ダイアログプロシージャへのポインタです。
	stBInfo.lParam = 0L;	//プロシージャに送るパラメーターです。

	//　ダイアログボックスを表示します。
	pidlRetFolder = ::SHBrowseForFolder( &stBInfo );

 
	//　pidlRetFolderにはそのフォルダを表すアイテムＩＤリストへのポインタが
	//　入っています。chPutFolderには選択されたフォルダ名（非フルパス）だけ
	//　しか入っていないので、このポインタを利用します。

	if( pidlRetFolder != NULL )
	{
		//　フルパスを取得します。
		bRes = ::SHGetPathFromIDList( pidlRetFolder, chPutFolder );
		if( bRes != FALSE ){
			m_InputDir = chPutFolder;
		}

		::CoTaskMemFree( pidlRetFolder );
	}

 
	UpdateData(FALSE);
}


bool CTstCDRomDlg::searchInputDir(const std::string &folderName,int dirLevel )
{

	if(dirLevel<1){
//		m_StudyList.clear();
		m_curFolderOrg = folderName;
		if(!makeOutputDir(m_curFolderOrg)){
			return false;
		}
	}

	HANDLE hFind ;

	WIN32_FIND_DATA wfd;
	char filepath[MAX_PATH];
	filepath[0] = 0;

	std::string Pattern = folderName+"\\*.*";  

	bool ret_val = true;
	 
	hFind = FindFirstFile(Pattern.c_str(), &wfd) ;
	if( hFind == INVALID_HANDLE_VALUE) {
		 
		return false;
	}


	bool cancelFlag = false;
	do{
	 

		if (!stricmp(wfd.cFileName, ".")){
			continue;
		}
		if (!stricmp(wfd.cFileName, "..")){
			continue;
		}
				 
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
		 
			//Folder単位の区切りで停止させる。
			if(m_toCancelFlag){
				cancelFlag = true;
				break;
			}
		
			std::string SubFolder = folderName+"\\"+wfd.cFileName;

 		 
			if(!makeOutputDir(SubFolder)){
				return false;
			}

			m_LogInfoFlag = true;//One Series only

			bool finishFlag = false;
#if 0
			//check the folder name  which is study date
			if(!checkFolder(SubFolder,wfd.cFileName,dirLevel,finishFlag)){
				//ignore this folder
				if(finishFlag){
				/*
				*  キャンセルではない！
				*   これ以上処理しない。
				*/
	//				cancelFlag = true;
					break;
				}else{
					continue;
				}
			}

			if(m_procCallback){
				m_procCallback->procFolder(folderName,SubFolder);
			}
#endif

			if(!searchInputDir(SubFolder,dirLevel+1)){
				 cancelFlag = true;
				 break;
			}

			//Folder単位の区切りで停止させる。
			if(m_toCancelFlag){
				cancelFlag = true;
				break;
			}
			 
		}else{
		

			std::string src_fileName = folderName+"\\"+wfd.cFileName;
			std::string des_fileName = m_curFolderDes +"\\"+wfd.cFileName;

			if(m_curFolderOrg.size()>0){
				if(folderName != m_curFolderOrg){
					CString erro_msg;
					erro_msg.Format("folder diff: %s , %s ",folderName,m_curFolderOrg);
					MessageBox(erro_msg,"",MB_ICONERROR);
				}
			}
	//		dispFolder(SubFolder, wfd.cFileName,dirLevel);


			if(m_LogInfoFlag){
				logoutDicomInfo(src_fileName);
				m_LogInfoFlag = false;//One Series only
			}
	//		printf(".");
			bool continueFlag = true;
		//	bool ret_b = true;
		 	bool ret_b = doDciomModify(src_fileName, des_fileName );

			if(ret_b) {
				if(!continueFlag){
					cancelFlag = true;
					break;
				}
			}
#if 0
			if( ((m_runCount++) % (m_dispInterval)) == 0){
				if(m_procCallback){
					m_procCallback->procFile(folderName,wfd.cFileName);
				}
				printf(".");
			}
#endif

			continue;//DICOMファイルとサブフォルダ混在の場合。
		 
		}
		 
	}while(FindNextFile(hFind,&wfd)==TRUE);
	printf("\n");

//	printf(" [%d] \n",::GetCurrentThreadId());

	FindClose(hFind);

	if(cancelFlag){
		ret_val = false;
	}
	return ret_val;
}

bool CTstCDRomDlg::makeOutputDir(const std::string &folderName)
{
	TRACE1("new folder %s \n",folderName.c_str());
	m_Result = folderName.c_str();

	UpdateData(FALSE);

	m_curFolderOrg = folderName ;

	CString str_folderOrg_temp = m_curFolderOrg.c_str();
	m_curFolderDes =  m_OutputDir+ str_folderOrg_temp.Mid(m_InputDir.GetLength());
	 
	TRPlatform::MakeDirIfNeedTo(m_curFolderDes.c_str());

	return true;
}
bool CTstCDRomDlg::doDciomModify(const std::string &org_dicomFile,const std::string &dest_dicomFile)
{
	if(m_toCancelFlag) return false;

	TRACE2("proc file %s to %s \n",org_dicomFile.c_str(),dest_dicomFile.c_str());

	//display progress
	m_RestStr2 = m_RestStr2 + ".";
	if(m_RestStr2.GetLength()>64){
		m_RestStr2 = ".";
	}
	UpdateData(FALSE);
	//
	dispatchMsg();
 
	/////////

	CDicomProc dicomProc;
//	bool ret_b = dicomProc.FlipHori(org_dicomFile,dest_dicomFile,true /* LA_except*/);
	bool ret_b = dicomProc.FlipHori(org_dicomFile,dest_dicomFile,m_FlipAllFlag==FALSE /* LA_except*/, m_NewUIDFlag==TRUE/*NewUIDFlag*/);

	if(!ret_b){
	//LA/PA/CARPUS以外は処理しない
//		::CopyFile(org_dicomFile.c_str(),dest_dicomFile.c_str(),FALSE);
	}

	return true;
}
bool CTstCDRomDlg::logoutDicomInfo(const std::string &org_dicomFile)
{
	CDicomProc dicomProc;
	dicomProc.logoutDicomInfo(org_dicomFile);
	return true;
}
void CTstCDRomDlg::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	if(m_runningFlag){
		m_toCancelFlag = true;
	}else{
		OnOK();
	}
	
}

void CTstCDRomDlg::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	if(m_runningFlag){
		m_toCancelFlag = true;
	}else{
		OnCancel();
	}
}

void CTstCDRomDlg::dispatchMsg()
{
	MSG msgCur;   
	
	CWinThread *pThread = AfxGetThread();
	for(int i=0;i<100;i++){
		if(!::PeekMessage(&msgCur, NULL, NULL, NULL, PM_NOREMOVE)){
		 return;
		}

		 BOOL ret_b = pThread->PumpMessage();
		 if(ret_b == FALSE) {
			 break;
		 }
	}
}