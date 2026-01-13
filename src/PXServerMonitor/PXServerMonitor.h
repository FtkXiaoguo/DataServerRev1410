// PXServerMonitor.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル
#include "SingleInstance.h"

// CPXServerMonitorApp:
// このクラスの実装については、PXServerMonitor.cpp を参照してください。
//

class CPXServerMonitorApp : public CWinApp
{
public:
	CPXServerMonitorApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();
public:
	CSingleInstance m_singleApp;

public:
// 実装

	DECLARE_MESSAGE_MAP()
};

extern CPXServerMonitorApp theApp;