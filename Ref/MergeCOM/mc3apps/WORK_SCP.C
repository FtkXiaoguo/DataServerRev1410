/*************************************************************************
 *
 *       System: MergeCOM-3 Integrator's Tool Kit
 *
 *    $Workfile: work_scp.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: 
 *               Modality Worklist Service Class Provider
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

#if !defined(_MSDOS)         && !defined(_RMX3)   && !defined(INTEL_WCC) && \
    !defined(_ALPHA_OPENVMS) && !defined(OS9_68K) && !defined(UNIX)  && \
    !defined(_WIN32)         && !defined(OS9_PPC) && !defined(VXWORKS) && \
    !defined(__OS2__)
#define UNIX            1
#endif

/* Defined for MACINTOSH */
#if defined(_MACINTOSH) 
#include <Types.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <time.h>
#include <console.h>
#include <SIOUX.h>
#undef UNIX
#endif

#if defined(_WIN32)
#include <conio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>


#if defined(UNIX)
#ifdef STRINGS
#include <strings.h>
#endif
#ifdef LYNX_OS
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/wait.h>
#include <fcntl.h>
#endif

#if defined(INTEL_WCC)
#include <limits.h>
#endif

#include "mergecom.h"
#include "mcstatus.h"
#include "diction.h"
#include "qr.h"
#include "workdata.h"

/* Application Default Values */
#if defined(INTEL_WCC)
extern volatile int errno;
#endif


/* Global Variables */
char G_app_title[ AE_LEN ]="MERGE_WORK_SCP";
char G_data_file[ PN_LEN ]="work.dat";
int G_timeout = 3000;
int G_dummy_patient = 1;
int G_logit = FALSE;
int G_logtree = FALSE;
extern LIST G_patientList;

/* Function prototypes */
static void          GetOptions( int argc, char **argv );
static int    HandleAssociation( int  A_associationID );
static MC_STATUS ProcessCFINDRQ( int A_associationID, int A_messageID,
                                 RESP_STATUS *A_retStatus );
static void    DisposeAttribute( int A_msgItemId, unsigned long A_tag,
                                 MC_VR A_valueRep, int A_numValues,
                                 LIST *A_attributeList );
void             WorklistSearch( char *A_stationAEtitle,
                                 char *A_procStepStartDate,
                                 char *A_procStepStartTime,
                                 char *A_description,
                                 char *A_modality,
                                 char *A_scheduledPerfPhysician,
                                 char *A_patientName,
                                 char *A_patientID,
                                 LIST *A_workList );
static int       SendCFINDReply( int A_associationID,
                                 LIST *A_workList,
                                 LIST *A_attributeList );
static int       ProcessNCREATERQ( int A_associationID, int A_messageID,
                                   RESP_STATUS *A_retStatus );
static int           SetNCREATERQ( int A_messageID, int A_stepAttributeSeq,
                                   int A_rspMsg,    int A_rspStepAttributeSeq,
                                   PATIENT_REC       *patientRec,
                                   REQ_PROC_REC      *reqProcRec,
                                   SCH_PROC_STEP_REC *schProcStepRec );
static int          ProcessNSETRQ( int A_associationID, int A_messageID );
NODE      *SelectMPPSFromDatabase( PATIENT_REC        *A_patientRec,
                                   REQ_PROC_REC       *A_reqProcRec,
                                   PERF_PROC_STEP_REC *A_perfProcStepRec,
                                   PERF_SERIES_REC    *A_perfSeriesRec,
                                   char               *A_reqSOPinstance );
static int     SetMPPSValues( int                A_messageID,
                              int                A_rspMsgID,
                              int                A_rspPerfSeriesSeqID,
                              int                A_rspImageSeqID,
                              PERF_PROC_STEP_REC *A_perfProcStepRec );
NODE *SelectSeriesInstanceFromDatabase( PERF_PROC_STEP_REC *A_perfProcStepRec,
                                        char            *A_seriesInstanceUID );
static int   CheckForImageSQ( PERF_PROC_STEP_REC *A_perfProcStepRec,
                              int A_perfSeriesSeqID,
                              int A_rspImageSeqID );
static void   DumpAttributes( PERF_PROC_STEP_REC *A_perfProcStepRec,
                              int                A_rspMsgID,
                              int                A_rspSeriesID,
                              int                A_rspImageID );
NODE *SelectImageRecFromDatabase( PERF_PROC_STEP_REC *A_perfProcStepRec,
                                  char               *A_sopInstanceUID );
static RESP_STATUS CheckIfComplete( int                A_associationID,
                                    int                A_messageID,
                                    PERF_PROC_STEP_REC *A_modPerfProcRec,
                                    char            *performedProcStepStatus );
static int RetrieveAndSetValue( int            A_sourceID,
                                int            A_destID,
                                unsigned long  A_tag,
                                RETURN_TYPE    A_sourceType,
                                RETURN_TYPE    A_destType,
                                char           *A_databaseStr,
                                size_t         A_databaseStrSize,
                                char           *A_functionName,
                                int            A_sendBack );
static int   CompareDateStrings( char *A_baseDate, char *A_tokenDate );
static int   CompareTimeStrings( char *A_baseTime, char *A_tokenTime );
static void           SplitDate( char *A_date, char *A_one, char *A_two );
/*
** The next function is a Macintosh-only function.
*/
#if defined(_MACINTOSH) && defined(__MWERKS__)
static int  HandleMacQuitEvents();
#endif



/*****************************************************************************
**
** NAME
**    main - Modality Worklist Service Class Provider
**
** SYNOPSIS
**    work_scp [options]
**
**    This program is somewhat based on the qr_scp program, also available
**    in this distribution.  This program receives a C-FIND message, and
**    based on this message, it searches a simple datafile for matches, and
**    then sends this information back to the requester in the form of a
**    C-FIND reply message.
**
*****************************************************************************/

#ifdef VXWORKS
int  workscp(int argc, char** argv);
int workscp(int argc, char** argv)
#else
int  main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    MC_STATUS  status;                  /* The status of a call to toolkit   */
    MC_STATUS  associationOpen = MC_NORMAL_COMPLETION;
    int        applicationID;           /* Application ID                    */
    int        associationID;           /* The association ID                */
    int        calledAppID;             /* Ths application that's calling us */
    char       applicationTitle[ AE_LEN ];  /* The title of the application  */
/*    RESP_STATUS retStatus = C_FIND_PENDING;  Status retd from C_FIND       */
    char       strTime[ 9 ];            /* Today's date in YYYYMMDD format   */
    char       strHMS[ 7 ];             /* Today's hours, minutes, and sec   */
/*    int        res = FALSE;              The result of the date comparison */
    int        didItWork;               /* A flag that is used to test the   */
                                        /* result of reading the "dat" file  */
#if defined(_MACINTOSH) && defined(__MWERKS__)
    int        done = FALSE;
#endif 
#if defined(_MSDOS)  || defined(INTEL_WCC) || defined(_RMX3) || \
    defined(OS9_68K) || defined(OS9_PPC) || defined(MACH_NEXT) || \
    defined(_PHAR_LAP) || defined(VXWORKS) || defined(_WIN32)
    int         quit;
#endif

    /*
    ** The Macintosh needs to use the SIOUX window for input and output.
    ** The following code sets up the SIOUX options, and asks the user
    ** for the command line arguments.
    */
#if defined(_MACINTOSH) && defined(__MWERKS__)
    SIOUXSettings.initializeTB = true;
    SIOUXSettings.standalone = true;
    SIOUXSettings.setupmenus = true;
    SIOUXSettings.autocloseonquit = false;
    SIOUXSettings.asktosaveonclose = true;
    SIOUXSettings.showstatusline = true;
    argc = ccommand(&argv);
#endif 

    strcpy (applicationTitle, "MERGE_WORK_SCP");
    GetOptions( argc, argv );
    GetCurrentDate( strTime, strHMS );

    /*
    ** First, we want to tell the list management functions the size of each
    ** of the patient nodes that are placed onto the list.  The patientList
    ** is global, and defined in "workdata.c".
    */
    if ( LLCreate( &G_patientList, sizeof( PATIENT_REC ) ) != SUCCESS )
    {
        /*
        ** If we've failed to create the list, we must exit.
        */
        printf( "Failed to create the linked list structure for the modality\n"
                "worklist and performed procedure step database list.\n" );
        exit(1);
    }

    /*
    ** We must now initialize our worklist and performed procedure step
    ** database by reading information from our "dat" file.
    */
    didItWork = ReadDat( G_data_file );

    /*
    ** Once we've initialized our "database", we can continue with the normal
    ** setup of this application with respect to the toolkit, etc.
    */

#if defined (_MSDOS) || defined(_RMX3) || defined(INTEL_WCC) || \
    defined (OS9_68K) || defined(_PHAR_LAP) || defined(VXWORKS) || \
    defined (_WIN32)
    printf("\tPress 'Q' or Esc to cancel server and\n");
    printf("\tserver will stop when session ends.\n");
    quit = 0;
#endif

    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */ 
  
    status = MC_Library_Initialization( NULL, NULL, NULL ); 
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "main", __LINE__, status,
                  "Unable to initialize the library.\n" );
        exit ( EXIT_FAILURE );
    }
  
    /*
    ** We register this application as the work service class provider.
    */
    status = MC_Register_Application ( &applicationID,
                                       G_app_title );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "main", __LINE__, status,
                  "Unable to register the application with the"
                  " library.\n" );
        exit ( EXIT_FAILURE );
    }

    /*
    ** This is the outer infinite loop.  This loop ensures that this
    ** server will be looking for an association after one has been
    ** dropped.
    */
    
    /*
    ** The Mac uses event handling in order to allow the user to use the
    ** application menu to quit this application.  There are some special
    ** needs that arise due to this fact.  The Mac version is "quittable",
    ** while the Unix based versions don't have a menu to allow them to quit.
    ** In Unix, a simple CTRL-C will suffice...
    */
#if defined(_MACINTOSH) && defined(__MWERKS__)
    while( done == FALSE )
#else
    while( TRUE )
#endif    
    {      
        /*
        ** OK.  Now we wait for an association from a client.
        */
        printf ( "Waiting for an association.\n" );
#if defined(_MACINTOSH) && defined(__MWERKS__)
        printf ( "Press 'Q', 'q', or 'CMD-.' to stop this server.\n" );
#endif
        status = MC_TIMEOUT;
        while( status == MC_TIMEOUT )
        {
#if defined(INTEL_WCC) || defined(INTEL_BCC)
            if (kbhit())
            {
                switch (getch())
                {
                    case    0:      /* Ignore function keys */
                       getch();
                       break;
                    
                    case    'Q':    /* Quit */
                    case    'q':
                    case    27:
                       quit = 1;
                }
            }
      
            if (quit)
                break;
#elif defined(_MSDOS) || defined(_PHAR_LAP) || defined(_WIN32)
            if (_kbhit())
            {
                switch (_getch())
                {
                    case    0:      /* Ignore function keys */
                       _getch();
                       break;
                
                    case    'Q':    /* Quit */
                    case    'q':
                    case    27:
                       quit = 1;
                }
            }
      
            if (quit)
                break;
#elif defined(_RMX3)
            switch (_getch())
            {
               case    0:      /* Ignore function keys */
                  _getch();
                  break;
        
               case    'Q':    /* Quit */
               case    'q':
               case    27:
                  quit = 1;
            }
            if (quit)
                break;
#endif                                        
            status = MC_Wait_For_Association ( "Worklist_Service_List",
                                               1,
                                               &calledAppID,
                                               &associationID );
#if defined(_MACINTOSH) && defined(__MWERKS__)
            /*
            ** The following Macintosh-only function handles the user
            ** attempting to quit this application.
            */                                 
            done = HandleMacQuitEvents();
            if ( done == TRUE )
            {
                status = MC_NORMAL_COMPLETION;
                break;
            }
#endif                                               
        } /* Wait for Association WHILE loop */
        
#if defined (_MSDOS) || defined(_RMX3) || defined(INTEL_WCC) || \
    defined (OS9_68K) || defined(_PHAR_LAP) || defined(VXWORKS) || \
    defined (_WIN32)
        if ( quit == 1 )
        {
            break;
        }
#endif
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Release_Application ( &applicationID );
            LogError( ERROR_LVL_1, "main", __LINE__, status,
                      "Failed to wait for an association.\n" );
            break;
        }
#if defined(_MACINTOSH) && defined(__MWERKS__)
        if ( ( status == MC_NORMAL_COMPLETION ) && ( done == TRUE ) )
        {
            MC_Release_Application ( &applicationID );
            break;
 
        }
#endif

        
        associationOpen = MC_NORMAL_COMPLETION;
        if ( calledAppID != applicationID )
        {
            MC_Release_Application( &applicationID );
            LogError( ERROR_LVL_1, "main", __LINE__, status,
                      "Unexpected application identifier received from "
                      "MC_Wait_For_Association.\n" );
            break;
        }
        
        /*
        ** At this point, we've received an association.  Tell the user
        ** from whom...
        */
        printf ( "%s: Association received\n", applicationTitle );
      
        /*
        ** Since we've received an association, we now will attempt to
        ** accept it and process it.
        */
        status = MC_Accept_Association( associationID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "main", __LINE__, status,
                      "Failed to accept the association.\n" );
            break;
        }

        if ( HandleAssociation( associationID ) != SUCCESS )
        {
            MC_Abort_Association( &associationID );
        }
        else
        {
            MC_Close_Association( &associationID );
        }

        /* --------------------------------------- */
        /* We're finished: release the application */
        /* --------------------------------------- */

    } /* End of association while loop */

    status = MC_Release_Application ( &applicationID );

    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");

    /*
    ** And then, free all of the memory associated with the linked list
    ** database.
    */
    FreeTree();
    printf ( "... WORK_SCP FINISHED.\n\n" );
   
    exit ( EXIT_SUCCESS );
   
}


/*****************************************************************************
**
** NAME
**    HandleAssociation 
**
** SYNOPSIS
**    static int HandleAssociation( int  A_associationID );
**
** ARGUMENTS
**    int A_associationID - The ID of a valid association.
**
** DESCRIPTION
**    This function obtains a message from an open association, and
**    then sends the message off to be parsed based on what type of
**    message it is.
**
** RETURNS
**    SUCCESS or FAILURE
**
** SEE ALSO
**
*****************************************************************************/
static int
HandleAssociation(
                     int  A_associationID
                 )
{
    int        retValue;
    MC_STATUS  status;
    int        associationOpen = MC_NORMAL_COMPLETION;
    int        readMessageID = -1;
    MC_COMMAND command;
    char       *serviceName;
    RESP_STATUS retStatus = C_FIND_PENDING;

    /*
    ** While we have an open association, we will wait for a message,
    ** hopefully from someone requesting a C_FIND.
    ** This is the inner, semi-infinite loop.
    */
    while( associationOpen != MC_ASSOCIATION_CLOSED )
    {
        /*
        ** If we've successfully accepted the association, we can attempt
        ** to read a message from the client.  After all, we are the 
        ** server and a client should be requesting something from us.
        */
        status = MC_Read_Message( A_associationID,
                                  G_timeout,
                                  &readMessageID,
                                  &serviceName,
                                  &command );

        /*
        ** After attempting the read from the client, we hope that the
        ** read was successful.  If not, we probably should be prepared for
        ** it NOT being successful.
        */
        switch ( status )
        {
            case MC_NORMAL_COMPLETION:
                break;
                
            /*
            ** All of the following cases happen when the SCU had
            ** done something wrong.  In these cases, no message ID
            ** was returned, and no further calls can be made to this
            ** association.  We don't want to exit if this happens, we
            ** will keep on processing, waiting for other associations.
            */
            case MC_ASSOCIATION_CLOSED:
            case MC_ASSOCIATION_ABORTED:
            case MC_NETWORK_SHUT_DOWN:
            case MC_INACTIVITY_TIMEOUT:
            case MC_INVALID_MESSAGE_RECEIVED:
                associationOpen = MC_ASSOCIATION_CLOSED;
                continue;
                
            /*
            ** There is a possibility of MC_CONFIG_INFO_ERROR, but
            ** if this is the case, we probably want to exit.
            */
            default:
                MC_Free_Message ( &readMessageID );
                LogError( ERROR_LVL_1, "main", __LINE__, status,
                          "Unable to read status of message.\n" );
                return( FAILURE );
        }
    
        /*
        ** To get to here, we would have had a successful read.  Now, let's
        ** look at specifically what we were told to do.
        */
        switch( command )
        {
            case C_FIND_RQ:
                printf ( "--- Received a C_FIND_RQ request.\n" );

                /*
                ** Since we've received a C_FIND, we must process it.
                ** This involves reading the message, performing a query
                ** of our database, and then sending a response to the
                ** client.
                */
                status = ProcessCFINDRQ( A_associationID, readMessageID,
                                         &retStatus );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &readMessageID );
                    LogError( ERROR_LVL_1, "main", __LINE__,
                              MC_NORMAL_COMPLETION,
                              "Error processing the C-FIND request "
                              "message from the SCU.\n" );
                    associationOpen = MC_ASSOCIATION_CLOSED;
                    continue;
                }
                printf ( "--- Done processing C_FIND_RQ\n" );
                break;
            case C_CANCEL_RQ:
                /*
                ** Sometimes, the client sends back a C_CANCEL_RQ
                ** and we receive it AFTER we've sent all of the
                ** matches to it already.
                **
                ** In this case, we do nothing.
                */
                printf ( "--- Received a C_CANCEL_RQ message from"
                         " client AFTER all responses were sent --\n" );
                printf ( "--- Ignoring...\n" );
                break;
            case N_CREATE_RQ:
                /*
                ** Once we receive an N_CREATE_RQ for a MPPS, we must
                ** remove the data from the worklist (since it's complete),
                ** and then create the MPPS instance.
                */
                printf( "--- received an N_CREATE_RQ from client --\n" );

                retValue = ProcessNCREATERQ( A_associationID, readMessageID,
                                           &retStatus );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &readMessageID );
                    LogError( ERROR_LVL_1, "main", __LINE__,
                              MC_NORMAL_COMPLETION,
                              "Error processing the N-CREATE request "
                              "message from the SCU.\n" );
                    associationOpen = MC_ASSOCIATION_CLOSED;
                    continue;
                }
                if ( G_logtree )
                    DumpTree();
                printf ( "--- Done processing N-CREATE request\n" );
                break;
            case N_SET_RQ:
                printf( "--- received an N_SET_RQ from client --\n" );
                retValue = ProcessNSETRQ( A_associationID, readMessageID );
                if ( G_logtree )
                    DumpTree();
                break;
            case N_GET_RQ:
                printf( "--- received an N_GET_RQ from client --\n" );
                break;
            default:
                printf ( "--- Received an unknown command from"
                         " client --\n" );
                printf ( " --- Command:  %d\n", command );
                break;
        } /* end of switch */

		/* If read was successful, free the message now before reading again */
		if(readMessageID > 0)
		{
			MC_Free_Message ( &readMessageID );
			readMessageID = -1;
		}


    } /* End of inner, semi-infinite infinite loop */

    return( SUCCESS );
}



/*****************************************************************************
**
** NAME
**    ProcessCFINDRQ - Process a C-FIND Request
**
** SYNOPSIS
**    static MC_STATUS ProcessCFINDRQ ( int         A_associationID,
**                                      int         A_messageID, 
**                                      RESP_STATUS *A_retStatus )
**
** ARGUMENTS
**    A_associationID     int              Association ID 
**    A_messageID         int              Message ID
**    A_retStatus         RESP_STATUS *    Return status from operation
**
** DESCRIPTION
**    This routine processes a C-FIND request.  It looks at the search fields
**    and calls the search function to find them in the data file.  The ID
**    arrays are used to determine which which items are found in each
**    field.  After all searches have been performed, this routine "ands"
**    all of the ID arrays into one final ID array.  This array will tell if
**    there has been a complete match given the requirements of the user.
**
** RETURNS
**    MC_NORMAL_COMPLETION if the routine finishes properly.
**    other MC_ return value if the routine detects an error.
**
** SEE ALSO
**    MC_Get_Value_To_String
**    Perform_Work_Search
**
*****************************************************************************/

