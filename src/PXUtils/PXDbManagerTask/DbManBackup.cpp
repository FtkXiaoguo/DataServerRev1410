// DbManBackup.cpp: CDbManBackup クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbManBackup.h"

#include "rtvloadoption.h"

#include "PxNetDB.h"

#include "AqCore/TRLogger.h"
#include "AqCore/TRPlatform.h"

#include "AppComConfiguration.h"
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDbManBackup::CDbManBackup()
{
	m_backupEmptyStudy = false;
	//m_configName = "C:\\AQNetConfig\\PXDbBackup.cfg";
	m_backupDestFolder = "C:\\PxDBBack";

	std::string configRootPath = "" ; 
  
	//#7 2012/03/08 K.Ko
	AppComConfiguration::GetConfigRootPath(configRootPath) ;
 
	m_configName  = configRootPath+"PXDbBackup.cfg";

	std::string LogRootPath ;
	AppComConfiguration::GetLogFilesLocation(LogRootPath);
	TRPlatform::MakeDirIfNeedTo(LogRootPath.c_str());

	m_logFileName = LogRootPath+"PXDbBackup.log";

	//
	m_BackupGenerationNumber = 0;
	for(int i=0;i<BACKUP_GENERATIONS;i++){
		m_BackupGenerationAgeDays[i] = 0;
	}
}

CDbManBackup::~CDbManBackup()
{

}
void CDbManBackup::addDbName(std::string DbName)
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
static char _backupDestFolder[128]={0,};	 

#define DBNAME_MAX (16)
static char _DbName[DBNAME_MAX][128] ;
static char _DbName_entry[DBNAME_MAX][128] ;
	 	 
static char _Generation_entry[DBNAME_MAX][128] ;

static char _temp_str[128];
bool CDbManBackup::loadConfiguration(const char *fileName)
{
	int i;
	iRTVOptions cstore_opt;

	for(i=0;i<DBNAME_MAX;i++){
		_DbName[i][0] = 0;
		sprintf(_DbName_entry[i],"DbName%d",i+1);
	}
	cstore_opt.Add("LogFileName",_LogFileName,sizeof(_LogFileName));
	cstore_opt.Add("BackupDestFolder",_backupDestFolder,sizeof(_backupDestFolder));
	 
	for(i=0;i<DBNAME_MAX;i++){
		cstore_opt.Add(_DbName_entry[i],_DbName[i],sizeof(_DbName[i]));
	}
///
	cstore_opt.Add("GenerationNumber",&m_BackupGenerationNumber,sizeof(m_BackupGenerationNumber));
	
	for( i=0;i<BACKUP_GENERATIONS;i++){
		sprintf(_Generation_entry[i],"ageDays%d",i+1);
		cstore_opt.Add(_Generation_entry[i],&(m_BackupGenerationAgeDays[i]),sizeof(m_BackupGenerationAgeDays[i]));
	}


	if(!cstore_opt.Load(fileName)){
		return false;
	}

	if(strlen(_LogFileName) == 0){
		setLogFileName(m_logFileName);
	}else{
		setLogFileName(_LogFileName);
	}

	if(strlen(_backupDestFolder) >0){
		 m_backupDestFolder = _backupDestFolder;
	}

	for(i=0;i<DBNAME_MAX;i++){
		if(strlen(_DbName[i]) > 0){
			 addDbName(_DbName[i]);
		}
	}

	return true;
}

std::string CDbManBackup::getBackupDbName(std::string dbName)
{
	return m_backupDestFolder + std::string("\\") + dbName + std::string(".dat");
}
bool CDbManBackup::renameFileName(const std::string srcFileName, const std::string destFileName)
{
	if (TRPlatform::access(srcFileName.c_str(),TRPlatform::F_OK)==0){
	//src file exisitence

		//try to delete dest file
		if (TRPlatform::access(destFileName.c_str(),TRPlatform::F_OK)==0){
			if(remove(destFileName.c_str()))
			{
				getLogger()->LogMessage("ERROR cano not delete %s \n" ,destFileName.c_str() );
 				getLogger()->FlushLog();
				return false;
			}
		 
		}
		//do it
		if(TRPlatform::rename(srcFileName.c_str(), destFileName.c_str()) <0){
			getLogger()->LogMessage("ERROR cano not rename %s to %s \n" ,
				srcFileName.c_str(),
				destFileName.c_str());
 			getLogger()->FlushLog();
			return false;
		};
	}
	return true;
}

