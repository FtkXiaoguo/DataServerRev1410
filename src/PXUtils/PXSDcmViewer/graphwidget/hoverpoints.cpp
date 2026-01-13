/****************************************************************************
**
**
****************************************************************************/

#ifdef QT_OPENGL_SUPPORT
#include <QGLWidget>
#endif

//#include "arthurwidgets.h"
#include "hoverpoints.h"

#define printf

HoverPoints::HoverPoints(QWidget *widget, PointShape shape)
    : QObject(widget)
{
    m_widget = widget;
    widget->installEventFilter(this);

    m_connectionType = CurveConnection;
    m_sortType = NoSort;
    m_shape = shape;
    m_pointPen = QPen(QColor(255, 255, 255, 191), 1);
    m_connectionPen = QPen(QColor(255, 255, 255, 127), 2);
    m_pointBrush = QBrush(QColor(191, 191, 191, 127));
    m_pointDrawSize = QSize(9, 9);
    m_currentIndex = -1;
    m_editable = true;
	m_fixPointsNum = false;
    m_enabled = true;

	m_editMode = CurveEditMode;
	m_CenterLinePos = 0;
	m_currentEditIndex = -1;
	m_currentEditWWIndex = -1;

    connect(this, SIGNAL(pointsChanged(const QPolygonF &)),
            m_widget, SLOT(update()));
}


void HoverPoints::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_widget->update();
    }
}


bool HoverPoints::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_widget && m_enabled) {
        switch (event->type()) {

		case QEvent::MouseButtonDblClick:
		{
			QMouseEvent *me = (QMouseEvent *) event;

            QPointF clickPos = me->pos();
            int index = hitPoints(clickPos);

			//
			 if (me->button() == Qt::LeftButton) {
				m_curTime_mouse_press = QTime::currentTime ()   ;
                if (index == -1) {
      //              if (!m_editable)
					if (m_fixPointsNum)
                        return false;
                    int pos = 0;
                    // Insert sort for x or y
                    if (m_sortType == XSort) {
                        for (int i=0; i<m_EditPoints.size(); ++i)
                            if (m_EditPoints.at(i).x() > clickPos.x()) {
                                pos = i;
                                break;
                            }
                    } else if (m_sortType == YSort) {
                        for (int i=0; i<m_EditPoints.size(); ++i)
                            if (m_EditPoints.at(i).y() > clickPos.y()) {
                                pos = i;
                                break;
                            }
                    }

					if(m_editMode == CenterLineMode){
						QPointF new_point= clickPos;
						new_point.setY(10);
						m_WWPoints.insert(pos,clickPos);
						//
						m_LutPoints.insert(pos,QColor(100,100,100));
					};
				 
					
					checkLockCenterLine(clickPos);
                    m_EditPoints.insert(pos, clickPos);
                    m_locks.insert(pos, 0);
                    m_currentIndex = pos;
                    firePointChange();
                } else {
					
             //       m_currentIndex = index;
					 
					m_currentEditIndex = index;
					 
					editPointsProperty();
					firePointChange();
					 

                }
                return true;

            } else if (me->button() == Qt::RightButton) {
  //              if (index >= 0 && m_editable) {
				if (index >= 0 && (!m_fixPointsNum)) {
                    if (m_locks[index] == 0) {
                        m_locks.remove(index);
                        m_EditPoints.remove(index);
						if(m_WWPoints.size()>=index){
							m_WWPoints.remove(index);
						};
						if(m_LutPoints.size()>=index){
							m_LutPoints.remove(index);
						};

                    }
                    firePointChange();
                    return true;
                }
            }
		}
			break;
        case QEvent::MouseButtonPress:
        {
            QMouseEvent *me = (QMouseEvent *) event;

            QPointF clickPos = me->pos();
            int index = hitPoints(clickPos);

            if (me->button() == Qt::LeftButton) {
				m_curTime_mouse_press = QTime::currentTime ()   ;
 
                if (index == -1) {

                } else {
                    m_currentIndex = index;
                }
                return true;

            } else if (me->button() == Qt::RightButton) {

				if (index == -1) {

                } else {
                    m_currentEditWWIndex = index;
					m_currentEditWWPos  = clickPos;
					if((m_currentEditWWIndex>=0) && (m_editMode == CenterLineMode)){
						m_oldEditWWPoint = m_WWPoints.at(m_currentEditWWIndex);
					}
					 

                }
            }

        }
        break;

        case QEvent::MouseButtonRelease:
			{
			
				if(m_editMode == CenterLineMode){

					QTime mouse_release_time  = QTime::currentTime () ;
					int time1 = m_curTime_mouse_press.minute()*60 + m_curTime_mouse_press.second();
					int time2 = mouse_release_time.minute()*60 + mouse_release_time.second();
					int time_span = time2-time1;
					if(time_span>1){
						m_currentEditIndex = m_currentIndex;
		//				editPointsProperty();
					}
				}
				m_currentIndex = -1;
				m_currentEditWWIndex = -1;
				 
			}
            break;

        case QEvent::MouseMove:
			if (m_currentIndex >= 0){
                movePoint(m_currentIndex, ((QMouseEvent *)event)->pos());
				m_curTime_mouse_press = QTime::currentTime () ;
			}
			if((m_currentEditWWIndex>=0) && (m_editMode == CenterLineMode)){
					QMouseEvent *me = (QMouseEvent *) event;
					QPointF clickPos = me->pos();
					QPointF diff = clickPos-m_currentEditWWPos;
 
					double rate =  diff.ry();;//10.0;

					QPointF point = m_oldEditWWPoint;//m_WWPoints.at(m_currentEditWWIndex);
					double new_ww = point.ry()+rate;//*(1.0+rate);
					if(new_ww <1.0) new_ww = 1.0;
					point.setY(new_ww);
					m_WWPoints[m_currentEditWWIndex] = point;

					firePointChange();
						
			}
            break;

        case QEvent::Resize:
        {
#if 0
            QResizeEvent *e = (QResizeEvent *) event;
            if (e->oldSize().width() == 0 || e->oldSize().height() == 0)
                break;
            qreal stretch_x = e->size().width() / qreal(e->oldSize().width());
            qreal stretch_y = e->size().height() / qreal(e->oldSize().height());
            for (int i=0; i<m_points.size(); ++i) {
                QPointF p = m_points[i];
                movePoint(i, QPointF(p.x() * stretch_x, p.y() * stretch_y), false);
            }

            firePointChange();
#else
		//ŠO‘¤ˆ—‚ÉˆÚs
#endif
            break;
        }

        case QEvent::Paint:
        {
            QWidget *that_widget = m_widget;
            m_widget = 0;
            QApplication::sendEvent(object, event);
            m_widget = that_widget;
            paintPoints();
#ifdef QT_OPENGL_SUPPORT
            ArthurFrame *af = qobject_cast<ArthurFrame *>(that_widget);
            if (af && af->usesOpenGL())
                af->glWidget()->swapBuffers();
#endif
            return true;
        }
		
        default:
            break;
        }
    }

    return false;
}


