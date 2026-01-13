/***********************************************************************
 * AqCore.cpp
 *---------------------------------------------------------------------
 *
 * 
 */

#include "AqCore.h"
#include <cassert>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define _WIN32_DCOM
#include "Windows.h"

#ifndef _NO_SDK
#  include <Psapi.h>
#  pragma comment(lib, "Psapi.lib")
#  pragma comment( lib, "ole32.lib" )
#endif

/*
#ifdef AQCORE_EXPORTS
AqLoggerInterface::AqLoggerInterface()
{
	int a = 0;
	++a;
}

//#error here

AqLoggerInterface::~AqLoggerInterface(void)
{
	int a = 0;
	++a;
}

void AqLoggerInterface::SetLogLevel(int iFlag) {}
int	AqLoggerInterface::GetLogLevel() { return -1; }

void AqLoggerInterface::LogMessage(const char *fmt, ...) {}
void AqLoggerInterface::LogMessage(int iLevel, const char *fmt, ...) {}
void AqLoggerInterface::LogMessageWithSysError(const char *fmt, ...) {}

void AqLoggerInterface::WriteLogMessage(const char* ifmt, va_list arguments, const char* iPrefix) {}
void AqLoggerInterface::FlushLog(void) {}
void AqLoggerInterface::RotateLog(unsigned iMaxLogSize) {}
#endif
*/



static AqLoggerInterface dumLogger;
AqLoggerInterface* pAqLogger = &dumLogger;

void SetAqLogger(AqLoggerInterface* iLogger) 
{
	if(iLogger)
		pAqLogger = iLogger;
	else
		pAqLogger = &dumLogger;
}

AqLoggerInterface* GetAqLogger() {return pAqLogger;}


bool AqCOMThreadInit::InitCOM()
{
	HRESULT		hr;
	hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) 
	{
		//ATLASSERT( 0 ); //show error in debug mode
		GetAqLogger()->LogMessage("Error: fail to CoInitializeEx: %d\n", hr);
		return false;
	}
	else
	{
		return true;
	}
}

void AqCOMThreadInit::UnInitCOM() {::CoUninitialize();}



// A class to quard COM thread initialization and uninitialization. 
// Every thread plan to use COM has to add AqCOMThreadInit as thread process variable 

AqCOMThreadInit::AqCOMThreadInit() : m_initFlag(0)
{
	if(InitCOM())
		m_initFlag = 1;

}

AqCOMThreadInit::~AqCOMThreadInit()
{
	if(m_initFlag)
		UnInitCOM(), m_initFlag=0; 

}

//-----------------------------------------------------------------------------
AqBuffer::AqBuffer()
{ 
	m_bufferUnaligned = 0; 
	Reset(); 
}

//-----------------------------------------------------------------------------

AqBuffer::AqBuffer (const AqBuffer& iBuffer) 
{
	m_bufferUnaligned = 0; 
	Reset();
	this->operator=(iBuffer);
} 

//-----------------------------------------------------------------------------

const AqBuffer& AqBuffer::operator=(const AqBuffer& iBuffer)
{
	if (Allocate(iBuffer.m_bytesAllocated, iBuffer.IsPageAligned()))
	{
		m_numberOfBytesStored = iBuffer.m_numberOfBytesStored;
		if(m_numberOfBytesStored > m_bytesAllocated)
			m_numberOfBytesStored = m_bytesAllocated;
		memcpy(GetData(), iBuffer.GetData(), m_numberOfBytesStored);

		m_bufferUsed = iBuffer.m_bufferUsed;
	}

	return *this;
}

//-----------------------------------------------------------------------------

void AqBuffer::Reset()
{
	if (m_bufferUnaligned)
		delete [] m_bufferUnaligned, m_bufferUnaligned = 0;
	
	m_bytesAllocated = 0;
	m_numberOfBytesStored = 0;
	
	m_bufferUsed = false;

	m_bufferAlignedPtr = 0;
}


//-----------------------------------------------------------------------------

