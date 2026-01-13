/***********************************************************************
 * AqCore.h
 *---------------------------------------------------------------------
 * 
 *	Modification
 *  Will move some useful macro definition from AqNet common to AqCore.h
 *  so that they can be shared between WS and AqNet
 */


#ifndef AQCORE_H_
#define AQCORE_H_

#include <stdarg.h>

/* verbosity level: useful to control the amount of stuff that
 * gets logged
 */
enum 
{ 
	kErrorOnly	= 0, 
	kWarning	= 1, 
	kInfo		= 2, 
	kDebug		= 3, 
	kTrace		= 4
};


//------------------------------------------------------
/*
class AqLoggerInterface
{
public:

	AqLoggerInterface();
	virtual ~AqLoggerInterface(void);

	virtual void SetLogLevel(int iFlag);
	virtual int	GetLogLevel();

	virtual void LogMessage(const char *fmt, ...);
	virtual void LogMessage(int iLevel, const char *fmt, ...);
	virtual void LogMessageWithSysError(const char *fmt, ...);

	virtual void WriteLogMessage(const char* ifmt, va_list arguments, const char* iPrefix=0);
	virtual void FlushLog(void);
	virtual void RotateLog(unsigned iMaxLogSize=0);

};
*/

class AqLoggerInterface
{
public:

	AqLoggerInterface(void) {}
	virtual ~AqLoggerInterface(void) {}

	virtual void SetLogLevel(int iFlag) {};
	virtual int	GetLogLevel() {return 0;};

	virtual void LogMessage(const char *fmt, ...) {};
	virtual void LogMessage(int iLevel, const char *fmt, ...) {};
	virtual void LogMessageWithSysError(const char *fmt, ...) {};

	virtual void WriteLogMessage(const char* ifmt, va_list arguments, const char* iPrefix=0) {};
	virtual void FlushLog(void) {};
	virtual void RotateLog(unsigned iMaxLogSize=0) {};

};

// set up the library error logger, pass null pointer to turn off logging
extern "C" void SetAqLogger(AqLoggerInterface* iLogger);
extern "C" AqLoggerInterface* GetAqLogger();

// every application which uses COM system must define: CComModule _Module;

//------------------------------------------------------------------------------------------------
// AqLib default thread mode set up class

class AqCOMThreadInit
{
public:
	AqCOMThreadInit();
	~AqCOMThreadInit();

	static bool InitCOM();
	static void UnInitCOM();

private:
	long m_initFlag;
};

//------------------------------------------------------------------------------------------------
#define ASTRNCPY(t, s) {strncpy(t, s, sizeof(t)); t[sizeof(t)-1] = 0;}
#define AUSTRNCPY(t, s) {wcsncpy(t, s, sizeof(t)/sizeof(wchar_t)); t[sizeof(t)/sizeof(wchar_t) -1] = L'\0';}


#ifndef HDISK_SECTOR_SIZE
#  define HDISK_SECTOR_SIZE 512
#endif

#ifndef PAGE_ALIGNED
#  define PAGE_ALIGNED 4096
#endif

#ifndef PAGE_SIZE_MASK
#  define PAGE_SIZE_MASK (PAGE_ALIGNED-1)
#endif

#ifndef BYTE_ALIGNED
#  define BYTE_ALIGNED 16
#endif


class AqBuffer
{
public :
	AqBuffer();
	AqBuffer (const AqBuffer& iBuffer) ;

	virtual ~AqBuffer() { Reset(); }

	const AqBuffer& operator = (const AqBuffer& iBuffer);
	
	void  Reset();
	char* GetData() const { return (m_bufferAlignedPtr)?m_bufferAlignedPtr:m_bufferUnaligned; }
	virtual void * CopyBin(const void* iData, unsigned int iLen);

	int  GetbytesAllocated() const { return m_bytesAllocated; }
	bool IsPageAligned() const { return (((unsigned int)m_bufferAlignedPtr % PAGE_ALIGNED) == 0); }
	bool Is16BAligned() const { return (((unsigned int)m_bufferAlignedPtr % BYTE_ALIGNED) == 0); }
	bool Allocate(int iBytesToAllocate, bool iPageAlign=false, bool i16BAlign=false);
	bool InBuffer(const void* iptr) const;


	bool  m_bufferUsed;
    int   m_numberOfBytesStored;

protected:

	char* m_bufferUnaligned; 
	int	  m_bytesAllocated;
	char* m_bufferAlignedPtr;
};


#if 0
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

