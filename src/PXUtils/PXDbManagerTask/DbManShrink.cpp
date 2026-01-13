// DbManBackup.cpp: CDbManShrink クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbManShrink.h"

#include "rtvloadoption.h"

#include "PxNetDB.h"

#include "AqCore/TRLogger.h"
#include "AqCore/TRPlatform.h"

#include "AppComConfiguration.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDbManShrink::CDbManShrink()
{
 

	std::string configRootPath = "" ; 
  
	//#7 2012/03/08 K.Ko
	AppComConfiguration::GetConfigRootPath(configRootPath) ;
 
	m_configName  = configRootPath+"PXDbShrink.cfg";

	std::string LogRootPath ;
	AppComConfiguration::GetLogFilesLocation(LogRootPath);
	TRPlatform::MakeDirIfNeedTo(LogRootPath.c_str());

	m_logFileName = LogRootPath+"PXDbShrink.log";

	//
 
}

CDbManShrink::~CDbManShrink()
{

}
void CDbManShrink::addDbName(std::string DbName)
{
	int size = m_DbNameList.size();
	for(int i=0;i<size;i++){
		if(m_DbNameList[i] == DbName){
			return;
		}
	}
	m_DbNameList.push_back(DbName);
}
//
static char _LogFileName[128]={0,};	 
 
#define DBNAME_MAX (16)
static char _DbName[DBNAME_MAX][128] ;
static char _DbName_entry[DBNAME_MAX][128] ;
	 	 
 
static char _temp_str[128];
bool CDbManShrink::loadConfiguration(const char *fileName)
{
	int i;
	iRTVOptions cstore_opt;

	for(i=0;i<DBNAME_MAX;i++){
		_DbName[i][0] = 0;
		sprintf(_DbName_entry[i],"DbName%d",i+1);
	}
	cstore_opt.Add("LogFileName",_LogFileName,sizeof(_LogFileName));
 	 
	for(i=0;i<DBNAME_MAX;i++){
		cstore_opt.Add(_DbName_entry[i],_DbName[i],sizeof(_DbName[i]));
	}
///
 

	if(!cstore_opt.Load(fileName)){
		return false;
	}

	if(strlen(_LogFileName) == 0){
		setLogFileName(m_logFileName);
	}else{
		setLogFileName(_LogFileName);
	}

	 
	for(i=0;i<DBNAME_MAX;i++){
		if(strlen(_DbName[i]) > 0){
			 addDbName(_DbName[i]);
		}
	}

	return true;
}

  

bool CDbManShrink::shrinkOneDb(const std::string dbName)
{
 
	bool ret_b =  shrinkDB( dbName );
	if(!ret_b){
		return false;
	}
	return true;
 
}
 
