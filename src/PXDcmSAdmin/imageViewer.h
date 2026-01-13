#ifndef IMAGE_VIEWER_GUI__H
#define IMAGE_VIEWER_GUI__H

#include "ui_imageViewer.h"
 
#if 0 //not yet
 #include <QGLWidget>
class CanvasWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = 0 , const char *name = 0);
    virtual void initializeGL();
    virtual void resizeGL( int width, int height );
    virtual void paintGL();
signals:
public slots:
protected:
	GLuint m_texture2D;
	int m_sizeX;
	int m_sizeY;
	void *m_data;
	//
	int m_disp_width;
	int m_disp_height;
};
#endif

class CImageViewer : public QDialog {
	Q_OBJECT
	public:
		CImageViewer( QWidget *parent = 0);
		 
	 
		void openViewer();
	private:
		Ui::imageViewer ui;
	public slots:
	
 
	 
protected:
	virtual void resizeEvent(QResizeEvent *);
//	CanvasWidget *m_openGLWidget;
private:

	 
	 
};


#endif //IMAGE_VIEWER_GUI__H


