
#ifndef PMSIF_H
#define PMSIF_H

#define DllImport   __declspec( dllimport )
#define DllExport   __declspec( dllexport )

typedef struct {
	char cPatientID[24];
	char cPatientName1[256];
	char cPatientName2[256];
	char cPatientNameKana1[256];
	char cPatientNameKana2[256];
	char cBirthDate[24];
	char cSex[4];
	char cDesc[256];
} PMS_patientInfo;

enum {
	PMS_ERR_CODE_OK = 0,										// 成功
	PMS_ERR_CODE_INI_READ,										// Iniファイルが開けない。存在しない。
	PMS_ERR_CODE_INI_INVALID,									// Iniファイルの内容が正しくない。
	PMS_ERR_CODE_NET_CONNECT,									// 共有フォルダにアクセスできない。
	PMS_ERR_CODE_NO_ID_DIR,										// 指定されたIDのフォルダがない。
	PMS_ERR_CODE_CSV_READ,										// CSVファイルが存在しない。開けない。
	PMS_ERR_CODE_CSV_INVALID,									// CSVファイルの内容が異常。
	PMS_ERR_CODE_PID_INVALID,									// 患者名IDが異常。
	PMS_ERR_CODE_NAME_INVALID,									// 患者名（文字コード）が異常。
	PMS_ERR_CODE_NAME_KANA_INVALID,								// カナ患者名（文字コード）が異常。
	PMS_ERR_CODE_BOD_INVALID,									// 生年月日が異常。
	PMS_ERR_CODE_SX_INVALID										// 性別が異常。
};


DllExport int PMS_initialize();
DllExport int PMS_getPatientInfo( const char*, PMS_patientInfo& );
DllExport void PMS_getVersion( int&, int&, int&, int& );


#endif	// PMSIF_H

