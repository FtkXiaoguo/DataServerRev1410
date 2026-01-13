/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "libmtime.h"

#include "CStore.h"

#include <assert.h>
#include <sys/timeb.h>
#include "rtvpoolaccess.h"
#include "AppComUtil.h"
//#include "TRDICOMUtil.h"
#include "Globals.h"
#include "DiskSpaceManager.h"
#include "SeriesDirMonitor.h"
#include "Compression.h"
#include "NMObject.h"

#ifdef USE_AUX_DATA //#28 2012/06/20 K.Ko
#include "AuxData.h"
#endif

#include "AppComCacheWriter.h"
#include "Conversion.h"


#include "AutoRoutingAEMan.h" //#17 2012/05/11 K.Ko


#include "rtvsutil.h"

//#define NEW_PROFILE_STORE

#ifdef _PROFILE
#include "ScopeTimer.h"
#endif

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "CheckMemoryLeak.h"

extern MC_STATUS GetPixelData(int messageID,unsigned long tag,void* userInfo, 
					   int dataSize,void* dataBufferPtr,int isFirst,int isLast);

#ifdef NEW_PROFILE_STORE
#define BeginNewProFile()  MTIME decl_start_timer; decl_start_timer.update();
#define EndNewProFile(start_timer,spent_time)  { MTIME decl_end_timer;  decl_end_timer.update(); spent_time  = elapse( start_timer, decl_end_timer ); };
#define LogNewProfile(disp_msg) {\
    float spent_time; \
	MTIME decl_end_timer; \
	decl_end_timer.update(); \
	spent_time  = elapse( decl_start_timer, decl_end_timer ); \
	LogMessage(kInfo," <<<Profile>>> %s spent time %.3f Sec \n", disp_msg,spent_time/1000.0f); \
	FlushLog();\
}
#else
#define BeginNewProFile()   ;
#define LogNewProfile(disp_msg) ;
#endif
//-----------------------------------------------------------------------------
CStore::CStore (DiCOMConnectionInfo& connectInfo, int iMessageID):
	   RTVDiCOMStore(connectInfo, iMessageID)
{
	m_processorName = "CStore";
	m_fileName[0] = 0;
	m_imageDBSaved = -1; // not processing any image yet

	m_dbData.Clear ();
	m_db.InitDatabaseInfo();
}

//-----------------------------------------------------------------------------
CStore::~CStore()
{
	/*
    * m_messageID は
	* RTVDiCOMStore::theProcessHeader()
	*  m_pImage = new CPxDicomImage(m_messageID);
	*  CPxDicomMessage::~CPxDicomMessage(void) で MC_Free_Message (&m_messageID);
	*
	*/
	if (m_OBOWbuffer)
	{
		delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
	}

	RTVInactiveManager& imanger=RTVInactiveManager::theManager();
	// received one image but failed insert to database, 
	// delete the map entry to let later push go through
	if((m_imageDBSaved == 0 || m_imageDBSaved == 99) && m_pImage)
	{
		
		// make sure the monitor exists
		SeriesDirMonitor* pSeriesDirMonitor = (SeriesDirMonitor*)imanger.LockHandler(m_seriesUID, this, true);
		if(pSeriesDirMonitor)
		{
			pSeriesDirMonitor->RemoveInstanceUID(m_pImage->GetSOPInstanceUID());
		}
		
	}
	
	imanger.LockHandler(m_seriesUID, this, false);

}

void CStore::LogProcessStatus(void)
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
		case kHandleUSImage_AddUSImage:
			pState = "In kHandleUSImage AddUSImage";
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
		case kHandleNMImage_AddNMImage:
			pState = "In HandleNMImage AddNMImage";
			break;
		case kHandlePTImage_AddPTImage:
			pState = "In HandlePTImage AddPTImage";
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
	LogMessage("STATE_INFO: %s in state: %s\n", m_processorName, pState);
}

//  check UIDs
static inline bool IsValidDICOMUID(const char* s)
{
	 bool yes = true;
	 for (; yes && s && *s; ++s)
		 yes = (isdigit(*s) || *s=='.');
	 return yes;
}


/* remove spaces from both ends of a string */
//extern char* iRTVDeSpaceDe(char *s, int replaceInPlace=1);
 

//-----------------------------------------------------------------------------
//
int CStore::CoerceSOPInstanceUID ()
{
	int	_status;
	
try{ // #11  2012/03/23 K.Ko
		 
	const char* studyUID = m_pImage->GetStudyInstanceUID();
	ASTRNCPY(m_seriesUID, m_pImage->GetSeriesInstanceUID());
	const char* instanceUID = m_pImage->GetSOPInstanceUID();

	//	Transaction Log
	LogMessage(kDebug,"TRANS: CoerceSOPInstanceUID() - Attempting to store SOPInstanceUID = %s\n", 
		instanceUID);

	///
	// you can check modality here
	//
	switch(m_pImage->GetModality())
	{
	case kUS:
	break;
	case kCT:
	break;
	case kPX:
	break;
	default:
	break;
	}



	/* Reject bad dicom files   
	 * TODO
	 */
	if (!IsValidDICOMUID(m_seriesUID) || 
		!IsValidDICOMUID(instanceUID))
	{
		LogMessage("ERROR:[C%08d] ErrorDetected: Invalid SeriesUID(%s) or SOPUID(%s)\n", DicomServError_InvalidDicomUID,m_seriesUID, instanceUID);
		FlushLog();
	}


	bool firstImage = false;
	bool oldSeriesDir = false, oldCacheDir = false;

	// setup series monitor and directories
	RTVInactiveManager& imanger=RTVInactiveManager::theManager();
	SeriesDirMonitor* pSeriesDirMonitor = (SeriesDirMonitor*)imanger.LockHandler(m_seriesUID, this, true);
	if(pSeriesDirMonitor != 0)
	{
		// set the directories from monitor
		ASTRNCPY(m_seriesDir, pSeriesDirMonitor->GetWatchDir());
		ASTRNCPY(m_cacheDir, pSeriesDirMonitor->GetCacheDir());
	}
	else
	{
		// lock whole manager
		//TRCSLock fplock(imanger.AquireLockObject());

		// new first file coming, check which dirtectory to use for store data
		std::string studyUIDstr = studyUID ;
		std::string seriesUIDstr = m_seriesUID;
		std::string testSeriesDir, testCacheDir;

		testSeriesDir = RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom(seriesUIDstr, studyUIDstr);
		if(!testSeriesDir.empty())
		{
			oldSeriesDir = true;
			ASTRNCPY(m_seriesDir, testSeriesDir.c_str());
		}
		
 		if(gConfig.m_writeCache !=0){
			testCacheDir = RTVDiskSpaceManager::GetDirectoryToReadCacheFrom(seriesUIDstr, studyUIDstr);
			if(!testCacheDir.empty())
			{
				oldCacheDir = true;
				ASTRNCPY(m_cacheDir, testCacheDir.c_str());
			}
		}else{
 			oldCacheDir = false;
			ASTRNCPY(m_cacheDir, "");
		}

		if(!oldSeriesDir || !oldCacheDir)
		{
			_status = RTVDiskSpaceManager::GetAvailableMedia(kSeriesReserveSpace, testSeriesDir, testCacheDir);
			if (_status != RTVDiskSpaceManager::kOK)
			{
				LogMessage("ERROR:[C%08d] CoerceSOPInstanceUID - GetAvailableMedia failed for %dMB. on series:%s\n", 
					DicomServError_InvalidMediaPoint,
					kSeriesReserveSpace, m_seriesUID);
				FlushLog();
				m_errorResponseStatus = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
				return MC_SYSTEM_ERROR;		
			}
			
			if(!oldSeriesDir)
			{
				sprintf (m_seriesDir, "%s%s/%s", testSeriesDir.c_str(), studyUID, m_seriesUID); 
				
			}
			
			if(!oldCacheDir)
			{
				sprintf (m_cacheDir, "%s%s/%s", testCacheDir.c_str(), studyUID, m_seriesUID); 
				
			}
		}
		if(gConfig.m_writeCache ==0){ //K.Ko
			ASTRNCPY(m_cacheDir, "");
		}

		bool doCompress = false;
		std::string SOPClassUID = m_pImage->GetSOPClassUID();
		if(IsSOPClassUIDCT(SOPClassUID) || IsSOPClassUIDMR(SOPClassUID))
			doCompress = true;
	
		//Launch Series Monitor
		char patientID[100];
		MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENT_ID, kVR_LO, patientID);
		SeriesDirMonitor* pNewSeriesDirMonitor = new SeriesDirMonitor(*(ConnectInfo()), patientID,
			m_seriesDir, m_cacheDir, kDicomImage, m_seriesUID, doCompress);
		
		if(gConfig.m_traceDicomLevel >=2){ //#693
			char patientName[100];
			MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENTS_NAME, kVR_LO, patientName);

			std::string str_temp;
			AutoRoutingAEMan::ReformatJapaneseDicom(patientName, str_temp);

			LogMessage( "INFO:[C%08d]  create new SeriesMonitor : patientName:%s, id:%d, sereisUID:%s \n", 
				DicomServInfor_CStoreInfo,
				str_temp.c_str(),//patientName ,
				patientID,
				m_seriesUID);
			FlushLog();
		}

		//let RTVInactiveManager owns monitor now
		imanger.Handover(m_seriesUID, pNewSeriesDirMonitor);
		
		// query the monitor again in case someone did before this one
		pSeriesDirMonitor = (SeriesDirMonitor*)imanger.LockHandler(m_seriesUID, this, true);

		if(!pSeriesDirMonitor)
		{
			LogMessage( "ERROR:[C%08d] CoerceSOPInstanceUID() - Can not get series Monitor (SOPInstanceUID = %s)\n", 
				DicomServError_SeriesMonitorError,
				instanceUID);
			FlushLog();

			m_errorResponseStatus = C_STORE_FAILURE_INVALID_DATASET;
			return m_errorResponseStatus;
		}
			

		if(pSeriesDirMonitor == pNewSeriesDirMonitor)
			firstImage = true;
		

		// update the directories from monitor, in case someone did before this one
		ASTRNCPY(m_seriesDir, pSeriesDirMonitor->GetWatchDir());
		ASTRNCPY(m_cacheDir, pSeriesDirMonitor->GetCacheDir());

		if(!oldSeriesDir && firstImage)
		{
			
			int count = 4;
			do
			{
				_status = TRPlatform::MakeDirIfNeedTo(m_seriesDir);
				if (_status < 0)
				{
					Sleep(50);
				}

			} while(_status < 0 && count-- > 0);

			if(count != 4 && _status == 0)
			{
				GetAqLogger()->LogMessage(/*kInfo, */"(%d) - makedir success in retry: (%d) dir %s on SOPInstanceUID = %s\n", 
					m_connectInfo.AssociationID, count, m_seriesDir, instanceUID);
				m_errorResponseStatus = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
			}


			if (_status < 0)
			{
				GetAqLogger()->LogMessageWithSysError("ERROR:[C%08d] (%d) - fail to pre-create dir %s on SOPInstanceUID = %s\n", 
					DicomServError_SaveDicomError,
					m_connectInfo.AssociationID, m_seriesDir, instanceUID);
				m_errorResponseStatus = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
				//
				FlushLog();

				return MC_SYSTEM_ERROR;
			}
		}
		
		LogMessage(kDebug,"TRANS: InitMessageStore() - Created directory %s\n", m_seriesDir);

		//	Build file name
		//strcat(m_seriesDir, "/"); 


		//	Check for existence of the Cache sub-directory

		// added by shiying hu, 2005.3.17
		// do not create cache dir if writeCacheForCRImage == 0 and the current modality is CR
		if ( (gConfig.m_writeCacheForCRImage != 0 || m_pImage && m_pImage->GetModality() != kCR) &&
			!oldCacheDir && firstImage)
		{
			if(gConfig.m_writeCache ==0){ //K.Ko
				//Cacheフォルダ作成しない。
			}else{
			
				_status = TRPlatform::MakeDirIfNeedTo(m_cacheDir);
				if (_status < 0)
				{
					GetAqLogger()->LogMessageWithSysError("WARNING:[C%08d]: (%d) - fail to pre-create dir %s on SOPInstanceUID = %s\n", 
						DicomServError_SaveDicomError,
						m_connectInfo.AssociationID, m_cacheDir, instanceUID);
				}
				LogMessage(kDebug,"TRANS: Preprocess() - Created directory %s\n", m_cacheDir);
			}
		}
		
	}
	
	
