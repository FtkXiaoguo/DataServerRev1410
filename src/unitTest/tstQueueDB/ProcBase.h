#pragma once

#include "rtvthread.h"

 
class CProcBase : public iRTVThreadProcess
{
public:
	CProcBase(void);
	~CProcBase(void);
protected:
	bool doInitDB();
	bool m_initDBFlag;
	unsigned long m_countNN ;
};
