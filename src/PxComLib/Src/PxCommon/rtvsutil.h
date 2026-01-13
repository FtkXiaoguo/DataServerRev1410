/***********************************************************************
 * rtvsutil.h
 *---------------------------------------------------------------------
 *		Copyright, Mitsubishi Electric Information Technology Center 
 *					America, Inc., 2000, All rights reserved.
 *
 *		Copyright (c) TeraRecon,  Inc. 2001,2002
 *
 *
 *	PURPOSE:
 *		Utilities that convert error status into text. Also
 *      inline convenience functions
 *
 *	AUTHOR(S):  T.C. Zhao, August 2000.
 *
 *   
 *-------------------------------------------------------------------
 */

#ifndef RTVSUTIL_H_
#define RTVSUTIL_H_

//----------------------------------------------------------------
#ifndef iRTVSAlloc_
#define iRTVSAlloc_
/*
 * Utilities to guard a new'ed bunch of objects - similar to
 * the old old alloc.
 * Usage: iRTVSnewGuard<MyClass> p(10);
 *        MyClass *objs = p;
 */

template <class T> class iRTVSAlloc
{
public:
	iRTVSAlloc(int N=0)		{ if (N) m_objs = new T[N]; else m_objs = 0; }
	iRTVSAlloc(T* P)		{ m_objs = P;}
	~iRTVSAlloc(void)		{ Free();}
	operator T*() 			{ return m_objs; }
	operator void*()		{ return m_objs;}
	bool Bad(void) const	{ return m_objs == 0;}
	void operator=(T* p)	{ Free(); m_objs = p;}
	void Free(void)			{ if (m_objs) delete []m_objs; m_objs = 0;}
private:
	T*	m_objs;
};

#endif    /* iRTVSAlloc_ */


//----------------------------------------------------------------
#ifndef iRTVSPointerGuard_
#define iRTVSPointerGuard_

template <class T> class iRTVSPointerGuard
{
public:
	iRTVSPointerGuard(T* obj=0) { p = obj;}
	~iRTVSPointerGuard(void)	{ Clean();}
	void operator=(T* obj)		{ Clean(); p = obj;}
	operator T*() 				{ return p; }
private:
	T*	 p;
	void Clean(void) 
	{
		if (p)
			delete p, p = 0;
	}
};

#endif

//----------------------------------------------------------------
#ifndef iRTVSmallocGuard_
#define iRTVSmallocGuard_
/*
 * Utilities to guard a malloc'ed piece of memory
 *  char *a = malloc(100);
 * iRTVSmallocGuard<char*> guard = a;
 * or
 * iRTVSmallocGuard<char> guard(100);
 *  a  = guard;
 */

template <class T> class iRTVSmallocGuard
{
 public:
	iRTVSmallocGuard(T& iP)		{ m_mem = iP;}
	iRTVSmallocGuard(int N)		{ m_mem = T(malloc(N*sizeof *T));}
	~iRTVSmallocGuard(void)		{ Free(); }
	void Free(void)				{ if (m_mem) free(m_mem); m_mem = 0;}
	void operator=(const T& A)	{ m_mem = A;}
	operator T()				{ return m_mem;}
 private:
	   T m_mem;
};
#endif    /* iRTVSmallocGuard */

//------------------------------------------------------------------
// same as strncpy except it terminates the string

#ifndef nvrstrncpy_
#define nvrstrncpy_

#include <string.h>
inline char* nvrstrncpy(char *dest, const char* src, unsigned count)
{
	*(strncpy(dest, src, count-1) + count - 1) = '\0';
	return dest;
}

#endif /* nvrstrncpy_ */

//------------------------------------------------------------------
// clamp 
#ifndef iRTVSClamp_
#define iRTVSClamp_

template <class T> inline T iRTVSClamp(T A, T vmin, T vmax)
{
	if ( A < vmin ) 	A = vmin;
	else if (A > vmax) 	A = vmax;
	return A;
}

#endif /* iRTVSClamp_ */

//------------------------------------------------------------------
#include <ctype.h>
// the crt is not good for non-asciis
//------------------------------------------------------------------
// T.C. Zhao 2005.10.11
// remove trailing spaces - use this to replace DeSpaceDe for
// DICOM tag processing. In theory, this is not exactly right for multi-byte
// characters, but this is how DB stores stuff.

//--------------------------------------------------------------------
static inline char *iRTVTrimRSpace(char *s)
{
	char * p = s + strlen(s) -1;
	
	for (; p >= s && iswspace(*p); --p )
		;
	*++p = '\0';
	return s;
}

//--------------------------------------------------------------------
static inline char* iRTVTrimLSpace(char *s,int replaceInPlace=1)
{
	char  *p = s;
	for ( ;p && iswspace(*p); ++p)
		;
	return (replaceInPlace&& p!=s) ? strcpy(s,p):p;
}

//--------------------------------------------------------------------
// remove white spaces from both ends of a string

static inline char* iRTVDeSpaceDe(char *s, int replaceInPlace=1)
{
	char * p = s + strlen(s) -1, *sh = s;
	
	for (; p >= s && iswspace(*p); --p )
		;
	*++p = '\0';
	
	for ( ; s < p && iswspace(*s); ++s)
		;
	return replaceInPlace ? strcpy(sh,s):s;
}


//------------------------------------------------------------------

#ifndef iRTVSSwap_
template <class T> inline void iRTVSSwap(T& A, T&B)
{
	T C = A;
	A = B;
	B = C;
}

#endif // !def iRTVSSwap_


template <class T> inline int iRTVSRound(T A)
{
	return int((A > 0.0f ? (A+0.5f):(A-0.5f)));
}

template <class T> inline T iRTVSMin(T A, T B)
{
	return A < B ? A:B;
}

template <class T> inline T iRTVSMax(T A, T B)
{
	return A > B ? A:B;
}

#endif
