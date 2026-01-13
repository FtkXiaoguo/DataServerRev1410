// testFxDcmData.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "objbase.h"
 
int doDbLibTest();
 
int main(int argc, char* argv[])
{
	 doDbLibTest();
	 //
 
	return 0;
}

// #include "PxNetDB.h"
#include "PxDB.h"

int doDbLibTest()
{
	HRESULT		hr;
	hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	CPxDB db;
	db.InitDatabaseInfo(false,10);

	return 0;
}