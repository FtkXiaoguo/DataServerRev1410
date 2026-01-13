/***********************************************************************
 * AppComCacheReader.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This file implements the member functions of the 
 *	    AppComCacheReader Object.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
 
#include "AppComCacheReader.h"
 

#ifndef RTVSPlatform_H_
#include "AqCore/TRPlatform.h"
#endif

#if !defined(TICACHE_H)
#include "TICache.h"
#endif

#include <math.h>

#include <assert.h>
#include <algorithm>

#if !defined (USE_HASH)
std::map<std::string, int> AppComCacheReader::m_cacheDataTokens;
#else
std::map<int, int> AppComCacheReader::m_tokenToIdx;
#endif

 

int AppComCacheReader::m_sortMRByScanTypeFirst = 0;

// Vikram 07/17/2006 Fix for data with no corelation with ImageNumber and ImagePosition
int AppComCacheReader::m_presortCTByImagePositionAfterImageNumber = 0;

// Murali 2007.01.03 Fix for sorting CT with scanOptions specified
// Default is to use scanoptions for sorting siemens CT data.
int  AppComCacheReader::m_sortCTByScanOptionsFirst	= 1; 

// #define DO_READ_TIME

//-----------------------------------------------------------------------------
// Indicies
const int   kSliceDescriptionIdx			=  1;
const int   kEndOfSliceDescriptionIdx		=  2;
const int   kModalityIdx					=  3;
const int   kSOPInstanceUIDIdx				=  4;
const int   kBitsStoredIdx					=  5;
const int   kBitsAllocatedIdx				=  6;
const int   kHighBitIdx						=  7;
const int   kSizeXIdx						=  8;
const int   kSizeYIdx						=  9;
const int   kStartOfDataInDataFileIdx		= 10;
const int   kSizeOfDataInBytesIdx			= 11;
const int   kImageTypeTokenIdx				= 12;

const int   kImagePositionIdx				= 13;
const int   kImageOrientationIdx			= 14;
const int   kPixelSpacingIdx				= 15;
const int   kNumberOfEntriesInHistogramIdx	= 16;
const int   kHistogramIdx					= 17;


const int   kRescaleSlopeIdx				= 18;
const int   kRescaleInterceptIdx			= 19;

const int   kReferencedSOPInstanceUIDIdx	= 20;



// CT
const int   kModalityCTIdx					= 21;
// This is a type-2 DICOM field but we need it for Tri-phasic data
const int   kImageNumberIdx				= 22; 

// MR
const int   kModalityMRIdx					= 23;

// XA
const int   kModalityXAIdx					= 24;
const int   kModalityXFIdx					= 25;
const int   kModalityRFIdx					= 26;

// CR/DR
const int   kModalityCRIdx					= 27;
const int   kModalityDRIdx					= 28;


// SC
const int   kModalitySCIdx					= 29;
const int   kSamplesPerPixelIdx				= 30;
const int   kPhotometricInterpretationIdx	= 31;
const int   kPlanarConfigurationIdx         = 32;

// SOP Class UID support
const int   kSOPClassUIDIdx				    =  33;

//
const int   kModalityOTIdx                  = 34;

// US
const int   kModalityUSIdx                  = 35;
const int   kModalityIVUSIdx                = 36;

// Vikram 09/05/02 - Needed to handle signed data
const int   kPixelRepresentationIdx			= 37;
const int   kSliceMinVoxelValueIdx			= 38;
const int   kSliceMaxVoxelValueIdx			= 39;

const int   kGlobalMinVoxelValueIdx	 = 40;
const int   kGlobalMaxVoxelValueIdx  = 41;

const int   kSliceCompressionFactorIdx = 42;
const int   kInputCompressionSizeXIdx  = 43;
const int   kInputCompressionSizeYIdx  = 44;

const int   kVOIWindowWidthIdx  = 45;
const int   kVOIWindowCenterIdx = 46;
const int	kModalityLUT		= 47;

// Vikram 07/22/04 - NM Support

const int kDetectorIdx				= 48;
const int kRotationIdx				= 49;
const int kEnergyWindowIdx			= 50;
const int kPhaseIdx					= 51;
const int kRRIntervalIdx			= 52;
const int kTimeSlotIdx				= 53;
const int kSliceIndexIdx			= 54;
const int kAngularViewIdx			= 55;
const int kTimeSliceIdx				= 56;
const int kHasValidOrientationIdx	= 57;
const int kIsRotationalIdx			= 58;

// fix 1000 & 1024 problem
const int kUsed1024Idx				= 59;

//	-- - 11/22/04
const int kSliceThicknessIdx		= 60;

// -- 2006.05.06
const int kImageDateIdx				= 61;
const int kImageTimeIdx				= 62;

// -- 2006.06.10
const int kPETDecayFactorIdx		= 63;
const int kPETUnitsIdx				= 64;
const int kPETTotalDoseIdx			= 65;
const int kPETStartTimeIdx			= 66;
const int kPETHalfLifeIdx			= 67;
const int kAcquisitionTimeIdx		= 68;

const int kScanOptionsIdx			= 69; // Murali 2007.01.03
const int kManufacturerIdx			= 70;
//-----------------------------------------------------------------------------

const int kHashTableSize = 1024;

int AQNetHashString (std::string& iStr)
{
	int i;
	unsigned int h = 0;
	int  c, d;
	
	h = iStr.size ();

	for (i = 0; i < iStr.size (); i++)
	{
//		h = 37 * h + iStr[i];

		d = iStr[i];
		c = d;
		c ^= c<<6;
        h += (c<<11) ^ (c>>1);
        h ^= (d<<14) + (d<<7) + (d<<4) + d;
	}

	return h & (kHashTableSize-1);
}

//-----------------------------------------------------------------------------
int AQNetHashString (char* iStr)
{
	int i;
	unsigned int h = 0;
	int  c, d;
	
	h = strlen (iStr);

	int n = strlen (iStr);
	for (i = 0; i < n; i++)
	{
//		h = 37 * h + iStr[i];

		d = iStr[i];
		c = d;
		c ^= c<<6;
        h += (c<<11) ^ (c>>1);
        h ^= (d<<14) + (d<<7) + (d<<4) + d;
	}

	return h & (kHashTableSize-1);
}

//-----------------------------------------------------------------------------
// We transform multiframe into multi-image(SOP) by appending the frame number
// to SOPInstanceUID so we can look up the frame using sopinstanceUID.
// This function is used for both generation and lookup (T.C.Zhao 2005.05.23)
static inline void TransformMultiFrameSOPInstanceUID(std::string& ioSopInstanceUID, int iFrameNumber)
{
	char buffer[64];
	sprintf (buffer, "_%06d",iFrameNumber);
	ioSopInstanceUID += buffer;
}

//-----------------------------------------------------------------------------
bool AppComCacheReader::ShouldSortByScanOption(const char* iSOPClassUID, const char* iManufactureer, const char* iScanOption)
{
	return (AppComCacheReader::m_sortCTByScanOptionsFirst >= 1) &&
		   (iScanOption && *iScanOption) &&
		   (iManufactureer[0] && strnicmp(iManufactureer,"Siemens",7)) &&
		   IsSOPClassUIDCT(std::string(iSOPClassUID));
		   
}

//-----------------------------------------------------------------------------
AppComCacheReader::AppComCacheReader ()
{
	m_descriptionSize = 0;
	m_descriptionTime = 1;
	m_ignoreCompressedCache = 0;
 
#if defined (USE_STDIO)
	m_srcFile = 0;
#endif

	Clear ();

#if !defined (USE_HASH)
	// Create the cacheDataTokens Map
	// Vikram 12/27/01
	//
	if (m_cacheDataTokens.size () == 0)
	{
		m_cacheDataTokens[kSliceDescriptionStr]				= kSliceDescriptionIdx;
		m_cacheDataTokens[kEndOfSliceDescriptionStr]		= kEndOfSliceDescriptionIdx;
		m_cacheDataTokens[kModalityStr]						= kModalityIdx;
		m_cacheDataTokens[kSOPInstanceUIDStr]				= kSOPInstanceUIDIdx;
		m_cacheDataTokens[kBitsStoredStr]					= kBitsStoredIdx;
		m_cacheDataTokens[kBitsAllocatedStr]				= kBitsAllocatedIdx;
		m_cacheDataTokens[kHighBitStr]						= kHighBitIdx;
		m_cacheDataTokens[kSizeXStr]						= kSizeXIdx;
		m_cacheDataTokens[kSizeYStr]						= kSizeYIdx;
		m_cacheDataTokens[kStartOfDataInDataFileStr]		= kStartOfDataInDataFileIdx;
		m_cacheDataTokens[kSizeOfDataInBytesStr]			= kSizeOfDataInBytesIdx;
		m_cacheDataTokens[kImageTypeTokenStr]				= kImageTypeTokenIdx;

		m_cacheDataTokens[kImagePositionStr]				= kImagePositionIdx;
		m_cacheDataTokens[kImageOrientationStr]				= kImageOrientationIdx;
		m_cacheDataTokens[kPixelSpacingStr]					= kPixelSpacingIdx;
		m_cacheDataTokens[kNumberOfEntriesInHistogramStr]	= kNumberOfEntriesInHistogramIdx;
		m_cacheDataTokens[kHistogramStr]					= kHistogramIdx;


		m_cacheDataTokens[kRescaleSlopeStr]				    = kRescaleSlopeIdx;
		m_cacheDataTokens[kRescaleInterceptStr]			    = kRescaleInterceptIdx;

		m_cacheDataTokens[kReferencedSOPInstanceUIDStr]	    = kReferencedSOPInstanceUIDIdx;



		// CT
		m_cacheDataTokens[kModalityCTStr]					= kModalityCTIdx;
		// This is a type-2 DICOM field but we need it for Tri-phasic data
		m_cacheDataTokens[kImageNumberStr]				= kImageNumberIdx; 

		// MR
		m_cacheDataTokens[kModalityMRStr]					= kModalityMRIdx;

		// XA
		m_cacheDataTokens[kModalityXAStr]					= kModalityXAIdx;
		m_cacheDataTokens[kModalityXFStr]					= kModalityXFIdx;
		m_cacheDataTokens[kModalityRFStr]					= kModalityRFIdx;

		// CR/DR
		m_cacheDataTokens[kModalityCRStr]					= kModalityCRIdx;
		m_cacheDataTokens[kModalityDRStr]					= kModalityDRIdx;

		// SC
		m_cacheDataTokens[kModalitySCStr]				    = kModalitySCIdx;
		m_cacheDataTokens[kSamplesPerPixelStr]				= kSamplesPerPixelIdx;
		m_cacheDataTokens[kPhotometricInterpretationStr]	= kPhotometricInterpretationIdx;
		m_cacheDataTokens[kPlanarConfigurationStr]			= kPlanarConfigurationIdx;

        // SOP ClassUID
		m_cacheDataTokens[kSOPClassUIDStr]			        = kSOPClassUIDIdx;

        // OT
		m_cacheDataTokens[kModalityOTStr]			        = kModalityOTIdx;

        // US
		m_cacheDataTokens[kModalityUSStr]			        = kModalityUSIdx;
		m_cacheDataTokens[kModalityIVUSStr]			        = kModalityIVUSIdx;

		// Vikram 09/05/02 - Needed to handle signed data
		m_cacheDataTokens[kPixelRepresentationStr]			= kPixelRepresentationIdx;
		m_cacheDataTokens[kSliceMinVoxelValueStr]			= kSliceMinVoxelValueIdx;
		m_cacheDataTokens[kSliceMaxVoxelValueStr]			= kSliceMaxVoxelValueIdx;

		m_cacheDataTokens[kGlobalMinVoxelValueStr]			= kGlobalMinVoxelValueIdx;
		m_cacheDataTokens[kGlobalMaxVoxelValueStr]			= kGlobalMaxVoxelValueIdx;

		m_cacheDataTokens[kSliceCompressionFactorStr]			= kSliceCompressionFactorIdx;
		m_cacheDataTokens[kInputCompressionSizeXStr]			= kInputCompressionSizeXIdx;
		m_cacheDataTokens[kInputCompressionSizeYStr]			= kInputCompressionSizeYIdx;

		m_cacheDataTokens[kVOIWindowWidthStr]			= kVOIWindowWidthIdx;
		m_cacheDataTokens[kVOIWindowCenterStr]			= kVOIWindowCenterIdx;
		m_cacheDataTokens[kModalityLUTStr]				= kModalityLUT;

		// Vikram 07/22/04 - NM Support
		m_cacheDataTokens[kDetectorStr]					= kDetectorIdx;
		m_cacheDataTokens[kRotationStr]					= kRotationIdx;
		m_cacheDataTokens[kEnergyWindowStr				= kEnergyWindowIdx;
		m_cacheDataTokens[kPhaseStr]					= kPhaseIdx;
		m_cacheDataTokens[kRRIntervalStr]				= kRRIntervalIdx;
		m_cacheDataTokens[kTimeSlotStr]					= kTimeSlotIdx;
		m_cacheDataTokens[kSliceIndexStr]				= kSliceIndexIdx;
		m_cacheDataTokens[kAngularViewStr]				= kAngularViewIdx;
		m_cacheDataTokens[kTimeSliceStr]				= kTimeSliceIdx;
		m_cacheDataTokens[kHasValidOrientationStr]		= kHasValidOrientationIdx;
		m_cacheDataTokens[kIsRotationalStr]				= kIsRotationalIdx;

		m_cacheDataToken[kUsed1024]						= kUsed1024;

		m_cacheDataToken[kSliceThicknessStr]			= kSliceThicknessIdx;

		// -- 2006.05.06
		m_cacheDataToken[kImageDateStr]					= kImageDateIdx;
		m_cacheDataToken[kImageTimeStr]					= kImageTimeIdx;

		// -- 2006.06.06
		m_cacheDataToken[kPETDecayFactorStr]			= kPETDecayFactorIdx;
		m_cacheDataToken[kPETUnitsStr]					= kPETUnitsIdx;
		m_cacheDataToken[kPETTotalDoseStr]				= kPETTotalDoseIdx;
		m_cacheDataToken[kPETStartTimeStr]				= kPETStartTimeIdx;
		m_cacheDataToken[kPETHalfLifeStr]				= kPETHalfLifeIdx;
		m_cacheDataToken[kAcquisitionTimeStr]			= kAcquisitionTimeIdx;

		//Murali 2007.01.03
		m_cacheDataToken[kScanOptionsStr]				= kScanOptionsIdx;
		m_cacheDataToken[kManufacturerStr]				= kManufacturerIdx;

	}
#else
		int ut   = 0;
		unsigned int hash = 0;
		if (m_tokenToIdx.size () == 0)
		{

			hash               = AQNetHashString (std::string (kSliceDescriptionStr));
			m_tokenToIdx[hash] = kSliceDescriptionIdx; ut++;

			hash               = AQNetHashString (std::string (kEndOfSliceDescriptionStr));
			m_tokenToIdx[hash] = kEndOfSliceDescriptionIdx; ut++;


			hash               = AQNetHashString (std::string (kModalityStr));
			m_tokenToIdx[hash] = kModalityIdx; ut++;

			hash               = AQNetHashString (std::string (kSOPInstanceUIDStr));
			m_tokenToIdx[hash] = kSOPInstanceUIDIdx; ut++;


			hash               = AQNetHashString (std::string (kBitsStoredStr));
			m_tokenToIdx[hash] = kBitsStoredIdx; ut++;

			hash               = AQNetHashString (std::string (kBitsAllocatedStr));
			m_tokenToIdx[hash] = kBitsAllocatedIdx; ut++;

			hash               = AQNetHashString (std::string (kHighBitStr));
			m_tokenToIdx[hash] = kHighBitIdx; ut++;

			hash               = AQNetHashString (std::string (kSizeXStr));
			m_tokenToIdx[hash] = kSizeXIdx; ut++;

			hash               = AQNetHashString (std::string (kSizeYStr));
			m_tokenToIdx[hash] = kSizeYIdx; ut++;

			hash               = AQNetHashString (std::string (kStartOfDataInDataFileStr));
			m_tokenToIdx[hash] = kStartOfDataInDataFileIdx; ut++;

			hash               = AQNetHashString (std::string (kSizeOfDataInBytesStr));
			m_tokenToIdx[hash] = kSizeOfDataInBytesIdx; ut++;

			hash               = AQNetHashString (std::string (kImageTypeTokenStr));
			m_tokenToIdx[hash] = kImageTypeTokenIdx; ut++;
			
			hash               = AQNetHashString (std::string (kImagePositionStr));
			m_tokenToIdx[hash] = kImagePositionIdx; ut++;

			hash               = AQNetHashString (std::string (kImageOrientationStr));
			m_tokenToIdx[hash] = kImageOrientationIdx; ut++;

			hash               = AQNetHashString (std::string (kPixelSpacingStr));
			m_tokenToIdx[hash] = kPixelSpacingIdx; ut++;

			hash               = AQNetHashString (std::string (kNumberOfEntriesInHistogramStr));
			m_tokenToIdx[hash] = kNumberOfEntriesInHistogramIdx; ut++;

			hash               = AQNetHashString (std::string (kHistogramStr));
			m_tokenToIdx[hash] = kHistogramIdx; ut++;

			hash               = AQNetHashString (std::string (kRescaleSlopeStr));
			m_tokenToIdx[hash] = kRescaleSlopeIdx; ut++;

			hash               = AQNetHashString (std::string (kRescaleInterceptStr));
			m_tokenToIdx[hash] = kRescaleInterceptIdx; ut++;

			hash               = AQNetHashString (std::string (kReferencedSOPInstanceUIDStr));
			m_tokenToIdx[hash] = kReferencedSOPInstanceUIDIdx; ut++;


			// CT
			hash               = AQNetHashString (std::string (kModalityCTStr));
			m_tokenToIdx[hash] = kModalityCTIdx; ut++;

			// This is a type-2 DICOM field but we need it for Tri-phasic data
			hash               = AQNetHashString (std::string (kImageNumberStr));
			m_tokenToIdx[hash] = kImageNumberIdx; ut++;

			// MR
			hash               = AQNetHashString (std::string (kModalityMRStr));
			m_tokenToIdx[hash] = kModalityMRIdx; ut++;

			// XA
			hash               = AQNetHashString (std::string (kModalityXAStr));
			m_tokenToIdx[hash] = kModalityXAIdx; ut++;

			hash               = AQNetHashString (std::string (kModalityXFStr));
			m_tokenToIdx[hash] = kModalityXFIdx; ut++;

			hash               = AQNetHashString (std::string (kModalityRFStr));
			m_tokenToIdx[hash] = kModalityRFIdx; ut++;


			// CR/DR
			hash               = AQNetHashString (std::string (kModalityCRStr));
			m_tokenToIdx[hash] = kModalityCRIdx; ut++;

			hash               = AQNetHashString (std::string (kModalityDRStr));
			m_tokenToIdx[hash] = kModalityDRIdx; ut++;



			// SC
			hash               = AQNetHashString (std::string (kModalitySCStr));
			m_tokenToIdx[hash] = kModalitySCIdx; ut++;

			hash               = AQNetHashString (std::string (kSamplesPerPixelStr));
			m_tokenToIdx[hash] = kSamplesPerPixelIdx; ut++;

			hash               = AQNetHashString (std::string (kPhotometricInterpretationStr));
			m_tokenToIdx[hash] = kPhotometricInterpretationIdx; ut++;

			hash               = AQNetHashString (std::string (kPlanarConfigurationStr));
			m_tokenToIdx[hash] = kPlanarConfigurationIdx; ut++;



			// SOP ClassUID
			hash               = AQNetHashString (std::string (kSOPClassUIDStr));
			m_tokenToIdx[hash] = kSOPClassUIDIdx; ut++;


			// OT
			hash               = AQNetHashString (std::string (kModalityOTStr));
			m_tokenToIdx[hash] = kModalityOTIdx; ut++;


			// US
			hash               = AQNetHashString (std::string (kModalityUSStr));
			m_tokenToIdx[hash] = kModalityUSIdx; ut++;

			hash               = AQNetHashString (std::string (kModalityIVUSStr));
			m_tokenToIdx[hash] = kModalityIVUSIdx; ut++;


			// Vikram 09/05/02 - Needed to handle signed data
			hash               = AQNetHashString (std::string (kPixelRepresentationStr));
			m_tokenToIdx[hash] = kPixelRepresentationIdx; ut++;

			hash               = AQNetHashString (std::string (kSliceMinVoxelValueStr));
			m_tokenToIdx[hash] = kSliceMinVoxelValueIdx; ut++;

			hash               = AQNetHashString (std::string (kSliceMaxVoxelValueStr));
			m_tokenToIdx[hash] = kSliceMaxVoxelValueIdx; ut++;




			hash               = AQNetHashString (std::string (kGlobalMinVoxelValueStr));
			m_tokenToIdx[hash] = kGlobalMinVoxelValueIdx; ut++;

			hash               = AQNetHashString (std::string (kGlobalMaxVoxelValueStr));
			m_tokenToIdx[hash] = kGlobalMaxVoxelValueIdx; ut++;



			hash               = AQNetHashString (std::string (kSliceCompressionFactorStr));
			m_tokenToIdx[hash] = kSliceCompressionFactorIdx; ut++;

			hash               = AQNetHashString (std::string (kInputCompressionSizeXStr));
			m_tokenToIdx[hash] = kInputCompressionSizeXIdx; ut++;

			hash               = AQNetHashString (std::string (kInputCompressionSizeYStr));
			m_tokenToIdx[hash] = kInputCompressionSizeYIdx; ut++;


			hash               = AQNetHashString (std::string (kVOIWindowWidthStr));
			m_tokenToIdx[hash] = kVOIWindowWidthIdx; ut++;

			hash               = AQNetHashString (std::string (kVOIWindowCenterStr));
			m_tokenToIdx[hash] = kVOIWindowCenterIdx; ut++;

			hash               = AQNetHashString (std::string (kModalityLUTStr));
			m_tokenToIdx[hash] = kModalityLUT; ut++;


			// Vikram 07/22/04 - NM Support
			hash               = AQNetHashString (std::string (kDetectorStr));
			m_tokenToIdx[hash] = kDetectorIdx; ut++;

			hash               = AQNetHashString (std::string (kRotationStr));
			m_tokenToIdx[hash] = kRotationIdx; ut++;

			hash               = AQNetHashString (std::string (kEnergyWindowStr));
			m_tokenToIdx[hash] = kEnergyWindowIdx; ut++;

			hash               = AQNetHashString (std::string (kPhaseStr));
			m_tokenToIdx[hash] = kPhaseIdx; ut++;

			hash               = AQNetHashString (std::string (kRRIntervalStr));
			m_tokenToIdx[hash] = kRRIntervalIdx; ut++;

			hash               = AQNetHashString (std::string (kTimeSlotStr));
			m_tokenToIdx[hash] = kTimeSlotIdx; ut++;

			hash               = AQNetHashString (std::string (kSliceIndexStr));
			m_tokenToIdx[hash] = kSliceIndexIdx; ut++;

			hash               = AQNetHashString (std::string (kAngularViewStr));
			m_tokenToIdx[hash] = kAngularViewIdx; ut++;

			hash               = AQNetHashString (std::string (kTimeSliceStr));
			m_tokenToIdx[hash] = kTimeSliceIdx; ut++;

			hash               = AQNetHashString (std::string (kHasValidOrientationStr));
			m_tokenToIdx[hash] = kHasValidOrientationIdx; ut++;

			hash               = AQNetHashString (std::string (kIsRotationalStr));
			m_tokenToIdx[hash] = kIsRotationalIdx; ut++;

			hash				= AQNetHashString(std::string (kUsed1024Str));
			m_tokenToIdx[hash] = kUsed1024Idx; ut++;

			hash				= AQNetHashString(std::string (kSliceThicknessStr));
			m_tokenToIdx[hash] = kSliceThicknessIdx; ut++;
		
			// -- 2006.06
			hash				= AQNetHashString(std::string (kImageDateStr));
			m_tokenToIdx[hash] = kImageDateIdx; ut++;

			hash				= AQNetHashString(std::string (kImageTimeStr));
			m_tokenToIdx[hash] = kImageTimeIdx; ut++;

			hash				= AQNetHashString(std::string (kPETDecayFactorStr));
			m_tokenToIdx[hash] = kPETDecayFactorIdx; ut++;

			hash				= AQNetHashString(std::string (kPETUnitsStr));
			m_tokenToIdx[hash] = kPETUnitsIdx; ut++;


			hash				= AQNetHashString(std::string (kPETTotalDoseStr));
			m_tokenToIdx[hash] = kPETTotalDoseIdx; ut++;
			
			
			hash				= AQNetHashString(std::string (kPETStartTimeStr));
			m_tokenToIdx[hash] = kPETStartTimeIdx; ut++;
			
			
			hash				= AQNetHashString(std::string (kPETHalfLifeStr));
			m_tokenToIdx[hash] = kPETHalfLifeIdx; ut++;
			
			hash				= AQNetHashString(std::string (kAcquisitionTimeStr));
			m_tokenToIdx[hash] = kAcquisitionTimeIdx; ut++;

			// Murali 2007.01.03
			hash				= AQNetHashString(std::string (kScanOptionsStr));
			m_tokenToIdx[hash]	= kScanOptionsIdx; ut++;

			hash				= AQNetHashString(std::string (kManufacturerStr));
			m_tokenToIdx[hash]	= kManufacturerIdx; ut++;


			int sz = m_tokenToIdx.size();
//		    printf ("\n\n 777777777777777777777 %d %d\n", ut, sz);
			// Final check to make sure all the strings are unique
			assert (ut == m_tokenToIdx.size());
		}

#endif
}

//-----------------------------------------------------------------------------

AppComCacheReader::~AppComCacheReader ()
{
	Clear ();
}

//-----------------------------------------------------------------------------

void AppComCacheReader::Clear ()
{
	m_isCacheDescriptionRead = false;

	m_descriptionIsValidated = 0;

	m_cacheDataFilename = "";
	m_compressedL0CacheDataFilename = "";
	m_modality          = "";
	m_sopClassUID       = "";
	m_isMultiPhasic     = false;
	m_firstMultiPhasicGroupID  = -1;
	m_numberOfMultiPhasicGroups = 1;

	// Vikram 06/08/2005
	m_currentNumberOfMultiPhasicGroups = 1;
	m_currentFirstGroup = 0;

	m_firstMultiPhasicImageNumber = -999;

    m_currentSortType    = kSortUnknown;
    m_sortTypeOnStack    = kSortUnknown;

	m_isPruned = 0;


#if !defined (USE_STDIO)

    if (m_srcFileStream.is_open ()) m_srcFileStream.close();
	m_srcFileStream.clear ();

#else
	if (m_srcFile)
	{
		fclose (m_srcFile);
	}

	m_srcFile = 0;
#endif

    // Delete all the slices
    // Vikram 07/05/02 - Sub-series support
    int i = 0;
    for (i = 0; i < m_allSlices.size (); i++)
    {
        m_allSlices[i].DeleteData();
    }


	for (i = 0; i < m_groups.size (); i++)
	{
		m_groups[i].Clear();
	}

    m_allSlices.clear ();
    m_groups.clear ();
	m_sortedSOPToSliceLocation.clear();
	m_cacheDirectory = "";
	// Vikram 12/27/01
	m_uniqueSOPInstanceUIDS.clear ();


	m_L0CompressedCacheRead = false;

	// Release all the old slice buffer and mark them as useable
	for ( i = 0; i < kNumberOfIRTVSliceBuffers; i++)
	{
		m_sliceBuffers[i].FreeAllMemory (); 
		m_compressedSliceBuffers[i].FreeAllMemory();
	}
}


//-----------------------------------------------------------------------------
bool AppComCacheReader::IsValid(void) const
{
	return (m_allSlices.size() > 0 && !m_isPruned);
}

//-----------------------------------------------------------------------------
bool AppComCacheReader::DescriptionIsValidated(void) const
{
	 return IsValid() && m_descriptionIsValidated;
}

//-----------------------------------------------------------------------------
// This only returns the first 64 characters of the next line

#if !defined (USE_STDIO)
bool AppComCacheReader::GetNextValidLine (std::string& oLine, ifstream& iSrcFile)
{
	// This function strips out comments and 
	// blank lines and returns the next line
	// that needs to be parsed
	
	// Clear the string first
	//
	oLine.assign ("");	

	bool validLine = false;
	int ch = iSrcFile.peek();

	while (!validLine && ch != EOF) 
	{
		// Go past the leading white spaces
		
		
		// Get the next char
		ch = iSrcFile.get();
		while (ch == ' ' || ch == '\t')	ch = iSrcFile.get();

		if (ch != EOF)
		{
			int cnt = 0;
			validLine = true;

			// now new line, form feed and carriage return
			while (ch != '\n' && ch != '\f' && ch != '\r')
			{	
				oLine += ch;
				cnt ++;
				if(cnt > 2000) // !!! Is 2000 big enough?
				{
					// something wrong with description file
					ch = EOF;
					validLine = false;
					break;
				}
				ch =  iSrcFile.get();
			}
		}
		else
		{
			break;
		}
	}

	if (validLine)
		return true;
	else
		return false; // Reached the end of file
}
#else
//-----------------------------------------------------------------------------
bool AppComCacheReader::GetNextValidLine (std::string& oLine, FILE* iSrcFile)
{
	// This function strips out comments and 
	// blank lines and returns the next line
	// that needs to be parsed
	
	// Clear the string first
	//
	oLine.assign ("");	

	bool validLine = false;
	
	// Just peek at the next char
	int ch = getc (iSrcFile);
	
	ungetc (ch, iSrcFile);

	while (!validLine && ch != EOF) 
	{
		// Go past the leading white spaces
		
		
		// Get the next char
		ch = getc (iSrcFile);
		while (ch == ' ' || ch == '\t')	ch = getc (iSrcFile);

		if (ch != EOF)
		{
			int cnt = 0;
			validLine = true;

			// now new line, form feed and carriage return
			while (ch != '\n' && ch != '\f' && ch != '\r')
			{	
				oLine += ch;
				cnt ++;
				if(cnt > 2000) // !!! Is 2000 big enough?
				{
					// something wrong with description file
					ch = EOF;
					validLine = false;
					break;
				}
				ch =  getc (iSrcFile);
			}
		}
		else
		{
			break;
		}
	}

	if (validLine)
		return true;
	else
		return false; // Reached the end of file
}
//-----------------------------------------------------------------------------
bool AppComCacheReader::GetNextValidLine (char* oLine, FILE* iSrcFile)
{
	// This function strips out comments and 
	// blank lines and returns the next line
	// that needs to be parsed
	

	bool validLine = false;
	
	// Just peek at the next char
	int ch = getc (iSrcFile);
	
	ungetc (ch, iSrcFile);
	int cnt = 0;

	while (!validLine && ch != EOF) 
	{
		// Go past the leading white spaces
		
		
		// Get the next char
		ch = getc (iSrcFile);
		while (ch == ' ' || ch == '\t')	ch = getc (iSrcFile);

		if (ch != EOF)
		{
			cnt = 0;
			validLine = true;

			// now new line, form feed and carriage return
			while (ch != '\n' && ch != '\f' && ch != '\r')
			{	
				oLine[cnt] = ch;
				cnt ++;
				if(cnt > kLengthOfEachLineInBytes - 4) // !!! Is 768 big enough?
				{
					// something wrong with description file
					ch = EOF;
					oLine[cnt] = '\0';
					validLine = false;
					break;
				}
				ch =  getc (iSrcFile);
			}
			
		}
		else
		{
			break;
		}
	}

	if (validLine)
	{
		oLine[cnt] = '\0';
		return true;
	}
	else
		return false; // Reached the end of file
}
#endif
//-----------------------------------------------------------------------------
// Get the Keyword
int GetKeyword (std::string& iS, std::string& oKW)
{
	char* delim = " =";

	std::string::size_type start = iS.find_first_not_of (delim);
	std::string::size_type end = iS.find_first_of (delim, start);

	// Get the right substring
    oKW = iS.substr (start, end-start);

	// Convert to all small
	std::transform (oKW.begin(), oKW.end(), oKW.begin(), tolower);

	return kSuccess;
}

//-----------------------------------------------------------------------------

std::string GetValue (std::string iS)
{
	std::string::size_type idx = 0;
	idx = iS.find_first_of ("=");
	idx = iS.find_first_not_of (' ', idx+1);

	if (idx == std::string::npos)
	{
		return "";
	}
	else
		return iS.substr(idx);
}

// #define DO_READ_TIME
//-----------------------------------------------------------------------------
#if !defined (USE_STDIO)
int AppComCacheReader::ReadCacheDescription (const char* iCacheLocation,
												bool iProcessSlices,
												bool iForceReadCompressed)
{
	int status = kSuccess;

	// Name of the Cache File
	std::string lFilename = iCacheLocation;
	lFilename += "/";
	lFilename += kCacheDescriptionFileName; // Terarecon Cache description 
	
	
	// -- 2006.07.18
	// Moved this out of the sliceInfo to reduce the memory usage for
	// large datasets
	int numberOfEntriesInHistogram = 0;
	unsigned int histogram[4096];


	/* -- 4-30-2002
	 * we also need to check file size to make sure the file did
	 * not grow since the last time we read it
	 */
	unsigned fileModTime = TRPlatform::GetFileModificationTime(lFilename.c_str());
	unsigned fileSize	 = TRPlatform::GetFileSize(lFilename.c_str());

	if (fileSize == 0)
		return kErrFileStateIsNotGood;



	if (fileSize !=kCacheDataFileSize &&  
		fileSize !=kCacheDataFileSizeBig && 
		!m_cacheDirectory.empty() && 
		m_cacheDirectory.compare(iCacheLocation) == 0 &&
		m_descriptionSize == fileSize		&& 
		m_descriptionTime == fileModTime   && 

		// -- JULY-18-2003 
		// we have to have a mode where we always try to read cache
	 	((m_ignoreCompressedCache && !iForceReadCompressed) || m_L0CompressedCachePresent))
	{
		return kSuccess;
	}

