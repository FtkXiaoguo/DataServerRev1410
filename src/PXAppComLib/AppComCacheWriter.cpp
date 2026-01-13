/***********************************************************************
 * AppComCacheWriter.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This file implements the member functions of the 
 *	    TerareconCache Object.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

#pragma warning (disable: 4786)

//#include <CRTDBG.H>

#include <math.h>
#include "AqCore/TRPlatform.h"
#include "RTVSliceInformation.h"
#include "diskspacemanager.h"
#include "AppComCacheWriter.h"
#include "AppComDataConversion.h"

const int kSizeOfHistogram = 4096;
#define Stream_Size 4096*4*2

#ifdef CacheWriterVerbose
static TRCriticalSection cs;
#endif

#if !defined(TICACHE_H)
#include "TICache.h"
#endif

#include "PETObjectAttributes.h"

//-----------------------------------------------------------------------------

int AppComCacheWriter::CloseCache(const char* iCacheLocation)
{
	AppComCacheWriter cache;
	if(iCacheLocation == NULL || strlen(iCacheLocation) == 0)
		return kSuccess; //clsoe noting should be treated success
	
	// only do setup instead of open to avoid openning the closed files.
	cache.Setup(iCacheLocation, kCacheDataFileName, kCacheDataFileSize, 
						  kCacheDescriptionFileName, kCacheDescriptionFileSize);
	return cache.Close();
}

bool AppComCacheWriter::HasCache(const char* iCacheLocation)
{
	AppComCacheWriter cache;
	if(iCacheLocation == NULL || strlen(iCacheLocation) == 0)
		return false;
	
	// only do setup instead of open to avoid openning the closed files.
	cache.Setup(iCacheLocation, kCacheDataFileName, kCacheDataFileSize, 
						  kCacheDescriptionFileName, kCacheDescriptionFileSize);

	return (access(cache.m_dataFilePath, 0) == 0 && access(cache.m_descFilePath, 0)== 0);
}


// 03/14/2003 -- temporary control for modalityLUT usage
//
int AppComCacheWriter::m_useModalityLUTForCR = 0;
void AppComCacheWriter::SetUseModalityLUT(int iYN)
{
	m_useModalityLUTForCR = iYN;
}


//-----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/* 08/04/2002 --
 * Instead of converting the data inline, we use a converter
 */
AppComCacheWriter::AppComCacheWriter()
{
//	m_dataConverter = new AppComDataConverter;
	m_dataConverter = 0;
	m_externalConverter = 0;
	m_dataFileSize = 0;
}


//-----------------------------------------------------------------------------
AppComCacheWriter::~AppComCacheWriter()
{
	if (m_dataConverter && !m_externalConverter)
		delete m_dataConverter;
	
	m_dataConverter = 0;
	m_dataFileSize = 0;
}
 
// create or attach to the serial cache 
int AppComCacheWriter::SetCache(const char* iCacheLocation, bool big)
{
	int status = kSuccess;
	if(iCacheLocation == NULL || strlen(iCacheLocation) == 0)
	{
		Reset();
		return kErrBadInputParameters;
	}
	else
	{
		if(m_cacheLocation.IsEmpty() || m_cacheLocation != iCacheLocation)
		{
			if(big) {
				if (m_dataFileSize <= 0) m_dataFileSize = kCacheDataFileSizeBig;
			
			}
			else {
				if (m_dataFileSize <= 0) m_dataFileSize = kCacheDataFileSize;
			}
			status = Setup(iCacheLocation, kCacheDataFileName, m_dataFileSize, 
						  kCacheDescriptionFileName, kCacheDescriptionFileSize);

		}
		if( status != kSuccess) return status;
	}
	return Open();

}

//-----------------------------------------------------------------------------------------
bool AppComCacheWriter::ReserveSpace()
{
	int rSpace = (m_descSize+m_dataSize)/(1000*1024);
	return RTVDiskSpaceManager::MakeMediaSpace(rSpace, std::string(m_cacheLocation));
}

//-----------------------------------------------------------------------------------------
AppComDataConverter* AppComCacheWriter::SetDataConverter(AppComDataConverter * iConverter, int iIsExternal)
{
	AppComDataConverter * old = m_dataConverter;
	if ( iConverter )
	{
		m_dataConverter = iConverter;
		m_externalConverter = iIsExternal;
	}
	return m_dataConverter;
}

//-----------------------------------------------------------------------------------------
//
//	Convert an image from [r1r2r3... g1g2g3... b1b2b3...] to [r1g1b1 r2g2b2 r3g3b3 ...]
//
int AppComCacheWriter::InterleaveColorPlanes(unsigned char* ioPixels, int iRows, int iColumns)
{
	unsigned long nPixels = iRows * iColumns;
	unsigned char* newPixels = new unsigned char[nPixels*3];

	unsigned char* red   = ioPixels;
	unsigned char* green = ioPixels + nPixels;
	unsigned char* blue  = ioPixels + (2*nPixels);

	int i=0;
	for(int j=0; j<nPixels;j++)
	{
		newPixels[i++] = red[j];
		newPixels[i++] = green[j];
		newPixels[i++] = blue[j];
	}

	memcpy(ioPixels, newPixels, nPixels*3);
	delete[] newPixels;
	return kSuccess;
}

//-----------------------------------------------------------------------------
int AppComCacheWriter::AddSCImage (const char* iCacheLocation,
				                      const char* iModality,
				                      const char* iSOPInstanceUID,
						              const char* iSOPClassUID,

				                      int iBitsAllocated,
				                      int iBitsStored,
				                      int iHighBit,
				                        
				                      int iSizeX, 
				                      int iSizeY,  

				                      bool  iIsLittleEndian,

				                      std::vector <std::string>& iImageTypeTokens,

				                      unsigned char*     iPixels,

				                      int iSamplesPerPixel,
				                      ePhotometricInterpretation iPhotometricInterpretation,
				                      ePlanarConfiguration iPlanarConfiguration,
                                      eCompressionMethod iCompressionMethod,
									  int iWindowWidth,
									  int iWindowCenter,
									  // -- 2004.08.27 
									  // we need instance number for batch2D
									  int iInstanceNumber)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddSCImage(" << iCacheLocation <<")\n";
cs.Leave();
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	// make sure cacahe is avaliable
	// because cache can be closed by anyone at any time
	int status = SetCache(iCacheLocation);
	if( status != kSuccess) return status;
	
	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}

