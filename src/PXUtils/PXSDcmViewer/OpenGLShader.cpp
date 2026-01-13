#include <gl/glew.h>
#include <gl/glut.h>

#include <math.h>

#include "OpenGLShader.h"

#include	<fstream>
#include	<iostream>
#include	<sstream>
using namespace std;

 #ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

 



#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)

COpenGLShader::COpenGLShader( )
{
	 m_VertShaderId = InvalidID;
	 m_FragShaderId = InvalidID;
	 m_ShaderProgramId = InvalidID;
}
COpenGLShader::~COpenGLShader()
{
	 if(m_VertShaderId != InvalidID) glDeleteObjectARB(m_VertShaderId);
	 if(m_FragShaderId != InvalidID) glDeleteObjectARB(m_FragShaderId);
	 if(m_ShaderProgramId != InvalidID) glDeleteObjectARB(m_ShaderProgramId);
}
 


bool COpenGLShader::Compile( GLuint  ShaderID)
{
	// check whether error occurred
	
	if ( glGetError() != GL_NO_ERROR ) {
		 
		return false;
	}

	// compile
	
	glCompileShaderARB( ShaderID );

	// get errors
	
	GLint	result;
	glGetObjectParameterivARB( ShaderID, GL_OBJECT_COMPILE_STATUS_ARB, &result );
	
	if ( glGetError() != GL_NO_ERROR || result == GL_FALSE ) {
		cerr << "ShaderObject::Compile(): cannot compile shader: "<< endl;

		int	length;
		glGetObjectParameterivARB( ShaderID, GL_OBJECT_INFO_LOG_LENGTH_ARB,
								   &length );
		if ( length > 0 ) {
			int	l;
			GLcharARB *info_log = new GLcharARB[ length ];
			glGetInfoLogARB( ShaderID, length, &l, info_log );
			cerr << info_log << endl;
			delete [] info_log;
		}
		return false;
	}

	return true;
}

bool COpenGLShader::ReadShaderObjectFromFile( GLuint &out_Shader,const std::string& filename, const GLenum shader_type )
{
	std::string	source;
	// create
	
 
		out_Shader  = glCreateShaderObjectARB( shader_type );
	//	if ( glGetError() != GL_NO_ERROR ) {
		if(	 CHECK_GL_ERROR() !=0){
			return false ;
		}
	 

 
	// read source file

	try{
		ifstream	f_in( filename.c_str(), ios::binary );
		if ( f_in.fail()) {
			 
			throw(1);
		}
		ostringstream	str_out;
		str_out << f_in.rdbuf();
		source = str_out.str();
		f_in.close();

		// set shader source

		const char	*src_s = source.c_str();
		int src_size = source.length();
		
		glShaderSourceARB( out_Shader, 1, &src_s, &src_size );
		if ( glGetError() != GL_NO_ERROR ) {
			 
			throw(1);
		}

		// compile
		if(!Compile(out_Shader)){
			throw(1);
		}
	}catch(int err_no)
	{
		glDeleteObjectARB(out_Shader);
		return false;
	}

	return true;
}
bool COpenGLShader::ReadShaderObjectFromString( GLuint &out_Shader,const std::string& stringBuff, const GLenum shader_type )
{
	std::string	source;
	// create
	
 
		out_Shader  = glCreateShaderObjectARB( shader_type );
	//	if ( glGetError() != GL_NO_ERROR ) {
		if(	 CHECK_GL_ERROR() !=0){
			return false ;
		}
	 

 
	// read source file

	try{
	 

		const char	*src_s = stringBuff.c_str();
		int src_size = stringBuff.length();
		
		glShaderSourceARB( out_Shader, 1, &src_s, &src_size );
		if ( glGetError() != GL_NO_ERROR ) {
			 
			throw(1);
		}

		// compile
		if(!Compile(out_Shader)){
			throw(1);
		}
	}catch(int err_no)
	{
		glDeleteObjectARB(out_Shader);
		return false;
	}

	return true;
}

