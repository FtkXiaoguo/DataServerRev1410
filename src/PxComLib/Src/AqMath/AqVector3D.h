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
// Filename:	AqVector3D.h
// Author:		David Guigonis
// Created:		Wednesday, July 12, 2006 at 1:47:18 PM
//

#ifndef _AQVECTOR3D_H_
#define _AQVECTOR3D_H_
//////////////////////////////////////////////////////////////////////////
#include "AqMathCore.h"
#include <assert.h>


#pragma warning( disable : 4244 )	// truncation from 'double' to 'float'
#pragma warning( disable : 4786 )	// truncation to 255 char in debug mode

template<class T>
class TVector3D
{
public:
	static const TVector3D XAxis;
	static const TVector3D YAxis;
	static const TVector3D ZAxis;
	static const TVector3D Origin;

	// ---- constructors ---- 
	TVector3D(void);						// creates a zero length vector
	TVector3D(const TVector3D& inVec);
	TVector3D(const T& inX, const T& inY, const T& inZ);
	TVector3D(const T& inXYZ);              // initialized x, y, z with the same value inXYZ
	TVector3D(const T [3]);
	
	// ---- Setting the value ---- 
	TVector3D& Assign(const T& x, const T& y, const T& z);
	TVector3D& Assign(const T& xyz);
	TVector3D& Assign(const T [3]);
	TVector3D& operator = (const TVector3D& inVec);
	
	// ---- Access, either read or write ---- 
	T& X(void)              { return m_v[0];}
	T& Y(void)              { return m_v[1];} 
	T& Z(void)              { return m_v[2];} 
	
	const T& X(void) const  { return m_v[0];}
	const T& Y(void) const  { return m_v[1];} 
	const T& Z(void) const  { return m_v[2];} 
	
	T&       operator [](int);
	const T& operator [](int) const;
	
	// ---- Reading out the value ---- 
	void  CopyTo(T* outX, T* outY, T* outZ) const;
	void  CopyTo(T outArray[3]) const;
	
	// ---- math operators ---- 
	TVector3D& operator +=(const TVector3D&);
	TVector3D& operator -=(const TVector3D&);
	TVector3D& operator *=(const T&);
	TVector3D& operator /=(const T&);
	TVector3D  operator +(const TVector3D&) const;
	TVector3D  operator -(const TVector3D&) const;
	TVector3D  operator -(void) const;
	TVector3D  operator *(const T&) const;
	TVector3D  operator /(const T&) const;
	friend TVector3D operator *(const double&, const TVector3D&);
	
	void Scale (const T& is);                              // *this *= is
	void Add   (const TVector3D& p);                       // *this += p
	void Sub   (const TVector3D& p);                       // *this -= p
	void Sum   (const TVector3D& p1, const TVector3D& p2); // *this =  p1 + p2
	void Diff  (const TVector3D& p1, const TVector3D& p2); // *this =  p1 - p2

	int  operator ==(const TVector3D&) const;
	int  operator !=(const TVector3D&) const;
	
	// ---- operations ---- 
	double Length(void) const;
	double LengthSquared(void) const;

	TVector3D& Normalize(const T& to = T(1));
	TVector3D Normalized(const T& to = T(1)) const;
	
	// dot product
    double   operator & (const TVector3D& inV) const;
	double Dot(const TVector3D& inV) const;

	// cross product
	TVector3D operator * (const TVector3D& inV) const;
	TVector3D Cross(const TVector3D& inV) const;

	friend double VLIDot(const TVector3D& inA, const TVector3D& inB);
	friend TVector3D VLICross(const TVector3D& inA, const TVector3D& inB);
	friend double VLIDistance(const TVector3D& inA, const TVector3D& inB);
	friend double VLIDistanceSquared(const TVector3D& inA, const TVector3D& inB);
	
protected:
	T m_v[3];
};

