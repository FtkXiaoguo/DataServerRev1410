/***********************************************************************
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------*/
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)
#pragma warning (disable: 4530)

#include <string>
#include <vector>

#include "IDcmLibApi.h"
#include "AuxData.h"

using namespace XTDcmLib;
//-----------------------------------------------------------------------------------------
//
bool AuxData::IsDeltaImagingReport(int iMessageID)
{
	char bodyPartExamined[kVR_CS];

	int status = MC_Get_Value_To_String(iMessageID, MC_ATT_BODY_PART_EXAMINED, sizeof(bodyPartExamined), bodyPartExamined);
	if (status == MC_NORMAL_COMPLETION)
		return strncmp(bodyPartExamined, kDeltaImagingRpt, sizeof(bodyPartExamined)) == 0;

	return false;
}

//-----------------------------------------------------------------------------------------
//
bool AuxData::CheckIsOldCaScore(int iMessageID)
{
	int	valueCount = 0;

	//	Check for old-style CaScore data in 0009 group
	MC_Get_Value_Count(iMessageID, TR_ATT_CA_SCORE, &valueCount);
	if (valueCount > 0)
		return true;
	
	MC_Get_Value_Count(iMessageID, TR_ATT_CA_REPORT, &valueCount);
	if (valueCount > 0)
		return true;

	return false;
}

//-----------------------------------------------------------------------------------------
//
bool AuxData::CheckHasAuxData(int iMessageID)
{
	int	valueCount = 0;

	//	Check for binary data in 0077 private group
	MC_Get_pValue_Count(iMessageID, TR_CREATOR, TR_GROUP, TR_ATT_FIRST_BINARY_DATA, &valueCount);
	if (valueCount > 0)
		return true;

	// -- 2005.11.07
	// We may have cases where we don't have binary data but the data is TeraRecon specific anyway
	MC_Get_pValue_Count(iMessageID, TR_CREATOR, TR_GROUP, TR_ATT_BINARY_DATA_NAME, &valueCount);
	if (valueCount > 0)
		return true;

	return CheckIsOldCaScore(iMessageID);
}

//-----------------------------------------------------------------------------------------
//
int AuxData::Init(int iMessageID, const char* iStudyUID, const char* iSeriesUID, const char* iSOPUID)
{
	m_auxData.Clear();
	m_auxReferencs.clear();
	m_isOldCaScore = false;
	m_hasAuxData = false;
	m_messageID = iMessageID;

	m_cofVertion = m_cofRevision = 0;
	
	int status;
	unsigned short msize;
	
	if(!CheckHasAuxData(iMessageID))
		return 1;

	MC_VR vr;
	int numVals;
	status = MC_Get_pAttribute_Info(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_NUMBER_OF_BINARY_DATA, &vr, &numVals);

	//	How many binary objects are in the message?
	status = MC_Get_pValue_To_UShortInt(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_NUMBER_OF_BINARY_DATA, &msize);
	if (status != MC_NORMAL_COMPLETION /*|| msize <= 0*/) // -- 2005.10.08 new cof can have 0 binnary data
	{
		//	Doesn't seem to have private data - what about old style CaScore?
		if (CheckIsOldCaScore(m_messageID))
			m_isOldCaScore = true;
		else
			return -1;
	}

	if (!m_isOldCaScore)
	{
		//	Some validation
		int valueCount = 0;
		status = MC_Get_pValue_Count(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_BINARY_DATA_TYPE, &valueCount);
		// even if we don't have binary data, we may still use the type. valueCount==msize is 
		// too strict
		if (status != MC_NORMAL_COMPLETION || (valueCount != msize && msize))
			return -2;

		status = MC_Get_pValue_Count(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_BINARY_DATA_NAME, &valueCount);
		if (status != MC_NORMAL_COMPLETION || (valueCount != msize && msize))
			return -3;
	}

	status = ExtractFirstOne();
	if (status != 0)
		return status;
	
	ASTRNCPY(m_auxData.m_auxStudyInstanceUID, iStudyUID);
	ASTRNCPY(m_auxData.m_auxSeriesInstanceUID, iSeriesUID);
	ASTRNCPY(m_auxData.m_auxSOPInstanceUID, iSOPUID);

	return 0;
}

//--------------------------------------------------------------------------------
// remove white spaces from both ends of a string 
// -- 11/29/2002
#include "rtvsutil.h"  // tcz use a common one from rtvsutil.h

