#include "AEList.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>

 #include "AEsGUI.h"
  
#include "QString.h"

class CPXMyAEList: public CPXTableWidget
{
public:
	CPXMyAEList(QWidget *parent = 0): CPXTableWidget(parent)
	{
		m_Owner = 0;
		m_sordColumnFlag = false;

		setupLineColor(
				QColor(207,222,226),//QColor(182,207,184),
				QColor(240,240,240)
			  );
	}
     
	~CPXMyAEList(){
	}
	virtual void setupTableWidget(CAEList *parent)
	{
		m_Owner = parent;
		CPXTableWidget::setupTableWidget();

		setSelectionMode(QAbstractItemView::SingleSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);

	}

	QString getAETitle(int row) 
	{
		QString ret_string;
		QTableWidgetItem * ae_item = QTableWidget::item (  row, 1 ) ;

		if(ae_item){
			ret_string = ae_item->text();
		}

		return ret_string;
	}

	

protected:
	CAEList *m_Owner;
};

 
CAEList::CAEList(QWidget *parent) : QDialog(parent)
{
	m_isOK = false;
	
	ui.setupUi(this);
	 
	m_AEListGUI = new CPXMyAEList;
	m_AEListGUI->setObjectName(QString::fromUtf8("AEList"));
	m_AEListGUI->setupTableWidget(this);

	ui.LayoutAEList->removeWidget(ui.tableWidgetAEList);
	ui.tableWidgetAEList->hide();
	ui.LayoutAEList->addWidget(m_AEListGUI);
	//

	QObject::connect(m_AEListGUI, SIGNAL(clicked(QModelIndex)), this, SLOT(onSelectAE()));
	

	m_AEListGUI->clear();
//	ui.tabWidget->insertTab(0,_AEsMan_,"AEs");
	 
	listAEs();

	//#36 2012/09/28
	ui.btOK->setDisabled(true);
}
CAEList::~CAEList()
{
	delete m_AEListGUI;
}
	
void CAEList::listAEs()
{
	 CPxDcmsAEManageRemote RemoteAE;
	 RemoteAE.queryAEs(m_remoteAEList,true/*storageOnly*/ );
	 //
	 m_AEListGUI->setColumnCount(4);
	 int col_index = 0;
	//AE Name
	QTableWidgetItem *item = new QTableWidgetItem("AE Name");
	m_AEListGUI->setHorizontalHeaderItem(col_index,  item);
	m_AEListGUI->setColumnWidth(col_index,150);
	col_index++;
	//AE Tile
	item = new QTableWidgetItem("AE Title");
	m_AEListGUI->setHorizontalHeaderItem(col_index,  item);
	m_AEListGUI->setColumnWidth(col_index,150);
	col_index++;
	//IP Address
	item = new QTableWidgetItem("IP Address");
	m_AEListGUI->setHorizontalHeaderItem(col_index,  item);
	m_AEListGUI->setColumnWidth(col_index,90);
	col_index++;
	//Port
	item = new QTableWidgetItem("Port");
	m_AEListGUI->setHorizontalHeaderItem(col_index,  item);
	m_AEListGUI->setColumnWidth(col_index,90);
	col_index++;

	//setup the header Height
	m_AEListGUI->horizontalHeaderItem(0)->setSizeHint(QSize(100,TABLE_W_HEADER_HEIGHT));
	//////////
	int size = m_remoteAEList.size();
	m_AEListGUI->setRowCount(size);
	QString str_temp;
	//
	
	
	AEsMap::iterator it = m_remoteAEList.begin();
	int i=0;
	while(it!=m_remoteAEList.end()){
		col_index = 0;

		AEItemData item_data = it->second;

		m_AEListGUI->setRowHeight(i, 20);

		//AE Name
		item = new QTableWidgetItem(item_data.m_AEName.c_str());
		m_AEListGUI->setItem(i ,col_index++, item );
		 
		//AE Tile
		item = new QTableWidgetItem(item_data.m_AETitle.c_str());
		m_AEListGUI->setItem(i ,col_index++, item );

		//IP Address
		item = new QTableWidgetItem(item_data.m_IP.c_str());
		m_AEListGUI->setItem(i ,col_index++, item );

		//Port
		str_temp = QString::number(item_data.m_PortNum);
		item = new QTableWidgetItem(str_temp);
		m_AEListGUI->setItem(i ,col_index++, item );
 

		it++;
		i++;
	}

}
void CAEList::onSelectAE()
{
	int sel=m_AEListGUI->currentRow();

	 QString AE_Title = m_AEListGUI->getAETitle(sel) ;
 
	 //#36 2012/09/28
	 ui.btOK->setDisabled(false);
	// int x=0;

}
void CAEList::onOK()
{
	m_curRow = m_AEListGUI->currentRow();

	accept();
	m_isOK = true;
}
void CAEList::onExit()
{
	m_isOK = false;
	reject();
}
AEItemData CAEList::getSelectedAE()
{
	AEItemData ret_data;
	AEsMap::iterator it = m_remoteAEList.begin();
	int i=0;
	while(it!=m_remoteAEList.end()){
		if(i++ == m_curRow){
			ret_data = it->second;
			break; 
		}
		it++;
	}

	return ret_data;

}