#if defined (DO_READ_TIME)
	clock_t  start, finish;

	start = clock ();
#endif

#ifdef _DEBUG
	if (!m_cacheDirectory.empty() && m_cacheDirectory.compare(iCacheLocation) == 0 && m_L0CompressedCachePresent == true )
		cerr << "\n\nReading a growing cache file " << endl;
#endif
	
	m_descriptionSize = fileSize;
	m_descriptionTime = fileModTime;
	
	Clear ();
	// First Check to see if the directory exists
	if (::access(iCacheLocation, 2) != 0)
	{
		return kErrCannotAccessDirectory;
	}
	
	// Name of the Cache Directory
	m_cacheDirectory    = iCacheLocation;
	
	m_cacheDataFilename = iCacheLocation;
	m_cacheDataFilename += "/";
	m_cacheDataFilename += kCacheDataFileName;

	m_compressedL0CacheDataFilename = iCacheLocation;
	m_compressedL0CacheDataFilename += "/";
	m_compressedL0CacheDataFilename += kJ2kCacheL0DataFileName;

	
	iRTVPtrToSliceInformation psi;
#if !defined (USE_STDIO)
	m_srcFileStream.open (lFilename.c_str(), ios::binary | ios::in | ios::nocreate);

	if (!m_srcFileStream.is_open ())
	{
		Clear ();
		return kErrCannotOpenFile;
	}

