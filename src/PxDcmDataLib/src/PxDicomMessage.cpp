//
//	CPxDicomMessage.cpp
//
#pragma warning (disable: 4530)

#include <stdio.h>
#include <assert.h>

#include "AqCore/TRPlatform.h"
 
#include "IDcmLibApi.h"

#include "rtvsutil.h"

#include "PxDicomMessage.h"
#include "rtvsprotocol.h"

//#include "nvli.h"
#include "CheckMemoryLeak.h"

using namespace XTDcmLib;

int CPxDicomMessage::msgApplicationID;
unsigned long CPxDicomMessage::tagToAddToSC;
unsigned long CPxDicomMessage::tagToDeleteFromSC;

#define  sRTVRootUID	   "2.16.840.1.113669.632.21"
#define	 kRGBRGB			0
//	  - 05/10/02 - Added this to provide info about how big tag values can be so 
//		buffers can be allocated to hold them.
static int kVRMaxLength[27] =
{
     16,	// AE
     4,		// AS 
	 16,	// CS
	 10,	// DA
	 16,	// DS
	 26,	// DT
	 12,	// IS
	 64,	// LO
	 10240,	// LT
	 1600,	// PN
	 16,	// SH
	 1024,	// ST
	 16,	// TM
	 10240,	// UT
     64,	// UI
	 2,		// SS
	 2,		// US
	 4,		// AT
	 4,		// SL
	 4,		// UL
	 4,		// FL
	 8,		// FD
	 -1,	// UNKNOWN_VR
	 -1,	// OB
	 -1,	// OW 
	 -1,	// OL
	 -1,	// SQ     
};

#ifdef _TRACE_MEMORY
#include <map>
#include "AqCore/TRCriticalsection.h"

enum  SMessageID_TYPE
{ 
	 kMC_Open_Empty_Message	= 0,
	 kMC_Open_Message,
	 kMC_Read_Message,
	 kMC_Read_Message_To_Tag,
};

struct SMessageID
{
	SMessageID(int id, SMessageID_TYPE type):m_id(id),m_type(type), m_count(0) {};
	~SMessageID() {};
	int m_id;
	SMessageID_TYPE m_type;
	int m_count;
};

class CMessageIDMap
{
public:	
	CMessageIDMap() {};
	~CMessageIDMap(){}; // no virtual destructor, do not derive from it
	//virtual ~RTVMapAccess(){};

	// public member variables to provid possiblly direct map access
	// use it carefully.
	TRCriticalSection m_cs; //access control resource
	typedef std::map<int, SMessageID*> SMessageID_map;
	SMessageID_map m_map;

	bool Add(int id, SMessageID_TYPE type)
	{
		TRCSLock fplock(&m_cs); // lock map, unlock when function return
		SMessageID* pMsg = 0;
		SMessageID_map::iterator iter = m_map.find(id);
		if (iter != m_map.end() ) 
		{
			pMsg = iter->second;
			if(pMsg->m_type != type)
			{
				printf("*********Tried to add message %d, with wrong type %d!!\n", id, type);
				return false;
			}
			else
				pMsg->m_count++;
		}
		else
		{
			pMsg = new SMessageID(id, type);
			pMsg->m_count = 1;
			m_map[id] = pMsg;
		}
		return true;
	};

	int Size()
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		return m_map.size();
	};
	

	void Remove(int id)
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		SMessageID* pMsg = 0;
		SMessageID_map::iterator iter = m_map.find(id);
		if (iter != m_map.end() ) // find key
		{
			pMsg = iter->second;
			pMsg->m_count--;
			if(pMsg->m_count <= 0)
			{
				m_map.erase(iter);
				delete pMsg;
			}
		}
		else
			printf("Tried to free message %d, not in map!!\n", id);;
	};
	
	void Print()
	{
		TRCSLock fplock(&m_cs); // lock pool, unlock when function return
		if( m_map.size() < 1)
		{
			return;
		};

		printf("*********The opened Message IDs are:\n\n"); 
		SMessageID* pMsg = 0;
		SMessageID_map::iterator iter;
		const char* sType = 0;
		for (iter=m_map.begin(); iter != m_map.end(); iter++)
		{
			pMsg = iter->second;
			switch(pMsg->m_type)
			{
				case  kMC_Open_Empty_Message:
					sType = "MC_Open_Empty_Message";
					break;
				case  kMC_Open_Message:
					sType = "MC_Open_Message";
					break;

				case  kMC_Read_Message:
					sType = "MC_Read_Message";
					break;

				case  kMC_Read_Message_To_Tag:
					sType = "MC_Read_Message_To_Tag";
					break;
				default:
					sType = "UNKNOWN";
					break;
			}
			printf("type:%s, id=%d, count=%d\n", sType, pMsg->m_id, pMsg->m_count);
		}


	};
	
};


static CMessageIDMap messageIDMap;
static int fileID_Count = 0;
static int itemID_Count = 0;

//------------------------------------------------------------------------------------------
//
int MergeToolKit::GetOpenMessageCount(void)
{
	return messageIDMap.Size();
}

//------------------------------------------------------------------------------------------
//
int MergeToolKit::GetOpenFileCount(void)
{
	return fileID_Count;
}

//------------------------------------------------------------------------------------------
//
int MergeToolKit::GetOpenItemCount(void)
{
	return itemID_Count;
}

