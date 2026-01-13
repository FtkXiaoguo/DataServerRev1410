/***********************************************************************
 * AppComConversion.cpp
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */

#include "AppComDataConversion.h"

#if !defined (RTVSPROTOCOL_H_)
#include "rtvsprotocol.h"
#endif

#if !defined (RTVSSWAP_H_)
#include "rtvsswap.h"
#endif

#include <stdio.h>
#include <string.h>
//#include <windows.h>
//-----------------------------------------------------------------------------

bool AppComDataConverter::m_bypassVOILUTForAllMR = false;
bool AppComDataConverter::m_bypassVOILUTOnlyForHighContrastMR =false;
int	 AppComDataConverter::m_minWindowToClampTo = -1;
int  AppComDataConverter::m_maxWindowToClampTo = -1;
int	 AppComDataConverter::m_wwIndicatingHighContrast = 5000;
int	 AppComDataConverter::m_useMRModalityLUT;


AppComDataConverter::AppComDataConverter(void)
{
	m_useLUT = 1;
	m_knownMin = m_knownMax = 0;
	m_mappingScale = 1.0;
	m_mappingOffset = 0.0;

	m_voiWindowWidth  = 0;
	m_voiWindowCenter = 0;
	m_CTCLUTID[0] = '\0';
}

//-----------------------------------------------------------------------------

AppComDataConverter::~AppComDataConverter(void)
{


}

//-----------------------------------------------------------------------------

#include <assert.h>
#include <math.h>

//-----------------------------------------------------------------------------
int AppComDataConverter::ConvertCTToVoxel(unsigned short *inOut, 
								   int iNpixels,int isSigned,
								   float iRescaleSlope, 
								   float iRescaleIntercept,
								   unsigned short *out)
{
	if (inOut == 0 ) return -1;
	
	float rescaleIntercept =  (iRescaleIntercept + 1024.0f); // from HU to all positive
	int val, i, use1024 = 0;
	
	
	if (!out)
		out = inOut;
	
	
	// since app may use the offset info from the header, we need to make sure
	// we always use that
	int allowdMin = 0;
	if (!use1024)
	{
		if (fabs(rescaleIntercept) < 25.0)
			rescaleIntercept = 0.0f;
	}
	else
	{
		if (fabs(rescaleIntercept) < 25.0)
			allowdMin = int( rescaleIntercept);
	}
	
	m_mappingScale  =  1.0f;
	m_mappingOffset =  0;
	
	
	int maxVal = (1<<16)-1;
	char id[64];
	_snprintf(id, sizeof id, "%g%g_s%d_%d",iRescaleSlope,rescaleIntercept,isSigned, allowdMin);
	id[sizeof id - 1] = '\0';


	if (strcmp(id, m_CTCLUTID))
	{	
		if (isSigned)
		{
			
			for ( i = 0; i <= maxVal; i++)
			{
				val = int((*(short *)&i) * iRescaleSlope + rescaleIntercept + 0.5f);
				m_CTLUT[i] = (val < allowdMin   ? allowdMin:(val > 4095 ? 4095:val));
			}
		}
		else
		{
			for ( i = 0; i <= maxVal; i++)
			{
				val = int(i * iRescaleSlope + rescaleIntercept + 0.5f);
				m_CTLUT[i] = (val < allowdMin   ? allowdMin:(val > 4095 ? 4095:val));
			}
		}

		strcpy(m_CTCLUTID,id);
	}

	for ( i = 0; i < iNpixels; i++)
	{
		out[i] = m_CTLUT[inOut[i]];
	}

	return 0;

}


