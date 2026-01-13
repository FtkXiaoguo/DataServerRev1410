// AQNetDefaultScheduler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
#include "AppComScheduleTask.h"
 

#include "string"
static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};
static char gPxDcmServerBinHomeBuff[2*MAX_PATH+1]={0,};
void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	int pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);
	strcpy(gPxDcmServerBinHomeBuff,sub_str_temp.c_str());
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
	strcpy(gPxDcmServerHomeBuff,sub_str_temp.c_str());

  }else{
	  gPxDcmServerHomeBuff[0] = 0;
  }
}

#include "AqCore/AqString.h"

void main(int argc, char* argv[])
{
	getHomeFromModulePath();
//	AppComConfiguration::setHomeFolder(gPxDcmServerHomeBuff) ;

	/* #623 2010/09/24 K.KO
	* タスク登録時に、Default値を実行しないフラグの追加
	*  コマンドオプション
	*  -a 0 実行しない。(指定しない場合　実行しない)
	*  -a 1 実行する。
	*/
	bool  default_active = false; //#623 2010/09/24 K.KO
	
	try{
		TaskInfo  oInfo;
		
	
		AqUString u_str_temp;
		u_str_temp.Format(L"%S",gPxDcmServerBinHomeBuff);

		oInfo.m_pwszWorkingDirectory = u_str_temp;//L"C:/Program Files/AQNet/bin";
		oInfo.m_startTime  = 4;        // in hours

		struct tm when;
		time_t now ; 
		time( &now );
		when = *localtime( &now );
 
		int day = when.tm_mday;
		int mon = when.tm_mon+1;
		int year = when.tm_year +1900;
  
		oInfo.m_beginDay  = day;
		oInfo.m_beginMonth = mon;
		oInfo.m_beginYear = year;
		oInfo.m_triggerType = TASK_TIME_TRIGGER_DAILY;

 
		if(argc > 1)  
		{

			int isAPS = 0;

	 
			for(int i=1;i<argc;i++){
				if (argv[i][0] == '-'){
					switch(argv[i][1])
					{
					case 'a':
					case 'A':
						{
						try {
							default_active = (atoi(argv[i+1]) ==1);
						}catch(...)
						{
							default_active = false;
						}
						i++; //jump to next
						}
						break;
					}
				}else{
				 
					try {
						isAPS = atoi(argv[i]);  // return 0 if the input cannot be converted to a value of that type
					}
					catch (...)
					{
						isAPS = 0;
					}

				}
			}
#if 0
			if( isAPS ==1)  // create auto deletion task scheduler, path is different here
			{
				oInfo.m_pwszWorkingDirectory = L"C:/Program Files/AQAPS/bin";

				oInfo.m_lpcwszTaskName = L"AQNetAutoDeletion";
				oInfo.m_pwszApplicationName = L"AQNetAutoDeletion.exe";
				oInfo.m_pwszParameters = L"0";

				AppComScheduleTask autoDeletionTask(oInfo);
				if(!autoDeletionTask.exist())
				{
					autoDeletionTask.create();
				}
			}
#endif
		}
 
 

		/// auto AqDB backup  #623 2010/04/13  K.Ko
		{
			oInfo.m_startTime  = 3;        // in hours
 			oInfo.m_lpcwszTaskName = L"PxDBBackupTask"; //#865 2010/09/27 K.Ko changed the task name

	 		oInfo.m_pwszApplicationName = L"PxDBBackup.bat";
		 
			oInfo.m_pwszParameters = L"";
			oInfo.m_triggerType = TASK_TIME_TRIGGER_DAILY;

		
			AppComScheduleTask autoAqDBBackupTask(oInfo);

	 		autoAqDBBackupTask.setStartTimeMinute(50); // add start_time in minute, please check oInfo.m_startTime
	 		 
		 	if(!autoAqDBBackupTask.exist())
			{
				bool ret_b = autoAqDBBackupTask.create(default_active);
				if(ret_b){
					printf(" create PxDBBackupTask OK \n");
				}else{
					printf(" create PxDBBackupTask NG \n");
				}
			}else{
				printf(" PxDBBackupTask exist ! \n");
			}
	 
		}
		//

	
		///
		{
			oInfo.m_startTime  = 4;        // in hours
 			oInfo.m_lpcwszTaskName = L"PxDBShrinkTask"; //#865 2010/09/27 K.Ko changed the task name
			oInfo.m_pwszApplicationName = L"PxDBShrink.bat";
			oInfo.m_pwszParameters = L"";
			oInfo.m_triggerType = TASK_TIME_TRIGGER_DAILY;

		
			AppComScheduleTask shrinkDatabaseTask(oInfo);
			shrinkDatabaseTask.setStartTimeMinute(0); // add start_time in minute, please check oInfo.m_startTime
			if(!shrinkDatabaseTask.exist())
			{
	//			bool ret_b =shrinkDatabaseTask.create(default_active);
				bool ret_b =shrinkDatabaseTask.create(true); //2010/10/06 K.Ko Shrinkは初期に「実行」にする。既存仕様に合わせる。
				if(ret_b){
					printf(" create PxDBShrinkTask OK \n");
				}else{
					printf(" create PxDBShrinkTask NG \n");
				}
			}else{
				printf("  PxDBShrinkTask exist ! \n");
			}
		 
		}



		/// add PXDbManagerTask   2010/05/17  K.Ko
		{
 			oInfo.m_lpcwszTaskName = L"PxDBManagerTask"; //#865 2010/09/27 K.Ko changed the task name
			oInfo.m_pwszApplicationName = L"PxDBManager.bat";
			oInfo.m_pwszParameters = L"0";
			oInfo.m_triggerType = TASK_EVENT_TRIGGER_AT_SYSTEMSTART;

			AppComScheduleTask  AqDBManagerTask(oInfo);
			if(!AqDBManagerTask.exist())
			{
				bool ret_b = AqDBManagerTask.create(default_active);
				if(ret_b){
					printf(" create PxDBManagerTask OK \n");
				}else{
					printf(" create PxDBManagerTask NG \n");
				}
			}else{
				printf(" PxDBManagerTask exist ! \n");
			}
		}
		//

	}
	catch (...)
	{}
}

