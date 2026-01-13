// TstCPxDicomImage.cpp: CTstVLIDicomImage クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <ctime>

#include "TstVLIDicomImage.h"

#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#include "IDcmLibApi.h "
#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
static char _str_buff[1024];

CTstVLIDicomImage::CTstVLIDicomImage()
{
	m_DicomImage = 0;

	m_ImageNumber = 1;

	m_SeriesInstanceID = "1.2.392.200183.123.456.200409021920.031.6";
	m_InstanceCreatorUID = "1.2.392.200183.123.456";

	m_pitchX = 0.25;
	m_pitchY = 0.25;
	m_pitchZ = 0.25;

	m_useRescale = false;

	m_BitsAllocated = 16;
	m_BitsStored = 16;
	m_HighBit    = 15;

	m_Modality = 0;
//
	 m_SeriesDescription[0] = 0;
	 m_StudyDescription[0] = 0;

	 m_ReferringPhysician[0] = 0;

 
}

CTstVLIDicomImage::~CTstVLIDicomImage()
{
	if(m_DicomImage) delete m_DicomImage;

}

bool CTstVLIDicomImage::loadDicom(const char *filename,bool bHeaderOnly)
{
#ifdef USE_NEW_LIB
 CPxDicomImage dicom_loader;
#else
  VLIDicomImage dicom_loader;
#endif
	
  dicom_loader.HandoverID();
	if(kNormalCompletion != dicom_loader.Load(filename,bHeaderOnly) )
	{
		return false;
	}
 

	if(m_DicomImage) delete m_DicomImage;
#ifdef USE_NEW_LIB
	m_DicomImage = new CPxDicomImage;
#else
	m_DicomImage = new VLIDicomImage;
 
#endif

	
  	m_DicomImage->CheckValid();

  //	m_DicomImage->Load(filename,bHeaderOnly);

    setupDefault();

	int ImageNo =0;
#ifdef USE_NEW_LIB
	PxDicomStatus GetValue(unsigned long iTag, int* oValue);
#else
	VLIDicomStatus GetValue(unsigned long iTag, int* oValue);
#endif
	if(kNormalCompletion !=dicom_loader.GetValue(0x00200013,&ImageNo))
	{
		ImageNo = 0;
	}

	setImageNumber(ImageNo);

	unsigned short sizeY = dicom_loader.GetNumberOfRows();
     
	unsigned short sizeX = dicom_loader.GetNumberOfColumns();

    unsigned short NumberOfFrames  = dicom_loader.GetNumberOfFrames();
   
	int bufSize = dicom_loader.GetSamplesPerPixel()*(dicom_loader.GetBitsAllocated()/8);
	bufSize *= (sizeY*sizeX);
	if(NumberOfFrames>0){
		bufSize *=NumberOfFrames;
	}
	setupImage(sizeX,sizeY,ImageNo,dicom_loader.GetImagePixels(),bufSize);


	{
		char photInterp[17];
#ifdef USE_NEW_LIB
		PxDicomStatus status = dicom_loader.GetValue(0x00280004, photInterp, sizeof(photInterp)-1);
#else
		VLIDicomStatus status = dicom_loader.GetValue(0x00280004, photInterp, sizeof(photInterp)-1);		
#endif
		if (status == kNormalCompletion)
		{
			 //	(0028,0004) CS    [MONOCHROME2]                          #    12,  1  PhotometricInterpretation
			m_DicomImage->SetValue(0x00280004,photInterp);

		}

		//	 0x00280006
		m_DicomImage->SetValue(0x00280006,dicom_loader.GetPlanarConfiguration());

		

	
	}
	 
	//	(0028,0002) US    1                                      #     2,  1  SamplesPerPixel
	m_DicomImage->SetValue(0x00280002,dicom_loader.GetSamplesPerPixel());

	//	(0028,0100) US    16                                     #     2,  1  BitsAllocated
	m_DicomImage->SetValue(0x00280100,dicom_loader.GetBitsAllocated());

//	(0028,0101) US    16                                     #     2,  1  BitsStored
	m_DicomImage->SetValue(0x00280101,dicom_loader.GetBitsStored());

//	(0028,0102) US    15                                     #     2,  1  HighBit
	m_DicomImage->SetValue(0x00280102,dicom_loader.GetHighBit());

 
#if 0
	 unsigned short GetBitsStored (void) {return m_bitsStored;}
    
    unsigned short GetHighBit (void) {return m_highBit;}
    
    unsigned short GetSamplesPerPixel (void) {return m_samplesPerPixel;}
    
    unsigned short GetBytesPerPixel (void) {return m_bytesPerPixel;}
   
	double PixelSpacing_out[2];
		void GetPixelSpacing (PixelSpacing_out);

	dicom_loader.GetImagePixels();
#endif
//	m_DicomImage->

	return true;
}
#ifdef USE_NEW_LIB
CPxDicomImage *CTstVLIDicomImage::getDicomImage() const 
#else
VLIDicomImage *CTstVLIDicomImage::getDicomImage() const 
#endif
{ 
	return m_DicomImage;
}
void CTstVLIDicomImage::outputTAG(const char *outputFile)
{
	openOutFile(outputFile);

	unsigned short  ushort_temp;

	long long_temp;


#ifdef USE_NEW_LIB
	PxDicomStatus status;
#else
	VLIDicomStatus status;	
#endif
	//

	int  oTransferSyntax;
	bool  oIsCompressed;
	status = m_DicomImage->ExtractTransferSyntax (oTransferSyntax, oIsCompressed);
	outputMessage(" TransferSyntax [%d] TransferSyntax %d , oIsCompressed %d\n",status,oTransferSyntax, oIsCompressed);

	//
	ushort_temp = m_DicomImage->GetBitsAllocated();
	outputMessage(" BitsAllocated %d \n",ushort_temp);

	//
	ushort_temp = m_DicomImage->GetBitsStored();
	outputMessage(" BitsStored %d \n",ushort_temp);
		
	//
	ushort_temp = m_DicomImage->GetHighBit();
	outputMessage(" HighBit %d \n",ushort_temp);

	//
	ushort_temp = m_DicomImage->GetBytesPerPixel();
	outputMessage(" BytesPerPixel %d \n",ushort_temp);

	//
	long_temp	= m_DicomImage->GetNumberOfBytesOfPixelData();
	outputMessage(" NumberOfBytesOfPixelData %d \n",long_temp);


	//
	double oThickness;
	oThickness = m_DicomImage->GetSliceThickness( );
	outputMessage(" SliceThickness   %.4f  \n",oThickness);

	//
	closeOutFile();
}