void HoverPoints::paintPoints()
{
    QPainter p;
#ifdef QT_OPENGL_SUPPORT
    ArthurFrame *af = qobject_cast<ArthurFrame *>(m_widget);
    if (af && af->usesOpenGL())
        p.begin(af->glWidget());
    else
        p.begin(m_widget);
#else
    p.begin(m_widget);
#endif

    p.setRenderHint(QPainter::Antialiasing);

    if (m_connectionPen.style() != Qt::NoPen && m_connectionType != NoConnection) {
        p.setPen(m_connectionPen);

		if(m_EditPoints.size()>1){
			if (m_connectionType == CurveConnection) {
				QPainterPath path;
				path.moveTo(m_EditPoints.at(0));
				for (int i=1; i<m_EditPoints.size(); ++i) {
					QPointF p1 = m_EditPoints.at(i-1);
					QPointF p2 = m_EditPoints.at(i);
					qreal distance = p2.x() - p1.x();

					path.cubicTo(p1.x() + distance / 2, p1.y(),
								 p1.x() + distance / 2, p2.y(),
								 p2.x(), p2.y());
				}
				p.drawPath(path);
			} else {
				p.drawPolyline(m_EditPoints);
			}
		}
    }

    p.setPen(m_pointPen);
    p.setBrush(m_pointBrush);

 
	QColor lutColor;
    for (int i=0; i<m_EditPoints.size(); ++i) {
        QRectF bounds = pointBoundingRect(i);
		if(getLutPointColor(i,lutColor)){
//			lutColor = QColor(100,0,100);
	
			p.setPen(lutColor);
 			p.setBrush(QBrush(lutColor));
		}
        if (m_shape == CircleShape)
             p.drawEllipse(bounds);
        else
             p.drawRect(bounds);
    }
 

}

