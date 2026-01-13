/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Tool Kit
 *
 *    $Workfile: stor_scp.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is a sample service class provider application
 *               for the Storage Service Class
 *
 *************************************************************************
 *
 *                      COPYRIGHT (C) 1994-2004
 *		  Merge Technologies Incorporated (d/b/a Merge eFilm)
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


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <conio.h>
#include <time.h>
#include <signal.h>
#include <process.h>
#include <sys/timeb.h>


#include "mergecom.h"
#include "mc3msg.h"
#include "mc3media.h"
#include "diction.h"



/*
 *  Local definitions
 */
#define AE_LENGTH 16
#define UI_LENGTH 64

#define MAX_THREAD_SERVERS 5
#define THREAD_STACK_SIZE  16384

#define BINARY_READ "rb"
#define BINARY_WRITE "wb"
#define BINARY_APPEND "rb+"
#define BINARY_READ_APPEND "a+b"
#define BINARY_CREATE "w+b"
#define TEXT_READ "r"
#define TEXT_WRITE "w"



/*
 * Boolean used to handle return values from functions
 */
typedef enum
{
    SAMP_SUCCESS = 1,
    SAMP_FAILURE = 0
} SAMP_STATUS;

typedef enum
{
    SAMP_TRUE = 1,
    SAMP_FALSE = 0
} SAMP_BOOLEAN;

/*
 * Structure to store local application information 
 */
typedef struct stor_scp_options
{
    int     AssociationID;
    int     ApplicationID;
    
    char    LocalAE[AE_LENGTH+2]; 
    int     ListenPort;

    TRANSFER_SYNTAX SaveSyntax;
    
    SAMP_BOOLEAN Verbose;
    
    SAMP_BOOLEAN ListMessages;
    SAMP_BOOLEAN SaveStream;
    SAMP_BOOLEAN SaveMedia;
    
} STORAGE_OPTIONS;


/*
 *  Structure passed to our callback functions
 */
typedef struct CALLBACKINFO
{        
   FILE*  fp;
   int    messageID;
   char*  serviceName;
   char   prefix[30];
} CBinfo;


/*
 * Structure to maintain list of instances sent
 */
typedef struct instance_node
{
    char   SOPClassUID[UI_LENGTH+2];
    char   SOPInstanceUID[UI_LENGTH+2];
    
    struct instance_node* Next;

} InstanceNode;


typedef struct transaction_node
{
    char  RemoteAE[AE_LENGTH+2];
    char  TransactionUID[UI_LENGTH+2];

    InstanceNode* InstanceList;

    struct transaction_node* Next;
} TransactionNode;



/*
 *  Module static variables
 */
CRITICAL_SECTION G_CriticalSection;
static int       G_threadCount;
static int       G_imageSaveNumber = 0;
STORAGE_OPTIONS  G_options;




/*
 *  Module function prototypes
 */
       int main(        int                 argc, 
                        char**              argv );

static void         PrintCmdLine( void );

static SAMP_STATUS  TestCmdLine( 
                        int                 A_argc,   
                        char*               A_argv[], 
                        STORAGE_OPTIONS*    A_options );

static SAMP_BOOLEAN PollQuitKey( void );

static void         Handle_Association(
                        void*               A_arg );

static SAMP_STATUS  HandleStorageCommitMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID, 
                        int                 A_messageID,
                        char*               A_transactionUID,
                        TransactionNode**   A_list );

static SAMP_STATUS  ParseStorageCommitMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_messageID,
                        char*               A_remoteAE,
                        TransactionNode**   A_node );

static SAMP_STATUS SendStorageCommitNEvent(
                        STORAGE_OPTIONS*    A_options,
                        TransactionNode**   A_list );

static SAMP_STATUS SetAndSendNEventMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID,
                        TransactionNode*    A_node );


static SAMP_STATUS  WriteToMessage(  
                        STORAGE_OPTIONS*    A_options,
                        char*               A_filename,
                        int*                A_msgID,
                        char*               A_messageType );

static SAMP_STATUS  WriteToMedia(
                        STORAGE_OPTIONS*    A_options,
                        char*               A_filename,
                        int*                A_msgID,
                        char*               A_messageType );

static SAMP_STATUS  AddGroup2Elements(
                        STORAGE_OPTIONS*    A_options,
                        TRANSFER_SYNTAX     A_transSyntax, 
                        int                 A_fileID );

static MC_STATUS    FileObjToMedia(
                        char*               A_filename,
                        void*               A_userInfo,
                        int                 A_dataSize,
                        void*               A_dataBuffer,
                        int                 A_isFirst,
                        int                 A_isLast );

static MC_STATUS    MsgObjToFile(
                        int                 A_msgID,
                        void*               A_userInfo,
                        int                 A_dataSize,
                        void*               A_dataBuffer,
                        int                 A_isFirst,
                        int                 A_isLast );

static char*        GetSyntaxDescription(
                        TRANSFER_SYNTAX     A_syntax );

static void         PrintError(
                        char*               A_string, 
                        MC_STATUS           A_status );

static void         NTsigint_handler (
                        int                 A_signo );




/****************************************************************************
 *
 *  Function    :   main
 *
 *  Description :   Loops waiting for associations or quit key press
 *
 ****************************************************************************/
int main( int argc, char** argv )
{
    SAMP_STATUS     sampStatus;
    MC_STATUS       mcStatus;
    int             calledApplicationID;
    unsigned long   threads_id;
    static int      quota_message = 1;

   
    printf("stor_scp:\n");
    
    sampStatus = TestCmdLine(argc, argv, &G_options);
    if (sampStatus != SAMP_SUCCESS)
        return(EXIT_FAILURE);
   
    printf("\n");
    printf("\tPress 'Q' or Esc to cancel server and\n");
    printf("\tserver will stop when session ends.\n");

    /*
     * Initialize the critical section before any threads are created.
     * This insures that the critical section is ready before a thread
     * tries to use it.
     */
    InitializeCriticalSection ( &G_CriticalSection );


    /* ------------------------------------------------------- */ 
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */ 
    mcStatus = MC_Library_Initialization ( NULL, NULL, NULL ); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize library",mcStatus);
        return ( EXIT_FAILURE );
    }

    /*
     *  Register this DICOM application
     */
    mcStatus = MC_Register_Application(&G_options.ApplicationID, G_options.LocalAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to register \"MERGE_STORE_SCP\"",mcStatus);
        return(EXIT_FAILURE);
    }

    /*
     * If a listen port has been specified, set the port.  This must be
     * done before the first call to MC_Wait_For_Association is made.
     */ 
    if (G_options.ListenPort != -1)
    {
        mcStatus = MC_Set_Int_Config_Value( TCPIP_LISTEN_PORT,
                                            G_options.ListenPort );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to set listen port, defaulting",mcStatus);
        }
    }
    


    /*
     *  Loop, handling associations - waiting for quit keypress
     */
    for (;;)
    {      
        /*
         * Poll in a platform specific way if a quit key has ben pressed.
         */
        if (PollQuitKey())
            break;


        /*
         * A one second timeout has been chosen so the UI is responsive
         * to the quit keys, although it causes heavy system load.  In
         * most applications, this timeout should be increased.
         */
        mcStatus = MC_Wait_For_Association( "Storage_SCP_Service_List", 1,
                                            &calledApplicationID,
                                            &G_options.AssociationID);
        if (mcStatus == MC_TIMEOUT)
            continue;
        else if (mcStatus == MC_UNKNOWN_HOST_CONNECTED)
        {
            printf("\tUnknown host connected, association rejected \n");
            continue;
        }
        else if (mcStatus == MC_NEGOTIATION_ABORTED)
        {
            printf("\tAssociation aborted during negotiation \n");
            continue;
        }
        else if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Error on MC_Wait_For_Association",mcStatus);
            printf("\t\tProgram aborted.\n");
            break;
        }
      
        if (calledApplicationID != G_options.ApplicationID)
        {
            printf("\tUnexpected application identifier on MC_Wait_For_Association.\n");
            printf("\t\tProgram aborted.\n");
            break;
        }


        if ( G_threadCount > MAX_THREAD_SERVERS )
        {
            mcStatus = MC_Reject_Association(G_options.AssociationID,TRANSIENT_LOCAL_LIMIT_EXCEEDED); 
            if ( quota_message )
            {
                quota_message = 0;
                printf ( "Server Quota readched at least once\n" );
            }
        }
        else
        {
            /*
             * Start up a thread to handle the association
             */
            threads_id = _beginthread ( Handle_Association,
                                        (unsigned)THREAD_STACK_SIZE,
                                        (void *)G_options.AssociationID
                                      );
            switch ( threads_id )
            {
                case -1:
                    printf ( "\tUnable to begin server thread.\n" );
                    printf ( "\tProgram aborted.\n" );
                    exit ( EXIT_FAILURE );

                default:
                    G_threadCount++;
                    break;
            } 
        } 


    }   /* Loop till quit requested or error */

    mcStatus = MC_Release_Application(&G_options.ApplicationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Release_Application failed",mcStatus);
    }

    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");
   
    printf("End of program\n");
    return(EXIT_SUCCESS);
}   /* END main() */



