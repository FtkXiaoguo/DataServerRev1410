#ifndef DISP_HIST_H
#define DISP_HIST_H

#include "ui_dispHist.h"
 
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

class CDispHist;
#ifdef USE_QWT_PLOT
#include <qwt_plot.h>

class CDrawLineWidget  : public QwtPlot
{
#else
class CDrawLineWidget  : public QWidget
{
#endif
  Q_OBJECT
public:
    CDrawLineWidget(QWidget *parent);

	void setupHistData(CHistogramData *hist,CDispHist *owner);
	protected:
	virtual void mouseMoveEvent(QMouseEvent *);
	void paintEvent(QPaintEvent *e); 
	CHistogramData *m_hist_data;

	CDispHist *m_Owner;

	float m_y_gain;
	float m_x_gain;
};

class CGraphWidget;
class CDispHist : public QDialog {
	Q_OBJECT
	public:
		CDispHist(QWidget *parent = 0);
		void setupHistData(CHistogramData *hist);

		void onDisplayCursor(int posX,int posY);
	private:
		Ui::dispHist ui;
	public slots:
	 
		 
protected:
	 
	void paintEvent(QPaintEvent *e);
	void dispData();
	 CGraphWidget *m_OpGraphwidget;
	 //
	 CDrawLineWidget *m_drawLineWidget;
	 
private:
	 
	 
};

#endif //DISP_HIST_H


