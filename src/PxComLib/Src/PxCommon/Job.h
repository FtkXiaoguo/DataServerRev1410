/***********************************************************************
 * Job.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2004, All rights reserved.
 *
 *	PURPOSE:
 *		I/O interface for Queue jobs
 *
 *	AUTHOR(S):  Rob Lewis, March 2004
 *				Chetan Krishnamurthy, Oct, 2005
 *  
 *-------------------------------------------------------------------
 */
#ifndef JOB_H
#define JOB_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include <windows.h>
#include <time.h>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h> 
#include <map>
#include "AqCore/AqCore.h"
#include "rtvloadoption.h"

typedef std::map<std::string, std::string> KVP_MAP;

enum eJobStatus
{
	kQWorking = 0,
	kQPending,
	kQRetry,
	kQSuspended,
	kQRequest,
	kQSuccessful,
	kQCancelled,
	kQFailed,

	kQControl,
	kQTmp,

	kQSentinel,
	kQNumberOfQueues,
	kQFailedNoRetry
};

extern const char* gQueueStr[];
extern int gRetryTime[];

enum eJobError
{
	kJobFail = -1,
	kJobSuccess = 0,
	kJobInvalidJobType,
	kJobInvalidDataPath,
	kJobInvalidJobPath,
	kJobInvalidArgument,
	kJobInvalidStatus,
	kJobInvalidRootPath,
	kJobRenameFailed,
	kJobDeleteFailed,
	kJobEmptyJob,
	kJobUnrecognizedCommand,
	kJobInvalidJob,
	kJobFailedToExecuteJob
};

enum eJobType
{
	kJobInvalidType = -1,
	kSendJob = 0,
	kPrintJob,
	kControlJob,
	kPrefetchJob,
	kRetrieveJob
};

enum eJobPriority
{
	kJobUnknownPriority		= -1,
	kJobNowPriority			= 99,
	kJobEmergencyPriority	= 100,
	kJobHighPriority		= 200,
	kJobNormalPriority		= 300,
	kJobLowPriority			= 400
};

enum eJobRunState
{
	kJobWaitingToRun = 0,
	kJobRunning,
	kJobDone
};

const int gDaysToKeepCompletedJobs = 2;
const int gMinutesUntilAutoPromotion = 60;

extern const char* kJOBKEYdisplayName;
extern const char* kJOBKEYtargetProcessName;
extern const char* kJOBKEYrequestingUserID;
extern const char* kJOBKEYjobID;
extern const char* kJOBKEYsubmitTime;
extern const char* kJOBKEYsourceAE;
extern const char* kJOBKEYdestAE;
extern const char* kJOBKEYpatientID;
extern const char* kJOBKEYpatientName;
extern const char* kJOBKEYseriesNumber;
extern const char* kJOBKEYaccessionNumber;
extern const char* kJOBKEYpriority;
extern const char* kJOBKEYstatus;
extern const char* kJOBKEYcompleted;
extern const char* kJOBKEYtotal;
extern const char* kJOBKEYexecutionTime;
extern const char* kJOBKEYcomment;
extern const char* kJOBKEYprecedingProcess;
extern const char* kJOBKEYAssociatedProcess;
extern const char* kJobKeyAssociatedProcessID;
extern const char* kJobKeyprecedingProcessID;
extern const char* kJobKeyPathToInputFile;
//--------------------------------------------------------------------
//
class CJob
{
public:

	enum
	{
		kSubmitTime = 1,
		kSequenceTime,
	};

	static int GenerateID(bool iReset = false);
	
	//function to add key value pair into the jobfile
	void AddKey(const char* iKey, std::string iValue);	
	void AddKey(const char* iKey, int iValue)
	{
		char val[32];
		_snprintf(val, sizeof val, "%d", iValue);
		val[sizeof(val) - 1] = 0;
		AddKey(iKey, val);
	}
	
