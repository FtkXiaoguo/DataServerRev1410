 
#include "mainwindow.h"
#include <QtGui/QProgressDialog>
 
#include <QtGui>
#include <QPrinter>
 #include <QtCore/QTimer>
 #include "AboutInfo.h"
 
 
#include "dispHist.h"

#include "assert.h"
 
#include <qdir.h>
#include <qfileinfo.h>
#include <qlist.h>

#include "imageViewerOpenGL.h"

#include "imageViewerGDI.h"

#define  DefRegSetting  RegSetting("PreXion","TstOpenGLViewer");
#define DefaultDir "dataFolder"

extern QString g_dicom_file_path;
extern QString g_dicom_file_list;
extern QString g_dicom_series_folder;
extern QString g_dicom_file;
extern int g_enable_menu;
static CanvasWidget::LutType  g_lut_tbl_[] = 
{
	CanvasWidget::LutType_Linear,
	CanvasWidget::LutType_Gama,
	CanvasWidget::LutType_GamaRev,
	CanvasWidget::LutType_GamaGama,
	CanvasWidget::LutType_GamaRevGamaRev,
};
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
 
{
	m_disp_HU_val = 0.0f;

	m_settingWWWL_flag = false;

	m_CanvasWidgetOpenGLTemp = 0;
	m_CanvasWidgetGLShaderTemp = 0;

	m_multiFrameFlag = false;
	m_newSeriesFlag = false;
	m_internalGUIChgFlag = false;
	m_CurrentPixelData = 0;
	m_CurrentDicomInfo = 0;
	m_filter_created_flag = false;
	m_first_show_flag = true;

	m_hist_data = new CHistogramData ;
	m_valid = false;
	 
	
	ui.setupUi(this);

	m_openGLWidget = 0;
 	initGLSL();

	ui.cbBFilter->addItem("none ");
	ui.cbBFilter->addItem("filter1");
	ui.cbBFilter->addItem("filter2");
static QIcon liear_icon(QPixmap(":/rcFiles/rcFiles/Linear.PNG"));	
static QIcon gama_icon(QPixmap(":/rcFiles/rcFiles/Gama.PNG"));	
static QIcon gamaRev_icon(QPixmap(":/rcFiles/rcFiles/GamaRev.PNG"));	
static QIcon gamaGama_icon(QPixmap(":/rcFiles/rcFiles/GamaGama.PNG"));	
static QIcon gamaRevGamaRev_icon(QPixmap(":/rcFiles/rcFiles/GamaRevGamaRev.PNG"));	
 

	//
 	ui.cbBLut->addItem(liear_icon,"linear");
	ui.cbBLut->addItem(gama_icon,"gama");
	ui.cbBLut->addItem(gamaRev_icon,"gamaRev");
	ui.cbBLut->addItem(gamaGama_icon,"gamaG");
	ui.cbBLut->addItem(gamaRevGamaRev_icon,"gamaRR");
	ui.cbBLut->setIconSize(QSize(100,34));
	
//	m_openGLWidget->resize(200,200);
 
  
	{
		static QIcon radio_contrast_icon(QPixmap(":/rcFiles/rcFiles/ContrastButton.PNG"));
		ui.rBContrast->setIcon(radio_contrast_icon);	

		static QIcon radio_pan_icon(QPixmap(":/rcFiles/rcFiles/PanButton.PNG"));
		ui.rBPan->setIcon(radio_pan_icon);	

		static QIcon radio_zoom_icon(QPixmap(":/rcFiles/rcFiles/ZoomButton.PNG"));
		ui.rBZoom->setIcon(radio_zoom_icon);	

		static QIcon radio_cursor_icon(QPixmap(":/rcFiles/rcFiles/CursorButton.PNG"));
		ui.rBCursor->setIcon(radio_cursor_icon);

	}

	{
		static QIcon fit_zoom_icon(QPixmap(":/rcFiles/rcFiles/FitZoomButton.PNG"));
		ui.pBFitZoom->setIcon(fit_zoom_icon);	

		static QIcon auto_wwwl_icon(QPixmap(":/rcFiles/rcFiles/AutoWWWLButton.PNG"));
		ui.pBAutoWWWL->setIcon(auto_wwwl_icon);	
		
		static QIcon histogram_icon(QPixmap(":/rcFiles/rcFiles/HistogramButton.PNG"));
		ui.pBHist->setIcon(histogram_icon);	
		
			
		
	}

	{
		 
		static QIcon step_left_icon(QPixmap(":/rcFiles/rcFiles/StepLeftButton.PNG"));
		ui.pBStepLeft->setIcon(step_left_icon);
		//
		static QIcon step_right_icon(QPixmap(":/rcFiles/rcFiles/StepRightButton.PNG"));
		ui.pBStepRight->setIcon(step_right_icon);


	}
	ui.cbBLut->hide();
 	ui.cbBFilter->hide();

	{
		QStatusBar *status_bar = statusBar();
	 
		for(int i=0;i<STATUS_FIELD_NUM;i++){
			 m_status_field[i] = new QLabel(status_bar);
			 if( (i==StatusIndex_DispFrame) ||
				 (i==StatusIndex_DispCursor)
				 ){
					m_status_field[i]->setFrameShape(QFrame::Box);
				}
		//	 m_status_field[i]->setText(QString::number(i));
			 m_status_field[i]->setText("         ");
			status_bar->addWidget(m_status_field[i]);
		}
 
	}

	ui.SliderFrame->setSingleStep(1);
	ui.SliderFrame->setPageStep(1);

	ui.rBCursor->setChecked(true);

	setupMultiFrameGUI(false);

	{
		setWindowTitle("PXSDcmViewer");
	}


	if(g_enable_menu ==0){
		ui.actionOpen->setDisabled(true);
		ui.actionOpenDir->setDisabled(true);
	}

	m_valid = true;


//	this->setFocusPolicy( Qt::StrongFocus);
	 
}
 
