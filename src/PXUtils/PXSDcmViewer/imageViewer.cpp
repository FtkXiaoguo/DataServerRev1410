#include "imageViewer.h"

 
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

#if 0
inline float checkPanX(const CTexVieweParam &inParam, float delta_panX)
{
	return delta_panX;
	//giveup
	 float v_imageSizeX = inParam.m_imageSizeX*inParam.m_zoom;
	 
	 bool check_end =false;
	 float ret_pan = 0.0f;
//	 if(delta_panX<0.0){
		 if(inParam.m_tex_x1<1.0f){
		 //チェック不要
	//		 return delta_panX;
		 }
		 //右側のはみだし量
		 float new_tex_x1 = inParam.m_tex_x1-delta_panX;
		 float Ver_outsize_p = inParam.m_ViewSizeX*(inParam.m_ver_x1+1.0f)*0.5f-inParam.m_ViewSizeX;
		 float Tex_outsize_p = new_tex_x1*v_imageSizeX - v_imageSizeX;
		 ret_pan = (Tex_outsize_p - Ver_outsize_p )/v_imageSizeX;
		if(ret_pan <0.0f){
			ret_pan = delta_panX;
		}else{
		 	ret_pan += delta_panX;//ret_pan = 0.0f;
			check_end = true;
		}
		 
//	 }else{
	if(!check_end){
		 if(inParam.m_tex_x0>0.0f){
		 //チェック不要
	//		 return delta_panX;
		 }
		 //左側のはみだし量
		 float new_tex_x0 = inParam.m_tex_x0-delta_panX;
		 float Ver_outsize_n = inParam.m_ViewSizeX*(inParam.m_ver_x0+1.0f)*0.5f;
		 float Tex_outsize_n = new_tex_x0*v_imageSizeX; 
		 ret_pan = (Tex_outsize_n - Ver_outsize_n)/v_imageSizeX;
		if(ret_pan >0.0f){
			ret_pan = delta_panX;
		}else{
		 	ret_pan += delta_panX;
		}
	 }
	
	return ret_pan;
}
inline float checkPanY(const CTexVieweParam &inParam, float delta_panY)
{
	return delta_panY;
	//giveup

	 float v_imageSizeY = inParam.m_imageSizeY*inParam.m_zoom;
	 
	 bool check_end =false;

	 float ret_pan = 0.0f;
	// if(delta_panY<0.0){
		 if(inParam.m_tex_y1<1.0f){
		 //チェック不要
	//		 return delta_panY;
		 }
		//下側のはみだし量
		 float new_tex_y1 = inParam.m_tex_y1-delta_panY;
		 
		 float Ver_outsize_p = inParam.m_ViewSizeY*(inParam.m_ver_y1+1.0f)*0.5f-inParam.m_ViewSizeY;
		 float Tex_outsize_p = new_tex_y1*v_imageSizeY - v_imageSizeY;
		 ret_pan = (Tex_outsize_p - Ver_outsize_p)/v_imageSizeY;
		 if(ret_pan <0.0f){
			ret_pan = delta_panY;
		}else{
			ret_pan += delta_panY;
			check_end = true;
		}
//	 }else{
	if(!check_end){
		 if(inParam.m_tex_y0>0.0f){
		 //チェック不要
//			 return delta_panY;
		 }
		 //上側のはみだし量
		 float new_tex_y0 = inParam.m_tex_y0-delta_panY;
		 float Ver_outsize_n = inParam.m_ViewSizeY*(inParam.m_ver_y0+1.0f)*0.5f;
		 float Tex_outsize_n = new_tex_y0*v_imageSizeY; 
		 ret_pan = (Tex_outsize_n - Ver_outsize_n)/v_imageSizeY;
		 if(ret_pan >0.0f){
			ret_pan = delta_panY;
		}else{
			ret_pan += delta_panY;
		}
	 }
	
	return ret_pan;
}
 
#endif