//GATE_ONLY
	//return MC_NORMAL_COMPLETION;
 
	if (gConfig.m_bypassDatabaseQuery)
		return MC_NORMAL_COMPLETION;

	static std::string testFailSOPUID;
	if ((gConfig.m_failStoreOneImage || gConfig.m_failStoreOneImagePermanently) && firstImage)
	{
		testFailSOPUID = instanceUID;
		m_errorResponseStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
		SetErrorResponseReason("TEST ONLY!! First image rejected");
		return MC_SYSTEM_ERROR;
	}

	if (gConfig.m_failStoreOneImagePermanently && !testFailSOPUID.compare(instanceUID))
	{
		m_errorResponseStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
		SetErrorResponseReason("TEST ONLY!! First image rejected permanently");
		return MC_SYSTEM_ERROR;
	}

	//	Check to see if this uid is a new uid in the process and db
	if ( pSeriesDirMonitor->AddInstanceUID(instanceUID) )
	{
		m_imageDBSaved = 0; // mark the image is in memory and start to process
		return MC_NORMAL_COMPLETION;
	}
	else
	{

#ifdef USE_AUX_DATA //#28 2012/06/20 K.Ko
		//for private data, let it in
		AuxData auxData;
		if (auxData.Init(m_messageID, studyUID, m_seriesUID, instanceUID) == 0)
		{
			m_imageDBSaved = 99; // mark the image is in DB
			return MC_NORMAL_COMPLETION;
		}
#endif


	}

	//	 - 03/27/03
	if (gConfig.m_overwriteDuplicates == 829)
	{
		if(!firstImage)
		{
			// this should not happen, the first image should trigger all instances cleaned up
			LogMessage("ERROR: CoerceSOPInstanceUID() - overwrite sereis on non-first image\n", m_seriesDir);
			//m_errorResponseStatus = C_STORE_FAILURE_DUPLICATE_SOP;
			//return MC_SYSTEM_ERROR;
		}

		
		//	In order to overwrite duplicates, we will blow away the entire series. 
		//		Very dangerous, since the sender may have accidentally assigned the same
		//		UID to two different instances, maybe even belonging to different patients!
		//		But we need this for now to solve bug# 3496.

		LogMessage(kWarning,"WARNING: encountered duplicate instance %s\n", instanceUID);
		LogMessage(kWarning,"WARNING: Deleting series %s\n", m_seriesUID);

		AppComUtil::DeleteSeries(&m_db, studyUID, m_seriesUID);
		pSeriesDirMonitor->ClearInstanceMap();

		try
		{
			// tczhao 2005.08.04
			// update the study information for PIR 
			DICOMData studyInfo;
			m_pImage->FillPatientInfo(studyInfo);
			if (m_db.UpdateStudyInfo(studyInfo) != kOK)
			{
				LogMessage(kWarning,"WARNING: Update StudyInfo failed in Overwrite duplicates. studyUID=%s\n",studyUID);
			}
		}
		catch (...) 
		{
			LogMessage(kWarning,"WARNING: Overrwite studyInfo threw exception\n");
		}

		
		//InitMessageStore(); // re-create delete directory
		_status = TRPlatform::MakeDirIfNeedTo(m_seriesDir);
		if (_status < 0)
		{
			LogMessage("ERROR: CoerceSOPInstanceUID() - failed to make dir %s\n", m_seriesDir);
			m_errorResponseStatus = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
			return MC_SYSTEM_ERROR;
		}

		
		if(!pSeriesDirMonitor->AddInstanceUID(instanceUID))
			return MC_SYSTEM_ERROR;

		m_imageDBSaved = 0; // had the image and start to process
	}
	else if (!gConfig.m_changeDuplicateUIDs)
	{
		//	So we reject and move on
		LogMessage(kWarning, "WARNING: (in series %s) Duplicate SOP Instance = %s Will Not Be Stored\n", m_seriesUID, instanceUID);
		
		if (gConfig.m_sendErrorForDuplicateUID)
		{
			m_errorResponseStatus = C_STORE_FAILURE_DUPLICATE_SOP;
			SetErrorResponseReason("Duplicate SOP Instance not stored");
		} else
		{
			m_errorResponseStatus = C_STORE_SUCCESS;
		}
		
		//m_imageDBSaved = -1; // reject the image.
		return MC_SYSTEM_ERROR;
	} 
	else
	{
		//	We need to come up with a new uid and insert that one
		std::string newUID = TRPlatform::GenerateUID();

		LogMessage(kDebug, "TRANS: CoerceSOPInstanceUID() - Duplicate SOP Instance %s was changed to %s\n", 
			instanceUID, newUID.c_str());
		m_pImage->SetSOPInstanceUID(newUID.c_str());
		if (MC_Set_Value_From_String(m_messageID, MC_ATT_SOP_INSTANCE_UID, newUID.c_str()) != MC_NORMAL_COMPLETION)
		{
			LogMessage(kWarning,"WARNING: CoerceSOPInstanceUID() - failed to set changed UID back into message object for UID = %s\n", 
				instanceUID);
		}

		m_pImage->SetSOPInstanceUID(newUID.c_str());
		instanceUID = m_pImage->GetSOPInstanceUID();
		if(!pSeriesDirMonitor->AddInstanceUID(instanceUID))
			return MC_SYSTEM_ERROR;

		m_imageDBSaved = 0; // had the image and start to process

		//strncpy(m_sopInstanceUID, m_UID, kVR_UI);
	}
}catch(...) // #11  2012/03/23 K.Ko
{
	LogMessage("ERROR:[C%08d]  : [Exception] CStore::CoerceSOPInstanceUID \n", 
			DicomServError_Exception);
	FlushLog();
	return MC_ERROR;
}


	return MC_NORMAL_COMPLETION;
}



#ifdef _PROFILE
#include "ScopeTimer.h"
#endif
//-----------------------------------------------------------------------------