void MainWindow::initGLSL()
{
	if(m_openGLWidget) delete m_openGLWidget;

//	if(CanvasWidget::checkOpenGLShader()){
 
	m_CanvasWidgetGLShaderTemp = new CanvasWidgetOpenGL(ui.gBdrawArea  );

	 
	//まずはOpenGLでTry
	m_openGLWidget			= m_CanvasWidgetGLShaderTemp;
	 
 	 
	m_openGLWidget->setupMainWindow(this);
	m_openGLWidget->mySetMouseTracking(true);//for mouseMoveEvent
	//
	
}
 
#if 0
void MainWindow::change2OpenGL()
{
//	delete m_CanvasWidgetOpenGLTemp;

	m_CanvasWidgetOpenGLTemp	= new CanvasWidgetOpenGL(ui.gBdrawArea  );
	m_CanvasWidgetOpenGLTemp->setupMainWindow(this);
	m_CanvasWidgetOpenGLTemp->mySetMouseTracking(true);//for mouseMoveEvent
	
	m_openGLWidget			= m_CanvasWidgetOpenGLTemp;
}
#endif

void MainWindow::paintEvent(QPaintEvent *e /* event */)
{
 
	 QMainWindow::paintEvent(e);
	QStatusBar *status_bar = statusBar();
	if(status_bar){
		 for(int i=0;i<STATUS_FIELD_NUM;i++){
			 if(m_DispStatusBarMsg[i].size()>0){
				
	 				m_status_field[i]->setText(m_DispStatusBarMsg[i]);
		 
 			//		status_bar->showMessage(m_DispStatusBarMsg);
				 
			 }
		 }
	}

}
 


