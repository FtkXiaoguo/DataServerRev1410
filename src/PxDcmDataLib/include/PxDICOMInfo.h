/***********************************************************************
 *  AqDICOMInfo.h
 *---------------------------------------------------------------------
 *		Copyright, Terarecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		Provide AQNet DICOM data store and retriev structure
 *
 *	AUTHOR(S):  Gang Li, June 2004, Vikram Simha July 2005
 *
 *-------------------------------------------------------------------
 */
#ifndef	AQDICOMINFO_H
#define	AQDICOMINFO_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxDICOMStorage.h"

#include "RTVDICOMDef.h"
#include <vector>
#include <string>

#include <windows.h> // for HANDLE

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif /* GUID_DEFINED */

//-----------------------------------------------------------------------------
/*
typedef enum {
    INVALID_TRANSFER_SYNTAX = 0,
    IMPLICIT_LITTLE_ENDIAN = 100,
    EXPLICIT_LITTLE_ENDIAN,
    EXPLICIT_BIG_ENDIAN,
    IMPLICIT_BIG_ENDIAN,
    DEFLATED_EXPLICIT_LITTLE_ENDIAN,
    RLE,                         
    JPEG_BASELINE,               
    JPEG_EXTENDED_2_4,           
    JPEG_EXTENDED_3_5,           
    JPEG_SPEC_NON_HIER_6_8,      
    JPEG_SPEC_NON_HIER_7_9,      
    JPEG_FULL_PROG_NON_HIER_10_12,    
    JPEG_FULL_PROG_NON_HIER_11_13,    
    JPEG_LOSSLESS_NON_HIER_14,        
    JPEG_LOSSLESS_NON_HIER_15,        
    JPEG_EXTENDED_HIER_16_18,         
    JPEG_EXTENDED_HIER_17_19,         
    JPEG_SPEC_HIER_20_22,             
    JPEG_SPEC_HIER_21_23,             
    JPEG_FULL_PROG_HIER_24_26,        
    JPEG_FULL_PROG_HIER_25_27,        
    JPEG_LOSSLESS_HIER_28,       
    JPEG_LOSSLESS_HIER_29,       
    JPEG_LOSSLESS_HIER_14,
    JPEG_2000_LOSSLESS_ONLY,
    JPEG_2000,
    PRIVATE_SYNTAX_1,
    PRIVATE_SYNTAX_2

} TRANSFER_SYNTAX; 
*/

//-----------------------------------------------------------------------------
const int gkPixelStoreFailed = 127;

#ifndef HDISK_SECTOR_SIZE
#define HDISK_SECTOR_SIZE 512
#endif



//-----------------------------------------------------------------------------
// struct for DICOM data description stored in DICOM header file
// the struct will be stored in preamble section (128 bytes) of DICOM data file.

struct AqDICOMHeaderDescription
{
    // >> Data section will have exact match in pixel file header
    char			m_version;							// Version of the Cache Format
    char			m_originalTransferSyntax;			// Orginal TransferSyntax
    char			m_isPixelDataStoredInPixelFile;		// Pixel data stored to pixel file (1/0), 127 -> store failed
    char			m_isPixelDataRemovedFromOriginal;	// Have we removed the pixel Data from the original message/file? (1/0)
    GUID			m_linkKey;							// Unique ID that links DICOM header with the pixel block
    // << End Data section will have exact match in pixel file header
    char			m_pad0[4];			// pad to avoid compiler insert padding

    __int64			m_byteOffsetToStartOfData;		// Offset to pixel data with 512 bytes header offset in pixel file 0
    __int64			m_byteOffsetToStartOfData1;		// Offset to pixel data with 512 bytes header offset in pixel file 1
    __int64			m_byteOffsetToStartOfData2;		// Offset to pixel data with 512 bytes header offset in pixel file 2
    __int64			m_byteOffsetToStartOfData3;		// Offset to pixel data with 512 bytes header offset in pixel file 3
    __int64			m_byteOffsetToStartOfData4;		// Offset to pixel data with 512 bytes header offset in pixel file 4
    __int64			m_byteOffsetToStartOfData5;		// Offset to pixel data with 512 bytes header offset in pixel file 5
    __int64			m_byteOffsetToStartOfData6;		// Offset to pixel data with 512 bytes header offset in pixel file 6
    
