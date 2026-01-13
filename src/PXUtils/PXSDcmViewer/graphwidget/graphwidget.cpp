/****************************************************************************
**
**
****************************************************************************/

#include "graphwidget.h"


#include <QDebug>

#include <QWheelEvent>

#include "graphwidget.h"


#include <math.h>

////////////

////////////////////
ShadeWidget::ShadeWidget(ShadeType type, QWidget *parent)
    : QWidget(parent), m_shade_type(type)
 	, m_alpha_gradient(QLinearGradient(0, 0, 0, 0)) 
{
	m_usr0[0] = m_usr0[1] = 0.0;
	m_usr1[0] = m_usr1[1] = 1.0;
	 
	m_usr_coord_k[0] = m_usr_coord_k[1] = 1.0;
	m_usr_coord_b[0] = m_usr_coord_b[1] = 0.0;

	 int m_ColorBar_lut_size;
	 m_ColorBar_lut_node = 0;
	 m_ColorBar_lut_rgb = 0;

	 m_hoverPoints = 0;

	 m_editable = true;
	 m_LockToCenterLine = false;
}
void ShadeWidget::setEditable(bool editable) 
{ 
	m_editable = editable;
	if(m_hoverPoints){
		m_hoverPoints->setEditable(m_editable);
	}
}
void ShadeWidget::setLockToCenterLine(bool lock) 
{ 
	m_LockToCenterLine = lock;
	if(m_hoverPoints){
		m_hoverPoints->setLockToCenterLine(m_LockToCenterLine);
	}
}
 

void ShadeWidget::init()
{
	 // Checkers background
    if (m_shade_type == ARGBShade) {
        QPixmap pm(20, 20);
        QPainter pmp(&pm);
        pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
        pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
        pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
        pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
        pmp.end();
        QPalette pal = palette();
        pal.setBrush(backgroundRole(), QBrush(pm));
        setAutoFillBackground(true);
        setPalette(pal);

    } else {
        setAttribute(Qt::WA_NoBackground);

    }

    QPolygonF points;
    points << QPointF(0, sizeHint().height())
           << QPointF(sizeHint().width(), 0);


	QLutVectorPoints lut_points;

    m_hoverPoints = new HoverPoints(this, HoverPoints::CircleShape);
//     m_hoverPoints->setConnectionType(HoverPoints::LineConnection);
    m_hoverPoints->setPoints(points,lut_points);
    m_hoverPoints->setPointLock(0, HoverPoints::LockToLeft);
    m_hoverPoints->setPointLock(1, HoverPoints::LockToRight);
    m_hoverPoints->setSortType(HoverPoints::XSort);

	m_hoverPoints->setEditable(m_editable);
	m_hoverPoints->setLockToCenterLine(m_LockToCenterLine);
	m_hoverPoints->setCenterLinePos(height()/2.0);

//    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(m_hoverPoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SIGNAL(colorsChanged()));

	SetColorBarLut(64);

	m_update_ColorBar_flag = true;
}
void ShadeWidget::closeEvent(QCloseEvent *)
{
	destroy();
}
void ShadeWidget::destroy()
{
	 

	if(m_hoverPoints){
		delete [] m_hoverPoints;
		m_hoverPoints = 0;
	}

	if(m_ColorBar_lut_node){
		delete [] m_ColorBar_lut_node;
		m_ColorBar_lut_node = 0;
	}
	if(m_ColorBar_lut_rgb){
		delete [] m_ColorBar_lut_rgb;
		m_ColorBar_lut_rgb = 0;
	}
	m_ColorBar_lut_size = 0;
}

 void ShadeWidget::resizeEvent(QResizeEvent *e)
{
	double *dataX = 0;
	double *dataY = 0;
	double *dataRGB=0;
	bool data_saved = false;
	 
	int points_nn = m_hoverPoints->getPointsSize();
	if(points_nn>0){
		dataX = new double[points_nn];
		dataY = new double[points_nn];
		dataRGB = new double[points_nn*3];
		if(getCurveData( dataX, dataY,  points_nn, dataRGB)){
			data_saved = true;
		}
		 
	}
 
	init_coord(m_usr0[0],m_usr0[1], m_usr1[0],m_usr1[1]);

    QWidget::resizeEvent( e);


	if(data_saved){
		
		setCurveData( dataX, dataY,  points_nn,dataRGB) ;
		 
	}

	if(dataX) delete [] dataX;
	if(dataY) delete [] dataY;
	if(dataRGB) delete [] dataRGB;
	
}
#if 0
QPolygonF ShadeWidget::getPoints() const
{
    return m_hoverPoints->getPoints();
}
QLutVectorPoints ShadeWidget::getEditLutPoints() const
{
    return m_hoverPoints->getEditLutPoints();
}
#endif

