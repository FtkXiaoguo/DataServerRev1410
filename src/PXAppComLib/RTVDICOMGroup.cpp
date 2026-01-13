/***********************************************************************
 * RTVDICOMGroup.cpp
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
//-----------------------------------------------------------------------------
#pragma warning (disable: 4786)

#include "RTVDICOMGroup.h"

#ifndef NVLIMATH_H_
//#include "nvlimath.h"
#endif

#if !defined(TRMODELMATRIXGENERATOR_H)
//#include "TRModelMatrixGenerator.h"
#endif


 
#include "AppComDataConversion.h"
 

//#include "AqPercentRToR.h"

#ifndef AWARE_ONLY
#include "j2kErr.h"
#include "j2kEncoder.h"
#endif

#include <algorithm>
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


// -- 2006.06.11
int  iRTVDICOMGroup::m_correctPET = 1;

//-----------------------------------------------------------------------------
iRTVDICOMGroup::iRTVDICOMGroup ()
{
	Initialize();
}


//-----------------------------------------------------------------------------

void iRTVDICOMGroup::Initialize ()
{
	m_ptrToAllSlices.clear ();
	m_imageTypeTokens.clear ();

	m_srcFile = 0;
	m_compressedSrcFile = 0;
	// Make the model matrix identity
	m_modelMatrix[0]  = 1.0;
	m_modelMatrix[1]  = 0.0;
	m_modelMatrix[2]  = 0.0;
	m_modelMatrix[3]  = 0.0;

	m_modelMatrix[4]  = 0.0;
	m_modelMatrix[5]  = 1.0;
	m_modelMatrix[6]  = 0.0;
	m_modelMatrix[7]  = 0.0;

	m_modelMatrix[8]  = 0.0;
	m_modelMatrix[9]  = 0.0;
	m_modelMatrix[10] = 1.0;
	m_modelMatrix[11] = 0.0;

	m_modelMatrix[12] = 0.0;
	m_modelMatrix[13] = 0.0;
	m_modelMatrix[14] = 0.0;
	m_modelMatrix[15] = 1.0;

	// Make the scan matrix identity

	m_scanMatrix[0]  = 1.0;
	m_scanMatrix[1]  = 0.0;
	m_scanMatrix[2]  = 0.0;
	m_scanMatrix[3]  = 0.0;

	m_scanMatrix[4]  = 0.0;
	m_scanMatrix[5]  = 1.0;
	m_scanMatrix[6]  = 0.0;
	m_scanMatrix[7]  = 0.0;

	m_scanMatrix[8]  = 0.0;
	m_scanMatrix[9]  = 0.0;
	m_scanMatrix[10] = 1.0;
	m_scanMatrix[11] = 0.0;

	m_scanMatrix[12] = 0.0;
	m_scanMatrix[13] = 0.0;
	m_scanMatrix[14] = 0.0;
	m_scanMatrix[15] = 1.0;

	m_xScale = m_yScale = m_zScale = 1.0f; // -- mar-01-02


	m_isUniformlySpaced = false;

	m_modality = "";
	m_sopClassUID = "";
	m_imageTypeTokensAsOneString = "";

	m_groupID = 0;

	m_sizeX = 0;
	m_sizeY = 0;

	m_bitsUsedPerPixel = 8;
	m_bitsAllocated = 8;
	m_bitsStored = 8;
	m_highBit = 7;

	m_bytesPerPixel = m_bytesPerSlice = 0;

	m_scanType = kScanTypeUnknown;
	m_planeType = "U";
	m_referencedGroupID = 0;

    m_samplesPerPixel = 1;
    m_photometricInterpretation = kMONOCHROME1;
    m_planarConfiguration = kRGBRGB;
    m_sortInAscendingOrder = false;

    m_ptrToAllSlices.clear ();
    m_ptrToWorkingSlices.clear ();
    m_currentSliceToLoad = 0;
	m_currentCompressedSliceToLoad = 0;
	m_spacing = 0.0;
	m_lastPosition = 0.0;

	m_planeNormal[0] = 0.0;
	m_planeNormal[1] = 0.0;
	m_planeNormal[2] = 0.0;
	m_planeD = 0.0;

	m_currentMinVoxelValue = 0;
	m_currentMaxVoxelValue = 4095;

	m_voiWindowWidth  = 0;
	m_voiWindowCenter = 0;

	m_slope = 1.0;
	m_offset = 0.0;

	m_rescaleSlope = 1.0;
	m_rescaleIntercept = 0.0;

	m_isConsolidated = false;

	// -- 2004.12.29 for PET SUV support
	m_PETProcessed = 0; 
	m_remapScale		= 1.0;
	m_remapIntercept	= 0.0;

	m_useGMLConversion = 0;

	m_sortCTByScanOptionsFirst = 0; // Murali 2007.01.03

}


//-----------------------------------------------------------------------------
// Clears the list in the group
void iRTVDICOMGroup::Clear ()
{
    m_ptrToAllSlices.clear ();
    m_ptrToWorkingSlices.clear ();
	m_eraseList.clear();
	m_currentMinVoxelValue = 0;
	m_currentMaxVoxelValue = 4095;
	m_PETProcessed = 0; 
	m_remapScale		= 1.0;
	m_remapIntercept	= 0.0;
	m_isConsolidated = false;
}

//-----------------------------------------------------------------------------

iRTVDICOMGroup::~iRTVDICOMGroup ()
{
	CloseDataFile();
	CloseCompressedDataFile();
	Clear();
}

//-----------------------------------------------------------------------------

void iRTVDICOMGroup::AddFirstToGroup(iRTVPtrToSliceInformation& iPSi, int iGroupID, eSortType iCurrentSortType)
{
	
#if 0
    m_currentSortType = iCurrentSortType;
    m_referencedGroupID = m_groupID = iGroupID;

	m_modality		= iPSi.m_ptr->m_modality;
    m_sopClassUID   = iPSi.m_ptr->m_sopClassUID;
    m_bitsAllocated = iPSi.m_ptr->m_bitsAllocated;
	m_bitsStored	= iPSi.m_ptr->m_bitsStored;
	m_highBit		= iPSi.m_ptr->m_highBit;
	m_sizeX			= iPSi.m_ptr->m_sizeX;
	m_sizeY			= iPSi.m_ptr->m_sizeY;

	m_voiWindowWidth  = iPSi.m_ptr->m_voiWindowWidth;
	m_voiWindowCenter = iPSi.m_ptr->m_voiWindowCenter;

	m_bitsUsedPerPixel = m_bitsStored;

	m_sopInstanceUID			= iPSi.m_ptr->m_sopInstanceUID;
	m_referencedSOPInstanceUID	= iPSi.m_ptr->m_referencedSOPInstanceUID;	
	m_sopInstanceUID			= iPSi.m_ptr->m_sopInstanceUID;
	m_samplesPerPixel           = iPSi.m_ptr->m_samplesPerPixel;
    m_photometricInterpretation = iPSi.m_ptr->m_photometricInterpretation;
    m_planarConfiguration       = iPSi.m_ptr->m_planarConfiguration;

	// This is hard coded fro right now
	// Vikram 11/11/01
	if (m_bitsUsedPerPixel > 12)
		m_bitsUsedPerPixel = 12;

	m_bytesPerPixel = ((m_bitsAllocated+7) / 8);
//	m_bytesPerSlice =  m_bytesPerPixel * m_sizeX * m_sizeY;

	m_imageTypeTokens = iPSi.m_ptr->m_imageTypeTokens;
	m_imageTypeTokensAsOneString = iPSi.m_ptr->m_imageTypeTokensAsOneString;

	nvli::VLIMatrix::Identity ().CopyTo (m_modelMatrix);
	nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);

	// Add the sop uid and referenced uid if its XA
	if ((m_modality.compare (kModalityXAStr) == 0) ||
		(m_modality.compare (kModalityXFStr) == 0) ||
		(m_modality.compare (kModalityRFStr) == 0) ||
		(m_modality.compare (kModalityCRStr) == 0) ||
		(m_modality.compare (kModalityDRStr) == 0) ||
		(m_modality.compare (kModalityDXStr) == 0) ||
		(m_modality.compare (kModalityOTStr) == 0))
	{
		;
	}
    else // SC
    if ((m_modality.compare (kModalitySCStr) == 0) || 
		 IsSOPClassUIDSC(m_sopClassUID))
    {
        ;
    }    
	else // US
    if (IsSOPClassUIDUSFamily(m_sopClassUID))
    {
        ;
     }
	//	TC & RL 07/29/03 - Rearrange logic (fix SC/XA problem)
	else
	{	
		// Calculate the plane of the slice
		m_planeNormal[0] = (  iPSi.m_ptr->m_imageOrientation[1]*iPSi.m_ptr->m_imageOrientation[5] - iPSi.m_ptr->m_imageOrientation[4]*iPSi.m_ptr->m_imageOrientation[2]);
		m_planeNormal[1] = (-(iPSi.m_ptr->m_imageOrientation[0]*iPSi.m_ptr->m_imageOrientation[5] - iPSi.m_ptr->m_imageOrientation[3]*iPSi.m_ptr->m_imageOrientation[2]));
		m_planeNormal[2] = (  iPSi.m_ptr->m_imageOrientation[0]*iPSi.m_ptr->m_imageOrientation[4] - iPSi.m_ptr->m_imageOrientation[3]*iPSi.m_ptr->m_imageOrientation[1]);
		
		double nLength = sqrt( m_planeNormal[0]*m_planeNormal[0] + m_planeNormal[1]*m_planeNormal[1] + m_planeNormal[2]*m_planeNormal[2] );
		
		// Normalize
		if (nLength > 0.01)
		{
			double OneOverLength = 1.0/nLength;

			m_planeNormal[0] *= OneOverLength;
			m_planeNormal[1] *= OneOverLength;
			m_planeNormal[2] *= OneOverLength;
		
		}



		m_planeD         = - m_planeNormal[0]*iPSi.m_ptr->m_imagePosition[0] 
						   - m_planeNormal[1]*iPSi.m_ptr->m_imagePosition[1] 
						   - m_planeNormal[2]*iPSi.m_ptr->m_imagePosition[2];
	
		double lModelMatrix[16];

		m_scanType = TRModelMatrixGenerator::GetModelMatrix (iPSi.m_ptr->m_imageOrientation, false,
                                                             lModelMatrix, m_sortInAscendingOrder);

		for (int i = 0; i < 16; i++)
		{
			// The scan matrix is the matrix without the scales in it.
			// At this point the Modelmatrix does not have scales in it
			m_scanMatrix[i] = m_modelMatrix[i] = (float)lModelMatrix[i];
		}

//       iPSi.m_ptr->m_scanType             = m_scanType;
        iPSi.m_ptr->m_sortInAscendingOrder = m_sortInAscendingOrder;


		// -- JULY-15-2003 For CT, we always use (1.0,1024)
		if (IsSOPClassUIDCT(m_sopClassUID))
		{
			m_slope = 1.0f;
			// client assumes HU.
			m_offset = 0.0; // 1024.0f;

			m_scanOptions = iPSi.m_ptr->m_scanOptions; 
		}

	}

	// Moved it here from above to accomodate 24-bit Data
    // Vikram
	int realSamples = m_samplesPerPixel + (m_samplesPerPixel == 3);
    m_bytesPerSlice =  realSamples * m_bytesPerPixel * m_sizeX * m_sizeY;

	m_ptrToAllSlices.push_back (iPSi);
#endif
}




//-----------------------------------------------------------------------------
// -- 2003-07-07
// remove a slice from a group
//
void iRTVDICOMGroup::AddToEraseList(int iSlice)
{
	int sz = m_ptrToAllSlices.size();
	
	if (iSlice==-1)
	{
		int i, n = m_ptrToAllSlices.size();
		for ( i = 0; i < n; ++i)
			m_eraseList.push_back(i);
		return;
	}
	
	if (iSlice >= sz)
	{
		assert(iSlice <= sz);
		return;
	}
	
	m_eraseList.push_back(iSlice);
}

//-----------------------------------------------------------------------------
void iRTVDICOMGroup::EraseGoner(void)
{
	// this must run from n-1 to 0
	for ( int n = m_eraseList.size(); --n >=0;)
		m_ptrToAllSlices.erase(m_ptrToAllSlices.begin() + m_eraseList[n]);

	m_eraseList.clear();
}

// END Of -- 2003-07-07


//-----------------------------------------------------------------------------
// -- 2003-07-09 need this for consolidation
bool iRTVDICOMGroup::AddToGroupAlmost2D(iRTVPtrToSliceInformation& iPSi)
{
	// changed by shiying, to be consistent with IsCompatible method.
	// one more check here is m_imageTypeTokensAsOneString
	if (!(m_imageTypeTokensAsOneString == iPSi.m_ptr->m_imageTypeTokensAsOneString) ||
					m_sizeX != iPSi.m_ptr->m_sizeX ||
					m_sizeY != iPSi.m_ptr->m_sizeY ||
					m_samplesPerPixel != iPSi.m_ptr->m_samplesPerPixel ||
					m_bitsAllocated != iPSi.m_ptr->m_bitsAllocated ||
					m_bitsStored != iPSi.m_ptr->m_bitsStored ||
					m_highBit != iPSi.m_ptr->m_highBit ||
					m_planarConfiguration != iPSi.m_ptr->m_planarConfiguration ||
					m_photometricInterpretation  != iPSi.m_ptr->m_photometricInterpretation)
	return false;

	m_ptrToAllSlices.push_back(iPSi);
	return true;
}


//-----------------------------------------------------------------------------
// This return false if the current slice being added should be in a new group.
//
bool iRTVDICOMGroup::AddToGroup(iRTVPtrToSliceInformation& iPSi, bool iIgnoreGroupSpacing)
{ 
//	std::string::size_type  idx;
//	idx = iPSi.m_ptr->m_imageTypeTokensAsOneString.find ("DERIVED");

    iPSi.m_ptr->m_sortInAscendingOrder = m_sortInAscendingOrder;

#if 0
	// --  JULY-09-2003 There are many legitimate and volumeable
	// derived series. Can't just toss all derived types into the same bin.
	// However, we don't attempt to make subseries with different types
	// of images
	if ((IsModalityCT(m_modality) || IsModalityMR(m_modality))  || IsModalityPT(m_modality) &&
		m_imageTypeTokensAsOneString == iPSi.m_ptr->m_imageTypeTokensAsOneString)
//	if ((IsModalityCT(m_modality) || IsModalityMR(m_modality)) && idx == std::string::npos)
	{

		// Vikram 02/20/03 If the x and y sizes do not match we cannot add to the same group
		//
		if (m_sizeX != iPSi.m_ptr->m_sizeX ||
			m_sizeY != iPSi.m_ptr->m_sizeY)
		{
			return false;
		}


		// Murali 2007.01.03 If the ScanOptions is specified, then it must be same for all the slices in the group
		if(m_scanOptions.length() && m_sortCTByScanOptionsFirst)
		{
			if(strncmp(m_scanOptions.data(), iPSi.m_ptr->GetScanOptions(), m_scanOptions.length ()) != 0)
			{
				return false;
			}
		}

		int sz = m_ptrToAllSlices.size();
		if (sz > 1)
		{

			double position = fabs(m_planeNormal[0]* iPSi.m_ptr->m_imagePosition[0] +
				                   m_planeNormal[1]* iPSi.m_ptr->m_imagePosition[1] +
						           m_planeNormal[2]* iPSi.m_ptr->m_imagePosition[2] + m_planeD);
		
			double spacing = position - m_lastPosition;

			// If the spacing is not conformant to this groups spacing
			// Then do not add the slice to this group
			if (!iIgnoreGroupSpacing)
			{

				if ( fabs(m_spacing - spacing) > iRTVSliceInformation::GetImagePostionTolerance())
				{
					return false;
				}
			}

			m_lastPosition = position;
		}	
		else
		{
			// Store the Initial Distance
			m_spacing = fabs(m_planeNormal[0]* iPSi.m_ptr->m_imagePosition[0] +
				             m_planeNormal[1]* iPSi.m_ptr->m_imagePosition[1] +
						     m_planeNormal[2]* iPSi.m_ptr->m_imagePosition[2] + m_planeD);

			m_lastPosition = m_spacing;
		}
	}

	m_ptrToAllSlices.push_back (iPSi);

#endif

	return true;
}

//-----------------------------------------------------------------------------

void iRTVDICOMGroup::SortAndProcess ()
{
#if 0
	if ((IsModalityCT(m_modality) || IsModalityMR(m_modality) || IsModalityPT(m_modality))) //&& 
		//m_currentSortType == kSortByImagePosition)
	{
		// Vikram 006/13/03 2D Rendering Fix
		if (m_currentSortType == kSortAs2DSlices)
			return;


		if (m_currentSortType == kSortByImageNumber)
		{
			// Make sure you sort the group with the right alogorithm
			if (m_ptrToAllSlices.size () > 1)
			{
				if (m_ptrToAllSlices[0].m_ptr->m_imageNumber != m_ptrToAllSlices[1].m_ptr->m_imageNumber)
				{
					// Sort the elements by image  number
					// as imagenumber is valid
					std::stable_sort (m_ptrToAllSlices.begin (), m_ptrToAllSlices.end (), iRTVPtrToSliceInformation::SortByImageNumber);
				}
				else
				{
					// Sort the elements by SOPInstanceUID
					std::stable_sort (m_ptrToAllSlices.begin (), m_ptrToAllSlices.end (), iRTVPtrToSliceInformation::SortBySOPInstanceUID);
				}
			}
		}
		else
		{
			std::stable_sort (m_ptrToAllSlices.begin (), m_ptrToAllSlices.end());
		}
	}

    // Add the sop uid and referenced uid if its XA
	if ((m_modality.compare (kModalityXAStr) == 0) ||
		(m_modality.compare (kModalityXFStr) == 0) ||
		(m_modality.compare (kModalityRFStr) == 0))
	{
		for (int i = 0; i < m_imageTypeTokens.size (); i++)
		{

			 if (m_imageTypeTokens[i].compare ("BIPLANE A") == 0)
			 {
				 m_planeType = "A";
				 break;
			 }
			 else
			 if (m_imageTypeTokens[i].compare ("BIPLANE B") == 0)
			 {
				 m_planeType = "B";
				 break;
			 }			
		}
	}
#endif

  //  CalculateScalingAndUniformity (m_ptrToAllSlices);

}

//-----------------------------------------------------------------------------
// -- 2003-03-18
double iRTVDICOMGroup::GetSlope(void)
{
	return m_ptrToAllSlices[0].m_ptr->m_rescaleSlope;
}

//-----------------------------------------------------------------------------
// This function figures out if this sub-series is a rotational subseries
//

bool iRTVDICOMGroup::IsSubSeriesRotational (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices)
{
	bool isRotational = false;

	if (iVectorOfSlices.size () > 1)
	{
#if _DEBUG
		double a[6], b[6];

		for (int i = 0; i < 6; i++)
		{
			a[i] = iVectorOfSlices[0].m_ptr->m_imageOrientation[i];
			b[i] = iVectorOfSlices[1].m_ptr->m_imageOrientation[i];
		}
#endif
		double fudge = 0.01;
		if ((fabs (iVectorOfSlices[0].m_ptr->m_imageOrientation[0] - iVectorOfSlices[1].m_ptr->m_imageOrientation[0]) > fudge) ||
			(fabs (iVectorOfSlices[0].m_ptr->m_imageOrientation[1] - iVectorOfSlices[1].m_ptr->m_imageOrientation[1]) > fudge) ||
			(fabs (iVectorOfSlices[0].m_ptr->m_imageOrientation[2] - iVectorOfSlices[1].m_ptr->m_imageOrientation[2]) > fudge) ||
			(fabs (iVectorOfSlices[0].m_ptr->m_imageOrientation[3] - iVectorOfSlices[1].m_ptr->m_imageOrientation[3]) > fudge) ||
			(fabs (iVectorOfSlices[0].m_ptr->m_imageOrientation[4] - iVectorOfSlices[1].m_ptr->m_imageOrientation[4]) > fudge) ||
			(fabs (iVectorOfSlices[0].m_ptr->m_imageOrientation[5] - iVectorOfSlices[1].m_ptr->m_imageOrientation[5]) > fudge))
		{
			isRotational = true;
		}
	}

	return isRotational;
}

//--------------------------------------------------------------
bool iRTVDICOMGroup::IsRotational(void)
{
	assert(m_ptrToAllSlices.size() > 0);
	return m_ptrToAllSlices[0].m_ptr->m_isRotational || IsSubSeriesRotational(this->m_ptrToAllSlices);
}

//--------------------------------------------------------------
// -- 2005.12.13
// Lots of PET data is missing orientation information, check it here
bool iRTVDICOMGroup::BadOrientation(void)
{
	if (m_ptrToAllSlices.size() == 0)
		return true;
	
#if 0
	nvli::VLIVector3D a,b;
	
//	for ( int N = m_ptrToAllSlices.size(); --N >=0;)
	int N = 0;
	{
		a.Assign(m_ptrToAllSlices[N].m_ptr->m_imageOrientation[0],
			m_ptrToAllSlices[N].m_ptr->m_imageOrientation[1],
			m_ptrToAllSlices[N].m_ptr->m_imageOrientation[2]);
		
		a.Normalize();
		
		b.Assign(m_ptrToAllSlices[N].m_ptr->m_imageOrientation[3],
			m_ptrToAllSlices[N].m_ptr->m_imageOrientation[4],
			m_ptrToAllSlices[N].m_ptr->m_imageOrientation[5]);
		
		b.Normalize();
		
		if (a.Length() < 0.1 || b.Length() < 0.1)
			return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
// Vikram 03/19/03 Figures out if a scan has rotation in it
// Vikram 08/15/2006 - Fix for Gantry Tilted Data
bool iRTVDICOMGroup::HasShear (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices, float& oShearCorrection )
{
	bool hasShear = false;
	
	// Vikram 08/15/2006 - Fix for Gantry Tilted Data
	oShearCorrection = 1.0;
	
#if 0
	// Vikram 04/22/03 MRI TILT (Sota-san Dataset) Fix
/*	if (m_currentSortType == kSortByImageNumber && m_modality.compare (kModalityMRStr) == 0)
	{
		return false;
	}
*/
	if (iVectorOfSlices.size () > 1)
	{
#if _DEBUG
		double a[6], b[6];

		for (int i = 0; i < 6; i++)
		{
			a[i] = iVectorOfSlices[0].m_ptr->m_imageOrientation[i];
			b[i] = iVectorOfSlices[1].m_ptr->m_imageOrientation[i];
		}
#endif
		double fudge = 0.01;
		
		int idx = iVectorOfSlices.size ()-1;
	
		nvli::VLIVector3D r, c, rxc, ipv;

		r.Assign (iVectorOfSlices[0].m_ptr->m_imageOrientation[0],
			      iVectorOfSlices[0].m_ptr->m_imageOrientation[1],
				  iVectorOfSlices[0].m_ptr->m_imageOrientation[2]);


		c.Assign (iVectorOfSlices[0].m_ptr->m_imageOrientation[3],
			      iVectorOfSlices[0].m_ptr->m_imageOrientation[4],
				  iVectorOfSlices[0].m_ptr->m_imageOrientation[5]);

		rxc = nvli::VLICross (r, c);

		// -- 
		// 2004.07.23 this can happen with some old cache that does not 
		// have image orientation
		if (rxc.Length() == 0)
		{
			return false;
		}

		rxc.Normalize ();

		ipv.Assign (iVectorOfSlices[idx].m_ptr->m_imagePosition[0] - iVectorOfSlices[0].m_ptr->m_imagePosition[0],
                    iVectorOfSlices[idx].m_ptr->m_imagePosition[1] - iVectorOfSlices[0].m_ptr->m_imagePosition[1],
					iVectorOfSlices[idx].m_ptr->m_imagePosition[2] - iVectorOfSlices[0].m_ptr->m_imagePosition[2]);

		if (ipv.Length () > 0.01)
		{
			ipv.Normalize ();

			// Vikram 08/15/2006 - Fix for Gantry Tilted Data
			float ipvDOTrxc = fabsf(nvli::VLIDot (ipv, rxc));
			

			// Vikram 08/15/2006 - Fix for Gantry Tilted Data
			if (fabs (ipvDOTrxc - 1) > 0.001)
			{
				// Vikram 08/15/2006 - Fix for Gantry Tilted Data
				oShearCorrection = ipvDOTrxc;
				hasShear = true;
			}
		}

	}
