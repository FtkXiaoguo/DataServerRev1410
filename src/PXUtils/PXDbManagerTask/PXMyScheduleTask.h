// PXMyScheduleTask.h: PXMyScheduleTask クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PXMYSCHEDULETASK_H__D09B9FC5_D44B_4EA5_830F_447C6235BAFF__INCLUDED_)
#define AFX_PXMYSCHEDULETASK_H__D09B9FC5_D44B_4EA5_830F_447C6235BAFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\PXAppComLib\AppComScheduleTask.h"

#include <string>

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

class PXMyScheduleTask : public AppComScheduleTask  
{
public:
	PXMyScheduleTask();
	PXMyScheduleTask (const TaskInfo& iTaskInfo);
	virtual ~PXMyScheduleTask();
	bool init();

	bool MonitorExec(std::string &retMess);
	bool RuningStaus(bool &runingFlag);
protected:

	bool isValidTask();
	bool MonitorDaliy(std::string &retMess);
	bool MonitorWeekly(std::string &retMess);
	bool MonitorMonthly(std::string &retMess);
	bool MonitorSystemStart(std::string &retMess);
//
	bool getTimeFromLastRun(double &timeSpan);//minute
	bool getTimeToNextRun(double &timeSpan);//minute
	bool execTaskInstant();
//
	int  m_ReservedMinutesToNextRun;
 
	//
	TASK_TRIGGER   m_oTrigger;
	TaskInfo  m_info;

	static void calLocaleTime(const SYSTEMTIME &input,  time_t &output);

};

#endif // !defined(AFX_PXMYSCHEDULETASK_H__D09B9FC5_D44B_4EA5_830F_447C6235BAFF__INCLUDED_)
