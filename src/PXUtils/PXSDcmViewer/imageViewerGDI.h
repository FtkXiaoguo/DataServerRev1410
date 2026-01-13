#ifndef IMAGE_VIEWER_GDI_GUI__H
#define IMAGE_VIEWER_GDI_GUI__H
 
 
#include "imageViewer.h"
 
#include <QWidget>

class CanvasWidgetGDI :  public  QWidget ,  public CanvasWidget
{
    Q_OBJECT
public:
	 

    explicit CanvasWidgetGDI(QWidget *parent = 0 , const char *name = 0 );
 

	virtual bool createFilters() ;
	virtual void setLutProc();
	virtual void destroyAll();
	virtual void myUpdate();
	virtual void mySetMouseTracking(bool on);

	virtual void updateViewer();

	virtual void myResize(QSize &size)  ;
signals:
public slots:
protected:
	
	 virtual int getWidth() { return geometry().width();}  ;
	virtual int getHeight(){ return geometry().height();}  ;


	virtual void setupImageData();

	virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

	virtual void closeEvent(QCloseEvent *);
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *);
#endif

protected:
	 
};
 



#endif //IMAGE_VIEWER_GDI_GUI__H


