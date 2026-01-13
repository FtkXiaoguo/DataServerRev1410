// modifyDicomPADlg.h : ヘッダー ファイル
//

#if !defined(AFX_TSTCDROMDLG_H__43830E2C_BB2D_4E0F_842F_E3F6B96507F3__INCLUDED_)
#define AFX_TSTCDROMDLG_H__43830E2C_BB2D_4E0F_842F_E3F6B96507F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
/////////////////////////////////////////////////////////////////////////////
// CTstCDRomDlg ダイアログ

class CTstCDRomDlg : public CDialog
{
// 構築
public:
	CTstCDRomDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CTstCDRomDlg)
	enum { IDD = IDD_MODIFY_DCMPH_DIALOG };
	 
 
	CString	m_Result;
 
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CTstCDRomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	bool m_LogInfoFlag ;
	std::string  m_curFolderOrg; 
	std::string  m_curFolderDes;
	void dispatchMsg();
	bool m_runningFlag;
	bool searchInputDir(const std::string &folderName,int dirLevel=0);
	
	bool doDciomModify(const std::string &org_dicomFile,const std::string &dest_dicomFile);
	bool logoutDicomInfo(const std::string &org_dicomFile);
	bool makeOutputDir(const std::string &folderName);
	bool m_toCancelFlag;
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CTstCDRomDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	 
	afx_msg void OnBtStart();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtSelOutputDir();
	afx_msg void OnBnClickedBtSelInputDir();
	CString m_InputDir;
	CString m_OutputDir;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString m_RestStr2;
	BOOL m_NewUIDFlag;
	BOOL m_FlipAllFlag;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TSTCDROMDLG_H__43830E2C_BB2D_4E0F_842F_E3F6B96507F3__INCLUDED_)
