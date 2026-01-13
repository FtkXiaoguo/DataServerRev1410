/***********************************************************************
 * AqTString.h
 *---------------------------------------------------------------------
 */


#ifndef AQTSTRING_H_
#define AQTSTRING_H_

#pragma warning (disable: 4530)
#pragma warning (disable: 4996)
#pragma warning (disable: 4018)


#include "AqCore.h"
#include <assert.h>
#include <stdio.h>
#include <vector>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

//#define _WIN32_DCOM
#include "Windows.h"


template<class T>
struct AqTStringData
{
	unsigned int m_refCount;
	unsigned int m_bufferSize;

	// the real buffer following this struct
	T* GetBuffer()
	{ return (T*)(this + 1); }

};

_declspec(selectany) unsigned int gEmptyStringData[] = { 0, 0, 0, 0 }; // two bytes terminator data for both asii and unicode


template<class T1, class T2>
class  AqTString
{
public :
	AqTString() { Init(); };

	AqTString (const AqTString& iStr) {Init(); *this = iStr; }
	AqTString (const T1* iStr){ Init(); Copy(iStr); }
	AqTString (T1 iChar){ Init(); Copy(iChar); }
	AqTString (const T2* iStr, unsigned int iCodePage){ Init(); Convert(iStr, iCodePage); }
	
	~AqTString(){Release();};


	bool InBuffer(const void* iptr) const {return (iptr && (iptr >= m_pBuffer  && iptr <= m_pBuffer+GetBufferSize()) );}
	unsigned int GetBufferSize() const {return GetDataInfo()->m_bufferSize;}
	unsigned int GetLength() const 
	{	
		if(m_pBuffer == c_pEmptyBuffer)	return 0;	
		return StrLen(m_pBuffer);
	}

	bool IsEmpty() const {return GetLength() == 0;}

	bool Swap(AqTString& iStr);

	T1 operator [](int iIndex) const { assert(iIndex >= 0 && iIndex <= (int)GetBufferSize()); return m_pBuffer[iIndex]; }
	void SetAt(T1 iChar, unsigned int iIndex) {	assert(iIndex < GetBufferSize()); m_pBuffer[iIndex] = iChar; }

		
	const AqTString& operator = (const AqTString& iStr);
	const AqTString& operator = (const T1* istr) {Copy(istr); return *this;}
	const AqTString& operator = (T1 iChar) {Copy(iChar); return *this;}

	T1 * Copy(const T1* iStr);
	T1 * Copy(T1 iChar);

	AqTString& operator +=(const AqTString& iStr) {Concat(GetString(), GetLength(), iStr, iStr.GetLength()); return *this;}
	AqTString& operator +=(T1 iChar) {Concat(GetString(), GetLength(), &iChar, 1); return *this;}
	AqTString& operator +=(const T1* iStr) {
		assert(IsStringGood(iStr)); Concat(GetString(), GetLength(), iStr,  StrLen(iStr)); return *this;
	}

	friend AqTString operator +(const AqTString& iStr1, const AqTString& iStr2);
	friend AqTString operator +(const AqTString& iStr, T1 iChar);
	friend AqTString operator +(T1 iChar, const AqTString& iStr);
	friend AqTString operator +(const AqTString& iStr1, const T1* iStr2);
	friend AqTString operator +(const T1* iStr1, const AqTString& iStr2);

	int Format(const T1* iFormatStr, ...);
	int VFormat(const T1*  iFormatStr, va_list iArgList);

	int Compare(const AqTString& s) const {return Compare( (const T1*)s );}
	int Compare(const T1* s) const {return (!s)? 1 : StrCmp(m_pBuffer, s);}

	int CompareNoCase(const AqTString& s) const {return CompareNoCase( (const T1*)s );}
	int CompareNoCase(const T1* s) const {return (!s)? 1 : StrNoCaseCmp(m_pBuffer, s);}

	operator const T1 *() const {return m_pBuffer;}
	const T1 * GetString() const {return m_pBuffer;}

	const T1* Convert (const T2* iString, unsigned int iCodePage);
	const T1* ConvertUTF8 (const T2* iString) {return Convert(iString, CP_UTF8);}
	