void CTstVLIDicomImage::openNewDicom()
{
	if(m_DicomImage) delete m_DicomImage;
#ifdef USE_NEW_LIB
	m_DicomImage = new CPxDicomImage;
#else
	m_DicomImage = new VLIDicomImage;
#endif

	m_DicomImage->CheckValid();

	setupDefault();
  
	if(0){
		int sizeX = 256;
		int sizeY = 256;
		unsigned short *data = new unsigned short[sizeX*sizeY];
		for(int y_i = 0 ;y_i <sizeY; y_i++){
			for(int x_i = 0 ;x_i <sizeX; x_i++){
				data[y_i*sizeX + x_i] = x_i;
			}
		}
		setupImage(sizeX,sizeY,0,(unsigned char*)data);
	}
}
void CTstVLIDicomImage::saveDicom(const char *filename)
{
	if(!m_DicomImage) return;
#ifdef USE_NEW_LIB
	PxDicomStatus status = m_DicomImage->Save(filename);
#else
	VLIDicomStatus status = m_DicomImage->Save(filename);
#endif

}

void CTstVLIDicomImage::ConvertToFile(const char *filename)
{
	if(!m_DicomImage) return;
#ifdef USE_NEW_LIB
	PxDicomStatus status = m_DicomImage->ConvertToFile(filename);
#else
	VLIDicomStatus status = m_DicomImage->ConvertToFile(filename);
#endif
}

