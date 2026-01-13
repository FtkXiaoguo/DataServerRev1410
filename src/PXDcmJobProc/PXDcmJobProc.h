/***********************************************************************
 * PXDcmJobProc.h
 *---------------------------------------------------------------------
 *   
 *-------------------------------------------------------------------
 */

#ifndef TIDICOMServer_H_
#define TIDICOMServer_H_

#include "PXDcmJobProcError.h" //2010/04/27 K.Ko

#include "AppVersion.h"

// T.C. Zhao 2003-02-13 added version number
#define		kDICOMServerVersion		DCMAPP_VERSION_MAJOR
#define		kDICOMServerRevision	DCMAPP_VERSION_MINOR
#define		kDICOMServerTweaks		DCMAPP_VERSION_SUB
#define		kDICOMServerBuild		DCMAPP_VERSION_MICRO

#include "RTVDaemon.h"

//-----------------------------------------------------------------------------

class PXDcmJobProcServiceProcessor : public RTVDaemonProcessor
{
public:
	PXDcmJobProcServiceProcessor(const char *iName) : RTVDaemonProcessor(iName)
	{
		m_stop = 0;
	}

	void	Stop(void);
	int		IsStopped(void) const { return m_stop;}
	int		PreProcess(void);
	int		Process(int argc, char **argv);

	void    waitEvent(int imSec);//#82 2014/09/29 K.Ko change main thread
private:
	int		m_stop;

	void*			m_wakeupEvent;//#82 2014/09/29 K.Ko change main thread
};


#endif
