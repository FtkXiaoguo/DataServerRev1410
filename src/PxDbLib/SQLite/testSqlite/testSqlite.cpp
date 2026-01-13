// testSqlite.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "string.h"

#include "string"
#include "vector"


void testsqlite();
int main(int argc, char* argv[])
{
	printf("test sqlite3!\n");
	testsqlite();
	return 0;
}

#include "sqlite3.h"
void testsqlite()
{
	char *file_name = "F:\\PXSDataServer\\DataServer\\src\\bin_v8\\data\\PxSendQueue.db";

	sqlite3 *sqliteDB; 
	int rc = sqlite3_open(file_name, &sqliteDB);
	if( rc ){
 
		sqlite3_close(sqliteDB);
		return ;;
	}
	sqlite3_stmt *pSelect;
	const char *zTail;
	
	char *sql_cmd = "SELECT * FROM resultQueue";
	int sql_len = strlen(sql_cmd);
	 
	//Compiling An SQL Statement
	rc = sqlite3_prepare(	sqliteDB,            /* Database handle */
							sql_cmd,       /* SQL statement, UTF-8 encoded */
							sql_len,              /* Maximum length of zSql in bytes. */
							&pSelect,  /* OUT: Statement handle */
							&zTail     /* OUT: Pointer to unused portion of zSql */
							);
	if( rc!=SQLITE_OK ){
		 
		return;
	} 

	//
	if((rc = sqlite3_step(pSelect)) ==  SQLITE_BUSY) {
        return;
	}
	//
	/* if we have a result set... */
      if( SQLITE_ROW == rc ){
		  int col_i;
		  int nCol = sqlite3_column_count(pSelect);
		
		  for(col_i=0;col_i<nCol;col_i++){
			  std::string col_name =  (char *)sqlite3_column_name(pSelect, col_i);
			  
		  }
		  std::vector<int> col_types;
		  col_types.resize(nCol);
		  
		  //
		  do{
		//	  QARecordData new_record;
              /* extract the data and data types */
			  printf("\n>>>\n");
              for(col_i=0; col_i<nCol; col_i++){
                std::string col_val = (char *)sqlite3_column_text(pSelect, col_i);
                col_types[col_i] = sqlite3_column_type(pSelect, col_i);
                if( !col_types[col_i] && (col_types[col_i]!=SQLITE_NULL) ){
                  rc = SQLITE_NOMEM;
                  break; /* from for */
                }
				 
				printf(" %s \n",col_val.c_str());
              } /* end for */

			   
              /* if data and types extracted successfully... */
              if( SQLITE_ROW == rc ){ 
         
					for(int run_i =0 ;run_i<5;run_i++)
					{
						if((rc = sqlite3_step(pSelect)) ==  SQLITE_BUSY) {
						// sleep instead requesting result again immidiately.
							sqlite3_sleep(500);
						  }else{
							  break;
						  }
					}
             
              }
            } while( SQLITE_ROW == rc );

   //         sqlite3_free(pData);

	  }

	  //
	  sqlite3_finalize(pSelect);

	  sqlite3_close(sqliteDB);
}