class AqStrBuffer : public AqBuffer
{
public :
	AqStrBuffer() : m_stringData(false), m_unicodeStr(false) {};
	AqStrBuffer (const AqStrBuffer& iBuffer) : AqBuffer(iBuffer) 
	{m_stringData=iBuffer.m_stringData; m_unicodeStr=iBuffer.m_unicodeStr;} 

	AqStrBuffer (const char* istr){	m_bufferUnaligned = 0; Reset(); CopyStr(istr);}
	AqStrBuffer (const wchar_t* iustr){	m_bufferUnaligned = 0; Reset(); CopyStr(iustr);}

	virtual ~AqStrBuffer(){};

	const AqStrBuffer& operator = (const AqStrBuffer& iBuffer) 
	{
		AqBuffer::operator=(iBuffer); 
		m_stringData=iBuffer.m_stringData; 
		m_unicodeStr=iBuffer.m_unicodeStr;
		return *this;
	}

	const AqStrBuffer& operator = (const char* istr) {CopyStr(istr); return *this;}
	const AqStrBuffer& operator = (const wchar_t* iustr) {CopyStr(iustr); return *this;}

	bool operator == (const AqStrBuffer & iBuffer ) const;
	bool operator != (const AqStrBuffer & iBuffer ) const { return !(*this == iBuffer);};

	bool operator == (const char* iStr ) const;
	bool operator != (const char* iStr ) const { return !(*this == iStr);};

	bool operator == (const wchar_t* iStr ) const;
	bool operator != (const wchar_t* iStr ) const { return !(*this == iStr);};
	
	bool Swap(AqStrBuffer& iBuffer);

	void * CopyBin(const void* iData, unsigned int iLen);
	char * CopyStr(const char* iString);
	wchar_t* CopyStr(const wchar_t* iuString);

	const char * GetString() const;
	operator const char *(void) const {return GetString();}

	const wchar_t * GetUString() const;
	operator const wchar_t * (void) const {return GetUString();}

	char * BinToHex(const void *iData=0, int iDataSize=0);
	void * HexToBin(const char* iString=0);

	wchar_t* ToUnicode (const char* iString=0, unsigned int iCodePage=0);
	char* ToMultiByte (const wchar_t* iuString=0, unsigned int iCodePage=0);

	char* TruncString (unsigned int iSize);
	wchar_t* TruncUnicode (unsigned int iSize);
	
	bool IsStringData() const {return m_stringData;}
	bool IsUnicodeStr() const {return m_stringData && m_unicodeStr;}

	void MarkBinData () { m_stringData = false;}
	void MarkStringData () { m_stringData = true; m_unicodeStr = false;}
	void MarkUnicodeData () { m_stringData = true; m_unicodeStr = true;}

protected:

	bool m_stringData;
	bool m_unicodeStr;
	
};

struct AqPairBuffer
{
	AqStrBuffer Key;
	AqStrBuffer Value;
};
#endif


class AqFileMapBuffer
{
public :
	AqFileMapBuffer() : m_hdFile(0), m_hdMap(0), m_buffer(0), m_attached(false) { m_filename[0] = 0; Reset();}
	AqFileMapBuffer (const AqFileMapBuffer& iBuffer) : m_hdFile(0), m_hdMap(0), m_buffer(0),  m_attached(false)
	{	
		m_filename[0] = 0;
		Reset();
		this->operator=(iBuffer);
	}

	virtual ~AqFileMapBuffer(){ Reset();}

	const AqFileMapBuffer& operator = (const AqFileMapBuffer& iBuffer);
	
	void Reset();

	// SH, 2006-06-29,
	// For online case, PE needs to load Mask file and then convert them to COF
	// At the same time, server needs to laod the mask file and apply the mask.
	// therefore, we need to open the file as readonly instead read&write.
	// but for some reason, even PE opens it as readonly, server still can not open the file
	// will leave the code there and need more investigation later.
	bool Allocate(long iBytesToAllocate, const char* iFilepath=0,const bool iOpenFileMapAsReadOnlyMode = false);
	
	char* GetData() const {return m_buffer;}
	long  GetSize() const {return m_size;}

protected:

	void *	m_hdFile;
	void *	m_hdMap;
	char*	m_buffer; 
	long	m_size;
	char	m_filename[256];
	bool	m_attached;
};


extern "C" char * AqBinToHex(const void *iData, int iDataSize, char *oStrBuf);
extern "C" void * AqHexToBin(const char *iHexStr, void *oDataBuf);

#ifndef _NO_SDK
extern "C" const char* GetCurrentProcessName();
#endif


#endif //AQCORE_H_
