 /*---------------------------------------------------------------------

 *
 *   
 *-------------------------------------------------------------------*/


#include "AppComScheduleTask.h"
#include <time.h>

 

//-----------------------------------------------------------------------
//
AppComScheduleTask::AppComScheduleTask (const  TaskInfo& iTaskInfo)
{
	m_startTimeMinute = 0;// #623 DB backup  2010/09/15 K.Ko
	m_taskInfo = iTaskInfo;
	m_pITS = 0;
	m_pITask = 0;
	m_pITaskTrigger = 0;
	m_pIPersistFile = 0;
}

//-----------------------------------------------------------------------
//
AppComScheduleTask::AppComScheduleTask ()
{
	m_startTimeMinute = 0;// #623 DB backup  2010/09/15 K.Ko
	m_pITS = 0;
	m_pITask = 0;
	m_pITaskTrigger = 0;
	m_pIPersistFile = 0;
}

void AppComScheduleTask::setTaskInfo(const TaskInfo& iTaskInfo)
{
	m_taskInfo = iTaskInfo;
}
//-----------------------------------------------------------------------
//
bool AppComScheduleTask::create(bool enable) //add arg enable #623 2010/09/24 K.KO 
{

	if(!initialITask(true)) return false; 
		
	// Only for old version to remove arguments which are not allowed
	HRESULT hr = m_pITask->SetParameters(L"");
	if (FAILED(hr)) return false;
 
	if(!enable){
		enableTask(false);
	}
	if(!setupTrigger()) return false;

	if(!saveToFile()) return false;
	return true;
}

//-----------------------------------------------------------------------
//
bool AppComScheduleTask::deleteTask()
{
	if(exist())
	{ 
		if(m_pITS) 
		{
			if(m_pITS->Delete(m_taskInfo.m_lpcwszTaskName) == S_OK )
			return true;
		}
		return false;
	}
 
	return true;
}
//-----------------------------------------------------------------------
//
bool AppComScheduleTask::run()
{
	if(!initialITask(false)) return false; 
	if(m_pITask) 
	{
		if(m_pITask->Run() == S_OK ){
				return true;
		}
	} 
 
	return false;
}

bool AppComScheduleTask::getTriggerInfo(TASK_TRIGGER&  oTrigger)
{
	 
	if(!initialITask(false)) return false; 

	HRESULT hr = S_OK;
	WORD plTriggerCount;
	hr = m_pITask->GetTriggerCount (&plTriggerCount);
	if (FAILED(hr)) return false;
	 
	if(plTriggerCount == 0) return false;
 
	ITaskTrigger *pITaskTrigger;
	hr = m_pITask->GetTrigger(0,  &pITaskTrigger);
	if (FAILED(hr)) return false;
 
	hr = pITaskTrigger->GetTrigger( &oTrigger);
	if (FAILED(hr)) return false;
 
	return true;
}

//-----------------------------------------------------------------------
//
bool AppComScheduleTask::getParameterStr(LPWSTR& oParaStr)
{
	if(!initialITask(false)) return false; 
	HRESULT hr = m_pITask->GetParameters(&oParaStr);
	if (FAILED(hr))	return false;

 
	return true;
}

bool AppComScheduleTask::resetApplicationName(LPCWSTR iAppName)
{
	HRESULT hr = S_OK;
	if(!m_pITask) return false;
	hr = m_pITask->SetApplicationName(iAppName);
	if (FAILED(hr)) return false;
		
	LPCWSTR pwszAccountName = L"";
	LPCWSTR pwszPassword = NULL;
	hr = m_pITask->SetAccountInformation(pwszAccountName,
										 pwszPassword);
	if (FAILED(hr)) return false;

	if(!saveToFile()) return false;
	return true;

}

