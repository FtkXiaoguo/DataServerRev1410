#pragma once

#pragma warning (disable: 4530)

#include "string"
#include "vector"

//#define USE_RECYECLEQUEUE

#define ImageFileNameLen (128)
struct QueueEntryBinHeader
{
	unsigned int id_code;
	unsigned int seriesNumber;
	char seriesFolder[1024];
};

class CPxQueueBinFile
{
public:
	CPxQueueBinFile();
	~CPxQueueBinFile();
	bool writeQueueBinFile(const std::string &fileName);
	bool writeQueueBinFileEx(const std::string &fileName,const std::string &serisFolder,const std::vector<std::string> &list);
	bool readQueueBinFile(const std::string &fileName);
	bool readQueueBinFileEx(const std::string &fileName,std::string &serisFolder,std::vector<std::string> &list);
//////
	std::string m_seriesFolder;
	std::vector<std::string> m_imageFileList;
protected:
	QueueEntryBinHeader m_header;
};

#define SQLITE_DB_FILTE "PxSendQueue.db"

class CPxQueueEntryExt
{
public:
	//#142_NoPatientName_NoComment
	//std::string m_PatientName ;//VARCHAR(64)   
	std::string m_PatientID ;//VARCHAR(16)
	std::string m_Date ;//VARCHAR(64)  study date or series date time
	std::string m_BirthDate; //VARCHAR(64) 0212/06/15 added K.Ko
	int	m_SeriesNumber;
	int m_EntryImages; //used by block ( PxQueuelevel_EntryFile)
	int m_ImagesInSeries;
	//#142_NoPatientName_NoComment
	std::string m_Comment ;//VARCHAR(64)  study description or series description
};
class CPxQueueEntry
{
enum 
{ 
	kErrorOnly	= 0, 
	kWarning	= 1, 
	kInfo		= 2, 
	kDebug		= 3, 
	kTrace		= 4
};

public:
	enum PxQueueLevel {
		PxQueueLevel_Unknown	= 0,
		PxQueueLevel_Study		= 1,
		PxQueueLevel_Series		= 2,
		PxQueueLevel_Image		= 3,
		PxQueuelevel_EntryFile  = 4,
	};
	enum PxQueueStatus {
		PxQueueStatus_Unknown		= 0,
		PxQueueStatus_Processing	= 1,  //Processing
		PxQueueStatus_Standby		= 2,  //waiting for read process
		PxQueueStatus_Finished		= 4,  //Processing is finshied.
		PxQueueStatus_Failed		= 8,  //Processing is faile, re-try it.
		PxQueueStatus_Empty			= 16, //Queue is empty, can use it recycle.
	};
	enum PxQueuePriorityLevel {
		PxQueuePriority_Low			= 0,
		PxQueuePriority_Default		= 1,
//		PxQueuePriority_High		= 2,
		PxQueuePriority_All			= 4,
	};

	enum PxQueueCmdType{//#48
		PxQueueCmd_DICOM = 0,
		PxQueueCmd_JPEG,
	};
	
	CPxQueueEntry(void) ;
	~CPxQueueEntry(void) ;

	
bool operator ==( const CPxQueueEntry &entry2) const;
//
	int m_QueueID ;//
	std::string m_JobID ;			// VARCHAR(128)  //#19 2012/05/21 K.Ko
	int m_cmdID;
	int	m_SendLevel ;// 0
	std::string m_StudyInstanceUID ;// VARCHAR(64) , // PxQueueStudyLevel
	std::string m_SeriesInstanceUID;// VARCHAR(64) , // PxQueueSeriesLevel
	std::string m_SOPInstanceUID;	// VARCHAR(64) , // PxQueueImageLevel
	std::string m_DestinationAE ;//VARCHAR(64)  NOT NULL,
	int	m_Priority ;
	int	m_Status ;		// PxQueueStatus
	int m_RetryCount;
//	char m_ReadFlag;	// 
//	char m_WriteFlag;
	__int64 m_CreateTime;//datetime DEFAULT GETDATE()
	__int64 m_AccessTime ;//datetime DEFAULT GETDATE()
	//
	CPxQueueEntryExt m_extInfo;
	//
};
class CPxDB;
//class TRMutex;
class CMySecurityMutex;
class TRLogger;

 
class CPxQueueProc
{
	class CLockQueue 
	{
	public:
		CLockQueue(CPxQueueProc *queueProc)
		{
			m_QueueProc = queueProc;
			m_QueueProc->lockDB();
		}
		~CLockQueue( )
		{
			 
			m_QueueProc->unLockDB();
		}
	protected:
		CPxQueueProc *m_QueueProc;
		 

	};
public:

