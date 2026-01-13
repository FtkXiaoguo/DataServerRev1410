// tstAqNETCommon.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
void test(const char *cf_filename=0);
int main(int argc, char* argv[])
{
	printf(">>> RefApp <<<<<!\n");
	
#if 1
	if(argc>1){
		test(argv[1]);
	}else{
		test();
	}
#endif
	return 0;
}
 
#include "TstVLIDicomImage.h"

#include "TstStoreSCU.h"

 
void test(const char *cf_filename)
{

	CTstVLIDicomImage TstDicomImage;

	if(!TstDicomImage.initDcmTk()){
		printf("initDcmTk error \n");
		return;
	}

#if 0
	TstDicomImage.loadDicom("E:\\temp\\test.dcm");
	TstDicomImage.saveDicom("test_org.dcm");
	TstDicomImage.outputTAG("test_out.txt");
	TstDicomImage.ConvertToFile("test.dcm");
#else
//	TstDicomImage.openNewDicom();
 //	TstDicomImage.saveDicom("test.dcm");
#endif

	CTstStoreSCU::initClient();

	CTstStoreSCU TstStoreSCU;

	if(cf_filename){
		printf( ">>> parameter from %s \n" , cf_filename);
		TstStoreSCU.loadOption(cf_filename);//"testCStore.cf");
	}
// 	TstStoreSCU.testReadWriteDicom();


 //	TstStoreSCU.openAssociation();

	TstStoreSCU.initCount();
//	TstStoreSCU.sendStudyData();
	TstStoreSCU.sendDataContinue();

	TstStoreSCU.closeAssociation();
	TstStoreSCU.closeALL();

	TstDicomImage.releaseDcmTk();
}