#endif
	return hasShear;
}

//-----------------------------------------------------------------------------
// -- & Shiying 2005.10.28
// check if we have shear or rotation. Basically for ROI, we can't
// handle both of these (90 degree rotation excepted)
bool iRTVDICOMGroup::HasShearOrRotation (void)
{
	if (m_ptrToWorkingSlices.size() == 0)
	{
		assert(0);
		return false;
	}
#if 0
	// Vikram 08/15/2006 - Fix for Gantry Tilted Data
	float shearCorrection = 1.0;
	if (HasShear(m_ptrToWorkingSlices, shearCorrection))
		return true;
	
	double strictFudge = 0.0001;
	nvli::VLIVector3D r, c;
	
	r.Assign (m_ptrToWorkingSlices[0].m_ptr->m_imageOrientation[0],
		m_ptrToWorkingSlices[0].m_ptr->m_imageOrientation[1],
		m_ptrToWorkingSlices[0].m_ptr->m_imageOrientation[2]);
	
	
	c.Assign (m_ptrToWorkingSlices[0].m_ptr->m_imageOrientation[3],
		m_ptrToWorkingSlices[0].m_ptr->m_imageOrientation[4],
		m_ptrToWorkingSlices[0].m_ptr->m_imageOrientation[5]);
	
	r.Normalize();
	c.Normalize();
	
	float rMax = fabs(r[0]) > fabs(r[1]) ? fabs(r[0]) : fabs(r[1]);
	rMax = rMax > fabs(r[2]) ? rMax : fabs(r[2]);
	
	float cMax = fabs(c[0]) > fabs(c[1]) ? fabs(c[0]) : fabs(c[1]);
	cMax = cMax > fabs(c[2]) ? cMax : fabs(c[2]);
	
	if ( fabs( rMax - 1) > strictFudge || fabs(cMax -1 ) > strictFudge)
		return true;
	
#endif
	return false;
}