CanvasWidget::CanvasWidget(QWidget *parent ): 
m_ImageProcessor(0)
,m_paintFlag(false)
,m_imageBuffer(0)
,m_mainWindow(0)
,m_curFilterNo(0)
{
	 
	m_TexViewParam.m_zoom = 1.0;
	m_TexViewParam.m_panX = m_TexViewParam.m_panY = 0.0;
//	m_openGLOpCreated = false;

 
	 
	m_filterProc = false;

	int xx;
// 	setFixedSize(200, 200);
    
	m_TexViewParam.m_imageSizeX = 0;
	m_TexViewParam.m_imageSizeY = 0;
	
	m_imageQImageBuffer = 0;
	//
	m_lutBuffer = 0;
	m_lutCurveBuffer = 0;
	m_lutQImageBuffer  = 0;

	m_lutSize = 1024;
	m_ww = m_lutSize;
	m_wl = m_lutSize/2;
 
	////
	m_procImageBuffer = 0;

	 //
	  m_EvtProcHander =  new CMouseEvtHander;
	 
	  ///////////
	  CMouseEvtProcContrast *ProcContrast = new CMouseEvtProcContrast();
	  ProcContrast->setupOwner(this);
	  m_EvtProcHander->addMouseEvtProc(CMouseEvtHander::MouseEvtHand_Contrast,ProcContrast);
	  m_EvtProcHander->setCurrentMouseEvtMode(CMouseEvtHander::MouseEvtHand_Contrast);
	  

	  ///////////
	  CMouseEvtProcZoom *ProcZoom = new CMouseEvtProcZoom();
	  ProcZoom->setupOwner(this);
	  m_EvtProcHander->addMouseEvtProc(CMouseEvtHander::MouseEvtHand_Zoom,ProcZoom);
	  m_EvtProcHander->setCurrentMouseEvtMode(CMouseEvtHander::MouseEvtHand_Zoom);
	  ///
	  
	  ///////////
	  CMouseEvtProcPan *ProcPan = new CMouseEvtProcPan();
	  ProcPan->setupOwner(this);
	  m_EvtProcHander->addMouseEvtProc(CMouseEvtHander::MouseEvtHand_Pan,ProcPan);
	  m_EvtProcHander->setCurrentMouseEvtMode(CMouseEvtHander::MouseEvtHand_Pan);

	  ///////////
	  CMouseEvtProcCursor *ProcCursor = new CMouseEvtProcCursor();
	  ProcCursor->setupOwner(this);
	  m_EvtProcHander->addMouseEvtProc(CMouseEvtHander::MouseEvtHand_Cursor,ProcCursor);
	  m_EvtProcHander->setCurrentMouseEvtMode(CMouseEvtHander::MouseEvtHand_Cursor);


	  ///
	  m_curImagePixelinfo = new PxImagePixel ;
	  m_CurrentDicomInfo  = new PxDicomInfor;
}
 CanvasWidget::~CanvasWidget()
{
}
void CanvasWidget::setCurEventProcMode(CMouseEvtHander::MouseEvtHand_Mode eventProcMode)
{
	m_EvtProcHander->setCurrentMouseEvtMode(eventProcMode);

}
void CanvasWidget::destroyAll()
{
 
	if(m_EvtProcHander){
		delete m_EvtProcHander;
		m_EvtProcHander = 0;
	}
	if(m_curImagePixelinfo){
		delete m_curImagePixelinfo;
		m_curImagePixelinfo = 0;
	}
	if(m_CurrentDicomInfo){
		delete m_CurrentDicomInfo;
		m_CurrentDicomInfo = 0;
	}
	
	if(m_imageBuffer) {
		delete [] m_imageBuffer;
		m_imageBuffer = 0;
	}
	if(m_procImageBuffer){
		if(m_procImageBuffer) delete [] m_procImageBuffer;
		m_procImageBuffer = 0;
	}

	if(m_lutCurveBuffer) {
		delete [] m_lutCurveBuffer;
		m_lutCurveBuffer = 0;
	}

	if(m_lutBuffer) {
		delete [] m_lutBuffer;
		m_lutBuffer = 0;
	}

}
 
  
#define Text1D_MAX (1024*8)
void CanvasWidget::setupLut(int size)
{
	//1次元テクスチャ作成前にglDisableでGL_TEXTURE_2Dを無効

	int size_temp = size;
	size_temp = (size/2)*2;

	m_lutSize = size_temp;

	if(m_lutSize>Text1D_MAX){
		m_lutSize = Text1D_MAX;
	}
 
}

