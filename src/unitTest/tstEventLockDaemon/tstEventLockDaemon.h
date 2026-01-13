/***********************************************************************
 
 *-------------------------------------------------------------------
 */

#ifndef _TEST_EVE_LOCLDAEMON_er_H_
#define _TEST_EVE_LOCLDAEMON_er_H_

 

#include "RTVDaemon.h"

//-----------------------------------------------------------------------------

class TstEvtLockServiceProcessor : public RTVDaemonProcessor
{
public:
	TstEvtLockServiceProcessor(const char *iName) : RTVDaemonProcessor(iName)
	{
		m_stop = 0;
	}

	void	Stop(void);
	int		IsStopped(void) const { return m_stop;}
	int		PreProcess(void);
	int		Process(int argc, char **argv);
private:
	int		m_stop;
};


#endif //_TEST_EVE_LOCLDAEMON_er_H_
