/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Tool Kit
 *
 *    $Workfile: stor_scu.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is a sample Service Class User application
 *               for the Storage Service Class and the Storage Commitment
 *               service class.  The application has a number of features:
 *               - It can read in images in both the DICOM Part 10 format 
 *                 and the DICOM "stream" format.
 *               - The application determines the format of the object
 *                 before reading in.
 *               - The application supports DICOM "Asychronous Window
 *                 negotiation" and will transfer asychronously if 
 *                 negotiated.  
 *               - The AE Title, host name, and port number of the 
 *                 system being connected to can be specified on the 
 *                 command line.
 *               - A verbose mode can be specified where detailed
 *                 information is displayed as the application functions.
 *               - The local AE title can be specified on the command 
 *                 line.
 *               - The service list (found in the mergecom.app 
 *                 configuration file) used by the application to 
 *                 determine what services are negotiated can be specified
 *                 on the command line.
 *               - The application will support DICOM Part 10 formated
 *                 compressed/encapsulated if specified on the command
 *                 line.  One note, however, the standard service lists
 *                 found in the mergecom.app file must be extended with
 *                 a transfer syntax list to support these transfer
 *                 syntaxes.
 *               - If specified on the command line, the application will
 *                 send a storage commitment request to the same SCP as 
 *                 it is sending images.  The storage commitment request 
 *                 will be for the images included on the command line.
 *
 *************************************************************************
 *
 *      (c) 2005 Merge Technologies  Incorporated (d/b/a Merge eFilm)
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


/*
 * Standard OS Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <fcntl.h>
#endif

#if defined(_MACINTOSH) && defined(__MWERKS__)
#include <Types.h>
#include <console.h>
#include <SIOUX.h>
#endif


/*
 * MergeCOM-3 Includes
 */
#include "mc3media.h"
#include "mc3msg.h"
#include "mergecom.h"
#include "diction.h"



/*
 * Module constants
 */

/* DICOM VR Lengths */
#define AE_LENGTH 16
#define UI_LENGTH 64


#if defined(_MSDOS)     || defined(__OS2__)   || defined(_WIN32) || \
    defined(_MACINTOSH) || defined(INTEL_WCC) || defined(_RMX3)
#define BINARY_READ "rb"
#define BINARY_WRITE "wb"
#define BINARY_APPEND "rb+"
#define BINARY_READ_APPEND "a+b"
#define BINARY_CREATE "w+b"
#ifdef _MSDOS
#define TEXT_READ "rt"
#define TEXT_WRITE "wt"
#else
#define TEXT_READ "r"
#define TEXT_WRITE "w"
#endif
#else
#define BINARY_READ "r"
#define BINARY_WRITE "w"
#define BINARY_APPEND "r+"
#define BINARY_READ_APPEND "a+"
#define BINARY_CREATE "w+"
#define TEXT_READ "r"
#define TEXT_WRITE "w"
#endif 



/*
 * Module type declarations
 */

/*
 * CBinfo is used to callback functions when reading in stream objects
 * and Part 10 format objects.
 */
typedef struct CALLBACKINFO
{
    FILE*         fp;
    /* 
     * Note!   The size of this buffer impacts toolkit performance.  Higher 
     * values in general should result in increased performance of reading
     * files
     */
    char          buffer[64*1024];  
    unsigned long bytesRead;
} CBinfo;


/*
 * Boolean used to handle return values from functions
 */
typedef enum
{
    SAMP_TRUE = 1,
    SAMP_FALSE = 0
} SAMP_BOOLEAN;


/*
 * Structure to store local application information 
 */
typedef struct stor_scu_options
{
    int     StartImage;
    int     StopImage;

    char    RemoteAE[AE_LENGTH+2]; 
    char    LocalAE[AE_LENGTH+2]; 
    char    RemoteHostname[100]; 
    int     RemotePort;
    char    ServiceList[100]; 

    SAMP_BOOLEAN Verbose;
    SAMP_BOOLEAN HandleEncapsulated;
    SAMP_BOOLEAN StorageCommit;
    
    AssocInfo               asscInfo;
    
    SAMP_BOOLEAN UseFileList;
    char         FileList[1024];
} STORAGE_OPTIONS;


/*
 * Used to identify the format of an object
 */
typedef enum
{
    UNKNOWN_FORMAT = 0,
    MEDIA_FORMAT = 1,
    IMPLICIT_LITTLE_ENDIAN_FORMAT,
    IMPLICIT_BIG_ENDIAN_FORMAT,
    EXPLICIT_LITTLE_ENDIAN_FORMAT,
    EXPLICIT_BIG_ENDIAN_FORMAT
} FORMAT_ENUM;


/*
 * Structure to maintain list of instances sent & to be sent.
 * The structure keeps track of all instances and is used
 * in a linked list.
 */
typedef struct instance_node
{
    int    msgID;                       /* messageID of for this node */
    char   fname[1024];                 /* Name of file */
    TRANSFER_SYNTAX transferSyntax;     /* Transfer syntax of file */
    
    char   SOPClassUID[UI_LENGTH+2];    /* SOP Class UID of the file */
    char   serviceName[48];             /* MergeCOM-3 service name for SOP Class */
    char   SOPInstanceUID[UI_LENGTH+2]; /* SOP Instance UID of the file */
    
    unsigned long imageBytes;           /* size in bytes of the file */
    
    unsigned int dicomMsgID;            /* DICOM Message ID in group 0x0000 elements */
    unsigned int status;                /* DICOM status value returned for this file. */
    char   statusMeaning[100];          /* Textual meaning of "status" */   
    SAMP_BOOLEAN responseReceived;      /* Bool indicating we've received a response for a sent file */
    SAMP_BOOLEAN failedResponse;        /* Bool saying if a failure response message was received */
    SAMP_BOOLEAN imageSent;             /* Bool saying if the image has been sent over the association yet */
    SAMP_BOOLEAN mediaFormat;           /* Bool saying if the image was originally in media format (Part 10) */
    
    struct instance_node* Next;         /* Pointer to next node in list */

} InstanceNode;


/* 
 *  Local Function prototypes
 */

int main(               int                 argc, 
                        char**              argv);
                        
static SAMP_BOOLEAN TestCmdLine( 
                        int                 A_argc,   
                        char*               A_argv[], 
                        STORAGE_OPTIONS*    A_options );
                        
static SAMP_BOOLEAN AddFileToList(
                        InstanceNode**      A_list,
                        char*               A_fname);
                        
static SAMP_BOOLEAN UpdateNode(
                        InstanceNode*       A_node );

static void FreeList(   InstanceNode**      A_list );

static int GetNumNodes( InstanceNode*       A_list);

static int GetNumOutstandingRequests(
                        InstanceNode*       A_list);
                        
static SAMP_BOOLEAN StorageCommitment(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_appID, 
                        InstanceNode**      A_list);

static SAMP_BOOLEAN SetAndSendNActionMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID,
                        InstanceNode**      A_list);

static SAMP_BOOLEAN HandleNEventAssociation(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID, 
                        InstanceNode**      A_list);
                        
static SAMP_BOOLEAN ProcessNEventMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_messageID, 
                        InstanceNode**      A_list);

static SAMP_BOOLEAN CheckResponseMessage ( 
                        int                 A_responseMsgID, 
                        unsigned int*       A_status, 
                        char*               A_statusMeaning, 
                        size_t              A_statusMeaningLength );
                        
static FORMAT_ENUM CheckFileFormat( 
                        char*               A_filename );
                        
static SAMP_BOOLEAN ReadImage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_appID, 
                        InstanceNode*       A_node);
                        
static SAMP_BOOLEAN SendImage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID, 
                        InstanceNode*       A_node);

static SAMP_BOOLEAN ReadResponseMessages(STORAGE_OPTIONS*  A_options,
                        int                 A_associationID, 
                        int                 A_timeout,
                        InstanceNode**      A_list);

static SAMP_BOOLEAN ReadFileFromMedia( 
                        STORAGE_OPTIONS*    A_options,
                        int                 A_appID,
                        char*               A_filename,
                        int*                A_msgID,
                        TRANSFER_SYNTAX*    A_syntax,
                        unsigned long*      A_bytesRead);

static SAMP_BOOLEAN ReadMessageFromFile( 
                        STORAGE_OPTIONS*    A_options,
                        char*               A_fileName,
                        FORMAT_ENUM         A_format,
                        int*                A_msgID,
                        TRANSFER_SYNTAX*    A_syntax,
                        unsigned long*      A_bytesRead);

static MC_STATUS MediaToFileObj( 
                        char*               Afilename,
                        void*               AuserInfo,
                        int*                AdataSize,
                        void**              AdataBuffer,
                        int                 AisFirst,
                        int*                AisLast);
                                 
static MC_STATUS StreamToMsgObj( 
                        int                 AmsgID,
                        void*               AcBinformation,
                        int                 AfirstCall,
                        int*                AdataLen,
                        void**              AdataBuffer,
                        int*                AisLast);
                                 
static char* Create_Inst_UID(
                        void);

static void PrintError (char*               A_string, 
                        MC_STATUS           A_status);
                        
static char* GetSyntaxDescription(
                        TRANSFER_SYNTAX     A_syntax);



/****************************************************************************
 *
 *  Function    :   Main
 *
 *  Description :   Main routine for DICOM Storage Service Class SCU 
 *
 ****************************************************************************/