typedef TVector3D<double> AqVector3d;
typedef TVector3D<float>  AqVector3f;
typedef TVector3D<int>    AqVector3i;
typedef TVector3D<short>  AqVector3s;

typedef TVector3D<double> AqPoint3d;
typedef TVector3D<float>  AqPoint3f;
typedef TVector3D<int>    AqPoint3i;
typedef TVector3D<short>  AqPoint3s;

//////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTORS
//

template<class T> const TVector3D<T> TVector3D<T>::XAxis  = TVector3D<T>(T(1), T(0), T(0));
template<class T> const TVector3D<T> TVector3D<T>::YAxis  = TVector3D<T>(T(0), T(1), T(0));
template<class T> const TVector3D<T> TVector3D<T>::ZAxis  = TVector3D<T>(T(0), T(0), T(1));
template<class T> const TVector3D<T> TVector3D<T>::Origin = TVector3D<T>(T(0), T(0), T(0));

template<class T> inline
TVector3D<T>::TVector3D()
{
	m_v[0] = m_v[1] = m_v[2] = T(0);
}

template<class T> inline
TVector3D<T>::TVector3D(const TVector3D<T>& inVec)
{
	m_v[0] = inVec.m_v[0]; m_v[1] = inVec.m_v[1]; m_v[2] = inVec.m_v[2];
}

template<class T> inline
TVector3D<T>::TVector3D(const T& inX, const T& inY, const T& inZ)
{
	m_v[0] = inX; m_v[1] = inY; m_v[2] = inZ;
}

template<class T> inline
TVector3D<T>::TVector3D(const T& inXYZ)
{
	m_v[0] = m_v[1] = m_v[2] = inXYZ;
}

template<class T> inline
TVector3D<T>::TVector3D(const T inArr[3])
{
	m_v[0] = inArr[0]; m_v[1] = inArr[1]; m_v[2] = inArr[2];
}

//////////////////////////////////////////////////////////////////////////
//
// ARRAY ACCESS
//
template<class T> inline
T& TVector3D<T>::operator [](int i) 
{
	assert(i>= 0 && i <= 2);	
	return m_v[i];
}

template<class T> inline
const T& TVector3D<T>::operator [](int i) const
{
	assert(i>= 0 && i <= 2);	
	return m_v[i];
}

//////////////////////////////////////////////////////////////////////////
//
// ASSIGNMENT
//
template<class T> inline
TVector3D<T>& TVector3D<T>::Assign(const T& x, const T& y, const T& z)
{
	m_v[0] = x; m_v[1] = y; m_v[2] = z;
	return *this;
}

template<class T> inline
TVector3D<T>& TVector3D<T>::Assign(const T& xyz)
{
	m_v[0] = m_v[1] = m_v[2] = xyz;
	return *this;
}

template<class T> inline
TVector3D<T>& TVector3D<T>::Assign(const T v[3])
{
	m_v[0] = v[0]; m_v[1] = v[1]; m_v[2] = v[2];
	return *this;
}

