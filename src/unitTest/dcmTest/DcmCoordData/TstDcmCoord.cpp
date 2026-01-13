// testFxDcmData.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

 
#include "IDcmLibApi.h"
#include "PxDicomMessage.h"
#include "PxDicomImage.h"
#include "../PXSendDcmTest/genData/OctDatBase.h"
#include "../testDcmSrc/TstVLIDicomImage.h"
#define DICOM_MSG CPxDicomMessage
#define DICOM_IMG CPxDicomImage
using namespace XTDcmLib;

 
#include "AqCore/TRPlatform.h"

#define MyRootUID "1.2.392.200036.9163"   //PreXion DICOM root UID
std::string genNewUID(const std::string &id)
{
	char _char_buff[128];
	unsigned long tick_count = ::GetTickCount();
	sprintf(_char_buff, ".%u", tick_count);
	std::string new_instance_uid = std::string(MyRootUID) + "." + id + _char_buff;
	return new_instance_uid;
}

int doTstDicomCoord()
{
	CTstVLIDicomImage TstDicomImage;

	if (!TstDicomImage.initDcmTk()){
		printf("initDcmTk error \n");
		return 0;
	}

	std::string outputDir = "E:\\temp\\outputdicom";
	std::string CurPatientName = "test^dicomCoord";
	std::string CurPatientID = "000111";
	std::string CurStudyID = "000222";
	std::string CurStudyInstanceUID = genNewUID("63");
	std::string CurSeriesInstanceUID = CurStudyInstanceUID;
	int sizeX = 512;
	int sizeY = 512;
	int sizeZ = 512;
	float pitchX = 0.1f;
	float pitchY = 0.1f;
	float pitchZ = 0.1f;
	
	COctDatBase *DataGenPtr = new COctDatBase;
	DataGenPtr->setDim(sizeX, sizeY, sizeZ);

	int RescaleIntercept = -1024;
	float RescaleSlope = 1.0f;
 
	unsigned int setPixelValue = 200;
	DataGenPtr->genData(DataType_USHORT, 10, 200, (1000.0 - RescaleIntercept) / RescaleSlope);

	
	int ImageCount = 1;
	bool error_flag = false;
	for (int z_i = 0; z_i<sizeZ; z_i++){
		 
		CTstVLIDicomImage dicomImage;
		dicomImage.openNewDicom();
 
		CPxDicomImage *dicomImage_dicom = dicomImage.getDicomImage();
 
		if (!dicomImage_dicom) {
			error_flag = true;
			break;
		}


		dicomImage.setPatientName(CurPatientName);
		dicomImage.setPatientID(CurPatientID);
		dicomImage.setStudyID(CurStudyID);

		dicomImage.setStudyInstanceUID(CurStudyInstanceUID);
		//	dicomImage.setSeriesInstanceUID(m_CurSeriesID);
		dicomImage.setSeriesInstanceUID(CurSeriesInstanceUID);

		dicomImage.setupPitch(pitchX, pitchY, pitchZ);

		dicomImage.setupRescale(RescaleIntercept, RescaleSlope, true);
		dicomImage.setBits(12, 11, 16);
		 
		{

			//			unsigned short *data = new unsigned short[sizeX*sizeY];
			unsigned short *data = (unsigned short *)(DataGenPtr->getDataPtr(z_i));
			for (int y_i = 0; y_i <sizeY; y_i++){
				for (int x_i = 0; x_i <sizeX; x_i++){
					if ((x_i == 10) && (y_i==10)){
						data[y_i*sizeX + x_i] =  1000;
					}
					if ((x_i == 10) && (y_i == 20)){
						data[y_i*sizeX + x_i] = 2000;
					}
				}
			}
			 
			dicomImage.setupImage(sizeX, sizeY, z_i, (unsigned char*)data);

 
		}


		dicomImage.setImageNumber(ImageCount);

		std::string CurSOPInstanceUID;
	//	void CTstStoreSCU::genSOPInstanceUID()
		{
			char _char_buff[128];
			sprintf(_char_buff, ".%u", ImageCount);
			CurSOPInstanceUID = CurStudyInstanceUID + _char_buff;
		}
		dicomImage.setupSOPInstanceUID(CurSOPInstanceUID);

		dicomImage.setStudyDescription("test dicom coord");

		dicomImage.setSeriesDescription("dicom coord");
		//
		dicomImage.setModality(0);

		{
			char data_char_buff[64];
			struct tm cur_time;
			_getsystime(&cur_time);
			sprintf(data_char_buff, "%04d%02d%02d",
				cur_time.tm_year + 1900,
				cur_time.tm_mon + 1,
				cur_time.tm_mday
				);
			
			char time_char_buff[64];
			sprintf(time_char_buff, "%02d%02d%02d",
				cur_time.tm_hour,
				cur_time.tm_min,
				cur_time.tm_sec
				);
			dicomImage.setupCurDate(data_char_buff, time_char_buff);

		}

		dicomImage.prepareDICOM();

		char _output_file[256];
	//	sprintf(_output_file,"E:\\dev2\\DataServer\\src\\unitTest\\dcmTest\\DcmCoordData\\outputdicom\\dicom%04d.dcm", z_i);
		sprintf(_output_file, "E:\\temp\\outputdicom\\dicoms\\dicom%04d.dcm", z_i);
		dicomImage.saveDicom(_output_file);
		ImageCount++;
	}
	{
		char _rawFile[256];
		sprintf(_rawFile, "%s\\volbin_%d_%d_%d.raw", outputDir.c_str(), sizeX, sizeY, sizeZ);
		FILE *fp = fopen(_rawFile, "wb");

		for (int z_i = 0; z_i < sizeZ; z_i++){
			unsigned short *data = (unsigned short *)(DataGenPtr->getDataPtr(z_i));
			fwrite(data, 2, sizeX*sizeY, fp);
		}
		fclose(fp);
	}
	return 0;
}
 