void CTstVLIDicomImage::setupDefault()
{
	if(!m_DicomImage) return;
	
//////////////////////////

//(0002,0000) UL    196                                    #     4,  1  MetaElementGroupLength
//(0002,0001) OB    00\01                                  #     2,  1  FileMetaInformationVersion
	m_DicomImage->SetValue(0x00020001,"00\\01");

//(0002,0002) UI    =CTImageStorage                        #    26,  1  MediaStorageSOPClassUID
	m_DicomImage->SetValue(0x00020002,"1.2.840.10008.5.1.4.1.1.2");

//(0002,0003) UI    [1.2.392.200183.123.456.200409021920.031.6.3] #    44,  1  MediaStorageSOPInstanceUID
	m_DicomImage->SetValue(0x00020003,"1.2.392.200183.123.456.200409021920.031.6.3");

//(0002,0010) UI    =LittleEndianImplicit                  #    18,  1  TransferSyntaxUID
	m_DicomImage->SetValue(0x00020010,"LittleEndianImplicit");

//(0002,0012) UI    [2.16.840.1.113669.2.1.1]              #    24,  1  ImplementationClassUID
	m_DicomImage->SetValue(0x00020012,"2.16.840.1.113669.2.1.1");

//(0002,0013) SH    [MergeCOM3_340]                        #    14,  1  ImplementationVersionName
	m_DicomImage->SetValue(0x00020013,"MergeCOM3_340");

//(0002,0016) AE    [AUTOVOX]                              #     8,  1  SourceApplicationEntityTitle
	m_DicomImage->SetValue(0x00020016,"AUTOVOX");

//////////////////////////
 
//	(0008,0005) CS    (no value available)                   #     0,  0  SpecificCharacterSet

//	(0008,0008) CS    [ORIGINAL\PRIMARY\AXIAL]               #    22,  3  ImageType
	m_DicomImage->SetValue(0x00080008,"ORIGINAL\\PRIMARY\\AXIAL");

//	(0008,0014) UI    [1.2.392.200183.123.456]               #    22,  1  InstanceCreatorUID
	m_DicomImage->SetValue(0x00080014,"1.2.392.200183.123.456");

//	(0008,0016) UI    =CTImageStorage                        #    26,  1  SOPClassUID
	m_DicomImage->SetValue(0x00080016,"1.2.840.10008.5.1.4.1.1.2");

//	(0008,0018) UI    [1.2.392.200183.123.456.200409021920.031.6.3] #    44,  1  SOPInstanceUID
	m_DicomImage->SetValue(0x00080018,"1.2.392.200183.123.456.200409021920.031.6.3");

//	(0008,0020) DA    [20040902]                             #     8,  1  StudyDate
	m_DicomImage->SetValue(0x00080020,"20040902");



//	(0008,0021) DA    [20040902]                             #     8,  1  SeriesDate
	m_DicomImage->SetValue(0x00080021,"20040902");

//	(0008,0023) DA    [20040902]                             #     8,  1  ImageDate
	m_DicomImage->SetValue(0x00080023,"20040902");

//	(0008,0030) TM    [191945.484]                           #    10,  1  StudyTime
	m_DicomImage->SetValue(0x00080030,"191945.484");

//	(0008,0031) TM    [191945.484]                           #    10,  1  SeriesTime
	m_DicomImage->SetValue(0x00080031,"191945.484");

//	(0008,0033) TM    [1920]                                 #     4,  1  ImageTime
	m_DicomImage->SetValue(0x00080033,"1920");

//	(0008,0050) SH    [1]                                    #     2,  1  AccessionNumber
	m_DicomImage->SetValue(0x00080050,"1");

//	(0008,0060) CS    [CT]                                   #     2,  1  Modality
	m_DicomImage->SetValue(0x00080060,"CT");

//	(0008,0070) LO    [YoshidaDental]                        #    14,  1  Manufacturer
	m_DicomImage->SetValue(0x00080070,"Manufacturer");

//	(0008,0080) LO    (no value available)                   #     0,  0  InstitutionName
	m_DicomImage->SetValue(0x00080080,"FineCube");

//	(0008,0090) PN    [ReferringPhysiciansName]              #    24,  1  ReferringPhysiciansName

//	(0008,1010) SH    [001]                                  #     4,  1  StationName
//	(0008,1060) PN    [NameOfPhysiciansReadingStudy]         #    28,  1  NameOfPhysiciansReadingStudy

//	(0008,1070) PN    [.$B$?$+$R$m.(B]                       #    14,  1  OperatorsName
	m_DicomImage->SetValue(0x00081070,"オペレター名前");

//	(0008,1090) LO    [FineCube]                             #     8,  1  ManufacturerModelName
	m_DicomImage->SetValue(0x00081090,"FineCube");

//	(0008,1010) SH    [CT-01]                                #     6,  1  StationName
	m_DicomImage->SetValue(0x00081010,"CT-01");

 
//(0008,1030) LO    [.$B=QA0.(B]                           #    10,  1  StudyDescription
//(0008,103e) LO    [.$B4pK\.(BCT.$B2hA|.(B]               #    22,  2  SeriesDescription
	m_DicomImage->SetValue(0x00081030,"Study Description日本語");

	m_DicomImage->SetValue(0x0008103E,"Series Description日本語");



//	(0010,0010) PN    [DentalCT^TestData]                    #    18,  1  PatientName
	m_DicomImage->SetValue(0x00100010,"XTDebugName");

//	(0010,0020) LO    [20040902]                             #     8,  1  PatientID
	m_DicomImage->SetValue(0x00100020,"22200001");

//	(0010,0030) DA    [19840902]                             #     8,  1  PatientBirthDate
	m_DicomImage->SetValue(0x00100030,"19840902");

//	(0010,0040) CS    [O]                                    #     2,  1  PatientSex
	m_DicomImage->SetValue(0x00100040,"F");
	m_DicomImage->SetValue(0x00100040,"M");

//	(0010,1010) AS    (no value available)                   #     0,  0  PatientAge
//	(0018,0050) DS    [0.14]                                 #     4,  1  SliceThickness
	m_DicomImage->SetValue(0x00180050,0.14);

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
	m_DicomImage->SetPrivateValue("TR",0x0019,0x0010);
	 
//	(0019,1010) ??    35\31\33\20                            #     4,  1  Unknown Tag & Data
	m_DicomImage->SetValue(0x00191010,"35\\31\\33\\20");

//	(0019,1020) ??    35\31\32\20                            #     4,  1  Unknown Tag & Data
	m_DicomImage->SetValue(0x00191020,"35\\31\\32");

//	(0019,1063) ??    32\39                                  #     2,  1  Unknown Tag & Data
	m_DicomImage->SetValue(0x00191063,"32\\39");

#endif
//	(0020,000d) UI    [1.2.392.200183.123.456.20040902191927.1] #    40,  1  StudyInstanceUID
	m_DicomImage->SetValue(0x0020000d,"1.2.392.200183.123.456.20040902191927.1");

//	(0020,000e) UI    [1.2.392.200183.123.456.20040902191927.301.2] #    44,  1  SeriesInstanceUID
	m_DicomImage->SetValue(0x0020000e,"1.2.392.200183.123.456.20040902191927.301.2");

//	(0020,0010) SH    [0409021800]                           #    10,  1  StudyID
	m_DicomImage->SetValue(0x00200010,"0409021800");

//	(0020,0011) IS    [301]                                  #     4,  1  SeriesNumber
	m_DicomImage->SetValue(0x00200011,"301");

//	(0020,0012) IS    [1]                                    #     2,  1  AcquisitionNumber
	m_DicomImage->SetValue(0x00200012,"1");

//	(0020,0013) IS    [6]                                    #     2,  1  ImageNumber
	m_DicomImage->SetValue(0x00200013,"1");

//	(0020,0032) DS    [-35.8400\-35.8400\35.1400]            #    26,  3  ImagePositionPatient
	m_DicomImage->SetValue(0x00200032,"-35.8400\\-35.8400\\35.1400");

//	(0020,0037) DS    [1.000000\0.000000\0.000000\0.000000\1.000000\0.000000] #    54,  6  ImageOrientationPatient
	m_DicomImage->SetValue(0x00200037,"1.000000\\0.000000\\0.000000\\0.000000\\1.000000\\0.000000");

//	(0020,0052) UI    [1.2.392.200183.123.456.20040902191927.4] #    40,  1  FrameOfReferenceUID
	m_DicomImage->SetValue(0x00200052,"1.2.392.200183.123.456.20040902191927.4");

//	(0020,1040) LO    [x]                                    #     2,  1  PositionReferenceIndicator
	m_DicomImage->SetValue(0x00201040,"x");

//	(0020,1041) DS    [35.14]                                #     6,  1  SliceLocation
	m_DicomImage->SetValue(0x00201041,"35.14");

//	(0020,4000) LT    (no value available)                   #     0,  1  ImageComments

//	(0028,0002) US    1                                      #     2,  1  SamplesPerPixel
	m_DicomImage->SetValue(0x00280002,1);

//	(0028,0004) CS    [MONOCHROME2]                          #    12,  1  PhotometricInterpretation
	m_DicomImage->SetValue(0x00280004,"MONOCHROME2");

//	(0028,0010) US    512                                    #     2,  1  Rows
	m_DicomImage->SetValue(0x00280010,512);

//	(0028,0011) US    512                                    #     2,  1  Columns
	m_DicomImage->SetValue(0x00280011,512);

//	(0028,0030) DS    [0.140\0.140]                          #    12,  2  PixelSpacing
	m_DicomImage->SetValue(0x00280030,"0.140\\0.140");

//	(0028,0100) US    16                                     #     2,  1  BitsAllocated
	m_DicomImage->SetValue(0x00280100,16);

//	(0028,0101) US    16                                     #     2,  1  BitsStored
	m_DicomImage->SetValue(0x00280101,16);

//	(0028,0102) US    15                                     #     2,  1  HighBit
	m_DicomImage->SetValue(0x00280102,15);

//	(0028,0103) US    1                                      #     2,  1  PixelRepresentation
	m_DicomImage->SetValue(0x00280103,0);

//	(0028,1050) DS    [1000]                                 #     4,  1  WindowCenter
	m_DicomImage->SetValue(0x00281050,1000);

//	(0028,1051) DS    [2000]                                 #     4,  1  WindowWidth
	m_DicomImage->SetValue(0x00281051,2000);

//	(0028,1052) DS    [0]                                    #     2,  1  RescaleIntercept
	m_DicomImage->SetValue(0x00281052,0);

//	(0028,1053) DS    [1]                                    #     2,  1  RescaleSlope
	m_DicomImage->SetValue(0x00281053,1);

}
void CTstVLIDicomImage::setupImage(int sizeX,int sizeY, int FrameNo,unsigned char* iBuf,int bufSize)
{


   double posX = -35.8400;
   double posY = -35.8400;
   double posZ = 35.14 - m_pitchZ*FrameNo;

   sprintf(_str_buff,"%0.4f\\%0.4f\\%0.4f",posX,posY,posZ);
	//	(0020,0032) DS    [-35.8400\-35.8400\35.1400]            #    26,  3  ImagePositionPatient
	m_DicomImage->SetValue(0x00200032,_str_buff);

 
	//	(0018,0050) DS    [0.14]                                 #     4,  1  SliceThickness
	sprintf(_str_buff,"%f",m_pitchZ);
	m_DicomImage->SetValue(0x00180050,_str_buff);
	m_DicomImage->SetValue(0x00180050,m_pitchZ);


//	(0028,0010) US    512                                    #     2,  1  Rows
	m_DicomImage->SetValue(0x00280010,sizeY);

//	(0028,0011) US    512                                    #     2,  1  Columns
	m_DicomImage->SetValue(0x00280011,sizeX);
/////////

//	(0028,0030) DS    [0.140\0.140]                          #    12,  2  PixelSpacing
//	m_DicomImage->SetValue(0x00280030,"0.140\\0.140");
	sprintf(_str_buff,"%0.4f\\%0.4f",m_pitchX,m_pitchY );
	m_DicomImage->SetValue(0x00280030,_str_buff);

#if 0

//	(0028,0100) US    16                                     #     2,  1  BitsAllocated
	m_DicomImage->SetValue(0x00280100,16);

//	(0028,0101) US    16                                     #     2,  1  BitsStored
	m_DicomImage->SetValue(0x00280101,16);

//	(0028,0102) US    15                                     #     2,  1  HighBit
	m_DicomImage->SetValue(0x00280102,15);
	
#endif

// (0018,1100) DS    [81]                                   #     2,  1  ReconstructionDiameter
	m_DicomImage->SetValue(0x00181100,81);

	int pixel_size = 2*sizeX*sizeY;
	if(bufSize>0){
		pixel_size = bufSize;
	}
	m_DicomImage->SetPixelData(iBuf, pixel_size, IMPLICIT_LITTLE_ENDIAN);

}
void CTstVLIDicomImage::setPatientName(const string &name)
{
	string org_str,conv_str;
	org_str = name;
	ConvertSJISToJIS(org_str,conv_str);
	 
	m_DicomImage->SetValue(0x00100010,conv_str.c_str());
}
void CTstVLIDicomImage::setPatientID(const string &id)
{
	m_DicomImage->SetValue(0x00100020,id.c_str());
}

 
void CTstVLIDicomImage::setStudyID(const string & studyID)
{
	m_DicomImage->SetValue(0x00200010,studyID.c_str()); //StudyID

//	string study_instance_uid = "1.2.392.200183.123.456.20040902191927.";
//	study_instance_uid = study_instance_uid + studyID;
//	m_DicomImage->SetValue(0x0020000d,study_instance_uid.c_str()); //StudyInstanceUID
}
void CTstVLIDicomImage::setStudyInstanceUID(const string &studyInstanceUID)
{
	m_DicomImage->SetValue(0x0020000d,studyInstanceUID.c_str()); //StudyInstanceUID
}
void CTstVLIDicomImage::setSeriesInstanceUID(const string &seriesInstanceUID)
{
	m_SeriesInstanceID = seriesInstanceUID;
	m_DicomImage->SetValue(0x0020000e,seriesInstanceUID.c_str());
}

 
void CTstVLIDicomImage::setImageNumber(int no)
{
	m_ImageNumber = no;

	m_DicomImage->SetValue(0x00200013,no);
}
 


