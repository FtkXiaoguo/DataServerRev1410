/***********************************************************************
 * RTVDiCOMStore.h
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
#ifndef RTVDiCOMStore_H
#define RTVDiCOMStore_H

//-----------------------------------------------------------------------------
#include "RTVDiCOMService.h"
#include "RTVDicomDef.h"

//user of GetPixelData should add following line:
//extern MC_STATUS GetPixelData(int messageID,unsigned long tag,void* userInfo, 
//					   int dataSize,void* dataBufferPtr,int isFirst,int isLast);

class CPxDicomImage;
struct AuxDataInfo;
//-----------------------------------------------------------------------------

class RTVDiCOMStore : public RTVDiCOMService
{
public:
	static long c_totalInProcessing;
	static long c_maxStoreThread;

	static long c_totalCompressedThread;
	static long c_maxCompressedThread;

	enum eAbortStatus
	{
		// This is local to here
		kContinue      = 10000,
		kDoNotContinue = 10001
	};	

	enum ProcessState 
	{ 
		kInitialized, 
		kEnterPreprocess, 
		kHandleTerareconSpecific,
		kdbSaveRecord,
		kLeavePreprocess,
		kEnterProcess,
		kHandleCRImage_AddXAImage,
		kHandleCTImage_AddCTImage,
		kHandleMRImage_AddMRImage,
		kHandleSCImage_AddSCImage,
		kHandleUSImage_AddUSImage,
		kHandleXAImage_AddXAImage_1,
		kHandleXAImage_AddXAImage_2,
		kHandleXAImage_AddXAImage_3,
		kHandleXAImage_AddXAImage_4,
		kHandleNMImage_AddNMImage,
		kHandlePTImage_AddPTImage,
		kdbUpdateFileSize,
		kLeaveProcess,
		kEnterDestructor, 
		kLeaveDestructor,
	};

	enum ResponceSpeed 
	{
		kEarlyResponce,
		kLateResponce
	};
	
	RTVDiCOMStore (DiCOMConnectionInfo& connectInfo, int iMessageID);
	
	virtual ~RTVDiCOMStore();
	
	int HandoverPixelData(int dataSize,void* dataBufferPtr,int isFirst,int isLast);	
	int PreProcess();
	int ProcessHeader(ResponceSpeed rspSpeed=kEarlyResponce);
	void MessageReceiveFinish(int state=0);	// C_STORE_SUCCESS
	bool IsResponsed() {return m_responsed;};
	bool IsCompressed() { return m_isCompressed;};
	//const char* SeriesInstanceUID() {return m_seriesInstanceUID;};
	//virtual int Process() = 0; //in RTVThreadprocess
	int GetResponseStatus() {return m_errorResponseStatus;}
	void SetErrorResponseReason(const char* iReason) { ASTRNCPY(m_errorResponseReason, iReason); }

protected:
	void CleanUp ();
	int theProcessHeader();
	int DeInterlaceColorPlanes(unsigned char** ioFrameBuffer, int iRows, int iColumns);
	
	virtual int CoerceSOPInstanceUID () = 0;
	virtual int HandleTerareconSpecific () = 0;
	int SaveBinaryData(AuxDataInfo& iAuxInfo, bool iIsOldCaScore);
	int SaveBinaryData(unsigned long iTag, int iKey, std::string iName, std::string iSuffix, bool isOldCaScore = false, bool useBinarySize = false);
	
	ProcessState m_state;
	bool m_preprocessed;
	int m_errorResponseStatus;
	char m_errorResponseReason[kVR_LO];

	// This is set in PreProcess and is checked in Process to see
	// if it needs to continue
	bool			  m_startThread;

	CPxDicomImage*	  m_pImage;

	//char m_mediaLabel[_MAX_PATH];
	char m_cacheDir[_MAX_PATH];
	char m_seriesDir[_MAX_PATH];
	char m_seriesUID[65];

	bool			m_hasAuxData;
	unsigned char*	m_OBOWbuffer;
	unsigned long	m_OBOWoffset;
	unsigned long	m_OBOWlength;
	bool			m_inProcessing;
	bool			m_isCompressed;
	bool			m_responsed;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // RTVDiCOMStore_H
