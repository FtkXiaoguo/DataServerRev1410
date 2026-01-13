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

//-----------------------------------------------------------------------------
// use following struct to avoid ADO head file exposed
class _SQAData
{
public:
	_SQAData() : m_currentIndex(-1), m_transBegin(0), m_recordPostion(-1)
	{
		// To save unnecessary com object creation cost and do error hadling for it,
		// we are not create instances here, InitConnectionPtr, InitRecordsetPtr, 
		// and PreProcCall will do it.
		// mark all com pointer as unintitialized
		m_pConnection = 0;
		m_bConnection = false;
		
		m_pRecordset = 0;
		m_bRecordset = false;

		m_pCommand = 0;
		m_bCommand = false;
		
	};
	
	virtual ~_SQAData();

	virtual int InitConnectionPtr();
	virtual int InitRecordsetPtr();
	virtual int InitCommandPtr();

	virtual int Connect(const wchar_t* iConStr, bool logErr=true);
	virtual int Commit( bool iCommit);
	virtual int NewTrans();
	virtual int Disconnect(bool iCommit);
///
	long			m_currentIndex;
	int				m_transBegin;

	int				m_recordPostion;

	_ConnectionPtr	m_pConnection;
	bool			m_bConnection;

	_RecordsetPtr	m_pRecordset;
	bool			m_bRecordset;

	_CommandPtr		m_pCommand;
	bool			m_bCommand;

};

class _SQADataSQLite : public _SQAData
{
public:
	_SQADataSQLite() 
	{
	}
	virtual int InitConnectionPtr();
	virtual int InitRecordsetPtr();
	virtual int InitCommandPtr();

	virtual int Connect(const wchar_t* iConStr, bool logErr=true);
	virtual int Commit( bool iCommit);
	virtual int NewTrans();
	virtual int Disconnect(bool iCommit);

};

CComModule _Module;


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
_SQAData::~_SQAData()
{
	Disconnect(false);
}