bool AqBuffer::Allocate(int iBytesToAllocate, bool iPageAlign, bool i16BAlign)
{
	if(m_bufferUsed)
	{
		GetAqLogger()->LogMessage("Error: (AqBuffer::Allocate) buffer is in use\n");
		return false;
	}

	if((m_bytesAllocated >= iBytesToAllocate))
	{
		if (iPageAlign && iPageAlign == IsPageAligned())
			return true;
		else if (i16BAlign && i16BAlign == Is16BAligned())
			return true;
	}

	Reset();

	// this implementation suppose that PAGE_ALIGNED is a multiple of BYTE_ALIGNED
	assert((PAGE_ALIGNED % BYTE_ALIGNED) == 0);

	if(iPageAlign)
	{
		// prepare memory block can be page aligned and can be used in sector aligned
		// reading e.g. two more sectors.
		// it can read one extra sector in front and one in back,
		m_bufferUnaligned = new  char[iBytesToAllocate+PAGE_SIZE_MASK+2*HDISK_SECTOR_SIZE];
		
		//	Allocate and align the ptr to the next page boundary
		m_bufferAlignedPtr = (char *)((((unsigned int)m_bufferUnaligned) + PAGE_SIZE_MASK) & ~PAGE_SIZE_MASK);

		assert(IsPageAligned());
	}
	else if(i16BAlign)
	{
		m_bufferUnaligned = new  char[iBytesToAllocate + BYTE_ALIGNED];		
		unsigned long pos = reinterpret_cast<unsigned long>(m_bufferUnaligned);
		unsigned char offset = 0;
		if( (pos % BYTE_ALIGNED) != 0 )
		{
			unsigned long newpos = (1 + pos / BYTE_ALIGNED) * BYTE_ALIGNED;
			offset = (unsigned char)(newpos - pos);
		}
		
		m_bufferAlignedPtr = (char*)m_bufferUnaligned + offset;

		assert(Is16BAligned());
	}
	else
	{
		m_bufferUnaligned = new  char[iBytesToAllocate];
		m_bufferAlignedPtr = 0;
	}

	m_bytesAllocated = iBytesToAllocate;
	return true;
}

//-----------------------------------------------------------------------------

bool AqBuffer::InBuffer(const void* iptr) const
{
	char* pData = GetData();
	return (iptr && (iptr >= pData  && iptr <= pData+m_numberOfBytesStored) );
}

//-----------------------------------------------------------------------------

void * AqBuffer::CopyBin(const void* iData, unsigned int iLen)
{
	if(!iData || !Allocate(iLen, false))
		return 0;

	char* pstr = GetData();
	memcpy(pstr, iData, iLen);
	m_numberOfBytesStored = iLen;

	return pstr; 
}

#if 0
//-----------------------------------------------------------------------------

bool AqStrBuffer::operator == (const AqStrBuffer & iBuffer )  const
{
	return 
		( m_numberOfBytesStored != 0 &&
		m_numberOfBytesStored == iBuffer.m_numberOfBytesStored &&
		memcmp(GetData(), iBuffer.GetData(), m_numberOfBytesStored) == 0
		);


};

//-----------------------------------------------------------------------------

bool AqStrBuffer::operator == (const char* iStr )  const
{
	if(m_numberOfBytesStored == 0 && iStr && !iStr[0] )
		return true;

	return ( iStr && m_stringData && strcmp(iStr, GetString()) == 0);
};


//-----------------------------------------------------------------------------

bool AqStrBuffer::operator == (const wchar_t* iStr )  const
{
	if(m_numberOfBytesStored == 0 && iStr && wcslen(iStr) == 0  )
		return true;

	return ( iStr && m_unicodeStr && wcsicmp(iStr, GetUString()) == 0);
};

//-----------------------------------------------------------------------------

void * AqStrBuffer::CopyBin(const void* iData, unsigned int iLen)
{
	void *pstr = AqBuffer::CopyBin(iData, iLen);
	MarkBinData();
	return pstr; 
}


//-----------------------------------------------------------------------------

char * AqStrBuffer::CopyStr(const char* iString)
{
	if(!iString)
		iString = "";

	int nBytesSource = strlen(iString);
	if(nBytesSource > 128000000)
	{
		m_numberOfBytesStored = 0;
		GetAqLogger()->LogMessage("Error: (AqStrBuffer::CopyStr) string is too big: %d\n", nBytesSource);
		return 0;
	}


	if(!CopyBin(iString, nBytesSource+1))
		return 0;

	char* pstr = GetData();
	*(pstr + nBytesSource) = 0;
	MarkStringData();
	return pstr; 
}


//-----------------------------------------------------------------------------

