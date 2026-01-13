/*************************************************************************
 *
 *       System: MergeCOM-3 Integrator's Tool Kit Sample Applications
 *
 *    $Workfile: qr_scp.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: Q/R Sample Service Class Provider Application.
 *
 *************************************************************************
 *
 *		(c) 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)
 *                     Milwaukee, Wisconsin  53214
 *
 *                     -- ALL RIGHTS RESERVED --
 *
 *  This software is furnished under license and may be used and copied
 *  only in accordance with the terms of such license and with the
 *  inclusion of the above copyright notice.  This software or any other
 *  copies thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of the software is hereby
 *  transferred.
 *
 ************************************************************************/

/*
 *  Run-time version number string
 */
static char id_string[] = "$Header$";

/* $NoKeywords: $ */

/* Default is unix */
#if !defined(_MSDOS)         && !defined(_WIN32)  && !defined(_RMX3) && \
    !defined(INTEL_WCC)      && !defined(__OS2__) && !defined(UNIX) && \
    !defined(_ALPHA_OPENVMS) && !defined(OS9_68K) && !defined(OS9_PPC) && \
    !defined(VXWORKS) && !defined(_MACINTOSH)
#ifndef UNIX
#define UNIX 1
#endif
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(OS9_68K) && !defined(OS9_PPC)
#include <fcntl.h>
#endif

#if defined(_MACINTOSH)
#include <Types.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <time.h>
#include <console.h>
#include <SIOUX.h>
#endif

#ifdef UNIX
#ifdef STRINGS
#include <strings.h>
#endif
#ifdef LYNX_OS
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/wait.h>
#include <errno.h>
#endif

#include "mergecom.h"
#include "mcstatus.h"
#include "diction.h"
#include "qr.h"


/* 
 * Structure to hold Application Default Values 
 */
typedef struct app_config
{
    char    TimeOut[TM_LEN];
    char    ApplicationTitle[AE_LEN];
    char    RemoteApplicationTitle[AE_LEN];
    char    ServiceList[SERVICELIST_LEN];
    char    ImageType[FILENAME_LEN];
    char    DataFileName[FILENAME_LEN];
} AppConfig;


/* 
 * Call back information for the StreamToMessage function 
 */
typedef struct callback_info
{
   FILE *stream;
   int   messageID;
   short buffer[1024];
} CBinfo;


/* 
 * Function Prototypes 
 */
static void GetOptions             ( int           A_argc, 
                                     char**        A_argv, 
                                     AppConfig*    A_myConfig );
static void SetProgramDefaults     ( AppConfig*    A_myConfig );
static QR_STATUS HandleAssociation ( int           A_applicationID, 
                                     int           A_associationID,
                                     AppConfig*    A_myConfig ); 
static QR_STATUS ProcessCFINDRQ    ( int           A_associationID,
                                     int           A_messageID,
                                     AppConfig*    A_myConfig,
                                     int*          A_sendBack );
static RESP_STATUS ProcessCMOVERQ  ( int           A_applicationID,
                                     int           A_messageID,
                                     AppConfig*    A_myConfig );
static QR_STATUS SendCFINDReply    ( int           A_associationID, 
                                     int*          A_bIDarray,
                                     AppConfig*    A_myConfig, 
                                     RESP_STATUS   A_retStatus,
                                     int*          A_sendBack, 
                                     int           A_messageID );
static MC_STATUS StreamToMessage   ( int           A_msgID, 
                                     void*         A_cbInformation,
                                     int           A_firstCall, 
                                     int*          A_dataLen,
                                     void**        A_dataBuffer, 
                                     int*          A_isLast );
static QR_STATUS SearchForKeyMatch ( int           A_field, 
                                     char*         A_key, 
                                     int*          A_bIDArray, 
                                     char*         A_filename );
static QR_STATUS PerformDatabaseSearch 
                                   ( AppConfig*    A_myConfig, 
                                     char*         A_queryLevel, 
                                     int           A_messageID, 
                                     unsigned long A_tag, 
                                     int*          A_bIDarray,
                                     RESP_STATUS*  A_retStatus, 
                                     int*          A_sendBack);
                                     

/*****************************************************************************
**
** NAME
**    main - Query/Retrieve provider main program
**
** SYNOPSIS
**    qr_scp
**
** DESCRIPTION
**    This is the main routine of the Query/Retrieve SCP sample application.
**    It waits for associations, and then calls Handle_Association when one
**    arrives.
**
*****************************************************************************/
#ifdef VXWORKS
int  qrscp(int argc, char** argv);
int qrscp(int argc, char** argv)
#else
int main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    AppConfig    myConfig;
    MC_STATUS    status;
    QR_STATUS    qrStatus;
    int          applicationID;
    int          calledApplicationID;
    int          associationID;
    AssocInfo    assocInfo;

#if defined(_MACINTOSH) && defined(__MWERKS__)
    EventRecord  asEvent;
    Boolean      aqQuit = false;

	SIOUXSettings.initializeTB = true;
	SIOUXSettings.standalone = true;
	SIOUXSettings.setupmenus = true;
	SIOUXSettings.autocloseonquit = false;
	SIOUXSettings.asktosaveonclose = true;
	SIOUXSettings.showstatusline = true;
	argc = ccommand(&argv);
#endif

    /*
     * Set up the default variables and get the command line options
     */
    SetProgramDefaults ( &myConfig );
    GetOptions ( argc, argv, &myConfig );

    /*
     * Print the application header to the screen 
     */
    printf ( "\n" );
    printf ( "QR_SCP - Query/Retrieve Storage Class Provider.\n" );
	printf ( "(c) 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)\n");
	printf ( "All rights reserved.\n\n");
    printf ( "BEGINNING QR_SCP.\n\n" );
  
   
    /* ------------------------------------------------------- */ 
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */ 
    status = MC_Library_Initialization ( NULL, NULL, NULL ); 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintErrorMessage( myConfig.ApplicationTitle, 
                           "Unable to initialize library",
                           status, NULL );
        return ( EXIT_FAILURE );
    }

  
    /*
     * Register the Q/R SCP's local application title
     */
    status = MC_Register_Application ( &applicationID,
                                       myConfig.ApplicationTitle );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( myConfig.ApplicationTitle, 
                            "MC_Register_Application",
                            status, NULL );
        return ( EXIT_FAILURE );
    }

   printf ( "%s: Waiting for an association.\n", myConfig.ApplicationTitle );

    DO_FOREVER
    {
#if defined(_MACINTOSH)
		if (WaitNextEvent(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask,
								&asEvent, 10, NULL)) {
			if (asEvent.what == keyDown) {
				unsigned char	abChar = asEvent.message & charCodeMask;
				Boolean			aqCmnd = ((asEvent.modifiers & cmdKey) != 0);
				
				if ((abChar == 'Q') || (abChar == 'q') || (aqCmnd && (abChar == '.')))
					aqQuit = true;
			} else if (asEvent.what == mouseDown) {
				WindowPtr	apWindow;
				short			awWhere;
				
				awWhere = FindWindow(asEvent.where, &apWindow);
				if (awWhere == inMenuBar) {
					MenuSelect(asEvent.where);
					HiliteMenu(0);
				} else if (awWhere == inSysWindow)
					SystemClick(&asEvent, apWindow);
			}
		}
#if defined(__MWERKS__)
		else
			SIOUXHandleOneEvent(&asEvent);
#endif
		if (aqQuit)
			break;
#endif    

        /*
         * Wait for an assoc. request from the remote DICOM app. 
         */
        status = MC_Wait_For_Association (   myConfig.ServiceList,
                                             0,
                                             &calledApplicationID,
                                             &associationID );
        if (status == MC_TIMEOUT)
            continue;
        else if ( status == MC_NEGOTIATION_ABORTED )
        {
            printf("%s: Association aborted during negotiation.\n",myConfig.ApplicationTitle);
            continue;
        }
        else if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( myConfig.ApplicationTitle, "MC_Wait_For_Association",
                                status, NULL );
            break;
        }
        
        if ( calledApplicationID != applicationID )
        {
            MC_Reject_Association ( associationID, PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED );
            PrintErrorMessage ( myConfig.ApplicationTitle, "MC_Wait_For_Association", -1,
                                "Unexpected application identifier from MC_Wait_For_Association" );
            break;
        }
        
        printf ( "%s: Association received\n", myConfig.ApplicationTitle );
       
        /*
         * Get the remote AE title.  This is needed later to fill in
         * several a field in the C-STORE-RQ messages.
         */ 
        status = MC_Get_Association_Info(associationID, &assocInfo);
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Reject_Association ( associationID, TRANSIENT_NO_REASON_GIVEN );
            PrintErrorMessage ( myConfig.ApplicationTitle, 
                                "MC_Get_Association_Info", status,
                                "Originator application entity title" );
            break;        
        }

        strcpy(myConfig.RemoteApplicationTitle,assocInfo.RemoteApplicationTitle);
        
        /*
         * Handle the association.  A real Q/R SCP would fork a child to handle
         * the association, or perhaps create a thread.
         */
        qrStatus = HandleAssociation ( applicationID, associationID, &myConfig );
        if ( qrStatus != QR_SUCCESS )
        {
            MC_Abort_Association ( &associationID );
            MC_Release_Application ( &applicationID );
            PrintErrorMessage ( myConfig.ApplicationTitle, "HandleAssociation", -1,
                                "Unexpected return value from HandleAssociation" );
            break;        
        }
    }


    /*
     * We're finished: release the application 
     */
    status = MC_Release_Application ( &applicationID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( myConfig.ApplicationTitle, 
                            "MC_Release_Application", status, NULL );
        return ( EXIT_FAILURE );
    }

    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");

    printf ( "... %s FINISHED.\n\n",myConfig.ApplicationTitle );
   
    return ( EXIT_SUCCESS );
   
} /* main */



