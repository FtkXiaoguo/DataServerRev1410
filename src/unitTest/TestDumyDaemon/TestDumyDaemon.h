/***********************************************************************
 *-------------------------------------------------------------------
 */

#ifndef TEST_DUMY_DAEMON_H_
#define TEST_DUMY_DAEMON_H_

 
#include "RTVDaemon.h"

//-----------------------------------------------------------------------------

class TestDumyServiceProcessor : public RTVDaemonProcessor
{
public:
	TestDumyServiceProcessor(const char *iName) : RTVDaemonProcessor(iName)
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


#endif //TEST_DUMY_DAEMON_H_
