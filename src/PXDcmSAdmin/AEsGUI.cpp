#include "QtHelper.h"

#include "AEsGUI.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>
  #include <QHostAddress>

//CPxDcmsAEManage AEMan;
CAEItem::CAEItem(bool isLocalAE,QWidget *parent) : QWidget(parent)
{
	m_AEManage = 0;
	m_isLocalAE = isLocalAE;
	ui.setupUi(this);
	 
	ui.sBPortNum->setMinimum(0);//#48
	 
}
 
void CAEItem::disableDetail(bool enable )
{
	if(enable){
		ui.chkPushData->show();
		ui.chkQRData->hide();//show();
		ui.chkQRFromMe->show();
	}else{
		ui.chkPushData->hide();
		ui.chkQRData->hide();
		ui.chkQRFromMe->hide();
		//
 
		ui.IpAddrs1->setEnabled(false);
		ui.IpAddrs2->setEnabled(false);
		ui.IpAddrs3->setEnabled(false);
		ui.IpAddrs4->setEnabled(false);
 
		ui.lEHostName->setEnabled(false);
		ui.sBPortNum->setEnabled(false);
	}
}

void CAEItem::addAEItem(const AEItemData &AEItem,bool modify)
{
//	this->ui.comboBox->addItem(AEItem.m_AEName.c_str());

	if(!m_AEManage) return;
	m_AEManage->addAE(AEItem,modify);
	queryDB();
}
void CAEItem::delAEItem(const std::string AEName)
{
	if(!m_AEManage) return;
	m_AEManage->deleteAE(AEName);
	queryDB();
}
//
void CAEItem::readAEItem(AEItemData &AEItem)
{
	AEItem.m_AEName		= QString2Str(ui.lEAEName->text()) ;
	AEItem.m_AETitle	= QString2Str(ui.lEAETile->text()) ;
	AEItem.m_HostName	= QString2Str(ui.lEHostName->text()) ;
	QString	str_temp	=	QString::number(ui.IpAddrs1->value())+"."+
							QString::number(ui.IpAddrs2->value())+"."+
							QString::number(ui.IpAddrs3->value())+"."+
							QString::number(ui.IpAddrs4->value());
	AEItem.m_IP			= QString2Str(str_temp) ;
	AEItem.m_PortNum	= ui.sBPortNum->value();
	//
	AEItem.m_CanPushData	= ui.chkPushData->isChecked() ? 1 : 0 ;
	AEItem.m_CanQR			= ui.chkQRData->isChecked() ? 1 : 0 ;
	AEItem.m_CanQRFromMe	= ui.chkQRFromMe->isChecked() ? 1 : 0 ;
	//
	
}
void CAEItem::onAddAEItem()
{
	AEItemData AEItem;
	
	readAEItem(AEItem);

	addAEItem(AEItem,false/*modify*/);

	setCurrentAE(AEItem.m_AEName.c_str());
	 
}
void CAEItem::onUpdateAEItem()
{
	AEItemData AEItem;
	
	readAEItem(AEItem);
	//
	AEItem.m_AEID	= m_AEList[AEItem.m_AEName].m_AEID;

	addAEItem(AEItem,true/*modify*/);

	setCurrentAE(AEItem.m_AEName.c_str());
}
void CAEItem::onDeleteAEItem()
{
	std::string ae_name = QString2Str(ui.comboBox->currentText()) ;
	delAEItem(ae_name);

	setCurrentAE();
}
void CAEItem::onSelectAE(QString ae)
{
	std::string ae_str = QString2Str(ae);
	displayAEItem( ae_str);
}
bool CAEItem::queryDB()
{
	if(!m_AEManage) return false;
	

	m_AEList.clear();
	m_AEManage->queryAEs(m_AEList);

	ui.comboBox->clear();
	int size = m_AEList.size();


	AEsMap::iterator it = m_AEList.begin();
	while(it!=m_AEList.end()){
		std::string AEName = it->first;
		if(AEName.size()>0){
			ui.comboBox->addItem(AEName.c_str());
		}
		it++;
	}
	 
	return true;
}
void CAEItem::setCurrentAE(const QString &AEName)
{
	if(AEName.isEmpty()){
		ui.comboBox->setCurrentIndex(0);
		return;
	}
	int cur_index = -1;
	int index_temp = 0;
	AEsMap::iterator it = m_AEList.begin();
	while(it!=m_AEList.end()){
		if(AEName == (it->first).c_str()){
			cur_index = index_temp;
			break;
		};
		it++;
		index_temp++;
	}
	ui.comboBox->setCurrentIndex(cur_index);
}
void CAEItem::displayAEItem(const std::string AEName)
{
	if(AEName.size()<1) return ;

	const AEItemData at_item = m_AEList[AEName];
	//
	ui.lEAEName->setText(at_item.m_AEName.c_str());
	ui.lEAETile->setText(at_item.m_AETitle.c_str());
	ui.lEHostName->setText(at_item.m_HostName.c_str());

	{
	QHostAddress host_addr(at_item.m_IP.c_str());
	quint32 ipv_addr = host_addr.toIPv4Address();
	ui.IpAddrs1->setValue((ipv_addr&(0xff000000))>>24);
	ui.IpAddrs2->setValue((ipv_addr&(0x00ff0000))>>16);
	ui.IpAddrs3->setValue((ipv_addr&(0x0000ff00))>>8);
	ui.IpAddrs4->setValue((ipv_addr&(0x000000ff)));
	}
	ui.sBPortNum->setValue(at_item.m_PortNum);
	//
	ui.chkPushData->setChecked(at_item.m_CanPushData !=0);
	ui.chkQRData->setChecked(at_item.m_CanQR !=0);
	ui.chkQRFromMe->setChecked(at_item.m_CanQRFromMe !=0);
	 
	 
}
/////////

CAEsMan::CAEsMan(QWidget *parent) : TabPageComInf(parent)
{
	m_LocalAEs=m_RemoteAES=0;
	m_LocalAEManage = m_RemoteManage = 0;	
	ui.setupUi(this);
	 
	 
}
 
void CAEsMan::init()
{
	if(m_LocalAEs) delete m_LocalAEs;
	m_LocalAEs	= new CAEItem(true,ui.gBLocalAEs);
	if(m_RemoteAES) delete m_RemoteAES;
	m_RemoteAES = new CAEItem(false,ui.gBRemoteAEs);

	if(m_LocalAEManage) delete m_LocalAEManage;
	m_LocalAEManage = new CPxDcmsAEManageLocal;
	 
	//
	if(m_RemoteManage) delete m_RemoteManage;
	m_RemoteManage	= new CPxDcmsAEManageRemote ;
	 

	//
	m_LocalAEs->setAEManage(m_LocalAEManage);
	m_RemoteAES->setAEManage(m_RemoteManage);

	////////////

	ui.lyLocalAEs->addWidget(m_LocalAEs);
	ui.lyRemoteAEs->addWidget(m_RemoteAES);

	m_LocalAEs->disableDetail();
//	m_LocalAEs->queryDB();
	
	m_RemoteAES->disableDetail(true);
//	m_RemoteAES->queryDB();
//	ui.gBLocalAEs->
}

 //////////////////
void CAEsMan::pageEntry()
{
	int x =0;
	//
	m_LocalAEs->queryDB();
	
	 
	m_RemoteAES->queryDB();
}
void CAEsMan::pageExit()
{
	int x =0;
}
/////////////////