#ifdef VXWORKS
int  storscu(int argc, char** argv);
int storscu(int argc, char** argv)
#else
int  main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    SAMP_BOOLEAN            sampBool;
    STORAGE_OPTIONS         options;
    MC_STATUS               mcStatus;
    int                     applicationID = -1;
    int                     associationID = -1;
    int                     imageCurrent = 0;
    time_t                  assocStartTime = 0L;
    time_t                  assocEndTime = 0L;
    time_t                  imageStartTime = 0L;
    time_t                  imageEndTime = 0L;
    time_t                  totalTime = 0L;
    char                    fname[512];  /* Extra long, just in case */
    ServiceInfo             servInfo;
    unsigned long           bytesInImage = 0L;
    unsigned long           totalBytesRead = 0L;
    int                     imagesSent = 0L;
    int                     totalImages = 0L;
    InstanceNode*           instanceList = NULL;
    InstanceNode*           node = NULL;
    FILE*                   fp = NULL;
    int                     fstatus = 0;

    /*
     * Macintosh specific code to handle command line arguments in a 
     * console window (SIOUX) application.
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

    /*
     * Test the command line parameters, and populate the options
     * structure with these parameters
     */
    sampBool = TestCmdLine( argc, argv, &options ); 
    if ( sampBool == SAMP_FALSE )
    {
        return(EXIT_FAILURE);
    }


    /* ------------------------------------------------------- */ 
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */ 
    mcStatus = MC_Library_Initialization ( NULL, NULL, NULL ); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize library", mcStatus);
        return ( EXIT_FAILURE );
    }

    /*
     *  Register this DICOM application
     */
    mcStatus = MC_Register_Application(&applicationID, options.LocalAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to register \"%s\":\n", options.LocalAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }


    /*
     * Create a linked list of all files to be transferred.
     * Retreive the list from a specified file on the command line,
     * or generate the list from the start & stop numbers on the 
     * command line
     */
    if (options.UseFileList)
    {
        /* Read the command line file to create the list */
        fp = fopen(options.FileList, TEXT_READ);
        if (!fp)
        {
            printf("ERROR: Unable to open %s.\n", options.FileList);
            return(EXIT_FAILURE);
        }
        
        for (;;) /* forever loop until break */
        {
            fstatus = fscanf(fp, "%s", fname);
            if (fstatus == EOF || fstatus == 0)
            {
                fclose(fp);
                break;
            }

            if (fname[0] == '#') /* skip commented out rows */
                continue;
                
            sampBool = AddFileToList( &instanceList, fname );
            if (!sampBool)
            {
                printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
            }
         }
    }
    else
    {
        /* Traverse through the possible names and add them to the list based on the start/stop count */ 
        for (imageCurrent = options.StartImage; imageCurrent <= options.StopImage; imageCurrent++)
        {
            sprintf(fname, "%d.img", imageCurrent);
            sampBool = AddFileToList( &instanceList, fname );
            if (!sampBool)
            {
                printf("Warning, cannot add SOP instance to File List, image will not be sent [%s]\n", fname);
            }
        }
    }
   
    totalImages = GetNumNodes( instanceList );
    
    if (options.Verbose)
    {
        printf("Opening connection to remote system:\n");
        printf("    AE title: %s\n", options.RemoteAE);
        if (options.RemoteHostname[0])
            printf("    Hostname: %s\n", options.RemoteHostname);
        else
            printf("    Hostname: Default in mergecom.app\n");
        
        if (options.RemotePort != -1)
            printf("        Port: %d\n", options.RemotePort);
        else
            printf("        Port: Default in mergecom.app\n");
        
        if (options.ServiceList[0])
            printf("Service List: %s\n", options.ServiceList);
        else
            printf("Service List: Default in mergecom.app\n");
            
        printf("   Files to Send: %d \n", totalImages);
    }  

    time(&assocStartTime);
     
    /*
     *   Open association and override hostname & port parameters if 
     *   they were supplied on the command line.
     */
    mcStatus = MC_Open_Association( applicationID, &associationID,
                                    options.RemoteAE,
                                    options.RemotePort != -1 ? &options.RemotePort : NULL, 
                                    options.RemoteHostname[0] ? options.RemoteHostname : NULL,
                                    options.ServiceList[0] ? options.ServiceList : NULL );
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n", options.RemoteAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }
   
    mcStatus = MC_Get_Association_Info( associationID, &options.asscInfo); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Association_Info failed", mcStatus);
    }

    if (options.Verbose)
    {
        printf("Connecting to Remote Application:\n");
        printf("  Remote AE Title:          %s\n", options.asscInfo.RemoteApplicationTitle);
        printf("  Local AE Title:           %s\n", options.asscInfo.LocalApplicationTitle);
        printf("  Host name:                %s\n", options.asscInfo.RemoteHostName);
        printf("  IP Address:               %s\n", options.asscInfo.RemoteIPAddress);
        printf("  Local Max PDU Size:       %d\n", options.asscInfo.LocalMaximumPDUSize);
        printf("  Remote Max PDU Size:      %d\n", options.asscInfo.RemoteMaximumPDUSize);
        printf("  Max operations invoked:   %d\n", options.asscInfo.MaxOperationsInvoked);
        printf("  Max operations performed: %d\n", options.asscInfo.MaxOperationsPerformed);
        printf("  Implementation Version:   %s\n", options.asscInfo.RemoteImplementationVersion);
        printf("  Implementation Class UID: %s\n\n\n", options.asscInfo.RemoteImplementationClassUID);
        
        printf("Services and transfer syntaxes negotiated:\n");
        
        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are 
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
        mcStatus = MC_Get_First_Acceptable_Service(associationID,&servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
            printf("  %-30s: %s\n",servInfo.ServiceName, 
                              GetSyntaxDescription(servInfo.SyntaxType));
            
        
            mcStatus = MC_Get_Next_Acceptable_Service(associationID,&servInfo);
        }
            
        if (mcStatus != MC_END_OF_LIST)
        {
            PrintError("Warning: Unable to get service info",mcStatus);
        }
        
        printf("\n\n");
    }
    else
        printf("Connected to remote system [%s]\n\n", options.RemoteAE);
    
    
    /*
     *   Send all requested images.  Traverse through instanceList to get all 
     *   files to send
     */
    node = instanceList;
    while ( node )
    {
        time(&imageStartTime);

        /*
         * Determine the image format and read the image in.  If the 
         * image is in the part 10 format, convert it into a message.
         */
        sampBool = ReadImage( &options, 
                              applicationID, 
                              node);
        if (!sampBool)
        {
            node->imageSent = SAMP_FALSE;
            printf("Can not open image file [%s]\n", node->fname);
            node = node->Next;
            continue;
        }
       
        totalBytesRead += node->imageBytes;

         
         
        /*
         * Send image read in with ReadImage.  
         *
         * Because SendImage may not have actually sent an image even
         * though it has returned success, the calculation of 
         * performance data below may not be correct.
         */
        sampBool = SendImage( &options, 
                              associationID, 
                              node);
        if (!sampBool)
        {
            node->imageSent = SAMP_FALSE;
            printf("Failure in sending file [%s]\n", node->fname);
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            break;
        }
       
        if ( node->imageSent == SAMP_TRUE )
        {
            /*
             * Save image transfer information in list
             */
            sampBool = UpdateNode( node );
            if (!sampBool)
            {
                printf("Warning, unable to update node with information [%s]\n", node->fname);
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                break;
            }
            
            imagesSent++;
        }
        else
        {
            node->responseReceived = SAMP_TRUE;
            node->failedResponse = SAMP_TRUE;
        }
        
        mcStatus = MC_Free_Message(&node->msgID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message failed for request message", mcStatus);
        }


        /*
		 * The following is the core code for handling DICOM asynchronous
		 * transfers.  With asynchronous communications, the SCU is allowed
		 * to send multiple request messages to the server without 
		 * receiving a response message.  The MaxOperationsInvoked is 
		 * negotiated over the association, and determines how many request
		 * messages the SCU can send before a response must be read.
		 *
		 * In this code, we first poll to see if a response message is 
		 * available.  This means data is readily available to be read over 
		 * the connection.  If there is no data to be read & asychronous 
		 * operations have been negotiated, and we haven't reached the max
		 * number of operations invoked, we can just go ahead and send
		 * the next request message.  If not, we go into the loop below
		 * waiting for the response message from the server.
		 *
		 * This code alows network transfer speeds to improve.  Instead of
		 * having to wait for a response message, the SCU can immediately 
		 * send the next request message so that the connection bandwidth
		 * is better utilized.
         */
        sampBool = ReadResponseMessages( &options, associationID, 0, &instanceList );
        if (!sampBool)
        {
            printf("Failure in reading response message, aborting association.\n");
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            break;
        }
       
        /*
         * 0 for MaxOperationsInvoked means unlimited operations.  don't poll if this is the case, just
         * go to the next request to send.
         */
        if ( options.asscInfo.MaxOperationsInvoked > 0 )
            while ( GetNumOutstandingRequests( instanceList ) >= options.asscInfo.MaxOperationsInvoked )
            {
                sampBool = ReadResponseMessages( &options, associationID, 10, &instanceList );
                if (!sampBool)
                {
                    printf("Failure in reading response message, aborting association.\n");
                    MC_Abort_Association(&associationID);
                    MC_Release_Application(&applicationID);
                    break;
                }
            }


        /*
         *  How long did it take?
         */
        imageEndTime = time((time_t*)NULL);
        if ( options.Verbose )
            printf("     Time: %d seconds\n\n", (int)(imageEndTime - imageStartTime));
        else
            printf("\tSent %s image (%d of %d), elapsed time: %d seconds\n", node->serviceName, imagesSent, totalImages, (int)(imageEndTime - imageStartTime));
            
        /*
         * Traverse through file list
         */
        node = node->Next;
        
    }   /* END for loop for each image */


    /*
     * Wait for any remaining C-STORE-RSP messages.  This will only happen
	 * when asynchronous communications are used.
     */
    while ( GetNumOutstandingRequests( instanceList ) > 0 )
    {
        sampBool = ReadResponseMessages( &options, associationID, 10, &instanceList );
        if (!sampBool)
        {
            printf("Failure in reading response message, aborting association.\n");
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            break;
        }
    }
        
    /*
     * A failure on close has no real recovery.  Abort the association
     * and continue on.
     */
    mcStatus = MC_Close_Association(&associationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Close association failed", mcStatus);
        MC_Abort_Association(&associationID);
    }

    /*
     * Calculate the transfer rate.  Note, for a real performance
     * numbers, a function other than time() to measure elapsed
     * time should be used.
     */
    if (options.Verbose)
    {
        printf("Association Closed.\n" );
    }
   
    /*
     * Time calculation
     */
    assocEndTime = time((time_t*)NULL);
    totalTime = assocEndTime - assocStartTime;
    
    /*
     * Check for divide by zero becaue of a quick transfer.
     */
    if (totalTime == 0) totalTime = 1;
        
    printf("Data Transferred: %luMB\n", totalBytesRead / (1024 * 1024) );
    printf("   Transfer Rate: %luKB/s\n", (totalBytesRead / totalTime) / 1024 );
        

    /*
     * Now, do Storage Commitment if app is configured to do it
     */
    if (options.StorageCommit == SAMP_TRUE)
        StorageCommitment( &options, 
                           applicationID,
                           &instanceList );
        
    /*
     * Release the dICOM Application
     */
    mcStatus = MC_Release_Application(&applicationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Release_Application failed", mcStatus);
    }

    /*
     * Free the node list's allocated memory
     */
    FreeList( &instanceList );
    
    /*
     * Release all memory used by the MergeCOM-3 tool kit.
     */
    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");

    return(EXIT_SUCCESS);
}                               


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
    printf("\nUsage stor_scu remote_ae start stop -a local_ae -n remote_host \n");
    printf("               -p remote_port -l service_list -f filename -c -v -e\n");
    printf("\tremote_ae   name of remote Application Entity Title\n");
    printf("\tstart       start image number (optional if -f specified)\n");
    printf("\tstop        stop image number (optional if -f specified)\n");
    printf("\tfilename    optional specify a file containing a list of images to transfer\n");
    printf("\tlocal_ae    optional specify the local Application Title\n");
    printf("\t            (Default: MERGE_STORE_SCU)\n");
    printf("\tremote_host optional specify the remote hostname\n");
    printf("\t            (Default: found in the mergecom.app file for remote_ae)\n");
    printf("\tremote_port optional specify the remote TCP listen port\n");
    printf("\t            (Default: found in the mergecom.app file for remote_ae)\n");
    printf("\tservice_list optional specify the service list to use when negotiating\n");
    printf("\t            (Default: found in the mergecom.app file for remote_ae)\n");
    printf("\t-c          Do storage commitment for the transferred files\n");
    printf("\t-v          execute in verbose mode, print negotiation information\n");
    printf("\t-e          transfer encapsulated or compressed images\n");
    printf("\t            (Files must be in DICOM Part 10 format)\n");
    printf("\n\tImage files must be in the current directory.\n");
    printf("\n\tImage files must be named 0.img, 1.img, 2.img, etc.\n");

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
 *                  SAMP_FALSE
 *
 *  Description :   Test command line for valid arguements.  If problems
 *                  are found, display a message and return SAMP_FALSE
 *
 *************************************************************************/
