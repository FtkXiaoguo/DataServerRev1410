// AqVesselTree.cpp: implementation of the AqVesselTree class.
//
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
// Filename:	AqVesselTree.cpp
// Author:		Sha He
// Created:		Monday, May 08, 2006 at 10:19:06 AM
//
//////////////////////////////////////////////////////////////////////

// Sha 2006.5.8 Save trees in COF (ID:5592) 


#include "AqVesselTree.h"
using namespace AqTree;

//////////////////////////////////////////////////////////////////////
// Class CSection:
//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
CSection::CSection()
{
}

//------------------------------------------------------------------------------
CSection::~CSection()
{
}

//------------------------------------------------------------------------------
CSection& CSection::operator =(const CSection& in)
{
	return *this;
}

//////////////////////////////////////////////////////////////////////
																////
															////
														////
													////
												////
											////
										////
									////
								////
							////
						////
					////
				////
			////
		////
	////
//////////////////////////////////////////////////////////////////////
// Class CVesselElement:
//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
CVesselElement::CVesselElement()
{
	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_Radius = -1.0;

	m_pSection = 0;
}

//------------------------------------------------------------------------------
CVesselElement::~CVesselElement()
{
#define RELEASE_OBJECT(p) if (p) { delete p; p = 0; }

	RELEASE_OBJECT(m_pSection)
}

//------------------------------------------------------------------------------
CVesselElement& CVesselElement::operator =(const CVesselElement& in)
{
	m_x = in.m_x;
	m_y = in.m_y;
	m_z = in.m_z;
	m_Radius = in.m_Radius;

	if (in.m_pSection)
	{
		CSection* pSection = GetSection();
		if (pSection)
		{
			*pSection = *in.m_pSection;
		}
	}
	else
	{
		RELEASE_OBJECT(m_pSection)
	}

	return *this;
}

//------------------------------------------------------------------------------
void CVesselElement::SetCenterPoint(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

//------------------------------------------------------------------------------
void CVesselElement::SetCenterPoint(double x, double y, double z)
{
	m_x = float(x);
	m_y = float(y);
	m_z = float(z);
}

//------------------------------------------------------------------------------
void CVesselElement::GetCenterPoint(float& x, float& y, float& z) const
{
	x = m_x;
	y = m_y;
	z = m_z;
}

//------------------------------------------------------------------------------
void CVesselElement::GetCenterPoint(double& x, double& y, double& z) const
{
	x = m_x;
	y = m_y;
	z = m_z;
}

//------------------------------------------------------------------------------
void CVesselElement::SetRadius(float r)
{
	m_Radius = r;
}

//------------------------------------------------------------------------------
void CVesselElement::SetRadius(double r)
{
	m_Radius = float(r);
}

//------------------------------------------------------------------------------
bool CVesselElement::GetRadius(float& r) const
{
	if (m_Radius < 0)
	{
		return false;
	}

	r = m_Radius;
	return true;
}

//------------------------------------------------------------------------------
bool CVesselElement::GetRadius(double& r) const
{
	if (m_Radius < 0)
	{
		return false;
	}

	r = m_Radius;
	return true;
}

//------------------------------------------------------------------------------
bool CVesselElement::HasSection() const
{
	return (m_pSection != 0);
}

//------------------------------------------------------------------------------
CSection* CVesselElement::GetSection()
{
	if (m_pSection == 0)
	{
		try { m_pSection = new CSection; }
		catch (...) { }
	}
	return m_pSection;
}

//------------------------------------------------------------------------------
const CSection* CVesselElement::GetSection() const
{
	return m_pSection;
}
