#pragma once

#include "ProcBase.h"
#include "PxQueue.h"

class TRLogger;
class CProcQueue : public CProcBase
{
public:
	CProcQueue(void);
	~CProcQueue(void);
	void setwatchOnAE(const std::string &ae){ m_watchOnAE = ae;};
	virtual  int	Process(void);
	//
	bool watchFilter(const CPxQueueEntry &entry);
	bool doQueueWork(const CPxQueueEntry &entry);
protected:
	 std::string m_watchOnAE;
	 //
	 TRLogger *m_Logger;
};
