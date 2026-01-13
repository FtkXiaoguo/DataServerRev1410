/*----------------------------------------------------------------------
 *  PxDicomMessage.h
 *---------------------------------------------------------------------
 *  
 *----------------------------------------------------------------------*/

#ifndef FX_DICOM_MESSAGE_H
#define FX_DICOM_MESSAGE_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4616)

//#include "rtvPoolAccess.h"

#include "PxDicomDict.h"
#include "PxDicomStatus.h"
#include "PxDICOMUtil.h"

 
#include "PxDicomServer.h"
 

#include "PxDBData.h"

/////////////////////////////////////////////////////////////////////
 
/////////////////////////////////////////////////////////////////////


const int kMaxPN =	64 + 4;					// Max Person Name string size
const int kMaxUID = 64 + 4;					// Max UID string size

const int kInvalidMessageID  = -1;

const unsigned short NO_DATASET_PRESENT = 0x0101;
const unsigned short DATASET_PRESENT = 0x0102;

enum 
{
	kAE_VR = 00,
	kAS_VR, kCS_VR, kDA_VR, kDS_VR, kDT_VR, kIS_VR, kLO_VR, kLT_VR, kPN_VR, kSH_VR, kST_VR, kTM_VR, kUT_VR,
	kUI_VR, kSS_VR, kUS_VR, kAT_VR, kSL_VR, kUL_VR, kFL_VR, kFD_VR, kUNKNOWN_VR, kOB_VR, kOW_VR, kOL_VR, kOF_VR, kSQ_VR  
};

enum eImageStorageTypes
{
	kCRImage = 0,
	kDXImage,
	kCTImage,
	kMRImage,
	kXAImage,
	kXARFImage,
	kXABPImage,
	kSCImage,
	kUSImage,
	kUSMFImage,
	kNMImage,
	kPTImage,
	kUnknownImage
};

struct pDerivedSeriesInfo;

class  CPxDicomMessage
{
public:
	CPxDicomMessage();											// Default Constructor

	explicit CPxDicomMessage(int iID);							// Conversion constructor

	//	Use this constructor to make an "empty" message of a certain storage class
	explicit CPxDicomMessage(eImageStorageTypes iStorageClass, CPxDicomMessage* iMsg = 0);

	// Remove this attribute from the message
	PxDicomStatus DeleteAttribute(unsigned long iTag);

	// Reset this tag's value to empty
	PxDicomStatus ClearValue(unsigned long iTag);

	// Reset all values to empty - call this before reusing the message
	PxDicomStatus ClearAllValues();

	// Set iTags's value to iValue
	PxDicomStatus SetValue(unsigned long iTag, const char* iValue);
	PxDicomStatus SetValue(unsigned long iTag, int iValue);
	PxDicomStatus SetValue(unsigned long iTag, float iValue);
	PxDicomStatus SetValue(unsigned long iTag, double iValue);
	PxDicomStatus SetValue(unsigned long iTag, unsigned char* iBuf, unsigned long iBufsize, int iVR = kOB_VR);

	// Set Pixel Data - must also set message transfer syntax at the same time
	PxDicomStatus SetPixelData(unsigned char* iBuf, unsigned long iBufSize, int iTransferSyntax);

