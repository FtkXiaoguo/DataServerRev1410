#pragma once

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif
 
///
/// @NAME 
/// 　再構成ライブラリの共通出力インタフェース
//   
///


namespace PxReconLib
{
///
/// @NAME 
///  再構成ライブラリのエラー定義
//   
/// 
typedef enum {
	PANO_RECON_Success = 0,
	PANO_RECON_Failed,	
	PANO_RECON_OutOfMemory,
	PANO_RECON_TimeOut,
	PANO_RECON_InvalidArgument,
//	PANO_RECON_InvalidDevice,
	PANO_RECON_Uncomplete,
	PANO_RECON_Busy,
	PANO_RECON_Exception,
	PANO_RECON_Unknow,
} ReconLib_Error;

typedef int ReconLib_Status;

///
/// @NAME 
///  PxReconLoggerInterface -再構成ライブラリのログ出力インタフェース
//     上位アプリケーションはこれを実装し、再構成ライブラリにセットする。
/// 
/// @SECTION 
/// 再構成ライブラリのログ出力。	
class PxReconLoggerInterface
{
public:
 
	/////////////
	/// @brief メッセージの出力。       
	virtual void LogInfo(const char *fmt, ...) = 0;

	/////////////
	/// @brief エラーメッセージの出力。       
	virtual void LogError(const char *fmt, ...) = 0;

	/////////////
	/// @brief 警告メッセージの出力。  
	virtual void LogWarning(const char *fmt, ...) = 0;

	/////////////
	/// @brief デバッグメッセージの出力。 
	virtual void LogDebug(const char *fmt, ...) = 0;

};


///
/// @NAME 
///  PxReconLoggerInterface -再構成ライブラリのログ出力インタフェース
//     上位アプリケーションはこれを実装し、再構成ライブラリにセットする。
/// 
/// @SECTION 
/// 再構成ライブラリのログ出力。	
class PxReconObserver
{
public:
 
	/////////////
	/// @brief 内部処理状態の報告。       
	virtual void Progress(int progress,int step) = 0;
 
};


enum DataType
{
	DataType_unknown = 0,
	DataType_uchar,
	DataType_ushort,
	DataType_short,
	DataType_uint,
	DataType_int,
	DataType_float,
};
 
///
/// @NAME 
///  CPxPanoReconParam -再構成パラメータ
/// 
/// @SECTION 
///  再構成パラメータを管理する。
class  CPxReconParam
{
public:
	/////////////
	/// @brief 再構成パラメータデータを廃棄する。
	/// @param 　
	/// @return 　
	//  virtual void destroy(void) = 0;
};

///
/// @NAME 
///  CPxPanoProjData -再構成の入力データ(複数フレーム）
/// 
/// @SECTION 
///  再構成の入力データ。
class  CPxProjData
{
public:
	 
	
	/////////////
	/// @brief データを廃棄する。
	/// @return　　
	virtual void destroy(void ) = 0;
	 
	//////////////
	/// @brief 外部のデータ（メモリ）を取り付ける。
	///   NotReleaseがtrueの時は、destroyの時はメモリ釈放しない。
	/// @param pData [in] アタッチするデータのアドレス
	/// @param sizeX [in] データ幅の指定
	/// @param sizeY [in] データ高さの指定
	/// @param frames [in] フレーム数の指定　
	/// @param DataType [in] データタイプの指定　
	/// @param NotRelease [in] true: 取り付けたメモリ釈放しない。false:釈放する。　
	/// @return  
	 virtual void attachData(void *pData,int sizeX,int sizeY, int frames ,DataType type,bool NotRelease=false) = 0;

	/////////////
	/// @brief データ作成。
	/// @param sizeX [in] データ幅の指定
	/// @param sizeY [in] データ高さの指定
	/// @param frames [in] フレーム数の指定　
	/// @param DataType [in] データタイプの指定　
	/// @return ReconLib_Error　定義
	 virtual ReconLib_Status create(int sizeX,int sizeY, DataType type, int frames=1 ) = 0;

	 
	 /////////////
	/// @brief データタイプの取得。
	/// @return DataType　データタイプを返す。
	 virtual DataType getDataType(void ) const= 0;
	  

	/////////////
	/// @brief データサイズの取得。
	/// @param sizeX [out] データ幅を返す
	/// @param sizeY [out] データ高さを返す
	/// @param NumFrame [out] フレーム数を返す
	/// @return　
	 virtual void getDimension(int &sizeX,int &sizeY, int &frames ) const = 0;
	
	 
	/////////////
	/// @brief データのバッファーアドレスの取得。
	/// @return バッファーアドレスを返す
	 virtual void * getData(void) const = 0;
	  
protected:
	
 
 
};

///
/// @NAME 
///  CPxPanoReconData -再構成の結果データ
/// 
/// @SECTION 
///  再構成の結果データ。
////////////
class  CPxReconData 
{
public:
	 
