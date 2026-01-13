#include "imageViewerOpenGL.h"

 
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
#include "imageProcCPU.h"

#include "PXDcmSAdmin/PxDcmStudysManage.h"
#include "PXDcmSAdmin/QtHelper.h"

#include "dispHist.h"
#include "mainwindow.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif
 
 

inline void calZoomPanTexPos(const CTexVieweParam &inParam, CTexVieweParam &outParam)
{
	 float v_imageSizeX = inParam.m_imageSizeX*inParam.m_zoom;
	 float v_imageSizeY = inParam.m_imageSizeY*inParam.m_zoom;
 
	 ///////////
	 outParam.m_ver_x0 = (inParam.m_ViewSizeX/2.0f-v_imageSizeX/2.0f)/inParam.m_ViewSizeX*2.0f -1.0f;
	 outParam.m_ver_y0 = (inParam.m_ViewSizeY/2.0f-v_imageSizeY/2.0f)/inParam.m_ViewSizeY*2.0f -1.0f;
	 //
	 outParam.m_ver_x1 = (inParam.m_ViewSizeX/2.0f-v_imageSizeX/2.0f +v_imageSizeX)/inParam.m_ViewSizeX*2.0f -1.0f;
	 outParam.m_ver_y1 = (inParam.m_ViewSizeY/2.0f-v_imageSizeY/2.0f +v_imageSizeY)/inParam.m_ViewSizeY*2.0f -1.0f;

	 ///////////
 
#if 0
	 outParam.m_tex_x0 = 0.0f + inParam.m_panX;
	 outParam.m_tex_y0 = 0.0f + inParam.m_panY;
	 //
	 outParam.m_tex_x1 = 1.0f ;
	 outParam.m_tex_y1 = 1.0f ;
#else
	 outParam.m_tex_x0 = 0.0f;
	 outParam.m_tex_y0 = 0.0f;
	 //
	 outParam.m_tex_x1 = 1.0f ;
	 outParam.m_tex_y1 = 1.0f ;
	 //
	 outParam.m_ver_x0	-= inParam.m_panX;
	 outParam.m_ver_y0  -= inParam.m_panY;
	 //
	 outParam.m_ver_x1  -= inParam.m_panX;
	 outParam.m_ver_y1  -= inParam.m_panY;
 
#endif
}


 
CanvasWidgetOpenGL::CanvasWidgetOpenGL(QWidget *parent, const char *name,QGLFormat &format) : QGLWidget(format,parent),CanvasWidget(parent)
 
{
	m_canUseShader = false;
	   m_openGLOp = 0;// new COpenGLShader;
	
	   setAutoFillBackground(true);

}
 
 
void CanvasWidgetOpenGL::destroyAll()
{
	if(m_openGLOp){
		delete m_openGLOp;
		m_openGLOp = 0;
	}
}
 
void CanvasWidgetOpenGL::myUpdate()
{
	update();
}

void CanvasWidgetOpenGL::mySetMouseTracking(bool on)
{
	setMouseTracking( on);
}
void CanvasWidgetOpenGL::initCommon()
{
    qglClearColor(QColor(Qt::darkGray));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);
 
}

void CanvasWidgetOpenGL::makeLut()
{
	if(!m_canUseShader){
		m_ImageProcessor->setWWWL(m_ww,m_wl,m_displayPixelMax);
	}else{
		CanvasWidget::makeLut();
	}
}
bool CanvasWidgetOpenGL::createFilters()
{
	if(!m_ImageProcessor) {
		 
//		m_ImageProcessor = new CImageProcShader((COpenGLShader*)m_openGLOp);
		if(m_canUseShader){
			m_ImageProcessor = new CImageProcShader((COpenGLShader*)m_openGLOp);
		}else{
			m_ImageProcessor = new CImageProcCPU;
		}
		
		 
	}
	return CanvasWidget::createFilters();
}

