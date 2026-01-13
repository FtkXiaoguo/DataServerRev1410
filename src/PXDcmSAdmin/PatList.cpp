#include "PatList.h"
#include "QtHelper.h"
#include <time.h>

#include <QtCore/qdatetime.h>

#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
 #include <QtCore/qstring.h>
#include <QMessageBox.h>

 #include "AEsGUI.h"
#include "PxDcmStudysManage.h"

#include "assert.h"

#include "imageViewer.h"

//#140_search_Japanese_JIS_UTF8
#include <QLocale>
void  _getCurrentAppSpecificCharacterSet(QLocale::Country ct, std::string &CharacterSet)
{
	CharacterSet.clear();
	switch (ct){
	default:
		break;
	case QLocale::Japan:
		//#2217_Pass_Args_File_to_Anonymize
		CharacterSet = "SJIS";
		break;
	case QLocale::Taiwan:
		// Chinese (Taiwan) 0x0404 zh-TW
		CharacterSet = "BIG5"; //Big5
		break;
	case QLocale::China:
		// Chinese (PR China) 0x0804 zh-CN
		CharacterSet = "ISO_IR 100\\GB18030";
		break;
	case QLocale::Russia://#2108 2021/01/06
		CharacterSet = "ISO_IR 100\\ISO_IR 144";

		break;
	}
}
const int view_width_diff_study_series = 72;
 QVector<QRgb> m_color_table;
CImageDrawWidget::CImageDrawWidget(QWidget *parent) : QWidget(parent)
{
	 
m_bitMap = 0;
m_qImage = 0;
for(int i=0;i<256;i++){
	m_color_table.append(qRgb(i, i,i));
}
}
CImageDrawWidget::~CImageDrawWidget()
{
	if(m_bitMap) delete [] m_bitMap;
	if(m_qImage) delete m_qImage;
}
void CImageDrawWidget::clearImage()
{
	m_sizeX = 64;
	m_sizeY = 64;
	if(m_bitMap) delete [] m_bitMap;
	m_bitMap = new unsigned char[m_sizeX*m_sizeY];
	for(int i=0;i<m_sizeX*m_sizeY;i++){
		m_bitMap[i] = 0;
	}
	m_qImage = new QImage(m_bitMap,m_sizeX,m_sizeY,m_sizeX, QImage::Format_Indexed8);
	 
	m_qImage->setColorTable(m_color_table);

	//adjust draw size
	QSize draw_area_size = size();
	 
	m_drawSizeX = 0;//draw_area_size.width();
	m_drawSizeY = 0;//draw_area_size.height();
	 

	update();

}
void CImageDrawWidget::setupBitMap(const unsigned char *bitmap,int sizeX,int sizeY,int bbp)
{
	 m_sizeX = sizeX;
	 m_sizeY = sizeY;
	 if(m_bitMap) delete [] m_bitMap;
	 m_bitMap = new unsigned char[m_sizeX*m_sizeY*bbp];
	 memcpy(m_bitMap,bitmap,m_sizeX*m_sizeY*bbp);

	 int bytesPerLine = m_sizeX;
	if(m_qImage) delete m_qImage;

	if(bbp >1){
		bytesPerLine = m_sizeX*bbp;
 
		m_qImage = new QImage(m_bitMap,m_sizeX,m_sizeY,bytesPerLine, QImage::Format_RGB32);
	}else{
		m_qImage = new QImage(m_bitMap,m_sizeX,m_sizeY,bytesPerLine, QImage::Format_Indexed8);
	}
	m_qImage->setColorTable(m_color_table);

	//adjust draw size
	QSize draw_area_size = size();
	double alfa_x = draw_area_size.width()/(double)m_sizeX ;
	double alfa_y = draw_area_size.height()/(double)m_sizeY ;
	if(alfa_x >alfa_y){
		m_drawSizeX = alfa_y*m_sizeX;
		m_drawSizeY = draw_area_size.height();
	}else{
		m_drawSizeX = draw_area_size.width();
		m_drawSizeY = alfa_x*m_sizeY;
	}
	 

	this->update();
}
void CImageDrawWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin( this );

	if(m_qImage!=0){
		 
		QPixmap bitmap = QPixmap::fromImage(*m_qImage,Qt::MonoOnly);

		 QSize draw_size = this->size();
		QSize bitmap_size = bitmap.size();

	//	draw_size = bitmap_size;


		painter.drawPixmap( 0,0,m_drawSizeX,m_drawSizeY, bitmap,0,0,bitmap_size.width(),bitmap_size.height() );
	}

	painter.end();
}
void CImageDrawWidget::creatDrawArea(QWidget *parent)
{
	resize(parent->size());
}

///////////////////

/////////////
//DBから検索したリストの詳細をキープする
//表示画面上の情報は少なく、ソートされることもある。
//下記のリストと順番しない。
static PxDicomInforList _Study_Objs;
static PxDicomInforList _Series_Objs;
static PxImageInforList _Images_Objs;

////#12 2012/03/26 K.Ko  
int findObjNumberFromStudyUID(const std::string &study_uid)
{
	int ret_no = -1;
	int size=_Study_Objs.size();
	for(int i=0;i<size;i++){
		if(study_uid == _Study_Objs[i].m_studyUID){
			ret_no = i;
			break;
		}
	}
	return ret_no;
}
int findObjNumberFromSeriesUID(const std::string &series_uid)
{
	int ret_no = -1;
	int size=_Series_Objs.size();
	for(int i=0;i<size;i++){
		if(series_uid == _Series_Objs[i].m_seriesUID){
			ret_no = i;
			break;
		}
	}
	return ret_no;
}
//////////

///////////////////////
// date selection
enum { kToday = 0, kYesterday, kThisWeek, kLastWeek, 
			kThisMonth, kLastMonth, kFromLastMonth,
			kThisYear, kLastYear, kFromLastYear,
			kAll};
const int select_Data_len = 10;
// Japanese
const char* pSelect_Date_J[] = { "今日", "昨日", "今週", "先週", 
			"今月", "先月", "先月以後" , 
			"今年", "昨年", "昨年以後"
};
		