//-----------------------------------------------------------------------------
// Vikram 03/19/03 This functions make sure that the scale on the axis of aquisition is
// right based on the image orientation and how the slices are actually loaded
//
void iRTVDICOMGroup::FixScaleOnLoadAxis (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices)
{

	if (iVectorOfSlices.size () > 1)
	{
#if _DEBUG
		double a[6], b[6];

		for (int i = 0; i < 6; i++)
		{
			a[i] = iVectorOfSlices[0].m_ptr->m_imageOrientation[i];
			b[i] = iVectorOfSlices[1].m_ptr->m_imageOrientation[i];
		}
#endif
		double fudge = 0.01;
		
#if 0
		nvli::VLIVector3D r, c, rxc, ipv;

		r.Assign (iVectorOfSlices[0].m_ptr->m_imageOrientation[0],
			      iVectorOfSlices[0].m_ptr->m_imageOrientation[1],
				  iVectorOfSlices[0].m_ptr->m_imageOrientation[2]);


		c.Assign (iVectorOfSlices[0].m_ptr->m_imageOrientation[3],
			      iVectorOfSlices[0].m_ptr->m_imageOrientation[4],
				  iVectorOfSlices[0].m_ptr->m_imageOrientation[5]);

		rxc = nvli::VLICross (r, c);
		rxc.Normalize ();

		int idx = iVectorOfSlices.size ()-1;
		ipv.Assign (iVectorOfSlices[idx].m_ptr->m_imagePosition[0] - iVectorOfSlices[0].m_ptr->m_imagePosition[0],
                    iVectorOfSlices[idx].m_ptr->m_imagePosition[1] - iVectorOfSlices[0].m_ptr->m_imagePosition[1],
					iVectorOfSlices[idx].m_ptr->m_imagePosition[2] - iVectorOfSlices[0].m_ptr->m_imagePosition[2]);

		if (ipv.Length () > 0.01)
		{
			ipv.Normalize ();

			float ipvDOTrxc = nvli::VLIDot (ipv, rxc);

			if (ipvDOTrxc < 0.0) // The scale needs to be flipped
			{
				nvli::VLIMatrix mm;
				nvli::VLIMatrix scale = nvli::VLIMatrix::Scale( 1.0, 1.0, -1.0);
				
				mm.Assign (m_modelMatrix);

				mm *= scale;
				
				mm.CopyTo (m_modelMatrix);
				mm.CopyTo (m_scanMatrix);

			}
		}
#endif

	}

}

//-----------------------------------------------------------------------------
// This Fuction takes out any non 90 degree rotation or shear that might be present in the matrix

int iRTVDICOMGroup::RemoveShearAndNon90DegreeRotation ()
{
	// Take out all the shear & non 90 degree rotation if present so it renders okay in 2D
	double in[3], out[3];
	in[0] = m_modelMatrix[0];
	in[1] = m_modelMatrix[1];
	in[2] = m_modelMatrix[2];
	
 
	out[0] = out[1] = out[2] = 0.0;
//	TRModelMatrixGenerator::GetAxisMostParallelToVector (in, out);
	
	m_modelMatrix[0] = out[0];
	m_modelMatrix[1] = out[1];
	m_modelMatrix[2] = out[2];
	
	
	
	in[0] = m_modelMatrix[4];
	in[1] = m_modelMatrix[5];
	in[2] = m_modelMatrix[6];
	
//	TRModelMatrixGenerator::GetAxisMostParallelToVector (in, out);
	
	m_modelMatrix[4] = out[0];
	m_modelMatrix[5] = out[1];
	m_modelMatrix[6] = out[2];
	
	
	in[0] = m_modelMatrix[8];
	in[1] = m_modelMatrix[9];
	in[2] = m_modelMatrix[10];
	
//	TRModelMatrixGenerator::GetAxisMostParallelToVector (in, out);
	
	m_modelMatrix[8]  = out[0];
	m_modelMatrix[9]  = out[1];
	m_modelMatrix[10] = out[2];
	
	for (int l = 0; l < 16; l++)
	{
		// Copy the modelMatrix to the Scan Matrix
		m_scanMatrix[l] = m_modelMatrix[l];
	}

	return true;
}


//-----------------------------------------------------------------------------
// Changed this on 01/23/02 to take into account the min and max spacing
// Vikram
// Still need to reorganize into groups if non-uniformly spaced
//
// Vikram 08/15/2006 - Fix for Gantry Tilted Data
int iRTVDICOMGroup::CalculateScalingAndUniformity (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices, bool iHasShear, float iShearCorrection)
{
	if (iVectorOfSlices.size () == 0) return kErrCacheNotWritten;

#if 0
	double spacing    = 1.0;
	double minSpacing = 1.0;
	double maxSpacing = 1.0;
	double sumOfSpacings = 1.0;
	const double kFudge = 0.0001;

	m_isUniformlySpaced = 0;


	if (m_modality.compare (kModalityCTStr) == 0 || 
		m_modality.compare (kModalityMRStr) == 0 || 
		m_modality.compare (kModalityPTStr) == 0)	
	{
		// Vikram & Murali 12/27/2006
		// Solve the issue of single slice volumes sorted
		// by Image Position
		//
		if (iVectorOfSlices.size () > 1 || m_currentSortType == kSortByImagePosition)
		{
			minSpacing = 1.0;
			maxSpacing = 1.0;

			if (iVectorOfSlices.size () > 1)
			{
				// first pair
				spacing = iVectorOfSlices[1].GetAxialPosition () - iVectorOfSlices[0].GetAxialPosition ();
				
				minSpacing = spacing;
				maxSpacing = spacing;
			}

			// Initialize
			sumOfSpacings = spacing;
			
			m_isUniformlySpaced = 1;
			
			// The rest of the slices
			for (int i = 1; i < iVectorOfSlices.size()-1; i++)
			{			
				spacing = iVectorOfSlices[i+1].GetAxialPosition () - iVectorOfSlices[i].GetAxialPosition ();
				
				sumOfSpacings += spacing;
				minSpacing = (spacing < minSpacing) ? spacing : minSpacing;
				maxSpacing = (spacing > maxSpacing) ? spacing : maxSpacing;
				
			}
			
			// Vikram 08/15/2006 - Fix for Gantry Tilted Data
			if (iHasShear)
			{
				// Need to account for shear in CT/MR datasets
				sumOfSpacings /= iShearCorrection;
				minSpacing    /= iShearCorrection;
				maxSpacing    /= iShearCorrection;
			}

			
		}
		else
		{
			RemoveShearAndNon90DegreeRotation ();
		}
	}

	// -- 2007.05.24 fix according to DICOM standard
	double sx = iVectorOfSlices[0].m_ptr->m_pixelSpacing[1];
	double sy = iVectorOfSlices[0].m_ptr->m_pixelSpacing[0];
	double sz = 1.0;

	if ((m_modality.compare (kModalityCTStr) == 0 || 
		 m_modality.compare (kModalityMRStr) == 0 || m_modality.compare (kModalityPTStr) == 0) && iVectorOfSlices.size() > 1)	
	{
		// Fix for 4D single slice dynamic MR
	
		if (fabs (sumOfSpacings) > 0.001)
		{
			sz = sumOfSpacings / ((double) iVectorOfSlices.size()-1);
		}
		else
		{
			sz = 1.0;

			// Fix for 4D single slice dynamic MR
			// Need to take this out when we fix the model matrix stuff
			// Vikram 02/27/03
			if (m_currentSortType == kSortByImageNumber)
			{
				RemoveShearAndNon90DegreeRotation ();
				m_isUniformlySpaced = 0; // false
			}
		}
	}

	// Set the m_isUniformlySpaced flag based on the difference in min and max spacing
	// if the difference is greater than 10 %

	double diff = fabs(maxSpacing - minSpacing);
	
	// Murali 2006.12.19 :minSpacing/maxSpacing are checked against fudge to ensure that multiphasic data is handled properly.
	if (fabs(minSpacing) < kFudge || fabs(maxSpacing) < kFudge || diff > fabs(minSpacing * 0.1))
	{
		m_isUniformlySpaced = 0; // false
	}
 
	// Vikram 06/17/04 NM Support 
	if (m_modality.compare (kModalityNMStr) == 0)
	{
		sz = sx;

		m_isUniformlySpaced = 1;
	}

	if (m_currentSortType == kSortByImageNumber ) // && m_isUniformlySpaced)
	{
		// Needed to fix the problem with "every N" or sort by image number and
		// the dataset is uniform
		// Vikram & TC 02/19/03

		// Vikram 03/04/03
		// We really need to fix the scaling whenever we sort by image number as long
		// as the sub-series is not a rotational sub-series.
		// E.G. When we have the Body data set with one slice missing.
		//
		if (!IsSubSeriesRotational(iVectorOfSlices))
		{
			sz = fabs(sz);
			FixScaleOnLoadAxis (iVectorOfSlices);
		}
	}
	else
	{
		// We are not sorting in ascending order and we will keep the scale
		// always +ve 
		// Vikram 03/29/02

		sz = fabs(sz);
	}
#if 0
	nvli::VLIMatrix mm;
	nvli::VLIMatrix scale = nvli::VLIMatrix::Scale( sx, sy, sz);

	// BEGIN -- (mar-01-02)
	m_xScale = sx;
	m_yScale = sy;
	m_zScale = sz;
	// END --
	

	
	mm.Assign (m_modelMatrix);
    
    // Need the inverse of the model matrix to go to 
    // HFS Space
//	mm = mm.Inverse () * scale; // Old method WRONG !!
	mm = mm * scale;   // Vikram 03/19/03
	
	// Check to see if the modality is MR and the series is a rotatatonal series then
	// we need to just take the scales into account 
	// and we need to set the series as not uniform

	if (m_modality.compare (kModalityMRStr) == 0)
	{
		// Check to see if the series is a roational series
		if (IsSubSeriesRotational(iVectorOfSlices))
		{
			m_scanType = kAxial;
			m_zScale = 1.0;
			mm = nvli::VLIMatrix::Scale( sx, sy, 1.0);
			m_isUniformlySpaced = 0; //false

			// Make the scan matrix identity
			nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);
		}
		
	}
	
	mm.CopyTo(m_modelMatrix);

