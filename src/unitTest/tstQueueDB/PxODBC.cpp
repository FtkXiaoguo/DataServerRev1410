#include "PxODBC.h"

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "string"

#pragma comment(lib, "odbc32.lib")

//#include "Adocls/AdoClass.h"

class ODBC_CONT
{
public:
	ODBC_CONT(){
		henv = 0;
		hdbc = 0;
		hstmt = 0;
	}
	HENV	henv;	//環境ハンドル
	HDBC	hdbc;	//接続ハンドル
	HSTMT   hstmt;  //ステートメントハンドル
};

#define ODBC_Hander ((ODBC_CONT*)m_odbcCnt)
CPxODBC::CPxODBC(void)
{
	m_odbcCnt = new ODBC_CONT;
}

CPxODBC::~CPxODBC(void)
{
	if(m_odbcCnt){
		delete ODBC_Hander;
	}
}
bool CPxODBC::opendDB(const char* datasouce,const char*username,const char*pasword)
{
	if(m_odbcCnt){
		delete ODBC_Hander;
	}
	m_odbcCnt = new ODBC_CONT;

	RETCODE	rc;

	//
	//char	ret_connect_str[SQL_MAX_OPTION_STRING_LENGTH];
	//SWORD	ret_len;

	//環境ハンドルを割り振る
	rc	= SQLAllocEnv(&(ODBC_Hander->henv));
	printf("SQLAllocEnv --> %d\n", rc);
	if(rc != 0){
		return false;
	}

	//接続ハンドルを割り振る
	rc	= SQLAllocConnect(ODBC_Hander->henv, &(ODBC_Hander->hdbc));
	printf("SQLAllocConnect --> %d\n", rc);
	if(rc != 0){
		return false;
	}

	{
	 /* Connect to the DSN mydsn */
		SQLCHAR  outstr[256];
		SQLSMALLINT  outstrlen;
	    SQLRETURN ret;
		ret = SQLDriverConnect(ODBC_Hander->hdbc, NULL, (SQLCHAR*)datasouce, SQL_NTS,
				 outstr, sizeof(outstr), &outstrlen,
				 SQL_DRIVER_COMPLETE);
	  if (SQL_SUCCEEDED(ret)) {
		printf("Connected\n");
		printf("Returned connection string was:\n\t%s\n", outstr);
		if (ret == SQL_SUCCESS_WITH_INFO) {
		  printf("Driver reported the following diagnostics\n");
//		  extract_error("SQLDriverConnect", *hdbc, SQL_HANDLE_DBC);
		}
	// 	SQLDisconnect(*hdbc);		/* disconnect from driver */
	  } else {
		fprintf(stderr, "Failed to connect\n");
		return false;
	//	extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
	  }

	  rc = ret;
	}


	//
	rc = SQLAllocStmt(ODBC_Hander->hdbc,&(ODBC_Hander->hstmt));
	printf("SQLAllocStmt --> %d\n", rc);
	if(rc != 0){
		return false;
	}

	return true;
}
bool CPxODBC::closeDB()
{
	printf("--- CloseDataBase ---\n");

	

	RETCODE	rc;


	if(ODBC_Hander->hstmt){
#if 1
	//	rc	= SQLFreeStmt(ODBC_Hander->hstmt,SQL_DROP);
		rc	= SQLFreeHandle(SQL_HANDLE_STMT,ODBC_Hander->hstmt);

	 	printf("SQLFreeEnv --> %d\n", rc);
	 	if(rc != 0){
	 		return false;
	 	}
#endif
	}

	//データ・サーバーからの切断
	rc	= SQLDisconnect(ODBC_Hander->hdbc);
	printf("SQLDisconnect --> %d\n", rc);
	if(rc != 0){
		return false;
	}

	if(ODBC_Hander->hdbc){
		//接続ハンドルの解放
		rc	= SQLFreeConnect(ODBC_Hander->hdbc);
		printf("SQLFreeConnect --> %d\n", rc);
		if(rc != 0){
			return false;
		}
	}

	if(ODBC_Hander->henv){
		//環境ハンドルの解放
		rc	= SQLFreeEnv(ODBC_Hander->henv);
		printf("SQLFreeEnv --> %d\n", rc);
		if(rc != 0){
			return false;
		}
	}

	

	delete ODBC_Hander;
	m_odbcCnt = 0;

	return true;
	
}
void CPxODBC::dumpError(void)
{
	SQLCHAR errstatus[256];
	SQLCHAR errmsg[256];
	SDWORD errcode;
	SWORD sz;

	SQLError(ODBC_Hander->henv,ODBC_Hander->hdbc,ODBC_Hander->hstmt,errstatus,&errcode,errmsg,sizeof(errmsg),&sz);
	printf("%s(%d)%*s\n",errstatus,errcode,(int)sz,errmsg);
}
void CPxODBC::command_select()
{
	UCHAR select[128];
	RETCODE rc;
	SDWORD id;
	//SDWORD datalen;
	SDWORD data2len;
	//UCHAR data[64];
	SDWORD data;
	UCHAR data2[256];

	HSTMT &hstmt = ODBC_Hander->hstmt;

	printf("select start\n");
	strcpy((char *)select,"SELECT QueueID,Status,DestinationAE from sendQueue  ");
	if(SQLExecDirect(hstmt,select,SQL_NTS) != SQL_SUCCESS){
		dumpError();
		return;
	}
	SQLBindCol(hstmt,1,SQL_C_SLONG,&id,0,NULL);
	SQLBindCol(hstmt,2,SQL_C_SLONG,&data,0,NULL);
	SQLBindCol(hstmt,3,SQL_C_CHAR,data2,(SWORD)sizeof(data2),&data2len);
	printf("\t QueueID      Status     DATA2\n");
 	for(;;){
		rc = SQLFetch(hstmt);
		if(rc == SQL_NO_DATA_FOUND)break;
		if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO){
			dumpError();;
			return;
		}
		printf("\t  %d        %d        %s\n",id,data,data2);
 	}
	return;
}