    // size 80 bytes
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This struct will be put in the first block of pixel file and sort dictionary
// in sort dictionary, following the AqDICOMSeriesInfo is a list of AqDICOMImageInfo 
// structure for all frames for all images (not include the frame offset tables).


struct AqDICOMSeriesInfo
{
    // >> Data section that has exact match in DICOM header
    char			m_version;							// Version of the Cache Format
    char			m_originalTransferSyntax;			// Orginal TransferSyntax
    char			m_isPixelDataStoredInPixelFile;		// Pixel data stored to pixel file (1/0), 127 -> store failed
    char			m_isPixelDataRemovedFromOriginal;	// Have we removed the pixel Data from the original message/file? (1/0)
    GUID			m_linkKey;							// Unique ID that links DICOM header with the pixel block
    // << End data section that has exact match in DICOM header
    
    long	m_numberOfFrames;
    long	m_seriesNumber;

    char	m_studyInstanceUID[ kVR_UI ];
    char	m_seriesInstanceUID[ kVR_UI ];			
    char	m_seriesDescription[ kVR_LO ];			
    char	m_modality[ kVR_CS ];					
    char	m_bodyPartExamined[ kVR_CS ];			
    char	m_viewPosition[ kVR_CS ];

    //size -> 274

    char	padding[238];
};

//-----------------------------------------------------------------------------
// struct for DICOM pixel validation information stored in start of each DICOM pixel data
// (fix size 512 bytes)
// If the image has multiple frames, the first AqDICOMImageInfo is followed by a frame 
// offset table. Then a list of frames which has this structure in front.

struct AqDICOMImageInfo
{
    // >> Data section will have exact match in pixel file header
    char			m_version;							// Version of the Cache Format
    char			m_originalTransferSyntax;			// Orginal TransferSyntax
    char			m_isPixelDataStoredInPixelFile;		// Pixel data stored to pixel file (1/0), 127 -> store failed
    char			m_isPixelDataRemovedFromOriginal;	// Have we removed the pixel Data from the original message/file? (1/0)
    GUID			m_linkKey;							// Unique ID that links DICOM header with the pixel block
    // << End Data section will have exact match in pixel file header

    unsigned long	m_storedTransferSyntax;		// Stored TransferSyntax. This could be different
    	                                        // from the orginal transfer syntax. E.g. If we decide
    	                                        // to store all imcoming data as J2K_lossless

    __int64			m_byteOffsetToStartOfData;	// Offset to start of this data. 
    											// This consists of a 512 byte header + pixel data

    unsigned long	m_dataSize;					// Size of pixel data in pixel block
    											// For multi-frame,
    											// m_dataSize = size of all frame header + 
    	                                        //              sise of all frame pixel

    unsigned long	m_dataSizeWithPadding;		// Pixel data size in pixel block + padding
    											// For multi-frame,
    											// m_dataSizeWithPadding = size of all frame header + 
    											//                         size of all frame pixel  +
    											//                         size of all padding
    // Size 40 bytes

    // Information for sorting and compression
    // Currently the data is save as uncompressed little-endian
    float m_pixelSpacing[2];		// 6 floats together for alignment
    float m_aspectRatio;
    float m_imagePosition[3];
    
    double m_rescaleSlope;
    double m_rescaleIntercept;
    double m_imageOrientation[6];

    //128 bytes

    char m_SOPInstanceUID[kVR_UI];
    char m_SOPClassUID[kVR_UI];
    char m_imageTypeTokens[kVR_UI];
    char m_referencedSOPInstanceUID[kVR_UI]; // 260 char togather for alignment
    
    int  m_instanceNumber;
    // Size 392 bytes

    unsigned short m_rows;
    unsigned short m_columns;
    unsigned short m_numberOfFrames;
    unsigned short m_frameID;



  	float m_windowWidth;
  	float m_windowCenter;
    int m_smallestPixelValue;
    int m_largestPixelValue;
    
    unsigned char m_bitsAllocated;
    unsigned char m_bitsStored;
    unsigned char m_highBit;
    unsigned char m_pixelRepresentation;
    unsigned char m_photometricInterpretation; // (use enum)
    unsigned char m_samplesPerPixel;
    

    unsigned short m_planarConfiguration; 
    
    //char pad0; //size -> 424
    float m_sliceThickness;

