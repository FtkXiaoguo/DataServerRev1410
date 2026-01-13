/***********************************************************************
 * TestDBDaemon.cpp
 
 *
 *-------------------------------------------------------------------
 */
#define _WIN32_DCOM


#pragma warning (disable: 4503)
#pragma warning (disable: 4786)
#pragma warning (disable: 4996)

 
#include <io.h>

 
#include "rtvPoolAccess.h"
 
 
#include "AqCore/TRLogger.h"
#include "TestDumyDaemon.h"
 
 
 
#include "IDcmLibApi.h"

#include "PXLicenseManagerIf.h"
#include "ThreadedLicenseChecker.h"

 
//#include "../testDumyDll/TstDumyCls.h";

//	Global config & logging

TRLogger gLogger;
 
TestDumyServiceProcessor processor("TestDBDaemon");
 
RTVDaemon service(&processor);


/***************************************************************************
 *
 *   main accepts the following options
 *
 *      -install  install the server as a service (manual start)
 *      -auto     install the server as a service (auto start)
 *
 *      -remove   remove the service
 *      -debug    run the server as a console program
 *	    -start	  installs and runs the server as a service
 *		-port n   which port to listen to
 ***************************************************************************/

int main(int argc, char *argv[])
{
	service.Start(argc, argv);
    return 0;
}

static char gTestDumyServerHomeBuff[2*MAX_PATH+1]={0,};
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
	strcpy(gTestDumyServerHomeBuff,sub_str_temp.c_str());

  }else{
	  gTestDumyServerHomeBuff[0] = 0;
  }
}

int run_flag = 1;
int ServerMain (int argc, char** argv)
{

	getHomeFromModulePath();
	std::string logfilename_str = std::string(gTestDumyServerHomeBuff) +  "../log/testDumyDaemon.log";
	const char* logFilename =  logfilename_str.c_str();
 
 
 
	 
	gLogger.SetLogFile(logFilename);
	gLogger.SetLogLevel(2);

	// set up common library error log
	SetAqLogger(&gLogger);

 
  
	gLogger.LogMessage("----  start  ---- \n");
	gLogger.FlushLog();
 
#if 0
	int dicomServerHaspStatus = LicenseManager::theLicenseManager().CheckFeature(ePREXION_PXDICOMSERVER3);

	gLogger.LogMessage("License CheckFeature %d \n",dicomServerHaspStatus);
	gLogger.FlushLog();
#else
	gLogger.LogMessage("none License Check \n");
	gLogger.FlushLog();
#endif


#if 1
	//
	//	Initialize MergeCOM DICOM library
	//
	int status;
	//	Initialize the toolkit
//	status = XTDcmLib::MC_Library_Initialization(MC_Config_Values, MC_Dictionary_Values, NULL);
	
	status = getTestID();

	if (status != MC_NORMAL_COMPLETION ){
		gLogger.LogMessage("MC_Library_Initialization error \n");
		gLogger.FlushLog();
	}else{
		gLogger.LogMessage("MC_Library_Initialization OK \n");
		gLogger.FlushLog();
	};
#else
	CTstDumyCls tstDumyDll;

	gLogger.LogMessage("tstDumyDll getID %d \n",tstDumyDll.getTestID());
	gLogger.FlushLog();
#endif

	int run_count = 0;
	while(run_flag == 1){
 
		gLogger.LogMessage("running %d \n",run_count++ );
		gLogger.FlushLog();

		::Sleep(1000);
	}
	gLogger.LogMessage("---- all end ---- \n" );
	gLogger.FlushLog();

 

	return 0;
}

int	TestDumyServiceProcessor::PreProcess(void)
{
	return 0;
}

int TestDumyServiceProcessor::Process(int argc, char **argv)
{
	m_stop = 0;
	int rcode = ServerMain(argc, argv);
	m_stop = 1;
	return rcode;
}

void TestDumyServiceProcessor::Stop(void)
{
//	Listener::theListener().RequestTermination(1); // trigger main loop to break

	 run_flag = 0;
 
}

 