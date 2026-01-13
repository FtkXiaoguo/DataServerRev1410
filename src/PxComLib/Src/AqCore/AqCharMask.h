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
// Filename:	AqCharMask.h
// Author:		David Guigonis
// Created:		Friday, July 07, 2006 at 3:45:59 PM
//

#ifndef _AQCHARMASK_H_
#define _AQCHARMASK_H_
//////////////////////////////////////////////////////////////////////////
#include "AqImage.h"
#include <vector>

class AqBitMask;

//////////////////////////////////////////////////////////////////////////
//
// Represent a volume of 8 bit images: 8 bit per voxel
//
//////////////////////////////////////////////////////////////////////////

class AqCharMask
{
public:
	AqCharMask() { Clear(); }
	virtual ~AqCharMask() { Clear(); }
	
	// Unpack the bit mask into this
	// Do not modify bits other than iMaskId if this has the right size
	bool Unpack(const AqBitMask& iBitMask, int iMaskId, bool iReverseMask = false);

	//////////////////////////////////////////////////////////////////////////
	// Adding/Removing AqImage to/from the volume

	bool AddImage(const AqImage& iImage);

	AqImage* InitFirstImage(unsigned short iSizeX, unsigned short iSizeY);

	AqImage* GetNewImage();

	void RemoveLastImages(int iNumToRemoveFromEnd = 1);

	// same as InitFirstImage, but share data instead
	AqImage*  AttachImage(char* iData, unsigned short iSizeX, unsigned short iSizeY);
	AqImage*  AttachImage(AqImage& iImage);  

	//////////////////////////////////////////////////////////////////////////
	// Data access

	AqImage* GetImage(unsigned int iImageIndex);
	const AqImage* GetImage(unsigned int iImageIndex) const;

	bool IsEmpty() const { return m_images.empty(); }

	bool SetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ, unsigned char iValue);
	unsigned char GetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ) const ;

	inline unsigned short GetSizeX() const { return (m_images.size() > 0) ? m_images[0]->GetSizeX() : 0; }
	inline unsigned short GetSizeY() const { return (m_images.size() > 0) ? m_images[0]->GetSizeY() : 0; }
	inline unsigned short GetSizeZ() const { return  m_images.size(); }

	void Clear();

protected:
	std::vector<AqImage*> m_images;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
inline bool AqCharMask::SetVoxelValue(unsigned short iX,
									  unsigned short iY,
									  unsigned short iZ,
									  unsigned char iValue)
{
	if (iZ >= m_images.size()) 
	{
		return false;
	}

	AqImage* im = m_images[iZ];
	if (!(im && im->IsInside(iX, iY)))
	{
		return false;
	}

	im->SetPixelValue(iX, iY, iValue);
	return true;
}

//////////////////////////////////////////////////////////////////////////
inline unsigned char AqCharMask::GetVoxelValue(unsigned short iX,
											   unsigned short iY,
											   unsigned short iZ) const
{
	assert( iZ < m_images.size() );

	if (iZ >= m_images.size()) 
	{
		return false;
	}

	AqImage* im = m_images[iZ];
	assert(im);

	return (unsigned char)(im->GetPixelValue(iX, iY));
}

//////////////////////////////////////////////////////////////////////////
inline void AqCharMask::Clear()
{
	while( !m_images.empty() )
	{
		delete m_images.back();
		m_images.pop_back();
	}
}

//////////////////////////////////////////////////////////////////////////
inline void AqCharMask::RemoveLastImages(int iNumToRemoveFromEnd /* = 1 */)
{
	for(int i=0; i<iNumToRemoveFromEnd; ++i)
	{
		delete m_images.back();
		m_images.pop_back();
	}
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQCHARMASK_H_
