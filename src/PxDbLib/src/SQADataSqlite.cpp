/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */

//#include "AqCore/stdafx.h"
#include "DBCore.h"
#include "AqCore/TRLogger.h"

 
 
#include "DatasetSqlite.h"


#include "sqlite3.h"

#include "CheckMemoryLeak.h"

//#define SLEEP_MSEC 500
#define SLEEP_MSEC 100  //#29 2012/06/22 K.Ko

#if 0
///////////
// ïsóvÇ…Ç»ÇÈÅH
#include <atlbase.h>
#include <OledbErr.h>

#ifndef ADO_IMPORT
#define ADO_IMPORT
#pragma warning (disable: 4146)
#import "msadox.dll" rename_namespace("ADOX") rename("EOF", "EndOfFile")
#import "msado15.dll" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
using namespace ADOCG;
#endif

#endif
/////////
#include "SQADataSqlite.h"



//////
#if 0
sqlite3 *_SQADataSQLite::m_sqliteDB=0;
sqlite3_mutex  *_SQADataSQLite::m_mutex=0;
int	_SQADataSQLite::m_binitSQLite = 0;
#endif
/////////
void _SQADataSQLite::initSQLite()
{
	 
}
_SQADataSQLite::_SQADataSQLite() : m_currentIndex(-1), m_transBegin(0), m_recordPostion(-1)
{
 	m_sqliteDB = 0;
	m_pSelect = 0;
		
	//	m_pConnection = 0;
		m_bConnection = false;
		
	//	m_pRecordset = 0;
		m_bRecordset = false;

	//	m_pCommand = 0;
		m_bCommand = false;
	//
	m_retryNN = 50;//5; //#29 2012/06/22 K.Ko
}
//------------------------------------------------------------------------------------------------
_SQADataSQLite::~_SQADataSQLite()
{
	 	Disconnect(false);
}