static SAMP_BOOLEAN TestCmdLine(  /* Test Command Line */
        int                 A_argc,   
        char*               A_argv[], 
        STORAGE_OPTIONS*    A_options )
{
    int       i;
    int       argCount=0;
    
#ifndef VXWORKS
    if (A_argc < 4)
    {
        PrintCmdLine();
        return SAMP_FALSE;
    }
#endif
   
    /*
     * Set default values
     */
    A_options->StartImage = 1;
    A_options->StopImage = 1;

    strcpy(A_options->LocalAE, "MERGE_STORE_SCU");
    A_options->RemoteAE[0] = '\0'; 
    A_options->RemoteHostname[0] = '\0'; 
    A_options->RemotePort = -1;
    A_options->ServiceList[0] = '\0'; 
    A_options->Verbose = SAMP_FALSE;
    A_options->HandleEncapsulated = SAMP_FALSE;
    A_options->StorageCommit = SAMP_FALSE;

    A_options->UseFileList = SAMP_FALSE;
    A_options->FileList[0] = '\0'; 
#ifdef VXWORKS
    A_options->Verbose = SAMP_TRUE;
    strcpy(A_options->RemoteAE, "MERGE_STORE_SCP");
#endif /* #ifdef VXWORKS */

    /*
     * Loop through each arguement
     */     
    for (i = 1; i < A_argc; i++)
    {
        if ( !strcmp(A_argv[i], "-h") || !strcmp(A_argv[i], "/h") ||
             !strcmp(A_argv[i], "-H") || !strcmp(A_argv[i], "/H") ||
             !strcmp(A_argv[i], "-?") || !strcmp(A_argv[i], "/?"))
        {
            PrintCmdLine();
            return SAMP_FALSE; 
        }
        else if ( !strcmp(A_argv[i], "-a") || !strcmp(A_argv[i], "-A"))
        {
            /*
             * Set the Local AE
             */
            i++;
            strcpy(A_options->LocalAE, A_argv[i]);
        } 
        else if ( !strcmp(A_argv[i], "-n") || !strcmp(A_argv[i], "-N"))
        {
            /*
             * Remote Host Name
             */
            i++;
            strcpy(A_options->RemoteHostname,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-p") || !strcmp(A_argv[i], "-P"))
        {
            /*
             * Remote Port Number
             */
            i++;
            A_options->RemotePort = atoi(A_argv[i]);
        
        }
        else if ( !strcmp(A_argv[i], "-l") || !strcmp(A_argv[i], "-L"))
        {
            /*
             * Service List
             */
            i++;
            strcpy(A_options->ServiceList,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-v") || !strcmp(A_argv[i], "-V"))
        {
            /*
             * Verbose mode
             */
            A_options->Verbose = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-c") || !strcmp(A_argv[i], "-C"))
        {
            /*
             * StorageCommit mode
             */
            A_options->StorageCommit = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-f") || !strcmp(A_argv[i], "-F"))
        {
            /*
             * Config file with filenames
             */
            i++;
            A_options->UseFileList = SAMP_TRUE;
            strcpy(A_options->FileList,A_argv[i]);
        }
        else if ( !strcmp(A_argv[i], "-e") || !strcmp(A_argv[i], "-E"))
        {
            /*
             * Handle encapsulated objects.  This means we will not 
             * ignore them whe reading in.
             */
            A_options->HandleEncapsulated = SAMP_TRUE;
        }
        else
        {
            /*
             * Parse through the rest of the options
             */
            
            argCount++;
            switch (argCount)
            {
                case 1:
                    strcpy(A_options->RemoteAE, A_argv[i]);
                    break;
                case 2:
                    A_options->StartImage = atoi(A_argv[i]);
                    break;
                case 3:
                    A_options->StopImage = atoi(A_argv[i]);
                    break;
                default:
                    printf("Unkown option: %s\n",A_argv[i]);
                    break;
            }
        }
    }

    /*
     * If the hostname & port are specified on the command line, 
     * the user may not have the remote system configured in the 
     * mergecom.app file.  In this case, force the default service 
     * list, so we can attempt to make a connection, or else we would
     * fail.
     */
    if ( A_options->RemoteHostname[0] 
    &&  !A_options->ServiceList[0] 
     && ( A_options->RemotePort != -1))
    {
        strcpy(A_options->ServiceList, "Storage_SCU_Service_List");
    }


    if (A_options->StopImage < A_options->StartImage)
    {
        printf("Image stop number must be greater than or equal to image start number.\n");
        PrintCmdLine();
        return SAMP_FALSE;
    }
    
    return SAMP_TRUE;
    
}/* TestCmdLine() */


/****************************************************************************
 *
 *  Function    :   AddFileToList
 *
 *  Parameters  :   A_list     - List of nodes.
 *                  A_fname    - The name of file to add to the list
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Create a node in the instance list for a file to be sent
 *                  on this association.  The node is added to the end of the
 *                  list.
 *
 ****************************************************************************/
static SAMP_BOOLEAN AddFileToList(
                              InstanceNode**    A_list,
                              char*             A_fname)
{
    InstanceNode*    newNode;
    InstanceNode*    listNode;

    newNode = (InstanceNode*)malloc(sizeof(InstanceNode));
    if (!newNode)
    {
        PrintError("Unable to allocate object to store instance information", MC_NORMAL_COMPLETION);
        return ( SAMP_FALSE );
    }

    memset( newNode, 0, sizeof(InstanceNode) );

    strncpy(newNode->fname, A_fname, sizeof(newNode->fname));
    newNode->fname[sizeof(newNode->fname)-1] = '\0';
    
    newNode->responseReceived = SAMP_FALSE;
    newNode->failedResponse = SAMP_FALSE;
    newNode->imageSent = SAMP_FALSE;
    newNode->msgID = -1;
    newNode->transferSyntax = IMPLICIT_LITTLE_ENDIAN;

    if ( !*A_list )
    {
        /*
         * Nothing in the list
         */
        newNode->Next = *A_list;
        *A_list = newNode;
    }
    else
    {
        /*
         * Add to the tail of the list
         */
        listNode = *A_list;
        
        while ( listNode->Next )
            listNode = listNode->Next;
      
        listNode->Next = newNode;
    }
    
    return ( SAMP_TRUE );
}    


/****************************************************************************
 *
 *  Function    :   UpdateNode
 *
 *  Parameters  :   A_node     - node to update
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Update an image node with info about a file transferred
 *
 ****************************************************************************/
static SAMP_BOOLEAN UpdateNode(
                              InstanceNode*     A_node)
{
    MC_STATUS        mcStatus;
    
    /*
     * Get DICOM msgID for tracking of responses
     */
    mcStatus = MC_Get_Value_To_UInt(A_node->msgID, 
                    MC_ATT_MESSAGE_ID,
                    &(A_node->dicomMsgID));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_UInt for Message ID failed", mcStatus);
        A_node->responseReceived = SAMP_TRUE;
        return(SAMP_FALSE);
    }
   
    A_node->responseReceived = SAMP_FALSE;
    A_node->failedResponse = SAMP_FALSE;
    A_node->imageSent = SAMP_TRUE;
    
    return ( SAMP_TRUE );
}


/****************************************************************************
 *
 *  Function    :   FreeList
 *
 *  Parameters  :   A_list     - Pointer to head of node list to free.
 *
 *  Returns     :   nothing
 *
 *  Description :   Free the memory allocated for a list of nodesransferred
 *
 ****************************************************************************/
static void FreeList( InstanceNode**    A_list )
{
    InstanceNode*    node;
    
    /*
     * Free the instance list
     */
    while (*A_list)
    {
        node = *A_list;
        *A_list = node->Next;
        
        if ( node->msgID != -1 )
            MC_Free_Message(&node->msgID);
        
        free( node );
    }
}


/****************************************************************************
 *
 *  Function    :   GetNumNodes
 *
 *  Parameters  :   A_list     - Pointer to head of node list to get count for
 *
 *  Returns     :   int, num node entries in list
 *
 *  Description :   Gets a count of the current list of instances.
 *
 ****************************************************************************/
static int GetNumNodes( InstanceNode*       A_list)

{
    int            numNodes = 0;
    InstanceNode*  node;

    node = A_list;
    while (node)
    {
        numNodes++;
        node = node->Next;
    }
    
    return numNodes;
} 


/****************************************************************************
 *
 *  Function    :   GetNumOutstandingRequests
 *
 *  Parameters  :   A_list     - Pointer to head of node list to get count for
 *
 *  Returns     :   int, num messages we're waiting for c-store responses for
 *
 *  Description :   Checks the list of instances sent over the association &
 *                  returns the number of responses we're waiting for.
 *
 ****************************************************************************/
static int GetNumOutstandingRequests(
                        InstanceNode*       A_list)

{
    int            outstandingResponseMsgs = 0;
    InstanceNode*  node;

    node = A_list;
    while (node)
    {
        if ( ( node->imageSent == SAMP_TRUE )
          && ( node->responseReceived == SAMP_FALSE ) )
            outstandingResponseMsgs++;
        node = node->Next;
    }
    return outstandingResponseMsgs;
}    


/****************************************************************************
 *
 *  Function    :   StorageCommitment
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Perform storage commitment for a set of storage objects.
 *                  The list was created as the objects themselves were
 *                  sent.
 *
 ****************************************************************************/
static SAMP_BOOLEAN StorageCommitment(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_appID, 
                        InstanceNode**      A_list)
{
    int           associationID = -1;
    int           calledApplicationID = -1;
    SAMP_BOOLEAN  sampStatus;
    MC_STATUS     mcStatus;

    if (!*A_list)
    {
        printf("No objects to commit.\n");
        return ( SAMP_TRUE );
    }

    /*
     * Open association to remote AE.  Instead of using the default service
     * list, use a special service list that only includes storage 
     * commitment.
     */
    mcStatus = MC_Open_Association( A_appID, &associationID,
                                    A_options->RemoteAE,
                                    A_options->RemotePort != -1 ? &A_options->RemotePort : NULL, 
                                    A_options->RemoteHostname[0] ? A_options->RemoteHostname : NULL,
                                    "Storage_Commit_SCU_Service_List" );
                                        
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n",A_options->RemoteAE);
        printf("\t%s\n", MC_Error_Message(mcStatus));
        return(SAMP_FALSE);
    }

    /*
     * Populate the N-ACTION message for storage commitment and sent it
     * over the network.  Also wait for a response message.
     */
    sampStatus = SetAndSendNActionMessage( A_options,
                                           associationID,
                                           A_list );
    if ( !sampStatus )
    {
        MC_Abort_Association(&associationID);
        return SAMP_FALSE;
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
    }
   

    if (A_options->Verbose)
        printf("Waiting for N-EVENT-REPORT Association\n");


    for(;;)
    {
        /*
         * Wait as an SCU for an association from the storage commitment
         * SCP.  This association will contain an N-EVENT-REPORT-RQ message.
         */
        mcStatus = MC_Wait_For_Association( "Storage_Commit_SCU_Service_List", 30,
                                            &calledApplicationID,
                                            &associationID);
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
            PrintError("Error on MC_Wait_For_Association for storage commitment",mcStatus);
            break;
        }
        
        /*
         * Handle the N-EVENT association.  We are only expecting a single
         * association, so quite after we've handled te association.
         */
        HandleNEventAssociation( A_options,
                                 associationID, 
                                 A_list );
        break;
    }

    return ( SAMP_TRUE );
}



/****************************************************************************
 *
 *  Function    :   SetAndSendNActionMessage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered 
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Populate an N-ACTION-RQ message to be sent, and wait
 *                  for a response to the request.
 *
 ****************************************************************************/
static SAMP_BOOLEAN SetAndSendNActionMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID,
                        InstanceNode**      A_list)

{
    MC_STATUS      mcStatus;
    int            messageID;
    int            itemID;
    InstanceNode*  node;
    int            responseMessageID;
    char*          transactionUID;
    char*          responseService;
    MC_COMMAND     responseCommand;
    int            responseStatus;


    mcStatus = MC_Open_Message( &messageID, "STORAGE_COMMITMENT_PUSH",
                                N_ACTION_RQ );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Error opening Storage Commitment message",mcStatus);
        return ( SAMP_FALSE );
    }

    /*
     * Set the well-known SOP instance for storage commitment Push, as
     * listed in DICOM PS3.4, J.3.5
     */
    mcStatus = MC_Set_Value_From_String( messageID, 
                                         MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                         "1.2.840.10008.1.20.1.1" );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for requested sop instance uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return ( SAMP_FALSE );
    }

    /*
     * Action ID as defined in DICOM PS3.4
     */
    mcStatus = MC_Set_Next_Value_From_Int( messageID,
                                           MC_ATT_ACTION_TYPE_ID,
                                           1 );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to set ItemID in n-action message", mcStatus);
        MC_Free_Message( &messageID );
        return ( SAMP_FALSE );
    }

    /*
     * Set the transaction UID.  Note that in a real storage commitment
     * application, this UID should be tracked and associated with the 
     * SOP instances asked for commitment with this request.  That way if 
     * multiple storage commitment requests are outstanding, and an 
     * N-EVENT-REPORT comes in, we can associate the message with the 
     * proper storage commitment request.  Commitment or commitment 
     * failure for specific objects can then be tracked.
     */
    transactionUID = Create_Inst_UID();
    mcStatus = MC_Set_Value_From_String( messageID, 
                                         MC_ATT_TRANSACTION_UID,
                                         transactionUID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for transaction uid failed",mcStatus);
        MC_Free_Message(&messageID);
        return ( SAMP_FALSE );
    }

    if (A_options->Verbose)
    {
        printf("\nSending N-Action with transaction UID: %s\n", transactionUID);
    }

    /*
     * Create an item for each SOP instance we are asking commitment for.
     * The item contains the SOP Class & Instance UIDs for the object.
     */
    node = *A_list;
    while (node)
    {
        mcStatus = MC_Open_Item( &itemID,
                                 "REF_SOP_MEDIA" );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }        

        /*
         * Set_Next_Value so that we can set multiple items within
         * the sequence attribute.
         */
        mcStatus = MC_Set_Next_Value_From_Int( messageID,
                                               MC_ATT_REFERENCED_SOP_SEQUENCE,
                                               itemID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set ItemID in n-action message", mcStatus);
            MC_Free_Item( &itemID );
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }
        
        mcStatus = MC_Set_Value_From_String( itemID,
                                             MC_ATT_REFERENCED_SOP_CLASS_UID,
                                             node->SOPClassUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set SOP Class UID in n-action message", mcStatus);
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }
        
        mcStatus = MC_Set_Value_From_String( itemID,
                                             MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                             node->SOPInstanceUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to set SOP Instance UID in n-action message", mcStatus);
            MC_Free_Message( &messageID );
            return ( SAMP_FALSE );
        }
        
        if (A_options->Verbose)
        {
            printf("   Object SOP Class UID: %s\n", node->SOPClassUID );
            printf("Object SOP Instance UID: %s\n\n", node->SOPInstanceUID );
        }

        node = node->Next;
    }
    

    /*
     * Once the message has been built, we are then able to perform the
     * N-ACTION-RQ on it.
     */
    mcStatus = MC_Send_Request_Message( A_associationID, messageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to send N-ACTION-RQ message",mcStatus);
        MC_Free_Message( &messageID );
        return ( SAMP_FALSE );
    }

    /*
     * After sending the message, we free it.
     */
    mcStatus = MC_Free_Message( &messageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to free N-ACTION-RQ message",mcStatus);
        return ( SAMP_FALSE );
    }
    

    /*
     *  Wait for N-ACTION-RSP message.
     */
    mcStatus = MC_Read_Message(A_associationID, 30, &responseMessageID,
                 &responseService, &responseCommand);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed for N-ACTION-RSP", mcStatus);
        return ( SAMP_FALSE );
    }

    /*
     * Check the status in the response message.
     */
    mcStatus = MC_Get_Value_To_Int(responseMessageID, MC_ATT_STATUS, &responseStatus);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_Int for N-ACTION-RSP status failed",mcStatus);
        MC_Free_Message(&responseMessageID);
        return ( SAMP_FALSE );
    }

    switch (responseStatus)
    {
        case N_ACTION_NO_SUCH_SOP_INSTANCE:
            printf("N-ACTION-RSP failed because of invalid UID\n");
            MC_Free_Message(&responseMessageID);
            return ( SAMP_FALSE );
        case N_ACTION_PROCESSING_FAILURE:
            printf("N-ACTION-RSP failed because of processing failure\n");
            MC_Free_Message(&responseMessageID);
            return ( SAMP_FALSE );
            
        case N_ACTION_SUCCESS:
            break;
            
        default:
            printf("N-ACTION-RSP failure, status=0x%x\n",responseStatus);
            MC_Free_Message(&responseMessageID);
            return ( SAMP_FALSE );
    }

    mcStatus = MC_Free_Message( &responseMessageID );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        PrintError("Unable to free N-ACTION-RSP message",mcStatus);
        return ( SAMP_FALSE );
    }

    
    return( SAMP_TRUE );

} /* End of SetAndSendNActionMessage */



