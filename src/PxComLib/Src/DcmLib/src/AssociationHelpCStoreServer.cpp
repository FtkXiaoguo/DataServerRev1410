//  
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4101)
 
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


#include "CheckMemoryLeak.h"

using namespace std ;




struct StoreCallbackData
{
  char* imageFileName;
  DcmFileFormat* dcmff;
  T_ASC_Association* assoc;
};


OFBool             opt_abortDuringStore = OFFalse;
OFBool             opt_abortAfterStore = OFFalse;
#if 0
static void
CStoreSCPCallback(
    void *callbackData,
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ *req,
    char * /*imageFileName*/, DcmDataset **imageDataSet,
    T_DIMSE_C_StoreRSP *rsp,
    DcmDataset **statusDetail)
    /*
     * This function.is used to indicate progress when storescp receives instance data over the
     * network. On the final call to this function (identified by progress->state == DIMSE_StoreEnd)
     * this function will store the data set which was received over the network to a file.
     * Earlier calls to this function will simply cause some information to be dumped to stdout.
     *
     * Parameters:
     *   callbackData  - [in] data for this callback function
     *   progress      - [in] The state of progress. (identifies if this is the initial or final call
     *                   to this function, or a call in between these two calls.
     *   req           - [in] The original store request message.
     *   imageFileName - [in] The path to and name of the file the information shall be written to.
     *   imageDataSet  - [in] The data set which shall be stored in the image file
     *   rsp           - [inout] the C-STORE-RSP message (will be sent after the call to this function)
     *   statusDetail  - [inout] This variable can be used to capture detailed information with regard to
     *                   the status information which is captured in the status element (0000,0900). Note
     *                   that this function does specify any such information, the pointer will be set to NULL.
     */
{
  DIC_UI sopClass;
  DIC_UI sopInstance;

#if 0
  // determine if the association shall be aborted
  if( (opt_abortDuringStore && progress->state != DIMSE_StoreBegin) ||
      (opt_abortAfterStore && progress->state == DIMSE_StoreEnd) )
  {
    if (opt_verbose)
      printf("ABORT initiated (due to command line options)\n");
    ASC_abortAssociation((OFstatic_cast(StoreCallbackData*, callbackData))->assoc);
    rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
    return;
  }

  // if opt_sleepAfter is set, the user requires that the application shall
  // sleep a certain amount of seconds after having received one PDU.
  if (opt_sleepDuring > 0)
  {
    OFStandard::sleep(OFstatic_cast(unsigned int, opt_sleepDuring));
  }

  // dump some information if required (depending on the progress state)
  if (opt_verbose)
  {
    switch (progress->state)
    {
      case DIMSE_StoreBegin:
        printf("RECV:");
        break;
      case DIMSE_StoreEnd:
        printf("\n");
        break;
      default:
        putchar('.');
        break;
    }
    fflush(stdout);
  }

  // if this is the final call of this function, save the data which was received to a file
  // (note that we could also save the image somewhere else, put it in database, etc.)
  if (progress->state == DIMSE_StoreEnd)
  {
    // do not send status detail information
    *statusDetail = NULL;

    // remember callback data
    StoreCallbackData *cbdata = OFstatic_cast(StoreCallbackData *, callbackData);

    // Concerning the following line: an appropriate status code is already set in the resp structure,
    // it need not be success. For example, if the caller has already detected an out of resources problem
    // then the status will reflect this.  The callback function is still called to allow cleanup.
    //rsp->DimseStatus = STATUS_Success;

    // we want to write the received information to a file only if this information
    // is present and the options opt_bitPreserving and opt_ignore are not set.
    if ((imageDataSet)&&(*imageDataSet)&&(!opt_bitPreserving)&&(!opt_ignore))
    {
      OFString fileName;

      // in case option --sort-conc-studies is set, we need to perform some particular
      // steps to determine the actual name of the output file
      if( opt_sortConcerningStudies != NULL )
      {
        // determine the study instance UID in the (current) DICOM object that has just been received
        // note that if findAndGetString says the resulting study instance UID equals NULL, the study
        // instance UID in the (current) DICOM object is an empty string; in general: no matter what
        // happened, we want to create a new string that will contain a corresponding value for the
        // current study instance UID
        const char *tmpstr1 = NULL;
        OFString currentStudyInstanceUID;
        DcmTagKey studyInstanceUIDTagKey( DCM_StudyInstanceUID );
        OFCondition ec = (*imageDataSet)->findAndGetString( studyInstanceUIDTagKey, tmpstr1, OFFalse );
        if( ec != EC_Normal )
        {
          fprintf(stderr, "storescp: No study instance UID found in data set.\n");
          rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
          return;
        }
        if (tmpstr1) currentStudyInstanceUID = tmpstr1;

        // if this is the first DICOM object that was received or if the study instance UID in the
        // current DICOM object does not equal the last object's study instance UID we need to create
        // a new subdirectory in which the current DICOM object will be stored
        if( lastStudyInstanceUID.empty() || lastStudyInstanceUID != currentStudyInstanceUID)
        {
          // if lastStudyInstanceUID is non-empty, we have just completed receiving all objects for one
          // study. In such a case, we need to set a certain indicator variable (lastStudySubdirectoryPathAndName),
          // so that we know that executeOnEndOfStudy() might have to be executed later. In detail, this indicator
          // variable will contain the path and name of the last study's subdirectory, so that we can still remember
          // this directory, when we execute executeOnEndOfStudy(). The memory that is allocated for this variable
          // here will be freed after the execution of executeOnEndOfStudy().
          if( ! lastStudyInstanceUID.empty() )
          {
            lastStudySubdirectoryPathAndName = subdirectoryPathAndName;
          }

          // create the new lastStudyInstanceUID value according to the value in the current DICOM object
          lastStudyInstanceUID = currentStudyInstanceUID;

          // get the current time (needed for subdirectory name)
          OFDateTime dateTime;
          dateTime.setCurrentDateTime();
          // create a name for the new subdirectory. pattern: "[opt_sortConcerningStudies]_[YYYYMMDD]_[HHMMSSMMM]" (use current datetime)
          char buf[32];
          sprintf(buf, "_%04u%02u%02u_%02u%02u%02u%03u",
            dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
            dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond());
          OFString subdirectoryName = opt_sortConcerningStudies;
          subdirectoryName += buf;

          // create subdirectoryPathAndName (string with full path to new subdirectory)
          subdirectoryPathAndName = cbdata->imageFileName;
          size_t position = subdirectoryPathAndName.rfind(PATH_SEPARATOR);
          if (position != OFString_npos) subdirectoryPathAndName.erase(position+1);
          subdirectoryPathAndName += subdirectoryName;

          // check if the subdirectory is already existent
          // if it is already existent dump a warning
          if( OFStandard::dirExists(subdirectoryPathAndName) )
          {
            fprintf(stderr, "storescp: Warning: Subdirectory for studies already existent. (%s)\n", subdirectoryPathAndName.c_str() );
          }

          // if it is not existent create it
#ifdef HAVE_WINDOWS_H
          if( _mkdir( subdirectoryPathAndName.c_str() ) == -1 )
#else
          if( mkdir( subdirectoryPathAndName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
#endif
          {
            fprintf(stderr, "storescp: Could not create subdirectory %s.\n", subdirectoryPathAndName.c_str() );
            rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
            return;
          }
          // all objects of a study have been received, so a new subdirectory is started.
          // ->timename counter can be reset, because the next filename can't cause a duplicate.
          // if no reset would be done, files of a new study (->new directory) would start with a counter in filename
          if (opt_timeNames)
            timeNameCounter = -1;
        }

        // integrate subdirectory name into file name (note that cbdata->imageFileName currently contains both
        // path and file name; however, the path refers to the output directory captured in opt_outputDirectory)
        char *tmpstr5 = strrchr( cbdata->imageFileName, PATH_SEPARATOR );
        fileName = subdirectoryPathAndName;
        fileName += tmpstr5;

        // update global variable outputFileNameArray
        // (might be used in executeOnReception() and renameOnEndOfStudy)
        outputFileNameArray.push_back(++tmpstr5);
      }
      // if option --sort-conc-studies is not set, the determination of the output file name is simple
      else
      {
        fileName = cbdata->imageFileName;

        // update global variables outputFileNameArray
        // (might be used in executeOnReception() and renameOnEndOfStudy)
        const char *tmpstr6 = strrchr( fileName.c_str(), PATH_SEPARATOR );
        outputFileNameArray.push_back(++tmpstr6);
      }

      // determine the transfer syntax which shall be used to write the information to the file
      E_TransferSyntax xfer = opt_writeTransferSyntax;
      if (xfer == EXS_Unknown) xfer = (*imageDataSet)->getOriginalXfer();

      // store file either with meta header or as pure dataset
      OFCondition cond = cbdata->dcmff->saveFile(fileName.c_str(), xfer, opt_sequenceType, opt_groupLength,
          opt_paddingType, OFstatic_cast(Uint32, opt_filepad),
          OFstatic_cast(Uint32, opt_itempad), !opt_useMetaheader);
      if (cond.bad())
      {
        fprintf(stderr, "storescp: Cannot write image file: %s\n", fileName.c_str());
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
      }

      // check the image to make sure it is consistent, i.e. that its sopClass and sopInstance correspond
      // to those mentioned in the request. If not, set the status in the response message variable.
      if ((rsp->DimseStatus == STATUS_Success)&&(!opt_ignore))
      {
        // which SOP class and SOP instance ?
        if (! DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, opt_correctUIDPadding))
        {
           fprintf(stderr, "storescp: Bad image file: %s\n", fileName.c_str());
           rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
        }
        else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
        {
          rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
        }
        else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
        {
          rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
        }
      }

    }

    // in case opt_bitPreserving is set, do some other things
    if( opt_bitPreserving )
    {
      // we need to set outputFileNameArray and outputFileNameArrayCnt to be
      // able to perform the placeholder substitution in executeOnReception()
      char *tmpstr7 = strrchr( cbdata->imageFileName, PATH_SEPARATOR );
      outputFileNameArray.push_back(++tmpstr7);
    }
  }
#endif
  // return
  return;
}
#endif

OFCondition AssociationHelpServer::CStoreSCP(
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
 T_DIMSE_C_StoreRQ *req;
//  char imageFileName[2048];
 // sprintf(imageFileName,"dbg_imageFile.dcm");

  // assign the actual information of the C-STORE-RQ command to a local variable
  req = &msg->msg.CStoreRQ;
  ResponseParam->m_store_request = msg->msg.CStoreRQ;

 
  // dump some information if required
  if (opt_verbose)
  {
	DCMLIB_LOG_DEBUG("C_StoreRQ Received: \n");

 
   // DIMSE_printCStoreRQ(stdout,req);
	{
		OFString strTemp;                                 
        DIMSE_dumpMessage(strTemp, *req, DIMSE_INCOMING);  
		strTemp += OFString("\n");
		DCMLIB_LOG_DEBUG(strTemp.c_str());
	}
	DCMLIB_LOG_DEBUG("----\n");
  }

  // intialize some variables


  // if opt_bitPreserving is set, the user requires that the data shall be
  // written exactly as it was received. Depending on this option, function
  // DIMSE_storeProvider must be called with certain parameters.

 
 //always  opt_bitPreserving = OFFalse;

 
 
	cond = My_DIMSE_storeProvider(assoc, ResponseParam, NULL, opt_useMetaheader, &dataset_out,
  //     0/*callback*/, 0/*callbackData*/, 
	   opt_blockMode, opt_dimse_timeout);
 

 
  // if some error occured, dump corresponding information and remove the outfile if necessary
  if (cond.bad())
  {
    DCMLIB_LOG_ERROR("storescp: Store SCP Failed: \n");
  //  DimseCondition::dump(cond);
	{
		 OFString strTemp;
		 DimseCondition::dump(strTemp, cond);
		 strTemp += OFString("\n");
		 DCMLIB_LOG_ERROR(strTemp.c_str());
	}
     
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
  //{
  //  OFStandard::sleep(OFstatic_cast(unsigned int, opt_sleepAfter));
  //}

  // return return value
  return cond;
}


//never use it
#if 0
void AssociationHelpServer::executeOnReception()
    /*
     * This function deals with the execution of the command line which was passed
     * to option --exec-on-reception of the storescp. This command line is captured
     * in opt_execOnReception. Note that the command line can contain the placeholders
     * PATH_PLACEHOLDER and FILENAME_PLACEHOLDER which need to be substituted before the command line is actually
     * executed. PATH_PLACEHOLDER will be substituted by the path to the output directory into which
     * the last file was written; FILENAME_PLACEHOLDER will be substituted by the filename of the last
     * file which was written.
     *
     * Parameters:
     *   none.
     */
{
  OFString cmd = opt_execOnReception;

#if 0
  // in case a file was actually written
  if( !opt_ignore )
  {
    // perform substitution for placeholder #p; note that
    //  - in case option --sort-conc-studies is set, #p will be substituted by subdirectoryPathAndName
    //  - and in case option --sort-conc-studies is not set, #p will be substituted by opt_outputDirectory
    OFString dir = (opt_sortConcerningStudies == NULL) ? OFString(opt_outputDirectory) : subdirectoryPathAndName;
    cmd = replaceChars( cmd, OFString(PATH_PLACEHOLDER), dir );

    // perform substitution for placeholder #f; note that outputFileNameArray.back()
    // always contains the name of the file (without path) which was written last.
    OFString outputFileName = outputFileNameArray.back();
    cmd = replaceChars( cmd, OFString(FILENAME_PLACEHOLDER), outputFileName );
  }

  // perform substitution for placeholder #a
  cmd = replaceChars( cmd, OFString(CALLING_AETITLE_PLACEHOLDER), callingaetitle );

  // perform substitution for placeholder #c
  cmd = replaceChars( cmd, OFString(CALLED_AETITLE_PLACEHOLDER), calledaetitle );

#endif
  // Execute command in a new process
  executeCommand( cmd );
}
#endif

///////////////////
// divide the  DIMSE_sendStoreResponse
/////////////////////

typedef struct {
    void *callbackData;
    T_DIMSE_StoreProgress *progress;
    T_DIMSE_C_StoreRQ *request;
    char *imageFileName; 
    DcmDataset **imageDataSet;
    T_DIMSE_C_StoreRSP *response;
    DcmDataset	**statusDetail;
    DIMSE_StoreProviderCallback	callback;
} DIMSE_PrivateProviderContext;

#if 0
static void 
privateProviderCallback(void *callbackData, unsigned long bytes)
{
    DIMSE_PrivateProviderContext *ctx;
    ctx = (DIMSE_PrivateProviderContext*)callbackData;
    ctx->progress->state = DIMSE_StoreProgressing;
    ctx->progress->progressBytes = bytes;
    ctx->progress->callbackCount++;
    if (ctx->callback) {
        ctx->callback(ctx->callbackData, ctx->progress, ctx->request, 
	    ctx->imageFileName, ctx->imageDataSet, ctx->response,
	    ctx->statusDetail);
    }
}
#endif
OFCondition
AssociationHelpServer::My_DIMSE_storeProvider( 
	T_ASC_Association *assoc, 
	DcmXTSCPResponseParam *ResponseParam,
	const char* imageFileName, int writeMetaheader,
	DcmDataset **imageDataSet,
//	DIMSE_StoreProviderCallback callback, void *callbackData,
	T_DIMSE_BlockingMode blockMode, int timeout)
    /*
     * This function receives a data set over the network and either stores this data in a file (exactly as it was
     * received) or it stores this data in memory. Before, during and after the process of receiving data, the callback
     * function which was provided by the caller (if it was provided) will be called to indicate progress.
     * 
     * Parameters:
     *   assoc           - [in] The association (network connection to another DICOM application).
     *   presIDCmd       - [in] The ID of the presentation context which was specified in the PDV which contained
     *                          the DIMSE command.
     *   request         - [in] The DIMSE C-STORE-RQ message that was received.
     *   imageFileName   - [in] If this variable does not equal NULL, the information (which was received over the network)
     *                          will be written to a file exactly as it was received over the network. In such a case, this
     *                          this variable contains the name of the file the information shall be written to.
     *   writeMetaheader - [in] Specifies if the resulting file shall only contain the dataset which was received
     *                          (OFFalse) or if it shall contain both metaheader and dataset information (OFTrue)
     *                          (i.e if the file will be written according to the DICOM file format).
     *   imageDataSet    - [inout] If this variable does not equal NULL, and at the same time imageFileName equals NULL,
     *                          this variable will in the end contain the information which was received over the network.
     *                          Note that this function assumes that either imageFileName or imageDataSet does not equal NULL.
     *   callback        - [in] Pointer to a function which shall be called to indicate progress.
     *   callbackData    - [in] Pointer to data which shall be passed to the progress indicating function
     *   blockMode       - [in] The blocking mode for receiving data (either DIMSE_BLOCKING or DIMSE_NONBLOCKING)
     *   timeout         - [in] Timeout interval for receiving data (if the blocking mode is DIMSE_NONBLOCKING).
     */
{	
	/////////////////////////
	T_DIMSE_C_StoreRSP &response			= ResponseParam->m_store_response;
	T_ASC_PresentationContextID presIdCmd	= ResponseParam->m_presIdCmd;
	T_DIMSE_C_StoreRQ *request				= &(ResponseParam->m_store_request);
	DcmDataset * &statusDetail				= ResponseParam->m_statusDetail;
	statusDetail = NULL;
	////////////////////////
    OFCondition cond = EC_Normal;
    DIMSE_PrivateProviderContext callbackCtx;
    DIMSE_ProgressCallback privCallback = NULL;
    T_ASC_PresentationContextID presIdData = 0;
 //   T_DIMSE_C_StoreRSP response;
 //   DcmDataset *statusDetail = NULL;
    T_DIMSE_StoreProgress progress;

    /* initialize the C-STORE-RSP message variable */
    //bzero((char*)&response, sizeof(response));
	memset((char*)&response, 0, sizeof(response));
    response.DimseStatus = STATUS_Success;	/* assume */
    response.MessageIDBeingRespondedTo = request->MessageID;
    response.DataSetType = DIMSE_DATASET_NULL;	/* always for C-STORE-RSP */
    strcpy(response.AffectedSOPClassUID, request->AffectedSOPClassUID);
    strcpy(response.AffectedSOPInstanceUID, request->AffectedSOPInstanceUID);
    response.opts = (O_STORE_AFFECTEDSOPCLASSUID | O_STORE_AFFECTEDSOPINSTANCEUID);
    if (request->opts & O_STORE_RQ_BLANK_PADDING) response.opts |= O_STORE_RSP_BLANK_PADDING;
    if (dcmPeerRequiresExactUIDCopy.get()) response.opts |= O_STORE_PEER_REQUIRES_EXACT_UID_COPY;

 
	//none callback
	privCallback = NULL;
 

    
    /* in the following, we want to receive data over the network and do something with this data. If the */
    /* imageFileName does not equal NULL, the caller required that the data shall be written to a file */
    /* exactly the way it was received over the network. Hence, a filestream will be created and the data */
    /* set will be received and written to the file through the call to DIMSE_receiveDataSetInFile(...).*/
    /* If the imageFileName does equal NULL but at the same time imageDataSet does not equal NULL, the */
    /* data shall be received and stored in memory. This will be handled through the call to function */
    /* DIMSE_receiveDataSetInMemory(...). The case in which both variables are NULL is considered to */
    /* be an error and will be handled correspondingly. */

	if (imageDataSet != NULL)
    {
        /* receive data and store it in memory */
        cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout,
		&presIdData, imageDataSet, privCallback, &callbackCtx);
    } else {
		DCMLIB_LOG_ERROR("My_DIMSE_storeProvider imageDataSet != NULL \n");

        /* if both variables are set to NULL, report an error */
 		return DIMSE_BADDATA;
    }

    /* check if presentation context IDs of the command (which was received earlier) and of the data */
    /* set (which was received just now) differ from each other. If this is the case, return an error. */
    if (cond.good() && (presIdData != presIdCmd))
    {
    	cond = makeDcmnetCondition(DIMSEC_INVALIDPRESENTATIONCONTEXTID, OF_error, "DIMSE: Presentation Contexts of Command and Data Differ");
    }

    /* depending on the error status, set the success indicating flag in the response message */
    if (cond == EC_Normal) {
        response.DimseStatus = STATUS_Success;
    } else if (cond == DIMSE_OUTOFRESOURCES) {
        response.DimseStatus = STATUS_STORE_Refused_OutOfResources;
    } else {
        return cond;
    }
   
	/* execute final callback (user does not have to provide callback) */
#if 0
    if (callback) {
        progress.state = DIMSE_StoreEnd;
	progress.callbackCount++;
	/* execute final callback */
	callback(callbackData, &progress, request, 
	    (char*)imageFileName, imageDataSet,
	    &response, &statusDetail);
    }
#endif
    

	return cond; //K.Ko
}

OFCondition
AssociationHelpServer::My_DIMSE_sendStoreResponse( 
	T_ASC_Association *assoc, 
	DcmXTSCPResponseParam *ResponseParam
	)
{
	OFCondition cond = EC_Normal;
    /* send a C-STORE-RSP message over the network to the other DICOM application */
    OFCondition cond2 = DIMSE_sendStoreResponse(assoc, 
		ResponseParam->m_presIdCmd,
		&(ResponseParam->m_store_request),
		&(ResponseParam->m_store_response),
		ResponseParam->m_statusDetail);
 
    /* if we already had an error condition, don't overwrite */
    if (cond.good()) cond = cond2;

    /* return result value */    
    return cond;
}

////////////////////

////////////////////
