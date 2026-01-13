/***********************************************************************
 * $Id: HandleSeriesComplete.cpp 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE: Handles Series Complete
 *	
 *
 *	
 *  
 *-------------------------------------------------------------------
 */

//Extracted from SeriesDirMonitor, Sep, 06 - Chetan
//Extracted from SeriesDirMonitor, Jan, 06 - Chetan //new functions & updates

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "HandleSeriesComplete.h"

#include "rtvsutil.h"
#include "AppComUtil.h"

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "AuxData.h"
#include "AppComCacheWriter.h"
#include "TRCompressedCacheWriter.h"
#include "AqCore/TRLogger.h"
#include "diskspacemanager.h"
#include "JobControl.h"

//!!! need to make job create as a executable instead of class
//#include "../QueueManager/Common/PrefetchJob.h"
//#include "../QueueManager/Common/PPrintJob.h"
//#include "SeriesPusher.h"


#include "ProgressAPI.h"
//#include "APS/AqPE/AqProcessEngine.h"

const char*	kSCSOPClassUID = "1.2.840.10008.5.1.4.1.1.7";

TRLogger gRoutingLoggerHCS;
const char* gPEPath = "C:/Program Files/AQAPS/PE/";

//-----------------------------------------------------------------------------------------------------
//
static inline int IsIPAddress(const char* iName)
{
	//	-- - 11/12/04 - Without this check, NULL string 
	//		is considered a valid IP address
	if (!iName || strlen(iName) < 3)
		return 0;

	int yes;
	
	for ( yes = 1; yes && *iName; iName++)
		yes = (isdigit(*iName) || *iName=='.');
	
	return yes;
}


//-----------------------------------------------------------------------------
//
int HandleSeriesComplete::Process()
{	
	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}

	AqCOMThreadInit comInitGuard;
	int filterID;

	std::map<int, int>::iterator iter;

	if(!m_isGate)
	{
		if (m_objectType == kCaScore)
		{
			ExtractCaScore();
			return 0;
		} 
	}

	int status;
	if (m_objectType != kDicomImage)
	{
		GetAqLogger()->LogMessage("ERROR: HandledCompletedSeries::Process() - invalid ojbectType %d\n", m_objectType);
		return -1;
	}
	

	GetAqLogger()->LogMessage(kInfo, "INFO: HandledCompletedSeries started for dir = %s\n", m_cacheDir);

	status = AuditLogReceivedSeries();
	if (status != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: Audit log not created for series: %s\n", m_seriesInstanceUID);
		
	}
	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}

	/*
	//	-- - 2005.08.16: db init was ok, so immediately register the new series in the db - 
	//		moved from lower down so this step is not missed by early exit points.
	status = m_db.AddNewSeries("", 105, m_seriesInstanceUID);
	if(status != kOK)
	{
		GetAqLogger()->LogMessage("INFO: HandleSeriesComplete::Process() - AddNewSeries failed for %s\n", m_seriesInstanceUID);
		return status; 
	}
	*/


	//	Evaluate Tag Filters
	std::vector<std::string> routingLogInfo;
	std::map<int, int> IDsOfFiltersThatPassed;
	int nPassedFilters = GetListOfApplicableTagFilterIDs(IDsOfFiltersThatPassed, routingLogInfo);
	GetAqLogger()->LogMessage(kInfo, "DEBUG: %d Tag Filters passed\n", nPassedFilters);

	
	if (!m_isGate)
	{
		std::string modality = m_progressInfoMap["Modality"];

		if ( (m_db.IsAPSEnableded()) && (strcasecmp((char*)modality.c_str(),"CT") ==0) )
		{
			PraseTagFiltersForDataProcessingPatterns(IDsOfFiltersThatPassed);
		}
		else
		{
			GetAqLogger()->LogMessage(kInfo, "INFO: Job skipped; Modality = %s or APS is not enabled \n", modality.c_str());
		}


		if(TerminationRequested())
		{
			GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
			return  -1 * __LINE__;		
		}

		//	For each filter, get list of Printers
		for(iter = IDsOfFiltersThatPassed.begin(); iter != IDsOfFiltersThatPassed.end(); iter++)
		{
			filterID = iter->first;

			//
			//	Did this filterID have any associated Printers?
			//
			std::vector<FilmingPatternEntry> printerV;

			m_db.GetAutoFilmingPattern(filterID, printerV);
			FilmingPatternEntry printer;

			for(int printerIdx = 0; printerIdx < printerV.size(); printerIdx++)
			{	
				printer = printerV[printerIdx];
				CreateAndSubmitPrintJob(printer.m_printerName, printer.m_displayMode, printer.m_skipN);
			}

			if(TerminationRequested())
			{
				GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
				return  -1 * __LINE__;		
			}

		}
	}
	
	//	For DeltaImaging

	if (m_queryRIS)
	{
		if (m_isDeltaReport)
		{
			std::vector<StudySeries> infos;
			GetDuplicateReportInfos(infos);
			for(int idx = 0; idx < infos.size(); idx++)
			{
				AppComUtil::DeleteSeries(&m_db, infos[idx].m_studyInstanceUID, infos[idx].m_seriesInstanceUID);
			}
		}
		else
		{
			ExecuteReportGrabber();			
		}
	}

	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}

	if(!m_isGate)
	{
		std::vector<int> assignedGroups;
		if (nPassedFilters > 0)
		{
			m_db.InsertIntoGroupSeries(m_seriesInstanceUID, IDsOfFiltersThatPassed, assignedGroups);
			for(int i = 0; i < assignedGroups.size(); i++)
			{
				status = AuditLogAssignSeries(m_seriesInstanceUID, assignedGroups[i]);
				if (status != kOK)
				{
					GetAqLogger()->LogMessage("ERROR: AuditLogAssignSeries not created for series: %s\n", m_seriesInstanceUID);
					
				}

				if(TerminationRequested())
				{
					GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
					return  -1 * __LINE__;		
				}

			}
		}
	}

	std::vector<KVP_MAP> prefetchPatternsV, prefetchQRSourceV;
	KVP_MAP kvpMap;
	std::vector<RoutingAEInfo> routingAEsSubtotal, routingAEs;

	std::string qSQL1 = "";
	qSQL1 += " SELECT ID, Modality, StudyNotOlderThan, UnitType, MaxNumberResults ";
	qSQL1 += " FROM PrefetchPattern ";

	std::string qSQL2 = "";
	qSQL2 += " SELECT RemoteAE.AETitle, RemoteAE.HostName, RemoteAE.IPAddress, RemoteAE.Port ";
	qSQL2 += " FROM PrefetchPattern ";
	qSQL2 += " INNER JOIN PrefetchPatternAE ON PrefetchPatternID = PrefetchPattern.ID ";
	qSQL2 += " INNER JOIN RemoteAE ON RemoteAE.ID = QRSourceAEID ";

	//	For each filter, get list of routingAE's
	for(iter = IDsOfFiltersThatPassed.begin(); iter != IDsOfFiltersThatPassed.end(); iter++)
	{
		filterID = iter->first;

		//
		//	Did this filterID have any associated PrefetchPattern?
		//
		
		if(TerminationRequested())
		{
			GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
			return  -1 * __LINE__;		
		}
		
		if(!m_isGate)
		{
			char tfID[8];
			_snprintf(tfID, sizeof tfID, "%d", filterID);
			tfID[sizeof(tfID)-1] = 0;
			std::string q1 = qSQL1 + " WHERE TagFilterID = '" + std::string(tfID) + "'";
			status = m_db.SQLQuery(q1.c_str(), prefetchPatternsV);

			for(int pfpi = 0; pfpi < prefetchPatternsV.size(); pfpi++)
			{
				kvpMap = prefetchPatternsV[pfpi];
				std::string q2 = qSQL2 + " WHERE PrefetchPattern.ID = '" + kvpMap["ID"] + "'";
				status = m_db.SQLQuery(q2.c_str(), prefetchQRSourceV);
				CreateAndSubmitPrefetchJob(kvpMap, prefetchQRSourceV);
			}
			if(TerminationRequested())
			{
				GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
				return  -1 * __LINE__;		
			}

		}

		//
		//	Did this filterID have any associated routingAE's?
		//
		routingAEsSubtotal.clear();
		status = m_db.GetRoutingAEInfos(filterID, routingAEsSubtotal);
		if (status != kOK && status != kNoResult)
		{
			//	TODO: Get more useful info for log of error condition (which filter failed?)
			GetAqLogger()->LogMessage("ERROR: db error %d while trying to get routingAEs for filterID = %d\n", status, filterID);
			
			continue;
		}
		else if (routingAEsSubtotal.size() <= 0)
		{
			GetAqLogger()->LogMessage(kDebug, "DEBUG: no routingAE's for filterID = %d\n", filterID);
			
			continue;
		}

		//	We got some, so add them to the total list
		if (routingAEsSubtotal.size() > 0) 
		{
			GetAqLogger()->LogMessage(kDebug, "DEBUG: Routing to: ");
			
		}

		for(int i = 0; i < routingAEsSubtotal.size(); i++)
		{
			routingAEs.push_back(routingAEsSubtotal[i]);
			GetAqLogger()->LogMessage(kDebug, "[%s, %s, %s, %d] ", routingAEsSubtotal[i].m_AE.m_AETitle, 
				routingAEsSubtotal[i].m_AE.m_hostName, routingAEsSubtotal[i].m_AE.m_IPAddress, routingAEsSubtotal[i].m_AE.m_port);
			
		}

		if (routingAEsSubtotal.size() > 0) 
		{
			GetAqLogger()->LogMessage("\n");
			
		}

	}

	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}

	if (routingAEs.size() <= 0)
	{
		//	No routingAEs were found based on TagFilters and Schedule - use the default if it's there
		if (strcmp(m_defaultRoutingTargetAE, "NONE") != 0 && IsIPAddress(m_defaultRoutingTargetIP) && 
			m_defaultRoutingTargetPort > 0)
		{
			RoutingAEInfo rae;
			ASTRNCPY(rae.m_AE.m_AEName, m_defaultRoutingTargetAE);
			ASTRNCPY(rae.m_AE.m_AETitle, m_defaultRoutingTargetAE);
			ASTRNCPY(rae.m_AE.m_hostName, m_defaultRoutingTargetIP);
			ASTRNCPY(rae.m_AE.m_IPAddress, m_defaultRoutingTargetIP);
			rae.m_AE.m_IsLocalAE = 0;
			rae.m_AE.m_port = m_defaultRoutingTargetPort;
			rae.m_compressionFactor = m_defaultRoutingCompressionMethod;
			rae.m_compressionMethod = m_defaultRoutingCompressionFactor;

			routingAEs.push_back(rae);
		}
		else if (strcmp(m_defaultRoutingTargetAE, "NONE") != 0 || strcmp(m_defaultRoutingTargetIP, "0.0.0.0") != 0 || m_defaultRoutingTargetPort != 0)
		{
			GetAqLogger()->LogMessage("ERROR: Invalid default routing target: [%s, %s, %d]\n", 
				m_defaultRoutingTargetAE, m_defaultRoutingTargetIP, m_defaultRoutingTargetPort);
			
		}
	}

	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}

	if (routingAEs.size() <= 0)
	{
		//	No routingAEs and no default - if it's GATE, then delete the data

		if(m_isGate)
		{
			//	Only for AqGATE - there are no routing destinations
			//		delete data immediately...no point in keeping it around
			GetAqLogger()->LogMessage(kWarning,"WARNING: DELETING IMAGES - no routing path found for: %s\n", m_watchedDir);
			
			int status = TRPlatform::RemoveDirectory(m_watchedDir);
			if (status != 0)
			{	
				GetAqLogger()->LogMessage("ERROR: failed to cleanup images for Datapath = %s\n", m_watchedDir);
				
			}
			TRPlatform::RemoveParentDirectoryIfEmpty(m_watchedDir);
		}
		return 0;	
	}

	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}

	int r;
	std::string logString;
	if (strlen(m_routingLogFile) > 0)
	{
		//	Log the remote AE names
		//gRoutingLoggerHCS.LogMessage("****\n");
		
		std::string targetList = "Routing to: [";
		for(r = 0; r < routingAEs.size(); r++)
		{
			targetList += routingAEs[r].m_AE.m_AEName;
			if (r+1 < routingAEs.size())
				targetList += ", ";
		}
		//gRoutingLoggerHCS.LogMessage("%s]\n", targetList.c_str());

		//	Log the specific info requested for Aorticare
		for(r = 0; r < routingLogInfo.size(); r++)
		{
			logString += "     " + routingLogInfo[r] + "\n";
			//gRoutingLoggerHCS.LogMessage("     %s\n", routingLogInfo[r].c_str());
		}

		gRoutingLoggerHCS.LogMessage("****\n%s]\n %s ****\n", targetList.c_str(), logString.c_str());
		gRoutingLoggerHCS.FlushLog();
	}

	for(r = 0; r < routingAEs.size(); r++)
	{
		status = AuditLogSendSeries(routingAEs[r].m_AE);
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: AuditLogSendSeries not created for series: %s\n", m_seriesInstanceUID);
			
		}
		if(TerminationRequested())
		{
			GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
			return  -1 * __LINE__;		
		}

	}

	std::vector<std::string> seriesLocation;
	seriesLocation.push_back(m_watchedDir);
