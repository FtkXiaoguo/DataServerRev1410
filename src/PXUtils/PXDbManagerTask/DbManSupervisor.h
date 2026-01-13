// DbManSupervisor.h: CDbManSupervisor クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBMANSUPERVISOR_H__72C1670E_5695_44CD_BFDB_EF680645C95B__INCLUDED_)
#define AFX_DBMANSUPERVISOR_H__72C1670E_5695_44CD_BFDB_EF680645C95B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include <vector>



#include "DbManagerTaskBase.h"

class PXMyScheduleTask;
class CDbManSupervisor  : public DbManagerTaskBase
{
#define MONITOR_TASK_MAX (8)
public:
	CDbManSupervisor();
	virtual ~CDbManSupervisor();
	virtual int doMain(int argc, char** argv) ;

	 bool doMonitorTask();
	void addTaskName(std::string taskName);
protected:
	virtual bool loadConfiguration(const char *fileName);
	PXMyScheduleTask *m_ScheduleTask;
	//
	int  m_RunTimeout[MONITOR_TASK_MAX]; //Sec
	//
	std::vector<std::string> m_MonitorTaskList;
};

#endif // !defined(AFX_DBMANSUPERVISOR_H__72C1670E_5695_44CD_BFDB_EF680645C95B__INCLUDED_)
