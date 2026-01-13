/***********************************************************************
 * RTVSliceInformation.cpp
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon, Inc 2001, All rights reserved.
 *
 *	PURPOSE:
 *		This file implements the member functions of the 
 *	    TerareconCache Object.
 *
 *	AUTHOR(S):  Vikram Simha September 2001
 *   
 *-------------------------------------------------------------------
 */


//-----------------------------------------------------------------------------
//                          iRTVSliceInformation Members
#pragma warning (disable: 4786)

#if !defined(RTVSLICEINFORMATION_H)
#include "RTVSliceInformation.h"
#endif

#include "math.h"

#include < assert.h>

#if !defined(TRMODELMATRIXGENERATOR_H)
#include "TRModelMatrixGenerator.h"
#endif

#if !defined(TICACHE_H)
#include "TiCache.h"
#endif

const int   kDefaultMinMaxVoxelValue = 99999;

// Vikram 08/30/04 - Adding the ability to configure image tolerance
double iRTVSliceInformation::m_imagePositionTolerance = 0.05;

//-----------------------------------------------------------------------------

iRTVSliceInformation::iRTVSliceInformation()
{
	Initialize();
}

iRTVSliceInformation::~iRTVSliceInformation () 
{
	m_imageTypeTokens.clear();
}

//-----------------------------------------------------------------------------

void iRTVSliceInformation::Initialize()
{
	memset (m_imagePosition,    0, sizeof (m_imagePosition));
	memset (m_imageOrientation, 0, sizeof (m_imageOrientation));
	memset (m_histogram,        0, sizeof (m_histogram));


	m_pixelSpacing[0] = 1.0;
	m_pixelSpacing[1] = 1.0;

	m_rescaleSlope     = 1.0;
	m_rescaleIntercept = 0.0;
	m_used1024 = 0;

	m_sizeX = 0;
	m_sizeY = 0;

	m_sopInstanceUID = "";
    m_sopClassUID = "";
	m_referencedSOPInstanceUID = "";
	m_modality = "CT";
	m_version  = "";

	m_numberOfEntriesInHistogram = 0;


	m_bitsAllocated = 8;
	m_bitsStored = 8;
	m_highBit = 7;

	m_startOfDataInDataFile = 0;
	m_sizeOfData = 0;
	m_startOfL0CompressedDataInDataFile = 0;
	m_sizeOfL0CompressedData = 0;
	m_imageTypeTokens.clear ();
    m_imageTypeTokensAsOneString = "";


	m_scanType = kScanTypeUnknown; 

	m_imageNumber = 0;

    m_uniqueIndex = 0;

    m_samplesPerPixel = 1;
    m_photometricInterpretation = kMONOCHROME1;
    m_planarConfiguration = kRGBRGB;
    m_sortInAscendingOrder = false;
    m_sliceNumber = -1;
	m_positonInAllSlicesVector = -1;

	m_pixelRepresentation = 1; // 2's complement
	m_sliceMinVoxelValue  =  kDefaultMinMaxVoxelValue;
	m_sliceMaxVoxelValue  = -kDefaultMinMaxVoxelValue;

	m_globalMinVoxelValue  =  kDefaultMinMaxVoxelValue;
	m_globalMaxVoxelValue  = -kDefaultMinMaxVoxelValue;

	m_compressionFactor = 1.0;
	m_inputL0CompressionSizeX = 512;
	m_inputL0CompressionSizeY = 512;

	m_voiWindowWidth  = 0.0;
	m_voiWindowCenter = 0.0;

	m_sVOIWindowWidth = 0;
	m_sVOIWindowCenter = 0;

	m_useModalityLUT = 0;

	m_axialPosition = 0;

	m_sequence = 0;

	m_hasPixelSpacing = 0;
	m_hasWindowLevel = 0;

	m_sliceThickness = 0.0f;
	m_sliceLocation	= 0.0f;
	m_hasSliceThickness = 0;

	// Vikram 07/22/04 - NM Support

	m_detector				= 0;
	m_rotation				= 0;
	m_energyWindow			= 0;
	m_phase					= 0;
	m_rrInterval			= 0;
	m_timeSlot				= 0;
	m_sliceIndex			= 0;
	m_angularView			= 0;
	m_timeSlice				= 0;
	m_hasValidOrientation	= 1;
	m_isRotational			= 0;

	// T.C. Zhao 20056.05.06
	m_imageDate = "";
	m_imageTime = "";
	m_hasImageDateTime = 0;

	// Murali 2007.01.04
	m_scanOptions = "";
	m_manufacturer= ""; 

	// T.C. Zhao 2006.06.10 PET SUV
	m_PETAttrib.Reset();
}

//-----------------------------------------------------------------------------

void iRTVSliceInformation::MakeImageTypeTokensIntoOneString ()
{
    int i = 0;

    for (i = 0; i < m_imageTypeTokens.size(); i++)
    {
	    m_imageTypeTokensAsOneString += m_imageTypeTokens[i];
    }
}



