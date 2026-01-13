// testDeltaViewRunCmd.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CtestDeltaViewRunCmdApp:
// このクラスの実装については、testDeltaViewRunCmd.cpp を参照してください。
//

class CtestDeltaViewRunCmdApp : public CWinApp
{
public:
	CtestDeltaViewRunCmdApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CtestDeltaViewRunCmdApp theApp;