uint ShadeWidget::colorAt(int x)
{
    generateShade();

    QPolygonF pts;
	QLutVectorPoints lut_points;
	
	m_hoverPoints->getPoints(pts,lut_points);
    for (int i=1; i < pts.size(); ++i) {
        if (pts.at(i-1).x() <= x && pts.at(i).x() >= x) {
            QLineF l(pts.at(i-1), pts.at(i));
            l.setLength(l.length() * ((x - l.x1()) / l.dx()));
            return m_shade.pixel(qRound(qMin(l.x2(), (qreal(m_shade.width() - 1)))),
                                 qRound(qMin(l.y2(), qreal(m_shade.height() - 1))));
        }
    }
    return 0;
}


void ShadeWidget::setGradientStops(const QGradientStops &stops)
{
#if 1
    if (m_shade_type == ARGBShade) {
        m_alpha_gradient = QLinearGradient(0, 0, width(), 0);

        for (int i=0; i<stops.size(); ++i) {
            QColor c = stops.at(i).second;
            m_alpha_gradient.setColorAt(stops.at(i).first, QColor(c.red(), c.green(), c.blue()));
        }

        m_shade = QImage();
        generateShade();
        update();
    }
#endif
}


void ShadeWidget::paintEvent(QPaintEvent *)
{
     generateShade();

    QPainter p(this);
     p.drawImage(0, 0, m_shade);

    p.setPen(QColor(146, 146, 146));
    p.drawRect(0, 0, width() - 1, height() - 1);
}


