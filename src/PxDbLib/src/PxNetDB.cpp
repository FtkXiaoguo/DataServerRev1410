/***********************************************************************
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */
//start from DBToolsWeb.cpp 1.113
#include "PxNetDB.h"

#include "AqCore/TRPlatform.h"
#include "AqCore/AqTString.h"
#include "CheckMemoryLeak.h"

int CPxDcmDB::m_sDBOption;

using namespace std;

CPxDcmDB::CPxDcmDB()
{
}

CPxDcmDB::~CPxDcmDB()
{
}


// general patient/study/series queries. We truncate description to 68 chars
/*
struct pRTVSDicomInformation
{
	uint32		m_validFields;
	DICOMName	m_patientName;
	DICOMUID	m_patientUID;
	char		m_patientSex[4];
	DICOMDate	m_patientBirthDate;
	DICOMNUM	m_studyCount;

	DICOMUID	m_studyUID;
	DICOMNUM	m_studyNumber;	// same as studyID
	DICOMDate	m_studyDate;	// can contain ranges yyyymmdd-yyyymmdd
	DICOMDate	m_studyTime;
	DICOMUID	m_accessionNumber;
	DICOMName	m_radiologistName;
	DICOMName	m_physicianName;
	char		m_modalitiesInStudy[16];
	DICOMName	m_studyDescription;
	DICOMNUM	m_seriesCount;
	DICOMNUM	m_imagesInStudy;

	DICOMUID	m_seriesUID;
	DICOMNUM	m_seriesNumber;	
	char		m_modality[8];
	DICOMNUM	m_imageCount;
	DICOMName	m_seriesDescription;

	DICOMNUM	m_scanType;

	uint16		m_auxDataStatus;  // read, unread, interactiveReport etc 
	uint16		m_pad;

	// the following really should be DICOM Source  for now, cheat a bit
	// to reduce the data size
	 
	uint16		m_type;
	uint16		m_port;
	char		m_AETitle[20];
	char		m_IPAddress[20];
	char		m_hostname[32];

};
*/

//#136 2021/01/12 N.Furutsuki unicode version
static void MakeUserStudiesFilter(const pRTVSDicomInformation& iKey, AqUString& cond, int iUserID/*=0*/)
{
	DICOMData filter;

	memset(&filter, 0, sizeof(DICOMData));
	
	ASTRNCPY(filter.m_characterSet, iKey.m_characterSet);
	ASTRNCPY(filter.m_studyInstanceUID, iKey.m_studyUID);
	ASTRNCPY(filter.m_patientsName, iKey.m_patientName);
	ASTRNCPY(filter.m_patientID, iKey.m_patientID);
	ASTRNCPY(filter.m_patientsBirthDate, iKey.m_patientBirthDate);
	ASTRNCPY(filter.m_patientsSex, iKey.m_patientSex);	
	//iKey.m_patientsAge;
	ASTRNCPY(filter.m_studyDate, iKey.m_studyDate);
	ASTRNCPY(filter.m_studyTime, iKey.m_studyTime);
	ASTRNCPY(filter.m_accessionNumber, iKey.m_accessionNumber);
	ASTRNCPY(filter.m_studyID, iKey.m_studyID);
	ASTRNCPY(filter.m_radiologistName, iKey.m_radiologistName);
	ASTRNCPY(filter.m_referringPhysiciansName, iKey.m_physicianName);
	ASTRNCPY(filter.m_modalitiesInStudy, iKey.m_modalitiesInStudy); // -- -5/13/03 Typo fix. 
	ASTRNCPY(filter.m_studyDescription, iKey.m_studyDescription);
	//ASTRNCPY(filter, iKey.m_numberOfStudyRelatedInstance
	//ASTRNCPY(filter, iKey.m_characterSet[ kVR_CS ];	

	ASTRNCPY(filter.m_seriesInstanceUID, iKey.m_seriesUID);
	//iKey.m_seriesNumber;				
	ASTRNCPY(filter.m_seriesDescription, iKey.m_seriesDescription);
	ASTRNCPY(filter.m_modality, iKey.m_modality);
	ASTRNCPY(filter.m_bodyPartExamined,iKey.m_bodyPartExamined); // -- 06/04/2003
	//iKey.m_viewPosition
	//iKey.m_numberOfSeriesRelatedInstanc
	//filter.m_readStatus = iKey.m_auxDataStatus;
	filter.m_readStatus = CPxDcmDB::RTVSReadStatusToDB(iKey.m_readStatus);

	//#136 2021/01/12 N.Furutsuki unicode version
	CPxDB::MakeUserStudiesFilterU(filter, cond, iUserID);
}


