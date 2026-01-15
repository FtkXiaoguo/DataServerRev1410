//  
//
//////////////////////////////////////////////////////////////////////

 
#include "AssociationHelp.h"
#include "FXDcmLibLogger.h"
//////////////////
 


 #include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */

#include "CheckMemoryLeak.h"

extern  OFBool opt_verbose_progressCallback ;

void
AssociationHelpClient::StoreProgressCallback(void * /*callbackData*/,
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ * /*req*/)
{
    if (opt_verbose_progressCallback) {
        switch (progress->state) {
        case DIMSE_StoreBegin:
            DCMLIB_LOG_TRACE("XMIT: \n"); break;
        case DIMSE_StoreEnd:
            DCMLIB_LOG_TRACE("\n"); break;
        default:
   //         putchar('.'); 
			break;
        }
   //     fflush(stdout);
    }
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
AssociationHelpClient::My_DIMSE_storeUser(
	T_ASC_Association *assoc, 
//	T_ASC_PresentationContextID presId,
//	T_DIMSE_C_StoreRQ *request,
	DcmXTSCPResponseParam *ResponseParam,
//	const char *imageFileName, 
	DcmDataset *imageDataSet,
	DIMSE_StoreUserCallback callback, void *callbackData,
//	T_DIMSE_BlockingMode blockMode, 
//	int timeout,
//	T_DIMSE_C_StoreRSP *response,
//	DcmDataset **statusDetail,
    T_DIMSE_DetectedCancelParameters *checkForCancelParams,
    long imageFileTotalBytes)
    /*
     * This function transmits data from a file or a dataset to an SCP. The transmission is
     * conducted via network and using DIMSE C-STORE messages. Additionally, this function
     * evaluates C-STORE-Response messages which were received from the SCP.
     * 
     * Parameters:
     *   assoc                - [in] The association (network connection to SCP).
     *   presId               - [in] The ID of the presentation context which shall be used
     *   request              - [in] Represents a DIMSE C-Store Request Message. Contains corresponding
     *                               information, e.g. message ID, affected SOP class UID, etc.
     *   imageFileName        - [in] The name of the file which is currently processed.
     *   imageDataSet         - [in] The data set which is currently processed.
     *   callback             - [in] Pointer to a function which shall be called to indicate progress.
     *   callbackData         - [in] Pointer to data which shall be passed to the progress indicating function
     *   blockMode            - [in] The blocking mode for receiving data (either DIMSE_BLOCKING or DIMSE_NONBLOCKING)
     *   timeout              - [in] Timeout interval for receiving data. If the blocking mode is DIMSE_NONBLOCKING
     *   response             - [out] Represents a DIMSE C-Store Response Message. Contains corresponding
     *                                information, e.g. message ID being responded to, affected SOP class UID, etc.
     *                                This variable contains in the end the C-STORE-RSP command which was received
     *                                as a response to the C-STORE-RQ which was sent.
     *   statusDetail         - [out] If a non-NULL value is passed this variable will in the end contain detailed
     *                                information with regard to the status information which is captured in the status
     *                                element (0000,0900) of the response message. Note that the value for element (0000,0900)
     *                                is not contained in this return value but in response.
     *   checkForCancelParams - [out] Indicates, if a C-Cancel (Request) Message was encountered. Contains corresponding
     *                                information, e.g. a boolean value if a corresponding message was encountered and the
     *                                C-Cancel (Request) Message itself (in case it actually was encountered).
     *   imageFileTotalBytes  - [in] The size of the file which is currently processed in bytes.
     */
{
    OFCondition cond = EC_Normal;
    T_DIMSE_Message   rsp;
    DIMSE_PrivateUserContext callbackCtx;
    DIMSE_ProgressCallback privCallback = NULL;
    T_DIMSE_StoreProgress progress;
    progress.callbackCount = 0;

	///////////////////
	//K.KO
	T_DIMSE_Message &req = ResponseParam->m_reqMsg;
	T_ASC_PresentationContextID presId = ResponseParam->m_presIdCmd;
 	T_DIMSE_C_StoreRQ *request = &(ResponseParam->m_store_request);
	///////////////////
    /* if there is no image file or no data set, no data can be sent */
	if (imageDataSet == NULL) {
		DCMLIB_LOG_ERROR("My_DIMSE_storeUser imageDataSet == NULL \n");
		return DIMSE_NULLKEY;
	}
    
    /* initialize the variables which represent DIMSE C-STORE request and DIMSE C-STORE response messages */
	memset((char*)&req, 0, sizeof(req));
	memset((char*)&rsp, 0, sizeof(rsp));
    //bzero((char*)&req, sizeof(req));
    //bzero((char*)&rsp, sizeof(rsp));

    /* set corresponding values in the request message variable */
    req.CommandField = DIMSE_C_STORE_RQ;
    request->DataSetType = DIMSE_DATASET_PRESENT;
    req.msg.CStoreRQ = *request;

    /* set up callback routine which is used to indicate progress */
    if (callback != NULL) {
        /* in case the caller indicated that he has his own progress indicating */
        /* function set some variables correspondingly so that this particular */
        /* function will be called whenever progress shall be indicated. */
        privCallback = privateUserCallback;	/* function defined above */
		callbackCtx.callbackData = callbackData;
        progress.state = DIMSE_StoreBegin;
		progress.callbackCount = 1;
		progress.progressBytes = 0;
		if (imageFileTotalBytes > 0) {
			progress.totalBytes = imageFileTotalBytes; 
		}else{
//			if (imageFileName != NULL) {
//				progress.totalBytes = OFStandard::getFileSize(imageFileName);
//			}else {
				progress.totalBytes = dcmGuessModalityBytes(request->AffectedSOPClassUID);
//			}
        }
		callbackCtx.progress = &progress;
		callbackCtx.request = request;
        callbackCtx.callback = callback;
		/* execute initial callback */
		callback(callbackData, &progress, request);
    } else {
        /* in case the caller does not have his own progress indicating function no */
        /* corresponding function will be called when progress shall be indicated. */
        privCallback = NULL;
    }
    
    /* send C-STORE-RQ message and instance data using file data or data set */
     
    cond = DIMSE_sendMessageUsingMemoryData(assoc, presId, &req, 
    NULL, imageDataSet, privCallback, &callbackCtx);
     
    
    if (cond != EC_Normal) {
		DCMLIB_LOG_ERROR("My_DIMSE_storeUser DIMSE_sendMessageUsingMemoryData error \n");
		return cond;
    }

    /* execute final callback */
    if (callback) {
        progress.state = DIMSE_StoreEnd;
		progress.callbackCount++;
		/* execute final callback */
		callback(callbackData, &progress, request);
    }

    /* check if a C-CANCEL-RQ message was encountered earlier */
    if (checkForCancelParams != NULL) {
        checkForCancelParams->cancelEncountered = OFTrue;
    }
	return EC_Normal;
}

OFCondition 
AssociationHelpClient::My_DIMSE_readCStoreResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,DcmDataset **dataset_out,
										 DcmXTSCPResponseParam *ResponseParam,
										  T_DIMSE_BlockingMode blockMode, 
										  int timeout,
										  T_DIMSE_DetectedCancelParameters *checkForCancelParams) 
{
#if 1 //K.Ko

	OFCondition cond = EC_Normal;

	T_DIMSE_C_StoreRSP *response	= &(ResponseParam->m_store_response);
	T_DIMSE_C_StoreRQ *request		= &(ResponseParam->m_store_request);
    /* try to receive C-STORE-RSP */
    do
    {
        /* remember the ID of the presentation context in a local variable */
        T_ASC_PresentationContextID thisPresId = ResponseParam->m_presIdCmd;

        /* try to receive a C-STORE-RSP over the network. */
        cond = DIMSE_receiveCommand(assoc, blockMode, timeout, 
            &(ResponseParam->m_presIdCmd), msg_out, &(ResponseParam->m_statusDetail));
		if (cond != EC_Normal) {
			DCMLIB_LOG_ERROR("My_DIMSE_readCStoreResponse DIMSE_receiveCommand error \n");
			return cond;
		}

        /* if everything was successful so far, the rsp variable contains the command which */
        /* was received check if we encountered a C-CANCEL-RQ; if so, set some variables */
        if (checkForCancelParams != NULL && msg_out->CommandField == DIMSE_C_CANCEL_RQ)
        {
            checkForCancelParams->cancelEncountered = OFTrue;
            checkForCancelParams->req = msg_out->msg.CCancelRQ;
            checkForCancelParams->presId = thisPresId;
        } else {
        /* if we did not receive a C-CANCEL-RQ */

            /* if we did also not encounter a C-STORE-RSP, something is wrong */
            if (msg_out->CommandField != DIMSE_C_STORE_RSP)
            {
              char buf[256];
              sprintf(buf, "DIMSE: Unexpected Response Command Field: 0x%x", (unsigned)msg_out->CommandField);
              return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf);
            }
    
            /* if we get to here, we received a C-STORE-RSP; store this message in the reference parameter */
            *response = msg_out->msg.CStoreRSP;          // BoundsChecker warning !?	

            /* check if the response relates to the request which was sent earlier; if not, return an error */
            if (response->MessageIDBeingRespondedTo != request->MessageID)
            {
              char buf2[256];
              sprintf(buf2, "DIMSE: Unexpected Response MsgId: %d (expected: %d)", response->MessageIDBeingRespondedTo, request->MessageID);
              return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf2);
            }
        }
    } while (checkForCancelParams != NULL && msg_out->CommandField == DIMSE_C_CANCEL_RQ);
#endif
    /* return result value */
    return EC_Normal;
}
