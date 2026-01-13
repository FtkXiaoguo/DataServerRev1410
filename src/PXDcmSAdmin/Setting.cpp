#include "Setting.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
#include <QtCore/QTimer>
#include <QtCore/qstring.h>

#include "AEsGUI.h"
#include "AutoRouting.h"
#include "assert.h"

static std::vector<TabPageComInf*> _tab_page_list;
#define TabName_AEs "AEs"
#define TabName_AutoRouting "AutoRouting"

static QIcon *g_TabOffIcon = 0;
static QIcon *g_TabOnIcon = 0;

CAEsMan *_AEsMan_=0;
CAutoRoutingConf *_ARConf_ = 0;
CSetting::CSetting(QWidget *parent) : TabPageComInf(parent)
,m_TabPageReady(false)
{
	
g_TabOffIcon	= new QIcon(QPixmap(":/rcFiles/rcFiles/TabOff.PNG"));
g_TabOnIcon		= new QIcon(QPixmap(":/rcFiles/rcFiles/TabOn.PNG"));

	m_TabPages = 0;

	ui.setupUi(this);
	 
	m_TabPages = 2;
	 _tab_page_list.resize(m_TabPages);
	
	ui.tabWidget->clear();

	int page_index = 0;
//	QPixmap pixmap(10,40);
//	 QIcon icon(pixmap);
	_AEsMan_ = new CAEsMan(this);
	_AEsMan_->init();
	ui.tabWidget->insertTab(page_index,_AEsMan_,*g_TabOffIcon,"AEs");
	_tab_page_list[page_index] = _AEsMan_;
	page_index++;
	//
	_ARConf_ = new CAutoRoutingConf(this);
	_ARConf_->init();
	ui.tabWidget->insertTab(page_index,_ARConf_,*g_TabOffIcon,"AutoRouting");
	_tab_page_list[page_index] = _ARConf_;
	page_index++;

	assert(m_TabPages==page_index);

	ui.tabWidget->setIconSize(QSize(70,35));

	m_TabPageReady = true;
	showTabePage(0);
	 
}
 
void CSetting::onTabChanged(int tab_index)
{
	int x = tab_index;
	showTabePage(tab_index);
}

//////////////////
void CSetting::pageEntry()
{
	int x =0;
}
void CSetting::pageExit()
{
	int x =0;
}
/////////////////

void CSetting::showTabePage(int index)
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