void CTstVLIDicomImage::setupCurDate(string studyDate,string seriesTime)
{


//	(0008,0020) DA    [20040902]                             #     8,  1  StudyDate
	m_DicomImage->SetValue(0x00080020,studyDate.c_str());


//	(0008,0021) DA    [20040902]                             #     8,  1  SeriesDate
	m_DicomImage->SetValue(0x00080021,studyDate.c_str());

//	(0008,0023) DA    [20040902]                             #     8,  1  ImageDate
	m_DicomImage->SetValue(0x00080023,studyDate.c_str());

//	(0008,0030) TM    [191945.484]                           #    10,  1  StudyTime
	m_DicomImage->SetValue(0x00080030,seriesTime.c_str());

//	(0008,0031) TM    [191945.484]                           #    10,  1  SeriesTime
	m_DicomImage->SetValue(0x00080031,seriesTime.c_str());

//	(0008,0033) TM    [1920]                                 #     4,  1  ImageTime
	m_DicomImage->SetValue(0x00080033,seriesTime.c_str());

}

void CTstVLIDicomImage::setupSOPInstanceUID(const string &uid)
{
	if(uid.size()>1){
		m_DicomImage->SetValue(0x00080018,uid.c_str()); ///SOPInstanceUID
		return;
	}

	char char_buff[64];
	sprintf(char_buff,".%d",m_ImageNumber);
 
	string SOPInstanceUID = m_InstanceCreatorUID + m_SeriesInstanceID ;

	//append date;
	struct tm cur_time;
	 _getsystime(&cur_time);
  	sprintf(char_buff,".%04d%02d%02d%02d%02d%02d",
					cur_time.tm_year+1900,
					cur_time.tm_mon+1,
					cur_time.tm_mday,
					cur_time.tm_hour,
					cur_time.tm_min,
					cur_time.tm_sec
					);
	SOPInstanceUID = SOPInstanceUID  + char_buff;

	//append image number;
	sprintf(char_buff,".%d",m_ImageNumber);
	SOPInstanceUID = SOPInstanceUID + char_buff;

	m_DicomImage->SetValue(0x00080018,SOPInstanceUID.c_str()); ///SOPInstanceUID

}

