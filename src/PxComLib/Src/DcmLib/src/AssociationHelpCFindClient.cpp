//  
//
//////////////////////////////////////////////////////////////////////

 
#include "AssociationHelp.h"
#include "FXDcmLibLogger.h"
//////////////////
 
#define INCLUDE_CSTDLIB
#define INCLUDE_CSTRING

 #include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */


#include "CheckMemoryLeak.h"

void AssociationHelpClient::FindProgressCallback(
    void *callbackData,
        T_DIMSE_C_FindRQ *request,      /* original find request */
        int responseCount,
        T_DIMSE_C_FindRSP *response,    /* pending response received */
        DcmDataset *responseIdentifiers /* pending response identifiers */)
{

}


typedef struct {
    void *callbackData;
    T_DIMSE_StoreProgress *progress;
    T_DIMSE_C_StoreRQ *request;
    DIMSE_StoreUserCallback callback;
} DIMSE_PrivateUserContext;

static void 
privateUserCallback(void *callbackData, unsigned long bytes)
{
    DIMSE_PrivateUserContext *ctx;
    ctx = (DIMSE_PrivateUserContext*)callbackData;
    ctx->progress->state = DIMSE_StoreProgressing;
    ctx->progress->progressBytes = bytes;
    ctx->progress->callbackCount++;
    if (ctx->callback) {
        ctx->callback(ctx->callbackData, ctx->progress, ctx->request);
    }
}



OFCondition
AssociationHelpClient::My_DIMSE_findUser(
        T_ASC_Association *assoc,
		DcmXTSCPResponseParam *ResponseParam,
  //      T_ASC_PresentationContextID presID,
  //      T_DIMSE_C_FindRQ *request, 
		DcmDataset *requestIdentifiers,
 //       DIMSE_FindUserCallback callback, void *callbackData,
  //      T_DIMSE_BlockingMode blockMode, int timeout,
   //     T_DIMSE_C_FindRSP *response, DcmDataset **statusDetail
   T_DIMSE_DetectedCancelParameters *checkForCancelParams
		)
    /*
     * This function sends a C-FIND-RQ message and data set information containing the given
     * search mask over the network connection to an SCP. Having sent this information, the
     * function tries to receive (one or more) C-FIND-RSP messages on the network connection
     * from the SCP. For C-FIND-RSP messages with a "pending" status (in element (0000,0900))
     * the function will also try to receive data set information over the network (this data
     * set information refers to one record that matches the search mask which was sent to the
     * SCP) and call the callback function which was passed. Having encountered a C-FIND-RSP
     * messages with a "success" status, this function terminates and returns to its caller.
     *
     * Parameters:
     *   assoc                - [in] The association (network connection to SCP).
     *   presId               - [in] The ID of the presentation context which shall be used
     *   request              - [in] Represents a DIMSE C-Find Request Message. Contains corresponding
     *                               information, e.g. message ID, affected SOP class UID, etc.
     *   requestIdentifiers   - [in] Data set object which contains the search mask,i.e. which specifies
     *                               the set of objects which will be requested by C-FIND-RQ.
     *   callback             - [in] Pointer to a function which shall be called to indicate progress.
     *   callbackData         - [in] Pointer to data which shall be passed to the progress indicating function
     *   blockMode            - [in] The blocking mode for receiving data (either DIMSE_BLOCKING or DIMSE_NONBLOCKING)
     *   timeout              - [in] Timeout interval for receiving data. If the blocking mode is DIMSE_NONBLOCKING
     *   response             - [out] Represents a DIMSE C-Find Response Message. Contains corresponding
     *                                information, e.g. message ID being responded to, affected SOP class UID, etc.
     *                                This variable contains in the end the last C-FIND-RSP message which was received
     *                                as a response to the C-FIND-RQ which was sent. Usually, this message will show a
     *                                status of "success".
     *   statusDetail         - [out] If a non-NULL value is passed this variable will in the end contain detailed
     *                                information with regard to the status information which is captured in the status
     *                                element (0000,0900) of the response message. Note that the value for element (0000,0900)
     *                                is not contained in this return value but in response.
     */
{
      
   
	///////////////////
	//K.KO
	T_DIMSE_Message &req = ResponseParam->m_reqMsg;
	T_ASC_PresentationContextID presID = ResponseParam->m_presIdCmd;
 	T_DIMSE_C_FindRQ *request = &(ResponseParam->m_find_request);
	///////////////////

    /* if there is no search mask, nothing can be searched for */
	if (requestIdentifiers == NULL) {
		DCMLIB_LOG_ERROR("My_DIMSE_findUser requestIdentifiers == NULL \n" );
		return DIMSE_NULLKEY;
	}

    /* initialize the variables which represent DIMSE C-FIND-RQ and DIMSE C-FIND-RSP messages */
    bzero((char*)&req, sizeof(req));
//    bzero((char*)&rsp, sizeof(rsp));

    /* set corresponding values in the request message variable */
    req.CommandField = DIMSE_C_FIND_RQ;
    request->DataSetType = DIMSE_DATASET_PRESENT;
    req.msg.CFindRQ = *request;

   

    /* send C-FIND-RQ message and search mask data (requestIdentifiers) */
    OFCondition cond = DIMSE_sendMessageUsingMemoryData(assoc, presID, &req,
                                          NULL, requestIdentifiers,
                                          NULL, NULL);
     return cond;
}

