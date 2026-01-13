// PXMyScheduleTask.cpp: PXMyScheduleTask クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PXMyScheduleTask.h"

#include <time.h>

//#1009 2011/10/04 K.Ko add log
#include "DbManagerTaskBase.h"
#include "AqCore/TRLogger.h"
#include "AqCore/TRPlatform.h"
///
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

PXMyScheduleTask::PXMyScheduleTask()
{

	m_ReservedMinutesToNextRun = 10;
}
PXMyScheduleTask::PXMyScheduleTask (const TaskInfo& iTaskInfo):AppComScheduleTask(iTaskInfo)
{
	m_ReservedMinutesToNextRun = 10;
}
PXMyScheduleTask::~PXMyScheduleTask()
{

}
bool PXMyScheduleTask::init()
{
	if(!initialITask(false)) return false; 
	return true;
}
 bool PXMyScheduleTask::isValidTask()
 {
	 bool ret_b = false;
	 HRESULT hr = S_OK;

	///
	HRESULT status;
	hr = m_pITask->GetStatus( &status);

	if (FAILED(hr)) {
		//#1009 2011/10/04 K.Ko add log
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0010] --- isValidTask - GetStatus error \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
		return false;
	}
#if 0
	//Dbg 2011/10/04 K.Ko
	DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - GetStatus 0x%x \n" ,status  );
 	DbManagerTaskBase::getLogger()->FlushLog();
#endif

	switch(status){
	case SCHED_S_TASK_READY :
	//	DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - SCHED_S_TASK_READY \n"  );
	//	printf("SCHED_S_TASK_READY\n");
		ret_b = true;
		break;
	case SCHED_S_TASK_RUNNING:
	//	DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - SCHED_S_TASK_RUNNING \n"  );
//		printf("SCHED_S_TASK_RUNNING\n");

		break;
	case SCHED_S_TASK_HAS_NOT_RUN: //#1009 2011/10/04 K.Ko  add
	//	DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - SCHED_S_TASK_HAS_NOT_RUN \n"  );
//		printf("SCHED_S_TASK_HAS_NOT_RUN\n");

		ret_b = true; //#1009 2011/10/04 K.Ko  add
		break;
	case SCHED_S_TASK_NOT_SCHEDULED: //#1009 2011/10/04 K.Ko  add
	//	DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - SCHED_S_TASK_NOT_SCHEDULED \n"  );
//		printf("SCHED_S_TASK_NOT_SCHEDULED\n");

		ret_b = true; //#1009 2011/10/04 K.Ko  add
		break;
	case SCHED_S_TASK_DISABLED:
	//	DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - SCHED_S_TASK_DISABLED \n"  );
//		printf("SCHED_S_TASK_DISABLED\n");
		break;
	default:
		DbManagerTaskBase::getLogger()->LogMessage(" --- isValidTask - xxxx \n"  );
//		printf("SCHED_S_TASK_DISABLED\n");
		ret_b = true;
		break;
	};
	///
	//	DbManagerTaskBase::getLogger()->FlushLog();

	 return ret_b;
 }
void PXMyScheduleTask::calLocaleTime(const SYSTEMTIME &input,  time_t &output)
{
	time_t cur_t = time(0);
	struct tm cur_tm = *localtime(&cur_t);

	cur_tm.tm_mon += 1;
	cur_tm.tm_year += 1900;

	//
	cur_tm.tm_year = input.wYear;
	cur_tm.tm_mon = input.wMonth;
	cur_tm.tm_mday = input.wDay;
	//
	cur_tm.tm_hour = input.wHour;
	cur_tm.tm_min = input.wMinute;
	cur_tm.tm_sec = input.wSecond;


	cur_tm.tm_mon -=1;
	cur_tm.tm_year -=1900;

	output = mktime(&cur_tm);

};

bool PXMyScheduleTask::getTimeFromLastRun(double &timeSpan) //minute
{
	HRESULT hr = S_OK;
	//
	timeSpan = 60*24*30;//30 Day
	if(!isValidTask()){
		return false;
	}

	time_t cur_t = time(0);
	//
	SYSTEMTIME RecentRunTime;
	hr = m_pITask->GetMostRecentRunTime(&RecentRunTime);
	if (FAILED(hr)) 
		return false;
 
	//#1009 2011/10/04 K.Ko 
	if(	(RecentRunTime.wYear<1) || 
		(RecentRunTime.wMonth<1) ||
		(RecentRunTime.wDay<1)
		)
	{
		return true;
	}
	 
	time_t last_run_t  ;
	calLocaleTime(RecentRunTime,last_run_t);

	double from_last_minutes = difftime(cur_t,last_run_t)/60.0;
	timeSpan = from_last_minutes;

	if(timeSpan<0) timeSpan=-timeSpan;
	return true;

}
bool PXMyScheduleTask::getTimeToNextRun(double &timeSpan)//minute
{
	HRESULT hr = S_OK;
	//
	if(!isValidTask()){
		return false;
	}

	time_t cur_t = time(0);
	//
	SYSTEMTIME NextRunTime;
	hr = m_pITask->GetNextRunTime(&NextRunTime);
	if (FAILED(hr)) 
		return false;
 
	time_t next_run_t  ;
	calLocaleTime(NextRunTime,next_run_t);

	double to_next_minutes = difftime(cur_t,next_run_t)/60.0;
	timeSpan = to_next_minutes;

	if(timeSpan<0) timeSpan=-timeSpan;
	return true;
}
bool PXMyScheduleTask::MonitorDaliy(std::string &retMess)
{
 
	HRESULT hr = S_OK;

	if(!isValidTask()){
		//#1009  2011/10/04 K.Ko
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0010] --- isValidTask error \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
		return false;
	}

	double from_last_minutes ;

	double to_next_minutes;

	if(!getTimeFromLastRun(from_last_minutes)){
		//#1009 2011/10/04 K.Ko
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0010] --- getTimeFromLastRun error \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
		return false;
	}

	//
	if(!getTimeToNextRun(to_next_minutes)){
		//#1009 2011/10/04 K.Ko
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0010] --- getTimeToNextRun error \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
		return false;
	}

