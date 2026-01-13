#include "PxQueueProc.h"

#include "PxDB.h"
 #include "AqCore/TRLogger.h"

#ifndef USE_MSSQL_SERVER_QUEUE
#include "Aqcore/trcriticalsection.h"
#endif

#include "AqCore/TRPlatform.h"

static TRLogger *_g_logger_ = 0;

//////////////////////////////
bool ExeSQLExecute(SQA &sqa,CPxDB *pDb,int retryNN)
{
	int retcd;
	 
	for(int try_i=0;try_i<retryNN;try_i++){
		retcd = pDb->SQLExecuteBegin(sqa);
		pDb->SQLExecuteEnd(sqa);
		if(retcd == kOK) break;
	}
	return retcd;
}
///////////////////////////////
// for windows 7
class CSetSecurityAttNullACL
{
public:
	CSetSecurityAttNullACL(){
		m_psd = 0;
	}
	~CSetSecurityAttNullACL(){
		if(m_psd) free(m_psd);
	}
	bool setSecurityAttNullACL(SECURITY_ATTRIBUTES &sa)
	{
		if(m_psd) free(m_psd);

	
		m_psd = (SECURITY_DESCRIPTOR *) malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		if(m_psd == 0) return false;
		
		if(!InitializeSecurityDescriptor(m_psd,SECURITY_DESCRIPTOR_REVISION)){
			free(m_psd);
			return false;
		}
		 if (!SetSecurityDescriptorDacl(m_psd, TRUE, (PACL) NULL, FALSE))
		{
			free(m_psd);
			return false;
		}	 
		//
		sa.nLength = sizeof (SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = m_psd;
		sa.bInheritHandle = TRUE; 

		return true;
	}
protected:
	SECURITY_DESCRIPTOR *m_psd;
};

class CMySecurityEvent : public CSetSecurityAttNullACL
{
public:
	CMySecurityEvent()
	{
	}
	HANDLE MyCreateEnvet(	BOOL bManualReset,                       // リセットのタイプ
						  BOOL bInitialState,                      // 初期状態
						  LPCTSTR lpName                           // イベントオブジェクトの名前
						) 
	{
#if 0
		//既存のものをＯｐｅｎしてみる
	//	m_event = ::OpenEvent(EVENT_MODIFY_STATE,TRUE,lpName);
		m_event = ::OpenEvent(EVENT_ALL_ACCESS,TRUE,lpName); //<- WaitForSingleObjectに使われる
		

		if(!m_event){
			if(_g_logger_){
				_g_logger_->LogMessage(kDebug,"OpenEvent %s null \n",lpName);
				_g_logger_->FlushLog();
			}
			SECURITY_ATTRIBUTES secAttribute;
		
			if(!setSecurityAttNullACL(secAttribute)){
				return 0;
			}
			m_event = CreateEvent(&secAttribute,bManualReset,bInitialState,lpName);
		}else{
			if(_g_logger_){
				_g_logger_->LogMessage(kDebug,"OpenEvent %s OK \n",lpName);
				_g_logger_->FlushLog();
			}
		}
#else
		SECURITY_ATTRIBUTES secAttribute;
		
		if(!setSecurityAttNullACL(secAttribute)){
			return 0;
		}
		/*
		*  既に同じ名前のEventがCreateされていても、同一ものとしてCreateできる
		*/
		m_event = CreateEvent(&secAttribute,bManualReset,bInitialState,lpName);
		if(!m_event){
			//ここに来ない
			if(_g_logger_){
				_g_logger_->LogMessage(kDebug,"MyCreateEnvet CreateEvent %s is null  -> OpenEvent\n",lpName);
				_g_logger_->FlushLog();
			}

			m_event = ::OpenEvent(EVENT_ALL_ACCESS,TRUE,lpName);//<- WaitForSingleObjectに使われる
		
		}
		if(!m_event){
			if(_g_logger_){
				_g_logger_->LogMessage(kErrorOnly," MyCreateEnvet [ %s ] is NULL  \n",lpName);
				_g_logger_->FlushLog();
			}
		}else{
		 
			if(_g_logger_){
				_g_logger_->LogMessage(kDebug," MyCreateEnvet [ %s ] = 0x%x  \n",lpName,m_event);
				_g_logger_->FlushLog();
			}
		}
		 

#endif
		return m_event;

	}
	 
protected:
	HANDLE m_event;
};

///////////////////////////////
class CMySecurityMutex : public CSetSecurityAttNullACL
{
public:
	enum { kError = -1, kOK = 0, kTimeOut};
	CMySecurityMutex(const char* iName, int iAutoLock=0)
	{
		DWORD err = 0;

		SECURITY_ATTRIBUTES secAttribute;
		
		if(!setSecurityAttNullACL(secAttribute)){
			if(_g_logger_){
				_g_logger_->LogMessage(kErrorOnly," CMySecurityMutex setSecurityAttNullACL [%s] error  \n",iName);
				_g_logger_->FlushLog();
			} ;
		}

		m_mutex = ::CreateMutex(&secAttribute, iAutoLock, iName);

		if (m_mutex == 0)
		{
			err = ::GetLastError();
			if(ERROR_ACCESS_DENIED == err)
			{
				m_mutex = ::OpenMutex(MUTEX_ALL_ACCESS, TRUE, iName);
			}
		}
	 
		if (m_mutex == 0)
		{
			if(_g_logger_){
				_g_logger_->LogMessage(kErrorOnly," CMySecurityMutex Mutex [%s] is NUL \n",iName);
				_g_logger_->FlushLog();
			} ;
		}
 
	}

	virtual ~CMySecurityMutex(void)
	{
		if (m_mutex)
		{
			CloseHandle(m_mutex);
		}

		m_mutex = 0;
	}

	/* Request the exclusive lock. iMilliSec specifies
	 * how long to wait before giving up. 0 mean infinity
	 * Check function return for the status
	 */
	int		Acquire(long iMilliSec=0) const
	{
		if (!m_mutex)
		return kError;
		
		if (iMilliSec <= 0)
			iMilliSec= INFINITE;

		DWORD status = WaitForSingleObject(m_mutex,iMilliSec);
		return (status == WAIT_OBJECT_0) ? kOK:(status==WAIT_TIMEOUT ?kTimeOut:kError);
	}

	/* release lock */
	int		Release(void) const
	{
		 
		return ReleaseMutex(m_mutex) ? kOK:kError;
	}
private:
 