/*
	SeriesPusher* pusher = new SeriesPusher(routingAEs, seriesLocation);
	pusher->SetThreadingQueueSize(GetAqLogger()->m_pusherQueueSize);
	RTVOneShotThreadManager::theManager().AddRequest(pusher);
*/

	if(TerminationRequested())
	{
		GetAqLogger()->LogMessage("Stop requested for series completion: %s\n", m_seriesInstanceUID);
		return  -1 * __LINE__;		
	}
	int sendNow = !m_queueSendJobs;
//	SeriesPusher pusher(routingAEs, seriesLocation,sendNow);
//	pusher.Process();

	

	return 0;
}

//-----------------------------------------------------------------------------
//
void 
HandleSeriesComplete::GetSOPInstanceListByGroup(SOPINSTANCE_BY_GROUP &iSopInstanceByGroup)
{
	AppComCacheReader tcr;
	tcr.SetIgnoreCompressedCache(true);
	int cacheStatus = tcr.ReadCacheDescription (m_cacheDir);

#ifndef NONE_GROUPS_IMP
	std::vector<iRTVDICOMGroup> & groups = tcr.GetGroups();

	for ( int groupIndex = 0; groupIndex < groups.size(); groupIndex++ )
	{
		iRTVDICOMGroup group = groups[groupIndex];
		
		if (strcmp(group.GetSOPClassUID().c_str(),kSCSOPClassUID) == 0)
		{
			continue;
		}

		const std::vector <iRTVPtrToSliceInformation>& slicesInfo = group.GetPtrToAllSlices ();

		SOPINSTANCE_LIST  oSOPInstanceList;

		for ( int i = slicesInfo.size()-1; i >= 0; i-- )
		{
			oSOPInstanceList.push_back(slicesInfo[i].GetSOPInstanceUID());
		}

		iSopInstanceByGroup[groupIndex] = oSOPInstanceList;
	}
#endif
}

//-----------------------------------------------------------------------------
//
void HandleSeriesComplete::GetStudyUID(std::string &oStudyUID)
{
	char *t1, *t2;
	char buf[300];
	strcpy(buf, m_watchedDir);

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

	*t1 = 0;

	t1 = strrchr(buf, '/');
	t2 = strrchr(buf, '\\');

	if(t1 < t2)
		{ t1 = t2;}

	if(!t1)
		{ return;}

	oStudyUID = t1+1;

	return;
}

