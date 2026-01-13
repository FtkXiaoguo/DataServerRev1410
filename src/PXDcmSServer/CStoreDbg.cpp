/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

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
#include "AuxData.h"
#include "AppComCacheWriter.h"
#include "Conversion.h"


#include "rtvsutil.h"

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

int CStore::dbg_dumyProc(int level)  //2012/03/07 for test
{
	int	status;

	/*
	*  at first :  ‚±‚ê‚Í•K—v
	*/
	status = MC_Get_Message_Service(m_messageID, &m_serviceName, &m_command);
	if (status != MC_NORMAL_COMPLETION)
	{
		 printf("**** MC_Get_Message_Service   \n");
		 return -1;
	}

	sprintf(m_fileName, "dbg_testdicom_%d.dcm",::GetCurrentThreadId());

	m_state  = kLeaveProcess;
	switch(level){
	case 0:
		{
		// do nothing
		//
		/*
		*  
		* RTVDiCOMStore::theProcessHeader() not yet
		*   
		*
		*/
		
		SuccessResponce(false/*error*/);
		MC_Free_Message (&m_messageID);

		}
		break;
	case 1:
		{
		/*
		*  use CPxDicomImage only
		*/
			if(m_pImage) delete m_pImage, m_pImage=0;

			m_pImage = new CPxDicomImage(m_messageID); //from here the messageID should be freed by m_pImage

			SuccessResponce(false/*error*/);
		}
		break;
	case 2:
		{
		/*
		*  use CPxDicomImage and write file only
		*/
			if(m_pImage) delete m_pImage, m_pImage=0;
			m_pImage = new CPxDicomImage(m_messageID); //from here the messageID should be freed by m_pImage
			 
			//
			//	Write the image to a file
			//
			//Store_Pool.Add(m_messageID, this);
			int	status = m_pImage->ConvertToFile(m_fileName, m_connectInfo.LocalApplicationTitle);
			if (status != MC_NORMAL_COMPLETION)
			{
			 printf("**** ConvertToFile %s error \n",m_fileName);
			}
			SuccessResponce(false/*error*/);
		}
		break;

		case 3:
		{
		/*
		*  use CPxDicomImage and write file - save DB
		*/
		
			if(m_pImage) delete m_pImage, m_pImage=0;
			m_pImage = new CPxDicomImage(m_messageID); //from here the messageID should be freed by m_pImage
		 	const char* studyUID = m_pImage->GetStudyInstanceUID();
	
			const char* instanceUID = m_pImage->GetSOPInstanceUID();

			//
			//	Write the image to a file
			//
			//Store_Pool.Add(m_messageID, this);
			int	status = m_pImage->ConvertToFile(m_fileName, m_connectInfo.LocalApplicationTitle);
			if (status != MC_NORMAL_COMPLETION)
			{
			 printf("**** ConvertToFile %s error \n",m_fileName);
			}
		 
			try
			{
			 
				m_pImage->FillSortInfo(m_dbData);
				int dbstat = m_db.SaveDICOMData(m_dbData);
		 
				if(dbstat != kOK){
					printf("**** SaveDICOMData   error \n" );
				}
			}
			catch (...) 
			{
				LogMessage(kWarning,"WARNING: Overrwite studyInfo threw exception\n");
			}

			SuccessResponce(false/*error*/);
		}

	default:
		break;
	}

	

	return 0;

}


int CStore::dbg_stepByStepProc(int level)  //2012/03/07 for test
{
	int	 preProcessStatus;
	m_state  = kLeaveProcess;

	switch(level){
	case 0:
	{
		preProcessStatus =  PreProcessNoneResponce() ;
		m_state  = kLeaveProcess;
		SuccessResponce(false/*error*/);
	};
	break;
	case 1:
	{
		//		preProcessStatus =  PreProcessNoneResponce() ;
		//‘ã‚í‚è‚É
		if(m_pImage) delete m_pImage, m_pImage=0;
		m_pImage = new CPxDicomImage(m_messageID); //from here the messageID should be freed by m_pImage
			
		const char* studyUID = m_pImage->GetStudyInstanceUID();
		ASTRNCPY(m_seriesUID, m_pImage->GetSeriesInstanceUID());
		const char* instanceUID = m_pImage->GetSOPInstanceUID();

		int status = MC_Get_Message_Service(m_messageID, &m_serviceName, &m_command);
		if (status != MC_NORMAL_COMPLETION)
		{
			 printf("**** MC_Get_Message_Service   \n");
			 return -1;
		}

		m_state  = kLeaveProcess;
		m_startThread = true; //igore the error

		///
	//	ASTRNCPY(m_seriesDir,"E:\\temp\\dbg_dicm_dir");
		sprintf(m_seriesDir,"E:\\temp\\dbg_dicm_dir\\%s\\%s",studyUID,m_seriesUID);
		TRPlatform::MakeDirIfNeedTo(m_seriesDir);
 		preProcessStatus =  Process() ;
		SuccessResponce(false/*error*/);
	};
	break;
	case 2:
	{
		preProcessStatus =  PreProcessNoneResponce() ;

		m_state  = kLeaveProcess;
		m_startThread = true; //igore the error
 		preProcessStatus =  Process() ;
		SuccessResponce(false/*error*/);
	};
	break;
	default:
	break;
	}


	return preProcessStatus;
}