void CanvasWidget::viewImage(const PxImagePixel *image ,const PxDicomInfor *dicomInfo)
{
	*m_CurrentDicomInfo = *dicomInfo;
	*m_curImagePixelinfo = *image;
	m_curImagePixelinfo->m_pixelData = 0;//just dicom infor

	if(m_imageBuffer) delete [] m_imageBuffer;
	m_TexViewParam.m_imageSizeX = image->m_sizeX;
	m_TexViewParam.m_imageSizeY = image->m_sizeY;
	m_imageBuffer = new float [m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY*image->m_samplesPerPixel ];
	float *data_p = (float*)m_imageBuffer;

	setupDrawText();

//	if(!m_openGLOpCreated){
	if(1){
	// process buffer
		if(m_procImageBuffer) delete [] m_procImageBuffer;
		m_procImageBuffer = new float [m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY*image->m_samplesPerPixel ];
	}

	m_Pixel_Gain = 256.0f;
	m_Pixel_Offset  = 0.0f;
	if(image->m_samplesPerPixel == 3){
		//
	}else{
		if(image->m_bits==8){
			
			unsigned char *p_pixel_src = image->m_pixelData;

 
			int all_size = m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY*image->m_samplesPerPixel;
			for(int i=0;i<all_size;i++){
				data_p[ i ] =  p_pixel_src[i]/256.0f;
			}
 
			
			m_displayPixelMax = 256.0f;

			m_Pixel_Gain = 256.0f;
			m_Pixel_Offset  = 0.0f;
		}else
		if(image->m_bits>8)
		{
			if(image->m_pixelRepresentation == 1){
				int used_bits_max  = (1<<image->m_bits) ;
				
				if(image->m_hu_offset>0.0){
				//means: 12Bit CT
					used_bits_max  = (1<<12) ;
				}
				m_displayPixelMax = (float)used_bits_max;
				float gain = 1.0/(float)used_bits_max;
				short *p_pixel_src = ( short *)image->m_pixelData;
				//
				 
				float offset_val = image->m_rescaleIntercept*gain ;
				offset_val += image->m_hu_offset*gain;;
				//
				gain *=image->m_rescaleSlope;
				
				 
				for(int y_i=0;y_i<m_TexViewParam.m_imageSizeY;y_i++){
					for(int x_i=0;x_i<m_TexViewParam.m_imageSizeX;x_i++){
						int index =  (y_i*m_TexViewParam.m_imageSizeX + x_i);
						float f_temp = p_pixel_src[index]*gain + offset_val;
						if(f_temp <0.0f) f_temp = 0.0f;
						if(f_temp >1.0f) f_temp = 1.0f;
						data_p[ index ] =  f_temp;
						 
					}
				}

				m_Pixel_Gain	= 1.0f/gain ;
				m_Pixel_Offset  = -offset_val*m_Pixel_Gain;

			}else{
				int used_bits_max  = (1<<image->m_bits) ;
				
				m_displayPixelMax = (float)used_bits_max;

				float gain = 1.0/(float)used_bits_max;
				unsigned short *p_pixel_src = ( unsigned short *)image->m_pixelData;

				float offset_val = image->m_rescaleIntercept*gain ;
				offset_val += image->m_hu_offset*gain;;
				//
				gain *=image->m_rescaleSlope;
				
				 
				for(int y_i=0;y_i<m_TexViewParam.m_imageSizeY;y_i++){
					for(int x_i=0;x_i<m_TexViewParam.m_imageSizeX;x_i++){
						int index =  (y_i*m_TexViewParam.m_imageSizeX + x_i);

						float f_temp = p_pixel_src[index]*gain + offset_val;

						if(f_temp <0.0f) f_temp = 0.0f;
						if(f_temp >1.0f) f_temp = 1.0f;
						data_p[ index ] =  f_temp;

						 
					}
				}
				m_Pixel_Gain	= 1.0f/gain ;
				m_Pixel_Offset  = -offset_val*m_Pixel_Gain-image->m_hu_offset;
			}
			

		}
	}

	setupImageData();
 

}
void CanvasWidget::setLutProc()
{
	 
	doFilter(m_curFilterNo);
 	
}
void CanvasWidget::makeLutCurve(int no)
{
	//setup lut
	//lut size is fixed  1024
	// because can not create Texurure1D size > 8*1024 ??
	 

 
	
	if(m_lutCurveBuffer) delete [] m_lutCurveBuffer;
	m_lutCurveBuffer =	new float[m_lutSize];

	if(m_lutBuffer) delete [] m_lutBuffer;
	m_lutBuffer =	new float[m_lutSize];


 
	float *p_data = (float*)m_lutCurveBuffer;
	float lut_gain = 1.0/(float)(m_lutSize);

 
	switch(no)
	{
	case LutType_Linear:
		{
			 
			for(int i=0;i<m_lutSize ;i++){
				 
				float vv = i*lut_gain;
				p_data[i]	=     vv;
			}
		}
		break;

	case LutType_Gama:
		{
			double gama_alpha  = 1.0/2.8;
			for(int i=0;i<m_lutSize ;i++){
				 
				float vv = i*lut_gain;
				 
				p_data[i]	=    pow((double)vv,gama_alpha);
				 
			}
		};
		break;
	case LutType_GamaRev:
		{
			double gama_alpha  =  2.8;
			for(int i=0;i<m_lutSize ;i++){
				 
				float vv = i*lut_gain;
				 
				p_data[i]	=    pow((double)vv,gama_alpha);
				 
			}
		}
		break;
	case LutType_GamaGama:
		{

	 
			double gama_alpha  = 1.0/2.8;

			int p1,p2;
			float p_v1 = 0.4f;
			float p_v2 = 0.6f;
			//
			for(int i=0;i<m_lutSize ;i++){
				 
				float vv = i*lut_gain;
				 
				p_data[i]	=    pow((double)vv,gama_alpha);
				if(p_data[i] >= p_v1){
					p1 = i;
					break;
				}
			}
			for(int i=m_lutSize-1;i>0 ;i--){
				 
				float vv = i*lut_gain;
				 
				p_data[i]	=  1.0-pow((double)(1.0-vv),gama_alpha);
				if(p_data[i] <= p_v2){
					p2 = i;
					break;
				}
			}
			//
			float dd = (p_v2-p_v1)/(p2-p1);
			for(int i=p1;i<p2 ;i++){
				 
				float vv = (i-p1)*dd + p_v1;
				 
				p_data[i]	=     vv ;
				 
			}
		}
		break;
	case LutType_GamaRevGamaRev:
		{ 
			double gama_alpha  =  1.8;

			int p1,p2;
			float p_v1 = 0.2f;
			float p_v2 = 0.8f;
			//
			for(int i=0;i<m_lutSize ;i++){
				 
				float vv = i*lut_gain;
				 
				p_data[i]	=    pow((double)vv,gama_alpha);
				if(p_data[i] >= p_v1){
					p1 = i;
					break;
				}
			}
			for(int i=m_lutSize-1;i>0 ;i--){
				 
				float vv = i*lut_gain;
				 
				p_data[i]	=  1.0-pow((double)(1.0-vv),gama_alpha);
				if(p_data[i] <= p_v2){
					p2 = i;
					break;
				}
			}
			//
			float dd = (p_v2-p_v1)/(p2-p1);
			for(int i=p1;i<p2 ;i++){
				 
				float vv = (i-p1)*dd + p_v1;
				 
				p_data[i]	=     vv ;
				 
			}
		}
		break;
	}

 
	makeLut();
}