/****************************************************************************
 *
 *  Function    :   HandleNEventAssociation
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_associationID - Association ID registered 
 *                  A_list     - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Handle a storage commitment association when expecting 
 *                  an N-EVENT-REPORT-RQ message.  In a typical DICOM 
 *                  application this may be handled in a child process or
 *                  a seperate thread from where the MC_Wait_For_Association
 *                  call is made.
 *
 ****************************************************************************/
static SAMP_BOOLEAN HandleNEventAssociation(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID, 
                        InstanceNode**      A_list)
{
    MC_STATUS     mcStatus;
    SAMP_BOOLEAN  sampStatus;
    RESP_STATUS   respStatus;
    int           messageID;
    int           rspMessageID;
    MC_COMMAND    command;
    char*         serviceName;

    if (A_options->Verbose)
        printf("Accepting N-EVENT association\n");

    /*
     * Accept the association.
     */
    mcStatus = MC_Accept_Association( A_associationID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * Make sure the association is cleaned up.
         */
        MC_Reject_Association( A_associationID,
                               TRANSIENT_NO_REASON_GIVEN );
        PrintError("Error on MC_Accept_Association", mcStatus);
        return SAMP_FALSE;
    }

    for (;;)
    {
        /*
         * Note, only the requestor of an association can close the association.
         * So, we wait here in the read message call after we have received
         * the N-EVENT-REPORT for the connection to close.
         */
        mcStatus = MC_Read_Message( A_associationID, 
                                    30, 
                                    &messageID,
                                    &serviceName, 
                                    &command);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            if (mcStatus == MC_TIMEOUT)
            {
                printf("Timeout occured waiting for message.  Waiting again.\n");
                continue;
            }
            else if (mcStatus == MC_ASSOCIATION_CLOSED)
            {       
                printf("Association Closed.\n");
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
                PrintError("Unexpected event, association aborted", mcStatus);
                break;
            }
            
            PrintError("Error on MC_Read_Message", mcStatus);
            MC_Abort_Association(&A_associationID);
            break;
        }

        sampStatus = ProcessNEventMessage(A_options, messageID, A_list);
        if (sampStatus == SAMP_TRUE )
            respStatus = N_EVENT_SUCCESS; 
        else
            respStatus = N_EVENT_PROCESSING_FAILURE; 


        mcStatus = MC_Free_Message(&messageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message of PRINTER,N_EVENT_REPORT_RSP error",mcStatus);
            return ( SAMP_FALSE ); 
        } 

        /*
         * Now lests send a response message.
         */
        mcStatus = MC_Open_Message ( &rspMessageID, 
                                     "STORAGE_COMMITMENT_PUSH", 
                                     N_EVENT_REPORT_RSP );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error of N-EVENT response",mcStatus);
            return ( SAMP_FALSE ); 
        }

        mcStatus = MC_Send_Response_Message( A_associationID,  
                                             respStatus, 
                                             rspMessageID );
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message for N_EVENT_REPORT_RSP error",mcStatus);
            MC_Free_Message(&rspMessageID);
            return( SAMP_FALSE ); 
        }

        mcStatus = MC_Free_Message(&rspMessageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message of PRINTER,N_EVENT_REPORT_RSP error",mcStatus);
            return ( SAMP_FALSE ); 
        } 

    }

    return ( SAMP_TRUE );
}


