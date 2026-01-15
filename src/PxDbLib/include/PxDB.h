/***********************************************************************
 *  PxDB.h 
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */

#ifndef	__PX_FXDB_h__
#define	__PX_FXDB_h__

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "DBCore.h"
#include "PxDBData.h"


#include <string>
#include <map>
#include <vector>


/////////////////////////////////////////////////////////////////////

typedef std::map<std::string, std::string> KVP_MAP;

class  CPxDB : public DBCore
{

public:
	// #15 add locale ConectionInfo 2012/04/24
	void SetMyDBName(const wchar_t *DBName);

	static void SetLogger(AqLoggerInterface* iLogger);

	static const char *getAppDefaultRegistry() ;//#46
	static const char *getAppCommonRegistry() ;//#46
	static int GetRegDBInfo(std::string& oDBName, std::string& oSharedServerDir, std::string& oUser, std::string& oPassword);
	static int SetRegDBInfo(const char* iDBName, const char* iSharedServerDir, const char* iUser, const char* iPassword);

	static bool SetWatermark(int iMediaPointID, int iHigh, int iLow=-1);
	static const std::vector<MediaPoint>& GetMediaPoints();
	static bool SaveMediaPoints(const std::vector<MediaPoint>& mediaPoint);//#46

	static const MediaPoint* GetMediaPoint(const char* iDir);
	static const MediaPoint* GetMediaPoint(int iID);
	
	static bool InCluster();
	static void	InitDBServerName(const char* iDBName=0, const char* iSharedServerDir=0, const char* iUname=0, const char* iPswd=0);
	static const char * GetServerName() {return c_dbServerName;};
	static const char * GetSharedServerDir() {return c_sharedServerDir;};
	static bool InitDatabaseInfo(bool iRedo=false, int iretry=0);
	static bool FileExists(const char* iDir, const char* iPat);

	static int	GetSeriesPath(const char* iStudyUID, const char* iSeriesUID, std::string &oSeriesPath, 
					  const char* iPattern=0,
					  const char* iMediaLabel=0);
	static int	MakeSeriesPath(const char* iTopDir, const char* iStudyUID, const char* iSeriesUID, std::string &oSeriesPath);
	static int	InitMediaPoints( bool iRedo=false );
	static int	InitCharacterLib(void);//#140_search_Japanese_JIS_UTF8

	static int MakeUserStudiesFilter(const DICOMData& iKey, AqString& cond, int iUserID=0);
	//#136 2021/01/12 N.Furutsuki unicode version
	static int MakeUserStudiesFilterU(const DICOMData& iKey, AqUString& cond, int iUserID = 0);
	static unsigned int getCodePageFromCharatorSet(const char *charSet);

	CPxDB();
	virtual ~CPxDB();
	int TestConnectionInfo(int iretry=0);

	int	SQLGetString(const char* iQueryStr, std::string& oVal);
	int	SQLGetInt(const char* iQueryStr, int& oVal);
	int	SQLMakeGetID(const char* iQueryStr, int& oVal);
	int	SQLGet2Int(const char* iQueryStr, int& oVal1, int& oVal2);
	int	SQLMakeGet2ID(const char* iQueryStr, int& oVal1, int& oVal2);
	int	SQLStringValues(const char* iQueryStr, std::vector<std::string>& oVal);
	int	SQLQuery(const char* iQueryStr, std::vector<KVP_MAP>& oVal);

	int	SQLIntValues(const char* iQueryStr, std::vector<int>& oVal);
	double GetDBDateTime();

	// UserAccount
	int MakeImportUser(const char * iUsername, const char * iUserDomain, const char * iLastname, 
					  const char * iFirstname, const char * iEmail, int& oUserID, int& oDomainID);
	int GetUserID(const char * iUsername, const char * iUserDomain);
	int GetUser2ID(const char * iUsername, const char * iUserDomain, int& oUserID, int& oDomainID);
	int	GetUserAccount(int iUserID, UserAccount& oUserAccount);
	bool HasThisAqNETUser(const char* iUsername);
	int GetAqNETUserAccount(const char* iUsername, UserAccount& oUserAccount);
	int	QueryUserAccount(const char* iWhereFilter, UserAccount& oUserAccount);
	int DeleteUserAccount(int iUserID);

