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

#include "TstCStoreSCP.h"

static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};
 void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	int pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
	strcpy(gPxDcmServerHomeBuff,sub_str_temp.c_str());

  }else{
	  gPxDcmServerHomeBuff[0] = 0;
  }
}

void test(const char *cf_filename)
{

	getHomeFromModulePath();
#if 1
	CTstVLIDicomImage TstDicomImage;

	if(!TstDicomImage.initDcmTk()){
		printf("initDcmTk error \n");
		return;
	}


	CTstCStoreSCP TstCStoreSCP;

	TstCStoreSCP.setupHomeFolder(gPxDcmServerHomeBuff);

	if(cf_filename){
		printf( ">>> parameter from %s \n" , cf_filename);
		TstCStoreSCP.loadOption(cf_filename);//"testCStore.cf");
	}

	TstCStoreSCP.initCStoreSCP();

//	TstStoreSCU.sendStudyData();
	TstCStoreSCP.receiveDataContinue();


	TstCStoreSCP.closeAll();

	TstDicomImage.releaseDcmTk();
#endif
}