/****************************************************************************
 *
 *  Function    :   ProcessNEventMessage
 *
 *  Parameters  :   A_options   - Pointer to structure containing input
 *                                parameters to the application
 *                  A_messageID - Association ID registered 
 *                  A_list      - List of objects to request commitment for.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Handle a storage commitment association when expecting 
 *                  an N-EVENT-REPORT-RQ message.  In a typical DICOM 
 *                  application this may be handled in a child process or
 *                  a seperate thread from where the MC_Wait_For_Association
 *                  call is made.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ProcessNEventMessage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_messageID, 
                        InstanceNode**      A_list)
{
    char          uidBuffer[UI_LENGTH+2];
    char          sopClassUID[UI_LENGTH+2];
    char          sopInstanceUID[UI_LENGTH+2];
    unsigned short eventType;
    MC_STATUS     mcStatus;
    int           itemID;


    mcStatus = MC_Get_Value_To_String( A_messageID, 
                    MC_ATT_TRANSACTION_UID, 
                    sizeof(uidBuffer), uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to retreive transaction UID", mcStatus);
        uidBuffer[0] = '\0';
        return SAMP_FALSE;
    }

    /*
     * At this time, the transaction UID should be linked to a 
     * transaction UID of a previous storage commitment request.
     * The following code can then compare the successful SOP 
     * instances and failed SOP instances with those that were
     * requested.
     */
    mcStatus = MC_Get_Value_To_UShortInt( A_messageID, 
                                          MC_ATT_EVENT_TYPE_ID, 
                                          &eventType );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to retreive event type ID", mcStatus);
        eventType = 0;
    }

    switch( eventType )
    {
        case 1: /* SUCCESS */
            printf ( "\nN-EVENT Transaction UID: %s is SUCCESS\n", uidBuffer );
            
            break;
        case 2: /* FAILURE */
            printf ( "\nN-EVENT Transaction UID: %s is FAILURE\n", uidBuffer );
            /*
             * At this point, the failure list is traversed through
             * to determine which images failed for the transaction.
             * This should be compared to the originals.
             */
            if (A_options->Verbose)
                printf("    Failed to commit SOP Instances:\n");

            mcStatus = MC_Get_Next_Value_To_Int( A_messageID,
                                                 MC_ATT_FAILED_SOP_SEQUENCE,
                                                 &itemID );
            while ( mcStatus == MC_NORMAL_COMPLETION )
            {
                mcStatus = MC_Get_Value_To_String( itemID,
                                                   MC_ATT_REFERENCED_SOP_CLASS_UID,
                                                   sizeof(sopClassUID),
                                                   sopClassUID );
                if ( mcStatus != MC_NORMAL_COMPLETION )
                {
                    PrintError("Unable to get SOP Class UID in n-event-rq message", mcStatus);
                    sopClassUID[0] = '\0';
                }

                mcStatus = MC_Get_Value_To_String( itemID,
                                                   MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                                   sizeof(sopInstanceUID),
                                                   sopInstanceUID );
                if ( mcStatus != MC_NORMAL_COMPLETION )
                {
                    PrintError("Unable to get SOP Instance UID in n-event-rq message", mcStatus);
                    sopInstanceUID[0] = '\0';
            }

                if (A_options->Verbose)
                {
                    printf("       SOP Class UID: %s\n", sopClassUID );
                    printf("    SOP Instance UID: %s\n\n", sopInstanceUID );
                }

                mcStatus = MC_Get_Next_Value_To_Int( A_messageID,
                                                     MC_ATT_FAILED_SOP_SEQUENCE,
                                                     &itemID );
            }

            break;
        default:
            printf( "Transaction UID: %s event_report invalid event type %d\n",
                    uidBuffer, eventType );
    }

        
    /*
     * We should be comparing here the original SOP instances in the 
     * original transaction to what was returned here.
     */
    if (A_options->Verbose)
        printf("    Successfully commited SOP Instances:\n");

    mcStatus = MC_Get_Next_Value_To_Int( A_messageID,
                                         MC_ATT_REFERENCED_SOP_SEQUENCE,
                                         &itemID );
    while ( mcStatus == MC_NORMAL_COMPLETION )
    {
        mcStatus = MC_Get_Value_To_String( itemID,
                                           MC_ATT_REFERENCED_SOP_CLASS_UID,
                                           sizeof(sopClassUID),
                                           sopClassUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to get SOP Class UID in n-event-rq message", mcStatus);
            sopClassUID[0] = '\0';
        }

        mcStatus = MC_Get_Value_To_String( itemID,
                                           MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                           sizeof(sopInstanceUID),
                                           sopInstanceUID );
        if ( mcStatus != MC_NORMAL_COMPLETION )
        {
            PrintError("Unable to get SOP Instance UID in n-event-rq message", mcStatus);
            sopInstanceUID[0] = '\0';
        }

        if (A_options->Verbose)
        {
            printf("       SOP Class UID: %s\n", sopClassUID );
            printf("    SOP Instance UID: %s\n\n", sopInstanceUID );
        }


        mcStatus = MC_Get_Next_Value_To_Int( A_messageID,
                                             MC_ATT_REFERENCED_SOP_SEQUENCE,
                                             &itemID );
    }
 
         
    return ( SAMP_TRUE );
}


/****************************************************************************
 *
 *  Function    :   ReadImage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_node     - The node in our list of instances
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Determine the format of a DICOM file and read it into
 *                  memory.  Note that in a production application, the 
 *                  file format should be predetermined (and not have to be
 *                  "guessed" by the CheckFileFormat function).  The 
 *                  format for this application was chosen to show how both
 *                  DICOM Part 10 format files and "stream" format objects
 *                  can be sent over the network.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadImage(
                              STORAGE_OPTIONS*  A_options,
                              int               A_appID, 
                              InstanceNode*     A_node)
{
    FORMAT_ENUM             format = UNKNOWN_FORMAT;
    SAMP_BOOLEAN            sampBool = SAMP_FALSE;
    MC_STATUS               mcStatus;


    format = CheckFileFormat( A_node->fname );
    switch(format)
    {
        case MEDIA_FORMAT:
            A_node->mediaFormat = SAMP_TRUE;
            sampBool = ReadFileFromMedia( A_options, 
                                          A_appID, 
                                          A_node->fname, 
                                          &A_node->msgID, 
                                          &A_node->transferSyntax, 
                                          &A_node->imageBytes );
            break;
                
        case IMPLICIT_LITTLE_ENDIAN_FORMAT:
        case IMPLICIT_BIG_ENDIAN_FORMAT:
        case EXPLICIT_LITTLE_ENDIAN_FORMAT:
        case EXPLICIT_BIG_ENDIAN_FORMAT:
            A_node->mediaFormat = SAMP_FALSE;
            sampBool = ReadMessageFromFile( A_options, 
                                            A_node->fname, 
                                            format, 
                                            &A_node->msgID, 
                                            &A_node->transferSyntax, 
                                            &A_node->imageBytes );
            break;
            
        case UNKNOWN_FORMAT:
            PrintError("Unable to determine the format of file",
                       MC_NORMAL_COMPLETION);
            sampBool = SAMP_FALSE;
            break;
    }
    if ( sampBool == SAMP_TRUE )
    {
        mcStatus = MC_Get_Value_To_String(A_node->msgID, 
                        MC_ATT_SOP_CLASS_UID,
                        sizeof(A_node->SOPClassUID),
                        A_node->SOPClassUID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Get_Value_To_String for SOP Class UID failed", mcStatus);
        }

        mcStatus = MC_Get_Value_To_String(A_node->msgID, 
                        MC_ATT_SOP_INSTANCE_UID,
                        sizeof(A_node->SOPInstanceUID),
                        A_node->SOPInstanceUID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Get_Value_To_String for SOP Instance UID failed", mcStatus);
        }
    }

    return sampBool;
}    


/****************************************************************************
 *
 *  Function    :   SendImage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_filename - Name of file to open
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *                  A_syntax   - The transfer syntax the original image was
 *                               encoded as.  This currently is not used,
 *                               but could be used if encapsulated/compressed
 *                               transfer syntaxes are supported.
 *                  A_imageBytes-Size in bytes of the image being sent.  Used
 *                               for display purposes.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE on failure where association must be aborted
 *
 *  Description :   Determine the format of a DICOM file and read it into
 *                  memory.  Note that in a production application, the 
 *                  file format should be predetermined (and not have to be
 *                  "guessed" by the CheckFileFormat function).  The 
 *                  format for this application was chosen to show how both
 *                  DICOM Part 10 format files and "stream" format objects
 *                  can be sent over the network.
 *
 *                  SAMP_TRUE is returned on success, or when a recoverable
 *                  error occurs.
 *
 ****************************************************************************/
static SAMP_BOOLEAN SendImage(STORAGE_OPTIONS*  A_options,
                              int               A_associationID, 
                              InstanceNode*     A_node)
{
    MC_STATUS       mcStatus;

    A_node->imageSent = SAMP_FALSE;
    
    /* Get the SOP class UID and set the service */
    mcStatus = MC_Get_MergeCOM_Service(A_node->SOPClassUID, A_node->serviceName, sizeof(A_node->serviceName));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_MergeCOM_Service failed", mcStatus);
        return ( SAMP_TRUE );
    }            
                 
    mcStatus = MC_Set_Service_Command(A_node->msgID, A_node->serviceName, C_STORE_RQ);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Service_Command failed", mcStatus);
        return ( SAMP_TRUE );
    }
            
    /* set affected SOP Instance UID */
    mcStatus = MC_Set_Value_From_String(A_node->msgID, 
                      MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                      A_node->SOPInstanceUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed for affected SOP Instance UID", mcStatus);
        return ( SAMP_TRUE );
    }
            
    /*
     *  Send the message
     */
    if (A_options->Verbose)
    {
        printf("     File: %s\n", A_node->fname);
        if ( A_node->mediaFormat )
        printf("   Format: DICOM Part 10 Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));
        else
        printf("   Format: Stream Format(%s)\n", GetSyntaxDescription(A_node->transferSyntax));
        printf("SOP Class: %s (%s)\n", A_node->SOPClassUID, A_node->serviceName);
        printf("      UID: %s\n", A_node->SOPInstanceUID);
        printf("     Size: %lu bytes\n", A_node->imageBytes);
    }

    mcStatus = MC_Send_Request_Message(A_associationID, A_node->msgID);
    if (mcStatus == MC_ASSOCIATION_ABORTED
     || mcStatus == MC_SYSTEM_ERROR)
    {
        /*
         * At this point, the association has been dropped, or we should
         * drop it in the case of MC_SYSTEM_ERROR.
         */
        PrintError("MC_Send_Request_Message failed", mcStatus);
        return ( SAMP_FALSE );
    }
    else if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * This is a failure condition we can continue with
         */
        PrintError("Warning: MC_Send_Request_Message failed", mcStatus);
        return ( SAMP_TRUE );
    }
    
    A_node->imageSent = SAMP_TRUE;

    return ( SAMP_TRUE );
}    