bool CDbManBackup::backupOneDb(const std::string dbName)
{
	std::string backupDbDat			= getBackupDbName(dbName);
	
	std::string backupDbDatTemp     = backupDbDat + std::string(".tmp");
	std::string backup_backupDbDat  = backupDbDat + std::string(".bak");
	
	//back databas to xxx.dat.tmp  at first
	bool ret_b =  backDB( dbName,backupDbDatTemp);
	if(!ret_b){
		return false;
	}
	//rename the old xxx.dat to xxx.dat.bak
//	if(!renameFileName(backupDbDat,backup_backupDbDat)){
	if(!shiftGenerationFile(backupDbDat,0/*genNum=0 start*/)){
		return false;
	}
	//rename xxx.dat.tmp to xxx.dat , finally
	if(!renameFileName(backupDbDatTemp,backupDbDat)){
		return false;
	}

	return ret_b;
}
/*
*  古いデータの世代管理
*/
bool CDbManBackup::shiftGenerationFile(const std::string srcFileName,int genNum)
{

	/*
	*  shift data :  genNum世代 -> (genNum+1)世代
	*/
	char _str_buff_[8];
	bool ret_b = true;
 
	/*
	* 先に (genNum+1)世代 -> (genNum+2)世代　、必要があればバックアップ
	*/
	if(genNum<(m_BackupGenerationNumber-1)){
		//Recursion call,  the generation number is not so big.
		ret_b = shiftGenerationFile( srcFileName, genNum+1);
	}
	if(!ret_b) return false;
//	if(genNum<1){
//		return true;//the recursion end
//	}
	

	//
	//genNum世代
	//
	sprintf(_str_buff_, ".bak%d",genNum);
	std::string backupdat1  = srcFileName + std::string(_str_buff_);
	if(genNum == 0){
		//original
		backupdat1  = srcFileName ;
	}

	//
	//(genNum+1)世代
	//
	sprintf(_str_buff_, ".bak%d",genNum+1);
	std::string backupdat2  = srcFileName + std::string(_str_buff_);
 
	

//	if(genNum <2){
//		backupdat1  = srcFileName ;
//	}
	//generation shift   backupdat1->backupdat2
	if(needBackupGeneration(backupdat2,m_BackupGenerationAgeDays[genNum])){
		ret_b = renameFileName(backupdat1,backupdat2);
	}
	return ret_b;
 
}
bool CDbManBackup::needBackupGeneration(const std::string backupfile,int ageDays)
{
	if (TRPlatform::access(backupfile.c_str(),TRPlatform::F_OK)!=0){
		return true;
	}
	//
	//file  time
	struct _stat statBuf;
	statBuf.st_mtime = 0;
	::_stat(backupfile.c_str(),&statBuf);
	
	int ageMinues =  (int)((time(0) - statBuf.st_mtime)/60.0) ;
#if 1
 	int age = (int)(ageMinues/60.0/24.0);
#else
#pragma message( "### CDbManBackup::needBackupGeneration JUST TEST age for Minues" )
	int age = ageMinues;
#endif
	return (age>=ageDays);
}
bool CDbManBackup::doDbManTask()
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
	 
	if(!findValidData(m_backupEmptyStudy /*ignoreStudyCheck*/)){
		getLogger()->LogMessage(" --- no  data \n"  );
 		getLogger()->FlushLog();
		return false;
	}
	 
	//

	getLogger()->LogMessage(" --- backup the database \n"  );
 	getLogger()->FlushLog();

	{
		if ( TRPlatform::MakeDirIfNeedTo(m_backupDestFolder.c_str()) != 0 )
		{
			getLogger()->LogMessage("ERROR:[C0010] can not make folder %s \n" ,m_backupDestFolder.c_str() );
 			getLogger()->FlushLog();
			return false;
		}
	}

