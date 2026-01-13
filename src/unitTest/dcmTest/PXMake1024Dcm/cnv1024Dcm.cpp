// testFxDcmData.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
 

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

 
#include "IDcmLibApi.h"
#include "PxDicomMessage.h"
#include "PxDicomImage.h"
#define DICOM_MSG CPxDicomMessage
#define DICOM_IMG CPxDicomImage
using namespace XTDcmLib;
 
 
#include "TstVLIDicomImage.h"
 
#include "AqCore/TRPlatform.h"



#define MyRootUID "1.2.392.200036.9163"   //PreXion DICOM root UID

 
 
 
std::string g_PatientName = "test high res1000^1000";
std::string g_PatientID = "5011000";

std::string genStudyInstanceUID()
{
	char _char_buff[128];

	unsigned long tick_count = ::GetTickCount();
	sprintf(_char_buff,".%u",tick_count );
	 
	std::string StudyInstanceUID = std::string(MyRootUID) + _char_buff;

	return StudyInstanceUID;
}
 
std::string genSOPInstanceUID(int imageCount,std::string &StudyInstanceUID)
{
	char _char_buff[128];
	sprintf(_char_buff,".%u",imageCount+1);
	std::string  SOPInstanceUID = StudyInstanceUID  +_char_buff;

	return SOPInstanceUID;
}
template<class typeT>
void resolutionXY( typeT *input,int sizeX,int sizeY,  typeT *output)
{
 
	int new_sizeX = sizeX*2;
	for(int y_i=0;y_i<sizeY;y_i++){
		int new_y_i = y_i*2;
		int y_i_p = (sizeY+y_i+1)%sizeY;
		for(int x_i=0;x_i<sizeX;x_i++){
			int new_x_i = x_i*2;
			int x_i_p = (sizeX+x_i+1)%sizeX;

			typeT val_nn	= input[y_i*sizeX + x_i];
			typeT val_np	= input[y_i*sizeX + x_i_p];
			typeT val_pn	= input[y_i_p*sizeX + x_i];
			typeT val_pp	= input[y_i_p*sizeX + x_i_p];

			output[new_y_i*new_sizeX + new_x_i]			= val_nn;
			output[new_y_i*new_sizeX + new_x_i+1]		= (typeT)(0.5*val_nn+0.5*val_np);
			//
			output[(new_y_i+1)*new_sizeX + new_x_i ]	= (typeT)(0.5*val_nn+0.5*val_pn);
			output[(new_y_i+1)*new_sizeX + new_x_i+1 ]	= (typeT)(0.5*val_nn+0.5*val_pp);
		}
	}
 
}
template<class typeT>
void resolutionSlice(  typeT *input1,  typeT *input2,int sizeX,int sizeY,  typeT *output)
{
	for(int y_i=0;y_i<sizeY;y_i++){
		 
		for(int x_i=0;x_i<sizeX;x_i++){
 			output[y_i*sizeX +  x_i]			= (typeT)(0.5* input1[y_i*sizeX + x_i] + 0.5* input2[y_i*sizeX + x_i]) ;
//			output[y_i*sizeX +  x_i]			=  input2[y_i*sizeX + x_i];
			 
		}
	}
}
std::string g_curStudyInstanceUID;