	int	UpdateUserPassword(int iUserAccountID, const char* iPassword, int iInterval = 0);
	int	AssignUsertoGroups(int iUserAccountID, const std::vector<int>& iGroupIDs);
	int	AssignUsertoGroups(int iUserID, const std::vector< AqString >& iGroupsSID);
	int DisableUserAccount(int iUserAccountID);
	bool IsPasswordExpired(int iAccountID);
	int DisableInactiveAccounts(int iInactiveIntervalInDays, std::vector<int>& oUserIDAffected );
	int SetUserLoginTime(int iAccountID); 
	int	GetUserAssignedGroups(int iAccountID, std::vector<UserGroup>& oUserGroups);
	int	GetUserAssignedGroupIDs(int iAccountID, std::vector<int>& oUserGroupIDs, bool& oHasShared);

	
	// Organization
	int	GetOrganizationNames (std::vector <std::string>&oVal);
	int	GetOrganization (int iOrganizationID, Organization &oVal);
	int	GetOrganization (const char* iName, Organization &oVal);
	int	QueryOrganization(std::vector<Organization>& oVal, const char* iWhereFilter);
 
	// Domain
	static int	GetAQNetDomainID() { return c_AQNetDomainID; };
	bool IsSSOEnable();
	
	int	GetDomainID(const char* iDomainName);
	int	GetDomainNames(std::vector<std::string>& oVal);
	int	GetDomainInfo(int iDomainID, DomainInfo &oVal);
    int	GetDomainInfo(const char* iDomainName , std::vector<DomainInfo> &oVal);
	int	QueryDomain(const char* iSQLStr, std::vector<Domain> &oVal);
	int	QueryDomain(const char* iSQLStr, std::vector<DomainInfo> &oVal);

	// UserGroup
	static int	GetAdmUserGroupID() { return c_admGroupID; };
	
	int	GetUserGroupID (const char* iGroupName, int iDomainID);
	int	GetUserGroup (int iUserGroupID, UserGroup &oVal);
	int	GetUserGroup (const char* iUserGroupName, int iDomainID, UserGroup &oVal);
	int	QueryUserGroup(std::vector<UserGroup>& oVal, const char* iWhereFilter = 0);
	int QueryUserGroup(std::vector<UserGroupInfo> &oVal, const char* iWhereFilter);
	int GetDefaultLoginGroupID(int iAccountID, int& oGroupID);
	int UpdateDefaultLoginGroup(int iUserAccountUID, int iDefaultGroupID);
	int GetUserGroupInfo(int iUserGroupID, UserGroupInfo &oVal);
	bool IsUserInGroup(int iUserID, int iGroupID);
	


	int	GetGroupRights(int iGroupID, std::vector<AqStringKV>& oRights );
	int	SetGroupRights(int iGroupID, const char* iKey, const char* iValue);
	int	RemoveGroupRights(int iGroupID, const char* iKey);
	bool GroupHasTheRights(int iGroupID, const char* iKey, const char* iValue);
	bool CanGroupAccessAllData(int iGroupID);

	//bool QuerySeriesUID(std::vector<std::string> &iFilter);
	// All AE tables 
	
	const enum AE_TYPE
	{
		kRemoteAE = -1,
		kQRAllowedAE,
		kStoreTargetAE,
		kQRSourceAE,
		kLocalAE
	};
 
	int GetApplicationEntity(int iAETitleID, ApplicationEntity & oVal, AE_TYPE type);
	int	GetAllApplicationEntity(std::vector <ApplicationEntity> &oVal, AE_TYPE type);
    int	QueryApplicationEntity(AE_TYPE type, std::vector<ApplicationEntity> &oVal, const char* iWhereFilter );
	int AddRemoteAE(ApplicationEntity& iAE, const std::vector<int>& iGroupIDs,
	 			int iQRAllowed, int iQRSource, int iStoreAE);
	int	DeleteRemoteAE(const char* iAEName);
	int ModifyRemoteAE(ApplicationEntity& iAE, const std::vector<std::string> &iGroupNames,
						 int iQRAllowed, int iQRSource, int iStoreAE);
	int ModifyRemoteAE(ApplicationEntity& iAE, const std::vector<int> &iGroupIDs,
						 int iQRAllowed, int iQRSource, int iStoreAE);
 
	int	AssignQRSourceAEGroup(int iAEID, const std::vector<int>& iGroupUIDs);

	int GetRemoteAESpecification (int iRemoteAEID, RemoteAESpecification& oVal);

	// QRAllowedAE 
	bool IsRetrieveAE (const ApplicationEntity& iAE ); // Done