//	backupOldBackupDat();

	//step -2
//	cmd_str = "backupDBBackupData.bat";

	bool ret_b = true;
	int size = m_DbNameList.size();
	for(int i=0;i<size;i++){
		if(!backupOneDb(m_DbNameList[i])){

			getLogger()->LogMessage("ERROR:[C0006] --- backup  database %s failed \n" ,m_DbNameList[i] );
 			getLogger()->FlushLog();
			
			ret_b = ret_b&&false;

		}else{
			 ;
		}
	}

	getLogger()->LogMessage(" --- backup  database successful \n"  );
 	getLogger()->FlushLog();

	return ret_b;
}
bool CDbManBackup::backDB(const std::string dbName,const std::string backName)
{
	int retcd;


	CPxDcmDB db;

	SQA sqa ;

	AqString strSQL;

	//backup database
	strSQL.Format(" BACKUP DATABASE  %s  TO DISK = '%s' WITH INIT ",
						dbName.c_str(),
						backName.c_str()
						);
	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	retcd = db.SQLExecuteBegin(sqa); // do AsyncExecute
	if(retcd != kOK) {
		getLogger()->LogMessage("ERROR:[C0008] *** run SQL error: can not BACKUP DATABASE %s to %s  *** \n",dbName.c_str(),backName.c_str() );
 		getLogger()->FlushLog();
		 
	}

	db.SQLExecuteEnd(sqa);	

	return retcd == kOK;
}
void CDbManBackup::ParseCommandLine(int argc, char** argv)
{
	CDbManBase::ParseCommandLine(argc, argv) ;
}
int CDbManBackup::doMain(int argc, char** argv)
{

	ParseCommandLine(argc, argv);

	if(skipRun()){
		getLogger()->LogMessage(" skip BACKUP DATABAS \n" );
 		getLogger()->FlushLog();
		return 0;
	}

	if(!loadConfiguration(m_configName.c_str())){
		getLogger()->LogMessage("ERROR:[C0009] *** loadConfiguration %s error *** \n",m_configName.c_str() );
 		getLogger()->FlushLog();
		return -1;
	}
 
	getLogger()->LogMessage(" \n");
	getLogger()->LogMessage(" === BACKUP DATABASE === \n");
 	getLogger()->FlushLog();

	//
//	setLogFileName("dbbacup.log");

//	setBackupDestFolder("C:\\AQNetDBBack");
//	addDbName("AqNETDB");
//	addDbName("AqNETHistDB");

	setDbServerType(m_DbServType);

//	bool open_flag = tryOpenDB();
//	bool data_flag = findValidData();	

	bool do_flag = doDbManTask();

	return do_flag;
}
int CDbManBackup::initMain(int argc, char** argv)
{

	ParseCommandLine(argc, argv);

	if(skipRun()){
		getLogger()->LogMessage(" skip BACKUP DATABAS \n" );
 		getLogger()->FlushLog();
		return 0;
	}

	if(!loadConfiguration(m_configName.c_str())){
		getLogger()->LogMessage("ERROR:[C0009] *** loadConfiguration %s error *** \n",m_configName.c_str() );
 		getLogger()->FlushLog();
		return -1;
	}
 
	getLogger()->LogMessage(" \n");
	getLogger()->LogMessage(" === BACKUP DATABASE === \n");
 	getLogger()->FlushLog();

	//
//	setLogFileName("dbbacup.log");

//	setBackupDestFolder("C:\\AQNetDBBack");
//	addDbName("AqNETDB");
//	addDbName("AqNETHistDB");

	setDbServerType(m_DbServType);

//	bool open_flag = tryOpenDB();
//	bool data_flag = findValidData();	

//	bool do_flag = doDbManTask();

	return 0;
}