wchar_t* AqStrBuffer::CopyStr(const wchar_t* iuString)
{
	if(!iuString)
		iuString = L"";

	int nBytesSource = (wcslen(iuString))*2;
	if(nBytesSource > 128000000)
	{
		m_numberOfBytesStored = 0;
		GetAqLogger()->LogMessage("Error: (AqStrBuffer::CopyStr) wide string is too big: %d\n", nBytesSource);
		return 0;
	}

	if(!CopyBin(iuString, nBytesSource+2))
		return 0;

	char* pstr = GetData();
	*(LPWSTR)(pstr + nBytesSource) = L'\0';

	MarkUnicodeData ();
	return (wchar_t*)pstr; 
}

//-----------------------------------------------------------------------------
const char * AqStrBuffer::GetString() const 
{
	if(m_numberOfBytesStored == 0)
		return "";

	return GetData();
}

//-----------------------------------------------------------------------------
const wchar_t * AqStrBuffer::GetUString() const 
{
	if(m_numberOfBytesStored == 0)
		return L"";

	return (const wchar_t *)GetData();
}


//-----------------------------------------------------------------------------

bool AqStrBuffer::Swap(AqStrBuffer& iBuffer)
{
	bool  saved_bufferUsed = iBuffer.m_bufferUsed;
    int   saved_numberOfBytesStored = iBuffer.m_numberOfBytesStored;

	char* saved_bufferUnaligned = iBuffer.m_bufferUnaligned; 
	int	  saved_bytesAllocated = iBuffer.m_bytesAllocated;
	char* saved_bufferPageAlignedPtr = iBuffer.m_bufferAlignedPtr;

	
	bool saved_stringData = iBuffer.m_stringData;
	bool saved_unicodeStr = iBuffer.m_unicodeStr;

	iBuffer.m_bufferUsed = m_bufferUsed;
    iBuffer.m_numberOfBytesStored = m_numberOfBytesStored;

	iBuffer.m_bufferUnaligned = m_bufferUnaligned; 
	iBuffer.m_bytesAllocated = m_bytesAllocated;
	iBuffer.m_bufferAlignedPtr = m_bufferAlignedPtr;

	iBuffer.m_stringData = m_stringData;
	iBuffer.m_unicodeStr = m_unicodeStr;

	this->m_bufferUsed = saved_bufferUsed;
    this->m_numberOfBytesStored = saved_numberOfBytesStored;

	this->m_bufferUnaligned = saved_bufferUnaligned; 
	this->m_bytesAllocated = saved_bytesAllocated;
	this->m_bufferAlignedPtr = saved_bufferPageAlignedPtr;
	
	this->m_stringData = saved_stringData;
	this->m_unicodeStr = saved_unicodeStr;


	return true;
}

//-----------------------------------------------------------------------------

char * AqStrBuffer::BinToHex(const void *iData, int iDataSize)
{

	AqStrBuffer  buf2;

	if(iData == 0 || iDataSize == 0)
	{
		iData = GetData();
		iDataSize = m_numberOfBytesStored;
		if(!iData)
			return 0;

	}

	if( InBuffer(iData) )
	{
		if(IsStringData())
			return (char *)iData;

		if(!Swap(buf2))
			return 0;
	}
	
	int hexSize = iDataSize*2+2+1;

	Allocate(hexSize, false);
	m_numberOfBytesStored = hexSize;

	MarkStringData ();

	return AqBinToHex(iData, iDataSize, GetData());
}

//-----------------------------------------------------------------------------

void * AqStrBuffer::HexToBin(const char* iString)
{
	if(iString == 0)
		iString = GetData();

	if(!iString)
		return 0;

	if( InBuffer(iString) )
	{
		if(!IsStringData())
			return (void *)iString;

		if(iString != GetData())
		{
			GetAqLogger()->LogMessage("Error: (AqStrBuffer::HexToBin) not start at head\n");
			return 0; // not start with head, can not do anything in place
		}

	}
	else
	{
		Allocate((strlen(iString)/2-1), false);
	}

	m_numberOfBytesStored = strlen(iString);
	if(m_numberOfBytesStored < 2)
	{
		m_numberOfBytesStored = 0;
		return 0;
	}

	m_numberOfBytesStored = (m_numberOfBytesStored/2)-1;

	MarkBinData();
	return AqHexToBin(iString, GetData()) ;
}


//-----------------------------------------------------------------------------