void CanvasWidgetOpenGL::setupImageData()
{
	//
	m_openGLOp->deleteTexture(m_imageBufferTex);

	if( m_curImagePixelinfo->m_samplesPerPixel == 4){
		m_openGLOp->createTexture2D_RGBA((const float*)m_imageBuffer, m_TexViewParam.m_imageSizeX,m_TexViewParam.m_imageSizeY,m_imageBufferTex);
	}else{
		m_openGLOp->createTexture2D((const float*)m_imageBuffer,m_TexViewParam.m_imageSizeX,m_TexViewParam.m_imageSizeY,m_imageBufferTex);
	}

	m_openGLOp->setupSize(m_TexViewParam.m_imageSizeX,m_TexViewParam.m_imageSizeY);
//	setupViewSize();
}

void CanvasWidgetOpenGL::initializeGL()
{
	
	m_canUseShader = COpenGLExt::checkOpenGLShader();

 //  	m_canUseShader = false;

	if(!m_canUseShader){
 
		m_openGLOp =  new COpenGLCpuShader;
 
	}else{

		m_openGLOp =  new COpenGLShader;
	}
    

	glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 

	//initCommon();

	m_openGLOp->init();

 
 
}


void CanvasWidgetOpenGL::updateViewer()
{
	updateGL();
}

void CanvasWidgetOpenGL::resizeGL( int width, int height ){
//    glViewport(0,0,width,height);

//	m_TexViewParam.m_ViewSizeX = geometry().width();
//    m_TexViewParam.m_ViewSizeY = geometry().height();
    
 
}

void CanvasWidgetOpenGL::paintEvent(QPaintEvent *e)
{
	if(!m_paintFlag){
	//clear background
		QRect myrect = geometry();
		QPainter p;
		p.begin(this);
		p.drawRect(myrect);
	}
	QGLWidget::paintEvent( e);

		
}


bool CanvasWidgetOpenGL::bindTexture()
{
#ifdef USE_SHARDER_LUT	 
	if(m_canUseShader){
		m_openGLOp->bindTexture(1,m_lutBufferTex,GL_TEXTURE_1D);
	}
#endif
	 

	m_openGLOp->bindTexture(0,m_imageBufferTex,GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, m_imageBufferTex);

	return true;
}


