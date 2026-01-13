/***********************************************************************
 * Compression.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Handles Encoding and Decoding of Encapsulated Pixel Data
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

#include "Compression.h"

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
#include "ijl.h"

#ifndef AWARE_ONLY
#include "j2kDecoder.h"
#endif

#include <fstream>
//#include "../J2KCompression/Aware/Include/j2k.h"
typedef void aw_j2k_object;
#define AW_J2K_ENDIAN_LOCAL_MACHINE 0 /* default value */
#define AW_J2K_ENDIAN_SMALL 1
#define AW_J2K_ENDIAN_BIG 2


#ifdef _PROFILE
#include "scopetimer.h"
extern iRTVBase gLogger;
#endif

MSG_TO_COMPRESSION_MAP Compression::cbObjectMap;

const char* errStr[] = 
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
	"IJLError",
	"PegasusError",
	"RLEError",
	"J2KError",
	"CompressionException"
};

const char* awareErrStr1[] = 
{
	"AWARE_SUCCESSFUL_OPERATION",
	"AWARE_MALLOC_ERROR",
	"AWARE_UNSUPPORTED_JP2K_TYPE_ERROR",
	"AWARE_MISSING_COD_MARKER",
	"AWARE_MISSING_QCD_MARKER",
	"AWARE_ROWS_RANGE_ERROR",
	"AWARE_COLS_RANGE_ERROR",
	"AWARE_ASPECT_RATIO_ERROR",
	"AWARE_RATIO_RANGE_ERROR",
	"AWARE_NULL_DATA",
	"AWARE_COMPRESS_ERROR",
	"AWARE_MALLAT_LEVELS_ERROR",
	"AWARE_RESOLUTION_LEVELS_ERROR",
	"AWARE_BYTESTREAM_ERROR",
	"AWARE_MARKER_ERROR",
	"AWARE_MARKER_LENGTH_ERROR",
	"AWARE_USER_ABORT",
	"AWARE_TOLERANCE_ERROR",
	"AWARE_OUT_OF_DATA_ERROR",
	"AWARE_SHORT_DATA_ERROR",
	"AWARE_IMAGE_MISMATCH_ERROR",
	"AWARE_LOG_RATIO_ERROR",
	"AWARE_BPP_ERROR",
	"AWARE_GUARD_BITS_TOO_FEW",
	"AWARE_ILLEGAL_CODESTREAM_ERROR",
	"AWARE_BAD_COMPONENT",
	"AWARE_BAD_PRECINCT_SPECIFICATION",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_INVALID_OBJECT",
	"AWARE_BAD_PARAMETER",
	"AWARE_BUFFER_TOO_SHORT"
};

const char* awareErrStr100[] = 
{
	"AWARE_FILE_OPEN",
	"AWARE_FILE_CREATE",
	"AWARE_FILE_READ",
	"AWARE_FILE_WRITE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_UNKNOWN_ERROR_CODE",
	"AWARE_INPUT_FORMAT_MISMATCH",
	"AWARE_INPUT_NOTSET"
};

const char* awareErrStr200[] = 
{
	"AWARE_UNKNOWN_IMAGE_TYPE",
	"AWARE_UNSUPPORTED_IMAGE_TYPE",
	"AWARE_INVALID_IMAGE",
	"AWARE_INVALID_PARAMETER",
	"JP2_PARSE_ERROR",
	"JP2_NO_ICCPROFILE",
	"JP2_NO_ENUM_COLORSPACE",
	"AWARE_DICOM_LIBRARY",
	"AWARE_INVALID_CALL_SEQUENCE",
	"AWARE_UNSUPPORTED_FUNCTION",
	"AWARE_MEMORYMAP_OUTPUT",
	"AWARE_MEMORYMAP_INPUT"
};

std::string Compression::ErrorString(int iErrorcode)
{
	int err = iErrorcode;

	if (err >= 0)
	{
		return errStr[err];
	}

	err = -err;
	
	if (err <= 32)
	{
		return awareErrStr1[err];
	}
	else if (100 <= err && err <= 111)
	{
		return awareErrStr100[err - 100];
	}
	else if (200 <= err && err <= 211)
	{
		return awareErrStr200[err - 200];
	}
	else if (err == 900)
	{
		return "AWARE_LIBRARY_EXPIRED";
	}
	else if (err == 999)
	{
		return "AWARE_LIBRARY_INTERNAL";
	}
	else
	{
		return "Unknown error";
	}
}

//
//	Try to dynamically link to Aware DLL
//		if we can't find it, fall back to Kakadu
//
static bool gTriedToLoadAwareDLL = false;
static bool gLoadedAwareDLL = false;
static bool LoadAwareDLL(void);

typedef INT (WINAPI *AWARE_CREATE)(aw_j2k_object **j2k_object);
typedef INT (WINAPI *AWARE_SET_INPUT_RAW_ENDIANNESS)(aw_j2k_object *j2k_object, unsigned long int fEndianness);
typedef INT (WINAPI *AWARE_SET_INPUT_IMAGE)(aw_j2k_object *j2k_object,
												unsigned char* image_buffer, unsigned long int image_buffer_length);
typedef INT (WINAPI *AWARE_SET_INPUT_IMAGE_TYPE)(aw_j2k_object *j2k_object, int image_type,
												unsigned char* image_buffer, unsigned long int image_buffer_length);
typedef INT (WINAPI *AWARE_SET_INPUT_RAW_CHANNEL_BPP)(aw_j2k_object *j2k_object, unsigned long int channel, long int bpp);
typedef INT (WINAPI *AWARE_SET_INPUT_IMAGE_RAW)(aw_j2k_object *j2k_object, unsigned char *image_buffer,
												unsigned long int rows, unsigned long int cols,
												unsigned long int nChannels, unsigned long int bpp,
												BOOL bInterleaved);
typedef INT (WINAPI *AWARE_SET_OUTPUT_TYPE)(aw_j2k_object *j2k_object, int image_type);
typedef INT (WINAPI *AWARE_SET_OUTPUT_J2K_RATIO)(aw_j2k_object *j2k_object, float ratio);
typedef INT (WINAPI *AWARE_GET_OUTPUT_IMAGE)(aw_j2k_object *j2k_object, unsigned char** image_buffer,
											 unsigned long int *image_buffer_length);
typedef INT (WINAPI *AWARE_GET_INPUT_IMAGE_INFO)(aw_j2k_object *j2k_object, unsigned long int *cols,
												 unsigned long int *rows, unsigned long int *bpp, 
												 unsigned long int *nChannels);
typedef INT (WINAPI *AWARE_SET_OUTPUT_RAW_ENDIANNESS)(aw_j2k_object *j2k_object, unsigned long int fEndianness);
typedef INT (WINAPI *AWARE_GET_OUTPUT_IMAGE_RAW)(aw_j2k_object *j2k_object, unsigned char** image_buffer,
												 unsigned long int *image_buffer_length, unsigned long int *rows,
												 unsigned long int *cols, unsigned long int *nChannels,
												  unsigned long int *bpp, BOOL bInterleaved);
typedef INT (WINAPI *AWARE_DESTROY)(aw_j2k_object *j2k_object);
typedef INT (WINAPI *AWARE_GET_LIBRARY_VERSION)(unsigned long *library_version, char* version_string, 
												unsigned long version_string_buffer_length);
