 

#include "imageProcBase.h"

 
 
CImageProcBase::CImageProcBase( )
{
	m_filterMap.clear();
	m_ww = 4096.0f - 1024.0f;
	m_wl = m_ww/2.0f;
	m_displayPixelMax=m_ww;  
}
CImageProcBase::~CImageProcBase()
{
	 
 
}

void CImageProcBase::addFilter(const std::string &name, CImageFilter *filter)
{
	m_filterMap[name] = filter;
}

CImageFilter * CImageProcBase::getFilter(const std::string &name )
{
	CImageFilter *ret_ptr = 0;
	std::map<std::string, CImageFilter *>::iterator it = m_filterMap.find(name);//();
	if(it!=m_filterMap.end()){
		 
		ret_ptr = it->second;
		 
	}
//	return m_filterMap[name];
	return ret_ptr;
}
CImageFilter * CImageProcBase::getCurrentFilter()
{
	return  getFilter(m_curFilerName);
}
/////////
CImageFilterNull::CImageFilterNull()
{
}
CImageFilterNull::~CImageFilterNull()
{
}

/////////
CImageFirFilter::CImageFirFilter()
{
	
	m_coefW = 5 ;
	m_coefH= 5 ;
	m_coef = new float[m_coefW*m_coefH];

	m_filerNN = 5;
	m_GaussianW = new float[m_filerNN];
	m_Sigma = DefGaussian::calSigma(2.4);
	DefGaussian::Gaussian(m_GaussianW,m_filerNN,  m_Sigma, -1/*gw_center*/,true /*powerNormal*/) ;
	
	for(int j=0;j<m_coefH;j++){
		float h_d = m_GaussianW[j];
		for(int i=0;i<m_coefW;i++){
			float f_v = m_GaussianW[i]*h_d;
			m_coef[j*m_coefW + i] = f_v;
			 
		}
	}
}
CImageFirFilter::~CImageFirFilter()
{
	delete [] m_coef;
	delete [] m_GaussianW;
}

CImageBiLateralFilter::CImageBiLateralFilter()
{
	float FWHM = 2.4f;
	m_filerNN = DefGaussian::calSizeFromFWHM(FWHM);//5;
	m_coefW = m_filerNN ;
	m_coefH= m_filerNN ;
	m_coef = new float[m_coefW*m_coefH];

	m_GaussianW = new float[m_filerNN];
	m_Sigma = DefGaussian::calSigma(FWHM);
	DefGaussian::Gaussian(m_GaussianW,m_filerNN,  m_Sigma, -1/*gw_center*/,true /*powerNormal*/) ;
	
	for(int j=0;j<m_coefH;j++){
		float h_d = m_GaussianW[j];
		for(int i=0;i<m_coefW;i++){
			float f_v = m_GaussianW[i]*h_d;
			m_coef[j*m_coefW + i] = f_v;
		 
		}
	}
}
CImageBiLateralFilter::~CImageBiLateralFilter()
{
	delete [] m_coef;
	delete [] m_GaussianW;
}