/*
	if (iPhotometricInterpretation != kMONOCHROME2)
	{
		return kErrFormatNotSupported;
	}

	if (iSamplesPerPixel > 1) 
	{
		return kErrFormatNotSupported;
	}
*/
	int bytesPerPixel = (iBitsAllocated + 7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;
	unsigned long maxVal = (unsigned long) (pow(2.,(double)iBitsStored)) - 1;


	unsigned char* pxls = iPixels;
	unsigned char pixel;

	int minVoxelValue = 0;
	int maxVoxelValue = 0;

    if (bytesPerPixel * iSamplesPerPixel > 2) 
	{
         if (iPlanarConfiguration != kRGBRGB)
        {
			InterleaveColorPlanes(pxls, iSizeX, iSizeY);
        }
		pxls = iPixels;
	}
    else
    {

	    if (bytesPerPixel == 1)
	    {
		    for (int i = 0; i < iSizeX * iSizeY; i++)
		    {
				pixel = pxls[i]<<bitsToShift;
				pxls[i] = (pixel > maxVal)?maxVal:pixel; //it is unsigned char, no under cut
				//pxls[i] = clamp((unsigned char)(pxls[i]<<bitsToShift), (unsigned char) 0, (unsigned char)maxVal);
		    }
	    } 
		else if (bytesPerPixel == 2)
	    {
			int totalSize = iSizeX*iSizeY;

		

			if (IsModalityMR(std::string (iModality)) || IsModalityPT(std::string (iModality)))
			{
				// -- JUN-26-2003  CacheWriter does not convert for MR
				m_dataConverter->FindMinMax((short *)iPixels, totalSize, iIsLittleEndian,
				                                  minVoxelValue, maxVoxelValue);
			//	m_dataConverter->ConvertMRToVoxel((short *)iPixels, totalSize, iIsLittleEndian,
			//	                                  minVoxelValue, maxVoxelValue);
			}
			// -- 2005.02.28
			// For SC, we don't need to convert the data using the slope&intercept. The only
			// thing we need to do is shift the pixels if necessary
			else if (IsSOPClassUIDSC(std::string(iSOPClassUID)))
			{
				if (bitsToShift > 0)
				{
					unsigned short * pp = (unsigned short *)iPixels;
					for (int i = iSizeX * iSizeY; --i >=0; )
					{
						pp[i] = (pp[i]>>bitsToShift);
					}
				}
			}
			else if (IsModalityCT(std::string (iModality)))
			{
				m_dataConverter->ConvertToVoxel((unsigned short *)iPixels, totalSize, 
			                            1.0,0, iIsLittleEndian != 0);
			}
	    }
    }

	ULONGLONG fp = 0;
	unsigned long sizeOfDataInBytes = bytesPerPixel * iSamplesPerPixel * iSizeX * iSizeY;
	// Write the Data Cache first in one action to make sure it is in one block
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	
	if (status != kSuccess) return status;

	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, iBitsStored,
						           iHighBit, iSizeX, iSizeY, fp, sizeOfDataInBytes, iImageTypeTokens, kNone,
								   iWindowWidth, iWindowCenter);

	if (status != kSuccess) return status;
	
	// SC Image specific strings
	ost << kSamplesPerPixelStr << " = "       << iSamplesPerPixel       << "\n";
	ost << kPhotometricInterpretationStr << " = " << iPhotometricInterpretation << "\n";

	if (bytesPerPixel == 2 && IsModalityMR(std::string (iModality)))
	{
		ost << kSliceMinVoxelValueStr << " = " << minVoxelValue << "\n";
		ost << kSliceMaxVoxelValueStr << " = " << maxVoxelValue << "\n";
	}

	// -- 2004.08.27
	// need this for batch2D
	ost << kImageNumberStr << " = " << iInstanceNumber << "\n";

	if (iSamplesPerPixel > 1)
		ost << kPlanarConfigurationStr << " = " << iPlanarConfiguration << "\n";
		
	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	// Write the description in one action to make sure it is in one block
	status = WriteDesc(ost);
		
	return status;
}

//-----------------------------------------------------------------------------
// Add a single CT image to Cache
int AppComCacheWriter::AddCTImage (const char* iCacheLocation,
									  const char* iModality,
						              const char* iSOPInstanceUID,
                                      const char* iSOPClassUID,

									  int iBitsAllocated,
						              int iBitsStored,
						              int iHighBit,
						
						              int iSizeX, 
						              int iSizeY,  

						              bool  iIsLittleEndian,

						              std::vector <std::string>& iImageTypeTokens,

						              double iImagePosition[3], 
						              double iImageOrientation[6], 
						              double iPixelSpacing[2],
									  double iSliceThickness,
						
						              double iRescaleSlope,
						              double iRescaleIntercept,
									  int	 iImageNumber,
									  int	 iPixelRepresentation,

						              unsigned char*     iPixels,
									  unsigned int		 iSizeOfCompressedData,
						              eCompressionMethod iCompressionMethod,
									  int iWindowWidth,
									  int iWindowCenter,
									  const char* imageDate,
									  const char* imageTime,
									  const char* scanOptions,
									  const char* manufacturer)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddCTImage(" << iCacheLocation <<")\n";
cs.Leave();	
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	// make sure cacahe is avaliable
	int status = SetCache(iCacheLocation);
	if( status != kSuccess) return status;

#if 0
	// test overhead testing
	ULONGLONG Ofp = 0;
	// Write the Data Cache First
	status = WriteData(iPixels, 0, Ofp);	
	char sbuf0[Stream_Size];
	ostrstream ost0(sbuf0, Stream_Size);
	status = WriteDesc(ost0);
	return status;