/****************************************************************************
 *
 *  Function    :   ReadResponseMessages
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_imageBytes-Size in bytes of the image being sent.  Used
 *                               for display purposes.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE on failure where association must be aborted
 *
 *  Description :   Determine the format of a DICOM file and read it into
 *                  memory.  Note that in a production application, the 
 *                  file format should be predetermined (and not have to be
 *                  "guessed" by the CheckFileFormat function).  The 
 *                  format for this application was chosen to show how both
 *                  DICOM Part 10 format files and "stream" format objects
 *                  can be sent over the network.
 *
 *                  SAMP_TRUE is returned on success, or when a recoverable
 *                  error occurs.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadResponseMessages(STORAGE_OPTIONS*  A_options,
                              int               A_associationID, 
                              int               A_timeout,
                              InstanceNode**    A_list)
{
    MC_STATUS       mcStatus;
    SAMP_BOOLEAN    sampBool;
    int             responseMessageID;
    char*           responseService;
    MC_COMMAND      responseCommand;
    static char     affectedSOPinstance[UI_LENGTH+2];
    unsigned int    dicomMsgID;
    InstanceNode*   node;
    
    /*
     *  Wait for response
     */
    mcStatus = MC_Read_Message(A_associationID, A_timeout, &responseMessageID,
                 &responseService, &responseCommand);
    if (mcStatus == MC_TIMEOUT)
        return ( SAMP_TRUE );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed", mcStatus);
        return ( SAMP_FALSE );
    }
    
    mcStatus = MC_Get_Value_To_UInt(responseMessageID, 
                    MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO,
                    &dicomMsgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_UInt for Message ID Being Responded To failed.  Unable to process response message.", mcStatus);
        return(SAMP_TRUE);
    }
    
    node = *A_list;
    while (node)
    {
        if ( node->dicomMsgID == dicomMsgID )
        {
            break;
        }
        node = node->Next;
    }
   
    if ( !node )
    {
        printf( "Message ID Being Responded To tag does not match message sent over association: %d\n", dicomMsgID );
        MC_Free_Message(&responseMessageID);
        return ( SAMP_TRUE );
    }
   
    node->responseReceived = SAMP_TRUE;
        
    sampBool = CheckResponseMessage ( responseMessageID, &node->status, node->statusMeaning, sizeof(node->statusMeaning) );
    if (!sampBool)
    {
        node->failedResponse = SAMP_TRUE;
    }
    
    if ( ( A_options->Verbose ) || ( node->status != C_STORE_SUCCESS ) )
        printf("   Status: %s\n", node->statusMeaning);
        
    node->failedResponse = SAMP_FALSE;

    mcStatus = MC_Free_Message(&responseMessageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for response message", mcStatus);
        return ( SAMP_TRUE );
    }

    return ( SAMP_TRUE );
}    


/****************************************************************************
 *
 *  Function    :   CheckResponseMessage
 *
 *  Parameters  :   A_responseMsgID  - The message ID of the response message
 *                                     for which we want to check the status
 *                                     tag.
 *
 *  Returns     :   SAMP_TRUE on success or warning status
 *                  SAMP_FALSE on failure status
 *
 *  Description :   Examine the status tag in the response to see if we
 *                  the C-STORE-RQ was successfully received by the SCP.
 *
 ****************************************************************************/
static SAMP_BOOLEAN CheckResponseMessage ( int A_responseMsgID, unsigned int* A_status, char* A_statusMeaning, size_t A_statusMeaningLength )
{
    MC_STATUS mcStatus;
    SAMP_BOOLEAN returnBool = SAMP_TRUE;

    mcStatus = MC_Get_Value_To_UInt ( A_responseMsgID,
                                      MC_ATT_STATUS,
                                      A_status );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        /* Problem with MC_Get_Value_To_UInt */
        PrintError ( "MC_Get_Value_To_UInt for response status failed", mcStatus );
        strncpy( A_statusMeaning, "Unknown Status", A_statusMeaningLength );
        return SAMP_FALSE;
    }

    /* MC_Get_Value_To_UInt worked.  Check the response status */

    switch ( *A_status )
    {
        /* Success! */
        case C_STORE_SUCCESS:
            strncpy( A_statusMeaning, "C-STORE Success.", A_statusMeaningLength );
            break;
            
        /* Warnings.  Continue execution. */

        case C_STORE_WARNING_ELEMENT_COERCION:
            strncpy( A_statusMeaning, "Warning: Element Coersion... Continuing.", A_statusMeaningLength );
            break;

        case C_STORE_WARNING_INVALID_DATASET:
            strncpy( A_statusMeaning, "Warning: Invalid Dataset... Continuing.", A_statusMeaningLength );
            break;

        case C_STORE_WARNING_ELEMENTS_DISCARDED:
            strncpy( A_statusMeaning, "Warning: Elements Discarded... Continuing.", A_statusMeaningLength ); 
            break;

        /* Errors.  Abort execution. */

        case C_STORE_FAILURE_REFUSED_NO_RESOURCES:
            strncpy( A_statusMeaning, "ERROR: REFUSED, NO RESOURCES.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        case C_STORE_FAILURE_INVALID_DATASET:
            strncpy( A_statusMeaning, "ERROR: INVALID_DATASET.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        case C_STORE_FAILURE_CANNOT_UNDERSTAND:
            strncpy( A_statusMeaning, "ERROR: CANNOT UNDERSTAND.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;

        case C_STORE_FAILURE_PROCESSING_FAILURE:
            strncpy( A_statusMeaning, "ERROR: PROCESSING FAILURE.  ASSOCIATION ABORTING.", A_statusMeaningLength );
            returnBool = SAMP_FALSE;
            break;
            
        default:
            sprintf( A_statusMeaning, "Warning: Unknown status (0x%04x)... Continuing.", *A_status );
            returnBool = SAMP_FALSE;
            break;
    }

    return returnBool;
}


/****************************************************************************
 *
 *  Function    :   ReadFileFromMedia
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_filename - Name of file to open
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *                  A_syntax   - The transfer syntax the message was encoded
 *                               in is returned here.
 *                  A_bytesRead- Total number of bytes read in image.  Used
 *                               only for display and to calculate the
 *                               transfer rate.
 *
 *  Returns     :   SAMP_TRUE on success
 *                  SAMP_FALSE on failure to read the object
 *
 *  Description :   This function reads a file in the DICOM Part 10 (media)
 *                  file format.  Before returning, it determines the
 *                  transfer syntax the file was encoded as, and converts
 *                  the file into the tool kit's "message" file format
 *                  for use in the network routines.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadFileFromMedia( 
                              STORAGE_OPTIONS*  A_options,
                              int               A_appID,
                              char*             A_filename,
                              int*              A_msgID,
                              TRANSFER_SYNTAX*  A_syntax,
                              unsigned long*    A_bytesRead )
{
    CBinfo        callbackInfo;
    MC_STATUS   mcStatus;
    char        transferSyntaxUID[UI_LENGTH+2];

    if (A_options->Verbose)
    {
        printf("Reading %s in DICOM Part 10 format\n", A_filename);
    }

    /*
     * Create new File object 
     */
    mcStatus = MC_Create_Empty_File(A_msgID, A_filename);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to create file object",mcStatus);
        return( SAMP_FALSE );
    }


    /*
     * Read the file off of disk
     */
    mcStatus = MC_Open_File(A_appID,
                           *A_msgID,
                            &callbackInfo,
                            MediaToFileObj);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        if (callbackInfo.fp)
            fclose(callbackInfo.fp);
        PrintError("MC_Open_File failed, unable to read file from media", mcStatus);
        MC_Free_File(A_msgID);
        return( SAMP_FALSE );
    }
    
    if (callbackInfo.fp)
        fclose(callbackInfo.fp);

    *A_bytesRead = callbackInfo.bytesRead;
    
    /*
     * Get the transfer syntax UID from the file to determine if the object
     * is encoded in a compressed transfer syntax.  IE, one of the JPEG or
     * the RLE transfer syntaxes.  If we've specified on the command line
     * that we are supporting encapsulated/compressed transfer syntaxes,
     * go ahead an use the object, if not, reject it and return failure.
     *
     * Note that if encapsulated transfer syntaxes are supported, the 
     * services lists in the mergecom.app file must be expanded using 
     * transfer syntax lists to contain the JPEG syntaxes supported. 
     * Also, the transfer syntaxes negotiated for each service should be 
     * saved (as retrieved by the MC_Get_First/Next_Acceptable service
     * calls) to match with the actual syntax of the object.  If they do
     * not match the encoding of the pixel data may have to be modified
     * before the file is sent over the wire.
     */
    mcStatus = MC_Get_Value_To_String(*A_msgID, 
                            MC_ATT_TRANSFER_SYNTAX_UID,
                            sizeof(transferSyntaxUID),
                            transferSyntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String failed for transfer syntax UID",
                   mcStatus);
        MC_Free_File(A_msgID);
        return SAMP_FALSE;
    }

    mcStatus = MC_Get_Enum_From_Transfer_Syntax(
                transferSyntaxUID,
                A_syntax);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Invalid transfer syntax UID contained in the file: %s\n",
               transferSyntaxUID);
        MC_Free_File(A_msgID);
        return SAMP_FALSE; 
    } 

    /*
     * If we don't handle encapsulated transfer syntaxes, let's check the
     * image transfer syntax to be sure it is not encoded as an encapsulated
     * transfer syntax.
     */
    if (!A_options->HandleEncapsulated)
    {
        switch (*A_syntax)
        {
            case IMPLICIT_LITTLE_ENDIAN:
            case EXPLICIT_LITTLE_ENDIAN:
            case EXPLICIT_BIG_ENDIAN:
            case IMPLICIT_BIG_ENDIAN:
            case DEFLATED_EXPLICIT_LITTLE_ENDIAN:
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
                printf("Warning: Encapsulated transfer syntax (%s) image specified\n", 
                       GetSyntaxDescription(*A_syntax));
                printf("         Not sending image.\n");
                MC_Free_File(A_msgID);
                return SAMP_FALSE; 
            case INVALID_TRANSFER_SYNTAX:
                printf("Warning: Invalid transfer syntax (%s) specified\n", 
                       GetSyntaxDescription(*A_syntax));
                printf("         Not sending image.\n");
                MC_Free_File(A_msgID);
                return SAMP_FALSE; 
        }
    }


    if (A_options->Verbose)
        printf("Reading DICOM Part 10 format file in %s: %s\n", 
               GetSyntaxDescription(*A_syntax),
               A_filename);
   
    /*
     * Convert the "file" object into a "message" object.  This is done
     * because the MC_Send_Request_Message call requires that the object
     * be a "message" object.  Any of the tags in the message can be
     * accessed when the object is a "file" object.  
     */
    mcStatus = MC_File_To_Message( *A_msgID );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to convert file object to message object", mcStatus);
        MC_Free_File(A_msgID);
        return( SAMP_FALSE );
    }
        
    return SAMP_TRUE;
    
} /* ReadFileFromMedia() */


