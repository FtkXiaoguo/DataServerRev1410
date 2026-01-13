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
 **** File       : AqMask.cpp									  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Implementation of the AqMask class.			  ****
 **** 															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 PC 12-12-2005 -- Rewrote methods to pack/unpack mask buffers based
 on GL's recent memory management changes.
	
 GL 12-09-2005 -- Make AqVolume as the super class.
 Remove duplicated member functions and variables that were 
 in AqVolume. Removed empty fuinctions such as And, Or, and 
 Not.														  

 PC 4/21/06 -- Method to pack/unpack from an AqVolume
 ********************************************************************/





// AqMask.cpp: implementation of the AqMask class.
//
//////////////////////////////////////////////////////////////////////

#include "AqMask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////

void AqMask::SetVoxelInfo(int	iVoxelSizeX, int iVoxelSizeY, unsigned int iBitsPerPixel) { 
	m_voxelSizeX = iVoxelSizeX; m_voxelSizeY = iVoxelSizeY; m_bitsPerPixel = iBitsPerPixel; 
};


	
bool AqMask::Pack(char *iUnpackedBuffer, 
				   int iUnPackedXSize,
				   int iUnPackedYSize,
				   int iUnPackedZSize,	
				   int iMaskId/*=0*/, 
				   bool	iReverseMask/*=0*/,
				   int iUnpackedMasksPerByte/*=8*/ 
				   ) {
	if ( !iUnpackedBuffer ) {
		assert(0);
		return false;
	}

	if ( iUnPackedXSize * iUnPackedYSize * iUnPackedZSize == 0 )
	{
		assert(0);
		return false;
	}

	// temporary
	if ( iUnPackedXSize % iUnpackedMasksPerByte != 0 )
		{
		assert(0);
		return false;
	}

	int unpackedSliceSize = iUnPackedXSize * iUnPackedYSize;

	m_voxelSizeX = iUnPackedXSize;
	m_voxelSizeY = iUnPackedYSize;
	m_bitsPerPixel = 1;

	int packedSizeX = iUnPackedXSize / iUnpackedMasksPerByte;
	int packedSizeY = iUnPackedYSize;
	AqImage* pImage;

	for ( int z = 0; z < iUnPackedZSize; z++)
	{
		if(z == 0)
			pImage = InitFirstImage(packedSizeX, packedSizeY, kAqVolumeDataTypeUnsignedChar);
		else
			pImage = GetImage();


		if (!_packSlice((iUnpackedBuffer + z*unpackedSliceSize), 
					unpackedSliceSize, 
					(char*)(pImage->GetData()),
					pImage->GetDataSize(), 
					iMaskId,
					iReverseMask,
					iUnpackedMasksPerByte) ) {				
				//if (AddImage(*pImage)) {
					assert(0);		
					return false;
				//}
		} else {
			// send message that buffer has been updated	
		}
	}
	
	return true;
}




// Prashant Chopra 4/21/06 -- Method to pack/unpack from an AqVolume
bool AqMask::Pack(AqVolume &iUnpackedMaskVolume, 				   
				   int iMaskId/*=0*/, 
				   int lastSliceIndex/*=-1*/,
				   bool	iReverseMask/*=0*/,
				   int iUnpackedMasksPerByte/*=8*/)
{
	if ( iUnpackedMaskVolume.GetSizeZ() <= 0 
		|| lastSliceIndex>(iUnpackedMaskVolume.GetSizeZ()-1) 
		|| lastSliceIndex < -1) {
		assert(0);
		return false;
	}

	lastSliceIndex = lastSliceIndex==-1 ? (iUnpackedMaskVolume.GetSizeZ()-1) : lastSliceIndex;


	int iUnPackedXSize = iUnpackedMaskVolume.GetSizeX(),
				   iUnPackedYSize = iUnpackedMaskVolume.GetSizeY(),
				   iUnPackedZSize = (lastSliceIndex+1);

	if ( iUnPackedXSize * iUnPackedYSize * iUnPackedZSize == 0 )
	{
		assert(0);
		return false;
	}

	// temporary
	if ( iUnPackedXSize % iUnpackedMasksPerByte != 0 )
		{
		assert(0);
		return false;
	}

	int unpackedSliceSize = iUnPackedXSize * iUnPackedYSize;

	m_voxelSizeX = iUnPackedXSize;
	m_voxelSizeY = iUnPackedYSize;
	m_bitsPerPixel = 1;

	int packedSizeX = iUnPackedXSize / iUnpackedMasksPerByte;
	int packedSizeY = iUnPackedYSize;
	AqImage* pImage;

	for ( int z = 0; z < iUnPackedZSize; z++)
	{
		if(z == 0)
			pImage = InitFirstImage(packedSizeX, packedSizeY, kAqVolumeDataTypeUnsignedChar);
		else
			pImage = GetImage();

		if (!pImage) {
			assert(0);
			return false;
		}

		AqImage *nextMaskImage = iUnpackedMaskVolume.GetImage(z);
		if (!nextMaskImage) {
			assert(0);
			return false;
		}

		if (!_packSlice(nextMaskImage->GetData(), 
					unpackedSliceSize, 
					(char*)(pImage->GetData()),
					pImage->GetDataSize(), 
					iMaskId,
					iReverseMask,
					iUnpackedMasksPerByte) ) {								
					assert(0);		
					return false;				
		} else {
			// send message that buffer has been updated	
		}
	}
	
	return true;
}