/********************************************************************
 *
 *  Function    :   PrintCmdLine
 *
 *  Parameters  :   none
 *
 *  Returns     :   nothing
 *
 *  Description :   Prints program usage
 *
 ********************************************************************/
static void PrintCmdLine(void)
{
    printf("\nUsage stor_scp -a local_ae -p listen_port -l -f -m -t syntax -v\n");
    printf("\tlocal_ae    optional specify the local Application Title\n");
    printf("\t            (Default: MERGE_STORE_SCP)\n");
    printf("\tlisten_port optional specify the local listen port\n");
    printf("\t            (Default: found in the mergecom.pro file)\n");
    printf("\t-l          list contents of received messages to stdout\n");
    printf("\t-f          save contents of received messages in DICOM \"stream\" format\n");
    printf("\t-m          save contents of received messages in DICOM Part 10 format\n");
    printf("\tsyntax      specify the transfer syntax of received objects, where\n");
    printf("\t            syntax equals:\n");
    printf("\t              il for Implicit VR Little Endian\n");
    printf("\t              el for Explicit VR Little Endian\n");
    printf("\t              eb for Explicit VR Big Endian\n");
    printf("\t-v          execute in verbose mode, print detailed information\n");
    printf("\n\tImage files are stored in the current directory and named \n");
    printf("\t0.img, 1.img, 2.img, etc. for stream format\n");
} /* end PrintCmdLine() */



/*************************************************************************
 *
 *  Function    :   TestCmdLine
 *
 *  Parameters  :   Aargc   - Command line arguement count
 *                  Aargv   - Command line arguements
 *                  A_options - Local application options read in.
 *
 *  Return value:   SAMP_TRUE
 *                  SAMP_FAILURE
 *
 *  Description :   Test command line for valid arguements.  If problems
 *                  are found, display a message and return SAMP_FAILURE
 *
 *************************************************************************/