wchar_t* AqStrBuffer::ToUnicode (const char* iString, unsigned int iCodePage) 
{
	AqStrBuffer tmpBuf;

	if(iString == 0)
		iString = GetData();
	
	if(!iString)
		return 0;

	if(InBuffer(iString))
	{
		if(!IsStringData()) // not a string
		{
			GetAqLogger()->LogMessage("Error: (AqStrBuffer::ToUnicode) not a string\n");
			return 0;
		}

		if(IsUnicodeStr()) // no convert need
			return (wchar_t* )iString;

		if(!Swap(tmpBuf))
			return 0;
	}

	if(iCodePage == 0)
		iCodePage = CP_ACP; //CP_ACP is 0
	// Converts a string into a Unicode wide-character string
	// Adds zero-termination
	
	int nBytesSource = strlen(iString) * 2;
	// Query the number of WChars required to store the destination string
	int nWCharNeeded = MultiByteToWideChar (iCodePage, MB_PRECOMPOSED, iString, nBytesSource, NULL, 0);
	
	// Allocate the required amount of space plus 2 more bytes for '\0'
	if(!Allocate((nWCharNeeded + 1) * 2, false))
		return  0;

	LPWSTR pWideServer = (LPWSTR)(GetData());
	
	// Do the conversion
	nWCharNeeded = MultiByteToWideChar(iCodePage, MB_PRECOMPOSED, iString,
		nBytesSource, pWideServer, nWCharNeeded);
	
	*(pWideServer + nWCharNeeded) = L'\0';
	m_numberOfBytesStored = (nWCharNeeded + 1) * 2;

	MarkUnicodeData();

	return pWideServer;
}


//-----------------------------------------------------------------------------

char* AqStrBuffer::ToMultiByte (const wchar_t* iuString, unsigned int iCodePage) 
{
	AqStrBuffer tmpBuf;

	if(iuString == 0)
		iuString = (wchar_t*)GetData();
	
	if(!iuString)
		return 0;

	if(InBuffer(iuString))
	{
		if(!IsStringData()) // not a string
		{
			GetAqLogger()->LogMessage("Error: (AqStrBuffer::ToMultiByte) not a string\n");
			return 0;
		}

		if(!IsUnicodeStr()) // no convert need
			return (char*)iuString;

		if(!Swap(tmpBuf))
			return 0;
	}

	if(iCodePage == 0)
		iCodePage = CP_ACP; //CP_ACP is 0

	// Converts a string into a Unicode wide-character string
	// Adds zero-termination
	int nBytesSource = wcslen(iuString);
	// Query the number of WChars required to store the destination string
	int nCharNeeded = WideCharToMultiByte (iCodePage, WC_COMPOSITECHECK, iuString, -1, 0, 0, 0, 0);
	
	// Allocate the required amount of space plus 2 more bytes for '\0'
	Allocate(nCharNeeded + 1, false);
	char* pstr = GetData();
	
	// Do the conversion
	nCharNeeded = WideCharToMultiByte(iCodePage, WC_COMPOSITECHECK, iuString,
		nBytesSource, pstr, nCharNeeded, 0, 0);
	
	*(pstr + nCharNeeded) = 0;
	m_numberOfBytesStored = nCharNeeded + 1;

	MarkStringData();

	return pstr;
}


//-----------------------------------------------------------------------------

char* AqStrBuffer::TruncString (unsigned int iSize)
{
	char* p = GetData();
	if(!p)
		return p;

	if(!IsStringData()) // not a string
		return p;

	if (strlen(p) > iSize)
		p[iSize] = 0;
	return p;
}


//-----------------------------------------------------------------------------

wchar_t* AqStrBuffer::TruncUnicode (unsigned int iSize)
{
	wchar_t* p = (wchar_t* )GetData();

	if(!p)
		return p;

	if(!IsUnicodeStr()) // not a string
		return p;

	if (wcslen(p) > iSize)
		p[iSize] = 0;
	return p;
}
#endif


//-----------------------------------------------------------------------------
// Pack binary data into a hex string 
char * AqBinToHex(const void *iData, int iDataSize, char *oStrBuf) 
{
  static char hex[17] = "0123456789ABCDEF";

  if(!iData || iDataSize < 1 || !oStrBuf)
	  return 0;

  int i;
  const unsigned char *u = (const unsigned char *) iData;
  char *c = oStrBuf;
  register unsigned char uu;
  *(c++) = '0'; 
  *(c++) = 'x'; 	
  for (i = 0; i < iDataSize; i++,u++) 
  {
    uu = *u;
    *(c++) = hex[(uu & 0xf0) >> 4];
    *(c++) = hex[uu & 0xf];
  }
  *(c) = 0;
  return oStrBuf;
}