bool AqMask::Unpack(AqVolume &iUnpackedMaskVolume, 				   
				   int iMaskId/*=0*/, 
				   bool	iReverseMask/*=0*/,
				   int iUnpackedMasksPerByte/*=8*/)
{
	if (GetSizeZ() <=0) {
		// the first image is used to configure
		// a temporary container image so this test is needed
		return false;
	}

	
	int iUnPackedXSize = m_voxelSizeX;
	int iUnPackedYSize = m_voxelSizeY;
	int iUnPackedZSize = GetSizeZ();
		
	if ( iUnPackedXSize * iUnPackedYSize == 0 )
	{
		assert(0);
		return false;
	}

	// temporary
	if ( iUnpackedMasksPerByte>8 || iUnPackedXSize % iUnpackedMasksPerByte != 0 ){
			assert(0);		
			return false;
	}

	int packedXSize = iUnPackedXSize / iUnpackedMasksPerByte;
	int packedYSize = iUnPackedYSize;
	int packedSliceSize = packedXSize * packedYSize;
	int unpackedSliceSize = iUnPackedXSize * iUnPackedYSize;

	iUnpackedMaskVolume.Clear();

	AqImage *pImage;
	for ( int z = 0; z < iUnPackedZSize; z++)
	{		
		pImage = GetImage(z);
		
		if (!pImage) {
			assert(0);		
			return false;
		}

		AqImage *nextMaskImage = 0;
		if(z == 0)
			nextMaskImage = iUnpackedMaskVolume.InitFirstImage(iUnPackedXSize, iUnPackedYSize, kAqVolumeDataTypeUnsignedChar);
		else
			nextMaskImage = iUnpackedMaskVolume.GetImage();

		if (!_unpackSlice(nextMaskImage->GetData(), 
					 unpackedSliceSize, 
					 (char*)(pImage->GetData()),
					 pImage->GetDataSize(), 
					 iMaskId,					 
					 iUnpackedMasksPerByte)) {			
						assert(0);		
						return false;
		}
	
	}	
	return true;
}
// Prashant Chopra 4/21/06 -- Method to pack/unpack from an AqVolume