static MC_STATUS
ProcessCFINDRQ ( int         A_associationID,
                 int         A_messageID,
                 RESP_STATUS *A_retStatus
               )
{
    MC_STATUS    status;
    int          itemID;
    LIST         attributeList; /* A list of attributes that's in the request */
    ATTRIBUTE_REC attributeRec; /* A structure containing info about an attri */
    LIST         workList;      /* The list of matching worklist items        */

    unsigned long tag;
    MC_VR         valueRep;
    int           numValues;
    char procStepStartDate[ DA_LEN*2 ] = "";
    char procStepStartTime[ TM_LEN*2 ] = "";
    char modality[ CS_LEN ] = "";
    char scheduledPerfPhysician[ PN_LEN ] = "";
    char description[ PN_LEN ] = "";
    char patientName[ PN_LEN ] = "";
    char patientID[ LO_LEN ] = "";
    char stationAEtitle[ AE_LEN ] = "";

    /*
    ** Create a linked list of attribute records.  This list will eventally
    ** contain a list of attributes that were sent to us via the C_FIND_RQ.
    ** We also create a list that will hold the actual work list.
    */
    LLCreate( &attributeList, sizeof( ATTRIBUTE_REC ) );
    LLCreate( &workList, sizeof( WORKLIST_REC ) );

    /*
    ** In order to query the "database", we first need to know what values
    ** were sent to us, validate that they sent all the required keys, and
    ** then perform a search based on the required keys.
    ** Here, we obtain the first tag from the request message.
    */
    status = MC_Get_First_Attribute( A_messageID, &tag,
                                     &valueRep, &numValues );
    if( status != MC_NORMAL_COMPLETION )
    {
        return( status );
    }

    /*
    ** We now need to decide what to do with this attribute.
    */
    DisposeAttribute( A_messageID, tag, valueRep, numValues, &attributeList );

    /*
    ** Since a request message probably never contains a single attribute, we
    ** now loop through all of the other attributes that may be in this
    ** message
    */
    while( MC_Get_Next_Attribute( A_messageID, &tag, &valueRep, &numValues ) !=
           MC_NO_MORE_ATTRIBUTES )
    {
        /*
        ** If the tag is a sequence, then we need to parse all of the pieces
        ** of that sequence.  Since the worklist contains a single query, we
        ** only list one here.  This can be extended, should you want to parse
        ** any other sequences.
        */
        if ( tag == MC_ATT_SCHEDULED_PROCEDURE_STEP_SEQUENCE )
        {
            /*
             * First save info on the sequence itself
             */
            DisposeAttribute( A_messageID, tag, valueRep,
                              numValues, &attributeList );
            status = MC_Get_Value_To_Int( A_messageID,
                                          tag,
                                          &itemID );
            if ( status == MC_NORMAL_COMPLETION )
            {
                status = MC_Get_First_Attribute( itemID, &tag,
                                                 &valueRep, &numValues );
                if( status != MC_NORMAL_COMPLETION )
                {
                    return( status );
                }

                /*
                ** We now need to decide what to do with this attribute.
                */
                DisposeAttribute( itemID, tag, valueRep, numValues, 
                                  &attributeList );
                
                while( MC_Get_Next_Attribute( itemID, 
                                              &tag, 
                                              &valueRep, 
                                              &numValues ) !=
                       MC_NO_MORE_ATTRIBUTES )
                {
                    DisposeAttribute( itemID, tag, valueRep,
                                      numValues, &attributeList );
                }
            }
        }
        else
        {
            /*
            ** If the tag isn't a sequence, we just "dispose" of it in the
            ** ordinary way:  we save its info in the list of sent sequences.
            */
            DisposeAttribute( A_messageID, tag, valueRep,
                              numValues, &attributeList );
        }
                          
                         
    } /* end of 'while getting more attributes' loop */

    /*
    ** OK.  Now, attributeList contains a list of all "important" tags that
    ** were sent to us in the request message.  All group zero, and all type
    ** three attributes have been filtered out, and aren't part of
    ** attributeList - since we aren't required to return them.
    ** Now we build a query based on values important to us.
    */
    LLRewind( &attributeList );
    while( LLPopNode( &attributeList, &attributeRec ) != NULL )
    {
        /*
        ** We need to extract our query values from the list of queried
        ** attributes.  We do this by examining our query list and copying
        ** the obtained value from the node of the list.
        */
        switch( attributeRec.tag )
        {
            case MC_ATT_SCHEDULED_STATION_AE_TITLE:
                strcpy( stationAEtitle, attributeRec.value );
                break;
            case MC_ATT_SCHEDULED_PROCEDURE_STEP_START_DATE:
                strcpy( procStepStartDate, attributeRec.value );
                break;
            case MC_ATT_SCHEDULED_PROCEDURE_STEP_START_TIME:
                strcpy( procStepStartTime, attributeRec.value );
                break;
            case MC_ATT_SCHEDULED_PROCEDURE_STEP_DESCRIPTION:
                strcpy( description, attributeRec.value );
                break;
            case MC_ATT_MODALITY:
                strcpy( modality, attributeRec.value );
                break;
            case MC_ATT_SCHEDULED_PERFORMING_PHYSICIANS_NAME:
                strcpy( scheduledPerfPhysician, attributeRec.value );
                break;
            case MC_ATT_PATIENTS_NAME:
                strcpy( patientName, attributeRec.value );
                break;
            case MC_ATT_PATIENT_ID:
                strcpy( patientID, attributeRec.value );
                break;
        } /* end of switch */

    }

    WorklistSearch( stationAEtitle, procStepStartDate,
                    procStepStartTime, description, modality,
                    scheduledPerfPhysician, patientName, patientID,
                    &workList );
    /*
    ** The query will return a list of matching worklist entries.  We must
    ** build a response message based on what was requested of us, and what
    ** values are in the worklist query.
    **
    ** SendCFINDReply will send back only information that
    ** was requested of us, OR null-send-back information that was
    ** requested, but we can't provide.
    */
    SendCFINDReply( A_associationID, &workList, &attributeList );

    /*
    ** Since we're done with the workList and the attributeList, we can
    ** destroy them.
    */
    LLDestroy( &attributeList );
    LLDestroy( &workList );

    return ( MC_NORMAL_COMPLETION );
}



/*****************************************************************************
**
** NAME
**    DisposeAttribute -   Decides if an attribute is needed for a modality
**                         worklist search, and then places the attribute
**                         onto the list that is passed into this function.
**
** SYNOPSIS
**    static void DisposeAttribute( 
**                                    int           A_msgItemId,
**                                    unsigned long A_tag,
**                                    MC_VR         A_valueRep,
**                                    int           A_numValues,
**                                    LIST          *A_attributeList
**                                )
**
** ARGUMENTS
**    int           A_msgItemID      - a valid message or item id
**    insigned long A_tag            - a DICOM tag
**    MC_VR         A_valueRec       - the VR of a tag
**    int           A_numValues      - the number of values that an attribute
**                                     contains
**    LIST          *A_attributeList - a pointer to a list of attributes
**
** DESCRIPTION
**    Given a message or item ID, a tag, the tag's value representation,
**    and number of possible values that the tag contains, this function
**    decides if the attribute can be placed into the attribute list that
**    is passed into this function.  The list of attributes will then
**    contain all of the attributes that need to be responded to in an
**    C_STORE response for the worklist.
**
** RETURNS
**    nothing
**
*****************************************************************************/

static void
DisposeAttribute( 
                  int           A_msgItemId,
                  unsigned long A_tag,
                  MC_VR         A_valueRep,
                  int           A_numValues,
                  LIST          *A_attributeList
                )
{
    char tagDescription[60];
    int  itemID;
    char value[ PN_LEN ]; /* Value actually can hold any DICOM value.        */
                          /* We choose PN_LEN 'cause it's the biggest string */
    char stringItemID[ PN_LEN ];/* A string containing a number              */
    ATTRIBUTE_REC attributeRec; /* A structure containing info about an attri */
    MC_STATUS     status;       /* The status of a call to toolkit           */

    /*
    ** Decide how to handle each attribute, based on its tag and its VR.
    */
    switch ( A_tag )
    {
        /*
        ** This is the main sequence that we have to deal with.  In this case,
        ** we must parse the actual sequence, itself.
        */
        case MC_ATT_SCHEDULED_PROCEDURE_STEP_SEQUENCE:
            attributeRec.nullFlag = FALSE;
            status = MC_Get_Value_To_Int( A_msgItemId,
                                          A_tag,
                                          &itemID );
            if ( status == MC_NULL_VALUE )
            {
                attributeRec.nullFlag = TRUE;
            }

            /*
            ** Now, put the sequence's identifier into our returned attribute
            ** list, along with a representation of its value.
            */
            attributeRec.msgORitemID = A_msgItemId;
            attributeRec.tag = A_tag;
            attributeRec.valueRep = A_valueRep;
            attributeRec.numValues = A_numValues;
            sprintf( stringItemID, "%d", itemID );
            strcpy( attributeRec.value, stringItemID );
            LLInsert( A_attributeList, &attributeRec );
            break;



        /*
        ** These sequences aren't dealt with, because this is a sample app,
        ** and these are either conditional sequences, or type two.  If these
        ** sequences are in the request, we just NULL out the response
        ** message, for all components of these sequences.
        */
        case MC_ATT_REQUESTED_PROCEDURE_CODE_SEQUENCE:
        case MC_ATT_REFERENCED_STUDY_SEQUENCE:
        case MC_ATT_REFERENCED_PATIENT_SEQUENCE:
        case MC_ATT_SCHEDULED_ACTION_ITEM_CODE_SEQUENCE:
            attributeRec.nullFlag = FALSE;
            status = MC_Get_Value_To_String( A_msgItemId,
                                             A_tag,
                                             sizeof( char ) * PN_LEN, value );
            if ( status == MC_NULL_VALUE )
                attributeRec.nullFlag = TRUE;
            else if ( status != MC_NORMAL_COMPLETION )
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_2, "DisposeAttribute", __LINE__, status,
                          "Request message, cannot read sequence tag: %s\n"
                          "C-FIND request may fail...\n",
                          tagDescription ); 
            }
            attributeRec.msgORitemID = A_msgItemId;
            attributeRec.tag = A_tag;
            attributeRec.valueRep = A_valueRep;
            attributeRec.numValues = A_numValues;
            strcpy( attributeRec.value, value );
            LLInsert( A_attributeList, &attributeRec );

            break;


        /*
        ** But patient name and ID are required request attributes.  Therefore,
        ** it had better not be not sent.
        */
        case MC_ATT_PATIENTS_NAME:
        case MC_ATT_PATIENT_ID: /* patient ID */
            attributeRec.nullFlag = FALSE;
            status = MC_Get_Value_To_String( A_msgItemId,
                                             A_tag,
                                             sizeof( char ) * PN_LEN, value );
            if ( status == MC_NULL_VALUE )
                attributeRec.nullFlag = TRUE;
            else if ( status != MC_NORMAL_COMPLETION )
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_2, "DisposeAttribute", __LINE__, status,
                          "Request message, cannot read tag: %s\n"
                          "C-FIND request may fail...\n", 
                          tagDescription ); 
            }
            attributeRec.msgORitemID = A_msgItemId;
            attributeRec.tag = A_tag;
            attributeRec.valueRep = A_valueRep;
            attributeRec.numValues = A_numValues;
            strcpy( attributeRec.value, value );
            LLInsert( A_attributeList, &attributeRec );
            break;

        /*
        ** All of these are type 2 and while they can't be queried upon, we
        ** still must handle them in the return.
        */
        case MC_ATT_REQUESTED_PROCEDURE_PRIORITY:
        case MC_ATT_PATIENT_TRANSPORT_ARRANGEMENTS:
        case MC_ATT_ACCESSION_NUMBER:
        case MC_ATT_REQUESTING_PHYSICIAN:
        case MC_ATT_REFERRING_PHYSICIANS_NAME:
        case MC_ATT_ADMISSION_ID:
        case MC_ATT_CURRENT_PATIENT_LOCATION:
        case MC_ATT_PATIENTS_BIRTH_DATE:
        case MC_ATT_PATIENTS_SEX:
        case MC_ATT_PATIENTS_WEIGHT:
        case MC_ATT_CONFIDENTIALITY_CONSTRAINT_ON_PATIENT_DATA_DESCRIP:
        case MC_ATT_PATIENT_STATE:
        case MC_ATT_PREGNANCY_STATUS:
        case MC_ATT_MEDICAL_ALERTS:
        case MC_ATT_CONTRAST_ALLERGIES:
        case MC_ATT_SPECIAL_NEEDS:
        /*
        ** these tags are in the sequence, treat them the same.
        */
        case MC_ATT_REQUESTED_PROCEDURE_ID:
        case MC_ATT_REQUESTED_PROCEDURE_DESCRIPTION:
        case MC_ATT_STUDY_INSTANCE_UID:
        case MC_ATT_SCHEDULED_STATION_AE_TITLE:
        case MC_ATT_SCHEDULED_PROCEDURE_STEP_START_DATE:
        case MC_ATT_SCHEDULED_PROCEDURE_STEP_START_TIME:
        case MC_ATT_MODALITY:
        case MC_ATT_SCHEDULED_PERFORMING_PHYSICIANS_NAME:
        case MC_ATT_SCHEDULED_PROCEDURE_STEP_DESCRIPTION:
        case MC_ATT_SCHEDULED_STATION_NAME:
        case MC_ATT_SCHEDULED_PROCEDURE_STEP_LOCATION:
        case MC_ATT_SCHEDULED_PROCEDURE_STEP_ID:
                        
            attributeRec.nullFlag = FALSE;
            status = MC_Get_Value_To_String( A_msgItemId, A_tag,
                                             sizeof( char ) * PN_LEN, value );
            if ( status == MC_NULL_VALUE )
            {
                attributeRec.nullFlag = TRUE;
            }
            else if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_3, "DisposeAttribute", __LINE__, status,
                          "Request message cannot read a non-required tag:\n"
                          "  %s\n"
                          "Tag will not be in the response message.\n",
                          A_tag );
            }
            attributeRec.msgORitemID = A_msgItemId;
            attributeRec.tag = A_tag;
            attributeRec.valueRep = A_valueRep;
            attributeRec.numValues = A_numValues;
            strcpy( attributeRec.value, value );
            LLInsert( A_attributeList, &attributeRec );
            break;

        /*
        ** All of the rest of the tags are either group zero elements, or
        ** are defined as type 3 by DICOM, or are private, or just don't
        ** belong here.  We heave these into the giant bit bucket in the sky,
        ** since we aren't required to return them.
        */
        default:
            break;

    } /* end of switch */
}


