/****************************************************************************
**
**
****************************************************************************/

#ifndef HOVERPOINTS_H
#define HOVERPOINTS_H

#include <QtGui>

QT_FORWARD_DECLARE_CLASS(QBypassWidget)

typedef QVector<QColor> QLutVectorPoints ;
class HoverPoints : public QObject
{
    Q_OBJECT
public:
	enum EditMode {
		CurveEditMode,
		CenterLineMode,
	};
	/*
	*  CurveEditMode:
	*     m_points: 
	*          x: WL,  node
	*          y: Opacity
	* 
	*  CenterLineMode:
	*     m_points: 
	*          x: WL,  node
	*          y: CenterLine fixed
	*     m_WWPoints: (“à•”“I•â•)
	*          x: WL , node
	*          y: WW
	*     m_LutPoints:
	*           m_points.x -> WL,node (‹¤’Êj
	*           R,G,B
	*     
	*/
    enum PointShape {
        CircleShape,
        RectangleShape
    };

    enum LockType {
        LockToLeft   = 0x01,
        LockToRight  = 0x02,
        LockToTop    = 0x04,
        LockToBottom = 0x08
    };

    enum SortType {
        NoSort,
        XSort,
        YSort
    };

    enum ConnectionType {
        NoConnection,
        LineConnection,
        CurveConnection
    };

    HoverPoints(QWidget *widget, PointShape shape);

    bool eventFilter(QObject *object, QEvent *event);

    void paintPoints();

    inline QRectF boundingRect() const;
    void setBoundingRect(const QRectF &boundingRect) { m_bounds = boundingRect; }

    void getPoints(QPolygonF &points,QLutVectorPoints &lut_poits) const ;// { return m_points; }
    void setPoints(const QPolygonF &points,const QLutVectorPoints &lut_poits  );
	/*
	*  CenterLineMode:
	*           x: WL
	*           y: WW ->m_WWPoints(“à•”)
	*/
//	QLutVectorPoints getEditLutPoints() const { return m_LutPoints;};
	int getPointsSize(){ return m_EditPoints.size();};

    QSizeF pointDrawSize() const { return m_pointDrawSize; }
    void setPointDrawSize(const QSizeF &size) { m_pointDrawSize = size; }

    SortType sortType() const { return m_sortType; }
    void setSortType(SortType sortType) { m_sortType = sortType; }

    ConnectionType connectionType() const { return m_connectionType; }
    void setConnectionType(ConnectionType connectionType) { m_connectionType = connectionType; }

    void setConnectionPen(const QPen &pen) { m_connectionPen = pen; }
    void setShapePen(const QPen &pen) { m_pointPen = pen; }
    void setShapeBrush(const QBrush &brush) { m_pointBrush = brush; }

    void setPointLock(int pos, LockType lock) { m_locks[pos] = lock; }

    void setEditable(bool editable) { m_editable = editable; }
    bool editable() const { return m_editable; }
	void setFixPointsNum(bool fixPoints) { m_fixPointsNum = fixPoints;}
	void setLockToCenterLine(bool lock){ lock ? m_editMode = CenterLineMode: CurveEditMode;};
	void setCenterLinePos(double pos){m_CenterLinePos = pos;};
public slots:
    void setEnabled(bool enabled);
    void setDisabled(bool disabled) { setEnabled(!disabled); }

signals:
    void pointsChanged(const QPolygonF &points);

public:
    void firePointChange();

private:
	void editPointsProperty();

	bool checkLockCenterLine(QPointF &point) const ;

    inline QRectF pointBoundingRect(int i) const;
	inline bool getLutPointColor(int i,QColor &lutColor) const;
	inline int hitPoints(const QPointF &clickPos) const;
    void movePoint(int i, const QPointF &newPos, bool emitChange = true);

    QWidget *m_widget;

    QPolygonF m_EditPoints;
    QRectF m_bounds;
    PointShape m_shape;
    SortType m_sortType;
    ConnectionType m_connectionType;

    QVector<uint> m_locks;

    QSizeF m_pointDrawSize;
    int m_currentIndex;
    bool m_editable;
	bool m_fixPointsNum;
    bool m_enabled;

    QPen m_pointPen;
    QBrush m_pointBrush;
    QPen m_connectionPen;
	//
	EditMode m_editMode;
	double m_CenterLinePos;
	//
	QPolygonF m_WWPoints;
	QLutVectorPoints m_LutPoints;
	//
	QTime m_curTime_mouse_press;
	int m_currentEditIndex; 
	int m_currentEditWWIndex;
	QPointF m_oldEditWWPoint;
	QPointF m_currentEditWWPos; //x,y position
};



inline QRectF HoverPoints::pointBoundingRect(int i) const
{
	double w_r = 0.0;
	double h_r = 0.0;
	if(m_WWPoints.size()>i){
		QPointF p = m_WWPoints.at(i);
		w_r =  p.y() ;
		if(w_r<0.5) {
			w_r = 0.5;
		}
		h_r = m_CenterLinePos*1.2;//13.0;
	}
    QPointF p = m_EditPoints.at(i);
    qreal w = m_pointDrawSize.width();
    qreal h = m_pointDrawSize.height();
	if(m_editMode == CenterLineMode){
		if(w_r>2.0)
			w  =w_r;
		if(h_r>2.0)
			h =h_r;
	}else{	
		//w *=0.5;
		//h *=0,5;
		
	}
    qreal x = p.x() - w / 2;
    qreal y = p.y() - h / 2;
    return QRectF(x, y, w, h);
}

inline QRectF HoverPoints::boundingRect() const
{
    if (m_bounds.isEmpty())
        return m_widget->rect();
    else
        return m_bounds;
}
inline bool HoverPoints::getLutPointColor(int i,QColor &lutColor) const
{
	if(m_LutPoints.size()<(i+1)) return false;
	lutColor = m_LutPoints[i];
	return true;
}

inline int HoverPoints::hitPoints(const QPointF &clickPos) const
{
	int index = -1;
 
    for (int i=0; i<m_EditPoints.size(); ++i) {
        QPainterPath path;
        if (m_shape == CircleShape)
            path.addEllipse(pointBoundingRect(i));
        else
            path.addRect(pointBoundingRect(i));

        if (path.contains(clickPos)) {
            index = i;
            break;
        }
    }
	return index;
}
#endif // HOVERPOINTS_H