//-----------------------------------------------------------------------------------------
int	CPxDcmDB::OpenUserStudy(SQA& iSqa, int iGroupID, int iUserID, const pRTVSDicomInformation& iFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::OpenUserStudy start\n");

	const char* pUserStudyView;

	if(getLocalDBType() == kDBType_SQLite){
		pUserStudyView = "SELECT DISTINCT ReadStatus, StudyInstanceUID, PatientsName, "
				"PatientID, PatientsBirthDate, PatientsSex, StudyDate, StudyTime, AccessionNumber, StudyID," //tczhao 2005.07.20 StudyID added
				"ReadingPhysiciansName, ReferringPhysiciansName, ModalitiesInStudy, StudyDescription,"
				"NumberOfStudyRelatedSeries, NumberOfStudyRelatedInstances, CharacterSet, AuxRefStudyUID FROM UserStudyView";
	}else{
		pUserStudyView = "SELECT DISTINCT ReadStatus, StudyInstanceUID, PatientsName, "
				"PatientID, PatientsBirthDate, PatientsSex, StudyDate, StudyTime, AccessionNumber, StudyID," //tczhao 2005.07.20 StudyID added
				"ReadingPhysiciansName, ReferringPhysiciansName, ModalitiesInStudy, StudyDescription,"
				"NumberOfStudyRelatedSeries, NumberOfStudyRelatedInstances, CharacterSet, HasAux FROM UserStudyView";
	}

	AqString strSQL;
	AqString filter = "";

	if(CanGroupAccessAllData(iGroupID))
		// don't use group constrain
		strSQL.Format("%s WHERE UserID=%d", pUserStudyView, iUserID);

	else
		strSQL.Format("%s WHERE (GroupID=%d OR GroupID=%d)AND UserID=%d", pUserStudyView, iGroupID, 
				(c_publicGroup.m_groupUID != 0)?c_publicGroup.m_groupUID:iGroupID, iUserID);
 
	//#136 2021/01/12 N.Furutsuki unicode version
	//////////////////  
	// for unicode filed
	//////////////////

	AqUString filterTemp = L"";
	::MakeUserStudiesFilter(iFilter, filterTemp, iUserID);

	//make unicode SQL
	AqUString UstrSQL;
	UstrSQL.ConvertUTF8(strSQL);

	if (filterTemp.GetLength() > 0){
		UstrSQL.Format(L"%s AND %s", UstrSQL, filterTemp);
	}
	//Sqa command	 
	iSqa.setOptions(kDBAsyncExecute);
	iSqa.FormatCommandText(L"%s", UstrSQL);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -AQNetDB::OpenUserStudy with: %s\n", strSQL);
	int retcd = SQLExecuteBegin(iSqa); // do AsyncExecute
	retcd = iSqa.MoveFirst(); if (retcd != kOK)  return retcd;
 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::OpenUserStudy end\n");
	return kOK;
}

int	CPxDcmDB::GetUserStudy(SQA& iSqa, pRTVSDicomInformation& oVal, const pRTVSDICOMDevice& source)
{
	pRTVSDicomInformation* pInfo = &oVal;

	memset(pInfo, 0, sizeof *pInfo);
	int readStatus = 0;

	iSqa.SetIndex(0);
	SQL_GET_INT(readStatus, iSqa);
	pInfo->m_readStatus |= DBReadStatusToRTVS(readStatus);

	SQL_GET_STR(pInfo->m_studyUID, iSqa);
	SQL_GET_STR(pInfo->m_patientName, iSqa);
	SQL_GET_STR(pInfo->m_patientID, iSqa);
	SQL_GET_STR(pInfo->m_patientBirthDate, iSqa);
	SQL_GET_STR(pInfo->m_patientSex, iSqa);
	SQL_GET_STR(pInfo->m_studyDate, iSqa);
	SQL_GET_STR(pInfo->m_studyTime, iSqa);
	SQL_GET_STR(pInfo->m_accessionNumber, iSqa);
	SQL_GET_STR(pInfo->m_studyID,iSqa);		//tczhao 2005.07.20 Added studyID
	SQL_GET_STR(pInfo->m_radiologistName, iSqa);
	SQL_GET_STR(pInfo->m_physicianName, iSqa);
	SQL_GET_STR(pInfo->m_modalitiesInStudy, iSqa);
	SQL_GET_STR(pInfo->m_studyDescription, iSqa);
	iSqa.getDataInt(pInfo->m_seriesCount, sizeof(pInfo->m_seriesCount));
	iSqa.getDataInt(pInfo->m_imagesInStudy, sizeof(pInfo->m_imagesInStudy));
	SQL_GET_STR(pInfo->m_characterSet, iSqa); // T.C 2006.02.17 Internationalization needs this
	// used in image server as an indicator as has aux data
	if(getLocalDBType() == kDBType_SQLite){
		char _str_temp[128];
		_str_temp[0] = 0;
		SQL_GET_STR(_str_temp, iSqa);
		if(_str_temp[0]!=0){
			pInfo->m_auxDataInfo |= kpRTVSAuxScene; 
		}
	}else{
		if(iSqa.getDataInt())
			pInfo->m_auxDataInfo |= kpRTVSAuxScene; 
	}
	
	pInfo->m_source = source;

	return kOK;
}