void CPxODBC::command_execProc()
{
	UCHAR cmd_str[128];
	RETCODE rc;
	 
	//SDWORD datalen;
	SDWORD data2len;
	//UCHAR data[64];
	SDWORD data;
	UCHAR data2[256];

	HSTMT &hstmt = ODBC_Hander->hstmt;

	printf("exe Proc start\n");
 
	strcpy((char *)cmd_str,
		"{CALL PxQueueDB.dbo.ChgQueueStatus(?,?,?,?)} "
	 );
 
	/*
	* 使い方不明 2012/04/18
	*/
	
	SQLINTEGER  InParameterSize1;
	SQLINTEGER  InParameterSize2;
	SQLINTEGER  InParameterSize3;

 
	 SQLLEN cbParm1 = SQL_NTS;

	SQLINTEGER  id = 2597;
	InParameterSize1 = sizeof(id);
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,		SQL_C_SSHORT, SQL_INTEGER, 
								0, 0, &id,			0, &InParameterSize1);

	SQLINTEGER  CurStatus = 16;
	InParameterSize2 = sizeof(CurStatus);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,		SQL_C_SSHORT, SQL_INTEGER, 
								0, 0, &CurStatus,	0, &InParameterSize2);

	SQLINTEGER  NewStatus = 1;
	InParameterSize3 = sizeof(NewStatus);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT,		SQL_C_SSHORT, SQL_INTEGER, 
								0, 0, &NewStatus,	0, &InParameterSize3);

	SQLINTEGER returnVal;
	cbParm1 = sizeof(returnVal);
	SQLBindParameter(hstmt, 4, SQL_PARAM_OUTPUT,	SQL_C_SSHORT, SQL_INTEGER, 
								0, 0, &returnVal,	0, &cbParm1);


	if(SQLPrepare(hstmt, cmd_str, SQL_NTS)== SQL_ERROR  )
	{
		dumpError();
		return ;
	}
    
	
	if(SQLExecute(hstmt) == SQL_ERROR ){
		dumpError();
		return;
	}
 
	int ret_val=-100;
