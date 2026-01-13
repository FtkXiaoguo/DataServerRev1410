#include "QueueViewer.h"
#include "PxQueue.h"
#include "QueueMan.h"

#include "PXTableWidget.h"
#include "QtHelper.h"

#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>
#include <QSettings>
  
#include "QString.h"
 #include "assert.h"

 
extern int g_user_mode  ; //#39

CQueueViewer::CQueueViewer(QWidget *parent) : QDialog(parent)
,m_viewDispReady(false)
,m_pOwner(0)
{
	m_refWidth = 552;//420;
	m_curWidth = m_refWidth;

	m_QueueListColumn = 7+1;//Ref #12 最後の列にID追加／非表示
	ui.setupUi(this); 
	m_queueTableWidget = new CPXTableWidget(this);
	ui.verticalLayout->insertWidget(0,m_queueTableWidget);

	m_queueTableWidget->setupLineColor(
				QColor(207,233,222),//QColor(182,207,184),
				QColor(240,240,240)
			  );

	if(g_user_mode == 1){//#39
		ui.pBDelete->setDisabled(true);
	}

}
CQueueViewer::~CQueueViewer()
{
	 
}
void CQueueViewer::onRefresh(){
	int x= 0;
	updateList();
	
}
void CQueueViewer::resetColumnWidth(int w)
{
	if(!m_viewDispReady) return;

	float chg_rate = (float)w/m_refWidth;
//	chg_rate = 1.0; //for check refWidth;

	//setup the header Height
	m_queueTableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(100,TABLE_W_HEADER_HEIGHT));


	m_curWidth = w;

	int col_index = 0;
	//Status
	 
	m_queueTableWidget->setColumnWidth(col_index,20*chg_rate);
	col_index++;

	//Level
	m_queueTableWidget->setColumnWidth(col_index,20*chg_rate);
	col_index++;

	//PatientName
	m_queueTableWidget->setColumnWidth(col_index,140*chg_rate);
	col_index++;

	//StudyDate
	m_queueTableWidget->setColumnWidth(col_index,120*chg_rate);
	col_index++;
	
	//SeriesNumber
	m_queueTableWidget->setColumnWidth(col_index,60*chg_rate);
	col_index++;

	//Images
	m_queueTableWidget->setColumnWidth(col_index,50*chg_rate);
	col_index++;

	//Destination AE
	m_queueTableWidget->setColumnWidth(col_index,100*chg_rate);
	col_index++;

	//ID
	//Ref #12 最後の列にID追加／非表示
	col_index++;
}
int CQueueViewer::dispHeader(int col_index)
{
	m_queueTableWidget->setColumnCount(getListColumn());//m_QueueListColumn);

 
	QTableWidgetItem *item;

	/*
	* 以下のsetColumnWidthは一時的なもの。
	* 実際のサイズは　resetColumnWidth で調整する。
	*/
	//Status
	item = new QTableWidgetItem("S");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,20);
	col_index++;

	//Level
	item = new QTableWidgetItem("L");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,20);
	col_index++;

	//PatientName
	//item = new QTableWidgetItem("PatientName");
	//#142_NoPatientName_NoComment
	//PatientID
	item = new QTableWidgetItem("PatientID");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,120);
	col_index++;

	//StudyDate
	item = new QTableWidgetItem("Date");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,100);
	col_index++;
	
	//SeriesNumber
	item = new QTableWidgetItem("SeriesNo.");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,60);
	col_index++;

	//Images
	item = new QTableWidgetItem("Images");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,50);
	col_index++;

	//Destination AE
	item = new QTableWidgetItem("AE");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,50);
	col_index++;

	//ID
	//Ref #12 最後の列にID追加／非表示
	item = new QTableWidgetItem("ID");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,10);
  	m_queueTableWidget->setColumnHidden(col_index,true);
	col_index++;

 
	m_viewDispReady = true;
	resetColumnWidth(m_curWidth);
	return col_index;
}


