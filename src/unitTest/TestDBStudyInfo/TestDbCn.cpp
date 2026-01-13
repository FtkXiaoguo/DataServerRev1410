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
 
bool TestDBProcessAddStudy::doAddStudyCn()
{
	
	beginNewImage();
	strcpy(m_studyInfo->m_patientID, "1234555888");
 	strcpy(m_studyInfo->m_patientsName, "赵^子龙");
	strcpy(m_studyInfo->m_referringPhysiciansName, "武^则天");
//	wcscpy((wchar_t*)m_studyInfo->m_patientsNameWStr, L"赵^之龙");
//	wcscpy((wchar_t*)m_studyInfo->m_patientsNameWStr, L"日本語^渓流");
//	m_pImage->FillPatientInfo(studyInfo);
	strcpy(m_studyInfo->m_studyDescription, "检查描述");
	strcpy(m_studyInfo->m_seriesDescription, "序列描述");

	strcpy(m_studyInfo->m_characterSet, "ISO_IR 100\\GB18030");
	if (g_db.SaveDICOMData(*m_studyInfo) != kOK){
		gLogger.LogMessage("UpdateStudyInfo error \n");
		gLogger.FlushLog();
	}

	checkNext();

	return true;
}
 
bool TestDBProcessSearchStudy::doSearchStudyCn()
{
	int iGroupID = 1;
	int iUserID = 1;

	std::vector<pRTVSDicomInformation> oVal;
	pRTVSDicomInformation  iFilter;
	memset(&iFilter, 0, sizeof(pRTVSDicomInformation));
	//strcpy(iFilter.m_patientID, "123455*");
	   strcpy(iFilter.m_patientName, "*赵*");
	//strcpy(iFilter.m_patientName, "*test*");
	//strcpy(iFilter.m_patientName, "*日本語*");

	strcpy(iFilter.m_characterSet, "ISO_IR 100\\GB18030");
	int	retn = g_db.GetUserStudies(iGroupID, iUserID, oVal, iFilter);
	 

	{
		std::vector<DICOMPatient>  PatientList;
		DICOMData iFilter;
		//	strcpy(iFilter.m_patientID, "123455*");
		strcpy(iFilter.m_patientsName, "*赵*");
		strcpy(iFilter.m_characterSet, "ISO_IR 100\\GB18030");
		int	retn = g_db.GetPatientList(PatientList, &iFilter);
		//

		std::vector<DICOMStudy>  StudyList;
		retn = g_db.GetStudyList(StudyList, &iFilter);
		//
		retn = g_db.GetStudyListEx(StudyList, &iFilter, 10/*TopN=*/, 1/*iSortStudyDate*/);

		std::vector<DICOMSeries>  SeriesList;
		retn = g_db.GetSeriesList(SeriesList, &iFilter);

		std::vector<DICOMInstance>  InstanceList;
		retn = g_db.GetInstanceList(InstanceList, &iFilter, SeriesList[0].m_seriesInstanceUID);

}
	return true;
}
 