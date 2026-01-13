// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"


void tstDcmDataFileRead();
void tstDcmDataFileWrite();
 
int _tmain(int argc, _TCHAR* argv[])
{

//  	tstDcmDataFileRead();
	tstDcmDataFileWrite();
 

	return 0;
}

#include "IDcmLib.h"

using namespace XTDcmLib;
void tstDcmDataFileRead()
{
 
	IDcmLib *dcmlib_instance = IDcmLib::createInstance();
//	DcmDataSet *dataset_instance = dcmlib_instance->createDcmDataSet();
//	dataset_instance->readFile("E:\\temp\\testdata_out.dcm");

 

	DcmXTDicomMessage *dcm_file_instance = dcmlib_instance->createDicomMessage();

	dcm_file_instance->setMaxReadLength(16);

//	dcm_file_instance->readFile("E:\\temp\\testdata_out.dcm");
	dcm_file_instance->readFile("C:\\temp\\org_pano.dcm");

//	dcm_file_instance->readFromDumpFile("E:\\temp\\testdata1.dcm.txt");
//	dcm_file_instance->readFromDumpFile("E:\\temp\\QR_Image.dcm.txt");


	DcmXTDataSet *dataset = dcm_file_instance->getDcmXTDataSet();

//	DcmXTComInterface *comInterface = (DcmXTComInterface *)dataset;
//	comInterface->Set_Value(0x00080022,"tttttttttttt");
//	comInterface->Set_Value(0x00280010,256);

	{
	char string_val[1024];
	 
 

 	dcm_file_instance->Get_Value(0x00200037,string_val,1024);
//	int str_len = string_val.size() ;
#if 0	 

	dcm_file_instance->Get_Value(0x00080020,string_val);

	dcm_file_instance->Get_Value(0x00020010,string_val);
#endif

 	dcm_file_instance->writeToDumpFile("test_dcm.txt");


	}

	dcmlib_instance->destroy();
 
	int x =0;
	printf("x%d \n",x);
	return ;
}