static SAMP_STATUS TestCmdLine(
        int                 A_argc,   
        char*               A_argv[], 
        STORAGE_OPTIONS*    A_options )
{
    int       i;
    int       argCount=0;
    
   
    /*
     * Set default values
     */
    strcpy(A_options->LocalAE, "MERGE_STORE_SCP");

    A_options->ListMessages = SAMP_FALSE;
    A_options->SaveStream = SAMP_FALSE;
    A_options->SaveMedia = SAMP_FALSE;
    
    A_options->ListenPort = -1;
    A_options->SaveSyntax = IMPLICIT_LITTLE_ENDIAN;
    
    A_options->Verbose = SAMP_FALSE;


    /*
     * Loop through each arguement and store configuration information 
     * in A_options
     */     
    for (i = 1; i < A_argc; i++)
    {
        if ( !strcmp(A_argv[i], "-h") || !strcmp(A_argv[i], "/h") ||
             !strcmp(A_argv[i], "-H") || !strcmp(A_argv[i], "/H") ||
             !strcmp(A_argv[i], "-?") || !strcmp(A_argv[i], "/?"))
        {
            PrintCmdLine();
            return SAMP_FAILURE; 
        }
        else if ( !strcmp(A_argv[i], "-a") || !strcmp(A_argv[i], "-A"))
        {
            /*
             * Set the Local AE
             */
            i++;
            strcpy(A_options->LocalAE, A_argv[i]);
        } 
        else if ( !strcmp(A_argv[i], "-p") || !strcmp(A_argv[i], "-P"))
        {
            /*
             * Local Port Number
             */
            i++;
            A_options->ListenPort = atoi(A_argv[i]);
        
        }
        else if ( !strcmp(A_argv[i], "-v") || !strcmp(A_argv[i], "-V"))
        {
            /*
             * Verbose mode
             */
            A_options->Verbose = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-l") || !strcmp(A_argv[i], "-L"))
        {
            /*
             * List messages
             */
            A_options->ListMessages = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-f") || !strcmp(A_argv[i], "-F"))
        {
            /*
             * Save streamed objects in the 
             */
            A_options->SaveStream = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-m") || !strcmp(A_argv[i], "-M"))
        {
            /*
             * Save received objects in the DICOM Part 10 format
             */
            A_options->SaveMedia = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-t") || !strcmp(A_argv[i], "-T"))
        {
            /*
             * A save transfer syntax has been specified
             */
            i++;
            A_options->SaveMedia = SAMP_TRUE;
            if ( !strcmp(A_argv[i], "il") || !strcmp(A_argv[i], "IL"))
                A_options->SaveSyntax = IMPLICIT_LITTLE_ENDIAN;
            else if ( !strcmp(A_argv[i], "el") || !strcmp(A_argv[i], "EL"))
                A_options->SaveSyntax = EXPLICIT_LITTLE_ENDIAN;
            else if ( !strcmp(A_argv[i], "eb") || !strcmp(A_argv[i], "EB"))
                A_options->SaveSyntax = EXPLICIT_BIG_ENDIAN;
            else
                printf("Ignoring unkown transfer syntax: %s\n",A_argv[i]);
        }
        else
        {
            printf("Ignoring unkown option: %s\n",A_argv[i]);
             
        }
    }

    if (A_options->Verbose)
    {
        if (A_options->ListMessages)
            printf("\tList received images.\n");
            
        if (A_options->SaveMedia)
        {
            printf("\tWrite received images to file in DICOM Part 10 (media) format.\n");
            printf("\tImage files will be named x.img, where \"x\" is a number.\n");
        }
        else if (A_options->SaveStream)
        {
            printf("\tWrite received images to file in DICOM \"stream\" format.\n");
            printf("\tImage files will be named x.img, where \"x\" is a number.\n");
        }
        else
        {
            printf("\t\"Bit bucket\" operation.\n");
        }
    }
    return SAMP_SUCCESS;
    
}/* TestCmdLine() */



/*************************************************************************
 *
 *  Function    :   PollQuitKey
 *
 *  Parameters  :   none
 *
 *  Return value:   SAMP_SUCCESS if quit key pressed
 *                  SAMP_FAILURE all other conditions
 *
 *  Description :   Platform specific code to test if a quit key has been
 *                  pressed.
 *
 *************************************************************************/
static SAMP_BOOLEAN PollQuitKey(void) 
{

#if defined(INTEL_BCC)
    if ( kbhit () )
#else
    if ( _kbhit () )
#endif
    {
        switch ( _getch () )
        {
            case    0:      /* Ignore function keys */
                _getch();
                break;
            
            case   'Q':    /* Quit */
            case   'q':
            case    27:
                return SAMP_TRUE;
        }
    }
      
    return SAMP_FALSE;
}



/****************************************************************************
 *
 *  Function    :    Handle_Association
 *
 *  Parameters  :    Aoptions - Structure that holds configuration parameters
 *
 *  Returns     :    nothing
 *
 *  Description :    Processes a received association requests
 *
 ****************************************************************************/
static void Handle_Association(void* A_arg)
{
    MC_STATUS   mcStatus;
    SAMP_STATUS sampStatus;
    RESP_STATUS respStatus = C_STORE_SUCCESS;
    time_t      startTime;
    time_t      imageStartTime;
    time_t      currentTime;
    char        fname[32];
    MC_COMMAND  command;
    int         assocID;
    int         msgID;
    int         rspMsgID;
    char*       serviceName;
    long        seconds;
    float       perSecond;
    char        prefix[30] = {0};
    AssocInfo   asscInfo;
    ServiceInfo servInfo;
    int         nimages = 0;
    TransactionNode* transactionList = NULL;


    /*
     * Now, copy the association ID out of the options structure and save
     * it in a local variable, then exit the critical section.
     */
    assocID = (int)A_arg;
     
     
    /*
     *  Need thread ID number for messages displayed
     */
    sprintf ( prefix, "TID(%lu)", GetCurrentThreadId () );


    /*
     * Each thread will handle shutdown
     */
    signal ( SIGINT, NTsigint_handler );
    signal ( SIGBREAK, NTsigint_handler );



    time (&startTime);
      
    mcStatus = MC_Get_Association_Info( assocID, &asscInfo); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Association_Info failed", mcStatus);
    }
    else
    {
        if (G_options.Verbose)
        {
            printf("Connection from Remote Application:\n");
            printf("   AE Title:                 %s\n", asscInfo.RemoteApplicationTitle);
            printf("   Host name:                %s\n", asscInfo.RemoteHostName);
            printf("   IP Address:               %s\n", asscInfo.RemoteIPAddress);
            printf("   Implementation Version:   %s\n", asscInfo.RemoteImplementationVersion);
            printf("   Implementation Class UID: %s\n", asscInfo.RemoteImplementationClassUID);
            printf("   Requested AE Title:       %s\n\n\n", asscInfo.LocalApplicationTitle);
        }
    }

    if (G_options.Verbose)
    {
        mcStatus = MC_Get_Association_Info( assocID, &asscInfo); 

        printf("Services and transfer syntaxes negotiated:\n");

        /*
         * This the services negotiated
         */
        mcStatus = MC_Get_First_Acceptable_Service(assocID,&servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
            printf("   %-30s: %s\n",servInfo.ServiceName, 
                                    GetSyntaxDescription(servInfo.SyntaxType));
                
            
            mcStatus = MC_Get_Next_Acceptable_Service(assocID,&servInfo);
        }

        /*
         * Catch error, but because it doesn't impact operation, don't do 
         * anything about it.
         */
        if (mcStatus != MC_END_OF_LIST)
            PrintError("Warning: Unable to get service info",mcStatus);
        
        printf("\n\n");
    }
    else
        printf("%s\n\tAssociation Received.\n\n", prefix);
        
 

    mcStatus = MC_Accept_Association(assocID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * Make sure the association is cleaned up.
         */
        MC_Reject_Association(assocID,
                              TRANSIENT_NO_REASON_GIVEN);
        PrintError("Error on MC_Accept_Association", mcStatus);

        EnterCriticalSection ( &G_CriticalSection );
        --G_threadCount;
        LeaveCriticalSection ( &G_CriticalSection );

        _endthread();
        return;
    }
   
    for (;;)
    {
        time (&imageStartTime);
        mcStatus = MC_Read_Message( assocID, 
                                    30, 
                                    &msgID,
                                    &serviceName, 
                                    &command);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            MC_Free_Message(&msgID);
            if (mcStatus == MC_ASSOCIATION_CLOSED)
            {       
                printf("%s\tAssociation Closed.\n", prefix);
                time (&currentTime);
                seconds = (long)(currentTime - startTime);
                if (nimages)
                {
                    perSecond = (float)seconds /(float)nimages;
                    printf ("%s\t\tElapsed time = %ld seconds - %d images, %.1f seconds per image\n",
                    prefix, seconds, nimages, perSecond);
                }
                else
                    printf ("%s\t\tNo images received\n", prefix);
                break;
            }
            else if (mcStatus == MC_NETWORK_SHUT_DOWN
                 ||  mcStatus == MC_ASSOCIATION_ABORTED
                 ||  mcStatus == MC_INVALID_MESSAGE_RECEIVED
                 ||  mcStatus == MC_CONFIG_INFO_ERROR)
            {
                /*
                 * In this case, the association has already been closed
                 * for us.
                 */
                PrintError("Unexpected event", mcStatus);
                break;
            }
            
            PrintError("Error on MC_Read_Message", mcStatus);
            MC_Abort_Association(&assocID);
            break;
        }
        
        if (!strcmp(serviceName, "STORAGE_COMMITMENT_PUSH"))
        {
            sampStatus = HandleStorageCommitMessage( &G_options,
                                        assocID, 
                                        msgID, 
                                        asscInfo.RemoteApplicationTitle,
                                        &transactionList );
            
            if (G_options.ListMessages)
            {
                MC_List_Message( msgID, NULL );
            }
            
            mcStatus = MC_Free_Message(&msgID);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message for request message error", mcStatus);
                MC_Abort_Association(&assocID);
                break;
            }
            
            continue;
        }
      
        /*
         * Set the number of images and the status to success
         */
        nimages++;
        respStatus = C_STORE_SUCCESS;       
        
        /*
         * First, if list message has been specified, list the objects.
         */
        if (G_options.ListMessages)
        {
            MC_List_Message(msgID, NULL);
        }
      
        if (G_options.SaveMedia)
        {
            EnterCriticalSection ( &G_CriticalSection );
            sprintf(fname, "%d.img", G_imageSaveNumber++);
            LeaveCriticalSection ( &G_CriticalSection );
            
            /* 
             * The message object will be free'd by WriteToFile
             */
            sampStatus = WriteToMedia( &G_options,
                                       fname,
                                       &msgID,
                                       serviceName );
            if (sampStatus == SAMP_SUCCESS)
                respStatus = C_STORE_SUCCESS;       
            else
                respStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
            
        }
        else if (G_options.SaveStream)
        {
            EnterCriticalSection ( &G_CriticalSection );
            sprintf(fname, "%d.img", G_imageSaveNumber++);
            LeaveCriticalSection ( &G_CriticalSection );

            /* 
             * The message object will be free'd by WriteToMessage
             */
            sampStatus = WriteToMessage( &G_options,
                                         fname, 
                                         &msgID,
                                         serviceName);
            if (sampStatus == SAMP_SUCCESS)
                respStatus = C_STORE_SUCCESS;       
            else
                respStatus = C_STORE_FAILURE_PROCESSING_FAILURE;
        }
        else
        {
            printf ("%s\t\tImage %d received (%s)", prefix, nimages, serviceName);

            mcStatus = MC_Free_Message(&msgID);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message for request message error", mcStatus);
                MC_Abort_Association(&assocID);
                break;
            }
        }


        /*
         *  Acquire a response message object, send it, and free it
         */
        mcStatus = MC_Open_Message (&rspMsgID, serviceName, C_STORE_RSP);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message failed", mcStatus);
            MC_Abort_Association(&assocID);
            break;
        }

        mcStatus = MC_Send_Response_Message(assocID, respStatus, rspMsgID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message failed", mcStatus);
            MC_Abort_Association(&assocID);
            MC_Free_Message(&rspMsgID);
            break;
        }
      
        mcStatus = MC_Free_Message(&rspMsgID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message for response message failed", mcStatus);
            MC_Abort_Association(&assocID);
            break;
        }
        
        time (&currentTime);
        seconds = (long)(currentTime - imageStartTime);
        printf("\t%ld second%s\n\n", seconds, seconds == 1 ? "" : "s");
    }

    if (transactionList)
    {
        printf("Sending storage commitment N-EVENT message\n");
        
        SendStorageCommitNEvent(
                        &G_options,
                        &transactionList );
    }
    
    /*
     * Ending the thread so do the cleanup
     */
    printf ( "%s\tEnding thread naturally.\n", prefix );

    /*
     * The G_threadCount variable is a shared global.  Therefore, before
     * using it we must enter a critical section.
     */
    EnterCriticalSection ( &G_CriticalSection );
    --G_threadCount;
    LeaveCriticalSection ( &G_CriticalSection );

    _endthread();
    return;      
}


