/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */

//#include "AqCore/stdafx.h"
#include "DBCore.h"
#include "AqCore/TRLogger.h"

#include <atlbase.h>
#include <OledbErr.h>

#ifndef ADO_IMPORT
#define ADO_IMPORT
#pragma warning (disable: 4146)
#import "msadox.dll" rename_namespace("ADOX") rename("EOF", "EndOfFile")
#import "msado15.dll" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
using namespace ADOCG;
#endif

 #include "CheckMemoryLeak.h"

#include "SQADataADO.h"

//CComModule _Module;


///////////////////////////////////////////////////////////
//                                                       //
//      PrintComError Function                           //
//                                                       //
///////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
static void LogComError(const char* header, _com_error &e)
{
   // Log COM errors. 
   GetAqLogger()->LogMessage("ERROR{ %d DBCore::%s }: %s(0X%X)\n", 
	   GetCurrentThreadId(), header, (LPCSTR)e.Description(), e.Error());
   GetAqLogger()->FlushLog();
}

//------------------------------------------------------------------------------------------------
_SQADataADO::~_SQADataADO()
{
	Disconnect(false);
}

//------------------------------------------------------------------------------------------------
int _SQADataADO::InitConnectionPtr()
{
	if (m_bConnection)
		return kOK;
	HRESULT hr = m_pConnection.CreateInstance( __uuidof( Connection ) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQADataADO::InitConnectionPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bConnection = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQADataADO::InitRecordsetPtr()
{
	if (m_bRecordset)
		return kOK;
	HRESULT hr = m_pRecordset.CreateInstance( __uuidof( Recordset) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQADataADO::InitRecordsetPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bRecordset = true;
	return kOK;
}

int _SQADataADO::InitCommandPtr()
{
	if (m_bCommand)
		return kOK;
	HRESULT hr = m_pCommand.CreateInstance( __uuidof( Command) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQADataADO::InitCommandPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bCommand = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQADataADO::Connect(const wchar_t* iConStr, bool logErr)
{
	if (InitConnectionPtr() != kOK)
	{
		if(logErr)
		{
			GetAqLogger()->LogMessage("ERROR: -_SQADataADO::Connect Cannot CreateInstance of Connection.\n" );
			GetAqLogger()->FlushLog();
		}
		return kComInitError;
	}

	if(m_pConnection->State == adStateOpen) 
		return kOK;
	m_pConnection->CursorLocation = adUseClient;
	m_pConnection->ConnectionTimeout = 30;
	m_pConnection->CommandTimeout = 600;

	try
	{
		HRESULT hr = m_pConnection->Open(iConStr, L"", L"", -1);
		if( hr != S_OK )
		{
			if(logErr)
			{
				GetAqLogger()->LogMessage( "ERROR: -_SQADataADO::Connect Cannot connection.\n" );
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
						GetAqLogger()->LogMessage( "ERROR: -_SQADataADO::Connect Error number: %x\t%s", 
								pErr->Number,pErr->Description);
					}
				}
				
				GetAqLogger()->FlushLog();
			}
			ATLASSERT( 0 );
			return kDBOpenError;
		}
 
	}
	catch( _com_error& e )
	{
		if(logErr)
			LogComError("_SQADataADO::Connect", e);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQADataADO::Commit( bool iCommit)
{
	if (!m_bConnection) return kOK;
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
	return kOK;
}

//------------------------------------------------------------------------------------------------
int	_SQADataADO::NewTrans()
{
	if (!m_bConnection) return kOK;
	if(m_pConnection->State != adStateOpen) return kOK;

	// can not start new transaction before commit
	if(m_transBegin > 0)
		return kDBException;
	try
	{
		m_pConnection->BeginTrans();
		m_transBegin =  1;

	}
	catch( _com_error& e )
	{
		LogComError("SQLNewTrans", e);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}
	return kOK;

}


//------------------------------------------------------------------------------------------------
int _SQADataADO::Disconnect(bool iCommit)
{
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
	return kOK;
}