void CTstVLIDicomImage::setSeriesDescription(const char *des)
{ 
	strcpy(m_SeriesDescription , des);
};
void CTstVLIDicomImage::setStudyDescription(const char *des)
{ 
	strcpy(m_StudyDescription , des);
} 
void CTstVLIDicomImage::setReferringPhysician(const char *name)
{
	strcpy(m_ReferringPhysician , name);
}
void CTstVLIDicomImage::prepareDICOM()
{
 
	string org_str,conv_str;


	if(m_SeriesDescription[0]){
		org_str = m_SeriesDescription;
		ConvertSJISToJIS(org_str,conv_str);
//		sprintf(_str_buff,"SeriesDes日本語 [%d]",m_testDataType);
//(0008,103e) LO    [.$B4pK\.(BCT.$B2hA|.(B]               #    22,  2  SeriesDescription
		m_DicomImage->SetValue(0x0008103E,conv_str.c_str());
	}

	if(m_StudyDescription[0]){
		org_str = m_StudyDescription;
		ConvertSJISToJIS(org_str,conv_str);
//(0008,1030) LO    [.$B=QA0.(B]                           #    10,  1  StudyDescription
		m_DicomImage->SetValue(0x00081030,conv_str.c_str());
	}

	if(m_ReferringPhysician[0]){
		org_str = m_ReferringPhysician;
		ConvertSJISToJIS(org_str,conv_str);
//(0008,0090) PN    [ReferringPhysiciansName]              #    24,  1  ReferringPhysiciansName
		m_DicomImage->SetValue(0x00080090,conv_str.c_str());
	}

	switch(m_Modality){
	default:
	case 0: //CT
	//	(0008,0060) CS    [CT]                                   #     2,  1  Modality
	m_DicomImage->SetValue(0x00080060,"CT");
	break;
	case 1: //PX
	//	(0008,0060) CS    [PX]                                   #     2,  1  Modality
	m_DicomImage->SetValue(0x00080060,"PX");
	break;
	}

	//	(0008,0016) UI    =CTImageStorage                        #    26,  1  SOPClassUID
//	m_DicomImage->SetValue(0x00080016,"1.2.840.10008.3.1.2.1.1");

 
	if(m_useRescale){
	//	(0028,1052) Rescale Intercept 
		m_DicomImage->SetValue(0x00281052, m_RescaleIntercept);
	//	(0028,1053) Rescale Slope 
		m_DicomImage->SetValue(0x00281053, m_RescaleSlope);
	//	(0028,0103) US    1                                      #     2,  1  PixelRepresentation
		m_DicomImage->SetValue(0x00280103,0); // unsigned short

	}else{
	//	(0028,1052) Rescale Intercept 
		m_DicomImage->SetValue(0x00281052, 0);
	//	(0028,1053) Rescale Slope 
		m_DicomImage->SetValue(0x00281053, 1);
	//	(0028,0103) US    1                                      #     2,  1  PixelRepresentation
		m_DicomImage->SetValue(0x00280103,1); //  short
	}

	//	(0028,0100) US    16                                     #     2,  1  BitsAllocated
	m_DicomImage->SetValue(0x00280100,m_BitsAllocated);
	//	(0028,0101) US    16                                     #     2,  1  BitsStored
	m_DicomImage->SetValue(0x00280101, m_BitsStored);

	//	(0028,0102) US    15                                     #     2,  1  HighBit
	m_DicomImage->SetValue(0x00280102,m_HighBit);


}

