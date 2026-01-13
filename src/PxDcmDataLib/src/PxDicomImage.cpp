//
// CPxDicomImage.cpp
//
//
#pragma warning (disable: 4786)
#pragma warning (disable: 4616)
#pragma warning (disable: 4530)

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "IDcmLibApi.h"

#include "PxDICOMUtil.h"
//#include "RTVDicomService.h"
#include "PxDicomImage.h"
#include "auxdata.h"
#include "rtvsutil.h"
#include "PxDBData.h"

#include "AqCore/AqCore.h"
#include "AqCore/TRCriticalsection.h"
#include "AqCore/TRAtomic.h"

#include "CheckMemoryLeak.h"

using namespace XTDcmLib;

// Axes of Acquisition
#define kAX		0
#define kCOR	1
#define kSAG	2

// Directions in which the head can point
#define kHF	0		// Head First
#define kFF	1		// Feet First

// Directions in which the nose can point
#define kSU	0		// Supine
#define kDL 1		// Decubitus Left
#define kPR	2		// Prone

//#define kDR 3		// Decubitus Right 
// 
// Changed this so it does not confiict with the enum 
// eDicomModality
#define kDecubitusR 3		// Decubitus Right 


static const double kUnitVectors[3][2][4][6] = 
{
	{ // Axial
		{ // Head First
			{1,0,0,0,1,0},		// Supine
			{0,-1,0,1,0,0},		// Decubitus Left
			{-1,0,0,0,-1,0},	// Prone
			{0,1,0,-1,0,0}		// Decubitus Right
		},
		
		{ // Feet First
			{-1,0,0,0,1,0},		// Supine
			{0,1,0,1,0,0},		// Decubitus Left
			{1,0,0,0,-1,0},		// Prone
			{0,-1,0,-1,0,0}		// Decubitus Right	
		}
	},

	{ // Coronal - Not handled yet	
		{
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9}
		},
		
		{
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9}
		}
	},

	{ // Sagittal - Not handled yet
		{
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9}
		},
		
		{
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9},
			{9,9,9,9}
		}
	}
};

//-----------------------------------------------------------------------------
//
int CPxDicomImage::WaitForConversionToComplete()
	{
		if (m_isConverting) 
			m_conversionComplete->Wait(2000);
		return 0;
	}

void CPxDicomImage::SetConversionComplete()
	{
		m_isConverting = 0;
		m_converted = 1;
		m_conversionComplete->Post();
	}
//-----------------------------------------------------------------------------
//
bool   CPxDicomImage::createMyResource()
{
	m_cs = new TRCriticalSection;
	m_conversionComplete = new TRSemaphore;
	return true;
}
//-----------------------------------------------------------------------------
//
CPxDicomImage::CPxDicomImage()
{
	m_msgLogoutInvalidUIDFlag = 1;//#91 2017/01/12 N.Furutsuki

	createMyResource();

	m_palette = 0; // 
	m_imagePixels.clear();
	m_OBOWbuffer = 0;

	Reset();
}


//-----------------------------------------------------------------------------
//
CPxDicomImage::CPxDicomImage(int iID, int MsgLogout )
{
	m_msgLogoutInvalidUIDFlag = MsgLogout;//#91 2017/01/12 N.Furutsuki

	createMyResource();

	m_palette = 0; // 
	m_imagePixels.clear();
	m_OBOWbuffer = 0;

	Reset();
	PopulateFromMessage(iID);
}


//-----------------------------------------------------------------------------
//
CPxDicomImage::~CPxDicomImage()
{
	 
	if (m_cs)
		delete m_cs, m_cs = 0;
	if (m_conversionComplete)
		delete m_conversionComplete, m_conversionComplete = 0;

	

	if (m_palette)
		delete m_palette, m_palette = 0;

	ReleaseImagePixels();
	if(m_OBOWbuffer)
		delete[] m_OBOWbuffer, m_OBOWbuffer=0;
}

//-----------------------------------------------------------------------------
//
//   - set default values for members
//
void CPxDicomImage::Reset(void)
{
	m_imageStorageType = kUnknownImage;
	m_scanType = kScanTypeUnknown;
	m_modality = kUnsupportedModality;
	m_modalityStr = "Unknown";
	m_patientPosition = kPatientPositionUnknown;

	m_axisOfAcquisition[0] = 0;
	m_axisOfAcquisition[1] = 0;
	m_axisOfAcquisition[2] = 0;

	m_numberOfBytesOfPixelData = 0L;
    m_numberOfColumns = 0;
	m_numberOfRows = 0;

	m_pixelRepresentation = 0;
	m_bitsAllocated = 0;
	m_bitsStored = 0;
	m_highBit = 0;
	m_bytesPerPixel = 0;
	m_samplesPerPixel = 0;
	m_photometricInterpretation = kUnsupportedFormat;
	m_planarConfiguration = kRGBRGB;
	m_isSecondaryCapture = false;
	m_isBiPlane = false;
	m_instanceNumber = 0;
	m_isLittleEndian = 1;
	m_scaledW = 0;
	m_scaledH = 0;

	if(m_OBOWbuffer)
		delete[] m_OBOWbuffer, m_OBOWbuffer=0;

	m_OBOWoffset = 0;
	m_OBOWlength = 0;
	
	if (m_palette)
		delete m_palette, m_palette = 0;

	ReleaseImagePixels();


	CPxDicomMessage::Reset();
	
	m_studyInstanceUID[0] = '\0';
	m_seriesInstanceUID[0] = '\0';
	m_SOPInstanceUID[0] = '\0';
	m_SOPClassUID[0] = '\0'; 
	m_referencedSOPInstanceUID[0] = '\0';

	m_imageTypeTokens.clear();
 
	m_rescaleIntercept = 0.0;
	m_rescaleSlope     = 1.0;

	m_windowCenter	 = 0;
	m_windowWidth	 = 0;

	m_numberOfFrames = 1;	
	
	m_pixelSpacing[0] = 1.0;
	m_pixelSpacing[1] = 1.0;
	m_sliceMinVoxelValue =  99999;
	m_sliceMaxVoxelValue = -99999;

	m_isCompressed = false; 

	m_converted = 0;
	m_isConverting = 0;
	m_autoConvert = 0;

	//	Default to upper left
	m_imagePosition[0] = 0;
	m_imagePosition[1] = 0;
	m_imagePosition[2] = 0;

	//	Default to Axial Supine
	m_imageOrientation[0] = 1;
	m_imageOrientation[1] = 0;
	m_imageOrientation[2] = 0;
	m_imageOrientation[3] = 0;
	m_imageOrientation[4] = 1;
	m_imageOrientation[5] = 0;

	m_hasValidImageOrientation = false;
	m_hasValidImagePosition = false;
	m_filePath = "";

	m_sliceThickness = 0.;

	m_imageDate[0] = '\0';
	m_imageTime[0] = '\0';

	//  - PET SUV stuff
	m_PETDecayFactor = 0.0f;
	m_PETUnits[0] = '\0';
	m_radionuclideTotalDose = 0;
	m_acquisitionTime[0] = '\0';
	m_radioPharmaStartTime[0] = '\0';
	m_radionuclideHalfLife = 0;
	m_patientWeight = 0.0;

	m_scanOptions[0]  = '\0'; // 
	m_manufacturer[0] = '\0';

}


//-----------------------------------------------------------------------------
//
//	Copy Constructor
//
CPxDicomImage::CPxDicomImage(const CPxDicomImage& iImage)
{
	createMyResource();

	m_palette = 0; // 
	m_imagePixels.clear();
	m_OBOWbuffer = 0;

	Reset();

	// 
	// Conslidate copy operator and copy constructor. no reason
	// to have duplicated code
	*this = iImage;
} 

//-----------------------------------------------------------------------------
//
//	Assignment operator
//
CPxDicomImage& CPxDicomImage::operator= (const CPxDicomImage& iImage)	

