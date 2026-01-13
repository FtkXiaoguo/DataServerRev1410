// PXAddMediaPoint.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PXMediaPoint.h"
#include "PXAddMediaPoint.h"
#include "AQBrowseForFolder.h"

#include "PxNetDB.h"

#include  <io.h>
#include  <direct.h>
extern long gHWaterMark;
extern long gLWaterMark;

extern const char* MP_LABEL_HEAD  ;
extern const char*	PXAppCacheName;
// CPXAddMediaPoint ダイアログ

IMPLEMENT_DYNAMIC(CPXAddMediaPoint, CDialog)

CPXAddMediaPoint::CPXAddMediaPoint(CWnd* pParent /*=NULL*/)
	: CDialog(CPXAddMediaPoint::IDD, pParent)
	, m_path(_T(""))
	, m_label(_T(""))
	, m_hWaterMark(gHWaterMark)
	, m_lWaterMark(gLWaterMark)
	, m_type(_T("RAID"))
{

	m_labelExist = false;
	m_onEditLabel = _T("");
}

CPXAddMediaPoint::~CPXAddMediaPoint()
{
}

void CPXAddMediaPoint::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_path);
	DDX_Text(pDX, IDC_EDIT1, m_label);
	DDX_Text(pDX, IDC_HWaterMark, m_hWaterMark);
	DDX_Text(pDX, IDC_LWaterMark, m_lWaterMark);
	DDX_CBString(pDX, IDC_COMBO_TYPE, m_type);
}


BEGIN_MESSAGE_MAP(CPXAddMediaPoint, CDialog)
	ON_BN_CLICKED(IDOK, &CPXAddMediaPoint::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_BROWS_ROOT, &CPXAddMediaPoint::OnBnClickedButtonBrowsRoot)
	ON_BN_CLICKED(IDCANCEL, &CPXAddMediaPoint::OnBnClickedCancel)
	ON_EN_SETFOCUS(IDC_EDIT1, &CPXAddMediaPoint::OnSetfocusEditLabel)
END_MESSAGE_MAP()


// CPXAddMediaPoint メッセージ ハンドラ

void CPXAddMediaPoint::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	
	UpdateData(TRUE);
	CString tmpStr;
	int retcd = checkValue();
	if( retcd < 0 )
	{
		return ;
	}

	if(!checkMediaPoint())
	{
//		m_pathCtl.SetFocus();
		return ;
	}

	// check the label path
	tmpStr.Format("%s%s", m_path, m_label);
	if(access(tmpStr, 0) != 0)
	{
		if(mkdir(tmpStr) != 0)
		{
			tmpStr.Format("Can't setup media directory: %s%s", m_path, m_label);
			MessageBox( tmpStr, "", MB_ICONWARNING );
			return ;
		}
	}

	// check the cache directory
	tmpStr.Format("%s%s", m_path, PXAppCacheName);
	if(access(tmpStr, 0) != 0)
	{
		if(mkdir(tmpStr) != 0)
		{
			tmpStr.Format("Can't setup cache directory: %sAQNetCache", m_path);
			MessageBox( tmpStr, "", MB_ICONWARNING );
			return ;
		}
	}

	UpdateData(FALSE);

	OnOK();


}
void CPXAddMediaPoint::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	OnCancel();
}

void CPXAddMediaPoint::OnBnClickedButtonBrowsRoot()
{
	UpdateData(TRUE);

	m_path.TrimLeft();
	m_path.TrimRight();
	fixPath(m_path);

	static TCHAR sLocal[FILENAME_MAX] = {0,};
	CAQBrowseForFolder browse(false, true /* diable "New Folder" button */);
	if (browse.Exec(this->GetSafeHwnd(), "Media Point Selector" ,sLocal))
	{
		CString szPath = browse.m_strPathName;
		if (szPath.IsEmpty())
		{
			::AfxMessageBox(_T("Error : No selected  folder."));
			return;
		}
		m_path = szPath;
		strcpy(sLocal, szPath);	// save last path
		UpdateData(FALSE);
	}

}


/********************************************************************/
/* Check Value                                                      */
/********************************************************************/

int CPXAddMediaPoint::checkValue()
{
	UpdateData( TRUE );
	CString tmpStr;


	
	if( m_label.IsEmpty() ||  m_path.IsEmpty() || m_type.IsEmpty() )
	{
		MessageBox( "Please set all parameter." );
		return -1;
	}

	MediaPoint mp;

	if(m_label.GetLength() >= sizeof(mp.m_mediaLabel) ||
		m_path.GetLength() >= sizeof(mp.m_mediaPoint) || 
		m_type.GetLength() >= sizeof(mp.m_mediaType))
	{
		MessageBox( "some parameter length too long." );
		return -1;
	}
	
	return 0;
}

/********************************************************************/
/* Check Path                                                       */
/********************************************************************/

bool CPXAddMediaPoint::fixPath( CString& Path )
{
	bool changed = false;
	if(Path.GetLength() == 1)
	{
		Path += ":\\";
		changed = true;
	}

	if(Path.Right(1) == '/')
	{
		Path.SetAt((Path.GetLength()-1),'\\');
		changed = true;
	}
	
	if( Path.Right(1) != '\\' )
	{
		Path += '\\';
		changed = true;
	}
	return changed;

}


void CPXAddMediaPoint::OnSetfocusEditLabel() 
{
	static bool insideHandle = false;
	if(insideHandle)
		return ;

	insideHandle = true;
	if(!checkMediaPoint()){
//		m_pathCtl.SetFocus();
	}

	insideHandle = false;
	return ;
}


bool CPXAddMediaPoint::checkMediaPoint()
{

	CString tmpStr;

	UpdateData(TRUE);
 
	fixPath(m_path);
 
	UpdateData(FALSE);

	if(m_onEditLabel != _T("") && m_label == m_onEditLabel)
		return true;

	if(strlen(m_path) < 3 || access(m_path, 0) != 0)
	{
		MessageBox("Please correct the invalid media point", "", MB_ICONSTOP );
		return false;
	}

	m_labelExist = false;
//	m_labelCtl.SetReadOnly(FALSE);

	searchLable( m_path, tmpStr, MP_LABEL_HEAD);
	
	if(!tmpStr.IsEmpty())
	{
	//	m_pMPlist->validateLabel(tmpStr, true);
		m_labelExist = true;
//		m_labelCtl.SetReadOnly();
		if (m_label != tmpStr)
		{
			m_label = tmpStr;
			UpdateData( FALSE );
			MessageBox( "Found existed media label in the mounting media.\n Set media label to:\n"+
				m_label);
		}

	}
	else if (m_label.IsEmpty())
	{
		tmpStr = MP_LABEL_HEAD;
		tmpStr += "R";
//		m_pMPlist->validateLabel(tmpStr);
		m_label = tmpStr;
 
		UpdateData(FALSE);
	}
	

	return true;
}



void CPXAddMediaPoint::searchLable( const char* root, CString& label, const char *header)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	CString		findPath;
	CString fileName;

	label.Empty();
	findPath.Format( "%s*.*", root );
	hFind = FindFirstFile(findPath, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{	
			// don't count '..', '.' files
			if(FindFileData.cFileName[0] != '.' && FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{
				fileName = FindFileData.cFileName;
				if ((header == NULL ) || fileName.Find(header) == 0)
				{
					label = fileName;
					break;
				}
			}

		} while(FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

