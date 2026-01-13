//
//	VLIDicom.cpp
//

#pragma warning(disable: 4786) 
 

#include "VLIDicom.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#include "pxDicomServer.h"

#else
#include "rtvMergeToolKit.h"

#endif

#include "AqCore/TRPlatform.h"
//#include "VLIDicomVolume.h"
//#include "TRDICOMUtil.h"
#include "PxDICOMUtil.h"

#include "DiCOMStore.h"
#include "AppComDataConversion.h"
#include "rtvsutil.h"

 
#include "AqCore/TRLogger.h"
 
TRLogger gLogger;

#include "VLIDicomConfig.h"

//#define _PROFILE_THIS
#if defined(_PROFILE)
	#include "ScopeTimer.h"
#elif defined(_PROFILE_THIS)
	#include "ScopeTimer.h"
#endif

int VLIDicomApplicationID = -1;
char VLIDicomLocalAETitle[kMaxLocalAELen];

static const char* kDefaultLocalCache   = "C:/AQNet/Raid1/AQNetCache";
static TRCriticalSection cs;


//-----------------------------------------------------------------------------------------
//
int GetMoveMessageID(int iMessageID)
{
	int id = kInvalidMessageID;

	MC_Get_Value_To_Int(iMessageID, MC_ATT_MOVE_ORIGINATOR_MESSAGE_ID, &id);
	return id;
}


// version and build info (-- 12/13/2001)
//----------------------------------------------------------------------------------------
//
const char* VLIDicom::GetBuildString(void)
{
	static char buf[64];
	
	if (!buf[0])
		_snprintf(buf,sizeof buf,"built on %s",__DATE__);
	
	return buf;
}

//----------------------------------------------------------------------------------------
//
const char* VLIDicom::GetVersionString(void)
{
	static char buf[128];
	
	char mergeVersion[64];
	int vLength = sizeof mergeVersion;
	
	if (!buf[0])
	{
		_snprintf(buf, sizeof buf, "V%d.%d.%d", kVLIDICOMSCUVersion, kVLIDICOMSCURevision, kVLIDICOMSCUTweaks);
		
		MC_STATUS mcStatus = MC_Get_Version_String(vLength, mergeVersion);
		if (mcStatus == MC_NORMAL_COMPLETION)
		{
			STRNCAT_S(buf, " - Merge_", sizeof buf);
			STRNCAT_S(buf, mergeVersion, sizeof buf);
		}
	}
	return buf;
 }

//----------------------------------------------------------------------------------------
//
void VLIDicom::GetVersion(int& oMajor, int& oMinor, int& oTweaks)
{
	oMajor = kVLIDICOMSCUVersion;
	oMinor = kVLIDICOMSCURevision;
	oTweaks = kVLIDICOMSCUTweaks;
}

bool VLIDicomBase::m_printDebugMessages = false;
bool VLIDicomBase::	m_reopenAssociationEachTime = true;
//-----------------------------------------------------------------------------------------
//	
//	Default Constructor
//
//-----------------------------------------------------------------------------------------
VLIDicomBase::VLIDicomBase() 
{
#if defined(_PROFILE)
	ScopeTimer::Init(this);
#elif defined(_PROFILE_THIS)
	ScopeTimer::Init(this);
#endif

	strncpy(m_localCache, kDefaultLocalCache, kMaxPathNameLen);
	m_status = kNormalCompletion;

	
	//	-- - 05/03/02 - Need this so certain kinds of queries can timeout early.  Measured in seconds
	m_timeoutOnQuery = 60;	
	m_timeoutOnRetrieve = 45;
	m_maxDICOMQueryResults = 1000;
	//	-- 08/06/02 Should duplicate query results be removed?  Defaults to Yes.
	m_removeDuplicateQueryResults = 1;
	m_connectCMoveBySeriesUID = 0;
}

