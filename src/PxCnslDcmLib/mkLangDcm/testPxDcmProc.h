//
// パノラマ再構成エンジンの使用方法の説明。
// 関数の呼び出し順番を主目的とする。
// パラメータおよびデータの詳細設定は省略。
///

 
#pragma once

//パノラマ再構成エンジンのヘッダ情報
#include "PxCnslDcmLib.h"
#include <QString>
using namespace PxReconLib;

  
class CTestDcmAPICls
{
public:
	enum LANG_DEF {
		LANG_ID_CHINESE = 0,
		LANG_ID_RUSSIAN,
		LANG_ID_JAPANESE,
		LANG_ID_LATIN,
		LANG_ID_CHINESE_TW,

	};
	enum MODALITY_DEF {
		MODALITY_ID_PANO = 0,
		MODALITY_ID_CEPHA,
		MODALITY_ID_CT,
	
	};
	CTestDcmAPICls(void) {
		 m_pDcmEng = 0;
		 m_dcm_port = 105;
		 m_dcm_HostName = "127.0.0.1";
		 m_dcm_RemoteAE = "DENTALCT_AE";
	}
	~CTestDcmAPICls() {
		 
	}
	void makeMulitLangDcm(LANG_DEF lang, MODALITY_DEF modality, bool bUseUTF8 = false, int CTVolSize = 64);
	void doTestCepha(void);
	bool init(void);
	void destroy(void);
	/* 
	void setupPatInfo(const QString &Name, const QString& ID,const QString OpName,const QString comment){
		m_PatientName = Name;
		m_PatientID = ID;
		m_OpName = OpName;
		m_StudyComment = comment;
	}
	*/
protected:
	void setupTestData(unsigned short *dataBuff, int sizeX, int sizeY, float pitchX, float pitchY);
	CPxDcmEngine *m_pDcmEng;
	int m_dcm_port;
 
	QString m_dcm_HostName;
	QString m_dcm_RemoteAE;
public:
	QString m_PatientName;
	QString m_PatientID;
	QString m_OpName;
	QString m_PatientComment;
	QString m_StudyDes;
	QString m_SeriesDes;
	QString m_InstitutionName;
	QString m_StationName;

};


 
///
//上位アプリケーションのログ出力実装。
//
class QFile;


class PX2CnslLogger : public PxReconLoggerInterface
{
public:
 /////////////
	PX2CnslLogger ( );
	~PX2CnslLogger();
	void close(void);

	/// @brief メッセージの出力。       
	virtual void LogInfo(const char *fmt, ...);

	virtual void LogError(const char *fmt, ...);
	virtual void LogWarning(const char *fmt, ...);
	virtual void LogDebug(const char *fmt, ...);

protected:
	 
 
};
 