int	CPxDcmDB::GetUserStudies(int iGroupID, int iUserID, vector<pRTVSDicomInformation>& oVal, const pRTVSDicomInformation& iFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserStudies start\n");
	
	SQA sqa(getLocalDBType());
	int retcd = OpenUserStudy(sqa, iGroupID, iUserID, iFilter);

	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;

	oVal.resize(size);
	 	
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		retcd = GetUserStudy(sqa, oVal[index++], iFilter.m_source);
		if(retcd == kOK)
			retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserStudies end\n");
	return kOK;
}

//-----------------------------------------------------------------------------------------
// iGroupID is not used, need fix later on
//
int	CPxDcmDB::GetUserSeries(int iGroupID, int iUserID, vector<pRTVSDicomInformation>& oVal, const pRTVSDicomInformation& iFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserSeries start\n");
	
	AqString	strSQL;

	AqUString filter = L"";
	if(CanGroupAccessAllData(iGroupID))
	{
		// no group constrain
		strSQL.Format("SELECT DISTINCT ReadStatus, AuxMask, SeriesInstanceUID, SeriesNumber, "
			"SeriesDescription, Modality, BodyPartExamined, NumberOfSeriesRelatedInstances, "
			"DATEDIFF(Day,GETDATE(),HoldToDate), seriesDate, seriesTime "
			"FROM UserSeriesViewX WHERE UserID=%d ", iUserID);

	}
	else
	{
		// add userID constrain to get user status 
		strSQL.Format("SELECT DISTINCT ReadStatus, AuxMask, SeriesInstanceUID, SeriesNumber, "
			"SeriesDescription, Modality, BodyPartExamined, NumberOfSeriesRelatedInstances, "
			"DATEDIFF(Day,GETDATE(),HoldToDate), seriesDate, seriesTime "
			"FROM UserSeriesViewX WHERE (GroupID=%d OR GroupID=%d) AND UserID=%d ",
			iGroupID, (c_publicGroup.m_groupUID != 0)?c_publicGroup.m_groupUID:iGroupID, iUserID);
 	}

	//make unicode SQL
	AqUString UstrSQL;
	UstrSQL.ConvertUTF8(strSQL);

	::MakeUserStudiesFilter(iFilter, filter, iUserID);
	if (filter.GetLength() > 0){
		UstrSQL = UstrSQL + L" AND " + filter;
	}

	SQA sqa(getLocalDBType());
	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserSeries with: %s\n", strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;

	oVal.resize(size);

	pRTVSDicomInformation* pInfo;
	int index = 0;
	int auxDatatype;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
			
		SQL_GET_INT(pInfo->m_readStatus, sqa);
		pInfo->m_readStatus = DBReadStatusToRTVS(pInfo->m_readStatus);

		SQL_GET_INT(auxDatatype, sqa);
		// tcz 2005.11.07 readStatus and auxDatainfo is separated now. use assign
	//	pInfo->m_auxDataInfo |= AuxDataTypeToRTVMask(auxDatatype);
		pInfo->m_auxDataInfo = AuxDataTypeToRTVMask(auxDatatype);

		SQL_GET_STR(pInfo->m_seriesUID, sqa);
		sqa.getDataInt(pInfo->m_seriesNumber, sizeof(pInfo->m_seriesNumber));
		SQL_GET_STR(pInfo->m_seriesDescription, sqa);
		SQL_GET_STR(pInfo->m_modality, sqa);
		SQL_GET_STR(pInfo->m_bodyPartExamined, sqa); // -- 06/04/2003
		sqa.getDataInt(pInfo->m_imageCount, sizeof(pInfo->m_imageCount));

		SQL_GET_INT(pInfo->m_daysToLock, sqa);
		SQL_GET_STR(pInfo->m_seriesDate, sqa);
		SQL_GET_STR(pInfo->m_seriesTime, sqa);

		//m_offlineFlag
		//m_modifyTime

		pInfo->m_source = iFilter.m_source;
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserSeries end\n");
	 
	return kOK;
}

