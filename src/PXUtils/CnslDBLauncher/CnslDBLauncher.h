// CephLauncher.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CCnslDBLauncherApp:
// このクラスの実装については、CephLauncher.cpp を参照してください。
//

class CCnslDBLauncherApp : public CWinApp
{
public:
	CCnslDBLauncherApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance(); // return app exit code
// 実装

	DECLARE_MESSAGE_MAP()
};

extern CCnslDBLauncherApp theApp;