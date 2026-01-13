/***********************************************************************
 * Compression.h
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
#ifndef COMPRESSION_H
#define COMPRESSION_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#define AWARE_ONLY

#include <vector>
#include "TICache.h"
#include "rtvPoolAccess.h"


//#include "j2kEncoder.h"

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

class Compression;
typedef RTVMapAccess<int, Compression*> MSG_TO_COMPRESSION_MAP;

class CPxDicomImage;
class Compression
{
public:
	enum
	{
		kSuccess = 0,
		kUnsupportedCompressionMethod,
		kInvalidArguments,
		kInvalidFormat,
		kMustCallSetParameters,
		kBadItemTag,
		kMissingSequenceDelimiter,
		kMemoryAllocationFailed,
		kUnsupportedBitDepth,
		kNotImplemented,
		kIJLError,
		kPegasusError,
		kRLEError,
		kJ2KError,
		kCompressionException,
		kUnknownStatus
	};

	typedef struct pixStruct
	{
		uint32 size;
		uint8* buf;
	};

	Compression();
	~Compression() 
	{ 
		if (m_offsetTable) 
			delete[] m_offsetTable; 

		cbObjectMap.Remove(m_messageID);
	}

	//
	//	Decoding API
	//
	int SetPixels(CPxDicomImage* ioImage, uint8* iPixelData, long iPixelDataSize, long iFrameSize, 
				  long iBitDepth, int iTransferSyntax, int iSamplesPerPixel);

	void SetUseStandardCodecs(int iYesNo) { m_useStandardCodecs = iYesNo; }

	//	Whole image API
	int Decode(long& oUncompressedSize);
	uint8* GetPixels() { return m_pixelData; }

	//	Single frame API
	int DecodeNextFrame(uint8*& oFramePixels);

	// decompress Gray-scale RLE compressed images, Chetan, Oct, 05.
	int DecompressGrayScaleRLE();

	//
	//	Encoding API
	//

	//	Default to lossless
	void SetEncodeAsDICOM(bool iYesNo) { m_encodeDicom = iYesNo; }
	void SetCompressionFactor(int iFactor = 1) { m_compressionFactor = iFactor; }
	void SetBigEndian(bool iYesNo = false) { m_bigEndian = iYesNo; }
	int  Encode(unsigned char* iData, int iWidth, int iHeight, int iChannels,
			    char** oData, int& oLen, int iStride=0, int iChannelDepth=8, int iSigned = 0, int iInterleaved = 1);

	static bool IsCompressed(int iSyntax);
	static bool IsLossyCompressed(int iSyntax);
	static bool IsLosslessCompressed(int iSyntax);

	static std::string GetJ2KToolkitVersionString(void);
	static std::string ErrorString(int iErrorcode);

	int GetUncompressedPixels(int CBMsgFileItemID, unsigned long CBtag, void* CBuserInfo,
							  int CBdataSize, void* CBdataBuffer, int CBisFirst, int CBisLast);

	static MSG_TO_COMPRESSION_MAP cbObjectMap;

private:
	char  Read8();
	uint16 Read16();
	uint32 Read32();

	void Write8(char iVal);
	void Write16(uint16 iVal);
	void Write32(uint32 iVal);

	void Skip(uint32);

	int UncompressFragment8(uint32);				//	IJL
	int UncompressFragmentPegasus(uint32);
	int UncompressFragmentJ2K(uint32 iLength);

//	int UncompressFragment12(uint32);				//	IJG
	int UncompressFragmentRLE(uint32);
	int CombineFragments(long& oUncompressedSize);
	int DoDecodeNextFrame(uint8*& oFramePixels);
	int DoDecodeNextEncapsulatedFrame(uint8*& oFramePixels);
	int DoEncode(unsigned char* iData, int iWidth, int iHeight, int iChannels,
			    char** oData, int& oLen, int iStride=0, int iChannelDepth=8, int iSigned = 0, int iInterleaved = 1);

	uint8* m_pixelData;								//	Compressed pixels
	uint8* m_ptr;									//	current position in Compressed Pixels
	long m_pixelDataSize;
	long m_bitDepth;
	int m_messageID;
	CPxDicomImage* m_pImage;

	std::vector<pixStruct> m_uncompressedPixels;	//	Uncompressed Pixels

	uint8* m_uncompressedFramePixels;				//	For Frame by Frame case
	uint32 m_uncompressedFramePixelsSize;
	eCompressionMethod m_compressionMethod;
	long m_frameSize;
	int m_samplesPerPixel;
	void* m_decompressContext;
	int m_transferSyntax;
	uint32* m_offsetTable;
	uint8* m_startOfFirstFragment;
	int m_frameNumber;

	int m_compressionFactor;
	bool m_bigEndian;
	bool m_encodeDicom;
	int m_useStandardCodecs;

	unsigned char*	m_OBOWbuffer;
	unsigned long	m_OBOWoffset;
	unsigned long	m_OBOWlength;

#ifndef AWARE_ONLY
	j2kEncoder m_encoder;
#endif
};

#endif // COMPRESSION_H