//-----------------------------------------------------------------------------
// Unpack binary data from a hex string
void * AqHexToBin(const char *iHexStr, void *oDataBuf) 
{
	if(!iHexStr || !oDataBuf || strlen(iHexStr) < 2 )
	  return 0;

  register unsigned char uu = 0;
  register int d;
  unsigned char *u = (unsigned char *) oDataBuf;
  const char *c = iHexStr+2;
  int i, sz;
  sz = (strlen(iHexStr)-2)/2;
  for (i = 0; i < sz; i++, u++) 
  {
    d = *(c++);
    if ((d >= '0') && (d <= '9'))
      uu = ((d - '0') << 4);
    else if ((d >= 'A') && (d <= 'F'))
      uu = ((d - ('A'-10)) << 4);

    d = *(c++);
    if ((d >= '0') && (d <= '9'))
      uu |= (d - '0');
    else if ((d >= 'A') && (d <= 'F'))
      uu |= (d - ('A'-10));

    *u = uu;
  }
  return oDataBuf;
}


//-----------------------------------------------------------------------------

#ifndef _NO_SDK

const char* GetCurrentProcessName()
{
	//char* cline = GetCommandLine();
	static char szProcessName[256] = "";

	if(szProcessName[0] != 0)
		return szProcessName;

    unsigned long cbNeeded, lRetVal;
    HMODULE hMod[4];

	
	HANDLE hProcess = GetCurrentProcess();
    lRetVal = EnumProcessModules(hProcess, hMod, 4, &cbNeeded);
    if (lRetVal == 1) 
	{
		lRetVal = GetModuleBaseName(hProcess, hMod[0], szProcessName, sizeof(szProcessName));
		if (lRetVal > 0) 
		{
			char* pext = strrchr(szProcessName, '.');
			if(pext)
				*pext = 0;
			else
				szProcessName[255] = 0;
		}

	}
    

	return szProcessName;
}

//-----------------------------------------------------------------------------


/*//static DWORD c_SysGran = 0;
	// Get the system allocation granularity.
	if(c_SysGran == 0)
	{
		
		SYSTEM_INFO SysInfo; 
		GetSystemInfo(&SysInfo);
		c_SysGran = SysInfo.dwAllocationGranularity;
	}

*/
//-----------------------------------------------------------------------------

const AqFileMapBuffer& AqFileMapBuffer::operator=(const AqFileMapBuffer& iBuffer)
{
	if(iBuffer.m_attached)
	{
		Allocate(iBuffer.m_size, iBuffer.m_filename);
		
	}
	else
	{
		if (Allocate(iBuffer.m_size))
			memcpy(m_buffer, iBuffer.m_buffer, m_size);
	}

	return *this;
}


//-----------------------------------------------------------------------------

void AqFileMapBuffer::Reset()
{
	m_size = 0;

	if(m_buffer)
		UnmapViewOfFile(m_buffer), m_buffer=0;
	
	if(m_hdMap)
		CloseHandle(m_hdMap), m_hdMap=0;

	if(m_hdFile)
	{
		CloseHandle(m_hdFile), m_hdFile=0;
	}

	if(!m_attached && m_filename[0])
		DeleteFile(m_filename);

	m_filename[0] = 0;
	m_attached = false;
}