/*****************************************************************************
**
** NAME
**    WorklistSearch 
**
** SYNOPSIS
**    void WorklistSearch
** ARGUMENTS
**    char A_stationAEtitle[ AE_LEN ]         - station AE title
**    char A_procStepStartDate[ DA_LEN*2 ]    - procedure step start date
**    char A_procStepStartTime[ TM_LEN*2 ]    - procedure step start time
**    char A_description[ PN_LEN ]            - procedure step description
**    char A_modality[ CS_LEN ]               - modality
**    char A_scheduledPerfPhysician[ PN_LEN ] - scheduled performing physician
**                                              name
**    char A_patientName[ PN_LEN ]            - patient name
**    char A_patientID[ LO_LEN ]              - patient id
**    LIST *A_workList                        - pointer to a completed worklist
**
** DESCRIPTION
**    This function builds a list of possible worklist entries from the
**    "database", and then attempts to find worklist entries that match
**    the key values that are passed into this function.
** RETURNS
**    nothing
**
**
*****************************************************************************/
void
WorklistSearch(
                char *A_stationAEtitle,
                char *A_procStepStartDate,
                char *A_procStepStartTime,
                char *A_description,
                char *A_modality,
                char *A_scheduledPerfPhysician,
                char *A_patientName,
                char *A_patientID,
                LIST *A_workList
              )
{
    PATIENT_REC        patientRec;
    REQ_PROC_REC       reqProcRec;
    SCH_PROC_STEP_REC  schProcStepRec;
    WORKLIST_REC       worklistRec;
    LIST               completeWorklist;
    char               startTime[ 7 ]="";
    int                nodeCount = 0;

   if ( LLCreate( &completeWorklist, sizeof( WORKLIST_REC ) ) == FAILURE )
   {
        LogError( ERROR_LVL_1, "WorklistSearch", __LINE__, MC_NORMAL_COMPLETION,
                  "Failed to create the linked list containing all\n"
                  "worklist entries.  Cannot query the worklist database.\n" );
   }

   /*
   ** Starting from the beginning of the patient list...
   */
   LLRewind( &G_patientList );

   /*
   ** ...we pull out all the attributes that we can, and build a complete
   ** worklist.
   */
   while ( LLPopNode( &G_patientList, &patientRec ) != NULL )
   {
       /*
       ** Copy all of the patient information into the worklist record.
       */
       strcpy( worklistRec.patientID, patientRec.patientID );
       strcpy( worklistRec.patientName, patientRec.patientName );
       strcpy( worklistRec.patientBirthDate, patientRec.patientBirthDate );
       strcpy( worklistRec.patientSex, patientRec.patientSex );
       strcpy( worklistRec.patientWeight, patientRec.patientWeight );

       /*
       ** And then look for all of the information out of the requested
       ** procedure information.
       */
       LLRewind( patientRec.requestedProcedureList );
       while( LLPopNode( patientRec.requestedProcedureList,
                         &reqProcRec ) != NULL )
       {
           /*
           ** Copy all of the requested procedure information into the worklist
           ** record.
           */
           strcpy( worklistRec.accessionNumber, reqProcRec.accessionNumber );
           strcpy( worklistRec.requestingPhysician,
                   reqProcRec.requestingPhysician );
           strcpy( worklistRec.referringPhysician,
                   reqProcRec.referringPhysician );
           strcpy( worklistRec.requestedProcID, reqProcRec.requestedProcID );
           strcpy( worklistRec.requestedProcPriority,
                   reqProcRec.requestedProcPriority );
           strcpy( worklistRec.studyInstanceUID, reqProcRec.studyInstanceUID );
           strcpy( worklistRec.requestedProcDescription,
                   reqProcRec.requestedProcDescription );

           /*
           ** and finally, look for all of the information out of the
           ** scheduled procedure record.
           */
           LLRewind( reqProcRec.scheduledProcedureStepList );
           while( LLPopNode( reqProcRec.scheduledProcedureStepList,
                             &schProcStepRec ) != NULL )
           {
               /*
               ** copy all of the information from the scheduled procedure
               ** step record.
               */
               strcpy( worklistRec.scheduledAETitle,
                       schProcStepRec.scheduledAETitle );
               strcpy( worklistRec.scheduledStartDate,
                       schProcStepRec.scheduledStartDate );
               /*
               ** We have an option to enter the scheduled procedure start
               ** date in the database file as "TODAY".  This allows a
               ** particular record to function as though is was scheduled for
               ** today's date.  This doesn't have anything to do with DICOM.
               ** This is a way to make the datafile more useable...
               */
               if ( strcmp( worklistRec.scheduledStartDate, "TODAY" ) == 0 )
               {
                   GetCurrentDate( worklistRec.scheduledStartDate,
                                   startTime );
               }
               strcpy( worklistRec.scheduledStartTime,
                       schProcStepRec.scheduledStartTime );
               strcpy( worklistRec.modality, schProcStepRec.modality );
               strcpy( worklistRec.scheduledPerformPhysicianName,
                       schProcStepRec.scheduledPerformPhysicianName );
               strcpy( worklistRec.scheduledProcStepDescription,
                       schProcStepRec.scheduledProcStepDescription );
               strcpy( worklistRec.scheduledProcStepLocation,
                       schProcStepRec.scheduledProcStepLocation );
               strcpy( worklistRec.scheduledProcStepID,
                       schProcStepRec.scheduledProcStepID );

               /*
               ** We have one other possibility that we need to take into
               ** account here.  That is, if a worklist item has already
               ** had a performed procedure started for it, then it doesn't
               ** belong in a worklist.
               ** We look at the existance of a performed procedure step
               ** list entry.
               */
               if ( LLNodes( reqProcRec.performedProcedureStepList ) == 0 )
               {
                   /*
                   ** If we have an empty performed procedure step list, then
                   ** we can take the assembled worklist node and insert it
                   ** into the complete work list.
                   */
                   if ( LLInsert( &completeWorklist,
                                  &worklistRec ) != SUCCESS )
                   {
                       LogError( ERROR_LVL_1, "WorklistSearch", __LINE__,
                                 MC_NORMAL_COMPLETION,
                                 "Failed to insert a worklist node into the\n"
                                 "complete worklist.\n" );
                   }
               } /* end of if we don't have a perf proc step list */

           } /* end of while schProcStepRec */

       } /* end of while reqProcRec */
                         
   } /* end of while patientList */

   /*
   ** OK, we now have a list that contains ALL possible worklist entries
   ** at this time.  We now need to attempt to match the data that the user
   ** has given us, with the data from our worklist.
   ** Then can query on: (we handle ONLY the required fields)
   **        stationAEtitle
   **        procStepStartDate
   **        procStepStartTime
   **        description
   **        modality
   **        scheduledPerfPhysician
   **        patientName
   **        patientID
   ** The match can be on any combination of these fields, and can include
   ** wildcards.  Also, just to compound the problem, dates and times can
   ** be compared with range matching.
   ** If any of the user's query options was retrieved as blank, we assume
   ** that they just wanted the result in the response, and that anything
   ** will match on that particular field.
   */
   /*
   ** First, we pop a node off of the complete worklist.
   */
   LLRewind( &completeWorklist );
   while( LLPopNode( &completeWorklist, &worklistRec ) != NULL )
   {
       if ( G_logit )
       {
           nodeCount++;
           printf( "WorklistSearch:  node:  %d\n", nodeCount );
       }
       /*
       ** We must check for the existance of wildcards in all of the user's
       ** query options.  If there are wildcards, then we have a match.
       ** Note that this code doesn't handle the "?" wildcard.  According to
       ** DICOM, the question mark should match a single character.  Also,
       ** this code will only handle a single "*" wildcard and will not
       ** match on a partial string followed by a wildcard (ex:  wild*).
       ** This will need to be implemented in an actual application.
       */
       if ( G_logit )
           printf( "StationAETitle =  sent:  '%s', database:  '%s'",
                    A_stationAEtitle,
                    worklistRec.scheduledAETitle );
       if ( strstr( A_stationAEtitle, "*" ) == NULL )
       {
           /*
           ** We DON'T have a wildcard within the user's query for this field.
           ** Then we check to see if the user entered anything.
           */
           if( strlen( A_stationAEtitle ) > 0 )
           {
               /*
               ** OK, they entered something.  Therefore we look for an
               ** exact match.
               */
               if ( strcmp( A_stationAEtitle,
                            worklistRec.scheduledAETitle ) != 0 )
               {
                   /*
                   ** We've failed...
                   */
                   if ( G_logit )
                       printf( "... FAILED.\n\n" );
                   continue;
               }
           }
       } /* end of if stationAEtitle doesn't contain a wildcard */
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       /*
       ** The only way we could've gotten this far is that the user entered
       ** a wildcard for their query, they entered nothing for their query,
       ** or what the actually did enter matches this database entry for the
       ** station AE title.  Now we match the next field.  Fear not, there are
       ** only 7 more matches to go...
       */
       if ( G_logit )
           printf( "Description =  sent:  '%s', database:  '%s'",
                    A_description,
                    worklistRec.scheduledProcStepDescription );
       if ( strstr( A_description, "*" ) == NULL )
       {
           /*
           ** We DON'T have a wildcard within the user's query for this field.
           ** Then we check to see if the user entered anything.
           */
           if( strlen( A_description ) > 0 )
           {
               /*
               ** OK, they entered something.  Therefore we look for an
               ** exact match.
               */
               if ( strcmp( A_description,
                            worklistRec.scheduledProcStepDescription ) != 0 )
               {
                   /*
                   ** We've failed...
                   */
                   if ( G_logit )
                       printf( "... FAILED.\n\n" );
                   continue;
               }
           }
       } /* end of if description doesn't contain a wildcard */
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       /*
       ** The only way we could've gotten this far is that the user entered
       ** a wildcard for their query, they entered nothing for their query,
       ** or what the actually did enter matches this database entry for the
       ** description.  Now we match the next field.  Fear not, there are
       ** only 6 more matches to go...
       */
       if ( G_logit )
           printf( "Modality =  sent:  '%s', database:  '%s'",
                    A_modality,
                    worklistRec.modality );
       if ( strstr( A_modality, "*" ) == NULL )
       {
           /*
           ** We DON'T have a wildcard within the user's query for this field.
           ** Then we check to see if the user entered anything.
           */
           if( strlen( A_modality ) > 0 )
           {
               /*
               ** OK, they entered something.  Therefore we look for an
               ** exact match.
               */
               if ( strcmp( A_modality,
                            worklistRec.modality ) != 0 )
               {
                   /*
                   ** We've failed...
                   */
                   if ( G_logit )
                       printf( "... FAILED.\n\n" );
                   continue;
               }
           }
       } /* end of if modality doesn't contain a wildcard */
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       /*
       ** The only way we could've gotten this far is that the user entered
       ** a wildcard for their query, they entered nothing for their query,
       ** or what the actually did enter matches this database entry for the
       ** modality.  Now we match the next field.  Fear not, there are
       ** only 5 more matches to go...
       */
       if ( G_logit )
           printf( "scheduledPerfPhysician =  sent:  '%s', database:  '%s'",
                    A_scheduledPerfPhysician,
                    worklistRec.scheduledPerformPhysicianName );
       if ( strstr( A_scheduledPerfPhysician, "*" ) == NULL )
       {
           /*
           ** We DON'T have a wildcard within the user's query for this field.
           ** Then we check to see if the user entered anything.
           */
           if( strlen( A_scheduledPerfPhysician ) > 0 )
           {
               /*
               ** OK, they entered something.  Therefore we look for an
               ** exact match.
               */
               if ( strcmp( A_scheduledPerfPhysician,
                            worklistRec.scheduledPerformPhysicianName ) != 0 )
               {
                   /*
                   ** We've failed...
                   */
                   if ( G_logit )
                       printf( "... FAILED.\n\n" );
                   continue;
               }
           }
       } /* end of if scheduledPerfPhysician doesn't contain a wildcard */
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       /*
       ** The only way we could've gotten this far is that the user entered
       ** a wildcard for their query, they entered nothing for their query,
       ** or what they actually did enter matches this database entry for the
       ** scheduled physician.  Now we match the next field.  Fear not, there
       ** are only 4 more matches to go...
       */
       if ( G_logit )
           printf( "patientName =  sent:  '%s', database:  '%s'",
                    A_patientName,
                    worklistRec.patientName );
       if ( strstr( A_patientName, "*" ) == NULL )
       {
           /*
           ** We DON'T have a wildcard within the user's query for this field.
           ** Then we check to see if the user entered anything.
           */
           if( strlen( A_patientName ) > 0 )
           {
               /*
               ** OK, they entered something.  Therefore we look for an
               ** exact match.
               */
               if ( strcmp( A_patientName,
                            worklistRec.patientName ) != 0 )
               {
                   /*
                   ** We've failed...
                   */
                   if ( G_logit )
                       printf( "... FAILED.\n\n" );
                   continue;
               }
           }
       } /* end of if patientName doesn't contain a wildcard */
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       /*
       ** The only way we could've gotten this far is that the user entered
       ** a wildcard for their query, they entered nothing for their query,
       ** or what the actually did enter matches this database entry for the
       ** patient name.  Now we match the next field.  Fear not, there are
       ** only 3 more matches to go...
       */
       if ( G_logit )
           printf( "patientID =  sent:  '%s', database:  '%s'",
                    A_patientID,
                    worklistRec.patientID );
       if ( strstr( A_patientID, "*" ) == NULL )
       {
           /*
           ** We DON'T have a wildcard within the user's query for this field.
           ** Then we check to see if the user entered anything.
           */
           if( strlen( A_patientID ) > 0 )
           {
               /*
               ** OK, they entered something.  Therefore we look for an
               ** exact match.
               */
               if ( strcmp( A_patientID,
                            worklistRec.patientID ) != 0 )
               {
                   /*
                   ** We've failed...
                   */
                   if ( G_logit )
                       printf( "... FAILED.\n\n" );
                   continue;
               }
           }
       } /* end of if patientID doesn't contain a wildcard */
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       /*
       ** The only way we could've gotten this far is that the user entered
       ** a wildcard for their query, they entered nothing for their query,
       ** or what the actually did enter matches this database entry for the
       ** patiend ID.  Now we match the next field.  Fear not, there are
       ** only 2 more matches to go...We have special functions to handle
       ** the date and time comparisons!
       */
       if ( G_logit )
           printf( "startDate =  sent:  '%s', database:  '%s'",
                    A_procStepStartDate,
                    worklistRec.scheduledStartDate );
       /*
       ** We compare start date with an empty string, because it may have
       ** been sent as a NULL.  A NULL matches with anything.
       */
       if ( strcmp( A_procStepStartDate, "" ) != 0 )
       {
           if ( CompareDateStrings( worklistRec.scheduledStartDate,
                                    A_procStepStartDate ) != TRUE )
           {
               /* failure */
               if ( G_logit )
                   printf( "... FAILED.\n\n" );
               continue;
           }
       }
       if ( G_logit )
           printf( "... SUCCESS.\n" );

       if ( G_logit )
           printf( "startTime =  sent:  '%s', database:  '%s'",
                    A_procStepStartTime,
                    worklistRec.scheduledStartTime );
       /*
       ** We compare start time with an empty string, because it may have
       ** been sent as a NULL.  A NULL matches with anything.
       */
       if ( strcmp( A_procStepStartTime, "" ) != 0 )
       {
           if ( CompareTimeStrings( worklistRec.scheduledStartTime,
                                    A_procStepStartTime ) != TRUE )
           {
               /* failure */
               if ( G_logit )
                   printf( "... FAILED.\n\n" );
               continue;
           }
       }
       if ( G_logit )
       {
           printf( "... SUCCESS.\n" );
           printf( "\n" );
       }

       /*
       ** If we've made it this far, that we have a match!  We now place this
       ** node onto the list that contains matching worklist entries.
       */
       LLInsert( A_workList, &worklistRec );

   } /* end of while completeWorklist */

   /*
   ** Since we are no longer using the complete worklist after we're done
   ** searching through it (and building the real worklist), we can free up
   ** the memory consumed by it.
   */
   LLDestroy( &completeWorklist );
}


/*****************************************************************************
**
** NAME
**    SendCFINDReply - Send a C-FIND reply message.
**
** SYNOPSIS
**    static int SendCFINDReply
**
** ARGUMENTS
**    A_associationID = The toolkit identifier for an open assiciation
**    workList        = a pointer to a linked list of worklist records
**    attributeList   = a pointer to a linked list containing all attributes
**                      sent to this SCP during a C_FIND request.
**
** DESCRIPTION
**   This routine assembles a reply message to a C-FIND request.  If the
**   user has requested that certain fields be retrieved, this routine
**   adds them to the reply message and sends it to the remote application.
**
** RETURNS
**    SUCCESS if the routine finishes properly.
**    FAILURE if the routine detects an error.
**
** SEE ALSO
**    MC_Empty_Message
**    MC_Free_Message
**    MC_Open_Message
**    MC_Send_Response_Message
**    MC_Set_Value_From_String
**
*****************************************************************************/

static int SendCFINDReply( int  A_associationID,
                           LIST *A_workList,
                           LIST *A_attributeList )
{
    MC_STATUS     status;
    int           messageID;
    int           objectID;
    int           ackMSG;
    int           item_id;
    int           readMessageID = -1;
    int           gotCancel = FALSE;
    int           ackSent = FALSE;
    char          *serviceName;
    MC_COMMAND    command =0;
    WORKLIST_REC  worklistRec;
    ATTRIBUTE_REC attributeRec;
    char          tagDescription[60];
    char*         valuePtr;

    VAL_SENT attribute_flag             =NOT_SENT; /* Flags for return values */


    static char   S_prefix[] = "SendCFINDReply";

    /* 
    ** Generally, for each worklist return entry in the workList, we must return
    ** the value, if it was requested.  We know the request status of each
    ** value because these are in attributeList.  We set a flag for every
    ** attribute that wasn't NULL.
    */
    /*
    ** NOTE - It would be nice if the sent attributes and the worklist were
    ** parsed at the same time.  Rather than complicate things unnecessarily,
    ** this app attempts to underscore the fact that there are some different/
    ** separate things going on here, in an easy to understand manner -- hence,
    ** the reason for doing this in separate steps.
    **
    ** A "real" application would probably need to do separate steps as well:
    **    1)  Decide which attributes were in the request message
    **    2)  Query a database and obtain a worklist that matches the user's
    **        query values. (using something like SQL)
    **    3)  Return the matching worklist entries based upon the attributes
    **        contained within the request message
    ** The point is that a "real" application would need to use
    ** "MC_Get_First/Next_Attribute..." to see which attributes were in the
    ** request message, query a database somehow, and return matching values
    ** based upon the user's requested data and the list of sent keys.
    */

    /*
    ** Since this sample application built its list of sent attributes in
    ** another part of the code, and passed this linked list to us,
    ** the first thing that we do is to parse exactly which attributes were
    ** set in the request message.  We trapse through the list of attributes
    ** and set a flag for each value that was sent.  This is a tri-state flag
    ** that is used to see if an attribute was sent, not sent, or null.
    */
    
    /*
    ** FIRST, we will open a reply message that will eventually be sent to the
    ** client.  This is a C_FIND response message.
    */
    status = MC_Open_Message ( &messageID,
                               "MODALITY_WORKLIST_FIND",
                               C_FIND_RSP );

    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                  "Could not open the worklist response message.\n" );
        return ( FAILURE );
    }    
    
    
    /*
    ** We now build the contents of our response message.  We do this based
    ** on the actual responses, and what the user asked to receive.
    ** Recall that "workList" is a list of records that matched the user's
    ** query.  For each node in the worklist, we must decide which attributes
    ** the user wanted back.  This is based on the attributes that they sent
    ** to us during the query, and upon DICOM itself.  Refer to Annex K in
    ** part 4 of the standard for complete information.
    */
    LLRewind( A_workList );
    while( ( LLPopNode( A_workList, &worklistRec ) != NULL) &&
           ( gotCancel == FALSE ) )
    {
       /*
        ** We have to open the item each time we use it because when
        ** MC_Empty_Message is called, the item is free'd, rather
        ** than emptied.  We start building the response message based on
        ** attributes that MUST be returned (type 1 ).  Is just so happens
        ** that a good part of these attributes are contained within an item
        ** (a.k.a. a sequence)...
        */
        status = MC_Open_Item( &item_id, "SCHEDULED_PROCEDURE_STEP" );

        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to open the PROCEDURE STEP "
                      "SEQUENCE item.\n" );
            MC_Free_Message ( &messageID );
            return ( FAILURE );
        }

        /*
        ** We then must place a reference to this item (sequence) into the
        ** original message.
        */
        status = MC_Set_Value_From_Int ( messageID,
                                         MC_ATT_SCHEDULED_PROCEDURE_STEP_SEQUENCE,
                                         item_id );
     
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            MC_Free_Item ( &item_id );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to set the ITEM ID:  %d\n", item_id );
            return ( FAILURE );
        } 


        /*
        ** We pop a single worklist entry off of the worklist.
        */
        printf ( "%s: Match Found: Patient Name = %s\n",
             S_prefix, worklistRec.patientName );


        /*
        ** Now, let's traverse through each element within the list of return
        ** attributes
        */
        LLRewind( A_attributeList );
        while( LLPopNode( A_attributeList, &attributeRec ) != NULL )
        {
            if( attributeRec.nullFlag != TRUE )
                attribute_flag = SENT;
            else
                attribute_flag = IS_NULL;
                
            switch( attributeRec.tag )
            {
                case MC_ATT_SCHEDULED_STATION_AE_TITLE:
                    objectID = item_id;
                    valuePtr = worklistRec.scheduledAETitle;
                    break;
                case MC_ATT_SCHEDULED_PROCEDURE_STEP_START_DATE:
                     objectID = item_id;
                     valuePtr = worklistRec.scheduledStartDate;
                    break;
                case MC_ATT_SCHEDULED_PROCEDURE_STEP_START_TIME:
                     objectID = item_id;
                     valuePtr = worklistRec.scheduledStartTime;
                    break;
                case MC_ATT_MODALITY:   /* modality */
                    objectID = item_id;
                    valuePtr = worklistRec.modality;
                    break;
                case MC_ATT_SCHEDULED_PERFORMING_PHYSICIANS_NAME:
                    objectID = item_id;
                    valuePtr = worklistRec.scheduledPerformPhysicianName;
                    break;
                case MC_ATT_SCHEDULED_PROCEDURE_STEP_DESCRIPTION:
                    objectID = item_id;
                    valuePtr = worklistRec.scheduledProcStepDescription;
                    break;
                case MC_ATT_SCHEDULED_STATION_NAME:
                    objectID = item_id;
                    valuePtr = NULL;
                    break;
                case MC_ATT_SCHEDULED_PROCEDURE_STEP_LOCATION:
                    objectID = item_id;
                    valuePtr = worklistRec.scheduledProcStepLocation;
                    break;
                case MC_ATT_SCHEDULED_ACTION_ITEM_CODE_SEQUENCE:
                    objectID = item_id;
                    valuePtr = NULL;
                    break;
                case MC_ATT_SCHEDULED_PROCEDURE_STEP_ID:
                    objectID = item_id;
                    valuePtr = worklistRec.scheduledProcStepID;
                    break;
                case MC_ATT_REQUESTED_PROCEDURE_ID:
                    objectID = messageID;
                    valuePtr = worklistRec.requestedProcID;
                    break;
                case MC_ATT_REQUESTED_PROCEDURE_DESCRIPTION:
                    objectID = messageID;
                    valuePtr = worklistRec.requestedProcDescription;
                    break;
                case MC_ATT_REQUESTED_PROCEDURE_CODE_SEQUENCE:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_STUDY_INSTANCE_UID:
                    objectID = messageID;
                    valuePtr = worklistRec.studyInstanceUID;
                    break;
                case MC_ATT_REFERENCED_STUDY_SEQUENCE:
                    objectID = messageID;
                    valuePtr = NULL;
                    break; 
                case MC_ATT_PATIENTS_NAME:
                    objectID = messageID;
                    valuePtr = worklistRec.patientName;
                    break;
                case MC_ATT_PATIENT_ID:
                    objectID = messageID;
                    valuePtr = worklistRec.patientID;
                    break;
                case MC_ATT_REQUESTED_PROCEDURE_PRIORITY:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_PATIENT_TRANSPORT_ARRANGEMENTS:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_ACCESSION_NUMBER:
                    objectID = messageID;
                    valuePtr = worklistRec.accessionNumber;
                    break;
                case MC_ATT_REQUESTING_PHYSICIAN:
                    objectID = messageID;
                    valuePtr = worklistRec.requestingPhysician;
                    break;
                case MC_ATT_REFERRING_PHYSICIANS_NAME:
                    objectID = messageID;
                    valuePtr = worklistRec.referringPhysician;
                    break;
                case MC_ATT_ADMISSION_ID:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_CURRENT_PATIENT_LOCATION:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_REFERENCED_PATIENT_SEQUENCE:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_PATIENTS_BIRTH_DATE:
                    objectID = messageID;
                    valuePtr = worklistRec.patientBirthDate;
                    break;
                case MC_ATT_PATIENTS_SEX:
                    objectID = messageID;
                    valuePtr = worklistRec.patientSex;
                    break;
                case MC_ATT_PATIENTS_WEIGHT:
                    objectID = messageID;
                    valuePtr = worklistRec.patientWeight;
                    break;
                case MC_ATT_CONFIDENTIALITY_CONSTRAINT_ON_PATIENT_DATA_DESCRIP:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_PATIENT_STATE:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_PREGNANCY_STATUS:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_MEDICAL_ALERTS:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_CONTRAST_ALLERGIES:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                case MC_ATT_SPECIAL_NEEDS:
                    objectID = messageID;
                    valuePtr = NULL;
                    break;
                default:
                    continue;
            } /* end of switch of attributeRec */


    
            if( attribute_flag != NOT_SENT )
            {
                if ( !valuePtr )
                {
                    status = MC_Set_Value_To_NULL ( objectID,
                                                    attributeRec.tag );
                    if ( status != MC_NORMAL_COMPLETION )
                    {
                        MC_Free_Message ( &messageID );
                        MC_Get_Tag_Info(attributeRec.tag, tagDescription, sizeof(tagDescription));
                        LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                                  "Failed to set %s to NULL" ,
                                  tagDescription);
                        return ( FAILURE );
                    }
                }
                else
                {
                    status = MC_Set_Value_From_String ( objectID,
                                                        attributeRec.tag,
                                                        valuePtr );
          
                    if ( status != MC_NORMAL_COMPLETION )
                    {
                        MC_Free_Message ( &messageID );
                        MC_Get_Tag_Info(attributeRec.tag, tagDescription, sizeof(tagDescription));
                        LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                                  "Failed to set %s to:  %s\n",
                                  tagDescription,
                                  valuePtr );
                        return ( FAILURE );
                    }
                }
            } 
      
        } /* end of while scanning attribute list */
      
      

        /*
        ** Study instance UID, which is type 1 return.
        */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_STUDY_INSTANCE_UID,
                                            worklistRec.studyInstanceUID );
      
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to set the study "
                      "instance UID:  %s\n",
                      worklistRec.studyInstanceUID );
            return ( FAILURE );
        }


      

        /*
        ** patient name, which is required...
        */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_PATIENTS_NAME,
                                            worklistRec.patientName );
      
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to set the patient's "
                      "name:  %s\n",
                      worklistRec.patientName );
            return ( FAILURE );
        }

        /*
        ** patient ID, which is required...
        */
        status = MC_Set_Value_From_String ( messageID,
                                            MC_ATT_PATIENT_ID,
                                            worklistRec.patientID );
      
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to set the patient ID:  %s\n",
                      worklistRec.patientID );
            return ( FAILURE );
        }


        /* Wow!  That should be it! */

        /*
        ** Here is where we finally send the response message back to the
        ** client.  This is the message that we were building in the
        ** above statements...
        */
        status = MC_Send_Response_Message ( A_associationID,
                                            C_FIND_PENDING,
                                            messageID );
     
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to send the response message to the SCU.\n" );
            return ( FAILURE );
        }

        /*
        ** After sending a reply message, we will look for any
        ** messages from the SCU.  The only message that we would
        ** expect at this point would be some type of cancel.
        ** After every reply we send, we will do a poll message
        ** to see if the SCU has attempted to tell us something...
        */
        status = MC_Read_Message( A_associationID, 0, &readMessageID,
                                  &serviceName, &command );
        /*
        ** Since we are polling the toolkit, we want to check for
        ** errors that AREN'T timeouts...
        */
        if ( ( status != MC_NORMAL_COMPLETION ) &&
             ( status != MC_TIMEOUT ) )
        {
             MC_Free_Message ( &messageID );
             LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                       "Failed to check for a cancel message from the "
                       "SCU.\n" );
             return ( FAILURE );
        }
        switch( command )
        {
            case C_CANCEL_RQ:
                printf( "--- Received a C_CANCEL_REQUEST\n" );
                /*
                ** At this point, since we've gotten a cancel, we
                ** want to stop sending the SCU reply messages.
                ** We want to make it look like we've done all
                ** the matching that we could.
                */
                gotCancel = TRUE;

                /*
                ** Now, we must open a response message, that is sent
                ** to the SCU, acknowledging our receipt of its CANCEL
                ** request.
                */
                status = MC_Open_Message ( &ackMSG,
                                           "MODALITY_WORKLIST_FIND",
                                           C_FIND_RSP );

                if ( status != MC_NORMAL_COMPLETION )
                {
                    LogError( ERROR_LVL_1, "SendCFINDReply",
                              __LINE__, status,
                              "Could not open the worklist message.\n" );
                }

                /*
                ** And then, send the message to the SCU...
                */
                printf( "--- Sending acknowledgement of "
                        "C_CANCEL_REQUEST message...\n" );
                status = MC_Send_Response_Message ( A_associationID,
                                            C_FIND_CANCEL_REQUEST_RECEIVED,
                                                   ackMSG );
    
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &ackMSG );
                    MC_Free_Message ( &messageID );
                    MC_Free_Message ( &readMessageID );
                    LogError( ERROR_LVL_1, "SendCFINDReply",
                              __LINE__, status,
                              "Failed to send the response message "
                              "to the SCU.\n" );
                    return ( FAILURE );
                }

                /*
                ** Once we are successful at sending the acknowledgement,
                ** we use this flag to make sure that we don't send a C_FIND
                ** with a SUCCESS later on.  The above message should be the
                ** last sent when a CANCEL happens.
                */
                ackSent = TRUE;

                /*
                ** And then, free the acknowledgement message...
                */
                status = MC_Free_Message ( &ackMSG );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &messageID );
                    LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                              "Failed to free the ack message object.\n" );
                    return ( FAILURE );
                }

                status = MC_Free_Message ( &readMessageID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    MC_Free_Message ( &messageID );
                    LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                              "Failed to free the cancel message object.\n" );
                    return ( FAILURE );
                }

                break;
            default:
                status = MC_Free_Message ( &ackMSG );

				/* If somethine other then a C_CANCEL_RQ was sent, even though
				 *  it shouldn't happen, we should free the readMessage
				 */
				if(readMessageID>0)
				{
					MC_Free_Message ( &readMessageID );
					readMessageID = -1;
				}
                break;
        }
 
        /*
        ** After the message is sent, we clear out the message.
        */
        status = MC_Empty_Message ( messageID );
     
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to empty the message object.\n" );
            return ( FAILURE );
        }

    } /* End of 'while' that walks us through the worklist list */
   
    /*
    ** Clear out the message for reuse one last time in the response
    */
    status = MC_Empty_Message ( messageID );
      
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &messageID );
        LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                  "Failed to empty the message object.\n" );
        return ( FAILURE );
    }
   
    /*
    ** If we haven't sent out a C_CANCEL acknowledgement message, then we
    ** send out a SUCCESS message.
    */
    if ( ackSent != TRUE )
    {
        /*
        ** Now we send back a C_FIND_SUCCESS message to signal the client
        ** that the response messages are complete and that the query was
        ** succesful.
        */
        status = MC_Send_Response_Message ( A_associationID,
                                            C_FIND_SUCCESS,
                                            messageID );
      
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                      "Failed to send the response message to the SCU.\n" );
            return ( FAILURE );
        }
    }

    /* -------- */
    /* Clean Up */
    /* -------- */
    status = MC_Free_Message ( &messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "SendCFINDReply", __LINE__, status,
                  "Failed to free the message object.\n" );
        return ( FAILURE );
    }

    return ( SUCCESS );

} /* End of SendCFINDReply */