#endif

#endif

	return kSuccess;
}
//-----------------------------------------------------------------------------
int iRTVDICOMGroup::BuildModelMatrix (int iStart, int iEnd)
{
	int sliceNumber = 0;
	int start = iStart;
	int end   = iEnd;

#if 0
//	m_currentMinVoxelValue =  kDefaultMinMaxVoxelValue;
//	m_currentMaxVoxelValue = -kDefaultMinMaxVoxelValue;
	m_currentMinVoxelValue = m_ptrToAllSlices[iStart].m_ptr->m_sliceMinVoxelValue;
	m_currentMaxVoxelValue = m_ptrToAllSlices[iStart].m_ptr->m_sliceMaxVoxelValue;

	// Vikram 05/04/04 Fixing Linked Multi-Data load without cache
	m_ptrToWorkingSlices.clear ();
  
	if (IsSOPClassUIDCT(m_sopClassUID) || 
		IsSOPClassUIDMR(m_sopClassUID)  || 
		IsSOPClassUIDPT(m_sopClassUID) ||
		IsSOPClassUIDNM(m_sopClassUID) ||
		(m_sopClassUID == "" && (IsModalityCT(m_modality) || 
		                         IsModalityMR(m_modality) || 
								 IsModalityNM(m_modality) || 
								 IsModalityPT(m_modality))))
    {
       
 		int minVV = 0;
		int maxVV = 0;
		// Need to copy the slice over to make sure that we calculate the
        // right set of slices to load
		
		for (int i = start; i <= end; i++)
        {
			minVV = m_ptrToAllSlices[i].m_ptr->m_sliceMinVoxelValue;
			maxVV = m_ptrToAllSlices[i].m_ptr->m_sliceMaxVoxelValue;

			if (minVV < m_currentMinVoxelValue) m_currentMinVoxelValue = minVV;
			if (maxVV > m_currentMaxVoxelValue) m_currentMaxVoxelValue = maxVV;

            m_ptrToWorkingSlices.push_back (m_ptrToAllSlices[i]);
            m_ptrToWorkingSlices[sliceNumber].m_ptr->m_sliceNumber = sliceNumber;
			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_positonInAllSlicesVector = i;
            sliceNumber++;
        }


       double lModelMatrix[16];

		// Vikram 03/19/03 
		// Here we need to determine if the scan shear in it or a rotation in it
		bool	hasShear = false;
		float	shearCorrection = 1.0;

		// That is we atleast have 2 slices
		if (m_ptrToWorkingSlices.size() > 1)
		{
			if (!IsSubSeriesRotational (m_ptrToWorkingSlices))
			{
				hasShear = HasShear (m_ptrToWorkingSlices, shearCorrection);
			}
		}

#if 0
        // Generate the Initial Model Matrix
        m_scanType = TRModelMatrixGenerator::GetModelMatrix (m_ptrToAllSlices[start].m_ptr->m_imageOrientation, hasShear,
                                                             lModelMatrix, m_sortInAscendingOrder);
      
#endif
        for (int l = 0; l < 16; l++)
		{
			// The scan matrix does not have scales in it
			// At this point the modelMatrix does not have the scales in it
			m_scanMatrix[l] = m_modelMatrix[l] = (float)lModelMatrix[l];
		}



         // Vikram 08/15/2006 - Fix for Gantry Tilted Data
		CalculateScalingAndUniformity (m_ptrToWorkingSlices, hasShear, shearCorrection);

         // Now Sort the list of working slices to reflect the order in which the should be loaded
         std::stable_sort (m_ptrToWorkingSlices.begin(), m_ptrToWorkingSlices.end(), iRTVPtrToSliceInformation::SortByPositionInCache);

    }
    else
    {
        // Need to copy the slice over to make sure that we calculate the
        // right set of slices to load
        for (int i = start; i <= end; i++)
        {
            m_ptrToWorkingSlices.push_back (m_ptrToAllSlices[i]);
            m_ptrToWorkingSlices[sliceNumber].m_ptr->m_sliceNumber = sliceNumber;
 			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_positonInAllSlicesVector = i;
            sliceNumber++;
        }
		
		// Vikram 05/04/04 Fixing Linked Multi-Data load without cache
		nvli::VLIMatrix::Identity ().CopyTo (m_modelMatrix);
 		nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);
        
		CalculateScalingAndUniformity (m_ptrToWorkingSlices);
    }

#endif
	return kSuccess;
}

//-----------------------------------------------------------------------------
// Vikram 05/04/04 Fixing Linked Multi-Data load without cache
int iRTVDICOMGroup::BuildModelMatrixForSort2D (int iStart, int iEnd)
{
	int sliceNumber = 0;
	int start = iStart;
	int end   = iEnd;

#if 0
	m_currentMinVoxelValue = m_ptrToAllSlices[iStart].m_ptr->m_sliceMinVoxelValue;
	m_currentMaxVoxelValue = m_ptrToAllSlices[iStart].m_ptr->m_sliceMaxVoxelValue;

	// Vikram 05/04/04 Fixing Linked Multi-Data load without cache
	m_ptrToWorkingSlices.clear ();
  
	if (IsSOPClassUIDCT(m_sopClassUID) || 
		IsSOPClassUIDMR(m_sopClassUID)  || 
		IsModalityPT(m_sopClassUID) ||
		IsModalityNM(m_sopClassUID) ||
	//	IsModalityDX(m_sopClassUID) ||
		(m_sopClassUID == "" && (IsModalityCT(m_modality) || 
		                         IsModalityMR(m_modality) || 
		                         IsModalityNM(m_modality) || 
							//	 IsModalityDX(m_modality) || 
								 IsModalityPT(m_modality))))
    {
       
		nvli::VLIMatrix::Identity ().CopyTo (m_modelMatrix);
		nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);

		m_scanType = kAxial;

		int minVV = 0;
		int maxVV = 0;
		// Need to copy the slice over to make sure that we calculate the
        // right set of slices to load
		
		for (int i = start; i <= end; i++)
        {
			minVV = m_ptrToAllSlices[i].m_ptr->m_sliceMinVoxelValue;
			maxVV = m_ptrToAllSlices[i].m_ptr->m_sliceMaxVoxelValue;

			if (minVV < m_currentMinVoxelValue) m_currentMinVoxelValue = minVV;
			if (maxVV > m_currentMaxVoxelValue) m_currentMaxVoxelValue = maxVV;

            m_ptrToWorkingSlices.push_back (m_ptrToAllSlices[i]);
            m_ptrToWorkingSlices[sliceNumber].m_ptr->m_sliceNumber = sliceNumber;
			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_positonInAllSlicesVector = i;
            sliceNumber++;
        }
    }
    else
    {
        // Need to copy the slice over to make sure that we calculate the
        // right set of slices to load
        for (int i = start; i <= end; i++)
        {
            m_ptrToWorkingSlices.push_back (m_ptrToAllSlices[i]);
            m_ptrToWorkingSlices[sliceNumber].m_ptr->m_sliceNumber = sliceNumber;
 			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_positonInAllSlicesVector = i;
            sliceNumber++;
        }
		
		nvli::VLIMatrix::Identity ().CopyTo (m_modelMatrix);
		nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);
        CalculateScalingAndUniformity (m_ptrToWorkingSlices);
    }
#endif
	return kSuccess;
}




//-----------------------------------------------------------------------------
int  iRTVDICOMGroup::OpenDataFile (std::string& iLoadFile, int iBegin, int iNumSlices)
{
	
#if 0
	CloseDataFile();
	m_currentSliceToLoad = 0;
	m_ptrToWorkingSlices.clear ();

    int sliceNumber = 0;
    int start = 0;
    int end   = m_ptrToAllSlices.size()-1;

    if (iBegin > -1 && iNumSlices > -1)
    {
        start = iBegin;
        end =   start + iNumSlices - 1;
    }

    if (end >= m_ptrToAllSlices.size() || start < 0)
    {
        return -1;
    }


	m_srcFile = CreateFile(iLoadFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
				OPEN_EXISTING, FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN, 0);
	//			OPEN_EXISTING, 0, 0);
	if (m_srcFile == INVALID_HANDLE_VALUE)  
	{
		m_srcFile = 0;
		return kErrFileStateIsNotGood;
	}

	// Vikram 06/13/03 2D Rendering Fix
	if (m_currentSortType == kSortAs2DSlices)
	{
		nvli::VLIMatrix::Identity ().CopyTo (m_modelMatrix);
		nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);

		int minVV = 0;
		int maxVV = 0;
		int sliceNumber = 0;

		for (int i = start; i <= end; i++)
		{
			minVV = m_ptrToAllSlices[i].m_ptr->m_sliceMinVoxelValue;
			maxVV = m_ptrToAllSlices[i].m_ptr->m_sliceMaxVoxelValue;

			if (minVV < m_currentMinVoxelValue) m_currentMinVoxelValue = minVV;
			if (maxVV > m_currentMaxVoxelValue) m_currentMaxVoxelValue = maxVV;

			m_ptrToWorkingSlices.push_back (m_ptrToAllSlices[i]);
			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_sliceNumber = sliceNumber;
			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_positonInAllSlicesVector = i;
			sliceNumber++;
		}		
	}
	else
	{
		BuildModelMatrix(start, end);
	}

#endif
	return kSuccess;
}
//-----------------------------------------------------------------------------
int  iRTVDICOMGroup::OpenCompressedDataFile (std::string& iCompressedFile)
{
	
	CloseCompressedDataFile();

	m_currentCompressedSliceToLoad = 0;

	if (::access (iCompressedFile.c_str(), 0) == 0)
	{
		m_compressedSrcFile = fopen (iCompressedFile.c_str(), "rb");
	}
	else
		return kErrCouldNotOpenL0CompressedCache;

	if (m_compressedSrcFile > 0) 
    {
		setvbuf (m_compressedSrcFile, 0, _IOFBF, 2*1024*1024);

		return kSuccess;
    }
	else
		return kErrFileStateIsNotGood;
	
}

//-----------------------------------------------------------------------------

int  iRTVDICOMGroup::CloseDataFile ()
{
	if (m_srcFile)
		CloseHandle (m_srcFile), m_srcFile = 0;	
	return kSuccess;
}