int  CStore::Process()
{
	BeginNewProFile();

	m_state = kEnterProcess;
#ifdef _PROFILE
	ScopeTimer("CStore::Process()");
#endif

	// Check to see if this thread needs to run
	if (!m_startThread) return 0;

	bool first=false;
	//	Transaction Log
	LogMessage(kDebug,"TRANS: Processing C-STORE request from (%s %s) to %s\n", 
		m_connectInfo.RemoteApplicationTitle, m_connectInfo.RemoteHostName, m_connectInfo.LocalApplicationTitle); 
	AqCOMThreadInit comInitGuard;

	 
	int status = -1;
	{
		BeginNewProFile();
		status = theProcess();
		LogNewProfile("theProcess") ;
	}

	//#16 2012/04/26 K.Ko
	if(status == MC_NORMAL_COMPLETION)
	{
		if(!doPostJob()){
			LogMessage(kDebug,"doPostJob error\n");
			FlushLog();
	
		}
	}

	CleanUp();
	
	if(status != MC_NORMAL_COMPLETION)
	{
		//SendResponseMessage(m_errorResponseStatus, C_STORE_RSP); // send over in other thread not work
		//MC_Free_Message(&m_messageID); //it should be freed by m_pImage
	}

	RTVInactiveManager::theManager().LockHandler(m_seriesUID, this, false);

	LogNewProfile("Process") ;

	m_state = kLeaveProcess;
	return status;
}

//-----------------------------------------------------------------------------
// This function handles Terarecon specific files that are pushed
//
int CStore::HandleTerareconSpecific ()
{
#ifdef USE_AUX_DATA //#28 2012/06/20 K.Ko
//GATE_ONLY
//	return RTVDiCOMStore::kContinue;
 

	m_state = kHandleTerareconSpecific;

	AuxData auxData;
	//	Do we have any aux data?
	if (auxData.Init(m_messageID, m_pImage->GetStudyInstanceUID(), m_pImage->GetSeriesInstanceUID(), m_pImage->GetSOPInstanceUID()) != 0)
	{	
		return RTVDiCOMStore::kContinue;	
	}

	LogMessage(kDebug,"TRANS: HandleTerareconSpecific() - TeraRecon Private Data encountered\n");
	m_hasAuxData = true;

	//	Update the database
	int dbstat;
	if ((dbstat = m_db.SaveAuxDataInfo(auxData.m_auxData, auxData.m_auxReferencs)) != kOK)
	{
		if(dbstat == kDBOutResource || dbstat == kDBTimeout)
			SaveForRetry();

		LogMessage("ERROR:[C%08d] Can't save AuxDataInfo: %s\n", DicomServError_SaveDBError,auxData.m_auxData.m_name);
		return RTVDiCOMStore::kDoNotContinue;
	}

/*	int     
	int		GetAuxDataInfo(std::string iSeriesInstanceUID, std::vector<AuxDataInfo>& oVal);
	int		GetAuxRefererce(std::string iAuxSOPInstanceUID, std::vector<AuxReference>& oVal);
	int		HasAuxData(std::string iOrigStudyInstanceUID, int& oMask);*/

	return SaveBinaryData(auxData.m_auxData, auxData.IsOldCaScore());
#else
	return -1;
#endif

}

#ifdef _DEBUG
static int imagecount = 0;
#endif

