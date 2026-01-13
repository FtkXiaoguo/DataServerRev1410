// DbManSupervisor.cpp: CDbManSupervisor クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbManSupervisor.h"

#include "PXMyScheduleTask.h"

#include "rtvloadoption.h"

#include "PxNetDB.h"

#include "AqCore/TRLogger.h"
#include "AqCore/TRPlatform.h"

#include "DbScheduleTask.h"

#include "AppComConfiguration.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDbManSupervisor::CDbManSupervisor()
{

	//m_configName = "C:\\AQNetConfig\\PXDbManagerTask.cfg";
	std::string configRootPath = "" ; 
  
	//#7 2012/03/08 K.Ko
	AppComConfiguration::GetConfigRootPath(configRootPath) ;
 
	m_configName  = configRootPath+"PXDbManagerTask.cfg";

	std::string LogRootPath ;
	AppComConfiguration::GetLogFilesLocation(LogRootPath);
	TRPlatform::MakeDirIfNeedTo(LogRootPath.c_str());

	m_logFileName = LogRootPath+"PXDbManagerTask.log";

	///////
	 m_ScheduleTask = new PXMyScheduleTask;
	 //
	 //default task
 	 addTaskName("PxDBBackupTask"); //#865 2010/09/27 K.Ko changed the task name
 	 addTaskName("PxDBShrinkTask");
	 //
	 for(int i=0;i<MONITOR_TASK_MAX;i++){
	 
		m_RunTimeout[i] = 0;
	}
}

CDbManSupervisor::~CDbManSupervisor()
{
	delete m_ScheduleTask;
}
int CDbManSupervisor::doMain(int argc, char** argv)
{
	ParseCommandLine(argc, argv);

	if(!loadConfiguration(m_configName.c_str())){
		getLogger()->LogMessage("ERROR: *** loadConfiguration %s error *** \n",m_configName.c_str() );
 		getLogger()->FlushLog();
		return -1;
	}
 
	getLogger()->LogMessage(" \n");
	getLogger()->LogMessage(" === DB Manager Task === \n");
 	getLogger()->FlushLog();

		
	bool do_flag = doMonitorTask();

	return do_flag;
 
}

static char _LogFileName[128]={0,};	 


static char _MonitorTaskName[MONITOR_TASK_MAX][128] ;
static char _MonitorTask_entry[MONITOR_TASK_MAX][128] ;
	 	
static char _Timeout_entry[MONITOR_TASK_MAX][128] ;
 

static char _temp_str[128];
bool CDbManSupervisor::loadConfiguration(const char *fileName)
{
	int i;
	iRTVOptions cstore_opt;

	for(i=0;i<MONITOR_TASK_MAX;i++){
		_MonitorTaskName[i][0] = 0;
		sprintf(_MonitorTask_entry[i],"TaskName%d",i+1);
		//
		sprintf(_Timeout_entry[i],"Timeout%d",i+1);
		
		m_RunTimeout[i] = 0;
	}
	cstore_opt.Add("LogFileName",_LogFileName,sizeof(_LogFileName));
	 
	
	

	for(i=0;i<MONITOR_TASK_MAX;i++){
		cstore_opt.Add(_MonitorTask_entry[i],_MonitorTaskName[i],sizeof(_MonitorTaskName[i]));
		//
		cstore_opt.Add(_Timeout_entry[i],&(m_RunTimeout[i]),sizeof(m_RunTimeout[i]));
	}

	if(!cstore_opt.Load(fileName)){
		return false;
	}

	if(strlen(_LogFileName) == 0){
		setLogFileName(m_logFileName);
	}else{
		setLogFileName(_LogFileName);
	}

	for(i=0;i<MONITOR_TASK_MAX;i++){
		if(strlen(_MonitorTaskName[i]) > 0){
			 addTaskName(_MonitorTaskName[i]);
		}
	}

	return true;
}

void CDbManSupervisor::addTaskName(std::string taskName)
{
	int size = m_MonitorTaskList.size();
	for(int i=0;i<size;i++){
		if(m_MonitorTaskList[i] == taskName){
			return;
		}
	}

	m_MonitorTaskList.push_back(taskName);
}

bool CDbManSupervisor::doMonitorTask()
{
		
	bool ret_b = true;
	int size = m_MonitorTaskList.size();

	CDbScheduleTask db_sheduleTaskTemp;
	for(int i=0;i<size;i++){
		bool timeoutFlag = false;
		if(!db_sheduleTaskTemp.doMonitor(m_MonitorTaskList[i] ,timeoutFlag,m_RunTimeout[i])) {
			getLogger()->LogMessage("ERROR:[C0005] --- doMonitor %s failed \n" ,m_MonitorTaskList[i].c_str() );
 	 		getLogger()->FlushLog();
			continue;
		}
		if(timeoutFlag){
			getLogger()->LogMessage("ERROR:[C0006] --- doMonitor %s ,timeout \n" ,m_MonitorTaskList[i].c_str());
 	 		getLogger()->FlushLog();
			continue;
		}
		std::string mess = db_sheduleTaskTemp.getRunMessage();
		if(mess.size()>0){
			getLogger()->LogMessage("Warning --- doMonitor %s is ignored \n" ,m_MonitorTaskList[i].c_str());
 	 		getLogger()->FlushLog();
		}
		
 
	//		getLogger()->LogMessage("ERROR:[C0006] --- backup  database %s failed \n" ,m_MonitorTaskList[i] );
 	//		getLogger()->FlushLog();
			
	//		ret_b = ret_b&&false;

	 
	}

	getLogger()->LogMessage(" --- DB Manger Task finished \n"  );
 	getLogger()->FlushLog();


	return true;
}