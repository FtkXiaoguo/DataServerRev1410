#pragma once


#include "afxcmn.h"
// CPXMediaPointMain ダイアログ

enum
{
	cMediaPointPath = 0,
	cMediaPointType,
	cMediaPointLabel,
	cHighWaterMark,
	cLowWaterMark
};

class CPXMediaPointMain : public CDialog
{
	DECLARE_DYNAMIC(CPXMediaPointMain)

public:
	CPXMediaPointMain(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CPXMediaPointMain();

// ダイアログ データ
	enum { IDD = IDD_MEDIAPOINT_DIALOG };

protected:
	CFont	 m_font;
 
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog(); 
	void	setupControl();
	void	loadMediaPointList();
	void	addMediaPoint( const char* path, const char* type, const char* label, long hw, long lw, int iOnItem=-1);
	void	saveDriveList();
public:
	afx_msg void OnBnClickedBAdd();
	afx_msg void OnBnClickedBDelete();
	afx_msg void OnBnClickedBEdit();
	afx_msg void OnBnClickedOk();
	CListCtrl m_mediaPointList;
};
