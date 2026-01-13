#pragma once


// CMySimpleDialog ダイアログ

class CMySimpleDialog : public CDialog
{
	DECLARE_DYNAMIC(CMySimpleDialog)

public:
	CMySimpleDialog(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CMySimpleDialog();

// ダイアログ データ
	enum { IDD = IDD_CONFIRM_DELETE_DRIVE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
};
