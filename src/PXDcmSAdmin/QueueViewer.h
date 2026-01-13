#ifndef QUEUEVIEWER_GUI__H
#define QUEUEVIEWER_GUI__H
 
#include "ui_QueueViewer.h"
 #include "QueueDBProc.h" 
 
class CPXTableWidget;
class CQueueMan;
class CQueueViewer : public QDialog {
	Q_OBJECT
	public:
		CQueueViewer( QWidget *parent = 0);
	virtual ~CQueueViewer();	 
	virtual void resetColumnWidth(int w);
	 
	void onRefresh();
	void setupOwner(CQueueMan *owner){ m_pOwner = owner;};
	protected:
		Ui::QueueListMain ui;
	public slots:
		void onDelete()		{ doDelete();};
		void onResend()		{ doResend();};
		void onRestore()	{ doRestore();};
	 
	
protected:
	int getQueueID(int sel_row);
	QueuViewItem *getViewItem(int sel_row);
	virtual void resizeEvent(QResizeEvent *);
	//
	virtual void doDelete(){};
	virtual void doResend(){};
	virtual void doRestore(){};
	
	virtual int dispHeader(int col_index=0);//return last column index
	virtual void dispQueueList();
	virtual int dispQueueItem(int row,QueuViewItem &queue_item,int col_index=0);//return last column index
	/////////////
	virtual int getListColumn() { return m_QueueListColumn;};
	virtual void updateList()=0;
	//
	int m_QueueListColumn;
	CPXTableWidget *m_queueTableWidget;
	
	//
	std::vector<QueuViewItem> m_queueList;
	//
	CQueueMan * m_pOwner;
private:
	int m_curWidth;
	int m_refWidth;
	bool m_viewDispReady;
	//
	
};


class CSendQueueViewer : public  CQueueViewer
{
	Q_OBJECT
	public:
		CSendQueueViewer( QWidget *parent = 0);
	virtual ~CSendQueueViewer();	
	protected:
		virtual void doDelete();
		virtual void doRestore();
		//
		virtual void updateList();
		virtual int dispQueueItem(int row,QueuViewItem &queue_item,int col_index=0);//return last column index
	 
		int checkTryIndexNo(int reTryCount);
		
};


class CResultQueueViewer : public  CQueueViewer
{
	Q_OBJECT
	public:
		CResultQueueViewer( QWidget *parent = 0);
	virtual ~CResultQueueViewer();	
	void setupFailedOnly(bool b) { m_failedOnly = b;};
	protected:
		virtual void doDelete();
		virtual void doResend();
 
		//
		virtual void updateList();
		virtual int dispHeader(int col_index=0);//return last column index
		virtual int dispQueueItem(int row,QueuViewItem &queue_item,int col_index=0);//return last column index
	/////////////
		virtual int getListColumn() { return m_QueueListColumn;};
	/////
		bool m_failedOnly ;
};
#endif //QUEUEVIEWER_GUI__H


