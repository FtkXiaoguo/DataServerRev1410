/***********************************************************************
 * CFind.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes CFind Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_FIND_H
#define C_FIND_H

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
#else
#include "rtvMergeToolKit.h"
#endif

#include "rtvthread.h"
#include "RTVDiCOMService.h"
#include "PxNetDB.h"

class CFind: public RTVDiCOMService, public iRTVThreadFunction
{
	enum
	{
		kPatientLevel = 0,
		kStudyLevel,
		kSeriesLevel,
		kInstanceLevel
	};

public:
	CFind(DiCOMConnectionInfo& connectInfo, int iMessageID);
	~CFind();

	int Process();
	void LogProcessStatus(void) {};

protected:
	int	ThreadFunction(void* data);
	void BuildResponseMessage(int i, int iMsgID, long iMask);
	int HandleError(MC_STATUS iErrorCode, RESP_STATUS iResponseCode, const char* iErrorMsg, int iDebugLevel = 0);
	void Print(int i, int iMsgID, long iMask);	// For debugging

private:
	CPxDcmDB		m_db;
	SQA			m_sqa;
	DICOMData	m_filter;
	bool		m_cancel;
	int			m_status;
	int			m_queryLevel;
	int			m_numberOfMatches;
	std::vector<DICOMPatient>  m_patientData;
	std::vector<DICOMStudy>	   m_studyData;
	std::vector<DICOMSeries>   m_seriesData;
	std::vector<DICOMInstance> m_imageData;

	int m_rspID;
};


#endif // C_FIND_H
