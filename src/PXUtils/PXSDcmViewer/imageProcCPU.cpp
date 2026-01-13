 

#include "imageProcCPU.h"
 
#include "math.h"


 
CImageFilterNullCPU::CImageFilterNullCPU() 
{
}
CImageFilterNullCPU::~CImageFilterNullCPU()
{
}

bool CImageFilterNullCPU::doFilter()
{
	int proc_size = m_sizeX*m_sizeY;

#if 0
	if( m_pLut!=0){
		for(int i=0;i<proc_size;i++){
			int lut_index = m_imageIn[i]*m_LutSize;
			lut_index = (lut_index+m_LutSize)%m_LutSize;
			 
			m_imageOut[i] = m_pLut[lut_index];
		}
	}else{

		for(int i=0;i<proc_size;i++){
			m_imageOut[i] = m_imageIn[i];
		}
	}
#else
	 
	calPixelGainOffset();

	for(int i=0;i<proc_size;i++){
		float lut_val =  m_pixelGain*m_imageIn[i] + m_pixelOffset;

		if(lut_val<0.0f) lut_val = 0.0f;
		if(lut_val>0.99999999999f) lut_val = 0.99999999999f;

		m_imageOut[i] = lut_val;

	}
#endif
	 

	return true;
}

 
CImageFirFilterCPU::CImageFirFilterCPU()
{
}
CImageFirFilterCPU::~CImageFirFilterCPU()
{
}


bool CImageFirFilterCPU::doFilter ()
{
	for(int y_i=0;y_i<m_sizeY;y_i++){
		int index_y = y_i*m_sizeX;
		for(int x_i=0;x_i<m_sizeX;x_i++){
			
			float sum = 0.0f;
			for(int j=0;j<m_coefH;j++){
				int w_index_y = ((y_i+j-m_coefH/2 + m_sizeY )%m_sizeY) * m_sizeX;
				for(int i=0;i<m_coefW;i++){
					int w_index_x = (x_i+i-m_coefW/2 + m_sizeX )%m_sizeX;
					 
					sum += m_coef[j*m_coefW + i] *  m_imageIn[w_index_y  + w_index_x];
				}
			}
			//
			m_imageOut[index_y + x_i] = sum;
		}
	}
	return true;
}
//////////////
 
CImageBiLateralFilterCPU::CImageBiLateralFilterCPU()
{
}

CImageBiLateralFilterCPU::~CImageBiLateralFilterCPU()
{
}
float heuclideanLen(float pix  , float cc, float d)
{
    float mod = (pix - cc) ;
	mod = mod*mod;

	return exp(-mod / (2 * d * d));
}
bool CImageBiLateralFilterCPU::doFilter()
{
	float pix_sigma = 0.15f;//DefGaussian::calSigma(2.3/255.0);
	for(int y_i=0;y_i<m_sizeY;y_i++){
		int index_y = y_i*m_sizeX;
		for(int x_i=0;x_i<m_sizeX;x_i++){
			
			float sum = 0.0f;

			float center_v = m_imageIn[index_y + x_i];
			for(int j=0;j<m_coefH;j++){
				int w_index_y = ((y_i+j-m_coefH/2 + m_sizeY )%m_sizeY) * m_sizeX;
				for(int i=0;i<m_coefW;i++){
					int w_index_x = (x_i+i-m_coefW/2 + m_sizeX )%m_sizeX;

					float cur_pix = m_imageIn[w_index_y  + w_index_x];
					float bi_v = heuclideanLen(cur_pix ,center_v,pix_sigma);
					sum += m_coef[j*m_coefW + i] * cur_pix *bi_v;
				}
			}
			//
			m_imageOut[index_y + x_i] = sum;
		}
	}
	return true;
}

//////////////
CImageProcCPU::CImageProcCPU( )
{
	  
}
CImageProcCPU::~CImageProcCPU()
{
	 

	 
}


bool CImageProcCPU::createFilter(const std::string &name, int filterType)
{
	CImageFilter *new_filter = 0;
	switch(filterType){
		case ImageFilter_None:
			new_filter = new CImageFilterNullCPU;
			break;
		case ImageFilter_FIR:
			new_filter = new CImageFirFilterCPU;
			break;
		case ImageFilter_BiLateral:
			new_filter = new CImageBiLateralFilterCPU;
			break;
	}
	
	addFilter(name,new_filter);

	return true;
}

void CImageProcCPU::selCurrentFilter(const std::string &name) 
{
	CImageProcBase::selCurrentFilter( name);
}
	