    // added fields are NM image sorting
    short m_energyWindowIndex;
    short m_detectorIndex;
    short m_phaseIndex;
    short m_rotationIndex;
    short m_rrIntervalIndex;
    short m_timeSlotIndex;
    short m_sliceIndex;
    short m_angularViewIndex;
    short m_timeSliceIndex;
    // Size -> 442

	// Murali 2007.01.03 copied from instanceLevel, need this for sorting
	char m_imageDate[ kVR_DA ];
	char m_imageTime[ kVR_TM ];
	char m_wasLossyCompressed;

	// Murali 2007.01.03 copied from instanceLevel, need this for sorting	
	char m_scanOptions[kVR_CS];
	char m_manufacturer[kVR_LO];

    char padding[66]; // pad up to sector size(512)

};

//-----------------------------------------------------------------------------
// This is place holder class that is inserted into a list so 
// everything in AqDICOMImageInfo is not copied twice
// This class does not override the copy constructor or assignment operator
// as we want only the address of m_ptr to be copied
// Vikram Simha 07/10/2005
//
class AqPtrToDICOMImageInfo
{ 
	public:		
    	explicit AqPtrToDICOMImageInfo (AqDICOMImageInfo* iPtr)
    	{
    		m_ptr = iPtr;
    	}

    	AqPtrToDICOMImageInfo ()
    	{
    		m_ptr = 0;
    	}

    	~AqPtrToDICOMImageInfo ()
    	{

    		// The pointer is not deleted in the desctuctor intentionally
    		// This is done to over come the deletion that occurs when you 
    		// insert into a vector - Vikram
    		//
    		//if (m_ptr) delete m_ptr;
    	}
    	

    	// This is called explicity outside
    	//
    	void DeleteData()
    	{
    		if (m_ptr) delete m_ptr, m_ptr=0;
    	}


    	bool operator < (const AqPtrToDICOMImageInfo&) const;
    	bool operator == (const AqPtrToDICOMImageInfo&) const;
    	
    	
		// Get the pointer to the acutal data
    	AqDICOMImageInfo* AllocateAndGetPtrToAqDicomImageInfo (void)  
		{
			if (!m_ptr) m_ptr = new AqDICOMImageInfo();
			return m_ptr;
		}
    	
    	// Access functions
    	char GetCacheVersion (void) const			{return m_ptr ? m_ptr->m_version : -126;}
    	char GetOriginalTransferSyntax (void) const {return m_ptr ? m_ptr->m_originalTransferSyntax : 0;}
    
    	char IsPixelDataStoredInPixelFile   (void) const {return m_ptr ? m_ptr->m_isPixelDataStoredInPixelFile : 0;}
    	char IsPixelDataRemovedFromOriginal (void) const {return m_ptr ? m_ptr->m_isPixelDataRemovedFromOriginal : 0;}

//		GUID GetLinkKey (void) const {return m_ptr ? m_ptr->m_linkKey : ;}

    	unsigned long GetStoredTransferSyntax (void) const {return m_ptr ? m_ptr->m_storedTransferSyntax : 0;}

    	__int64		  GetByteOffsetToStartOfData (void) const {return m_ptr ? m_ptr->m_byteOffsetToStartOfData : 0;} 
    	unsigned long GetDataSize                (void) const {return m_ptr ? m_ptr->m_dataSize :0;}
    	unsigned long GetDataSizeWithPaddeing    (void) const {return m_ptr ? m_ptr->m_dataSizeWithPadding : 0;}
    
    	const float*  GetPixelSpacing  (void) const {return m_ptr ? m_ptr->m_pixelSpacing : 0;}
    	float		  GetAspectRatio   (void) const {return m_ptr ? m_ptr->m_aspectRatio : 0;}
    	const float*  GetImagePosition (void) const {return m_ptr ? m_ptr->m_imagePosition : 0;}

    	double GetRescaleSlope     (void) const {return m_ptr ? m_ptr->m_rescaleSlope : 1;}
    	double GetRescaleIntercept (void) const {return m_ptr ? m_ptr->m_rescaleIntercept:0;}

    	const double* GetImageOrientation (void) const {return m_ptr ? m_ptr->m_imageOrientation : 0;}

    	const char* GetSOPInstanceUID  (void) const {return m_ptr ? m_ptr->m_SOPInstanceUID : 0;}
    	const char* GetSOPClassUID     (void) const {return m_ptr ? m_ptr->m_SOPClassUID : 0;}
    	const char* GetImageTypeTokens (void) const {return m_ptr ? m_ptr->m_imageTypeTokens : 0;}
    	
