/***********************************************************************
 * RTVDiCOMStore.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes RTVDiCOMStore Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */

#include <assert.h>
#include "RTVDiCOMStore.h"


#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "AuxData.h"
#include "PxDicomutil.h"
#include "PxDicomImage.h"

#include <io.h>

long RTVDiCOMStore::c_totalInProcessing = 0;
long RTVDiCOMStore::c_maxStoreThread = 100;
long RTVDiCOMStore::c_totalCompressedThread = 0;
long RTVDiCOMStore::c_maxCompressedThread = 12;

//-----------------------------------------------------------------------------------------
//
MC_STATUS GetPixelData(int messageID,unsigned long tag,void* userInfo, 
					   int dataSize,void* dataBufferPtr,int isFirst,int isLast)
{
	if(!TRDICOMUtil::IsMainLevelObject(messageID))
		return MC_NORMAL_COMPLETION;

	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	
	RTVDiCOMStore* pRTVDiCOMStore = (RTVDiCOMStore*) userInfo;
	return (MC_STATUS)pRTVDiCOMStore->HandoverPixelData(dataSize, dataBufferPtr, isFirst, isLast);
}


//-----------------------------------------------------------------------------
RTVDiCOMStore::RTVDiCOMStore (DiCOMConnectionInfo& connectInfo, int iMessageID): 
		RTVDiCOMService(connectInfo, iMessageID)
{
    //strncpy(m_mediaLabel, iMediaLabel, sizeof(m_mediaLabel));
	m_cacheDir[0] = 0;
	m_seriesDir[0] = 0;
	m_seriesUID[0] = 0;

	m_pImage = 0;

	m_errorResponseStatus = C_STORE_SUCCESS;
	m_errorResponseReason[0] = 0;

	m_startThread = true;
	m_preprocessed =  false;

	m_hasAuxData = false;
	m_OBOWbuffer = 0;
	m_OBOWoffset = 0;
	m_OBOWlength = 0;

	m_state = kInitialized;
	m_inProcessing = false;
	m_isCompressed = false;
	m_responsed = false;

}

//-----------------------------------------------------------------------------

RTVDiCOMStore::~RTVDiCOMStore()
{
	if(m_inProcessing)
	{
		if(!m_responsed)
		{
			m_errorResponseStatus = C_STORE_SUCCESS;
			MessageReceiveFinish(m_errorResponseStatus);
		}
		m_inProcessing = false;
		InterlockedDecrement(&c_totalInProcessing);
		if(m_isCompressed)
			InterlockedDecrement(&c_totalCompressedThread);
	}

	if(m_pImage != 0)
		delete m_pImage, m_pImage=0;
	//MC_Free_Message(&m_messageID); //it should be freed by m_pImage
	CleanUp ();
}

//-----------------------------------------------------------------------------
void RTVDiCOMStore::CleanUp ()
{
	//	Release memory
	if(m_OBOWbuffer)
	{
		delete[] m_OBOWbuffer;
		m_OBOWbuffer = 0;
		m_OBOWoffset = 0;
		m_OBOWlength = 0;
	}
}


