  
/*---------------------------------------------------------------------
 *			Copyright, TeraRecon, Inc 2001 to 2006 All rights reserved.
 *
 *	PURPOSE:
 *			To access database APIs.
 *			All functions will be called in java side. .
 *
 *	AUTHOR(S):  Junnan Wu, July 2002.
 *
 *   
 *-------------------------------------------------------------------*/
 #pragma warning (disable: 4786)
#pragma warning (disable: 4503)  
 
#include "stdafx.h"

class CtestDumyDll2App : public CWinApp
{
public:
	CtestDumyDll2App();

// オーバーライド
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};


//
//TODO: この DLL が MFC DLL に対して動的にリンクされる場合、
//		MFC 内で呼び出されるこの DLL からエクスポートされたどの関数も
//		関数の最初に追加される AFX_MANAGE_STATE マクロを
//		持たなければなりません。
//
//		例:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 通常関数の本体はこの位置にあります
//		}
//
//		このマクロが各関数に含まれていること、MFC 内の
//		どの呼び出しより優先することは非常に重要です。
//		これは関数内の最初のステートメントでなければな 
//		らないことを意味します、コンストラクタが MFC
//		DLL 内への呼び出しを行う可能性があるので、オブ
//		ジェクト変数の宣言よりも前でなければなりません。
//
//		詳細については MFC テクニカル ノート 33 および
//		58 を参照してください。
//


// CtestDumyDll2App

BEGIN_MESSAGE_MAP(CtestDumyDll2App, CWinApp)
END_MESSAGE_MAP()


// CtestDumyDll2App コンストラクション

CtestDumyDll2App::CtestDumyDll2App()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の CtestDumyDll2App オブジェクトです。

CtestDumyDll2App theApp;


// CtestDumyDll2App 初期化

BOOL CtestDumyDll2App::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
