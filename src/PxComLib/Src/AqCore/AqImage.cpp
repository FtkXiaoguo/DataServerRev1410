
/*********************************************************************
 *********************************************************************
 **************											**************
 **************	This document contains information that	**************
 **************	   is proprietary to PreXion 	**************
 **************		     All rights reserved.			**************
 **************			Copyright PreXion 		**************
 **************				 2005-2006					**************
 **************											**************
 *********************************************************************
 *********************************************************************
 *********************************************************************
 ****															  ****
 **** File       : AqImage.cpp									  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Definition of AqImage class.					  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqImage.cpp: implementation of the AqImage class.
//
//////////////////////////////////////////////////////////////////////

#include "AqImage.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//GL 12-9-2005  revise the class to handle memory management
//because the change is so big, diff is hard to read. Please read whole source code
// for verification.

// DavidG: 4-28-2006 Allow sharing data buffer
// DavidG: 6-28-2006 Memory alignment, copy methods

AqImage::AqImage() : m_data(0), m_shareBuffer(false)
{
	Reset();
}


AqImage::AqImage(unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType) : m_data (0), m_shareBuffer(false)
{
	Reset();
	Config(iSizeX, iSizeY, iDataType);
}

AqImage::AqImage(const AqImage &iImage) : m_data (0), m_shareBuffer(false)
{
	*this = iImage;	
}


AqImage::~AqImage()
{
	Reset();
}


void AqImage::Reset()
{
	m_buffer.Reset();

	m_data = 0;
	m_shareBuffer = false;

	m_sizeX  = 0;
	m_sizeY  = 0;

	m_dataType = kAqVolumeDataTypeUnsignedShort;
	
	/*
	m_minBoundValue = 0;
	m_maxBoundValue = 0;

	m_minDataValue = 0;
	m_maxDataValue = 0;
	*/

}

// If iImage is shared, this will be shared.
// If iImage is not shared, this will not be shared (have its own buffer).
void AqImage::Copy(const AqImage& iImage)
{
	Reset();

	m_shareBuffer = iImage.m_shareBuffer;
	m_sizeX       = iImage.m_sizeX;
	m_sizeY       = iImage.m_sizeY;
	m_dataType    = iImage.m_dataType;

	/*
	m_minBoundValue = iImage.m_minBoundValue;
	m_maxBoundValue = iImage.m_maxBoundValue;
	m_minDataValue = iImage.MinDataValue();
	m_maxDataValue = iImage.MaxDataValue();
	*/

	if( iImage.m_shareBuffer )
	{
		m_data = iImage.m_data;
	}
	else
	{
		long imSize = iImage.GetDataSize();
		if(imSize != 0)
		{
			m_buffer.Allocate(imSize, false, true);
			m_data = m_buffer.GetData();
			
			if (iImage.m_data)
				memcpy(m_data, iImage.m_data, imSize);
		}
	}
}

// this will always have its own buffer, no matter what iImage is.
void AqImage::DeepCopy(const AqImage& iImage)
{
	Reset();

	m_shareBuffer = false;
	m_sizeX       = iImage.m_sizeX;
	m_sizeY       = iImage.m_sizeY;
	m_dataType    = iImage.m_dataType;

	/*
	m_minBoundValue = iImage.m_minBoundValue;
	m_maxBoundValue = iImage.m_maxBoundValue;
	m_minDataValue = iImage.MinDataValue();
	m_maxDataValue = iImage.MaxDataValue();
	*/

	long imSize = iImage.GetDataSize();
	if(imSize != 0)
	{
		m_buffer.Allocate(imSize, false, true);
		m_data = m_buffer.GetData();
		
		if (iImage.m_data)
			memcpy(m_data, iImage.m_data, imSize);
	}
}

const AqImage &AqImage::operator =(const AqImage& iImage) 
{
	Copy(iImage);
	return *this;
}



void AqImage::Config(unsigned short iSizeX, unsigned short iSizeY,  eAqVolumeDataType iDataType) 
{
	assert(!m_shareBuffer && "Re-Config of a shared image. Is it intended?");

	if(m_shareBuffer || iSizeX != m_sizeX || iSizeY != m_sizeY || iDataType != m_dataType)
		Reset();

	m_sizeX = iSizeX;
	m_sizeY = iSizeY;
	m_dataType = iDataType;

	long imSize = GetDataSize();
	if (imSize == 0)
	{
		m_buffer.Reset();
		m_data = 0;
	}
	else if(m_data == 0 || imSize > 0)
	{
		m_buffer.Allocate(imSize, false, true);
		m_data = m_buffer.GetData();
	}

	/*
	m_minBoundValue = _minValuePerPixel();
	m_maxBoundValue = _maxValuePerPixel();

	m_minDataValue = m_minBoundValue;
	m_maxDataValue = m_maxBoundValue;
	*/
}



bool AqImage::Attach(const AqImage &iImage)
{
	Attach(iImage.GetData(), iImage.GetSizeX(), iImage.GetSizeY(), iImage.DataType());
	return true;
}

bool AqImage::Attach(char* iData, unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType)
{
	Reset();

	assert( ((unsigned int)iData % 16) == 0 && "Data needs to be 16-byte aligned");

	m_data     = iData;
	m_sizeX    = iSizeX;
	m_sizeY    = iSizeY;
	m_dataType = iDataType;

	m_shareBuffer = true;
	return true;
}

bool AqImage::SetPixelValue(unsigned short iX, unsigned short iY, short iValue) {
	if (iX > (m_sizeX-1) || iY > (m_sizeY-1)) 
		return false;

	unsigned short m_bpp = _bytesPerPixel();

	long m_offset = m_bpp*(iY*m_sizeX + iX);
	*((short *)(m_data + m_offset)) =  iValue;	
	return true;
}


short AqImage::GetPixelValue(unsigned short iX, unsigned short iY) const 
{
	// shiying hu, 2005-12-09
	// should throw exception here
	if (iX > (m_sizeX-1) || iY > (m_sizeY-1))
		return 0;
//		return m_minDataValue;

	unsigned short m_bpp = _bytesPerPixel();

	long m_offset = m_bpp*(iY*m_sizeX + iX);
	return *((short *)(m_data + m_offset));
}



// private
/*
void AqImage::ComputeDataRange()
{
	short iValue = 0;

	for (int i = 0; i< m_sizeX; i++) 
	{
		for (int j = 0; j< m_sizeY; j++) 
		{
			iValue = GetPixelValue(i, j);
			m_minDataValue = m_minDataValue > iValue ? iValue : m_minDataValue;
			m_maxDataValue = m_maxDataValue < iValue ? iValue : m_maxDataValue;
		}
	}

}
*/





//Add by YZ for debuging purpose
void AqImage::GetMinMaxValue( short& oMin, short& oMax )
{
	if (m_sizeX<=0 || m_sizeY<=0)
		return;

	unsigned short t_bpp = _bytesPerPixel();

	short value = 0;
	for(int iY=0;iY<m_sizeY;iY++)
		for(int iX=0;iX<m_sizeX;iX++)
		{
			long t_offset = t_bpp*(iY*m_sizeX + iX);
			value = *((short *)(m_data + t_offset));
			if(iX==0 && iY==0)
			{
				oMin = oMax = value;
			}
			if(value<oMin)
			{
				oMin = value;
			}
			if(value>oMax)
			{
				oMax = value;
			}
		}
}