    	const char* GetReferencedSOPInstanceUID (void) const {return m_ptr ? m_ptr->m_referencedSOPInstanceUID : 0;}
    
    	int GetInstanceNumber (void) const {return m_ptr ? m_ptr->m_instanceNumber : 0;}
    

    	unsigned short GetNumberOfRows    (void) const {return m_ptr ? m_ptr->m_rows : 0;}
    	unsigned short GetNumberofColumns (void) const {return m_ptr ? m_ptr->m_columns : 0;}

    	unsigned short GetXSize (void) const {return m_ptr ? m_ptr->m_columns : 0;}
    	unsigned short GetYSize (void) const {return m_ptr ? m_ptr->m_rows : 0;}
    	
    	unsigned short GetNumberOfFrames  (void) const {return m_ptr ? m_ptr->m_numberOfFrames : 0;}
    	unsigned short GetFrameID         (void) const {return m_ptr ? m_ptr->m_frameID : 0;}

    	int GetWindowWidth (void) const {return m_ptr ? (int)(m_ptr->m_windowWidth) : 0;}
    	int GetWindoCenter (void) const {return m_ptr ? (int)(m_ptr->m_windowCenter) : 0;}


    	int GetSmallestPixelValue (void) const {return m_ptr ? m_ptr->m_smallestPixelValue : 0;}
    	int GetLargertPixelValue  (void) const {return m_ptr ? m_ptr->m_largestPixelValue : 0;}

    	unsigned char GetBitsAlloocated (void) const {return m_ptr ? m_ptr->m_bitsAllocated : 0;}
    	unsigned char GetBitsStored     (void) const {return m_ptr ? m_ptr->m_bitsStored : 0;}
    	unsigned char GetHighBit        (void) const {return m_ptr ? m_ptr->m_highBit : 0;}

    	unsigned char GetPixelRepresentation	   (void) const {return m_ptr ? m_ptr->m_pixelRepresentation : 0;}
    	unsigned char GetPhotometricInterpretation (void) const {return m_ptr ? m_ptr->m_photometricInterpretation : 0;}
    	unsigned char GetPlanarConfiguration	   (void) const {return m_ptr ? m_ptr->m_planarConfiguration : 0;}
    	unsigned char GetSamplePerPixels           (void) const {return m_ptr ? m_ptr->m_samplesPerPixel : 0;}

    	// Added fields for NM image sorting
    	unsigned short GetEnergyWindowIndex (void) const {return m_ptr ? m_ptr->m_energyWindowIndex : 0;}
    	unsigned short GetDetectorIndex     (void) const {return m_ptr ? m_ptr->m_detectorIndex : 0;}
    	unsigned short GetPhaseIndex		(void) const {return m_ptr ? m_ptr->m_phaseIndex : 0;}
    	unsigned short GetRotationIndex     (void) const {return m_ptr ? m_ptr->m_rotationIndex : 0;}
    	unsigned short GetRRIntervalIndex   (void) const {return m_ptr ? m_ptr->m_rrIntervalIndex : 0;}
    	unsigned short GetTimeSlotIndex     (void) const {return m_ptr ? m_ptr->m_timeSlotIndex : 0;}
    	unsigned short GetSliceIndex		(void) const {return m_ptr ? m_ptr->m_sliceIndex : 0;}
    	unsigned short GetAngularViewIndex  (void) const {return m_ptr ? m_ptr->m_angularViewIndex : 0 ;}
    	unsigned short GetTimeSliceIndex	(void) const {return m_ptr ? m_ptr->m_timeSliceIndex : 0;}

    private:
    	AqDICOMImageInfo* m_ptr;
};

//-----------------------------------------------------------------------------
enum NMSortingIndex
{
    ENERGY_WINDOW = 1,
    DETECTOR,
    PHASE,
    ROTATION,
    RRINTERVAL,
    TIME_SLOT,
    SLICE,
    AUGULAR_VIEW,
    TIME_SLICE
};

//-----------------------------------------------------------------------------
// struct for NM specific information
// temporary, will be changed according
// actual information needed during sorting process.
struct AqDICOMNMImageInfo
{
    int	 m_modalityInfoSize; // include AqDICOMNMImageInfo + vector informations
    int  m_modalityInfoSizeWithPadding;

