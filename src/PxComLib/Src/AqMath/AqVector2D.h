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
// Filename:	AqVector2D.h
// Author:		David Guigonis
// Created:		Thursday, July 13, 2006 at 2:44:27 PM
//

#ifndef _AQVECTOR2D_H_
#define _AQVECTOR2D_H_
//////////////////////////////////////////////////////////////////////////


template<class T>
class TVector2D
{
public:
	static const TVector2D XAxis;
	static const TVector2D YAxis;
	static const TVector2D Origin;

	// ---- constructors ---- 
	TVector2D(void);						// creates a zero length vector
	TVector2D(const TVector2D& inVec);
	TVector2D(const T& inX, const T& inY);
	TVector2D(const T& inXY);               // initialized x, y with the same value inXY
	TVector2D(const T [2]);
	
	// ---- Setting the value ---- 
	TVector2D& Assign(const T& x, const T& y);
	TVector2D& Assign(const T& xy);
	TVector2D& Assign(const T [2]);
	TVector2D& operator = (const TVector2D& inVec);
	
	// ---- Access, either read or write ---- 
	T& X(void)              { return m_v[0];}
	T& Y(void)              { return m_v[1];} 
	
	const T& X(void) const  { return m_v[0];}
	const T& Y(void) const  { return m_v[1];} 
	
	T&       operator [](int);
	const T& operator [](int) const;
	
	// ---- Reading out the value ---- 
	void  CopyTo(T* outX, T* outY) const;
	void  CopyTo(T outArray[2]) const;
	
	// ---- math operators ---- 
	TVector2D& operator +=(const TVector2D&);
	TVector2D& operator -=(const TVector2D&);
	TVector2D& operator *=(const T&);
	TVector2D& operator /=(const T&);
	TVector2D  operator +(const TVector2D&) const;
	TVector2D  operator -(const TVector2D&) const;
	TVector2D  operator -(void) const;
	TVector2D  operator *(const T&) const;
	TVector2D  operator /(const T&) const;
	friend TVector2D operator *(const double&, const TVector2D&);
	
	void Scale (const T& is);                              // *this *= is
	void Add   (const TVector2D& p);                       // *this += p
	void Sub   (const TVector2D& p);                       // *this -= p
	void Sum   (const TVector2D& p1, const TVector2D& p2); // *this =  p1 + p2
	void Diff  (const TVector2D& p1, const TVector2D& p2); // *this =  p1 - p2

	int  operator ==(const TVector2D&) const;
	int  operator !=(const TVector2D&) const;
	
	// ---- operations ---- 
	double Length(void) const;
	double LengthSquared(void) const;

	TVector2D& Normalize(const T& to = T(1));
	TVector2D Normalized(const T& to = T(1)) const;
	
	// dot product
    double   operator & (const TVector2D& inV) const;
	double Dot(const TVector2D& inV) const;

	friend double VLIDot(const TVector2D& inA, const TVector2D& inB);
	friend double VLIDistance(const TVector2D& inA, const TVector2D& inB);
	friend double VLIDistanceSquared(const TVector2D& inA, const TVector2D& inB);
	
protected:
	T m_v[2];
};

typedef TVector2D<double> AqVector2d;
typedef TVector2D<float>  AqVector2f;
typedef TVector2D<int>    AqVector2i;
typedef TVector2D<short>  AqVector2s;

typedef TVector2D<double> AqPoint2d;
typedef TVector2D<float>  AqPoint2f;
typedef TVector2D<int>    AqPoint2i;
typedef TVector2D<short>  AqPoint2s;

//////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTORS
//

template<class T> const TVector2D<T> TVector2D<T>::XAxis  = TVector2D<T>(T(1), T(0));
template<class T> const TVector2D<T> TVector2D<T>::YAxis  = TVector2D<T>(T(0), T(1));
template<class T> const TVector2D<T> TVector2D<T>::Origin = TVector2D<T>(T(0), T(0));

template<class T> inline
TVector2D<T>::TVector2D()
{
	m_v[0] = m_v[1] = T(0);
}

