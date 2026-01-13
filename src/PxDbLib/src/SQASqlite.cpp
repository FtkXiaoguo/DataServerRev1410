/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */


//#include "AqCore/stdafx.h"
#include "SQASqlite.h"
#include "AqCore/TRLogger.h"

#include "SQA.h"

#include <atlbase.h>
#include <OledbErr.h>

#include <time.h>

#include "DatasetSqlite.h"

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

/////////////////////////

 


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

#define My_pData ((_SQADataSQLite*)_pData)
SQASqlite::SQASqlite() 
{
	m_options =  0;
	_pData = new _SQADataSQLite();
	 
	m_dataset = new AQDataSetSqlite();
	SQASqlite::Reset();
}

//------------------------------------------------------------------------------------------------
SQASqlite::~SQASqlite()
{

	if(_pData) delete My_pData; _pData = 0;
	if(m_dataset) delete m_dataset;m_dataset=0;
}


//------------------------------------------------------------------------------------------------
void SQASqlite::Reset()
{
 	m_options = 0; 
 	m_commandTimeout = 600;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetCommandText(const wchar_t* iCmd)
{
	 
	My_pData->setCommand(iCmd);
	 
	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetCommandText(const char* iCmd)
{
	if (!iCmd)
		iCmd = "";
	
	return SetCommandText((wchar_t*)_bstr_t(iCmd));
	//TO DO should we use UTF8 code page?
	//return SetCommandText(m_cUStr.ToUnicode(iCmd));
}



const wchar_t* SQASqlite::GetCommandText()
{
	return My_pData->getCommand();
	 
	
}

const wchar_t* SQASqlite::GetParamtersString()
{
	
	return m_Parameter;
}


//-------------------------------------------------------------------------
int SQASqlite::AddParameter(const wchar_t* iValue, const char* iName, bool output)
{

	 

	return kOK;
}

//-------------------------------------------------------------------------
int SQASqlite::AddParameter(const char* iValue, const char* iName, bool output)
{

 
	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::AddParameter(long iValue, const char* iName, bool output)
{

	 

	return kOK;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::AddParameter(char iValue, const char* iName, bool output)
{

	 

	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::AddParameter(short iValue, const char* iName, bool output)
{
 
	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::AddParameter(double iValue, const char* iName, bool output)
{

	 
	 return kOK;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::AddParameter(float iValue, const char* iName, bool output)
{
 

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::AddParameter(__int64 iValue, const char* iName, bool output)
{
	 

	return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(__int64 iValue, long iIdx, const char* iName)
{
	 

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(const wchar_t* iValue, long iIdx, const char* iName)
{
	 

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(const char* iValue, long iIdx, const char* iName)
{
 
	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(long iValue, long iIdx, const char* iName)
{
	 

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(char iValue, long iIdx, const char* iName)
{
	 
	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(short iValue, long iIdx, const char* iName)
{
	 
	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(double iValue, long iIdx, const char* iName)
{
	 

	 return kOK;
}


//------------------------------------------------------------------------------------------------
int SQASqlite::SetParameter(float iValue, long iIdx, const char* iName)
{
	 

	 return kOK;
}

int SQASqlite::GetRecordPostion() 
{
	return 0;
};

//------------------------------------------------------------------------------------------------
int SQASqlite::GetRecordCount()
{
	return m_dataset->m_dataset.size();
	return 0;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::GetIndex()
{
	return 0;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::SetIndex(int iInd)
{
	return 0;
}

//------------------------------------------------------------------------------------------------
int	SQASqlite::MoveFirst()
{
	m_dataset->m_curRow = 0;
	m_dataset->m_curCol = 0;
	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQASqlite::MoveLast()
{
	

	return kOK;
}


//------------------------------------------------------------------------------------------------
int	SQASqlite::MovePrevious()
{
	

	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQASqlite::MoveNext()
{
	m_dataset->m_curRow++;
	m_dataset->m_curCol = 0;
	 
	return kOK;
}

//------------------------------------------------------------------------------------------------
int	SQASqlite::Move(long iNumber)
{
	

	return kOK;
}

//------------------------------------------------------------------------------------------------
int SQASqlite::NextRecordset()
{
	
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
int SQASqlite::GetFieldCount()
{
	int count = 0;
	
	return count;
}

//------------------------------------------------------------------------------------------------
// skip the data. advance the data index
//
void SQASqlite::SkipData()
{
	
}

//------------------------------------------------------------------------------------------------
// Get Data as std::string
//
int SQASqlite::getDataST(AqUString& oName, AqUString& oValue, int index)
{
	

	return 0;
}

//------------------------------------------------------------------------------------------------
// Get Data unicode string
//
const wchar_t* SQASqlite::getDataUS(int index)
{
	std::string std_str = getStringData( index);
	if(std_str.size()<1) return 0;

	AqString str_temp(std_str.c_str());
	//str_temp.Format("%S", std_str.c_str());;
	m_cUStr.ConvertUTF8( str_temp );;
	return m_cUStr;
}

std::string SQASqlite::getStringData(int index)
{
	std::string ret_str ;
	if(index<0){
		index = m_dataset->m_curCol ;
	}
	m_dataset->m_curCol = index+1;

	
	QARecordData cur_record = m_dataset->m_dataset[m_dataset->m_curRow];

	if(index>=cur_record.size()){
		GetAqLogger()->LogMessage("ERROR: -SQASqlite::getDataInt index %d > dataset size\n",index );				
	}else{
		
#if 0
		std::string  col_name = m_dataset->m_colNames[index];
		QARecordData::iterator it = cur_record.begin();
		for(;it!=cur_record.end();it++){
			if(it->first == col_name){
				break;
			}
			;
		}

		ret_str = it->second;
#else
		ret_str = cur_record[index];
#endif
		 
	}

	return ret_str;
}
 

//------------------------------------------------------------------------------------------------
// Get Data as double for date
//

double SQASqlite::getDataDate(int index)
{
	return 0;

}



//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

int SQASqlite::getDataInt(int index)
{
	int ret_i = 0;
	std::string std_str = getStringData( index);
	if(std_str.size()<1) return 0;

	ret_i = atoi(std_str.c_str());
	 
	return ret_i;

}

__int64 SQASqlite::getDataInt64(int index)
{
	return 0;

}

//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

void SQASqlite::getDataInt(char* oNum, int size, int index)
{
 return  ;

}

//------------------------------------------------------------------------------------------------
// Get Data Double                                 
//

double SQASqlite::getDataD(int index)
{
	return 0;

}

static char* OutResourceStr = "has been chosen as the deadlock victim";
int SQASqlite::ExecuteBegin(SQAParam * param)
{
 
	int ret_v = kFailedUnknown;
	try{
		ret_v = My_pData->ExecuteBegin(m_dataset);
		m_dataset->m_curRow = 0;
		m_dataset->m_curCol = 0;
	}catch(...)
	{
		ret_v = kFailedUnknown;
	}
	return ret_v;
 
}



 int SQASqlite::ExecuteProcBegin(SQAParam * param)  //2012/04/17 K.Ko
{
#if 0
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
#else
	return kFailedUnknown;
#endif
}

//2012/04/17 K.Ko
int SQASqlite::GetParamtersCount()
{
#if 0
	long pNum = -1;
	try
	{
	  pNum = My_pData->m_pCommand->Parameters->GetCount();
	}
	catch( ... )
	{
		 
	}
	return pNum;
#else
	return -1;
#endif
}
AqUString SQASqlite::GetParamtersString(int index)
{
#if 0
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
#else
	AqUString returnStr;
	returnStr.Empty();
	return returnStr;
#endif
}