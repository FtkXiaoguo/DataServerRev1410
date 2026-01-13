#include "imageViewer.h"

#include "QtHelper.h"

#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>

 #include "AEsGUI.h"
  #include "PxDcmDbManage.h"


 
/*
* not yet
* just try it
*
*/
#if 0
CanvasWidget::CanvasWidget(QWidget *parent, const char *name) : QGLWidget(QGLFormat(QGL::SampleBuffers),parent)
{
	int xx;
// 	setFixedSize(200, 200);
    setAutoFillBackground(true);

	m_sizeX = 128;
	m_sizeY = 128;
	m_data = new char[m_sizeX*m_sizeY*4];

	char *data_p = (char*)m_data;
	for(int y_i=0;y_i<m_sizeY;y_i++){
		for(int x_i=0;x_i<m_sizeX;x_i++){
			int index = 4*(y_i*m_sizeX + x_i);
			data_p[index + 0] = x_i%255;
			data_p[index + 1] = x_i%255;
			data_p[index + 2] = x_i%255;
			data_p[index + 3] = x_i%255;
		}
	}
}

void CanvasWidget::initializeGL(){
    m_disp_width = geometry().width();
    m_disp_height = geometry().height();
    glViewport(0,0, m_disp_width, m_disp_height);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-m_disp_width *0.5, m_disp_width*0.5, -m_disp_height* 0.5, m_disp_height* 0.5, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0);
	//
	glGenTextures( 1, &m_texture2D );
	glBindTexture( GL_TEXTURE_2D, m_texture2D );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 4, m_sizeX, m_sizeY, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_data );
	glBindTexture( GL_TEXTURE_2D, 0 );
}
void CanvasWidget::resizeGL( int width, int height ){
    glViewport(0,0,width,height);
}
void CanvasWidget::paintGL(){
	glDisable(GL_DEPTH_TEST);

#if 0
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glColor4f(1.0f,0.0f,0.0f,1.0f);
    glBegin(GL_TRIANGLES);
      glVertex3f( -100.0f, 100.0f, 0.0f);
      glVertex3f(-100.0f,-100.0f, 0.0f);
      glVertex3f( 100.0f,-100.0f, 0.0f);
    glEnd();
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
//	glOrtho(0.0, m_disp_width, m_disp_height, 0.0, -1.0, 1.0);
	glOrtho(-m_disp_width *0.5, m_disp_width*0.5, -m_disp_height* 0.5, m_disp_height* 0.5, -1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D , m_texture2D);
	glEnable(GL_TEXTURE_2D);
#if 0
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0f, 1.0f); glVertex2d(10 , 230);//¶‰º
	glTexCoord2f(0.0f, 0.0f); glVertex2d(10 ,  10);//¶ã
	glTexCoord2f(1.0f, 0.0f); glVertex2d( 310 ,  10);//‰Eã
	glTexCoord2f(1.0f, 1.0f); glVertex2d( 310 , 230);//‰E‰º
	glEnd();
	glDisable(GL_ALPHA_TEST);
#endif
	glDisable(GL_TEXTURE_2D);

//	glutSwapBuffers();


}
#endif
 
CImageViewer::CImageViewer(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this); 
//	m_openGLWidget = new CanvasWidget(ui.drawArea);
 	 
}

void CImageViewer::resizeEvent(QResizeEvent *event)
{
	QDialog::resizeEvent(event);

	QSize size = ui.drawArea->size();

//	m_openGLWidget->resize(size);
}
void CImageViewer::openViewer()
{
	
	
	// openGLWidget->initializeGL();

}