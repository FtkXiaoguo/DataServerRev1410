#ifndef IMAGE_VIEWER_OPENGL_GUI__H
#define IMAGE_VIEWER_OPENGL_GUI__H
 
 
#include "imageViewer.h"
 
#include <QGLWidget>

class CanvasWidgetOpenGL : public  QGLWidget , public CanvasWidget
{
    Q_OBJECT
public:
	 

    explicit CanvasWidgetOpenGL(QWidget *parent = 0 , const char *name = 0,QGLFormat &format=QGLFormat(QGL::SampleBuffers));

	bool getCanUseOpenGL() { return m_canUseShader;};
 
	virtual void destroyAll();
	virtual void myUpdate();
	virtual void mySetMouseTracking(bool on);

	virtual void setLutProc();
	virtual void updateViewer();
	virtual void doFilter(int no) ;
	virtual void initializeGL();
    virtual void resizeGL( int width, int height );
    virtual void paintGL();

	virtual bool createFilters() ;

	virtual void myResize(QSize &size)  ;
signals:
public slots:
protected:
	virtual void setupDrawText();
	virtual void drawTextInfo(QColor textColor,QPoint offset=QPoint(0,0));
	virtual void makeLut();

	virtual int getWidth() { return geometry().width();}  ;
	virtual int getHeight(){ return geometry().height();}  ;

	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *);

	virtual void initCommon();

	bool bindTexture();
	 
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
	//
	QStringList	m_drawTextLeftUp;
	QStringList	m_drawTextLeftDown;
	QStringList	m_drawTextRightUp;
	QStringList	m_drawTextRightDown;
 

	COpenGLExt *m_openGLOp;
	 GLuint m_imageBufferTex;
	 GLuint m_lutBufferTex;

	 bool m_canUseShader;
};
 



#endif //IMAGE_VIEWER_OPENGL_GUI__H