//-----------------------------------------------------------------------------

int  iRTVDICOMGroup::CloseCompressedDataFile ()
{
	if (m_compressedSrcFile)
		fclose (m_compressedSrcFile), m_compressedSrcFile = 0;	

	return kSuccess;
}

//----------------------------------------------------------------------------
// -- 2005.03.24
// Take care of the case of a GML scan without the type tag
bool iRTVDICOMGroup::UseNewPETDataConversion(void) const
{
	return m_useGMLConversion != 0 ||
		   m_PETUnits=="GML" || m_PETUnits=="BQML" || m_PETUnits=="PROPCNTS";
}

//----------------------------------------------------------------------------
// -- 2004.12.30
// PET related stuff
float iRTVDICOMGroup::m_minGML	= 0.0f;
float iRTVDICOMGroup::m_maxGML	= 41.0f;
float iRTVDICOMGroup::m_minBQML = 0.0f;
float iRTVDICOMGroup::m_maxBQML = 0.0f;

//-----------------------------------------------------------------------------
// -- 2004.08.25
// need this in multiple places
void iRTVDICOMGroup::ProcessPETData(void)
{	
	if (m_PETProcessed)
		return;

#if 0
	m_PETProcessed =  1;

	m_useGMLConversion = 0;

	float averageSlope = 0.0f, averageOffset = 0.0, averageVOIWidth = 0.0, averageVOICenter = 0.0;
	iRTVSliceInformation *thisSlice = 0;

	// -- 2004.12.29 support SUV
	thisSlice = m_ptrToWorkingSlices[0].m_ptr;

	double fmin = thisSlice->m_rescaleSlope * thisSlice->m_sliceMinVoxelValue + thisSlice->m_rescaleIntercept;
	double fmax = thisSlice->m_rescaleSlope * thisSlice->m_sliceMaxVoxelValue + thisSlice->m_rescaleIntercept;
	double f = 0.0;

	// -- 2006.06.10 do correction - although it is the right thing
	// to do so each slice is normalized, the effect is minimal here, may even
	// make the contrast worse. Do correction here only if m_correctPET == 2
	if (m_PETUnits != "BQML" || m_correctPET != 2)
	{
		for ( int i = this->m_ptrToWorkingSlices.size(); --i >= 0; )
		{
			thisSlice = m_ptrToWorkingSlices[i].m_ptr;
			
			averageSlope	+= thisSlice->m_rescaleSlope;
			averageOffset	+= thisSlice->m_rescaleIntercept;
			averageVOIWidth += thisSlice->m_voiWindowWidth;
			averageVOICenter+= thisSlice->m_voiWindowCenter;
			
			f = thisSlice->m_rescaleSlope * thisSlice->m_sliceMinVoxelValue + thisSlice->m_rescaleIntercept;
			if (f < fmin) fmin = f;
			f = thisSlice->m_rescaleSlope * thisSlice->m_sliceMaxVoxelValue + thisSlice->m_rescaleIntercept;
			if (f > fmax) fmax = f;
			
#ifdef _DEBUG
			fprintf(stderr,"Image: %03d GML=%g\n", thisSlice->m_imageNumber, f);
#endif
		}
	}
	else
	{
		double factor;
		for ( int i = this->m_ptrToWorkingSlices.size(); --i >= 0; )
		{
			thisSlice = m_ptrToWorkingSlices[i].m_ptr;

			if ((factor = thisSlice->m_PETAttrib.CalculateDecayCorrection()) <= 0.0)
				factor = 1.0;

			averageSlope	+= thisSlice->m_rescaleSlope*factor;
			averageOffset	+= thisSlice->m_rescaleIntercept;
			averageVOIWidth += thisSlice->m_voiWindowWidth;
			averageVOICenter+= thisSlice->m_voiWindowCenter;
			
			f = thisSlice->m_rescaleSlope*factor * thisSlice->m_sliceMinVoxelValue + thisSlice->m_rescaleIntercept;
			if (f < fmin) fmin = f;
			f = thisSlice->m_rescaleSlope*factor * thisSlice->m_sliceMaxVoxelValue + thisSlice->m_rescaleIntercept;
			if (f > fmax) fmax = f;
			
#ifdef _DEBUG
			fprintf(stderr,"Image: %03d GML=%g\n", thisSlice->m_imageNumber, f);
#endif
			
		}
	}

	averageSlope	/= m_ptrToWorkingSlices.size();
	averageOffset	/= m_ptrToWorkingSlices.size();
	averageVOIWidth /= m_ptrToWorkingSlices.size();
	averageVOICenter /= m_ptrToWorkingSlices.size();


	m_averageSlope		= averageSlope;
	m_averageIntercept  = averageOffset;
	m_averageVOIWidth	= averageVOIWidth;
	m_averageVOICenter	= averageVOICenter;

	// clamp GML
	m_clampedMin = fmin;
	m_clampedMax = fmax;

	if (m_PETUnits == "GML")
	{
		if (m_maxGML > m_minGML)
		{
			if (fmin < m_minGML)
				m_clampedMin = m_minGML;
			if (fmax > m_maxGML)
				m_clampedMax = m_maxGML;
		}
	}
	else if (m_PETUnits == "BQML")
	{
		if (m_maxBQML > m_minBQML)
		{
			if (fmin < m_minBQML)
				m_clampedMin = m_minBQML;
			if (fmax > m_maxBQML)
				m_clampedMax = m_maxBQML;
		}	
	}
	else if (m_PETUnits == "PROPCNTS")
	{
		;
	}
	// if the max is not too large, try to get all the pixels
	else 
	{
		if (fmax < 1.2*m_maxGML)
		{
			if (fmax > m_maxGML)
				m_clampedMax = m_maxGML;
			if (fmin < m_minGML)
				m_clampedMin = m_minGML;
			m_useGMLConversion = 1;
		}
		else if (fmax < 2000.0f)
		{
			m_useGMLConversion = 0;//1; // TCZ 2005.03.31 haven't tested enough for this
			if (fmin < 0)
				m_clampedMin = 0;
		}

	}

	// using the clamped to convert back to [0,4095]
	m_remapScale		= 4095.0/(m_clampedMax - m_clampedMin);
	m_remapIntercept	= -m_clampedMin*m_remapScale;

#endif

}

//-----------------------------------------------------------------------------
// -- final fix for v1.4.1 2004.08.24
void iRTVDICOMGroup::OldPETConvert(unsigned short* iData, int iSliceNum)
{
#if 1
	return ;
#else
	if ((m_currentMinVoxelValue < m_currentMaxVoxelValue) &&
		(m_currentMinVoxelValue < 0 || m_currentMaxVoxelValue > ((1<<m_bitsUsedPerPixel) -1)))
	{
		// We need to adjust the dynamic range of the slice
		int totalSize = m_sizeX * m_sizeY;
		short* pData = (short*)iData;
		
		AQNetDataConverter converter;
		int voiRescaled = 0;
		
		int sliceNum = iSliceNum;//iBuffer.GetWorkingSliceNumber();
		
		if (sliceNum >= 0 && sliceNum < this->m_ptrToWorkingSlices.size())
		{
			m_rescaleSlope		= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleSlope;
			m_rescaleIntercept	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleIntercept;
			m_voiWindowWidth	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth;
			m_voiWindowCenter	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter;
		}
		else
		{
			assert(0);
		}
		
		if (fabs(1.0-m_averageSlope) > 0.05 )
		{
			// without knowing the histogram, this is at best a rough estimate
			float extraScale = 1.0f, maxScaled = 1.0, minScaled = 0.0, diff;
			maxScaled = m_currentMaxVoxelValue * m_averageSlope + m_averageIntercept;
			minScaled = m_currentMinVoxelValue * m_averageSlope + m_averageIntercept;
			diff = maxScaled - minScaled;
			
			if (diff < 4000)
			{
				if (diff < 0.5) diff = 0.5;
				extraScale = 4094.0f/diff;
			}
			
			// correct voi
			if (m_voiWindowWidth && m_voiWindowWidth < 50 && diff < 4000)
			{
				m_voiWindowWidth = m_voiWindowWidth * extraScale;
				m_voiWindowCenter = m_voiWindowCenter * extraScale*0.95;
				if (m_voiWindowWidth > 4094) m_voiWindowWidth = 4094;
				if (m_voiWindowCenter > 2047) m_voiWindowCenter = 2047;
				voiRescaled = 1;
			}
			
			// we may also have wrong voi
			if (diff  < m_voiWindowWidth && extraScale <= 1.1)
			{
				if (diff < 0.5)
					diff = 0.5;
				extraScale = (m_averageVOIWidth>7000.0f ? 7000.0f:m_averageVOIWidth) /diff;
			}
			
			
			if (m_voiWindowWidth)
			{
				if (m_voiWindowWidth > 100)
				{
					converter.SetVOIWindowWidth (m_voiWindowWidth);
					converter.SetVOIWindowCenter (m_voiWindowCenter);
				}
			}
			
			if (m_voiWindowWidth == 0 && m_voiWindowCenter == 0)
			{
				if (diff > 4900)
					extraScale = 4900.0f/maxScaled;	
			}
			
			
			if (m_ptrToWorkingSlices[sliceNum].m_ptr->m_pixelRepresentation == 1)
			{
				converter.ReplaceNegative(pData,totalSize);
			}
			
			
			converter.ConvertPTToVoxel((unsigned short*)pData,totalSize,
				m_rescaleSlope, m_rescaleIntercept,extraScale,true);
			
			m_slope = 1.05*extraScale/(m_voiWindowCenter ? m_voiWindowCenter:2000);
		}
		else
		{
			converter.SetVOIWindowWidth (m_voiWindowWidth);
			converter.SetVOIWindowCenter (m_voiWindowCenter);
			converter.RescalePT(pData, pData, totalSize, m_currentMinVoxelValue, m_currentMaxVoxelValue);
			// -- JULU-14-2003 remember scale and offset
			converter.GetPixelToVoxelMapping(m_slope, m_offset); 
		}
		
		// hack alert
		int ww = converter.GetVOIWindowWidth(),wl = converter.GetVOIWindowCenter();
		
		if (m_voiWindowWidth && (wl > 4096 || ww > 4095))
		{
			m_offset = 0;
			m_slope  = 1800.0/m_voiWindowCenter;
			m_slope = max(3600.0/m_voiWindowWidth, m_slope);	
		}
		
		if (voiRescaled)
		{
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth = m_voiWindowWidth/m_slope;
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter = m_voiWindowCenter/m_slope;
		}
	}	
#endif
}