#else
	m_srcFile = fopen (lFilename.c_str(), "rb");
	
	if (!m_srcFile)
	{
		Clear ();
		return kErrCannotOpenFile;
	}

#endif


	OnFileOpen(lFilename.c_str());

	int  errorCode = kSuccess;
	std::string str ("hello");
	std::string::size_type begin, end;;
	
	bool LookingHeader = true;
	int i, idx, toRead;
	char* delim;

#if !defined (USE_STDIO)
	while(GetNextValidLine (str, m_srcFileStream) && errorCode == kSuccess)
#else
	while(GetNextValidLine (str, m_srcFile) && errorCode == kSuccess)
#endif
	{
		//printf("[%s]\n", str.c_str());
		// Is this a token ?
		std::string::size_type iS = str.find_first_not_of ("<");
		if (iS == std::string::npos) // empty line skip it
			continue;
		
		delim = "> ";
		std::string::size_type iE = str.find_first_of (delim, iS);
		std::string token =  str.substr(iS, iE-iS);
		
#if !defined (USE_HASH)
		idx = m_cacheDataTokens[token];
#else
		idx = m_tokenToIdx[AQNetHashString(token)];
#endif
		if(LookingHeader && idx != kSliceDescriptionIdx)
		{
			errorCode = kErrFileStateIsNotGood;
			if(psi.m_ptr) delete psi.m_ptr;
			break;
		}

	
		switch (idx)
		{
			// Cache Description
		    case kSliceDescriptionIdx:
			    LookingHeader = false;
			    psi.m_ptr = new iRTVSliceInformation ();
			    iS = iE+1;
			    psi.m_ptr->m_version = str.substr(iS, str.find_first_of ('>', iS)-iS);
			break; 
			    
			    // Modality
		    case kModalityIdx:
			    m_modality = psi.m_ptr->m_modality = GetValue (str);
			break; 
			    
			    // SOP Instance UID
		    case kSOPInstanceUIDIdx:
			    psi.m_ptr->m_sopInstanceUID = GetValue (str);
			break; 
			    
			    // Bits Stored
		    case kBitsStoredIdx:
			    psi.m_ptr->m_bitsStored = atoi (GetValue (str).c_str());
			break; 
			    
			    // Bits Allocated
		    case kBitsAllocatedIdx:
			    psi.m_ptr->m_bitsAllocated = atoi (GetValue (str).c_str());
			break; 
			    
			    // High Bit
		    case kHighBitIdx:
			    psi.m_ptr->m_highBit = atoi (GetValue (str).c_str());
			break; 
			    
			    // X Size
		    case kSizeXIdx:
			    psi.m_ptr->m_sizeX = atoi (GetValue (str).c_str());
			break; 
			    
			    // Y Size
		    case kSizeYIdx:
			    psi.m_ptr->m_sizeY = atoi (GetValue (str).c_str());
			break; 
			    
			    // Start Of Data In Data File
		    case kStartOfDataInDataFileIdx:
			    psi.m_ptr->m_startOfDataInDataFile = _atoi64 (GetValue (str).c_str());
			break; 
			    
			    // Size Of Data In Data File
		    case kSizeOfDataInBytesIdx:
			    psi.m_ptr->m_sizeOfData = _atoi64 (GetValue (str).c_str());
			break; 
			    
			    // Image Type Tokens
		    case kImageTypeTokenIdx:
			    psi.m_ptr->m_imageTypeTokens.push_back(GetValue (str));
			break; 
			    
			    // CT rescale slope
		    case kRescaleSlopeIdx:
			    psi.m_ptr->m_rescaleSlope = atof (GetValue (str).c_str());
			break; 
			    
			    // Rescale intercept
		    case kRescaleInterceptIdx:
			    psi.m_ptr->m_rescaleIntercept = atof (GetValue (str).c_str());
			break; 
			    
			case kUsed1024Idx:
				psi.m_ptr->m_used1024 = atoi(GetValue(str).c_str());
			break;

			    // Image number
		    case kImageNumberIdx:
			    psi.m_ptr->m_imageNumber = atoi (GetValue (str).c_str());
			break; 
			    
			    // Referenced SOP instance UID - XA
		    case kReferencedSOPInstanceUIDIdx:
			    psi.m_ptr->m_referencedSOPInstanceUID = GetValue (str);
			break;
			    
			    // Image position
		    case kImagePositionIdx:
			    begin = str.find_first_of ("=");
			    begin = str.find_first_not_of (' ', begin+1);
			    end	  = str.find_first_of (' ', begin);
			    
			    psi.m_ptr->m_imagePosition[0] = atof (str.substr(begin, end-begin).c_str());
			    
			    begin = end;
			    end	  = str.find_first_of (' ', begin+1);
			    psi.m_ptr->m_imagePosition[1] = atof (str.substr(begin, end-begin).c_str());
			    
			    begin = end;
			    psi.m_ptr->m_imagePosition[2] = atof (str.substr(begin).c_str());
			break; 
			    
			    // Image orientation
		    case kImageOrientationIdx:
			    begin = str.find_first_of ("=");
			    begin = str.find_first_not_of (' ', begin+1);
			    end	  = str.find_first_of (' ', begin);
			    
			    psi.m_ptr->m_imageOrientation[0] = atof (str.substr(begin, end-begin).c_str());
			    
			    for (i = 1; i < 5; i++)
			    {
				    begin = end;
				    end	  = str.find_first_of (' ', begin+1);
				    psi.m_ptr->m_imageOrientation[i] = atof (str.substr(begin, end-begin).c_str());
			    }
			    
			    begin = end;
			    psi.m_ptr->m_imageOrientation[5] = atof (str.substr(begin).c_str());
			break; 
			    
			    // Image orientation
		    case kPixelSpacingIdx:
			    begin = str.find_first_of ("=");
			    begin = str.find_first_not_of (' ', begin+1);
			    end	  = str.find_first_of (' ', begin);
			    
			    psi.m_ptr->m_pixelSpacing[0] = atof (str.substr(begin, end-begin).c_str());
			    
			    begin = end;
			    psi.m_ptr->m_pixelSpacing[1] = atof (str.substr(begin).c_str());

				psi.m_ptr->m_hasPixelSpacing = 1;
			break; 
			    
			 // Number of entries in the histogram
			// -- 2006.07.18 use local copy of the histogram.
		    case kNumberOfEntriesInHistogramIdx:
			  //  psi.m_ptr->m_numberOfEntriesInHistogram = atoi (GetValue (str).c_str());
				psi.m_ptr->m_numberOfEntriesInHistogram = 0;
				numberOfEntriesInHistogram = atoi (GetValue (str).c_str());
			break;
			    
			    // Number of entries in the histogram
				// -- 2006.07.18 use local copy of the histogram.
		    case kHistogramIdx:
			    toRead = /*psi.m_ptr->m_*/numberOfEntriesInHistogram*sizeof(unsigned int);

#if !defined (USE_STDIO)
				m_srcFileStream.read ((char*)(/*psi.m_ptr->m_*/histogram), toRead);
#else
				fread ((char*)(/*psi.m_ptr->m_*/histogram), 1, toRead, m_srcFile);
#endif
			break;
			    
			    // Vikram 03/24/02
			    // SC Related 
			    // Number of entries in the histogram
		    case kSamplesPerPixelIdx:
			    psi.m_ptr->m_samplesPerPixel = atoi (GetValue (str).c_str());
			break;
			    
			    // Number of entries in the histogram
		    case kPhotometricInterpretationIdx:
			    psi.m_ptr->m_photometricInterpretation = atoi (GetValue (str).c_str());
			break;
			    
			    // Number of entries in the histogram
		    case kPlanarConfigurationIdx:
			    psi.m_ptr->m_planarConfiguration = atoi (GetValue (str).c_str());
			break;
			    // End SC Related  - Vikram 03/24/02
			    
			    // Vikram 05/01/02
			    // SOPClassUID
		    case kSOPClassUIDIdx:
			    m_sopClassUID = GetValue (str).c_str();
			    psi.m_ptr->m_sopClassUID = GetValue (str).c_str();
			break;
			    
			    // End Vikram05/01/02


			// Vikram 09/05/02 - Needed to handle signed data
		    case kPixelRepresentationIdx:
			    psi.m_ptr->m_pixelRepresentation = atoi (GetValue (str).c_str());
			break;

		    case kSliceMinVoxelValueIdx:
			    psi.m_ptr->m_sliceMinVoxelValue = atoi (GetValue (str).c_str());
			break;

		    case kSliceMaxVoxelValueIdx:
			    psi.m_ptr->m_sliceMaxVoxelValue = atoi (GetValue (str).c_str());
			break;

			// End Vikram 09/05/02 - Needed to handle signed data
			    
			    // End of slice description so put in the list
		    case kEndOfSliceDescriptionIdx:
			    m_uniqueSOPInstanceUIDS[psi.m_ptr->m_sopInstanceUID] = 1;

                // Vikram 07/05/02 Sub-Series support
                // At this point all we need to do is add it 
                // the list of all slices
                m_allSlices.push_back (psi);
			    
                psi.m_ptr = NULL;
			    LookingHeader = true;
		    break;
	
		    case kVOIWindowWidthIdx:
			    psi.m_ptr->m_voiWindowWidth = atof (GetValue (str).c_str());
				psi.m_ptr->m_hasWindowLevel++;
			break;

			case kVOIWindowCenterIdx:
			    psi.m_ptr->m_voiWindowCenter = atof (GetValue (str).c_str());
				psi.m_ptr->m_hasWindowLevel++;
			break;

			case kModalityLUT:
				psi.m_ptr->m_useModalityLUT = atoi(GetValue(str).c_str());
			break;

			case kDetectorIdx:
				psi.m_ptr->m_detector = atoi(GetValue(str).c_str());
			break;

			case kRotationIdx:
				psi.m_ptr->m_rotation = atoi(GetValue(str).c_str());
			break;

			case kEnergyWindowIdx:
				psi.m_ptr->m_energyWindow = atoi(GetValue(str).c_str());
			break;

			case kPhaseIdx:
				psi.m_ptr->m_phase = atoi(GetValue(str).c_str());
			break;

			case kRRIntervalIdx:
				psi.m_ptr->m_rrInterval = atoi(GetValue(str).c_str());
			break;

			case kTimeSlotIdx:
				psi.m_ptr->m_timeSlot = atoi(GetValue(str).c_str());
			break;

			case kSliceIndexIdx:
				psi.m_ptr->m_sliceIndex = atoi(GetValue(str).c_str());
			break;

			case kAngularViewIdx:
				psi.m_ptr->m_angularView = atoi(GetValue(str).c_str());
			break;

			case kTimeSliceIdx:
				psi.m_ptr->m_timeSlice = atoi(GetValue(str).c_str());
			break;

			case kHasValidOrientationIdx:
				psi.m_ptr->m_hasValidOrientation = atoi(GetValue(str).c_str());
			break;

			case kIsRotationalIdx:
				psi.m_ptr->m_isRotational = atoi(GetValue(str).c_str());
			break;

			case kSliceThicknessIdx:
				psi.m_ptr->m_sliceThickness = float(atof(GetValue(str).c_str()));
				psi.m_ptr->m_hasSliceThickness = 1;
			break;
			
			// -- 2006.05  Image Date and Time
			case kImageDateIdx:
				psi.m_ptr->m_imageDate = GetValue(str).c_str();
				psi.m_ptr->m_hasImageDateTime++;
				break;
				
			case kImageTimeIdx:
				psi.m_ptr->m_imageTime = GetValue(str).c_str();
				psi.m_ptr->m_hasImageDateTime++;
				break;

			// -- 2006.06.10 More PET SUV stuff
			case kPETTotalDoseIdx:
				psi.m_ptr->m_PETAttrib.m_totalDose = atoi(m_token[1]);
				break;
			case kPETStartTimeIdx:
				psi.m_ptr->m_PETAttrib.m_startTime = m_token[1];
				break;
			case kPETUnitsIdx:
				psi.m_ptr->m_PETAttrib.m_units = m_token[1];
				break;
			case kAcquisitionTimeIdx:
				psi.m_ptr->m_PETAttrib.m_acquisitionTime = m_token[1];
				break;
			case kPETHalfLifeIdx:
				psi.m_ptr->m_PETAttrib.m_halfLife = atoi(m_token[1]);
				break;
			case kPETDecayFactorIdx:
				psi.m_ptr->m_PETAttrib.m_decayFactor = atof(m_token[1]);
				break;

			case kScanOptionsIdx:
				psi.m_ptr->m_scanOptions = m_token[1];;
				break;

			case kManufacturerIdx:
				psi.m_ptr->m_manufacturer = m_token[1];;
				break;


		    default:
			    // garbage indescription file abort reading
			    errorCode = kErrFileStateIsNotGood;
			    if(psi.m_ptr) delete psi.m_ptr;
		    break;
		}
	}

#if !defined (USE_STDIO)
	m_srcFileStream.close ();
	m_srcFileStream.clear ();
#else
	fclose (m_srcFile);
	m_srcFile = 0;
#endif

#if defined (DO_READ_TIME)
	finish = clock ();
	double duration = (double)(finish-start)/ CLOCKS_PER_SEC;

	printf ("\n\n 88888888 %f\n\n", duration);
#endif


	if ((status = DescriptionDone()) != kSuccess)
		return status;

	// If Compressed Cache is present Read it here
	// Vikram 09/10/02
	status = ReadCompressedCacheDescription();
	
	if (status != kSuccess)
	{
		// -- 4-30-2002
		// need to reset the cachedirectory so we don't get stuck on
		// a bad cache
		m_cacheDirectory = "";
		if(errorCode != kSuccess)  return errorCode;
	}

	if (iProcessSlices)
	{
		status = ProcessAllSlices ();
		status = ProcessByImagePosition ();
	}	

	if (status != kSuccess || errorCode != kSuccess) 
	{	
		// -- 4-30-2002
		// need to reset the cachedirectory so we don't get stuck on
		// a bad cache
		m_cacheDirectory = "";
		if(errorCode != kSuccess)  return errorCode;
	}

	m_isCacheDescriptionRead = true;

#if defined (DO_READ_TIME)
	finish = clock ();
	 duration = (double)(finish-start)/ CLOCKS_PER_SEC;

	printf ("\n\n 90909090909 %f\n\n", duration);