#endif
	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}

	// This is the number of bytes per pixel
	int bytesPerPixel = (iBitsAllocated+7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;


	unsigned int histogram[kSizeOfHistogram];
	unsigned int sizeOfHistogram = kSizeOfHistogram;

    int i = 0;
    int totalSize = iSizeX*iSizeY;
	// Swap the pixels and take care of calculating the histogram
	 
	// -- 2004.09.09
	// Optional use of 1024 for all CT offsets. If we change this,
	// we need to make corresponding changes in the thinclient
	int use1024 = 0;
	int minPixelValue = 0, maxPixelValue = 0;

	// AX detection Vikram 05/03/2006
	bool is3DAngio          = false;
	bool isDerivedImageType = false;

	if (bytesPerPixel == 2)
	{
		short* pxls = (short*) iPixels;

		memset (histogram, 0, kSizeOfHistogram*sizeof(unsigned int));
		int bitsToShift = iHighBit+1-iBitsStored;
		
		// Vikram 04/12/2006 AX Data Support
		// Need to change the way we detect AX 05/03/2006
		//
		// First find if the image type is derived
		for (int l = 0; l < iImageTypeTokens.size(); l++)
		{
			if (_stricmp("DERIVED", iImageTypeTokens[l].c_str()) == 0)
			{
				isDerivedImageType = true;
			}

			if (_stricmp("3DANGIO", iImageTypeTokens[l].c_str()) == 0)
			{
				is3DAngio = true;
			}
		}
		//
	    // End AX detection 05/03/2006

		if (is3DAngio && isDerivedImageType) // Then it is an AX image
		{
			if (iPixelRepresentation == 1)
			{
				m_dataConverter->FindMinMax((short *)iPixels, totalSize, iIsLittleEndian,
					                         minPixelValue, maxPixelValue);
			}
			else
			{
				m_dataConverter->FindMinMax((unsigned short *)iPixels, totalSize, iIsLittleEndian,
					                         minPixelValue, maxPixelValue);
			}
		}
		else
		{
			// -- 2008.06.15 use the new converter that takes care of both signed and unsigned data
			// with wrap-around fix
			m_dataConverter->ConvertCTToVoxel((unsigned short *)iPixels, totalSize, iPixelRepresentation,
												iRescaleSlope,iRescaleIntercept);

//			m_dataConverter->ConvertToVoxel((unsigned short *)iPixels, totalSize, 
//											iRescaleSlope,iRescaleIntercept, iIsLittleEndian != 0,
//											(unsigned short*)0, use1024);

			int  maxVal = 0;
			// Vikram 03/20/2006 - Re-enabling Histogram Calculation
			// -- 04/04/2006 can't use iPixels[i]. 
			// Need to use pxels, also added error check and Debug statement
			for (int i = 0; i < totalSize; i++)
			{
				//		histogram[iPixels[i]]++;
				assert(pxls[i] >=0 && pxls[i] < kSizeOfHistogram);
				if (pxls[i] >=0 && pxls[i] < kSizeOfHistogram)
				{
					histogram[pxls[i]]++;
#ifdef _DEBUG
					if (histogram[pxls[i]] > 0 && maxVal < pxls[i])
						maxVal = pxls[i];
#endif
				}
			}
		}

#ifdef _DEBUG
	//	fprintf(stderr,"***********HistogramMax=%d\n", maxVal);
#endif
	}
	else
	{
		return kErrFormatNotSupported;
	}

	ULONGLONG fp = 0;
	unsigned long sizeOfDataInBytes = bytesPerPixel*iSizeX*iSizeY;
	// Write the Data Cache First
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	
	if (status != kSuccess) return status;

	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, 
								iBitsStored, iHighBit, iSizeX, iSizeY,fp, 
								sizeOfDataInBytes, iImageTypeTokens, kNone,
								   iWindowWidth, iWindowCenter);
	if (status != kSuccess) return status;
	ost << kRescaleSlopeStr << " = "       << iRescaleSlope       << "\n";
	ost << kRescaleInterceptStr << " = " << iRescaleIntercept << "\n";
	ost << kImageNumberStr << " = " << iImageNumber << "\n";
	
	if (use1024)
		ost << kUsed1024Str << " = " << use1024  << "\n";

	// -- 20056.05.05
	if (imageDate && *imageDate)
		ost << kImageDateStr << " = " << imageDate << endl;
	
	if (imageTime && *imageTime)
		ost << kImageTimeStr << " = " << imageTime << endl;

	// Murali 2007.01.03
	if (scanOptions && *scanOptions)
	{
		ost << kScanOptionsStr << " = " << scanOptions << endl;
	}

	if (manufacturer && *manufacturer)
	{
		ost << kManufacturerStr << " = " << manufacturer << endl;
	}

	status = PushCTMRCommonAttributes (ost, iImagePosition, iImageOrientation, 
										iPixelSpacing, iSliceThickness, sizeOfHistogram, histogram, iPixelRepresentation);


	if (status != kSuccess) return status;

	// Vikram 04/12/2006 AX Data Support
	// If (is3DAngio && isDerivedImageType) then output the min and max pixel values
	if (is3DAngio && isDerivedImageType) // Then it is an AX image
	{
		ost << kSliceMinVoxelValueStr << " = " << minPixelValue << "\n";
		ost << kSliceMaxVoxelValueStr << " = " << maxPixelValue << "\n";
	}


	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	status = WriteDesc(ost);

	return status;
}

//-----------------------------------------------------------------------------
// Add a single MR Image to cache
int AppComCacheWriter::AddMRImage (const char* iCacheLocation,
									  const char* iModality,
						              const char* iSOPInstanceUID,
                                      const char* iSOPClassUID,

						              int iBitsAllocated,
						              int iBitsStored,
						              int iHighBit,
						
						              int iSizeX, 
						              int iSizeY,  

						              bool  iIsLittleEndian,

						              std::vector <std::string>& iImageTypeTokens,

						              double iImagePosition[3], 
						              double iImageOrientation[6], 
						              double iPixelSpacing[2],
									  double iSliceThickness,

									  int	 iImageNumber,
									  int	 iPixelRepresentation,

						              unsigned char*     iPixels,
									  int&	ioMinPixelValue, int& ioMaxPixelValue,
									  unsigned int		 iSizeOfCompressedData,
						              eCompressionMethod iCompressionMethod,
									  int iWindowWidth,
									  int iWindowCenter,
									  // -- 2006.04.17
									  double slope,
									  double intercept,
									  const char* imageDate,
									  const char* imageTime)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddMRImage(" << iCacheLocation <<")\n";
cs.Leave();
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	// make sure cacahe is avaliable
	int status = SetCache(iCacheLocation);
	if( status != kSuccess) return status;

	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}



	// This is the number of bytes per pixel
	int bytesPerPixel = (iBitsAllocated+7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;


	unsigned int histogram[kSizeOfHistogram];
	unsigned int sizeOfHistogram = kSizeOfHistogram;

    int i = 0;
    int totalSize = iSizeX*iSizeY;

	m_dataConverter->SetKnownMinMax(ioMinPixelValue, ioMaxPixelValue);
	int minVoxelValue = 0;
	int maxVoxelValue = 0;
	// Swap the pixels and take care of calculating the histogram
	if (bytesPerPixel == 2)
	{
		unsigned short* pxls = (unsigned short*) iPixels;
		
		memset (histogram, 0, kSizeOfHistogram*sizeof(unsigned int));

		// -- JUN-26-2003  CacheWriter does not convert for MR
		if (iPixelRepresentation == 1)
		{
		   m_dataConverter->FindMinMax((short *)iPixels, totalSize, iIsLittleEndian,
				                              minVoxelValue, maxVoxelValue);
		//	m_dataConverter->ConvertMRToVoxel((short *)iPixels, totalSize, iIsLittleEndian,
		//		                              minVoxelValue, maxVoxelValue);
		}
		else
		{
			m_dataConverter->FindMinMax((unsigned short *)iPixels, totalSize, iIsLittleEndian,
				                              minVoxelValue, maxVoxelValue);
		//	m_dataConverter->ConvertMRToVoxel((unsigned short *)iPixels, totalSize, iIsLittleEndian,
		//		                              minVoxelValue, maxVoxelValue);
		}
	}
	else if (bytesPerPixel == 1)
	{
		memset (histogram, 0, kSizeOfHistogram*sizeof(unsigned int));

		
		// -- JUN-26-2003  CacheWriter does not convert for MR
		if (iPixelRepresentation == 1)
		{
			m_dataConverter->FindMinMax((char *)iPixels, totalSize, true,
				                              minVoxelValue, maxVoxelValue);
		//	m_dataConverter->ConvertMRToVoxel((char *)iPixels, totalSize,
		//		                              minVoxelValue, maxVoxelValue);
		}
		else
		{
			m_dataConverter->FindMinMax((unsigned char *)iPixels, totalSize,true,
				                              minVoxelValue, maxVoxelValue);
		//	m_dataConverter->ConvertMRToVoxel((unsigned char *)iPixels, totalSize,
		//		                              minVoxelValue, maxVoxelValue);
		}
	}
	else
	{
		return kErrFormatNotSupported;
	}

	ioMinPixelValue = minVoxelValue;
	ioMaxPixelValue = maxVoxelValue;
	
	ULONGLONG fp = 0;
	unsigned int sizeOfDataInBytes = bytesPerPixel*iSizeX*iSizeY;
	// Write the Data Cache First
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	
	if (status != kSuccess) return status;

	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, 
									iBitsStored, iHighBit, iSizeX, iSizeY,fp, 
									sizeOfDataInBytes, iImageTypeTokens, kNone,
								   iWindowWidth, iWindowCenter);
	if (status != kSuccess) return status;

	// -- 20056.05.05
	if (imageDate && *imageDate)
		ost << kImageDateStr << " = " << imageDate << endl;
	
	if (imageTime && *imageTime)
		ost << kImageTimeStr << " = " << imageTime << endl;
		
	status = PushCTMRCommonAttributes (ost, iImagePosition, iImageOrientation, 
									iPixelSpacing, iSliceThickness, sizeOfHistogram, histogram, iPixelRepresentation);
	if (status != kSuccess) return status;
	ost << kImageNumberStr << " = " << iImageNumber << "\n";

	ost << kSliceMinVoxelValueStr << " = " << minVoxelValue << "\n";
	ost << kSliceMaxVoxelValueStr << " = " << maxVoxelValue << "\n";

	
	ost << kRescaleSlopeStr << " = " << slope << "\n";
	ost << kRescaleInterceptStr << " = " << intercept << "\n";


	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	
	status = WriteDesc(ost);		
	

	return status;

}

