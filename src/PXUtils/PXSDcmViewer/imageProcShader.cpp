
#include "imageProcShader.h"
 
#include <gl/glew.h>
#include <gl/glut.h>
#include "OpenGLShader.h"
 
 
bool CImageFilter::loadFilter()
{
	return false;
};

CImageFilterNullShader::CImageFilterNullShader(COpenGLShader *shader):
m_ShaderLoaded(false)
,m_histogram(0)
{
	m_shader=shader;

	m_VertFileName = "imageView.vert";
	m_FragFileName = "imageView.frag";

}
CImageFilterNullShader::~CImageFilterNullShader()
{
	if(m_histogram){
		delete [] m_histogram;
	}
}
 
bool CImageFilterNullShader::loadFilter()
{
	if(m_ShaderLoaded) return false;
	if(m_shader){
		if(m_VertString.size()>0){
			if(!m_shader->createVertShaderFromString(m_VertString,m_VerShardID)){
				return false;
			}
		}else{
			if(!m_shader->createVertShaderFromFile(m_VertFileName,m_VerShardID)){
				return false;
			}
		}
		if(m_FragString.size()>0){
			if(!m_shader->createFragShaderFromString(m_FragString,m_FragShardID)){
				return false;
			}
		}else{
			if(!m_shader->createFragShaderFromFile(m_FragFileName,m_FragShardID)){
				return false;
			}
		}

		m_ShaderLoaded = true;

		return true;
	}else{
		return false;
	}

	return true;
}

bool CImageFilterNullShader::activeFilter()
{
#ifndef USE_SHARDER_LUT
//changed LUT
	m_shader->SetUniform1f("gain", m_pixelGain); 
	m_shader->SetUniform1f("offset", m_pixelOffset); 

#endif

	return true;
}

bool CImageFilterNullShader::doFilter() 
{

#ifndef USE_SHARDER_LUT
//changed LUT	 
	 
	calPixelGainOffset();
	 
#endif

	if(m_shader){
		 
		return m_shader->setupShader(m_VerShardID,m_FragShardID);
 
	}else{
		return false;
	}

	return true;
}



	////
CImageFirFilterShader::CImageFirFilterShader(COpenGLShader *shader):
m_ShaderLoaded(false)
{
	 
	m_shader=shader;
	//
	m_VertFileName = "imageFilter.vert";
	m_FragFileName = "imageFilter.frag";
};
CImageFirFilterShader::~CImageFirFilterShader()
{
};
bool CImageFirFilterShader::loadFilter()
{
//	GLuint texture;
//	m_shader->createTexture1D(m_GaussianW,m_filerNN, texture);
//	m_GaussianTex = texture;

	if(m_ShaderLoaded) return false;
	if(m_shader){
		 

		if(m_VertString.size()>0){
			if(!m_shader->createVertShaderFromString(m_VertString,m_VerShardID)){
				return false;
			}
		}else{
			if(!m_shader->createVertShaderFromFile(m_VertFileName,m_VerShardID)){
				return false;
			}
		}
		if(m_FragString.size()>0){
			if(!m_shader->createFragShaderFromString(m_FragString,m_FragShardID)){
				return false;
			}
		}else{
			if(!m_shader->createFragShaderFromFile(m_FragFileName,m_FragShardID)){
				return false;
			}
		}

		m_ShaderLoaded = true;

		return true;
	}else{
		return false;
	}

}
bool  CImageFirFilterShader::activeFilter()
{
	 
#ifndef USE_SHARDER_LUT
//changed LUT
	m_shader->SetUniform1f("gain", m_pixelGain); 
	m_shader->SetUniform1f("offset", m_pixelOffset); 
#endif
 
//	m_shader->bindTexture(2,m_GaussianTex,GL_TEXTURE_1D);
//	m_shader->SetUniform1i("GaussianTex",2); 

	m_shader->SetUniform1i("gw_X", m_filerNN); 
	m_shader->SetUniform1i("gw_y", m_filerNN); 

	m_shader->SetUniform1fv( "filterCoef", m_filerNN, m_GaussianW );
 
 
	return true;
		 
}
	/////////
bool CImageFirFilterShader::doFilter ()
{
#ifndef USE_SHARDER_LUT
//changed LUT	 
	 
	calPixelGainOffset();
	 
#endif
	if(m_shader){
		 
		return m_shader->setupShader(m_VerShardID,m_FragShardID);
 
	}else{
		return false;
	}

}

