/*****************************************************************
 * DiskSpaceManager.cpp
 *
 *
 *	Copyright(c) PreXion  2001, All rights reserved.
 *
 *	PURPOSE:
 *     Manage disk spaces by removing old files. 
 *	   This object starts and runs in a seperate thread. 
 *	   The design is not 100% safe if margin is zero or very small.
 *     A minimum of 50Mbytes is needed.
 *
 *     Both synchronous and asynchronous modes are supported and used.
 *
 *	
 *
 *
 *****************************************************************/

#include "diskspacemanager.h"
#include "RTVRegistry.h"
#include "TICache.h"
#include "TRCompressedCacheWriter.h"
//#include "TerareconCacheWriter.h"

#include "AqCore/TRLogger.h"
#include "PxNetDB.h"

static const int sMinMargin = 250;	// absolute minimum diskspace reserved
static const int sHighMargin = 500; // minimum warning diskspace

int RTVDiskSpaceManager::m_sEarlyHighWatermarkEmail = 0;

//-----------------------------------------------------------------------------
iRTVSGetSpace::iRTVSGetSpace(void): iRTVThreadProcess("GetSpace")
{
	m_freedSpace = 0;
	m_MBytesToRemove = 0;
}


//-----------------------------------------------------------------------
void iRTVSGetSpace::Reset(void)
{
	// wait up to 5 seconds
	WaitForProcessToEnd();		
	if (IsRunning())
	{
		GetAqLogger()->LogMessage("DiskSpaceManager.GetSpace() hang\n");
		return ;
	}
	
	m_freedSpace = 0;
	m_processStatus = kToBeStarted;
}


//-----------------------------------------------------------------------
// more or less a one shot deal
#define  sTemplateStudyUID "2.16.840.1.114053.2100.9.2"
int iRTVSGetSpace::Process(void)
{
	tDSMFiles::iterator p;
	tDSMFiles &files = m_manager->m_files;

	int removed = 0;
	char buf[MAX_PATH];
	int status, MBytesToRemove = m_MBytesToRemove;
	
	// lock file list until the function returns
	TRCSLock L(&m_manager->m_cs);
	int day, hour;

	// -- 2005.11.14
	// close the window where we may have zero files to delete	
	long filenumber = files.size();
	int numDir = 1+MBytesToRemove/50;
	static long lastT = 0;
	long curT = time(0);
	if(filenumber < numDir && (curT-lastT) > 60)
	{
		lastT = curT;
		L.Unlock();
		m_manager->Update(3*numDir);
		L.Lock();
		if (filenumber == 0)
			GetAqLogger()->LogMessage(kWarning,"Warning: DiskSpaceManger on (%s) has no file to delete\n", 
					m_manager->m_directory);
	}

	long skipped = 0;
	for (p = files.begin(); removed < MBytesToRemove && !files.empty(); p = files.begin())
	{
		if (!strcmp(p->first.GetName(),".") || !strcmp(p->first.GetName(),".."))
		{
			files.erase(p);
			continue;
		}

		/* make exceptions for templates */
		if (strstr(p->first.GetName(),sTemplateStudyUID))
		{
			files.erase(p);
			continue;
		}
		
		if (m_manager->m_tail[0] != '\0')
		{
			snprintf(buf, sizeof buf,"%s/%s/%s", m_manager->m_directory, p->first.GetName(),m_manager->m_tail);	
		}
		else
		{
			snprintf(buf, sizeof buf,"%s/%s", m_manager->m_directory, p->first.GetName());
		}
		
		// we should not remove files that are less than 1 day old except it is
		// a pulled file
		day = p->first.GetAge(TRFile::kAccessTime, &hour);
		if (hour <= 2 && day < 1 && !strstr(p->first.GetName(),"pull"))
		{
			GetAqLogger()->LogMessage(kInfo, "Warning: skip dir(%s) that is accessed in last 2 hours\n", p->first.GetName());
			skipped++;
			files.erase(p);
			continue; //break;
		}
		if(day <= 3)
		{
			// send a warning email
#ifdef _DEBUG
			fprintf(stderr,"Warning: starting to clean up cache files less than three days!\n");
#endif
		}
			
		if (TRPlatform::IsDirectory(buf))
		{
			int available  = TRPlatform::GetAvailableDiskSpace(m_manager->m_directory);
			status = TRPlatform::RemoveDirectory(buf);
			int newspace = TRPlatform::GetAvailableDiskSpace(m_manager->m_directory) - available;
			removed += newspace;
			GetAqLogger()->LogMessage("DiskSpaceManager: %s removed. %d MBytes\n", buf, newspace);

		}
		else if ((status = TRPlatform::remove(buf)) == 0)
		{
			removed += p->first.GetSizeInKiloBytes()/1024; // some truncation, don't care
			GetAqLogger()->LogMessage("DiskSpaceManager: %s removed. %d kBytes\n", buf, p->first.GetSizeInKiloBytes());
		}
		
		if (status != 0)
		{
			GetAqLogger()->LogMessage(kWarning, "Warning: DiskSpaceManger(%d):failed to remove %s\n", status, buf);
		}
		
		files.erase(p);
	}

	if(filenumber == skipped)
	{

		GetAqLogger()->LogMessage(kWarning,"Warning: all files (%d) are accessed in last 2 hours\n", skipped);
	}

	m_freedSpace = removed;
	
	return 0;
}

