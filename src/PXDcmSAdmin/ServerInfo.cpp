#include "ServerInfo.h"
#include <QProgressDialog>
 
#include <QtGui>
//#include <QPrinter>
 #include <QtCore/QTimer>
#include <QtCore/qstring.h>

#include "QtHelper.h"
 #include "AEsGUI.h"
  
#include "AppVersion.h"
 #include "QueueDBProc.h"

#include "AppComConfiguration.h"//#47

#define DefOKIcion (0)
#define DefErrIcon (1)
#define DefWarnIcon (2)


QVector<MediaPointInfoGUI> m_MediaPointGUIList;

#define ADD_MP_GUI(no){\
	MediaPointInfoGUI newMpGuiTemp;		\
	newMpGuiTemp.m_name			= ui.name##no;	\
	newMpGuiTemp.m_utilization	= ui.utilization##no;	\
	newMpGuiTemp.m_total		= ui.total##no;	\
	newMpGuiTemp.m_free			= ui.free##no;	\
	m_MediaPointGUIList.append(newMpGuiTemp);	\
}

std::map<int,QString> g_service_status_name;
void initSeriveStatusTab();

CServerInfo::CServerInfo(QWidget *parent) : TabPageComInf(parent)
{
	initSeriveStatusTab();
	 
	ui.setupUi(this);
	 
#if 0
	MediaPointInfoGUI newMpGuiTemp;
	newMpGuiTemp.m_name			= ui.name1;
	newMpGuiTemp.m_utilization		= ui.utilization1;
	newMpGuiTemp.m_total			= ui.total1;
	newMpGuiTemp.m_used			= ui.used1;
	m_MediaPointGUIList.append(newMpGuiTemp);
#else
	ADD_MP_GUI(1);
	ADD_MP_GUI(2);
	ADD_MP_GUI(3);
	ADD_MP_GUI(4);
#endif

	m_MediaPointGUIList[0].m_name->setText("tt");
	m_MediaPointGUIList[0].m_utilization->setValue(10);

	//
//	getMediaPointInf();

//	dispInfoMediaPointInf();
	//
	
	QString ver_info = DCMAPP_VERSION_STRING;
	ver_info = QString("           PxDataServer [ Ver") + ver_info + QString(" ] ");
	
	int isJpegGateway = AppComConfiguration::GetJpegGatewayFlag();//#47
	int isDICOMGateway = AppComConfiguration::GetGatewayFlag();//#47
	QString gateway_str ="";
	if(isDICOMGateway) gateway_str += " GateWay";
	if(isJpegGateway) gateway_str += " -- JPEG GateWay";
	ver_info += "[";
	ver_info += gateway_str;
	ver_info += "]";

	ver_info +=  QString("       Copyright 2012 PreXion Co., Ltd.  All Rights Reserved."); 
	ui.verInfo->setText(ver_info);
	//
//	updateServiceInfo();
//	dispServiceInfo();
}
 
void CServerInfo::getMediaPointInf(){
	m_mediaPointList.clear();

	bool ret_b = CPxDcmDbManage::getMediaPointInfo(m_mediaPointList);
}
void CServerInfo::dispInfoMediaPointInf()
{
	QString str_temp;
	int disp_no = 0;
	int mp_info_size = m_mediaPointList.size();
	for(disp_no=0;disp_no<mp_info_size;disp_no++){
		m_MediaPointGUIList[disp_no].m_name->setText(m_mediaPointList[disp_no].m_name.c_str());
		//
		str_temp = QString("%1GB").arg(m_mediaPointList[disp_no].m_total/1024.0,
											0, 'f', 2);//小数点２
		m_MediaPointGUIList[disp_no].m_total->setText(str_temp);
		//
		str_temp = QString("%1GB").arg(m_mediaPointList[disp_no].m_free/1024.0,
											0, 'f', 2);
		m_MediaPointGUIList[disp_no].m_free->setText(str_temp);

		float uti_rate = 100.0-100.0*(float)m_mediaPointList[disp_no].m_free/m_mediaPointList[disp_no].m_total;
		m_MediaPointGUIList[disp_no].m_utilization->setValue(uti_rate);
	}
	//
	int mp_gui_size = m_MediaPointGUIList.size();
	for( /*continue*/;disp_no<mp_gui_size;disp_no++){
		m_MediaPointGUIList[disp_no].m_name->hide();
		m_MediaPointGUIList[disp_no].m_total->hide();;
		m_MediaPointGUIList[disp_no].m_free->hide();;
		m_MediaPointGUIList[disp_no].m_utilization->hide();;
	}
}
#include "PXLicenseManagerIf.h"
 #include "AppComUtil.h"
//	Convert LicenseStatus to string