/*****************************************************************************
**
** NAME
**    HandleAssociation - Process the new DICOM association
**
** ARGUMENTS
**    A_applicationID    int            Application ID
**    A_associationID    int            Association ID for the first assocation
**    A_myConfig         AppConfig *    Configuration struct for file name
**
** DESCRIPTION
**    This routine handles the newly opened association.  It reads a message
**    from the network then processes that message by calling the appropriate
**    processing routine.
**
** RETURNS
**    QR_SUCCESS if the routine finishes properly.
**    QR_FAILURE if the routine detects an error.
**
** SEE ALSO
**    PrintErrorMessage
**    ProcessCFINDRQ
**    ProcessCMOVERQ
**    MC_Accept_Association
**    MC_Free_Message
**    MC_Get_Value_To_String
**    MC_Read_Message
**
*****************************************************************************/
static QR_STATUS HandleAssociation ( int A_applicationID, 
                                     int A_associationID,
                                     AppConfig* A_myConfig )
{
    static char       S_prefix[] = "HandleAssociation";
    MC_STATUS         status;
    QR_STATUS         qrStatus;
    MC_COMMAND        command;
    RESP_STATUS       retStatus;
    char              *serviceName;
    int               messageID;
    int               rspMsgID;
    time_t            startTime;
    int               sendBack[DATABASE_ENTRIES];

   
    INIT_ARRAY(sendBack, DATABASE_ENTRIES, 0);
    time ( &startTime );
   
    /*
     * Accept the remote apps request for a DICOM session 
     */
    status = MC_Accept_Association ( A_associationID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Accept_Association",
                            status, NULL );
        return ( QR_FAILURE );
    }
   
    DO_FOREVER
    {
        /*
         * Wait for the remote DICOM app to send a message 
         */
        status = MC_Read_Message ( A_associationID,
                                   atoi ( A_myConfig->TimeOut ),
                                   &messageID,
                                   &serviceName,
                                   &command );
        if (status == MC_TIMEOUT)
        {
            printf("Timed out waiting for a request message.  Continueing.\n");
            continue;
        }
        else if (status == MC_ASSOCIATION_CLOSED)
            return ( QR_SUCCESS );
        else if (status == MC_NETWORK_SHUT_DOWN
              || status == MC_ASSOCIATION_ABORTED)
        {
            PrintErrorMessage ( S_prefix, "MC_Read_Message",
                                status, NULL );
            return ( QR_SUCCESS );
        }
        else if (status != MC_NORMAL_COMPLETION)
        {
            MC_Free_Message ( &messageID );
            MC_Abort_Association ( &A_associationID );
            PrintErrorMessage ( S_prefix, "MC_Read_Message",
                                status, NULL );
            return ( QR_FAILURE );
        }
        
        switch ( command )
        {
            case C_FIND_RQ:
                printf ( "%s: --- Received a C_FIND_RQ request.\n", S_prefix );
                qrStatus = ProcessCFINDRQ ( A_associationID,
                                            messageID, 
                                            A_myConfig, 
                                            sendBack );
                if ( qrStatus != QR_SUCCESS )
                {
                    MC_Free_Message ( &messageID );
                    PrintErrorMessage ( S_prefix, "ProcessCFINDRQ", -1,
                                        "Error processing find message" );
                    return ( QR_FAILURE );
                }
               
                status = MC_Free_Message ( &messageID );
                if (status != MC_NORMAL_COMPLETION)
                {
                    PrintErrorMessage( S_prefix, "MC_Free_Message failed", status, NULL);
                    return ( QR_FAILURE );
                } 
                
                /*
                 * Note that the ProcessCFINDRQ function sends the 
                 * appropriate response messages.  We do not have to
                 * send one here.
                 */
                printf ( "%s: --- Done processing C_FIND_RQ\n", S_prefix );
                break;
                    
            case C_MOVE_RQ:
                /*
                 * Process the move request
                 */
                retStatus = ProcessCMOVERQ ( A_applicationID, 
                                             messageID, 
                                             A_myConfig );
                    
                /*
                 * Send back an empty response message signaling the query is finished 
                 */
                status = MC_Open_Message(&rspMsgID,serviceName,C_MOVE_RSP);
                if (status != MC_NORMAL_COMPLETION)
                {
                    PrintErrorMessage( S_prefix, "Unable to open empty request message", status, NULL);
                    return ( QR_FAILURE );
                }
               
                /*
                 * Free the message.  Note that this is done after the MOVE_RSP is
                 * created because the serviceName used in MC_Open_Message is released
                 * when the message is free'd causing a potential memory access problem.
                 */ 
                status = MC_Free_Message ( &messageID );
                if (status != MC_NORMAL_COMPLETION)
                {
                    PrintErrorMessage( S_prefix, "MC_Free_Message failed", status, NULL);
                    return ( QR_FAILURE );
                }   

                status = MC_Send_Response_Message ( A_associationID,
                                                    retStatus,
                                                    rspMsgID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &rspMsgID );
                    PrintErrorMessage ( S_prefix, "MC_Send_Response_Message",
                                        status, NULL );
                    return ( QR_FAILURE );
                }

                status = MC_Free_Message(&rspMsgID);
                if (status != MC_NORMAL_COMPLETION)
                {
                    PrintErrorMessage( S_prefix, "MC_Free_Message", status, NULL);
                    return ( QR_FAILURE );
                }   
                 
                printf ( "%s: --- Done processing C_MOVE_RQ\n", S_prefix );
                break;
                    
        } /* switch command */
             
    } /* DO_FOREVER */
} /* HandleAssociation */



