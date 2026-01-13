/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */

  
#ifndef	__SQADataSQlite_h__
#define	__SQADataSQlite_h__


#include "SQAData.h"
#include <string>


struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_mutex;

class AQDataSetSqlite;
class _SQADataSQLite : public _SQAData
{
public:
	static void initSQLite();
	_SQADataSQLite() ;
	 
	virtual ~_SQADataSQLite() ;

	virtual int InitConnectionPtr();
	virtual int InitRecordsetPtr();
	virtual int InitCommandPtr();

	virtual int Connect(const wchar_t* iConStr, bool logErr=true);
	virtual int Commit( bool iCommit);
	virtual int NewTrans();
	virtual int Disconnect(bool iCommit);

	void setCommand(const wchar_t* cmd);
	const wchar_t* getCommand();
	//
	int  ExecuteBegin(AQDataSetSqlite *dataset );
 
protected:
	int cnvErrorCode(int rc);
	long			m_currentIndex;
	int				m_transBegin;

	int				m_recordPostion;

//	_ConnectionPtr	m_pConnection;
	bool			m_bConnection;

//	_RecordsetPtr	m_pRecordset;
	bool			m_bRecordset;

	std::string		m_strCommand;
	bool			m_bCommand;

	 sqlite3 *m_sqliteDB; 
	//
	AqUString		 m_strUCommand ;
	//
	sqlite3_stmt *m_pSelect;
	//
//	QADataset		m_dataset;
	//
	int m_retryNN;
	//
//	static sqlite3_mutex  *m_mutex;
//	static int		m_binitSQLite;
};

#endif //__SQADataSQlite_h__