	AqTString Mid( int iFirst, int iCount  ) const;
	AqTString Left( int iCount ) const;
	AqTString Right( int iCount ) const;

	T1* TrimRight(int iCodePage=0);
	T1* TrimLeft(int iCodePage=0);
	T1* TruncString (unsigned int iSize);
	T1* Resize (unsigned int iSize);


	void ToUpper();
	void ToLower();
	
	int Find(const T1 * iSubStr, int iStart=0) const;
	int Replace(T1 iOldChar, T1 iNewChar, int iCodePage=0);
	int Replace(const T1* iOldStr, const T1* iNewStr);

	void Reverse();
	
	void Empty() {Release();}
	T1* GetBuffer(unsigned int iBufferMinSize=0);

	
protected:
	bool Allocate(int iLen);
	void Release();

	bool Concat(const T1* iData1, int iSize1, const T1* iData2, int iSize2);
	AqTStringData<T1>* GetDataInfo() const {assert(m_pBuffer != 0); return ((AqTStringData<T1>*)m_pBuffer) - 1;}
	void Init()	{ m_pBuffer = (T1*)(c_pEmptyBuffer); }
	
	T1* m_pBuffer;

	static const AqTStringData<T1>* c_pEmptyData;
	static const T1*  c_pEmptyBuffer;


public:
	static unsigned int StrLen(const T1* iStr);
	static int Vsprintf(T1* buffer, size_t count, const T1* format, va_list argptr );
	static int IsSpace(T1 c);
	static  T1* StrStr( T1* iStr, const T1* iStrCharSet);
	static const T1* StrStr( const T1* iStr, const T1* iStrCharSet );
 	static int StrCmp(const T1* iStr1, const T1* iStr2);
 	static int StrNoCaseCmp(const T1* iStr1, const T1* iStr2);
	static T1* NextChar( const T1* iStr, unsigned int iCodePage);
	static int Convert(unsigned int iCodePage, unsigned int iFlags, const T2* iStr, int inByte, T1* oBuffer, int iBufSize);

};