{
	int i;
	Reset();
	
	//	- Added initialization of members 
	//		(not initializing caused db insert problems)
	m_imageStorageType = iImage.m_imageStorageType;
	m_patientPosition = iImage.m_patientPosition;
	m_windowCenter = iImage.m_windowCenter;
	m_windowWidth = iImage.m_windowWidth;
	m_isCompressed = iImage.m_isCompressed;
	m_isBiPlane = iImage.m_isBiPlane;
	m_instanceNumber = iImage.m_instanceNumber;
	m_isLittleEndian = iImage.m_isLittleEndian;
	m_OBOWoffset = iImage.m_OBOWoffset;
	m_OBOWlength = iImage.m_OBOWlength;
	m_autoConvert = iImage.m_autoConvert;
	m_hasValidImageOrientation = iImage.m_hasValidImageOrientation;
	m_hasValidImagePosition = iImage.m_hasValidImagePosition;
	m_filePath = iImage.m_filePath;

	ASTRNCPY(m_studyInstanceUID, iImage.m_studyInstanceUID);
	ASTRNCPY(m_seriesInstanceUID, iImage.m_seriesInstanceUID);
	ASTRNCPY(m_SOPInstanceUID, iImage.m_SOPInstanceUID);
	ASTRNCPY(m_SOPClassUID, iImage.m_SOPClassUID);
	ASTRNCPY(m_referencedSOPInstanceUID, iImage.m_referencedSOPInstanceUID);

	m_imagePixels = iImage.m_imagePixels;
	m_scanType = iImage.m_scanType;
	m_modality = iImage.m_modality;
	m_modalityStr = iImage.m_modalityStr;
	m_patientPosition = iImage.m_patientPosition;


	m_imageTypeTokens = iImage.m_imageTypeTokens;
	

	for( i=0;i<3;i++)
	{
		m_axisOfAcquisition[i] = iImage.m_axisOfAcquisition[i];
		m_imagePosition[i] = iImage.m_imagePosition[i];
	}

	for(i=0;i<6;i++)
	{
		m_imageOrientation[i] = iImage.m_imageOrientation[i];
	}

	for(i=0;i<2;i++)
	{
		m_pixelSpacing[i] = iImage.m_pixelSpacing[i];
	}

	m_numberOfBytesOfPixelData = iImage.m_numberOfBytesOfPixelData;
	m_rescaleIntercept = iImage.m_rescaleIntercept;
	m_rescaleSlope = iImage.m_rescaleSlope;
	m_numberOfColumns =  iImage.m_numberOfColumns;
	m_numberOfRows =  iImage.m_numberOfRows;
	m_numberOfFrames = iImage.m_numberOfFrames;
	m_scaledW		= iImage.m_scaledW;
	m_scaledH		= iImage.m_scaledH;
	m_converted		= iImage.m_converted;
	m_isConverting	= iImage.m_isConverting;
	m_pixelRepresentation = iImage.m_pixelRepresentation;
	m_bitsAllocated = iImage.m_bitsAllocated;
	m_bitsStored = iImage.m_bitsStored;
	m_highBit = iImage.m_highBit;
	m_bytesPerPixel = iImage.m_bytesPerPixel;
	m_samplesPerPixel = iImage.m_samplesPerPixel;
	m_photometricInterpretation = iImage.m_photometricInterpretation;
	m_planarConfiguration = iImage.m_planarConfiguration;

	//	
	strncpy(m_SOPClassUID, iImage.m_SOPClassUID, sizeof(m_SOPClassUID));
	m_isSecondaryCapture = iImage.m_isSecondaryCapture;
	m_sliceMinVoxelValue =  iImage.m_sliceMinVoxelValue;
	m_sliceMaxVoxelValue = iImage.m_sliceMaxVoxelValue;

	if (iImage.GetPalette())
	{
		m_palette = new PxDicomPalette(*iImage.GetPalette());
	}
	m_sliceThickness = iImage.m_sliceThickness;


	// 
	strcpy(m_imageDate, iImage.m_imageDate);
	strcpy(m_imageTime, iImage.m_imageTime);

	// 
	strcpy(m_PETUnits, iImage.GetPETUnits());
	m_PETDecayFactor = iImage.GetPETDecayFactor();
	m_radionuclideTotalDose = iImage.m_radionuclideTotalDose;
	strcpy(m_acquisitionTime,iImage.m_acquisitionTime);
	strcpy(m_radioPharmaStartTime,iImage.m_radioPharmaStartTime);
	m_radionuclideHalfLife = iImage.m_radionuclideHalfLife;
	m_patientWeight		= iImage.m_patientWeight;

	strcpy(m_scanOptions, iImage.m_scanOptions);
	strncpy(m_manufacturer, iImage.m_manufacturer,
						    sizeof(m_manufacturer));
	return *this;
}



//-----------------------------------------------------------------------------
//
int CPxDicomImage::GetLargestPixelValueInSeries(void) 
{
	int maxVal = 0;
	if (GetValue(kVLILargestPixelValueInSeries,&maxVal) != kNormalCompletion)
		maxVal = 0;
	return maxVal;
}

//-----------------------------------------------------------------------------
//
int CPxDicomImage::GetSmallestPixelValueInSeries(void) 
{
	int minVal = 0;
	if (GetValue(kVLISmallestPixelValueInSeries,&minVal) != kNormalCompletion)
		minVal = 0;
	return minVal;
}


//-----------------------------------------------------------------------------
//
int CPxDicomImage::GetLargestPixelValue(void) 
{
	int maxVal = 0;
	if (GetValue(kVLILargestImagePixelValue,&maxVal) != kNormalCompletion)
		maxVal = 0;
	return maxVal;
}

//-----------------------------------------------------------------------------
//
int CPxDicomImage::GetSmallestPixelValue(void) 
{
	int minVal = 0;
	if (GetValue(kVLISmallestImagePixelValue,&minVal) != kNormalCompletion)
		minVal = 0;
	return minVal;
}
//-----------------------------------------------------------------------------
//
bool UnitVectorsAreEqual(const double* uv1, const double* uv2)
{
	bool isEqual = true;

	for(int i=0;i<6;i++)
	{
		if (uv1[i] != uv2[i])
		{
			isEqual = false;
		}
	}

	return isEqual;
}

//-----------------------------------------------------------------------------
//
void CPxDicomImage::ConvertPatientPosition(char* iPosition)
{
	if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kHF][kSU]))
	{
		m_patientPosition = kHeadFirstSupine;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kHF][kDL]))
	{
		m_patientPosition = kHeadFirstDecubitusLeft;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kHF][kPR]))
	{
		m_patientPosition = kHeadFirstProne;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kHF][kDecubitusR]))
	{
		m_patientPosition = kHeadFirstDecubitusRight;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kFF][kSU]))
	{
		m_patientPosition = kFeetFirstSupine;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kFF][kDL]))
	{
		m_patientPosition = kFeetFirstDecubitusLeft;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kFF][kPR]))
	{
		m_patientPosition = kFeetFirstProne;
	} else if (UnitVectorsAreEqual(m_imageOrientation, kUnitVectors[kAX][kFF][kDecubitusR]))
	{
		m_patientPosition = kFeetFirstDecubitusRight;
	} else
	{
		m_patientPosition = kPatientPositionUnknown;
	}
}

//-----------------------------------------------------------------------------
// instead of dragging in auxdata class here to affect many projects,
// do this on the fly
static inline bool HasAuxData(int msgID)
{
	int valueCount = 0;
	
	MC_Get_pValue_Count(msgID, TR_CREATOR, TR_GROUP, TR_ATT_FIRST_BINARY_DATA, &valueCount);
	
	if (valueCount <= 0)
		MC_Get_Value_Count(msgID, TR_ATT_CA_REPORT, &valueCount);
	
	if (valueCount <= 0)
		MC_Get_Value_Count(msgID, TR_ATT_CA_SCORE, &valueCount);
	
	return valueCount > 0;
}

