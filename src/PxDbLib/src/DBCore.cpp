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
#include "SQADataSqlite.h"

//CComModule _Module;


class DBCoreSQAParam: public SQAParam
{
public :
	DBCoreSQAParam (DBCore *db_core) {
		m_db_core = db_core;
	}
	virtual void Progress(int workRemaining, int workCompleted){
		m_db_core->Progress( workRemaining,  workCompleted) ;
	};
protected:
	DBCore *m_db_core;
};

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

///////
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
///

//static TRCriticalSection Update_cs;
static _ConnectionPtr	PermanentConnectionPtr = 0; // set to 0 to turn off pooling
static wchar_t ConectionInfo[256] = L"";
static wchar_t _ConStr[256+20] = L"";

DBType	DBCore::m_GlobalDBType = kDBType_MSSQL; // add SQLite 2011/09/08 K.Ko

const wchar_t* DBCore::GetDBConectionInfo()
{
	return ConectionInfo;
}

////////////////////////
// #15
void DBCore::SetMyDBConectionInfo(const wchar_t *iCinfo)
{
	if(!iCinfo || wcslen(iCinfo) >= 256 || wcsicmp(m_My_ConectionInfo,iCinfo) == 0)
		return; 

	wcscpy(m_My_ConectionInfo, iCinfo);
	wcscpy(m_My_ConStr, iCinfo);
	wcscat(m_My_ConStr, L";OLE DB Services=-1");
}
 
////////////////////////

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

void DBCore::setupGlobalDBType(DBType type)
{
	m_GlobalDBType = type;
	SQA::setDefaultDbType(m_GlobalDBType);
}
void DBCore::setupLocaleDBType(DBType type) 
{
	m_LocaleDBType = type;
}; // add SQLite 2011/09/08 K.Ko
 

//------------------------------------------------------------------------------------------------
DBCore::DBCore() : m_cancelFlag(0)
{
	m_LocaleDBType = kDBType_Default; // add SQLite 2012/05/14 K.Ko

	m_My_ConectionInfo[0] = 0;
	m_My_ConStr[0] = 0;
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
	// #15 add locale ConectionInfo 2012/04/24
	if(m_My_ConStr[0] != 0){
		return  iSqa.getSQAData()->Connect(m_My_ConStr, logErr);
		
	}else{
		return  iSqa.getSQAData()->Connect(_ConStr, logErr);
	}
}

//------------------------------------------------------------------------------------------------
int DBCore::closeDatabase( SQA& iSqa, bool iCommit)
{
	return iSqa.getSQAData()->Disconnect(iCommit);
}


//------------------------------------------------------------------------------------------------
int	DBCore::ExecuteBegin(SQA& iSqa)
{
	int rcode = openDatabase( iSqa);
	if( rcode != kOK ) return kDBOpenError;
 
	// use transaction
	if(!(iSqa.getOptions() & kDBNoTransaction))
	{
		rcode = SQLNewTrans(iSqa, false);
		if( rcode != kOK ) return kDBOpenError;
	}
////
 
	DBCoreSQAParam SQAParam(this);
	int ret_c = iSqa.ExecuteBegin(&SQAParam);
	return ret_c;
//////

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
		if(!(iSqa.getOptions() & kDBNoLogOnIntegrityViolation) )
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
	return iSqa.getSQAData()->Commit(iCommit);
}

//------------------------------------------------------------------------------------------------
int	DBCore::SQLNewTrans( SQA& iSqa, bool iAutoCommitExistOne/*=true*/)
{
	if(getLocalDBType() == kDBType_SQLite){
		;;
	}else{
		if(iSqa.getSQAData()->m_transBegin > 0)
		{
			if(!iAutoCommitExistOne)
				return kOK;	// don't want to end existed session, do nothing 
			else
				iSqa.getSQAData()->Commit(true);
		}
	}

	return iSqa.getSQAData()->NewTrans();
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
	SQA sqa(getLocalDBType());
	sqa.setOptions(kDBExecuteNoRecords|kDBNoTransaction);
	sqa.SetCommandText(iSQLStr);
	if(iCommandTimeout >= 0)
		sqa.setCommandTimeout(iCommandTimeout);

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
	SQA sqa(getLocalDBType());
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


// #88 2016/09/26 by N.Furutsuki
const char* DBCore::GetSQLServerUser()
{
	static char serverUserName[256] = { 0 };

	HKEY hRootKey;
	char szBuff[256] = { 0 };
	DWORD cbBuff;

	// レジストリキーを開きます
	const char keyName[] = "Software\\PreXion\\AQServer";
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS){
		cbBuff = sizeof(szBuff);
		if (RegQueryValueEx(hRootKey, "SQLServerUser", NULL, NULL, (LPBYTE)szBuff, &cbBuff) == ERROR_SUCCESS){
			memset(serverUserName, 0x00, sizeof(serverUserName));
			strncpy(serverUserName, szBuff, sizeof(serverUserName) - 1);
		}

		RegCloseKey(hRootKey);
	}
	return serverUserName;

}

// #88 2016/09/26 by N.Furutsuki
const char* DBCore::GetSQLServerPassword()
{
	static char serverPasswordName[256] = { 0 };

	HKEY hRootKey;
	char szBuff[256] = { 0 };
	DWORD cbBuff;

	// レジストリキーを開きます
	const char keyName[] = "Software\\PreXion\\AQServer";
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS){
		cbBuff = sizeof(szBuff);
		if (RegQueryValueEx(hRootKey, "SQLServerPassword", NULL, NULL, (LPBYTE)szBuff, &cbBuff) == ERROR_SUCCESS){
			memset(serverPasswordName, 0x00, sizeof(serverPasswordName));
			strncpy(serverPasswordName, szBuff, sizeof(serverPasswordName) - 1);
		}

		RegCloseKey(hRootKey);
	}
	return serverPasswordName;

}

int	DBCore::SQLExecuteProcBegin(SQA& iSqa) //2012/04/17 K.Ko
{
	AqLoggerInterface* pLog = GetAqLogger();

	int loglevel = pLog->GetLogLevel();
	if(loglevel >= kDebug)
	{
		AqString cmdStr, paraStr;
		cmdStr.ConvertUTF8(iSqa.GetCommandText());
		paraStr.ConvertUTF8(iSqa.GetParamtersString());

		pLog->LogMessage(kDebug, "Info %d DBCore::SQLExecuteProcBegin with SQL:%s %s\n", GetCurrentThreadId(), cmdStr, paraStr);
	}

	try
	{
		int rcode = ExecuteProcBegin(iSqa);
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
		if(!(iSqa.getOptions() & kDBNoLogOnIntegrityViolation) )
		{
			AqString cmdStr, paraStr;
			cmdStr.ConvertUTF8(iSqa.GetCommandText());
			paraStr.ConvertUTF8(iSqa.GetParamtersString());

			pLog->LogMessage("ERROR %d DBCore::SQLExecuteProcBegin with SQL:%s %s\n", GetCurrentThreadId(), cmdStr, paraStr);
			pLog->FlushLog();
		}
		return kDBException;
	}
}
int	DBCore::ExecuteProcBegin(SQA& iSqa)
{
	int rcode = openDatabase( iSqa);
	if( rcode != kOK ) return kDBOpenError;
 
	// use transaction
	if(!(iSqa.getOptions() & kDBNoTransaction))
	{
		rcode = SQLNewTrans(iSqa, false);
		if( rcode != kOK ) return kDBOpenError;
	}
////
 
	DBCoreSQAParam SQAParam(this);
	int ret_c = iSqa.ExecuteProcBegin(&SQAParam);
	return ret_c;
//////

}

