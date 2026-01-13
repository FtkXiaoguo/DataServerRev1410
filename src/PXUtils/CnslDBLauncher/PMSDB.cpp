#include "StdAfx.h"
#include "PMSDB.h"

#include "PxDB.h"
#include "AqCore/TRLogger.h"
extern TRLogger g_pxLogger_;

#include "PMSIF1_Lib/PMSIF.h"

#define PMSDB_FIELD_NAMES "PATIENTID,NAME0,NAME1,NAME2,NAME3,SEX,BIRTHDATE,COMMENTS"

CPMSDB::CPMSDB(void)
{
	m_retryNN = 5; 
}

CPMSDB::~CPMSDB(void)
{
}
static bool insert2DB(const PMS_patientInfo &infor)
{	
	CPxDB pxDb;

	AqString	strSQL_Exist;

	strSQL_Exist.Format("SELECT * FROM TBL_PATIENT WHERE "
						"PATIENTID='%s' AND NAME0='%s' AND NAME1='%s' AND NAME2='%s' AND NAME3='%s' AND SEX='%s' AND BIRTHDATE=%d",
						infor.cPatientID,
						infor.cPatientName1,
						infor.cPatientName2,
						infor.cPatientNameKana1,
						infor.cPatientNameKana2,
						infor.cSex,
						strlen(infor.cBirthDate)>0 ? atoi(infor.cBirthDate) : 0,
	 					infor.cDesc);


	AqString	strSQL;
	strSQL.Format("IF NOT EXISTS (%s)"
				"INSERT TBL_PATIENT ("
				PMSDB_FIELD_NAMES
				") VALUES ( "
				"'%s',"		// -- PATIENTID
				"'%s',"		// -- NAME0
				"'%s',"		// -- NAME1
				"'%s',"		// -- NAME2
				"'%s',"		// -- NAME3
				"'%s',"		// -- SEX
				"%d,"		// -- BIRTHDATE
				"'%s'"		// -- REC0_DESP
				")",
				strSQL_Exist,
				infor.cPatientID,
				infor.cPatientName1,
				infor.cPatientName2,
				infor.cPatientNameKana1,
				infor.cPatientNameKana2,
				infor.cSex,
				strlen(infor.cBirthDate)>0 ? atoi(infor.cBirthDate) : 0,
	 			infor.cDesc
				
				);

	SQA sqa ;
//	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	 
	bool ret_b = true;
	try{
		int retcd = pxDb.SQLExecuteBegin(sqa);
		if(retcd != kOK){
			throw(-1);
		} 
	 
	   
	}catch(int){
		ret_b = false;
	}catch(...){
		ret_b = false;
	}

	pxDb.SQLExecuteEnd(sqa);
	 
	return ret_b;
 
}

