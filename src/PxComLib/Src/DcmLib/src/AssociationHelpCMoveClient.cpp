//  
//
//////////////////////////////////////////////////////////////////////

 
#include "AssociationHelp.h"
#include "FXDcmLibLogger.h"
//////////////////
 

 #include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */

#include "CheckMemoryLeak.h"



/////////////
static int
selectReadable(
    T_ASC_Association *assoc,
    T_ASC_Network *net,
    T_ASC_Association *subAssoc,
    T_DIMSE_BlockingMode blockMode, int timeout)
{
    T_ASC_Association *assocList[2];
    int assocCount = 0;

    if (net != NULL && subAssoc == NULL) {
        if (ASC_associationWaiting(net, 0)) {
            /* association request waiting on network */
            return 2;
        }
    }
    assocList[0] = assoc;
    assocCount = 1;
    assocList[1] = subAssoc;
    if (subAssoc != NULL) assocCount++;
    if (subAssoc == NULL) {
        timeout = 1;            /* poll wait until an assoc req or move rsp */
    } else {
        if (blockMode == DIMSE_BLOCKING) {
            timeout = 10000;    /* a long time */
        }
    }
    if (!ASC_selectReadableAssociation(assocList, assocCount, timeout)) {
        /* none readable */
        return 0;
    }
    if (assocList[0] != NULL) {
        /* main association readable */
        return 1;
    }
    if (assocList[1] != NULL) {
        /* sub association readable */
        return 2;
    }
    /* should not be reached */
    return 0;
}


OFCondition
AssociationHelpClient::My_DIMSE_moveUser(
    /* in */
    T_ASC_Association *assoc,
	DcmXTSCPResponseParam *ResponseParam,
//    T_ASC_PresentationContextID presID,
//    T_DIMSE_C_MoveRQ *request,
    DcmDataset *requestIdentifiers
 //   DIMSE_MoveUserCallback callback, void *callbackData
    /* blocking info for response */
 //   T_DIMSE_BlockingMode blockMode, int timeout,
    /* sub-operation provider callback */
  //  T_ASC_Network *net,
  //  DIMSE_SubOpProviderCallback subOpCallback, void *subOpCallbackData,
    /* out */
//    T_DIMSE_C_MoveRSP *response, DcmDataset **statusDetail,
  //  DcmDataset **rspIds,
   // OFBool ignorePendingDatasets
   )
{
   
    
    
   
	///////////////////
	//K.KO
	T_DIMSE_Message &req = ResponseParam->m_reqMsg;
	T_ASC_PresentationContextID presID = ResponseParam->m_presIdCmd;
 	T_DIMSE_C_MoveRQ *request = &(ResponseParam->m_move_request);
	///////////////////

    if (requestIdentifiers == NULL) return DIMSE_NULLKEY;
    memset((char*)&req, 0, sizeof(req));
 //   bzero((char*)&req, sizeof(req));
  
    req.CommandField = DIMSE_C_MOVE_RQ;
    request->DataSetType = DIMSE_DATASET_PRESENT;
    req.msg.CMoveRQ = *request;

    

    OFCondition cond = DIMSE_sendMessageUsingMemoryData(assoc, presID, &req, NULL, requestIdentifiers, NULL, NULL);
    if (cond != EC_Normal) {
        return cond;
    }

	return cond;
}

OFCondition 
AssociationHelpClient::My_DIMSE_readCMoveResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,
					  DcmXTSCPResponseParam *ResponseParam,
//					  DIMSE_MoveUserCallback callback, void *callbackData,
					  T_DIMSE_BlockingMode blockMode, int timeout,
					  /* sub-operation provider callback */
						T_ASC_Network *net,