//-----------------------------------------------------------------------------
// Add a single XA/XF Image to cache
int AppComCacheWriter::AddXAImage (const char* iCacheLocation,
									  const char* iModality,
						              const char* iSOPInstanceUID,
                                      const char* iSOPClassUID,

						              int iBitsAllocated,
						              int iBitsStored,
						              int iHighBit,
						
						              int iSizeX, 
						              int iSizeY,  

						              bool  iIsLittleEndian,

						              std::vector <std::string>& iImageTypeTokens,
									  int iInstanceNumber, //tcz 2005.06.10
									  int iPixelRepresentation,
						              unsigned char*     iPixels,

									  ePhotometricInterpretation iPhotometricInterpretation,
						              const char* iReferencedSOPInstanceUID,
									  int iWindowWidth,
									  int iWindowCenter,
									  unsigned int		 iSizeOfCompressedData,
						              eCompressionMethod iCompressionMethod,
									  double rescaleSlope, double rescaleIntercept)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddXAImage(" << iCacheLocation <<")\n";
cs.Leave();
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	// make sure cacahe is avaliable
	int status = SetCache(iCacheLocation, true);
	if( status != kSuccess) 
	{
		status = SetCache(iCacheLocation);
		if (status != kSuccess)
			return status;
	}

	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}

	
	// This is the number of bytes per pixel
	int bytesPerPixel = (iBitsAllocated+7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;
	int i = 0;
	int totalSize = iSizeX*iSizeY;
	
	/* --  03/14/2003 use modalityLUT if exists. 
	 * we also need the pixel encoding tag 
	 */
	if (strcmp(iModality,"CR")==0)
	{
		int min, max;
		m_dataConverter->ConvertCRToVoxel((char*) iPixels, totalSize,
									     rescaleSlope, rescaleIntercept,
										 iIsLittleEndian,
										 bytesPerPixel,
										 m_useModalityLUTForCR,
										 bitsToShift,0, &min, &max, 										 
										 &iWindowWidth, &iWindowCenter,
										 iPixelRepresentation);
#ifdef _DEBUG
		char pp[128];
		snprintf(pp, sizeof pp, "In TRCacheWriter. CR min=%d max=%d", min, max);
//		cerr << pp << endl;
#endif
	}
	else
	{
		// Swap Data
		// More to be done here - Vikram 11/07/01
		if (bytesPerPixel == 2)
		{
			
			unsigned short* pxls = (unsigned short*) iPixels;
			unsigned short pixel;
			if (iIsLittleEndian)
			{
				
				for (i = 0; i < totalSize; i++)
				{
					pixel = pxls[i]<<bitsToShift;
					pxls[i] = (pixel > 4095)?4095:pixel;
					//pxls[i] = clamp((unsigned short)(pxls[i]<<bitsToShift), (unsigned short)0, (unsigned short)4095);
				}
			}
			else
			{	
				for (i = 0; i < totalSize; i++)
				{
					pixel = pxls[i];
					pixel = pixel ? ((pixel & 0xff) << 8) | ((pixel >> 8) & 0xff) : 0;
					pixel = pixel<<bitsToShift;
					pxls[i] = (pixel > 4095)?4095:pixel;
					//pxls[i] = clamp((unsigned short)(swap16(pxls[i])<<bitsToShift), (unsigned short)0, (unsigned short)4095);
				}
			}
		}
		
	}

	ULONGLONG fp = 0;
	unsigned int sizeOfDataInBytes = bytesPerPixel*iSizeX*iSizeY;
	// Write the Data Cache First
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	

	if (status != kSuccess) return status;
	
	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, 
									iBitsStored, iHighBit, iSizeX, iSizeY,fp, 
									sizeOfDataInBytes, iImageTypeTokens, kNone,
									iWindowWidth, iWindowCenter);

	if (status != kSuccess) return status;

	// -- 2005.06.10
	// RF etc.
	ost << kImageNumberStr << " = " << iInstanceNumber << "\n";

	ost << kPhotometricInterpretationStr << " = " << iPhotometricInterpretation << "\n";

	// -- 2003-03-18 temporary output in description file for ModalityLUT
	ost << kRescaleSlopeStr << " = " << rescaleSlope << "\n";
	ost << kRescaleInterceptStr << " = " << rescaleIntercept << "\n";
	ost << kModalityLUTStr << " = " << m_useModalityLUTForCR << "\n";

	if (iReferencedSOPInstanceUID && iReferencedSOPInstanceUID[0])
		ost << kReferencedSOPInstanceUIDStr << " = " << iReferencedSOPInstanceUID << "\n";

	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	status = WriteDesc(ost);
	return status;	

}

//-----------------------------------------------------------------------------
// Add a single CR/DR/DX Image to cache
int AppComCacheWriter::AddCRDRDXImage (const char* iCacheLocation,
									      const char* iModality,
						                  const char* iSOPInstanceUID,
                                          const char* iSOPClassUID,

						                  int iBitsAllocated,
						                  int iBitsStored,
						                  int iHighBit,
						
						                  int iSizeX, 
						                  int iSizeY,  

						                  bool  iIsLittleEndian,

						                  std::vector <std::string>& iImageTypeTokens,
										  int iInstanceNumber,   // tcz 2005.06.10
										  int iPixelRepresentation,
						                  unsigned char*     iPixels,

									      ePhotometricInterpretation iPhotometricInterpretation,
									      unsigned int		 iSizeOfCompressedData,
						                  eCompressionMethod iCompressionMethod,

									     double rescaleSlope, 
										 double rescaleIntercept,
								         double iPixelSpacing[2],
										 int    iWindowWidth,
										 int    iWindowCenter)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddCRDRDXImage(" << iCacheLocation <<")\n";