//-----------------------------------------------------------------------------
int RTVDiCOMStore::HandoverPixelData(int dataSize,void* dataBufferPtr,int isFirst,int isLast)
{
	if (isFirst)
	{
		//
		//	Get the filesize
		//
		if (m_OBOWlength < 1)
		{
			LogMessage( "ERROR: (%d) - RTVDiCOMStore::HandoverPixelData() - Invalid Filesize: %d for message id %d\n", m_connectInfo.AssociationID, m_OBOWlength, m_messageID);
			FlushLog();
			return MC_CALLBACK_CANNOT_COMPLY;
		}
		//
		//	Allocate the memory
		//
		if(m_OBOWbuffer)
			delete[] m_OBOWbuffer;
//		m_OBOWbuffer = new unsigned char[m_OBOWlength + 2];

		//	02/16/04 - -- - add 1024 to buffer allocation to avoid
		//		crash bug with J2KLib (last memcpy in kakadu does not adjust
		//		copy of compressed data at end of buffer - just copies 512 bytes regardless
		//		of the actual remaining size, causing a read beyond the array-bounds.
		m_OBOWbuffer = new unsigned char[m_OBOWlength + 2 + 1024];
		if (m_OBOWbuffer == NULL)
		{
			m_OBOWlength = 0;
			LogMessage( "ERROR: (%d) - RTVDiCOMStore::HandoverPixelData() - Failed to allocate buffer for message id %d\n", m_connectInfo.AssociationID, m_messageID );
			FlushLog();
			return MC_CALLBACK_CANNOT_COMPLY;
		} 
		m_OBOWoffset = 0;
	}

	if (dataSize > 0)
	{
#if 0
		assert(m_OBOWoffset + dataSize <= m_OBOWlength);
		if (m_OBOWoffset + dataSize > m_OBOWlength)
		{
			LogMessage( "ERROR: (%d) - RTVDiCOMStore::HandoverPixelData() - data bigger than buffer for message id %d\n", m_connectInfo.AssociationID, m_messageID );
			FlushLog();
			return MC_CALLBACK_CANNOT_COMPLY;
		}

		memcpy((m_OBOWbuffer+m_OBOWoffset), dataBufferPtr, dataSize);
		m_OBOWoffset += dataSize;
#else
		if (m_OBOWoffset + dataSize > m_OBOWlength)
		{
			const char* studyInstanceUID = "unknown";
			if(m_pImage)
				studyInstanceUID = m_pImage->GetStudyInstanceUID();
			LogMessage( "ERROR: (%d) - RTVDiCOMStore::HandoverPixelData() - data(%d) bigger than "
				"buffer(%d) for association id: %d - ignoring extra bytes for studyUID:%s\n", 
				m_messageID, m_OBOWoffset + dataSize, m_OBOWlength,
				m_connectInfo.AssociationID, studyInstanceUID );
			FlushLog();
		}
		else
		{
			memcpy((m_OBOWbuffer+m_OBOWoffset), dataBufferPtr, dataSize);
		}

		m_OBOWoffset += dataSize;
#endif
	
	}

#ifdef _DEBUG
	if (isLast)
	{
		assert(m_OBOWoffset <= m_OBOWlength);
	}
#endif

	return (MC_STATUS) kNormalCompletion;

}


int RTVDiCOMStore::PreProcess ()
{
	return ProcessHeader();
}

void RTVDiCOMStore::MessageReceiveFinish(int state)
{
	if (m_errorResponseReason[0] != '\0')
		SendResponseMessage(state, C_STORE_RSP, m_errorResponseReason);
	else
		SendResponseMessage(state, C_STORE_RSP);

	m_responsed = true;
}

int RTVDiCOMStore::ProcessHeader(ResponceSpeed rspSpeed)
{
	if(m_preprocessed)
		return MC_NORMAL_COMPLETION;

	m_inProcessing = true;
	InterlockedIncrement(&c_totalInProcessing);

	if(m_isCompressed)
		InterlockedIncrement(&c_totalCompressedThread);	

	m_errorResponseStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
	m_preprocessed = true;
	m_state = kEnterPreprocess;

	int	status;
	// This variable is checked in the process to see if it needs to run.
	m_startThread = true;

	//	Transaction Log
	LogMessage(kDebug,"TRANS: (%d) - RTVDiCOMStore::Preprocess() - Processing C-STORE request from (%s %s) to %s\n", m_connectInfo.AssociationID, m_connectInfo.RemoteApplicationTitle, m_connectInfo.RemoteHostName, m_connectInfo.LocalApplicationTitle); 
	LogMessage(kDebug,"TRANS: (%d) - RTVDiCOMStore::Preprocess() - BEGIN --\n", m_connectInfo.AssociationID); 

	// Get the Service Name and Command for SendResponseMessage
	status = MC_Get_Message_Service(m_messageID, &m_serviceName, &m_command);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_errorResponseStatus = C_STORE_FAILURE_CANNOT_UNDERSTAND;
		MessageReceiveFinish(m_errorResponseStatus);
		LogMessage( "ERROR: (%d) - RTVDiCOMStore::Preprocess() - DcmLib error %d on MC_Get_Message_Service() - failed to get service name and command\n", m_connectInfo.AssociationID, status);
		m_startThread = false;
		m_state = kLeavePreprocess;
		return status;
	}
	
	status = theProcessHeader();


	if(status != MC_NORMAL_COMPLETION)
	{
		//m_errorResponseStatus = C_STORE_FAILURE_CANNOT_UNDERSTAND;
		MessageReceiveFinish(m_errorResponseStatus);
		m_startThread = false;
		m_state = kLeavePreprocess;
	}
	else if(rspSpeed == kEarlyResponce)
	{
		m_errorResponseStatus = C_STORE_SUCCESS;
		MessageReceiveFinish(m_errorResponseStatus);
	}

	LogMessage(kDebug,"TRANS: (%d) - RTVDiCOMStore::Preprocess() - END   --\n", m_connectInfo.AssociationID); 
	m_state = kLeavePreprocess;
	return status;
}