void ShadeWidget::generateShade()
{
    if (m_shade.isNull() || m_shade.size() != size() || m_update_ColorBar_flag) {

		m_update_ColorBar_flag = false;

        if (m_shade_type == ARGBShade) {
            m_shade = QImage(size(), QImage::Format_ARGB32_Premultiplied);
            m_shade.fill(0);

            QPainter p(&m_shade);
             p.fillRect(rect(), m_alpha_gradient);

            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QLinearGradient fade(0, 0, 0, height());
            fade.setColorAt(0, QColor(0, 0, 0, 255));
            fade.setColorAt(1, QColor(0, 0, 0, 0));
             p.fillRect(rect(), fade);

        } else {
#if 0
            m_shade = QImage(size(), QImage::Format_RGB32);
            QLinearGradient shade(0, 0, 0, height());
            shade.setColorAt(1, Qt::black);

            if (m_shade_type == RedShade)
                shade.setColorAt(0, Qt::red);
            else if (m_shade_type == GreenShade)
                shade.setColorAt(0, Qt::green);
            else
                shade.setColorAt(0, Qt::blue);
 

            QPainter p(&m_shade);
            p.fillRect(rect(), shade);
#else
			

				m_shade = QImage(size(), QImage::Format_RGB32);
				QLinearGradient shade(0, 0, width(), 0);

				if(!m_LockToCenterLine){

				 
					for(int i=0;i<m_ColorBar_lut_size;i++){
						shade.setColorAt( m_ColorBar_lut_node[i], 
									QColor(	m_ColorBar_lut_rgb[3*i + 0]*255,
											m_ColorBar_lut_rgb[3*i + 1]*255,
											m_ColorBar_lut_rgb[3*i + 2]*255));
					}
	          
				}else{
					shade.setColorAt(0,QColor(220,220,207));
					shade.setColorAt(width(),QColor(220,220,207));
					;;
				}
	 
				QPainter p(&m_shade);
				p.fillRect(rect(), shade);
			
#endif
        }
    }


}
void ShadeWidget::init_coord(double usrX0,double usrY0, double usrX1,double usrY1)
{
	m_usr0[0] = usrX0;
	m_usr0[1] = usrY0;
	m_usr1[0] = usrX1;
	m_usr1[1] = usrY1;
	 
	m_usr_coord_k[0] = width()/(m_usr1[0]-m_usr0[0]);
	m_usr_coord_b[0] = -width()*m_usr0[0]/(m_usr1[0]-m_usr0[0]);
	//
	m_usr_coord_k[1] = height()/(m_usr1[1]-m_usr0[1]);
	m_usr_coord_b[1] = -height()*m_usr0[1]/(m_usr1[1]-m_usr0[1]);

}
void ShadeWidget::coord_usr2win(double usr_x, double usr_y, double &pos_x, double &pos_y) const
{
	pos_x	= m_usr_coord_k[0]*usr_x + m_usr_coord_b[0];
	pos_y	= m_usr_coord_k[1]*usr_y + m_usr_coord_b[1];
}
void ShadeWidget::coord_win2usr(double pos_x, double pos_y,double &usr_x, double &usr_y) const
{
	usr_x	= (pos_x-m_usr_coord_b[0])/m_usr_coord_k[0];
	usr_y	= (pos_y- m_usr_coord_b[1])/m_usr_coord_k[1];
}
void ShadeWidget::setCurveData(double *dataX,double *dataY, int size,double *lut_rgb)
{
	int i;
	 QPolygonF points;

	 QLutVectorPoints lut_points;

	 double x,y;
	 double dumy_data;
	 for(i=0;i<size;i++){
		if(m_LockToCenterLine){
			//dataX, dataY —¼•û‹¤‚wŽ²
			coord_usr2win(dataX[i]	,0	,x	,dumy_data);
			coord_usr2win(dataY[i]	,0	,y	,dumy_data);
			 
		}else{
			coord_usr2win(dataX[i],dataY[i],x,y);
		}
		 points << QPointF(x, y) ;

		 //
		if(lut_rgb){
	//		lut_points.insert(i,QColor(lut_rgb[3*i+0],lut_rgb[3*i+1],lut_rgb[3*i+2]));
			QColor color_temp = QColor((int)(lut_rgb[3*i+0]*255.0+0.5),(int)(lut_rgb[3*i+1]*255.0+0.5),(int)(lut_rgb[3*i+2]*255.0+0.5));
			lut_points << color_temp;
		}
	 }

	 
	 

    m_hoverPoints->setPoints(points,lut_points);
	
  //  m_hoverPoints->setPointLock(0, HoverPoints::LockToLeft);
  //  m_hoverPoints->setPointLock(size-1, HoverPoints::LockToRight);
  //  m_hoverPoints->setSortType(HoverPoints::XSort);

	m_hoverPoints->firePointChange();

}
bool ShadeWidget::getCurveData(double *dataX,double *dataY, int size,double *lut_rgb/*len=3*size*/) const
{
	bool lut_rgb_flag = false;
	QPolygonF p_points ;//=  getPoints();
	QLutVectorPoints lut_points;

	m_hoverPoints->getPoints(p_points,lut_points);

	int nn = p_points.count();
	int data_size = size;
	if(nn<data_size) data_size = nn;

	
	if(lut_points.size()>=nn) lut_rgb_flag=true;
	if(lut_rgb==0)  lut_rgb_flag=false;

	double dumy_data;
	for(int i=0;i<nn;i++){
		if(m_LockToCenterLine){
			//dataX, dataY —¼•û‹¤‚wŽ²
			coord_win2usr(p_points[i].rx(),	0, dataX[i], dumy_data);
			coord_win2usr(p_points[i].ry(),	0, dataY[i], dumy_data);
		}else{
			coord_win2usr(p_points[i].rx(),p_points[i].ry(), dataX[i], dataY[i]);
		}

		if(lut_rgb_flag){
			lut_rgb[3*i + 0] = lut_points[i].red()/255.0;
			lut_rgb[3*i + 1] = lut_points[i].green()/255.0;
			lut_rgb[3*i + 2] = lut_points[i].blue()/255.0;
		}
	}
	return true;
}


