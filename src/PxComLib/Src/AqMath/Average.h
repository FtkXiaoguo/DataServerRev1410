// -*- C++ -*-
// Copyright 2006 TeraRecon, Inc.
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
// Filename:	Average.h
// Author:		David Guigonis
// Created:		Wednesday, August 15, 2006 at 10:48:26 AM
//

#ifndef _AVERAGET_H_
#define _AVERAGET_H_
///////////////////////////////////////////////////////////////////////////////
#include <utility>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
// Overview
//------------------------------------
//	class AverageT calculate the average of a set of value.
//	class StdDevT calculate the standard deviation of a set of value.
//	class MinMaxT calculate the min and max of a set of value.
//------------------------------------
//
///////////////////////////////////////////////////////////////////////////////

/*  Example -------------------------
#include "Average.h"

int main(int argc, char* argv[])
{
	int v[] = {1, 4, 8, 10, 14, 7};
	const int size = sizeof(v) / sizeof(int);

	StdDevT<int> avg;
	for(int i=0; i<size; ++i) {
		avg.Update( v[i] );
	}

	printf("Number of values: %d\n", avg.Num() ); 
	printf("Average: %d\n", avg.Value() ); 
	printf("Standard Deviation: %d\n", avg.Deviation() ); 

	return 0;
}
*/
//------------------------------------

///////////////////////////////////////////////////////////////////////////////
template <class T>
class AverageT
{
public:
    AverageT() { m_count = 0; }
    AverageT(const T& a) : m_sum(a), m_count(1) {}
    AverageT(const T& a,int num) : m_sum(a), m_count(num) {}
	
    void Update(const T& iValue);

    AverageT& operator & (const T& iValue);
    AverageT& operator & (const AverageT& iValue);

    T          Value() const;
    int        Num() const     { return m_count; }
    T&         Sum()           { return m_sum; }
    const T&   Sum() const     { return m_sum; }
    int        Reset ()        { return m_count = 0; }

private:
    T m_sum;
    int m_count;
};

///////////////////////////////////////////////////////////////////////////////
template <class T>
class StdDevT : public AverageT<T>
{
    typedef AverageT<T> Base_;
public:

	StdDevT() : Base_() {}
    StdDevT(const T& a) : Base_(a), m_squareVal(sq_Val(a)) {}
    StdDevT(const T& a, int num, const T& sqv) : Base_(a,num), m_squareVal(sqv) {}

    void     Update(const T& a);

    StdDevT& operator & (const T& iValue);
    StdDevT& operator & (const StdDevT& iValue);

    T        Deviation() const;
    T&       SumSquares()        { return m_squareVal; }
    int      Reset()             { return Base_::Reset(); }
		
private:
    T m_squareVal;
};

///////////////////////////////////////////////////////////////////////////////

template <class T>
class MinMaxT : private std::pair<T,T>
{
public:
    MinMaxT(const T& s, const T& b) : std::pair<T,T>(s,b) {}
    
	void     Update(const T& iValue);

    const T& MinV() const { return first; }
    const T& MaxV() const { return second; }

    void     Reset(const T& a, const T& b);

    T        Diff() const;

    MinMaxT& operator & (const T& iValue);
};

///////////////////////////////////////////////////////////////////////////////
template <class T>
T sq_Val(const T& a) { return a*a; }

template <class T>
T sqrt_Val(const T& a) { return sqrt(a); }

///////////////////////////////////////////////////////////////////////////////
typedef AverageT<double> AverageD;
typedef StdDevT<double>  StdDevD;
typedef MinMaxT<double>  MinMaxD;

typedef AverageT<float> AverageF;
typedef StdDevT<float>  StdDevF;
typedef MinMaxT<float>  MinMaxF;

typedef AverageT<int> AverageI;
typedef StdDevT<int>  StdDevI;
typedef MinMaxT<int>  MinMaxI;

///////////////////////////////////////////////////////////////////////////////
// AverageT implementation
template<class T>
void AverageT<T>::Update(const T& a)
{
	if( m_count ) {
		++m_count;
		m_sum += a;
	}
	else {
		m_count = 1;
		m_sum = a;
	}
}

template<class T>
AverageT<T>& AverageT<T>::operator & (const T& a)
{
	Update(a);
	return *this;
}

template<class T>
AverageT<T>& AverageT<T>::operator & (const AverageT& a)
{
	if( !m_count ) {
		*this = a;
		return *this;
	}
	
	if( !a.m_count ) {
		return *this;
	}
	
	m_sum += a.m_sum;
	m_count += a.m_count;
	return *this;
}

template<class T>
T AverageT<T>::Value() const
{
	if( ! m_count )
		return m_sum;
	
	return m_sum/double(m_count);
}

///////////////////////////////////////////////////////////////////////////////
// StdDevT implementation
template<class T>
void StdDevT<T>::Update(const T& a)
{
	Num() ? m_squareVal += sq_Val(a) : m_squareVal = sq_Val(a) ; //proper initialization, can't initialize it by 0
	Base_::Update( a );
}

template<class T>
StdDevT<T>& StdDevT<T>::operator & (const T& a)
{
	Update(a);
	return *this;
}

template<class T>
StdDevT<T>& StdDevT<T>::operator & (const StdDevT& a)
{
	if( !a.Num() ) {
		return *this;
	}
	
	Num() ? m_squareVal += a.m_squareVal : m_squareVal = a.m_squareVal;
	
	((Base_*)this)->operator & ( a );

	return *this;
}

template<class T>
T StdDevT<T>::Deviation() const
{
	if( !Num() ) {
		assert(0);// check num first
		return m_squareVal;
	}
	
	return sqrt_Val(m_squareVal / Num() - sq_Val(Value()));
}

///////////////////////////////////////////////////////////////////////////////
// MinMaxT implementation
template<class T>
void MinMaxT<T>::Update(const T& a)
{
	if( a < first ) {
		first = a;
	}
	
	if( a > second ) {
		second = a;
	}
}

template<class T>
void MinMaxT<T>::Reset(const T& a, const T& b)
{
	first = a;
	second = b;
}

template<class T>
MinMaxT<T>& MinMaxT<T>::operator & (const T& a)
{
	Update(a);
	return *this;
}

template<class T>
T MinMaxT<T>::Diff() const
{
	return second - first;
}

///////////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AVERAGET_H_