	// Set Private Tag
	PxDicomStatus SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, 
								   const char* iValue, int iVR = kLO_VR, int iValueNumber = 1);
	PxDicomStatus SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, 
								   unsigned short iValue, int iVR = kUS_VR, int iValueNumber = 1);
	PxDicomStatus SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, 
								   unsigned long iValue, int iVR = kUS_VR, int iValueNumber = 1);
	PxDicomStatus SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, 
								   float iValue, int iVR = kFL_VR, int iValueNumber = 1);
	PxDicomStatus SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, 
								   unsigned char* iBuf, unsigned long iBufsize, int iVR = kOB_VR);

	// Set iTag's value to NULL
	PxDicomStatus SetValueNull(unsigned long iTag);


	PxDicomStatus GetValueCount   (unsigned long iTag, int* oValue); // added TC 11/13/01
	PxDicomStatus GetAttributeInfo(unsigned long iTag, int* oValueCount, int* oMaxLength); // added RL 05/10/02

	// Return iTag's value in oValue
	//   iSize is size of char buffer - you only need to specify for char values larger than kMaxPN
	PxDicomStatus GetValue(unsigned long iTag, char* oValue, int iSize=kMaxPN);
	std::string GetValue(unsigned long iTag);
	PxDicomStatus GetValue(unsigned long iTag, int* oValue);
	PxDicomStatus GetValue(unsigned long iTag, float* oValue);

	//	- Need this to get values out of items - Referenced SOPInstanceUID for example
	PxDicomStatus GetValue(int iItemID, unsigned long iTag, char* oValue, int iSize=kMaxPN);

	// Set iTags's next value to iValue
	PxDicomStatus SetNextValue(unsigned long iTag, const char* iValue);
	PxDicomStatus SetNextValue(unsigned long iTag, int iValue);
	PxDicomStatus SetNextValue(unsigned long iTag, float iValue);
	PxDicomStatus SetNextValue(unsigned long iTag, double iValue);

	// Set iTag's value to NULL
	PxDicomStatus SetValueToNull(unsigned long iTag);
	PxDicomStatus SetNextValueNull(unsigned long iTag);

	// Return iTag's next value in oValue
	//
	PxDicomStatus GetNextValue(unsigned long iTag, char* oValue, int iSize=kMaxPN);
	PxDicomStatus GetNextValue(unsigned long iTag, int* oValue);
	PxDicomStatus GetNextValue(unsigned long iTag, float* oValue);

	// Get the AE Title of the DICOM server from which this message came.
	// A return value of "" indicates that this message didn't come from a DICOM server.
	//
	const DicomDataSource& GetDataSource() const {return m_dataSource;}
	void SetDataSource(DicomDataSource* iSource);

	// Returns true if iTag is set to some value (even if it is set to NULL)
	// Only returns false if iTag is empty (either it was never set, or it was cleared)
	bool IsSet(unsigned long iTag);
		
	// Status member is used to get status info after calls that don't return status
	PxDicomStatus GetStatus() const { return m_status; }

	// For internal use
	void SetID(int iID) { m_messageID = iID; }
	int GetID() const { return m_messageID; }
	int HandoverID() { m_doNotFree = true; return m_messageID; }

	bool CheckValid();

	// For STL
	bool operator <  (const CPxDicomMessage&);
	bool operator == (const CPxDicomMessage&);

	virtual ~CPxDicomMessage();
	
	void    FreeMessage(void); //

/*	//	For handling Load and Save
	typedef RTVMapAccess<int, PxDicomMessage*> MSG_TO_PxDicomMessage_MAP;	
	static MSG_TO_PxDicomMessage_MAP map_msgIDToPxDicomMessage;
	
	enum
	{
		kPixelData = 0,
		kPrivateBinary,
		kPrivateIcon
	} AttributeType;

	int ProcessBinaryData(BufferType iAttributeType, CBType CBtype, unsigned long* dataSizePtr,
					      void** dataBufferPtr,int isFirst,int* isLastPtr);

	//	member access for OBOW buffers
	unsigned char* GetDataBuffer(AttributeType iAttributeType) { return m_buffer[iAttributeType]; }
	void SetDataBuffer(AttributeType iAttributeType, unsigned char* iBuffer, unsigned long iBufferSize) 
	{ 
		m_buffer[iAttributeType] = iBuffer;
		m_length[iAttributeType] = iBufferSize;
		m_offset[iAttributeType] = 0;
	}
*/
	//	For saving to and reading from disk
	/*virtual*/ PxDicomStatus Load(const char *iFilePath, int iHeaderOnly = 0, int iKeepAsFile = 0);

	/*virtual*/ PxDicomStatus LoadWithPreamble(const char *iFilePath, char * oPreamble);

				PxDicomStatus LoadHeader(const char* iFileName, unsigned long& oOffset, unsigned long& oSize);

	/*virtual*/ PxDicomStatus Save(const char* iSavePath, const char* iLocalAE = 0);

	/*virtual*/ PxDicomStatus SaveWithPreamble(const char* iSavePath, 
									const char* iPreamble,
									const char* iLocalAE = 0);

	PxDicomStatus ConvertToSC();

	int GetTransferSyntax() const { return m_transferSyntax; }
	void SetTransferSyntax(int iSyntax) { m_transferSyntax = iSyntax; }

	const char*	GenerateUID(const char* iUID=0);
	PxDicomStatus PopulateSCImage(CPxDicomMessage* iMsg, 
								   int iRows,
								   int iColumns,
								   int iBytesPerPixel,
								   unsigned char* iPixels,
								   const char* iSeriesInstanceUID=0,
								   int iSeriesNumber=200, 
								   int iInstanceNumber=1, 
								   const char *iSoftwareRev=0,
								   int iWindow=0, int iLevel=0, float iSlope = 1.0f, float iIntercept = 0.0f);

	PxDicomStatus PopulateSC12bitImage(const pDerivedSeriesInfo * ipHeader12Bit,
										unsigned char* iPixels,
										const char* iSeriesInstanceUID,
										int iSeriesNumber, 
										int iInstanceNumber, 
										const char * iSoftwareRev);


	virtual void Reset(void);
	
	PxDicomStatus ProcessGetPixelOffset(int CBtype, unsigned long* dataSizePtr, void** dataBufferPtr);
	int GetPixelDataSize(void) const { return m_pixelDataSize; }
	int GetPixelDataOffset(void) const { return m_pixelDataOffset; }

	PxDicomStatus CopyValue(CPxDicomMessage* iMsg, unsigned long iTag);

	// added by s.h to support audit trail
	// later should consolidate with FillSortInfo in VLIImage.
	PxDicomStatus  FillPatientInfo(DICOMData& oData);
	
	//	This is to allow on-site config of the contents of SC images.  To keep it simple,
	//		only a single tag can be added or removed.  
	static unsigned long tagToAddToSC;
	static unsigned long tagToDeleteFromSC;