/*****************************************************************************
**
** NAME
**    ProcessCFINDRQ - Process a C-FIND Request
**
** ARGUMENTS
**    A_associationID     int              Association ID 
**    A_messageID         int              Message ID
**    A_myConfig          AppConfig *      Structure for data file name
**    A_sendBack          int *            Array of info to send back
**
** DESCRIPTION
**    This routine processes a C-FIND request.  It looks at the search
**    fields at the patient and study level and calls the search function
**    to find them in the data file.  The ID array is used to determine 
**    which lines in the data base are matched.  With each search field
**    non-matching lines in the ID array are marked.  After each search 
**    field is examined, the remaining fields that are still marked
**    are returned as a match to the user.
**
** RETURNS
**    QR_SUCCESS if the routine finishes properly.
**    QR_FAILURE if the routine detects an error.
**
** SEE ALSO
**    MC_Get_Value_To_String
**    PerformDatabaseSearch
**
*****************************************************************************/
static QR_STATUS ProcessCFINDRQ ( int    A_associationID, 
                                  int    A_messageID,
                                  AppConfig*   A_myConfig, 
                                  int*   A_sendBack)
{
    QR_STATUS             qrStatus;
    MC_STATUS             status;
    char                  queryLevel[LO_LEN];
    char                  pat_id[LO_LEN]; 
    int                   matchFieldArray[DATABASE_ENTRIES];
    int                   messageOK = FALSE;
    unsigned long         tag;
    MC_VR                 vr;
    int                   numValues;
    RESP_STATUS           retStatus = C_FIND_PENDING;
     
    static char           S_prefix[] = "ProcessCFINDRQ";
   
    INIT_ARRAY(matchFieldArray, DATABASE_ENTRIES, TRUE);

    /*
     * Get the query level from request message.  This is used so we know
     * to look at the patient or study level keys. 
     */
    status = MC_Get_Value_To_String ( A_messageID, 
                                      MC_ATT_QUERY_RETRIEVE_LEVEL,
                                      sizeof(queryLevel), queryLevel );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix,
            "QUERY_RETRIEVE_LEVEL, MC_Get_Value_To_String", status, NULL );
        retStatus = C_FIND_FAILURE_INVALID_DATASET;
    }
   
    /*
     * Get the unique keys based on the level the query is.
     */ 
    if (!strncmp ( queryLevel, STUDY_LEVEL, sizeof(STUDY_LEVEL) ))
    {
        /*
         * Study level query, check the patient ID
         */
        status = MC_Get_Value_To_String ( A_messageID,
                                          MC_ATT_PATIENT_ID,
                                          sizeof(pat_id),
                                          pat_id );
        if (status == MC_NORMAL_COMPLETION)
        {
            qrStatus = SearchForKeyMatch ( PID,  
                                           pat_id,  
                                           matchFieldArray,  
                                           A_myConfig->DataFileName );
            if (qrStatus == QR_SUCCESS)
            {
                messageOK = TRUE;
            }
            else
                retStatus = C_FIND_FAILURE_INVALID_DATASET;
        }
        else
        {
            retStatus = C_FIND_FAILURE_INVALID_DATASET;
            PrintErrorMessage ( S_prefix, "MC_Get_Value_To_String for patient ID", status, NULL);
        }
    }
  
    /*
     * If an error occured that return status has changed from
     * C_FIND_PENDING.
     */ 
    if (retStatus == C_FIND_PENDING)
    {
        /*
         * Traverse through all of the fields in the message and call
         * our "database" search function to determine what matches
         * we have found. 
         */
        status = MC_Get_First_Attribute(A_messageID,&tag,&vr,&numValues);
        while (status == MC_NORMAL_COMPLETION) 
        {
            /*
             * Skip the command group elements in the message, the 
             * query/retrieve level tag, and tags that do not have a
             * value.
             */
            if ((tag & 0xffff0000)
             && (tag != MC_ATT_QUERY_RETRIEVE_LEVEL)
             && numValues)
            { 
                /*
                 * Try to match the tag to the particular message
                 */       
                qrStatus = PerformDatabaseSearch ( A_myConfig, queryLevel, A_messageID, tag, matchFieldArray,
                                                   &retStatus, A_sendBack);
                if (qrStatus == QR_SUCCESS)
                    messageOK = TRUE;
            }
                         
            status = MC_Get_Next_Attribute(A_messageID,&tag,&vr,&numValues);
        }
    }
      
    /*
     * Make sure the message was valid 
     */
    if ( TRUE != messageOK )
    {
        retStatus = C_FIND_FAILURE_INVALID_DATASET;
    }

    /*
     * Send the requested info back to the user 
     */
    qrStatus = SendCFINDReply ( A_associationID, matchFieldArray, A_myConfig,
                                retStatus, A_sendBack, A_messageID);
    if ( qrStatus != QR_SUCCESS )
    {
        PrintErrorMessage ( S_prefix, "SendCFINDReply", status,
                            "Error occured in SendCFINDReply");
        return ( QR_FAILURE ); 
    }   
    
    return ( QR_SUCCESS );
} /* ProcessCFINDRQ */