//-----------------------------------------------------------------------------
// -- final fix for v1.4.1 2004.08.24
void iRTVDICOMGroup::OldPETConvert(iRTVSliceBuffer& iBuffer)
{
#if 1
	OldPETConvert((unsigned short*)iBuffer.GetData(),iBuffer.GetWorkingSliceNumber());
#else
	if ((m_currentMinVoxelValue < m_currentMaxVoxelValue) &&
		(m_currentMinVoxelValue < 0 || m_currentMaxVoxelValue > ((1<<m_bitsUsedPerPixel) -1)))
	{
		// We need to adjust the dynamic range of the slice
		int totalSize = m_sizeX * m_sizeY;
		short* pData = (short*)iBuffer.GetData();
		
		AQNetDataConverter converter;
		int voiRescaled = 0;
		
		int sliceNum = iBuffer.GetWorkingSliceNumber();
		
		if (sliceNum >= 0 && sliceNum < this->m_ptrToWorkingSlices.size())
		{
			m_rescaleSlope		= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleSlope;
			m_rescaleIntercept	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleIntercept;
			m_voiWindowWidth	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth;
			m_voiWindowCenter	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter;
		}
		else
		{
			assert(0);
		}
		
		if (fabs(1.0-m_averageSlope) > 0.05 )
		{
			// without knowing the histogram, this is at best a rough estimate
			float extraScale = 1.0f, maxScaled = 1.0, minScaled = 0.0, diff;
			maxScaled = m_currentMaxVoxelValue * m_averageSlope + m_averageIntercept;
			minScaled = m_currentMinVoxelValue * m_averageSlope + m_averageIntercept;
			diff = maxScaled - minScaled;
			
			if (diff < 4000)
			{
				if (diff < 0.5) diff = 0.5;
				extraScale = 4094.0f/diff;
			}
			
			// correct voi
			if (m_voiWindowWidth && m_voiWindowWidth < 50 && diff < 4000)
			{
				m_voiWindowWidth = m_voiWindowWidth * extraScale;
				m_voiWindowCenter = m_voiWindowCenter * extraScale*0.95;
				if (m_voiWindowWidth > 4094) m_voiWindowWidth = 4094;
				if (m_voiWindowCenter > 2047) m_voiWindowCenter = 2047;
				voiRescaled = 1;
			}
			
			// we may also have wrong voi
			if (diff  < m_voiWindowWidth && extraScale <= 1.1)
			{
				if (diff < 0.5)
					diff = 0.5;
				extraScale = (m_averageVOIWidth>7000.0f ? 7000.0f:m_averageVOIWidth) /diff;
			}
			
			
			if (m_voiWindowWidth)
			{
				if (m_voiWindowWidth > 100)
				{
					converter.SetVOIWindowWidth (m_voiWindowWidth);
					converter.SetVOIWindowCenter (m_voiWindowCenter);
				}
			}
			
			if (m_voiWindowWidth == 0 && m_voiWindowCenter == 0)
			{
				if (diff > 4900)
					extraScale = 4900.0f/maxScaled;	
			}
			
			
			if (m_ptrToWorkingSlices[sliceNum].m_ptr->m_pixelRepresentation == 1)
			{
				converter.ReplaceNegative(pData,totalSize);
			}
			
			
			converter.ConvertPTToVoxel((unsigned short*)pData,totalSize,
				m_rescaleSlope, m_rescaleIntercept,extraScale,true);
			
			m_slope = 1.05*extraScale/(m_voiWindowCenter ? m_voiWindowCenter:2000);
		}
		else
		{
			converter.SetVOIWindowWidth (m_voiWindowWidth);
			converter.SetVOIWindowCenter (m_voiWindowCenter);
			converter.RescalePT(pData, pData, totalSize, m_currentMinVoxelValue, m_currentMaxVoxelValue);
			// -- JULU-14-2003 remember scale and offset
			converter.GetPixelToVoxelMapping(m_slope, m_offset); 
		}
		
		// hack alert
		int ww = converter.GetVOIWindowWidth(),wl = converter.GetVOIWindowCenter();
		
		if (m_voiWindowWidth && (wl > 4096 || ww > 4095))
		{
			m_offset = 0;
			m_slope  = 1800.0/m_voiWindowCenter;
			m_slope = max(3600.0/m_voiWindowWidth, m_slope);	
		}
		
		if (voiRescaled)
		{
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth = m_voiWindowWidth/m_slope;
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter = m_voiWindowCenter/m_slope;
		}
	}
#endif
}



//------------------------------------------------------------------------------
template <class T> unsigned short Nint(T f)
{
	return unsigned short ( f < 0 ? (f-0.4):(f+0.4));
}

//-----------------------------------------------------------------------------
// -- 2004.12.30 Support SUV
void iRTVDICOMGroup::PETConvert(iRTVSliceBuffer& iBuffer)
{
	// We need to adjust the dynamic range of the slice
	unsigned short* pData = (unsigned short*)iBuffer.GetData();
	int sliceNum = iBuffer.GetWorkingSliceNumber();

	// unified Q/R and cache cases
	PETConvert(pData,sliceNum);
}


//-----------------------------------------------------------------------------
// -- 2005.03.17 Support PET Q/R - Separated out from PETConvert(buffer)
void iRTVDICOMGroup::PETConvert(unsigned short* iData, int sliceNum)
{
#if 0
	AQNetDataConverter converter;
	int voiRescaled = 0;
	unsigned short* pData = iData;
	int totalSize = m_sizeX * m_sizeY;

	if (!pData)
		return;

	if (sliceNum >= 0 && sliceNum < this->m_ptrToWorkingSlices.size())
	{
		// -- 2006.06.10
		double factor = m_ptrToWorkingSlices[sliceNum].m_ptr->m_PETAttrib.CalculateDecayCorrection();
		if (factor <= 0.0)
			factor = 1;
		if (m_PETUnits != "BQML" || !m_correctPET)
		{
			m_rescaleSlope		= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleSlope;
			m_rescaleIntercept	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleIntercept;
			m_voiWindowWidth	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth;
			m_voiWindowCenter	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter;
		}
		else
		{
			m_rescaleSlope		= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleSlope*factor;
			m_rescaleIntercept	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleIntercept;
			m_voiWindowWidth	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth;
			m_voiWindowCenter	= m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter;
		}
	}
	else
	{
		assert(0);
	}
	
	if (fabs(1.0-m_averageSlope) > 0.005 )
	{
		if (m_ptrToWorkingSlices[sliceNum].m_ptr->m_pixelRepresentation == 1)
		{
			converter.ReplaceNegative((short*)pData,totalSize);
		}

		unsigned short *s = pData;
		float f, fmax = 0.0;
		int smax = 0;

		for (int  i = 0; i < totalSize; i++)
		{
			f = s[i] * m_rescaleSlope + m_rescaleIntercept;

			if (fmax < f) fmax = f;

			if (f < m_clampedMin)
				f =  m_clampedMin;
			else if (f > m_clampedMax)
				f =  m_clampedMax;

			s[i] = Nint(f * m_remapScale + m_remapIntercept);
			if (smax < s[i]) smax = s[i];
		
		}

		// get the slope and offset that converts [0,4095] back to 
		m_slope  = (m_clampedMax - m_clampedMin)/4095;
		m_offset = m_clampedMin;

#ifdef _DEBUG
		fprintf(stderr,"imageNumber=%03d GML=%g Pixel=%d slope=%g slope*pixel=%g\n",
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_imageNumber,f, smax, m_slope,smax*m_slope);
#endif		
		// VOI processing is more or less for working around bad PET dicom files.
		// The division by m_slope is kind of wrong [compensates for other stuff]
		if (m_voiWindowWidth < 0.1)
		{
			int WW = 350, WL=175;
			if (m_ptrToWorkingSlices[sliceNum].m_ptr->m_PETAttrib.m_units == "BQML")
			{
				WW = 800;
				WL = 400;
			}
			
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth =  WW / m_slope;
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter = WL / m_slope;
		}
		else if (m_voiWindowWidth > 4095*m_slope)
		{
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth =  350 / m_slope;
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter = 175 / m_slope;
		}
		else 
		{
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowWidth = (m_voiWindowWidth * m_remapScale)/m_slope;
			m_ptrToWorkingSlices[sliceNum].m_ptr->m_voiWindowCenter = (m_voiWindowCenter* m_remapScale + m_remapIntercept)/m_slope;
		}
	}
	else
	{
		converter.SetVOIWindowWidth (m_voiWindowWidth);
		converter.SetVOIWindowCenter (m_voiWindowCenter);
		converter.RescalePT(pData, pData, totalSize, m_currentMinVoxelValue, m_currentMaxVoxelValue);
		// -- JULU-14-2003 remember scale and offset
		converter.GetPixelToVoxelMapping(m_slope, m_offset); 
	}
#endif
}