	// StoreTargetAE
	bool IsStoreTargetAE (const char* iAETitle, ApplicationEntity& oAE);// Done
    
	// LocalAE
	int	ActiveLocalAE(const char* iHostname, bool iOnline);
	int	GetAllLocalAE(int iPort, std::map< std::string, int >& localAEs, char* iHostname=0); // Done
	int	GetLocalAENames(int iPort, std::vector<std::string>&oVal);
	int	GetEnabledLocalAE(std::vector<ApplicationEntity>& oAEList, int iRetry=0);

	int	DeleteLocalAE(const char* iAEName);
	int	InitDefaultLocalAE(ApplicationEntity& iAE);

    // AutoRoutingAE
	int	GetRoutingAEInfos(int iLocalAEID, std::vector<RoutingAEInfo>& oVal);
	int	GetRoutingAEInfos(const char* iLocalAEName, std::vector<RoutingAEInfo>& oVal);
    
	int GetRoutingTimeRanges(int iRoutingScheduleID, int iDayofWeek, std::vector<TimeRange>& oVal);
 
	// QRSourceAEGroupAssignment
	int	GetQRSourceAEs(int iGroupID, std::vector<ApplicationEntity> &oVal);
	
 
	// AuxData for PrivateData and PrivateDataReference tables
	int SaveAuxDataInfo(AuxDataInfo& iAuxData, const std::vector<AuxReference>& iReferences);
	int	GetAuxDataInfos(const char* iSeriesInstanceUID, std::vector<AuxDataInfo>& oVal);
	int	GetPatientAuxData(const DICOMData& iPatientFilter, const AuxDataInfo& iAuxDataFilter, std::vector<PatientAuxDataInfo>& oVal);
	int	GetPatientAuxData(const std::vector<AuxReference>& iReferenceFilter, AuxDataInfo& ioAuxData );
	int	GetAuxRefererces(const char* iAuxSOPInstanceUID, std::vector<AuxReference>& oVal);
	int	CheckAuxDataStudy(int iAuxID, const char* iRefStudyUID, std::string& oAuxStudyUID);
	int	HasAuxData(const char* iOrigStudyInstanceUID, int& oMask);
	int	GetSeriesAuxDataMask(const char* iSeriesInstanceUID, int& oMask);
	bool IsAuxSeries(const char* iSeriesInstanceUID);
	int	GetAuxDataInfos(const std::vector<const char*> iRefSeriesUIDs, std::vector<AuxDataInfo>&oVal, const AuxDataInfo& iFilter, int iTopN=0);
	int GetAuxDataInfo(int iAuxID, AuxDataInfo & oAuxInfo);// -- 2006.05.26

	// All AE tables 
	
	const enum QUERY_TYPE
	{
		kQueryAll = 0,
		kQueryTemplateOnly,
		kQueryUnassignedOnly,
	};

	int GetTotalSeries2Display(const DICOMData* iFilter, int iQueryOption, int &oTotalItems, const char* receiveDate1=0, const char*  receiveDate2=0);
	int GetTotalSeries2Display4Group(const DICOMData* iFilter, int iGroupID, int &oTotalItems, const char* receiveDate1=0, const char*  receiveDate2=0);
	int GetTotalSeries2Display4User(const DICOMData* iFilter, int iUserID, int iType, int &oTotalItems, const char* receiveDate1=0, const char*  receiveDate2=0);
	int	GetSeriesDisplayInfoOnServer4Page(std::vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter, int iQueryType, 
				int iItemsPerPage, int iPageNumber,const char* receiveDate1=0, const char* receiveDate2=0);

	int	GetSeriesDisplayInfoOnServer4Group4Page(std::vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter,  
				int iItemsPerPage, int iPageNumber, int iGroupID, const char* receiveDate1=0, const char* receiveDate2=0);
	int	GetSeriesDisplayInfoOnServer4User4Page(std::vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter,  
				int iItemsPerPage, int iPageNumber, int iUserID, int iSeriesStatusType,const char* receiveDate1=0, const char* receiveDate2=0);
	
	int	GetSeriesDisplayInfoOnServer(std::vector<SeriesDisplayInfo> &oVal, const char* iWhereFilter = 0, int iSize = -1);
	int	GetSeriesDisplayInfoOnServer(std::vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter, const char* receiveDate1=0, const char*  receiveDate2=0);
	int GetGroupSeriesInfo(std::vector<SeriesDisplayInfo> &oVal, int iGroupID);

	
	
	
	int	GetSeriesPath(const char* iSeriesUID, std::string &oSeriesPath);
	int	GetStudyPath (const char* iStudyUID,  std::string &oStudyPath);
 
