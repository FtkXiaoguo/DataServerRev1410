#include "ImplantOpenGL.h"


#include "libtgl.h"
#include "ImplantOpenGLConnect.h"

#include "AqCore/TRLogger.h"
extern TRLogger gLogger;

class CImplantOpenGLConServerImp : public CImplantOpenGLConServer
{
public:
	CImplantOpenGLConServerImp(CImplantOpenGL *owner){
		m_owner = owner;
	};
	~CImplantOpenGLConServerImp(){
	};
protected:
	virtual bool exec_init(char *arg_buff,int &size)
	{
		gLogger.LogMessage(" CImplantOpenGLConServerImp::exec_init \n" );
		gLogger.FlushLog();

		m_owner->tstDraw();
		return CImplantOpenGLConServer::exec_init(arg_buff, size);
	}
	virtual bool exec_render(char *arg_buff,int &size)
	{
		gLogger.LogMessage(" CImplantOpenGLConServerImp::exec_render \n" );
		gLogger.FlushLog();

		return CImplantOpenGLConServer::exec_render(arg_buff, size);
	}
	//
	CImplantOpenGL *m_owner;
};
CImplantOpenGL::CImplantOpenGL(void)
{
	m_initFlag = false;
	m_tstgl = new TGL;
}

CImplantOpenGL::~CImplantOpenGL(void)
{
	delete m_tstgl;
}
int CImplantOpenGL::init_gl( int sx, int sy )
{
	int retcd = m_tstgl->InitWindow( sx, sy );

	m_initFlag = true;
	return retcd;
}

void CImplantOpenGL::tstDraw()
{
	init_gl(256,256);
}

void CImplantOpenGL::doTst(bool session0Flag)
{
	gLogger.LogMessage(" CImplantOpenGL::doTst \n" );
	gLogger.FlushLog();

	CImplantOpenGLConServer *pOpenGLConServer = new CImplantOpenGLConServerImp(this);

	gLogger.LogMessage(" CImplantOpenGL::doTst 1\n" );
	gLogger.FlushLog();

	pOpenGLConServer->Init( session0Flag);
	pOpenGLConServer->Exec();

	pOpenGLConServer->Term();


	delete pOpenGLConServer;
}