#pragma once

#include "PxReconApiCom.h"

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

#ifdef MakePxCnslDcmAPI 
	#define IPxPxCnslDcmDefDllCls __declspec(dllexport)
	 
#else 
	#define IPxPxCnslDcmDefDllCls __declspec(dllimport)
	 
#endif


namespace PxReconLib 
{
  
typedef enum {
	PXDCM_Success = 0,
	PXDCM_Failed,	
	PXDCM_OutOfMemory,
	PXDCM_TimeOut,
	PXDCM_InvalidArgument,
//	PXDCM_InvalidDevice,
	PXDCM_Uncomplete,
	PXDCM_Busy,
	PXDCM_Exception,
	PXDCM_Unknow,
} _Error;

typedef int ReconLib_Status;



///
/// @NAME 
///  CPxCephReconData -DICOMヘッダ情報
/// 
/// @SECTION 
///  -DICOMヘッダ情報。
////////////
class IPxPxCnslDcmDefDllCls CPxDcmHeaderInfo  
{
public:
	static bool isValid(const char*tagVale) {
			return tagVale[0] != 0;
		}
	CPxDcmHeaderInfo();
 
	virtual ~CPxDcmHeaderInfo();
//#define assignValue(dest, src)  strncpy_s((dest),sizeof(dest),(src),sizeof(dest)); 
	 
#define DCM_CHAR_LEN (64)
#define DCM_PN_LEN (64*3)
	//#1288,#1495,#1496 
	//----------------
	// PN: 
	// Component-1 (64 Bytes) = Component-2 (64 Bytes) = Component-3 (64 Bytes)
	//   Roma-char            =  Japanese-char(Kanji)  =   (katakana/hiragana)
	//----------------
	// 1 component
	//  field-1   ^ field-2   ^ field-3   ^ field-4   ^ field-1   
	//   yamada   ^  taro
	//    Obama   ^  Barack   ^ Hussein   ^  Mr.      ^  Jr.
		char m_SpecificCharacterSet[DCM_PN_LEN + 2];//(0008,0005) //#1288 for Chinese
		char m_PatName[DCM_PN_LEN+2];				//(0010,0010)
		char m_PatID[DCM_CHAR_LEN+2];				//(0010,0020) 
		char m_PatBirthDay[DCM_CHAR_LEN+2];			//(0010,0030) 
		char m_PatAge[DCM_CHAR_LEN+2];				//(0010,1010) 
		char m_PatGender[4];						//(0010,0040)
		char m_PatientComments[DCM_CHAR_LEN+2];		//(0010,4000)//#103
		unsigned short m_PregnancyStatus;			//(0010,21C0)  #611 妊娠状況
 //
		char m_KVP[DCM_CHAR_LEN+2];					//(0018,0060)
		char m_XRayTubeCurrent[DCM_CHAR_LEN+2];		//(0018,1151) //uA
 		char m_ExposureTime[DCM_CHAR_LEN+2];		//(0018,1150)
		char m_ExposureVal[DCM_CHAR_LEN+2];			//(0018,1152) //曝射量
		//
		char m_ViewPosition[DCM_CHAR_LEN+2];		//(0018,5101)

		//
		char m_InstanceCreatorUID[DCM_CHAR_LEN + 2];	//((0008,0014))
		///
		char m_StudyComment[DCM_CHAR_LEN+2];		//(0008,1030)
 		char m_SeriesComment[DCM_CHAR_LEN+2];		//(0008,103e) 
		char m_StationName[DCM_CHAR_LEN+2];			//(0008,1010) 
		char m_InstitutionName[DCM_CHAR_LEN+2];		//(0008,0080)
		///
		char m_Manufacturer[DCM_CHAR_LEN+2];			//(0008,0070)
		char m_ManufacturerModelName[DCM_CHAR_LEN+2];	//(0008,1090)
		/////
		char m_ReferringPhysiciansName[DCM_PN_LEN+2];		//(0008,0090)
		char m_OperatorsName[DCM_PN_LEN+2];		//(0008,1070)	

