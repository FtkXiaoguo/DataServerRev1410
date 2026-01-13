#ifndef AUTO_ROUTING_GUI__H
#define AUTO_ROUTING_GUI__H

#include "ui_AutoRouting.h"

 
#include "PxDcmsAEManage.h"
 #include "TabPageCom.h" 

///////////////////////////////////////////////////////////
class MyQHeaderView : public QHeaderView
{
Q_OBJECT
	public:
	MyQHeaderView(Qt::Orientation orientation,QWidget *parent = 0);
     
	virtual ~MyQHeaderView();
//
protected:
//	virtual int sizeHintForRow(int row) const;
//  virtual int sizeHintForColumn(int column) const;
//	virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    virtual QSize sectionSizeFromContents(int logicalIndex) const;
};


///////////////////////////////////////////////////////////
class CMyTableView: public QTableView
{
Q_OBJECT
	public:

	CMyTableView(QWidget *parent = 0);
     
	virtual ~CMyTableView();
	void resetHeaderViewer();
	bool updateSelectedList();

	QVector<int> &getSelectedRows() { return m_selectedRows;};

	public slots:
		void onAddTag();

protected:
	QAction *m_Act_openAddTag;//#49
	QMenu *m_myMenu ; //#49 
	virtual void mousePressEvent(QMouseEvent *);//#49
	virtual void onMenu(const QPoint & pos);//#49
	QVector<int>	m_selectedRows;
	MyQHeaderView *m_MyHeaderView;
};

//
///////////////////////////////////////////////////////////

class CPxTagFilterModel;
class CPxRoutingPatternModel;
class CAutoRoutingConf : public TabPageComInf {
	Q_OBJECT
	public:
		CAutoRoutingConf( QWidget *parent = 0);
		~CAutoRoutingConf( );
		void init();
		void addTagFilter(std::string name);
//////////////////
	virtual void pageEntry();
	virtual void pageExit();
/////////////////	
		
	private:
		Ui::AutoRouting ui;
	public slots:
		 
		void onARPatApply();
		void onAddFilter();
		void onDeleteFilter();
		void onAddFilterEntry();
		void onDeleteFilterEntry();
		void onApplyFilterEntry();
	//	void onApplyFilter();
		void onTagFilterChanged(int);
		void onFilterNameChanged(QString);
		void onAddRoutingAE();
		void onDeleteRoutingAE();
		void onCheckAutoRouting();
		void onRefresh();
		 
protected:
	virtual void resizeEvent(QResizeEvent *);
	/////////
	void updateTagFilterTableViewer();
	void updateRoutingPatternTableViewer();
	//
	void updateDisplay();
	void resetTagFilterColumnWidth(int w);
	void resetRoutingPatternColumnWidth(int w);
	void updateTagFilter();
	void dispCurrentTagFilter();
	void dispCurrentRoutingPattern();
	//
	void updateRoutingPattern();
	void destroy();
	bool queryDB();
	
	// AEsMap m_AEList;
//	 CPxDcmsAEManage *m_RemoteAEManage;
	 CPxTagFilterModel *m_tagFilterModel;
	 CPxRoutingPatternModel *m_routingPatternModel;
	 //
	 std::vector<CPxTagFilterModel *> m_TagFilterList;
	 //
	 ///////////////////////////
	 CMyTableView *m_tbVFilterList;
	 CMyTableView *m_tbVAutoRoutingList;

private:
	enum TagFilterStatus
	{
		TagFilter_NoChanged = 0,
		TagFilter_AddNew = 1,
		TagFilter_Modify = 2,
	};

	 int m_TagFilterModifyStatus;
	 
 /////////////////////
	int m_curTagFilterViewerWidth;
	int m_refTagFilterViewerWidth;
	int m_curRoutingPatternViewerWidth;
	int m_refRoutingPatternViewerWidth;
};


#endif //AUTO_ROUTING_GUI__H