//-----------------------------------------------------------------------------------------
//
int	CPxDcmDB::GetUserSeriesAuxStauts(int iUserID, pRTVSDicomInformation* ioVal, int iSize)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserSeriesAuxStauts start\n");
	AqString	strSQL;

	SQA sqa(getLocalDBType());
	pRTVSDicomInformation* pInfo;
	int i, auxDatatype, retcd;
	for( i=0; i<iSize; i++)
	{
		pInfo = &(ioVal[i]);
		// get series read status again for this user
		pInfo->m_readStatus = kpRTVSIsUnread;  //default to unread
		strSQL.Format("SELECT PxDcmDB.dbo.GetSeriesReadStatus(%d, '%s'), "
			"PxDcmDB.dbo.GetSeriesAuxMask('%s')", iUserID, pInfo->m_seriesUID, pInfo->m_seriesUID);

		sqa.SetCommandText(strSQL);
		retcd = SQLExecuteBegin(sqa) || sqa.MoveFirst();
		if(retcd != kOK)
			continue;

		SQL_GET_INT(auxDatatype, sqa);
		if(auxDatatype) // get the read stauts
			pInfo->m_readStatus = DBReadStatusToRTVS(auxDatatype);

		SQL_GET_INT(auxDatatype, sqa); // get the aux data type
		pInfo->m_auxDataInfo |= AuxDataTypeToRTVMask(auxDatatype);
	}
	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserSeriesAuxStauts end\n");
	 
	return kOK;
}


int	CPxDcmDB::AuxDataTypeToRTVMask(int iType)
{
	int mask = 0;

	if(iType & AuxDataInfo::kIsAux)
		mask |= kpRTVSIsAux;

	if(iType & AuxDataInfo::kScene)
		mask |= kpRTVSAuxScene;

	if(iType & AuxDataInfo::kTemplate)
		mask |= kpRTVSAuxTemplate;	

	if(iType & AuxDataInfo::kCaScore)
		mask |= kpRTVSAuxCaScore;	
		
	if(iType & AuxDataInfo::kCustom)
		mask |= kpRTVSAuxCustom;	
		
	if(iType & AuxDataInfo::kInteractiveReport)
		mask |= kpRTVSAuxIR;	
		
	if(iType & AuxDataInfo::kCaReport)
		mask |= kpRTVSAuxCaReport;

	// tcz 2004.11.15 new feature for v1.5
	if(iType & AuxDataInfo::kNetScene)
		mask |= kpRTVSAuxNetScene;

	if (iType & AuxDataInfo::kFindings)
		mask |= kpRTVSAuxFindings;

	// -- 2005.11.21 new COFs
	if (iType & AuxDataInfo::kMask)
		mask |= kpRTVSAuxMask;

	if (iType & AuxDataInfo::kCenterline)
		mask |= kpRTVSAuxCenterline;

	if (iType & AuxDataInfo::kCandidates)
		mask |= kpRTVSAuxCandidates;

	if (iType & AuxDataInfo::kRigidTransform)
		mask |= kpRTVSAuxRigidTransform;

	if (iType & AuxDataInfo::kParametricMapEnhancingRatio)
		mask |= kpRTVSAuxParametricMapEnhancingRatio;

	if (iType & AuxDataInfo::kParametricMapUptake)
		mask |= kpRTVSAuxParametricMapUptake;
	// end of 2005.11.21 new COF

	// TCZ 2006.05.08 for third party integration
	if (iType & AuxDataInfo::kCustom3rdParty1)
		mask |= kpRTVSAuxCustom3rdParty1;

	// SH, 2006.09.25
	if (iType & AuxDataInfo::kResampledVolume)
		mask |= kpRTVSAuxResampledVolume;


	return mask;
}

int	CPxDcmDB::DBReadStatusToRTVS(int iReadstatus)
{
	int readstatus = 0;

	if(iReadstatus & kDBIsUnknown)
		readstatus |= kpRTVSIsUnknown;
	
	if(iReadstatus & kDBIsUnread)
		readstatus |= kpRTVSIsUnread;

	if(iReadstatus & kDBIsPartiallyRead)
		readstatus |=kpRTVSIsPartiallyRead;

	if(iReadstatus & kDBIsRead)
		readstatus |= kpRTVSIsRead;

	if(iReadstatus & kDBIsHiden)
		readstatus |= kpRTVSIsHiden;

	return readstatus;
}

int	CPxDcmDB::RTVSReadStatusToDB(int iReadstatus)
{
	int readstatus = 0;

	if(iReadstatus & kpRTVSIsUnknown)
		readstatus |= kDBIsUnknown;
	
	if(iReadstatus & kpRTVSIsUnread)
		readstatus |= kDBIsUnread;

	if(iReadstatus & kpRTVSIsPartiallyRead)
		readstatus |= kDBIsPartiallyRead;

	if(iReadstatus & kpRTVSIsRead)
		readstatus |= kDBIsRead;

	if(iReadstatus & kpRTVSIsHiden)
		readstatus |= kDBIsHiden;

	return readstatus;
}

