/***********************************************************************
 * VLIDicom.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		DICOM SCU API
 *
 *
 *-------------------------------------------------------------------
 */

#ifndef VLI_DICOM_H
#define VLI_DICOM_H

#ifdef _WIN32
#pragma warning (disable: 4786)
#pragma warning (disable: 4616)
#endif

#include "rtvPoolAccess.h"
//#include "CPxDicomImage.h"
#include "PxDicomImage.h"

//#include "PxDicomStatus.h"
#include "PxDicomStatus.h"
#include "VLIDicomConfig.h"

typedef enum
{
	kVLIRetrieve = 0,
	kVLILoad,
	kVLIConvert,
	kVLIUpdate,
	kVLIQuery
} eAction;

const int kVLIDICOMSCUVersion	=1;
const int kVLIDICOMSCURevision	=2;
const int kVLIDICOMSCUTweaks	=2;

class VLIAssociationChecker;
class DiCOMStore;
typedef std::map<std::string, VLICachedInfo*> tStudyCache;
//-----------------------------------------------------------------------------
//
class VLIDicomBase : public iRTVBase
{
public:	
	static bool		m_reopenAssociationEachTime;
	//	Constructors
	VLIDicomBase();										// Default Constructor

	// Destructor
	virtual ~VLIDicomBase();
	
	/***********************************************
	 *
	 *	Config
	 */

	// Get / Set for configurable parameters.  List of keys is defined in VLIDicomConfig.h
	// Both functions expect the char* to have been allocated prior to the call. It is
	// recommended that the char* be sized to kMaxConfigLen (also defined in VLIDicomConfig.h)
	//TODO: sepreate instatance configuration with globle configuration
	PxDicomStatus SetConfigValue(int iParameterKey, const char* iParameterValue);
	PxDicomStatus GetConfigValue(int iParameterKey, char* oParameterValue);

	// state control and query
	const char*	GetModality(const char* iSeriesUID, const char* iStudyUID);
	const char* CacheDir() {return m_localCache;};
	int IsCancelled() const { return m_cancel; }

	void  CancelTask(void) 
	{
		m_cancel = 1;
	}
	
	void ResetCancelFlag() 
	{ 
		m_cancel = 0; 
	}

	// Functions for managing the list of known DICOM servers
	PxDicomStatus AddServer(const char* iAETitle, const char* iHostname, int iPort, int iConnect = 1);
	PxDicomStatus RemoveServer(DicomServer& iServer);
	PxDicomStatus UpdateServer(DicomServer& iServer);
	int			   GetServerCount(void) const { return m_serverList.size();}
	DicomServerList& GetServerList(void) { return m_serverList;}
	PxDicomStatus RemoveAllServers();
	PxDicomStatus AdvanceServer();
	PxDicomStatus SetCurrentServer(DicomServer& iServer);
	DicomServer&   GetCurrentServer(void);

	PxDicomStatus LoadDicomHeader(CPxDicomImage*& oMessage, const char* iFilePath);
		
	//	Progress
	//	Callback function called from within the implementation to give progress updates
	//	To use - derive from this class, override this method, and provide an action for
	//	each type of action as listed in enum eAction.  
	virtual void Progress(int oRemaining, int oCompleted, int oFailed, int oWarning, eAction oAction) {}

	//	-- - 10/21/02 - Overide iRTVBase functions to add additional info
	void LogMessage(const char *fmt, ...);
	void LogMessage(int iLevel, const char* fmt, ...);

	// a hack to switch dicomservers -- 04/01/2003
	void UpdateStudyCache(const char* iStudyUID,  DicomDataSource* iSource, const char* modality);
	void SetReopenAssociationEachTime(bool iYesNo);

	// -- 2005.10.11
	DicomDataSource* GetStudySource(const char* iStudyUID);

protected:
	void PrintOnError(const char* iMsg, int iErrorCode);
	//=============================================================
	// Added by JWU 01/30/02
	void  PrintStrOnDebug(const char* iString, int iErrorCode);
	void  PrintIntOnDebug(const char* iString, int iErrorCode);
	void  PrintOutOnError(const char* iString, int iErrorCode);

	DicomDataSource& PickBestServer    (std::vector<DicomDataSource> dsv);
	PxDicomStatus	m_status;

	//	-- - 05/03/02 - Use this to control how long (in uSec) before queries timeout.  
	int m_timeoutOnQuery;
	int m_timeoutOnRetrieve;

	//	-- 08/06/02 Should duplicate query results be removed?
	int m_removeDuplicateQueryResults;
	int m_maxDICOMQueryResults;

