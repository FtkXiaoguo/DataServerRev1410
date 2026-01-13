// tstAqNETCommon.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
void test(const char *cf_filename=0);
int main(int argc, char* argv[])
{
	printf("PXQRdcmTest  ...\n");
	
	if(argc>1){
		test(argv[1]);
	}else{
		test(0);
	}
	return 0;
}
 