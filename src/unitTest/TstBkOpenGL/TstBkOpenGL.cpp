/***********************************************************************
 
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#define _WIN32_DCOM

#include "TstBkOpenGL.h"

#include "Globals.h"
#include "rtvPoolAccess.h"

//#include "AQNetDB.h"
 
#include "OpenGLBkProc.h"
  #include "ImplantSimLib\ImplantOpenGL.h"

#include "ImplantOpenGLConnect.h";

extern TRLogger gLogger;
 
//-----------------------------------------------------------------------------
//
char *ToUpper(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = toupper(*s);
	return ret;
}

//-----------------------------------------------------------------------------
//
static char* GetMyName(void)
{
	static char myName[128];

	if (myName[0])
		return myName;
	
	char *p;
	gethostname(myName, sizeof(myName)-1);
	if ( ( p = strchr(myName,'.')))
		*p = '\0';
	return myName;
}


TestOpenGLBase::TestOpenGLBase() 
{
	m_initDBFlag = false;
	m_PatientName = "testDb**";
	
	m_ImpOpenGL = new CImplantOpenGL;
	//
	m_startProc = new COpenGLBkProc;

	m_proc_dom_hd = new OpenGLProc_COM;
	//
	m_ImplantOpenGLConClient = new CImplantOpenGLConClient;
}
TestOpenGLBase::~TestOpenGLBase() 
{
	destroy();
	delete m_ImpOpenGL;
	delete m_proc_dom_hd;
	delete m_startProc;

	delete m_ImplantOpenGLConClient;
}
void TestOpenGLBase::destroy()
{
//	delete this;
}

extern char g_run_bk_exe_path[];

bool TestOpenGLBase::doInitDB()
{
	if(m_initDBFlag){
		return true;
	}

	 
	bool testSQLite = true;


	bool run_bk_proc = true;
	if(run_bk_proc){
		 
		char *run_cmd = g_run_bk_exe_path;//"PxOpenGLBK.exe";
		m_startProc->startMyProc(m_proc_dom_hd,run_cmd);
	}
	
	m_initDBFlag = true;

	::Sleep(300);

	m_ImplantOpenGLConClient->Init(m_startProc->getSessionID() == 0);

	return true;
}

  
int TestOpenGLBase::Process(void)
{
	doInitDB();

 
	int timeoutCounts = 0;

	 m_run_count = 0;
	while(!TerminationRequested())
	{
		::Sleep(2000);
	 
//		gLogger.LogMessage("TestOpenGLBase: run_count %d [%s]\n",m_run_count,m_processorName);
//		gLogger.FlushLog();

//		doTstOpenGLMyself();
		doDBProc();
		m_run_count++;
	}

	return 0;
}

 bool TestOpenGLBase::doDBProc()
{
	gLogger.LogMessage(" doDBProc \n");
	gLogger.FlushLog();

#if 0
	OpenGLProc_COM proc_dom_hd;

	COpenGLBkProc startProc;
	char *run_cmd = "PxOpenGLBK.exe";
	startProc.startMyProc(&proc_dom_hd,run_cmd);
#endif

	char _data_[10];
	*(int*)_data_ = m_run_count;

	if(m_run_count ==0){
		m_ImplantOpenGLConClient->exec_cmd(ImplantOpenGLCmd_init,_data_,10);
	}else{
		m_ImplantOpenGLConClient->exec_cmd(ImplantOpenGLCmd_render,_data_,10);
	}


	return true;
}


 bool TestOpenGLBase::doTstOpenGLMyself()
 {
	 gLogger.LogMessage(" doTstOpenGLMyself \n");
	 gLogger.FlushLog();

	CImplantOpenGL ImpOpenGL;
	ImpOpenGL.tstDraw();


	return true;
 }