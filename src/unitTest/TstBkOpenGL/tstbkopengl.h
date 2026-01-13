/***********************************************************************
 
 *-------------------------------------------------------------------
 */
#ifndef TEST_BK_OPENGL_H
#define TEST_BK_OPENGL_H

#include "rtvthread.h"

 
class CImplantOpenGL;
class COpenGLBkProc;
class  OpenGLProc_COM;
class CImplantOpenGLConClient;
class TestOpenGLBase : public iRTVThreadProcess
{	
public:
	void destroy();
	int Process(void);
	void setPatientName(std::string name){
		m_PatientName = name;
	}
	TestOpenGLBase() ;
protected:
	
	virtual ~TestOpenGLBase() ;

	////////////////////////////
 	///////////////////////////

 
 
	virtual bool doDBProc() ;
	bool doTstOpenGLMyself();
 
	bool doInitDB();

 	bool m_initDBFlag;

	int m_iGroupID ;
	int m_iUserID  ;

	std::string m_PatientName;

	int m_run_count;
	//
	CImplantOpenGL *m_ImpOpenGL;

	COpenGLBkProc  *m_startProc;

	OpenGLProc_COM *m_proc_dom_hd;

	/////////
	CImplantOpenGLConClient *m_ImplantOpenGLConClient;
};
 
 
#endif // TEST_BK_OPENGL_H