//-----------------------------------------------------------------------------
//
//GL PopulateFromMessage did not reset tags before populate
// only work for fresh created object
void CPxDicomImage::PopulateFromMessage(int iID)
{
	MC_STATUS stat;

	m_messageID = iID;
	int dummy;

	stat = MC_Get_Value_To_String(iID, MC_ATT_STUDY_INSTANCE_UID, sizeof(m_studyInstanceUID), m_studyInstanceUID);
	if (stat != MC_NORMAL_COMPLETION)
	{
		if (m_msgLogoutInvalidUIDFlag == 1){//#91 2017/01/12 N.Furutsuki
			m_status = (PxDicomStatus)stat;
			GetAqLogger()->LogMessage(kErrorOnly, "ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get MC_ATT_STUDY_INSTANCE_UID - %s.\n", MC_Error_Message((MC_STATUS)m_status));
		}
		return;
	}

	iRTVDeSpaceDe(m_studyInstanceUID);
		

	stat = MC_Get_Value_To_String(iID, MC_ATT_SERIES_INSTANCE_UID, sizeof(m_seriesInstanceUID), m_seriesInstanceUID);
	if (stat != MC_NORMAL_COMPLETION)
	{
		if (m_msgLogoutInvalidUIDFlag == 1){//#91 2017/01/12 N.Furutsuki
			m_status = (PxDicomStatus)stat;
			GetAqLogger()->LogMessage(kErrorOnly, "ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get MC_ATT_SERIES_INSTANCE_UID - %s.\n", MC_Error_Message((MC_STATUS)m_status));
		}
		return;
	}

	iRTVDeSpaceDe(m_seriesInstanceUID);


	stat = MC_Get_Value_To_String(iID, MC_ATT_SOP_INSTANCE_UID, sizeof(m_SOPInstanceUID), m_SOPInstanceUID);
	if (stat != MC_NORMAL_COMPLETION)
	{
		if (m_msgLogoutInvalidUIDFlag == 1){//#91 2017/01/12 N.Furutsuki
			m_status = (PxDicomStatus)stat;
			GetAqLogger()->LogMessage(kErrorOnly, "ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get MC_ATT_SOP_INSTANCE_UID - %s.\n", MC_Error_Message((MC_STATUS)m_status));
		}
		return;
	}

	iRTVDeSpaceDe(m_SOPInstanceUID);


	// 
	MC_Get_Value_To_String(iID, MC_ATT_CONTENT_DATE, sizeof(m_imageDate), m_imageDate);
	m_imageDate[sizeof(m_imageDate) - 1] = '\0';
	MC_Get_Value_To_String(iID, MC_ATT_CONTENT_TIME, sizeof(m_imageTime), m_imageTime);
	m_imageTime[sizeof(m_imageTime) - 1] = '\0'; 


	// 
	MC_Get_Value_To_String(iID, MC_ATT_ACQUISITION_TIME, sizeof m_acquisitionTime, m_acquisitionTime);


	// for scanoptions based sorting
	MC_Get_Value_To_String(iID, MC_ATT_SCAN_OPTIONS, sizeof m_scanOptions, m_scanOptions);
	MC_Get_Value_To_String(iID, MC_ATT_MANUFACTURER, sizeof m_manufacturer, m_manufacturer);

	// 
	// We can't just let pixel stuff fail the private data which
	// may or may not have an image it in. Extract what we need/can.
 
	if (HasAuxData(m_messageID))
	{
		bool fail = false;
		ExtractImageTypeTokens(m_imageTypeTokens, dummy);
		ExtractTransferSyntax(m_transferSyntax, m_isCompressed);
		MC_Get_Value_To_String(m_messageID, MC_ATT_SOP_CLASS_UID, sizeof(m_SOPClassUID), m_SOPClassUID);
		iRTVDeSpaceDe(m_SOPClassUID);
		// can't set the failure m_status for private data if there is no image
		if (ExtractPixelCellInfo(m_bitsAllocated, m_bitsStored, m_highBit, m_pixelRepresentation, false) != kNormalCompletion)
			fail = true;

		// If we don't do this here, modality gets "unkown" in the db
		if (DetermineImageStorageType () != kNormalCompletion) 
			fail = true;
		
		if (fail)
			return;
	}
	// END of 2004.04.06

	if ((m_status = ExtractPixelCellInfo(m_bitsAllocated, m_bitsStored, m_highBit, m_pixelRepresentation, m_msgLogoutInvalidUIDFlag == 1)) != kNormalCompletion)
																											//#91 2017/01/12 N.Furutsuki
		return;  
	if ((m_status = ExtractSamplesPerPixel(m_samplesPerPixel, m_planarConfiguration)) != kNormalCompletion) 
		return; 
	if ((m_status = ExtractPhotometricInterpretation (m_photometricInterpretation))   != kNormalCompletion) 
		return; 
	if ((m_status = ExtractImageDimensions(m_numberOfRows, m_numberOfColumns, m_numberOfFrames)) != kNormalCompletion) 
		return;
	if ((m_status = DetermineImageStorageType ()) != kNormalCompletion) 
		return;

	if (m_imageStorageType == kSCImage)
	{
		m_isSecondaryCapture = true;
	} else
	{
		m_isSecondaryCapture = false;
	}

	if (!m_isSecondaryCapture && m_imageStorageType != kCRImage &&m_imageStorageType != kUSImage )
	{
		if ((m_status = ExtractImageTypeTokens(m_imageTypeTokens, dummy)) != kNormalCompletion)
		{
			GetAqLogger()->LogMessage(kWarning,"WARNING: CPxDicomImage::PopulateFromMessage -  "
				"Failed to get MC_ATT_IMAGE_TYPE - %s.\n",MC_Error_Message((MC_STATUS)m_status));
//		 	return;
			//	2005.05.12 - If IMAGE_TYPE is missing, don't choke.  It may be a DICOM
			//		violation, but we only use this for sorting into sub-groups.  If it's not there,
			//		then it won't contribute to the sort.  No big deal.
		}
	}

	m_scaledW = m_numberOfColumns;
	m_scaledH = m_numberOfRows;

	m_bytesPerPixel = (m_bitsAllocated + 7) / 8;
//	m_bytesPerPixel = m_bitsAllocated / 8;
//	m_bytesPerPixel = ((m_bitsAllocated % 8) > 0) ? m_bytesPerPixel + 1 : m_bytesPerPixel;

	//	- 02/21/02
	m_bytesPerPixel *= m_samplesPerPixel;
	m_numberOfBytesOfPixelData = m_bytesPerPixel * m_numberOfRows * m_numberOfColumns * m_numberOfFrames;

#if 0
	if ((m_imageTypeTokens.size() > 0) && strcmp(m_imageTypeTokens[0].c_str(), "ORIGINAL") == 0)
	{
		m_isDerived = false;
	} else
	{
		m_isDerived = true;
	}
#endif

	/* 03/14/2003 --
	 * always try to extract modalityLUT. However for CT, modalityLUT is type 1, for other
	 * modality, it's not type 1.
	 */
	if ((m_status = ExtractSlopeIntercept(m_rescaleSlope, m_rescaleIntercept)) != kNormalCompletion) 
	{
		if (m_imageStorageType == kCTImage)
		{
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get slope intercept - %s.\n",MC_Error_Message((MC_STATUS)m_status));
			return;
		}
	}
 

	//	 11/05/02
//	if (m_imageStorageType == kCTImage || m_imageStorageType == kMRImage)
	{
		ExtractVOILut(m_windowCenter, m_windowWidth);
	}


	m_instanceNumber = 0;
	
	//	- add support for PET
	if (m_imageStorageType == kPTImage)
	{

		//  changed instanceNumber to int
	//	MC_Get_Value_To_UShortInt(iID, MC_ATT_IMAGE_INDEX, &m_instanceNumber); 
		MC_Get_Value_To_Int(iID, MC_ATT_IMAGE_INDEX, &m_instanceNumber); 


		//  PET FIX FOR DATASETS THAT DONT HAVE IMAGE INDEX
		// We need to do the same as we do in the SCU
		// If Image index is zero check to see if instance number as a value
		// If that is non zero take that.

		if (m_instanceNumber == 0)
		{
			// Notchecking for status ?? is that okay ? 
			// changed instanceNumber to int
			MC_Get_Value_To_Int(iID, MC_ATT_INSTANCE_NUMBER, &m_instanceNumber); 
		}
		
		// more stuff for SUV suppor. We don't care too much
		// if some of these fail as we can't derive SUV for all scans anyway.
		
		MC_Get_Value_To_String(iID, MC_ATT_UNITS, sizeof m_PETUnits, m_PETUnits);
		MC_Get_Value_To_Double(iID, MC_ATT_DECAY_FACTOR, &m_PETDecayFactor);
		MC_Get_Value_To_String(iID, MC_ATT_ACQUISITION_TIME, sizeof m_acquisitionTime, m_acquisitionTime);
		MC_Get_Value_To_Double(iID, MC_ATT_PATIENTS_WEIGHT, &m_patientWeight);
	
		// the following is in Radiopharmaceutial Info Sequence	
		int sqID = -1;	
		MC_Get_Value_To_Int(iID, MC_ATT_RADIOPHARMACEUTICAL_INFORMATION_SEQUENCE, &sqID);
		if (sqID >= 0)
		{
			MC_Get_Value_To_Int(sqID, MC_ATT_RADIONUCLIDE_HALF_LIFE, &m_radionuclideHalfLife);
			MC_Get_Value_To_String(sqID, MC_ATT_RADIOPHARMACEUTICAL_START_TIME, sizeof m_radioPharmaStartTime, m_radioPharmaStartTime);
			MC_Get_Value_To_Int(sqID, MC_ATT_RADIONUCLIDE_TOTAL_DOSE, &m_radionuclideTotalDose );
		}
		// End of 2006.06.10 Modification
	}
	else
	{
		// changed instanceNumber to int
		MC_Get_Value_To_Int(iID, MC_ATT_INSTANCE_NUMBER, &m_instanceNumber); 
	}

	// we don't want to fail right away but do want
	// to know about it as m_transferSyntax defaults to implicit little endian
	if (ExtractTransferSyntax(m_transferSyntax, m_isCompressed) != kNormalCompletion)
	{
//		assert(0);
	}

	//	
	m_isBiPlane = false;
	if (m_imageStorageType == kXAImage)
	{
		if ((m_status = ExtractReferencedSOPInstanceUID(m_isBiPlane)) != kNormalCompletion)
		{
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get ReferencedSOPInstanceUID - %s.\n",MC_Error_Message((MC_STATUS)m_status));
			return;
		}
	}

	// Moved here from below to look for pixel spacing for CR/DR/DX
	// Also now setting the status for only the modalities that absolutely need it.

	PxDicomStatus tStatus = kNormalCompletion;
	if ((tStatus = ExtractPixelSpacing(m_pixelSpacing)) != kNormalCompletion)
	{
		if (m_imageStorageType == kCTImage || 
			m_imageStorageType == kMRImage || 
			m_imageStorageType == kPTImage) 
			// || m_imageStorageType == kNMImage)	Type 2 for NM
		{
			m_status = tStatus;
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get pixel spacing - %s.\n",MC_Error_Message((MC_STATUS)m_status));
			return;
		}	
	}

	ExtractSliceThickness(m_sliceThickness);

	
	//m_status = ExtractPixelData(m_imagePixels);

	if (m_isSecondaryCapture || (m_imageStorageType != kCTImage && m_imageStorageType != kMRImage &&
								 m_imageStorageType != kPTImage && m_imageStorageType != kNMImage))
	{
		//	XA and SC (and CR?) do not have these tags - don't do rest of constructor
		return;
	}
	//	

//	if ((m_status = ExtractPixelSpacing(m_pixelSpacing)) != kNormalCompletion)
//		return;       

	//	 
	if ((m_status = ExtractImageOrientation(m_imageOrientation)) != kNormalCompletion)
	{
		if (m_imageStorageType == kNMImage)
			m_status = kNormalCompletion;
		else
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get patient image orientation - %s.\n",MC_Error_Message((MC_STATUS)m_status));
		return;       
	}

	if ((m_status = ExtractImagePosition(m_imagePosition)) != kNormalCompletion)
	{
		if (m_imageStorageType == kNMImage)
			m_status = kNormalCompletion;
		else
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::PopulateFromMessage - "
				"Failed to get patient image position - %s.\n",MC_Error_Message((MC_STATUS)m_status));
		return;       
	}

	// Get the cross product of the column and row to get the 
	// Scan Type

	double cross[3];
	cross[0] = (  m_imageOrientation[1]*m_imageOrientation[5] - m_imageOrientation[4]*m_imageOrientation[2]);
	cross[1] = (-(m_imageOrientation[0]*m_imageOrientation[5] - m_imageOrientation[3]*m_imageOrientation[2]));
	cross[2] = (  m_imageOrientation[0]*m_imageOrientation[4] - m_imageOrientation[3]*m_imageOrientation[1]);

	SetAxisOfAcquisition (cross);

	cross[0] = fabs(cross[0]);
	cross[1] = fabs(cross[1]);
	cross[2] = fabs(cross[2]);

	if ((cross[0] > cross[1]) && (cross[0] > cross [2]))
	{
		SetScanType (kSagittal);
	}
	else
	if ((cross[1] > cross[0]) && (cross[1] > cross [2]))
	{
		 SetScanType (kCoronal);
	}
	else
	if ((cross[2] > cross[0]) && (cross[2] > cross [1]))
	{
		SetScanType (kAxial);
	}
	else
	{
		SetScanType (kScanTypeUnknown);
	}

	char pp[8];
//		m_status, MC_Get_Value_To_String(iID, MC_ATT_PATIENT_POSITION,8,pp);
	ConvertPatientPosition(pp);

}	

//-----------------------------------------------------------------------------
//
void CPxDicomImage::GetPixelSpacing(double out[2])
{
    out[0] = m_pixelSpacing[0];
    out[1] = m_pixelSpacing[1];
}

