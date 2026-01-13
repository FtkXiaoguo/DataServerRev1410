/***********************************************************************
 * AqString.h
 *---------------------------------------------------------------------
 */


#ifndef AQSTRING_H_
#define AQSTRING_H_

#include "AqCore/AqTString.h"
#if 0

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


struct AqStringData
{
	int m_refCount;
	int m_bufferSize;

	// the real buffer following this struct
	char* GetBuffer()
	{ return (char*)(this + 1); }

};


struct AqUStringData
{
	int m_refCount;
	int m_bufferSize;

	// the real buffer following this struct
	wchar_t* GetBuffer()
	{ return (wchar_t*)(this + 1); }

};


class AqUString;

class AqString
{
public :
	AqString() { Init(); };

	AqString (const AqString& iStr) {Init(); *this = iStr; }
	AqString (const char* iStr){ Init(); Copy(iStr); }
	AqString (char iChar){ Init(); Copy(iChar); }

	~AqString(){Release();};

	bool InBuffer(const void* iptr) const {return (iptr && (iptr >= m_pBuffer  && iptr <= m_pBuffer+GetBufferSize()) );}
	int GetBufferSize() const {return GetDataInfo()->m_bufferSize;}
	int GetLength() const {	if(m_pBuffer == c_pEmptyBuffer)	return 0;	return strlen(m_pBuffer);}
	bool IsEmpty() const {return GetLength() == 0;}

	bool Swap(AqString& iStr);

	char operator [](int iIndex) const;
	void SetAt(char iChar, int iIndex);		// no string mode check
		
	const AqString& operator = (const AqString& iStr);
	const AqString& operator = (const char* istr) {Copy(istr); return *this;}
	const AqString& operator = (char iChar) {Copy(iChar); return *this;}

	void * CopyBin(const void* iData, unsigned int iLen);
	char * Copy(const char* iStr);
	char * Copy(char iChar);

	AqString& operator +=(const AqString& iStr) {Concat(GetString(), GetLength(), iStr, iStr.GetLength()); return *this;}
	AqString& operator +=(char iChar) {Concat(GetString(), GetLength(), &iChar, 1); return *this;}
	AqString& operator +=(const char* iStr) {
		assert(AqString::IsStringGood(iStr)); Concat(GetString(), GetLength(), iStr, strlen(iStr)); return *this;
	}

	friend AqString operator +(const AqString& iStr1, const AqString& iStr2);
	friend AqString operator +(const AqString& iStr, char iChar);
	friend AqString operator +(char iChar, const AqString& iStr);
	friend AqString operator +(const AqString& iStr1, const char* iStr2);
	friend AqString operator +(const char* iStr1, const AqString& iStr2);

	int Format(const char* iFormatStr, ...);
	int VFormat(const char*  iFormatStr, va_list iArgList);

	int Compare(const AqString& s) const {return Compare( (const char*)s );}
	int Compare(const char* s) const {return (!s)? 1 : strcmp(m_pBuffer, s);}

	operator const char *() const {return m_pBuffer;}
	const char * GetString() const {return m_pBuffer;}

	char * BinToHex(const void *iData=0, int iDataSize=0);
	void * HexToBin(const char* iString=0);

	char* ToMultiByte (const wchar_t* iuString, unsigned int iCodePage=0);

	
	AqString Mid( int iFirst, int iCount  ) const;
	AqString Left( int iCount ) const;
	AqString Right( int iCount ) const;

	char* TrimRight();
	char* TrimLeft();
	char* TruncString (unsigned int iSize);
	char* Resize (unsigned int iSize);

	int Find(const char * iSubStr, int iStart=0) const;
	int Replace(char iOldChar, char iNewChar);
	int Replace(const char* iOldStr, const char* iNewStr);
	
	

	bool Allocate(int iLen);
	void Release();
	char* GetBuffer() {return m_pBuffer;}

	static bool  IsStringGood(const char* iStr, int iLength = -1);

	static bool UnitTest();

	// SH, add string to number conversion functions. 2006-08-16
	enum eNumberType
	{
		kUnknown, kBool, kShort, kUShort, kInt, kUInt,kLong, kULong,
		kFloat, kDouble, kChar,kUChar
	};

	static bool ConvertStringToNumber(const char *iString,const eNumberType iType,
									  void * oNumber);

	static bool	ConvertNumberToString(const void * iNumber,const eNumberType iType,
									  AqString & oString);

protected:
	bool Concat(const char* iData1, int iSize1, const char* iData2, int iSize2);
	AqStringData* GetDataInfo() const {assert(m_pBuffer != 0); return ((AqStringData*)m_pBuffer) - 1;}
	void Init()	{ m_pBuffer = (char*)(c_pEmptyBuffer); }
	

	char* m_pBuffer;

	static AqStringData* c_pEmptyData;
	static const char*  c_pEmptyBuffer;
};


inline AqString operator +(const AqString& iStr1, const AqString& iStr2)
{ AqString str; str.Concat(iStr1, iStr1.GetLength(), iStr2, iStr2.GetLength()); return str; }