/*---------------------------------------------------------------------
 * The diskSpaceManager
 *   Basically iRTVSGetSpace does the work.
 *--------------------------------------------------------------------*/

RTVDiskSpaceManager::RTVDiskSpaceManager(void) : iRTVThreadProcess("DiskSpaceManager")
{
	m_lowWaterMark = sMinMargin;
	m_highWaterMark = sHighMargin;
	
	//snprintf(m_tail,sizeof m_tail,"%s", "cache");
	m_tail[0] = '\0';
	m_directory[0] = 0;
	
	m_demandSpace.m_manager = this;
	for ( int i = 0; i < kMaxWorkerThreads; i++)
		m_getSpace[i].m_manager = this;
	m_lastAlert = 0;
	m_lastLowAlert = 0;
}


//-----------------------------------------------------------------------
RTVDiskSpaceManager::~RTVDiskSpaceManager(void)
{ 
	SetEnded(1); 
	m_semaphore.Post();
	
	for ( int i = 0; i < kMaxWorkerThreads;i++)
		m_getSpace[i].Reset();
}


//-----------------------------------------------------------------------
static inline int IsSlash(int c) { return c=='/' || c=='\\';}
void RTVDiskSpaceManager::SetTail(const char* iTail)
{
	if (!iTail || !*iTail)
	{
		m_tail[0] = '\0';
		return;
	}
	
	// get rid of leading slash if any
	snprintf(m_tail, sizeof m_tail, "%s", IsSlash(*iTail) ? iTail+1: iTail);
}

//-----------------------------------------------------------------------
int RTVDiskSpaceManager::Start(const char *iDir)
{
	if (iDir && *iDir)
	{
		snprintf(m_directory, sizeof m_directory, "%s", iDir);
	}
	else
	{
		GetAqLogger()->LogMessage("** DiskSpaceManager: bad directory requested\n");
		return kBadDir;
	}
	
	if (*m_directory)
	{
		if (GetProcessStatus() == kRunning)
		{
#ifdef _DEBUG
			printf("--**DiskSpaceManager already running\n");
#endif
		}
		else
		{
			m_theThread.Create(this);
		}
	}
	
	return GetProcessStatus() == kRunning ? kOK:kFailed;
}