//-----------------------------------------------------------------------------
//
void CPxDicomImage::SetPixelSpacing(double in[2])
{
    m_pixelSpacing[0] = in[0];
    m_pixelSpacing[1] = in[1];
}


//-----------------------------------------------------------------------------
//
void CPxDicomImage::GetImagePosition(double out[3])
{
    out[0] = m_imagePosition[0];
    out[1] = m_imagePosition[1];
    out[2] = m_imagePosition[2];
}

//-----------------------------------------------------------------------------
//
void CPxDicomImage::SetImagePosition(double in[3])
{
    m_imagePosition[0] = in[0];
    m_imagePosition[1] = in[1];
    m_imagePosition[2] = in[2];
}

//-----------------------------------------------------------------------------
//
void CPxDicomImage::GetAxisOfAcquisition(double out[3])
{
    out[0] = m_axisOfAcquisition[0];
    out[1] = m_axisOfAcquisition[1];
    out[2] = m_axisOfAcquisition[2];
}

//-----------------------------------------------------------------------------
//
void CPxDicomImage::SetAxisOfAcquisition(double in[3])
{
    m_axisOfAcquisition[0] = in[0];
    m_axisOfAcquisition[1] = in[1];
    m_axisOfAcquisition[2] = in[2];
}

//-----------------------------------------------------------------------------
//
// Set the Scan type
//
void CPxDicomImage::SetScanType (eDicomScanType inType)
{
    m_scanType = inType;
}


//-----------------------------------------------------------------------------
//
// Set the modality
//
void CPxDicomImage::SetModality (eDicomModality inModality)
{
	m_modality = inModality;
}

//-----------------------------------------------------------------------------
//
// This is for STL 
//
bool CPxDicomImage::operator < (const CPxDicomImage& inImg) 
{
    if (m_scanType == kScanTypeUnknown)
    {
        // This is the same as a Axial Scan
        //
        return (m_imagePosition[2] > inImg.m_imagePosition[2]);
    }
    else
        return (m_imagePosition[m_scanType] > inImg.m_imagePosition[m_scanType]);

}

//-----------------------------------------------------------------------------
//
// This is for STL 
//
bool CPxDicomImage::operator == (const CPxDicomImage& inImg) 
{
 //   if (m_scanType == kScanTypeUnknown)
 //   {
        // This is the same as a Axial Scan
        //
        return (m_imagePosition[2] == inImg.m_imagePosition[2]);
 //   }
 //   else
 //       return (m_imagePosition[m_scanType] == inImg.m_imagePosition[m_scanType]);
}

//-----------------------------------------------------------------------------
//
void CPxDicomImage::ReleaseImagePixels()
{
	TRCSLock L(m_cs);
	int size = m_imagePixels.size();
	if (size <= 0) return;
	for(int i=0; i<size; i++)
	{
		if(m_imagePixels[i])
		{
			//	 
			if (m_OBOWbuffer == m_imagePixels[i])
				m_OBOWbuffer = 0;

			delete [] m_imagePixels[i], m_imagePixels[i]=0;
		}
	}

	m_imagePixels.clear();
}

//-----------------------------------------------------------------------------
//
unsigned char* CPxDicomImage::GetImagePixels(void)
{ 
	return GetFrame(0); // this type image has only one frame
}

//-----------------------------------------------------------------------------
//
unsigned char* CPxDicomImage::GetFrame(int iFrameNumber)
{
	PxDicomStatus status;
	unsigned char* fb;

	if (iFrameNumber < 0 || iFrameNumber >= m_numberOfFrames)
	{
		return 0;
	}

	//	 
	//		created using CPxDicomMessage::Load(), merge has the pixel data; go get it.
	if (m_imagePixels.size() == 0)
	{
		status = ExtractPixelData(m_imagePixels);
		if (status != kNormalCompletion)
		{
			return 0;
		}
	}
	
	TRCSLock L(m_cs);
	if(iFrameNumber >= m_imagePixels.size()) return 0;

	fb = m_imagePixels[iFrameNumber];

	return fb;
}

//-----------------------------------------------------------------------------
//
int CPxDicomImage::AddFrame(unsigned char* inPixels) 
{
	TRCSLock L(m_cs);
	int frameNumber = m_imagePixels.size();
	m_imagePixels.push_back(inPixels);
	return frameNumber;
}

//-----------------------------------------------------------------------------
//
int CPxDicomImage::RleaseFrame(unsigned char* inPixels)
{
	if (inPixels == 0) return -1;

	TRCSLock L(m_cs);
	
	for(int i=0; i<m_imagePixels.size(); i++)
	{
		if(m_imagePixels[i] == inPixels)
		{
			//	 
			if (m_OBOWbuffer == m_imagePixels[i])
				m_OBOWbuffer = 0;

			delete [] m_imagePixels[i], m_imagePixels[i]=0;
			return i;
		}
	}
	return -1;
}

static std::map<std::string, int> gSOPClassMap;
static std::map<std::string, int> gServiceMap;

//-----------------------------------------------------------------------------
//
void CPxDicomImage::PopulateSOPClassMap()
{
	//	Map SOP Service Names to enum
	gServiceMap["STANDARD_CT"]					= kCTImage;
	gServiceMap["STANDARD_CR"]					= kCRImage;
	gServiceMap["STANDARD_DX_PRESENT"]			= kDXImage;
	gServiceMap["STANDARD_DX_PROCESS"]			= kDXImage;
	gServiceMap["STANDARD_XRAY_ANGIO"]			= kXAImage;
	gServiceMap["STANDARD_XRAY_RF"]				= kXARFImage;
	gServiceMap["STANDARD_XRAY_ANGIO_BIPLANE"]	= kXABPImage;
	gServiceMap["STANDARD_MR"]					= kMRImage;
	gServiceMap["STANDARD_NM"]					= kNMImage;
	gServiceMap["STANDARD_PET"]					= kPTImage;
	gServiceMap["STANDARD_US"]					= kUSImage;
	gServiceMap["STANDARD_US_RETIRED"]			= kUSImage;
	gServiceMap["STANDARD_US_MF"]				= kUSMFImage;
	gServiceMap["STANDARD_US_MF_RETIRED"]		= kUSMFImage;
	gServiceMap["STANDARD_SEC_CAPTURE"]			= kSCImage;

	//	Get SOP Class UID's from Merge (based on service name) and
	//		map them to enum as well
	char uid[kVR_UI];
	std::map<std::string, int>::iterator iter;
	for(iter = gServiceMap.begin(); iter != gServiceMap.end(); iter++)
	{
		MC_Get_UID_From_MergeCOM_Service(iter->first.c_str(), uid, sizeof(uid));
		gSOPClassMap[uid] = iter->second;
	}
}

//-----------------------------------------------------------------------------
//
std::string CPxDicomImage::ConvertModalityFromEnumToString(int iModality)
{
	switch(iModality)
	{
	case kCT:
		return "CT";
		break;
	case kMR:
		return "MR";
		break;
	case kXA:
		return "XA";
		break;
	case kXF:
		return "XF";
		break;
	case kRF:
		return "RF";
		break;
	case kCR:
		return "CR";
		break;
	case kDR:
		return "DR";
		break;
	case kUS:
		return "US";
		break;
	case kDX:
		return "DX";
		break;
	case kNM:
		return "NM";
		break;
	case kPT:
		return "PT";
		break;
	case kPX:  //K.Ko 2010.03/01  http://xtserv/trac/aqviewer/ticket/622
		return "PX";
		break;
	};

	return "";
}

static std::map<std::string, int> gModalityMap;

//-----------------------------------------------------------------------------
//
void CPxDicomImage::PopulateModalityMap()
{
	gModalityMap["CT"] = kCT;
	gModalityMap["MR"] = kMR;
	gModalityMap["XA"] = kXA;
	gModalityMap["XF"] = kXF;
	gModalityMap["RF"] = kRF;
	gModalityMap["CR"] = kCR;
	gModalityMap["DR"] = kDR;
	gModalityMap["US"] = kUS;
	gModalityMap["DX"] = kDX;
	gModalityMap["NM"] = kNM;
	gModalityMap["PT"] = kPT;
	//
	gModalityMap["PX"] = kPX; //K.Ko 2010.03/01  http://xtserv/trac/aqviewer/ticket/622
}

//-----------------------------------------------------------------------------
//
int CPxDicomImage::ConvertModalityFromStringToEnum(const char* iModality)
{
	if (gModalityMap.size() < 11)
		CPxDicomImage::PopulateModalityMap();

	std::map<std::string, int>::iterator iter = gModalityMap.find(iModality);
	if (iter != gModalityMap.end())
		return iter->second;

	return kUnsupportedModality;
}

//-----------------------------------------------------------------------------
//
int CPxDicomImage::RleaseFrame(int index)
{
	if(index < 0) return -1; 

	TRCSLock L(m_cs);
	int size = m_imagePixels.size();
	if (index >= size) return -1;
	
	if(m_imagePixels[index])
	{
		//	 2004.11.24 - So we don't try to delete the same pointer twice
		if (m_OBOWbuffer == m_imagePixels[index])
			m_OBOWbuffer = 0;

		delete [] m_imagePixels[index], m_imagePixels[index]=0;
		return index;
	}
	return -1;
}

unsigned short CPxDicomImage::GetNumberOfFrameBuffers() 
{ 
	unsigned short n;

	TRCSLock L(m_cs);
	n = m_imagePixels.size(); 
	return n;
}

long int CPxDicomImage::GetFrameSizeInBytes()
{
	return m_numberOfRows * m_numberOfColumns * m_bytesPerPixel; // * m_samplesPerPixel;
}

