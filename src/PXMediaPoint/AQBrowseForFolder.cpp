#include "stdafx.h"

#include "AQBrowseForFolder.h"

#define IDC_CH_SUB_DIR		1000
#define IDC_PATH_EDIT		2000

bool	CAQBrowseForFolder::m_bEnableCreateNewFolder = false;
bool	CAQBrowseForFolder::m_bEnableEditBox = false;
bool	CAQBrowseForFolder::m_bEnableSubDirButton = false;
bool	CAQBrowseForFolder::m_bSearchSubDirState = false;
WNDPROC	CAQBrowseForFolder::m_VSSelectOrgProc;
WNDPROC	CAQBrowseForFolder::m_VSEditOrgProc;
char	CAQBrowseForFolder::m_strPathName[_MAX_PATH];

static	HWND	g_OpenedBrowseFolderHwnd = 0;

CAQBrowseForFolder::CAQBrowseForFolder(bool bEnableEditBox, 
									   bool bEnableCreateNewFolder,
									   bool bEnableSubDirButton, 
									   bool bSubDirState )
{
	m_bEnableEditBox = bEnableEditBox;
	m_bEnableCreateNewFolder = bEnableCreateNewFolder;
	SetEnableSubDirButton(bEnableSubDirButton);
	SetSearchSubDirState(bSubDirState);
}

CAQBrowseForFolder::~CAQBrowseForFolder()
{
}

LRESULT CALLBACK CAQBrowseForFolder::VSSelectFolderSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((uMsg == WM_COMMAND) && (LOWORD(wParam) == IDC_CH_SUB_DIR)) 
	{
		CWnd	*pClient = CWnd::FromHandle(hWnd);
		CButton	*pSubDir = (CButton *)pClient->GetDlgItem(IDC_CH_SUB_DIR);

		SetSearchSubDirState(GetSearchSubDirState() ? false : true);
		pSubDir->SetCheck(GetSearchSubDirState() ? TRUE : FALSE);
	}
	return(CallWindowProc(m_VSSelectOrgProc, hWnd, uMsg, wParam, lParam));
}

LRESULT CALLBACK CAQBrowseForFolder::VSEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((uMsg == WM_COMMAND) && (LOWORD(wParam) == IDC_PATH_EDIT)) 
	{
		CWnd*	pClient = CWnd::FromHandle (hWnd);
		CEdit*	pEdit = (CEdit*)pClient->GetDlgItem(IDC_PATH_EDIT);
		if (pEdit)
		{
			CAQBrowseForFolder::GetPathFromEdit(pEdit);
		}
	}
	return(CallWindowProc(m_VSEditOrgProc, hWnd, uMsg, wParam, lParam));
}

int CALLBACK CAQBrowseForFolder::CallbackSelectDir(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	TCHAR	szDir[MAX_PATH];
	if (uMsg == BFFM_INITIALIZED) 
	{
		g_OpenedBrowseFolderHwnd = hWnd;

		SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);

		if (GetEnableSubDirButton()) 
		{
			CButton	*pSubDir = new CButton;
			if (pSubDir) 
			{
				RECT	rectClient, rectOK, rectCancel, rect;
				CWnd	*pClient	= CWnd::FromHandle(hWnd);
				CWnd	*pOK		= pClient->GetDlgItem(IDOK);
				CWnd	*pCANCEL	= pClient->GetDlgItem(IDCANCEL);
				pClient->GetClientRect(&rectClient);
				pOK->GetWindowRect(&rectOK);
				pCANCEL->GetWindowRect(&rectCancel);
				pClient->ScreenToClient(&rectOK) ;
				pClient->ScreenToClient(&rectCancel) ;

				rect = CRect(rectClient.right - rectCancel.right,
				             rectCancel.top,
				             rectOK.left,
				             rectCancel.bottom);
				
				if (pSubDir->Create(_T("Search Sub-Folder"), WS_CHILD | WS_VISIBLE | BS_CHECKBOX, rect, pClient, IDC_CH_SUB_DIR)) 
				{
					HFONT hFontCurr = (HFONT)pClient->SendMessage(WM_GETFONT, 0, 0);
					if (hFontCurr != NULL) pSubDir->PostMessage(WM_SETFONT, (WPARAM)hFontCurr, 0); 

					pSubDir->SetCheck(GetSearchSubDirState() ? TRUE : FALSE);

					m_VSSelectOrgProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)VSSelectFolderSubProc);
				}
			}
		}

		if ( m_bEnableEditBox )
		{
			CEdit* pEdit = new CEdit;
			if (pEdit)
			{
				RECT	rectClient, rect;
				CWnd	*pClient	= CWnd::FromHandle(hWnd);
				pClient->GetClientRect(&rectClient);

				rect.top	= 30;
				rect.bottom = rect.top + 26;
				rect.left	= 12;
				rect.right	= rectClient.right - 15;

				if (pEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, rect, pClient, IDC_PATH_EDIT)) 
				{
					pEdit->SetLimitText(_MAX_PATH);

					HFONT hFontCurr = (HFONT)pClient->SendMessage(WM_GETFONT, 0, 0);
					if (hFontCurr != NULL) pEdit->PostMessage(WM_SETFONT, (WPARAM)hFontCurr, 0); 

					pEdit->SetWindowText((const char*)lpData);
					CAQBrowseForFolder::GetPathFromEdit(pEdit);

					m_VSEditOrgProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)VSEditProc);
				}
			}
		}
	}
	else if (uMsg==BFFM_SELCHANGED && ::SHGetPathFromIDList((LPITEMIDLIST)lParam, szDir)) 
	{
		strcpy( CAQBrowseForFolder::m_strPathName, szDir );
	}

	return 0;
}

