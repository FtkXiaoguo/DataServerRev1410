/***********************************************************************
 * TICache.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This file contains the strings shared by the Cache reader and
 *		Cache writer(s)
 *
 *
 *   
 *-------------------------------------------------------------------
 */

#if !defined(TICACHE_H)
#include "TICache.h"
#endif
//-----------------------------------------------------------------------------
const char* kCacheDescriptionFileName = "Cache.description";
const char* kCacheDataFileName = "Cache.data";

const char* kJ2kCacheL0DescriptionFileName    = "J2kCacheL0.description";
const char* kJ2kCacheL0DataFileName           = "J2kCacheL0.data";
const char* kJ2kCacheL1DescriptionFileName    = "J2kCacheL1.description";
const char* kJ2kCacheL1DataFileName           = "J2kCacheL1.data";

const unsigned _int64	kCacheDescriptionFileSize	= 40*1024*1024; // 40 MB
const unsigned _int64	kCacheDataFileSize			= 2000u*1024*1024; // 2GB 
const unsigned _int64	kCacheDataFileSizeBig		= kCacheDataFileSize*3;
//const unsigned _int64	kMaxDataSize 	= 1800*1024*1024;	// 1.8GB

const unsigned int	kSeriesReserveSpace = 1620;  // maxdata+description

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Keywords

const char* kSliceDescriptionStr				= "SliceDescription";
const char* kEndOfSliceDescriptionStr			= "/SliceDescription";
const char* kModalityStr						= "Modality";
const char* kSOPInstanceUIDStr					= "SOPInstanceUID";
const char* kSOPClassUIDStr					    = "SOPClassUID";
const char* kBitsStoredStr						= "BitsStored";
const char* kBitsAllocatedStr					= "BitsAllocated";
const char* kHighBitStr							= "HighBit";
const char* kSizeXStr							= "SizeX";
const char* kSizeYStr							= "SizeY";
const char* kStartOfDataInDataFileStr			= "StartOfDataInDataFile";
const char* kSizeOfDataInBytesStr				= "SizeOfDataInBytes";
const char* kImageTypeTokenStr					= "ImageTypeToken";

const char* kImagePositionStr					= "ImagePosition";
const char* kImageOrientationStr				= "ImageOrientation";
const char* kPixelSpacingStr					= "PixelSpacing";
const char* kNumberOfEntriesInHistogramStr		= "NumberOfEntriesInHistogram";
const char* kHistogramStr						= "Histogram";


const char* kRescaleSlopeStr					= "RescaleSlope";
const char* kRescaleInterceptStr				= "RescaleIntercept";

const char* kReferencedSOPInstanceUIDStr		= "ReferencedSOPInstanceUID";



// CT
const char* kModalityCTStr = "CT";
const char* kImageNumberStr = "ImageNumber"; 

// MR
const char* kModalityMRStr = "MR";

// XA
const char* kModalityXAStr = "XA";
const char* kModalityXFStr = "XF";
const char* kModalityRFStr = "RF";

// CR/DR
const char* kModalityCRStr = "CR";
const char* kModalityDRStr = "DR";

// SC Images
const char* kModalityOTStr = "OT";
const char* kModalitySCStr = "SC";

// US
const char* kModalityUSStr = "US";
const char* kModalityIVUSStr = "IVUS";

// NM
const char* kModalityNMStr = "NM";

// PET
const char* kModalityPTStr = "PT";

// -- 2006.06.10 PET SUV needs - probably much more. For MDCT'06, this is enough
const char* kPETDecayFactorStr					= "DecayFactor";
const char* kPETUnitsStr						= "Units";
const char* kPETTotalDoseStr					= "TotalDose";
const char* kPETStartTimeStr					= "RadiopharmaceuticalStartTime";
const char* kPETHalfLifeStr						= "RadiopharmaceuticalHalfLife";
const char* kAcquisitionTimeStr					= "AcquisitionTime";


// DX
const char* kModalityDXStr = "DX";


const char* kSamplesPerPixelStr           = "SamplesPerPixel";
const char* kPhotometricInterpretationStr = "PhotometricInterpretation";
const char* kPlanarConfigurationStr       = "PlanarConfiguration";

// Currently Supported SOPClassUID's
const char* kCRImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.1";
const char* kCTImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.2";
const char* kMRImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.4";
const char* kSCImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.7";



const char* kUSImageStorageSOPClassUID          = "1.2.840.10008.5.1.4.1.1.6.1";
const char* kUSMFImageStorageSOPClassUID        = "1.2.840.10008.5.1.4.1.1.3.1";
const char* kUSRetiredImageStorageSOPClassUID   = "1.2.840.10008.5.1.4.1.1.6";
const char* kUSMFRetiredImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.3";


const char* kXAImageStorageSOPClassUID        = "1.2.840.10008.5.1.4.1.1.12.1";
const char* kXARFImageStorageSOPClassUID      = "1.2.840.10008.5.1.4.1.1.12.2";
const char* kXABiPlaneImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.12.3";

const char* kNMImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.20";
const char* kPTImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.128";

const char* kDXImageStorageSOPClassUID = "1.2.840.10008.5.1.4.1.1.1.1";


