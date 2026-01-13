 
#include "dispHist.h"
#include <QtGui/QProgressDialog>
 
#include <QtGui>
#include <QPrinter>
 
 
#ifdef USE_QWT_PLOT
////////////////////
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>
#include <qwt_column_symbol.h>
#include <qwt_series_data.h>
////////////////////

class CPlotHistogram: public QwtPlotHistogram
{
public:
    CPlotHistogram( const QString &, const QColor & );

    void setColor( const QColor & );
    void setValues( uint numValues, const int * );
};

CPlotHistogram::CPlotHistogram( const QString &title, const QColor &symbolColor ):
    QwtPlotHistogram( title )
{
    setStyle( QwtPlotHistogram::Columns );

    setColor( symbolColor );
}

void CPlotHistogram::setColor( const QColor &symbolColor )
{
    QColor color = symbolColor;
    color.setAlpha( 180 );

    setPen( QPen( Qt::black ) );
    setBrush( QBrush( color ) );

    QwtColumnSymbol *symbol = new QwtColumnSymbol( QwtColumnSymbol::Box );
    symbol->setFrameStyle( QwtColumnSymbol::Raised );
    symbol->setLineWidth( 2 );
    symbol->setPalette( QPalette( color ) );
    setSymbol( symbol );
}

void CPlotHistogram::setValues( uint numValues, const int *values )
{
    QVector<QwtIntervalSample> samples( numValues );
    for ( uint i = 0; i < numValues; i++ )
    {
        QwtInterval interval( double( i ), i + 1.0 );
        interval.setBorderFlags( QwtInterval::ExcludeMaximum );

        samples[i] = QwtIntervalSample( values[i], interval );
    }

    setData( new QwtIntervalSeriesData( samples ) );
}

#endif

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

