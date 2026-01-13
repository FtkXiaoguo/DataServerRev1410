// tstAqNETCommon.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
 
#include <string>

std::string in_folder = "C:\\temp\\download20131024145740\\12249057\\58863355\\";
std::string out_folder = "C:\\temp\\cnv1024_folder\\";
int doCnv1024DicomFile(const std::string &in_FolderName,const std::string &out_FolderName );
int main(int argc, char* argv[])
{
	printf(">>> RefApp <<<<<!\n");
	
 
	if(argc>1){
		in_folder = argv[1];
		if(argc>2){
			out_folder = argv[2];
		}
		 
	}

	doCnv1024DicomFile(in_folder, out_folder);
	return 0;
}
 
 
  