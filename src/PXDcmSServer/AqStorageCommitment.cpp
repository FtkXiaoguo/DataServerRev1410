/***********************************************************************
 * AqStorageCommitment.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Process Storage Commitment Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */

#pragma warning (disable: 4786)
#pragma warning (disable: 4616)


#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#include "PxDicomutil.h"

#else
#include "rtvMergeToolkit.h"
#endif

//#include "TRDICOMUtil.h"
#include "AqStorageCommitment.h"

const char* kStorageCommitmentServiceName = "STORAGE_COMMITMENT_PUSH";

//-----------------------------------------------------------------------------
//
AqStorageCommitment::AqStorageCommitment(DiCOMConnectionInfo& connectInfo, int iMessageID):
	   RTVDiCOMService(connectInfo, iMessageID)
{
	m_state = kInitialized;
}

//-----------------------------------------------------------------------------
//
AqStorageCommitment::~AqStorageCommitment()
{
	m_state = kEnterDestructor;

	CleanUp();

	m_state = kLeaveDestructor;
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::PreProcess()
{
	m_state = kEnterPreprocess;

	m_state = kLeavePreprocess;
	return 0;
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::Process()
{
	m_state = kEnterProcess;

	int mcStatus;
	std::string transactionUID = "";

	mcStatus = ParseNActionMessage(transactionUID);

	//	Send NActionRsp message - success if Parse was successful, failure otherwise.
	mcStatus = SendNActionRsp(mcStatus);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - SendNActionRsp() failed", mcStatus);
        return mcStatus;
    }

	if (transactionUID.size() < 1)
	{
        GetAqLogger()->LogMessage("ERROR: ParseNActionMessage appeared to succeed, but produced zero length transactionUID...cannot proceed");
        return (mcStatus);
	}

	mcStatus = HandleNAction(m_mapTranactions[transactionUID]);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - HandleNAction failed", mcStatus);
        return (mcStatus);
    }
	
	m_state = kLeaveProcess;
	return 0;
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::ParseNActionMessage(std::string& oTransactionUID)
{
    MC_STATUS mcStatus;
    int itemID;
    char uid[65];
	NAction nAct;
	oTransactionUID = "";

	//	Remember who connected to us
	//	TODO: handle case where N-EVENT-REPORT-RQ message is sent over a different association
    nAct.m_remoteAE = m_connectInfo.RemoteApplicationTitle;

	//	Remember the transaction UID so we can correlate messages with this transaction
    mcStatus = MC_Get_Value_To_String(m_messageID, MC_ATT_TRANSACTION_UID, sizeof uid, uid);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - MC_Get_Value_To_String for Transaction UID failed", mcStatus);
        return mcStatus;
    }

	//	TODO: add transactionUID to all relevant log messages
	//	TODO: add kInfo level log messages for each step in the StorageCommitment transaction.
	nAct.m_transactionUID = uid;

	//	These are the instances we are being asked to commit to store.  Later, we have to check each one
	//		to make sure we actually have (and will keep) them.
    mcStatus = MC_Get_Value_To_Int(m_messageID, MC_ATT_REFERENCED_SOP_SEQUENCE, &itemID);
    while (mcStatus == MC_NORMAL_COMPLETION)
    {
		SOPInstanceDescriptor sopDescr;
        mcStatus = MC_Get_Value_To_String(itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, sizeof uid, uid);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            GetAqLogger()->LogMessage("ERROR: %d - Unable to get SOP Class UID in n-action message", mcStatus);
            return mcStatus;
        }
		sopDescr.m_sopClassUID = uid;

        mcStatus = MC_Get_Value_To_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, sizeof uid, uid);
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            GetAqLogger()->LogMessage("ERROR: %d - Unable to get SOP Instance UID in n-action message", mcStatus);
            return mcStatus;
        }
		sopDescr.m_sopInstanceUID = uid;
		nAct.m_vSOPInstances.push_back(sopDescr);

        mcStatus = MC_Get_Next_Value_To_Int(m_messageID, MC_ATT_REFERENCED_SOP_SEQUENCE, &itemID);
    }
	
	//	NOTE: it's probably not necessary to have a map of these here.  This seems more necessary on the
	//		SCU side, where N_EVENT_REPORT_RQ messages have to be correlated with the matching N_ACTION_RQ message.
	//		For now, it doesn't hurt...maybe there will be threading etc...
	m_mapTranactions[nAct.m_transactionUID] = nAct;
	oTransactionUID = nAct.m_transactionUID;

    return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::SendNActionRsp(int iStatus)
{
    int rspMessageID;
	MC_STATUS mcStatus;

    mcStatus = MC_Open_Message (&rspMessageID, kStorageCommitmentServiceName, N_ACTION_RSP);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - MC_Open_Message error for n-action-rsp", mcStatus);
        return mcStatus;
    }

	/*
	#define N_ACTION_CLASS_INSTANCE_CONFLICT        ((RESP_STATUS)0x0119)
	#define N_ACTION_DUPLICATE_INVOCATION           ((RESP_STATUS)0x0210)
	#define N_ACTION_INVALID_ARGUMENT_VALUE         ((RESP_STATUS)0x0115)
	#define N_ACTION_MISTYPED_ARGUMENT              ((RESP_STATUS)0x0212)
	#define N_ACTION_NO_SUCH_ARGUMENT               ((RESP_STATUS)0x0114)
	#define N_ACTION_NO_SUCH_SOP_CLASS              ((RESP_STATUS)0x0118)
	#define N_ACTION_NO_SUCH_SOP_INSTANCE           ((RESP_STATUS)0x0112)
	#define N_ACTION_PROCESSING_FAILURE             ((RESP_STATUS)0x0110)
	#define N_ACTION_RESOURCE_LIMITATION            ((RESP_STATUS)0x0213)
	#define N_ACTION_UNRECOGNIZED_OPERATION         ((RESP_STATUS)0x0211)
	#define N_ACTION_NO_SUCH_ACTION_TYPE            ((RESP_STATUS)0x0123)
	*/

	int statusCode = (iStatus == MC_NORMAL_COMPLETION) ? N_ACTION_SUCCESS : N_ACTION_PROCESSING_FAILURE;
	if (statusCode != N_ACTION_SUCCESS)
	{
		//	Add comment into the rsp msg indicating what error code we failed on
		//		useful if we get network capture...
		char msg[1024];
		_snprintf(msg, sizeof msg, "Failure code = %d", iStatus);
		MC_Set_Value_From_String(rspMessageID, MC_ATT_ERROR_COMMENT, msg);
	}

    mcStatus = MC_Send_Response_Message(m_connectInfo.AssociationID, statusCode, rspMessageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - MC_Send_Response_Message error", mcStatus);
        MC_Free_Message(&rspMessageID);
        return mcStatus;
    }

	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::HandleNAction(NAction& iAction)
{
	//	TODO: Need to do something here - like verify that we actually have
	//		each of the instances we are comitting to store, and can supply them
	//		to C-MOVE requesters.  Also, something about ensuring long-term storage.
	//
	//		1. Lock the studies containing the instances (prevent autodelete)
	//		2. Check db and filesize.  Currently, db does not store filesize, but pixelOffset + dataSize is close

	int mcStatus;

	mcStatus = SendNEventMessage(iAction);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - SendNEventMessage() failed", mcStatus);
        return mcStatus;
    }

	mcStatus = ReceiveNEventResponse();
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d - ReceiveNEventResponse() failed", mcStatus);
        return mcStatus;
    }

	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::SendNEventMessage(NAction& iAction)
{
    MC_STATUS      mcStatus;
    int            messageID;
    int            itemID;

    mcStatus = MC_Open_Message(&messageID, kStorageCommitmentServiceName, N_EVENT_REPORT_RQ);
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        GetAqLogger()->LogMessage("ERROR: %d - opening Storage Commitment n-event-report-rq message",mcStatus);
        return mcStatus;
    }

    // Set the well-known SOP instanceUID for storage commitment Push, as listed in DICOM PS3.4, J.3.5
    mcStatus = MC_Set_Value_From_String(messageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, "1.2.840.10008.1.20.1.1");
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d MC_Set_Value_From_String for requested sop instance uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return mcStatus;
    }

	//	TODO: Need to handle the failed case, where EVENT_TYPE_ID is set to 2, and :
	//		1. REFERENCED_SOP_SEQUENCE contains a list of successful SOP Instance UIDs
	//		2. FAILED_SOP_SEQUENCE contains a list of failed SOP Insatance UIDs
	//	See DICOM PS3.4, J.3.3.1.1
    mcStatus = MC_Set_Next_Value_From_Int(messageID, MC_ATT_EVENT_TYPE_ID, 1);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d Unable to set event type ID in n-action message", mcStatus);
        MC_Free_Message(&messageID);
        return mcStatus;
    }

    mcStatus = MC_Set_Value_From_String(messageID, MC_ATT_TRANSACTION_UID, iAction.m_transactionUID.c_str());
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d MC_Set_Value_From_String for transaction uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return mcStatus;
    }

	//	RETRIEVE_AE_TITLE is set to the AE title remote hosts should send C-MOVE-RQ messages to in order to retrieve these SOP instances.
    mcStatus = MC_Set_Value_From_String(messageID, MC_ATT_RETRIEVE_AE_TITLE, TRDICOMUtil::CalculateInboundLocalAETitle().c_str());
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d MC_Set_Value_From_String for transaction uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return mcStatus;
    }

    for(int i = 0; i < iAction.m_vSOPInstances.size(); i++)
    {
        mcStatus = MC_Open_Item(&itemID, "REF_SOP_MEDIA_AE");
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            MC_Free_Item(&itemID);
            MC_Free_Message(&messageID);
            return mcStatus;
        }
        
        mcStatus = MC_Set_Next_Value_From_Int(messageID, MC_ATT_REFERENCED_SOP_SEQUENCE, itemID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            GetAqLogger()->LogMessage("ERROR: %d Unable to set ItemID in n-event message", mcStatus);
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return mcStatus;
        }
        
        mcStatus = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, iAction.m_vSOPInstances[i].m_sopClassUID.c_str());
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            GetAqLogger()->LogMessage("ERROR: %d Unable to set SOP Class UID in n-event message", mcStatus);
            MC_Free_Message( &messageID );
            return mcStatus;
        }
        
        mcStatus = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, iAction.m_vSOPInstances[i].m_sopInstanceUID.c_str());
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            GetAqLogger()->LogMessage("ERROR: %d Unable to set SOP Instance UID in n-event message", mcStatus);
            MC_Free_Message( &messageID );
            return mcStatus;
        }   
    }
   
    mcStatus = MC_Send_Request_Message(m_connectInfo.AssociationID, messageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d Unable to send N-ACTION-RQ message",mcStatus);
        MC_Free_Message( &messageID );
        return mcStatus;
    }

    mcStatus = MC_Free_Message(&messageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d Unable to free N-ACTION-RQ message",mcStatus);
        return mcStatus;
    }
    

    return(MC_NORMAL_COMPLETION);
}