/********************************************************************
 *
 *  Function    :    HandleStorageCommitMessage
 *
 *  Parameters  :    A_associationID - Association identifier
 *                   A_messageID - ID of message received
 *                   A_list      - List of transactions.
 *
 *  Returns     :    SAMP_SUCCESS - if not errors occurred
 *                   SAMP_FAILURE - if errors occurred
 *
 *  Description :    
 *
 ********************************************************************/
static SAMP_STATUS HandleStorageCommitMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID, 
                        int                 A_messageID,
                        char*               A_remoteAE,
                        TransactionNode**   A_list )
{
    TransactionNode* node;
    InstanceNode*    instanceNode;
    MC_STATUS        mcStatus;
    SAMP_STATUS      sampStatus;
    int              rspMessageID;
    
    mcStatus = MC_Open_Message (&rspMessageID, 
                                "STORAGE_COMMITMENT_PUSH", 
                                N_ACTION_RSP);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error for n-action-rsp", mcStatus);
        return ( SAMP_FAILURE );
    }
    
    sampStatus = ParseStorageCommitMessage(
                          A_options,
                          A_messageID,
                          A_remoteAE,
                          &node );
    if ( sampStatus == SAMP_SUCCESS )
    {
        /*
         * Save in the transaction list
         */
        node->Next = *A_list;
        *A_list = node;
       
        mcStatus = MC_Send_Response_Message(A_associationID,
                                            N_ACTION_SUCCESS, 
                                            rspMessageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", mcStatus);
            MC_Free_Message(&rspMessageID);
            return SAMP_FAILURE;
        }
        
    }
    else
    {
        if (node)
        {
            while( node->InstanceList )
            {
                instanceNode = node->InstanceList;
                node->InstanceList = node->InstanceList->Next;
           
                free( instanceNode );
            }
            
            free( node );
        }
   
        printf("Sending processing failure status for N-ACTION-RQ\n");

        mcStatus = MC_Send_Response_Message(A_associationID,
                                            N_ACTION_PROCESSING_FAILURE, 
                                            rspMessageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", mcStatus);
            MC_Free_Message(&rspMessageID);
            return ( SAMP_FAILURE );
        }
   
    }
    

    mcStatus = MC_Free_Message(&rspMessageID); /* Free the response object */ 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error:\n", mcStatus);
        return ( SAMP_FAILURE );
    } 

    return ( SAMP_SUCCESS );
}


/********************************************************************
 *
 *  Function    :    ParseStorageCommitMessage
 *
 *  Parameters  :    A_messageID - ID of message received
 *                   A_remoteAE  - Remote Application Entity Title
 *                   A_node      - Transaction node allocated in this
 *                                 function
 *
 *  Returns     :    SAMP_SUCCESS - if not errors occurred
 *                   SAMP_FAILURE - if errors occurred
 *
 *  Description :    
 *
 ********************************************************************/
static SAMP_STATUS ParseStorageCommitMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_messageID,
                        char*               A_remoteAE,
                        TransactionNode**   A_node )
{
    InstanceNode*    instanceNode;
    MC_STATUS        mcStatus;
    int              itemID;
    
    
    *A_node = malloc(sizeof(TransactionNode));
    if (!*A_node)
    {
        PrintError("Unable to allocate object to store storage commitment information", MC_NORMAL_COMPLETION);
        return ( SAMP_FAILURE );
    }
   
    (*A_node)->InstanceList = NULL;
    
    strncpy((*A_node)->RemoteAE, A_remoteAE, sizeof((*A_node)->RemoteAE));

    mcStatus = MC_Get_Value_To_String(A_messageID, 
                    MC_ATT_TRANSACTION_UID,
                    sizeof((*A_node)->TransactionUID),
                    (*A_node)->TransactionUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String for Transaction UID failed", mcStatus);
        return(SAMP_FAILURE);
    }

    if (A_options->Verbose)
    {
        printf("Received Storage Commitment N-ACTION-RQ from %s for\n", A_remoteAE);
        printf("    transaction UID: %s\n", (*A_node)->TransactionUID );
        printf("    Objects:\n");
    }
    else
        printf("Received Storage Commitment N-ACTION-RQ\n\n");


    mcStatus = MC_Get_Next_Value_To_Int( A_messageID,
                                           MC_ATT_REFERENCED_SOP_SEQUENCE,
                                           &itemID );
    while ( mcStatus == MC_NORMAL_COMPLETION )
    {
        instanceNode = malloc(sizeof(InstanceNode));
        if (!instanceNode)
        {
            PrintError("Unable to allocate InstanceNode object", MC_NORMAL_COMPLETION);
            return ( SAMP_FAILURE );
        }


        mcStatus = MC_Get_Value_To_String( itemID,
                                           MC_ATT_REFERENCED_SOP_CLASS_UID,
                                           sizeof(instanceNode->SOPClassUID),
                                           instanceNode->SOPClassUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to get SOP Class UID in n-action message", mcStatus);
            free(instanceNode);
            return ( SAMP_FAILURE );
        }

        mcStatus = MC_Get_Value_To_String( itemID,
                                           MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                           sizeof(instanceNode->SOPInstanceUID),
                                           instanceNode->SOPInstanceUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to get SOP Instance UID in n-action message", mcStatus);
            free(instanceNode);
            return ( SAMP_FAILURE );
        }
 
        if (A_options->Verbose)
        {
            printf("       SOP Class UID: %s\n", instanceNode->SOPClassUID );
            printf("    SOP Instance UID: %s\n\n", instanceNode->SOPInstanceUID );
        }

        instanceNode->Next = (*A_node)->InstanceList;
        (*A_node)->InstanceList = instanceNode;

        mcStatus = MC_Get_Next_Value_To_Int( A_messageID,
                                             MC_ATT_REFERENCED_SOP_SEQUENCE,
                                             &itemID );
    }

    return ( SAMP_SUCCESS );
}


