#include "PXTableWidget.h"
//#include "PatList.h"

#include "QtHelper.h"

#include <QProgressDialog>

#include <QtGui>
//#include <QPrinter>
#include <QtCore/QTimer>
#include <QtCore/qstring.h>

 
#include "AEsGUI.h"
#include "PxDcmStudysManage.h"
 

///////////////////

CPXTableWidget::CPXTableWidget(QWidget *parent) : QTableWidget(parent)
{
	m_last_column_sort_time = new QTime;
	*m_last_column_sort_time = QTime::currentTime();

	m_evenLineColor =	QColor(209,231,213);//QColor(220,255,200);
	m_oddLineColor	=	QColor(255,255,255);
	
	 

	for(int i=0;i<LIST_COLUMN_MAX;i++){
		m_columnSortOrder[i] = -1;;
	}

	m_currentSortColumn = -1;

	m_sordColumnFlag = true;
	
	
}
CPXTableWidget::~CPXTableWidget()
{
	delete m_last_column_sort_time;
}
 
void CPXTableWidget::createActions()
{
	
	 
	connect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));

	connect(verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));
	//
	 
}
void CPXTableWidget::selectColumn(int sel_column)
{
	bool do_flag = false;
 
	if(m_last_column_sort_time->elapsed() > 500){
		do_flag =true;
	}
	m_last_column_sort_time->start();

	if(!do_flag) {
		return ;
	}


	if(!m_sordColumnFlag) return;

	if(sel_column <0) return;
	/*
	* set up Sort on Column
	*/
	int col = sel_column;
	if(m_columnSortOrder[col] == Qt::AscendingOrder){
//		sortByColumn(col,Qt::DescendingOrder); 
		m_columnSortOrder[col] = Qt::DescendingOrder;
	}else{
//		sortByColumn(col,Qt::AscendingOrder); 
		m_columnSortOrder[col] = Qt::AscendingOrder;
	}
	m_currentSortColumn = col;
	//do it
	updateSortColumn();
}
void CPXTableWidget::selectRow(int sel_row)
{
	int row = sel_row;
}
 
void CPXTableWidget::setupTableWidget()
{


	setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
	//

	createActions();

}
void CPXTableWidget::mousePressEvent(QMouseEvent *e)
{
	Qt::MouseButton button = e->button() ;
    Qt::MouseButtons buttonStates = e->buttons() ;

	if(button == Qt::RightButton){
		onMenu(e->pos());
	}
	QTableWidget::mousePressEvent(e);
}
void CPXTableWidget::mouseReleaseEvent(QMouseEvent *e)
{
	QTableWidget::mouseReleaseEvent(e);
}
void CPXTableWidget::onMenu(const QPoint & pos)
{
	
}


bool CPXTableWidget::updateSelectedList()
{
	m_selectedRows.clear();

 	QModelIndexList sel_list =  selectedIndexes();

 
	int sel_size = sel_list.size();

	 
	/*
	* 行、列の配置順番は選択方法／範囲により、変る。
	*/
	for(int index=0;index<sel_size;	){
		QModelIndex model_index = sel_list.at(index);
		int col = model_index.column();
		int row = model_index.row();
		 
		if(!m_selectedRows.contains(row)){
			m_selectedRows.append(row);
		}
		 
		index ++;
	}
 
	sel_list.clear();
	return true;
}

void CPXTableWidget::paintEvent(QPaintEvent *event)
{
	QTableWidget::paintEvent(event);
	drawLineColor();
}
void CPXTableWidget::drawLineColor()
{
	///
	 
	///
	 QColor color ;
	for (int i = 0;i < rowCount();i++)
	{
		if (i % 2 == 0)
		{
	//		color = QColor(152,222,255);
			color = m_evenLineColor;//QColor(220,255,200);
		}else{
			color = m_oddLineColor;//QColor(255,255,255);
		}
		//
		for (int j = 0;j < columnCount();j++)
		{
			QTableWidgetItem *my_item = item(i,j);
			if (my_item)
			{
			//	const QColor color = QColor(152,222,255);
				my_item->setBackgroundColor(color);
			}

		}
		 
	}

}



void CPXTableWidget::updateSortColumn()
{
	if(!m_sordColumnFlag) return;

	if(m_currentSortColumn<0) return;
	/*
	* set up Sort on Column
	*/
	int col = m_currentSortColumn;

	if(m_columnSortOrder[col] == Qt::AscendingOrder){
		sortByColumn(col,Qt::AscendingOrder); 
	}else{
		sortByColumn(col,Qt::DescendingOrder); 
	} 
	 
	QHeaderView *header = horizontalHeader();
	header->setSortIndicatorShown(true);

}
void CPXTableWidget::setDefaultSortColumn(int col,bool AscendingOrder)
{
	if(!m_sordColumnFlag) return;

	m_currentSortColumn = col;

	m_columnSortOrder[col] = AscendingOrder? Qt::AscendingOrder : Qt::DescendingOrder;
}
/////////////
