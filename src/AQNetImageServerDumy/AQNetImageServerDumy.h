/***********************************************************************
 * PXDcmJobProc.h
 *---------------------------------------------------------------------
 *   
 *-------------------------------------------------------------------
 */

#ifndef AENET_IMAGE_SERVER_DUMMY_H
#define AENET_IMAGE_SERVER_DUMMY_H
 
#include "AppVersion.h"

 
#include "RTVDaemon.h"

//-----------------------------------------------------------------------------

class PXAqnetImageServerDumy : public RTVDaemonProcessor
{
public:
	PXAqnetImageServerDumy(const char *iName) : RTVDaemonProcessor(iName)
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


#endif //AENET_IMAGE_SERVER_DUMMY_H