cs.Leave();
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	// make sure cacahe is avaliable
	int status = SetCache(iCacheLocation, true);
	if( status != kSuccess) 
	{
		status = SetCache(iCacheLocation);
		if (status != kSuccess)
			return status;
	}

	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}

	
	// This is the number of bytes per pixel
	int bytesPerPixel = (iBitsAllocated+7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;
	int i = 0;
	int totalSize = iSizeX*iSizeY;
	
	/* --  03/14/2003 use modalityLUT if exists. 
	 * we also need the pixel encoding tag 
	 */
	if (strcmp(iModality,"CR")==0)
	{
		int min, max;
		m_dataConverter->ConvertCRToVoxel((char*) iPixels, totalSize,
									     rescaleSlope, rescaleIntercept,
										 iIsLittleEndian,
										 bytesPerPixel,
										 m_useModalityLUTForCR,
										 bitsToShift,0, &min, &max,
										 &iWindowWidth, &iWindowCenter,
										 iPixelRepresentation);


		/* scale the voi lut */
		float	slope       = 1.0f, offset = 0.0;
		m_dataConverter->GetPixelToVoxelMapping(slope, offset); 
		iWindowWidth = int(iWindowWidth * slope + 0.5f);
		iWindowCenter = int((iWindowCenter * slope + offset)*1.0001f);

		iWindowWidth = iRTVSClamp(iWindowWidth,0,4096);
		iWindowCenter = iRTVSClamp(iWindowCenter,-200,5000);

#ifdef _DEBUG
		char pp[128];
		snprintf(pp, sizeof pp, "In TRCacheWriter. CR min=%d max=%d", min, max);
//		cerr << pp << endl;
#endif
	}
	else
	if (strcmp(iModality,"DX")==0)
	{
		unsigned short* pData = (unsigned short*) iPixels;

		int		globalMinVV = 0,    globalMaxVV = 0;
		float	slope       = 1.0f, offset = 0.0;

		int		totalSize   =  iSizeX*iSizeY;

		m_dataConverter->FindMinMax(pData, totalSize, true,globalMinVV, globalMaxVV);
	//	m_dataConverter->SetVOIWindowWidth (iWindowWidth);
	//	m_dataConverter->SetVOIWindowCenter (iWindowCenter);
		m_dataConverter->RescaleMR (pData, pData, totalSize, globalMinVV, globalMaxVV);

		/* scale the voi lut */
		m_dataConverter->GetPixelToVoxelMapping(slope, offset); 
		iWindowWidth = int(iWindowWidth * slope + 0.5f);
		iWindowCenter = int((iWindowCenter * slope + offset)*1.0001f);

		iWindowWidth = iRTVSClamp(iWindowWidth,0,4096);
		iWindowCenter = iRTVSClamp(iWindowCenter,-200,5000);

	}


	ULONGLONG fp = 0;
	unsigned int sizeOfDataInBytes = bytesPerPixel*iSizeX*iSizeY;
	// Write the Data Cache First
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	

	if (status != kSuccess) return status;
	
	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, 
									iBitsStored, iHighBit, iSizeX, iSizeY,fp, 
									sizeOfDataInBytes, iImageTypeTokens, kNone, 
									iWindowWidth, iWindowCenter);

	if (status != kSuccess) return status;

	ost << kPixelSpacingStr << " = " 
		<< iPixelSpacing[0] << " " 
		<< iPixelSpacing[1] << "\n";

	// -- 2005.06.10 image number
	ost << kImageNumberStr << " = " << iInstanceNumber << "\n";

	ost << kPhotometricInterpretationStr << " = " << iPhotometricInterpretation << "\n";

	// -- 2003-03-18 temporary output in description file for ModalityLUT
	ost << kRescaleSlopeStr << " = " << rescaleSlope << "\n";
	ost << kRescaleInterceptStr << " = " << rescaleIntercept << "\n";
	ost << kModalityLUTStr << " = " << m_useModalityLUTForCR << "\n";


	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	status = WriteDesc(ost);
	return status;	

}

//-----------------------------------------------------------------------------
#include "PxDicomimage.h"
int AppComCacheWriter::AddUSImage (const char* iCacheLocation,
				                      const char* iModality,
				                      const char* iSOPInstanceUID,
						              const char* iSOPClassUID,

				                      int iBitsAllocated,
				                      int iBitsStored,
				                      int iHighBit,
				                        
				                      int iSizeX, 
				                      int iSizeY,  

				                      bool  iIsLittleEndian,

				                      std::vector <std::string>& iImageTypeTokens,

				                      unsigned char*     iPixels,

				                      int iSamplesPerPixel,
				                      ePhotometricInterpretation iPhotometricInterpretation,
				                      ePlanarConfiguration iPlanarConfiguration,
                                      eCompressionMethod iCompressionMethod,
									  int iWindowWidth,
									  int iWindowCenter,
									  PxDicomPalette* palette)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddUSImage(" << iCacheLocation <<")\n";
cs.Leave();
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	// make sure cacahe is avaliable
	// because cache can be closed by anyone at any time
	int status = SetCache(iCacheLocation);
	if( status != kSuccess) return status;
	
	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}


	int bytesPerPixel = (iBitsAllocated + 7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;
	unsigned long maxVal = (unsigned long) (pow(2.,(double)iBitsStored)) - 1;


	unsigned char* pxls = iPixels;
	unsigned char *rgb = 0;
	iRTVSmallocGuard<unsigned char*> rgbGuard(rgb);
	unsigned char pixel;

	int minVoxelValue = 0;
	int maxVoxelValue = 0;

    if (bytesPerPixel * iSamplesPerPixel > 2) 
	{
        if (iPlanarConfiguration != kRGBRGB)
        {
            return kErrFormatNotSupported;
        }
		pxls = iPixels;
	}
    else
    {
		if (bytesPerPixel == 1)
		{
			for (int i = 0; i < iSizeX * iSizeY; i++)
			{
				pixel = pxls[i]<<bitsToShift;
				pxls[i] = (pixel > maxVal)?maxVal:pixel; 
			}
		} 
		else if (bytesPerPixel == 2)
		{
			unsigned short * p = (unsigned short*) pxls;
			unsigned short px;
			for (int i = 0; i < iSizeX * iSizeY; i++)
			{
				px = p[i]<<bitsToShift;
				p[i] = (px > maxVal)?maxVal:px; 
			}
		}
		
		// -- 2004.04.16 
		// hack palette color support in. not the best way to do this.
		if (iPhotometricInterpretation == kPALETTE_COLOR && palette)
		{
			if (!(rgb = new unsigned char[3*iSizeX * iSizeY]))
				return kErrFormatNotSupported;

			rgbGuard = rgb;
			
			int xy, ic, N = iSizeX * iSizeY;
			
			if (bytesPerPixel == 1)
			{
				for (xy = 0, ic = 0; xy < N; ++xy, ic +=3)
				{
					palette->Lookup(pxls[xy], rgb[ic], rgb[ic+1],rgb[ic+2]);
				}
			}
			else if (bytesPerPixel==2)
			{
				unsigned short *p = (unsigned short *)pxls;
				for (xy = 0, ic = 0; xy < N; ++xy, ic +=3)
				{
					palette->Lookup(p[xy], rgb[ic], rgb[ic+1],rgb[ic+2]);
				}
			}

			pxls = rgb;
			iPixels = rgb;
			bytesPerPixel = 1;
			iSamplesPerPixel = 3;
			iPlanarConfiguration = kRGBRGB;
		}	
    }

	ULONGLONG fp = 0;
	unsigned long sizeOfDataInBytes = bytesPerPixel * iSamplesPerPixel * iSizeX * iSizeY;
	// Write the Data Cache first in one action to make sure it is in one block
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	
	if (status != kSuccess) return status;

	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, iBitsStored,
						           iHighBit, iSizeX, iSizeY, fp, sizeOfDataInBytes, iImageTypeTokens, kNone,
								   iWindowWidth, iWindowCenter);

	if (status != kSuccess) return status;
	
	// SC Image specific strings
	ost << kSamplesPerPixelStr << " = "       << iSamplesPerPixel       << "\n";
	ost << kPhotometricInterpretationStr << " = " << iPhotometricInterpretation << "\n";


	if (iSamplesPerPixel > 1)
		ost << kPlanarConfigurationStr << " = " << iPlanarConfiguration << "\n";
		
	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	// Write the description in one action to make sure it is in one block
	status = WriteDesc(ost);
		
	return status;
}

