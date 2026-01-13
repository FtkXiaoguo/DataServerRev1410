#include <gl/glew.h>
#include <gl/glut.h>

#include <math.h>

#include "OpenGLExt.h"

#include	<fstream>
#include	<iostream>
#include	<sstream>
using namespace std;

 #ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

 
//#define DBG_NON_Shader
 
static char _error_buff_[2048*4];
// GL ERROR CHECK
int COpenGLExt::CheckGLError(char *file, int line)
   {
	   GLenum glErr;
	   int    retCode = 0;

	   glErr = glGetError();

	   char *error_out_p = _error_buff_;
	   error_out_p[0] = 0;
	   while (glErr != GL_NO_ERROR) 
       {
	     const GLubyte* sError = gluErrorString(glErr);

		 if (sError){
			 sprintf(error_out_p,"GL Error # %d ( %s ) in File %s at line: %d \n",glErr,gluErrorString(glErr),file,line);
			//cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << endl;
		}
		 else
		 {
			 sprintf(error_out_p,"GL Error # %d (no message available) in File %s at line: %d \n",glErr,file,line);
		   // cout << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << endl;
		}
			
		 retCode = 1;
		 glErr = glGetError();

		 error_out_p += strlen(error_out_p);
	   }
	   return retCode;
   }
   
#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)

COpenGLExt::COpenGLExt( )
{
	  
}
COpenGLExt::~COpenGLExt()
{
	 
}
bool COpenGLExt::checkOpenGLShader()
{
	bool ret_b = true;
	m_GlExtSupported = " ";
#if 1
	if (!glutExtensionSupported("GL_ARB_vertex_program")) {
 //     printf("Sorry, this demo requires GL_ARB_vertex_program\n");
      ret_b = false;
		m_GlExtSupported += " GL_ARB_vertex_program not supported";
	}else{
		m_GlExtSupported += " GL_ARB_vertex_program is supported";
	}
   if (!glutExtensionSupported("GL_ARB_fragment_program")) {
  //    printf("Sorry, this demo requires GL_ARB_fragment_program\n");
      ret_b = false;
	  m_GlExtSupported += " GL_ARB_fragment_program not supported";
   }else{
	   m_GlExtSupported += " GL_ARB_fragment_program is supported";
   }
   if(!glutExtensionSupported("GL_ARB_shader_objects")) {
	    printf("Sorry, this demo requires GL_ARB_fragment_program\n");
      ret_b = false;
	  m_GlExtSupported += " GL_ARB_shader_objects not supported";
   }else{
		m_GlExtSupported += " GL_ARB_shader_objects is supported";
   }
#endif

#ifdef DBG_NON_Shader
   ret_b = false;
#endif
   return ret_b;
}
bool COpenGLExt::m_bHasMultitexture = false;
bool COpenGLExt::m_initFlag = false;
std::string COpenGLExt::m_GlExtensionAll;
std::string COpenGLExt::m_GlExtSupported;
bool COpenGLExt::init()
{
	 if(m_initFlag) return m_bHasMultitexture;
   GLenum	glew_error = glewInit();
	if ( glew_error != GLEW_OK ) {
		cerr << "error: " << glewGetErrorString( glew_error ) << endl;
		return false;
	}
	m_bHasMultitexture = (1==glewGetExtension("GL_ARB_multitexture"));
	if ( !m_bHasMultitexture ) {
		cerr << "error: " << "GL_ARB_multitexture not supported!" << endl;
		return false;
	}

#ifdef DBG_NON_Shader
   return false;
#endif

	const GLubyte *gl_ext = glGetString(GL_EXTENSIONS);// glGetString（GL_EXTENSIONS）；

	m_GlExtensionAll = (const char *)gl_ext;
//////////////
	m_initFlag = true;
	return true;
}


bool COpenGLExt::bindTexture(int textureNo,GLuint texture,GLenum target)
{
	if(m_bHasMultitexture){
	 
		glActiveTextureARB(GL_TEXTURE0_ARB+textureNo);
 
	}
	glBindTexture(target, texture);
	return true;
}

bool COpenGLExt::createTexture2D(const unsigned short *image_data, int sizeX,int sizeY, GLuint &texture)
{
	return false;
   //not yet

	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);//GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);//GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_SHORT, image_data);

	 glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, sizeX, sizeY, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, image_data);