//-----------------------------------------------------------------------------
int CPxDcmDB::GetNumberOfInstancesInSeries(const char* iSeriesUID)
{
	SQA sqa(getLocalDBType());
	
	sqa.setOptions(0);
	
	AqString strSQL;
	strSQL.Format("SELECT serieslevelid FROM dbo.serieslevel where seriesinstanceuid='%s'", iSeriesUID);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	
	int iSeriesLevelID = 0;
	if(retcd == kOK && sqa.MoveFirst()==kOK) 
	{
		SQL_GET_INT(iSeriesLevelID, sqa);
	}
	SQLExecuteEnd(sqa);
	strSQL.Format("SELECT count(*) FROM dbo.instanceview WHERE dbo.instanceview.serieslevelid=%d", iSeriesLevelID);
	sqa.SetCommandText(strSQL);
	retcd = SQLExecuteBegin(sqa);
	
	int instanceCount = 0;
	
	if(retcd == kOK && sqa.MoveFirst()==kOK) 
	{	
		SQL_GET_INT(instanceCount, sqa);
	}
	
	SQLExecuteEnd(sqa);
	
	return instanceCount;
}

//----------------------------------------------------------------------------
#include "rtvsutil.h"
#include "rtvsfileGuard.h"
static int LoadDBInfoFromDisk(std::string& iSeriesPath, int instanceCount, vector<AqDICOMImageInfo>& oVal)
{
	char sortFile[300];
	snprintf(sortFile,sizeof sortFile,"%s/cache.sort", iSeriesPath.c_str());
	sortFile[sizeof sortFile - 1] = '\0';

	HANDLE handle = CreateFile(sortFile, GENERIC_READ, FILE_SHARE_READ,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL/*FILE_FLAG_NO_BUFFERING*/, NULL);
	
	if (!instanceCount || handle == INVALID_HANDLE_VALUE)
		return -1;
	
	iRTVSFileGuard hguard;
	hguard.AddHandle(handle);

	int size = instanceCount * sizeof AqDICOMImageInfo, headerSize  = 2 * sizeof (int);

	iRTVSAlloc<char> mem(size + headerSize);
	char * data = mem;
	if (!data)
		return -3;

//	int fileSize = GetFileSize(handle,0);
		
	unsigned long bytesRead = 0;
	ReadFile(handle, data, size + headerSize, &bytesRead, 0);
	hguard.CloseOpenHandles();

	int *header = (int *)data;

	if (bytesRead != size + headerSize || header[0] != instanceCount || header[1] != sizeof AqDICOMImageInfo )
	{
		DeleteFile(sortFile);
		return -2;
	}

	// copy the stuff
	AqDICOMImageInfo *sortInfo = (AqDICOMImageInfo *) (data + headerSize);
	AqDICOMImageInfo *info;
	oVal.resize(instanceCount);
	for(int i = 0; i < instanceCount; i++)
	{
		info = &oVal[i];
		*info = sortInfo[i];	
	}

	return 0;
}

