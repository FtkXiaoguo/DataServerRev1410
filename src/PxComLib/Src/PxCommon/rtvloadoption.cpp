/***********************************************************************
 * rtvloadoption.cpp
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
 
#include "rtvloadoption.h"
#include "AqCore/TRAtomic.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <share.h>

#include "rtvsfileGuard.h"
#include "rtvsutil.h"

//-----------------------------------------------------------------------------
// keeps the value and does the proper conversion
static const int kMaxValueLengthInString = (1024+4);
class NVValue
{
public:
	enum 
	{
		kUnknown, kBool, kShort, kUShort, kInt, kUInt,kLong, kULong,
		kFloat, kDouble, kString,kUString, kChar,kUChar
	};
	
	NVValue(short*	iV=0,unsigned m=0)			{ m_value = (void *)iV; m_type = kShort;	m_mask=m;}
	NVValue(bool*	iV, unsigned m=0)			{ m_value = (void *)iV; m_type = kBool;		m_mask=m;}
	NVValue(int*	iV,	unsigned m=0)			{ m_value = (void *)iV; m_type = kInt;		m_mask=m;}
	NVValue(long*	iV,	unsigned m=0)			{ m_value = (void *)iV; m_type = kLong;		m_mask=m;}
	NVValue(float*	iV,	unsigned m=0)			{ m_value = (void *)iV; m_type = kFloat;	m_mask=m;}
	NVValue(double*	iV,	unsigned m=0)			{ m_value = (void *)iV; m_type = kDouble;	m_mask=m;}
	NVValue(char&	iV,unsigned m=0)			{ m_value = (void *)&iV;m_type = kChar;		m_mask=m;}
	NVValue(unsigned char&	iV, unsigned m=0)	{ m_value = (void *)&iV;m_type = kUChar;	m_mask=m;}
	NVValue(unsigned short* iV,	unsigned m=0)	{ m_value = (void *)iV; m_type = kUShort;	m_mask=m;}
	NVValue(unsigned int*	iV,	unsigned m=0)	{ m_value = (void *)iV; m_type = kUInt;		m_mask=m;}
	NVValue(unsigned long*	iV,	unsigned m=0)	{ m_value = (void *)iV; m_type = kULong;	m_mask=m;}
	NVValue(char* iV, int iLen=0,unsigned m=0)  { m_value = (void *)iV; m_type = kString; m_length = iLen;m_mask=m;}
	NVValue(unsigned char* iV, int iLen=0,unsigned m=0)  { m_value = (void *)iV; m_type = kUString; m_length = iLen;m_mask=m;}

	// conversions
	void	Convert(const char* iValue);		// from string to value
	char*	ConvertS(char* oS, int n=kMaxValueLengthInString);		// from value to string
	inline unsigned GetMask(void) const { return m_mask;}
private:
	void*			m_value;
	int				m_type;
	int				m_length;
	unsigned int	m_mask;
};

// char* comparison
class CharStarCMP
{
public:
	CharStarCMP(int caseSensitive=0) { m_case = caseSensitive;}
	~CharStarCMP() {}
	bool operator()(const char* A,const char *B) const {return (m_case?strcmp:stricmp)(A,B)<0;}
private:
	int m_case;
};


#pragma warning (disable: 4786)
#pragma warning (disable: 4616)

#include <map>
#include <string>
using std::map;
using std::string;

typedef map<const char*, NVValue,CharStarCMP> NameValue;

class iRTVOptionsInternal
{
public:
	iRTVOptionsInternal(int caseSensitive=0) : m_pair(CharStarCMP(caseSensitive)) { }
	~iRTVOptionsInternal(void) 
	{
		for (; m_generatedVal.size() > 0;)
		{
			free(*m_generatedVal.begin());
			m_generatedVal.erase(m_generatedVal.begin());
		}
	}

	NameValue	m_pair;
	operator NameValue& (void) { return m_pair;}
	string		m_theData;
	std::vector<char*>			m_generatedVal;
	std::map<std::string,int>	m_generatedName;
};


iRTVOptions::iRTVOptions(int iCaseSensitive)
{
	m_implementation = new iRTVOptionsInternal(iCaseSensitive);
	m_commentChar = '#'; 
	m_seperatorChar='=';
	m_isCaseSensitive = iCaseSensitive;
	m_bits = 0;
	m_buf = 0;
	m_length = 0;

#ifdef _DEBUG
	m_verbose = 1;
#else
	m_verbose = 0;
#endif

}

#ifdef _WIN32
#define snprintf _snprintf
#endif

//--------------------------------------------------------------------------------
int iRTVOptions::SetLogLevel(int iLevel)
{
	int ret = m_verbose;
	m_verbose = iLevel;
	return ret;
}


//--------------------------------------------------------------------------------
iRTVOptions::~iRTVOptions(void)
{
	if (m_implementation)
		delete m_implementation, m_implementation = 0;

	CleanLoadedData();

}

//--------------------------------------------------------------------------------
// Both the copy constructor and = operator purposely not
// copying the map entries
iRTVOptions::iRTVOptions(const iRTVOptions& iOpt)
{
	*this = iOpt;
	m_implementation = new iRTVOptionsInternal(!iOpt.IsCaseSensitive());
}

//--------------------------------------------------------------------------------
iRTVOptions& iRTVOptions::operator=(const iRTVOptions& iOpt)
{
	m_commentChar	= iOpt.GetCommentChar();
	m_seperatorChar = iOpt.GetSeperatorChar();
	m_verbose		= iOpt.GetVerbose();
	m_isCaseSensitive	= iOpt.IsCaseSensitive();
	m_buf = 0;
	m_length = 0;
	return *this;
}

//--------------------------------------------------------------------------------
void iRTVOptions::CleanLoadedData(void)
{
	if (m_buf && m_length> 0)
		delete []m_buf, m_buf = 0;

	m_length = 0;
}

//--------------------------------------------------------------------------------
int iRTVOptions::GetCount(void) const
{
	return m_implementation->m_pair.size();
}

//--------------------------------------------------------------------------------
const char* iRTVOptions::GetOptionName(int iC) const
{
	if (iC < 0 || iC >= GetCount())
		return "";

	NameValue::iterator it = m_implementation->m_pair.begin();
	for ( ; --iC >= 0; )
		it++;

	return it->first;
}

//--------------------------------------------------------------------------------
const char*	iRTVOptions::GetOptionValue(const char *iOption)
{
	static char sBuf[8][256];
	static TRAtomicVar<int> sSeq;
	int seq = ++sSeq;
	char  s[kMaxValueLengthInString], *sbuf = sBuf[seq&7];
	NameValue::iterator p;

	sbuf[0] = '\0';
	if ((p = m_implementation->m_pair.find(iOption)) != m_implementation->m_pair.end())
	{
		snprintf(sbuf, sizeof (sBuf[0]), "%s", p->second.ConvertS(s));
	}

	sbuf[sizeof sBuf[0] - 1] = '\0';
	return (const char*) sbuf;
}

//--------------------------------------------------------------------------------
void iRTVOptions::Add(const char* iN, int*  iV, unsigned m)			{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, long* iV, unsigned m)			{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, bool* iV, unsigned m)			{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, unsigned int* iV, unsigned m)	{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, unsigned long*iV, unsigned m)	{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, short* iV, unsigned m)		{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, float* iV, unsigned m)		{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, double* iV, unsigned m)		{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, unsigned short* iV, unsigned m)	{m_implementation->m_pair[iN] = NVValue(iV,m);}
void iRTVOptions::Add(const char* iN, char* iV, int iLen, unsigned m)	{m_implementation->m_pair[iN] = NVValue(iV, iLen,m);}
void iRTVOptions::Add(const char* iN, unsigned char* iV, int iLen, unsigned m)	{m_implementation->m_pair[iN] = NVValue(iV, iLen,m);}

//--------------------------------------------------------------------------------
int iRTVOptions::Remove(const char* iN) 
{
	if (m_implementation->m_pair.find(iN) == m_implementation->m_pair.end())
		return -1;
	m_implementation->m_pair.erase(iN);
	return 0;
}


//--------------------------------------------------------------------------------
// remove white spaces from both ends of a string
#include <ctype.h>
static inline char* DeSpaceDe(char *s, int replaceInPlace=1)
{
	if(*s == 0)
		return s;
	char * p = s + strlen(s) -1, *sh = s;
	

	for (; *s && p >= s && iswspace(*p); --p )
		;
	*++p = '\0';
	
	for ( ; *s && s < p && iswspace(*s); ++s)
		;
	return replaceInPlace ? strcpy(sh,s):s;
}



//--------------------------------------------------------------------------------
static int Split(const char *buf, char *name, char *value, int n, int seperator)
{
	char *sep;
	int str_len = strlen(buf);
	if(str_len<1) return 0;

	char *_str_buf = new char[str_len+1];
	strcpy(_str_buf,buf);
	if (!(sep = strchr(_str_buf,seperator))){
		delete [] _str_buf;
		return 0;
	}

	*sep = '\0';
	snprintf(name, n, "%s", _str_buf);
	snprintf(value, n, "%s", sep+1);
	name[ n - 1] = value[n - 1] = '\0';
	DeSpaceDe(name);
	DeSpaceDe(value);

	delete [] _str_buf;
	return 1;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
//--------------------------------------------------------------------------------
int	iRTVOptions::Load(const char *iFileName, int iLock, bool iGenKeys)
{
	FILE *fp = 0;
	iRTVSFileGuard openFiles;

	
	if (!iFileName || !*iFileName)
	{
		if (m_verbose)
			fprintf(stderr,"bad filename %s\n", iFileName ? iFileName:"null");
		return kBadArgument;
	}

	struct stat statBuf;
				
	if(::stat(iFileName, &statBuf)) 
	{
#ifdef _DEBUG
		if (m_verbose)
			fprintf(stderr,"can't stat %s\n", iFileName);
#endif
		return kNoFile;
	}

//	DWORD fileAttrib = GetFileAttributes(iFileName);
//	if (fileAttrib & FILE_ATTRIBUTE_SYSTEM)
//	{
//		return kSystemFile;
//	}

	if (iLock > 0)
	{
		for ( ; !fp && --iLock >= 0;)
		{
			fp = _fsopen(iFileName, "r",_SH_DENYWR);
			if (!fp) Sleep(80);
		}
	}
	else 
	{
		fp = fopen(iFileName, "r");
	}

	if ( !fp )
	{
#ifdef _DEBUG
		if (m_verbose)
			fprintf(stderr,"can't open %s\n", iFileName);
#endif
		return kErrOpen;
	}

	openFiles.AddOpenFile(fp);

	CleanLoadedData();
   
	m_buf = new char[statBuf.st_size+2];

	if (!m_buf)
	{
		fprintf(stderr,"Can't allocate %d bytes for RTVOption::Load()\n");
		return kErrAlloc;
	}

	m_length = statBuf.st_size+1;

	int n = fread(m_buf,1,statBuf.st_size,fp);

	assert(n <= statBuf.st_size);

	openFiles.CloseOpenFiles();
	 
	// n & statBuf.st_size may or may not be same depending on the tb interpretion
	// of the file. We always use n;
	m_buf[statBuf.st_size] = '\0';

	if (n >= 0) 
	{
		m_buf[n] = '\0';
		m_length = n;
	}

	if ( n < 0)
		return kErrRead;

	int status = Parse(m_buf, iGenKeys);
	return status;
}


 
#include <Windows.h>
//--------------------------------------------------------------------------------
// Only load existing file with limited line length 200 bytes
//
//multi-node case - chetan
//int	iRTVOptions::LoadWithLock  (const char*	iFileName, bool iGenKeys, int iSharingOption, const char* iConstraint)

int	iRTVOptions::LoadWithLock  (const char*	iFileName, bool iGenKeys, int iSharingOption)
{
	if (!iFileName ) {
		if (m_verbose)
			fprintf(stderr,"File name is null\n");
		return -1;
	}

	// Try to get file size to allow to allocate buffer below
	struct _stat attr;
	int result = _stat( iFileName, &attr );
	if( result != 0 || attr.st_size == 0)
	{
		if (m_verbose)
			fprintf(stderr,"Error to get file %s attributes\n",iFileName );
		return -1;
	}

	//	Try to open an existing file, if not existing, return   
	//chetan - added key to control file sharing option.
	HANDLE handle;
	switch(iSharingOption)
	{
		//allows shared file read only
	case iRTVOptions::kLoadOptionSharedReadOnly:
		 handle = CreateFile(iFileName, GENERIC_READ , FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		break;

		//allows shared file write only
	case iRTVOptions::kLoadOptionSharedWriteOnly:
		handle = CreateFile(iFileName, GENERIC_READ , FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		break;

		//does not allow shared file writing or reading
	case iRTVOptions::kLoadOptionNoSharedReadWrite:
		//multi-node case - chetan
		//handle = CreateFile(iFileName, GENERIC_READ , FILE_SHARE_DELETE | 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		handle = CreateFile(iFileName, GENERIC_READ , 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
	
		//deafult is shared file read only.
	default:
		handle = CreateFile(iFileName, GENERIC_READ , FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
	}
 		
	if (handle == INVALID_HANDLE_VALUE)
	{
		if (m_verbose)
			fprintf(stderr,"Can't open %s, file may not be existing\n", iFileName);
		return -1;
	}

	//	We should now have a valid file handle.  Let's lock it.  Then we can read from it, then unlock. 
	OVERLAPPED offset;
	// lock
	offset.Offset = offset.OffsetHigh = 0;
	BOOL noError = LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
 	if (!noError)
	{
		CloseHandle(handle);
		if (m_verbose)
			fprintf(stderr,"Can't lock file %s\n", iFileName);
		return -1;
	}
	

	// Read 
	DWORD  dwBytesRead ;
    char *buff = new char[attr.st_size+1];
	try{
		// Try to read all file to buffer.  
		if (!ReadFile(handle, buff, attr.st_size, &dwBytesRead, NULL) || dwBytesRead <= 0) 
		{  
			delete []buff; buff = 0;
			UnlockFileEx(handle, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
			CloseHandle(handle);
			if (m_verbose)
				fprintf(stderr,"Read from file %s failed with bytes read %l\n", iFileName,dwBytesRead);
			return -1;
		}
		buff[attr.st_size] = '\0';
		if(dwBytesRead > 0) buff[dwBytesRead] = '\0';
	}
	catch (...){
	 	if (m_verbose > 0)
			fprintf(stderr,"Exception occured when reading from file %s", iFileName);
	}

/*	if (strlen(iConstraint) > 0)
	{
		char newFileName[128];

		ASTRNCPY(newFileName, iFileName);

		strcat(newFileName, ".");
		strcat(newFileName, iConstraint);

		MoveFileEx(iFileName, newFileName, MOVEFILE_REPLACE_EXISTING);
	}*/
	 
	//	Unlock and close the file
	noError = UnlockFileEx(handle, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
	CloseHandle(handle);
	if (!noError)               
	{
		if (m_verbose)
			fprintf(stderr,"Failed to unlock file %s\n", iFileName);
		delete []buff; buff = 0;
		return -1;
	}

	int status = Parse(buff, iGenKeys);
	delete []buff; buff = 0;

  
#if 0
	NameValue::iterator p;
	fprintf(stdout,"count = %d\n\n", GetCount());
	char s[30];
	fprintf(stdout,"\n\n--------- Configuration Values ---------\n\n");
	for ( p = m_implementation->m_pair.begin(); p != m_implementation->m_pair.end(); p++)
		fprintf(stdout, "%s = %s\n", p->first, p->second.ConvertS(s));
	fprintf(stdout,"----------------------------------------\n\n");

 
 //	SaveWithLock (iFileName);
#endif

	return status>=0? 0:-1;
 
}