	//	Create a new job - call Save() to commit to disk
	//	Note: After creation, you have to set all necessary members before preceed.
	virtual int Create(const char* iID, const char* iPath, const char* iDataPath = 0);
	
	//
	//	Job control functions
	//
	int Move(const std::string iNewDir, const std::string iNewFilename = "");
	int Submit(const std::string iNewPath, int iRunNow = 0); 

	
	//
	//	File i/o
	//
	int Load(const std::string iDir, const std::string iFilename, int iLocked = 0, const std::string iProcessNodeID = "");
	int Load(const char* iDir, const char* iFilename, int iLocked = 0, const std::string iProcessNodeID = "");
	int Load(const std::string iFullPath, int iLocked = 0);
	int Load(int iLocked = 0, std::string iProcessNodeID ="");
	int Save(int iLocked = 1, bool iCreateIfNotThere = false);
	int Clean(int iLocked = 0);

	//not being used right now - chetan 26th Jan
	std::string GetProgFileName(std::string iJobFile);

	//	Member access
	//
	void SetPriority(int iVal)    { m_priority = iVal; }
	int  GetPriority(void) const { return m_priority; }

	int  GetMaxRetries(void);

	void SetRetries(int iRetries) { if(m_retries<0) m_retries = 0; else m_retries = (m_retries>GetMaxRetries())?GetMaxRetries():iRetries; }
	int  GetRetries(void) const { return m_retries; }

	void SetStatus(int iStatus) { m_status = iStatus; }
	int  GetStatus(void) const { return m_status; }

	void SetDataPath(const std::string iPath) { ASTRNCPY(m_dataPath, iPath.c_str()); }
	std::string GetDataPath(void) const { return (strlen(m_dataPath)) ? std::string(m_dataPath) : ""; }

	void SetRetryPath(const std::string iPath) { ASTRNCPY(m_retryPath, iPath.c_str()); }
	std::string GetRetryPath(void) const { return (strlen(m_retryPath)) ? std::string(m_retryPath) : ""; }

	void SetFileFilter(const std::string iFilter) { ASTRNCPY(m_fileFilter, iFilter.c_str()); }
	std::string GetFileFilter(void) const { return (strlen(m_fileFilter)) ? std::string(m_fileFilter) : ""; }

	std::string GetExecuteTime(void) const { return m_executeTime; }
	void SetExecuteTime(std::string iTime, int iMinutesToAdd = 0);
	void SetExecuteTime(void); //	Set execute time to now
	
	void SetCancelledTime();
	void SetCancelledTime(time_t iTime);
	time_t GetCancelledTime(void); 

	void SetSequenceBeginTime(std::string iTime, int iMinutesToAdd = 0);
	void SetSequenceBeginTime(time_t iTime);
	std::string GetSequenceBeginTime(void) const { return m_sequenceBeginTime; }
	
	
	void SetTargetProcessName(const std::string iName) {ASTRNCPY(m_targetProcessName, iName.c_str());}
	std::string GetTargetProcessName(void) { return m_targetProcessName;}

	// exepted that a slash is provided a the end !!!!!
	void SetTargetProcessPath(const std::string iName) {ASTRNCPY(m_targetProcessPath, iName.c_str());}
	std::string GetTargetProcessPath(void) { return m_targetProcessPath;}

	void SetPrecedingProcessName(const std::string iName) {ASTRNCPY(m_precedingProcess, iName.c_str());}
	std::string GetPrecedingProcessName(void) { return m_precedingProcess;}

	void SetPrecedingProcessID(const std::string iName) {ASTRNCPY(m_precedingProcessID, iName.c_str());}
	std::string GetPrecedingProcessID(void) { return m_precedingProcessID;}

	void SetAssociatedProcess(const std::string iName) {ASTRNCPY(m_associatedProcess, iName.c_str());}
	std::string GetAssociatedProcess(void) { return m_associatedProcess;}

	void SetAssociatedProcessID(const std::string iName) {ASTRNCPY(m_associatedProcessID, iName.c_str());}
	std::string GetAssociatedProcessID(void) { return m_associatedProcessID;}

