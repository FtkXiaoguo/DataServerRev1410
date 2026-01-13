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

#include "PxNetDB.h"
 
#include <string>
#include <codecvt>
#include <locale>
extern CPxDcmDB	g_db;
 
bool TestDBProcessAddStudy::doAddStudyRu()
{
	beginNewImage();
	strcpy(m_studyInfo->m_patientID, "1234555888");
 	strcpy(m_studyInfo->m_patientsName, "Каренина^Анна");
	strcpy(m_studyInfo->m_referringPhysiciansName, "Каренина^Алексея");
	strcpy(m_studyInfo->m_studyDescription, "Комментарий1");
	strcpy(m_studyInfo->m_seriesDescription, "Комментарий2");

	strcpy(m_studyInfo->m_characterSet, "ISO_IR 100\\ISO_IR 144");
	if (g_db.SaveDICOMData(*m_studyInfo) != kOK){
		gLogger.LogMessage("UpdateStudyInfo error \n");
		gLogger.FlushLog();
	}

	checkNext();

	return true;
}
 

bool TestDBProcessSearchStudy::doSearchStudyRu()
{
	int iGroupID = 1;
	int iUserID = 1;

	std::vector<pRTVSDicomInformation> oVal;
	pRTVSDicomInformation  iFilter;
	memset(&iFilter, 0, sizeof(pRTVSDicomInformation));
	//strcpy(iFilter.m_patientID, "123455*");
	strcpy(iFilter.m_patientName, "*Каренина*");
	 

	strcpy(iFilter.m_characterSet, "ISO_IR 100\\ISO_IR 144");
	int	retn = g_db.GetUserStudies(iGroupID, iUserID, oVal, iFilter);

#if 0
	std::vector<DICOMPatient>  oVal;
	DICOMData iFilter;
	strcpy(iFilter.m_patientID, "123455*");
	int	retn = g_db.GetPatientList(oVal, &iFilter);
	//	int	GetStudyList(std::vector<DICOMStudy>& oVal, DICOMData* iFilter, int TopN = 0, bool iSort = false);

#endif
	return true;
}
