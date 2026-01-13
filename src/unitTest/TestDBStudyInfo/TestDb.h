/***********************************************************************
 
 *-------------------------------------------------------------------
 */
#ifndef TEST_DB_H
#define TEST_DB_H

#include "rtvthread.h"

#include "PxNetDB.h"

#include "vector"
class TestDBProessBase : public iRTVThreadProcess
{	
public:
	enum LANG_DEF {
		LANG_ID_Unknown = 0,
		LANG_ID_CHINESE  ,
		LANG_ID_RUSSIAN ,
		LANG_ID_LATIN1,
		LANG_ID_JAPANESE

	};

	void shutdownFinish() { onShutdown();};
	void destroy();
	int Process(void);
	void setPatientName(std::string name){
		m_PatientName = name;
	}
	void setLoopInterval(int inter /*mSec*/) { m_loopInterval = inter;};

	LANG_DEF m_LangID;
protected:
	TestDBProessBase() ;
	virtual ~TestDBProessBase() {
	}

	virtual void onShutdown() { };
	////////////////////////////
	void setupStudyDescription(std::string studyUID,std::string des);
	///////////////////////////

	bool doSearchStudy();
	bool doSearchBitData(void);
	virtual bool doSearchSeries(std::string study_uid);

	virtual bool procSeries(std::vector<pRTVSDicomInformation> & SeriesOut);

	virtual bool doDBProc() = 0;
 
	bool doInitDB();

 	bool m_initDBFlag;

	int m_iGroupID ;
	int m_iUserID  ;

	std::string m_PatientName;

	int m_run_count;
	int m_loopInterval; //mSec
};
class TestDBProcessList :  public TestDBProessBase
{
public:
 
	static TestDBProcessList *createInstace();
	
//	static TestDBProcessList& theTestDBProcess();
	

 	virtual ~TestDBProcessList(){};
	int	 PreProcess(void);
 
private:
	TestDBProcessList();
 
	virtual bool doDBProc();
};

class TestDBProcessUpdate :  public TestDBProessBase
{
public:
	static TestDBProcessUpdate *createInstace();
//	static TestDBProcessUpdate& theTestDBProcess();
	virtual ~TestDBProcessUpdate(){};
 
	int	 PreProcess(void);

	virtual bool doDBProc();
private:
	virtual bool doSearchSeries(std::string study_uid);
	virtual bool procSeries(std::vector<pRTVSDicomInformation> & SeriesOut);

	TestDBProcessUpdate();
	bool doUpdate();
  
};
 
class TestDBProcessBigDb :  public TestDBProessBase
{
public:
#define STR_MAX_LEN (2048)
#define STR_FILED_LEN (3)

#define DefLastDbDate "LastBigDbDate"
	static TestDBProcessBigDb *createInstace();
//	static TestDBProcessUpdate& theTestDBProcess();
	virtual ~TestDBProcessBigDb(){};
 
	int	 PreProcess(void);

	virtual bool doDBProc();

	void setBigDbLen(int len){m_testBigDBLen = len;};
	void setupBackupFlag(bool flag) { m_runBackupFlag = flag;};
	void setBackupInterval(int interval){m_bakcupInterval = interval;};
private:
	static void WriteKey(std::string keyName,int val);
	static int ReadKey(std::string keyName);
	virtual void onShutdown() ;
	TestDBProcessBigDb();
	bool dispLastData();
	bool doCreateTestBigDB();
	bool doCheckBigDB(int id,int field_index);
	bool doInsertBigDB(int index);
	bool runBackup();
	
	bool doDeleteDB(int index);
static bool m_creatBigDBFlag;
	 int m_testBigDBLen;
	//
	int m_lastDate;
	//
	bool m_runBackupFlag;
	int m_bakcupInterval; //Sec
	//
	std::vector<int> m_deleteIDList;
};

 
class TestDBProcessAddStudy : public TestDBProessBase
{
public:
	static TestDBProcessAddStudy *createInstace();
	//	static TestDBProcessUpdate& theTestDBProcess();
	TestDBProcessAddStudy();
	virtual ~TestDBProcessAddStudy();

	int	 PreProcess(void);

	virtual bool doDBProc();
private:
	 
	void beginNewStudy(void);
	void beginNewSeries(void);
	void beginNewImage(void);
	void checkNext(void);
	bool doAddStudy();
	bool doAddStudyCn();
	bool doAddStudyRu();
	bool doAddStudyL1();
	bool doAddStudyJp();
	bool doMyAddStudy();
	DICOMData *m_studyInfo;
	//
	int m_studyCount;
	int m_seriesCount;
	int m_imageCount;
	//
	int m_seriesNum;
	int m_imageNum;
};

class TestDBProcessSearchStudy : public TestDBProessBase
{
public:
	static TestDBProcessSearchStudy *createInstace();
	//	static TestDBProcessUpdate& theTestDBProcess();
	TestDBProcessSearchStudy();
	virtual ~TestDBProcessSearchStudy();

	int	 PreProcess(void);

	virtual bool doDBProc();
private:

	bool doSearchStudy();
	bool doSearchStudyCn();
	bool doSearchStudyRu();
	bool doSearchStudyL1();
	bool doSearchStudyJp();
	bool doMySearchStudy();

	DICOMData *m_studyInfo;
	 
};

#endif // LISTENER_H