//--------------------------------------------------------------------------------
// s will be used as a filename, replace illegal chars
// -- 2003-02-05

//	Rob & Vikram 09-16-03 - Added checks for unprintable chars
static inline bool IsBadChar(int c)
{
	return c < 37 || c > 126 || c=='*' || c == '/' || c=='\\' || c==':';
}

static char* FilterBadChar(char *s)
{
	char * p = s + strlen(s) -1;

	for ( ; p >= s; --p)
	{
		if (IsBadChar(*p)) *p = '-';
	}

	return s;
}

//-----------------------------------------------------------------------------------------
//
int AuxData::ExtractReferences()
{
	int status;
	m_auxReferencs.clear();
	
	// GL 4-26-2006 modify for volume information
	int itemCount =0;
	status = MC_Get_pValue_Count(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_ORIGINAL_SERIES_AND_STUDY_INSTANCE_UID, &itemCount);
	if (status != MC_NORMAL_COMPLETION || itemCount < 1)
		return 0;

	//chetan - don't need this check - verified with TC

#if 0
	bool hasVolumeID =  m_cofVertion > 2 || (m_cofVertion == 2 && m_cofRevision > 1) ;
#endif

	int numberOfSeries = itemCount / 2;
	m_auxReferencs.resize(numberOfSeries);

	char *pSeriesUID, *pStudyUID, *pVolumeID;
	for(int i=0; i < numberOfSeries; i++)
	{
		pSeriesUID = m_auxReferencs[i].m_referencedSeriesInstanceUID;
		pStudyUID = m_auxReferencs[i].m_referencedStudyInstanceUID;
		pVolumeID = m_auxReferencs[i].m_volumeID;

		if(i == 0)
			status = MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_ORIGINAL_SERIES_AND_STUDY_INSTANCE_UID, kVR_UI, pSeriesUID);
		else
			status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_ORIGINAL_SERIES_AND_STUDY_INSTANCE_UID, kVR_UI, pSeriesUID);

		if (status != MC_NORMAL_COMPLETION)
			return -10;

		status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_ORIGINAL_SERIES_AND_STUDY_INSTANCE_UID, kVR_UI, pStudyUID);
		if (status != MC_NORMAL_COMPLETION)
			return -11;

		/* remove spaces from UIDs - WS may generate UIDs with spaces at the end
		 * -- (2003-02-05)
		 */
		
		iRTVDeSpaceDe(pSeriesUID);
		iRTVDeSpaceDe(pStudyUID);

		//chetan - don't need this check - verified with TC
#if 0		
		if(!hasVolumeID)
		{
			pVolumeID[0] = 0;
			continue;
		}
#endif
		
		if(i == 0)
			status = MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_REFERENCED_VOLUME_ID, kVR_UI, pVolumeID);
		else
			status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_REFERENCED_VOLUME_ID, kVR_UI, pVolumeID);
	
		// -- 2006.05.10
		// can't fail this as there are private data that don't support volumeID
	//	if (status != MC_NORMAL_COMPLETION)
	//		return -11;

	}

	return 0;
}



