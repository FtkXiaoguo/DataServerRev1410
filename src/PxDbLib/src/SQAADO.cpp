/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */


//#include "AqCore/stdafx.h"
#include "SQAADO.h"
#include "AqCore/TRLogger.h"

#include "SQA.h"

#include <atlbase.h>
#include <OledbErr.h>

#include <time.h>

#ifndef ADO_IMPORT
#define ADO_IMPORT
#pragma warning (disable: 4146)
#import "msadox.dll" rename_namespace("ADOX") rename("EOF", "EndOfFile")
#import "msado15.dll" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
using namespace ADOCG;
#endif

 #include "CheckMemoryLeak.h"
 
#include "SQADataADO.h"
#include "SQADataSqlite.h"

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

//------------------------------------------------------------------------------------------------

#define My_pData ((_SQADataADO*)_pData)
SQAADO::SQAADO() 
{
	m_options =  0;
	_pData = new _SQADataADO();
	 
	SQAADO::Reset();
}

//------------------------------------------------------------------------------------------------
SQAADO::~SQAADO()
{

	if(_pData) { // memory leak 2012/02/20 K.Ko
		delete My_pData;
		_pData = 0;
	}
}


//------------------------------------------------------------------------------------------------
void SQAADO::Reset()
{
 	m_options = 0; 
 	m_commandTimeout = 600;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetCommandText(const wchar_t* iCmd)
{
	 
	// clear old paramters list
	if(	My_pData->m_bCommand && My_pData->m_pCommand->Parameters->GetCount() > 0)
	{
		My_pData->m_pCommand = 0;
		My_pData->m_bCommand = false;
	}

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (!iCmd)
		iCmd = L"";

	My_pData->m_pCommand->CommandText = iCmd;
	 
	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetCommandText(const char* iCmd)
{
	if (!iCmd)
		iCmd = "";
	
	return SetCommandText((wchar_t*)_bstr_t(iCmd));
	//TO DO should we use UTF8 code page?
	//return SetCommandText(m_cUStr.ToUnicode(iCmd));
}



const wchar_t* SQAADO::GetCommandText()
{
	try
	{
		return My_pData->m_pCommand->CommandText;

	}
	catch( ... )
	{
		return L"";
	}
	
}

const wchar_t* SQAADO::GetParamtersString()
{
	try
	{
		long pNum = My_pData->m_pCommand->Parameters->GetCount();
		if(pNum <= 0)
			return L"";
		
		AqUString tStr;
		_ParameterPtr pprm;
		
		m_cUStr.Format(L"With %d paramters: ", pNum);
		for(long i=0; i<pNum; i++)
		{
			pprm = My_pData->m_pCommand->Parameters->GetItem(i);

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
int SQAADO::AddParameter(const wchar_t* iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName, adVarWChar, 
			 output?adParamOutput:adParamInput, 1024, iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
		 

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}

//-------------------------------------------------------------------------
int SQAADO::AddParameter(const char* iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName, adVarChar, 
			 output?adParamOutput:adParamInput, 526, iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
		 

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::AddParameter(long iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName,adInteger, 
			 output?adParamOutput:adParamInput,sizeof(long),iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int SQAADO::AddParameter(char iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName,adTinyInt, 
			 output?adParamOutput:adParamInput,sizeof(char),(BYTE)iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQAADO::AddParameter(short iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{

		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName,adSmallInt, 
			 output?adParamOutput:adParamInput,sizeof(short),iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQAADO::AddParameter(double iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{
		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName, adDouble, 
			 output?adParamOutput:adParamInput,sizeof(double),iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQAADO::AddParameter(float iValue, const char* iName, bool output)
{

	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{
		 _ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName, adSingle, 
			 output?adParamOutput:adParamInput,sizeof(float), iValue);
		 My_pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::AddParameter(__int64 iValue, const char* iName, bool output)
{
	if (My_pData->InitCommandPtr() != kOK)
		return kComInitError;

	if (iName == 0) iName = "";
	try
	{
		_variant_t v1;
		v1.vt = VT_I8;
		v1.llVal = iValue;
		_ParameterPtr pprm = My_pData->m_pCommand->CreateParameter(iName,adBigInt, 
			 output?adParamOutput:adParamInput,sizeof(__int64), v1);
		 My_pData->m_pCommand->Parameters->Append(pprm);
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(__int64 iValue, long iIdx, const char* iName)
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
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = v1;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(const wchar_t* iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(const char* iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(long iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(char iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = (BYTE)iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(short iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(double iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQAADO::SetParameter(float iValue, long iIdx, const char* iName)
{
	_variant_t vtIdx;
	if (iName != 0) 
		vtIdx = iName;
	else
		vtIdx = iIdx;
	try
	{
		 My_pData->m_pCommand->Parameters->Item[vtIdx]->Value = iValue;
	}
	catch( ... )
	{
		ATLASSERT( 0 );
 		return kParameterError;
	}

	 return kOK;
}

int SQAADO::GetRecordPostion() {return My_pData->m_recordPostion;};

//------------------------------------------------------------------------------------------------
int SQAADO::GetRecordCount()
{
	//if(m_options & kDBServerCursor) return kDBException;
	
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}
	return My_pData->m_pRecordset->RecordCount;
}

//------------------------------------------------------------------------------------------------
int SQAADO::GetIndex()
{
	return My_pData->m_currentIndex;
}

//------------------------------------------------------------------------------------------------
int SQAADO::SetIndex(int iInd)
{
	 // do not allow change index when no connection
	if(iInd>=0 && My_pData->m_currentIndex >= 0)
		My_pData->m_currentIndex = iInd;
	return My_pData->m_currentIndex;
}

//------------------------------------------------------------------------------------------------
int	SQAADO::MoveFirst()
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		//ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		if( My_pData->m_pRecordset->RecordCount < 1 )
			return kNoResult; // did not get any record;

		My_pData->m_pRecordset->MoveFirst(); 

		if( My_pData->m_pRecordset->BOF || My_pData->m_pRecordset->EndOfFile )
			return kNoResult; // did not get any record;

		My_pData->m_currentIndex = 0; // reset field index
		My_pData->m_recordPostion = 0;

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQAADO::MoveLast()
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		if( My_pData->m_pRecordset->RecordCount < 1 )
			return kNoResult; // did not get any record;

		My_pData->m_pRecordset->MoveLast(); 

		if( My_pData->m_pRecordset->BOF || My_pData->m_pRecordset->EndOfFile )
			return kNoResult; // did not get any record;
		
		My_pData->m_currentIndex = 0; // reset field index
		My_pData->m_recordPostion = My_pData->m_pRecordset->RecordCount - 1;

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
int	SQAADO::MovePrevious()
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
        My_pData->m_pRecordset->MovePrevious();

		// Trap for BOF
		if(My_pData->m_pRecordset->BOF)
		{
			MoveFirst();
			return kNoMoreResult;
		}

		My_pData->m_currentIndex = 0;
		My_pData->m_recordPostion--;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQAADO::MoveNext()
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		My_pData->m_pRecordset->MoveNext();

		// Trap for EndOfFile
		if( My_pData->m_pRecordset->EndOfFile )
		{
			MoveLast();
			return kNoMoreResult;
		}
	
		My_pData->m_currentIndex = 0;
		My_pData->m_recordPostion++;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQAADO::Move(long iNumber)
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	try
	{
		// Store bookmark in case the Move goes too far 
        // forward or backward.
        _variant_t varBookmark = My_pData->m_pRecordset->Bookmark;

        My_pData->m_pRecordset->Move(iNumber);
		// Trap for BOF or EOF.
		if(My_pData->m_pRecordset->BOF || My_pData->m_pRecordset->EndOfFile)
		{
			My_pData->m_pRecordset->Bookmark = varBookmark;
			return kParameterError;
		}

		My_pData->m_currentIndex = 0;
		My_pData->m_recordPostion += iNumber;

	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------
int SQAADO::NextRecordset()
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return kComInitError; // record not initialized
	}

	long    lngRec = 0;
	VARIANT_BOOL bEOF, bBOF;

	try
	{
	    My_pData->m_pRecordset = My_pData->m_pRecordset->NextRecordset((VARIANT *)lngRec);
		My_pData->m_pRecordset->get_EndOfFile( &bEOF );
		My_pData->m_pRecordset->get_BOF( &bBOF );

		if( bEOF == VARIANT_FALSE || bBOF == VARIANT_FALSE )
		{
			My_pData->m_pRecordset->MoveFirst(); 
		}
		else
			return kNoResult; // did not get any record;
		
		My_pData->m_pRecordset->get_EndOfFile( &bEOF );
		My_pData->m_currentIndex = 0; // reset field index
	
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return kDBException;
	}
	My_pData->m_recordPostion = -1;
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
int SQAADO::GetFieldCount()
{
	int count = 0;
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
	}

	try
	{
		count =  My_pData->m_pRecordset->Fields->Count;
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
void SQAADO::SkipData()
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return;
	}
	My_pData->m_currentIndex++;
}

//------------------------------------------------------------------------------------------------
// Get Data as std::string
//
int SQAADO::getDataST(AqUString& oName, AqUString& oValue, int index)
{
	variant_t curIdx;
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return -1;
	}

	try
	{
		if(index > -1) 
			My_pData->m_currentIndex = index;

		curIdx = My_pData->m_currentIndex++;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[curIdx]->Value;
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

		bstr_t& name = My_pData->m_pRecordset->Fields->Item[curIdx]->Name;
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
const wchar_t* SQAADO::getDataUS(int index)
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return L""; // record not initialized
	}

	try
	{
		if(index > -1) My_pData->m_currentIndex = index;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value;
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
// Get Data as double for date
//

double SQAADO::getDataDate(int index)
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) My_pData->m_currentIndex = index;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value;
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



//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

int SQAADO::getDataInt(int index)
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) My_pData->m_currentIndex = index;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			return 0;
		else // cast all long, int, smallint, and tinyint to long
			return (long) var;

		//return My_pData->m_pRecordset->Fields->GetItem(My_pData->m_currentIndex++ )->Value.intVal;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return 0;
	}
}

__int64 SQAADO::getDataInt64(int index)
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) My_pData->m_currentIndex = index;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value;
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

void SQAADO::getDataInt(char* oNum, int size, int index)
{
#ifdef _DEBUG
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
	}
#endif //_DEBUG

	try
	{
		long vl;
		char buf[34];
		if(index > -1) My_pData->m_currentIndex = index;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			vl = 0;
		else // cast all long, int, smallint, and tinyint to long
			vl = (long)var;
		
		//CComVariant var = My_pData->m_pRecordset->Fields->GetItem(My_pData->m_currentIndex++ )->Value;
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

double SQAADO::getDataD(int index)
{
	if(!My_pData->m_bRecordset || My_pData->m_pRecordset->State != adStateOpen) 
	{
		ATLASSERT( 0 ); //show error in debug mode
		return 0; // record not initialized
	}

	try
	{
		if(index > -1) My_pData->m_currentIndex = index;
		variant_t& var = My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value;
		if( var.vt == VT_NULL )
			return 0.0;
		else // casting to make it works with float too
			return (double) var;
	
		
		//return (double)(My_pData->m_pRecordset->Fields->Item[My_pData->m_currentIndex++]->Value);
		//return My_pData->m_pRecordset->Fields->GetItem(My_pData->m_currentIndex++ )->Value.dblVal;
	}
	catch( _com_error& )
	{
		ATLASSERT( 0 );
 		return 0;
	}
}

static char* OutResourceStr = "has been chosen as the deadlock victim";
int SQAADO::ExecuteBegin(SQAParam * param)
{
	if (My_pData->InitCommandPtr() != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin InitCommandPtr fail.\n" );
		GetAqLogger()->FlushLog();
//		closeDatabase( iSqa, false );
		My_pData->Disconnect(false);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kComInitError;
	}

	My_pData->m_pCommand->ActiveConnection = My_pData->m_pConnection;
	My_pData->m_pCommand->CommandTimeout = getCommandTimeout();
	//did in SQA //iSqa.getSQAData()->m_pCommand->CommandText = _bstr_t(iSqa.getSQAData()->m_commandText);
	My_pData->m_pCommand->CommandType = adCmdText;
	
	if(!(getOptions() & kDBExecuteNoRecords) )
	{
		if (My_pData->InitRecordsetPtr() != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin Cannot CreateInstance of Recordset.\n" );
			GetAqLogger()->FlushLog();
			//closeDatabase( iSqa, false );
			My_pData->Disconnect(false);
			ATLASSERT( 0 ); //show exception on error in debug mode
			return kComInitError;
		}

		if(My_pData->m_pRecordset->State == adStateOpen) 
			My_pData->m_pRecordset->Close();

		My_pData->m_pRecordset->CursorType = adOpenStatic;
		My_pData->m_pRecordset->CursorLocation = adUseClient;
	}

	long options = adCmdText;
	if(getOptions() & kDBAsyncExecute)
		options |= adAsyncExecute;

	int retry = 0;

	//	-- - 05/14/03 - Reset cancel flag before entering the loop.  Otherwise, we could have a previous
	//		irrelevant cancel event (i.e. bug# 3669)
	m_cancelFlag = 0;
	do {
		try
		{
			My_pData->m_pConnection->Errors->Clear();

			if(getOptions() & kDBExecuteNoRecords )
			{
				My_pData->m_pCommand->Execute(0, 0, options|adExecuteNoRecords);
				if(options & adAsyncExecute)
				{
					int index = 0;
					while( My_pData->m_pCommand->GetState() == adStateExecuting )
					{
						Sleep(20); // -- JUN-05-2003 changed from 50ms

						//TRACE( "waiting %d...\n", index++);
						index++;
						if(param){
							param->Progress(-1,index);
						}
						if( m_cancelFlag )
						{
							My_pData->m_pCommand->Cancel();
							if(param){
								param->Progress(-1, -1);
							}
							m_cancelFlag = 0;
					//		closeDatabase( iSqa, false );
							My_pData->Disconnect(false);
							GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin operation cancelled.\n" );
							return kDBCancell;
						}

					}
					// use -1 so we don't turn off the progress 
				//	Progress(-1, iSqa.getSQAData()->m_pRecordset->RecordCount);
				}

			}
			else
			{
				//iSqa.getSQAData()->m_pRecordset = 
				//iSqa.getSQAData()->m_pCommand->Execute(0,0,adCmdText);
				My_pData->m_pRecordset->Open(My_pData->m_pCommand.GetInterfacePtr(),
					&vtMissing, My_pData->m_pRecordset->CursorType, adLockReadOnly, options);
				if(options & adAsyncExecute)
				{
					int index = 0;
					while( My_pData->m_pRecordset->GetState() == adStateExecuting )
					{
						Sleep(20); // -- JUN-05-2003 changed from 50ms

						//TRACE( "waiting %d...\n", index++);
						index++;
						if(param){
							param->Progress(-1,index);
						}
						if( m_cancelFlag )
						{
							My_pData->m_pRecordset->Cancel();
							if(param){
								param->Progress(-1, -1);
							}
							m_cancelFlag = 0;
					//		closeDatabase( iSqa, false );
							My_pData->Disconnect(false);
							GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin operation cancelled.\n" );
							return kDBCancell;
						}

					}
					// use -1 so we don't turn off the progress 
				//	Progress(-1, iSqa.getSQAData()->m_pRecordset->RecordCount);
				}
			}

			My_pData->m_currentIndex = 0; // reset field index
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


				cmdStr.ConvertUTF8(GetCommandText());
				GetAqLogger()->LogMessage(kWarning,"Warning: -DBCore::SQLExecuteBegin, SQL:%s\n, com_error: %s\n", cmdStr, errStr);
				Sleep(100+retry*100);

			}
			else
			{
				if(eCode == DB_E_INTEGRITYVIOLATION && getOptions() & kDBNoLogOnIntegrityViolation )
					return kDBException;
				cmdStr.ConvertUTF8(GetCommandText());
				paraStr.ConvertUTF8(GetParamtersString());
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

	return kOK ;
}

 int SQAADO::ExecuteProcBegin(SQAParam * param)  //2012/04/17 K.Ko
{
	if (My_pData->InitCommandPtr() != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: -DBCore::SQLExecuteBegin InitCommandPtr fail.\n" );
		GetAqLogger()->FlushLog();
//		closeDatabase( iSqa, false );
		My_pData->Disconnect(false);
		ATLASSERT( 0 ); //show exception on error in debug mode
		return kComInitError;
	}

	My_pData->m_pCommand->ActiveConnection = My_pData->m_pConnection;
	My_pData->m_pCommand->CommandTimeout = getCommandTimeout();
	//did in SQA //iSqa.getSQAData()->m_pCommand->CommandText = _bstr_t(iSqa.getSQAData()->m_commandText);
//	My_pData->m_pCommand->CommandText = _bstr_t(iSqa.getSQAData()->m_commandText);;
	My_pData->m_pCommand->CommandType = adCmdStoredProc;
	
	 


	int retry = 0;

	//	-- - 05/14/03 - Reset cancel flag before entering the loop.  Otherwise, we could have a previous
	//		irrelevant cancel event (i.e. bug# 3669)
	m_cancelFlag = 0;
	do {
		try
		{
			My_pData->m_pConnection->Errors->Clear();
			 
			My_pData->m_pCommand->Execute(0, 0, adCmdStoredProc);
				 
			My_pData->m_currentIndex = 0; // reset field index
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


				cmdStr.ConvertUTF8(GetCommandText());
				GetAqLogger()->LogMessage(kWarning,"Warning: -DBCore::SQLExecuteBegin, SQL:%s\n, com_error: %s\n", cmdStr, errStr);
				Sleep(100+retry*100);

			}
			else
			{
				if(eCode == DB_E_INTEGRITYVIOLATION && getOptions() & kDBNoLogOnIntegrityViolation )
					return kDBException;
				cmdStr.ConvertUTF8(GetCommandText());
				paraStr.ConvertUTF8(GetParamtersString());
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

	return kOK ; ;
}

 //2012/04/17 K.Ko
int SQAADO::GetParamtersCount()
{
	long pNum = -1;
	try
	{
	  pNum = My_pData->m_pCommand->Parameters->GetCount();
	}
	catch( ... )
	{
		 
	}
	return pNum;
}
AqUString SQAADO::GetParamtersString(int index)
{
	AqUString returnStr;
	returnStr.Empty();
	try
	{
		long pNum = My_pData->m_pCommand->Parameters->GetCount();

		if(index>=pNum ){
			return returnStr;
		}
		
		_ParameterPtr pprm;
		
		 
	//	for(long i=0; i<pNum; i++)
		{
	//		if(i != index) continue;
		 
			pprm = My_pData->m_pCommand->Parameters->GetItem( (long)index);

			switch(pprm->GetType())
			{
				case adVarChar:
					returnStr.Format(L"'%s'",  (wchar_t*)(_bstr_t)(pprm->GetValue()));
					break;

				case adVarWChar:
					returnStr.Format(L"'%s'",  (wchar_t*)(_bstr_t)(pprm->GetValue()));
					break;

				case adInteger:
				case adSmallInt:
				case adTinyInt:
					returnStr.Format(L"%d",  (long)(pprm->GetValue()));
					break;

				case adDouble:
				case adSingle:
					returnStr.Format(L"%f",  (double)(pprm->GetValue()));
					break;

				default:
					returnStr = L"?";
					break;
			}
		}

	}
	catch( ... )
	{
		 ;
	}

	return returnStr;
}