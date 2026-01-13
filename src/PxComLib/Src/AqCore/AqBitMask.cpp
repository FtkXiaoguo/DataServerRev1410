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
// Filename:	AqBitMask.cpp
// Author:		David Guigonis
// Created:		Friday, July 07, 2006 at 3:46:22 PM
//

#include "StdAfx.h"
#include "AqBitMask.h"
#include "AqCharMask.h"


bool AqBitMask::AddImage(const AqBitImage& iImage) 
{	
	if (!m_images.empty())
	{
		const AqBitImage* im = m_images.front();
		if (im->GetSizeX() != iImage.GetSizeX() ||
			im->GetSizeY() != iImage.GetSizeY()
			)
			return false;
	}
	
	AqBitImage* pImage = new AqBitImage();
	pImage->Copy(iImage);
	
	m_images.push_back(pImage);
	
	return true;
}

AqBitImage* AqBitMask::InitFirstImage(unsigned short iSizeX, unsigned short iSizeY)
{
	if (!m_images.empty())
		Clear();

	AqBitImage* pImage = new AqBitImage(iSizeX, iSizeY);
	m_images.push_back(pImage);
	
	return pImage;
}

AqBitImage* AqBitMask::GetImage(unsigned int iImageIndex)
{
	int imageN = m_images.size();
	if ( (iImageIndex < 0 && imageN == 0) || iImageIndex >= imageN) 
		return 0;

	return m_images[iImageIndex];
}

const AqBitImage* AqBitMask::GetImage(unsigned int iImageIndex) const
{
	int imageN = m_images.size();
	if ( (iImageIndex < 0 && imageN == 0) || iImageIndex >= imageN) 
		return 0;

	return m_images[iImageIndex];
}

AqBitImage* AqBitMask::GetNewImage()
{
	if (m_images.empty())
		return 0;
	
	AqBitImage* iniImage = m_images.front();
	assert( iniImage );
	
	AqBitImage* newImage = new AqBitImage(iniImage->GetSizeX(), iniImage->GetSizeY());
	m_images.push_back(newImage);
	return newImage;
}

AqBitImage* AqBitMask::AttachImage(char* iData, unsigned short iSizeX, unsigned short iSizeY)
{
	if (!m_images.empty())
		Clear();

	AqBitImage* pImage = new AqBitImage();
	pImage->Attach(iData, iSizeX, iSizeY);
	m_images.push_back(pImage);
	
	return pImage;
}

AqBitImage* AqBitMask::AttachImage(AqBitImage& iImage)
{
	return AttachImage(iImage.GetData(), iImage.GetSizeX(), iImage.GetSizeY());
}

// Pack iCharMask into this
bool AqBitMask::Pack(const AqCharMask& iCharMask, int iMaskId, bool iReverseMask /* = false */)
{
	if( iCharMask.IsEmpty() )
	{
		Clear();
		return true;
	}

	if (GetSizeX() != iCharMask.GetSizeX() ||
		GetSizeY() != iCharMask.GetSizeY()
		)
	{
		Clear();
	}

	if (GetSizeZ() > iCharMask.GetSizeZ())
	{
		// remove the extra slices if necessary
		for(int i=iCharMask.GetSizeZ(); i<GetSizeZ(); ++i)
			delete m_images[i];
		m_images.resize(iCharMask.GetSizeZ());
	}
	else
	{
		// add extra bit images
		for(int i=GetSizeZ(); i<iCharMask.GetSizeZ(); ++i)
		{
			AqBitImage* bitIm = GetNewImage();
			if (!bitIm)
			{
				return false;
			}
		}
	}

	// init the first image if necessary
	if (IsEmpty() && !InitFirstImage(iCharMask.GetSizeX(), iCharMask.GetSizeY()))
	{
		return false;
	}

	assert( GetSizeZ() == iCharMask.GetSizeZ() );

	// update the bit images
	for(int i=0; i<GetSizeZ(); ++i)
	{
		AqBitImage* bitIm = GetImage(i);
		assert(bitIm);

		const AqImage* charIm = iCharMask.GetImage(i);
		assert(charIm);

		bool rv = bitIm->Pack(*charIm, iMaskId, iReverseMask);
		if (!rv) return false;
	}

	return true;
}

// Packed the input continuous mask buffer into a bit mask
bool AqBitMask::Pack(const char* iUnpackedBuffer,
					 int iUnPackedXSize, int iUnPackedYSize, int iUnPackedZSize,
					 int iMaskId, bool iReverseMask /* = false */,
					 bool iReverseInZ /* = false */)
{
	if ( !iUnpackedBuffer )
	{
		return false;
	}

	if ( iUnPackedXSize * iUnPackedYSize * iUnPackedZSize == 0 )
	{
		return false;
	}

	const unsigned short unpackedSliceSize = iUnPackedXSize * iUnPackedYSize;
	AqImage tmpSlice;

	Clear();

	for(int z=0; z<iUnPackedZSize; ++z)
	{
		int sliceIndex = iReverseInZ ? iUnPackedZSize-1-z : z;		

		// get a new packed image
		AqBitImage* bim = 0;
		if (z == 0)
		{
			bim = InitFirstImage(iUnPackedXSize, iUnPackedYSize);
		}
		else
		{
			bim = GetNewImage();
		}
		if (!bim)
		{
			return false;
		}

		// get a temporary unpacked image
		bool rv = tmpSlice.Attach((char*)(iUnpackedBuffer + sliceIndex * unpackedSliceSize),
			iUnPackedXSize, iUnPackedYSize, kAqVolumeDataTypeUnsignedChar);
		if (!rv)
		{
			return false;
		}

		// pack
		rv = bim->Pack(tmpSlice, iMaskId, iReverseMask);
		if (!rv)
		{
			return false;
		}
	}
	
	return true;
}

// Unpacked this bit mask into a continuous mask buffer
bool AqBitMask::Unpack(char* oUnpackedBuffer, int iMaskId,
						bool iReverseMask /* = false */,
						bool iReverseInZ /* = false */) const
{
	if ( !oUnpackedBuffer )
	{
		return false;
	}

	const unsigned short sizeX = GetSizeX();
	const unsigned short sizeY = GetSizeY();
	const unsigned short sizeZ = GetSizeZ();

	const unsigned short unpackedSliceSize = sizeX * sizeY;
	AqImage tmpSlice;

	for(int z=0; z<sizeZ; ++z)
	{
		int sliceIndex = iReverseInZ ? sizeZ-1-z : z;		

		const AqBitImage* bim = GetImage(sliceIndex);
		if (!bim)
		{
			return false;
		}

		bool rv = tmpSlice.Attach(oUnpackedBuffer + z * unpackedSliceSize,
			sizeX, sizeY, kAqVolumeDataTypeUnsignedChar);
		if (!rv)
		{
			return false;
		}

		rv = bim->Unpack(tmpSlice, iMaskId, iReverseMask);
		if (!rv)
		{
			return false;
		}
	}

	return true;
}
