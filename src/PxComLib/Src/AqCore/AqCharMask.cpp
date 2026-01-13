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
// Filename:	AqCharMask.cpp
// Author:		David Guigonis
// Created:		Friday, July 07, 2006 at 3:46:22 PM
//

#include "StdAfx.h"
#include "AqCharMask.h"
#include "AqBitMask.h"


bool AqCharMask::AddImage(const AqImage& iImage) 
{	
	if (!m_images.empty())
	{
		const AqImage* im = m_images.front();
		if (im->GetSizeX() != iImage.GetSizeX() ||
			im->GetSizeY() != iImage.GetSizeY() ||
			im->DataType() != iImage.DataType()
			)
			return false;
	}
	
	AqImage* pImage = new AqImage();
	pImage->Copy(iImage);
	
	m_images.push_back(pImage);
	
	return true;
}

AqImage* AqCharMask::InitFirstImage(unsigned short iSizeX, unsigned short iSizeY)
{
	if (!m_images.empty())
		Clear();

	AqImage* pImage = new AqImage(iSizeX, iSizeY, kAqVolumeDataTypeUnsignedChar);
	m_images.push_back(pImage);
	
	return pImage;
}

AqImage* AqCharMask::GetImage(unsigned int iImageIndex)
{
	int imageN = m_images.size();
	if ( (iImageIndex < 0 && imageN == 0) || iImageIndex >= imageN) 
		return 0;

	return m_images[iImageIndex];
}

const AqImage* AqCharMask::GetImage(unsigned int iImageIndex) const
{
	int imageN = m_images.size();
	if ( (iImageIndex < 0 && imageN == 0) || iImageIndex >= imageN) 
		return 0;

	return m_images[iImageIndex];
}

AqImage* AqCharMask::GetNewImage()
{
	if (m_images.empty())
		return 0;	
	
	AqImage* iniImage = m_images.front();
	assert( iniImage );
	
	AqImage* newImage = new AqImage(iniImage->GetSizeX(), iniImage->GetSizeY(), kAqVolumeDataTypeUnsignedChar);
	m_images.push_back(newImage);
	return newImage;
}

AqImage* AqCharMask::AttachImage(char* iData, unsigned short iSizeX, unsigned short iSizeY)
{
	if (!m_images.empty())
		Clear();

	AqImage* pImage = new AqImage();
	pImage->Attach(iData, iSizeX, iSizeY, kAqVolumeDataTypeUnsignedChar);
	m_images.push_back(pImage);
	
	return pImage;
}

AqImage* AqCharMask::AttachImage(AqImage& iImage)
{
	if (iImage.DataType() != kAqVolumeDataTypeUnsignedChar )
	{
		assert(0 && "Can only contains unsigned char");
		return 0;
	}

	return AttachImage(iImage.GetData(), iImage.GetSizeX(), iImage.GetSizeY());
}

// Unpack the bit mask into this
// Do not modify bits other than iMaskId if this has the right size
bool AqCharMask::Unpack(const AqBitMask& iBitMask, int iMaskId, bool iReverseMask /* = false */)
{
	if( iBitMask.IsEmpty() )
	{
		Clear();
		return true;
	}

	if (GetSizeX() != iBitMask.GetSizeX() ||
		GetSizeY() != iBitMask.GetSizeY()
		)
	{
		Clear();
	}

	if (GetSizeZ() > iBitMask.GetSizeZ())
	{
		// remove the extra slices if necessary
		for(int i=iBitMask.GetSizeZ(); i<GetSizeZ(); ++i)
			delete m_images[i];
		m_images.resize(iBitMask.GetSizeZ());
	}
	else
	{
		// add extra bit images
		for(int i=GetSizeZ(); i<iBitMask.GetSizeZ(); ++i)
		{
			AqImage* bitIm = GetNewImage();
			if (!bitIm)
			{
				return false;
			}
		}
	}

	// init the first image if necessary
	if (IsEmpty() && !InitFirstImage(iBitMask.GetSizeX(), iBitMask.GetSizeY()))
	{
		return false;
	}

	assert( GetSizeZ() == iBitMask.GetSizeZ() );

	// update the bit images
	for(int i=0; i<GetSizeZ(); ++i)
	{
		AqImage* charIm = GetImage(i);
		assert(charIm);

		const AqBitImage* bitIm = iBitMask.GetImage(i);
		assert(bitIm);

		bool rv = bitIm->Unpack(*charIm, iMaskId, iReverseMask);
		if (!rv) return false;
	}

	return true;
}
