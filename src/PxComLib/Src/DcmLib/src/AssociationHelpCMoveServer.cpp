//  
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
 
#include "AssociationHelp.h"
#include "FXDcmLibLogger.h"
//////////////////
 

///
#define PATH_PLACEHOLDER "#p"
#define FILENAME_PLACEHOLDER "#f"
#define CALLING_AETITLE_PLACEHOLDER "#a"
#define CALLED_AETITLE_PLACEHOLDER "#c"

//////////////////

//#include "dcmtk/dcmnet/diutil.h"
//#include "dcmtk/dcmnet/dimse.h"		/* always include the module header */
//#include "dcmtk/dcmnet/cond.h"
#include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */


using namespace std ;

#include "CheckMemoryLeak.h"

struct CMoveCallbackData
{
  char* imageFileName;
  DcmFileFormat* dcmff;
  T_ASC_Association* assoc;
};


//OFBool             opt_abortDuringStore = OFFalse;
//OFBool             opt_abortAfterStore = OFFalse;


////////////////////

////////////////////

OFCondition
AssociationHelpServer::My_DIMSE_moveProvider(
        T_ASC_Association *assoc,
		DcmXTSCPResponseParam *ResponseParam,
		DcmDataset **reqIds, //output
 //       DIMSE_MoveProviderCallback callback, void *callbackData,
        T_DIMSE_BlockingMode blockMode, int timeout)
    /*
     * This function receives a data set which represents the search mask over the network and
     * stores this data in memory. Then, it tries to select corresponding records which match the
     * search mask from some database (done whithin the callback function) and sends corresponding
     * C-FIND-RSP messages to the other DICOM application this application is connected with.
     * The selection of each matching record and the sending of a corresponding C-FIND-RSP message
     * is conducted in a loop since there can be more than one search result. In the end, also the
     * C-FIND-RSP message which indicates that there are no more search results will be sent.
     *
     * Parameters:
     *   assoc           - [in] The association (network connection to another DICOM application).
     *   presIDCmd       - [in] The ID of the presentation context which was specified in the PDV which contained
     *                          the DIMSE command.
     *   request         - [in] The DIMSE C-FIND-RQ message that was received.
     *   callback        - [in] Pointer to a function which shall be called to indicate progress.
     *   callbackData    - [in] Pointer to data which shall be passed to the progress indicating function
     *   blockMode       - [in] The blocking mode for receiving data (either DIMSE_BLOCKING or DIMSE_NONBLOCKING)
     *   timeout         - [in] Timeout interval for receiving data (if the blocking mode is DIMSE_NONBLOCKING).
     */
{
	
        

	/////////////////////////
	T_DIMSE_C_MoveRSP &rsp			= ResponseParam->m_move_response;
	T_ASC_PresentationContextID presIdCmd	= ResponseParam->m_presIdCmd;
	T_DIMSE_C_MoveRQ *request				= &(ResponseParam->m_move_request);
	DcmDataset * &statusDetail				= ResponseParam->m_statusDetail;
	statusDetail = NULL;
	////////////////////////

	/////////
    T_ASC_PresentationContextID presIdData;
 
    DcmDataset *rspIds = NULL;
    OFBool cancelled = OFFalse;
    OFBool normal = OFTrue;
    int responseCount = 0;


 
    OFCondition cond = EC_Normal;
   
  
    cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout, &presIdData, reqIds, NULL, NULL);

    if (cond.good()) {

		static bool dbg_write_dicm_flag = false;
		if(dbg_write_dicm_flag){
			DcmFileFormat dfileTemp(*reqIds);
			dfileTemp.saveFile("dbg_CMoveCmd.dcm");

		
		}

        if (presIdData != presIdCmd) {
          cond = makeDcmnetCondition(DIMSEC_INVALIDPRESENTATIONCONTEXTID, OF_error, "DIMSE: Presentation Contexts of Command and Data Differ");
        } else {

			DCMLIB_LOG_DEBUG("My_DIMSE_moveProvider DIMSE_receiveDataSetInMemory presIdData == presIdCmd \n");

            bzero((char*)&rsp, sizeof(rsp));
            rsp.DimseStatus = STATUS_Pending;   /* assume */

 
			{
 
                cond = DIMSE_checkForCancelRQ(assoc, presIdCmd, request->MessageID);
                if (cond == EC_Normal) {
                    /* cancel received */
                    rsp.DimseStatus = STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication;
                    cancelled = OFTrue;
                } else if (cond == DIMSE_NODATAAVAILABLE) {
                    /* timeout */
                } else {
                    /* some execption condition occured, bail out */
                    normal = OFFalse;
                }

                if (normal) {
 

                     if (cancelled) {
                         /* make sure */
                         rsp.DimseStatus =
                           STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication;
                         if (rspIds != NULL) {
                             delete reqIds;
                             reqIds = NULL;
                         }
                     }

       //              cond = DIMSE_sendMoveResponse(assoc, presIdCmd, request, &rsp, rspIds, statusDetail);

                     if (rspIds != NULL) {
                         delete rspIds;
                         rspIds = NULL;
                     }
                     if (statusDetail != NULL) {
                         delete statusDetail;
                         statusDetail = NULL;
                     }
                }
            }
        }
		cond = EC_Normal;
    }

  //  delete reqIds;
    delete rspIds;
    return cond;
}