int CQueueViewer::dispQueueItem(int row,QueuViewItem &queue_item,int col_index)
{
static QIcon StudyIcon(QPixmap(":/rcFiles/rcFiles/studyLevel.PNG"));//StudyPixmap);
static QIcon SeriesIcon(QPixmap(":/rcFiles/rcFiles/seriesLevel.PNG"));//SeriesPixmap); 
static QIcon BlockIcon(QPixmap(":/rcFiles/rcFiles/blockLevel.PNG"));//SeriesPixmap); 

	QTableWidgetItem *item;	 
	//Level
	switch(queue_item.m_Level){
	case CPxQueueEntry::PxQueueLevel_Study:
		item = new QTableWidgetItem(StudyIcon,"study");
		break;
	case CPxQueueEntry::PxQueueLevel_Series:
		item = new QTableWidgetItem(SeriesIcon,"series");
		break;
	case CPxQueueEntry::PxQueueLevel_Image:
	case CPxQueueEntry::PxQueuelevel_EntryFile:
		item = new QTableWidgetItem(BlockIcon,"block");
		break;
	}
	m_queueTableWidget->setItem(row ,col_index++, item );

	//PatientName
	//item = new QTableWidgetItem(Str2QString(queue_item.m_patientName));
	//#142_NoPatientName_NoComment
	//PatientID //2023_03_16
	item = new QTableWidgetItem(Str2QString(queue_item.m_patientID));
	m_queueTableWidget->setItem(row ,col_index++, item );

	//StudyDate
	 
	item = new QTableWidgetItem(Str2QString(queue_item.m_dispDate));
	 
	m_queueTableWidget->setItem(row ,col_index++, item );

	//SeriesNumber
	if(queue_item.m_Level == CPxQueueEntry::PxQueueLevel_Study){
		item = new QTableWidgetItem("----");
	}else{
		item = new QTableWidgetItem(Str2QString(queue_item.m_seriesNumber));
	}
	m_queueTableWidget->setItem(row ,col_index++, item );

	//Images
	if(queue_item.m_Level == CPxQueueEntry::PxQueueLevel_Study){
		item = new QTableWidgetItem("----");
	}else{
		item = new QTableWidgetItem(Str2QString(queue_item.m_images));
	}
	m_queueTableWidget->setItem(row ,col_index++, item );

	//Destination AE
	item = new QTableWidgetItem(Str2QString(queue_item.m_destinationAE));
	m_queueTableWidget->setItem(row ,col_index++, item );

	//ID
	//Ref #12 最後の列にID追加／非表示
	char _str_buff[1128];
	sprintf(_str_buff,"%d",queue_item.m_QueueID);
	item = new QTableWidgetItem(_str_buff);
	m_queueTableWidget->setItem(row ,col_index++, item );

	return col_index;
}
void CQueueViewer::dispQueueList()
{
	QTableWidgetItem *item;
	int size = m_queueList.size();
	m_queueTableWidget->setRowCount(size);

 
	//
	int col_index = getListColumn();
	for(int i=0;i<size;i++){
		
		m_queueTableWidget->setRowHeight(i, 20);
		
		col_index = 0;
		col_index = dispQueueItem(i,m_queueList[i],col_index);


	}
	assert(getListColumn() == col_index);

	m_queueTableWidget->updateSortColumn();
}

void CQueueViewer::resizeEvent(QResizeEvent *resize_e)
{
	QDialog::resizeEvent( resize_e);

	const QSize &size = resize_e->size();
	 
	resetColumnWidth(size.width());
	 
}
int CQueueViewer::getQueueID(int sel_row)
{
	int ret_val = -1;
	int queue_id_col = getListColumn()-1; //Ref #12 最後の列にID追加／非表示
	QTableWidgetItem * queue_id_item = m_queueTableWidget->item( sel_row, queue_id_col ) ;

	if(m_queueTableWidget){
		ret_val = queue_id_item->text().toInt();
	}
	return ret_val;

}
//
 
QueuViewItem * CQueueViewer::getViewItem(int sel_row)
{
	int queue_id = getQueueID(sel_row);
	if(queue_id<0) return 0;

	QueuViewItem *ret_item = 0;
	int list_size = m_queueList.size();
	for(int i=0;i<list_size;i++){
		if(m_queueList[i].m_QueueID == queue_id){
			ret_item = &(m_queueList[i]);
			break;
		}
	} 
	return ret_item;

}

///
/////////////////
///
CSendQueueViewer::CSendQueueViewer( QWidget *parent ):
CQueueViewer(parent)
{
//	ui.pBDelete->hide();
	ui.pBResend->hide();
}
CSendQueueViewer::~CSendQueueViewer()
{
}
void CSendQueueViewer::updateList()
{
	CQueueDBProc QueueDB;
	 
	QueueDB.getSendQueueList(m_queueList);

	int size = m_queueList.size();

	//
	dispHeader();
	dispQueueList();

	m_queueTableWidget->setupTableWidget();

}
 int CSendQueueViewer::checkTryIndexNo(int reTryCount)
 {
	 int ret_no = 0;
	 if(reTryCount < 2) 
		 ret_no = 0;
	 else if (reTryCount < 4)
		 ret_no = 1;
	 else
		 ret_no = 2;

	 return ret_no;
 }
