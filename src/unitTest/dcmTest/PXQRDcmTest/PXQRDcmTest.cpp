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
		test();
	}
	return 0;
}
 
#include "TstVLIDicomImage.h"

#include "TstQRSCUApp.h"

#include "DcmServTst.h"
 
void test(const char *cf_filename)
{

	CTstVLIDicomImage TstDicomImage;

	if(!TstDicomImage.initDcmTk()){
		printf("initDcmTk error \n");
		return;
	}

#if 0
	CDcmServTst dcmServTst;
	dcmServTst.Init();

 
	//
	 
	dcmServTst.dotst(1);
	 
	return;
#endif

	CTstQRSCUApp TstQRSCU;

	if(cf_filename){
		printf( ">>> parameter from %s \n" , cf_filename);
		TstQRSCU.loadOption(cf_filename);//"testCStore.cf");
	}

	

	TstQRSCU.initParam();

	TstQRSCU.openAssociation();
//	TstStoreSCU.sendStudyData();
	TstQRSCU.sendDataContinue();

	TstQRSCU.closeAssociation();

	TstQRSCU.closeAll();
	TstDicomImage.releaseDcmTk();
}