// testPanoAPI.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
 
#include "testPxDcmProc.h"

 
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
 
#include <QApplication>

#include "mainwindow.h"
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow main_w;
	main_w.show();
	return a.exec();

	CTestDcmAPICls testDcm ;
	//////////////////////////////////////////////
	//グローバル初期化

	if(!testDcm.init()){
		printf("init error ---stop!\n");
		return -1;
	}
	 
 
	//testDcm.doTestPanoFiles(); return 0;

	//testDcm.doTestPano(CTestDcmAPICls::LANG_ID_CHINESE);
	// testDcm.doTestCepha();

	//////////////////////////////////////////////
	//全て終了
	//////////////////////////////////////////////
	testDcm.destroy();

	return 0;
}

//////////////
/// テストデータ
///////////////////
void setupProjData(int blk_i, int blk_size,float *frame_buff,int sizeX,int sizeY,int run_num  )
{
	int set_nn =200;//test pattern
	if(run_num==1){
		set_nn = 400;//TMJ-2 ---2nd
	}

	for(int j=0;j<blk_size;j++){

		int frame_no = blk_i*blk_size +j;
		float *frame_data = frame_buff + sizeX*sizeY*j;
		//setup frame data
				
		{//test data patten
			 
			for(int y_i=0;y_i<sizeY;y_i++){
				float set_val = 1000.0f;

				if( (frame_no/set_nn)%2 == 0){
					if(y_i<sizeY/2){
						set_val = 0.0f;
					}
				}else{
					if(y_i>sizeY/2){
						set_val = 0.0f;
					}
				}
				 
				//////////
				for(int x_i=0;x_i<sizeX;x_i++){
					frame_data[y_i*sizeX + x_i] = set_val ; //<-just test data
				}
			}
 
		}
	}
}

void setupProjData1(int blk_i, int blk_size,float *frame_buff,int sizeX,int sizeY,int run_num  )
{
	int set_nn =200;//test pattern
	if(run_num==1){
		set_nn = 400;//TMJ-2 ---2nd
	}

	for(int j=0;j<blk_size;j++){

		int frame_no = blk_i*blk_size +j;
		float *frame_data = frame_buff + sizeX*sizeY*j;
		//setup frame data
				
		{//test data patten
			 
			for(int y_i=0;y_i<sizeY;y_i++){
				float set_val = 1000.0f;

				if( (frame_no%5 )== 0){
					 
					set_val = 0.0f;
					 
				} 
				 
				//////////
				for(int x_i=0;x_i<sizeX;x_i++){
					frame_data[y_i*sizeX + x_i] = set_val ; //<-just test data
				}
			}
 
		}
	}
}