void CanvasWidget::setupViewSize()
{
 
	m_TexViewParam.m_ViewOrgX  = 2;
	m_TexViewParam.m_ViewOrgY  = 2;
	m_TexViewParam.m_ViewSizeX = getWidth()-4;
    m_TexViewParam.m_ViewSizeY = getHeight()-4;


	if( (m_TexViewParam.m_imageSizeX<1) ||
		(m_TexViewParam.m_imageSizeY<1)
		){
			return;
	}

 
}
void CanvasWidget::changeZoomRate(float zoom_alfa)
{
	float saved_zoom = m_TexViewParam.m_zoom  ;

	m_TexViewParam.m_zoom += (zoom_alfa);

	
	if(m_TexViewParam.m_zoom < 0.2){
		m_TexViewParam.m_zoom = saved_zoom;
	}

	updateViewer();
}
void CanvasWidget::chgZoom(QPoint delta )
{
#if 0
	int w = size().width();
	int h = size().height();
#else
	int w = getWidth();
	int h = getHeight();
#endif

	float geo_rate = w;
	if(h<w) {
		geo_rate = h;
	}
	float zoomX = delta.rx()/geo_rate;  
	float zoomY = delta.ry()/geo_rate;


	float zoom_alfa = zoomY*m_TexViewParam.m_zoom ;
	if(zoomX*zoomX > zoomY*zoomY){
		zoom_alfa = zoomX*m_TexViewParam.m_zoom ;
	} 

#if 1
	changeZoomRate(zoom_alfa);
#else	
	float saved_zoom = m_TexViewParam.m_zoom  ;

	m_TexViewParam.m_zoom += (zoom_alfa);

	
	if(m_TexViewParam.m_zoom < 0.2){
		m_TexViewParam.m_zoom = saved_zoom;
	}

	updateViewer();
#endif
}
void CanvasWidget::chgPan(QPoint delta)
{
#if 0
	int w = size().width();
	int h = size().height();
#else
	int w = getWidth();
	int h = getHeight();
#endif

	float geo_rate_w = w;
	float geo_rate_h = h;
	 
//	float delta_panX = delta.rx()/m_TexViewParam.m_zoom/geo_rate;  
//	float delta_panY = -delta.ry()/m_TexViewParam.m_zoom/geo_rate;

	float delta_panX = delta.rx() /geo_rate_w;  
	float delta_panY = -delta.ry() /geo_rate_h;


//	delta_panX = checkPanX(m_TexViewParam, delta_panX);
 //	delta_panY = checkPanY(m_TexViewParam, delta_panY);



	m_TexViewParam.m_panX -= delta_panX;
	m_TexViewParam.m_panY -= delta_panY;

	updateViewer();
	 
}