/********************************************************************
 *
 *  Function    :    SendStorageCommitNEvent
 *
 *  Parameters  :    A_appID     - application ID
 *                   A_node      - Transaction node allocated in this
 *                                 function
 *
 *  Returns     :    SAMP_SUCCESS - if not errors occurred
 *                   SAMP_FAILURE - if errors occurred
 *
 *  Description :    
 *
 ********************************************************************/
static SAMP_STATUS SendStorageCommitNEvent(
                        STORAGE_OPTIONS*    A_options,
                        TransactionNode**   A_list )
{
    TransactionNode* node;
    InstanceNode*    instanceNode;
    int           associationID = -1;
    int           calledApplicationID = -1;
    SAMP_STATUS   sampStatus;
    MC_STATUS     mcStatus;


    if (!*A_list)
    {
        printf("No objects to commit.\n");
        return ( SAMP_SUCCESS );
    }
    
    node = *A_list;
    
    while ( node )
    {
        if (A_options->Verbose)
            printf("\nOpening N-EVENT-REPORT association to %s\n", node->RemoteAE);

        mcStatus = MC_Open_Association( A_options->ApplicationID, &associationID,
                                    node->RemoteAE,
                                    NULL,
                                    NULL,
                                    "Storage_Commit_SCP_Service_List" );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            printf("Unable to open association with \"%s\":\n",node->RemoteAE);
            printf("\t%s\n", MC_Error_Message(mcStatus));
            return ( SAMP_FAILURE );
        }

        sampStatus = SetAndSendNEventMessage( A_options,
                                              associationID,
                                              node );
        if ( !sampStatus )
        {
            MC_Abort_Association(&associationID);
        }
        else
        {
            /*
             * When the close association fails, there's nothing really to be
             * done.  Let's still continue on and wait for an N-EVENT-REPORT
             */
            mcStatus = MC_Close_Association( &associationID);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                PrintError("Close association failed", mcStatus);
                MC_Abort_Association(&associationID);
            }
            
            if (A_options->Verbose)
                printf("Closing N-EVENT-REPORT association\n\n");
        }
        
        while ( node->InstanceList )
        {
            instanceNode = node->InstanceList->Next;
            free( node->InstanceList );
            node->InstanceList = instanceNode;
        }
        
        *A_list = node->Next;
        free ( node );
        node = *A_list;
    }
   
    
    return ( SAMP_SUCCESS );

}



/****************************************************************************
 *
 *  Function    :   SetAndSendNEventMessage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered 
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_SUCCESS
 *                  SAMP_FAILURE
 *
 *  Description :   Perform storage commitment for a set of storage objects.
 *                  When completed, the list of objects is freed.
 *
 ****************************************************************************/
static SAMP_STATUS SetAndSendNEventMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID,
                        TransactionNode*    A_node)

{
    MC_STATUS      mcStatus;
    int            messageID;
    int            itemID;
    InstanceNode*  instanceNode;
    int            responseMessageID;
    char*          responseService;
    MC_COMMAND     responseCommand;
    int            responseStatus;


    mcStatus = MC_Open_Message( &messageID, "STORAGE_COMMITMENT_PUSH",
                              N_EVENT_REPORT_RQ );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Error opening Storage Commitment n-event-report-rq message",mcStatus);
        return ( SAMP_FAILURE );
    }

    /*
     * Set the well-known SOP instance for storage commitment Push, as
     * listed in DICOM PS3.4, J.3.5
     */
    mcStatus = MC_Set_Value_From_String( messageID, 
                                         MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                         "1.2.840.10008.1.20.1.1");
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for requested sop instance uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return ( SAMP_FAILURE );
    }

    mcStatus = MC_Set_Next_Value_From_Int( messageID,
                                           MC_ATT_EVENT_TYPE_ID,
                                           1 );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to set event type ID in n-action message", mcStatus);
        MC_Free_Message( &messageID );
        return ( SAMP_FAILURE );
    }

    /*
     * Set the transaction UID.  Note that in a real storage commitment
     * application, this UID should be tracked and associated with the 
     * SOP instances asked for commitment with this request.  That way if 
     * multiple storage commitment requests are outstanding, and an 
     * N-EVENT-REPORT comes in, we can associate the message with the 
     * proper storage commitment request.
     */
    mcStatus = MC_Set_Value_From_String( messageID, 
                                         MC_ATT_TRANSACTION_UID,
                                         A_node->TransactionUID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for transaction uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return ( SAMP_FAILURE );
    }


    instanceNode = A_node->InstanceList;
    while (instanceNode)
    {
        mcStatus = MC_Open_Item( &itemID,
                                 "REF_SOP_MEDIA_AE" );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return ( SAMP_FAILURE );
        }        
        
        mcStatus = MC_Set_Next_Value_From_Int( messageID,
                                               MC_ATT_REFERENCED_SOP_SEQUENCE,
                                               itemID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set ItemID in n-event message", mcStatus);
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return ( SAMP_FAILURE );
        }
        
        mcStatus = MC_Set_Value_From_String( itemID,
                                             MC_ATT_REFERENCED_SOP_CLASS_UID,
                                             instanceNode->SOPClassUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set SOP Class UID in n-event message", mcStatus);
            MC_Free_Message( &messageID );
            return ( SAMP_FAILURE );
        }
        
        mcStatus = MC_Set_Value_From_String( itemID,
                                             MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                             instanceNode->SOPInstanceUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set SOP Instance UID in n-event message", mcStatus);
            MC_Free_Message( &messageID );
            return ( SAMP_FAILURE );
        }
        
        instanceNode = instanceNode->Next;
    }
    
    if ( A_options->Verbose )
        printf("Sending N-EVENT-REPORT message\n");


    /*
     * Once the message has been built, we are then able to perform the
     * N-EVENT-REPORT-RQ on it.
     */
    mcStatus = MC_Send_Request_Message( A_associationID, messageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to send N-ACTION-RQ message",mcStatus);
        MC_Free_Message( &messageID );
        return ( SAMP_FAILURE );
    }

    /*
     * After sending the message, we free it.
     */
    mcStatus = MC_Free_Message( &messageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to free N-ACTION-RQ message",mcStatus);
        return ( SAMP_FAILURE );
    }
    

    /*
     *  Wait for response
     */
    mcStatus = MC_Read_Message(A_associationID, 30, &responseMessageID,
                 &responseService, &responseCommand);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed for N-EVENT-REPORT-RSP", mcStatus);
        return ( SAMP_FAILURE );
    }

    mcStatus = MC_Get_Value_To_Int(responseMessageID, MC_ATT_STATUS, &responseStatus);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_Int for N-EVENT-REPORT-RSP status failed",mcStatus);
        MC_Free_Message(&responseMessageID);
        return ( SAMP_FAILURE );
    }

    switch (responseStatus)
    {
        case N_EVENT_PROCESSING_FAILURE:
            printf("N-EVENT-REPORT-RSP failed because of processing failure\n");
            MC_Free_Message(&responseMessageID);
            return ( SAMP_FAILURE );
        case N_EVENT_SUCCESS:
            if ( A_options->Verbose )
                printf("N-EVENT-REPORT message received with SUCCESS status\n");
            break;
        default:
            printf("N-EVENT-REPORT-RSP failure, status=%x\n",responseStatus);
            MC_Free_Message(&responseMessageID);
            return ( SAMP_FAILURE );
    }

    mcStatus = MC_Free_Message( &responseMessageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to free N-EVENT_REPORT-RSP message",mcStatus);
        return ( SAMP_FAILURE );
    }

    
    return( SAMP_SUCCESS );

} /* End of SetAndSendNActionMessage */





