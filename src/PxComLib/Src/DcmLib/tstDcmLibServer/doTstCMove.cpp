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
  
int processCMove(int associationID,int messageID)
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

	 
	char	targetAETitle[16+1];	//	Where should we send to?
	char	targetIPAddress[66];
	int		targetPort;

	// Get the destination AE title so we know where to send the C-STORE messages
	mcStatus = MC_Get_Value_To_String (messageID, MC_ATT_MOVE_DESTINATION, sizeof(targetAETitle), targetAETitle);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
		printf("ParseCMoveRQMessage - Missing MoveDestination %d", C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN); 
		return(EXIT_FAILURE);
    }

	 

	/*
     *  Acquire a response message object, send it, and free it
     */
	int       finalCMoveRespMSG;//  rspMsgID;

	//	Message to be used in responses
	mcStatus = MC_Open_Empty_Message(&finalCMoveRespMSG);

  //  mcStatus = MC_Open_Message (&rspMsgID, ServiceName, C_STORE_RSP);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Open_Message failed", mcStatus);
        return(EXIT_FAILURE);
    }
 
	mcStatus = MC_Set_Service_Command(finalCMoveRespMSG, ServiceName, C_MOVE_RSP);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf(" %d, %d on MC_Set_Service_Command()\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

	//SendCMoveResponse(C_MOVE_SUCCESS_NO_FAILURES);


	mcStatus = MC_Send_Response_Message(associationID, C_MOVE_SUCCESS_NO_FAILURES, finalCMoveRespMSG);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf(" %d, %d on MC_Send_Response_Message()\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

    mcStatus = MC_Free_Message(&finalCMoveRespMSG);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Free_Message for response message failed", mcStatus);
//        MC_Abort_Association(&associationID);
        return(EXIT_FAILURE);
    }

}