/*****************************************************************************
**
** NAME
**    ProcessNCREATERQ - Handle the receipt of an N_CREATE request
**
** SYNOPSIS
**    static int ProcessNCREATERQ
**
** ARGUMENTS
**    int A_associationID      - the ID of a valid association
**    int A_messageID          - the ID of an N_CREATE message
**    RESP_STATUS *A_retStatus - a response string
**
** DESCRIPTION
**    Upon receipt of an N_CREATE message from a modality performed
**    procedure step SCU, this SCP will attempt to insert a MPPS instance,
**    and all of its data, into the database.  Also, after the MPPS instance
**    has been created, if this information came from a modality worklist,
**    the worklist will be marked as "not available" for worklist queries
**    anymore.
**
** RETURNS
**    SUCCESS or FAILURE
**
** SEE ALSO
**
*****************************************************************************/

static int
ProcessNCREATERQ(
                    int A_associationID,
                    int A_messageID,
                    RESP_STATUS *A_retStatus
                )
{
    MC_STATUS status;
    int       notFound = FALSE;
    char      patientID[ LO_LEN ];
    char      requestedProcID[ SH_LEN ];
    char      scheduledProcStepID[ SH_LEN ];
    PATIENT_REC       *patientRec;
    REQ_PROC_REC      *reqProcRec;
    SCH_PROC_STEP_REC *schProcStepRec;
    int       rspMsg;
    int       stepAttributeSeq;
    int       rspStepAttributeSeq;

    /*
    ** The first thing that we want to do when we receive an N_CREATE
    ** request message is to determine if we were given any patient
    ** information.  Unfortunately, these are type-2 attributes and
    ** cannot be depended upon being in the request.  If we have this
    ** information, then we will do a query of our internal database,
    ** looking for the patient (and worklist) that matches.  If we don't
    ** have any patient info, then we will create a dummy patient record
    ** in our database, and tie the MPPS to this dummy patient.
    ** The characteristics of the "database" that the MPPS SCP uses in
    ** real life will determine whether or not a developer will need to
    ** do the same.
    ** In these sample apps, an MPPS without patient information can be
    ** thought of as a STAT procedure -- one that wasn't scheduled via
    ** modality worklist.
    */
    status = MC_Get_Value_To_String( A_messageID,
                                     MC_ATT_PATIENT_ID,
                                     LO_LEN,
                                     patientID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        switch( status )
        {
            case MC_INVALID_TAG:
            case MC_EMPTY_VALUE:
            case MC_NULL_VALUE:
                /*
                ** These are good "bad" cases.
                */
                notFound = TRUE;
                break;
            default:
                LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                          "Unable to find patientID within MPPS "
                          "N_CREATE RQ\n" );
                return( FAILURE );
        }
    }

    /* read - scheduled step attribute sequence - type 1 */
    status = MC_Get_Value_To_Int( A_messageID,
                                  MC_ATT_SCHEDULED_STEP_ATTRIBUTES_SEQUENCE,
                                  &stepAttributeSeq );
    if ( status != MC_NORMAL_COMPLETION )
    {
        /*
        ** This is a type 1 attribute, and we can't find it with the request
        ** message.  This is bad, and we must stop here!
        */
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Failed to read the scheduled step attribute "
                  "sequence item from the request message.\n" );
        return( FAILURE );
    }

    /*
    ** "not found" refers to the ability to read the type 2 value for
    ** patient ID.
    */
    if ( notFound == TRUE )
    {
        /*
        ** We need to save our "dummy" key values, because these will
        ** be used instead of searching for key values from the N_CREATE
        ** message.
        */
        sprintf( patientID, "%d", G_dummy_patient );
        sprintf( requestedProcID, "%d", G_dummy_patient );
        sprintf( scheduledProcStepID, "%d", G_dummy_patient );
        G_dummy_patient++;

        /*
        ** If we couldn't find the patient ID within our N_CREATE message
        ** from the SCU, then we want to create a dummy patient record.
        ** and insert it into our database.  The "dummy" patient and its
        ** created records will contain a key that matches the global variable
        ** "G_dummy_patient".  We increment this global after each dummy
        ** patient we create.  We probably don't really need to do this, but
        ** it aides in being able to tell each "dummy" patient apart.
        */
        CreateDummyPatient( A_messageID, patientID );
    }
    else
    {
        /* 
        ** The next two needed values are contained within a sequence item.
        */

        /*
        ** If we were able to find a patient ID, then we attempt to search
        ** for other database data.  If we are unable to find this other
        ** information, needed for searching our database, then we must fail
        ** and insert a "dummy" record, just like above.
        */
        /* requested procedure => requestedProcID */
        status = MC_Get_Value_To_String( stepAttributeSeq,
                                         MC_ATT_REQUESTED_PROCEDURE_ID,
                                         SH_LEN,
                                         requestedProcID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Unable to find requested procedure ID within MPPS "
                      "N_CREATE RQ\n" );
        }

        /* scheduled procedure => scheduledProcStepID */
        status = MC_Get_Value_To_String( stepAttributeSeq,
                                         MC_ATT_SCHEDULED_PROCEDURE_STEP_ID,
                                         SH_LEN,
                                         scheduledProcStepID );
        if ( status != MC_NORMAL_COMPLETION )
        {
                LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                          "Unable to find scheduled procedure step ID "
                          " within MPPS N_CREATE RQ\n" );
        }
    }

    /*
    ** Now, attempt to find the patient from our database.
    ** Since we've already determined that the user has given us a patient ID
    ** key value, then we can attempt to search the database for the other
    ** necessary key values.  If we had created a dummy patient, requested
    ** procedure and scheduled procedure above, then we search for this
    ** dummy patient via the following function.  In both cases, we are given
    ** pointers to each relevant data.
    */
    patientRec = ObtainPatientPtr( patientID );
    reqProcRec = ObtainReqProcPtr( patientID, requestedProcID );
    schProcStepRec = ObtainSchProcStepPtr( patientID,
                                           requestedProcID,
                                           scheduledProcStepID );
    if ( ( patientRec == NULL ) || ( reqProcRec == NULL ) ||
         ( schProcStepRec == NULL ) )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to obtain a pointer to the database's internal\n"
                  "data structures.  Can not perform N_CREATE request.\n" );
        return( FAILURE );
    }

    /*
    ** Now that we have a pointer to the affected patient, requested procedure,
    ** and scheduled procedure, we can read in all of the other attributes
    ** from the request message, and set them up in the response message.
    ** We read the N_CREATE attributes because we want to fill in information
    ** within our "dummy" records, if it is available.  If we aren't dealing
    ** with a "dummy" record(s), then we don't replace the information that
    ** the database contains (except for requested procedure description and
    ** scheduled procedure step description, since these are more "known" to a
    ** modality than an IS machine).
    ** In either case, the replacement decision is based on this sample
    ** applications' design.  A real implementation will have varying
    ** requirements.
    */

    /*
    ** The first thing that we want to do is to open our N_CREATE response
    ** message.  We will be filling in attributes for the response message as
    ** we parse the request message.
    */
    status = MC_Open_Message( &rspMsg, "PERFORMED_PROCEDURE_STEP",
                              N_CREATE_RSP );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to open the MPPS N_CREATE response message\n" );
        return( FAILURE );
    }

    /*
    ** The response message contains a sequence, which we need to open.
    */
    status = MC_Open_Item( &rspStepAttributeSeq,
                           "SCHEDULED_STEP_ATTRIBUTE_SEQUENCE" );

    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Failed to open the scheduled step attribute "
                  "sequence item.\n" );
        return ( FAILURE );
    }

    /*
    ** and then place the reference to the item into the original message.
    */
    status = MC_Set_Value_From_Int( rspMsg,
                                    MC_ATT_SCHEDULED_STEP_ATTRIBUTES_SEQUENCE,
                                    rspStepAttributeSeq );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Item( &rspStepAttributeSeq );
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Failed to set the scheduled step attribute "
                  "sequence item into the response message.\n" );
        return ( FAILURE );
    }

    /*
    ** Now that we have a response message opened, and the initial item
    ** created and placed within it, we can go ahead with our parsing of
    ** the actual N_CREATE request message.
    */
    if ( SetNCREATERQ( A_messageID, stepAttributeSeq,
                       rspMsg, rspStepAttributeSeq,
                       patientRec, reqProcRec, schProcStepRec ) != SUCCESS )
    {
        /*
        ** Free the messages and quit.
        */
        MC_Free_Message ( &rspMsg );
        return( FAILURE );
    }

    /*
    ** And then, we can send the response message back to the SCU.
    */
    status = MC_Send_Response_Message( A_associationID,
                                       N_CREATE_SUCCESS,
                                       rspMsg );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to send the N-CREATE response message.\n" );
    }
    
    MC_Free_Message ( &rspMsg );
    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**    SetNCREATERQ