//						DIMSE_SubOpProviderCallback subOpCallback, void *subOpCallbackData,
						/* out */
					     
						DcmDataset **rspIds,
						OFBool ignorePendingDatasets)
 {
	 DIC_US msgId;
    
	  T_DIMSE_Message  rsp;
	 int responseCount = 0;
    T_ASC_Association *subAssoc = NULL;
	 
	 OFBool firstLoop = OFTrue;


	 ///////////////////
	//K.KO
	 T_ASC_PresentationContextID presID = ResponseParam->m_presIdCmd;

	 T_DIMSE_C_MoveRQ *request = &(ResponseParam->m_move_request);
	T_DIMSE_C_MoveRSP *response		= &(ResponseParam->m_move_response);
	DcmDataset  **statusDetail		= &(ResponseParam->m_statusDetail);
	//////////////////////
	 
	 DIC_US status = STATUS_Pending;
	 OFCondition cond = EC_Normal;
    /* receive responses */

	 msgId = request->MessageID;
     memset((char*)&rsp, 0, sizeof(rsp));
	 // bzero((char*)&rsp, sizeof(rsp));


    while (cond == EC_Normal && status == STATUS_Pending) {

        /* if user wants, multiplex between net/subAssoc
         * and move responses over main assoc.
         */
        switch (selectReadable(assoc, net, subAssoc, blockMode, timeout)) {
        case 0:
            /* none are readable, timeout */
            if ((blockMode == DIMSE_BLOCKING) || firstLoop) {
                firstLoop = OFFalse;
                continue;  /* continue with while loop */
            } else {
                return DIMSE_NODATAAVAILABLE;
            }
            /* break; */   // never reached after continue or return statement
        case 1:
            /* main association readable */
            firstLoop = OFFalse;
            break;
        case 2:
            /* net/subAssoc readable */
//            if (subOpCallback) {
//                subOpCallback(subOpCallbackData, net, &subAssoc);
 //           }
            firstLoop = OFFalse;
            continue;    /* continue with main loop */
            /* break; */ // never reached after continue statement
        }
		memset((char*)&rsp, 0, sizeof(rsp));
        //bzero((char*)&rsp, sizeof(rsp));

        cond = DIMSE_receiveCommand(assoc, blockMode, timeout, &presID, &rsp, statusDetail);
        if (cond != EC_Normal) {
            return cond;
        }
        if (rsp.CommandField != DIMSE_C_MOVE_RSP) {
            char buf1[256];
            sprintf(buf1, "DIMSE: Unexpected Response Command Field: 0x%x", (unsigned)rsp.CommandField);
            return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf1);
        }

        *response = rsp.msg.CMoveRSP;

        if (response->MessageIDBeingRespondedTo != msgId) {
            char buf2[256];
            sprintf(buf2, "DIMSE: Unexpected Response MsgId: %d (expected: %d)", response->MessageIDBeingRespondedTo, msgId);
            return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf2);
        }

        status = response->DimseStatus;
        responseCount++;

        switch (status) {
        case STATUS_Pending:
            if (*statusDetail != NULL) {
                DCMLIB_LOG_WARN("moveUser: Pending with statusDetail, ignoring detail \n");
                delete *statusDetail;
                *statusDetail = NULL;
            }
            if (response->DataSetType != DIMSE_DATASET_NULL) {
                DCMLIB_LOG_WARN("moveUser: Status Pending, but DataSetType!=NULL \n");
                if (! ignorePendingDatasets) {
                    // Some systems send an (illegal) dataset following C-MOVE-RSP messages
                    // with pending status, which is a protocol violation, but we need to
                    // handle this nevertheless. The MV300 has been reported to exhibit
                    // this behavior.
                    DCMLIB_LOG_WARN("Reading but ignoring response identifier set \n");
                    DcmDataset *tempset = NULL;
                    cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout, &presID, &tempset, NULL, NULL);
                    delete tempset;
                    if (cond != EC_Normal) {
                        return cond;
                    }
                } else {
                    // The alternative is to assume that the command set is wrong
                    // and not to read a dataset from the network association.
                    DCMLIB_LOG_WARN("Assuming NO response identifiers are present \n");
                }
            }

            /* execute callback */
//            if (callback) {
 //               callback(callbackData, request, responseCount, response);
  //          }
            break;
        default:
            if (response->DataSetType != DIMSE_DATASET_NULL) {
                cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout, &presID, rspIds, NULL, NULL);
                if (cond != EC_Normal) {
                    return cond;
                }
            }
            break;
        }
    }

    /* do remaining sub-association work, we may receive a non-pending
     * status before the sub-association has cleaned up.
     */
	/*
    while (subAssoc != NULL) {
        if (subOpCallback) {
            subOpCallback(subOpCallbackData, net, &subAssoc);
        }
    }

	*/
    return cond;
}