// Vikram 09/05/02 - Needed to handle signed data
const char*   kPixelRepresentationStr		= "PixelRepresentation";
const char*   kSliceMinVoxelValueStr		= "SliceMinVoxelValue";
const char*   kSliceMaxVoxelValueStr		= "SliceMaxVoxelValue";

const char*   kGlobalMinVoxelValueStr		= "GlobalMinVoxelValue";
const char*   kGlobalMaxVoxelValueStr		= "GlobalMaxVoxelValue";

const char*   kSliceCompressionFactorStr		= "SliceCompressionFactor";
const char*   kInputCompressionSizeXStr			= "InputCompressionSizeX";
const char*   kInputCompressionSizeYStr			= "InputCompressionSizeY";

const char*   kVOIWindowWidthStr     = "VOIWindowWidth";
const char*   kVOIWindowCenterStr    = "VOIWindowCenter";
const char*	 kModalityLUTStr		= "UseModalityLUT";


// Vikram 07/22/04 NM - Support

const char* kDetectorStr                        = "Detector";
const char* kRotationStr                        = "Rotation";
const char* kEnergyWindowStr                    = "EnergyWindow";
const char* kPhaseStr                           = "Phase";
const char* kRRIntervalStr                      = "RRInterval";
const char* kTimeSlotStr                        = "TimeSlot";
const char* kSliceIndexStr                      = "SliceIndex";
const char* kAngularViewStr                     = "AngularView";
const char* kTimeSliceStr                       = "TimeSlice";
const char* kHasValidOrientationStr             = "HasValidOrientation";
const char* kIsRotationalStr                    = "IsRotational";

const char* kStartAngleStr						= "StartAngle";
const char* kAngularStepStr						= "AngularStep";
const char* kStepDirectionStr					= "StepDirection";
const char* kSpacingBetweenSlicesStr			= "SpacingBetweenSlices";
const char* kSliceThicknessStr					= "SliceThickness";
const char* kUsed1024Str						= "Used1024";
const char*	kImageDateStr						= "ImageDate";
const char* kImageTimeStr						= "ImageTime";
const char* kScanOptionsStr						= "ScanOptions";
const char* kManufacturerStr					= "Manufacturer";

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Convenience Functions
bool IsModalityCT   (std::string& iModality)
{
    return (iModality.compare (kModalityCTStr) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsModalityMR   (std::string& iModality)
{
    return (iModality.compare (kModalityMRStr) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsModalityXA   (std::string& iModality)
{
    return ((iModality.compare (kModalityXAStr) == 0) ||
            (iModality.compare (kModalityXAStr) == 0) ||
            (iModality.compare (kModalityRFStr) == 0));
};

//-----------------------------------------------------------------------------
//
bool IsModalityCRDR (std::string& iModality)
{
    return ((iModality.compare (kModalityCRStr) == 0) ||
            (iModality.compare (kModalityDRStr) == 0));
};

//-----------------------------------------------------------------------------
//
bool IsModalityOT   (std::string& iModality)
{
    return (iModality.compare (kModalityOTStr) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsModalitySC   (std::string& iModality)
{
    return (iModality.compare (kModalitySCStr) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsModalityUS   (std::string& iModality)
{
    return ((iModality.compare (kModalityUSStr) == 0) ||
            (iModality.compare (kModalityIVUSStr) == 0));
}

//-----------------------------------------------------------------------------
//
bool IsModalityDX   (std::string& iModality)
{
    return ((iModality.compare (kModalityDXStr) == 0));
}
//-----------------------------------------------------------------------------
// Convenience Functions
bool IsModalityPT   (std::string& iModality)
{
    return (iModality.compare (kModalityPTStr) == 0);
}

//-----------------------------------------------------------------------------
// Convenience Functions
bool IsModalityNM   (std::string& iModality)
{
    return (iModality.compare (kModalityNMStr) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDCRDR  (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kCRImageStorageSOPClassUID) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDDX  (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kDXImageStorageSOPClassUID) == 0);
}
//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDCT (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kCTImageStorageSOPClassUID) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDMR (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kMRImageStorageSOPClassUID) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDSC  (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kSCImageStorageSOPClassUID) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDNM  (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kNMImageStorageSOPClassUID) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDPT  (std::string& iSOPClassUID)
{
    return (iSOPClassUID.compare (kPTImageStorageSOPClassUID) == 0);
}

//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDXAFamily (std::string& iSOPClassUID)// XA/XF/RF
{
    return ((iSOPClassUID.compare (kXAImageStorageSOPClassUID) == 0) ||
            (iSOPClassUID.compare (kXABiPlaneImageStorageSOPClassUID) == 0) ||
            (iSOPClassUID.compare (kXARFImageStorageSOPClassUID) == 0));
}


//-----------------------------------------------------------------------------
//
bool IsSOPClassUIDUSFamily (std::string& iSOPClassUID) // US/IVUS
{
    return ((iSOPClassUID.compare (kUSImageStorageSOPClassUID) == 0) ||
            (iSOPClassUID.compare (kUSMFImageStorageSOPClassUID) == 0) ||
            (iSOPClassUID.compare (kUSRetiredImageStorageSOPClassUID) == 0) ||
            (iSOPClassUID.compare (kUSMFRetiredImageStorageSOPClassUID) == 0));
}

int GetDefaultCacheSize(void)
{
	return (kCacheDataFileSize+kCacheDescriptionFileSize)/(1024*1024);
}




