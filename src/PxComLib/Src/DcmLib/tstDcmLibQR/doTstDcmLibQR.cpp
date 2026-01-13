// tstAqNETCommon.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

 
 
//#include "TstVLIDicomImage.h"

#include "TstQRSCU.h"

 
void test(const char *cf_filename=0)
{

#if 0
	CTstVLIDicomImage TstDicomImage;

	if(!TstDicomImage.initDcmTk()){
		printf("initDcmTk error \n");
		return;
	}
#endif

	CTstQRSCU TstQRSCU;

	if(cf_filename){
		printf( ">>> parameter from %s \n" , cf_filename);
//		TstQRSCU.loadOption(cf_filename);//"testCStore.cf");
	}

//	TstQRSCU.openAssociation();

	TstQRSCU.initParam();
//	TstStoreSCU.sendStudyData();
	TstQRSCU.sendDataContinue();

	TstQRSCU.closeAssociation();
}