//-----------------------------------------------------------------------------
//
//temporary hack
int HandleSeriesComplete::CreateAndSubmitPEJob(std::string& iPEName, std::string& iStudyInstanceUId, std::string& iSeriesInstanceUID)
{
#if 0
	const int cLocked = 1;

	const bool cCreateIfNotThere = true;

	CJob peJobObject;

	char newJobIDCmd[MAX_PATH];

	// Getting root path of the print job
	std::string PEQueueRootPath = CJobControl::GetJobIDCreationPath(iPEName);

	// Generating command line for new Job ID creation
	_snprintf(newJobIDCmd, sizeof(newJobIDCmd), "newjobid -QueueRootPath %s", (PEQueueRootPath).c_str());
	newJobIDCmd[sizeof(newJobIDCmd)-1] = 0; 

	// Creating a new JobID using the system command
	int id = system(newJobIDCmd);
	char idStr[16];
	_snprintf(idStr, sizeof(idStr), "%d", id);
	idStr[sizeof(idStr)-1] = 0;

	std::string tmpDir = "C:/tmp";
	peJobObject.Create(idStr, CJobControl::GetQueuePath(iPEName, kQTmp).c_str(), tmpDir.c_str());

	peJobObject.SetTargetProcessName(iPEName);

	KVP_MAP addedOptions;

	addedOptions["findingFileDir"] = "C:\\AQNetCache\\AQNetImport"; // place to store xml file, should be in some temporary place
	addedOptions["dicomWrapperDir"] = "C:\\AQNetCache\\AQNetImport"; // place to store dicom file, could be under aqImport directory
	addedOptions["outputMaskBytes"] = "0"; 
	addedOptions["deleteCOFFile"] = "1"; 
	addedOptions["PEOutputSeriesUID"] = TRPlatform::GenerateUID(); 
	addedOptions["SeriesInstanceUID"] = iSeriesInstanceUID;
	addedOptions["StudyID"] = iStudyInstanceUId;
	addedOptions["Start"] = "0";
	addedOptions["End"] = "265";
	addedOptions["groupID"] = "0";
	addedOptions["outputMaskBytes"] = "0";

	KVP_MAP::iterator iter;
	for(iter = addedOptions.begin(); iter != addedOptions.end(); iter++)
	{
		peJobObject.AddKey(iter->first.c_str(), (char*) iter->second.c_str());
	}

	// Saving the job in the temp folder
	peJobObject.Save(cLocked, cCreateIfNotThere);

	// Submitting the job into the request queqe
	peJobObject.Submit(CJobControl::GetRequestPath());

#endif
	return 0;

}

//-----------------------------------------------------------------------------
//
int HandleSeriesComplete::CreateAndSubmitPEJob(std::string& iPEName, std::string& iStudyInstanceUId, std::string& iSeriesInstanceUID, std::vector<std::string>& iSopInstanceList)
{
#if 0
	try
	{
		CJob peJobObject;

		AqVolumeIDInfoVector volumeInfo;

		volumeInfo.AddNew();

		volumeInfo[0].m_studyUID = iStudyInstanceUId.c_str();
		volumeInfo[0].m_seriesUID = iSeriesInstanceUID.c_str();
		volumeInfo[0].m_instanceCount = iSopInstanceList.size();
		volumeInfo[0].m_dataType = "UnKnown";
		volumeInfo[0].m_sortOrder = AqVolumeIDInfo::kAqHeadToFoot;

		for (int i=0;i<iSopInstanceList.size();i++)
		{
			volumeInfo[0].m_instances.push_back(iSopInstanceList[i].c_str());
		}

		char newJobIDCmd[MAX_PATH];
		char *pch = strtok ((char *)iPEName.data(),".");
		if (pch != NULL)
		{	   
			iPEName = pch;
		}	

		// Getting root path of the print job
		std::string PEQueueRootPath = CJobControl::GetJobIDCreationPath(iPEName);

		// Generating command line for new Job ID creation
		_snprintf(newJobIDCmd, sizeof(newJobIDCmd), "newjobid -QueueRootPath %s", (PEQueueRootPath).c_str());
		newJobIDCmd[sizeof(newJobIDCmd)-1] = 0; 

		// Creating a new JobID using the system command
		int id = system(newJobIDCmd);
		char idStr[16];
		_snprintf(idStr, sizeof(idStr), "%d", id);
		idStr[sizeof(idStr)-1] = 0;

		KVP_MAP infoKeys;

		infoKeys[kJOBKEYtargetProcessName]	= iPEName;

		std::string patientID = m_progressInfoMap["PatientID"];
		infoKeys[kJOBKEYpatientID] = patientID;

		std::string patientName = m_progressInfoMap["PatientsName"];
		infoKeys[kJOBKEYpatientName] = patientName;

		std::string accessionNumber = m_progressInfoMap["AccessionNumber"];
		infoKeys[kJOBKEYaccessionNumber] = accessionNumber;

		std::string seriesNumber = m_progressInfoMap["SeriesNumber"];
		infoKeys[kJOBKEYseriesNumber] = seriesNumber;

		JobInfoPublisher publisher(id);
		publisher.SetInfo(infoKeys);

		std::string tmpDir = "C:/tmp";
		peJobObject.Create(idStr, CJobControl::GetQueuePath(iPEName, kQTmp).c_str(), tmpDir.c_str());

		std::string workingFolder = CJobControl::GetWorkingFolderRoot(idStr);

		TRPlatform::MakeDirIfNeedTo(workingFolder.c_str());

		AqVolumeIDInfo::WriteInputVolumeIDInfo(volumeInfo, workingFolder.c_str());
		
		const int cLocked = 1;
		const bool cCreateIfNotThere = true;

		peJobObject.AddKey(kJobKeyPathToInputFile, (workingFolder + "VolumeInfo.txt").c_str());
	
		peJobObject.SetTargetProcessName(iPEName);
		peJobObject.SetTargetProcessPath(gPEPath);
 

		// Saving the job in the temp folder
		peJobObject.Save(cLocked, cCreateIfNotThere);

		// Submitting the job into the request queqe
		peJobObject.Submit(CJobControl::GetRequestPath());
	}
	catch(...)
	{
		GetAqLogger()->LogMessage("ERROR: HandleSeriesComplete::CreateAndSubmitPEJob(): Threw exception \n");
	}

#endif
	return 0;
}

//-----------------------------------------------------------------------------
//
int HandleSeriesComplete::CreateAndSubmitPrintJob(std::string iPrinterName, std::string iDisplayMode, int iskipN)
{
#if 0
	CCPrintJob printObj;
	
	char newJobIDCmd[MAX_PATH];

	// Getting root path of the print job
	std::string PrintQueueRootPath = CJobControl::GetJobIDCreationPath("AqNetPrint");

	// Generating command line for new Job ID creation
	_snprintf(newJobIDCmd, sizeof(newJobIDCmd), "newjobid -QueueRootPath %s", (PrintQueueRootPath).c_str());
	newJobIDCmd[sizeof(newJobIDCmd)-1] = 0; 

	// Creating a new JobID using the system command
	int id = system(newJobIDCmd);
	char idStr[16];
	_snprintf(idStr, sizeof(idStr), "%d", id);
	idStr[sizeof(idStr)-1] = 0;

	std::string submitTime;
	submitTime =  TRPlatform::YYYYMMDDHHMMSSUUUTimeStamp();
	
	PrintQueueRootPath = "c:\\AquariusNETFlimingData\\";

	std::string dataPath = PrintQueueRootPath +  "data" + "/" + iPrinterName + "/" + idStr + "_" + submitTime + "/";

	std::string paramPath = PrintQueueRootPath + "param" + "/" + iPrinterName + "/" + idStr + "_" + submitTime + "/";
	//deleting the data dir in case it already exists.
	TRPlatform::RemoveDirectory(dataPath.c_str());
	
	//copying data files for processing
	//***************************************************************************************************
	TRPlatform::MakeDirIfNeedTo(dataPath.c_str());

	std::string watchedDir = m_watchedDir;

	//finddata struct to search folders
	struct _finddata_t dicomFile;
	//find first file in the directoy
	long fileStatus = _findfirst( (watchedDir +"\\" + "*.dcm").c_str() ,&dicomFile);
	CopyFile((watchedDir + "\\" + dicomFile.name).c_str(),( dataPath + dicomFile.name).c_str(),0 );
	//find next file in the directory
	while(_findnext(fileStatus,&dicomFile) == 0)
	{
		CopyFile((watchedDir + "\\" + dicomFile.name).c_str(),(dataPath + dicomFile.name).c_str(),0 );
	}
	//***************************************************************************************************

	// Generating the job in a temp folder
	std::string tmpDir = "C:/tmp";
	printObj.Create(idStr, CJobControl::GetQueuePath("AQNetPrint", kQTmp).c_str(), tmpDir.c_str());

	// Setting printer name
	printObj.SetPrinterName(iPrinterName.c_str()); 

	// Setting the new source path after copying files
	printObj.SetSourcePath(dataPath.c_str()); 

	printObj.SetParamPath(paramPath.c_str()); 

	printObj.SetJobPath(1); //indicates creation by DICOM server

	printObj.SetCopyNum(1); //need to get this from DB 

	KVP_MAP infoKeys;

	infoKeys[kJOBKEYtargetProcessName]	= "AQNetPrint";

	std::string patientID = m_progressInfoMap["PatientID"];
	infoKeys[kJOBKEYpatientID] = patientID;

	std::string patientName = m_progressInfoMap["PatientsName"];
	infoKeys[kJOBKEYpatientName] = patientName;

	std::string accessionNumber = m_progressInfoMap["AccessionNumber"];
	infoKeys[kJOBKEYaccessionNumber] = accessionNumber;

	std::string destAE = iPrinterName;
	infoKeys[kJOBKEYdestAE] = destAE;

	std::string localAE = TRDICOMUtil::CalculateLocalAETitle();
	infoKeys[kJOBKEYsourceAE] = localAE;

	std::string seriesNumber = m_progressInfoMap["SeriesNumber"];
	infoKeys[kJOBKEYseriesNumber] = seriesNumber;

	JobInfoPublisher publisher(id);
	publisher.SetInfo(infoKeys);

	//need to change this
	printObj.SetSaveStatus(0); //need to get this from the web //we dont need this any more

	printObj.SetDispMode(iDisplayMode.c_str()); //need to get from DB
	
	printObj.SetSkipN(iskipN);

	const int cLocked = 1;
	const bool cCreateIfNotThere = true;

	// Saving the job in the temp folder
	printObj.Save(cLocked, cCreateIfNotThere);

	// Submitting the job into the request queqe
	printObj.Submit(CJobControl::GetRequestPath());

#endif
	return 0;
}