// PrashantChopra 4/14/06 - Specific for WS
bool AqMask::UnpackDownsampled(char* opUnPackedBuffer,
		   const int iUnPackedXSize,
		   const int iUnPackedYSize,
		   const int iUnPackedZSize,		   
		   const int iMaskId/*=0*/,
		   int iUnpackedMasksPerByte/*=8*/)
{
	if ( !opUnPackedBuffer )
		return false;

	if ( iUnPackedXSize * iUnPackedYSize * iUnPackedZSize == 0 )
		return false;

	// temporary
	if ( iUnpackedMasksPerByte>8 || iUnPackedXSize % iUnpackedMasksPerByte != 0 ){
			assert(0);		
			return false;
	}


	if (GetSizeZ() <=0) {
		// the first image is used to configure
		// a temporary container image so this test is needed
		return false;
	}

	// only downsample..no upsample
	if ( iUnPackedZSize >= GetSizeZ() )
		return false;
	// only downsample..no upsample


	int packedXSize = iUnPackedXSize / 8;
	int packedYSize = iUnPackedYSize;
	int packedSliceSize = packedXSize * packedYSize;
	int unpackedSliceSize = iUnPackedXSize * iUnPackedYSize;

	float skipFactor =  ((float)GetSizeZ()/(float)iUnPackedZSize);

	// create a dummy image
	AqImage *pImageSrc = GetImage(0);
	if (!pImageSrc) {
		return false;
	}

	
	AqImage pImage(pImageSrc->GetSizeX(), pImageSrc->GetSizeY(), pImageSrc->DataType());
	

	// start from the first interval
	float iImagePosition = 0;

	for ( int z = 0; z < iUnPackedZSize; z++)	
	{
		// Nov 22, 2005 Kimitake Abe(5421)
		if (!GetInterpolatedImage(iImagePosition, &pImage)) {
			return false;
		}

		iImagePosition+=skipFactor;	

		char * pPackedBuffer = (char *)(pImage.GetData());
		if ( !pPackedBuffer ) 
				return false;					
		
		if (!_unpackSlice((opUnPackedBuffer + z*unpackedSliceSize), 
					 unpackedSliceSize, 
					 pPackedBuffer,
					 packedSliceSize, 					 
					 iMaskId,					 
					 iUnpackedMasksPerByte)) {			
						assert(0);		
						return false;
		}		
		
	}

	return true;
}
// PrashantChopra 4/14/06 - Specific for WS



bool AqMask::Unpack(char *oUnpackedBuffer, 
				   int iUnPackedXSize,
				   int iUnPackedYSize,
				   int iUnPackedZSize,	
				   int iMaskId/*=0*/, 
				   int iUnpackedMasksPerByte/*=8*/ ) {
	if ( !oUnpackedBuffer )	{
			assert(0);		
			return false;
	}

	if ( iUnPackedXSize * iUnPackedYSize * iUnPackedZSize == 0) {
			assert(0);		
			return false;
	}

	// temporary
	if ( iUnpackedMasksPerByte>8 || iUnPackedXSize % iUnpackedMasksPerByte != 0 ){
			assert(0);		
			return false;
	}

	int packedXSize = iUnPackedXSize / iUnpackedMasksPerByte;
	int packedYSize = iUnPackedYSize;
	int packedSliceSize = packedXSize * packedYSize;
	int unpackedSliceSize = iUnPackedXSize * iUnPackedYSize;

	AqImage *pImage;
	for ( int z = 0; z < iUnPackedZSize; z++)
	{		
		pImage = GetImage(z);
		if (!pImage) {
			assert(0);		
			return false;
		}

		if (!_unpackSlice((oUnpackedBuffer + z*unpackedSliceSize), 
					 unpackedSliceSize, 
					 (char*)(pImage->GetData()),
					 pImage->GetDataSize(), 
					 iMaskId,					 
					 iUnpackedMasksPerByte)) {			
						assert(0);		
						return false;
		}
	
	}

	return true;
}


bool AqMask::Set(int iX, int iY, int iZ, unsigned int iValue/*=1*/) 
{
	// get offset into the volume 
	int originalPixelOffset = iY*m_voxelSizeX + iX;
	int packedPixelOffset = ((m_bitsPerPixel*originalPixelOffset)/kAqBitsPerByte);

	// get whole byte
	unsigned char pByte = (unsigned char)GetVoxelValue(packedPixelOffset, 0, iZ);

	// create new byte to and with
	// create intra-byte offset
	char pIntraByteOffset = originalPixelOffset%kAqBitsPerByte;
	unsigned char pMaskByte = 1;
	pMaskByte = pMaskByte<<(kAqBitsPerByte-pIntraByteOffset-1);

	pByte = pByte|pMaskByte;

	// put it
	return (SetVoxelValue(packedPixelOffset, 0, iZ, (short)pByte) == kAqSuccess);	
}

bool AqMask::UnSet(int iX, int iY, int iZ) {
int originalPixelOffset = iY*m_voxelSizeX + iX;

	
	
	int packedPixelOffset = ((m_bitsPerPixel*originalPixelOffset)/kAqBitsPerByte);;//(int)floor((double)originalPixelOffset*m_pByteConversionFactor);

	// get whole byte
	unsigned char pByte = (unsigned char)GetVoxelValue(packedPixelOffset, 0, iZ);

	// create new byte to and with
	// create intra-byte offset
	char pIntraByteOffset = originalPixelOffset%kAqBitsPerByte;
	unsigned char pMaskByte = 1;
	pMaskByte = pMaskByte<<(kAqBitsPerByte-pIntraByteOffset-1);
	pMaskByte = ~pMaskByte;

	pByte = pByte&pMaskByte;	
	// put it
	return (SetVoxelValue(packedPixelOffset, 0, iZ, (short)pByte) == kAqSuccess);	
}

