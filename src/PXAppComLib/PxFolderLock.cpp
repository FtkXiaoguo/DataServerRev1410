/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)



#include "PxFolderLock.h"
 


#include <memory>
///////////////////////////////////////////////
//#include "TRPlatform.h"
#	include <direct.h>
#include <sys/stat.h>
//#include "windows.h"

#include "afx.h"
//#include "AqCore/TRPlatform.h"

#include <time.h>

CPxFolderLocker::CPxFolderLocker() :
m_waitInterval(200)
,m_lockFlag(false)
, m_cancelFlag(0)
{

	 
}

CPxFolderLocker::CPxFolderLocker(const std::string &folderName, const std::string &lock_file ):
m_waitInterval(200)
, m_lockFlag(false)
, m_cancelFlag(0)
{
	
	 
	m_lockFileName = folderName + "\\" + lock_file;

 
}
void CPxFolderLocker::setup(const std::string &folderName, const std::string &lock_file )
{
	m_lockFileName = folderName + "\\" + lock_file;
}
CPxFolderLocker::~CPxFolderLocker()
{
	unlock();
}

bool file_exist(const char *fileName)
{
	struct stat statBuf;

	if (::stat(fileName, &statBuf) != 0)
		return false;

	return true;
}
bool CPxFolderLocker::lock(int try_nn  /*x 200Sec*/)
{
	try{
		int wait_nn = 0;
		 
		while (!lock_in((float)try_nn*m_waitInterval))
		{
			if (m_cancelFlag){
				//check the cancel flag;
				if (*m_cancelFlag){
					return false;
				}
			}
			::Sleep(m_waitInterval);
			if (wait_nn++ > try_nn){
				return false;
			}
		}
	}
	catch (...)
	{
	}
	return true;
}
#define LOCK_FILE_TIME_F ("%04d/%02d/%02d_%02d:%02d:%02d")
bool CPxFolderLocker::lock_in(float time_span /*mSec*/)
{
	time_span /= 1000.0f;//mSec -> Sec
	bool ret_b = false;
	if (file_exist(m_lockFileName.c_str())){

		bool fileIsExpired = false;
		CString read_line;
		CStdioFile file_read;
		if (TRUE != file_read.Open(m_lockFileName.c_str(), CFile::modeRead)){
			return false;
		}
		struct tm file_time;
		if (TRUE == file_read.ReadString(read_line)){
			if (read_line.GetLength() >= 19){
				const char *time_str = (LPCTSTR)read_line;
				sscanf_s(time_str,LOCK_FILE_TIME_F,
					&file_time.tm_year, &file_time.tm_mon, &file_time.tm_mday,
					&file_time.tm_hour, &file_time.tm_min, &file_time.tm_sec);

				CTime locked_time(file_time.tm_year, file_time.tm_mon, file_time.tm_mday, file_time.tm_hour, file_time.tm_min, file_time.tm_sec);
				//
				time_t theTime = time(NULL);
				struct tm aTime;
				localtime_s(&aTime,&theTime);
				CTime cur_time(theTime);
				CTimeSpan time_diff = cur_time - locked_time;
				float diff_time = time_diff.GetTotalHours()*60.0f*60.0f+ time_diff.GetTotalMinutes() * 16+time_diff.GetTotalSeconds();
				 
				fileIsExpired = diff_time > time_span;
			}
		}
		file_read.Close();
		if (fileIsExpired){
			::DeleteFile(m_lockFileName.c_str());
		}
		else{
			return false;
		}
		 
	}

	time_t theTime = time(NULL);
	struct tm aTime;
	localtime_s(&aTime, &theTime);


	int day = aTime.tm_mday;
	int month = aTime.tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept
	int year = aTime.tm_year + 1900; // Year is # years since 1900
	int hour = aTime.tm_hour;
	
	FILE *fp = 0;
	fopen_s(&fp, m_lockFileName.c_str(), "wt");
	if (fp == 0) return false;
	fprintf(fp, LOCK_FILE_TIME_F,
		aTime.tm_year + 1900, aTime.tm_mon + 1, aTime.tm_mday,
		aTime.tm_hour, aTime.tm_min, aTime.tm_sec);
	fclose(fp);
	
	m_lockFlag = true;
	return true;
}
void CPxFolderLocker::unlock(void)
{
	try {
		if (m_lockFlag){
			::DeleteFile(m_lockFileName.c_str());
		}
	}
	catch (...)
	{

	}
	m_lockFlag = false;
}