void CanvasWidget::chgContrast(QPoint delta)
{

	float geo_rate = 400.0;
	 
	float deltaX = delta.rx()/geo_rate;  
	float deltaY = delta.ry()/geo_rate;

	float new_ww = m_ww*(1.0f+deltaX);
	float new_wl = m_wl*(1.0f+deltaY);

	// setupWWWL(new_ww,new_wl);

	 if(m_mainWindow){
		 m_mainWindow->notifyWWWL(new_ww,new_wl);
	 }
 
//	 updateViewer();

}
void CanvasWidget::onChgCursorStatus()
{
	 if(m_mainWindow){
		 m_mainWindow->chgCursorStatus();
	 }
}
void CanvasWidget::coordXY(int wndX,int wndY, int &imageX,int &imageY)
{
#if 1 //not yet

 
	// Viewer <-> Window  比例不変
	//Windows Postionそのまま
	//Viewerのサイズ変更のみ（仮想）
	float disp_x = wndX;
	float disp_y = wndY;
 

	float disp_ver_w = m_TexViewParam.m_ver_x1-m_TexViewParam.m_ver_x0;
	float disp_ver_h = m_TexViewParam.m_ver_y1-m_TexViewParam.m_ver_y0;

 
	//Viewer全領域　<-> vertex [-1,1]
	float ver_x = (float)(disp_x-m_TexViewParam.m_ViewOrgX)*2.0f/(m_TexViewParam.m_ViewSizeX) - 1.0f;
	float ver_y = (float)(disp_y-m_TexViewParam.m_ViewOrgY)*2.0f/(m_TexViewParam.m_ViewSizeY) - 1.0f;
 

	float disp_tex_w = m_TexViewParam.m_tex_x1-m_TexViewParam.m_tex_x0;
	float disp_tex_h = m_TexViewParam.m_tex_y0-m_TexViewParam.m_tex_y1;//<- up/Down

	/////////////////
	float tex_x = (ver_x-m_TexViewParam.m_ver_x0)*disp_tex_w/disp_ver_w +  m_TexViewParam.m_tex_x0;
	float tex_y = (-ver_y-m_TexViewParam.m_ver_y1)*disp_tex_h/disp_ver_h +  m_TexViewParam.m_tex_y0;
	//-------------^^--------------------------^^---up/Down----

	//Textur <-> Image  全領域
	imageX = tex_x*m_TexViewParam.m_imageSizeX;
	imageY = tex_y*m_TexViewParam.m_imageSizeY;
 
#endif
}
void CanvasWidget::onCursorMove(QPoint point)
{
	if(!m_curImagePixelinfo) return;
	if( (m_curImagePixelinfo->m_sizeX<1) || (m_curImagePixelinfo->m_sizeY<1)) return;

	 
	int imageX,imageY;
	coordXY(point.x(),point.y(),imageX,imageY);

	QString str_temp;

	signed short  ret_pixel;
	
	if(getPixelData(imageX,imageY, ret_pixel)){

		str_temp = QString("(%1,  %2), %3").arg(
								QString::number(imageX),QString::number(imageY),QString::number(ret_pixel) );
	}else{
		str_temp = QString("(%1,  %2) ----").arg(
								QString::number(imageX),QString::number(imageY) );
	}

	{
		int space_size = 17-str_temp.size();
		for(int i=0;i<space_size;i++){
			str_temp = QString(" ") + str_temp;
		}
	}
	if(m_mainWindow){
		m_mainWindow->setupStatusBarMsg(str_temp,MainWindow::StatusIndex_DispCursor);
	} 

}
void CanvasWidget::onWheelEvent(int d)
{
	int step = d >0 ? 1:-1;
	if(m_mainWindow){
		 m_mainWindow->onFrameStep(step);
	} 
}