OFCondition
AssociationHelpServer::My_DIMSE_sendMoveResponse(T_ASC_Association * assoc,
//        T_ASC_PresentationContextID presID,
//        T_DIMSE_C_FindRQ *request,
//       T_DIMSE_C_FindRSP *response, DcmDataset *rspIds,
//        DcmDataset *statusDetail
 
        DcmXTSCPResponseParam *ResponseParam
	)
    /*
     * This function takes care of sending a C-FIND-RSP message over the network to the DICOM
     * application this application is connected with.
     *
     * Parameters:
     *   assoc        - [in] The association (network connection to another DICOM application).
     *   presID       - [in] The ID of the presentation context which was specified in the PDV
     *                       which contained the DIMSE C-FIND-RQ command.
     *   request      - [in] The DIMSE C-FIND-RQ command which was received earlier.
     *   response     - [inout] The C-FIND-RSP command which shall be sent. Might be modified.
     *   statusDetail - [in] Detailed information with regard to the status information which is captured
     *                       in the status element (0000,0900). Note that the value for element (0000,0900)
     *                       is contained in this variable.
     */
{

	 

    OFCondition         cond = DIMSE_sendMoveResponse(
		assoc,
		ResponseParam->m_presIdCmd,
		&(ResponseParam->m_move_request),
		&(ResponseParam->m_move_response),
		ResponseParam->m_response_datas,
		ResponseParam->m_statusDetail
		
		);
	 
  return cond;
 
   
}
 


//////////////////
void CMoveProviderCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_MoveRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP *response,
        DcmDataset **responseIdentifiers,
        DcmDataset **statusDetail)
{

}

 
OFCondition AssociationHelpServer::CMoveSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  DcmDataset *dataset_out,
  DcmXTSCPResponseParam *ResponseParam)
    /*
     * This function processes a DIMSE C-STORE-RQ commmand that was
     * received over the network connection.
     *
     * Parameters:
     *   assoc  - [in] The association (network connection to another DICOM application).
     *   msg    - [in] The DIMSE C-STORE-RQ message that was received.
     *   presID - [in] The ID of the presentation context which was specified in the PDV which contained
     *                 the DIMSE command.
     */
{
  OFCondition cond = EC_Normal;
 T_DIMSE_C_MoveRQ *req;
//  char imageFileName[2048];

  // assign the actual information of the C-STORE-RQ command to a local variable
  req = &msg->msg.CMoveRQ;
  ResponseParam->m_move_request = msg->msg.CMoveRQ;

 
  // dump some information if required
  if (opt_verbose)
  {
    DCMLIB_LOG_DEBUG("C_MoveRQ Received:  \n");
 //   DIMSE_printCStoreRQ(stdout,req);
	{
		OFString strTemp;                                 
        DIMSE_dumpMessage(strTemp, *req, DIMSE_INCOMING);  
		strTemp += OFString("\n");
		DCMLIB_LOG_DEBUG(strTemp.c_str());
	}
  }

 

  // if opt_bitPreserving is set, the user requires that the data shall be
  // written exactly as it was received. Depending on this option, function
  // DIMSE_storeProvider must be called with certain parameters.

 
  //always  opt_bitPreserving = OFFalse;
 
	cond =  My_DIMSE_moveProvider(
		/* in */
		assoc, ResponseParam, &dataset_out,
  //     CMoveProviderCallback, &callbackData, 
	   /* blocking info for data set */
	   opt_blockMode, opt_dimse_timeout);
  

  // if some error occured, dump corresponding information and remove the outfile if necessary
  if (cond.bad())
  {
    DCMLIB_LOG_ERROR("movescp: CMove SCP Failed: \n");
   // DimseCondition::dump(cond);
	{
		 OFString strTemp;
		 DimseCondition::dump(strTemp, cond);
		 strTemp += OFString("\n");
		 DCMLIB_LOG_ERROR(strTemp.c_str());
	}
    // remove file
   
  }


  // if everything was successful so far and option --exec-on-reception is set,
  // we want to execute a certain command which was passed to the application
//  if( cond.good() && opt_execOnReception != NULL )
 //   executeOnReception();

  // if everything was successful so far, go ahead and handle possible end-of-study events
  //if( cond.good() )
  //  executeEndOfStudyEvents();

  // if opt_sleepAfter is set, the user requires that the application shall
  // sleep a certain amount of seconds after storing the instance data.
 // if (opt_sleepAfter > 0)
 // {
 //   OFStandard::sleep(OFstatic_cast(unsigned int, opt_sleepAfter));
 // }

  // return return value
  return cond;
}