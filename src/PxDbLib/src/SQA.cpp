/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */
 #include "SQAComInf.h"
//#include "AqCore/stdafx.h"
#include "DBCore.h"
#include "AqCore/TRLogger.h"

#include <atlbase.h>
#include <OledbErr.h>


#include "SQAADO.h"
#include "SQASqlite.h"


#if 1

#ifndef ADO_IMPORT
#define ADO_IMPORT
#pragma warning (disable: 4146)
#import "msadox.dll" rename_namespace("ADOX") rename("EOF", "EndOfFile")
#import "msado15.dll" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
using namespace ADOCG;
#endif

#endif

#include "SQAData.h"

 #include "CheckMemoryLeak.h"
 
static int _glb_DBType_ = kDBType_MSSQL;
//------------------------------------------------------------------------------------------------
void SQA::setDefaultDbType(int DBType) //2012/05/14
{
	_glb_DBType_ = DBType;
}
//------------------------------------------------------------------------------------------------
SQA::SQA(int DBType) 
{
	if(DBType == kDBType_Default){
		DBType = _glb_DBType_;
	}

	m_DBType = DBType;

	switch(DBType){ // add SQLite 2011/09/08 K.Ko
	case kDBType_MSSQL:
	m_SQComInf = new SQAADO();
	 
	break;
	case kDBType_SQLite:
#ifdef USE_SQL_LITE
 	m_SQComInf = new SQASqlite();
#endif
	break;
	}

//	_glb_DBType_ = DBType;
 
}

//------------------------------------------------------------------------------------------------
SQA::~SQA()
{

	if(m_SQComInf) {
		// memory leak 2012/02/20 K.Ko
	//	switch(_glb_DBType_){ 
		switch(m_DBType){
		case kDBType_MSSQL:
		{
			SQAADO *sq_coninf_ptr = (SQAADO*)m_SQComInf;
			delete sq_coninf_ptr;
		}
		break;
		case kDBType_SQLite:
		{
 #ifdef USE_SQL_LITE
			SQASqlite *sq_coninf_ptr = (SQASqlite*)m_SQComInf;
			delete sq_coninf_ptr;
#endif
		 
		}
		break;
		}

	 
		m_SQComInf = 0;
	}
}


//------------------------------------------------------------------------------------------------
void SQA::Reset()
{
	m_SQComInf->Reset();
	 
}


//------------------------------------------------------------------------------------------------
int SQA::SetCommandText(const wchar_t* iCmd)
{
	 return m_SQComInf->SetCommandText( iCmd);
}


//------------------------------------------------------------------------------------------------
int SQA::SetCommandText(const char* iCmd)
{
	return m_SQComInf->SetCommandText( iCmd);
}