#if 1

	SQLRETURN   retcode, retcode2;
#if 1
	// Process the result set and move to the streamed output parameters.
   if(SQLFetch( hstmt )== SQL_ERROR  )
	{
		dumpError();
		return ;
   } 
#else
   // Clear any result sets generated.
   while ( ( retcode = SQLMoreResults(hstmt) ) != SQL_NO_DATA )
      ;

#endif


   
	

	// SQLGetData retrieves and displays the picture in parts.
   // The streamed output parameter is available.
 //  while (retcode == SQL_PARAM_DATA_AVAILABLE) 
   {
       SQLPOINTER token;                         // Output by SQLParamData.
       SQLINTEGER cbLeft;                       // #bytes remained
  //     retcode = SQLParamData(hstmt, &token);
   //    if ( retcode == SQL_PARAM_DATA_AVAILABLE ) 
	//   if ( retcode != SQL_ERROR ) 
       {
           // A do-while loop retrieves the picture in parts.
           do 
           {
               retcode2 = SQLGetData( 
                            hstmt, 
                            1,//(UWORD) token,       // the value of the token is the ordinal. 
                            SQL_C_BINARY,        // The C-type of the picture.
                            &ret_val,         // A small buffer. 
                            sizeof(ret_val), // The size of the buffer.
                            &cbLeft);            // How much data we can get.
           }
           while ( retcode2 == SQL_SUCCESS_WITH_INFO );
//	   }else{
 		   dumpError();
 	   }
   }


#else
	SQLBindCol(hstmt,1,SQL_C_SLONG,&ret_val,0,NULL);
	rc = SQLFetch(hstmt);
	if(rc == SQL_NO_DATA_FOUND){
		return ;;
	}else{
		if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO){
			dumpError();;
			return ;
		}
	}
#endif

	return ;
}


void test();
void CPxODBC::doTest()
{
//	test();
	char datasouce[128] = "";
	char username[128] = "";
	char pasword[128] = "";
	char str[128] = "";

	printf("DBへの接続\n");
	printf("ODBCデータベース：");
 
	strcpy(datasouce,"DSN=DSNPxQueueDB;");
//	printf("ユーザー：");
	//scanf("%c",&username);
//	gets(username);
//	printf("パスワード：");
	//scanf("%c",&pasword);
//	gets(pasword);

	//	データベースを開く
	 
	bool ret = opendDB(datasouce);
	if(ret){
		printf("SUCCESS OPEN\n");
	}else{
		printf("ERROR OPEN\n"); return;
	}

  

	//command_select();

	 command_execProc();


	//	データベースを閉じる
	ret = closeDB();
	if (ret ){
		printf("SUCCESS CLOSE\n");
	}else{
		printf("ERROR CLOSE\n"); return;
	}
;
}

using namespace std;
#define	SQL_BUFF_MAX	1024
#define	SQL_CONDITION_MAX	512

RETCODE	GetSqlError(HENV henv, HDBC hdbc, HSTMT hstmt, RETCODE rc,	//In
		SQLCHAR szSqlState[6], SQLCHAR szErrorMsg[256]);			//Out


//int opendatabase(HENV *henv, HDBC *hdbc, char connect_str[]);
int opendatabase(HENV *henv, HDBC *hdbc, HSTMT *hstmt,char datasouce[],char username[],char pasword[]);
int closedatabase(HENV *henv, HDBC *hdbc);
void command_select(HENV henv, HDBC hdbc,HSTMT hstmt);

void command_execProc(HENV henv, HDBC hdbc,HSTMT hstmt);
void err(HENV henv,HDBC hdbc,HSTMT hstmt);