	// UserSeriesStatus
	int	GetUserSeriesInfo (std::vector<SeriesDisplayInfo>& oVal, int iUserID, int iGroupID);
	int	GetUserSeriesInfo (std::vector<SeriesDisplayInfo>& oVal, int iUserID, const char* iWhereFilter = 0, int iTopSize= -1);

	int	GetUserSeriesInfo (int iUserID, std::vector<SeriesDisplayInfo>& oVal, const DICOMData* iFilter=0);
 	int	GetUserSeriesInfo (int iUserID, std::vector<SeriesDisplayInfo>& oVal, int iStatusFlag, const DICOMData* iFilter=0);
 	int	GetUserSeriesInfo (int iUserID, int iGroupID, std::vector<SeriesDisplayInfo>& oVal, const DICOMData* iFilter=0);
	
	int	GetUserSeriesStatus(int iUserID, const char* iSeriesUID, int& oStatus);

	// changed by shiying hu, 6-13-2005 due to database design change
	int	UpdateUserSeriesStatus(const char* iSeriesUID, int iUserID, int iStatus);


	bool VerifySeriesUIDByInstances(const char* iSeriesUID, const std::vector<std::string>& iInstanceUIDs);
	
	enum eSeriesStatus
	{
		eNotExists		= -99,
		eBadInCompleted	= -1,
		eCompLeted		= 0,
		eInprogress		= 1,

		eUnknown		= 999
	};

	// GroupSeries
	int	AddNewSeries(const char* iSeriesUID);
	int	MarkBadSeries(const char*  iSeriesUID);
	eSeriesStatus GetSeriesStatus(const char*  iSeriesUID);
	int SetSeriesStatus(const char*  iSeriesUID, CPxDB::eSeriesStatus iStat);

	int	DeleteSeries(const char* iSeriesUID);
	int AssignGroupSeries(std::vector<int>& iGroupIDs, std::vector<std::string>& seriesUIDs);
	int AssignGroupSeries(std::vector<int>& iGroupIDs, std::vector<std::string>& seriesUIDs, int iRetries, int iTimemSec = 250);
	int UnassignGroupSeries(int iGroupID, std::vector<std::string>& seriesUIDs);

	int	GetMapStrings(const char* iQueryStr, std::map<std::string, std::vector<std::string> >& oVal);

	// for scan manager (--, 04/30/2003)
	int	GetSeries(const char* iStudyUID, std::vector<std::string>& oSeries);
	int	GetSOPUID(const char* iSeriesUID, std::vector<std::string>& oSOP);
	int	DeleteStudy(const char* iStudyUID);

	// DICOM data
	bool HasThisInstance (const char* iSopInstanceUID, const char* iSeriesInstanceUID);
	int	RemoveInstance( const char* iSeriesUID, const char* iSopInstanceUID);
	int	GetNumberOfSeriesRelatedInstances(const char* iSeriesInstanceUID);
	int	GetNumberOfSeriesRelatedFrames(const char* iSeriesInstanceUID);

	virtual int	SaveDICOMData(const DICOMData& iData, int iInstanceStatus=kImageFormatInDCM);
	int	GetPatientList( std::vector<DICOMPatient>& oVal, const DICOMData*  iFilter);
	//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
	// added: bCnvUTF8
	int	GetStudyList(std::vector<DICOMStudy>& oVal, const DICOMData*  iFilter, int TopN = 0, bool iSort = false, bool bCnvUTF8 = false);

	//#60 2013/07/03
	/*
	*  iSortStudyDate: 0: none ,1:ASC, 2: DESC
	*/
	int	GetStudyListEx(std::vector<DICOMStudy>& oVal, const DICOMData* iFilter, int TopN = 0, int iSortStudyDate = 0, bool bCnvUTF8 = false);