void CanvasWidgetOpenGL::paintGL()
{
 
	if(!m_paintFlag) return;
 
	 
//	makeCurrent();

	 
    glEnable(GL_TEXTURE_2D);

	glEnable( GL_TEXTURE_1D ); 
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
 
    glMatrixMode( GL_MODELVIEW);
    glLoadIdentity();
 
	glViewport(m_TexViewParam.m_ViewOrgX, m_TexViewParam.m_ViewOrgY, m_TexViewParam.m_ViewSizeX, m_TexViewParam.m_ViewSizeY);
	   
 
	bindTexture();
	 
	bool Shader_actived_flag = false;
 
	if(m_ImageProcessor){
		if(m_ImageProcessor->isShader()){
			
 
			CImageFilter *curFilter =  (m_ImageProcessor->getCurrentFilter());
			if(curFilter){
				m_openGLOp->activeShader(true,(m_curImagePixelinfo->m_samplesPerPixel==4));
			 	curFilter->activeFilter();

				Shader_actived_flag = true;
			}else{
				m_openGLOp->activeShader(false);
			}
		}
 
	 
	}
 
	
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
 
 
 
	calZoomPanTexPos(m_TexViewParam,m_TexViewParam);
	
 
	glTexCoord2f(m_TexViewParam.m_tex_x0,m_TexViewParam.m_tex_y1);		glVertex3f(m_TexViewParam.m_ver_x0,  m_TexViewParam.m_ver_y0, 0.5);
	glTexCoord2f(m_TexViewParam.m_tex_x1,m_TexViewParam.m_tex_y1);		glVertex3f(m_TexViewParam.m_ver_x1,  m_TexViewParam.m_ver_y0, 0.5);
	glTexCoord2f(m_TexViewParam.m_tex_x1,m_TexViewParam.m_tex_y0);		glVertex3f(m_TexViewParam.m_ver_x1,	 m_TexViewParam.m_ver_y1, 0.5);
	glTexCoord2f(m_TexViewParam.m_tex_x0,m_TexViewParam.m_tex_y0);		glVertex3f(m_TexViewParam.m_ver_x0,	 m_TexViewParam.m_ver_y1, 0.5);
 
 
    glEnd();
 
	if(Shader_actived_flag){
  		m_openGLOp->activeShader(false);
	}

 	glPixelZoom(1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
 

    glDisable(GL_TEXTURE_2D);
 
	drawTextInfo(QColor(0 , 255 , 255 ));
//	drawTextInfo(QColor(28*0.8 , 162*0.8 , 223*0.8  ),QPoint(1,1));


}
void CanvasWidgetOpenGL::setLutProc()
{
	if(!m_ImageProcessor) return;
	 
	if(!m_canUseShader){
		if(m_ImageProcessor){
			CImageFilter *filter_proc = m_ImageProcessor->getCurrentFilter();
			if(filter_proc){
				filter_proc->setWWWL(m_ww,m_wl,m_displayPixelMax);
			//	filter_proc->setupLut(m_lutBuffer,m_lutSize);
			}
		}
	}else{
   
#ifdef USE_SHARDER_LUT
		m_openGLOp->deleteTexture(m_lutBufferTex);
		m_openGLOp->createTexture1D((float*)m_lutBuffer,m_lutSize,m_lutBufferTex);
#else
		CImageFilter *filter_proc = m_ImageProcessor->getCurrentFilter();
		if(filter_proc){
			filter_proc->setWWWL(m_ww,m_wl,m_displayPixelMax);
		 
		}
#endif
 
	}
	 
	CanvasWidget::setLutProc();
 	
}
void CanvasWidgetOpenGL::doFilter(int filter_no)
{
 
	CanvasWidget::doFilter(filter_no);

//	m_paintFlag = false; //ˆêŽž“I‚É•`‰æ‚µ‚È‚¢‚æ‚¤‚É‚·‚é

	if(m_curImagePixelinfo->m_samplesPerPixel == 3){
		//
	}else{
		if(m_curFilterNo>=0){
		 
			CImageFilter *curFilter = m_ImageProcessor->getCurrentFilter();
			if(!curFilter) return;
			curFilter->setupProcImage(m_TexViewParam.m_imageSizeX,m_TexViewParam.m_imageSizeY,m_imageBuffer,m_procImageBuffer);
				 
	 		curFilter->doFilter();

			if(!m_ImageProcessor->isShader()){
		 		m_openGLOp->deleteTexture(m_imageBufferTex);
	 			m_openGLOp->createTexture2D((float*)m_procImageBuffer,m_TexViewParam.m_imageSizeX,m_TexViewParam.m_imageSizeY,m_imageBufferTex);
			}

		}else{
			m_ImageProcessor->selCurrentFilter("");
		 	m_openGLOp->deleteTexture(m_imageBufferTex);
	 		m_openGLOp->createTexture2D((float*)m_imageBuffer,m_TexViewParam.m_imageSizeX,m_TexViewParam.m_imageSizeY,m_imageBufferTex);

		}

//		m_paintFlag = true;

//		updateViewer();
	}
 
}
void CanvasWidgetOpenGL::resizeEvent(QResizeEvent *event)
{
	

	QGLWidget::resizeEvent(event);

	QSize size = this->size();

	myResize(size);
	 
	 
	
}
void CanvasWidgetOpenGL::myResize(QSize &size)
{
	float org_usr_zoom  = 1.0f;
	{
		float dispRateX = (float)m_TexViewParam.m_ViewSizeX/m_TexViewParam.m_imageSizeX;
		float dispRateY = (float)m_TexViewParam.m_ViewSizeY/m_TexViewParam.m_imageSizeY;

		float basic_zoom = dispRateX;
		if(dispRateY<dispRateX){
			basic_zoom = dispRateY;
		}
		org_usr_zoom = m_TexViewParam.m_zoom/basic_zoom;
	}

	QRect prev_rect =  geometry();
	resize(size);
 	setupViewSize();

	if( (m_TexViewParam.m_imageSizeX<1) ||
		(m_TexViewParam.m_imageSizeY<1)
		){
			return;
	}

	if(1){
		float dispRateX = (float)m_TexViewParam.m_ViewSizeX/m_TexViewParam.m_imageSizeX;
		float dispRateY = (float)m_TexViewParam.m_ViewSizeY/m_TexViewParam.m_imageSizeY;

		float new_basic_zoom = dispRateX;
		if(dispRateY<dispRateX){
			new_basic_zoom = dispRateY;
		}

		float new_zoom = org_usr_zoom*new_basic_zoom;
	//	if(try_zoom >m_TexViewParam.m_zoom){
			changeZoomRate(new_zoom - m_TexViewParam.m_zoom);
	//	}
	}
}


void CanvasWidgetOpenGL::mousePressEvent(QMouseEvent *e)
{
	QGLWidget::mousePressEvent(e);
	m_EvtProcHander->mousePressEvent(e);
}
void CanvasWidgetOpenGL::mouseReleaseEvent(QMouseEvent *e)
{
	QGLWidget::mouseReleaseEvent(e);
	m_EvtProcHander->mouseReleaseEvent(e);
}
void CanvasWidgetOpenGL::mouseDoubleClickEvent(QMouseEvent *e)
{
	QGLWidget::mouseDoubleClickEvent(e);
	m_EvtProcHander->mouseDoubleClickEvent(e);
}
void CanvasWidgetOpenGL::mouseMoveEvent(QMouseEvent *e)
{
	QGLWidget::mouseMoveEvent(e);
	if(!m_EvtProcHander->mouseMoveEvent(e)){
		//Null proc
		//
	//	this->updateViewer();
	};
}
#ifndef QT_NO_WHEELEVENT
void CanvasWidgetOpenGL::wheelEvent(QWheelEvent *e)
{
	QGLWidget::wheelEvent(e);
	m_EvtProcHander->wheelEvent(e);
}
#endif
 

void CanvasWidgetOpenGL::closeEvent(QCloseEvent *e)
{
	QGLWidget::closeEvent(e);
	destroyAll();
}

#if 0
	if (p->SeriesTime.GetLength() == 6 /* hh:mm:ss */)
		{
			p->SeriesTime.Insert(2, _T(":"));
			p->SeriesTime.Insert(5, _T(":"));
		}
#endif

QString _normal_Time_str(const QString &time_string){
		QString str_temp;
		str_temp = time_string.mid(0,2) + QString(":");
		str_temp = str_temp + time_string.mid(2,2) + QString(":");
		str_temp = str_temp + time_string.mid(4,2) ;

		return str_temp;
}
QString _normal_Date_str(const QString &date_string){
		QString str_temp;
		str_temp = date_string.mid(0,4) + QString("/");
		str_temp = str_temp + date_string.mid(4,2) + QString("/");
		str_temp = str_temp + date_string.mid(6,2) ;

		return str_temp;
}
void CanvasWidgetOpenGL::setupDrawText()
{
	m_drawTextLeftUp.clear();
	m_drawTextLeftDown.clear();
	m_drawTextRightUp.clear();
	m_drawTextRightDown.clear();

	if(!m_CurrentDicomInfo) return;

	QString str_temp;

	///////////////////////////////
	//left-up
	if(m_CurrentDicomInfo->m_patientName.size()>0){
		str_temp = Str2QString(m_CurrentDicomInfo->m_patientName);
	}
	if(m_CurrentDicomInfo->m_patientSex.size()>0){
		str_temp = str_temp+QString("(")+Str2QString(m_CurrentDicomInfo->m_patientSex)+QString(")");
	}

	if(str_temp.size()>0){
		m_drawTextLeftUp.append(str_temp);
		str_temp.clear();
	}	
	///
	if(m_CurrentDicomInfo->m_patientID.size()>0){
		m_drawTextLeftUp.append(Str2QString(m_CurrentDicomInfo->m_patientID));
	}
	///
	 
	if(m_CurrentDicomInfo->m_patientBirthDate.size()>0){
		if(m_CurrentDicomInfo->m_patientBirthDate.size() == 8){
			QString birthday_temp = Str2QString(m_CurrentDicomInfo->m_patientBirthDate) ;
			str_temp =  _normal_Date_str(birthday_temp);
		}else{
			str_temp =  Str2QString(m_CurrentDicomInfo->m_patientBirthDate) ;
		}
	}
	if(str_temp.size()>0){
		m_drawTextLeftUp.append(str_temp);
		str_temp.clear();
	}

	///
	{
//		string.Format(_T("St: %s Se: %s"), pInfo->StudyNumber, pInfo->SeriesNumber);
//			pObj->SetString( string );

		if(m_CurrentDicomInfo->m_seriesNumber.size()>0){
			m_drawTextLeftUp.append(QString("Se: ") + Str2QString(m_CurrentDicomInfo->m_seriesNumber));
		}
	}
	
	///
	if(m_CurrentDicomInfo->m_studyTime.size()>0){
		if( (m_CurrentDicomInfo->m_studyTime.size() == 6) ||
			(m_CurrentDicomInfo->m_studyTime[3] != ':') )
		{
			str_temp = _normal_Time_str(Str2QString(m_CurrentDicomInfo->m_studyTime));
		}else{
			str_temp =  Str2QString(m_CurrentDicomInfo->m_studyTime);
		}
		m_drawTextLeftUp.append(str_temp);
		str_temp.clear();
	}
	
	
	///////////////////////////////
	//right-up
#if 0
	if(m_CurrentDicomInfo->m_studyDescription.size()>0){
		m_drawTextRightUp.append(Str2QString(m_CurrentDicomInfo->m_studyDescription));
	}
	///
	if(m_CurrentDicomInfo->m_studyDate.size()>0){
		str_temp = Str2QString(m_CurrentDicomInfo->m_studyDate);
	}

	if(m_CurrentDicomInfo->m_studyTime.size()>0){
		str_temp = str_temp+QString(" ")+Str2QString(m_CurrentDicomInfo->m_studyTime);
	}
	if(str_temp.size()>0){
		m_drawTextRightUp.append(str_temp);
		str_temp.clear();
	}

	if(m_CurrentDicomInfo->m_modalitiesInStudy.size()>0){
		m_drawTextRightUp.append(Str2QString(m_CurrentDicomInfo->m_modalitiesInStudy));
	}
#endif
	if(m_CurrentDicomInfo->m_InstitutionName.size()>0){
		m_drawTextRightUp.append(Str2QString(m_CurrentDicomInfo->m_InstitutionName));
	}
	if(m_CurrentDicomInfo->m_ManufacturerModelName.size()>0){
		m_drawTextRightUp.append(Str2QString(m_CurrentDicomInfo->m_ManufacturerModelName));
	}
	if(m_CurrentDicomInfo->m_StationName.size()>0){
		m_drawTextRightUp.append(Str2QString(m_CurrentDicomInfo->m_StationName));
	}
	///
	
	///////////////////////////////
	//left-down

	if(m_CurrentDicomInfo->m_KVP.size()>0){
		QString kvp_temp = Str2QString(m_CurrentDicomInfo->m_KVP);
		kvp_temp = QString::number(kvp_temp.toFloat(),'f',2);
		str_temp = kvp_temp+QString("kV ");
		m_drawTextLeftDown.append(str_temp);
		str_temp.clear();
	}
	if(m_CurrentDicomInfo->m_XRayTubeCurrent.size()>0){
		QString mA_temp = Str2QString(m_CurrentDicomInfo->m_XRayTubeCurrent);
		mA_temp = QString::number(mA_temp.toFloat(),'f',2);
		str_temp = mA_temp + QString("mA");
		m_drawTextLeftDown.append(str_temp);
		str_temp.clear();
	}
	 
	///

	if(m_CurrentDicomInfo->m_ExposureTime.size()>0){
		QString time_temp = Str2QString(m_CurrentDicomInfo->m_ExposureTime);
		time_temp = QString::number(time_temp.toFloat()/1000.0,'f',1);
		str_temp = time_temp + QString("Sec");
		m_drawTextLeftDown.append(str_temp);
		 
	}


	///////////////////////////////
	//right-down
	 
 
	if(m_CurrentDicomInfo->m_studyDescription.size()>0){
		m_drawTextRightDown.append(Str2QString(m_CurrentDicomInfo->m_studyDescription));
	}

	if(m_CurrentDicomInfo->m_seriesDescription.size()>0){
		m_drawTextRightDown.append(Str2QString(m_CurrentDicomInfo->m_seriesDescription));
	}
	///
 
 
	 	
}
void CanvasWidgetOpenGL::drawTextInfo(QColor textColor,QPoint offset)
{
//	QFont myFont( "TypeWriter", 10 );
	QFont myFont( "System", 10,-1/*weight*/,false/*italic*/ );

	QFontMetrics fm(myFont);

  
   int yy = fm.height();

 
 
   glColor4f(textColor.red()/255.0, textColor.green()/255.0, textColor.blue()/255.0, 1.0);


	int posX ,posY;

	int lineHeight = fm.height()+2;//18;
	int char_w = 6;
	{//left-up
		posX = 10;
		posY = lineHeight;

		posX +=offset.x();
		posY +=offset.y();
		for(int i=0;i<m_drawTextLeftUp.size();i++){
			renderText(m_TexViewParam.m_ViewOrgX+posX,m_TexViewParam.m_ViewOrgY+posY+i*lineHeight,m_drawTextLeftUp[i],myFont); 
		}
		
	}

	{//right-up
		posX = 10;
		posY = lineHeight;
		{
			int max_len  = 0;
			for(int i=0;i<m_drawTextRightUp.size();i++){
				int len =   fm.width( m_drawTextRightUp[i]);
				if(len>max_len){
					max_len = len;
				}
			}
			posX = m_TexViewParam.m_ViewOrgX+m_TexViewParam.m_ViewSizeX - max_len;//*char_w ;
			posX -=10;
		}

		posX +=offset.x();
		posY +=offset.y();		
		for(int i=0;i<m_drawTextRightUp.size();i++){
			renderText(m_TexViewParam.m_ViewOrgX+posX,m_TexViewParam.m_ViewOrgY+posY+i*lineHeight,m_drawTextRightUp[i],myFont); 
		}
		
	}
	 

	{//left-down
		posX = 10;
		posY = m_TexViewParam.m_ViewOrgY+m_TexViewParam.m_ViewSizeY-(m_drawTextLeftDown.size() )*lineHeight;
	 	posY += 8;

		posX +=offset.x();
		posY +=offset.y();		  
		for(int i=0;i<m_drawTextLeftDown.size();i++){
			renderText(m_TexViewParam.m_ViewOrgX+posX,m_TexViewParam.m_ViewOrgY+posY+i*lineHeight,m_drawTextLeftDown[i],myFont); 
		}
		
	}

	{//right-down
		 
		{
			int max_len  = 0;
			for(int i=0;i<m_drawTextRightDown.size();i++){
				int len = fm.width( m_drawTextRightDown[i]);
				if(len>max_len){
					max_len = len;
				}
			}
			posX = m_TexViewParam.m_ViewOrgX+m_TexViewParam.m_ViewSizeX - max_len;//*char_w ;
		 	posX -=10;
		}
		posY = m_TexViewParam.m_ViewOrgY+m_TexViewParam.m_ViewSizeY-(m_drawTextRightDown.size() )*lineHeight;
	 	posY += 8;

		posX +=offset.x();
		posY +=offset.y();		
		for(int i=0;i<m_drawTextRightDown.size();i++){
			renderText(m_TexViewParam.m_ViewOrgX+posX,m_TexViewParam.m_ViewOrgY+posY+i*lineHeight,m_drawTextRightDown[i],myFont); 
		}
		
	}
	 
	
}