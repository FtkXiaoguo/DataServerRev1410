/***********************************************************************
 * Conversion.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Handles Pixel Data Conversion
 *
 *
 *
 *-------------------------------------------------------------------
 */


//Chetan
//Code review changes - Gray, Oct - 05
//***********************************************************************************
//need to make sure memory deallocation is done by the compression, and should
//not rely on the caller to do this
//need to change some function definitions and the way they access member variables
//************************************************************************************

#include "Conversion.h"


#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "PxDicomimage.h"

#include <assert.h>
#include "AqCore/TRCriticalsection.h"

#ifdef _PROFILE
#include "scopetimer.h"
extern iRTVBase gLogger;
#endif

const char* cnvErrStr[] = 
{
	"Successful operation",
	"UnsupportedCompressionMethod",
	"InvalidArguments",
	"InvalidFormat",
	"MustCallSetParameters",
	"BadItemTag",
	"MissingSequenceDelimiter",
	"MemoryAllocationFailed",
	"UnsupportedBitDepth",
	"NotImplemented",
};

std::string Conversion::ErrorString(int iErrorcode)
{
	int err = iErrorcode;

	if (err >= 0)
	{
		return cnvErrStr[err];
	}

	return "Unknown error";
}

//-----------------------------------------------------------------------------
//
Conversion::Conversion()
{
}

#include <fstream>
using namespace std;
//-----------------------------------------------------------------------------
//
int Conversion::ConvertYBR_FULLToRGB(CPxDicomImage* iImage)
{ 
	if ( !iImage || !iImage->GetFrame(0) || iImage->GetSamplesPerPixel() != 3 )
		return kCnvInvalidFormat;

	for ( int frmCnt = 0; frmCnt < iImage->GetNumberOfFrames(); frmCnt++ )
	{
		unsigned char* pix = iImage->GetFrame(frmCnt);

		int pixSize = iImage->GetNumberOfRows() * iImage->GetNumberOfColumns();
		
		const int R = 0;
		const int G = 1;
		const int B = 2;

		for ( int i = 0; i < pixSize; i++)
		{
			int Y  = pix[i*3];
			int Cb = pix[i*3 + 1];
			int Cr = pix[i*3 + 2];

			pix[i*3 + R] = (uint8) (float(Y)                          + 1.402*(float(Cr-128)));
			pix[i*3 + G] = (uint8) (float(Y) - 0.344 *(float(Cb-128)) - 0.714*(float(Cr-128)));
			pix[i*3 + B] = (uint8) (float(Y) + 1.772* (float(Cb-128)));

/*
			Y = (Y < 16) ? 16 : Y;
			Y = (Y > 235) ? 235 : Y;

			pix[i*3 + R] = (uint8) (1.164 * ((float)(Y - 16)) + 1.596 * ((float)(Cr - 128)));
			pix[i*3 + G] = (uint8) (1.164 * ((float)(Y - 16)) - 0.391 * ((float)(Cb - 128)) - 0.813 * ((float)(Cr - 128)));
			pix[i*3 + B] = (uint8) (1.164 * ((float)(Y - 16)) + 2.018 * ((float)(Cb - 128)));
*/
		}
	}

	iImage->SetPhotometricInterpretation(kRGB);
	iImage->SetPlanarConfiguration(kRGBRGB);

#if _DEBUG
	ofstream filestm;
	filestm.open("c:\\test.raw", ios::out | ios::binary);
	filestm.write((const char*)iImage->GetFrame(0), iImage->GetNumberOfRows() * iImage->GetNumberOfColumns() * iImage->GetBytesPerPixel());
#endif
	return kSuccess;
}