#endif
	return status;
}
#else // #define USE_STDIO
//-----------------------------------------------------------------------------
int AppComCacheReader::ReadCacheDescription (const char* iCacheLocation, 
												bool iProcessSlices,
												bool iForceReadCompressed)
{
	int status = kSuccess;

#if 0 
	// Name of the Cache File
	std::string lFilename = iCacheLocation;
	lFilename += "/";
	lFilename += kCacheDescriptionFileName; // Terarecon Cache description 
	
	// -- 2006.07.18
	// Moved this out of the sliceInfo to reduce the memory usage for
	// large datasets
	int numberOfEntriesInHistogram = 0;
	unsigned int histogram[4096];


	/* -- 4-30-2002
	 * we also need to check file size to make sure the file did
	 * not grow since the last time we read it
	 */
	unsigned fileModTime = TRPlatform::GetFileModificationTime(lFilename.c_str());
	unsigned fileSize	 = TRPlatform::GetFileSize(lFilename.c_str());

	if (fileSize == 0)
		return kErrFileStateIsNotGood;


	if (fileSize !=kCacheDataFileSize &&  
		fileSize !=kCacheDataFileSizeBig && 
		!m_cacheDirectory.empty() && 
		m_cacheDirectory.compare(iCacheLocation) == 0 &&
		m_descriptionSize == fileSize		&& 
		m_descriptionTime == fileModTime   && 
	 	((m_ignoreCompressedCache && !iForceReadCompressed) || m_L0CompressedCachePresent))
	{
		return kSuccess;
	}

#if defined (DO_READ_TIME)
	clock_t  start, finish;

	start = clock ();
#endif

#ifdef _DEBUG
	if (!m_cacheDirectory.empty() && m_cacheDirectory.compare(iCacheLocation) == 0 && m_L0CompressedCachePresent == true )
		fprintf (stderr,"\n\nReading a growing cache file \n");
#endif
	
	m_descriptionSize = fileSize;
	m_descriptionTime = fileModTime;
	
	Clear ();
	// First Check to see if the directory exists
	if (::access(iCacheLocation, 2) != 0)
	{
		return kErrCannotAccessDirectory;
	}
	
	// Name of the Cache Directory
	m_cacheDirectory    = iCacheLocation;
	
	m_cacheDataFilename = iCacheLocation;
	m_cacheDataFilename += "/";
	m_cacheDataFilename += kCacheDataFileName;

	m_compressedL0CacheDataFilename = iCacheLocation;
	m_compressedL0CacheDataFilename += "/";
	m_compressedL0CacheDataFilename += kJ2kCacheL0DataFileName;

	
	iRTVPtrToSliceInformation psi;

	m_srcFile = fopen (lFilename.c_str(), "rb");
	
	if (!m_srcFile)
	{
		Clear ();
		return kErrCannotOpenFile;
	}

	OnOpenFile(lFilename.c_str());

	int  errorCode = kSuccess;

	// Allocate and clear the keyword value pairs
//	memset (kwv, 0, 8*kLengthOfkeywordValueInBytes);
//	memset (nextLine, 0, kLengthOfEachLineInBytes);

	bool LookingHeader = true;
	int i, idx, toRead;


	while(GetNextValidLine (m_nextLine, m_srcFile) && errorCode == kSuccess)
	{
	
		if (m_nextLine[0] == '<')
		{
			if (m_nextLine[1] != '/')
			{
				sprintf (m_token[0], "%s", kSliceDescriptionStr);
				sscanf (m_nextLine, "%*s %s>", m_token[1]);
			}
			else
			{
				sprintf (m_token[0], "%s", kEndOfSliceDescriptionStr);
			}
		}
		else
		{
			sscanf (m_nextLine, "%s = %s %s %s %s %s %s", m_token[0], m_token[1], m_token[2],
													      m_token[3], m_token[4], m_token[5], m_token[6]);
		}
		
			
#if !defined (USE_HASH)
		idx = m_cacheDataTokens[std::string (m_token[0])];
#else
		idx = m_tokenToIdx[AQNetHashString(m_token[0])];
#endif
		if(LookingHeader && idx != kSliceDescriptionIdx)
		{
			errorCode = kErrFileStateIsNotGood;
			if(psi.m_ptr) delete psi.m_ptr;
			break;
		}




		switch (idx)
		{
			// Cache Description
		    case kSliceDescriptionIdx:
			    LookingHeader = false;
//			    psi.m_ptr = new iRTVSliceInformation ();
//			    psi.m_ptr->m_version = m_token[1];
			break; 
			    
			    // Modality
		    case kModalityIdx:
			    m_modality = psi.m_ptr->m_modality = m_token[1];
			break; 
			    
			    // SOP Instance UID
		    case kSOPInstanceUIDIdx:
			    psi.m_ptr->m_sopInstanceUID = m_token[1];
			break; 
			    
			    // Bits Stored
		    case kBitsStoredIdx:
			    psi.m_ptr->m_bitsStored = atoi (m_token[1]);
			break; 
			    
			    // Bits Allocated
		    case kBitsAllocatedIdx:
			    psi.m_ptr->m_bitsAllocated = atoi (m_token[1]);
			break; 
			    
			    // High Bit
		    case kHighBitIdx:
			    psi.m_ptr->m_highBit = atoi (m_token[1]);
			break; 
			    
			    // X Size
		    case kSizeXIdx:
			    psi.m_ptr->m_sizeX = atoi (m_token[1]);
			break; 
			    
			    // Y Size
		    case kSizeYIdx:
			    psi.m_ptr->m_sizeY = atoi (m_token[1]);
			break; 
			    
			    // Start Of Data In Data File
		    case kStartOfDataInDataFileIdx:
			    psi.m_ptr->m_startOfDataInDataFile = _atoi64 (m_token[1]);
			break; 
			    
			    // Size Of Data In Data File
		    case kSizeOfDataInBytesIdx:
			    psi.m_ptr->m_sizeOfData = _atoi64 (m_token[1]);
			break; 
			    
			    // Image Type Tokens
		    case kImageTypeTokenIdx:
				psi.m_ptr->m_imageTypeTokens.push_back(std::string(m_token[1]));
			break; 
			    
			    // CT rescale slope
		    case kRescaleSlopeIdx:
			    psi.m_ptr->m_rescaleSlope = atof (m_token[1]);
			break; 
			    
			    // Rescale intercept
		    case kRescaleInterceptIdx:
			    psi.m_ptr->m_rescaleIntercept = atof (m_token[1]);
			break; 
			    
			case kUsed1024Idx:
				psi.m_ptr->m_used1024 = atoi(m_token[1]);
			break;
			    // Image number
		    case kImageNumberIdx:
			    psi.m_ptr->m_imageNumber = atoi (m_token[1]);
			break; 
			    
			    // Referenced SOP instance UID - XA
		    case kReferencedSOPInstanceUIDIdx:
				psi.m_ptr->m_referencedSOPInstanceUID = m_token[1];
			break;
			    
			    // Image position
		    case kImagePositionIdx:

			    psi.m_ptr->m_imagePosition[0] = atof (m_token[1]);
			    psi.m_ptr->m_imagePosition[1] = atof (m_token[2]);
			    psi.m_ptr->m_imagePosition[2] = atof (m_token[3]);
			break; 
			    
			    // Image orientation
		    case kImageOrientationIdx:		    
			    for (i = 0; i < 6; i++)
			    {
				    psi.m_ptr->m_imageOrientation[i] = atof (m_token[i+1]);
			    }
			break; 
			    
			    // Image orientation
		    case kPixelSpacingIdx:
			    psi.m_ptr->m_pixelSpacing[0] = atof (m_token[1]);			    
			    psi.m_ptr->m_pixelSpacing[1] = atof (m_token[2]);
				psi.m_ptr->m_hasPixelSpacing = 1;
			break; 
			    
			    // Number of entries in the histogram
				// -- 2006.07.18 use local copy of the histogram.
		    case kNumberOfEntriesInHistogramIdx:
			    /*psi.m_ptr->m_*/numberOfEntriesInHistogram = atoi (m_token[1]);
				 psi.m_ptr->m_numberOfEntriesInHistogram = 0;
			break;
			    
			    // Number of entries in the histogram
				// -- 2006.07.18 use local copy of the histogram.
		    case kHistogramIdx:
			    toRead = /*psi.m_ptr->m_*/numberOfEntriesInHistogram*sizeof(unsigned int);

				fread ((char*)(/*psi.m_ptr->m_*/histogram), 1, toRead, m_srcFile);
				getc (m_srcFile); // To move to the next line
			break;
			    
			    // Vikram 03/24/02
			    // SC Related 
			    // Number of entries in the histogram
		    case kSamplesPerPixelIdx:
			    psi.m_ptr->m_samplesPerPixel = atoi (m_token[1]);
			break;
			    
			    // Number of entries in the histogram
		    case kPhotometricInterpretationIdx:
			    psi.m_ptr->m_photometricInterpretation = atoi (m_token[1]);
			break;
			    
			    // Number of entries in the histogram
		    case kPlanarConfigurationIdx:
			    psi.m_ptr->m_planarConfiguration = atoi (m_token[1]);
			break;
			    // End SC Related  - Vikram 03/24/02
			    
			    // Vikram 05/01/02
			    // SOPClassUID
		    case kSOPClassUIDIdx:
			    m_sopClassUID = m_token[1];
			    psi.m_ptr->m_sopClassUID = m_token[1];
			break;
			    
			    // End Vikram05/01/02


			// Vikram 09/05/02 - Needed to handle signed data
		    case kPixelRepresentationIdx:
			    psi.m_ptr->m_pixelRepresentation = atoi (m_token[1]);
			break;

		    case kSliceMinVoxelValueIdx:
			    psi.m_ptr->m_sliceMinVoxelValue = atoi (m_token[1]);
			break;

		    case kSliceMaxVoxelValueIdx:
			    psi.m_ptr->m_sliceMaxVoxelValue = atoi (m_token[1]);
			break;

			// End Vikram 09/05/02 - Needed to handle signed data
			    
			    // End of slice description so put in the list
		    case kEndOfSliceDescriptionIdx:
			    m_uniqueSOPInstanceUIDS[psi.m_ptr->m_sopInstanceUID] = 1;

                // Vikram 07/05/02 Sub-Series support
                // At this point all we need to do is add it 
                // the list of all slices
                m_allSlices.push_back (psi);
			    
                psi.m_ptr = NULL;
			    LookingHeader = true;
		    break;
	
		    case kVOIWindowWidthIdx:
			    psi.m_ptr->m_voiWindowWidth = atof (m_token[1]);
				psi.m_ptr->m_hasWindowLevel++; // -- 2004.01.23
			break;

			case kVOIWindowCenterIdx:
			    psi.m_ptr->m_voiWindowCenter = atof (m_token[1]);
				psi.m_ptr->m_hasWindowLevel++; // -- 2004.01.23
			break;

			case kModalityLUT:
				psi.m_ptr->m_useModalityLUT = atoi(m_token[1]);
				break;

			// Vikram 07/22/04 - NM Support
			case kDetectorIdx:
				psi.m_ptr->m_detector = atoi(m_token[1]);;
			break;

			case kRotationIdx:
				psi.m_ptr->m_rotation = atoi(m_token[1]);;
			break;

			case kEnergyWindowIdx:
				psi.m_ptr->m_energyWindow = atoi(m_token[1]);;
			break;

			case kPhaseIdx:
				psi.m_ptr->m_phase = atoi(m_token[1]);;
			break;

			case kRRIntervalIdx:
				psi.m_ptr->m_rrInterval = atoi(m_token[1]);;
			break;

			case kTimeSlotIdx:
				psi.m_ptr->m_timeSlot = atoi(m_token[1]);;
			break;

			case kSliceIndexIdx:
				psi.m_ptr->m_sliceIndex = atoi(m_token[1]);;
			break;

			case kAngularViewIdx:
				psi.m_ptr->m_angularView = atoi(m_token[1]);;
			break;

			case kTimeSliceIdx:
				psi.m_ptr->m_timeSlice = atoi(m_token[1]);;
			break;

			case kHasValidOrientationIdx:
				psi.m_ptr->m_hasValidOrientation = atoi(m_token[1]);
			break;

			case kIsRotationalIdx:
				psi.m_ptr->m_isRotational = atoi(m_token[1]);
			break;
			
			case kSliceThicknessIdx:
				psi.m_ptr->m_sliceThickness = float(atof(m_token[1]));
				psi.m_ptr->m_hasSliceThickness = 1;
				break;
				
			// -- 2006.06 Added Image Date and Time
			case kImageDateIdx:
				psi.m_ptr->m_imageDate = m_token[1];
				break;
				
			case kImageTimeIdx:
				psi.m_ptr->m_imageTime = m_token[1];
				break;

			// -- 2006.06.10 Added SUV stuff
			case kPETTotalDoseIdx:
				psi.m_ptr->m_PETAttrib.m_totalDose = atoi(m_token[1]);
				break;
			case kPETStartTimeIdx:
				psi.m_ptr->m_PETAttrib.m_startTime = m_token[1];
				break;
			case kPETUnitsIdx:
				psi.m_ptr->m_PETAttrib.m_units = m_token[1];
				break;
			case kAcquisitionTimeIdx:
				psi.m_ptr->m_PETAttrib.m_acquisitionTime = m_token[1];
				break;
			case kPETHalfLifeIdx:
				psi.m_ptr->m_PETAttrib.m_halfLife = atoi(m_token[1]);
				break;
			case kPETDecayFactorIdx:
				psi.m_ptr->m_PETAttrib.m_decayFactor = atof(m_token[1]);
				break;

			case kScanOptionsIdx:
				psi.m_ptr->m_scanOptions = m_token[1];;
				break;

			case kManufacturerIdx:
				psi.m_ptr->m_manufacturer = m_token[1];;
				break;

		    default:

				// -- 2004.07.27
				// old code too restrictive for development/debugging
				if (!isalpha(m_token[0][0]))
				{
					// garbage indescription file abort reading
					errorCode = kErrFileStateIsNotGood;
					if(psi.m_ptr) delete psi.m_ptr;
				}
#ifdef _DEBUG
				else
				{
				   printf("CacheReader:: unhandled key %s\n",m_token[0]);
				}
#endif
		    break;
		}
	}


	fclose (m_srcFile);
	m_srcFile = 0;

	if ((status = DescriptionDone()) != kSuccess)
		return status;

#if defined (DO_READ_TIME)
	finish = clock ();
	double duration = (double)(finish-start)/ CLOCKS_PER_SEC;

	printf ("\n\n 88888888 %f\n\n", duration);
#endif
	// If Compressed Cache is present Read it here
	// Vikram 09/10/02
	status = ReadCompressedCacheDescription();
	
	if (status != kSuccess)
	{
		// -- 4-30-2002
		// need to reset the cachedirectory so we don't get stuck on
		// a bad cache
		m_cacheDirectory = "";
		if(errorCode != kSuccess)  return errorCode;
	}

	if (iProcessSlices)
	{
		status = ProcessAllSlices ();
		status = ProcessByImagePosition ();
	}	

	if (status != kSuccess || errorCode != kSuccess) 
	{	
		// -- 4-30-2002
		// need to reset the cachedirectory so we don't get stuck on
		// a bad cache
		m_cacheDirectory = "";
		if(errorCode != kSuccess)  return errorCode;
	}

	m_isCacheDescriptionRead = true;


	return kSuccess;


#if defined (DO_READ_TIME)
	finish = clock ();
	 duration = (double)(finish-start)/ CLOCKS_PER_SEC;

	printf ("\n\n 90909090909 %f\n\n", duration);
#endif

#endif
	return status;
}
#endif
//-----------------------------------------------------------------------------

/* on demand process/sort */
/* TCZ 2003-02-06 */
int AppComCacheReader::SetSortMethod(int iSort, int iPhase)
{
	
    if (iSort == kSortByImagePosition)
    {
        ProcessByImagePosition ();
    }
    else if (iSort == kSortByImageNumber)
	{
		ProcessByImageNumber(iPhase);
	}
	else  if (iSort == kSortAs2DSlices)
    {
        ProcessAs2DSlices();
    }
	else
	{
		return kErrSortType;
	}

	return 0;
}

// -- JULY-09-2003
bool AppComCacheReader::HasImageNumber(void) const
{
#if 0
	int N = m_allSlices.size();
	return m_allSlices[0].m_ptr->m_imageNumber != m_allSlices[N-1].m_ptr->m_imageNumber ||
		   m_allSlices[0].m_ptr->m_imageNumber != m_allSlices[N/2].m_ptr->m_imageNumber;
#else
	return false;
#endif
}


//----------------------------------------------------------------------------
// -- 2004.07.26
inline bool IsTomo(const char* iModality)
{
	return	strcmp(iModality,"CT") == 0 ||
			strcmp(iModality,"MR") == 0 ||
			strcmp(iModality,"PT") == 0 ||
			strcmp(iModality,"NM") == 0;
}

