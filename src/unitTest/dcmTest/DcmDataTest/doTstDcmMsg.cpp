// testFxDcmData.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

#ifdef USE_NEW_LIB
#include "IDcmLibApi.h"
#include "PxDicomMessage.h"
#include "PxDicomImage.h"
#define DICOM_MSG CPxDicomMessage
#define DICOM_IMG CPxDicomImage
using namespace XTDcmLib;
 
#else
#include "VLIDicomImage.h" 
#include "rtvMergeToolKit.h "
 
#define DICOM_MSG VLIDicomMessage
#define DICOM_IMG VLIDicomImage
#endif
 
#include "AqCore/TRPlatform.h"

#define MyRootUID "1.2.392.200036.9163"   //PreXion DICOM root UID
std::string genNewUID(std::string old_uid)
{
 
	int pos = old_uid.find_last_of(".");
	std::string prex_str = old_uid.substr(0,pos);
	
	std::string time_stamp = TRPlatform::YYYYMMDDHHMMSSUUUTimeStamp();
	int rest_size = 64-prex_str.size();
	int start_pos = time_stamp.size() - rest_size+1;
	if(start_pos >1){
		time_stamp = time_stamp.substr(start_pos);
	}
	std::string new_instance_uid = prex_str+"."+time_stamp;

	return new_instance_uid;
}

int doTstDcmMsg()
{
	TRDICOMUtil::InitialDICOM("");
	 
 	DICOM_MSG *newMsg = new DICOM_MSG;

 
// 	newMsg->Load("C:\\temp\\pano_dcm\\K8000.dat",false/*headerOnly*/);
// 	newMsg->Load("C:\\temp\\pano_dcm\\Panoura15.dcm",false/*headerOnly*/);

 //	newMsg->Load("dbg_dicom.dcm");

 	newMsg->Load("C:\\temp\\cephalo\\2012_04sample\\LA_HeadPhamtom.dcm");

//	newMsg->Load("test_test1.dcm");
	{
		DICOM_IMG *newDicom = new DICOM_IMG(newMsg->GetID());
		newDicom->SetValue(0x00080060,"CT");//Modality
		//(0008,0016) UI    =SecondaryCaptureImageStorage          #    26,  1  SOPClassUID
	 	newDicom->SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.7");
		//"1.2.840.10008.5.1.4.1.1.2", "CTImageStorage", "STANDARD_CT" },
	//	newDicom->SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.2");

		newDicom->SetTransferSyntax(EXPLICIT_LITTLE_ENDIAN);
		char str_buff[512];
		//新しいUID生成

		sprintf(str_buff,"%s.124",MyRootUID);
		//SOPInstanceUID
		std::string new_sop_instance_uid = genNewUID(str_buff);
		newDicom->SetValue(0x00080018,new_sop_instance_uid.c_str());//SOPInstanceUID
	 
		//StudyInstanceUID
		std::string new_study_instance_uid = genNewUID(str_buff);
		newDicom->SetValue(0x0020000d,new_study_instance_uid.c_str());//SeriesInstanceUID

		//SeriesInstanceUID
		std::string new_series_instance_uid = genNewUID(str_buff);
		newDicom->SetValue(0x0020000e,new_series_instance_uid.c_str());//SeriesInstanceUID

		 
		//PatientName
		newDicom->SetValue(0x00100010,"test1^cephalo"); 
		//PatientID
		newDicom->SetValue(0x00100020,"12222"); 


		if(0){
			int sizeX = newDicom->GetNumberOfRows();
			int sizeY = newDicom->GetNumberOfColumns();
			unsigned char *pixel_ptr = newDicom->GetImagePixels();
			FILE *fp = fopen("test_img.bin","wb");
			fwrite(pixel_ptr,2,sizeX*sizeY,fp);
			fclose(fp);
		}

		newDicom->Save("test1.dcm");

		delete newDicom;

		return 0;
	}

	 
	{
		DICOM_IMG *newDicom = new DICOM_IMG(newMsg->GetID());
		newDicom->GetImagePixels();

	}
// 	newMsg->SetValue(0x00020001,"00\\01");

	newMsg->SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.2");
	newMsg->SetValue(0x00080060,"CT");
	newMsg->SetValue(0x00200011,10);

	newMsg->SetValue(0x00280030,"0.1560\\0.1560"); 
//	newMsg->SetValue(0x00080060,"CS");

 //   MC_Set_Value_To_NULL(newMsg->GetID(),0x00080020);
	newMsg->Save("test1.dcm");


	delete newMsg;
	return 0;
}

