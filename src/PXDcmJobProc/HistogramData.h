#ifndef DISP_HIST_DATA_H
#define DISP_HIST_DATA_H

 
class CHistogramData
{
public:
	CHistogramData();
	~CHistogramData();
	void createHist(int size);
	void findValidPeak();
	void findRange(int &min,int &max);

	int m_totalPixels;
	int m_hist_size;
	int *m_hist;
	//
	int m_validPeak;
	//
	int m_huVal;
};
 
#endif //DISP_HIST_DATA_H