	int	GetSeriesList( std::vector<DICOMSeries>& oVal, const DICOMData*  iFilter, bool iSort=false,bool bCnvUTF8=false);
	int GetInstanceList( const DICOMStudy& iStudy, const DICOMSeries& iSeries, 
		std::vector<DICOMData>& oVal, const char* iRealSeriesUID=0);
	int	GetInstanceList( std::vector<DICOMInstance>& oVal, const DICOMData*  iFilter, const char* iSeriesInstanceUID);
	int	GetInstanceList( std::vector<DICOMInstanceX>& oVal, const DICOMData*  iFilter, const char* iSeriesInstanceUID, int iNInstace=0);
	int	GetInstanceStatus(const char* iSeriesUID, const char* iSopInstanceUID, int& iStatus);
	int	SetInstanceStatus(const char* iSeriesUID, const char* iSopInstanceUID, int iStatus);
	int	MakeInstance( const DICOMData& iData);
	int UpdatePixelMinMax(const DICOMData& iData);

	//	DICOM Tag Filters
	int GetTagFilterRules(std::vector<TagFilterRule>& oRules, const char* iWhereFilter = 0);
	int	InsertIntoGroupSeries(const char* iSeriesInstanceUID, std::map<int, int>& iTagFilterIDs, std::vector<int>& oAssignedGroups);

	// pretetch
	int GetPrefetchPatternEntry(std::vector<PrefetchPatternEntry>& oVal);

	int	GetPathToInstanceList(const char* iStudyUID, const char* iSeriesUID, const std::vector<std::string>& iInstances,
		std::vector<std::string>& oPaths);
	
	int	GetPathToInstanceList(const char* iStudyUID, const char* iSeriesUID, const std::vector<AqString>& iInstances,
		std::vector<std::string>& oPaths);

	int	GetPathToInstanceList(const char* iStudyUID, const char* iSeriesUID, const std::vector<const char*>& iInstances,
		std::vector<std::string>& oPaths);

	// DICOM Printers
	int	QueryPrinter(const char* iSQLStr, std::vector<PrinterInfo> &oVal);
	int	QueryPrinter(std::vector<PrinterInfo>& oVal, int iGroupID = 0, const char* iWhereFilter = 0);
	int	GetPrinterNames(std::vector <std::string>&oVal);
	int	GetPrinterInfo(const char* iName, PrinterInfo& oVal);
	int	GetPrinterAssignedGroupIDs(const char* iName, std::vector<int>&iGroupIDs);
	int	ModifyPrinter(const char* iUpdatePrinterSQL,int iPrinterUID, const std::vector<int> &iGroupIDs);
	int	AddPrinterDefinition(const char* iAddPrinterSQL, const char* iPrinterName, const std::vector<int>& iGroupIDs);
	long GetMBLimit(); //database size limit
	double GetTotalDataMBSize(const char* dbname=0); //default to get main dataabse, "" to total size

	int GetAutoFilmingPattern(int iFilterID, std::vector<FilmingPatternEntry>&oVal);

	int GetJobByFilters(const std::vector<int>& iFilters,  std::vector<DataProcessJob>& oJobs, bool iZeroID=false);

	// AqNETOption
	int GetAqNETOption(const char* iTag, AqString& oVal);
	int GetAqNETOption(const char* iTag, long& oVal);
	bool IsAPSEnableded();

	int GetDBSizeInfo(DBSizeInfo& ioData);

	//history logging
	bool IsAuditTrailEnabled();
	int	MakeAqObjectType(AqObjectType& ioObject);
	int	GetAqObjectType(int iID, AqObjectType& oObject, bool nameOnly=false);
	int	GetAllAqObjectTypeName(std::vector<std::string>& oVal);

	static int	GetAqObjectTypeID(const char* iName);
	static int	GetAqObjectTypeID(eAqObjectType iKey);

	int	MakeAqObject(AqObjectInterface& ioObject);
	int	GetAqObject(int iID, AqObjectInterface& oObject);

	// int	MakeAEObject(ApplicationEntity ioAE); use MakeAqObject
	int	MakeAEObject(const char* iAEName, int& objectID, int IsLocalAE=0);
	int	MakeAEObject(int AE_ID, int& objectID, int IsLocalAE=0);

	
	int	MakeUserObjectFromUserAccount(int iUserID, int& objectID);

	int MakeSeriesAttribute(const AuxDataInfo& iAuxInfo, int& oSeriesAttributeID);

	int	MakePatientObject(DICOMData& ioPatient, int iTransferSyntax, int iSourceAE_ID=0, int iSeriesAttributeID=0);
	int	MakePatientObject(const char* iSeriesLevelUID, int& oStudyIndex, int& oSeriesIndex, int iSourceAE_ID=0);
	int	MakePatientObject(int iSeriesLevelID, int& oStudyIndex, int& oSeriesIndex, int iSourceAE_ID=0);