void CServerInfo::dispHaspInfo()
{
	int isJpegGateway = AppComConfiguration::GetJpegGatewayFlag();//#47
	if (0 != AppComConfiguration::GetLocalBackupFlag()){
		isJpegGateway = 1;//#94
	}

	int hasp_icon_status = DefErrIcon;

	int daysToExpire = 0;
 
	int gHASPFeatureID = ePREXION_PXDICOMSERVER3;

	int dicomServerHaspStatus = LicenseManager::kLMUndefined;
	std::string hasp_info_str = "Unknown Status";
	try {
#ifdef _DEBUG
#pragma message("excute without HASP at DEBUG mode")
		dicomServerHaspStatus = LicenseManager::kLMLicenseValid;
#else
		dicomServerHaspStatus = PXLicenseManager::CheckLicense(gHASPFeatureID, daysToExpire);
#endif
		hasp_info_str = AppComUtil::ConvertErrorCodeToString(dicomServerHaspStatus);
		
 
	}catch(...){
		;;
	}
	char _str_buff[256];
	if(dicomServerHaspStatus==LicenseManager::kLMLicenseWillExpire){
		sprintf(_str_buff,"  HASP License : %s daysToExpire [%d] ",hasp_info_str.c_str(),daysToExpire);
	}else{
		sprintf(_str_buff,"  HASP License : %s  ",hasp_info_str.c_str());
	}
	 
	ui.HaspInfo->setText( _str_buff);

	if(daysToExpire >0){
		hasp_icon_status = DefOKIcion;
	}else{
		///////
		bool checkHifDevice = true;
		if(isJpegGateway !=0){ //#47
			int GPUHaspStatus = LicenseManager::kLMUndefined;
			try {
				GPUHaspStatus = PXLicenseManager::CheckLicense(ePREXION_FINECUBE_IMAGESERVER, daysToExpire);
				hasp_info_str = AppComUtil::ConvertErrorCodeToString(GPUHaspStatus);
				 
			}catch(...){
				;;
			}
			if(GPUHaspStatus==LicenseManager::kLMLicenseWillExpire){
				sprintf(_str_buff,"  HASP License(From ImageServer) : %s daysToExpire [%d] ",hasp_info_str.c_str(),daysToExpire);
			}else{
				sprintf(_str_buff,"  HASP License(From ImageServer) : %s  ",hasp_info_str.c_str());
			}
			ui.HaspInfo->setText( _str_buff);

			if(daysToExpire >0){
				hasp_icon_status = DefOKIcion;
				checkHifDevice = false;
			}

		}
		///////
		//#33 HIFチェックの追加 2012/09/06 K.Ko
		char chkHifMsg[256];
		chkHifMsg[0] = 0;
		 
		if(checkHifDevice){//#47
			if(AppComUtil::ChkHifDrv(chkHifMsg,256) == ChkHifDrv_OK ){ //#33 HIFチェックの追加 2012/09/06 K.Ko
				ui.HaspInfo->setText(chkHifMsg);
				hasp_icon_status = DefOKIcion;
			}else{
				ui.HaspInfo->setText(chkHifMsg);
			}
		}
		
	};
	setupStatusIcon(hasp_icon_status,ui.HaspIcon);
}


//////////////////
void CServerInfo::pageEntry()
{
	dispHaspInfo();
	//
	getMediaPointInf();
	dispInfoMediaPointInf();
	//
	updateServiceInfo();
	dispServiceInfo();
}
void CServerInfo::pageExit()
{
}
////////////////




#include "RTVDaemon.h"
/////////////////////
const char *serviceNameJobProc = "PXDcmJobProc";
	 
const char *serviceNameDicomServer = "PXDcmSServer";


