/***********************************************************************
 * PXDcmSServer.cpp
 *---------------------------------------------------------------------
 *    Copyright, PreXion 2011, All rights reserved.
 *
 *		 PX DICOM  SERVER
 *
 *
 *
 *-------------------------------------------------------------------
 */

#pragma warning (disable: 4503)
#include <io.h>

#include "rtvsocket.h"
#include "rtvPoolAccess.h"

 
#include "AQNetImageServerDumy.h"

 

 
 
PXAqnetImageServerDumy processor("AQNetImageServer");
 

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
//#undef DBG_APP_MODE

 
int main(int argc, char *argv[])
{
	 
 
 	service.Start(argc, argv);
 
    return 0;
}
 
static int g_toFinish  = 0;
int ServerMain (int argc, char** argv)
{
	 
	while(g_toFinish == 0){
		::Sleep(1000);
	}

	return 0;
}

int	PXAqnetImageServerDumy::PreProcess(void)
{
	return 0;
}

int PXAqnetImageServerDumy::Process(int argc, char **argv)
{
	m_stop = 0;
	int rcode = ServerMain(argc, argv);
	m_stop = 1;

	return rcode;
}

void PXAqnetImageServerDumy::Stop(void)
{
	g_toFinish = true;
	
}
