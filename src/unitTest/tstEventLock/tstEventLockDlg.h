// tstEventLockDlg.h : ヘッダー ファイル
//

#include "afxwin.h"
#if !defined(AFX_TSTEVENTLOCKDLG_H__25D9A0DC_7130_4147_BC3A_8ACF2F3276DE__INCLUDED_)
#define AFX_TSTEVENTLOCKDLG_H__25D9A0DC_7130_4147_BC3A_8ACF2F3276DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTstEventLockDlg ダイアログ

class CTstEventLockDlg : public CDialog
{
// 構築
public:
	CTstEventLockDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CTstEventLockDlg)
	enum { IDD = IDD_TSTEVENTLOCK_DIALOG };
		// メモ: この位置に ClassWizard によってデータ メンバが追加されます。
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CTstEventLockDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CTstEventLockDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLockDb();
	afx_msg void OnUnlockDb();
	afx_msg void OnQueueEvent();
	afx_msg void OnWaitQueue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CButton m_CtrlChkGlobal;
	afx_msg void OnBnClickedInitDb();
	BOOL m_isGlobal;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TSTEVENTLOCKDLG_H__25D9A0DC_7130_4147_BC3A_8ACF2F3276DE__INCLUDED_)
