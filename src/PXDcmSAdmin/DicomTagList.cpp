#include <QtCore/qdatetime.h>

#include "DicomTagList.h"
 


#include "PXTableWidget.h"
#include "QtHelper.h"

#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
#include <qmessagebox.h>

// #include <QtCore/QTimer>
#include <QtCore/qstring.h>

#include "PxTagFilterManager.h"
  
#include "QString.h"
 #include "assert.h"
 
extern int g_user_mode  ; //#39
 

CDicomTagList::CDicomTagList(QWidget *parent) : QDialog(parent)
,m_viewDispReady(false)
 
{
	m_refWidth = 552;//420;
	m_curWidth = m_refWidth;

	m_QueueListColumn = 2;// 
	ui.setupUi(this); 
 	m_DicomTagListTableWidget = new CPXTableWidget(this);
  	ui.verticalLayout->insertWidget(0,m_DicomTagListTableWidget);
 //	ui.verticalLayout_list->insertWidget(0,m_DicomTagListTableWidget);
	m_DicomTagListTableWidget->setupLineColor(
				QColor(207,233,222),//QColor(182,207,184),
				QColor(240,240,240)
			  );

	 
	if(g_user_mode == 1){//#39
		ui.pBDelete->setDisabled(true);
	}

}
CDicomTagList::~CDicomTagList()
{
	 
}

void CDicomTagList::doInit()
{
	CPxTagFilterManager TagFilterMan;
	
	 
	TagFilterMan.getTagNameListEx(m_TagNameList);
	//
	dispHeader();

	dispQueueList();

	m_DicomTagListTableWidget->setupTableWidget();
}

void CDicomTagList::onRefresh(){
	int x= 0;
	updateList();
	
}
void CDicomTagList::resetColumnWidth(int w)
{
	if(!m_viewDispReady) return;

	float chg_rate = (float)w/m_refWidth;
//	chg_rate = 1.0; //for check refWidth;

	//setup the header Height
	m_DicomTagListTableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(100,TABLE_W_HEADER_HEIGHT));


	m_curWidth = w;

	int col_index = 0;
#if 0
	//No.
	 
	m_DicomTagListTableWidget->setColumnWidth(col_index,60*chg_rate);
	col_index++;
#endif

	//TagID
	m_DicomTagListTableWidget->setColumnWidth(col_index,80*chg_rate);
	col_index++;

	//TagName
	m_DicomTagListTableWidget->setColumnWidth(col_index,320*chg_rate);
	col_index++;
  
	 
}
int CDicomTagList::dispHeader(int col_index)
{
	m_DicomTagListTableWidget->setColumnCount(getListColumn());//m_QueueListColumn);

 
	QTableWidgetItem *item;

	/*
	* 以下のsetColumnWidthは一時的なもの。
	* 実際のサイズは　resetColumnWidth で調整する。
	*/
#if 0
	//No.
	item = new QTableWidgetItem("No.");
	m_DicomTagListTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_DicomTagListTableWidget->setColumnWidth(col_index,20);
	col_index++;
#endif

	//TagID
	item = new QTableWidgetItem("TagID");
	m_DicomTagListTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_DicomTagListTableWidget->setColumnWidth(col_index,20);
	col_index++;

	//TagName
	item = new QTableWidgetItem("TagName");
	m_DicomTagListTableWidget->setHorizontalHeaderItem(col_index,  item);
	m_DicomTagListTableWidget->setColumnWidth(col_index,120);
	col_index++;

	 
	m_viewDispReady = true;
	resetColumnWidth(m_curWidth);
	return col_index;
}


int CDicomTagList::dispQueueItem(int row,PxDBUtil::DB_ID_NameEx &dicomTag_item,int col_index)
{
 char _str_buff[128];
	QString NoStr;
	 
	QTableWidgetItem *item;
#if 0
	item = new QTableWidgetItem("--");
	m_DicomTagListTableWidget->setItem(row ,col_index++, item );
#endif

	//Tag ID
 	unsigned short tag_g = (dicomTag_item.m_TAG&(0xffff0000) ) >>16;
 	unsigned short tag_e = (dicomTag_item.m_TAG&(0x0000ffff) );

	sprintf(_str_buff,"(%04X,%04X)",tag_g,tag_e);
	item = new QTableWidgetItem(_str_buff);
	m_DicomTagListTableWidget->setItem(row ,col_index++, item );

	//Tag Name
	item = new QTableWidgetItem(Str2QString(dicomTag_item.m_name));
	m_DicomTagListTableWidget->setItem(row ,col_index++, item );

   

	return col_index;
}
void CDicomTagList::dispQueueList()
{
	QTableWidgetItem *item;
	int size = m_TagNameList.size();
	m_DicomTagListTableWidget->setRowCount(size);

 
	//
	int col_index = getListColumn();
	for(int i=0;i<size;i++){
		
		m_DicomTagListTableWidget->setRowHeight(i, 20);
		
		col_index = 0;
		col_index = dispQueueItem(i,m_TagNameList[i],col_index);


	}
	assert(getListColumn() == col_index);

	m_DicomTagListTableWidget->updateSortColumn();
}

