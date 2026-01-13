/*************************************************************************
 *  TRAtomic.h
 *
 *---------------------------------------------------------------------
 *   
 *
 */

#ifndef TRATOMIC_H_
#define TRATOMIC_H_

#include "TRCriticalsection.h"


//-----------------------------------------------------------------------------

template <class C> class TRAtomicVar
{
public:
	TRAtomicVar(C iVal=0) {m_value = iVal;}
	C	 operator ++ (void)				{TRCSLock L(&m_cs); return ++m_value;}
	C	 operator ++ (int)				{TRCSLock L(&m_cs); return m_value++;}
	C	 operator -- (void)				{TRCSLock L(&m_cs); return --m_value;}
	C	 operator -- (int)				{TRCSLock L(&m_cs); return m_value--;}
	C	 operator += (int iV)			{TRCSLock L(&m_cs); m_value += iV; return m_value;}
	C	 operator -= (int iV)			{TRCSLock L(&m_cs); m_value -= iV; return m_value;}
	C	 operator = (const C& iVal)		{TRCSLock L(&m_cs); return m_value = iVal;}
	bool operator < (const C& iVal)		{TRCSLock L(&m_cs); return m_value < iVal;}
	bool operator <= (const C& iVal)	{TRCSLock L(&m_cs); return m_value <= iVal;}
	bool operator > (const C& iVal)		{TRCSLock L(&m_cs); return m_value > iVal;}
	bool operator >= (const C& iVal)	{TRCSLock L(&m_cs); return m_value >= iVal;}
	operator int(void)			  const {TRCSLock L(&m_cs); return int (m_value);}
	operator short(void)          const {TRCSLock L(&m_cs); return short(m_value);}
	operator unsigned int(void)	  const {TRCSLock L(&m_cs); return unsigned int(m_value);}
	operator unsigned short(void) const {TRCSLock L(&m_cs); return unsigned short(m_value);}
	operator bool(void)			  const {TRCSLock L(&m_cs); return int(m_value) != 0;}
	bool operator ==(const TRAtomicVar<C> &b) {TRCSLock L(&m_cs); return m_value == b.m_value;}
private:
	C                  m_value;
	TRCriticalSection  m_cs;
};

#if defined(_WIN32)
#pragma warning (disable: 4661) // no suitable operator  
#endif


template class TRAtomicVar<int>;
template class TRAtomicVar<unsigned int>;

template <typename T> T		operator+(T a, TRAtomicVar<T>& b)	{ return a + T(b);}
template <typename T> T		operator-(T a, TRAtomicVar<T>& b)	{ return a - T(b);}
template <typename T> bool	operator!(TRAtomicVar<T>& a)		{ return !T(a);}
template <typename T> bool	operator==(T a,TRAtomicVar<T>& b)  { return a==T(b);}
template <typename T> bool	operator==(TRAtomicVar<T>& b,T a)  { return a==T(b);}

#endif
	