int CSendQueueViewer::dispQueueItem(int row,QueuViewItem &queue_item,int col_index ) //return last column index
{
static QIcon NormalIcon(QPixmap(":/rcFiles/rcFiles/ok.PNG"));
static QIcon ProcIcon(QPixmap(":/rcFiles/rcFiles/processing.PNG"));
static QIcon ProcErrIcon(QPixmap(":/rcFiles/rcFiles/proc_error.PNG"));
static QIcon Retry1Icon(QPixmap(":/rcFiles/rcFiles/retry1.PNG"));
static QIcon Retry2Icon(QPixmap(":/rcFiles/rcFiles/retry2.PNG"));
static QIcon Retry3Icon(QPixmap(":/rcFiles/rcFiles/retry3.PNG"));

	QTableWidgetItem *item;	 
	col_index = 0;
	//Status
	switch(queue_item.m_status){
		case CPxQueueEntry::PxQueueStatus_Standby:
			{
				item = new QTableWidgetItem(NormalIcon,"Normal");
			}
		break;
		case CPxQueueEntry::PxQueueStatus_Failed:
			{
				int index_no = checkTryIndexNo(queue_item.m_retryCount);
				switch(index_no){
					case 0:
						item = new QTableWidgetItem(Retry1Icon,"retry1");
						break;
					case 1:
						item = new QTableWidgetItem(Retry2Icon,"retry2");
						break;
					case 2:
					default:
						item = new QTableWidgetItem(Retry3Icon,"retry3");
						break;
				}
				
				
			}

		break;
		case CPxQueueEntry::PxQueueStatus_Processing:
			{
 
				if(QueuViewItem::isBrokenEntry(queue_item)){
 
					item = new QTableWidgetItem(ProcErrIcon,"ProcErr");
				}else{
					item = new QTableWidgetItem(ProcIcon,"Processing");
				}
			}
		break;
	}
 
	m_queueTableWidget->setItem(row ,col_index++, item );

	return CQueueViewer::dispQueueItem( row,queue_item,col_index );
}

 
void CSendQueueViewer::doDelete()
{
	m_queueTableWidget->updateSelectedList();

	QVector<int> &selected_rows = m_queueTableWidget->getSelectedRows();

	int sel_size = selected_rows.size();
	if(sel_size<1) return;

	std::vector<QueuViewItem> proce_list;
	for(int i=0;i<sel_size;i++){
//		const QueuViewItem &item = m_queueList[selected_rows[i]];
		//Ref #12 最後の列にID追加／非表示
		//ソートしたケース
		const QueuViewItem *item = this->getViewItem(selected_rows[i]);
		if(item==0){
			continue;
		}
		bool do_flag = true;
		if(item->m_status == CPxQueueEntry::PxQueueStatus_Processing){
			 do_flag = false;
		}
		if(do_flag){
			proce_list.push_back(*item);
		}
	}
	 
	////
	CQueueDBProc QueueDB;
	 
	QueueDB.deleteSendQueueList(proce_list);

	if(m_pOwner){
		m_pOwner->onRefresh();
	}

}

