 
#include "AboutInfo.h"
#include <QtGui/QProgressDialog>
 
#include <QtGui>
#include <QPrinter>
 #include <QtCore/QTimer>
 
 #include <QGLWidget>

 #include "OpenGLExt.h"
#include "AppVersion.h"

AboutInfo::AboutInfo(QWidget *parent) : QDialog(parent)
 
{
	ui.setupUi(this);
	 
#if 1
	bool  bHasMultitexture = COpenGLExt::init();

  /* ƒxƒ“ƒ_[‚ÆGPU */
	char gpuStr[256];
	sprintf_s(gpuStr, sizeof(gpuStr), "%s: OpenGL %s", 
		glGetString(GL_RENDERER), glGetString(GL_VERSION));

	
	std::string glExtStr = COpenGLExt::getGlExtension();

	bool canUseGlSl = COpenGLExt::checkOpenGLShader();

	QString disp_str_gl_info = gpuStr;

	disp_str_gl_info = QString("GPU: ") + disp_str_gl_info;
	if(bHasMultitexture){
		disp_str_gl_info += QString("\n  Can use Multitexture ");
	}else{
		disp_str_gl_info += QString("\n  Can not use Multitexture ");
	}


	if(canUseGlSl){
		disp_str_gl_info += QString("\n  Can use shader ");
	}else{
		disp_str_gl_info += QString("\n  Can not use shader ");
		disp_str_gl_info += COpenGLExt::getGlExtSupported().c_str();
	}

	disp_str_gl_info += "\n";
	disp_str_gl_info += glExtStr.c_str();
	ui.textBrowser->setText(disp_str_gl_info);
#endif
	QString disp_str  ;
	QString ver_info = DCMAPP_VERSION_STRING;
	ver_info = QString("           PXSDcmViewer [ Ver") + ver_info + QString(" ] \n");
	ver_info +=  QString("       Copyright 2012 PreXion Co., Ltd.  All Rights Reserved."); 
 

	ui.gpuInfo->setText(ver_info);
 
	 
}
 
 
  