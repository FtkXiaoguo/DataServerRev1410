//
//	CPxDicomImageLoad.cpp
//
#pragma warning (disable: 4786)
#pragma warning (disable: 4616)
#pragma warning (disable: 4530)

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <sys/stat.h>

#include "rtvbase.h"
#include "IDcmLibApi.h "
#include "PxDICOMUtil.h"
#include "PxDicomMessage.h"
#include "PxDicomImage.h"
//#include "Compression.h"
//#include "CPxDicomImageLoad.h"

#include "CheckMemoryLeak.h"

using namespace XTDcmLib;

MC_STATUS ImageGetPixelData(int m_messageID,unsigned long tag,void* userInfo,CALLBACK_TYPE CBtype, 
					 unsigned long* dataSizePtr,void** dataBufferPtr,int isFirst,int* isLastPtr)
{
	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	CPxDicomImage* pImage = (CPxDicomImage*) userInfo;
	return (MC_STATUS)pImage->ProcessPIXEL(tag, CBtype, dataSizePtr, dataBufferPtr, isFirst, isLastPtr);
}

//-----------------------------------------------------------------------------
int CPxDicomImage::ProcessPIXEL(unsigned long tag, int CBtype, unsigned long* dataSizePtr,
							    void** dataBufferPtr,int isFirst,int* isLastPtr)
{
/*	//	Get the pointer to the image object, theProcessHeader must created and initialiez it
	if(!m_pImage || TerminationRequested())
	{
		LogMessage("***DICOM Callback: Got null image pointer for message %d\n", m_messageID);
		return kCallbackCannotComply;
	}
*/
	//	Merge tells us how much data is coming
	if (CBtype == PROVIDING_DATA_LENGTH)
	{
		//	In the uncompressed case, the buffer size is passed in. Otherwise, it should have been set ahead
		//		of time into m_OBOWlength (such as the filesize).
		if (!IsCompressed() && *dataSizePtr != 0xFFFFffff)
		{
			m_OBOWlength = *dataSizePtr;
		} 

		if (m_OBOWlength < 1)
			return kCallbackCannotComply;;

		m_OBOWbuffer = new unsigned char[m_OBOWlength];
		if (!m_OBOWbuffer)
		{
			return kCallbackCannotComply;
		}
		return kNormalCompletion;
	}
	
	//	Merge provides data to us
	if (CBtype == PROVIDING_DATA)
	{
		//	-- - 2006.06.08 - took this out; it's initialized already in the constructor.  By resetting it 
		//		here, we were missing the offset table, which had already been copied into m_OBOWbuffer.
//		if (isFirst) 
//			m_OBOWoffset = 0;

		assert (m_OBOWoffset + *dataSizePtr <= m_OBOWlength);
		if (m_OBOWoffset + *dataSizePtr > m_OBOWlength)
		{
			return kCallbackCannotComply;						
		}

		memcpy(m_OBOWbuffer + m_OBOWoffset, *dataBufferPtr, *dataSizePtr);
		m_OBOWoffset += *dataSizePtr;
		
		if (*isLastPtr)
		{
			//	Resize the buffer to actual compressed data size
			m_OBOWlength = m_OBOWoffset;

			unsigned char* newBuffer = new unsigned char[m_OBOWlength];
			memcpy(newBuffer, m_OBOWbuffer, m_OBOWlength);
			
			delete[] m_OBOWbuffer;
			m_OBOWbuffer = newBuffer;
		}

		return kNormalCompletion;
	} 
	
	//	Merge provides data to us
	if (CBtype == PROVIDING_OFFSET_TABLE)
	{
		//	-- - 2006.06.08 - took this out; it's initialized already in the constructor.
//		if (isFirst) 
//			m_OBOWoffset = 0;

		memcpy(m_OBOWbuffer + m_OBOWoffset, *dataBufferPtr, *dataSizePtr);
		m_OBOWoffset += *dataSizePtr;

		return kNormalCompletion;
	} 

	//	Merge tells us how much data is on media
	if (CBtype == PROVIDING_MEDIA_DATA_LENGTH)
	{
		return kCallbackCannotComply;
	}
	
	//	Merge asks us how much data we will provide
	if (CBtype == REQUEST_FOR_DATA_LENGTH)
	{
		*dataSizePtr = m_OBOWlength;
		return kNormalCompletion;
	} 
	
	//	Merge asks us to provide some data
	if (CBtype == REQUEST_FOR_DATA)
	{
		*dataSizePtr = m_OBOWlength;
		*dataBufferPtr = m_OBOWbuffer;
		*isLastPtr = 1;
		return kNormalCompletion;
	}

	return kCallbackCannotComply;
};

