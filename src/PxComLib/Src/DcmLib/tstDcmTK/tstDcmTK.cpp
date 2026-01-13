// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
 
void tstDcmTkString();
void tstMemory();
int _tmain(int argc, _TCHAR* argv[])
{

	for(int i=0;i<10;i++) {
		printf("run %d \n",i);
		tstMemory();
	}

//	tstDcmTkString();
	printf("end\n");
	return 0;
}



#include "dcmtk/ofstd/ofstring.h"


void tstDcmTkString()
{
	
 OFString str("ttt");

}

void tstMemory()
{
	for(int run_i = 0;run_i<100;run_i++){
		int buff_size = 14*1024*512;
		int *buff = (int *)malloc(buff_size);
 	 	delete [] buff;
	//	free(buff);
	}
}