int doTstReadDcm()
{
 
	TRDICOMUtil::InitialDICOM("");
	DICOM_MSG testDcm;

 	DICOM_IMG *newDicom = new DICOM_IMG;

	if(0){
//		newDicom->Load("E:\\temp\\diff_reslice\\1471629.org\\00256DCM");
		newDicom->Load("E:\\temp\\diff_reslice\\52668322.reslice\\00012DCM");
		int bits_allocated = newDicom->GetBitsAllocated();
		int bits_storeded = newDicom->GetBitsStored();
		int bits_highbit = newDicom->GetHighBit();
		//
		double RescaleIntercept = newDicom->GetRescaleIntercept();
		double RescaleSlope		= newDicom->GetRescaleSlope();
		//
		int sizeX = newDicom->GetNumberOfRows();
		int sizeY = newDicom->GetNumberOfColumns();
		unsigned char *pixel_ptr = newDicom->GetImagePixels();
		FILE *fp = fopen("test_img.bin","wb");
		fwrite(pixel_ptr,2,sizeX*sizeY,fp);
		fclose(fp);
	}
	newDicom->Load("test.dcm");


	int int_temp;
	unsigned int uint_temp;
	short short_temp;
	unsigned short unshort_temp;
	long long_temp;
	unsigned long unlong_temp;
	float f_temp;
	double d_temp;

	newDicom->GetValue(0x00280010,&int_temp);
//	newDicom->GetValue(0x00280010,&uint_temp);
	//
//	newDicom->GetValue(0x00280010,&short_temp);
//	newDicom->GetValue(0x00280010,&unshort_temp);
	//
//	newDicom->GetValue(0x00280010,&long_temp);
//	newDicom->GetValue(0x00280010,&unlong_temp);
	//
	newDicom->GetValue(0x00280010,&f_temp);
//	newDicom->GetValue(0x00280010,&d_temp);

	{
		MC_STATUS status;
		int msgID = newDicom->GetID();
		status = MC_Get_Value_To_Int(msgID,0x00280010,&int_temp);
 	 	status = MC_Get_Value_To_UInt(msgID,0x00280010,&uint_temp);
		//
 	 	status = MC_Get_Value_To_ShortInt(msgID,0x00280010,&short_temp);
 	 	status = MC_Get_Value_To_UShortInt(msgID,0x00280010,&unshort_temp);
 		//
 	 	status = MC_Get_Value_To_LongInt(msgID,0x00280010,&long_temp);
 	 	status = MC_Get_Value_To_ULongInt(msgID,0x00280010,&unlong_temp);
		//
 	 	status = MC_Get_Value_To_Float(msgID,0x00280010,&f_temp);
 	 	status = MC_Get_Value_To_Double(msgID,0x00280010,&d_temp);
	}
	{
		
		int sizeX = 128;
		int sizeY = 256;
		//(0028,0010) US 616                                      #   2, 1 Rows
		//(0028,0011) US 1232                                     #   2, 1 Columns
		newDicom->SetValue(0x00280010,sizeX);
		newDicom->SetValue(0x00280011,sizeY);

		newDicom->SetBitsAllocated(16);
		newDicom->SetBitsStored(12);
		newDicom->SetBytesPerPixel(1);
		unsigned char *pixel_data_ptr = new unsigned char[sizeX*sizeY*2];
		newDicom->SetPixelData(pixel_data_ptr,sizeX*sizeY*2,IMPLICIT_LITTLE_ENDIAN);
	}

	

	newDicom->Save("test1.dcm");
	delete newDicom;
 
	return 0;
}

