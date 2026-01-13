// DbScheduleTask.cpp: CDbScheduleTask クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbScheduleTask.h"

#include <time.h>
 
#include "PXMyScheduleTask.h"

#include "AqCore/TRLogger.h"
#include "AqCore/AqString.h"

#include "DbManagerTaskBase.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDbScheduleTask::CDbScheduleTask()
{

}

CDbScheduleTask::~CDbScheduleTask()
{

}
bool CDbScheduleTask::doMonitor(const std::string taskName,bool &timeoutFlag,int waitTimeout/*Sec*/)
{
	bool ret_b = true; //fix bug 2012/06/18 K.Ko ref: aqviewer #1009
	TaskInfo  oInfo;

#if 0
	std::wstring str2(taskName.length(), L' '); // Make room for characters

// Copy string to wstring.
	std::copy(taskName.begin(), taskName.end(), str2.begin());


 	//	oInfo.m_lpcwszTaskName = L"AqDBBackup";
	oInfo.m_lpcwszTaskName = str2.c_str();
#else
	//change 2012/06/18 
	AqUString u_str_temp;
	u_str_temp.Format(L"%S",taskName.c_str());
	oInfo.m_lpcwszTaskName = u_str_temp;
#endif
	if(!runTask(&oInfo)){
		DbManagerTaskBase::getLogger()->LogMessage("ERROR:[C0005] --- runTask %s failed \n" ,taskName.c_str() );
 	 	DbManagerTaskBase::getLogger()->FlushLog();
		return false;
	}
	 
	timeoutFlag = false;
	if(waitTimeout>0){
		int waitBaseTime = 500;//mSec
		int loopNN = (int)(waitTimeout*1000.0/waitBaseTime);
		bool runingFlag=false;
		for(int i=0;i<loopNN;i++){
			if(!taskIsRuning(&oInfo,runingFlag)){
				ret_b = false;
				break;
			}
			if(!runingFlag){
				ret_b = true;
				break;
			}
		}
		timeoutFlag = runingFlag;
	}
	 
	return ret_b;
 
}
bool CDbScheduleTask::runTask(TaskInfo  *oInfo)
{
	PXMyScheduleTask procTask(*oInfo);
  
	return procTask.MonitorExec(m_runMessage);
	 
}
bool CDbScheduleTask::taskIsRuning(TaskInfo  *oInfo,bool &runingFlag )
{
	//you should re-instance PXMyScheduleTask
	PXMyScheduleTask waitTask(*oInfo);
    return waitTask.RuningStaus(runingFlag);
}