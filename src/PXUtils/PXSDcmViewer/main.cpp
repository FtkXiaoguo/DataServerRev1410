#include "mainwindow.h"
 
QString g_dicom_file_path;
QString g_dicom_file_list;

QString g_dicom_series_folder;
QString g_dicom_file;

int w_pos_X = -1;
int w_pos_Y = -1;
int w_Width = 0; //-1: full screen
int w_Height = 0;

int g_enable_menu = 1;
int main(int argc, char *argv[])
{
	 

    QApplication app(argc, argv);

	QStringList argv_list = QCoreApplication::arguments();


	if(argv_list.size()>1){
	

#if 1
		int index = 1;
		while(index<argv_list.size()){
			QString arg_temp = argv_list[index];

			if(arg_temp.contains("-path")){
				if(++index >= argv_list.size()) break;
				 g_dicom_file_path =argv_list[index];
				 ++index;
				 continue;
			}
			//
			if(arg_temp.contains("-list")){
				if(++index >= argv_list.size()) break;
				 g_dicom_file_list =argv_list[index];
				 ++index;
				 continue;
			}
			//
			if(arg_temp.contains("-file")){
				if(++index >= argv_list.size()) break;
				 g_dicom_file =argv_list[index];
				 ++index;
				 continue;
			}
			//
			if(arg_temp.contains("-folder")){
				if(++index >= argv_list.size()) break;
				 g_dicom_series_folder =argv_list[index];
				 ++index;
				 continue;
			}
			//
			if(arg_temp.contains("-OrgX")){
				if(++index >= argv_list.size()) break;
				QString str_temp = argv_list[index];
				w_pos_X = str_temp.toInt();
				 ++index;
				 continue;
			}
			if(arg_temp.contains("-OrgY")){
				if(++index >= argv_list.size()) break;
				QString str_temp = argv_list[index];
				w_pos_Y = str_temp.toInt();
				 ++index;
				 continue;
			}
			//
			if(arg_temp.contains("-Width")){
				if(++index >= argv_list.size()) break;
				QString str_temp = argv_list[index];
				w_Width = str_temp.toInt();
				 ++index;
				 continue;
			}
			if(arg_temp.contains("-Height")){
				if(++index >= argv_list.size()) break;
				QString str_temp = argv_list[index];
				w_Height = str_temp.toInt();
				 ++index;
				 continue;
			}

			if(arg_temp.contains("-m")){
				 g_enable_menu = 0;
				 ++index;
				 continue;
			}
			
			++index;
		}
#else
		bool getPath_flag = false;
		bool getFileList_flag = false;
		bool getDicomFile_flag = false;
		bool getDicomSeriesFolder_flag = false;
	 	for(int i=1;i<argv_list.size();i++){
	
			QString arg_temp = argv_list[i];
			if(getPath_flag){
				g_dicom_file_path = argv_list[i];
				getPath_flag = false;
				continue;
			}
			if(getFileList_flag){
				g_dicom_file_list = argv_list[i];
				getFileList_flag = false;
				continue;
			}
			//
			if(getDicomSeriesFolder_flag){
				g_dicom_series_folder = argv_list[i];
				getDicomSeriesFolder_flag = false;
				continue;
			}
			if(getDicomFile_flag){
				g_dicom_file = argv_list[i];
				getDicomFile_flag = false;
				continue;
			}

			if(arg_temp.contains("-path")){
				 getPath_flag = true;
				 continue;
			}
			if(arg_temp.contains("-list")){
				 getFileList_flag = true;
				 continue;
			}
			//
			if(arg_temp.contains("-file")){
				 getDicomFile_flag = true;
				 continue;
			}
			if(arg_temp.contains("-folder")){
				 getDicomSeriesFolder_flag = true;
				 continue;
			}
		}
#endif
	}
 
	app.setStyle("Cleanlooks");
	
    MainWindow window;

	window.initDcm();
    
	window.hide();

	if( (w_Width<0) || (w_Height<0)){
//		window.setWindowState(Qt::WindowFullScreen);
		window.setWindowState(Qt::WindowMaximized);
	}else{
		if((w_pos_X>=0)&&(w_pos_Y>=0)){
			window.move(w_pos_X,w_pos_Y);
		}
		
		if((w_Width>0) && (w_Height>0)){
 			window.resize(w_Width,w_Height);
		}
	}

	window.show();
    return app.exec();
}