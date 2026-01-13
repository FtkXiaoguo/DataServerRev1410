#ifndef IMAGE_VIEWER_GUI__H
#define IMAGE_VIEWER_GUI__H
 
 


#include "mouseEvtProc.h"

class CTexVieweParam
{
public:
	int m_imageSizeX;
	int m_imageSizeY;
	//
 
 
	int m_ViewOrgX;
	int m_ViewOrgY;
	int m_ViewSizeX;
	int m_ViewSizeY;
 
	 
	//
	float m_zoom;
	float m_panX;
	float m_panY;

	///
	float m_tex_x0;
	float m_tex_x1;
	float m_tex_y0;
	float m_tex_y1;
	//
	float m_ver_x0;
	float m_ver_x1;
	float m_ver_y0;
	float m_ver_y1;
};

class QGLPixelBuffer;
class COpenGLExt;

class PxImagePixel;
class PxDicomInfor;

class CImageProcBase;
class CHistogramData;
class MainWindow;

class CanvasWidget : public  CMouseEvtProcOwner
{
 
public:
	enum LutType {
		LutType_Linear,
		LutType_Gama,
		LutType_GamaRev,
		LutType_GamaGama,
		LutType_GamaRevGamaRev,
	};

    explicit CanvasWidget(QWidget *parent = 0 );
	virtual ~CanvasWidget();

	virtual void destroyAll();

	virtual void myUpdate()=0;
	virtual void mySetMouseTracking(bool on)=0;
 
	void setupWWWL(float ww,float wl);
	void setFilterProc(bool filterProc){ m_filterProc = filterProc;};
	virtual void setLutProc();

	void setupLut(int size);
	void viewImage(const PxImagePixel *image,const PxDicomInfor *dicomInfo);
	bool calImageHist(const PxImagePixel *image,CHistogramData *makeHist );

	virtual void myResize(QSize &size) = 0;
	void makeLutCurve(int no);

//	bool createShader();
	virtual void updateViewer()=0;

	void resetViewerSize();
	//
	virtual bool createFilters() ;
	virtual void doFilter(int no) ;
	//
	void enablePaint(bool flag){
		m_paintFlag = flag;
	};
	//
	int getHistogram(float *histBuffer);
	//
	void setCurEventProcMode(CMouseEvtHander::MouseEvtHand_Mode eventProcMode);

	void setupMainWindow(MainWindow *wnd) { m_mainWindow = wnd;};
	////////
	virtual void chgZoom(QPoint delta ) ;
  	virtual void chgContrast(QPoint delta) ;
	virtual void chgPan(QPoint delta) ;

	virtual void onCursorMove(QPoint point);
	virtual void onWheelEvent(int d) ;
	virtual void onChgCursorStatus();
	/////////
	void changeZoomRate(float zoom_alfa);
signals:
public slots:
protected:
	virtual int getWidth() =0;
	virtual int getHeight() =0;

	
//	virtual void paintEvent(QPaintEvent *);

	
	/////////
	void setupViewSize();
	
//	virtual void initCommon();
	virtual void setupImageData() = 0;
	virtual void setupDrawText() = 0;

	void coordXY(int wndX,int wndY, int &imageX,int &imageY);
	
	bool getPixelData(int x,int y,signed short  &ret_pixel) const;
	/////
	
	/////
 
	float *m_imageBuffer;//data;
	float *m_procImageBuffer;
	QImage *m_imageQImageBuffer;
	//
	float m_Pixel_Gain;
	float m_Pixel_Offset;

	CTexVieweParam m_TexViewParam;
 
	
	
	float *m_lutCurveBuffer;
	float *m_lutBuffer;
	QImage *m_lutQImageBuffer;
	 int m_lutSize;
	

	//
	bool m_filterProc;
	//
//	bool m_openGLOpCreated;

protected:
	virtual void makeLut();
	float	m_ww;
	float	m_wl;// [0,...]  +m_disp_HU_val Ç≥ÇÍÇΩÇ‡ÇÃÅB
	
	int m_curFilterNo;

	CMouseEvtHander *m_EvtProcHander;
	
	//
//	float m_zoom;
//	float m_pan[2];
	//
	PxImagePixel *m_curImagePixelinfo;
	PxDicomInfor *m_CurrentDicomInfo;

	CImageProcBase *m_ImageProcessor;
	//
	bool m_paintFlag;
	//
	 
	MainWindow *m_mainWindow;

	float m_displayPixelMax;// [0,max]
	//

	 

};
 



#endif //IMAGE_VIEWER_GUI__H