//-----------------------------------------------------------------------------------------------------
//
static MC_STATUS GetDataFromFile(char* CBFileName, void* CBuserInfo, int* CBdataSizePtr, void** CBdataBufferPtr, 
						         int CBisFirst, int* CBisLastPtr)
{
	HandleSeriesComplete *instance = (HandleSeriesComplete *)CBuserInfo;
	if (CBFileName == NULL)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandleSeriesComplete::GetDataFromFile() - null CBFileName for series %s from %s\n", instance->m_seriesInstanceUID, instance->m_connectInfo.RemoteHostName);
		GetAqLogger()->FlushLog();
		return MC_CALLBACK_CANNOT_COMPLY;
	}
	return (MC_STATUS)instance->HandleDataFromFile(CBdataSizePtr, CBdataBufferPtr, CBisFirst, CBisLastPtr);
}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::HandleDataFromFile(int* CBdataSizePtr, void** CBdataBufferPtr, 
						         int CBisFirst, int* CBisLastPtr)
{
	if (!CBisFirst)
		return MC_CALLBACK_CANNOT_COMPLY;
	
	int n;
	unsigned long fsize;
	FILE* CBfile;

	//	Make sure we can get the right filesize
	fsize = m_CBfileSize;
	fsize = (fsize % 2) ? fsize+1 : fsize;
	assert (fsize > 0);
	if (fsize <= 0)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandleSeriesComplete::GetDataFromFile() - filesize <= 0 for %s of series %s from %s\n", m_CBfileName, m_seriesInstanceUID, m_connectInfo.RemoteHostName);
		GetAqLogger()->FlushLog();
		return MC_CALLBACK_CANNOT_COMPLY;
	}
	
	//	Make sure we can get the right file buffer
	if(m_CBfileBuf) delete[] m_CBfileBuf, m_CBfileBuf=0;
	m_CBfileBuf = new char[fsize + 2];
		
	//	Open the file
	CBfile = fopen(m_CBfileName, "rb");
	
	//	Read the file in
	
	n = fread(m_CBfileBuf, fsize, 1, CBfile);
	
	//	Close the file
	fclose(CBfile);

	if (n != 1)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandleSeriesComplete::GetDataFromFile() - Could not read file %s of %s from %s\n", m_CBfileName, m_seriesInstanceUID, m_connectInfo.RemoteHostName);
		GetAqLogger()->FlushLog();
		return MC_CALLBACK_CANNOT_COMPLY;
	}
	
	//
	//	Assign callback "return" values
	//
	*CBisLastPtr = 1;
	*CBdataSizePtr = fsize;
	*CBdataBufferPtr = m_CBfileBuf;
	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------------------------------
//
void HandleSeriesComplete::ExtractCaScore()
{
	struct _stat statBuf;
	int fileID;
	int status, mcStatus;
	char outfileName[MAX_PATH];
	FILE* outfile;
	
	strncpy(m_CBfileName, m_watchedDir, MAX_PATH); //m_CBfileName should be made static
	STRNCAT_S(m_CBfileName, "/CaScore.dcm", MAX_PATH); //m_CBfileName should be made static

	strncpy(outfileName, m_watchedDir, MAX_PATH);
	STRNCAT_S(outfileName, "/CaScore.dat", MAX_PATH);

	outfile = fopen (outfileName, "w");

	//
	//	Open the file
	//
	mcStatus = MC_Create_Empty_File(&fileID, m_CBfileName);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandledCompletedSeries::ExtractCaScore() - DcmLib error: %d on opening %s, aborting extract from %s\n", mcStatus, outfileName, m_connectInfo.RemoteHostName); //m_connectInfo should be made static
		GetAqLogger()->FlushLog();
		fclose(outfile);
		return;
	}

	MessageFreeGuard guard(fileID, true);

	//
	//	Get the filesize, and map it for the callbacks
	//
	status = _stat(m_CBfileName, &statBuf);
	if (status < 0)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandledCompletedSeries::ConverToVox() - _stat failed on %s, aborting extract from %s\n", outfileName, m_connectInfo.RemoteHostName); //m_connectInfo should be made static
		GetAqLogger()->FlushLog();
		fclose(outfile);
		return;
	}
	m_CBfileSize = (unsigned long) statBuf.st_size; // m_CBfileSize should be made static

	//
	//	Read in the file.  Using separate function call to invoke
	//	GetTagOBOW callback instead of using RegisterCallback for
	//	thread safety.
	//
	mcStatus = MC_Open_File_Bypass_OBOW(m_connectInfo.ApplicationID, fileID, this, GetDataFromFile);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandleSeriesComplete::ConverToVox() - DcmLib error: %d on MC_Open_File_Bypass_OBOW on file %s, aborting extract from %s\n", mcStatus, m_CBfileName, m_connectInfo.RemoteHostName);
		GetAqLogger()->FlushLog();
		fclose(outfile);
		TRPlatform::remove(outfileName);
		return;
	}


	char* buffer = new char[statBuf.st_size];
	int size;
	
	mcStatus = MC_Get_Value_To_Buffer(fileID, TR_ATT_CA_SCORE, sizeof(buffer), buffer, &size);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage(  "ERROR: HandledCompletedSeries::ConverToVox() - DcmLib error: %d on MC_Get_Value_To_Function on file %s, aborting extract from %s\n", mcStatus, m_CBfileName, m_connectInfo.RemoteHostName); //needs to be made static
		GetAqLogger()->FlushLog();
		fclose(outfile);
		TRPlatform::remove(outfileName);
		return;
	}
	
	fwrite(buffer, size, 1, outfile);
	
	//
	//	Release unused resources
	//
	mcStatus = MC_Free_File(&fileID);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage(kWarning,  "WARNING: HandledCompletedSeries::ConverToVox() - MC_Free_Message failed during extract from %s\n", m_connectInfo.RemoteHostName); //needs to be made static
		GetAqLogger()->FlushLog();
	}
	fclose(outfile);
	TRPlatform::remove(m_CBfileName);
	if (buffer)
	{
		delete[] buffer, buffer = 0;
	}
}

const int cHeaderOnly = 1;
const int cKeepAsFile = 1;
//extern char *ToUpper(char *s);

