#include "mainwindow.h"
#include "Login.h"

#include "PxDcmDbManage.h"
#include "QMessageBox"
#include "QueueDBProc.h"

int g_user_mode = 0; //#39
int g_first_display_tab_index = 0;
int main(int argc, char *argv[])
{
	 

    QApplication app(argc, argv);

	//#39
	QStringList argv_list = QCoreApplication::arguments();
	if(argv_list.size()>1){
		for(int i=1;i<argv_list.size();i++){
			QString arg_temp = argv_list[i];
			if(arg_temp.contains("-u")){
				g_user_mode = 1;
				g_first_display_tab_index = 0;
			}else if (arg_temp.contains("-d")){
				g_user_mode = 1;
				g_first_display_tab_index = 1;
			}
		}
	}

	if(!CPxDcmDbManage::initDB()){
		QMessageBox msgbox;
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.setText("     initDB failed! \r\n Please check the DBServer and Mediapoint");
		msgbox.exec();
		return 0;

	};
	//
#if 1
	if(g_user_mode ==0) {//#39
		CLogin login;
		login.exec();
		if(!login.isLogin()){
			return 0;
		}
	}
#endif
	CQueueDBProc::init();

	CPxDcmDbManage::initDcmtk();
	CPxDcmDbManage::initDispSpaceManager();

	
    MainWindow window;
	//
#if 0
 
    QStyle *arthurStyle = new ArthurStyle();
	window.setStyle(arthurStyle);
	//
	QList<QWidget *> widgets = qFindChildren<QWidget *>(&window);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
	//
#endif

	std::string style_file_name = CQueueDBProc::getStlyeFileName();
	window.loadStyleSheet(style_file_name);
    window.show();
    return app.exec();
}