bool AppComScheduleTask::resetRepeatInterval(int iValue)
{
	HRESULT hr = S_OK;
	if(!initialITask(false)) return false; 
	hr = m_pITask->GetTrigger(0,  &m_pITaskTrigger);
	if (FAILED(hr)) return false;

	if(!m_pITaskTrigger) return false;

	TASK_TRIGGER  oTrigger;
	hr = m_pITaskTrigger->GetTrigger( &oTrigger);
	if (FAILED(hr)) return false;

	oTrigger.MinutesInterval = iValue;
	hr = m_pITaskTrigger->SetTrigger (&oTrigger);
	if (FAILED(hr)) return false;
  
	if(!saveToFile()) return false;
	return true;

}
//-----------------------------------------------------------------------
//
bool AppComScheduleTask::createNewTask()
{
	 
	if(!m_pITS) 
		return false;

	HRESULT hr = S_OK;
	LPCWSTR lpcwszTaskName = m_taskInfo.m_lpcwszTaskName;
	hr =m_pITS->NewWorkItem(lpcwszTaskName,			// Name of task
							CLSID_CTask,			// Class identifier 
							IID_ITask,				// Interface identifier
							(IUnknown**)&m_pITask);	// Address of task interface
  
	if (FAILED(hr)) 
		return false;
	
	LPCWSTR pwszApplicationName = m_taskInfo.m_pwszApplicationName;  
	hr = m_pITask->SetApplicationName(pwszApplicationName);
	if (FAILED(hr)) 
		return false;
 
	// specify the current working directory for Test 
	LPCWSTR pwszWorkingDirectory = m_taskInfo.m_pwszWorkingDirectory; 
	hr = m_pITask->SetWorkingDirectory(pwszWorkingDirectory);
	if (FAILED(hr)) 
		return false;
 
	// clear the parameters for Task.
	LPCWSTR pwszParameters = m_taskInfo.m_pwszParameters;
	hr = m_pITask->SetParameters(pwszParameters);
	if (FAILED(hr)) 
		return false;

	// Call ITask::SetAccountInformation to specify the account name
	// and the account password for Task.
	LPCWSTR pwszAccountName = L"";
	LPCWSTR pwszPassword = NULL;
	hr = m_pITask->SetAccountInformation(pwszAccountName,
										 pwszPassword);
	if (FAILED(hr)) 
		return false;

	return true;
 
}


//-----------------------------------------------------------------------
// Based on application name
bool AppComScheduleTask::exist()
{
	return initialITask(false);
}


//-----------------------------------------------------------------------
// step 1
bool AppComScheduleTask::initialITask(bool iCreateNew)
{
	HRESULT hr = S_OK;
 
	// Call CoInitialize to initialize the COM library and then
	// CoCreateInstance to get the Task Scheduler object.
	hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {
		hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **) &m_pITS);
		if (FAILED(hr))  return false;
	}
	else {
		return false;
	}
  
	// Call ITaskScheduler::Activate to get the Task object or create new one.
	LPCWSTR lpcwszTaskName;
	lpcwszTaskName = m_taskInfo.m_lpcwszTaskName;
	hr = m_pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown**) &m_pITask);
 
//	if (FAILED(hr))
 
	if(hr != S_OK)
	{
	    // Try to create a new one
		if(iCreateNew) {
			return createNewTask();
		}
		else{

			return false;
		}
	} 

	return true;
}