    unsigned short m_numberOfEnergyWindow;
    unsigned short m_numberOfDetector;
    unsigned short m_numberOfPhase;
    unsigned short m_numberOfRotation;
    unsigned short m_numberOfRRInterval;
    unsigned short m_numberOfTimeSlot;
    unsigned short m_numberOfSlice;
//	unsigned short m_numberOfAngularView;
//	unsigned short m_numberOfTimeSlice;

    char m_frameIncrementPointer[9];
    char padding; // pad up size(32)

};

//-----------------------------------------------------------------------------
// dumpy struct for energy window information
struct AqDICOMNMEnergyWindowInfo
{
    int dumpyNumber;
    	
};

//-----------------------------------------------------------------------------
// struct for detector information
// temporary, will be changed according
// actual information needed during sorting process.
struct AqDICOMNMDetectorInfo
{
    double m_imagePosition[3];		
    double m_imageOrientation[6];	
    int    m_focalDistance;			
    int	   m_distSourceToDetector;	
    char   m_collimatorType[8];		
};

//-----------------------------------------------------------------------------
// struct for rotation information
// temporary, will be changed according
// actual information needed during sorting process.
struct AqDICOMNMRotationInfo
{
    double m_startAngle;			
    double m_angularStep;			
    int m_scanArc;					
    int m_frameDuration;			
    int m_numberOfFramesInRotation;	
    int	m_rotationDirection;		
};

//-----------------------------------------------------------------------------
// dumpy struct for RRInterval
struct AqDICOMNMRRInternalInfo
{
    int dumpyNumber;	
};

//-----------------------------------------------------------------------------
// dumpy struct for phase
struct AqDICOMNMPhaseInfo
{
    int dumpyNumber;	
};

//-----------------------------------------------------------------------------
// dumpy struct for TimeSlot
struct AqDICOMNMTimeSlotInfo
{
    int dumpyNumber;	
};

//-----------------------------------------------------------------------------
// dumpy struct for Slice
struct AqDICOMNMSliceInfo
{
    int dumpyNumber;	
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class HandleGuard
{
public:
    HandleGuard(HANDLE iHandle = INVALID_HANDLE_VALUE): m_handle(iHandle) {};
    	virtual ~HandleGuard() {Close();}
    
    void Set(HANDLE iHandle) {Close(); m_handle = iHandle;};

    void Close()  
    {
    	if(m_handle != INVALID_HANDLE_VALUE) 
    		CloseHandle(m_handle), m_handle=INVALID_HANDLE_VALUE;
    }
    

    private:
    HANDLE m_handle;

};
//-----------------------------------------------------------------------------
class GDIOBJGuard
{
	public:
		GDIOBJGuard(HGDIOBJ iHandle = INVALID_HANDLE_VALUE) :  m_handle(iHandle) {};
		~GDIOBJGuard() {Close();}
    
		void Set(HGDIOBJ iHandle) {Close(); m_handle = iHandle;};
		void Close()  
		{
    		if(m_handle != INVALID_HANDLE_VALUE) 
    			DeleteObject(m_handle), m_handle=INVALID_HANDLE_VALUE;
		}

	protected:
		HGDIOBJ m_handle;

};

//-----------------------------------------------------------------------------
class DCGuard
{
	public:
		DCGuard(HDC iHandle = (HDC)INVALID_HANDLE_VALUE) :  m_handle(iHandle) {};
		~DCGuard() {Close();}

		void Set(HDC iHandle) {Close(); m_handle = iHandle;};
		void Close()  
		{
    		if(m_handle == INVALID_HANDLE_VALUE)
    			return;

    		for(int i=0; i<m_oldGDIObjects.size(); i++) 
    			::SelectObject(m_handle, m_oldGDIObjects[i]);
    		DeleteDC(m_handle), m_handle=(HDC)INVALID_HANDLE_VALUE;
		}

		void AddOldGDIObj(HGDIOBJ iObj) {m_oldGDIObjects.push_back(iObj);}; 

	protected:

		std::vector<HGDIOBJ> m_oldGDIObjects;
		HDC m_handle;

};


//-----------------------------------------------------------------------------

extern int SectorReadFile(HANDLE hFile, __int64 iBytesToSkip, int iBytesToRead, AqBuffer& iBuffer);
extern int SectorReadFile(HANDLE hFile, __int64 iBytesToSkip, int iBytesToRead, char* iBuffer);

//-----------------------------------------------------------------------------
#endif	/* AQDICOMINFO_H */