/********************************************************************
 *
 *  Function    :    WriteToMessage
 *
 *  Parameters  :    Aoptions   - Structure that holds configuration 
 *                                parameters
 *                   A_filename - Filename to write to
 *                   A_msgID    - ID of message to write
 *                   A_msgType  - Character string describing the
 *                                message type
 *
 *  Returns     :    SAMP_SUCCESS - if not errors occurred
 *                   SAMP_FAILURE - if errors occurred
 *
 *  Description :    Function is used to write a message received 
 *                   over the network to a file in the DICOM "stream"
 *                   format.
 *
 ********************************************************************/
static SAMP_STATUS WriteToMessage( STORAGE_OPTIONS* A_options,
                                   char*     A_filename,
                                   int*      A_msgID,
                                   char*     A_messageType )
{
    MC_STATUS        mcStatus;
    CBinfo           callbackInfo;
    TRANSFER_SYNTAX  resultSyntax;
    TRANSFER_SYNTAX  messageSyntax;
    char             sopInstanceUID[UI_LENGTH+2];

    /*
     * Get the transfer syntax that the message was transfered over
     * the network as.  This is to determine if it is an encapsulated/
     * JPEG transfer syntax.  Note that by default the program does
     * not support encapsulated transfer syntaxes.  Transfer syntax lists
     * containing the encapsualted syntaxes must be added the service
     * lists.
     */
    mcStatus = MC_Get_Message_Transfer_Syntax( *A_msgID, &messageSyntax );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Warning: error on MC_Get_Message_Transfer_Syntax", mcStatus);
        resultSyntax = A_options->SaveSyntax;
    }
    else
    {
        switch (messageSyntax)
        {
            case IMPLICIT_LITTLE_ENDIAN:
            case EXPLICIT_LITTLE_ENDIAN:
            case EXPLICIT_BIG_ENDIAN:
            case IMPLICIT_BIG_ENDIAN:
                resultSyntax = A_options->SaveSyntax;
                break;
                
            case RLE:
            case JPEG_BASELINE:
            case JPEG_EXTENDED_2_4:
            case JPEG_EXTENDED_3_5:
            case JPEG_SPEC_NON_HIER_6_8:
            case JPEG_SPEC_NON_HIER_7_9:
            case JPEG_FULL_PROG_NON_HIER_10_12:
            case JPEG_FULL_PROG_NON_HIER_11_13:
            case JPEG_LOSSLESS_NON_HIER_14:
            case JPEG_LOSSLESS_NON_HIER_15:
            case JPEG_EXTENDED_HIER_16_18:
            case JPEG_EXTENDED_HIER_17_19:
            case JPEG_SPEC_HIER_20_22:
            case JPEG_SPEC_HIER_21_23:
            case JPEG_FULL_PROG_HIER_24_26:
            case JPEG_FULL_PROG_HIER_25_27:
            case JPEG_LOSSLESS_HIER_28:
            case JPEG_LOSSLESS_HIER_29:
            case JPEG_LOSSLESS_HIER_14:
            case JPEG_2000_LOSSLESS_ONLY:
            case JPEG_2000:
            case JPEG_LS_LOSSLESS:
            case JPEG_LS_LOSSY:
            case MPEG2_MPML:
            case PRIVATE_SYNTAX_1:
            case PRIVATE_SYNTAX_2:
                resultSyntax = messageSyntax;
                printf("Warning: Encapsulated transfer syntax (%s) image specified\n", 
                       GetSyntaxDescription(messageSyntax));
                printf("         Not sending image.\n");
                MC_Free_File(A_msgID);
                return SAMP_FAILURE; 
                break;
        }
    } 


    if (A_options->Verbose)
    {
        /*
         * Get the SOP Instance UID from the message, and then print out
         * image information
         */
        mcStatus = MC_Get_Value_To_String( *A_msgID,
                                           MC_ATT_SOP_INSTANCE_UID,
                                           sizeof(sopInstanceUID),
                                           sopInstanceUID );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to get SOP Instance UID from image file",mcStatus);
        }
        
        printf("Writing image in DICOM \"Stream\" format:\n");
        printf("\tMessage Type: %s\n", A_messageType);
        printf("\tFile Name: %s\n", A_filename);
        printf("\tStored Transfer syntax: %s\n", GetSyntaxDescription(resultSyntax));
        printf("\tSOP Instance UID: %s\n", sopInstanceUID );
    
    }
    else
        printf("Writing %s image in DICOM \"Stream\" format:  %s\n", A_messageType, A_filename);



    callbackInfo.fp = fopen(A_filename, BINARY_WRITE);
    if (!callbackInfo.fp)
    {
        printf("\tUnable to open output file: %s\n", A_filename);
        MC_Free_Message(A_msgID);
        return SAMP_FAILURE;
    }

    /*
     * do the callback to copy all the elements into the new file which is
     *  just stream formatted.
     */
    mcStatus = MC_Message_To_Stream( *A_msgID,
                                     0x00000000,
                                     0xFFFFFFFF,
                                     A_options->SaveSyntax,
                                     (void*)&callbackInfo,
                                     MsgObjToFile );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        if (callbackInfo.fp)
            fclose(callbackInfo.fp);
        PrintError("Error in message to stream", mcStatus);
        MC_Free_Message(A_msgID);
        return SAMP_FAILURE;
    }
    
    if (callbackInfo.fp)
        fclose(callbackInfo.fp);

    mcStatus = MC_Free_Message(A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message for request message error", mcStatus);
        return SAMP_FAILURE;
    }

    return SAMP_SUCCESS;
} /* WriteToMessage() */



/********************************************************************
 *
 *  Function    :    WriteToMedia
 *
 *  Parameters  :    Aoptions   - Structure that holds configuration 
 *                                parameters
 *                   A_filename - Filename to write to
 *                   A_msgID    - ID of message received to write
 *                   A_msgType  - Character string describing the
 *                                message type
 *
 *  Returns     :    SAMP_SUCCESS - If successful
 *                   SAMP_FAILURE - If error occurred
 *
 *  Description :    Converts a message received over the network
 *                   into a DICOM-3 formatted file.
 *
 ********************************************************************/
