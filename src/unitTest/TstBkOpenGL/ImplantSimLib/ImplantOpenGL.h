#pragma once

class TGL;
class CImplantOpenGL
{
public:
	CImplantOpenGL(void);
	~CImplantOpenGL(void);

	void doTst(bool session0Flag);
	void tstDraw();
protected:
	bool m_initFlag;
	 int init_gl( int sx, int sy );
///
TGL *m_tstgl;
};