void ShadeWidget::SetColorBarLut(int size, double *node, double *rgb)
{
	//m_points_editor->SetLutCurve(size,  node,  rgb);
	if(m_ColorBar_lut_node){
		delete [] m_ColorBar_lut_node;
		m_ColorBar_lut_node = 0;
	}
	if(m_ColorBar_lut_rgb){
		delete [] m_ColorBar_lut_rgb;
		m_ColorBar_lut_rgb = 0;
	}
	m_ColorBar_lut_size = 0;

	if(size<1) return ;
	m_ColorBar_lut_size = size;
	m_ColorBar_lut_node	= new double[m_ColorBar_lut_size];
	m_ColorBar_lut_rgb	= new double[3*m_ColorBar_lut_size];

	double dd = 1.0/(m_ColorBar_lut_size-1.0);
	for(int i=0;i<m_ColorBar_lut_size;i++){
		if(node == 0){
			m_ColorBar_lut_node[i] = i*dd ;
		}else{
			m_ColorBar_lut_node[i] = node[i];
		}
		if(rgb == 0 ){
			m_ColorBar_lut_rgb[3*i + 0] = i*dd;
			m_ColorBar_lut_rgb[3*i + 1] = i*dd;
			m_ColorBar_lut_rgb[3*i + 2] = i*dd;
		}else{
			m_ColorBar_lut_rgb[3*i + 0] = rgb[3*i + 0];
			m_ColorBar_lut_rgb[3*i + 1] = rgb[3*i + 1];
			m_ColorBar_lut_rgb[3*i + 2] = rgb[3*i + 2];
		}
	}
	m_update_ColorBar_flag = true;

	update();
}

//////////////////////////

////////////////
CGraphWidget::CGraphWidget(QWidget *parent) :
  QWidget(parent)
{
	//
    m_instanceID = 0;
	QHBoxLayout *layout = new QHBoxLayout;
	layout->setMargin(1);
	setLayout(layout);
 
//	m_points_editor = new ShadeWidget(ShadeWidget::ARGBShade, this);
	m_points_editor = new ShadeWidget(ShadeWidget::GreenShade, this);
	layout->addWidget(m_points_editor);
	m_points_editor->init();

	connect(m_points_editor, SIGNAL(colorsChanged()), this, SLOT(OnGetCurvePoints()));

}

void CGraphWidget::closeEvent(QCloseEvent *)
{
	
	destroy();
}
void CGraphWidget::destroy()
{
	if(m_points_editor){
		delete m_points_editor;
		m_points_editor = 0;
	}
}
void CGraphWidget::setEditable(bool editable)
{
	if(m_points_editor){
		m_points_editor->setEditable( editable)	;
	}
}
void CGraphWidget::setLockToCenterLine(bool lock)
{
	if(m_points_editor){
		m_points_editor->setLockToCenterLine( lock)	;
	}
}

void CGraphWidget::setCurveData(double *dataX,double *dataY, int size,double *lut_rgb )
{
	if(!m_points_editor) return;

	m_points_editor->setCurveData(dataX,dataY,  size,lut_rgb);

}
#if 0
const QPolygonF CGraphWidget::getCurveData() const
{
	return m_points_editor->getPoints();
}
#endif
void CGraphWidget::OnGetCurvePoints()
{
	
	emit curvePointsChanged();

	HoverPoints *hoverPtr = m_points_editor->hoverPoints() ;
	QPolygonF p_points;// =  m_points_editor->getPoints();
	QLutVectorPoints lut_points;

	hoverPtr->getPoints(p_points,lut_points);

	int nn = p_points.count();

	if(0){
		FILE *fp = fopen("curve_points.csv","wt");
		fprintf(fp,"No., x, y \n");
		for(int i=0;i<nn;i++){
			fprintf(fp,"%d, %f, %f \n",i,
				p_points[i].rx(),p_points[i].ry() );
		}
		fclose(fp);
	}
}
void CGraphWidget::init_coord(double usrX0,double usrY0, double usrX1,double usrY1)
{
	m_points_editor->init_coord( usrX0, usrY0,  usrX1, usrY1);
}
int CGraphWidget::getCurveNumber() const
{
//	QPolygonF p_points =  m_points_editor->getPoints();
//	int nn = p_points.count();
	int nn = 0;
	HoverPoints *hoverPtr = m_points_editor->hoverPoints() ;
	if(hoverPtr){
		nn = hoverPtr->getPointsSize();
	}

	return nn;
}
bool CGraphWidget::getCurveData(double *dataX,double *dataY, int size,double *lut_rgb/*len=3*size*/) const
{
	return m_points_editor->getCurveData( dataX, dataY, size,lut_rgb);
	;
}
 
void CGraphWidget::SetColorBarLut(int size, double *node, double *rgb)
{
	m_points_editor->SetColorBarLut(size,  node,  rgb);
}