// Non-Japanese
const char* pSelect_Date_E[] = { "Today", "Yesterday", "This Week", "Last Week", 
	"This Month", "Last Month", "From Last Month",  
	"This Year", "Last Year", "From Last Year"
};
static void SprintTwoDates(char *out, struct tm& firstDay, struct tm& lastDay)
{
	sprintf(out,"%d%02d%02d-%d%02d%02d",
			firstDay.tm_year + 1900,firstDay.tm_mon+1, firstDay.tm_mday,
			lastDay.tm_year + 1900, lastDay.tm_mon+1, lastDay.tm_mday);
}
static const char* getSelectDate(int when)
{
	static char out[10][32];
	static int nbuf;
	char *p = out[++nbuf%10];
	static const int secsInADay = 24*60*60;
	struct tm start, end;
	
	time_t t = time(0);
	struct tm today = *localtime(&t);
	*p = '\0';
	
	if (when ==   kToday)
	{
		sprintf(p,"%d%02d%02d", today.tm_year + 1900, today.tm_mon+1, today.tm_mday);
	}
	else if (when ==  kYesterday)
	{
		t -= secsInADay;
		start = *localtime(&t);
		sprintf(p,"%d%02d%02d", start.tm_year + 1900, start.tm_mon+1, start.tm_mday);
	}
	else if (when ==  kThisWeek)
	{
		// this week means from last Sunday until today		
		t -= today.tm_wday*secsInADay;
		start = *(localtime(&t));	
		SprintTwoDates(p, start, today);
	}
	else if (when ==  kLastWeek)
	{	
		t -= today.tm_wday*secsInADay;
		end = *(localtime(&t));	
		t = ( t - 7*secsInADay);
		start = *localtime(&t);
		SprintTwoDates(p, start, end);
	}
	else if (when ==  kThisMonth)
	{
		start = today;
		start.tm_mday = 1;
		SprintTwoDates(p, start, today);
	}
	else if (when ==  kLastMonth)
	{
		t -= (today.tm_mday)*secsInADay;
		end = *localtime(&t);
		start = end;
		start.tm_mday = 1;
		SprintTwoDates(p, start, end);
	}
	else if (when ==  kFromLastMonth) //#1060 2011/02/24 by K.Ko add FromLastMonth  
	{	
		t -= (today.tm_mday)*secsInADay;
		end = today;
		start = *localtime(&t);
		start.tm_mday = 1;
		SprintTwoDates(p, start, end);
	}
	else if (when ==  kThisYear)
	{
		start.tm_year = today.tm_year;
		start.tm_mon = 0;
		start.tm_mday = 1;
		SprintTwoDates(p, start, today);
	}
	else if (when ==  kLastYear)
	{
		start.tm_year = end.tm_year = today.tm_year - 1;
		start.tm_mon = 0;
		end.tm_mon = 11;
		start.tm_mday = 1;
		end.tm_mday = 31;
		SprintTwoDates(p, start, end);
	}
	else if (when ==  kFromLastYear) //#1060 2011/02/24 by K.Ko add  FromLastYear
	{
		start.tm_year = end.tm_year = today.tm_year - 1;
		start.tm_mon = 0;
	//	end.tm_mon = 11;
		start.tm_mday = 1;
//		end.tm_mday = 31;
		end = today;
		SprintTwoDates(p, start, end);
	}
	else 
		return 0;
	
	return p;	
}

PatListMain::PatListMain(QWidget *parent) : TabPageComInf(parent)
{
	//#140_search_Japanese_JIS_UTF8
	CPxDcmStudysManage::InitCharacterLib();
	QLocale curLocale = QLocale::system();
	QLocale::Country ct = curLocale.country();
	std::string CharacterSet;
	_getCurrentAppSpecificCharacterSet(ct, CharacterSet);
	m_CharacterSet = CharacterSet ;

	m_studyListColumn	= 9+1; //#12 2012/03/26 K.Ko 最後の列にStudyUID追加／非表示
	m_seriesListColumn	= 4+1; //#12 2012/03/26 K.Ko 最後の列にSeriesUID追加／非表示
 
	//

	m_refStudyWidth		= 940;
	m_curStudyWidth		= m_refStudyWidth;
 
	m_viewDispStudyReady = false;
	m_viewDispSeriesReady = false;
	////////
	readCdmLines();

	m_ProcSeriesCmd1 = 0;
	ui.setupUi(this);
	
	{
		ui.cmBStudyDateSel->clear();
		for(int i=0;i<select_Data_len;i++){
			ui.cmBStudyDateSel->addItem(pSelect_Date_E[i]);
		}
		//
		ui.dEStudyDate1->setDate(QDate::currentDate());
		ui.dEStudyDate2->setDate(QDate::currentDate());
	}
	
	 
	//StudyList
	 
	m_StudyList = new CPXPatientList(ui.groupBoxStudyList);
	m_StudyList->setObjectName(QString::fromUtf8("StudyList"));
	m_StudyList->setupTableWidget(CPXPatientList::ListType_Study,this);

	ui.LayoutStudyList->removeWidget(ui.tableWidgetStudyList);
	ui.tableWidgetStudyList->hide();
	ui.LayoutStudyList->addWidget(m_StudyList);
	//
	m_StudyList->setDefaultSortColumn(2,false/*AscendingOrder*/); //StudyData sort
	//
	//SeriesList
	
	m_SeriesList = new CPXPatientList(ui.groupBoxSeriesList);
	m_SeriesList->setObjectName(QString::fromUtf8("SeriesList"));
	m_SeriesList->setupTableWidget(CPXPatientList::ListType_Series,this);
	//
	m_SeriesList->setDefaultSortColumn(2,false/*AscendingOrder*/); //StudyData sort

	ui.LayoutSeriesList->removeWidget(ui.tableWidgetSeriesList);
	ui.tableWidgetSeriesList->hide();
	ui.LayoutSeriesList->addWidget(m_SeriesList);
        

        
	QObject::connect(m_StudyList, SIGNAL(clicked(QModelIndex)), this, SLOT(onSelectStudy()));
	QObject::connect(m_SeriesList, SIGNAL(clicked(QModelIndex)), this, SLOT(onSelectSeries()));
	//
	QObject::connect(m_StudyList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDclickStudy()));
	QObject::connect(m_SeriesList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDclickSeries()));


//	m_StudyList->addAction();

	//
	//draw area
	
	m_preViewImageArea = new CImageDrawWidget(ui.gBDrawArea);
	