		char m_AccessionNumber[DCM_CHAR_LEN+2];		//(0008,0050) //#201 2013/08/22
//
		char m_StudyDate[DCM_CHAR_LEN+2];			//(0008,0020) 
		char m_StudyTime[DCM_CHAR_LEN+2];			//(0008,0030)
		char m_SeriesDate[DCM_CHAR_LEN+2];			//(0008,0021) 
		char m_SeriesTime[DCM_CHAR_LEN+2];			//(0008,0031)
//
		char m_SeriesNumber[DCM_CHAR_LEN+2];		//(0020,0011) 
		char m_ImageNumber[DCM_CHAR_LEN+2];			//(0020,0013)
//
		char m_StudyInstanceUID[DCM_CHAR_LEN+2];	//(0020,000d)
		char m_SeriesInstanceUID[DCM_CHAR_LEN+2];	//(0020,000e) 
		char m_SOPInstanceUID[DCM_CHAR_LEN+2];		//(0008,0018)
 
//		char m_PatientOrientation[DCM_CHAR_LEN+2];	//(0020,0020)  
//
		char m_PixelSpacing[DCM_CHAR_LEN+2];		//(0028,0030)
		char m_ImagerPixelSpacing[DCM_CHAR_LEN+2];		//(0018,1164)	// #283 2015/07/01
		//
		///
		char m_ScanOptions[DCM_CHAR_LEN + 2];			//(0018,0022)
		char m_DeviceSerialNumber[DCM_CHAR_LEN+2];	//(0018,1000)
		char m_SoftwareVersion[DCM_CHAR_LEN+2];		//(0018,1020)
		//
		int m_WindowCenter;							//(0028,1050) 
		int m_WindowWidth;							//(0028,1051)

		int m_ScanMode;
};

///
/// @NAME 
///  CPxDcmEngine -DICOM処理エンジンのインタフェース
/// 
/// @SECTION 
///  DICOM処理機能を提供する。
class CPxDcmEngine  
{
public:
	
	/////////////
	/// @brief 再構成エンジンを廃棄する。
	/// @param 　
	/// @return 　
	virtual void destroy(void ) = 0;
	
	//#1288,#1495,#1496 
	/////////////
	/// @brief 文字列の自動日本語JIS変換（Default True)。
	/// @param  bCheckJIS　[in] set to false when Chinese/Russian language
	/// @return  
	virtual void setCheckJISInside(bool bCheckJIS) = 0;

	/////////////
	/// @brief 新規Studyデータの作成。
	/// @param UID　[in] StudyUIDの指定。NULL：内部自動生成。　
	/// @return 　true: 正常終了, false: エラー
	virtual bool beginNewStudy(const char *UID=0 ) = 0;
	
	/////////////
	/// @brief 新規シリーズデータの作成。StudyUIDのもとにSeriesUID新規作成する。
	/// @param 　
	/// @return 　true: 正常終了, false: エラー
	virtual bool beginNewSeries(void) = 0;


	/////////////
	/// @brief DICOMに書き込むヘッダー情報の取得
	/// @param param_out　[out] DICOMヘッダー情報を返す。　 　
	/// @return 
	virtual void getPatienInfo(CPxDcmHeaderInfo &param_out)			= 0;

	/////////////
	/// @brief DICOMに書き込むヘッダー情報の設定
	/// @param param_in　[in] 設定する DICOMヘッダー情報。
	/// @return 
	virtual void setPatienInfo(const CPxDcmHeaderInfo &param_in)	= 0;

	//#1498
	////////////
	/// @brief DICOMに指定タグの値を設定する
	/// @param tagID　[in] タグＩＤ。
	/// @param tagVal　[in] 指定するタグの値。
	/// @return 
	virtual void setTagValue(unsigned long tagID, const char *tagVal) = 0;