template<class T> inline
TVector2D<T>::TVector2D(const TVector2D<T>& inVec)
{
	m_v[0] = inVec.m_v[0]; m_v[1] = inVec.m_v[1];
}

template<class T> inline
TVector2D<T>::TVector2D(const T& inX, const T& inY)
{
	m_v[0] = inX; m_v[1] = inY;
}

template<class T> inline
TVector2D<T>::TVector2D(const T& inXY)
{
	m_v[0] = m_v[1] = inXY;
}

template<class T> inline
TVector2D<T>::TVector2D(const T inArr[2])
{
	m_v[0] = inArr[0]; m_v[1] = inArr[1];
}

//////////////////////////////////////////////////////////////////////////
//
// ARRAY ACCESS
//
template<class T> inline
T& TVector2D<T>::operator [](int i) 
{
	assert(i>= 0 && i <= 1);
	return m_v[i];
}

template<class T> inline
const T& TVector2D<T>::operator [](int i) const
{
	assert(i>= 0 && i <= 1);
	return m_v[i];
}

//////////////////////////////////////////////////////////////////////////
//
// ASSIGNMENT
//
template<class T> inline
TVector2D<T>& TVector2D<T>::Assign(const T& x, const T& y)
{
	m_v[0] = x; m_v[1] = y;
	return *this;
}

template<class T> inline
TVector2D<T>& TVector2D<T>::Assign(const T& xy)
{
	m_v[0] = m_v[1] = xy;
	return *this;
}

template<class T> inline
TVector2D<T>& TVector2D<T>::Assign(const T v[2])
{
	m_v[0] = v[0]; m_v[1] = v[1];
	return *this;
}