bool AqMask::IsSet(int iX, int iY, int iZ) {
		// get offset into the volume 
	int originalPixelOffset = iY*m_voxelSizeX + iX;

	
	
	int packedPixelOffset = ((m_bitsPerPixel*originalPixelOffset)/kAqBitsPerByte);;//(int)floor((double)originalPixelOffset*m_pByteConversionFactor);

	// get whole byte
	unsigned char pByte = (unsigned char)GetVoxelValue(packedPixelOffset, 0, iZ);

	// create new byte to and with
	// create intra-byte offset
	char pIntraByteOffset = originalPixelOffset%kAqBitsPerByte;

	unsigned char pMaskByte = 1;
	pMaskByte = pMaskByte<<(kAqBitsPerByte-pIntraByteOffset-1);

	pByte = pByte&pMaskByte;
	
	return (pByte > 0);	
}


// private methods
bool AqMask::_packSlice(char *iUnpackedSlice, int iUnPackedSliceSize, 
					char *oPackedSlice, int iPackedSliceSize, int iMaskId/*=0*/, 
					bool iReverseMask/*=0*/,int iUnpackedMasksPerByte/*=8*/) 
{

	if ( !iUnpackedSlice || !oPackedSlice || iUnpackedMasksPerByte>8 || iMaskId<0 || iMaskId>(iUnpackedMasksPerByte-1)) {
		assert(0);
		return false;
	}

	// Shiying Hu, 2006-02-02,
	// should initialize packed buffer.
	// do not know when this is commented out.
	memset(oPackedSlice, 0, iPackedSliceSize);
	
	static unsigned char bits[] = {128, 64, 32, 16, 8, 4, 2, 1};

	unsigned char maskdsts =  (1<<iMaskId);

	for (int i = 0; i < iUnPackedSliceSize; ++i)
	{
		// modified by shiying hu, 2006-01-26
		unsigned char flag = false;
		flag = (iUnpackedSlice[i] & maskdsts);

		if ( (flag && !iReverseMask) || (!flag && iReverseMask))
		{
			int bOff = 7-(i%iUnpackedMasksPerByte);
			unsigned char flagByte = (1<<bOff);
			// set i'th bit in packed mask buffer
			// find the byte to set : oPackedMaskBuffer[i/8]

			// now OR it with teh flag byte
			// create flagbyte 
			
			//oPackedSlice[i/iUnpackedMasksPerByte] |= bits[i%iUnpackedMasksPerByte];
			oPackedSlice[i/iUnpackedMasksPerByte] |= flagByte;
		}

	}
	return true;
}


bool AqMask::_unpackSlice(char *oUnpackedSlice, int iUnPackedSliceSize, 
					char *iPackedSlice, int iPackedSliceSize, int iMaskId/*=0*/, 
					int iUnpackedMasksPerByte/*=8*/) {
	if ( !oUnpackedSlice || !iPackedSlice || iUnpackedMasksPerByte>8 || iMaskId<0 || iMaskId>(iUnpackedMasksPerByte-1)) {
		assert(0);
		return false;
	}
	
	static unsigned char bits[] = {128, 64, 32, 16, 8, 4, 2, 1};

	unsigned char maskdsts =  (1<<iMaskId);

	for (int i = 0; i < iUnPackedSliceSize; ++i)
	{
		// parse the compressed buffer
		int bOff = 7-(i%iUnpackedMasksPerByte);
		unsigned char flagByte = (1<<bOff);
		// set i'th bit in packed mask buffer
		// find the byte to set : oPackedMaskBuffer[i/8]
		// now OR it with teh flag byte
		// create flagbyte 
				
		//oPackedMaskBuffer[i/8] |= bits[i&(iMaskId)];
		// Nov 20, 2005 Kimitake Abe(5421)
		oUnpackedSlice[i] &= ~maskdsts;
		if (iPackedSlice[i/iUnpackedMasksPerByte] & flagByte) {
			oUnpackedSlice[i] |= maskdsts;
		}
		// Abe(5421)
	}
	return true;
}


