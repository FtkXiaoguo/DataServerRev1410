// -*- C++ -*-
// Copyright 2006 PreXion 
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF TERARECON, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART,
// IS STRICTLY PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN
// PERMISSION OF TERARECON, INC.
//
// Filename:	TRRange.h
// Author:		David Guigonis
// Created:		Wednesday, May 10, 2006 at 1:42:20 PM
//

#ifndef _TRRANGE_H_
#define _TRRANGE_H_
//////////////////////////////////////////////////////////////////////////
#include "TRPairH.h"
#include <limits>

//
// Represent a range of value, border included: i.e. [10..52]
//
// Exemples:
//  [10..52] & [5..20] = [10..20]
//  [10..52] | [5..20] = [ 5..52] and an empty range
//  [10..52] ^ [5..20] = [ 5.. 9] and [21..52]
//

// see below why use "T* iv", and not "T iv"

template<typename T, T* iv>
class TRRange : public TRPairH<T>
{
public:
	static const T invalid_value;

	TRRange()                { clear(); }
	TRRange(T b, T e)        : TRPairH<T>(b,e) { if( begin() > end() ) std::swap(begin(), end()); }

	bool empty() const       { return ( first == invalid_value && second == invalid_value ); }
	void clear()             { begin() = end() = invalid_value; }

	T& begin()               { return first; }
	T& end()                 { return second; }

	const T& begin() const   { return first; }
	const T& end() const     { return second; }

	T length() const;

	bool includes(const TRRange& s) const;

	// contract the range
	void contract(const T& iter);
	TRRange& operator -= (const T& iter);

	// expand the range
	void expand(const T& iter);
	TRRange& operator += (const T& iter);

	// intersection of ranges
	TRRange operator & (const TRRange& s) const;

	// union of ranges
	TRPairH<TRRange> operator | (const TRRange& s) const;

	// xor of ranges
	TRPairH<TRRange> operator ^ (const TRRange& s) const;
};

//////////////////////////////////////////////////////////////////////////
short          TRRange_Max_Short    = SHRT_MAX;
unsigned short TRRange_Max_UShort   = USHRT_MAX;
int            TRRange_Max_Int      = INT_MAX;
unsigned int   TRRange_Max_UInt     = UINT_MAX;

typedef TRRange<short,          &TRRange_Max_Short>   RangeS;
typedef TRRange<unsigned short, &TRRange_Max_UShort>  RangeUS;
typedef TRRange<int,            &TRRange_Max_Int>     RangeI;
typedef TRRange<unsigned int,   &TRRange_Max_UInt>    RangeUI;


// be careful of the meaning of length() with float/double
// might need to specialize it to get a meaningful result

//float          TRRange_Max_Float  = FLT_MAX;
//double         TRRange_Max_Double = DBL_MAX;
//typedef TRRange<float,          &TRRange_Max_Float>  RangeF;
//typedef TRRange<double,         &TRRange_Max_Double> RangeD;


//////////////////////////////////////////////////////////////////////////
// Yet Another Visual C++ Bug
// Try to see what happens when you declare this (as it should be)
//
//      typedef TRRange<float, FLT_MAX> RangeF;
//
// or, if you are in a hurry, go to:
//      http://support.microsoft.com/default.aspx?scid=kb;en-us;Q263601
//

template<typename T, T* iv>
const T TRRange<T,iv>::invalid_value = *iv;

// intersection of sections
template<typename T, T* iv> inline TRRange<T,iv>
TRRange<T,iv>::operator & (const TRRange& s) const
{
	TRRange overlap;

	if( empty() || s.empty() )
		return overlap;

	overlap.begin() = max(begin(), s.begin());
	overlap.end()   = min(end(), s.end());

	if( overlap.begin() > overlap.end() )
		overlap.clear();

	return overlap;
}

// union of sections
template<typename T, T* iv> inline TRPairH<TRRange<T,iv> >
TRRange<T,iv>::operator | (const TRRange& s) const
{
	if( (*this & s).empty() )
		return TRPairH<TRRange>(*this, s);

	TRRange overlap;
	overlap.begin() = min(begin(), s.begin());
	overlap.end()   = max(end(), s.end());

	return TRPairH<TRRange>(overlap, TRRange());
}

// xor of sections
template<typename T, T* iv> inline TRPairH<TRRange<T,iv> >
TRRange<T,iv>::operator ^ (const TRRange& s) const
{
	if( (*this & s).empty() )
		return TRPairH<TRRange>(*this, s);

	TRRange s1;
	if( begin() == s.begin() )
		s1.clear();
	else {
		s1.begin() = min(begin(), s.begin());
		s1.end()   = max(begin(), s.begin()) - 1;
	}

	TRRange s2;
	if( end() == s.end() )
		s2.clear();
	else {
		s2.begin() = min(end(), s.end()) + 1;
		s2.end()   = max(end(), s.end());
	}

	return TRPairH<TRRange>(s1,s2);
}

template<typename T, T* iv> inline T
TRRange<T,iv>::length() const
{
	return end() - begin() + T(1);
}

template<typename T, T* iv> inline bool
TRRange<T,iv>::includes(const TRRange& s) const
{
	return ( begin() <= s.begin() && end() >= s.end() );
}


template<typename T, T* iv> inline void
TRRange<T,iv>::contract(const T& iter)
{
	if( empty() )
		return;
	
	begin() += iter;
	end()   -= iter;

	if( begin() > end() )
		clear();
}

template<typename T, T* iv> inline TRRange<T,iv>&
TRRange<T,iv>::operator -= (const T& iter)
{
	contract(iter);
	return *this;
}

template<typename T, T* iv> inline void
TRRange<T,iv>::expand(const T& iter)
{
	if( empty() )
		return;
	
	begin() -= iter;
	end()   += iter;
}

template<typename T, T* iv> inline TRRange<T,iv>&
TRRange<T,iv>::operator += (const T& iter)
{
	expand(iter);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _TRRANGE_H_
