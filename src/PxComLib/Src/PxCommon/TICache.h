/***********************************************************************
 * TICache.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon, Inc 2001, All rights reserved.
 *
 *	PURPOSE:
 *		This file contains datastructures share by the cache writer
 *      and reader.
 *
 *	AUTHOR(S):  Vikram Simha September 2001
 *   
 *-------------------------------------------------------------------
 */
#if !defined(TICACHE_H)
#define TICACHE_H

#include <string>
//-----------------------------------------------------------------------------
// Vikram 09/10/02 Changed #defines to const char *
extern const char* kCacheDescriptionFileName;
extern const char* kCacheDataFileName;

extern const char* kJ2kCacheL0DescriptionFileName;
extern const char* kJ2kCacheL0DataFileName;
extern const char* kJ2kCacheL1DescriptionFileName;
extern const char* kJ2kCacheL1DataFileName;



extern const unsigned _int64	kCacheDescriptionFileSize;
extern const unsigned _int64	kCacheDataFileSize;
extern const unsigned _int64	kCacheDataFileSizeBig;
//extern const unsigned _int64    kMaxDataSize ;
extern const unsigned int		kSeriesReserveSpace;



// DICOM Tag = (0018, 5100)
//
enum eDicomPatientPosition
{
	kHeadFirstProne = 0,        // HFP
	kHeadFirstSupine,           // HFS
	kHeadFirstDecubitusRight,   // HFDR
	kHeadFirstDecubitusLeft,    // HFDL
	kFeetFirstProne,            // FFP
	kFeetFirstSupine,           // FFS
	kFeetFirstDecubitusRight,   // FFDR
	kFeetFirstDecubitusLeft,    // FFDL
	kPatientPositionUnknown
};

// DICOM Tag = (0020, 0032)
//
enum eDicomScanType
{
	kSagittal = 0,
	kCoronal,
	kAxial,
	kScanTypeUnknown
};

// DICOM Tag = (0008, 0060)
//
enum eDicomModality
{
	kCT = 0,
	kMR,
	kXA,
	kXF,
	kRF,
	kUS,
	kCR,
	kDR,
	kDX,
	kSC,
	kNM,
	kPT,
	kPX, //K.Ko 2010.03/01  http://xtserv/trac/aqviewer/ticket/622
	kUnsupportedModality
};

enum ePhotometricInterpretation
{
	kUnsupportedFormat = 99,
	kMONOCHROME1 = 0, // 1 sample per pixel  - grayscale from white to black
	kMONOCHROME2,     // 1 sample per pixel  - grayscale from black to white
	kPALETTE_COLOR,   // 1 sample per pixel  - 1 image plane with 3 LUTs
	kRGB,			  // 3 samples per pixel - 3 separate image planes
	kYBR_FULL
};

enum ePlanarConfiguration	//	Only used if Samples / Pixel > 1
{
	kRGBRGB = 0,
	kRRGGBB = 1
};

// Error codes
//
enum eErrorCode
{
	kSuccess = 0,
	kCancelled,
	kErrCannotAccessDirectory,
	kErrCannotOpenFile,
	kErrNotInitialized,
	kErrBadInputParameters,
	kErrNoDiskSpace,
	kErrCannotMapItIn,
	kErrCannotMapItOut,
	kErrFormatNotSupported,
	kErrBeginNotCalled,
	kUnableToParseCacheDescription,
	kChannelOutOfRange,
	kErrCacheNotWritten,
	kErrXAPlaneOurOfRange,
	kErrFileStateIsNotGood,
	kErrFileIsNotOpen,
	kErrNoGroupsPresent,
	kErrCannotCloseFile,
	kErrBadImage,
    kErrSortType,
	kErrCouldNotOpenL0CompressedCache,
	kErrCouldNotDeleteL0CompressedCache,
	kErrCouldNotDeleteL1CompressedCache,
	kErrCouldNotWriteL0CompressedCache,
	kErrCouldNotWriteL1CompressedCache,
	kErrJ2kCompressionFailed,
	kUnknown
};

// Compression Method
//
enum eCompressionMethod
{
	kNone = 0,
	kRLE,
	kJPEG
};


// Sort type
//
enum eSortType
{
	kSortUnknown				  = -1,
    kSortByImagePosition          = 1,
    kSortByImageNumber            = 2,
    kSortByListOfSOPInstanceUIDs  = 3,
    kSortAs2DSlices				  = 4
};