bool COpenGLShader::createVertShaderFromFile(const std::string& VertFileName,GLuint &ShaderID)
{
	 
	if(!ReadShaderObjectFromFile(ShaderID,VertFileName,GL_VERTEX_SHADER_ARB)){
		return false;
	}

	return true;
}
bool COpenGLShader::createFragShaderFromFile(const std::string& FragFileName,GLuint &ShaderID)
{
	if(!checkOpenGLShader()){
		return false;
	}

	if(!ReadShaderObjectFromFile(ShaderID,FragFileName,GL_FRAGMENT_SHADER_ARB)){
		return false;
	}

	return true;
}

bool COpenGLShader::createVertShaderFromString(const std::string& VertString,GLuint &ShaderID)
{
	 
	if(!ReadShaderObjectFromString(ShaderID,VertString,GL_VERTEX_SHADER_ARB)){
		return false;
	}

	return true;
}
bool COpenGLShader::createFragShaderFromString(const std::string& FragString,GLuint &ShaderID)
{
	if(!checkOpenGLShader()){
		return false;
	}

	if(!ReadShaderObjectFromString(ShaderID,FragString,GL_FRAGMENT_SHADER_ARB)){
		return false;
	}

	return true;
}

bool COpenGLShader::createAndSetupShader(const std::string& VertFileName,const std::string& FragFileName)
{
	if(!checkOpenGLShader()){
		return false;
	}
#if 1
	glUseProgramObjectARB(0);
	 

	if(m_ShaderProgramId != InvalidID) {

#if 0
		if(m_VertShaderId != InvalidID) {
			glDetachObjectARB(m_ShaderProgramId,m_VertShaderId);
			if(	 CHECK_GL_ERROR() !=0){
				return false ;
			}
		}
		  
		if(m_FragShaderId != InvalidID) {
			glDetachObjectARB(m_ShaderProgramId,m_FragShaderId);
			if(	 CHECK_GL_ERROR() !=0){
				return false ;
			}
		}
#endif
		glDeleteObjectARB(m_ShaderProgramId);
		if(	 CHECK_GL_ERROR() !=0){
			return false ;
		}
	}
 
#if 1
	if(m_VertShaderId != InvalidID) {
		 
		glDeleteObjectARB(m_VertShaderId);
		if(	 CHECK_GL_ERROR() !=0){
			return false ;
		}
	}
	  
	if(m_FragShaderId != InvalidID) {
		 
		glDeleteObjectARB(m_FragShaderId);
		if(	 CHECK_GL_ERROR() !=0){
			return false ;
		}
	}
#endif
	
	
#endif

	if(!ReadShaderObjectFromFile(m_VertShaderId,VertFileName,GL_VERTEX_SHADER_ARB)){
		return false;
	}

	if(!ReadShaderObjectFromFile(m_FragShaderId,FragFileName,GL_FRAGMENT_SHADER_ARB)){
		return false;
	}
 
	//
	//プログラムオブジェクトの作成        
	m_ShaderProgramId = glCreateProgramObjectARB(); 
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	glAttachObjectARB(m_ShaderProgramId,m_VertShaderId);    
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	glAttachObjectARB(m_ShaderProgramId,m_FragShaderId);
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	GLint linked;        
	glLinkProgramARB(m_ShaderProgramId); 
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	glGetObjectParameterivARB(m_ShaderProgramId, GL_LINK_STATUS, &linked);        
	 
	if ( glGetError() != GL_NO_ERROR || m_ShaderProgramId == GL_FALSE ) {
			cerr << " Link(): cannot link program object"
			 << endl;
		
		int	length;
		glGetObjectParameterivARB( m_ShaderProgramId, GL_OBJECT_INFO_LOG_LENGTH_ARB,
								   &length );
		if ( length > 0 ) {
			int	l;
			GLcharARB *info_log = new GLcharARB[ length ];
			glGetInfoLogARB( m_ShaderProgramId, length, &l, info_log );
			cerr << info_log << endl;
			delete [] info_log;
		}
		 
		return false;
	}
 

	return true;
}