	////////////
	/// @brief DICOMに指定タグの値を取得する
	/// @param tagID　[in] タグＩＤ。
	/// @param tagVal　[out] タグの値を返す。
	/// @param tagValSize　[in] タグの値を保存できるtagValのバッファサイズ。
	/// @return 
	virtual void getTagValue(unsigned long tagID, char *tagVal, int tagValSize) = 0;

	/////////////
	/// @brief DICOMに出力Pixelデータの指定
	/// @param data　[in] Pixelのバイナリ（unsigned short)。
	/// @param sizeX　[in] PixelデータサイズX
	/// @param sizeY　[in] PixelデータサイズY
	/// @return  true: 正常終了, false: エラー
	virtual bool setupPixelData(const void* data,int sizeX,int sizeY)	= 0;

	/////////////
	/// @brief DICOMデータのオープン（２Dデータ一枚）
	/// @param  
	/// @return  true: 正常終了, false: エラー
	virtual bool openDicom(void) = 0;

	/////////////
	/// @brief DICOMデータのクローズ
	/// @param  
	/// @return  true: 正常終了, false: エラー
	virtual void closeDicom(void) = 0;

	/////////////
	/// @brief セファロ再構成処理結果をDICOMファイルに保存。
	/// 　    
	/// @return  true: 正常終了, false: エラー。
	virtual bool saveDicom(const char *fileName) = 0;

	/////////////
	/// @brief DICOM送信先の指定。
	/// @param host　   [in] DICOMサーバーのホスト名（IP)の指定。初期値： localhost 
	/// @param portNum　[in] DICOMサーバーの受信TCP/IPポート番号。初期値： 105 
	/// @param AE　     [in] DICOMサーバー側のAEを指定する。初期値： DENTALCT_AE 
	/// @return   
	virtual void setupDcmServ( const char *host,int portNum, const char*AE) = 0;

	/////////////
	/// @brief 送信側のAE（Local)の指定。
	/// @param AE　   [in] 送信側のAE（Local)。初期値： PX2DCM_AE  
	/// @return   
	virtual void setupLocalAE(const char *AE) = 0;

	/////////////
	/// @brief DICOMデータをDICOMサーバーへ送信する。
	/// @param
	/// @return  true: 正常終了, false: エラー。  
	virtual bool sendDicom(void) = 0;
};

  
///
///
/// @NAME 
///  CPxPanoReconLib -DICOM処理ライブラリ
/// 
/// @SECTION 
///  DICOM処理ライブラリを管理するトップクラス。
///
class IPxPxCnslDcmDefDllCls  CPxCnslDcmLib
{
public:
	enum ModalityDef {
		Def_Unknown = 0,
		Def_Pano,
		Def_Cephalo,
		 
	};

	CPxCnslDcmLib(void);
	~CPxCnslDcmLib(void);
 
	/////////////
	/// @brief ROOT UIDの指定。
	/// @param 　rootUID [in]  指定するRootUID
	/// @return 　
	static void setRootUID(const char* rootUID );

	/////////////
	/// @brief DICOM処理ライブラリ初期化。
	/// 
	/// @param  tempFolder [in] DICOM処理エンジン内部用の作業フォルダの指定。
	/// @return ReconLib_Error　定義 
	static ReconLib_Status initLib(const char *tempFolder=0);

	static ReconLib_Status releaseLib(void);

	/////////////
	/// @brief DICOM処理エンジンのログ出力の設定。
	/// @param  logger [in]  @ref PxPanoReconLoggerInterface 上位アプリケーションで実装したログ出力。  
	/// @return   
 	static void setLogger(PxReconLoggerInterface *logger);

	/////////////
	/// @brief DICOM処理エンジンのインタフェースの作成
	/// @param modality　[in] パノラマ、セファロの指定。
	/// @return  正常：　CPxPanoReconEngineのインタフェース, 失敗: 0
	static CPxDcmEngine *createPxDcmEngine(ModalityDef modality);
};

////////////
}