/***********************************************************************
 * DiCOMStore.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes DiCOMStore Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "DiCOMStore.h"


#include <sys/stat.h>
#include <direct.h>
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "DiCOMListener.h"

#include "Compression.h"
#include "AuxData.h"
//#include "TRDICOMUtil.h"
#include "PxDICOMUtil.h"
#include "AppComCacheWriter.h"
#include "VLIDicom.h"
#include "PxNetDB.h"
#include "Conversion.h"

#ifndef NVR_SERVER_H_
//#include "nvr_server.h"
#endif
//extern iRTVSServerControl<int> gServer;
static TRCriticalSection cs;
/*
	// Must be called last 
	static void SetCStoreResponseOption(int, int, int);
	//	Used for setting merge logging level
    static int m_TISendSopClassUid;
    static int m_TISendSopInstanceUid;
    static int m_TISendMsgIdResponse;
    static int m_TISendResponsePriority;

int VLIDicom::m_TISendSopClassUid = kDoNothing;
int VLIDicom::m_TISendSopInstanceUid = kDoNothing;
int VLIDicom::m_TISendMsgIdResponse = kDoNothing;
int VLIDicom::m_TISendResponsePriority = kDoNothing;

//----------------------------------------------------------------------------------------
//
void VLIDicom::SetCStoreResponseOption(int SOP, int SOPClass, int msgID)
{
	m_TISendSopClassUid = SOPClass;
	m_TISendSopInstanceUid = SOP;
	m_TISendMsgIdResponse = msgID;
}

*/


//-----------------------------------------------------------------------------
// static initialize
DiCOMStore::MSG_TO_DiCOMStore_MAP DiCOMStore::map_mssageToDiCOMStore;

//-----------------------------------------------------------------------------
//

DiCOMStore::DiCOMStore(DiCOMConnectionInfo& connectInfo, int iMessageID):
	   RTVDiCOMStore(connectInfo, iMessageID)
{
	m_processorName = "DiCOMStore";
	m_PIXELbuffer = 0;
	m_PIXELoffset = 0;
	m_PIXELlength = 0;
	map_mssageToDiCOMStore.Add(iMessageID, this);
	m_head_only = false;
	m_state = kInitialized;
	m_seriesInstanceUID[0] = 0;
	m_studyInstanceUID[0] = 0;
}

//-----------------------------------------------------------------------------

DiCOMStore::~DiCOMStore()
{
	map_mssageToDiCOMStore.Remove(this);
	VLIDicom::DisconnectCMove(this);
	if(m_PIXELbuffer)
		delete[] m_PIXELbuffer, m_PIXELbuffer=0;
}


//-----------------------------------------------------------------------------
int DiCOMStore::ProcessPIXEL(unsigned long tag, int CBtype, unsigned long* dataSizePtr,
							void** dataBufferPtr,int isFirst,int* isLastPtr)
{
	//	Get the pointer to the image object, theProcessHeader must created and initialiez it
	if(!m_pImage || TerminationRequested())
	{
		LogMessage("***DICOM Callback: Got null image pointer for message %d\n", m_messageID);
		return kCallbackCannotComply;
	}

	//	Merge tells us how much data is coming
	if (CBtype == PROVIDING_DATA_LENGTH)
	{
		if (!m_pImage->IsCompressed())
		{
			m_PIXELlength = *dataSizePtr;
		} 
		else 
		{	
			m_PIXELlength = m_pImage->GetFrameSizeInBytes() * m_pImage->GetNumberOfFrames();
		}

		//	-- - 10/21/04 - Hack fix for #5367
		m_PIXELbuffer = new unsigned char[(int)(m_PIXELlength * 1.5+0.5)];
		m_PIXELoffset = 0;
		if (!m_PIXELbuffer)
		{
			LogMessage("***DICOM Callback: Couldn't allocate memory for message %d\n", m_messageID);
			return kCallbackCannotComply;
		}
		return kNormalCompletion;
	}
	
	//	Merge provides data to us
	if (CBtype == PROVIDING_DATA)
	{
		//if (isFirst) m_PIXELoffset = 0;

		assert (m_PIXELoffset + *dataSizePtr <= m_PIXELlength * 1.5);
		if (m_PIXELoffset + *dataSizePtr > m_PIXELlength * 1.5)
		{
			LogMessage("***DICOM Callback: Buffer overrun on messageID %d\n", m_messageID);
			return kCallbackCannotComply;						
		}

		memcpy(m_PIXELbuffer + m_PIXELoffset, *dataBufferPtr, *dataSizePtr);
		m_PIXELoffset += *dataSizePtr;
		
		if (*isLastPtr)
		{
			//	Resize the buffer to actual compressed data size
			m_PIXELlength = m_PIXELoffset;

			unsigned char* newBuffer = new unsigned char[m_PIXELlength];
			if (!newBuffer)
			{
				//we need to delete the pixel buffer first. 
				//The check for this is there at top.
				//delete [] m_PIXELbuffer;

				LogMessage("DiCOMStore::ProcessPIXEL() - cannot allocate memory\n");
				return kCallbackCannotComply;
			}

			memcpy(newBuffer, m_PIXELbuffer, m_PIXELlength);
			
			delete[] m_PIXELbuffer;
			m_PIXELbuffer = newBuffer;
		}

		return kNormalCompletion;
	} 
	
	//	Merge provides data to us
	if (CBtype == PROVIDING_OFFSET_TABLE)
	{
		//if (isFirst) 
		//	m_PIXELoffset = 0;

		assert(m_PIXELbuffer);
		if (!m_PIXELbuffer)
		{
			LogMessage("DiCOMStore::ProcessPIXEL() - attempt to copy offset table to null buffer\n");
			return kCallbackCannotComply;
		}
		
		memcpy(m_PIXELbuffer + m_PIXELoffset, *dataBufferPtr, *dataSizePtr);
		m_PIXELoffset += *dataSizePtr;
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
		*dataSizePtr = (m_head_only)?0:m_PIXELlength;
		return kNormalCompletion;
	} 
	
	//	Merge asks us to provide some data
	if (CBtype == REQUEST_FOR_DATA)
	{
		*dataSizePtr = (m_head_only)?0:m_PIXELlength;
		*dataBufferPtr = m_PIXELbuffer;
		*isLastPtr = 1;
		return kNormalCompletion;
	}


	LogMessage("***DICOM Callback: Got invalid CBtype %d for message id %d\n", CBtype, m_messageID);
	return kCallbackCannotComply;
}

