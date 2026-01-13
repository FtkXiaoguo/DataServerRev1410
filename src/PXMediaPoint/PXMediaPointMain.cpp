// PXMediaPointMain.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PXMediaPoint.h"
#include "PXMediaPointMain.h"
#include "PXAddMediaPoint.h"
#include "MySimpleDialog.h"


#include <string>
#include <vector>
#include  <io.h>
#include  <direct.h>

#include "PxNetDB.h"
///////////////////////////
#define BUFSIZE 1024            // ample space for our string buffer

const int	cDriveListMax	= 16;
const char*	PXAppCacheName = "PXCache";
//const char*	AQNetHome = "C:\\AQNet\\";
const char* MP_LABEL_HEAD = "PXDICOM_";
const int MP_LENGTH = 256;

const HKEY kDefaultKey       = HKEY_LOCAL_MACHINE;
const char* kDefaultLocation = "Software\\TeraRecon\\Aquarius\\AQNet\\1.0";
//const char* kDefaultAQCommonLocation = "Software\\TeraRecon\\Aquarius\\Common";


// CPXMediaPointMain ダイアログ

IMPLEMENT_DYNAMIC(CPXMediaPointMain, CDialog)

CPXMediaPointMain::CPXMediaPointMain(CWnd* pParent /*=NULL*/)
	: CDialog(CPXMediaPointMain::IDD, pParent)
{

}

CPXMediaPointMain::~CPXMediaPointMain()
{
}

void CPXMediaPointMain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_mediaPointList);
}


BEGIN_MESSAGE_MAP(CPXMediaPointMain, CDialog)

	ON_BN_CLICKED(IDC_B_ADD, &CPXMediaPointMain::OnBnClickedBAdd)
	ON_BN_CLICKED(IDC_B_DELETE, &CPXMediaPointMain::OnBnClickedBDelete)
	ON_BN_CLICKED(IDC_B_EDIT, &CPXMediaPointMain::OnBnClickedBEdit)
	ON_BN_CLICKED(IDOK, &CPXMediaPointMain::OnBnClickedOk)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CMediaPointDlg message handlers
 
BOOL CPXMediaPointMain::OnInitDialog()
{
	CDialog::OnInitDialog();
	// center the dialog on the screen
	CenterWindow();

 
	
	setupControl();	

	loadMediaPointList();

//	addMediaPoint("iii"," ooo","label",100,300);
	return TRUE;
}
// CPXMediaPointMain メッセージ ハンドラ

void CPXMediaPointMain::OnBnClickedBAdd()
{
	int num = m_mediaPointList.GetItemCount();
	if( num >= cDriveListMax )
	{
		CString msg;
		msg.Format( "Cannot add new drive list. Max : %d", cDriveListMax );
		MessageBox( msg );
		return  ;
	}

	
	CPXAddMediaPoint	dlg(this);
	
	int retcd = dlg.DoModal();
	if( retcd != IDOK )
	{
		return  ;
	}

	addMediaPoint(dlg.m_path, dlg.m_type, dlg.m_label, dlg.m_hWaterMark, dlg.m_lWaterMark );
}

void CPXMediaPointMain::OnBnClickedBDelete()
{
	int num = m_mediaPointList.GetNextItem( -1, LVNI_SELECTED );
	if( num < 0 )
		return  ;

	CMySimpleDialog  dlg;
	if( dlg.DoModal() != IDOK ){
		return  ;
	}

	CString tmpStr;
	for( ; num != -1; )
	{
		tmpStr = m_mediaPointList.GetItemText( num, cMediaPointPath);
		m_mediaPointList.DeleteItem( num );
		num = m_mediaPointList.GetNextItem( num-1, LVNI_SELECTED );
	}

	return  ;
}

void CPXMediaPointMain::OnBnClickedBEdit()
{
	int num = m_mediaPointList.GetNextItem( -1, LVNI_SELECTED );
	if( num < 0 )
		return  ;

	CPXAddMediaPoint	dlg(this);
	dlg.m_type = m_mediaPointList.GetItemText( num, cMediaPointType );
	dlg.m_label = m_mediaPointList.GetItemText( num, cMediaPointLabel );
	dlg.m_onEditLabel = dlg.m_label;
	dlg.m_path = m_mediaPointList.GetItemText( num, cMediaPointPath );
	dlg.m_hWaterMark = atol(m_mediaPointList.GetItemText( num, cHighWaterMark ));
	dlg.m_lWaterMark = atol(m_mediaPointList.GetItemText( num, cLowWaterMark ));

	if( dlg.DoModal() != IDOK )
		return  ;

	addMediaPoint(dlg.m_path, dlg.m_type, dlg.m_label, dlg.m_hWaterMark, dlg.m_lWaterMark, num );
}

void CPXMediaPointMain::OnBnClickedOk()
{
	saveDriveList();

	OnOK();

	
}

/********************************************************************/
/* Setup Control                                                    */
/********************************************************************/