void CSendQueueViewer::doRestore()
{
	m_queueTableWidget->updateSelectedList();

	QVector<int> &selected_rows = m_queueTableWidget->getSelectedRows();

	int sel_size = selected_rows.size();
	if(sel_size<1) return;

	std::vector<QueuViewItem> proce_status_list;
	std::vector<QueuViewItem> proce_priority_list;
	for(int i=0;i<sel_size;i++){
	//	const QueuViewItem &item = m_queueList[selected_rows[i]];
		//Ref #12 最後の列にID追加／非表示
		//ソートしたケース
		const QueuViewItem *item = this->getViewItem(selected_rows[i]);
		if(item==0){
			continue;
		}

		bool do_chg_status_flag = false;
		bool do_chg_priority_flag = false;
		if(item->m_status == CPxQueueEntry::PxQueueStatus_Processing){
			if(QueuViewItem::isBrokenEntry(*item)){
				do_chg_status_flag = true;
			}
			 
		}else if(item->m_status == CPxQueueEntry::PxQueueStatus_Failed){
		//Retry中のEntryのPriorityを上げる。
			do_chg_priority_flag = true;
		}
		if(do_chg_status_flag){
			proce_status_list.push_back(*item);
		}
		if(do_chg_priority_flag){
			proce_priority_list.push_back(*item);
		}
	}
	 
	////
	CQueueDBProc QueueDB;
	 
	QueueDB.restoreSendQueueList(proce_status_list,proce_priority_list);

	if(m_pOwner){
		m_pOwner->onRefresh();
	}

}
////////////////////////////
///
CResultQueueViewer::CResultQueueViewer( QWidget *parent ):
CQueueViewer(parent)
,m_failedOnly(false)
{
	ui.pBRestore->hide();
}
CResultQueueViewer::~CResultQueueViewer()
{
}
void CResultQueueViewer::updateList()
{
	CQueueDBProc QueueDB;
	 
	QueueDB.getResultQueueList(m_queueList,m_failedOnly);

	
	{
		std::vector<QueuViewItem> queueList;		
		QueueDB.getResultQueueList(queueList, true);
		int size = queueList.size();

		//regedit operator
		QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\PreXion\\PXDataServer", QSettings::NativeFormat);  
		const int nQueueProcError = reg.value("QueueProcError",1).toInt();
		int nValue = 0;
		if (size > 0) nValue = 1;	
		reg.setValue("QueueProcError",nValue);
		qDebug()<<"m_queueList.size()="<<size<<" SOFTWARE\\PreXion\\PXDataServer\\QueueProcError="<<nValue;
	}
	//
	dispHeader();
	dispQueueList();
	//
	m_queueTableWidget->setupTableWidget();
}
int CResultQueueViewer::dispHeader(int col_index)//return last column index
{
#if 0
	m_queueTableWidget->setColumnCount(getListColumn());//m_QueueListColumn);

 
	QTableWidgetItem *item;

	col_index = 0;
	//Status
	item = new QTableWidgetItem("S");
	m_queueTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_queueTableWidget->setColumnWidth(col_index,20);
	col_index++;
#endif
	return CQueueViewer::dispHeader(col_index);
}
int CResultQueueViewer::dispQueueItem(int row,QueuViewItem &queue_item,int col_index ) //return last column index
{
static QIcon OKIcon(QPixmap(":/rcFiles/rcFiles/ok.PNG"));//StudyPixmap);
static QIcon ErrIcon(QPixmap(":/rcFiles/rcFiles/error.PNG"));//SeriesPixmap); 

	QTableWidgetItem *item;	 
	col_index = 0;
	//Status
 
	if(queue_item.m_status == CPxQueueEntry::PxQueueStatus_Failed){
		item = new QTableWidgetItem(ErrIcon,"Failed");
	}else{
		item = new QTableWidgetItem(OKIcon,"Sucess");
	}
 
	m_queueTableWidget->setItem(row ,col_index++, item );

	return CQueueViewer::dispQueueItem( row,queue_item,col_index );
}

void CResultQueueViewer::doDelete()
{
	m_queueTableWidget->updateSelectedList();

	QVector<int> &selected_rows = m_queueTableWidget->getSelectedRows();

	int sel_size = selected_rows.size();
	if(sel_size<1) return;

	std::vector<QueuViewItem> proce_list;
	for(int i=0;i<sel_size;i++){
		//const QueuViewItem &item = m_queueList[selected_rows[i]];
		//Ref #12 最後の列にID追加／非表示
		//ソートしたケース
		const QueuViewItem *item = this->getViewItem(selected_rows[i]);
		if(item==0){
			continue;
		}

		bool do_flag = true;
		 
		if(do_flag){
			proce_list.push_back(*item);
		}
	}
	 
	////
	CQueueDBProc QueueDB;
	 
	QueueDB.deleteResultQueueList(proce_list);

	if(m_pOwner){
		m_pOwner->onRefresh();
	}
}
void CResultQueueViewer::doResend()
{
	assert(m_pOwner);
	m_queueTableWidget->updateSelectedList();

	QVector<int> &selected_rows = m_queueTableWidget->getSelectedRows();

	int sel_size = selected_rows.size();
	if(sel_size<1){
		qDebug()<<"sel_size is 0";
		return;
	}

	std::vector<QueuViewItem> proce_list;
	for(int i=0;i<sel_size;i++){
		//const QueuViewItem &item = m_queueList[selected_rows[i]];
		//Ref #12 最後の列にID追加／非表示
		//ソートしたケース
		const QueuViewItem *item = this->getViewItem(selected_rows[i]);
		if(item==0){
			continue;
		}

		bool do_flag = false;
		 
		if(item->m_status == CPxQueueEntry::PxQueueStatus_Failed){
			do_flag = true;
		}
		if(do_flag){
			proce_list.push_back(*item);
		}
	}
	 
	////
	CQueueDBProc QueueDB;
	 
	QueueDB.resendResultQueueList(proce_list);


	if(m_pOwner){
		m_pOwner->onRefresh();
	}
}
 