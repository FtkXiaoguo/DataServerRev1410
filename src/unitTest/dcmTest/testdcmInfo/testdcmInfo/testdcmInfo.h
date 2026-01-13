
// testdcmInfo.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CtestdcmInfoApp:
// このクラスの実装については、testdcmInfo.cpp を参照してください。
//

class CtestdcmInfoApp : public CWinApp
{
public:
	CtestdcmInfoApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CtestdcmInfoApp theApp;