//GL load did not reset tags before load only work for fresh created object,
//and it is not multi-thread safe, beacause MC_Register_Callback_Function for globle call back used
//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomImage::Load(const char *iFilePath, int iHeaderOnly)
{
	PxDicomStatus status;

#if 1
	m_filePath = iFilePath;

	//	Much simpler - don't need to deal with pixels
	if (iHeaderOnly)
	{
		status = LoadHeader(iFilePath, 0);
		if (status != kNormalCompletion)
		{
			status = LoadHeaderFromStream(iFilePath, 0);
			if (status != kNormalCompletion)
			{
				return status;
			}
		}
	
		// -- 2005.05.05
		// For some applications, we don't really need to extract
		// all image related stuff. We just want the messageID.
		if (iHeaderOnly != -1)
			PopulateFromMessage(m_messageID);
	
		return kNormalCompletion;
	}

	//	Make sure we have a valid message id.  If we can't get one, we're done
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	MC_STATUS		mcStatus;
	MediaCBinfo		cbinfo;
 	int				fileID;
	char			syntaxUID[65];
 	TRANSFER_SYNTAX syntax;
	static int		gImageApplicationID;
    long offset = -1;

	//	This is so that callbacks do not collide with each other.
	if (!gImageApplicationID)
	{
		mcStatus = MC_Register_Application(&gImageApplicationID, "CPxDicomImage");
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) mcStatus;
		}
	}

	//	Associate the callback defined in this .cpp file with the Application defined above
	mcStatus = MC_Register_Callback_Function(gImageApplicationID, MC_ATT_PIXEL_DATA, this, ImageGetPixelData);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) mcStatus;
	}

	mcStatus = MC_Create_Empty_File(&fileID, iFilePath);
	if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) mcStatus;  
    }
	
	//	Get the file's size, so we know how big a buffer to allocate for the pixels
	struct _stat statbuf;
	if (_stat(iFilePath, &statbuf) != 0)
	{
		return kInvalidFile;
	}
	
	long fsize = statbuf.st_size;
	if (fsize <= 0)
	{
		return kInvalidFile;
	}
	m_OBOWlength = fsize;

	//	Use merge to read the file into fileID
	mcStatus = MC_Open_File(gImageApplicationID, fileID, &cbinfo, AqMediaToFileObj);
	if (mcStatus != MC_NORMAL_COMPLETION)
    {
		return (PxDicomStatus) mcStatus;
	}

	//	Need to do this before converting from file to message
	mcStatus = MC_Get_Value_To_String(fileID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (mcStatus == MC_NORMAL_COMPLETION)
	{
		mcStatus = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) mcStatus;  
		}
	}

	//	Convert from file to message.  This drops the part 10 Meta header
	mcStatus = MC_File_To_Message(fileID); 
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) mcStatus; 
	}

	//	If it's a compressed type, we need to do this, or network transfer won't work right
	m_transferSyntax = (int) syntax;
#if 0 //TC & Rob JULY-02-2003 should always set transfer syntax
	if ((syntax != IMPLICIT_LITTLE_ENDIAN &&
		syntax != EXPLICIT_LITTLE_ENDIAN &&
		syntax != EXPLICIT_BIG_ENDIAN))
#endif
	{
		mcStatus = MC_Set_Message_Transfer_Syntax(fileID, syntax);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) mcStatus;  
		}
		/*
		* for test
		*/
		{
			TRANSFER_SYNTAX ret_syantax;
			mcStatus = MC_Get_Message_Transfer_Syntax(fileID, &ret_syantax);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				return (PxDicomStatus) mcStatus;  
			}
		}

	}

	//	In case there was already a message here, free it.  We just made a new one with the file.
	MC_Free_Message(&m_messageID);
	m_messageID = fileID;

	//	Now extract the CPxDicomImage member data
	PopulateFromMessage(m_messageID);

	//	Not compressed - just push back pixels and return
	//	TODO: If uncompressed, but multi-frame, do we need to push back each frame separately?
	if (!IsCompressed())
	{
		m_imagePixels.push_back(m_OBOWbuffer);

		//	to avoid double delete
		m_OBOWbuffer = 0;
	}
	else
	{
		//	Pixels are compressed - must decompress them
#if 0
		int stat;
		Compression compr;
		uint8* frameBuffer = 0;

		stat = compr.SetPixels(this, m_OBOWbuffer, m_OBOWlength, GetFrameSizeInBytes(), GetBitsStored(), syntax, GetSamplesPerPixel());
		if (stat != Compression::kSuccess)
		{
			if(frameBuffer) delete[] frameBuffer, frameBuffer=0;
			return kCompressionFailure;
		}

		frameBuffer = 0;

		int numberOfFrames = GetNumberOfFrames();
		long frameSize = GetFrameSizeInBytes();

		for(int frame = 0; frame < numberOfFrames; frame++)
		{
			stat = compr.DecodeNextFrame(frameBuffer);
			if (stat != kSuccess || !frameBuffer)
			{
				if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
				if(m_OBOWbuffer) delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
				return kCompressionFailure;
			} 

			m_imagePixels.push_back(frameBuffer);
		} 

		// release compressed pixel buffer
		if (m_OBOWbuffer)
		{
			delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
		}

		m_isCompressed = 0;
#endif
	}
	
	if (!m_autoConvert)
		return kNormalCompletion;

	m_converted = 0;

	m_isConverting = 0;

