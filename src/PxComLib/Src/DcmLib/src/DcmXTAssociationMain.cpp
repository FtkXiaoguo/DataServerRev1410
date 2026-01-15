// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include<WinSock2.h>
#include "DcmXTAssociationMain.h"

//////////////////
 

#include "dcmtk/dcmnet/diutil.h"


#include "AssociationHelp.h"
///
#include "CheckMemoryLeak.h"

 DcmXTAssociationMain::DcmXTAssociationMain()
{
	
//	m_association = 0;
	
	//m_clientHelper = new AssociationHelpClient ;
	//m_serverHelper = new AssociationHelpServer;
//
 	m_ReceivedMsg = new T_DIMSE_Message;

	m_ResponseParam = new DcmXTSCPResponseParam;
//
//	m_ASC_NetWork = 0;
//	m_ASC_Params = 0;

}
DcmXTAssociationMain::~DcmXTAssociationMain()
{
//	delete m_clientHelper;
//	delete m_serverHelper;
 	delete m_ReceivedMsg;
	delete m_ResponseParam;
}

bool DcmXTAssociationMain::m_initWinSoket = false;

bool DcmXTAssociationMain::initNetwork()
{
#ifdef _WINDOWS

	if(!m_initWinSoket){
		WSADATA data;
		WSAStartup(MAKEWORD(2,0), &data);
		m_initWinSoket = true;
	}
#endif
	return true;
}

bool DcmXTAssociationMain::destroy()
{

	 
	return true;
}
bool  DcmXTAssociationMain::abort()
{
#if 0 //2012/02/13 K.Ko

	if(m_association){
		ASC_abortAssociation(m_association);
		return destroy();
	}
#endif

	return true;
}
	//
//

///