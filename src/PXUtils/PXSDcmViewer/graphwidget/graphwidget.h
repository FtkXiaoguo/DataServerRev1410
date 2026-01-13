/****************************************************************************
**
*
****************************************************************************/

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include "hoverpoints.h"

class ShadeWidget : public QWidget
{
    Q_OBJECT
public:
    enum ShadeType {
        RedShade,
        GreenShade,
        BlueShade,
        ARGBShade
    };

    ShadeWidget(ShadeType type, QWidget *parent);

	
	void destroy();

	void init();

    void setGradientStops(const QGradientStops &stops);

    void paintEvent(QPaintEvent *e);

 	QSize sizeHint() const {  return QSize(250, 140); }
  //  QPolygonF getPoints() const;
	//QLutVectorPoints getEditLutPoints() const;

    HoverPoints *hoverPoints() const { return m_hoverPoints; }

    uint colorAt(int x);

	bool getCurveData(double *dataX,double *dataY, int size,double *lut_rgb=0/*lut_rgb[3*size]: 0.0--1.0*/) const;
	void setCurveData(double *dataX,double *dataY, int size,double *lut_rgb=0/*lut_rgb[3*size]: 0.0--1.0*/);
//	bool getEditLutData(double *lut_rgb/*len=3*size*/, int size,double *dataNode=0) const;

	void init_coord(double usrX0,double usrY0, double usrX1,double usrY1);

	void SetColorBarLut(int size, double *node=0, double *rgb=0/*rgb[3*size]: 0.0--1.0*/);//カラーバー表示のため

	void setEditable(bool editable);
	void setLockToCenterLine(bool lock);
protected:
	virtual void resizeEvent(QResizeEvent *);
signals:
    void colorsChanged();

	protected:
	 virtual void closeEvent(QCloseEvent *);

private:
	double m_usr0[2];
	double m_usr1[2];
	double m_usr_coord_k[2];
	double m_usr_coord_b[2];
	

	void coord_usr2win(double usr_x, double usr_y, double &pos_x, double &pos_y)  const;
	void coord_win2usr(double pos_x, double pos_y,double &usr_x, double &usr_y)  const;
    void generateShade();

	bool m_LockToCenterLine;
	bool m_editable;
    ShadeType m_shade_type;
    QImage m_shade;
    HoverPoints *m_hoverPoints;
     QLinearGradient m_alpha_gradient;
	 //カラーバー表示のため
	 int m_ColorBar_lut_size;
	  double *m_ColorBar_lut_node;
	  double *m_ColorBar_lut_rgb;
	  bool m_update_ColorBar_flag;
  
   
};
////////


///
 
class CGraphWidget  : public QWidget
{
  Q_OBJECT
public:
    CGraphWidget(QWidget *parent);
	void setInstanceID( int id){m_instanceID = id;};

	void setCurveData(double *dataX,double *dataY, int size,double *lut_rgb=0/*lut_rgb[3*size]: 0.0--1.0*/);
	
	int getCurveNumber() const;
	bool getCurveData(double *dataX,double *dataY, int size,double *lut_rgb=0/*lut_rgb[3*size]: 0.0--1.0*/) const;
//	bool getEditLutData(double *lut_rgb/*len=3*size*/, int size,double *dataNode=0) const;

//	const QPolygonF getCurveData() const;

	void init_coord(double usrX0,double usrY0, double usrX1,double usrY1);

	void SetColorBarLut(int size, double *node=0, double *rgb=0/*rgb[3*size]: 0.0--1.0*/);

	void setEditable(bool editable);
	void setLockToCenterLine(bool lock);
  protected:
	  virtual void closeEvent(QCloseEvent *);
	void destroy();
	////////////
public slots:	
	void OnGetCurvePoints();
	
signals:
    void curvePointsChanged( );
private:
	 

	ShadeWidget *m_points_editor;
 
	//
   int m_instanceID;
};

#endif