//-------------------------------------------------------------------------------
static inline char *ToUpper(char *s)
{
	char *ret = s;

	for ( ; s && *s; s++)
		*s = toupper(*s);
	return ret;
}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::GetListOfApplicableTagFilterIDs(std::map<int, int>& oFilterIDMap, std::vector<std::string>& oRoutingLogInfo)
{
	char val[kVR_LT];
	PxDicomStatus dcmStatus = kNormalCompletion;

	//	Get one DICOM file so we can evaulate the rules against the DICOM tags in the file
	//		There are no rules that apply below the series level, so any DCM file will suffice.
	std::vector<TRFileName> fileList;

	//m_watchedDirectory -> m_watchedDir
	int status = TRPlatform::iGetDirectoryList(m_watchedDir, "*.dcm", fileList, 1);
	if (status < 0 || fileList.size() < 1)
	{
		GetAqLogger()->LogMessage(kInfo,"INFO: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() - found no DICOM files\n");
		return 0;
	}

	//	Try to load the DICOM header
	CPxDicomMessage msg;
	std::string fullPath = std::string(m_watchedDir) + "/" + std::string(fileList[0].GetName()); //m_watchedDirectory needs to be static
	dcmStatus = msg.Load(fullPath.c_str(), cHeaderOnly, cKeepAsFile);
	if (dcmStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() - failed to load DCM file: %s\n", fullPath.c_str());
		return -1;
	}

	{
		m_progressInfoMap.clear();	

		strcpy(val, "");
		msg.GetValue(kVLIPatientsName, val, sizeof val);
		m_progressInfoMap["PatientsName"] = val;

		strcpy(val, "");
		msg.GetValue(kVLIPatientId, val, sizeof val);
		m_progressInfoMap["PatientID"] = val;

		strcpy(val, "");
		msg.GetValue(kVLIStudyDate, val, sizeof val);
		m_progressInfoMap["StudyDate"] = val;

		strcpy(val, "");
		msg.GetValue(kVLIAccessionNumber, val, sizeof val);
		m_progressInfoMap["AccessionNumber"] = val;

		strcpy(val, "");
		msg.GetValue(kVLISeriesNumber, val, sizeof val);
		m_progressInfoMap["SeriesNumber"] = val;

		strcpy(val, "");
		msg.GetValue(kVLIModality, val, sizeof val);
		m_progressInfoMap["Modality"] = val;
	}

	std::string logInfo;
	if (strlen(m_routingLogFile) > 0)
	{
		strcpy(val, "");
		msg.GetValue(kVLIPatientsName, val, sizeof val);
		logInfo = "Patient name (0010,0010): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLIPatientId, val, sizeof val);
		logInfo = "Patient ID (0010,0020): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLIStudyDescription, val, sizeof val);
		logInfo = "Study Description (0008,1030): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLIInstitutionName, val, sizeof val);
		logInfo = "Institution (0008,0080): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLIPatientsBirthDate, val, sizeof val);
		logInfo = "Patient DOB (0010,0030): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLIPatientsSex, val, sizeof val);
		logInfo = "Patient Gender (0010,0040): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLIStudyId, val, sizeof val);
		logInfo = "Study ID (0020,0010): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);

		strcpy(val, "");
		msg.GetValue(kVLISeriesNumber, val, sizeof val);
		logInfo = "Series No (0020,0011): " + std::string(val);
		oRoutingLogInfo.push_back(logInfo);
	}

	if (m_queryRIS)
	{
		m_isDeltaReport = AuxData::IsDeltaImagingReport(msg.GetID());
	}

	oFilterIDMap.clear();
	std::map<int, int> tmpFilterMap;

	//	Get the list of Tag Filter Rules from the database
	std::vector<TagFilterRule> ruleV;
	status = m_db.GetTagFilterRules(ruleV);
	if (status != kOK || ruleV.size() <= 0) 
	{
		GetAqLogger()->LogMessage(kInfo,"INFO: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() - found no TagFilter Rules\n");
		
		return 0;
	}

	//	Check the rules against the actual DICOM values
	char valueFromFile[kVR_LT];
	int ruleIsTrue = 0;
	for(int i = 0; i < ruleV.size(); i++)
	{
		//	Trim white space from both ends
		iRTVDeSpaceDe(ruleV[i].m_value);

		int filterID			  = ruleV[i].m_filterID;
		unsigned long tagFromRule = ruleV[i].m_tag;
		int comparator            = ruleV[i].m_comparatorID;
		char* valueFromRule = ruleV[i].m_value;

		valueFromFile[0] = 0;
		dcmStatus = msg.GetValue(tagFromRule, valueFromFile, kVR_LT);
		if (dcmStatus != kNormalCompletion)
		{
			//	-- - 2005.11.21 - Fix #6166 - if the tag is not present, or is empty, make sure we treat
			//		it as empty.  Fall through and let the comparator test do the right thing.
			if (dcmStatus == kInvalidTag ||
				dcmStatus == kEmptyValue ||
				dcmStatus == kNullValue)
			{
				valueFromFile[0] = 0;
			}
			else
			{
				//	-- - 2005.11.21 - Fix #6166 - Can't tell if value is there or not - fail all cases
				GetAqLogger()->LogMessage("ERROR: Failed to get Tag %X file: %s, returned error: %d[%s]\n", tagFromRule, fullPath.c_str(), 
					dcmStatus, MC_Error_Message((MC_STATUS)dcmStatus));

				tmpFilterMap[filterID] = 0;
				continue;
			}
		}

		//	Trim white space from both ends
		iRTVDeSpaceDe(valueFromFile);

		ruleIsTrue = 0;
		switch(comparator)
		{
		case kTagIs:
			ruleIsTrue = !stricmp(valueFromFile, valueFromRule);
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) IS %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromFile, valueFromRule, ruleIsTrue != 0);
			
			break;
		case kTagIsNot:
			ruleIsTrue =  stricmp(valueFromFile, valueFromRule);
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) IS NOT %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromFile, valueFromRule, ruleIsTrue != 0);
			
			break;
		case kTagContains:
			ruleIsTrue = (strstr(ToUpper(valueFromFile), ToUpper(valueFromRule)) != NULL);
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) CONTAINS %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromFile, valueFromRule, ruleIsTrue != 0);
			
			break;
		case kTagDoesNotContain:
			ruleIsTrue = (strstr(ToUpper(valueFromFile), ToUpper(valueFromRule)) == NULL);
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) DOES NOT CONTAIN %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromFile, valueFromRule, ruleIsTrue != 0);
			
			break;
		default:
			GetAqLogger()->LogMessage("ERROR: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() - invalid comparator %d contained in rule:\n", comparator);
			GetAqLogger()->LogMessage("ERROR: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() -    [%d, %d, %d, %s]\n", 
				filterID, tagFromRule, comparator, valueFromRule);
			
			tmpFilterMap[filterID] = 0;
			continue;
		};


		//	RULE1 AND RULE2 AND ... RULEN.
		if (!ruleIsTrue)
		{
			//	if any rule evals to false, then the filter is false
			tmpFilterMap[filterID] = 0;
		}
		else
		{
			if (tmpFilterMap.find(filterID) == tmpFilterMap.end())
			{
				//	It's not there, so add it as true
				tmpFilterMap[filterID] = 1;
			}
		}
	}

	//	Compose final output map only from rules that passed.  Just don't add the failed ones
	//		That way, oFilterIDMap.size() can tell us how many rules passed.
	std::map<int, int>::iterator iter;
	for(iter = tmpFilterMap.begin(); iter != tmpFilterMap.end(); iter++)
	{
		int filterID = iter->first;
		int filterPassed = iter->second;
		if (filterPassed)
			oFilterIDMap[filterID] = filterPassed;
	}
	
	return oFilterIDMap.size();
}

enum
{
	kPFDays = 0,
	kPFWeeks,
	kPFMonths,
	kPFYears
};