//	m_imageViewer = new CImageViewer; //not yet
	switchStudyDate();

}
PatListMain::~PatListMain()
{
	if(m_ProcSeriesCmd1){
		if(m_ProcSeriesCmd1->state() == QProcess::Running){
			m_ProcSeriesCmd1->terminate();
		}
	}
}

void PatListMain::keyPressEvent(QKeyEvent *e)
{
	if (e->type() == QEvent::KeyPress)
	{
		if (e->key() == Qt::Key_Return)
			onSearch();	
	}
}
		
void PatListMain::mousePressEvent(QMouseEvent *e)
{
	

	QWidget::mousePressEvent(e);
}
void PatListMain::mouseReleaseEvent(QMouseEvent *e)
{
	QWidget::mouseReleaseEvent(e);
}
void PatListMain::closeEvent(QCloseEvent *)
{
	
}
void PatListMain::resizeEvent ( QResizeEvent * resize_e )
{
	TabPageComInf::resizeEvent( resize_e);
	if(m_preViewImageArea){
		QSize  draw_size = ui.gBDrawArea->size();
		m_preViewImageArea->creatDrawArea(ui.gBDrawArea);
	}
	const QSize &size = resize_e->size();
	 
	int study_view_w	= size.width()  - 336;
	int series_view_w	= study_view_w  - view_width_diff_study_series;
	//
	m_curStudyWidth = study_view_w;
	resetStudyColumnWidth(study_view_w);
	//
	resetSeriesColumnWidth(series_view_w);
	 
	 
}

inline void _addRight_Asterisk(std::string &str)
{
	int s_size = str.size();
	if (str.size() > 0){
		if (str[s_size - 1] != '*'){
			str = str+ "*";
		}
	}
}
void PatListMain::onSearch()
{
	
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	//clear all
	m_StudyList->setRowCount(0);
	m_SeriesList->setRowCount(0);
	m_preViewImageArea->clearImage();
	//


	StudyQuery query;
	query.m_patientName		= QString2Str(ui.lEPatientName->text().trimmed());
	query.m_patientID		= QString2Str(ui.lEPatientID->text().trimmed());

	_addRight_Asterisk(query.m_patientName);

	//study date
	if(ui.cBStudyDateSel->checkState() == Qt::Checked){
		int sel_no = ui.cmBStudyDateSel->currentIndex();
		if((sel_no>=kToday)&&(sel_no<=kFromLastYear)){
			query.m_studyDate = getSelectDate(sel_no);
		}
	}else{
		if(ui.cBStudyDate->checkState() != Qt::Checked){
			query.m_studyDate.empty();
		}else{
			QDate date_from = ui.dEStudyDate1->date();
			QDate date_to = ui.dEStudyDate2->date();
			 
			QString str_temp;
			QString date_str;
			//from date
			str_temp.sprintf("%04d",date_from.year());
			date_str += str_temp;
			str_temp.sprintf("%02d",date_from.month());
			date_str += str_temp;
			str_temp.sprintf("%02d",date_from.day());
			date_str += str_temp;
			//
			date_str += QString("-");
			// to date
			str_temp.sprintf("%04d",date_to.year());
			date_str += str_temp;
			str_temp.sprintf("%02d",date_to.month());
			date_str += str_temp;
			str_temp.sprintf("%02d",date_to.day());
			date_str += str_temp;
			query.m_studyDate		= QString2Str(date_str);
		}
	}

	//comment
	query.m_studyDescription = QString2Str(ui.lEComment->text().trimmed());
	_addRight_Asterisk(query.m_studyDescription);

	CPxDcmStudysManage DcmDbMan(m_CharacterSet);

	if (!DcmDbMan.queryStudys(query, _Study_Objs)){
		//added 2022/12/01
	}
	else{
		dispStudyHeader();
		dispSearchStudys();
	}

	QApplication::restoreOverrideCursor();
	 
}
void PatListMain::searchSeries(const std::string &studyUID)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	DcmDbMan.querySeries(studyUID,_Series_Objs);

	dispSeriesHeader();
	dispSearchSeries();

	m_preViewImageArea->clearImage();

	QApplication::restoreOverrideCursor();
}
void PatListMain::searchImages(const std::string &studyUID,const std::string &seriesUID)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	DcmDbMan.queryImages(studyUID,seriesUID,_Images_Objs);

//	dispSeriesHeader();
//	dispSearchSeries();

	//#27 2012/06/14
	PxDicomInfor *pDicom_info = getCurrentStudyInfo();

	dispSOPImage(&(_Images_Objs[0]),pDicom_info);
	setupFrameNoRange(_Images_Objs.size(),0);

	QApplication::restoreOverrideCursor();
}
void PatListMain::setupFrameNoRange(int frames,int curNo)
{
	ui.sBFrameNo->setRange(1,frames); //edit box
	ui.sBFrameNo->setValue(curNo);

    ui.vSFrameNo->setRange(1,frames); //slider
	ui.vSFrameNo->setValue(curNo+1);

}
void PatListMain::onSelect(QTableWidgetItem* item1,QTableWidgetItem* item2)
{
	int c1 = item1->column();
	if(item2){
	int c2 = item2->column();
	}
}
void PatListMain::onSelectStudy()
{
	int sel=m_StudyList->currentRow();

	QString sel_uid = m_StudyList->getStudyUID(sel);

	 
//	searchSeries(_Study_Objs[sel].m_studyUID);
	//#12 2012/03/26 K.Ko
	if(sel_uid.size()>0){
		searchSeries(QString2Str(sel_uid));
	}
}
void PatListMain::onSelectSeries()
{
	int sel_study=m_StudyList->currentRow();
//	std::string studyUID = _Study_Objs[sel_study].m_studyUID;

	//#12 2012/03/26 K.Ko
	QString sel_study_uid = m_StudyList->getStudyUID(sel_study);
	if(sel_study_uid.size()<1) return;
	std::string studyUID = QString2Str(sel_study_uid);


	int sel_series=m_SeriesList->currentRow();
//	std::string seriesUID = _Series_Objs[sel_series].m_seriesUID;
	//#12 2012/03/26 K.Ko
	QString sel_series_uid = m_SeriesList->getSeriesUID(sel_series);
	if(sel_series_uid.size()<1) return;
	std::string seriesUID = QString2Str(sel_series_uid);

	
	searchImages( studyUID,seriesUID);
}
void PatListMain::onDclickStudy()
{
	int sel=m_StudyList->currentRow();
//	searchSeries(_Study_Objs[sel].m_studyUID);
//	_Study_Objs[sel].
	////#12 2012/03/26 K.Ko
	QString sel_uid = m_StudyList->getStudyUID(sel);
	if(sel_uid.size()<1) return;
	std::string studyUID = QString2Str(sel_uid);
	if(studyUID.size()<1) return;

	////
	std::string series_uid;
	int obj_size = _Series_Objs.size();

	if(obj_size<1) return;
 
	int image_count_max = 0;

	int index = 0;
	for(int i=0;i<obj_size;i++){
		if(_Series_Objs[i].m_imageCount.size()>0){
			int imageCount = Str2QString(_Series_Objs[i].m_imageCount).toInt();
			if(image_count_max<imageCount){
				image_count_max = imageCount;
				index = i;
				
			}
		}
	}
	series_uid = _Series_Objs[index].m_seriesUID ;
	if(series_uid.size()<1) return;

	exeDefaultSeries(studyUID,series_uid);

}