#if 0
	//Dbg 2011/10/04 K.Ko
	DbManagerTaskBase::getLogger()->LogMessage(" --- to execTaskInstant \n"   );
 	DbManagerTaskBase::getLogger()->FlushLog();
#endif

	double hours_span = from_last_minutes/60.0;

#if 0
	//Dbg 2011/10/04 K.Ko
	DbManagerTaskBase::getLogger()->LogMessage(" --- hours_span %f  \n",hours_span );
 	DbManagerTaskBase::getLogger()->FlushLog();
#endif
 	if(hours_span > 24.0){ //daliy
//	if(1){ //just test it
		if(to_next_minutes > m_ReservedMinutesToNextRun ){
			if(!execTaskInstant()){
				return false;
			}
		}
	}

	return true;
}
bool PXMyScheduleTask::MonitorWeekly(std::string &retMess)
{
	HRESULT hr = S_OK;

	if(!isValidTask()){
		return false;
	}

	double from_last_minutes ;

	double to_next_minutes;

	if(!getTimeFromLastRun(from_last_minutes)){
		return false;
	}
	//
	if(!getTimeToNextRun(to_next_minutes)){
		return false;
	}

	double days_span = from_last_minutes/60.0/24.0;
	if(days_span > 7.0){ //weekly
		if(to_next_minutes > m_ReservedMinutesToNextRun ){
			if(!execTaskInstant()){
				return false;
			}
		}
	}
	return true;
}
bool PXMyScheduleTask::MonitorMonthly(std::string &retMess)
{
	HRESULT hr = S_OK;

	if(!isValidTask()){
		return false;
	}

	double from_last_minutes ;

	double to_next_minutes;

	if(!getTimeFromLastRun(from_last_minutes)){
		return false;
	}
	//
	if(!getTimeToNextRun(to_next_minutes)){
		return false;
	}

	double days_span = from_last_minutes/60.0/24.0;
	if(days_span > 30.0){ //Monthly
		if(to_next_minutes > m_ReservedMinutesToNextRun ){
			if(!execTaskInstant()){
				return false;
			}
		}
	}

	return true;
}
bool PXMyScheduleTask::MonitorSystemStart(std::string &retMess)
{
	retMess = std::string("TASK_EVENT_TRIGGER_AT_SYSTEMSTART is ignored");
	return true;
}


//2010/04/14 K.Ko
bool PXMyScheduleTask::MonitorExec(std::string &retMess)
{
 
	retMess = "";

	 
	if(!getTriggerInfo( m_oTrigger,  m_info)){

		//#1009  2011/10/04 K.Ko
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0010] --- getTriggerInfo error \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();

		return false;
	}

#if 0
	//Dbg 2011/10/04 K.Ko
	{
		DbManagerTaskBase::getLogger()->LogMessage("m_info --- time:%d, day:%d ,mon:%d, year:%d ,trig:%d, %d ,%d, %d, %d, %d \n",  
			m_info.m_startTime ,        
			m_info.m_beginDay ,
			m_info.m_beginMonth,
			m_info.m_beginYear ,
			//
			m_info.m_triggerType ,
			m_info.m_minutesDuration,  
			m_info.m_minutesInterval,  	
			m_info.m_startDaysOfWeek, 
			//
			m_info.m_WeeksInterval,
			m_info.m_startDays
			);
 		DbManagerTaskBase::getLogger()->FlushLog();
	}
#endif



	bool ret_b = true;
	switch( m_oTrigger.TriggerType){
	case TASK_TIME_TRIGGER_DAILY:
		ret_b = MonitorDaliy(retMess);
		break;
	case TASK_TIME_TRIGGER_WEEKLY:
		ret_b = MonitorWeekly(retMess);
		break;
	case TASK_TIME_TRIGGER_MONTHLYDATE:
		ret_b = MonitorMonthly(retMess);
		break;
	case TASK_EVENT_TRIGGER_AT_SYSTEMSTART:
		ret_b =MonitorSystemStart(retMess);
		break;
	}
	//
	
	return ret_b;
}
bool PXMyScheduleTask::RuningStaus(bool &runingFlag)
{
	HRESULT hr = S_OK;
	HRESULT status;
 
	if(!init()) {
		return false;
	}
	 
	hr = m_pITask->GetStatus( &status);
	if (FAILED(hr)) 
		return false;

	runingFlag = (status == SCHED_S_TASK_RUNNING) ;

	return true;

}
bool  PXMyScheduleTask::execTaskInstant( )
{

	HRESULT hr = S_OK;
       
	hr = m_pITask->Run();
	if (FAILED(hr)) {
		//#1009  2011/10/04 K.Ko
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0010] --- execTaskInstant error \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
		return false;
	}else{
#if 0
		//Dbg 2011/10/04 K.Ko
		DbManagerTaskBase::getLogger()->LogMessage("  --- execTaskInstant OK \n"   );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
#endif
	}

	
	return true;
}