template<class T> inline
TVector3D<T>& TVector3D<T>::operator =(const TVector3D<T>& v)
{
	m_v[0] = v[0]; m_v[1] = v[1]; m_v[2] = v[2];
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//
// MATH OPERATORS
//
template<class T> inline
TVector3D<T>& TVector3D<T>::operator +=(const TVector3D<T>& v)
{
	m_v[0] += v[0]; m_v[1] += v[1]; m_v[2] += v[2];
	return *this;
}

template<class T> inline
TVector3D<T>& TVector3D<T>::operator -=(const TVector3D<T>& v)
{
	m_v[0] -= v[0]; m_v[1] -= v[1]; m_v[2] -= v[2];
	return *this;
}

template<class T> inline
TVector3D<T>& TVector3D<T>::operator *=(const T& c)
{
	m_v[0] *= c; m_v[1] *= c; m_v[2] *= c;
	return *this;
}

template<class T> inline
TVector3D<T>& TVector3D<T>::operator /=(const T& c)
{
	if(AqMathCore::IsZero(c)) {
		//math_error(!RTVIsZero(c));
	} 
	else 
	{
		double recip = 1.0 / c;
		//m_v[0] *= recip; m_v[1] *= recip; m_v[2] *= recip;
		m_v[0] = T(recip * m_v[0]);
		m_v[1] = T(recip * m_v[1]);
		m_v[2] = T(recip * m_v[2]);
	}
	return *this;
}

template<class T> inline
TVector3D<T> TVector3D<T>::operator +(const TVector3D<T>& v) const
{
	return TVector3D(m_v[0] + v[0], m_v[1] + v[1], m_v[2] + v[2]);
}

template<class T> inline
TVector3D<T> TVector3D<T>::operator -(const TVector3D<T>& v) const
{
	return TVector3D(m_v[0] - v[0], m_v[1] - v[1], m_v[2] - v[2]);
}

template<class T> inline
TVector3D<T> TVector3D<T>::operator -() const
{
	return TVector3D(-m_v[0], -m_v[1], -m_v[2]);
}

template<class T> inline
TVector3D<T> TVector3D<T>::operator *(const T& c) const
{
	return TVector3D(m_v[0] * c, m_v[1] * c, m_v[2] * c);
}

template<class T> inline
TVector3D<T> TVector3D<T>::operator /(const T& c) const
{
	if(AqMathCore::IsZero(c)) 
	{
		//    	math_error(!RTVIsZero(c));
		return TVector3D(m_v[0], m_v[1], m_v[2]);
	} 
	else 
	{
		double recip = 1.0/c;
		return TVector3D(T(m_v[0] * recip), T(m_v[1] * recip), T(m_v[2] * recip));
	}
}

template<class T> inline
TVector3D<T> operator *(const double& c, const TVector3D<T>& v)
{
	return TVector3D<T>(T(c * v[0]), T(c * v[1]), T(c * v[2]));
}

template<class T> inline
void TVector3D<T>::Scale(const T& is)          
{
    m_v[0] *= is;
    m_v[1] *= is;
    m_v[2] *= is;
}

template<class T> inline
void TVector3D<T>::Add(const TVector3D<T>& p)
{
	m_v[0] += p.m_v[0];
	m_v[1] += p.m_v[1];
	m_v[2] += p.m_v[2];
}

template<class T> inline
void TVector3D<T>::Sub(const TVector3D<T>& p)
{
	m_v[0] -= p.m_v[0];
	m_v[1] -= p.m_v[1];
	m_v[2] -= p.m_v[2];
}

template<class T> inline
void TVector3D<T>::Sum(const TVector3D<T>& p1, const TVector3D<T>& p2)
{
	m_v[0] = p1.m_v[0] + p2.m_v[0];
	m_v[1] = p1.m_v[1] + p2.m_v[1];
	m_v[2] = p1.m_v[2] + p2.m_v[2];
}

template<class T> inline
void TVector3D<T>::Diff(const TVector3D<T>& p1, const TVector3D<T>& p2)
{
	m_v[0] = p1.m_v[0] - p2.m_v[0];
	m_v[1] = p1.m_v[1] - p2.m_v[1];
	m_v[2] = p1.m_v[2] - p2.m_v[2];
}

template<class T> inline
int  TVector3D<T>::operator ==(const TVector3D<T>& v) const
{
	return ((m_v[0] == v[0]) && (m_v[1] == v[1]) && (m_v[2] == v[2]));
}

template<class T> inline
int TVector3D<T>::operator !=(const TVector3D<T>& v) const
{
	return (!(*this == v));
}

//////////////////////////////////////////////////////////////////////////
//
// OPERATIONS
//
template<class T> inline
double TVector3D<T>::Length() const
{
	return AqMathCore::Sqrt(m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2]);
}

template<class T> inline
double TVector3D<T>::LengthSquared() const
{
	return (m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2]);
}

