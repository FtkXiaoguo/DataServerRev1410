// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

 
#include "DcmXTAssociationListenerMain.h"

#include "DcmXTApplicationInstanceMain.h"

#include "DcmXTDicomMessageMain.h"
//////////////////
 
#define INCLUDE_CSTDLIB
#define INCLUDE_CSTRING
#include "dcmtk/ofstd/ofstdinc.h"

#include "AssociationHelp.h"

#include "DcmXTUtilMain.h"

#include "CheckMemoryLeak.h"
///
#include "FXDcmLibLogger.h"

 DcmXTAssociationListenerMain::DcmXTAssociationListenerMain()
{
	
	 m_ApplicationInstanceMain = 0;
	 

	m_serverHelper = new AssociationHelpServer;

}
DcmXTAssociationListenerMain::~DcmXTAssociationListenerMain()
{

	delete m_serverHelper;

 
}
 
void DcmXTAssociationListenerMain::Delete()
{
	delete this;
}
bool DcmXTAssociationListenerMain::abort()
{
//	return DcmXTAssociationMain::abort();
	return true;
	 
}

 
bool DcmXTAssociationListenerMain::waiting(const char*ServiceList,int portNum,int timeout,char *calledAE_Buff,int bufLen,DcmXtError &errorCode)
{
	 
	DCMLIB_LOG_TRACE("DcmXTAssociationListenerMain::waiting \n" ); 
	
	OFCondition cond = ECC_Normal;
	

	errorCode = DcmXtErr_Normal;
	if(!initNetwork()){
		DCMLIB_LOG_ERROR("DcmXTAssociationListenerMain::waiting initNetwork error \n" ); 
		return false;
	}

	

	/* initialize network, i.e. create an instance of T_ASC_Network*. */
	 m_opt_acse_timeout = 30;

	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded())
	{
		DCMLIB_LOG_WARN("Warning: no data dictionary loaded, check environment variable: %s\n",DCM_DICT_ENVIRONMENT_VARIABLE);
	}




	if(!m_ApplicationInstanceMain){
		DCMLIB_LOG_ERROR("DcmXTAssociationListenerMain::waiting invalide m_ApplicationInstanceMain \n" ); 
		return false;
	}

	m_ApplicationInstanceMain->createNetWork(portNum, timeout );
	//同じportNumなら、そのまま。

	T_ASC_Network *ASC_NetWork = m_ApplicationInstanceMain->getSCPAscNetwork();


 
	/*
	*  use m_associationTemp following 
	*/
	//
 	while (cond.good())
	{
		// do not destroy the previous m_associationTemp 2012/02/13 K.Ko
		// just set to 0
		m_associationTemp = 0;//

		/* receive an association and acknowledge or reject it. If the association was */
		/* acknowledged, offer corresponding services and invoke one or more if required. */
		cond = m_serverHelper->receiveAssociation(ASC_NetWork,m_associationTemp,timeout);


		
		if(m_associationTemp){
			//had received association
			break;
		}
		if(timeout>0) {
			errorCode = DcmXtErr_Timeout;
			 
			 break;
		}
    }
 

	if((m_associationTemp==0) || (errorCode!=DcmXtErr_Normal) ){
		DCMLIB_LOG_TRACE("DcmXTAssociationListenerMain::waiting return error \n" ); 	
		return false;
	}else{
		{//#10 2012/03/22 K.Ko
			char uidBuffer[128];
			uidBuffer[0] = 0;
			if(IDcmLibApi::Get_String_Config_Value(IMPLEMENTATION_CLASS_UID, sizeof(uidBuffer), uidBuffer)){
				if(strlen(uidBuffer)>1){
					strncpy(m_associationTemp->params->ourImplementationClassUID,
								uidBuffer,sizeof(m_associationTemp->params->ourImplementationClassUID)-1);
				//	 strcpy(m_associationTemp->params->DULparams.callingImplementationClassUID,
				//			m_associationTemp->params->ourImplementationClassUID);
				}
				//
			}
			uidBuffer[0] = 0;
			if(IDcmLibApi::Get_String_Config_Value(IMPLEMENTATION_VERSION, sizeof(uidBuffer), uidBuffer)){
				if(strlen(uidBuffer)>1){
					strncpy(m_associationTemp->params->ourImplementationVersionName,
						uidBuffer,sizeof(m_associationTemp->params->ourImplementationVersionName)-1);
				//	 strcpy(m_associationTemp->params->DULparams.callingImplementationVersionName,
				//			m_associationTemp->params->ourImplementationVersionName);
				}
				//
			}

		}

//		calledAE = m_associationTemp->params->DULparams.calledAPTitle;
		strncpy(calledAE_Buff,m_associationTemp->params->DULparams.calledAPTitle,bufLen);
	}
	 
	errorCode = DcmXtErr_Normal;

	DCMLIB_LOG_TRACE("DcmXTAssociationListenerMain::waiting return normal \n" ); 
	

	return true;
}