void MainWindow::resizeEvent(QResizeEvent *event)
{
	

	QMainWindow::resizeEvent(event);

	QSize size = ui.gBdrawArea->size();

	if(!m_valid) return ;
 		
	if( m_openGLWidget){
		m_openGLWidget->myResize(size);
	}
	 
	
}
#if 0
void MainWindow::onChkFilter()
{
	m_openGLWidget->setFilterProc(ui.chkFilter->checkState() == Qt::Checked);
	update();
	 

	m_openGLWidget->update();
}
#endif
void MainWindow::onChangeFilter(int no)
{
	if(!m_valid) return ;
	if(!m_openGLWidget) return ;
	 
//	m_openGLWidget->setFilterProc(no!=0);

	m_openGLWidget->doFilter(no );
	update();
	 

	m_openGLWidget->updateViewer();
	 
	int xx = no;
}
void MainWindow::onChangeLut(int no)
{

	if(!m_valid) return ;
	if(!m_openGLWidget) return ;

	if(	(no<0) ||
		(no>=(sizeof(g_lut_tbl_)/sizeof(CanvasWidget::LutType)))){
			return ;
	}
	m_openGLWidget->makeLutCurve(g_lut_tbl_[no]);

	m_openGLWidget->setLutProc();
	update();
	m_openGLWidget->updateViewer();
}

void MainWindow::onAppExit()
{
	close();
}
#include "PXDcmSAdmin/PxDcmStudysManage.h"

#define QString2Str(Qstr) std::string((Qstr).toLocal8Bit())

QString _g_dir;
void MainWindow::onFileOpen()
{
	if(!m_valid) return ;

 	 

	QSettings DefRegSetting

	if(_g_dir.isEmpty()){
		QString last_folder = RegSetting.value(DefaultDir).toString();
		if(last_folder.size()>1){
			_g_dir = last_folder;
		}
	}
	;;
	QString fileName = 
		_g_dir.size()>1 ? QFileDialog::getOpenFileName(this,"",_g_dir) : QFileDialog::getOpenFileName(this);

	if(fileName.size()<1) return;
	_g_dir = QFileInfo(fileName).path();

	hide();
	//
	m_newSeriesFlag  = true;
	openDicomFile(fileName);
	m_multiFrameFlag = false;
	setupMultiFrameGUI(false);

	show();

}

	 
void MainWindow::openDicomFile(const QString &dicomFile)
{
	 
	if(m_CurrentDicomInfo) delete m_CurrentDicomInfo;
	m_CurrentDicomInfo = new PxDicomInfor;

	CPxDcmStudysManage read_dcm;
//	PxImagePixel pixelData;
	if(m_CurrentPixelData) delete m_CurrentPixelData;
	m_CurrentPixelData = new PxImagePixel;
	read_dcm.loadImage(QString2Str(dicomFile), *m_CurrentPixelData,true/*rawdata*/ ,m_CurrentDicomInfo); 

	//m_openGLWidget->viewImage(m_CurrentPixelData, m_hist_data);
	m_openGLWidget->viewImage(m_CurrentPixelData,m_CurrentDicomInfo );
	if(!m_filter_created_flag){
		if(!m_openGLWidget->createFilters()){
			QMessageBox msgbox;
			msgbox.setIcon(QMessageBox::Critical);
			msgbox.setText("     createFilters error\n");
	 		msgbox.exec();
		}
		
		m_filter_created_flag = true;
	}
	
	
	if(m_newSeriesFlag){
		m_newSeriesFlag = false;

		 
		m_disp_HU_val = m_CurrentPixelData->m_hu_offset;

		int pixelMax = 1024*4;
		int ww = 1024*3;
		int wl = 1000;
		 
		pixelMax  = (1<<m_CurrentPixelData->m_bits) ;
				
		if(m_CurrentPixelData->m_hu_offset>0.0){
		//means: 12Bit CT
			pixelMax  = (1<<12) ;
		} 
		ww = pixelMax*3/4;
		wl = pixelMax/2;
		  
		if(m_CurrentPixelData->m_samplesPerPixel==4){
		//RGB
			pixelMax = 256;
			ww = 256;
			wl = ww/2;
		}
	//	ww = ww - m_disp_HU_val;
		wl = wl - m_disp_HU_val;

		if(m_CurrentPixelData->m_wl>0.0f){
			wl = m_CurrentPixelData->m_wl;
		}
		if(m_CurrentPixelData->m_ww>0.0f){
			ww = m_CurrentPixelData->m_ww;
		}
	 
	//	ww = ww + m_disp_HU_val;
	//	wl = wl + m_disp_HU_val;

	 	setupImageDisplay(ww,wl,pixelMax,m_disp_HU_val);

	//	setupImageDisplay((1024*3),1000,1024*4);
		m_openGLWidget->resetViewerSize();

	}else{
		m_openGLWidget->updateViewer();
	}
	
 	m_openGLWidget->enablePaint(true);

}
void  MainWindow::onFileOpenDir()
{
	if(!m_valid) return ;

	QSettings DefRegSetting

	if(_g_dir.isEmpty()){
		QString last_folder = RegSetting.value(DefaultDir).toString();
		if(last_folder.size()>1){
			_g_dir = last_folder;
		}
	}
	;;
	QFileDialog::Options option =	QFileDialog::ShowDirsOnly	|
									QFileDialog::ReadOnly		|
									QFileDialog::DontUseNativeDialog;
	QString dir_Name = 
		_g_dir.size()>1 ? QFileDialog::getExistingDirectory(this,"",_g_dir,option) : QFileDialog::getExistingDirectory(this,"","",option);

	if(dir_Name.size()<1) return;
	_g_dir = dir_Name;

  	hide();
	 
	openDicomFolder(dir_Name);
  	show();
 

}

