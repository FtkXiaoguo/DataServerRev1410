#ifndef IMAGEPRC_SHADER_H
#define IMAGEPRC_SHADER_H
 
 #include "imageProcBase.h"
 
class COpenGLShader;
 
class CImageFilterNullShader : public  CImageFilterNull
{
public:
	CImageFilterNullShader(COpenGLShader *shader) ;
	~CImageFilterNullShader() ;

	virtual bool loadFilter();
	virtual bool activeFilter();
	virtual bool doFilter() ;
	
protected:
	 
	bool m_ShaderLoaded;
	unsigned int m_VerShardID;
	unsigned int m_FragShardID;
	COpenGLShader *m_shader;
	float *m_histogram;
	//
	

};

class CImageFirFilterShader : public  CImageFirFilter 
{
public:
	CImageFirFilterShader(COpenGLShader *shader);
	~CImageFirFilterShader();

	virtual bool loadFilter();
	virtual bool activeFilter();
	/////////
	virtual bool doFilter ();
protected:
	unsigned int m_GaussianTex;

	bool m_ShaderLoaded;
	unsigned int m_VerShardID;
	unsigned int m_FragShardID;
	COpenGLShader *m_shader;
};
//////
class CImageBiLateralFilterShader : public  CImageBiLateralFilter 
{
public:
	CImageBiLateralFilterShader(COpenGLShader *shader);
	~CImageBiLateralFilterShader();

	virtual bool loadFilter();
	virtual bool activeFilter();
	/////////
	 
	virtual bool doFilter ();
protected:
	unsigned int m_GaussianTex;

	bool m_ShaderLoaded;
	unsigned int m_VerShardID;
	unsigned int m_FragShardID;
	COpenGLShader *m_shader;
};

class CImageProcShader   : public CImageProcBase
{
	 
public:
    CImageProcShader(COpenGLShader *shader);
	~CImageProcShader();
	 virtual bool isShader() { return true;};
	
	virtual bool createFilter(const std::string &name, int type);
	virtual void selCurrentFilter(const std::string &name) ;

	
protected:
	COpenGLShader *m_shader;
};
 



#endif //IMAGEPRC_CPU_H


