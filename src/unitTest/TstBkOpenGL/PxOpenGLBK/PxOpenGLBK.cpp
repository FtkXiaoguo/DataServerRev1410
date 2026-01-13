// PxOpenGLBK.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include "AqCore/TRLogger.h"

#include "windows.h"

#include "ImplantOpenGL.h"

#include <string>
TRLogger gLogger;

 
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


int _tmain(int argc, _TCHAR* argv[])
{
	bool session0Flag =false;
	
	getHomeFromModulePath();

	if(argc>1){
		int g_sessionID = -1;

		g_sessionID =atoi(argv[1]);
		session0Flag = (g_sessionID==0);
	}
	std::string log_file = gPxDcmServerHomeBuff + std::string("log\\PxOpenGLBK.log");
	gLogger.SetLogFile(log_file.c_str());
	gLogger.SetLogLevel(2);

	gLogger.LogMessage("---- start PxOpenGLBK session0Flag %d ---- \n",session0Flag );
	gLogger.FlushLog();

	CImplantOpenGL ImpOpenGL;
	ImpOpenGL.doTst(session0Flag);
	//ImpOpenGL.tstDraw();

	gLogger.LogMessage("---- end ---- \n" );
	gLogger.FlushLog();
	return 0;
}