//-----------------------------------------------------------------------------
// Code Review Changes
// As of 02/27/02 we need to correctly handle the following storage classes
//		SOP Class Name							SOP Class UID
//
//		Computed Radiography					1.2.840.10008.5.1.4.1.1.1
//		CT Image								1.2.840.10008.5.1.4.1.1.2
//		MR Image								1.2.840.10008.5.1.4.1.1.4
//		Secondary Capture Image					1.2.840.10008.5.1.4.1.1.7
//		X-Ray Angiographic Image				1.2.840.10008.5.1.4.1.1.12.1
//		X-Ray Radiofluoroscopic Image			1.2.840.10008.5.1.4.1.1.12.2
//		X-Ray Angiographic Bi-Plane Image		1.2.840.10008.5.1.4.1.1.12.3
PxDicomStatus CPxDicomImage::DetermineImageStorageType ()
{
	PxDicomStatus status;

	char mod[17];
    status = (PxDicomStatus) MC_Get_Value_To_String(m_messageID,MC_ATT_MODALITY,16,mod); 
    if (status != kNormalCompletion)
    {
		//	04/16/03 - If SOPClass is Secondary Capture, then Modality is type 3.  Don't return.
		MC_Get_Value_To_String(m_messageID, MC_ATT_SOP_CLASS_UID, sizeof(m_SOPClassUID), m_SOPClassUID);
		if (strcmp (m_SOPClassUID, "1.2.840.10008.5.1.4.1.1.7") == 0)
		{
			strcpy(mod, "SC");
			m_modality = kSC;
		}
		else
		{
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::DetermineImageStorageType. "
				"Failed to get MC_ATT_MODALITY - %s.\n",MC_Error_Message((MC_STATUS)status));
			return status;
		}
		iRTVDeSpaceDe(m_SOPClassUID);
    }
          
	m_modalityStr = mod;

	m_modality = (eDicomModality) CPxDicomImage::ConvertModalityFromStringToEnum(mod);

	status = (PxDicomStatus) MC_Get_Value_To_String(m_messageID, MC_ATT_SOP_CLASS_UID, sizeof(m_SOPClassUID), m_SOPClassUID);
	if (status != kNormalCompletion)
	{
		GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::DetermineImageStorageType. "
			"Failed to get MC_ATT_SOP_CLASS_UID - %s.\n",MC_Error_Message((MC_STATUS)status));
		return status;
	}
	iRTVDeSpaceDe(m_SOPClassUID);

	m_imageStorageType = kUnknownImage;

	if (gSOPClassMap.size() < 15)
	{
		PopulateSOPClassMap();
	}

	std::map<std::string, int>::iterator iter = gSOPClassMap.find(m_SOPClassUID);
	if (iter != gSOPClassMap.end())
		m_imageStorageType = iter->second;

	if (m_imageStorageType == kSCImage)
	{
		// Here try to figure out if it does belong to any other nodality
		// by checking the modality tag.
		//
		// Rob 08/28/03 Fix for 24-bit CR/DR SC
		if (m_modality == kCR && m_samplesPerPixel != 3)
		{
			m_imageStorageType = kCRImage;
		}

		return kNormalCompletion;
	}


	return kNormalCompletion;

}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractImageTypeTokens (std::vector <std::string>& oTokens, int& oNumber)
{
	PxDicomStatus status;

	int numberOfImageTypeTokens = 0;
	oTokens.clear ();


	//	How many tokens are there?
	status = (PxDicomStatus) MC_Get_Value_Count(m_messageID, MC_ATT_IMAGE_TYPE, &numberOfImageTypeTokens);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	} 
	else 
	if (status == MC_NORMAL_COMPLETION)
	{
		//	Get the tokens

		// 
		//		where Image Type fields are encoded using '/' as separator instead of '\'
		char imageType[512];				
		char* token;
		oNumber = numberOfImageTypeTokens;

		for(int tok = 0;tok < oNumber; tok++)
		{
			//	Get the first token
			if (tok == 0)
			{
				status = (PxDicomStatus) MC_Get_Value_To_String(m_messageID, MC_ATT_IMAGE_TYPE, sizeof(imageType), imageType);
				if (status != MC_NORMAL_COMPLETION && status != MC_NULL_VALUE)
				{
					return status;
				}

				//	Look for improperly encoded Image Type based on known issue with Imatron C-300
				char* slashPos = strchr(imageType, '/');

				//	This one has the problem - we need to decode it differently
				if (slashPos != NULL)
				{
					token = imageType;
					do
					{
						*slashPos = '\0';					// Mark the end of the token
						oTokens.push_back(token);			// add it to the output vector
						token = slashPos + 1;				// advance to the next token
						slashPos = strchr(token, '/');		// search for the next separator
					} while (slashPos != NULL); 

					//	add the last token to the output vector
					oTokens.push_back(token);
					return kNormalCompletion;
				}
			} 
			else
			{
				status = (PxDicomStatus) MC_Get_Next_Value_To_String(m_messageID, MC_ATT_IMAGE_TYPE, sizeof(imageType), imageType);
				if (status != MC_NORMAL_COMPLETION && status != MC_NULL_VALUE)
				{
					return status;
				}
			}

			//	Add token to the vector
			token = imageType;
			oTokens.push_back(token);
		}
	}

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractSliceThickness(double& oThickness)
{
	PxDicomStatus status;

	double thickness = oThickness = 0.;
	status = (PxDicomStatus) MC_Get_Value_To_Double     (m_messageID, MC_ATT_SLICE_THICKNESS, &thickness);
	if (status != MC_NORMAL_COMPLETION)
		return status;

	oThickness = thickness;

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractPixelSpacing    (double oSpacing[2])
{
	PxDicomStatus status;

	double spacing[2];
	status = (PxDicomStatus) MC_Get_Value_To_Double     (m_messageID, MC_ATT_PIXEL_SPACING, &spacing[0]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_PIXEL_SPACING, &spacing[1]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	oSpacing[0] = spacing[0];
	oSpacing[1] = spacing[1];

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractPixelAspectRatio    (double & oAspectRatio) const
{
	PxDicomStatus status;

	int ratio[2];
	status = (PxDicomStatus) MC_Get_Value_To_Int(m_messageID, MC_ATT_PIXEL_ASPECT_RATIO, &ratio[0]);
	if ( status == MC_INVALID_TAG ) // if the attribute is not present
	{
		oAspectRatio = 1.0;
		return kNormalCompletion;
	}
	
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Int(m_messageID, MC_ATT_PIXEL_ASPECT_RATIO, &ratio[1]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	oAspectRatio = ratio[0] * 1.0 / ratio[1];
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractImageOrientation (double oOrientation[6])
{
	PxDicomStatus status;

	double orientation[6];

	status = (PxDicomStatus) MC_Get_Value_To_Double     (m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[0]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[1]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[2]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[3]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[4]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[5]);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	oOrientation[0] = orientation[0];
	oOrientation[1] = orientation[1];
	oOrientation[2] = orientation[2];
	oOrientation[3] = orientation[3];
	oOrientation[4] = orientation[4];
	oOrientation[5] = orientation[5];


	m_hasValidImageOrientation = true;

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
// 
// The image position and image orientation for NM is of type 2C. Can't
// reject the data. We may need to refine this later.
static inline int OKFor2CTags(int status)
{
	return status == MC_NORMAL_COMPLETION || status == kNullValue || status == kNoMoreValues;
}

//-----------------------------------------------------------------------------
PxDicomStatus CPxDicomImage::ExtractImagePosition     (double oPosition[3])
{
	PxDicomStatus status;

	double position[3] = { 0.0, 0.0, 0.0 };
	status = (PxDicomStatus) MC_Get_Value_To_Double     (m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,    &position[0]);
	if (!OKFor2CTags(status))
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,    &position[1]);
	if (!OKFor2CTags(status))
	{
		return status;
	}

	status = (PxDicomStatus) MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,    &position[2]);
	if (!OKFor2CTags(status))
	{
		return status;
	}

	oPosition[0] = position[0];
	oPosition[1] = position[1];
	oPosition[2] = position[2];

	m_hasValidImagePosition = true;

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
//	11/5/02
//
PxDicomStatus CPxDicomImage::ExtractVOILut (double& oWindowCenter, double& oWindowWidth)
{
	MC_STATUS status;

	double windowCenter = 0;
	double windowWidth  = 0;

	status = MC_Get_Value_To_Double(m_messageID, MC_ATT_WINDOW_CENTER, &windowCenter);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;
	}

	status = MC_Get_Value_To_Double(m_messageID, MC_ATT_WINDOW_WIDTH, &windowWidth);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;
	}

	oWindowCenter = windowCenter;
	oWindowWidth  = windowWidth;

	return (PxDicomStatus) MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractSlopeIntercept (double& oSlope, double& oIntercept)
{
	PxDicomStatus status;

	double slope = 1.;
	double intercept = 0.;


	status = (PxDicomStatus) MC_Get_Value_To_Double(m_messageID, MC_ATT_RESCALE_INTERCEPT, &intercept);
	if (status != MC_NORMAL_COMPLETION)
		intercept = (double) 0.0;

	status = (PxDicomStatus) MC_Get_Value_To_Double(m_messageID, MC_ATT_RESCALE_SLOPE, &slope);
	if (status != MC_NORMAL_COMPLETION)
		slope = (double) 1.0;

	oSlope     = slope;
	oIntercept = intercept;

	return kNormalCompletion;
}

//--------------------------------------------------------------------
// Get the table values -- this is the only way ?
// 
//
struct TableData
{
	TableData(char *D, int len, int offset=0)
	{
		m_data = D;
		m_length = len;
		m_offset = offset;
	}

	char *	m_data;
	int		m_length;
	int		m_offset;
};

//----------------------------------------------------------------------
MC_STATUS GetLutTableData(int messageID,unsigned long tag,void* userInfo, 
					   int dataSize,void* dataBufferPtr,int isFirst,int isLast)
{

	TableData *table = (TableData *)userInfo;

	assert(table && table->m_data);
	MC_STATUS status = MC_NORMAL_COMPLETION;

	if (dataSize > 0 && dataBufferPtr)
	{
		if (table->m_offset + dataSize <= table->m_length)
		{
		   memcpy(table->m_data + table->m_offset, dataBufferPtr, dataSize);
		}
		else
		{
			assert(table->m_offset + dataSize <= table->m_length);
			status = MC_CALLBACK_CANNOT_COMPLY;
		}
	}

	return status;

}

//--------------------------------------------------------------------------------
// 
PxDicomStatus CPxDicomImage::ExtractPalette(PxDicomPalette& ioP)
{
	static unsigned int desc[] = 
	{
		kVLIRedPaletteColorLookupTableDescriptor,kVLIGreenPaletteColorLookupTableDescriptor,kVLIBluePaletteColorLookupTableDescriptor,
		/* sentinel */
		0
	};
	static unsigned int dataTag[] = 
	{
		kVLIRedPaletteColorLookupTableData, kVLIGreenPaletteColorLookupTableData, kVLIBluePaletteColorLookupTableData,
		/* sentinel */
		0
	};


	PxDicomStatus status = kNormalCompletion;

	int entryCount[4], mapmin[4], precision[4], i;
	
	for ( i = 0; desc[i] && status == kNormalCompletion; ++i)
	{
		status = GetValue(desc[i],&entryCount[i]);
		if (status == kNormalCompletion)
			status = GetNextValue(desc[i],&mapmin[i]);
		if (status == kNormalCompletion)
			GetNextValue(desc[i],&precision[i]);
		if (status == kNormalCompletion)
			ioP.SetProperty(i, entryCount[0], mapmin[i], precision[i]);
	}
	
	if (status != kNormalCompletion)
	{
		GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractPalette. "
			"Failed to ExtractPalette - %s.\n",MC_Error_Message((MC_STATUS)status));
		return status;
	}

	if (!ioP.OK())
		return kCouldNotAllocateMemory;

	for ( i = 0; dataTag[i] && status == kNormalCompletion; ++i)
	{
		
//		status = (PxDicomStatus)MC_Get_Value_To_Buffer(m_messageID,dataTag[i],
//			     ioP.GetDataLength(i), ioP.GetData(i), &ret );
		TableData td((char *)ioP.GetData(i),ioP.GetDataLength(i),0);
		status = (PxDicomStatus)MC_Get_Value_To_Function(m_messageID,  dataTag[i], &td, GetLutTableData);
	}

	return status;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractPhotometricInterpretation (int& oPhotometricInterpretation)
{
	PxDicomStatus status;

	char photInterp[17];

    status = (PxDicomStatus) MC_Get_Value_To_String(m_messageID, MC_ATT_PHOTOMETRIC_INTERPRETATION, sizeof(photInterp), photInterp); 
    if (status != MC_NORMAL_COMPLETION)
    {
		GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractPhotometricInterpretation - "
			"Failed to get MC_ATT_PHOTOMETRIC_INTERPRETATION - %s.\n",MC_Error_Message((MC_STATUS)status));
       return status;
    }

	if (!strcmp(photInterp, "MONOCHROME1"))
	{
		oPhotometricInterpretation = kMONOCHROME1;
	} 
	else if (!strcmp(photInterp, "MONOCHROME2"))
	{
		oPhotometricInterpretation = kMONOCHROME2;
	} 
	else if (!strcmp(photInterp, "PALETTE COLOR"))
	{
		oPhotometricInterpretation = kPALETTE_COLOR;
		// -- 2004.04.15
		// if we know we're dealing with PALETTE_COLOR, extract the lookup table
		// right here.
		m_palette = new PxDicomPalette;
		if (m_palette)
		{
			status = ExtractPalette(*m_palette);
			m_palette->ConvertPrecision(8);
		}
		else
		{
			status = kCouldNotAllocateMemory; // memory
		}
	} 
	else if (!strcmp(photInterp, "RGB"))
	{
		oPhotometricInterpretation = kRGB;
	} 
	else if (!strcmp(photInterp, "YBR_FULL"))
	{
		oPhotometricInterpretation = kYBR_FULL;
	} 

	//  handle YBR images
	// According to DICOM standard, we need to handle _422 differently
	// but somehow handling it using kBYR_FULL works. We only have
	// one dataset (US-67) to test
	else if (!strcmp(photInterp, "YBR_FULL_422"))
	{
		oPhotometricInterpretation = kYBR_FULL;
	}
	else
	{
		oPhotometricInterpretation = kUnsupportedFormat;
	} 

	return status;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractSamplesPerPixel (unsigned short& oSamplesPerPixel, unsigned short& oPlanarConfiguration)
{
	PxDicomStatus status;

    status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID, MC_ATT_SAMPLES_PER_PIXEL, &oSamplesPerPixel); 
    if (status != MC_NORMAL_COMPLETION || oSamplesPerPixel < 0 || oSamplesPerPixel > 255)
    {
		GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractSamplesPerPixel - "
			"Failed to get MC_ATT_SAMPLES_PER_PIXEL - %s.\n",MC_Error_Message((MC_STATUS)status));
		return status;
    }

	if (oSamplesPerPixel > 1)
	{
		status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID, MC_ATT_PLANAR_CONFIGURATION, &oPlanarConfiguration); 
		if (status != MC_NORMAL_COMPLETION || oPlanarConfiguration < -32767 || oPlanarConfiguration > 32767)
		{
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractSamplesPerPixel - "
				"Failed to get MC_ATT_PLANAR_CONFIGURATION - %s.\n",MC_Error_Message((MC_STATUS)status));
			return status;
		}
	}

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractPixelCellInfo (unsigned short& oBitsAllocated, 
													unsigned short& oBitsStored, 
													unsigned short& oHighBit, 
													unsigned short& oPixelRepresentation,
													bool ibLogging)
{
	PxDicomStatus status;

	status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_BITS_ALLOCATED,&oBitsAllocated); 
    if (status != kNormalCompletion || oBitsAllocated < 0 || oBitsAllocated > 255)
    {
		if ( ibLogging )
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractPixelCellInfo - "
				"Failed to get MC_ATT_BITS_ALLOCATED - %s.\n",MC_Error_Message((MC_STATUS)status));
        return status;
    }
          
    status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_BITS_STORED,&oBitsStored); 
    if (status != kNormalCompletion || oBitsStored < 0 || oBitsStored > 255)
    {
		if ( ibLogging )
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractPixelCellInfo - "
				"Failed to get MC_ATT_BITS_STORED - %s.\n",MC_Error_Message((MC_STATUS)status));
        return status;
    }
          
    status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_HIGH_BIT,&oHighBit); 
    if (status != kNormalCompletion || oHighBit < 0 || oHighBit > 255)
    {
		if ( ibLogging )
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractPixelCellInfo - "
				"Failed to get MC_ATT_HIGH_BIT - %s.\n",MC_Error_Message((MC_STATUS)status));
        return status;
    }

	status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_PIXEL_REPRESENTATION,&oPixelRepresentation);
    if (status != kNormalCompletion || oPixelRepresentation < 0 || oPixelRepresentation > 255)
    {
		if ( ibLogging )
			GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractPixelCellInfo - "
				"Failed to get MC_ATT_PIXEL_REPRESENTATION - %s.\n",MC_Error_Message((MC_STATUS)status));
        return status;
    }
          
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractImageDimensions(unsigned short& oNumberOfRows, unsigned short& oNumberOfColumns, unsigned short& oNumberOfFrames)
{
	PxDicomStatus status;

	status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_ROWS,&m_numberOfRows); 
	m_scaledH = m_numberOfRows;
    if (status != kNormalCompletion || m_numberOfRows < -32767 || m_numberOfRows > 32767)
    {
		GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractImageDimensions - "
			"Failed to get MC_ATT_ROWS - %s.\n",MC_Error_Message((MC_STATUS)status));
        return status;
    }

          
    status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_COLUMNS,&m_numberOfColumns); 
	m_scaledW = m_numberOfColumns;
    if (status != kNormalCompletion || m_numberOfColumns < -32767 || m_numberOfColumns > 32767)
    {
		GetAqLogger()->LogMessage(kErrorOnly,"ERROR: CPxDicomImage::ExtractImageDimensions - "
			"Failed to get MC_ATT_COLUMNS - %s.\n",MC_Error_Message((MC_STATUS)status));
        return status;
    }
          
    status = (PxDicomStatus) MC_Get_Value_To_UShortInt(m_messageID,MC_ATT_NUMBER_OF_FRAMES,&m_numberOfFrames); 
    if (status != kNormalCompletion || m_numberOfFrames < 0 || m_numberOfFrames > 32767)
    {
        m_numberOfFrames = 1;
    }

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractTransferSyntax(int& oTransferSyntax, bool& oIsCompressed)
{
	PxDicomStatus status;

	char syntaxUID[65];
 	TRANSFER_SYNTAX syntax;

	oIsCompressed = false;
	m_isLittleEndian = true;
	oTransferSyntax = IMPLICIT_LITTLE_ENDIAN;

	status = (PxDicomStatus) MC_Get_Message_Transfer_Syntax(m_messageID, &syntax);
	if (status != kNormalCompletion)
	{
		status = (PxDicomStatus) MC_Get_Value_To_String(m_messageID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
		if (status != kNormalCompletion)
		{
			return status;  
		}

		status = (PxDicomStatus) MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != kNormalCompletion)
		{
			return status;  
		}
	}
	oTransferSyntax = (int) syntax;
	
	if (syntax == IMPLICIT_LITTLE_ENDIAN || syntax == EXPLICIT_BIG_ENDIAN ||
		syntax == EXPLICIT_LITTLE_ENDIAN || syntax == IMPLICIT_BIG_ENDIAN)
	{
		oIsCompressed = false;

		//	 
		if (syntax == IMPLICIT_LITTLE_ENDIAN || syntax == EXPLICIT_LITTLE_ENDIAN)
		{
			m_isLittleEndian = true;
		} else
		{
			m_isLittleEndian = false;
		}
	} else
	{
		oIsCompressed = true;
	}

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
//
PxDicomStatus CPxDicomImage::ExtractReferencedSOPInstanceUID(bool& oIsBiPlane)
{
	if (m_imageTypeTokens.size() >= 3)
	{
		if (!strcmp(m_imageTypeTokens[2].c_str(), "SINGLE PLANE"))
		{
			oIsBiPlane = false;
		} else if (!strcmp(m_imageTypeTokens[2].c_str(), "BIPLANE A") || !strcmp(m_imageTypeTokens[2].c_str(), "BIPLANE B"))
		{
			oIsBiPlane = true;
		}

		//	Get the item number for the Referenced Image Sequence so we can figure out where to find the other plane
		if (oIsBiPlane)
		{
			int refItemNumber;
			PxDicomStatus status = (PxDicomStatus) MC_Get_Value_To_Int(m_messageID, MC_ATT_REFERENCED_IMAGE_SEQUENCE, &refItemNumber);
			if (status != kNormalCompletion)
			{
				return status;
			} else
			{
				//	Parse the sequence item to get the Referenced SOP Instance UID
				status = (PxDicomStatus) MC_Get_Value_To_String(refItemNumber, MC_ATT_REFERENCED_SOP_INSTANCE_UID , 
					sizeof(m_referencedSOPInstanceUID), m_referencedSOPInstanceUID);
				if (status != kNormalCompletion)
				{
					return status;
				}
				iRTVDeSpaceDe(m_referencedSOPInstanceUID);
			}
		} 
		else
		{
			m_referencedSOPInstanceUID[0] = 0;
		}
	} 

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
MC_STATUS ImageGetPixelData(int messageID,unsigned long tag,void* userInfo, 
					   int dataSize,void* dataBufferPtr,int isFirst,int isLast)
{
	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	CPxDicomImage* pImage = (CPxDicomImage*) userInfo;
	return (MC_STATUS)pImage->HandoverPixelData(dataSize, dataBufferPtr, isFirst, isLast);
}

//-----------------------------------------------------------------------------------------
//
int CPxDicomImage::HandoverPixelData(int dataSize,void* dataBufferPtr,int isFirst,int isLast)
{
	if (isFirst)
	{
		//
		//	Get the filesize
		//
		if (m_OBOWlength < 1)
		{
			return MC_CALLBACK_CANNOT_COMPLY;
		}
		//
		//	Allocate the memory
		//
		if(m_OBOWbuffer)
			delete[] m_OBOWbuffer;
		m_OBOWbuffer = new unsigned char[m_OBOWlength + 2];
		if (m_OBOWbuffer == 0)
		{
			m_OBOWlength = 0;
			return MC_CALLBACK_CANNOT_COMPLY;
		} 
		m_OBOWoffset = 0;
	}

	if (dataSize > 0)
	{
		assert(m_OBOWoffset + dataSize <= m_OBOWlength);
		if (m_OBOWoffset + dataSize > m_OBOWlength)
		{
			return MC_CALLBACK_CANNOT_COMPLY;
		}

		memcpy((m_OBOWbuffer+m_OBOWoffset), dataBufferPtr, dataSize);
		m_OBOWoffset += dataSize;
	
	}

	return (MC_STATUS) kNormalCompletion;

}
//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::ExtractPixelData(std::vector<unsigned char*>& oImagePixels)
{
	MC_STATUS status;

	int vc = -1;
	status = MC_Get_Value_Count(m_messageID, MC_ATT_PIXEL_DATA, &vc);

	if (vc < 1 || status != MC_NORMAL_COMPLETION)
		return kNormalCompletion;

	status = MC_Get_Value_Length(m_messageID, MC_ATT_PIXEL_DATA, 1, &m_OBOWlength);
	if (status != MC_NORMAL_COMPLETION)
		return (PxDicomStatus) status;

	status = MC_Get_Value_To_Function(m_messageID, MC_ATT_PIXEL_DATA, this, ImageGetPixelData);
	if (status != MC_NORMAL_COMPLETION)
		return (PxDicomStatus) status;

	oImagePixels.push_back(m_OBOWbuffer), m_OBOWbuffer = 0;
	return kNormalCompletion;
}



//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::RemoveZeroPixelPadding()
{	
	MC_STATUS status;

	int paddingValue;
	status = MC_Get_Value_To_Int(m_messageID, MC_ATT_PIXEL_PADDING_VALUE, &paddingValue);
	if (status != MC_NORMAL_COMPLETION)
		return (PxDicomStatus) status;

	unsigned char* pixels = 0;

	int i,j;
	for(i = 0; i < m_imagePixels.size(); i++)
	{
		pixels = GetFrame(i);
		if (!pixels)
			continue;

		for(j = 0; j < GetFrameSizeInBytes(); j++)
		{
			if (pixels[j] == paddingValue)
				pixels[j] = 0;
		}
	}

	return kNormalCompletion;
}


//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::Save(const char* iSavePath, const char* iLocalAE)
{
	// 
	CBinfo cbinfo;
	int status;

#if 1 //K.Ko
	//	Duplicate the message object so that adding the file metaheaders to it doesn't
	//		screw up the original message
	int newMsgID = -1;
	status = MC_Duplicate_Message(m_messageID, &newMsgID, (TRANSFER_SYNTAX) m_transferSyntax, 0, 0);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;  
	}
#else
	int newMsgID = m_messageID;
#endif
	CPxDicomMessage msg(newMsgID);
	MessageFreeGuard msgGuard(newMsgID);

	long frameSize = GetFrameSizeInBytes();

	unsigned char* pixels = 0;
	iRTVSAlloc<unsigned char> pixelGuard(frameSize * m_numberOfFrames);
	pixels = pixelGuard;

	if (m_imagePixels.size() > 0)
	{
		//	Whatever pixel data are in the message already - delete it so we can
		//	replace it with our own
		status = MC_Delete_Attribute(newMsgID, MC_ATT_PIXEL_DATA);

		int i;
		for (i=0; i<m_imagePixels.size(); i++)
		{
			memcpy(pixels + (i * frameSize), GetFrame(i), frameSize);
		}

		status = (MC_STATUS) msg.SetValue(kVLIPixelData, pixels, frameSize * m_numberOfFrames);
		if (status != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) status;
		}
	}

	//	Convert to a DICOM Part 10 file object
	int fileID = msg.GetID();
	status = MC_Message_To_File(fileID,  iSavePath);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;  
	}

	//	The message guard needs to know we did that so it can free it as a file
	msgGuard.SetFileType(true);

	status = TRDICOMUtil::AddGroup2Elements(fileID, (TRANSFER_SYNTAX) m_transferSyntax, iLocalAE);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;
	}
	
	//	
	//	So the callback can check if it's passed too much data
	status = MC_Get_File_Length(fileID, &cbinfo.dataSize);
	if (status != MC_NORMAL_COMPLETION)
		cbinfo.dataSize = 0;

 

	//	Write the file to disk
	status = MC_Write_File(fileID, 0, &cbinfo, AqFileObjToMedia);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;
	}

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus  CPxDicomImage::ConvertToFile(const char* iFilePath, const char* iLocalAE)
{
	//
	//	Write the image to a file
	//
	PxDicomStatus status;
	if( m_filePath.size()<1) // it is not file ID, convert it
	{
		//	Convert to a DICOM Part 10 file object
		status = (PxDicomStatus) MC_Message_To_File(m_messageID,  iFilePath);
		if (status != MC_NORMAL_COMPLETION)
			return status;

		//	Moved to here so MC_Message_To_File() is not called on retry (because it will fail)
		m_filePath = iFilePath;
	}

	status  = WriteFile(iFilePath, iLocalAE);
	if (status != kNormalCompletion)
		return status;

	return kNormalCompletion;
}