void MainWindow::openDicomFolder(const QString &dicomDir)

{
		QDir dicom_list_dir(dicomDir);
	// 	dicom_list_dir.setFilter( QDir::Dirs | QDir::Files | QDir::Hidden ); // 隠しファイルも表示
		dicom_list_dir.setSorting( QDir::DirsFirst | QDir::Name ); // ソート順 
		 
		 QStringList filters;
		 filters << "*DCM" ;
		 dicom_list_dir.setNameFilters(filters);

		
		QStringList fileList= dicom_list_dir.entryList();

		if(fileList.size()<1) return;

		QStringList fullPath_fileList;
		
		for(int i=0;i<fileList.size();i++){
			QString filename_temp = dicomDir + "/"+fileList.at(i);
			fullPath_fileList.append(filename_temp);
		 
		}
		 openDicomSeries(fullPath_fileList);

	

}
void MainWindow::closeEvent(QCloseEvent *e)
{
	
	if(_g_dir.size()>1){
		QSettings DefRegSetting;

		RegSetting.setValue(DefaultDir,_g_dir);
	}

	QMainWindow::closeEvent(e);

	if(m_hist_data) delete m_hist_data;

#if 0
	if(m_openGLWidget){
		m_openGLWidget->destroyAll();
		delete m_openGLWidget;
	}
#else
	if(m_CanvasWidgetOpenGLTemp){
		delete m_CanvasWidgetOpenGLTemp;
	}
	if(m_CanvasWidgetGLShaderTemp){
		delete m_CanvasWidgetGLShaderTemp;
	}
#endif

	if(m_CurrentPixelData) delete m_CurrentPixelData;
 
	if(m_CurrentDicomInfo) delete m_CurrentDicomInfo;
	
	 


}
void MainWindow::initDcm()
{
	CPxDcmDbManage::initDcmtk();
}
void MainWindow::onInformation()
{
	AboutInfo info;
	info.exec();
}
void MainWindow::onChgMouse()
{
	setupStatusBarMsg("    "/*clear message*/,MainWindow::StatusIndex_DispCursor);
	CMouseEvtHander::MouseEvtHand_Mode mouse_mode ;
	if(ui.rBContrast->isChecked()){
		mouse_mode = CMouseEvtHander::MouseEvtHand_Contrast;
		static QCursor contrast_cur(QPixmap(":/rcFiles/rcFiles/ContrastCur.PNG"));
		ui.gBdrawArea->setCursor( contrast_cur );

	}else 
	if(ui.rBZoom->isChecked()){
		mouse_mode = CMouseEvtHander::MouseEvtHand_Zoom;
		//
 ;
		static QCursor zoom_cur(QPixmap(":/rcFiles/rcFiles/ZoomCur.PNG"));
		ui.gBdrawArea->setCursor( zoom_cur );


	}else
	if(ui.rBPan->isChecked()){
		mouse_mode = CMouseEvtHander::MouseEvtHand_Pan;
		static QCursor pan_cur(QPixmap(":/rcFiles/rcFiles/PanCur.PNG"));
		ui.gBdrawArea->setCursor( pan_cur );
	}else
	if(ui.rBCursor->isChecked()){
		mouse_mode = CMouseEvtHander::MouseEvtHand_Cursor;
		static QCursor point_cur;
		point_cur.setShape(Qt::ArrowCursor);
		ui.gBdrawArea->setCursor( point_cur );

	}
 
	m_openGLWidget->setCurEventProcMode(mouse_mode);
}
void MainWindow::onHistoGram()
{
	if(!m_CurrentPixelData) return;

	if(!m_openGLWidget->calImageHist(m_CurrentPixelData, m_hist_data)) return;

	m_hist_data->m_huVal = m_CurrentPixelData->m_hu_offset;

	m_hist_data->findValidPeak();
	if(0){
		FILE *fp = fopen("dbg_hist.csv","wt");
		fprintf(fp,"No., Hist\n");
		for(int i=0;i<m_hist_data->m_hist_size;i++){
			fprintf(fp,"%d, %d\n",i,m_hist_data->m_hist[i]);
		}
		fclose(fp);
	}
 CDispHist hist(this);
 hist.setupHistData(m_hist_data);
	hist.exec();
}