//-----------------------------------------------------------------------------
// T.C. Zhao 2003-07-07
double iRTVSliceInformation::GetAxialPosition(void) const
{
	iRTVSliceInformation *me = const_cast<iRTVSliceInformation*>(this);

	if (m_axialPosition == 0.0)
	{
		double cross[3];
		cross[0] = (  m_imageOrientation[1]*m_imageOrientation[5] - m_imageOrientation[4]*m_imageOrientation[2]);
		cross[1] = (-(m_imageOrientation[0]*m_imageOrientation[5] - m_imageOrientation[3]*m_imageOrientation[2]));
		cross[2] = (  m_imageOrientation[0]*m_imageOrientation[4] - m_imageOrientation[3]*m_imageOrientation[1]);
		
		assert (fabs(m_imageOrientation[0]*m_imageOrientation[3] + 
			         m_imageOrientation[1]*m_imageOrientation[4] +
					 m_imageOrientation[2]*m_imageOrientation[5]) <= 0.001);




		me->m_axialPosition = cross[0] * m_imagePosition[0] +
							  cross[1] * m_imagePosition[1] +
							  cross[2] * m_imagePosition[2];
	}
	
	return m_axialPosition;
}

//-----------------------------------------------------------------------------
void iRTVSliceInformation::DetermineScanType()
{
    double cross[3];
		
	cross[0] = (  m_imageOrientation[1]*m_imageOrientation[5] - m_imageOrientation[4]*m_imageOrientation[2]);
	cross[1] = (-(m_imageOrientation[0]*m_imageOrientation[5] - m_imageOrientation[3]*m_imageOrientation[2]));
	cross[2] = (  m_imageOrientation[0]*m_imageOrientation[4] - m_imageOrientation[3]*m_imageOrientation[1]);
    
    m_scanType = TRModelMatrixGenerator::GetScanType(cross);
}
//-----------------------------------------------------------------------------

bool iRTVPtrToSliceInformation::operator < (const iRTVPtrToSliceInformation& iRTVPSI)const
{
    // Changed the sorting order to match HFS
    // Vikram 03/29/02
    if (m_ptr->m_sortInAscendingOrder)
	    return (m_ptr->m_imagePosition[m_ptr->m_scanType] < iRTVPSI.m_ptr->m_imagePosition[m_ptr->m_scanType]);
    else
	    return (m_ptr->m_imagePosition[m_ptr->m_scanType] > iRTVPSI.m_ptr->m_imagePosition[m_ptr->m_scanType]);

}


//-----------------------------------------------------------------------------
// Need to rework this.
bool iRTVPtrToSliceInformation::operator == (const iRTVPtrToSliceInformation& iRTVPSI)const
{
	return (fabs(m_ptr->m_imagePosition[m_ptr->m_scanType] - iRTVPSI.m_ptr->m_imagePosition[m_ptr->m_scanType])) < 0.0001;
}
//-----------------------------------------------------------------------------
bool iRTVPtrToSliceInformation::SortByImageNumber (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
    return (iPSi0.m_ptr->m_imageNumber < iPSi1.m_ptr->m_imageNumber);
}

//-----------------------------------------------------------------------------
bool iRTVPtrToSliceInformation::SortByImageNumberSOP (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
	if(iPSi0.m_ptr->m_imageNumber == iPSi1.m_ptr->m_imageNumber)
		return (iPSi0.m_ptr->m_sopInstanceUID < iPSi1.m_ptr->m_sopInstanceUID);

    return (iPSi0.m_ptr->m_imageNumber < iPSi1.m_ptr->m_imageNumber);
}

//-----------------------------------------------------------------------------
// T.C. Zhao JULY-10-2003 In consolidation phase, we need to sort by
// sequence number
bool iRTVPtrToSliceInformation::SortBySequenceNumber (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
    return (iPSi0.m_ptr->m_sequence < iPSi1.m_ptr->m_sequence);
}

//-----------------------------------------------------------------------------
bool iRTVPtrToSliceInformation::SortByPositionInCache (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
    return (iPSi0.m_ptr->m_startOfDataInDataFile < iPSi1.m_ptr->m_startOfDataInDataFile);
}

//-----------------------------------------------------------------------------

bool iRTVPtrToSliceInformation::SortByImagePosition (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
	// This is for the stable_sort if scan_type is different do not change the position
	// Vikram & TC. 
	if (iPSi0.m_ptr->m_scanType != iPSi1.m_ptr->m_scanType)
	{
		return false;
	}
	else
		return (iPSi0.m_ptr->m_imagePosition[iPSi0.m_ptr->m_scanType] < iPSi1.m_ptr->m_imagePosition[iPSi1.m_ptr->m_scanType]);
}

//-----------------------------------------------------------------------------

bool iRTVPtrToSliceInformation::SortByScanOptions (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
	// Murali/ TC. 2006.10.20. Only if both the scanoption strings are equal, compare the position.
	int diff = strcmp(iPSi0.m_ptr->GetScanOptions(), iPSi1.m_ptr->GetScanOptions());
	if(diff)
	{
		return (diff < 0);
	}
	
	return (iPSi0.GetAxialPosition() < iPSi1.GetAxialPosition());	
}

//-----------------------------------------------------------------------------

bool iRTVPtrToSliceInformation::SortBySOPInstanceUID (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
	return (iPSi0.m_ptr->m_sopInstanceUID < iPSi1.m_ptr->m_sopInstanceUID);
}

//-----------------------------------------------------------------------------
bool iRTVPtrToSliceInformation::SortByScanType (const iRTVPtrToSliceInformation& iPSi0, const iRTVPtrToSliceInformation& iPSi1)
{
    return (iPSi0.m_ptr->m_scanType > iPSi1.m_ptr->m_scanType);
}
