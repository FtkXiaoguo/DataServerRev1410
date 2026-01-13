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
// Filename:	AqDisc3d.h
// Author:		David Guigonis
// Created:		Monday, May 22, 2006 at 1:10:46 PM
//

#ifndef _AQDISC3D_H_
#define _AQDISC3D_H_
//////////////////////////////////////////////////////////////////////////
//#include "AqCore\AqPoint3d.h"
#include "AqMath\AqVector3d.h"

#include "AqMathConst.h"
#include <algorithm>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
template<class T>
class TDisc3D
{
public:
	TDisc3D();
	TDisc3D(const TVector3D<T>& iCenter, const T& iRadius);
	TDisc3D(const T& iXCenter, const T& iYCenter, const T& iZCenter, const T& iRadius);
	virtual ~TDisc3D();

	TVector3D<T>&       center();
	const TVector3D<T>& center() const;

	// radius
	T&       r();
	const T& r() const;
	
	// diameter
	T d() const;

	// perimeter
	double perimeter() const;

	// area
	double area() const;
	
	// comparison operators
	bool      operator == (const TDisc3D& iDisc) const;
	bool      operator != (const TDisc3D& iDisc) const;

protected:
	TVector3D<T> m_center;
	T           m_radius;
};

//////////////////////////////////////////////////////////////////////////
//  Useful function to calculate the
//  area of the intersection on the XY plane
//  The pointer could be NULL if not interested 
//  in the specific value
template<class T>
void IntersectAreaOnXY(const TDisc3D<T>& iDisc1, const TDisc3D<T>& iDisc2,
					   double* oIntersectArea,
					   double* oNormIntersectArea);

//////////////////////////////////////////////////////////////////////////
typedef TDisc3D<short>  Disc3s;
typedef TDisc3D<int>    Disc3i;
typedef TDisc3D<float>  Disc3f;
typedef TDisc3D<double> Disc3d;


//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

template<class T> inline
TDisc3D<T>::TDisc3D()
: m_center(T(0)), m_radius(T(0))
{
}

template<class T> inline
TDisc3D<T>::TDisc3D(const TVector3D<T>& iCenter, const T& iRadius)
: m_center(iCenter), m_radius(iRadius)
{
}

template<class T> inline
TDisc3D<T>::TDisc3D(const T& iXCenter, const T& iYCenter, const T& iZCenter, const T& iRadius)
: m_center(iXCenter,iYCenter,iZCenter), m_radius(iRadius)
{
}

template<class T> inline
TDisc3D<T>::~TDisc3D()
{
}

//////////////////////////////////////////////////////////////////////////
template<class T> inline
TVector3D<T>& TDisc3D<T>::center()
{
	return m_center;
}

template<class T> inline
const TVector3D<T>& TDisc3D<T>::center() const
{
	return m_center;
}

template<class T> inline
T& TDisc3D<T>::r()
{
	return m_radius;
}

template<class T> inline
const T& TDisc3D<T>::r() const
{
	return m_radius;
}

template<class T> inline
T TDisc3D<T>::d() const
{
	return 2 * m_radius;
}

template<class T> inline
double TDisc3D<T>::perimeter() const
{
	return 2 * AqMathConst::PI * m_radius;
}

template<class T> inline
double TDisc3D<T>::area() const
{
	return AqMathConst::PI * m_radius * m_radius;
}

template<class T> inline
bool TDisc3D<T>::operator == (const TDisc3D& disc) const
{
	return ( m_center == disc.m_center && m_radius == disc.m_radius );
}

template<class T> inline
bool TDisc3D<T>::operator != (const TDisc3D& disc) const
{
	return !(*this == disc);
}

//////////////////////////////////////////////////////////////////////////
// Formula took from this web address:
// http://mathworld.wolfram.com/Circle-CircleIntersection.html
//

// area of the intersection on the XY plane
//  the pointer could be NULL is not interested 
//  in the specific value
template<class T> inline
void IntersectAreaOnXY(const TDisc3D<T>& iDisc1, const TDisc3D<T>& iDisc2,
					   double* oIntersectArea,
					   double* oNormIntersectArea)
{
	if( !oIntersectArea && !oNormIntersectArea)
		return; // nothing to compute

	double a_d = iDisc1.d();
	double b_d = iDisc2.d();

	if( a_d <= 0.0 && b_d <= 0.0 ) {
		if( oIntersectArea )     *oIntersectArea = 0.0;
		if( oNormIntersectArea ) *oNormIntersectArea = 0.0;
		return;
	}

	const double a_x = iDisc1.center().X();
	const double a_y = iDisc1.center().Y();
	const double b_x = iDisc2.center().X();
	const double b_y = iDisc2.center().Y();

	const double dx = a_x - b_x;
	const double dy = a_y - b_y;

	double a_area = iDisc1.area();	
	double b_area = iDisc2.area();

	if( dx == 0.0 && dy == 0.0 ) {
		// same center
		if( oIntersectArea )     *oIntersectArea = _MIN(a_area, b_area);
		if( oNormIntersectArea ) *oNormIntersectArea = 1.0;
		return;
	}

	const double d2 = dx*dx + dy*dy;
	const double d = sqrt(d2);

	// check if circles are included into each others
	// is one circle reduce to only center (r == 0)?
	if( a_d <= 0.0 && d < b_d ) {
		if( oIntersectArea )     *oIntersectArea = 0.0;
		if( oNormIntersectArea ) *oNormIntersectArea = 1.0;
		return;
	}
	else if( b_d <= 0.0 && d < a_d ) {
		if( oIntersectArea )     *oIntersectArea = 0.0;
		if( oNormIntersectArea ) *oNormIntersectArea = 1.0;
		return;
	}

	double R, R2, r, r2;

	// ensure that r < R
	if( a_d < b_d ) {
		r = iDisc1.r();   r2 = r * r;
		R = iDisc2.r();   R2 = R * R;
	}
	else {
		R = iDisc1.r();   R2 = R * R;
		r = iDisc2.r();   r2 = r * r;
	}

	double A = 0.0;
	if( d <= (R-r) ) {
		// small disc inside big disc
		A = AqMathConst::PI * r2;
	}
	else if( d >= (r+R) ) {
		// no intersection
		A = 0.0;
	}
	else {
		// intersection
		double val = (d2+r2-R2) / (2*d*r);
		val = _MIN(1.0, _MAX(-1.0, val) );
		
		A = r2*acos( val );
		
		val = (d2+R2-r2) / (2*d*R);
		val = _MIN(1.0, _MAX(-1.0, val) );
		
		A += R2*acos( val );
		
		val = abs( (-d+r+R)*(d+r-R)*(d-r+R)*(d+r+R) );
		A -= 0.5 * sqrt( val );
		
		A = abs(A);
	}

	if( oIntersectArea ) *oIntersectArea = A;
	if( oNormIntersectArea ) {
		// normalize the area by the smallest disc area
		*oNormIntersectArea = A / (AqMathConst::PI * r2);
	}
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQDISC3D_H_