	HANDLE	m_mutex;
};

///////////////////////////////
const char *g_all_queue_field =  
//		"QueueID,"  // no ID
		"JobID"
		",cmdID"
		",SendLevel"
		",StudyInstanceUID"
 		",SeriesInstanceUID"
		",SOPInstanceUID"
		",DestinationAE"
		",Priority"
		",Status "
		",RetryCount "
		",CreateTime"
		",AccessTime" ;
const char *g_all_queue_field_ext =  
//		"QueueID,"  // no ID
		"JobID"
		",cmdID"
		",SendLevel"
		",StudyInstanceUID"
 		",SeriesInstanceUID"
		",SOPInstanceUID"
		",DestinationAE"
		",Priority"
		",Status "
		",RetryCount "
		",CreateTime"
		",AccessTime"
		",PatientName " 
		",PatientID "
		",Date "
		",BirthDate "				//BirthDate // 0212/06/15 added K.Ko
		",SeriesNumber "
		",EntryImages "
		",ImagesInSeries "
		",Comment ";


char *msg_lel_tbl [] = {
	"kErrorOnly", 
	"kWarning", 
	"kInfo", 
	"kDebug", 
	"kTrace"
};

#define TimeStrFormat "%04d/%02d/%02d %02d:%02d:%02d"



//////////////////////////
CPxQueueBinFile::CPxQueueBinFile()
{
	m_header.id_code = 0x1234;
}
CPxQueueBinFile::~CPxQueueBinFile()
{
}
bool CPxQueueBinFile::writeQueueBinFile(const std::string &fileName)
{
	return writeQueueBinFileEx(fileName,m_seriesFolder,m_imageFileList);
}
bool CPxQueueBinFile::writeQueueBinFileEx(const std::string &fileName,const std::string &serisFolder,const std::vector<std::string> &list)
{
	bool ret_b = true;

	strncpy(m_header.seriesFolder,serisFolder.c_str(),1024);
	m_header.seriesNumber = list.size();
	try{
		FILE *fp = fopen(fileName.c_str(),"wb");
		if(!fp){
			return false;
		}
		int write_len = sizeof(QueueEntryBinHeader);
		int nn = fwrite(&m_header,1,write_len,fp);
		if(nn == write_len){
			if(m_header.seriesNumber>0){
				write_len = ImageFileNameLen*m_header.seriesNumber;
				char *entry_buff = new char[write_len];
				for(int i=0;i<m_header.seriesNumber;i++){
					strcpy(entry_buff+i*ImageFileNameLen, list[i].c_str());
				}
				nn = fwrite(entry_buff,1,write_len,fp);
				if(nn != write_len){
					ret_b = false;
				}
				delete [] entry_buff;
			}
		}else{
			ret_b = false;
		}
		fclose(fp);
	}catch(...){
		ret_b = false;
	}
	return ret_b;
}
bool CPxQueueBinFile::readQueueBinFile(const std::string &fileName)
{
	return readQueueBinFileEx(fileName,m_seriesFolder,m_imageFileList);
}
bool CPxQueueBinFile::readQueueBinFileEx(const std::string &fileName,std::string &serisFolder,std::vector<std::string> &list)
{
//////
	bool ret_b = true;
	try {
		FILE *fp = fopen(fileName.c_str(),"rb");
		if(!fp){
			return false;
		}
		int read_len = sizeof(QueueEntryBinHeader);
		int nn = fread(&m_header,1,read_len,fp);
		
		if(nn == read_len){
			serisFolder = m_header.seriesFolder;
			if(m_header.seriesNumber>0){
				read_len = ImageFileNameLen*m_header.seriesNumber;
				char *entry_buff = new char[read_len];
				nn = fread(entry_buff,1,read_len,fp);
				if(nn != read_len){
					ret_b = false;
					 
				}else{
					list.clear();
					list.resize(m_header.seriesNumber);
					for(int i=0;i<m_header.seriesNumber;i++){
						list[i] =  entry_buff+i*ImageFileNameLen ;
					}
				}
				delete [] entry_buff;
				
			}
		}else{
			ret_b = false;
		}
		fclose(fp);
	}catch(...){
		ret_b = false;
	}
	return ret_b;
}
 
///////////
const char *p_GLOBAL_SEND_QUEUE_EVENT_NAME = "Global\\Px_SendQueuEvent_xx" ;
const char *p_SEND_QUEUE_EVENT_NAME = "Px_SendQueuEvent_xx" ;

void* CPxQueueProc::m_sendQueuEvent = 0 ;
bool CPxQueueProc::m_globalInited = false;
//
CMySecurityMutex *CPxQueueProc::m_mutex_sqliteDB_send = 0;
CMySecurityMutex *CPxQueueProc::m_mutex_sqliteDB_result = 0;
////////
CMySecurityEvent _my_create_event_; //<- instance 必要
void CPxQueueProc::Global_init(bool global)
{
	if(m_globalInited) return;

//	m_sendQueuEvent = ::CreateEvent(0 , TRUE, FALSE,p_SEND_QUEUE_EVENT_NAME);//p_SEND_QUEUE_EVENT_NAME);

	const char *name_str = global ? p_GLOBAL_SEND_QUEUE_EVENT_NAME : p_SEND_QUEUE_EVENT_NAME;
	m_sendQueuEvent = _my_create_event_.MyCreateEnvet(TRUE, FALSE,name_str);
#if 0
	if(!m_sendQueuEvent){
		//既に存在している
		m_sendQueuEvent = ::OpenEvent(SYNCHRONIZE,TRUE,name_str);
	}
#endif
	m_globalInited = (m_sendQueuEvent!= 0);
	//
	
#ifndef USE_MSSQL_SERVER_QUEUE 
 
	char _mutex_name_buf[256];
	if(global){
		sprintf(_mutex_name_buf,"Global\\DB_Mutex_%s_%s_",SQLITE_DB_FILTE,"sendQueue");
 		sprintf(_mutex_name_buf,"Global\\DB_Mutex_%s_%s_",SQLITE_DB_FILTE,"resultQueue");
	}else{
		sprintf(_mutex_name_buf,"DB_Mutex_%s_%s_",SQLITE_DB_FILTE,"sendQueue");
 		sprintf(_mutex_name_buf,"DB_Mutex_%s_%s_",SQLITE_DB_FILTE,"resultQueue");
	}
 
	m_mutex_sqliteDB_send	= new CMySecurityMutex(_mutex_name_buf);
	m_mutex_sqliteDB_result = new CMySecurityMutex(_mutex_name_buf);
#endif
	 
}
void CPxQueueProc::setLogger(TRLogger *logger)
{
	_g_logger_ = logger;
}
std::string getTimeStringFromTM(const struct tm &tm_temp)
{
	char _str_buf[64];
 
	sprintf(_str_buf, 
			TimeStrFormat,
			tm_temp.tm_year + 1900,
			tm_temp.tm_mon + 1,
			tm_temp.tm_mday,
			tm_temp.tm_hour,
			tm_temp.tm_min,
			tm_temp.tm_sec
			);
	return _str_buf;
}
//
time_t  getTimeFromString( const char *time_str)
{
	 
	if(!time_str) return -1;
	if(strlen(time_str)<18) return -1;
	 
	struct tm tm_temp;
	memset(&tm_temp,0,sizeof(struct tm));
	sscanf(time_str, 
			TimeStrFormat,
			&(tm_temp.tm_year),
			&(tm_temp.tm_mon),
			&(tm_temp.tm_mday),
			&(tm_temp.tm_hour),
			&(tm_temp.tm_min),
			&(tm_temp.tm_sec)
			);
	//#26 2012/06/12
	tm_temp.tm_isdst= -1;      /* 夏時間無効 */

	tm_temp.tm_year -= 1900;
	tm_temp.tm_mon -= 1;
	return mktime(&tm_temp);
}


void CPxQueueProc::LogMessage(int iLevel, const char *fmt, ...)
{
	char str_buff[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(str_buff, fmt, args);
	va_end(args);

	if(_g_logger_){
		_g_logger_->LogMessage(iLevel,str_buff);
		_g_logger_->FlushLog();
	}else{
		printf("[%s] %s \n",msg_lel_tbl[iLevel],str_buff);
	}
}

CPxQueueEntry::CPxQueueEntry(void)
{
	m_JobID = " ";
	m_cmdID = PxQueueCmd_DICOM  ;//#48
	m_QueueID = -1;//
	m_SendLevel = 0;// 0
 	m_StudyInstanceUID	= " "; 
 	m_SeriesInstanceUID = " ";  
	m_SOPInstanceUID	= " ";  
 	m_DestinationAE		= " ";  
	m_Priority = 0;
	m_Status = 0;
	m_RetryCount = 0;
	m_CreateTime = time(0);
	m_AccessTime = time(0);
	//#142_NoPatientName_NoComment
	//m_extInfo.m_PatientName	= " "; //VARCHAR(64)   
	m_extInfo.m_PatientID	= " "; //VARCHAR(16)
	m_extInfo.m_Date		= " "; //VARCHAR(64)  study date or series date time
	m_extInfo.m_BirthDate	= " "; // 0212/06/15 added K.Ko
	m_extInfo.m_SeriesNumber = 0;
	m_extInfo.m_EntryImages = 0; //used by block ( PxQueuelevel_EntryFile)
	m_extInfo.m_ImagesInSeries = 0;
	//#142_NoPatientName_NoComment
	//m_extInfo.m_Comment	= " ";//VARCHAR(64)  study description or series description
};
CPxQueueEntry::~CPxQueueEntry(void)
{
};
bool CPxQueueEntry::operator ==( const CPxQueueEntry &entry2) const
{
	bool ret_b =
		(m_JobID				== entry2.m_JobID)&&
		(m_cmdID				== entry2.m_cmdID)	&&
		(m_QueueID				== entry2.m_QueueID)	&&
		(m_SendLevel			== entry2.m_SendLevel)	&&
		(m_StudyInstanceUID		== entry2.m_StudyInstanceUID)	&&
		(m_SeriesInstanceUID	== entry2.m_SeriesInstanceUID)	&&
		(m_SOPInstanceUID		== entry2.m_SOPInstanceUID)	&&
		(m_DestinationAE		== entry2.m_DestinationAE)	&&
		(m_Priority				== entry2.m_Priority)	&&
		(m_Status				== entry2.m_Status)	&&
		(m_RetryCount			== entry2.m_RetryCount)	&&
		(m_CreateTime			== entry2.m_CreateTime)	&&
		(m_AccessTime			== entry2.m_AccessTime)	 ;
	bool ext_b =
	//#142_NoPatientName_NoComment
		//(m_extInfo.m_PatientName	== entry2.m_extInfo.m_PatientName)	&&
		(m_extInfo.m_PatientID == entry2.m_extInfo.m_PatientID) &&
		(m_extInfo.m_Date == entry2.m_extInfo.m_Date) &&
		(m_extInfo.m_BirthDate == entry2.m_extInfo.m_BirthDate) && // 0212/06/15 added K.Ko
		(m_extInfo.m_SeriesNumber == entry2.m_extInfo.m_SeriesNumber) &&
		(m_extInfo.m_EntryImages == entry2.m_extInfo.m_EntryImages) &&
		(m_extInfo.m_ImagesInSeries == entry2.m_extInfo.m_ImagesInSeries);// &&
	//#142_NoPatientName_NoComment
	//	(m_extInfo.m_Comment		== entry2.m_extInfo.m_Comment);


	  
	return ret_b && ext_b;
}
inline void readEntryFromSqa(SQA &sqa,CPxQueueEntry &entry)
{
	char _str_buff[256];
	
	
	SQL_GET_INT(entry.m_QueueID, sqa);
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_JobID = _str_buff;
	//
	SQL_GET_INT(entry.m_cmdID, sqa);
	//
	SQL_GET_INT(entry.m_SendLevel, sqa);
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_StudyInstanceUID = _str_buff;
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_SeriesInstanceUID = _str_buff;
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_SOPInstanceUID = _str_buff;
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_DestinationAE = _str_buff;
		 
	//
	SQL_GET_INT(entry.m_Priority, sqa);
	SQL_GET_INT(entry.m_Status, sqa);
	SQL_GET_INT(entry.m_RetryCount, sqa);
	//
#ifdef USE_MSSQL_SERVER_QUEUE
	entry.m_CreateTime = VariantTimeToTime_t(sqa.getDataDate());
	entry.m_AccessTime = VariantTimeToTime_t(sqa.getDataDate());
#else
	SQL_GET_STR(_str_buff, sqa);
	entry.m_CreateTime = getTimeFromString(_str_buff);
	SQL_GET_STR(_str_buff, sqa);
	entry.m_AccessTime = getTimeFromString(_str_buff);
#endif

}

inline void readEntryFromSqa_ext(SQA &sqa,CPxQueueEntry &entry)
{
	char _str_buff[256];
	
	
	SQL_GET_INT(entry.m_QueueID, sqa);
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_JobID = _str_buff;
	//
	SQL_GET_INT(entry.m_cmdID, sqa);
	//
	SQL_GET_INT(entry.m_SendLevel, sqa);
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_StudyInstanceUID = _str_buff;
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_SeriesInstanceUID = _str_buff;
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_SOPInstanceUID = _str_buff;
	//
	SQL_GET_STR(_str_buff, sqa);
	entry.m_DestinationAE = _str_buff;
		 
	//
	SQL_GET_INT(entry.m_Priority, sqa);
	SQL_GET_INT(entry.m_Status, sqa);
	SQL_GET_INT(entry.m_RetryCount, sqa);
	//
#ifdef USE_MSSQL_SERVER_QUEUE
	entry.m_CreateTime = VariantTimeToTime_t(sqa.getDataDate());
	entry.m_AccessTime = VariantTimeToTime_t(sqa.getDataDate());
#else
	SQL_GET_STR(_str_buff, sqa);
	entry.m_CreateTime = getTimeFromString(_str_buff);
	SQL_GET_STR(_str_buff, sqa);
	entry.m_AccessTime = getTimeFromString(_str_buff);
#endif
////////////////
	// extInfo
	SQL_GET_STR(_str_buff, sqa);
	//entry.m_extInfo.m_PatientName	= _str_buff;//#142_NoPatientName_NoComment
	SQL_GET_STR(_str_buff, sqa);
	entry.m_extInfo.m_PatientID		= _str_buff;
	SQL_GET_STR(_str_buff, sqa);
	entry.m_extInfo.m_Date			= _str_buff;
	SQL_GET_STR(_str_buff, sqa);
	entry.m_extInfo.m_BirthDate		= _str_buff; //0212/06/15 added K.Ko
	SQL_GET_INT(entry.m_extInfo.m_SeriesNumber, sqa);
	SQL_GET_INT(entry.m_extInfo.m_EntryImages, sqa);
	SQL_GET_INT(entry.m_extInfo.m_ImagesInSeries, sqa);
	SQL_GET_STR(_str_buff, sqa);
	//	entry.m_extInfo.m_Comment		= _str_buff;//#142_NoPatientName_NoComment

}

///////////////////////////
std::string CPxQueueProc::m_DBName;
std::string CPxQueueProc::m_EntryFolder;

CPxQueueProc::CPxQueueProc(void):
m_needQueuEvent(true),
m_breakFlag(false),
m_initFlag(false),
m_runCount(0)
{

	m_dbEexRetryNN = 10; //See Also _SQADataSQLite::m_retryNN

	m_queue_db = new CPxDB ;
 
	m_mutex_sqliteDB = 0;

}

CPxQueueProc::~CPxQueueProc(void)
{


	delete m_queue_db;
}

void CPxQueueProc::initDB()
{
//	m_queue_db->SetMyDBName(L"PxQueueDB");
	AqUString dbname_str;
	dbname_str.Format(L"%S",m_DBName.c_str());
	m_queue_db->SetMyDBName(dbname_str);
}
bool CPxQueueProc::readOneEntry(int id,CPxQueueEntry &entry)
{


	AqString topNStr = "";
	AqString	strSQL;
 
	strSQL.Format( "SELECT "
		"QueueID,"
		"%s"
		" FROM %s"
		" WHERE QueueID = %d",
		 
	//	g_all_queue_field,
		g_all_queue_field_ext,
		getQueueTableName(), 
		id
		);

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);
	int retcd = m_queue_db->SQLExecuteBegin(sqa);
	 
	bool ret_b = true;
	if(retcd != kOK){
		ret_b = false;
		LogMessage(kErrorOnly," SQLExecuteBegin failed");
		 
	}else{
		int size = sqa.GetRecordCount(); 
		if(size < 1) {
			ret_b = false;
		}else{
			retcd = sqa.MoveFirst(); 
			if(retcd != kOK) {
				ret_b = false;
				LogMessage(kErrorOnly," MoveFirst failed");
				 
			}else{
		//		readEntryFromSqa(sqa,entry);
				readEntryFromSqa_ext(sqa,entry);
			}
		}

	}
	m_queue_db->SQLExecuteEnd(sqa);	
	
	return ret_b ;
	 
}
bool CPxQueueProc::readQueueSimpleEntry(std::vector<CPxQueueEntry> &queueTemp,int priority,int status1,int status2)
{
	return readQueue_in(queueTemp,false,priority, status1, status2);
}
bool CPxQueueProc::readQueueExt(int priority,int status1,int status2)
{
	return readQueue_in(m_queueFullEntry,true,priority, status1, status2);
}
bool CPxQueueProc::readQueue_in(std::vector<CPxQueueEntry> &queueTemp,bool extInfo,int priority,int status1,int status2)
{
	LogMessage(kDebug,"CPxQueueProc::readQueue_in -- start \n");

	char _str_buff[256];
	

	AqString topNStr = "";
	AqString	strSQL;
 
	 
	strSQL.Format( "SELECT "
		"QueueID,"
		"%s"
		" FROM %s",
		extInfo ? g_all_queue_field_ext : g_all_queue_field,
		getQueueTableName()
		);
	AqString	whereFilter;

	if(priority == CPxQueueEntry::PxQueuePriority_All){
		if(status1 == CPxQueueEntry::PxQueueStatus_Unknown){
			whereFilter.Format(
				" ORDER BY AccessTime ASC"
				);
		}else{
			if(status2 == CPxQueueEntry::PxQueueStatus_Unknown){
				whereFilter.Format(
				" WHERE Status = %d"
				" ORDER BY AccessTime ASC",
				status1);
			}else{
				whereFilter.Format(
				" WHERE "
				"(Status = %d  OR Status = %d)"
				" ORDER BY AccessTime ASC",
				status1,status2);
			}
		}
	}else{
		if(status1 == CPxQueueEntry::PxQueueStatus_Unknown){
			whereFilter.Format(
				"WHERE Priority = %d"
				" ORDER BY AccessTime ASC",
				priority);
		}else{
			if(status2 == CPxQueueEntry::PxQueueStatus_Unknown){
				whereFilter.Format(
				" WHERE Status = %d"
				" AND Priority = %d"
				" ORDER BY AccessTime ASC",
				status1,priority);
			}else{
				whereFilter.Format(
				" WHERE "
				"(Status = %d  OR Status = %d)"
				" AND Priority = %d"
				" ORDER BY AccessTime ASC",
				status1,status2,priority);
			}
		}
	}
		
	strSQL += whereFilter;
	 

 
	return ExeReadQueue(queueTemp ,extInfo,strSQL);
 

}
bool CPxQueueProc::readQueueAll()
{
	return readQueueAll_in(m_queueFullEntry,true);
}
bool CPxQueueProc::readQueueAll_in(std::vector<CPxQueueEntry> &queueTemp ,bool extInfo)
{
	LogMessage(kDebug,"CPxQueueProc::readQueueAll_in -- start \n");

	char _str_buff[256];
	

	AqString topNStr = "";
	AqString	strSQL;
 
	 
	strSQL.Format( "SELECT "
		"QueueID,"
		"%s"
		" FROM %s",
		extInfo ? g_all_queue_field_ext : g_all_queue_field,
		getQueueTableName()
		);
	AqString	whereFilter;

	whereFilter.Format(
		" WHERE Status <> %d"
		" ORDER BY AccessTime ASC",
		CPxQueueEntry::PxQueueStatus_Empty);
 	
	strSQL += whereFilter;

	bool ret_b = false;
	for(int try_i=0;try_i<m_dbEexRetryNN;try_i++){
		ret_b = ExeReadQueue(queueTemp ,extInfo,strSQL);
		if(ret_b) break;
	}
	return ret_b;
	 
}
bool CPxQueueProc::ExeReadQueue(std::vector<CPxQueueEntry> &queueTemp,bool extInfo,const char *strSQL)
{
	queueTemp.clear();
	
	LogMessage(kDebug,"CPxQueueProc::readQueueAll_in DBType[%d],sql[%s] \n",m_queue_db->getDBType(),strSQL);
	 

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);
	bool ret_b = true;

