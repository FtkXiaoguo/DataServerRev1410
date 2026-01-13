// AQNetDefaultScheduler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

 

#include "DbManBackup.h"
#include "DbManSupervisor.h"
#include "DbManShrink.h"

#include "AppComConfiguration.h"

#include "windows.h"

//
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

int install_main(int argc, char** argv );
int supervisor_main(int argc, char** argv );
int DbBackup_main(int argc, char** argv );
int DbShrink_main(int argc, char** argv );

void getHomeFromModulePath();
int main(int argc, char* argv[])
{
	getHomeFromModulePath();
	AppComConfiguration::setHomeFolder(gPxDcmServerHomeBuff) ;

	if(argc <2) return 0;
	
	char *cmd_name = argv[1];
	
	if(strcmp(cmd_name,	"install") == 0){
		return install_main(  argc,  argv );
	}else 
	if(strcmp(cmd_name,	"super") == 0){
		return supervisor_main(  argc,  argv );
	}else 
	if(strcmp(cmd_name,	"backup") == 0){
		return DbBackup_main(  argc,  argv );
	}else 
	if(strcmp(cmd_name,	"shrink") == 0){
		return DbShrink_main(  argc,  argv );
	}else{
		printf(" do nothing \n");
	}

	
	return 1;
}

int install_main(int argc, char** argv )
{
	return 0;
}
int supervisor_main(int argc, char** argv )
{
	
	CDbManSupervisor dbManSupervisor;

	dbManSupervisor.doMain(argc,  argv);
	return 0;
}
int DbBackup_main(int argc, char** argv )
{
	CDbManBackup dbMan;

	dbMan.doMain(argc,  argv);
	return 0;
}
int DbShrink_main(int argc, char** argv )
{
//	printf("not supported yet \n");
	CDbManShrink dbMan;

	dbMan.doMain(argc,  argv);
	return 0;
}