	CPxQueueProc(void);
	~CPxQueueProc(void);

	//static __time64_t getCurTime();//is time(0);

	static void Global_init(bool global=true);
	void setBreakFlag(bool flag){m_breakFlag = flag;};
	static void setLogger(TRLogger *logger);
	//
	static std::string getQueueEntryFileName(const CPxQueueEntry &entry);
	//
	

	//
	virtual void initRes();
	virtual void selTest();
 
	virtual bool isValid(){ return m_initFlag;};
	void* getSendQueuEvent() { return m_sendQueuEvent;};

	bool readQueueSimpleEntry(std::vector<CPxQueueEntry> &queueTemp,int priority,int status1=CPxQueueEntry::PxQueueStatus_Standby,int status2=CPxQueueEntry::PxQueueStatus_Unknown);
	bool readQueueExt(int priority,int status1=CPxQueueEntry::PxQueueStatus_Standby,int status2=CPxQueueEntry::PxQueueStatus_Unknown);
	bool readQueueAll();
	

	bool addQueue(const CPxQueueEntry &entry);

	bool deleteQueue(int QueueID);

	static void setupQueueDBName(const std::string dbname){ m_DBName = dbname;};;
	static void setupQueueEntryFolder(const std::string folder) { m_EntryFolder = folder;};
	
	/*
	*  find out the complete record to reuse it
	*/
	bool recycleQueue(int  minutes ,int hours=0, int days=0);

	int getQueueSize() { return m_queueFullEntry.size();};

	/*
	* change the Status
	*/
	bool changeStatus(int id,int toStatus,int fromStatus=CPxQueueEntry::PxQueueStatus_Unknown);
	bool changePriority(int id,int priority,int status=CPxQueueEntry::PxQueueStatus_Unknown, int RetryCount=-1);
	//
	bool getSeriesLevel();
static void checkPriority(CPxQueueEntry &entry);
 

 bool getRecycleIDs(std::vector<int> &recyceIDs);
 std::vector<CPxQueueEntry> &getEntryList() { return m_queueFullEntry;};
 
 //#19 2012/05/21 K.Ko
 void createJobID(CPxQueueEntry &entry,bool newFlag=true);
 static std::string genJobUID(const CPxQueueEntry &entry);
 
 bool readOneEntry(int id,CPxQueueEntry &entry);

protected:

	void deleteEntryFile(const CPxQueueEntry &entry);
	//#19 2012/05/21 K.Ko

	std::string m_currentJobID;
	std::string m_ProcThreadID;

	void initDB();
	void lockDB();
	void unLockDB();
	bool addEntryToQueue(const CPxQueueEntry &entry,int newStatus=CPxQueueEntry::PxQueueStatus_Unknown);
	bool readQueue_in(std::vector<CPxQueueEntry> &queueTemp,bool extInfo,int priority,int status1=CPxQueueEntry::PxQueueStatus_Standby,int status2=CPxQueueEntry::PxQueueStatus_Unknown);
	bool readQueueAll_in(std::vector<CPxQueueEntry> &queueTemp,bool extInfo);
	bool ExeReadQueue(std::vector<CPxQueueEntry> &queueTemp,bool extInfo, const char *sql);
	
	virtual const char *getQueueTableName() const = 0;
	virtual const char *getChgQueueStatusProcName() const = 0;

	
virtual void LogMessage(int iLevel, const char *fmt, ...) ;


//	bool updatePriority(const CPxQueueEntry &entry);

//	bool setStatus(int id, int status);
	bool getStatus(int id, int &status);
 
	bool addNewQueue(const CPxQueueEntry &entry);
	bool updateQueue(int id, const CPxQueueEntry &entry,int checkShareStatus=CPxQueueEntry::PxQueueStatus_Unknown);
	std::vector<CPxQueueEntry> m_queueFullEntry;
	//
	std::vector<CPxQueueEntry> m_queueSeriesLevel;
	int m_runCount;
 
	CPxDB *m_queue_db;
	static std::string m_DBName;
	static std::string m_EntryFolder;
	//
	CMySecurityMutex *m_mutex_sqliteDB;
	static CMySecurityMutex *m_mutex_sqliteDB_send;
	static CMySecurityMutex *m_mutex_sqliteDB_result;
	bool m_initFlag;
	//
	bool m_breakFlag;
	//
	/////////////
	bool m_needQueuEvent; //2012/06/21
	static void* m_sendQueuEvent;
	static bool m_globalInited;
	//
	int m_dbEexRetryNN; //DBê⁄ë±retryâÒêî #29 2012/06/22 K.Ko  See Also _SQADataSQLite::m_retryNN

};
