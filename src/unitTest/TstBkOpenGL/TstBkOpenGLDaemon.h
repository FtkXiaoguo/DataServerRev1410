/***********************************************************************
 * TIDICOMServer.h
 *---------------------------------------------------------------------
 *		Copyright, Mitsubishi Electric Information Technology Center 
 *					America, Inc., 2000, All rights reserved.
 *
 *	PURPOSE:
 *     Boiler plate code to make the TIDICOMServer a NT service.
 *
 *	AUTHOR(S):  Gang Li. 05-19-2002.
 *
 *  Mod. History:
 *
 *   
 *-------------------------------------------------------------------
 */

#ifndef TIDICOMServer_H_
#define TIDICOMServer_H_

// T.C. Zhao 2003-02-13 added version number
#define		kDICOMServerVersion		2
#define		kDICOMServerRevision	2
#define		kDICOMServerTweaks		0
#define		kDICOMServerBuild		10

#include "RTVDaemon.h"

//-----------------------------------------------------------------------------

class TIDICOMServiceProcessor : public RTVDaemonProcessor
{
public:
	TIDICOMServiceProcessor(const char *iName) : RTVDaemonProcessor(iName)
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


#endif