//-----------------------------------------------------------------------------
//
int CStore::theProcess()
{
	int	status = -1;

try{ // #11  2012/03/23 K.Ko

	LogMessage(kDebug, "DEBUG: CStore::theProcess() - Enter\n");

	sprintf(m_shortfileName,"%05d_%s.dcm",m_pImage->GetInstanceNumber(), m_pImage->GetSOPInstanceUID());
    sprintf(m_fileName, "%s/%05d_%s.dcm", m_seriesDir, 
		m_pImage->GetInstanceNumber(), m_pImage->GetSOPInstanceUID());
 
	//
	//	Write the image to a file
	//
	//Store_Pool.Add(m_messageID, this);
	{
		BeginNewProFile()   ;

		status = m_pImage->ConvertToFile(m_fileName, m_connectInfo.LocalApplicationTitle);
		if (status != MC_NORMAL_COMPLETION)
		{
			// make sure directory made
			int _status, count = 4;
			do
			{
				_status = TRPlatform::MakeDirIfNeedTo(m_seriesDir);
				if (_status < 0)
					Sleep(50);

			} while(_status < 0 && count-- > 0);

			if (_status < 0)
			{
				GetAqLogger()->LogMessageWithSysError("(%d) - CStore::theProcess() fail to create dir %s on SOPInstanceUID = %s\n", 
					m_connectInfo.AssociationID, m_seriesDir, m_pImage->GetSOPInstanceUID());
				m_errorResponseStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
				return status;
			}	

			// try again
			status = m_pImage->ConvertToFile(m_fileName, m_connectInfo.LocalApplicationTitle);
		}
		LogNewProfile("ConvertToFile") ;
	}

	//koko for post response error test
	//status = MC_SYSTEM_ERROR;

	if (status != MC_NORMAL_COMPLETION)
	{
		//	-- - 2005.10.17 - The file may have been partially created.
		//		We aren't going to create a db entry for this file, so let's delete it to be sure.
		TRPlatform::remove(m_fileName);

		LogMessage( "ERROR:[C%08d] CStore::Process() - DcmLib error (%d, %s) at ConvertToFile - on series %s on SOPInstanceUID = %s\n",
			DicomServError_SaveDicomError,
			status, MC_Error_Message((MC_STATUS)status), 
			m_pImage->GetSeriesInstanceUID(), m_pImage->GetSOPInstanceUID());
		FlushLog();
		m_errorResponseStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
		return status;
	}

	LogMessage(kInfo, "INFO: CStore::Process() - Saved file %s\n", m_fileName);

//GATE_ONLY
//	return 0;


	int dbstat = kDBTimeout;

	{
	BeginNewProFile()   ;
	///
		m_pImage->FillSortInfo(m_dbData);

		if (gLogger.GetLogLevel() >= kDebug)
		{
			LogMessage("DEBUG: CStore::theProcess() - Instance %s contains: \n", m_dbData.m_SOPInstanceUID);
			LogMessage("       PatientID          = %s\n", m_dbData.m_patientID);
			LogMessage("       Study Instance UID = %s\n", m_dbData.m_studyInstanceUID);
			LogMessage("       SeriesInstanceUID  = %s\n", m_dbData.m_seriesInstanceUID);
			LogMessage("       SOP Instance UID   = %s\n", m_dbData.m_SOPInstanceUID);
			LogMessage("       Series Number      = %d\n", m_dbData.m_seriesNumber);
			LogMessage("       Instance Number    = %d\n", m_dbData.m_instanceNumber);
			LogMessage("       Patient Name       = %s\n", m_dbData.m_patientsName);
			LogMessage("       Patient Birthdate  = %s\n", m_dbData.m_patientsBirthDate);
			LogMessage("       Study Date         = %s\n", m_dbData.m_studyDate);
			LogMessage("       Study Time         = %s\n", m_dbData.m_studyTime);
			LogMessage("       Accession Number   = %s\n", m_dbData.m_accessionNumber);
			LogMessage("       Study ID           = %s\n", m_dbData.m_studyID);
			LogMessage("       Modality           = %s\n", m_dbData.m_modality);
			LogMessage("       SOP Class UID      = %s\n", m_dbData.m_SOPClassUID);
			LogMessage("       Character Set      = %s\n", m_dbData.m_characterSet);
			LogMessage("       Rows               = %d\n", m_dbData.m_rows);
			LogMessage("       Columns            = %d\n", m_dbData.m_columns);
			LogMessage("       Bits Allocated     = %d\n", m_dbData.m_bitsAllocated);
			LogMessage("       Bits Stored        = %d\n", m_dbData.m_bitsStored);
		}

		//	Set values for database columns not directly derived from DICOM header
		ASTRNCPY(m_dbData.m_stationName,m_connectInfo.RemoteHostName);



		// save to database
		
		if (!gConfig.m_bypassDatabaseInsert)
		{
			if(m_imageDBSaved != 99)
	 			dbstat = m_db.SaveDICOMData(m_dbData);
			else
				dbstat = kOK; // bypass saved instance
			
			if (dbstat != kOK)
			{

				LogMessage("ERROR:[C%08d] CStore::theProcess() - SaveDICOMData() returned %d - "
					"Error: Could not save record in database: record info below on SOPInstanceUID = %s:\n", 
					DicomServError_SaveDBError,
					dbstat, m_dbData.m_SOPInstanceUID);
				LogMessage("       PatientID          = %s\n", m_dbData.m_patientID);
				LogMessage("       Study Instance UID = %s\n", m_dbData.m_studyInstanceUID);
				LogMessage("       SeriesInstanceUID  = %s\n", m_dbData.m_seriesInstanceUID);
				LogMessage("       SOP Instance UID   = %s\n", m_dbData.m_SOPInstanceUID);
				LogMessage("       Series Number      = %d\n", m_dbData.m_seriesNumber);
				LogMessage("       Instance Number    = %d\n", m_dbData.m_instanceNumber);
				LogMessage("       Patient Name       = %s\n", m_dbData.m_patientsName);
				LogMessage("       Patient Birthdate  = %s\n", m_dbData.m_patientsBirthDate);
				LogMessage("       Study Date         = %s\n", m_dbData.m_studyDate);
				LogMessage("       Study Time         = %s\n", m_dbData.m_studyTime);
				LogMessage("       Accession Number   = %s\n", m_dbData.m_accessionNumber);
				LogMessage("       Study ID           = %s\n", m_dbData.m_studyID);
				LogMessage("       Modality           = %s\n", m_dbData.m_modality);
				LogMessage("       SOP Class UID      = %s\n", m_dbData.m_SOPClassUID);
				LogMessage("       Character Set      = %s\n", m_dbData.m_characterSet);
				LogMessage("       Rows               = %d\n", m_dbData.m_rows);
				LogMessage("       Columns            = %d\n", m_dbData.m_columns);
				LogMessage("       Bits Allocated     = %d\n", m_dbData.m_bitsAllocated);
				LogMessage("       Bits Stored        = %d\n", m_dbData.m_bitsStored);

				FlushLog();

				if(dbstat == kDBOutResource || dbstat == kDBTimeout)
				{
					// move m_fileName or save the messsage
					SaveForRetry();
				}
				else
				{	
					TRPlatform::remove(m_fileName);
				}

				m_errorResponseStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
				return MC_SYSTEM_ERROR;
			}
			else
			{
				m_imageDBSaved = 1; // saved the instance
			}
		}
		else
		{
			m_imageDBSaved = 1; // fake the instance saved
		}

	LogNewProfile(" save DB ") ;
	}

	{
	BeginNewProFile()   ;
		// Take care of Ca Score, Reports, etc
#ifdef USE_AUX_DATA  //#28 2012/06/20 K.Ko
		if (HandleTerareconSpecific() == RTVDiCOMStore::kDoNotContinue)
		{
			return MC_SYSTEM_ERROR;
		}
#endif
	LogNewProfile(" HandleTerareconSpecific ") ;
	}

	////2010/03/16 k.ko #660 {[(
	// that'all , finished
	//
	if(m_imageDBSaved!=1){
		return MC_SYSTEM_ERROR;
	}
	return MC_NORMAL_COMPLETION;
	///////////////////////////
	/////
	////2010/03/16 k.ko #660 )]}


//The define is commented to enable controlling the cache writing using the gConfig.m_writeCache flag
// Murali - 2006.01.05

//#ifdef _EX_CACHE_WRITER
//	return 0; //GL 9-4-2003 switch to external cache writer
//#endif

	if (gConfig.m_writeCache != 1 || TerminationRequested() )
	{
		//the user:
		//	-->  do not want to write cache. 
		//  -->  OR do not want to use the in-process cache writer. 
		//  OR Termination Request from the system itself.
		return 0;
	}

	// added by shiying hu, 2005.3.17
	if ( gConfig.m_writeCacheForCRImage == 0 && m_pImage && m_pImage->GetModality() == kCR )
		return 0;


	// Vikram 02/27/02 - Code Review Changes
	// As of 02/27/02 we need to correctly handle the following storage classes
	//
	//		SOP Class Name							SOP Class UID
	//
	//		Computed Radiography					1.2.840.10008.5.1.4.1.1.1
	//		CT Image								1.2.840.10008.5.1.4.1.1.2
	//		MR Image								1.2.840.10008.5.1.4.1.1.4
	//		Secondary Capture Image					1.2.840.10008.5.1.4.1.1.7
	//		X-Ray Angiographic Image				1.2.840.10008.5.1.4.1.1.12.1
	//		X-Ray Radiofluoroscopic Image			1.2.840.10008.5.1.4.1.1.12.2
	//		X-Ray Angiographic Bi-Plane Image		1.2.840.10008.5.1.4.1.1.12.3

	//      Here are the steps used to store the images
	//      o. Figure out type of Image based on the Storage Class UID
	//		o. Get all the PixelData
	//		o. Uncompress any compressed data
	//		o. Handle Each individual Image Type

	unsigned long pixelDataSize = 0;
	if (m_dbData.m_dataSize == 0)
	{
		status = MC_Get_Value_Length(m_messageID, MC_ATT_PIXEL_DATA, 1, &pixelDataSize);
		if (status != MC_NORMAL_COMPLETION)
		{
			LogMessage(kWarning, "WARNING: CStore::Process() - DcmLib error (%d,%s) - failed to get pixel data size on SOPInstanceUID = %s\n", 
				status, MC_Error_Message((MC_STATUS)status), m_pImage->GetSOPInstanceUID());
		
			/* 
			 * Some template files generated by the workstation does not
			 * have an image. Need to have everything go through
			 */
#define  sTemplateStudyUID "2.16.840.1.114053.2100.9.2"
			
			if (strcmp(m_pImage->GetStudyInstanceUID(), sTemplateStudyUID) == 0)
				status = kNormalCompletion;
#undef sTemplateStudyUID

			//	 Some AUX data also don't have pixel data.  It's hard to know
			//		for sure which ones will and which won't.  So we just assume it doesn't have to.
			if (m_hasAuxData)
				status = kNormalCompletion;

			/*  END  */
			return status;
		}
		m_dbData.m_dataSize = pixelDataSize;
	}
	else
	{
		pixelDataSize = m_dbData.m_dataSize;
	}

	bool useIJL = (m_dbData.m_transferSyntax == JPEG_BASELINE);
	int storageType = m_pImage->GetImageStorageType();

	m_OBOWlength = (pixelDataSize > 0) ? pixelDataSize: m_pImage->GetNumberOfBytesOfPixelData();

	status = kContinue;
	int smallestPixelValue, largestPixelValue;
	switch (m_pImage->GetImageStorageType())
	{
		case kCRImage:
		case kDXImage:
			status = HandleCRImage ();
			break;

		case kCTImage:
			status = HandleCTImage ();
			break;

		case kMRImage:
			smallestPixelValue = m_dbData.m_smallestPixelValue;
			largestPixelValue = m_dbData.m_largestPixelValue;
			status = HandleMRImage ();
			if (!gConfig.m_bypassDatabaseInsert )
			{
				if(smallestPixelValue != m_dbData.m_smallestPixelValue ||
					largestPixelValue != m_dbData.m_largestPixelValue)
					dbstat = m_db.UpdatePixelMinMax(m_dbData);
			}

			break;

		case kSCImage:
			status = HandleSCImage ();
			break;

		case kXAImage:
		case kXARFImage:
		case kXABPImage:
			status = HandleXAImage ();
			break;

		case kUSImage:
		case kUSMFImage:
			status = HandleUSImage ();
			break;

		case kNMImage:
			status = HandleNMImage ();
			break;

		case kPTImage:
			status = HandlePTImage ();
			break;
		default:
			LogMessage( "ERROR: CStore::Process() - CacheWriter failed to add image - Unsupported modality %s on SOPInstanceUID = %s\n",
				m_dbData.m_modality, m_dbData.m_SOPInstanceUID);
			break;

	}

}catch(...) // #11  2012/03/23 K.Ko
{
	LogMessage("ERROR:[C%08d]  : [Exception] CStore::theProcess \n", 
			DicomServError_Exception);
	FlushLog();
	status = MC_ERROR;
}

	return status;
}