void MainWindow::openDicomSeries(const QStringList &dicomList)
{
	
	m_multiFrameFlag = true;
	setupMultiFrameGUI(true);

	m_newSeriesFlag = true;

	m_DicomFileList = dicomList;
	if(m_DicomFileList.size()<=1){
		setupMultiFrameGUI(false);
		 
		if(m_DicomFileList.size()==1){
			openDicomFile(m_DicomFileList[0]);
		}
	}else{
		setupMultiFrameGUI(true);
		 
		ui.SliderFrame->setRange(0,m_DicomFileList.size()-1);
		onFrameMove(0);
	}
 
}
void MainWindow::onFrameStepLeft()
{
	onFrameStep(-1);
}
void MainWindow::onFrameStepRight()
{
	onFrameStep(+1);
}

 
void MainWindow::onFrameMove(int frameNo){
	int x = frameNo;

	if(	(frameNo<0) ||
		(frameNo>=m_DicomFileList.size())){
			return;
	}
	QString msg_temp;
	msg_temp = QString(" Slice: %1/%2").arg(QString::number(frameNo +1),QString::number(m_DicomFileList.size()));
	{
		int space_size = 17-msg_temp.size();
		for(int i=0;i<space_size;i++){
			msg_temp = QString(" ") + msg_temp;
		}
	}
	setupStatusBarMsg(msg_temp,StatusIndex_DispFrame);

	QString fileName = m_DicomFileList.at(frameNo);

 
	openDicomFile(fileName);
 
}

std::string MainWindow::read_shader_file(const QString &fileNameInRes)
{
std::string ret_str;
{
	QString fileName = QString(":/openGLShader/openGLShader/")+fileNameInRes ;
	

	QFile file(fileName);
;
	try {
		file.open( QIODevice::ReadOnly | QIODevice::Text);
 
		QTextStream in(&file);
	   while ( !in.atEnd() )
	   {
		  QString line = in.readLine() + QString("\n");
		  ret_str = ret_str + QString2Str(line) ;;
	   }
 
		file.close();
	}catch(...){
	}

}
return ret_str;
}


