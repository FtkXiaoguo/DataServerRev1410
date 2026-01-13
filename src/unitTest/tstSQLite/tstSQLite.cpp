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
#include "AQNetConfiguration.h"

 
int ServerMain (int argc, char** argv);
std::string g_db_file_name;
int main(int argc, char *argv[])
{

 // 	CPxODBC pxODBC;
 // 	pxODBC.doTest();

	if(argc>1){
		g_db_file_name = argv[1];
	}
	ServerMain ( argc, argv);
    return 0;
}
  
 static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};

void getHomeFromModulePath()
{
  char Path[2*MAX_PATH+1]; 

  if(0!=GetModuleFileName( NULL, Path, 2*MAX_PATH )){// 実行ファイルの完全パスを取得

	 std::string str_temp = Path;
	 std::string key= "/\\";
	int pos = str_temp.find_last_of(key);
	std::string sub_str_temp = str_temp.substr(0,pos);
	//up folder
	pos = sub_str_temp.find_last_of(key);
	sub_str_temp = sub_str_temp.substr(0,pos+1);
	strcpy(gPxDcmServerHomeBuff,sub_str_temp.c_str());

  }else{
	  gPxDcmServerHomeBuff[0] = 0;
  }
}
std::string queue_db_fileName;

 

void testDB1();
int ServerMain (int argc, char** argv)
{
	 
 gLogger.SetLogFile(".\\tstQueueDB.log");
 gLogger.SetLogLevel(0);
  
 gLogger.LogMessage("---- start ServerMain ---- \n" );
 gLogger.FlushLog();
 
 SetAqLogger(&gLogger);
 //::CoInitialize( NULL);
   
 testDB1();

	return 0;
}
void writeRecord1(CPxDB &queue_db);
void readRecord1(CPxDB &queue_db);
void testDB1()
{
	AqCOMThreadInit comInitGuard; // for CoInitializeEx

	CPxDB queue_db;
	queue_db.setupLocaleDBType(kDBType_SQLite);

	AqUString dbname_str;
	dbname_str.Format(L"%S","test.db");
	queue_db.SetMyDBName(dbname_str);
	//
	unsigned int start_time = ::GetTickCount();
	for(int i=0;i<50;i++){
  //		writeRecord1(queue_db);
 		readRecord1(queue_db);
	}
	unsigned int end_time = ::GetTickCount();
	printf(" spent time %.2f \n",(end_time-start_time)/1000.0);

}

void writeRecord1(CPxDB &queue_db)
{
	int i;
	char str_buff[512];
	AqString	strSQL;

	int test_nn = 10;
	std::string test_fd;
	for( i=0;i<test_nn;i++){
		sprintf(str_buff,",test%d",i+1);
		test_fd =  test_fd + str_buff;
	}
	std::string test_val;
	memset(str_buff,'c',512);
	for( i=0;i<test_nn;i++){
		sprintf(str_buff+200,"%d",i);
		test_val =  test_val + std::string(",'")+str_buff + std::string("'");
	}

	std::string testInt_fd;
	for( i=0;i<test_nn;i++){
		sprintf(str_buff,",testInt%d",i+1);
		testInt_fd =  testInt_fd + str_buff;
	}
	//
	std::string testInt_val;
	for( i=0;i<test_nn;i++){
		sprintf(str_buff,",%d",i);
		testInt_val =  testInt_val + str_buff;
	}

	strSQL.Format( "INSERT INTO sendQueue ( "
				"JobID, SendLevel, Status " 
				"%s "
				"%s "
				")"
				"VALUES"
				"("
				" 1 , 1, 2"
				" %s"
				" %s"
				" )"
				,
				test_fd.c_str(),
				testInt_fd.c_str(),
				test_val.c_str(),
				testInt_val.c_str()
				);


	SQA sqa(queue_db.getDBType());
	sqa.SetCommandText(strSQL);
	int retcd = queue_db.SQLExecuteBegin(sqa);

	queue_db.SQLExecuteEnd(sqa);

}


void readRecord1(CPxDB &queue_db)
{
	int i;
	char str_buff[512];
	AqString	strSQL;

	int id_len = 10;
	std::string test_fd;
	for( i=0;i<id_len;i++){
		sprintf(str_buff,",test%d",i+1);
		test_fd =  test_fd + str_buff;
	}
	std::string test_val;
	memset(str_buff,'c',512);
	for( i=0;i<id_len;i++){
		sprintf(str_buff+200,"%d",i);
		test_val =  test_val + std::string(",'")+str_buff + std::string("'");
	}

	std::string testInt_fd;
	for( i=0;i<id_len;i++){
		sprintf(str_buff,",testInt%d",i+1);
		testInt_fd =  testInt_fd + str_buff;
	}
	//
	std::string testInt_val;
	for( i=0;i<id_len;i++){
		sprintf(str_buff,",%d",i);
		testInt_val =  testInt_val + str_buff;
	}

	strSQL.Format( " SELECT "
				"JobID, SendLevel, Status " 
				"%s "
				"%s "
				 
				"FROM sendQueue"
				,
				test_fd.c_str(),
				testInt_fd.c_str()
			 
				);


	SQA sqa(queue_db.getDBType());
	sqa.SetCommandText(strSQL);
	int retcd = queue_db.SQLExecuteBegin(sqa);

	int nn = sqa.GetRecordCount();

	 
	int i_temp;
	char str_temp[1024];
	
	retcd = sqa.MoveFirst(); 
	if(retcd != kOK) {
	 
		printf(" MoveFirst failed \n");
		return ;
		 
	} 

	for( i=0; i<nn;i++){
		SQL_GET_INT(i_temp, sqa);
		SQL_GET_INT(i_temp, sqa);
		SQL_GET_INT(i_temp, sqa);
		//
		{
			for(int id_i=0;id_i<id_len;id_i++){
				SQL_GET_STR(str_temp, sqa);
			}
		}
		//
		{
			for(int id_i=0;id_i<id_len;id_i++){
				SQL_GET_INT(i_temp, sqa);
			}
		}
		//
		retcd = sqa.MoveNext(); 
		if(retcd != kOK) {
	 
			printf(" MoveFirst failed \n");
			return ;
		 
		} 
	}

	queue_db.SQLExecuteEnd(sqa);

}