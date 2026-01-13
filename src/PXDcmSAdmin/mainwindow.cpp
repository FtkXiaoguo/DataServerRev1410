#include "AppVersion.h"
#include "QtHelper.h"

#include "mainwindow.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
 
#include "Setting.h"
#include "PatList.h"
#include "ServerInfo.h"
#include "QueueMan.h"
#include "AppVersion.h"
 

#include "assert.h"

CSetting *_Setting_=0;
PatListMain *_PatListMan_ = 0;
CServerInfo *_ServerInfo_ = 0;
CQueueMan	*_QueueMan_ = 0;

static std::vector<TabPageComInf*> _tab_page_list;
//#define TabName_PatList "PatList"
#define TabName_PatList "DataList"

#define TabName_Setting "Setting"
#define TabName_ServerInfo "ServerInfo"
#define TabName_QueueManager "QueueManager"

static QIcon *g_TabOffIcon = 0;
static QIcon *g_TabOnIcon = 0;

extern int g_user_mode  ; //#39
extern int g_first_display_tab_index  ;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
,m_TabPageReady(false)
,m_firstFlag(true)//#39
{
	//2012/11/19 adjust GUI
	m_ver_info = DCMAPP_VERSION_STRING;
	m_ver_info = QString(" Ver ")  + m_ver_info  ;
	
g_TabOffIcon	= new QIcon(QPixmap(":/rcFiles/rcFiles/TabOff.PNG"));
g_TabOnIcon		= new QIcon(QPixmap(":/rcFiles/rcFiles/TabOn.PNG"));

 
	m_TabPages = 0;
	ui.setupUi(this);
	
	 
	ui.tabWidget->clear();
#if 1
	
	///////////////////
//	QPixmap pixmap(10,40);
//	 QIcon icon(pixmap);
if(g_user_mode == 0){//#39
	 m_TabPages = 4;
}else{
	 m_TabPages = 2;
}
	 _tab_page_list.resize(m_TabPages);
	
	 int page_index = 0;
	_PatListMan_ = new PatListMain;
	ui.tabWidget->insertTab(page_index,_PatListMan_,*g_TabOffIcon,TabName_PatList);
	_tab_page_list[page_index] = _PatListMan_;
	page_index++;
	 
	_QueueMan_ = new CQueueMan(this);
	ui.tabWidget->insertTab(page_index,_QueueMan_,*g_TabOffIcon,TabName_QueueManager);
	_tab_page_list[page_index] = _QueueMan_;
	page_index++;

 if(g_user_mode == 0){//#39
	_Setting_ = new CSetting(this);
	ui.tabWidget->insertTab(page_index,_Setting_,*g_TabOffIcon,TabName_Setting);
	_tab_page_list[page_index] = _Setting_;
	page_index++;

	_ServerInfo_ = new CServerInfo(this);
	ui.tabWidget->insertTab(page_index,_ServerInfo_,*g_TabOffIcon,TabName_ServerInfo);
	_tab_page_list[page_index] = _ServerInfo_;
	page_index++;
}	
//
	assert(m_TabPages==page_index);
	

	ui.tabWidget->setIconSize(QSize(70,40));
	//
	m_TabPageReady = true;
	//showTabePage(0);
	showTabePage(g_first_display_tab_index);
	//
	setWindowTitle(DCMAPP_NAME);
	resize(1000,700);
#endif

 
}
 
 

void MainWindow::paintEvent(QPaintEvent *e /* event */)
{
	 QMainWindow::paintEvent(e);

	 QStatusBar *status_bar = statusBar();
	if(status_bar){
		//2012/11/19 adjust GUI
	 
		status_bar->showMessage(m_ver_info);
	}
}
void MainWindow::onTabChanged(int tab_index)
{
	//int x = tab_index;
	showTabePage(tab_index);
}
void MainWindow::showTabePage(int index)
{
	if(!m_TabPageReady) return;
	if((index<0) || (index>=m_TabPages)){
		return;
	};

	if( (m_curTabeIndex>=0) && (m_curTabeIndex<m_TabPages)){
		_tab_page_list[m_curTabeIndex]->pageExit();
		ui.tabWidget->setTabIcon(m_curTabeIndex,*g_TabOffIcon);
	}
	_tab_page_list[index]->pageEntry();
	ui.tabWidget->setCurrentIndex(index);
 	ui.tabWidget->setTabIcon(index,*g_TabOnIcon);

	m_curTabeIndex = index;
}

void MainWindow::loadStyleSheet(std::string fileName)
{
	try{
		QString qss_file_name = Str2QString(fileName);
		QFile file(qss_file_name);
		file.open(QFile::ReadOnly);
		QString styleSheet = QLatin1String(file.readAll());
		if(styleSheet.size()>1){
	    
			qApp->setStyleSheet(styleSheet);
		}
	}catch(...){
	}
 //    qApp->setStyle("motif");
//	qApp->setStyle("macintosh");
	qApp->setStyle("Cleanlooks");

}

void MainWindow::showEvent(QShowEvent *event)
{
	QMainWindow::showEvent( event);
//#39
	if(g_user_mode == 1){
		if(m_firstFlag){
			m_firstFlag = false;//‹N“®ŽžAˆê‰ñ‚Ì‚ÝŽÀs
			_PatListMan_->listupToday();
		}
	}
}