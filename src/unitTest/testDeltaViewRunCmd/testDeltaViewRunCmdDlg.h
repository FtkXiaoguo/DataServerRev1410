// testDeltaViewRunCmdDlg.h : ヘッダー ファイル
//

#pragma once


// CtestDeltaViewRunCmdDlg ダイアログ
class CtestDeltaViewRunCmdDlg : public CDialog
{
// コンストラクション
public:
	CtestDeltaViewRunCmdDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

// ダイアログ データ
	enum { IDD = IDD_TESTDELTAVIEWRUNCMD_DIALOG };

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
	CString m_disp_msg;
};