void pickupDicomFile(const QString &fileListStr, QStringList &outList,const QString path)
{
	outList.clear();
	if(fileListStr.size()<1){
		return;
	}

	QString fileListTemp = fileListStr;
	int pos_next = 0;
	int pos = 0;

	int total_size = fileListStr.size();

	QString fileName;

	//
	QString prex_paht = path;
	if(prex_paht.size()>0){
		prex_paht = prex_paht.trimmed();
		if(prex_paht.indexOf("/",prex_paht.size()-1,Qt::CaseInsensitive)>1){
			;//OK
		}else if (prex_paht.indexOf("\\",prex_paht.size()-1,Qt::CaseInsensitive)>1){
			;//OK
		}else{
			prex_paht = prex_paht + QString("/");
		}
	}
	while(pos>=0){
		if(pos_next>=total_size){
			break;
		}
		pos = fileListTemp.indexOf(";",pos_next);
		if(pos>=0){
			fileName = fileListTemp.mid(pos_next,(pos-pos_next));
		}else{
			fileName = fileListTemp.mid(pos_next);
		}
		if(prex_paht.size()>0){
			fileName = prex_paht+fileName;
		}
		outList.append(fileName);

		pos_next = pos+1;
	}
}
void MainWindow::showEvent(QShowEvent *event)
{
	QMainWindow::showEvent( event);
 
	if(m_first_show_flag){

		if(g_dicom_file_list.size()>0){
			QStringList fullPath_fileList;
		
			pickupDicomFile(g_dicom_file_list,fullPath_fileList,g_dicom_file_path);
		 
		   openDicomSeries(fullPath_fileList);
		}else 
		if(g_dicom_series_folder.size()>0){
			openDicomFolder(g_dicom_series_folder);
		}else
		if(g_dicom_file.size()>0){

			m_newSeriesFlag  = true;
			openDicomFile(g_dicom_file);
			m_multiFrameFlag = false;
			setupMultiFrameGUI(false);

		}else{
			m_multiFrameFlag = false;
			setupMultiFrameGUI(false);
			if(m_openGLWidget){
				//Display NUll
				QSize size = ui.gBdrawArea->size();
				m_openGLWidget->myResize(size);
			 
			}
		}
 
		m_first_show_flag = false;
	}
}
void MainWindow::setupImageDisplay(int ww,int wl,int pixel_max,float hu_offset)
{
	int set_min = (int)(-hu_offset);
	int set_max = (int)(pixel_max-hu_offset);

	m_settingWWWL_flag = true;

	ui.SliderWW->setRange(1,pixel_max);
	ui.spinBoxWW->setRange(1,pixel_max);
	ui.SliderWW->setValue(ww);
	ui.spinBoxWW->setValue(ww);
	//
	ui.SliderWL->setRange(set_min,set_max);
	ui.spinBoxWL->setRange(set_min,set_max);
	ui.SliderWL->setValue(wl);
	ui.spinBoxWL->setValue(wl);
	//
	
	m_openGLWidget->enablePaint(false);

  	m_openGLWidget->setupLut(1024*4);//LUT固定  
// 	m_openGLWidget->setupLut( pixel_max);
	onChangeLut( ui.cbBLut->currentIndex());
	 
	
	//先にサイズをセットして置く
	QSize size = ui.gBdrawArea->size();
	m_openGLWidget->myResize(size);

	m_settingWWWL_flag = false;

	setupWWWL();

	m_openGLWidget->doFilter(ui.cbBFilter->currentIndex() );

	m_openGLWidget->enablePaint(true);

	m_openGLWidget->myUpdate();

	
}
void MainWindow::onChgSliderWW(int ww)
{
	if(m_internalGUIChgFlag){
	//	m_internalGUIChgFlag = false;
	//	return ;
	}else{
		m_internalGUIChgFlag = true;
		ui.spinBoxWW->setValue(ww);
		m_internalGUIChgFlag = false;
	}
	
	if(m_settingWWWL_flag){

	}else{
		setupWWWL();
	}

}
		
