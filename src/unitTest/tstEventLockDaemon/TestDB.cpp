// TestDB.cpp: CTestDB クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4503)
#pragma warning (disable: 4786)
 
#include "TestDB.h"
#include "windows.h"
//#include "afxwin.h"

 #include "PxQueue.h"
 #include "AQNetConfiguration.h"

#include "string"

#ifndef MAX_PATH
#define MAX_PATH 1204
#endif
static char gPxDcmServerHomeBuff[2*MAX_PATH+1]={0,};

static void getHomeFromModulePath()
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

extern TRLogger gLogger;
//////////////////////
class CMyPxQueueProc : public  CPxWorkQueue
{
public:
	void lockDB(){
		CPxWorkQueue::lockDB();
	}
	void unlockDB(){
		CPxWorkQueue::unLockDB();
	}
	void addQueueEvent(){
		::SetEvent(m_sendQueuEvent);
	}
	bool waitQueueEvent(){
		HANDLE sendQueu_h = getSendQueuEvent();
		if(!sendQueu_h){
//			::AfxMessageBox("handl is null");
			return false;
		}
		::WaitForSingleObject(sendQueu_h, INFINITE);
		::ResetEvent(sendQueu_h);
		return true;
	}
};
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTestDB::CTestDB()
{

}

CTestDB::~CTestDB()
{

}
void CTestDB::initTestDB(bool global)
{
	CPxQueueProc::setLogger(&gLogger);

	CPxQueueProc::Global_init(global);

	getHomeFromModulePath();

	AQNetConfiguration::setHomeFolder(gPxDcmServerHomeBuff) ;
	std::string dataRootPath = "";
	AQNetConfiguration::GetDataRootPath(dataRootPath);

	std::string queue_db_fileName = dataRootPath + "\\" + SQLITE_DB_FILTE;
	CPxQueueProc::setupQueueDBName(queue_db_fileName);

}
void CTestDB::addQueueEvent()
{
	CMyPxQueueProc db;
	db.initRes();
	db.addQueueEvent();
}
bool CTestDB::waitQueueEvent()
{
	CMyPxQueueProc db;
	db.initRes();
	return db.waitQueueEvent();
}
void CTestDB::lockDB()
{
	CMyPxQueueProc db;
	db.initRes();
	db.lockDB();
}
void CTestDB::unlockDB()
{
	CMyPxQueueProc db;
	db.initRes();
	db.unlockDB();
}