	//	-- 01/30/04 for forcing transfer syntax to a specific one
	//	
	//	0 -> do NOT force transfer syntax
	//	1 -> Implicit Little Endian
	//	2 -> Explicit Little Endian
	//	3 -> Explicit Big Endian
	int	m_forceTransferSyntax;

	DicomServer m_currentServer;
	DicomDataSource  m_dataSource;
	char m_localCache[kMaxPathNameLen];
	tStudyCache m_studyCache;
	int	m_cancel;
	static bool m_printDebugMessages;

	//	-- - 05/21/02
	DicomServerList m_serverList;

	int m_connectCMoveBySeriesUID;
};

 
//-----------------------------------------------------------------------------
//
class AsyncOpenAssociation;
// -- (to close Association each query automatically 12/17/2001)
class VLIAssociationChecker : public iRTVThreadFunction
{
public:
	VLIAssociationChecker(DicomServer* iServer, VLIDicomBase* instance, const char* iWhere="??", const char* serviceList= "TIDICOMServer_Service_List");
	~VLIAssociationChecker(void);
	bool				OK(void) const { return m_status == kNormalCompletion;}
	PxDicomStatus		GetStatus(void) const;

protected:
	int VLIAssociationChecker::ThreadFunction(void* data);
	AsyncOpenAssociation		*m_openAssoc;
private:
	PxDicomStatus	m_status;
	VLIDicomBase*	m_owner;
	DicomServer*	m_server;
	const char*		m_where;
	int				m_associationID;
};

//-----------------------------------------------------------------------------
// GL move QUERY function to this help class from VLIDicom 3/5/03
class CFindSCU : public VLIDicomBase
{
public:
	CFindSCU();

	virtual ~CFindSCU();

	//Query
	// Convenience functions that perform queries.  If optional argument iSource is not given, 
	// results are returned for all known data sources.
	PxDicomStatus QueryListOfPatients (DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply);
	PxDicomStatus QueryListOfStudies  (DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply, const char* iPatientID);
	PxDicomStatus QueryListOfSeries   (DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply, const char* iStudyUID);
	PxDicomStatus QueryListOfImages   (DicomDataSource& iSource, std::vector<CPxDicomMessage*>& oReply, const char* iSeriesUID, const char* iStudyUID = NULL);
	PxDicomStatus QueryFindDataSources(std::vector<DicomDataSource>&  oReply, const char* iSeriesUID, const char* iStudyUID, std::string& oModality, bool queryOne=false);
	PxDicomStatus QuerySOPByInstanceNumber (DicomDataSource& iSource, int iInstanceNumber, const char* iSeriesInstanceUID, const char* iStudyInstanceUID, std::string& oSOPUID);

	// Search iSource for matches to constraints set in iQuery
	// call SetCurrentServer, before call Query for right server to query on
	PxDicomStatus Query(CPxDicomMessage& iQuery, std::vector<CPxDicomMessage*>& oReply);
	
	//search all known dataSources
	PxDicomStatus QueryAllServers(CPxDicomMessage& iQuery, std::vector<CPxDicomMessage*>& oReply, 
		bool queryOne=false, bool removeDups=true);

private:
	PxDicomStatus SetQueryLevel(CPxDicomMessage& iMessage);
	PxDicomStatus GetAndParseCFindResponseMessages(int iMsgID, std::vector<CPxDicomMessage*>& oReply);
	PxDicomStatus PrepareAndSendCFindReqMSG(int iMsgID);
	PxDicomStatus DoCFindCancellation(int iMsgID, int iResponseID);
	PxDicomStatus RemoveDuplicateQueryResults(CPxDicomMessage& iQuery, std::vector<CPxDicomMessage*>& ioMessageVector);
	PxDicomStatus CheckForAndRemoveDuplicateAtIndex(std::vector<CPxDicomMessage*>& ioMessageVector, std::vector<CPxDicomMessage*>::iterator iter, unsigned long iTag);

	char	m_queryLevel[16];
	bool	m_isCFindCancelResquestSent;
};


class CMoveSCU;
typedef RTVMapAccess<std::string, CMoveSCU*> VLIDICOMPOOL;
typedef RTVMapAccess<DiCOMStore*, int> DiCOMStorePOOL;

//-----------------------------------------------------------------------------
// GL move retrieve function to this help class from VLIDicom 3/5/03
class CMoveSCU : public CFindSCU
{
public:
	// retrieve and listener connection functions
	//static VLIDicom* NewHeader(DiCOMStore* pStore, CPxDicomImage* iImage);
	static CMoveSCU* FromSerialKey(DiCOMStore* pStore);
	static bool ConnectCMove(DiCOMStore* pStore);
	static bool GetCacheDir(DiCOMStore* pStore, char* cacheDir, int len);
	static bool HandoverImage(DiCOMStore* pStore, CPxDicomImage* iImage);
	static bool DisconnectCMove(DiCOMStore* pStore);

