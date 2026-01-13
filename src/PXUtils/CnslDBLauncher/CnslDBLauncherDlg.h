// CephLauncherDlg.h : ヘッダー ファイル
//

#pragma once
#include "PMSDB.h"

// CDBLauncherMsgDlg ダイアログ
class CDBLauncherMsgDlg : public CDialog
{
// コンストラクション
public:
	CDBLauncherMsgDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

// ダイアログ データ
	enum { IDD = IDD_DBLAUNCHER_DIALOG };

	CPMSDB::EXE_CODE m_errorCode;
	std::string m_exeMsg;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_LableMsg;
	CString m_DispMsg;
};