//-----------------------------------------------------------------------
void RTVDiskSpaceManager::SetDirectory(const char* iDir)
{
	if (iDir)
	{
		snprintf(m_directory, sizeof m_directory, "%s", iDir);
		if (GetProcessStatus() == kRunning)
			GetAqLogger()->LogMessage("DiskSpaceManagerRunning: DIR=%s, HW/LW: %d/%d\n", 
			m_directory, m_highWaterMark, m_lowWaterMark);	
	}
}

//-----------------------------------------------------------------------
void RTVDiskSpaceManager::SetWaterMarks(int iHighMBytes, int iLowMBytes)
{
	// we can't allow less than 50Mbytes of margin
	if (iLowMBytes < sMinMargin)
		iLowMBytes = sMinMargin;
	
	if (iHighMBytes < sHighMargin)
		iHighMBytes = sHighMargin;

	//if(iHighMBytes < 2*iLowMBytes) // make sure high water mark is twice of low water mark
	//	iHighMBytes = 2*iLowMBytes;
	if(iHighMBytes <= iLowMBytes)
		iHighMBytes = iLowMBytes+1;

	m_lowWaterMark = iLowMBytes;
	m_highWaterMark = iHighMBytes;
}


//-----------------------------------------------------------------------
int RTVDiskSpaceManager::Update(int iNumberOfDirectories)
{
	// the monitoing is only work good on directory. It is not good to monitoting
	// on indivadual cache files because we could have thousands files that will take
	// too much memory to map. It is desired to keep the file-access information inside database
	TRCSLock lock(&m_cs);
	return TRPlatform::iGetDirectoryList(m_directory, m_files, iNumberOfDirectories);
}

//-----------------------------------------------------------------------
/* Wakeup every hour or so to get a list of files sorted according 
* to file access time
*/
int	RTVDiskSpaceManager::Process(void)
{
	int checkTimeInSec = 3600+1800; // one and half an hour
	int status, i;
	static int last;
	GetAqLogger()->LogMessage("DiskSpaceManagerRunning: DIR=%s, HW/LW: %d/%d\n", 
			m_directory, m_highWaterMark, m_lowWaterMark);

	Update();

#ifdef _DEBUG
	Print(m_files);
#endif
	
	do
	{
		status = m_semaphore.Wait(checkTimeInSec*1000);
		
		if (!m_ended)
		{
			if (status == TRSemaphore::kTimeOut)
			{
				Update(1600);  // 10MB/s. 50MB/study -> about 2hr's worth
			}
			else
			{
				// search for a free thread
				for (  i = 0; i < kMaxWorkerThreads; i++)
				{
					if (m_getSpace[i].GetProcessStatus() != iRTVThreadProcess::kRunning)
					{
						m_workerThread[i].Create(&m_getSpace[i]);
						last = i;
						break;
					}
				}
				
				// check if we have found a free one or not
				if ( i == kMaxWorkerThreads)
				{
					i = (last + 1 ) % kMaxWorkerThreads;
					m_getSpace[i].WaitForProcessToEnd(5000);
					m_getSpace[i].Reset();
					if (m_getSpace[i].GetProcessStatus() != iRTVThreadProcess::kRunning)
					{
						last = i;
						m_workerThread[i].Create(&m_getSpace[i]);
					}
				}
			}
			
		}
	} while (!m_ended);
	
	return 0;
}

//-----------------------------------------------------------------------
template <class T> static void sPrint(T& f)
{
	T::iterator p;
	for ( p = f .begin(); p != f.end(); ++p)
	{
		if (strcmp(p->first.GetName(),".") && strcmp(p->first.GetName(),".."))
		{
			unsigned t=p->first.GetAgeInSeconds(TRFile::kAccessTime)+p->first.GetAgeInSeconds()/4;
			fprintf(stderr,"%s\t Age=%g hrs\n", p->first.GetName(), t/3600.f);
		}
	}
}

//-----------------------------------------------------------------------
void RTVDiskSpaceManager::Print(tDSMFiles& files)  const { sPrint(files);}

