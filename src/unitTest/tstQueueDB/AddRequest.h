#pragma once

#include "ProcBase.h"

class CAddRequest : public CProcBase
{
public:
	CAddRequest(void);
	~CAddRequest(void);
virtual  int	Process(void);

void setAE(const std::string &ae){ m_AE = ae;};
void setMyID( int id) { m_MyID = id;};
void setPriority(int p) { m_priority= p;};
void setLoopInterval(int intval){ m_loopInterval = intval;};
protected:
	std::string m_AE;
	int m_priority;
	int m_loopInterval;
	void addReq();
	int m_MyID;
	
};