//-----------------------------------------------------------------------------
template<class T1, class T2>
bool AqTString<T1, T2>::Allocate(int iLen)
{
	assert(iLen >= 0);
	assert(iLen <= (2147483647 - 1)/sizeof(T1));

	Release();

	if (iLen == 0)
		return true;

	int bSize  = (iLen+1)*sizeof(T1);
	
	AqTStringData<T1>* pData = 0;
	
	try {pData = (AqTStringData<T1>*)new char[sizeof(AqTStringData<T1>) + bSize];} catch(...) {}

	if(pData == 0)
	{
		GetAqLogger()->LogMessage("Error: (AqTString::Allocate) failed\n");
		return false;
	}

	pData->m_bufferSize = bSize;
	m_pBuffer = pData->GetBuffer();
	
	m_pBuffer[iLen] = 0;

	return true;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
void AqTString<T1, T2>::Release()
{
	AqTStringData<T1>* pData = GetDataInfo();
	if (pData != c_pEmptyData)
	{
		delete[] (char*)pData;
		Init();
	}
}

//-----------------------------------------------------------------------------	
template<class T1, class T2>
T1* AqTString<T1, T2>::GetBuffer(unsigned int iBufferMinSize) 
{
	AqTStringData<T1>* pData = GetDataInfo();
	if (iBufferMinSize < 1)
	{
		assert(pData->m_bufferSize > 0); // can't use empty buffer
	}
	else if(pData->m_bufferSize < iBufferMinSize)
	{
		Allocate(iBufferMinSize);
		m_pBuffer[0] = 0;
	}
	
	return m_pBuffer;
}
	

//-----------------------------------------------------------------------------
template<class T1, class T2>
const AqTString<T1, T2>& AqTString<T1, T2>::operator = (const AqTString<T1, T2>& iStr)
{
	// TO DO, when have reference count, do ref increase
	Copy(iStr.m_pBuffer);
	return *this;
}

//-----------------------------------------------------------------------------
template<class T1, class T2>
T1* AqTString<T1, T2>::Copy(const T1* iString)
{
	if( iString == m_pBuffer )
		return m_pBuffer;

	if(!IsStringGood(iString) || !iString[0])
	{
		Release();
		return m_pBuffer;
	}

	int nBytesSource = StrLen(iString);
	if(nBytesSource == 0)
	{
		Release();
		return m_pBuffer;
	}

	
	// save string in case it is inside this string
	AqTString<T1, T2> tstr; 
	Swap(tstr);
	
	if(!Allocate(nBytesSource))
		return 0;

	memcpy((char*)m_pBuffer, (char*)iString, nBytesSource*sizeof(T1));
	return m_pBuffer;
}



//-----------------------------------------------------------------------------
template<class T1, class T2>
T1* AqTString<T1, T2>::Copy(T1 iChar)
{
	if(!Allocate(1))
		return 0;

	m_pBuffer[0] = iChar;
	return m_pBuffer;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
bool AqTString<T1, T2>::Concat(const T1* iStr1, int iSize1, const T1* iStr2, int iSize2)
{
	assert(IsStringGood(iStr1));
	assert(IsStringGood(iStr2));

	AqTString<T1, T2> str;
	
	if(!str.Allocate(iSize1+iSize2))
		return false;

	char* p = (char*)str.m_pBuffer;

	iSize1 = iSize1*sizeof(T1);
	iSize2 = iSize2*sizeof(T1);

	memcpy(p, (const void*)iStr1, iSize1);
	memcpy(p+iSize1, (const void*)iStr2, iSize2);

	this->Swap(str);
	return true;
}



//-----------------------------------------------------------------------------
template<class T1, class T2>
int AqTString<T1, T2>::Format(const T1* iFormatStr, ...)
{
	assert(IsStringGood(iFormatStr));

	va_list argList;
	va_start(argList, iFormatStr);
	int rcode = VFormat(iFormatStr, argList);
	va_end(argList);
	return rcode;
}

//-----------------------------------------------------------------------------
template<class T1, class T2>
int AqTString<T1, T2>::VFormat(const T1* iFormatStr, va_list iArgList)
{
	int count;
	int bufSize;

	int slen=0, nArg=0;
	const T1* p = iFormatStr;
	if(sizeof(T1) != 1)
	{
		while(*p != 0)
		{
			if(*p == L'%')
				nArg++;
			slen++;
			p++;
		}
	}
	else
	{
		while(*p != 0)
		{
			if(*p == '%')
				nArg++;
			slen++;
			p++;
		}

	}
	bufSize = slen + nArg*64 + 128; // estmated buffer size

	
	// save this string, in case it is used in formating
	AqTString<T1, T2> tstr;
	Swap(tstr);
	
	while(true)
	{
		if(!Allocate(bufSize))
			break;

		va_list myArgList = iArgList;
		count = Vsprintf(m_pBuffer, bufSize, iFormatStr, myArgList);

		if(count >= 0)
		{
			AqTString<T1, T2> copystr;
			Swap(copystr);

			if(!Allocate(count))
				break;

			memcpy(m_pBuffer, copystr.m_pBuffer, count*sizeof(T1));
			return count;

		}
		bufSize = bufSize * 5;

	}

	return 0;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
bool AqTString<T1, T2>::Swap(AqTString<T1, T2>& iStr)
{
	T1 *pSaved = m_pBuffer;

	m_pBuffer = iStr.m_pBuffer;
	iStr.m_pBuffer = pSaved;

	return true;
}

//-----------------------------------------------------------------------------
template<class T1, class T2>
T1* AqTString<T1, T2>::TruncString (unsigned int iSize)
{
	if (StrLen(m_pBuffer) > iSize)
		m_pBuffer[iSize] = 0;
	return m_pBuffer;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
T1* AqTString<T1, T2>::Resize (unsigned int iSize)
{

	if(iSize < 0)
		return m_pBuffer;

	unsigned int oBufSize = GetBufferSize();

	AqTString<T1, T2> orgStr;
	Swap(orgStr); // save string for copy back
	if(!Allocate(iSize))
	{
		Swap(orgStr);
		return m_pBuffer;
	}

	// save back
	if(oBufSize < iSize)
	{
		memcpy(m_pBuffer, orgStr.m_pBuffer, oBufSize*sizeof(T1));
	}
	else
	{
		memcpy(m_pBuffer, orgStr.m_pBuffer, iSize*sizeof(T1));
		m_pBuffer[iSize] = 0;
	}
	
	return m_pBuffer;
}



//-----------------------------------------------------------------------------
template<class T1, class T2>
AqTString<T1, T2> AqTString<T1, T2>::Mid( int iFirst, int iCount  ) const
{
	AqTString<T1, T2> str;
	int slen = GetLength();
	if(iCount < 1 || iFirst+iCount > slen )
		return str;
	
	str.Allocate(iCount);
	memcpy(str.m_pBuffer, m_pBuffer+iFirst, iCount*sizeof(T1));
	return str;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
AqTString<T1, T2> AqTString<T1, T2>::Left(int iCount  ) const
{
	AqTString<T1, T2> str;
	int slen = GetLength();
	if(iCount < 1)
		return str;

	if( iCount >= slen)
		return *this;
	
	str.Allocate(iCount);
	memcpy(str.m_pBuffer, m_pBuffer, iCount*sizeof(T1));
	return str;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
AqTString<T1, T2> AqTString<T1, T2>::Right(int iCount  ) const
{
	AqTString<T1, T2> str;
	int slen = GetLength();
	if(iCount < 1)
		return str;

	if( iCount >= slen)
		return *this;
	
	str.Allocate(iCount);
	
	memcpy(str.m_pBuffer, m_pBuffer + slen-iCount, iCount*sizeof(T1));
	return str;
}



//-----------------------------------------------------------------------------
template<class T1, class T2>
T1* AqTString<T1, T2>::TrimRight(int iCodePage)
{
	//CopyOnWrite();

	T1* p = m_pBuffer;
	T1* pEnd = 0;

	// scan from head for mult-byte safe
	while (*p != 0)
	{
		if (IsSpace(*p))
		{
			if (pEnd == 0)
				pEnd = p;
		}
		else
		{
			pEnd = 0;
		}
		p = NextChar(p, iCodePage);
	}

	if (pEnd)
		*pEnd = 0;

	return m_pBuffer;

}


//-----------------------------------------------------------------------------
template<class T1, class T2>
T1 * AqTString<T1, T2>::TrimLeft(int iCodePage)
{
 
	//CopyOnWrite();

	T1* p = m_pBuffer;
	if(!IsSpace(*p))
		return m_pBuffer;
	else
		p = NextChar(p, iCodePage);

	while (*p != 0 && IsSpace(*p))
	{
		p = NextChar(p, iCodePage);
	}

	Copy(p);
	return m_pBuffer;

}


//-----------------------------------------------------------------------------
template<class T1, class T2>
int AqTString<T1, T2>::Find(const T1 * iSubStr, int iStart) const
{
	if(!IsStringGood(iSubStr) || !iSubStr[0] || iStart > GetLength())
		return -1;

	const T1 * p = StrStr(m_pBuffer + iStart, iSubStr);
	return (p == 0) ? -1 : (int)(p - m_pBuffer);
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
int AqTString<T1, T2>::Replace(T1 iOldChar, T1 iNewChar, int iCodePage)
{
	int count = 0;

	if (iOldChar == iNewChar)
		return count;

	//CopyOnWrite();
	T1 * p = m_pBuffer;
	T1 * pEnd = p + GetLength();
	
	//TO DO, cache string length

	while (p < pEnd)
	{
		// replace instances of the specified character only
		if (*p == iOldChar)
		{
			*p = iNewChar;
			count++;
		}
		p = NextChar(p, iCodePage);
	}

	return count;
}


//-----------------------------------------------------------------------------
template<class T1, class T2>
int AqTString<T1, T2>::Replace(const T1 * iOldStr, const T1 * iNewStr)
{

	if(!IsStringGood(iOldStr) || !IsStringGood(iNewStr))
		return 0;

	int sLenOld = StrLen(iOldStr);
	if (sLenOld == 0)
		return 0;

	// find new string length
	int count = 0;
	 T1* pStart = m_pBuffer;
	 T1* pTarget;
	while ((pTarget = StrStr(pStart, iOldStr)) != 0)
	{
		count++;
		pStart = pTarget + sLenOld;
	}

	if (count == 0)
		return 0;

	int sLenNew = StrLen(iNewStr);
	int strLength = GetLength();
	int newLength =  strLength + (sLenNew - sLenOld) * count;

	if(sLenNew > sLenOld)
		Resize(newLength);
	
	// do replace
	int balance;
	pStart = m_pBuffer;

	while ( (pTarget = StrStr(pStart, iOldStr)) != 0)
	{
		balance = strLength - ((int)(pTarget - m_pBuffer) + sLenOld);
		memmove((void*)(pTarget + sLenNew), pTarget + sLenOld, balance * sizeof(T1));
		memcpy((void*)pTarget, iNewStr, sLenNew * sizeof(T1));
		pStart = pTarget + sLenNew;
		pStart[balance] = 0;
		strLength += (sLenNew - sLenOld);
	}


	return count;
}



//-----------------------------------------------------------------------------
template<class T1, class T2>
void AqTString<T1, T2>::Reverse()
{
	T1 *q, *p, t;
	
	q = m_pBuffer;
	p = q+GetLength()-1;
	while (q < p)
	{
		t = *q;
		*q = *p;
		*p = t;
		q++;
		p--;
	}

}



//-----------------------------------------------------------------------------
template<class T1, class T2>
const T1* AqTString<T1, T2>::Convert (const T2* iStr, unsigned int iCodePage) 
{
	if(!IsStringGood(iStr) || !iStr[0])
	{
		Release();
		return m_pBuffer;
	}
	
	
	AqTString<T1, T2> tmpBuf;
	if(InBuffer(iStr))
	{
		if(!Swap(tmpBuf))
			return m_pBuffer;
	}

	// iCodePage = CP_UTF8; 
	// iCodePage = CP_ACP; //CP_ACP is 0

	// Converts a string into a Unicode wide-character string
	// Adds zero-termination
	int nBytesSource = AqTString<T2, T1>::StrLen(iStr);
	// Query the number of WChars required to store the destination string
	int nCharNeeded = Convert (iCodePage, 0, iStr, nBytesSource, 0, 0);
	
	// Allocate the required amount of space plus 2 more bytes for '\0'
	Allocate(nCharNeeded);

	// Do the conversion
	nCharNeeded = Convert(iCodePage, 0, iStr, nBytesSource, m_pBuffer, nCharNeeded);
	
	*(m_pBuffer + nCharNeeded) = 0;

	return m_pBuffer;
}


template<class T1, class T2>
inline AqTString<T1, T2> operator +(const AqTString<T1, T2>& iStr1, const AqTString<T1, T2>& iStr2)
{ AqTString<T1, T2> str; str.Concat(iStr1, iStr1.GetLength(), iStr2, iStr2.GetLength()); return str; }

template<class T1, class T2>
inline AqTString<T1, T2> operator +(const AqTString<T1, T2>& iStr, T1 iChar)
{ AqTString<T1, T2> str; str.Concat(iStr, iStr.GetLength(), &iChar, 1); return str;}

template<class T1, class T2>
inline AqTString<T1, T2> operator +(T1 iChar, const AqTString<T1, T2>& iStr)
{ AqTString<T1, T2> str; str.Concat(&iChar, 1, iStr, iStr.GetLength()); return str;}

template<class T1, class T2>
inline AqTString<T1, T2> operator +(const AqTString<T1, T2>& iStr1, const T1* iStr2)
{
	assert(IsStringGood(iStr2));
	AqTString<T1, T2> str; str.Concat(iStr1, iStr1.GetLength(), iStr2,  AqTString<T1, T2>::StrLen(iStr2));	return str;
}

template<class T1, class T2>
inline AqTString<T1, T2> operator +(const T1* iStr1, const AqTString<T1, T2>& iStr2)
{
	assert(IsStringGood(iStr1));
	AqTString<T1, T2> str;	str.Concat(iStr1, AqTString<T1, T2>::StrLen(iStr1), iStr2, iStr2.GetLength());	return str;
}


// Compare helpers
template<class T1, class T2>
inline bool operator ==(const AqTString<T1, T2>& s1, const AqTString<T1, T2>& s2)	{ return s1.Compare(s2) == 0; }
template<class T1, class T2>
inline bool operator ==(const AqTString<T1, T2>& s1, const T1* s2)	{ return s1.Compare(s2) == 0; }
template<class T1, class T2>
inline bool operator ==(const T1* s1, const AqTString<T1, T2>& s2)	{ return s2.Compare(s1) == 0; }

template<class T1, class T2>
inline bool operator !=(const AqTString<T1, T2>& s1, const AqTString<T1, T2>& s2) { return s1.Compare(s2) != 0; }
template<class T1, class T2>
inline bool operator !=(const AqTString<T1, T2>& s1, const T1* s2) { return s1.Compare(s2) != 0; }
template<class T1, class T2>
inline bool operator !=(const T1* s1, const AqTString<T1, T2>& s2)	{ return s2.Compare(s1) != 0; }

template<class T1, class T2>
inline bool operator <(const AqTString<T1, T2>& s1, const AqTString<T1, T2>& s2) { return s1.Compare(s2) < 0; }
template<class T1, class T2>
inline bool operator <(const AqTString<T1, T2>& s1, const T1* s2) { return s1.Compare(s2) < 0; }
template<class T1, class T2>
inline bool operator <(const T1* s1, const AqTString<T1, T2>& s2) { return s2.Compare(s1) > 0; }

template<class T1, class T2>
inline bool operator >(const AqTString<T1, T2>& s1, const AqTString<T1, T2>& s2) { return s1.Compare(s2) > 0; }
template<class T1, class T2>
inline bool operator >(const AqTString<T1, T2>& s1, const T1* s2) { return s1.Compare(s2) > 0; }
template<class T1, class T2>
inline bool operator >(const T1* s1, const AqTString<T1, T2>& s2) { return s2.Compare(s1) < 0; }

template<class T1, class T2>
inline bool operator <=(const AqTString<T1, T2>& s1, const AqTString<T1, T2>& s2) { return s1.Compare(s2) <= 0; }
template<class T1, class T2>
inline bool operator <=(const AqTString<T1, T2>& s1, const T1* s2)	{ return s1.Compare(s2) <= 0; }
template<class T1, class T2>
inline bool operator <=(const T1* s1, const AqTString<T1, T2>& s2)	{ return s2.Compare(s1) >= 0; }

template<class T1, class T2>
inline bool operator >=(const AqTString<T1, T2>& s1, const AqTString<T1, T2>& s2) { return s1.Compare(s2) >= 0; }
template<class T1, class T2>
inline bool operator >=(const AqTString<T1, T2>& s1, const T1* s2)	{ return s1.Compare(s2) >= 0; }
template<class T1, class T2>
inline bool operator >=(const T1* s1, const AqTString<T1, T2>& s2)	{ return s2.Compare(s1) <= 0; }




//************************************ AqString/AqUString *********************************************************

typedef AqTString<char, wchar_t> AqString;
typedef AqTString<wchar_t, char> AqUString;



_declspec(selectany) const AqTStringData<char>* AqString::c_pEmptyData = (AqTStringData<char>*)&gEmptyStringData;
_declspec(selectany) const AqTStringData<wchar_t>* AqUString::c_pEmptyData = (AqTStringData<wchar_t>*)&gEmptyStringData;

_declspec(selectany) const char*  AqString::c_pEmptyBuffer = (const char*)(((char*)&gEmptyStringData) + sizeof(AqTStringData<char>));
_declspec(selectany) const wchar_t*  AqUString::c_pEmptyBuffer = (const wchar_t*)(((char*)&gEmptyStringData) + sizeof(AqTStringData<wchar_t>));



//-----------------------------------------------------------------------------
inline bool IsStringGood(const char* iStr)
{
	if(iStr && !::IsBadStringPtrA(iStr, 0))
		return true;

	GetAqLogger()->LogMessage(kWarning, "Error: encounter bad string pointer\n");
	return false;
}


inline bool IsStringGood(const wchar_t* iStr)
{
	if(iStr && !::IsBadStringPtrW(iStr, 0))
		return true;

	GetAqLogger()->LogMessage(kWarning, "Error: encounter bad unicode string pointer\n");
	return false;
}


//-----------------------------------------------------------------------------
inline unsigned int AqString::StrLen(const char* iStr)  {return strlen(iStr);}
inline unsigned int AqUString::StrLen(const wchar_t* iStr) {return wcslen(iStr);}

//-----------------------------------------------------------------------------
inline int AqString::IsSpace(char c) {return isspace(c);}
inline int AqUString::IsSpace(wchar_t c) {return iswspace(c);}

//-----------------------------------------------------------------------------
//K.Ko 2010/04/23
inline   char* AqString::StrStr(  char *iStr, const char *iStrCharSet ) {return strstr(iStr, iStrCharSet);}
inline  const char* AqString::StrStr( const char *iStr, const char *iStrCharSet ) {return strstr(iStr, iStrCharSet);}

inline const wchar_t* AqUString::StrStr(const wchar_t *iStr, const wchar_t *iStrCharSet) { return wcsstr(iStr, iStrCharSet); }
inline       wchar_t* AqUString::StrStr( wchar_t *iStr, const wchar_t *iStrCharSet) { return wcsstr(iStr, iStrCharSet); }

//inline  const wchar_t* AqUString::StrStr( const wchar_t *iStr, const wchar_t *iStrCharSet ) {return wcsstr(iStr, iStrCharSet);}

//-----------------------------------------------------------------------------
inline void AqString::ToUpper() { CharUpperA(m_pBuffer); }
inline void AqUString::ToUpper() { CharUpperW(m_pBuffer);}

//-----------------------------------------------------------------------------
inline void AqString::ToLower() { CharLowerA(m_pBuffer); }
inline void AqUString::ToLower() {CharLowerW(m_pBuffer);}

//-----------------------------------------------------------------------------
inline char * AqString::NextChar( const char *iStr, unsigned int iCodePage) { return CharNextExA(iCodePage, iStr, 0); }
inline wchar_t * AqUString::NextChar( const wchar_t *iStr, unsigned int iCodePage) { return CharNextW(iStr); }


//-----------------------------------------------------------------------------
#if 0 //K.Ko 2010/04/23
inline int AqString::StrCmp(const T1* iStr1, const T1* iStr2) {return strcmp(iStr1, iStr2);}

inline int AqUString::StrCmp(const T1* iStr1, const T1* iStr2) {return wcscmp(iStr1, iStr2);}


//-----------------------------------------------------------------------------
inline int AqString::StrNoCaseCmp(const T1* iStr1, const T1* iStr2) {return stricmp(iStr1, iStr2);}

inline int AqUString::StrNoCaseCmp(const T1* iStr1, const T1* iStr2) {return wcsicmp(iStr1, iStr2);}
#else
inline int AqString::StrCmp(const char * iStr1, const char* iStr2) {return strcmp(iStr1, iStr2);}

inline int AqUString::StrCmp(const wchar_t* iStr1, const wchar_t* iStr2) {return wcscmp(iStr1, iStr2);}

//-----------------------------------------------------------------------------
inline int AqString::StrNoCaseCmp(const char* iStr1, const char* iStr2) {return stricmp(iStr1, iStr2);}

inline int AqUString::StrNoCaseCmp(const wchar_t* iStr1, const wchar_t* iStr2) {return wcsicmp(iStr1, iStr2);}
#endif 

//////
//--- {[( 2010/04/30

inline AqUString operator +(const wchar_t* iStr1, const AqUString& iStr2)
{
	assert(IsStringGood(iStr1));
	AqUString str;	str.Concat(iStr1, AqUString::StrLen(iStr1), iStr2, iStr2.GetLength());	return str;
}

inline AqString operator +(const char* iStr1, const AqString& iStr2)
{
	assert(IsStringGood(iStr1));
	AqString str;	str.Concat(iStr1, AqString::StrLen(iStr1), iStr2, iStr2.GetLength());	return str;
}


inline AqUString operator +(const AqUString& iStr1, const wchar_t* iStr2)
{
	assert(IsStringGood(iStr2));
	AqUString str; str.Concat(iStr1, iStr1.GetLength(), iStr2,  AqUString::StrLen(iStr2));	return str;
}

inline AqString operator +(const AqString& iStr1, const char* iStr2)
{
	assert(IsStringGood(iStr2));
	AqString str; str.Concat(iStr1, iStr1.GetLength(), iStr2,  AqString::StrLen(iStr2));	return str;
}

inline AqUString operator +(const AqUString& iStr1, const AqUString& iStr2)
{ AqUString str; str.Concat(iStr1, iStr1.GetLength(), iStr2, iStr2.GetLength()); return str; }

inline AqString operator +(const AqString& iStr1, const AqString& iStr2)
{ AqString str; str.Concat(iStr1, iStr1.GetLength(), iStr2, iStr2.GetLength()); return str; }

//---- )]}

//-----------------------------------------------------------------------------
inline int AqString::Convert(unsigned int iCodePage, unsigned int iFlags, const wchar_t* iStr, int inByte, char* oBuffer, int iBufSize)
{
	return WideCharToMultiByte (iCodePage, iFlags, iStr, inByte, oBuffer, iBufSize, 0, 0);
}


inline int AqUString::Convert(unsigned int iCodePage, unsigned int iFlags, const char* iStr, int inByte, wchar_t* oBuffer, int iBufSize)
{
	return MultiByteToWideChar (iCodePage, iFlags, iStr, inByte, oBuffer, iBufSize);
}



//-----------------------------------------------------------------------------
inline int AqString::Vsprintf(char *buffer, size_t count, const char *format, va_list argptr ) 
{return _vsnprintf(buffer, count, format, argptr );}

inline int AqUString::Vsprintf(wchar_t *buffer, size_t count, const wchar_t *format, va_list argptr ) 
{return _vsnwprintf(buffer, count, format, argptr );}


//-----------------------------------------------------------------------------
inline bool StrToNumber(const char* iStr, int& oValue)
{
	if(!IsStringGood(iStr))
		return false;
	oValue = atoi(iStr);
	return true;
}

inline bool StrToNumber(const char* iStr, long& oValue)
{
	if(!IsStringGood(iStr))
		return false;
	oValue = atol(iStr);
	return true;
}

inline bool StrToNumber(const char* iStr, double& oValue)
{
	if(!IsStringGood(iStr))
		return false;
	oValue = atof(iStr);
	return true;
}

inline bool StrToNumber(const char* iStr, float& oValue)
{
	if(!IsStringGood(iStr))
		return false;
	oValue = (float)atof(iStr);
	return true;
}


//-----------------------------------------------------------------------------
inline char * AqBinToHex(const void *iData, int iDataSize, AqString& oStr)
{
	return AqBinToHex(iData, iDataSize, (char *)(oStr.GetBuffer(iDataSize*2+2)));
}

//-----------------------------------------------------------------------------
struct AqStringKV
{
	AqString Key;
	AqString Value;
};

//-----------------------------------------------------------------------------
struct AqUStringKV
{
	AqUString Key;
	AqUString Value;
};

//-----------------------------------------------------------------------------
struct AqMixStringKV
{
	AqUString Key;
	AqString Value;
};


//-----------------------------------------------------------------------------
class AqStrList
{
public:
	AqStrList() {};
	virtual ~AqStrList() {};

	virtual int Size() const = 0;
	virtual void Resize(unsigned int iSize) = 0;
	virtual void Clear() = 0;
	
	virtual void Append(const char* iStr) = 0;
	virtual const char* GetAt(unsigned int index) const = 0;
	virtual const char* operator [](unsigned int index) const = 0;
	virtual void SetAt(const char* iStr, unsigned int index) = 0;
};


//-----------------------------------------------------------------------------
class AqStringList : public AqStrList
{
public:
	AqStringList() {};
	AqStringList(const AqStringList& iList) { m_instances = iList.m_instances; }
	virtual ~AqStringList() {};

	virtual int Size() const {return m_instances.size();}
	virtual void Resize(unsigned int iSize) { m_instances.resize(iSize);}
	virtual void Clear(){m_instances.clear();}
	
	virtual void Append(const char* iStr) {m_instances.push_back(iStr);}
	virtual const char* GetAt(unsigned int index) const {if(index >= Size()) return 0; return m_instances[index];}
	virtual const char* operator [](unsigned int index) const {if(index >= Size()) return 0; return m_instances[index];}
	virtual void SetAt(const char* iStr, unsigned int index) {if(index >= Size()) return; m_instances[index] = iStr;}
	
	const AqStringList& operator = (const AqStringList& iList) {m_instances = iList.m_instances; return *this;}

	std::vector<AqString>	m_instances; 

};


#endif //AQTSTRING_H_