	void SetRetryAttempt(bool iVal){ m_retryAttempt = iVal; }
	int  GetRetryAttempt(void) const { return m_retryAttempt; }

	void SetCommandLineOptions(const std::string iOptions) {ASTRNCPY(m_cmdLineOptions, iOptions.c_str());}
	std::string GetCommandLineOptions(void) { return m_cmdLineOptions;}
	void AddCommandLineOptions(const std::string iOptions);

	void SetDoneUnits(int iN) { m_doneUnits = iN; }
	int  GetDoneUnits(void) const { return m_doneUnits; }

	void SetTotalUnits(int iN) { m_totalUnits = iN; }
	int  GetTotalUnits(void) const { return m_totalUnits; }

	void SetRunState(int iState) { m_runState = iState; }
	int  GetRunState(void) const { return m_runState; }

	void SetSequenceStatus(bool iSequenceStatus) {m_sequenceStatus = iSequenceStatus;}
	bool GetSequenceStatus(void) const {return m_sequenceStatus;}
	
	void SetValidJobFile(bool iSequenceStatus) {m_validJobFile = iSequenceStatus;}
	int GetValidJobFile(void) const {return m_validJobFile;}
	
	void SetProcessID(int iProcessID) {m_processID = iProcessID;}
	int GetProcessID(void) const {return m_processID;}

	void SetRequestingUserID(int iID) {m_requestingUserID = iID;}
	int GetRequestingUserID(void) const {return m_requestingUserID;}

	void SetPatientName(const char* iVal) { ASTRNCPY(m_patientName, iVal); m_option.Add("patientName",m_patientName);}
	std::string GetPatientName(void) const { return m_patientName; }

	void SetCommandLine(const char* iVal) { ASTRNCPY(m_commandLine, iVal); m_option.Add("commandLine",m_commandLine);}
	std::string GetCommandLine(void) const { return m_commandLine; }

	void SetPatientID(const char* iVal) { ASTRNCPY(m_patientID, iVal); m_option.Add("patientID",m_patientID);}
	std::string GetPatientID(void) const { return m_patientID; }
	
	void SetSeriesNumber(const char* iVal) { ASTRNCPY(m_seriesNumber, iVal); m_option.Add("seriesNumber",m_seriesNumber);}
	std::string GetSeriesNumber(void) const { return m_seriesNumber; }

	std::string GetID(void) const { return m_id; }
	std::string GetDir(void) { return m_dir; }
	std::string GetSubmitTime(void) const { return m_submitTime; }

	std::string GetValue(std::string iKey);

	void SetVerbose(int iVerbose) { m_verbose = iVerbose; }
	int  GetVerbose(void) const { return m_verbose; }

	void SetDisplayName(const std::string iName) { ASTRNCPY(m_displayName, iName.c_str()); m_option.Add("displayName",m_displayName);}
	std::string GetDisplayName(void) { return m_displayName; }

	void SetSourceAE(const char* iVal) { ASTRNCPY(m_sourceAE, iVal); m_option.Add("sourceAE",m_sourceAE);}
	std::string GetSourceAE(void) const { return m_sourceAE; }

	void SetDestAE(const char* iVal) { ASTRNCPY(m_destAE, iVal); m_option.Add("destAE",m_destAE);}
	std::string GetDestAE(void) const { return m_destAE; }

	void SetAccessionNumber(const char* iVal) { ASTRNCPY(m_accessionNumber, iVal); m_option.Add("accessionNumber",m_accessionNumber);}
	std::string GetAccessionNumber(void) const { return m_accessionNumber; }

	void SetProcessNodeID(const char* iVal) { ASTRNCPY(m_processNodeID, iVal); m_option.Add("ProcessNodeID",m_processNodeID);}
	std::string GetProcessNodeID(void) const { return m_processNodeID; }