//-----------------------------------------------------------------------------
// This is called as soon as the Cache is read to process 
// the slice and create a list of slices that are sorted by 
// the axis of aquisition
//
// NOTE: To avoid processing multi-frame per SOPInstance modalities like XA and US
//       differently from single frame per SOPInstance modalities like CT/MR when 
//       we store enties into m_sortedSOPToSliceLocation each SOPInstanceUID will
//       be suffixed with a _frameNumber e.g. "_0"
//     
int AppComCacheReader::ProcessAllSlices ()
{
#if 1
	return -1;
#else
 
    int i = 0;
    // Vikram 07/5/02
    // 
    __int64 start = 0;
    __int64 size  = 0;

    int frameNumber = 0;
    std::string previousSOPInstanceUID = "";
    std::string sopInstanceUID = "";

    int numberOfSlices = m_allSlices.size();
    bool calculateScanType = false;

    if (IsTomo(m_modality.c_str()))
    {
        calculateScanType = true;
		bool sortByImageNumber = true;
		
		// If there image number is valid sort by image numer 
		// if not sort by SOPInstanceUID

		if (m_allSlices.size() > 1)
		{
		//	if (m_allSlices[0].m_ptr->m_imageNumber != m_allSlices[1].m_ptr->m_imageNumber)
			// the above is too weak as some breast MR data has duplicated image numbers
			if (HasImageNumber())
			{
				// Sort the elements by image  number
				//std::stable_sort (m_allSlices.begin (), m_allSlices.end (), iRTVPtrToSliceInformation::SortByImageNumber);
				// GL fix laod wrong subseries bug by add InstanceSOPUID as second condition
				std::stable_sort (m_allSlices.begin (), m_allSlices.end (), 
					iRTVPtrToSliceInformation::SortByImageNumberSOP);
			}
			else
			{
				// Sort the elements by SOPInstanceUID
				std::stable_sort (m_allSlices.begin (), m_allSlices.end (), iRTVPtrToSliceInformation::SortBySOPInstanceUID);
			}
		}

			
		// -- 2005.11.17
		// Need this to support 
		if (m_sortMRByScanTypeFirst && m_modality == "MR")
		{
			for (i = 0; i < numberOfSlices; i++)
			{
				m_allSlices[i].m_ptr->DetermineScanType();
			}
			
			SortByScanType();
		}

		// Murali 2007.01.03: Check for scan options sorting flag.
		bool bSortUsingScanOptions = IsScanOptionsUsedForSorting();	
		if(bSortUsingScanOptions)
		{
			std::stable_sort (m_allSlices.begin (), m_allSlices.end (), iRTVPtrToSliceInformation::SortByScanOptions);	
			
		}

		// Vikram 07/17/2006 Fix for data with no corelation with ImageNumber and ImagePosition
		if (!bSortUsingScanOptions && m_presortCTByImagePositionAfterImageNumber && m_modality == "CT")
		{
			for (i = 0; i < numberOfSlices; i++)
			{
				m_allSlices[i].m_ptr->DetermineScanType();
			}

			std::stable_sort (m_allSlices.begin (), m_allSlices.end (), iRTVPtrToSliceInformation::SortByImagePosition);	
		}
    }
	// -- 2005.06.08
	// even if not tomo, we still need to sort the slices into some kind of order,
	// say, image number or SOP. Otherwise the C-STORE sequence will affect
	// the image order
	else
	{
		if (m_allSlices.size() > 1)
			std::stable_sort (m_allSlices.begin (), m_allSlices.end (), 
			iRTVPtrToSliceInformation::SortByImageNumberSOP);
	}

    for (i = 0; i < numberOfSlices; i++)
    {
       m_allSlices[i].m_ptr->MakeImageTypeTokensIntoOneString ();

		m_allSlices[i].m_ptr->m_sequence = i; // -- 2003-07-09
#ifdef _DEBUG
		int sn = m_allSlices[i].m_ptr->m_imageNumber;
#endif

        start = m_allSlices[i].m_ptr->m_startOfDataInDataFile;
        size  = m_allSlices[i].m_ptr->m_sizeOfData;
        
        // Calculate the scan Type per slice
        if (calculateScanType) m_allSlices[i].m_ptr->DetermineScanType();

        // We only increment the frame number if the previous
        // SOPInstanceUID is the same. Remember that XA Multi-Frame is 
        // sequential - Vikram 07/08/02
        sopInstanceUID = m_allSlices[i].m_ptr->m_sopInstanceUID;

        if (sopInstanceUID.compare (previousSOPInstanceUID) == 0)
        {
            frameNumber ++;
        }
        else
            frameNumber = 0;
        
        // Copy the raw SOPInstanceUID
        previousSOPInstanceUID = sopInstanceUID;
      
		//	char buffer[64];
		//	sprintf (buffer, "_%06d",frameNumber);
		//	sopInstanceUID += buffer;

		TransformMultiFrameSOPInstanceUID(sopInstanceUID,frameNumber);

		/* -- 07/31/2002
		 * indexInVectorOfSlices is used as a direct index, can't use i+1.
		 * changed to i
		 */
        m_sortedSOPToSliceLocation[sopInstanceUID] = iRTVSOPLocation(i/*i+1*/, start, size);
    }   

    return kSuccess;
#endif

}

// -- 2005.11.17
// Consolidate common functions to support ScanType sort
//-----------------------------------------------------------------------------
void AppComCacheReader::SortByImageNumber(void)
{
#if 0
	if (m_allSlices.size() > 1)
	{
		//	if (m_allSlices[0].m_ptr->m_imageNumber != m_allSlices[1].m_ptr->m_imageNumber)
		// the above is too weak as some breast MR data has duplicated image numbers
		if (HasImageNumber())
		{
			// Sort the elements by image  number
			//std::stable_sort (m_allSlices.begin (), m_allSlices.end (), iRTVPtrToSliceInformation::SortByImageNumber);
			// GL fix laod wrong subseries bug by add InstanceSOPUID as second condition
			std::stable_sort (m_allSlices.begin (), m_allSlices.end (), 
				iRTVPtrToSliceInformation::SortByImageNumberSOP);
		}
		else
		{
			// Sort the elements by SOPInstanceUID
			std::stable_sort (m_allSlices.begin (), m_allSlices.end (), iRTVPtrToSliceInformation::SortBySOPInstanceUID);
		}
	}
#endif
}

// -- 2005.11.17
// Consolidate common functions to support ScanType sort
//--------------------------------------------------------------------
void AppComCacheReader::SortByScanType(void)
{
#if 0
	// Vikram11/16/2005 Fix for Cox Health MR data that has an MR series that
	// first has axial slices starting with image number 1 and then coronal slices
	// Starting with image number 1. So sorting by scan type after sorting by Image number 
	// puts all the slices in scan order before they can be sorted helps this case
	std::stable_sort (m_allSlices.begin (), m_allSlices.end (), 
		iRTVPtrToSliceInformation::SortByScanType);
#endif
	
}

//-----------------------------------------------------------------------------
// -- 2006.02.13
// Not the best way to check. Should optimize by replacing this with
// a check while reading.
static bool AllSliceIsSOPClassSC(std::vector<iRTVPtrToSliceInformation>& allSlices)
{
	  int N = allSlices.size();
	  for ( int i = 0; i < N; i++)
	  {
		  if (!IsSOPClassUIDSC(std::string(allSlices[i].GetSOPClassUID())))
				return false;
	  }

	  return true;
}