	/////////////
	/// @brief データのバッファーアドレスの取得。
	/// @return バッファーアドレスを返す
	virtual const void* getData(void)  const = 0;


	virtual bool create(int sizeX, int sizeY, float  pitchX, float  pitchY) = 0;;
	virtual void destroy(void) = 0;;
	 
	/////////////
	/// @brief データタイプの取得。
	/// @return DataType　データタイプを返す。
	virtual DataType getDataType(void) const = 0;
	 

	/////////////
	/// @brief データサイズの取得。
	/// @param sizeX [out] データ幅を返す
	/// @param sizeY [out] データ高さを返す
	/// @param pitchX [out] ピッチＸを返す
	/// @param pitchY [out] ピッチＸを返す
	/// @return　
	virtual void getDimension(int &sizeX, int &sizeY, float &pitchX, float &pitchY) const = 0;
	 

	virtual void setDataType(DataType type) = 0;
	 
	virtual void setupPitch(float  pitchX, float  pitchY) = 0;
	 
	 
	/////////////
	/// @brief データ表示時の推奨WW/WLの取得。
	/// @param ww [out] データ表示のWWを返す
	/// @param wl [out] データ表示のWLを返す
	/// 
	virtual void getWWWL(int &ww, int &wl) const = 0;
	 
	/////////////
	/// @brief データ表示時の推奨WW/WLの設定。
	/// @param ww [in] データ表示のWWを返す
	/// @param wl [in] データ表示のWLを返す
	/// 
	virtual void setWWWL(int ww, int wl) = 0;
	 
 
	 
};

 

///
/// @NAME 
///  CPxReconEngine -パノラマ再構成処理エンジンのインタフェース
/// 
/// @SECTION 
///  再構成処理機能を提供する。
class CPxReconEngine
{
public:
	 
	/////////////
	/// @brief 再構成エンジンを廃棄する。
	/// @param 　
	/// @return 　
	virtual void destroy(void ) = 0;


	/////////////
	/// @brief 再構成エンジン内の処理パラメータの初期値の取得。
	/// @param  PxReconObserver [in] ObServerハンドル。再構成エンジン内から呼び出される。0(NULL)指定で解除。
	/// @return ReconLib_Error　定義
	virtual  ReconLib_Status addObserver(PxReconObserver *obServer)  = 0;
	
 
	/////////////
	/// @brief 再構成エンジン内の処理パラメータの初期値の取得。
	/// @param  paramBuff [out] 
	/// @return ReconLib_Error　定義。
	virtual  ReconLib_Status getDefualtParam(CPxReconParam *paramBuff) const = 0;

	/////////////
	/// @brief 再構成エンジン内のカレントパラメータを返す。
	/// @param  Param  
	/// @return 　NULL:　エラー。
	virtual  const CPxReconParam * getParam(void) const = 0;

	/////////////
	/// @brief 再構成処理のパラメータデータをセットする。
	/// @param  Param [in] 再構成処理のパラメータデータ
	/// @return ReconLib_Error　定義。
	virtual ReconLib_Status setParam(const CPxReconParam *Param) = 0;
   

	/////////////
	/// @brief 再構成処理を行う。
	/// @param projData [in] データ @ref CPxProjData。
	/// @return ReconLib_Error　定義。
	virtual ReconLib_Status setFrameData(const CPxProjData *projData) = 0;

#if 0
	/////////////
	/// @brief 再構成処理の状況を返す。
	/// @param progress [out] 処理進捗状況（0-99)を返す。
	/// @return ReconLib_Error　定義。
	virtual ReconLib_Status getStatus(int &progress)=0;
#endif
	/////////////
	/// @brief 再構成処理結果を取得する。
	/// 　    
	/// @return  CPxReconData,　正しい結果が無いときは0;
	virtual const CPxReconData* getReconData() const  = 0;

	//
};

 enum CephMode {
		CephMode_Unknown	= 0,
		CephMode_LA1  ,
	 	CephMode_LA2 , //inverse dir
		CephMode_PA1  ,
	 	CephMode_PA2 , //inverse dir
		CephMode_CARPUS ,
	};
 enum PanoMode
 {
	 PanoMode_unknown = 0,
	 PanoMode_Normal,
	 PanoMode_Child,
	 PanoMode_TMJ,
	 PanoMode_TMJ1_2,
	 PanoMode_TMJ2_2,
	 PanoMode_Bitewing,

 };
////////////
}