/*
//-----------------------------------------------------------------------------
// 
int CStore::UpdateInstanceInDB()
{
	char strSQL[4096];
	SQA sqa;
	int retcd;

	SYSTEMTIME t;
	GetLocalTime(&t);	
	sprintf(m_dbData.m_insertDate, "%04d%02d%02d", t.wYear, t.wMonth, t.wDay);
	sprintf(m_dbData.m_insertTime, "%02d%02d%02d", t.wHour, t.wMinute, t.wSecond);

	sprintf(strSQL,"UPDATE InstanceLevel SET \
					seriesInstanceUID = '%s' \
					SOPClassUID = '%s' \
					InstanceNumber = '%s' \
					Rows = '%s' \
					Columns = '%s' \
					ImageType = '%s' \
					BitsAllocated = '%s' \
					BitsStored = '%s' \
					PixelRepresentation = '%s' \
					PatientOrientation = '%s' \
					InsertDate = '%s' \
					InsertTime = '%s' \
					FileName = '%s' \
					SubSeriesNumber = '%s' \
					ImagePosition = '%s' \
					ImagePositionX = '%s' \
					ImagePositionY = '%s' \
					ImagePositionZ = '%s' \
					ImageOrientationPatientXX = '%s' \
					ImageOrientationPatientXY = '%s' \
					ImageOrientationPatientXZ = '%s' \
					ImageOrientationPatientYX = '%s' \
					ImageOrientationPatientYY = '%s' \
					ImageOrientationPatientYZ = '%s' \
					PixelDataOffset = '%s' \
					MediaLabel = '%s' \
					WHERE SOPInstanceUID = '%s'",
					m_dbData.m_seriesInstanceUID, 
					m_dbData.m_SOPClassUID,
					m_dbData.m_instanceNumber,
					m_dbData.m_rows,
					m_dbData.m_columns,
					m_dbData.m_imageTypeTokens,
					m_dbData.m_bitsAllocated,
					m_dbData.m_bitsStored,
					m_dbData.m_pixelRepresentation,
					m_dbData.m_patientOrientation,
					m_dbData.m_insertDate,
					m_dbData.m_insertTime,
					m_dbData.m_fileName,
					m_dbData.m_subSeriesNumber,
					m_dbData.m_imagePosition,
					m_dbData.m_imagePositionX,
					m_dbData.m_imagePositionY,
					m_dbData.m_imagePositionZ,
					m_dbData.m_imageOrientationPatientXX,
					m_dbData.m_imageOrientationPatientXY,
					m_dbData.m_imageOrientationPatientXZ,
					m_dbData.m_imageOrientationPatientYX,
					m_dbData.m_imageOrientationPatientYY,
					m_dbData.m_imageOrientationPatientYZ,
					m_dbData.m_pixelDataOffset,
					m_dbData.m_mediaLabel,
					m_dbData.m_SOPInstanceUID);
	LogMessage(kDebug, "DEBUG: CStore()::UpdateInstanceInDB() - SQL = : %s\n", strSQL);

	sqa.m_options = kDBLockExecute|kDBServerCursor;
	retcd = m_db.SQLExecute(strSQL, sqa);
	if (retcd != kOK)
	{
		LogMessage("ERROR: CStore()::UpdateInstanceInDB() - failed to update instance with SOPInstanceUID = %s", m_dbData.m_SOPInstanceUID);
		return retcd;
	}

	return kOK;
}
*/
//-----------------------------------------------------------------------------
// Handles incoming CR/DR Iimages
int CStore::HandleCRImage ()
{
	char* refSOP = 0;
	std::vector <std::string> ImageTypeTokens;
	ImageTypeTokens.clear ();

	if (GetSingleFrameData () == kDoNotContinue) return kDoNotContinue;

	AppComCacheWriter cacheWriter;
	//	Chop uncompressed pixel buffer into frames
//	int bytesPerPixel = m_dbData.m_bitsAllocated / 8;
//	long bytesPerFrame = m_dbData.m_rows * m_dbData.m_columns * bytesPerPixel;
//	int numberOfFrames = m_uncompressedSize / bytesPerFrame;

    // TC's self-cleaning Critical-Section
    // TC says: "This thing is great man! This thing is really great"
    //TRNamedCSLock localCriticalSection (m_seriesInstanceUID);
	//---------------------------------------------------------------------
	// Add a single XA/XF Image to cache
	m_state = kHandleCRImage_AddXAImage;

	cacheWriter.SetUseModalityLUT(gConfig.m_useModalityLUT);
	int _status = cacheWriter.AddCRDRDXImage (m_cacheDir, 
											  m_dbData.m_modality,
											  m_dbData.m_SOPInstanceUID, 
											  m_dbData.m_SOPClassUID,
											  m_dbData.m_bitsAllocated, 
											  m_dbData.m_bitsStored, 
											  m_dbData.m_highBit, 
											  m_dbData.m_columns, 
											  m_dbData.m_rows, 
											  m_pImage->IsLittleEndian(), 
											  ImageTypeTokens,
											  m_dbData.m_instanceNumber, // 
											  m_dbData.m_pixelRepresentation,
											  m_OBOWbuffer, 
											  (ePhotometricInterpretation) m_dbData.m_photometricInterpretation,
											   0, kNone,
											  m_dbData.m_rescaleSlope,
											  m_dbData.m_rescaleIntercept,
											  m_pImage->GetPixelSpacing (),
											  (int)m_dbData.m_windowWidth,
											  (int)m_dbData.m_windowCenter);

	if (_status != kSuccess)
	{
		LogMessage( "ERROR: HandleCRImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", 
			_status,  m_dbData.m_SOPInstanceUID);
		return _status;
	}

	return kContinue;
}