void PatListMain::onDclickSeries()
{
	int sel=m_StudyList->currentRow();
 
	QString sel_uid = m_StudyList->getStudyUID(sel);
	if(sel_uid.size()<1) return;
	std::string studyUID = QString2Str(sel_uid);
	if(studyUID.size()<1) return;


	////
	int sel_series=m_SeriesList->currentRow();
 
	QString sel_series_uid = m_SeriesList->getSeriesUID(sel_series);
	if(sel_series_uid.size()<1) return;

	std::string seriesUID = QString2Str(sel_series_uid);


	exeDefaultSeries(studyUID,seriesUID);

}
void PatListMain::onImageFrameNo(int FramNo)
{
//from edit spin
	int obj_size = _Images_Objs.size();

	int frameIndex = FramNo-1;
// 	if(frameIndex<(obj_size-1)){
	//#74 2014/01/07  K.Ko
	if(frameIndex<(obj_size)){
		if(frameIndex>=0){

			//#27 2012/06/14
			PxDicomInfor *pDicom_info = getCurrentStudyInfo();

			dispSOPImage(&(_Images_Objs[frameIndex]),pDicom_info);

			ui.vSFrameNo->setValue(FramNo);
		}
	}
	

}
void PatListMain::onSliderFrameNo(int FramNo)
{
//from slider
	int obj_size = _Images_Objs.size();
	int frameIndex = FramNo-1;
 //	if(frameIndex<(obj_size-1)){
	//#74 2014/01/07  K.Ko
 	if(frameIndex<(obj_size)){
		if(frameIndex>=0){

			//#27 2012/06/14
			//#27 2012/06/14
			PxDicomInfor *pDicom_info = getCurrentStudyInfo();

			dispSOPImage(&(_Images_Objs[frameIndex]),pDicom_info);

			ui.sBFrameNo->setValue(FramNo);
		}
	}
	
}
void PatListMain::resetStudyColumnWidth(int w)
{
	 if(!m_viewDispStudyReady) return;

	float chg_rate = (float)w/m_refStudyWidth;

	//setup the header Height
	m_StudyList->horizontalHeaderItem(0)->setSizeHint(QSize(100,TABLE_W_HEADER_HEIGHT));


	if(chg_rate<1.0){
		return ;
	}

	int col_index = 0;
	//PatientID
	m_StudyList->setColumnWidth(col_index,90*chg_rate);
	col_index++;

	//PatientName
	m_StudyList->setColumnWidth(col_index,120*chg_rate);
	col_index++;

	//Study Date
	m_StudyList->setColumnWidth(col_index,120*chg_rate);
	col_index++;

	//BirthDate
	m_StudyList->setColumnWidth(col_index,100*chg_rate);
	col_index++;

	//PatientSex
	m_StudyList->setColumnWidth(col_index,80*chg_rate);
	col_index++;

	//Comment
	m_StudyList->setColumnWidth(col_index,140*chg_rate);
	col_index++;

	//modality
	m_StudyList->setColumnWidth(col_index,90*chg_rate);
	col_index++;

	//accessionNumber
	m_StudyList->setColumnWidth(col_index,80*chg_rate);
	col_index++;

	//physicianName
	m_StudyList->setColumnWidth(col_index,120*chg_rate);
	col_index++;

	//studyUID --- for sort management #12 2012/03/26 K.Ko
 
	
}


void PatListMain::dispStudyHeader()
{
	m_StudyList->setColumnCount(m_studyListColumn);

	int col_index = 0;
	//PatientID
	QTableWidgetItem *item = new QTableWidgetItem("PatientID");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,90);
	col_index++;

	//PatientName
	item = new QTableWidgetItem("PatientName");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,120);
	col_index++;

	//Study Date
	item = new QTableWidgetItem("StudyDate");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,120);
	col_index++;

	//BirthDate
	item = new QTableWidgetItem("BirthDate");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,100);
	col_index++;

	//PatientSex
	item = new QTableWidgetItem("Gender");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,80);
	col_index++;

	//Comment
	item = new QTableWidgetItem("Comment");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,140);
	col_index++;

	//modality
	item = new QTableWidgetItem("Modality");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,90);
	col_index++;


	//accessionNumber
	item = new QTableWidgetItem("StudyID");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,80);
	col_index++;

	//physicianName
	item = new QTableWidgetItem("physicianName");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,120);
	col_index++;


	//studyUID --- for sort management #12 2012/03/26 K.Ko
	item = new QTableWidgetItem("studyUID");
	m_StudyList->setHorizontalHeaderItem(col_index,  item);
	m_StudyList->setColumnWidth(col_index,100);
  	m_StudyList->setColumnHidden(col_index,true);
	col_index++;

	assert(m_studyListColumn == col_index);

	
	m_viewDispStudyReady = true;

	resetStudyColumnWidth(m_curStudyWidth);

}
void PatListMain::dispSearchStudys()
{
	int size = _Study_Objs.size();
	m_StudyList->setRowCount(size);
	
	//
	int col_index;
	for(int i=0;i<size;i++){
		
		m_StudyList->setRowHeight(i, 20);
		
		col_index = 0;
		//PatientID
		QTableWidgetItem *item = new QTableWidgetItem(_Study_Objs[i].m_patientID.c_str());
		m_StudyList->setItem(i ,col_index++, item );
	
		 
		//PatientName
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_patientName));
		m_StudyList->setItem(i ,col_index++, item );


		//Study Date
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_studyDate + std::string(" ") + _Study_Objs[i].m_studyTime));
		m_StudyList->setItem(i ,col_index++,  item);


		//BirthDate
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_patientBirthDate));
		m_StudyList->setItem(i ,col_index++,  item);


		//PatientSex
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_patientSex));
		m_StudyList->setItem(i ,col_index++,  item);


		//Comment
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_studyDescription));
		m_StudyList->setItem(i,col_index++,  item);


		//modality
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_modality));
		m_StudyList->setItem(i,col_index++,  item);


		//accessionNumber
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_studyID));
		m_StudyList->setItem(i,col_index++,  item);


		//physicianName
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_physicianName));
		m_StudyList->setItem(i,col_index++,  item);


		//studyUID --- for sort management #12 2012/03/26 K.Ko
		item = new QTableWidgetItem(Str2QString(_Study_Objs[i].m_studyUID));
		m_StudyList->setItem(i,col_index++,  item);


		 
	}
	assert(m_studyListColumn == col_index);

	m_StudyList->updateSortColumn();
}

