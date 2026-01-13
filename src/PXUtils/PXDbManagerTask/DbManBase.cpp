// DbManBase.cpp: CDbManBase クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbManBase.h"

#include "PxNetDB.h"


#include "AqCore/TRLogger.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////


CDbManBase::CDbManBase()
{


}

CDbManBase::~CDbManBase()
{

}
void CDbManBase::ParseCommandLine(int argc, char** argv)
{
	DbManagerTaskBase::ParseCommandLine(argc, argv) ;
}

int CDbManBase::doMain(int argc, char** argv)
{
	ParseCommandLine(argc, argv);

	return 0;
}
 int CDbManBase::initMain(int argc, char** argv) 
 {
	 ParseCommandLine(argc, argv);

	return 0;
 }

bool CDbManBase::findValidData(bool ignoreStudyCheck)
{
	int retcd;
	int size;

	CPxDcmDB db;

	SQA sqa;

	AqString strSQL;

	//check study data list
	strSQL.Format("SELECT * FROM dbinfo");
	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	retcd = db.SQLExecuteBegin(sqa); // do AsyncExecute
	if(retcd != kOK) {
		getLogger()->LogMessage("ERROR:[C0002] *** run SQL error: SELECT * FROM dbinfo *** \n" );
 		getLogger()->FlushLog();
		return false;
	}
	//
	size = sqa.GetRecordCount(); 
	if(size < 1) {
		getLogger()->LogMessage("WARNING:[C0003] ***  Can not find data from  Table dbinfo *** \n" );
 		getLogger()->FlushLog();
		return false;
	}

	if(ignoreStudyCheck) return true;

	//check study data list
	strSQL.Format("SELECT * FROM UserStudyView");
	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	retcd = db.SQLExecuteBegin(sqa); // do AsyncExecute
	if(retcd != kOK) {
		getLogger()->LogMessage("ERROR:[C0004] *** run SQL error:  SELECT * FROM UserStudyView *** \n" );
 		getLogger()->FlushLog();
		return false;
	}
	//
	size = sqa.GetRecordCount(); 
	if(size < 1) {
		getLogger()->LogMessage("WARNING:[C0005] ***  Can not find data from  UserStudyView *** \n" );
 		getLogger()->FlushLog();
		return false;
	}

	return true;
}
bool CDbManBase::tryOpenDB()
{
	AqCOMThreadInit comInitGuard;

	CPxDcmDB db;

#if 1 
//
// AQNetDB::InitDatabaseInfo 内対応、ここは不要 2010/09/15 K.KO
//
	switch(m_DbServType){
	case MSSQL_2000:
		{
			db.InitDBServerName();
		}
		break;

	case MSSQL_2005:
		{
			TCHAR  MyComputerName[256];
			unsigned long nSize = sizeof(MyComputerName)-1 ;
			GetComputerName(MyComputerName, &nSize);
			std::string dbServerNameTemp = MyComputerName;
			dbServerNameTemp = dbServerNameTemp + "\\SQLEXPRESS";
		 
			m_DBServerName = dbServerNameTemp.c_str() ;
			db.InitDBServerName(m_DBServerName.c_str());
		}
		break;
	default:
		break;
	}
#endif

	//
	if(!db.InitDatabaseInfo(false /*iRedo*/, 20 /* iretry */)) //retry 10 x 5Sec
	{
	//	printf(" InitDatabaseInfo error \n");
		
 		getLogger()->LogMessage("ERROR:[C0001] *** db InitDatabaseInfo error*** \n" );
 		getLogger()->FlushLog();
		return false;
	}
 
//	printf("--db connection    \n");
	// turn on connection pool
	db.ConnectionPooling(true);

 //	getLogger()->LogMessage("---- db connection OK ------\n" );
 //	getLogger()->FlushLog();
	return true;
}

std::string CDbManBase::getSQLCmd(bool cmd_option) const
{
	std::string ret_str;

	switch(m_DbServType){
	case MSSQL_2000:
		{
			ret_str = "osql";
			if(cmd_option){
				ret_str += " -b -E -n";
			}
		}
		break;

	case MSSQL_2005:
		{
			ret_str = "sqlcmd -S localhost\\SQLEXPRESS ";
			if(cmd_option){
				ret_str += " -b -E";
			}
		}
		break;
	default:
		break;
	}

	return ret_str;
}

bool CDbManBase::runCmd(const std::string cmd)
{

	try {
		int id = system(cmd.c_str());
 
		if(id != 0){
			getLogger()->LogMessage("ERROR: *** CDbManBase::runCmd %s \n" ,cmd.c_str());
 			getLogger()->FlushLog();
			return false;
		}
	}catch(...)
	{
		getLogger()->LogMessage("ERROR: *** catch exception CDbManBase::runCmd %s \n" ,cmd.c_str());
 		getLogger()->FlushLog();
	}
	return true;
}

 