//-----------------------------------------------------------------------------
//
int HandleSeriesComplete::CreateAndSubmitPrefetchJob(KVP_MAP& iPrefetchArgs, std::vector<KVP_MAP>& iRemoteAEs)
{
#if 0
	CPrefetchJob pj;
	const int cLocked = 1;
	const bool cCreateIfNotThere = true;
	int status = 0,i;
	char jobID[8];
	JOB_MAP foundJobs;
	JOB_MAP::iterator iter;
	std::vector<CJob*> jobV;
	//	Try to find any existing prefetch jobs for this PatientID.  If there is one, don't create a new one.
	int nResults = CJobControl::GetJobList("AQNetPrefetch", foundJobs, kmAllJobs, "patientID", m_progressInfoMap["PatientID"].c_str());
//	nResults+= CJobControl::GetRequestJobList("AQNetPrefetch", foundJobs, "patientID", m_progressInfoMap["PatientID"].c_str(), 9999);
	if (nResults > 0)
	{
		CJobControl::ReleaseJobMap(foundJobs);

		GetAqLogger()->LogMessage(kInfo,"INFO: New prefetch job not created.  Existing job found for patientID = %s, seriesUID = %s\n", m_progressInfoMap["PatientID"].c_str(), m_seriesInstanceUID);
			
		return 0;
#if 0 //Chetan March 14th
		bool foundJobFromDifferentSeries = false;
		for(iter = foundJobs.begin(); iter != foundJobs.end(); iter++)
		{
			jobV = iter->second;
			for(i = 0; i <jobV.size(); i++)
			{
				CPrefetchJob* pfJob = (CPrefetchJob*) jobV[i];
				if (strcmp(pfJob->GetOrigSeriesUID().c_str(), m_seriesInstanceUID) != 0)
				{
					foundJobFromDifferentSeries = true;
				}
			}
		}
		if (foundJobFromDifferentSeries)
		{
			GetAqLogger()->LogMessage(kInfo,"INFO: New prefetch job not created.  Existing job found for patientID = %s, seriesUID = %s\n", m_progressInfoMap["PatientID"].c_str(), m_seriesInstanceUID);
			
			return 0;
		}
#endif
	}

	CJobControl::ReleaseJobMap(foundJobs);

	//	Get a new JobID
	std::string PrefetchQueueRootPath = CJobControl::GetJobIDCreationPath("AqNETPrefetch");
	char newJobIDCmd[MAX_PATH];
	_snprintf(newJobIDCmd, sizeof(newJobIDCmd), "newjobid -QueueRootPath %s", PrefetchQueueRootPath.c_str());
	newJobIDCmd[sizeof(newJobIDCmd)-1] = 0;

	int id = system(newJobIDCmd);
	if (id <= 0)
	{
		GetAqLogger()->LogMessage("ERROR: Get Next JobID failed with return code %d", id);
		
		return -1;
	}
	_snprintf(jobID, sizeof(jobID), "%d", id);
	jobID[sizeof(jobID)-1] = 0;

	std::string tmpDir = "C:/tmp";
	status = pj.Create(jobID, CJobControl::GetQueuePath("AQNetPrefetch", kQTmp).c_str(), tmpDir.c_str());
	if (status != kJobSuccess)
	{
		GetAqLogger()->LogMessage("ERROR: Create job failed with status %d", status);
		
		return -1;
	}

	//	Compose list of remote servers
	const std::string COMMA = std::string(",");
	std::string serverList = "";
	KVP_MAP oneAE;
	for(i = 0; i < iRemoteAEs.size(); i++)
	{
		oneAE = iRemoteAEs[i];
		serverList += oneAE["AETitle"] + COMMA + oneAE["HostName"] + COMMA + oneAE["Port"] + COMMA;
	}

	//	Where's the data coming from?
	pj.SetSourceList(serverList.c_str());

	//	Where's it going?
	pj.SetDestAE(m_connectInfo.LocalApplicationTitle);

	//	Who's asking?
	pj.SetLocalAE(TRDICOMUtil::CalculateLocalAETitle().c_str());

	//	Set prefetch arguments
	pj.SetIncomingModality(m_progressInfoMap["Modality"].c_str());
	pj.SetModalitiesToKeep(iPrefetchArgs["Modality"].c_str());
	pj.SetKeepN(atoi(iPrefetchArgs["MaxNumberResults"].c_str()));

	int daysOld = atoi(iPrefetchArgs["StudyNotOlderThan"].c_str());
	int units = atoi(iPrefetchArgs["UnitType"].c_str());
	switch(units)
	{
	case kPFWeeks:
		daysOld *= 7;
		break;
	case kPFMonths:
		daysOld = (int) (float(daysOld) * 30.4167);
		break;
	case kPFYears:
		daysOld *= 365;
		break;
	default:
		break;
	};

	pj.SetDaysOld(daysOld);

	//	For progress reporting
	pj.SetPatientID(m_progressInfoMap["PatientID"].c_str());
	pj.SetPatientName(m_progressInfoMap["PatientsName"].c_str());
	pj.SetStudyDate(m_progressInfoMap["StudyDate"].c_str());
	pj.SetAccessionNumber(m_progressInfoMap["AccessionNumber"].c_str());
	pj.SetSeriesNumber(m_progressInfoMap["SeriesNumber"].c_str());
	pj.SetOrigSeriesUID(m_seriesInstanceUID);
	
	//	Submit Job
	pj.Save(cLocked, cCreateIfNotThere);
	status = pj.Submit(CJobControl::GetRequestPath());
	if (status != kJobSuccess)
	{
		GetAqLogger()->LogMessage("ERROR: Failed to submit job: return status = %d\n", status);
		
	}

#endif

	return 0;

}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::GetDuplicateReportInfos(std::vector<StudySeries>& oVal)
{	
	oVal.clear();  


	SQA sqa;
	sqa.SetCommandText(
		" SELECT AuxStudyUID, AuxSeriesUID, CONVERT(VARCHAR, PrivateData.Date, 21) FROM studyLevel "
		" JOIN SeriesLevel ON StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID "
		" JOIN PrivateData on SeriesInstanceUID = AuxSeriesUID "
		" WHERE BodyPartExamined = '0Delta_Report0' "
		" AND PrivateData.name = (SELECT top 1 name from PrivateData "
		" WHERE AuxSeriesUID = ?) "
		" AND StudyInstanceUID = (SELECT top 1 AuxStudyUID FROM PrivateData  "
		" WHERE AuxSeriesUID = ?) "
		" ORDER BY PrivateData.Date DESC ");

	sqa.AddParameter(m_seriesInstanceUID);
	sqa.AddParameter(m_seriesInstanceUID);
	
	int retcd = m_db.SQLExecuteBegin(sqa);
	if(retcd != kOK)  
		return retcd;

	int size = sqa.GetRecordCount(); 
	if (size < 2)
	{
		return kNoResult;
	}

	retcd = sqa.MoveFirst(); 
	if(retcd != kOK)  
		return retcd;

	size--;
	oVal.resize(size);
	StudySeries* pInfo;

	int index = 0;
	retcd = sqa.MoveNext();
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);

		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_date, sqa);
		retcd = sqa.MoveNext();
	}
	m_db.SQLExecuteEnd(sqa);	 

	return kOK;
}

