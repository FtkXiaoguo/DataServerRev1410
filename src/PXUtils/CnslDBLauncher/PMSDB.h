#pragma once

#include <string>


class CPMSDB
{
public:
	enum EXE_CODE {
		ExeCode_unknown = -1,
		ExeCode_InitDB_Error,
		ExeCode_InsertDB_Error,
		ExeCode_No_Error,
		ExeCode_ID_Existing,
		ExeCode_PatientInfo_Existing,
		ExeCode_GetPatientInfo_Error,
		ExeCode_PatientInfo_Invalid,
		ExeCode_Failed,
		
	};
	CPMSDB(void);
	~CPMSDB(void);


	bool importDB(const std::string &PatientID,EXE_CODE &error_code, std::string &msg);

	const std::string &getLastError() const { return m_lastError;};
protected:
	void reportReadCSVFileError(int erro,const std::string &PatientID);
	virtual bool confirmErrorCode(const EXE_CODE &error_code,const std::string &msg){ return true;};
	int m_retryNN;
	bool initDB();
	std::string m_lastError;
};
