// tstEventLock.h : TSTEVENTLOCK アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_TSTEVENTLOCK_H__6F22671D_1000_45C6_838E_AB103EEBC6E8__INCLUDED_)
#define AFX_TSTEVENTLOCK_H__6F22671D_1000_45C6_838E_AB103EEBC6E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CTstEventLockApp:
// このクラスの動作の定義に関しては tstEventLock.cpp ファイルを参照してください。
//

class CTstEventLockApp : public CWinApp
{
public:
	CTstEventLockApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CTstEventLockApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CTstEventLockApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TSTEVENTLOCK_H__6F22671D_1000_45C6_838E_AB103EEBC6E8__INCLUDED_)