//-----------------------------------------------------------------------
void SendWaterMarkEmail(const char* message)
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

//-----------------------------------------------------------------------
int RTVDiskSpaceManager::SpaceAvailable()
{
	int available;
	
	available = TRPlatform::GetAvailableDiskSpace(m_directory);
	return (available - m_highWaterMark);
}

//-----------------------------------------------------------------------
// demand space if block == 1
int	RTVDiskSpaceManager::MakeSpace(int iMBytes)
{
	int available, requested;
	available = SpaceAvailable();
	
	// have enough space
	if (available >= iMBytes)
		return kOK;
	
	// only delete the amount of we have to so we keep more files
	requested = iMBytes - available;

	// -- 2006.06.27
	// Change the highwater mark alert logic: send email only if 
	// we can't clean the disk. Reduce number of emails. For compatibility
	// reasons, we also add an option to go back to old behavior
	long curTime = time(0);

	if (m_sEarlyHighWatermarkEmail)
	{
		if( (curTime - m_lastAlert) > 3600*24)
		{
			m_lastAlert = curTime;
			char message[1024];
			sprintf (message, "Start to clean cache on %s", m_directory);
			SendWaterMarkEmail(message);
			GetAqLogger()->LogMessage(kWarning,"Warning: sent high water mark warning email\n");
		}
	}

	// want the space synchrounousely
	TRCSLock lock(&m_cs);

	m_demandSpace.Reset();
	m_demandSpace.SetRequestSize(requested);
	m_demandSpace.Process();
	available = TRPlatform::GetAvailableDiskSpace(m_directory);

	// -- 2006.06.27
	// We only need to send high-water mark when we truely run out of space,
	// that is, can't clean cache for the space to go below high watermark
	if (available >= (iMBytes + m_highWaterMark))
		return kOK;
	
	curTime = time(0);
	if( (curTime - m_lastAlert) >= 3600*24)
	{
		m_lastAlert = curTime;
		char message[1024];
		sprintf (message, "Start to clean cache on %s", m_directory);
		SendWaterMarkEmail(message);
		GetAqLogger()->LogMessage(kWarning,"Warning: sent high water mark warning email\n");
	}
	
	
	if(available >= (iMBytes+m_lowWaterMark))
		return kOK;
	
	if( (curTime - m_lastLowAlert) > 3600)
	{
		m_lastLowAlert = curTime;
		char message[1024];
		sprintf (message, "Low water mark alert on %s", m_directory);
		SendWaterMarkEmail(message);
		GetAqLogger()->LogMessage(kWarning,"Warning: sent low water mark warning email\n");
	}
	
	return -1;
}

//-----------------------------------------------------------------------
// use group space managers to manage space on each Archive Devices
// Next version will make one space manager to manage multip devices
SpaceManagerGroup RTVDiskSpaceManager::c_SpaceManagers;
bool RTVDiskSpaceManager::c_started = false;
std::vector <AppComDevice> RTVDiskSpaceManager::c_raidDevices;

int RTVDiskSpaceManager::StartupAll()
{
	if(c_started) return kOK;

	int rcode;

    // Set up a group space managers to manage space on each Archive Devices
	// Next version will make one space manager to manage multip devices
	if(c_raidDevices.size() == 0)
	{
		rcode = AppComConfiguration::GetArchiveDevices (AppComConfiguration::gkRAIDType, c_raidDevices);
		if(rcode != RTVRegistry::kSuccess || c_raidDevices.size() < 1)
			return kFailed;
	}
	
	RTVDiskSpaceManager* pspm;
	std::string cacheDir;
	int highWaterMark, lowWaterMark;
	const MediaPoint* mp;
    for (int i = 0; i < c_raidDevices.size(); i++)
    {
		pspm = new RTVDiskSpaceManager();
		cacheDir = c_raidDevices[i].GetPathToCacheOnDevice();

		/* --  2003-02-12
		 * needs existence of cachedir for everything to work
		 */
		if (!cacheDir.empty())
			TRPlatform::MakeDirIfNeedTo(cacheDir.c_str());
		c_SpaceManagers[cacheDir] = pspm;

		mp = CPxDB::GetMediaPoint(c_raidDevices[i].GetPathToDevice().c_str());
		if(mp)
		{
			highWaterMark = mp->m_highWaterMark;
			lowWaterMark = mp->m_lowWaterMark;
		}
		else
		{
			highWaterMark=10000; lowWaterMark=500;
		}
		
		pspm->SetWaterMarks(highWaterMark, lowWaterMark);
		pspm->Start(cacheDir.c_str());
    }
	c_started = true;
	return kOK;
}

