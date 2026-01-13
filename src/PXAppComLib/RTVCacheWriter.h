/***********************************************************************
 * $Id: RTVCacheWriter.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used to start and clean up external cache writer
 *		DICOM Series.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

#if !defined(RTVCACHEWRITER_H)
#define RTVCACHEWRITER_H

#include "rtvthread.h"
#include <string>


//-----------------------------------------------------------------------------
class RTVCacheWriter  : public iRTVThreadProcess
{
public:
	static int c_logLevel;
	static bool c_doCompress;
	static int AddJob(const char* iDICOMDir);
	static int StartAndWait(const char* iDICOMDir, int iRequestTime=0);
	
	virtual ~RTVCacheWriter();
	
	int Process();
	int GetStatus() {return m_status;};

protected:
	RTVCacheWriter(const char* iDICOMDir);
	std::string	m_DICOMDirectory;
	int			m_requestTime;
	int			m_status;
};
//-----------------------------------------------------------------------------

#endif // !defined(RTVCACHEWRITER_H)