void MainWindow::onChgSliderWL(int wl)
{
	if(m_internalGUIChgFlag){
//		m_internalGUIChgFlag = false;
//		return ;
	}else{
		m_internalGUIChgFlag = true;
		ui.spinBoxWL->setValue(wl);
		m_internalGUIChgFlag = false;
	}

	if(m_settingWWWL_flag){

	}else{
		setupWWWL();
	}
}
void MainWindow::setupWWWL()
{
	if(!m_CurrentPixelData){
		return;
	}

	m_openGLWidget->enablePaint(false);
	m_openGLWidget->setupWWWL(ui.SliderWW->value(),ui.SliderWL->value()+m_disp_HU_val);
	m_openGLWidget->enablePaint(true);
	m_openGLWidget->updateViewer();
}
void MainWindow::onChgEditWW(int ww)
{
	if(m_internalGUIChgFlag){
//		m_internalGUIChgFlag = false;
		return ;
		 
	}else{
		m_internalGUIChgFlag = true;
		ui.SliderWW->setValue(ww);
		m_internalGUIChgFlag = false;
	}
}
void MainWindow::onChgEditWL(int wl)
{
	if(m_internalGUIChgFlag){
//		m_internalGUIChgFlag = false;
		return ;
	 
	}else{

		m_internalGUIChgFlag = true;
		ui.SliderWL->setValue(wl);
		m_internalGUIChgFlag = false;
	}
}
void MainWindow::setupStatusBarMsg(const QString &msg,int index)
{ 
	if(index>=STATUS_FIELD_NUM) return; 
	m_DispStatusBarMsg[index] = msg;

	this->update();
};

void MainWindow::onFrameStep(int step)
{
	if(!m_multiFrameFlag) return;

	int cur_pos = ui.SliderFrame->value();
	ui.SliderFrame->setValue(cur_pos+step);
}

void MainWindow::setupMultiFrameGUI(bool multiFrame)
{
	 if(!m_valid) return ;

	QRect drawAreaRect = ui.gBdrawArea->geometry();
	QRect Org_FrameAreaRect = ui.groupBox_FrameNo->geometry();
	if(multiFrame){
 		ui.SliderFrame->show(); 
 		ui.pBStepLeft->show(); 
		ui.pBStepRight->show(); 
		ui.groupBox_FrameNo->setMaximumHeight(40);
		ui.groupBox_FrameNo->setMinimumHeight(40);
	}else{
		ui.SliderFrame->hide(); 
 		ui.pBStepLeft->hide(); 
		ui.pBStepRight->hide(); 
		ui.groupBox_FrameNo->setMaximumHeight(1);
		ui.groupBox_FrameNo->setMinimumHeight(1);

		setupStatusBarMsg("     ",StatusIndex_DispFrame);//clear 

	}
	QRect New_FrameAreaRect = ui.groupBox_FrameNo->geometry();

	
	int delta_h = New_FrameAreaRect.height()-Org_FrameAreaRect.height();
	
		

	/*
	* force : CanvasWidgetOpenGL::resizeEvent
	*/
	if(delta_h !=0){
		ui.gBdrawArea->setGeometry(drawAreaRect.left(),drawAreaRect.top(),
			drawAreaRect.width(),drawAreaRect.height()-delta_h);
	}
 	

 //	QRect new_size = geometry();
	 
 //	setGeometry( QRect(new_size.left(),new_size.top(),new_size.width()-1,new_size.height()-1));
	 
 //	setGeometry(new_size);
	 

}
 
void MainWindow::notifyWWWL(int ww,int wl /*+m_disp_HU_val*/)
{
	if(!m_CurrentPixelData) return;

	m_settingWWWL_flag = true;


	ui.spinBoxWW->setValue(ww);
	ui.spinBoxWL->setValue(wl-m_disp_HU_val);

	m_settingWWWL_flag = false;
 
#if 1
	setupWWWL();

#else
 	m_openGLWidget->enablePaint(false);

	m_openGLWidget->setupWWWL(ww,wl);//ui.SliderWW->value(),ui.SliderWL->value());

	m_openGLWidget->enablePaint(true);
	m_openGLWidget->updateViewer();
#endif
	 
}

