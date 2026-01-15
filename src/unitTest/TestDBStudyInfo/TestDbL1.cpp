/***********************************************************************
 
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#define _WIN32_DCOM

#include "TestDb.h"

#include "Globals.h"
#include "rtvPoolAccess.h"

#include "PxDBSqlite.h"
 
#include <string>
#include <codecvt>
#include <locale>
extern CPxDBSQLite	g_db;
 
bool TestDBProcessAddStudy::doAddStudyL1()
{
	beginNewImage();
	strcpy(m_studyInfo->m_patientID, "1234555888");
 	strcpy(m_studyInfo->m_patientsName, "2ndPatNameÜÏ^1stPatNameëÉtréma");
	strcpy(m_studyInfo->m_referringPhysiciansName, "2ndOpNameâîûêô-àèù^1stOpNameÞÞÈÛ");
	strcpy(m_studyInfo->m_studyDescription, "âîûêô1");
	strcpy(m_studyInfo->m_seriesDescription, "âîûêô2");

	strcpy(m_studyInfo->m_characterSet, "ISO 2022 IR 100");
	if (g_db.SaveDICOMData(*m_studyInfo) != kOK){
		gLogger.LogMessage("UpdateStudyInfo error \n");
		gLogger.FlushLog();
	}

	checkNext();

	return true;
}
 

bool TestDBProcessSearchStudy::doSearchStudyL1()
{
	int iGroupID = 1;
	int iUserID = 1;

	std::vector<pRTVSDicomInformation> oVal;
	pRTVSDicomInformation  iFilter;
	memset(&iFilter, 0, sizeof(pRTVSDicomInformation));
	//strcpy(iFilter.m_patientID, "123455*");
	strcpy(iFilter.m_patientName, "*NameëÉtréma*");
 

	strcpy(iFilter.m_characterSet, "ISO_IR 100");
	int	retn = g_db.GetUserStudies(iGroupID, iUserID, oVal, iFilter);

	retn = g_db.GetUserSeries(iGroupID, iUserID, oVal, iFilter);



	{
		std::vector<DICOMPatient>  PatientList;
		DICOMData iFilter;
		//	strcpy(iFilter.m_patientID, "123455*");
		strcpy(iFilter.m_patientsName, "*NameëÉtréma*");
		strcpy(iFilter.m_characterSet, "ISO_IR 100");
		int	retn = g_db.GetPatientList(PatientList, &iFilter);
		//

		std::vector<DICOMStudy>  StudyList;
		retn = g_db.GetStudyList(StudyList, &iFilter);

		std::vector<DICOMSeries>  SeriesList;
		retn = g_db.GetSeriesList(SeriesList, &iFilter);

		std::vector<DICOMInstance>  InstanceList;
		retn = g_db.GetInstanceList(InstanceList, &iFilter, SeriesList[0].m_seriesInstanceUID);

	}
	//	int	GetStudyList(std::vector<DICOMStudy>& oVal, DICOMData* iFilter, int TopN = 0, bool iSort = false);

	return true;
}