void CPXMediaPointMain::setupControl()
{
	

	int		w;
	CRect	rc;

	m_mediaPointList.GetWindowRect( &rc );
	w = ( rc.Width()-5 ) / 9;

//	m_mediaPointList.SetupList();
	m_mediaPointList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	m_mediaPointList.InsertColumn( cMediaPointPath,		_T( "Media Point" )		,	LVCFMT_LEFT,150 );
	m_mediaPointList.InsertColumn( cMediaPointType,		_T( "Type" )			,	LVCFMT_LEFT,50 ) ;
	m_mediaPointList.InsertColumn( cMediaPointLabel,	_T( "Media Label" )		,	LVCFMT_LEFT,150  );
	m_mediaPointList.InsertColumn( cHighWaterMark,		_T( "High Watermark" )	,	LVCFMT_LEFT,100  );
	m_mediaPointList.InsertColumn( cLowWaterMark,		_T( "Low Watermark" )	,	LVCFMT_LEFT,100 );

}

/********************************************************************/
/* Load MediaPoint List                                             */
/********************************************************************/
long gHWaterMark = 10000;
long gLWaterMark = 500;

void CPXMediaPointMain::loadMediaPointList()
{
#if 1
	std::vector<MediaPoint>	vData;
	m_mediaPointList.DeleteAllItems();

	CPxDB::InitMediaPoints();
	vData = CPxDB::GetMediaPoints();

	int num = vData.size();
	for( int i=0; i<num; i++ )
	{
		if(i == 0)
		{
			 gHWaterMark = vData[i].m_highWaterMark;
			 gLWaterMark = vData[i].m_lowWaterMark;
		}
		// add mounted media only
		if(vData[i].m_mediaPoint[0] != 0)
			addMediaPoint(vData[i].m_mediaPoint, vData[i].m_mediaType, vData[i].m_mediaLabel, vData[i].m_highWaterMark, vData[i].m_lowWaterMark );
	}
#endif
}


/********************************************************************/
/* Add MediaPoint                                                   */
/********************************************************************/

void CPXMediaPointMain::addMediaPoint( const char* path, const char* type, const char* label, long hw, long lw, int iOnItem )
{
	if(!label[0])
		return;

	CString tmpStr;
	// check the label path
	tmpStr.Format("%s%s", path, label);
	if((access(tmpStr, 0) != 0) && (mkdir(tmpStr) != 0))
		return;

	// check the cache directory
	tmpStr.Format("%s%s", path, PXAppCacheName);
	if((access(tmpStr, 0) != 0) && (mkdir(tmpStr) != 0))
		return;

	int num = m_mediaPointList.GetItemCount();
	if(iOnItem >= 0)
	{
		num = iOnItem;
		m_mediaPointList.DeleteItem( num );
	}
 
	char item_str_temp[256];
	int index = 0;
	LV_ITEM item;

	////////////////

	//path
	sprintf(item_str_temp,"%s",path);
	item.mask		= LVIF_TEXT | LVIF_IMAGE;
	item.pszText	= item_str_temp;
	item.iItem		= 0;
	item.iSubItem	= index++;
	//item.iImage		= this->GetIconIndex( strTemp );
	m_mediaPointList.InsertItem( &item );

	//type
	sprintf(item_str_temp,"%s",type);
	item.mask		= LVIF_TEXT;
	item.pszText	= item_str_temp;
	item.iSubItem	= index++;
	m_mediaPointList.SetItem( &item );

	//type
	sprintf(item_str_temp,"%s",label);
	item.mask		= LVIF_TEXT;
	item.pszText	= item_str_temp;
	item.iSubItem	= index++;
	m_mediaPointList.SetItem( &item );

	//hw
	sprintf(item_str_temp,"%d",hw);
	item.mask		= LVIF_TEXT;
	item.pszText	= item_str_temp;
	item.iSubItem	= index++;
	m_mediaPointList.SetItem( &item );

	//lw
	sprintf(item_str_temp,"%d",lw);
	item.mask		= LVIF_TEXT;
	item.pszText	= item_str_temp;
	item.iSubItem	= index++;
	m_mediaPointList.SetItem( &item );

	m_mediaPointList.SetItemState( 0, LVNI_SELECTED|LVNI_FOCUSED, LVNI_SELECTED|LVNI_FOCUSED );

}


/********************************************************************/
/* Save Drive List                                                  */
/********************************************************************/

void CPXMediaPointMain::saveDriveList()
{
	std::vector<MediaPoint> mediaPointList;

	int listNum = m_mediaPointList.GetItemCount();

	if(listNum<1) return;

	mediaPointList.resize(listNum);

	CString tmpStr;
	for( int i=0; i<listNum; i++ )
	{
		strcpy(mediaPointList[i].m_mediaType,m_mediaPointList.GetItemText( i, cMediaPointType ));
		strcpy(mediaPointList[i].m_mediaLabel,m_mediaPointList.GetItemText( i, cMediaPointLabel ));
 

		tmpStr = m_mediaPointList.GetItemText( i, cMediaPointPath );
		tmpStr.Replace('/', '\\');
		strcpy(mediaPointList[i].m_mediaPoint,tmpStr);
 
 

		tmpStr = m_mediaPointList.GetItemText( i, cHighWaterMark );
		mediaPointList[i].m_highWaterMark =  atol(tmpStr) ;

		tmpStr = m_mediaPointList.GetItemText( i, cLowWaterMark );
		mediaPointList[i].m_lowWaterMark =  atol(tmpStr) ;
		

	}
	 
	if(!CPxDB::SaveMediaPoints(mediaPointList)){
		CString msg;
		msg.Format( "Cannot write Mediapoint "  );
		MessageBox( msg );
		return  ;
	}
}
