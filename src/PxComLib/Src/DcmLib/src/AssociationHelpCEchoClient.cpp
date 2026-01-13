//  
//
//////////////////////////////////////////////////////////////////////

 
#include "AssociationHelp.h"

//////////////////
 
#define INCLUDE_CSTDLIB
#define INCLUDE_CSTRING

 #include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */

#include "CheckMemoryLeak.h"


//#32 2012/08/06 for Echo-SCU
OFCondition
AssociationHelpClient::My_DIMSE_echoUser(T_ASC_Association * assoc, DcmXTSCPResponseParam *ResponseParam
	)
{
//	T_DIMSE_Message req;//, rsp;
	T_DIMSE_Message &req = ResponseParam->m_reqMsg;

	T_ASC_PresentationContextID presID = ResponseParam->m_presIdCmd;
	 

    const char *sopClass;

    /* which SOP class  */
    sopClass = UID_VerificationSOPClass;

    /* which presentation context should be used */
    presID = ASC_findAcceptedPresentationContextID(assoc, sopClass);
    if (presID == 0)
    {
        char buf[1024];
        sprintf(buf, "DIMSE: No Presentation Context for: %s", sopClass);
        return makeDcmnetCondition(DIMSEC_NOVALIDPRESENTATIONCONTEXTID, OF_error, buf);
    }
 
	bzero((char*)&req, sizeof(req));
 //   bzero((char*)&rsp, sizeof(rsp));

    req.CommandField = DIMSE_C_ECHO_RQ;
    req.msg.CEchoRQ.MessageID = ResponseParam->m_echo_request.MessageID;
    strcpy(req.msg.CEchoRQ.AffectedSOPClassUID,
	   sopClass);
    req.msg.CEchoRQ.DataSetType = DIMSE_DATASET_NULL;
    


    OFCondition cond = DIMSE_sendMessageUsingMemoryData(assoc, presID, &req, NULL, NULL, NULL, NULL);
    if (cond.bad()) return cond;

	return cond;

}

OFCondition 
AssociationHelpClient::My_DIMSE_readEchoResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,DcmDataset **dataset_out,
					  DcmXTSCPResponseParam *ResponseParam,
					  T_DIMSE_BlockingMode blockMode, 
					  int timeout,
					  int &command)
{

	OFCondition cond;

	T_ASC_PresentationContextID presID = ResponseParam->m_presIdCmd;

	T_DIMSE_Message rsp;

	bzero((char*)&rsp, sizeof(rsp));

	/* receive response */
    cond = DIMSE_receiveCommand(assoc, blockMode, timeout, &presID, &rsp,   &(ResponseParam->m_statusDetail));
    if (cond.bad()) return cond;

	memcpy(&(ResponseParam->m_echo_response),&(rsp.msg.CEchoRSP),sizeof(T_DIMSE_C_EchoRSP));

	command = rsp.CommandField;
#if 0
	if (rsp.CommandField != DIMSE_C_ECHO_RSP)
    {
        char buf1[256];
        sprintf(buf1, "DIMSE: Unexpected Response Command Field: 0x%x", (unsigned)rsp.CommandField);
        return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf1);
    }
#endif

	return cond;
}