	void SetLinkToManifest(const char* iVal) { ASTRNCPY(m_linkToManifest, iVal); m_option.Add("LinkToManifest",m_linkToManifest);}
	std::string GetLinkToManifest(void) const { return m_linkToManifest; }

	virtual bool CJob::IsTimeToRun(void );
	//
	//	Time management
	//
	bool ItsTimeToRun(void);
	static std::string GetRemainingTime(const char* iTime);
	void UpdateTimeStamp(bool iSetExecuteTime = false);
	int HowManyMinutesHaveIBeenInTheQueue(int iKey = kSubmitTime);

	//
	//	Utility
	//
	void UpdateForRetry(bool iIncrement = true);
	int UpdateFilename(bool iUseExecuteTime = false);	
	virtual std::string GenerateCommandLine(void);
	void Print(FILE* iFP = 0);
	bool HasCompleted(void) const;
	
	//	
	//	Housekeeping
	//
	virtual ~CJob(void);

	const CJob& operator = ( const CJob& iData ) 
	{
		if(this != &iData) Copy(iData);
		return *this; 
	};

	void Copy ( const CJob& iData );

	CJob(const CJob& iData) 
	{ 
		Copy(iData); 
	};

	void Clear(void) { m_option.Clear(); }

	CJob() ;	

protected:

	//
	//	Convenience functions
	//
	void InitializeMembers(void);
	void InitializeOptions(void);
	std::string QueueName(void);
	std::string GetFullPath(void) { return std::string(m_dir) + "/" + std::string(m_filename); } 
	void SetDir(const char* iDir) { ASTRNCPY(m_dir, iDir); }
	void SetFilename(const char* iFilename) { ASTRNCPY(m_filename, iFilename); }
	static time_t JobTimeToTimeT(const char* iTime, time_t& oResult, int iMinutesToAdd = 0);
	void SetExecuteTime(time_t iTime);
	int RunNow(void);
	
	//
	//	Data members
	//
	KVP_MAP m_addedOptions;
			//	internal representation of KVP's from job file
	char m_dir[MAX_PATH];			//	path to the directory where the job file lives
	char m_filename[MAX_PATH];		//	job file name
	char m_id[16];				//	job ID (should be obtained by running NewJobID.exe
	char m_dataPath[MAX_PATH];		//	path to the directory where the job's data files live
	char m_retryPath[MAX_PATH];		//	where to find the files that need to be retried
	char m_fileFilter[32];		//	used by the target process to restrict the processed files
	char m_submitTime[64];		//	time when job was submitted
	char m_executeTime[64];		//	time when job will execute
	char m_sequenceBeginTime[64];
	char m_cancelledTime[64];
	int  m_priority;			//	see eJobPriority above for range of values
	int  m_retries;				//	how many times has this job been retried?	
	int	 m_doneUnits;			//	used for progress updates
	int  m_totalUnits;			//	used for progress updates
	int  m_runState;			//	set when the job is finished executing
	int  m_status;				//	which queue the job is in.  See eJobStatus above.
	int	 m_verbose;
	int  m_total;
	int  m_processID;
	int  m_validJobFile;
	bool m_sequenceStatus;
	bool m_retryAttempt;
	int	 m_requestingUserID;
	char m_patientName[256];
	char m_patientID[64];
	char m_seriesNumber[16];
	//	These are used to build the command line
	char m_targetProcessName[64];
	char m_cmdLineOptions[512];
	char m_progressPath[MAX_PATH];
	char m_precedingProcess[MAX_PATH];
	char m_associatedProcess[MAX_PATH];
	char m_displayName[32];
	char m_sourceAE[17];
	char m_destAE[17];
	char m_accessionNumber[17];
	char m_targetProcessPath[MAX_PATH];
	char m_precedingProcessID[MAX_PATH];
	char m_associatedProcessID[MAX_PATH];
	char m_processNodeID[128];
	char m_linkToManifest[MAX_PATH];
	char m_commandLine[MAX_PATH];
	iRTVOptions m_option;
};

#endif
