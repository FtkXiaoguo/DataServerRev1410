#include "QueueMan.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>

 #include "QueueViewer.h"
  
#include "QString.h"

 
 
CQueueMan::CQueueMan(QWidget *parent) : TabPageComInf(parent)
{
	 
	ui.setupUi(this);
	 
	m_sendQueue = new CSendQueueViewer(this);
	ui.lySendQueue->addWidget(m_sendQueue);
	m_resultQueue = new CResultQueueViewer(this);
	ui.lyResultQueue->addWidget(m_resultQueue);

	//
	m_sendQueue->setupOwner(this);
	m_resultQueue->setupOwner(this);
}
CQueueMan::~CQueueMan()
{
	 delete m_sendQueue;
	 delete m_resultQueue;
}
	
void CQueueMan::resizeEvent(QResizeEvent *resize_e)
{
	TabPageComInf::resizeEvent( resize_e);

#if 0 // changed to CQueueViewer::resizeEvent
	const QSize &size = resize_e->size();
	 
	int view_w = size.width()/2 - 70;
	m_sendQueue->resetColumnWidth(view_w);
	m_resultQueue->resetColumnWidth(view_w);
#endif
}
void CQueueMan::onRefresh()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	((CResultQueueViewer*)m_resultQueue)->setupFailedOnly(ui.rBFailedOnly->isChecked());

	m_sendQueue->onRefresh();
	m_resultQueue->onRefresh();

	QApplication::restoreOverrideCursor();
}
 void CQueueMan::onFailedOnly()
 {
	 onRefresh();
 }
 void CQueueMan::onAll()
 {
	 onRefresh();
 }
//////////////////
void CQueueMan::pageEntry()
{
	onRefresh();
	 
}
void CQueueMan::pageExit()
{
	int x =0;
}
/////////////////