/****************************************************************************
 *
 *  Function    :   ReadMessageFromFile
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_filename - Name of file to open
 *                  A_format   - Enum containing the format of the object
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *                  A_syntax   - The transfer syntax read is returned here.
 *                  A_bytesRead- Total number of bytes read in image.  Used
 *                               only for display and to calculate the
 *                               transfer rate.
 *
 *  Returns     :   SAMP_TRUE  on success
 *                  SAMP_FALSE on failure to open the file
 *
 *  Description :   This function reads a file in the DICOM "stream" format.
 *                  This format contains no DICOM part 10 header information.
 *                  The transfer syntax of the object is contained in the 
 *                  A_format parameter.  
 *
 *                  When this function returns failure, the caller should
 *                  not do any cleanup, A_msgID will not contain a valid
 *                  message ID.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadMessageFromFile( 
                              STORAGE_OPTIONS*  A_options,
                              char*             A_filename,
                              FORMAT_ENUM       A_format,
                              int*              A_msgID,
                              TRANSFER_SYNTAX*  A_syntax,
                              unsigned long*    A_bytesRead ) 
{
    MC_STATUS               mcStatus;
    unsigned long           errorTag;
    CBinfo                  callbackInfo;  

    /*
     * Determine the format
     */
    switch( A_format )
    {
        case IMPLICIT_LITTLE_ENDIAN_FORMAT: 
            *A_syntax = IMPLICIT_LITTLE_ENDIAN; 
            if (A_options->Verbose)
                printf("Reading DICOM \"stream\" format file in %s: %s\n", 
                       GetSyntaxDescription(*A_syntax),
                       A_filename);
            break;
        case IMPLICIT_BIG_ENDIAN_FORMAT:    
            *A_syntax = IMPLICIT_BIG_ENDIAN; 
            if (A_options->Verbose)
                printf("Reading DICOM \"stream\" format file in %s: %s\n", 
                       GetSyntaxDescription(*A_syntax),
                       A_filename);
            break;
        case EXPLICIT_LITTLE_ENDIAN_FORMAT: 
            *A_syntax = EXPLICIT_LITTLE_ENDIAN; 
            if (A_options->Verbose)
                printf("Reading DICOM \"stream\" format file in %s: %s\n", 
                       GetSyntaxDescription(*A_syntax),
                       A_filename);
            break;
        case EXPLICIT_BIG_ENDIAN_FORMAT:    
            *A_syntax = EXPLICIT_BIG_ENDIAN; 
            if (A_options->Verbose)
                printf("Reading DICOM \"stream\" format file in %s: %s\n", 
                       GetSyntaxDescription(*A_syntax),
                       A_filename);
            break;
        default: 
            return SAMP_FALSE;
    }

    /*
     * Open an empty message object to load the image into
     */
    mcStatus = MC_Open_Empty_Message(A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open empty message", mcStatus);
        return SAMP_FALSE;
    }

    /*
     * Open and stream message from file
     */
    callbackInfo.fp = fopen(A_filename, BINARY_READ);

    if (!callbackInfo.fp)
    {
        printf("ERROR: Unable to open %s.\n", A_filename);
        MC_Free_Message(A_msgID);
        return SAMP_FALSE;
    }

    mcStatus = MC_Stream_To_Message(*A_msgID,
                                    MC_ATT_GROUP_0008_LENGTH, 
                                    0xffffFFFF,
                                    *A_syntax,
                                    &errorTag,
                                    (void*) &callbackInfo, /* data for StreamToMsgObj */
                                    StreamToMsgObj);

    if (callbackInfo.fp)
        fclose(callbackInfo.fp);

    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Stream_To_Message error, possible wrong transfer syntax guessed",
            mcStatus);
        MC_Free_Message(A_msgID);
        return SAMP_FALSE;
    }         

    *A_bytesRead = callbackInfo.bytesRead;

    return SAMP_TRUE;

} /* ReadMessageFromFile() */


/****************************************************************************
 *
 *  Function    :   MediaToFileObj
 *
 *  Parameters  :   A_fileName   - Filename to open for reading
 *                  A_userInfo   - Pointer to an object used to preserve
 *                                 data between calls to this function.
 *                  A_dataSize   - Number of bytes read
 *                  A_dataBuffer - Pointer to buffer of data read
 *                  A_isFirst    - Set to non-zero value on first call
 *                  A_isLast     - Set to 1 when file has been completely 
 *                                 read
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success
 *                  any other MC_STATUS value on failure.
 *
 *  Description :   Callback function used by MC_Open_File to read a file
 *                  in the DICOM Part 10 (media) format.
 *
 ****************************************************************************/
static MC_STATUS MediaToFileObj( char*     A_filename,
                                 void*     A_userInfo,
                                 int*      A_dataSize,
                                 void**    A_dataBuffer,
                                 int       A_isFirst,
                                 int*      A_isLast)
{

    CBinfo*         callbackInfo = (CBinfo*)A_userInfo;
    size_t          bytes_read;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
        callbackInfo->fp = fopen(A_filename, BINARY_READ);
    }
    
    if (!callbackInfo->fp)
       return MC_CANNOT_COMPLY;

    bytes_read = fread(callbackInfo->buffer, 1, sizeof(callbackInfo->buffer),
                       callbackInfo->fp);
    if (ferror(callbackInfo->fp))
        return MC_CANNOT_COMPLY;

    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytes_read;
    callbackInfo->bytesRead += bytes_read;
    return MC_NORMAL_COMPLETION;
    
} /* MediaToFileObj() */


/*************************************************************************
 *
 *  Function    :  StreamToMsgObj
 *
 *  Parameters  :  A_msgID         - Message ID of message being read
 *                 A_CBinformation - user information passwd to callback
 *                 A_isFirst       - flag to tell if this is the first call
 *                 A_dataSize      - length of data read
 *                 A_dataBuffer    - buffer where read data is stored
 *                 A_isLast        - flag to tell if this is the last call
 *
 *  Returns     :  MC_NORMAL_COMPLETION on success
 *                 any other return value on failure.
 *
 *  Description :  Reads input stream data from a file, and places the data
 *                 into buffer to be used by the MC_Stream_To_Message function.
 *
 **************************************************************************/
static MC_STATUS StreamToMsgObj( int        A_msgID,
                                 void*      A_CBinformation,
                                 int        A_isFirst,
                                 int*       A_dataSize,
                                 void**     A_dataBuffer,
                                 int*       A_isLast)
{
    size_t          bytesRead;
    CBinfo*         callbackInfo = (CBinfo*)A_CBinformation;

    if (A_isFirst)
        callbackInfo->bytesRead = 0L;
        
    bytesRead = fread(callbackInfo->buffer, 1,
                      sizeof(callbackInfo->buffer),
                      callbackInfo->fp);
    if (ferror(callbackInfo->fp))
    {
        perror("\tRead error when streaming message from file.\n");
        return MC_CANNOT_COMPLY;
    }

    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytesRead;

    callbackInfo->bytesRead += bytesRead;

    return MC_NORMAL_COMPLETION;
} /* StreamToMsgObj() */


/****************************************************************************
 *
 *  Function    :   CheckValidVR
 *
 *  Parameters  :   A_VR - string to check for valid VR.
 *
 *  Returns     :   SAMP_BOOLEAN
 *
 *  Description :   Check to see if this char* is a valid VR.  This function
 *                  is only used by CheckFileFormat.
 *
 ****************************************************************************/
static SAMP_BOOLEAN CheckValidVR( char    *A_VR)
{
    static const char* const VR_Table[27] = 
    {   
        "AE", "AS", "CS", "DA", "DS", "DT", "IS", "LO", "LT", 
        "PN", "SH", "ST", "TM", "UT", "UI", "SS", "US", "AT", 
        "SL", "UL", "FL", "FD", "UN", "OB", "OW", "OL", "SQ" 
    };
    int i;
   
    for (i =  0; i < 27; i++)
    {
        if ( !strcmp( A_VR, VR_Table[i] ) )
            return SAMP_TRUE;
    }

    return SAMP_FALSE;
} /* CheckValidVR() */


/****************************************************************************
 *
 *  Function    :    CheckFileFormat
 *
 *  Parameters  :    Afilename      file name of the image which is being
 *                                  checked for a format.
 *
 *  Returns     :    FORMAT_ENUM    enumberation of possible return values
 *
 *  Description :    Tries to determing the messages transfer syntax.  
 *                   This function is not fool proof!  It is mainly 
 *                   useful for testing puposes, and as an example as
 *                   to how to determine an images format.  This code
 *                   should probably not be used in production equipment,
 *                   unless the format of objects is known ahead of time,
 *                   and it is guarenteed that this algorithm works on those
 *                   objects.
 *
 ****************************************************************************/