//-----------------------------------------------------------------------------
#include "TRPlatform.h"
bool AqFileMapBuffer::Allocate(long iBytesToAllocate, const char* iFilepath/*=0*/, 
							   const bool iOpenFileMapAsReadOnlyMode /*=false*/)
{
	Reset();
	
	// SH, 2006-07-14, Check available memory space
	if( GetAqLogger()->GetLogLevel() >= kInfo)
	{
		int availablePhysicalMemory = TRPlatform::GetRAM()->m_availablePhysical;
		int availableVirtualMemory = TRPlatform::GetRAM()->m_availableVirtual;
		GetAqLogger()->LogMessage("Info: AqFileMapBuffer::Allocate: Before allocate %l (Bytes) - "
			"Available physical memory = %d(Mbytes), available virtual memory = %d(Mbytes)\n",
			iBytesToAllocate,availablePhysicalMemory, availableVirtualMemory);
	}

	if(iBytesToAllocate < 1 && iFilepath==0)
	{
		GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate get invalid allocate size: %d\n", iBytesToAllocate);
		return false;
	}
	
	if(iFilepath == 0)
	{
		char curDir[MAX_PATH];
		if(GetCurrentDirectory(MAX_PATH, curDir) == 0 || (strlen(curDir) < 4 && GetTempPath(MAX_PATH, curDir) == 0))
		{
			GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate fail to make up temp directory\n");
			return false;
		}

		if(!GetTempFileName(curDir, "AqBuffer", 0, m_filename))
			return false;

		// Create the test file. Open it "Create Always" to overwrite any
		// existing file. The data is re-created below.
		m_hdFile = CreateFile(m_filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if (m_hdFile == INVALID_HANDLE_VALUE) 
		{
			m_hdFile = 0;
			m_filename[0] = 0;
			GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate fail to create file %s\n", m_filename);
			return false;
		}

	}
	else
	{
		ASTRNCPY(m_filename, iFilepath);

		// Create the test file. Open existing file.
		if ( iOpenFileMapAsReadOnlyMode )
		{
			m_hdFile = CreateFile(m_filename, GENERIC_READ , 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		else
		{
			m_hdFile = CreateFile(m_filename, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		}

		if (m_hdFile == INVALID_HANDLE_VALUE) 
		{
			m_hdFile = 0;
			m_filename[0] = 0;
			GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate fail to open file %s\n", m_filename);
			return false;
		}

		// get the file size 
		if(iBytesToAllocate < 1)
		{
			BY_HANDLE_FILE_INFORMATION fInfo;
			if(!GetFileInformationByHandle(m_hdFile, &fInfo) && fInfo.nFileSizeLow <= 0)
			{
				m_hdFile = 0;
				m_filename[0] = 0;
				GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate can't attach to zero size file: %s\n", m_filename);
				return false;
			}

			iBytesToAllocate = fInfo.nFileSizeLow;
		}

		if (iBytesToAllocate < 1)
		{
			m_hdFile = 0;
			m_filename[0] = 0;
			GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate can't attach to zero size file: %s\n", m_filename);
			return false;
		}
	
		m_attached = true;
	}



	// Create a file-mapping object for the file.
	if ( iOpenFileMapAsReadOnlyMode )
	{
		m_hdMap = CreateFileMapping( m_hdFile, // current file handle
									NULL, // default security
									PAGE_READONLY, // read permission
									0,  // size of mapping object, high
									iBytesToAllocate, // size of mapping object, low
									0); // name of mapping object
	}
	else
	{
		m_hdMap = CreateFileMapping( m_hdFile, // current file handle
									NULL, // default security
									PAGE_READWRITE, // read/write permission
									0,  // size of mapping object, high
									iBytesToAllocate, // size of mapping object, low
									0); // name of mapping object
	}

	if (m_hdMap == 0) 
	{
		Reset();
		GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate fail to map file %s\n", m_filename);
		return false;
	}

	// Map the view and test the results.
	if ( iOpenFileMapAsReadOnlyMode )
	{
		m_buffer = (char*)MapViewOfFile(m_hdMap, // handle to mapping object
								   FILE_MAP_READ, // read/write permission 
								   0, // high-order 32 bits of file offset
								   0, // low-order 32 bits of file offset
								   0); // number of bytes to map
	}
	else
	{
		m_buffer = (char*)MapViewOfFile(m_hdMap, // handle to mapping object
								   FILE_MAP_ALL_ACCESS, // read/write permission 
								   0, // high-order 32 bits of file offset
								   0, // low-order 32 bits of file offset
								   0); // number of bytes to map
	}
	
	
	if (m_buffer == 0) 
	{
		Reset();
		GetAqLogger()->LogMessage("Error: AqFileMapBuffer::Allocate fail to map view file %s\n", m_filename);
		return false;
	}


	m_size = iBytesToAllocate;	
	if( GetAqLogger()->GetLogLevel() >= kInfo)
	{
		int availablePhysicalMemory = TRPlatform::GetRAM()->m_availablePhysical;
		int availableVirtualMemory = TRPlatform::GetRAM()->m_availableVirtual;
		GetAqLogger()->LogMessage("Info: AqFileMapBuffer::Allocate: After allocate - "
			"Available physical memory = %d(Mbytes), available virtual memory = %d(Mbytes)\n",
			availablePhysicalMemory, availableVirtualMemory);

	}
	return true;
}
#endif