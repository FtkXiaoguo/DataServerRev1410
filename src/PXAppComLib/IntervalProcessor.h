/***********************************************************************
 * IntervalProcessor.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include "RTVThread.h" 
 
class CIntervalProcessor : public iRTVThreadProcess 
{
public:
	CIntervalProcessor(const char *iProcessorName=0);
	virtual ~CIntervalProcessor(void);
	 
virtual  int	Process(void) ;	

	virtual void ForceStop(int iMsec);//#82 2014/09/29 K.Ko change to virtual
	
	virtual  void	RequestTermination(int iFlag=1);
	virtual  int	WaitForProcessToEnd(int iMsec=8000, int iDelta=10);
	void setWakeupEvent();
void setIntervalTime(int time /*mSec*/) { m_intervalTime = time;};
protected:
	virtual int doProcess(){ return 0;};
	 
	int m_intervalTime;
	void*			m_wakeupEvent;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // INTERVAL_PROCESSOR_H