static bool readDB(const std::string &PatientID, PMS_patientInfo &infor)
{
	
	CPxDB pxDb;

	AqString	strSQL;
	strSQL.Format("SELECT "
				PMSDB_FIELD_NAMES
				" FROM TBL_PATIENT WHERE PATIENTID='%s'",
				PatientID.c_str());

	SQA sqa ;
//	sqa.setOptions(kDBAsyncExecute);
	sqa.SetCommandText(strSQL);
	 
	bool ret_b = true;
	try{
		int retcd = pxDb.SQLExecuteBegin(sqa);
		if(retcd != kOK){
			throw(-1);
		} 
		int size = sqa.GetRecordCount(); 
		if(size < 1) {
			throw(1);
		} 

		retcd = sqa.MoveFirst(); 
		if(retcd != kOK){
			throw(-1);
		} 
	
	  
		int date_temp;
		{
			SQL_GET_STR(infor.cPatientID,		sqa);
			SQL_GET_STR(infor.cPatientName1,	sqa);
			SQL_GET_STR(infor.cPatientName2,	sqa);
			SQL_GET_STR(infor.cPatientNameKana1,sqa);
			SQL_GET_STR(infor.cPatientNameKana2,sqa);
			SQL_GET_STR(infor.cSex,				sqa);
			SQL_GET_INT(date_temp,				sqa);
			sprintf(infor.cBirthDate,"%d",date_temp);
			SQL_GET_STR(infor.cDesc,			sqa);
		}
	}catch(int errorNO){
		if(errorNO<0){
			ret_b = false;
		}
	}catch(...){
		ret_b = false;
	}

	pxDb.SQLExecuteEnd(sqa);
	 
	return ret_b;
}
bool CPMSDB::importDB(const std::string &PatientID,EXE_CODE &error_code, std::string &msg)
{

	msg = PatientID;
	error_code = ExeCode_No_Error;
	if(!initDB()){
		error_code = ExeCode_InitDB_Error;
		return false;
	}
	 


	PMS_patientInfo patientInfo;
	memset(&patientInfo,0,sizeof(PMS_patientInfo));
	if(1){
		
		int nRet = PMS_getPatientInfo( PatientID.c_str(), patientInfo );
		if ( nRet !=PMS_ERR_CODE_OK ) {
			reportReadCSVFileError(nRet,PatientID);
		//	g_pxLogger_.LogMessage(kErrorOnly,"ERROR: CPMSDB::importDB PMS_getPatientInfo [%s]  failed\n",PatientID.c_str());
		//	g_pxLogger_.FlushLog();
			error_code = ExeCode_GetPatientInfo_Error;
			msg = getLastError();
			return false;
		}else{
			sprintf_s(patientInfo.cDesc,256,"%s","　");//既往歴に全角スペースを入れておく
			g_pxLogger_.LogMessage(kTrace,"CPMSDB::importDB read from deltaview information \n");
			g_pxLogger_.LogMessage(kTrace,"\n    ID: %s \n     Name: %s, %s\n     Kana: %s, %s\n     Sex: %s, Desc: %s\n ",
				patientInfo.cPatientID,
				patientInfo.cPatientName1,patientInfo.cPatientName2,
				patientInfo.cPatientNameKana1,patientInfo.cPatientNameKana2,
				patientInfo.cSex,patientInfo.cDesc
				);
			g_pxLogger_.FlushLog();
		}
		
	}
//	msg = patientInfo.cPatientID;

	{//check Patient Info
		if(	(strlen(patientInfo.cPatientID)<1) ||
 			(strlen(patientInfo.cPatientName1)<1) ||
 			(strlen(patientInfo.cPatientName2)<1) ||
			(strlen(patientInfo.cPatientNameKana1)<1) ||
			(strlen(patientInfo.cPatientNameKana2)<1) )
		{
			g_pxLogger_.LogMessage(kErrorOnly,"ERROR: CPMSDB::importDB readed PMS_getPatientInfo is invalid \n" );
			g_pxLogger_.FlushLog();
			error_code = ExeCode_PatientInfo_Invalid;
			return false;
		}
	}

	{//check existing data
		PMS_patientInfo exist_patientInfo;
		memset(&exist_patientInfo,0,sizeof(PMS_patientInfo));
		bool run_flag=false;
		for(int run_i = 0;run_i<m_retryNN;run_i++){
			if((run_flag=readDB(patientInfo.cPatientID, exist_patientInfo))){
				break;
			}
		}
		if(run_flag){
			g_pxLogger_.LogMessage(kTrace,"CPMSDB::importDB read DB \n");
			g_pxLogger_.LogMessage(kTrace," Name %s, %s  \n",exist_patientInfo.cPatientName1,exist_patientInfo.cPatientName2);
			g_pxLogger_.LogMessage(kTrace," Kana %s, %s  \n",exist_patientInfo.cPatientNameKana1,exist_patientInfo.cPatientNameKana2);
			g_pxLogger_.FlushLog();

			if(	(strcmp(exist_patientInfo.cPatientName1,		patientInfo.cPatientName1) == 0) &&
				(strcmp(exist_patientInfo.cPatientName2,		patientInfo.cPatientName2) == 0) &&
				(strcmp(exist_patientInfo.cPatientNameKana1,	patientInfo.cPatientNameKana1) == 0) &&
				(strcmp(exist_patientInfo.cPatientNameKana2,	patientInfo.cPatientNameKana2) == 0)
				){
					g_pxLogger_.LogMessage(kWarning,"WARNING: CPMSDB::importDB [%s] is already existed , skip \n",PatientID.c_str());
					g_pxLogger_.FlushLog();
					error_code = ExeCode_PatientInfo_Existing;
					return true;
			}

			if(strlen(exist_patientInfo.cPatientID)>0){
			//Existing
				if(	(strcmp(exist_patientInfo.cPatientName1,		patientInfo.cPatientName1) != 0) ||
					(strcmp(exist_patientInfo.cPatientName2,		patientInfo.cPatientName2) != 0) ||
					(strcmp(exist_patientInfo.cPatientNameKana1,	patientInfo.cPatientNameKana1) != 0) ||
					(strcmp(exist_patientInfo.cPatientNameKana2,	patientInfo.cPatientNameKana2) != 0) ||
					(strcmp(exist_patientInfo.cBirthDate,			patientInfo.cBirthDate) != 0) ||
					(strcmp(exist_patientInfo.cSex,					patientInfo.cSex) != 0)
					){
						g_pxLogger_.LogMessage(kWarning,"ERROR: CPMSDB::importDB ID[%s] is already existed \n",PatientID.c_str());
						g_pxLogger_.FlushLog();
						error_code = ExeCode_ID_Existing;
						 
						if(!confirmErrorCode(error_code,msg)){
							return false;
						}
						
				}
			}
				 
		}
	}
	g_pxLogger_.LogMessage(kTrace," CPMSDB::importDB insert2DB -- start \n",PatientID.c_str());
	g_pxLogger_.FlushLog();
	bool run_flag=false;
	for(int run_i = 0;run_i<m_retryNN;run_i++){
		if((run_flag=insert2DB(patientInfo))){
			break;
		}
		g_pxLogger_.LogMessage(kTrace," CPMSDB::importDB insert2DB retry -- %d \n",run_i);
		g_pxLogger_.FlushLog();
	}
	
	if(!run_flag){
		g_pxLogger_.LogMessage(kErrorOnly,"ERROR: CPMSDB::importDB insert2DB [%s]  failed\n",PatientID.c_str());
		g_pxLogger_.FlushLog();
		error_code = ExeCode_InsertDB_Error;
		return false;
	}else{
		g_pxLogger_.LogMessage(kTrace,"CPMSDB::importDB insert2DB [%s]  OK\n",PatientID.c_str());
		g_pxLogger_.FlushLog();
	}


	return true;
}
bool CPMSDB::initDB()
{
	HRESULT		hr;
	hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	CHAR  MyComputerName[256];
	unsigned long nSize = sizeof(MyComputerName)-1 ;
	GetComputerName(MyComputerName, &nSize);

	AqUString strSQL;

	strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S%S;Integrated Security=SSPI;Initial Catalog=%S;APPLICATION NAME=%S", 
		MyComputerName,"\\SQLEXPRESS",
		"DENTALCT", 
		GetCurrentProcessName());

	CPxDB db;
	db.SetDBConectionInfo(strSQL);

	if( db.TestConnectionInfo(5)  != kOK )
	{
		g_pxLogger_.LogMessage(kErrorOnly,"ERROR: CPMSDB::initDB DB Connection failed\n");
	 
		g_pxLogger_.FlushLog();
		return false;
	}

	 
	return true;
}