//-----------------------------------------------------------------------------
int AppComDataConverter::ConvertToVoxel(unsigned short *inOut, 
									   int iNpixels,
									   float iRescaleSlope, 
									   float iRescaleIntercept,
									   bool iIsLittleEndian, unsigned short *out,
									   int use1024)
{
	// SH, 2009-09-21
	// We have some convertion for CT dataset which does not really
	// provide HU + 1024 values in cache
	// Therefore, we have code in AqDicomBase to compensate for this.
	// If any part of code here is changed, please inform APS developer.
	

	if (inOut == 0 ) return 0;

	float rescaleIntercept =  (iRescaleIntercept + 1024.0f); // from HU to all positive
	short val;
	int i;

	iIsLittleEndian = true; // Merge does the swap
	
	assert(iIsLittleEndian==true);

	assert(use1024 == 0); // not quite tested yet

	if (!out)
		out = inOut;


	// since app may use the offset info from the header, we need to make sure
	// we always use that
	int allowdMin = 0;
	if (!use1024)
	{
		if (fabs(rescaleIntercept) < 25.0)
			rescaleIntercept = 0.0f;
	}
	else
	{
		if (fabs(rescaleIntercept) < 25.0)
			allowdMin = int( rescaleIntercept);
	}
	
	m_mappingScale  =  1.0f;

	// client assumes HU
	m_mappingOffset =  0;//fabs(rescaleIntercept) > 24 ? rescaleIntercept:0;

//	m_mappingOffset = iRescaleIntercept-rescaleIntercept;
	
	if (m_useLUT)
	{
		int maxVal = (1<<16)-1;
		
		for ( i = 0; i <= maxVal; i++)
		{
			val = (short)((*(short *)&i) * iRescaleSlope + rescaleIntercept);
			m_CTLUT[i] = (val < 0   ? 0:(val > 4095 ? 4095:val));
		}
		
		for ( i = 0; i < iNpixels; i++)
		{
			out[i] = m_CTLUT[inOut[i]];
			//			histogram [inOut[i]]++;
		}
	}
	else
	{
		for ( i = 0; i < iNpixels; i++)
		{
			val = (short)((*(short *)&inOut[i]) * iRescaleSlope + rescaleIntercept);
			out[i] = (val < allowdMin ? allowdMin:(val > 4095?4095:val));
			//			histogram [inOut[i]]++;
		}
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
int AppComDataConverter::ConvertPTToVoxel(unsigned short *inOut, 
									      int iNpixels,
									      float iRescaleSlope, 
									      float iRescaleIntercept,
										  float iFactorToMultiply,
									      bool iIsLittleEndian, unsigned short *out)
{
	if (inOut == 0 ) return 0;

	float rescaleIntercept =  (iRescaleIntercept); 
	unsigned short val;
	int i;

	iIsLittleEndian = true; // Merge does the swap
	
	assert(iIsLittleEndian==true);

	if (!out)
		out = inOut;


	m_mappingScale  =  1.0f;
	// client assumes HU
	m_mappingOffset =  0;//fabs(rescaleIntercept) > 24 ? rescaleIntercept:0;

	
	if (m_useLUT)
	{
		int maxVal = (1<<16)-1;
		
		for ( i = 0; i <= maxVal; i++)
		{
			val = (short)(((*(short *)&i) * iRescaleSlope + rescaleIntercept) * iFactorToMultiply);
			
			m_CTLUT[i] = (val < 0   ? 0:(val > 4095 ? 4095:val));
		}
		
		for ( i = 0; i < iNpixels; i++)
		{
			out[i] = m_CTLUT[inOut[i]];
			//			histogram [inOut[i]]++;
		}
	}
	else
	{
		for ( i = 0; i < iNpixels; i++)
		{
			val =  unsigned short((inOut[i] * iRescaleSlope + rescaleIntercept)* iFactorToMultiply);
			out[i] =  (val > 4095?4095:val);
			//			histogram [inOut[i]]++;
		}
	}
	 
	return 0;
}

//-----------------------------------------------------------------------------
int AppComDataConverter::ConvertPTToVoxel(short *inOut, 
									      int iNpixels,
									      float iRescaleSlope, 
									      float iRescaleIntercept,
										  float iFactorToMultiply,
									      bool iIsLittleEndian, unsigned short *out)
{
	if (inOut == 0 ) return 0;

	float rescaleIntercept =  (iRescaleIntercept); 
	short val;
	int i;

	iIsLittleEndian = true; // Merge does the swap
	
	assert(iIsLittleEndian==true);

	if (!out)
		out = (unsigned short *)inOut;

	m_mappingScale  =  1.0f;
	// client assumes HU
	m_mappingOffset =  0;//fabs(rescaleIntercept) > 24 ? rescaleIntercept:0;

	
	if (m_useLUT)
	{
		int maxVal = (1<<16)-1;
		
		for ( i = 0; i < maxVal; i++)
		{
			val = (short)(((*(short *)&i) * iRescaleSlope + rescaleIntercept) * iFactorToMultiply);
			
			m_CTLUT[i] = (val < 0   ? 0:(val > 4095 ? 4095:val));
		}
		
		for ( i = 0; i < iNpixels; i++)
		{
			out[i] = m_CTLUT[inOut[i]];
		}
	}
	else
	{
		for ( i = 0; i < iNpixels; i++)
		{
			val = (short)(((*(short *)&inOut[i]) * iRescaleSlope + rescaleIntercept)* iFactorToMultiply);
			out[i] = (val < 0 ? 0:(val > 4095?4095:val));
		}
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
#include <math.h>
int AppComDataConverter::ConvertCRToVoxel(char* inOut, int iNPixels,
										 float slope, float intercept,
										 bool iIsLittleEndian,
										 int bytesPerPixel,
										 int iUseModalityLUT,
										 unsigned bitsToShift,
										 char* oOut, int* oMin, int* oMax, 
										 int* iWindowWidth, int* iWindowCenter,
										 int iIsSigned) 
{
	
	int useModalityLUT = (iUseModalityLUT &&(fabs(slope-1.0) > 0.001||fabs(intercept) > 0.001));
	int i, mmin = 4095, mmax = -1;
	
	iIsLittleEndian = true; // Merge does the swap

	int iWW = *iWindowWidth;
	int iWL = *iWindowCenter;


	// T.C. Zhao & Rob Lewis 2005.10.27
	// If we take modalityLUT into consideration, we may need to expand the
	// bit depth to hold the physical value. Can't do this for v1.6.1.
	// Will come back and fix it in v1.7. For now, just ignore modality lut if 
	// that's the case

	if (bytesPerPixel == 1 && iUseModalityLUT)
	{
		int ss = int(slope * 255 + intercept);
		if (ss > 255 || ss < 0)
		{
			slope		= 1.0f;
			intercept	= 0.0f;
		}
	}

	if (bytesPerPixel == 2)
	{
		if (!iIsSigned)
		{
			unsigned short* pxls = (unsigned short*) inOut;
			unsigned short* out = (unsigned short *) (oOut ? oOut : inOut);
			int val;
				
			if (useModalityLUT)
			{
				m_mappingScale  = 1.0f;
				m_mappingOffset = 0.0f;
			
				for (i = 0; i < iNPixels; ++i)
				{
					val = int(pxls[i] * slope + intercept);
					val = (val > 4095)  ? 4095 : val;
					out[i] = unsigned short((val < 0) ? 0 : val);
					if (out[i] < mmin) 	mmin = out[i];
					if (out[i] > mmax)  mmax = out[i];
				}
			}
			else	//	Not using modality LUT
			{
				if(bitsToShift > 0)
				{
					m_mappingScale = float (1<<bitsToShift);
					m_mappingOffset = 0.0f;

					for (i = 0; i < iNPixels; i++)
					{
						val = int(pxls[i]) << bitsToShift;
						out[i] = (val > 4095) ? 4095 : val;
						if (out[i] < mmin) 	mmin = out[i];
						if (out[i] > mmax)  mmax = out[i];
					}
				}
				else
				{
					// Murali 2007.02.12 get the dynamic min / max
					FindMinMax(pxls,iNPixels, true, mmin, mmax);	
					
					// find the recommended min / max
					int minvv = iWL - iWW / 2;
					int maxvv = minvv + iWW;

					// try to find the intersection of recommended VOI range with dynamic range
					if ( iWW > 1 )
					{
						mmin  = (minvv > mmin) ? minvv : mmin;
						mmin = (mmin>0) ? mmin : 0;
						mmax = (maxvv < mmax) ? maxvv : mmax;
					}

					RescaleMR (pxls, out, iNPixels, mmin, mmax);			
				}
			}
		}
		else //	Signed
		{
			short* pxls = (short*) inOut;
			short* out = (short *) (oOut ? oOut : inOut);
			int val;

			if (useModalityLUT)
			{
				m_mappingScale  = 1.0f;
				m_mappingOffset = 0.0f;
			
				for (i = 0; i < iNPixels; ++i)
				{
					val = int(pxls[i] * slope + intercept);
					out[i] = short(val > 4095  ? 4095 : val);
					out[i] = (out[i] < 0 ? 0 : out[i]);
					if (out[i] < mmin) 	mmin = out[i];
					if (out[i] > mmax)  mmax = out[i];
				}
			}
			else	// Not using modality LUT
			{				
				if(bitsToShift > 0)
				{
					m_mappingScale = float (1<<bitsToShift);
					m_mappingOffset = 0.0f;

					for (i = 0; i < iNPixels; i++)
					{
						val = pxls[i]<<bitsToShift;
						out[i] = (val > 4095) ? 4095 : val;
						out[i] = (out[i] < 0) ? 0 : out[i];
						if (out[i] < mmin) 	mmin = out[i];
						if (out[i] > mmax)  mmax = out[i];
					}
				}
				else
				{
					// Murali 2007.02.12 get the dynamic min / max
					FindMinMax(pxls, iNPixels, true, mmin, mmax);	

					// find the recommended min / max
					int minvv = iWL - iWW / 2;
					int maxvv = minvv + iWW;

					// try to find the intersection of recommended VOI range with dynamic range
					if ( iWW > 1 )
					{
						mmin  = (minvv > mmin) ? minvv : mmin;
						mmax = (maxvv < mmax) ? maxvv : mmax;
					}
					
					RescaleMR (pxls, out, iNPixels, mmin, mmax);			
				}

			}
		}
	}
	else if (bytesPerPixel == 1)
	{
		if (!iIsSigned)
		{
			unsigned char* pxls = (unsigned char*) inOut;
			unsigned char* out =  (unsigned char *) (oOut ? oOut : inOut);
			int val;
			
			m_mappingScale  = 1.0f ;
			m_mappingOffset = 0.0f;
				
			for (i = 0; i < iNPixels; i++)
			{
				val = int(pxls[i] * slope + intercept);
				val = (val > 255  ? 255: (val < 0 ? 0:val));	
				out[i] = unsigned char(val);
				if (out[i] < mmin) 	mmin = out[i];
				if (out[i] > mmax)  mmax = out[i];
			}
		}
		else	// Signed
		{
			char* pxls = (char*) inOut;
			char* out =  (char *) (oOut ? oOut : inOut);
			int val;
			
			m_mappingScale  = 1.0f ;
			m_mappingOffset = 0.0f;
				
			for (i = 0; i < iNPixels; i++)
			{
				val = int(pxls[i] * slope + intercept);
				val = (val > 255  ? 255: (val < 0 ? 0:val));	
				out[i] = signed char(val);
				if (out[i] < mmin) 	mmin = out[i];
				if (out[i] > mmax)  mmax = out[i];
			}
		}
	}
	else
	{
		return -1;
	}
	
	
	if (oMin) *oMin = mmin;
	if (oMax) *oMax = mmax;
	
	return 0;
}

//-----------------------------------------------------------------------------

int AppComDataConverter::ConvertToVoxel(unsigned short* inOut, int iNPixels,
										unsigned int iBitsToDownShift,
										bool iIsLittleEndian, unsigned short* out)
{
	if (inOut == 0 ) return 0;

	iIsLittleEndian = true; // Merge does the swap

	assert(iIsLittleEndian == true);
	
	if (!out)
		out = inOut;
	
	int i;

	if (iIsLittleEndian)
	{
		
		if (m_knownMax > 10000)
		{
			for (i = 0; i < iNPixels; i++)
			{
				out[i] = (inOut[i] >> 4);
			}
			
			m_mappingScale = 1.0/16.0f;
			m_mappingOffset = 0.0f;
		}
		else
		{		
			for (i = 0; i < iNPixels; i++)
			{
				out[i] = (inOut[i] > 4095)?4095:inOut[i];
			}
			
			m_mappingScale =  1.0f; 
			m_mappingOffset = 0.0f;
		}
	}
	else
	{
		unsigned short tmp;
		if (m_knownMax > 10000)
		{
			for (i = 0; i < iNPixels; i++)
			{
				tmp = iRTVSSwap(inOut[i]);
				out[i] = (tmp >> 4);
			}
			
			m_mappingScale = 1.0/16.0f;
			m_mappingOffset = 0.0f;
		}
		else
		{		
			for (i = 0; i < iNPixels; i++)
			{
				tmp = iRTVSSwap(inOut[i]);
				out[i] = (tmp > 4095)?4095:tmp;
			}
			
			m_mappingScale =  1.0f;
			m_mappingOffset = 0.0f;
		}
	}
	return 0;
	
}

//-----------------------------------------------------------------------------

int AppComDataConverter::ConvertToVoxel(unsigned char* inOut, int iNPixels,
									   unsigned int iBitsToDownShift,
									   bool iIsLittleEndian, unsigned char* out)
{
	if (inOut == 0 ) return 0;

	iIsLittleEndian = true; // Merge does the swap

	
	try
	{
		if (out)
		{
			memcpy(out, inOut, iNPixels);
		}
	}
	catch (...)
	{
		return kpRTVSErrException;
		
	}
	return 0;
	
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

inline bool IsInRange (int iV)
{
	return ((iV >= 0) && (iV < 4096));
}

//-----------------------------------------------------------------------------

void AppComDataConverter::GetMinAndMaxWindow(int  iMinVoxelValue, int  iMaxVoxelValue,
											int& oMinW,          int& oMaxW, bool isMR)
{

	int minW = m_voiWindowCenter - m_voiWindowWidth/2;
	int maxW = minW + m_voiWindowWidth-1;

	int minVoxelValue = iMinVoxelValue;
	int maxVoxelValue = iMaxVoxelValue;

	if (m_knownMin != 0  && m_knownMax != 0)
	{
		minVoxelValue = m_knownMin;
		maxVoxelValue = m_knownMax;
	}


	if (m_voiWindowWidth < 1) // VOI LU Not Present
	{
		minW = minVoxelValue;
		maxW = maxVoxelValue;
	}


	// Check to see if te dataset is MR
	if (isMR)
	{
		if (m_bypassVOILUTForAllMR || (m_bypassVOILUTOnlyForHighContrastMR && m_voiWindowWidth >= m_wwIndicatingHighContrast))
		{
			// Do we need to clamp
			if (m_minWindowToClampTo >= 0)
			{
				minW = (minVoxelValue < m_minWindowToClampTo) ? m_minWindowToClampTo : minVoxelValue;

			}
			else
				minW = minVoxelValue;


			if (m_maxWindowToClampTo >= 2)
			{
				maxW = (maxVoxelValue > m_maxWindowToClampTo) ? m_maxWindowToClampTo: maxVoxelValue;
			}
			else
				maxW = maxVoxelValue;

		}
	}


	oMinW = minW;
	oMaxW = maxW;
}

//-----------------------------------------------------------------------------

int AppComDataConverter::DetermineHowToResamplMRData(int iMinVoxelValue, int iMaxVoxelValue)
{
	int status = kRescaleRange;

	int minW = 0;
	int maxW = 0;


	GetMinAndMaxWindow (iMinVoxelValue, iMaxVoxelValue, minW, maxW, true);

	if (IsInRange (minW) && IsInRange (maxW))
	{
		return kTruncate;
	}
	else
	{
		if ((maxW - minW) > 4095)
		{
			return kRescaleRange;
		}
		else
		{
			return kRemap;
		}
	}

	return kRescaleRange;
}

//-----------------------------------------------------------------------------

int AppComDataConverter::RescaleData(short* iPixels, short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue, bool isMR)
{
	if (iPixels == 0 || oPixels == 0) return 0;

	int iWhatTodo = kRescaleRange;
	int minW = 0;
	int maxW = 0;

	iWhatTodo = DetermineHowToResamplMRData (iMinVoxelValue, iMaxVoxelValue);

	GetMinAndMaxWindow (iMinVoxelValue, iMaxVoxelValue, minW, maxW, isMR);

	// Murali 2006.12.06 This check is added to ensure that the buffer is expanded to a preset dynamic range. 
	if(abs(iMaxVoxelValue - iMinVoxelValue) < kMinMRDynamicRangeWithModalityLUTApplied && isMR)
	{
		float a = 1.0f;
		float b = 1.0f;
		float range  = float(kMRDynamicRangeWithModalityLUT) ;
				
		a = range / (float)(maxW - minW);
		b = -1.0f * a * ((float)iMinVoxelValue);
		
		m_mappingScale = a;
		m_mappingOffset = b;
		short pv = 0;
		
		for (int i = 0; i < iNPixels; i++)
		{
			pv = iPixels[i];
			pv = (pv < minW) ? minW : ((pv > maxW) ? maxW : pv);
			oPixels[i] = short(a*(float)pv + b);
			if (oPixels[i] < 0) oPixels[i] = 0;
		}
	}
	else if (iWhatTodo == kTruncate)
	{
		for (int i = 0; i < iNPixels; i++)
		{
			oPixels[i] = (iPixels[i] < 0 )   ? 0    : iPixels[i];
			oPixels[i] = (oPixels[i] > 4095) ? 4095 : oPixels[i];
		}
	}
	else
	if (iWhatTodo == kRemap)
	{
		for (int i = 0; i < iNPixels; i++)
		{ 
			oPixels[i] = iPixels[i] - minW;
			oPixels[i] = (oPixels[i] < 0 )   ? 0    : oPixels[i];
			oPixels[i] = (oPixels[i] > 4095) ? 4095 : oPixels[i];
		}

		m_mappingOffset = -(float)minW;
		m_mappingScale = 1.0f;
	}
	else
	{
		// a(min) + b = 0
		// a(max) + b = 1<<m_bitsUsedPerPixel) -1;
		//if (minW < 0) minW = 0;

		float a = 1.0f;
		float b = 1.0f;
		float result = 0.0f;
		float range  = float(maxW - minW +1);
		
		if (range > 4095.0f) range = 4095.0f;
		
		a = range / (float)(maxW - minW);
		b = -1.0f * a * ((float)minW);
		
		m_mappingScale = a;
		m_mappingOffset = b;
		short pv = 0;
		

		for (int i = 0; i < iNPixels; i++)
		{
			pv = iPixels[i];
			pv = (pv < minW) ? minW : ((pv > maxW) ? maxW : pv);
			oPixels[i] = short(a*(float)pv + b);

			if (oPixels[i] < 0) oPixels[i] = 0;
		}
	}


	return 0;
}

//-----------------------------------------------------------------------------
int AppComDataConverter::RescaleData(unsigned short* iPixels, unsigned short* oPixels, 
										 int iNPixels,
									     int iMinVoxelValue,
									     int iMaxVoxelValue, bool isMR)
{
	if (iPixels == 0 || oPixels == 0) return 0;

	int iWhatTodo = kRescaleRange;
	int minW = 0;
	int maxW = 0;

	iWhatTodo = DetermineHowToResamplMRData (iMinVoxelValue, iMaxVoxelValue);

	GetMinAndMaxWindow (iMinVoxelValue, iMaxVoxelValue, minW, maxW, isMR);


	// Murali 2006.12.06 This check is added to ensure that the buffer is expanded to a preset dynamic range. 
	if(abs(iMaxVoxelValue - iMinVoxelValue) < kMinMRDynamicRangeWithModalityLUTApplied && isMR)
	{
		float a = 1.0f;
		float b = 1.0f;
		float range  = float(kMRDynamicRangeWithModalityLUT) ;
				
		a = range / (float)(maxW - minW);
		b = -1.0f * a * ((float)iMinVoxelValue);
		
		m_mappingScale = a;
		m_mappingOffset = b;
		unsigned short pv = 0;
		
		for (int i = 0; i < iNPixels; i++)
		{
			pv = iPixels[i];
			pv = (pv < minW) ? minW : ((pv > maxW) ? maxW : pv);
			oPixels[i] = unsigned short(a*(float)pv + b);
			if (oPixels[i] < 0) oPixels[i] = 0;
		}
	}
	else if (iWhatTodo == kTruncate)
	{
		for (int i = 0; i < iNPixels; i++)
		{ 
			oPixels[i] = iPixels[i] & 0x0fff;
		}
	}
	else
	if (iWhatTodo == kRemap)
	{
		// 2007.08.01 kunikichi, old change offset procedure was worng
		int pv;
		for (int i = 0; i < iNPixels; i++)
		{ 
			pv = oPixels[i] - minW;
			pv = (pv < 0) ? 0 : ((pv > 4095) ? 4095 : pv);
			oPixels[i] = pv;
		}

		m_mappingOffset = -(float)minW;
		m_mappingScale = 1.0f;
	}
	else
	{
		// a(min) + b = 0
		// a(max) + b = 1<<m_bitsUsedPerPixel) -1;
		float a = 1.0f;
		float b = 1.0f;
		float result = 0.0f;
		float range  = maxW - minW +1.0f;
		
		if (range > 4095.0f) range = 4095.0f;
		
		a = range / (float)(maxW - minW);
		b = -1.0f * a * ((float)minW);
		
		m_mappingScale	= a;
		m_mappingOffset = b;
		unsigned short pv;
		for (int i = 0; i < iNPixels; i++)
		{
			pv = iPixels[i];
			pv = (pv < minW) ? minW : ((pv > maxW) ? maxW : pv);
			oPixels[i] = unsigned short( a*(float)iPixels[i] + b);
		}
	}


	return 0;
}

//-----------------------------------------------------------------------------

int AppComDataConverter::RescaleMR(short* iPixels, short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue)
{
	return RescaleData(iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue, true);

}

//-----------------------------------------------------------------------------
int AppComDataConverter::RescaleMR(unsigned short* iPixels, unsigned short* oPixels, 
										 int iNPixels,
									     int iMinVoxelValue,
									     int iMaxVoxelValue)
{
	
	return RescaleData(iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue, true);

}

//-----------------------------------------------------------------------------
int AppComDataConverter::RescalePT(unsigned short* iPixels, unsigned short* oPixels, 
										 int iNPixels,
									     int iMinVoxelValue,
									     int iMaxVoxelValue)
{
	
	// these are all hacks
	int range = (iMaxVoxelValue - iMinVoxelValue);
	if ((m_voiWindowWidth > range || range > 3* m_voiWindowWidth))
	{
		m_voiWindowCenter =  iMinVoxelValue + range/2;
		m_voiWindowWidth = range;
	}
	
	return RescaleData(iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue, false);

}

//-----------------------------------------------------------------------------
int AppComDataConverter::RescalePT(short* iPixels, short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue)
{
	// these are all hacks
	int range = (iMaxVoxelValue - iMinVoxelValue);
	if ( (m_voiWindowWidth > range || range > 3* m_voiWindowWidth))
	{
		m_voiWindowCenter =  iMinVoxelValue + range/2;
		m_voiWindowWidth = range;
	}

	return RescaleData(iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue, false);

}


//-----------------------------------------------------------------------------
int AppComDataConverter::RescaleNM(unsigned short* iPixels, unsigned short* oPixels, 
										 int iNPixels,
									     int iMinVoxelValue,
									     int iMaxVoxelValue)
{
	
	// Fix to solve the bug (ID 7864) in that the data above 4095 was truncated. Murali/Vikram 2007.01.23
	// This fix forces to rescale the data into 0 - 4095 range instead of truncation.
	int range = abs(iMaxVoxelValue - iMinVoxelValue);
	if (m_voiWindowWidth <= 4095 && range >= 4095)
	{
		m_voiWindowCenter	=  iMinVoxelValue + range/2;
		m_voiWindowWidth	=  range;
	}

	return RescaleData(iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue, false);

}

//-----------------------------------------------------------------------------

int AppComDataConverter::RescaleDX(unsigned short* iPixels, unsigned short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue)
{

	return RescaleData(iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue);
 
}

//-----------------------------------------------------------------------------
int AppComDataConverter::RescaleDX(short* iPixels, short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue)
{

	return RescaleMR (iPixels, oPixels, iNPixels, iMinVoxelValue, iMaxVoxelValue);
}

//-----------------------------------------------------------------------------
// This Function Interpolates the images.
void AppComDataConverter::ShrinkImageBy2(unsigned char* ioPixels, int iBytesPerPixel, 
									   int iWidth, int iHeight, 
									   unsigned short& oWidth, unsigned short& oHeight)
{
	// Make sure that the data is not null
	if (ioPixels == 0) return;

	int w = iWidth;
	int h = iHeight;
	

	oWidth  = w;
	oHeight = h;

	if (iBytesPerPixel > 4) return;



	int wBy2 = w/2;
	int hBy2 = h/2;

	oWidth  = wBy2;
	oHeight = hBy2;



	int y = 0, y2 = 0;
	int x = 0, x2 = 0;
	
	int y2W = 0;
	int y2PlusOneW = 0, yWBy2 = 0;

	if (iBytesPerPixel == 4 || iBytesPerPixel == 3)
	{
		unsigned char* src = (unsigned char*)ioPixels;
		unsigned char* dst = (unsigned char*)ioPixels;


		for (y = 0; y < hBy2; y++) 
		{
			y2         = y << 1;
			y2W        = y2*w*iBytesPerPixel;
			y2PlusOneW = y2W + w*iBytesPerPixel;
			yWBy2      = y*wBy2*iBytesPerPixel;

			for (x = 0; x < wBy2*iBytesPerPixel; x+=iBytesPerPixel) 
			{
				x2 = x << 1;
				
				// Green
				dst[0] = (src[y2W        + x2]     + src[y2W        + x2 + iBytesPerPixel] +
						  src[y2PlusOneW + x2]     + src[y2PlusOneW + x2 + iBytesPerPixel]) >> 2;
				// Blue
				dst[1] = (src[y2W        + x2 + 1] + src[y2W        + x2  + iBytesPerPixel+1] +
						  src[y2PlusOneW + x2 + 1] + src[y2PlusOneW + x2  + iBytesPerPixel+1]) >> 2;
				// Red
				dst[2] = (src[y2W        + x2 + 2] + src[y2W        + x2 + iBytesPerPixel+2] +
						  src[y2PlusOneW + x2 + 2] + src[y2PlusOneW + x2 + iBytesPerPixel+2]) >> 2;

				dst += 3;
			} 
		} 
	}
	else
	if (iBytesPerPixel == 2)
	{
		unsigned short* src = (unsigned short*)ioPixels;
		unsigned short* dst = (unsigned short*)ioPixels;


		for (y = 0; y < hBy2; y++) 
		{
			y2         = y<<1;
			y2W        = y2*w;
			y2PlusOneW = y2W + w;
			yWBy2      = y*wBy2;

			for (x = 0; x < wBy2; x++) 
			{
				x2 = x<<1;
				dst[yWBy2 + x] = (src[y2W + x2]        + src[y2W + x2 + 1] +
								  src[y2PlusOneW + x2] + src[y2PlusOneW + x2 + 1]) >> 2;
			} 
		} 
	}
	else
	if (iBytesPerPixel == 1)
	{
		unsigned char* src = (unsigned char*)ioPixels;
		unsigned char* dst = (unsigned char*)ioPixels;


		for (y = 0; y < hBy2; y++) 
		{
			y2         = y<<1;
			y2W        = y2*w;
			y2PlusOneW = y2W + w;
			yWBy2      = y*wBy2;

			for (x = 0; x < wBy2; x++) 
			{
				x2 = x<<1;
				dst[yWBy2 + x] = (src[y2W + x2]        + src[y2W + x2 + 1] +
								  src[y2PlusOneW + x2] + src[y2PlusOneW + x2 + 1]) >> 2;
			} 
		} 
	}

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This Function skips pixels to shrink the image
void AppComDataConverter::ShrinkImageBy2SkipPixels(unsigned char* ioPixels, int iBytesPerPixel, 
									              int iWidth, int iHeight, 
									              unsigned short& oWidth, unsigned short& oHeight)
{
	// Make sure that the data is not null
	if (ioPixels == 0) return;

	int w = iWidth;
	int h = iHeight;
	

	oWidth  = w;
	oHeight = h;

	if (iBytesPerPixel > 4) return;



	int wBy2 = w/2;
	int hBy2 = h/2;

	oWidth  = wBy2;
	oHeight = hBy2;



	int y = 0, y2 = 0;
	int x = 0, x2 = 0;
	
	int y2W = 0;
	int y2PlusOneW = 0, yWBy2 = 0;

	if (iBytesPerPixel == 4 || iBytesPerPixel == 3)
	{
		unsigned char* src = (unsigned char*)ioPixels;
		unsigned char* dst = (unsigned char*)ioPixels;


		for (y = 0; y < hBy2; y++) 
		{
			y2         = y << 1;
			y2W        = y2*w*iBytesPerPixel;
			
			for (x = 0; x < wBy2*iBytesPerPixel; x+=iBytesPerPixel) 
			{
				x2 = x << 1;
				
				// Green
				dst[0] = src[y2W + x2];
				// Blue
				dst[1] = src[y2W + x2 + 1];
				// Red
				dst[2] = src[y2W + x2 + 2];

				dst += 3;
			} 
		} 
	}
	else
	if (iBytesPerPixel == 2)
	{
		unsigned short* src = (unsigned short*)ioPixels;
		unsigned short* dst = (unsigned short*)ioPixels;


		for (y = 0; y < hBy2; y++) 
		{
			y2         = y<<1;
			y2W        = y2*w;
			
			for (x = 0; x < wBy2; x++) 
			{
				x2 = x<<1;
				*dst++ = src[y2W + x2];
;
			} 
		} 
	}
	else
	if (iBytesPerPixel == 1)
	{
		unsigned char* src = (unsigned char*)ioPixels;
		unsigned char* dst = (unsigned char*)ioPixels;


		for (y = 0; y < hBy2; y++) 
		{
			y2         = y << 1;
			y2W        = y2*w;
			
			for (x = 0; x < wBy2; x++) 
			{
				x2 = x<<1;
				*dst++ = src[y2W + x2];
			} 
		} 
	}

}
//-----------------------------------------------------------------------------

void AppComDataConverter::ShrinkImageBySkippingPixels(unsigned char* ioPixels, int iBytesPerPixel, 
													  int iWidth, int iHeight, 
													  unsigned short& oWidth, 
													  unsigned short& oHeight, float fScaleRatio)
{

	try
	{

		// Make sure that the data is not null and the target image size is not 0!
		if (!ioPixels || iBytesPerPixel < 1 || iBytesPerPixel > 4 || !oWidth || !oHeight) 
		{
			return;
		}

		int w = iWidth;
		int h = iHeight;

		//	LARGE_INTEGER nStartTime, nEndTime, nCountsPerSecond;
		//	QueryPerformanceFrequency((LARGE_INTEGER*)&nCountsPerSecond);		
		//	QueryPerformanceCounter((LARGE_INTEGER*)&nStartTime);	

		// compute the scale ratio only if not passed already. 
		// Also same scaleratio will be used for row/columns to preserve the aspect ratio. - Murali
		if(fScaleRatio <= 0.0)
		{
			// choose the min src image dimension and its corresponding output image size 
			// for computing the scale ratio - Murali.
			float fSrcDimMin	= (float) (iWidth < iHeight ? iWidth : iHeight);
			float fDenominator	= (fSrcDimMin == iWidth) ? oWidth : oHeight;
			fScaleRatio = fSrcDimMin/fDenominator;
		}

		int wDst = (int)(w/fScaleRatio);
		int hDst = (int)(h/fScaleRatio);

		// update the target image width and height
		oWidth  = wDst;
		oHeight = hDst;

		int y = 0, y2 = 0;
		int x = 0, x2 = 0;
	
		int* xLut = new int[wDst];
		int* yLut = new int[hDst];
		if(!xLut || !yLut)
		{
			delete [] xLut; xLut = 0;
			delete [] yLut; yLut = 0;
			printf("AppComDataConverter::ShrinkImageBySkippingPixels: allocation failed! \n");
			return;
		}

		//Create a ylut for the new image coordinates
		for (y = 0; y < hDst; y++) 
		{
			yLut[y]  = int(y * fScaleRatio)*w*iBytesPerPixel; 
		}

		//Create a xlut for the new image coordinates
		for (x = 0; x < wDst; x++) 
		{
			xLut[x] = int(x * fScaleRatio)* iBytesPerPixel;
		}

		unsigned char* src = (unsigned char*)ioPixels;
		unsigned char* dst = (unsigned char*)ioPixels;

		if (iBytesPerPixel == 4 || iBytesPerPixel == 3)
		{
			for (y = 0; y < hDst; y++) 
			{
				y2 = yLut[y];			
				for (x = 0; x < wDst; x++) 
				{
					x2 = xLut[x];	

					// Green
					dst[0] = src[y2 + x2];
					// Blue
					dst[1] = src[y2 + x2 + 1];
					// Red
					dst[2] = src[y2 + x2 + 2];

					dst += 3;
				} 
			} 
		}
		else
		if (iBytesPerPixel == 2)
		{
			for (y = 0; y < hDst; y++) 
			{
				y2 = yLut[y];									
				for (x = 0; x < wDst; x++) 
				{
					x2 = xLut[x];	
					dst[0] = src[y2 + x2];
					dst[1] = src[y2 + x2 + 1];
					dst += 2;
				} 
			} 
		}
		else
		if (iBytesPerPixel == 1)
		{
			for (y = 0; y < hDst; y++) 
			{
				y2 = yLut[y];						
				for (x = 0; x < wDst; x++) 
				{
					x2 = xLut[x];	
					*dst++ = src[y2 + x2];
				} 
			} 
		}

	//	QueryPerformanceCounter((LARGE_INTEGER*)&nEndTime);
	//	long double nTimeTaken = (long double)(((nEndTime.QuadPart - nStartTime.QuadPart) * 1000)) / (long double) (nCountsPerSecond.QuadPart);
	//	printf("Data Conversion Time Taken - %f ms. \n", nTimeTaken);	

		if(xLut)
		{ 
			delete [] xLut; xLut = 0; 
		}

		if(yLut)
		{   
			delete [] yLut; yLut = 0; 
		}
	}
	catch(...)
	{
		printf("AppComDataConverter::ShrinkImageBySkippingPixels: threw exception! \n");
	}
}

//-----------------------------------------------------------------------------
void AppComDataConverter::Convert32To24Bit(unsigned char* ioPixels, int iBytesPerPixel, 
									              int iWidth, int iHeight)
{
	if (iBytesPerPixel != 4) return;

	unsigned char* src = (unsigned char*)ioPixels;
	unsigned char* dst = (unsigned char*)ioPixels;
	
	int totalNumberOfbytes = iHeight * iWidth * iBytesPerPixel;
	
	for (int x = 0; x < totalNumberOfbytes; x +=iBytesPerPixel) 
	{				
		// Green
		dst[0] = src[x];
		// Blue
		dst[1] = src[x + 1];
		// Red
		dst[2] = src[x + 2];
		
		dst += 3;
	} 
					
}
//-----------------------------------------------------------------------------