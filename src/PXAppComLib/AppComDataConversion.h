/***********************************************************************
 *---------------------------------------------------------------------
 *	
 *
 *	PURPOSE:
 *	  Convert CT/MR data into voxels
 *
 *  
 *-------------------------------------------------------------------
 */

#ifndef APPCOM_DATACONVERSION_H_
#define APPCOM_DATACONVERSION_H_

#include "rtvsswap.h"

#ifndef RTVSUTIL_H_
#include "rtvsutil.h"
#endif

// Murali 2006.12.06 If the MR dynamic range is less than 150, it will be remapped to 500.
const int kMinMRDynamicRangeWithModalityLUTApplied		= 150;
const int kMRDynamicRangeWithModalityLUT				= 500;

//-----------------------------------------------------------------------------
class AppComDataConverter
{
	public:
		AppComDataConverter(void);
		virtual ~AppComDataConverter(void);

		/* 
		 * Convert data to voxels (0-4095)
		 * If out==0, the conversion is done in place
		 */

		virtual int ConvertCTToVoxel(unsigned short *inOut, 
								   int iNpixels,int isSigned,
								   float iRescaleSlope, 
								   float iRescaleIntercept,
								   unsigned short *out=0);


		virtual int ConvertToVoxel(unsigned short *inOut,  int iNpixels,
						   float iRescaleSlope,  float iRescaleIntercept,
						   bool iIsLittleEndian=true, unsigned short* out=0, int use1024=0);
		
		virtual int	ConvertToVoxel(unsigned short* inOut, int NPixels,
						   unsigned int iBitsToDownShift=0,
						   bool iIsLittleEndian=true,unsigned short* out=0);
		
		
		virtual int	ConvertToVoxel(unsigned char* inOut, int NPixels,
						           unsigned int iBitsToDownShift=0,
						           bool iIsLittleEndian=true,unsigned char* out=0);

		virtual int ConvertPTToVoxel(unsigned short *inOut, 
									 int iNpixels,
									 float iRescaleSlope, 
									 float iRescaleIntercept,
									 float iFactorToMultiply,
									 bool iIsLittleEndian, unsigned short *out=0);

		virtual int ConvertPTToVoxel(short *inOut, 
									 int iNpixels,
									 float iRescaleSlope, 
									 float iRescaleIntercept,
									 float iFactorToMultiply,
									 bool iIsLittleEndian, unsigned short *out=0);


		virtual int ConvertCRToVoxel(char* inOut, int iNPixels,
									 float slope, float intercept,
									 bool iIsLittleEndian,
									 int bytesPerPixel,
									 int useModalityLUT, unsigned bitsToShift=0, char* iOut=0,
									 int* oMin=0, int* oMax=0,
									 int* iWindowWidth = 0, 
									 int* iWindowCenter = 0,
									 int iIsSigned = 0);

		
		template <class T> int FindMinMax(T* ioData, int nPixels, bool iIsLittleEndian,  int& oMin, int& oMax)
		{
			
			if (ioData == 0 ) return -1;
			int i = 0;
			
			iIsLittleEndian = true; // Merge does the swap
			
			if (!iIsLittleEndian)
			{
				for ( i = 0; i < nPixels; i++)
					ioData[i] = iRTVSSwap(ioData[i]);
			}
			
			oMin = oMax = ioData[0];
			
			for (i = 1; i < nPixels; i++)
			{
				if (ioData[i] > oMax) oMax = ioData[i];
				if (ioData[i] < oMin) oMin = ioData[i];
			}
			
			return 0;
		}

		//-------------------------------------------------------------------
		template <class T> void ReplaceNegative(T* ioData, int nPixels, int iReplaceVal=0)
		{
			for (int i = 1; i < nPixels; ++i)
			{
				if (ioData[i] < 0) ioData[i] = T(iReplaceVal);
			}
		}
		
		virtual int RescaleDX(unsigned short* iPixels, unsigned short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue);

		virtual int RescaleDX(short* iPixels, short* oPixels, 
								  int iNPixels,
								  int iMinVoxelValue,
								  int iMaxVoxelValue);


		// Vikram 03/01/2004
		// Dr. Vlymen's problem where we are rescaling the high contrast MR data wrong.

		static void SetBypassVOILUTForAllMR (bool iF, int iMinWindowToClampTo, int iMaxWindowToClampTo)
		{
			m_bypassVOILUTForAllMR = iF;

			m_minWindowToClampTo = iMinWindowToClampTo;
			m_maxWindowToClampTo = iMaxWindowToClampTo;
		}

		static void SetBypassVOILUTForHighContrastMR (bool iF, int iMinWindowToClampTo, int iMaxWindowToClampTo, int iWWIndicatingHighContrast=5000)
		{
			m_bypassVOILUTOnlyForHighContrastMR = iF;

			m_minWindowToClampTo = iMinWindowToClampTo;
			m_maxWindowToClampTo = iMaxWindowToClampTo;
			m_wwIndicatingHighContrast = iWWIndicatingHighContrast;
		}



		static bool GetBypassVOILUTForAllMR () { return m_bypassVOILUTForAllMR; }
		static bool GetBypassVOILUTForHighContrastMR () { return m_bypassVOILUTOnlyForHighContrastMR; }

		static int  GetMinWindowToClampTo () { return m_minWindowToClampTo; }
		static int  GetMaxWindowToClampTo () { return m_maxWindowToClampTo; }