protected:
 	PxDicomStatus LoadHeader(const char* iFilePath, int iKeepAsFile = 0);

	
	/////////////////////////////////////////////////////////
	// added by shiying hu to have common source code
	// for different loading functions.
	// (load with/without preamble, load with file/memory)
	
	// register application if necessary
	PxDicomStatus StartLoad();

	// load file which does not have group 2
	PxDicomStatus LoadFromMediaStream(const char * iFilePath,
									MediaCBinfo & iCBInfo,
									unsigned long iStartTag,// = AQNET_MC_ATT_GROUP_0002_LENGTH,
									unsigned long iStopTag,// = 0xffFFffFF,
									int iTransferSyntax );//= AQNET_IMPLICIT_LITTLE_ENDIAN);

	PxDicomStatus LoadHeaderFromStream(const char* iFilePath, int iKeepAsFile);
	PxDicomStatus LoadFromMemoryStream(MemoryCBinfo & iCBInfo);

	// get transfer syntax, 
	// convert fileID to messageID
	PxDicomStatus LoadFromFileID(int iFileID, int iKeepAsFileObj = 0);

	PxDicomStatus LoadFileInternal(const char *iFilePath,
									char* oPreamble, int iKeepAsFileObj = 0);

	PxDicomStatus LoadMemoryInternal(const char *iFileInMemory,
										const unsigned long iFileSize);

	PxDicomStatus SaveStart(const char* iSavePath, 
							 const char * iPreamble);

	/////////////////////////////////////////////////////////

	PxDicomStatus WriteFile(const char* iSavePath, const char* iLocalAE = 0);

#if 0	
	//added by s.h to support write to memroy
	PxDicomStatus WriteMemory(AqBuffer & oData, const char* iLocalAE);
#endif

	PxDicomStatus PopulateSCImage();
	PxDicomStatus SetValueIfNoValue(unsigned long iTag, int iValue = 0);
	PxDicomStatus SetValueIfNoValue(unsigned long iTag, const char* iValue);
	PxDicomStatus CopyValue(CPxDicomMessage* iMsg, unsigned long iTag, int iVR, int iType, const char* iDefaultValue = 0);
	PxDicomStatus CopyPrivateTags(CPxDicomMessage* iMsg);
	PxDicomStatus SetReferencedSequence(CPxDicomMessage* iMsg, unsigned long iTag);


	bool HasValue(unsigned long iTag);
	int m_messageID;
	DicomDataSource m_dataSource;	
	PxDicomStatus m_status;

	//	This is static because we only want to register the applicationID one time.
	//	We do this to avoid callback between registered callbacks.
	static int msgApplicationID;

	int m_transferSyntax;
	char m_UID[65];

	bool m_doNotFree;

	unsigned long m_pixelDataSize;
	unsigned long m_pixelDataOffset;

/*	void RegisterCallbacks();
	unsigned char*	m_buffer[3];
	unsigned long	m_offset[3];
	unsigned long	m_length[3];
*/

private:
	//GL hide it
	CPxDicomMessage(const CPxDicomMessage& iMsg);				// Copy Constructor
	CPxDicomMessage& operator= (const CPxDicomMessage& iMsg);	// = operator
public:
	static std::string GetCharacterSets(int iMsgID);
};

//--------------------------------------------------------------------
//
class  MessageFreeGuard
{
public:
	MessageFreeGuard(int& iMsgID, bool fileType=false) : 
	  m_messageID(iMsgID), m_fileType(fileType) { m_guard = true;};
	~MessageFreeGuard() { Free(); };

	void SetFileType(bool fileType) {m_fileType = fileType;};
	void Free();
	void Release() {m_guard = false;};

protected:
	int& m_messageID;
	bool m_guard;
	bool m_fileType;

};

#endif // VLI_DICOM_MESSAGE_H