//-----------------------------------------------------------------------------
int  DiCOMStore::Process()
{
	m_state = kEnterProcess;
	// Check to see if this thread needs to run
	if (!m_startThread) return 0;

	AqCOMThreadInit comInitGuard;

	//	Transaction Log
	LogMessage(kDebug,"TRANS: (%d) - DiCOMStore::Process() - Processing C-STORE request from (%s %s) to %s\n", m_connectInfo.AssociationID, m_connectInfo.RemoteApplicationTitle, m_connectInfo.RemoteHostName, m_connectInfo.LocalApplicationTitle); 

	int	status = theProcess();
	if(status != kSuccess)
	{
		if (status == kRetrieveAborted)
		{
			LogMessage("INFO: DiCOMStore::Process() - Retrieve aborted for SOP = %s\n", m_pImage->GetSOPInstanceUID());
		}
		else
		{
			LogMessage("ERROR: DiCOMStore::Process() - theProcess() returned %d - aborting Retrieve for SOP = %s\n", status, m_pImage->GetSOPInstanceUID());
		}

		VLIDicom::HandoverImage(this, 0); //	notifiy Vlidicom to cancell
	}
	else
	{
		if(TerminationRequested())
		{
			status = kSystemError; // cancelled by move
			LogMessage("ERROR: DiCOMStore::Process() - cancelled\n");
		}
		else
		{
			//	-- - 11/16/04: Setting m_pImage = 0 causes DiconnectCMove to fail because it can't
			//		create serialKey because it can't call m_pImage->GetSeriesInstanceUID(); causes C-MOVE to 
			//		hang for 2 minutes.  So we copy out the uid, to avoid this...
			ASTRNCPY(m_seriesInstanceUID, m_pImage->GetSeriesInstanceUID());
			ASTRNCPY(m_studyInstanceUID, m_pImage->GetStudyInstanceUID());
			if(VLIDicom::HandoverImage(this, m_pImage)) // finish image retrieve
			m_pImage = 0; //	Handover ownership
/*			if(DiCOMListener::theListener().m_config.storesOriginals)
			{
				HRESULT hr = ::CoInitialize(NULL);
				if (hr != S_OK)
				{
					LogMessage(kInfo, "INFO: DiCOMStore::Process() - initialize COM return %d\n", hr);
				}

				m_dbData.Clear ();
				m_db.InitDatabaseInfo(); //do it before use it
				//!!!!	Update database
				if(status == MC_NORMAL_COMPLETION)
				{
					//Write file to disk
				  	char fileName[MAX_PATH];
					_snprintf(fileName,sizeof fileName, "%s.dcm",m_pImage->GetSOPInstanceUID());
					SaveDicomFile(m_origDir, fileName);
				}
			}
*/
		}
	}
	
	m_state = kLeaveProcess;
	VLIDicom::DisconnectCMove(this);
	m_messageID = -1;
	if(m_PIXELbuffer)
		delete[] m_PIXELbuffer, m_PIXELbuffer=0;

	return status;
}

#ifdef _DEBUG
static int imagecount = 0;
#endif

//-----------------------------------------------------------------------------
//
int DiCOMStore::UpdateProgress()
{
	CMoveSCU* pMover = VLIDicom::FromSerialKey(this);
	if (!pMover)
		return -1;

	int nc = pMover->IncrementImagesReceivedSoFar();
	int nt = pMover->GetTotalExpectedImages();
	int nr = nt - nc;

	if (nt > 0)
	{
		// -- 2005.03.11
		// For multiimage/multiframe XA, the accouting of frames is wrong, we
		// could end up with nr < -1. For v1.5, hack the progress itself rather than
		// fixing the underline problems
		if (nr < -1)
		{
			int onc = nc;
			if (nc < 30)
				nc = 30;
			else if (nc < 50)
				nc = 50;
			else if (nc < 100)
				nc = 100;
			else if (nc < 300)
				nc = 300;
			else
				nc = 1000;
			nr = nc - onc;
		}
		pMover->Progress(nr, nc, 0, 0, kVLIRetrieve);
	}
	else
	{
		pMover->Progress(-1, nc, 0, 0, kVLIRetrieve);
	}

	return 0;
}