void MainWindow::chgCursorStatus(bool reverse)
{
	 
	if(reverse){
		if(ui.rBCursor->isChecked()){
			ui.rBPan->setChecked(true);
		
		}else
		if(ui.rBPan->isChecked()){
			ui.rBZoom->setChecked(true);
		
		}else
		if(ui.rBZoom->isChecked()){
			ui.rBContrast->setChecked(true);
		
		}else
		if(ui.rBContrast->isChecked()){
			ui.rBCursor->setChecked(true);

		}

	}else{
		if(ui.rBContrast->isChecked()){
			ui.rBZoom->setChecked(true);
		
		}else
		if(ui.rBZoom->isChecked()){
			ui.rBPan->setChecked(true);
		
		}else
		if(ui.rBPan->isChecked()){
			ui.rBCursor->setChecked(true);
		
		}else
		if(ui.rBCursor->isChecked()){
			ui.rBContrast->setChecked(true);

		}
	}
 
	onChgMouse();

}
void MainWindow::setCursorStatus(int index)
{
	bool update_flag = true;
	switch(index){
		case 0:
		ui.rBContrast->setChecked(true);
		break;
		case 1:
		ui.rBZoom->setChecked(true);
		break;
		case 2:
		ui.rBPan->setChecked(true);
		break;
		case 3:
		ui.rBCursor->setChecked(true);
		break;
		default:
		update_flag = false;

	}
	if(update_flag){
		onChgMouse();
	}
}
void MainWindow::onFitZoom()
{
	if(m_openGLWidget){
		m_openGLWidget->resetViewerSize();
		m_openGLWidget->updateViewer();
	}
}
void MainWindow::onAutoWWWL()
{
	if(!m_CurrentPixelData) return;
	if(!m_openGLWidget->calImageHist(m_CurrentPixelData, m_hist_data)) return;

	m_hist_data->m_huVal = m_CurrentPixelData->m_hu_offset;

	m_hist_data->findValidPeak();

	int pixel_min,pixel_max;
 	m_hist_data->findRange(pixel_min,pixel_max);

	pixel_min  = pixel_min-m_CurrentPixelData->m_hu_offset;
	pixel_max  = pixel_max-m_CurrentPixelData->m_hu_offset;

	int ww = pixel_max-pixel_min;
	int wl = ww/2 + pixel_min;
	notifyWWWL(ww,wl+m_disp_HU_val);

//	this->setupWWWL();
}

#include "QKeyEvent"
void MainWindow::keyPressEvent(QKeyEvent *e)
{
	QMainWindow::keyPressEvent( e);
	switch(e->key()){
	case Qt::Key_F1:
		onFitZoom();
		break;
	case Qt::Key_F2:
		onAutoWWWL();
		break;

	case Qt::Key_F4:
		onHistoGram();
		break;
	case Qt::Key_F5:
		setCursorStatus(0);
		break;
	case Qt::Key_F6:
		setCursorStatus(1);
		break;
	case Qt::Key_F7:
		setCursorStatus(2);
		break;
	case Qt::Key_F8:
		setCursorStatus(3);
		break;
	case Qt::Key_Alt:
		chgCursorStatus();
		break;
	case Qt::Key_Control:
		chgCursorStatus(true /*reverse*/);
		break;
	case Qt::Key_1:
	case Qt::Key_A:
	case Qt::Key_Minus:
		{
			if(m_openGLWidget){
				int w = ui.gBdrawArea->geometry().width();
				int h = ui.gBdrawArea->geometry().height();
				float geo_rate = w;
				if(h<w) {
					geo_rate = h;
				}
				
				QPoint delta(-0.1*geo_rate,-0.1*geo_rate);
				m_openGLWidget->chgZoom(delta );
			}
		};
		break;
	
	case Qt::Key_2:
	case Qt::Key_Z:
	case Qt::Key_Plus:
		{
			if(m_openGLWidget){
				int w = ui.gBdrawArea->geometry().width();
				int h = ui.gBdrawArea->geometry().height();
				float geo_rate = w;
				if(h<w) {
					geo_rate = h;
				}
				
				QPoint delta(0.1*geo_rate,0.1*geo_rate);
				m_openGLWidget->chgZoom(delta );
			}
		};
		break;
	///
	case Qt::Key_F11:
	case Qt::Key_Comma: // ',' -> '<'
	case Qt::Key_Less:  //'<'
	case Qt::Key_Left:
		onFrameStepLeft();
		break;
	///
	case Qt::Key_F12:
	case Qt::Key_Period: // ',' -> '>'
	case Qt::Key_Greater: //'>' 
	case Qt::Key_Right:
		onFrameStepRight();
		break;
	//////
	default:
		break;
	}
}
void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
	QMainWindow::keyReleaseEvent( e);
}