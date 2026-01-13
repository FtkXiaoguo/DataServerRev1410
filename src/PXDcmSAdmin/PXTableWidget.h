#ifndef PX_TABLEWIDGET_H
#define PX_TABLEWIDGET_H

 
#include <QTableWidget>


#define LIST_COLUMN_MAX 64

#define TABLE_W_HEADER_HEIGHT (30)
class PatListMain;
class CPXTableWidget: public QTableWidget
{
Q_OBJECT
	public:

	CPXTableWidget(QWidget *parent = 0);
     
    virtual ~CPXTableWidget();
	void setupLineColor(const QColor &odd,const QColor &even) {
		m_evenLineColor = odd;
		m_oddLineColor	= even;
	}
	virtual void setupTableWidget();

	bool updateSelectedList();

	QVector<int> &getSelectedRows() { return m_selectedRows;};
	public slots:
		
		//
		
	void selectColumn(int column);
	void selectRow(int row);
 
	 
	//
	int getCurrentSortColumn() const { return m_currentSortColumn;};
	void updateSortColumn();
	void setDefaultSortColumn(int col,bool AscendingOrder=true);
	
protected:
	void drawLineColor();
 
	virtual void createActions();
	virtual void onMenu(const QPoint & pos);
	////
	virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void paintEvent(QPaintEvent *event);
	//
	 
	//
 
	int m_columnSortOrder[LIST_COLUMN_MAX];
	int m_currentSortColumn;

	QVector<int>	m_selectedRows;
	//
	QColor m_evenLineColor;
	QColor m_oddLineColor;
	//
	bool	m_sordColumnFlag;
	//
	QTime  *m_last_column_sort_time ;
};

 

#endif