//-----------------------------------------------------------------------------
//
int DiCOMStore::theProcess()
{
	int status = kSuccess;
#ifdef _DEBUG
	imagecount++;
cs.Enter();
	printf("image count = %d\n", imagecount);
cs.Leave();
#endif
	
#if 0
	if(!m_PIXELlength || TerminationRequested()) return kRetrieveAborted;

	// Take care of Ca Score, Reports, etc
	if (HandleTerareconSpecific() == RTVDiCOMStore::kDoNotContinue)
		return kRetrieveAborted;
#else
	// -- 2004.02.18
	// We need to give PrivateData a chance
	
	if (TerminationRequested())
		return kRetrieveAborted;
	
	status = HandleTerareconSpecific();
	
	if (status != RTVDiCOMStore::kContinue)
	{
	       return status == RTVDiCOMStore::kDoNotContinue ? kSystemError:kRetrieveAborted;
	}
	
	if (m_PIXELlength == 0)
	{
		if (!m_hasAuxData)
		{
			LogMessage("ERROR: DiCOMStore::theProcess() retrieved faild for SOP = %s\n", m_pImage->GetSOPInstanceUID());
			return kSystemError;
		}
		return kSuccess;
	}
	// END of 2004.02.18 modification by --
#endif
	

	//	Write one to local cache
	//  -- - 03/14/03 - Added error check - bug # 3133
	status = WriteDICOMFileInCache();
	if (status < 0)
		return kSystemError;

	if(TerminationRequested())
	{
		LogMessage("INFO: DiCOMStore::theProcess - retrieve cancelled for SOP = %s\n", m_pImage->GetSOPInstanceUID());
		return kRetrieveAborted;
	}

	//	Handover ownership of whole image buffer
	std::string clssop = m_pImage->GetSOPClassUID();
	if(IsSOPClassUIDXAFamily(clssop) || IsSOPClassUIDUSFamily(clssop))
	{
		status = HandleXAImage(m_PIXELbuffer, m_PIXELlength);
	}
	// -- 2004.05.13 added NM handling
	else if (IsSOPClassUIDNM(clssop))
	{
		//	-- - 2004.07.25 make SCU side NM handling like DICOM server
		status = HandleNMImage(m_PIXELbuffer, m_PIXELlength);
  	//	status = HandleXAImage(m_PIXELbuffer, m_PIXELlength);
	}
	else if(!m_pImage->IsCompressed())
	{
		m_pImage->AddFrame(m_PIXELbuffer);
		m_PIXELbuffer = 0; // handover the buffer
		status = kSuccess;
		UpdateProgress();
	}
	else
	{
		uint8* frameBuffer = 0;
		//	-- - 06/17/02 - Decompress a frame at a time to avoid huge peak memory usage
		//	The code for decompressing the whole image at once is below in comments
		int _status;
		Compression compr;

//		compr.SetUseStandardCodecs(gServer.m_configParams.m_useStandardCodecs);
		_status = compr.SetPixels(m_pImage, m_PIXELbuffer, m_PIXELlength, 
								  m_pImage->GetFrameSizeInBytes(), m_pImage->GetBitsStored(), 
								  (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), m_pImage->GetSamplesPerPixel());
		if (_status != Compression::kSuccess)
		{
			LogMessage("ERROR: DiCOMStore::theProcess() - Decompressor failed at SetPixels - Returned %d\n", _status);
			if(frameBuffer) delete[] frameBuffer, frameBuffer=0;
			status =  kCompressionFailure;
			return kCompressionFailure;
		}

		int numberOfFrames = m_pImage->GetNumberOfFrames();
		long frameOffset = 0;
		
		for(int frame = 0; frame < numberOfFrames; frame++)
		{
			//	frameBuffer is allocated, and populated with uncompressed pixels for this frame
			frameBuffer = 0;
			_status = compr.DecodeNextFrame(frameBuffer);
			if (_status != Compression::kSuccess || !frameBuffer || TerminationRequested())
			{
				LogMessage("ERROR: DiCOMStore::theProcess() - Decompressor failed - Returned %d\n", _status);
				status =  kCompressionFailure;
				break;
			}
			m_pImage->AddFrame(frameBuffer);
			frameBuffer = 0; // handover buffer to image
			UpdateProgress();

			//fprintf(stderr,"Frame %d\n", frame);
		}
		if(frameBuffer) delete[] frameBuffer, frameBuffer=0;
	}
	return status;
}