//-----------------------------------------------------------------------------
// Keyword
extern const char* kSliceDescriptionStr;
extern const char* kEndOfSliceDescriptionStr;
extern const char* kModalityStr;
extern const char* kSOPInstanceUIDStr;
extern const char* kSOPClassUIDStr;
extern const char* kBitsStoredStr;
extern const char* kBitsAllocatedStr;
extern const char* kHighBitStr;
extern const char* kSizeXStr;
extern const char* kSizeYStr;
extern const char* kStartOfDataInDataFileStr;
extern const char* kSizeOfDataInBytesStr;
extern const char* kImageTypeTokenStr;

extern const char* kImagePositionStr;
extern const char* kImageOrientationStr;
extern const char* kPixelSpacingStr;
extern const char* kNumberOfEntriesInHistogramStr;
extern const char* kHistogramStr;


extern const char* kRescaleSlopeStr;
extern const char* kRescaleInterceptStr;
extern const char* kModalityLUTStr;

extern const char* kReferencedSOPInstanceUIDStr;

extern const char* kDetectorStr;
extern const char* kRotationStr;
extern const char* kEnergyWindowStr;
extern const char* kPhaseStr;
extern const char* kRRIntervalStr;
extern const char* kTimeSlotStr;
extern const char* kSliceIndexStr;
extern const char* kAngularViewStr;
extern const char* kTimeSliceStr;
extern const char* kHasValidOrientationStr;
extern const char* kIsRotationalStr;


extern const char* kStartAngleStr;
extern const char* kAngularStepStr;
extern const char* kStepDirectionStr;
extern const char* kSpacingBetweenSlicesStr;
extern const char* kSliceThicknessStr;

// T.C. Zhao 2006.05.05
extern const char* kUsed1024Str;
extern const char* kImageDateStr;
extern const char* kImageTimeStr;

//Murali 2007.01.03
extern const char* kScanOptionsStr;
extern const char* kManufacturerStr;

// CT
extern const char* kModalityCTStr;
// This is a type-2 DICOM field but we need it for Tri-phasic data
extern const char* kImageNumberStr; 

// MR
extern const char* kModalityMRStr;

// XA
extern const char* kModalityXAStr;
extern const char* kModalityXFStr;
extern const char* kModalityRFStr;

// CR/DR
extern const char* kModalityCRStr;
extern const char* kModalityDRStr;
extern const char* kModalityOTStr; // TC Zhao 06-20-2002



// SC
extern const char* kModalitySCStr;

// US
extern const char* kModalityUSStr;
extern const char* kModalityIVUSStr;

// NM
extern const char* kModalityNMStr;

// PET
extern const char* kModalityPTStr;

// T.C. Zhao 2006.06.10
extern const char* kPETDecayFactorStr;
extern const char* kPETUnitsStr;
extern const char* kPETTotalDoseStr;
extern const char* kPETStartTimeStr;
extern const char* kPETHalfLifeStr;
extern const char* kAcquisitionTimeStr;

// DX
extern const char* kModalityDXStr; // Vikram 01/14/2004


extern const char* kSamplesPerPixelStr;
extern const char* kPhotometricInterpretationStr;
extern const char* kPlanarConfigurationStr;

// Currently Supported SOPClassUID's
extern const char* kCRImageStorageSOPClassUID;
extern const char* kCTImageStorageSOPClassUID;
extern const char* kMRImageStorageSOPClassUID;

extern const char* kSCImageStorageSOPClassUID;

extern const char* kUSImageStorageSOPClassUID;
extern const char* kUSMFImageStorageSOPClassUID;
extern const char* kUSRetiredImageStorageSOPClassUID;
extern const char* kUSMFRetiredImageStorageSOPClassUID;

extern const char* kXAImageStorageSOPClassUID;
extern const char* kXABiPlaneImageStorageSOPClassUID;
extern const char* kXARFImageStorageSOPClassUID;

extern const char* kNMImageStorageSOPClassUID;
extern const char* kPTImageStorageSOPClassUID;

extern const char* kDXImageStorageSOPClassUID;


// Vikram 09/05/02 - Needed to handle signed data
extern const char*   kPixelRepresentationStr;
extern const char*   kSliceMinVoxelValueStr;
extern const char*   kSliceMaxVoxelValueStr;

extern const char*   kGlobalMinVoxelValueStr;
extern const char*   kGlobalMaxVoxelValueStr;