void test()
{
	RETCODE	ret;

	HENV	henv;	//環境ハンドル
	HDBC	hdbc;	//接続ハンドル
	HSTMT   hstmt;  //ステートメントハンドル
		
	/*char	connect_str[SQL_CONDITION_MAX];

	sprintf(connect_str, "DSN=testdb;UID=testuser;PWD=aihara06");
	//sprintf(connect_str, "DSN=MySQL testdb;UID=testuser;PWD=aihara06");
	*/

	char datasouce[128] = "";
	char username[128] = "";
	char pasword[128] = "";
	char str[128] = "";

	printf("DBへの接続\n");
	printf("ODBCデータベース：");
	//scanf("%c",&datasouce);
//	gets(datasouce);
	strcpy(datasouce,"DSN=DSNPxQueueDB;");
//	printf("ユーザー：");
	//scanf("%c",&username);
//	gets(username);
//	printf("パスワード：");
	//scanf("%c",&pasword);
//	gets(pasword);

	//	データベースを開く
	//ret = opendatabase(&henv, &hdbc, connect_str);
	ret = opendatabase(&henv, &hdbc, &hstmt,datasouce,username,pasword);
	if(ret == 0){
		printf("SUCCESS OPEN\n");
	}else{
		printf("ERROR OPEN\n"); return;
	}


	// command_select(henv,hdbc,hstmt);
	 

	 command_execProc(henv,hdbc,hstmt);


	//	データベースを閉じる
	ret = closedatabase(&henv, &hdbc);
	if (ret == 0){
		printf("SUCCESS CLOSE\n");
	}else{
		printf("ERROR CLOSE\n"); return;
	}

	return;	



}


//int opendatabase(HENV *henv, HDBC *hdbc, char connect_str[]){
int opendatabase(HENV *henv, HDBC *hdbc,HSTMT *hstmt ,char datasouce[],char username[],char pasword[]){
	printf("--- OpenDataBase ---\n");

	//
	RETCODE	rc;

	//
	//char	ret_connect_str[SQL_MAX_OPTION_STRING_LENGTH];
	//SWORD	ret_len;

	//環境ハンドルを割り振る
	rc	= SQLAllocEnv(henv);
	printf("SQLAllocEnv --> %d\n", rc);
	if(rc != 0){return -1;}

	//接続ハンドルを割り振る
	rc	= SQLAllocConnect(*henv, hdbc);
	printf("SQLAllocConnect --> %d\n", rc);
	if(rc != 0){return -1;}

#if 1
	{
	 /* Connect to the DSN mydsn */
		SQLCHAR  outstr[256];
		SQLSMALLINT  outstrlen;
	  SQLRETURN ret;
		ret = SQLDriverConnect(*hdbc, NULL, (SQLCHAR*)datasouce, SQL_NTS,
				 outstr, sizeof(outstr), &outstrlen,
				 SQL_DRIVER_COMPLETE);
	  if (SQL_SUCCEEDED(ret)) {
		printf("Connected\n");
		printf("Returned connection string was:\n\t%s\n", outstr);
		if (ret == SQL_SUCCESS_WITH_INFO) {
		  printf("Driver reported the following diagnostics\n");
//		  extract_error("SQLDriverConnect", *hdbc, SQL_HANDLE_DBC);
		}
	// 	SQLDisconnect(*hdbc);		/* disconnect from driver */
	  } else {
		fprintf(stderr, "Failed to connect\n");
	//	extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
	  }

	  rc = ret;
	}
#else

 
	//データ・ソースに(拡張)接続する
	/*rc	= SQLDriverConnect( *hdbc, 0,
		(unsigned char *)connect_str,
		SQL_NTS,
		(unsigned char *)ret_connect_str,
		(SWORD)sizeof( ret_connect_str ),
		&ret_len, SQL_DRIVER_NOPROMPT );*/
	rc = SQLConnect(*hdbc,
//		(unsigned char *)datasouce,
		(unsigned char *)"driver={SQL Server};server=MONE\SQLEXPRESS;database=PxQueueDB",

		SQL_NTS,
		(unsigned char *)username,
		SQL_NTS,
		(unsigned char *)pasword,
		SQL_NTS
		);

	printf("SQLConnect --> %d\n", rc);
	
	if(rc == 0){
		;
	}else{
		return -1;
	}
#endif

	//
	rc = SQLAllocStmt(*hdbc,hstmt);
	printf("SQLAllocStmt --> %d\n", rc);
	if(rc != 0){return -1;}

	return 0;
}


