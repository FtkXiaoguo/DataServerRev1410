//  
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4101)
 
#include "AssociationHelp.h"
#include "dcmtk/ofstd/ofbmanip.h" 
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


void AssociationHelpBase::setupLogLevel(int logLevel){
		switch(logLevel){
		case IDcmLib::LOGLEVEL_OFF_LOG:
		case IDcmLib::LOGLEVEL_NOT_SET:
		case IDcmLib::LOGLEVEL_FALTAL:
		case IDcmLib::LOGLEVEL_ERROR:
		case IDcmLib::LOGLEVEL_WARN:
			opt_verbose = OFFalse;
			opt_debug	= OFFalse;
			break;
		case IDcmLib::LOGLEVEL_INFO:
		case IDcmLib::LOGLEVEL_DEBUG:
			opt_verbose = OFFalse;
			opt_debug	= OFTrue;
			break;
		case IDcmLib::LOGLEVEL_TRACE:
		case IDcmLib::LOGLEVEL_ALL:
			opt_verbose = OFTrue;
			opt_debug	= OFTrue;
			break;	
		}
	};

/////////////////////
//// for Association Server
/////////////////////
AssociationHelpServer::AssociationHelpServer()
{
	init();
}
AssociationHelpServer::~AssociationHelpServer()
{
	 
 
}
void AssociationHelpServer::setAcceptStorageSOPClassUIDs(const char **sopclssList,int num)
{
	 m_AcceptStorageSOPClassUIDs = sopclssList;
	 m_NumAcceptStorageSOPClassUIDs = num;

}
void AssociationHelpServer::init()
{
	AssociationHelpBase::init();

	m_maxPDU = ASC_DEFAULTMAXPDU;
	opt_secureConnection = OFFalse;    // default: no secure connection
	 
	//opt_execOnReception = NULL;        // default: don't execute anything on reception
	opt_execOnEndOfStudy = NULL;       // default: don't execute anything on end of study

  
	opt_timeNames = OFFalse;

	callingaetitle;  // calling AE title will be stored here
	calledaetitle;   // called AE title will be stored here

 
	opt_rejectWithoutImplementationUID = OFFalse;

 

	#ifdef _WIN32
 
	//opt_execSync = OFFalse;            // default: execute in background
	#endif


	/////

	opt_promiscuous = OFFalse;
	opt_respondingaetitle = "STORESCP";
	///
	opt_ignore = OFFalse;
	opt_uniqueFilenames = OFFalse;
	 opt_outputDirectory =  "." ;
	  timeNameCounter = -1;

//	  opt_bitPreserving = OFFalse;
	  opt_useMetaheader = OFTrue;

//	  opt_sleepAfter = 0;
	  opt_sortConcerningStudies = NULL;
	
	 m_AcceptStorageSOPClassUIDs = 0 ;
	 m_NumAcceptStorageSOPClassUIDs = 0 ;

}	


OFString AssociationHelpServer::replaceChars( const OFString &srcstr, const OFString &pattern, const OFString &substitute )
    /*
     * This function replaces all occurrences of pattern in srcstr with substitute and returns
     * the result as a new OFString variable. Note that srcstr itself will not be changed,
     *
     * Parameters:
     *   srcstr     - [in] The source string.
     *   pattern    - [in] The pattern string which shall be substituted.
     *   substitute - [in] The substitute for pattern in srcstr.
     */
{
  OFString result = srcstr;
  size_t pos = 0;

  while( pos != OFString_npos )
  {
    pos = result.find( pattern, pos );

    if( pos != OFString_npos )
    {
      result.replace( pos, pattern.size(), substitute );
      pos += substitute.size();
    }
  }

  return( result );
}

