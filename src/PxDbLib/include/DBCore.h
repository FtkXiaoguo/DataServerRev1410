/***********************************************************************
 * FxDBCore.h
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */


#ifndef	__FXDBCore_h__
#define	__FXDBCore_h__

#include "AqCore/AqString.h"
#include <time.h>

#include "SQA.h"

#define USE_HEADER_DIRECT  // do not use dll
/////////////////////////////////////////////////////////////////////
#ifdef USE_HEADER_DIRECT
#define FxDefDllCls_FxDbLib
#else
#ifdef MakeDll_FxDbLib
	#define FxDefDllCls_FxDbLib __declspec(dllexport)
#else 
	#define FxDefDllCls_FxDbLib __declspec(dllimport)
#endif
#endif
/////////////////////////////////////////////////////////////////////



class _SQAData;

//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
#define SQL_GET_STR_UTF8(_t, _s) _s.setDataUTF8(_t, sizeof(_t))
#define SQL_GET_STR(_t, _s)		_s.setDataS(_t, sizeof(_t))
#define SQL_GET_INT(_t, _s)		_t = _s.getDataInt()
#define SQL_GET_DATE(_t, _s)	_t = _s.getDataDate()
#define SQL_GET_DOUBLE(_t, _s)	_t = _s.getDataD()
#define SQL_GET_FLOAT(_t, _s)	_t = _s.getDataD()
#define SQL_GET_INT64(_t, _s)	_t = _s.getDataInt64()


class FxDefDllCls_FxDbLib DBCore
{
public:
	static void ConnectionPooling(bool on);
	static void SetDBConectionInfo(const wchar_t *iCinfo);
	static const wchar_t*	GetDBConectionInfo();
	static const char* GetSQLServerDBNameExt();

	// #88 2016/09/26 by N.Furutsuki
	static const char* GetSQLServerUser();
	static const char* GetSQLServerPassword();

	DBCore();
	virtual ~DBCore();
	// #15 add locale ConectionInfo 2012/04/24
	void SetMyDBConectionInfo(const wchar_t *iCinfo);
 
	
	DBType getDBType() const { return getLocalDBType();}; //add 2012/04/10 K.Ko
//	void setupDBType(DBType type) { m_DBType = type;}; // add SQLite 2011/09/08 K.Ko
	static void setupGlobalDBType(DBType type);
	void setupLocaleDBType(DBType type) ; // add SQLite 2011/09/08 K.Ko
	void	SetCancelFlag( int iFlag ) {m_cancelFlag = iFlag;};

	int		SQLExecuteBegin(SQA& iSqa);
	int		SQLExecuteProcBegin(SQA& iSqa);//2012/04/17 K.Ko

	int		SQLCommit( SQA& iSqa, bool iCommit=true);
	int		SQLNewTrans( SQA& iSqa, bool iAutoCommitExistOne=true);
	void    SQLExecuteEnd(SQA& iSqa, bool iCommit=true );
	int		SQLExecute(const wchar_t* iSQLStr, int iCommandTimeout=-1);
	int		SQLExecute(const char* iSQLStr, int iCommandTimeout=-1);
 
	virtual void Progress(int workRemaining, int workCompleted) {};

#ifdef UNITEST
	Test(int argc, char* argv[]);
#endif
	
protected:
	DBType getLocalDBType() const
	{
		if(m_LocaleDBType == kDBType_Default){
			return m_GlobalDBType;
		}else{
			return m_LocaleDBType;
		}
	};

	int		ExecuteBegin(SQA& iSqa);
	int		ExecuteProcBegin(SQA& iSqa);//2012/04/17 K.Ko

	int		openDatabase( SQA& iSqa,  bool logErr=true);
	int		closeDatabase( SQA& iSqa, bool commit=true );
 
	int		m_cancelFlag;
	//
static DBType		m_GlobalDBType; // add SQLite 2011/09/08 K.Ko
	DBType			m_LocaleDBType; // add SQLite 2012/05/14 K.Ko
 
//
// #15 add locale ConectionInfo 2012/04/24
	wchar_t m_My_ConectionInfo[256];  
	wchar_t m_My_ConStr[256+20]; 
//////

};

time_t VariantTimeToTime_t(double varDate);

#endif	/* __FXDBCore_h__ */
