/***********************************************************************
 * $Id: RTVCacheWriter.cpp 35 2008-08-06 02:57:21Z atsushi $
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
#pragma warning (disable: 4786)
#include "RTVCacheWriter.h"
#include "TICache.h"
#include <time.h>
#include "diskspacemanager.h"


int RTVCacheWriter::c_logLevel = 0;
bool RTVCacheWriter::c_doCompress = false;

// create RTVCacheWriter instance, then turn it to One shot Tread manager
//called in SeriesDirMonitor

int RTVCacheWriter::AddJob(const char* iDICOMDir)
{
	RTVCacheWriter* p = new RTVCacheWriter(iDICOMDir);
	RTVOneShotThreadManager::theManager().AddRequest(p);
	return kSuccess;
}


//-----------------------------------------------------------------------------
RTVCacheWriter::RTVCacheWriter(const char* iDICOMDir):
		m_DICOMDirectory(iDICOMDir), m_status(kSuccess)
{
	m_processorName = "RTVCacheWriter";
	m_requestTime = time(0);

}

//-----------------------------------------------------------------------------

RTVCacheWriter::~RTVCacheWriter()
{
}


//-----------------------------------------------------------------------------
int RTVCacheWriter::Process()
{
	if(TerminationRequested()) // fullfill cancell requested before start thread
		return (m_status = kCancelled);

	m_status = kRunning;
	return StartAndWait(m_DICOMDirectory.c_str(), m_requestTime);
}

int RTVCacheWriter::StartAndWait(const char* iDICOMDir, int iRequestTime)
{
	if(iRequestTime == 0)
		iRequestTime = time(0);

		
	std::string cacheDir = "";
	std::string seriesUID, studyUID;
	
	char *t1, *t2;
	char buf[300];
	strcpy(buf, iDICOMDir);
	// get rid of tail slash
	int slen = strlen(buf)-1;
	char tail = buf[slen];
	if (tail == '/' || tail == '\\')
		buf[slen] = 0;

	// get series UID and study UID
	t1 = strrchr(buf, '/');
	t2 = strrchr(buf, '\\');
	if(t1 < t2)
		t1 = t2;
	if(!t1) 
		kErrCacheNotWritten;

	seriesUID = t1+1;
	*t1 = 0;

	t1 = strrchr(buf, '/');
	t2 = strrchr(buf, '\\');
	if(t1 < t2)
		t1 = t2;
	if(!t1) 
		kErrCacheNotWritten;
	studyUID = t1+1;
		
	cacheDir = RTVDiskSpaceManager::GetDirectoryToWriteCacheTo(seriesUID, studyUID);
	if(cacheDir.length() < 5)
		return kErrCacheNotWritten;

	// Make sure we have no trailing '/'
	slen = cacheDir.length()-1;
	tail = cacheDir[slen];
	if (tail == '/' || tail == '\\')
		cacheDir.resize(slen);

	// check if there is a good cache already
	WIN32_FIND_DATA FindFileData;
	HANDLE handle;

	sprintf(buf, "%s/%s", cacheDir.c_str(), kCacheDescriptionFileName);
	handle = FindFirstFile(buf, &FindFileData);
	if (handle != INVALID_HANDLE_VALUE)
	{
		TRFile cacheFile(FindFileData);
		int mtime = cacheFile.GetAgeInSeconds(TRFile::kCreateTime);
		if(mtime+1 > iRequestTime) // the cache is created after request time, skip build it
			return kSuccess;
	}


	// It is assumed that the cachewriter executable will be located in the same directory
	// as this module file. Verify it! Murali 01/12/2006. 
	const char* kCWFileName = "AQNetCacheWriter.exe";	
	char CacheWriterFileName[800];
	bool pathFound = false;

	char pModuleFileName[500];
	DWORD fileNameLen = GetModuleFileName(NULL, pModuleFileName, sizeof(pModuleFileName));
	if(fileNameLen)
	{	
		std::string moduleFilePath(pModuleFileName);
		int pos = moduleFilePath.find_last_of("\\"); 						
		if(pos > 0)
		{
			std::string moduleDirPath;
			moduleDirPath.assign(moduleFilePath.data(),0, pos); 

			_snprintf(	CacheWriterFileName, sizeof(CacheWriterFileName)-1, "%s\\%s",
						moduleDirPath.data(),
						kCWFileName); 

			DWORD tmp = GetFileAttributes(CacheWriterFileName);
			if(tmp != 0xFFFFFFFF)
			{
				pathFound = true;
				OutputDebugString(CacheWriterFileName);
			}
		}				
	}

	// fall back to the default filename.
	if(!pathFound)
	{
		strcpy(CacheWriterFileName, kCWFileName);
	};

	sprintf(buf, "-l %d", c_logLevel);
	int rcode = 0;
	if(c_doCompress)
	{	
		rcode = _spawnl( _P_WAIT, CacheWriterFileName, "AQNetCacheWriter", 
			"-b", buf, iDICOMDir, cacheDir.c_str(), NULL );
	}
	else
	{
		rcode =_spawnl( _P_WAIT, CacheWriterFileName, "AQNetCacheWriter", 
			buf, iDICOMDir, cacheDir.c_str(), NULL );
	}
	
	// To be removed after the initial testing - Murali
	char message[200];
	_snprintf(message,199, "RTVCacheWriter::StartAndWait : rcode %d\n", rcode);
	OutputDebugString(message);

	if(rcode != 0) // some error happend, clean cache
	{
		TRPlatform::RemoveDirectory(cacheDir.c_str());
	}
	return kSuccess;	
}


//-----------------------------------------------------------------------------