//---------------------------------------------------------------------
// Add a single PT Image to cache
int AppComCacheWriter::AddPTImage (const char* iCacheLocation,
						const char* iModality,
						const char* iSOPInstanceUID,
						const char* iSOPClassUID,

						int iBitsAllocated,
						int iBitsStored,
						int iHighBit,
						
						int iSizeX, 
						int iSizeY,  

						bool  iIsLittleEndian,

						std::vector <std::string>& iImageTypeTokens,

						double iImagePosition[3], 
						double iImageOrientation[6], 
						double iPixelSpacing[2],
					    double iSliceThickness,
						
						double iRescaleSlope,
						double iRescaleIntercept,
						int	 iImageNumber,
						int	 iPixelRepresentation, // Vikram 09/05/02 - Need to deal with signed data

						unsigned char*     iPixels,
						unsigned int	   iSizeOfCompressedData,
						eCompressionMethod iCompressionMethod,
						double iWindowWidth,
						double iWindowCenter,
						// -- 2006.05
						const char* imageDate,
						const char* imageTime,
					    PETObjectAttributes& iAttrib)  // -- 2006.06.10 SUV stuff
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddPTImage(" << iCacheLocation <<")\n";
cs.Leave();	
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	int minVoxelValue = 0;
	int maxVoxelValue = 0;
	
	// make sure cacahe is avaliable
	int status = SetCache(iCacheLocation);
	if( status != kSuccess) return status;

#if 0
	// test overhead testing
	ULONGLONG Ofp = 0;
	// Write the Data Cache First
	status = WriteData(iPixels, 0, Ofp);	
	char sbuf0[Stream_Size];
	ostrstream ost0(sbuf0, Stream_Size);
	status = WriteDesc(ost0);
	return status;
#endif
	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}

	// This is the number of bytes per pixel
	int bytesPerPixel = (iBitsAllocated+7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;


	unsigned int histogram[kSizeOfHistogram];
	unsigned int sizeOfHistogram = kSizeOfHistogram;
	double rescaleIntercept = iRescaleIntercept;

    int i = 0;
    int totalSize = iSizeX*iSizeY;
	// Swap the pixels and take care of calculating the histogram
	 
	if (bytesPerPixel == 2)
	{
		short* pxls = (short*) iPixels;

		memset (histogram, 0, kSizeOfHistogram*sizeof(unsigned int));
		int bitsToShift = iHighBit+1-iBitsStored;


		if (iPixelRepresentation == 0 )
		{
			
			m_dataConverter->FindMinMax((unsigned short *)iPixels, totalSize, iIsLittleEndian,
										minVoxelValue, maxVoxelValue);
		}
		else
		{
			m_dataConverter->FindMinMax((short *)iPixels, totalSize, iIsLittleEndian,
										minVoxelValue, maxVoxelValue);
			
		}

	}
	else
	{
		return kErrFormatNotSupported;
	}

	ULONGLONG fp = 0;
	unsigned long sizeOfDataInBytes = bytesPerPixel*iSizeX*iSizeY;
	// Write the Data Cache First
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	
	if (status != kSuccess) return status;

	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, 
								iBitsStored, iHighBit, iSizeX, iSizeY,fp, 
								sizeOfDataInBytes, iImageTypeTokens, kNone,
								   iWindowWidth, iWindowCenter);
	if (status != kSuccess) return status;
	ost << kRescaleSlopeStr << " = "       << iRescaleSlope       << "\n";
	ost << kRescaleInterceptStr << " = " << iRescaleIntercept << "\n";
	ost << kImageNumberStr << " = " << iImageNumber << "\n";
	ost << kSliceMinVoxelValueStr << " = " << minVoxelValue << "\n";
	ost << kSliceMaxVoxelValueStr << " = " << maxVoxelValue << "\n";

		// -- 20056.05.05
	if (imageDate && *imageDate)
		ost << kImageDateStr << " = " << imageDate << endl;
	
	if (imageTime && *imageTime)
		ost << kImageTimeStr << " = " << imageTime << endl;

		// PET specific stuff
	ost << kPETDecayFactorStr << " = " << iAttrib.m_decayFactor << endl;
	ost << kPETUnitsStr	   << " = " << iAttrib.m_units.c_str() << endl;
	ost << kPETTotalDoseStr << " = " << iAttrib.m_totalDose << endl;
	ost << kPETStartTimeStr << " = " << iAttrib.m_startTime.c_str() << endl;
	ost << kPETHalfLifeStr  << " = " << iAttrib.m_halfLife << endl;
	ost << kAcquisitionTimeStr << " = " << iAttrib.m_acquisitionTime.c_str() << endl;
	ost << "PatientWeight"	<< " = " << iAttrib.m_patientWeight << endl;

	status = PushCTMRCommonAttributes (ost, iImagePosition, iImageOrientation, 
										iPixelSpacing, iSliceThickness, sizeOfHistogram, histogram, iPixelRepresentation);
	if (status != kSuccess) return status;




	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	status = WriteDesc(ost);

	return status;
}