void CAQBrowseForFolder::GetPathFromEdit(CEdit* pEdit)
{
	if (pEdit)
	{
		char buf[_MAX_PATH];
		memset(buf, NULL, _MAX_PATH);
		//pEdit->UpdateData();
		if (pEdit->GetWindowText(buf, _MAX_PATH))
		{
			strcpy (CAQBrowseForFolder::m_strPathName, buf);
		}
	}
}

bool CAQBrowseForFolder::Exec(HWND hOwner, const char* sTitle, char *sLocal)
{
	BOOL			bResult = FALSE;
	BROWSEINFO		bi;
	LPSTR			lpBuffer;
	LPITEMIDLIST	pidlRoot;
	LPITEMIDLIST	pidlBrowse;
	LPMALLOC		lpMalloc = NULL;

	HRESULT hr = SHGetMalloc(&lpMalloc);
	if (FAILED(hr)) return(FALSE);

	{
		HWND	hwnd = AfxGetMainWnd()->GetSafeHwnd();

		if ((lpBuffer = (LPSTR) lpMalloc->Alloc(_MAX_PATH)) == NULL) 
		{
			return(FALSE);
		}
		if (!SUCCEEDED(SHGetSpecialFolderLocation(	hwnd,
													CSIDL_DESKTOP,
													&pidlRoot))) 
		{
			lpMalloc->Free(lpBuffer);
			return(FALSE);
		}
	}

	bi.hwndOwner = hOwner;
	bi.pidlRoot = pidlRoot;
	bi.pszDisplayName = lpBuffer;
	bi.lpszTitle = sTitle;
	if ( m_bEnableEditBox )
	{
		bi.ulFlags = BIF_STATUSTEXT | BIF_RETURNONLYFSDIRS;
	}
	else
	{
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
	}
	if ( m_bEnableCreateNewFolder )
	{
//		bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
		bi.ulFlags |= BIF_NEWDIALOGSTYLE;
	}
	bi.lpfn = CallbackSelectDir;
	bi.lParam = (LPARAM)sLocal;

	::EnableWindow( hOwner, FALSE );

	g_OpenedBrowseFolderHwnd = 0;

	pidlBrowse = SHBrowseForFolder(&bi);

	g_OpenedBrowseFolderHwnd = 0;

	::EnableWindow( hOwner, TRUE );

	if (pidlBrowse != NULL) 
	{
		if (SHGetPathFromIDList(pidlBrowse, lpBuffer)) 
		{
			strcpy(sLocal, lpBuffer);
			bResult = TRUE;
		}
		lpMalloc->Free(pidlBrowse);
	}

	// Crean up
	lpMalloc->Free(pidlRoot);
	lpMalloc->Free(lpBuffer);
	lpMalloc->Release();

	return(bResult ? true : false);
}

// 2005.06.23 Nobu - #5850 bug (Browse For Folder doesn't close when disconnect)
void
CAQBrowseForFolder::CloseWindow()
{
	if( !g_OpenedBrowseFolderHwnd )	return;
	if( !IsWindow(g_OpenedBrowseFolderHwnd) )	return;

	::SendMessage( g_OpenedBrowseFolderHwnd, WM_COMMAND, IDCANCEL, 0L );
} 