int  g_sizeX = 512;
int  g_sizeY = 512;
unsigned short *g_new_slice_512 = 0;
unsigned short *g_new_slice_1024 = 0;
void procDicomFile(int dicom_no,CTstVLIDicomImage *dicomImage_ptr1,CTstVLIDicomImage *dicomImage_ptr2,const std::string &out_FolderName)
{
	if(g_new_slice_512==0){
		g_new_slice_512 = new unsigned  short[g_sizeX*g_sizeY];
	}
		
	if(g_new_slice_1024==0){
		g_new_slice_1024 = new unsigned  short[g_sizeX*g_sizeY*4];
	}

	char _file_name[256];
	sprintf(_file_name,"%04dDcm",2*dicom_no);
	std::string out_file1 = out_FolderName + _file_name;
	//
	sprintf(_file_name,"%04dDcm",2*dicom_no+1);
	std::string out_file2 = out_FolderName + _file_name;
	//
	CTstVLIDicomImage *src_image = dicomImage_ptr1;

	CPxDicomImage *dicomImage_dicom = src_image->getDicomImage();
 
	if(!dicomImage_dicom) return  ;

	int image_number = dicom_no*2;
	 
	src_image->setImageNumber(image_number);
	 
	src_image->setPatientName(g_PatientName );
	src_image->setPatientID(g_PatientID);
	//src_image->setStudyID(m_CurStudyID);

	src_image->setStudyInstanceUID(g_curStudyInstanceUID);
//	src_image->setSeriesInstanceUID(m_CurSeriesID);
	src_image->setSeriesInstanceUID(g_curStudyInstanceUID);

 
	{
		dicomImage_dicom->SetValue(0x00081070,"OperatorName");
 
 
//(0008,1030) LO    [.$B=QA0.(B]                           #    10,  1  StudyDescription
//(0008,103e) LO    [.$B4pK\.(BCT.$B2hA|.(B]               #    22,  2  SeriesDescription
	dicomImage_dicom->SetValue(0x00081030,"Study Description ");

	dicomImage_dicom->SetValue(0x0008103E,"Series Description ");
	}

 
	src_image->setupSOPInstanceUID(genSOPInstanceUID(image_number,g_curStudyInstanceUID));

	src_image->prepareDICOM();
 	src_image->setupPitch(0.12,0.12,0.12);
		 
	CPxDicomImage *ptr_dicom1 = new CPxDicomImage(dicomImage_dicom->GetID());
	CPxDicomImage *ptr_dicom2 = new CPxDicomImage(dicomImage_ptr2->getDicomImage()->GetID());
	 
	ptr_dicom1->HandoverID();//m_doNotFree
	ptr_dicom2->HandoverID();

	resolutionXY(( short*)ptr_dicom1->GetImagePixels(),g_sizeX,g_sizeY,(short*)g_new_slice_1024);
//			 
 	src_image->setupImage(g_sizeX*2 ,g_sizeY*2 ,image_number,(unsigned char*)g_new_slice_1024);
 
	src_image->saveDicom(out_file1.c_str());

	//dicom 2
	image_number = dicom_no*2+1;
	src_image->setImageNumber(image_number);

	src_image->setupSOPInstanceUID(genSOPInstanceUID(image_number,g_curStudyInstanceUID));
	resolutionSlice((  short*)ptr_dicom1->GetImagePixels(),(  short*)ptr_dicom2->GetImagePixels(),g_sizeX,g_sizeY,(short*)g_new_slice_512);
	resolutionXY(( short*)g_new_slice_512,g_sizeX,g_sizeY,(short*)g_new_slice_1024);

	src_image->prepareDICOM();
 	src_image->setupPitch(0.12,0.12,0.12);

	src_image->setupImage(g_sizeX*2 ,g_sizeY*2 ,image_number,(unsigned char*)g_new_slice_1024);
	src_image->saveDicom(out_file2.c_str());

	delete ptr_dicom1;
	delete ptr_dicom2;
}
int doCnv1024DicomFileList(const std::string &in_FolderName,const std::string &out_FolderName,std::vector<TRFileName> &DicomFileList )
{
	
	if(!TRDICOMUtil::InitialDICOM("DBA6-5B453") )
		return  0;

	g_curStudyInstanceUID = genStudyInstanceUID();

	CTstVLIDicomImage *dicomImage_hd1 = new CTstVLIDicomImage;
	CTstVLIDicomImage *dicomImage_hd2 = new CTstVLIDicomImage;


	CTstVLIDicomImage *dicomImage_proc1;  
	CTstVLIDicomImage *dicomImage_proc2;

	int list_size = DicomFileList.size();
	std::string fileName1 = in_FolderName + DicomFileList[0].GetName();
	
	dicomImage_hd1->loadDicom(fileName1.c_str());
	dicomImage_proc1 = dicomImage_hd1;
	dicomImage_proc2 = dicomImage_hd2;
	for(int i=1;i<list_size;i++){
		printf(" %d/%d ...\n",i,list_size);
		if((i%2) == 1){
			dicomImage_proc1 = dicomImage_hd1;
			dicomImage_proc2 = dicomImage_hd2;
		}else{
			dicomImage_proc1 = dicomImage_hd2;
			dicomImage_proc2 = dicomImage_hd1;
		}
		std::string fileName = in_FolderName + DicomFileList[i].GetName();
		dicomImage_proc2->loadDicom(fileName.c_str());
		//
		procDicomFile( i-1, dicomImage_proc1, dicomImage_proc2,out_FolderName);
	 
	}
	//last
	if(((list_size)%2) == 1){
		dicomImage_proc1 = dicomImage_hd1;
		dicomImage_proc2 = dicomImage_hd2;
	}else{
		dicomImage_proc1 = dicomImage_hd2;
		dicomImage_proc2 = dicomImage_hd1;
	}
	std::string fileName = in_FolderName + DicomFileList[list_size-1].GetName();
	dicomImage_proc2->loadDicom(fileName.c_str());
	//
	procDicomFile( list_size-1, dicomImage_proc1, dicomImage_proc2,out_FolderName);


	delete dicomImage_hd1;
	delete dicomImage_hd2;

	return 0;
}

int doCnv1024DicomFile(const std::string &in_FolderName,const std::string &out_FolderName )
{
	std::vector<TRFileName> DicomFileList;
	std::vector<TRFileName>::iterator iter;

	std::string seriesPath;
	int tmpStatus = TRPlatform::iGetDirectoryList(in_FolderName.c_str(), "*DCM", DicomFileList);
	if (tmpStatus < 0 || DicomFileList.size() < 3){
		return false;
	}

	unsigned long start_time = ::GetTickCount();

	for (iter = DicomFileList.begin(); iter < DicomFileList.end(); iter++)
	{
		if (!strcmp(iter->GetName(),".") || !strcmp(iter->GetName(), ".."))
			continue;
		seriesPath = in_FolderName + std::string("/") + iter->GetName();
		 
	}

	doCnv1024DicomFileList(in_FolderName, out_FolderName,DicomFileList);

	unsigned long end_time = ::GetTickCount();
	printf("\n <<< maek study list spent time %.2f >>> \n",(end_time - start_time)/1000.0f);

	return true;
}