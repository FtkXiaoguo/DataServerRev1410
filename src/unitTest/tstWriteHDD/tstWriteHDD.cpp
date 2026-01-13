/***********************************************************************
 * TestDBDaemon.cpp
 
 *
 *-------------------------------------------------------------------
 */
#define _WIN32_DCOM

#include "stdafx.h"

#include "windows.h"

#pragma warning (disable: 4503)
#include <io.h>

#include "string"

#ifndef MAX_PATH
#define MAX_PATH 512
#endif
 
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

int ServerMain (int argc, char** argv);
int main(int argc, char *argv[])
{

	getHomeFromModulePath();
	ServerMain ( argc, argv);
    return 0;
}


void testWrite();
 
void testRead();
 
int ServerMain (int argc, char** argv)
{
	testWrite();
	//
	::Sleep(1000);

	testRead();
	 
	
	return 0;
}
const int data_frame_size = 1024*1024;
const int loop_NN = 512;
void testWrite()
{
	 
	std::string test_file_name = std::string(gPxDcmServerHomeBuff) + "testWriteHDD.dat";
 
	unsigned short * data_buff = new unsigned short[data_frame_size];
 
	FILE *fp = fopen(test_file_name.c_str(),"wb");

	DWORD start_timer = ::GetTickCount();
	printf("write ... \n");
	for(int i=0;i<loop_NN;i++){
		printf(".");
		data_buff[0] = i;
		data_buff[data_frame_size-1] = i;
		int write_nn = fwrite(data_buff,sizeof(unsigned short),data_frame_size,fp);
		if(write_nn != data_frame_size){
			printf("write error \n");
			break;
		}
	}
	fclose(fp);

	DWORD end_timer = ::GetTickCount();

	delete [] data_buff;

	float spent_time = (end_timer-start_timer)/1000.0f;
	printf(" \n write spent time %.2f Sec\n",spent_time);
	printf(" %.2f MB/Sec \n",data_frame_size*loop_NN/1000000.0/spent_time);
}
void testRead()
{
	 
	std::string test_file_name = std::string(gPxDcmServerHomeBuff) + "testWriteHDD.dat";

    
	unsigned short * data_buff = new unsigned short[data_frame_size];
 
	FILE *fp = fopen(test_file_name.c_str(),"rb");

	DWORD start_timer = ::GetTickCount();
	printf("read ... \n");
	for(int i=0;i<loop_NN;i++){
		printf(".");
		
		int read_nn = fread(data_buff,sizeof(unsigned short),data_frame_size,fp);
		if(read_nn != data_frame_size){
			printf("read error \n");
			break;
		}
		if((data_buff[0] != i) || (data_buff[data_frame_size-1] != i)){
			printf("read data verify error \n");
			break;
		}
	}
	fclose(fp);

	DWORD end_timer = ::GetTickCount();

	delete [] data_buff;

	float spent_time = (end_timer-start_timer)/1000.0f;
	printf(" \n read spent time %.2f Sec\n",spent_time);
	printf(" %.2f MB/Sec \n",data_frame_size*loop_NN/1000000.0/spent_time);
}