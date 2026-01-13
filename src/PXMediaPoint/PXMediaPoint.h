// PXMediaPoint.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CPXMediaPointApp:
// このクラスの実装については、PXMediaPoint.cpp を参照してください。
//

class CPXMediaPointApp : public CWinApp
{
public:
	CPXMediaPointApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CPXMediaPointApp theApp;