	try {
		int retcd = m_queue_db->SQLExecuteBegin(sqa);
		 
		
		if(retcd != kOK){
			ret_b = false;
			LogMessage(kErrorOnly," SQLExecuteBegin failed");
		}else{
			int size = sqa.GetRecordCount(); 

			LogMessage(kDebug,"CPxQueueProc::readQueueAll_in size [%d] \n",size);
			if(size < 1) {
				
			}else{
				queueTemp.resize(size);


				int index = 0;
				retcd = sqa.MoveFirst(); 
				if(retcd != kOK) {
					ret_b = false;
					LogMessage(kErrorOnly," MoveFirst failed");
					 
				}else{
					while( retcd == kOK && index < size )
					{
	 
						if(extInfo){
							readEntryFromSqa_ext(sqa,queueTemp[index]);
						}else{
							readEntryFromSqa(sqa,queueTemp[index]);
						}
	 
						index++;
						retcd = sqa.MoveNext();  
					}
				}
			}
		}

		m_queue_db->SQLExecuteEnd(sqa);	
	}catch(...){
		LogMessage(kErrorOnly,"readQueueAll_in::readQueue error");
		ret_b = false;
	 
	}
	LogMessage(kDebug,"CPxQueueProc::readQueueAll_in -- end [%d] \n",ret_b);
	 