//-----------------------------------------------------------------------
int RTVDiskSpaceManager::UpdateAll()
{
	if(!c_started) return kOK;

	int highWaterMark, lowWaterMark;
	const MediaPoint* mp;
	RTVDiskSpaceManager* pspm;
	SpaceManagerGroup::iterator iter;
	CPxDB::InitMediaPoints(true); // force to get fresh one, not from cache one
	for (iter=c_SpaceManagers.begin(); iter != c_SpaceManagers.end(); iter++)
	{
		pspm = iter->second;
		
		mp = CPxDB::GetMediaPoint(pspm->m_directory);
		if(mp)
		{
			highWaterMark = mp->m_highWaterMark;
			lowWaterMark = mp->m_lowWaterMark;
		}
		else
		{
			highWaterMark=10000; lowWaterMark=500;
		}

		pspm->SetWaterMarks(highWaterMark, lowWaterMark);
	}
	return kOK;
}

//-----------------------------------------------------------------------
void RTVDiskSpaceManager::ShutdownAll()
{
	if(!c_started) return;
	
	c_started = false;

	RTVDiskSpaceManager* pspm;
	SpaceManagerGroup::iterator iter;
	for (iter=c_SpaceManagers.begin(); iter != c_SpaceManagers.end(); iter++)
	{
		pspm = iter->second;
		delete pspm;
	}
	c_SpaceManagers.clear();
}

//-----------------------------------------------------------------------------
// Really need to complete this function
//
int RTVDiskSpaceManager::GetAvailableMedia(int reserveSpace, std::string& oOriginalRootDir, 
                                           std::string& oCacheRootDir)
{
    oOriginalRootDir.empty();
    oCacheRootDir.empty();

	if(!c_started) return kFailed;

	// looking for device has enough space
	RTVDiskSpaceManager* pspm =  NULL;
	SpaceManagerGroup::iterator iter;
	std::string managePath;
	for (iter=c_SpaceManagers.begin(); iter != c_SpaceManagers.end(); iter++)
	{
		pspm = iter->second;
		// TCZ&GL: 2004.08.27
		// kind of virtual highwater mark - we want to make sure
		// switch of disks happens before cache deletion.
		// in theory, a global accumulator keeping track of 
		// all requests and granted space should be used here
		const int extra = 3500; // 3.5GB
		if(pspm->SpaceAvailable() >= reserveSpace + extra )
		{
			managePath = iter->first;
			break;
		}
		else
		{
			pspm = NULL;
		}
	}

	// no device has has enough space, start making space
	if (pspm == NULL) 
	{
		for (iter=c_SpaceManagers.begin(); iter != c_SpaceManagers.end(); iter++)
		{
			pspm = iter->second;
			if(pspm->MakeSpace(reserveSpace) == kOK)
			{
				managePath = iter->first;
				break;
			}
			else
				pspm = NULL;
		}
	}

	if (pspm != NULL)
    {
        // Root Dir with the Trailing slash.
		std::vector<AppComDevice> aqnd;
		AppComConfiguration::GetArchiveDevices (AppComConfiguration::gkRAIDType, aqnd);
		std::string cacheDir;
		for (int i = 0; i < aqnd.size(); i++)
		{
			cacheDir = aqnd[i].GetPathToCacheOnDevice();
			if ( cacheDir.find(managePath) != std::string::npos)
			{
		        oOriginalRootDir = aqnd[i].GetPathToOriginalDICOMDataOnDevice ();
				oCacheRootDir    = managePath;
				return kOK;
			}
		}
        return kFailed;
    }
    else
        return kFailed;
}