void CServerInfo::setupStatusIcon(int JobProIcon,QLabel *labeIcon)
{
static QPixmap OKIcon(":/rcFiles/rcFiles/ServerOk.PNG");
static QPixmap ErrIcon(":/rcFiles/rcFiles/ServerError.PNG");
static QPixmap WarnIcon(":/rcFiles/rcFiles/ServerWarn.PNG");

	switch(JobProIcon){
	case DefOKIcion:
		labeIcon->setPixmap(OKIcon);
		break;
	case DefErrIcon:
		labeIcon->setPixmap(ErrIcon);
		break;
	case DefWarnIcon:
		labeIcon->setPixmap(WarnIcon);
		break;
	}
}
void CServerInfo::dispServiceInfo()
{


	QString SeriveStatusPre = "  Service:   ";
	//JobProc
	{
		int JobProIcon = DefWarnIcon;
		QString JobProcStatusStr = "Invalid";
		if(m_JobProcInstalledFlag){
			JobProcStatusStr = g_service_status_name[m_JobProcStatus];//"installed";
		}
		ui.LbJobProc->setText(SeriveStatusPre + JobProcStatusStr);
		
		if(m_JobProcInstalledFlag){
			if(m_JobProcStatus == SERVICE_RUNNING){
				JobProIcon = DefOKIcion;
				ui.pBStartJobProc->setEnabled(false);
				ui.pBStopJobProc->setEnabled(true);
			}else if(m_JobProcStatus == SERVICE_STOPPED){
				ui.pBStartJobProc->setEnabled(true);
				ui.pBStopJobProc->setEnabled(false);
			}else{
				ui.pBStartJobProc->setEnabled(false);
				ui.pBStopJobProc->setEnabled(false);
			}
		}else{
			JobProIcon = DefErrIcon;
			ui.pBStartJobProc->setEnabled(false);
			ui.pBStopJobProc->setEnabled(false);
		}
		setupStatusIcon(JobProIcon,ui.LbJobProcIcon);
	}

	//DcmServer
	{
		int DcmServIcon = DefWarnIcon;
		QString DcmServStatusStr = "Invalid";
		if(m_DcmServInstalledFlag){
			DcmServStatusStr = g_service_status_name[m_DcmServStatus];//"installed";
		}
		ui.LbDcmServ->setText(SeriveStatusPre + DcmServStatusStr);
	 

		if(m_DcmServInstalledFlag){
			if(m_DcmServStatus == SERVICE_RUNNING){
				DcmServIcon = DefOKIcion;
				ui.pBStartDcmServ->setEnabled(false);
				ui.pBStopDcmServ->setEnabled(true);
			}else if(m_DcmServStatus == SERVICE_STOPPED){
				ui.pBStartDcmServ->setEnabled(true);
				ui.pBStopDcmServ->setEnabled(false);
			}else{
				ui.pBStartDcmServ->setEnabled(false);
				ui.pBStopDcmServ->setEnabled(false);
			}
		}else{
			DcmServIcon = DefErrIcon;
			ui.pBStartDcmServ->setEnabled(false);
			ui.pBStopDcmServ->setEnabled(false);
		}
		setupStatusIcon(DcmServIcon,ui.LbDcmServIcon);
	}


}
void initSeriveStatusTab()
{
	g_service_status_name[SERVICE_STOPPED]			= "SERVICE_STOPPED";
	g_service_status_name[SERVICE_STOP_PENDING]		= "SERVICE_STOP_PENDING";
	g_service_status_name[SERVICE_START_PENDING]	= "SERVICE_START_PENDING";
	g_service_status_name[SERVICE_RUNNING]			= "SERVICE_RUNNING";
	g_service_status_name[SERVICE_CONTINUE_PENDING] = "SERVICE_CONTINUE_PENDING";
	g_service_status_name[SERVICE_PAUSE_PENDING]	= "SERVICE_PAUSE_PENDING";
	g_service_status_name[SERVICE_PAUSED]			= "SERVICE_PAUSED";
}
void CServerInfo::updateServiceInfo()
{
	int oStartType;
	BOOL status ;
	//
	 
	status = RTVDaemon::GetServiceStartType(serviceNameJobProc,  oStartType);  
	if((status!=0) && (oStartType != -1)){
		m_JobProcInstalledFlag = true;
		status = RTVDaemon::GetServiceState(serviceNameJobProc, m_JobProcStatus);
		 
	}else{
		m_JobProcInstalledFlag = false;
	}
	//
	 
	status = RTVDaemon::GetServiceStartType(serviceNameDicomServer,  oStartType);  
	if((status!=0) && (oStartType != -1)){
		m_DcmServInstalledFlag = true;
		status = RTVDaemon::GetServiceState(serviceNameDicomServer, m_DcmServStatus);
		 
	}else{
		m_DcmServInstalledFlag = false;
	}

	return;
}

void CServerInfo::onStopJobProc()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	RTVDaemon::StopService(serviceNameJobProc);  
	//
	updateServiceInfo();
	dispServiceInfo();

	QApplication::restoreOverrideCursor();
}
void CServerInfo::onStartJobProc()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	RTVDaemon::MyStartService(serviceNameJobProc);  
	//
	updateServiceInfo();
	dispServiceInfo();

	QApplication::restoreOverrideCursor();
}
void CServerInfo::onStopDcmServ()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	RTVDaemon::StopService(serviceNameDicomServer);  
	//
	updateServiceInfo();
	dispServiceInfo();

	QApplication::restoreOverrideCursor();
}
void CServerInfo::onStartDcmServ()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	RTVDaemon::MyStartService(serviceNameDicomServer);
	//
	updateServiceInfo();
	dispServiceInfo();

	QApplication::restoreOverrideCursor();
}

#define OPEN_LOG_CMD "notepad.exe "
void CServerInfo::onOpenJobProcLogFile()
{
	QProcess *pProcess = new QProcess;
	std::string log_folder = CQueueDBProc::getLogFileFolder();
	QString log_file = Str2QString(log_folder)+"PXDcmJobProc.log";

	QStringList cmd_arg_list;
	cmd_arg_list.append(log_file);

	pProcess->start(OPEN_LOG_CMD,cmd_arg_list);

}
void CServerInfo::onOpenDcmServLogFile()
{
	QProcess *pProcess = new QProcess;
	std::string log_folder = CQueueDBProc::getLogFileFolder();
	QString log_file = Str2QString(log_folder)+"PXDcmSServer.log";

	QStringList cmd_arg_list;
	cmd_arg_list.append(log_file);

	pProcess->start(OPEN_LOG_CMD,cmd_arg_list);
}