#ifndef OPENGL_CPU_SHADER_VIEWER_H
#define OPENGL_CPU_SHADER_VIEWER_H
 
 

#include "OpenGLExt.h"

class QImage;
class COpenGLCpuShader   : public COpenGLExt
{
	 
public:
    COpenGLCpuShader( );
	~COpenGLCpuShader();
	 
///////////
protected:
	bool  Compile( GLuint  ShaderID);
	 
	//
 
	 
};
 



#endif //OPENGL_CPU_SHADER_VIEWER_H