//-----------------------------------------------------------------------------
// Create groups and sub groups based on Axis of aquisition
//
//#include "rtvsSubSeriesSorter.h"
int AppComCacheReader::ProcessByImagePosition ()
{
    int numberOfSlices = m_allSlices.size();

#if 0
	// -- 2004.01.21 
	// this can happen for bad description or data file
	if (numberOfSlices == 0)
	{
 		assert(numberOfSlices);
		return -1;
	}

#if defined( _DEBUG) && 1
	// tcz 2005.06.10 can be useful in debugging
	for ( int j = 0; j < numberOfSlices; ++j)
	{
		fprintf(stderr,"slice=%3d imageNumber=%d\n", j+1, m_allSlices[j].m_ptr->m_imageNumber);
	}
#endif

	if (m_currentSortType == kSortByImagePosition)
	{
		return kSuccess;
	}

		// -- 2005.11.17
	// When m_sortMRByScanTypeFirst is off, we should turn off all new sort
	if (m_sortMRByScanTypeFirst && m_modality == "MR")
	{
		this->SortByScanType();
		this->GenerateSortedIndex();
	}
 

	iRTVSubSeriesSorter sorter;

    m_groups.clear ();
    m_currentSortType = kSortByImagePosition;
	m_isMultiPhasic = false;
	m_numberOfMultiPhasicGroups = 1;
	
	// Vikram 06/08/2005
	m_currentNumberOfMultiPhasicGroups = 1;
	m_currentFirstGroup = 0;
	
	m_currentPhases = -1;

    // Create a new group with the first slice
    CreateNewGroup (m_allSlices[0]);
    
    int i = 0;

	// -- 2006.02.13
	// We have a logic problem here. We must check all slices to be SC otherwise
	// we will have to leave the sorting by the modality as SC sorting just puts every
	// slice into the same group.
    if (IsSOPClassUIDSC(m_sopClassUID) &&      // SC
		AllSliceIsSOPClassSC(m_allSlices))
    {
	
        for (i = 1; i < numberOfSlices; i++)
        {
            AddSCToGroup (m_allSlices[i]);
        }
    }
	else
    if (IsModalityCT(m_modality) || IsModalityPT(m_modality))       // CT
    {

		for (i = 1; i < numberOfSlices; i++)
        {
            AddCTToGroupByImagePosition (m_allSlices[i]);
        }
		
		// -- 2003-07-07 this fixes the interleaved multiphasic data
		sorter.Consolidate(m_groups);
    }
    else    
	if (IsModalityMR(m_modality))       // MR
		{

#if 0		
			// -- 2005.11.17
			// This screws up preview and rotational series. Need to move this out
			// to an earlier point in the sorting chain
			if (m_sortMRByScanTypeFirst)
			{
				// Vikram11/16/2005 Fix for Cox Health MR data that has an MR series that
				// first has axial slices starting with image number 1 and then coronal slices
				// Starting with image number 1. So sorting by scan type after sorting by Image number 
				// puts all the slices in scan order before they can be sorted helps this case
				std::stable_sort (m_allSlices.begin (), m_allSlices.end (), 
					iRTVPtrToSliceInformation::SortByScanType);
				
			}
#endif	   
			for (i = 1; i < numberOfSlices; i++)
			{
				AddMRToGroupByImagePosition (m_allSlices[i]);
			}
			
			// -- 2003-07-07 this fixes the interleaved multiphasic data
		 	sorter.Consolidate(m_groups);
		}
    else    
    if (IsModalityXA(m_modality))       // XA
    {
        for (i = 1; i < numberOfSlices; i++)
        {
            AddXAToGroup (m_allSlices[i]);
        }
    }    
	else
	if (IsModalityUS(m_modality))       // US/IVUS
	{
		for (i = 1; i < numberOfSlices; i++)
        {
            AddUSToGroup (m_allSlices[i]);
        }
		sorter.ConsolidateUS(m_groups); // -- 2004.04.21
	}
	else
	if (IsModalityNM(m_modality))       // NM support -- 2004.05.11
	{
		for (i = 1; i < numberOfSlices; i++)
        {
            AddNMToGroup (m_allSlices[i]);
        }
	//	sorter.ConsolidateUS(m_groups); // -- 2004.04.21
	}
    else                        // CR/DR/OT/
    {
        for (i = 1; i < numberOfSlices; i++)
        {
            AddRestOfTheModalitiesToGroup (m_allSlices[i]);
        }
		
    }

    ProcessGroups ();

#endif
    return kSuccess;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This Function first sorts the the entire series by its instance number
// The general criteria to form new groups are as follows:
// - If the image type tokens are different  and iPhases == 1
// - If iPhases is more other than 1 than every iPhase is a new group
int AppComCacheReader::ProcessByImageNumber (int iPhases)
{
    int i = 0;

	if (iPhases > m_allSlices.size ())
	{
		return kErrBadInputParameters;
	}

	// -- 2004.01.21 
	// this can happen for bad description or data file
	if (m_allSlices.size () == 0)
	{
		assert(m_allSlices.size ());
		return -1;
	}

	if (m_currentSortType == kSortByImageNumber && m_currentPhases == iPhases)
	{
		return kSuccess;
	}

	// -- 2005.11.17
	// When m_sortMRByScanTypeFirst is off, we should turn off all new sort
	if (m_sortMRByScanTypeFirst && m_modality == "MR")
	{
		SortByImageNumber();
		GenerateSortedIndex();
	}

 
    if (IsTomo(m_modality.c_str()) )
    {
         // Create the  necessary groups
        m_groups.clear ();
        m_currentSortType = kSortByImageNumber;
		m_currentPhases = iPhases;

        if (iPhases > 1)
        {
            for (i = 0; i < iPhases; i++)
            {
                CreateNewGroup (m_allSlices[i]);
            }
    
            // Now add each nth element into a new group
            for (i = iPhases; i < m_allSlices.size(); i++)
            {
                m_groups[i%iPhases].AddToGroup(m_allSlices[i], true);
            }
        }
        else // iPhase == 1
        {
            CreateNewGroup (m_allSlices[0]);

            // Now add each nth element into a new group
            for (i = 1; i < m_allSlices.size(); i++)
            {
                AddCTMRToGroupByImageNumber (m_allSlices[i]);
            }
        }

        ProcessGroups ();
    }

    return kSuccess;
}

//-----------------------------------------------------------------------------
int AppComCacheReader::ProcessAs2DSlices ()
{
    int i = 0;

	if (m_currentSortType == kSortAs2DSlices)
	{
		return kSuccess;
	}

 	// -- 2005.11.17
	// When m_sortMRByScanTypeFirst is off, we should turn off all new sort
	if (m_sortMRByScanTypeFirst && m_modality == "MR")
	{
		SortByImageNumber();
		GenerateSortedIndex();
	}

    if (IsModalityCT(m_modality) || IsModalityMR (m_modality) || IsModalityPT(m_modality) || IsModalityNM(m_modality))
    {
         // Create the  necessary groups
        m_groups.clear ();
        m_currentSortType = kSortAs2DSlices;

        CreateNewGroup (m_allSlices[0]);

        // Now add each nth element into a new group
        for (i = 1; i < m_allSlices.size(); i++)
        {
            AddCTMRToGroupAs2DSlices (m_allSlices[i]);
        }

        ProcessGroups ();
    }

    return kSuccess;
}
//-----------------------------------------------------------------------------
// This function loads a list of SOPInstance UID's. This list is still sorted by position
// 
int AppComCacheReader::ProcessByListOfSOPInstanceUIDs (std::vector<std::string>& iSOPInstanceUIDs)
{
    int i = 0;

    m_groups.clear ();
    m_currentSortType = kSortByListOfSOPInstanceUIDs;
	
    int numberOfSOPs = iSOPInstanceUIDs.size();
    iRTVSOPLocation sopLocation;

    sopLocation = Find (iSOPInstanceUIDs[0].c_str(), 0);

    if (sopLocation.Bad())
    {
        return kErrSortType;
    }
    else
	{
        CreateNewGroup(m_allSlices[sopLocation.m_indexInVectorOfSlices]);
	}

    for (i = 1; i < numberOfSOPs; i ++)
    {
        sopLocation = Find (iSOPInstanceUIDs[i].c_str(), 0);

	//	fprintf(stderr,"num=%3d index=%3d SOP=%s\n",i,sopLocation.m_indexInVectorOfSlices,iSOPInstanceUIDs[i].c_str());

        if (sopLocation.Bad())
        {
            return kErrSortType;
        }
        else
		{
            m_groups[0].AddToGroup (m_allSlices[sopLocation.m_indexInVectorOfSlices]);
		}
            
    }
 
    ProcessGroups ();
    return kSuccess;
}
//-----------------------------------------------------------------------------

int AppComCacheReader::AddCTToGroupByImagePosition (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;
    int i = 0;
	bool sliceHasBeenAdded = false;

#if 0
	int gi, k, delta;

	// -- 2/26/2003
	// take care of multiphasic data in 111222 or 123 456
	if (m_isMultiPhasic)
	{
		gi = 0;
		delta = 1;
	}
	else
	{
		// Vikram 08/21/2006  Sorting Fix for Stanford CT Runoff - Partially interleaved data 
		/*      
				Image Number    Axial Image Position
				2238,			-1399.2000
				1593,			-1398.6000
				2237,			-1398.4000
				1592,			-1397.8000
				2236,			-1397.6000
				1591,			-1397.0000
				2235,			-1396.8000
				1590,			-1396.2000
		*/

		if (m_presortCTByImagePositionAfterImageNumber)
		{
			gi = 0;
			delta = 1;
		}
		else
		{
			gi = m_groups.size() - 1;
			delta = -1;
		}
	}
 
	
	int skipMultiPhasicIndexing = !ShouldSortByScanOption(iPSi.GetSOPClassUID(),
		                         iPSi.m_ptr->m_manufacturer.c_str(),iPSi.m_ptr->m_scanOptions.c_str());
 

    for ( k = 0, i = gi; k < m_groups.size(); k++, i += delta)
	{
		
		if ((m_groups[i].m_imageTypeTokensAsOneString.compare (iPSi.m_ptr->m_imageTypeTokensAsOneString) == 0) &&
            (m_groups[i].m_scanType == iPSi.m_ptr->m_scanType) &&
			(m_groups[i].m_sopClassUID == iPSi.m_ptr->m_sopClassUID)) // tcz 2005.08.12
		{
			// Here we need to Check to see other things to detect 
			// multi-phasic Cardiac data sets - Vikram Simha
			// If the Image Postition is the same then it is a different
			// phase
			// Here we always know that there is atleas one group and 
			// atleast one slice in it.
			
			double ip[3];

			
			ip[0] = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imagePosition[0];
			ip[1] = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imagePosition[1];
			ip[2] = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imagePosition[2];

			// Vikram 08/16/2006 - Cannot use the same tolerance as we use for slice spacing. Especially 
			// with the new 64 slice scanners producing datasets that have 0.1 mm spacing.

			float kToleranceForMultiphasicDetection = 0.01f;

//			if ((fabs (ip[0] - iPSi.m_ptr->m_imagePosition[0]) <= iRTVSliceInformation::GetImagePostionTolerance()) &&
//				(fabs (ip[1] - iPSi.m_ptr->m_imagePosition[1]) <= iRTVSliceInformation::GetImagePostionTolerance()) &&
//				(fabs (ip[2] - iPSi.m_ptr->m_imagePosition[2]) <= iRTVSliceInformation::GetImagePostionTolerance()) &&
//				(m_groups[i].m_ptrToAllSlices.size() == 1))

			if ((fabs (ip[0] - iPSi.m_ptr->m_imagePosition[0]) <= kToleranceForMultiphasicDetection) &&
				(fabs (ip[1] - iPSi.m_ptr->m_imagePosition[1]) <= kToleranceForMultiphasicDetection) &&
				(fabs (ip[2] - iPSi.m_ptr->m_imagePosition[2]) <= kToleranceForMultiphasicDetection) &&
				(m_groups[i].m_ptrToAllSlices.size() == 1))
			{
				// The data set is multi-phasic 
				if (!m_isMultiPhasic)
				{
					m_isMultiPhasic = true;	
					m_firstMultiPhasicGroupID  = i;
//					m_firstMultiPhasicImageNumber = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imageNumber;
					m_firstMultiPhasicImageNumber = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_sequence;
				}

					m_numberOfMultiPhasicGroups++;
					m_currentNumberOfMultiPhasicGroups ++;
					needToCreateNewGroup = true;
				break;
			}
			else
			{
				int idx = i;
				if (m_isMultiPhasic && !skipMultiPhasicIndexing)
				{
					// We need more logic here is the data contains 
					// multi-phasic data and other image type tokens.
					// 
//					idx = (iPSi.m_ptr->m_imageNumber-m_firstMultiPhasicImageNumber) % m_numberOfMultiPhasicGroups;
					idx = (iPSi.m_ptr->m_sequence-m_firstMultiPhasicImageNumber) % m_numberOfMultiPhasicGroups;
					idx += m_firstMultiPhasicGroupID;
				}


				if (m_groups[idx].AddToGroup (iPSi))
				{
					needToCreateNewGroup = false;
					sliceHasBeenAdded = true;
					break;
				}
			}
		}
	}

    // Create New group
	if (needToCreateNewGroup || !sliceHasBeenAdded)
	{
		CreateNewGroup (iPSi);
	}

#endif
	return kSuccess;
}

//-----------------------------------------------------------------------------

int AppComCacheReader::AddCTMRToGroupByImageNumber (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
    for (int i = 0; i < m_groups.size (); i++)
	{
		if (m_groups[i].m_imageTypeTokensAsOneString.compare (iPSi.m_ptr->m_imageTypeTokensAsOneString) == 0)
		{
			// Vikram 02/20/03 - make sure the size of all the slice in the group are the same
			if (m_groups[i].m_sizeX == iPSi.m_ptr->m_sizeX &&
				m_groups[i].m_sizeY == iPSi.m_ptr->m_sizeY &&
				// -- 2005.07.07 Need to check bitdepth as well
				m_groups[i].m_bitsStored == iPSi.m_ptr->m_bitsStored &&
				m_groups[i].m_samplesPerPixel == iPSi.m_ptr->m_samplesPerPixel)
			{
				m_groups[i].m_ptrToAllSlices.push_back (iPSi);
				needToCreateNewGroup = false;
				break;
			}
		}
	}

    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
#endif
	return kSuccess;
}

//-----------------------------------------------------------------------------
// Vikram 06/13/03 Pure 2D Rendering
int AppComCacheReader::AddCTMRToGroupAs2DSlices (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
    for (int i = 0; i < m_groups.size (); i++)
	{
		if (m_groups[i].m_sizeX == iPSi.m_ptr->m_sizeX &&
			m_groups[i].m_sizeY == iPSi.m_ptr->m_sizeY &&
			m_groups[i].m_bitsStored == iPSi.m_ptr->m_bitsStored &&
			m_groups[i].m_highBit == iPSi.m_ptr->m_highBit &&
			m_groups[i].m_samplesPerPixel == iPSi.m_ptr->m_samplesPerPixel)

		{
			m_groups[i].m_ptrToAllSlices.push_back (iPSi);
			needToCreateNewGroup = false;
			break;
		}
	}

    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
#endif
	return kSuccess;
}




//-----------------------------------------------------------------------------

int AppComCacheReader::AddMRToGroupByImagePosition (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;
    int i = 0;
	bool sliceHasBeenAdded = false;

#if 0
 	std::string::size_type  iS;
	iS = iPSi.m_ptr->m_imageTypeTokensAsOneString.find ("DERIVED");

	// -- JULY-09-2003  take care of 1122 and 1212 multiphasic
	int gi = 0, k, delta = 1;
	if (!m_isMultiPhasic)
	{
		m_currentFirstGroup = gi = m_groups.size() - 1;
		delta = -1;

		// Vikram 06/08/2005
//		m_currentFirstGroup = 0;
	}



	// Vikram 06/08/2005 1212_3434_5656 Fix
	// Solving the problem of MR datasets which have a single series
	// but are 12 12 12 34 34 34 56 56 56 - Data from Albert Einstien Patient 1212_3434_5656
	// To sort a data set like this we need to have the concept of the current set of groups that 
	// we are adding sliec too.
	// It would look something like this
	//
	// - Start off with the currentFirstGroup = currentLastGroup = 0 
	// - If we have the patter 123 123 123 123 what happens is the first 3 slices are the same and we keep 
	//   incrementing the currentLastGroup until we have added a second slice into the group
	//
	// - In the 12 12 34 34 56 56 case what happens is one adds several slices and suddenly 
	//   you come across a slice that has the same position as the begining slice of the first group
	//   So Now
	// - You need to set the currentFirstGroup and currentLastGroup to the new group index just created
	// - Increment the currentLastGroup until you add a second slice into the currentFirstGroup
	
	// Vikram 06/08/2005 1212_3434_5656 Fix
	bool resetMultiPhasicGroupID_1212_3434_5656Case = false;
	gi = m_currentFirstGroup;

	// Murali 2006.12.19 :Required to reset the multiphasic group info, when the scanType of incoming slice is different.
	// This approach works only if all the slices are sorted using scan type already before invoking this method.
	bool bIsScanTypeDifferent = true;

    for ( k = m_currentFirstGroup, i = gi; k < m_groups.size(); k++, i += delta)
	{

		// -- JULY-09-2003
		// We should not care about derived image types. What matter is a group should
		// not contain different types.
 
		if (iS != std::string::npos) 
		{
			// If is of Derived type
			if (m_groups[i].m_imageTypeTokensAsOneString.compare (iPSi.m_ptr->m_imageTypeTokensAsOneString) == 0) 
			{
				if (m_groups[i].AddToGroup (iPSi))
				{
					needToCreateNewGroup = false;
					sliceHasBeenAdded = true;
					break;
				}
			}
		}
		else
 
		if ((m_groups[i].m_imageTypeTokensAsOneString.compare (iPSi.m_ptr->m_imageTypeTokensAsOneString) == 0) 
			&& (m_groups[i].m_scanType == iPSi.m_ptr->m_scanType))
		{

			bIsScanTypeDifferent = false;


			// Here we need to Check to see other things to detect 
			// multi-phasic Cardiac data sets - Vikram Simha
			// If the Image Postition is the same then it is a different
			// phase
			// Here we always know that there is atleas one group and 
			// atleast one slice in it.
			
			double ip[3];

			
			ip[0] = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imagePosition[0];
			ip[1] = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imagePosition[1];
			ip[2] = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imagePosition[2];

			// The dataset is multi-phasic only if the group size is ==1
			// if not its not multi-phasic

			if ((fabs (ip[0] - iPSi.m_ptr->m_imagePosition[0]) <= iRTVSliceInformation::GetImagePostionTolerance()) &&
				(fabs (ip[1] - iPSi.m_ptr->m_imagePosition[1]) <= iRTVSliceInformation::GetImagePostionTolerance()) &&
				(fabs (ip[2] - iPSi.m_ptr->m_imagePosition[2]) <= iRTVSliceInformation::GetImagePostionTolerance()) &&
				(m_groups[i].m_ptrToAllSlices.size() == 1))
			{
					// The data set is multi-phasic 
					if (!m_isMultiPhasic)
					{
						m_isMultiPhasic = true;	
						m_currentFirstGroup = m_firstMultiPhasicGroupID  = i;
						// --  JULY-09-2003 can't rely on image number
				//		m_firstMultiPhasicImageNumber = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_imageNumber;
						m_firstMultiPhasicImageNumber = m_groups[i].m_ptrToAllSlices[0].m_ptr->m_sequence;
					}

					m_numberOfMultiPhasicGroups++;
					m_currentNumberOfMultiPhasicGroups ++;
					needToCreateNewGroup = true;
					break;
			}
			else
			{
				int idx = i;
				if (m_isMultiPhasic)
				{
					// We need more logic here is the data contains 
					// multi-phasic data and other image type tokens.
					// 
					// -- JULY-09-2003 can't rely on image number
		//			idx = (iPSi.m_ptr->m_imageNumber-m_firstMultiPhasicImageNumber) % m_numberOfMultiPhasicGroups;
					idx = (iPSi.m_ptr->m_sequence-m_firstMultiPhasicImageNumber) % m_currentNumberOfMultiPhasicGroups;
					idx += m_firstMultiPhasicGroupID;
				}

				if (m_groups[idx].AddToGroup (iPSi))
				{
					needToCreateNewGroup = false;
					sliceHasBeenAdded = true;
					break;
				}
				else // Vikram 06/08/2005 1212_3434_5656 Fix
				{
					resetMultiPhasicGroupID_1212_3434_5656Case = true;
					needToCreateNewGroup = true;
					break;

				}
			}
		}
	}

    // Create New group
	if (needToCreateNewGroup || !sliceHasBeenAdded)
	{
		CreateNewGroup (iPSi);

		// Vikram 06/08/2005 1212_3434_5656 Fix
		if (resetMultiPhasicGroupID_1212_3434_5656Case || bIsScanTypeDifferent)
		{
			resetMultiPhasicGroupID_1212_3434_5656Case = 0;
			m_currentFirstGroup = m_firstMultiPhasicGroupID  = m_groups.size () - 1;
			m_currentNumberOfMultiPhasicGroups = 1;
			
			// Vikram & TC Code Review 06/10/2005 Need to keep these two variables updated.
			m_numberOfMultiPhasicGroups++;
			m_firstMultiPhasicImageNumber = m_groups[m_currentFirstGroup].m_ptrToAllSlices[0].m_ptr->m_sequence;
		}
	}
#endif
	return kSuccess;
}


//-----------------------------------------------------------------------------

int AppComCacheReader::AddXAToGroup (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
    // See if we need to create a new group
	for (int i = 0; i < m_groups.size (); i++)
	{
		if (m_groups[i].m_sopInstanceUID.compare (iPSi.m_ptr->m_sopInstanceUID) == 0)
		{
			needToCreateNewGroup = false;
			m_groups[i].AddToGroup (iPSi);
			break;
		}
	}

    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
#endif
	return kSuccess;
}
//-----------------------------------------------------------------------------
// VIkram 07/22/04
int AppComCacheReader::AddNMToGroup (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
    for (int i = 0; i < m_groups.size (); i++)
    {
	    if (m_groups[i].m_sizeX == iPSi.m_ptr->m_sizeX &&
            m_groups[i].m_sizeY == iPSi.m_ptr->m_sizeY &&
            m_groups[i].m_bitsAllocated == iPSi.m_ptr->m_bitsAllocated &&
			m_groups[i].m_bitsStored == iPSi.m_ptr->m_bitsStored &&
			m_groups[i].m_highBit == iPSi.m_ptr->m_highBit &&
			m_groups[i].m_samplesPerPixel == iPSi.m_ptr->m_samplesPerPixel &&
			m_groups[i].m_photometricInterpretation == iPSi.m_ptr->m_photometricInterpretation &&
			m_groups[i].m_planarConfiguration == iPSi.m_ptr->m_planarConfiguration)
		{

		
			iRTVPtrToSliceInformation& ref = m_groups[i].m_ptrToAllSlices[0];


			if ((m_groups[i].m_sopInstanceUID.compare (iPSi.m_ptr->m_sopInstanceUID) == 0) &&
				 (ref.m_ptr->m_detector == iPSi.m_ptr->m_detector)						   &&
				 (ref.m_ptr->m_energyWindow == iPSi.m_ptr->m_energyWindow)				   &&
				 (ref.m_ptr->m_rrInterval == iPSi.m_ptr->m_rrInterval)				   &&
				 (ref.m_ptr->m_phase == iPSi.m_ptr->m_phase)				   &&
				 (ref.m_ptr->m_timeSlot == iPSi.m_ptr->m_timeSlot))

			{
				needToCreateNewGroup = false;
				m_groups[i].AddToGroup (iPSi);
				break;
			}    
		}
	}

    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
#endif
	return kSuccess;
}
//-----------------------------------------------------------------------------
int AppComCacheReader::AddUSToGroup (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
    for (int i = 0; i < m_groups.size (); i++)
    {
	    if (m_groups[i].m_sizeX == iPSi.m_ptr->m_sizeX &&
            m_groups[i].m_sizeY == iPSi.m_ptr->m_sizeY &&
            m_groups[i].m_bitsAllocated == iPSi.m_ptr->m_bitsAllocated &&
			m_groups[i].m_bitsStored == iPSi.m_ptr->m_bitsStored &&
			m_groups[i].m_highBit == iPSi.m_ptr->m_highBit &&
			m_groups[i].m_samplesPerPixel == iPSi.m_ptr->m_samplesPerPixel &&
			m_groups[i].m_photometricInterpretation == iPSi.m_ptr->m_photometricInterpretation &&
			m_groups[i].m_planarConfiguration == iPSi.m_ptr->m_planarConfiguration)
		if (m_groups[i].m_sopInstanceUID.compare (iPSi.m_ptr->m_sopInstanceUID) == 0)
	    {
		    needToCreateNewGroup = false;
		    m_groups[i].AddToGroup (iPSi);
		    break;
	    }    
    }

    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
#endif
	return kSuccess;
}

//-----------------------------------------------------------------------------

int AppComCacheReader::AddRestOfTheModalitiesToGroup (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
   // Uncomment this when Shuji can fix the thin client to handle 
   // US/CR/DR/ etc with one group multiple slices
    // See if we need to create a new group
    for (int i = 0; i < m_groups.size (); i++)
    {
	    if (m_groups[i].m_sizeX == iPSi.m_ptr->m_sizeX &&
            m_groups[i].m_sizeY == iPSi.m_ptr->m_sizeY &&
            m_groups[i].m_bitsAllocated == iPSi.m_ptr->m_bitsAllocated &&
			m_groups[i].m_bitsStored == iPSi.m_ptr->m_bitsStored &&
			m_groups[i].m_highBit == iPSi.m_ptr->m_highBit)
	    {
		    needToCreateNewGroup = false;
		    m_groups[i].AddToGroup (iPSi);
		    break;
	    }    
    }
#endif 
    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
	return kSuccess;
}

//-----------------------------------------------------------------------------

int AppComCacheReader::AddSCToGroup (iRTVPtrToSliceInformation& iPSi)
{
    bool needToCreateNewGroup = true;

#if 0
	// -- 2004.12.14
	// The client can't handle multiframe CRs, always use separate groups
	if (iPSi.m_ptr->m_modality != "CR" && iPSi.m_ptr->m_modality != "DR")
	{
		// See if we need to create a new group
		for (int i = 0; i < m_groups.size (); i++)
		{
			if (m_groups[i].m_samplesPerPixel == iPSi.m_ptr->m_samplesPerPixel &&
				m_groups[i].m_photometricInterpretation == iPSi.m_ptr->m_photometricInterpretation &&
				m_groups[i].m_planarConfiguration == iPSi.m_ptr->m_planarConfiguration &&
				// -- AUG-7-2003 need to check the size
				m_groups[i].m_sizeX == iPSi.m_ptr->m_sizeX &&
				m_groups[i].m_sizeY == iPSi.m_ptr->m_sizeY)
			{
				needToCreateNewGroup = false;
				m_groups[i].AddToGroup (iPSi);
				break;
			}    
		}
	}

    // Create New group
	if (needToCreateNewGroup)
	{
		CreateNewGroup (iPSi);
	}
#endif
	return kSuccess;
}

//-----------------------------------------------------------------------------
// Creates a new group
//
int AppComCacheReader::CreateNewGroup (iRTVPtrToSliceInformation& iPSi)
{
	iRTVDICOMGroup group;

	group.AddFirstToGroup (iPSi, m_groups.size(), m_currentSortType);

	// Murali 2007.01.03
	group.SetSortCTByScanOptionsFirst(IsScanOptionsUsedForSorting());

	m_groups.push_back(group);

	return kSuccess;
}

//-----------------------------------------------------------------------------
// Creates a new group
//
int AppComCacheReader::ProcessGroups ()
{
	int i;
	if (m_groups.size () > 0)
	{
		for ( i = 0; i < m_groups.size (); i++)
		{
			m_groups[i].SortAndProcess ();
		}
		
		if ((m_groups[0].GetModality ().compare (kModalityXAStr) == 0) ||
			(m_groups[0].GetModality ().compare (kModalityXFStr) == 0) ||
			(m_groups[0].GetModality ().compare (kModalityRFStr) == 0))
		{
			// If we have more than one group the we need to set ref groupid
			// if the data is a biplane data set
			if (m_groups.size () > 1)
			{
				for (i = 0; i < m_groups.size(); i++)
				{
					std::string str = m_groups[i].GetReferencedSOPInstanceUID ();
					int id          = m_groups[i].GetGroupID ();
					
					for (int j = 0; j < m_groups.size(); j++)
					{
						if (j != i)
						{
							if (m_groups[j].GetSOPInstanceUID().compare (str) == 0)
							{
								m_groups[i].m_referencedGroupID = j;
							}
						}
					}
				}
			}
		}

		// Murali 2006.03.20 Any additional validation on the sorting would be done here.
		if(m_groups.size() > 1)
		{
			ValidateAndFixSortedGroups();
		}

		// added by shiying hu, to update model matrix and min, max value
		// for each group
		for (i = 0; i < m_groups.size(); i++)
		{
			m_groups[i].UpdateGroupInfo();
		}
	
		return kSuccess;
	}
	else
		return kErrNoGroupsPresent;
}

//-----------------------------------------------------------------------------
// Changed the Find function
//
iRTVSOPLocation  AppComCacheReader::Find(const char* iSOPUID, int iFrameNumber)
{
	static iRTVSOPLocation BadLocation(-1,-1, -1);


	// 06/05/03 If you want any sop then return the first one. 
	// 24-bit preview fix
	if (!iSOPUID || !*iSOPUID ||  (strcasecmp(iSOPUID,"Any") == 0) || (strcasecmp(iSOPUID,"AnySOP") == 0))
	{
			return m_sortedSOPToSliceLocation.begin ()->second;
	}

	// -- (07/12/2002)
	if (IsModalityCT(m_modality) || IsModalityMR(m_modality) || IsModalityPT(m_modality))
		iFrameNumber = 0;
    

	std::string lSOPInstaceUID = iSOPUID;
	std::map<std::string, iRTVSOPLocation>::iterator p;

#if 0

	if ((p = m_sortedSOPToSliceLocation.find(lSOPInstaceUID)) != m_sortedSOPToSliceLocation.end())
		return p->second;
#endif

	TransformMultiFrameSOPInstanceUID(lSOPInstaceUID,iFrameNumber);
	
    if ((p = m_sortedSOPToSliceLocation.find(lSOPInstaceUID)) != m_sortedSOPToSliceLocation.end())
		return p->second;
	
#if defined(_DEBUG) && 1
	fprintf(stderr,"SOP not found: %s from the following %d SOPS\n", iSOPUID, m_sortedSOPToSliceLocation.size());
	for (p = m_sortedSOPToSliceLocation.begin(); p != m_sortedSOPToSliceLocation.end(); p++)
		fprintf(stderr,"SOP=%s\n",p->first.c_str());
#endif
	 
    return  BadLocation;
}

//-----------------------------------------------------------------------------
// Creates a new group
//
int  AppComCacheReader::OpenDataFile (int iGroupID, int iBegin, int iNumSlices)
{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		m_groups[iGroupID].OpenDataFile (m_cacheDataFilename, iBegin, iNumSlices);
		m_groups[iGroupID].OpenCompressedDataFile (m_compressedL0CacheDataFilename);
	}
	else
		return kErrCannotOpenFile;

	return kSuccess;
}

