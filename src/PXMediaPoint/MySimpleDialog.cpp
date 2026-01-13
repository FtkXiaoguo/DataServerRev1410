// MySimpleDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PXMediaPoint.h"
#include "MySimpleDialog.h"


// CMySimpleDialog ダイアログ

IMPLEMENT_DYNAMIC(CMySimpleDialog, CDialog)

CMySimpleDialog::CMySimpleDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMySimpleDialog::IDD, pParent)
{

}

CMySimpleDialog::~CMySimpleDialog()
{
}

void CMySimpleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMySimpleDialog, CDialog)
END_MESSAGE_MAP()


// CMySimpleDialog メッセージ ハンドラ