//-----------------------------------------------------------------------------
int WriteDBInfoToDisk(std::string& iSeriesPath, int instanceCount, vector<AqDICOMImageInfo>& iVal)
{
	// don't bother with saving if number of images is small. Since we're not pre-generating this,
	// disk cache make sense for 4D only [which tend to have large number of images
	if (iVal.size() < 500)
		return 0;

	char sortFile[300];
	snprintf(sortFile,sizeof sortFile,"%s/cache.sort", iSeriesPath.c_str());
	sortFile[sizeof sortFile - 1] = '\0';
	
	int size = instanceCount * sizeof AqDICOMImageInfo, headerSize  = 2 * sizeof (int);
	iRTVSAlloc<char> mem(size + headerSize);
	char * data = mem;
	if (!data)
		return -3;
	
	
	HANDLE handle =  CreateFile(sortFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (instanceCount <=0 || handle == INVALID_HANDLE_VALUE)
		return -1;
	
	iRTVSFileGuard hguard;
	hguard.AddHandle(handle);
	
	int *header = (int *)data;
	header[0] = instanceCount;
	header[1] = sizeof AqDICOMImageInfo;
	
	AqDICOMImageInfo *sortInfo = (AqDICOMImageInfo *) (data + headerSize);
	for ( int i = 0; i < instanceCount; i++)
	{
		sortInfo[i] = iVal[i];
	}
	
	unsigned long bytesWritten = 0;
	WriteFile(handle,data, size + headerSize, &bytesWritten, 0);
	
	if (bytesWritten != size + headerSize)
	{
		hguard.CloseOpenHandles();
		DeleteFile(sortFile);
		return -1;
	}
	
	return 0;
	
}

//-----------------------------------------------------------------------------
int CPxDcmDB::GetSeriesSortInfo(const char* iSeriesUID, vector<AqDICOMImageInfo>& oVal, int iNInstace)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::GetUserSeries start\n");

	if(!iSeriesUID || !iSeriesUID[0])
		return kParameterError;

	std::string seriesPath;

	// check if we have disk cache
	if (CPxDcmDB::m_sDBOption & CPxDcmDB::kLoadSortInfoFromDisk && GetSeriesPath(iSeriesUID, seriesPath) == kOK && !seriesPath.empty())
	{
		int instanceCount = GetNumberOfInstancesInSeries(iSeriesUID);
		
		if (LoadDBInfoFromDisk(seriesPath, instanceCount, oVal) >= 0)
			return kOK;
	}

	SQA sqa(getLocalDBType()); sqa.setOptions(kDBAsyncExecute);

	AqString topStr= "";
	if(iNInstace > 0)
	{
		topStr.Format(" Top %d ", iNInstace);
	}
	sqa.FormatCommandText("SELECT %s SOPClassUID, InstanceView.*, Manufacturer FROM SeriesLevel JOIN InstanceView " 
		"ON SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID JOIN SOPClassUIDs ON "
		"InstanceView.SOPClassID = SOPClassUIDs.SOPClassID WHERE SeriesInstanceUID=? "
		"And SeriesLevel.Status <> -2 Order by InstanceNumber", topStr);

	sqa.AddParameter(iSeriesUID);
	int retcd = SQLExecuteBegin(sqa);

	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;

	oVal.resize(size);

	AqDICOMImageInfo* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		memset(pInfo, 0, sizeof(AqDICOMImageInfo));
		
		//SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		//pInfo->m_modality = iModality;
		SQL_GET_STR(pInfo->m_SOPClassUID, sqa);
		sqa.SkipData(); //InstanceLevelID
		SQL_GET_STR(pInfo->m_SOPInstanceUID, sqa);
				
		sqa.SkipData(); //SeriesLevelID
		sqa.SkipData(); //SOPClassID

		SQL_GET_INT(pInfo->m_storedTransferSyntax, sqa);

		SQL_GET_INT(pInfo->m_instanceNumber, sqa);
		SQL_GET_INT(pInfo->m_rows, sqa);
		SQL_GET_INT(pInfo->m_columns, sqa);
		SQL_GET_INT(pInfo->m_numberOfFrames, sqa);

		SQL_GET_STR(pInfo->m_imageTypeTokens, sqa);
 
//#1 2012/02/10 K.Ko reduce the instanceLevel's field
/*
		SQL_GET_INT(pInfo->m_bitsAllocated, sqa);
		SQL_GET_INT(pInfo->m_bitsStored, sqa);
		SQL_GET_INT(pInfo->m_highBit, sqa);
		SQL_GET_INT(pInfo->m_pixelRepresentation, sqa);
		SQL_GET_INT(pInfo->m_photometricInterpretation, sqa);
		SQL_GET_INT(pInfo->m_planarConfiguration, sqa);

		SQL_GET_FLOAT(pInfo->m_windowWidth, sqa);
		SQL_GET_FLOAT(pInfo->m_windowCenter, sqa);
		SQL_GET_INT(pInfo->m_smallestPixelValue, sqa);
		SQL_GET_INT(pInfo->m_largestPixelValue, sqa);
		SQL_GET_INT(pInfo->m_samplesPerPixel, sqa);

		SQL_GET_FLOAT(pInfo->m_pixelSpacing[0], sqa);
		SQL_GET_FLOAT(pInfo->m_pixelSpacing[1], sqa);
		SQL_GET_FLOAT(pInfo->m_aspectRatio, sqa);
		SQL_GET_FLOAT(pInfo->m_rescaleSlope, sqa);
		SQL_GET_FLOAT(pInfo->m_rescaleIntercept, sqa);

		sqa.SkipData(); //SQL_GET_STR(pInfo->m_patientOrientation, sqa);
		sqa.SkipData(); //SQL_GET_FLOAT(pInfo->m_slicePosition, sqa);
		SQL_GET_FLOAT(pInfo->m_sliceThickness, sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[0], sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[1], sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[2], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[0], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[1], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[2], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[3], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[4], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[5], sqa);
*/
		SQL_GET_INT(pInfo->m_byteOffsetToStartOfData, sqa);
		SQL_GET_INT(pInfo->m_dataSize, sqa);
//#1 2012/02/10 K.Ko reduce the instanceLevel's field
/*
		SQL_GET_STR(pInfo->m_referencedSOPInstanceUID, sqa);

		// Murali 2007.01.04 
		sqa.SkipData(); //SQL_GET_INT(pInfo->m_status, sqa);
	
		SQL_GET_STR(pInfo->m_imageDate, sqa);
		SQL_GET_STR(pInfo->m_imageTime, sqa);
		SQL_GET_INT(pInfo->m_wasLossyCompressed, sqa);
		SQL_GET_STR(pInfo->m_scanOptions, sqa);
		
		sqa.SkipData(); // Skip DBName		
		SQL_GET_STR(pInfo->m_manufacturer, sqa);
*/
		// ignore status
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);	

	if (CPxDcmDB::m_sDBOption & CPxDcmDB::kLoadSortInfoFromDisk && GetSeriesPath(iSeriesUID, seriesPath) == kOK && !seriesPath.empty())
	{
		WriteDBInfoToDisk(seriesPath, oVal.size(), oVal);
	}


	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeries end\n");
	return kOK;
}