**
** SYNOPSIS
**    static int SetNCREATERQ
**
** ARGUMENTS
**    int A_messageID                     - the ID of the request message
**    int A_stepAttributeSeq              - the ID of a procedure step item
**    int A_rspMsg                        - the ID of a response message
**    int A_rspStepAttributeSeq           - the ID of a response sequence
**    PATIENT_REC       *A_patientRec     - a pointer to a patient record
**    REQ_PROC_REC      *A_reqProcRec     - a pointer to a requested procedure
**                                          step record
**    SCH_PROC_STEP_REC *A_schProcStepRec - a pointer to a scheduled procedure
**                                          step record.
**
** DESCRIPTION
**    This function will populate all of the attribute values in the N_CREATE
**    response message.  It will also perform the update of the database
**    should any of the request values require that the database be updated.
**
** RETURNS
**    SUCCESS or FAILURE
**
** SEE ALSO
**
*****************************************************************************/
static int
SetNCREATERQ(
                int A_messageID,
                int A_stepAttributeSeq,
                int A_rspMsg,
                int A_rspStepAttributeSeq,
                PATIENT_REC       *A_patientRec,
                REQ_PROC_REC      *A_reqProcRec,
                SCH_PROC_STEP_REC *A_schProcStepRec
            )
{
    MC_STATUS          status;
    int                perfSeriesSeq;
    int                rspPerfSeriesSeq;
    int                refImageSeq;
    int                rspRefImageSeq;
    int                refStudySeq;
    int                actItemCodeSeq;
    char               performedProcStepUID[ UI_LEN ];
    PERF_PROC_STEP_REC perfProcStepRec; /* records that we're adding to */
    PERF_SERIES_REC    perfSeriesRec;   /* the database                 */
    IMAGE_REC          imageRec;        /*  "         "        "        */

    /*
    ** Create the affected SOP instance UID (0000,1000) for return to the SCU.
    ** This is the "pointer" to our MPPS instance that we're going to create..
    ** This is also the performedProcStepUID from PERF_PROC_STEP_REC.  We
    ** attempt to read the requested value from the SCU.  If it isn't set,
    ** then we need to fail.  Unlike some other normalized services, the
    ** N_CREATE for MPPS specifies that the SCU must send an affected SOP
    ** instance UID.
    */
    status = MC_Get_Value_To_String( A_messageID,
                                     MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                     (sizeof( char ) * UI_LEN),
                                     performedProcStepUID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Failed to read the requested SOP instance "
                  "UID attribute "
                  "from the request message.\n" );
        return( FAILURE );
    }
    else
    {
        /*
        ** Should we have found the requested SOP class UID from the SCU, then
        ** we need to use this value in our database.
        */
        strcpy( perfProcStepRec.performedProcStepUID, performedProcStepUID );
    }

    /* 
    ** We attempted to open the scheduled step attribute sequence above, when
    ** we read in our initial search criteria.  Don't do it again, here...
    */

    /*
    ** We've already created the response message's sequence item, and placed
    ** the reference to it within the response message.  Don't do it again...
    */

    /* study instance UID, within the sequence - type 1 */
    if ( RetrieveAndSetValue( A_stepAttributeSeq,
                              A_rspStepAttributeSeq,
                              MC_ATT_STUDY_INSTANCE_UID,
                              TYPE_1,
                              TYPE_1,
                              A_reqProcRec->studyInstanceUID,
                              UI_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* accession number, within the sequence - type 2 */
    if ( RetrieveAndSetValue( A_stepAttributeSeq,
                              A_rspStepAttributeSeq,
                              MC_ATT_ACCESSION_NUMBER,
                              TYPE_2,
                              TYPE_2,
                              A_reqProcRec->accessionNumber,
                              SH_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** We need to also handle referenced study sequence.  We don't support
    ** it, but we need to send back a NULL if it was in the request message.
    */
    status = MC_Get_Value_To_Int( A_stepAttributeSeq,
                                  MC_ATT_REFERENCED_STUDY_SEQUENCE,
                                  &refStudySeq );
    if ( ( status == MC_NORMAL_COMPLETION ) ||
         ( status == MC_NULL_VALUE ) )
    {
        status = MC_Set_Value_To_NULL( A_rspStepAttributeSeq,
                                  MC_ATT_REFERENCED_STUDY_SEQUENCE );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to NULL the referenced study sequence." );
        }
    } /* end of if */
    else
    {
        /*
        ** We don't really care if there was an error or not, this tag just
        ** won't be in the response.
        */
    }

    /*
    ** We need to also handle scheduled action item code sequence.  We
    ** don't support it, but we need to send back a NULL if it was in the
    ** request message.
    */
    status = MC_Get_Value_To_Int( A_stepAttributeSeq,
                                  MC_ATT_SCHEDULED_ACTION_ITEM_CODE_SEQUENCE,
                                  &actItemCodeSeq );
    if ( ( status == MC_NORMAL_COMPLETION ) ||
         ( status == MC_NULL_VALUE ) )
    {
        status = MC_Set_Value_To_NULL( A_rspStepAttributeSeq,
                                  MC_ATT_SCHEDULED_ACTION_ITEM_CODE_SEQUENCE );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to NULL the scheduled action item code "
                      "sequence." );
        }
    } /* end of if */
    else
    {
        /*
        ** We don't really care if there was an error or not, this tag just
        ** won't be in the response.
        */
    }

    /*
    ** We need to also handle scheduled procedure step ID.  We
    ** don't support it, but we need to send back a NULL if it was in the
    ** request message.
    */
    if ( RetrieveAndSetValue( A_stepAttributeSeq,
                              A_rspStepAttributeSeq,
                              MC_ATT_SCHEDULED_PROCEDURE_STEP_ID,
                              TYPE_2,
                              TYPE_2,
                              A_schProcStepRec->scheduledProcStepID,
                              SH_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** We need to also handle requested procedure ID.  We
    ** don't support it, but we need to send back a NULL if it was in the
    ** request message.
    */
    if ( RetrieveAndSetValue( A_stepAttributeSeq,
                              A_rspStepAttributeSeq,
                              MC_ATT_REQUESTED_PROCEDURE_ID,
                              TYPE_2,
                              TYPE_2,
                              A_reqProcRec->requestedProcID,
                              SH_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* requested procedure description, within the sequence - type 2 */
    if ( RetrieveAndSetValue( A_stepAttributeSeq,
                              A_rspStepAttributeSeq,
                              MC_ATT_REQUESTED_PROCEDURE_DESCRIPTION,
                              TYPE_2,
                              TYPE_2,
                              A_reqProcRec->requestedProcDescription,
                              LO_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** We've already attempted to obtain scheduled procedure step ID, since
    ** it's a key value
    */

    /* scheduled procedure step description, within the sequence - type 2 */
    if ( RetrieveAndSetValue( A_stepAttributeSeq,
                              A_rspStepAttributeSeq,
                              MC_ATT_SCHEDULED_PROCEDURE_STEP_DESCRIPTION,
                              TYPE_2,
                              TYPE_2,
                              A_schProcStepRec->scheduledProcStepDescription,
                              LO_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* patient name - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PATIENTS_NAME,
                              TYPE_2,
                              TYPE_2,
                              A_patientRec->patientName,
                              PN_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* patientID */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PATIENT_ID,
                              TYPE_2,
                              TYPE_2,
                              A_patientRec->patientID,
                              LO_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* patient's birth date - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PATIENTS_BIRTH_DATE,
                              TYPE_2,
                              TYPE_2,
                              A_patientRec->patientBirthDate,
                              DA_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }
                                     
    /* patient's sex - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PATIENTS_SEX,
                              TYPE_2,
                              TYPE_2,
                              A_patientRec->patientSex,
                              CS_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** Now, we are going to be dealing with data that isn't in the database
    ** yet.  The next attributes will mandate that we insert a performed
    ** procedure record, performed series record, and possibly image records
    ** into the database.
    */
    /* performed procedure step ID - type 1 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_ID,
                              TYPE_1,
                              TYPE_1,
                              perfProcStepRec.performedProcStepID,
                              CS_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed station AE title - type 1 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_STATION_AE_TITLE,
                              TYPE_1,
                              TYPE_1,
                              perfProcStepRec.performedAETitle,
                              AE_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed station name - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_STATION_NAME,
                              TYPE_2,
                              TYPE_2,
                              perfProcStepRec.performedStationName,
                              SH_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed location - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_LOCATION,
                              TYPE_2,
                              TYPE_2,
                              perfProcStepRec.performedLocation,
                              PN_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed procedure step start date - type 1 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_START_DATE,
                              TYPE_1,
                              TYPE_1,
                              perfProcStepRec.performedProcStepStartDate,
                              DA_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed procedure step start time - type 1 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_START_TIME,
                              TYPE_1,
                              TYPE_1,
                              perfProcStepRec.performedProcStepStartTime,
                              TM_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed procedure step status - type 1 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_STATUS,
                              TYPE_1,
                              TYPE_1,
                              perfProcStepRec.performedProcStepStatus,
                              CS_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** We need to also handle performed series sequence.  We
    ** don't support it, but we need to send back a NULL if it was in the
    ** request message.
    */
    status = MC_Get_Value_To_Int( A_messageID,
                                  MC_ATT_PERFORMED_SERIES_SEQUENCE,
                                  &perfSeriesSeq );
    if ( ( status == MC_NORMAL_COMPLETION ) ||
         ( status == MC_NULL_VALUE ) )
    {
        status = MC_Set_Value_To_NULL( A_rspMsg,
                                  MC_ATT_PERFORMED_SERIES_SEQUENCE );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to NULL the performed series "
                      "sequence." );
        }
    } /* end of if */
    else
    {
        /*
        ** We don't really care if there was an error or not, this tag just
        ** won't be in the response.
        */
    }

    /* performed procedure step description  - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_DESCRIPTION,
                              TYPE_2,
                              TYPE_2,
                              perfProcStepRec.performedProcStepDescription,
                              LO_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed procedure type description - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_TYPE_DESCRIPTION,
                              TYPE_2,
                              TYPE_2,
                              perfProcStepRec.performedProcTypeDescription,
                              LO_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed procedure step end date - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_END_DATE,
                              TYPE_2,
                              TYPE_2,
                              perfProcStepRec.performedProcStepEndDate,
                              DA_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* performed procedure step end time - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_END_TIME,
                              TYPE_2,
                              TYPE_2,
                              perfProcStepRec.performedProcStepEndTime,
                              TM_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* modality - type 1 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_MODALITY,
                              TYPE_1,
                              TYPE_1,
                              perfProcStepRec.modality,
                              CS_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /* study id - type 2 */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsg,
                              MC_ATT_STUDY_ID,
                              TYPE_2,
                              TYPE_2,
                              A_reqProcRec->studyID,
                              SH_LEN,
                              "ProcessNCREATERQ",
                              FALSE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** and now that the performed procedure step record is completed, we
    ** can insert it into the performed procedure step list that is part of
    ** the requested procedure list.
    */
    if ( InsertPerfProc( &perfProcStepRec,
                         A_patientRec->patientID,
                         A_reqProcRec->requestedProcID ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Failed to insert the performed procedure step "
                  "record into the database for:\n"
                  "patientID:  %s\nrequestedProcID:  %s\n",
                   A_patientRec->patientID, A_reqProcRec->requestedProcID );
        return( FAILURE );
    }

    /* performed series sequence - type 2 */
    status = MC_Get_Value_To_Int( A_messageID,
                                  MC_ATT_PERFORMED_SERIES_SEQUENCE,
                                  &perfSeriesSeq );
    if ( status != MC_NORMAL_COMPLETION )
    {
        /*
        ** If there was a series, then we continue.  If not, then we can
        ** most likely just continue.
        */
        switch( status )
        {
            case MC_INVALID_TAG:
            case MC_EMPTY_VALUE:
            case MC_NULL_VALUE:
                /*
                ** These are "good"-bad values.
                */
                break;
            default:
                LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                          "Failed to read the performed series sequence  "
                          "from the request message.\n" );
                break;
        } /* end of switch */

    } /* end of if */

    /*
    ** Since there may not be a performed series sequence, we only want to
    ** handle the attributes that make it up when we are able to read it from
    ** the request message.
    */
    if ( status == MC_NORMAL_COMPLETION )
    {
        /*
        ** We need to open a sequence in the response message.
        */
        status = MC_Open_Item( &rspPerfSeriesSeq,
                               "PERFORMED_SERIES_SEQUENCE" );

        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to open the performed series sequence "
                      "item.\n" );
            return ( FAILURE );
        }

        /*
        ** and then place the reference to the item into the original message.
        */
        status = MC_Set_Value_From_Int( A_rspMsg,
                                        MC_ATT_PERFORMED_SERIES_SEQUENCE,
                                        rspPerfSeriesSeq );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Item( &rspPerfSeriesSeq );
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to set the performed series attribute "
                      "sequence item into the response message.\n" );
            return ( FAILURE );
        }

        /* Now, use the sequence to obtain some series information */
        /* performing physician's name, part of perfSeriesSeq - type 1 */
        if ( RetrieveAndSetValue( perfSeriesSeq,
                                  rspPerfSeriesSeq,
                                  MC_ATT_PERFORMING_PHYSICIANS_NAME,
                                  TYPE_1,
                                  TYPE_1,
                                  perfSeriesRec.performingPhysicianName,
                                  PN_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /* protocol name */
        if ( RetrieveAndSetValue( perfSeriesSeq,
                                  rspPerfSeriesSeq,
                                  MC_ATT_PROTOCOL_NAME,
                                  TYPE_1,
                                  TYPE_1,
                                  perfSeriesRec.protocolName,
                                  LO_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /* operator's name */
        if ( RetrieveAndSetValue( perfSeriesSeq,
                                  rspPerfSeriesSeq,
                                  MC_ATT_OPERATORS_NAME,
                                  TYPE_2,
                                  TYPE_2,
                                  perfSeriesRec.operatorName,
                                  PN_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /* series instance UID */
        if ( RetrieveAndSetValue( perfSeriesSeq,
                                  rspPerfSeriesSeq,
                                  MC_ATT_SERIES_INSTANCE_UID,
                                  TYPE_1,
                                  TYPE_1,
                                  perfSeriesRec.seriesInstanceUID,
                                  UI_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /* series description */
        if ( RetrieveAndSetValue( perfSeriesSeq,
                                  rspPerfSeriesSeq,
                                  MC_ATT_SERIES_DESCRIPTION,
                                  TYPE_2,
                                  TYPE_2,
                                  perfSeriesRec.seriesDescription,
                                  LO_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /* retrieve AE title */
        if ( RetrieveAndSetValue( perfSeriesSeq,
                                  rspPerfSeriesSeq,
                                  MC_ATT_RETRIEVE_AE_TITLE,
                                  TYPE_2,
                                  TYPE_2,
                                  perfSeriesRec.retrieveAETitle,
                                  AE_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /*
        ** and we are done with the performed series and can insert it into the
        ** database.
        */
        if ( InsertPerfSeries( &perfSeriesRec,
                               A_patientRec->patientID,
                               A_reqProcRec->requestedProcID,
                               perfProcStepRec.performedProcStepUID )
                                                        != SUCCESS )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to insert the performed procedure step "
                      "record into the database for:\n"
                      "patientID:  %s\nrequestedProcID:  %s\n",
                      "performedProcStepUID:  %s\n",
                      A_patientRec->patientID, A_reqProcRec->requestedProcID,
                      perfProcStepRec.performedProcStepUID );
            return( FAILURE );
        }

    } /* end of: if we've read a performed series sequence from the request */

    /* referenced image sequence */
    status = MC_Get_Value_To_Int( A_messageID,
                                  MC_ATT_REFERENCED_IMAGE_SEQUENCE,
                                  &refImageSeq );
    if ( status != MC_NORMAL_COMPLETION )
    {
        /*
        ** If there was a sequence, then we continue.  If not, then we can
        ** most likely just continue.
        */
        switch( status )
        {
            case MC_INVALID_TAG:
            case MC_EMPTY_VALUE:
            case MC_NULL_VALUE:
                /*
                ** These are "good"-bad values.
                */
                break;
            default:
                LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                          "Failed to read the referenced image sequence "
                          "attribute "
                          "from the request message.\n" );
                return( FAILURE );
        } /* end of switch */
    }
    /*
    ** If we are able to read a referenced image sequence from the request
    ** message, then we want to make sure that an image record is properly
    ** setup in the database, as well as responded to in the RSP message.
    */
    if ( status == MC_NORMAL_COMPLETION )
    {
        /*
        ** We need to open a sequence in the response message.
        */
        status = MC_Open_Item( &rspRefImageSeq,
                               "REFERENCED_IMAGE_SEQUENCE" );

        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to open the referenced image sequence "
                      "item.\n" );
        }

        /*
        ** and then place the reference to the item into the original message.
        */
        status = MC_Set_Value_From_Int( perfSeriesSeq,
                                        MC_ATT_REFERENCED_IMAGE_SEQUENCE,
                                        rspRefImageSeq );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Item( &rspRefImageSeq );
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to set the referenced image series attribute "
                      "sequence item into the response message.\n" );
            return( FAILURE );
        }

        /* referenced SOP class UID */
        if ( RetrieveAndSetValue( refImageSeq,
                                  rspRefImageSeq,
                                  MC_ATT_REFERENCED_SOP_CLASS_UID,
                                  TYPE_1,
                                  TYPE_1,
                                  imageRec.sopClassUID,
                                  UI_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /* referenced SOP instance UID */
        if ( RetrieveAndSetValue( refImageSeq,
                                  rspRefImageSeq,
                                  MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                  TYPE_1,
                                  TYPE_1,
                                  imageRec.sopInstanceUID,
                                  UI_LEN,
                                  "ProcessNCREATERQ",
                                  FALSE ) != SUCCESS )
        {
            return( FAILURE );
        }

        /*
        ** ...and we are done with the image record and can insert it into the
        ** database.
        */
        if ( InsertImage( &imageRec,
                          A_patientRec->patientID,
                          A_reqProcRec->requestedProcID,
                          perfProcStepRec.performedProcStepUID ) != SUCCESS )
        {
            LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                      "Failed to insert the image "
                      "record into the database for:\n"
                      "patientID:  %s\nrequestedProcID:  %s\n"
                      "performedProcStepUID:  %s\n",
                       A_patientRec->patientID, A_reqProcRec->requestedProcID,
                       perfProcStepRec.performedProcStepUID );
            return( FAILURE );
        }

    } /* end of: if we've read a referenced image sequence in the RQ msg */
   
    /*
    ** NULL out the values that are type 2, and we don't support them.
    */
    status = MC_Set_Value_To_NULL( A_rspMsg,
                                   MC_ATT_REFERENCED_PATIENT_SEQUENCE );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to set NULL value for referenced patient sequence "
                  "in the N_CREATE_RSP message\n" );
        return( FAILURE );
    }

    status = MC_Set_Value_To_NULL( A_rspMsg,
                                   MC_ATT_PROCEDURE_CODE_SEQUENCE );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to set NULL value for procedure code sequence "
                  "in the N_CREATE_RSP message\n" );
        return( FAILURE );
    }

    status = MC_Set_Value_To_NULL( A_rspMsg,
                                   MC_ATT_PERFORMED_ACTION_ITEM_SEQUENCE );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to set NULL value for performed action item sequence "
                  "in the N_CREATE_RSP message\n" );
        return( FAILURE );
    }

    /*
    ** Set the affected SOP instance UID group zero element.
    */

    printf( "Sending affected SOP instance UID:  %s\n", performedProcStepUID );
    status = MC_Set_Value_From_String( A_rspMsg,
                                       MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                       performedProcStepUID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNCREATERQ", __LINE__, status,
                  "Unable to set the value for affected SOP instance UID "
                  "in the N_CREATE_RSP message\n" );
        return( FAILURE );
    }

    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**    ProcessNSETRQ - Handle the receipt of an N_SET request
**
** SYNOPSIS
**    static int ProcessNSETRQ
**
** ARGUMENTS
**    int  A_associationID - A valid association ID
**    int  A_messageID     - the ID of an N_SET request message
**
** DESCRIPTION
**                    Upon receipt of an N_SET message, this function
**                    examines the Performed Procedure Step status.  If
**                    the status was sent and is now being updated to
**                    COMPLETED or DISCONTINUED, then some checking is
**                    done to ensure that a performed series sequence exists.
**                    If the status isn't being updated, then the particular
**                    MPPS that is being updated is retrieved, through the
**                    referenced SOP instance UID, and the appropriate
**                    N_SET updates are accomplished.
**
** RETURNS
**     FAILURE or SUCCESS
**
** SEE ALSO
**
*****************************************************************************/

static int
ProcessNSETRQ(
                 int  A_associationID,
                 int  A_messageID
             )
{
    MC_STATUS status;
    RESP_STATUS respStatus;
    int       rspMsgID = -1;
/*    int       perfSeriesSeqID = -1;
    int       imageSeqID = -1; */
    int       rspPerfSeriesSeqID = 0;
    int       rspImageSeqID = 0;
    char      reqSOPinstance[ UI_LEN ];
    char      performedProcStepStatus[ CS_LEN ] = "";
    PATIENT_REC        patientRec;
    REQ_PROC_REC       reqProcRec;
    PERF_PROC_STEP_REC *modPerfProcRec;
    PERF_PROC_STEP_REC perfProcStepRec;
    PERF_SERIES_REC    perfSeriesRec;
    NODE               *dataNode = NULL;
    void               *dataPtr;

    /*
    ** Before doing anything, we open a N_SET response message, a performed
    ** series sequence, and an image sequence.  We then place the references
    ** to both series' into the response message.
    */
    status = MC_Open_Message( &rspMsgID, "PERFORMED_PROCEDURE_STEP",
                              N_SET_RSP );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to open the MPPS N_SET response message\n" );
        return( FAILURE );
    }
    status = MC_Open_Item( &rspPerfSeriesSeqID, "PERFORMED_SERIES_SEQUENCE" );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to open the performed series sequence\n" );
        return( FAILURE );
    }
    status = MC_Set_Value_From_Int( rspMsgID,
                                    MC_ATT_PERFORMED_SERIES_SEQUENCE,
                                    rspPerfSeriesSeqID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        /*
        ** If we can't set the seq. ID into the message, then we need to
        ** free the item we've created, before exiting.
        */
        MC_Free_Item( &rspPerfSeriesSeqID );
        MC_Free_Message ( &rspMsgID );
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to set the performed series sequence "
                  "in the response message\n" );
        return( FAILURE );
    }
    status = MC_Open_Item( &rspImageSeqID, "REFERENCED_IMAGE_SEQUENCE" );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to open the referenced image sequence\n" );
        return( FAILURE );
    }
    status = MC_Set_Value_From_Int( rspPerfSeriesSeqID,
                                    MC_ATT_REFERENCED_IMAGE_SEQUENCE,
                                    rspImageSeqID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        /*
        ** If we can't set the seq. ID into the message, then we need to
        ** free the item we've created, before exiting.
        */
        MC_Free_Item( &rspImageSeqID );
        MC_Free_Message ( &rspMsgID );
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to set the referenced image sequence "
                  "in the performed series sequence, in the\n"
                  "response message\n" );
        return( FAILURE );
    }
    
    /*
    ** Now that we have the reponse message object, we can continue with
    ** attempting to validate the the N_SET request message sent to us in this
    ** request matches the rules set forth by DICOM, with respect to the
    ** message's status.
    **
    **
    ** First, we attempt to obtain the requested SOP instance UID from
    ** the N_SET message.  This will be used to validate that the MPPS
    ** instance actually exists in our database.
    */
    status = MC_Get_Value_To_String( A_messageID,
                                     MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                     sizeof( reqSOPinstance ),
                                     reqSOPinstance );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to read the requested SOP instance UID in the "
                  "N_SET request message.\n"
                  "Cannot process N_SET request message.\n" );
        status = MC_Send_Response_Message( A_associationID,
                                           N_SET_MISSING_ATTRIBUTE_VALUE,
                                           rspMsgID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNSETRQ",
                      __LINE__, status,
                      "Unable to send the N_SET response "
                      "message to the SCU\n" );
        }

        return( FAILURE );
    }

    if ( G_logit )
        printf( "N_SET_RQ:  received a requested SOP instance UID of:\n%s\n",
                reqSOPinstance );

    /*
    ** We then can set the affected sop instance UID in the response message.
    ** This will let the SCU know which SOP instance we're working with -- and
    ** it had better be the same one!
    */
    status = MC_Set_Value_From_String( rspMsgID,
                                       MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                       reqSOPinstance );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to set the affected SOP instance UID in the "
                  "N_SET response message.\n" );
        status = MC_Send_Response_Message( A_associationID,
                                           N_SET_MISSING_ATTRIBUTE_VALUE,
                                           rspMsgID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNSETRQ",
                      __LINE__, status,
                      "Unable to send the N_SET response "
                      "message to the SCU\n" );
        }
        return( FAILURE );
    }

    /*
    ** Once we have the requested SOP instance UID, we can tell which MPPS
    ** instance we are being told to N_SET.  We now need to use the requested
    ** SOP instance UID in order to obtain the MPPS instance from our internal
    ** database.  The data that can be N_SET is contained within the
    ** performed procedure step, performed series, or image records within
    ** the database.  We must search for performed procedure step records,
    ** based on the requested SOP instance UID -- which is also the same as
    ** the performedProcStepUID.
    ** dataNode is a pointer to the node that contains the actual performed
    ** procedure step record.
    */
    dataNode = SelectMPPSFromDatabase( &patientRec,
                                       &reqProcRec,
                                       &perfProcStepRec,
                                       &perfSeriesRec,
                                       reqSOPinstance );

    /*
    ** OK. We've either searched the entire database without finding a matching
    ** key value, or we've suceeded in finding a match.  Which one was it?
    */
    if ( dataNode == NULL )
    {
        /*
        ** If we can't find what the SCU wants us to N_SET, then we must fail.
        */
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to find the requested SOP instance UID in the "
                  "database:  %s\n"
                  "Cannot perform N_SET operation "
                  "on an unknown MPPS\n", reqSOPinstance );
        status = MC_Send_Response_Message( A_associationID,
                                           N_SET_NO_SUCH_SOP_INSTANCE,
                                           rspMsgID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNSETRQ",
                      __LINE__, status,
                      "Unable to send the N_SET response "
                      "message to the SCU\n" );
        }
        return( FAILURE );
    }

    /*
    ** At this point, we've found the performed procedure step record that
    ** the SCU wants to perform the N_SET upon.  "node" is also pointing to
    ** that particular node.  We can now obtain a pointer to that node's data
    ** such that we can modify that data via the N_SET request.
    ** We use "modPerfProcRec" to MODify the data during an N_SET operation,
    ** thus providing us with the SET capability.  This isn't important in a
    ** "real" database, and is only important to this sample application's
    ** "database".
    ** This is because our database has no API to perform UPDATE's.  We just
    ** simply modify the actual data.  You wouldn't do this with a real
    ** application!
    ** All of this updating would most likely be done via an SQL-like database
    ** that the SCP manages.
    */
    dataPtr = LLGetDataPtr( dataNode );
    modPerfProcRec = (PERF_PROC_STEP_REC *)dataPtr;

    if ( G_logit )
        printf( "  MPPS database status:  %s\n",
                modPerfProcRec->performedProcStepStatus );

    if ( ( respStatus = CheckIfComplete( A_associationID,
                                         A_messageID,
                                         modPerfProcRec,
                                         performedProcStepStatus )) !=
                                                      N_SET_SUCCESS )
    {
        /*
        ** For one of the reasons that was generated by the above function,
        ** we cannot modify this MPPS instance.  We must send a response
        ** message that tells this to the user.
        */
        status = MC_Send_Response_Message( A_associationID,
                                           respStatus,
                                           rspMsgID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                      "Unable to send the N_SET response message to the "
                      "SCU\n" );
        }
        /*
        ** Since we are done, we can discard the response message and all of
        ** its contents.
        */
        status = MC_Free_Message ( &rspMsgID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                      "Failed to free the message object.\n" );
        }
        return( FAILURE );
    }
    else if ( G_logit )
    {
        printf( "  MPPS N_SET status:  NOT SENT\n" );
    }
                
    /*
    ** Since they didn't send us a status FIELD that was completed or
    ** discontinued, then we can go about the N_SET without having to worry
    ** about all of the rules that pertain to the setting of a status to
    ** COMPLETED or DISCONTINUED.
    */
 
    /*
    ** OR: we have all of the rules taken care of.  If we've somehow
    ** made it this far, then we can start parsing all of the request
    ** tags and start building the response message.
    */

    if ( SetMPPSValues( A_messageID, rspMsgID,
                        rspPerfSeriesSeqID, rspImageSeqID,
                        &perfProcStepRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Failed to set the MPPS values in the database.\n" );
        return( FAILURE );
    }

    /*
    ** Once the request message is parsed, the response message is built,
    ** and the database is updated with the N_SET messages, we can send the
    ** N_SET response message back to the SCU.
    */
    status = MC_Send_Response_Message( A_associationID,
                                       N_SET_SUCCESS,
                                       rspMsgID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Unable to send the N_SET response message to the "
                  "SCU\n" );
    }

	MC_Free_Message ( &rspMsgID );

    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**     SelectMPPSFromDatabase
