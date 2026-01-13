// ViewerDicomDB.h : ViewerDicomDB.DLL のメイン ヘッダー ファイル
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CViewerDicomDBApp
// このクラスの実装に関しては ViewerDicomDB.cpp を参照してください。
//

class CViewerDicomDBApp : public CWinApp
{
public:
	CViewerDicomDBApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
