#ifndef IMAGEPRC_BASE_H
#define IMAGEPRC_BASE_H
 
#include "math.h"

#include "string"
#include "map"
namespace DefGaussian
{
#ifndef M_PI
#define M_PI	 	3.1415926535897932384626433832795028842
#endif
static float calSigma(float FWHM)
	{
		float sigma = FWHM/2.3548f;
		return sigma;
	}
	static float calFWHW(float sigma)
	{
		float FWHM = sigma*2.3548f;
		return FWHM;
	}
	static int calSizeFromFWHM(float FWHM)
	{
		int size = (int)(FWHM*3.0f+0.5f);
		if(size >0){
			if(size%2 ==0){
				size++; //äÔêîÇ…Ç∑ÇÈ
			}
		}
		return size;
	}
	template<class T> static  void Gaussian(T *g_data,int size, float sigma=10.0f,int gw_center=-1,bool powerNormal=false) 
	{
		//FWHM(Full Width at Half Maximum) = 2*qsqrt(2*ln(2))* sigma = 2.3548*sigma
//		double  delta = g;
		int i;
		double gg=1.0/sqrt(2.0*M_PI)/sigma;


		double xdd  = 1.0/2.0/sigma/sigma;

		double xx;

		double sum_weight = 0.0;
		double max_v = 0.0;
		double center=(size-1)/2.0;
		if(gw_center >0){
			center = gw_center;
		}
 		for(i=0;i<size;i++){
			xx = (i-center);
			double exp_val = exp(xdd*xx*xx);
	//		g_data[i] =  gg/exp_val;

			g_data[i] =  (T)(1.0/exp_val);

			sum_weight += g_data[i];	//for debug test

			if(g_data[i] > max_v) max_v = g_data[i];
		}
		double normal = 1.0/max_v;
		if(powerNormal){
			normal = 1.0/sum_weight;
		} 
		for(i=0;i<size;i++){
			g_data[i] =  (T)(g_data[i]*normal) ;
		}
		 
//		printf(" %f \n",sum_weight);
	}
}

class CImageFilter
{
public:
	 
	CImageFilter(){
		m_sizeX = 0;
		m_sizeY = 0;
		m_imageIn =0;
		m_imageOut=0;
		m_pLut = 0;
		m_LutSize = 0;
	};
	~CImageFilter(){};

	virtual bool doFilter() = 0;
	virtual void setupProcImage(int sizeX,int sizeY,const float *in,float*out){
		m_sizeX = sizeX;
		m_sizeY = sizeY;
		m_imageIn = in;
		m_imageOut=out;
		};
		virtual bool loadFilter() ;
		virtual bool activeFilter() {return false;};
		
		void setupFilterShader(const std::string &VertString ,const std::string &FragString )
		{
			m_VertString = VertString;
			m_FragString = FragString;
		}

		virtual void setupLut(float *lut,int lutSize){
			m_pLut = lut;
			m_LutSize = lutSize;
		}
	virtual void setWWWL(float ww,float wl,float dispMax) {
		m_ww = ww;m_wl=wl;m_displayPixelMax=dispMax;
	};
	virtual void calPixelGainOffset(){
		 
 
		float cal_k = 1.0f/m_ww;
		float cal_b = m_wl-m_ww/2.0f ;

		m_pixelGain = cal_k*m_displayPixelMax;
		m_pixelOffset = -cal_k*cal_b;

	};
protected:
	virtual std::string getFilterPath() const { return std::string(":/openGLShader/openGLShader/");};
	int m_sizeX;
	int m_sizeY;
	const float *m_imageIn;
	float *m_imageOut;
	//
	float *m_pLut;
	int m_LutSize;
	///
	float m_pixelGain ; 
	float m_pixelOffset ; 
	////

	std::string m_VertFileName;
	std::string m_FragFileName;

	std::string m_VertString;
	std::string m_FragString;
	///
	float	m_ww;
	float	m_wl;
	float   m_displayPixelMax;
};
 
 
class CImageFilterNull : public  CImageFilter
{
public:
	CImageFilterNull() ;
	~CImageFilterNull() ;
	virtual bool doFilter() = 0;
protected:
	 
};


class CImageFirFilter : public  CImageFilter
{
public:
	CImageFirFilter() ;
	~CImageFirFilter() ;
	virtual bool doFilter() = 0;
protected:
	int m_filerNN;
	float *m_GaussianW;
	float m_Sigma;

	float *m_coef;
	int m_coefW;
	int m_coefH;
};
////
class CImageBiLateralFilter : public  CImageFilter
{
public:
	CImageBiLateralFilter();
	~CImageBiLateralFilter();
	virtual bool doFilter() = 0;
protected:
	int m_filerNN;
	float *m_GaussianW;
	float m_Sigma;
	//
	float *m_coef;
	int m_coefW;
	int m_coefH;
};

class CImageProcBase   
{
	 
public:
	enum ImageFilterType{
		ImageFilter_None,
		ImageFilter_FIR,
		ImageFilter_BiLateral,
	};
    CImageProcBase();
	~CImageProcBase();
	virtual bool isShader() { return false;};
	
	virtual bool createFilter(const std::string &name, int type)=0;
	virtual void selCurrentFilter(const std::string &name){ m_curFilerName = name;} ;

	virtual void setWWWL(float ww,float wl,float dispMax) { 
		m_ww = ww;m_wl=wl;m_displayPixelMax=dispMax;
	};
	CImageFilter * getCurrentFilter();

	
protected:
	void addFilter(const std::string &name, CImageFilter *file);
	CImageFilter * getFilter(const std::string &name );

	std::string m_curFilerName;
	std::map<std::string, CImageFilter *> m_filterMap;

	float	m_ww;
	float	m_wl;
	float   m_displayPixelMax;
};
 



#endif //IMAGEPRC_BASE_H


