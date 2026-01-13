/***********************************************************************
 * $Id: CCacheWriter.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used to create the Terarecon Cache for incoming 
 *		DICOM Series.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

//-----------------------------------------------------------------------------

#if !defined(CCacheWriter_H)
#define CCacheWriter_H

#include "rtvPoolAccess.h"
#include "AqCore/AqString.h"

void CloseLockedFile(HANDLE& hd);
class CFilePool : public RTVPoolAccess<AqString, HANDLE>
{
public:	
	CFilePool() {};
	virtual ~CFilePool() {};
};


//-----------------------------------------------------------------------------
class CCacheWriter  
{
friend class CCacheWriterLock; //help class to acquire a file lock, and automatic release the lock

public:
	CCacheWriter();
	virtual ~CCacheWriter();

	int  Setup(const char* cacheLocation, 
			   const char* dataFileName,
			   ULONGLONG dataSize,
			   const char* descFileName = NULL,
			   ULONGLONG descSize = 0);

	void  Reset();
	virtual bool  ReserveSpace() = 0;

	const char* GetLocation() { return  m_cacheLocation;};
	const char* GetDataPath() { return  m_dataFilePath;};
	const char* GetDescPath() { return  m_descFilePath;};
	bool IsSameLocation(const char* location) {return (m_cacheLocation != location);};

	int Open();
	int Close();

protected:
	int  OpenCacheFile(const char* filename, ULONGLONG size);
	int MakeCacheFile(const char* filepath, ULONGLONG size, ULONGLONG& oFilePos);
	ULONGLONG GetPosition(HANDLE hFile);
	DWORD Seek(HANDLE hFile, LONGLONG nOffset, DWORD dwFrom = FILE_CURRENT);

	int Write (const char* filePath, 
		             unsigned char* iPixels, 
					 unsigned int iNumberOfBytes, 
					 ULONGLONG*  oFilePos=NULL);

	static CFilePool C_filePool; 	// classwide cache pool
	AqString	m_cacheLocation;
	AqString	m_dataFilePath;
	ULONGLONG   m_dataSize;
	AqString	m_descFilePath;
	ULONGLONG   m_descSize;
	bool		m_initialized;
};

//-----------------------------------------------------------------------------
// Help class to provid CCacheWriter access control
class CCacheWriterLock : public RTVObjectAutoLock<AqString, HANDLE>
{
public:
	CCacheWriterLock(CCacheWriter* pc):RTVObjectAutoLock<AqString, HANDLE>(&(pc->C_filePool)) {};
	virtual ~CCacheWriterLock() {};
private:
};



#endif // !defined(CCacheWriter_H)