//--------------------------------------------------------------------------------
// Save only if the orignal file is existing.
// Case 1. File load is ok --> modify some values --->try to save back but original file deleted/moved out by other process.
//
int	iRTVOptions::SaveWithLock  (const char*	iFileName, bool iCreateIfNotThere, const char* iOptionName)
{
	if (!iFileName ) {
		if (m_verbose)
			fprintf(stderr,"File name is null\n");
		return -1;
	}

	DWORD dwCreationDisposition = iCreateIfNotThere ? OPEN_ALWAYS : OPEN_EXISTING;

	//	Try to open an existing file, if not existing, return   
 	HANDLE handle = CreateFile(iFileName,  GENERIC_WRITE,  0,
		NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		if (m_verbose)
			fprintf(stderr,"Can't open %s, file may not existing\n", iFileName);
		return 0; // save will do nothing if file is not existing but it needs different return value. 
	}

	//	We should now have a valid file handle.  
	//	Try to lock
	OVERLAPPED offset;
	offset.Offset = offset.OffsetHigh = 0;
	BOOL ret = LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
 	if (!ret)
	{
		CloseHandle(handle);
		if (m_verbose)
			fprintf(stderr,"Can't lock file %s\n", iFileName);
		return -1;
	}

 
	// Now it is time to write based on updated m_implementation
	NameValue::iterator p; 
	char tempBuf [kMaxValueLengthInString], s[kMaxValueLengthInString];
	if(!iOptionName) iOptionName = "";
	try{
		DWORD  bytesWritten;
		for ( p = m_implementation->m_pair.begin(); p != m_implementation->m_pair.end(); p++)
		{
			tempBuf[0] = 0; s[0] = 0;
			snprintf(tempBuf, sizeof (tempBuf), "%s%s %c %s\r\n", iOptionName, p->first, m_seperatorChar, p->second.ConvertS(s));
			ret = WriteFile(handle, tempBuf, strlen(tempBuf), &bytesWritten, 0);
			if (!ret || bytesWritten != strlen(tempBuf))
			{
				UnlockFileEx(handle, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
				CloseHandle(handle);
				if (m_verbose)
					fprintf(stderr,"Write to file %s failed with bytes written %l\n", iFileName,bytesWritten);
				return -1;
			}
		}
		ret = SetEndOfFile(handle); // need to get ride of old entries left.
		if(!ret)
		{
			UnlockFileEx(handle, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
			CloseHandle(handle);
			if (m_verbose)
				fprintf(stderr,"Set end of file for file %s failed\n", iFileName);
			return -1;
		}
	}
	catch (...) 
	{
		if (m_verbose)
			fprintf(stderr,"Exception occured when writing to file %s", iFileName);
	}
		//	Unlock and close the file
	BOOL noError = UnlockFileEx(handle, 0, 0xFFFFFFFF, 0x7FFFFFFF, &offset);
	CloseHandle(handle);
    if (!noError)               
	{
		if (m_verbose)
				fprintf(stderr,"Failed to unlock file %s\n", iFileName);
			return -1;
    }

	return 0;
}

//--------------------------------------------------------------------------------
static const char* GetNextLine(const char* iBytes, char* buf, int bufLen )
{
	if (!iBytes) 
	{
		*buf = '\0';
		return 0;
	}

	for (int i = 0; *iBytes && (*iBytes != '\n' && *iBytes != '\r') && i < bufLen; ++i)
	{
		*buf++ = *iBytes++;
	}

	*buf = '\0';
	return *iBytes ? iBytes+1 : 0;
}

//--------------------------------------------------------------------------------
int	iRTVOptions::Parse(const char *iBytes, int iGenKeys)
{
	NameValue::iterator p;
	char buf[kMaxValueLengthInString], name[kMaxValueLengthInString], value[kMaxValueLengthInString];
	const char* head = iBytes;
	int n;
	
	if (!iBytes || !*iBytes)
	{
		if (m_verbose)
			fprintf(stderr,"bad data\n");
		return kErrFormat;
	}

	m_bits = 0;
	
	for ( n = 0 ; (head = GetNextLine(head,buf, sizeof buf)) || strlen(buf);)
	{
		if (buf[0] == m_commentChar || strlen(buf) < 4) 
			continue; // ignore comments and empty lines

		if (Split(buf,name, value, sizeof name, m_seperatorChar))
		{
			if ((p = m_implementation->m_pair.find(name)) != m_implementation->m_pair.end() &&value[0] != '\0')
			{
				p->second.Convert(value);
				m_bits |= p->second.GetMask();
				n++;
			}
			else
			{
				if (m_verbose > 0 && !iGenKeys)
					fprintf(stderr,"%s: %s = %s\n", value[0] ? "UnknownToken":"EmptyValue",name, value);
			
				if (iGenKeys)
				{
					char *newVal = (char*)malloc(kMaxValueLengthInString);
					char *newName = 0;

					newVal[0] = '\0';
					nvrstrncpy(newVal,value,kMaxValueLengthInString);
					newVal[kMaxValueLengthInString-1] = '\0';

					std::map<std::string,int>::iterator it;

					if ((it = m_implementation->m_generatedName.find(name)) == m_implementation->m_generatedName.end())
					{
						m_implementation->m_generatedName[name] = 1;
						it = m_implementation->m_generatedName.find(name);
					}

					Add(it->first.c_str(),newVal,kMaxValueLengthInString);
					m_implementation->m_generatedVal.push_back(newVal);
				
				}
			}
		}
		else
		{
			if (m_verbose > 0)
				fprintf(stderr,"Ignoring line %s\n", buf);
		}	
	}
	
	return n;
}


//--------------------------------------------------------------------------------
// ReadHead inside start mark, stop mark
int	iRTVOptions::ReadHeader(FILE *fp, const char* startMark, const char* stopMark)
{
	NameValue::iterator p;
	char buf[128], name[128], value[128];
	int n;
	bool startFound = false;

	for (buf[0] = '\0', n = 0 ; fgets(buf,sizeof(buf), fp) && !ferror(fp);)
	{
		if (buf[0] == m_commentChar || strlen(buf) < 4) 
			continue; // ignore comments and empty lines
	
		DeSpaceDe(buf);

		if(startMark && !startFound)
		{
			startFound = (strcmp(buf, startMark) == 0);
		}

		if(stopMark && strcmp(buf, stopMark) == 0)
			break;

		if (Split(buf,name, value, sizeof name, m_seperatorChar))
		{
			if ((p = m_implementation->m_pair.find(name)) != m_implementation->m_pair.end() &&value[0] != '\0')
			{
				p->second.Convert(value);
				m_bits |= p->second.GetMask();
				n++;
			}
			else
			{
				if (m_verbose > 0)
					fprintf(stderr,"%s: %s = %s\n", value[0] ? "UnknownToken":"EmptyValue",name, value);
			}
		}
		else
		{
			if (m_verbose > 0)
				fprintf(stderr,"Ignoring line %s\n", buf);
		}	
	}

	return n;
}

//--------------------------------------------------------------------------------
int iRTVOptions::ParseLine(const char* iLine)
{
	char name[128], value[kMaxValueLengthInString];
	NameValue::iterator p;
	
	if (!iLine || !*iLine || strlen(iLine) < 4 || *iLine==m_commentChar)
		return 0;
	
	if (!Split(iLine,name, value, sizeof name, m_seperatorChar))
		return 0;

	if ((p = m_implementation->m_pair.find(name)) != m_implementation->m_pair.end())
	{
		p->second.Convert(value);
		m_bits |= p->second.GetMask();
		return p->second.GetMask() ? p->second.GetMask():1;
	}
	else
	{
		if (m_verbose > 0)
			fprintf(stderr,"Unknown token: %s = %s\n", name, value);
		return 0;
	}
}

//--------------------------------------------------------------------------------
char*	iRTVOptions::ConvertS(const char *iOption)
{
	static char sBuf[8][256];
	static TRAtomicVar<int> sSeq;
	int seq = ++sSeq;
	char  s[kMaxValueLengthInString], *sbuf = sBuf[seq&7];
	NameValue::iterator p;

	sbuf[0] = '\0';
	if ((p = m_implementation->m_pair.find(iOption)) != m_implementation->m_pair.end())
	{
		snprintf(sbuf, sizeof (sBuf[0]), "%s %c %s", p->first, m_seperatorChar, p->second.ConvertS(s));
	}

	sbuf[sizeof sBuf[0] - 1] = '\0';
	return sbuf;
}

//--------------------------------------------------------------------------------
inline NameValue::iterator operator + (NameValue::iterator p, int n)
{
	for  (int i = 0; i < n; i++)
		p++;
	return p;
}

//--------------------------------------------------------------------------------
char* iRTVOptions::ConvertS(int iOptions)
{
	NameValue::iterator p;

	if (iOptions >= GetCount() || iOptions < 0)
		return "";

	p = m_implementation->m_pair.begin();
	p = p + iOptions;

	return ConvertS((const char *)p->first);
}

//--------------------------------------------------------------------------------
void iRTVOptions::Clear(void)
{
	m_bits = 0;
	m_implementation->m_pair.clear();
}

//--------------------------------------------------------------------------------
int	iRTVOptions::Save(TRLogger* ipLogger, const char* prefix)
{
	NameValue::iterator p;
	char s[kMaxValueLengthInString];
	
	if (!prefix)
		prefix = "";
	
	for ( p = m_implementation->m_pair.begin(); p != m_implementation->m_pair.end(); p++)
	{
		ipLogger->LogMessage("%s%s %c %s\n", prefix, p->first, m_seperatorChar, p->second.ConvertS(s));
	}
	
#if 0
	// this is questionable - need to resolve
 	if (fp != stderr && fp != stdout)
 		fclose(fp);
#endif
	
	return 0;
}

//--------------------------------------------------------------------------------
int	iRTVOptions::Save(FILE* fp, const char* prefix)
{
	NameValue::iterator p;
	char s[kMaxValueLengthInString];
	
    if (!fp)
	{
		if (m_verbose)
			fprintf(stderr,"iRTVOptions::Save - invalid stream\n");
		return -1;
	}

	if (!prefix)
		prefix = "";
	
	for ( p = m_implementation->m_pair.begin(); p != m_implementation->m_pair.end(); p++)
	{
		fprintf(fp,"%s%s %c %s\n", prefix, p->first, m_seperatorChar, p->second.ConvertS(s));
	}
	
#if 0
	// this is questionable - need to resolve
 	if (fp != stderr && fp != stdout)
 		fclose(fp);
#endif
	
	return 0;
}

//--------------------------------------------------------------------------------
// save the option into a string form: name = value\nname=value\n
const char* iRTVOptions::BuildString(const char* iPrefix)
{
	char tmp[256],s[kMaxValueLengthInString];
	NameValue::iterator p;
	const char* prefix = iPrefix;
	string& out = m_implementation->m_theData;

	out = "";

	if (!prefix) prefix = "";
	
	for ( p = m_implementation->m_pair.begin(); p != m_implementation->m_pair.end(); p++)
	{
		snprintf(tmp,sizeof tmp,"%s%s %c %s\n", prefix, p->first, m_seperatorChar, p->second.ConvertS(s));
		tmp[sizeof tmp - 1] = '\0';
		out += tmp;
	}
	
	return out.c_str();
}

#include "AQCore/TRPlatform.h"
//--------------------------------------------------------------------------------
int	iRTVOptions::Save(const char *iFileName, const char* objName, int iLock)
{
	FILE *fp = 0;
	int status;
	iRTVSFileGuard openFiles;
	
	if (iFileName == 0 || strcmp(iFileName,"stderr") == 0)
	{
		iFileName = "";
		fp = stderr;
	}
	else if (strcmp(iFileName,"stdout") == 0)
	{
		fp = stdout;
	}

    if (TRPlatform::IsSystemFile(iFileName))
		return kSystemFile;

	if (iLock > 0)
	{
		for ( ; !fp && --iLock >=0; )
		{
			fp = _fsopen(iFileName, "w", _SH_DENYRD);
			if (!fp) Sleep(80);
		}
	}
	else if (!fp)
	{
		fp = fopen(iFileName, "w");
	}
	
    if (!fp)
	{
		if (m_verbose)
			fprintf(stderr,"Can't open %s for writing\n", iFileName);
		
		return kErrOpen;
	}

	openFiles.AddOpenFile(fp);
	
	if (objName && *objName)
		fprintf(fp,"\n < %s >\n", objName);
	
	status = Save(fp, objName);

	if (objName && *objName)
		fprintf(fp," </%s>\n", objName);

	return status;
}

//--------------------------------------------------------------------------------
// WriteHeader with start mark, stop mark
int	iRTVOptions::WriteHeader (FILE *fp, const char* startMark, const char* stopMark)
{
	NameValue::iterator p;
	char s[kMaxValueLengthInString];
	int wroteBytes = 0;

	if(startMark)
		wroteBytes += fprintf(fp,"%s\n", startMark);
	
	for ( p = m_implementation->m_pair.begin(); p != m_implementation->m_pair.end(); p++)
	{
		wroteBytes += fprintf(fp,"%s %c %s\n", p->first, m_seperatorChar, p->second.ConvertS(s));
	}
	
	if (stopMark)
		wroteBytes += fprintf(fp,"%s\n", stopMark);
	
	return wroteBytes;
}

//--------------------------------------------------------------------------------
#include <assert.h>
// support functions
void NVValue::Convert(const char *iValue)
{
	try
	{
		if (m_type == kFloat)
		{
			*(float*)m_value = float(atof(iValue));
		}
		else if (m_type == kDouble)
		{
			*(double*)m_value = atof(iValue);
		}
		else if (m_type == kShort)
		{
			*(short*)m_value = short(atoi(iValue));
		}
		else if (m_type == kInt)
		{
			*(int*) m_value = int(atoi(iValue));
		}
		else if (m_type == kLong)
		{
			*(long*) m_value =  atoi(iValue);
		}
		else if (m_type == kULong)
		{
			*(unsigned long*) m_value =  unsigned long(atoi(iValue));
		}
		else if (m_type == kUShort)
		{
			*(unsigned short*)m_value = unsigned short(atoi(iValue));
		}
		else if (m_type == kUInt)
		{
			*(unsigned int*) m_value = unsigned int(atoi(iValue));
		}
		else if (m_type == kChar)
		{
			*(char*) m_value = char(atoi(iValue));
		}
		else if (m_type == kUChar)
		{
			*(unsigned char*) m_value = unsigned char(atoi(iValue));
		}
		else if (m_type == kBool)
		{
			*(bool *)m_value = (iValue[0] == 'y' || iValue[0] == 'Y' ||
				iValue[0] == 't' || iValue[0] == 'T' ||
				atoi(iValue) > 0);
		}
		else if (m_type == kString || m_type == kUString)
		{
			if (m_length)
			{
				strncpy((char *)m_value, iValue, m_length);
			}
			else
			{
				strcpy((char *)m_value, iValue);
			}
		}
		else
		{
			assert(0);
			fprintf(stderr,"Convert: unknown type %d\n", m_type);
		}
	}
	catch (...)
	{
		assert(0);
	}
}

//--------------------------------------------------------------------------------
char*	NVValue::ConvertS(char* oS, int n)
{
	try
	{
		if (m_type == kFloat)
		{
			snprintf(oS,n,"%g" ,*(float*)m_value);
		}
		else if (m_type == kDouble)
		{
			snprintf(oS,n,"%g" ,*(double*)m_value);
		}
		else if (m_type == kShort)	
		{
			snprintf(oS,n,"%d" ,*(short*)m_value);
		}
		else if (m_type == kUShort)	
		{
			snprintf(oS,n,"%u" ,*(unsigned short*)m_value);
		}
		else if (m_type == kInt)
		{
			snprintf(oS,n,"%d" ,*(int*)m_value);
		}
		else if (m_type == kUInt)
		{
			snprintf(oS,n,"%u" ,*(unsigned int*)m_value);
		}
		else if (m_type == kLong)
		{
			snprintf(oS,n,"%d" ,*(long*)m_value);
		}
		else if (m_type == kULong)
		{
			snprintf(oS,n,"%u" ,*(unsigned long*)m_value);
		}
		else if (m_type == kChar)
		{
			snprintf(oS,n,"%d",*(char *)m_value);
		}
		else if (m_type == kUChar)
		{
			snprintf(oS,n,"%u",*(unsigned char *)m_value);
		}
		else if (m_type == kString || m_type == kUString)
		{
			if (n != 1)
			{
				snprintf(oS, n, "%s", (char *)m_value);
			}
			else
			{
				*oS = *(char *)m_value;
			}
		}
		else if (m_type == kBool)
		{
			snprintf(oS,n,"%d", *(bool *)m_value ? 1:0);
		}
		else
		{
			assert(0);
			fprintf(stderr,"Bad Type: %d\n", m_type);
			*oS = '\0';
		}
	}
	catch (...)
	{
		fprintf(stderr,"LoadOption::conversion threw exception \n");
	}

	if (n > 1)
		oS[n - 1] = '\0';
	return oS;
}
