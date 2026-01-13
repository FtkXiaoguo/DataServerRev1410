
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
 **** File       : AqVolume.h									  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface for AqVolume class.				  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/

// AqVolume.h: interface for the AqVolume class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _AQVOLUME_H_
#define _AQVOLUME_H_
//////////////////////////////////////////////////////////////////////////

#include "AqImage.h"
#include <vector>

//GL 12-9-2005  remove super class AqObjectBase to use in place memory management
// removed	eAqReturnType AddImage(char *iDataPointer, unsigned short iSizeX, ...
// to avoid takeover outside buffer ownership.
// The AddImage is changed to copy input data to internal buffer
// InitFirstImage is use to create the first image, and GetImage is use to create new image
// The cahnge is too big to use diff. Please read old and new codes for verification 

// Shiying 12-9-2005 remove min and max data value
// because it is hard to manage these values because we give data buffer to user
// could provide method to calculate on the fly late.

// DavidG 4-28-2006 Allow sharing data buffer

class AqVolume
{
public:
//	AqVolume(): m_minDataValue(0), m_maxDataValue(0) {};	
	AqVolume() {}
	virtual ~AqVolume() { Clear(); }
	
	bool AddImage(const AqImage& iImage);

	AqImage* InitFirstImage(unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType);

	// same as InitFirstImage, but share data instead
	AqImage*  AttachImage(char* iData, unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType);
	AqImage*  AttachImage(AqImage& iImage);  

	AqImage* GetImage(int iImageIndex = -1); // default to get a new image, if want to share data use AddImage instead

	bool IsEmpty() const { return m_images.empty(); }

	// This function is troublesome. shiying hu.
	// Intention is first call GetImage and fill 
	//bool UpdateInfo(const AqImage&);

	bool SetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ, short iValue);
	short GetVoxelValue(unsigned short iX, unsigned short iY, unsigned short iZ) const ;

	inline unsigned short GetSizeX() const { return (m_images.size() > 0)?m_images[0]->GetSizeX():0;}
	inline unsigned short GetSizeY() const { return (m_images.size() > 0)?m_images[0]->GetSizeY():0;}
	inline unsigned short GetSizeZ() const {return	m_images.size();}

	/*
	inline short MinDataValue() const {	return m_minDataValue;}
	inline short MaxDataValue() const {	return m_maxDataValue;}
	*/

	// Prashant Chopra 4/14/06 -- To interpolate (downsample) for AqMask images
	bool GetInterpolatedImage(const float iImagePosition, AqImage *oImage) const;
	// Prashant Chopra 4/14/06 -- To interpolate (downsample) for AqMask images

	void Clear();

protected:
	//GL 12-9-2005  removed following image information. Using information from first image instead

	//unsigned short			m_sizeX;
	//unsigned short			m_sizeY;

	//eAqVolumeDataType		m_dataType;

	std::vector<AqImage*>	m_images;

	/*
	short					m_minDataValue;
	short					m_maxDataValue;
	*/

	// Do not use this method, use Copy method instead
	AqVolume(const AqVolume& iVolume) { *this = iVolume; } // shallow copy
	const AqVolume &operator =(const AqVolume&); // shallow copy
};


//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQVOLUME_H_