	return ret_b ;
}
 bool CPxQueueProc::getRecycleIDs(std::vector<int> &recyceIDs)
{
	recyceIDs.clear();

	 

//	m_queue_db->SetMyDBName(L"PxQueueDB");
	AqString strSQL;
	strSQL.Format("Select QueueID from %s WHERE Status=%d", 
		getQueueTableName(),
		CPxQueueEntry::PxQueueStatus_Empty);
	 
	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);
	int retcd = m_queue_db->SQLExecuteBegin(sqa);
	 
	bool ret_b = true;
	if(retcd != kOK){
		LogMessage(kErrorOnly," SQLExecuteBegin failed");
		ret_b = false;
	}
	if(ret_b){
		int size = sqa.GetRecordCount(); 
		if(size < 1) {
		}else{
			recyceIDs.resize(size);

			int index = 0;
			retcd = sqa.MoveFirst(); 
			if(retcd != kOK) {
				ret_b = false;
				LogMessage(kErrorOnly," MoveFirst failed");
				 
			}else{
				while( retcd == kOK && index < size )
				{
					SQL_GET_INT(recyceIDs[index] , sqa);
					index++;
					retcd = sqa.MoveNext();  
				}
			}
		}
	}

	m_queue_db->SQLExecuteEnd(sqa);
	return ret_b;
}
bool CPxQueueProc::addQueue(const CPxQueueEntry &entry)
{
	return addEntryToQueue( entry,CPxQueueEntry::PxQueueStatus_Standby);
}
bool CPxQueueProc::addEntryToQueue(const CPxQueueEntry &entry,int newStatus)
{
//	CPxQueueProc::CLockQueue lock(this);//2012/06/21

	
	std::vector<int> recyceIDs;

	if(!getRecycleIDs(recyceIDs)){

	}


	CPxQueueEntry entry_temp = entry;

	if(newStatus!=CPxQueueEntry::PxQueueStatus_Unknown){
		entry_temp.m_Status = newStatus;//CPxQueueEntry::PxQueueStatus_Standby;
	}
	checkPriority(entry_temp);

	bool ret_b = false;
	if(recyceIDs.size()>0)
	{
		//recycel the empty queue
		
		for(int index=0;index<recyceIDs.size();index++){
			if(updateQueue( recyceIDs[index],  entry_temp, CPxQueueEntry::PxQueueStatus_Empty)){
				ret_b = true;
				break;
			}else{
				//try next empty ID
			}
		}
	}
	if(!ret_b){
		//none recycleID
		ret_b =  addNewQueue(entry_temp) ;
	} 
	if(ret_b){
		if(m_needQueuEvent){ //2012/06/21
			if(m_sendQueuEvent) {
				::SetEvent(m_sendQueuEvent);
			}
		}
		
	}

	return ret_b;
}

