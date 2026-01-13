
// testDraw.h : ヘッダー ファイル
//

#pragma once
#include "afxwin.h"


// CtestdcmInfoDlg ダイアログ
class CtestDraw : public CStatic
{
// コンストラクション
public:
	CtestDraw();	
	CString m_drawStr;
	DECLARE_MESSAGE_MAP()
protected:
	 
	afx_msg void OnPaint();
	 
public:
	 
};