inline AqString operator +(const AqString& iStr, char iChar)
{ AqString str; str.Concat(iStr, iStr.GetLength(), &iChar, 1); return str;}


inline AqString operator +(char iChar, const AqString& iStr)
{ AqString str; str.Concat(&iChar, 1, iStr, iStr.GetLength()); return str;}


inline AqString operator +(const AqString& iStr1, const char* iStr2)
{
	assert(AqString::IsStringGood(iStr2));
	AqString str; str.Concat(iStr1, iStr1.GetLength(), iStr2, strlen(iStr2));	return str;
}

inline AqString operator +(const char* iStr1, const AqString& iStr2)
{
	assert(AqString::IsStringGood(iStr1));
	AqString str;	str.Concat(iStr1, strlen(iStr1), iStr2, iStr2.GetLength());	return str;
}



// Compare helpers
inline bool operator ==(const AqString& s1, const AqString& s2)	{ return s1.Compare(s2) == 0; }
inline bool operator ==(const AqString& s1, const char* s2)	{ return s1.Compare(s2) == 0; }
inline bool operator ==(const char* s1, const AqString& s2)	{ return s2.Compare(s1) == 0; }

inline bool operator !=(const AqString& s1, const AqString& s2) { return s1.Compare(s2) != 0; }
inline bool operator !=(const AqString& s1, const char* s2) { return s1.Compare(s2) != 0; }
inline bool operator !=(const char* s1, const AqString& s2)	{ return s2.Compare(s1) != 0; }

inline bool operator <(const AqString& s1, const AqString& s2) { return s1.Compare(s2) < 0; }
inline bool operator <(const AqString& s1, const char* s2) { return s1.Compare(s2) < 0; }
inline bool operator <(const char* s1, const AqString& s2) { return s2.Compare(s1) > 0; }

inline bool operator >(const AqString& s1, const AqString& s2) { return s1.Compare(s2) > 0; }
inline bool operator >(const AqString& s1, const char* s2) { return s1.Compare(s2) > 0; }
inline bool operator >(const char* s1, const AqString& s2) { return s2.Compare(s1) < 0; }

inline bool operator <=(const AqString& s1, const AqString& s2) { return s1.Compare(s2) <= 0; }
inline bool operator <=(const AqString& s1, const char* s2)	{ return s1.Compare(s2) <= 0; }
inline bool operator <=(const char* s1, const AqString& s2)	{ return s2.Compare(s1) >= 0; }

inline bool operator >=(const AqString& s1, const AqString& s2) { return s1.Compare(s2) >= 0; }
inline bool operator >=(const AqString& s1, const char* s2)	{ return s1.Compare(s2) >= 0; }
inline bool operator >=(const char* s1, const AqString& s2)	{ return s2.Compare(s1) <= 0; }



struct AqPairString
{
	AqString Key;
	AqString Value;
};



class AqUString
{
public :
	AqUString() { Init(); };
	AqUString (const AqUString& iStr) {Init(); *this = iStr; }
	AqUString (const wchar_t* iUStr){ Init(); Copy(iUStr);}
	AqUString (wchar_t iUChar){ Init(); Copy(iUChar);}
	AqUString (const AqString& iStr) {Init(); *this = iStr; }
	AqUString (const char* iStr){ Init(); Copy(iStr);}
	AqUString (char iChar){ Init(); Copy(iChar);}

	~AqUString(){Release();};

	bool InBuffer(const void* iptr) const {char* p = (char*)m_pBuffer;	return (iptr && (iptr >= p  && iptr <= p+GetBufferSize()));}

	int GetBufferSize() const {return GetDataInfo()->m_bufferSize;}
	int GetLength() const {if(m_pBuffer == c_pEmptyBuffer)	return 0; return wcslen(m_pBuffer);}
	bool IsEmpty() const {return GetLength() == 0;}

	bool Swap(AqUString& iStr);

	wchar_t operator [](int iIndex) const;
	void SetAt(wchar_t iChar, int iIndex);	// no string mode check
		
	const AqUString& operator = (const AqUString& iStr);
	const AqUString& operator = (wchar_t iChar) {Copy(iChar); return *this;}
	const AqUString& operator = (const wchar_t* iustr) {Copy(iustr); return *this;}

	const AqUString& operator = (const AqString& iStr){Copy(iStr); return *this;};
	const AqUString& operator = (char iChar) {Copy(iChar); return *this;}
	const AqUString& operator = (const char* iStr) {Copy(iStr); return *this;}

	wchar_t* Copy(const wchar_t* iuStr);
	wchar_t* Copy(wchar_t iUChar);
	wchar_t* Copy(const char* iStr);
	wchar_t* Copy(char iChar);

	// concatenation, will convert string mode according to the first argment 
	AqUString& operator +=(const AqUString& iStr);
	AqUString& operator +=(const AqString& iStr);

	AqUString& operator +=(char iChar);
	AqUString& operator +=(wchar_t iUChar);

	AqUString& operator +=(const char* iStr);
	AqUString& operator +=(const wchar_t* iUStr);

	friend AqUString operator +(const AqUString& iStr1, const AqUString& iStr2);