bool COpenGLShader::setupShader(GLuint VertShader,GLuint FragShader)
{
 

	glUseProgramObjectARB(0);
	 
	if(m_ShaderProgramId != InvalidID) {
		//
		if(m_VertShaderId != InvalidID) {
			glDetachObjectARB(m_ShaderProgramId,m_VertShaderId);
			if(	 CHECK_GL_ERROR() !=0){
				return false ;
			}
		}
		  
		if(m_FragShaderId != InvalidID) {
			glDetachObjectARB(m_ShaderProgramId,m_FragShaderId);
			if(	 CHECK_GL_ERROR() !=0){
				return false ;
			}
		}

		glDeleteObjectARB(m_ShaderProgramId);
		if(	 CHECK_GL_ERROR() !=0){
			return false ;
		}
	}

	m_VertShaderId = VertShader;
	m_FragShaderId = FragShader;

	//プログラムオブジェクトの作成        
	m_ShaderProgramId = glCreateProgramObjectARB(); 
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	glAttachObjectARB(m_ShaderProgramId,m_VertShaderId);    
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	glAttachObjectARB(m_ShaderProgramId,m_FragShaderId);
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	GLint linked;        
	glLinkProgramARB(m_ShaderProgramId); 
	if(	 CHECK_GL_ERROR() !=0){
		return false ;
	}
	glGetObjectParameterivARB(m_ShaderProgramId, GL_LINK_STATUS, &linked);        
	 
	if ( glGetError() != GL_NO_ERROR || m_ShaderProgramId == GL_FALSE ) {
			cerr << " Link(): cannot link program object"
			 << endl;
		
		int	length;
		glGetObjectParameterivARB( m_ShaderProgramId, GL_OBJECT_INFO_LOG_LENGTH_ARB,
								   &length );
		if ( length > 0 ) {
			int	l;
			GLcharARB *info_log = new GLcharARB[ length ];
			glGetInfoLogARB( m_ShaderProgramId, length, &l, info_log );
			cerr << info_log << endl;
			delete [] info_log;
		}
		 
		return false;
	}
 

	return true;
}
GLint
COpenGLShader::GetUniformLocation( const char *name )
	{
		GLint ul = glGetUniformLocationARB(  m_ShaderProgramId, name );
		if ( ul == -1 ) {
			std::cerr << "ProgramObject::GetUniformLocation(): "
					  << "no such uniform named " << name << std::endl;
		}
		return ul;
	}

GLint
	COpenGLShader::GetAttribLocation( const char *name )
	{
		GLint al = glGetAttribLocationARB( m_ShaderProgramId, name );
		if ( al == -1 ) {
			std::cerr << "ProgramObject::GetAttribLocation(): "
					  << "no such attribute named " << name << std::endl;
		}
		return al;
	}
