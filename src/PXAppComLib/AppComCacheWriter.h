/***********************************************************************
 * AppComCacheWriter.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used to create the Terarecon Cache for incoming 
 *		DICOM Series.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

//-----------------------------------------------------------------------------

#if !defined(APPCOM_CACHEWRITER_H)
#define APPCOM_CACHEWRITER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include <vector>
#include <strstream>

//#include <strstrea.h>

#include "NMObject.h"
#include "TICache.h"
#include "CCacheWriter.h"
#include "PETObjectAttributes.h"

using namespace std ;
class AppComDataConverter;
class VLIDicomPalette;
//-----------------------------------------------------------------------------

class CPxDicomImage;
class AppComCacheWriter : public CCacheWriter
{
	public:
		// cache can be closed by anyone just by location
		static int CloseCache(const char* iCacheLocation);
		static bool HasCache(const char* iCacheLocation);
		//instance feature
		AppComCacheWriter();
		virtual ~AppComCacheWriter();

		virtual bool		ReserveSpace();
		static void			SetUseModalityLUT(int iYesNo);
		void SetDataFileSize(int iSizeInKBytes) {m_dataFileSize = iSizeInKBytes *1024;}
		//---------------------------------------------------------------------
		// Add a single CT image to Cache
		int AddCTImage (const char* iCacheLocation,
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
						
						int iVOIWindowWidth,
					    int iVOIWIndowCenter,
						// -- 20056.05.05
						const char* imageDate,
						const char* imageTime,
						const char* scanOptions,
						const char* manufacturer); // Murali 2007.01.03;

		//---------------------------------------------------------------------
		// Add a single MR Image to cache
		int AddMRImage (const char* iCacheLocation,
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

						int	   iImageNumber,
						int	 iPixelRepresentation, // Vikram 09/05/02 - Need to deal with signed data

						unsigned char*     iPixels,
						int&    ioMinPixelValue,  // -- 08/04/2002 to deal with hi-contrast MR
						int&    ioMaxPixelValue,	// -- 08/04/2002 to deal with hi-contrast MR
						unsigned int	   iSizeOfCompressedData,
						eCompressionMethod iCompressionMethod,
						int iVOIWindowWidth,
					    int iVOIWIndowCenter,
						double slope,
						double intercept,
						const char* imageDate,
						const char* imageTime);


		//---------------------------------------------------------------------
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
												  int iInstanceNumber, // -- 2005.06.10 need this for OT etc
												  int iPixelRepresentation, 
												  unsigned char*     iPixels,
												
												  ePhotometricInterpretation iPhotometricInterpretation,

												  unsigned int		 iSizeOfCompressedData,
												  eCompressionMethod iCompressionMethod,
												
												  double rescaleSlope, 
												  double rescaleIntercept,

 												  double iPixelSpacing[2],
												  int    iWindowWidth,
												  int    iWindowCenter);



		//---------------------------------------------------------------------
		// Add a single XA/XF Image to cache
		int AddXAImage (const char* iCacheLocation,
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

						int iInstanceNumber, // -- 2005.06.10 need this for RF etc
						int iPixelRepresentation,	
						unsigned char*     iPixels,

						ePhotometricInterpretation iPhotometricInterpretation,
						const char*		   iReferencedSOPInstanceUID,
						// -- 2004/03/24 Added VOILUT info
						int    iWindowWidth,
						int    iWindowCenter,
						unsigned int	   iSizeOfCompressedData = 0,
						eCompressionMethod iCompressionMethod = kNone,
						/* added 03/14/2003 modalityLUT for CR */
						double rescaleSlope=1.0,
						double rescaleIntercept=0.0);	

		//---------------------------------------------------------------------
		// Add a single SC Image to cache
		int AddSCImage (const char* iCacheLocation,
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
						ePlanarConfiguration iPlanarConfiguration ,
						eCompressionMethod iCompressionMethod,
						int iVOIWindowWidth,
					    int iVOIWIndowCenter,
						// -- needs W/L and InstanceNumber
						int iInstanceNumber);	

		//---------------------------------------------------------------------
		// Add a single US Image to cache
		int AddUSImage (const char* iCacheLocation,
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
						ePlanarConfiguration iPlanarConfiguration = kRGBRGB,
						eCompressionMethod iCompressionMethod = kNone,
						int iVOIWindowWidth = 0,
					    int iVOIWIndowCenter = 0,
						PxDicomPalette* palette=0);	

		//---------------------------------------------------------------------
		// Add a single PT Image to cache
	
	 
		
		int AddPTImage (const char* iCacheLocation,
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
						unsigned int	   iSizeOfCompressedData ,
						eCompressionMethod iCompressionMethod,
						double iVOIWindowWidth ,
					    double iVOIWIndowCenter,
						// -- 2006.05 for Cardiac
						const char* imageDate,
						const char* imageTime,
						// -- 2006.06.10 for SUV
						PETObjectAttributes &iAttrib);