void tstDcmDataFileWrite()
{
 
	IDcmLib *dcmlib_instance = IDcmLib::createInstance();
 //	DcmDataSet *dataset_instance = dcmlib_instance->createDcmDataSet();
 //	dataset_instance->readFile("testdata.dcm");

 
	DcmXTUtil *dcmUtil = dcmlib_instance->getUtil();

	unsigned long iTag = 0x00080016;// UI    =SecondaryCaptureImageStorage          #    26,  1  SOPClassUID
	const char *tag_name = dcmUtil->getTagName(iTag);
	 

	dcmlib_instance->setupWriteBufferLen(128*1024);
 

	DcmXTDicomMessage *dcm_file_instance = dcmlib_instance->createDicomMessage();
	 
#if 0
	//dcm_file_instance->readFile("testdata.dcm");
	dcm_file_instance->readFile("testdata1.dcm");
#else
	dcm_file_instance->open();//openMessage("null");
 

	dcm_file_instance->Set_TransferSyntax(DcmXT_EXS_LittleEndianExplicit);
	{
	//(0002,0000) UL    196                                    #     4,  1  MetaElementGroupLength
		//(0002,0001) OB    00\01                                  #     2,  1  FileMetaInformationVersion
			dcm_file_instance->Set_Value(0x00020001,"00\\01");

		//(0002,0002) UI    =CTImageStorage                        #    26,  1  MediaStorageSOPClassUID
			dcm_file_instance->Set_Value(0x00020002,"1.2.840.10008.5.1.4.1.1.2");

		//(0002,0003) UI    [1.2.392.200183.123.456.200409021920.031.6.3] #    44,  1  MediaStorageSOPInstanceUID
			dcm_file_instance->Set_Value(0x00020003,"1.2.392.200183.123.456.200409021920.031.6.3");

		//(0002,0010) UI    =LittleEndianImplicit                  #    18,  1  TransferSyntaxUID
			dcm_file_instance->Set_Value(0x00020010,"LittleEndianImplicit");

		//(0002,0012) UI    [2.16.840.1.113669.2.1.1]              #    24,  1  ImplementationClassUID
			dcm_file_instance->Set_Value(0x00020012,"2.16.840.1.113669.2.1.1");

		//(0002,0013) SH    [MergeCOM3_340]                        #    14,  1  ImplementationVersionName
			dcm_file_instance->Set_Value(0x00020013,"MergeCOM3_340");

		//(0002,0016) AE    [AUTOVOX]                              #     8,  1  SourceApplicationEntityTitle
			dcm_file_instance->Set_Value(0x00020016,"AUTOVOX");

		//////////////////////////
		 
		//	(0008,0005) CS    (no value available)                   #     0,  0  SpecificCharacterSet

		//	(0008,0008) CS    [ORIGINAL\PRIMARY\AXIAL]               #    22,  3  ImageType
			dcm_file_instance->Set_Value(0x00080008,"ORIGINAL\\PRIMARY\\AXIAL");

		//	(0008,0014) UI    [1.2.392.200183.123.456]               #    22,  1  InstanceCreatorUID
			dcm_file_instance->Set_Value(0x00080014,"1.2.392.200183.123.456");

		//	(0008,0016) UI    =CTImageStorage                        #    26,  1  SOPClassUID
			dcm_file_instance->Set_Value(0x00080016,"1.2.840.10008.5.1.4.1.1.2");

		//	(0008,0018) UI    [1.2.392.200183.123.456.200409021920.031.6.3] #    44,  1  SOPInstanceUID
			dcm_file_instance->Set_Value(0x00080018,"1.2.392.200183.123.456.200409021920.031.6.3");

		//	(0008,0020) DA    [20040902]                             #     8,  1  StudyDate
			dcm_file_instance->Set_Value(0x00080020,"20040902");



		//	(0008,0021) DA    [20040902]                             #     8,  1  SeriesDate
			dcm_file_instance->Set_Value(0x00080021,"20040902");

		//	(0008,0023) DA    [20040902]                             #     8,  1  ImageDate
			dcm_file_instance->Set_Value(0x00080023,"20040902");

		//	(0008,0030) TM    [191945.484]                           #    10,  1  StudyTime
			dcm_file_instance->Set_Value(0x00080030,"191945.484");

		//	(0008,0031) TM    [191945.484]                           #    10,  1  SeriesTime
			dcm_file_instance->Set_Value(0x00080031,"191945.484");

		//	(0008,0033) TM    [1920]                                 #     4,  1  ImageTime
			dcm_file_instance->Set_Value(0x00080033,"1920");

		//	(0008,0050) SH    [1]                                    #     2,  1  AccessionNumber
			dcm_file_instance->Set_Value(0x00080050,"1");

		//	(0008,0060) CS    [CT]                                   #     2,  1  Modality
			dcm_file_instance->Set_Value(0x00080060,"CT");

		//	(0008,0070) LO    [YoshidaDental]                        #    14,  1  Manufacturer
			dcm_file_instance->Set_Value(0x00080070,"Manufacturer");

		//	(0008,0080) LO    (no value available)                   #     0,  0  InstitutionName
			dcm_file_instance->Set_Value(0x00080080,"FineCube");

		//	(0008,0090) PN    [ReferringPhysiciansName]              #    24,  1  ReferringPhysiciansName

		//	(0008,1010) SH    [001]                                  #     4,  1  StationName
		//	(0008,1060) PN    [NameOfPhysiciansReadingStudy]         #    28,  1  NameOfPhysiciansReadingStudy

		//	(0008,1070) PN    [.$B$?$+$R$m.(B]                       #    14,  1  OperatorsName
			dcm_file_instance->Set_Value(0x00081070,"オペレター名前");

		//	(0008,1090) LO    [FineCube]                             #     8,  1  ManufacturerModelName
			dcm_file_instance->Set_Value(0x00081090,"FineCube");

		//	(0008,1010) SH    [CT-01]                                #     6,  1  StationName
			dcm_file_instance->Set_Value(0x00081010,"CT-01");

		 
		//(0008,1030) LO    [.$B=QA0.(B]                           #    10,  1  StudyDescription
		//(0008,103e) LO    [.$B4pK\.(BCT.$B2hA|.(B]               #    22,  2  SeriesDescription
			dcm_file_instance->Set_Value(0x00081030,"Study Description日本語");

			dcm_file_instance->Set_Value(0x0008103E,"Series Description日本語");



		//	(0010,0010) PN    [DentalCT^TestData]                    #    18,  1  PatientName
			dcm_file_instance->Set_Value(0x00100010,"XTDebugName");

		//	(0010,0020) LO    [20040902]                             #     8,  1  PatientID
			dcm_file_instance->Set_Value(0x00100020,"22200001");

		//	(0010,0030) DA    [19840902]                             #     8,  1  PatientBirthDate
			dcm_file_instance->Set_Value(0x00100030,"19840902");

		//	(0010,0040) CS    [O]                                    #     2,  1  PatientSex
			dcm_file_instance->Set_Value(0x00100040,"F");
			dcm_file_instance->Set_Value(0x00100040,"M");

		//	(0010,1010) AS    (no value available)                   #     0,  0  PatientAge
		//	(0018,0050) DS    [0.14]                                 #     4,  1  SliceThickness
			dcm_file_instance->Set_Value(0x00180050,0.14);

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
			dcm_file_instance->SetPrivateValue("TR",0x0019,0x0010);
			 
		//	(0019,1010) ??    35\31\33\20                            #     4,  1  Unknown Tag & Data
			dcm_file_instance->Set_Value(0x00191010,"35\\31\\33\\20");

		//	(0019,1020) ??    35\31\32\20                            #     4,  1  Unknown Tag & Data
			dcm_file_instance->Set_Value(0x00191020,"35\\31\\32");

		//	(0019,1063) ??    32\39                                  #     2,  1  Unknown Tag & Data
			dcm_file_instance->Set_Value(0x00191063,"32\\39");

		#endif
		//for TROPHY
	{
		 
		if(!dcm_file_instance->Add_PrivateTag(0x0009,0x0011, LO,"Tropy")){ 
			return  ;
		}

		if(!dcm_file_instance->Set_Value( 0x00090011, "TROPHY" )){ 
			return  ;
		}

		///
		if(!dcm_file_instance->Add_PrivateTag(0x0009,0x1105, DS,"Tropy1")){ 
			return  ;
		}
		if(!dcm_file_instance->Set_Value( 0x00091105, "1.8.7.0",false,"Tropy1" )){ 
			return  ;
		}
		//
		if(!dcm_file_instance->Add_PrivateTag(0x0009,0x1111, US,"Tropy2")){ 
			return  ;
		}
		if(!dcm_file_instance->Set_Value( 0x00091111, "8",false,"Tropy2" )){ 
			return  ;
		}

	}
	{

 
		if(!dcm_file_instance->Begin_Sequence( 0x0018a001 )){ 
			return  ;
		}
 
	 	if(!dcm_file_instance->Set_Value( 0x00080070, "Trophy Radiologie")){ 
	 		return  ;
	 	}
		//
		{
			//sub-sequence
			if(!dcm_file_instance->Begin_Sequence( 0x0040a170 )){ 
				return  ;
			}
			//...
			if(!dcm_file_instance->Set_Value( 0x00080100, "109101")){ 
	 			return  ;
	 		}
			if(!dcm_file_instance->Set_Value( 0x00080102, "DCM")){ 
	 			return  ;
	 		}
			if(!dcm_file_instance->Set_Value( 0x00080100, "109101")){ 
	 			return  ;
	 		}
			if(!dcm_file_instance->Set_Value( 0x00080104, "Acquisition Equipment")){ 
	 			return  ;
	 		}
			 
			if(!dcm_file_instance->End_Sequence( 0x0040a170 )){ 
				return  ;
			}
		}

		if(!dcm_file_instance->Set_Value( 0x0018a003, "Acquisition Software Module")){ 
	 		return  ;
	 	}
		 

		//....
		if(!dcm_file_instance->End_Sequence( 0x0018a001 )){ 
			return  ;
		}

	}
		//	(0020,000d) UI    [1.2.392.200183.123.456.20040902191927.1] #    40,  1  StudyInstanceUID
			dcm_file_instance->Set_Value(0x0020000d,"1.2.392.200183.123.456.20040902191927.1");

		//	(0020,000e) UI    [1.2.392.200183.123.456.20040902191927.301.2] #    44,  1  SeriesInstanceUID
			dcm_file_instance->Set_Value(0x0020000e,"1.2.392.200183.123.456.20040902191927.301.2");

		//	(0020,0010) SH    [0409021800]                           #    10,  1  StudyID
			dcm_file_instance->Set_Value(0x00200010,"0409021800");

		//	(0020,0011) IS    [301]                                  #     4,  1  SeriesNumber
			dcm_file_instance->Set_Value(0x00200011,"301");

		//	(0020,0012) IS    [1]                                    #     2,  1  AcquisitionNumber
			dcm_file_instance->Set_Value(0x00200012,"1");

		//	(0020,0013) IS    [6]                                    #     2,  1  ImageNumber
			dcm_file_instance->Set_Value(0x00200013,"1");

		//	(0020,0032) DS    [-35.8400\-35.8400\35.1400]            #    26,  3  ImagePositionPatient
			dcm_file_instance->Set_Value(0x00200032,"-35.8400\\-35.8400\\35.1400");

		//	(0020,0037) DS    [1.000000\0.000000\0.000000\0.000000\1.000000\0.000000] #    54,  6  ImageOrientationPatient
			dcm_file_instance->Set_Value(0x00200037,"1.000000\\0.000000\\0.000000\\0.000000\\1.000000\\0.000000");

		//	(0020,0052) UI    [1.2.392.200183.123.456.20040902191927.4] #    40,  1  FrameOfReferenceUID
			dcm_file_instance->Set_Value(0x00200052,"1.2.392.200183.123.456.20040902191927.4");

		//	(0020,1040) LO    [x]                                    #     2,  1  PositionReferenceIndicator
			dcm_file_instance->Set_Value(0x00201040,"x");

		//	(0020,1041) DS    [35.14]                                #     6,  1  SliceLocation
			dcm_file_instance->Set_Value(0x00201041,"35.14");

		//	(0020,4000) LT    (no value available)                   #     0,  1  ImageComments

		//	(0028,0002) US    1                                      #     2,  1  SamplesPerPixel
			dcm_file_instance->Set_Value(0x00280002,1);

		//	(0028,0004) CS    [MONOCHROME2]                          #    12,  1  PhotometricInterpretation
			dcm_file_instance->Set_Value(0x00280004,"MONOCHROME2");

		//	(0028,0010) US    512                                    #     2,  1  Rows
			dcm_file_instance->Set_Value(0x00280010,512);

		//	(0028,0011) US    512                                    #     2,  1  Columns
			dcm_file_instance->Set_Value(0x00280011,512);

		//	(0028,0030) DS    [0.140\0.140]                          #    12,  2  PixelSpacing
			dcm_file_instance->Set_Value(0x00280030,"0.140\\0.140");

		//	(0028,0100) US    16                                     #     2,  1  BitsAllocated
			dcm_file_instance->Set_Value(0x00280100,16);

		//	(0028,0101) US    16                                     #     2,  1  BitsStored
			dcm_file_instance->Set_Value(0x00280101,16);

		//	(0028,0102) US    15                                     #     2,  1  HighBit
			dcm_file_instance->Set_Value(0x00280102,15);

		//	(0028,0103) US    1                                      #     2,  1  PixelRepresentation
			dcm_file_instance->Set_Value(0x00280103,0);

		//	(0028,1050) DS    [1000]                                 #     4,  1  WindowCenter
			dcm_file_instance->Set_Value(0x00281050,1000);

		//	(0028,1051) DS    [2000]                                 #     4,  1  WindowWidth
			dcm_file_instance->Set_Value(0x00281051,2000);

		//	(0028,1052) DS    [0]                                    #     2,  1  RescaleIntercept
			dcm_file_instance->Set_Value(0x00281052,0);

		//	(0028,1053) DS    [1]                                    #     2,  1  RescaleSlope
			dcm_file_instance->Set_Value(0x00281053,1);
	}

#endif

	dcm_file_instance->writeToDumpFile("test_dcm.txt");
	dcm_file_instance->writeFile("test_dcm.dcm");

}

 