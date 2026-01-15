// XtMfcLib.cpp : DLL 用の初期化処理の定義を行います。
//
 
#pragma warning (disable: 4819)

#include "stdafx.h"

#if 0
class CDcmLibDllApp : public CWinApp
{
public:
	CDcmLibDllApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

 

// CtestDumyDll2App

BEGIN_MESSAGE_MAP(CDcmLibDllApp, CWinApp)
END_MESSAGE_MAP()


// CtestDumyDll2App コンストラクション

CDcmLibDllApp::CDcmLibDllApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の CtestDumyDll2App オブジェクトです。

CDcmLibDllApp theApp;


// CtestDumyDll2App 初期化

BOOL CDcmLibDllApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
#else
void th_lib_close(void);
void process_detach(void)
{
    printf("ThDcmLib: process_detach \n");

}
void process_attach(void)
{
    printf("ThDcmLib: process_attach \n");

}
void detach_thread(void)
{
    

}
BOOL APIENTRY DllMain(HMODULE ,
    DWORD  ul_reason_for_call,
    LPVOID 
)
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        process_attach();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        detach_thread();
        break;
    case DLL_PROCESS_DETACH:
        process_detach();
        break;
    }
    return TRUE;
}
#endif