static SAMP_STATUS WriteToMedia( STORAGE_OPTIONS* A_options,
                                 char*            A_filename,
                                 int*             A_msgID,
                                 char*            A_messageType )
{
    MC_STATUS        mcStatus;
    SAMP_STATUS      sampStatus;
    CBinfo           callbackInfo;
    TRANSFER_SYNTAX  messageSyntax;
    TRANSFER_SYNTAX  resultSyntax;
    char             sopInstanceUID[UI_LENGTH+2];


    /*
     * Get the transfer syntax that the message was transfered over
     * the network as.  This is to determine if it is JPEG encoded.
     */
    mcStatus = MC_Get_Message_Transfer_Syntax( *A_msgID, &messageSyntax );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Error on MC_Get_Message_Transfer_Syntax", mcStatus);
        MC_Free_Message(A_msgID);
        return SAMP_FAILURE;
    }
    else
    {
        /*
         * For the standard syntaxes, save in the configured syntax.
         * For all the encapsualted/compressed syntaxes, save in the
         * syntax received.
         */
        switch (messageSyntax)
        {
            case IMPLICIT_LITTLE_ENDIAN:
            case EXPLICIT_LITTLE_ENDIAN:
            case EXPLICIT_BIG_ENDIAN:
            case IMPLICIT_BIG_ENDIAN:
                resultSyntax = A_options->SaveSyntax;
                break;
                
            default:
                resultSyntax = messageSyntax;
                break;
        }
    } 

    /*
     * Now convert message object to a file object.  This changes the 
     * tool kit's internal representation of the file from a message to a 
     * file object.  It also allows the group 0x0002 elements to be used
     * in the object.  Any other attributes within the message can still 
     * be dealt with when it is classified as a file.
     */
    mcStatus = MC_Message_To_File( *A_msgID, A_filename );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Conversion of message to file", mcStatus);
        MC_Free_File(A_msgID);
        return SAMP_FAILURE;
    }

    /*
     * Add the group 2 elements to the object.
     */
    sampStatus = AddGroup2Elements( A_options, resultSyntax, *A_msgID );
    if ( sampStatus == SAMP_FAILURE )
    {
        PrintError("Adding group two elements", mcStatus);
        MC_Free_File(A_msgID);
        return SAMP_FAILURE;
    }


    if (A_options->Verbose)
    {
        /*
         * Get the SOP Instance UID from the message, and then print out
         * image information
         */
        mcStatus = MC_Get_Value_To_String( *A_msgID,
                                           MC_ATT_SOP_INSTANCE_UID,
                                           sizeof(sopInstanceUID),
                                           sopInstanceUID );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to get SOP Instance UID from image file",mcStatus);
        }
        
        printf("Writing image in DICOM Part 10 format:\n");
        printf("\tMessage Type: %s\n", A_messageType);
        printf("\tFile Name: %s\n", A_filename);
        printf("\tStored Transfer syntax: %s\n", GetSyntaxDescription(resultSyntax));
        printf("\tSOP Instance UID: %s\n", sopInstanceUID );
    
    }
    else
        printf("Writing %s image in DICOM Part 10 format:  %s\n", 
               A_messageType, 
               A_filename);
    
    
    /*
     * Write out the new file.
     */
    mcStatus = MC_Write_File( *A_msgID,
                              0,
                              &callbackInfo,
                              FileObjToMedia );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        if (callbackInfo.fp)
            fclose(callbackInfo.fp);
        PrintError("MC_Write_File failed", mcStatus);
        MC_Free_File(A_msgID);
        return SAMP_FAILURE;
    }

    if (callbackInfo.fp)
        fclose(callbackInfo.fp);
 
    mcStatus = MC_Free_File(A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_File for converted message failed", mcStatus);
        return SAMP_FAILURE;
    }

    return SAMP_SUCCESS;
} /* WriteToMedia() */



/********************************************************************
 *
 *  Function    :   FileMetaInfoVersion
 *
 *  Parameters  :   
 *
 *  Returns     :   MC_NORMAL_COMPLETION
 *
 *  Description :   Adds group 2 element for File Meta Information Version
 *                  to the preamble of the DICOM file object
 *
 ********************************************************************/
static MC_STATUS FileMetaInfoVersion( int           A_msgID,
                                      unsigned long A_tag,
                                      int           A_isFirst,
                                      void*         A_info,
                                      int*          A_dataSize,
                                      void**        A_dataBufferPtr,
                                      int*          A_isLastPtr)
{
    static char version[] = {0x00,0x01};

    *A_dataSize = 2;
    *A_dataBufferPtr = version;
    *A_isLastPtr = 1;

    return MC_NORMAL_COMPLETION;

} /* FileMetaInfoVersion() */


/****************************************************************************
 *
 *  Function    :   AddGroup2Elements
 *
 *  Parameters  :   A_options    - Input parameters
 *                  AtransSyntax - structure for config options
 *                  AfileID      - File to add meta information to
 *
 *  Returns     :   SAMP_STATUS
 *
 *  Description :   Sets group two information in a media file  
 *
 ****************************************************************************/
static SAMP_STATUS AddGroup2Elements( STORAGE_OPTIONS*  A_options,
                                      TRANSFER_SYNTAX   A_transSyntax, 
                                      int               A_fileID )
{
    MC_STATUS mcStatus;
    char      uidBuffer[UI_LENGTH+2];
    char      syntaxUID[UI_LENGTH+2];

    /*
     * Get the correct UID for the new transfer syntax
     */
    mcStatus = MC_Get_Transfer_Syntax_From_Enum(A_transSyntax,
                                                syntaxUID,
                                                sizeof(syntaxUID));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get transfer syntax UID", mcStatus);
        return SAMP_FAILURE;
    }

    /*
     * Set the new transfer syntax for this message
     */
    mcStatus = MC_Set_Value_From_String(A_fileID,
                                        MC_ATT_TRANSFER_SYNTAX_UID,
                                        syntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set transfer syntax UID in group 2 elements", mcStatus);
        return SAMP_FAILURE;
    }

    /*
     * Set other media group 2 elements
     */
    mcStatus = MC_Set_Value_From_Function( A_fileID,
                                           MC_ATT_FILE_META_INFORMATION_VERSION,
                                           NULL,
                                           FileMetaInfoVersion );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add file meta information version", mcStatus);
        return SAMP_FAILURE;
    }


    mcStatus = MC_Get_Value_To_String( A_fileID,
                                       MC_ATT_SOP_CLASS_UID,
                                       sizeof(uidBuffer),
                                       uidBuffer );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from image file", mcStatus);
        return SAMP_FAILURE; 
    }

    mcStatus = MC_Set_Value_From_String( A_fileID,
                                         MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID,
                                         uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP Class UID", mcStatus);
        return SAMP_FAILURE;
    }

    mcStatus = MC_Get_Value_To_String(A_fileID,
                                      MC_ATT_SOP_INSTANCE_UID,
                                      sizeof(uidBuffer),
                                      uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from image file",mcStatus);
        return SAMP_FAILURE;
    }

    mcStatus = MC_Set_Value_From_String( A_fileID,
                                         MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID,
                                         uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP instance UID",mcStatus);
        return SAMP_FAILURE;
    }

    mcStatus = MC_Set_Value_From_String( A_fileID,
                                         MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,
                                         A_options->LocalAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add source application entity title",mcStatus);
        return SAMP_FAILURE;
    }

    return SAMP_SUCCESS;

} /* AddGroup2Elements() */




