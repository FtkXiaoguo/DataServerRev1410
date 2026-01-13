#ifndef QUEUEMAN_GUI__H
#define QUEUEMAN_GUI__H

#include "ui_QueueMan.h"
 
#include <QDialog> 
#include "TabPageCom.h"

class CQueueViewer; 
class CQueueMan : public TabPageComInf  {
	Q_OBJECT
	public:
		CQueueMan( QWidget *parent = 0);
	virtual ~CQueueMan();	 
//////////////////
	virtual void pageEntry();
	virtual void pageExit();
/////////////////
	private:
		Ui::QueueManage ui;
	public slots:
		 void onRefresh();
		 void onFailedOnly();
         void onAll();

	 
protected:
	 virtual void resizeEvent(QResizeEvent *);
private:
CQueueViewer *m_sendQueue ;
CQueueViewer *m_resultQueue ;	 
};


#endif //QUEUEMAN_GUI__H


