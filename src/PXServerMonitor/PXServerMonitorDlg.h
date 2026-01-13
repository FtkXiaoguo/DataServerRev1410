// PXServerMonitorDlg.h : ヘッダー ファイル
//
#pragma once


// CPXServerMonitorDlg ダイアログ
class CPXServerMonitorDlg : public CDialog
{
	const LONG m_nTimerElapse;
	unsigned m_nStatus;
// コンストラクション
public:
	CPXServerMonitorDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

// ダイアログ データ
	enum { IDD = IDD_PXSERVERMONITOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_NotifyData;
	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMenuItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	BOOL InitializeNotifyData();
public:
	afx_msg void OnDestroy();
	afx_msg void OnClose();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	BOOL MonitorReg(void);
	void CheckRegValue(UINT value);
	void FlashBackGround(void);
	afx_msg void OnBnClickedButtonExcuteDcomsadmin();
	void MsgPump();
};