typedef INT (WINAPI *AWARE_GET_OUTPUT_IMAGE_FILE_TYPE)(aw_j2k_object *j2k_object, int image_type, char* filename);
typedef INT (WINAPI *AWARE_GET_INPUT_CHANNEL_BPP)(aw_j2k_object *j2k_object, unsigned long int channel, long int* bpp);
typedef INT (WINAPI *AWARE_SET_INTERNAL_OPTION)(aw_j2k_object *j2k_object, int option, int value);
typedef INT (WINAPI *AWARE_SET_OUTPUT_J2K_XFORM)(aw_j2k_object *j2k_object, int type, int levels);


static AWARE_CREATE dyn_aw_j2k_create = 0;
static AWARE_SET_INPUT_RAW_ENDIANNESS dyn_aw_j2k_set_input_raw_endianness = 0;
static AWARE_SET_INPUT_IMAGE dyn_aw_j2k_set_input_image = 0;
static AWARE_SET_INPUT_IMAGE_TYPE dyn_aw_j2k_set_input_image_type = 0;
static AWARE_SET_INPUT_RAW_CHANNEL_BPP dyn_aw_j2k_set_input_raw_channel_bpp = 0;
static AWARE_SET_INPUT_IMAGE_RAW dyn_aw_j2k_set_input_image_raw = 0;
static AWARE_SET_OUTPUT_TYPE dyn_aw_j2k_set_output_type = 0;
static AWARE_SET_OUTPUT_J2K_RATIO dyn_aw_j2k_set_output_j2k_ratio = 0;
static AWARE_GET_OUTPUT_IMAGE dyn_aw_j2k_get_output_image = 0;
static AWARE_GET_INPUT_IMAGE_INFO dyn_aw_j2k_get_input_image_info = 0;
static AWARE_SET_OUTPUT_RAW_ENDIANNESS dyn_aw_j2k_set_output_raw_endianness = 0;
static AWARE_GET_OUTPUT_IMAGE_RAW dyn_aw_j2k_get_output_image_raw = 0;
static AWARE_DESTROY dyn_aw_j2k_destroy = 0;
static AWARE_GET_LIBRARY_VERSION dyn_aw_j2k_get_library_version = 0;
static AWARE_GET_OUTPUT_IMAGE_FILE_TYPE dyn_aw_j2k_get_output_image_file_type = 0;
static AWARE_GET_INPUT_CHANNEL_BPP dyn_aw_j2k_get_input_channel_bpp = 0;
static AWARE_SET_INTERNAL_OPTION dyn_aw_j2k_set_internal_option = 0;
static AWARE_SET_OUTPUT_J2K_XFORM dyn_aw_j2k_set_output_j2k_xform = 0;

//-----------------------------------------------------------------------------
//
static bool LoadAwareDLL(void)
{
	gTriedToLoadAwareDLL = true;

	HMODULE hDll = NULL;
	hDll = ::LoadLibrary( "awj2k.dll" );

 	if (!hDll)
	{
		return gLoadedAwareDLL = false;
	}

	dyn_aw_j2k_create = (AWARE_CREATE) ::GetProcAddress(hDll, "aw_j2k_create");
	if (!dyn_aw_j2k_create)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_input_raw_endianness = (AWARE_SET_INPUT_RAW_ENDIANNESS) ::GetProcAddress(hDll, "aw_j2k_set_input_raw_endianness");
	if (!dyn_aw_j2k_set_input_raw_endianness)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_input_image = (AWARE_SET_INPUT_IMAGE) ::GetProcAddress(hDll, "aw_j2k_set_input_image");
	if (!dyn_aw_j2k_set_input_image)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_input_image_type = (AWARE_SET_INPUT_IMAGE_TYPE) ::GetProcAddress(hDll, "aw_j2k_set_input_image_type");
	if (!dyn_aw_j2k_set_input_image_type)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_input_raw_channel_bpp = (AWARE_SET_INPUT_RAW_CHANNEL_BPP) ::GetProcAddress(hDll, "aw_j2k_set_input_raw_channel_bpp");
	if (!dyn_aw_j2k_set_input_raw_channel_bpp)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_input_image_raw = (AWARE_SET_INPUT_IMAGE_RAW) ::GetProcAddress(hDll, "aw_j2k_set_input_image_raw");
	if (!dyn_aw_j2k_set_input_image_raw)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_output_type = (AWARE_SET_OUTPUT_TYPE) ::GetProcAddress(hDll, "aw_j2k_set_output_type");
	if (!dyn_aw_j2k_set_output_type)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_output_j2k_ratio = (AWARE_SET_OUTPUT_J2K_RATIO) ::GetProcAddress(hDll, "aw_j2k_set_output_j2k_ratio");
	if (!dyn_aw_j2k_set_output_j2k_ratio)
		return gLoadedAwareDLL = false;
	
	dyn_aw_j2k_get_output_image = (AWARE_GET_OUTPUT_IMAGE) ::GetProcAddress(hDll, "aw_j2k_get_output_image");
	if (!dyn_aw_j2k_get_output_image)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_get_input_image_info = (AWARE_GET_INPUT_IMAGE_INFO) ::GetProcAddress(hDll, "aw_j2k_get_input_image_info");
	if (!dyn_aw_j2k_get_input_image_info)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_output_raw_endianness = (AWARE_SET_OUTPUT_RAW_ENDIANNESS) ::GetProcAddress(hDll, "aw_j2k_set_output_raw_endianness");
	if (!dyn_aw_j2k_set_output_raw_endianness)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_get_output_image_raw = (AWARE_GET_OUTPUT_IMAGE_RAW) ::GetProcAddress(hDll, "aw_j2k_get_output_image_raw");
	if (!dyn_aw_j2k_get_output_image_raw)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_destroy = (AWARE_DESTROY) ::GetProcAddress(hDll, "aw_j2k_destroy");
	if (!dyn_aw_j2k_destroy)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_get_library_version = (AWARE_GET_LIBRARY_VERSION) ::GetProcAddress(hDll, "aw_j2k_get_library_version");
	if (!dyn_aw_j2k_get_library_version)
		return gLoadedAwareDLL = false;
	
	dyn_aw_j2k_get_output_image_file_type = (AWARE_GET_OUTPUT_IMAGE_FILE_TYPE) ::GetProcAddress(hDll, "aw_j2k_get_output_image_file_type");
	if (!dyn_aw_j2k_get_output_image_file_type)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_get_input_channel_bpp = (AWARE_GET_INPUT_CHANNEL_BPP) ::GetProcAddress(hDll, "aw_j2k_get_input_channel_bpp");
	if (!dyn_aw_j2k_get_input_channel_bpp)
		return gLoadedAwareDLL = false;
	
	dyn_aw_j2k_set_internal_option = (AWARE_SET_INTERNAL_OPTION) ::GetProcAddress(hDll, "aw_j2k_set_internal_option");
	if (!dyn_aw_j2k_set_internal_option)
		return gLoadedAwareDLL = false;

	dyn_aw_j2k_set_output_j2k_xform = (AWARE_SET_OUTPUT_J2K_XFORM) ::GetProcAddress(hDll, "aw_j2k_set_output_j2k_xform");
	if (!dyn_aw_j2k_set_output_j2k_xform)
		return gLoadedAwareDLL = false;

	return gLoadedAwareDLL = true;
}