void PatListMain::resetSeriesColumnWidth(int w)
{
	if(!m_viewDispSeriesReady) return;

	float chg_rate = (float)w/(m_refStudyWidth-view_width_diff_study_series);

	//setup the header Height
	m_SeriesList->horizontalHeaderItem(0)->setSizeHint(QSize(100,TABLE_W_HEADER_HEIGHT));

	if(chg_rate<1.0){
		return ;
	}
	//////////////////
	int col_index = 0;
	//SeriesNumber
	m_SeriesList->setColumnWidth(col_index,90*chg_rate);
	col_index++;

	//ImageCount
	m_SeriesList->setColumnWidth(col_index,100*chg_rate);
	col_index++;

	//Series Date
	m_SeriesList->setColumnWidth(col_index,120*chg_rate);
	col_index++;

	//Comment
	m_SeriesList->setColumnWidth(col_index,160*chg_rate);
	col_index++;

	//seriesUID --- for sort management #12 2012/03/26 K.Ko


}
//
void PatListMain::dispSeriesHeader()
{
	m_SeriesList->setColumnCount(m_seriesListColumn);

	int col_index = 0;
	//SeriesNumber
	QTableWidgetItem *item = new QTableWidgetItem("SeriesNo.");
	m_SeriesList->setHorizontalHeaderItem(col_index,  item); 
	m_SeriesList->setColumnWidth(col_index,90);
	col_index++;

	//ImageCount
	item = new QTableWidgetItem("ImageCount");
	m_SeriesList->setHorizontalHeaderItem(col_index,  item);
	m_SeriesList->setColumnWidth(col_index,100);
	col_index++;

	//Series Date
	item = new QTableWidgetItem("SeriesDate");
	m_SeriesList->setHorizontalHeaderItem(col_index,  item);
	m_SeriesList->setColumnWidth(col_index,120);
	col_index++;

	//Comment
	item = new QTableWidgetItem("Comment");
	m_SeriesList->setHorizontalHeaderItem(col_index,  item);
	m_SeriesList->setColumnWidth(col_index,160);
	col_index++;

	//seriesUID --- for sort management #12 2012/03/26 K.Ko
	item = new QTableWidgetItem("seriesUID");
	m_SeriesList->setHorizontalHeaderItem(col_index,  item);
	m_SeriesList->setColumnWidth(col_index,100);
 	m_SeriesList->setColumnHidden(col_index,true);
	col_index++;

	assert(m_seriesListColumn == col_index);

	
	m_viewDispSeriesReady = true;
	////////
	int series_view_w = m_curStudyWidth-view_width_diff_study_series;
 
	resetSeriesColumnWidth(series_view_w);
 

	 
}
void PatListMain::dispSearchSeries()
{
	int size = _Series_Objs.size();
	m_SeriesList->setRowCount(size);
	
	int col_index;
	//
	for(int i=0;i<size;i++){
		m_SeriesList->setRowHeight(i, 20);
		
		col_index = 0;
		//SeriesNumber
		QTableWidgetItem *item = new QTableWidgetItem(_Series_Objs[i].m_seriesNumber.c_str());
		m_SeriesList->setItem(i ,col_index++, item );
		 
		//ImageCount
		item = new QTableWidgetItem(_Series_Objs[i].m_imageCount.c_str());
		m_SeriesList->setItem(i ,col_index++, item );

		//Series Date
		item = new QTableWidgetItem((_Series_Objs[i].m_seriesDate + std::string(" ") + _Series_Objs[i].m_seriesTime).c_str());
		m_SeriesList->setItem(i ,col_index++,  item);
		 
		//Comment
		item = new QTableWidgetItem(Str2QString(_Series_Objs[i].m_seriesDescription));
		m_SeriesList->setItem(i ,col_index++,  item);
		 
		//seriesUID --- for sort management #12 2012/03/26 K.Ko
		item = new QTableWidgetItem(Str2QString(_Series_Objs[i].m_seriesUID));
		m_SeriesList->setItem(i,col_index++,  item);
	}
	assert(m_seriesListColumn == col_index);

	m_SeriesList->updateSortColumn();
}

