#include "PXPatientList.h"
#include "PatList.h"

#include "QtHelper.h"

#include <QProgressDialog>

#include <QtGui>
//#include <QPrinter>
#include <QtCore/QTimer>
#include <QtCore/qstring.h>
#include <QMenu.h>
 
#include "AEsGUI.h"
#include "PxDcmStudysManage.h"
 
extern int g_user_mode  ; //#39
///////////////////

CPXPatientList::CPXPatientList(QWidget *parent) : CPXTableWidget(parent)
{
	m_type = ListType_Unknown;
	m_myMenu = 0;
	
#if 0
	for(int i=0;i<LIST_COLUMN_MAX;i++){
		m_columnSortOrder[i] = -1;;
	}

	m_currentSortColumn = -1;
#endif
	
}
CPXPatientList::~CPXPatientList()
{
}
void CPXPatientList::createActions()
{
	m_Act_deleteStudy = new QAction(tr("DeleteStudy"),this);
	connect(m_Act_deleteStudy, SIGNAL(triggered()), this, SLOT(onDeleteStudy()));
	//
	m_Act_deleteSeries = new QAction(tr("DeleteSeries"),this);
	connect(m_Act_deleteSeries, SIGNAL(triggered()), this, SLOT(onDeleteSeries()));
	//
	////////////
	m_Act_pushStudy = new QAction(tr("PushStudy"),this);
	connect(m_Act_pushStudy, SIGNAL(triggered()), this, SLOT(onPushStudy()));
	//
	m_Act_pushSeries = new QAction(tr("PushSeries"),this);
	connect(m_Act_pushSeries, SIGNAL(triggered()), this, SLOT(onPushSeries()));

 
	////////

	if(m_parentListMan){

		
		const CmdLineList &menu_cmd = m_parentListMan->getCmdLine();

		int menu_size = menu_cmd.size();
		for(int i=0;i<SERIES_CMD_MAX;i++){
			if(i<menu_size){
				m_Act_OnSeriesCmd[i] = new QAction(tr(Str2char(menu_cmd[i].m_menuName)),this);
			}else{
				m_Act_OnSeriesCmd[i] = new QAction(tr("OnSeriesCmde"),this);
			}
		}

		 
		connect(m_Act_OnSeriesCmd[0], SIGNAL(triggered()), this, SLOT(onSeriesCmd0()));
		//
		 
		connect(m_Act_OnSeriesCmd[1], SIGNAL(triggered()), this, SLOT(onSeriesCmd1()));
		//
		 
		connect(m_Act_OnSeriesCmd[2], SIGNAL(triggered()), this, SLOT(onSeriesCmd2()));
		//
		 
		connect(m_Act_OnSeriesCmd[3], SIGNAL(triggered()), this, SLOT(onSeriesCmd3()));
		//
		 
		 
	}
	 
	CPXTableWidget::createActions();
	 
}

#if 0
void CPXPatientList::selectColumn(int sel_column)
{
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
void CPXPatientList::selectRow(int sel_row)
{
	int row = sel_row;
}
#endif
 
void CPXPatientList::setupTableWidget(ListType type,PatListMain *parentListMan)
{
	m_type = type;

	setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
	//
	m_parentListMan = parentListMan;

	//
	CPXTableWidget::setupTableWidget();

}
 
void CPXPatientList::onMenu(const QPoint & pos)
{
	if(!m_myMenu) m_myMenu = new QMenu(this);
	m_myMenu->clear();
	switch(m_type){
	case ListType_Study:
		m_myMenu->addAction(m_Act_deleteStudy);
		if(g_user_mode == 1){//#39
			m_Act_deleteStudy->setEnabled(false);
		}
		m_myMenu->addAction(m_Act_pushStudy);
		break;
	case ListType_Series:
		{
		m_myMenu->addAction(m_Act_deleteSeries);
		if(g_user_mode == 1){//#39
			m_Act_deleteSeries->setEnabled(false);
		}
		m_myMenu->addAction(m_Act_pushSeries);
//		m_myMenu->addAction(m_Act_OnSeriesCmd[0]);
		if(m_parentListMan){
			const CmdLineList &menu_cmd = m_parentListMan->getCmdLine();

			int menu_size = menu_cmd.size();

			for(int i=0;i<menu_size;i++){
				m_myMenu->addAction(m_Act_OnSeriesCmd[i]);
			}
		}
		}
		break;
	}
	m_myMenu->popup(this->mapToGlobal(pos),m_Act_deleteStudy); 
}
void CPXPatientList::onDeleteStudy()
{
	if(m_parentListMan){
		m_parentListMan->onDeleteStudy();
	}
	 
}
void CPXPatientList::onDeleteSeries()
{
	if(m_parentListMan){
		m_parentListMan->onDeleteSeries();
		
	}
}
void CPXPatientList::onPushStudy()
{
	if(m_parentListMan){
		m_parentListMan->onPushStudy();
		
	}
}
void CPXPatientList::onPushSeries()
{
	if(m_parentListMan){
		m_parentListMan->onPushSeries();
		
	}
}

void CPXPatientList::onSeriesCmd0( )
{
 
if(m_parentListMan){
	m_parentListMan->onSeriesCmd(0);
}
}
void CPXPatientList::onSeriesCmd1( )
{
 
if(m_parentListMan){
	m_parentListMan->onSeriesCmd(1);
}
}
void CPXPatientList::onSeriesCmd2( )
{
 
if(m_parentListMan){
	m_parentListMan->onSeriesCmd(2);
}
}
void CPXPatientList::onSeriesCmd3( )
{
 
if(m_parentListMan){
	m_parentListMan->onSeriesCmd(3);
}
}

#if 0
bool CPXPatientList::updateSelectedList()
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



void CPXPatientList::paintEvent(QPaintEvent *event)
{
	QTableWidget::paintEvent(event);
	drawLineColor();
}

void CPXPatientList::drawLineColor()
{
	 QColor color = QColor(152,222,255);
	for (int i = 0;i < rowCount();i++)
	{
		if (i % 2 == 0)
		{
	//		color = QColor(152,222,255);
			color = QColor(220,255,200);
		}else{
			color = QColor(255,255,255);
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

#endif

QString CPXPatientList::getStudyUID(int row)
{
	QString ret_string;
	int uid_col = columnCount()-1; //#12 2012/03/26 K.Ko 最後の列にStudyUID追加／非表示
	QTableWidgetItem * study_uid_item = QTableWidget::item (  row, uid_col ) ;

	if(study_uid_item){
		ret_string = study_uid_item->text();
	}
	return ret_string;

}
QString CPXPatientList::getSeriesUID(int row)
{
	QString ret_string;
	int uid_col = columnCount()-1; //#12 2012/03/26 K.Ko 最後の列にSeriesUID追加／非表示
	QTableWidgetItem * series_uid_item = QTableWidget::item (  row, uid_col ) ;

	if(series_uid_item){
		ret_string = series_uid_item->text();
	}
	return ret_string;

}

#if 0
void CPXPatientList::updateSortColumn()
{
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
void CPXPatientList::setDefaultSortColumn(int col,bool AscendingOrder)
{
	m_currentSortColumn = col;

	m_columnSortOrder[col] = AscendingOrder? Qt::AscendingOrder : Qt::DescendingOrder;
}
#endif
/////////////
