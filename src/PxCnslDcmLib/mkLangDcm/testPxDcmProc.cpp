// testPanoAPI.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
#pragma warning(disable : 4305)

#include "stdafx.h"

#include "PxCnslDcmLib.h"

#include "testPxDcmProc.h"
#include <math.h>
#include <string>
#include <QString>
#include <QTextCodec>
using namespace PxReconLib;

/*
* アプリケーション側のログ出力
*/
#include <stdio.h>
#include <stdarg.h>
  

 
/*
* アプリケーション側のログ出力
*/
//
//上位アプリケーションのログ出力実装。
//
PX2CnslLogger::PX2CnslLogger (  )
{
	 
         
	}

PX2CnslLogger::~PX2CnslLogger()
	{
		close();
	}

void PX2CnslLogger::close(void)
	{
		 
	}

	/// @brief メッセージの出力。       
void PX2CnslLogger:: LogInfo(const char *fmt, ...)
	{
		char _str_buff[2048];
		printf("LogInfo: "); 
		va_list args;
		va_start(args, fmt);
 //		vprintf(fmt, args);
 		vsnprintf_s(_str_buff,2048,fmt, args);
		
		printf(" %s \n",_str_buff);
		 
		va_end(args);
 
	}

void PX2CnslLogger::LogError(const char *fmt, ...)
	{
		char _str_buff[2048];
		printf("LogError: "); 
		va_list args;
		va_start(args, fmt);
	//	vprintf(fmt, args);
		vsnprintf_s(_str_buff,2048,fmt, args);
		
		printf(" %s \n",_str_buff);

		va_end(args);
 
	}

void PX2CnslLogger::LogWarning(const char *fmt, ...)
	{
		char _str_buff[2048];
		printf("LogWarning:  "); 
		va_list args;
		va_start(args, fmt);
	//	vprintf(fmt, args);

		vsnprintf_s(_str_buff,2048,fmt, args);
		
		printf(" %s \n",_str_buff);

		va_end(args);
	}

void PX2CnslLogger::LogDebug(const char *fmt, ...)
	{
		char _str_buff[2048];

		printf("LogDebug: "); 
		va_list args;
		va_start(args, fmt);
	//	vprintf(fmt, args);
		vsnprintf_s(_str_buff,2048,fmt, args); 
		
		printf(" %s \n",_str_buff);

		va_end(args);
	}




 

PX2CnslLogger g_test_logger ;




bool CTestDcmAPICls::init(void)
{
	//////////////////////////////////////////////
	//グローバル初期化　一回のみ
	//////////////////////////////////////////////

	//アプリケーション側のログ出力にセット。
	CPxCnslDcmLib::setLogger(&g_test_logger);

	//ライブラリの初期化
	//
	//注意：　指定された作業フォルダは先に作成しておくこと。
	ReconLib_Status retSts = CPxCnslDcmLib::initLib( );
	if(PXDCM_Success != retSts ){
		printf("initLib error \n");
		return false;
	}

	///////////////////////////////////////////////
	///////////////////////////////////////////////
//	CPxCnslDcmLib::setRootUID("1.2.392.200036.9163.1005");

	///////////////////////////
	printf(" >>>--pano recon init \n");

	
	 
	return true;

}
	
void CTestDcmAPICls::destroy(void)
{
	if(!m_pDcmEng) return;
	//////////////////////////////////////////////
	//全て終了
	//////////////////////////////////////////////
	m_pDcmEng->destroy();
	m_pDcmEng = 0;
}
 
class CPanoCoord
{
public:
	CPanoCoord(float pitchX, float pitchY, int sizeX, int sizeY)
	{
		m_pitchX = pitchX;
		m_pitchY = pitchY;
		m_sizeX = sizeX;
		m_sizeY = sizeY;
		//
		m_centerX = sizeX / 2.0f*m_pitchX;
		m_centerY = sizeY / 2.0f*m_pitchY;
	}
	int usr2Dot_X(float u){
		float f_data = u + m_centerX;
		int ret_i = (int)(f_data / m_pitchX);
		return ret_i;
	}
	int usr2Dot_Y(float u){
		float f_data = u + m_centerY;
		int ret_i = (int)(f_data / m_pitchY);
		 
		return ret_i;
	}
	bool valid_pos(int x, int y){
		bool ret_b =
			(x >= 0) &&
			(x < m_sizeX) &&
			(y >= 0) &&
			(y < m_sizeY)
			;
		return ret_b;
	}
protected:
	float m_centerX;
	float m_centerY;
	float m_pitchX;
	float m_pitchY;
	int m_sizeX;
	int m_sizeY;
};
#define M_PI (3.14159265359)