void MergeToolKit::PrintMergeIDCounts()
{
	printf("\nUnclosed Merge resources:\n\n");
	if(fileID_Count)
		printf("***********nUnclosed file ID count = %d\n\n", fileID_Count);
	if(itemID_Count)
		printf("***********nUnclosed item ID count = %d\n\n", itemID_Count);

	messageIDMap.Print();
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Create_File
                                       (int*            AfileID, 
                                        const char*     AfileName, 
                                        const char*     AserviceName, 
                                        MC_COMMAND      Acommand)
{
	MC_STATUS status = MergeToolKit::MC_Create_File(AfileID, AfileName, AserviceName, Acommand);
	if (status == MC_NORMAL_COMPLETION && *AfileID > 0)
	{
		fileID_Count++;
	}
	return status;
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Create_Empty_File  
                                       (int*            AfileID,
                                        const char*     AfileName)
{
	MC_STATUS status = MergeToolKit::MC_Create_Empty_File(AfileID, AfileName);
	if (status == MC_NORMAL_COMPLETION && *AfileID > 0)
	{
		fileID_Count++;
	}
	return status;
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Open_Empty_Message (int*           AmessageID)
{
	MC_STATUS status = MergeToolKit::MC_Open_Empty_Message(AmessageID);
	if (status == MC_NORMAL_COMPLETION && *AmessageID > 0)
	{
		messageIDMap.Add(*AmessageID, kMC_Open_Empty_Message);
//		map_msgIDToCPxDicomMessage.Add(m_messageID, this);
	}
	return status;
}

                                             
//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Open_Item  (int*           AitemID,
                                                 const char*    AitemName)
{
	MC_STATUS status = MergeToolKit::MC_Open_Item(AitemID, AitemName);
	if (status == MC_NORMAL_COMPLETION && *AitemID > 0)
	{
		itemID_Count++;
	}
	return status;
}


//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Open_Message (int*           AmessageID,
                                                 const char*    AserviceName,
                                                 MC_COMMAND     Acommand)
{
	MC_STATUS status = MergeToolKit::MC_Open_Message(AmessageID, AserviceName, Acommand);
	if (status == MC_NORMAL_COMPLETION && *AmessageID > 0)
	{
		messageIDMap.Add(*AmessageID, kMC_Open_Message);	
	}
	return status;
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Read_Message   (int            AssociationID,
                                                     int            Timeout,
                                                     int*           MessageID,
                                                     char**         ServiceName,
													 MC_COMMAND*    Command)
{
	MC_STATUS status = MergeToolKit::MC_Read_Message(AssociationID, Timeout, MessageID, ServiceName, Command);
	if (status == MC_NORMAL_COMPLETION && *MessageID > 0)
	{
		messageIDMap.Add(*MessageID, kMC_Read_Message);	
	}
	return status;
}                                                     
//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Read_Message_To_Tag
                                                    (int            AssociationID,
                                                     int            Timeout,
                                                     unsigned long  AstopTag,
                                                     int*           MessageID,
                                                     char**         ServiceName,
                                                     MC_COMMAND*    Command)
{
	MC_STATUS status = MergeToolKit::MC_Read_Message_To_Tag(AssociationID, Timeout, AstopTag, MessageID, ServiceName, Command);
	if (status == MC_NORMAL_COMPLETION && *MessageID > 0)
	{
		messageIDMap.Add(*MessageID, kMC_Read_Message_To_Tag);	
	}
	return status;
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Free_File
                                       (int*            AfileID)
{
	MC_STATUS status = MergeToolKit::MC_Free_File(AfileID);
	if (status == MC_NORMAL_COMPLETION)
	{
		fileID_Count--;
	}
	return status;
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Free_Item  (int*           AitemID)
{
	MC_STATUS status = MergeToolKit::MC_Free_Item(AitemID);
	if (status == MC_NORMAL_COMPLETION)
	{
		itemID_Count--;
	}
	return status;
}

//------------------------------------------------------------------------------------------
//
EXP_PRE MC_STATUS EXP_FUNC MC_Free_Message (int* AmessageID)
{
	int messageID = *AmessageID;
	MC_STATUS status = MergeToolKit::MC_Free_Message(AmessageID);
	if (status == MC_NORMAL_COMPLETION)
	{
		messageIDMap.Remove(messageID);
	}
	return status;
}

#endif

//------------------------------------------------------------------------------------------
//
CPxDicomMessage::CPxDicomMessage(void):m_doNotFree(false)
{
	m_messageID = kInvalidMessageID;
	Reset();
}

void CPxDicomMessage::Reset(void)
{
	FreeMessage();
//	if (!m_doNotFree && m_messageID != kInvalidMessageID)
//	{
//		MC_Free_Message (&m_messageID);
//	}
//	m_messageID = kInvalidMessageID;

	m_dataSource.type = kUnknownSource;
	m_dataSource.m_server = 0;
	m_status = kNormalCompletion;
	m_transferSyntax = IMPLICIT_LITTLE_ENDIAN;
	m_doNotFree = false;
	m_pixelDataSize = 0;
	m_pixelDataOffset = 0;

}

//------------------------------------------------------------------------------------------
//
CPxDicomMessage::~CPxDicomMessage(void)
{
	FreeMessage();
//	if (!m_doNotFree && m_messageID != kInvalidMessageID)
//	{
//		MC_Free_Message (&m_messageID);
//	}
}

//------------------------------------------------------------------------------------------
// 
// We need this when we want to free a message without doing a complete
// reset, especially if it is called from VLIDICOMImage, too much work.
void CPxDicomMessage::FreeMessage(void)
{
	if (!m_doNotFree && m_messageID != kInvalidMessageID)
	{
		MC_Free_Message (&m_messageID);
	}
	
	m_messageID = kInvalidMessageID;
}
//------------------------------------------------------------------------------------------
//
//	Copy Constructor
//
CPxDicomMessage::CPxDicomMessage(const CPxDicomMessage& iMsg)
{
	m_messageID = iMsg.m_messageID;
	m_status = iMsg.m_status;
	m_dataSource.type = iMsg.m_dataSource.type;
	m_dataSource.m_server = iMsg.m_dataSource.m_server;
	m_doNotFree = iMsg.m_doNotFree;
	m_pixelDataSize = iMsg.m_pixelDataSize;
	m_pixelDataOffset = iMsg.m_pixelDataOffset;
}

//------------------------------------------------------------------------------------------
//
//	Assignment operator
//
CPxDicomMessage& CPxDicomMessage::operator=(const CPxDicomMessage& iMsg)
{
	m_messageID = iMsg.m_messageID;
	m_status = iMsg.m_status;
	m_dataSource.type = iMsg.m_dataSource.type;
	m_dataSource.m_server = iMsg.m_dataSource.m_server;
	m_doNotFree = iMsg.m_doNotFree;
	m_pixelDataSize = iMsg.m_pixelDataSize;
	m_pixelDataOffset = iMsg.m_pixelDataOffset;
	return *this;
}

//------------------------------------------------------------------------------------------
//
CPxDicomMessage::CPxDicomMessage(int iID):m_doNotFree(false)
{
	m_messageID = kInvalidMessageID;
	Reset();
	m_messageID = iID;
}

//------------------------------------------------------------------------------------------
//
CPxDicomMessage::CPxDicomMessage(eImageStorageTypes iStorageClass, CPxDicomMessage* iMsg)
{
	/*
enum eImageStorageTypes
{
	kCRImage = 0,
//	kDXImage,
	kCTImage,
	kMRImage,
	kXAImage,
	kXARFImage,
	kXABPImage,
	kSCImage,
	kUSImage,
	kUSMFImage,
	kUnknownImage
};	*/

	m_messageID = kInvalidMessageID;
	Reset();

	switch(iStorageClass)
	{
	case kSCImage:
		if (!iMsg)
			m_status = PopulateSCImage();
		else
		;//	m_status = PopulateSCImageFromTemplate(iMsg);
		break;
	default:
		break;
	};
}

static char blankPixelData[] = {0x00, 0x00, 0x00, 0x00};

//------------------------------------------------------------------------------------------
//
MC_STATUS SetBlankPixelData(int A_msgID, unsigned long A_tag, int A_isFirst, void* A_info, 
							int* A_dataSize, void** A_dataBufferPtr, int* A_isLastPtr)
{
	assert(A_tag == MC_ATT_PIXEL_DATA);
	assert(A_isFirst != 0);

	if (A_isFirst)
	{
		*A_dataSize = 4;
		*A_dataBufferPtr = blankPixelData;
		*A_isLastPtr = 1;
	}

    return MC_NORMAL_COMPLETION;
}

//------------------------------------------------------------------------------------------
//
static char* DICOMToday(void)
{
	static char buf[4][20];
	static int k;
	char *p = buf[k];

	time_t t = time(0);
	struct tm today = *localtime(&t);
	sprintf(p,"%d%02d%02d", today.tm_year + 1900, today.tm_mon+1, today.tm_mday);
	k = (k+1)&3;
	return p;
}

//------------------------------------------------------------------------------------------
//
static char* DICOMNow(void)
{
	static char buf[4][20];
	static int k;
	char *p = buf[k];
	time_t t = time(0);
	struct tm today = *localtime(&t);
	sprintf(p,"%02d%02d%02d", today.tm_hour,today.tm_min,today.tm_sec);
	k = (k+1)&3;
	return p;
}

//	Type, VM, Size, Tag
struct SCTableEntry
{
	int m_type;
	int m_vr;
	const char* m_defaultValue;
	int m_tag;
};

//	NOTE: Type 0 means we always override the value and
//		  If it's a UID, a Date or Time, and has no default, it will be calculated
const int cSCTableColumns = 4;
static SCTableEntry SCTable[] =
{
	//  copy CHARSET attr for SC [Bug 8409]
	//  General Attributes
	{2, kCS_VR, 0,			kVLISpecificCharacterSet},
	
	//	Patient Module Attributes
	{2, kPN_VR, "AQNetSC",	kVLIPatientsName},
	{2, kLO_VR, "11111111",	kVLIPatientId},
	{2, kDA_VR, 0,			kVLIPatientsBirthDate},
	{2, kCS_VR, 0,			kVLIPatientsSex},
	{3, kTM_VR, 0,			kVLIPatientsBirthTime},
	{3, kLO_VR, 0,			kVLIOtherPatientIds},
	{3, kPN_VR, 0,			kVLIOtherPatientNames},
	{3, kSH_VR, 0,			kVLIEthnicGroup},
	{3, kLT_VR, 0,			kVLIPatientComments},

	//	General Study Module Attributes
	{1, kUI_VR, 0,			kVLIStudyInstanceUid},
	{2, kDA_VR, 0,			kVLIStudyDate},
	{2, kTM_VR, 0,			kVLIStudyTime},
	{2, kPN_VR, 0,			kVLIReferringPhysiciansName},
	{2, kSH_VR, 0,			kVLIStudyId},
	{2, kSH_VR, 0,			kVLIAccessionNumber},
	{3, kLO_VR, 0,			kVLIStudyDescription},
	{3, kPN_VR, 0,			kVLIPhysiciansOfRecord},
	{3, kPN_VR, 0,			kVLINameOfPhysiciansReadingStudy},

	//	General Series Module Attributes
	{1, kCS_VR, "OT",		kVLIModality},
	{0, kUI_VR, 0,			kVLISeriesInstanceUid},
	{3, kCS_VR, 0,			kVLILaterality},
	{0, kDA_VR, 0,			kVLISeriesDate},
	{0, kTM_VR, 0,			kVLISeriesTime},
	{3, kPN_VR, 0,			kVLIPerformingPhysiciansName},
	{3, kLO_VR, 0,			kVLIProtocolName},
	{3, kCS_VR, 0,			kVLIBodyPartExamined},
	
	//	General Image Module Attributes
	{2, kCS_VR, 0,			kVLIPatientOrientation},
//	{2, kDA_VR, 0,			kVLIContentDate},
//	{2, kTM_VR, 0,			kVLIContentTime},

	// modified by shiying hu, 2006-02-08
	

	
	{0, kDA_VR, 0,			kVLIAcquisitionDate},
	{0, kTM_VR, 0,			kVLIAcquisitionTime},

	//	SC Image Equipment Module Attributes
	{0, kCS_VR, "WSD",		kVLIConversionType},
	{0, kLO_VR, "TERARECON",kVLISecondaryCaptureDeviceManufacturer},
	{0, kLO_VR,	"AQNET",	kVLISecondaryCaptureDeviceManufacturersModelName},

	//	SC Image Module Attributes
	{0,	kDA_VR, 0,			kVLIDateOfSecondaryCapture},
	{0,	kTM_VR, 0,			kVLITimeOfSecondaryCapture},

	//	SOP Common Module Attributes
	{0, kUI_VR, "1.2.840.10008.5.1.4.1.1.7", kVLISopClassUid},
	{0, kUI_VR, 0,			kVLISopInstanceUid},
	{0, kDA_VR, 0,			kVLIInstanceCreationDate},
	{0, kTM_VR, 0,			kVLIInstanceCreationTime},

	//	General Equipment Module Attributes
	{3, kSH_VR, 0,			kVLIStationName},
	{2, kLO_VR, 0,			kVLIManufacturer},
	{3, kLO_VR, 0,			kVLIInstitutionName},

	//	THIS MUST BE THE LAST ENTRY IN THE TABLE
	{-1, 0, 0, 0}
};


static SCTableEntry SC12BitTable[] =
{
	// 2007.05.21 kunikichi copy CHARSET attr for SC
	//  General Attributes
	{3, kCS_VR, "ISO.IR 100",kVLISpecificCharacterSet},
	//	General Series Module Attributes
	{0, kUI_VR, 0,			kVLISeriesInstanceUid},
	{0, kDA_VR, 0,			kVLISeriesDate},
	{0, kTM_VR, 0,			kVLISeriesTime},
	
	//	General Image Module Attributes
	/*
	// Modified by shiying hu, 2006-02-08
	// Upon discussion with TC, decides to keep
	// acquisition data and time because they represent time
	// when original image is aqcuired.
	{0, kDA_VR, 0,			kVLIAcquisitionDate},
	{0, kTM_VR, 0,			kVLIAcquisitionTime},
	*/

	//	SC Image Equipment Module Attributes
	{0, kLO_VR, "TERARECON",kVLISecondaryCaptureDeviceManufacturer},
	{0, kLO_VR,	"AQNET",	kVLISecondaryCaptureDeviceManufacturersModelName},

	//	SOP Common Module Attributes


	//{0, kUI_VR, 0,			kVLISopClassUid},	

	{0, kUI_VR, 0,			kVLISopInstanceUid},
	{0, kDA_VR, 0,			kVLIInstanceCreationDate},
	{0, kTM_VR, 0,			kVLIInstanceCreationTime},

	//	THIS MUST BE THE LAST ENTRY IN THE TABLE
	{-1, 0, 0, 0}
};


enum
{
	kATT_Type = 0,
	kATT_VR,
	kATT_Default,
	kATT_Tag
};

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::CopyValue(CPxDicomMessage* iMsg, unsigned long iTag)
{
	MC_STATUS vcStatus = MC_NORMAL_COMPLETION;
	PxDicomStatus status = kNormalCompletion;
	int count;
	int msgID;
	MC_VR vr;
	unsigned long maxValueLength = 0;
	char* value = 0;
	int i = 0;

	if (!iMsg)
		return kInvalidArguments;
	
	msgID = iMsg->GetID();
	vcStatus = MC_Get_Attribute_Info(msgID, iTag, &vr, &count); 
	if (vcStatus != MC_NORMAL_COMPLETION)
		return (PxDicomStatus) vcStatus;

	for(i = 1; i <= count; i++)
	{
		unsigned long valueLength = 0;
		vcStatus = MC_Get_Value_Length(msgID, iTag, i, &valueLength); 
		if (vcStatus != MC_NORMAL_COMPLETION)
			return (PxDicomStatus) vcStatus;

		maxValueLength = (valueLength > maxValueLength) ? valueLength : maxValueLength;
	}
	maxValueLength += 2;

	//
	//ref Viewer #2111
	// for safety
	if (count > 1){ 
		maxValueLength *= count;
	}
	if (maxValueLength < (kMaxPN * 3)){ 
		maxValueLength = (kMaxPN * 3);
	}

	//	Create and initialize buffer for source value
	iRTVSAlloc<char> tmpBuf(maxValueLength);
	value = tmpBuf;

	if (!value)
		return kCouldNotAllocateMemory;

	memset(value, 0, maxValueLength);

	//	Get first value
	status = iMsg->GetValue(iTag, value, maxValueLength);

	// 
	if(status == kNormalCompletion)
		status = SetValue(iTag, value);
	else if(status == kNullValue)
		status = SetValueNull(iTag);
	else
		return status;		
	
	if (status != kNormalCompletion)
		return status;

	//	Get any subsequent values
	for(i = 2; i <= count; i++)
	{
		memset(value, 0, maxValueLength);

		status = iMsg->GetNextValue(iTag, value, maxValueLength);

		if (status == kNormalCompletion) 
			status = SetNextValue(iTag, value);
		else if(status == kNullValue)
			status  = SetNextValueNull(iTag);
		else			
			return status;

		if (status != kNormalCompletion)
			return status;
	}

	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::CopyValue(CPxDicomMessage* iMsg, unsigned long iTag, int iVR, int iType, const char* iDefaultValue)
{
	PxDicomStatus status = kNormalCompletion;
	
	if (iMsg)
		status = CopyValue(iMsg, iTag);
	
	//	If we couldn't get it, what should we do?  Depends on if Type1, Type2 or Type3
	if (!iMsg || status != kNormalCompletion)
	{
		if (iDefaultValue)
		{
			SetValue(iTag, iDefaultValue);
			return kNormalCompletion;
		}

		//	No value in the source and no default
		switch(iType)
		{
		case 1:
			if (iVR == UI)
			{
				SetValue(iTag, GenerateUID());
				return kNormalCompletion;
			}
			else
				return status;
		case 2:
			SetValueNull(iTag);
			return kNormalCompletion;
		case 3:
			return kNormalCompletion;
		};
	}

	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetReferencedSequence(CPxDicomMessage* iMsg, unsigned long iTag)
{
	if (!iMsg)
		return kNormalCompletion;

	int tID = iMsg->GetID();

	char referencedSOPClassUID[kVR_UI];
	char referencedSOPInstanceUID[kVR_UI];
	PxDicomStatus status = GetValue(tID, kVLISopClassUid,	referencedSOPClassUID, kVR_UI);
	PxDicomStatus status1= GetValue(tID, kVLISopInstanceUid, referencedSOPInstanceUID, kVR_UI);
	if (status == status1 == kNormalCompletion)
	{
		int sqID = -1;
		int mcStatus, mcStatus2;
		
		mcStatus = MC_Open_Item(&sqID, "Referenced Sequence");
		if (mcStatus == MC_NORMAL_COMPLETION && sqID)
		{
			mcStatus = MC_Set_Value_From_String(sqID, kVLIReferencedSopClassUid, referencedSOPClassUID);
			mcStatus2= MC_Set_Value_From_String(sqID, kVLIReferencedSopInstanceUid, referencedSOPInstanceUID);
			if (mcStatus == mcStatus2 == MC_NORMAL_COMPLETION)
			{
				status = SetValue(iTag, sqID);
			}
		}
	}

	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::CopyPrivateTags(CPxDicomMessage* iMsg) 
{
	MC_STATUS mcStatus;
	PxDicomStatus status;
	int mID = iMsg->GetID();
	unsigned long tag;
	MC_VR vr;
	int numValues;

	mcStatus = MC_Get_First_Attribute(mID, &tag, &vr, &numValues);
	while( mcStatus == MC_NORMAL_COMPLETION)
	{
		unsigned long a,b,c;
		a = tag & 0xffff0000;
		b = a >> 0x10;
		c = b % 2;

		//	Is it private? (i.e. odd group)
		if (tag && c)
		{
			status = CopyValue(iMsg, tag);
			if (status != kNormalCompletion)
				return status;
		}

		mcStatus = MC_Get_Next_Attribute(mID, &tag, &vr, &numValues);
	}

	return kNormalCompletion;
}


//-------- ----------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::PopulateSCImage(CPxDicomMessage* iMsg, 
															int iRows,
															int iColumns,
															int iBytesPerPixel,
															unsigned char* iPixels,
															const char* iSeriesInstanceUID,
															int iSeriesNumber, 
															int iInstanceNumber, 
															const char * iSoftwareRev,
															int iWindow, int iLevel, float iSlope, float iIntercept)
{
	PxDicomStatus status;
	int i=0;
	int tag, vr, type;
	const char* defaultValue;
	char today[32], now[32];

	strcpy(today, DICOMToday());
	strcpy(now, DICOMNow());

	status = (PxDicomStatus) MC_Open_Empty_Message(&m_messageID);
	if (status != kNormalCompletion)
	{
		m_messageID = kInvalidMessageID;
		return status;
	}

	for(i=0; SCTable[i].m_type >= 0; i++)
	{
		tag = SCTable[i].m_tag;
		vr = SCTable[i].m_vr;
		type = SCTable[i].m_type;
		defaultValue = SCTable[i].m_defaultValue;

		//	Override the source value
		if (type == 0)
		{				
			if (defaultValue)
				SetValue(tag, defaultValue);
			else if (vr == kUI_VR)
				SetValue(tag, GenerateUID());
			else if (vr == kDA_VR)
				SetValue(tag, today);
			else if (vr == kTM_VR)
				SetValue(tag, now);
			else
				return kSystemError;
		}
		else
		{
			status = CopyValue(iMsg, tag, vr, type, defaultValue);
			if (status != kNormalCompletion)
				return status;
		}
	}

/*
 *	Should we put this in or not??
 *
	SetReferencedSequence(iMsg, kVLIReferencedPatientSequence);
	SetReferencedSequence(iMsg, kVLIReferencedStudySequence);
	SetReferencedSequence(iMsg, kVLISourceImageSequence);
*/

	SetValue(kVLISeriesNumber, iSeriesNumber);
	SetValue(kVLIInstanceNumber, iInstanceNumber);
	

	//		don't want to replace valid SeriesInstanceUID with empty string
	if (iSeriesInstanceUID && strlen(iSeriesInstanceUID) > 5)
		SetValue(kVLISeriesInstanceUid, iSeriesInstanceUID);

	SetValue(kVLIImageType, "DERIVED");
	SetNextValue(kVLIImageType, "SECONDARY");
	SetNextValue(kVLIImageType, "AQNETSC");

	if (iSoftwareRev)
		SetValue(kVLISecondaryCaptureDeviceSoftwareVersions, iSoftwareRev);

	//	Patient Position is 2C - only do if CT or MR
	char modality[kVR_CS];
	GetValue(kVLIModality, modality, kVR_CS);
	if (!strcmp(modality, "CT") || !strcmp(modality, "MR"))
		CopyValue(iMsg, kVLIPatientPosition, kCS_VR, 2);

	//
	//	Populate Image Pixel Module Attributes
	//

	int size = iRows * iColumns * iBytesPerPixel;

	if (size <= 0 || iPixels == 0)
	{
		return kInvalidImagePixels;
	}
	
		
	SetValue(kVLIRows,		iRows);
	SetValue(kVLIColumns,	iColumns);
	if (iBytesPerPixel == 3)
	{
		SetValue(kVLISamplesPerPixel, 3);
		SetValue(kVLIPhotometricInterpretation,"RGB");
		SetValue(kVLIPlanarConfiguration,		kRGBRGB);
		SetValue(kVLIBitsAllocated,				8);
		SetValue(kVLIBitsStored,				8);
		SetValue(kVLIHighBit,					7);
		SetValue(kVLIPixelRepresentation,		0);
	}
	else if (iBytesPerPixel == 1)
	{
		SetValue(kVLISamplesPerPixel, 1);
		SetValue(kVLIPhotometricInterpretation,"MONOCHROME2");
		DeleteAttribute(kVLIPlanarConfiguration);
		SetValue(kVLIBitsAllocated,				8);
		SetValue(kVLIBitsStored,				8);
		SetValue(kVLIHighBit,					7);
		SetValue(kVLIPixelRepresentation,		0);

		//	 - Recommended window / level
		SetValue(kVLIWindowCenter,				127);
		SetValue(kVLIWindowWidth,				256);
	}
	else if (iBytesPerPixel == 2)
	{
		SetValue(kVLISamplesPerPixel, 1);
		SetValue(kVLIPhotometricInterpretation,"MONOCHROME2");
		DeleteAttribute(kVLIPlanarConfiguration);
		SetValue(kVLIBitsAllocated,				16);
		SetValue(kVLIBitsStored,				12);
		SetValue(kVLIHighBit,					11);
		SetValue(kVLIPixelRepresentation,		0);	
		// 
		// We do need window level for SC to show up properly on PACS
		SetValue(kVLIWindowCenter,				iLevel);
		SetValue(kVLIWindowWidth,				iWindow);
		//  Slope Intercept support
		if ( iSlope != 0.0f && !(iSlope == 1.0f && iIntercept == 0.0f) )
		{
			SetValue(kVLIRescaleSlope,			iSlope);
			SetValue(kVLIRescaleIntercept,		iIntercept);
		}
	}


	if (size <= 0 || iPixels == 0)
	{
		return kInvalidImagePixels;
	}

	SetPixelData(iPixels, size, IMPLICIT_LITTLE_ENDIAN);
/*
	SetValue(kVLIPixelData, iPixels, size);	
	
 	m_transferSyntax = IMPLICIT_LITTLE_ENDIAN;
	status = (PxDicomStatus) MC_Set_Message_Transfer_Syntax(m_messageID, (TRANSFER_SYNTAX) m_transferSyntax);
	if (status != kNormalCompletion)
		return status;
*/
//	CopyPrivateTags(iMsg);

	//	This is to allow on-site config of the contents of SC images.  To keep it simple,
	//		only a single tag can be added or removed.  
	if (tagToAddToSC)
		CopyValue(iMsg, tagToAddToSC, 0, 2);

	if (tagToDeleteFromSC)
		DeleteAttribute(tagToDeleteFromSC);

	return m_status;//kNormalCompletion;
}



//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::PopulateSC12bitImage(const pDerivedSeriesInfo * ipHeader12Bit,
													unsigned char* iPixels,
													const char* iSeriesInstanceUID,
													int iSeriesNumber, 
													int iInstanceNumber, 
													const char * iSoftwareRev)
{
	if ( !ipHeader12Bit )
		return kInvalidArguments;


	PxDicomStatus status;
	int i=0;
	int tag, vr, type;
	const char* defaultValue;
	char today[32], now[32];

	strcpy(today, DICOMToday());
	strcpy(now, DICOMNow());

	/*
	status = (PxDicomStatus) MC_Open_Empty_Message(&m_messageID);
	if (status != kNormalCompletion)
	{
		m_messageID = kInvalidMessageID;
		return status;
	}
	*/

	for(i=0; SC12BitTable[i].m_type >= 0; i++)
	{
		tag = SC12BitTable[i].m_tag;
		vr = SC12BitTable[i].m_vr;
		type = SC12BitTable[i].m_type;
		defaultValue = SC12BitTable[i].m_defaultValue;

		//	Override the source value
		if (defaultValue)
			status = SetValue(tag, defaultValue);
		else if (vr == kUI_VR)
			status = SetValue(tag, GenerateUID());
		else if (vr == kDA_VR)
			status = SetValue(tag, today);
		else if (vr == kTM_VR)
			status = SetValue(tag, now);
		else
			return kSystemError;
	}

/*
 *	Should we put this in or not??
 *
	SetReferencedSequence(iMsg, kVLIReferencedPatientSequence);
	SetReferencedSequence(iMsg, kVLIReferencedStudySequence);
	SetReferencedSequence(iMsg, kVLISourceImageSequence);
*/

	status = SetValue(kVLISeriesNumber, iSeriesNumber);
	status = SetValue(kVLIInstanceNumber, iInstanceNumber);
	
	if (iSeriesInstanceUID)
		status = SetValue(kVLISeriesInstanceUid, iSeriesInstanceUID);

	status = SetValue(kVLIImageType, "DERIVED");
	status = SetNextValue(kVLIImageType, "SECONDARY");
	status = SetNextValue(kVLIImageType, "AQNET12BIT");

	if (iSoftwareRev)
		status = SetValue(kVLISecondaryCaptureDeviceSoftwareVersions, iSoftwareRev);

	//	Patient Position is 2C - only do if CT or MR

	//
	//	Populate Image Pixel Module Attributes
	//

	int size = ipHeader12Bit->m_height * ipHeader12Bit->m_width * ipHeader12Bit->m_bytesPerPixel;

	if (size <= 0 || iPixels == 0)
	{
		return kInvalidImagePixels;
	}

	status = SetValue(kVLIRows,		ipHeader12Bit->m_height);
	status = SetValue(kVLIColumns,	ipHeader12Bit->m_width);
	status = SetValue(kVLISamplesPerPixel, 1);
	status = SetValue(kVLIPhotometricInterpretation,"MONOCHROME2");
	DeleteAttribute(kVLIPlanarConfiguration);
	status = SetValue(kVLIBitsAllocated,				16);
	status = SetValue(kVLIBitsStored,				12);
	status = SetValue(kVLIHighBit,					11);
	status = SetValue(kVLIPixelRepresentation,		0);	
	
	// 12 bit specific information
	status = SetValue(kVLIWindowCenter,		ipHeader12Bit->m_level);
	status = SetValue(kVLIWindowWidth,		ipHeader12Bit->m_window);
	// 2007.05.25 kunikichi Slope Intercept support
	if ( ipHeader12Bit->m_rescaleSlope != 0.0f && 
		 !(ipHeader12Bit->m_rescaleSlope == 1.0f && ipHeader12Bit->m_rescaleIntercept == 0.0f) )
	{
		status = SetValue(kVLIRescaleSlope,			ipHeader12Bit->m_rescaleSlope);
		status = SetValue(kVLIRescaleIntercept,		ipHeader12Bit->m_rescaleIntercept);
	}
	status = SetValue(kVLISliceThickness,	ipHeader12Bit->m_thickness);
	status = SetValue(kVLISliceLocation,	ipHeader12Bit->m_location);
	status = SetValue(kVLISeriesDescription, ipHeader12Bit->m_seriesDescription);

	// values for following tags are fake value.
	// should be corrected later. 
	// shiying hu, 11.08.2004

	// set image position patient
	status = SetValue(kVLIImagePositionPatient,		ipHeader12Bit->m_imagePosition[0]);
	status = SetNextValue(kVLIImagePositionPatient, ipHeader12Bit->m_imagePosition[1]);
	status = SetNextValue(kVLIImagePositionPatient, ipHeader12Bit->m_imagePosition[2]);
	
	// set image orientation patient
	status = SetValue(kVLIImageOrientationPatient,	   ipHeader12Bit->m_imageOrientation[0]);
	status = SetNextValue(kVLIImageOrientationPatient, ipHeader12Bit->m_imageOrientation[1]);
	status = SetNextValue(kVLIImageOrientationPatient, ipHeader12Bit->m_imageOrientation[2]);
	status = SetNextValue(kVLIImageOrientationPatient, ipHeader12Bit->m_imageOrientation[3]);
	status = SetNextValue(kVLIImageOrientationPatient, ipHeader12Bit->m_imageOrientation[4]);
	status = SetNextValue(kVLIImageOrientationPatient, ipHeader12Bit->m_imageOrientation[5]);
	
	// set pixel spacing
	status = SetValue(kVLIPixelSpacing,		ipHeader12Bit->m_pixelSpacing[0]);
	status = SetNextValue(kVLIPixelSpacing, ipHeader12Bit->m_pixelSpacing[1]);

	// set reconstruction diameter
//	status = SetValue(kVLIReconstructionDiameter, ipHeader12Bit->m_pixelSpacing[0]*(ipHeader12Bit->m_width-1));
	// 2009.08.18 Y.Kimura Fixed FOV
	status = SetValue(kVLIReconstructionDiameter, ipHeader12Bit->m_pixelSpacing[0]*(ipHeader12Bit->m_width));

//	CopyPrivateTags(iMsg);
//	status = SetValue(kVLIPixelData, iPixels, size);						  	

	SetPixelData(iPixels, size, IMPLICIT_LITTLE_ENDIAN);

	return m_status;//kNormalCompletion;
}


//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::PopulateSCImage()
{
	PxDicomStatus status;

	status = (PxDicomStatus) MC_Open_Empty_Message(&m_messageID);
	if (status != kNormalCompletion)
	{
		m_messageID = kInvalidMessageID;
		return status;
	}

	//	Patient Module
	status = SetValueNull(kVLIPatientsName);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIPatientId);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIPatientsBirthDate);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIPatientsSex);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIPatientsName);
	if (status != kNormalCompletion)
		return status;

	//	Study Module
	status = SetValue(kVLIStudyInstanceUid, GenerateUID());
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIStudyDate);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIStudyTime);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIReferringPhysiciansName);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIStudyId);
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLIAccessionNumber);
	if (status != kNormalCompletion)
		return status;

	//	General Series Module
	status = SetValue(kVLIModality, "OT");
	if (status != kNormalCompletion)
		return status;

	status = SetValue(kVLISeriesInstanceUid, GenerateUID());
	if (status != kNormalCompletion)
		return status;

	status = SetValueNull(kVLISeriesNumber);
	if (status != kNormalCompletion)
		return status;

	//	SC Image Equipment Module
	status = SetValue(kVLIConversionType, "WSD");
	if (status != kNormalCompletion)
		return status;
	 
	//	General Image Module
	status = SetValueNull(kVLIInstanceNumber);
	if (status != kNormalCompletion)
		return status;

	//	Image Pixel Module
	status = SetValue(kVLISamplesPerPixel, 1);
	if (status != kNormalCompletion)
		return status;

	status = SetValue(kVLIPhotometricInterpretation, "MONOCHROME2");
	if (status != kNormalCompletion)
		return status;

	status = SetValue(kVLIRows, 2);
	if (status != kNormalCompletion)
		return status;

 	status = SetValue(kVLIColumns, 2);
	if (status != kNormalCompletion)
		return status;

 	status = SetValue(kVLIBitsAllocated , 16);
	if (status != kNormalCompletion)
		return status;

	status = SetValue(kVLIBitsStored , 12);
	if (status != kNormalCompletion)
		return status;

 	status = SetValue(kVLIHighBit , 11);
	if (status != kNormalCompletion)
		return status;

 	status = SetValue(kVLIPixelRepresentation , 0);
	if (status != kNormalCompletion)
		return status;

	status = (PxDicomStatus) MC_Set_Value_From_Function(m_messageID, MC_ATT_PIXEL_DATA, 0, (SetBlankPixelData));
	if (status != kNormalCompletion)
		return status;

 	status = SetValueNull(kVLIPatientOrientation);
 	if (status != kNormalCompletion)
 		return status;

	//	SOP Common Module
	status = SetValue(kVLISopClassUid, "1.2.840.10008.5.1.4.1.1.7");
	if (status != kNormalCompletion)
		return status;

	status = SetValue(kVLISopInstanceUid, GenerateUID());
	if (status != kNormalCompletion)
		return status;
	
	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValueIfNoValue(unsigned long iTag, int iValue)
{
	MC_STATUS status;
	int valueCount = -1;

	status = MC_Get_Value_Count(m_messageID, iTag, &valueCount);
	if (status == MC_EMPTY_VALUE || valueCount == 0)
	{
		if (!iValue)
		{
			MC_Set_Value_To_NULL(m_messageID, iTag);
		}
		else
		{
			MC_Set_Value_From_Int(m_messageID, iTag, iValue);
		}

		return kNormalCompletion;
	}

	return (PxDicomStatus) status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValueIfNoValue(unsigned long iTag, const char* iValue)
{
	MC_STATUS status;
	int valueCount = -1;

	status = MC_Get_Value_Count(m_messageID, iTag, &valueCount);
	if (status == MC_EMPTY_VALUE || valueCount == 0)
	{
		if (!iValue)
		{
			MC_Set_Value_To_NULL(m_messageID, iTag);
		}
		else
		{
			MC_Set_Value_From_String(m_messageID, iTag, iValue);
		}

		return kNormalCompletion;
	}

	return (PxDicomStatus) status;
}

//------------------------------------------------------------------------------------------
//
bool CPxDicomMessage::HasValue(unsigned long iTag)
{
	MC_STATUS status;
	int valueCount = -1;

	status = MC_Get_Value_Count(m_messageID, iTag, &valueCount);
	if (status == MC_EMPTY_VALUE || valueCount == 0 || status != MC_NORMAL_COMPLETION)
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::ConvertToSC()
{
	PxDicomStatus status;

	if (!this->CheckValid())
	{
		return m_status;
	}

	//	Since this is becoming secondary capture, we know we are not compressing the pixels
 	m_transferSyntax = IMPLICIT_LITTLE_ENDIAN;
	status = (PxDicomStatus) MC_Set_Message_Transfer_Syntax(m_messageID, (TRANSFER_SYNTAX) m_transferSyntax);
	if (status != kNormalCompletion)
		return status;
	
	//	Patient Module
	status = SetValueIfNoValue(kVLIPatientsName);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIPatientId);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIPatientsBirthDate);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIPatientsSex);
	if (status != kNormalCompletion)
		return status;

	//	Study Module
	if (!HasValue(kVLIStudyInstanceUid))
	{
		return kRequiredAttributeMissing;
	}

	status = SetValueIfNoValue(kVLIStudyDate);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIStudyTime);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIReferringPhysiciansName);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIStudyId);
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLIAccessionNumber);
	if (status != kNormalCompletion)
		return status;

	//	General Series Module
	status = SetValueIfNoValue(kVLIModality, "OT");
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLISeriesInstanceUid, GenerateUID());
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLISeriesNumber);
	if (status != kNormalCompletion)
		return status;

	//	SC Image Equipment Module
	status = SetValueIfNoValue(kVLIConversionType, "WSD");
	if (status != kNormalCompletion)
		return status;
	 
	//	General Image Module
	status = SetValueIfNoValue(kVLIInstanceNumber);
	if (status != kNormalCompletion)
		return status;

	//	Image Pixel Module
	if (!HasValue(kVLISamplesPerPixel))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIPhotometricInterpretation))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIRows))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIColumns))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIBitsAllocated))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIBitsStored))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIHighBit))
	{
		return kRequiredAttributeMissing;
	}

	if (!HasValue(kVLIPixelRepresentation))
	{
		return kRequiredAttributeMissing;
	}

 	status = SetValueIfNoValue(kVLIPatientOrientation);
 	if (status != kNormalCompletion)
 		return status;

	//	SOP Common Module
	status = SetValue(kVLISopClassUid, "1.2.840.10008.5.1.4.1.1.7");
	if (status != kNormalCompletion)
		return status;

	status = SetValue(kVLIImageType, "DERIVED");
	if (status != kNormalCompletion)
		return status;

	status = SetNextValue(kVLIImageType, "SECONDARY");
	if (status != kNormalCompletion)
		return status;

	status = SetValueIfNoValue(kVLISopInstanceUid, GenerateUID());
	if (status != kNormalCompletion)
		return status;
	
	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