//------------------------------------------------------------------------------------------------
int SQA::FormatCommandText(const wchar_t* fmt, ...)
{
	 

	try
	{
		va_list args;
		va_start(args, fmt);
		m_cUStr.VFormat(fmt, args);
		va_end(args);

		SetCommandText(m_cUStr);
//		((_SQADataADO*)_pData)->m_pCommand->CommandText = _bstr_t(m_cUStr);
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
	return m_SQComInf->GetCommandText( );
	 
	
}

const wchar_t* SQA::GetParamtersString()
{ 
	return m_SQComInf->GetParamtersString( );
}


//-------------------------------------------------------------------------
int SQA::AddParameter(const wchar_t* iValue, const char* iName, bool output)
{

	 return m_SQComInf->AddParameter( iValue,  iName,  output);
}

//-------------------------------------------------------------------------
int SQA::AddParameter(const char* iValue, const char* iName, bool output)
{
	return m_SQComInf->AddParameter( iValue,  iName, output);
}


//------------------------------------------------------------------------------------------------
int SQA::AddParameter(long iValue, const char* iName, bool output)
{

	return m_SQComInf->AddParameter( iValue,  iName, output);
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(char iValue, const char* iName, bool output)
{

	return m_SQComInf->AddParameter( iValue,  iName, output);
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(short iValue, const char* iName, bool output)
{
	return m_SQComInf->AddParameter( iValue,  iName, output);
}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(double iValue, const char* iName, bool output)
{

	return m_SQComInf->AddParameter( iValue,  iName, output);

}

//------------------------------------------------------------------------------------------------
int SQA::AddParameter(float iValue, const char* iName, bool output)
{

	return m_SQComInf->AddParameter( iValue,  iName, output);

}


//------------------------------------------------------------------------------------------------
int SQA::AddParameter(__int64 iValue, const char* iName, bool output)
{
	return m_SQComInf->AddParameter( iValue,  iName, output);

}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(__int64 iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);

}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(const wchar_t* iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(const char* iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(long iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(char iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(short iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(double iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}


//------------------------------------------------------------------------------------------------
int SQA::SetParameter(float iValue, long iIdx, const char* iName)
{
	return m_SQComInf->SetParameter( iValue, iIdx, iName);
}

int SQA::GetRecordPostion() {
	return m_SQComInf->GetRecordPostion();
	
};

//------------------------------------------------------------------------------------------------
int SQA::GetRecordCount()
{
	return m_SQComInf->GetRecordCount();
}

//------------------------------------------------------------------------------------------------
int SQA::GetIndex()
{
	return m_SQComInf->GetIndex();
}

//------------------------------------------------------------------------------------------------
int SQA::SetIndex(int iInd)
{
	 return m_SQComInf->SetIndex(iInd);
}

//------------------------------------------------------------------------------------------------
int	SQA::MoveFirst()
{
	 
	 return m_SQComInf->MoveFirst();
}

//------------------------------------------------------------------------------------------------
int	SQA::MoveLast()
{
	return m_SQComInf->MoveLast();
}


//------------------------------------------------------------------------------------------------
int	SQA::MovePrevious()
{
	return m_SQComInf->MovePrevious();
}

//------------------------------------------------------------------------------------------------
int	SQA::MoveNext()
{
	return m_SQComInf->MoveNext();
 
}

//------------------------------------------------------------------------------------------------
int	SQA::Move(long iNumber)
{
	 return m_SQComInf->Move(iNumber);
}

//------------------------------------------------------------------------------------------------
int SQA::NextRecordset()
{
	return m_SQComInf->NextRecordset();
}

//------------------------------------------------------------------------------------------------
//
int SQA::GetFieldCount()
{
	return m_SQComInf->GetFieldCount();
}

//------------------------------------------------------------------------------------------------
// skip the data. advance the data index
//
void SQA::SkipData()
{
	 m_SQComInf->SkipData();
}

//------------------------------------------------------------------------------------------------
// Get Data as std::string
//
int SQA::getDataST(AqUString& oName, AqUString& oValue, int index)
{
	return m_SQComInf->getDataST(oName,  oValue ,index);

}

//------------------------------------------------------------------------------------------------
// Get Data unicode string
//
const wchar_t* SQA::getDataUS(int index)
{
	return m_SQComInf->getDataUS( index);

}

//------------------------------------------------------------------------------------------------
// Get Data as char pointer 
//
const char* SQA::getDataS(int index)
{
	m_cStr.Convert(getDataUS(index), CP_ACP);
	return m_cStr; 
}
//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
const char* SQA::getDataUTF8(int index)
{
	m_cStr.Convert(getDataUS(index), CP_UTF8);
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
//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
void SQA::setDataUTF8(char* iTarget, int iSize, int index)
{
	const char* str = getDataUTF8(index);

	strncpy(iTarget, str, iSize);
	iTarget[iSize - 1] = '\0';
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
	return m_SQComInf->getDataDate( index);

}



//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

int SQA::getDataInt(int index)
{
	 return m_SQComInf->getDataInt( index);
}

__int64 SQA::getDataInt64(int index)
{
	 return m_SQComInf->getDataInt64( index);
}

//------------------------------------------------------------------------------------------------
// Get Data int                                                    
//

void SQA::getDataInt(char* oNum, int size, int index)
{
	 m_SQComInf->getDataInt(  oNum,  size,  index);

}

//------------------------------------------------------------------------------------------------
// Get Data Double                                 
//

double SQA::getDataD(int index)
{
	return m_SQComInf->getDataD( index);
	
}

 _SQAData *SQA::getSQAData()
 {
	 return m_SQComInf->getSQAData( );
 }
	  //
int	SQA::getOptions()
{
	 return m_SQComInf->getOptions( );
}
void SQA::setOptions(int opt)
{
	 m_SQComInf->setOptions( opt);
}
int SQA::getCommandTimeout()
{
	 return m_SQComInf->getCommandTimeout( );
 }
void SQA::setCommandTimeout(int timeout)
{
	m_SQComInf->setCommandTimeout( timeout);
}

int SQA::ExecuteBegin(SQAParam * param)
{
	return m_SQComInf->ExecuteBegin( param);

}

int SQA::ExecuteProcBegin(SQAParam * param)
{
	return m_SQComInf->ExecuteProcBegin( param);

}

//2012/04/17 K.Ko
int SQA::GetParamtersCount(){
	return m_SQComInf->GetParamtersCount();
}
AqUString  SQA::GetParamtersString(int index){
	return m_SQComInf->GetParamtersString(index);
}
	  ////////