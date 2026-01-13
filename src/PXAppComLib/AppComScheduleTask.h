 /*---------------------------------------------------------------------
 *
 *   
 *-------------------------------------------------------------------*/
#ifndef	__APPCOM_ScheduleTask_h__
#define	__APPCOM_ScheduleTask_h__

#include <windows.h>
#include <winbase.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <wchar.h>

struct TaskInfo
{
	TaskInfo(){Clear();};
	~TaskInfo() {Clear();};

	void Clear() {memset(this, 0, sizeof(*this));};

	int m_startTime ;        // only in hours
    int m_beginDay ;
    int m_beginMonth;
    int m_beginYear ;
    int m_triggerType ;
	int m_minutesDuration;  
	int m_minutesInterval;  	
	// #623 DB backup  2010/09/15 K.Ko
	int m_startDaysOfWeek; //WEEKLY  0:Sunday,1:Monday,...
	int m_WeeksInterval;

	int m_startDays;       //MONTHLYDATE : 0:1st,1:2nd,...
	//
	LPCWSTR m_pwszWorkingDirectory;
	LPCWSTR m_lpcwszTaskName;
	LPCWSTR m_pwszApplicationName;
	LPCWSTR m_pwszParameters;

};
 
class AppComScheduleTask
{
public:
 
	void setStartTimeMinute(int minute) { m_startTimeMinute = minute;}; // 2010/05/24 K.Ko
	AppComScheduleTask ();
	AppComScheduleTask (const TaskInfo& iTaskInfo);
	~AppComScheduleTask() {
		if(m_pIPersistFile) m_pIPersistFile->Release();
		if(m_pITaskTrigger) m_pITaskTrigger->Release();
		if(m_pITask) m_pITask->Release();
		if(m_pITS) m_pITS->Release();
		CoUninitialize();
	 
	};
	void setTaskInfo(const TaskInfo& iTaskInfo);

	bool create(bool enable=true); //#623 2010/09/24 K.KO
	bool run();
	bool exist(); // based on task name
	bool getTriggerInfo(TASK_TRIGGER& oTrigger);
	bool getTriggerInfo(TASK_TRIGGER& oTrigger,TaskInfo &info);
	bool getParameterStr(LPWSTR& oParaStr);
	bool deleteTask();
	bool resetApplicationName(LPCWSTR iAppName);
	bool  resetRepeatInterval(int iValue);

protected:
	bool enableTask(bool enable);//  #623 2010/09/24 K.KO
	bool initialITask(bool iCreateNew);
	bool createNewTask();
	bool setupTrigger();
	bool saveToFile();

static bool getTriggerExt(const TASK_TRIGGER &trigger, TaskInfo& outTaskInfo); //2010/04/14 K.Ko
static bool setTriggerExt(const TaskInfo& iTaskInfo, TASK_TRIGGER &outTrigger); //2010/04/14 K.Ko

	TaskInfo m_taskInfo;
	ITaskScheduler *m_pITS;
	ITask *m_pITask;
	ITaskTrigger *m_pITaskTrigger;
	IPersistFile *m_pIPersistFile;
	//
	int m_startTimeMinute ; // #623 DB backup  2010/09/15 K.Ko
};

#endif //__APPCOM_ScheduleTask_h__