//-----------------------------------------------------------------------------
//
Compression::Compression()
{
	m_pixelData = 0;
	m_ptr = 0;
	m_pixelDataSize = 0;
	m_bitDepth = 8;

	m_compressionMethod = kNone;
	m_frameSize = 0;
	m_offsetTable = 0;
	m_frameNumber = 0;
	m_startOfFirstFragment = 0;

	m_compressionFactor = 1;
	m_bigEndian = false;
	m_encodeDicom = true;

	m_useStandardCodecs = 0;

	m_OBOWbuffer = 0;
	m_OBOWoffset = 0;
	m_OBOWlength = 0;

#ifdef _PROFILE
	ScopeTimer::Init(&gLogger);
#endif
	
	m_uncompressedFramePixelsSize = 0;
	m_pImage = 0;
	m_messageID = -1;
}

//-----------------------------------------------------------------------------
//
int Compression::SetPixels(CPxDicomImage* iImage, uint8* iPixelData, long iPixelDataSize, long iFrameSize, 
						   long iBitDepth, int iTransferSyntax, int iSamplesPerPixel)
{ 
	m_messageID = iImage->GetID();
	if (m_messageID < 0 || !iPixelData || iPixelDataSize <= 0 || iBitDepth <= 0 || iSamplesPerPixel <= 0)
		return kInvalidArguments;

	m_pImage = iImage;

	if (iTransferSyntax != JPEG_BASELINE &&
		iTransferSyntax != JPEG_LOSSLESS_HIER_14 &&
		iTransferSyntax != JPEG_LOSSLESS_NON_HIER_14 &&
		iTransferSyntax != JPEG_EXTENDED_2_4 &&
		iTransferSyntax != RLE &&
		iTransferSyntax != JPEG_2000_LOSSLESS_ONLY &&
		iTransferSyntax != JPEG_2000)
	{
		return kUnsupportedCompressionMethod;
	}
	m_transferSyntax = iTransferSyntax;
	m_decompressContext = 0;


	m_pixelData = iPixelData; 
	m_ptr =	iPixelData;
	m_pixelDataSize = iPixelDataSize;
	m_bitDepth = iBitDepth;
	m_frameSize = iFrameSize;
	m_samplesPerPixel = iSamplesPerPixel;

	//	-- - 08/07/02	Trick Pegasus into thinking its 16 bits if not 8,10,12 or 16
	if (iBitDepth > 8 && 
		iBitDepth < 16 &&
		iBitDepth != 10 &&
		iBitDepth != 12)
	{
		m_bitDepth = 16;
		MC_Set_Value_From_UShortInt(m_messageID, MC_ATT_BITS_STORED, 16);
	}

	cbObjectMap.Add(m_messageID, this, true);
	return kSuccess;
}

//-----------------------------------------------------------------------------
//
void Compression::Write8(char iVal)
{
	char* p = (char*) m_ptr;

	*p = iVal;
	m_ptr += 1;
}

//-----------------------------------------------------------------------------
//
void Compression::Write16(uint16 iVal)
{
	uint16* p = (uint16*) m_ptr;

	*p = iVal;
	m_ptr += 2;
}

//-----------------------------------------------------------------------------
//
void Compression::Write32(uint32 iVal)
{
	uint32* p = (uint32*) m_ptr;

	*p = iVal;
	m_ptr += 4;
}

//-----------------------------------------------------------------------------
//
char Compression::Read8()
{
	char val = *((char*) m_ptr);
	m_ptr += 1;
	return val;
}

//-----------------------------------------------------------------------------
//
uint16 Compression::Read16()
{
	uint16 val = *((uint16*) m_ptr);
	m_ptr += 2;
	return val;
}

//-----------------------------------------------------------------------------
//
uint32 Compression::Read32()
{
	uint32 val = *((uint32*) m_ptr);
	m_ptr += 4;
	return val;
}

//-----------------------------------------------------------------------------
//
void Compression::Skip(uint32 n)
{
	m_ptr += n;
}

//-----------------------------------------------------------------------------
//
MC_STATUS pixelCB(int CBMsgFileItemID, unsigned long CBtag, void* CBuserInfo,
								int CBdataSize, void* CBdataBuffer, int CBisFirst, int CBisLast)
{
	Compression* p = (Compression*) CBuserInfo;
	MSG_TO_COMPRESSION_MAP* cbObjectMap = &p->cbObjectMap;
	Compression* pCompr = (Compression*) cbObjectMap->Get(CBMsgFileItemID);
	if (!pCompr)
		return MC_CALLBACK_CANNOT_COMPLY;

	return (MC_STATUS) pCompr->GetUncompressedPixels(CBMsgFileItemID, CBtag, CBuserInfo, CBdataSize, CBdataBuffer, CBisFirst, CBisLast);
}