/////////////////
// uniform variable
	
	// int

	void COpenGLShader::
	SetUniform1i( const char *name, GLint v0 )
	{
		glUniform1iARB( GetUniformLocation( name ), v0 );
	}

	void COpenGLShader::
	SetUniform2i( const char *name, GLint v0, GLint v1 )
	{
		glUniform2iARB( GetUniformLocation( name ), v0, v1 );
	}

	void COpenGLShader::
	SetUniform3i( const char *name, GLint v0, GLint v1, GLint v2 )
	{
		glUniform3iARB( GetUniformLocation( name ), v0, v1, v2 );
	}

	void COpenGLShader::
	SetUniform4i( const char *name, GLint v0, GLint v1, GLint v2, GLint v3 )
	{
		glUniform4iARB( GetUniformLocation( name ), v0, v1, v2, v3 );
	}

	// float

	void COpenGLShader::
	SetUniform1f( const char *name, GLfloat v0 )
	{
		glUniform1fARB( GetUniformLocation( name ), v0 );
	}

	void COpenGLShader::
	SetUniform2f( const char *name, GLfloat v0, GLfloat v1 )
	{
		glUniform2fARB( GetUniformLocation( name ), v0, v1 );
	}

	void COpenGLShader::
	SetUniform3f( const char *name, GLfloat v0, GLfloat v1, GLfloat v2 )
	{
		glUniform3fARB( GetUniformLocation( name ), v0, v1, v2 );
	}

	void COpenGLShader::
	SetUniform4f( const char *name,
				  GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
	{
		glUniform4fARB( GetUniformLocation( name ), v0, v1, v2, v3 );
	}

	// int array

	void COpenGLShader::
	SetUniform1iv( const char *name, GLuint count, const GLint *v )
	{
		glUniform1ivARB( GetUniformLocation( name ), count, v );
	}

	void COpenGLShader::
	SetUniform2iv( const char *name, GLuint count, const GLint *v )
	{
		glUniform2ivARB( GetUniformLocation( name ), count, v );
	}

	void COpenGLShader::
	SetUniform3iv( const char *name, GLuint count, const GLint *v )
	{
		glUniform3ivARB( GetUniformLocation( name ), count, v );
	}

	void COpenGLShader::
	SetUniform4iv( const char *name, GLuint count, const GLint *v )
	{
		glUniform4ivARB( GetUniformLocation( name ), count, v );
	}

	// float array

	void COpenGLShader::
	SetUniform1fv( const char *name, GLuint count, const GLfloat *v )
	{
		glUniform1fvARB( GetUniformLocation( name ), count, v );
	}

	void COpenGLShader::
	SetUniform2fv( const char *name, GLuint count, const GLfloat *v )
	{
		glUniform2fvARB( GetUniformLocation( name ), count, v );
	}

	void COpenGLShader::
	SetUniform3fv( const char *name, GLuint count, const GLfloat *v )
	{
		glUniform3fvARB( GetUniformLocation( name ), count, v );
	}

	void COpenGLShader::
	SetUniform4fv( const char *name, GLuint count, const GLfloat *v )
	{
		glUniform4fvARB( GetUniformLocation( name ), count, v );
	}

	// matrix

	void COpenGLShader::
	SetMatrix2fv( const char *name, GLuint count, GLboolean transpose,
				  const GLfloat *v )
	{
		glUniformMatrix2fvARB( GetUniformLocation( name ), count,
							   transpose, v );
	}

	void COpenGLShader::
	SetMatrix3fv( const char *name, GLuint count, GLboolean transpose,
				  const GLfloat *v )
	{
		glUniformMatrix3fvARB( GetUniformLocation( name ), count,
							   transpose, v );
	}

	void COpenGLShader::
	SetMatrix4fv( const char *name, GLuint count, GLboolean transpose,
				  const GLfloat *v )
	{
		glUniformMatrix4fvARB( GetUniformLocation( name ), count,
							   transpose, v );
	}

	// attribute variable
	
	// float

	void COpenGLShader::
	SetAttrib1f( GLint al, GLfloat v0 )
	{
		glVertexAttrib1fARB( al, v0 );
	}

	void COpenGLShader::
	SetAttrib2f( GLint al, GLfloat v0, GLfloat v1 )
	{
		glVertexAttrib2fARB( al, v0, v1 );
	}

	void COpenGLShader::
	SetAttrib3f( GLint al, GLfloat v0, GLfloat v1, GLfloat v2 )
	{
		glVertexAttrib3fARB( al, v0, v1, v2 );
	}

	void COpenGLShader::
	SetAttrib4f( GLint al, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
	{
		glVertexAttrib4fARB( al, v0, v1, v2, v3 );
	}

	// float array

	void COpenGLShader::
	SetAttrib1fv( GLint al, const GLfloat *v )
	{
		glVertexAttrib1fvARB( al, v );
	}

	void COpenGLShader::
	SetAttrib2fv( GLint al, const GLfloat *v )
	{
		glVertexAttrib2fvARB( al, v );
	}

	void COpenGLShader::
	SetAttrib3fv( GLint al, const GLfloat *v )
	{
		glVertexAttrib3fvARB( al, v );
	}

	void COpenGLShader::
	SetAttrib4fv( GLint al, const GLfloat *v )
	{
		glVertexAttrib4fvARB( al, v );
	}
///////////

void COpenGLShader::activeShader(bool onOff,bool isRGBA )
{
	if(onOff){
		glUseProgramObjectARB(m_ShaderProgramId);
		 
		//
		SetUniform1i("rt_w",m_sizeX); 
		SetUniform1i("rt_h",m_sizeY);
		//
		SetUniform1i("sceneTex", 0);
// 		SetUniform1i("lutTex", 1);

		SetUniform1i("isRGBA", isRGBA? 1:0);
 

	}else{
		glUseProgramObjectARB(0);
		 
	}

	return ;
}
  