template<class T> inline
TVector3D<T>& TVector3D<T>::Normalize(const T& to)
{
	if (AqMathCore::IsZero(m_v[0]) && AqMathCore::IsZero(m_v[1]) && AqMathCore::IsZero (m_v[2])) return *this;
	
	*this *= T(to / Length());
	
	return *this;
}

template<class T> inline
TVector3D<T> TVector3D<T>::Normalized(const T& to) const
{
	if (AqMathCore::IsZero(m_v[0]) && AqMathCore::IsZero(m_v[1]) && AqMathCore::IsZero (m_v[2])) return *this;
	
	return (*this) * T(to / Length());
}

template<class T> inline
void TVector3D<T>::CopyTo(T *x, T *y, T *z) const
{
	*x = m_v[0]; 
	*y = m_v[1]; 
	*z = m_v[2];
}

template<class T> inline
void TVector3D<T>::CopyTo(T f[3]) const
{
	f[0] = m_v[0]; f[1] = m_v[1]; f[2] = m_v[2];
}

template<class T> inline
double TVector3D<T>::operator &(const TVector3D<T>& v2) const
{
	return m_v[0]*v2.m_v[0] +
		m_v[1]*v2.m_v[1] +
		m_v[2]*v2.m_v[2];
}

template<class T> inline
double TVector3D<T>::Dot(const TVector3D<T>& v2)  const
{
	return m_v[0]*v2.m_v[0] +
		m_v[1]*v2.m_v[1] +
		m_v[2]*v2.m_v[2];
}

template<class T> inline
TVector3D<T> TVector3D<T>::operator *(const TVector3D<T>& v2) const
{
	return TVector3D(m_v[1] * v2.m_v[2] - m_v[2] * v2.m_v[1],
		m_v[2] * v2.m_v[0] - m_v[0] * v2.m_v[2],
		m_v[0] * v2.m_v[1] - m_v[1] * v2.m_v[0]);
}

template<class T> inline
TVector3D<T> Cross(const TVector3D<T>& v2)
{
	return TVector3D(m_v[1] * v2.m_v[2] - m_v[2] * v2.m_v[1],
		m_v[2] * v2.m_v[0] - m_v[0] * v2.m_v[2],
		m_v[0] * v2.m_v[1] - m_v[1] * v2.m_v[0]);
}

template<class T> inline
double VLIDot(const TVector3D<T>& v1, const TVector3D<T>& v2)
{
	return v1.m_v[0] * v2.m_v[0] + v1.m_v[1] * v2.m_v[1] + v1.m_v[2] * v2.m_v[2];
}

template<class T> inline
TVector3D<T> VLICross(const TVector3D<T>& v1, const TVector3D<T>& v2)
{
	return TVector3D(v1.m_v[1] * v2.m_v[2] - v1.m_v[2] * v2.m_v[1],
		v1.m_v[2] * v2.m_v[0] - v1.m_v[0] * v2.m_v[2],
		v1.m_v[0] * v2.m_v[1] - v1.m_v[1] * v2.m_v[0]);
}

template<class T> inline
double VLIDistance(const TVector3D<T>& v1, const TVector3D<T>& v2)
{
	return AqMathCore::Sqrt(AqMathCore::Square(v1.m_v[0] - v2.m_v[0]) +
							AqMathCore::Square(v1.m_v[1] - v2.m_v[1]) +
							AqMathCore::Square(v1.m_v[2] - v2.m_v[2]));
}

template<class T> inline
double VLIDistanceSquared(const TVector3D<T>& v1, const TVector3D<T>& v2)
{
	return AqMathCore::Square(v1.m_v[0] - v2.m_v[0]) +
		   AqMathCore::Square(v1.m_v[1] - v2.m_v[1]) +
		   AqMathCore::Square(v1.m_v[2] - v2.m_v[2]);
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQVECTOR3D_H_