/*****************************************************************************
**
** NAME
**    ProcessCMOVERQ - Process a C-MOVE request.
**
** ARGUMENTS
**    A_applicationID     int            Application ID.
**    A_messageID         int            Message ID. 
**    A_myConfig          AppConfig *    Config struct for data file name
**
** DESCRIPTION
**    This function is used to process a C-MOVE request.  When this routine
**    executes, it acts as a C-STORE Service Class User.  It opens an
**    association with a remote application, opens a store request message,
**    streams the message in from a file and sends the message to the remote
**    application.
**
** RETURNS
**    C_MOVE_SUCCESS_NO_FAILURES
**    C_MOVE_FAILURE_INVALID_DATASET
**    C_MOVE_FAILURE_REFUSED_CANT_CALCULATE
**    C_MOVE_FAILURE_UNABLE_TO_PROCESS
**    C_MOVE_FAILURE_REFUSED_CANT_PERFORM
**
** SEE ALSO
**    MC_Abort_Association
**    MC_Close_Association
**    MC_Free_Message
**    MC_Get_Value_To_String
**    MC_Get_Value_To_UInt
**    MC_Open_Association
**    MC_Open_Message
**    MC_Send_Request_Message
**    MC_Stream_To_Message
**    PrintErrorMessage
**    WriteToListFile
**
*****************************************************************************/
static RESP_STATUS ProcessCMOVERQ ( int        A_applicationID, 
                                    int        A_messageID, 
                                    AppConfig* A_myConfig )
{
 
    static char   S_prefix[]="ProcessCMOVERQ";
    FILE         *fp;
    MC_STATUS     status;
    MC_COMMAND    command;
    CBinfo        callBackInfo;
    int           associationID;
    int           messageID;
    int           responseMessageID;
    unsigned int  dicomMessageID;
    int           i;
    unsigned int  response;
    unsigned long errorTagPtr;
    char          pat_id[LO_LEN],
                  pat_name[PN_LEN], 
                  pat_bday[DA_LEN], 
                  pat_sex[CS_LEN],
                  stu_inst[UI_LEN], 
                  stu_date[DA_LEN], 
                  stu_time[TM_LEN], 
                  stu_anum[CS_LEN],
                  stu_id[CS_LEN],
                  stu_desc[LO_LEN];
    char          *serviceName;
    char          serviceName2[SERVICENAME_LEN];
    char          level[LO_LEN];
    char          movePatientID[LO_LEN];
    char          moveStudyUID[UI_LEN];
    char          SOPClassUID[UI_LEN];
    char          remoteApplicationTitle[AE_LEN];
    char          affectedSOPinstance[UI_LEN];


    /*
     * At this point the SCP turns into a C-STORE SCU.  Get
     * the AE title to move the messages to. 
     */
    status = MC_Get_Value_To_String ( A_messageID,
                                      MC_ATT_MOVE_DESTINATION,
                                      sizeof(remoteApplicationTitle),
                                      remoteApplicationTitle );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "AE Title, MC_Get_Value_To_String",
                            status, NULL );
        return C_MOVE_FAILURE_INVALID_DATASET;
    }

    /*
     * Determine what level we are at so that we can process the move request
     * properly.
     */
    status = MC_Get_Value_To_String ( A_messageID, MC_ATT_QUERY_RETRIEVE_LEVEL,
                                      sizeof ( level ), level );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "QUERY_RETRIEVE_LEVEL, MC_Get_Value_To_String", 
                            status, NULL );
        return C_MOVE_FAILURE_INVALID_DATASET;
    }

    /*
     * Get the patient ID & study instance UID for the move.
     */
    status = MC_Get_Value_To_String ( A_messageID,
                                      MC_ATT_PATIENT_ID,
                                      sizeof(movePatientID),
                                      movePatientID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "Patient ID, MC_Get_Value_To_String",
                            status, NULL );
        return C_MOVE_FAILURE_INVALID_DATASET;
    }

    printf ( "%s: --- Received a C_MOVE_RQ request for %s.\n",
             S_prefix, movePatientID );

    for ( i = 0; i < (int)strlen ( movePatientID); i++ )
        if ( movePatientID[i] == ' ' ) movePatientID[i] = '\0';
        
        
    if (!strncmp ( level, STUDY_LEVEL, sizeof(STUDY_LEVEL) ))
    {
        status = MC_Get_Value_To_String ( A_messageID,
                                          MC_ATT_STUDY_INSTANCE_UID,
                                          sizeof(moveStudyUID),
                                          moveStudyUID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "Study Instance UID, MC_Get_Value_To_String",
                                status, NULL );
            return C_MOVE_FAILURE_INVALID_DATASET;
        }
    }
     
    printf("Remote AE Title for move: %s\n", remoteApplicationTitle);

    /*
     * Estabish an association 
     */
    status = MC_Open_Association ( A_applicationID,
                                   &associationID,
                                   remoteApplicationTitle,
                                   NULL,
                                   NULL,
                                   NULL );
    if ( status == MC_ASSOCIATION_REJECTED )
    {
        return C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
    }
    else if ( status == MC_INVALID_APPLICATION_TITLE
           || status == MC_INVALID_SERVICE_LIST_NAME
           || status == MC_INVALID_HOST_NAME
           || status == MC_CONFIG_INFO_MISSING
           || status == MC_CONFIG_INFO_ERROR
           || status == MC_INVALID_PORT_NUMBER )
    {
        return C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN;
    }
    else if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Association", status, NULL );
        return C_MOVE_FAILURE_INVALID_DATASET;
    }

    /*
     * We use the data file test.dat for this application.  In a normal
     * Q/R SCP there would be database calls in this area.
     */
    if ( NULL == (fp = fopen ( A_myConfig->DataFileName, TEXT_READ ) ) )
    {
        PrintErrorMessage ( S_prefix, "fopen", -1,
                "Could not open data file." );
        MC_Abort_Association ( &associationID );
        return C_MOVE_FAILURE_REFUSED_CANT_CALCULATE;
    }


    /*
     * Loop through all data items 
     */
    for ( i = 0; i < DATABASE_ENTRIES; i++ )
    {
        /*
         * Get the data from the file 
         */
        fscanf ( fp, FMT_STR, pat_id, pat_name, pat_bday, pat_sex,
                stu_inst, stu_date, stu_time, stu_anum, stu_id, stu_desc );

        if ( strcmp ( movePatientID, pat_id ) )
            continue; 
        
        if (!strncmp ( level, STUDY_LEVEL, sizeof(STUDY_LEVEL) ))
        {
            if ( strcmp ( moveStudyUID, stu_inst ) )
                continue; 
        }
            
        /* 
         * If this is a match, send it 
         */
        status = MC_Open_Empty_Message(&messageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage(S_prefix,"Unable to empty request message",status,NULL);
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        /*
         * Stream in the data 
         */
        if ( NULL == ( callBackInfo.stream = fopen ( A_myConfig->ImageType, BINARY_READ) ) )
        {
            PrintErrorMessage ( S_prefix, "fopen", -1, "Couldn't open Image File" );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        callBackInfo.messageID = messageID;

        status = MC_Stream_To_Message ( messageID,
                                        MC_ATT_GROUP_0008_LENGTH,
                                        0xffFFffFF,
                                        IMPLICIT_LITTLE_ENDIAN,
                                        &errorTagPtr,
                                        (void *)&callBackInfo,
                                        StreamToMessage );
        if ( status != MC_NORMAL_COMPLETION )
        {
            printf ( "***\t%s: ErrorTagPtr = %lx.\n", S_prefix, errorTagPtr );
            PrintErrorMessage ( S_prefix, "MC_Stream_To_Message", status, NULL );
            fclose ( fp );
            fclose ( callBackInfo.stream );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        fclose ( callBackInfo.stream );

        /* 
         * Get the SOP class UID and set the service 
         */
        status = MC_Get_Value_To_String( messageID, 
                                         MC_ATT_SOP_CLASS_UID,
                                         sizeof(SOPClassUID),
                                         SOPClassUID);
        if (status != MC_NORMAL_COMPLETION)
        {  
            PrintErrorMessage(S_prefix, "SOP,MC_Get_Value_To_String failed",
                              status,NULL);
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
            
        status = MC_Get_MergeCOM_Service(SOPClassUID, serviceName2, sizeof(serviceName2));
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage(S_prefix, "MC_Get_MergeCOM_Service failed",status,NULL);
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
    
        status = MC_Set_Service_Command(messageID, serviceName2, C_STORE_RQ);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage(S_prefix,"MC_Set_Service_Command failed",status,NULL);
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }


        /*
         * We now override the fields in the sample image with the 
         * values in our "database".
         */

        /*
         * Send back the patient ID 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_PATIENT_ID,
                                            pat_id );
     
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        /*
         * Send back the patient name 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_PATIENTS_NAME,
                                            pat_name );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String", status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
     
        /*
         * Send back the patient birthday 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_PATIENTS_BIRTH_DATE,
                                            pat_bday );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
     
        /*
         * Send back the patient sex? 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_PATIENTS_SEX,
                                            pat_sex );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
        
        /*
         * Send back the study instance 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_STUDY_INSTANCE_UID,
                                            stu_inst );
     
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
     
        /*
         * Send back the study date 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_STUDY_DATE,
                                            stu_date );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
      
        /*
         * Send back the study time 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_STUDY_TIME,
                                            stu_time );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
        
        /*
         * Send back the study accession number 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_ACCESSION_NUMBER,
                                            stu_anum );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
        
        /*
         * Send back the study id 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_STUDY_ID,
                                            stu_id );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
       
        /*
         * Send back the study description 
         */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_STUDY_DESCRIPTION,
                                            stu_desc );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                    status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
        
        /*
         * Get affected SOP Instance UID 
         */
        status = MC_Get_Value_To_String(messageID,
                        MC_ATT_SOP_INSTANCE_UID,
                        sizeof(affectedSOPinstance),
                        affectedSOPinstance);
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "affected SOP,MC_Get_Value_To_String", 
                    status, "Affected SOP Instance UID" );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        /*
         * Set the affected SOP Instance UID 
         */
        status = MC_Set_Value_From_String(messageID,
                          MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                          affectedSOPinstance);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage( S_prefix, "MC_Set_Value_From_String",
                               status, "Affected SOP Instance UID" );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        /*
         * Set the move originator application entity title 
         */
        status = MC_Set_Value_From_String(messageID,
                          MC_ATT_MOVE_ORIGINATOR_APPLICATION_ENTITY_TITLE,
                          A_myConfig->RemoteApplicationTitle);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage( S_prefix, "MC_Set_Value_From_String",
                               status, "Originator Application Entity Title" );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
        
        
        /*
         * Get the move originator message ID.  The "DICOM Message ID" is
         * different from this message ID which is containe din the 
         * C-MOVE-RQ message's group 0 elements (and usually handled 
         * automatically by the tool kit). 
         */
        status = MC_Get_Value_To_UInt(A_messageID,
                          MC_ATT_MESSAGE_ID,
                          &dicomMessageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage( S_prefix, "MC_Set_Value_From_String",
                               status, "Move Originator Message ID" );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }
        /*
        
         * Set the move originator message ID 
         */
        status = MC_Set_Value_From_UInt(messageID,
                          MC_ATT_MOVE_ORIGINATOR_MESSAGE_ID,
                          dicomMessageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintErrorMessage( S_prefix, "MC_Set_Value_From_String",
                               status, "Move Originator Message ID" );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_UNABLE_TO_PROCESS;
        }

        /* 
         * Send it off 
         */
        status = MC_Send_Request_Message ( associationID,
                                           messageID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Send_Request_Message", status,
                        NULL );
            MC_Abort_Association ( &associationID );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
        }
    
        /*
         * Wait for a reply 
         */
        status = MC_Read_Message ( associationID,
                                   atoi ( A_myConfig->TimeOut ),
                                   &responseMessageID,
                                   &serviceName,
                                   &command );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Read_Message", status, NULL );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
        }
    
        status = MC_Get_Value_To_UInt ( responseMessageID,
                                        MC_ATT_STATUS,
                                        &response );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Get_Value_To_UInt for C-STORE status",
                    status, NULL );
            MC_Free_Message ( &responseMessageID );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
        }
 
        /*
         * Check for a successful C-STORE 
         */
        if ( response != C_STORE_SUCCESS )
        {
            PrintErrorMessage ( S_prefix, "Received message error",
                    -1, "C_STORE_SUCCESS not obtained." );
            MC_Free_Message ( &responseMessageID );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
        }
      
        status = MC_Free_Message ( &messageID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Free_Message for C-STORE-RQ",
                    status, NULL );
            MC_Free_Message ( &responseMessageID );
            fclose ( fp );
            MC_Abort_Association ( &associationID );
            return C_MOVE_FAILURE_REFUSED_CANT_PERFORM;
        }
  

    } /* for */

    fclose ( fp );

    /*
     * Clean Up 
     */
    status = MC_Close_Association ( &associationID );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintErrorMessage ( S_prefix, "MC_Close_Association",
                            status, "Could not close association." );
    }

    return C_MOVE_SUCCESS_NO_FAILURES;
} /* ProcessCMOVERQ */