template<class T> inline
TVector2D<T>& TVector2D<T>::operator =(const TVector2D<T>& v)
{
	m_v[0] = v[0]; m_v[1] = v[1];
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//
// MATH OPERATORS
//
template<class T> inline
TVector2D<T>& TVector2D<T>::operator +=(const TVector2D<T>& v)
{
	m_v[0] += v[0]; m_v[1] += v[1];
	return *this;
}

template<class T> inline
TVector2D<T>& TVector2D<T>::operator -=(const TVector2D<T>& v)
{
	m_v[0] -= v[0]; m_v[1] -= v[1];
	return *this;
}

template<class T> inline
TVector2D<T>& TVector2D<T>::operator *=(const T& c)
{
	m_v[0] *= c; m_v[1] *= c;
	return *this;
}

template<class T> inline
TVector2D<T>& TVector2D<T>::operator /=(const T& c)
{
	if(AqMathCore::IsZero(c)) {
		//math_error(!RTVIsZero(c));
	} 
	else 
	{
		double recip = 1.0 / c;
		//m_v[0] *= recip; m_v[1] *= recip;
 		m_v[0] = T(recip * m_v[0]);
 		m_v[1] = T(recip * m_v[1]);
	}
	return *this;
}

template<class T> inline
TVector2D<T> TVector2D<T>::operator +(const TVector2D<T>& v) const
{
	return TVector2D(m_v[0] + v[0], m_v[1] + v[1]);
}

template<class T> inline
TVector2D<T> TVector2D<T>::operator -(const TVector2D<T>& v) const
{
	return TVector2D(m_v[0] - v[0], m_v[1] - v[1]);
}

template<class T> inline
TVector2D<T> TVector2D<T>::operator -() const
{
	return TVector2D(-m_v[0], -m_v[1]);
}

template<class T> inline
TVector2D<T> TVector2D<T>::operator *(const T& c) const
{
	return TVector2D(m_v[0] * c, m_v[1] * c);
}

template<class T> inline
TVector2D<T> TVector2D<T>::operator /(const T& c) const
{
	if(AqMathCore::IsZero(c)) 
	{
		//    	math_error(!RTVIsZero(c));
		return TVector2D(m_v[0], m_v[1]);
	} 
	else 
	{
		double recip = 1.0/c;
		return TVector2D(T(m_v[0] * recip), T(m_v[1] * recip));
	}
}

template<class T> inline
TVector2D<T> operator *(const double& c, const TVector2D<T>& v)
{
	return TVector2D<T>(T(c * v[0]), T(c * v[1]));
}

template<class T> inline
void TVector2D<T>::Scale(const T& is)          
{
    m_v[0] *= is;
    m_v[1] *= is;
}

template<class T> inline
void TVector2D<T>::Add(const TVector2D<T>& p)
{
	m_v[0] += p.m_v[0];
	m_v[1] += p.m_v[1];
}

template<class T> inline
void TVector2D<T>::Sub(const TVector2D<T>& p)
{
	m_v[0] -= p.m_v[0];
	m_v[1] -= p.m_v[1];
}

template<class T> inline
void TVector2D<T>::Sum(const TVector2D<T>& p1, const TVector2D<T>& p2)
{
	m_v[0] = p1.m_v[0] + p2.m_v[0];
	m_v[1] = p1.m_v[1] + p2.m_v[1];
}

template<class T> inline
void TVector2D<T>::Diff(const TVector2D<T>& p1, const TVector2D<T>& p2)
{
	m_v[0] = p1.m_v[0] - p2.m_v[0];
	m_v[1] = p1.m_v[1] - p2.m_v[1];
}

template<class T> inline
int  TVector2D<T>::operator ==(const TVector2D<T>& v) const
{
	return ((m_v[0] == v[0]) && (m_v[1] == v[1]));
}

template<class T> inline
int TVector2D<T>::operator !=(const TVector2D<T>& v) const
{
	return (!(*this == v));
}

//////////////////////////////////////////////////////////////////////////
//
// OPERATIONS
//
template<class T> inline
double TVector2D<T>::Length() const
{
	return AqMathCore::Sqrt(m_v[0]*m_v[0] + m_v[1]*m_v[1]);
}

template<class T> inline
double TVector2D<T>::LengthSquared() const
{
	return (m_v[0]*m_v[0] + m_v[1]*m_v[1]);
}

template<class T> inline
TVector2D<T>& TVector2D<T>::Normalize(const T& to)
{
	if (AqMathCore::IsZero(m_v[0]) && AqMathCore::IsZero(m_v[1])) return *this;
	
	*this *= T(to / Length());
	
	return *this;
}

template<class T> inline
TVector2D<T> TVector2D<T>::Normalized(const T& to) const
{
	if (AqMathCore::IsZero(m_v[0]) && AqMathCore::IsZero(m_v[1])) return *this;
	
	return (*this) * T(to / Length());
}

template<class T> inline
void TVector2D<T>::CopyTo(T *x, T *y) const
{
	*x = m_v[0]; 
	*y = m_v[1]; 
}

template<class T> inline
void TVector2D<T>::CopyTo(T f[2]) const
{
	f[0] = m_v[0]; f[1] = m_v[1];
}

template<class T> inline
double TVector2D<T>::operator &(const TVector2D<T>& v2) const
{
	return m_v[0]*v2.m_v[0] +
		m_v[1]*v2.m_v[1];
}

template<class T> inline
double TVector2D<T>::Dot(const TVector2D<T>& v2)  const
{
	return m_v[0]*v2.m_v[0] +
		m_v[1]*v2.m_v[1];
}

template<class T> inline
double VLIDot(const TVector2D<T>& v1, const TVector2D<T>& v2)
{
	return v1.m_v[0] * v2.m_v[0] + v1.m_v[1] * v2.m_v[1];
}

template<class T> inline
double VLIDistance(const TVector2D<T>& v1, const TVector2D<T>& v2)
{
	return AqMathCore::Sqrt(AqMathCore::Square(v1.m_v[0] - v2.m_v[0]) +
							AqMathCore::Square(v1.m_v[1] - v2.m_v[1]));
}

template<class T> inline
double VLIDistanceSquared(const TVector2D<T>& v1, const TVector2D<T>& v2)
{
	return AqMathCore::Square(v1.m_v[0] - v2.m_v[0]) +
		   AqMathCore::Square(v1.m_v[1] - v2.m_v[1]));
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQVECTOR2D_H_
