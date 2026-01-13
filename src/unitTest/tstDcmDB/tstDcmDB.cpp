/***********************************************************************
 * TestDBDaemon.cpp
 
 *
 *-------------------------------------------------------------------
 */
#define _WIN32_DCOM


#pragma warning (disable: 4503)
#include <io.h>

 
#include "PxNetDB.h"
 #include "AqCore/TRLogger.h"
 TRLogger gLogger;

int ServerMain (int argc, char** argv);
int main(int argc, char *argv[])
{

	ServerMain ( argc, argv);
    return 0;
}


 
 CPxDcmDB g_db;
 void testRead();
void testWrite();
int ServerMain (int argc, char** argv)
{
	 
 gLogger.SetLogFile("C:\\AQNetLog\\tstDBInit.log");
 gLogger.SetLogLevel(3);
  
 gLogger.LogMessage("---- start ServerMain ---- \n" );
 gLogger.FlushLog();
 
 SetAqLogger(&gLogger);
 //::CoInitialize( NULL);
 
	printf(" test DB InitDatabaseInfo ...\n");
::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // for database to work in multi-thread mode

bool testSQLite = false;
	 
	if(testSQLite){
		g_db.setupGlobalDBType(kDBType_SQLite);

		CPxDcmDB::InitDBServerName("F:\\PXDcmServer\\src\\PxDbLib\\SQLite\\PxDcmDB.db");

		if(!g_db.InitDatabaseInfo())
		{
			gLogger.FlushLog();
			printf("** DB::InitDatabaseInfo failed   \n" );
		 
			return 0;
		}else{
			gLogger.FlushLog();
			printf("** DB::InitDatabaseInfo OK   \n" );
		}

	}else{
		{
 

			TCHAR  MyComputerName[256];
			unsigned long nSize = sizeof(MyComputerName)-1 ;
			GetComputerName(MyComputerName, &nSize);
			std::string dbServerNameTemp = MyComputerName;
  // #813: SQL Server 2000‚Æ2005‚É—¼‘Î‰ž‚·‚é
			dbServerNameTemp = dbServerNameTemp + DBCore::GetSQLServerDBNameExt();
 

			printf(" DB sever name %s \n",dbServerNameTemp.c_str());
			CPxDcmDB::InitDBServerName(dbServerNameTemp.c_str());

			if(!g_db.InitDatabaseInfo())
			{
				gLogger.FlushLog();
				printf("** DB::InitDatabaseInfo failed   \n" );
			 
				return 0;
			}else{
				gLogger.FlushLog();
				printf("** DB::InitDatabaseInfo OK   \n" );
			}

		} 
   }
	
	{
		AqString strSQL;
		strSQL.Format("SELECT MajorVersion FROM dbinfo" );

		int oVal;
		g_db.SQLGetInt(strSQL, oVal);
	}
 
	testRead();
 
	return 0;
}

void testRead()
{
	SQA sqa;
	sqa.SetCommandText(
		L" SELECT * FROM organization "
		L" WHERE Name = ?) ");

	sqa.AddParameter("yyy");
 
	int retcd = g_db.SQLExecuteBegin(sqa);
	if(retcd != kOK)  {
		return ;
	}
		 

	int size = sqa.GetRecordCount(); 
	 

	retcd = sqa.MoveFirst(); 
	if(retcd != kOK) { 
		return ;
	}

	size--;
 
	int index = 0;
	retcd = sqa.MoveNext();
	while( retcd == kOK && index < size )
	{
	 
		int ID;
		SQL_GET_INT(ID, sqa);
		char Name[100];
		SQL_GET_STR(Name, sqa);
		char Address[100];
		SQL_GET_STR(Address, sqa);
	 
		char Fax[100];
		SQL_GET_STR(Fax, sqa);
		char Desc[100];
		SQL_GET_STR(Desc, sqa);
	 
	 
		retcd = sqa.MoveNext();
	}
	g_db.SQLExecuteEnd(sqa);	 

}