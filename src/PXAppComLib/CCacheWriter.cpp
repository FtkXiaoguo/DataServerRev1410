/***********************************************************************
 * CCacheWriter.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This file implements the member functions of the 
 *	    TerareconCache Object.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

#pragma warning (disable: 4786)

#include "CCacheWriter.h"

#include "AqCore/AqCore.h"
#include "AqCore/TRPlatform.h"
#include <TICache.h>

#include <CRTDBG.H>


void CloseLockedFile(HANDLE& hd)
{
	if(!hd) return;
	// close file
	SetEndOfFile(hd); // resize the file at current position
	CloseHandle(hd);
}

/*class CleanupException: public std::exception
{
};
*/

CFilePool CCacheWriter::C_filePool;
//-----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCacheWriter::CCacheWriter()
{
	Reset();
}

//-----------------------------------------------------------------------------

CCacheWriter::~CCacheWriter()
{
}

int CCacheWriter::Setup(const char* cacheLocation, 
						const char* dataFileName,
						ULONGLONG dataSize,
					    const char* descFileName,
						ULONGLONG descSize)
{
	m_cacheLocation = (cacheLocation == NULL)?"":cacheLocation;
	m_dataFilePath = (dataFileName == NULL)?"":m_cacheLocation+"/"+dataFileName;
	m_dataSize = dataSize;
	m_descFilePath = (descFileName == NULL)?"":m_cacheLocation+"/"+descFileName;
	m_descSize = descSize;

	m_initialized = true;
	return kSuccess;
}

void  CCacheWriter::Reset()
{
	m_cacheLocation = "";
	m_dataFilePath = "";
	m_dataSize = 0;
	m_descFilePath = "";
	m_descSize = 0;
	m_initialized = false;
}

static TRCriticalSection OpenCS;
int  CCacheWriter::Open()
{
	if(!m_initialized || m_cacheLocation == "") return kErrNotInitialized;
	
	if (C_filePool.Has(m_dataFilePath)) return kSuccess;

	TRCSLock fplock(&OpenCS); // lock the open function
#if 1
	// check ghost data file
	if(GetFileAttributes(m_descFilePath) == 0xffffffff && GetFileAttributes(m_dataFilePath) != 0xffffffff)
	{
		int retry = 4;
		while(1)
		{
			// remove ghost data file
			if(DeleteFile(m_dataFilePath))
				break;

			if(retry <= 0) 
			{
				GetAqLogger()->LogMessageWithSysError("CCacheWriter::Open can not delete ghost data file:%s \n", m_dataFilePath);
				return kErrCannotOpenFile;
			}
			Sleep(250);
			retry--;
		}
	}
#endif

	if(!ReserveSpace()) return kErrNoDiskSpace;
	//gConfig.LogMessage(kErrorMsg, kLogToFile,  "INFO: Open cache:%s\n", m_cacheLocation);

	int status = OpenCacheFile (m_descFilePath, m_descSize); 
	if(status != kSuccess) return status;
	
	status = OpenCacheFile (m_dataFilePath, m_dataSize);
	if(status != kSuccess) 
	{
		C_filePool.Delete(m_descFilePath);
		return status;
	}

	return kSuccess;
}

int CCacheWriter::Close()
{
	// close cache will not disable cache activity permentely
	// cache can be reopened at any time after this closing.
	if(!m_initialized || m_cacheLocation == "") 
		return kSuccess;

	bool st1, st2;
	st1 = C_filePool.Delete(m_dataFilePath);
	st2 = C_filePool.Delete(m_descFilePath);
	return (st1 && st2)?kSuccess:kErrCannotCloseFile;
}

int CCacheWriter::MakeCacheFile(const char* filepath, ULONGLONG size, ULONGLONG& oFilePos)
{
	HANDLE hd = INVALID_HANDLE_VALUE;
	int retry = 4;
	while(1)
	{
		hd = CreateFile(filepath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hd != INVALID_HANDLE_VALUE)  break;

		if(retry <= 0) 
		{
			GetAqLogger()->LogMessageWithSysError("CCacheWriter::MakeCacheFile on file:%s \n", filepath);
			return kErrCannotOpenFile;
		}
		TRPlatform::MakeDirIfNeedTo(m_cacheLocation);
		Sleep(250);
		
		retry--;
	}
	
	Seek(hd, 0, FILE_END); // set to end of file for append mode
	oFilePos = GetPosition(hd);
	// preset the size to avoid fragment
	if(oFilePos < size)
	{
		Seek(hd, size, FILE_BEGIN);
		SetEndOfFile(hd);
	}
	CloseHandle(hd);

	//we may add defragment code here. 
	return kSuccess;

}

DWORD CCacheWriter::Seek(HANDLE hFile, LONGLONG nOffset, DWORD dwFrom)
{
	_ASSERTE(hFile != NULL);
	_ASSERTE(dwFrom == FILE_BEGIN || dwFrom == FILE_END || dwFrom == FILE_CURRENT);

	LARGE_INTEGER liOffset;
	liOffset.QuadPart = nOffset;
	return ::SetFilePointer(hFile, liOffset.LowPart, &liOffset.HighPart, dwFrom);
}