#if 0
		//---------------------------------------------------------------------
		// Add a single NM Image to cache
		int AddNMImage (const char* iCacheLocation,
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
						int	 iImageNumber,
						int	 iPixelRepresentation, // Vikram 09/05/02 - Need to deal with signed data

						unsigned char*     iPixels,
						unsigned int	   iSizeOfCompressedData  ,
						eCompressionMethod iCompressionMethod ,
						double iVOIWindowWidth ,
					    double iVOIWIndowCenter  );
#endif

		// 08/06/2002 --
		// Abstract out CT/MR data conversions
		AppComDataConverter*	SetDataConverter(AppComDataConverter* iConverter, int isExternal=1);

		//---------------------------------------------------------------------


	private:

		// -- 04-AUG-2002
		// changed all inline data conversion to object AppComDataConverter
		AppComDataConverter*	m_dataConverter;
		int					m_externalConverter;
		unsigned _int64	m_dataFileSize;

		int SetCache(const char* iCacheLocation, bool big=false);
		int PushCommonAttributes (ostrstream& ost,
								  const char* iModality,
								  const char* iSOPInstanceUID,
                                  const char* iSOPClassUID,
								  int iBitsAllocated,
								  int iBitsStored,
								  int iHighBit,
								  int iSizeX, 
								  int iSizeY,  
								  unsigned long iStartOfDataInDataFile,
								  unsigned long iSizeOfDataInBytes,
								  std::vector <std::string>& iImageTypeTokens,
								  eCompressionMethod iCompressionMethod,
								  double iWindowWidth = 0,
								  double iWindowCenter = 0);
		
		int PushCTMRCommonAttributes (ostrstream& ost,
									  double iImagePosition[3], 
									  double iImageOrientation[6], 
									  double iPixelSpacing[2],
									  double iSliceThickness,
									  unsigned int iSizeOfHistogram,
									  unsigned int iHistogram[4096],
									  int iPixelRepresentation,
									  bool iHasValidOrientation = true);
		
		int PushNMAttributes (ostrstream& ost, int iFrameNumber, CNMObject& iNMObject);

		int WriteData (unsigned char* iPixels, unsigned int iNumberOfBytes, ULONGLONG& oFilePos);
		int WriteDesc (ostrstream& ost);
		int InterleaveColorPlanes(unsigned char* ioPixels, int iRows, int iColumns);

		//---------------------------------------------------------------------
		//---------------------------------------------------------------------
		// Swap a 2-byte entity 
		template <class T>  T swap16(T in)
		{
			_ASSERTE(sizeof(in) == 2);
			return in ? ((in & 0xff) << 8) | ((in >> 8) & 0xff) : 0;
		}

		//---------------------------------------------------------------------
		// Swap a 4-byte quantity. Should work for both ints and floats 
		template <class T>  T swap32(T iNum)
		{
			_ASSERTE(sizeof(iNum) == 4);
			uint32 in = *(uint32 *)&iNum;

			in =  ((in >> 24 ) & 0x000000ff) |
				  ((in >> 8  ) & 0x0000ff00) |
				  ((in << 8  ) & 0x00ff0000) |
				  ((in << 24));
			return *(T *)&in;
		}
		

		//---------------------------------------------------------------------
		// Clamp the a to be between imin and imax
		template <class T> T clamp(T a, T imin, T imax)
		{
			return a < imin ? imin: ( a > imax ? imax:a);
		}

		static int	m_useModalityLUTForCR;
};
//-----------------------------------------------------------------------------

#endif // !defined(APPCOM_CACHEWRITER_H)