////never use it
#if 0
 void AssociationHelpServer::executeCommand( const OFString &cmd )
    /*
     * This function executes the given command line. The execution will be
     * performed in a new process which can be run in the background
     * so that it does not slow down the execution of storescp.
     *
     * Parameters:
     *   cmd - [in] The command which shall be executed.
     */
{
#ifdef HAVE_FORK
  pid_t pid = fork();
  if( pid < 0 )     // in case fork failed, dump an error message
    fprintf( stderr, "storescp: Cannot execute command '%s' (fork failed).\n", cmd.c_str() );
  else if (pid > 0)
  {
    /* we are the parent process */
    /* remove pending zombie child processes */
    cleanChildren(pid, OFTrue);
  }
  else // in case we are the child process, execute the command etc.
  {
    // execute command through execl will terminate the child process.
    // Since we only have a single command string and not a list of arguments,
    // we 'emulate' a call to system() by passing the command to /bin/sh
    // which hopefully exists on all Posix systems.

    if (execl( "/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL ) < 0)
      fprintf( stderr, "storescp: Cannot execute /bin/sh.\n" );

    // if execl succeeds, this part will not get executed.
    // if execl fails, there is not much we can do except bailing out.
    abort();
  }
#else
  PROCESS_INFORMATION procinfo;
  STARTUPINFO sinfo;
  OFBitmanipTemplate<char>::zeroMem((char *)&sinfo, sizeof(sinfo));
  sinfo.cb = sizeof(sinfo);

  // execute command (Attention: Do not pass DETACHED_PROCESS as sixth argument to the below
  // called function because in such a case the execution of batch-files is not going to work.)
  if( !CreateProcess(NULL, OFconst_cast(char *, cmd.c_str()), NULL, NULL, 0, 0, NULL, NULL, &sinfo, &procinfo) )
    fprintf( stderr, "storescp: Error while executing command '%s'.\n" , cmd.c_str() );

  if (opt_execSync)
  {
      // Wait until child process exits (makes execution synchronous).
      WaitForSingleObject(procinfo.hProcess, INFINITE);
  }

  // Close process and thread handles to avoid resource leak
  CloseHandle(procinfo.hProcess);
  CloseHandle(procinfo.hThread);
#endif
}
#endif


//never use it
#if 0
void AssociationHelpServer::executeEndOfStudyEvents()
    /*
     * This function deals with the execution of end-of-study-events. In detail,
     * events that need to take place are sepcified by the user through certain
     * command line options. The options that define these end-of-study-events
     * are "--rename-on-eostudy" and "--exec-on-eostudy".
     *
     * Parameters:
     *   none.
     */
{
#if 0
  // if option --rename-on-eostudy is set and variable lastStudySubdirectoryPathAndName
  // does not equal NULL (i.e. we received all objects that belong to one study, or - in
  // other words - it is the end of one study) we want to rename the output files that
  // belong to the last study. (Note that these files are captured in outputFileNameArray)
  if( opt_renameOnEndOfStudy && ! lastStudySubdirectoryPathAndName.empty() )
    renameOnEndOfStudy();

  // if option --exec-on-eostudy is set and variable lastStudySubdirectoryPathAndName does
  // not equal NULL (i.e. we received all objects that belong to one study, or - in other
  // words - it is the end of one study) we want to execute a certain command which was
  // passed to the application
  if( opt_execOnEndOfStudy != NULL && ! lastStudySubdirectoryPathAndName.empty() )
    executeOnEndOfStudy();

  lastStudySubdirectoryPathAndName.clear();
#endif
}
#endif



DUL_PRESENTATIONCONTEXT *
AssociationHelpServer::findPresentationContextID(LST_HEAD * head,
                          T_ASC_PresentationContextID presentationContextID)
{
    DUL_PRESENTATIONCONTEXT *pc;
    LST_HEAD **l;
    OFBool found = OFFalse;

    if (head == NULL)
        return NULL;

    l = &head;
    if (*l == NULL)
        return NULL;

    pc = OFstatic_cast(DUL_PRESENTATIONCONTEXT *, LST_Head(l));
    (void)LST_Position(l, OFstatic_cast(LST_NODE *, pc));

    while (pc && !found) {
        if (pc->presentationContextID == presentationContextID) {
            found = OFTrue;
        } else {
            pc = OFstatic_cast(DUL_PRESENTATIONCONTEXT *, LST_Next(l));
        }
    }
    return pc;
}

/** accept all presenstation contexts for unknown SOP classes,
 *  i.e. UIDs appearing in the list of abstract syntaxes
 *  where no corresponding name is defined in the UID dictionary.
 *  @param params pointer to association parameters structure
 *  @param transferSyntax transfer syntax to accept
 *  @param acceptedRole SCU/SCP role to accept
 */
OFCondition AssociationHelpServer::acceptUnknownContextsWithTransferSyntax(
    T_ASC_Parameters * params,
    const char* transferSyntax,
    T_ASC_SC_ROLE acceptedRole)
{
    OFCondition cond = EC_Normal;
    int n, i, k;
    DUL_PRESENTATIONCONTEXT *dpc;
    T_ASC_PresentationContext pc;
    OFBool accepted = OFFalse;
    OFBool abstractOK = OFFalse;

    n = ASC_countPresentationContexts(params);
    for (i = 0; i < n; i++)
    {
        cond = ASC_getPresentationContext(params, i, &pc);
        if (cond.bad()) return cond;
        abstractOK = OFFalse;
        accepted = OFFalse;

        if (dcmFindNameOfUID(pc.abstractSyntax) == NULL)
        {
            abstractOK = OFTrue;

            /* check the transfer syntax */
            for (k = 0; (k < OFstatic_cast(int, pc.transferSyntaxCount)) && !accepted; k++)
            {
                if (strcmp(pc.proposedTransferSyntaxes[k], transferSyntax) == 0)
                {
                    accepted = OFTrue;
                }
            }
        }

        if (accepted)
        {
            cond = ASC_acceptPresentationContext(
                params, pc.presentationContextID,
                transferSyntax, acceptedRole);
            if (cond.bad()) return cond;
        } else {
            T_ASC_P_ResultReason reason;

            /* do not refuse if already accepted */
            dpc = findPresentationContextID(
                              params->DULparams.acceptedPresentationContext,
                                            pc.presentationContextID);
            if ((dpc == NULL) ||
                ((dpc != NULL) && (dpc->result != ASC_P_ACCEPTANCE)))
            {

                if (abstractOK) {
                    reason = ASC_P_TRANSFERSYNTAXESNOTSUPPORTED;
                } else {
                    reason = ASC_P_ABSTRACTSYNTAXNOTSUPPORTED;
                }
                /*
                 * If previously this presentation context was refused
                 * because of bad transfer syntax let it stay that way.
                 */
                if ((dpc != NULL) &&
                    (dpc->result == ASC_P_TRANSFERSYNTAXESNOTSUPPORTED))
                    reason = ASC_P_TRANSFERSYNTAXESNOTSUPPORTED;

                cond = ASC_refusePresentationContext(params,
                                              pc.presentationContextID,
                                              reason);
                if (cond.bad()) return cond;
            }
        }
    }
    return EC_Normal;
}



/** accept all presenstation contexts for unknown SOP classes,
 *  i.e. UIDs appearing in the list of abstract syntaxes
 *  where no corresponding name is defined in the UID dictionary.
 *  This method is passed a list of "preferred" transfer syntaxes.
 *  @param params pointer to association parameters structure
 *  @param transferSyntax transfer syntax to accept
 *  @param acceptedRole SCU/SCP role to accept
 */
OFCondition AssociationHelpServer::acceptUnknownContextsWithPreferredTransferSyntaxes(
    T_ASC_Parameters * params,
    const char* transferSyntaxes[], int transferSyntaxCount,
    T_ASC_SC_ROLE acceptedRole)
{
    OFCondition cond = EC_Normal;
    /*
    ** Accept in the order "least wanted" to "most wanted" transfer
    ** syntax.  Accepting a transfer syntax will override previously
    ** accepted transfer syntaxes.
    */
    for (int i=transferSyntaxCount-1; i>=0; i--)
    {
        cond = acceptUnknownContextsWithTransferSyntax(params, transferSyntaxes[i], acceptedRole);
        if (cond.bad()) return cond;
    }
    return cond;
}

//2012/03/23 K.Ko 
OFCondition
AssociationHelpServer::waitCommands(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,T_ASC_PresentationContextID &out_presID,int timeout)
    /*
     * This function receives DIMSE commmands over the network connection
     * and handles these commands correspondingly. Note that in case of
     * storscp only C-ECHO-RQ and C-STORE-RQ commands can be processed.
     *
     * Parameters:
     *   assoc - [in] The association (network connection to another DICOM application).
     */
{
  OFCondition cond = EC_Normal;
//  T_DIMSE_Message msg;
 //  T_ASC_PresentationContextID presID = 0;
   out_presID = 0;

   
  DcmDataset *statusDetail = NULL;

  // start a loop to be able to receive more than one DIMSE command
//  while( cond == EC_Normal || cond == DIMSE_NODATAAVAILABLE || cond == DIMSE_OUTOFRESOURCES )
  /*
  * donot loop here, just do once K.Ko
  */
  {
	 
    // receive a DIMSE command over the network
    if( timeout <0 )
      cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &out_presID, msg_out, &statusDetail);
    else
      cond = DIMSE_receiveCommand(assoc, DIMSE_NONBLOCKING, OFstatic_cast(int, timeout), &out_presID, msg_out, &statusDetail);

    // check what kind of error occurred. If no data was
    // received, check if certain other conditions are met
    if( cond == DIMSE_NODATAAVAILABLE )
    {
//		DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::processCommands DIMSE_receiveCommand DIMSE_NODATAAVAILABLE");

      // If in addition to the fact that no data was received also option --eostudy-timeout is set and
      // if at the same time there is still a study which is considered to be open (i.e. we were actually
      // expecting to receive more objects that belong to this study) (this is the case if lastStudyInstanceUID
      // does not equal NULL), we have to consider that all objects for the current study have been received.
      // In such an "end-of-study" case, we might have to execute certain optional functions which were specified
      // by the user through command line options passed to storescp.

    }
	if (statusDetail != NULL)
    {
      DCMLIB_LOG_DEBUG("Extra Status Detail: \n");

	  std::stringstream sting_temp;
      statusDetail->print(sting_temp);//COUT);
	  DCMLIB_LOG_DEBUG((sting_temp.str()+ ("\n")).c_str());
      delete statusDetail;
    }

  }
  return cond;
}
///
OFCondition
AssociationHelpServer::processCommands(T_ASC_Association * assoc,T_DIMSE_Message *new_msg,DcmDataset *dataset_out
									   ,DcmXTSCPResponseParam *ResponseParam)
    /*
     * This function receives DIMSE commmands over the network connection
     * and handles these commands correspondingly. Note that in case of
     * storscp only C-ECHO-RQ and C-STORE-RQ commands can be processed.
     *
     * Parameters:
     *   assoc - [in] The association (network connection to another DICOM application).
     */
{
  OFCondition cond = EC_Normal;
//  T_DIMSE_Message msg;

#if 0 ///
  //moved to AssociationHelpServer::waitCommands  2012/03/23 K.Ko
   T_ASC_PresentationContextID presID = 0;

   

  DcmDataset *statusDetail = NULL;

  // start a loop to be able to receive more than one DIMSE command
//  while( cond == EC_Normal || cond == DIMSE_NODATAAVAILABLE || cond == DIMSE_OUTOFRESOURCES )
  /*
  * donot loop here, just do once K.Ko
  */
  {
	 
    // receive a DIMSE command over the network
    if( timeout <0 )
      cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &presID, msg_out, &statusDetail);
    else
      cond = DIMSE_receiveCommand(assoc, DIMSE_NONBLOCKING, OFstatic_cast(int, timeout), &presID, msg_out, &statusDetail);

    // check what kind of error occurred. If no data was
    // received, check if certain other conditions are met
    if( cond == DIMSE_NODATAAVAILABLE )
    {
//		DCMLIB_LOG_TRACE("DcmXTAssociationServerMain::processCommands DIMSE_receiveCommand DIMSE_NODATAAVAILABLE");

      // If in addition to the fact that no data was received also option --eostudy-timeout is set and
      // if at the same time there is still a study which is considered to be open (i.e. we were actually
      // expecting to receive more objects that belong to this study) (this is the case if lastStudyInstanceUID
      // does not equal NULL), we have to consider that all objects for the current study have been received.
      // In such an "end-of-study" case, we might have to execute certain optional functions which were specified
      // by the user through command line options passed to storescp.

    }

    // if the command which was received has extra status
    // detail information, dump this information
    if (statusDetail != NULL)
    {
      DCMLIB_LOG_DEBUG("Extra Status Detail: \n");

	  std::stringstream sting_temp;
      statusDetail->print(sting_temp);//COUT);
	  DCMLIB_LOG_DEBUG((sting_temp.str()+ ("\n")).c_str());
      delete statusDetail;
    }

    // check if peer did release or abort, or if we have a valid message
    if (cond == EC_Normal)
    {
		ResponseParam->clear();

		ResponseParam->m_presIdCmd = presID;
#else

  {
    {
#endif
      // in case we received a valid message, process this command
      // note that storescp can only process a C-ECHO-RQ and a C-STORE-RQ
      switch (new_msg->CommandField)
      {
        case DIMSE_C_ECHO_RQ:
			DCMLIB_LOG_TRACE(" DIMSE_receiveCommand DIMSE_C_ECHO_RQ \n");
          // process C-ECHO-Request
            cond = echoSCP(assoc, new_msg, dataset_out,ResponseParam);
          break;
        case DIMSE_C_STORE_RQ:
			DCMLIB_LOG_TRACE(" DIMSE_receiveCommand DIMSE_C_STORE_RQ \n");
          // process C-STORE-Request
          cond = CStoreSCP(assoc, new_msg, dataset_out,ResponseParam);
	 	  cond = EC_Normal;
          break;
		case DIMSE_C_FIND_RQ:
			DCMLIB_LOG_TRACE(" DIMSE_receiveCommand DIMSE_C_FIND_RQ \n");
			cond = CFindSCP(assoc, new_msg, dataset_out,ResponseParam);
	// 		cond = EC_Normal;
		  break;
	    case DIMSE_C_CANCEL_RQ:
			DCMLIB_LOG_TRACE(" DIMSE_receiveCommand DIMSE_C_CANCEL_RQ \n");
				
	 		cond = EC_Normal;
		  break;
		case DIMSE_C_MOVE_RQ:
			DCMLIB_LOG_TRACE(" DIMSE_receiveCommand DIMSE_C_MOVE_RQ \n");
			
		 	cond = CMoveSCP(assoc, new_msg, dataset_out,ResponseParam);
			 
	 		cond = EC_Normal;
		  break;
        default:
          // we cannot handle this kind of message
          cond = DIMSE_BADCOMMANDTYPE;
		  {
		//	 char str_buff[128]; sprintf(str_buff,"0x%x",OFstatic_cast(unsigned, msg_out->CommandField));
			DCMLIB_LOG_ERROR(" DcmXTAssociationServerMain::processCommands Cannot handle command: 0x%x\n",OFstatic_cast(unsigned, new_msg->CommandField));
		  }
          break;
      }
    }
  }
  return cond;
}

extern OFGlobal<OFBool> dcmDisableGethostbyaddr ;//#50
///
OFCondition AssociationHelpServer::receiveAssociation(T_ASC_Network *net, 
												 T_ASC_Association * &assocReceive,int endOfStudyTimeout)
{

//  T_ASC_Association *assoc;
  OFCondition cond;
 

  //#50
  //IPアドレスからホスト名に変換しないようにする。
  dcmDisableGethostbyaddr.set( OFTrue);//#50
  try {

  // try to receive an association. Here we either want to use blocking or
  // non-blocking, depending on if the option --eostudy-timeout is set.
  if( endOfStudyTimeout == -1 )
    cond = ASC_receiveAssociation(net, &assocReceive, m_maxPDU, NULL, NULL, opt_secureConnection);
  else
    cond = ASC_receiveAssociation(net, &assocReceive, m_maxPDU, NULL, NULL, opt_secureConnection, DUL_NOBLOCK, OFstatic_cast(int, endOfStudyTimeout));

  if (cond.code() == DULC_FORKEDCHILD)
  {
      // if (opt_verbose) DimseCondition::dump(cond);
      throw(-1);//goto cleanup;
  }

  // if some kind of error occured, take care of it
  if (cond.bad())
  {
    // check what kind of error occurred. If no association was
    // received, check if certain other conditions are met
    if( cond == DUL_NOASSOCIATIONREQUEST )
    {
      // If in addition to the fact that no association was received also option --eostudy-timeout is set
      // and if at the same time there is still a study which is considered to be open (i.e. we were actually
      // expecting to receive more objects that belong to this study) (this is the case if lastStudyInstanceUID
      // does not equal NULL), we have to consider that all objects for the current study have been received.
      // In such an "end-of-study" case, we might have to execute certain optional functions which were specified
      // by the user through command line options passed to storescp.

    }
    // If something else was wrong we might have to dump an error message.
    else
    {
      if( opt_verbose ) DimseCondition::dump(cond);
    }

    // no matter what kind of error occurred, we need to do a cleanup
     throw(-1);//goto cleanup;
  }

  if(opt_verbose)
  {
 
      DCMLIB_LOG_DEBUG("Association Received \n");
 
  }

  if(opt_debug)
  {
	std::stringstream sting_temp;
    DCMLIB_LOG_DEBUG("Parameters: \n");
    ASC_dumpParameters(assocReceive->params,sting_temp);// COUT);
	DCMLIB_LOG_DEBUG((sting_temp.str()+ ("\n")).c_str());

    DIC_AE callingTitle;
    DIC_AE calledTitle;
    //ASC_getAPTitles(assocReceive->params, callingTitle, calledTitle, NULL);
    ASC_getAPTitles(assocReceive->params, callingTitle, sizeof(callingTitle), calledTitle, sizeof(calledTitle), NULL, 0);


    DCMLIB_LOG_DEBUG("called AE: %s  \n" ,calledTitle );
    DCMLIB_LOG_DEBUG("calling AE: %s \n" ,callingTitle);
  }

 

  /*
  * moved ! K.Ko
  */

  /* now do the real work, i.e. receive DIMSE commmands over the network connection */
  /* which was established and handle these commands correspondingly. In case of */
  /* storscp only C-ECHO-RQ and C-STORE-RQ commands can be processed. */


  //koko

#if 1
//  cond = processCommands(assoc, msg_out);
#else
  cond = EC_Normal;
#endif

  if (cond == DUL_PEERREQUESTEDRELEASE)
  {
    if (opt_verbose) printf("Association Release\n");
    cond = ASC_acknowledgeRelease(assocReceive);
  }
  else if (cond == DUL_PEERABORTEDASSOCIATION)
  {
    if (opt_verbose) printf("Association Aborted\n");
  }
  else
  {
#if 0
    fprintf(stderr, "storescp: DIMSE Failure (aborting association)\n");
    /* some kind of error so abort the association */
    cond = ASC_abortAssociation(assocReceive);
#endif
  }

  }catch(int error)
  {
	  
//	fprintf(stderr, "error  jump to cleanup \n");
  
	//cleanup:

	  if(cond.code() == DULC_FORKEDCHILD) return cond;

	  cond = ASC_dropSCPAssociation(assocReceive);
	  if (cond.bad())
	  {
		DimseCondition::dump(cond);
 
	  }
	  cond = ASC_destroyAssociation(&assocReceive);
	  if(cond.bad())
	  {
		DimseCondition::dump(cond);
 
	  }
  }

  return cond;
}
OFCondition AssociationHelpServer::acceptAssociation( T_ASC_Association * &assocReceive)
{
	OFCondition cond;

	  char buf[BUFSIZ];

const char* knownAbstractSyntaxes[] =
	  {
		UID_VerificationSOPClass
	  };


	 OFString sprofile;
	  const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
  int numTransferSyntaxes = 0;

  try {
	
  switch (m_networkTransferSyntax)
  {
    case EXS_LittleEndianImplicit:
      /* we only support Little Endian Implicit */
      transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 1;
      break;
    case EXS_LittleEndianExplicit:
      /* we prefer Little Endian Explicit */
      transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 3;
      break;
    case EXS_BigEndianExplicit:
      /* we prefer Big Endian Explicit */
      transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 3;
      break;
#if 0
    case EXS_JPEGProcess14SV1TransferSyntax:
      /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
      transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
    case EXS_JPEGProcess1TransferSyntax:
      /* we prefer JPEGBaseline (default lossy for 8 bit images) */
      transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
    case EXS_JPEGProcess2_4TransferSyntax:
      /* we prefer JPEGExtended (default lossy for 12 bit images) */
      transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
#endif
    case EXS_JPEG2000:
      /* we prefer JPEG2000 Lossy */
      transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
    case EXS_JPEG2000LosslessOnly:
      /* we prefer JPEG2000 Lossless */
      transferSyntaxes[0] = UID_JPEG2000LosslessOnlyTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
    case EXS_RLELossless:
      /* we prefer RLE Lossless */
      transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
#ifdef WITH_ZLIB
    case EXS_DeflatedLittleEndianExplicit:
      /* we prefer Deflated Explicit VR Little Endian */
      transferSyntaxes[0] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 4;
      break;
#endif
    default:
      /* We prefer explicit transfer syntaxes.
       * If we are running on a Little Endian machine we prefer
       * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
       */
      if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
      {
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
      }
      else
      {
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
      }
 
      transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 3;
 
      break;
  }

  
  {
    /* accept the Verification SOP Class if presented */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes( assocReceive->params, knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes), transferSyntaxes, numTransferSyntaxes);
    if (cond.bad())
    {
      if (opt_verbose) DimseCondition::dump(cond);
      throw(-1);//goto cleanup;
    }
#if 0
	if(m_NumAcceptStorageSOPClassUIDs>0){
		cond = ASC_acceptContextsWithPreferredTransferSyntaxes( assocReceive->params, m_AcceptStorageSOPClassUIDs, m_NumAcceptStorageSOPClassUIDs, transferSyntaxes, numTransferSyntaxes);
	    
	}else{
		/* the array of Storage SOP Class UIDs comes from dcuid.h */
		cond = ASC_acceptContextsWithPreferredTransferSyntaxes( assocReceive->params, dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs, transferSyntaxes, numTransferSyntaxes);
	    
	}
#endif
	if (cond.bad())
    {
      if (opt_verbose) DimseCondition::dump(cond);
      throw(-1);//goto cleanup;
    }

    if (opt_promiscuous)
    {
      /* accept everything not known not to be a storage SOP class */
      cond = acceptUnknownContextsWithPreferredTransferSyntaxes(
        assocReceive->params, transferSyntaxes, numTransferSyntaxes);
      if (cond.bad())
      {
        if (opt_verbose) DimseCondition::dump(cond);
        throw(-1);//goto cleanup;
      }
    }
  }

  /* set our app title */
  ASC_setAPTitles(assocReceive->params, NULL, NULL, opt_respondingaetitle);

  /* acknowledge or reject this association */
  cond = ASC_getApplicationContextName(assocReceive->params, buf,sizeof(buf));
  if ((cond.bad()) || strcmp(buf, UID_StandardApplicationContext) != 0)
  {
    /* reject: the application context name is not supported */
    T_ASC_RejectParameters rej =
    {
      ASC_RESULT_REJECTEDPERMANENT,
      ASC_SOURCE_SERVICEUSER,
      ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED
    };

    if (opt_verbose) printf("Association Rejected: bad application context name: %s\n", buf);
    cond = ASC_rejectAssociation(assocReceive, &rej);
    if (cond.bad())
    {
      if (opt_verbose) DimseCondition::dump(cond);
    }
    throw(-1);//goto cleanup;

  }
  else if (opt_rejectWithoutImplementationUID && strlen(assocReceive->params->theirImplementationClassUID) == 0)
  {
    /* reject: the no implementation Class UID provided */
    T_ASC_RejectParameters rej =
    {
      ASC_RESULT_REJECTEDPERMANENT,
      ASC_SOURCE_SERVICEUSER,
      ASC_REASON_SU_NOREASON
    };

    if (opt_verbose) printf("Association Rejected: No Implementation Class UID provided\n");
    cond = ASC_rejectAssociation(assocReceive, &rej);
    if (cond.bad())
    {
      if (opt_verbose) DimseCondition::dump(cond);
    }
    throw(-1);//goto cleanup;
  }
  else
  {
#ifdef PRIVATE_STORESCP_CODE
    PRIVATE_STORESCP_CODE
#endif
    cond = ASC_acknowledgeAssociation(assocReceive);
    if (cond.bad())
    {
      DimseCondition::dump(cond);
      throw(-1);//goto cleanup;
    }
    if (opt_verbose)
    {
      DCMLIB_LOG_DEBUG("Association Acknowledged (Max Send PDV: %d) \n",assocReceive->sendPDVLength);
      if (ASC_countAcceptedPresentationContexts(assocReceive->params) == 0)
        DCMLIB_LOG_DEBUG("    (but no valid presentation contexts) \n");
	  if (opt_debug) {
		  std::stringstream sting_temp;
		  ASC_dumpParameters(assocReceive->params, sting_temp);
		  DCMLIB_LOG_DEBUG((sting_temp.str()+ ("\n")).c_str());

	  }
    }
  }

#ifdef BUGGY_IMPLEMENTATION_CLASS_UID_PREFIX
  /* active the dcmPeerRequiresExactUIDCopy workaround code
   * (see comments in dimse.h) for a implementation class UID
   * prefix known to exhibit the buggy behaviour.
   */
  if (0 == strncmp(assoc->params->theirImplementationClassUID,
      BUGGY_IMPLEMENTATION_CLASS_UID_PREFIX,
      strlen(BUGGY_IMPLEMENTATION_CLASS_UID_PREFIX)))
  {
    dcmEnableAutomaticInputDataCorrection.set(OFFalse);
    dcmPeerRequiresExactUIDCopy.set(OFTrue);
  }
#endif

  // store calling and called aetitle in global variables to enable
  // the --exec options using them. Enclose in quotation marks because
  // aetitles may contain space characters.
  DIC_AE callingTitle;
  DIC_AE calledTitle;
  if (ASC_getAPTitles(assocReceive->params, callingTitle,sizeof(callingTitle), calledTitle,sizeof(calledTitle), NULL,0).good())
  {
    callingaetitle = "\"";
    callingaetitle += callingTitle;
    callingaetitle += "\"";
    calledaetitle = "\"";
    calledaetitle += calledTitle;
    calledaetitle += "\"";
  }
  else
  {
    // should never happen
    callingaetitle.clear();
    calledaetitle.clear();
  }

  }catch(int error)
  {
	  
//	fprintf(stderr, "error  jump to cleanup \n");
  
	//cleanup:

	  if(cond.code() == DULC_FORKEDCHILD) return cond;

	  cond = ASC_dropSCPAssociation(assocReceive);
	  if (cond.bad())
	  {
		DimseCondition::dump(cond);
	//    exit(1);
	  }
	  cond = ASC_destroyAssociation(&assocReceive);
	  if(cond.bad())
	  {
		DimseCondition::dump(cond);
	 //   exit(1);
	  }
  }

	return cond;
}
 
///


/////////////////////
OFCondition AssociationHelpServer::echoSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  DcmDataset *dataset_out,
  DcmXTSCPResponseParam *ResponseParam)
     
{

 	ResponseParam->m_echo_request = msg->msg.CEchoRQ;


  // dump some information if required
  if (opt_verbose)
  {
    DCMLIB_LOG_DEBUG("Received EchoRQ  \n");
  //  DIMSE_printCStoreRQ(stdout,req);
  }

  
#if 1

  OFString temp_str;
//  OFLOG_INFO(movescuLogger, "Received Echo Request");
//  OFLOG_DEBUG(movescuLogger, DIMSE_dumpMessage(temp_str, msg->msg.CEchoRQ, DIMSE_INCOMING));

  /* the echo succeeded !! */
  OFCondition cond = DIMSE_sendEchoResponse(assoc, ResponseParam->m_presIdCmd, &msg->msg.CEchoRQ, STATUS_Success, NULL);
  if (cond.bad())
  {
 //   OFLOG_ERROR(movescuLogger, "Echo SCP Failed: " << DimseCondition::dump(temp_str, cond));
  }
  return cond;
#else
  return ASC_NORMAL;
#endif
   
}