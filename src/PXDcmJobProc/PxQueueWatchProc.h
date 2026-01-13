/***********************************************************************
 * PxQueueWatchProc.h
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */
#ifndef PXQUEUEWATCHPROC_H
#define PXQUEUEWATCHPROC_H

//#include "rtvthread.h"
#include "IntervalProcessor.h"

#include "PxQueue.h"

class COutputJpegProc;
class CSendDicomProc;
class CPxWorkQueue;
class PxQueueWatchProc  : public CIntervalProcessor//public iRTVThreadProcess
{
public:
	typedef std::map<std::string, int> AE_ApplicationID_Map;

//	static PxQueueWatchProc& thePxQueueWatchProc();
	
	PxQueueWatchProc(bool isOutJpegGateWay);//#48
	~PxQueueWatchProc();

	static void setupQueueLog();

	bool DICOM_Initialization();
	static void DICOM_Release();
	int	 PreProcess(void);
	int Process(void);
 
	///
	
	bool watchFilter(const CPxQueueEntry &entry);
	bool doQueueWork(const CPxQueueEntry &entry);
	///
	void setupSQLiteDBDir(const std::string &folderName){ m_SQLiteDB_Dir = folderName;};
	//
	virtual  void	RequestTermination(int iFlag=1);
	//
	void setWatchPriority(int priority){ m_watchPriority = priority;};

//	static	bool doInitDB(); //2013/03/19
private:
	///////////////
 
	bool  getImageFileListFromEntryFile(const CPxQueueEntry &entry, std::string &seriesFolder ,std::vector<std::string> &ImageFileList);
	//
	static	bool doInitDB();

	bool m_initDBFlag;
	
	unsigned long m_countNN ;
	////////////////


	virtual bool checkLicense();// K.Ko 2010/05/21
 
	
	int m_newAE;
	AE_ApplicationID_Map m_AEApplicationIDMap;
	DWORD m_start_mem;

	bool m_firstCheckFlag;
	bool m_checkedLicenseStatus;// K.Ko 2010/05/21
	FILETIME m_lastCheckLicenseTime;// K.Ko 2010/05/21
	////////
	CSendDicomProc  *m_CSendDicomProc_hdr;

	COutputJpegProc *m_outputJPEGProc_hdr;//#48
	//
	std::string m_SQLiteDB_Dir;
	//
	CPxWorkQueue * m_QueueProc;
	//
	int	m_watchPriority;
	//
	 
};

#endif // PXQUEUEWATCHPROC_H
