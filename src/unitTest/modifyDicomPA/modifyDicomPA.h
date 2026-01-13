// modifyDicomPA.h : TSTCDROM アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_TSTCDROM_H__5BDA66E6_2DA9_4C8E_93E0_03AD2847461E__INCLUDED_)
#define AFX_TSTCDROM_H__5BDA66E6_2DA9_4C8E_93E0_03AD2847461E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CTstCDRomApp:
// このクラスの動作の定義に関しては modifyDicomPA.cpp ファイルを参照してください。
//

class CTstCDRomApp : public CWinApp
{
public:
	CTstCDRomApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CTstCDRomApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CTstCDRomApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TSTCDROM_H__5BDA66E6_2DA9_4C8E_93E0_03AD2847461E__INCLUDED_)
