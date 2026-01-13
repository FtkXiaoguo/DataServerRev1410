/***********************************************************************
 *-------------------------------------------------------------------
 */

#ifndef	__SQA_h__
#define	__SQA_h__



/////////////////
const enum
{
	kDBAsyncExecute =		(1 << 1),
	kDBLockExecute =		(1 << 2),
	kDBExecuteNoRecords =	(1 << 3),
	kDBNoStringRTrim =		(1 << 4),
	kDBNoLogOnIntegrityViolation = (1 << 5),
	kDBNoTransaction =		(1 << 6)
};

const enum DBStatus {
	kOK = 0,
	kNoResult,
	kComInitError,
	kDBOpenError,
	kDBException,
	kDBCancell,
	kParameterError,
	kNoMoreResult,
	kNoDiskSpace,
	kDBOutResource,
	kDBTimeout,
	kFailedUnknown
};


// add SQLite 2011/09/08 K.Ko
const enum DBType
{
	kDBType_Default = 0,
	kDBType_MSSQL  ,
	kDBType_SQLite,
};
////////////////
class SQAParam
{
public :
virtual void Progress(int workRemaining, int workCompleted)=0;
};

class SQAComInf;
class _SQAData;
class  SQA
{
 
public:
	static void setDefaultDbType(int DBType); //2012/05/14
 	SQA( int DBType=kDBType_Default ) ;  

	virtual~SQA() ;
	  void Reset() ;
	  int	GetRecordCount() ;
	  int SetIndex(int iInd) ;
	  int GetIndex() ;
	  int GetFieldCount() ;

	  int GetRecordPostion() ;
	
	  int	MoveFirst() ;
	  int	MoveLast() ;
	  int	MovePrevious() ;
	  int	MoveNext() ;
	  int	Move(long iNumber) ;
    
	  int NextRecordset() ;

	  void	SkipData() ;
	  const char* getDataS(int index=-1) ;
	  const char* getDataUTF8(int index = -1);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8

	  const wchar_t* getDataUS(int index=-1) ;
	  void	setDataS(char* iTarget, int iSize, int index=-1) ;
	  void	setDataUTF8(char* iTarget, int iSize, int index = -1);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
	  void	setDataS(wchar_t* iTarget, int iSize, int index=-1) ;
	
	  int     getDataInt(int index=-1) ;
	  void	getDataInt(char* oNum, int size, int index=-1) ;
	  double	getDataD(int index=-1) ;
	  double  getDataDate(int index=-1) ;
	  int		getDataST(AqUString& oName, AqUString& oValue, int index=-1) ;
	  __int64 getDataInt64(int index=-1) ;
	

	  int SetCommandText(const wchar_t* iCmd) ;
	  int SetCommandText(const char* iCmd) ;
	  int FormatCommandText(const wchar_t* fmt, ...) ;
	  int FormatCommandText(const char* fmt, ...) ;
	  const wchar_t* GetCommandText() ;
	  const wchar_t* GetParamtersString() ;
	  //2012/04/17 K.Ko
	  int GetParamtersCount() ; 
	  AqUString GetParamtersString(int index) ;
	  ////////

	  int AddParameter(const char* iValue, const char* iName=0, bool output=false) ;
	  int AddParameter(const wchar_t* iValue, const char* iName=0, bool output=false) ;

	  int AddParameter(long iValue, const char* iName=0, bool output=false) ;
	  int AddParameter(int iValue, const char* iName=0, bool output=false)  {return AddParameter((long)iValue, iName, output) ;}
	  int AddParameter(char iValue, const char* iName=0, bool output=false) ;


	  int AddParameter(short iValue, const char* iName=0, bool output=false) ;
	  int AddParameter(double iValue, const char* iName=0, bool output=false ) ;
	  int AddParameter(float iValue, const char* iName=0, bool output=false ) ;
	  int AddParameter(__int64 iValue, const char* iName=0, bool output=false) ;


	  int SetParameter(const wchar_t* iValue, long iIdx, const char* iName=0) ;
	  int SetParameter(const char* iValue, long iIdx, const char* iName=0) ;

	  int SetParameter(long iValue, long iIdx, const char* iName=0) ;
	  int SetParameter(int iValue, long iIdx, const char* iName=0) {return SetParameter((long)iValue, iIdx, iName) ;}
	  int SetParameter(char iValue, long iIdx, const char* iName=0) ;
	  int SetParameter(short iValue, long iIdx, const char* iName=0) ;
	  int SetParameter(double iValue, long iIdx, const char* iName=0) ;
	  int SetParameter(float iValue, long iIdx, const char* iName=0) ;
	  int SetParameter(__int64 iValue, long iIdx, const char* iName=0) ;

	  ////
	  int ExecuteBegin(SQAParam * param) ;

	  int ExecuteProcBegin(SQAParam * param) ;//2012/04/17 K.Ko
	  ////


	  _SQAData *getSQAData() ;
	  //
		int	getOptions();
		void setOptions(int opt);
		int getCommandTimeout();
		void setCommandTimeout(int timeout);

protected:
		AqUString	m_cUStr;
		AqString	m_cStr;
		SQAComInf *m_SQComInf;

		int m_DBType;//2012/05/14
} ;


#endif //__SQA_h__