std::string _filterName[] = {
	"NoneFilter",
	"filter1",
	"filter2",
};
bool CanvasWidget::createFilters()
{
#if 0
	if(!m_ImageProcessor) {
		if(m_useShader){
			m_ImageProcessor = new CImageProcShader((COpenGLShader*)m_openGLOp);
		}else{
			m_ImageProcessor = new CImageProcCPU;
		}
		
	}
#endif
	if(!m_ImageProcessor->createFilter(_filterName[0],CImageProcBase::ImageFilter_None)){
		return false;
	}
	if(!m_ImageProcessor->createFilter(_filterName[1],CImageProcBase::ImageFilter_FIR)){
		return false;
	}
	if(!m_ImageProcessor->createFilter(_filterName[2],CImageProcBase::ImageFilter_BiLateral)){
		return false;
	}

	return true;
}
void CanvasWidget::doFilter(int filter_no)
{
//	return ;
	m_curFilterNo = filter_no;

 
	if(m_curImagePixelinfo->m_samplesPerPixel == 3){
		//
	}else{
		if(m_curFilterNo>=0){
			m_ImageProcessor->selCurrentFilter(_filterName[m_curFilterNo]);
			 
			m_ImageProcessor->setWWWL(m_ww,m_wl,m_displayPixelMax);
		 
			CImageFilter *filter_proc = m_ImageProcessor->getCurrentFilter();
			if(filter_proc){
				filter_proc->setWWWL(m_ww,m_wl,m_displayPixelMax);
			 
			}

		} 

	 
 
	}
 
}

 
void CanvasWidget::makeLut()
{
	if(!m_lutBuffer) return;

#ifdef USE_SHARDER_LUT
	 

	float val_max = m_displayPixelMax ;//m_lutSize;


	float cal_ww = m_ww/val_max*m_lutSize;
	float cal_wl = m_wl/val_max*m_lutSize;

	float cal_k = 1.0/cal_ww;
	float cal_b = cal_wl-cal_ww/2.0f ;

	 
	for(int i=0;i<m_lutSize;i++){
		float lut_r = cal_k*(i - cal_b);

		if(lut_r<0.0f) lut_r = 0.0f;
		if(lut_r>0.99999999999f) lut_r = 0.99999999999f;

		m_lutBuffer[i] =   lut_r;//m_lutCurveBuffer[i]*lut_r;
	}

	bool dbg_flag = false;
	if(dbg_flag){
		FILE *fp = fopen("dbg_lut.csv","wt");
		fprintf(fp,"No., lut \n");

		float *p_data = (float*)m_lutBuffer;
		for(int i=0;i<m_lutSize ;i++){
			fprintf(fp,"%d, %.6f \n",i,p_data[i]);
		}
		fclose(fp);

	}
#endif
 
}
void CanvasWidget::setupWWWL(float ww,float wl)
{
	m_ww = ww;
	m_wl = wl;

	makeLut();
 
//	updateViewer();

	setLutProc();
	
}