//-----------------------------------------------------------------------------
//
int Compression::GetUncompressedPixels(int CBMsgFileItemID, unsigned long CBtag, void* CBuserInfo,
								int CBdataSize, void* CBdataBuffer, int CBisFirst, int CBisLast)
{
	if (CBisFirst)
	{
		if (!m_OBOWlength)
			return MC_CALLBACK_CANNOT_COMPLY;

		m_OBOWoffset = 0;
		m_OBOWbuffer = new unsigned char[m_OBOWlength];
		if (!m_OBOWbuffer)
			return MC_CALLBACK_CANNOT_COMPLY;
	}

	assert (m_OBOWoffset + CBdataSize <= m_OBOWlength);
	if (m_OBOWoffset + CBdataSize > m_OBOWlength)
	{
		return MC_CALLBACK_CANNOT_COMPLY;						
	}

	memcpy(m_OBOWbuffer + m_OBOWoffset, CBdataBuffer, CBdataSize);
	m_OBOWoffset += CBdataSize;
	
	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
int Compression::DecodeNextFrame(uint8*& oFramePixels)
{
	int status = kUnknownStatus;
	bool useStandardCodecs = (m_useStandardCodecs && (m_transferSyntax == JPEG_LOSSLESS_HIER_14 || m_transferSyntax == JPEG_EXTENDED_2_4));
	try
	{
		if (useStandardCodecs)
		{
			if (m_frameNumber < 1)
			{
				MC_Register_Compression_Callbacks(m_messageID, MC_Standard_Compressor, MC_Standard_Decompressor);
				m_OBOWlength = m_frameSize;
				status = MC_Get_Encapsulated_Value_To_Function(m_messageID, MC_ATT_PIXEL_DATA, this, pixelCB);
				//	NOTE: so many successful return values are unconventional...ut it's Merge's fault!!  Check out the docs for MC_Get_Encapsulated_Value_To_Function()
				status = (status == MC_END_OF_FRAME || status == MC_NORMAL_COMPLETION || status == MC_END_OF_DATA || status == MC_NO_MORE_VALUES) ? kSuccess : kPegasusError;
			}
			else
			{
				status = MC_Get_Next_Encapsulated_Value_To_Function(m_messageID, MC_ATT_PIXEL_DATA, this, pixelCB);
				//	NOTE: so many successful return values are unconventional...ut it's Merge's fault!!  Check out the docs for MC_Get_Encapsulated_Value_To_Function()
				status = (status == MC_END_OF_FRAME || status == MC_NORMAL_COMPLETION || status == MC_END_OF_DATA || status == MC_NO_MORE_VALUES) ? kSuccess : kPegasusError;
			}

			if (status == kSuccess)
			{	
				m_frameNumber++;
			}
		}
		else
		{
			status = DoDecodeNextFrame(oFramePixels);
		}
	}
	catch(...)
	{
		status = kCompressionException;
	}

	if (useStandardCodecs)
	{
		oFramePixels = m_OBOWbuffer;
		m_uncompressedFramePixelsSize = m_OBOWoffset;
	}
	return status;
}

//-----------------------------------------------------------------------------
//
int Compression::DoDecodeNextFrame(uint8*& oFramePixels)
{
	uint16 group;
	uint16 element;
	uint32 vl;
	long frameBytes = 0;

	if (m_frameNumber == 0)
	{
		//	Read in the Offset Table Header
		group = Read16();
		element = Read16();
		vl = Read32();

		//	Read offset table
		//  changed by shiying, gang li, 2005-10-13
		if (vl/4 > 0)
		{
			if ( !m_offsetTable )
			{
				delete [] m_offsetTable, m_offsetTable = 0;
			}

			m_offsetTable = new uint32[vl/4];
			if (!m_offsetTable)
				return kMemoryAllocationFailed;

			uint32 i;
			for(i=0; i<(vl/4); i++)
			{
				m_offsetTable[i] = Read32();
			}
		}

		m_startOfFirstFragment = m_ptr;
		//	Ignore the Offset Table
//		Skip(vl);
	}

	if (m_frameNumber > 0 && m_offsetTable)
	{
		m_ptr = m_startOfFirstFragment +  m_offsetTable[m_frameNumber];
	}

	long bytesRead = m_ptr - m_pixelData;
	while(bytesRead < m_pixelDataSize && frameBytes < m_frameSize)
	{
		//	Read in the tag and value length
		group = Read16();
		element = Read16();
		vl = Read32();

		//	Improperly encoded PixelData element
		if (group != 0xfffe) 
		{
			return kBadItemTag;
		}

		// Sequence Delimiter Tag
		if (element == 0xe0dd) 
		{
			assert(vl == 0);
			break;
		} 
		
		//	Improperly encoded PixelData element
		if (element != 0xe000) 
		{
			return kBadItemTag;
		}
			
		//	choose the right codec
		if (m_transferSyntax == RLE)
		{
			int status = UncompressFragmentRLE(vl);
			if (status != kSuccess)
			{
				return kRLEError;
			}
			frameBytes += m_uncompressedFramePixelsSize;
		} 
		else if (m_transferSyntax == JPEG_LOSSLESS_HIER_14 ||
				 m_transferSyntax == JPEG_EXTENDED_2_4)
		{
			int status = UncompressFragmentPegasus(vl);
			if (status != kSuccess)
			{
				return kPegasusError;
			}
			frameBytes += m_uncompressedFramePixelsSize;
		}
		else if (m_transferSyntax == JPEG_2000_LOSSLESS_ONLY ||
				 m_transferSyntax == JPEG_LOSSLESS_NON_HIER_14 ||
				 m_transferSyntax == JPEG_2000)
		{
			if (m_transferSyntax == JPEG_2000_LOSSLESS_ONLY &&
				m_compressionFactor > 1)
			{
				m_compressionFactor = 1;
			}

			int status = UncompressFragmentJ2K(vl);
			if (status != kSuccess)
			{
				return status;
			}
			frameBytes += m_uncompressedFramePixelsSize;
		}
		else if (m_transferSyntax == JPEG_BASELINE)
		{
			if (m_bitDepth == 8)
			{
				int status = UncompressFragment8(vl);
				if (status != IJL_OK)
				{
					return kIJLError;
				}

				frameBytes += m_uncompressedFramePixelsSize;
			} /*else if (m_bitDepth <= 12)
			{
				UncompressFragment12(vl);
			} */
			else
			{
				return kUnsupportedBitDepth;
			}
		}

		bytesRead = m_ptr - m_pixelData;
	}

	if (m_uncompressedFramePixelsSize > 0)
	{
		oFramePixels = m_uncompressedFramePixels;
		m_frameNumber++;
		return kSuccess;
	}

	return kMissingSequenceDelimiter;
}


#ifndef AWARE_ONLY
#include "j2kErr.h"
#include "rtvsswap.h"
#endif

//-----------------------------------------------------------------------------
//
	
#include "../J2KCompression/Aware/Include/j2k.h"
#include "../J2KCompression/Aware/Include/aw_j2k_errors.h"

//-----------------------------------------------------------------------------
//
int Compression::Encode(unsigned char*  iData, int iWidth, int iHeight, int iChannels,
	   			         char** oData, int& oLen, int iStride, int iChannelDepth, int iSigned, int iInterleaved)
{
	int status = kSuccess;

	try
	{
		status = DoEncode(iData, iWidth, iHeight, iChannels, oData, oLen, iStride, iChannelDepth, iSigned, iInterleaved);
	}
	catch(...)
	{
		status = kCompressionException;
	}

	return status;	
}

//-----------------------------------------------------------------------------
//
int Compression::DoEncode(unsigned char*  iData, int iWidth, int iHeight, int iChannels,
	   			         char** oData, int& oLen, int iStride, int iChannelDepth, int iSigned, int iInterleaved)
{
	uint32* lengthPointer;
/*
/*
 *	Used for testing / debugging only 
 	FILE* fp = fopen("c:\\tmp\\pre.dat", "wb");
	if (fp)
	{
		fwrite(iData, 524288, 1, fp);
		fclose(fp);
		fp = 0;
	}
*/
	int bytesPerPixel  = (iChannelDepth + 7)/8;
	int bitsPerPixel = /*(iChannelDepth > 12) ? 12 : */iChannelDepth * iChannels;
	long inputSize = (long) (iWidth * iHeight * iChannels * bytesPerPixel);
	unsigned int compressedSize = (int) inputSize * 1.2;

	if (!*oData)
	{
		*oData = new char[compressedSize];
	}

	m_ptr = (unsigned char*) *oData;

	if (!*oData)
	{
		return kMemoryAllocationFailed;
	}


	if (m_encodeDicom)
	{
		//	Write empty offset table
		Write16(0xfffe);
		Write16(0xe000);
		Write32(0x0);

		//	Write Fragment sequence
		Write16(0xfffe);
		Write16(0xe000);
		lengthPointer = (uint32*) m_ptr;
		Write32(0x0);		//	Don't know the length yet - we'll put it in later
	}

	unsigned char *in;
	in = (unsigned char*) iData;
 
/*
 *	Used for testing / debugging only
	unsigned short *s = (unsigned short*) in, *se;

		for (se = s + iWidth * iHeight; s < se; s++)
			*s = iRTVSSwap(*s);

	fp = fopen("c:\\tmp\\mid.dat", "wb");
	if (fp)
	{
		fwrite(in, 524288, 1, fp);
		fclose(fp);
		fp = 0;
	}
*/
	//
	//	Use Aware if we can find the dll
	//
	if (!gLoadedAwareDLL && !gTriedToLoadAwareDLL)
	{
		LoadAwareDLL();
	}

	if (gLoadedAwareDLL)
	{
		int retval;
		aw_j2k_object *j2k_object;

		j2k_object = NULL;
		retval = dyn_aw_j2k_create(&j2k_object);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

		retval = dyn_aw_j2k_set_input_raw_endianness(
				j2k_object,
				AW_J2K_ENDIAN_SMALL);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

		retval = dyn_aw_j2k_set_input_image_raw(
				j2k_object,
				iData,
				(unsigned long int) iHeight,
				(unsigned long int) iWidth,
				(unsigned long int) iChannels,
				(unsigned long int) bitsPerPixel,
				iInterleaved);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

		if (iChannelDepth > 12)
		{
			retval = dyn_aw_j2k_set_internal_option(j2k_object,
					 AW_J2K_INTERNAL_ARITHMETIC_OPTION, AW_J2K_INTERNAL_USE_FLOATING_POINT);		
			if (retval != NO_ERROR)
			{
				return -retval;
			}
		}
		else
		{
			retval = dyn_aw_j2k_set_internal_option(j2k_object,
					 AW_J2K_INTERNAL_ARITHMETIC_OPTION, AW_J2K_INTERNAL_USE_FIXED_POINT);		
			if (retval != NO_ERROR)
			{
				return -retval;
			}
		}

		int bpp = iSigned ? -1*iChannelDepth : iChannelDepth;
		for (int i=0; i<iChannels; i++)
		{
			retval = dyn_aw_j2k_set_input_raw_channel_bpp(j2k_object, i, bpp);
			if (retval != NO_ERROR)
			{
				return -retval;
			}
		}

		retval = dyn_aw_j2k_set_output_type(j2k_object, AW_J2K_FORMAT_J2K);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

		//	Other choice is AW_J2K_WV_TYPE_I97.  This one works for lossy and lossless,
		//		and seems to produce better images even in difficult cases.
		retval = dyn_aw_j2k_set_output_j2k_xform(j2k_object, AW_J2K_WV_TYPE_R53, -1);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

		if (m_compressionFactor < 2)
		{
			//	Lossless compression
			m_compressionFactor = 0;
		}

		retval = dyn_aw_j2k_set_output_j2k_ratio(j2k_object, float(m_compressionFactor));
		if (retval != NO_ERROR)
		{
			return -retval;
		}

		unsigned long int outSize = compressedSize;
		unsigned char* bufp = 0;
		{
	//		ScopeTimer timer("AWARE Encode");
			retval = dyn_aw_j2k_get_output_image(j2k_object, &m_ptr, &outSize);
			if (retval != NO_ERROR)
			{
				return -retval;
			}
		}

		compressedSize = (unsigned int) outSize;
		dyn_aw_j2k_destroy(j2k_object);
	}
	else
	{
#ifdef AWARE_ONLY
		return kJ2KError;
#else
		//	
		//	Couldn't find the Aware dll, so use Kakadu instead
		//

		int codecStatus = kJ2KError;

		m_encoder.setBigEndian(false);//m_bigEndian);
		m_encoder.setStride(iWidth);
		m_encoder.setFastMode(true);
		m_encoder.setCompressionRatio(1.0f / m_compressionFactor);


#ifndef _DEBUG
		try
		{
#endif
	//		ScopeTimer timer("KAKADU Encode");

			codecStatus = m_encoder.compress(&in, iWidth, iHeight, iChannels, bitsPerPixel, (unsigned char*) m_ptr, compressedSize);
#ifndef _DEBUG
		}
		catch (...)
		{
			if (codecStatus == 0)
				codecStatus = kJ2KError;
		}
#endif
		if (codecStatus != 0)
		{
			return kJ2KError;
		} 
#endif
	}

/*
 *	Used for testing / debugging only
	long size = iWidth * iHeight * iChannels * bytesPerPixel;
	unsigned char* uncompressedBuffer = new uint8[size*1.2];
	unsigned char* outData[3] = {(unsigned char*) uncompressedBuffer,0,0};
	unsigned int uncompressedBytes = size;

	j2kDecoder decoder;
	unsigned int w, h, spp, bs;
	codecStatus = decoder.getImgProperty(m_ptr, w, h, spp, bs);
	codecStatus = decoder.decompress(m_ptr, 
									 size, 
									 w,
									 h,
									 spp,
									 bs,
									 outData,
									 &uncompressedBytes);

	fp = fopen("c:\\tmp\\post.dat", "wb");
	if (fp)
	{
		fwrite(outData[0], size, 1, fp);
		fclose(fp);
	}
*/

	oLen = compressedSize;
	m_ptr += oLen;

	if (m_encodeDicom)
	{
		//	Pad to even length if necessary
		if (oLen % 2)
		{
			*m_ptr = 0;
			++m_ptr;
			++oLen;
		}

		//	Backfill value length field
		*lengthPointer = (uint32) oLen; 

		//	Write Sequence Delimiter
		Write16(0xfffe);
		Write16(0xe0dd);
		Write32(0x0);

		//	Account for encapsulation bytes added
		oLen += 24;
	}

	return kSuccess;
}

//-----------------------------------------------------------------------------
//
int Compression::Decode(long& oUncompressedSize)
{
	m_decompressContext = 0;

	//	Read in the Offset Table Header
	uint16 group = Read16();
	uint16 element = Read16();
	uint32 vl = Read32();

	//	Ignore the Offset Table
	Skip(vl);

	long bytesRead = m_ptr - m_pixelData;
	while(bytesRead < m_pixelDataSize)
	{
		//	Read in the tag and value length
		group = Read16();
		element = Read16();
		vl = Read32();

		//	Improperly encoded PixelData element
		if (group != 0xfffe) 
		{
			return kBadItemTag;
		}

		// Sequence Delimiter Tag
		if (element == 0xe0dd) 
		{
			assert(vl == 0);
			return CombineFragments(oUncompressedSize);
		} 
		
		//	Improperly encoded PixelData element
		if (element != 0xe000) 
		{
			return kBadItemTag;
		}
	
		//	choose the right codec
		if (m_transferSyntax == RLE)
		{
			UncompressFragmentRLE(vl);
		} 
		else if (m_transferSyntax == JPEG_2000_LOSSLESS_ONLY ||
				 m_transferSyntax == JPEG_2000)
		{
			if (m_transferSyntax == JPEG_2000_LOSSLESS_ONLY &&
				m_compressionFactor > 1)
			{
				m_compressionFactor = 1;
			}

			int status = UncompressFragmentJ2K(vl);
			if (status != kSuccess)
			{
				return kJ2KError;
			}
		}
		else if (m_transferSyntax != JPEG_BASELINE)
		{
			UncompressFragmentPegasus(vl);
		}
		else if (m_bitDepth == 8)
		{
			UncompressFragment8(vl);
		} /*else if (m_bitDepth <= 12)
		{
			UncompressFragment12(vl);
		} */else
		{
			return kUnsupportedBitDepth;
		}
	}

	return kMissingSequenceDelimiter;
}

//-----------------------------------------------------------------------------
//
int Compression::CombineFragments(long& oUncompressedSize)
{
	int i;
	uint32 size = 0;
	for( i=0; i<m_uncompressedPixels.size(); i++)
	{
		size += m_uncompressedPixels[i].size;
	}

	// seems that m_pixelData is from SetPixel function.
	// therefore, inside compression, it is not safe to delete it.
	// if we do want to delete, please check if m_pixelData == 0
	delete[] m_pixelData;
	m_pixelData = new uint8[size];
	m_ptr = m_pixelData;

	if (!m_pixelData)
	{
		return kMemoryAllocationFailed;
	}

	oUncompressedSize = size;

	for(i=0; i<m_uncompressedPixels.size(); i++)
	{
		memcpy(m_ptr, m_uncompressedPixels[i].buf, m_uncompressedPixels[i].size);
		m_ptr += m_uncompressedPixels[i].size;
		delete[] m_uncompressedPixels[i].buf;
		m_uncompressedPixels[i].buf = 0;
	}

	m_uncompressedPixels.clear();

	return kSuccess;
}

//-----------------------------------------------------------------------------
//
//	RLE decompression - see DICOM PS3.5 Annex G
//
int Compression::UncompressFragmentRLE(uint32 iLength)
{
	if (m_samplesPerPixel != 3)
	{
		int status = DecompressGrayScaleRLE();
		return status;
	}

	//	We only know how to process 3 components
	char* segmentHeaderStart = (char*) m_ptr;
	uint32 numberOfSegments = Read32();
	if (numberOfSegments != m_samplesPerPixel)
	{
		return kNotImplemented;
	}

	//	Allocate memory for the segments (one for each YBR component)
	int componentSize = m_frameSize / m_samplesPerPixel;

	assert((m_frameSize % m_samplesPerPixel) == 0);

	unsigned char* outBuf = new unsigned char[m_frameSize];

	if (!outBuf)
	{
		return kMemoryAllocationFailed;
	}

	//	Get the offsets for each component (Y, Cb, Cr)
	uint32 offset[3];
	offset[0] = Read32();
	offset[1] = Read32();
	offset[2] = Read32();

	//	Skip remaining offset table entries - should be 0 anyway
	Skip(4 * 13);

	//	ASSERT: m_ptr should be pointing to the first byte of the compressed Y segment
	char n;
	long count, i;
	
	//	Apply RLE decompression for each of the three planes
	for(i = 0; i < m_samplesPerPixel; i++)
	{
		count = 0;

		//	Move to the start of the segment
		m_ptr = (unsigned char*) segmentHeaderStart + offset[i];
		do
		{
			//	This byte tells us if we are doing a repeat run or a replicate run
			n = Read8();

			//	Replicate run
			if (n >= 0 && n <= 127)
			{
				memcpy(outBuf + (i*componentSize) + count, m_ptr, n+1);
				m_ptr += n+1;
				count += n+1;
			} 
			//	Repeat run
			else if (n <= -1 && n >= -127)
			{
				memset(outBuf + (i*componentSize) + count, *m_ptr, (-n)+1);
				m_ptr++;
				count += (-n)+1;
			}
			else if (n == -128)
			{
				;
			}
		} while (count < componentSize);

		//	Skip padding chars
		m_ptr = ((m_ptr - (unsigned char*) segmentHeaderStart + offset[i]) % 2) ? m_ptr + 1 : m_ptr;
	}

	//	YBR -> RGB
	uint8 Y, Cb, Cr;
	const int R = 0;
	const int G = (1 * componentSize);
	const int B = (2 * componentSize);
	for (i = 0; i < componentSize; i++)
	{
		Y  = outBuf[i];
		Cb = outBuf[i + (1 * componentSize)];
		Cr = outBuf[i + (2 * componentSize)];
		
		// -- 2004.04.05
		// Can't figure out where the first set of the conversion came from
		// Replaced with the one from DICOM documentation  PS 3.3
#if 0
		outBuf[i + R] = (uint8) (float(Y) - 7.06 *(float(Cb-128)) + 1.40*(float(Cr-128)));
		outBuf[i + G] = (uint8) (float(Y) - 0.34 *(float(Cb-128)) - 0.71*(float(Cr-128)));
		outBuf[i + B] = (uint8) (float(Y) + 1.772*(float(Cb-128)) + 1.54*(float(Cr-128)));
#else
		// This should be right for YBR_FULL see PS 3.3 
		// -- 2004.04.05
		outBuf[i + R] = (uint8) (float(Y)                          + 1.402*(float(Cr-128)));
		outBuf[i + G] = (uint8) (float(Y) - 0.344 *(float(Cb-128)) - 0.714*(float(Cr-128)));
		outBuf[i + B] = (uint8) (float(Y) + 1.772* (float(Cb-128)));
#endif

/*
		Y = (Y < 16) ? 16 : Y;
		Y = (Y > 235) ? 235 : Y;

		outBuf[i + R] = (uint8) (1.164 * ((float)(Y - 16)) + 1.596 * ((float)(Cr - 128)));
		outBuf[i + G] = (uint8) (1.164 * ((float)(Y - 16)) - 0.391 * ((float)(Cb - 128)) - 0.813 * ((float)(Cr - 128)));
		outBuf[i + B] = (uint8) (1.164 * ((float)(Y - 16)) + 2.018 * ((float)(Cb - 128)));
*/
	}

	m_uncompressedFramePixelsSize = m_frameSize;
	m_uncompressedFramePixels = outBuf;
	return kSuccess;
}

//-----------------------------------------------------------------------------
//
int Compression::UncompressFragmentJ2K(uint32 iLength)
{
//	printf("UncompressFragmentJ2K: ENTER\n");

	unsigned char*  buf = m_ptr;
	if (!gLoadedAwareDLL && !gTriedToLoadAwareDLL)
	{
		LoadAwareDLL();
	}

	if (gLoadedAwareDLL)
	{
//		printf("UncompressFragmentJ2K: Aware Loaded\n");
		int retval;
		aw_j2k_object *j2k_object;

		j2k_object = NULL;
		retval = dyn_aw_j2k_create(&j2k_object);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

//		printf("UncompressFragmentJ2K: j2k_object created\n");
		//
		//	Set the input image
		//
		unsigned long int buffer_length = (unsigned long int) iLength;
		retval = dyn_aw_j2k_set_input_image(j2k_object, buf, buffer_length);
		if (retval != NO_ERROR)
		{
			return -retval;
		}

/*
		printf("UncompressFragmentJ2K: Buffer Length = %ld\n", buffer_length);
		printf("UncompressFragmentJ2K: wrote compressed image: c:/tmp/in.j2k\n");

		FILE* fp = fopen("c:\\tmp\\in.j2k", "wb");
		if (fp)
		{
			fwrite(buf, buffer_length, 1, fp);
			fclose(fp);
			fp = 0;
		}
*/
		//
		//	Allocate the right size buffer to hold the uncompressed pixels
		//
		unsigned long int cols, rows, bpp, nChannels;
		cols = rows = bpp = nChannels = 0;
		retval = dyn_aw_j2k_get_input_image_info(j2k_object, &cols, &rows, &bpp, &nChannels);
		if (retval != NO_ERROR)
		{
			return -retval;
		}
//		printf("UncompressFragmentJ2K: rows = %d, cols = %d, bpp = %d, nChannels = %d\n", cols, rows, bpp, nChannels);

		long int signedBpp;
		retval = dyn_aw_j2k_get_input_channel_bpp(j2k_object, 0, &signedBpp);
		if (retval != NO_ERROR)
		{
			return -retval;
		}
//		printf("UncompressFragmentJ2K: signedBpp = %d\n", signedBpp);

		bool isSigned = (signedBpp < 0);
		//	TODO: not sure what to do with this fact.

		int bytesPerPixel = (bpp + 7) / 8;
		unsigned long int size = cols * rows * nChannels * bytesPerPixel;

//		printf("UncompressFragmentJ2K: bytesPerPixel = %d, size = %ld\n", bytesPerPixel, size);

		m_uncompressedFramePixels = new uint8[size*10];
		if (!m_uncompressedFramePixels)
			return kMemoryAllocationFailed;

		//
		//	Set output to little-endian
		//
		retval = dyn_aw_j2k_set_output_raw_endianness(j2k_object, AW_J2K_ENDIAN_SMALL);
		if (retval != NO_ERROR)
		{
			return -retval;
		}
//		printf("UncompressFragmentJ2K: SetOutputRawEndianness to little endian\n");

		//
		//	Decompress
		//
		BOOL kNotInterleaved = TRUE;
		unsigned char* bufp = (unsigned char*) m_uncompressedFramePixels;
		retval = dyn_aw_j2k_get_output_image_raw(
				 j2k_object, &bufp, &size, &rows, &cols, &nChannels, &bpp, kNotInterleaved);
		if (retval != NO_ERROR)
		{
			return -retval;
		}
		m_pImage->SetPlanarConfiguration(kRGBRGB);

/*
		printf("UncompressFragmentJ2K: wrote uncompressed image: c:/tmp/out.raw\n");
		fp = fopen("c:\\tmp\\out.raw", "wb");
		if (fp)
		{
			fwrite(bufp, size, 1, fp);
			fclose(fp);
			fp = 0;
		}
*/
		retval = dyn_aw_j2k_destroy(j2k_object);
		m_uncompressedFramePixelsSize = size;
	}
	else
	{
#ifdef AWARE_ONLY
		return kJ2KError;
#else
		unsigned int uncompressedBytesReceived;
		unsigned char* uncompressedPixels = 0;

		int codecStatus;
		j2kDecoder decoder;
		unsigned int width, height, samplesPerPixel, bitsStored;

		codecStatus = decoder.getImgProperty(buf, width, height, samplesPerPixel, bitsStored);
		int bytesPerPixel = (bitsStored + 7) / 8;
		unsigned long size = width * height * samplesPerPixel * bytesPerPixel;
		m_uncompressedFramePixels = new uint8[size];
		if (!m_uncompressedFramePixels)
			return kMemoryAllocationFailed;
		uncompressedBytesReceived = size;

		unsigned char*	outData[3] = {(unsigned char*) m_uncompressedFramePixels,0,0};	
		unsigned char* tmpBuf = 0;

	#ifndef _DEBUG
		try
		{
	#endif
			//	03/01/04 - -- - Need to do this to avoid crashes caused by 
			//	Kakadu reading one too many blocks.
			tmpBuf = new unsigned char[iLength + 1024];
			memcpy(tmpBuf, buf, iLength);
			memset(tmpBuf + iLength, 0, 1024);

			codecStatus = decoder.decompress(tmpBuf, 
										 iLength, 
										 width,
										 height,
										 samplesPerPixel,
										 bitsStored,
										 outData,
										 &uncompressedBytesReceived);
	#ifndef _DEBUG
		}
		catch (...)
		{
			if (codecStatus == 0)
				codecStatus = kJ2KError;
		}
	#endif

		if (tmpBuf)
			delete[] tmpBuf, tmpBuf = 0;

		if (codecStatus != 0)
		{
			delete [] m_uncompressedFramePixels, m_uncompressedFramePixels = 0;
			return kJ2KError;
		}

		m_uncompressedFramePixelsSize = uncompressedBytesReceived;

		//	03/04/04 - -- - If we don't check this, 8-bit data will 
		//		cause severe memory corruption / crash.
		if (bytesPerPixel == 2)
		{
			//	Byte swap - since the J2K encoder outputs big endian
			unsigned short *s = (unsigned short*) m_uncompressedFramePixels, *se;
			for (se = s + width * height; s < se; s++)
				*s = iRTVSSwap(*s);
		}
#endif
	}
/*
 *	Used for testing / debugging only
	FILE* fp = fopen("c:\\tmp\\mid.dat", "wb");
	if (fp)
	{
		fwrite(m_uncompressedFramePixels, 524288, 1, fp);
		fclose(fp);
		fp = 0;
	}
*/
	//	Update the internal buffer pointer
	m_ptr += iLength;
	return kSuccess;
}

//-----------------------------------------------------------------------------
//
int Compression::UncompressFragmentPegasus(uint32 iLength)
{
	unsigned char*  buf = m_ptr;
	unsigned long uncompressedBytesReceived;
	void* uncompressedPixels = 0;
	int status;
	int isFirstDecompressCall = 1;
	
	static TRCriticalSection cs;
	TRCSLock lock(&cs);

	//	Decompress the frame
	status = MC_Standard_Decompressor(m_messageID, &m_decompressContext, (unsigned long) iLength,
				buf, &uncompressedBytesReceived, &uncompressedPixels, isFirstDecompressCall,0 , 0);
	if (status != MC_NORMAL_COMPLETION)
	{
		return kPegasusError;
	}

	// added by shiying hu, gang li, 2005-10-13
	// later need to check max size
	if ( uncompressedBytesReceived == 0  )
	{
		return kPegasusError;
		
	}

	//	Copy out the pixels
	m_uncompressedFramePixels = new uint8[uncompressedBytesReceived];
	if (!m_uncompressedFramePixels)
		return kMemoryAllocationFailed;

	m_uncompressedFramePixelsSize = uncompressedBytesReceived;
	memcpy(m_uncompressedFramePixels, uncompressedPixels, uncompressedBytesReceived);	//!!!! Do we need to free uncompressedPixels?

	status = MC_Standard_Decompressor(m_messageID, &m_decompressContext, 0, 0, 0, 0, 0, 0, 1);
	if (status != MC_NORMAL_COMPLETION)
	{
		return kPegasusError;
	}

	//	Update the internal buffer pointer
	m_ptr += iLength;
	return kSuccess;
}

//-----------------------------------------------------------------------------
//
int Compression::UncompressFragment8(uint32 iLength)
{
#if 1
	return -1;
#else
	IJLERR status;
	DWORD imageSize;
	BYTE*  buf = m_ptr;
	JPEG_CORE_PROPERTIES jcprops;

	//	Initialize IJL
	status = ijlInit(&jcprops);
	if (status != IJL_OK)
	{
		return status;
	}

	//	Set up for decompression from buffer
	jcprops.JPGFile		 = NULL;
	jcprops.JPGBytes	 = buf;
	jcprops.JPGSizeBytes = iLength;

	//	Get the jpg properties
	status = ijlRead(&jcprops, IJL_JBUFF_READPARAMS);
	if (status != IJL_OK)
	{
		return status;
	}

	if (m_samplesPerPixel == 1)
	{
		//	Set up for 8-bit luminance grayscale
		jcprops.JPGColor = IJL_G;
		jcprops.DIBColor = IJL_G;
		jcprops.DIBChannels = 1;
	}
	else if (m_samplesPerPixel == 3)
	{
		//	Set up for 8-bit luminance color
		//ijlRead will get jcprops.JPGColor property from image header. If hearer is missing then set it to YCbCr 
		//(Good to use Photometric interpretation)
		if (!(jcprops.JPGColor == IJL_RGB || jcprops.JPGColor == IJL_YCBCR || jcprops.JPGColor == IJL_BGR))
			jcprops.JPGColor = IJL_YCBCR;
		jcprops.DIBColor = IJL_RGB;
		jcprops.DIBChannels = 3;		
	}
	else
	{
		return kInvalidFormat;
	}

	//	Calculate buffer size
	imageSize = jcprops.JPGWidth * jcprops.JPGHeight * jcprops.DIBChannels;

	//	Allocate memory

	// added by shiying, gang li, 2005-10-13
	if ( imageSize == 0 )
	{
		return IJL_BUFFER_TOO_SMALL;
	}

	buf = new BYTE[imageSize];
	if (!buf)
	{
		return IJL_BUFFER_TOO_SMALL;
	}

	//	Set up buffer properties
	jcprops.DIBWidth	= jcprops.JPGWidth;
	jcprops.DIBHeight	= jcprops.JPGHeight;
	jcprops.DIBPadBytes = 0;
	jcprops.DIBBytes	= buf;

	//GL & RL temp fix for reversed image
	//if(jcprops.JPGChannels!=3)
	//	jcprops.DIBHeight = -jcprops.JPGHeight;
	

	//	Uncompress the pixels
	status = ijlRead(&jcprops, IJL_JBUFF_READWHOLEIMAGE);
	if (status != IJL_OK)
	{
		return status;
	}

	//GL & & RL temp fix for reversed image, reverse image for gray scal
	/*if(jcprops.JPGChannels!=3)
	{
		unsigned char tmpBuf[2048];
		unsigned char* pf;
		unsigned char* en;
		for(int i=0; i<jcprops.JPGHeight/2; i++)
		{
			pf = buf+i*jcprops.JPGWidth;
			en = buf+(jcprops.JPGHeight-i-1)*jcprops.JPGWidth;
			memcpy(tmpBuf, pf, jcprops.JPGWidth);
			memcpy(pf, en, jcprops.JPGWidth);
			memcpy(en, tmpBuf, jcprops.JPGWidth);

		}
	}*/

	ijlFree(&jcprops);

	m_uncompressedFramePixelsSize = imageSize;
	m_uncompressedFramePixels = buf;

	m_ptr += iLength;


	return kSuccess;
#endif
}

//-----------------------------------------------------------------------------
//
bool Compression::IsCompressed(int iSyntax)
{
	return 	!(iSyntax == IMPLICIT_LITTLE_ENDIAN || 
			  iSyntax == EXPLICIT_LITTLE_ENDIAN || 
			  iSyntax == IMPLICIT_BIG_ENDIAN);
}

//-----------------------------------------------------------------------------
//
bool Compression::IsLossyCompressed(int iSyntax)
{
	return (iSyntax == JPEG_BASELINE || 
			iSyntax == JPEG_2000);
}

//-----------------------------------------------------------------------------
//
bool Compression::IsLosslessCompressed(int iSyntax)
{
	return (iSyntax == JPEG_LOSSLESS_HIER_14 || 
			iSyntax == JPEG_LOSSLESS_NON_HIER_14 ||
			iSyntax == JPEG_2000_LOSSLESS_ONLY);
}

//-----------------------------------------------------------------------------
//
std::string Compression::GetJ2KToolkitVersionString(void)
{
	if (!gLoadedAwareDLL && !gTriedToLoadAwareDLL)
	{
		LoadAwareDLL();
	}

	if (gLoadedAwareDLL)
	{	
		unsigned long version;
		char versionChar[AW_J2K_LIBRARY_VERSION_STRING_MINIMUM_LENGTH];
		memset(versionChar, 0, sizeof(versionChar));

		dyn_aw_j2k_get_library_version(&version, versionChar, sizeof(versionChar));
		return versionChar;
	}
	else
		return "Not Found";
}

/*
int Compression::UncompressFragment12(uint32 iLength)
{
	//	Input buffer
	JOCTET*  buf = m_ptr;

	//	Set up some ijg data structures
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	//	Set the data source to be our in-memory compressed pixel buffer
	jpeg_mem_src(&cinfo, buf, iLength);

	jpeg_read_header(&cinfo, TRUE);

	printf("Image Height = %d\n",cinfo.image_height);
	printf("Image Width  = %d\n",cinfo.image_width);
//	jpeg_start_decompress(&cinfo);

//	while (

	return kSuccess;
}

*/

//Chetan - Oct , 2005
//Function to decompress RLE compressed DICOM data sets
//tested only with one CT data set (1 slice)
//number of RLE segments were equal to 2 (att10.dcm)
//this is a fix for Bug C6047
int Compression::DecompressGrayScaleRLE()
{
	int k;
	char* segmentHeaderStart = (char*) m_ptr;

	//number of RLE segments
	uint32 numberOfSegments = Read32();
	if ( (m_bitDepth == 8) && (numberOfSegments != 1) )
	{
		return kBadItemTag;
	}
	if ( (m_bitDepth == 16) && (numberOfSegments != 2) )
	{
		return kBadItemTag;
	}
	if (numberOfSegments > 16)
	{
		return kBadItemTag;
	}

	long count = 0;

	// added by shiying, gang li, 2005-10-13
	if (m_frameSize > 4096*4096*2 || m_frameSize <= 0 ) //code review change
	{
		return kMemoryAllocationFailed;
	}
	unsigned char* charDataBuffer = new unsigned char[m_frameSize];	
	if (!charDataBuffer)
	{
		return kMemoryAllocationFailed;
	}

	int componentSize = m_frameSize/numberOfSegments;

	unsigned int *offsetValue = new unsigned int[numberOfSegments];
	for( k=0;k<numberOfSegments;k++)
	{
		offsetValue[k] = Read32();
	}
				
	//skipping rest of the header
	//header is 16*4 bytes long
	Skip((16-k)*4);

	char n;

	for(long i=0;i<numberOfSegments;i++)
	{
		count = 0;

		//	Move to the start of the segment
		m_ptr = (unsigned char*) segmentHeaderStart + offsetValue[i];
	
		do
		{
			//  This byte tells us if we are doing a repeat run or a replicate run
			n = Read8();

			//	Replicate run
			if (n >= 0 && n <= 127)
			{
				memcpy(charDataBuffer + (i*componentSize) + count, m_ptr, n+1);
				m_ptr += n+1;
				count += n+1;
			} 

			//	Repeat run
			else if (n <= -1 && n >= -127)
			{
				memset(charDataBuffer + (i*componentSize) + count, *m_ptr, (-n)+1);
				m_ptr++;
				count += (-n)+1;
			}
			else if (n == -128)
			{
				;
			}
		} while (count < componentSize);					

		//	Skip padding chars
		m_ptr = ((m_ptr - (unsigned char*) segmentHeaderStart + offsetValue[i]) % 2) ? m_ptr + 1 : m_ptr;
	}

	if (offsetValue)
	{
		delete [] offsetValue, offsetValue = 0;
	}

	if(m_bitDepth == 16)
	{
		//added by shiying hu, gang li, 2005-10-13
		if ( m_frameSize/numberOfSegments <= 0 )
			return kMemoryAllocationFailed;

		short int *shortDataBuffer = new short int[m_frameSize/numberOfSegments];
	
		//combining MSB and LSB to generate the 12- or 16- bit image data
		for(int index =0;index < m_frameSize/numberOfSegments;index++)
		{
			*(shortDataBuffer+index) = (*(charDataBuffer+index) << 8) + *(charDataBuffer+m_frameSize/numberOfSegments+index) ;
		}

		m_uncompressedFramePixels = (unsigned char*)shortDataBuffer;

		if (charDataBuffer)
		{
			delete [] charDataBuffer;
			charDataBuffer = 0;
		}
	}

	if (m_bitDepth == 8)
	{
		m_uncompressedFramePixels = (unsigned char*)charDataBuffer;		
	}

	m_uncompressedFramePixelsSize = m_frameSize;
	
	return kSuccess;
}