//-----------------------------------------------------------------------------
// Handles incoming XA images
int DiCOMStore::HandleXAImage (unsigned char* pixel, unsigned long pixelLen)
{
	AppComCacheWriter cacheWriter;
	int status = kSuccess;

	bool compressed = m_pImage->IsCompressed();
	Compression compr;
	uint8* frameBuffer = 0;
	if (compressed)
	{
//		compr.SetUseStandardCodecs(gServer.m_configParams.m_useStandardCodecs);
		status = compr.SetPixels(m_pImage, pixel, pixelLen, m_pImage->GetFrameSizeInBytes(), m_pImage->GetBitsStored(), (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), m_pImage->GetSamplesPerPixel());
		if (status != Compression::kSuccess)
		{
			LogMessage("ERROR: DiCOMStore::HandleXAImage() - Decompressor failed at SetPixels - Returned %d\n", status);
			if(frameBuffer) delete[] frameBuffer, frameBuffer=0;
			return kDoNotContinue;
		}

		frameBuffer = 0;
	} 
	else
	{
		frameBuffer = pixel;
	}
	int numberOfFrames = m_pImage->GetNumberOfFrames();
	long frameSize = m_pImage->GetFrameSizeInBytes();

	for(int frame = 0; frame < numberOfFrames; frame++)
	{
		if(TerminationRequested())
		{
			if(compressed && frameBuffer) delete [] frameBuffer, frameBuffer = 0;
			return kSystemError;
		}

		if (compressed)
		{
			status = compr.DecodeNextFrame(frameBuffer);
			if (status != kSuccess || !frameBuffer )
			{
				LogMessage( "ERROR: (%d) - CStore::HandleXAImage() - Decompressor failed - Returned %d on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, status, m_pImage->GetSOPInstanceUID());
				if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
				return kSystemError;
			}

			/* -- 06/02/2003
			 * The decompressor always converts the color into RGBA, we need to set the
			 * photometricInterpretation to reflect that. Otherwise the de-Interlacing
			 * code will not get invoked.
			 */
			if (m_pImage->GetPhotometricInterpretation() == kYBR_FULL)
			{
				m_pImage->SetPhotometricInterpretation(kRGB);
			//	m_pImage->SetPlanarConfiguration(kRGBRGB);
			}
		}

		m_state = kHandleXAImage_AddXAImage_1;

		if (m_pImage->GetPhotometricInterpretation() == kYBR_FULL)
		{
			//2007.05.31 kunikichi support YBR_FULL conversion
			//return kDoNotContinue;
			if ( Conversion::ConvertYBR_FULLToRGB(m_pImage) != Conversion::kCnvSuccess )
			{
				LogMessage("Unhandled YBR image\n");
				return kDoNotContinue;
			}
		}

	
		if (m_pImage->GetImageStorageType() == kUSImage ||
			m_pImage->GetImageStorageType() == kUSMFImage ||
			m_pImage->GetImageStorageType() == kNMImage)
		{
			/* -- 06/02/2003
			 * samples per pixel is 3, we only handle RGB 
			 */
			if (m_pImage->GetSamplesPerPixel() == 3 && m_pImage->GetPhotometricInterpretation() != kRGB )
			{
				LogMessage("Unhandled YBR image\n");
				assert(0);
				return kDoNotContinue;
			}

			if (m_pImage->GetPhotometricInterpretation() == kRGB && 
				m_pImage->GetPlanarConfiguration() != kRGBRGB)
			{
				status = DeInterlaceColorPlanes(&frameBuffer, m_pImage->GetNumberOfRows(), m_pImage->GetNumberOfColumns());
			
			//	if (!status)  // fix typo 06/02/2003 --
				if ( status != 0)
				{
					LogMessage( "ERROR: (%d) - CStore::HandleXAImage() - Failed to de-interlace color planes on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, m_pImage->GetSOPInstanceUID());
					return kDoNotContinue;
				}
			}

			LogMessage(kDebug,"DEBUG: CStore::HandleXAImage() - Entering cacheWriter for instanceUID = %s\n", m_pImage->GetSOPInstanceUID());
			status = cacheWriter.AddUSImage (
							m_cacheDir, 
							m_pImage->GetModalityStr().c_str(),
							m_pImage->GetSOPInstanceUID(), 
							m_pImage->GetSOPClassUID(),
							m_pImage->GetBitsAllocated(), 
							m_pImage->GetBitsStored(), 
							m_pImage->GetHighBit(), 
							m_pImage->GetNumberOfColumns(), 
							m_pImage->GetNumberOfRows(), 
							m_pImage->IsLittleEndian(),
							m_pImage->GetImageTypeTokens(),
							frameBuffer,
							m_pImage->GetSamplesPerPixel(),
							(ePhotometricInterpretation) m_pImage->GetPhotometricInterpretation(),
							kRGBRGB, 
							kNone,
							(int)m_pImage->GetWindowWidth(),
							(int)m_pImage->GetWindowCenter(),
							m_pImage->GetPalette());				
		} else
		{
			LogMessage(kDebug,"DEBUG: Store::HandleXAImage() - Entering cacheWriter for instanceUID = %s\n", m_pImage->GetSOPInstanceUID());
			status = cacheWriter.AddXAImage (m_cacheDir, 
				m_pImage->GetModalityStr().c_str(),
				m_pImage->GetSOPInstanceUID(), 
				m_pImage->GetSOPClassUID(),
				m_pImage->GetBitsAllocated(), 
				m_pImage->GetBitsStored(), 
				m_pImage->GetHighBit(), 
				m_pImage->GetNumberOfColumns(), 
				m_pImage->GetNumberOfRows(), 
				m_pImage->IsLittleEndian(), 
				m_pImage->GetImageTypeTokens(),
				m_pImage->GetInstanceNumber(),
				(int) m_pImage->GetPixelRepresentation(),
				frameBuffer, 
				(ePhotometricInterpretation) m_pImage->GetPhotometricInterpretation(),
				m_pImage->GetReferencedSOPInstanceUID(), 
				(int)m_pImage->GetWindowWidth(), 
				(int)m_pImage->GetWindowCenter(),
				0,kNone,
				m_pImage->GetRescaleSlope(),
				m_pImage->GetRescaleIntercept());
		}
		LogMessage(kDebug,"DEBUG: CStore::HandleXAImage() - returned from cacheWriter for instanceUID = %s\n", m_pImage->GetSOPInstanceUID());
		UpdateProgress();

		if (compressed)
		{
		 	if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		}
		else
		{
			frameBuffer += frameSize;
		}

		if (status != kSuccess)
		{
			LogMessage( "ERROR: (%d) - CStore::HandleXAImage() - CacheWriter failed to add image compressed data loop- Returned %d on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, status, m_pImage->GetSOPInstanceUID());
			return status;
		}

		
	} 
	
	return status;
}

#if 1
//-----------------------------------------------------------------------------
//
int DiCOMStore::HandleNMImage (unsigned char* pixel, unsigned long pixelLen)
{
#if 0
	int status = kSuccess;
	int id = m_pImage->GetID();

	//	Assume not compressed
	if (m_pImage->IsCompressed())
	{
		LogMessage("WARNING: no support for compressed NM - could not process SOP=%s\n", m_pImage->GetSOPInstanceUID());
		return kDoNotContinue;
	}

	AppComCacheWriter cacheWriter;
	m_state = kHandleNMImage_AddNMImage;
	int _status;
	unsigned char* framePtr = pixel;
	long frameSize = m_pImage->GetFrameSizeInBytes();

	//	Process each frame

	CNMObject nmObject(m_pImage);
	for(int i=0; i<m_pImage->GetNumberOfFrames(); i++)
	{
		_status = cacheWriter.AddNMImage (m_cacheDir, 
										m_pImage->GetModalityStr().c_str(),
										m_pImage->GetSOPInstanceUID(),
										m_pImage->GetSOPClassUID(),
										m_pImage->GetBitsAllocated(),
										m_pImage->GetBitsStored(),
										m_pImage->GetHighBit(), 
										m_pImage->GetNumberOfColumns(),
										m_pImage->GetNumberOfRows(),
										m_pImage->IsLittleEndian(),
										m_pImage->GetImageTypeTokens(),
										nmObject,
										m_pImage->GetPixelSpacing(),
										m_pImage->GetRescaleSlope(),
										m_pImage->GetRescaleIntercept(),
										i,
										m_pImage->GetPixelRepresentation(),
										framePtr,
										0,
										kNone,
										m_pImage->GetWindowWidth (),
										m_pImage->GetWindowCenter ());
		if (_status != kSuccess)
		{
			LogMessage( "ERROR: (%d) - DiCOMStore::HandleNMImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, _status, m_pImage->GetSOPInstanceUID());
			return kDoNotContinue;
		}
		UpdateProgress();
		framePtr += frameSize;
	}

	return status;
#else
	return 0;
#endif
}
#else
//--------------------------------------------------------------------------
// Handles incoming NM images
// Added -- 2004.05.13
int DiCOMStore::HandleNMImage (unsigned char* pixel, unsigned long pixelLen)
{
	AppComCacheWriter cacheWriter;
	int status = kSuccess;
	char*	refSOP = 0;

	int                       numberOfImageTypeTokens = 0;
	std::vector <std::string> ImageTypeTokens;
	int numberOfPlanes = 1;


	GetImageTypeTokens (ImageTypeTokens, numberOfImageTypeTokens);

	if (numberOfImageTypeTokens >= 3)
	{
		
	} // if (numberOfImageTypeTokens >= 3)

	bool compressed = m_pImage->IsCompressed();
	Compression compr;
	uint8* frameBuffer = 0;
	if (compressed)
	{
		compr.SetUseStandardCodecs(gServer.m_configParams.m_useStandardCodecs);
		status = compr.SetPixels(m_messageID, pixel, pixelLen, m_pImage->GetFrameSizeInBytes(), m_pImage->GetBitsStored(), (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), m_pImage->GetSamplesPerPixel());
		if (status != Compression::kSuccess)
		{
			LogMessage("ERROR: DiCOMStore::HandleXAImage() - Decompressor failed at SetPixels - Returned %d\n", status);
			if(frameBuffer) delete[] frameBuffer, frameBuffer=0;
			return kDoNotContinue;
		}

		frameBuffer = 0;
	} 
	else
	{
		frameBuffer = pixel;
	}

	int numberOfFrames = m_pImage->GetNumberOfFrames();
	long frameSize = m_pImage->GetFrameSizeInBytes();

	for(int frame = 0; frame < numberOfFrames; frame++)
	{
		if(TerminationRequested())
		{
			if(compressed && frameBuffer) delete [] frameBuffer, frameBuffer = 0;
			return kSystemError;
		}

		if (compressed)
		{
			status = compr.DecodeNextFrame(frameBuffer);
			if (status != kSuccess || !frameBuffer )
			{
				LogMessage( "ERROR: (%d) - CStore::HandleXAImage() - Decompressor failed - Returned %d on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, status, m_pImage->GetSOPInstanceUID());
				if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
				return kSystemError;
			}

			/* -- 06/02/2003
			 * The decompressor always converts the color into RGBA, we need to set the
			 * photometricInterpretation to reflect that. Otherwise the de-Interlacing
			 * code will not get invoked.
			 */
			if (m_pImage->GetPhotometricInterpretation() == kYBR_FULL)
			{
				m_pImage->SetPhotometricInterpretation(kRGB);
			//	m_pImage->SetPlanarConfiguration(kRGBRGB);
			}
		}

		m_state = kHandleXAImage_AddXAImage_1;

		if (m_pImage->GetPhotometricInterpretation() == kYBR_FULL)
		{
			LogMessage("Unhandled YBR image\n");
			return kDoNotContinue;
		}

	
		if (m_pImage->GetImageStorageType() == kNMImage )
		{
			/* -- 06/02/2003
			 * samples per pixel is 3, we only handle RGB 
			 */
			if (m_pImage->GetSamplesPerPixel() == 3 && m_pImage->GetPhotometricInterpretation() != kRGB )
			{
				LogMessage("Unhandled YBR image\n");
				assert(0);
				return kDoNotContinue;
			}

			if (m_pImage->GetPhotometricInterpretation() == kRGB && 
				m_pImage->GetPlanarConfiguration() != kRGBRGB)
			{
				status = DeInterlaceColorPlanes(&frameBuffer, m_pImage->GetNumberOfRows(), m_pImage->GetNumberOfColumns());
			
			//	if (!status)  // fix typo 06/02/2003 --
				if ( status != 0)
				{
					LogMessage( "ERROR: (%d) - CStore::HandleXAImage() - Failed to de-interlace color planes on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, m_pImage->GetSOPInstanceUID());
					return kDoNotContinue;
				}
			}

			LogMessage(kDebug,"DEBUG: CStore::HandleNMImage() - Entering cacheWriter for instanceUID = %s\n", m_pImage->GetSOPInstanceUID());
			status = cacheWriter.AddPTImage (m_cacheDir,
									m_pImage->GetModalityStr().c_str(),
									m_pImage->GetSOPInstanceUID(),
									m_pImage->GetSOPClassUID(),
									m_pImage->GetBitsAllocated(),
									m_pImage->GetBitsStored(),
									m_pImage->GetHighBit(), 
									m_pImage->GetNumberOfColumns(),
									m_pImage->GetNumberOfRows(),
									m_pImage->IsLittleEndian(),
									m_pImage->GetImageTypeTokens(),
									m_pImage->GetImagePosition(),
									m_pImage->GetImageOrientation(),
									m_pImage->GetPixelSpacing(),
									m_pImage->GetRescaleSlope(),
									m_pImage->GetRescaleIntercept(),
									m_pImage->GetInstanceNumber(),
									m_pImage->GetPixelRepresentation(),
									frameBuffer,
									0,
									kNone,
									m_pImage->GetWindowWidth (),
									m_pImage->GetWindowCenter ());			
		} 
	
		LogMessage(kDebug,"DEBUG: CStore::HandleNMImage() - returned from cacheWriter for instanceUID = %s\n", m_pImage->GetSOPInstanceUID());
		UpdateProgress();

		if (compressed)
		{
		 	if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		}
		else
		{
			frameBuffer += frameSize;
		}

		if (status != kSuccess)
		{
			LogMessage( "ERROR: (%d) - CStore::HandleXAImage() - CacheWriter failed to add image compressed data loop- Returned %d on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, status, m_pImage->GetSOPInstanceUID());
			return status;
		}
	} 
	
	return status;
}
#endif

//-------------------------------------------------------------------
void DiCOMStore::LogProcessStatus(void)
{
	char *pState = "Unknown";
	switch(m_state)
	{
		case kInitialized:
			pState = "Initialized";
			break;
		case kEnterPreprocess:
			pState = "EnterPreprocess";
			break;
		case kdbSaveRecord:
			pState = "db SaveRecord";
			break;
		case kHandleTerareconSpecific:
			pState = "In HandleTerareconSpecific";
			break;
		case kLeavePreprocess:
			pState = "LeavePreprocess";
			break;
		case kEnterProcess:
			pState = "Enter Process";
			break;
		case kdbUpdateFileSize:
			pState = "Enter db UpdateFileSize";
			break;
		case kHandleCRImage_AddXAImage:
			pState = "In HandleCRImage AddXAImage";
			break;
		case kHandleCTImage_AddCTImage:
			pState = "In HandleCTImage AddCTImage";
			break;
		case kHandleMRImage_AddMRImage:
			pState = "In HandleMRImage AddMRImage";
			break;
		case kHandleSCImage_AddSCImage:
			pState = "In kHandleSCImage AddSCImage";
			break;
		case kHandleXAImage_AddXAImage_1:
			pState = "In HandleXAImage AddXAImage 1";
			break;
		case kHandleXAImage_AddXAImage_2:
			pState = "In HandleXAImage AddXAImage 2";
			break;
		case kHandleXAImage_AddXAImage_3:
			pState = "In HandleXAImage AddXAImage 3";
			break;
		case kHandleXAImage_AddXAImage_4:
			pState = "In HandleXAImage AddXAImage 4";
			break;
		case kLeaveProcess:
			pState = "LeaveProcess";
			break;
		case kEnterDestructor:
			pState = "EnterDestructor";
			break;
		case kLeaveDestructor:
			pState = "LeaveDestructor";
			break;
	}
	LogMessage("STATE_INFO: %s ID=(%d,%d) in state: %s\n", m_processorName, m_connectInfo.AssociationID, m_messageID, pState);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int DiCOMStore::CoerceSOPInstanceUID ()
{
	//	Transaction Log
	//LogMessage(kDebug,"TRANS: (%d) - DiCOMStore::CoerceSOPInstanceUID() - Attempting to store SOPInstanceUID = %s\n", m_connectInfo.AssociationID, m_pImage->GetSOPInstanceUID());
	// registor this to VLIDicom as soon as possible. (in preprocess)
	if (VLIDicom::m_testMoveOriginator)
	{
		LogMessage(kInfo, "INFO: C-STORE Lookup Key for [%s,%s]\n", 
			m_pImage->GetSeriesInstanceUID(), ConnectInfo()->RemoteApplicationTitle);
	}
	else
	{
		LogMessage(kInfo, "INFO: C-STORE Lookup Key for [%s,%s]\n",
			m_pImage->GetSeriesInstanceUID(), ConnectInfo()->RemoteIPAddress);
	}

	if(!VLIDicom::ConnectCMove(this))
	{
		LogMessage(kDebug, "ERROR: - DiCOMStore::CoerceSOPInstanceUID - could not found Move handler for series %s on host %s\n", 
			m_pImage->GetSeriesInstanceUID(), m_connectInfo.RemoteIPAddress);
		m_errorResponseStatus = MC_SYSTEM_ERROR;
		return MC_SYSTEM_ERROR;
	}
	return MC_NORMAL_COMPLETION;
}


#include "diskspacemanager.h"
//-----------------------------------------------------------------------------
int DiCOMStore::WriteDICOMFileInCache()
{
	std::string fileFilter = "";
	
	// -- 2004.04.21
	// Before we write the file, check if we already have the series on disk [local retrieve]
	// We do this to avoid potential duplicate message problems
	std::string fileSpec = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom (
							m_pImage->GetSeriesInstanceUID(),m_pImage->GetStudyInstanceUID());
	
	if (!fileSpec.empty())
	{
		fileSpec += "*.dcm";
		struct _finddata_t cfile;
		long h;
		if (( h = _findfirst(fileSpec.c_str(), &cfile)) != -1)
		{
			_findclose(h);
			return kSuccess;
		}
	}

	char fileName[MAX_PATH];
	_snprintf(fileName,sizeof fileName, "%s.dcm",m_pImage->GetSOPInstanceUID());

	bool writeToDisk = true;
	int storageType = m_pImage->GetImageStorageType();
	if (storageType == kCTImage || storageType == kMRImage)
	{
		//
		//	First check to see if there's already one there
		//
		long hFile;
		struct _finddata_t cfile;
		fileFilter = m_cacheDir + std::string("/*.dcm");

		hFile = _findfirst(fileFilter.c_str(), &cfile);

		// Write it to disk
		if (hFile != -1)
		{
			writeToDisk = false;
		}

		_findclose(hFile);
	}

	if (writeToDisk) // no error check here because save file here is optional
		SaveDicomFile(m_cacheDir, fileName, true);// != MC_NORMAL_COMPLETION)

	return kSuccess;
}

//-----------------------------------------------------------------------------------------
//
int DiCOMStore::SaveDicomFile(const char* iSavePath, const char* iFileName, bool head_only)
{
	char fileName[kMaxPathNameLen];
    CBinfo callbackInfo;
	PxDicomStatus status;
	
	//	Build file name
	strcpy(fileName, iSavePath);
	strcat(fileName, "/"); 
	strcat(fileName, iFileName);
	
	//
	//	Write the image to a file
	//

	//	-- - 2005.11.16 - Need to do this, otherwise Merge callbacks get confused about
	//		(7fe0,0010) PixelData embedded within the SQ
	MC_Delete_Attribute(m_messageID, MC_ATT_ICON_IMAGE_SEQUENCE);

	int fileID = -1;

//#define NO_DUP
#if defined(NO_DUP)

	fileID = m_messageID;

	status = (PxDicomStatus) MC_Message_To_File(fileID,  fileName);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("** DICOM ERROR (%s,%d): at MC_Message_To_File() for file %s\n", DicomStatusString(status), status, iFileName);
		MC_Free_File(&fileID);
		return status;
	}
		
	status = (PxDicomStatus) TRDICOMUtil::AddGroup2Elements(fileID, (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), m_connectInfo.LocalApplicationTitle);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("** DICOM ERROR (%s,%d): at AddGroup2Elements() for file %s\n", DicomStatusString(status), status, iFileName);
		MC_Free_File(&fileID);
		return status;
	}
	
	//	-- - 2004.04.27
	//	So the callback can check if it's passed too much data
	status = (PxDicomStatus) MC_Get_File_Length(fileID, &callbackInfo.dataSize);
	if (status != MC_NORMAL_COMPLETION)
		callbackInfo.dataSize = 0;

	status = (PxDicomStatus) MC_Write_File(fileID, 0, &callbackInfo, TRDICOMUtil::FileObjToMedia);
	if (status != MC_NORMAL_COMPLETION && !head_only)
	{
		LogMessage("** DICOM ERROR (%s,%d): at MC_Write_File() for file %s\n", DicomStatusString(status), status, iFileName);
	}

	if (callbackInfo.fp)
		fclose(callbackInfo.fp);

	//	-- - 2004.04.27
	//	Convert it back to a message object - so it can be used later on
	status = (PxDicomStatus) MC_File_To_Message(fileID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage("** DICOM ERROR (%s,%d): at MC_File_To_Message() for file %s\n", DicomStatusString(status), status, iFileName);
		MC_Free_File(&fileID);
		return status;
	}

#else
	status = (PxDicomStatus) MC_Duplicate_Message(m_messageID, &fileID, (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), 0, 0);
	if (head_only && status == MC_CALLBACK_CANNOT_COMPLY)
		//	In this case, the pixel data callbacks failed, but we don't care
		;
	else if (status != MC_NORMAL_COMPLETION)
	{
//		LogMessage("** DICOM ERROR (%s,%d): at MC_Duplicate_Message() for file %s\n", DicomStatusString(status), status,iFileName);
		MC_Free_File(&fileID);
		return status;
	}

	status = (PxDicomStatus) MC_Message_To_File(fileID,  fileName);
	if (status != MC_NORMAL_COMPLETION)
	{
//		LogMessage("** DICOM ERROR (%s,%d): at MC_Message_To_File() for file %s\n", DicomStatusString(status), status, iFileName);
		MC_Free_File(&fileID);
		return status;
	}
		
	status = (PxDicomStatus) TRDICOMUtil::AddGroup2Elements(fileID, (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), m_connectInfo.LocalApplicationTitle);
	if (status != MC_NORMAL_COMPLETION)
	{
//		LogMessage("** DICOM ERROR (%s,%d): at AddGroup2Elements() for file %s\n", DicomStatusString(status), status, iFileName);
		MC_Free_File(&fileID);
		return status;
	}
	
	//	-- - 2004.04.27
	//	So the callback can check if it's passed too much data
	status = (PxDicomStatus) MC_Get_File_Length(fileID, &callbackInfo.dataSize);
	if (status != MC_NORMAL_COMPLETION)
		callbackInfo.dataSize = 0;

	map_mssageToDiCOMStore.Add(fileID, this);
	m_head_only = head_only;
	status = (PxDicomStatus) MC_Write_File(fileID, 0, &callbackInfo, AqFileObjToMedia);
	if (status != MC_NORMAL_COMPLETION)
	{
//		LogMessage("** DICOM ERROR (%s,%d): at MC_Write_File() for file %s\n", DicomStatusString(status), status, iFileName);
	}
	map_mssageToDiCOMStore.Remove(fileID);
	m_head_only = false;

	if (callbackInfo.fp)
		fclose(callbackInfo.fp);

	MC_Free_File(&fileID);
#endif /* !NO_DUP */

	//This was to test that the write / read all worked.
	//VLIDicomImage* image;
	//status = (PxDicomStatus) LoadDicomFile(image, fileName);

	return status;
}

//-----------------------------------------------------------------------------
// This function handles Terarecon specific files that are pushed
//
int DiCOMStore::HandleTerareconSpecific ()
{
	CPxDB	db;
	int	_status;
	struct _stat statBuf;
	m_state = kHandleTerareconSpecific;

	//rintf (m_seriesDir, "%s%s/%s", m_originalRootDir,
	//_pImage->GetStudyInstanceUID(), m_pImage->GetSeriesInstanceUID()); 
	if(TerminationRequested())
	{
		LogMessage(kDebug, "ERROR: - DiCOMStore::HandleTerareconSpecific() - cancelled\n");
		m_errorResponseStatus = MC_SYSTEM_ERROR;
		return MC_SYSTEM_ERROR;
	}
	if(!VLIDicom::GetCacheDir(this, m_cacheDir, sizeof(m_cacheDir)))
	{
		LogMessage(kDebug, "ERROR: - DiCOMStore::HandleTerareconSpecific() - failed to get cache dir\n");
		m_errorResponseStatus = MC_SYSTEM_ERROR;
		return MC_SYSTEM_ERROR;
	}

	// get rid of tail slash
	int str_end = strlen(m_cacheDir)-1;
	if(m_cacheDir[str_end] == '/' || m_cacheDir[str_end] == '\\')
		m_cacheDir[str_end] = 0;

    //sprintf (m_cacheDir, "%s%s/%s", m_cacheRootDir,m_pImage->GetStudyInstanceUID(), m_pImage->GetSeriesInstanceUID()); 

	//
	//	Check for existence of the Original Directory	
	//
	//
	//	It doesn't exist - create it
	//
/*
	if (_stat(m_origDir, &statBuf))
	{
		//	It doesn't exist - create it
		_status = TRPlatform::MakeDirIfNeedTo(m_origDir);
		if (_status < 0)
		{
			LogMessage("ERROR: (%d) - DiCOMStore::InitMessageStore() - TRPlatform::MakeDirIfNeedTo failed for dir %s on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, m_origDir, m_pImage->GetSOPInstanceUID());
			m_errorResponseStatus = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
			return MC_SYSTEM_ERROR;
		}
	}
*/	
	//	Check for existence of the Cache sub-directory	
	if (_stat(m_cacheDir, &statBuf))
	{
		//	It doesn't exist - create it
		_status = TRPlatform::MakeDirIfNeedTo(m_cacheDir);
		if (_status < 0)
		{
			LogMessage( "ERROR: (%d) - DiCOMStore::InitMessageStore() - TRPlatform::MakeDirIfNeedTo failed for dir %s on SOPInstanceUID = %s\n", m_connectInfo.AssociationID, m_cacheDir, m_pImage->GetSOPInstanceUID());
			m_errorResponseStatus = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
			return MC_SYSTEM_ERROR;		
		}
	}

	int status = 0;

	AuxData auxData;


	//	Do we have any aux data?
//	if (auxData.Init(m_messageID, m_pImage->GetStudyInstanceUID(), m_pImage->GetSeriesInstanceUID(), m_pImage->GetSOPInstanceUID()) != 0)
//	{	
//		return RTVDiCOMStore::kContinue;	
//	}

	// tcz 2006.05.08 enhanced this a little: return positive to inidcate
	// having none private data. keep the negative as error status
	int ret = auxData.Init(m_messageID, m_pImage->GetStudyInstanceUID(), m_pImage->GetSeriesInstanceUID(), m_pImage->GetSOPInstanceUID());
	if (ret != 0)
	{
		if (ret < 0)
			LogMessage("ERROR:CStore::HandleTerareconSpecific() failed. type=%s\n", auxData.m_auxData.m_subtype);
		return RTVDiCOMStore::kContinue;
	}

	LogMessage(kDebug,"TRANS: (%d) - CStore::HandleTerareconSpecific() - TeraRecon Private Data encountered\n", m_connectInfo.AssociationID);
	m_hasAuxData = true;

	//	Update the database
	db.InitDatabaseInfo(); // use this object as logger
	if (db.SaveAuxDataInfo(auxData.m_auxData, auxData.m_auxReferencs) != kOK)
	{
		LogMessage("Can't save AuxDataInfo: %s\n", auxData.m_auxData.m_name);
		return RTVDiCOMStore::kDoNotContinue;

	}

	return SaveBinaryData(auxData.m_auxData, auxData.IsOldCaScore());
}


const char* DiCOMStore::GetSeriesInstanceUID() const 
{
	const char* p = "";
	if(m_pImage)
		p = m_pImage->GetSeriesInstanceUID();
	else
		return m_seriesInstanceUID;
	return p;
}

const char* DiCOMStore::GetStudyInstanceUID() const 
{
	const char* p = "";
	if(m_pImage)
		p = m_pImage->GetStudyInstanceUID();
	else
		return m_studyInstanceUID;
	return p;
}