//------------------------------------------------------------------------------------------------
int _SQADataSQLite::InitConnectionPtr()
{
	if (m_bConnection)
		return kOK;
#if 0
	HRESULT hr = m_pConnection.CreateInstance( __uuidof( Connection ) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQADataSQLite::InitConnectionPtr (0X%X)\n", hr);
		return kComInitError;
	}
#endif
	m_bConnection = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQADataSQLite::InitRecordsetPtr()
{
	if (m_bRecordset)
		return kOK;
#if 0
	HRESULT hr = m_pRecordset.CreateInstance( __uuidof( Recordset) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQADataSQLite::InitRecordsetPtr (0X%X)\n", hr);
		return kComInitError;
	}
#endif
	m_bRecordset = true;
	return kOK;
}

int _SQADataSQLite::InitCommandPtr()
{

	if (m_bCommand)
		return kOK;
#if 0
	HRESULT hr = m_pCommand.CreateInstance( __uuidof( Command) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQADataSQLite::InitCommandPtr (0X%X)\n", hr);
		return kComInitError;
	}
#endif

	m_bCommand = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQADataSQLite::Connect(const wchar_t* iConStr, bool logErr)
{
	if (InitConnectionPtr() != kOK)
	{
		if(logErr)
		{
			GetAqLogger()->LogMessage("ERROR: -_SQADataSQLite::Connect Cannot CreateInstance of Connection.\n" );
			GetAqLogger()->FlushLog();
		}
		return kComInitError;
	}

#if 0
 	if(m_pConnection->State == adStateOpen) 
 		return kOK;
	m_pConnection->CursorLocation = adUseClient;
	m_pConnection->ConnectionTimeout = 30;
	m_pConnection->CommandTimeout = 600;
#else
	if(m_sqliteDB) return kOK;
#endif

	try
	{
#if 1
		AqString db_name ;
		db_name.ConvertUTF8(iConStr);
		
		// strSQL.Format(L"Provider=SQLITE;Data Source=%S",c_dbServerName);
		std::string str_temp = (const char*)db_name;
		std::string::size_type index = str_temp.find("Data Source=");
		if( index == std::string::npos )  // åüçıÇ≈Ç´ÇΩÇ©Ç«Ç§Ç©
		{
			return kDBOpenError;
		}
		str_temp = str_temp.substr( index ) ;
		//
		//
		index = str_temp.find("=");
		if( index == std::string::npos )  // åüçıÇ≈Ç´ÇΩÇ©Ç«Ç§Ç©
		{
			return kDBOpenError;
		}
		str_temp = str_temp.substr( index+1 ) ;
		//
		index = str_temp.find(";");
		if( index == std::string::npos )  // åüçıÇ≈Ç´ÇΩÇ©Ç«Ç§Ç©
		{
			return kDBOpenError;
		}
		str_temp = str_temp.substr(0,index ) ;
	//	db_name.
		const char *file_name = str_temp.c_str();

		int rc = -1;
		{
			 
			for(int run_i =0 ;run_i<m_retryNN;run_i++) //#29 2012/06/22 K.Ko
			{
				rc = sqlite3_open(file_name, &m_sqliteDB);
				if( rc !=SQLITE_OK){
				 
					sqlite3_close(m_sqliteDB);
					sqlite3_sleep(SLEEP_MSEC);
					 
				}else{
					 
					break;
				}
			}
			
		}
		if(rc!=SQLITE_OK){
			return kComInitError;
		}


#else
		HRESULT hr = m_pConnection->Open(iConStr, L"", L"", -1);
		if( hr != S_OK )
		{
			if(logErr)
			{
				GetAqLogger()->LogMessage( "ERROR: -_SQADataSQLite::Connect Cannot connection.\n" );
				// Print Provider Errors from Connection object.
				// pErr is a record object in the Connection's Error collection.
				ErrorPtr    pErr  = NULL;

				if( (m_pConnection->Errors->Count) > 0)
				{
					long nCount = m_pConnection->Errors->Count;
					// Collection ranges from 0 to nCount -1.
					for(long i = 0;i < nCount;i++)
					{
						pErr = m_pConnection->Errors->GetItem(i);
						GetAqLogger()->LogMessage( "ERROR: -_SQADataSQLite::Connect Error number: %x\t%s", 
								pErr->Number,pErr->Description);
					}
				}
				
				GetAqLogger()->FlushLog();
			}
			ATLASSERT( 0 );
			return kDBOpenError;
		}
#endif
 
	}
	catch( ... )
	{
		//if(logErr)
		//	LogComError("_SQADataSQLite::Connect", e);
		//ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQADataSQLite::Commit( bool iCommit)
{
	if (!m_bConnection) return kOK;
#if 0
	if(m_pConnection->State != adStateOpen) return kOK;

	try
	{
		if(m_transBegin > 0)
		{
			if(iCommit)
				m_pConnection->CommitTrans();
			else
				m_pConnection->RollbackTrans();
			m_transBegin = 0; // only one transcation session can exist
		}
	}
	catch( _com_error& e )
	{
		LogComError("SQLCommit", e);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}
#endif

	return kOK;
}
 static int callback(void *NotUsed, int argc, char **argv, char **azColName)
 {
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
		printf("\n");
	return 0;
}

//------------------------------------------------------------------------------------------------
int	_SQADataSQLite::NewTrans()
{
	if (!m_bConnection) return kOK;

	char *zErrMsg = 0;

	int rc=kComInitError;
	if(m_sqliteDB){
		const char *sql_cmd = m_strCommand.c_str();
		
#if 0
		int rc = sqlite3_exec(m_sqliteDB, sql_cmd, callback, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
		}
#else
		const char *zTail;
	
		for(int run_i =0 ;run_i<m_retryNN;run_i++) //#29 2012/06/22 K.Ko
		{
			if(m_pSelect){
	 			sqlite3_finalize(m_pSelect);
	  			m_pSelect = 0;
			}
			//Compiling An SQL Statement
			rc = sqlite3_prepare(	m_sqliteDB,            /* Database handle */
									sql_cmd,       /* SQL statement, UTF-8 encoded */
									m_strCommand.size(),              /* Maximum length of zSql in bytes. */
									&m_pSelect,  /* OUT: Statement handle */
									&zTail     /* OUT: Pointer to unused portion of zSql */
									);
			if( rc!=SQLITE_OK ){
				sqlite3_sleep(SLEEP_MSEC);
			}else{
				break;
			}
		}

#endif
	}
 
	return cnvErrorCode(rc);

}


//------------------------------------------------------------------------------------------------
int _SQADataSQLite::Disconnect(bool iCommit)
{
#if 1
	if(m_sqliteDB){

		if(m_pSelect){
	 		sqlite3_finalize(m_pSelect);
	  	    m_pSelect = 0;
		}
		sqlite3_close(m_sqliteDB);
//		delete m_sqliteDB;
		m_sqliteDB = 0;
	}
#else
	try
	{
		if (m_bConnection && m_pConnection->State == adStateOpen)
		{
			Commit(iCommit);
			m_pConnection->Close();
			m_currentIndex = -1;
			m_recordPostion = -1;

		}
	}
	catch( _com_error& e )
	{
		LogComError("Disconnect", e);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}
#endif

	return kOK;
}

void _SQADataSQLite::setCommand(const wchar_t* cmd){
	m_strUCommand = cmd;
	AqString cmd_temp ;
	cmd_temp.ConvertUTF8(cmd);
	m_strCommand = cmd_temp;
};
const wchar_t* _SQADataSQLite::getCommand()
{
	return (const wchar_t*)m_strUCommand;
}

int  _SQADataSQLite::ExecuteBegin(AQDataSetSqlite *dataset )
{
	if(!dataset){
	//	 
	//	ATLASSERT( 0 ); //show exception on error in debug mode
		return kFailedUnknown;
	}

	int rc=SQLITE_ERROR;
	if(m_sqliteDB){
		 
		
 

		for(int run_i =0 ;run_i<m_retryNN;run_i++) //#29 2012/06/22 K.Ko
		{
		  if((rc = sqlite3_step(m_pSelect)) ==  SQLITE_BUSY) {
        // sleep instead requesting result again immidiately.
			sqlite3_sleep(SLEEP_MSEC);
		  }else{
			  break;
		  }
		}
		/* if we have a result set... */
      if( SQLITE_ROW == rc ){
		  int col_i;
		  int nCol = sqlite3_column_count(m_pSelect);
		  dataset->m_dataset.clear();
	//	  std::vector<std::string > col_names;

		  dataset->m_colNames.clear();
		  for(col_i=0;col_i<nCol;col_i++){
	//		  std::string col_name =  (char *)sqlite3_column_name(m_pSelect, col_i);
			  dataset->m_colNames.push_back((char *)sqlite3_column_name(m_pSelect, col_i));
		  }
		  std::vector<int> col_types;
		  col_types.resize(nCol);
		  
		  //
		  do{
			  QARecordData new_record;
			  new_record.resize(nCol); //2012/05/22 K.Ko
              /* extract the data and data types */
              for(col_i=0; col_i<nCol; col_i++){
                std::string col_val = (char *)sqlite3_column_text(m_pSelect, col_i);
                col_types[col_i] = sqlite3_column_type(m_pSelect, col_i);
                if( !col_types[col_i] && (col_types[col_i]!=SQLITE_NULL) ){
                  rc = SQLITE_NOMEM;
                  break; /* from for */
                }
				 
	//			new_record[dataset->m_colNames[col_i]] = col_val;
				//2012/05/22 K.Ko
				new_record[col_i] = col_val; 

              } /* end for */

			  dataset->m_dataset.push_back(new_record);

              /* if data and types extracted successfully... */
              if( SQLITE_ROW == rc ){ 
         
					for(int run_i =0 ;run_i<m_retryNN;run_i++) //#29 2012/06/22 K.Ko
					{
						if((rc = sqlite3_step(m_pSelect)) ==  SQLITE_BUSY) {
						// sleep instead requesting result again immidiately.
							sqlite3_sleep(SLEEP_MSEC);
						  }else{
							  break;
						  }
					}
             
              }
            } while( SQLITE_ROW == rc );

   //         sqlite3_free(pData);

	  }
 
	}
 
	
	return cnvErrorCode(rc);
}

int _SQADataSQLite::cnvErrorCode(int rc)
{
	int ret_v = kFailedUnknown;
	switch(rc){
	 case SQLITE_OK:
	 case SQLITE_ROW:
	 case SQLITE_DONE:
		 ret_v= kOK;
	}
	return ret_v;
}