//GL it is not multi-thread safe, beacause LoadHeader is not

PxDicomStatus CPxDicomImage::GetSortInfo(const char* iFilePath, DICOMData& oData)
{
	unsigned long pixSize, pixOffset;
	Reset();

	//load DICOM file without pixel, but pixSize, pixOffset
	PxDicomStatus status = kNormalCompletion;

	status = LoadHeader(iFilePath, pixOffset, pixSize);

	if (status != kNormalCompletion)
		return status;

	PopulateFromMessage(m_messageID);
	m_filePath = iFilePath;

	status = FillSortInfo(oData, false);
	if (status != kNormalCompletion)
		return status;

	oData.m_pixelOffset = pixOffset;
	oData.m_dataSize = pixSize;
	
	return kNormalCompletion;


}

// this function only work with file ID after the file saved or read in
PxDicomStatus  CPxDicomImage::FillSortInfo(DICOMData& oData, bool bFillPixelInfo)
{
	oData.Clear();

	// Following code is taken from CStore.cpp
	// It also adds additional information which
	// is not in CStore.cpp
	int idx = 0;
	int stat[100];

	//	Study Level tags
	ASTRNCPY(oData.m_studyInstanceUID, m_studyInstanceUID);
	stat[idx++] = GetValue(m_messageID, MC_ATT_PATIENTS_NAME, oData.m_patientsName, kVR_PN);
	stat[idx++] = GetValue(m_messageID, MC_ATT_REFERRING_PHYSICIANS_NAME, oData.m_referringPhysiciansName, kVR_PN);
	stat[idx++] = GetValue(m_messageID, MC_ATT_NAME_OF_PHYSICIANS_READING_STUDY, oData.m_radiologistName, kVR_PN);

	stat[idx++] = GetValue(m_messageID, MC_ATT_SPECIFIC_CHARACTER_SET, oData.m_characterSet, sizeof(oData.m_characterSet) - 1);//#138 2021/02/12 N.Furutsuki
	//#59
	//#137 2021/01/12 N.Furutsuki ref: GetValue

	stat[idx++] = GetValue(m_messageID, MC_ATT_STUDY_DESCRIPTION, oData.m_studyDescription, kVR_LO);
	stat[idx++] = GetValue(m_messageID, MC_ATT_SERIES_DESCRIPTION, oData.m_seriesDescription, kVR_LO);

	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENT_ID, kVR_LO, oData.m_patientID);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENTS_BIRTH_DATE, kVR_DA, oData.m_patientsBirthDate);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENTS_SEX, kVR_CS, oData.m_patientsSex);

	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_DATE, kVR_DA, oData.m_studyDate);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_TIME, kVR_TM, oData.m_studyTime);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_ACCESSION_NUMBER, kVR_SH, oData.m_accessionNumber);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_ID, kVR_SH,	oData.m_studyID);

	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_MODALITIES_IN_STUDY, kVR_CS, oData.m_modalitiesInStudy);
	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_NUMBER_OF_STUDY_RELATED_SERIES, &oData.m_numberOfStudyRelatedSeries);
	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_NUMBER_OF_STUDY_RELATED_INSTANCES, &oData.m_numberOfStudyRelatedInstances);
	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_PATIENTS_AGE, &oData.m_patientsAge);
	
	//	Series Level tags
	ASTRNCPY(oData.m_seriesInstanceUID, m_seriesInstanceUID);
	ASTRNCPY(oData.m_modality, GetModalityStr().c_str());

	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_SERIES_NUMBER, &oData.m_seriesNumber);
	oData.m_transferSyntax = m_transferSyntax;
	// series date and time
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_DATE, kVR_DA,	oData.m_seriesDate);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_TIME, kVR_TM,	oData.m_seriesTime);
 

	//  added BodyPartExamined
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_BODY_PART_EXAMINED, sizeof oData.m_bodyPartExamined, oData.m_bodyPartExamined);

	oData.m_imageOrientation[0] =  m_imageOrientation[0];
	oData.m_imageOrientation[1] =  m_imageOrientation[1];
	oData.m_imageOrientation[2] =  m_imageOrientation[2];
	oData.m_imageOrientation[3] =  m_imageOrientation[3];
	oData.m_imageOrientation[4] =  m_imageOrientation[4];
	oData.m_imageOrientation[5] =  m_imageOrientation[5];

	//	Instance Level tags
	oData.m_instanceNumber = m_instanceNumber;

	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENT_ORIENTATION, kVR_IS, oData.m_patientOrientation);

	ASTRNCPY(oData.m_SOPInstanceUID, m_SOPInstanceUID);
	ASTRNCPY(oData.m_SOPClassUID, m_SOPClassUID);

	oData.m_rows = m_numberOfRows;
	oData.m_columns = m_numberOfColumns;
	oData.m_bitsAllocated = m_bitsAllocated;
	oData.m_bitsStored = m_bitsStored;
	oData.m_pixelRepresentation = m_pixelRepresentation;
	oData.m_numberOfFrames = m_numberOfFrames;
	
	//	Image Type
 	std::string imageTypeString = "";
	for(int i=0; i<m_imageTypeTokens.size(); i++)
	{
		if (i > 0)
		{
			imageTypeString += "\\";
		}
		imageTypeString += m_imageTypeTokens[i];
	}
	ASTRNCPY(oData.m_imageTypeTokens, imageTypeString.c_str());

	//	ImagePosition - but only if it's MR or CT and not SC
	//  Different from CStore.
	//  In CStore, only CT, MR having image position.
	oData.m_imagePosition[0] = m_imagePosition[0];
	oData.m_imagePosition[1] = m_imagePosition[1];
	oData.m_imagePosition[2] = m_imagePosition[2];

	// additional information not present in CStore
	oData.m_highBit = m_highBit;
	oData.m_photometricInterpretation = m_photometricInterpretation;
	oData.m_planarConfiguration = m_planarConfiguration;
	oData.m_windowWidth = m_windowWidth;
	oData.m_windowCenter = m_windowCenter;
	oData.m_smallestPixelValue = GetSmallestPixelValue();
	oData.m_largestPixelValue = GetLargestPixelValue();
	oData.m_samplesPerPixel = m_samplesPerPixel;

	oData.m_pixelSpacing[0] = m_pixelSpacing[0];
	oData.m_pixelSpacing[1] = m_pixelSpacing[1];

	double aspectRatio;
	ExtractPixelAspectRatio(aspectRatio);
	oData.m_aspectRatio = aspectRatio;
	oData.m_rescaleSlope = m_rescaleSlope;
	oData.m_rescaleIntercept = m_rescaleIntercept;


	// other information
	memset(oData.m_viewPosition, 0, sizeof(oData.m_viewPosition));
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_VIEW_POSITION, 
		sizeof(oData.m_viewPosition), oData.m_viewPosition);

	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, 
		MC_ATT_NUMBER_OF_SERIES_RELATED_INSTANCES, 
		&oData.m_numberOfSeriesRelatedInstances);

	unsigned short imagePosition = 0;
	stat[idx++] = MC_Get_Value_To_UShortInt(m_messageID, MC_ATT_IMAGE_POSITION, &imagePosition);
	oData.m_slicePosition = imagePosition;
		
	double sliceThickness = 0;
	stat[idx++] = MC_Get_Value_To_Double(m_messageID, MC_ATT_SLICE_THICKNESS, &sliceThickness);
	oData.m_sliceThickness = sliceThickness;
	
	if(m_filePath != "" && bFillPixelInfo)
	{
		unsigned long pixSize=0, pixOffset=0;
		stat[idx++] = TRDICOMUtil::GetPixelDataFileOffset(m_messageID, m_filePath.c_str(), 
			(TRANSFER_SYNTAX) m_transferSyntax, pixSize, pixOffset);
		oData.m_pixelOffset = pixOffset;
		oData.m_dataSize = pixSize;
	}

	ASTRNCPY(oData.m_referencedSOPInstanceUID, m_referencedSOPInstanceUID);

	char lossyCmpr[17];
	stat[idx++] = GetValue(MC_ATT_LOSSY_IMAGE_COMPRESSION, lossyCmpr, kVR_CS);
	oData.m_wasLossyCompressed = (!strcmp(lossyCmpr, "01")) ? 1 : 0;
	stat[idx++] = GetValue(MC_ATT_CONTENT_DATE, oData.m_imageDate, kVR_DA);
	stat[idx++] = GetValue(MC_ATT_CONTENT_TIME, oData.m_imageTime, kVR_TM);

	stat[idx++] = GetValue(MC_ATT_SCAN_OPTIONS, oData.m_scanOptions, kVR_CS);
	stat[idx++] = GetValue(MC_ATT_MANUFACTURER, oData.m_manufacturer, 
										sizeof(oData.m_manufacturer) );


	// save level can be changed to only instance level
	// if study and sereis already exist
	//oData.m_saveLevel = kStudyLevel|kSeriesLevel|kInstanceLevel;

	// following fields are not filled here
	//oData.m_stationName
	//oData.m_offlineFlag;
	//oData.m_IsQRData;
	//oData.m_seriesModifyTime;
	//oData.m_readStatus;
	return kNormalCompletion;
}


