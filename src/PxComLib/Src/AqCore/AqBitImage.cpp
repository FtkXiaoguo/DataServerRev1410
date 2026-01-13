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
// Filename:	AqBitImage.cpp
// Author:		David Guigonis
// Created:		Wednesday, June 28, 2006 at 2:56:05 PM
//

#include "StdAfx.h"
#include "AqBitImage.h"

//#define USE_ROW_ALIGNMENT
#undef USE_ROW_ALIGNMENT
 
#ifdef USE_ROW_ALIGNMENT
#  define ROW_ALIGNMENT 16
#endif

const int AqBitImage::m_bitsPerByte = 8;

AqBitImage::AqBitImage()
{
	Reset();
}

AqBitImage::AqBitImage(unsigned short iSizeX, unsigned short iSizeY)
{
	Reset();
	Config(iSizeX, iSizeY);
}

AqBitImage::~AqBitImage()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void AqBitImage::Reset()
{
	m_image.Config(0, 0, kAqVolumeDataTypeUnsignedChar);
	
	m_sizeX = 0;
	m_sizeY = 0;
}

//////////////////////////////////////////////////////////////////////////
// Do not use this method, use Copy method instead
AqBitImage::AqBitImage(const AqBitImage &iImage)
{
	*this = iImage;
}

const AqBitImage &AqBitImage::operator =(const AqBitImage& iImage)
{
	Copy(iImage);
	return *this;
}
//////////////////////////////////////////////////////////////////////////

// If iImage is shared, this will be shared.
// If iImage is not shared, this will not be shared (have its own buffer).
void AqBitImage::Copy(const AqBitImage& iImage)
{
	m_image.Copy(iImage.m_image);
	m_sizeX   = iImage.m_sizeX;
	m_sizeY   = iImage.m_sizeY;
	m_rowSize = iImage.m_rowSize;
}

// this will always have its own buffer, no matter what iImage is.
void AqBitImage::DeepCopy(const AqBitImage& iImage)
{
	m_image.DeepCopy(iImage.m_image);
	m_sizeX   = iImage.m_sizeX;
	m_sizeY   = iImage.m_sizeY;
	m_rowSize = iImage.m_rowSize;
}

//////////////////////////////////////////////////////////////////////////
// Pack iImage into this
//bool AqBitImage::Pack(const AqImage& iImage, int iMaskId, bool iReverseMask /* = false */)
//{
//	if( iImage.BytesPerPixel() != 1 )
//	{
//		Reset();
//		return false;
//	}
//
//	Config(iImage.GetSizeX(), iImage.GetSizeY());
//	::memset(GetData(), 0, GetDataSize());
//
//	unsigned char* p = (unsigned char*)iImage.GetData();
//	register unsigned char maskdsts =  (1<<iMaskId);
//
//	if (!iReverseMask)
//	{
//		for(int i=0; i<GetSizeY(); ++i)
//		{
//			for(int j=0; j<GetSizeX(); ++j)
//			{
//				if ((*p++ & maskdsts) > 0)
//				{
//					SetON(j,i);
//				}
//			}
//		}
//	}
//	else
//	{
//		// Reverse the mask
//		for(int i=0; i<GetSizeY(); ++i)
//		{
//			for(int j=0; j<GetSizeX(); ++j)
//			{
//				if ((*p++ & maskdsts) == 0)
//				{
//					SetON(j,i);
//				}
//			}
//		}
//	}
//
//	return true;
//}

