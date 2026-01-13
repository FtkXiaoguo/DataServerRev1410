/***********************************************************************
 * SeriesDirMonitor.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Monitor incoming DICOM series and convert to Vox format when 
 *		timeout has elapsed with no images being added to the series 
 *		directory.
 *
 *	
 *  
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "SeriesDirMonitor.h"
#include "DataMonitor.h"

#include "HandleSeriesComplete.h"
#include "AqCore/TRPlatform.h"
#include "AppComCacheWriter.h"
#include "TRCompressedCacheWriter.h"
#include "JobControl.h"
#include "JobInfo.h"
#include "AqCore/AqString.h"

#include "Globals.h"
//The define is commented to enable controlling the cache writing using the gConfig.m_writeCache flag
// Murali - 2006.01.05
//#ifdef _EX_CACHE_WRITER
#include "RTVCacheWriter.h"
//#endif

#include "AutoRoutingAEMan.h" //#17 2012/05/11 K.Ko
#include "AutoRoutingMonitor.h"
enum
{
	kValidateCacheWithoutCacheReader = 1,
	kValidateCacheWithCacheReader = 2,
};

const char* kAQNetCacheWriter = "AQNetCacheWriter";


//-----------------------------------------------------------------------------------------------------
//
SeriesDirMonitor::SeriesDirMonitor(const DiCOMConnectionInfo& iCInfo, const char* iPatientID, const char* dirName,
		 const char* cacheDir, DicomObjectType iType, const char* iSeriesUID, bool doCompress)
{
	m_forceComplteFlag  =false;//#21 2012/05/29 K.Ko
	m_AutoRoutingMan = new AutoRoutingAEMan ;//#17 2012/05/11 K.Ko
 
if(gConfig.m_AutoRoutingTrig == AutoRouringTirg_BlockSize){
	//別スレッドで監視開始
	m_AutoRoutingMan->setAutoRoutingBlockSize(gConfig.m_AutoRoutingBlockSize);
	CAutoRoutingMonitor::theAutoRoutingMonitor().Handover(iSeriesUID, m_AutoRoutingMan);
}
 
	m_lastActiveTime = 0;
	m_connectInfo = iCInfo;
	m_isDeltaReport = false;
	ASTRNCPY(m_watchedDirectory, dirName);
	ASTRNCPY(m_cacheDir, cacheDir);
	m_objectType = iType;
	
	//m_processorName = "Series Dir Monitor";	
	m_doCompressedCacheWrite = doCompress;
	m_seriesLevelObjectID = 0;
	m_studyLevelObjectID = 0;

	ASTRNCPY(m_seriesInstanceUID, iSeriesUID);

	GetAqLogger()->LogMessage(kInfo,"INFO:(%d|%X) new SeriesDirMonitor on:%s, from %s; %s; %s\n", 
		m_connectInfo.AssociationID, this,
		m_seriesInstanceUID, m_connectInfo.RemoteHostName, m_watchedDirectory, m_cacheDir);

	m_instanceMapInitFlag = 0;
	m_db.InitDatabaseInfo();

#if 0
	if (gConfig.m_doCompressedCache)
	{
		// stop privous started CompressedCacheWriter
		if(TRCompressedCacheWriter::Stop(m_cacheDir) != kSuccess)
			GetAqLogger()->LogMessage("ERROR: SeriesDirMonitor - Could not stop TRCompressedCacheWriter on:%s\n",
				m_cacheDir);
	}
#endif

	GetAqLogger()->LogMessage(kInfo,"The cache writing flag :- %d.\n", gConfig.m_writeCache);


	// add prefix to patient ID for classify out from series UID
	m_fixedPatientID[0] = 0;

#if 0
	//#13 
	//DataMonitor 起動しない
	if(iPatientID && iPatientID[0])
	{
		AqString aString = iPatientID;
		aString.TrimRight(); aString.TrimLeft();
		
		snprintf(m_fixedPatientID, 125, "_~!@PatientID@%s", aString);
		RTVInactiveManager& imanger=RTVInactiveManager::theManager();

		if(imanger.LockHandler(m_fixedPatientID, this, true) == 0) // no data for this patient
		{
			//Launch Data Monitor and hook up data monitor for this one
			DataMonitor* pDataMonitor = new DataMonitor(iCInfo, iPatientID, m_seriesInstanceUID);
			
			//let RTVInactiveManager owns monitor now
			imanger.Handover(m_fixedPatientID, pDataMonitor);
			imanger.LockHandler(m_fixedPatientID, this, true);
		}
		else
		{
			GetAqLogger()->LogMessage(kInfo,"INFO:(%d|%X) lock DataMonitor(%s) on: %s for series %s from %s\n", 
				m_connectInfo.AssociationID, this, m_fixedPatientID, iPatientID, m_seriesInstanceUID, m_connectInfo.RemoteHostName);
		}
	}
#endif

#ifdef USE_JOBPUBLISH
	m_pPublisher = 0;
#endif
	m_SereisCompleteProcessed = false;
}

//-----------------------------------------------------------------------------------------------------
//
void SeriesDirMonitor::CreateJobInfoPublisher(int iNumberOfInstances)
{
#ifdef USE_JOBPUBLISH
	if (m_pPublisher)
		return;

	
	std::vector<int> jobs;
	KVP_MAP progressKVP;
	progressKVP[kJOBKEYtargetProcessName] = "AqNETDICOMServer";
	progressKVP["SeriesUID"] = m_seriesInstanceUID;
	int status = JobInfoSubscriber::QueryJobs(progressKVP, jobs);

	int jobID = (jobs.size() > 0) ? jobs[0] : CJob::GenerateID();
	m_pPublisher = new JobInfoPublisher(jobID);

	if (!m_pPublisher)
		return;
	
	//	Populate the JobInfo keys from the job
	progressKVP[kJOBKEYdisplayName] = "Receive";
	m_pPublisher->SetInfo(progressKVP);
	m_pPublisher->SetStatus(kQWorking);
	m_pPublisher->SetCompleted(iNumberOfInstances);
	m_pPublisher->SetProgress(iNumberOfInstances, 0);
#endif
}

//-----------------------------------------------------------------------------------------------------
//
SeriesDirMonitor::~SeriesDirMonitor()
{
 
if(gConfig.m_AutoRoutingTrig != AutoRouringTirg_BlockSize){
 	delete m_AutoRoutingMan  ;//#17 2012/05/11 K.Ko
}else{
	//別スレッド中に削除される。
}
 
	if(m_fixedPatientID[0] != 0)
	{
		// let data monitor off the hook for this one	
		RTVInactiveManager& imanger=RTVInactiveManager::theManager();
		imanger.LockHandler(m_fixedPatientID, this, false);
	}
	
	if(m_lastActiveTime > 0)
	{
		GetAqLogger()->LogMessage(kInfo,"INFO:(%d|%X) close SeriesDirMonitor on:%s, from %s\n", 
			m_connectInfo.AssociationID, this,
			m_seriesInstanceUID, m_connectInfo.RemoteHostName);
	}
	else
	{
		GetAqLogger()->LogMessage(kInfo,"INFO:(%d|%X) close UNUSED SeriesDirMonitor on:%s, from %s\n", 
			m_connectInfo.AssociationID, this,
			m_seriesInstanceUID, m_connectInfo.RemoteHostName);
	}
#ifdef USE_JOBPUBLISH
	if (m_pPublisher)
	{
		m_pPublisher->SetProgress(m_pPublisher->GetCompleted(), 0);
		m_pPublisher->SetStatus(kQSuccessful);
		delete m_pPublisher, m_pPublisher = 0;
	}
#endif
}


//-----------------------------------------------------------------------------------------------------
//
void SeriesDirMonitor::InitInstanceMap(void)
{
	std::vector<std::string> SOPList;
	m_db.GetSOPUID(m_seriesInstanceUID, SOPList);
	
	TRCSLock fplock(&m_processInstanceMap.m_cs); // lock map, unlock when function return

	for(int i=0;i<SOPList.size();i++)
	{
		m_processInstanceMap.m_map[SOPList[i]] = 1;
	}

	m_instanceMapInitFlag = 1;

	if (gConfig.m_reportProgress)
	{
		CreateJobInfoPublisher(m_processInstanceMap.m_map.size());
	}
}

//-----------------------------------------------------------------------------------------------------
//
bool SeriesDirMonitor::AddInstanceUID(const char* iUID)
{
	bool wasAdded = false;

	if (gConfig.m_bypassDatabaseQuery)
		return true;

	TRCSLock fplock(&m_processInstanceMap.m_cs);
	if(!m_instanceMapInitFlag)
	{
//		DWORD start_time = ::GetTickCount(); 2010/03/15 K.Ko
 		InitInstanceMap();
//		DWORD end_time = ::GetTickCount();
//		GetAqLogger()->LogMessage("INFO: InitInstanceMap time  %d mSec\n", end_time-start_time );
		
	}

	wasAdded = m_processInstanceMap.Add(iUID,1);
	
#ifdef USE_JOBPUBLISH
	if (wasAdded && m_pPublisher)
	{
		int completed = m_pPublisher->GetCompleted() + 1;
		m_pPublisher->SetCompleted(completed);
		if (!(completed % gConfig.m_reportProgressFrequency))
			m_pPublisher->SetProgress(completed, 0);
	}
#endif

	return wasAdded;
}


//-----------------------------------------------------------------------------------------------------
//
bool SeriesDirMonitor::IsTimeOver(DWORD TickCount)
{
	if(m_forceComplteFlag){  //#21 2012/05/29 K.Ko
		m_keepMap.Clear();
		return true;
	}

	long overtime = (TickCount-m_lastActiveTime)/1000 - gConfig.m_seriesCompleteTimeout;


	if (overtime > 3600)
	{
		m_keepMap.Clear(); // clear locks for 1 hour old lock
		return true;
	}
	else
		return (overtime > 0);

}

//-----------------------------------------------------------------------------------------------------
//
void SeriesDirMonitor::ForceTimeOut(void)
{	
	AppComCacheWriter::CloseCache(m_cacheDir);
}


//-----------------------------------------------------------------------------------------------------
//
int SeriesDirMonitor::Process(void)
{	
	int recode = 0;
 
	//	-- - 2005.08.16: db init was ok, so immediately register the new series in the db - 
	//		moved from lower down so this step is not missed by early exit points.
	int status = m_db.AddNewSeries(m_seriesInstanceUID);
	if(status != kOK)
	{
		GetAqLogger()->LogMessage(kInfo,"INFO: SeriesDirMonitor::Process() - AddNewSeries failed for %s\n", m_seriesInstanceUID);
		return 0; 
	}

	recode =  HandleCache();
 

	if(!m_SereisCompleteProcessed)
	{
		m_SereisCompleteProcessed = true;

		if (gConfig.m_inProcessSeriesCompletion == kOutOfProcessSeriesCompletion)
		{
			DoOutOfProcessSeriesCompletion();
		}
		else
		{
			DoInProcessSeriesCompletion();
		}
	}

	if(!CanStop(GetTickCount()))
	{
		GetAqLogger()->LogMessage(kInfo,"INFO: SeriesDirMonitor::Process() - complete abort for potential new incoming image %s\n", m_seriesInstanceUID);
		return -1; // abort series completion
	}

	return recode;
}


int  SeriesDirMonitor::HandleCache(void)
{
#if 1
	//2012/03/02 K.Ko Do not use Cache
	return 0;

#else
	//This change is made to enable controlling the cache writing using the m_cacheWriteFlag flag.
	//Murali 2005.01.05
	if(gConfig.m_writeCache == 0)
	{
		//No reason for the control to be here
	}
	else if (gConfig.m_writeCache == 1)
	{
		//The user requested that the cache be written using the in-process cache writer.  

		//	Close cache file
		if (m_cacheDir[0] != 0)
		{
			int status = TerareconCacheWriter::CloseCache(m_cacheDir);
			if(status != kSuccess)
			{
				GetAqLogger()->LogMessage(kInfo,"INFO: SeriesDirMonitor::Process() - CloseCache cancelled %s\n", m_cacheDir);

				//	TODO: return here will skip post-processes like routing and prefetch.
				//		Need to eliminate this, but don't want to do it yet because not sure of
				//		the consequences.
				return status; // GL 7-31-2002 return error to let inactivemanager try closing again
			}
			
			GetAqLogger()->LogMessage(kInfo,"INFO: SeriesDirMonitor closed cache for dir = %s\n", m_cacheDir);

//			if (!m_dbFlag)
//				return 0;

			// check if new files in pushing
			if(!CanStop(GetTickCount()))
				return -1; // abort series completion

			//	-- - 09/30/02 - Added this in case some errors occured during cache writing, 
			//	we should delete the entire cache.
			bool deleteCache = false;
			int cached = 0, nFrames=0;
			if (gConfig.m_writeCache != 0 )
			{
				deleteCache = CheckCache(cached, nFrames);

				// cache mismatched, check if new files in pushing
				if(deleteCache)
				{
					if(!CanStop(GetTickCount()))
						return -1; // abort series completion

					Sleep(3000);
					deleteCache = CheckCache(cached, nFrames); // retry one more time

					if(!CanStop(GetTickCount()))
						return -1; // abort series completion
					//else // do it
				}

			}

			if (deleteCache)
			{
				if (gConfig.m_keepBadCache)
				{
					GetAqLogger()->LogMessage("ERROR: SeriesDirMonitor::Process() - Number of "
					"cached slices (%d) doesn't match originals (%d) for cache %s\n", 
					cached, nFrames, m_cacheDir);
				}
				else
				{
					GetAqLogger()->LogMessage("ERROR: SeriesDirMonitor::Process() - Number of "
					"cached slices (%d) doesn't match originals (%d) - deleting cache %s\n", 
					cached, nFrames, m_cacheDir);

					//	TODO: didn't handle case where new HandleSeriesComplete starts writing to this cache in between
					//		the time we checked above and here.
					//	TODO: didn't handle case of failed removal - this results in bad cache lying around
					//	status = TRPlatform::RemoveDirectory(m_cacheDir);
				
					// -- 2004.02.18 
					// The above is too aggressive. We may have other things in the cache that
					// do not concern DICOM server
					//
					std::string gonner = m_cacheDir;
					gonner += "/cache.description";
					TRPlatform::remove(gonner.c_str());
					gonner = m_cacheDir;
					gonner  += "/cache.data";
					TRPlatform::remove(gonner.c_str());
					status = !TRPlatform::access(gonner.c_str(),0);
					if (status != 0)
					{
						GetAqLogger()->LogMessage("ERROR: SeriesDirMonitor::Process() - Failed to remove cache %s\n", gonner.c_str());
					}
				}
			} 
			else if(cached != 0)
			{

				//	ASSERT: we have a valid cache - so we can write a compressed cache
				if(m_doCompressedCacheWrite && gConfig.m_doCompressedCache)
				{
 					TRCompressedCacheWriter::Start(m_cacheDir, true);
					m_cacheDir[0] = 0;
				}
			}

		}
	}
	else if(gConfig.m_writeCache == 2)
	{

		// To be removed after initial tests - Murali 01/12/2006
//		GetAqLogger()->LogMessage("Launching the external cache writer.\n");

		//The user requested that the cache be written using the external cache writer.  
		RTVCacheWriter::AddJob(m_watchedDirectory);

	}	// flag controlling cache writing

	else if(gConfig.m_writeCache == 3)
	{
		// Murali 03/07/2006
		GetAqLogger()->LogMessage(kDebug, "Launching the external cache writer through QueueManager.\n");

		// The user requested that the cache be written using the external cache writer controlled by QueueManager.  
		this->CreateAndSubmitCacheWriteJob(m_watchedDirectory);

	}	// flag controlling cache writing!

	return 0;
#endif
}


//-------------------------------------------------------------------
//
static inline bool IsLineBreak(int c) { return c=='\r' || c=='\n';}
int GetValidLine( FILE* fp, char* oBuf, int ioBufLen)
{
	int i, c;

	for (ioBufLen--, i = 0; (c = getc(fp)) != EOF && !IsLineBreak(c) && i < ioBufLen;)
		oBuf[i++] = c;
	oBuf[i] = '\0';
	return c;
}

//-----------------------------------------------------------------------------------------------------
//
int HowManySOPsInCache(FILE* fp)
{
	char lineBuf[200];

	int sopCountInCache = 0;
	while (GetValidLine(fp, lineBuf, sizeof lineBuf) != EOF)
	{
		lineBuf[strlen("SOPInstanceUID")] = 0;
		if (!strcmp(lineBuf,"SOPInstanceUID"))
			 sopCountInCache++;
	}
	
	return sopCountInCache;
}

//-----------------------------------------------------------------------------------------------------
//
#if 0
bool SeriesDirMonitor::CheckCache(int& oCached, int& onFrames)
{
	bool deleteCache = false;

	if (gConfig.m_validateCache == kValidateCacheWithCacheReader)
	{
		AppComCacheReader reader;
		reader.SetIgnoreCompressedCache(!gConfig.m_doCompressedCache);

		int readStatus = reader.ReadCacheDescription (m_cacheDir);
		if (!readStatus)	// Success
			oCached = reader.GetAllSlices().size();
	}
	else if (gConfig.m_validateCache == kValidateCacheWithoutCacheReader)
	{
		char fileName[MAX_PATH];
		strcpy(fileName, m_cacheDir);
		strcat(fileName, "\\Cache.description");
		FILE* fp = fopen(fileName, "rb");
		
		if (fp)
		{
			oCached = HowManySOPsInCache(fp);
			fclose(fp);
		}

	}

	if(oCached > 0)
	{
		int nInstances = m_db.GetNumberOfSeriesRelatedInstances(m_seriesInstanceUID);
		if (oCached != nInstances)
		{
			// sereis might be XA or series recoird hasn't updated yet
			// try frames.
			onFrames = m_db.GetNumberOfSeriesRelatedFrames(m_seriesInstanceUID);
			if (oCached != onFrames)	//	has incorrect frame data.
			{
				//	mismatch - delete cache
				deleteCache = true;
			}
		}
	}
	
	return deleteCache;
}
#endif
//-----------------------------------------------------------------------------
//
#if 0
int SeriesDirMonitor::CreateAndSubmitCacheWriteJob(const char* pDicomDir)
{	
#if 0
	try
	{
		char newJobIDCmd[MAX_PATH];

		// Getting root path of the print job
		std::string CacheWriteQueueRootPath = CJobControl::GetJobIDCreationPath(kAQNetCacheWriter);

		// Generating command line for new Job ID creation
		_snprintf(newJobIDCmd, sizeof(newJobIDCmd), "newjobid -QueueRootPath %s", (CacheWriteQueueRootPath).c_str());

		newJobIDCmd[sizeof(newJobIDCmd)-1] = 0; 

		// Creating a new JobID using the system command
		int jobID = system(newJobIDCmd);

		char idStr[16];
		_snprintf(idStr, sizeof(idStr), "%d", jobID);
		idStr[sizeof(idStr)-1] = 0;
	
		std::string cacheDir = "";
		std::string seriesUID, studyUID;
	

		char *t1, *t2;
		char buf[300];
		strcpy(buf, pDicomDir);

		// get rid of tail slash
		int slen = strlen(buf)-1;
		char tail = buf[slen];
		if (tail == '/' || tail == '\\')
		{
			buf[slen] = 0;
		}

		// get series UID and study UID
		t1 = strrchr(buf, '/');
		t2 = strrchr(buf, '\\');
		if(t1 < t2) 
			{ t1 = t2;}

		if(!t1) 
			{ kErrCacheNotWritten;}

		seriesUID = t1+1;
		*t1 = 0;

		t1 = strrchr(buf, '/');
		t2 = strrchr(buf, '\\');

		if(t1 < t2)
			{ t1 = t2;}

		if(!t1)
			{ kErrCacheNotWritten;}

		studyUID = t1+1;
		
 
		CJob cacheWriteJob;			// CJob describing the job file.
		
		// Generating the job in a temp folder
		std::string tmpDir = "C:/tmp";

		char tmpQueuePath[MAX_PATH];
		_snprintf(tmpQueuePath, sizeof(tmpQueuePath)-1, "%s", CJobControl::GetQueuePath(kAQNetCacheWriter, kQTmp).c_str());
		int len =  strlen(tmpQueuePath);
		if(tmpQueuePath[len-1] == '/')
		{	
			tmpQueuePath[len-1]= 0;	
		}

		int status = cacheWriteJob.Create(idStr, tmpQueuePath, tmpDir.c_str());
		if (status != kJobSuccess)
		{
			GetAqLogger()->LogMessage("ERROR: Create job failed with status %d", status);
			return -1;
		}

		// set the job file with known KeyValue Pairs
		cacheWriteJob.AddKey("srcDir",  pDicomDir);
		cacheWriteJob.AddKey("destDir", m_cacheDir);
		cacheWriteJob.AddKey("seriesUid", m_seriesInstanceUID);
		cacheWriteJob.AddKey("studyUid", studyUID.data());

//		Discussion with Gray: Compression cache is not required and cache writing will be done
//		only for regulat cache - Murali 2006.03.10.
		cacheWriteJob.AddKey("cacheType", "r");	//regular only

		char strtmp[50];
		_snprintf(strtmp, sizeof(strtmp)-1,"%d", gConfig.m_validateCache); 
		cacheWriteJob.AddKey("validateCache", strtmp);

		_snprintf(strtmp, sizeof(strtmp)-1,"%d", gConfig.m_keepBadCache); 
		cacheWriteJob.AddKey("keepBadCache", strtmp);

		
		cacheWriteJob.SetTargetProcessName(kAQNetCacheWriter);
		
		const int cLocked = 1;
		const bool cCreateIfNotThere = true;

		// Saving the job in the temp folder
		cacheWriteJob.Save(cLocked, cCreateIfNotThere);

		// Submitting the job into the request queqe
		cacheWriteJob.Submit(CJobControl::GetRequestPath());
		OutputDebugString("SeriesDirMonitor::CreateAndSubmitCacheWriteJob : CW Job submitted \n");
 
	}
	catch(...)
	{
		GetAqLogger()->LogMessage("ERROR: SeriesDirMonitor::CreateAndSubmitCacheWriteJob(): Threw exception \n");
		OutputDebugString("ERROR: SeriesDirMonitor::CreateAndSubmitCacheWriteJob(): Threw exception \n");
		return -1;
	}

#endif
	return 0;

}
#endif

//-----------------------------------------------------------------------------
//
void
SeriesDirMonitor::DoInProcessSeriesCompletion()
{
	tryAutoRoutingOnSeriesComplete();
#if 0 // Series終了時の処理、自動転送など
	HandleSeriesComplete* pHcsObj = new HandleSeriesComplete();
	pHcsObj->m_applicationID = m_connectInfo.ApplicationID;
	ASTRNCPY(pHcsObj->m_cacheDir, m_cacheDir);
	ASTRNCPY(pHcsObj->m_connectInfo.RemoteIPAddress, m_connectInfo.RemoteIPAddress);
	ASTRNCPY(pHcsObj->m_connectInfo.RemoteApplicationTitle, m_connectInfo.RemoteApplicationTitle);
	pHcsObj->m_connectInfo.ApplicationID = m_connectInfo.ApplicationID;
	pHcsObj->m_connectInfo.RemoteAEAqObjectID = m_connectInfo.RemoteAEAqObjectID;
	ASTRNCPY(pHcsObj->m_connectInfo.RemoteHostName, m_connectInfo.RemoteHostName);
	pHcsObj->m_connectInfo.LocalAEAqObjectID = m_connectInfo.LocalAEAqObjectID;
	ASTRNCPY(pHcsObj->m_connectInfo.LocalApplicationTitle, m_connectInfo.LocalApplicationTitle);
	pHcsObj->m_defaultRoutingCompressionFactor = gConfig.m_defaultRoutingCompressionFactor;
	pHcsObj->m_defaultRoutingCompressionMethod = gConfig.m_defaultRoutingCompressionMethod;
	ASTRNCPY(pHcsObj->m_defaultRoutingTargetAE, gConfig.m_defaultRoutingTargetAE);
	ASTRNCPY(pHcsObj->m_defaultRoutingTargetIP,gConfig.m_defaultRoutingTargetIP);
	pHcsObj->m_defaultRoutingTargetPort = gConfig.m_defaultRoutingTargetPort;
	
	pHcsObj->m_doCompressedCacheWrite = m_doCompressedCacheWrite;
	pHcsObj->m_objectType = m_objectType;
	pHcsObj->m_queryRIS = gConfig.m_queryRIS;
	pHcsObj->m_queueSendJobs = gConfig.m_queueSendJobs;
	if (strlen(gConfig.m_routingLogFile)  > 0)
		ASTRNCPY(pHcsObj->m_routingLogFile, gConfig.m_routingLogFile);
	pHcsObj->m_seriesCompleteTimeout = gConfig.m_seriesCompleteTimeout;
	ASTRNCPY(pHcsObj->m_seriesInstanceUID, m_seriesInstanceUID);
	pHcsObj->m_seriesLevelObjectID = m_seriesLevelObjectID;
	ASTRNCPY(pHcsObj->m_studyInstanceUID, m_seriesInstanceUID);
	pHcsObj->m_validateCache = gConfig.m_validateCache;
	ASTRNCPY(pHcsObj->m_watchedDir, m_watchedDirectory);
	pHcsObj->m_dicomServerLogLevel = gConfig.m_dicomLogLevel;
	pHcsObj->m_logLevel = gConfig.m_dicomLogLevel;
	pHcsObj->m_networkCapture = gConfig.m_networkCapture;
	pHcsObj->m_autoBumpLogLevel = gConfig.m_autoBumpLogLevel;

//GATE_ONLY
//	pHcsObj->m_isGate = 1;
 
	pHcsObj->m_isGate = 0;
 

	//calling the process function directly. we can bypass the commandline parsing

	if (gConfig.m_inProcessSeriesCompletion == kInProcessSeriesCompletion)
	{
		pHcsObj->Process();
		delete pHcsObj;
	}
	else
	{
		RTVOneShotThreadManager::theManager().AddRequest(pHcsObj);
	}


#endif

	return;
}

//-----------------------------------------------------------------------------
//
void
SeriesDirMonitor::DoOutOfProcessSeriesCompletion()
{
	tryAutoRoutingOnSeriesComplete();
#if 0 // Series終了時の処理、外部処理コマンド
	char buf[32];
	int base10 = 10;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO)); 
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = SW_HIDE;

	std::string cmdLineArgs;
	
	if (m_db.IsAPSEnableded())
	{
		cmdLineArgs = "C:/program files/AQAPS/bin/HandleSeriesComplete.exe";
	}
	else
	{
		cmdLineArgs = "C:/program files/AQNet/bin/HandleSeriesComplete.exe";
	}

	cmdLineArgs += " -watchedDir " + std::string(m_watchedDirectory);

	cmdLineArgs += " -cacheDir " + std::string(m_cacheDir);

	cmdLineArgs += " -objectType " + std::string(itoa((int)m_objectType, buf, base10));

	cmdLineArgs += " -doCompressedCache " + std::string(itoa(m_doCompressedCacheWrite, buf, base10));

	cmdLineArgs += " -seriesInstanceUID " + std::string(m_seriesInstanceUID);

	cmdLineArgs += " -seriesLevelObjectID " + std::string(itoa(m_seriesLevelObjectID, buf, base10));

	cmdLineArgs += " -studyLevelObjectID " + std::string(itoa(m_studyLevelObjectID,  buf, base10));

	cmdLineArgs += " -RemoteAEAqObjectID " + std::string(itoa(m_connectInfo.RemoteAEAqObjectID, buf, base10));

	cmdLineArgs += " -LocalAEAqObjectID " + std::string(itoa(m_connectInfo.LocalAEAqObjectID, buf, base10));

	cmdLineArgs += " -RemoteHostName " + std::string(m_connectInfo.RemoteHostName);

	cmdLineArgs += " -RemoteIPAddress " + std::string(m_connectInfo.RemoteIPAddress);

	cmdLineArgs += " -applicationID " + std::string(itoa(m_connectInfo.ApplicationID, buf, base10));

	cmdLineArgs += " -RemoteApplicationTitle " + std::string(m_connectInfo.RemoteApplicationTitle);

	cmdLineArgs += " -localApplicationTitle " + std::string(m_connectInfo.LocalApplicationTitle);

	cmdLineArgs += " -queryRIS " + std::string(itoa(gConfig.m_queryRIS, buf, base10));

	if (strlen(gConfig.m_routingLogFile)  > 0)
		cmdLineArgs += " -routingLogFile " + std::string(gConfig.m_routingLogFile);

	//cmdLineArgs += " -validateCache " + std::string(itoa(gConfig.m_validateCache, buf, base10));

	cmdLineArgs += " -defaultRoutingTargetIP " + std::string(gConfig.m_defaultRoutingTargetIP);

	cmdLineArgs += " -seriesCompleteTimeout " + std::string(itoa(gConfig.m_seriesCompleteTimeout, buf, base10));

	cmdLineArgs += " -defaultRoutingCompressionMethod " + std::string(itoa( gConfig.m_defaultRoutingCompressionMethod, buf, base10));

	cmdLineArgs += " -defaultRoutingCompressionFactor " + std::string(itoa(gConfig.m_defaultRoutingCompressionFactor, buf, base10));

	cmdLineArgs += " -queueSendJobs " + std::string(itoa(gConfig.m_queueSendJobs, buf, base10));

	cmdLineArgs += " -routingTargetAE " + std::string(gConfig.m_defaultRoutingTargetAE);

	cmdLineArgs += " -defaultRoutingTargetPort " + std::string(gConfig.m_defaultRoutingTargetAE);

	cmdLineArgs += " -dicomServerLogLevel " + std::string(itoa(gConfig.m_dicomLogLevel, buf, base10));

	//calling the "handlecompletedseries" process
	int status = CreateProcess(	NULL,					// if no module name (use command line). 
								(char *)cmdLineArgs.data()	,	// Command line. 							
								NULL,					// Process handle not inheritable. 
								NULL,					// Thread handle not inheritable. 
								FALSE,					// Set handle inheritance to FALSE. 
								CREATE_NEW_CONSOLE,		// No creation flags. 
								NULL,					// Use parent's environment block. 
								NULL,					// Use parent's starting directory. 
								&si,					// Pointer to STARTUPINFO structure.
								&pi); 

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	if (status == 0)
	{
		int error = GetLastError();
		GetAqLogger()->LogMessage("The following error occurred while running handleseriescomplete.exe %d", error);
		return;
	}

#endif
}


const char* kJOBKEYdisplayName			= "processDisplayName";
const char* kJOBKEYtargetProcessName	= "targetProcessName";
const char* kJOBKEYrequestingUserID		= "requestingUserID";
const char* kJOBKEYjobID				= "jobID";
const char* kJOBKEYsubmitTime			= "submitTime";
const char* kJOBKEYsourceAE				= "sourceAE";
const char* kJOBKEYdestAE				= "destAE";
const char* kJOBKEYpatientID			= "patientID";
const char* kJOBKEYpatientName			= "patientName";
const char* kJOBKEYseriesNumber			= "seriesNumber";
const char* kJOBKEYaccessionNumber		= "accessionNumber";
const char* kJOBKEYpriority				= "priority";
const char* kJOBKEYstatus				= "status";
const char* kJOBKEYcompleted			= "completed";
const char* kJOBKEYtotal				= "total";
const char* kJOBKEYexecutionTime		= "executeTime";
const char* kJOBKEYcomment				= "comment";
const char* kJOBKEYprecedingProcess		= "precedingProcess";
const char* kJOBKEYAssociatedProcess	= "associatedProcess";
const char* kJobKeyAssociatedProcessID	= "associatedProcessID";
const char* kJobKeyprecedingProcessID	= "precedingProcessID";
const char* kJobKeyPathToInputFile      = "pathToInputFile";


//Series毎にAutoRouting　2012/05/23　K.KO
bool SeriesDirMonitor::tryAutoRoutingOnSeriesComplete()
{
	 
	AutoRoutingAEMan *pAutoRoutingAEMan = getAutoRoutingMan();

	if(!pAutoRoutingAEMan) {
		return false;
	}
 
	if(gConfig.m_AutoRoutingTrig == AutoRouringTirg_BlockSize){
		pAutoRoutingAEMan->setStopFlag(true);//別スレッドの処理の削除
	}else{
 
		return  pAutoRoutingAEMan->tryAutoRoutingOneSeries();
	}
	return true;
 
}
void SeriesDirMonitor::forceComplte() //#21 2012/05/29 K.Ko
{
	m_forceComplteFlag = true;
	RTVInactiveManager& imanger=RTVInactiveManager::theManager();
	imanger.setWakeupEvent();//長時間Ｗａｉｔから目覚まし
}