#ifdef USE_QWT_PLOT
CDrawLineWidget::CDrawLineWidget(QWidget *parent):QwtPlot(parent)
{
#else
CDrawLineWidget::CDrawLineWidget(QWidget *parent):QWidget(parent)
{
#endif
	m_hist_data = 0;

}
void CDrawLineWidget::paintEvent(QPaintEvent *e)
{
#ifdef USE_QWT_PLOT
	return ;
#endif


//#define marg_x (6)
//#define marg_y (6)
#define draw_marg_x (10)
#define draw_marg_y (20)
 
	if(!m_hist_data) return ;

	 QSize draw_area_size = size();

//	QWidget::paintEvent( e);
	 QPainter p;
 
    p.begin(this);

	p.setViewport(0,draw_area_size.height(),draw_area_size.width(),-draw_area_size.height());

	QPen pen(QColor(128, 162, 223), 1);
	 p.setPen(pen);

	 m_y_gain =  draw_area_size.height()/(10*m_hist_data->m_totalPixels/m_hist_data->m_hist_size);
	 if(m_hist_data->m_validPeak>0){
		  m_y_gain =  (draw_area_size.height()-draw_marg_y)/(m_hist_data->m_validPeak*1.2) ;
	 }
	 
	 m_x_gain = (float)(draw_area_size.width()-draw_marg_x)/m_hist_data->m_hist_size;
	 QPainterPath path;
	 
	 for(int i=0;i<m_hist_data->m_hist_size;i++){
		int pox_x = draw_marg_x + i*m_x_gain;
		path.moveTo(pox_x,draw_marg_y);
		int pos_y = draw_marg_y +m_hist_data->m_hist[i]*m_y_gain;
		path.lineTo(pox_x,pos_y);
	 }
	 p.drawPath(path);

	 
	 
	 QPen pen_bd(QColor(16, 41, 17), 2);
	 p.setPen(pen_bd);
	 p.drawRect(draw_marg_x,draw_marg_y,draw_area_size.width()-draw_marg_x-2,draw_area_size.height()-draw_marg_y-2);

	 //
	 p.setViewport(0,0,draw_area_size.width(),draw_area_size.height());
	
	 int axis_pos_y = draw_area_size.height()-draw_marg_y+12;

	 char _str_buff[256];
	 //pos - 1
	 sprintf(_str_buff,"%d",(int)(0)-m_hist_data->m_huVal);
	 p.drawText(draw_marg_x,axis_pos_y,	_str_buff);
	 //pos - 2
	 sprintf(_str_buff,"%d",(int)(m_hist_data->m_hist_size/2)-m_hist_data->m_huVal);
	 p.drawText(draw_marg_x + m_x_gain*m_hist_data->m_hist_size/2 ,	axis_pos_y,	_str_buff);
	 //pos - 3
	 sprintf(_str_buff,"%d",(int)(m_hist_data->m_hist_size)-m_hist_data->m_huVal);
	 p.drawText(draw_marg_x + m_x_gain*m_hist_data->m_hist_size-30 ,		axis_pos_y,	_str_buff);
}
 
void CDrawLineWidget::mouseMoveEvent(QMouseEvent *e)
{
	QWidget::mouseMoveEvent(e);

	QPoint pos = e->pos();
	int pos_x  = (pos.x()-draw_marg_x)/m_x_gain;

	if(pos_x<0) pos_x = 0;
	if(pos_x>=m_hist_data->m_hist_size) pos_x = m_hist_data->m_hist_size-1;
	
	int pos_y = m_hist_data->m_hist[pos_x];
	m_Owner->onDisplayCursor(pos_x-m_hist_data->m_huVal,pos_y);
	 
}
void CDrawLineWidget::setupHistData(CHistogramData *hist,CDispHist *owner)
{
	this->m_Owner = owner;
	m_hist_data=hist;

	setMouseTracking( true);
/////////////
#ifdef USE_QWT_PLOT
	QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableX( false );
    grid->enableY( true );
    grid->enableXMin( false );
    grid->enableYMin( false );
    grid->setMajPen( QPen( Qt::black, 0, Qt::DotLine ) );
    grid->attach( this );

  
    CPlotHistogram *histogramJune = new CPlotHistogram( "Histogram", Qt::red );
    histogramJune->setValues(
		m_hist_data->m_hist_size, m_hist_data->m_hist );
    histogramJune->attach( this );

 
/////////////
#endif
};
///////////
CDispHist::CDispHist(QWidget *parent) : QDialog(parent)
 
{
		 
	ui.setupUi(this);

  
	 
#if 0
	m_OpGraphwidget = new CGraphWidget(ui.gBDrawArea);
	 
	ui.DrawAreaLayout->removeWidget(ui.gBDrawArea);
	ui.gBDrawArea->hide();

	ui.DrawAreaLayout->insertWidget(0,m_OpGraphwidget);
	dispData();
#else
	m_drawLineWidget = new CDrawLineWidget(ui.gBDrawArea);
	 
	ui.DrawAreaLayout->removeWidget(ui.gBDrawArea);
	ui.gBDrawArea->hide();

	ui.DrawAreaLayout->insertWidget(0,m_drawLineWidget);
	 
#endif
}
 void CDispHist::paintEvent(QPaintEvent *e) 
 {
	 
 }
void CDispHist::dispData()
{
#if 0
	//Op Table
		{
			int op_table_size = 100;
			if(op_table_size<1) return;
			double *dataX = new double[op_table_size];
			double *dataY = new double[op_table_size];
			for(int i=0;i<op_table_size;i++){
			 
				dataX[i] = i;
 
				dataY[i] = i*10.0;
			}
 
			m_OpGraphwidget->init_coord(0,0, op_table_size,op_table_size*10.0);
	 		m_OpGraphwidget->setCurveData(dataX,dataY,op_table_size);
			m_OpGraphwidget->setEditable(false);
			delete [] dataX;
			delete [] dataY;
		}
		//
#endif
}
void CDispHist::setupHistData(CHistogramData *hist )
{
	m_drawLineWidget->setupHistData(hist,this);

	
	//////////////
}
void CDispHist::onDisplayCursor(int posX,int posY)
{
	QString msg;
	msg = QString("(%1,  %2) ").arg(
								QString::number(posX),QString::number(posY) ); 
	ui.lineEdit->setText(msg);
}