/*****************************************************************************
 * Some palette related functions
 * 
 *****************************************************************************/

void PxDicomPalette::SetProperty(int iIndex, int iCount, int iMin, int iPrec)
{
	if (iIndex < 0 || iIndex > kMaxComp)
		return;

	if (iCount <= 0 ) 
	{
		return;
	}

	if (!(m_entry[iIndex] = new unsigned short[iCount]))
	{
		m_status = -1;
		return;
	}

	m_entryCount[iIndex]	= iCount;
	m_mapmin[iIndex]		= iMin;
	m_precision[iIndex]		= iPrec;
}

//---------------------------------------------------------------------------------
int	PxDicomPalette::ConvertPrecision(int toBits)
{
	if (toBits != 8 && toBits != 16)
		return -1;
	
	for (int j = 0; j < kMaxComp; j++)
	{	
		if (toBits == m_precision[j])
			continue;
		
		float factor = ((1<<toBits) - 1)/float((1<<m_precision[j]) -  1);		
		unsigned short *data = m_entry[j];
		
		for ( int i = m_entryCount[j]; --i >= 0; )
		{
			data[i] = unsigned short(data[i] * factor + 0.5f);
		}
		
		m_precision[j] = toBits;
	}

	return 0;
}

//---------------------------------------------------------------------------------
int	PxDicomPalette::Lookup(int iPixels, unsigned short& oR, unsigned short& oG, unsigned short&oB)
{
    int N;

	if (!OK()) 
		return -1;

	unsigned short rgb[4];

	for ( int i = 0; i < 3; ++i)
	{
		N = iRTVSClamp(iPixels - m_mapmin[i], 0, m_entryCount[i] - 1);
		rgb[i] = m_entry[i][N];
	}

	oR = rgb[0];
	oG = rgb[1];
	oB = rgb[2];

	return 0;
}

//---------------------------------------------------------------------------------
int	PxDicomPalette::Lookup(int iPixels, unsigned char& oR, unsigned char& oG, unsigned char&oB)
{
	if (!OK()) 
		return -1;

	unsigned short rgb[4];

	Lookup(iPixels, rgb[0],rgb[1],rgb[2]);

	if (m_precision[0] > 8)
		oR = unsigned char(rgb[0] * 255.1f/((1<<m_precision[0])-1));
	else
		oR = unsigned char(rgb[0]);

	if (m_precision[0] > 8)
		oG = unsigned char(rgb[1] * 255.1f/((1<<m_precision[1])-1));
	else
		oG = unsigned char(rgb[1]);

	if (m_precision[0] > 8)
		oB = unsigned char(rgb[2] * 255.1f/((1<<m_precision[2])-1));
	else
		oB = unsigned char(rgb[2]);

	return 0;
}

//----------------------------------------------------------------------------------
void	PxDicomPalette::SetData(int iIndex, unsigned char*  iData,	int iStart, int iNum)
{


}

//---------------------------------------------------------------------------------
void	PxDicomPalette::SetData(int iIndex, unsigned short* iData,	int iStart, int iNum)
{

}