static FORMAT_ENUM CheckFileFormat( char*    A_filename )
{
    FILE*            fp;
    char             signature[5] = "\0\0\0\0\0";
    char             vR[3] = "\0\0\0";
    
    union
    {
        unsigned short   groupNumber;
        char      i[2];
    } group;
    
    unsigned short   elementNumber;
    
    union
    {
        unsigned short  shorterValueLength;
        char      i[2];
    } aint;
    
    union
    {
        unsigned long   valueLength;
        char      l[4];
    } along;
   
   

    if ( (fp = fopen(A_filename, BINARY_READ)) != NULL)
    {
        if (fseek(fp, 128, SEEK_SET) == 0)
        {
            /*
             * Read the signature, only 4 bytes
             */
            if (fread(signature, 1, 4, fp) == 4)
            {
                /* 
                 * if it is the signature, return true.  The file is 
                 * definately in the DICOM Part 10 format.
                 */
                if (!strcmp(signature, "DICM"))
                {
                    fclose(fp);
                    return MEDIA_FORMAT;
                }
            }
        }
        
        fseek(fp, 0, SEEK_SET);
        
        /*
         * Now try and determine the format if it is not media
         */
        if (fread(&group.groupNumber, 1, sizeof(group.groupNumber), fp) != 
            sizeof(group.groupNumber))
        {
            printf("ERROR: reading Group Number\n");
            return UNKNOWN_FORMAT;
        }
        if (fread(&elementNumber, 1, sizeof(elementNumber), fp) !=
            sizeof(elementNumber))
        {
            printf("ERROR: reading Element Number\n");
            return UNKNOWN_FORMAT;
        }

        if (fread(vR, 1, 2, fp) != 2)
        {
            printf("ERROR: reading VR\n");
            return UNKNOWN_FORMAT;
        }

        /*
         * See if this is a valid VR, if not then this is implicit VR
         */
        if (CheckValidVR(vR))
        {
            /*
             * we know that this is an explicit endian, but we don't
             *  know which endian yet.
             */
            if (!strcmp(vR, "OB") 
             || !strcmp(vR, "OW") 
             || !strcmp(vR, "OL") 
             || !strcmp(vR, "UT") 
             || !strcmp(vR, "UN") 
             || !strcmp(vR, "SQ"))
            {
                /* 
                 * need to read the next 2 bytes which should be set to 0
                 */
                if (fread(vR, 1, 2, fp) != 2)
                    printf("ERROR: reading VR\n");
                else if (vR[0] == '\0' && vR[1] == '\0')
                {
                    /*
                     * the next 32 bits is the length
                     */
                    if (fread(&along.valueLength, 1, sizeof(along.valueLength),
                        fp) != sizeof(along.valueLength))
                        printf("ERROR: reading Value Length\n");
                    fclose(fp);

                    /*
                     * Make the assumption that if this tag has a value, the
                     * length of the value is going to be small, and thus the
                     * high order 2 bytes will be set to 0.  If the first
                     * bytes read are 0, then this is a big endian syntax.
                     *
                     * If the length of the tag is zero, we look at the 
                     * group number field.  Most DICOM objects start at
                     * group 8. Test for big endian format with the group 8
                     * in the second byte, or else defailt to little endian
                     * because it is more common.
                     */
                    if (along.valueLength)
                    {
                        if ( along.l[0] == '\0' && along.l[1] == '\0') 
                            return EXPLICIT_BIG_ENDIAN_FORMAT;
                        return EXPLICIT_LITTLE_ENDIAN_FORMAT;
                    }
                    else
                    {
                        if (group.i[1] == 8)
                            return EXPLICIT_BIG_ENDIAN_FORMAT;
                        return EXPLICIT_LITTLE_ENDIAN_FORMAT;
                    }
                }
                else
                {
                    printf("ERROR: Data Element not correct format\n");
                    fclose(fp);
                }
            }
            else
            {
                /*
                 * the next 16 bits is the length
                 */
                if (fread(&aint.shorterValueLength, 1,
                    sizeof(aint.shorterValueLength), fp) !=
                    sizeof(aint.shorterValueLength))
                    printf("ERROR: reading short Value Length\n");
                fclose(fp);

                /*
                 * Again, make the assumption that if this tag has a value,
                 * the length of the value is going to be small, and thus the
                 * high order byte will be set to 0.  If the first byte read
                 * is 0, and it has a length then this is a big endian syntax.
                 * Because there is a chance the first tag may have a length
                 * greater than 16 (forcing both bytes to be non-zero, 
                 * unless we're sure, use the group length to test, and then
                 * default to explicit little endian.
                 */
                if  (aint.shorterValueLength 
                 && (aint.i[0] == '\0'))
                    return EXPLICIT_BIG_ENDIAN_FORMAT;
                else
                {
                    if (group.i[1] == 8)
                        return EXPLICIT_BIG_ENDIAN_FORMAT;
                    return EXPLICIT_LITTLE_ENDIAN_FORMAT;
                }
            }
        }
        else
        {
            /* 
             * What we read was not a valid VR, so it must be implicit
             * endian, or maybe format error
             */
            if (fseek(fp, -2L, SEEK_CUR) != 0) 
            {
                printf("ERROR: seeking in file\n");
                return UNKNOWN_FORMAT;
            }

            /*
             * the next 32 bits is the length
             */
            if (fread(&along.valueLength, 1, sizeof(along.valueLength), fp) !=
                sizeof(along.valueLength))
                printf("ERROR: reading Value Length\n");
            fclose(fp);

            /*
             * This is a big assumption, if this tag length is a
             * big number, the Endian must be little endian since
             * we assume the length should be small for the first
             * few tags in this message.
             */
            if (along.valueLength)
            {
                if ( along.l[0] == '\0' && along.l[1] == '\0' ) 
                    return IMPLICIT_BIG_ENDIAN_FORMAT;
                return IMPLICIT_LITTLE_ENDIAN_FORMAT;
            }
            else
            {
                if (group.i[1] == 8)
                    return IMPLICIT_BIG_ENDIAN_FORMAT;
                return IMPLICIT_LITTLE_ENDIAN_FORMAT;
            }
        }
    }
    return UNKNOWN_FORMAT;
} /* CheckFileFormat() */


/****************************************************************************
 *
 *  Function    :   GetSyntaxDescription
 *
 *  Description :   Return a text description of a DICOM transfer syntax.
 *                  This is used for display purposes.
 *
 ****************************************************************************/
static char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr = NULL;
    
    switch (A_syntax)
    {
    case IMPLICIT_LITTLE_ENDIAN: ptr = "Implicit VR Little Endian"; break;
    case EXPLICIT_LITTLE_ENDIAN: ptr = "Explicit VR Little Endian"; break;
    case EXPLICIT_BIG_ENDIAN:    ptr = "Explicit VR Big Endian"; break;
    case IMPLICIT_BIG_ENDIAN:    ptr = "Implicit VR Big Endian"; break;
    case DEFLATED_EXPLICIT_LITTLE_ENDIAN: ptr = "Deflated Explicit VR Little Endian"; break;
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
    case JPEG_SPEC_HIER_20_22:   ptr = "JPEG Spectral Selection Hierarchical (Process 20 & 22)"; break;
    case JPEG_SPEC_HIER_21_23:   ptr = "JPEG Spectral Selection Hierarchical (Process 21 & 23)"; break;
    case JPEG_FULL_PROG_HIER_24_26: ptr = "JPEG Full Progression, Hierarchical (Process 24 & 26)"; break;
    case JPEG_FULL_PROG_HIER_25_27: ptr = "JPEG Full Progression, Hierarchical (Process 25 & 27)"; break;
    case JPEG_LOSSLESS_HIER_28:  ptr = "JPEG Lossless, Hierarchical (Process 28)"; break;
    case JPEG_LOSSLESS_HIER_29:  ptr = "JPEG Lossless, Hierarchical (Process 29)"; break;
    case JPEG_LOSSLESS_HIER_14:  ptr = "JPEG Lossless, Non-Hierarchical, First-Order Prediction"; break;
    case JPEG_2000_LOSSLESS_ONLY:ptr = "JPEG 2000 Lossless Only"; break;
    case JPEG_2000:              ptr = "JPEG 2000"; break;
    case JPEG_LS_LOSSLESS:       ptr = "JPEG-LS Lossless"; break;
    case JPEG_LS_LOSSY:          ptr = "JPEG-LS Lossy (Near Lossless)"; break;
    case MPEG2_MPML:             ptr = "MPEG2 Main Profile @ Main Level"; break;
    case PRIVATE_SYNTAX_1:       ptr = "Private Syntax 1"; break;
    case PRIVATE_SYNTAX_2:       ptr = "Private Syntax 2"; break;
    case INVALID_TRANSFER_SYNTAX:ptr = "Invalid Transfer Syntax"; break;
    }
    return ptr;
}


/****************************************************************************
 *
 *  Function    :   Create_Inst_UID
 *
 *  Parameters  :   none
 *                  
 *  Returns     :   A pointer to a new UID
 *
 *  Description :   This function creates a new UID for use within this 
 *                  application.  Note that this is not a valid method
 *                  for creating UIDs within DICOM because the "base UID"
 *                  is not valid.  
 *                  UID Format:
 *                  <baseuid>.<deviceidentifier>.<serial number>.<process id>
 *                       .<current date>.<current time>.<counter>
 *
 ****************************************************************************/
static char * Create_Inst_UID()
{
    static short UID_CNTR = 0;
    static char  deviceType[] = "1";
    static char  serial[] = "1";
    static char  Sprnt_uid[68];
    char         creationDate[68];
    char         creationTime[68];
    time_t       timeReturn;
    struct tm*   timePtr;
#ifdef UNIX
    unsigned long pid = getpid();
#endif


    timeReturn = time(NULL);
    timePtr = localtime(&timeReturn);
    sprintf(creationDate, "%d%d%d",
           (timePtr->tm_year + 1900),
           (timePtr->tm_mon + 1),
            timePtr->tm_mday);
    sprintf(creationTime, "%d%d%d",
            timePtr->tm_hour,
            timePtr->tm_min,
            timePtr->tm_sec);

#ifdef UNIX    
    sprintf(Sprnt_uid, "2.16.840.1.999999.%s.%s.%d.%s.%s.%d", 
                       deviceType,
                       serial,
                       pid,
                       creationDate,
                       creationTime,
                       UID_CNTR++);
#else
    sprintf(Sprnt_uid, "2.16.840.1.999999.%s.%s.%s.%s.%d", 
                       deviceType,
                       serial,
                       creationDate,
                       creationTime,
                       UID_CNTR++);
#endif

    return(Sprnt_uid);
}


/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
static void PrintError(char* A_string, MC_STATUS A_status)
{
    char        prefix[30] = {0};
    /*
     *  Need process ID number for messages
     */
#ifdef UNIX    
    sprintf(prefix, "PID %d", getpid() );
#endif 
    if (A_status == -1)
    {
        printf("%s\t%s\n",prefix,A_string);
    }
    else
    {
        printf("%s\t%s:\n",prefix,A_string);
        printf("%s\t\t%s\n", prefix,MC_Error_Message(A_status));
    }
}