bool CPxQueueProc::addNewQueue(const CPxQueueEntry &entry)
{
	
//	char _CreateTimeBuf[128];
//	char _AccessTimeBuf[128];



	AqString	strSQL;
 

	;

	time_t t_temp = entry.m_CreateTime;
	struct tm tm_temp = *localtime(&t_temp);
	std::string CreateTimeStr = getTimeStringFromTM(tm_temp);
	 
 //
	t_temp = entry.m_AccessTime;
	tm_temp = *localtime(&t_temp);
	std::string AccessTimeStr = getTimeStringFromTM(tm_temp);
	 

	strSQL.Format( "INSERT INTO %s "

		"("
 
		"%s"
 

		")"
		///////////////////////
		" VALUES"
		///////////////////////
		"("
		"'%s'"		//JobID
		",%d"		//cmdID
		",%d"		//SendLevel
		",'%s'"		//StudyInstanceUID
 		",'%s'"		//SeriesInstanceUID
		",'%s'"		//SOPInstanceUID
		",'%s'"		//DestinationAE
		",%d"		//Priority
		",%d"		//Status
		",%d"		//RetryCount
		",'%s'"		//CreateTime
		",'%s'"		//AccessTime
	////////////////
	// extInfo
		",'%s'"		//PatientName
		",'%s'"		//PatientID
		",'%s'"		//Date
		",'%s'"		//BirthDate // 0212/06/15 added K.Ko
		",%d"		//SeriesNumber
		",%d"		//EntryImages
		",%d"		//ImagesInSeries
		",'%s'"		//Comment
		")"
		///////////////////////
		,getQueueTableName()
	//	,g_all_queue_field
		,g_all_queue_field_ext
		,entry.m_JobID.c_str()
		,entry.m_cmdID
		,entry.m_SendLevel
		,(entry.m_StudyInstanceUID.size()<1) ?	" " : entry.m_StudyInstanceUID.c_str()
 		,(entry.m_SeriesInstanceUID.size()<1) ? " " : entry.m_SeriesInstanceUID.c_str()
		,(entry.m_SOPInstanceUID.size()<1) ?	" " : entry.m_SOPInstanceUID.c_str()
		,entry.m_DestinationAE.c_str()
		,entry.m_Priority
		,entry.m_Status
		,entry.m_RetryCount
	 	,CreateTimeStr.c_str()
		,AccessTimeStr.c_str()
//		,"2012/04/06 12:10:11"
//		,"2012/04/06 12:10:11"
////////////////
	// extInfo
	//#142_NoPatientName_NoComment
	//	,(entry.m_extInfo.m_PatientName.size()<1) ?	" " : entry.m_extInfo.m_PatientName.c_str()
	,	" "
		,(entry.m_extInfo.m_PatientID.size()<1) ?	" " : entry.m_extInfo.m_PatientID.c_str()
		,(entry.m_extInfo.m_Date.size()<1) ?		" " : entry.m_extInfo.m_Date.c_str()
		,(entry.m_extInfo.m_BirthDate.size()<1) ?	" " : entry.m_extInfo.m_BirthDate.c_str() // 0212/06/15 added K.Ko
		,entry.m_extInfo.m_SeriesNumber
		,entry.m_extInfo.m_EntryImages
		,entry.m_extInfo.m_ImagesInSeries
	//#142_NoPatientName_NoComment
	//	,(entry.m_extInfo.m_Comment.size()<1) ?		" " : entry.m_extInfo.m_Comment.c_str()
		," "
		);

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);

	int  retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);
	 

	return (retcd == kOK) ;
}

