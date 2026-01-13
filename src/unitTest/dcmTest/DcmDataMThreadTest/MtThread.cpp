// AssociationHandler.cpp: CAssociationHandler クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MtThread.h"

#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#include "IDcmLibApi.h "
using namespace XTDcmLib;
#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif

#include "rtvloadoption.h"

 

#include "TstVLIDicomImage.h"
#include "TstVLIDicomMessage.h"
 
 
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////



CMtHandler::CMtHandler(int id )
{
	 
m_id = id;
m_count = 0;	 
}

CMtHandler::~CMtHandler()
{

}
int	CMtHandler::PreProcess(void) 
{
 m_count = 0; 
	return 0;
}
	
bool CMtHandler::readDicomMessage()
{
	char _fileName_[512];
	sprintf(_fileName_,"dcm\\test_dicom%d.dcm",m_id);

	CTstVLIDicomMessage dicomImage;
	bool load_header_only = true;
	bool ret_f = dicomImage.loadDicom(_fileName_,load_header_only);

	printf(" id[%d] - readDicomMessage [%d] ret_flag[%d] \n",m_id,m_count++,ret_f);

	sprintf(_fileName_,"out_dicom%d.dcm",m_id);
	dicomImage.saveDicom(_fileName_);

	return ret_f;
}
bool CMtHandler::readDicomImage()
{
 
	char _fileName_[512];
	sprintf(_fileName_,"dcm\\test_dicom%d.dcm",m_id%5+1);

	CTstVLIDicomImage dicomImage;
	bool ret_f = dicomImage.loadDicom(_fileName_);

	printf(" id[%d] - readDicomImage [%d] ret_flag[%d] \n",m_id,m_count++,ret_f);

	sprintf(_fileName_,"out_dicom%d.dcm",m_id);
	dicomImage.saveDicom(_fileName_);

	return ret_f;
}
int	CMtHandler::Process(void)
{
 
    readDicomMessage();
//	readDicomImage();
	return 0;
}

 