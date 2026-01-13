
// testdcmInfoDlg.h : ヘッダー ファイル
//

#pragma once
#include "afxwin.h"
#include "testDraw.h"

// CtestdcmInfoDlg ダイアログ
class CtestdcmInfoDlg : public CDialogEx
{
// コンストラクション
public:
	CtestdcmInfoDlg(CWnd* pParent = NULL);	// 標準コンストラクター

// ダイアログ データ
	enum { IDD = IDD_TESTDCMINFO_DIALOG };

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
	afx_msg void OnBnClickedBtSelFile();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonTest();
	CString m_InputStr;
	CString m_DispStr;
	CEdit m_CtrlTagDisp;
	CString m_dicmFileName;
	CString m_StrAppMode;
	CString m_StrDispHex;
	CtestDraw m_CtrlDraw;
};