//--------------------------------------------------------------------------
int CPxDcmDB::UpdateStudyAccessTime(const char* iStudyUID)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::UpdateStudyAccessTime start\n");
	if(!iStudyUID || !iStudyUID[0])
		return kParameterError;

	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("Update dbo.StudyLevel set AccessTime = GETDATE() WHERE StudyInstanceUID = '%s' ", iStudyUID);		
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;
	SQLExecuteEnd(sqa);
	
	return kOK;
}



//####################### Disk space management functions ####################################################

bool CPxDcmDB::GetAvailableMedia(int reserveSpace, std::string& oRootDir)
{
    oRootDir.empty();

	int nMedia = c_mediaPoints.size();
	if(nMedia < 1) return false;

	// TCZ&GL: 2004.08.27
	// kind of virtual highwater mark - we want to make sure
	// switch of disks happens before cache deletion.
	// in theory, a global accumulator keeping track of 
	// all requests and granted space should be used here
	const int extra = 3500; // 3.5GB
	// looking for device has enough space
	int i, mIndex =  -1;
	AqString rootDir;
	for (i=0; i< nMedia; i++)
	{
		rootDir = c_mediaPoints[i].m_mediaPoint;
		rootDir += c_mediaPoints[i].m_mediaLabel;
		if(CheckMediaSpace((reserveSpace + extra), rootDir))
			break;
		rootDir.Empty();
	}

	// no device has has enough space, start making space
	if (rootDir.IsEmpty()) 
	{
		for (i=0; i< nMedia; i++)
		{
			rootDir = c_mediaPoints[i].m_mediaPoint;
			rootDir += c_mediaPoints[i].m_mediaLabel;
			if(MakeMediaSpace(reserveSpace, rootDir))
				break;
			rootDir.Empty();
		}
	}

	if (rootDir.IsEmpty())
		return false;

	oRootDir = rootDir;
	return true;
}

bool CPxDcmDB::CheckMediaSpace(int reserveSpace, const char* iDir, bool checkLow)
{
	int available = TRPlatform::GetAvailableDiskSpace(iDir);
	int waterMark;

	const MediaPoint* mp = GetMediaPoint(iDir);
	if(!mp)
		return false;

	waterMark = (checkLow)?mp->m_lowWaterMark:mp->m_highWaterMark;

	return ((available - waterMark - reserveSpace) > 0);
}

static int	m_lastAlert = 0;
static int	m_lastLowAlert = 0;

//-----------------------------------------------------------------------
static void SendWaterMarkEmail(const char* message)
{
	char cmdline[1024];
	// use pythonw.exe to avoid popup windows, not CREATE_NO_WINDOW flag
	// CREATE_NO_WINDOW flag can not start python script
	sprintf (cmdline, "waterMarkEmail.exe \"%s\"\n", message);
	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof(pi) );
	CreateProcess(NULL,
					cmdline,	// Command line. 
					NULL,	// Process handle not inheritable. 
					NULL,	// Thread handle not inheritable. 
					FALSE,	// Set handle inheritance to FALSE. 
					DETACHED_PROCESS,
					NULL,	// Use parent's environment block. 
					NULL,	// Use parent's starting directory. 
					&si,	// Pointer to STARTUPINFO structure.
					&pi );	// Pointer to PROCESS_INFORMATION structure.
	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
}


