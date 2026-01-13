
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
 **** File       : AqImage.h									  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface for AqImage class.					  ****
 ****															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 ********************************************************************/
// AqImage.h: interface for the AqImage class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _AQIMAGE_H_
#define _AQIMAGE_H_
//////////////////////////////////////////////////////////////////////////
#include "AqBaseConstants.h"

//GL 12-9-2005  remove super class AqObjectBase to use in place memory management
// Add Config,  Reset, GetDataSize methods for memory management
// the AqImage::AqImage(char *iDataPointer, unsigned short iSizeX, unsigned short iSizeY, ...
// is removed to favor using Config for setting up buffer and GetData to using the buffer
// After the changes, AqImage never take out side buffer ownership. 


// Shiying 12-9-2005 remove min and max data value
// because it is hard to manage these values because we give data buffer to user
// could provide method to calculate on the fly late.

// DavidG: 4-28-2006 Allow sharing data buffer
// DavidG: 6-28-2006 Memory alignment, copy methods

class AqImage
{
public:
	AqImage();
	AqImage(unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType);

	virtual ~AqImage();

	// If iImage is shared, this will be shared.
	// If iImage is not shared, this will not be shared (have its own buffer).
	void Copy(const AqImage& iImage);

	// this will always have its own buffer, no matter what iImage is.
	void DeepCopy(const AqImage& iImage);
	
	void Config(unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType);

	inline bool IsSharingData() const { return m_shareBuffer; }
	bool        Attach(const AqImage &iImage);
	bool        Attach(char* iData, unsigned short iSizeX, unsigned short iSizeY, eAqVolumeDataType iDataType);

	inline char* GetData() const     { return m_data; }
	inline int   GetDataSize() const { return (int)m_sizeX*m_sizeY*_bytesPerPixel(); }
	
	//void ComputeDataRange();

	bool IsInside(unsigned short iX, unsigned short iY) const { return (iX < m_sizeX) && (iY < m_sizeY); }

	bool  SetPixelValue(unsigned short iX, unsigned short iY, short iValue);
	short GetPixelValue(unsigned short iX, unsigned short iY) const;
	
	inline unsigned short GetSizeX() const { return m_sizeX; }
	inline unsigned short GetSizeY() const { return m_sizeY; }

	/*
	inline short MinDataValue() const {	return m_minDataValue;}
	inline short MaxDataValue() const {	return m_maxDataValue;}
	*/

	inline eAqVolumeDataType DataType () const     { return m_dataType; }
	inline unsigned short    BytesPerPixel() const { return _bytesPerPixel(); }
	
	void GetMinMaxValue( short& oMin, short& oMax );
	
protected:
	AqBuffer            m_buffer;
	char 				*m_data;
	unsigned short		m_sizeX;
	unsigned short		m_sizeY;

	eAqVolumeDataType	m_dataType;

	bool                m_shareBuffer;

	// allow access to m_data for efficiency
	friend class AqBitImage;

	/*
	short				m_minBoundValue;
	short				m_maxBoundValue;

	short				m_minDataValue;
	short				m_maxDataValue;
	*/

	// Do not use this method, use Copy method instead
	AqImage(const AqImage &iImage);
	const AqImage &operator =(const AqImage&);
	//

	void Reset();

	inline unsigned short  _bytesPerPixel() const {
		unsigned short bytesPerPixel = sizeof(unsigned short);

		switch (m_dataType) {
			case kAqVolumeDataTypeUnsignedChar:		bytesPerPixel = sizeof(unsigned char); break;
			case kAqVolumeDataTypeChar:				bytesPerPixel = sizeof(char); break;

			case kAqVolumeDataTypeUnsignedBpp12:	bytesPerPixel = sizeof(unsigned short); break;
			case kAqVolumeDataTypeBpp12:			bytesPerPixel = sizeof(short); break;		

			case kAqVolumeDataTypeUnsignedShort:	bytesPerPixel = sizeof(unsigned short); break;
			case kAqVolumeDataTypeShort:			bytesPerPixel = sizeof(short); break;
			default					   :			break;
		}

		return bytesPerPixel;
	};

	/*
	inline int _minValuePerPixel() const {
		int minValuePerPixel = 0;

		switch (m_dataType) {
			case kAqVolumeDataTypeUnsignedChar:		minValuePerPixel = 0; break;
			case kAqVolumeDataTypeChar:				minValuePerPixel = -127; break;

			case kAqVolumeDataTypeUnsignedBpp12:	minValuePerPixel = 0; break;
			case kAqVolumeDataTypeBpp12:			minValuePerPixel = -2047; break;			
			
			case kAqVolumeDataTypeUnsignedShort:	minValuePerPixel = 0; break;
			case kAqVolumeDataTypeShort:			minValuePerPixel = -32767; break;

			default					   :			break;
		}

		return minValuePerPixel;
	};

	inline int _maxValuePerPixel() const {
		int maxValuePerPixel = 32767;

		switch (m_dataType) {
			case kAqVolumeDataTypeUnsignedChar:		maxValuePerPixel = 255; break;
			case kAqVolumeDataTypeChar:				maxValuePerPixel = 127; break;

			case kAqVolumeDataTypeUnsignedBpp12:	maxValuePerPixel = 4095; break;
			case kAqVolumeDataTypeBpp12:			maxValuePerPixel = 2047; break;			
			

			case kAqVolumeDataTypeUnsignedShort:	maxValuePerPixel = 65535; break;
			case kAqVolumeDataTypeShort:			maxValuePerPixel = 32767; break;
			default					   :			break;
		}

		return maxValuePerPixel;
	};
	*/

	

};

//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _AQIMAGE_H_