template<class dataT> 
class CMyArray
{
public:
	CMyArray(CPanoCoord *coord,dataT *Array, int sizeX, int sizeY)
	{
		m_coord = coord;
		m_ArrayBuffer = Array;
		m_sizeX = sizeX;
		m_sizeY = sizeY;
	}
	void setVal(int x, int y, dataT val){
		if (m_coord->valid_pos(x, y)){
			m_ArrayBuffer[y*m_sizeX + x] = val;
		}
	}
protected:
	CPanoCoord *m_coord;
	dataT *m_ArrayBuffer;
	int m_sizeX;
	int m_sizeY;

};


void drawLine(CPanoCoord &PanoCoord_h, CMyArray<unsigned short>  &myArray, float startX, float startY, float endX, float endY)
{//add circl
	//	float R = 10.0f;
	int NN = 1000;

	double dx = (endX - startX) / (NN - 1);
	double dy = (endY - startY) / (NN - 1);


	for (int i = 0; i < NN; i++){

		float f_pos_x = startX + i*dx;
		float f_pos_y = startY + i*dy;
		int i_pos_x = PanoCoord_h.usr2Dot_X(f_pos_x);
		int i_pos_y = PanoCoord_h.usr2Dot_Y(f_pos_y);


		if (PanoCoord_h.valid_pos(i_pos_x, i_pos_y)){
			for (int i_y = 0; i_y < 1; i_y++){
				for (int i_x = 0; i_x < 1; i_x++){
					myArray.setVal(i_pos_x + i_x, i_pos_y + i_y, 2000);
				}
			}
		}
	}
}
void drawCircle(CPanoCoord &PanoCoord_h, CMyArray<unsigned short>  &myArray, float CenterX = 0.0f, float CenterY = 0.0f, float R = 10.0f,int NN=1000,bool draw_r=false)
{//add circl
//	float R = 10.0f;
	 
	float c_x = CenterX;
	float c_y = CenterY;

	float del_a = M_PI*2.0 / NN;

	for (int i = 0; i < NN; i++){

		float f_pos_x = R*cos(del_a*i) + c_x;
		float f_pos_y = R*sin(del_a*i) + c_y;


		int i_pos_x = PanoCoord_h.usr2Dot_X(f_pos_x);
		int i_pos_y = PanoCoord_h.usr2Dot_Y(f_pos_y);

		
		if (PanoCoord_h.valid_pos(i_pos_x, i_pos_y)){
			if (draw_r){
				drawLine(PanoCoord_h, myArray, c_x, c_y, f_pos_x, f_pos_y);
			}
			for (int i_y = 0; i_y < 1; i_y++){
				for (int i_x = 0; i_x < 1; i_x++){
					myArray.setVal(i_pos_x + i_x, i_pos_y + i_y, 2000);
				}
			}
		}
	}
}
#include <QDir>
void CTestDcmAPICls::makeMulitLangDcm(LANG_DEF lang, MODALITY_DEF modality, bool bUseUTF8, int CTVolSize)
{
	QString dicom_folder_name = "dicoms";
	QDir qdcmdir(dicom_folder_name );
	qdcmdir.removeRecursively();
	if (!qdcmdir.exists()){
		QDir curdir(".");
		curdir.mkpath(dicom_folder_name);
	}
	
	//////////////////
	int sizeX = 3170;
	int sizeY = 1216;
	float pitchX = 0.0652f;
	float pitchY = 0.0652f;
	int sliceNum = 1;
	if (modality == MODALITY_ID_CT){
		sizeX = CTVolSize;
		sizeY = CTVolSize;
		pitchX = 0.5*(64.0f / CTVolSize);
		pitchY = 0.5*(64.0f / CTVolSize);
		sliceNum = CTVolSize;
	}
	if (modality == MODALITY_ID_CEPHA){
		sizeX = 1200;
		sizeY = 2000;
	}
	if (m_pDcmEng) m_pDcmEng->destroy();
	

	QString Patname = m_PatientName;
	{
	//test latin1
		std::string testStr = Patname.toLocal8Bit().data();
		const char *c_ptr = testStr.c_str();
		//std::string testLatinStr = Patname.toLatin1().data();
		//use latin1!
		std::string testLatinStr = Patname.toLatin1().toStdString();
		const char *c_Latin_ptr = testLatinStr.c_str();
		int dbg_xx = 0;
	}
	QTextCodec* codec = nullptr;
	bool bCheckJIS = false;

	switch (lang){
	case LANG_ID_CHINESE:
	{
		codec = QTextCodec::codecForName("GB18030");
		Patname = QString("=") + Patname;
	}
	break;
	case LANG_ID_RUSSIAN:
	{
		codec = QTextCodec::codecForName("Windows-1251");
	}
	break;
	case LANG_ID_JAPANESE:
	{
		if (bUseUTF8){
			//UTF8
			codec = QTextCodec::codecForName("UTF-8");
		}
		else{
			codec = QTextCodec::codecForName("Shift-JIS");
			//Convert to SJIS at first
			//Convert to JIS inside lib.
			bCheckJIS = true;
		}
		Patname = QString("=") + Patname;
	}
	break;
	case LANG_ID_LATIN:
	{
		//Latin-1
		codec = QTextCodec::codecForName("ISO-8859-1");
	}
	break;
	case LANG_ID_CHINESE_TW:
	{
		 //UTF8
		codec = QTextCodec::codecForName("UTF-8");
	}
	break;
	}
	if (codec == nullptr){
		return  ;
	}
	 
	QByteArray PatnameStr = codec->fromUnicode(Patname);
	 
	std::string PatID_str = m_PatientID.toStdString() ;
	//
 

	QByteArray OpNameStr = codec->fromUnicode(m_OpName);
	QByteArray PatComment_str = codec->fromUnicode(m_PatientComment);
	QByteArray StudyDes_str = codec->fromUnicode(m_StudyDes);
	QByteArray SeriesDes_str = codec->fromUnicode(m_SeriesDes);
	QByteArray InstitutionName_str = codec->fromUnicode(m_InstitutionName);
	QByteArray StationName_str = codec->fromUnicode(m_StationName);

	CPxDcmHeaderInfo pat_info;
	// 
	if (modality == MODALITY_ID_CEPHA){
		m_pDcmEng = CPxCnslDcmLib::createPxDcmEngine(CPxCnslDcmLib::Def_Cephalo);
	}
	else{
		m_pDcmEng = CPxCnslDcmLib::createPxDcmEngine(CPxCnslDcmLib::Def_Pano);
	}

	if (!m_pDcmEng){
		printf("createPxDcmEngine error \n");
		return;
	}
	m_pDcmEng->setCheckJISInside(bCheckJIS);
	m_pDcmEng->setupDcmServ(m_dcm_HostName.toStdString().c_str(), m_dcm_port, m_dcm_RemoteAE.toStdString().c_str());

	m_pDcmEng->beginNewStudy();
	 
	m_pDcmEng->getPatienInfo(pat_info);
	if (modality == MODALITY_ID_CEPHA){
		pat_info.m_ScanMode = CephMode_LA1;
	}

	switch (lang){
	case LANG_ID_CHINESE:
	{
		strncpy(pat_info.m_SpecificCharacterSet, "ISO_IR 100\\GB18030"
			, sizeof(pat_info.m_ManufacturerModelName));
	}
	break;
	case LANG_ID_RUSSIAN:
	{
		strncpy(pat_info.m_SpecificCharacterSet, "ISO_IR 100\\ISO_IR 144"
			, sizeof(pat_info.m_ManufacturerModelName));
	}
	break;
	case LANG_ID_JAPANESE:
	{
		if (bUseUTF8){
			strncpy(pat_info.m_SpecificCharacterSet, "ISO_IR 192"
				, sizeof(pat_info.m_ManufacturerModelName));
		}
		else{
			strncpy(pat_info.m_SpecificCharacterSet, "ISO 2022 ISO_IR 100\\ISO 2022 ISO_IR 13\\ISO 2022 IR 87"
				, sizeof(pat_info.m_ManufacturerModelName));
		}
	}
	break;
	case LANG_ID_LATIN:
	{
		//CT same as E2 //[\ISO 2022 IR 100\ISO 2022 IR 87
		strncpy(pat_info.m_SpecificCharacterSet, "\\ISO 2022 IR 100\\ISO 2022 IR 87"
			, sizeof(pat_info.m_ManufacturerModelName));
	}
	break;
	case LANG_ID_CHINESE_TW:
	{
		strncpy(pat_info.m_SpecificCharacterSet, "ISO_IR 192"
			, sizeof(pat_info.m_ManufacturerModelName));
	}
	break;
	}

	
	strncpy(pat_info.m_PatientComments, PatComment_str , sizeof(pat_info.m_PatientComments));
	strncpy(pat_info.m_StudyComment, StudyDes_str, sizeof(pat_info.m_StudyComment));
	strncpy(pat_info.m_SeriesComment, SeriesDes_str, sizeof(pat_info.m_SeriesComment));
	//
	strncpy(pat_info.m_InstitutionName, InstitutionName_str, sizeof(pat_info.m_InstitutionName));
	strncpy(pat_info.m_StationName, StationName_str, sizeof(pat_info.m_StationName));
	 
	strncpy(pat_info.m_Manufacturer, "PreXion", sizeof(pat_info.m_Manufacturer));
	strncpy(pat_info.m_ManufacturerModelName, "PreXion", sizeof(pat_info.m_ManufacturerModelName));

	//strncpy(pat_info.m_PatName,"PanoTest^Phantom",sizeof(pat_info.m_PatName));
	strncpy(pat_info.m_PatName, PatnameStr, sizeof(pat_info.m_PatName));
	strncpy(pat_info.m_PatID, PatID_str.c_str(), sizeof(pat_info.m_PatName));
	////
	strncpy(pat_info.m_ReferringPhysiciansName, OpNameStr, sizeof(pat_info.m_ReferringPhysiciansName));
	 
	////
	strncpy(pat_info.m_PatBirthDay, "19800101", sizeof(pat_info.m_PatBirthDay));
	strncpy(pat_info.m_PatAge, "032Y", sizeof(pat_info.m_PatAge));
	strncpy(pat_info.m_PatGender, "O", sizeof(pat_info.m_PatGender));
	 
	strncpy(pat_info.m_PatientComments, "Digital phantom", sizeof(pat_info.m_PatientComments));
	 
	///////////////////
	//X-Ray Info
	strncpy(pat_info.m_KVP, "90", sizeof(pat_info.m_KVP));
	strncpy(pat_info.m_XRayTubeCurrent, "3.4", sizeof(pat_info.m_XRayTubeCurrent));
	///
	
	{

		sprintf_s(pat_info.m_SeriesComment, sizeof(pat_info.m_ManufacturerModelName), "PitchX:%.3f,PitchY:%.3f", pitchX, pitchY);
		sprintf_s(pat_info.m_PixelSpacing, sizeof(pat_info.m_PixelSpacing), "%.4f\\%.4f", pitchY, pitchX);


		/////////////
		m_pDcmEng->setPatienInfo(pat_info);

				///////////
		m_pDcmEng->beginNewSeries();
		float SliceThickness = pitchX;
		for (int d_i = 0; d_i < sliceNum; d_i++){
			bool ret_b = m_pDcmEng->openDicom();
			unsigned short *data_buff = new unsigned short[sizeX*sizeY];

			setupTestData(data_buff, sizeX, sizeY, pitchX, pitchY);

			if (modality == MODALITY_ID_PANO){
				//Positioner Type(0018, 1508)
				m_pDcmEng->setTagValue(0x00181508, "PANORAMIC");
				 
				//Positioner Type(0008,0060)
				//m_pDcmEng->setTagValue(0x00080060, "PX");
				 
			}
			if (modality == MODALITY_ID_CT){
				//0008,0016  SOP Class UID: 1.2.840.10008.5.1.4.1.1.2 
				m_pDcmEng->setTagValue(0x00080016, "1.2.840.10008.5.1.4.1.1.2");
				//0018,0050  Slice Thickness: 0.987500
				m_pDcmEng->setTagValue(0x00180050, std::to_string(SliceThickness).c_str());
				//0020,0013  Image Number: 1 
				m_pDcmEng->setTagValue(0x00200013, std::to_string(d_i+1).c_str());
				//0020,0032  Image Position (Patient): -40.185001373\-40.185001373\-36.537498
				m_pDcmEng->setTagValue(0x00200032, 
					(std::string("-40.0\\-40.0\\") + std::to_string(-36.0 + SliceThickness*d_i)).c_str());
				//0020, 0037  Image Orientation(Patient) : 1.000000000\0.000000000\0.000000000\0.000000000\1.000000000\0.000000000
				m_pDcmEng->setTagValue(0x00200037, "1.0\\0.0\\0.0\\0.0\\1.0\\0.0");
			}
			///
			m_pDcmEng->setupPixelData(data_buff, sizeX, sizeY);

#if 1
			std::string savedicomFile = dicom_folder_name.toStdString()+"\\test_";
			savedicomFile = savedicomFile + std::to_string(d_i) + ".dcm";
			m_pDcmEng->saveDicom(savedicomFile.c_str());  // "test.dcm");
#else
			m_pDcmEng->saveDicom(0);//just for setup TAG
#endif
			
			ret_b = m_pDcmEng->sendDicom();
			//

			delete[] data_buff;
			m_pDcmEng->closeDicom();
		}
	}
	

}

