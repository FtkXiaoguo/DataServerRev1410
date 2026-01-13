/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "IntervalProcessor.h"


CIntervalProcessor::CIntervalProcessor(const char *iProcessorName) :
iRTVThreadProcess(iProcessorName)
{
	m_intervalTime = 6000;//mSec
	m_wakeupEvent = CreateEvent(0, TRUE, FALSE, 0);
}
CIntervalProcessor::~CIntervalProcessor()
{
	CloseHandle(m_wakeupEvent);
}
void CIntervalProcessor::RequestTermination(int iFlag )
{
	iRTVThreadProcess::RequestTermination( iFlag );
	SetEvent(m_wakeupEvent);
}
void CIntervalProcessor::setWakeupEvent()
{
	SetEvent(m_wakeupEvent);
}

void CIntervalProcessor::ForceStop(int imSec)//#82 2014/09/29 K.Ko change to virtual
{
	if (!m_theThread)
		return;
	
	//for setEvent(m_wakeupEvent)
	RequestTermination(1);
	::Sleep(30);
	RequestTermination(1);
	::Sleep(30);
	RequestTermination(1);

	m_theThread->Terminate(imSec, true);
}
int CIntervalProcessor::WaitForProcessToEnd(int imSec, int iDelta)
{
	//’·ŠÔ‚v‚‚‰‚”‚©‚ç–ÚŠo‚Ü‚µ
	DWORD dwWaitStatus = WaitForSingleObject(m_wakeupEvent, imSec);
	if(!m_ended){ //RequestTerminationˆÈŠO
		ResetEvent(m_wakeupEvent);
	}
	return dwWaitStatus;
}
int	CIntervalProcessor::Process(void)
{
	int recode = 1;

	ResetEvent(m_wakeupEvent);
	while(!TerminationRequested())
	{
		WaitForProcessToEnd(m_intervalTime);

		if(TerminationRequested()){
			break;
		}
		recode = doProcess();
	}
	return recode;
}