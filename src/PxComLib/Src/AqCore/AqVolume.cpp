
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
 **** File       : AqVolume.cpp									  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Definition of AqVolume class.				  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqVolume.cpp: implementation of the AqVolume class.
//
//////////////////////////////////////////////////////////////////////

#include "AqVolume.h"


//GL 12-9-2005  
// The change is too big to use diff. Please read old and new read codes for verification 

// DavidG 4-28-2006 Allow sharing data buffer

const AqVolume& AqVolume::operator =(const AqVolume& iVolume) 
{
	Clear();

	/*
	m_minDataValue = iVolume.m_minDataValue;
	m_maxDataValue = iVolume.m_maxDataValue;
	*/

	int imageN = iVolume.m_images.size();
	if(imageN > 0)
	{
		for(int i=0;i<imageN; i++)
		{
			const AqImage* im = iVolume.m_images[i];
			AddImage(*im);
		}
	}
	
	return *this;
}

/*
bool AqVolume::UpdateInfo(const AqImage& iImage)
{
	int imageN = m_images.size();
	if (imageN == 0)
		return false;

	if (m_images[0]->GetSizeX() != iImage.GetSizeX() || 
			m_images[0]->GetSizeY() != iImage.GetSizeY() || 
			m_images[0]->DataType() != iImage.DataType() 
		)
		return false;

	short imgMin = iImage.MinDataValue(), imgMax = iImage.MaxDataValue();

	m_minDataValue	= imgMin < m_minDataValue ? imgMin : m_minDataValue;
	m_maxDataValue  = imgMax > m_maxDataValue ? imgMax : m_maxDataValue;
	return true;
}
*/

bool AqVolume::AddImage(const AqImage& iImage) 
{	
	// modified by shiying hu, 2005-12-09

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

AqImage* AqVolume::InitFirstImage(unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType)
{
	if(m_images.size() > 0)
		Clear();

	AqImage* pImage = new AqImage(iSizeX, iSizeY, iDataType);
	m_images.push_back(pImage);
	
	//m_minDataValue	= m_maxDataValue  = 0;
	return pImage;
}

AqImage *AqVolume::GetImage(int iImageIndex)
{
	int imageN = m_images.size();
	if ( (iImageIndex < 0 && imageN == 0) || iImageIndex >= imageN) 
		return 0;

	if ( iImageIndex < 0)
	{
		if(iImageIndex == -1)
		{
			AqImage* iniImage = m_images[0];
			assert( iniImage );

			AqImage* newImage = new AqImage(iniImage->GetSizeX(), iniImage->GetSizeY(), iniImage->DataType());
			m_images.push_back(newImage);
			return newImage;
		}
		else
		{
			return 0;
		}
	}

	return m_images[iImageIndex];
}

AqImage* AqVolume::AttachImage(char* iData, unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType)
{
	if(m_images.size() > 0)
		Clear();

	AqImage* pImage = new AqImage();
	pImage->Attach(iData, iSizeX, iSizeY, iDataType);
	m_images.push_back(pImage);
	
	//m_minDataValue	= m_maxDataValue  = 0;
	return pImage;
}

AqImage* AqVolume::AttachImage(AqImage& iImage)
{
	return AttachImage(iImage.GetData(), iImage.GetSizeX(), iImage.GetSizeY(), iImage.DataType());
}

bool AqVolume::SetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ, short iValue) 
{
	if (iZ >= m_images.size()) 
	{
		// ideally, this should return min value for the specified data type
		return false;
	}

	return  
		m_images[iZ]->SetPixelValue(iX, iY, iValue);
}


short AqVolume::GetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ) const
{
	if (iZ >= m_images.size()) 
	{
		// ideally, this should return min value for the specified data type
		return 0;
	}

	return m_images[iZ]->GetPixelValue(iX, iY);
}


void AqVolume::Clear() 
{
	int imageN = m_images.size();
	if(imageN)
	{
		for (int i=0; i<imageN; i++)
			delete (AqImage*)(m_images[i]);

		m_images.clear(); 
//		m_maxDataValue=m_minDataValue=0; 
	}
};



// 3/29/06 Prashant Chopra Interpolate between images
bool AqVolume::GetInterpolatedImage(const float iImagePosition, AqImage *oImage) const
{

	if ( iImagePosition < 0 || iImagePosition > (m_images.size()-1)) {
		//		throw exception
		return false;
	}


	int m_sizeX = m_images[0]->GetSizeX(),
		m_sizeY = m_images[0]->GetSizeY();


	if (! (oImage->DataType() == kAqVolumeDataTypeUnsignedChar ||
			oImage->DataType() == kAqVolumeDataTypeChar) ) {
		assert(0 && "Data type has to be char or uchar!"); 
		return false;
	}
	
	if (!oImage || oImage->GetSizeX()!=m_sizeX || oImage->GetSizeY()!=m_sizeY || 
			m_images[0]->DataType() != oImage->DataType())  {
		return false;
	}

	// boundary -- special case		
	if ((iImagePosition==0.0 || iImagePosition==m_images.size()-1)) {
		// copy the boundary image
		memcpy(oImage->GetData(), m_images[(int)iImagePosition]->GetData(), m_sizeX*m_sizeY*sizeof(unsigned char));
		return true;
	}

	int indexAbove = (int)ceil(iImagePosition);
	int indexBelow = (int)floor(iImagePosition);
	float distFromLowerImage = iImagePosition - (float)indexBelow;


	// fetch a slice above
	assert(m_images[indexAbove]);
	unsigned char *pAbove	= (unsigned char*)m_images[indexAbove]->GetData();
	assert(pAbove);

	// fetch a slice below
	assert(m_images[indexBelow]);
	unsigned char *pBelow	= (unsigned char*)m_images[indexBelow]->GetData();
	assert(pBelow);

	unsigned char *pOut		= (unsigned char*)oImage->GetData();
	assert(pOut);


	float newValue = 0.0;	
	unsigned char upperMaskByte=0, lowerMaskByte=0;

	for (long imageLength = 0; imageLength<m_sizeX*m_sizeY; imageLength++) {
		// linearly interpolate
		upperMaskByte = *pAbove++;
		lowerMaskByte = *pBelow++;

		*pOut = upperMaskByte;
		*pOut |= lowerMaskByte;
		*pOut++;
	}	

	return true;
}
// 3/29/06 Prashant Chopra Interpolate between images
