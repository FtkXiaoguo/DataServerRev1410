#include "imageViewerGDI.h"

 
#include "imageProcCPU.h"
#include "imageProcShader.h"


#include <QtGui/QProgressDialog>
 
#include <QtGui>
#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>
 
 #include <QGLPixelBuffer>

#include "OpenGLShader.h"
#include "OpenGLCpuShader.h"


#include "PXDcmSAdmin/PxDcmStudysManage.h"

#include "dispHist.h"
#include "mainwindow.h"

 
 
CanvasWidgetGDI::CanvasWidgetGDI(QWidget *parent, const char *name) : CanvasWidget(parent)
 
{
	  
}

void CanvasWidgetGDI::destroyAll()
{
	 
}
 
void CanvasWidgetGDI::myUpdate()
{
	update();
}

void CanvasWidgetGDI::mySetMouseTracking(bool on)
{
	setMouseTracking( on);
}
void CanvasWidgetGDI::myResize(QSize &size)
{
	resize(size);
 	setupViewSize();

	if( (m_TexViewParam.m_imageSizeX<1) ||
		(m_TexViewParam.m_imageSizeY<1)
		){
			return;
	}

}


void CanvasWidgetGDI::updateViewer()
{
//	updateGL();
}
void CanvasWidgetGDI::setupImageData()
{
	 
}


void CanvasWidgetGDI::mousePressEvent(QMouseEvent *e)
{
	QWidget::mousePressEvent(e);
	m_EvtProcHander->mousePressEvent(e);
}
void CanvasWidgetGDI::mouseReleaseEvent(QMouseEvent *e)
{
	QWidget::mouseReleaseEvent(e);
	m_EvtProcHander->mouseReleaseEvent(e);
}
void CanvasWidgetGDI::mouseDoubleClickEvent(QMouseEvent *e)
{
	QWidget::mouseDoubleClickEvent(e);
	m_EvtProcHander->mouseDoubleClickEvent(e);
}
void CanvasWidgetGDI::mouseMoveEvent(QMouseEvent *e)
{
	QWidget::mouseMoveEvent(e);
	if(!m_EvtProcHander->mouseMoveEvent(e)){
		//Null proc
		//
	//	this->updateViewer();
	};
}
#ifndef QT_NO_WHEELEVENT
void CanvasWidgetGDI::wheelEvent(QWheelEvent *e)
{
	QWidget::wheelEvent(e);
	m_EvtProcHander->wheelEvent(e);
}
#endif
 

void CanvasWidgetGDI::closeEvent(QCloseEvent *e)
{
	QWidget::closeEvent(e);
	destroyAll();
}


void CanvasWidgetGDI::setLutProc()
{
	 
 
	if(m_ImageProcessor){
		CImageFilter *filter_proc = m_ImageProcessor->getCurrentFilter();
		if(filter_proc){
			filter_proc->setupLut(m_lutBuffer,m_lutSize);
		}
	}
 
	CanvasWidget::setLutProc();
 	
}

bool CanvasWidgetGDI::createFilters()
{
	if(!m_ImageProcessor) {
		 
		m_ImageProcessor = new CImageProcCPU; 
		 
	}
	return CanvasWidget::createFilters();
}