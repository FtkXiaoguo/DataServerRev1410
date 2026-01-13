/***********************************************************************
 * CAutoDelStudyProc.h
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */
#ifndef CAutoDelStudyProc_H
#define CAutoDelStudyProc_H

//#include "rtvthread.h"
#include "IntervalProcessor.h"

 
class COutputJpegProc;
class CSendDicomProc;
class CPxWorkQueue;
class CAutoDelStudyProc  : public CIntervalProcessor//public iRTVThreadProcess
{
public:
	typedef std::map<std::string, int> AE_ApplicationID_Map;

//	static CAutoDelStudyProc& theCAutoDelStudyProc();
	
	CAutoDelStudyProc();//#48
	~CAutoDelStudyProc();

	static void setupQueueLog();

	bool DICOM_Initialization();
	static void DICOM_Release();
	int	 PreProcess(void);
	int Process(void);
 
	///
	

	///
	void setupSQLiteDBDir(const std::string &folderName){ m_SQLiteDB_Dir = folderName;};
	//
	virtual  void	RequestTermination(int iFlag=1);
	//
	void setWatchPriority(int priority){ m_watchPriority = priority;};
private:
	///////////////
	//
	bool doInitDB();

	bool searchStudy(std::vector<std::string> &studyList);

	bool getResultQueueList(std::vector<std::string> &studyList,bool failedOnly=true);
	bool getSendQueueList(std::vector<std::string> &studyList);
	bool deleteStudy(const std::string &studyUID);
	
	bool m_initDBFlag;
	
	unsigned long m_countNN ;
	////////////////

	bool doDeleteStudy();

	virtual bool checkLicense();// K.Ko 2010/05/21
 
	
	int m_newAE;
	AE_ApplicationID_Map m_AEApplicationIDMap;
	DWORD m_start_mem;

	bool m_firstCheckFlag;
	bool m_checkedLicenseStatus;// K.Ko 2010/05/21
	FILETIME m_lastCheckLicenseTime;// K.Ko 2010/05/21
	////////
	 
	//
	std::string m_SQLiteDB_Dir;
	//
	 
	//
	int	m_watchPriority;
	//
	 
};

#endif // CAutoDelStudyProc_H