//-----------------------------------------------------------------------------
int iRTVDICOMGroup::ProcessLoadData(iRTVSliceBuffer& iBuffer, bool iPadOnly)
{
#if 0

	float averageSlope = 0.0f, averageOffset = 0.0, averageVOIWidth = 0.0, averageVOICenter = 0.0;
	int i = 0;

	// -- 2004.04.19
	// many PET data from GE has bogus slopes and/or VOI lut, we need
	// to guess what the extra scaling we should do. Average slope will

	if (IsSOPClassUIDPT(m_sopClassUID))
	{
		ProcessPETData();
		
		averageSlope		= this->m_averageSlope;
		averageOffset		= this->m_averageIntercept;
		averageVOIWidth		= this->m_averageVOIWidth;
		averageVOICenter	= this->m_averageVOICenter;
	}

    // If this is a 24-bit dataset we need to pad so VLI can directly load it 
    // on to the board.
    // Vikram 05/01/02
    if (m_samplesPerPixel == 3)
    {
        int totalSize = m_sizeX * m_sizeY;
        int idx = 0;
        unsigned int*  tBuffer = new unsigned int [totalSize];
        unsigned char* pData = (unsigned char*)iBuffer.GetData();
        
		unsigned char* ptBuffer;
        for (i = 0; i < totalSize; i++)
        {
            ptBuffer = (unsigned char*)&tBuffer[i];
            ptBuffer[0] = pData[idx++];
            ptBuffer[1] = pData[idx++];
            ptBuffer[2] = pData[idx++]; 
			ptBuffer[3] = 255;
        }
        
        memcpy (pData, tBuffer, sizeof (int) * totalSize);
        // Delete the data
        delete [] tBuffer;

    }
	else if(!iPadOnly)
	{
		int sliceNum = iBuffer.GetWorkingSliceNumber();
		float slope	 = m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleSlope;
		float intercept = m_ptrToWorkingSlices[sliceNum].m_ptr->m_rescaleIntercept;
		
		if ( IsSOPClassUIDPT(m_sopClassUID) ||
			(m_sopClassUID == "" && (IsModalityPT(m_modality))))
		{
			if (m_bytesPerPixel == 2) // only for 16 bit data for right now
			{
				if (UseNewPETDataConversion())
					PETConvert(iBuffer);
				else
					OldPETConvert(iBuffer);
			}
		}
		// -- 2004.05.16 Added NM conversion
		else if ( IsSOPClassUIDNM(m_sopClassUID) ||
			(m_sopClassUID == "" && (IsModalityNM(m_modality))))
		{
			if (m_bytesPerPixel == 2) // only for 12 bit data for right now
			{
#if 0
				// We need to adjust the dynamic range of the slice
				int totalSize = m_sizeX * m_sizeY;
				unsigned short* pData = (unsigned short*)iBuffer.GetData();
				int minVoxelValue = m_currentMinVoxelValue;
				int maxVoxelValue = m_currentMaxVoxelValue;
				AQNetDataConverter converter;

			
				converter.SetVOIWindowWidth (m_voiWindowWidth);
				converter.SetVOIWindowCenter (m_voiWindowCenter);
				
					// -- 2006.01.13
				// workaround for some bad data characterized by W/L=(65k,32k)
				if (m_voiWindowWidth > 40000 && m_voiWindowCenter > 20000 &&
					maxVoxelValue < 2000 && minVoxelValue < 0 )
				{
					
					int m = maxVoxelValue - minVoxelValue;
					converter.SetVOIWindowWidth ( m);
					converter.SetVOIWindowCenter ( -minVoxelValue + m/2);
					short *s = (short *)pData;
					for ( i = 0; i < totalSize; i++)
					{
						s[i] += minVoxelValue;
						if (s[i] < 0)
							s[i] = 0;
						
					}

					m_slope = m/m_voiWindowWidth;

				}
				else
				{
					
					converter.RescaleNM(pData, pData, totalSize, 0, maxVoxelValue);
					// -- JULU-14-2003 remember scale and offset
					converter.GetPixelToVoxelMapping(m_slope, m_offset); 
				}
#endif
			}
		}
		// Do we need to adjust for signed MR data
		else if ( IsSOPClassUIDMR(m_sopClassUID) ||
			(m_sopClassUID == "" && (IsModalityMR(m_modality))))
		{
			if (m_bytesPerPixel == 2) // only for 12 bit data for right now
			{
				// -- 2006.03.17
				// Handle MR modalityLUT
				int isSigned = m_ptrToWorkingSlices[sliceNum].m_ptr->m_pixelRepresentation;

				int vmin	 = m_currentMinVoxelValue, vmax = m_currentMaxVoxelValue;

				int hasLUT = (fabs(slope - 1.0) > 0.001) || ( fabs(intercept) > 0.01);

				if ( hasLUT || ((vmin < vmax) && (vmin < 0 || vmax > ((1<<m_bitsUsedPerPixel) -1))))
				{
					// We need to adjust the dynamic range of the slice
					int totalSize = m_sizeX * m_sizeY;
					short* pData = (short*)iBuffer.GetData();
#if 0
					AQNetDataConverter converter;

					converter.SetVOIWindowWidth (m_voiWindowWidth);
					converter.SetVOIWindowCenter (m_voiWindowCenter);
					
				
					if (isSigned)
					{
						converter.ProcessMRModalityLUT(pData,totalSize, slope, intercept, vmin, vmax);
					}
					else
					{
						converter.ProcessMRModalityLUT((unsigned short*)pData,totalSize, slope, intercept, vmin, vmax);
					}

					// we can't use min/max per slice 
					if (hasLUT && converter.m_useMRModalityLUT)
					{
						vmin = Nint(m_currentMinVoxelValue * slope + intercept);
						vmax = Nint(m_currentMaxVoxelValue * slope + intercept);
					}

					// -- 2006.01.23 This is to avoid excess saturation
					if (m_voiWindowWidth > 4096 && 	vmax >= 4*m_voiWindowWidth)
					{
						int n = vmax/m_voiWindowWidth;
						converter.SetVOIWindowWidth (2*m_voiWindowWidth);
						m_voiWindowWidth = 2*m_voiWindowWidth;
					}

					// -- 2006.09.05 we scale the data only if the values are out of range.
					// This check preserves more original data
					if (vmin < 0 || vmax > 4095 || (vmin > vmax) || abs(vmax - vmin) <= kMinMRDynamicRangeWithModalityLUTApplied)
						converter.RescaleMR (pData, pData, totalSize, vmin, vmax);


					// -- JULU-14-2003 remember scale and offset
					converter.GetPixelToVoxelMapping(m_slope, m_offset); 
#endif
				}
			}
		}
		// -- 2006.06.08
		// Get mapping offset
		else if (IsSOPClassUIDCT(m_sopClassUID) ||
			(m_sopClassUID == "" && (IsModalityCT(m_modality))))
		{
			// cache already converted data. just get the offset
		//	m_offset = intercept;

		}
		else if (IsSOPClassUIDSC(m_sopClassUID) && m_bytesPerPixel == 2)
		{
#if 1
			AQNetDataConverter converter;
			short* pData = ( short*)iBuffer.GetData();
			int totalSize = m_sizeX * m_sizeY;

			if (IsModalityPT(m_modality))
			{
				if (UseNewPETDataConversion())
					PETConvert(iBuffer);
				else
					OldPETConvert(iBuffer);
			}
			else if (IsModalityMR(m_modality))
			{
				// We need to calculate the dynamic range and map it to lower 12 bits
				
				int minVoxelValue = m_currentMinVoxelValue;
				int maxVoxelValue = m_currentMaxVoxelValue;
			
				
				// Need to do it for data that was SC/12-bit and pushed before 1.2
				if ((m_currentMinVoxelValue > m_currentMaxVoxelValue) &&
					(m_currentMinVoxelValue ==  kDefaultMinMaxVoxelValue || 
					m_currentMaxVoxelValue == - kDefaultMinMaxVoxelValue))
				{
					// -- JUN-26-2003 only need to find minmax
					converter.FindMinMax (pData, totalSize, true, minVoxelValue, maxVoxelValue);
					//						converter.ConvertMRToVoxel (pData, totalSize, true, minVoxelValue, maxVoxelValue);
				}
				
				converter.SetVOIWindowWidth  (int(m_voiWindowWidth));
				converter.SetVOIWindowCenter (int(m_voiWindowCenter));
				converter.RescaleMR (pData, pData, totalSize, minVoxelValue, maxVoxelValue);
			 
				// -- JULU-14-2003 remember scale and offset
				converter.GetPixelToVoxelMapping(m_slope, m_offset); 
			}
			// -- 2005.05.19
			// Fixed AQWS 12bit SC loading - can break other SC's - need testing
			else if (IsModalityCT(m_modality))
			{
				converter.ConvertToVoxel((unsigned short*)pData, totalSize,1,-1024,true,0);
		//		m_sinfo->m_rescaleSlope, m_sinfo->m_rescaleIntercept, true, 0);

			}
#else
			if (IsModalityMR(m_modality) || IsModalityPT(m_modality))
			{
				// We need to calculate the dynamic range and map it to lower 12 bits
				int totalSize = m_sizeX * m_sizeY;
				short* pData = ( short*)iBuffer.GetData();
				int minVoxelValue = m_currentMinVoxelValue;
				int maxVoxelValue = m_currentMaxVoxelValue;
				AQNetDataConverter converter;
				
				// Need to do it for data that was SC/12-bit and pushed before 1.2
				if ((m_currentMinVoxelValue > m_currentMaxVoxelValue) &&
					(m_currentMinVoxelValue ==  kDefaultMinMaxVoxelValue || 
					m_currentMaxVoxelValue == - kDefaultMinMaxVoxelValue))
				{
					// -- JUN-26-2003 only need to find minmax
					converter.FindMinMax (pData, totalSize, true, minVoxelValue, maxVoxelValue);
					//						converter.ConvertMRToVoxel (pData, totalSize, true, minVoxelValue, maxVoxelValue);
				}
				
				converter.SetVOIWindowWidth (m_voiWindowWidth);
				converter.SetVOIWindowCenter (m_voiWindowCenter);
				
				if (IsModalityMR(m_modality))
					converter.RescaleMR (pData, pData, totalSize, minVoxelValue, maxVoxelValue);
				else
					converter.RescalePT (pData, pData, totalSize, minVoxelValue, maxVoxelValue);

				// -- JULU-14-2003 remember scale and offset
				converter.GetPixelToVoxelMapping(m_slope, m_offset); 

					// hack alert
				if (IsModalityPT(m_modality))
				{
					int ww = converter.GetVOIWindowWidth(),wl = converter.GetVOIWindowCenter();
					
						
					if (m_voiWindowWidth > 1.0e-5 && (wl > 4096 || ww > 4095))
					{
						m_offset = 0;
						m_slope  = 1500.0/m_voiWindowCenter;
						m_slope = max(3000.0/m_voiWindowWidth, m_slope);
						
					}
				}
			}
#endif // secondary captured MR or PET
		}
	}


#endif
	return kSuccess;
}

//-----------------------------------------------------------------------------
DWORD ReadFileWin32(HANDLE hFile, __int64 iBytesToSkip, int iBytesToRead, iRTVSliceBuffer& iBuffer)
{
	// make a sector aligned skip
	int frontBytes = iBytesToSkip%HDISK_SECTOR_SIZE;
	int errcode;
	
	int bytesToSkip = iBytesToSkip - frontBytes;
	if(bytesToSkip >= 0)
	{
		LARGE_INTEGER liOffset;
		liOffset.QuadPart = bytesToSkip;

		DWORD Moved = ::SetFilePointer(hFile, liOffset.LowPart, &liOffset.HighPart, FILE_BEGIN);
		
		// Check return value will not work, when move is larger than 4GB
		if(Moved == INVALID_SET_FILE_POINTER) 
		{
			errcode = GetLastError(); // this is not reliable too, but we have no choice
			if(errcode != NO_ERROR)
				return 0;
		}
		
	}
	
	// make a sector aligne read e.g. read multip sectore and store sector aligned memory
	int fileBytesToRead = frontBytes + iBytesToRead;
	int backBytes = fileBytesToRead%HDISK_SECTOR_SIZE ;
	if(backBytes)
		fileBytesToRead = fileBytesToRead - backBytes + HDISK_SECTOR_SIZE;
	
	// buffer has one more sector in front the page
	// set the pBufferStart before frontBytes bytes on the first page 
	// so that the real data (after frontBytes bytes) will falling on the page
	int actualPad = 0;
	char* pBufferStart = iBuffer.GetData()-frontBytes; 


	// in some rare case when we have odd-image size, frontBytes
	// could be non-zero. take care of that.
	if (frontBytes)
	{
		actualPad = HDISK_SECTOR_SIZE - frontBytes;
		pBufferStart = iBuffer.GetData() - HDISK_SECTOR_SIZE;
	}

	DWORD nBytesRead;
	ReadFile (hFile, pBufferStart, fileBytesToRead, &nBytesRead, 0);

#ifdef _DEBUG
	if (nBytesRead <= 0)
		errcode = GetLastError();
#endif

	if (actualPad)
		memmove(iBuffer.GetData(), iBuffer.GetData() -  actualPad, iBytesToRead);

	return nBytesRead;
}

//-----------------------------------------------------------------------------
// -- 2005.10.11
// Reads the slice raw data - except for CT where data is in HU
int iRTVDICOMGroup::ReadRawSlice (iRTVSliceBuffer& iBuffer, int iSliceNumber)
{
	if (iSliceNumber < 0 || iSliceNumber >= m_ptrToAllSlices.size() ||
		m_srcFile == 0
		)
		return kErrBadInputParameters;
	
	iBuffer.Allocate (m_bytesPerSlice);
	__int64 bytesToSkip = m_ptrToAllSlices[iSliceNumber].m_ptr->m_startOfDataInDataFile;
	int     bytesToRead = m_ptrToAllSlices[iSliceNumber].m_ptr->m_sizeOfData;
	//		int     bytesToRead = 512*512*2;
	int rr = ReadFileWin32(m_srcFile, bytesToSkip, bytesToRead, iBuffer);
	
		if (rr <= 0) 
			return kErrFileStateIsNotGood;
		
	iBuffer.SetWorkingSliceNumber(iSliceNumber); // Murali&TC
	// -- 2005.10.11
	iBuffer.m_pixelRepresentation = m_ptrToAllSlices[iSliceNumber].GetPixelRepresentation();
	return kSuccess;	
}

//-----------------------------------------------------------------------------
// Vikram 09/23/03 Subtract Volumes
// This function just reads the slice from the file without doing any post process
int iRTVDICOMGroup::ReadSlice (iRTVSliceBuffer& iBuffer, int iSliceNumber)
{
	if (iSliceNumber < 0 || iSliceNumber >= m_ptrToAllSlices.size() || m_srcFile == 0)
		return kErrBadInputParameters;

	iBuffer.Allocate (m_bytesPerSlice);
	__int64 bytesToSkip = m_ptrToAllSlices[iSliceNumber].m_ptr->m_startOfDataInDataFile;
	int     bytesToRead = m_ptrToAllSlices[iSliceNumber].m_ptr->m_sizeOfData;
//		int     bytesToRead = 512*512*2;
	int rr = ReadFileWin32(m_srcFile, bytesToSkip, bytesToRead, iBuffer);

	if (rr <= 0) 
		return kErrFileStateIsNotGood;
	
	iBuffer.SetWorkingSliceNumber(iSliceNumber);// Murali&TC
	return ProcessLoadData(iBuffer, true);
}