//-----------------------------------------------------------------------------
// Handles incoming CT images
int CStore::HandleCTImage ()
{

	if (GetSingleFrameData () == kDoNotContinue) return kDoNotContinue;

	AppComCacheWriter cacheWriter;

    // TC's self-cleaning Critical-Section
    // TC says: "This thing is great man! This thing is really great"
    //TRNamedCSLock localCriticalSection (m_seriesInstanceUID);
	m_state = kHandleCTImage_AddCTImage;

	int _status = cacheWriter.AddCTImage (m_cacheDir, 
										m_dbData.m_modality,
										m_dbData.m_SOPInstanceUID, 
                                        m_dbData.m_SOPClassUID,
										m_dbData.m_bitsAllocated, 
										m_dbData.m_bitsStored, 
										m_dbData.m_highBit, 
										m_dbData.m_columns, 
										m_dbData.m_rows, 
										m_pImage->IsLittleEndian(),
										m_pImage->GetImageTypeTokens(),
										m_pImage->GetImagePosition(),
										m_pImage->GetImageOrientation(),
										m_pImage->GetPixelSpacing(),
										m_pImage->GetSliceThickness(),
										m_dbData.m_rescaleSlope,
										m_dbData.m_rescaleIntercept,
										m_dbData.m_instanceNumber, 
										m_dbData.m_pixelRepresentation,
										m_OBOWbuffer,
										0,
										kNone,
										(int)m_dbData.m_windowWidth,
										(int)m_dbData.m_windowCenter,
										m_pImage->GetImageDate(),
										m_pImage->GetImageTime(),
										m_pImage->GetScanOptions(),
										m_pImage->GetManufacturer());


	if (_status != kSuccess)
	{
		LogMessage( "ERROR: HandleCTImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", 
			_status, m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}

	return kContinue;
}

//-----------------------------------------------------------------------------
// Handles incoming MR images
int CStore::HandleMRImage ()
{
	if (GetSingleFrameData () == kDoNotContinue) return kDoNotContinue;

	
	AppComCacheWriter cacheWriter;

    // TC's self-cleaning Critical-Section
    // TC says: "This thing is great man! This thing is really great"
    //TRNamedCSLock localCriticalSection (m_seriesInstanceUID);
	m_state = kHandleMRImage_AddMRImage;
	int _status = cacheWriter.AddMRImage (m_cacheDir, 
										m_dbData.m_modality,
										m_dbData.m_SOPInstanceUID, 
                                        m_dbData.m_SOPClassUID,
										m_dbData.m_bitsAllocated, 
										m_dbData.m_bitsStored, 
										m_dbData.m_highBit, 
										m_dbData.m_columns, 
										m_dbData.m_rows, 
										m_pImage->IsLittleEndian(),
										m_pImage->GetImageTypeTokens(),
										m_pImage->GetImagePosition(),
										m_pImage->GetImageOrientation(),
										m_pImage->GetPixelSpacing(),
										m_pImage->GetSliceThickness(),
										m_dbData.m_instanceNumber,
										m_dbData.m_pixelRepresentation,
										m_OBOWbuffer,
										m_dbData.m_smallestPixelValue,
										m_dbData.m_largestPixelValue,
										0,
										kNone,
										(int)m_dbData.m_windowWidth,
										(int)m_dbData.m_windowCenter,
										m_dbData.m_rescaleSlope,
										m_dbData.m_rescaleIntercept,
										m_pImage->GetImageDate(),
										m_pImage->GetImageTime());

	if (_status != kSuccess)
	{
		LogMessage( "ERROR: HandleMRImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", 
			_status, m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}

	return kContinue;
}

//-----------------------------------------------------------------------------
// Handles incoming SC images
int CStore::HandleSCImage ()
{	
	//	-- - 2005.10.25 - Fix bug #6099: Add support for Multi-Frame SC
	if (m_pImage->GetNumberOfFrames() > 1)
		if (m_pImage->GetModality() == kUS)
			return HandleUSImage();
		else 
			return HandleXAImage();

	if (GetSingleFrameData () == kDoNotContinue) return kDoNotContinue;

	AppComCacheWriter cacheWriter;
    // TC's self-cleaning Critical-Section
    // TC says: "This thing is great man! This thing is really great"
    //TRNamedCSLock localCriticalSection (m_seriesInstanceUID);
	m_state = kHandleSCImage_AddSCImage;
	int _status = cacheWriter.AddSCImage (
						m_cacheDir, 
 						m_dbData.m_modality,
                       //"SC",
						m_dbData.m_SOPInstanceUID,
                        m_dbData.m_SOPClassUID,
						m_dbData.m_bitsAllocated, 
						m_dbData.m_bitsStored, 
						m_dbData.m_highBit, 
						m_dbData.m_columns, 
						m_dbData.m_rows, 
						m_pImage->IsLittleEndian(),
						m_pImage->GetImageTypeTokens(),
						m_OBOWbuffer,
						m_dbData.m_samplesPerPixel,
						(ePhotometricInterpretation) m_dbData.m_photometricInterpretation,
						(ePlanarConfiguration) m_pImage->GetPlanarConfiguration(), 
						kNone,
						(int)m_dbData.m_windowWidth,
						(int)m_dbData.m_windowCenter,
						m_dbData.m_instanceNumber);
	if (_status != kSuccess)
	{
		LogMessage( "ERROR: HandleSCImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", 
			_status, m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}

	return kContinue;
}

//-----------------------------------------------------------------------------
// Handles incoming US images
int CStore::HandleUSImage ()
{
	int status;

	AppComCacheWriter cacheWriter;

	// call MC_Get_Value_To_Function will go through GetPixelData and HandoverPixelData 
	// to fill pixel data into m_OBOWbuffer
	if(TerminationRequested()) return kDoNotContinue;

	status = MC_Get_Value_To_Function(m_messageID, MC_ATT_PIXEL_DATA, this, GetPixelData);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: HandleUSImage() - Error: (%d, %s) on retrieving pixel data on SOPInstanceUID = %s\n", 
			status, MC_Error_Message((MC_STATUS)status), 
			m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}
	
	//	 - 07/25/02 - Decompress a frame at a time to avoid huge peak memory usage
	//	using new compressor with Pegasus supporting

	bool compressed = m_pImage->IsCompressed();
	Compression compr;
	uint8* frameBuffer = 0;
	if (compressed)
	{
		compr.SetUseStandardCodecs(gConfig.m_useStandardCodecs);
		status = compr.SetPixels(m_pImage, m_OBOWbuffer, m_OBOWlength, m_pImage->GetFrameSizeInBytes(), m_dbData.m_bitsStored, (TRANSFER_SYNTAX)m_dbData.m_transferSyntax, m_dbData.m_samplesPerPixel);
		if (status != Compression::kSuccess)
		{
			LogMessage("ERROR: CStore::HandleUSImage() - Decompressor failed at SetPixels - Returned %d\n", status);
			if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
			return kDoNotContinue;
		}

		frameBuffer = 0;
	} 
	else
	{
		frameBuffer = m_OBOWbuffer;
	}
	int numberOfFrames = m_pImage->GetNumberOfFrames();
	long frameSize = m_pImage->GetFrameSizeInBytes();
	long uncompressedBytesReturned = 0L;

	for(int frame = 0; frame < numberOfFrames; frame++)
	{
		if (compressed)
		{
			status = compr.DecodeNextFrame(frameBuffer);
			if (status != kSuccess || !frameBuffer || TerminationRequested())
			{
				LogMessage( "ERROR: HandleUSImage() - Decompressor failed - Returned %d on SOPInstanceUID = %s\n",
					status, m_dbData.m_SOPInstanceUID);
				if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
				return kDoNotContinue;
			} 

			// if ybr, we did both photometric and planar config
			if (m_dbData.m_photometricInterpretation == kYBR_FULL)
			{
				m_pImage->SetPhotometricInterpretation(kRGB);
			//	m_pImage->SetPlanarConfiguration(kRGBRGB);
			}
		}


		if (m_pImage->GetPhotometricInterpretation() == kYBR_FULL)
		{
			//2007.05.31  support YBR_FULL conversion
			//return kDoNotContinue;
			if ( Conversion::ConvertYBR_FULLToRGB(m_pImage) != Conversion::kCnvSuccess )
			{
				LogMessage("Unhandled YBR image\n");
				return kDoNotContinue;
			}
		}
		
		//	 - 06/02/03 - Added check for 3 samples / pixel.  If it's not, we shouldn't go in here
		if (m_dbData.m_samplesPerPixel == 3 && m_pImage->GetPlanarConfiguration() != kRGBRGB)
		{
			status = DeInterlaceColorPlanes(&frameBuffer, m_dbData.m_rows, m_dbData.m_columns);
			
			if (status != 0)
			{
				LogMessage( "ERROR: HandleUSImage() - Failed to de-interlace color planes on SOPInstanceUID = %s\n", 
					m_dbData.m_SOPInstanceUID);
				return kDoNotContinue;
			}
		}

		m_state = kHandleUSImage_AddUSImage;

		status = cacheWriter.AddUSImage (
						m_cacheDir, 
						m_dbData.m_modality,
						m_dbData.m_SOPInstanceUID, 
						m_dbData.m_SOPClassUID,
						m_dbData.m_bitsAllocated, 
						m_dbData.m_bitsStored, 
						m_dbData.m_highBit, 
						m_dbData.m_columns, 
						m_dbData.m_rows, 
						m_pImage->IsLittleEndian(),
						m_pImage->GetImageTypeTokens(),
						frameBuffer,
						m_dbData.m_samplesPerPixel,
						(ePhotometricInterpretation)m_dbData.m_photometricInterpretation,
						kRGBRGB, 
						kNone,
						(int)m_dbData.m_windowWidth,
						(int)m_dbData.m_windowCenter,
						m_pImage->GetPalette());

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
			LogMessage( "ERROR: HandleUSImage() - CacheWriter failed - Returned %d on SOPInstanceUID = %s\n", 
				status, m_dbData.m_SOPInstanceUID);
			return kDoNotContinue;
		}	
	} 

	// release pixels buffer ASAP
	if (m_OBOWbuffer)
	{
		delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
	}
	
	return kContinue;
}


//-----------------------------------------------------------------------------
// Handles incoming XA images
int CStore::HandleXAImage ()
{
	int status;

	AppComCacheWriter cacheWriter;

	// call MC_Get_Value_To_Function will go through GetPixelData and HandoverPixelData 
	// to fill pixel data into m_OBOWbuffer
	if(TerminationRequested()) return kDoNotContinue;
	status = MC_Get_Value_To_Function(m_messageID, MC_ATT_PIXEL_DATA, this, GetPixelData);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: HandleXAImage() - Error: (%d, %s) on retrieving pixel data on SOPInstanceUID = %s\n", 
			status, MC_Error_Message((MC_STATUS)status), 
			m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}
	
	//	- 07/25/02 - Decompress a frame at a time to avoid huge peak memory usage
	//	using new compressor with Pegasus supporting

	bool compressed = m_pImage->IsCompressed();
	Compression compr;
	uint8* frameBuffer = 0;
	if (compressed)
	{
		compr.SetUseStandardCodecs(gConfig.m_useStandardCodecs);
		status = compr.SetPixels(m_pImage, m_OBOWbuffer, m_OBOWlength, m_pImage->GetFrameSizeInBytes(), m_dbData.m_bitsStored, (TRANSFER_SYNTAX)m_dbData.m_transferSyntax, m_dbData.m_samplesPerPixel);
		if (status != Compression::kSuccess)
		{
			LogMessage("ERROR: CStore::HandleXAImage() - Decompressor failed at SetPixels - Returned %d\n", status);
			if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
			return kDoNotContinue;
		}

		frameBuffer = 0;
	} 
	else
	{
		frameBuffer = m_OBOWbuffer;
	}
	int numberOfFrames = m_pImage->GetNumberOfFrames();
	long frameSize = m_pImage->GetFrameSizeInBytes();

	for(int frame = 0; frame < numberOfFrames; frame++)
	{
		if (compressed)
		{
			status = compr.DecodeNextFrame(frameBuffer);
			if (status != kSuccess || !frameBuffer || TerminationRequested())
			{
				LogMessage("ERROR: HandleXAImage() - Decompressor failed - Returned %d on SOPInstanceUID = %s\n", 
					status, m_dbData.m_SOPInstanceUID);
				if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
				return kDoNotContinue;
			} 
		}
		m_state = kHandleXAImage_AddXAImage_1;

		status = cacheWriter.AddXAImage (
				m_cacheDir, 
				m_dbData.m_modality,
				m_dbData.m_SOPInstanceUID, 
				m_dbData.m_SOPClassUID,
				m_dbData.m_bitsAllocated, 
				m_dbData.m_bitsStored, 
				m_dbData.m_highBit, 
				m_dbData.m_columns, 
				m_dbData.m_rows, 
				m_pImage->IsLittleEndian(), 
				m_pImage->GetImageTypeTokens(),
				m_dbData.m_instanceNumber, 
				(int) m_dbData.m_pixelRepresentation,
				frameBuffer, 
				(ePhotometricInterpretation) m_dbData.m_photometricInterpretation,
				m_dbData.m_referencedSOPInstanceUID, 
				(int)m_dbData.m_windowWidth, (int)m_dbData.m_windowCenter);

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
			LogMessage( "ERROR: HandleXAImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", 
				status, m_dbData.m_SOPInstanceUID);
			return kDoNotContinue;
		}

		
	} 
	// release pixels buffer ASP
	if (m_OBOWbuffer)
	{
		delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
	}
	
	return kContinue;
}

