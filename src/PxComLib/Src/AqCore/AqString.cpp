/***********************************************************************
 * AqString.cpp
 *---------------------------------------------------------------------
 **************	This document contains information that	**************
 **************	   is proprietary to TeraRecon, Inc.	**************
 **************		     All rights reserved.			**************
 **************			Copyright TeraRecon, Inc.		**************
 **************				 2005-2006					**************
 *
 *	PURPOSE:
 *		string class
 *  
 *
 *	AUTHOR:  Gang Li, June. 2006.
 * 
 */

#include "AqString.h"
#include "AqCore.h"
#include <memory.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define _WIN32_DCOM
#include "Windows.h"



static int gEmptyStringData[] = { 0, 0, 0, 0 }; // two bytes terminator data for both asii and unicode

AqStringData* AqString::c_pEmptyData = (AqStringData*)&gEmptyStringData;
const char*  AqString::c_pEmptyBuffer = (const char*)(((char*)&gEmptyStringData) + sizeof(AqStringData));


//-----------------------------------------------------------------------------

bool  AqString::IsStringGood(const char* iStr, int iLength)
{
	if(iStr == 0 || ::IsBadStringPtrA(iStr, iLength))
	{
		GetAqLogger()->LogMessage("Error: encounter bad string pointer\n");
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------

bool AqString::Allocate(int iLen)
{
	assert(iLen >= 0);
	assert(iLen <= 2147483647 - 1);

	Release();

	if (iLen == 0)
		return true;

	int bSize = iLen+1;
	AqStringData* pData = 0;
	
	try {pData = (AqStringData*)new char[sizeof(AqStringData) + bSize * sizeof(char)];} catch(...) {}

	if(pData == 0)
	{
		GetAqLogger()->LogMessage("Error: (AqString::Allocate) failed\n");
		return false;
	}

	pData->m_bufferSize = bSize;
	m_pBuffer = pData->GetBuffer();
	m_pBuffer[iLen] = '\0';

	return true;
}


//-----------------------------------------------------------------------------

void AqString::Release()
{
	AqStringData* pData = GetDataInfo();
	if (pData != c_pEmptyData)
	{
		delete[] (char*)pData;
		Init();
	}
}

	
//-----------------------------------------------------------------------------

void * AqString::CopyBin(const void* iData, unsigned int iLen)
{
	if(!iData || iData == m_pBuffer)
		return m_pBuffer; 

	AqString tstr;
	if(InBuffer(iData))
		Swap(tstr);

	if(!Allocate(iLen))
		return 0;

	memcpy(m_pBuffer, iData, iLen);

	return m_pBuffer; 
}

const AqString& AqString::operator = (const AqString& iStr)
{
	// TO DO, when have reference count, do ref increase
	Copy(iStr);
	return *this;
}

//-----------------------------------------------------------------------------

char * AqString::Copy(const char* iStr)
{
	if(!IsStringGood(iStr) || iStr == m_pBuffer)
		return m_pBuffer;

	int nBytesSource = strlen(iStr);
	if(nBytesSource == 0)
		return m_pBuffer;

	// save string in case it is inside this string
	AqString tstr;
	Swap(tstr);
	
	if(!Allocate(nBytesSource))
		return 0;

	memcpy(m_pBuffer, iStr, nBytesSource);
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

char * AqString::Copy(char iChar)
{
	if(!Allocate(1))
		return 0;

	m_pBuffer[0] = iChar;
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

bool AqString::Concat(const char* iData1, int iSize1, const char* iData2, int iSize2)
{
	AqString str;
	
	if(!str.Allocate(iSize1+iSize2))
		return false;

	memcpy(str.m_pBuffer, iData1, iSize1);
	memcpy(str.m_pBuffer+iSize1, iData2, iSize2);

	this->Swap(str);
	return true;
}


//-----------------------------------------------------------------------------

char AqString::operator [](int iIndex) const
{
	assert(iIndex >= 0);
	assert(iIndex < GetBufferSize());
	return m_pBuffer[iIndex];

}

//-----------------------------------------------------------------------------

void AqString::SetAt(char iChar, int iIndex)
{
	assert(iIndex >= 0);
	assert(iIndex < GetBufferSize());
	m_pBuffer[iIndex] = iChar;
}


//-----------------------------------------------------------------------------

int AqString::Format(const char* iFormatStr, ...)
{
	assert(AqString::IsStringGood(iFormatStr));

	va_list argList;
	va_start(argList, iFormatStr);
	int rcode = VFormat(iFormatStr, argList);
	va_end(argList);
	return rcode;
}

//-----------------------------------------------------------------------------

int AqString::VFormat(const char* iFormatStr, va_list iArgList)
{
	int count;
	int bufSize;

	int slen=0, nArg=0;
	const char* p = iFormatStr;
	while(*p != 0)
	{
		if(*p == '%')
			nArg++;
		slen++;
		p++;
	}
	bufSize = slen + nArg*64 + 128; // estmated buffer size


	// save this string, in case it is used in formating
	AqString tstr;
	Swap(tstr);
	
	while(true)
	{
		if(!Allocate(bufSize))
			break;

		va_list myArgList = iArgList;
		count = _vsnprintf(m_pBuffer, bufSize, iFormatStr, myArgList);
		if(count >= 0)
		{
			AqString copystr;
			Swap(copystr);

			if(!Allocate(count))
				break;
			memcpy(m_pBuffer, copystr.m_pBuffer, count);
			return count;
		}
		bufSize = bufSize * 5;

	}

	return 0;
}


//-----------------------------------------------------------------------------

bool AqString::Swap(AqString& iStr)
{
	char *pSaved = m_pBuffer;

	m_pBuffer = iStr.m_pBuffer;
	iStr.m_pBuffer = pSaved;

	return true;
}


//-----------------------------------------------------------------------------

char * AqString::BinToHex(const void *iData, int iDataSize)
{

	AqString  buf2;

	if(iData == 0 || iDataSize == 0)
	{
		iData = m_pBuffer;
		iDataSize = GetBufferSize()-1;
		if(!iData)
			return 0;

	}

	if( InBuffer(iData) )
	{
		if(!Swap(buf2))
			return 0;
	}
	
	int hexSize = iDataSize*2+2;

	Allocate(hexSize);

	return AqBinToHex(iData, iDataSize, m_pBuffer);
}


//-----------------------------------------------------------------------------

void * AqString::HexToBin(const char* iString)
{
	if(iString == 0)
		iString = m_pBuffer;

	if( InBuffer(iString) )
	{

		if(iString != m_pBuffer || m_pBuffer == c_pEmptyBuffer)
		{
			GetAqLogger()->LogMessage("Error: (AqString::HexToBin) not start at head\n");
			return 0; // not start with head, can not do anything in place
		}

	}
	else
	{
		Allocate((strlen(iString)/2-1));
	}


	if(GetDataInfo()->m_bufferSize < 3)
	{
		return 0;
	}

	return AqHexToBin(iString, m_pBuffer) ;
}


//-----------------------------------------------------------------------------

char* AqString::ToMultiByte (const wchar_t* iuString, unsigned int iCodePage) 
{
	AqString tmpBuf;

	if(!AqUString::IsStringGood(iuString))
		return m_pBuffer;
	
	if(InBuffer(iuString))
	{
		if(!Swap(tmpBuf))
			return m_pBuffer;
	}

	if(iCodePage == 0)
		iCodePage = CP_ACP; //CP_ACP is 0

	// Converts a string into a Unicode wide-character string
	// Adds zero-termination
	int nBytesSource = wcslen(iuString);
	// Query the number of WChars required to store the destination string
	int nCharNeeded = WideCharToMultiByte (iCodePage, WC_COMPOSITECHECK, iuString, -1, 0, 0, 0, 0);
	
	// Allocate the required amount of space plus 2 more bytes for '\0'
	Allocate(nCharNeeded);

	// Do the conversion
	nCharNeeded = WideCharToMultiByte(iCodePage, WC_COMPOSITECHECK, iuString,
		nBytesSource, m_pBuffer, nCharNeeded, 0, 0);
	
	*(m_pBuffer + nCharNeeded) = '\0';

	return m_pBuffer;
}




//-----------------------------------------------------------------------------
char* AqString::TruncString (unsigned int iSize)
{
	if (strlen(m_pBuffer) > iSize)
		m_pBuffer[iSize] = L'\0';
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

char* AqString::Resize (unsigned int iSize)
{
	if(iSize < 0)
		return m_pBuffer;

	unsigned int oBufSize = GetBufferSize();

	AqString orgStr;
	Swap(orgStr); // save string for copy back
	if(!Allocate(iSize))
	{
		Swap(orgStr);
		return m_pBuffer;
	}

	// save back
	if(oBufSize < iSize)
	{
		memcpy(m_pBuffer, orgStr.m_pBuffer, oBufSize*sizeof(char));
	}
	else
	{
		memcpy(m_pBuffer, orgStr.m_pBuffer, iSize*sizeof(char));
		m_pBuffer[iSize] = '\0';
	}

	return m_pBuffer;
	
}


//-----------------------------------------------------------------------------

AqString AqString::Mid( int iFirst, int iCount  ) const
{
	AqString str;
	int slen = GetLength();
	if(iCount < 1 || iFirst+iCount > slen )
		return str;
	
	str.Allocate(iCount);
	memcpy(str.m_pBuffer, m_pBuffer+iFirst, iCount*sizeof(char));
	return str;
}


//-----------------------------------------------------------------------------

AqString AqString::Left(int iCount  ) const
{
	AqString str;
	int slen = GetLength();
	if(iCount < 1)
		return str;

	if( iCount >= slen)
		return *this;
	
	str.Allocate(iCount);
	memcpy(str.m_pBuffer, m_pBuffer, iCount*sizeof(char));
	return str;
}


//-----------------------------------------------------------------------------

AqString AqString::Right(int iCount  ) const
{
	AqString str;
	int slen = GetLength();
	if(iCount < 1)
		return str;

	if( iCount >= slen)
		return *this;
	
	str.Allocate(iCount);
	
	memcpy(str.m_pBuffer, m_pBuffer + slen-iCount, iCount*sizeof(char));
	return str;
}


//-----------------------------------------------------------------------------

char* AqString::TrimRight()
{
	//CopyOnWrite();

	char* p = m_pBuffer;
	char* pEnd = 0;

	// scan from head for mult-byte safe
	while (*p != '\0')
	{
		if (isspace(*p))
		{
			if (pEnd == 0)
				pEnd = p;
		}
		else
		{
			pEnd = 0;
		}
		p = ::CharNextA(p);
	}

	if (pEnd)
		*pEnd ='\0';

	return m_pBuffer;

}

//-----------------------------------------------------------------------------

char* AqString::TrimLeft()
{
	//CopyOnWrite();

	char* p = m_pBuffer;
	if(!isspace(*p))
		return m_pBuffer;
	else
		p = ::CharNextA(p);

	while (*p != '\0' && isspace(*p))
	{
		p = ::CharNextA(p);
	}

	Copy(p);
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

int AqString::Find(const char * iSubStr, int iStart) const
{
	assert(IsStringGood(iSubStr));

	if (iStart > GetLength())
		return -1;

	char * p = strstr(m_pBuffer + iStart, iSubStr);
	return (p == 0) ? -1 : (int)(p - m_pBuffer);
}


//-----------------------------------------------------------------------------

int AqString::Replace(char iOldChar, char iNewChar)
{
	int count = 0;

	if (iOldChar == iNewChar)
		return count;

	//CopyOnWrite();
	char * p = m_pBuffer;
	char * pEnd = p + GetLength();
	
	//TO DO, cache string length

	while (p < pEnd)
	{
		// replace instances of the specified character only
		if (*p == iOldChar)
		{
			*p = iNewChar;
			count++;
		}
		p = ::CharNextA(p);
	}

	return count;
}


int AqString::Replace(const char* iOldStr, const char* iNewStr)
{
	if(!IsStringGood(iOldStr) || !IsStringGood(iNewStr))
		return 0;

	int sLenOld = strlen(iOldStr);
	if (sLenOld == 0)
		return 0;

	

	// find new string length
	int count = 0;
	char* pStart = m_pBuffer;
	char * pTarget;
	while ((pTarget = strstr(pStart, iOldStr)) != 0)
	{
		count++;
		pStart = pTarget + sLenOld;
	}

	if (count == 0)
		return 0;

	int sLenNew = strlen(iNewStr);
	int strLength = GetLength();
	int newLength =  strLength + (sLenNew - sLenOld) * count;

	if(sLenNew > sLenOld)
		Resize(newLength);
	
	// do replace
	int balance;
	pStart = m_pBuffer;

	while ( (pTarget = strstr(pStart, iOldStr)) != 0)
	{
		balance = strLength - ((int)(pTarget - m_pBuffer) + sLenOld);
		memmove(pTarget + sLenNew, pTarget + sLenOld, balance * sizeof(char));
		memcpy(pTarget, iNewStr, sLenNew * sizeof(char));
		pStart = pTarget + sLenNew;
		pStart[balance] = '\0';
		strLength += (sLenNew - sLenOld);
	}


	return count;
}


/////// start Unicode AqUString

AqUStringData* AqUString::c_pEmptyData = (AqUStringData*)&gEmptyStringData;
const wchar_t*  AqUString::c_pEmptyBuffer = (const wchar_t*)(((char*)&gEmptyStringData) + sizeof(AqUStringData));

//-----------------------------------------------------------------------------

bool  AqUString::IsStringGood(const wchar_t* iStr, int iLength)
{
	if(iStr == 0 || ::IsBadStringPtrW(iStr, iLength))
	{
		GetAqLogger()->LogMessage("Error: encounter bad unicode string pointer\n");
		return false;
	}

	return true;
}



//-----------------------------------------------------------------------------

bool AqUString::Allocate(int iLen)
{
	assert(iLen >= 0);
	assert(iLen <= (2147483647 - 1)/2);

	Release();

	if (iLen == 0)
		return true;

	int bSize  = iLen*2+2;
	
	AqUStringData* pData = 0;
	
	try {pData = (AqUStringData*)new char[sizeof(AqUStringData) + bSize * sizeof(char)];} catch(...) {}

	if(pData == 0)
	{
		GetAqLogger()->LogMessage("Error: (AqUString::Allocate) failed\n");
		return false;
	}

	pData->m_bufferSize = bSize;
	m_pBuffer = pData->GetBuffer();
	
	m_pBuffer[iLen] = L'\0';

	return true;
}


//-----------------------------------------------------------------------------

void AqUString::Release()
{
	AqUStringData* pData = GetDataInfo();
	if (pData != c_pEmptyData)
	{
		delete[] (char*)pData;
		Init();
	}
}

	

//-----------------------------------------------------------------------------

const AqUString& AqUString::operator = (const AqUString& iStr)
{
	// TO DO, when have reference count, do ref increase
	Copy(iStr.m_pBuffer);
	return *this;
}

//-----------------------------------------------------------------------------

wchar_t* AqUString::Copy(const wchar_t* iuString)
{
	if(!IsStringGood(iuString) || iuString == m_pBuffer)
		return m_pBuffer;

	int nBytesSource = wcslen(iuString);
	if(nBytesSource == 0)
		return m_pBuffer;


	// save string in case it is inside this string
	AqUString tstr; 
	Swap(tstr);
	
	if(!Allocate(nBytesSource))
		return 0;

	memcpy((char*)m_pBuffer, (char*)iuString, nBytesSource*2);
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

wchar_t * AqUString::Copy(const char* iStr)
{
	ToUnicode(iStr);
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

wchar_t* AqUString::Copy(wchar_t iUChar)
{
	if(!Allocate(1))
		return 0;

	m_pBuffer[0] = iUChar;
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

wchar_t * AqUString::Copy(char iChar)
{
	char buf[2]; buf[0] = iChar; buf[1] = 0;

	ToUnicode(buf);
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

bool AqUString::Concat(const wchar_t* iData1, int iSize1, const wchar_t* iData2, int iSize2)
{
	
	AqUString str;
	
	if(!str.Allocate(iSize1+iSize2))
		return false;

	char* p = (char*)str.m_pBuffer;

	iSize1 = iSize1*2;
	iSize2 = iSize2*2;

	memcpy(p, (const void*)iData1, iSize1);
	memcpy(p+iSize1, (const void*)iData2, iSize2);

	this->Swap(str);
	return true;
}


//-----------------------------------------------------------------------------

AqUString operator +(const AqUString& iStr1, const AqUString& iStr2)
{
	AqUString str;
	str.Concat(iStr1, iStr1.GetLength(), iStr2, iStr2.GetLength());
	return str;

}

//-----------------------------------------------------------------------------

AqUString operator +(const AqUString& iStr, wchar_t iUChar)
{
	AqUString str;
	
	str.Concat(iStr, iStr.GetLength(), &iUChar, 1);
	return str;
}


//-----------------------------------------------------------------------------

AqUString operator +(wchar_t iUChar, const AqUString& iStr)
{
	AqUString str;
	str.Concat(&iUChar, 1, iStr, iStr.GetLength());
	return str;
}


//-----------------------------------------------------------------------------

AqUString operator +(const AqUString& iStr, char iChar)
{
	AqUString str;

	char buf[2]; buf[0] = iChar; buf[1] = 0;
	str.ToUnicode(buf);
	str.Concat(iStr, iStr.GetLength(), str, str.GetLength());

	return str;
}


//-----------------------------------------------------------------------------

AqUString operator +(char iChar, const AqUString& iStr)
{
	AqUString str;

	char buf[2]; buf[0] = iChar; buf[1] = 0;
	str.ToUnicode(buf);

	str.Concat(str, str.GetLength(), iStr, iStr.GetLength());
	return str;
}


//-----------------------------------------------------------------------------

AqUString operator +(const AqUString& iStr1, const wchar_t* iStr2)
{	
	assert(AqUString::IsStringGood(iStr2));

	AqUString str;
	str.Concat(iStr1, iStr1.GetLength(), iStr2, wcslen(iStr2));
	return str;
}


//-----------------------------------------------------------------------------

AqUString operator +(const wchar_t* iUStr, const AqUString& iStr)
{	
	AqUString str;

	assert(AqUString::IsStringGood(iUStr));
	str.Concat(iUStr, wcslen(iUStr), iStr, iStr.GetLength());
	return str;
}



//-----------------------------------------------------------------------------

AqUString operator +(const AqUString& iStr1, const char* iStr2)
{
	assert(AqString::IsStringGood(iStr2));

	AqUString str;

	str.ToUnicode(iStr2);
	str.Concat(iStr1, iStr1.GetLength(), str, str.GetLength());
	return str;
}

//-----------------------------------------------------------------------------

AqUString operator +(const char* iStr1, const AqUString& iStr2)
{
	assert(AqString::IsStringGood(iStr1));

	AqUString str;

	str.ToUnicode(iStr1);
	str.Concat(str, str.GetLength(), iStr2, iStr2.GetLength());
	return str;
}


//-----------------------------------------------------------------------------

AqUString& AqUString::operator +=(const AqUString& iStr)
{
	Concat(GetString(), GetLength(), iStr, iStr.GetLength());
	return *this;
}
//-----------------------------------------------------------------------------

AqUString& AqUString::operator +=(const AqString& iStr)
{
	AqUString str;
	str.ToUnicode(iStr);
	
	Concat(GetString(), GetLength(), str, str.GetLength());
	return *this;
}

//-----------------------------------------------------------------------------

AqUString& AqUString::operator +=(wchar_t iUChar)
{
	Concat(GetString(), GetLength(), &iUChar, 1);
	return *this;

}

//-----------------------------------------------------------------------------

AqUString& AqUString::operator +=(char iChar)
{
	AqUString str;

	char buf[2]; buf[0] = iChar; buf[1] = 0;
	str.ToUnicode(buf);
	Concat(GetString(), GetLength(), str, str.GetLength());
	return *this;
}


//-----------------------------------------------------------------------------

AqUString& AqUString::operator +=(const wchar_t* iStr)
{
	assert(IsStringGood(iStr));

	Concat(GetString(), GetLength(), iStr, wcslen(iStr));
	return *this;
}



//-----------------------------------------------------------------------------

AqUString& AqUString::operator +=(const char* iStr)
{
	assert(AqString::IsStringGood(iStr));

	AqUString str;
	str.ToUnicode(iStr);
	
	Concat(GetString(), GetLength(), str, str.GetLength());
	return *this;

}


//-----------------------------------------------------------------------------

wchar_t AqUString::operator [](int iIndex) const
{
	assert(iIndex >= 0);
	assert(iIndex < GetBufferSize());
	return m_pBuffer[iIndex];

}

//-----------------------------------------------------------------------------

void AqUString::SetAt(wchar_t iChar, int iIndex)
{
	assert(iIndex >= 0);
	assert(iIndex*2 < GetBufferSize());
	m_pBuffer[iIndex] = iChar;
}


//-----------------------------------------------------------------------------

int AqUString::Format(const wchar_t* iFormatStr, ...)
{
	assert(AqUString::IsStringGood(iFormatStr));

	va_list argList;
	va_start(argList, iFormatStr);
	int rcode = VFormat(iFormatStr, argList);
	va_end(argList);
	return rcode;
}

//-----------------------------------------------------------------------------

int AqUString::VFormat(const wchar_t* iFormatStr, va_list iArgList)
{
	int count;
	int bufSize;

	int slen=0, nArg=0;
	const wchar_t* p = iFormatStr;
	while(*p != L'\0')
	{
		if(*p == L'%')
			nArg++;
		slen++;
		p++;
	}
	bufSize = slen + nArg*64 + 128; // estmated buffer size

	
	// save this string, in case it is used in formating
	AqUString tstr;
	Swap(tstr);
	
	while(true)
	{
		if(!Allocate(bufSize))
			break;

		va_list myArgList = iArgList;
		count = _vsnwprintf(m_pBuffer, bufSize, iFormatStr, myArgList);
		if(count >= 0)
		{
			AqUString copystr;
			Swap(copystr);

			if(!Allocate(count))
				break;

			memcpy(m_pBuffer, copystr.m_pBuffer, count*2);
			return count;

		}
		bufSize = bufSize * 5;

	}

	return 0;
}


//-----------------------------------------------------------------------------

bool AqUString::Swap(AqUString& iStr)
{
	wchar_t *pSaved = m_pBuffer;

	m_pBuffer = iStr.m_pBuffer;
	iStr.m_pBuffer = pSaved;

	return true;
}


//-----------------------------------------------------------------------------

wchar_t* AqUString::ToUnicode (const char* iString, unsigned int iCodePage) 
{
	AqUString tmpBuf;

	if(!AqString::IsStringGood(iString))
		return m_pBuffer;
	

	if(InBuffer(iString))
	{
		if(!Swap(tmpBuf))
			return m_pBuffer;
	}

	if(iCodePage == 0)
		iCodePage = CP_ACP; //CP_ACP is 0
	// Converts a string into a Unicode wide-character string
	// Adds zero-termination
	
	int nBytesSource = strlen(iString) * 2;
	// Query the number of WChars required to store the destination string
	int nWCharNeeded = MultiByteToWideChar (iCodePage, MB_PRECOMPOSED, iString, nBytesSource, NULL, 0);
	
	// Allocate the required amount of space plus 2 more bytes for '\0'
	if(!Allocate(nWCharNeeded) )
		return  0;

	// Do the conversion
	nWCharNeeded = MultiByteToWideChar(iCodePage, MB_PRECOMPOSED, iString,
		nBytesSource, m_pBuffer, nWCharNeeded);
	
	*(m_pBuffer + nWCharNeeded) = L'\0';	
	return m_pBuffer;
}



//-----------------------------------------------------------------------------

wchar_t* AqUString::TruncString (unsigned int iSize)
{
	if (wcslen(m_pBuffer) > iSize)
		m_pBuffer[iSize] = L'\0';
	return m_pBuffer;
}


//-----------------------------------------------------------------------------

wchar_t* AqUString::Resize (unsigned int iSize)
{

	if(iSize < 0)
		return m_pBuffer;

	unsigned int oBufSize = GetBufferSize();

	AqUString orgStr;
	Swap(orgStr); // save string for copy back
	if(!Allocate(iSize))
	{
		Swap(orgStr);
		return m_pBuffer;
	}

	// save back
	if(oBufSize < iSize)
	{
		memcpy(m_pBuffer, orgStr.m_pBuffer, oBufSize*sizeof(wchar_t));
	}
	else
	{
		memcpy(m_pBuffer, orgStr.m_pBuffer, iSize*sizeof(wchar_t));
		m_pBuffer[iSize] = L'\0';
	}
	
	return m_pBuffer;
}



//-----------------------------------------------------------------------------

AqUString AqUString::Mid( int iFirst, int iCount  ) const
{
	AqUString str;
	int slen = GetLength();
	if(iCount < 1 || iFirst+iCount > slen )
		return str;
	
	str.Allocate(iCount);
	memcpy(str.m_pBuffer, m_pBuffer+iFirst, iCount*sizeof(wchar_t));
	return str;
}


//-----------------------------------------------------------------------------

AqUString AqUString::Left(int iCount  ) const
{
	AqUString str;
	int slen = GetLength();
	if(iCount < 1)
		return str;

	if( iCount >= slen)
		return *this;
	
	str.Allocate(iCount);
	memcpy(str.m_pBuffer, m_pBuffer, iCount*sizeof(wchar_t));
	return str;
}


//-----------------------------------------------------------------------------

AqUString AqUString::Right(int iCount  ) const
{
	AqUString str;
	int slen = GetLength();
	if(iCount < 1)
		return str;

	if( iCount >= slen)
		return *this;
	
	str.Allocate(iCount);
	
	memcpy(str.m_pBuffer, m_pBuffer + slen-iCount, iCount*sizeof(wchar_t));
	return str;
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

wchar_t* AqUString::TrimRight()
{
	//CopyOnWrite();

	wchar_t* p = m_pBuffer;
	wchar_t* pEnd = 0;

	// scan from head for mult-byte safe
	while (*p != L'\0')
	{
		if (iswspace(*p))
		{
			if (pEnd == 0)
				pEnd = p;
		}
		else
		{
			pEnd = 0;
		}
		p = ::CharNextW(p);
	}

	if (pEnd)
		*pEnd = L'\0';

	return m_pBuffer;

}


//-----------------------------------------------------------------------------

wchar_t * AqUString::TrimLeft()
{
	//CopyOnWrite();

	wchar_t* p = m_pBuffer;
	if(!iswspace(*p))
		return m_pBuffer;
	else
		p = ::CharNextW(p);

	while (*p != L'\0' && iswspace(*p))
	{
		p = ::CharNextW(p);
	}

	Copy(p);
	return m_pBuffer;

}


//-----------------------------------------------------------------------------

int AqUString::Find(const wchar_t * iSubStr, int iStart) const
{
	assert(IsStringGood(iSubStr));

	if (iStart > GetLength())
		return -1;

	wchar_t * p = wcsstr(m_pBuffer + iStart, iSubStr);
	return (p == 0) ? -1 : (int)(p - m_pBuffer);
}


//-----------------------------------------------------------------------------

int AqUString::Replace(wchar_t iOldChar, wchar_t iNewChar)
{
	int count = 0;

	if (iOldChar == iNewChar)
		return count;

	//CopyOnWrite();
	wchar_t * p = m_pBuffer;
	wchar_t * pEnd = p + GetLength();
	
	//TO DO, cache string length

	while (p < pEnd)
	{
		// replace instances of the specified character only
		if (*p == iOldChar)
		{
			*p = iNewChar;
			count++;
		}
		p = ::CharNextW(p);
	}

	return count;
}


int AqUString::Replace(const wchar_t * iOldStr, const wchar_t * iNewStr)
{

	if(!IsStringGood(iOldStr) || !IsStringGood(iNewStr))
		return 0;

	int sLenOld = wcslen(iOldStr);
	if (sLenOld == 0)
		return 0;

	// find new string length
	int count = 0;
	wchar_t* pStart = m_pBuffer;
	wchar_t* pTarget;
	while ((pTarget = wcsstr(pStart, iOldStr)) != 0)
	{
		count++;
		pStart = pTarget + sLenOld;
	}

	if (count == 0)
		return 0;

	int sLenNew = wcslen(iNewStr);
	int strLength = GetLength();
	int newLength =  strLength + (sLenNew - sLenOld) * count;

	if(sLenNew > sLenOld)
		Resize(newLength);
	
	// do replace
	int balance;
	pStart = m_pBuffer;

	while ( (pTarget = wcsstr(pStart, iOldStr)) != 0)
	{
		balance = strLength - ((int)(pTarget - m_pBuffer) + sLenOld);
		memmove(pTarget + sLenNew, pTarget + sLenOld, balance * sizeof(wchar_t));
		memcpy(pTarget, iNewStr, sLenNew * sizeof(wchar_t));
		pStart = pTarget + sLenNew;
		pStart[balance] = L'\0';
		strLength += (sLenNew - sLenOld);
	}


	return count;
}


//--------------------------------------------------------------------------------------------
// Murali Ayyapillai 2006.07.06
// Defines and Constants Required for conversion of Unicode To UTF-8 and vice versa.
// Reference :	// http://en.wikipedia.org/wiki/Utf-8

const int ASCII				= 0x007f;
const int UTF8_2_MAX		= 0x07ff;  
const int UTF8_3_MAX		= 0xffff;
const int UTF8_1ST_OF_2		= 0xc0;    // 110x xxxx
const int UTF8_1ST_OF_3		= 0xe0;    // 1110 xxxx
const int UTF8_TRAIL		= 0x80;    // 10xx xxxx

#define HIGHER_6_BIT(u)   ((u) >> 12)
#define MIDDLE_6_BIT(u)   (((u) & 0x0fc0) >> 6)
#define LOWER_6_BIT(u)    ((u) & 0x003f)
#define BIT7(a)           ((a) & 0x80)
#define BIT6(a)           ((a) & 0x40)

//-------------------------------------------------------------------------------------------
/**
 * Converts the passed unicode string into UTF-8 and returns the buffer in the outStr object.
 */ 
int UnicodeToUTF8(const wchar_t* iStr, int srclen, AqString& outStr)
{	
	if(iStr == 0 || srclen <= 0)
	{
		GetAqLogger()->LogMessage("Error: (UnicodeToUTF8) source string null \n");
		return 0;
	}

	LPCWSTR lpWC  = iStr;
	int cchU8	  = 0;			// # of UTF8 chars generated

	int  dstlen   = 1024;		// allocate a tmp buffer to hold the translated elements
	char lpDestStr[1024];		 

	while ( (srclen--) && (cchU8 < dstlen) )	// both src and dst ptrs must be valid.
	{

		 if (*lpWC <= ASCII)
		 {             
			 lpDestStr[cchU8++] = (char)*lpWC;	//  Found ASCII.
		 }
		 else if (*lpWC <= UTF8_2_MAX)
		 {             
			 if ((cchU8 + 1) < dstlen) //		Found 2 byte sequence if < 0x07ff (11 bits).
			 {
				 //  Use upper 5 bits in first byte.
				 //  Use lower 6 bits in second byte.

				 lpDestStr[cchU8++] = UTF8_1ST_OF_2 | (*lpWC >> 6);
				 lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(*lpWC);
			 }
			 else
			 {
				 //  Error - buffer too small.
				 srclen++;
				 GetAqLogger()->LogMessage("Error: (UnicodeToUTF8) Insufficent buffer length \n");
				 break;
			 }
		 }
		 else //if (*lpWC <= UTF8_3_MAX)
			  // This check is not required as the current UTF-8 standard allows a maximum of 3 bytes only.
		 {             
			 if ((cchU8 + 2) < dstlen)	//  Found 3 byte sequence.
			 {
				 //  Use upper  4 bits in first byte.
				 //  Use middle 6 bits in second byte.
				 //  Use lower  6 bits in third byte.
				 lpDestStr[cchU8++] = UTF8_1ST_OF_3 | HIGHER_6_BIT(*lpWC); 
				 lpDestStr[cchU8++] = UTF8_TRAIL    | MIDDLE_6_BIT(*lpWC);
				 lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(*lpWC);
			 }
			 else
			 {
				 //  Error - buffer too small.
				 srclen++;
				 break;
			 }
		 }

		lpWC++;
	}

	lpDestStr[cchU8] = '\0';

	//copy the translated buffer into string object.
//	outStr.Release(); 
	//outStr.Allocate(cchU8);
	//outStr.Copy(lpDestStr);	
	outStr = lpDestStr;	

	 //  Return the number of UTF-8 characters written.
	 return (cchU8); 
}

/**
 * Converts the passed UTF-8 string into unicode and returns the unicode buffer in the outStr object.
 */ 
int UTF8ToUnicode(const char* iStr, int srclen, AqUString& outStr)
{
	if(iStr == 0 || srclen <= 0)
	{
		GetAqLogger()->LogMessage("Error: (UTF8ToUnicode) source string null \n");
		return 0;
	}

     int nTB = 0;                   // # trail bytes to follow
     int cchWC = 0;                 // # of Unicode code points generated
     LPCSTR pUTF8 = iStr;
     char UTF8;
 
	 int dstlen		 = 1024;
	 wchar_t lpDestStr[1024];

     while ((srclen--) && (cchWC < dstlen) )
     {
         //  See if there are any trail bytes.
         if (BIT7(*pUTF8) == 0)
         {
             //  Found ASCII.
             lpDestStr[cchWC++] = (wchar_t)*pUTF8;
         }
         else if (BIT6(*pUTF8) == 0)
         {
             //  Found a trail byte.
             //  Note : Ignore the trail byte if there was no lead byte.
             if (nTB != 0)
             {
                 //  Decrement the trail byte counter.
                 nTB--;
 
                 //  Make room for the trail byte and add the trail byte
                 //  value.
                 lpDestStr[cchWC] <<= 6;	// shift the 5 bits
                 lpDestStr[cchWC] |= LOWER_6_BIT(*pUTF8);
 
                 if (nTB == 0)
                 {
                     //  End of sequence.  Advance the output counter.
                     cchWC++;
                 }
             }
         }
         else
         {
             //  Found a lead byte.
             if (nTB > 0)
             {
                 //  Error - previous sequence not finished.
                 nTB = 0;
                 cchWC++;
             }
             else
             {
                 //  Calculate the number of bytes to follow.
                 //  Look for the first 0 from left to right.
                 UTF8 = *pUTF8;
                 while (BIT7(UTF8) != 0)
                 {
                     UTF8 <<= 1;
                     nTB++;
                 }
 
                 //  Store the value from the first byte and decrement
                 //  the number of bytes to follow.
                 lpDestStr[cchWC] = UTF8 >> nTB;
                 nTB--;
             }
         }
 
         pUTF8++;
     }
 
	lpDestStr[cchWC] = L'\0';

	//copy the translated buffer into string object.
	//outStr.Reset(); 
	//outStr.MarkUnicodeData();
	//outStr.Allocate(cchWC*sizeof(wchar_t)); 	
	//outStr.CopyStr(lpDestStr);	
	outStr = lpDestStr;	
	 
	//  Return the number of Unicode characters written.
	return (cchWC);
}

//----------------------------------------------------------------------------------------

/**
 * Converts the passed Ascii string into unicode and returns the unicode buffer in the outStr object.
 */ 
int MBCSToUnicode(const char* iStr, int srclen, const char* scs, AqUString& outStr)
{

	return 0;
}

int UnicodeToMBCS(const wchar_t* iStr, int srclen, const char* scs, AqString& outStr)
{

	return 0;
}


//************************ unit test *****************************
bool  AqString::UnitTest()
{
	const char *pTest = "test AqString";
	const char *pTest2 = "test AqString 2";

	{
		AqString str;
		
		if (str.m_pBuffer[0] != 0 || !str.IsEmpty())
		{
			printf("fail on AqString str\n");
			return false;
		}
	}

	{
		AqString str(pTest);
		
		
		if (strcmp(str.m_pBuffer, pTest) != 0)
		{
			printf("fail on AqString str(p)\n");
			return false;
		}

	}
	
	{
		char c = 'T';
		AqString str(c);
		
		
		if (str.m_pBuffer[0] != c || str.m_pBuffer[1] != 0)
		{
			printf("fail on AqString str(c)\n");
			return false;
		}


	}

	{
		const char *p0, *p;
		AqString str =  pTest;
		
		p = pTest+4;
				
		if (str.InBuffer(pTest) || str.InBuffer(p))
		{
			printf("fail on str.InBuffer against outside pointer\n");
			return false;
		}
		
		p0 = str;
		p = p0+4;

		if (!str.InBuffer(p0) || !str.InBuffer(p))
		{
			printf("fail on str.InBuffer against inside pointer\n");
			return false;
		}

		if(str.GetBufferSize() != int(strlen(pTest)+1))
		{
			printf("fail on str.GetBufferSize\n");
			return false;
		}

		if(str.GetLength() != (int)strlen(pTest))
		{
			printf("fail on str.GetLength\n");
			return false;
		}

	}

	{
		AqString str(pTest);
		AqString str2(pTest2);
		AqString str0;

		str.Swap(str2);
		if(str != pTest2 || str2 != pTest)
		{
			printf("fail on str.swap two string\n");
			return false;
		}
		
		str2.Swap(str0);
		if(str0 != pTest || str2 != "")
		{
			printf("fail on str.swap with empty string\n");
			return false;
		}

	}

	// TO DO add more test case;
	return true;
}


//-----------------------------------------------------------------------------
// SH, 2006-08-16
// Functions to convert number to string and string to number
// Move code from rtvloadoption
#include <assert.h>
bool AqString::ConvertStringToNumber(const char *iString,
									 const eNumberType iType,
									 void * oNumber)
{
	bool status = true;
	try
	{
		if (iType == kFloat)
		{
			*(float*)oNumber = float(atof(iString));
		}
		else if (iType == kDouble)
		{
			*(double*)oNumber = atof(iString);
		}
		else if (iType == kShort)
		{
			*(short*)oNumber = short(atoi(iString));
		}
		else if (iType == kInt)
		{
			*(int*) oNumber = int(atoi(iString));
		}
		else if (iType == kLong)
		{
			*(long*) oNumber =  atoi(iString);
		}
		else if (iType == kULong)
		{
			*(unsigned long*) oNumber =  unsigned long(atoi(iString));
		}
		else if (iType == kUShort)
		{
			*(unsigned short*)oNumber = unsigned short(atoi(iString));
		}
		else if (iType == kUInt)
		{
			*(unsigned int*) oNumber = unsigned int(atoi(iString));
		}
		else if (iType == kChar)
		{
			*(char*) oNumber = char(atoi(iString));
		}
		else if (iType == kUChar)
		{
			*(unsigned char*) oNumber = unsigned char(atoi(iString));
		}
		else if (iType == kBool)
		{
			*(bool *)oNumber = (iString[0] == 'y' || iString[0] == 'Y' ||
				iString[0] == 't' || iString[0] == 'T' ||
				atoi(iString) > 0);
		}
		else
		{
			assert(0);
			fprintf(stderr,"Convert: unknown type %d\n", iType);
			status = false;
		}
	}
	catch (...)
	{
		assert(0);
		status = false;
	}

	return status;
}

//--------------------------------------------------------------------------------
static const int kMaxValueLengthInString = (1024+4);

bool	AqString::ConvertNumberToString(const void * iNumber,
										const eNumberType iType,
										AqString & oString)
{
	char stringBuffer[kMaxValueLengthInString];

	bool status = true;
	try
	{
		if (iType == kFloat)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%g" ,*(float*)iNumber);
		}
		else if (iType == kDouble)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%g" ,*(double*)iNumber);
		}
		else if (iType == kShort)	
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%d" ,*(short*)iNumber);
		}
		else if (iType == kUShort)	
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%u" ,*(unsigned short*)iNumber);
		}
		else if (iType == kInt)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%d" ,*(int*)iNumber);
		}
		else if (iType == kUInt)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%u" ,*(unsigned int*)iNumber);
		}
		else if (iType == kLong)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%d" ,*(long*)iNumber);
		}
		else if (iType == kULong)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%u" ,*(unsigned long*)iNumber);
		}
		else if (iType == kChar)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%d",*(char *)iNumber);
		}
		else if (iType == kUChar)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%u",*(unsigned char *)iNumber);
		}
		else if (iType == kBool)
		{
			_snprintf(stringBuffer,kMaxValueLengthInString,"%d", *(bool *)iNumber ? 1:0);
		}
		else
		{
			assert(0);
			fprintf(stderr,"Bad Type: %d\n", iType);
			*stringBuffer = '\0';
			status = false;
		}
	}
	catch (...)
	{
		fprintf(stderr,"LoadOption::conversion threw exception \n");
		status = false;
	}

	stringBuffer[kMaxValueLengthInString - 1] = '\0';

	oString = stringBuffer;

	return status;
}


