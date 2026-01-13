#include "ProcBase.h"

CProcBase::CProcBase(void)
{
	m_initDBFlag = false;
	
	m_countNN = 0;
}

CProcBase::~CProcBase(void)
{
}

void g_initDB();
bool CProcBase::doInitDB()
{
	if(m_initDBFlag){
		return true;
	}

	g_initDB();
	 
	m_initDBFlag = true;
	return true;
}