bool CPxQueueProc::updateQueue(int id, const CPxQueueEntry &entry,int checkShareStatus)
{
//	char _CreateTimeBuf[128];
//	char _AccessTimeBuf[128];
	 



	AqString	strSQL;

	AqString	whereFilter;
 
	if(checkShareStatus != CPxQueueEntry::PxQueueStatus_Unknown){
		/*
		* 一旦 Status -> PxQueueStatus_Processingにセットする。
		*/
		if(!changeStatus(id,
					CPxQueueEntry::PxQueueStatus_Processing/* new status*/,
					checkShareStatus/* current*/)){
		//			CPxQueueEntry::PxQueueStatus_Empty/* current*/)){
			//競合発生かもしれない。
				return false;
		}
	}


	time_t t_temp = entry.m_CreateTime;
	struct tm tm_temp = *localtime(&t_temp);
	std::string CreateTimeStr = getTimeStringFromTM(tm_temp);
	 
	 
 //
	t_temp = entry.m_AccessTime;
	tm_temp = *localtime(&t_temp);
	std::string AccessTimeStr = getTimeStringFromTM(tm_temp);
 
	 
	strSQL.Format( "UPDATE %s SET "

		"JobID = '%s'"					//JobID
		",cmdID = %d"					//cmdID
		",SendLevel = %d"				//SendLevel
		",StudyInstanceUID = '%s'"		//StudyInstanceUID
 		",SeriesInstanceUID = '%s'"		//SeriesInstanceUID
		",SOPInstanceUID = '%s'"		//SOPInstanceUID
		",DestinationAE = '%s'"			//DestinationAE
		",Priority = %d"				//Priority
		",Status = %d"					//Status
		",RetryCount = %d"				//RetryCount
		",CreateTime = '%s'"			//CreateTime
		",AccessTime = '%s'"			//AccessTime
	////////////////
	// extInfo
		",PatientName = '%s'"			//PatientName
		",PatientID = '%s'"				//PatientID
		",Date = '%s'"					//Date
		",BirthDate = '%s'"				//BirthDate // 0212/06/15 added K.Ko
		",SeriesNumber = %d"			//SeriesNumber
		",EntryImages = %d"				//EntryImages
		",ImagesInSeries = %d"			//ImagesInSeries
		",Comment = '%s'"				//Comment
		 
		 
		 
		///////////////////////
		,getQueueTableName()
		,entry.m_JobID.c_str()
		,entry.m_cmdID
		,entry.m_SendLevel
		,(entry.m_StudyInstanceUID.size()<1) ?	"" : entry.m_StudyInstanceUID.c_str()
 		,(entry.m_SeriesInstanceUID.size()<1) ? "" : entry.m_SeriesInstanceUID.c_str()
		,(entry.m_SOPInstanceUID.size()<1) ?	"" : entry.m_SOPInstanceUID.c_str()
		,entry.m_DestinationAE.c_str()
		,entry.m_Priority
		,entry.m_Status
		,entry.m_RetryCount
	 	,CreateTimeStr.c_str()
		,AccessTimeStr.c_str()
//		,"2012/04/06 12:10:11"
//		,"2012/04/06 12:10:11"
		////////////////////////////
	////////////////
	// extInfo
	//#142_NoPatientName_NoComment
	//	,(entry.m_extInfo.m_PatientName.size()<1) ?	" " : entry.m_extInfo.m_PatientName.c_str()
		, " "
		,(entry.m_extInfo.m_PatientID.size()<1) ?	" " : entry.m_extInfo.m_PatientID.c_str()
		,(entry.m_extInfo.m_Date.size()<1) ?		" " : entry.m_extInfo.m_Date.c_str()
		,(entry.m_extInfo.m_BirthDate.size()<1) ?	" " : entry.m_extInfo.m_BirthDate.c_str() // 0212/06/15 added K.Ko
		,entry.m_extInfo.m_SeriesNumber
		,entry.m_extInfo.m_EntryImages
		,entry.m_extInfo.m_ImagesInSeries
	//#142_NoPatientName_NoComment
	//	,(entry.m_extInfo.m_Comment.size()<1) ?		" " : entry.m_extInfo.m_Comment.c_str()
		, " "

		);

	
		whereFilter.Format( 
			///////////////////////
			" WHERE "
			///////////////////////
			" QueueID = %d"
			,id
			);
	

	strSQL += whereFilter;

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);

	int retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);
	 

	if( retcd != kOK) {
		return false;
	}


	
	return true ;
}