	friend AqUString operator +(const AqUString& iStr, wchar_t iUChar);
	friend AqUString operator +(wchar_t iUChar, const AqUString& iStr);
	friend AqUString operator +(const AqUString& iStr, char iChar);
	friend AqUString operator +(char iChar, const AqUString& iStr);

	friend AqUString operator +(const AqUString& iStr, const wchar_t* iUStr);
	friend AqUString operator +(const wchar_t* iUStr, const AqUString& iStr);
	friend AqUString operator +(const AqUString& iStr1, const char* iStr2);
	friend AqUString operator +(const char* iStr1, const AqUString& iStr2);

	
	int Format(const wchar_t* iFormatStr, ...);
	int VFormat(const wchar_t* iFormatStr, va_list iArgList);


	int Compare(const AqUString& s) const {return Compare( s.GetString() ); }
	int Compare(const wchar_t* s) const {return (!IsStringGood(s))? 1 : wcscmp(m_pBuffer, s);}

	operator const wchar_t *() const {return m_pBuffer;}
	const wchar_t * GetString() const {return m_pBuffer;}

	wchar_t* ToUnicode (const char* iString, unsigned int iCodePage=0);

	AqUString Mid( int iFirst, int iCount  ) const;
	AqUString Left( int iCount ) const;
	AqUString Right( int iCount ) const;

	wchar_t* TrimRight();
	wchar_t* TrimLeft();
	wchar_t* TruncString (unsigned int iSize);
	wchar_t* Resize (unsigned int iSize);

	int Find(const wchar_t * iSubStr, int iStart=0) const;
	int Replace(wchar_t iOldChar, wchar_t iNewChar);
	int Replace(const wchar_t * iOldStr, const wchar_t * iNewStr);
	
	bool Allocate(int iLen);
	void Release();
	wchar_t* GetBuffer() {return m_pBuffer;}

	static bool  IsStringGood(const wchar_t* iStr, int iLength = -1);

protected:
	bool Concat(const wchar_t* iData1, int iSize1, const wchar_t* iData2, int iSize2);
	AqUStringData* GetDataInfo() const {assert(m_pBuffer != 0); return ((AqUStringData*)m_pBuffer) - 1;}
	void Init()	{ m_pBuffer = (wchar_t*)(c_pEmptyBuffer); }

	wchar_t* m_pBuffer;

	static AqUStringData* c_pEmptyData;
	static const wchar_t* c_pEmptyBuffer;

};



// Compare helpers
inline bool operator ==(const AqUString& s1, const AqUString& s2)	{ return s1.Compare(s2) == 0; }
inline bool operator ==(const AqUString& s1, const wchar_t* s2)	{ return s1.Compare(s2) == 0; }
inline bool operator ==(const wchar_t* s1, const AqUString& s2)	{ return s2.Compare(s1) == 0; }

inline bool operator !=(const AqUString& s1, const AqUString& s2) { return s1.Compare(s2) != 0; }
inline bool operator !=(const AqUString& s1, const wchar_t* s2) { return s1.Compare(s2) != 0; }
inline bool operator !=(const wchar_t* s1, const AqUString& s2)	{ return s2.Compare(s1) != 0; }

inline bool operator <(const AqUString& s1, const AqUString& s2) { return s1.Compare(s2) < 0; }
inline bool operator <(const AqUString& s1, const wchar_t* s2) { return s1.Compare(s2) < 0; }
inline bool operator <(const wchar_t* s1, const AqUString& s2) { return s2.Compare(s1) > 0; }

inline bool operator >(const AqUString& s1, const AqUString& s2) { return s1.Compare(s2) > 0; }
inline bool operator >(const AqUString& s1, const wchar_t* s2) { return s1.Compare(s2) > 0; }
inline bool operator >(const wchar_t* s1, const AqUString& s2) { return s2.Compare(s1) < 0; }


inline bool operator <=(const AqUString& s1, const AqUString& s2) { return s1.Compare(s2) <= 0; }
inline bool operator <=(const AqUString& s1, const wchar_t* s2)	{ return s1.Compare(s2) <= 0; }
inline bool operator <=(const wchar_t* s1, const AqUString& s2)	{ return s2.Compare(s1) >= 0; }

inline bool operator >=(const AqUString& s1, const AqUString& s2) { return s1.Compare(s2) >= 0; }
inline bool operator >=(const AqUString& s1, const wchar_t* s2)	{ return s1.Compare(s2) >= 0; }
inline bool operator >=(const wchar_t* s1, const AqUString& s2)	{ return s2.Compare(s1) <= 0; }



struct AqPairUString
{
	AqUString Key;
	AqUString Value;
};


// Murali Ayyapillai 2006.07.06
int UnicodeToUTF8(const wchar_t* iStr, int srclen, AqString& outStr);
int UTF8ToUnicode(const char* iStr, int srclen, AqUString& outStr);
int MBCSToUnicode(const char* iStr, int srclen, const char* scs, AqUString& outStr);
int UnicodeToMBCS(const wchar_t* iStr, int srclen, const char* scs, AqString& outStr);

#endif

#endif //AQSTRING_H_