**
** SYNOPSIS
**     NODE *SelectMPPSFromDatabase
**
** ARGUMENTS
**     PATIENT_REC        *A_patientRec      - A pointer to the database's
**                                             patient record
**     REQ_PROC_REC       *A_reqProcRec      - A pointer to the database's
**                                             requested procedure record
**     PERF_PROC_STEP_REC *A_perfProcStepRec - A pointer to the database's
**                                             performed procedure step record
**     PERF_SERIES_REC    *A_perfSeriesRec   - A pointer to the database's
**                                             performed series record
**     char               *A_reqSOPinstance  - The key value to search the
**                                             database for.
**
** DESCRIPTION
**     This function searches the database for a matching SOP instance UID.
**     Once found, pointers to the database's internal structores are obtained
**     so that the actual data can be modified.
**
** RETURNS
**     A pointer to a NODE
** SEE ALSO
**
*****************************************************************************/
NODE *
SelectMPPSFromDatabase(
                          PATIENT_REC        *A_patientRec,
                          REQ_PROC_REC       *A_reqProcRec,
                          PERF_PROC_STEP_REC *A_perfProcStepRec,
                          PERF_SERIES_REC    *A_perfSeriesRec,
                          char               *A_reqSOPinstance
                      )
{
    int                found = FALSE;
    NODE               *node = NULL;

    if( G_logit )
        printf( "ProcessNSETRQ:  Searching for MPPS:\n" );
    LLRewind( &G_patientList );
    while( LLPopNode( &G_patientList, A_patientRec )  != NULL )
    {
        /*
        ** After we obtain a patient, obtain the requested procedure rec
        ** for that patient.
        */
        LLRewind( A_patientRec->requestedProcedureList );
        while( LLPopNode( A_patientRec->requestedProcedureList,
                          A_reqProcRec ) != NULL )
        {
            /*
            ** Then we need to look at the actual performed procedure step
            ** records for a patient/requested procedure combination.
            */
            LLRewind( A_reqProcRec->performedProcedureStepList );
            node = NULL;
            while( node == NULL )
            {
                node = LLPopNode( A_reqProcRec->performedProcedureStepList,
                                    A_perfProcStepRec );
                if ( node == NULL )
                    break;

                /*
                ** And then look for a matching key.
                */
                if ( strcmp( A_perfProcStepRec->performedProcStepUID,
                             A_reqSOPinstance ) == 0 )
                {
                    /*
                    ** Match found.
                    */
                    found = TRUE;
                    if ( G_logit )
                        printf( "  MPPS found:  %s\n",
                                A_perfProcStepRec->performedProcStepUID );
                    break;
                }
            } /* end of while, perfProcStepRec */
            /*
            ** If we found a matching perf proc step, then we must break.
            */
            if ( found == TRUE )
                break;
        } /* end of while, reqProcRec */
            /*
            ** If we found a matching perf proc step, then we must break.
            */
        if ( found == TRUE )
            break;
    } /* end of while, A_patientList */
    return( node );
}


/*****************************************************************************
**
** NAME 
**    SetMPPSValues
**
** SYNOPSIS
**    static int SetMPPSValues
**
** ARGUMENTS
**    int                A_messageID          - a valid message ID
**    int                A_rspMsgID           - a valid response msg ID
**    int                A_rspPerfSeriesSeqID - a response sequence ID
**    int                A_rspImageSeqID      - a response sequence ID
**    PERF_PROC_STEP_REC *A_perfProcStepRec   - a pointer to the database's
**                                              performed procedure step rec
**
** DESCRIPTION
**    This function will attempt to perform the actual database updates
**    necessary to place items into the database.
**
** RETURNS
**    SUCCESS or FAILURE
**
** SEE ALSO
**
*****************************************************************************/
static int
SetMPPSValues(
                 int                A_messageID,
                 int                A_rspMsgID,
                 int                A_rspPerfSeriesSeqID,
                 int                A_rspImageSeqID,
                 PERF_PROC_STEP_REC *A_perfProcStepRec
             )
{
    MC_STATUS status;
    int       perfSeriesSeqID = -1;
    char      seriesInstanceUID[ UI_LEN ];
    void      *dataPtr;
    NODE      *node;
    PERF_SERIES_REC *modPerfSeriesRec;

    /* 
    ** In the case of an N_SET response, we choose to send back ALL data,
    ** whether or not the data is contained within the request message or not.
    ** This is a design decision.  A real implementation may make a different
    ** decision!
    ** Note that RetrieveAndSetValue has the ability to send back a value
    ** even if it isn't able to retrieve a value within the request message.
    ** This can be switched on or off based on the final argument to the
    ** function call:  TRUE=always send back something.
    */

    /*
    ** First, performed procedure step status. Type 3/1
    */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsgID,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_STATUS,
                              TYPE_3,
                              TYPE_1,
                              A_perfProcStepRec->performedProcStepStatus,
                              CS_LEN,
                              "ProcessNSETRQ",
                              TRUE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** Then, we look for:  performed procedure step description (type 3/2).
    ** We support it, so we will provide them with an intelligent response.
    */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsgID,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_DESCRIPTION,
                              TYPE_3,
                              TYPE_2,
                              A_perfProcStepRec->performedProcStepDescription,
                              LO_LEN,
                              "ProcessNSETRQ",
                              TRUE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** Then, we look for:  performed procedure type description (type 3/2).
    */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsgID,
                              MC_ATT_PERFORMED_PROCEDURE_TYPE_DESCRIPTION,
                              TYPE_3,
                              TYPE_2,
                              A_perfProcStepRec->performedProcTypeDescription,
                              LO_LEN,
                              "ProcessNSETRQ",
                              TRUE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** Then, we look for:  procedure code sequence (type 3/2), which we don't
    ** support.
    */
    status = MC_Set_Value_To_NULL( A_rspMsgID,
                                   MC_ATT_PROCEDURE_CODE_SEQUENCE );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Failed to NULL the procedure code sequence "
                  "in the N_SET response message.\n" );
        return( FAILURE );
    }

    /*
    ** Then, we look for:  performed procedure step end date (type 3/1).
    ** NOTE:  In this case, we consider the return type to be of type 2.  That
    ** is why it is different below.  The standard is inconsistant in the case
    ** of the date and time, such that if the user requests the date or time
    ** before it is actually set, we can't provide them with it.
    */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsgID,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_END_DATE,
                              TYPE_3,
                              TYPE_2,
                              A_perfProcStepRec->performedProcStepEndDate,
                              DA_LEN,
                              "ProcessNSETRQ",
                              TRUE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** Then, we look for:  performed procedure step end time (type 3/1).
    ** NOTE:  In this case, we consider the return type to be of type 2.  That
    ** is why it is different below.  The standard is inconsistant in the case
    ** of the date and time, such that if the user requests the date or time
    ** before it is actually set, we can't provide them with it.
    */
    if ( RetrieveAndSetValue( A_messageID,
                              A_rspMsgID,
                              MC_ATT_PERFORMED_PROCEDURE_STEP_END_TIME,
                              TYPE_3,
                              TYPE_2,
                              A_perfProcStepRec->performedProcStepEndTime,
                              TM_LEN,
                              "ProcessNSETRQ",
                              TRUE ) != SUCCESS )
    {
        return( FAILURE );
    }

    /*
    ** Then, we deal with:  performed action item sequence (type 3/2),
    ** which we don't support.  Since it's type 2 return, we send them back
    ** a NULL, no matter what.
    */
    status = MC_Set_Value_To_NULL( A_rspMsgID,
                              MC_ATT_PERFORMED_ACTION_ITEM_SEQUENCE );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                  "Failed to NULL the performed action item sequence "
                  "in the N_SET response message.\n" );
        return( FAILURE );
    }

    /*
    ** Then, we see if we were able to obtain the performed series sequence
    ** from above.  If we did obtain it, then we must get all of the attributes
    ** that are part of that sequence.
    ** We may already attempted to obtain the sequence while validating the
    ** "rules" above when we checked the status.
    ** In any case, were're going to look for it again.  That's why this is
    ** here, and seems to be repeated.
    **
    ** Also, the performed series sequence may be a multiple valued attribute.
    ** This is how series' are sent.  We need to use MC_Get_Next_Value...
    ** insead of MC_Get_Value...
    */
    while( perfSeriesSeqID != 0 )
    {
        status = MC_Get_Next_Value_To_Int( A_messageID,
                                           MC_ATT_PERFORMED_SERIES_SEQUENCE,
                                           &perfSeriesSeqID );
        switch( status )
        {
            case MC_NORMAL_COMPLETION:
                break;
            case MC_NULL_VALUE:
            case MC_INVALID_TAG:
            case MC_NO_MORE_VALUES:
                perfSeriesSeqID = 0;
                break;
            default:
                LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                          "Failed to get the performed series sequence ID\n"
                          "from the N_SET request message.\n" );
                perfSeriesSeqID = 0;
                break;
        }

        /*
        ** If we have a performed series sequence Id, then we are able to look
        ** for a matching one on within the database.  If we don't have any
        ** items on the list, then we will be updating an item that doesn't
        ** exist in our database.
        ** Since an N_SET ALWAYS does an update (sometimes on a "blank" item)
        ** we interpret this as an insert.  For this case, we end up inserting
        ** a dummy item into the performed series list, and then
        ** we modify the dummy item.
        */
        if ( perfSeriesSeqID > 0 )
        {
            /*
            ** Since we've found a series, we need to get the series instance
            ** UID so that we can see if it matches.
            */
            status = MC_Get_Value_To_String( perfSeriesSeqID,
                                             MC_ATT_SERIES_INSTANCE_UID,
                                             sizeof( seriesInstanceUID ),
                                             seriesInstanceUID );
            switch ( status )
            {
                case MC_NORMAL_COMPLETION:
                    break;
                case MC_INVALID_TAG:
                case MC_NULL_VALUE:
                    strcpy( seriesInstanceUID, "" );
                    break;
                default:
                    LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                              "Failed to get the series instance UID "
                              "from the N_SET request message.\n" );
                    return( FAILURE );
            }
            /*
            ** We want to send back an actual response if either the N_SET_RQ
            ** contained an actual value, or if the database contains a good
            ** value.
            */
            if ( strcmp( seriesInstanceUID, "" ) == 0 )
            {
                /* 
                ** If they didn't send us an series instance UID, then we
                ** can't go on.
                */
                LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                          "Failed to read a valid series instance UID "
                          "from the N_SET request message.\n"
                          "The N_SET request will fail.\n" );
                return( FAILURE );
            }

            node = SelectSeriesInstanceFromDatabase( A_perfProcStepRec,
                                                     seriesInstanceUID );
            if ( node == NULL )
            {
                LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                          "Failed to obtain a series instance UID from "
                          "the database, or create an empty one.\n" );
                return( FAILURE );
            }

            /*
            ** Obtain the pointers to the actual data.
            */
            dataPtr = LLGetDataPtr( node );
            modPerfSeriesRec = (PERF_SERIES_REC *)dataPtr;

            if ( RetrieveAndSetValue( perfSeriesSeqID,
                                      A_rspPerfSeriesSeqID,
                                      MC_ATT_SERIES_INSTANCE_UID,
                                      TYPE_3,
                                      TYPE_1,
                                      modPerfSeriesRec->seriesInstanceUID,
                                      UI_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }

            /*
            ** Get the performing physician's name (type 2/2).
            */
            if ( RetrieveAndSetValue( perfSeriesSeqID,
                                      A_rspPerfSeriesSeqID,
                                      MC_ATT_PERFORMING_PHYSICIANS_NAME,
                                      TYPE_2,
                                      TYPE_2,
                                      modPerfSeriesRec->performingPhysicianName,
                                      PN_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }

            /*
            ** protocol name (type 1/1).
            */
            if ( RetrieveAndSetValue( perfSeriesSeqID,
                                      A_rspPerfSeriesSeqID,
                                      MC_ATT_PROTOCOL_NAME,
                                      TYPE_1,
                                      TYPE_1,
                                      modPerfSeriesRec->protocolName,
                                      LO_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }

            /*
            ** operator's name (type 2/2).
            */
            if ( RetrieveAndSetValue( perfSeriesSeqID,
                                      A_rspPerfSeriesSeqID,
                                      MC_ATT_OPERATORS_NAME,
                                      TYPE_2,
                                      TYPE_2,
                                      modPerfSeriesRec->operatorName,
                                      PN_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }
    
            /*
            ** series description (type 2/2).
            */
            if ( RetrieveAndSetValue( perfSeriesSeqID,
                                      A_rspPerfSeriesSeqID,
                                      MC_ATT_SERIES_DESCRIPTION,
                                      TYPE_2,
                                      TYPE_2,
                                      modPerfSeriesRec->seriesDescription,
                                      LO_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }
    
            /*
            ** retrieve AE title (type 2/2).
            */
            if ( RetrieveAndSetValue( perfSeriesSeqID,
                                      A_rspPerfSeriesSeqID,
                                      MC_ATT_RETRIEVE_AE_TITLE,
                                      TYPE_2,
                                      TYPE_2,
                                      modPerfSeriesRec->retrieveAETitle,
                                      AE_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }

            /*
            ** referenced image sequence (type 2/2).
            ** The user had better send us a referenced image sequence if the
            ** status is complete, and the database doesn't contain any image
            ** records.
            ** We already validated above that the rule is correct.  The only
            ** thing left to do at this time is to put the pieces of the image
            ** record into the response message, if they should exist, and to
            ** update the database, if it should be updated.
            ** If we were sent a status during the validation, we would've
            ** obtained the sequence ID already.  In our case, we will just
            ** get it again.
            */
      
            if ( CheckForImageSQ( A_perfProcStepRec,
                                  perfSeriesSeqID,
                                  A_rspImageSeqID ) != SUCCESS )
            {
                LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                          "Failed to find an image sequence, or to create "
                          "an empty one.\n" );
                return( FAILURE );
            }

            /*
            ** Referenced standalone SOP instance sequence (type 2/2), which we
            ** don't support.
            */
            status = MC_Set_Value_To_NULL( A_rspPerfSeriesSeqID,
                           MC_ATT_REFERENCED_STANDALONE_SOP_INSTANCE_SEQUENCE );
            if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                          "Failed to NULL the referenced standalone SOP "
                          "instance "
                          "sequence in the N_SET response message.\n" );
                return( FAILURE );
            }
        

        } /* end of "if we have a performed series sequence in the RQ */
        else
        {
            /* 
            ** If we weren't sent a performed series sequence, then we also
            ** attempt to send back what the database actually containes
            ** (even if the request message chooses not to N_SET these
            ** attributes).  This is an implementation decision.  A real
            ** application may choose to do this differently.
            **
            ** First, we look for the performed series sequence in our
            ** database.
            */
            if ( LLNodes( A_perfProcStepRec->performedSeriesList ) > 0 )
            {
                /*
                ** If there was one, then we make sure all of its components are
                ** contained within the response message.
                */
                DumpAttributes( A_perfProcStepRec, A_rspMsgID,
                                A_rspPerfSeriesSeqID,
                                A_rspImageSeqID );
            }
            else
            {
                /*
                ** If there wasn't one, then we empty out this value in our
                ** response message.
                */
                status = MC_Set_Value_To_Empty( A_rspMsgID,
                                            MC_ATT_PERFORMED_SERIES_SEQUENCE );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                              "Failed to empty the performed series sequence "
                              "sequence in the N_SET response message.\n" );
                }
            }

        }
    } /* end of while there are more performed series sequences */
    return( SUCCESS );
}

/*****************************************************************************
**
** NAME
**    SelectSeriesInstanceFromDatabase
**
** SYNOPSIS
**    NODE *SelectSeriesInstanceFromDatabase
**
** ARGUMENTS
**    PERF_PROC_STEP_REC *A_perfProcStepRec   - pointer to the datbase's
**                                              performed procedure step rec
**    char               *A_seriesInstanceUID - a series instance UID
**
** DESCRIPTION
**    This function will attempt to find a pointer to a performed series
**    instance from the database, based on a series instance UID.
**
** RETURNS
**    A pointer to the found node or NULL
**
** SEE ALSO
**
*****************************************************************************/
NODE *
SelectSeriesInstanceFromDatabase(
                                    PERF_PROC_STEP_REC *A_perfProcStepRec,
                                    char               *A_seriesInstanceUID
                                )
{
    int             found = FALSE;
    NODE            *node;
    PERF_SERIES_REC perfSeriesRec;

    /*
    ** Now, we can search the database for the series instance UID that
    ** they sent to us.
    */
    LLRewind( A_perfProcStepRec->performedSeriesList );
    while( ( node = LLPopNode( A_perfProcStepRec->performedSeriesList,
                               &perfSeriesRec ) ) != NULL ) 
    {
        if ( strcmp( perfSeriesRec.seriesInstanceUID,
                     A_seriesInstanceUID ) == 0 )
        {
            /*
            ** We've found the series instance that they've given to us.
            */
            found = TRUE;
            break;
        }
    } /* end of while */
    
    if ( found == TRUE )
    {
        return( node );
    }
    else
    {
        /*
        ** If we can't find a node in the database, then we will insert
        ** a dummy one, and return the pointer to it.
        */
        strcpy( perfSeriesRec.performingPhysicianName, "" );
        strcpy( perfSeriesRec.protocolName, "" );
        strcpy( perfSeriesRec.operatorName, "" );
        strcpy( perfSeriesRec.seriesInstanceUID, A_seriesInstanceUID );
        strcpy( perfSeriesRec.seriesDescription, "" );
        strcpy( perfSeriesRec.retrieveAETitle, "" );
        if ( LLInsert( A_perfProcStepRec->performedSeriesList,
                       &perfSeriesRec ) != SUCCESS )
        {
            /*
            ** Failure
            */
        }

        /*
        ** Now, we need a pointer to the node that we've just inserted.
        */
        LLRewind( A_perfProcStepRec->performedSeriesList );
        while( ( node = LLPopNode( A_perfProcStepRec->performedSeriesList,
                                   &perfSeriesRec ) ) != NULL ) 
        {
            if ( strcmp( perfSeriesRec.seriesInstanceUID,
                         A_seriesInstanceUID ) == 0 )
            {
                /*
                ** We've found the series instance that they've given to us.
                */
                found = TRUE;
                break;
            }
        } /* end of while */
    }

    return( node );
}