void CTestDcmAPICls::setupTestData(unsigned short *data_buff, int sizeX, int sizeY, float pitchX, float pitchY)
{
	for (int y_i = 0; y_i < sizeY; y_i++){
		for (int x_i = 0; x_i < sizeX; x_i++){

			data_buff[y_i*sizeX + x_i] = 100;// x_i;
		}

	}
	float pano_w = pitchX*sizeX;
	float pano_h = pitchY*sizeY;
	float spot_d = sqrt(pano_w*pano_w + pano_h*pano_h) / 500.0f;

	CPanoCoord PanoCoord_h(pitchX, pitchY, sizeX, sizeY);
	CMyArray<unsigned short>  myArray(&PanoCoord_h, data_buff, sizeX, sizeY);

	drawCircle(PanoCoord_h, myArray, 0.0f, -5.0f, 5.0 /*R*/);
	drawCircle(PanoCoord_h, myArray, 0.0f, -5.0f, 5.0 + spot_d /*R*/, 12, true);

	drawCircle(PanoCoord_h, myArray, 20.0f, -5.0f, 10.0 /*R*/);
	drawCircle(PanoCoord_h, myArray, 20.0f, -5.0f, 10.0 + spot_d /*R*/, 12, true);

	drawCircle(PanoCoord_h, myArray, 60.0f, -5.0f, 20.0 /*R*/);
	drawCircle(PanoCoord_h, myArray, 60.0f, -5.0f, 20.0 + spot_d /*R*/, 12, true);
	/////////////
	{
		float sq_c_x = -60.0f;
		float sq_c_y = -20.0f;
		float sq_w = 40.0f;

		drawLine(PanoCoord_h, myArray, sq_c_x, sq_c_y, sq_c_x, sq_c_y + sq_w);
		drawLine(PanoCoord_h, myArray, sq_c_x, sq_c_y + sq_w, sq_c_x + sq_w, sq_c_y + sq_w);
		drawLine(PanoCoord_h, myArray, sq_c_x + sq_w, sq_c_y + sq_w, sq_c_x + sq_w, sq_c_y);
		drawLine(PanoCoord_h, myArray, sq_c_x + sq_w, sq_c_y, sq_c_x, sq_c_y);
	}

	{
		float sq_c_x = -60.0f;
		float sq_c_y = -20.0f;
		float sq_w = 20.0f;

		drawLine(PanoCoord_h, myArray, sq_c_x, sq_c_y, sq_c_x, sq_c_y + sq_w);
		drawLine(PanoCoord_h, myArray, sq_c_x, sq_c_y + sq_w, sq_c_x + sq_w, sq_c_y + sq_w);
		drawLine(PanoCoord_h, myArray, sq_c_x + sq_w, sq_c_y + sq_w, sq_c_x + sq_w, sq_c_y);
		drawLine(PanoCoord_h, myArray, sq_c_x + sq_w, sq_c_y, sq_c_x, sq_c_y);
	}


}
void CTestDcmAPICls::doTestCepha(void)
{
	
	//////////////////
	int sizeX = 2650;
	int sizeY = 2240;

	float pitch_tabl[] = {
		0.0652, 0.0652,

		0.0920, 0.0652,
		0.0652, 0.0920,

	};

	if(m_pDcmEng) {
		m_pDcmEng->destroy();
	}
	//パノラマー再構成エンジンの作成
	m_pDcmEng = CPxCnslDcmLib::createPxDcmEngine(CPxCnslDcmLib::Def_Cephalo);

	if(!m_pDcmEng){
		printf("createPxDcmEngine error \n");
		return  ;
	}
	CPxDcmHeaderInfo pat_info;


	m_pDcmEng->beginNewStudy();

	m_pDcmEng->getPatienInfo(pat_info);


	strncpy(pat_info.m_StudyComment, "test Cepha Size", sizeof(pat_info.m_ManufacturerModelName));
	strncpy(pat_info.m_Manufacturer, "PreXion", sizeof(pat_info.m_Manufacturer));
	strncpy(pat_info.m_ManufacturerModelName, "PreXion", sizeof(pat_info.m_ManufacturerModelName));
	strncpy(pat_info.m_PatName, "CephaTest^Phantom", sizeof(pat_info.m_PatName));
	strncpy(pat_info.m_PatID, "88889999", sizeof(pat_info.m_PatName));
	////
	strncpy(pat_info.m_PatBirthDay, "19800101", sizeof(pat_info.m_PatBirthDay));
	strncpy(pat_info.m_PatAge, "032Y", sizeof(pat_info.m_PatAge));
	strncpy(pat_info.m_PatGender, "O", sizeof(pat_info.m_PatGender));

	strncpy(pat_info.m_PatientComments, "Digital phantom", sizeof(pat_info.m_PatientComments));

	pat_info.m_WindowCenter = 1500;
	pat_info.m_WindowWidth = 3000;

 

 
	for (int run_i = 0; run_i < sizeof(pitch_tabl) / sizeof(float) / 2; run_i++){
		//////////////////
		 

		float pitchX = pitch_tabl[2 * run_i];// 0.0920;
		float pitchY = pitch_tabl[2 * run_i + 1];// 0.0652;


		
		sprintf_s(pat_info.m_PixelSpacing, sizeof(pat_info.m_PixelSpacing), "%.4f\\%.4f", pitchY, pitchX);
		sprintf_s(pat_info.m_ImagerPixelSpacing, sizeof(pat_info.m_ImagerPixelSpacing), "%.4f\\%.4f", pitchY*1.1f, pitchX*1.1f);



		//strcpy(pat_info.m_InstanceCreatorUID,"1.2.392.200036.9163.30.8899");
		//
		////////////////

		///////////////////
		pat_info.m_ScanMode = CephMode_LA1;
		sprintf_s(pat_info.m_SeriesComment, sizeof(pat_info.m_ManufacturerModelName), "LA: PitchX:%.3f,PitchY:%.3f", pitchX, pitchY);

		m_pDcmEng->setPatienInfo(pat_info);


		m_pDcmEng->beginNewSeries();
		 

		{
			bool ret_b = m_pDcmEng->openDicom();


			unsigned short *data_buff = new unsigned short[sizeX*sizeY];

#if 0
			for (int y_i = 0; y_i < sizeY; y_i++){
				for (int x_i = 0; x_i < sizeX; x_i++){

					unsigned short set_val = 100;
					//
					if ((y_i % 100 == 0) && (x_i % 100 == 0)){
						set_val = 15000;
					}
					data_buff[y_i*sizeX + x_i] = set_val;

				}

			}
#else
			setupTestData( data_buff,  sizeX,  sizeY,  pitchX,  pitchY);
#endif
			//
			m_pDcmEng->setupPixelData(data_buff, sizeX, sizeY);
#if 0
			m_pDcmEng->saveDicom("test_cepha_LA.dcm");
#else
			m_pDcmEng->saveDicom(0);//just for setup TAG
#endif

			m_pDcmEng->setupDcmServ(m_dcm_HostName.toStdString().c_str(), m_dcm_port, m_dcm_RemoteAE.toStdString().c_str());
			ret_b = m_pDcmEng->sendDicom();

			m_pDcmEng->closeDicom();
			///
			delete[] data_buff;
		}

		//
		pat_info.m_ScanMode = CephMode_PA1;
		sprintf_s(pat_info.m_SeriesComment, sizeof(pat_info.m_ManufacturerModelName), "PA: PitchX:%.3f,PitchY:%.3f", pitchX, pitchY);
		//

		m_pDcmEng->setPatienInfo(pat_info);

		m_pDcmEng->beginNewSeries();
		 

		{


			bool ret_b = m_pDcmEng->openDicom();


			unsigned short *data_buff = new unsigned short[sizeX*sizeY];

#if 0
			for (int y_i = 0; y_i < sizeY; y_i++){
				for (int x_i = 0; x_i < sizeX; x_i++){

					data_buff[y_i*sizeX + x_i] = x_i * 6;
				}

			}
#else
			setupTestData(data_buff, sizeX, sizeY, pitchX, pitchY);
#endif
			//
			m_pDcmEng->setupPixelData(data_buff, sizeX, sizeY);
#if 0
			m_pDcmEng->saveDicom("test_cepha_PA.dcm");
#else
			m_pDcmEng->saveDicom(0);//just for setup TAG
#endif
			m_pDcmEng->setupDcmServ(m_dcm_HostName.toStdString().c_str(), m_dcm_port, m_dcm_RemoteAE.toStdString().c_str());
			ret_b = m_pDcmEng->sendDicom();


			m_pDcmEng->closeDicom();
			///
			delete[] data_buff;
		}


	}
}