//-----------------------------------------------------------------------------
// Vikram 02/27/02 - Code Review Changes
// This function takes an incoming Dicom Image and stores it to disk and sends a 
// C_STORE_SUCESS Message if all goes well.
// This function also creates the follwing directories under the "DICOMCacheLocation" 
// directory:
// - SeriesLevel Directory same as SeriesInstanceUID
// - SeriesLevel/Original directory.
// - SeriesLevel/Cache directory.

int  RTVDiCOMStore::theProcessHeader()
{
	int status;

	if(m_pImage) delete m_pImage, m_pImage=0;

	m_pImage = new CPxDicomImage(m_messageID); //from here the messageID should be freed by m_pImage

	if (!m_pImage || m_pImage->GetStatus() != kNormalCompletion)
	{
	   char  sopInstanceUID[kVR_UI];
       MC_Get_Value_To_String(m_messageID, MC_ATT_SOP_INSTANCE_UID, sizeof(sopInstanceUID), sopInstanceUID);
	   LogMessage("ERROR: CPxDicomImage contructor failed for SOPInstanceUId=%s \n", sopInstanceUID);
	   m_errorResponseStatus = C_STORE_FAILURE_INVALID_DATASET;
	   return MC_SYSTEM_ERROR;
	}

	//	Transaction Log
	LogMessage(kDebug,"TRANS: (%d) - RTVDiCOMStore::theProcessHeader - Attempting to store SOPInstanceUID = %s\n", 
		m_connectInfo.AssociationID, m_pImage->GetSOPInstanceUID());
	
	m_isCompressed = m_pImage->IsCompressed();
	if(m_isCompressed)
		InterlockedIncrement(&c_totalCompressedThread);	

	// Coerce the SOP Instance UID if necessary
	status = CoerceSOPInstanceUID (); 
	if (status != MC_NORMAL_COMPLETION)
		return status;

	//
	//	Log the transfer syntax
	//
	//if(m_logger.GetVerbose() >= kDebug)
	if (GetAqLogger()->GetLogLevel() >= kDebug)
	{
		char transferSyntaxUID[kVR_UI];
		status = MC_Get_Transfer_Syntax_From_Enum((TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), transferSyntaxUID, kVR_UI);
		if (status == MC_NORMAL_COMPLETION)
			LogMessage("TRANS: (%d) - RTVDiCOMStore::Preprocess() - Transfer Syntax = %s\n", m_connectInfo.AssociationID, transferSyntaxUID);
	}

	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------------------
//
//	Convert an image from [r1r2r3... g1g2g3... b1b2b3...] to [r1g1b1 r2g2b2 r3g3b3 ...]  
//
int RTVDiCOMStore::DeInterlaceColorPlanes(unsigned char** ioFrameBuffer, int iRows, int iColumns)
{
	if (!ioFrameBuffer || !(*ioFrameBuffer))
		return -1;

	unsigned long nPixels = iRows * iColumns;
	unsigned char* pixels = new unsigned char[nPixels*3];
	if (!pixels)
		return -1;

	unsigned char* red   = *ioFrameBuffer;
	unsigned char* green = (*ioFrameBuffer) + nPixels;
	unsigned char* blue  = (*ioFrameBuffer) + (2*nPixels);

	int i=0;
	for(int j=0; j<nPixels;j++)
	{
		pixels[i++] = red[j];
		pixels[i++] = green[j];
		pixels[i++] = blue[j];
	}

//	delete[] *ioFrameBuffer;
//	*ioFrameBuffer = pixels;

	memcpy(*ioFrameBuffer,pixels, nPixels*3);
	delete[]pixels;

	return 0;
}


//-----------------------------------------------------------------------------
//
int RTVDiCOMStore::SaveBinaryData(AuxDataInfo& iAuxInfo, bool iIsOldCaScore)
{
	int status;

	if (iIsOldCaScore)
		return SaveBinaryData(TR_ATT_CA_SCORE,  iAuxInfo.m_key,  iAuxInfo.m_name, ".doc", true);

	
	if (iAuxInfo.m_type == AuxDataInfo::kCaScore || iAuxInfo.m_type == AuxDataInfo::kCaReport)
	{
		// 07/31/2002 -- 
		// We need to save the file under Name, not typeName
		//status = SaveBinaryData(TR_ATT_FIRST_BINARY_DATA + i,  iAuxInfo.m_key,  iAuxInfo.m_name, ".doc");
		return SaveBinaryData(TR_ATT_FIRST_BINARY_DATA,  iAuxInfo.m_key,  iAuxInfo.m_name, ".doc");
	}
	else if (iAuxInfo.m_type == AuxDataInfo::kTemplate|| 
		iAuxInfo.m_type == AuxDataInfo::kScene ||
		iAuxInfo.m_type == AuxDataInfo::kNetScene || // -- 2004.11.16 new feature for v1.5
		iAuxInfo.m_type == AuxDataInfo::kFindings)
	{
		// 07/31/2002 -- 
		// We need to save the file under Name, not typeName
		status = SaveBinaryData(TR_ATT_FIRST_BINARY_DATA,  iAuxInfo.m_key,  iAuxInfo.m_name, ".xml", false, true);
		if (status != kContinue)
		{
			return status;
		}

		//	Check for existence of thumbnail
		int	valueCount = 0;
		MC_Get_pValue_Count(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_FIRST_THUMBNAIL, &valueCount);
		if (valueCount > 0)
		{
			//	Save the thumbnail to disk
			status = SaveBinaryData(TR_ATT_FIRST_THUMBNAIL,  iAuxInfo.m_key, iAuxInfo.m_name, ".icon");
			if (status != kContinue)
			{
				return kDoNotContinue;
			}	
		}
	}
	else if (iAuxInfo.m_type == AuxDataInfo::kCustom)
	{
		// 09/26/2002 -- added custom data support
		return SaveBinaryData(TR_ATT_FIRST_BINARY_DATA,  iAuxInfo.m_key,  iAuxInfo.m_name, "");
	}
	// more COF types : tcz 2005.11.08
	else if (iAuxInfo.m_type == AuxDataInfo::kMask ||
			 iAuxInfo.m_type == AuxDataInfo::kCenterline ||
			 iAuxInfo.m_type == AuxDataInfo::kCandidates ||
			 iAuxInfo.m_type == AuxDataInfo::kRigidTransform ||
			 iAuxInfo.m_type == AuxDataInfo::kResampledVolume )//SH, 2005.09.25
	{
		status = SaveBinaryData(TR_ATT_FIRST_BINARY_DATA,  iAuxInfo.m_key,  iAuxInfo.m_name, ".xml", false, true);
		if (status != kContinue)
		{
			return status;
		}
	}
	else if (iAuxInfo.m_type == AuxDataInfo::kParametricMapEnhancingRatio ||
			 iAuxInfo.m_type == AuxDataInfo::kParametricMapUptake)
	{
		status = SaveBinaryData(TR_ATT_FIRST_BINARY_DATA,  iAuxInfo.m_key,  iAuxInfo.m_name, ".xml", false, true);
		if (status != kContinue)
		{
			return status;
		}
	}
	// -- 2006.05.08
	// 3rd party integration
	else if (iAuxInfo.m_type == AuxDataInfo::kCustom3rdParty1 )
	{
		status = SaveBinaryData(TR_ATT_FIRST_BINARY_DATA,  iAuxInfo.m_key,  iAuxInfo.m_name, ".dat");
		if (status != kContinue)
		{
			return status;
		}
		//	Check for existence of thumbnail
		int	valueCount = 0;
		MC_Get_pValue_Count(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_FIRST_THUMBNAIL, &valueCount);
		if (valueCount > 0)
		{
			//	Save the thumbnail to disk
			status = SaveBinaryData(TR_ATT_FIRST_THUMBNAIL,  iAuxInfo.m_key, iAuxInfo.m_name, ".icon");
			if (status != kContinue)
			{
				return kDoNotContinue;
			}	
		}
	}

	return kContinue;

}

//-----------------------------------------------------------------------------
//
int RTVDiCOMStore::SaveBinaryData(unsigned long iTag, int iKey, std::string iName, std::string iSuffix, bool isOldCaScore, bool useBinarySize)
{
	int status;
	//	Open the file
	char keyBuf[64];
	_snprintf(keyBuf, sizeof keyBuf, "%d", iKey);
	char fileName[MAX_PATH];

	const char* name = iName.c_str();
	const char* saveDir = m_cacheDir;

	//GL 11-7-2005 Save aux data to original directory, if we can
	//if(iTag == TR_ATT_FIRST_THUMBNAIL)
	//{
		if(access(m_seriesDir, 0) == 0) //valid dir
			saveDir = m_seriesDir;
	//}
	
	// -- 2005.03.28
	// This check is not sufficient - WS may generate fileName with dot in it.
//	if (strrchr(name, '.') == 0)
	if (strrchr(name,'.') == 0 || strstr(name,iSuffix.c_str()) == 0)
		_snprintf(fileName, sizeof fileName, "%s/%s_%s%s", saveDir, keyBuf, iName.c_str(), iSuffix.c_str());
	else
		_snprintf(fileName, sizeof fileName, "%s/%s_%s", saveDir, keyBuf, iName.c_str());

	FILE* auxFile = fopen(fileName, "wb");
	if (auxFile == 0)
	{
		return RTVDiCOMStore::kDoNotContinue;
	}

	//	Get the buffer size so callback can allocate buffer
	unsigned long binarySize = 0;

	if (!isOldCaScore)
	{
		status = MC_Get_pValue_Length(m_messageID, TR_CREATOR, TR_GROUP, iTag, 1, &binarySize);
		if (status != MC_NORMAL_COMPLETION)
		{
			fclose(auxFile);
			return RTVDiCOMStore::kDoNotContinue;
		}
	} 
	else
	{
		status = MC_Get_Value_Length(m_messageID, iTag, 1, &binarySize);
		if (status != MC_NORMAL_COMPLETION)
		{
			fclose(auxFile);
			return RTVDiCOMStore::kDoNotContinue;
		}
	}

	m_OBOWlength = binarySize;

	//	Read the binary data from the message
	if (!isOldCaScore)
	{
		status = MC_Get_pValue_To_Function(m_messageID, TR_CREATOR, TR_GROUP, iTag, this, GetPixelData);
		if (status != MC_NORMAL_COMPLETION)
		{
			fclose(auxFile);
			return RTVDiCOMStore::kDoNotContinue;
		}
	}
	else
	{
		MC_VR vr;
		int num_values;
		status = MC_Get_Attribute_Info(m_messageID, iTag, &vr, &num_values);
		if (vr == UNKNOWN_VR)
		{
			m_OBOWbuffer = new unsigned char[binarySize + 2];
			int returnedLength;
			status = MC_Get_Value_To_Buffer(m_messageID, iTag, binarySize + 2, m_OBOWbuffer, &returnedLength);
			if (status != MC_NORMAL_COMPLETION)
			{
				fclose(auxFile);
				return RTVDiCOMStore::kDoNotContinue;
			}
		}
		else
		{
			status = MC_Get_Value_To_Function(m_messageID, iTag, this, GetPixelData);
			if (status != MC_NORMAL_COMPLETION)
			{
				fclose(auxFile);
				return RTVDiCOMStore::kDoNotContinue;
			}
		}
	}

	unsigned long binaryDataSize = 0;
	status = MC_Get_pValue_To_ULongInt(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_BINARY_DATA_SIZE, &binaryDataSize);
	if (status != MC_NORMAL_COMPLETION || binaryDataSize <= 0 || !useBinarySize)
		;
	else
		binarySize = binaryDataSize;
	/*
	EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_ULongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned long* Abuffer);
*/

	// -- 2005.11.21
	// Because WS pads the data with a null, even for xml, we need to get rid of
	// the null (it chokes the parser). we also need to check for nulls
	//
	if (binarySize > 0 && m_OBOWbuffer[binarySize-1] == '\0' && strstr(iSuffix.c_str(),"xml"))
	{
		binarySize--;
	}

	size_t n = fwrite(m_OBOWbuffer, binarySize, 1, auxFile);

	delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
	fclose(auxFile);
	return RTVDiCOMStore::kContinue;
}