//-----------------------------------------------------------------------------

int  AppComCacheReader::CloseDataFile (int iGroupID)
{
	// force the top level directory access time updated
	// by open the bottom subdirectory, open file won't update top directory
	// if in the future the access time on cache files is saved in database,
	// we can remove the code.
	{
		WIN32_FIND_DATA fd;
		HANDLE handle;

		handle = FindFirstFile(m_cacheDirectory.c_str(), &fd);
		if (handle != INVALID_HANDLE_VALUE)
		{
			FindClose(handle);
		}
	}

	
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		m_groups[iGroupID].CloseDataFile ();
		m_groups[iGroupID].CloseCompressedDataFile ();
	}
	else
		return kErrCannotCloseFile;

	return kSuccess;
}

//-----------------------------------------------------------------------------

iRTVSliceBuffer*  AppComCacheReader::LoadSlice (int iGroupID, int iSliceNumber)

{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		int bufferNumber = -1;
		for (int i = 0; i < kNumberOfIRTVSliceBuffers; i++)
		{
			if (!m_sliceBuffers[i].IsUsed ())
			{
				bufferNumber = i;
				break;
			}
		}
		
		if (bufferNumber != -1)
		{
			if (m_groups[iGroupID].LoadSlice (m_sliceBuffers[bufferNumber], iSliceNumber) == kSuccess)
			{
				m_sliceBuffers[bufferNumber].SetIsUsed ();
				return &m_sliceBuffers[bufferNumber];
			}
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}

//-----------------------------------------------------------------------------
iRTVSliceBuffer*  AppComCacheReader::ReadSlice (int iGroupID, int iSliceNumber)

{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		int bufferNumber = -1;
		for (int i = 0; i < kNumberOfIRTVSliceBuffers; i++)
		{
			if (!m_sliceBuffers[i].IsUsed ())
			{
				bufferNumber = i;
				break;
			}
		}
		
		if (bufferNumber != -1)
		{
			if (m_groups[iGroupID].ReadSlice (m_sliceBuffers[bufferNumber], iSliceNumber) == kSuccess)
			{
				m_sliceBuffers[bufferNumber].SetIsUsed ();
				return &m_sliceBuffers[bufferNumber];
			}
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}
//-----------------------------------------------------------------------------
// Vikram 02/20/03 needed to do this to fix problems with slices of different sizes in
// the same series.
int AppComCacheReader::GetGroupIDForSOPLocation (iRTVSOPLocation& iSOPLocation,
													int& oSequenceInGroup) // -- JULY-2003
{
	
#if 0
		int nGrps = m_groups.size ();
		int groupIdToUse = -1;

        std::vector <iRTVPtrToSliceInformation> si;
		std::string sop = m_allSlices[iSOPLocation.m_indexInVectorOfSlices].m_ptr->m_sopInstanceUID;

		for (int g = 0; g < nGrps; g++)
		{
			si = m_groups[g].GetPtrToAllSlices ();

			for (int h = 0; h < si.size(); h++)
			{
				if (strcmp (sop.c_str (), si[h].m_ptr->m_sopInstanceUID.c_str()) == 0)
				{
					oSequenceInGroup = h;
					groupIdToUse = g;
					break;
				}
			}

			if (groupIdToUse >= 0)
				break;
		}

 
		return groupIdToUse;
#else
	return -1;
#endif
}

//-----------------------------------------------------------------------------
int AppComCacheReader::GetGroupIDForSOPUID (const char* iSOPUID)
{
	
#if 0
	std::vector <iRTVPtrToSliceInformation> si;
	std::string sop(iSOPUID);
	int nGrps = m_groups.size ();
	int groupIdToUse = -1;
	
	for (int g = 0; g < nGrps &&  groupIdToUse < 0; g++)
	{
		si = m_groups[g].GetPtrToAllSlices ();
		
		for (int h = 0; h < si.size() && groupIdToUse < 0; h++)
		{
			if (strcmp (sop.c_str (), si[h].m_ptr->m_sopInstanceUID.c_str()) == 0)
			{
				groupIdToUse = g;
			}
		}
	}
	
	return groupIdToUse;
#else
	return -1;
#endif
}

//-----------------------------------------------------------------------------
// Added on 08/23/02 to fix loading of preview

iRTVSliceBuffer*  AppComCacheReader::LoadSlice (iRTVSOPLocation& iSOPLocation, int iGroupID)

{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		int bufferNumber = -1;
		for (int i = 0; i < kNumberOfIRTVSliceBuffers; i++)
		{
			if (!m_sliceBuffers[i].IsUsed ())
			{
				bufferNumber = i;
				break;
			}
		}
		

		if (bufferNumber != -1)
		{
			if (m_groups[iGroupID].LoadSlice (m_sliceBuffers[bufferNumber], iSOPLocation) == kSuccess)
			{
				m_sliceBuffers[bufferNumber].SetIsUsed ();
				return &m_sliceBuffers[bufferNumber];
			}
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}
//-----------------------------------------------------------------------------
// Loads the Next best slice
iRTVSliceBuffer*  AppComCacheReader::LoadNextSlice (int iGroupID)

{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		int bufferNumber = -1;
		for (int i = 0; i < kNumberOfIRTVSliceBuffers; i++)
		{
			if (!m_sliceBuffers[i].IsUsed ())
			{
				bufferNumber = i;
				break;
			}
		}
		//printf("iGroupID=%d, bufferNumber=%d\n", iGroupID, bufferNumber);
		if (bufferNumber != -1)
		{
			if (m_groups[iGroupID].LoadNextSlice (m_sliceBuffers[bufferNumber]) == kSuccess)
			{
				m_sliceBuffers[bufferNumber].SetIsUsed ();
				return &m_sliceBuffers[bufferNumber];
			}
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Loads the Next best slice
iRTVSliceBuffer*  AppComCacheReader::LoadNextCompressedSlice (int iGroupID, float iCompressionRatio)

{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		int bufferNumber = -1;
		for (int i = 0; i < kNumberOfIRTVSliceBuffers; i++)
		{
			if (!m_compressedSliceBuffers[i].IsUsed ())
			{
				bufferNumber = i;
				break;
			}
		}
		//printf("iGroupID=%d, bufferNumber=%d\n", iGroupID, bufferNumber);
		if (bufferNumber != -1)
		{
			if (m_groups[iGroupID].LoadNextCompressedSlice (m_compressedSliceBuffers[bufferNumber], iCompressionRatio) == kSuccess)
			{
				m_compressedSliceBuffers[bufferNumber].SetIsUsed ();
				return &m_compressedSliceBuffers[bufferNumber];
			}
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}
//-----------------------------------------------------------------------------
int AppComCacheReader::GetXSize (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetXSize ();
}

//-----------------------------------------------------------------------------

int AppComCacheReader::GetYSize (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetYSize ();
}

//-----------------------------------------------------------------------------

int AppComCacheReader::GetZSize (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetZSize ();
}

//-----------------------------------------------------------------------------
int AppComCacheReader::GetBitsUsedPerPixel (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetBitsUsedPerPixel ();
}

//-----------------------------------------------------------------------------
int AppComCacheReader::GetBitsAllocated (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetBitsAllocated ();
}
//-----------------------------------------------------------------------------

int AppComCacheReader::GetBytesPerPixel (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetBytesPerPixel ();
}
//-----------------------------------------------------------------------------
bool AppComCacheReader::IsUniformlySpaced (int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].IsUniformlySpaced ();
}
//-----------------------------------------------------------------------------
std::string& AppComCacheReader::GetPlaneType(int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetPlaneType ();
}
//-----------------------------------------------------------------------------

std::string& AppComCacheReader::GetSOPInstanceUID(int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetSOPInstanceUID ();
}

//-----------------------------------------------------------------------------
std::string& AppComCacheReader::GetReferencedSOPInstanceUID(int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetReferencedSOPInstanceUID ();
}

//-----------------------------------------------------------------------------
int AppComCacheReader::GetReferencedGroupID(int iGroupID)
{
	assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetReferencedGroupID ();
}

//-----------------------------------------------------------------------------
double AppComCacheReader::GetSlope(int iGroupID)
{
	return (iGroupID >=0 && iGroupID < m_groups.size()) ? m_groups[iGroupID].GetSlope():1.0;
}

// t.c. zhao 05/01/2003
int AppComCacheReader::GetSortedSliceInformation(int iGroupID, tLoadedSlice& oSliceInfo)
{
	return (iGroupID >=0 && iGroupID < m_groups.size()) ? 
		m_groups[iGroupID].GetSortedSliceInformation(oSliceInfo):kErrFileStateIsNotGood;
}

//-----------------------------------------------------------------------------
// Right now this is very simple. This function returns the largest group
int AppComCacheReader::GetIndexOfBestGroupToLoad ()
{
    int idx = 0;
    int numberOfSlices = 0;
    for (int i = 0; i < m_groups.size(); i++)
    {
         if (m_groups[i].GetZSize () > numberOfSlices)
         {
             idx = i;
         }
    }

    return idx;
}

//-----------------------------------------------------------------------------
// Get samples per pixel
int AppComCacheReader::GetSamplesPerPixel (int iGroupID)
{
    assert (iGroupID >=0 && iGroupID < m_groups.size());

	return m_groups[iGroupID].GetSamplesPerPixel ();
}

//-----------------------------------------------------------------------------
// Get Photometric Interpretation
int AppComCacheReader::GetPhotometricInterpretation (int iGroupID)
{
    assert (iGroupID >=0 && iGroupID < m_groups.size());
	return m_groups[iGroupID].GetPhotometricInterpretation ();
}

//-----------------------------------------------------------------------------
// Get Planar Configuration
int AppComCacheReader::GetPlanarConfiguration (int iGroupID)
{
    assert (iGroupID >=0 && iGroupID < m_groups.size());
	return m_groups[iGroupID].GetPlanarConfiguration ();
}

//-----------------------------------------------------------------------------
int AppComCacheReader::AddSlice (iRTVPtrToSliceInformation& iPtr)
{
    
#if 0
    if (m_modality.compare("") == 0)
    {
        // Initialize if modality not intialized
        m_modality    = iPtr.m_ptr->m_modality;
        m_sopClassUID = iPtr.m_ptr->m_sopClassUID;
    }

    m_uniqueSOPInstanceUIDS[iPtr.m_ptr->m_sopInstanceUID] = 1;

    // Vikram 07/05/02 Sub-Series support
    // At this point all we need to do is add it 
    // the list of all slices
    m_allSlices.push_back (iPtr);

#endif
    return kSuccess;
}

//-----------------------------------------------------------------------------

int AppComCacheReader::PushSortStack()
{
    m_sortTypeOnStack = m_currentSortType;
    m_phasesOnStack   = m_currentPhases;

    m_currentSortType = kSortUnknown;
    m_currentPhases   = -1;
    m_groups.clear();

    return kSuccess;
}

//-----------------------------------------------------------------------------
// Restores the last sort type 

int AppComCacheReader::PopSortStack()
{
    int status = kSuccess;
  
    if (m_sortTypeOnStack == kSortByImagePosition)
    {
        ProcessByImagePosition ();
    }
    else
    if (m_sortTypeOnStack == kSortByImageNumber)
    {
        ProcessByImageNumber(m_phasesOnStack);
    }
	else
    if (m_sortTypeOnStack == kSortAs2DSlices)
    {
        ProcessAs2DSlices();
    }
    else
    {
        status = kErrSortType;
    }


    return status;
}

//-----------------------------------------------------------------------------
// Reads the Compressed Cache Description
#if !defined (USE_STDIO)
int AppComCacheReader::ReadCompressedCacheDescription ()
{ 
	int status = kSuccess;

	// Name of the Cache File
	std::string lFilename = m_cacheDirectory;

	lFilename += "/";
	lFilename += kJ2kCacheL0DescriptionFileName; // Terarecon Cache description 

	std::string lCacheDataFile = m_cacheDirectory;
	lCacheDataFile += "/";
	lCacheDataFile += kJ2kCacheL0DataFileName; // Terarecon Cache description 

	m_L0CompressedCacheRead    = false;
	m_L0CompressedCachePresent = false;

	// First Check to see if the files  exists and 
	if (::access(lFilename.c_str(), 0) == 0)
	{
		
		// Also make sure that the data file is present
		if (::access(lCacheDataFile.c_str(), 0) == 0)
		{
			m_L0CompressedCachePresent = true;
		}
		else
		{
			return kErrCannotOpenFile;
		}

#if !defined (USE_STDIO)
		ifstream l0DescriptionFile;
		l0DescriptionFile.open (lFilename.c_str(), ios::binary | ios::in);

		if (!l0DescriptionFile.is_open ())
		{
			return kErrCannotOpenFile;
		}
#else
		FILE *l0DescriptionFilePtr = 0;
		l0DescriptionFilePtr = fopen (lFilename.c_str(), "rb");

		if (!l0DescriptionFilePtr)
		{
			return kErrCannotOpenFile;
		}
#endif



		int  errorCode = kSuccess;
		std::string str;
		std::string sopUIDStr;
		
		bool LookingHeader = true;
		int idx = 0;
		char* delim;
	
		int currentSlice = 0;

#if !defined (USE_STDIO)
		while(GetNextValidLine (str, l0DescriptionFile) && errorCode == kSuccess)
#else
		while(GetNextValidLine (str, l0DescriptionFilePtr) && errorCode == kSuccess)
#endif
		{
			std::string::size_type iS = str.find_first_not_of ("<");
			if (iS == std::string::npos) // empty line skip it
				continue;
			
			delim = "> ";
			std::string::size_type iE = str.find_first_of (delim, iS);
			std::string token =  str.substr(iS, iE-iS);
			
#if !defined (USE_HASH)
		idx = m_cacheDataTokens[token];
#else
		idx = m_tokenToIdx[AQNetHashString(token)];
#endif
		
			if(LookingHeader && idx != kSliceDescriptionIdx)
			{
				status = errorCode = kErrFileStateIsNotGood;
				break;
			}

			switch (idx)
			{
				// Cache Description
				case kSliceDescriptionIdx:
					LookingHeader = false;
				break; 

			    // SOP Instance UID
				case kSOPInstanceUIDIdx:
					sopUIDStr = GetValue (str);
				break; 


			    // Start Of Data In Data File
				case kStartOfDataInDataFileIdx:
					m_allSlices[currentSlice].m_ptr->m_startOfL0CompressedDataInDataFile = atoi (GetValue (str).c_str());
				break; 
					
				// Size Of Data In Data File
				case kSizeOfDataInBytesIdx:
					m_allSlices[currentSlice].m_ptr->m_sizeOfL0CompressedData = atoi (GetValue (str).c_str());
				break; 

				// Min Voxel Value for the entire series
				case kGlobalMinVoxelValueIdx:
					m_allSlices[currentSlice].m_ptr->m_globalMinVoxelValue = atoi (GetValue (str).c_str());
				break; 

				// Max Voxel Value for the entire series
				case kGlobalMaxVoxelValueIdx:
					m_allSlices[currentSlice].m_ptr->m_globalMaxVoxelValue = atoi (GetValue (str).c_str());
				break; 

				// Compression factor
				case kSliceCompressionFactorIdx:
					m_allSlices[currentSlice].m_ptr->m_compressionFactor = atof (GetValue (str).c_str());
				break; 

				// X-Size of the input slice
				case kInputCompressionSizeXIdx:
					m_allSlices[currentSlice].m_ptr->m_inputL0CompressionSizeX = atoi (GetValue (str).c_str());
				break; 
				
				// Y-Size of the input slice
				case kInputCompressionSizeYIdx:
					m_allSlices[currentSlice].m_ptr->m_inputL0CompressionSizeY = atoi (GetValue (str).c_str());
				break; 

				// -- JULY-22-2003 we need a different variable to
				// keep track of the VOI lut info for compressed slices as
				// compressed slices are always post-
				case kVOIWindowWidthIdx:
					m_allSlices[currentSlice].m_ptr->m_sVOIWindowWidth = atof (GetValue (str).c_str());
					break;
					
				case kVOIWindowCenterIdx:
					m_allSlices[currentSlice].m_ptr->m_sVOIWindowCenter = atof (GetValue (str).c_str());
					break;

				// End of slice description so put in the list
				case kEndOfSliceDescriptionIdx:
					currentSlice ++; // increment to the next slice
					LookingHeader = true;
				break;
					
				default:
					// garbage indescription file abort reading
					status = errorCode = kErrFileStateIsNotGood;
				break;
			}

		}

#if !defined (USE_STDIO)
		l0DescriptionFile.close ();
		l0DescriptionFile.clear ();
#else
		if (l0DescriptionFilePtr)
		{
			fclose (l0DescriptionFilePtr);
		}
#endif
		m_L0CompressedCacheRead = true;
	}

	return status;
}
#else
//-----------------------------------------------------------------------------
int AppComCacheReader::ReadCompressedCacheDescription ()
{ 
	int status = kSuccess;

#if 0
	// Name of the Cache File
	std::string lFilename = m_cacheDirectory;

//	lFilename += "/";
	lFilename += kJ2kCacheL0DescriptionFileName; // Terarecon Cache description 

	std::string lCacheDataFile = m_cacheDirectory;
	lCacheDataFile += "/";
	lCacheDataFile += kJ2kCacheL0DataFileName; // Terarecon Cache description 

	m_L0CompressedCacheRead    = false;
	m_L0CompressedCachePresent = false;

	// First Check to see if the files  exists and 
	if (::access(lFilename.c_str(), 0) == 0)
	{
		
		// Also make sure that the data file is present
		if (::access(lCacheDataFile.c_str(), 0) == 0)
		{
			m_L0CompressedCachePresent = true;
		}
		else
		{
			return kErrCannotOpenFile;
		}


		FILE *l0DescriptionFilePtr = 0;
		l0DescriptionFilePtr = fopen (lFilename.c_str(), "rb");

		if (!l0DescriptionFilePtr)
		{
			return kErrCannotOpenFile;
		}


		int  errorCode = kSuccess;
		std::string sopUIDStr;
		
		bool LookingHeader = true;
		int idx = 0;

	
		int currentSlice = 0;

		while(GetNextValidLine (m_nextLine, l0DescriptionFilePtr) && errorCode == kSuccess)
		{
			if (m_nextLine[0] == '<')
			{
				if (m_nextLine[1] != '/')
				{
					sprintf (m_token[0], "%s", kSliceDescriptionStr);
					sscanf (m_nextLine, "%*s %s>", m_token[1]);
				}
				else
				{
					sprintf (m_token[0], "%s", kEndOfSliceDescriptionStr);
				}
			}
			else
			{
				sscanf (m_nextLine, "%s = %s %s %s %s %s %s", m_token[0], m_token[1], m_token[2],
															  m_token[3], m_token[4], m_token[5], m_token[6]);
			}
			
#if !defined (USE_HASH)
			idx = m_cacheDataTokens[std::string(m_token[0])];
#else
		idx = m_tokenToIdx[AQNetHashString(m_token[0])];
#endif
		
			if(LookingHeader && idx != kSliceDescriptionIdx)
			{
				status = errorCode = kErrFileStateIsNotGood;
				break;
			}

			switch (idx)
			{
				// Cache Description
				case kSliceDescriptionIdx:
					LookingHeader = false;
				break; 

			    // SOP Instance UID
				case kSOPInstanceUIDIdx:
					sopUIDStr = m_token[1];
				break; 


			    // Start Of Data In Data File
				case kStartOfDataInDataFileIdx:
					m_allSlices[currentSlice].m_ptr->m_startOfL0CompressedDataInDataFile = atoi (m_token[1]);
				break; 
					
				// Size Of Data In Data File
				case kSizeOfDataInBytesIdx:
					m_allSlices[currentSlice].m_ptr->m_sizeOfL0CompressedData = atoi (m_token[1]);
				break; 

				// Min Voxel Value for the entire series
				case kGlobalMinVoxelValueIdx:
					m_allSlices[currentSlice].m_ptr->m_globalMinVoxelValue = atoi (m_token[1]);
				break; 

				// Max Voxel Value for the entire series
				case kGlobalMaxVoxelValueIdx:
					m_allSlices[currentSlice].m_ptr->m_globalMaxVoxelValue = atoi (m_token[1]);
				break; 

				// Compression factor
				case kSliceCompressionFactorIdx:
					m_allSlices[currentSlice].m_ptr->m_compressionFactor = atof (m_token[1]);
				break; 

				// X-Size of the input slice
				case kInputCompressionSizeXIdx:
					m_allSlices[currentSlice].m_ptr->m_inputL0CompressionSizeX = atoi (m_token[1]);
				break; 
				
				// Y-Size of the input slice
				case kInputCompressionSizeYIdx:
					m_allSlices[currentSlice].m_ptr->m_inputL0CompressionSizeY = atoi (m_token[1]);
				break; 


				// -- JULY-22-2003 we need a different variable to
				// keep track of the VOI lut info for compressed slices as
				// compressed slices are always post-
				case kVOIWindowWidthIdx:
					m_allSlices[currentSlice].m_ptr->m_sVOIWindowWidth = atof (m_token[1]);
					break;
					
				case kVOIWindowCenterIdx:
					m_allSlices[currentSlice].m_ptr->m_sVOIWindowCenter = atof (m_token[1]);
					break;

				// End of slice description so put in the list
				case kEndOfSliceDescriptionIdx:
					currentSlice ++; // increment to the next slice
					LookingHeader = true;
				break;
					
				default:
					// garbage indescription file abort reading
					status = errorCode = kErrFileStateIsNotGood;
				break;
			}

		}

		if (l0DescriptionFilePtr)
		{
			fclose (l0DescriptionFilePtr);
		}

		m_L0CompressedCacheRead = true;
	}

#endif

	return status;
}
#endif

// -- 2004.04.05
// We need this to make subseries loading working correctly
void AppComCacheReader::PruneSOPs(std::map<std::string,int>& iSOPs) 
{
#if 0
	std::vector <iRTVPtrToSliceInformation>::iterator p;
	std::map<std::string, int>::iterator up;
	std::string thisSOP;
	int i, oSize = m_allSlices.size();
	
	m_uniqueSOPInstanceUIDS.clear();
	m_sortedSOPToSliceLocation.clear();
	
	for (i = 0; i < m_groups.size (); i++)
	{
		m_groups[i].Clear();
	}
	m_groups.clear();
	
	m_firstMultiPhasicGroupID  = -1;
	m_numberOfMultiPhasicGroups = 1;
	m_currentNumberOfMultiPhasicGroups = 1;
	m_firstMultiPhasicImageNumber = -999;
	
    m_currentSortType    = kSortUnknown;
    m_sortTypeOnStack    = kSortUnknown;
	
	for (p = m_allSlices.begin();  p != m_allSlices.end();)
	{
		thisSOP = (*p).m_ptr->m_sopInstanceUID;
		if (iSOPs.find(thisSOP) == iSOPs.end())
		{
			try
			{
				
				(*p).DeleteData();
			}
			catch (...)
			{
				assert(0);
			}

			p = m_allSlices.erase(p);
			
			
		}
		else
		{
			m_uniqueSOPInstanceUIDS[thisSOP] = 1;
			++p;
		}
	}
	
	ProcessAllSlices ();
	ProcessByImagePosition ();

	if (oSize != m_allSlices.size())
	{
		m_isPruned = 1;
		SetForceReadDescription();
	}
#endif
}

//-----------------------------------------------------------------------------
// -- 2004.04.13
void AppComCacheReader::SetForceReadDescription(void)
{
	m_descriptionTime = 1;
}


//-----------------------------------------------------------------------------
//-- 2004.11.10
// Find sliceInfo for a particular SOP
const iRTVPtrToSliceInformation* AppComCacheReader::GetSliceInfo(const char* iSOPUID, int iFrame)
{
	iRTVSOPLocation location = Find(iSOPUID, iFrame);
	
	if (location.Bad())
	{
		return 0;
	}

	return &this->m_allSlices[location.m_indexInVectorOfSlices];
}

//-----------------------------------------------------------------------------
// -- & Shiying Hu 2005.10.11
// Load the raw slice - except for CT where the data is in HU
iRTVSliceBuffer*  AppComCacheReader::ReadRawSlice (int iGroupID, int iSliceNumber)

{
	if (iGroupID >= 0 && iGroupID < m_groups.size () && m_isCacheDescriptionRead)
	{
		int bufferNumber = -1;
		for (int i = 0; i < kNumberOfIRTVSliceBuffers; i++)
		{
			if (!m_sliceBuffers[i].IsUsed ())
			{
				bufferNumber = i;
				break;
			}
		}
		
		if (bufferNumber != -1)
		{
			if (m_groups[iGroupID].ReadRawSlice (m_sliceBuffers[bufferNumber], iSliceNumber) == kSuccess)
			{
				m_sliceBuffers[bufferNumber].SetIsUsed ();
				return &m_sliceBuffers[bufferNumber];
			}
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}



//--------------------------------------------------------------------------------
// -- 2005.11.16
// Need this to sort mixed series right (axial&sagittal scan in one series)
void AppComCacheReader::SetSortMRByScanTypeFirst(int iYesNo)
{
	m_sortMRByScanTypeFirst = iYesNo;
}

//--------------------------------------------------------------------------------
// Vikram 07/17/2006 Fix for data with no corelation with ImageNumber and ImagePosition
void AppComCacheReader::SetPresortCTByImagePositionAfterImageNumber(int iYesNo)
{
	m_presortCTByImagePositionAfterImageNumber = iYesNo;
}


//---------------------------------------------------------------------------------
// Murali 2007.01.03 Fix for sorting CT with scanOptions specified
void AppComCacheReader::SetSortCTByScanOptionsFirst(int iYesNo)
{
	m_sortCTByScanOptionsFirst	=	iYesNo;
}

//-----------------------------------------------------------------------------
// -- 2005.11.17
// Consolidate common functions to support ScanType sort
//-----------------------------------------------------------------------------
void AppComCacheReader::GenerateSortedIndex(void)
{
#if 0
	int i = 0, frameNumber = 0;
    __int64 start, size;
    std::string previousSOPInstanceUID, sopInstanceUID;
    int numberOfSlices = m_allSlices.size();
	
	m_sortedSOPToSliceLocation.clear();
	
	for (i = 0; i < numberOfSlices; i++)
    {
		m_allSlices[i].m_ptr->MakeImageTypeTokensIntoOneString ();
		
		m_allSlices[i].m_ptr->m_sequence = i; // -- 2003-07-09
#ifdef _DEBUG
		int sn = m_allSlices[i].m_ptr->m_imageNumber;
#endif
		
        start = m_allSlices[i].m_ptr->m_startOfDataInDataFile;
        size  = m_allSlices[i].m_ptr->m_sizeOfData;
        
        // We only increment the frame number if the previous
        // SOPInstanceUID is the same. Remember that XA Multi-Frame is 
        // sequential - Vikram 07/08/02
        sopInstanceUID = m_allSlices[i].m_ptr->m_sopInstanceUID;
		
        if (sopInstanceUID.compare (previousSOPInstanceUID) == 0)
        {
            frameNumber ++;
        }
        else
		{
            frameNumber = 0;
		}
        
        // Copy the raw SOPInstanceUID
        previousSOPInstanceUID = sopInstanceUID;
		
		//	char buffer[64];
		//	sprintf (buffer, "_%06d",frameNumber);
		//	sopInstanceUID += buffer;
		
		TransformMultiFrameSOPInstanceUID(sopInstanceUID,frameNumber);
		
		/* -- 07/31/2002
		* indexInVectorOfSlices is used as a direct index, can't use i+1.
		* changed to i
		*/
        m_sortedSOPToSliceLocation[sopInstanceUID] = iRTVSOPLocation(i, start, size);
    }   
#endif
}

//---------------------------------------------------------------------------------------------
bool AppComCacheReader::IsScanOptionsUsedForSorting(void)
{
	bool bScanOptionsUsedForSorting = false;
	
#if 0
	if(m_allSlices.size() <=0 ) 
			return bScanOptionsUsedForSorting;

	if(AppComCacheReader::m_sortCTByScanOptionsFirst < 1 || 
			AppComCacheReader::m_sortCTByScanOptionsFirst > 2)
	{
		return bScanOptionsUsedForSorting;
	}

	if(strncmp(m_modality.c_str(), "CT", sizeof("CT")) != 0)
			return bScanOptionsUsedForSorting;

	bool bManufacturerIsSiemens = false;
	const char* strSiemens = "SIEMENS";		//reference to compare

	const char* manuStr = m_allSlices[0].m_ptr->m_manufacturer.c_str();	
	if(strlen(manuStr) && strnicmp(manuStr, strSiemens, strlen(strSiemens)) == 0)
	{
		bManufacturerIsSiemens = true;
	}

	if(!bManufacturerIsSiemens && AppComCacheReader::m_sortCTByScanOptionsFirst == 1)
	{
		return bScanOptionsUsedForSorting;
	}

	const char* scanOptions = m_allSlices[0].m_ptr->GetScanOptions();
	
	if(!scanOptions || !*scanOptions)
			return bScanOptionsUsedForSorting;
	
#endif
	return true;	
}

//---------------------------------------------------------------------------------
void AppComCacheReader::ValidateAndFixSortedGroups(void)
{
	if(m_groups.size() <= 0) return; 

#if 0
	if(IsScanOptionsUsedForSorting() &&  m_currentSortType == kSortByImagePosition)
	{
		const char* strSiemens = "SIEMENS";		//reference to compare

		const char* manuStr = m_allSlices[0].m_ptr->GetManufacturer();
		if(strlen(manuStr) && strnicmp(manuStr, strSiemens, strlen(strSiemens)) == 0)
		{
			std::stable_sort (m_groups.begin (), m_groups.end (), iRTVDICOMGroup::SortByPercentRR);		
		}	
	}
#endif
	//other validations could follow here.
}	
 