//-----------------------------------------------------------------------
bool RTVDiskSpaceManager::MakeMediaSpace(int reserveSpace, std::string cacheDir)
{
	if(!c_started) return false;
	// looking for device has the cache directory
	SpaceManagerGroup::iterator iter;
	for (iter=c_SpaceManagers.begin(); iter != c_SpaceManagers.end(); iter++)
	{
		if(cacheDir.find(iter->first) != std::string::npos) 
		{
			return ((iter->second->MakeSpace(reserveSpace)) == kOK)?true:false;
		}
	}
	return false;
}

// Vikram 03/22/02 Change to support Multiple Drives
//-----------------------------------------------------------------------------
// This function takes in the StudyUID and SeriesUID and returns
// the path to where this series can be written to. 
// Function returns empty string if the disk is full
 
std::string RTVDiskSpaceManager::GetDirectoryToWriteCacheTo  (const std::string& iSeriesUID, 
                                             const std::string& iStudyUID)
{
	std::string origDir;
	std::string cacheDirectory;
	
	// check if there is built cache
	cacheDirectory = GetDirectoryToReadCacheFrom(iSeriesUID, iStudyUID);
	if(!cacheDirectory.empty())
	{
		// overwrite the cache if has space.
		if(MakeMediaSpace(GetDefaultCacheSize(), cacheDirectory))
			return cacheDirectory;
		else 
			return "";
			
	}

	GetAvailableMedia(GetDefaultCacheSize(),origDir, cacheDirectory);
	if (cacheDirectory=="")
		return cacheDirectory;
	cacheDirectory += iStudyUID;
	cacheDirectory += "/";
	cacheDirectory += iSeriesUID;
	cacheDirectory += "/";

	
	return cacheDirectory;
}

//-------------------------------------------------------------------------------
// -- 2005.05.04
// top-level cache directories - always return the disk that has the most space
std::string RTVDiskSpaceManager::GetCacheDirectory(void)
{
	std::string orig, cacheDir;
	GetAvailableMedia(GetDefaultCacheSize(),orig, cacheDir);
	return cacheDir;
}

//-------------------------------------------------------------------------------
// -- 2005.05.02
// Import directories - always return the disk that has the most space
std::string RTVDiskSpaceManager::GetImportDirectory(void)
{
	std::string orig, cacheDir;

	GetAvailableMedia(GetDefaultCacheSize(),orig, cacheDir);

	if (!cacheDir.empty())
		cacheDir += "AQNetImport/";

	return cacheDir;
}

//-----------------------------------------------------------------------
bool RTVDiskSpaceManager::FileExists(const std::string& iDir, const std::string& iPat)
{
	std::string fileSpec;
	WIN32_FIND_DATA FindFileData;
	HANDLE handle;
	bool yes;
	
	fileSpec = iDir + "/";
	fileSpec += iPat;
	
	handle = FindFirstFile(fileSpec.c_str(), &FindFileData);
	
	if ((yes = (handle != INVALID_HANDLE_VALUE)))
		FindClose(handle);
	return yes;
}