	int	MakeActionWord(Actions& ioAction);
	int	GetActionID(const char* iName);
	int	GetActionID(eLogAction iKey);

	int	LogEvent(EventLog& iEvent); 

	inline int UpdateAqObjectID(AqObjectInterface& ioObject)
	{
		return (ioObject.HasValidLink())?kOK:MakeAqObject(ioObject);
	}

	int MakeSeriesReadUnreadObject(bool iIsRead, int& oID);

	int MakeEvent(EventLog& oEvent, 
				AqObjectInterface& ioActor, Actions& ioAct,
				AqObjectInterface& ioActOn,
				AqObjectInterface& ioRequestor,
				AqObjectInterface& ioActionFrom,
				AqObjectInterface& ioActionAt,
				AqObjectInterface& ioTransferTo); 


	int	LogEvent(AqObjectInterface& ioActor, Actions& ioAct,
				 AqObjectInterface&	ioActOn,
				 AqObjectInterface& ioRequestor,
				 AqObjectInterface& ioActionFrom,
				 AqObjectInterface& ioActionAt,
				 AqObjectInterface& ioTransferTo,
				 const char* iDescription,
				 int iStatus,
				 double	iTimeOfAction=0.0); 

	int	LogEvent(AqObjectInterface& ioActor, Actions& ioAct,
				 AqObjectInterface& ioRequestor,
				 AqObjectInterface& ioActionFrom,
				 AqObjectInterface& ioActionAt,
				 const char* iDescription,
				 int iStatus,
				 int iDICOMIndex, bool isStudy=false,
				 int iTransferToIndex = 0,
				 double	iTimeOfAction=0.0); 


	int UpdateStudyInfo(DICOMData &iData);  // tczhao 2005.08.04

	//int	MakeHistoryPatientInfo(const DICOMData& dData, int iHistAE=0, bool QRSeries=true);
	//int	MakeHistoryPatientInfo(const char* iSeriesUID, int iHistAE=0);


	// Added by shiying hu, 2006-02-02
	// AqPE needs a working directory. Currently, this working directory will be under 
	// the first cache directory.
	static int	GetAqNetCacheDir(std::string &oCacheDir);

	// Added by shiying hu, 2006-02-02
	// AqPE needs AqNetImport directory to dump its private dicom file
	static int	GetAqNetImportDir(std::string &oImportDir); 
	

	const enum LOCK_TYPE
	{
		kCreateDICOMDir = 0,
		kCreateCacheDir,

		kWriteDICOMDir,
		kWriteCacheDir,

		kReadDICOMDir,
		kReadCacheDir,

		kDeleteDICOMDir,
		kDeleteCacheDir,

		kQMCreateJob,
		kQMUpdateJob,
		kQMRunJob,
		kQMCancelJob,
		kQMDeleteJob,

		kCleanDiskSpace,


		kUnKnown,
	};

	int	CreateAppLock(SQA& iSqa, const char* iName, LOCK_TYPE iType, int iTimeOut=2000 );
	int	ReleaseAppLock(SQA& iSqa, const char* iName, LOCK_TYPE iType);

protected:

	int SavePrivateDataAndGetPK(const AuxDataInfo& iAuxData, int &pk);
	int	InsertToPrinterGroupAssignment(SQA& iSQA, int iPrinterID, const std::vector<int>& iGroupIDs);
	int	InsertToPrinterGroupAssignment(SQA& iSQA, int iPrinterID, const std::vector<std::string> &iGroupNames);
	int InsertToPrinter(SQA& iSQA, const char* iAddPrinterSQL, const char* iPrinterName, int &oPrinterID);
	int	AssignQRSourceAEGroup(SQA& iSQA, int iAEID, const std::vector<int>& iGroupIDs);
	int	GetTempRoutingAEInfos(int iLocalAEID, std::vector<RoutingAEInfo>& oVal, int& oSuspendOthers);
	int MakeUserSeriesFilter(const DICOMData* iFilter, std::string &oFilter, int iUserID,int iStatusType, const char* receiveDate1=0, const char* receiveDate2=0 );


	static char c_sharedServerDir[256];
	static char c_dbServerName[256];
	static int	c_admGroupID;
	static UserGroup c_publicGroup;
	static int  c_AQNetDomainID;
	static std::vector<MediaPoint> c_mediaPoints;

private:

};

 
#endif	/* __PX_FXDB_h__ */