//-----------------------------------------------------------------------------
// Assumes that there is enoungh memory allocate for the slice
int iRTVDICOMGroup::LoadSlice (iRTVSliceBuffer& iBuffer, int iSliceNumber)
{
	if (iSliceNumber < 0 || iSliceNumber >= m_ptrToAllSlices.size() ||
	m_srcFile == 0
	)
		return kErrBadInputParameters;

	iBuffer.Allocate (m_bytesPerSlice);
	__int64 bytesToSkip = m_ptrToAllSlices[iSliceNumber].m_ptr->m_startOfDataInDataFile;
	int     bytesToRead = m_ptrToAllSlices[iSliceNumber].m_ptr->m_sizeOfData;
//		int     bytesToRead = 512*512*2;
	int rr = ReadFileWin32(m_srcFile, bytesToSkip, bytesToRead, iBuffer);

	if (rr <= 0) 
		return kErrFileStateIsNotGood;
	
	iBuffer.SetWorkingSliceNumber(iSliceNumber); // Murali&TC
	return ProcessLoadData( iBuffer);
}
//-----------------------------------------------------------------------------
// Added on 08/23/02 to fix loading of preview
// Assumes that there is enoungh memory allocate for the slice
int iRTVDICOMGroup::LoadSlice (iRTVSliceBuffer& iBuffer, iRTVSOPLocation& iSOPLocation)
{
	if (m_srcFile <= 0)
		return kErrFileIsNotOpen;

	iBuffer.Allocate (m_bytesPerSlice);
	__int64 bytesToSkip = iSOPLocation.m_startOfDataInDataFile;
	int     bytesToRead = iSOPLocation.m_sizeOfData;
//		int     bytesToRead = 512*512*2;


	int rr = ReadFileWin32(m_srcFile, bytesToSkip, bytesToRead, iBuffer);


	if (rr <= 0) 
		return kErrFileStateIsNotGood;

	return ProcessLoadData( iBuffer);
}


//-----------------------------------------------------------------------------
// consolidated from renderer.cpp 
// -- 05/01/2003
int iRTVDICOMGroup::GetSortedSliceInformation(tLoadedSlice& oOut)
{
#if 0
	std::vector <iRTVPtrToSliceInformation>::iterator p;
	oOut.clear();

	for ( p = m_ptrToWorkingSlices.begin(); p != m_ptrToWorkingSlices.end(); ++p)
	{
		oOut[p->m_ptr->m_sliceNumber] = LoadedSliceInformation(*p->m_ptr);		                                                  
	}
#endif
	return kSuccess;
}
// End of -- 05/01/2003 & 06/16/2003


//-----------------------------------------------------------------------------
// Assumes that there is enoungh memory allocate for the slice
int iRTVDICOMGroup::LoadNextSlice (iRTVSliceBuffer& iBuffer)
{
	if (m_currentSliceToLoad >= m_ptrToWorkingSlices.size() || m_srcFile == 0)
		return kErrBadInputParameters;
	
	int     bytesToRead = m_ptrToWorkingSlices[m_currentSliceToLoad].m_ptr->m_sizeOfData;

	int bytesToAllocate = (m_bytesPerSlice > bytesToRead) ? m_bytesPerSlice : bytesToRead;
	iBuffer.Allocate (bytesToAllocate);
	iBuffer.SetNumberOfBytesStored (bytesToAllocate);
     
    //if (m_currentSliceToLoad == 0)
    //{
	//	__int64 bytesToSkip = m_ptrToWorkingSlices[m_currentSliceToLoad].m_ptr->m_startOfDataInDataFile;
	//    m_srcFileStream->seekg (bytesToSkip, ios::beg);
    //}
	// GL 7-31-2002 seek in all case because other SOP frames may mix in current SOP frames
	__int64 bytesToSkip = m_ptrToWorkingSlices[m_currentSliceToLoad].m_ptr->m_startOfDataInDataFile;


	int rr = ReadFileWin32(m_srcFile, bytesToSkip, bytesToRead, iBuffer);


	if (rr <= 0) 
		return kErrFileStateIsNotGood;

	
    iBuffer.SetSliceNumber (m_ptrToWorkingSlices[m_currentSliceToLoad].m_ptr->m_sliceNumber);
    iBuffer.SetPositonInAllSlicesVector (m_ptrToWorkingSlices[m_currentSliceToLoad].m_ptr->m_positonInAllSlicesVector);
	iBuffer.SetWorkingSliceNumber(m_currentSliceToLoad);	

	int stat = ProcessLoadData(iBuffer);
	if( stat != kSuccess)
		return stat;
	
    m_currentSliceToLoad++;

	return kSuccess;
}
//-----------------------------------------------------------------------------
// Loads the next Compressed Slice
int iRTVDICOMGroup::LoadNextCompressedSlice (iRTVSliceBuffer& iBuffer, float iCompressionRatio)
{
	if (m_currentCompressedSliceToLoad < m_ptrToWorkingSlices.size() &&  m_compressedSrcFile > 0)
	{

		unsigned int     bytesToRead = m_ptrToWorkingSlices[m_currentCompressedSliceToLoad].m_ptr->m_sizeOfL0CompressedData;
		__int64 bytesToSkip = m_ptrToWorkingSlices[m_currentCompressedSliceToLoad].m_ptr->m_startOfL0CompressedDataInDataFile;
		
		iRTVSliceBuffer transcodeBuffer;


		iBuffer.Allocate (bytesToRead);
		if(bytesToSkip >= 0)
			_lseeki64 (fileno(m_compressedSrcFile), SEEK_SET, bytesToSkip);
		int rr = 0;


	

		// Make sure that the compression ratio being asked is the 
		// same atleast the same as the one stored if not transcode it
		// Only if the compression Ratio is greater than the one that is stored at
		bool NeedToTranscode = (iCompressionRatio > m_ptrToWorkingSlices[m_currentCompressedSliceToLoad].m_ptr->m_compressionFactor);
		if (NeedToTranscode)
		{
			transcodeBuffer.Allocate (bytesToRead);
			rr = fread (transcodeBuffer.GetData(), 1, bytesToRead, m_compressedSrcFile);
		}
		else
			rr = fread (iBuffer.GetData(), 1, bytesToRead, m_compressedSrcFile);

		if (rr > 0) 
        {    

			if (NeedToTranscode)
			{
#ifndef AWARE_ONLY
				j2kEncoder encoder;
				encoder.setBigEndian(false);
				encoder.setFastMode(true);

				if(encoder.transcode ((unsigned char*)transcodeBuffer.GetData(), 0, 1.0/iCompressionRatio, 
								      (unsigned char*)iBuffer.GetData(), bytesToRead) != J2K_OK)
#endif
				{
					return kErrJ2kCompressionFailed;
				}
			}

			iBuffer.SetNumberOfBytesStored (bytesToRead);
            iBuffer.SetSliceNumber (m_ptrToWorkingSlices[m_currentCompressedSliceToLoad].m_ptr->m_sliceNumber);
			iBuffer.SetPositonInAllSlicesVector (m_ptrToWorkingSlices[m_currentCompressedSliceToLoad].m_ptr->m_positonInAllSlicesVector);
			iBuffer.SetWorkingSliceNumber(m_currentCompressedSliceToLoad);

			m_currentCompressedSliceToLoad++;
			return kSuccess;
        }
		else
			return kErrFileStateIsNotGood;

	}

	return kSuccess;
}
//-----------------------------------------------------------------------------
void iRTVDICOMGroup::GetImagePosition(int iSlice, double oIP[3])
{
	memcpy (oIP, (m_ptrToAllSlices[iSlice].m_ptr->m_imagePosition), sizeof m_ptrToAllSlices[iSlice].m_ptr->m_imagePosition);
}

//-----------------------------------------------------------------------------


// tc zhao oct-14-2002 
double* iRTVDICOMGroup::GetImageOrientation(int iSlice)
{
	assert(iSlice >=0 && iSlice < m_ptrToAllSlices.size());
	return m_ptrToAllSlices[iSlice].m_ptr->m_imageOrientation;
}

//-----------------------------------------------------------------------------
double *iRTVDICOMGroup::GetPixelSpacing(int iSlice)
{
	assert(iSlice >=0 && iSlice < m_ptrToAllSlices.size());
	return m_ptrToAllSlices[iSlice].m_ptr->m_pixelSpacing;
}
// end of tc zhao  oct-14-2002

//-----------------------------------------------------------------------------
// Vikram & TC 07/20/04
// Always use the scan matrix to calculate the scan type
int	iRTVDICOMGroup::GetScanType()	
{
#if 0
	nvli::VLIVector3D ov;
	nvli::VLIMatrix mat;

	ov.Assign (0,0,1);
	mat.Assign (m_scanMatrix);
    mat.TransformVector(ov, ov);

	double cross[3];

	ov.CopyTo(cross);
	return TRModelMatrixGenerator::GetScanType(cross);
#else
	return 0;
#endif
}

// zhao 2004.07.22
bool iRTVDICOMGroup::HasValidOrientation(int iSlice) const
{
	assert(iSlice >=0 && iSlice < m_ptrToAllSlices.size());
	return m_ptrToAllSlices[iSlice].m_ptr->m_hasValidOrientation != 0;
}

//----------------------------------------------------------------------------
// added by shiying hu, 2005-10-17
// we could like to have model matrix, min, max information ready when
// read cache description
int  iRTVDICOMGroup::UpdateGroupInfo (int iBegin, int iNumSlices)
{
    int sliceNumber = 0;
    int start = 0;
    int end   = m_ptrToAllSlices.size()-1;

    if (iBegin > -1 && iNumSlices > -1)
    {
        start = iBegin;
        end =   start + iNumSlices - 1;
    }

    if (end >= m_ptrToAllSlices.size() || start < 0)
    {
        return -1;
    }
#if 0

	// Vikram 06/13/03 2D Rendering Fix
	if (m_currentSortType == kSortAs2DSlices)
	{
		nvli::VLIMatrix::Identity ().CopyTo (m_modelMatrix);
		nvli::VLIMatrix::Identity ().CopyTo (m_scanMatrix);

		int minVV = 0;
		int maxVV = 0;
		int sliceNumber = 0;

		for (int i = start; i <= end; i++)
		{
			minVV = m_ptrToAllSlices[i].m_ptr->m_sliceMinVoxelValue;
			maxVV = m_ptrToAllSlices[i].m_ptr->m_sliceMaxVoxelValue;

			if (minVV < m_currentMinVoxelValue) m_currentMinVoxelValue = minVV;
			if (maxVV > m_currentMaxVoxelValue) m_currentMaxVoxelValue = maxVV;

			m_ptrToWorkingSlices.push_back (m_ptrToAllSlices[i]);
			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_sliceNumber = sliceNumber;
			m_ptrToWorkingSlices[sliceNumber].m_ptr->m_positonInAllSlicesVector = i;
			sliceNumber++;
		}		
	}
	else
	{
		BuildModelMatrix(start, end);
	}

#endif
	return kSuccess;
}

//----------------------------------------------------------------------------------------
bool iRTVDICOMGroup::SortByPercentRR(const iRTVDICOMGroup& iGroup0, const iRTVDICOMGroup& iGroup1)
{
#if 0
	AqPercentRToR oRRUtil;
	float  fPercRR0 = oRRUtil.GetPercentRToRFromScanOptions(iGroup0.m_scanOptions.c_str(),
														    iGroup0.m_scanOptions.length());																   
	float  fPercRR1 = oRRUtil.GetPercentRToRFromScanOptions(iGroup1.m_scanOptions.c_str(),
															iGroup1.m_scanOptions.length());
	return (fPercRR0 < fPercRR1);
#else
	return false;
#endif
}

//------------------------------------------------------------------------------
#include "PETObjectAttributes.h"
double tPETAttrib::CalculateDecayCorrection(void)
{

	return PETObjectAttributes::GetCorrectionFactor(m_startTime, m_acquisitionTime, m_halfLife);
}
