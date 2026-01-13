#pragma once


// CPXAddMediaPoint ダイアログ

class CPXAddMediaPoint : public CDialog
{
	DECLARE_DYNAMIC(CPXAddMediaPoint)

public:
	CPXAddMediaPoint(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CPXAddMediaPoint();

// ダイアログ データ
	enum { IDD = IDD_ADD_DRIVE };

protected:
	bool	checkMediaPoint();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()

	int		checkValue();
	void	searchLable( const char* root, CString& label, const char* header=NULL);
	bool	fixPath( CString& path );
	bool	m_labelExist;
public:
	afx_msg void OnBnClickedOk();
	CString m_path;
	CString m_label;
	UINT m_hWaterMark;
	UINT m_lWaterMark;
	CString m_type;

	CString m_onEditLabel;
	///
	afx_msg void OnBnClickedButtonBrowsRoot();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSetfocusEditLabel();
};