#if 0
//---------------------------------------------------------------------
// Add a single NM Image to cache
int AppComCacheWriter::AddNMImage (const char* iCacheLocation,
						const char* iModality,
						const char* iSOPInstanceUID,
						const char* iSOPClassUID,

						int iBitsAllocated,
						int iBitsStored,
						int iHighBit,
						
						int iSizeX, 
						int iSizeY,  

						bool  iIsLittleEndian,

						std::vector <std::string>& iImageTypeTokens,

						CNMObject& iNMObject,
						double iPixelSpacing[2],
						
						double iRescaleSlope,
						double iRescaleIntercept,
						int	 iFrameNumber,
						int	 iPixelRepresentation, // Vikram 09/05/02 - Need to deal with signed data

						unsigned char*     iPixels,
						unsigned int	   iSizeOfCompressedData,
						eCompressionMethod iCompressionMethod,
						double iWindowWidth,
						double iWindowCenter)
{
#ifdef CacheWriterVerbose
cs.Enter();
	cerr << ">>Thread: "<< GetCurrentThreadId() <<", AddPTImage(" << iCacheLocation <<")\n";
cs.Leave();	
#endif

	//	-- - 04/16/03 - Merge takes care of byte swapping automatically
	iIsLittleEndian = true;

	int minVoxelValue = 0;
	int maxVoxelValue = 0;
	
	// make sure cacahe is avaliable
	int status = SetCache(iCacheLocation);
	if( status != kSuccess) return status;

#if 0
	// test overhead testing
	ULONGLONG Ofp = 0;
	// Write the Data Cache First
	status = WriteData(iPixels, 0, Ofp);	
	char sbuf0[Stream_Size];
	ostrstream ost0(sbuf0, Stream_Size);
	status = WriteDesc(ost0);
	return status;
#endif
	// Only handle uncompressed files right now
	if (iCompressionMethod != kNone)
	{
		return kErrFormatNotSupported;
	}

	// This is the number of bytes per pixel
	int bytesPerPixel = (iBitsAllocated+7) / 8;
	int bitsToShift = iHighBit+1-iBitsStored;


	unsigned int histogram[kSizeOfHistogram];
	unsigned int sizeOfHistogram = 0;
	double rescaleIntercept = iRescaleIntercept;

    int i = 0;
    int totalSize = iSizeX*iSizeY;
	// Swap the pixels and take care of calculating the histogram
	 
	if (bytesPerPixel < 1 || bytesPerPixel > 2)
	{
		return kErrFormatNotSupported;
	}

	if (bytesPerPixel == 2)
	{
		sizeOfHistogram = kSizeOfHistogram;
		short* pxls = (short*) iPixels;

		memset (histogram, 0, kSizeOfHistogram*sizeof(unsigned int));
		int bitsToShift = iHighBit+1-iBitsStored;

	

		if (iPixelRepresentation == 0 )
		{
			
			m_dataConverter->FindMinMax((unsigned short *)iPixels, totalSize, iIsLittleEndian,
										minVoxelValue, maxVoxelValue);
		}
		else
		{
			m_dataConverter->FindMinMax((short *)iPixels, totalSize, iIsLittleEndian,
										minVoxelValue, maxVoxelValue);
			
		}
	}
	else if (bytesPerPixel == 1)
	{
		unsigned char pixel;
		unsigned char* pxls = (unsigned char*) iPixels;
		unsigned long maxVal = (unsigned long) (pow(2.,(double)iBitsStored)) - 1;

		for (int i = 0; i < iSizeX * iSizeY; i++)
		{
			pixel = pxls[i]<<bitsToShift;
			pxls[i] = (pixel > maxVal)?maxVal:pixel; //it is unsigned char, no under cut
		}

		if (iPixelRepresentation == 0 )
		{
			
			m_dataConverter->FindMinMax((unsigned char *)iPixels, totalSize, iIsLittleEndian,
										minVoxelValue, maxVoxelValue);
		}
		else
		{
			m_dataConverter->FindMinMax((char *)iPixels, totalSize, iIsLittleEndian,
										minVoxelValue, maxVoxelValue);
			
		}
	} 

		
		ULONGLONG fp = 0;
	unsigned long sizeOfDataInBytes = bytesPerPixel*iSizeX*iSizeY;
	// Write the Data Cache First
	status = WriteData(iPixels, sizeOfDataInBytes, fp);	
	if (status != kSuccess) return status;

	char sbuf[Stream_Size];
	ostrstream ost(sbuf, Stream_Size);

	status = PushCommonAttributes (ost, iModality, iSOPInstanceUID, iSOPClassUID, iBitsAllocated, 
								iBitsStored, iHighBit, iSizeX, iSizeY,fp, 
								sizeOfDataInBytes, iImageTypeTokens, kNone,
								   iWindowWidth, iWindowCenter);
	if (status != kSuccess) return status;
	ost << kRescaleSlopeStr << " = "       << iRescaleSlope       << "\n";
	ost << kRescaleInterceptStr << " = " << iRescaleIntercept << "\n";
	ost << kImageNumberStr << " = " << iFrameNumber << "\n";
	ost << kSliceMinVoxelValueStr << " = " << minVoxelValue << "\n";
	ost << kSliceMaxVoxelValueStr << " = " << maxVoxelValue << "\n";

	status = PushNMAttributes (ost, iFrameNumber, iNMObject);
	
	double imagePosition[3] = {0., 0., 0.};
	double imageOrientation[6] = {1., 0., 0., 0., 1., 0.};
	bool hasValidOrientation = iNMObject.CalculateImageOrientationAndPosition(iFrameNumber, imagePosition, imageOrientation);
	status = PushCTMRCommonAttributes (ost, imagePosition, imageOrientation, 
										iPixelSpacing, 0., sizeOfHistogram, histogram, iPixelRepresentation, hasValidOrientation);
	if (status != kSuccess) return status;

	ost << "</" << kSliceDescriptionStr << ">" << "\n"; // close section
	status = WriteDesc(ost);

	return status;
}
#endif

//-----------------------------------------------------------------------------	
//
int AppComCacheWriter::PushNMAttributes (ostrstream& ost, int iFrameNumber, CNMObject& iNMObject)
{
	int index;

	if ((index = iNMObject.GetDetectorIndex(iFrameNumber)) >= 0)		ost << kDetectorStr       << " = " << index << "\n";
	if ((index = iNMObject.GetRotationIndex(iFrameNumber)) >= 0)		ost << kRotationStr       << " = " << index << "\n";
	if ((index = iNMObject.GetEnergyWindowIndex(iFrameNumber)) >= 0)	ost << kEnergyWindowStr   << " = " << index << "\n";
	if ((index = iNMObject.GetPhaseIndex(iFrameNumber)) >= 0)			ost << kPhaseStr          << " = " << index << "\n";
	if ((index = iNMObject.GetRRIntervalIndex(iFrameNumber)) >= 0)		ost << kRRIntervalStr     << " = " << index << "\n";
	if ((index = iNMObject.GetTimeSlotIndex(iFrameNumber)) >= 0)		ost << kTimeSlotStr       << " = " << index << "\n";
	if ((index = iNMObject.GetSliceIndex(iFrameNumber)) >= 0)			ost << kSliceIndexStr     << " = " << index << "\n";
	if ((index = iNMObject.GetAngularViewIndex(iFrameNumber)) >= 0)		ost << kAngularViewStr    << " = " << index << "\n";
	if ((index = iNMObject.GetTimeSliceIndex(iFrameNumber)) >= 0)		ost << kTimeSliceStr      << " = " << index << "\n";

	int isRotational = (int) iNMObject.GetIsRotational();
	ost << kIsRotationalStr << " = " << isRotational << "\n";

	if (isRotational)
	{
		int rotIndex = iNMObject.GetRotationIndex(iFrameNumber);
		rotIndex = (rotIndex < 0) ? 0 : rotIndex;
	
		std::vector<CNMRotation> rotationV = iNMObject.GetRotations();

		double startAngle = rotationV[rotIndex].GetStartAngle();
		double angularStep = rotationV[rotIndex].GetAngularStep();
		int direction = rotationV[rotIndex].GetRotationDirection();

		ost << kStartAngleStr << " = " << startAngle << "\n";
		ost << kAngularStepStr << " = " << angularStep << "\n";
		ost << kStepDirectionStr << " = " << direction << "\n";
	}
	else
	{
		ost << kSpacingBetweenSlicesStr << " = " << iNMObject.GetSpacingBetweenSlices() << "\n";
		ost << kSliceThicknessStr << " = " << iNMObject.GetSliceThickness() << "\n";
	}

	return 0;
}

