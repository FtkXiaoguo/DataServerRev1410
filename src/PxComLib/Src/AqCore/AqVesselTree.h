// AqVesselTree.h: interface for the AqVesselTree class.
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
// Filename:	AqVesselTree.h
// Author:		Sha He
// Created:		Monday, May 08, 2006 at 10:19:06 AM
//

#ifndef _AQVESSELTREE_H_
#define _AQVESSELTREE_H_
//////////////////////////////////////////////////////////////////////////

// Sha 2006.5.8 Save trees in COF (ID:5592) 

#include "AqTree.h"

namespace AqTree
{

//-------------------------------------------------------------------
// Perpendicular cross-section:
class  CSection
{
public:
	CSection();
	virtual ~CSection();
	CSection& operator =(const CSection& in);

	// Implement orientation vectors, inner wall and outer wall here.
};

//-------------------------------------------------------------------
// An element of a vessel tree.
// Simplest variety is a centerline point. Richest variety includes a section.
class  CVesselElement
{
public:
	CVesselElement();
	virtual ~CVesselElement();
	CVesselElement& operator =(const CVesselElement& in);

	// Units are in mm, in Patient coordinates:
	void SetCenterPoint(float x, float y, float z);
	void SetCenterPoint(double x, double y, double z);
	void GetCenterPoint(float& x, float& y, float& z) const;
	void GetCenterPoint(double& x, double& y, double& z) const;

	void SetRadius(float r);
	void SetRadius(double r);
	bool GetRadius(float& r) const;  // Returns false if not available.
	bool GetRadius(double& r) const;  // Returns false if not available.

	// Cross-section:
	bool HasSection() const;  // Returns false if section doesn't exist.
	CSection* GetSection();  // Allocates if not exist. You can then set values in the returned object.
	const CSection* GetSection() const;  // To read. Returns NULL if not available.

protected:
	float m_x, m_y, m_z;  // Center point in mm, in Patient coordinates.
	float m_Radius;  // Radius in mm. Optional, <0 if not available.
	CSection* m_pSection;  // Optional, NULL if not available.
};

//-------------------------------------------------------------------

typedef AqTreeBranch<CVesselElement> AqVesselTree;


}; // end namespace AqTree

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQVESSELTREE_H_
