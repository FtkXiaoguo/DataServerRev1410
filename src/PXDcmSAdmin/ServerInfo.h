#ifndef SERVER_INFO_GUI__H
#define SERVER_INFO_GUI__H

#include "ui_AEMan.h"
#include "ui_ServerInfo.h"
 
 #include "PxDcmDbManage.h"
#include "TabPageCom.h" 
//
class MediaPointInfoGUI
{
public:
	QLabel *m_name;
	QProgressBar *m_utilization;
	QLabel *m_total;
	QLabel *m_free;
};
 
class CServerInfo : public TabPageComInf {
	Q_OBJECT
	public:
		CServerInfo( QWidget *parent = 0);
//////////////////
	virtual void pageEntry();
	virtual void pageExit();
/////////////////			 
	private:
		Ui::ServInfo ui;
	public slots:
		void onStopJobProc();
		void onStartJobProc();
		void onStopDcmServ();
		void onStartDcmServ();
		void onOpenJobProcLogFile();
		void onOpenDcmServLogFile();

		 
	 
protected:
	void setupStatusIcon(int JobProIcon,QLabel *labeIcon);
	void dispHaspInfo();
	void getMediaPointInf();
	void dispInfoMediaPointInf();
	void updateServiceInfo();
	void dispServiceInfo();

	 std::vector<MediaPointInfo> m_mediaPointList;
	 //
	 int m_JobProcInstalledFlag;
	 unsigned long  m_JobProcStatus;
	 int m_DcmServInstalledFlag;;
	 unsigned long m_DcmServStatus;
private:

	 
	 
};


#endif //SERVER_INFO_GUI__H