bool AqBitImage::Pack(const AqImage& iImage, int iMaskId, bool iReverseMask /* = false */)
{
	int i,j;
	if( iImage.BytesPerPixel() != 1 )
	{
		Reset();
		return false;
	}

	Config(iImage.GetSizeX(), iImage.GetSizeY());
	//::memset(GetData(), 0, GetDataSize());

	unsigned char* p = (unsigned char*)iImage.GetData();
	register unsigned char maskdsts =  (1<<iMaskId);
	const int sizeX = GetSizeX() / 8;

	if (!iReverseMask)
	{
		for( i=0; i<GetSizeY(); ++i)
		{
			register unsigned long offset = i * m_rowSize;
			register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;
						
			for( j=0; j<sizeX; ++j)
			{
				unsigned char flag = 0;

				if ((p[0] & maskdsts) > 0) flag |= 0x80;
				if ((p[1] & maskdsts) > 0) flag |= 0x40;
				if ((p[2] & maskdsts) > 0) flag |= 0x20;
				if ((p[3] & maskdsts) > 0) flag |= 0x10;

				if ((p[4] & maskdsts) > 0) flag |= 0x08;
				if ((p[5] & maskdsts) > 0) flag |= 0x04;
				if ((p[6] & maskdsts) > 0) flag |= 0x02;
				if ((p[7] & maskdsts) > 0) flag |= 0x01;

				p += 8;
				*pRow++ = flag;
			}

			for(j=sizeX*8; j<GetSizeX(); ++j)
			{
				SetBit(j,i, (*p++ & maskdsts) > 0);
			}
		}
	}
	else
	{
		// Reverse the mask
		for( i=0; i<GetSizeY(); ++i)
		{
			register unsigned long offset = i * m_rowSize;
			register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;
				
			int j;
			for( j=0; j<sizeX; ++j)
			{
				unsigned char flag = 0;

				if ((p[0] & maskdsts) == 0) flag |= 0x80;
				if ((p[1] & maskdsts) == 0) flag |= 0x40;
				if ((p[2] & maskdsts) == 0) flag |= 0x20;
				if ((p[3] & maskdsts) == 0) flag |= 0x10;

				if ((p[4] & maskdsts) == 0) flag |= 0x08;
				if ((p[5] & maskdsts) == 0) flag |= 0x04;
				if ((p[6] & maskdsts) == 0) flag |= 0x02;
				if ((p[7] & maskdsts) == 0) flag |= 0x01;

				p += 8;
				*pRow++ = flag;
			}

			for(j=sizeX*8; j<GetSizeX(); ++j)
			{
				SetBit(j,i, (*p++ & maskdsts) == 0);
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Unpack the bit image into oImage
//bool AqBitImage::Unpack(AqImage& oImage, int iMaskId, bool iReverseMask /* = false */) const
//{
//	if (oImage.GetSizeX() != GetSizeX() ||
//		oImage.GetSizeY() != GetSizeY() ||
//		oImage.DataType() != kAqVolumeDataTypeUnsignedChar)
//	{
//		oImage.Config(GetSizeX(), GetSizeY(), kAqVolumeDataTypeUnsignedChar);
//		if (oImage.GetData())
//			memset(oImage.GetData(), 0, oImage.GetDataSize());
//	}
//
//	assert( oImage.BytesPerPixel() == 1 );
//
//	unsigned char* p = (unsigned char*)oImage.GetData();
//	register unsigned char maskdstsON =  (1<<iMaskId);
//	register unsigned char maskdstsOFF =  ~maskdstsON;
//
//	if (!iReverseMask)
//	{
//		for(int i=0; i<GetSizeY(); ++i)
//		{
//			for(int j=0; j<GetSizeX(); ++j)
//			{
//				if ( GetBit(j,i) )
//				{
//					*p++ |= maskdstsON;
//				}
//				else
//				{
//					*p++ &= maskdstsOFF;
//				}
//			}
//		}
//	}
//	else
//	{
//		// Reverse the mask
//		for(int i=0; i<GetSizeY(); ++i)
//		{
//			for(int j=0; j<GetSizeX(); ++j)
//			{
//				if ( GetBit(j,i) )
//				{
//					*p++ &= maskdstsOFF;
//				}
//				else
//				{
//					*p++ |= maskdstsON;
//				}
//			}
//		}
//	}
//
//	return true;
//}

bool AqBitImage::Unpack(AqImage& oImage, int iMaskId, bool iReverseMask /* = false */) const
{
	int i,j;
	if (oImage.GetSizeX() != GetSizeX() ||
		oImage.GetSizeY() != GetSizeY() ||
		oImage.DataType() != kAqVolumeDataTypeUnsignedChar)
	{
		oImage.Config(GetSizeX(), GetSizeY(), kAqVolumeDataTypeUnsignedChar);
		if (oImage.GetData())
			memset(oImage.GetData(), 0, oImage.GetDataSize());
	}

	assert( oImage.BytesPerPixel() == 1 );

	unsigned char* p = (unsigned char*)oImage.GetData();
	register unsigned char maskdstsON =  (1<<iMaskId);
	register unsigned char maskdstsOFF =  ~maskdstsON;

	const int sizeX = GetSizeX() / 8;

	if (!iReverseMask)
	{
		for( i=0; i<GetSizeY(); ++i)
		{
			register unsigned long offset = i * m_rowSize;
			register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;
						
			for( j=0; j<sizeX; ++j)
			{
				unsigned char flag = *pRow++;

				if ( flag & 0x80 ) p[0] |= maskdstsON;
				else               p[0] &= maskdstsOFF;
				if ( flag & 0x40 ) p[1] |= maskdstsON;
				else               p[1] &= maskdstsOFF;
				if ( flag & 0x20 ) p[2] |= maskdstsON;
				else               p[2] &= maskdstsOFF;
				if ( flag & 0x10 ) p[3] |= maskdstsON;
				else               p[3] &= maskdstsOFF;

				if ( flag & 0x08 ) p[4] |= maskdstsON;
				else               p[4] &= maskdstsOFF;
				if ( flag & 0x04 ) p[5] |= maskdstsON;
				else               p[5] &= maskdstsOFF;
				if ( flag & 0x02 ) p[6] |= maskdstsON;
				else               p[6] &= maskdstsOFF;
				if ( flag & 0x01 ) p[7] |= maskdstsON;
				else               p[7] &= maskdstsOFF;

				p += 8;
			}

			for(j=sizeX*8; j<GetSizeX(); ++j)
			{
				if ( GetBit(j,i) )
				{
					*p++ |= maskdstsON;
				}
				else
				{
					*p++ &= maskdstsOFF;
				}
			}
		}
	}
	else
	{
		// Reverse the mask
		for( i=0; i<GetSizeY(); ++i)
		{
			register unsigned long offset = i * m_rowSize;
			register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;
				
			int j;
			for( j=0; j<sizeX; ++j)
			{
				unsigned char flag = *pRow++;

				if ( flag & 0x80 ) p[0] &= maskdstsOFF;
				else               p[0] |= maskdstsON;
				if ( flag & 0x40 ) p[1] &= maskdstsOFF;
				else               p[1] |= maskdstsON;
				if ( flag & 0x20 ) p[2] &= maskdstsOFF;
				else               p[2] |= maskdstsON;
				if ( flag & 0x10 ) p[3] &= maskdstsOFF;
				else               p[3] |= maskdstsON;

				if ( flag & 0x08 ) p[4] &= maskdstsOFF;
				else               p[4] |= maskdstsON;
				if ( flag & 0x04 ) p[5] &= maskdstsOFF;
				else               p[5] |= maskdstsON;
				if ( flag & 0x02 ) p[6] &= maskdstsOFF;
				else               p[6] |= maskdstsON;
				if ( flag & 0x01 ) p[7] &= maskdstsOFF;
				else               p[7] |= maskdstsON;

				p += 8;
			}

			for(j=sizeX*8; j<GetSizeX(); ++j)
			{
				if ( GetBit(j,i) )
				{
					*p++ &= maskdstsOFF;
				}
				else
				{
					*p++ |= maskdstsON;
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
int AqBitImage::GetDataSize(unsigned short iSizeX, unsigned short iSizeY)
{
	return GetRowDataSize(iSizeX, iSizeY) * iSizeY;
}

int AqBitImage::GetRowDataSize(unsigned short iSizeX, unsigned short iSizeY)
{
	int unpackedMasksPerByte = 8;

	unsigned short rowSize = iSizeX / unpackedMasksPerByte;
    if (iSizeX % unpackedMasksPerByte > 0)
	{
		// minimum number of bytes needed per row
		rowSize++;
	}

#ifdef ROW_ALIGNMENT
	// Align the rows
    rowSize += (rowSize % ROW_ALIGNMENT) > 0 ? ROW_ALIGNMENT - (rowSize % ROW_ALIGNMENT) : 0;
#endif

	return rowSize;
}

//////////////////////////////////////////////////////////////////////////
void AqBitImage::Config(unsigned short iSizeX, unsigned short iSizeY)
{
	m_rowSize = GetRowDataSize(iSizeX, iSizeY);

	m_image.Config(m_rowSize, iSizeY, kAqVolumeDataTypeUnsignedChar);
	m_sizeX = iSizeX;
	m_sizeY = iSizeY;
}

//////////////////////////////////////////////////////////////////////////
bool AqBitImage::Attach(const AqBitImage &iImage)
{
	return Attach(iImage.GetData(), iImage.GetSizeX(), iImage.GetSizeY());
}

//////////////////////////////////////////////////////////////////////////
bool AqBitImage::Attach(char* iData,
						unsigned short iUnpackedSizeX,
						unsigned short iUnpackedSizeY)
{
	m_rowSize = GetRowDataSize(iUnpackedSizeX, iUnpackedSizeY);
	
	bool rv = m_image.Attach(iData, m_rowSize, iUnpackedSizeY, kAqVolumeDataTypeUnsignedChar);
	if (!rv)
	{
		Reset();
		return false;
	}

	m_sizeX = iUnpackedSizeX;
	m_sizeY = iUnpackedSizeY;
	return true;
}

#undef USE_ROW_ALIGNMENT