bool CPxQueueProc::getStatus(int id, int &status)
{
	
	AqString	strSQL;

 
	strSQL.Format( "SELECT Status FROM %s "
					" WHERE QueueID = %d",
					getQueueTableName(),
					id);

	int retcd = m_queue_db->SQLGetInt(strSQL, status);
	 

	return (retcd == kOK) ;
}

bool CPxQueueProc::recycleQueue(int  minutes ,int hours , int days )
{
#ifndef USE_RECYECLEQUEUE
	return true;
#endif
	
	AqString	strSQL;

 

	time_t time_span = minutes*60 + hours*60*60 + days*24*60*60;
	time_t sel_time = time(0) - time_span;
	struct tm tm_temp = *localtime(&sel_time);
 
	strSQL.Format( "UPDATE %s SET "
					" Status = %d"
//					" WHERE AccessTime < '%04d-%02d-%02d %02d:%02d:%02d' " 
					" WHERE AccessTime < "
					"'%s'"
					" AND Status = %d ",
					getQueueTableName(),
					CPxQueueEntry::PxQueueStatus_Empty,
					///
					getTimeStringFromTM(tm_temp).c_str(),
					///
					CPxQueueEntry::PxQueueStatus_Finished
					);

	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);

	int retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);
	 
	return (retcd == kOK) ;
};

void CPxQueueProc::checkPriority(CPxQueueEntry &entry)
{
	if(entry.m_Priority<CPxQueueEntry::PxQueuePriority_Low){
		entry.m_Priority=CPxQueueEntry::PxQueuePriority_Low ;
	}
	else if(entry.m_Priority>CPxQueueEntry::PxQueuePriority_Default){
		entry.m_Priority=CPxQueueEntry::PxQueuePriority_Default ;
	}
}
 
bool CPxQueueProc::changeStatus(int id,int toStatus,int fromStatus)
{

	AqString	strSQL;

	AqString	strSQL_Update_Status;
	{
		time_t cur_time = time(0);
		struct tm tm_cur_temp = *localtime(&cur_time);

		strSQL_Update_Status.Format(
	//					"BEGIN TRANSACTION;" ///<- use TRANSACTION
						"UPDATE %s SET "
						" Status = %d"
#ifdef USE_MSSQL_SERVER_QUEUE
						",AccessTime = GETDATE() "
#else
						",AccessTime = "
						"'%s'"
#endif
						" WHERE QueueID = %d"
//						"commit;"			///<- use TRANSACTION
						,
						getQueueTableName(),
						toStatus,
						//////////
						getTimeStringFromTM(tm_cur_temp).c_str(),
						//////////
						id);
	}

	int successFlag = -1;
	int retcd = -1;
	if(fromStatus==CPxQueueEntry::PxQueueStatus_Unknown){
		 
		SQA sqa(m_queue_db->getDBType());
		sqa.SetCommandText(strSQL_Update_Status);
		 
		retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);

		return (retcd == kOK) ;
	}else{
		/*
		* check the current status, and use Update lock
		*/
		 
		SQA sqa(m_queue_db->getDBType());
 
#ifdef USE_MSSQL_SERVER_QUEUE 
		//use MS SQL
		sqa.SetCommandText(getChgQueueStatusProcName());
		sqa.AddParameter(id,"id");
		sqa.AddParameter(fromStatus ,"CurStatus");
		sqa.AddParameter(toStatus,"NewStatus");
		sqa.AddParameter(successFlag,"returnVal",true/*output*/);
 
		retcd = m_queue_db->SQLExecuteProcBegin(sqa);

		if(retcd == kOK) {

			AqUString retval_str = sqa.GetParamtersString(3);
			printf("exec %s %s \n",getChgQueueStatusProcName(),(const wchar_t*)retval_str);
			successFlag = atoi((const char*)((const wchar_t*)retval_str));
		}
		m_queue_db->SQLExecuteEnd(sqa);
#else
		//lock sqlite3 file
		//
		lockDB();
		try {
 
		successFlag = 0;
		strSQL.Format(	"SELECT Status from %s "
						" WHERE QueueID = %d",
						getQueueTableName(),
						id );
		sqa.SetCommandText(strSQL);

		bool doUpdateFlag = false;
		{
			bool run_success = false;
			for(int try_i=0;try_i<m_dbEexRetryNN;try_i++){
				retcd = m_queue_db->SQLExecuteBegin(sqa);
				//from here , DB will be locked until SQLExecuteEnd

				if(retcd == kOK) {
					if(sqa.GetRecordCount()>0){ 
		 
						int DB_cur_status = -1;
						retcd = sqa.MoveFirst(); 
						if(retcd == kOK){
							SQL_GET_INT(DB_cur_status, sqa);
							if(DB_cur_status == fromStatus){
								doUpdateFlag = true;
							}
							//success!
							run_success = true;
						}
					}

				}
				m_queue_db->SQLExecuteEnd(sqa);
				if(run_success) break;
			}
		}
		if(doUpdateFlag){
			//now do it
			sqa.SetCommandText(strSQL_Update_Status);

			{
				 
				int retcd = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);
				if(retcd == kOK) {
					successFlag = 1;
				}
			}
				 
		}
	
		 
 
		}catch(...)
		{
			successFlag = 0;
		}
		
		//unlock sqlite3 file
		//
		unLockDB();