// This is for STL 
//
bool CPxDicomMessage::operator < (const CPxDicomMessage& inMsg) 
{
    return (m_messageID > inMsg.m_messageID);
}

//------------------------------------------------------------------------------------------
//
// This is for STL 
//
bool CPxDicomMessage::operator == (const CPxDicomMessage& inMsg) 
{
    return (m_messageID == inMsg.m_messageID);
}

//------------------------------------------------------------------------------------------
//
bool CPxDicomMessage::CheckValid()
{
	if (m_messageID != kInvalidMessageID)
	{
		return true;
	}
	
	m_status = (PxDicomStatus) MC_Open_Empty_Message(&m_messageID);
	if (m_status != kNormalCompletion)
	{
		m_messageID = kInvalidMessageID;
		return false;
	}

#if 0
	//	Added by  - We think this should be here.because Open_Empty_Message doesn't set it?
	m_status = (PxDicomStatus) MC_Set_Value_From_Int(m_messageID, MC_ATT_MESSAGE_ID, m_messageID);
	if (m_status != kNormalCompletion)
	{
		return false;
	}
#endif

	return true;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValueNull(unsigned long iTag)
{
	if (this->CheckValid())
	{
		return((PxDicomStatus) MC_Set_Value_To_NULL(m_messageID,iTag));
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetNextValueNull(unsigned long iTag)
{
	if (this->CheckValid())
	{
		return((PxDicomStatus) MC_Set_Next_Value_To_NULL(m_messageID,iTag));
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValue(unsigned long iTag, const char* iValue)
{ 
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Value_From_String(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValue(unsigned long iTag, int iValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Value_From_Int(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValue(unsigned long iTag, float iValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Value_From_Float(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
// just for completeness
PxDicomStatus CPxDicomMessage::SetValue(unsigned long iTag, double iValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Value_From_Double(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, const char* iValue, int iVR, int iValueNumber)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	m_status = (PxDicomStatus) MC_Add_Private_Block(m_messageID, iCreatorCode, iGroup);
	m_status = (PxDicomStatus) MC_Add_Private_Attribute(m_messageID, iCreatorCode, iGroup, iElementByte, (MC_VR) iVR);

	if (iValueNumber < 1)
		return m_status = kInvalidValueNumber;
	else if (iValueNumber == 1)
		return m_status = (PxDicomStatus) MC_Set_pValue_From_String(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
	else
		return m_status = (PxDicomStatus) MC_Set_Next_pValue_From_String(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, unsigned short iValue, int iVR, int iValueNumber)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	m_status = (PxDicomStatus) MC_Add_Private_Block(m_messageID, iCreatorCode, iGroup);
	m_status = (PxDicomStatus) MC_Add_Private_Attribute(m_messageID, iCreatorCode, iGroup, iElementByte, (MC_VR) iVR);

	if (iValueNumber < 1)
		return m_status = kInvalidValueNumber;
	else if (iValueNumber == 1)
		return m_status = (PxDicomStatus) MC_Set_pValue_From_UShortInt(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
	else
		return m_status = (PxDicomStatus) MC_Set_Next_pValue_From_UShortInt(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, unsigned long iValue, int iVR, int iValueNumber)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	m_status = (PxDicomStatus) MC_Add_Private_Block(m_messageID, iCreatorCode, iGroup);
	m_status = (PxDicomStatus) MC_Add_Private_Attribute(m_messageID, iCreatorCode, iGroup, iElementByte, (MC_VR) iVR);

	if (iValueNumber < 1)
		return m_status = kInvalidValueNumber;
	else if (iValueNumber == 1)
		return m_status = (PxDicomStatus) MC_Set_pValue_From_ULongInt(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
	else
		return m_status = (PxDicomStatus) MC_Set_Next_pValue_From_ULongInt(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, float iValue, int iVR, int iValueNumber)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	m_status = (PxDicomStatus) MC_Add_Private_Block(m_messageID, iCreatorCode, iGroup);
	m_status = (PxDicomStatus) MC_Add_Private_Attribute(m_messageID, iCreatorCode, iGroup, iElementByte, (MC_VR) iVR);

	if (iValueNumber < 1)
		return m_status = kInvalidValueNumber;
	else if (iValueNumber == 1)
		return m_status = (PxDicomStatus) MC_Set_pValue_From_Float(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
	else
		return m_status = (PxDicomStatus) MC_Set_Next_pValue_From_Float(m_messageID, iCreatorCode, iGroup, iElementByte, iValue);
}

/*
struct CBDataStruct
{
	unsigned char* m_buf;
	unsigned long m_bufsize;
};
*/
//-----------------------------------------------------------------------------------------
//
MC_STATUS SetDataBuffer(int messageID, unsigned long tag, int isFirst, void* userInfo, 
							   int* dataSizePtr, void** dataBufferPtr, int* isLastPtr)
{
	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	CBDataStruct* cbdata = (CBDataStruct*) userInfo;
	
	if (!cbdata->m_buf)
		return MC_CALLBACK_CANNOT_COMPLY;

	*dataBufferPtr = cbdata->m_buf;
	*dataSizePtr = cbdata->m_bufsize;
	*isLastPtr = 1;
	return MC_NORMAL_COMPLETION;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetPrivateValue(const char* iCreatorCode, unsigned short iGroup, unsigned char iElementByte, 
											    unsigned char* iBuf, unsigned long iBufsize, int iVR)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	
	unsigned long evenBufferSize = (iBufsize % 2) ? iBufsize + 1 : iBufsize;
	unsigned char* evenBuffer = 0;
	bool needToDelete = false;

	if (evenBufferSize == iBufsize)
	{
		evenBuffer = iBuf;
	}
	else
	{
		evenBuffer = new unsigned char[evenBufferSize];
		if (!evenBuffer)
			return kCouldNotAllocateMemory;

		needToDelete = true;
		memcpy(evenBuffer, iBuf, iBufsize);
		evenBuffer[evenBufferSize - 1] = 0;					//	Padding byte
	}

	CBDataStruct cbdata;
	cbdata.m_buf = evenBuffer;
	cbdata.m_bufsize = evenBufferSize;

	m_status = (PxDicomStatus) MC_Add_Private_Block(m_messageID, iCreatorCode, iGroup);
	m_status = (PxDicomStatus) MC_Add_Private_Attribute(m_messageID, iCreatorCode, iGroup, iElementByte, (MC_VR) iVR);
	m_status = (PxDicomStatus) MC_Set_pValue_From_Function(m_messageID, iCreatorCode, iGroup, iElementByte, &cbdata, SetDataBuffer);

	if (needToDelete)
	{
		delete [] evenBuffer, evenBuffer = 0;
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValue(unsigned long iTag, unsigned char* iBuf, unsigned long iBufsize, int iVR)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	
	unsigned long evenBufferSize = (iBufsize % 2) ? iBufsize + 1 : iBufsize;
	unsigned char* evenBuffer = 0;
	bool needToDelete = false;

	if (evenBufferSize == iBufsize)
	{
		evenBuffer = iBuf;
	}
	else
	{
		evenBuffer = new unsigned char[evenBufferSize];
		if (!evenBuffer)
			return kCouldNotAllocateMemory;

		needToDelete = true;
		memcpy(evenBuffer, iBuf, iBufsize);
		evenBuffer[evenBufferSize - 1] = 0;					//	Padding byte
	}

	CBDataStruct cbdata;
	cbdata.m_buf = evenBuffer;
	cbdata.m_bufsize = evenBufferSize;

	m_status = (PxDicomStatus) MC_Set_Value_From_Function(m_messageID, iTag, &cbdata, SetDataBuffer);

	if (needToDelete)
	{
		delete [] evenBuffer, evenBuffer = 0;
	}

	return m_status;
}


//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetPixelData(unsigned char* iBuf, unsigned long iBufSize, int iTransferSyntax)
{
	if (!(this->CheckValid()))
	{
		return m_status;
	}

	PxDicomStatus status = SetValue(kVLIPixelData, iBuf, iBufSize);	
	if (status != kNormalCompletion)
		return status;
	
 	m_transferSyntax = (TRANSFER_SYNTAX) iTransferSyntax;
	return (PxDicomStatus) MC_Set_Message_Transfer_Syntax(m_messageID, (TRANSFER_SYNTAX) m_transferSyntax);
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetValueCount(unsigned long iTag, int* oValue)
{
	PxDicomStatus status;

	if(!oValue)
		return (PxDicomStatus)MC_SYSTEM_ERROR;

	*oValue = 0;

	if (this->CheckValid())
	{
		status = (PxDicomStatus)MC_Get_Value_Count(m_messageID, iTag, oValue);

		//
	//
		if (iTag == kVLIPatientsName || 
			iTag == kVLIReferringPhysiciansName || 
			iTag == kVLINameOfPhysiciansReadingStudy ||
			iTag == kVLIInstitutionName || 
			iTag == kVLIStudyDescription || 
			iTag == kVLISeriesDescription)
			// 2007.05.20 kunikichi 
			// it is incorrect to  treat 0008,0005 tag to be merged multiple value in one
			// iTag == kVLISpecificCharacterSet)
		{
			*oValue = (*oValue > 1) ? 1 : *oValue;
		}

		return status;
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetAttributeInfo(unsigned long iTag, int* oValueCount, int* oMaxLength)
{
	MC_VR vr;

	if (this->CheckValid())
	{
		m_status = (PxDicomStatus)MC_Get_Attribute_Info(m_messageID, iTag, &vr, oValueCount);
	}

	*oMaxLength = kVRMaxLength[vr];

	return m_status;
}

#include "rtvsutil.h"

//-----------------------------------------------------------------------------
//
int GetRestOfValues(int iMessageID, long iTag, int iBufsize, char* oBuf)
{
	int status, count;
	status = MC_Get_Value_Count(iMessageID, iTag, &count);

	if (status != MC_NORMAL_COMPLETION || count < 2)
		return MC_NORMAL_COMPLETION;

	int i;
	iRTVSAlloc<char> tmpBuf(iBufsize);

	for(i = 1; i < count; i++)
	{
		memset(tmpBuf, 0, iBufsize);

		status = MC_Get_Next_Value_To_String(iMessageID, iTag, iBufsize, tmpBuf);
		if (status != MC_NORMAL_COMPLETION)
			return status;

		tmpBuf[iBufsize - 1] = 0;

		strncat_s(oBuf, iBufsize, "\\", iBufsize);
		strncat_s(oBuf, iBufsize, tmpBuf, iBufsize);
	}

	return MC_NORMAL_COMPLETION;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetValue(int iItemID, unsigned long iTag, char* oValue, int iSize)
{
	MC_STATUS mcStatus;

	if(!oValue)
		return (PxDicomStatus)MC_SYSTEM_ERROR;

	oValue[0] = 0;

	if (this->CheckValid())
	{
		mcStatus = MC_Get_Value_To_String(iItemID, iTag, iSize, oValue);

		//
		//
		if (iTag == kVLIPatientsName || 
			iTag == kVLIReferringPhysiciansName || 
			iTag == kVLINameOfPhysiciansReadingStudy ||
			iTag == kVLIInstitutionName || 
			iTag == kVLIStudyDescription || 
			iTag == kVLISeriesDescription ||
			iTag == kVLISpecificCharacterSet) //#137 2021/01/12 N.Furutsuki
		{
			GetRestOfValues(iItemID, iTag, iSize, oValue);
		}
		
		return (PxDicomStatus) mcStatus;
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetValue(unsigned long iTag, char* oValue, int iSize)
{
	return GetValue(m_messageID, iTag, oValue, iSize);
}

//------------------------------------------------------------------------------------------
//
std::string CPxDicomMessage::GetValue(unsigned long iTag)
{
	char buf[10240];
	memset(buf, 0, sizeof(buf));
	GetValue(m_messageID, iTag, buf, sizeof buf);

	return (strlen(buf) > 0) ? buf : "";
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetValue(unsigned long iTag, int* oValue)
{
	if(!oValue)
		return (PxDicomStatus)MC_SYSTEM_ERROR;

	*oValue = 0;

	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Get_Value_To_Int(m_messageID, iTag, oValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetValue(unsigned long iTag, float* oValue)
{
	if(!oValue)
		return (PxDicomStatus)MC_SYSTEM_ERROR;

	*oValue = 0;

	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Get_Value_To_Float(m_messageID, iTag, oValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetNextValue(unsigned long iTag, const char* iValue)
{ 
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Next_Value_From_String(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetNextValue(unsigned long iTag, int iValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Next_Value_From_Int(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetNextValue(unsigned long iTag, float iValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Next_Value_From_Float(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
// for completeness
PxDicomStatus CPxDicomMessage::SetNextValue(unsigned long iTag, double iValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Next_Value_From_Double(m_messageID, iTag, iValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetNextValue(unsigned long iTag, char* oValue, int iSize)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Get_Next_Value_To_String(m_messageID, iTag, iSize, oValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetNextValue(unsigned long iTag, int* oValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Get_Next_Value_To_Int(m_messageID, iTag, oValue);
	}

	return m_status;
}	

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::GetNextValue(unsigned long iTag, float* oValue)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Get_Next_Value_To_Float(m_messageID, iTag, oValue);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::DeleteAttribute(unsigned long iTag)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Delete_Attribute(m_messageID, iTag);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::ClearValue(unsigned long iTag)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Value_To_Empty(m_messageID, iTag);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::ClearAllValues(void)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Empty_Message(m_messageID);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SetValueToNull(unsigned long iTag)
{
	if (this->CheckValid())
	{
		return (PxDicomStatus) MC_Set_Value_To_NULL(m_messageID, iTag);
	}

	return m_status;
}

//------------------------------------------------------------------------------------------
//
void CPxDicomMessage::SetDataSource(DicomDataSource* iSource)
{
	m_dataSource.type = iSource->type;
	m_dataSource.m_server = iSource->m_server;
}

//------------------------------------------------------------------------------------------
//
bool CPxDicomMessage::IsSet(unsigned long iTag)
{
	int numValues = 0;

	m_status = (PxDicomStatus) MC_Get_Attribute_Info(m_messageID, iTag, NULL, &numValues);
	if (numValues == 0)
		return false;
	return true;
}


//--------------------------------------------------------------------
// This function is passed as a parameter to the toolkit's MC_Stream_To_Message() function call.
// ARGUEMENTS
//    A_msgID          int     Message ID that is being streamed out
//    A_cbInformation  void *  User information used by this routine
//    A_firstCall      int     Set to true on the first call to this routine
//    A_dataLen        int *   Length of data returned by the call
//    A_dataBuffer     void ** Pointer to data returned by this call
//    A_isLast         int *   Set by this function to true when completed
// RETURNS
//    MC_NORMAL_COMPLETION if the routine finishes properly.
//    MC_CANNOT_COMPLY if the routine detects an error.
//
MC_STATUS AqStreamToMessage (int A_msgID, 
						   void* A_cbInformation,
                           int A_firstCall, 
						   int* A_dataLen,
                           void** A_dataBuffer, 
						   int* A_isLast)
{
   static char     S_prefix[] = "StreamToMessage";
   size_t          bytes_read;
   MediaCBinfo*   callbackInfo = (MediaCBinfo*)A_cbInformation;

   bytes_read = fread (callbackInfo->buffer, 1,sizeof(callbackInfo->buffer), callbackInfo->fp);

   if (ferror (callbackInfo->fp))
   {
		callbackInfo->errCode |= kCBERRReadFile;
		return(MC_CANNOT_COMPLY);
   }

   if (feof ( callbackInfo->fp))
      *A_isLast = 1;
   else
      *A_isLast = 0;

   *A_dataBuffer = callbackInfo->buffer;
    
 
   *A_dataLen = (int)bytes_read;

   return MC_NORMAL_COMPLETION; 

} /* StreamToMessage */


//------------------------------------------------------------------------------------------
//
MC_STATUS MemoryStreamToMessage (int A_msgID, 
						   void* A_cbInformation,
                           int A_firstCall, 
						   int* A_dataLen,
                           void** A_dataBuffer, 
						   int* A_isLast)
{
	static char     S_prefix[] = "MemoryStreamToMessage";
	MemoryCBinfo*   callbackInfo = (MemoryCBinfo*)A_cbInformation;

    if (A_firstCall)
    {
        callbackInfo->bytesRead = 0;
    }
    
	unsigned long leftBytes = callbackInfo->dataSize - callbackInfo->bytesRead;
	int bytesToCopy = (int)(leftBytes > sizeof(callbackInfo->buffer) ? sizeof(callbackInfo->buffer) : leftBytes);
	if ( bytesToCopy > 0 )
	{
		memcpy(callbackInfo->buffer,
			callbackInfo->memoryPointerForRead+callbackInfo->bytesRead,
			bytesToCopy);
	}
	callbackInfo->bytesRead += bytesToCopy;
	*A_dataBuffer = callbackInfo->buffer;
	*A_dataLen = bytesToCopy;

	if ( leftBytes <= sizeof(callbackInfo->buffer) )
	{
        *A_isLast = 1;
	}

    return  MC_NORMAL_COMPLETION;
} /* MemoryStreamToMessage */


//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadHeader(const char* iFilePath, int iKeepAsFile)
{
	MC_STATUS status;
	int fileID;
	char			syntaxUID[65];
	TRANSFER_SYNTAX syntax;
    MediaCBinfo	mcbInfo;

	// don't need this - we have to do it on the fly 
	struct _stat statbuf;
	
	if (_stat(iFilePath, &statbuf) != 0)
	{
		return kInvalidArguments;
	}
	
	//	Avoid pixel-data callbacks
	if (!msgApplicationID)
	{
		status = MC_Register_Application(&msgApplicationID, "CPxDicomMessage");
		if (status != kNormalCompletion)
		{
			return (PxDicomStatus) status;
		}
	}

    status = MC_Create_Empty_File(&fileID, iFilePath); 
    if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;
    }

    long offset = -1;
    status = MC_Open_File_Upto_Tag(msgApplicationID, fileID, &mcbInfo, 
        MC_ATT_GROUP_7FE0_LENGTH, &offset, AqMediaToFileObj); 
    if (status != kNormalCompletion)
    {
        MC_Free_File(&fileID);
        return (PxDicomStatus) status;
    }


	status = MC_Get_Value_To_String(fileID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status; 
		}
	}

	if (!iKeepAsFile)
	{
		status = MC_File_To_Message(fileID); 
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status; 
		}

		status = MC_Set_Message_Transfer_Syntax(fileID, syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status;  
		}
	}

#if 0
	m_transferSyntax = IMPLICIT_LITTLE_ENDIAN;
	status = MC_Set_Message_Transfer_Syntax(fileID, (TRANSFER_SYNTAX) m_transferSyntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_File(&fileID);
		return (PxDicomStatus) status;  
	}
#endif

	MC_Free_Message(&m_messageID);
	m_messageID = fileID; 

	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadHeaderFromStream(const char* iFilePath, int iKeepAsFile)
{
	MC_STATUS		status;
	char			syntaxUID[65];
	TRANSFER_SYNTAX syntax;
    MediaCBinfo		mcbInfo;

	//	Get rid of any previous message data
	MC_Free_Message(&m_messageID);
	m_messageID = -1;
	MC_Open_Empty_Message(&m_messageID);

	//	Is the file really there?
	struct _stat statbuf;
	if (_stat(iFilePath, &statbuf) != 0)
	{
		return kInvalidArguments;
	}
	
	//	Stream in the tags (everything up-to but not including pixel data)
	status = (MC_STATUS) LoadFromMediaStream(iFilePath, mcbInfo, MC_ATT_GROUP_0002_LENGTH, 
		MC_ATT_GROUP_7FE0_LENGTH, IMPLICIT_LITTLE_ENDIAN);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_Message(&m_messageID);
		return kInvalidFile;
	}

	//	Try to populate MessageTransferSyntax based on stored transfer syntax
	status = MC_Get_Value_To_String(m_messageID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_Message(&m_messageID);
			return (PxDicomStatus) status; 
		}
	}

	status = MC_Set_Message_Transfer_Syntax(m_messageID, syntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_Message(&m_messageID);
		return (PxDicomStatus) status;  
	}

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::StartLoad()
{
	MC_STATUS		status;

	if (!(this->CheckValid()))
	{
		return m_status;
	}

	if (!msgApplicationID)
	{
		status = MC_Register_Application(&msgApplicationID, "CPxDicomMessage");
		if (status != kNormalCompletion)
		{
			return (PxDicomStatus)status;
		}
	}

	return kNormalCompletion;
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadFromMediaStream(const char * iFilePath,
													MediaCBinfo & iCBInfo,
													unsigned long iStartTag,
													unsigned long iStopTag,
													int iTransferSyntax)
{
	if ( !iFilePath )
		return kInvalidArguments;

	MC_STATUS		status;

	status = MC_Empty_Message(m_messageID);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus)status;
	} 

	// Stream in the data 
	if (NULL == (iCBInfo.fp = fopen (iFilePath, "rb")))
	{
		return (PxDicomStatus)MC_SYSTEM_ERROR;
	}

	unsigned long errorTagPtr;
	return (PxDicomStatus)MC_Stream_To_Message(m_messageID,
								  iStartTag,
								  iStopTag,
								  (TRANSFER_SYNTAX) iTransferSyntax,  
								  &errorTagPtr,
								  (void *)&iCBInfo,
								  AqStreamToMessage);

}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadFromFileID(int iFileID, int iKeepAsFileObj)
{
	MC_STATUS		status;
	char			syntaxUID[65];
 	TRANSFER_SYNTAX syntax;

	status = MC_Get_Value_To_String(iFileID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&iFileID);
			return (PxDicomStatus)status;  
		}
	}
	if(!iKeepAsFileObj)
	{
		status = MC_File_To_Message(iFileID); 
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&iFileID);
			return (PxDicomStatus)status; 
		}


		if (syntax != IMPLICIT_LITTLE_ENDIAN &&
			syntax != EXPLICIT_LITTLE_ENDIAN &&
			syntax != EXPLICIT_BIG_ENDIAN)
		{
			status = MC_Set_Message_Transfer_Syntax(iFileID, syntax);
			if (status != MC_NORMAL_COMPLETION)
			{
				MC_Free_Message(&iFileID);
				return (PxDicomStatus)status;  
			}
		}
	}
	if (!m_doNotFree)
	{
		MC_Free_Message (&m_messageID);
	}

	m_transferSyntax = (int) syntax;
	m_messageID = iFileID;

	return kNormalCompletion;

}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadFileInternal(const char *iFilePath,
												 char* oPreamble, int iKeepAsFileObj)
{

	if ( !iFilePath )
		return kInvalidArguments;

	PxDicomStatus vliStatus = StartLoad();
	if ( vliStatus != kNormalCompletion )
		return vliStatus;

	MC_STATUS		status;
	int	fileID;
	status = MC_Create_Empty_File(&fileID, iFilePath);
	if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;  
    }
	
	MediaCBinfo		cbinfo;
	status = MC_Open_File(msgApplicationID, fileID, &cbinfo, AqMediaToFileObj);

	if ( status != MC_NORMAL_COMPLETION) 
	{
		MC_Free_File(&fileID);
		if ( oPreamble )
			return (PxDicomStatus) MC_SYSTEM_ERROR;

		vliStatus = LoadFromMediaStream(iFilePath, cbinfo, MC_ATT_GROUP_0002_LENGTH, 0xffFFffFF, IMPLICIT_LITTLE_ENDIAN);
		if (vliStatus != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) MC_SYSTEM_ERROR;
		}
		else
			return kNormalCompletion;
	}
	else
	{	
		if ( oPreamble )
		{
			status = MC_Get_File_Preamble(fileID,oPreamble);
			if ( status != MC_NORMAL_COMPLETION )
			{
				MC_Free_File(&fileID);
				return (PxDicomStatus) status;
			}
		}
		return (PxDicomStatus) LoadFromFileID(fileID, iKeepAsFileObj);
	}

}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::Load(const char *iFilePath, int iHeaderOnly, int iKeepAsFile)
{
	PxDicomStatus status;

	if ( !iFilePath )
		return kInvalidArguments;

	if (iHeaderOnly)
	{
		status = LoadHeader(iFilePath, iKeepAsFile);
		if (status != kNormalCompletion)
			status = LoadHeaderFromStream(iFilePath, iKeepAsFile);

		return status;
	}

	return LoadFileInternal(iFilePath,0, iKeepAsFile);
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadWithPreamble(const char *iFilePath, 
												 char *oPreamble)
{
	if ( !iFilePath || !oPreamble)
		return kInvalidArguments;

	return LoadFileInternal(iFilePath,oPreamble);
}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadFromMemoryStream(MemoryCBinfo & iCBInfo)
{
	MC_STATUS		status;

	status = MC_Empty_Message(m_messageID);
	if (status != MC_NORMAL_COMPLETION)
		return (PxDicomStatus)status;

	unsigned long errorTagPtr;
	return (PxDicomStatus)MC_Stream_To_Message(m_messageID,
								  MC_ATT_GROUP_0002_LENGTH,
								  0xffFFffFF,
								  IMPLICIT_LITTLE_ENDIAN,  
								  &errorTagPtr,
								  (void *)&iCBInfo,
								  MemoryStreamToMessage);

}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::LoadMemoryInternal(const char *iFileInMemory,
													const unsigned long iFileSize)
{
	if ( !iFileInMemory )
		return kInvalidArguments;

	PxDicomStatus vliStatus = StartLoad();
	if ( vliStatus != MC_NORMAL_COMPLETION )
		return (PxDicomStatus)vliStatus;

	MC_STATUS		status;
	int	fileID;
	status = MC_Create_Empty_File(&fileID, "");
	if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;  
    }

	MemoryCBinfo	cbinfo;
	cbinfo.memoryPointerForRead = iFileInMemory;
	cbinfo.dataSize = iFileSize;	
	status = MC_Open_File(msgApplicationID, fileID, &cbinfo, AqMemoryToFileObj);
	
	if ( status != MC_NORMAL_COMPLETION) 
	{
		MC_Free_File(&fileID);
		vliStatus = LoadFromMemoryStream(cbinfo);
		if (vliStatus != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) MC_SYSTEM_ERROR;
		}
		else
			return kNormalCompletion;
	}
	else
	{	
		return (PxDicomStatus) LoadFromFileID(fileID);
	}

}


//-----------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::ProcessGetPixelOffset(int CBtype, unsigned long* dataSizePtr, void** dataBufferPtr)
{
	//	Merge tells us how much data is on media
	if (CBtype == PROVIDING_MEDIA_DATA_LENGTH)
	{
		m_pixelDataSize   = (unsigned long) *dataSizePtr;
		m_pixelDataOffset = (unsigned long) *((long*) *dataBufferPtr);

		if (!m_pixelDataSize || !m_pixelDataOffset)
			return (PxDicomStatus) MC_CALLBACK_CANNOT_COMPLY;
			
		return (PxDicomStatus) MC_NORMAL_COMPLETION;
	}
	
	return (PxDicomStatus) MC_NORMAL_COMPLETION;

}

//-----------------------------------------------------------------------------
//
MC_STATUS GetPixelOffsetCB(int m_messageID,unsigned long tag,void* userInfo,CALLBACK_TYPE CBtype, 
					 unsigned long* dataSizePtr,void** dataBufferPtr,int isFirst,int* isLastPtr)
{
	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	CPxDicomMessage* pMessage = (CPxDicomMessage*) userInfo;
	return (MC_STATUS) pMessage->ProcessGetPixelOffset(CBtype, dataSizePtr, dataBufferPtr);
}

//-----------------------------------------------------------------------------
//
//GL it is not multi-thread safe, beacause MC_Register_Callback_Function for globle call back used
PxDicomStatus CPxDicomMessage::LoadHeader(const char* iFileName, unsigned long& oOffset, unsigned long& oSize)
{
	MC_STATUS status;
	char syntaxUID[65];
	TRANSFER_SYNTAX syntax;
	static int pixelOffsetApplicationID;

	if (!pixelOffsetApplicationID)
	{
		status = MC_Register_Application(&pixelOffsetApplicationID, "PIXELOFFSET");
		if (status != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) status;
		}
	}

	status = MC_Register_Callback_Function(pixelOffsetApplicationID, MC_ATT_PIXEL_DATA, this, GetPixelOffsetCB);
	if (status != MC_NORMAL_COMPLETION)
	{
		return  (PxDicomStatus) status;
	}

	int fileID = -1;
	status = MC_Create_Empty_File(&fileID, iFileName);
	if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;  
    }

	MediaCBinfo	cbinfo;
	status = MC_Open_File_Bypass_OBOW(pixelOffsetApplicationID, fileID, &cbinfo, AqMediaToFileObj);
	if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;  
    }

	oOffset = m_pixelDataOffset;
	oSize = m_pixelDataSize;   

	status = MC_Get_Value_To_String(fileID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status; 
		}
	}

	status = MC_File_To_Message(fileID); 
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_File(&fileID);
		return (PxDicomStatus) status; 
	}


	status = MC_Set_Message_Transfer_Syntax(fileID, syntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_File(&fileID);
		return (PxDicomStatus) status;  
	}
	m_transferSyntax = syntax;

	MC_Free_Message(&m_messageID);
	m_messageID = fileID; 

	return (PxDicomStatus) MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::WriteFile(const char* iSavePath, const char* iLocalAE)
{
	CBinfo cbinfo;
	int status;

	int fileID = m_messageID;

	status = TRDICOMUtil::AddGroup2Elements(fileID, (TRANSFER_SYNTAX) m_transferSyntax, iLocalAE);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;
	}
	
	//YZ - Grey, fix purify warning.
	cbinfo.fp = 0;

	
	//	So the callback can check if it's passed too much data
	status = MC_Get_File_Length(fileID, &cbinfo.dataSize);
	if (status != MC_NORMAL_COMPLETION)
		cbinfo.dataSize = 0;

	//	Write the file to disk
	status = MC_Write_File(fileID, 0, &cbinfo, AqFileObjToMedia);
	if (status != MC_NORMAL_COMPLETION)
	{
		return (PxDicomStatus) status;
	}

	//	if it's still open, close the file handle
	if (cbinfo.fp)
		fclose(cbinfo.fp);

	return kNormalCompletion;
}


//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::Save(const char* iSavePath, const char* iLocalAE)
{
	PxDicomStatus status;
	status = SaveStart(iSavePath,0);
	if ( status != kNormalCompletion )
		return status;

	status = WriteFile(iSavePath, iLocalAE);

	MC_File_To_Message(m_messageID);

	return status;

}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SaveWithPreamble(const char* iSavePath, 
									 const char * iPreamble, 
									 const char* iLocalAE)
{
	if ( !iPreamble )
		return kInvalidArguments;	

	PxDicomStatus status;
	status = SaveStart(iSavePath,iPreamble);
	if ( status != kNormalCompletion )
		return status;

	status = WriteFile(iSavePath, iLocalAE);

	MC_File_To_Message(m_messageID);

	return status;

}

//------------------------------------------------------------------------------------------
//
PxDicomStatus CPxDicomMessage::SaveStart(const char* iSavePath, 
									 const char * iPreamble)
{
	if ( !iSavePath )
		return kInvalidArguments;	

	PxDicomStatus status;

	if (!(this->CheckValid()))
	{
		return m_status;
	}

	status = (PxDicomStatus) MC_Message_To_File(m_messageID,  iSavePath);
	if (status != kNormalCompletion)
	{
		return status;  
	}

	if ( iPreamble )
	{
		status = (PxDicomStatus)MC_Set_File_Preamble(m_messageID, iPreamble);
		if (status != kNormalCompletion)
		{
			MC_File_To_Message(m_messageID);
			return status;  
		}
	}

	return kNormalCompletion;
}


//-----------------------------------------------------------------------------------------
//
//  AUG-14-2003 use Atomic variable for the sequence number
const char*	CPxDicomMessage::GenerateUID(const char* iUID)
{
#if 1
	std::string s = TRPlatform::GenerateUID(iUID);
	nvrstrncpy(m_UID, s.c_str(), sizeof m_UID);
#else
	static TRAtomicVar<unsigned int> sSeq;
	unsigned int seq = ++sSeq;
	char thisSuffix[64];
	
	_snprintf(thisSuffix,sizeof thisSuffix,"%u.%u.%u", 
		TRPlatform::GetIPAddress(),seq,time(0));
	thisSuffix[sizeof thisSuffix - 1] = '\0';
	
	_snprintf(m_UID, sizeof m_UID, "%s.%s", sRTVRootUID,thisSuffix);
	m_UID[sizeof m_UID - 1] = '\0';
	
	if (strlen(m_UID)&1)
		strcat(m_UID,"1");
#endif
	return m_UID;
}


#if 1
void MessageFreeGuard::Free() 
{
	if (m_messageID < 0 || !m_guard)
		return;
	if(m_fileType)
		MC_Free_File(&m_messageID);
	else
		MC_Free_Message(&m_messageID);
}
#endif


// this function fills patient information to oData 
// for audit trail event log.

// only work with file ID after the file saved or read in

PxDicomStatus  CPxDicomMessage::FillPatientInfo(DICOMData& oData)
{
	oData.Clear();

	int idx = 0;
	int stat[100];

	//	Study Level tags
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_INSTANCE_UID, 
		sizeof(oData.m_studyInstanceUID), oData.m_studyInstanceUID);

	iRTVDeSpaceDe(oData.m_studyInstanceUID); // this 

	stat[idx++] = GetValue(m_messageID, MC_ATT_PATIENTS_NAME, oData.m_patientsName, kVR_PN);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENT_ID, kVR_LO, oData.m_patientID);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENTS_BIRTH_DATE, kVR_DA, oData.m_patientsBirthDate);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENTS_SEX, kVR_CS, oData.m_patientsSex);

	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_DATE, kVR_DA, oData.m_studyDate);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_TIME, kVR_TM, oData.m_studyTime);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_ACCESSION_NUMBER, kVR_SH, oData.m_accessionNumber);
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_STUDY_ID, kVR_SH,	oData.m_studyID);
	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_PATIENTS_AGE, &oData.m_patientsAge);

	// for compatibility with various parts of the code where we remove space
	// from both ends of the string
	iRTVDeSpaceDe(oData.m_patientsName);
	iRTVDeSpaceDe(oData.m_patientID);
	iRTVDeSpaceDe(oData.m_accessionNumber);
	
	stat[idx++] = GetValue(m_messageID, MC_ATT_REFERRING_PHYSICIANS_NAME, oData.m_referringPhysiciansName, kVR_PN);

	// 
	stat[idx++] = GetValue(m_messageID, MC_ATT_NAME_OF_PHYSICIANS_READING_STUDY, oData.m_radiologistName,kVR_PN);
	stat[idx++] = MC_Get_Value_To_String(m_messageID,MC_ATT_STUDY_DESCRIPTION,
		sizeof(oData.m_studyDescription),oData.m_studyDescription);

	//	Series Level tags
	stat[idx++] = MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_INSTANCE_UID, 
		sizeof(oData.m_seriesInstanceUID), oData.m_seriesInstanceUID);

	iRTVDeSpaceDe(oData.m_seriesInstanceUID);


	stat[idx++] = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_SERIES_NUMBER, &oData.m_seriesNumber);

	// in dicom image, we fill SC even it is not in the header,
	// do we need to do the same thing here?
    stat[idx++] = MC_Get_Value_To_String(m_messageID,MC_ATT_MODALITY,
		sizeof(oData.m_modality),oData.m_modality); 
    if (stat[idx] != kNormalCompletion)
    {
		char sopClassUID[kVR_UI];
		MC_Get_Value_To_String(m_messageID, MC_ATT_SOP_CLASS_UID, sizeof(sopClassUID), sopClassUID);
		if (strcmp (sopClassUID, "1.2.840.10008.5.1.4.1.1.7") == 0)
		{
			strcpy(oData.m_modality, "SC");
		}
    }

    stat[idx++] = MC_Get_Value_To_String(m_messageID,MC_ATT_SERIES_DESCRIPTION,
		sizeof(oData.m_seriesDescription),oData.m_seriesDescription);
	

	// get transfer syntax
 	TRANSFER_SYNTAX syntax = (TRANSFER_SYNTAX)GetTransferSyntax();
	oData.m_transferSyntax = (int) syntax;

	return kNormalCompletion;
}


//#137 2021/01/12 N.Furutsuki
//------------------------------------------------------------------------------------------
// iByteLength = (string length * sizeof(wchar_t))
// if user provides string length then output string will be truncated.
std::string CPxDicomMessage::GetCharacterSets(int iMsgID)
{
	std::string charSet;
	//Extract char sets
	int count = 0;
	char buf[kVR_CS]; buf[0] = '\0';
	
	MC_STATUS mcStatus = MC_Get_Value_Count(iMsgID, MC_ATT_SPECIFIC_CHARACTER_SET, &count);
	if (mcStatus != MC_NORMAL_COMPLETION || count < 1) //If nothing present then set the value to null
		return "";

	mcStatus = MC_Get_Value_To_String(iMsgID, MC_ATT_SPECIFIC_CHARACTER_SET, sizeof buf, buf);
	if ( (mcStatus != MC_NORMAL_COMPLETION) && ( mcStatus != MC_NULL_VALUE ) ) //If extracttion failed then set the value to null
		return "";

	charSet = charSet + buf;

	//	Get the rest
	for(int i = 0; i < count-1; i++)
	{
		buf[0] = '\0';
		mcStatus = MC_Get_Next_Value_To_String(iMsgID, MC_ATT_SPECIFIC_CHARACTER_SET, sizeof buf, buf);
		if ( (mcStatus != MC_NORMAL_COMPLETION) && ( mcStatus != MC_NULL_VALUE ) ) //Use partly extracted SCS
			return "";

		charSet = charSet + "\\" + buf;
	}

	return charSet;
}