static QPointF bound_point(const QPointF &point, const QRectF &bounds, int lock)
{
    QPointF p = point;

    qreal left = bounds.left();
    qreal right = bounds.right();
    qreal top = bounds.top();
    qreal bottom = bounds.bottom();

    if (p.x() < left || (lock & HoverPoints::LockToLeft)) p.setX(left);
    else if (p.x() > right || (lock & HoverPoints::LockToRight)) p.setX(right);

    if (p.y() < top || (lock & HoverPoints::LockToTop)) p.setY(top);
    else if (p.y() > bottom || (lock & HoverPoints::LockToBottom)) p.setY(bottom);

    return p;
}

void HoverPoints::setPoints(const QPolygonF &points,const QLutVectorPoints &lut_poits)
{
	int i;
    m_EditPoints.clear();
	m_WWPoints.clear();
	for (  i=0; i<points.size(); ++i){
		QPointF p_temp_org = points.at(i);
		
		QPointF p_temp_temp = p_temp_org;
		if(checkLockCenterLine(p_temp_temp)){
			m_WWPoints <<p_temp_org;
		}
        m_EditPoints << bound_point(p_temp_temp, boundingRect(), 0);
		//
		
	}

    m_locks.clear();
    if (m_EditPoints.size() > 0) {
        m_locks.resize(m_EditPoints.size());

        m_locks.fill(0);
    }
//	if(m_editMode == CenterLineMode){
		setPointLock(0,HoverPoints::LockToLeft);
		setPointLock(points.size()-1, HoverPoints::LockToRight);
//	}

	m_LutPoints.clear();
	
	for (  i=0; i<lut_poits.size(); ++i){
		 m_LutPoints << lut_poits[i] ;
	}
	
}
void  HoverPoints::getPoints(QPolygonF &points,QLutVectorPoints &lut_poits) const 
{
	int i;
	points.clear();
	
	bool ww_point_ok = m_WWPoints.size()>=m_EditPoints.size();
	for (  i=0; i<m_EditPoints.size(); ++i){
		QPointF p_temp_org = m_EditPoints.at(i);
		if(ww_point_ok && (m_editMode == CenterLineMode)){
			QPointF ww_point = m_WWPoints.at(i);
			p_temp_org.setY(ww_point.y());
		}
		 
        points <<  p_temp_org;
		//
		
	}
	//
	lut_poits.clear();
	for (  i=0; i<m_LutPoints.size(); ++i){
		 
        lut_poits <<  m_LutPoints.at(i);
		//
		
	}
}

void HoverPoints::movePoint(int index, const QPointF &point, bool emitUpdate)
{
	if(!m_editable) return;

    QPointF p_temp = bound_point(point, boundingRect(), m_locks.at(index));

	checkLockCenterLine(p_temp);
 
	m_EditPoints[index] = p_temp;

    if (emitUpdate)
        firePointChange();
}


inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}


inline static bool y_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.y() < p2.y();
}

void HoverPoints::firePointChange()
{
//    printf("HoverPoints::firePointChange(), current=%d\n", m_currentIndex);

    if (m_sortType != NoSort) {

        QPointF oldCurrent;
        if (m_currentIndex != -1) {
            oldCurrent = m_EditPoints[m_currentIndex];
        }

        if (m_sortType == XSort)
            qSort(m_EditPoints.begin(), m_EditPoints.end(), x_less_than);
        else if (m_sortType == YSort)
            qSort(m_EditPoints.begin(), m_EditPoints.end(), y_less_than);

        // Compensate for changed order...
        if (m_currentIndex != -1) {
            for (int i=0; i<m_EditPoints.size(); ++i) {
                if (m_EditPoints[i] == oldCurrent) {
                    m_currentIndex = i;
                    break;
                }
            }
        }

//         printf(" - firePointChange(), current=%d\n", m_currentIndex);

    }

//     for (int i=0; i<m_points.size(); ++i) {
//         printf(" - point(%2d)=[%.2f, %.2f], lock=%d\n",
//                i, m_points.at(i).x(), m_points.at(i).y(), m_locks.at(i));
//     }

    emit pointsChanged(m_EditPoints);
}

bool HoverPoints::checkLockCenterLine(QPointF &point) const 
{
	if(m_editMode==CenterLineMode){
		point.setY(m_CenterLinePos);
		return true;
	}else{
		return false;
	}
}



void HoverPoints::editPointsProperty()
{
	if(m_currentEditIndex<0) return;
	if(m_LutPoints.size()>m_currentEditIndex){
		QColor color = QColorDialog::getColor( m_LutPoints[m_currentEditIndex]);
		if(color.isValid()){
			m_LutPoints[m_currentEditIndex] = color;
		}
		 
	}
}