int closedatabase(HENV *henv, HDBC *hdbc){
	printf("--- CloseDataBase ---\n");

	RETCODE	rc;

	//データ・サーバーからの切断
	rc	= SQLDisconnect(*hdbc);
	printf("SQLDisconnect --> %d\n", rc);
	if(rc != 0){return -1;}

	//接続ハンドルの解放
	rc	= SQLFreeConnect(*hdbc);
	printf("SQLFreeConnect --> %d\n", rc);
	if(rc != 0){return -1;}

	//環境ハンドルの解放
	rc	= SQLFreeEnv(*henv);
	printf("SQLFreeEnv --> %d\n", rc);
	if(rc != 0){return -1;}

	return 0;
}


void command_select(HENV henv, HDBC hdbc,HSTMT hstmt){
	UCHAR select[128];
	RETCODE rc;
	SDWORD id;
	//SDWORD datalen;
	SDWORD data2len;
	//UCHAR data[64];
	SDWORD data;
	UCHAR data2[256];
	printf("select start\n");
	strcpy((char *)select,"SELECT QueueID,Status,DestinationAE from sendQueue WHERE Status=16 ");
	if(SQLExecDirect(hstmt,select,SQL_NTS) != SQL_SUCCESS){
		err(henv,hdbc,hstmt);
		return;
	}
	SQLBindCol(hstmt,1,SQL_C_SLONG,&id,0,NULL);
	SQLBindCol(hstmt,2,SQL_C_SLONG,&data,0,NULL);
	SQLBindCol(hstmt,3,SQL_C_CHAR,data2,(SWORD)sizeof(data2),&data2len);
	printf("\t QueueID      Status     DATA2\n");
 	for(;;){
		rc = SQLFetch(hstmt);
		if(rc == SQL_NO_DATA_FOUND)break;
		if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO){
			err(henv,hdbc,hstmt);
			return;
		}
		printf("\t  %d        %d        %s\n",id,data,data2);
 	}
	return;
}


void err(HENV henv,HDBC hdbc,HSTMT hstmt){
	SQLCHAR errstatus[256];
	SQLCHAR errmsg[256];
	SDWORD errcode;
	SWORD sz;

	SQLError(henv,hdbc,hstmt,errstatus,&errcode,errmsg,sizeof(errmsg),&sz);
	printf("%s(%d)%*s\n",errstatus,errcode,(int)sz,errmsg);
}

void command_execProc(HENV henv, HDBC hdbc,HSTMT hstmt)
{
	UCHAR select[128];
	RETCODE rc;
	SDWORD id;
	//SDWORD datalen;
	SDWORD data2len;
	//UCHAR data[64];
	SDWORD data;
	UCHAR data2[256];
	printf("exe Proc start\n");
#if 0
	strcpy((char *)select,
	 			"DECLARE @result INT  ; "
				" set @result = -1 ; "
	 		//	"EXEC PxQueueDB.dbo.ChgQueueStatus '2597', '16', '1', @result OUTPUT"
	 			" SELECT 'result'=1 ; "
	//" SELECT 12 "
				);
#else
	strcpy((char *)select,
		"DECLARE @result INT   "
	 	"SET @result = 34  "
		"SELECT 'id'=1,'result' = 23 ");
#endif
	
	if(SQLPrepare(hstmt,
                     select,
                     sizeof(select))!= SQL_SUCCESS){
		err(henv,hdbc,hstmt);
		return;
	}

	
	if(SQLExecDirect(hstmt,select,SQL_NTS) != SQL_SUCCESS){
		err(henv,hdbc,hstmt);
		return;
	}

//	SQLBindCol(hstmt,1,SQL_C_SLONG,&id,0,NULL);
	SQLBindCol(hstmt,1,SQL_C_CHAR,data2,(SWORD)sizeof(data2),&data2len);

	rc = SQLFetch(hstmt);
	if(rc == SQL_NO_DATA_FOUND){
		;
	}else{
		if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO){
			err(henv,hdbc,hstmt);
			return;
		}else{
			printf("\t  %d  \n",id );
		}
	}
}