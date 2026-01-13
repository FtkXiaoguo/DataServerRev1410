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
// Filename:	AqBitImage.h
// Author:		David Guigonis
// Created:		Wednesday, June 28, 2006 at 11:16:47 AM
//

#ifndef _AQBITIMAGE_H_
#define _AQBITIMAGE_H_
//////////////////////////////////////////////////////////////////////////
#include "AqBaseTypes.h"
#include "AqImage.h"


//////////////////////////////////////////////////////////////////////////
//
// Represent an image with 1 bit per pixel
//
// Remarks: Using SetBit/GetBit methods is ~5-6% slower
// than working with byte directly
//
//////////////////////////////////////////////////////////////////////////

class AqBitImage
{
public:
	AqBitImage();
	AqBitImage(unsigned short iSizeX, unsigned short iSizeY);

	virtual ~AqBitImage();


	//////////////////////////////////////////////////////////////////////////
	// Data copy

	// If iImage is shared, this will be shared.
	// If iImage is not shared, this will not be shared (have its own buffer).
	void Copy(const AqBitImage& iImage);

	// this will always have its own buffer, no matter what iImage is.
	void DeepCopy(const AqBitImage& iImage);


	//////////////////////////////////////////////////////////////////////////
	// Bit Image creation

	static int BitsPerByte() { return m_bitsPerByte; }

	// Pack iImage into this
	bool Pack(const AqImage& iImage, int iMaskId, bool iReverseMask = false);

	// Unpack the bit image into oImage
	// Do not modify bits other than iMaskId if oImage has the right size
	bool Unpack(AqImage& oImage, int iMaskId, bool iReverseMask = false) const;

	void Config(unsigned short iSizeX, unsigned short iSizeY);

	bool Attach(const AqBitImage &iImage);
	bool Attach(char* iData, unsigned short iUnpackedSizeX, unsigned short iUnpackedSizeY);


	//////////////////////////////////////////////////////////////////////////
	// Data access

	bool IsInside(unsigned short iX, unsigned short iY) const;

	inline char* GetData() const        { return m_image.GetData(); }
	inline int   GetDataSize() const    { return m_image.GetDataSize(); }
	inline int   GetRowDataSize() const { return m_rowSize; }

	static int   GetDataSize(unsigned short iSizeX, unsigned short iSizeY);
	static int   GetRowDataSize(unsigned short iSizeX, unsigned short iSizeY);

	inline unsigned short GetSizeX() const { return m_sizeX; }
	inline unsigned short GetSizeY() const { return m_sizeY; }
	
	/////////////////////////////////
	// Set methods
	void SetBit(unsigned short iX, unsigned short iY, bool iValue);

	// Use these methods to turn a bit ON
	// Faster than calling SetBit(iX, iY, true)
	void SetON(unsigned short iX, unsigned short iY);

	// Use these methods to turn a bit OFF
	// Faster than calling SetBit(iX, iY, false)
	void SetOFF(unsigned short iX, unsigned short iY);

	/////////////////////////////////
	// Get methods
	bool GetBit(unsigned short iX, unsigned short iY) const;

	inline bool IsON(unsigned short iX, unsigned short iY) const  { return GetBit(iX, iY);  }
	inline bool IsOFF(unsigned short iX, unsigned short iY) const { return !GetBit(iX, iY); }


protected:
	AqImage           m_image;   // packed data
	unsigned short    m_sizeX;   // real image size x
	unsigned short    m_sizeY;   // real image size y
    unsigned short    m_rowSize; // rounded to the higher byte

	static const int  m_bitsPerByte;

	// Do not use this method, use Copy method instead
	AqBitImage(const AqBitImage &iImage);
	const AqBitImage &operator =(const AqBitImage&);
	//
	
	void Reset();
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
inline bool AqBitImage::IsInside(unsigned short iX, unsigned short iY) const
{
	return (iX < m_sizeX) && (iY < m_sizeY);
}

//////////////////////////////////////////////////////////////////////////
inline void AqBitImage::SetBit(unsigned short iX, unsigned short iY, bool iValue)
{
	assert( IsInside(iX, iY) );
	assert( BitsPerByte() == 8 );

	register unsigned long offset = iY * m_rowSize + (iX >> 3);
	register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;
	register unsigned char flag = 0x80 >> (iX & 7);

    if (iValue)
	{
	    *(pRow) |= flag;
    }
	else 
	{
		*(pRow) &= ~flag;
	}
}

//////////////////////////////////////////////////////////////////////////
inline void AqBitImage::SetON(unsigned short iX, unsigned short iY)
{
	assert( IsInside(iX, iY) );
	assert( BitsPerByte() == 8 );

	register unsigned long offset = iY * m_rowSize + (iX >> 3);
	register unsigned char flag = 0x80 >> (iX & 7);
	register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;

    *(pRow) |= flag;
}

//////////////////////////////////////////////////////////////////////////
inline void AqBitImage::SetOFF(unsigned short iX, unsigned short iY)
{
	assert( IsInside(iX, iY) );
	assert( BitsPerByte() == 8 );

	register unsigned long offset = iY * m_rowSize + (iX >> 3);
	register unsigned char flag = ~(0x80 >> (iX & 7));
	register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;

	*(pRow) &= flag;
}

//////////////////////////////////////////////////////////////////////////
inline bool AqBitImage::GetBit(unsigned short iX, unsigned short iY) const
{
	assert( IsInside(iX, iY) );
	assert( BitsPerByte() == 8 );

    register unsigned char flag = 0x80 >> (iX & 7);
	register unsigned long offset = iY * m_rowSize + (iX >> 3);
	register unsigned char* pRow = (unsigned char *)m_image.m_data + offset;

    return (*pRow & flag) ? true : false;
}

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQBITIMAGE_H_