void PatListMain::dispSOPImage(const PxImageInfor *Image,PxDicomInfor *dicom_info /*for safty check*/) //#27 2012/06/14
{
	PxImagePixel imagePixe;
	CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	int saftyCheckTag;
	if(!DcmDbMan.loadImage(*Image,imagePixe,saftyCheckTag,dicom_info)){

		m_preViewImageArea->clearImage();

		QMessageBox msgbox;
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.setText("   loadImage error");
		msgbox.exec();

		
		return;
	}
	QString saftyCheckStr;
	switch(saftyCheckTag){
		case CPxDcmStudysManage::SaftyCheckTag_None:
			saftyCheckStr.clear();
			break;
		case CPxDcmStudysManage::SaftyCheckTag_PatientName:
			saftyCheckStr="SaftyCheck Error On PatientName";
			break;
		case CPxDcmStudysManage::SaftyCheckTag_PatientID:
			saftyCheckStr="SaftyCheck Error On PatientID";
			break;
		case CPxDcmStudysManage::SaftyCheckTag_PatientBirthDate:
			saftyCheckStr="SaftyCheck Error On PatientBirthDate";
			break;
	}
	if(!saftyCheckStr.isEmpty()){
		QMessageBox msgbox;
		msgbox.setIcon(QMessageBox::Warning);
		msgbox.setText(saftyCheckStr);
		msgbox.exec();
	}

	//
	m_preViewImageArea->setupBitMap(imagePixe.m_pixelData,imagePixe.m_sizeX,imagePixe.m_sizeY,imagePixe.m_samplesPerPixel);

//	m_imageViewer->openViewer();
//	this->m_imageViewer->show();
}

static bool _cancel_study_ = false;
void PatListMain::cancelDeleteStudy()
{
	int x=0;
	_cancel_study_ = true;
}
#include <QEventLoop>
void PatListMain::onDeleteStudy()
{
 
	int  ret_q = QMessageBox::question(this," ","   Are you sure you want to delete selected data ?",
				QMessageBox::Ok|QMessageBox::Cancel,
				QMessageBox::Cancel
				);
	if(ret_q != QMessageBox::Ok){
		return ;
	}
 


	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	m_StudyList->updateSelectedList();
 

	QProgressDialog *progressDlg = new QProgressDialog("Delete Study Data in progress.", "Cancel", 0, 100);
     connect(progressDlg, SIGNAL(canceled()), this, SLOT(cancelDeleteStudy()));
	_cancel_study_ = false;
	 QEventLoop myEventLoop(this);
 
	 CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	QVector<int> selectedRows = m_StudyList->getSelectedRows();

	int row_num = selectedRows.size();
	//step-1
	//studyUID vector の作成
	//1行削除後に行番号が変ってしまうため。
	std::vector<std::string> studyUIDVector;
	{
		for(int i=0;i<row_num;i++){
			//std::string studyUID = _Study_Objs[selectedRows.at(i)].m_studyUID;
			//#12 2012/03/26 K.Ko
			QString sel_uid = m_StudyList->getStudyUID(selectedRows.at(i));
			if(sel_uid.size()>0){
				std::string studyUID = QString2Str(sel_uid);
				studyUIDVector.push_back(studyUID);
		//		DcmDbMan.delStudy(studyUID);
			}
		}
	}
	//
	//step-2
	//StudyUIDで行削除
	{
		
		progressDlg->show();
		progressDlg->setValue(0);
		{
			for(int i=0;i<100;i++){
				if(!myEventLoop.processEvents()){
					break;
				}
			}
		}

		row_num = studyUIDVector.size();
		for(int i=0;i<row_num;i++){
			 
			{
				for(int i=0;i<100;i++){
					if(!myEventLoop.processEvents()){
						break;
					}
				}
			}

			 
			if(_cancel_study_){
				break;
			}

			std::string studyUID = studyUIDVector[i];
			DcmDbMan.delStudy(studyUID);

			int steps = (int)(float(i)*100.0/(row_num-1.0));
			progressDlg->setValue(steps);
			{
				for(int i=0;i<100;i++){
					if(!myEventLoop.processEvents()){
						break;
					}
				}
			}
			

		}
	}

 
	delete progressDlg;

	//update
	onSearch();

	QApplication::restoreOverrideCursor();
}
void PatListMain::onDeleteSeries()
{
	int  ret_q = QMessageBox::question(this," ","   Are you sure you want to delete selected data ?",
				QMessageBox::Ok|QMessageBox::Cancel,
				QMessageBox::Cancel
				);
	if(ret_q != QMessageBox::Ok){
		return ;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int sel_study=m_StudyList->currentRow();
//	std::string studyUID = _Study_Objs[sel_study].m_studyUID;
	////#12 2012/03/26 K.Ko
	QString sel_study_uid = m_StudyList->getStudyUID(sel_study);
	if(sel_study_uid.size()<1) return;
	std::string studyUID = QString2Str(sel_study_uid);


#if 0
	int sel_series=m_SeriesList->currentRow();
//	std::string seriesUID = _Series_Objs[sel_series].m_seriesUID;
	////#12 2012/03/26 K.Ko
	QString sel_series_uid = m_SeriesList->getSeriesUID(sel_series);
	if(sel_series_uid.size()<1) return;
	std::string seriesUID = QString2Str(sel_series_uid);


	CPxDcmStudysManage DcmDbMan;
	DcmDbMan.delSeries(studyUID,seriesUID);
#else
	m_SeriesList->updateSelectedList();
	QVector<int> selectedRows = m_SeriesList->getSelectedRows();

	int row_num = selectedRows.size();
	//step-1
	//seriesUID vector の作成
	//1行削除後に行番号が変ってしまうため。
	std::vector<std::string> seriesUIDVector;
	{
		for(int i=0;i<row_num;i++){
			//std::string studyUID = _Study_Objs[selectedRows.at(i)].m_studyUID;
			//#12 2012/03/26 K.Ko
			QString sel_uid = m_SeriesList->getSeriesUID(selectedRows.at(i));
			if(sel_uid.size()>0){
				std::string seriesUID = QString2Str(sel_uid);
				seriesUIDVector.push_back(seriesUID);
		//		DcmDbMan.delStudy(studyUID);
			}
		}
	}

	//
	//step-2
	//seriesUIDで行削除
	CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	{
		row_num = seriesUIDVector.size();
		for(int i=0;i<row_num;i++){
			 
			std::string seriesUID = seriesUIDVector[i];
			DcmDbMan.delSeries(studyUID,seriesUID);
		}
	}
#endif

	//update
	int sel=m_StudyList->currentRow();
	//searchSeries(_Study_Objs[sel].m_studyUID);
	////#12 2012/03/26 K.Ko
	sel_study_uid = m_StudyList->getStudyUID(sel);
	if(sel_study_uid.size()<1) return;
	studyUID = QString2Str(sel_study_uid);
	searchSeries(studyUID);


	QApplication::restoreOverrideCursor();
}

std::string _cmd_fileName[] = {
	"onSeriesCmd1.txt",
	"onSeriesCmd2.txt",
	"onSeriesCmd3.txt",
	"onSeriesCmd4.txt",
};
QString PatListMain::checkMenuCmd(const QString &cmd_line) //#42 2012/12/20 menu launcher
{
	QString ret_cmd_line;

 
	if(cmd_line.contains("*@*&")){
		int pos =  cmd_line.indexOf("\\");
		QString cmd_main = cmd_line.mid(pos);
		ret_cmd_line = QString("\"")+QCoreApplication::applicationDirPath() + cmd_main;
		
	}else{
		ret_cmd_line = cmd_line;
	}

	return ret_cmd_line;
}
void PatListMain::readCdmLines()
{
	for(int i=0;i<SERIES_CMD_MAX;i++){

		SeriesCmdItem new_time;
		QFile file(Str2QString(_cmd_fileName[i]));
		if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			while(!file.atEnd()){
				QByteArray line = file.readLine();

				
				QString str_temp;
				if(line.size()>0){

					QString line_temp = line;
					int pos = line_temp.indexOf("=");
					QString key_name = line_temp.mid(0,pos);
					key_name = key_name.trimmed();
					
					QString	cmd_line = line_temp.mid(pos+1);
					cmd_line = cmd_line.trimmed();

					if(key_name == "MenuName"){
						new_time.m_menuName =QString2Str( cmd_line);
					}else
						if(key_name == "CmdLine"){
						QString menu_cmd = checkMenuCmd(cmd_line);//#42 2012/12/20 menu launcher
						new_time.m_cmdLine =QString2Str( menu_cmd);
					}else
						if(key_name == "CmdType"){
							new_time.m_type = cmd_line.toInt();
					}else
						if(key_name == "Default"){
							new_time.m_defaultCmd = cmd_line.toInt();
					}

				}
			}
				
			if(	(new_time.m_menuName.size()>0) &&
				(new_time.m_cmdLine.size()>0)
				){
					m_cmdLineList.push_back(new_time);
			}

		}

	}
}
void PatListMain::onSeriesCmd(int cmdID)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int sel_study=m_StudyList->currentRow();
	//std::string studyUID = _Study_Objs[sel_study].m_studyUID;
	////#12 2012/03/26 K.Ko
	QString sel_study_uid = m_StudyList->getStudyUID(sel_study);
	if(sel_study_uid.size()<1) return;
	std::string studyUID = QString2Str(sel_study_uid);


	int sel_series=m_SeriesList->currentRow();