//	m_isConverting = 1;
//	ThreadedDataConverter* dataConverter = new ThreadedDataConverter(this);
//	iRTVThread* converterThread = new iRTVThread(dataConverter);
////	RTVOneShotThreadManager::theManager().AddRequest(dataConverter);
	
#endif
	return kNormalCompletion;
}


//-----------------------------------------------------------------------------
//
#if 0
int ThreadedDataConverter::Process()
{
	unsigned short rows = m_pImage->GetNumberOfRows();
	unsigned short columns = m_pImage->GetNumberOfColumns();
	unsigned short nPixels = rows * columns;
	unsigned short bytesPerPixel = m_pImage->GetBytesPerPixel();

	int		voxelSize	= m_pImage->GetBitsAllocated();
	int		bitsStored	= m_pImage->GetBitsStored();
	int		highBits	= m_pImage->GetHighBit();
	int		pixelRep	= m_pImage->GetPixelRepresentation();
	float	slope		= m_pImage->GetRescaleSlope();
	float	offset		= m_pImage->GetRescaleIntercept();
	int     windowWidth = m_pImage->GetWindowWidth();
	int     windowCenter= m_pImage->GetWindowCenter();
    
	unsigned char* pd = m_pImage->GetImagePixels();

	if (pd == 0)
	{
		return -1;
	}

	SetKnownMinMax(m_pImage->GetSmallestPixelValueInSeries(), m_pImage->GetLargestPixelValueInSeries());
	
	void* slice;
	slice = malloc(bytesPerPixel * nPixels); // Needs to be like this - Vikram 04/16/02
	iRTVSmallocGuard<void *> sliceGuard(slice);

	if (voxelSize == 16)
	{
		short *out = (short *)slice, *src = (short*)pd;
		
		if (m_pImage->GetModality() == kCT)
		{
			ConvertToVoxel((unsigned short *)src, nPixels, slope, offset, true, (unsigned short *)out);
		}
		else if (m_pImage->GetModality() == kMR)
		{
			short *out = (short *)slice, *src = (short*)pd;	
			
			// a(min) + b = 0
			// a(max) + b = 1<<m_bitsUsedPerPixel) -1;
			
			//int minVV = groups[idx].GetCurrentMinVoxelValue ();
			//int maxVV = groups[idx].GetCurrentMaxVoxelValue ();
			//AQNetDataConverter converter;
			//converter.RescaleMR (src, out, nPixels, minVV, maxVV);
			int minVV, maxVV;
			GetKnownMinMax(minVV, maxVV);
			RescaleMR(src, out, nPixels, minVV, maxVV);
		}
		else if (m_pImage->GetModality() == kCR)
		{
			int min, max;
			ConvertCRToVoxel((char*)src, nPixels, slope, offset,
				true, bytesPerPixel, 1, 0,(char *)out, &min, &max, &windowWidth, &windowCenter, pixelRep);
		//	gServer.Message("CR: min=%d max=%d slope=%g\n", min,max,slope);
		}
		else
		{
			ConvertToVoxel((unsigned short *)src, nPixels,0,true, (unsigned short *)out);
		}
		
	}  
	else if (bytesPerPixel == 8)
	{
		unsigned char *out = (unsigned char *)slice, *src = (unsigned char *)pd, *se;
		for ( se = src + nPixels; src < se; src++, out++)
		{
			*out = *src;
		}	
	}
	
	m_pImage->SetConversionComplete();

	return 0;
}

#endif