/*****************************************************************************
**
** NAME
**    DumpAttributes
**
** SYNOPSIS
**    For all attributes contained within a performed series sequence,
**    this function will dump the values from the database that correspond
**    to the sequence.
**
** ARGUMENTS
**    PERF_PROC_STEP_REC *A_perfProcStepRec - pointer to a performed procedure
**    int                A_rspMsgID         - a response message ID
**
** DESCRIPTION
**
** RETURNS
**
** SEE ALSO
**
*****************************************************************************/
static void
DumpAttributes( 
                  PERF_PROC_STEP_REC *A_perfProcStepRec,
                  int                A_rspMsgID,
                  int                A_rspSeriesID,
                  int                A_rspImageID
              )
{
    MC_STATUS       status;
    PERF_SERIES_REC perfSeriesRec;
    IMAGE_REC       imageRec;

    /*
    ** First, we need to find the performed series record from the list.
    */
    LLRewind( A_perfProcStepRec->performedSeriesList );
    LLPopNode( A_perfProcStepRec->performedSeriesList, &perfSeriesRec );

    /*
    ** Performing physician's name
    */
    if ( strcmp( perfSeriesRec.performingPhysicianName, "" ) != 0 )
    {
        status = MC_Set_Value_From_String( A_rspSeriesID,
                                           MC_ATT_PERFORMING_PHYSICIANS_NAME,
                                        perfSeriesRec.performingPhysicianName );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to set the performed physician's "
                              "name in the N_SET response message.\n" );
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_rspSeriesID,
                                       MC_ATT_PERFORMING_PHYSICIANS_NAME );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to NULL the performed physician's "
                              "name in the N_SET response message.\n" );
        }
    }

    /*
    ** Protocol name
    */
    if ( strcmp( perfSeriesRec.protocolName, "" ) != 0 )
    {
        status = MC_Set_Value_From_String( A_rspSeriesID,
                                           MC_ATT_PROTOCOL_NAME,
                                           perfSeriesRec.protocolName );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to set the protocol name "
                              "in the N_SET response message.\n" );
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_rspSeriesID,
                                       MC_ATT_PROTOCOL_NAME );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to NULL the protocol name "
                              "in the N_SET response message.\n" );
        }
    }

    /*
    ** operator's name
    */
    if ( strcmp( perfSeriesRec.operatorName, "" ) != 0 )
    {
        status = MC_Set_Value_From_String( A_rspSeriesID,
                                           MC_ATT_OPERATORS_NAME,
                                           perfSeriesRec.operatorName );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to set the operator name "
                              "in the N_SET response message.\n" );
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_rspSeriesID,
                                       MC_ATT_OPERATORS_NAME );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to NULL the operator name "
                              "in the N_SET response message.\n" );
        }
    }

    /*
    ** series instance UID
    */
    if ( strcmp( perfSeriesRec.seriesInstanceUID, "" ) != 0 )
    {
        status = MC_Set_Value_From_String( A_rspSeriesID,
                                           MC_ATT_SERIES_INSTANCE_UID,
                                           perfSeriesRec.seriesInstanceUID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to set the series instance UID "
                              "in the N_SET response message.\n" );
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_rspSeriesID,
                                       MC_ATT_SERIES_INSTANCE_UID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to NULL the series instance UID "
                              "in the N_SET response message.\n" );
        }
    }

    /*
    ** Series Description
    */
    if ( strcmp( perfSeriesRec.seriesDescription, "" ) != 0 )
    {
        status = MC_Set_Value_From_String( A_rspSeriesID,
                                           MC_ATT_SERIES_DESCRIPTION,
                                           perfSeriesRec.seriesDescription );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to set the series description "
                              "in the N_SET response message.\n" );
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_rspSeriesID,
                                       MC_ATT_SERIES_DESCRIPTION );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to NULL the series description "
                              "in the N_SET response message.\n" );
        }
    }

    /*
    ** retrieve AE title
    */
    if ( strcmp( perfSeriesRec.retrieveAETitle, "" ) != 0 )
    {
        status = MC_Set_Value_From_String( A_rspSeriesID,
                                           MC_ATT_RETRIEVE_AE_TITLE,
                                           perfSeriesRec.retrieveAETitle );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to set the retrieve AE title "
                              "in the N_SET response message.\n" );
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_rspSeriesID,
                                       MC_ATT_RETRIEVE_AE_TITLE );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to NULL the retrieve AE title "
                              "in the N_SET response message.\n" );
        }
    }

    /*
    ** Referenced image sequence
    */
    LLRewind( A_perfProcStepRec->imageList );
    if( LLPopNode( A_perfProcStepRec->imageList, &imageRec ) != NULL )
    {
        /*
        ** referenced SOP class UID
        */
        if ( strcmp( imageRec.sopClassUID, "" ) != 0 )
        {
            status = MC_Set_Value_From_String( A_rspImageID,
                                             MC_ATT_REFERENCED_SOP_CLASS_UID,
                                               imageRec.sopClassUID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                                  "Failed to set the referenced SOP class UID "
                                  "in the N_SET response message.\n" );
            }
        }
        else
        {
            status = MC_Set_Value_To_NULL( A_rspImageID,
                                           MC_ATT_REFERENCED_SOP_CLASS_UID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                                  "Failed to NULL the referenced SOP class UID "
                                  "in the N_SET response message.\n" );
            }
        }

        /*
        ** referenced SOP instance UID
        */
        if ( strcmp( imageRec.sopInstanceUID, "" ) != 0 )
        {
            status = MC_Set_Value_From_String( A_rspImageID,
                                            MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                               imageRec.sopInstanceUID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                                  "Failed to set the ref SOP instance UID "
                                  "in the N_SET response message.\n" );
            }
        }
        else
        {
            status = MC_Set_Value_To_NULL( A_rspImageID,
                                           MC_ATT_REFERENCED_SOP_INSTANCE_UID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                                  "Failed to NULL the ref SOP instance UID "
                                  "in the N_SET response message.\n" );
            }
        }
    }
    else
    {
        /*
        ** If we don't have a references image sequence, then we empty the
        ** attribute from the response message.
        */
        status = MC_Set_Value_To_Empty( A_rspImageID,
                                        MC_ATT_REFERENCED_IMAGE_SEQUENCE );
        if ( status != MC_NORMAL_COMPLETION )
        {
            LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                              "Failed to empty the referenced image sequence "
                              "in the N_SET response message.\n" );
        }
    }

    status = MC_Set_Value_To_NULL( A_rspSeriesID,
                           MC_ATT_REFERENCED_STANDALONE_SOP_INSTANCE_SEQUENCE );
    if ( status != MC_NORMAL_COMPLETION )
    {
        LogError( ERROR_LVL_1, "DumpAttributes", __LINE__, status,
                          "Failed to NULL the standalone sop instance sequence "
                          "in the N_SET response message.\n" );
    }
}


/*****************************************************************************
**
** NAME
**    CheckForImageSQ
**
** SYNOPSIS
**    static int CheckForImageSQ
**
** ARGUMENTS
**    PERF_PROC_STEP_REC *A_perfProcStepRec - pointer to a performed procedure
**    int                A_perfSeriesSeqID  - a performed series sequence ID
**    int                A_rspImageSeqID    - a response image sequence ID
**
** DESCRIPTION
**    Given a performed series sequence ID and a response imge sequence ID,
**    this function attempts to find an image sequence.
**
** RETURNS
**    SUCCESS or FAILURE
**
** SEE ALSO
**
*****************************************************************************/
static int
CheckForImageSQ(
                   PERF_PROC_STEP_REC *A_perfProcStepRec,
                   int                A_perfSeriesSeqID,
                   int                A_rspImageSeqID
               )
{
    MC_STATUS status;
    int       imageSeqID = -1;
    char      sopInstanceUID[ UI_LEN ];
    NODE      *node;
    void      *dataPtr;
    IMAGE_REC *modImageRec;

    while ( imageSeqID != 0 )
    {
        status = MC_Get_Next_Value_To_Int( A_perfSeriesSeqID,
                                           MC_ATT_REFERENCED_IMAGE_SEQUENCE,
                                           &imageSeqID );
        switch( status )
        {
            case MC_NORMAL_COMPLETION:
                break;
            case MC_INVALID_TAG:
            case MC_NULL_VALUE:
            case MC_NO_MORE_VALUES:
                imageSeqID = 0;
                break;
            default:
            LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                      "Failed to get the referenced imate sequence  "
                      "from the N_SET request message.\n" );
        }
        if ( imageSeqID > 0 )
        {
            /*
            ** Since they sent us a referenced image sequence, then
            ** these two fields must be in the message.
            ** referenced SOP instance UID (type 1/1).
            */
            status = MC_Get_Value_To_String( imageSeqID,
                                             MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                             sizeof( sopInstanceUID ),
                                             sopInstanceUID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                LogError( ERROR_LVL_1, "ProcessNSETRQ",
                          __LINE__, status,
                          "Failed to get the referenced SOP "
                          "instance UID "
                          "in the N_SET request message.\n" );
                return( FAILURE );
            }
            /*
            ** Now we need to see if this particular SOP instance
            ** exists in the database.  If it does, then we can do
            ** the N_SET.  If not then we can't.
            */
            node = SelectImageRecFromDatabase( A_perfProcStepRec,
                                               sopInstanceUID );
            if ( node == NULL )
            {
                LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                          "Failed to obtain an image record from the "
                          "database.\n" );
                return( FAILURE );
            }

            /*
            ** Obtain the pointers to the actual data.
            */
            dataPtr = LLGetDataPtr( node );
            modImageRec = (IMAGE_REC *)dataPtr;

            if ( RetrieveAndSetValue( imageSeqID,
                                      A_rspImageSeqID,
                                      MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                      TYPE_1,
                                      TYPE_1,
                                      modImageRec->sopInstanceUID,
                                      UI_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }

            /*
            ** referenced SOP class UID (type 1/1).
            */
            if ( RetrieveAndSetValue( imageSeqID,
                                      A_rspImageSeqID,
                                      MC_ATT_REFERENCED_SOP_CLASS_UID,
                                      TYPE_1,
                                      TYPE_1,
                                      modImageRec->sopClassUID,
                                      UI_LEN,
                                      "ProcessNSETRQ",
                                      TRUE ) != SUCCESS )
            {
                return( FAILURE );
            }

        } /* end of if image sequence exists in the request message */

    } /* end of while there are image sequences */
    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**     SelectImageRecFromDatabase
**
** SYNOPSIS
**     NODE *SelectImageRecFromDatabase
**
** ARGUMENTS
**     PERF_PROC_STEP_REC *A_perfProcStepRec - a pointer to a performed
**                                             procedure step record
**     char               *A_sopInstanceUID  - a sop instance UID of an image
**
** DESCRIPTION
**     This function attempts to find a matching image record in the database
**     based on the sop instance passed in.
**
** RETURNS
**     A pointer to the node from the database that contains the match.
**
** SEE ALSO
**
*****************************************************************************/
NODE *
SelectImageRecFromDatabase(
                              PERF_PROC_STEP_REC *A_perfProcStepRec,
                              char               *A_sopInstanceUID
                          )
{
    int       found = FALSE;
    IMAGE_REC imageRec;
    NODE      *node;

    LLRewind( A_perfProcStepRec->imageList );
    while( ( node = LLPopNode( A_perfProcStepRec->imageList,
                               &imageRec ) ) != NULL )
    {
        if ( strcmp( imageRec.sopInstanceUID, A_sopInstanceUID ) == 0 )
        {
            /*
            ** We've found the SOP instance that they've given
            ** to us.
            */
            found = TRUE;
            break;
        }
    } /* end of while */
    if ( found == TRUE )
    {
        return( node );
    }
    else
    {
        /*
        ** If we can't find a node in the database, then we will insert
        ** a dummy one, and return the pointer to it.
        */
        strcpy( imageRec.sopInstanceUID, A_sopInstanceUID );
        strcpy( imageRec.sopClassUID, "" );
        if ( LLInsert( A_perfProcStepRec->imageList,
                       &imageRec ) != SUCCESS )
        {
            /*
            ** Failure
            */
        }

        /*
        ** Now, we need a pointer to the node that we've just inserted.
        */
        LLRewind( A_perfProcStepRec->imageList );
        while( ( node = LLPopNode( A_perfProcStepRec->imageList,
                                   &imageRec ) ) != NULL ) 
        {
            if ( strcmp( imageRec.sopInstanceUID,
                         A_sopInstanceUID ) == 0 )
            {
                /*
                ** We've found the sop instance that they've given to us.
                */
                found = TRUE;
                break;
            }
        } /* end of while */
    }
    return( node );
}


/*****************************************************************************
**
** NAME
**     CheckIfComplete
**
** SYNOPSIS
**     static RESP_STATUS CheckIfComplete
**
** ARGUMENTS
**     int                A_associationID            - association ID
**     int                A_messageID                - request msg ID
**     PERF_PROC_STEP_REC *A_modPerfProcRec          - pointer to a performed
**                                                     procedure step record
**     char               *A_performedProcStepStatus - the status
**
** DESCRIPTION
**    This function attempts to validate whether or not a given MPPS record
**    can be updated to a status of COMPLETED or DISCONTINUED.  All of the
**    rules that this function implements are specified by DICOM.
**
** SEE ALSO
**
*****************************************************************************/
static RESP_STATUS
CheckIfComplete(
                   int                A_associationID,
                   int                A_messageID,
                   PERF_PROC_STEP_REC *A_modPerfProcRec,
                   char               *A_performedProcStepStatus
               )
               
{
    MC_STATUS status;
    int       databaseComplete = FALSE;
    int       statusFound = FALSE;
    int       nodesInDatabase = 0;
    int       perfSeriesSeqID;
    int       imageSeqID;

    /*
    ** At this point, we should check to see what status is set within the
    ** database.  If it is set to COMPLETED or DISCONTINUED, then this
    ** operation is quite short because we can't touch it anymore!  Tell the
    ** SCU that it is attempting to N_SET a completed or discontinued record.
    */
    if ( ( strcmp( A_modPerfProcRec->performedProcStepStatus,
                   "COMPLETED" ) == 0 ) ||
         ( strcmp( A_modPerfProcRec->performedProcStepStatus,
                   "DISCONTINUED" ) == 0 ) )
    {
        databaseComplete = TRUE;
        return( N_SET_PROCESSING_FAILURE );
    }

    /*
    ** Then, we attempt to get the status attribute.  Since the status attribute
    ** is actually type 3 in the request, it may not be contained within the
    ** request.  We must check it because it may be set to COMPLETED or
    ** DISCONTINUED.  We also should have checked the status field obtained
    ** from our database to make sure that it isn't already completed or
    ** discontinued already, above.
    */
    status = MC_Get_Value_To_String( A_messageID,
                                     MC_ATT_PERFORMED_PROCEDURE_STEP_STATUS,
                                     CS_LEN,
                                     A_performedProcStepStatus );
    switch( status )
    {
        case MC_INVALID_TAG:
        case MC_EMPTY_VALUE:
        case MC_NULL_VALUE:
            statusFound = FALSE;
            break;
        case MC_NORMAL_COMPLETION:
            statusFound = TRUE;
            break;
        default:
            LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                      "Failed to obtain the performed procedure step status "
                      "attribute from the N_SET request message.\n" );
            break;
    } /* end of switch on status */

    if ( statusFound == TRUE )
    {
        if ( G_logit )
            printf( "  MPPS N_SET status:  %s\n",
                    A_performedProcStepStatus );
        /*
        ** Should the SCU have sent us a status, we need to check it.  If it
        ** is a "COMPLETED" or a "DISCONTINUED", we also need to do some other
        ** parsing...
        */
        if ( ( strcmp( A_performedProcStepStatus, "COMPLETED" ) == 0 ) ||
             ( strcmp( A_performedProcStepStatus, "DISCONTINUED" ) == 0 ) )
        {
            /*
            ** Since they've sent us a completed or discontinued status, if
            ** the database's status is NOT completed or discontinued, then
            ** the user had better send us a performed series sequence, OR
            ** a performed series list is already contained within the database
            */
            if ( LLNodes( A_modPerfProcRec->performedSeriesList ) > 0 )
            {
                /*
                ** We've found a performed series list in the database.  This
                ** is good because the SCU wants to complete or discontinue
                ** the MPPS.
                */
                nodesInDatabase = TRUE;
            }

            /*
            ** Look for the performed series sequence.  We will look to see
            ** if nodesInDatabase is FALSE and they didn't send us this seq
            ** in just a few moments...
            */
            status = MC_Get_Value_To_Int( A_messageID,
                                          MC_ATT_PERFORMED_SERIES_SEQUENCE,
                                          &perfSeriesSeqID );
            switch( status )
            {
                case MC_NORMAL_COMPLETION:
                    break;
                case MC_NULL_VALUE:
                case MC_INVALID_TAG:
                    if ( nodesInDatabase == FALSE )
                    {
                        /*
                        ** If they didn't send us a performed series
                        ** sequence, and the database doesn't already
                        ** contain one, then we must not allow this
                        ** N_SET to continue.
                        */
                        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__,
                                  status,
                                  "MPPS SCU attempted to send a COMPLETED "
                                  "or DISCONTINUED status, and a "
                                  "performed\n"
                                  "series sequence item doesn't exist "
                                  "either in the N_SET message,\nnor the "
                                  "database.\n" );
                        return( N_SET_MISSING_ATTRIBUTE_VALUE );
                    }
                    else
                    {
                        /*
                        ** Since the database already contains a
                        ** performed series list and they didn't give us
                        ** one, we can allow this to continue.
                        */
                        break;
                    } /* end of nodesInDatabase */
                default:
                    LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__, status,
                              "Failed to obtain performed series sequence "
                              "from N_SET request message.\n" );
                    return( N_SET_INVALID_ATTRIBUTE_VALUE );
            } /* end of switch */

            /*
            ** If they gave us a performed series and the status is
            ** being set to completed, then we need to see if there exists
            ** an image sequence in the database, or in the request message.
            */
            if ( ( status == MC_NORMAL_COMPLETION ) &&
                 ( ( strcmp( A_performedProcStepStatus, "COMPLETED" ) == 0 ) ||
                   ( strcmp( A_performedProcStepStatus,
                             "DISCONTINUED" ) == 0 ) ) )
            {
                /*
                ** Look for nodes in the database.
                */
                nodesInDatabase = FALSE;
                if ( LLNodes( A_modPerfProcRec->imageList ) > 0 )
                {
                    /*
                    ** We've found an image list in the database.  This
                    ** is good because the SCU wants to complete or discontinue
                    ** the MPPS.
                    */
                    nodesInDatabase = TRUE;
                }
                
                if ( nodesInDatabase == FALSE )
                {
                    /*
                    ** Then, check to see if the referenced image sequence 
                    ** exists in the request message.
                    */
                    status = MC_Get_Value_To_Int( perfSeriesSeqID,
                                               MC_ATT_REFERENCED_IMAGE_SEQUENCE,
                                                  &imageSeqID );
                    if ( status != MC_NORMAL_COMPLETION )
                    {
                        LogError( ERROR_LVL_1, "ProcessNSETRQ", __LINE__,
                                  status,
                                  "N_SET request message contains a status of "
                                  "either COMPLETE or DISCONTINUED, while "
                                  "database does not\ncontain an image "
                                  "record, nor does the N_SET request contain "
                                  "a referenced image sequence.\n"
                                  "The N_SET will be rejected.\n" );
                        return( N_SET_MISSING_ATTRIBUTE_VALUE );
                    } /* end of if status was NOT normal completion */
                } /* end of if there were nodes in the database */	
            } /* end of if:  status was completed or discontinued */
        } /* end of if:  status was containted within the N_SET */
    } /* end of if:  statusFound was TRUE */

    return( N_SET_SUCCESS );
}

/*****************************************************************************
**
** NAME
**    RetrieveAndSetValue
**
** SYNOPSIS
**    static int RetrieveAndSetValue
**
** ARGUMENTS
**    int            A_sourceID        - the id of a request message
**    int            A_destID          - the id of a response message
**    unsigned long  A_tag             - the tag's value
**    RETURN_TYPE    A_sourceType      - the type of the request message
**    RETURN_TYPE    A_destType        - the type of the response message
**    char           *A_databaseStr    - a pointer to the data to modify
**    size_t         A_databaseStrSize - the size of the modified data
**    char           *A_functionName   - a string denoting which function
**                                       has called this one
**    int            A_sendBack        - TRUE if A_destID should ALWAYS
**                                       contain a value for a given tag
**                                       FALSE will only return a value for
**                                       a tag if it is contained within the
**                                       request message
**
** DESCRIPTION
**   This function will perform updates on the string that is passed into
**   this function based on the request type, return type, and tag that
**   is being requested.  This function is used by the NSET, and NCREATE
**   functions to actually perform the building of the response message and
**   the actual database update.
**
** RETURNS
**   SUCCESS or FAILURE
**
** SEE ALSO
**
*****************************************************************************/
static int
RetrieveAndSetValue(
                        int            A_sourceID,
                        int            A_destID,
                        unsigned long  A_tag,
                        RETURN_TYPE    A_sourceType,
                        RETURN_TYPE    A_destType,
                        char           *A_databaseStr,
                        size_t         A_databaseStrSize,
                        char           *A_functionName,
                        int            A_sendBack
                    )
{
    char   sourceValue[120];
    char   tagDescription[60];
    int    sourceValueFlag = 0;
    MC_STATUS status;

    /*
     * First get the value, and see if it fulfills the source type requirements
     */
    status = MC_Get_Value_To_String( A_sourceID,
                                     A_tag,
                                     sizeof(sourceValue),
                                     sourceValue );
    switch ( status )
    {
        case MC_NORMAL_COMPLETION:
            sourceValueFlag = 1;
            break;
        case MC_NULL_VALUE:
            if ( A_sourceType == TYPE_1 )
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_1, A_functionName, __LINE__, status,
                          "Required tag (%s) has NULL value.\n",
                          tagDescription ); 
                return FAILURE;
            }
            strcpy( sourceValue, "" );
            sourceValueFlag = 1;
            break;
        case MC_INVALID_TAG:
        case MC_EMPTY_VALUE:
            if (A_sourceType == TYPE_1)
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_1, A_functionName, __LINE__, status,
                          "Required tag (%s) not in source message.\n",
                          tagDescription ); 
                return FAILURE;
            }
            else if (A_sourceType == TYPE_2)
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_2, A_functionName, __LINE__, status,
                          "Type 2 tag (%s) not in source message "
                          "(should be NULL).\n",
                          tagDescription ); 
                /*
                ** Even though it's not there, assume it should have been NULL
                */
                strcpy( sourceValue, "" );
                sourceValueFlag = 1;
            }
            break;
        default:
            MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
            if (A_sourceType == TYPE_1)
            {
                LogError( ERROR_LVL_1, A_functionName, __LINE__, status, 
                          "Internal error getting tag (%s) in "
                          "source message.\n",
                          tagDescription ); 
                return FAILURE;
            }
            else
                LogError( ERROR_LVL_2, A_functionName, __LINE__, status,
                          "Internal error getting tag (%s) in "
                          "source message.\n",
                          tagDescription ); 
    } /* end of switch */
    
    /*
     * Now we've either retrieved a value or not.  If we have a value, copy
     * it into the database entry.
     */
    if ( sourceValueFlag )
    {
        if ( strlen(sourceValue)+1 > A_databaseStrSize )
        {
            MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
            LogError( ERROR_LVL_2, A_functionName, __LINE__, 
                      MC_NORMAL_COMPLETION,
                      "Internal error with tag (%s):  "
                      "Database buffer too small.\n",
                      tagDescription ); 
            strncpy( A_databaseStr, sourceValue, A_databaseStrSize-1 );
        }
        strcpy( A_databaseStr, sourceValue );
    }

    /*
     * Now, let's set the response message.
     */
    if ( strlen(A_databaseStr) == 0 )
    {
        if (A_destType == TYPE_1)
        {
            MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
            LogError( ERROR_LVL_1, A_functionName, __LINE__, 
                      MC_NORMAL_COMPLETION, 
                      "No value for type 1 tag (%s).\n", tagDescription ); 
                      
            return FAILURE;
        }
        else if (A_destType == TYPE_2)
        {
            status = MC_Set_Value_To_NULL( A_destID, A_tag );
            if ( status != MC_NORMAL_COMPLETION )
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_1, A_functionName, __LINE__, status,
                          "Unable to set type 1 tag (%s) to NULL.\n",
                          tagDescription ); 
                return( FAILURE );
            }
        }
      
        /*
         * We don't have a value, and it's type 3, so don't return anything!
         * UNLESS we want to anyway!
         */
        if ( A_sendBack == TRUE )
        {
            status = MC_Set_Value_To_NULL( A_destID, A_tag );
            if ( status != MC_NORMAL_COMPLETION )
            {
                MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
                LogError( ERROR_LVL_1, A_functionName, __LINE__, status,
                          "Unable to set type 1 tag (%s) to NULL.\n",
                          tagDescription ); 
                return( FAILURE );
            }
        }
        return SUCCESS;
    }
    
    /*
     * Now, simply set the value in the response message.
     */
    status = MC_Set_Value_From_String( A_destID,
                                       A_tag,
                                       A_databaseStr );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
        LogError( ERROR_LVL_1, A_functionName, __LINE__, status,
                  "Unable to set type tag (%s) in destination message.\n",
                  tagDescription ); 
        return( FAILURE );
    }

    return SUCCESS;
}


