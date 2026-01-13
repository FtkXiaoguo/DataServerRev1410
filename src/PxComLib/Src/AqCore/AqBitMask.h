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
// Filename:	AqBitMask.h
// Author:		David Guigonis
// Created:		Friday, July 07, 2006 at 3:45:59 PM
//

#ifndef _AQBITMASK_H_
#define _AQBITMASK_H_
//////////////////////////////////////////////////////////////////////////
#include "AqBitImage.h"
#include <vector>

class AqCharMask;

//////////////////////////////////////////////////////////////////////////
//
// Represent a volume of bit images: 1 bit per voxel
//
// Remarks: Using 1 bit per voxel methods is ~5-6% slower
// than working with byte directly (AqCharMask)
//
//////////////////////////////////////////////////////////////////////////

class AqBitMask
{
public:
	AqBitMask() { Clear(); }
	virtual ~AqBitMask() { Clear(); }
	
	// Pack iCharMask into this
	bool Pack(const AqCharMask& iCharMask, int iMaskId, bool iReverseMask = false);

	// Packed the input continuous mask buffer into this bit mask
	bool Pack(const char* iUnpackedBuffer,
              int iUnPackedXSize, int iUnPackedYSize, int iUnPackedZSize,
			  int iMaskId, bool iReverseMask = false,
			  bool iReverseInZ = false);

	// Unpacked this bit mask into a continuous mask buffer
	bool Unpack(char* oUnpackedBuffer, int iMaskId,
                bool iReverseMask = false, bool iReverseInZ = false) const;

	//////////////////////////////////////////////////////////////////////////
	// Adding/Removing AqBitImage to/from the volume

	bool AddImage(const AqBitImage& iImage);

	AqBitImage* InitFirstImage(unsigned short iSizeX, unsigned short iSizeY);

	AqBitImage* GetNewImage();

	void RemoveLastImages(int iNumToRemoveFromEnd = 1);

	// same as InitFirstImage, but share data instead
	AqBitImage*  AttachImage(char* iData, unsigned short iSizeX, unsigned short iSizeY);
	AqBitImage*  AttachImage(AqBitImage& iImage);  

	//////////////////////////////////////////////////////////////////////////
	// Data access

	AqBitImage* GetImage(unsigned int iImageIndex);
	const AqBitImage* GetImage(unsigned int iImageIndex) const;
	
	bool IsEmpty() const { return m_images.empty(); }

	bool SetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ, bool iValue);
	bool GetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ) const ;

	inline unsigned short GetSizeX() const { return (m_images.size() > 0) ? m_images[0]->GetSizeX() : 0; }
	inline unsigned short GetSizeY() const { return (m_images.size() > 0) ? m_images[0]->GetSizeY() : 0; }
	inline unsigned short GetSizeZ() const { return  m_images.size(); }

	void Clear();

protected:
	std::vector<AqBitImage*> m_images;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
inline bool AqBitMask::SetVoxelValue(unsigned short iX,
									 unsigned short iY,
									 unsigned short iZ,
									 bool iValue)
{
	if (iZ >= m_images.size()) 
	{
		return false;
	}

	AqBitImage* im = m_images[iZ];
	if (!(im && im->IsInside(iX, iY)))
	{
		return false;
	}

	im->SetBit(iX, iY, iValue);
	return true;
}

//////////////////////////////////////////////////////////////////////////
inline bool AqBitMask::GetVoxelValue(unsigned short iX,
									 unsigned short iY,
									 unsigned short iZ) const
{
	assert( iZ < m_images.size() );

	if (iZ >= m_images.size()) 
	{
		return false;
	}

	AqBitImage* im = m_images[iZ];
	assert(im);

	return im->GetBit(iX, iY);
}

//////////////////////////////////////////////////////////////////////////
inline void AqBitMask::Clear()
{
	while( !m_images.empty() )
	{
		delete m_images.back();
		m_images.pop_back();
	}
}

//////////////////////////////////////////////////////////////////////////
inline void AqBitMask::RemoveLastImages(int iNumToRemoveFromEnd /* = 1 */)
{
	for(int i=0; i<iNumToRemoveFromEnd; ++i)
	{
		delete m_images.back();
		m_images.pop_back();
	}
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQBITMASK_H_