//	 int actual_format;
//	GetTexLevelParameteriv(GL_TEXTURE_2D, 0, TEXTURE_INTERNAL_FORMAT, &actual_format);

	 bool dbg_flag = false;
	 if(dbg_flag){
		 unsigned short *_dbg_data_buff = new unsigned short[sizeX*sizeY];
		 memset(_dbg_data_buff,0,2*sizeX*sizeY);
	//	 glReadPixels(0,0,sizeX, sizeY,GL_COLOR_INDEX, GL_UNSIGNED_SHORT, _dbg_data_buff);
		 glGetTexImage(GL_TEXTURE_2D,0,GL_LUMINANCE, GL_UNSIGNED_SHORT, _dbg_data_buff);

		 delete [] _dbg_data_buff;
	 }
	return true;
}
bool COpenGLExt::createTexture2D(const float *image_data, int sizeX,int sizeY, GLuint &texture)
{
	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);//GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);//GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_SHORT, image_data);

//	 image_data[0] = 0.1f;
//	 image_data[1] = 0.2f;
//	 glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA16, sizeX, sizeY, 0, GL_ALPHA,GL_FLOAT, image_data);

#if 0
 	 glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16, sizeX, sizeY, 0, GL_LUMINANCE,GL_FLOAT, image_data);
 
#else
	 //画像のサイズを 2n にあわせる必要はありません
	 /* テクスチャの割り当て */
	  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE16, sizeX, sizeY, GL_LUMINANCE, GL_FLOAT, image_data);

#endif

//	 int actual_format;
//	GetTexLevelParameteriv(GL_TEXTURE_2D, 0, TEXTURE_INTERNAL_FORMAT, &actual_format);

	 bool dbg_flag = false;
	 if(dbg_flag){
		 float *_dbg_data_buff = new float[sizeX*sizeY];
		 memset(_dbg_data_buff,0,sizeof(float)*sizeX*sizeY);
	//	 glReadPixels(0,0,sizeX, sizeY,GL_COLOR_INDEX, GL_UNSIGNED_SHORT, _dbg_data_buff);
		 glGetTexImage(GL_TEXTURE_2D,0,GL_ALPHA, GL_FLOAT, _dbg_data_buff);

		 delete [] _dbg_data_buff;
	 }
	return true;
}
bool COpenGLExt::createTexture2D_RGBA(const unsigned char *image_data, int sizeX,int sizeY, GLuint &texture)
{
	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//	 glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);

//	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0,  GL_BGRA , GL_UNSIGNED_INT_8_8_8_8_REV, image_data);

	 return true;

}
bool COpenGLExt::createTexture2D_RGBA(const float *image_data, int sizeX,int sizeY, GLuint &texture)
{
	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//	 glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);

//	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

#if 0
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0,  GL_BGRA , GL_FLOAT, image_data);
#else
	 //画像のサイズを 2n にあわせる必要はありません
	 /* テクスチャの割り当て */
	  gluBuild2DMipmaps(GL_TEXTURE_2D,  GL_RGBA, sizeX, sizeY,  GL_BGRA , GL_FLOAT, image_data);

#endif
	 return true;

}

bool COpenGLExt::createTexture1D(const float *data,int size,GLuint &texture)
{
	glDisable(GL_TEXTURE_2D);

 	glEnable( GL_TEXTURE_1D ); 

	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_1D, texture);

 
	/* テクスチャを拡大・縮小する方法の指定 */
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
	/* テクスチャの繰り返し方法の指定 */
 	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	/* テクスチャユニット１のテクスチャ環境 */
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
 // glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA16, size, 0, GL_ALPHA,GL_FLOAT, data); 
	glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA16, size/2, 0, GL_ALPHA,GL_FLOAT, data);
 
	return true;
}
bool COpenGLExt::createTexture1D(const unsigned short *data,int size,GLuint &texture)
{
   return false;
   //not yet
	glDisable(GL_TEXTURE_2D);

 	glEnable( GL_TEXTURE_1D ); 

	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_1D, texture);

 
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   	glTexImage1D(GL_TEXTURE_1D, 0, GL_ALPHA16, size, 0, GL_ALPHA,GL_UNSIGNED_SHORT, data); 
 
	return true;
}
//
 
bool COpenGLExt::deleteTexture(GLuint texture)
{
 
	glDeleteTextures(1 , &texture);
	return true;

}