//------------------------------------------------------------------------------------------------
int _SQAData::InitConnectionPtr()
{
	if (m_bConnection)
		return kOK;
	HRESULT hr = m_pConnection.CreateInstance( __uuidof( Connection ) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQAData::InitConnectionPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bConnection = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQAData::InitRecordsetPtr()
{
	if (m_bRecordset)
		return kOK;
	HRESULT hr = m_pRecordset.CreateInstance( __uuidof( Recordset) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQAData::InitRecordsetPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bRecordset = true;
	return kOK;
}

int _SQAData::InitCommandPtr()
{
	if (m_bCommand)
		return kOK;
	HRESULT hr = m_pCommand.CreateInstance( __uuidof( Command) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQAData::InitCommandPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bCommand = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQAData::Connect(const wchar_t* iConStr, bool logErr)
{
	if (InitConnectionPtr() != kOK)
	{
		if(logErr)
		{
			GetAqLogger()->LogMessage("ERROR: -_SQAData::Connect Cannot CreateInstance of Connection.\n" );
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
				GetAqLogger()->LogMessage( "ERROR: -_SQAData::Connect Cannot connection.\n" );
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
						GetAqLogger()->LogMessage( "ERROR: -_SQAData::Connect Error number: %x\t%s", 
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
			LogComError("_SQAData::Connect", e);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQAData::Commit( bool iCommit)
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
int	_SQAData::NewTrans()
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
int _SQAData::Disconnect(bool iCommit)
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

//------------------------------------------------------------------------------------------------
SQA::SQA():m_options(0)
{
	_pData = new _SQAData();
	SQA::Reset();
}

//------------------------------------------------------------------------------------------------
SQA::~SQA()
{

	if(_pData) delete _pData, _pData = 0;
}


//------------------------------------------------------------------------------------------------
void SQA::Reset()
{
	m_options = 0; 
	m_commandTimeout = 600;
}


//------------------------------------------------------------------------------------------------
int SQA::SetCommandText(const wchar_t* iCmd)
{
	// clear old paramters list
	if(	_pData->m_bCommand && _pData->m_pCommand->Parameters->GetCount() > 0)
	{
		_pData->m_pCommand = 0;
		_pData->m_bCommand = false;
	}

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (!iCmd)
		iCmd = L"";

	_pData->m_pCommand->CommandText = iCmd;
	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetCommandText(const char* iCmd)
{
	if (!iCmd)
		iCmd = "";
	
	return SetCommandText((wchar_t*)_bstr_t(iCmd));
	//TO DO should we use UTF8 code page?
	//return SetCommandText(m_cUStr.ToUnicode(iCmd));
}


//------------------------------------------------------------------------------------------------
int SQA::FormatCommandText(const wchar_t* fmt, ...)
{
	// clear old paramters list
	if(	_pData->m_bCommand && _pData->m_pCommand->Parameters->GetCount() > 0)
	{
		_pData->m_pCommand = 0;
		_pData->m_bCommand = false;
	}
	
	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (!fmt)
	{
		_pData->m_pCommand->CommandText = L"";
		return kOK;
	}

	try
	{
		va_list args;
		va_start(args, fmt);
		m_cUStr.VFormat(fmt, args);
		va_end(args);

		_pData->m_pCommand->CommandText = _bstr_t(m_cUStr);
	}
	catch (...)
	{
		return kDBException;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::FormatCommandText(const char* fmt, ...)
{
	try
	{
		va_list args;
		va_start(args, fmt);
		m_cStr.VFormat(fmt, args);
		va_end(args);
	}
	catch (...)
	{
		return kDBException;
	}

	return SetCommandText((wchar_t*)_bstr_t(m_cStr));
	//TO DO should we use UTF8 code page?
	//return SetCommandText(m_cUStr.ToUnicode(m_cStr));
}


const wchar_t* SQA::GetCommandText()
{
	try
	{
		return _pData->m_pCommand->CommandText;

	}
	catch( ... )
	{
		return L"";
	}
	
}

const wchar_t* SQA::GetParamtersString()
{
	try
	{
		long pNum = _pData->m_pCommand->Parameters->GetCount();
		if(pNum <= 0)
			return L"";
		
		AqUString tStr;
		_ParameterPtr pprm;
		
		m_cUStr.Format(L"With %d paramters: ", pNum);
		for(long i=0; i<pNum; i++)
		{
			pprm = _pData->m_pCommand->Parameters->GetItem(i);

			switch(pprm->GetType())
			{
				case adVarChar:
					tStr.Format(L"'%s'",  (wchar_t*)(_bstr_t)(pprm->GetValue()));
					break;

				case adVarWChar:
					tStr.Format(L"'%s'",  (wchar_t*)(_bstr_t)(pprm->GetValue()));
					break;

				case adInteger:
				case adSmallInt:
				case adTinyInt:
					tStr.Format(L"%d",  (long)(pprm->GetValue()));
					break;

				case adDouble:
				case adSingle:
					tStr.Format(L"%f",  (double)(pprm->GetValue()));
					break;

				default:
					tStr = L"?";
					break;

			}
			if(i == 0)
				m_cUStr += tStr;
			else
				m_cUStr += L", " + tStr;
				
		}

		return m_cUStr;

	}
	catch( ... )
	{
		return L"";
	}
	
}


//-------------------------------------------------------------------------
int SQA::AddParameter(const wchar_t* iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName, adVarWChar, 
			 output?adParamOutput:adParamInput, 1024, iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
		 

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}

//-------------------------------------------------------------------------
int SQA::AddParameter(const char* iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName, adVarChar, 
			 output?adParamOutput:adParamInput, 526, iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
		 

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::AddParameter(long iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName,adInteger, 
			 output?adParamOutput:adParamInput,sizeof(long),iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(char iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName,adTinyInt, 
			 output?adParamOutput:adParamInput,sizeof(char),(BYTE)iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(short iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName,adSmallInt, 
			 output?adParamOutput:adParamInput,sizeof(short),iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(double iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{
		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName, adDouble, 
			 output?adParamOutput:adParamInput,sizeof(double),iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(float iValue, const char* iName, bool output)
{

	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{
		 _ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName, adSingle, 
			 output?adParamOutput:adParamInput,sizeof(float), iValue);
		 _pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::AddParameter(__int64 iValue, const char* iName, bool output)
{
	if (_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{
		_variant_t v1;
		v1.vt = VT_I8;
		v1.llVal = iValue;
		_ParameterPtr pprm = _pData->m_pCommand->CreateParameter(iName,adBigInt, 
			 output?adParamOutput:adParamInput,sizeof(__int64), v1);
		 _pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(__int64 iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		_variant_t v1;
		v1.vt = VT_I8;
		v1.llVal = iValue;
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = v1;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(const wchar_t* iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(const char* iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(long iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(char iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = (BYTE)iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(short iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(double iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(float iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 _pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

int SQA::GetRecordPostion() {return _pData->m_recordPostion;};

//------------------------------------------------------------------------------------------------
int SQA::GetRecordCount()
{
	//if(m_options & kDBServerCursor) return kDBException;
	
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}
	return _pData->m_pRecordset->RecordCount;
}

//------------------------------------------------------------------------------------------------
int SQA::GetIndex()
{
	return _pData->m_currentIndex;
}

//------------------------------------------------------------------------------------------------
int SQA::SetIndex(int iInd)
{
	 // do not allow change index when no connection
	if(iInd>=0 && _pData->m_currentIndex >= 0)
		_pData->m_currentIndex = iInd;
	return _pData->m_currentIndex;
}

//------------------------------------------------------------------------------------------------
int	SQA::MoveFirst()
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		//ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		if( _pData->m_pRecordset->RecordCount < 1 )
			return kNoResult; // did not get any record;

		_pData->m_pRecordset->MoveFirst(); 

		if( _pData->m_pRecordset->BOF || _pData->m_pRecordset->EndOfFile )
			return kNoResult; // did not get any record;

		_pData->m_currentIndex = 0; // reset field index
		_pData->m_recordPostion = 0;

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQA::MoveLast()
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		if( _pData->m_pRecordset->RecordCount < 1 )
			return kNoResult; // did not get any record;

		_pData->m_pRecordset->MoveLast(); 

		if( _pData->m_pRecordset->BOF || _pData->m_pRecordset->EndOfFile )
			return kNoResult; // did not get any record;
		
		_pData->m_currentIndex = 0; // reset field index
		_pData->m_recordPostion = _pData->m_pRecordset->RecordCount - 1;

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int	SQA::MovePrevious()
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
        _pData->m_pRecordset->MovePrevious();

		// Trap for BOF
		if(_pData->m_pRecordset->BOF)
		{
			MoveFirst();
			return kNoMoreResult;
		}

		_pData->m_currentIndex = 0;
		_pData->m_recordPostion--;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQA::MoveNext()
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		_pData->m_pRecordset->MoveNext();

		// Trap for EndOfFile
		if( _pData->m_pRecordset->EndOfFile )
		{
			MoveLast();
			return kNoMoreResult;
		}
	
		_pData->m_currentIndex = 0;
		_pData->m_recordPostion++;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQA::Move(long iNumber)
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		// Store bookmark in case the Move goes too far 
        // forward or backward.
        _variant_t varBookmark = _pData->m_pRecordset->Bookmark;

        _pData->m_pRecordset->Move(iNumber);
		// Trap for BOF or EOF.
		if(_pData->m_pRecordset->BOF || _pData->m_pRecordset->EndOfFile)
		{
			_pData->m_pRecordset->Bookmark = varBookmark;
			return kParameterError;
		}

		_pData->m_currentIndex = 0;
		_pData->m_recordPostion += iNumber;

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int SQA::NextRecordset()
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	long    lngRec = 0;
	VARIANT_BOOL bEOF, bBOF;

	try
	{
	    _pData->m_pRecordset = _pData->m_pRecordset->NextRecordset((VARIANT *)lngRec);
		_pData->m_pRecordset->get_EndOfFile( &bEOF );
		_pData->m_pRecordset->get_BOF( &bBOF );

		if( bEOF == VARIANT_FALSE || bBOF == VARIANT_FALSE )
		{
			_pData->m_pRecordset->MoveFirst(); 
		}
		else
			return kNoResult; // did not get any record;
		
		_pData->m_pRecordset->get_EndOfFile( &bEOF );
		_pData->m_currentIndex = 0; // reset field index
	
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}
	_pData->m_recordPostion = -1;
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
int SQA::GetFieldCount()
{
	int count = 0;
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
	}

	try
	{
		count =  _pData->m_pRecordset->Fields->Count;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
	}

	return count;
}

//------------------------------------------------------------------------------------------------
// skip the data. advance the data index
//
void SQA::SkipData()
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return;
	}
	_pData->m_currentIndex++;
}

//------------------------------------------------------------------------------------------------
// Get Data as std::string
//
int SQA::getDataST(AqUString& oName, AqUString& oValue, int index)
{
	variant_t curIdx;
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return -1;
	}

	try
	{
		if(index > -1) 
			_pData->m_currentIndex = index;

		curIdx = _pData->m_currentIndex++;
		variant_t& var = _pData->m_pRecordset->Fields->Item[curIdx]->Value;
		if (var.vt == VT_NULL)
		{
			oValue = L"";
		}
		else
		{
			oValue =  (wchar_t*) (bstr_t) var;
			if(!(m_options & kDBNoStringRTrim))
				oValue.TrimRight();
		}

		bstr_t& name = _pData->m_pRecordset->Fields->Item[curIdx]->Name;
		oName = (wchar_t*) name;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
// Get Data unicode string
//
const wchar_t* SQA::getDataUS(int index)
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return L""; // record not initialized
	}

	try
	{
		if(index > -1) _pData->m_currentIndex = index;
		variant_t& var = _pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			return L"";
		
		if(!(m_options & kDBNoStringRTrim))
		{
			m_cUStr = (wchar_t*)var.bstrVal;
			m_cUStr.TrimRight();
			return m_cUStr;
		}
		else
		{
			return (wchar_t*)var.bstrVal;
		}
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return L"";
	}
}

//------------------------------------------------------------------------------------------------
// Get Data as char pointer 
//
const char* SQA::getDataS(int index)
{
	m_cStr.Convert(getDataUS(index), CP_ACP);
	return m_cStr; 
}

//------------------------------------------------------------------------------------------------
void SQA::setDataS(wchar_t* iTarget, int iSize, int index)
{
	const wchar_t* str = getDataUS(index);

	iSize = iSize/2;
	wcsncpy(iTarget, str, iSize);
	iTarget[iSize-1] = L'\0';
}


//------------------------------------------------------------------------------------------------
void SQA::setDataS(char* iTarget, int iSize, int index)
{
	const char* str = getDataS(index);

	strncpy(iTarget, str, iSize);
	iTarget[iSize-1] = '\0';
}


//------------------------------------------------------------------------------------------------
// Get Data as double for date
//

double SQA::getDataDate(int index)
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) _pData->m_currentIndex = index;
		variant_t& var = _pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			return 0.0;
		else 
			return var.date;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return 0;
	}

}

time_t VariantTimeToTime_t(double varDate)
{
    struct  tm  ctm;
    SYSTEMTIME  st;

    if( !VariantTimeToSystemTime(varDate, &st) ) return 0;
    
    ctm.tm_sec = st.wSecond;
    ctm.tm_min = st.wMinute;
    ctm.tm_hour = st.wHour;
    ctm.tm_mday = st.wDay;
    ctm.tm_mon = st.wMonth - 1;
    ctm.tm_year = st.wYear - 1900;
    ctm.tm_wday = st.wDayOfWeek;
    ctm.tm_isdst = -1;   // Force DST checking
    
	return mktime(&ctm);
}

//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

int SQA::getDataInt(int index)
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) _pData->m_currentIndex = index;
		variant_t& var = _pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			return 0;
		else // cast all long, int, smallint, and tinyint to long
			return (long) var;

		//return _pData->m_pRecordset->Fields->GetItem(_pData->m_currentIndex++ )->Value.intVal;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return 0;
	}
}

__int64 SQA::getDataInt64(int index)
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) _pData->m_currentIndex = index;
		variant_t& var = _pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL ||  var.vt != VT_I8)
			return 0;
		else 
			return var.llVal;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return 0;
	}
}

//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

void SQA::getDataInt(char* oNum, int size, int index)
{
#ifdef _DEBUG
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
	}
#endif //_DEBUG

	try
	{
		long vl;
		char buf[34];
		if(index > -1) _pData->m_currentIndex = index;
		variant_t& var = _pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			vl = 0;
		else // cast all long, int, smallint, and tinyint to long
			vl = (long)var;
		
		//CComVariant var = _pData->m_pRecordset->Fields->GetItem(_pData->m_currentIndex++ )->Value;
		//itoa(var.intVal, buf, 10);
		itoa(vl, buf, 10);
		strncpy(oNum, buf, size); oNum[size-1] = 0;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
	}
}

//------------------------------------------------------------------------------------------------
// Get Data Double                                 
//

double SQA::getDataD(int index)
{
	if(!_pData->m_bRecordset || _pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) _pData->m_currentIndex = index;
		variant_t& var = _pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			return 0.0;
		else // casting to make it works with float too
			return (double) var;
	
		
		//return (double)(_pData->m_pRecordset->Fields->Item[_pData->m_currentIndex++]->Value);
		//return _pData->m_pRecordset->Fields->GetItem(_pData->m_currentIndex++ )->Value.dblVal;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return 0;
	}
}

#if 0
//-------------------------------------------------------------------------
// Set DB Value                                                     
void SQA::setDBValue( SQA& iSqa , int i, const char* str )
{
	if(!_pData->m_bRecordset) return;

	CComBSTR bstr = _T( str );
	CComVariant value = bstr;
	_pData->m_pRecordset->Fields->GetItem( i )->Value = value;
}

//------------------------------------------------------------------------------------------------
void SQA::setDBValue( SQA& iSqa , int i, long n )
{
	if(!_pData->m_bRecordset) return;

	CComVariant value = n;
	_pData->m_pRecordset->Fields->GetItem( i )->Value = value;
}

//------------------------------------------------------------------------------------------------
void SQA::setDBValue( SQA& iSqa , int i, int n )
{
	if(!_pData->m_bRecordset) return;
	_RecordsetPtr* ppr = (_RecordsetPtr*)iSqa.m_ppRecordset;

	CComVariant value = n;
	CComVariant index = i;
	_pData->m_pRecordset->Fields->GetItem( index )->Value = value;
}

//------------------------------------------------------------------------------------------------
void SQA::setDBValue( SQA& iSqa , int i, double n )
{
	if(!_pData->m_bRecordset) return;
	_RecordsetPtr* ppr = (_RecordsetPtr*)iSqa.m_ppRecordset;

	CComVariant value = n;
	CComVariant index = i;
	_pData->m_pRecordset->Fields->GetItem( index )->Value = value;
}
#endif


//static TRCriticalSection Update_cs;
static _ConnectionPtr	PermanentConnectionPtr = 0; // set to 0 to turn off pooling
static wchar_t ConectionInfo[256] = L"";
static wchar_t _ConStr[256+20] = L"";


const wchar_t* DBCore::GetDBConectionInfo()
{
	return ConectionInfo;
}

void DBCore::SetDBConectionInfo(const wchar_t *iCinfo) 
{
	if(!iCinfo || wcslen(iCinfo) >= 256 || wcsicmp(ConectionInfo,iCinfo) == 0)
		return;

	bool hasPool = (PermanentConnectionPtr != 0)?true:false;
	if(hasPool)
		ConnectionPooling(false);

	wcscpy(ConectionInfo, iCinfo);
	wcscpy(_ConStr, iCinfo);
	wcscat(_ConStr, L";OLE DB Services=-1");

	if(hasPool)
		ConnectionPooling(true);
}

void DBCore::ConnectionPooling(bool on)
{
	static long inChanging = 0;
	if (InterlockedCompareExchange(&inChanging, 0, 1) == 1)
		return;

	if(on)
	{
		if(PermanentConnectionPtr != 0 || ConectionInfo[0] == L'\0')
		{
			inChanging = 0;
			return;
		}

		HRESULT hr = PermanentConnectionPtr.CreateInstance( __uuidof( Connection ) );	
		ATLASSERT( SUCCEEDED( hr ) );
		if( hr != S_OK )
		{
			PermanentConnectionPtr = 0;
			inChanging = 0;
			return;
		}
		hr = PermanentConnectionPtr->Open( ConectionInfo, L"", L"", -1 );
		if( hr != S_OK )
		{
			ATLASSERT( 0 );
			PermanentConnectionPtr = 0;
			inChanging = 0;
			return;
		}
		// as long as we keep the connection object
		// even pool is destroyed, it will be recreated by the first connection
		PermanentConnectionPtr->Close(); 

	}
	else
	{
		if(PermanentConnectionPtr != 0)
		{
			//PermanentConnectionPtr->Close();
			PermanentConnectionPtr = 0;
		}
	}
}

//------------------------------------------------------------------------------------------------
DBCore::DBCore() : m_cancelFlag(0)
{
}

//------------------------------------------------------------------------------------------------
DBCore::~DBCore()
{
}

static char* OutResourceStr = "has been chosen as the deadlock victim";
//static const char* DuplicateKeyStr ="duplicate key";
//static const char* Incorrectsyntax = "Incorrect syntax near '";

/********************************************************************/
/* Open Database                                                    */
/********************************************************************/
int DBCore::openDatabase( SQA& iSqa, bool logErr)
{
	return iSqa._pData->Connect(_ConStr, logErr);
}

//------------------------------------------------------------------------------------------------
int DBCore::closeDatabase( SQA& iSqa, bool iCommit)
{
	return iSqa._pData->Disconnect(iCommit);
}


//------------------------------------------------------------------------------------------------
int	DBCore::ExecuteBegin(SQA& iSqa)
{
	int rcode = openDatabase( iSqa);
	if( rcode != kOK ) return kDBOpenError;
 
	// use transaction
	if(!(iSqa.m_options & kDBNoTransaction))
	{
		rcode = SQLNewTrans(iSqa, false);
		if( rcode != kOK ) return kDBOpenError;
	}

	if (iSqa._pData->InitCommandPtr() != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin InitCommandPtr fail.\n" );
		GetAqLogger()->FlushLog();
		closeDatabase( iSqa, false );
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kComInitError;
	}

	iSqa._pData->m_pCommand->ActiveConnection = iSqa._pData->m_pConnection;
	iSqa._pData->m_pCommand->CommandTimeout = iSqa.m_commandTimeout;
	//did in SQA //iSqa._pData->m_pCommand->CommandText = _bstr_t(iSqa._pData->m_commandText);
	iSqa._pData->m_pCommand->CommandType = adCmdText;
	
	if(!(iSqa.m_options & kDBExecuteNoRecords) )
	{
		if (iSqa._pData->InitRecordsetPtr() != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin Cannot CreateInstance of Recordset.\n" );
			GetAqLogger()->FlushLog();
			closeDatabase( iSqa, false );
			ATLASSERT( 0 ); //show exception on error in debug mode
			return kComInitError;
		}

		if(iSqa._pData->m_pRecordset->State == adStateOpen) 
			iSqa._pData->m_pRecordset->Close();

		iSqa._pData->m_pRecordset->CursorType = adOpenStatic;
		iSqa._pData->m_pRecordset->CursorLocation = adUseClient;
	}

	long options = adCmdText;
	if(iSqa.m_options & kDBAsyncExecute)
		options |= adAsyncExecute;

	int retry = 0;

	//	Rob Lewis - 05/14/03 - Reset cancel flag before entering the loop.  Otherwise, we could have a previous
	//		irrelevant cancel event (i.e. bug# 3669)
	m_cancelFlag = 0;
	do {
		try
		{
			iSqa._pData->m_pConnection->Errors->Clear();

			if(iSqa.m_options & kDBExecuteNoRecords )
			{
				iSqa._pData->m_pCommand->Execute(0, 0, options|adExecuteNoRecords);
				if(options & adAsyncExecute)
				{
					int index = 0;
					while( iSqa._pData->m_pCommand->GetState() == adStateExecuting )
					{
						Sleep(20); // T.C. Zhao JUN-05-2003 changed from 50ms

						//TRACE( "waiting %d...\n", index++);
						index++;
						Progress(-1,index);
						if( m_cancelFlag )
						{
							iSqa._pData->m_pCommand->Cancel();
							Progress(-1, -1);
							m_cancelFlag = 0;
							closeDatabase( iSqa, false );
							GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin operation cancelled.\n" );
							return kDBCancell;
						}

					}
					// use -1 so we don't turn off the progress 
				//	Progress(-1, iSqa._pData->m_pRecordset->RecordCount);
				}

			}
			else
			{
				//iSqa._pData->m_pRecordset = 
				//iSqa._pData->m_pCommand->Execute(0,0,adCmdText);
				iSqa._pData->m_pRecordset->Open(iSqa._pData->m_pCommand.GetInterfacePtr(),
					&vtMissing, iSqa._pData->m_pRecordset->CursorType, adLockReadOnly, options);
				if(options & adAsyncExecute)
				{
					int index = 0;
					while( iSqa._pData->m_pRecordset->GetState() == adStateExecuting )
					{
						Sleep(20); // T.C. Zhao JUN-05-2003 changed from 50ms

						//TRACE( "waiting %d...\n", index++);
						index++;
						Progress(-1,index);
						if( m_cancelFlag )
						{
							iSqa._pData->m_pRecordset->Cancel();
							Progress(-1, -1);
							m_cancelFlag = 0;
							closeDatabase( iSqa, false );
							GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin operation cancelled.\n" );
							return kDBCancell;
						}

					}
					// use -1 so we don't turn off the progress 
				//	Progress(-1, iSqa._pData->m_pRecordset->RecordCount);
				}
			}

			iSqa._pData->m_currentIndex = 0; // reset field index
			break;
		}
		catch( _com_error &e )
		{
			AqString errStr = (char*)e.Description();
			AqString cmdStr, paraStr;
			HRESULT eCode = e.Error();

			retry++;
			
			if(eCode == DB_E_TIMEOUT || eCode == DB_E_OBJECTCREATIONLIMITREACHED || 
					eCode == DB_E_ERRORSINCOMMAND || //#658 2010/03/16 K.Ko
					eCode == DB_E_INTEGRITYVIOLATION ||
					strstr(errStr, OutResourceStr) > 0)
			{
				if( eCode == DB_E_TIMEOUT && retry >= 5)
				{
					LogComError("SQLExecuteBegin", e);
					return kDBTimeout;
				}else if( eCode == DB_E_ERRORSINCOMMAND && retry >= 60) //#658 2010/03/16 K.Ko
				{
					LogComError("SQLExecuteBegin", e);
					return kDBTimeout;
				}
				else if(retry >= 20)
				{
					LogComError("SQLExecuteBegin", e);
					return kDBOutResource;
				}


				cmdStr.ConvertUTF8(iSqa.GetCommandText());
				GetAqLogger()->LogMessage(kWarning,"Warning: -DBCore::SQLExecuteBegin, SQL:%s\n, com_error: %s\n", cmdStr, errStr);
				Sleep(100+retry*100);

			}
			else
			{
				if(eCode == DB_E_INTEGRITYVIOLATION && iSqa.m_options & kDBNoLogOnIntegrityViolation )
					return kDBException;
				cmdStr.ConvertUTF8(iSqa.GetCommandText());
				paraStr.ConvertUTF8(iSqa.GetParamtersString());
				errStr.Format("SQLExecuteBegin with SQL:%s %s, com_error: %s", cmdStr, paraStr, errStr);
				LogComError(errStr, e);

				ATLASSERT( 0 );
				if(eCode == DB_E_ABORTLIMITREACHED|| eCode == DB_S_STOPLIMITREACHED)
					return kDBOutResource;
				else	
					return kDBException;
			}
		}

//	} while(retry > 0 && retry < 20 );
	} while(retry > 0 && retry < 60 );//#658 2010/03/16 K.Ko

	if(retry>=60){ //#658 2010/03/16 K.Ko
		GetAqLogger()->LogMessage("ERROR: DBCore::ExecuteBegin  retry >=60 failed \n" );				
	}
	return kOK;
}

// add try block to avoid access exception when Integrity Violation exception happend
// !!! need to study why access exception happend.
int	DBCore::SQLExecuteBegin(SQA& iSqa)
{	
	AqLoggerInterface* pLog = GetAqLogger();

	int loglevel = pLog->GetLogLevel();
	if(loglevel >= kDebug)
	{
		AqString cmdStr, paraStr;
		cmdStr.ConvertUTF8(iSqa.GetCommandText());
		paraStr.ConvertUTF8(iSqa.GetParamtersString());

		pLog->LogMessage(kDebug, "Info %d DBCore::SQLExecuteBegin with SQL:%s %s\n", GetCurrentThreadId(), cmdStr, paraStr);
	}

	try
	{
		int rcode = ExecuteBegin(iSqa);
		/*  modified V1.8.5.5  2010/04/19 */
		if(rcode == kDBOpenError || rcode == kComInitError)
		{
			AqString cmdStr, paraStr;

			cmdStr.ConvertUTF8(iSqa.GetCommandText());
			paraStr.ConvertUTF8(iSqa.GetParamtersString());
			pLog->LogMessage(kInfo,"Info(%d) SQL statment:%s %s\n", GetCurrentThreadId(), cmdStr, paraStr);
			pLog->FlushLog();
		}
		return rcode;
	}
	catch( ... )
	{
		if(!(iSqa.m_options & kDBNoLogOnIntegrityViolation) )
		{
			AqString cmdStr, paraStr;
			cmdStr.ConvertUTF8(iSqa.GetCommandText());
			paraStr.ConvertUTF8(iSqa.GetParamtersString());

			pLog->LogMessage("ERROR %d DBCore::SQLExecuteBegin with SQL:%s %s\n", GetCurrentThreadId(), cmdStr, paraStr);
			pLog->FlushLog();
		}
		return kDBException;
	}
}

//------------------------------------------------------------------------------------------------
int DBCore::SQLCommit( SQA& iSqa, bool iCommit)
{
	return iSqa._pData->Commit(iCommit);
}

//------------------------------------------------------------------------------------------------
int	DBCore::SQLNewTrans( SQA& iSqa, bool iAutoCommitExistOne/*=true*/)
{
	if(iSqa._pData->m_transBegin > 0)
	{
		if(!iAutoCommitExistOne)
			return kOK;	// don't want to end existed session, do nothing 
		else
			iSqa._pData->Commit(true);
	}
	return iSqa._pData->NewTrans();
}

//------------------------------------------------------------------------------------------------
void DBCore::SQLExecuteEnd(SQA& iSqa, bool iCommit)
{
	closeDatabase( iSqa, iCommit );
}

//------------------------------------------------------------------------------------------------
//
int	DBCore::SQLExecute(const wchar_t* iSQLStr, int iCommandTimeout)
{
	SQA sqa;
	sqa.m_options = kDBExecuteNoRecords|kDBNoTransaction;
	sqa.SetCommandText(iSQLStr);
	if(iCommandTimeout >= 0)
		sqa.m_commandTimeout = iCommandTimeout;

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
		return retcd;

	SQLExecuteEnd(sqa);

	return kOK;
}



//------------------------------------------------------------------------------------------------
//
int	DBCore::SQLExecute(const char* iSQLStr, int iCommandTimeout)
{
	return SQLExecute(AqUString(iSQLStr, CP_UTF8), iCommandTimeout);
}



#ifdef UNITEST
int DBCore::Test(int argc, char* argv[])
{
	int errorCount = 0;
	char localName[ 255 ];
	unsigned long nSize = sizeof( localName );
	GetComputerName( localName, &nSize );

	AqUString strSQL;

	strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;APPLICATION NAME=%S;Integrated Security=SSPI", 
		localName, GetCurrentProcessName());
	
	SetDBConectionInfo(strSQL);
	ConnectionPooling(true);

	printf("test db connection\n");
	SQA sqa;
	int retcd;
	int iretry = 3;
	bool logerror = false;
	do
	{
		if(iretry == 1)
			logerror = true;

		retcd = openDatabase(sqa, logerror);
		closeDatabase( sqa );
		if(retcd == kOK || m_cancelFlag)
			break;

		Sleep(5000);
	} 
	while(iretry-- > 0);

	if(retcd != kOK)
	{
		printf("test db connection failed !\n");
		errorCount++;
	}


	printf("test create database\n");
	SQLExecute(L"drop database TestDB");
	

	retcd = SQLExecute(L"CREATE DATABASE TestDB");
	if(retcd != kOK)
	{
		printf("test create database failed!\n");
		errorCount++;
	}
	

	
	strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;APPLICATION NAME=%S;Initial Catalog=TestDB;Integrated Security=SSPI", 
		localName, GetCurrentProcessName());
	
	SetDBConectionInfo(strSQL);
	
			
	printf("test create table studylevel\n");
	
	retcd = SQLExecute(L"CREATE TABLE StudyLevel "
		L"("
		L"	StudyLevelID INT IDENTITY(1,1) PRIMARY KEY, "
		L"	StudyInstanceUID VARCHAR(64) not null,  "
		L"	PatientsName NVARCHAR(332),  "
		L"	PatientID NVARCHAR(64) DEFAULT '',  "
		L"	PatientsBirthDate VARCHAR(10),  "
		L"	PatientsSex	NVARCHAR(16),  "
		L"	PatientsAge	INT,  "
		L"	StudyDate VARCHAR(10) DEFAULT '',  "
		L"	StudyTime VARCHAR(16) DEFAULT '',  "
		L"	AccessionNumber NVARCHAR(16),  "
		L"	StudyID NVARCHAR(16) DEFAULT '', " 
		L"	ReadingPhysiciansName NVARCHAR(332), "
		L"	ReferringPhysiciansName NVARCHAR(332), " 
		L"	ModalitiesInStudy VARCHAR(64),  "
		L"	StudyDescription NVARCHAR(64),  "
		L"	NumberOfStudyRelatedSeries INT,  "
		L"	NumberOfStudyRelatedInstances INT, " 
		L"	CharacterSet VARCHAR(256),  "
		L"	Status INT DEFAULT 0, "
		L"	AccessTime datetime DEFAULT GETDATE()	 "
		L") "
		L"CREATE UNIQUE INDEX StudyInstanceUID_index on TestDB.dbo.StudyLevel(StudyInstanceUID) "
		L"CREATE INDEX PatientsName_index on TestDB.dbo.StudyLevel (PatientsName) "
		L"CREATE INDEX PatientID_index on TestDB.dbo.StudyLevel (PatientID) "
		L"CREATE INDEX StudyDate_index on TestDB.dbo.StudyLevel (StudyDate) "
		L"CREATE INDEX ModalitiesInStudy_index on TestDB.dbo.StudyLevel (ModalitiesInStudy) "
		L"CREATE INDEX StudyAccessTime_index on TestDB.dbo.StudyLevel (AccessTime) "

	);

	if(retcd != kOK)
	{
		printf("test create table studylevel failed!\n");
		errorCount++;
	}

	printf("test create stored procedure MakeStudy\n");

	retcd = SQLExecute(
		L" CREATE PROCEDURE MakeStudy  ("
		L" @StudyInstanceUID VARCHAR(64)," 
		L" @PatientsName NVARCHAR(332), "
		L" @PatientID NVARCHAR(64), "
		L" @PatientsBirthDate VARCHAR(10), "
		L" @PatientsSex	NVARCHAR(16), "
		L" @PatientsAge	INT, "
		L" @StudyDate VARCHAR(10), "
		L" @StudyTime VARCHAR(16), "
		L" @AccessionNumber NVARCHAR(16), "
		L" @StudyID NVARCHAR(16), "
		L" @ReadingPhysiciansName NVARCHAR(332),"
		L" @ReferringPhysiciansName NVARCHAR(332), "
		L" @ModalitiesInStudy VARCHAR(64), "
		L" @StudyDescription NVARCHAR(64), "
		L" @NumberOfStudyRelatedSeries INT," 
		L" @NumberOfStudyRelatedInstances INT, "
		L" @CharacterSet VARCHAR(16)"
		L" )"
		L" "
		L" AS"
		L" BEGIN"
		L" 	SET NOCOUNT ON"
		L" 	IF @NumberOfStudyRelatedSeries < 1"
		L" 		SET @NumberOfStudyRelatedSeries = 1"
		L" "
		L" 	IF NOT EXISTS(SELECT StudyLevelID FROM dbo.StudyLevel "
		L" 		WHERE StudyInstanceUID=@StudyInstanceUID )"
		L" 		INSERT INTO dbo.StudyLevel ("
		L" 		StudyInstanceUID, "
		L" 		PatientsName, "
		L" 		PatientID, "
		L" 		PatientsBirthDate, "
		L" 		PatientsSex, "
		L" 		PatientsAge, "
		L" 		StudyDate, "
		L" 		StudyTime, "
		L" 		AccessionNumber, "
		L" 		StudyID, "
		L" 		ReadingPhysiciansName,"
		L" 		ReferringPhysiciansName , "
		L" 		ModalitiesInStudy, "
		L" 		StudyDescription, "
		L" 		NumberOfStudyRelatedSeries, "
		L" 		NumberOfStudyRelatedInstances, "
		L" 		CharacterSet"
		L" 		) "
		L" 		VALUES ("
		L" 		@StudyInstanceUID,"
		L" 		@PatientsName,"
		L" 		@PatientID,"
		L" 		@PatientsBirthDate, "
		L" 		@PatientsSex, "
		L" 		@PatientsAge, "
		L" 		@StudyDate, "
		L" 		@StudyTime, "
		L" 		@AccessionNumber, "
		L" 		@StudyID, "
		L" 		@ReadingPhysiciansName,"
		L" 		@ReferringPhysiciansName, "
		L" 		@ModalitiesInStudy, "
		L" 		@StudyDescription, "
		L" 		@NumberOfStudyRelatedSeries, "
		L" 		@NumberOfStudyRelatedInstances, "
		L" 		@CharacterSet"
		L" 		)"
		L" 	"
		L" END"
		);

	if(retcd != kOK)
	{
		printf("test create stored procedure MakeStudy failed!\n");
		errorCount++;
	}

	printf("test insert to studylevel\n");

/*
		L" @StudyInstanceUID VARCHAR(64)," 
		L" @PatientsName NVARCHAR(332), "
		L" @PatientID NVARCHAR(64), "
		L" @PatientsBirthDate VARCHAR(10), "
		L" @PatientsSex	NVARCHAR(16), "
		L" @PatientsAge	INT, "
		L" @StudyDate VARCHAR(10), "
		L" @StudyTime VARCHAR(16), "
		L" @AccessionNumber NVARCHAR(16), "
		L" @StudyID NVARCHAR(16), "
		L" @ReadingPhysiciansName NVARCHAR(332),"
		L" @ReferringPhysiciansName NVARCHAR(332), "
		L" @ModalitiesInStudy VARCHAR(64), "
		L" @StudyDescription NVARCHAR(64), "
		L" @NumberOfStudyRelatedSeries INT," 
		L" @NumberOfStudyRelatedInstances INT, "
		L" @CharacterSet VARCHAR(16)"
*/



//94744	1.2.124.113532.10.8.8.59.20051215.125935.23252992	RICE^RICHARD	3693222	19420421	M	0	20051215	
//	172239.000000	RA053490212001	10718	RA053490212001	THOMPSON^PAUL^	CT	CT CHEST W	5	170	ISO_IR 100	0

	//select nchar(20013)+ nchar(25991)+ nchar(35797)+nchar(39564)  chinese test
	//	2D 4E 87 65 D5 8B 8C 9A 00
	
	char studyUID0[] = "1.2.124.113532.10.8.8.59.20051215.125935.23252992";
	
	//wchar_t patientsName[] = L"RICE^RICHARD";
	wchar_t patientsName0[] = L"\x4E2D\x6587\x8BD5\x9A8C";

	wchar_t patientID0[] = L"3693222";
	char patientsBirthDate0[]="19420421";
	wchar_t patientsSex0[] = L"M";
	int patientsAge0 = 0;
	char studyDate0[] = "20051215";
	char studyTime0[] = "172239.000000";
	wchar_t accessionNumber0[]= L"RA053490212001";
	wchar_t studyID0[] = L"10718";
	wchar_t readingPhysiciansName0[] = L"RA053490212001";
	wchar_t referringPhysiciansName0[] = L"THOMPSON^PAUL^";
	char modalitiesInStudy0[] = "CT";
	wchar_t studyDescription0[] = L"CT CHEST W";
	int numberOfStudyRelatedSeries0 = 5;
	int NumberOfStudyRelatedInstances0 = 170;
	char CharacterSet0[] = "ISO_IR 100";
	


	sqa.SetCommandText("EXEC MakeStudy ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?");
	sqa.AddParameter(studyUID0);
	sqa.AddParameter(patientsName0);
	sqa.AddParameter(patientID0);
	sqa.AddParameter(patientsBirthDate0);
	sqa.AddParameter(patientsSex0);
	sqa.AddParameter(patientsAge0);
	sqa.AddParameter(studyDate0);
	sqa.AddParameter(studyTime0);
	sqa.AddParameter(accessionNumber0);
	sqa.AddParameter(studyID0);
	sqa.AddParameter(readingPhysiciansName0);
	sqa.AddParameter(referringPhysiciansName0);
	sqa.AddParameter(modalitiesInStudy0);
	sqa.AddParameter(studyDescription0);
	sqa.AddParameter(numberOfStudyRelatedSeries0);
	sqa.AddParameter(NumberOfStudyRelatedInstances0);
	sqa.AddParameter(CharacterSet0);

	sqa.m_options = kDBLockExecute|kDBNoLogOnIntegrityViolation|kDBExecuteNoRecords;
	retcd = SQLExecuteBegin(sqa);

	if(retcd != kOK)
	{
		printf("test insert to studylevel failed!\n");
		errorCount++;
	}

	SQLExecuteEnd(sqa);



	printf("test select from studylevel\n");
	sqa.FormatCommandText(
		L"Select StudyInstanceUID, PatientsName, PatientID, PatientsBirthDate, PatientsSex, "
		L"PatientsAge, StudyDate, StudyTime, AccessionNumber, StudyID, ReadingPhysiciansName, ReferringPhysiciansName , "
		L"ModalitiesInStudy, StudyDescription, NumberOfStudyRelatedSeries, NumberOfStudyRelatedInstances, CharacterSet"
		L" From studylevel where StudyInstanceUID='%S' and PatientsName=N'%s' and CharacterSet='%S' and PatientsAge=%d",
		studyUID0,
		patientsName0,
		CharacterSet0,
		0
	);

	sqa.m_options = kDBLockExecute;
	retcd = SQLExecuteBegin(sqa);

	if(retcd != kOK ||  sqa.GetRecordCount() != 1 || sqa.MoveFirst()!=kOK)
	{
		printf("test select from studylevel failed!\n");
		errorCount++;
	}
	else
	{
		char studyUID[65];
		wchar_t patientsName[333];
		wchar_t patientID[65];
		char patientsBirthDate[11];
		wchar_t patientsSex[17];
		int patientsAge;
		char studyDate[11];
		char studyTime[17];
		wchar_t accessionNumber[17];
		wchar_t studyID[17];
		wchar_t readingPhysiciansName[333];
		wchar_t referringPhysiciansName[333];
		char modalitiesInStudy[65];
		wchar_t studyDescription[65];
		int numberOfStudyRelatedSeries;
		int NumberOfStudyRelatedInstances;
		char CharacterSet[17];
		


		SQL_GET_STR(studyUID, sqa);
		SQL_GET_STR(patientsName, sqa);
		SQL_GET_STR(patientID, sqa);
		SQL_GET_STR(patientsBirthDate, sqa);
		SQL_GET_STR(patientsSex, sqa);
		SQL_GET_INT(patientsAge, sqa);
		SQL_GET_STR(studyDate, sqa);
		SQL_GET_STR(studyTime, sqa);
		SQL_GET_STR(accessionNumber, sqa);
		SQL_GET_STR(studyID, sqa);
		SQL_GET_STR(readingPhysiciansName, sqa);
		SQL_GET_STR(referringPhysiciansName, sqa);
		SQL_GET_STR(modalitiesInStudy, sqa);
		SQL_GET_STR(studyDescription, sqa);
		SQL_GET_INT(numberOfStudyRelatedSeries, sqa);
		SQL_GET_INT(NumberOfStudyRelatedInstances, sqa);
		SQL_GET_STR(CharacterSet, sqa);

		if(AqString(studyUID) != studyUID0)
		{
			printf("test select from studylevel failed on study UID!\n");
			errorCount++;
		}

		if(AqUString(patientsName) != patientsName0)
		{
			printf("test select from studylevel failed on patientsName!\n");
			errorCount++;
		}	
	
		if(numberOfStudyRelatedSeries != numberOfStudyRelatedSeries0)
		{
			printf("test select from studylevel failed on numberOfStudyRelatedSeries!\n");
			errorCount++;
		}	
	
	
	}

	SQLExecuteEnd(sqa);


	printf("test pateint name query on studylevel\n");

	AqUString qpname(patientsName0);
	qpname.SetAt(L'%', 2);
	qpname.SetAt(L'\0', 3);
	
	// must use N' for unicode string
	sqa.FormatCommandText(
		L"Select StudyInstanceUID, PatientsName, PatientID, PatientsBirthDate, PatientsSex, "
		L"PatientsAge, StudyDate, StudyTime, AccessionNumber, StudyID, ReadingPhysiciansName, ReferringPhysiciansName , "
		L"ModalitiesInStudy, StudyDescription, NumberOfStudyRelatedSeries, NumberOfStudyRelatedInstances, CharacterSet"
		L" From studylevel where PatientsName like N'%s'",	qpname	);

	sqa.m_options = kDBLockExecute;
	retcd = SQLExecuteBegin(sqa);

	if(retcd != kOK ||  sqa.GetRecordCount() != 1 || sqa.MoveFirst()!=kOK)
	{
		printf("test pateint name query on studylevel failed!\n");
		errorCount++;
	}
	else
	{
		char studyUID[65];
		wchar_t patientsName[333];
		wchar_t patientID[65];
		char patientsBirthDate[11];
		wchar_t patientsSex[17];
		int patientsAge;
		char studyDate[11];
		char studyTime[17];
		wchar_t accessionNumber[17];
		wchar_t studyID[17];
		wchar_t readingPhysiciansName[333];
		wchar_t referringPhysiciansName[333];
		char modalitiesInStudy[65];
		wchar_t studyDescription[65];
		int numberOfStudyRelatedSeries;
		int NumberOfStudyRelatedInstances;
		char CharacterSet[17];
		


		SQL_GET_STR(studyUID, sqa);
		SQL_GET_STR(patientsName, sqa);
		SQL_GET_STR(patientID, sqa);
		SQL_GET_STR(patientsBirthDate, sqa);
		SQL_GET_STR(patientsSex, sqa);
		SQL_GET_INT(patientsAge, sqa);
		SQL_GET_STR(studyDate, sqa);
		SQL_GET_STR(studyTime, sqa);
		SQL_GET_STR(accessionNumber, sqa);
		SQL_GET_STR(studyID, sqa);
		SQL_GET_STR(readingPhysiciansName, sqa);
		SQL_GET_STR(referringPhysiciansName, sqa);
		SQL_GET_STR(modalitiesInStudy, sqa);
		SQL_GET_STR(studyDescription, sqa);
		SQL_GET_INT(numberOfStudyRelatedSeries, sqa);
		SQL_GET_INT(NumberOfStudyRelatedInstances, sqa);
		SQL_GET_STR(CharacterSet, sqa);

		if(AqString(studyUID) != studyUID0)
		{
			printf("test pateint name query on studylevel failed on study UID!\n");
			errorCount++;
		}

		if(AqUString(patientsName) != patientsName0)
		{
			printf("test pateint name query on studylevel failed on patientsName!\n");
			errorCount++;
		}	
	
		if(numberOfStudyRelatedSeries != numberOfStudyRelatedSeries0)
		{
			printf("test pateint name query on studylevel failed on numberOfStudyRelatedSeries!\n");
			errorCount++;
		}	
	
	
	}

	SQLExecuteEnd(sqa);




	return errorCount;


}
#endif //UNITEST

const char* DBCore::GetSQLServerDBNameExt()
{
	static char serverDBName[256]={0};
	
	HKEY hRootKey;
	char szBuff[256]={0};
	DWORD cbBuff;

	// レジストリキーを開きます
	const char keyName[] = "Software\\PreXion\\AQServer";
	if(	RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS){
		// 実際にアクセスし文字列を取得します
		cbBuff = sizeof(szBuff);	
		if( RegQueryValueEx(hRootKey, "SQLServerDBNameExt", NULL, NULL, (LPBYTE)szBuff, &cbBuff) == ERROR_SUCCESS){
			memset( serverDBName, 0x00, sizeof(serverDBName) );
			strncpy( serverDBName, szBuff, sizeof(serverDBName)-1 );
		}

		RegCloseKey(hRootKey);
	}
	return serverDBName;
//	return "\\SQLEXPRESS";
}


/////////

//------------------------------------------------------------------------------------------------
_SQAData::~_SQAData()
{
	Disconnect(false);
}

//------------------------------------------------------------------------------------------------
int _SQAData::InitConnectionPtr()
{
	if (m_bConnection)
		return kOK;
	HRESULT hr = m_pConnection.CreateInstance( __uuidof( Connection ) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQAData::InitConnectionPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bConnection = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQAData::InitRecordsetPtr()
{
	if (m_bRecordset)
		return kOK;
	HRESULT hr = m_pRecordset.CreateInstance( __uuidof( Recordset) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQAData::InitRecordsetPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bRecordset = true;
	return kOK;
}

int _SQAData::InitCommandPtr()
{
	if (m_bCommand)
		return kOK;
	HRESULT hr = m_pCommand.CreateInstance( __uuidof( Command) );	
	ATLASSERT( SUCCEEDED( hr ) );
	if( hr != S_OK )
	{
		GetAqLogger()->LogMessage("ERROR: _SQAData::InitCommandPtr (0X%X)\n", hr);
		return kComInitError;
	}
	m_bCommand = true;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQAData::Connect(const wchar_t* iConStr, bool logErr)
{
	if (InitConnectionPtr() != kOK)
	{
		if(logErr)
		{
			GetAqLogger()->LogMessage("ERROR: -_SQAData::Connect Cannot CreateInstance of Connection.\n" );
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
				GetAqLogger()->LogMessage( "ERROR: -_SQAData::Connect Cannot connection.\n" );
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
						GetAqLogger()->LogMessage( "ERROR: -_SQAData::Connect Error number: %x\t%s", 
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
			LogComError("_SQAData::Connect", e);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int _SQAData::Commit( bool iCommit)
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
int	_SQAData::NewTrans()
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
int _SQAData::Disconnect(bool iCommit)
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