		virtual int RescaleMR(short* iPixels, short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue);

		virtual int RescaleMR(unsigned short* iPixels, unsigned short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue);


		virtual int RescalePT(short* iPixels, short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue);

		virtual int RescalePT(unsigned short* iPixels, unsigned short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue);

		virtual int RescaleNM(unsigned short* iPixels, unsigned short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue);

		void	GetVoxelToPixelMapping(float& oScale, float& oOffset) const
		{
			oScale = 1.0f/m_mappingScale;
			oOffset = -m_mappingOffset;
		}

		void   GetPixelToVoxelMapping(float& oScale, float& oOffset) const
		{
			oScale	= m_mappingScale;
			oOffset = m_mappingOffset;
		
		}

		void SetKnownMinMax(int iMin, int iMax)
		{
			m_knownMin = iMin;
			m_knownMax = iMax;
		}

		void GetKnownMinMax(int& oMin, int& oMax) const
		{
			oMin = m_knownMin;
			oMax = m_knownMax;
		}

		// VOI LUT values
		void SetVOIWindowWidth(int iW)
		{
			m_voiWindowWidth = iW;
		}

		void SetVOIWindowCenter(int iL)
		{
			m_voiWindowCenter = iL;
		}


		int GetVOIWindowWidth()
		{
			return m_voiWindowWidth;
		}

		int GetVOIWindowCenter()
		{
			return m_voiWindowCenter;
		}
		
		


		void ShrinkImageBy2(unsigned char* ioPixels, int iBytesPerPixel, 
						   int iWidth, int iHeight, 
						   unsigned short& oWidth, unsigned short& oHeight);

		void ShrinkImageBy2SkipPixels(unsigned char* ioPixels, int iBytesPerPixel, 
						              int iWidth, int iHeight, 
						              unsigned short& oWidth, unsigned short& oHeight);

		/// Shrinks the given image to the desired target image size (oWidth/oHeight).
		// The target image width/height is returned in the oWidth/oHeight parameters.
		// In addition, the user can control the target image size by specifying the scaleRatio.
		// If scaleRatio is not set, it will be computed internally. The smallest src image dimension
		// and the corresponding dst image size will be used for scale ratio computation.
		void ShrinkImageBySkippingPixels(unsigned char* ioPixels, 
											int iBytesPerPixel, 
											int iWidth, int iHeight, 
											unsigned short& oWidth, 
											unsigned short& oHeight,
											float fScaleRatio = 0.0);

		void Convert32To24Bit(unsigned char* ioPixels, int iBytesPerPixel, 
									              int iWidth, int iHeight);

		// T.C. Zhao 2006.03.17
		template <class T> static void ProcessMRModalityLUT(T* ioData, int iSize,
													 float iSlope, float iIntercept,
													 int& oVmin, int& oVmax)
		{
			int hasLUT = (fabs(iSlope - 1.0) > 0.001 || fabs(iIntercept) > 0.01);

			if (!m_useMRModalityLUT || !hasLUT)
				return;

			oVmin = oVmax = 0;
			for ( int i = 0; i < iSize; ++i)
			{
				ioData[i] = T(ioData[i] * iSlope + iIntercept);
				if (ioData[i] < 0)
					ioData[i] = 0;

				if (ioData[i] < oVmin)
					oVmin = ioData[i];
				else if (ioData[i] > oVmax)
					oVmax = ioData[i];
			}
		}

		
		static void SetUseModalityLUTForMR(int iYN)
		{
			m_useMRModalityLUT = iYN;
		}

		/* this is the mapping that maps the 
		 * pixel value to voxel value (0 to 4095)
		 */
		float		m_mappingScale;
		float		m_mappingOffset;

		static int	m_useMRModalityLUT;

	private:
		
		/*
		 * known pixel (min,max)
		 */
		int			m_knownMin;
		int			m_knownMax;

		int			m_voiWindowWidth;
		int			m_voiWindowCenter;
		/*
		 * other stuff
		 */
		int			m_useLUT;
		short		m_CTLUT[(1<<16)];
		char		m_CTCLUTID[64];

		enum eDataHandleType
		{
			kTruncate,       // Ignore any values below zero and > 4095
			kRescaleRange,   // Rescale the data to go from 0 - 4095
			kRemap,			 // Remap (with scale and offset) to fit 0 - 4095
			kUnknown
		};

		
		// Vikram 03/01/2004
		// Dr. Vlymen's prblem where we are rescaling the high contrast MR data wrong.
		static bool m_bypassVOILUTForAllMR;
		static bool m_bypassVOILUTOnlyForHighContrastMR;

		static int m_minWindowToClampTo;
		static int m_maxWindowToClampTo;
		static int	m_wwIndicatingHighContrast;


		virtual int RescaleData(short* iPixels, short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue, bool IsMR=false);

		virtual int RescaleData(unsigned short* iPixels, unsigned short* oPixels, 
							  int iNPixels,
							  int iMinVoxelValue,
							  int iMaxVoxelValue, bool IsMR=false);

		int DetermineHowToResamplMRData( int iMinVoxelValue, int iMaxVoxelValue);

		void GetMinAndMaxWindow(int  iMinVoxelValue, int  iMaxVoxelValue,
								int& oMinW,          int& oMaxW, bool isMR);


};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif  /* !def APPCOM_DATACONVERSION_H_ */