//-----------------------------------------------------------------------
// step 2
bool AppComScheduleTask::setupTrigger()
{
	// Call ITask::TriggerCount to retrieve the number of triggers
	// associated with the task.
	if(! m_pITask) return false;
	HRESULT hr = S_OK;
	WORD plTriggerCount = 0;
	hr = m_pITask->GetTriggerCount (&plTriggerCount);
	if (FAILED(hr)) return false;

	WORD piNewTrigger;
	if(plTriggerCount == 0){
		// Call ITask::CreateTrigger to create new trigger.
		hr = m_pITask->CreateTrigger(&piNewTrigger, &m_pITaskTrigger);
		if (FAILED(hr)) return false;
	} 
	else {
		hr = m_pITask->GetTrigger(0,  &m_pITaskTrigger);
		if (FAILED(hr)) return false;
	}
  
	// Define TASK_TRIGGER structure. Note that wBeginDay,
	// wBeginMonth, and wBeginYear must be set to a valid 
	// day, month, and year respectively.
	TASK_TRIGGER pTrigger;
	hr = m_pITaskTrigger->GetTrigger( &pTrigger);
	if (FAILED(hr))
	{
		ZeroMemory(&pTrigger, sizeof (TASK_TRIGGER));
	} 

	bool makeTM = true;
	struct tm when;
	time_t now, result;

	time( &now );
	when = *localtime( &now );

	when.tm_mday = m_taskInfo.m_beginDay;
	when.tm_mon = m_taskInfo.m_beginMonth -1;
	when.tm_year = m_taskInfo.m_beginYear -1900;
	if( (result = mktime( &when )) == (time_t)-1 )
		makeTM = false;

	// Add code to set trigger structure?
	pTrigger.wBeginDay = m_taskInfo.m_beginDay;                   
	pTrigger.wBeginMonth =m_taskInfo.m_beginMonth;                
	pTrigger.wBeginYear =m_taskInfo.m_beginYear;               
	pTrigger.cbTriggerSize = sizeof (TASK_TRIGGER); 
	pTrigger.TriggerType = (TASK_TRIGGER_TYPE )m_taskInfo.m_triggerType;

	// The value of duration may be negative for old version, or m_minutesDuration is not initialized, 
	// MinutesDuration must be large than MinutesInterval
	if(m_taskInfo.m_minutesDuration > 0 && m_taskInfo.m_minutesInterval>0 && m_taskInfo.m_minutesDuration >= m_taskInfo.m_minutesInterval) { 
		pTrigger.MinutesInterval = m_taskInfo.m_minutesInterval;
		pTrigger.MinutesDuration = m_taskInfo.m_minutesDuration;
	}

	if((TASK_TRIGGER_TYPE )m_taskInfo.m_triggerType == TASK_TIME_TRIGGER_DAILY){
		pTrigger.Type.Daily.DaysInterval = 1;
	}
	else if ((TASK_TRIGGER_TYPE )m_taskInfo.m_triggerType == TASK_TIME_TRIGGER_WEEKLY ){
		pTrigger.Type.Weekly.WeeksInterval = 1;
		if (makeTM){
			pTrigger.Type.Weekly.rgfDaysOfTheWeek = 1 << when.tm_wday;
		} 
		else {	
			pTrigger.Type.Weekly.rgfDaysOfTheWeek = TASK_SUNDAY;
		}
	}
	else { // ((TASK_TRIGGER_TYPE )m_taskInfo.m_triggerType == TASK_TIME_TRIGGER_MONTHLY )
			pTrigger.Type.MonthlyDate.rgfDays = 1 << (m_taskInfo.m_beginDay-1);
			pTrigger.Type.MonthlyDate.rgfMonths =  1<< when.tm_mon;
	}
	pTrigger.wStartHour = m_taskInfo.m_startTime;                
//	pTrigger.wStartMinute = 0; // #623 DB backup  2010/09/15 K.Ko
	pTrigger.wStartMinute = m_startTimeMinute ;//// 2010/05/24
	pTrigger.rgFlags = 0; //Need to enable trigger
	
	//
	setTriggerExt(m_taskInfo,pTrigger);// #623 DB backup  2010/09/15 K.Ko
	

	hr = m_pITaskTrigger->SetTrigger (&pTrigger);
	if (FAILED(hr)) return false;
	return true;
 
}

//-----------------------------------------------------------------------
//   Call IPersistFile::Save to save trigger to disk.
//
bool AppComScheduleTask::saveToFile()
{
	if(! m_pITask) return false;
	HRESULT hr = S_OK;
	
	hr = m_pITask->QueryInterface(IID_IPersistFile, (void **)&m_pIPersistFile);
	if (FAILED(hr)) return false;

	hr = m_pIPersistFile->Save(NULL, TRUE);
	if (FAILED(hr)) return false;
	return true;
}

