// TstVLIDicomMessage.cpp: CTstVLIDicomMessage クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TstVLIDicomMessage.h"

#ifdef USE_NEW_LIB
#include "PxDicomMessage.h"
#include "IDcmLibApi.h "
#else
#include "VLIDicomMessage.h"
#include "rtvMergeToolKit.h "
#endif
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTstVLIDicomMessage::CTstVLIDicomMessage()
{
 
#ifdef USE_NEW_LIB
 
	m_DicomImage = new CPxDicomMessage;
#else
	m_DicomImage = new VLIDicomMessage;;
 
#endif
}

CTstVLIDicomMessage::~CTstVLIDicomMessage()
{
	if(m_DicomImage) delete m_DicomImage;

}
bool CTstVLIDicomMessage::loadDicom(const char *filename,bool bHeaderOnly )
{
 
  if(kNormalCompletion != m_DicomImage->Load(filename,bHeaderOnly) )
	{
		return false;
	}
  return true;
}
void CTstVLIDicomMessage::saveDicom(const char *filename)
{
	if(!m_DicomImage) return;
#ifdef USE_NEW_LIB
	PxDicomStatus status = m_DicomImage->Save(filename);
#else
	VLIDicomStatus status = m_DicomImage->Save(filename);
#endif

}