/***********************************************************************
 * JobControl.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2004, All rights reserved.
 *
 *	PURPOSE:
 *		API for performing Job Control tasks
 *
 *	AUTHOR(S):  Rob Lewis, April 2004
 *  
 *-------------------------------------------------------------------
 */
#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <string>
#include <map>
#include "Job.h"

enum eQueueFilter
{
	kmWorking			= (1 << 0),
	kmPending			= (1 << 1),
	kmRetry				= (1 << 2),
	kmSuspended			= (1 << 3),
	kmRequest			= (1 << 4),
	kmSuccessful		= (1 << 5),
	kmCancelled			= (1 << 6),
	kmFailed			= (1 << 7),
	kmControl			= (1 << 8),
	kmAllJobs			= (1 << 9) - 1 ,
	kmAllActiveJobs		= kmControl + kmWorking + kmRetry,   // control, working, retry //we dont need to look at everything. pending is polled seperately
	kmAllLiveJobs		= kmPending + kmWorking + kmRetry + kmSuspended,
	kmDependentJobs		= kmSuccessful + kmSuspended + kmCancelled + kmFailed + kmPending  + kmWorking + kmRetry, // - succesful and control. request is checked seperately

};

typedef std::vector<CJob*> JOB_VECTOR;
typedef std::map<int, JOB_VECTOR> JOB_MAP;

//--------------------------------------------------------------------
//
class CJobControl
{
public:
	static std::string GetQueuePath(std::string iJobType, int iStatus);
	static std::string CJobControl::GetRequestQueuePath(void);
	static std::string CJobControl::GetRequestPath(void);
	static std::string CJobControl::GetJobIDCreationPath(std::string iProcessName);
	
	static void SplitInputValue(std::string precedingProcessName, std::vector<std::string> & oValue);
	static std::string GetRootPath();
	//	NOTE: the filter is a mask which is 
	int FindJob(std::string iJobType, std::string iJobID, CJob*& oJob);
	static int  GetJobList(std::string iJobType, JOB_MAP& oJobMap, long iQueueFilter = kmAllJobs, std::string iKey = "", std::string iValue = "", int nResults = 100, int iOverRide = 0);
	static int GetRequestJobList(std::string iJobType, JOB_MAP& oJobMap, std::string iKey, std::string iValue, int nResults);
	static void ReleaseJobMap(JOB_MAP& ioJobs);
	static int GetTotalNumberOfJobs(std::string iJobType, long iQueueFiltrMask, int& oTotalNumberOfJobs);
	static int GetJobList(std::string iJobType, long iQueueFilter, int iJobStartingIndex, int iMaxNumberItems, std::vector<std::string>& oVal);

	static int Suspend(std::string iJobType, std::string iJobID);
	static int Resume(std::string iJobType, std::string iJobID);
	static int Cancel(std::string iJobType, std::string iJobID);
	static int Bump(std::string iJobType, std::string iJobID, int iNewPriority);

	static std::string GetWorkingFolderRoot(char* iJobID, bool iCreateWorkingFolder = true);

	//multi-node case - chetan
/*	static void SetProcessNodeID(std::string iProcessNodeID)
	{
		m_processNodeId = iProcessNodeID;

	}
	static std::string GetProcessNodeID()
	{
		return m_processNodeId;
	}*/

private:
	static int CreateAndSubmit(int iCmd, std::string iJobType, std::string iJobID, int iNewPriority = -1);
	static CJob* LoadJobWithProgress(const char* iPath, const char* iName, int iQueueIndex, int iJobType);
	static CJob* CreateAndLoad(const char* iPath, const char* iName, int iQueueIndex, int iJobType);
	static std::string GetJobType(std::string iJobType);

	//multi-node case - chetan
	//static std::string m_processNodeId;

	CJobControl(void)
	{
		//multi-node case - chetan
		//m_processNodeId = "";
	}

};

#endif //	JOB_CONTROL_H