/*****************************************************************************
**
** NAME
**    SendCFINDReply - Send a C-FIND reply message.
**
** ARGUMENTS
**    A_associationID    int              The ID of the first association.
**    A_matchFieldArray         int *            The ID array.
**    A_myConfig         AppConfig *      Config structure for data file name
**    A_retStatus        RESP_STATUS      Return status from operation
**    A_sendBack         int *            Array of info to send back
**    A_messageID        int              The message ID of the request
**
** DESCRIPTION
**   This routine assembles a reply message to a C-FIND request.  If the
**   user has requested that certain fields be retrieved, this routine
**   adds them to the reply message and sends it to the remote application.
**
** RETURNS
**    QR_SUCCESS if the routine finishes properly.
**    QR_FAILURE if the routine detects an error.
**
** SEE ALSO
**    MC_Empty_Message
**    MC_Free_Message
**    MC_Open_Message
**    MC_Send_Response_Message
**    MC_Set_Value_From_String
**    PrintErrorMessage
**
*****************************************************************************/
static QR_STATUS SendCFINDReply ( int A_associationID, int *A_matchFieldArray,
                                  AppConfig *A_myConfig, RESP_STATUS A_retStatus,
                                  int *A_sendBack, int A_messageID )
{
    FILE                 *fp;
    MC_STATUS             status;
    int                   messageID;
    int                   ackMsg;
    int                   readMessageID;
    int                   gotCancel = FALSE;
    int                   ackSent = FALSE;
    int                   i = 0;
    char                  level[CS_LEN];
    char                  pat_id[LO_LEN], 
                          pat_name[PN_LEN], 
                          pat_bday[DA_LEN], 
                          pat_sex[CS_LEN],
                          stu_inst[UI_LEN], 
                          stu_date[DA_LEN], 
                          stu_time[TM_LEN], 
                          stu_anum[CS_LEN],
                          stu_id[CS_LEN],
                          stu_desc[LO_LEN];
    char                  *serviceName;
    MC_COMMAND            command = 0;

    static char           S_prefix[] = "SendCFINDReply";


    /*
     * Get the query level from request message 
     */
    status = MC_Get_Value_To_String ( A_messageID, MC_ATT_QUERY_RETRIEVE_LEVEL,
        sizeof ( level ), level );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix,
            "QUERY_RETRIEVE_LEVEL, MC_Get_Value_To_String", status, NULL );
        return ( QR_FAILURE );
    }


    /*
     * Open the data file 
     */
    if ( NULL == (fp = fopen ( A_myConfig->DataFileName, TEXT_READ ) ) )
    {
        PrintErrorMessage ( S_prefix, "fopen", -1,
                "Could not open data file." );
        return ( QR_FAILURE );
    }

    /*
     * Open a message for the C-FIND-RSP 
     */
    status = MC_Open_Message ( &messageID,
                               "PATIENT_STUDY_ONLY_QR_FIND",
                               C_FIND_RSP );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Message",
                            status, NULL );
        return ( QR_FAILURE );
    }
               
    /*
     * Build the contents of the message 
     */
    if (A_retStatus == C_FIND_PENDING
     || A_retStatus == C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT)
    {
        while( ( i < DATABASE_ENTRIES ) && ( gotCancel == FALSE ) )
        {

            /*
             * Get the data from the file 
             */
            fscanf ( fp, FMT_STR, pat_id, pat_name, pat_bday, pat_sex,
                     stu_inst, stu_date, stu_time, stu_anum, stu_id, stu_desc );
          
            if ( A_matchFieldArray[i] == FOUND )
            {
                printf ( "%s: Match Found: Patient Name = %s\n", S_prefix, pat_name );
                
                /*
                 * Send back necessary DICOM fields 
                 */

                /*
                ** Send back the Retrieve AE Title 
                */
                status = MC_Set_Value_From_String ( messageID,
                    MC_ATT_RETRIEVE_AE_TITLE, A_myConfig->ApplicationTitle );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    PrintErrorMessage ( S_prefix,
                        "RETRIEVE_AE_TITLE, MC_Set_Value_From_String",
                         status, NULL );
                    return ( QR_FAILURE );
                }

                /*
                 * Send back the query level 
                 */
                status = MC_Set_Value_From_String ( messageID,
                    MC_ATT_QUERY_RETRIEVE_LEVEL, level );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    PrintErrorMessage ( S_prefix,
                        "QUERY_RETRIEVE_LEVEL, MC_Set_Value_From_String",
                         status, NULL );
                    return ( QR_FAILURE );
                }

                /* 
                 * Send back the patient ID. 
                 */
                status = MC_Set_Value_From_String ( messageID,
                                                    MC_ATT_PATIENT_ID,
                                                    pat_id );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &messageID );
                    PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                            status, NULL );
                    return ( QR_FAILURE );
                }

                /*
                 * Send back the patient name? 
                 */
                if ( A_sendBack[PN] == TRUE )
                {
                    status = MC_Set_Value_From_String ( messageID,
                                                        MC_ATT_PATIENTS_NAME,
                                                        pat_name );
                    if ( status != MC_NORMAL_COMPLETION )
                    {
                        MC_Free_Message ( &messageID );
                        PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                               status, NULL );
                        return ( QR_FAILURE );
                    }
         
                }

                /*
                 * Send back the patient birthday? 
                 */
                if ( A_sendBack[PBD] == TRUE )
                {
                    status = MC_Set_Value_From_String ( messageID,
                                                        MC_ATT_PATIENTS_BIRTH_DATE,
                                                        pat_bday );
                    if ( status != MC_NORMAL_COMPLETION )
                    {
                        MC_Free_Message ( &messageID );
                        PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                status, NULL );
                        return ( QR_FAILURE );
                    }
                }


                /*
                 * Send back the patient sex? 
                 */
                if ( A_sendBack[PS] == TRUE )
                {
                    status = MC_Set_Value_From_String ( messageID,
                                                        MC_ATT_PATIENTS_SEX,
                                                        pat_sex );
                    if ( status != MC_NORMAL_COMPLETION )
                    {
                        MC_Free_Message ( &messageID );
                        PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                status, NULL );
                        return ( QR_FAILURE );
                    }
        
                } 

                if (!strncmp ( level, STUDY_LEVEL, sizeof(STUDY_LEVEL) ))
                {
                    /*
                     * Send back the study instance? 
                     */
                    if ( A_sendBack[SI] == TRUE )
                    {
                        status = MC_Set_Value_From_String ( messageID,
                                                            MC_ATT_STUDY_INSTANCE_UID,
                                                            stu_inst );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                    status, NULL );
                            return ( QR_FAILURE );
                        }
                    }

                    /*
                     * Send back the study date? 
                     */
                    if ( A_sendBack[SD] == TRUE )
                    {
                        status = MC_Set_Value_From_String ( messageID,
                                                            MC_ATT_STUDY_DATE,
                                                            stu_date );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                status, NULL );
                            return ( QR_FAILURE );
                        }
         
                    }

                    /*
                     * Send back the study time? 
                     */
                    if ( A_sendBack[ST] == TRUE )
                    {
                        status = MC_Set_Value_From_String ( messageID,
                                                            MC_ATT_STUDY_TIME,
                                                            stu_time);
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                    status, NULL );
                            return ( QR_FAILURE );
                        }
            
                    }

                    /* 
                     * Send back the study accession number 
                     */
                    if ( A_sendBack[SAN] == TRUE )
                    {
                        status = MC_Set_Value_From_String ( messageID,
                                                            MC_ATT_ACCESSION_NUMBER,
                                                            stu_anum );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                status, NULL );
                            return ( QR_FAILURE );
                        }
            
                    }
       
                    /*
                     * Send back the study id? 
                     */
                    if ( A_sendBack[SID] == TRUE )
                    {
                        status = MC_Set_Value_From_String ( messageID,
                                                            MC_ATT_STUDY_ID,
                                                            stu_id );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                status, NULL );
                            return ( QR_FAILURE );
                        }
                    }
       
                    /*
                     * Send back the study description? 
                     */
                    if ( A_sendBack[SDI] == TRUE )
                    {
                        status = MC_Set_Value_From_String ( messageID,
                                                            MC_ATT_STUDY_DESCRIPTION,
                                                            stu_desc );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            PrintErrorMessage ( S_prefix, "MC_Set_Value_From_String",
                                status, NULL );
                            return ( QR_FAILURE );
                        }
            
                    }
                } /* if level is STUDY_LEVEL */


                /*
                 * Send the responce back to the user 
                 */
                status = MC_Send_Response_Message ( A_associationID,
                                                    A_retStatus,
                                                    messageID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &messageID );
                    PrintErrorMessage ( S_prefix, "MC_Send_Response_Message",
                            status, NULL );
                    return ( QR_FAILURE );
                }
     
                /*
                 * Clear out the message for reuse 
                 */
                status = MC_Empty_Message ( messageID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &messageID );
                    PrintErrorMessage ( S_prefix, "MC_Empty_Message", status,
                            NULL );
                    return ( QR_FAILURE );
                }

                /*
                ** After sending the response message to the SCU, we poll
                ** the toolkit to see if the SCU has attempted to send us a
                ** CANCEL message.
                */
                status = MC_Read_Message( A_associationID, 0, &readMessageID,
                                          &serviceName, &command );
                if ( ( status != MC_NORMAL_COMPLETION ) &&
                     ( status != MC_TIMEOUT ) )
                {
                   MC_Free_Message( &messageID );
                   LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                             "Failed to check for a cancel message from the "
                             "SCU.\n" );
                   return( QR_FAILURE );
                }
                switch( command )
                {
                    case C_CANCEL_RQ:
                        printf( "--- Received a C_CANCEL_REQUEST\n" );
                        gotCancel = TRUE;
                        status = MC_Open_Message( &ackMsg,
                                                  "PATIENT_STUDY_ONLY_QR_FIND",
                                                  C_FIND_RSP );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            LogError( ERROR_LVL_1, "SendCFINDReply",
                                      __LINE__, status,
                                      "Could not open the reply message.\n" );
                        }

                        /*
                        ** And then, send the message to the SCU...
                        */
                        printf( "--- Sending acknowledgement of "
                                "C_CANCEL_REQUEST message...\n" );
                        status = MC_Send_Response_Message ( A_associationID,
                                                C_FIND_CANCEL_REQUEST_RECEIVED,
                                                            ackMsg );
    
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &ackMsg );
                            MC_Free_Message ( &messageID );
                            MC_Free_Message ( &readMessageID );
                            LogError( ERROR_LVL_1, "SendCFINDReply",
                                      __LINE__, status,
                                      "Failed to send the response message "
                                      "to the SCU.\n" );
                            return ( QR_FAILURE );
                        }

                        /*
                        ** Once we are successful at sending the
                        ** acknowledgement, we use this flag to make sure
                        ** that we don't send a C_FIND with a SUCCESS later on.
                        ** The above message should be the last sent when a
                        ** CANCEL happens.
                        */
                        ackSent = TRUE;

                        /*
                        ** And then, free the acknowledgement message...
                        */
                        status = MC_Free_Message( &ackMsg );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            LogError( ERROR_LVL_1, "SendCFINDReply",
                                      __LINE__, status,
                                      "Failed to free the ack message "
                                      "object.\n" );
                            return ( QR_FAILURE );
                        }

                        status = MC_Free_Message( &readMessageID );
                        if ( status != MC_NORMAL_COMPLETION )
                        {
                            MC_Free_Message ( &messageID );
                            LogError( ERROR_LVL_1, "SendCFINDReply",
                                      __LINE__, status,
                                      "Failed to free the cancel message "
                                      "object.\n" );
                            return ( QR_FAILURE );
                        }
                    default:
                        status = MC_Free_Message( &ackMsg );
                        break;
                } /* end of switch */

            } /* if */

            i++;
        } /* for */
    }

    /* 
     * Clear out the message for reuse one last time in the response 
     */
    status = MC_Empty_Message ( messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &messageID );
        PrintErrorMessage ( S_prefix, "MC_Empty_Message", status, NULL );
        return ( QR_FAILURE );
    }
   
    /* 
     * Send back an empty response message signaling the query is finished 
     */
    if ( A_retStatus == C_FIND_PENDING ||
         A_retStatus == C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT )
    {
        A_retStatus = C_FIND_SUCCESS;
    }

    if ( ackSent != TRUE )
    {
        status = MC_Send_Response_Message ( A_associationID,
                                            A_retStatus,
                                            messageID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            PrintErrorMessage ( S_prefix, "MC_Send_Response_Message",
                   status, NULL );
            return ( QR_FAILURE );
        }
    }

    /* 
     * Clean Up 
     */
    status = MC_Free_Message ( &messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Free_Message",
                            status, "Could not free message." );
        return ( QR_FAILURE );
    }

    return ( QR_SUCCESS );

} /* SendCFINDReply */



