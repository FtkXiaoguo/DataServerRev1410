// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include "TstWrite.h"

void tstDcmDataFileRead();
void tstDcmDataFileWrite();
 
void tstItem();
int _tmain(int argc, _TCHAR* argv[])
{

	_CrtMemState s1,s2,s3,ss;
 _CrtMemCheckpoint(&s1);
 
 

 
  

	printf(" start ...\n");

	 tstItem();

// 	tstDcmDataFileRead();
 	tstDcmDataFileWrite();
 
 
//	int *tt= new int[100];

	_CrtMemCheckpoint(&s2);
  if(_CrtMemDifference(&ss,&s1,&s2)){
		_CrtMemDumpStatistics( &ss );
  }else{
	  _CrtMemDumpStatistics( &ss );
  }

	return 0;
}


void tstDcmDataFileRead()
{
 
#if 0
	IDcmLib *dcmlib_instance = IDcmLib::createInstance();
//	DcmDataSet *dataset_instance = dcmlib_instance->createDcmDataSet();
//	dataset_instance->readFile("E:\\temp\\testdata_out.dcm");

 

	DcmXTDicomMessage *dcm_file_instance = dcmlib_instance->createDicomMessage();

	dcm_file_instance->setMaxReadLength(16);

//	dcm_file_instance->readFile("E:\\temp\\testdata_out.dcm");
	dcm_file_instance->readFile("E:\\temp\\testdata1.dcm");

//	dcm_file_instance->readFromDumpFile("E:\\temp\\testdata1.dcm.txt");
//	dcm_file_instance->readFromDumpFile("E:\\temp\\QR_Image.dcm.txt");


	DcmXTDataSet *dataset = dcm_file_instance->getDcmXTDataSet();

//	DcmXTComInterface *comInterface = (DcmXTComInterface *)dataset;
//	comInterface->Set_Value(0x00080022,"tttttttttttt");
//	comInterface->Set_Value(0x00280010,256);

#endif
}

void tstDcmDataFileWrite()
{
 
 CTstWrite write_dcm;

 write_dcm.init();
 
 IDcmLibApi::CheckMemory();

 write_dcm.dotestItem();

  write_dcm.doTest();

  IDcmLibApi::CheckMemory( 1 /*del_tbl*/);
 
  //
  write_dcm.doTest();

  IDcmLibApi::CheckMemory(1 /*del_tbl*/);
  //
  write_dcm.doTest();

  IDcmLibApi::CheckMemory(1 /*del_tbl*/);

}

 
void tstItem()
{
 
 CTstWrite write_dcm;

 write_dcm.init();
 
 IDcmLibApi::CheckMemory();

 write_dcm.dotestItem();
 
}