ULONGLONG CCacheWriter::GetPosition(HANDLE hFile)
{
	_ASSERTE(hFile != NULL);
	ULONGLONG nPos = 0;

	LARGE_INTEGER liOffset;
	liOffset.QuadPart = 0;
	liOffset.LowPart = ::SetFilePointer(hFile, 0, &liOffset.HighPart, FILE_CURRENT);
	if (liOffset.LowPart != INVALID_SET_FILE_POINTER)
	{
		nPos = liOffset.QuadPart;
	}
	return nPos;
}


//-----------------------------------------------------------------------------
int CCacheWriter::OpenCacheFile (const char* filename, ULONGLONG size)
{
	TRCSLock fplock(&OpenCS); // lock the open function
	if(C_filePool.Has(filename)) return kSuccess;
	
	int status;
	ULONGLONG oFilePos;
		
	status = MakeCacheFile(filename, size, oFilePos);
	if (status != kSuccess)	return status;

	HANDLE hd = INVALID_HANDLE_VALUE;
	int retry = 0;
	while(1)
	{
		// open in append mode
		hd = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hd != INVALID_HANDLE_VALUE)  break;
		if(retry >= 3) 
		{
			GetAqLogger()->LogMessageWithSysError("CCacheWriter::OpenCacheFile on file:%s \n", filename);
			return kErrCannotOpenFile;
		}
		Sleep(500);
		retry++;
	}
	
	// start from begining for new created file
	// or start from the last write for reopened file.
	Seek(hd, oFilePos, FILE_BEGIN);
	// can't use this, it cause fragment again 
	//SetEndOfFile(hd); // restore to actualy data end point

	if( !C_filePool.Create(filename, hd, &CloseLockedFile))
	{
		CloseLockedFile(hd);
		status = kErrCannotMapItOut;
		GetAqLogger()->LogMessage("Error CannotMapItOut CCacheWriter::OpenCacheFile on file:%s \n", filename);
	}
	return status;
}


static const unsigned int gBlockSize = 65536;

//-----------------------------------------------------------------------------
// Write all data in one call to make sure it is in one block
int CCacheWriter::Write(const char* filePath, unsigned char* iPixels, 
							  unsigned int iNumberOfBytes, ULONGLONG* oFilePos)
{
	// make sure cacahe is avaliable ...
	CCacheWriterLock lock(this);
	HANDLE hd = lock.Grab(filePath); // grab the stream handle to write to
	if (hd == NULL) 
	{	
		int status = kErrCannotOpenFile;
#if 0
		if( filePath == m_descFilePath )
			status = OpenCacheFile (m_descFilePath, m_descSize);
		else if(filePath == m_dataFilePath)
			status = OpenCacheFile (m_dataFilePath, m_dataSize);

#else
		status = Open();
#endif
		if(status != kSuccess) 
			return status;

		hd = lock.Grab(filePath);
	}

	if (hd == NULL) 
	{
		GetAqLogger()->LogMessage("Error CannotMapItOut CCacheWriter::Write on file:%s \n", filePath);
		return kErrCannotMapItOut;
	}
	
	// Get the start offset for this writing
	// in the case multip threads write to the same file
	if(oFilePos != NULL)
		*oFilePos = GetPosition(hd);

	DWORD byteswritten;
#if 1
	int blocks = iNumberOfBytes/gBlockSize;
	
	
	if(blocks < 1)
	{
		if(!WriteFile(hd, (char*)iPixels, iNumberOfBytes, &byteswritten, 0))
		{
			GetAqLogger()->LogMessageWithSysError("CCacheWriter::Write on file:%s \n", filePath);
			return kErrCacheNotWritten;
		}
		else
		{
			return kSuccess;
		}
	}

	if(!WriteFile(hd, (char*)iPixels, gBlockSize, &byteswritten, 0))
	{
		GetAqLogger()->LogMessageWithSysError("CCacheWriter::Write on file:%s \n", filePath);
		return kErrCacheNotWritten;
	}

	
	unsigned char*  p = iPixels+gBlockSize;
	for (int i=1; i<blocks; i++)
	{
		if(!WriteFile(hd, (char*)p, gBlockSize, &byteswritten, 0))
		{
			GetAqLogger()->LogMessageWithSysError("CCacheWriter::Write on file:%s \n", filePath);
			return kErrCacheNotWritten;
		}
		p += gBlockSize;
	}
	
	unsigned int lastBlockBytes = iNumberOfBytes - blocks*gBlockSize;
	if(lastBlockBytes > 0 && !WriteFile(hd, (char*)p, lastBlockBytes, &byteswritten, 0))
	{
		GetAqLogger()->LogMessageWithSysError("CCacheWriter::Write on file:%s \n", filePath);
		return kErrCacheNotWritten;
	}

	return kSuccess;

#else

	
	int retCode = WriteFile(hd, (char*)iPixels, iNumberOfBytes, &byteswritten, 0);
	if (retCode == 0)
	{
		GetAqLogger()->LogMessageWithSysError("CCacheWriter::Write on file:%s \n", filePath);
		return kErrCacheNotWritten;
	}
	else
	{
		return kSuccess;
	}
#endif
}