OFCondition
AssociationHelpClient::My_DIMSE_readCFindResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,DcmDataset **dataset_out,
					  DcmXTSCPResponseParam *ResponseParam,
					  T_DIMSE_BlockingMode blockMode, 
					  int timeout,
					  T_DIMSE_DetectedCancelParameters *checkForCancelParams)
{
	 int responseCount = 0;
 
	DIC_US msgId;
	DIC_US status = STATUS_Pending;

	OFCondition cond = EC_Normal;

	   
//	DcmDataset *rspIds = NULL;
//	*dataset_out = NULL; //do not clear it
 
	T_DIMSE_C_FindRSP *response	= &(ResponseParam->m_find_response);
	T_DIMSE_C_FindRQ *request		= &(ResponseParam->m_find_request);

	 /* determine the message ID */
    msgId = request->MessageID;

    /* try to receive (one or more) C-STORE-RSP messages, continue loop as long */
    /* as no error occured and not all result information has been received. */

	bool do_once_flag = true;
    while (cond == EC_Normal && DICOM_PENDING_STATUS(status))
    {
		if(!do_once_flag) {
			break;
		}
		do_once_flag = false;
	/* initialize the response to collect */
        bzero((char*)msg_out, sizeof(*msg_out));
//        if (rspIds != NULL) {
//            delete rspIds;
 //           rspIds = NULL;
 //       }

        /* try to receive a C-FIND-RSP over the network. */
        cond = DIMSE_receiveCommand(assoc, blockMode, timeout, &(ResponseParam->m_presIdCmd),
                msg_out, &(ResponseParam->m_statusDetail));
        if (cond.bad()) return cond;

        /* if everything was successful so far, the rsp variable contains the command */
        /* which was received; if we did not encounter a C-FIND-RSP, something is wrong */
        if (msg_out->CommandField != DIMSE_C_FIND_RSP)
        {
          char buf1[256];
          sprintf(buf1, "DIMSE: Unexpected Response Command Field: 0x%x", (unsigned)msg_out->CommandField);
          return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf1);
        }

        /* if we get to here, we received a C-FIND-RSP; store this message in the reference parameter */
        *response = msg_out->msg.CFindRSP;

        /* check if the response relates to the request which was sent earlier; if not, return an error */
        if (response->MessageIDBeingRespondedTo != msgId)
        {
          char buf2[256];
          sprintf(buf2, "DIMSE: Unexpected Response MsgId: %d (expected: %d)", response->MessageIDBeingRespondedTo, msgId);
          return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf2);
        }

        /* determine the status which was returned in the current C-FIND-RSP */
        status = response->DimseStatus;

        /* increase counter which counts the amount of received C-FIND-RSP messages */
        responseCount++;

        /* depending on the status which was returned in the current C-FIND-RSP, we need to do something */
        switch (status) {
        case STATUS_Pending:
        case STATUS_FIND_Pending_WarningUnsupportedOptionalKeys:
            /* in these cases we received a C-FIND-RSP which indicates that a result data set was */
            /* found and will be sent over the network. We need to receive this result data set. */

            /* forget about status detail information if there is some */
            if ((ResponseParam->m_statusDetail) != NULL) {
                DCMLIB_LOG_WARN("findUser: Pending with statusDetail, ignoring detail \n");
                delete (ResponseParam->m_statusDetail);
                (ResponseParam->m_statusDetail) = NULL;
            }

            /* if the response message's data set type field reveals that there is */
            /* no data set attached to the current C-FIND-RSP, something is fishy */
            if (response->DataSetType == DIMSE_DATASET_NULL) {
                DCMLIB_LOG_WARN("findUser: Status Pending, but DataSetType==NULL \n");
                DCMLIB_LOG_WARN("  Assuming response identifiers are present \n");
            }

            /* receive the result data set on the network connection */
            cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout,
                &(ResponseParam->m_presIdCmd), dataset_out, NULL, NULL);
            if (cond != EC_Normal) {
                return cond;
            }

            /* execute callback */

            break;
        case STATUS_Success:
            /* in this case the current C-FIND-RSP indicates that */
            /* there are no more records that match the search mask */

            /* in case the response message's data set type field reveals that there */
            /* is a data set attached to the current C-FIND-RSP, something is fishy */
            if (response->DataSetType != DIMSE_DATASET_NULL) {
                DCMLIB_LOG_WARN("findUser: Status Success, but DataSetType!=NULL \n");
                DCMLIB_LOG_WARN("Assuming no response identifiers are present \n");
            }
            break;
        default:
            /* in all other cases, simply dump warnings if there is no data set */
            /* following the C-FIND-RSP message, do nothing else (but go ahead  */
            /* and try to receive the next C-FIND-RSP) */
            if (response->DataSetType != DIMSE_DATASET_NULL) {
				const char *status_temp = DU_cfindStatusString(status);
				if(!status_temp) status_temp = " ";
                DCMLIB_LOG_WARN("findUser: Status %s, but DataSetType!=NULL \n",status_temp  );
                DCMLIB_LOG_WARN("Assuming no response identifiers are present \n");
            }
            break;
        } //switch
    } //while

    /* return result value */
    return cond;
}