//-----------------------------------------------------------------------------
//
int AqStorageCommitment::ReceiveNEventResponse()
{
	MC_STATUS	   mcStatus;
    int            responseMessageID;
    char*          responseService;
    MC_COMMAND     responseCommand;
    int            responseStatus;

	//	TODO: replace 30 second timeout with appropriate configured timeout...but have to do it in a loop
	//		so we can check for shutdown
    mcStatus = MC_Read_Message(m_connectInfo.AssociationID, 30, &responseMessageID, &responseService, &responseCommand);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d MC_Read_Message failed for N-EVENT-REPORT-RSP", mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_Int(responseMessageID, MC_ATT_STATUS, &responseStatus);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: %d MC_Get_Value_To_Int for N-EVENT-REPORT-RSP status failed",mcStatus);
        MC_Free_Message(&responseMessageID);
        return mcStatus;
    }

    switch (responseStatus)
    {
        case N_EVENT_PROCESSING_FAILURE:
            GetAqLogger()->LogMessage("ERROR: N-EVENT-REPORT-RSP failed because of processing failure\n");
            MC_Free_Message(&responseMessageID);
            return mcStatus;
        case N_EVENT_SUCCESS:
            GetAqLogger()->LogMessage(kInfo, "ERROR: N-EVENT-REPORT message received with SUCCESS status\n");
            break;
        default:
            GetAqLogger()->LogMessage("ERROR: N-EVENT-REPORT-RSP failure, status=%x\n",responseStatus);
            MC_Free_Message(&responseMessageID);
            return mcStatus;
    }

    mcStatus = MC_Free_Message(&responseMessageID);
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        GetAqLogger()->LogMessage("ERROR: %d Unable to free N-EVENT_REPORT-RSP message",mcStatus);
        return mcStatus;
    }

	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
void AqStorageCommitment::CleanUp()
{
	m_mapTranactions.clear();
}

//-----------------------------------------------------------------------------
//
void AqStorageCommitment::LogProcessStatus(void)
{
	char *pState = "Unknown";
	switch(m_state)
	{
		case kInitialized:
			pState = "Initialized";
			break;
		case kEnterPreprocess:
			pState = "EnterPreprocess";
			break;
		case kLeavePreprocess:
			pState = "LeavePreprocess";
			break;
		case kEnterProcess:
			pState = "Enter Process";
			break;
		case kLeaveProcess:
			pState = "Leave Process";
			break;
		case kEnterDestructor:
			pState = "Enter Destructor";
			break;
		case kLeaveDestructor:
			pState = "Leave Destructor";
			break;
	}
	LogMessage("STATE_INFO: %s in state: %s\n", m_processorName, pState);
}
