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
 
bool TestDBProcessAddStudy::doAddStudyJp()
{
	beginNewImage();
	strcpy(m_studyInfo->m_patientID, "1234555888");
	strcpy(m_studyInfo->m_patientsName, "$B1S0l(B^$BBgBm(B");
	strcpy(m_studyInfo->m_referringPhysiciansName, "$B1S0l(B1^$BBgBm(B2");
	strcpy(m_studyInfo->m_studyDescription, "$B1S0l(B3");
	strcpy(m_studyInfo->m_seriesDescription, "^$BBgBm(B4");

	strcpy(m_studyInfo->m_characterSet, "ISO_IR 100\\ISO 2022 IR 87");
	if (g_db.SaveDICOMData(*m_studyInfo) != kOK){
		gLogger.LogMessage("UpdateStudyInfo error \n");
		gLogger.FlushLog();
	}

	checkNext();
	 

	return true;
}
 
#include "PxDbLib\src\PxDBCharacterLib.h"
extern JISCharacterLib *g_JisCharLib;
std::string _converte2JIS(const std::string &str)
{
	std::string strJIS = g_JisCharLib->convertSJIStoJIS(str);
 
	return strJIS;
};
std::string _converte2UTF8(const std::wstring &str)
{
	std::string strUTF8;
	int CharStrBuffSize = 400;
	int iCodePage = CP_UTF8;
	char *CharStrBuff = new char[CharStrBuffSize];
	int StrLen = WideCharToMultiByte(
		iCodePage, 0,
		str.c_str(), str.size(),
		CharStrBuff, CharStrBuffSize,
		NULL,
		NULL);
	CharStrBuff[StrLen] = 0;
	strUTF8 = CharStrBuff;
	delete[] CharStrBuff;

	return strUTF8;
};
bool TestDBProcessSearchStudy::doSearchStudyJp()
{
	int iGroupID = 1;
	int iUserID = 1;

	std::vector<pRTVSDicomInformation> oVal;
	pRTVSDicomInformation  iFilter;
	memset(&iFilter, 0, sizeof(pRTVSDicomInformation));
	//strcpy(iFilter.m_patientID, "123455*");
#if 0
	strcpy(iFilter.m_patientName, "*$B1S0l(B*");
	strcpy(iFilter.m_characterSet, "ISO_IR 100\\ISO 2022 IR 87");
#endif
#if 0
	strcpy(iFilter.m_patientName, "*田中*日本語*");
	strcpy(iFilter.m_studyDescription, "*インプラント*");
	strcpy(iFilter.m_characterSet, "SJIS");
#endif
#if 1
	std::string strTemp;
	strTemp = _converte2UTF8(L"田中*日本語*");
	strcpy(iFilter.m_patientName, strTemp.c_str());
	strTemp = _converte2UTF8(L"インプラント*");
	strcpy(iFilter.m_studyDescription, strTemp.c_str());
	strcpy(iFilter.m_characterSet, "ISO IR 192");
#endif
#if 0
	std::string strTemp;
	strTemp = _converte2JIS("*田中*日本語*");
	strcpy(iFilter.m_patientName, strTemp.c_str());
	strTemp = _converte2JIS("*インプラント*");
	strcpy(iFilter.m_studyDescription, strTemp.c_str());
	strcpy(iFilter.m_characterSet, "ISO 2022 IR 87");
#endif
	
	int	retn = g_db.GetUserStudies(iGroupID, iUserID, oVal, iFilter);
	retn = g_db.GetUserSeries(iGroupID, iUserID, oVal, iFilter);
	

	{
		std::vector<DICOMPatient>  PatientList;
		DICOMData iFilter;
		//	strcpy(iFilter.m_patientID, "123455*");
		strcpy(iFilter.m_patientsName, "*$BEDCf(B*");
		strcpy(iFilter.m_characterSet, "ISO_IR 100\\ISO 2022 IR 87");
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
