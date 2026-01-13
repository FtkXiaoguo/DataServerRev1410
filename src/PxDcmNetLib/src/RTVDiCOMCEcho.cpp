/***********************************************************************
 * $Id: RTVDiCOMCEcho.cpp 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes RTVDiCOMCEcho Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "PxDicomMessage.h"
#include "RTVDiCOMCEcho.h"

RTVDiCOMCEcho::RTVDiCOMCEcho(DiCOMConnectionInfo& connectInfo, int iMessageID):
	RTVDiCOMService(connectInfo, iMessageID)
{
	m_processorName = "RTVDiCOMCEcho";
}


//----------------------------------------------------------------------------------------------------
// Message with m_messageID(iMessageID) is freed here, it is better to be freed from where it is created.
// 
int RTVDiCOMCEcho::Process()
{
	MC_STATUS	status;
	int			rspMsgID;
	RESP_STATUS respStatus;
	int associationID = m_connectInfo.AssociationID;

	char uid[kMaxUID];
	LogMessage(kDebug,"DEBUG: RTVDiCOMCEcho::Process() - C-ECHO-RQ Received\n");

	status = MC_Get_Message_Service(m_messageID, &m_serviceName, &m_command);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Get_Value_To_String() - failed to get ServiceName from C-ECHO-RQ - Association %d: closed\n", status, associationID);
		//=====================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&m_messageID);
		//=====================================
		return status;
	}

	//
	//	Get the Affected SOP Class UID
	//
	status = MC_Get_Value_To_String(m_messageID, MC_ATT_AFFECTED_SOP_CLASS_UID, kMaxUID, uid);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Get_Value_To_String() - failed to get AffectedSOPClassUID from C-ECHO-RQ - Association %d: closed\n", status, associationID);
		//=====================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&m_messageID);
		//=====================================
		return status;
	}
	LogMessage(kDebug,"DEBUG: RTVDiCOMCEcho::Process() - C-ECHO-RQ Affected SOP Class UID = %s\n", uid);	

	//
	//	Get the MessageID
	//
	int messageID;
	status = MC_Get_Value_To_Int(m_messageID, MC_ATT_MESSAGE_ID, &messageID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Get_Value_To_Int() - failed to get MessageID from C-ECHO-RQ - Association %d: closed\n", status, associationID);
		//=====================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&m_messageID);
		//=====================================
		return status;
	}
	LogMessage(kDebug,"DEBUG: RTVDiCOMCEcho::Process() - C-ECHO-RQ MessageID = %d\n", messageID);

	//
	//	Get the Dataset Type
	//
	int datasetType;
	status = MC_Get_Value_To_Int(m_messageID, MC_ATT_DATA_SET_TYPE, &datasetType);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Get_Value_To_Int() - failed to get DatasetType from C-ECHO-RQ - Association %d: closed\n", status, associationID);
		//=====================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&m_messageID);
		//=====================================
		return status;
	}

	//
	//	Only valid Dataset Type
	//
	if (datasetType != 0x0101)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - Bad - DatasetType from C-ECHO-RQ - Expected 0x0101 - Got %h - Association %d: closed\n", status, datasetType, associationID);
		//=====================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&m_messageID);
		//=====================================
		return MC_PROTOCOL_ERROR;
	}

	//
	//	C-ECHO-RQ was OK.  Compose C-ECHO-RSP
	//
	//	status = MC_Open_Message(&rspMsgID, serviceName, C_ECHO_RSP);
	status = MC_Open_Empty_Message(&rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Open_Empty_Message() - failed to open empty response message for C-ECHO\n", status);
		// MC_Abort_Association(&m_associationID);
		//=====================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&m_messageID);
		//=====================================
		return status;
	}

	//=====================================
	// Added by Jwu 02_28_02
	// The Affected SOP Class UID is mandartory in request message but it is optional in CECHO response message.
	// Here we provide it 
	status = MC_Set_Value_From_String (rspMsgID, MC_ATT_AFFECTED_SOP_CLASS_UID, uid);
    if (status != MC_NORMAL_COMPLETION)
    {
        LogMessage(	kWarning, 
							"WARNING: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Set_Value_From_String() - Failed to Set Affected SOP Instance UID = %d for cEHCO response message\n",
						    status, uid);
		// no return here since it is optional.
    }

	// MessageID being responded to is Mandartoy in response message.
	status = MC_Set_Value_From_Int (rspMsgID, MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
       	LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Set_Value_From_String() - failed to set message ID for C-ECHO response message.\n", status);
		MC_Free_Message(&rspMsgID);
		MC_Free_Message(&m_messageID);
		return status; 
    }

	//=======================================

	status = MC_Set_Service_Command(rspMsgID, m_serviceName, C_ECHO_RSP);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Set_Service_Command() - failed to set Service and command for response message for C-ECHO. \n", status );
		// MC_Abort_Association(&m_associationID);
		//=================================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&rspMsgID);
		MC_Free_Message(&m_messageID);
		//==================================================
		return status;
	}

	//
	//	Send response message
	//
	respStatus = C_ECHO_SUCCESS;
	status = MC_Send_Response_Message(associationID, respStatus, rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Send_Response_Message() - failed to send response message for C-ECHO.\n", status );
		//MC_Abort_Association(&m_associationID);
		//=================================================
		// Added by Jwu 02_28_02
		MC_Free_Message(&rspMsgID);
		MC_Free_Message(&m_messageID);
		//==================================================
		return status;
	}

	//=================================================
	// Added by Jwu 02_28_02 
	LogMessage(kDebug,"DEBUG: RTVDiCOMCEcho::Process() - C-ECHO-RQ response message has been sent successfully.\n");
	//==================================================

	//
	//	Free response message
	//
	status = MC_Free_Message(&rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage(kWarning,  "WARNING: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Send_Response_Message() - failed to free response message for C-ECHO - Association %d: closed\n", status, associationID);
		//=================================================
		// Added by Jwu 02_28_02 
		MC_Free_Message(&m_messageID);
		//==================================================
		return status;
	}

 
	//
	//	Free message
	//
	MC_Free_Message(&m_messageID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage(kWarning, "WARNING: RTVDiCOMCEcho::Process() - DcmLib error %d on MC_Send_Response_Message() - failed to free C-ECHO message - Association %d: closed\n", status, associationID);
		return status;
	}
	 

	return MC_NORMAL_COMPLETION;
}
