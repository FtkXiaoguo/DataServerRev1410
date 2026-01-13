#ifndef OPENGL_SHADER_VIEWER_H
#define OPENGL_SHADER_VIEWER_H
 
 

#include "OpenGLExt.h"

class QImage;
class COpenGLShader   : public COpenGLExt
{
	 
public:
    COpenGLShader( );
	~COpenGLShader();
	 
	//
	bool createVertShaderFromFile(const std::string& VertFileName,GLuint &ShaderID);
	bool createFragShaderFromFile(const std::string& FragFileName,GLuint &ShaderID);
	bool createVertShaderFromString(const std::string& VertString,GLuint &ShaderID);
	bool createFragShaderFromString(const std::string& FragString,GLuint &ShaderID);


	bool setupShader(GLuint VertShader,GLuint FragShader);
	bool createAndSetupShader(const std::string& VertFileName,const std::string& FragFileName);
	virtual void activeShader(bool onOff,bool isRGBA=false);

	 
/////
	GLint
	GetUniformLocation( const char *name );

	GLint
	GetAttribLocation( const char *name );

	// uniform variable
	
	// int

	void
	SetUniform1i( const char *name, GLint v0 );

	void
	SetUniform2i( const char *name, GLint v0, GLint v1 );

	void
	SetUniform3i( const char *name, GLint v0, GLint v1, GLint v2 );

	void
	SetUniform4i( const char *name, GLint v0, GLint v1, GLint v2, GLint v3 );

	// float

	void
	SetUniform1f( const char *name, GLfloat v0 );

	void
	SetUniform2f( const char *name, GLfloat v0, GLfloat v1 );

	void
	SetUniform3f( const char *name, GLfloat v0, GLfloat v1, GLfloat v2 );

	void
	SetUniform4f( const char *name,
				  GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 );

	// int array

	void
	SetUniform1iv( const char *name, GLuint count, const GLint *v );

	void
	SetUniform2iv( const char *name, GLuint count, const GLint *v );

	void
	SetUniform3iv( const char *name, GLuint count, const GLint *v );

	void
	SetUniform4iv( const char *name, GLuint count, const GLint *v );

	// float array

	void
	SetUniform1fv( const char *name, GLuint count, const GLfloat *v );

	void
	SetUniform2fv( const char *name, GLuint count, const GLfloat *v );

	void
	SetUniform3fv( const char *name, GLuint count, const GLfloat *v );

	void
	SetUniform4fv( const char *name, GLuint count, const GLfloat *v );

	// matrix

	void
	SetMatrix2fv( const char *name, GLuint count, GLboolean transpose,
				  const GLfloat *v );

	void
	SetMatrix3fv( const char *name, GLuint count, GLboolean transpose,
				  const GLfloat *v );

	void
	SetMatrix4fv( const char *name, GLuint count, GLboolean transpose,
				  const GLfloat *v );

	// attribute variable
	
	// float

	void
	SetAttrib1f( GLint al, GLfloat v0 );

	void
	SetAttrib2f( GLint al, GLfloat v0, GLfloat v1 );

	void
	SetAttrib3f( GLint al, GLfloat v0, GLfloat v1, GLfloat v2 );

	void
	SetAttrib4f( GLint al, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 );

	// float array

	void SetAttrib1fv( GLint al, const GLfloat *v );
	void SetAttrib2fv( GLint al, const GLfloat *v );
	void SetAttrib3fv( GLint al, const GLfloat *v );
	void SetAttrib4fv( GLint al, const GLfloat *v );
///////////
protected:
	bool  Compile( GLuint  ShaderID);
	bool ReadShaderObjectFromFile( GLuint &out_Shader,const std::string& filename, const GLenum shader_type );
	bool ReadShaderObjectFromString( GLuint &out_Shader,const std::string& stringBuff, const GLenum shader_type );
	//
	GLuint m_VertShaderId;
	GLuint m_FragShaderId;
	//
	GLuint m_ShaderProgramId;
	//
 
	 
};
 



#endif //OPENGL_SHADER_VIEWER_H


