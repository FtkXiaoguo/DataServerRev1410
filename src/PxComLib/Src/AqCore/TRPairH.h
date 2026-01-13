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
// Filename:	TRPairH.h
// Author:		David Guigonis
// Created:		Wednesday, May 10, 2006 at 1:44:23 PM
//

#ifndef _TRPAIRH_H_
#define _TRPAIRH_H_
//////////////////////////////////////////////////////////////////////////
#include <cassert>

//
// Represent a pair of homogeneous class
// Equivalent to std::pair<T,T>
//

template<class T>
struct TRPairH
{
	typedef T base_type;
    T first;
    T second;

    TRPairH() {}
    TRPairH(const TRPairH & p)          : first(p.first), second(p.second) {}
    TRPairH(const T& el0, const T& el1) : first(el0), second(el1)          {}
    explicit TRPairH(const T& el)       : first(el), second(el)            {}

    ~TRPairH() {}

    TRPairH&  operator = (const TRPairH& p);

    bool      operator == (const TRPairH& p) const;

    T &       operator [] (int i);
    const T & operator [] (int i) const;
};

//////////////////////////////////////////////////////////////////////////
template<class T>
inline TRPairH<T>& TRPairH<T>::operator = (const TRPairH& p)
{
	first = p.first;
	second = p.second;
	return *this;
}

template<class T>
inline bool TRPairH<T>::operator == (const TRPairH& p) const
{
	return first == p.first && second == p.second;
}

template<class T>
inline T & TRPairH<T>::operator [] (int i)
{
    assert(i == 0 || i == 1);
    return i == 0 ? first : second;
}

template<class T>
inline const T & TRPairH<T>::operator [] (int i) const
{
    assert(i == 0 || i == 1);
    return i == 0 ? first : second;
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _TRPAIRH_H_