/****************************************************************************
 *
 *  Function    :   FileObjToMedia
 *
 *  Parameters  :   A_filename   - Filename to write to
 *                  A_userInfo   - Information to store between calls to this
 *                                 function
 *                  A_dataSize   - Size of A_dataBuffer
 *                  A_dataBuffer - Buffer containing data to write
 *                  A_isFirst    - set to true on first call
 *                  A_isLast     - Is set to true on the final call
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success, any other MC_STATUS
 *                  value on failure.
 *
 *  Description :   Callback function used to write DICOM file object to
 *                  media.  A pointer to this function is passed to 
 *                  MC_Write_File
 *
 ****************************************************************************/
static MC_STATUS FileObjToMedia( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    size_t     count;
    CBinfo*    cbInfo = (CBinfo*)A_userInfo;

    if (A_isFirst)
        cbInfo->fp = fopen(A_filename, BINARY_WRITE);

    if (!cbInfo->fp)
       return MC_CANNOT_COMPLY;

    count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfo->fp);
    if (count != (size_t)A_dataSize)
    {
        printf("fwrite error");
        return MC_CANNOT_COMPLY;
    }

    if (A_isLast)
    {
        /*
         * NULL ->fp so that the routine calling MC_Write file knows
         * not to close the stream.
         */
        fclose(cbInfo->fp);
        cbInfo->fp = NULL;
    }

    return MC_NORMAL_COMPLETION;

} /* FileObjToMedia() */




/*************************************************************************
 *
 *  Function    :   MsgObjToFile
 *
 *  Parameters  :   A_msgID      - Message id of message being read
 *                  A_userInfo   - Information to store between calls to
 *                                 this function
 *                  A_dataSize   - Length of data read
 *                  A_dataBuffer - Buffer where read data is stored
 *                  A_isFirst    - Flag to tell if this is the first call
 *                  A_isLast     - Flag to tell if this is the last call
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success, any other MC_STATUS
 *                  value on failure.
 *
 *  Description :   This function is used to write data in the "Stream"
 *                  format.  A pointer to the function is passed as a 
 *                  parameter to MC_Message_To_Stream.
 *
 **************************************************************************/
static MC_STATUS MsgObjToFile (int      A_msgID,
                               void*    A_userInfo,
                               int      A_dataSize,
                               void*    A_dataBuffer,
                               int      A_isFirst,
                               int      A_isLast)
{
    CBinfo*     callbackInfo = (CBinfo*)A_userInfo;
    size_t      count;

    if (!callbackInfo->fp)
    {
        printf("\tUnable to open output file\n");
        return MC_CANNOT_COMPLY;
    }

    count = fwrite(A_dataBuffer, 1, A_dataSize, callbackInfo->fp);
    if (count != (size_t)A_dataSize)
    {
        printf("\tfwrite error\n");
        return MC_CANNOT_COMPLY;
    }
    if (A_isLast)
    {
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }

    return MC_NORMAL_COMPLETION;
} /* MsgObjToFile() */



/****************************************************************************
 *
 *  Function    :   GetSyntaxDescription
 *
 *  Description :   Return a text description of a DICOM transfer syntax.
 *
 ****************************************************************************/
static char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr;
    
    switch (A_syntax)
    {
    case IMPLICIT_LITTLE_ENDIAN: ptr = "Implicit VR Little Endian"; break;
    case EXPLICIT_LITTLE_ENDIAN: ptr = "Explicit VR Little Endian"; break;
    case EXPLICIT_BIG_ENDIAN:    ptr = "Explicit VR Big Endian"; break;
    case IMPLICIT_BIG_ENDIAN:    ptr = "Implicit VR Big Endian"; break;
    case RLE:                    ptr = "RLE"; break;
    case JPEG_BASELINE:          ptr = "JPEG Baseline (Process 1)"; break;
    case JPEG_EXTENDED_2_4:      ptr = "JPEG Extended (Process 2 & 4)"; break;
    case JPEG_EXTENDED_3_5:      ptr = "JPEG Extended (Process 3 & 5)"; break;
    case JPEG_SPEC_NON_HIER_6_8: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)"; break;
    case JPEG_SPEC_NON_HIER_7_9: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)"; break;
    case JPEG_FULL_PROG_NON_HIER_10_12: ptr = "JPEG Full Progression, Non-Hierarchical (Process 10 & 12)"; break;
    case JPEG_FULL_PROG_NON_HIER_11_13: ptr = "JPEG Full Progression, Non-Hierarchical (Process 11 & 13)"; break;
    case JPEG_LOSSLESS_NON_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical (Process 14)"; break;
    case JPEG_LOSSLESS_NON_HIER_15: ptr = "JPEG Lossless, Non-Hierarchical (Process 15)"; break;
    case JPEG_EXTENDED_HIER_16_18: ptr = "JPEG Extended, Hierarchical (Process 16 & 18)"; break;
    case JPEG_EXTENDED_HIER_17_19: ptr = "JPEG Extended, Hierarchical (Process 17 & 19)"; break;
    case JPEG_SPEC_HIER_20_22: ptr = "JPEG Spectral Selection Hierarchical (Process 20 & 22)"; break;
    case JPEG_SPEC_HIER_21_23: ptr = "JPEG Spectral Selection Hierarchical (Process 21 & 23)"; break;
    case JPEG_FULL_PROG_HIER_24_26: ptr = "JPEG Full Progression, Hierarchical (Process 24 & 26)"; break;
    case JPEG_FULL_PROG_HIER_25_27: ptr = "JPEG Full Progression, Hierarchical (Process 25 & 27)"; break;
    case JPEG_LOSSLESS_HIER_28: ptr = "JPEG Lossless, Hierarchical (Process 28)"; break;
    case JPEG_LOSSLESS_HIER_29: ptr = "JPEG Lossless, Hierarchical (Process 29)"; break;
    case JPEG_LOSSLESS_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical, First-Order Prediction"; break;
    case JPEG_LS_LOSSLESS:      ptr = "JPEG-LS Lossless"; break;
    case JPEG_LS_LOSSY:         ptr = "JPEG-LS Lossy (Near Lossless)"; break;
    case MPEG2_MPML:            ptr = "MPEG2 Main Profile @ Main Level"; break;
    case PRIVATE_SYNTAX_1:      ptr = "Private Syntax 1"; break;
    case PRIVATE_SYNTAX_2:      ptr = "Private Syntax 2"; break;
    }
    return ptr;
}



/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
static void PrintError(char* A_string, MC_STATUS A_mcStatus)
{
    char        prefix[30] = {0};
    /*
     *  Need process ID number for messages
     */
#ifdef UNIX    
    sprintf(prefix, "PID %d", getpid() );
#endif 
    printf("%s\t%s:\n",prefix,A_string);
    printf("%s\t\t%s\n", prefix,MC_Error_Message(A_mcStatus));
}




/****************************************************************************
 *
 *  Function    :   NTsigint_handler
 *
 *  Description :   This is our SIGINT routine (The user wants to shut down)
 *
 ****************************************************************************/

static void NTsigint_handler
(
   int                Asigno
)

{
   int    Return   = EXIT_SUCCESS;
   DWORD  ThreadId = GetCurrentThreadId();

   printf ( "TID(%lu)\tEnding thread by a SIGNAL.", ThreadId );

   /*
    * The G_threadCount variable is a shared global.  Therefore, before
    * using it we must enter a critical section.
    */

   EnterCriticalSection ( &G_CriticalSection );
   --G_threadCount;
   LeaveCriticalSection ( &G_CriticalSection );

   exit(Return);

}
