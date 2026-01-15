/***********************************************************************
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */
//start from DBToolsWeb.cpp 1.113
#include "PxDBSqlite.h"

#include "AqCore/TRPlatform.h"
#include "AqCore/AqTString.h"
#include "CheckMemoryLeak.h"
 
#include "PxDBCharacterLib.h"
extern JISCharacterLib*  g_JisCharLib ;

using namespace std;

CPxDBSQLite::CPxDBSQLite()
{
}

CPxDBSQLite::~CPxDBSQLite()
{
}

int	CPxDBSQLite::SaveDICOMData(const DICOMData& dData, int iInstanceStatus)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SaveDICOMData start\n");

	SQA sqa(getLocalDBType());
	int retcd;

	 
	unsigned int iCodePage = CPxDB::getCodePageFromCharatorSet(dData.m_characterSet);
	if (iCodePage == _Def_MyCodePage_JIS) {
		//fixed 2023/03/15
		//write JIS to DB (not convertion)
		iCodePage = 1252;//Latin1  
	}
	// get series id first
	int seriesID = 0;
	sqa.FormatCommandText("SELECT SeriesLevelID FROM SeriesLevel WHERE SeriesInstanceUID='%s'",
		dData.m_seriesInstanceUID);
	sqa.setOptions(0);

	try {
		retcd = SQLExecuteBegin(sqa);
		if (retcd == kOK && (sqa.GetRecordCount() > 0) && sqa.MoveFirst() == kOK) {

			seriesID = sqa.getDataInt();
		}
	}
	catch (...)
	{

	}

	// make the series start from making study
	if (seriesID == 0)
	{
		//this a new series, add to history DB first
		//MakeHistoryPatientInfo(dData, false);

		int studyID = 0;
		// save the study data and get it's ID
		sqa.FormatCommandText("SELECT StudyInstanceUID FROM StudyLevel WHERE StudyInstanceUID='%s'",
			dData.m_studyInstanceUID);

		// try to get study ID first
		sqa.setOptions(0);
		retcd = SQLExecuteBegin(sqa);
		if (retcd == kOK && (sqa.GetRecordCount() > 0) && sqa.MoveFirst() == kOK)
		{
			studyID = sqa.getDataInt();
		}
		else
		{

			//#136 2021/01/12 N.Furutsuki unicode version
			AqUString UPpatientsName, UPhysiciansName, UStudyDescription;
			iCodePage = CP_ACP;
			UPpatientsName.Convert(dData.m_patientsName, iCodePage);
			UPhysiciansName.Convert(dData.m_referringPhysiciansName, iCodePage);
			UStudyDescription.Convert(dData.m_studyDescription, iCodePage);

			 
			sqa.FormatCommandText(L"INSERT INTO StudyLevel ( "
				L"StudyInstanceUID,"
				L"PatientsName,"
				L"PatientID,"
				L"PatientsBirthDate,"
				L"PatientsSex,"
				L"PatientsAge,"
				L"StudyDate,"
				L"StudyTime,"
				L"AccessionNumber,"
				L"StudyDescription"
			L") VALUES("
				L"'%s',"  //StudyInstanceUID
				L"'%s',"  //PatientsName
				L"'%s',"  //PatientID
				L"'%s',"  //PatientsBirthDate
				L"'%s',"  //PatientsSex
				L"'%d',"  //PatientsAge
				L"'%s',"  //StudyDate
				L"'%s',"  //StudyTime
				L"'%s',"  //AccessionNumber
				L"'%s' "  //StudyDescription
				L")",
				AqUString(dData.m_studyInstanceUID, CP_ACP), 
				UPpatientsName,
				AqUString(dData.m_patientID, CP_ACP),
				AqUString(dData.m_patientsBirthDate, CP_ACP),
				AqUString(dData.m_patientsSex, CP_ACP),
				dData.m_patientsAge,
				AqUString(dData.m_studyDate, CP_ACP),
				AqUString(dData.m_studyTime, CP_ACP),
				AqUString(dData.m_accessionNumber, CP_ACP),
				UStudyDescription
				);
			 
			 
		//	sqa.FormatCommandText(L"INSERT INTO StudyLevel ( StudyInstanceUID, PatientsName) VALUES ('111111.1111','tttttt111')");
#if 0
			sqa.SetCommandText(L"INSERT INTO StudyLevel( StudyInstanceUID,PatientName,PatientID) VALUES (’l1, ’l2, ..., ’lN)");
			sqa.AddParameter(dData.m_studyInstanceUID);
			//sqa.AddParameter(dData.m_patientsName);
			sqa.AddParameter(UPpatientsName);
			sqa.AddParameter(dData.m_patientID);
			sqa.AddParameter(dData.m_patientsBirthDate);
			sqa.AddParameter(dData.m_patientsSex);
			sqa.AddParameter(dData.m_patientsAge);
			sqa.AddParameter(dData.m_studyDate);
			sqa.AddParameter(dData.m_studyTime);
			sqa.AddParameter(dData.m_accessionNumber);
			sqa.AddParameter(dData.m_studyID);
			sqa.AddParameter(dData.m_radiologistName);
			//sqa.AddParameter(dData.m_referringPhysiciansName);
			sqa.AddParameter(UPhysiciansName);
			//sqa.AddParameter(dData.m_modalitiesInStudy);
			sqa.AddParameter(dData.m_modality); // use series modality because at this time only one series exists
			//sqa.AddParameter(dData.m_studyDescription);
			sqa.AddParameter(UStudyDescription);
			sqa.AddParameter(dData.m_numberOfStudyRelatedSeries);
			sqa.AddParameter(dData.m_numberOfStudyRelatedInstances);
			sqa.AddParameter(dData.m_characterSet);
#endif
			sqa.setOptions(kDBLockExecute | kDBNoLogOnIntegrityViolation | kDBExecuteNoRecords);

			retcd = SQLExecuteBegin(sqa);
			if (retcd != kOK && retcd != kDBException)
				return retcd;


			//SQLCommit(sqa); // for new study id
			//SQLNewTrans(sqa);

			//get id agine in case make failed because other thread did first
			sqa.FormatCommandText("SELECT StudyLevelID FROM StudyLevel WHERE StudyInstanceUID='%s'",
				dData.m_studyInstanceUID);

			// get study id again
			sqa.setOptions(0);
			retcd = SQLExecuteBegin(sqa);
			if (retcd != kOK)
				return retcd;

			retcd = sqa.MoveFirst();
			if (retcd != kOK)
				return retcd;
			if (sqa.GetRecordCount() > 0) {
				studyID = sqa.getDataInt();
			}

		}
		if(studyID == 0)
		{
			SQLExecuteEnd(sqa, false);
			return kDBException;
		}
		AqUString  USerisDescription;
		USerisDescription.Convert(dData.m_seriesDescription, iCodePage);

		// make series
		sqa.FormatCommandText(L"INSERT INTO SeriesLevel ( "
			L"StudyLevelID,		"
			L"SeriesInstanceUID, "
			L"SeriesNumber, 		"
			L"SeriesDescription, "
			L"Modality, 		"
			L"SeriesDate, 		"
			L"SeriesTime 		"
			L")VALUES("
			L"%d, "			//StudyLevelID
			L"'%s', "		//SeriesInstanceUID
			L"%d, "			//SeriesNumber
			L"'%s', "		//SeriesDescription
			L"'%s', "		//Modality
			L"'%s', "		//SeriesDate
			L"'%s' "		//SeriesTime
			L")",
			studyID,
			AqUString(dData.m_seriesInstanceUID, CP_ACP),
			dData.m_seriesNumber,
			USerisDescription,
			AqUString(dData.m_modality, CP_ACP),
			AqUString(dData.m_seriesDate, CP_ACP),
			AqUString(dData.m_seriesTime, CP_ACP)
		);
		#if 0
		//sqa.SetCommandText("EXEC MakeSeries ?, ?, ?, ?, ?, ?, ?, ?, ?,?,?");
		sqa.SetCommandText(L"EXEC MakeSeries ?, ?, ?, ?, ?, ?, ?, ?,?,?,?");

		sqa.AddParameter(studyID);
		sqa.AddParameter(dData.m_seriesInstanceUID);
		sqa.AddParameter(dData.m_seriesNumber);
		//sqa.AddParameter(dData.m_seriesDescription);
		sqa.AddParameter(USerisDescription);
		sqa.AddParameter(dData.m_modality);
		sqa.AddParameter(dData.m_bodyPartExamined);
		sqa.AddParameter(dData.m_viewPosition);
		//sqa.AddParameter(0);	//dData.m_numberOfSeriesRelatedInstances,
		sqa.AddParameter(dData.m_stationName);
		sqa.AddParameter(dData.m_seriesDate);
		sqa.AddParameter(dData.m_seriesTime);
		sqa.AddParameter(dData.m_manufacturer);
#endif
		sqa.setOptions(kDBLockExecute | kDBNoLogOnIntegrityViolation | kDBExecuteNoRecords);
		retcd = SQLExecuteBegin(sqa);
		if (retcd != kOK && retcd != kDBException)
			return retcd;


		//get id agine in case make failed because other thread did first
		sqa.FormatCommandText("SELECT SeriesLevelID FROM SeriesLevel WHERE SeriesInstanceUID='%s'",
			dData.m_seriesInstanceUID);

		sqa.setOptions(0);
		retcd = SQLExecuteBegin(sqa);
		if (retcd == kOK && (sqa.GetRecordCount() > 0) && sqa.MoveFirst() == kOK) 
			seriesID = sqa.getDataInt();
	}

	if (dData.m_SOPInstanceUID[0] == 0)
	{
		SQLExecuteEnd(sqa);
		return kOK;
	}

	//	sqa.SetCommandText("EXEC MakeInstance ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?");
	//#1 2012/02/10 K.Ko reduce the instanceLevel's field
	// changed to 11 field 
	sqa.SetCommandText("EXEC MakeInstance ?,?,?,?,?,?,?,?,?,?,?"); // 10 field

	sqa.AddParameter(dData.m_SOPInstanceUID);
	sqa.AddParameter(seriesID);
	sqa.AddParameter(dData.m_SOPClassUID);
	sqa.AddParameter(dData.m_transferSyntax);

	sqa.AddParameter(dData.m_instanceNumber);
	sqa.AddParameter(dData.m_rows);
	sqa.AddParameter(dData.m_columns);
	sqa.AddParameter(dData.m_numberOfFrames);
	sqa.AddParameter(dData.m_imageTypeTokens);
	//#1 2012/02/10 K.Ko reduce the instanceLevel's field
	/*
		sqa.AddParameter(dData.m_bitsAllocated);
		sqa.AddParameter(dData.m_bitsStored);
		sqa.AddParameter(dData.m_highBit);
		sqa.AddParameter(dData.m_pixelRepresentation);
		sqa.AddParameter(dData.m_photometricInterpretation);
		sqa.AddParameter(dData.m_planarConfiguration);

		sqa.AddParameter(dData.m_windowWidth);
		sqa.AddParameter(dData.m_windowCenter);
		sqa.AddParameter(dData.m_smallestPixelValue);
		sqa.AddParameter(dData.m_largestPixelValue);
		sqa.AddParameter(dData.m_samplesPerPixel);

		sqa.AddParameter(dData.m_pixelSpacing[0]);
		sqa.AddParameter(dData.m_pixelSpacing[1]);
		sqa.AddParameter(dData.m_aspectRatio);
		sqa.AddParameter(dData.m_rescaleSlope);
		sqa.AddParameter(dData.m_rescaleIntercept);

		sqa.AddParameter(dData.m_patientOrientation);
		sqa.AddParameter(dData.m_slicePosition);
		sqa.AddParameter(dData.m_sliceThickness);

		sqa.AddParameter(dData.m_imagePosition[0]);
		sqa.AddParameter(dData.m_imagePosition[1]);
		sqa.AddParameter(dData.m_imagePosition[2]);

		sqa.AddParameter(dData.m_imageOrientation[0]);
		sqa.AddParameter(dData.m_imageOrientation[1]);
		sqa.AddParameter(dData.m_imageOrientation[2]);
		sqa.AddParameter(dData.m_imageOrientation[3]);
		sqa.AddParameter(dData.m_imageOrientation[4]);
		sqa.AddParameter(dData.m_imageOrientation[5]);
	*/
	sqa.AddParameter(dData.m_pixelOffset);
	sqa.AddParameter(dData.m_dataSize);
	//#1 2012/02/10 K.Ko reduce the instanceLevel's field
	/*
		sqa.AddParameter(dData.m_referencedSOPInstanceUID);
		sqa.AddParameter(iInstanceStatus);

		sqa.AddParameter(dData.m_imageDate);
		sqa.AddParameter(dData.m_imageTime);
		sqa.AddParameter(dData.m_wasLossyCompressed);
		sqa.AddParameter(dData.m_scanOptions);

	*/
	sqa.setOptions(kDBLockExecute | kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	// try one more time in case make SOPClassID clashed
	if (retcd != kOK)
	{
		SQLExecuteEnd(sqa, false);
		GetAqLogger()->LogMessage(kInfo, "INFO: -CPxDB::SaveDICOMData try insert instance again when first attempt fail\n");
		retcd = SQLExecuteBegin(sqa);
	}
	if (retcd != kOK)
		return retcd;

	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SaveDICOMData end\n");
	return kOK;

}
