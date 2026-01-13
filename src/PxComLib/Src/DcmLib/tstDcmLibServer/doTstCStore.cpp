// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

 
#ifdef USE_NEW_LIB

#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
  
int processCStore(int associationID,int messageID)
{
	
	MC_STATUS mcStatus;
	char  *ServiceName;
	MC_COMMAND    Command;
	/*
	*
	*/

	//	Get m_serviceName so we can send it out in response messages
	mcStatus = MC_Get_Message_Service(messageID, &ServiceName, &Command);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf("ERROR: (%d) - CFind::Process() - Merge error (%d,%s) - Couldn't get Message Service\n", associationID, mcStatus, MC_Error_Message(mcStatus));
		return(EXIT_FAILURE);
	}
	/*
     *  Acquire a response message object, send it, and free it
     */
	int         rspMsgID;
    mcStatus = MC_Open_Message (&rspMsgID, ServiceName, C_STORE_RSP);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Open_Message failed", mcStatus);
        return(EXIT_FAILURE);
    }

	RESP_STATUS respStatus = C_STORE_SUCCESS;

    mcStatus = MC_Send_Response_Message(associationID, respStatus, rspMsgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Send_Response_Message failed", mcStatus);
     //   MC_Abort_Association(&associationID);
        MC_Free_Message(&rspMsgID);
       return(EXIT_FAILURE);
    }
  
    mcStatus = MC_Free_Message(&rspMsgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Free_Message for response message failed", mcStatus);
//        MC_Abort_Association(&associationID);
        return(EXIT_FAILURE);
    }

}