//	std::string seriesUID = _Series_Objs[sel_series].m_seriesUID;
	////#12 2012/03/26 K.Ko
	QString sel_series_uid = m_SeriesList->getSeriesUID(sel_series);
	if(sel_series_uid.size()<1) return;
	std::string seriesUID = QString2Str(sel_series_uid);


	CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	DcmDbMan.queryImages(studyUID,seriesUID,_Images_Objs);
 
	
	if(_Images_Objs.size()>0) 
	{
		 
		std::string fileName = DcmDbMan.getImageFileName(_Images_Objs[0]);

		int cur_image_num = ui.vSFrameNo->value()-1;// 
		//#37 2012/10/01
		if(cur_image_num<0){
		//左クリックしないと、何も表示されない時がある。
			cur_image_num = 0;
		}
		if(m_cmdLineList[cmdID].m_type == 1){
			fileName = DcmDbMan.getImageFileName(_Images_Objs[cur_image_num]);
		}else{
			fileName = DcmDbMan.getSeriesFolder(studyUID,seriesUID );
		}

		std::string cmd_line ;

 
#if 0
		QString str_temp;
		std::string cmd_line_line =  m_cmdLineList[cmdID].m_cmdLine ;
		if(cmd_line_line.size()>0){

	 		str_temp.sprintf(Str2char(cmd_line_line) ,Str2char(fileName));


			cmd_line = QString2Str(str_temp);

		}
 
     

		if(cmd_line.size()>0){
			if(m_ProcSeriesCmd1){
				if(m_ProcSeriesCmd1->state() == QProcess::Running){
					m_ProcSeriesCmd1->terminate();
				}
				delete m_ProcSeriesCmd1;
			} 
			m_ProcSeriesCmd1 = new QProcess ;
			 
			 
			m_ProcSeriesCmd1->start(Str2QString(cmd_line));
			m_ProcSeriesCmd1->waitForStarted();
		}
#else
		 
	exeCmdLine(m_cmdLineList[cmdID], fileName);
#endif

	}

	QApplication::restoreOverrideCursor();

}
void PatListMain::onStudyDateChk()
{
 	bool useStudyDataFlag = (ui.cBStudyDate->checkState() == Qt::Checked) ;
//	enableStudyDate(useStudyDataFlag);
	if(useStudyDataFlag){
		ui.cBStudyDateSel->setCheckState(Qt::Unchecked);
	}
	switchStudyDate();
}
void PatListMain::onStudyDateSelChk()
{
	bool useSelStudyDataFlag = (ui.cBStudyDateSel->checkState() == Qt::Checked) ;
 
	if(useSelStudyDataFlag){
		ui.cBStudyDate->setCheckState(Qt::Unchecked);
	}

	switchStudyDate();
}
void PatListMain::switchStudyDate()
{
	bool useStudyDataFlag = (ui.cBStudyDate->checkState() == Qt::Checked) ;
	bool useSelStudyDataFlag = (ui.cBStudyDateSel->checkState() == Qt::Checked) ;

	ui.dEStudyDate1->setEnabled(useStudyDataFlag);
	ui.dEStudyDate2->setEnabled(useStudyDataFlag);
	//
	ui.cmBStudyDateSel->setEnabled(useSelStudyDataFlag);
 
}