inline void addID2String(std::string &str,const std::string &PatientID)
{
	str = std::string("ID[") + PatientID +"]:" + str;
};
void CPMSDB::reportReadCSVFileError(int erro,const std::string &PatientID)
{
	 
	std::string error_msg;
	switch(erro){
		case PMS_ERR_CODE_INI_READ:
		error_msg = "Iniファイルが開けません";
		break;
		case PMS_ERR_CODE_INI_INVALID:
		error_msg = "Iniファイルの内容が正しくありません";
		break;
		case PMS_ERR_CODE_NET_CONNECT:
		error_msg = "共有フォルダにアクセスできません";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_NO_ID_DIR:
		error_msg = "指定されたIDのフォルダが存在しません";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_CSV_READ:
		error_msg = "CSVファイルが開けません";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_CSV_INVALID:
		error_msg = "CSVファイルの内容が異常";
		addID2String(error_msg,PatientID);
		break;
		//
		case PMS_ERR_CODE_PID_INVALID:			// 患者名IDが異常。
		error_msg = "患者情報：患者名IDチェックエラー";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_NAME_INVALID:			// 患者名（文字コード）が異常。
		error_msg = "患者情報：患者名（文字コード）チェックエラー";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_NAME_KANA_INVALID:	// カナ患者名（文字コード）が異常。
		error_msg = "患者情報：カナ患者名（文字コード）チェックエラー";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_BOD_INVALID:			// 生年月日が異常。
		error_msg = "患者情報：生年月日チェックエラー";
		addID2String(error_msg,PatientID);
		break;
		case PMS_ERR_CODE_SX_INVALID:			// 性別が異常。
		error_msg = "患者情報：性別チェックエラー";
		addID2String(error_msg,PatientID);
		break;

	}
	m_lastError =  error_msg;
	g_pxLogger_.LogMessage(kErrorOnly,"ReadCSVFileError: %s\n",error_msg.c_str());
	g_pxLogger_.FlushLog();

}