/////////////////////////////////

#define PX1_DATA

#include <Windows.h>
#include <string>
#include <vector>
inline bool getPropertyFromFileName(const std::string fileName, float &ww, float &wl, int &sizeX, int &sizeY)
{
	{//WW/WL
		auto pos1 = fileName.find("ww_");
		if (pos1 == std::string::npos) return false;

		std::string str_temp = fileName.substr(pos1 + 3);
		pos1 = str_temp.find("_wl_");
		if (pos1 < 1) return false;
		//
		std::string str_ww = str_temp.substr(0, pos1);
		ww = std::stof(str_ww);
		//
		str_temp = str_temp.substr(pos1 + 4);
		pos1 = str_temp.find("_");
		if (pos1 < 1) return false;
		std::string str_wl = str_temp.substr(0, pos1);

		wl = std::stof(str_wl);
	}
	{//size

		auto pos1 = fileName.find("_Dat_");
		if (pos1 == std::string::npos ) return false;

		std::string str_temp = fileName.substr(pos1 + 5);
		pos1 = str_temp.find("_");
		if (pos1 < 1) return false;
		//
		std::string str_x = str_temp.substr(0, pos1);
		sizeX = std::stoi(str_x);
		//
		str_temp = str_temp.substr(pos1 + 1);
		pos1 = str_temp.find("_");
		if (pos1 < 1) return false;
		std::string str_y = str_temp.substr(0, pos1);

		sizeY = std::stoi(str_y);
	}

	return true;
}
 