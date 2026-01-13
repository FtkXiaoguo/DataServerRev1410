// tstAqNETCommon.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
void test(const char *cf_filename=0);
int main(int argc, char* argv[])
{
	printf("PXDcmSCPTest  ...\n");
	
	if(argc>1){
		test(argv[1]);
	}else{
		test();
	}
	return 0;
}
 
#include "TstVLIDicomImage.h"
#include "TstMt.h"
 

#include "MtThread.h" 
void test(const char *cf_filename)
{

	CTstVLIDicomImage TstDicomImage;

	if(!TstDicomImage.initDcmTk()){
		printf("initDcmTk error \n");
		return;
	}


	CTstMt testMt;

	int th_num = 10;
	for(int i=0;i<th_num;i++){
		testMt.start(i+1);
	}
	char c = getchar();


}