extern const char*   kSliceCompressionFactorStr;
extern const char*   kInputCompressionSizeXStr;
extern const char*   kInputCompressionSizeYStr;

// VOI LUT Support
// Vikram 11/06/02
extern const char*   kVOIWindowWidthStr;
extern const char*   kVOIWindowCenterStr;
//-----------------------------------------------------------------------------
// Indicies
extern const int   kSliceDescriptionIdx;
extern const int   kEndOfSliceDescriptionIdx;
extern const int   kModalityIdx;
extern const int   kSOPInstanceUIDIdx;
extern const int   kSOPClassUIDIdx;
extern const int   kBitsStoredIdx;
extern const int   kBitsAllocatedIdx;
extern const int   kHighBitIdx;
extern const int   kSizeXIdx;
extern const int   kSizeYIdx;
extern const int   kStartOfDataInDataFileIdx;
extern const int   kSizeOfDataInBytesIdx;
extern const int   kImageTypeTokenIdx;

extern const int   kImagePositionIdx;
extern const int   kImageOrientationIdx;
extern const int   kPixelSpacingIdx;
extern const int   kNumberOfEntriesInHistogramIdx;
extern const int   kHistogramIdx;


extern const int   kRescaleSlopeIdx;
extern const int   kRescaleInterceptIdx;

extern const int   kReferencedSOPInstanceUIDIdx;

extern const int   kGlobalMinVoxelValueIdx;
extern const int   kGlobalMaxVoxelValueIdx;

extern const int   kSliceCompressionFactorIdx;
extern const int   kInputCompressionSizeXIdx;
extern const int   kInputCompressionSizeYIdx;



// CT
extern const int   kModalityCTIdx;
// This is a type-2 DICOM field but we need it for Tri-phasic data
extern const int   kImageNumberIdx; 

// MR
extern const int   kModalityMRIdx;

// XA
extern const int   kModalityXAIdx;
extern const int   kModalityXFIdx;
extern const int   kModalityRFIdx;

// CR/DR
extern const int   kModalityCRIdx;
extern const int   kModalityDRIdx;

// SC
extern const int   kModalitySCIdx;

// OT
extern const int   kModalityOTIdx;

// US
extern const int   kModalityUSIdx;
extern const int   kModalityIVUSIdx;

// NM
extern const int   kModalityNMIdx;

// PET
extern const int   kModalityPTIdx;
extern const int   kPETDecayFactorIdx;
extern const int   kPETUnitsIdx;

// Vikram 09/05/02 - Needed to handle signed data
extern const int   kPixelRepresentationIdx;
extern const int   kSliceMinVoxelValueIdx;
extern const int   kSliceMaxVoxelValueIdx;

extern const int   kDefaultMinMaxVoxelValue;

extern const int   kSamplesPerPixelIdx;;
extern const int   kPhotometricInterpretationIdx;;
extern const int   kPlanarConfigurationIdx;;


// VOI LUT Support
// Vikram 11/06/02
extern const int   kVOIWindowWidthIdx;
extern const int   kVOIWindowCenterIdx;

// Convenience Functions 
// Vikram 07/08/02
extern bool IsModalityCRDR (std::string& iModality);
extern bool IsModalityCT   (std::string& iModality);
extern bool IsModalityMR   (std::string& iModality);
extern bool IsModalityXA   (std::string& iModality);
extern bool IsModalityOT   (std::string& iModality);
extern bool IsModalityUS   (std::string& iModality);
extern bool IsModalitySC   (std::string& iModality);
extern bool IsModalityNM   (std::string& iModality);
extern bool IsModalityPT   (std::string& iModality);
extern bool IsModalityDX   (std::string& iModality);


extern bool IsSOPClassUIDCRDR       (std::string& iModality);
extern bool IsSOPClassUIDCT         (std::string& iModality);
extern bool IsSOPClassUIDMR         (std::string& iModality);
extern bool IsSOPClassUIDSC         (std::string& iModality);
extern bool IsSOPClassUIDNM         (std::string& iModality);
extern bool IsSOPClassUIDPT         (std::string& iModality);
extern bool IsSOPClassUIDDX         (std::string& iModality);

extern bool IsSOPClassUIDXAFamily   (std::string& iModality); // XA/XF/RF
extern bool IsSOPClassUIDUSFamily   (std::string& iModality); // US/IVUS


extern int GetDefaultCacheSize(void);


//-----------------------------------------------------------------------------
#endif // TERARECONCACHE_H
