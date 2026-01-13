 
#include "string.h"

#include "HistogramData.h"
 
 
 
///////////////////////
CHistogramData::CHistogramData()
{
	m_huVal = 0;
	m_totalPixels = 0;
	m_hist_size = 0;
	m_hist= 0;
}
CHistogramData::~CHistogramData()
{
	if(m_hist) delete [] m_hist;
}
void CHistogramData::findValidPeak()
{
	m_validPeak = 0;
	for(int i=1;i<(m_hist_size-1);i++){
		if(m_hist[i] > m_validPeak){
			m_validPeak = m_hist[i];
		}
	}
}
void CHistogramData::findRange(int &min,int &max)
{
	min = 0;
	max = m_hist_size-1;
#if 0
	int th_level_hi = m_validPeak * 0.02;
	int th_level_low = m_validPeak * 0.06;
#else

#if 0
	float ss = (float)m_totalPixels/m_hist_size;
	int th_level_hi =  ss* 0.1;
	int th_level_low = ss* 0.1;
#else
	 
	int th_level_hi =  m_totalPixels* 0.001;
	int th_level_low = m_totalPixels* 0.001;
#endif

	if(th_level_hi < 10){
		th_level_hi = 10;
	}
	if(th_level_low < 10){
		th_level_low = 10;
	}


#endif
	{
		float sum_pixel = 0.0f;
		for(int i=1;i<(m_hist_size-1);i++){
			sum_pixel = sum_pixel + m_hist[i];
			if(sum_pixel > th_level_low){
				min = i-1;
				break;
			}
		}
		 
	}
	{
		float sum_pixel = 0.0f;
		for(int i=m_hist_size-2;i>1;i--){
			sum_pixel = sum_pixel + m_hist[i];
			if(sum_pixel > th_level_hi){
				max = i+1;
				break;
			}
		}
	}
	if(min>max){
		int i_temp = max;
		max = min;
		min = i_temp;
	}
	if((max-min)<3){
		min--;
		max++;
	}
}
void CHistogramData::createHist(int size)
{
	if(m_hist_size != size){
		if(m_hist) delete [] m_hist;

		m_hist_size =size;
		m_hist = new int[m_hist_size];
	}
	m_totalPixels = 0;
 
	memset(m_hist,0,sizeof(int)*m_hist_size);
	 
	m_validPeak = 0;

	
}
 