//-----------------------------------------------------------------------------------------
//
void VLIDicomBase::SetReopenAssociationEachTime(bool iYesNo)
{
	m_reopenAssociationEachTime = iYesNo;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::AddServer(const char* iAETitle, const char* iHostname, int iPort, int iConnect)
{
	MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, 10);
	MC_Set_Int_Config_Value(CONNECT_TIMEOUT, 10);

	bool first = true;
	m_cancel = 0;

	DicomServer newServer(iAETitle,iHostname,iPort);
	
	if (!m_serverList.empty())
	{
		first = false;
		DicomServerListIter iter = m_serverList.begin();
		for(;iter != m_serverList.end(); iter++)
		{
			// Fixed this on 12/11/2001 --
			// we need to check both the AETitle and Host
			if (*iter == newServer) 
			{
				return kNormalCompletion;	// It's already there - no need to add it
			}
		}
	}
	
	newServer.m_associationID = -1;

	if (iConnect)
	{
		VLIAssociationChecker check(&newServer, this, "AddServer");
		PxDicomStatus status = check.GetStatus();

		if (status != kNormalCompletion)
			return status;

		if (newServer.m_associationID < 0)
			return kConnectionFailed;
	}
	
	m_serverList.push_back(newServer);

	if (first)
	{
		m_currentServer = *(m_serverList.begin());
	}
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::UpdateServer(DicomServer& iServer)
{
#ifdef _PROFILE
	ScopeTimer timer("UpdateServer");
#endif
	if (!m_serverList.empty())
	{
		for(DicomServerListIter iter = m_serverList.begin();iter != m_serverList.end(); iter++)
		{
			if (*iter == iServer)
			{
				*iter = iServer;
				return kNormalCompletion;			
			}
		}
	}
	
	return kNormalCompletion;
}

// -- 04/01/2003 - need this to select different servers - limited use
void	VLIDicomBase::UpdateStudyCache(const char* iStudyUID, DicomDataSource* iSource, const char* modality)
{
	tStudyCache::iterator it;
	if (!iSource) return;

	if (m_studyCache.find(iStudyUID) == m_studyCache.end())
		m_studyCache[iStudyUID] = new VLICachedInfo(*iSource);

	if (( it = m_studyCache.find(iStudyUID)) != m_studyCache.end())
	{
		it->second->SetDataSource(*iSource);
		it->second->SetModality(modality);
	}
}

//----------------------------------------------------------------------------------------
// -- 2005.10.11
// Terrible hack to get data source information
DicomDataSource* VLIDicomBase::GetStudySource(const char* iStudyUID)
{
	tStudyCache::iterator it;

	if (( it = m_studyCache.find(iStudyUID)) == m_studyCache.end())
		return 0;

	return &it->second->GetDataSource();
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::RemoveServer(DicomServer& iServer)
{
#ifdef _PROFILE
	ScopeTimer timer("RemoveServer");
#endif
	if (!m_serverList.empty())
	{
		for(DicomServerListIter iter = m_serverList.begin();iter != m_serverList.end(); iter++)
		{
			if (*iter == iServer)
			{
//				if (iter->m_associationID > 0)
//				{
					MC_Close_Association(&iter->m_associationID);
//				}
				iter->m_associationID = -1;
				m_serverList.erase(iter);
				if (!m_serverList.empty())
				{
					m_currentServer = *m_serverList.begin();
				}

				return kNormalCompletion;			

			}
		}
	}
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::RemoveAllServers()
{
#ifdef _PROFILE
	ScopeTimer timer("RemoveAllServers");
#endif
	DicomServer ds;
	if (!m_serverList.empty())
	{
		DicomServerListIter iter = m_serverList.begin();
		DicomServerListIter iter2;
		do
		{
			if (iter != m_serverList.end())
			{
				iter2 = iter;
			//	if (iter2->m_associationID > 0)
			//	{
					MC_Close_Association(&iter2->m_associationID);
			//	}
				iter2->m_associationID = -1;
				iter = m_serverList.erase(iter2);
			}
		} while (iter != m_serverList.end());
	}

	m_currentServer = DicomServer();
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::AdvanceServer()
{
#ifdef _PROFILE
	ScopeTimer timer("AdvanceServer");
#endif
	if (m_serverList.empty())
		return kEmptyServerList;

	if (m_serverList.size() > 1)
	{
		for(DicomServerListIter iter = m_serverList.begin();iter != m_serverList.end(); iter++)
		{
			if (*iter == m_currentServer)
			{
				//	-- - 11/17/02 - Copy back associationID, etc
				*iter = m_currentServer;		

				++iter;
				if (iter == m_serverList.end())
				{
					iter = m_serverList.begin();
				}
				m_currentServer = *iter;
			}
		}
	} 

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::SetCurrentServer(DicomServer& iServer)
{
#ifdef _PROFILE
	ScopeTimer timer("SetCurrentServer");
#endif
	if (m_serverList.empty())
	{
		return kEmptyServerList;
	}
	
	DicomServerListIter iter = m_serverList.begin();
	for(;iter != m_serverList.end(); iter++)
	{
		if (*iter == iServer)
		{
			m_currentServer = *iter;
			return kNormalCompletion;
		}
	}
	
	return kUnknownServer;
}

//----------------------------------------------------------------------------------------
//
int StringToBool(const char* s)
{
	if (!strcmp(s,"yes") || !strcmp(s,"on") || !strcmp(s,"true") ||
		!strcmp(s,"Yes") || !strcmp(s,"On") || !strcmp(s,"True") ||
		!strcmp(s,"YES") || !strcmp(s,"ON") || !strcmp(s,"TRUE"))
	{
		return 1;
	} else if (!strcmp(s,"no") || !strcmp(s,"off") || !strcmp(s,"false") ||
		!strcmp(s,"No") || !strcmp(s,"Off") || !strcmp(s,"False") ||
		!strcmp(s,"NO") || !strcmp(s,"OFF") || !strcmp(s,"FALSE"))
	{
		return 0;
	} else
	{
		return -1;
	}
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::SetConfigValue(int iParameterKey, const char* iParameterValue)
{
#ifdef _PROFILE
	ScopeTimer timer("SetConfigValue");
#endif
	int val = 0;
	
	if (iParameterValue == NULL)
	{
		return (kConfigInfoError);
	}
	
	if (iParameterKey >= kConfigVLIParmOffset)
	{
		switch (iParameterKey)
		{
		case kcLocalAeTitle:
			strncpy(VLIDicomLocalAETitle, iParameterValue, kMaxLocalAELen);
			return (PxDicomStatus) MC_Register_Application(&VLIDicomApplicationID, VLIDicomLocalAETitle);
			break;
		case kcLocalCache:
			strncpy(m_localCache, iParameterValue, kMaxPathNameLen);
			break;
/*
		case kcAutoCache:
			val = StringToBool(iParameterValue);
			if (val < 0)
			{
				return (kConfigInfoError);
			} else
			{
				m_autoCache = (val == 1) ? true : false;
			}
			break;
		case kcTISendSopClassUid:
			m_TISendSopClassUid = atoi(iParameterValue);
			break;
		case kcTISendSopInstanceUid:
			m_TISendSopInstanceUid = atoi(iParameterValue);
			break;
		case kcTISendMsgIdResponse:
			m_TISendMsgIdResponse = atoi(iParameterValue);
			break;
		case kcTISendResponsePriority:
			m_TISendResponsePriority = atoi(iParameterValue);
			break;
		case kcRetrieveStoresOriginals:
			m_retrieveStoresOriginals = atoi(iParameterValue);
			break;
		case kcNumberOfRetrieveThreads:
			m_numberOfRetrieveThreads = atoi(iParameterValue);
			break;
*/
		case kcTimeoutOnRetrieve:
			m_timeoutOnRetrieve = atoi(iParameterValue);
			break;
		case kcTimeoutOnQuery:
			m_timeoutOnQuery = atoi(iParameterValue);
			break;
		case kcMaxDICOMQueryResults:
			m_maxDICOMQueryResults = atoi(iParameterValue);
			break;
		case kcRemoveDuplicateQueryResults:
			m_removeDuplicateQueryResults = atoi(iParameterValue);
			break;
		case kcForceTransferSyntax:
			m_forceTransferSyntax = atoi(iParameterValue);
			break;
		case kcConnectCMoveBySeriesUID:
			m_connectCMoveBySeriesUID = atoi(iParameterValue);
			break;
		};
		
		return kNormalCompletion;
	} else if (iParameterKey >= kConfigLongParmOffset)
	{
		int key = iParameterKey - kConfigLongParmOffset;
		long val = atol(iParameterValue);
		return (PxDicomStatus) MC_Set_Long_Config_Value((LongParm)key, val);
	} else if (iParameterKey >= kConfigBoolParmOffset)
	{
		int key = iParameterKey - kConfigBoolParmOffset;
		int val = StringToBool(iParameterValue);
		if (val < 0)
		{
			return (kConfigInfoError);
		} else
		{
			return (PxDicomStatus) MC_Set_Bool_Config_Value((BoolParm)key, val);
		}
	} else if (iParameterKey >= kConfigIntParmOffset)
	{
		int key = iParameterKey - kConfigIntParmOffset;
		int val = atoi(iParameterValue);
		return (PxDicomStatus) MC_Set_Int_Config_Value((IntParm)key, val);
	} else if (iParameterKey >= kConfigStringParmOffset)
	{
		int key = iParameterKey - kConfigStringParmOffset;
		return (PxDicomStatus) MC_Set_String_Config_Value((StringParm)key, (char *)iParameterValue);
		//	These come from us, not MergeCOM
	} else
	{
		return (kConfigInfoError);
	}
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::GetConfigValue(int iParameterKey, char* oParameterValue)
{
#ifdef _PROFILE
	ScopeTimer timer("GetConfigValue");
#endif
	PxDicomStatus status;
	
	if (oParameterValue == NULL)
	{
		return (kConfigInfoError);
	}
	
	if (iParameterKey >= kConfigLongParmOffset)
	{
		int key = iParameterKey - kConfigLongParmOffset;
		long val;
		status = (PxDicomStatus) MC_Get_Long_Config_Value((LongParm)key, &val);
		if (status != kNormalCompletion)
		{
			return (status);
		}
		sprintf(oParameterValue,"%d",val);
		return (kNormalCompletion);
	} else if (iParameterKey >= kConfigBoolParmOffset)
	{
		int key = iParameterKey - kConfigBoolParmOffset;
		int val;
		status = (PxDicomStatus) MC_Get_Bool_Config_Value((BoolParm)key, &val);
		if (status != kNormalCompletion)
		{
			return (status);
		}
		if (val)
		{
			sprintf(oParameterValue,"true");
		} else
		{
			sprintf(oParameterValue,"false");
		}
		return (kNormalCompletion);
	} else if (iParameterKey >= kConfigIntParmOffset)
	{
		int key = iParameterKey - kConfigIntParmOffset;
		int val;
		status = (PxDicomStatus) MC_Get_Int_Config_Value((IntParm)key, &val);
		if (status != kNormalCompletion)
		{
			return (status);
		}
		sprintf(oParameterValue,"%d",val);
		return (kNormalCompletion);
	} else if (iParameterKey >= kConfigStringParmOffset)
	{
		int key = iParameterKey - kConfigStringParmOffset;
		
		status = (PxDicomStatus) MC_Get_String_Config_Value((StringParm)key, kMaxConfigLen, oParameterValue);
		if (status != kNormalCompletion)
		{
			return (status);
		}
		return (kNormalCompletion);
		//	These are from us, not MergeCOM
	} else if (iParameterKey >= kConfigVLIParmOffset)
	{
		switch (iParameterKey)
		{
		case kcLocalAeTitle:
			strncpy(oParameterValue, VLIDicomLocalAETitle, kMaxLocalAELen);
			break;
		case kcLocalCache:
			strncpy(oParameterValue, m_localCache, kMaxPathNameLen);
			break;
/*
		case kcAutoCache:
			if (m_autoCache)
			{
				sprintf(oParameterValue,"true");
			} else
			{
				sprintf(oParameterValue,"false");
			}
			break;
		case kcTISendSopClassUid:
			sprintf(oParameterValue, "%d", m_TISendSopClassUid);
			break;
		case kcTISendSopInstanceUid:
			sprintf(oParameterValue, "%d", m_TISendSopInstanceUid);
			break;
		case kcTISendMsgIdResponse:
			sprintf(oParameterValue, "%d", m_TISendMsgIdResponse);
			break;
		case kcTISendResponsePriority:
			sprintf(oParameterValue, "%d", m_TISendResponsePriority);
			break;
		case kcRetrieveStoresOriginals:
			sprintf(oParameterValue, "%d", m_retrieveStoresOriginals);
			break;
		case kcNumberOfRetrieveThreads:
			sprintf(oParameterValue, "%d", m_numberOfRetrieveThreads);
			break;
*/
		case kcTimeoutOnRetrieve:
			sprintf(oParameterValue, "%d", m_timeoutOnRetrieve);
			break;
		case kcTimeoutOnQuery:
			sprintf(oParameterValue, "%d", m_timeoutOnQuery);
			break;
		case kcMaxDICOMQueryResults:
			sprintf(oParameterValue, "%d", m_maxDICOMQueryResults);
			break;
		case kcForceTransferSyntax:
			sprintf(oParameterValue, "%d", m_forceTransferSyntax);
			break;
		case kcConnectCMoveBySeriesUID:
			sprintf(oParameterValue, "%d", m_connectCMoveBySeriesUID);
			break;
		};
		return (kNormalCompletion);
	}
	return(kConfigInfoError);
}

//-----------------------------------------------------------------------------
//
PxDicomStatus VLIDicomBase::LoadDicomHeader(CPxDicomImage*& oMessage, const char* iFilePath)
{
	const int cHeaderOnly = 1;
	CPxDicomMessage msg;
	PxDicomStatus status;

	status = msg.Load(iFilePath, cHeaderOnly);
	if (status != kNormalCompletion)
	{
		return status;
	}

	oMessage = new CPxDicomImage(msg.HandoverID()); 
	return kNormalCompletion;
}

typedef struct 
{
	_fsize_t size;
	unsigned int attrib;
	char path[kMaxPathNameLen];
} fileInfo;

#if 0
//-----------------------------------------------------------------------------------------
//
//	Recursively traverses "path" and builds a std::vector of filenames, including the absolute 
//	path such that each directory that contains plain files (non-directory files) will 
//	contribute only one file to the std::vector.  This is useful for querying the local cache.
void BuildFirstFileVector(std::vector<fileInfo>& files, char* path)
{
	long hFile;
	struct _finddata_t cfile;
	fileInfo f;
	int count = 0;
	
	//	Remember current working directory	
	char cwd[kMaxPathNameLen];
	if (_getcwd(cwd, kMaxPathNameLen) == NULL)
	{
		return;
	}
	
	//	Go to the local cache
	if(_chdir(path))	
	{
		return;
	}
	
	//	Build the list of files - store in std::vector "files"
	hFile = _findfirst("*", &cfile);
	while (_findnext(hFile, &cfile) == 0)
	{
		//	If the file is a directory
		if(cfile.attrib & _A_SUBDIR)
		{
			//	Ignore current and parent dirs
            if ((strcmp(cfile.name,".") != 0) && (strcmp(cfile.name,"..")!= 0))
			{
				char p[kMaxPathNameLen];
				
				strncpy(p, path, kMaxPathNameLen - (strlen(cfile.name) + 2));
				strcat(p, "\\");
				strcat(p, cfile.name);
				BuildFirstFileVector(files, p);
			}
		} else if (count < 1)	//	We only want the first file - ignore the rest
		{
			++count;
			f.size = cfile.size;
			f.attrib = cfile.attrib;
			strncpy(f.path, path, kMaxPathNameLen - (strlen(cfile.name) + 2));
			strcat(f.path,"\\");
			strcat(f.path, cfile.name);
			files.push_back(f);
		}
	}
	
	//	Close _find structure and return to original working directory
	_findclose(hFile);
	_chdir(cwd);
	
	return;
}
#endif

//----------------------------------------------------------------------------------------
//
DicomDataSource& VLIDicomBase::PickBestServer(std::vector<DicomDataSource> dsv)
{
	return dsv[0];
	/*	float max = 0.0;
	DicomDataSource& ds = dsv[0];
	
	  for (int i=0; i<dsv.size(); i++)
	  {
	  if (dsv[i].type == kLocalCache)
	  {
	  return dsv[i];
	  } else if (dsv[i].type == kDicomServer)
	  {
	  DicomServerListIter iter = m_serverList.begin();
	  for(;iter != m_serverList.end(); iter++)
	  {
	  if (!strcmp(iter->m_applicationEntityTitle, dsv[i].name))
	  {
	  if (iter->m_transferRate > max)
	  {
						max = iter->m_transferRate;
						ds = dsv[i];
						}
						}
						}
						} else
						{
						;//return kInvalidDataSource;
						}
						}
						return ds;
	*/
}

//----------------------------------------------------------------------------------------
//
const char*	VLIDicomBase::GetModality(const char* iSeriesUID, const char* iStudyUID)
{
	if (iStudyUID && m_studyCache.find(iStudyUID) != m_studyCache.end())
	{
		return m_studyCache[iStudyUID]->m_modality;
	}
	else
	{
		// we should do a Query to find out the modality
		return "";
	}
}

VLIDicomBase::~VLIDicomBase()
{
#ifdef _PROFILE
	ScopeTimer timer("~VLIDicom");
#endif

	if (!m_serverList.empty())
	{
		for(DicomServerListIter iter = m_serverList.begin();iter != m_serverList.end(); iter++)
		{			
			if (iter->m_associationID > 0)
			{
				MC_Close_Association(&iter->m_associationID);
			}
			//	m_serverList.erase(iter);
		}
	}
	
	// -- (Jan 2002)
	tStudyCache::iterator it;
	for ( it = m_studyCache.begin(); it != m_studyCache.end(); )
	{
		delete it->second;
		it = m_studyCache.erase(it);
	}
}

// misc. things related to dicom messages (TC 08/23/01)

static char *gVLIStatusArray1[] = 
{
    "NORMAL_COMPLETION",
	"EMPTY_SERVER_LIST",
	"UNKNOWN_SERVER",
	"CANNOT_OPEN_VOX_FILE",
	"DIRECTORY_DOES_NOT_EXIST",
	"CANNOT_GET_CURRENT_WORKING_DIRECTORY",
	"MISSING_REQUIRED_QUERY_ATTRIBUTE",
	"INVALID_QUERY_LEVEL",
	"NOT_ALL_IMAGES_FROM_SAME_SERIES",
	"INVALID_DATA_SOURCE",
	"INVALID_ARGUMENTS",
	"RETRIEVE_ERROR",
	"QUERY_CACHE_DISABLED",
	"COULD_NOT_ALLOCATE_MEMORY",
	"INVALID_RETRIEVE_LEVEL",
	"RETRIEVE_CANCELLED",
	"FIND_CANCELLED",
	"SORT_FAILED",
	"SOCKET_SELECT_ERROR",
	"CFIND_FAILED",
	"RETRIEVE_PARTIAL_FAILURE",
	"TOO_MANY_RETRIES",
	"INVALID_IMAGE_PIXELS"
};

static char *gVLIStatusArray2[] = 
{
    "ALREADY_REGISTERED",
		"ASSOCIATION_ABORTED",
		"ASSOCIATION_CLOSED",
		"ASSOCIATION_REJECTED",
		"ATTRIBUTE_HAS_VALUES",
		"BUFFER_TOO_SMALL",
		"CALLBACK_CANNOT_COMPLY",
		"CALLBACK_DATA_SIZE_NEGATIVE",
		"CALLBACK_DATA_SIZE_UNEVEN",
		"CALLBACK_PARM_ERROR",
		"CALLBACK_REGISTERED",
		"CANNOT_COMPLY",
		"CANT_ACCESS_PROFILE",
		"CONFIG_INFO_ERROR",
		"CONFIG_INFO_MISSING",
		"DDFILE_ERROR",
		"DOES_NOT_VALIDATE",
		"EMPTY_VALUE",
		"END_OF_DATA",
		"EXT_INFO_UNAVAILABLE",
		"FOUND",
		"FUNCTION_UNAVAILABLE",
		"INCOMPATIBLE_VR",
		"INCOMPATIBLE_VALUE",
		"INVALID_APPLICATION_ID",
		"INVALID_APPLICATION_TITLE",
		"INVALID_ASSOC_ID",
		"INVALID_CHARS_IN_VALUE",
		"INVALID_COMMAND",
		"INVALID_DATA_TYPE",
		"END_OF_LIST",
		"INVALID_GROUP",
		"INVALID_HOST_NAME",
		"INVALID_ITEM_ID",
		"INVALID_LENGTH_FOR_TITLE",
		"INVALID_LENGTH_FOR_VR",
		"INVALID_LICENSE",
		"INVALID_MESSAGE_ID",
		"INVALID_MESSAGE_RECEIVED",
		"INVALID_PARAMETER_NAME",
		"INVALID_PORT_NUMBER",
		"INVALID_PRIVATE_CODE",
		"INVALID_SERVICE_LIST_NAME",
		"INVALID_TAG",
		"INVALID_TRANSFER_SYNTAX",
		"INVALID_VALUE_FOR_VR",
		"INVALID_VALUE_NUMBER",
		"INVALID_VR_CODE",
		"LOG_EMPTY",
		"MESSAGE_EMPTY",
		"MESSAGE_VALIDATES",
		"MISSING_CONFIG_PARM",
		"MSGFILE_ERROR",
		"MUST_BE_POSITIVE",
		"NETWORK_SHUT_DOWN",
		"NO_APPLICATIONS_REGISTERED",
		"NO_CALLBACK",
		"NO_CONDITION_FUNCTION",
		"NO_FILE_SYSTEM",
		"NO_INFO_REGISTERED",
		"NO_LICENSE",
		"NO_MERGE_INI",
		"NO_MORE_ATTRIBUTES",
		"NO_MORE_VALUES",
		"NO_PROFILE",
		"NO_REQUEST_PENDING",
		"NON_SERVICE_ATTRIBUTE",
		"NOT_FOUND",
		"NOT_ONE_OF_ENUMERATED_VALUES",
		"NOT_ONE_OF_DEFINED_TERMS",
		"NULL_POINTER_PARM",
		"NULL_VALUE",
		"PROTOCOL_ERROR",
		"REQUIRED_ATTRIBUTE_MISSING",
		"REQUIRED_DATASET_MISSING",
		"REQUIRED_VALUE_MISSING",
		"STATE_VIOLATION",
		"SYSTEM_CALL_INTERRUPTED",
		"SYSTEM_ERROR",
		"TAG_ALREADY_EXISTS",
		"TEMP_FILE_ERROR",
		"TIMEOUT",
		"TOO_FEW_VALUES",
		"TOO_MANY_BLOCKS",
		"TOO_MANY_VALUES",
		"UNABLE_TO_CHECK_CONDITION",
		"UNACCEPTABLE_SERVICE",
		"UNEXPECTED_EOD",
		"UNKNOWN_ITEM",
		"UNKNOWN_SERVICE",
		"VALUE_MAY_NOT_BE_NULL",
		"VALUE_NOT_ALLOWED",
		"VALUE_OUT_OF_RANGE",
		"VALUE_TOO_LARGE",
		"VR_ALREADY_VALID",
		"LIBRARY_ALREADY_INITIALIZED", 
		"LIBRARY_NOT_INITIALIZED",
		"INVALID_DIRECTORY_RECORD_OFFSET",
		"INVALID_FILE_ID", 
		"INVALID_DICOMDIR_ID", 
		"INVALID_ENTITY_ID",
		"INVALID_MRDR_ID",
		"UNABLE_TO_GET_ITEM_ID",
		"INVALID_PAD",
		"ENTITY_ALREADY_EXISTS",
		"INVALID_LOWER_DIR_RECORD",
		"BAD_DIR_RECORD_TYPE",
		"UNKNOWN_HOST_CONNECTED",
		"INACTIVITY_TIMEOUT",
		"INVALID_SOP_CLASS_UID",
		"INVALID_VERSION",
		"OUT_OF_ORDER_TAG",
		"CONNECTION_FAILED",
		"UNKNOWN_HOST_NAME",
		"INVALID_FILE",
		"NEGOTIATION_ABORTED",
		"INVALID_SR_ID",
		"UNABLE_TO_GET_SR_ID",
 
		// 05/14/2003 --
		"DUPLICATE_NAME",
		"DUPLICATE_SYNTAX",
		"EMPTY_LIST",					// 120
		"MISSING_NAME",
		"INVALID_SERVICE_Name",
		"SERVICE_IN_USE",
		"INVALID_SYNTAX_NAME",
		"SYNTAX_IN_USE",
		"NoContext",
		"OOFFSET_TABLE_TOO_SHORT",
		"MISSING_DELIMITOR",
		"COMPRESSION_FAILURE",
		"END_Of_FRAME",					// 130
		"MustContinueBeforeReading",

		/* Added 05/14/2003 --
		 * new error code  from Merge 
		*/
		"COMPRESSOR_REQUIRED",
		"DECOMPRESSOR_REQUIRED",
		"DATA_AVAILABLE",
		"ZLIB_ERROR",
		"NOT_META_SOP"
};


//----------------------------------------------------------------------------------------
//
 const char* DicomStatusString(int iCode)
{
	static char tt[4][64];
	static int k;
	
	if (iCode < 0) iCode = -iCode;

	if (iCode > 0 && iCode <= sizeof gVLIStatusArray1/sizeof gVLIStatusArray1[0])
		return gVLIStatusArray1[iCode-1];
	else if (iCode >= 4000 && (iCode-4000) < sizeof gVLIStatusArray2/sizeof gVLIStatusArray2[0])
		return gVLIStatusArray2[iCode-4000];
	else
	{
		k = (k+1) & 3;
		sprintf(tt[k],"UnknownErrorCode(%d)",iCode);
		return tt[k];
	}
}

//----------------------------------------------------------------------------------------
//
void VLIDicomBase::PrintOnError(const char* iMsg, int iErrorCode)
{
#ifdef _DEBUG
	LogMessage("status after %s = %s\n",iMsg, DicomStatusString(iErrorCode));
#else
	if (m_printDebugMessages)
	{	
		LogMessage("status after %s = %s\n",iMsg, DicomStatusString(iErrorCode));
	} else if (iErrorCode != kNormalCompletion) 
    { 
		LogMessage("status after %s = %s\n",iMsg, DicomStatusString(iErrorCode));
	}
#endif
}

//----------------------------------------------------------------------------------------
//
void VLIDicomBase::PrintIntOnDebug(const char* iString, int iCode)
{
#ifdef _DEBUG
	LogMessage("%s %d\n", iString, iCode);
#else
	if (m_printDebugMessages)
	{
		LogMessage("%s %d\n", iString, iCode);
	}
#endif 
}

//----------------------------------------------------------------------------------------
//
void VLIDicomBase::PrintOutOnError(const char* iString, int iErrorCode)
{
 
	LogMessage("Error: status after %s = %s\n",iString, DicomStatusString(iErrorCode));
}

 
//----------------------------------------------------------------------------------------
//
void VLIDicomBase::PrintStrOnDebug(const char* iString, int iErrorCode)
{
#ifdef _DEBUG
	printf("Debug: status after %s = %s\n",iString, DicomStatusString(iErrorCode));
#endif 
}

//----------------------------------------------------------------------------------------
//
class AssociationData
{
public:
	AssociationData(const char* iAE, const char* iHost, int iPort, const char* iServiceList, int iAppID)
	{
		strncpy(m_applicationEntityTitle, iAE, sizeof m_applicationEntityTitle);
		strncpy(m_hostname, iHost, sizeof m_hostname);
		strncpy(m_serviceList, iServiceList, sizeof m_serviceList);
		m_port = iPort;
		m_applicationID = iAppID;
	}

	~AssociationData() {}

	int m_applicationID;
	char m_applicationEntityTitle[16 + 1];
	char m_hostname[128];
	int m_port;
	char m_serviceList[128];
};


//----------------------------------------------------------------------------------------
// -- 2004.01.22
// Better to run the member function version
class AsyncOpenAssociation : public iRTVThreadProcess
{
public:	
	AsyncOpenAssociation(const char* iAE, const char* iHost, int iPort, const char* iServiceList, int iAppID)
	{
		SetParam(iAE,iHost,iPort,iServiceList,iAppID);
	}

	void SetParam(const char* iAE, const char* iHost, int iPort, const char* iServiceList, int iAppID)
	{
		nvrstrncpy(m_applicationEntityTitle, iAE, sizeof m_applicationEntityTitle);
		nvrstrncpy(m_hostname, iHost, sizeof m_hostname);
		nvrstrncpy(m_serviceList, iServiceList, sizeof m_serviceList);
		m_port = iPort;
		m_applicationID = iAppID;
		m_associationID = -1;
		m_status = -1;
	}

	int	GetStatus(void)	const			{ return m_status;}
	int	GetAssociationID(void) const	{ return m_associationID;}

	int			Process(void)
	{
		m_status = (PxDicomStatus) MC_Open_Association(
						m_applicationID, 
						&m_associationID, 
						m_applicationEntityTitle,
						& m_port, 
						m_hostname, 
						(char *) m_serviceList);
		return 0;
	}

private:
	int		m_applicationID;
	char	m_applicationEntityTitle[16 + 4];
	char	m_hostname[128];
	int		m_port;
	char	m_serviceList[128];
	int		m_associationID;
	int		m_status;
};

//----------------------------------------------------------------------------------------
//
int VLIAssociationChecker::ThreadFunction(void* data)
{
#ifndef _DEBUG
try
	{
#endif
	AssociationData* adp = (AssociationData*) data;
	if (!adp)
		return kSystemError;

	int m_status = (PxDicomStatus) MC_Open_Association(
		adp->m_applicationID, 
		&m_associationID, 
		adp->m_applicationEntityTitle,
		&adp->m_port, 
		adp->m_hostname, 
		(char *)adp->m_serviceList);

	delete adp;

#ifndef _DEBUG 
	}
    catch (...)
	{
		m_status = kNegotiationAborted;
	}
#endif
	return m_status;
}


// This is to automatically close/open association on 
// Query/Retrieve (-- 12/17/2001)
VLIAssociationChecker::VLIAssociationChecker(DicomServer* iServer, VLIDicomBase* iInstance, const char* iWhere, const char* serviceList)
{ 

	m_openAssoc = 0;
	m_associationID = -1;

	if (!iServer || !iInstance || ! serviceList)
	{
		m_status = kInvalidArguments;
		return;
	}

	m_server = iServer;
	m_owner = iInstance;
	m_where = iWhere;		

	if (iServer && iServer->m_associationID < 0 && !iInstance->IsCancelled())
	{		
#if 1
		m_openAssoc = new AsyncOpenAssociation( iServer->m_applicationEntityTitle, iServer->m_hostname, iServer->m_port, serviceList, VLIDicomApplicationID);
	
		int s = RTVOneShotThreadManager::theManager().AddRequest(m_openAssoc,0);
		assert(s >= 0);
		
		int C = 0;
		for (int i = 0; i < 300 && !m_openAssoc->HasTerminated() && !iInstance->IsCancelled();++i)
		{
			Sleep(100);
			
			if (iInstance->IsCancelled())
			{
				iInstance->ResetCancelFlag();
				m_status = kNegotiationAborted;
				m_owner->LogMessage("(%s) Association request cancelled. Status=%d(%s)\n", iWhere ? iWhere:"?",m_status, DicomStatusString(m_status));
				if (!m_openAssoc->HasTerminated())
				{
					RTVOneShotThreadManager::theManager().SetDeleteWhenDone(m_openAssoc,1);
					m_openAssoc = 0;  // disown this pointer
				}
				else
				{
					m_associationID = m_openAssoc->GetAssociationID();
					m_status = (PxDicomStatus)m_openAssoc->GetStatus();
					RTVOneShotThreadManager::theManager().RemoveRequest(m_openAssoc,100);
				}
				return;
			}

			// -- 2005.09.29
			// show waiting progress for dead servers
			if ((i%13)==0 && !iInstance->IsCancelled())
			{
				C = i/13;
				m_owner->Progress(15-C,C,0,0,kVLIQuery);
			}
		}
		
		if (m_openAssoc->HasTerminated())
		{
			m_associationID = m_openAssoc->GetAssociationID();
			m_status = (PxDicomStatus)m_openAssoc->GetStatus();
			RTVOneShotThreadManager::theManager().RemoveRequest(m_openAssoc,100);
		}
		else 
		{
			m_status = iInstance->IsCancelled() ? m_status = kNegotiationAborted:kTimeout;
			iInstance->ResetCancelFlag();
			RTVOneShotThreadManager::theManager().SetDeleteWhenDone(m_openAssoc,1);
			m_openAssoc = 0; // disown this pointer
			return;
		}
#else
		AssociationData* assocData = 
			new AssociationData(iServer->m_applicationEntityTitle, iServer->m_hostname, iServer->m_port, serviceList, VLIDicomApplicationID);

		iRTVThreadRunClassFunction connectProcess(this, assocData);
		iRTVThread connectThread(&connectProcess);

		int processStatus = connectProcess.GetProcessStatus();
		while(processStatus != iRTVThreadProcess::kProcessTerminated && !iInstance->IsCancelled())
		{
			if (iInstance->IsCancelled())
			{
				iInstance->ResetCancelFlag();
				m_status = kNegotiationAborted;
				m_owner->LogMessage("(%s) Association request cancelled. Status=%d(%s)\n", iWhere ? iWhere:"?",m_status, DicomStatusString(m_status));
				return;
			}
			processStatus = connectProcess.GetProcessStatus();
			Sleep(100);
		}
#endif

		iServer->m_associationID = m_associationID;

		int t = 1;
		AssocInfo ai;
		m_status = (PxDicomStatus) MC_Get_Association_Info(iServer->m_associationID, &ai);
		setsockopt(ai.Tcp_socket,IPPROTO_TCP, TCP_NODELAY, (const char*)&t, sizeof(t));

		if (m_status != kNormalCompletion || iServer->m_associationID < 0)
		{
			m_owner->LogMessage("(%s) Can't open association. Status=%d(%s)\n", iWhere ? iWhere:"?",m_status, DicomStatusString(m_status));
		}
	}
	else
	{
		// it would be nice to be able to verify the association
		m_status = kNormalCompletion;
	}
}
/*
// This is to automatically close/open association on 
// Query/Retrieve (-- 12/17/2001)
VLIAssociationChecker::VLIAssociationChecker(DicomServer* iServer, VLIDicomBase* iInstance, const char* iWhere, const char* serviceList)
{
#ifdef _PROFILE
	ScopeTimer timer("VLIAssociationChecker");
#endif
	m_server = iServer;
	m_owner = iInstance;
	m_where = iWhere;		
	
	if (iServer && iServer->m_associationID < 0 && !iInstance->IsCancelled())
	{
		int tries = 0, delta[] = { 6, 3, 3, 6, 10};
		//	-- - 05/14/02 - Revised open association to loop 30 times with 1 sec sleep
		//		Also, checking m_cancel.
		do
		{
			//	cancel requests in a reasonable time
			MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT,    delta[tries]);
			MC_Set_Int_Config_Value(CONNECT_TIMEOUT,		delta[tries]);

			m_status = (PxDicomStatus) MC_Open_Association(VLIDicomApplicationID, 
				&iServer->m_associationID, 
				iServer->m_applicationEntityTitle,
				&iServer->m_port, 
				iServer->m_hostname, 
				(char *)serviceList);
		} while (!iInstance->IsCancelled() && tries++ < 5 && (m_status != kNormalCompletion || iServer->m_associationID < 0));

		MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, 10);
		MC_Set_Int_Config_Value(CONNECT_TIMEOUT, 10);

		if (!iInstance->IsCancelled())
		{
			int t = 1;
			AssocInfo ai;
			m_status = (PxDicomStatus) MC_Get_Association_Info(iServer->m_associationID, &ai);
			setsockopt(ai.Tcp_socket,IPPROTO_TCP, TCP_NODELAY, (const char*)&t, sizeof(t));
		} 
		else
		{
			iInstance->ResetCancelFlag();
			m_status = kNegotiationAborted;
			m_owner->LogMessage("(%s) Association request cancelled. Status=%d(%s)\n", iWhere ? iWhere:"?",m_status, DicomStatusString(m_status));
			return;
		}

		if (m_status != kNormalCompletion || iServer->m_associationID < 0)
		{
			m_owner->LogMessage("(%s) Can't open association. Status=%d(%s)\n", iWhere ? iWhere:"?",m_status, DicomStatusString(m_status));
		}
	}
	else
	{
		// it would be nice to be able to verify the association
		m_status = kNormalCompletion;
	}
}
*/
//----------------------------------------------------------------------------------------
//
PxDicomStatus VLIAssociationChecker::GetStatus(void) const { return m_status;}


// what status is considered ok so we can reset the assocID to invalid
inline bool OKCloseStatus(PxDicomStatus status)
{
	
	//	return status == kNormalCompletion || status == kInvalidAssocId;
#ifdef _DEBUG
	return status == kNormalCompletion;
#else
	return status == kNormalCompletion || status == kAssociationAborted ||  status == kInvalidAssocId;
#endif
	
}

//----------------------------------------------------------------------------------------
//
VLIAssociationChecker::~VLIAssociationChecker(void)
{
#ifdef _PROFILE
	ScopeTimer timer("~VLIAssociationChecker");
#endif
	if (m_server && VLIDicom::m_reopenAssociationEachTime && m_server->m_associationID > 0)
	{
		PxDicomStatus status = (PxDicomStatus)MC_Close_Association(&m_server->m_associationID);
		
		if (!OKCloseStatus(status))
		{
			m_owner->LogMessage("(%s) CloseAssoc() failed. Status=%d (%s)\n",m_where ? m_where:"?",status, DicomStatusString(status));
		}
		m_server->m_associationID = -1;

		//	-- - 11/17/02 - Added this so server list "knows" this assoc is closed.
		m_owner->UpdateServer(*m_server);
	}

	// check if we still own the pointer, that means the m_openAssoc
	// has already finished.
	if (m_openAssoc)
	{
		assert(m_openAssoc->HasTerminated());
		RTVOneShotThreadManager::theManager().RemoveRequest(m_openAssoc,20);
		delete m_openAssoc, m_openAssoc = 0;
	}
}

//----------------------------------------------------------------------
// 
//	Override iRTVBase method - so we can add connection info in front
//
void VLIDicomBase::LogMessage(const char *fmt, ...)
{
	//	Prepend (AssocID, MsgID) to log message
	char mbuf[255];
	//	Construct a message prefix that includes connection info
	sprintf(mbuf, "** DICOM: (%d)-[%s,%s,%d] ", m_currentServer.m_associationID, 
		m_currentServer.m_applicationEntityTitle, m_currentServer.m_hostname, m_currentServer.m_port);

	try
	{
		va_list args;
		va_start(args, fmt);
		gLogger.WriteLogMessage(fmt, args, mbuf);
	}
	catch (...)
	{
		gLogger.LogMessage("** ERROR in log format detected (%s)\n", fmt);
	}
}

//----------------------------------------------------------------------
// 
//	Override iRTVBase method - so we can add connection info in front
//
void VLIDicomBase::LogMessage(int iLevel, const char* fmt, ...)
{
	if (iLevel > gLogger.GetLogLevel())
		return;

	//	Prepend (AssocID, MsgID) to log message
	char mbuf[255];
	//	Construct a message prefix that includes connection info
	sprintf(mbuf, "** DICOM: (%d)-[%s,%s,%d] ", m_currentServer.m_associationID, 
		m_currentServer.m_applicationEntityTitle, m_currentServer.m_hostname, m_currentServer.m_port);

	try
	{
		va_list args;
		va_start(args, fmt);
		gLogger.WriteLogMessage(fmt, args, mbuf);
	}
	catch (...)
	{
		gLogger.LogMessage("** ERROR in log format detected (%s)\n", fmt);
	}
}

//----------------------------------------------------------------------------------------
//
CFindSCU::CFindSCU()
{
	m_status = kFindAborted;
	m_queryLevel[0] = '\0';
	m_isCFindCancelResquestSent = false;
}

CFindSCU::~CFindSCU()
{
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::QueryListOfPatients(DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply)
{
 #ifdef _PROFILE
	ScopeTimer timer("QueryListOfPatients");
#endif
	PxDicomStatus		status;
	CPxDicomMessage		message;
	
    //	Set up the query to search for this patient, 
    //		returning patient ID and birth date
    status = (PxDicomStatus) message.SetValue(kVLIPatientsName,"*"); 
	PrintOnError("Set Patient Name", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
    
    status = (PxDicomStatus) message.SetValueToNull(kVLIPatientId); 
	PrintOnError("Set Return Patient ID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
    
    status = (PxDicomStatus) message.SetValueToNull(kVLIPatientsBirthDate); 
	PrintOnError("Set Return Patient Birth" , status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    //	Study Level tags
    status = (PxDicomStatus) message.SetValueToNull(kVLIStudyInstanceUid); 
	PrintOnError("Set Return StudyInstanceUID", status); 
    if (status != kNormalCompletion)
    {
        return status;
    }
    
    status = (PxDicomStatus) message.SetValueToNull(kVLIStudyDate); 
	PrintOnError("Set Return StudyDate", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIStudyTime); 
	PrintOnError("Set Return StudyTime", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIAccessionNumber); 
	PrintOnError("Set Return AccessionNumber", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIStudyId); 
	PrintOnError("Set Return StudyID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	//	Execute the query
	if(SetCurrentServer(iSource.m_server) == kNormalCompletion)
		status = (PxDicomStatus) Query(message, oReply); 
	else 
		status = (PxDicomStatus) QueryAllServers(message, oReply); 
		
	PrintOnError("Query", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::QueryFindDataSources(std::vector<DicomDataSource>& oReply, 
											  const char* iStudyInstanceUID,
											  const char* iSeriesInstanceUID,
											  std::string& oModality, bool queryOne)
{
#ifdef _PROFILE
	ScopeTimer timer("QueryFindDataSources");
#endif
	PxDicomStatus		status;
	CPxDicomMessage		message;
	std::vector<CPxDicomMessage*> mReplys;
	
	if (!message.CheckValid())
	{
		return message.GetStatus();
	}
	
    status = (PxDicomStatus) MC_Set_Service_Command(message.GetID(), "STUDY_ROOT_QR_FIND", C_FIND_RQ); PrintOnError("Set Model", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) MC_Set_Value_From_String (message.GetID(), MC_ATT_QUERY_RETRIEVE_LEVEL, "SERIES"); PrintOnError("Set Level", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	//	Set up the query to search for all series from this study,
	//	    returning modality, series number and Series Instance UID
    status = (PxDicomStatus) message.SetValue(kVLIStudyInstanceUid,iStudyInstanceUID); PrintOnError("Set StudyInstUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValue(kVLISeriesInstanceUid,iSeriesInstanceUID); PrintOnError("Set SeriesInstUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIModality); PrintOnError("Set Return Modality", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
    
    status = (PxDicomStatus) message.SetValueToNull(kVLISeriesNumber); PrintOnError("Set Return SeriesNumber", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
    
	
	//	Execute the query
	//	Changed this to CHECK_NOOP 12/12/01 RL - Otherwise, if one of the servers had a problem, the results
	//	From the other servers won't populate oReply.
	//	Execute the query

	status = (PxDicomStatus) QueryAllServers(message, mReplys, queryOne, false); PrintOnError("QueryAllServers", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	//PxDicomStatus GetValue(unsigned long iTag, char* oValue, int iSize=kMaxPN);

	char modality[8];
    message.GetValue(kVLIModality, modality, 8);
	oModality = modality;

	for(int i=0;i<mReplys.size();i++)
	{
		oReply.push_back(mReplys[i]->GetDataSource());
		delete mReplys[i];
	}
	mReplys.clear();
	
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::QueryListOfStudies(DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply, const char* iPatientID)
{
#ifdef _PROFILE
	ScopeTimer timer("QueryListOfStudies");
#endif
	PxDicomStatus		status;
	CPxDicomMessage		message;
	
	//	Set up the query to search for all studies from this patient,
	//	    returning study instance UID and study date
    status = (PxDicomStatus) message.SetValue(kVLIPatientId,iPatientID); 
	PrintOnError("Set Patient ID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIStudyInstanceUid); 
	PrintOnError("Set Return StudyInstanceUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIStudyDate); 
	PrintOnError("Set Return StudyDate", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	//	Execute the query
	if(SetCurrentServer(iSource.m_server) == kNormalCompletion)
		status = (PxDicomStatus) Query(message, oReply); 
	else 
		status = (PxDicomStatus) QueryAllServers(message, oReply); 
		
	PrintOnError("Query", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::QueryListOfSeries(DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply, 
										   const char* iStudyUID)
{
#ifdef _PROFILE
	ScopeTimer timer("QueryListOfSeries");
#endif
	PxDicomStatus		status;
	CPxDicomMessage		message;
	
	//	Set up the query to search for all series from this study,
	//	    returning modality, series number and Series Instance UID
    status = (PxDicomStatus) message.SetValue(kVLIStudyInstanceUid,iStudyUID); 
	PrintOnError("Set StudyInstUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIModality); 
	PrintOnError("Set Return Modality", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLISeriesNumber); 
	PrintOnError("Set Return SeriesNumber", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLINumberOfSeriesRelatedInstances); 
	PrintOnError("Set Return NumberImages", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLISeriesInstanceUid); 
	PrintOnError("Set Return SeriesInstUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	//	Execute the query
	if(SetCurrentServer(iSource.m_server) == kNormalCompletion)
		status = (PxDicomStatus) Query(message, oReply); 
	else 
		status = (PxDicomStatus) QueryAllServers(message, oReply, true); 
		
	PrintOnError("Query", status);
    if (status != kNormalCompletion)
    {
        return status;
    }

	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::QueryListOfImages(DicomDataSource& iSource,
										   std::vector<CPxDicomMessage*>& oReply, 
										   const char* iSeriesUID, 
										   const char* iStudyUID)
{
#ifdef _PROFILE
	ScopeTimer timer("QueryListOfImages");
#endif
	PxDicomStatus		status;
	CPxDicomMessage		message;
	
	if (iStudyUID == NULL)
	{
		status = (PxDicomStatus) message.SetValueToNull(kVLIStudyInstanceUid); 
		PrintOnError("Set StudyInstUID", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		
	} else
	{
		status = (PxDicomStatus) message.SetValue(kVLIStudyInstanceUid,iStudyUID); 
		PrintOnError("Set StudyInstUID", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		
	}
	
    status = (PxDicomStatus) message.SetValue(kVLISeriesInstanceUid,iSeriesUID); 
	PrintOnError("Set SeriesInstUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLIImageNumber); 
	PrintOnError("Set Return Image Number", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus) message.SetValueToNull(kVLISopInstanceUid); 
	PrintOnError("Set Return SOPInstUID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	//	Execute the query
	if(SetCurrentServer(iSource.m_server) == kNormalCompletion)
		status = (PxDicomStatus) Query(message, oReply); 
	else 
		status = (PxDicomStatus) QueryAllServers(message, oReply, true); 
		
	PrintOnError("Query", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	return kNormalCompletion;
}

//----------------------------------------------------------------------
// 
PxDicomStatus CFindSCU::QuerySOPByInstanceNumber (DicomDataSource& iSource,
												   int iInstanceNumber, 
													const char* iSeriesInstanceUID, 
													const char* iStudyInstanceUID, 
													std::string& oSOPUID)
{
	CPxDicomMessage	queryMsg;
	std::vector<CPxDicomMessage*> replies;
	std::string modality;
	oSOPUID = "";

	//	Bad arguments
	if (!iStudyInstanceUID || !iSeriesInstanceUID || iInstanceNumber < 0)
	{
		LogMessage("ERROR: QuerySOPByInstanceNumber() - invalid arguments\n");
		return kFindAborted;
	}

	//	Compose query message
	queryMsg.SetValue(kVLIStudyInstanceUid,iStudyInstanceUID);
	queryMsg.SetValue(kVLISeriesInstanceUid,iSeriesInstanceUID);
	queryMsg.SetValue(kVLIImageNumber,iInstanceNumber);
    queryMsg.SetValueToNull(kVLISopInstanceUid); 
	
	//	Execute the query
	if(SetCurrentServer(iSource.m_server) == kNormalCompletion)
		m_status = (PxDicomStatus) Query(queryMsg, replies); 
	else 
		m_status = (PxDicomStatus) QueryAllServers(queryMsg, replies, true); 
		
	if (m_status != kNormalCompletion)
    {
		LogMessage(kInfo,"INFO: QuerySOPByInstanceNumber() - Query failed - returned %d\n", m_status);
        return m_status;
    }

	char uid[65];
	if (replies.size() <= 0)
	{
		LogMessage("ERROR: QuerySOPByInstanceNumber() - Query returned no results\n");
		return kFindAborted;
	}

	m_status = replies[0]->GetValue(kVLISopInstanceUid, uid, sizeof uid);
	if (m_status != kNormalCompletion)
	{
		LogMessage("ERROR: QuerySOPByInstanceNumber() - GetValue failed - returned %d\n", m_status);
		return m_status;
	}

	oSOPUID = uid;
	return m_status;
}

// Search iSource for matches to constraints set in iQuery.  If iSource is not given, all known dataSources are searched.
PxDicomStatus CFindSCU::Query(CPxDicomMessage& iQuery, std::vector<CPxDicomMessage*>& oReply)
{
	//	2005.12.09 - -- - Fix #6224 - Tokushima Univ InfoCom DICOM server tries to match 100 studies
	int num = 0;
	iQuery.GetValue(kVLINumberOfPatientRelatedStudies, &num);
	if (num == 100)
		iQuery.SetValueNull(kVLINumberOfPatientRelatedStudies);

	int	wasCancelled = 0;

	if(m_cancel)
	{
		m_status = kFindAborted;
		return m_status;
	}

	//	Reset cancel flag
	m_cancel = 0;
	m_isCFindCancelResquestSent = false;

	int mID = iQuery.GetID();
	if (mID == kInvalidMessageID)
	{
		m_status = kInvalidMessageId;
		return m_status;
	}

	//	Set Query Level
	m_queryLevel[0] = '\0';
	m_status = SetQueryLevel(iQuery);
	if(m_status != kNormalCompletion)
	{
		PrintOnError("SetQueryLevel", m_status);
		return m_status;
	}


#if 0
	m_status = SetCurrentServer(iSource.m_server);
	if(m_status != kNormalCompletion)
	{
		PrintOnError("Set Current Server", m_status);
		return m_status;
	}
#endif

	//	Set progress bar to the beginning
	Progress(-1,0,0,0,kVLIQuery);

	//	Make sure we have a valid association (TCZ 12/17/2001)
	VLIAssociationChecker check(&m_currentServer, this, "Query");
	if (check.OK())
	{

		//	Send the C-FIND request
		if (PrepareAndSendCFindReqMSG(mID) != kNormalCompletion)
		{
			//	Failed - log an error, and skip the responses - move to the next server
			PrintOnError("SendRequest in VLIDicom::Query --- PrepareAndSendCFindReqMSG()", m_status);
		} 
		else if (!m_cancel)
		{		
			//	Get the responses from this server
			if ((m_status = GetAndParseCFindResponseMessages(mID, oReply)) != kNormalCompletion)
			{
				PrintOnError("SendRequest in VLIDicom::Query --- GetAndParseCFindResponseMessages()", m_status);
			}
		}
	}

	//	This is necessary, even though we have a VLIAssociationChecker, since AdvanceServer changes m_currentServer
	//		i.e. if we don't do this here, this assoc won't be closed
	MC_Close_Association(&m_currentServer.m_associationID);

	wasCancelled = m_cancel;
	if (m_cancel)
		m_cancel = 0;


	//	Make sure study cache has datasource of the results that remain after removing duplicates
	if ( strcmp(m_queryLevel, "STUDY") == 0 )
	{
		std::vector<CPxDicomMessage*>::iterator replyIterator;
		tStudyCache::iterator cacheIterator;
		char studyUID[68];

		for(replyIterator = oReply.begin(); replyIterator != oReply.end(); replyIterator++)
		{
			m_status = (PxDicomStatus) MC_Get_Value_To_String ((*replyIterator)->GetID(), MC_ATT_STUDY_INSTANCE_UID,  sizeof studyUID,studyUID);
			const DicomDataSource &ds = (*replyIterator)->GetDataSource();

			//	It's already in there - just update datasource
			if ((cacheIterator = m_studyCache.find(studyUID)) != m_studyCache.end())
			{
				 cacheIterator->second->SetDataSource(ds);
			} else
			{
				LogMessage("Warning: no study cache entry for StudyUID %s\n", studyUID);
				m_studyCache[studyUID] = new VLICachedInfo((*replyIterator)->GetDataSource());
			}
		}
	}

	Progress(0, oReply.size(), 0,0, kVLIQuery);
	m_status = !wasCancelled ? kNormalCompletion:kFindAborted;
	return m_status;
}
 
// Search iSource for matches to constraints set in iQuery.  If iSource is not given, all known dataSources are searched.
PxDicomStatus CFindSCU::QueryAllServers(CPxDicomMessage& iQuery, std::vector<CPxDicomMessage*>& oReply, bool queryOne, bool removeDups)
{
	DicomServer			initialServer;
 	int					wasCancelled = 0;

	if(m_cancel)
	{
		m_status = kFindAborted;
		return m_status;
	}

	//	Reset cancel flag
	m_cancel = 0;
	m_isCFindCancelResquestSent = false;

	int mID = iQuery.GetID();
	if (mID == kInvalidMessageID)
	{
		m_status = kInvalidMessageId;
		return m_status;
	}

	//	Set Query Level
	m_queryLevel[0] = '\0';
	m_status = SetQueryLevel(iQuery);
	if(m_status != kNormalCompletion)
	{
		PrintOnError("SetQueryLevel", m_status);
		return m_status;
	}

	//	Does the Study Cache know which Server to ask?
	tStudyCache::iterator cacheIterator;
	char studyUID[68];
	if ( queryOne ) // && (!strcmp(m_queryLevel, "SERIES") || !strcmp(m_queryLevel, "IMAGE")))
	{
		m_status = (PxDicomStatus) MC_Get_Value_To_String (mID, MC_ATT_STUDY_INSTANCE_UID, sizeof(studyUID), studyUID);
		if ((cacheIterator = m_studyCache.find(studyUID)) != m_studyCache.end())
		{
			m_status = SetCurrentServer(cacheIterator->second->GetDataSource().m_server); 
			PrintOnError("Set Current Server", m_status);
			if (m_status == kNormalCompletion)
				return Query(iQuery, oReply);
			else
				LogMessage(kWarning, "WARNING: SetCurrentServer failed on cached server: %s\n", 
				cacheIterator->second->GetDataSource().m_server.m_applicationEntityTitle);
		} 
		else
		{
			LogMessage(kDebug,"Warning: no study cache entry for StudyUID %s\n", studyUID);
		}
	}

	DicomServer startServer = m_currentServer;
	do
	{
		Query(iQuery, oReply);
		if(queryOne && oReply.size() > 0)
			break;

		AdvanceServer();

	}while (!m_cancel && !(m_currentServer == startServer));

	//	If we know which server, talk only to that one.  Otherwise, need to search them all
	wasCancelled = m_cancel;
	if (m_cancel)
		m_cancel = 0;

	// We have a problem in std::sort as of 07/30/01. Can't figure it out right
	// now. Put a bad hack here so dicom.dll won't go away killing the server (TCZ 07/30/01)
	// XXXX TODO TODO TO DO

	try
	{
		if (!queryOne && removeDups && oReply.size())
		{
			m_status = (PxDicomStatus)RemoveDuplicateQueryResults(iQuery, oReply);
			if (m_status != MC_NORMAL_COMPLETION)
			{
				PrintOnError("Remove Duplicates in VLIDicom::Query",m_status); 
				return m_status;
			}
		}
	}
	catch (...)
	{
		LogMessage("RemoveDuplicate failed.\n");
	}

	//	Make sure study cache has datasource of the results that remain after removing duplicates
	if (!strcmp(m_queryLevel, "STUDY"))
	{
		std::vector<CPxDicomMessage*>::iterator replyIterator;
		tStudyCache::iterator cacheIterator;
		char studyUID[68];

	
		//	if (status != kNormalCompletion)
		//	{
		//		return status;
		//	}

		for(replyIterator = oReply.begin(); replyIterator != oReply.end(); replyIterator++)
		{
			m_status = (PxDicomStatus) MC_Get_Value_To_String ((*replyIterator)->GetID(), MC_ATT_STUDY_INSTANCE_UID,  sizeof studyUID,studyUID);
			const DicomDataSource &ds = (*replyIterator)->GetDataSource();

			//	It's already in there - just update datasource
			if ((cacheIterator = m_studyCache.find(studyUID)) != m_studyCache.end())
			{
				 cacheIterator->second->SetDataSource(ds);
			} else
			{
				LogMessage("Warning: no study cache entry for StudyUID %s\n", studyUID);
				m_studyCache[studyUID] = new VLICachedInfo((*replyIterator)->GetDataSource());
			}
		}
	}

	Progress(0, oReply.size(), 0,0, kVLIQuery);
	m_status = !wasCancelled ? kNormalCompletion:kFindAborted;
	return m_status;
}
 


//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::GetAndParseCFindResponseMessages(int iMsgID, std::vector<CPxDicomMessage*>& oReply)
{
	char				studyUID[65];
	PxDicomStatus		status;
	int					responseID;
	unsigned int		responseStatus;
 	char*				serviceName;
 	MC_COMMAND			command;
	DicomDataSource		dataSource;
	int					nResults = 0;

	time_t startTime, currentTime;
	time(&startTime);

	int ntotal = 100;

	for(;;)
	{
		if (m_maxDICOMQueryResults && nResults >= m_maxDICOMQueryResults)
		{
			// -- 2007.01.09 This should help debugging a lot.
			LogMessage("VLIDicom::Query: exceeding the limit of %d records\n",m_maxDICOMQueryResults);
			break;
		}
			
		//	Read the next response message - timeout after 3 seconds
		status = (PxDicomStatus)MC_Read_Message (m_currentServer.m_associationID, 1, &responseID, &serviceName, &command);
		if (status != kTimeout && status != kNormalCompletion)
		{
			PrintOnError("Read CFind Response Message in VLIDicom::Query()", status);
			return status;
		}

		time(&currentTime);

		//	Timeout on this read - check for total timeout
		if(!m_cancel && status == kTimeout)
		{
			//	total time for querying *this* server exceeded maximum allowed
			if((currentTime-startTime) > m_timeoutOnQuery)
			{
				LogMessage("VLIDicom::Query - TimeoutOnQuery of %d seconds expired", m_timeoutOnQuery);
				MC_Free_Message(&responseID);
				return kCFindFailed;
			}
			else
			{
				continue; 
			}
		}
		else
		{
			//	Something was received (or cancel) - reset timer
			startTime = currentTime;
		}

		//	If the CFind is to be canceled, first we send a C-CANCEL-FIND request, and proceed as usual
		//	If we don't get a C-CANCEL-FIND Response message, the server must not support C-FIND Cancel, so we abort anyway.
		if (m_cancel)
		{
			//	Reset cancel flag
			m_cancel = 0;

			if (!m_isCFindCancelResquestSent)
			{
				//	Send out the cancel request, or abort if we already did
				status = DoCFindCancellation(iMsgID, responseID);
				if (status != kNormalCompletion)
				{
					PrintOnError("VLIDicom::Query - DoCFindCancellation()", status);
					MC_Free_Message(&responseID);
					return status;
				}
			} else
			{
				// Server doesn't seem to support C-CANCEL-FIND - we'll just close the association and abandon the query
				PrintIntOnDebug ("CFind Cancel response NOT received after 3 messages on association =", m_currentServer.m_associationID); 
				MC_Free_Message(&responseID);
				return kFindAborted;
			}
		} 

		//	We really got something
		status = (PxDicomStatus) MC_Get_Value_To_UInt (responseID, MC_ATT_STATUS, &responseStatus);
		if (status != kNormalCompletion)
		{
			PrintOnError("Get CFind Response Status in VLIDicom::Query()", status);
			MC_Free_Message(&responseID);
			return status;
		}

		CPxDicomMessage* pMyReply;
		switch (responseStatus)
		{
			case C_FIND_SUCCESS:
				PrintIntOnDebug("VLIDicom::Query --- C_FIND_SUCCESS received on associationID =", m_currentServer.m_associationID);
				MC_Free_Message(&responseID);
				return kNormalCompletion;

			case C_FIND_CANCEL_REQUEST_RECEIVED:
				PrintIntOnDebug("VLIDicom::Query --- C_FIND_CANCEL_REQUEST_RECEIVED on associationID =", m_currentServer.m_associationID);
				MC_Free_Message(&responseID);
				return kFindAborted;

			case C_FIND_FAILURE_REFUSED_NO_RESOURCES:
				LogMessage("ERROR: C_FIND_FAILURE_REFUSED_NO_RESOURCES with associationID = %d\n",  m_currentServer.m_associationID);
				MC_Free_Message(&responseID);
				return kCFindFailed;	 
			case C_FIND_FAILURE_INVALID_DATASET:
				LogMessage("ERROR: C_FIND_FAILURE_INVALID_DATASET with associationID = %d\n",  m_currentServer.m_associationID);
				MC_Free_Message(&responseID);
				return kCFindFailed;	 
			case C_FIND_FAILURE_UNABLE_TO_PROCESS:
				LogMessage("ERROR: C_FIND_FAILURE_UNABLE_TO_PROCESS with associationID = %d\n",  m_currentServer.m_associationID);
				MC_Free_Message(&responseID);
				return kCFindFailed;	 
		
			case C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT:	// We got a message - so we
			case C_FIND_PENDING:
				// add it to the reply std::vector
				PrintIntOnDebug("VLIDicom::Query --- C_FIND_PENDING received on associationID =", m_currentServer.m_associationID);
				pMyReply = new CPxDicomMessage;
				if (!pMyReply)
				{
					MC_Free_Message(&responseID);
					return kBufferTooSmall;
				}

				dataSource.type = kDicomServer;
				dataSource.m_server = m_currentServer;
				pMyReply->SetID(responseID);
				pMyReply->SetDataSource(&dataSource);
				
				// added Jan 2001 by --: we cache the datasource and modality
				if (strcasecmp(m_queryLevel,"STUDY") == 0)
				{
					if (pMyReply->IsSet(kVLIStudyInstanceUid))
					{
						if (pMyReply->GetValue(kVLIStudyInstanceUid, studyUID, sizeof studyUID) == kNormalCompletion)
						{
							tStudyCache::iterator it;
							if ((it = m_studyCache.find(studyUID)) != m_studyCache.end())
							{
								assert(it->second);
								if (it->second)
									it->second->SetDataSource(dataSource);
							}
							else
							{
								m_studyCache[studyUID] = new VLICachedInfo(dataSource);
							}
						}
					}		
				}
				else if (strcasecmp(m_queryLevel,"SERIES") == 0)
				{
					studyUID[0]  = '\0';
					if (pMyReply->IsSet(kVLIStudyInstanceUid))
					{	
						pMyReply->GetValue(kVLIStudyInstanceUid, studyUID, sizeof studyUID);
					}
					
					if (studyUID[0])
					{	
						tStudyCache::iterator it;
						if (( it = m_studyCache.find(studyUID)) != m_studyCache.end())
						{
							char modality[8];
							if (pMyReply->GetValue(kVLIModality, modality, sizeof modality) == kNormalCompletion)
							{
								it->second->SetModality(modality);
								Message("Cached: %s\n", modality);
							}
						}
					}
				}

				++nResults;
				oReply.push_back(pMyReply);

				// tcz 2005.09.29
				// try to make the scroll bar look better
				if (oReply.size() > ntotal + 2)
					ntotal += 50;		
				//	Update progress bar - since we don't know how many total responses, it's just a running count
			//	this->Progress(-1, oReply.size(), 0, 0, kVLIQuery);
			 	this->Progress(ntotal - oReply.size(), ntotal, 0, 0, kVLIQuery);
				break;
				
				//	NOTE: this should be the only case where we return to the top of the loop.  Otherwise, we're out of the function

			default:
				PrintIntOnDebug("VLIDicom::Query --- default CFIND response received with associationID =", m_currentServer.m_associationID);
				char msg[68] = ""; // -- 01/25/02 Need to initialize this as Get_Value_To_String can fail
				status = (PxDicomStatus)MC_Get_Value_To_String(responseID, MC_ATT_ERROR_COMMENT, sizeof msg, msg);			 
				LogMessage("ERROR: Query - Received bad response status: %08x %s\n", responseStatus, msg);
				MC_Free_Message(&responseID);
				return kCFindFailed;
		}
	}
	 
	return kNormalCompletion;
}
 
//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::SetQueryLevel(CPxDicomMessage& iQuery)
{
#ifdef _PROFILE
	ScopeTimer timer("SetQueryLevel");
#endif
	PxDicomStatus status;
	int iMessageID = iQuery.GetID();
	
	if (iQuery.IsSet(kVLISopInstanceUid) ||
		iQuery.IsSet(kVLIImageNumber))
	{
		status = (PxDicomStatus) MC_Set_Service_Command(iMessageID, "STUDY_ROOT_QR_FIND", C_FIND_RQ); 
		PrintOnError("Set Model", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		
		status = (PxDicomStatus) MC_Set_Value_From_String (iMessageID, MC_ATT_QUERY_RETRIEVE_LEVEL, "IMAGE"); 
		PrintOnError("Set Level", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		
		strcpy(m_queryLevel,"IMAGE");
	} else if (iQuery.IsSet(kVLISeriesInstanceUid) ||
		iQuery.IsSet(kVLIModality) ||
		iQuery.IsSet(kVLISeriesNumber))
	{
		status = (PxDicomStatus) MC_Set_Service_Command(iMessageID, "STUDY_ROOT_QR_FIND", C_FIND_RQ); 
		PrintOnError("Set Model", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		
		status = (PxDicomStatus) MC_Set_Value_From_String (iMessageID, MC_ATT_QUERY_RETRIEVE_LEVEL, "SERIES"); 
		PrintOnError("Set Level", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		strcpy(m_queryLevel,"SERIES");
	} else if (iQuery.IsSet(kVLIStudyInstanceUid) ||
		iQuery.IsSet(kVLIAccessionNumber) ||
		iQuery.IsSet(kVLIStudyId) ||
		iQuery.IsSet(kVLIStudyDate) ||
		iQuery.IsSet(kVLIPatientId) ||
		iQuery.IsSet(kVLIPatientsName))
	{
		status = (PxDicomStatus) MC_Set_Service_Command(iMessageID, "STUDY_ROOT_QR_FIND", C_FIND_RQ); 
		PrintOnError("Set Model", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		
		status = (PxDicomStatus) MC_Set_Value_From_String (iMessageID, MC_ATT_QUERY_RETRIEVE_LEVEL, "STUDY"); 
		PrintOnError("Set Level", status);
		if (status != kNormalCompletion)
		{
			return status;
		}
		strcpy(m_queryLevel,"STUDY");
	}	
	else
	{
		return kMissingRequiredQueryAttribute;
	}
	
	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::DoCFindCancellation(int iMsgID, int iResponseID)
{
#ifdef _PROFILE
	ScopeTimer timer("DoCFindCancellation");
#endif

	PxDicomStatus  status;
 	if (!m_isCFindCancelResquestSent)
	{			 
		// Prepare and Send the C-FIND-CANCEL request message to the SCP 
		int cancelMessageID = -1;
		status = (PxDicomStatus)MC_Open_Empty_Message(&cancelMessageID);
		if (status != kNormalCompletion)
		{
			PrintOutOnError("Create Empty Message in VLIDicom::DoCFindCancellation()", status);
			return status;
		}
	 
		status = (PxDicomStatus)MC_Set_Service_Command(cancelMessageID, "STUDY_ROOT_QR_FIND", C_CANCEL_FIND_RQ);
		if (status != kNormalCompletion)
		{
			PrintOutOnError("Set Model in VLIDicom::DoCFindCancellation()", status);
			MC_Free_Message(&cancelMessageID);
			return status;
		}

		//	Set Message ID from the active Request message so we can cancel the correct message
		status = (PxDicomStatus)MC_Set_Value_From_Int(cancelMessageID, MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO, iMsgID);
	 	if (status != kNormalCompletion)
		{
			PrintOutOnError("Set Message ID in VLIDicom::DoCFindCancellation()", status);
			MC_Free_Message(&cancelMessageID);
			return status;
		}
	 
		//	Send the cancel request
		status = (PxDicomStatus)MC_Send_Request_Message(m_currentServer.m_associationID, cancelMessageID);
		if (status != kNormalCompletion)
		{
			PrintOutOnError("Send C-FIND-CANCEL RQ in VLIDicom::DoCFindCancellation()", status);
			MC_Free_Message(&cancelMessageID);
			return status;
		}
		
		MC_Free_Message(&cancelMessageID);	

		m_isCFindCancelResquestSent = true;
		PrintIntOnDebug ("CFind Cancel request sent with associationID =", m_currentServer.m_associationID); 
	} 

	return kNormalCompletion;;		 
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::PrepareAndSendCFindReqMSG(int iMsgID)
{
	PxDicomStatus  status;
	//	Set Affected SOP Class UID  
	char affSOPClassUID[65];
	status = (PxDicomStatus) MC_Get_UID_From_MergeCOM_Service("STUDY_ROOT_QR_FIND", affSOPClassUID, sizeof(affSOPClassUID));
	
	if(status != kNormalCompletion)
	{
		PrintOutOnError("Get affected SOP Class UID in VLIDicom::PrepareAndSendCFindReqMSG()", status);
		return status;
	}

	status = (PxDicomStatus)MC_Set_Value_From_String (iMsgID, MC_ATT_AFFECTED_SOP_CLASS_UID,  affSOPClassUID);
	 
	if (status != kNormalCompletion)
	{
		PrintOutOnError("Set affected SOP Class UID in VLIDicom::PrepareAndSendCFindReqMSG()", status);
		return status;
	}

	//	Set Priorotiy - RL & VS 12/20/01
	unsigned short priority = 0x0001;	// HIGH
	status = (PxDicomStatus)MC_Set_Value_From_UShortInt(iMsgID, MC_ATT_PRIORITY, priority);
	if (status != kNormalCompletion)
	{
		PrintOutOnError("Set Priority in VLIDicom::PrepareAndSendCFindReqMSG()", status);
		return status;
	}
 
	//	Set dataset Type to 0x0102 Dataset Present - RL 12/20/01
	status = (PxDicomStatus)MC_Set_Value_From_UShortInt(iMsgID, MC_ATT_DATA_SET_TYPE, DATASET_PRESENT);
	if (status != kNormalCompletion)
	{
		PrintOutOnError("Set Data Set Type in VLIDicom::PrepareAndSendCFindReqMSG()", status);
		return status;
	}

	status = (PxDicomStatus)MC_Send_Request_Message (m_currentServer.m_associationID, iMsgID);
	if (status != kNormalCompletion)
	{
		PrintOutOnError("SendRequest in VLIDicom::PrepareAndSendCFindReqMSG()", status);
		return status;
	}

	return status;
}

//-----------------------------------------------------------------------------------------
//
bool PatientLessThan(const CPxDicomMessage* m1, const CPxDicomMessage* m2)
{
	MC_STATUS status1, status2;
	char id1[kMaxUID];
	char id2[kMaxUID];
	
	status1 = MC_Get_Value_To_String(m1->GetID(), MC_ATT_PATIENT_ID, kMaxUID, id1);
	status2 = MC_Get_Value_To_String(m2->GetID(), MC_ATT_PATIENT_ID, kMaxUID, id2);
	
	return (strcmp(id1, id2) < 0);
}

//-----------------------------------------------------------------------------------------
//
bool StudyLessThan(const CPxDicomMessage* m1, const CPxDicomMessage* m2)
{
	MC_STATUS status1, status2;
	char id1[kMaxUID];
	char id2[kMaxUID];
	
	status1 = MC_Get_Value_To_String(m1->GetID(), MC_ATT_STUDY_INSTANCE_UID, kMaxUID, id1);
	status2 = MC_Get_Value_To_String(m2->GetID(), MC_ATT_STUDY_INSTANCE_UID, kMaxUID, id2);
	
	return (strcmp(id1, id2) < 0);
}

//-----------------------------------------------------------------------------------------
//
bool SeriesLessThan(const CPxDicomMessage* m1, const CPxDicomMessage* m2)
{
	MC_STATUS status1, status2;
	char id1[kMaxUID];
	char id2[kMaxUID];
	
	status1 = MC_Get_Value_To_String(m1->GetID(), MC_ATT_SERIES_INSTANCE_UID, kMaxUID, id1);
	status2 = MC_Get_Value_To_String(m2->GetID(), MC_ATT_SERIES_INSTANCE_UID, kMaxUID, id2);
	
	return (strcmp(id1, id2) < 0);
}

//-----------------------------------------------------------------------------------------
//
bool ImageLessThan(const CPxDicomMessage* m1, const CPxDicomMessage* m2)
{
	MC_STATUS status1, status2;
	char id1[kMaxUID];
	char id2[kMaxUID];
	
	status1 = MC_Get_Value_To_String(m1->GetID(), MC_ATT_SOP_INSTANCE_UID, kMaxUID, id1);
	status2 = MC_Get_Value_To_String(m2->GetID(), MC_ATT_SOP_INSTANCE_UID, kMaxUID, id2);
	
	return (strcmp(id1, id2) < 0);
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::RemoveDuplicateQueryResults(CPxDicomMessage& iQuery, std::vector<CPxDicomMessage*>& ioMessageVector)
{
#ifdef _PROFILE
	ScopeTimer timer("RemoveDuplicateQueryResults");
#endif

	PxDicomStatus status;				
	char levelString[32];
	unsigned long tag;
	
	//	The query level will tell us which tag to compare.  
	status = (PxDicomStatus) MC_Get_Value_To_String(iQuery.GetID(), MC_ATT_QUERY_RETRIEVE_LEVEL, 32, levelString); 
	PrintOnError("Get Level From Query", status);
	if (status != kNormalCompletion)
	{
		return kRequiredAttributeMissing;
	} 	
		
	if ((!strcmp(levelString, "STUDY") && !m_removeDuplicateQueryResults))
		return kNormalCompletion;

	// Added try/catch to avoid dicom.dll going away. not a fix. (TCZ 07/30/2001)
	// XXXX TODO TODO To Do
	try
	{	
		if (!strcmp(levelString, "IMAGE"))
		{
			tag = MC_ATT_SOP_INSTANCE_UID;
			std::stable_sort(ioMessageVector.begin(), ioMessageVector.end(), ImageLessThan);
		} else if (!strcmp(levelString, "SERIES"))
		{
			tag = MC_ATT_SERIES_INSTANCE_UID;
			std::stable_sort(ioMessageVector.begin(), ioMessageVector.end(), SeriesLessThan);
		} else if (!strcmp(levelString, "STUDY"))
		{
			tag = MC_ATT_STUDY_INSTANCE_UID;
			std::stable_sort(ioMessageVector.begin(), ioMessageVector.end(), StudyLessThan);
		} else if (!strcmp(levelString, "PATIENT"))
		{
			tag = MC_ATT_PATIENT_ID;
			std::stable_sort(ioMessageVector.begin(), ioMessageVector.end(), PatientLessThan);
		} else
		{
			return kInvalidQueryLevel;
		}
	}
	catch (...)
	{
		LogMessage("MergeSort failed\n");
		return kSortFailed;	
	}

	std::vector<CPxDicomMessage*>::iterator iter;
	int size;
	for(iter = ioMessageVector.begin(); iter != ioMessageVector.end(); iter++)
	{
		size = ioMessageVector.size();
		status = CheckForAndRemoveDuplicateAtIndex(ioMessageVector, iter, tag);
		if (status != kNormalCompletion) 
			return status;
	}

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CFindSCU::CheckForAndRemoveDuplicateAtIndex(std::vector<CPxDicomMessage*>& ioMessageVector, 
														   std::vector<CPxDicomMessage*>::iterator iter, 
														   unsigned long iTag)
{
#ifdef _PROFILE
	ScopeTimer timer("CheckForAndRemoveDuplicateAtIndex");
#endif
	PxDicomStatus status1, status2;
	char id1[kMaxUID];
	char id2[kMaxUID];

	std::vector<CPxDicomMessage*>::iterator current = iter;
	std::vector<CPxDicomMessage*>::iterator next    = current + 1;

	if (next == ioMessageVector.end())
		return kNormalCompletion;

	status1 = (PxDicomStatus) MC_Get_Value_To_String((*current)->GetID(), iTag, kMaxUID, id1); 
	PrintOnError("Get PatientID", status1);
	status2 = (PxDicomStatus) MC_Get_Value_To_String((*next)->GetID(), iTag, kMaxUID, id2); 
	PrintOnError("Get PatientID", status2);
	if (status1 != kNormalCompletion || status2 != kNormalCompletion)
	{
		return kRequiredAttributeMissing;
	}

	if (!strcmp(id1,id2))
	{
		delete *next;
		ioMessageVector.erase(next);

		//	Keep doing this until the 
		return CheckForAndRemoveDuplicateAtIndex(ioMessageVector, iter, iTag);
	}
	
	return kNormalCompletion;
}

int CMoveSCU::m_testMoveOriginator = 0;

//-----------------------------------------------------------------------------------------
//
void CMoveSCU::FreeReplyVector(std::vector<CPxDicomMessage*>& v)
{
	int sz = v.size();
	for (int i=0;i<sz;i++)
	{
		delete v[i];
	}
	v.clear();
}

//-----------------------------------------------------------------------------------------
//
void CMoveSCU::FreeReplyVector(std::vector<CPxDicomImage*>& v)
{
#ifdef _PROFILE
	ScopeTimer timer("FreeReplyVector");
#endif
	for (int i=0;i<v.size();i++)
	{
#ifdef _DEBUG
		fprintf(stderr,"1 - Deleting %d bytes\n", v[i]->GetNumberOfBytesOfPixelData());
#endif
		delete v[i];
	}
	
	v.clear();
}

//-----------------------------------------------------------------------------
VLIDICOMPOOL CMoveSCU::SerialKeyPool;


//----------------------------------------------------------------------------------------
//
CMoveSCU::CMoveSCU()
{
	m_cMoveRequestMessageID = -1;
	m_cMoveResponseID = -1;
	m_connectedDiCOMStores.Clear();
	m_pImages = 0;
	m_pStudyUIDs =  0;
	m_pSeriesUIDs = 0;
	m_pSOPUIDs = 0;

	m_totalExpectedImages = -1;
	m_imagesReceivedSoFar = -1;
	m_progressEvenIfNotLocalDest = false;
}

//----------------------------------------------------------------------------------------
//
bool CancellDiCOMStore(DiCOMStorePOOL::iterator& theIterator)
{
	theIterator->first->RequestTermination();
	return true; // continue the walk
}

CMoveSCU::~CMoveSCU()
{
#ifdef _PROFILE
	ScopeTimer timer("~CMoveSCU", GetCurrentThreadId());
#endif

	//	Handle cancellation
	m_bCmoveExit = true; // stop accept new connection
	if(m_connectedDiCOMStores.Size())
		m_connectedDiCOMStores.Walk(CancellDiCOMStore);
	//	Wait until the cancels are completed
	int count = 0;
	while(m_connectedDiCOMStores.Size() != 0 && count < 500)
	{
		Sleep(100);
		count++;
	}
	SerialKeyPool.Remove(this);
	m_connectedDiCOMStores.Clear();

	//	Free messages
	if (m_cMoveRequestMessageID > 0)
		MC_Free_Message(&m_cMoveRequestMessageID);

	if (m_cMoveResponseID > 0)
		MC_Free_Message(&m_cMoveResponseID);

	if( m_pImages)
	{
		std::vector<CPxDicomImage*>* pImages = m_pImages;
		m_pImages = 0; // tell no body should use this pointer
		FreeReplyVector(*pImages);
	}

}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::RetrieveSeries(std::vector<CPxDicomImage*>& oSeries, 
										const char* iSeriesUID, 
										const char* iStudyUID, 
										DicomDataSource* iSource,
										const char* iDestAETitle,
										bool iProgressEvenIfNotLocalDest)

{
#ifdef _PROFILE
	ScopeTimer timer("RetrieveSeries");
#endif
	m_cancel = 0;

	std::vector<std::string> seriesV, studyV, instanceV;
	std::vector<DicomDataSource> dsv;
	std::string modality;

	oSeries.clear();
	studyV.clear();
	seriesV.clear();
	instanceV.clear();

	seriesV.push_back(iSeriesUID);
	studyV.push_back(iStudyUID);

	return Retrieve(oSeries, studyV, seriesV, instanceV, "SERIES", iSource, iDestAETitle, iProgressEvenIfNotLocalDest);

	//
	//	The following code was used for async retrieve.  It is not being used right now
	//  If enable it, remember start the one shut thread maanger thread as follows in main()
	//	iRTVThread OneShotThreadManagerThread(&RTVOneShotThreadManager::theManager());
	//	RTVOneShotThreadManager::theManager().SetMaxRunThreads(3, "Retrieve");
	//
#if 0
	//	Single thread retrieve - do it the old way
	if (m_numberOfRetrieveThreads < 2)
	{
		return Retrieve(oSeries, studyV, seriesV, instanceV, "SERIES", iSource, iDestAETitle, iProgressEvenIfNotLocalDest);
	}

	PxDicomStatus status;
	MC_STATUS mcStatus;

	//	Need to check this here so we can do the instance level query to the right server
	if (iSource == NULL || iSource->type==kUnknownSource) 
	{
		// check cache first (-- Jan 2002)
		tStudyCache::iterator it;
		if ((it = m_studyCache.find(iStudyUID)) != m_studyCache.end())
		{
			dsv.push_back(it->second->m_dataSource);
		}
		else
		{	
			if (iStudyUID && iSeriesUID)
			{
				status = QueryFindDataSources(dsv, iStudyUID, iSeriesUID, modality);
				if (dsv.size() < 1)
				{
					m_status = kNotFound;
					return m_status;
				}
			}
		}
	
		iSource = &dsv[0];
	}

	//	Only do multi-thread for CT or MR
/*	if (!modality.compare("CT") || !modality.compare("MR"))
	{
		return Retrieve(oSeries, studyV, seriesV, instanceV, "SERIES", iSource, iDestAETitle, iProgressEvenIfNotLocalDest);
	}
*/
	//	Get the list of InstanceUID's for this series
	//	TODO: This is expensive for large series.  Optimize where possible.
	std::vector<CPxDicomMessage*> msgV;
	status = QueryListOfImages(msgV, iSeriesUID, iStudyUID, iSource);
	if (status != kNormalCompletion)
	{
		return status;
	}

	//	Divide the list by the number of retrieve threads, and start them
	int total = msgV.size();
	if (total < 1)
	{
		return kNotFound;
	//	If it's a single image, retrieve single thread
	} else if (total == 1)
	{
		return Retrieve(oSeries, studyV, seriesV, instanceV, "SERIES", iSource, iDestAETitle, iProgressEvenIfNotLocalDest);
	}

	int count=0;
	int chunkSize = total / m_numberOfRetrieveThreads;
	std::vector<std::string> chunk;
	std::vector< std::vector<std::string> > chunkV;
	char uid[kVR_UI];
	for(int i=0; i<m_numberOfRetrieveThreads; i++)
	{
		chunk.clear();
		for(int j=0; j<chunkSize; j++)
		{
			mcStatus = MC_Get_Value_To_String(msgV[count++]->GetID(), MC_ATT_SOP_INSTANCE_UID, sizeof uid, uid);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				LogMessage("Failed (%d,%s) on get MC_ATT_SOP_INSTANCE_UID\n", mcStatus, DicomStatusString(mcStatus));
				return (PxDicomStatus) mcStatus;
			}
			chunk.push_back(uid);
		}
		chunkV.push_back(chunk);
	}

	//	Any remaining uid's go in the last chunk
	while(count<msgV.size())
	{
		mcStatus = MC_Get_Value_To_String(msgV[count++]->GetID(), MC_ATT_SOP_INSTANCE_UID, sizeof uid, uid);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			LogMessage("Failed (%d,%s) on get MC_ATT_SOP_INSTANCE_UID\n", mcStatus, DicomStatusString(mcStatus));
			return (PxDicomStatus) mcStatus;
		}
		chunkV[m_numberOfRetrieveThreads-1].push_back(uid);
	}

	//	Set the retrieves in motion
	DicomDataSource ds = msgV[0]->GetDataSource();
	for(i=0; i< chunkV.size(); i++)
	{
		m_pImages = &oSeries;
		CMoveSCU* retriever = new CMoveSCU(this, oSeries, studyV, seriesV, chunkV[i], 
									"IMAGE", &ds, iDestAETitle, iProgressEvenIfNotLocalDest, 0);

		RTVOneShotThreadManager::theManager().AddRequest(retriever);
	}

	Sleep(500);		//	This is so we know the manager has started the threads

	//	Loop until they are done or cancelled
	int numThreads = RTVOneShotThreadManager::theManager().GetRunningThreads("Retrieve");
	while(numThreads > 0)
	{
		Sleep(100);
		numThreads = RTVOneShotThreadManager::theManager().GetRunningThreads("Retrieve");
	}

	
	//	Handle cancellation
	if(m_cancel)
	{
		m_bCmoveExit = true; // stop accept new connection
		if(m_connectedDiCOMStores.Size())
			m_connectedDiCOMStores.Walk(CancellDiCOMStore);
	}

	//	Wait until the cancels are completed
	count = 0;
	while(m_connectedDiCOMStores.Size() != 0 && count < 5000)
	{
		Sleep(100);
		count++;
	}
	SerialKeyPool.Remove(this);
	m_connectedDiCOMStores.Clear();
	
	return m_status;
#endif
}

//-----------------------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::RetrieveStudy(std::vector<CPxDicomImage*>& oImages, 
										const char* iSeriesUID,
										const char* iStudyUID, 
										DicomDataSource* iSource,
										const char* iDestAETitle,
										bool iProgressEvenIfNotLocalDest)

{
#ifdef _PROFILE
	ScopeTimer timer("RetrieveSeries");
#endif
	m_cancel = 0;

	std::vector<std::string> seriesV, studyV, instanceV;
	std::vector<DicomDataSource> dsv;
	std::string modality;

	oImages.clear();
	studyV.clear();
	seriesV.clear();
	instanceV.clear();

	seriesV.push_back(iSeriesUID);
	studyV.push_back(iStudyUID);

	return Retrieve(oImages, studyV, seriesV, instanceV, "STUDY", iSource, iDestAETitle, iProgressEvenIfNotLocalDest);
}
//-----------------------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::RetrieveImages(std::vector<CPxDicomImage*>& oImages,
										std::vector<const char*>& iSOPUIDs, 
										const char* iSeriesUID, 
										const char* iStudyUID, 
										DicomDataSource* iSource,
										const char* iDestAETitle)
{
	std::vector<std::string> instanceV, seriesV, studyV;
	seriesV.push_back(iSeriesUID);
	studyV.push_back(iStudyUID);

	for(int i=0; i<iSOPUIDs.size(); i++)
	{
		instanceV.push_back(iSOPUIDs[i]);
	}

	return Retrieve(oImages, studyV, seriesV, instanceV, "IMAGE", iSource, iDestAETitle, false); 
}

//----------------------------------------------------------------------------------------
//
std::string CMoveSCU::MakeSerialKey(const char* sUID, const char* remoteEntity)
{
	std::string	serialKey = sUID;
	return serialKey += remoteEntity;
}

//	-- - 05/21/02 - END
#include "AppComCacheWriter.h"
#include "AqCore/TRCriticalsection.h"
//-----------------------------------------------------------------------------------------
//
//	This function prepares and sends a C-MOVE request to the Q/R SCP.  It then
//	waits for the SCP to request a C-STORE association with us, which it will accept.
//  It then manages the two associations:
//		1. C-MOVE	our request to the Q/R SCP to move the series
//		2. C-STORE  the SCP's requests to store images
//	Both of these associations must be polled "simultaneously" to read any queued messages / status
//
PxDicomStatus CMoveSCU::Retrieve(std::vector<CPxDicomImage*>& oImages, 
								  std::vector<std::string>& iStudyUIDs,
								  std::vector<std::string>& iSeriesUIDs,
								  std::vector<std::string>& iSOPUIDs,
								  char* iLevel,
								  DicomDataSource* iSource,
								  const char* iDestAETitle,
								  bool iProgressEvenIfNotLocalDest)
{

	if (iSeriesUIDs.size() == 0)
	{
		return kInvalidArguments;
	}

	std::string key = "DICOMSCU1" + iSeriesUIDs[0];
	TRNamedCSLock lock(key.c_str());

#ifdef _PROFILE
//#ifdef _PROFILE_THIS
	ScopeTimer timer("Retrieve");
#endif
	//reset cancell flag
	m_cancel = 0;

	// save image list for process
	if(m_pImages)
		FreeReplyVector(*m_pImages);
	m_pImages = &oImages;

	m_pStudyUIDs = &iStudyUIDs;
	m_pSeriesUIDs = &iSeriesUIDs;
	m_pSOPUIDs = &iSOPUIDs;
	m_level = iLevel;
	m_destAETitle = iDestAETitle;
	m_bCmoveExit = false;
	m_progressEvenIfNotLocalDest = iProgressEvenIfNotLocalDest;

	//	Free messages
	if (m_cMoveRequestMessageID > 0)
		MC_Free_Message(&m_cMoveRequestMessageID);

	if (m_cMoveResponseID > 0)
		MC_Free_Message(&m_cMoveResponseID);

	// get rid of tail slash
	char cacheDir[kMaxPathNameLen];
  	strcpy(cacheDir, m_localCache);
  	int str_end = strlen(cacheDir)-1;
  	if(cacheDir[str_end] == '/' || cacheDir[str_end] == '\\')
  		cacheDir[str_end] = 0;
#if 0
	if(strlen(cacheDir) > 10)
		TRPlatform::RemoveDirectory(cacheDir);
#endif

	// No source specified - let's find out who's got it, and pick the best one
	if (iSource == NULL || iSource->type==kUnknownSource || iSource->m_server.m_applicationEntityTitle[0] == '\0') // added Unknown Check (TC 9/30/01)
	{
		m_dataSource.type = kUnknownSource;
	} 
	else
	{
		m_dataSource.type = iSource->type;
		m_dataSource.m_server = iSource->m_server;
	}

	m_status = DoCMove();

	//	 Reset progress;
	m_imagesReceivedSoFar = -1;
	m_totalExpectedImages = -1;

	if(m_cancel)
	{
		// tell all CStroe to stop
		m_bCmoveExit = true; // stop accept new connection
		m_connectedDiCOMStores.Walk(CancellDiCOMStore);
		m_status = kRetrieveAborted;
	}
	
	int count = 0, maxCount = 240;
	if(oImages.size() != 0)
	{
		if(oImages[0]->GetNumberOfFrameBuffers() == 0)
			maxCount = 2400; // someone doing cache writing, wait for 20 minutes
	}
	
	while(m_connectedDiCOMStores.Size() != 0 && count < maxCount)
	{
		Sleep(500);
		count++;
	}
	SerialKeyPool.Remove(this);
	m_connectedDiCOMStores.Clear();
	m_pImages = 0; //release pointer to stop any CStore to add images

  	int i;
	CPxDicomImage* pImage;
	std::string classSOP;

	if(oImages.size() != 0)
  	{
  		// check if there are images should be put into cache here
		if(AppComCacheWriter::HasCache(cacheDir) && m_status == kNormalCompletion)
		{
			AppComCacheWriter cacheWriter;
			int _status;
	
			for(i=0; i<oImages.size(); i++)
			{
				pImage = oImages[i];
				if(pImage->GetNumberOfFrameBuffers() <= 0)
					continue;
				classSOP = pImage->GetSOPClassUID();
				if(IsSOPClassUIDSC(classSOP))
				{

					_status = cacheWriter.AddSCImage (
						cacheDir, 
 						pImage->GetModalityStr().c_str(),
                       //"SC",
						pImage->GetSOPInstanceUID(),
                        pImage->GetSOPClassUID(),
						pImage->GetBitsAllocated(),
						pImage->GetBitsStored(), 
						pImage->GetHighBit(), 
						pImage->GetNumberOfColumns(), 
						pImage->GetNumberOfRows(), 
						pImage->IsLittleEndian(),
						pImage->GetImageTypeTokens(),
						pImage->GetImagePixels(),
						pImage->GetSamplesPerPixel(),
						(ePhotometricInterpretation) pImage->GetPhotometricInterpretation(),
						(ePlanarConfiguration) pImage->GetPlanarConfiguration(), 
						kNone,
						(int)pImage->GetWindowWidth(),
						(int)pImage->GetWindowCenter(),
						pImage->GetInstanceNumber()
						);
					if (_status != kSuccess)
					{
						m_status = kRetrievePartialFailure;
						LogMessage( "ERROR: SCU AddSCImage failed in cache: %s\n", cacheDir);
						break;
					}

				}
				else
				{
					LogMessage( "ERROR: SCU retrieved some images other than SC that are not in cache: %s\n", cacheDir);
					m_status = kRetrievePartialFailure;
					break;
				}
			}
			
			//we have cache, clean it to let image server load from cache
			FreeReplyVector(oImages);
		}
  	}
  
	//close cache
  	if(AppComCacheWriter::CloseCache(cacheDir) == kSuccess)
	{
		LogMessage(kInfo,"INFO: VLIDicom::Retrieve - closed cache for dir = %s\n", cacheDir);
	}

	if(m_status != kNormalCompletion)
	{
		//error occurs, clear output
		FreeReplyVector(oImages);

		// delete cache on error		
		std::string cachefile = cacheDir;
		cachefile += "/cache.description";
		
		// TC & Rob: SCU does not write cache for CT and MR, thus no matter
		// what the status is, leave it alone. 2003/02/27
		//
		if (!IsSOPClassUIDCT(classSOP) && !IsSOPClassUIDMR(classSOP))
		{
			LogMessage("ERROR: VLIDicom::Retrieve - remove cache on error: %s\n", cacheDir);		
			TRPlatform::remove(cachefile.c_str());
		}
		
		cachefile = cacheDir;
		cachefile += "/cache.data";
		TRPlatform::remove(cachefile.c_str());
	}
	return m_status;
}


void CMoveSCU::AddSerialKey(const char* sUID, const char* remoteIP, const char* remoteAE, int rMsgID)
{
	std::string serialKey;

	serialKey = MakeSerialKey(sUID, remoteIP);
	SerialKeyPool.Add(serialKey, this);
	LogMessage(kInfo, "INFO: C-MOVE Add Key for [%s, %s]\n", sUID, remoteIP);		

	serialKey = MakeSerialKey(sUID, remoteAE);
	SerialKeyPool.Add(serialKey, this);
	LogMessage(kInfo, "INFO: C-MOVE Add Key for [%s, %s]\n", sUID, remoteAE);		

	//	-- 2005.06.28 - Some PACS do not populate CMoveRequestMessageID.
	//	In that case, and when the C-STORE is delegated to a host different from
	//		the one who got the C-MOVE request, we need to be able to connect
	//		the DicomStore to the SCU by UID alone.
	if (m_connectCMoveBySeriesUID)
	{
		serialKey = MakeSerialKey(sUID, "");
		SerialKeyPool.Add(serialKey, this);
		LogMessage(kInfo, "INFO: C-MOVE Add Key for [%s]\n", sUID);		
	}

	if (!m_testMoveOriginator)
		return;

	char idBuf[16];
	_snprintf(idBuf, sizeof idBuf, "%d", rMsgID); idBuf[15] = 0;
	serialKey = MakeSerialKey(sUID, idBuf);
	SerialKeyPool.Add(serialKey, this);
	LogMessage(kInfo, "INFO: C-MOVE Add Key for [%s, %s]\n", sUID, idBuf);		

}


CMoveSCU* CMoveSCU::FromSerialKey(DiCOMStore* pStore)
{
#ifdef NO_SELF_STORE
 
	return 0;
#else
	CMoveSCU* pMover=0;
	if(!pStore) return pMover;

	const char* serUID = pStore->GetSeriesInstanceUID();
	const char* stuUID = pStore->GetStudyInstanceUID();
	const char* remoteIP =  pStore->ConnectInfo()->RemoteIPAddress;
	const char* remoteAE = pStore->ConnectInfo()->RemoteApplicationTitle;
	int rMsgID = GetMoveMessageID(pStore->GetMessageID());

	std::string serialKey;
	
	serialKey = MakeSerialKey(serUID, remoteIP);
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover) return pMover;

	serialKey = MakeSerialKey(serUID, remoteAE);
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover) return pMover;

	//	-- 2005.01.26 - Support Study Level C-MOVE
	serialKey = MakeSerialKey(stuUID, remoteIP);
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover) return pMover;

	serialKey = MakeSerialKey(stuUID, remoteAE);
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover) return pMover;

	char idBuf[16];
	_snprintf(idBuf, sizeof idBuf, "%d", rMsgID); idBuf[15] = 0;
	serialKey = MakeSerialKey(serUID, idBuf);
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover) return pMover;

	//	-- 2005.06.28 - Some PACS do not populate CMoveRequestMessageID.
	//	In that case, and when the C-STORE is delegated to a host different from
	//		the one who got the C-MOVE request, we need to be able to connect
	//		the DicomStore to the SCU by UID alone.  However, it is tried last - 
	//		only in the case where none of the other keys are there, then we try this one.
	//		The only problem is when the same data is requested from different SCU threads.
	//		In that case, the last one added will get the data, and the other one will fail.
	serialKey = MakeSerialKey(serUID, "");
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover && pMover->m_connectCMoveBySeriesUID)
		return pMover;

	serialKey = MakeSerialKey(stuUID, "");
	pMover = SerialKeyPool.Get(serialKey);
	if(pMover && pMover->m_connectCMoveBySeriesUID)
		return pMover;

	return 0;
#endif
}

//----------------------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::DoCMove()
{
#ifdef _PROFILE
//#ifdef _PROFILE_THIS
	ScopeTimer timer("DoCMove", GetCurrentThreadId());
#endif
	int i;
	//	Check for cancel
	if (m_cancel || !m_pStudyUIDs || !m_pSeriesUIDs || !m_pSOPUIDs )
	{
		m_status = kRetrieveAborted;
		return m_status;
	}

	//GL  the design to retriev multiple studies will not really work 
	// if studies are on the multip DICOM servers. need to re-design

	const std::vector<std::string>& m_studyUIDs =  *m_pStudyUIDs;
	const std::vector<std::string>& m_seriesUIDs = *m_pSeriesUIDs;
	const std::vector<std::string>& m_SOPUIDs = *m_pSOPUIDs;

	// No source specified - let's find out who's got it, and pick the best one
	if (m_dataSource.type == kUnknownSource) // added Unknown Check (TC 9/30/01)
	{
		std::string modality;
		// check cache first (-- Jan 2002)
		std::vector<DicomDataSource> dsv;
		tStudyCache::iterator it;
		if (m_studyUIDs.size() > 0 && (it = m_studyCache.find(m_studyUIDs[0].c_str())) != m_studyCache.end())
		{
			dsv.push_back(it->second->m_dataSource);
		}
		else
		{	
			if (m_studyUIDs.size() > 0 && m_seriesUIDs.size() > 0)
			{
				// only search the one server that has the study
				m_status = QueryFindDataSources(dsv, m_studyUIDs[0].c_str(), m_seriesUIDs[0].c_str(), modality, true);
				if (dsv.size() <= 0)
				{
					m_status = kNotFound;
					return m_status;
				}
			}
		}
		m_dataSource.type = dsv[0].type;
		m_dataSource.m_server = dsv[0].m_server;
	} 

	if (m_cancel)
	{
		m_pImages = 0;
		return kRetrieveAborted;
	}

	Progress(0,0,0,0,kVLIRetrieve);
	
	m_status = SetCurrentServer(m_dataSource.m_server); 
	PrintOnError("Set Current Server", m_status);
	if (m_status != kNormalCompletion)
		return m_status;
	
	//	Make sure we have a valid open association with the currently selected DICOM server
	VLIAssociationChecker check(&m_currentServer, this, "Retrieve");
	if (!check.OK())
	{
		m_status = check.GetStatus();
		return m_status;
	}

	//	Get socket info for cmove association
	AssocInfo ai;
	m_status = (PxDicomStatus)MC_Get_Association_Info(m_currentServer.m_associationID, &ai);
	if (m_status != kNormalCompletion)
	{
		LogMessage("Could not get association info on CMoveAssociation %d\n", m_currentServer.m_associationID);
		return m_status;
	}

	//	
	//	Prepare and Send the CMove Request Message
	//
	m_status = (PxDicomStatus)MC_Open_Empty_Message(&m_cMoveRequestMessageID);
	if (m_status != kNormalCompletion)
	{
		m_cMoveRequestMessageID = -1;
		return m_status;
	}

	m_status = (PxDicomStatus)MC_Set_Value_From_Int(m_cMoveRequestMessageID, MC_ATT_MESSAGE_ID, m_cMoveRequestMessageID);
	PrintOnError("Set MessageID Tag in CMove Request Message", m_status);
	if (m_status != kNormalCompletion)
	{
		return m_status;
	}
	
	m_status = PrepareCMOVERequestMessage(); 
	PrintOnError("Prepare C-Move-Rq Message", m_status);
	if (m_status != kNormalCompletion)
	{
		return m_status;
	}
	
	//	Check for cancel
	if (IsCancelled())
	{
		m_status = kRetrieveAborted;
		return m_status;
	}

	for(i=0; i<m_seriesUIDs.size(); i++)
 	{
		AddSerialKey(m_seriesUIDs[i].c_str(), ai.RemoteIPAddress, 
			ai.RemoteApplicationTitle, m_cMoveRequestMessageID);
	}
	
	//	-- 2005.01.26 - Support Study Level C-MOVE
	for(i=0; i<m_studyUIDs.size(); i++)
 	{
		AddSerialKey(m_studyUIDs[i].c_str(), ai.RemoteIPAddress, 
			ai.RemoteApplicationTitle, m_cMoveRequestMessageID);
	}

	m_status = (PxDicomStatus)MC_Send_Request_Message(m_currentServer.m_associationID, m_cMoveRequestMessageID); 
	PrintOnError("Send C-MOVE RQ", m_status);
	if (m_status != kNormalCompletion)
	{
		return m_status;
	}
	
	//	If the destination is not us, then once the request has been sent, we are done
	bool localDest = true;
	if (m_destAETitle != NULL && strcmp(m_destAETitle, VLIDicomLocalAETitle))
	{
		localDest = false;
	}

	if (!localDest && !m_progressEvenIfNotLocalDest)
	{
		m_status = kNormalCompletion;
		return m_status;
	}

	//
	//	When the SCP gets our C-MOVE request, it will request a C-STORE association with us.  When that happens
	//	we accept the association
	//

	//	-- - 05/04/02 - Rewrote this loop to avoid 15 second delays and unnecessary timeouts.  Also checking for cancel.
	m_status = (PxDicomStatus)-1;
	
	//	-- - 1/18/02 - A hack / quick fix to compensate for a problem in the logic of this function.  If a C-MOVE 
	//	response witn nr==0 comes before the first C-STORE-RQ (Toshiba), we were shutting down the association.
	unsigned int	cMoveResponseStatus = C_MOVE_PENDING_MORE_SUB_OPERATIONS;	// Status returned from SCP on C-MOVE assoc


	bool			cancelRequestSent = false;
	int				selectTimeout = 1;

	char* serviceName;
	MC_COMMAND	command;
	
	int messagesSinceCancelRequest = 0;
	time_t startTime, currentTime;
	time(&startTime);

//	::DebugBreak();

	int maxNC = 0; // -- 2006.10.17 Desperate attempt to get the number of images right

	for(;;)
	{
		//	Check for C-MOVE response messages
		if (IsCancelled())
		{
			if (messagesSinceCancelRequest >= 2)
			{
				m_status = kRetrieveAborted;
				return m_status;
			}

			if (!cancelRequestSent)
			{
				SendCMoveCancelRequest();	
				cancelRequestSent = true;
			}
			else
			{
				++messagesSinceCancelRequest;
			}
		}

		//	Check for messages
		//
		MC_Free_Message(&m_cMoveResponseID);
		m_status = (PxDicomStatus)MC_Read_Message(m_currentServer.m_associationID, 1, &m_cMoveResponseID, &serviceName, &command);

		time(&currentTime);

		//	Timeout on this read - check for total timeout
		if(m_status == kTimeout)
		{
			//	No message activity...timeout
			if((currentTime-startTime) > m_timeoutOnRetrieve)
			{
				LogMessage("VLIDicom::DoCMove - TimeoutOnRetrieve of %d seconds expired", m_timeoutOnRetrieve);
				MC_Free_Message(&m_cMoveResponseID);
				return kRetrievePartialFailure;
			}
			else
			{
				continue; 
			}
		}
		else
		{
			//	Something was received - reset timer
			startTime = currentTime;
		}
			
		if (m_status != kNormalCompletion)
		{
			return m_status;
		}

#ifdef _MSGDUMP
		TRDICOMUtil::DumpMessage(m_cMoveResponseID, serviceName, command);
#endif
		
	
		if (command != C_MOVE_RSP)
		{
			LogMessage("Expecting C-MOVE-RSP (%d), got %d (association %d)\n", command, m_currentServer.m_associationID);
			continue;
		}

		int nFailedInstances = 0;
		int	nr = -1,nc = 0,nf = 0,nw = 0;

		m_status = (PxDicomStatus)MC_Get_Value_To_Int(m_cMoveResponseID, MC_ATT_NUMBER_OF_COMPLETED_SUBOPERATIONS, &nc); 
		PrintOnError("Get resp nc", m_status);
	
		if (m_status == kNormalCompletion && nc > maxNC)
			maxNC = nc;

		m_status = HandleCMoveResponse(cMoveResponseStatus, nFailedInstances);
		if (m_status == kRetrieveAborted || cMoveResponseStatus != C_MOVE_PENDING_MORE_SUB_OPERATIONS)
		{
			if (cMoveResponseStatus == C_MOVE_SUCCESS_NO_FAILURES)
			{
				if (!localDest && m_progressEvenIfNotLocalDest)
					Progress(0, m_totalExpectedImages <=0 ? maxNC:m_totalExpectedImages, nFailedInstances, 0, kVLIRetrieve);				
			}

			return m_status;
		}
		
		m_status = (PxDicomStatus)MC_Get_Value_To_Int(m_cMoveResponseID, MC_ATT_NUMBER_OF_REMAINING_SUBOPERATIONS, &nr); 
		PrintOnError("Get resp nr", m_status);
		if (m_status != kNormalCompletion)
			nr = -1;
		
	
		m_status = (PxDicomStatus)MC_Get_Value_To_Int(m_cMoveResponseID, MC_ATT_NUMBER_OF_FAILED_SUBOPERATIONS, &nf); 
		PrintOnError("Get resp nf", m_status);
		if (m_status != kNormalCompletion)
			nf = 0;
		
		m_status = (PxDicomStatus)MC_Get_Value_To_Int(m_cMoveResponseID, MC_ATT_NUMBER_OF_WARNING_SUBOPERATIONS, &nw); 
		PrintOnError("Get resp nw", m_status);
		if (m_status != kNormalCompletion)
			nw = 0;
		
		MC_Free_Message(&m_cMoveResponseID); 
		if (m_totalExpectedImages < 0 && nr > 0)
		{
			m_totalExpectedImages = nr + nc;
		}
	
		if (!localDest && m_progressEvenIfNotLocalDest)
			Progress(nr, nc, nf, nw, kVLIRetrieve);
	}
	
	MC_Free_Message(&m_cMoveResponseID);
	m_status = kNormalCompletion;

	return m_status;
}

//----------------------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::PrepareCMOVERequestMessage()
{	
#ifdef _PROFILE
	ScopeTimer timer("PrepareCMOVERequestMessage");
#endif
	MC_STATUS status;
	
	//	Check for cancel
	if (m_cancel || !m_pStudyUIDs || !m_pSeriesUIDs || !m_pSOPUIDs )
	{
		m_status = kRetrieveAborted;
		return m_status;
	}

	const std::vector<std::string>& m_studyUIDs =  *m_pStudyUIDs;
	const std::vector<std::string>& m_seriesUIDs = *m_pSeriesUIDs;
	const std::vector<std::string>& m_SOPUIDs = *m_pSOPUIDs;


	
	// Need to read the Service List param from a config file 
	// And to read timeout values as well
	
	//
	//	Populate this VLIDicom object as the C-MOVE request message
	//
	
	//	Set the service and command
	status = MC_Set_Service_Command(m_cMoveRequestMessageID, "STUDY_ROOT_QR_MOVE", C_MOVE_RQ); 
	if (status != kNormalCompletion)
	{
		LogMessage("Failed (%d,%s) on MC_Set_Service_Command\n", status, DicomStatusString(status));
		return (PxDicomStatus) status;
	}
	
	
	//	Set the Move Destination - to me or 3rd party
	if (m_destAETitle != NULL)
	{		
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_MOVE_DESTINATION,m_destAETitle);
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_MOVE_DESTINATION\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
	} else
	{
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_MOVE_DESTINATION,VLIDicomLocalAETitle);
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_MOVE_DESTINATION\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
	}
	
	//	Set the Unique ID's - we are using heirarchical model only for now (as of 12/20/01 - RL & VS)
	if (strcmp(m_level,"STUDY") == 0)
	{
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_QUERY_RETRIEVE_LEVEL, "STUDY"); 
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_QUERY_RETRIEVE_LEVEL\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
		if (m_studyUIDs.size() < 1)
		{
			LogMessage("Study Level retrieve must specify at least 1 StudyInstanceUID\n");
			return kInvalidArguments;
		}
		
		//	Set the first one
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_STUDY_INSTANCE_UID, m_studyUIDs[0].c_str());
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_STUDY_INSTANCE_UID\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
		//	Set any remaining uid's
		int i;
		for (i=1; i<m_studyUIDs.size(); i++)
		{
			status = MC_Set_Next_Value_From_String (m_cMoveRequestMessageID, MC_ATT_STUDY_INSTANCE_UID, m_studyUIDs[i].c_str()); 
			PrintOnError("Set SeriesInstUID", status);
			if (status != kNormalCompletion)
			{
				LogMessage("Failed (%d,%s) on set MC_ATT_STUDY_INSTANCE_UID\n", status, DicomStatusString(status));
				return (PxDicomStatus)status;
			}
		}
	} 
	else if (strcmp(m_level,"SERIES") == 0)
	{
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_QUERY_RETRIEVE_LEVEL, "SERIES"); 
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_QUERY_RETRIEVE_LEVEL\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
		//	If this iStudyInstanceUID is NULL, we must be using the extended (relational) C-MOVE 
		if (m_studyUIDs.size() > 1)
		{
			LogMessage("Series Level retrieve must specify only 1 StudyInstanceUID\n");
			return kInvalidArguments;
		}
		else if (m_studyUIDs.size() == 1)
		{
			status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_STUDY_INSTANCE_UID, m_studyUIDs[0].c_str());
			if (status != kNormalCompletion)
			{
				LogMessage("Failed (%d,%s) on set MC_ATT_STUDY_INSTANCE_UID\n", status, DicomStatusString(status));
				return (PxDicomStatus)status;
			}
		}

		//	Series level query - have to provide series uid.
		if (m_seriesUIDs.size() < 1)
		{
			LogMessage("Series Level retrieve must specify at least 1 SeriesInstanceUID\n");
			return kInvalidArguments;
		}
		
		//	Set the first one
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_SERIES_INSTANCE_UID, m_seriesUIDs[0].c_str());
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_SERIES_INSTANCE_UID\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
		//	Set any remaining uid's
		int i;
		for (i=1; i<m_seriesUIDs.size(); i++)
		{
			status = MC_Set_Next_Value_From_String (m_cMoveRequestMessageID, MC_ATT_SERIES_INSTANCE_UID, m_seriesUIDs[i].c_str()); 
			PrintOnError("Set SeriesInstUID", status);
			if (status != kNormalCompletion)
			{
				LogMessage("Failed (%d,%s) on set MC_ATT_SERIES_INSTANCE_UID\n", status, DicomStatusString(status));
				return (PxDicomStatus)status;
			}
		}
	} 
	else if (strcmp(m_level, "IMAGE") == 0)
	{
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_QUERY_RETRIEVE_LEVEL, "IMAGE"); 
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_QUERY_RETRIEVE_LEVEL\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
		//	If this iStudyInstanceUID is NULL, we must be using the extended (relational) C-MOVE 
		if (m_studyUIDs.size() > 0) // added non-empty check (TC 06/2001)
		{
			status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_STUDY_INSTANCE_UID, m_studyUIDs[0].c_str()); 
			if (status != kNormalCompletion)
			{
				LogMessage("Failed (%d,%s) on set MC_ATT_STUDY_INSTANCE_UID\n", status, DicomStatusString(status));
				return (PxDicomStatus)status;
			}
			
			
			//	Relational mode - can't have one without the other
			if (m_seriesUIDs.size() != 1)
			{
				return kInvalidArguments;
			}
			
			status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_SERIES_INSTANCE_UID, m_seriesUIDs[0].c_str()); 
			if (status != kNormalCompletion)
			{
				LogMessage("Failed (%d,%s) on set MC_ATT_SERIES_INSTANCE_UID\n", status, DicomStatusString(status));
				return (PxDicomStatus)status;
			}	
		}
		
		//	Image level - have to provide image uid.
		if (m_SOPUIDs.size() < 1)
		{
			LogMessage("Image Level retrieve must specify at least 1 SOPInstanceUID\n");
			return kInvalidArguments;
		}

		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_SOP_INSTANCE_UID, m_SOPUIDs[0].c_str()); 
		if (status != kNormalCompletion)
		{
			LogMessage("Failed (%d,%s) on set MC_ATT_SOP_INSTANCE_UID\n", status, DicomStatusString(status));
			return (PxDicomStatus)status;
		}
		
		//	Set any remaining uid's
		int i;
		for (i=1; i<m_SOPUIDs.size(); i++)
		{
			status = MC_Set_Next_Value_From_String (m_cMoveRequestMessageID, MC_ATT_SOP_INSTANCE_UID, m_SOPUIDs[i].c_str()); 
			if (status != kNormalCompletion)
			{
				LogMessage("Failed (%d,%s) on set MC_ATT_SOP_INSTANCE_UID\n", status, DicomStatusString(status));
				return (PxDicomStatus)status;
			}
		}
		
	} else
	{
		return kInvalidRetrieveLevel;
	}
	
	//	Set Affected SOP Class UID - RL & VS 12/20/01
	char affSOPClassUID[65];
	status = MC_Get_UID_From_MergeCOM_Service("STUDY_ROOT_QR_MOVE", affSOPClassUID, sizeof(affSOPClassUID));
	if (status == kNormalCompletion)
	{
		status = MC_Set_Value_From_String (m_cMoveRequestMessageID, MC_ATT_AFFECTED_SOP_CLASS_UID,  affSOPClassUID); 
		PrintOnError("Set Affected SOP Class UID", status);
		if (status != kNormalCompletion)
		{
			return (PxDicomStatus)status;
		}
		
	}
	
	//	Set Priorotiy - RL & VS 12/20/01
	unsigned short priority = 0x0001;	// HIGH
	status = MC_Set_Value_From_UShortInt(m_cMoveRequestMessageID, MC_ATT_PRIORITY, priority); 
	PrintOnError("Set Priority", status);
	if (status != kNormalCompletion)
	{
		return (PxDicomStatus)status;
	}
	
	
	//	Set dataset Type to 0x0102 Dataset Present - RL 12/20/01
	status = MC_Set_Value_From_UShortInt(m_cMoveRequestMessageID, MC_ATT_DATA_SET_TYPE, DATASET_PRESENT); 
	PrintOnError("Set Data Set Type", status);
	if (status != kNormalCompletion)
	{
		return (PxDicomStatus)status;
	}
	
	return kNormalCompletion;
}
//-----------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::HandleCMoveResponse(unsigned int& cMoveResponseStatus, int& nFailedInstances)
{
#ifdef _PROFILE
	ScopeTimer timer("HandleCMoveResponse");
#endif
	PxDicomStatus status;
	PxDicomStatus returnStatus = kNormalCompletion;

	status = (PxDicomStatus)MC_Get_Value_To_UInt(m_cMoveResponseID, MC_ATT_STATUS, &cMoveResponseStatus); 
	PrintOnError("Get resp status", status);
	if (status != kNormalCompletion)
	{
		return status;
	}
	
	if (cMoveResponseStatus == C_MOVE_CANCEL_REQUEST_RECEIVED)
	{
#ifdef _DEBUG
		LogMessage("CMove Cancel Request received by SCP (association %d)\n", m_currentServer.m_associationID);
#endif
		return kRetrieveAborted;
	} else if (cMoveResponseStatus != C_MOVE_PENDING_MORE_SUB_OPERATIONS)
	{
		if (cMoveResponseStatus != C_MOVE_SUCCESS_NO_FAILURES)
		{
			returnStatus = kRetrievePartialFailure;

			//	BEGIN - -- 2005.07.27 - There may be more info in the C-MOVE-RSP message that we can log
			//		especially if the reason for C-MOVE failure is present, it's extremely valuable.
			int errCmtStatus;
			char errorComment[kVR_LO];
			memset(errorComment, 0, sizeof(errorComment));
			errCmtStatus = MC_Get_Value_To_String(m_cMoveResponseID, MC_ATT_ERROR_COMMENT, sizeof(errorComment), errorComment);
			LogMessage("ERROR: CMOVE failed - CMOVE-RSP identifier contains:\n");
			LogMessage("ERROR:      (0000,0900) Status = %x\n",			cMoveResponseStatus);
			if (errCmtStatus == MC_NORMAL_COMPLETION && strlen(errorComment) > 0)
				LogMessage("ERROR:      (0000,0902) Error Comment = %s\n",	errorComment);
			// END - -- 2005.07.27

			// add by jwu 02/06/02 to test failed SOP instanceUID list 
			char        SOPInstanceUID[64 + 1]; 
			int FailedSOPInstanceUIDCount = nFailedInstances = 0; 
			status = (PxDicomStatus)MC_Get_Value_Count(m_cMoveResponseID, MC_ATT_FAILED_SOP_INSTANCE_UID_LIST, &nFailedInstances); 
			if(status != kNormalCompletion) 
			{ 
				LogMessage("CMove assoc %d Ended in error with status %d to get failed SOP InStanceUID\n", m_currentServer.m_associationID, cMoveResponseStatus); 
			} 
			FailedSOPInstanceUIDCount = nFailedInstances;
			if (FailedSOPInstanceUIDCount >0) 
			{ 
				FailedSOPInstanceUIDCount = FailedSOPInstanceUIDCount - 1; 
				status = (PxDicomStatus)MC_Get_Value_To_String(m_cMoveResponseID, MC_ATT_FAILED_SOP_INSTANCE_UID_LIST, sizeof(SOPInstanceUID), 
					SOPInstanceUID);  
				
				if(status != kNormalCompletion) 
				{ 
					LogMessage("CMove assoc %d Ended in error with status %d to get failed SOP InStanceUID List\n", m_currentServer.m_associationID, cMoveResponseStatus); 
				} 
				else 
				{
					LogMessage("CMove  FAILED_SOP_INSTANCE_UID = %s\n", SOPInstanceUID); 
				} 
			} 
			
			
			while(FailedSOPInstanceUIDCount) 
			{ 
				FailedSOPInstanceUIDCount = FailedSOPInstanceUIDCount -1; 
				status = (PxDicomStatus)MC_Get_Next_Value_To_String(m_cMoveResponseID, MC_ATT_FAILED_SOP_INSTANCE_UID_LIST, sizeof(SOPInstanceUID), 
					SOPInstanceUID);  
				if(status != kNormalCompletion) 
				{ 
					LogMessage("CMove assoc %d Ended in error with status %d to get count of failed SOP InStanceUID List\n", m_currentServer.m_associationID, cMoveResponseStatus); 
				} 
				else 
				{
					LogMessage("CMove  FAILED_SOP_INSTANCE_UID = %s\n", SOPInstanceUID); 
				} 
				
			} 
			// end by jwu 02/06/02 
			LogMessage("CMove assoc %d Ended in error with status %d\n", m_currentServer.m_associationID, cMoveResponseStatus);
		}
		return returnStatus;
	} 

	return returnStatus;
}

//----------------------------------------------------------------------------------------
//
PxDicomStatus CMoveSCU::SendCMoveCancelRequest()
{
	PxDicomStatus status;
	
	//
	//	Send the C-MOVE-CANCEL request message to the SCP - RL 12/14/01
	//
	int cancelMessageID = -1;
    status = (PxDicomStatus)MC_Open_Empty_Message(&cancelMessageID); 
	PrintOnError("Create Empty Message", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus)MC_Set_Service_Command(cancelMessageID, "STUDY_ROOT_QR_MOVE", C_CANCEL_MOVE_RQ); 
	PrintOnError("Set Model", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	//	Get Message ID from the active Request message so we can cancel the correct message RL & VS 12/20/01
	int requestToCancelID = -1;
    status = (PxDicomStatus)MC_Get_Value_To_Int(m_cMoveRequestMessageID, MC_ATT_MESSAGE_ID, &requestToCancelID); 
	PrintOnError("Get Message ID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    status = (PxDicomStatus)MC_Set_Value_From_Int(cancelMessageID, MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO, requestToCancelID); 
	PrintOnError("Set Message ID", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	//	Set dataset Type to Null - no dataset present - RL 12/20/01
    status = (PxDicomStatus)MC_Set_Value_From_UShortInt(cancelMessageID, MC_ATT_DATA_SET_TYPE, NO_DATASET_PRESENT); 
	PrintOnError("Set Data Set Type", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
	
	//	Send the cancel request
    status = (PxDicomStatus)MC_Send_Request_Message(m_currentServer.m_associationID, cancelMessageID); 
	PrintOnError("Send C-MOVE-CANCEL RQ", status);
    if (status != kNormalCompletion)
    {
        return status;
    }
	
    MC_Free_Message(&cancelMessageID);
	
#ifdef _DEBUG
	LogMessage("CMove Cancel request sent (association %d)\n", m_currentServer.m_associationID);
#endif
	//	return kRetrieveAborted;		
	return kRetrieveAborted;
}

//-----------------------------------------------------------------------------------------
//
bool CMoveSCU::ConnectCMove(DiCOMStore* pStore)
{
	CMoveSCU* pVlidicom = FromSerialKey(pStore);

	if(!pVlidicom || pVlidicom->m_bCmoveExit) return false;

	pVlidicom->m_connectedDiCOMStores.Add(pStore, 0);
	return true;
}

bool CMoveSCU::GetCacheDir(DiCOMStore* pStore, char* cacheDir, int len)
{
	CMoveSCU* pVlidicom = FromSerialKey(pStore);

	if(!pVlidicom) return false;

	strncpy(cacheDir, pVlidicom->m_localCache, len);
	return true;
}

//-----------------------------------------------------------------------------------------
//
bool CMoveSCU::HandoverImage(DiCOMStore* pStore, CPxDicomImage* iImage)
{
	CMoveSCU* pVlidicom = FromSerialKey(pStore);

	if(!pVlidicom) return false;

	if(iImage == 0)
	{
		pVlidicom->CancelTask(); // retrieve failed, cancell it
		return true;
	}

cs.Enter();
	if(pVlidicom->m_pImages)
		if ((pVlidicom->m_pSeriesUIDs[0].size() > 0)&& (pVlidicom->m_pSeriesUIDs[0].at(0) != "")&& 
			(strcmp(pVlidicom->m_pSeriesUIDs[0].at(0).c_str(), iImage->GetSeriesInstanceUID()) == 0))
				pVlidicom->m_pImages->push_back(iImage);
		else if(pVlidicom->m_pSeriesUIDs[0].size() <= 0)
			pVlidicom->m_pImages->push_back(iImage); //For some unknown case like just image
		else
		{
			cs.Leave();
			delete iImage;
			iImage = 0;
			return true;
		}
cs.Leave();
	
	int sliceMinVoxelValue =  99999;
	int sliceMaxVoxelValue = -99999;
	std::string sopClassUID (iImage->GetSOPClassUID());


	if (IsSOPClassUIDMR (sopClassUID))
	{
#if 0
		AQNetDataConverter converter;

		int totalSize = iImage->GetNumberOfColumns() * iImage->GetNumberOfRows();

		// -- & Junnan Wu: 2005.11.16
		// need to handle 8bit MR. 
		if (iImage->GetBitsStored() <= 8)
		{
			converter.FindMinMax((unsigned char*)iImage->GetImagePixels(), 
								totalSize, iImage->IsLittleEndian(),
								sliceMinVoxelValue, sliceMaxVoxelValue);
			
		}
		else
		{	
			// Calculate the slice Min and Max
			converter.FindMinMax((short *)iImage->GetImagePixels(), 
								 totalSize, iImage->IsLittleEndian(),
								 sliceMinVoxelValue, sliceMaxVoxelValue);
		}
		iImage->SetMinMaxVoxelValues (sliceMinVoxelValue, sliceMaxVoxelValue);
		iImage->SetIsLittleEndian (true);
#endif
	}
	return true;
}

//-----------------------------------------------------------------------------------------
//
bool CMoveSCU::DisconnectCMove(DiCOMStore* pStore)
{
	CMoveSCU* pVlidicom = FromSerialKey(pStore);

	if(!pVlidicom) return false;

	pVlidicom->m_connectedDiCOMStores.Remove(pStore);
	return true;
}

//-----------------------------------------------------------------------------------------
//
DicomServer& VLIDicomBase::GetCurrentServer(void)
{
	return m_currentServer;
}

#if 0
//-----------------------------------------------------------------------------------------
//
VLIDicom* VLIDicom::NewHeader(DiCOMStore* pStore, CPxDicomImage* iImage)
{
#ifdef _PROFILE
	ScopeTimer timer("NewHeader");
#endif
	std::string serialKey = MakeSerialKey(pStore->SeriesInstanceUID(), (m_keyOnAETitle)?
		pStore->ConnectInfo()->RemoteApplicationTitle:pStore->ConnectInfo()->RemoteIPAddress);

	VLIDicom* pVlidicom = SerialKeyPool.Get(serialKey);
	if(pVlidicom)
	{
/*		if(pVlidicom->m_cancel) 
			return 0;
*/
		// call back NewHeader(iImage)
		pVlidicom->m_connectedDiCOMStores.Add(pStore, 0);
cs.Enter();
		if(pVlidicom->m_pImages)
			pVlidicom->m_pImages->push_back(iImage);
cs.Leave();

	}
	return pVlidicom;
}

//-----------------------------------------------------------------------------------------
//
void VLIDicom::NewFrame(CPxDicomImage* iImage, unsigned char* iFrame)
{
#ifdef _PROFILE
	ScopeTimer timer("NewFrame");
#endif
/*	if(m_cancel) 
	{
		m_connectedDiCOMStores.Walk(CancellDiCOMStore);
		return;
	}
*/	int frame_index = iImage->AddFrame(iFrame);
//	Progress(-1, frame_index, 0, 0, kCMoveSCU);
	// call back NewFrame(iImage, iFrame, frame_index)
}

//-----------------------------------------------------------------------------------------
//
void VLIDicom::NewImage(DiCOMStore* pStore, CPxDicomImage* iImage)
{
#ifdef _PROFILE
	ScopeTimer timer("NewImage");
#endif
/*	if(m_cancel) 
		return;
*/	
	m_connectedDiCOMStores.Remove(pStore);

	if(iImage == 0)
	{
		CancelTask(); // retrieve failed, cancell it
		return;
	}
	else
	{
		int sliceMinVoxelValue =  99999;
		int sliceMaxVoxelValue = -99999;
		std::string sopClassUID (iImage->GetSOPClassUID());


		if (IsSOPClassUIDMR (sopClassUID))
		{
			AQNetDataConverter converter;

			int totalSize = iImage->GetNumberOfColumns() * iImage->GetNumberOfRows();

			// Calculate the slice Min and Max
			converter.ConvertMRToVoxel((short *)iImage->GetImagePixels(), 
										totalSize, iImage->IsLittleEndian(),
				                        sliceMinVoxelValue, sliceMaxVoxelValue);

			iImage->SetMinMaxVoxelValues (sliceMinVoxelValue, sliceMaxVoxelValue);

			iImage->SetIsLittleEndian (true);
			
		}
	}	//call back NewImage(iImage)
	return;
}
#endif


