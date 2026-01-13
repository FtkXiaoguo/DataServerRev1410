/***********************************************************************
 
 *-------------------------------------------------------------------
 */
#ifndef TEST_DB_H
#define TEST_DB_H

#include "rtvthread.h"

#include "PxNetDB.h"

class TestDBProessBase : public iRTVThreadProcess
{	
public:
	void destroy();
	int Process(void);
	void setPatientName(std::string name){
		m_PatientName = name;
	}
protected:
	TestDBProessBase() ;
	virtual ~TestDBProessBase() {
	}

	////////////////////////////
	void setupStudyDescription(std::string studyUID,std::string des);
	///////////////////////////

	bool doSearchStudy();
	virtual bool doSearchSeries(std::string study_uid);

	virtual bool procSeries(std::vector<pRTVSDicomInformation> & SeriesOut);

	virtual bool doDBProc() = 0;
 
	bool doInitDB();

 	bool m_initDBFlag;

	int m_iGroupID ;
	int m_iUserID  ;

	std::string m_PatientName;

	int m_run_count;
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
	static TestDBProcessBigDb *createInstace();
//	static TestDBProcessUpdate& theTestDBProcess();
	virtual ~TestDBProcessBigDb(){};
 
	int	 PreProcess(void);

	virtual bool doDBProc();

	void setBigDbLen(int len){m_testBigDBLen = len;};
private:
	TestDBProcessBigDb();
	bool dispLastData();
	bool doCreateTestBigDB();
	bool doCheckBigDB(int id,int field_index);
	bool doInsertBigDB(int index);
	bool doUpdateBigDB(int index,int field_index);
	bool doDeleteBigDB(int index);
	//
	bool findBigDB(int index);
static bool m_creatBigDBFlag;
	 int m_testBigDBLen;
};

 
#endif // LISTENER_H
