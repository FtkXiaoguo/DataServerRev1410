// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"


void tstDcmDataSet();
int tstStoreSCU();
int tstMCSet();
int tstEchoSCU();

int initLib();
void releaseLib();
int _tmain(int argc, _TCHAR* argv[])
{

// 	tstDcmDataSet();

	initLib();

	tstStoreSCU();

	tstEchoSCU();

	for(int i=0;i<15000;i++){
 		tstStoreSCU();
	}

	releaseLib();
	return 0;
}