//-----------------------------------------------------------------------------	

//-----------------------------------------------------------------------------
// Need this to convert output __int64
/*ostream& operator<<(ostream& os, __int64 i )
{
    char buf[20];
    sprintf(buf,"%I64d", i );
    os << buf;
    return os;
}
*/

//-----------------------------------------------------------------------------
// make sure cacahe is avaliable, before call this
int AppComCacheWriter::PushCommonAttributes (ostrstream& ost,
												 const char* iModality,
						                         const char* iSOPInstanceUID,
                                                 const char* iSOPClassUID,

						                         int iBitsAllocated,
						                         int iBitsStored,
						                         int iHighBit,
						
						                         int iSizeX, 
						                         int iSizeY,  
									             unsigned long  iStartOfDataInDataFile,
									             unsigned long  iSizeOfDataInBytes,

						                         std::vector <std::string>& iImageTypeTokens,
									             eCompressionMethod iCompressionMethod,
												 double iWindowWidth,
												 double iWindowCenter)
{
	// -- 2006.07.18 Took out histogram from cache description. 
	// Changed version to v1.00 (was V0.91
	ost << "<" << kSliceDescriptionStr << " V1.00>" << "\n";
	ost << kModalityStr << " = "       << iModality       << "\n";
	ost << kSOPInstanceUIDStr << " = " << iSOPInstanceUID << "\n";
	ost << kSOPClassUIDStr << " = "    << iSOPClassUID << "\n";
	ost << kBitsAllocatedStr << " = "  << iBitsAllocated     << "\n";
	ost << kBitsStoredStr << " = "     << iBitsStored     << "\n";
	ost << kHighBitStr << " = "		 << iHighBit     << "\n";
	ost << kSizeXStr << " = "          << iSizeX          << "\n";
	ost << kSizeYStr << " = "          << iSizeY          << "\n";
	ost << kStartOfDataInDataFileStr << " = " << iStartOfDataInDataFile  << "\n";
	ost << kSizeOfDataInBytesStr << " = " << iSizeOfDataInBytes  << "\n";

	std::string::size_type idx = 0;
	for (int i = 0; i < iImageTypeTokens.size(); i++)
	{
		// Vikram 09/05/02 - Do not write an Image Type Token
		// if its blank
		idx = iImageTypeTokens[i].find_first_not_of (' ');

		if (idx != std::string::npos)
		{
			ost << kImageTypeTokenStr << " = " << iImageTypeTokens[i].c_str() << "\n";
		}
	}

	if (iWindowWidth != 0 || iWindowCenter != 0)
	{
		ost << kVOIWindowWidthStr  << " = " << iWindowWidth  << "\n";
		ost << kVOIWindowCenterStr << " = " << iWindowCenter  << "\n";
	}

	return kSuccess;
}


//-----------------------------------------------------------------------------
// make sure cacahe is avaliable, before call this
int AppComCacheWriter::PushCTMRCommonAttributes (ostrstream& ost,
													 double iImagePosition[3], 
						                             double iImageOrientation[6], 
						                             double iPixelSpacing[2],
													 double iSliceThickness,
													 unsigned int iSizeOfHistogram,
													 unsigned int iHistogram[4096],
													 int iPixelRepresentation,
													 bool iHasValidOrientation)

{
	//::ostrstream ost << "</" << kSliceDescriptionStr << ">" << "\n";
	//LPDWORD byteswritten;
	//BOOL recode = WriteFile(outCacheFile, ost.str(), strlen(ost.str()), &byteswritten, NULL);
	//if(recode != FALSE)
	//{

	if (iHasValidOrientation)
	{
		ost << kHasValidOrientationStr << " = 1\n";


		ost << kImageOrientationStr << " = " 
			<< iImageOrientation[0] << " " 
			<< iImageOrientation[1] << " "
			<< iImageOrientation[2] << " "
			<< iImageOrientation[3] << " "
			<< iImageOrientation[4] << " "
			<< iImageOrientation[5] << "\n";
	}
	else
	{
		ost << kHasValidOrientationStr << " = 0\n";

		ost << kImageOrientationStr << " = " 
			<< 1 << " " 
			<< 0 << " "
			<< 0 << " "
			<< 0 << " "
			<< 1 << " "
			<< 0 << "\n";

	}

	ost << kImagePositionStr << " = " 
		<< iImagePosition[0] << " " 
		<< iImagePosition[1] << " " 
		<< iImagePosition[2] << "\n";

	ost << kPixelSpacingStr << " = " 
		<< iPixelSpacing[0] << " " 
		<< iPixelSpacing[1] << "\n";

	if (iSliceThickness > 0.)
	{
		ost << kSliceThicknessStr << " = "
			<< iSliceThickness << "\n";
	}

	ost << kPixelRepresentationStr << " = " 
		<< iPixelRepresentation << "\n";

	// -- 2006.07.18
	// Disable histogram writing. Way faster all-around, especially reading
#if 0
	if (iSizeOfHistogram > 0)
	{
		ost << kNumberOfEntriesInHistogramStr << " = " << iSizeOfHistogram << "\n";
		ost << kHistogramStr << " = " << "\n";

		ost.write((const char*)iHistogram, kSizeOfHistogram*sizeof(unsigned int)); 
		ost << "\n" ;
	}
#endif
	
	return kSuccess;
}

//-----------------------------------------------------------------------------

int AppComCacheWriter::WriteData (unsigned char* iPixels, unsigned int iNumberOfBytes, ULONGLONG& oFilePos)
{
	return Write(m_dataFilePath, iPixels, iNumberOfBytes, &oFilePos);
}

int AppComCacheWriter::WriteDesc (ostrstream& ost)
{
	int status = Write(m_descFilePath, (unsigned char*)ost.str(), ost.pcount());
	ost.rdbuf()->freeze( 0 ); //unfreeze it
	return status;
}
