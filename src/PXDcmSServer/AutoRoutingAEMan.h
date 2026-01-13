/***********************************************************************
 * AutoRoutingAEMan.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *  Series毎にAutoRoutingを決める。
 *	
 *  
 *-------------------------------------------------------------------
 */
#ifndef AUTOROUTING_AE_MAN_H
#define AUTOROUTING_AE_MAN_H

#include <string>
#include <vector>
 
#include "rtvPoolAccess.h"

class CPxDicomImage;
class TRCriticalSection;

namespace PxDBUtil {
class CPxDBUtil;
}

 
class AutoRoutingTarget	//#48
{
public:
	enum {
		AutoRoutingType_DICOM = 0,
		AutoRoutingType_JPEG,
	};
	int m_procType;
	std::string m_AE;
};

class CPxWorkQueue;
class CPxQueueEntry;
class CPxQueueEntryExt;
class AutoRoutingAEMan 
	: public InactiveHandler  //別スレッド実行させるため 
{
public:
	AutoRoutingAEMan();
	~AutoRoutingAEMan();

	static void	ReformatJapaneseDicom( CPxQueueEntry &entry );//#25 20012/06/11 K.Ko
	static void	ReformatJapaneseDicom( const std::string &org, std::string &conv);//#25 20012/06/11 K.Ko

	void setStopFlag(bool flag) { m_stopFlag = flag;};
	bool getStopFlag() const { return m_stopFlag;};
//	bool getAEList(std::vector<std::string> &AEList, CPxDicomImage* img);//#48
	bool getAEList(std::vector<AutoRoutingTarget> &AEList, CPxDicomImage* img);
	void reset();
	bool isRoutingOnSchedule( CPxDicomImage* img) ;

	CPxWorkQueue * getWorkQueueInstance() { return m_WorkQueueInstance;};

	////#20 Series毎にAutoRoutingを行う　2012/05/23　K.KO 
	bool tryAutoRoutingOneSeries();
//	bool registerSeriesOnRouting(CPxDicomImage* img);
	//////////////////
	virtual int Process(void) ;
	virtual void Kick(void);
	virtual bool IsTimeOver(DWORD TickCount) ;
	virtual void ForceTimeOut();  
	//
	
	///////////////////
 
	bool doLast();
	void addImageFileName(const std::string &fileName);
	void setupSeriesFolder(const std::string &folder);
	void setAutoRoutingBlockSize(int size){ m_ImageFileBlockSize = size;};
 
protected:
	///////////////////
 
	bool tryAutoRoutingBlock(bool dolast);
	int getImageFileNameBlock(std::vector<std::string> &list,bool dolast=false);
	bool writeQueueBinFile(const CPxQueueEntry *entry,const std::vector<std::string> &fileList);
	
	///////////////////
	bool updateAEList( CPxDicomImage* img);
	bool getTagFilterIDFromRoutingSchedule( CPxDicomImage* img,std::vector<int> &TagFilterIDList);
	bool m_filterEnable;
	bool m_isRoutingOnSchedule;
	 
//	 std::vector<std::string> m_AEList; //#48
	std::vector<AutoRoutingTarget> m_AEList;
	 std::string m_StudyInstanceUID	;
	 std::string m_SeriesInstanceUID;
	 ////
	 // for entry extInfo
	 CPxQueueEntryExt *m_entrExtInfo;
	 //////////////////////

	 PxDBUtil::CPxDBUtil	*m_db;
	 //
	 TRCriticalSection *m_cs;
	 CPxWorkQueue * m_WorkQueueInstance;
	 ///////////
 /////////////
	 unsigned int m_lastActiveTime;

	 ///////
	 std::vector<std::string> m_ImageFileList;
	 int m_currentImageFileIndex;
	 int m_lastCheckImageFileSize;
	 int m_ImageFileBlockSize;
	 bool m_stopFlag;
	 //////
	 std::string m_seriesFolder;
	 int m_runCount;
	 bool m_waitFlag;
 
};

#endif // AUTOROUTING_AE_MAN_H