#include "AEList.h"
 void PatListMain::onPushStudy()
 {

	 AEItemData AE_title;
	 CAEList AEList;

	 AEList.exec();

	 if(AEList.isOK()){
		 AE_title = AEList.getSelectedAE(); 

		 pushSelectStudy(&AE_title);
	 }
	 int x=0;
 }
 void PatListMain::onPushSeries()
 {
	 AEItemData AE_title;
	 CAEList AEList;

	 AEList.exec();

	 if(AEList.isOK()){
		 AE_title = AEList.getSelectedAE(); 

		 pushSelectSeries(&AE_title);
	 }
	 int x=0;

 }
 bool PatListMain::pushSelectStudy(const AEItemData *AE_title_data)
 {
	 
	 if(!AE_title_data) return false;
	 std::string AE_Title = AE_title_data->m_AETitle;

	 QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	m_StudyList->updateSelectedList();
 

 
	CPxDcmStudysManage DcmDbMan(m_CharacterSet);
	QVector<int> selectedRows = m_StudyList->getSelectedRows();

	int row_num = selectedRows.size();
	 
	 
	for(int i=0;i<row_num;i++){
		//std::string studyUID = _Study_Objs[selectedRows.at(i)].m_studyUID;
		//#12 2012/03/26 K.Ko
		QString sel_uid = m_StudyList->getStudyUID(selectedRows.at(i));
		if(sel_uid.size()>0){
			std::string studyUID = QString2Str(sel_uid);
//			if(!DcmDbMan.pushDICOMStudy(AE_Title,studyUID)){
			//#48
			if(!DcmDbMan.pushDICOMStudy(AE_title_data,studyUID)){
				return false;
			}
			 
		}
	}
	 
	QApplication::restoreOverrideCursor();

	 return true;
 }
	
 bool PatListMain::pushSelectSeries(const AEItemData *AE_title_data)
 {
	 if(!AE_title_data) return false;
	 std::string AE_Title = AE_title_data->m_AETitle;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int sel_study=m_StudyList->currentRow();
//	std::string studyUID = _Study_Objs[sel_study].m_studyUID;
	////#12 2012/03/26 K.Ko
	QString sel_study_uid = m_StudyList->getStudyUID(sel_study);
	if(sel_study_uid.size()<1) return false;
	std::string studyUID = QString2Str(sel_study_uid);


 
	m_SeriesList->updateSelectedList();
	QVector<int> selectedRows = m_SeriesList->getSelectedRows();

	int row_num = selectedRows.size();
	 
	CPxDcmStudysManage DcmDbMan(m_CharacterSet);

	std::vector<std::string> seriesUIDVector;
	{
		for(int i=0;i<row_num;i++){
			//std::string studyUID = _Study_Objs[selectedRows.at(i)].m_studyUID;
			//#12 2012/03/26 K.Ko
			QString sel_uid = m_SeriesList->getSeriesUID(selectedRows.at(i));
			if(sel_uid.size()>0){
				std::string seriesUID = QString2Str(sel_uid);
			 
	//			if(!DcmDbMan.pushDICOMSeries(AE_Title,studyUID,seriesUID)){
				//#48
				if(!DcmDbMan.pushDICOMSeries(AE_title_data,studyUID,seriesUID)){
					return false;
				}
			}
		}
	}

	 
 

	QApplication::restoreOverrideCursor();

	 return true;
 }
PxDicomInfor *PatListMain::getCurrentStudyInfo() const 
{
	PxDicomInfor *ret_p = 0;

	int sel_study=m_StudyList->currentRow();
	QString sel_study_uid = m_StudyList->getStudyUID(sel_study);

	int cur_study_no = findObjNumberFromStudyUID(QString2Str(sel_study_uid));
	if((cur_study_no<0) || (cur_study_no>=_Study_Objs.size())){
		return ret_p;//0
	}
	ret_p = &(_Study_Objs[cur_study_no]);
	return ret_p;
}
 //////////////////
void PatListMain::pageEntry()
{
}
void PatListMain::pageExit()
{
}
////////////////

//#39
void PatListMain::listupToday()
{
	ui.cBStudyDateSel->setChecked(true);
 	ui.cmBStudyDateSel->setCurrentIndex(kToday);
	ui.cmBStudyDateSel->setEnabled(true);
	onSearch();
	 

}
//#42 2012/12/20
void PatListMain::exeDefaultSeries(const std::string &studyUID,const std::string &seriesUID)
{
QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	CPxDcmStudysManage DcmDbMan(m_CharacterSet);

	do{
		int cmd_list_size = m_cmdLineList.size();
		if(cmd_list_size<1) break;

		SeriesCmdItem exe_cmd ;
		if(!pickCmdLine(exe_cmd,false /*oneImage*/ )){
			break ;
		}
		std::string folderName = DcmDbMan.getSeriesFolder(studyUID,seriesUID );
	 

		exeCmdLine(exe_cmd, folderName);
	}while(0);

QApplication::restoreOverrideCursor();

}
void PatListMain::exeCmdLine(const SeriesCmdItem &cmd, const std::string &arg)//#42 2012/12/20
{
	std::string cmd_line ;
	QString str_temp;
	std::string cmd_line_line =  cmd.m_cmdLine ;
	if(cmd_line_line.size()<1) {
		return ;
	}
	 
 	str_temp.sprintf(Str2char(cmd_line_line) ,Str2char(arg));

	cmd_line = QString2Str(str_temp);

	 
	if(cmd_line.size()>0){
		if(m_ProcSeriesCmd1){
			if(m_ProcSeriesCmd1->state() == QProcess::Running){
				m_ProcSeriesCmd1->terminate();
			}
			delete m_ProcSeriesCmd1;
		} 
		m_ProcSeriesCmd1 = new QProcess ;
		 
		 
		m_ProcSeriesCmd1->start(Str2QString(cmd_line));
		m_ProcSeriesCmd1->waitForStarted();
	}

}

bool PatListMain::pickCmdLine(SeriesCmdItem &outCmdItem,bool oneImage )//#42 2012/12/20
{
	int cmd_list_size = m_cmdLineList.size();
	if(cmd_list_size<1) return false;

	int index = -1;
	for(int i=0;i<cmd_list_size;i++){
		bool check_flag = false;
		if(oneImage){
			check_flag = (m_cmdLineList[i].m_type == 1);
		}else{
			check_flag = (m_cmdLineList[i].m_type == 0);
		}
		if(check_flag){
			if(index<0){
				//m_defaultCmd 指定がないとき
				index = i;
			}
			if(m_cmdLineList[i].m_defaultCmd == 1){
				index = i;
				break;
			}
		}
		
	}
	if(index<0) return false;

	outCmdItem = m_cmdLineList[index];

	return true;
}