//-----------------------------------------------------------------------------
// This function takes in the StudyUID and SeriesUID
std::string RTVDiskSpaceManager::GetDirectoryToReadCacheFrom (const std::string& iSeriesUID, 
								const std::string& iStudyUID, const char* iPattern)
{
	std::string cacheDirectory;
	
	/*
	* -- 07/31/2002
	*
	* With multiple drives, if we left a directory on a low-disk space
	* drive that appears earlier in the m_raidDevices, ReadCache and 
	* WriteCache will be out of sync. The fix is to check for the requested
	 	 * pattern
	*/
	std::string firstDir;
	
	for (int i = 0; i < c_raidDevices.size(); i++)
	{
		cacheDirectory  = c_raidDevices[i].GetPathToCacheOnDevice();
		
		cacheDirectory += iStudyUID;
		cacheDirectory += "/";
		cacheDirectory += iSeriesUID;
		cacheDirectory += "/";
		
		if (TRPlatform::access(cacheDirectory.c_str(),0)==0)
		{
			if (firstDir.empty())
				firstDir = cacheDirectory;
			
			if (FileExists(cacheDirectory,iPattern))
				return cacheDirectory;
		}
		
	}
	
	return firstDir;
}

//-----------------------------------------------------------------------------
//	Added by -- - 12/18/02
int RTVDiskSpaceManager::DeleteCache(const std::string& iSeriesUID, const std::string& iStudyUID)
{
	int status = 0;
	std::string cacheDirectory;
	std::string studyDirectory;

	//	Delete cache folder for this series on all raid devices
	for (int i = 0; i < c_raidDevices.size(); i++)
	{
		cacheDirectory  = c_raidDevices[i].GetPathToCacheOnDevice();
		
		cacheDirectory += iStudyUID;
		studyDirectory = cacheDirectory;

		cacheDirectory += "/";
		cacheDirectory += iSeriesUID;
		cacheDirectory += "/";
		
		if ((_access(cacheDirectory.c_str(), 0)) == -1)
		{
			continue;
		}

		TRCompressedCacheWriter::Stop(cacheDirectory.c_str());
		status |= TRPlatform::RemoveDirectory(cacheDirectory.c_str());

		//	Also remove Study level directory if it's empty
		//	If there are more than 2 entries in the directory, it is not empty
		std::vector<TRFileName> fileList;
		TRPlatform::iGetDirectoryList(studyDirectory.c_str(), "*", fileList);
		if (fileList.size() != 2)
		{
			continue;
		}

		//	If the 2 entries are . and .., then it is ok to delete the directory
		std::vector<TRFileName>::iterator p;
		std::string name;
		bool deleteOK = true;
		for ( p = fileList.begin(); p < fileList.end(); p++)
		{
			if (!strcmp(p->GetName(), ".") || !strcmp(p->GetName(),".."))
				continue;
			
			deleteOK = false;
		}

		if (deleteOK)
		{
			status = TRPlatform::RemoveDirectory(studyDirectory.c_str());
		}
	}
	
	return status;
}


//-----------------------------------------------------------------------------
// This is the same as the above function but it is for the Original DICOM
// files. If this function returns a null ("") or zero length string it means
// either
// o. The Series is from an external DICOM server
// o. The disk and database are out of sync
std::string RTVDiskSpaceManager::GetDirectoryToReadOriginalFrom (const std::string& iSeriesUID, 
											const std::string& iStudyUID, const char* iPattern)
{   // Need to better look at this logic
	std::string cacheDirectory;
	
	for (int i = 0; i < c_raidDevices.size(); i++)
	{
		cacheDirectory  = c_raidDevices[i].GetPathToOriginalDICOMDataOnDevice();
        
		cacheDirectory += iStudyUID;
		cacheDirectory += "/";

		// t.c.zhao 2005.05.16
		// support null seriesUID
		if (!iSeriesUID.empty())
		{
			cacheDirectory += iSeriesUID;
			cacheDirectory += "/";
		}
		
		if (TRPlatform::access(cacheDirectory.c_str(),0)==0)
		{
			if (iPattern)
			{
				 if(FileExists(cacheDirectory,iPattern))
					return cacheDirectory;
			}
			else
				return cacheDirectory;
		}
		
	}
	
	return "";
}