#endif


		
		return (successFlag == 1) ;
	}

	
}
bool CPxQueueProc::changePriority(int id,int priority,int status, int RetryCount)
{
	AqString	strSQL;
	 
	if( (priority < CPxQueueEntry::PxQueuePriority_Low) ||
		(priority > CPxQueueEntry::PxQueuePriority_Default))
	{
			return false;
	}
	strSQL.Format(
				"UPDATE %s ",
				getQueueTableName());
				 


	AqString	strSQL_temp;
	strSQL_temp.Format("SET Priority = %d ",priority);
	strSQL = strSQL + strSQL_temp;
	//
	if(status != CPxQueueEntry::PxQueueStatus_Unknown){
		strSQL_temp.Format(",Status = %d ",status);
		strSQL = strSQL + strSQL_temp;
	}
	//
	if(RetryCount >= 0){
		strSQL_temp.Format(",RetryCount = %d ",RetryCount);
		strSQL = strSQL + strSQL_temp;
	}
	//
	strSQL_temp.Format(" WHERE QueueID = %d",id);
	strSQL = strSQL + strSQL_temp;
	 
	 
	SQA sqa(m_queue_db->getDBType());
	sqa.SetCommandText(strSQL);
	int retcd  = ExeSQLExecute(sqa,m_queue_db,m_dbEexRetryNN);

	return (retcd == kOK) ;

}

void CPxQueueProc::initRes()
{
	

#ifndef USE_MSSQL_SERVER_QUEUE 
	m_queue_db->setupLocaleDBType(kDBType_SQLite);

	
#endif
	initDB();
	m_initFlag = true;
}
void CPxQueueProc::lockDB()
{
//	return ;
#ifndef USE_MSSQL_SERVER_QUEUE 
	if(m_mutex_sqliteDB){
		m_mutex_sqliteDB->Acquire();
	}
#endif
}
void CPxQueueProc::unLockDB()
{
//	return ;
#ifndef USE_MSSQL_SERVER_QUEUE 
	if(m_mutex_sqliteDB){
		m_mutex_sqliteDB->Release();
	}
#endif
}

static TRCriticalSection _g_uid_key_cs_;
static int _g_uid_key_ = 0;
static int generateID()
{
 
	TRCSLock L(&_g_uid_key_cs_);
	_g_uid_key_++;
	if(_g_uid_key_<1) _g_uid_key_=1;

	return _g_uid_key_;
}

void CPxQueueProc::createJobID(CPxQueueEntry &entry,bool newFlag)
{
	bool do_flag = newFlag || (m_currentJobID.size()<1);
	if(do_flag){
		char _buff_[128];
		if(m_ProcThreadID.size()<1){
			sprintf(_buff_,"%u_%u",::GetCurrentProcessId(),::GetCurrentThreadId());
			m_ProcThreadID = _buff_;
		}
		sprintf(_buff_,"%s_%u",m_ProcThreadID.c_str(),generateID());//m_JobIDCount++);

#if 1
		entry.m_JobID = _buff_;
#else
		entry.m_JobID = entry.m_DestinationAE + _buff_;
		switch(entry.m_SendLevel){
			case CPxQueueEntry::PxQueueLevel_Study:
				entry.m_JobID = entry.m_JobID + entry.m_StudyInstanceUID;
				break;
			case CPxQueueEntry::PxQueueLevel_Series:
			case CPxQueueEntry::PxQueueLevel_Image:
				entry.m_JobID = entry.m_JobID + entry.m_SeriesInstanceUID;
				break;
		}
#endif
		m_currentJobID = entry.m_JobID;
	}else{
		entry.m_JobID = m_currentJobID;
	}
	//
}
std::string CPxQueueProc::genJobUID(const CPxQueueEntry &entry)
{
	std::string str_val ;

	str_val = entry.m_JobID;
	str_val = entry.m_DestinationAE + "_" + str_val;
	switch(entry.m_SendLevel){
		case CPxQueueEntry::PxQueueLevel_Study:
			str_val = str_val + entry.m_StudyInstanceUID;
			break;
		case CPxQueueEntry::PxQueueLevel_Series:
		case CPxQueueEntry::PxQueueLevel_Image:
			str_val = str_val + entry.m_SeriesInstanceUID;
			break;
	}

	return str_val;
}
 
bool CPxQueueProc::getSeriesLevel()
{
	return true;
}
std::string CPxQueueProc::getQueueEntryFileName(const CPxQueueEntry &entry)
{
	std::string fileName = entry.m_DestinationAE
							+std::string("_")
							+entry.m_JobID
							+std::string("_")
							+entry.m_SOPInstanceUID;
	std::string ret_str = m_EntryFolder  + fileName;

	return ret_str;
}
void CPxQueueProc::deleteEntryFile(const CPxQueueEntry &entry)
{
	if(entry.m_SendLevel != CPxQueueEntry::PxQueuelevel_EntryFile){
		return;
	}
	try{
		std::string fileName = getQueueEntryFileName(entry);
		TRPlatform::remove(fileName.c_str());
	}catch(...){
	}

}
bool CPxQueueProc::deleteQueue(int QueueID)
{
	CPxQueueEntry entry;
	if(!readOneEntry(QueueID, entry)){
		return false;
	}

	if(entry.m_SendLevel == CPxQueueEntry::PxQueuelevel_EntryFile){
		deleteEntryFile(entry);
	}
	bool ret_b = changeStatus(QueueID,CPxQueueEntry::PxQueueStatus_Empty);
	return ret_b;
}

#if 0
__time64_t CPxQueueProc::getCurTime() //is time(0);
{
	return time(0);
}
#endif

void CPxQueueProc::selTest()
{

	AqString	strSQL;

	bool ret_b = false;
 	ret_b = changeStatus(2597,1,16);
//	ret_b = changeStatus(2597,16);

	printf(" ret_b %d \n",ret_b );
	 
}