////////////////////////////
CImageBiLateralFilterShader::CImageBiLateralFilterShader(COpenGLShader *shader):
m_ShaderLoaded(false)
{
	 
	m_shader=shader;
	m_VertFileName = "imageFilterBiLateral.vert";
	m_FragFileName = "imageFilterBiLateral.frag";

};
CImageBiLateralFilterShader::~CImageBiLateralFilterShader()
{
};
bool CImageBiLateralFilterShader::loadFilter()
{
	 
	//
	if(m_ShaderLoaded) return false;
	if(m_shader){
		if(m_VertString.size()>0){
			if(!m_shader->createVertShaderFromString(m_VertString,m_VerShardID)){
				return false;
			}
		}else{
			if(!m_shader->createVertShaderFromFile(m_VertFileName,m_VerShardID)){
				return false;
			}
		}
		if(m_FragString.size()>0){
			if(!m_shader->createFragShaderFromString(m_FragString,m_FragShardID)){
				return false;
			}
		}else{
			if(!m_shader->createFragShaderFromFile(m_FragFileName,m_FragShardID)){
				return false;
			}
		}
		m_ShaderLoaded = true;

		return true;
	}else{
		return false;
	}

	 
}
bool  CImageBiLateralFilterShader::activeFilter()
{

#ifndef USE_SHARDER_LUT
//changed LUT
	m_shader->SetUniform1f("gain", m_pixelGain); 
	m_shader->SetUniform1f("offset", m_pixelOffset); 
#endif

	m_shader->SetUniform1i("gw_X", m_filerNN); 
	m_shader->SetUniform1i("gw_y", m_filerNN); 

	m_shader->SetUniform1f("sigma", 0.15);//m_Sigma); 

	m_shader->SetUniform1fv( "filterCoef", m_filerNN, m_GaussianW );
 
	return true;
}
bool CImageBiLateralFilterShader::doFilter ()
{
#ifndef USE_SHARDER_LUT
//changed LUT	 
	 
	calPixelGainOffset();
	 
#endif

	if(m_shader){
		 
		return m_shader->setupShader(m_VerShardID,m_FragShardID);
 
	}else{
		return false;
	}
}

////////////////////////////
CImageProcShader::CImageProcShader( COpenGLShader *shader)
{
	m_shader=shader;  
}
CImageProcShader::~CImageProcShader()
{
	 

	 
}

#include "mainwindow.h" 
bool CImageProcShader::createFilter(const std::string &name, int filterType)
{
	CImageFilter *new_filter = 0;
	switch(filterType){
		case ImageFilter_None:
			{
				CImageFilterNullShader *my_filter = new CImageFilterNullShader(m_shader);
				 
 
				std::string VertString = MainWindow::read_shader_file("imageView.vert");
				std::string FragString = MainWindow::read_shader_file("imageView.frag");
 

			 	my_filter->setupFilterShader(VertString,FragString);
				if(!my_filter->loadFilter()){
					return false;
				}
				new_filter = my_filter;
			}
			break;
			
		case ImageFilter_FIR:
			{
				CImageFirFilterShader *my_filter = new CImageFirFilterShader(m_shader);

				std::string VertString = MainWindow::read_shader_file("imageFilter.vert");
				std::string FragString = MainWindow::read_shader_file("imageFilter.frag");
	 	  	 	my_filter->setupFilterShader(VertString,FragString);

				if(!my_filter->loadFilter()){
					return false;
				}
				new_filter = my_filter;
			}
			break;
		case ImageFilter_BiLateral:
			{
				CImageBiLateralFilterShader *my_filter = new CImageBiLateralFilterShader(m_shader);

				std::string VertString = MainWindow::read_shader_file("imageFilterBiLateral.vert");
				std::string FragString = MainWindow::read_shader_file("imageFilterBiLateral.frag");
		 		my_filter->setupFilterShader(VertString,FragString);

				if(!my_filter->loadFilter()){
					return false;
				}
				new_filter = my_filter;
			}
			break;
	}
	
	addFilter(name,new_filter);

	return true;
}

void CImageProcShader::selCurrentFilter(const std::string &name) 
{
	CImageProcBase::selCurrentFilter( name);
}