void CDicomTagList::resizeEvent(QResizeEvent *resize_e)
{
	QDialog::resizeEvent( resize_e);

	const QSize &size = resize_e->size();
	 
	resetColumnWidth(size.width());

	doInit();
	 
}
int CDicomTagList::getQueueID(int sel_row)
{
	int ret_val = -1;
	int queue_id_col = getListColumn()-1; //Ref #12 最後の列にID追加／非表示
	QTableWidgetItem * queue_id_item = m_DicomTagListTableWidget->item( sel_row, queue_id_col ) ;

	if(m_DicomTagListTableWidget){
		ret_val = queue_id_item->text().toInt();
	}
	return ret_val;

}
//
 
#if 0
QueuViewItem * CDicomTagList::getViewItem(int sel_row)
{
	int queue_id = getQueueID(sel_row);
	if(queue_id<0) return 0;


	QueuViewItem *ret_item = 0;

#if 0	
	int list_size = m_queueList.size();
	for(int i=0;i<list_size;i++){
		if(m_queueList[i].m_QueueID == queue_id){
			ret_item = &(m_queueList[i]);
			break;
		}
	} 
#endif
	return ret_item;

}

#endif



 
#include "PxDcmDbManage.h"

unsigned long getNormalTagID(QString TagIDTemp)
{
	unsigned long ret_TAG_ID = 0;

	bool ok;
	int pos = TagIDTemp.indexOf("(");
	if(pos>=0){
		TagIDTemp=TagIDTemp.mid(pos+1);
	}
	//
	pos = TagIDTemp.indexOf(")");
	if(pos>=0){
		TagIDTemp=TagIDTemp.mid(0,pos );
	}
	//
	pos = TagIDTemp.indexOf(",");
	if(pos>=0){
		QString tag_g_str =TagIDTemp.mid(0,pos );
		QString tag_e_str =TagIDTemp.mid(pos+1 );
		unsigned short tag_g = tag_g_str.toUShort(&ok , 16);
		unsigned short tag_e = tag_e_str.toUShort(&ok , 16);

		ret_TAG_ID = (tag_g<<16)+tag_e;
		 
	}else{
		ret_TAG_ID = TagIDTemp.toULong(&ok , 16);
		 
	}
	return ret_TAG_ID ;
}
void CDicomTagList::onAdd()
{
	QString tag_id_str = ui.lETagID->text();

	unsigned long tagID = getNormalTagID(tag_id_str);

	QString tag_name_str = ui.lETagName->text();
	if(tag_name_str.isEmpty()){
		std::string tag_name_temp = CPxDcmDbManage::getTagName(tagID) ;
		tag_name_str = Str2QString(tag_name_temp);
	}
	if(tag_name_str.isEmpty()){
		QMessageBox msgbox;
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.setText("  Unknown Tag ");
		msgbox.exec();
		return;
	}
	CPxTagFilterManager TagFilterMan;
	if(!TagFilterMan.addTagName(tagID,QString2Str(tag_name_str))){
		QMessageBox msgbox;
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.setText(" Insert TAG to DB error ");
		msgbox.exec();
		return;
	}

	doInit();
}
void CDicomTagList::onDelete() 
{
	m_DicomTagListTableWidget->updateSelectedList();

	QVector<int> &selected_rows = m_DicomTagListTableWidget->getSelectedRows();

	int sel_size = selected_rows.size();
	if(sel_size<1) return;

	 
	CPxTagFilterManager TagFilterMan;
	for(int i=0;i<sel_size;i++){
		 QTableWidgetItem * queue_id_item = m_DicomTagListTableWidget->item( selected_rows[i], 0 ) ;

		if(queue_id_item==0){
			continue;
		}
		QString TagID = queue_id_item->text();

		//Tag ID
 		unsigned short tag_g = 0;
 		unsigned short tag_e = 0;

		sscanf(QString2char(TagID),"(%04X,%04X)",&tag_g,&tag_e);

		unsigned long tag_g_e = (tag_g<<16)+tag_e;
		TagFilterMan.deleteTagName(tag_g_e);
	}

	doInit();
	 
}

void CDicomTagList::onGetTagName()
{
	QString tag_id_str = ui.lETagID->text();

 
	unsigned long tagID = getNormalTagID(tag_id_str);
	//
	std::string tag_name = CPxDcmDbManage::getTagName(tagID) ;

	if(tag_name.size()<1 ){
		QMessageBox msgbox;
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.setText("  Unknown Tag ");
		msgbox.exec();
		return;
	}

	ui.lETagName->setText(Str2QString(tag_name));
}
/// 

void CDicomTagList::onClose()
{
	accept();
}