/*****************************************************************************
**
** NAME
**    SearchForKeyMatch - Performs a search for a key
**
** ARGUMENTS
**    A_field       int       Index into array that we are looking for
**    A_key         char *    Element value looking for 
**    A_matchFieldArray int *     Array where match is placed
**    A_filename    char *    Data file name
**
** DESCRIPTION
**    Search for a match for a specific key in the database.
**
** RETURNS
**    QR_SUCCESS if the routine finishes properly.
**    QR_FAILURE if the routine detects an error.
**
** SEE ALSO
**    PrintErrorMessage
**
*****************************************************************************/
static QR_STATUS SearchForKeyMatch ( int     A_field, 
                                     char*   A_key, 
                                     int*    A_matchFieldArray, 
                                     char*   A_filename )
{
    static char S_Prefix[] = "SearchForKeyMatch";
    int	     i;
    FILE*    fp;
    char     pat_id[LO_LEN], 
             pat_name[PN_LEN], 
             pat_bday[DA_LEN], 
             pat_sex[CS_LEN],
             stu_inst[UI_LEN], 
             stu_date[DA_LEN], 
             stu_time[TM_LEN], 
             stu_anum[CS_LEN],
             stu_id[CS_LEN],
             stu_desc[LO_LEN];


    if ( NULL == (fp = fopen ( A_filename, "r" ) ) )
    {
        PrintErrorMessage ( S_Prefix, "fopen", -1, "Could not open data file." );
        return ( QR_FAILURE );
    }

    for ( i = 0; i < DATABASE_ENTRIES; i++ )
    {
        fscanf ( fp, FMT_STR, pat_id, pat_name, pat_bday, pat_sex,
                            stu_inst, stu_date, stu_time, stu_anum,
                            stu_id, stu_desc );

        switch ( A_field )
        {
            case PID: if (strcmp( pat_id,   A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case PN:  if (strcmp( pat_name, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case PBD: if (strcmp( pat_bday, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case PS:  if (strcmp( pat_sex,  A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case SI:  if (strcmp( stu_inst, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case SD:  if (strcmp( stu_date, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case ST:  if (strcmp( stu_time, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case SAN: if (strcmp( stu_anum, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case SID: if (strcmp( stu_id,   A_key )) A_matchFieldArray[i] = NOTFOUND; break;
            case SDI: if (strcmp( stu_desc, A_key )) A_matchFieldArray[i] = NOTFOUND; break;
        } /* switch */
    } /* for */
   
    fclose ( fp );
    return ( QR_SUCCESS );
} /* SearchForKeyMatch */



/*****************************************************************************
**
** NAME
**    PerformDatabaseSearch - Performs a search for a C-FIND message
**
** ARGUMENTS
**    A_myConfig    AppConfig* The applications static config structure
**    A_queryLevel  char *    A string describing the query level for 
**                            A_messageID
**    A_messageID   int       The ID of a C-FIND-RQ message to query for
**    A_tag         unsigned long  Tag for which to search for a match   
**    A_matchFieldArray  int *     Array used by data base to specify matched 
**                                 records
**    A_retStatus   RESP_STATUS*   Status to return for C-FIND-RSP
**    A_sendBack    int *     Array containing a list of tags to return in
**                            C-FIND-RSP message
**
** DESCRIPTION
**    Search at the patient or study level based on a C-FIND-RQ message.
**
** RETURNS
**    QR_SUCCESS if the routine finishes properly.
**    QR_FAILURE if the routine detects an error.
**
** SEE ALSO
**    PrintErrorMessage
**    SearchForKeyMatch
**
*****************************************************************************/
static QR_STATUS PerformDatabaseSearch ( AppConfig*    A_myConfig, 
                                         char*         A_queryLevel, 
                                         int           A_messageID, 
                                         unsigned long A_tag, 
                                         int*          A_matchFieldArray,
                                         RESP_STATUS*  A_retStatus, 
                                         int*          A_sendBack)
{
    /*
     * Patient level tags that we can search on.  There are two arrays
     * here so that when we find a matching tag, we canfind the
     * the corresponding array location tag so that we can do a search
     * in our "database".  These two arrays are just used as a lookup
     * table.
     */
    unsigned long patientKeyTags[] = { MC_ATT_PATIENT_ID,
                                       MC_ATT_PATIENTS_NAME,
                                       MC_ATT_PATIENTS_SEX,
                                       MC_ATT_PATIENTS_BIRTH_DATE };
    int  patientKeyArrayLocation[] = { PID,
                                       PN,
                                       PS,
                                       PBD };
    /*
     * Study level tags that we can search on
     */
    unsigned long studyKeyTags[] = {  MC_ATT_STUDY_INSTANCE_UID,
                                      MC_ATT_STUDY_DATE,
                                      MC_ATT_STUDY_TIME,
                                      MC_ATT_ACCESSION_NUMBER,
                                      MC_ATT_STUDY_ID,
                                      MC_ATT_STUDY_DESCRIPTION };
    int  studyKeyArrayLocation[] = {  SI,
                                      SD,
                                      ST,
                                      SAN,
                                      SID,
                                      SDI };
    char           buffer[1024];
    int            i; 
    unsigned long* tagArrayPtr;
    int*           arrayArrayPtr;
    int            arraySize;
    MC_STATUS      status;
    
    /*
     * Based on the level of the C-FIND-RQ message, we search through one
     * of the two sets of arrays mentioned above.  We assign a pointer
     * to each array to a common variable so that we do not have to 
     * duplicate the code that searches.
     */ 
    if (!strncmp ( A_queryLevel, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) ))
    {
        arraySize = sizeof(patientKeyTags) / sizeof(unsigned long);
        tagArrayPtr = patientKeyTags;
        arrayArrayPtr = patientKeyArrayLocation; 
    }
    else if (!strncmp ( A_queryLevel, STUDY_LEVEL, sizeof(STUDY_LEVEL) ))
    {
        arraySize = sizeof(studyKeyTags) / sizeof(unsigned long);
        tagArrayPtr = studyKeyTags;
        arrayArrayPtr = studyKeyArrayLocation; 
    }
    else
    {
        return( QR_FAILURE ); 
    }
    
    /*
     * Search for the appropriate study or patient key in our database.
     */ 
    for (i=0;i<arraySize; i++)
    {
        /*
         * We will ignore fields in the C-FIND-RQ that we do not handle.
         * We potentially should have send a C-FIND-PENDING-NO-OPTIONAL-KEY-SUPPORT
         * return status if a field is filled in that we do not support.
         */
        if (tagArrayPtr[i] == A_tag)
        {
            /*
             * We have found a tag that we can search on.  We now need
             * to get the tag's value from the message, and then
             * call SearchForKeyMatch to see if there is a match in
             * the database for this value.  Based on the location in the
             * tag array we can look into the match key array to find the
             * appropriate field.
             */
            status = MC_Get_Value_To_String ( A_messageID,
                                              A_tag,
                                              sizeof(buffer),
                                              buffer );
            switch ( status )
            {
                case MC_NORMAL_COMPLETION:   
                    /* 
                     * Depending on what you accept for the patientID will 
                     * change the complexity of this section.  We accept a 
                     * "*" and a NULL value as meaning everything.  This could 
                     * instead give a partial match for a patient ID, we won't.       
                     */
                    if ( strtok ( buffer, "*" ) != NULL )
                    {
                        SearchForKeyMatch ( arrayArrayPtr[i], 
                                            buffer, 
                                            A_matchFieldArray, 
                                            A_myConfig->DataFileName );
                        *A_retStatus = C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT;
                    }
                case MC_NULL_VALUE:
                    A_sendBack[arrayArrayPtr[i]] = TRUE;
                    break;
                default:
                    return ( QR_FAILURE );
            }
            return( QR_SUCCESS );            
        } 
    } 
    return ( QR_FAILURE );
} /* PerformDatabaseSearch */
                                    
                                    
                                           
/*****************************************************************************
**
** NAME
**    StreamToMessage
**
** ARGUEMENTS
**    A_msgID          int     Message ID that is being streamed out
**    A_cbInformation  void *  User information used by this routine
**    A_firstCall      int     Set to true on the first call to this routine
**    A_dataLen        int *   Length of data returned by the call
**    A_dataBuffer     void ** Pointer to data returned by this call
**    A_isLast         int *   Set by this function to true when completed
**
** DESCRIPTION
**    This function is passed as a parameter to the toolkit's 
**    MC_Stream_To_Message() function call.
**
** RETURNS
**    MC_NORMAL_COMPLETION if the routine finishes properly.
**    MC_CANNOT_COMPLY if the routine detects an error.
**
** SEE ALSO
**
*****************************************************************************/

static MC_STATUS StreamToMessage   ( int A_msgID, void* A_cbInformation,
                                     int A_firstCall, int* A_dataLen,
                                     void** A_dataBuffer, int* A_isLast )
{
   static char     S_prefix[] = "StreamToMessage";
   size_t          bytes_read;
   CBinfo*         callbackInfo = (CBinfo*)A_cbInformation;

   if ( A_firstCall && A_msgID != callbackInfo->messageID )
   {
      PrintErrorMessage( S_prefix, "Wrong message ID!", -1, NULL );
      return ( MC_CANNOT_COMPLY );
   }

   bytes_read = fread ( callbackInfo->buffer, 1, sizeof(callbackInfo->buffer), callbackInfo->stream );

   if ( ferror ( callbackInfo->stream ) )
   {
      PrintErrorMessage( S_prefix, "fread", -1, "error returned by ferror" );
      return ( MC_CANNOT_COMPLY );
   }

   if ( feof ( callbackInfo->stream ) )
      *A_isLast = 1;
   else
      *A_isLast = 0;

   *A_dataBuffer = callbackInfo->buffer;
   *A_dataLen = (int)bytes_read;

   return ( MC_NORMAL_COMPLETION );

} /* StreamToMessage */



/*****************************************************************************
**
** NAME
**    GetOptions - Parse the command line arguements and set the config parms
**
** ARGUMENTS
**    A_argc        int            Number of parameters to the program
**    A_argv        char **        The list of arguements given to the program
**    A_myConfig    AppConfig *    Structure of Configuration Parameters  
**
** DESCRIPTION
**    This routine goes throught the command line options given on startup
**    and takes the correct options and used them to configure the way the 
**    program will operate.
**
** RETURNS
**    Returns any valid options given on the command line, but no status.
**
** SEE ALSO
**
*****************************************************************************/
static void GetOptions ( int A_argc, char **A_argv, AppConfig *A_myConfig )
{
    int          ai, ci;
    int          error = FALSE;

    for ( ai = 1; ai < A_argc && strcmp ( A_argv[ai], "--" ); ai ++ )
    {
        if ( A_argv[ai][0] == '-' )
        {
            for ( ci = 1; ci < (int)strlen ( A_argv[ai] ); ci++ )
            {
                switch ( A_argv[ai][ci] )
                {
                    case 'a':        /* Application Title */
                        if ( A_argv[ai][ci+1] != '\0' )
                        {
                            error = TRUE;
                        }
                        else if ( (ai + 1) == A_argc )
                        {
                            error = TRUE;
                        }
                        else
                        {
                            strcpy ( A_myConfig->ApplicationTitle, &(A_argv[ai+1][0]) );
                            ai++;
                            ci = strlen ( A_argv[ai] );
                        }
                        break;

                    case 'd':        /* Data file */
                        if ( A_argv[ai][ci+1] != '\0' )
                        {
                            error = TRUE;
                        }
                        else if ( (ai + 1) == A_argc )
                        {
                            error = TRUE;
                        }
                        else
                        {
                            strcpy ( A_myConfig->DataFileName, &(A_argv[ai+1][0]) );
                            ai++;
                            ci = strlen ( A_argv[ai] );
                        }
                        break;

                    case 'h':        /* Help */
                        printf ( "\n" );
                        printf ( "Query/Retrieve Service Class Provider\n" );
                        printf ( "\n" );
                        printf ( "Usage: %s [options]\n", A_argv[0] );
                        printf ( "\n" );
                        printf ( "Options:\n" );
                        printf ( "   -a atitle        Application Title\n" );
                        printf ( "   -d data          Data file name\n" );
                        printf ( "   -h               Print this help page\n" );
                        printf ( "   -o timeout       Time out value\n" );
                        printf ( "   -s slist         Service List\n" );
                        printf ( "   -t type          Image Type\n" );
                        printf ( "\n" );
                        exit ( EXIT_SUCCESS );
                        break;

                    case 'o':        /* time out */
                        if ( A_argv[ai][ci+1] != '\0' )
                        {
                            error = TRUE;
                        }
                        else if ( (ai + 1) == A_argc )
                        {
                            error = TRUE;
                        }
                        else
                        {
                            strcpy ( A_myConfig->TimeOut, &(A_argv[ai+1][0]) );
                            ai++;
                            ci = strlen ( A_argv[ai] );
                        }
                        break;

                    case 's':        /* Service List */
                        if ( A_argv[ai][ci+1] != '\0' )
                        {
                            error = TRUE;
                        }
                        else if ( (ai + 1) == A_argc )
                        {
                            error = TRUE;
                        }
                        else
                        {
                            strcpy ( A_myConfig->ServiceList, &(A_argv[ai+1][0]) );
                            ai++;
                            ci = strlen ( A_argv[ai] );
                        }
                        break;

                    case 't':        /* Image Type */
                        if ( A_argv[ai][ci+1] != '\0' )
                        {
                            error = TRUE;
                        }
                        else if ( (ai + 1) == A_argc )
                        {
                            error = TRUE;
                        }
                        else
                        {
                            strcpy ( A_myConfig->ImageType, &(A_argv[ai+1][0]) );
                            ai++;
                            ci = strlen ( A_argv[ai] );
                        }
                        break;

                    default:
                        break;

                } /* switch argv */

                if ( error == TRUE )
                {
                    printf ( "Usage: %s [options]\n", A_argv[0] );
                    printf ( "       %s -h\n", A_argv[0] );
                    return;
                }

            } /* for ci */

        } /* if argv */

    } /* for ai */

} /* GetOptions */



/*****************************************************************************
**
** NAME
**    SetProgramDefaults - Sets global application default parameters
**
** ARGUEMENTS
**    A_myConfig    AppConfig *   Structure of configuration parameters
**
** DESCRIPTION
**    This routine gives values to the configuration parameters to be 
**    used by the for other setup requirements.
**
** RETURNS
**    Returns with the parameters set, but no other status.
**
** SEE ALSO
**
*****************************************************************************/

static void SetProgramDefaults ( AppConfig *A_myConfig )
{

   strcpy ( A_myConfig->TimeOut, "3000" );
   strcpy ( A_myConfig->ApplicationTitle, "MERGE_QR_SCP" );
   strcpy ( A_myConfig->RemoteApplicationTitle, "" );
   strcpy ( A_myConfig->ServiceList, "Query_SCP_Service_List" );
#if defined(_MACINTOSH)
   strcpy ( A_myConfig->ImageType, "ct.img" );
   strcpy ( A_myConfig->DataFileName, "test.dat" );
#else
   strcpy ( A_myConfig->ImageType, "./ct.img" );
   strcpy ( A_myConfig->DataFileName, "./test.dat" );
#endif

   return;

} /* SetProgramDefaults */