bool CanvasWidget::calImageHist(const PxImagePixel *image,CHistogramData *makeHist )
{
 

	if(image->m_samplesPerPixel == 3){
		//
	}else{
		if(image->m_bits==8){
			if(makeHist){
				makeHist->createHist(256);
			}
			unsigned char *p_pixel_src = image->m_pixelData;

			if(makeHist){
				makeHist->m_totalPixels = m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY;

				if(image->m_samplesPerPixel == 4){
					for(int y_i=0;y_i<m_TexViewParam.m_imageSizeY;y_i++){
						for(int x_i=0;x_i<m_TexViewParam.m_imageSizeX;x_i++){
							int index =  (y_i*m_TexViewParam.m_imageSizeX + x_i);
							unsigned char data;
							data = p_pixel_src[4*index + 0];
							makeHist->m_hist[data] ++;
						 	data = p_pixel_src[4*index + 1];
						 	makeHist->m_hist[data] ++;
						 	data = p_pixel_src[4*index + 2];
						 	makeHist->m_hist[data] ++;
						//	 
							 
						}
					}

				}else{
					for(int y_i=0;y_i<m_TexViewParam.m_imageSizeY;y_i++){
						for(int x_i=0;x_i<m_TexViewParam.m_imageSizeX;x_i++){
							int index =  (y_i*m_TexViewParam.m_imageSizeX + x_i);
							unsigned char data = p_pixel_src[index];
							makeHist->m_hist[data] ++;
							 
							 
						}
					}
				}
			}
		}else
		if(image->m_bits>8)
		{
			if(image->m_pixelRepresentation == 1){
				int used_bits_max  = (1<<image->m_bits) ;
				
				if(image->m_hu_offset>0.0){
				//means: 12Bit CT
					used_bits_max  = (1<<12) ;
				}
				float gain = 1.0/(float)used_bits_max;
				short *p_pixel_src = ( short *)image->m_pixelData;
				//
				 
				float offset_val = image->m_rescaleIntercept*gain ;
				offset_val += image->m_hu_offset*gain;;
				//
				gain *=image->m_rescaleSlope;
				
				if(makeHist){
					makeHist->createHist(used_bits_max);
					makeHist->m_totalPixels = m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY;
				}
				for(int y_i=0;y_i<m_TexViewParam.m_imageSizeY;y_i++){
					for(int x_i=0;x_i<m_TexViewParam.m_imageSizeX;x_i++){
						int index =  (y_i*m_TexViewParam.m_imageSizeX + x_i);
						float hist_val =  p_pixel_src[index]*gain + offset_val;
						 
						if(makeHist){
							int index_no = hist_val*used_bits_max;
							if(index_no<0) index_no = 0;
							if(index_no>=used_bits_max) index_no = used_bits_max-1;
							makeHist->m_hist[index_no] ++;
						}
					}
				}

			}else{
				int used_bits_max  = (1<<image->m_bits) ;
				
				float gain = 1.0/(float)used_bits_max;
				short *p_pixel_src = ( short *)image->m_pixelData;

				float offset_val = image->m_rescaleIntercept*gain ;
				offset_val += image->m_hu_offset*gain;;
				//
				gain *=image->m_rescaleSlope;
				
				if(makeHist){
					makeHist->createHist(used_bits_max);
					makeHist->m_totalPixels = m_TexViewParam.m_imageSizeX*m_TexViewParam.m_imageSizeY;
				}

				for(int y_i=0;y_i<m_TexViewParam.m_imageSizeY;y_i++){
					for(int x_i=0;x_i<m_TexViewParam.m_imageSizeX;x_i++){
						int index =  (y_i*m_TexViewParam.m_imageSizeX + x_i);
						float hist_val =  p_pixel_src[index]*gain + offset_val;

						if(makeHist){
							int index_no = hist_val*used_bits_max;
							if(index_no<0) index_no = 0;
							if(index_no>=used_bits_max) index_no = used_bits_max-1;
							makeHist->m_hist[index_no] ++;
						}
						 
					}
				}
			}
			

		}
	}
	return true;
}

bool CanvasWidget::getPixelData(int pos_x,int pos_y,signed short  &ret_pixel) const
{
	if(!m_imageBuffer) return false;

	if(	(pos_x<0) ||(pos_y<0) ||
		(pos_x>=m_TexViewParam.m_imageSizeX) || (pos_y>=m_TexViewParam.m_imageSizeY) ){
			return false;
	}

	ret_pixel = (signed short)(m_imageBuffer[pos_y*m_TexViewParam.m_imageSizeX + pos_x] * m_Pixel_Gain + m_Pixel_Offset);
	return true;
}

void CanvasWidget::resetViewerSize()
{
	setupViewSize();

	float dispRateX = (float)m_TexViewParam.m_ViewSizeX/m_TexViewParam.m_imageSizeX;
	float dispRateY = (float)m_TexViewParam.m_ViewSizeY/m_TexViewParam.m_imageSizeY;

	m_TexViewParam.m_zoom = dispRateX;
	if(dispRateY<dispRateX){
		m_TexViewParam.m_zoom = dispRateY;
	}

	m_TexViewParam.m_panX = 0.0f;
	m_TexViewParam.m_panY = 0.0f;

}