// Handles incoming NM images
int CStore::HandleNMImage ()
{
	int id = m_pImage->GetID();

	//	Assume not compressed
	if (m_pImage->IsCompressed())
	{
		LogMessage(kWarning,"WARNING: no support for compressed NM - could not process SOP=%s\n", m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}

	if (GetSingleFrameData () == kDoNotContinue) return kDoNotContinue;

	AppComCacheWriter cacheWriter;

	m_state = kHandleNMImage_AddNMImage;
	int _status;
	unsigned char* framePtr = m_OBOWbuffer;
	long frameSize = m_pImage->GetFrameSizeInBytes();

	//	Process each frame
#if 0
	CNMObject nmObject(m_pImage);
	for(int i=0; i<m_pImage->GetNumberOfFrames(); i++)
	{
		_status = cacheWriter.AddNMImage (m_cacheDir, 
										m_dbData.m_modality,
										m_dbData.m_SOPInstanceUID,
										m_dbData.m_SOPClassUID,
										m_dbData.m_bitsAllocated,
										m_dbData.m_bitsStored,
										m_dbData.m_highBit, 
										m_dbData.m_columns,
										m_dbData.m_rows,
										m_pImage->IsLittleEndian(),
										m_pImage->GetImageTypeTokens(),
										nmObject,
										m_pImage->GetPixelSpacing(),
										m_dbData.m_rescaleSlope,
										m_dbData.m_rescaleIntercept,
										i,
										m_dbData.m_pixelRepresentation,
										framePtr,
										0,
										kNone,
										m_dbData.m_windowWidth,
										m_dbData.m_windowCenter);

		if (_status != kSuccess)
		{
			LogMessage( "ERROR: HandleNMImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n",
				_status, m_dbData.m_SOPInstanceUID);
			return kDoNotContinue;
		}

		framePtr += frameSize;
	}
#endif
	return kContinue;
}

//-----------------------------------------------------------------------------
// Handles incoming PT images
int CStore::HandlePTImage ()
{
	//	Assume not compressed
	if (m_pImage->IsCompressed())
	{
		LogMessage(kWarning,"WARNING: no support for compressed PET - could not process SOP=%s\n", m_dbData.m_SOPInstanceUID);
		return kDoNotContinue;
	}

	if (GetSingleFrameData () == kDoNotContinue) return kDoNotContinue;

	AppComCacheWriter cacheWriter;

    // TC's self-cleaning Critical-Section
    // TC says: "This thing is great man! This thing is really great"
    //TRNamedCSLock localCriticalSection (m_seriesInstanceUID);
	m_state = kHandlePTImage_AddPTImage;
	int _status;
	unsigned char* framePtr = m_OBOWbuffer;
	long frameSize = m_pImage->GetFrameSizeInBytes();

	//  PET SUV stuff
	PETObjectAttributes attrib;

	for(int i=0; i<m_pImage->GetNumberOfFrames(); i++)
	{
		attrib.Populate(m_pImage);

		_status = cacheWriter.AddPTImage (m_cacheDir, 
										m_dbData.m_modality,
										m_dbData.m_SOPInstanceUID,
										m_dbData.m_SOPClassUID,
										m_dbData.m_bitsAllocated,
										m_dbData.m_bitsStored,
										m_dbData.m_highBit, 
										m_dbData.m_columns,
										m_dbData.m_rows,
										m_pImage->IsLittleEndian(),
										m_pImage->GetImageTypeTokens(),
										m_pImage->GetImagePosition(),
										m_pImage->GetImageOrientation(),
										m_pImage->GetPixelSpacing(),
										m_pImage->GetSliceThickness(),
										m_dbData.m_rescaleSlope,
										m_dbData.m_rescaleIntercept,
										m_dbData.m_instanceNumber,
										m_dbData.m_pixelRepresentation,
										framePtr,
										0,
										kNone,
										m_dbData.m_windowWidth,
										m_dbData.m_windowCenter,
										//
										m_pImage->GetImageDate(),
										m_pImage->GetImageTime(),
										//  PET SUV
										attrib);
		if (_status != kSuccess)
		{
			LogMessage( "ERROR: HandlePTImage() - CacheWriter failed to add image - Returned %d on SOPInstanceUID = %s\n", 
				_status, m_dbData.m_SOPInstanceUID);
			return kDoNotContinue;
		}

		framePtr += frameSize;
	}

	return kContinue;
}

//-----------------------------------------------------------------------------
// This is used when handling CT/MR/CR/DR images which are single frame.
// For XA which can be multi-frame this is handle differently inside of the
// Handle XA Callback
int CStore::GetSingleFrameData ()
{	
	int status;
	// call MC_Get_Value_To_Function will go through GetPixelData and HandoverPixelData 
	// to fill pixel data into m_OBOWbuffer
	status = MC_Get_Value_To_Function(m_messageID, MC_ATT_PIXEL_DATA, this, GetPixelData);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: GetSingleFrameData - Error: (%d, %s) on retrieving pixel data on SOPInstanceUID = %s\n", 
			status, MC_Error_Message((MC_STATUS)status), 
			m_pImage->GetSOPInstanceUID());
		return kDoNotContinue;
	}

	if(!m_pImage->IsCompressed()) //got data in m_OBOWbuffer, just return
		return kContinue;;
	
	Compression compr;
	uint8* frameBuffer = 0;

	//	06/03/03 - Check status - otherwise, we might attempt to decompress even if we don't support the transfer syntax
	compr.SetUseStandardCodecs(gConfig.m_useStandardCodecs);
	status = compr.SetPixels(m_pImage, m_OBOWbuffer, m_OBOWlength, m_pImage->GetFrameSizeInBytes(), m_dbData.m_bitsStored, (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), m_pImage->GetSamplesPerPixel());
	if (status != kSuccess)
	{
		LogMessage("ERROR: GetSingleFrameData - SetPixels() returned %d for transfer syntax %d on SOPInstanceUID = %s\n", 
			status, (TRANSFER_SYNTAX)m_pImage->GetTransferSyntax(), 
			m_pImage->GetSOPInstanceUID());
		if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		return kDoNotContinue;
	}

	status = compr.DecodeNextFrame(frameBuffer);
	if (status != kSuccess || !frameBuffer)
	{
		LogMessage("ERROR: GetSingleFrameData - Decompressor failed - Returned %d on SOPInstanceUID = %s\n", 
			status, m_pImage->GetSOPInstanceUID());
		if(frameBuffer) delete [] frameBuffer, frameBuffer = 0;
		return kDoNotContinue;
	} 

	// release pixels buffer ASP
	if (m_OBOWbuffer)
	{
		delete[] m_OBOWbuffer, m_OBOWbuffer = 0;
	}
	m_OBOWbuffer = frameBuffer; // put uncompressed data to OBOWbuffer

	return kContinue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

		//
		//	Will do these later
		//
/*
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetPatientOrientation( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetBodyPartExamined( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetViewPosition( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImagePosition( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImagePositionX( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImagePositionY( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImagePositionZ( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageOrientationPatientXX( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageOrientationPatientXY( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageOrientationPatientXZ( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageOrientationPatientYX( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageOrientationPatientYY( const double n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageOrientationPatientYZ( const double n );

		//
		//	Meta info
		//
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetNumberOfStudyRelatedSeries( const long n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetNumberOfStudyRelatedInstances( const long n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetNumberOfSeriesRelatedInstances( const long n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetModalitiesInStudy( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetMediaLabel( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetMediaOnlineFlag( const long n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetSavePath( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetFileName( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetQueryLevel( const long n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetStationName( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetOfflineFlag( const long n );

		//
		//	These should not be public - should happen inside of SaveRecord()
		//
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetInsertDate( const char* str );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetInsertTime( const char* str );

		//
		//	Don't know what these are
		//
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetSubSeriesNumber( const long n );
		if (stat[idx++] == MC_NORMAL_COMPLETION) record.SetImageType( const char* str );
		*/
		/*		vData.push_back(record);
		//
		//	Update database
		//
		char filePath[MAX_PATH];
		char fileName[MAX_PATH];
		char savePath[MAX_PATH];

		CString PatientsName;
		CString PatientID;
		CString StuInsUID;
		CString SerInsUID;
		CString SOPInsUID;
		CString Date;
		CString Time;
		//
		// other header
		//

		for( int i=0; i<10; i++ )
		{
			// set dicom info
			vData[i].SetPatientsName( PatientsName );
			vData[i].SetPatientID( PatientID );
			//
			// set other header
			// 
			sprintf( savePath, "%s/%s/", StuInsUID, SerInsUID );
			sprintf( fileName, "%s.dcm", SOPInsUID );
			sprintf( filePath, "c:/TeraReconVolumes/%s%s", savePath, fileName );

			vData[i].SetFileName( fileName );
			vData[i].SetSavePath( savePath );

 			CTime t = CTime::GetCurrentTime();
 			Date.Format( "%04d%02d%02d", t.GetYear(), t.GetMonth(), t.GetDay() );
 			Time.format( "%02d%02d%02d", t.GetHour(), t.GetMinute(), t.GetSecond() );
 			vData[i].SetDate( Date );
 			vData[i].SetTime( Time );

			// get file size
			CFile::GetStatus( filePath, status );
			vData[i].SetFileSize( status.m_size / 1024 );		// set MB
		}

		int status = db.SaveRecord( vData );

*/

//----------------------------------------------------------------------------
// 2006.03.23 a temporary hack to copy a file
static int MoveOrCopyFile(const char* iSrc, const char* iDest)
{
	if (!iSrc || !iDest || !*iSrc || !*iDest)
		return -1;
	
	// try to rename
 	if ( rename(iSrc,iDest) == 0)
 		return 0;
	
	FILE *fdest = fopen(iDest,"wb");
	FILE *fsrc  = fopen(iSrc,"rb");
	
	if (!fsrc || !fdest)
	{
		if (fsrc)  fclose(fsrc);
		if (fdest) fclose(fdest);
		TRPlatform::remove(iDest);
		return -1;
	}
	
	char buf[4096];
	int n;
	
	for ( ;  ( n = fread(buf,1, sizeof buf, fsrc)) > 0 ;)
		fwrite(buf,1, n,fdest);
	
	fclose(fsrc);
	fclose(fdest);

	return 0;
}