/*****************************************************************************
**
** NAME
**    GetOptions - Get the command line options.
**
** SYNOPSIS
**    static MC_STATUS GetOptions ( int argc, char **argv )
**
** ARGUMENTS
**    argc          int            Number of command line arguments
**    argv          char **        The command line.
**
** DESCRIPTION
**    GetOptions is the routine that parses the command line and sets the
**    global variables associated with each parameter.
**
** SEE ALSO
**
*****************************************************************************/
static void
GetOptions ( int argc, char **argv )
{
    int          ai, ci;
    int          iError = FALSE;
    FILE         *test;

    for ( ai = 1; ai < argc && strcmp ( argv[ai], "--" ); ai ++ )
    {
        if ( argv[ai][0] == '-' )
        {
            for ( ci = 1; ci < (int)strlen ( argv[ai] ); ci++ )
            {
                switch ( argv[ai][ci] )
                {
                    case 'a':        /* Application Title */
                    case 'A':        /* Application Title */
                        if ( argv[ai][ci+1] != '\0' )
                        {
                            iError = TRUE;
                        }
                        else if ( (ai + 1) == argc )
                        {
                            iError = TRUE;
                        }
                        else
                        {
                            strcpy ( G_app_title, &(argv[ai+1][0]) );
                            printf( "'-a' received.  Setting local "
                                    "application title to:  '%s'.\n",
                                    G_app_title );
                            ai++;
                            ci = strlen ( argv[ai] );
                        }
                        break;

                    case 't':        /* Timeout */
                    case 'T':        /* Timeout */
                        if ( argv[ai][ci+1] != '\0' )
                        {
                            iError = TRUE;
                        }
                        else if ( (ai + 1) == argc )
                        {
                            iError = TRUE;
                        }
                        else
                        {
                            G_timeout = strtoul( &(argv[ai+1][0]),
                                                      NULL, 10 );
                            printf( "'-t' received.  Setting network "
                                    "timeout to:  '%d'.\n",
                                    G_timeout );
                            ai++;
                            ci = strlen ( argv[ai] );
                        }
                        break;

                    case 'h':        /* Help */
                    case 'H':        /* Help */
                        printf ( "\n" );
                        printf ( "Modality Worklist Service Class Provider\n" );
                        printf ( "\n" );
                        printf ( "Usage: %s [options] \n", argv[0] );
                        printf ( "\n" );
                        printf ( "Options:\n" );
                        printf ( "   -h                 "
                                 "Print this help page\n" );
                        printf ( "   -a <ae_title>      "
                                 "Local application entity\n" );
                        printf ( "   -d <datafile name> "
                                 "Datafile Name\n" );
                        printf ( "   -l                 "
                                 "Turn on search logging\n" );
                        printf ( "   -t <timeout value> "
                                 "Network Timeout Value\n" );
                        printf ( "   -u                 "
                                 "Turn on database logging\n" );
                        printf ( "\n" );
                        exit ( SUCCESS );
                        break;

                    case 'd': /* Datafile Name */
                    case 'D': /* Datafile Name */
                        if ( argv[ai][ci+1] != '\0' )
                        {
                            iError = TRUE;
                        }
                        else if ( (ai + 1) == argc )
                        {
                            iError = TRUE;
                        }
                        else
                        {
                            strcpy ( G_data_file, &(argv[ai+1][0]) );
                            ai++;
                            ci = strlen ( argv[ai] );
                            printf( "'-d' received.  Using datafile: "
                                    "'%s'.\n", G_data_file );

                            /*
                            ** Sanity says that we should atleast make sure
                            ** that we can find the user's file...
                            */
                            test = fopen( G_data_file, "r" );
                            if ( test == NULL )
                            {
                                printf( "Unable to open datafile '%s'.\n", 
                                         G_data_file );
                                exit( FAILURE );
                            }
                            fclose( test );
                        }
                        break;

                    case 'l':        /* Log search criteria */
                    case 'L':        /* Log search criteria */
                        G_logit = TRUE;
                        printf( "'-l' received.  Turning on "
                                "search logging.\n" );
                        break;

                    case 'u':        /* database dUmp logging */
                    case 'U':         /* database dUmp logging */
                        G_logtree = TRUE;
                        printf( "'-u' received.  Turning on "
                                "database dump log.\n" );

                    default:
                        break;

                } /* switch argv */

                if ( iError == TRUE )
                {
                    printf ( "Usage: %s [options] \n", argv[0] );
                    printf ( "       %s -h\n", argv[0] );
                    exit ( FAILURE );
                }

            } /* for ci */

        }
        else
        {
           printf( "Invalid argument\n" );
           printf ( "Usage: %s [options] \n", argv[0] );
           printf ( "       %s -h\n", argv[0] );
           exit( FAILURE );
        }

    } /* for ai */

}

/*****************************************************************************
**
** NAME
**    CompareDateStrings - Compares two date strings for a match
**
** SYNOPSIS
**    static int CompareDateStrings( char *baseDate, char *tokenDate )
**
** ARGUMENTS
**    baseDate  - The date from the database file.
**    tokenDate - The date that the user entered, for comparison.
**
** DESCRIPTION
**    baseDate must be of the form:  YYYYMMDD
**    tokenDate must be either:
**                 YYYYMMDD   =  A single date
**      YYYYMMDD - YYYYMMDD   =  A range of dates
**               YYYYMMDD -   =  A single date, where date one should come
**                               on date two, or after date two.
**               - YYYYMMDD   =  A single date, where date one should come
**                               on date two, or before date two.
**
**    Basically, date one is the date for which we want to compare against.
**    Date two can contain a single date, a range of dates, or a date where
**    the compared date must be either less than or greater than (inclusive)
**    the comparison date.
**
** RETURNS
**    TRUE -  When the baseDate is equal to the tokenDate,
**            baseDate is within the date range of tokenDate (inclusive),
**            baseDate comes after tokenDate (inclusive), or
**            baseDate comes before tokenDate (inclusive).
**    FALSE - otherwise.
**
** SEE ALSO
**
*****************************************************************************/
static int
CompareDateStrings(
                     char *A_baseDate,
                     char *A_tokenDate
                  )
{
    char one[9];
    char two[9];

    unsigned long lowDate;
    unsigned long highDate;
    unsigned long compDate;

    one[8]='\0'; /* Terminate the working strings */
    two[8]='\0'; /* Terminate the working strings */

    /*
    ** First, we see if the tokenDate, contains the separator
    ** token "-".  If it does, then we need to check date ranges.
    ** If not, then we just compare the two dates and return.
    */
    if ( strstr( A_tokenDate, "-" ) == NULL )
    {
        if ( strcmp( A_baseDate, A_tokenDate ) == 0 )
        {
            return( TRUE );
        }
        else
        {
            return( FALSE );
        }
    }

    /*
    ** OK, we actually have a token-separated string, so we need to parse
    ** the two parts of it.
    */
    SplitDate( A_tokenDate, one, two );

    /*
    ** Convert the date components into unsigned longs for comparison.
    ** It is contended that integer comparisons are faster than string ones...
    ** Besides, this does the possible whitespace trimming for us.
    */
    compDate = strtoul( A_baseDate, NULL, 10 );
    if ( errno == ERANGE )
    {
        printf( "Cannot support this date:  %s\n", A_baseDate );
        return( FALSE );
    }

    lowDate = strtoul( one, NULL, 10 );
    if ( errno == ERANGE )
    {
        printf( "Cannot support this date:  %s\n", one );
        return( FALSE );
    }

    highDate = strtoul( two, NULL, 10 );
    if ( errno == ERANGE )
    {
        printf( "Cannot support this date:  %s\n", two );
        return( FALSE );
    }

    /*
    ** Now, we have three unsigned long integers that hold our date
    ** components:  compDate, lowDate, and highDate.  We also have several
    ** possibilities for the comparison:
    ** 1)  If lowDate is 0, then we are looking to see if compDate is <=
    **     to highDate.
    ** 2)  If highDate is 0, then we are looking to see if compDate is >=
    **     to lowDate.
    ** 3)  If neither are 0, then we want to see if
    **     lowDate <= compDate <= highDate.
    ** 4)  If they are both 0, something went wrong.
    **
    */
    if ( ( lowDate == 0 ) && ( highDate == 0 ) )
    {
        /*
        ** This is the 'bad' case.  This is an error.
        */
        return( FALSE );
    }
    else if ( ( lowDate == 0 ) && ( compDate <= highDate ) )
    {
        return( TRUE );
    }
    else if ( ( highDate == 0 ) && ( compDate >= lowDate ) )
    {
        return( TRUE );
    }
    else if ( ( lowDate <= compDate ) && ( compDate <= highDate ) )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}

/*****************************************************************************
**
** NAME
**    SplitDate - Separate a date string.
**                NOTE:  This function also handles the separation of
**                       time strings into two pieces as well.
**
** SYNOPSIS
**    Given a date string, that contains (upto) two dates separated
**    by a "-" token character, SplitDate will parse out the passed
**    in string into its two components.
**    The passed in date string MUST be of either of the three forms:
**        YYYYMMDD-YYYYMMDD
**        -YYYYMMDD
**        YYYYMMDD-
**    Since this function also handles time string splitting, times are
**    in the form of:
**        HHMMSS-HHMMSS
**        -HHMMSS
**        HHMMS-
**
** ARGUMENTS
**    Adate - The date string that must, at least contain a "-" character.
**    Aone  - a pointer to a date (if any) to the left of the "-" token.
**            for return, only.
**    Atwo  - a pointer to a date (if any) to the right of the "-" token.
**            for return, only.
**
** RETURNS
**    Aone  - The date (if any) to the left of the "-" token.
**    Atwo  - The date (if any) to the right of the "-" token.
**
*****************************************************************************/
static void
SplitDate( 
           char *A_date,
           char *A_one,
           char *A_two
         )
{
    int  i = 0;
    int  leftTextFound = FALSE;
    int  tokenFound    = FALSE;
    char dupDate[80]; /* We need some arbitrary, big array */

    /*
    ** We don't want to mess with Adate's contents...
    */
    strcpy( dupDate, A_date );
    /*
    ** We need to walk the date string, searching for the "-" separator
    ** token.  If we find only whitespace to the left of it, we know that
    ** we are looking for dates less than or equal to the following date
    ** string.  If we find a date, followed by the separator, followed by
    ** some possible whitespace, then we have a date that should be
    ** greater than, or equal to the given date string.
    */
    for( i=0; (unsigned)i <= strlen( dupDate ); i++ )
    {
#ifdef VXWORKS
        if( (isdigit)( dupDate[i] ) && ( tokenFound == FALSE ) )
#else
        if( isdigit( dupDate[i] ) && ( tokenFound == FALSE ) )
#endif
        {
            /*
            ** In this case, we've found a part of a date, but haven't
            ** found the range token.  This way, we can parse the string
            ** directly with strtok.
            */
            leftTextFound = TRUE;
            i = strlen( dupDate);
            strcpy( A_one, strtok( dupDate, "-" ));
            strcpy( A_two, strtok( NULL, "-" ));
        }
        else if ( dupDate[i] == '-' )
        {
            /*
            ** In this case, we've found the range token, but  haven't found
            ** any date to the left of it.  In this case, we reverse our
            ** usage of strtok.
            */
            tokenFound = TRUE;
            i = strlen( dupDate );
            strcpy( A_two, strtok( dupDate, "-" ));
            strcpy( A_one, strtok( NULL, "-" ));
        }
    }
}


/*****************************************************************************
**
** NAME
**    CompareTimeStrings - Compares two time strings for a match
**
** SYNOPSIS
**    static int CompareTimeStrings( char *baseTime, char *tokenTime )
**
** ARGUMENTS
**    baseTime  - The Time from the database file.
**    tokenTime - The Time that the user entered, for comparison.
**
** DESCRIPTION
**    baseTime must be of the form:  HHMMSS
**    tokenTime must be either:
**                 HHMMSS   =  A single Time
**      HHMMSS - HHMMSS     =  A range of Times
**               HHMMSS -   =  A single Time, where Time one should come
**                             on Time two, or after Time two.
**               - HHMMSS   =  A single Time, where Time one should come
**                               on Time two, or before Time two.
**
**    Basically, Time one is the Time for which we want to compare against.
**    Time two can contain a single Time, a range of Times, or a Time where
**    the compared Time must be either less than or greater than (inclusive)
**    the comparison Time.
**
** RETURNS
**    TRUE -  When the baseTime is equal to the tokenTime,
**            baseTime is within the Time range of tokenTime (inclusive),
**            baseTime comes after tokenTime (inclusive), or
**            baseTime comes before tokenTime (inclusive).
**    FALSE - otherwise.
**
** SEE ALSO
**    CompareDateStrings, SplitDate
**
*****************************************************************************/
static int
CompareTimeStrings(
                     char *A_baseTime,
                     char *A_tokenTime
                  )
{
    char one[9];
    char two[9];

    unsigned long lowTime;
    unsigned long highTime;
    unsigned long compTime;

    one[8]='\0'; /* Terminate the working strings */
    two[8]='\0'; /* Terminate the working strings */

    /*
    ** Then, we see if the tokenTime, contains the separator
    ** token "-".  If it does, then we need to check time ranges.
    ** If not, then we just compare the two times and return.
    */
    if ( strstr( A_tokenTime, "-" ) == NULL )
    {
        if ( strcmp( A_baseTime, A_tokenTime ) == 0 )
        {
            return( TRUE );
        }
        else
        {
            return( FALSE );
        }
    }

    /*
    ** OK, we actually have a token-separated string, so we need to parse
    ** the two parts of it.
    ** NOTE:  SplitDate also splits times as well (if fact, it will actually
    ** split any "-" separated string into two components).
    */
    SplitDate( A_tokenTime, one, two );

    /*
    ** Convert the time components into unsigned longs for comparison.
    ** It is contended that integer comparisons are faster than string ones...
    ** Besides, this does the possible whitespace trimming for us.
    */
    compTime = strtoul( A_baseTime, NULL, 10 );
    if ( errno == ERANGE )
    {
        printf( "Cannot support this time:  %s\n", A_baseTime );
        return( FALSE );
    }

    lowTime = strtoul( one, NULL, 10 );
    if ( errno == ERANGE )
    {
        printf( "Cannot support this time:  %s\n", one );
        return( FALSE );
    }

    highTime = strtoul( two, NULL, 10 );
    if ( errno == ERANGE )
    {
        printf( "Cannot support this time:  %s\n", two );
        return( FALSE );
    }

    /*
    ** Now, we have three unsigned long integers that hold our time
    ** components:  compTime, lowTime, and highTime.  We also have several
    ** possibilities for the comparison:
    ** 1)  If lowTime is 0, then we are looking to see if compTime is <=
    **     to highTime.
    ** 2)  If highTime is 0, then we are looking to see if compTime is >=
    **     to lowTime.
    ** 3)  If neither are 0, then we want to see if
    **     lowTime <= compTime <= highTime.
    ** 4)  If they are both 0, something went wrong.
    **
    */
    if ( ( lowTime == 0 ) && ( highTime == 0 ) )
    {
        /*
        ** This is the 'bad' case.  This is an error.
        */
        return( FALSE );
    }
    else if ( ( lowTime == 0 ) && ( compTime <= highTime ) )
    {
        return( TRUE );
    }
    else if ( ( highTime == 0 ) && ( compTime >= lowTime ) )
    {
        return( TRUE );
    }
    else if ( ( lowTime <= compTime ) && ( compTime <= highTime ) )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}

#if defined(_MACINTOSH) && defined(__MWERKS__)
/*****************************************************************************
**
** NAME
**    HandleMacQuitEvents - Check the event queue, looking for events.
**
** SYNOPSIS
**    static int HandleMacQuitEvents()
**
** DESCRIPTION
**    Scans the event queue.  If there is a menu or keyboard event pending,
**    then attempt to process it.  If there is no event ready, then allow
**    the SIOUX windowing to handle events.
**
*****************************************************************************/


static int
HandleMacQuitEvents()
{
    EventRecord    asEvent;
    Boolean        aqQuit = false;

    /*
    ** We look for mouse down, mouse up, key down, key up, or auto
    ** key mask events.
    */
    if ( WaitNextEvent( mDownMask | mUpMask | keyDownMask |
                        keyUpMask | autoKeyMask,
                        &asEvent, 10, NULL ) ) 
    {
        if (asEvent.what == keyDown) 
        {
            /*
            ** The user pressed a key.  Set up some masks to see
            ** which key(s) were pressed, or if any modifier keys
            ** were pressed.
            */
            unsigned char  abChar = asEvent.message & charCodeMask;
            Boolean        aqCmnd = ( ( asEvent.modifiers & cmdKey)
                                                 != 0 );

            /*
            ** If the user selected "q" or "COMMAND-.", then
            ** they must want to quit this app.
            */
            if ( ( abChar == 'Q' ) || ( abChar == 'q' ) ||
                 ( aqCmnd && ( abChar == '.' ) ) )
            {
                aqQuit = true;
            }
        } 
        else if ( asEvent.what == mouseDown ) 
        {
            /*
            ** If a key wasn't pressed, then is the mouse
            ** button down?
            */
            WindowPtr  apWindow;
            short      awWhere;

            /*
            ** Since the mouse was pressed, which window was it
            ** pressed in?
            */
            awWhere = FindWindow( asEvent.where, &apWindow );
            if ( awWhere == inMenuBar ) 
            {
                /*
                ** If they pressed the mouse in the application's
                ** menu bar, then select that menu, and hilight
                ** that menu.
                */
                MenuSelect( asEvent.where );
                HiliteMenu( 0 );
            } 
            else if ( awWhere == inSysWindow )
            {
                /*
                ** If the user pressed the mouse down in a Sys
                ** window, then perform a system click event.
                */
                SystemClick( &asEvent, apWindow );
            }
        } /* Was it a mouse-down event? */
    } /* Events avaliable? */
    else
    {
        /*
        ** If there aren't any events in the event queue, then allow
        ** the SIOUX window to process events.
        */
        SIOUXHandleOneEvent(&asEvent);
    }

    /*
    ** If the user wants to quit the program, then allow the "infinite"
    ** loop to be exitted.
    */
    if (aqQuit)
    {
        return( TRUE );
    }

    return( FALSE );
}
#endif