int setDcmMsg2Pano()
{
	TRDICOMUtil::InitialDICOM("");
	 
 	DICOM_MSG *newMsg = new DICOM_MSG;

 
 	newMsg->Load("C:\\temp\\pano_dcm\\K8000.dat",false/*headerOnly*/);
// 	newMsg->Load("C:\\temp\\pano_dcm\\Panoura15.dcm",false/*headerOnly*/);

 //	newMsg->Load("dbg_dicom.dcm");

	 
	{
		DICOM_IMG *newDicom = new DICOM_IMG(newMsg->GetID());
		newDicom->GetImagePixels();

	}
// 	newMsg->SetValue(0x00020001,"00\\01");

	newMsg->SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.2");
	newMsg->SetValue(0x00080060,"CT");
	newMsg->SetValue(0x00200011,10);

	newMsg->SetValue(0x00280030,"0.1560\\0.1560"); 
//	newMsg->SetValue(0x00080060,"CS");

 //   MC_Set_Value_To_NULL(newMsg->GetID(),0x00080020);
	newMsg->Save("test1.dcm");


	delete newMsg;
	return 0;
}


int setDcmMsg2Scale(bool useScale=true)
{
	TRDICOMUtil::InitialDICOM("");
	 
 	DICOM_MSG *newMsg = new DICOM_MSG;

 
// 	newMsg->Load("C:\\temp\\org_pano.dcm",false/*headerOnly*/);
// 	newMsg->Load("C:\\temp\\pano_dcm\\Panoura15.dcm",false/*headerOnly*/);

  	
	int PixelRepresentation=0;
	newMsg->GetValue(0x00280103,&PixelRepresentation);
	 
	
	 
	
	if((PixelRepresentation ==1) && useScale){
		DICOM_IMG *newDicom = new DICOM_IMG(newMsg->GetID());

		char str_buff[512];
		//新しいUID生成
		//SOPInstanceUID
		newDicom->GetValue(0x00080018,str_buff,512);//SOPInstanceUID
		std::string new_sop_instance_uid = genNewUID(str_buff);
		newDicom->SetValue(0x00080018,new_sop_instance_uid.c_str());//SOPInstanceUID
	 
		//SeriesInstanceUID
		newDicom->GetValue(0x0020000e,str_buff,512);//SeriesInstanceUID
		std::string new_series_instance_uid = genNewUID(str_buff);
		newDicom->SetValue(0x0020000e,new_series_instance_uid.c_str());//SeriesInstanceUID



		short *pixel_data = (short*)(newDicom->GetImagePixels());
		 

		int sizeX = newDicom->GetNumberOfColumns();
		int sizeY = newDicom->GetNumberOfRows();
		int sizeZ = newDicom->GetNumberOfFrames();

		int allSize = sizeX*sizeY*sizeZ;
		for(int i=0;i<allSize;i++){
			pixel_data[i] += 1024;
		}
		 
		//
		newMsg->SetValue(0x00280103,0); //PixelRepresentation
		newMsg->SetValue(0x00281052,-1024); //RescaleIntercept
		newMsg->SetValue(0x00281053,1); //RescaleSlope


		newMsg->SetValue(0x0008103e,"RescaleIntercept-1024") ;//SeriesDescription


		newDicom->Save("test1.dcm");

		delete newDicom;
	}else{

	 
// 	newMsg->SetValue(0x00020001,"00\\01");

#if 0
	newMsg->SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.2");
	newMsg->SetValue(0x00080060,"CT");
	newMsg->SetValue(0x00200011,10);

	newMsg->SetValue(0x00280030,"0.1560\\0.1560"); 
#endif
//	newMsg->SetValue(0x00080060,"CS");

 //   MC_Set_Value_To_NULL(newMsg->GetID(),0x00080020);
		newMsg->Save("test1.dcm");
	}


	delete newMsg;
	return 0;
}

 int doTstWriteDcm()
 {
	 TRDICOMUtil::InitialDICOM("");

	 CPxDicomImage DicomImage ;

	 DicomImage.CheckValid();

	 {
		 //////////////////////////

		//(0002,0000) UL    196                                    #     4,  1  MetaElementGroupLength
		//(0002,0001) OB    00\01                                  #     2,  1  FileMetaInformationVersion
			DicomImage.SetValue(0x00020001,"00\\01");

		//(0002,0002) UI    =CTImageStorage                        #    26,  1  MediaStorageSOPClassUID
			DicomImage.SetValue(0x00020002,"1.2.840.10008.5.1.4.1.1.2");

		//(0002,0003) UI    [1.2.392.200183.123.456.200409021920.031.6.3] #    44,  1  MediaStorageSOPInstanceUID
			DicomImage.SetValue(0x00020003,"1.2.392.200183.123.456.200409021920.031.6.3");

		//(0002,0010) UI    =LittleEndianImplicit                  #    18,  1  TransferSyntaxUID
			DicomImage.SetValue(0x00020010,"LittleEndianImplicit");

		//(0002,0012) UI    [2.16.840.1.113669.2.1.1]              #    24,  1  ImplementationClassUID
			DicomImage.SetValue(0x00020012,"2.16.840.1.113669.2.1.1");

		//(0002,0013) SH    [MergeCOM3_340]                        #    14,  1  ImplementationVersionName
			DicomImage.SetValue(0x00020013,"MergeCOM3_340");

		//(0002,0016) AE    [AUTOVOX]                              #     8,  1  SourceApplicationEntityTitle
			DicomImage.SetValue(0x00020016,"AUTOVOX");

		//////////////////////////
		 
		//	(0008,0005) CS    (no value available)                   #     0,  0  SpecificCharacterSet

		//	(0008,0008) CS    [ORIGINAL\PRIMARY\AXIAL]               #    22,  3  ImageType
			DicomImage.SetValue(0x00080008,"ORIGINAL\\PRIMARY\\AXIAL");

		//	(0008,0014) UI    [1.2.392.200183.123.456]               #    22,  1  InstanceCreatorUID
			DicomImage.SetValue(0x00080014,"1.2.392.200183.123.456");

		//	(0008,0016) UI    =CTImageStorage                        #    26,  1  SOPClassUID
			DicomImage.SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.2");

		//	(0008,0018) UI    [1.2.392.200183.123.456.200409021920.031.6.3] #    44,  1  SOPInstanceUID
			DicomImage.SetValue(0x00080018,"1.2.392.200183.123.456.200409021920.031.6.3");

		//	(0008,0020) DA    [20040902]                             #     8,  1  StudyDate
			DicomImage.SetValue(0x00080020,"20040902");



		//	(0008,0021) DA    [20040902]                             #     8,  1  SeriesDate
			DicomImage.SetValue(0x00080021,"20040902");

		//	(0008,0023) DA    [20040902]                             #     8,  1  ImageDate
			DicomImage.SetValue(0x00080023,"20040902");

		//	(0008,0030) TM    [191945.484]                           #    10,  1  StudyTime
			DicomImage.SetValue(0x00080030,"191945.484");

		//	(0008,0031) TM    [191945.484]                           #    10,  1  SeriesTime
			DicomImage.SetValue(0x00080031,"191945.484");

		//	(0008,0033) TM    [1920]                                 #     4,  1  ImageTime
			DicomImage.SetValue(0x00080033,"1920");

		//	(0008,0050) SH    [1]                                    #     2,  1  AccessionNumber
			DicomImage.SetValue(0x00080050,"1");

		//	(0008,0060) CS    [CT]                                   #     2,  1  Modality
			DicomImage.SetValue(0x00080060,"CT");

		//	(0008,0070) LO    [YoshidaDental]                        #    14,  1  Manufacturer
			DicomImage.SetValue(0x00080070,"Manufacturer");

		//	(0008,0080) LO    (no value available)                   #     0,  0  InstitutionName
			DicomImage.SetValue(0x00080080,"FineCube");

		//	(0008,0090) PN    [ReferringPhysiciansName]              #    24,  1  ReferringPhysiciansName

		//	(0008,1010) SH    [001]                                  #     4,  1  StationName
		//	(0008,1060) PN    [NameOfPhysiciansReadingStudy]         #    28,  1  NameOfPhysiciansReadingStudy

		//	(0008,1070) PN    [.$B$?$+$R$m.(B]                       #    14,  1  OperatorsName
			DicomImage.SetValue(0x00081070,"オペレター名前");

		//	(0008,1090) LO    [FineCube]                             #     8,  1  ManufacturerModelName
			DicomImage.SetValue(0x00081090,"FineCube");

		//	(0008,1010) SH    [CT-01]                                #     6,  1  StationName
			DicomImage.SetValue(0x00081010,"CT-01");

		 
		//(0008,1030) LO    [.$B=QA0.(B]                           #    10,  1  StudyDescription
		//(0008,103e) LO    [.$B4pK\.(BCT.$B2hA|.(B]               #    22,  2  SeriesDescription
			DicomImage.SetValue(0x00081030,"Study Description日本語");

			DicomImage.SetValue(0x0008103E,"Series Description日本語");



		//	(0010,0010) PN    [DentalCT^TestData]                    #    18,  1  PatientName
			DicomImage.SetValue(0x00100010,"XTDebugName");

		//	(0010,0020) LO    [20040902]                             #     8,  1  PatientID
			DicomImage.SetValue(0x00100020,"22200001");

		//	(0010,0030) DA    [19840902]                             #     8,  1  PatientBirthDate
			DicomImage.SetValue(0x00100030,"19840902");

		//	(0010,0040) CS    [O]                                    #     2,  1  PatientSex
			DicomImage.SetValue(0x00100040,"F");
			DicomImage.SetValue(0x00100040,"M");

		//	(0010,1010) AS    (no value available)                   #     0,  0  PatientAge
		//	(0018,0050) DS    [0.14]                                 #     4,  1  SliceThickness
			DicomImage.SetValue(0x00180050,0.14);

		//	(0018,0060) DS    (no value available)                   #     0,  0  KVP
		//	(0018,0090) DS    (no value available)                   #     0,  0  DataCollectionDiameter
		//	(0018,1100) DS    (no value available)                   #     0,  0  ReconstructionDiameter
		//	(0018,1110) DS    (no value available)                   #     0,  0  DistanceSourceToDetector
		//	(0018,1111) DS    (no value available)                   #     0,  0  DistanceSourceToPatient
		//	(0018,1140) CS    (no value available)                   #     0,  0  RotationDirection
		//	(0018,1150) IS    (no value available)                   #     0,  0  ExposureTime
		//	(0018,1151) IS    (no value available)                   #     0,  0  XRayTubeCurrent
		//	(0018,1210) SH    (no value available)                   #     0,  0  ConvolutionKernel

		#if 0
		//	(0019,0010) LO    [TR]                                   #     2,  1  PrivateCreator
			DicomImage.SetPrivateValue("TR",0x0019,0x0010);
			 
		//	(0019,1010) ??    35\31\33\20                            #     4,  1  Unknown Tag & Data
			DicomImage.SetValue(0x00191010,"35\\31\\33\\20");

		//	(0019,1020) ??    35\31\32\20                            #     4,  1  Unknown Tag & Data
			DicomImage.SetValue(0x00191020,"35\\31\\32");

		//	(0019,1063) ??    32\39                                  #     2,  1  Unknown Tag & Data
			DicomImage.SetValue(0x00191063,"32\\39");

		#endif
		//	(0020,000d) UI    [1.2.392.200183.123.456.20040902191927.1] #    40,  1  StudyInstanceUID
			DicomImage.SetValue(0x0020000d,"1.2.392.200183.123.456.20040902191927.1");

		//	(0020,000e) UI    [1.2.392.200183.123.456.20040902191927.301.2] #    44,  1  SeriesInstanceUID
			DicomImage.SetValue(0x0020000e,"1.2.392.200183.123.456.20040902191927.301.2");

		//	(0020,0010) SH    [0409021800]                           #    10,  1  StudyID
			DicomImage.SetValue(0x00200010,"0409021800");

		//	(0020,0011) IS    [301]                                  #     4,  1  SeriesNumber
			DicomImage.SetValue(0x00200011,"301");

		//	(0020,0012) IS    [1]                                    #     2,  1  AcquisitionNumber
			DicomImage.SetValue(0x00200012,"1");

		//	(0020,0013) IS    [6]                                    #     2,  1  ImageNumber
			DicomImage.SetValue(0x00200013,"1");

		//	(0020,0032) DS    [-35.8400\-35.8400\35.1400]            #    26,  3  ImagePositionPatient
			DicomImage.SetValue(0x00200032,"-35.8400\\-35.8400\\35.1400");

		//	(0020,0037) DS    [1.000000\0.000000\0.000000\0.000000\1.000000\0.000000] #    54,  6  ImageOrientationPatient
			DicomImage.SetValue(0x00200037,"1.000000\\0.000000\\0.000000\\0.000000\\1.000000\\0.000000");

		//	(0020,0052) UI    [1.2.392.200183.123.456.20040902191927.4] #    40,  1  FrameOfReferenceUID
			DicomImage.SetValue(0x00200052,"1.2.392.200183.123.456.20040902191927.4");

		//	(0020,1040) LO    [x]                                    #     2,  1  PositionReferenceIndicator
			DicomImage.SetValue(0x00201040,"x");

		//	(0020,1041) DS    [35.14]                                #     6,  1  SliceLocation
			DicomImage.SetValue(0x00201041,"35.14");

		//	(0020,4000) LT    (no value available)                   #     0,  1  ImageComments

		//	(0028,0002) US    1                                      #     2,  1  SamplesPerPixel
			DicomImage.SetValue(0x00280002,1);

		//	(0028,0004) CS    [MONOCHROME2]                          #    12,  1  PhotometricInterpretation
			DicomImage.SetValue(0x00280004,"MONOCHROME2");

		//	(0028,0010) US    512                                    #     2,  1  Rows
			DicomImage.SetValue(0x00280010,512);

		//	(0028,0011) US    512                                    #     2,  1  Columns
			DicomImage.SetValue(0x00280011,512);

		//	(0028,0030) DS    [0.140\0.140]                          #    12,  2  PixelSpacing
			DicomImage.SetValue(0x00280030,"0.140\\0.140");

		//	(0028,0100) US    16                                     #     2,  1  BitsAllocated
			DicomImage.SetValue(0x00280100,16);

		//	(0028,0101) US    16                                     #     2,  1  BitsStored
			DicomImage.SetValue(0x00280101,16);

		//	(0028,0102) US    15                                     #     2,  1  HighBit
			DicomImage.SetValue(0x00280102,15);

		//	(0028,0103) US    1                                      #     2,  1  PixelRepresentation
			DicomImage.SetValue(0x00280103,0);

		//	(0028,1050) DS    [1000]                                 #     4,  1  WindowCenter
			DicomImage.SetValue(0x00281050,1000);

		//	(0028,1051) DS    [2000]                                 #     4,  1  WindowWidth
			DicomImage.SetValue(0x00281051,2000);

		//	(0028,1052) DS    [0]                                    #     2,  1  RescaleIntercept
			DicomImage.SetValue(0x00281052,0);

		//	(0028,1053) DS    [1]                                    #     2,  1  RescaleSlope
			DicomImage.SetValue(0x00281053,1);
	}


	DicomImage.Save("testNewDcm.dcm");
	 return 0;
 }


 void doMakeMultiFrame()
 {
	 TRDICOMUtil::InitialDICOM("");

	 CPxDicomImage DicomImage ;

	 DicomImage.CheckValid();

	 std::string dicom_dir = "K:\\temp\\tttt\\";
	 std::string dicom_file = dicom_dir + "01234567_2013_04_21_CT_Image001.dcm";
	 DicomImage.Load(dicom_file.c_str());


	 int sizeX = DicomImage.GetNumberOfColumns();
	 int sizeY = DicomImage.GetNumberOfRows();
	 int sizeZ  = 3;
	 unsigned char *pixel_data = new unsigned char[sizeX*sizeY*sizeZ*3];

	 for(int intZ=0;intZ<sizeZ;intZ++){
		 int R_flag = (intZ%3 ==0)? 1:0;
		 int G_flag = ((intZ%3-1) ==0)? 1:0;
		 int B_flag = ((intZ%3-2) ==0)? 1:0;
		unsigned char *frame_data_ptr = pixel_data + sizeX*sizeY*3*intZ;
		 for(int intY=0;intY<sizeY;intY++){
			 for(int intX=0;intX<sizeX;intX++){
				 int xx_index = 3*(intY*sizeX + intX);
				 
				 frame_data_ptr[ xx_index + 0] = intX%255*R_flag;
				 frame_data_ptr[ xx_index + 1] = intX%255*G_flag;
				 frame_data_ptr[ xx_index + 2] = intX%255*B_flag;
			 }
		 }
//		 DicomImage.AddFrame(frame_data_ptr);
	 }
	 DicomImage.SetValue(0x00280008,sizeZ);
	DicomImage.SetPixelData(pixel_data,sizeX*sizeY*sizeZ*3,IMPLICIT_LITTLE_ENDIAN);

 

	CPxDicomMessage  write_DicomImage(DicomImage.GetID());
	write_DicomImage.Save("tttt.dcm");

 }