bool CPxDcmDB::MakeMediaSpace(int reserveSpace, const char* iRootDir)
{
	AqString message;
	long curTime = time(0);
	if( (curTime - m_lastAlert) > 3600*24)
	{
		m_lastAlert = curTime;
		message.Format("Start to clean old DICOM files on %s", iRootDir);
		SendWaterMarkEmail(message);
		GetAqLogger()->LogMessage(kWarning,"Warning: sent high water mark warning email\n");
	}

	bool st = CheckMediaSpace(reserveSpace, iRootDir, true);
	if(st)
	{
		;// TODO start clean up process as detached process
		return true;
	}
	else
	{
		;// TODO start clean up process and wait for it;
		bool st = CheckMediaSpace(reserveSpace, iRootDir, true);
	}
		

	if (st)
		return true;

	if( (curTime - m_lastLowAlert) > 3600)
	{
		m_lastLowAlert = curTime;
		message.Format("Low water mark alert on %s", iRootDir);
		SendWaterMarkEmail(message);
		GetAqLogger()->LogMessage(kWarning,"Warning: sent low water mark warning email\n");
	}

	return false;
}

bool CPxDcmDB::CheckSeriesPath(int reserveSpace, const char* iStudyUID, const char* iSeriesUID, std::string& oiSeriesDir)
{
	string seriesDir;
	// if the series has a processed directory use it.
	if ( GetSeriesPath(iStudyUID, iSeriesUID, seriesDir) == kOK )
	{
		oiSeriesDir = seriesDir;
		return true;
	}

	// check disk space in the current directory
	if (CheckMediaSpace(reserveSpace, oiSeriesDir.c_str()))
		return true;

	// need new disk location
	if(!GetAvailableMedia(reserveSpace, seriesDir))
	{
		oiSeriesDir = "";
		return false;
	}

	oiSeriesDir = seriesDir +"/"+iStudyUID+"/"+iSeriesUID;
	return true;
}


bool CPxDcmDB::MakeCacheWriteDir(const char* iStudyUID, const char* iSeriesUID, std::string& oCacheDir)
{
	oCacheDir.empty();

	AqString cacheDir;
	// search the series that has pixel file.
	for (int i = 0; i < c_mediaPoints.size(); i++)
	{
		cacheDir.Format("%s/Cache/%s/%s", c_mediaPoints[i].m_mediaPoint, iStudyUID, iSeriesUID);

		if(GetFileAttributes(cacheDir) != 0xffffffff && CheckMediaSpace(1, cacheDir) )
		{
			oCacheDir = (const char*)cacheDir;
			return true;
		}
		
	}

	string rootDir;
	if( GetAvailableMedia(1, rootDir) != kOK)
		return false;
	
	cacheDir.Format("%s/Cache/%s/%s", rootDir.c_str(), iStudyUID, iSeriesUID);
	return (MakeSeriesPath(cacheDir, iStudyUID, iSeriesUID, oCacheDir) == kOK);
}


// #88 2016/09/26 by N.Furutsuki
bool CPxDcmDB::InitDatabaseInfo(bool iRedo, int iretry)
{

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -AQNetDB::InitDatabaseInfo start\n");
	GetAqLogger()->FlushLog();

	//	Make default databaseServerName upper(hostname)
	TRPlatform::NetInit();
	char dbServerName[128];
	gethostname(dbServerName, sizeof(dbServerName) - 1);

	//	Ignore domain
	char* p = strchr(dbServerName, '.');
	if (p != NULL)
		*p = '\0';

	//	Make upper case for SQL Server 
	for (int i = 0; i < sizeof(dbServerName) && dbServerName[i] != '\0'; i++)
	{
		if (islower(dbServerName[i]))
			dbServerName[i] = _toupper(dbServerName[i]);
	}
	//OutputDebugString(dbServerName);

	// #88 2016/09/26 by N.Furutsuki
	const char *db_user = 0;
	const char *db_pw = 0;
	if (DBCore::GetSQLServerUser()[0]){
		db_user = DBCore::GetSQLServerUser();
	}
	if (DBCore::GetSQLServerPassword()[0]){
		db_pw = DBCore::GetSQLServerPassword();
	}

	std::string dbServerNameTemp = dbServerName;
	std::string dbNameExt = DBCore::GetSQLServerDBNameExt();
	if (dbNameExt.size()>0){
		// > SQLServer 2005
		dbServerNameTemp = "localhost";
	}

// #813: SQL Server 2000‚Æ2005‚É—¼‘Î‰ž‚·‚é
	dbServerNameTemp = dbServerNameTemp + dbNameExt;//DBCore::GetSQLServerDBNameExt();

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDcmDB::InitDatabaseInfo %s,user:%s,pw:%s\n",
		dbServerNameTemp.c_str(),
		(db_user == 0) ? "" : db_user,
		(db_pw == 0) ? "" : db_pw);
	GetAqLogger()->FlushLog();
	if (m_GlobalDBType != kDBType_SQLite) {
		CPxDcmDB::InitDBServerName(dbServerNameTemp.c_str(), 0, db_user, db_pw);// #88 2016/09/26 by N.Furutsuki
	}

	return CPxDB::InitDatabaseInfo(iRedo, iretry);
}