	// Cleanup
	//	Need to call these to clean up replies from query and retrieve operations
	static void FreeReplyVector(std::vector<CPxDicomMessage*>& v);
	static void FreeReplyVector(std::vector<CPxDicomImage*>& v);

	//	-- 06/11/03 An attempt to fix #3729 - CMove failing because SerialKey lookup fails
	static int m_testMoveOriginator;

	CMoveSCU();
	virtual ~CMoveSCU();

	PxDicomStatus DoCMove();

	//Retrieve
	PxDicomStatus RetrieveStudy(std::vector<CPxDicomImage*>& oImages, 
										const char* iSeriesUID,
										const char* iStudyUID, 
										DicomDataSource* iSource = NULL,
										const char* iDestAETitle = NULL,
										bool iProgressEvenIfNotLocalDest = false);

	PxDicomStatus RetrieveSeries(std::vector<CPxDicomImage*>& oSeries, 
								  const char* iSeriesUID, 
								  const char* iStudyUID = NULL,
								  DicomDataSource* iSource = NULL,
								  const char* iDestAETitle = NULL,
								  bool iProgressEvenIfNotLocalDest = false);

	PxDicomStatus RetrieveImages(std::vector<CPxDicomImage*>& oImages, 
								  std::vector<const char*>& iSOPUIDs, 
								  const char* iSeriesUID = NULL, 
								  const char* iStudyUID = NULL,
								  DicomDataSource* iSource = NULL,
								  const char* iDestAETitle = NULL);

	//	Performs the actual retrieve at the level specified by iLevel.  The only two levels supported
	//	at this time are "SERIES" and "IMAGE".  For "SERIES" level,l iSOPInstanceUID is ignored.
	PxDicomStatus Retrieve(std::vector<CPxDicomImage*>& oImages, 
							std::vector<std::string>& iStudyUIDs,
							std::vector<std::string>& iSeriesUIDs, 
							std::vector<std::string>& iSOPUIDs, 
							char* iLevel, 
							DicomDataSource* iSource = NULL,
							const char* iDestAETitle = NULL,
							bool iProgressEvenIfNotLocalDest = false);

	//void NewFrame(CPxDicomImage* iImage, unsigned char* iFrame);
	//void NewImage(DiCOMStore* pStore, CPxDicomImage* iImage);

	int  IncrementImagesReceivedSoFar(void) { return ++m_imagesReceivedSoFar; }
	int  GetTotalExpectedImages(void) const { return m_totalExpectedImages; }

private:
	static std::string MakeSerialKey(const char* sUID, const char* remoteEntity);
	PxDicomStatus PrepareCMOVERequestMessage();
	PxDicomStatus HandleCMoveResponse(unsigned int& cMoveResponseStatus, int& nFailedInstances);
	PxDicomStatus SendCMoveCancelRequest();
	void AddSerialKey(const char* sUID, const char* remoteIP, const char* remoteAE, int rMsgID);
	
	// class maps
	static VLIDICOMPOOL SerialKeyPool;

	DiCOMStorePOOL m_connectedDiCOMStores;

	std::vector<CPxDicomImage*>*	m_pImages;
	const std::vector<std::string>* m_pStudyUIDs;
	const std::vector<std::string>* m_pSeriesUIDs;
	const std::vector<std::string>* m_pSOPUIDs;
	const char* m_level;
	const char* m_destAETitle;

	bool m_progressEvenIfNotLocalDest;
	bool m_bCmoveExit;
	int	m_cMoveRequestMessageID;
	int	m_cMoveResponseID;

	int m_totalExpectedImages;
	TRAtomicVar<int> m_imagesReceivedSoFar;
};


//-----------------------------------------------------------------------------
//
class  VLIDicom : public CMoveSCU
{
public:	

	static const char*	GetVersionString(void);
	static const char*  GetBuildString(void);
	static void			GetVersion(int& oMajor, int& oMinor, int& oTweaks);

	//	Constructors
	VLIDicom() {};											// Default Constructor
	//VLIDicom(const VLIDicom& iDicom);					// Copy Constructor
	//VLIDicom& operator= (const VLIDicom& iDicom);		// = operator

	// Destructor
	virtual ~VLIDicom(){};
};

#endif // VLI_DICOM_H