bool CDbManShrink::doDbManTask()
{

	getLogger()->LogMessage(" --- check the database  \n"  );
 	getLogger()->FlushLog();

	std::string cmd_str;
	if(!tryOpenDB()){
		getLogger()->LogMessage("ERROR:[C0010] OpenDB error  \n"  );
 		getLogger()->FlushLog();
		return false;
	}
	//
	 
	if(!findValidData(true /*ignoreStudyCheck*/)){
		getLogger()->LogMessage(" --- no  data \n"  );
 		getLogger()->FlushLog();
		return false;
	}
	 
	//

	getLogger()->LogMessage(" --- shrink the database \n"  );
 	getLogger()->FlushLog();

	 

	bool ret_b = true;
	int size = m_DbNameList.size();
	for(int i=0;i<size;i++){
		if(!shrinkOneDb(m_DbNameList[i])){

			getLogger()->LogMessage("ERROR:[C0006] --- shrink  database %s failed \n" ,m_DbNameList[i] );
 			getLogger()->FlushLog();
			
			ret_b = ret_b&&false;

		}else{
			 ;
		}
	}

	getLogger()->LogMessage(" --- shrink  database successful \n"  );
 	getLogger()->FlushLog();

	return ret_b;
}
bool CDbManShrink::shrinkDB(const std::string dbName)
{
	bool ret_b = true;
	int retcd;


	getLogger()->LogMessage(" --- shrink  database %s \n",dbName.c_str() );
 	getLogger()->FlushLog();

	CPxDcmDB db;

	SQA sqa ;

	AqString strSQL;

	//backup database
	strSQL.Format(" DBCC SHRINKDATABASE ( %s )  ",
						dbName.c_str() 
						 
						);
	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	retcd = db.SQLExecuteBegin(sqa); // do AsyncExecute
	if(retcd != kOK) {
		getLogger()->LogMessage("ERROR:[C0008] *** run SQL error: can not SHRINK DATABASE %s  *** \n",dbName.c_str() );
 		getLogger()->FlushLog();
		ret_b = false;
		 
	}else{
static char _str_buff[1024*256];
		char *p_str_temp = _str_buff;
		int DbId,FileID,CurrentSize,MinimumSize,UsedPates,EstimatedPagaes;
		int size = sqa.GetRecordCount(); 
		int col_nn = sqa.GetFieldCount();
		if((size >0) && (col_nn>=6)) {
			sprintf(p_str_temp,"\n %8s, %8s, %16s, %16s, %16s, %16s \n", 
									"DbId","FileID","CurrentSize","MinimumSize","UsedPates","EstimatedPagaes");
  
			p_str_temp = p_str_temp + strlen(p_str_temp);
			int index = 0;
			retcd = sqa.MoveFirst(); 
			if(retcd == kOK)  {
				while( retcd == kOK && index < size )
				{
				 
					SQL_GET_INT(DbId, sqa);
					SQL_GET_INT(FileID, sqa);
					SQL_GET_INT(CurrentSize, sqa);
					SQL_GET_INT(MinimumSize, sqa);
					SQL_GET_INT(UsedPates, sqa);
					SQL_GET_INT(EstimatedPagaes, sqa);
					retcd = sqa.MoveNext();
					index++;
					//
					sprintf(p_str_temp ," %8d, %8d, %16d, %18d, %18d, %18d \n", 
									DbId,FileID,CurrentSize,MinimumSize,UsedPates,EstimatedPagaes);
 					p_str_temp = p_str_temp + strlen(p_str_temp);
				}
			}
			getLogger()->LogMessage( "%s\n",_str_buff );
 			getLogger()->FlushLog();
		}

	}

	db.SQLExecuteEnd(sqa);	

	getLogger()->LogMessage(" --- shrink  database %s -- end \n",dbName.c_str() );
 	getLogger()->FlushLog();

	return ret_b;
}
void CDbManShrink::ParseCommandLine(int argc, char** argv)
{
	CDbManBase::ParseCommandLine(argc, argv) ;
}
int CDbManShrink::doMain(int argc, char** argv)
{

	ParseCommandLine(argc, argv);

	if(skipRun()){
		getLogger()->LogMessage(" skip SHRINK DATABAS \n" );
 		getLogger()->FlushLog();
		return 0;
	}

	if(!loadConfiguration(m_configName.c_str())){
		getLogger()->LogMessage("ERROR:[C0009] *** loadConfiguration %s error *** \n",m_configName.c_str() );
 		getLogger()->FlushLog();
		return -1;
	}
 
	getLogger()->LogMessage(" \n");
	getLogger()->LogMessage(" === SHRINK DATABASE === \n");
 	getLogger()->FlushLog();

	//


	setDbServerType(m_DbServType);


	bool do_flag = doDbManTask();

	return do_flag;
}
int CDbManShrink::initMain(int argc, char** argv)
{

	ParseCommandLine(argc, argv);

	if(skipRun()){
		getLogger()->LogMessage(" skip SHRINK DATABAS \n" );
 		getLogger()->FlushLog();
		return 0;
	}

	if(!loadConfiguration(m_configName.c_str())){
		getLogger()->LogMessage("ERROR:[C0009] *** loadConfiguration %s error *** \n",m_configName.c_str() );
 		getLogger()->FlushLog();
		return -1;
	}
 
	getLogger()->LogMessage(" \n");
	getLogger()->LogMessage(" === SHRINK DATABASE === \n");
 	getLogger()->FlushLog();



	setDbServerType(m_DbServType);



	return 0;
}