// DbScheduleTask.h: CDbScheduleTask クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBSCHEDULETASK_H__55E82276_C3E5_4B64_8C01_73F8DA64B6C2__INCLUDED_)
#define AFX_DBSCHEDULETASK_H__55E82276_C3E5_4B64_8C01_73F8DA64B6C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

struct TaskInfo;
class CDbScheduleTask  
{
public:
	CDbScheduleTask();
	virtual ~CDbScheduleTask();

	bool doMonitor(const std::string taskName,bool &timeoutFlag,int waitTimeout=0/*Sec*/);

	std::string getRunMessage() { return m_runMessage;};
protected:
	bool runTask(TaskInfo  *oInfo);
	bool taskIsRuning(TaskInfo  *oInfo,bool &runingFlag );
	std::string m_runMessage;
};

#endif // !defined(AFX_DBSCHEDULETASK_H__55E82276_C3E5_4B64_8C01_73F8DA64B6C2__INCLUDED_)