//-----------------------------------------------------------------------------------------
//
//	Extract first binary datum
//
#include <assert.h>
int AuxData::ExtractFirstOne()
{
	int status;
	char typestring[kVR_UI];

	//	Get Date
	status = MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_DATE, sizeof(m_auxData.m_auxSeriesDate), m_auxData.m_auxSeriesDate);
	if (status != MC_NORMAL_COMPLETION)
		return -6;

	//	Get Time
	status = MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_TIME, sizeof(m_auxData.m_auxSeriesTime), m_auxData.m_auxSeriesTime);
	if (status != MC_NORMAL_COMPLETION)
		return -7;

	//	It's a normal aux data
	if (!m_isOldCaScore)
	{
		//	Get Type
		status = MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_BINARY_DATA_TYPE, 
			sizeof(typestring), typestring);
		if (status != MC_NORMAL_COMPLETION)
			return -4;

		//	Get Name
		status = MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_BINARY_DATA_NAME, 
			sizeof(m_auxData.m_name), m_auxData.m_name);
		if (status != MC_NORMAL_COMPLETION)
			return -5;

		//	Get Algorthm ID / ParameterHash
		status = MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_ALGORITHM_ID, 
			sizeof(m_auxData.m_parameterHash), m_auxData.m_parameterHash);

		// -- 2006.05.08 
		// Can't fail this and volumeID as scenes or templates etc. may not have this populated
	//	if (status != MC_NORMAL_COMPLETION)
	//		return -6;

		//	Get Volume ID hash
		status = MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_VOLUME_ID, 
			sizeof(m_auxData.m_volumesHash), m_auxData.m_volumesHash);

	//	if (status != MC_NORMAL_COMPLETION)
	//		return -7;

		status = MC_NORMAL_COMPLETION;// tcz

		FilterBadChar(iRTVDeSpaceDe(m_auxData.m_name));

	}
	//	It's an old-style caScore file
	else
	{
		strcpy(typestring, "WS_Ca_Score");
		strcpy(m_auxData.m_name, m_auxData.m_auxSeriesDate);
		strcat(m_auxData.m_name, "_");
		strcat(m_auxData.m_name, m_auxData.m_auxSeriesTime);
	}

	m_auxData.m_type = TypeStringToEnum(typestring);
	strcpy(m_auxData.m_subtype, typestring);

	if (m_auxData.m_type ==  AuxDataInfo::kNone)
		return -101;

	

	/* -- 08-AUG-2002 */
	/* For template, we want typeName in the database, which is in seriesDescription */	
	if (m_auxData.m_type == AuxDataInfo::kTemplate)
	{
		MC_Get_Value_To_String(m_messageID, MC_ATT_SERIES_DESCRIPTION, sizeof(m_auxData.m_subtype), m_auxData.m_subtype);
	}
	else
	{
		status = ExtractAdditionalInformation();
		if (status != MC_NORMAL_COMPLETION)
			return status;
	}
	
	if (m_auxData.m_type != AuxDataInfo::kTemplate && !m_isOldCaScore)
	{
		status = ExtractReferences();
		if (status != 0)
			return -12;
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
// -- 2005.11.07
// extract additional information - among them: anatomy, processType, processName
int AuxData::ExtractAdditionalInformation()
{
	int valueCount = 0, status = MC_NORMAL_COMPLETION;

	MC_Get_pValue_Count(m_messageID, TR_CREATOR, TR_GROUP, TR_ATT_ADDITIONAL_INFORMATION, &valueCount);
	if (valueCount > 0)
	{
		// -- 2005.11.13
		// we're supposedly to have more than 5 attributes here but some files may not have them
		// all. Be lenient. We do require 3
		// ResultType\ProcessType\Anatomy\CoFVersion\CoFRevision\ModuleName\ModuleVersion\CompanyName\ProductName\ProductVersion
 
		char versionInfo[32];
		if (valueCount >= 3)
		{
			int extracted = 0;

			// first one is ResultType, we don't need that
			MC_Get_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
									sizeof(m_auxData.m_subtype), m_auxData.m_subtype);
			
			// next is the processType
			status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
												sizeof(m_auxData.m_processType), m_auxData.m_processType);
			
			// next is the anatomy
			status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
												 sizeof(m_auxData.m_subtype), m_auxData.m_subtype);
			
			if (status != MC_NORMAL_COMPLETION)
				return -7;
			
			extracted = 3;
			
			if (extracted < valueCount)
			{
				// next is the version information of the COF
				// skip two
				status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
					sizeof(versionInfo), versionInfo);

				if (status == MC_NORMAL_COMPLETION)
					m_cofVertion = atol(versionInfo);
				extracted++;
			}
			
			if (extracted < valueCount)
			{
				status = MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
					sizeof(versionInfo), versionInfo);
				if (status == MC_NORMAL_COMPLETION)
					m_cofRevision = atol(versionInfo);

				extracted++;
			}
			
			if (extracted < valueCount)
			{
				// this one the process name
				/*status = */MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
					sizeof(m_auxData.m_processName), m_auxData.m_processName);
				extracted++;

			}

// tcz 2006.05.04 moved volumeshash and parameter hash to top level tags
#if 0 
			m_auxData.m_volumesHash[0] = 0;
			m_auxData.m_parameterHash[0] = 0;
			// GL 4-26-2006 Add PE input information
			if ( (m_cofVertion > 2 || (m_cofVertion == 2 && m_cofRevision > 1)) &&  extracted < valueCount)
			{
				/*status = */MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
					sizeof(m_auxData.m_volumesHash), m_auxData.m_volumesHash);
				extracted++;

				/*status = */MC_Get_Next_pValue_To_String(m_messageID, TR_CREATOR, TR_GROUP,TR_ATT_ADDITIONAL_INFORMATION,
					sizeof(m_auxData.m_parameterHash), m_auxData.m_parameterHash);
				extracted++;
			}