AqObject dicomServerObjectHCS;
ApplicationEntity outboundLocalAEHCS;

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::MakeDicomServerObject(void)
{	
	int status;

	if (!dicomServerObjectHCS.GetID())
	{
		dicomServerObjectHCS.m_Type = kTypeDICOMServer;
		ASTRNCPY(dicomServerObjectHCS.m_EntityName, "AqNETDicomServer");
		ASTRNCPY(dicomServerObjectHCS.m_FullName, "AqNETDicomServer");
		ASTRNCPY(dicomServerObjectHCS.m_Description, "DICOM Server Process");
		ASTRNCPY(dicomServerObjectHCS.m_Hostname, TRPlatform::GetMyName());
		ASTRNCPY(dicomServerObjectHCS.m_Address, TRPlatform::GetIPAddressString());
		status = m_db.MakeAqObject(dicomServerObjectHCS);
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make DicomServerObject\n");
			
			return status;
		}
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::MakeOutboundAEObject(void)
{	
	int isLocal = 1, status, objectID = 0;
	
	if (outboundLocalAEHCS.GetID())
		return kOK;

	ASTRNCPY(outboundLocalAEHCS.m_AETitle, TRDICOMUtil::CalculateLocalAETitle().c_str());
	ASTRNCPY(outboundLocalAEHCS.m_hostName, TRPlatform::GetMyName());
	ASTRNCPY(outboundLocalAEHCS.m_IPAddress, TRPlatform::GetIPAddressString());
	ASTRNCPY(outboundLocalAEHCS.m_description, "outbound local AE title");
	outboundLocalAEHCS.m_IsLocalAE = 1;

	status = m_db.MakeAqObject(outboundLocalAEHCS);
	if (status != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: Failed to make AE Object for remote AE [%s, %s, %s]\n", outboundLocalAEHCS.m_AETitle, outboundLocalAEHCS.m_hostName, outboundLocalAEHCS.m_IPAddress);
		
		return status;
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::MakeRemoteAEObject(ApplicationEntity& iRemoteAE)
{	
	int notLocal = 0, status, objectID = 0;
	status = m_db.MakeAqObject(iRemoteAE);
	if (status != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: Failed to make AE Object for remote AE [%s, %s, %s]\n", iRemoteAE.m_AETitle, iRemoteAE.m_hostName, iRemoteAE.m_IPAddress);
		
		return status;
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::MakeLocalAEObject(void)
{	
	int isLocal = 1, status;
	if (!m_connectInfo.LocalAEAqObjectID) //needs to be made static
	{
		int localAEID;
		std::string strSQL = "SELECT ID from LocalAE where AETitle = '" + std::string(m_connectInfo.LocalApplicationTitle) + "'"; //needs to be static
		status = m_db.SQLGetInt(strSQL.c_str(), localAEID);
		if (status != kOK || localAEID <= 0)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make AE Object for local AE %s\n", m_connectInfo.LocalApplicationTitle); //needs to be made static
			
			return status;
		}

		status = m_db.MakeAEObject(localAEID, m_connectInfo.LocalAEAqObjectID, isLocal); //needs to be made static
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make AE Object for local AE %s\n", m_connectInfo.LocalApplicationTitle); //needs to be made static
			
			return status;
		}
	}

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
//	GRAMMAR: DICOMServer ReceivedSeries SeriesObject FROM:RemoteAEObject TO:LocalAEObject
//
int HandleSeriesComplete::AuditLogReceivedSeries(void)
{	
	if (!m_db.IsAuditTrailEnabled())
		return kOK;

	int status;

	//	ACTOR: Make DICOMServer object
	if (status = MakeDicomServerObject() != kOK)
	{
		return status;
	}

	//	ACTION_FROM: Make Remote AEObject if it's not already there
	ApplicationEntity remoteAE;
	ASTRNCPY(remoteAE.m_AETitle, m_connectInfo.RemoteApplicationTitle);
	ASTRNCPY(remoteAE.m_hostName, m_connectInfo.RemoteHostName);
	ASTRNCPY(remoteAE.m_IPAddress, m_connectInfo.RemoteIPAddress);
	remoteAE.m_IsLocalAE = 0;

	if (status = MakeRemoteAEObject(remoteAE) != kOK)
	{
		return status;
	}
	m_connectInfo.RemoteAEAqObjectID = remoteAE.GetID();

	//	ACTION_AT: Make Local AEObject if it's not already there
	if (status = MakeLocalAEObject() != kOK)
	{
		return status;
	}

	//	ACT_ON: Make SeriesObject
	if (!m_seriesLevelObjectID)
	{
		status = m_db.MakePatientObject(m_seriesInstanceUID, m_studyLevelObjectID, m_seriesLevelObjectID, m_connectInfo.RemoteAEAqObjectID );
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make PatientObject for seriesUID: %s\n", m_seriesInstanceUID);
			
			return status;
		}
	}

	//	Make SeriesEvenLog entry
	EventLog event;
	event.m_Actor		= dicomServerObjectHCS.GetID();
	event.m_Activity	= m_db.GetActionID("Receive");
	event.m_ActOn		= m_seriesLevelObjectID;
	event.m_Requestor	= 0;
	event.m_ActionFrom	= m_connectInfo.RemoteAEAqObjectID;
	event.m_ActionAt	= m_connectInfo.LocalAEAqObjectID;
	event.m_eventType	= kSeriesEventLog;

	m_db.LogEvent(event);

	return kOK;
}

//latest
//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::AuditLogSendSeries(ApplicationEntity& iRemoteAE)
{	
	if (!m_db.IsAuditTrailEnabled())
		return kOK;

	//	ACTOR: Make DICOMServer object
	int status = MakeDicomServerObject();
	if (status != kOK)
	{
		return status;
	}

	//	ACTION_FROM: Make Remote AEObject if it's not already there
	if (status = MakeRemoteAEObject(iRemoteAE) != kOK)
	{
		return status;
	}

	//	ACTION_AT: Make Outbound Local AEObject if it's not already there
	if (status = MakeOutboundAEObject() != kOK)
	{
		return status;
	}

	//	ACT_ON: Make SeriesObject
	if (!m_seriesLevelObjectID)
	{
		status = m_db.MakePatientObject(m_seriesInstanceUID, m_studyLevelObjectID, m_seriesLevelObjectID, m_connectInfo.RemoteAEAqObjectID );
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make seriesLevelObject for seriesUID: %s\n", m_seriesInstanceUID);
			
			return status;
		}
	}

	//	Make SeriesEvenLog entry
	EventLog event;
	event.m_Actor		= dicomServerObjectHCS.GetID();
	event.m_Activity	= m_db.GetActionID("Send");
	event.m_ActOn		= m_seriesLevelObjectID;
	event.m_Requestor	= 0;
	event.m_ActionFrom	= outboundLocalAEHCS.GetID();
	event.m_ActionAt	= iRemoteAE.GetID();
	event.m_eventType	= kSeriesEventLog;

	m_db.LogEvent(event);

	return kOK;
}


//latest
//------------------------------------------------------------------------------------------------------------------
//
//
void HandleSeriesComplete::ExecuteReportGrabber()
{

	std::string strSQL = "";

	//	Get the Accession Number
	std::string accessionNumber = "";
	std::string qStr = "'";

	// assign delta reports to new series, if has any study level report
	strSQL = " INSERT INTO PrivateDataReference (PrivateDataID, AuxRefStudyUID, AuxRefSeriesUID) ";
	strSQL += " SELECT PrivateData.ID, StudyInstanceUID, " + qStr + m_seriesInstanceUID + qStr; 
	strSQL += " FROM studyLevel JOIN SeriesLevel on StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID ";
	strSQL += " Join PrivateData on SeriesInstanceUID = AuxSeriesUID ";
	strSQL += " WHERE BodyPartExamined = '0Delta_Report0' ";
	strSQL += " AND StudyInstanceUID = (SELECT StudyInstanceUID FROM studyLevel ";
	strSQL += " JOIN SeriesLevel on StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID  ";
	strSQL += " WHERE SeriesInstanceUID = "+ qStr + std::string(m_seriesInstanceUID) + "')"; 
	m_db.SQLExecute(strSQL.c_str());
		
	// detect has any report assigned to new report, if not, go get new report
	strSQL = " SELECT top 1 PrivateDataID FROM PrivateDataReference ";
	strSQL += " WHERE AuxRefSeriesUID = "+ qStr + m_seriesInstanceUID + qStr;
	int dum = 0;
	int status = m_db.SQLGetInt(strSQL.c_str(), dum);
	if(dum < 1 || status != kOK)
	{
		strSQL = " SELECT AccessionNumber FROM StudyLevel JOIN SeriesLevel on StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID ";
	//	strSQL += " WHERE SeriesInstanceUID = '" + std::string(m_seriesInstanceUID) + "'";
				
		status = m_db.SQLGetString(strSQL.c_str(), accessionNumber);
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Could not get AccessionNumber (or grab reports) for seriesUID = %s\n", m_seriesInstanceUID);
			
		}
		else if (!accessionNumber.empty())
		{
			char cmdline[1024];
			sprintf(cmdline, "C:/Program Files/AQNet/utils/ReportGrabber.exe %s", accessionNumber.c_str());
			_flushall();
			STARTUPINFO si;
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi;
			ZeroMemory( &pi, sizeof(pi) );

			GetAqLogger()->LogMessage(kInfo,"INFO: HandledCompletedSeries::Process() - starting ReportGrabber with cmdLine: %s\n", cmdline);
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
		else
		{
			GetAqLogger()->LogMessage("DEBUG: ReportGrabber not called - No AccessionNumber for seriesUID = %s (maybe it's private data)\n", m_seriesInstanceUID);
			
		}
	}	

}

//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::MakeGroupObject(int groupID, UserGroup& oGroup)
{
	//	Make an AqObject for the group
	UserGroup group;
	int status = m_db.GetUserGroup(groupID, oGroup);
	if (status != kOK)
	{
		return -1;
	}

	status = m_db.MakeAqObject(oGroup);
	if (status != kOK || oGroup.GetID() <= 0)
	{
		return -1;
	}

	return kOK;
}

//Latest
//-----------------------------------------------------------------------------------------------------
//
int HandleSeriesComplete::AuditLogAssignSeries(const char* iSeriesUID, int iGroupID)
{

	if (!m_db.IsAuditTrailEnabled())
		return kOK;

	int status = kOK;

	//	ACTOR: Make DICOMServer object
	if (!dicomServerObjectHCS.GetID())
	{
		status = MakeDicomServerObject();
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make DICOMServer object for seriesUID: %s\n", m_seriesInstanceUID);
			
			return status;
		}
	}

	//	ACT_ON: Make SeriesObject
	if (!m_seriesLevelObjectID)
	{
		status = m_db.MakePatientObject(iSeriesUID, m_studyLevelObjectID, m_seriesLevelObjectID, m_connectInfo.RemoteAEAqObjectID );
		if (status != kOK)
		{
			GetAqLogger()->LogMessage("ERROR: Failed to make seriesLevelObject for seriesUID: %s\n", m_seriesInstanceUID);
			
			return status;
		}
	}

	//	ACTION_AT: Make UserGroup object
	UserGroup group;
	status = MakeGroupObject(iGroupID, group);
	if (status != kOK)
	{
		GetAqLogger()->LogMessage("ERROR: Failed to make UserGroupObject for groupID: %d, seriesUID: %s\n", iGroupID, m_seriesInstanceUID);
		
		return status;
	}

	//	Make SeriesEvenLog entry
	EventLog event;
	event.m_Actor		= dicomServerObjectHCS.GetID();
	event.m_Activity	= m_db.GetActionID("Assign");
	event.m_ActOn		= m_seriesLevelObjectID;
	event.m_Requestor	= 0;
	event.m_ActionFrom	= 0;
	event.m_ActionAt	= group.GetID();
	event.m_eventType	= kSeriesEventLog;

	m_db.LogEvent(event);

	return kOK;
}

//-----------------------------------------------------------------------------------------------------
//
BOOL 
HandleSeriesComplete::DetermineIfJobShouldBeCreated(std::vector<std::string> &iPEList, std::string iPEName)
{
	int size = iPEList.size();
	if ( size <=0 )
	{
		return true;
	}

	for (int i=0;i<size;i++)
	{
		int compare = stricmp((char*)iPEList[i].c_str(), (char*)iPEName.c_str());
		if (compare == 0)
		{
			return false;
		}
	}

	return true;

}

#if 0
//-----------------------------------------------------------------------------------------------------
//
void 
HandleSeriesComplete::PraseTagFiltersForDataProcessingPatterns(std::map<int, int> &iIDsOfFiltersThatPassed)
{
	std::map<int, int>::iterator iter;

	int filterID = 0, sequencing = 0;

	std::vector <std::string> peList;

	for(iter = iIDsOfFiltersThatPassed.begin(); iter != iIDsOfFiltersThatPassed.end(); iter++)
	{
		sequencing = 0;

		filterID = iter->first;

		std::vector<DataProcessingPattern> dataProcessingPattern;

		m_db.GetAPSPattern(filterID, dataProcessingPattern);

		for (int i=0;i<dataProcessingPattern.size();i++)
		{
			DataProcessingPattern pattern = dataProcessingPattern[i];

			if ( (pattern.m_processingJobList.size() < 0) || (pattern.m_processingJobList.size() > 1) )
			{
				continue;
			}
			else
			{	
				//////////////////////////////////////////////////////////////////////////////////////////////////////
				//do not create any study level PE jobs here
				//temp hack
				if (strcasecmp(pattern.m_processingJobList[0].m_processEngineName, "Registration.exe") == 0)
				{
					continue;
				}
				//temp hack
				//else if (strcasecmp(pattern.m_processingJobList[0].m_processEngineName, "ParametricMap.exe") == 0)
				//{
				//	continue;
				//}
				//temp hack
				else if (strcasecmp(pattern.m_processingJobList[0].m_processEngineName, "ParametricMap.exe") == 0)
				{
					continue;
				}	
				/*else
				{
					continue;
				}*/
					
				/////////////////////////////////////////////////////////////////////////////////////////////////////////


				/////////////////////////////////////////////////////////////////////////////////////////////////////////
				//check if the same PE (with no sequencing) appears in 2 jobs for the same series
				BOOL createJob = false;
				createJob = DetermineIfJobShouldBeCreated(peList, pattern.m_processingJobList[0].m_processEngineName);
				if (!createJob)
				{
					continue;
				}

				peList.push_back(pattern.m_processingJobList[0].m_processEngineName);
				///////////////////////////////////////////////////////////////////////////////////////////////////////////


				///////////////////////////////////////////////////////////////////////////////////////////////////////////
				//create a job for each sub-series
				std::string studyUID;
				GetStudyUID(studyUID);		
			
				std::string iSeriesUID(m_seriesInstanceUID);

				SOPINSTANCE_BY_GROUP sopInstanceMap;
				GetSOPInstanceListByGroup(sopInstanceMap);
				
				//create a job for each sub-volume in the series
				for (int index = 0;index < sopInstanceMap.size() ; index++)
				{
					SOPINSTANCE_LIST iSOPInstanceList = sopInstanceMap[index];

					CreateAndSubmitPEJob(std::string(pattern.m_processingJobList[0].m_processEngineName), studyUID, iSeriesUID, iSOPInstanceList);

					//CreateAndSubmitPEJob(pattern.m_processingJobList[0].m_processEngineName, studyUID, iSeriesUID);
				}
				///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			}
		}
	}

	return;
}
#endif


//-----------------------------------------------------------------------------------------------------
//
void 
HandleSeriesComplete::PraseTagFiltersForDataProcessingPatterns(std::map<int, int> &iIDsOfFiltersThatPassed)
{
 	std::map<int, int>::iterator iter;

	int filterID = 0, sequencing = 0;

	std::vector <std::string> peList;

	std::vector<int> filters;
	for(iter = iIDsOfFiltersThatPassed.begin(); iter != iIDsOfFiltersThatPassed.end(); iter++)
		filters.push_back(iter->first);

	std::vector<DataProcessJob> jobs;
	// get all jobs, but set id to zero
	int ret = m_db.GetJobByFilters(filters, jobs, true);
	if(ret != kOK)
		return;
	
	sequencing = 0;
	for (int i=0;i<jobs.size();i++)
	{
		DataProcessJob& job = jobs[i];

		for (int kk=0;kk<job.m_processors.size();kk++)
		{
			DataProcessor & processor = job.m_processors[kk];
			
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			//do not create any study level PE jobs here
			//temp hack
			if (strcasecmp(processor.m_handler, "Registration.exe") == 0)
			{
				continue;
			}
			//temp hack
			else if (strcasecmp(processor.m_handler, "ParametricMap.exe") == 0)
			{
				continue;
			}
			//temp hack
			else if (strcasecmp(processor.m_handler, "subtraction.exe") == 0)
			{
				continue;
			}
			//temp hack
			/*else
			{
				continue;
			}*/
			/////////////////////////////////////////////////////////////////////////////////////////////////////////


			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			//check if the same PE (with no sequencing) appears in 2 jobs for the same series
			BOOL createJob = false;
			createJob = DetermineIfJobShouldBeCreated(peList, processor.m_handler);
			if (!createJob)
			{
				continue;
			}

			peList.push_back(processor.m_handler);
			///////////////////////////////////////////////////////////////////////////////////////////////////////////


			///////////////////////////////////////////////////////////////////////////////////////////////////////////
			//create a job for each sub-series
			std::string studyUID;
			GetStudyUID(studyUID);		
		
			std::string iSeriesUID(m_seriesInstanceUID);

			SOPINSTANCE_BY_GROUP sopInstanceMap;
			GetSOPInstanceListByGroup(sopInstanceMap);
			
			//create a job for each sub-volume in the series
			for (int index = 0;index < sopInstanceMap.size() ; index++)
			{
				SOPINSTANCE_LIST iSOPInstanceList = sopInstanceMap[index];

				//not  a valid volume
				if (iSOPInstanceList.size() <= 20)
				{
					continue;
				}

				CreateAndSubmitPEJob(std::string(processor.m_handler), studyUID, iSeriesUID, iSOPInstanceList);

				//CreateAndSubmitPEJob(processor.m_handler, studyUID, iSeriesUID);
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		//peList.clear();
	}

	return;
}