bool CTstVLIDicomImage::createFromMessasgeID(int messageID)
{
	if(m_DicomImage) delete m_DicomImage;

#ifdef USE_NEW_LIB
	m_DicomImage = new CPxDicomImage(messageID);
#else
	m_DicomImage = new VLIDicomImage(messageID);
#endif
 
	return true;
}
unsigned char*	CTstVLIDicomImage::GetImagePixels()
{
	if(!m_DicomImage) return 0;

	return  m_DicomImage->GetImagePixels();
}
bool CTstVLIDicomImage::checkImageData(int z_no,int &ref_dataType, int & ref_seriesUID,int &totalFrame )
{
#ifdef USE_NEW_LIB
	CPxDicomImage* vli_dicomImage  = getDicomImage();
#else
	VLIDicomImage* vli_dicomImage  = getDicomImage();
#endif
 
	if(!vli_dicomImage) return false;
	std::string PatineName = vli_dicomImage->GetValue(kVLIPatientsName);
	std::string PatientID = vli_dicomImage->GetValue(kVLIPatientId);

	std::string StudyDes = vli_dicomImage->GetValue(kVLIStudyDescription);
	std::string SeriesDes = vli_dicomImage->GetValue(kVLISeriesDescription);


	int studyID=-1;
	int frameNum=-1;
	if(StudyDes.size()>0){
		sscanf(StudyDes.c_str(),getTestDataPatternFromStudyDesString().c_str(),&studyID);
	}


	int seriesUID=-1;
	int dataType=-1;
	if(SeriesDes.size()>0){
		sscanf(SeriesDes.c_str(),getTestDataPatternFromSereisDesString().c_str(),
								&frameNum,&seriesUID,&dataType);
	}

	if(frameNum>0){
		totalFrame = frameNum;
	}

	int imageNumber ;
	vli_dicomImage->GetValue(kVLIInstanceNumber,&imageNumber);

	unsigned char * pixel_data_ptr = GetImagePixels();

	int sizeX,sizeY;
	vli_dicomImage->GetValue(kVLIColumns,&sizeX);
	vli_dicomImage->GetValue(kVLIRows,&sizeY);


	unsigned short *image_data_ptr = (unsigned short *)pixel_data_ptr;

	test_data_struct *data_first = (test_data_struct *)pixel_data_ptr;
	test_data_struct *data_last  = (test_data_struct *)(pixel_data_ptr + 2*(sizeX*sizeY) - sizeof(test_data_struct) );


	//同じSereisにあるデータは同じとなるもの
	if(ref_dataType>=0){
		if(dataType != ref_dataType){
			m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR dataType[%d] != ref_dataType[%d] \n",
								dataType, ref_dataType);
			 m_Logger.FlushLog();
			 return false;
		}
	}else{
		//一回目
		ref_dataType = dataType;
	}
	//
	if(ref_seriesUID>=0){
		if(seriesUID != ref_seriesUID){
			m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR seriesUID[%d] != ref_seriesUID[%d] \n",
				seriesUID , ref_seriesUID );
			 m_Logger.FlushLog();
			 return false;
		}
	}else{
		//一回目
		ref_seriesUID = seriesUID;
	}
	/////////////////////


	//first data point
	if(data_first->uid != seriesUID){
		 m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR data_first->uid[%d] != seriesUID[%d] \n",
								data_first->uid , seriesUID);
		 m_Logger.FlushLog();
		 return false;
	}
	if(data_first->dataType != dataType){
		 m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR data_first->dataType[%d] != dataType[%d] \n",
								data_first->dataType , dataType);
		 m_Logger.FlushLog();
		 return false;
	}
	if(data_first->imageNumber != imageNumber){
		 m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR data_first->imageNumber[%d] != imageNumber[%d] \n",
								data_first->imageNumber , imageNumber);
		 m_Logger.FlushLog();
		 return false;
	}
	//last data point
	if(data_last->uid != seriesUID){
		 m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR data_last->uid[%d] != seriesUID[%d] \n",
								data_last->uid , seriesUID);
		 m_Logger.FlushLog();
		 return false;
	}
	if(data_last->dataType != dataType){
		 m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR data_last->dataType[%d] != dataType[%d] \n",
								data_last->dataType , dataType);
		 m_Logger.FlushLog();
		 return false;
	}
	if(data_last->imageNumber != imageNumber){
		 m_Logger.LogMessage(" CTstVLIDicomImage::checkImageData ERROR data_last->imageNumber[%d] != imageNumber[%d] \n",
								data_last->imageNumber , imageNumber);
		 m_Logger.FlushLog();
		 return false;
	}

	return true;
}
void CTstVLIDicomImage::embedTestPattern(int uid, int dataType,int imageNumber, unsigned char *Pixel, int PixelSize)
{
	test_data_struct *data_first = (test_data_struct *)Pixel;
	data_first->uid			= uid;
	data_first->dataType	= dataType;
	data_first->imageNumber	= imageNumber;
	//
	test_data_struct *data_last  = (test_data_struct *)(Pixel + PixelSize  - sizeof(test_data_struct));
	data_last->uid			= uid;
	data_last->dataType		= dataType;
	data_last->imageNumber	= imageNumber;

}