// TstStoreSCU.cpp: CTstCStoreSCP クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <ctime>

#include "TstMt.h"
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


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
 
CTstMt::CTstMt()
{

	 
}

CTstMt::~CTstMt()
{

} 

void CTstMt::start(int id)
{
	// not use it here!

//	m_Logger.LogMessage(" create new AssociationHandler\n");
//	m_Logger.FlushLog();

	//別スレッド起動
	CMtHandler*  Handler = new CMtHandler(id );

	Handover(Handler);

	return  ;
}

 