// #623 DB backup  2010/09/15 K.Ko 
bool AppComScheduleTask::getTriggerInfo(TASK_TRIGGER&  oTrigger,TaskInfo &info) //2010/04/14 K.Ko
{
	 
	if(!initialITask(false)) return false; 

	HRESULT hr = S_OK;
	WORD plTriggerCount;
	hr = m_pITask->GetTriggerCount (&plTriggerCount);
	if (FAILED(hr)) return false;
	 
	if(plTriggerCount == 0) return false;
 
	ITaskTrigger *pITaskTrigger;
	hr = m_pITask->GetTrigger(0,  &pITaskTrigger);
	if (FAILED(hr)) return false;
 
	hr = pITaskTrigger->GetTrigger( &oTrigger);
	if (FAILED(hr)) return false;
 
	//
	info.m_startTime	= oTrigger.wStartHour;
 
	info.m_beginDay		= oTrigger.wBeginDay;
	info.m_beginMonth	= oTrigger.wBeginMonth;
	info.m_beginYear	= oTrigger.wBeginYear;

	//
	info.m_minutesInterval	= oTrigger.MinutesInterval;
	info.m_minutesDuration	= oTrigger.MinutesDuration;


	info.m_triggerType	= oTrigger.TriggerType;
	;
	getTriggerExt( oTrigger, info); //2010/04/14 K.Ko

	return true;
}
//2010/04/14 K.Ko
 
bool AppComScheduleTask::getTriggerExt(const TASK_TRIGGER &trigger, TaskInfo& outTaskInfo) //2010/04/14 K.Ko
{
	switch(trigger.TriggerType){
		case TASK_TIME_TRIGGER_WEEKLY:
			{
			outTaskInfo.m_WeeksInterval		= trigger.Type.Weekly.WeeksInterval;
//			outTaskInfo.m_startDaysOfWeek	= trigger.Type.Weekly.rgfDaysOfTheWeek;
				
				unsigned long days_temp  = trigger.Type.Weekly.rgfDaysOfTheWeek ;

				for(int i=1; i< 9 ;i++){
					//1:Sunday, 2:Monday,...
					if( (days_temp & 0x01) != 0){
						outTaskInfo.m_startDaysOfWeek = i;
						break;
					}
					days_temp = days_temp >>1;
				}
			}
		break;
		case TASK_TIME_TRIGGER_MONTHLYDATE:
			{
				unsigned long days_temp  = trigger.Type.MonthlyDate.rgfDays ;

				for(int i=1; i< 33 ;i++){
					if( (days_temp & 0x01) != 0){
						outTaskInfo.m_startDays = i;
						break;
					}
					days_temp = days_temp >>1;
				}
			}
		break;
	}
	return true;
}
bool AppComScheduleTask::setTriggerExt(const TaskInfo& iTaskInfo, TASK_TRIGGER &outTrigger) //2010/04/14 K.Ko
{
	switch(iTaskInfo.m_triggerType){
		case TASK_TIME_TRIGGER_WEEKLY:
			outTrigger.Type.Weekly.WeeksInterval	= iTaskInfo.m_WeeksInterval;
//			outTrigger.Type.Weekly.rgfDaysOfTheWeek	= iTaskInfo.m_startDaysOfWeek;
			outTrigger.Type.Weekly.rgfDaysOfTheWeek	= ((unsigned long)1<<(iTaskInfo.m_startDaysOfWeek-1));
		break;
		case TASK_TIME_TRIGGER_MONTHLYDATE:
//			outTrigger.Type.MonthlyDate.rgfDays		= iTaskInfo.m_startDays;
			outTrigger.Type.MonthlyDate.rgfDays		= ((unsigned long)1<<(iTaskInfo.m_startDays-1));
			outTrigger.Type.MonthlyDate.rgfMonths	= 4095;
		break;
	}
	return true;
}


bool AppComScheduleTask::enableTask(bool enable)//#623 2010/09/24 K.KO
{
	if(! m_pITask) return false;
	HRESULT hr = S_OK;

	DWORD cur_Flags ;
	hr = m_pITask->GetFlags(&cur_Flags);
	//hr = m_pITask->GetTaskFlags(&cur_Flags);
	if(hr != S_OK ) {
		return false;
	}
	if(enable){
		cur_Flags = cur_Flags & (~((DWORD)(TASK_FLAG_DISABLED))); 
	}else{
		cur_Flags = cur_Flags | TASK_FLAG_DISABLED;
	}
	hr = m_pITask->SetFlags(cur_Flags);
	//hr = m_pITask->SetTaskFlags(cur_Flags);
	if(hr != S_OK ) {
		return false;
	}
	return  true;
}