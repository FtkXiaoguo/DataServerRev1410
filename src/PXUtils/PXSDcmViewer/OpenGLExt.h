#ifndef OPENGL_EXT_VIEWER_H
#define OPENGL_EXT_VIEWER_H
 
#include "string"

 #include	<iostream>

#define InvalidID (0xffffffff)


class QImage;
class COpenGLExt   
{
	 
public:
    COpenGLExt( );
	~COpenGLExt();
	static bool checkOpenGLShader();
 
	static bool init();
	static std::string getGlExtSupported() { return m_GlExtSupported;};
	static std::string getGlExtension() { return m_GlExtensionAll;};
	//

	virtual void activeShader(bool onOff,bool isRGBA=false){};
 

	bool  hasMultitexture() { return m_bHasMultitexture;} 

	bool bindTexture(int textureNo,GLuint texture,GLenum target);

	bool createTexture2D(const unsigned short *image_data, int sizeX,int sizeY, GLuint &texture);
	bool createTexture2D(const float *image_data, int sizeX,int sizeY, GLuint &texture);
	bool createTexture2D_RGBA(const unsigned char *image_data, int sizeX,int sizeY, GLuint &texture);
	bool createTexture2D_RGBA(const float *image_data, int sizeX,int sizeY, GLuint &texture);

	bool createTexture1D(const float *data,int size,GLuint &texture);
	bool createTexture1D(const unsigned short *data,int size,GLuint &texture);

	bool deleteTexture(GLuint texture);
	//
	void setupSize(int sizeX,int sizeY){ m_sizeX = sizeX;m_sizeY = sizeY;};
/////////
	 
 ////
protected:
	int CheckGLError(char *file, int line);

	int m_sizeX;
	int m_sizeY;
 

	QImage *m_image_temp;
 
 
#if 0
	//
	GLuint m_VertShaderId;
	GLuint m_FragShaderId;
	//
	GLuint m_sharderProgramId;
	//
#endif
	static std::string m_GlExtSupported;
	static std::string m_GlExtensionAll;

	static bool m_bHasMultitexture;
	static bool m_initFlag;
};
 



#endif //OPENGL_EXT_VIEWER_H


