/***********************************************************************
 * $Id: TRCompressedCacheWriter.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used to read the Terarecon Cache.
 *		DICOM Series.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

#if !defined(TRCOMPRESSEDCACHEWRITER_H)
#define TRCOMPRESSEDCACHEWRITER_H

#define AWARE_ONLY

#include "rtvthread.h"
#include "rtvPoolAccess.h"
#include <string>

#include "AppComCacheReader.h"

//-----------------------------------------------------------------------------
class TRCompressedCacheWriter  : public iRTVThreadProcess
{
	public:
		typedef RTVMapAccess<std::string, TRCompressedCacheWriter*> ProcessMap;
		static int Start(const char* iCacheDirectory, bool threading, float iCompressionFactor=10.0);
		static int Stop(const char* iCacheDirectory);
		virtual ~TRCompressedCacheWriter();
		
		int Process();
		int GetStatus() {return m_status;};
		int DeleteLevel0CacheFiles();

	protected:
		static ProcessMap c_pMap;

		TRCompressedCacheWriter(const char* iCacheDirectory, float iCompressionRatio);
		int WriteCompresssedCache ();
		int WriteLevel0CompresssedCache ();
		int WriteLevel1CompresssedCache ();

		std::string				m_cacheDirectory;
		float					m_compressionFactor;
		AppComCacheReader	m_cacheReader;
		int						m_status;
		bool					m_inMap;

		std::string m_srcDataFileName;
		std::string	m_level0CacheDesctiptionFileName;
		std::string m_level0CacheDataFileName;

		std::string	m_level1CacheDesctiptionFileName;
		std::string m_level1CacheDataFileName;
};
//-----------------------------------------------------------------------------

#endif // !defined(TRCOMPRESSEDCACHEWRITER_H)