#endif
		
		}
		else
		{
			assert(valueCount >= 3);
			return -8;
		}
	}

	return status;
}

//-----------------------------------------------------------------------------------------
//
//
//tcz 2006.05.04 will need to read from registry
static const char* kCustom3rdParty1 = "INSPACEBOOKMARKS"; 

int AuxData::TypeStringToEnum(const char* iTypeString)
{
	// -- 09/23/2002
	// the typeString is stored in CS VR which should not contain
	// small case letters.
	// We check case-insensitive so some old WS files can be
	// read correctly
	if (stricmp(iTypeString, "WS_3D_SCENE") == 0)
		return AuxDataInfo::kScene;
	else if (stricmp(iTypeString,"WS_3D_Template") == 0)
		return AuxDataInfo::kTemplate;
	else if (stricmp(iTypeString,"WS_Ca_Score") == 0)
		return AuxDataInfo::kCaScore;
	else if (stricmp(iTypeString,"WS_Ca_Report") == 0)
		return AuxDataInfo::kCaReport;
	else if (stricmp(iTypeString,"WS_Interactive_Report") == 0)
		return AuxDataInfo::kInteractiveReport;
	else if (stricmp(iTypeString,"TR_CUSTOM") == 0)
		return AuxDataInfo::kCustom;
	// tcz 2004.11.16 new feature for v1.5
	else if (stricmp(iTypeString,"AQNET_SCENE") == 0)
		return AuxDataInfo::kNetScene;
	// tcz 2005.03.24 new feature for v1.5.2
	else if (stricmp(iTypeString,"WS_FINDING") == 0 || 
		     stricmp(iTypeString,"CAD_FINDING") == 0 ||
			 stricmp(iTypeString,"TR_FINDING") == 0)
		return AuxDataInfo::kFindings;
	// the COF format tcz 2005.11.04
	else if (stricmp(iTypeString,"PARAENHANCING") == 0)
	{
		return AuxDataInfo::kParametricMapEnhancingRatio;
	}
	else if (stricmp(iTypeString,"PARAUPTAKE") == 0)
	{
			return AuxDataInfo::kParametricMapUptake;
	}
	else if (stricmp(iTypeString,"MASK") == 0)
	{
		return AuxDataInfo::kMask;
	}
	else if (stricmp(iTypeString,"CENTERLINETREE") == 0)
	{
		return AuxDataInfo::kCenterline;
	}
	else if (stricmp(iTypeString,"CANDIDATES") == 0)
	{
		return AuxDataInfo::kCandidates;
	}
	else if (stricmp(iTypeString, "RIGIDTRANSFORM") == 0) // tcz 2006.03.22
	{
		return AuxDataInfo::kRigidTransform;
	}
	// end of tcz 2005.11.04

	// -- 2006.05.04
	// For third party integration
	else if (stricmp(iTypeString, kCustom3rdParty1) == 0)
	{
		return AuxDataInfo::kCustom3rdParty1;
	}
	else if (stricmp(iTypeString, "AUXCUSTOM1") == 0)
	{
		return AuxDataInfo::kCustom3rdParty1;
	}
	else if (stricmp(iTypeString,"RESAMPLEDVOLUME") == 0 ) // SH, 2006.09.25
	{
		return AuxDataInfo::kResampledVolume;
	}
	else
	{
		return AuxDataInfo::kNone;
	}
}

//--------------------------------------------------------------------
// -- 2006.05.30 How we generate the filename
std::string AuxData::ComposeAuxDataFileName(int dbKey, const char* iAuxDataName, const char* iSuffix)
{
	char buf[256];

	// can't remember the reason for this check, but it was in the original code, keep it
	if (strrchr(iAuxDataName,'.') == 0 || strstr(iAuxDataName,iSuffix) == 0)
	{
		_snprintf(buf, sizeof buf, "%d_%s%s", dbKey, iAuxDataName, iSuffix);
	}
	else
	{
        _snprintf(buf, sizeof buf, "%d_%s", dbKey, iAuxDataName);
	}

	buf [sizeof buf - 1] = '\0';

	return std::string(buf);
}