//-----------------------------------------------------------------------
void CStore::SaveForRetry()
{
	std::vector<AppComDevice> raids;
	std::string importDir;
	const char* secretImport = "C:/AQNetImport";

	AppComConfiguration::GetArchiveDevices(AppComConfiguration::gkRAIDType,raids);

	if (raids.size() == 0)
	{
		importDir = secretImport;
	}
	else
	{
		importDir = raids[0].GetPathToCacheOnDevice() + "AQNetImport";
	}

	
	if(!TRPlatform::IsDirectory(importDir.c_str()))
	{
		if (TRPlatform::MakeDirIfNeedTo(importDir.c_str()) < 0)
		{
			LogMessage("CStore: failed to create directory %s\n",importDir.c_str());
			return;
		}
		else
		{
			chmod(importDir.c_str(),_S_IREAD | _S_IWRITE);
		}
	}

	char filename[256],dir[256];

	sprintf(dir,"%s/recover/%s/",importDir.c_str(), m_pImage->GetSeriesInstanceUID());

	TRPlatform::MakeDirIfNeedTo(dir);

	sprintf(filename, "%s%05d_%s.dcm", dir, m_pImage->GetInstanceNumber(), m_pImage->GetSOPInstanceUID());

	// if the file is saved previously, don't have to do anything
	 if (access(filename,0) == 0)
		 return;
	 
	 if (m_fileName[0])
	 {
		 MoveOrCopyFile(m_fileName, filename);
		 TRPlatform::remove(m_fileName);
		 m_fileName[0] = '\0';
	 }
	 else
	 {
		 assert(0) ; //   2006.03.23
		 int	status = m_pImage->ConvertToFile(filename, m_connectInfo.LocalApplicationTitle);
		 if (status != kNormalCompletion)
		 {
			 LogMessage("CStore: failed (%d) to convert image to file %s. SOPUID=%s\n", status,filename);
			 return ;
		 }
	 }
	
	//   2006.03.23 need this to get potential auto-assignment right
	char controlFile[256];
	strcat(strcpy(controlFile,dir),"control.txt");
	
	if (access(controlFile,0) == 0)
		return;
	
	FILE *fp = fopen(controlFile,"w");
	if (fp)
	{
		fprintf(fp,"AETitle = %s\n", m_connectInfo.LocalApplicationTitle);
		fclose(fp);
	}
	else
	{
		LogMessage("Failed to create control file %s\n", controlFile);
	}
}

int CStore::PreProcessNoneResponce()
{
BeginNewProFile();

	int	status = -1;
try{ // #11  2012/03/23 K.Ko
		 
		status = ProcessHeader(kLateResponce);
		if(MC_NORMAL_COMPLETION != status){
		//	GetAqLogger()->LogMessage("ERROR:[C%08d] CStore::ProcessHeader error \n",DicomServError_InvalidDicomHeader);
		//	GetAqLogger()->FlushLog();
		}
		
}catch(...) // #11  2012/03/23 K.Ko
{
	LogMessage("ERROR:[C%08d]  : [Exception] CStore::PreProcessNoneResponce \n", 
			DicomServError_Exception);
	FlushLog();
	status = MC_ERROR;
}
 
LogNewProfile("PreProcessNoneResponce ");
	 
 
	return status;	
}
int CStore::SuccessResponce(bool error)
{
	if(error || (m_state != kLeaveProcess))
	{
	 
		m_errorResponseStatus = C_STORE_FAILURE_CANNOT_UNDERSTAND;
		MessageReceiveFinish(m_errorResponseStatus);
		m_startThread = false;
	 
	}else{
		m_errorResponseStatus = C_STORE_SUCCESS;
		MessageReceiveFinish(m_errorResponseStatus);
	}

	return m_state;
}

#include "PxQueue.h"
bool CStore::doPostJob()  //#16 2012/04/26 K.Ko
{
 
	BeginNewProFile();

#if 1
	//#20 Series毎にAutoRoutingを行う　2012/05/23　K.KO
	//シリーズ纏めてキュー作成
	//同一シリーズ内、一回のみ判断
	if(!registerSeriesOnRouting()){
		LogMessage("ERROR:[C%08d]  : CStore::registerSeriesOnRouting \n", 
			DicomServError_Exception);
		FlushLog();
		return false;
	}else{
		return true;
	}
#else
	//以下はスライス毎の処理 -- 放棄
	std::vector<std::string> destAEList;

	CPxWorkQueue *pQueueProc=0;
	if(!getAutoRoutingAEs(destAEList ,pQueueProc)){
		return true;
	}

	 
	if(!pQueueProc){
		return false;
	}

	if(!pQueueProc->isValid()){
		std::string db_file_name = std::string(gConfig.m_DB_FolderName) + "\\";
		db_file_name = db_file_name + SQLITE_DB_FILTE;
		pQueueProc->setupDBName(db_file_name);
		pQueueProc->initRes();
	}
	
//	CPxWorkQueue QueueProc;

	int dest_AE_size = destAEList.size();
	
	CPxQueueEntry new_entry;
	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Image;
	//////////
	new_entry.m_StudyInstanceUID	= m_pImage->GetStudyInstanceUID();
	new_entry.m_SeriesInstanceUID	= m_pImage->GetSeriesInstanceUID();
	new_entry.m_SOPInstanceUID		= m_pImage->GetSOPInstanceUID();
	//////////

#if 1
	for(int i=0;i<dest_AE_size;i++){
		new_entry.m_DestinationAE		= destAEList[i];
		//////////
		new_entry.m_Priority			= 1;
		new_entry.m_Status				= 0;//don't care here
		new_entry.m_CreateTime			= time(0);
		new_entry.m_AccessTime			= time(0);

		//#19 2012/05/21 K.Ko
		pQueueProc->createJobID(new_entry,
							false /*newFlag*/ //同じSeries/AEの場合は同一JobID使用
							  );
		pQueueProc->addQueue(new_entry);
	}
#endif

#endif
	LogNewProfile("doPostJob") ;

	return true;
}

#if 0
//bool CStore::isAutoRouting(std::string &destAE/*output*/)
bool CStore::getAutoRoutingAEs(std::vector<std::string> &destAEList/*output*/,CPxWorkQueue * &pQueueProc/*output*/) //#16 2012/04/26 K.Ko
{
	//#17 2012/05/11 K.Ko

	bool ret_b = true;
//	destAE = "DATSERV2_AE";//"FCOLDMOON_AE";
#if 0
	std::map<int, int>  oFilterIDMap;
	oFilterIDMap.clear();
	std::map<int, int> tmpFilterMap;

	//	Get the list of Tag Filter Rules from the database
	std::vector<TagFilterRule> ruleV;
	int status = m_db.GetTagFilterRules(ruleV);
	if (status != kOK || ruleV.size() <= 0) 
	{
		LogMessage(kInfo,"INFO: CStore::doPostJob - found no TagFilter Rules\n");
		FlushLog();
		return false;
	}
#endif;
	RTVInactiveManager& imanger=RTVInactiveManager::theManager();
	SeriesDirMonitor* pSeriesDirMonitor = (SeriesDirMonitor*)imanger.LockHandler(m_seriesUID, this, true);
	if(pSeriesDirMonitor == 0)
	{
		LogMessage("ERROR:[C%08d]  : [Exception] CStore::isAutoRouting  SeriesDirMonitor == 0\n", 
			DicomServError_AutoRoutingError);
		FlushLog();
		ret_b = false;
		return ret_b;
	}
	AutoRoutingAEMan *pAutoRoutingMan = pSeriesDirMonitor->getAutoRoutingMan();
	if(pAutoRoutingMan == 0)
	{
		LogMessage("ERROR:[C%08d]  : [Exception] CStore::isAutoRouting  pAutoRoutingMan == 0\n", 
			DicomServError_AutoRoutingError);
		FlushLog();
		ret_b = false;
		return ret_b;
	}
	if(!pAutoRoutingMan->isRoutingOnSchedule(m_pImage)){
		return false;
	}

	ret_b = pAutoRoutingMan->getAEList(destAEList,m_pImage);

	if(ret_b){
		pQueueProc =pAutoRoutingMan->getWorkQueueInstance();
	}
	return ret_b;
	 
}
#endif
bool CStore::registerSeriesOnRouting()
{

	bool ret_b = true;
	RTVInactiveManager& imanger=RTVInactiveManager::theManager();
	SeriesDirMonitor* pSeriesDirMonitor = (SeriesDirMonitor*)imanger.LockHandler(m_seriesUID, this, true);
	if(pSeriesDirMonitor == 0)
	{
		LogMessage("ERROR:[C%08d]  : [Exception] CStore::isAutoRouting  SeriesDirMonitor == 0\n", 
			DicomServError_AutoRoutingError);
		FlushLog();
		ret_b = false;
		return ret_b;
	}
	AutoRoutingAEMan *pAutoRoutingMan = pSeriesDirMonitor->getAutoRoutingMan();
	if(pAutoRoutingMan == 0)
	{
		LogMessage("ERROR:[C%08d]  : [Exception] CStore::isAutoRouting  pAutoRoutingMan == 0\n", 
			DicomServError_AutoRoutingError);
		FlushLog();
		ret_b = false;
		return ret_b;
	}

	if(gConfig.m_AutoRoutingTrig == AutoRouringTirg_BlockSize){
 
		if(pAutoRoutingMan->isRoutingOnSchedule(m_pImage)){
			pAutoRoutingMan->addImageFileName(m_shortfileName);
			pAutoRoutingMan->setupSeriesFolder(m_seriesDir);
		};
	}else{
		//#20 Series毎にAutoRoutingを行う　2012/05/23　K.KO
		//シリーズ纏めてキュー作成
		//同一シリーズ内、一回のみ判断
		//#21 
		//シリーズ終了タイミングは　TimeOut 及び AssociationClose
	//just register it
		pAutoRoutingMan->isRoutingOnSchedule(m_pImage) ;
	}
	//
	return ret_b;
}