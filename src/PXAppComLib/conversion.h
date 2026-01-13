/***********************************************************************
 * Conversion.h
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
#ifndef COMVERSION_H_2007_05_30
#define COMVERSION_H_2007_05_30

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include <vector>
#include "TICache.h"
#include "rtvPoolAccess.h"

#ifndef RTVSTYPES
#define RTVSTYPES
typedef unsigned int	RTVSuint32;
typedef unsigned short	RTVSuint16;
typedef short			RTVSint16;
typedef int				RTVSint32;
typedef float			RTVSfloat32;
typedef double			RTVSfloat64;
typedef	unsigned char	RTVSuint8;
#endif

#define uint8			RTVSuint8
#define uint16			RTVSuint16
#define int16			RTVSint16
#define uint32			RTVSuint32
#define int32			RTVSint32
#define float32			RTVSfloat32
#define float64			RTVSfloat64

class CPxDicomImage;
class Conversion
{
public:
	enum
	{
		kCnvSuccess = 0,
		kCnvUnsupportedCompressionMethod,
		kCnvInvalidArguments,
		kCnvInvalidFormat,
		kCnvMustCallSetParameters,
		kCnvBadItemTag,
		kCnvMissingSequenceDelimiter,
		kCnvMemoryAllocationFailed,
		kCnvUnsupportedBitDepth,
		kCnvNotImplemented,
		kCnvUnknownStatus
	};

	Conversion();
	virtual ~Conversion() { };

	static std::string ErrorString(int iErrorcode);
	//
	//	Conversion API
	//
	static int ConvertYBR_FULLToRGB(CPxDicomImage* ioImage);



};

#endif // COMVERSION_H_2007_05_30
