/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Tool Kit
 *
 *    $Workfile: stor_scu.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (æ°´, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is a sample Service Class User application
 *               for the Storage Service Class.  The application has a 
 *               number of features:
 *               - It can read in images in both the DICOM Part 10 format 
 *                 and the DICOM "stream" format.
 *               - The application determines the format of the object
 *                 before reading in.
 *               - The AE Title, host name, and port number of the 
 *                 system being connected to can be specified on the 
 *                 command line.
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
 *
 *************************************************************************
 *
 *		© 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)
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

extern SecureSocketFunctions sampleSSfunctions;

/*
 * Application context used for Secure Socket Layer implementation
 */
typedef struct SSLcontext_struct {
    char*   certificateFile;
	char*   privateKeyFile;
	char*   passwrd;
    void*   ctx;
} SSLcontext;

/*
 * CBinfo is used to callback functions when reading in stream objects
 * and Part 10 format objects.
 */
typedef struct CALLBACKINFO
{
    FILE*         fp;
    char          buffer[16*1024];
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
    int     LoopCount;

    char    RemoteAE[AE_LENGTH+2]; 
    char    LocalAE[AE_LENGTH+2]; 
    char    RemoteHostname[100]; 
    int     RemotePort;
    char    ServiceList[100]; 

    SAMP_BOOLEAN Verbose;
    SAMP_BOOLEAN HandleEncapsulated;
    
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
 *  Local Function prototypes
 */

int main(               int                 argc, 
                        char**              argv);
                        
static SAMP_BOOLEAN TestCmdLine( 
                        int                 A_argc,   
                        char*               A_argv[], 
                        STORAGE_OPTIONS*    A_options );
                        
static SAMP_BOOLEAN CheckResponseMessage( 
                        int                 A_responseMsgID );
                        
static FORMAT_ENUM CheckFileFormat( 
                        char*               A_filename );
                        
static SAMP_BOOLEAN ReadImage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_appID, 
                        char*               A_filename,
                        int*                A_msgID,
                        TRANSFER_SYNTAX*    A_syntax,
                        unsigned long*      A_bytesRead);
                        
static SAMP_BOOLEAN SendImage(
                        STORAGE_OPTIONS*    A_options,
                        int                 A_associationID, 
                        int*                A_msgID,
                        TRANSFER_SYNTAX     A_syntax,
                        unsigned long       A_imageBytes);

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
int main(int argc, char** argv)
{
    SAMP_BOOLEAN            sampBool;
    STORAGE_OPTIONS         options;
    TRANSFER_SYNTAX         transferSyntax;
    MC_STATUS               mcStatus;
    int                     applicationID;
    int                     associationID;
    int                     messageID = -1;
    int                     i, imageCurrent;
    time_t                  startTime;
    time_t                  endTime;
    time_t                  totalTime = 0;
    char                    fname[128];
    AssocInfo               asscInfo;
    ServiceInfo             servInfo;
    unsigned long           bytesInImage = 0L;
    unsigned long           totalBytesRead = 0L;
    unsigned long           imagesSent = 0L;
    SSLcontext              securityContext;

    /* Set up SSL application context */
    securityContext.certificateFile = NULL; /* no client certificate */
	securityContext.privateKeyFile = NULL;
	securityContext.passwrd = NULL;
    securityContext.ctx = NULL;
 
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
            
        printf("          Images: %d though %d, %d time%s\n",
            options.StartImage, options.StopImage,
            options.LoopCount, options.LoopCount > 1 ? "s" : "");
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
     *  One association per loop
     */
    for (i = 0; i < options.LoopCount; i++)
    {       
        /*
         * Reset performance calc variables
         */
        totalBytesRead = 0L;
        totalTime = 0L;
         

        /*
         *   One association per loop, open association and override 
         *   hostname & port parameters if they were supplied on the 
         *   command line.  Provide security functions to use.
         */
        mcStatus = MC_Open_Secure_Association( applicationID, &associationID,
                                        options.RemoteAE,
                                        options.RemotePort != -1 ? &options.RemotePort : NULL, 
                                        options.RemoteHostname[0] ? options.RemoteHostname : NULL,
                                        options.ServiceList[0] ? options.ServiceList : NULL,
                                        &sampleSSfunctions, &securityContext);
                                        
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            printf("Unable to open association with \"%s\":\n", options.RemoteAE);
            printf("\t%s\n", MC_Error_Message(mcStatus));
            return(EXIT_FAILURE);
        }
       
        if (options.Verbose)
        {
            mcStatus = MC_Get_Association_Info( associationID, &asscInfo); 
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Get_Association_Info failed", mcStatus);
            }
            else
            {
                printf("Connecting to Remote Application:\n");
                printf("  AE Title:                 %s\n", asscInfo.RemoteApplicationTitle);
                printf("  Host name:                %s\n", asscInfo.RemoteHostName);
                printf("  IP Address:               %s\n", asscInfo.RemoteIPAddress);
                printf("  Implementation Version:   %s\n", asscInfo.RemoteImplementationVersion);
                printf("  Implementation Class UID: %s\n\n\n", asscInfo.RemoteImplementationClassUID);
            }
            
            printf("Services and transfer syntaxes negotiated:\n");
            
            /*
             * Go through all of the negotiated services.  If encapsulated /
             * compressed transfer syntaxes are supported, this code should be
             * expanded to save the services & transfer syntaxes that are 
             * negotiated so that they can be matched the with the transfer
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
         *   Send all requested images
         */
        for (imageCurrent = options.StartImage; imageCurrent <= options.StopImage; imageCurrent++)
        {
            time(&startTime);

            sprintf(fname, "%d.img", imageCurrent);

            /*
             * Determine the image format and read the image in.  If the 
             * image is in the part 10 format, convert it into a message.
             */
            sampBool = ReadImage( &options, 
                                  applicationID, 
                                  fname, 
                                  &messageID, 
                                  &transferSyntax,
                                  &bytesInImage );
            if (!sampBool)
            {
                printf("Can not open image file [%s]\n", fname);
                continue;
            }
           
            totalBytesRead += bytesInImage;

            /*
             * Send image read in with ReadImage.  Note that SendImage
             * will free messageID in all conditions.  This functionality
             * was placed in a function to better handle the failure modes
             * including when a connection can be preserved or when it should
             * be aborted on failure.
             *
             * Because SendImage may not have actually sent an image even
             * though it has returned success, the calculation of 
             * performance data below may not be correct.
             */
            sampBool = SendImage( &options, 
                                  associationID, 
                                  &messageID, 
                                  transferSyntax,
                                  bytesInImage );
            if (!sampBool)
            {
                printf("Failure in sending file [%s]\n", fname);
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                return(EXIT_FAILURE);
            }
            
            /*
             *  How long did it take?
             */
            endTime = time((time_t*)NULL);
            printf("\tElapsed time: %d seconds\n\n",
                (int)(endTime - startTime));
                
            totalTime += (endTime - startTime);
            
        }   /* END for loop for each image */


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
            
            /*
             * Check for divide by zero becaue of a quick transfer.
             */
            if (totalTime == 0) totalTime = 1;
            
            printf("Transfer Rate: %luKB/s\n", (totalBytesRead / totalTime) / 1024 );
        }
        
    } /* End of loop for each association */

    mcStatus = MC_Release_Application(&applicationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Release_Application failed", mcStatus);
    }

    /*
     * Release all memory used by the tool kit.
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
    printf("\nUsage stor_scu remote_ae start stop [loop] -a local_ae -n remote_host \n");
    printf("               -p remote_port -l service_list -v -e\n");
    printf("\tremote_ae   name of remote Application Entity Title\n");
    printf("\tstart       start image number\n");
    printf("\tstop        stop image number\n");
    printf("\tloop        optional number of times to loop\n");
    printf("\tlocal_ae    optional specify the local Application Title\n");
    printf("\t            (Default: MERGE_STORE_SCU)\n");
    printf("\tremote_host optional specify the remote hostname\n");
    printf("\t            (Default: found in the mergecom.app file for remote_ae)\n");
    printf("\tremote_port optional specify the remote TCP listen port\n");
    printf("\t            (Default: found in the mergecom.app file for remote_ae)\n");
    printf("\tservice_list optional specify the service list to use when negotiating\n");
    printf("\t            (Default: found in the mergecom.app file for remote_ae)\n");
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
    
    if (A_argc < 4)
    {
        PrintCmdLine();
        return SAMP_FALSE;
    }
   
    /*
     * Set default values
     */
    A_options->StartImage = 1;
    A_options->StopImage = 1;
    A_options->LoopCount = 1;

    strcpy(A_options->LocalAE, "MERGE_STORE_SCU");
    A_options->RemoteAE[0] = '\0'; 
    A_options->RemoteHostname[0] = '\0'; 
    A_options->RemotePort = -1;
    A_options->ServiceList[0] = '\0'; 
    A_options->Verbose = SAMP_FALSE;
    A_options->HandleEncapsulated = SAMP_FALSE;


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
                case 4:
                    A_options->LoopCount = atoi(A_argv[i]);
                    if (A_options->LoopCount <= 0 
                     || A_options->LoopCount > 32000)
                    {
                        printf("Loop count must be 1 thru 32000.\n");
                        return SAMP_FALSE;
                    }
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
 *  Function    :   ReadImage
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_filename - Name of file to open
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *                  A_syntax   - The transfer syntax the original image was
 *                               encoded as.
 *                  A_bytesRead- Total number of bytes read in image.  Used
 *                               only for display and to calculate the
 *                               transfer rate.
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
                              char*             A_filename,
                              int*              A_msgID,
                              TRANSFER_SYNTAX*  A_syntax,
                              unsigned long*    A_bytesRead)
{
    FORMAT_ENUM             format = UNKNOWN_FORMAT;
    SAMP_BOOLEAN            sampBool;


    format = CheckFileFormat( A_filename );
    switch(format)
    {
        case MEDIA_FORMAT:
            sampBool = ReadFileFromMedia( A_options, 
                                          A_appID, 
                                          A_filename, 
                                          A_msgID, 
                                          A_syntax, 
                                          A_bytesRead );
            break;
                
        case IMPLICIT_LITTLE_ENDIAN_FORMAT:
        case IMPLICIT_BIG_ENDIAN_FORMAT:
        case EXPLICIT_LITTLE_ENDIAN_FORMAT:
        case EXPLICIT_BIG_ENDIAN_FORMAT:
            sampBool = ReadMessageFromFile( A_options, 
                                            A_filename, 
                                            format, 
                                            A_msgID, 
                                            A_syntax, 
                                            A_bytesRead );
            break;
            
        case UNKNOWN_FORMAT:
            PrintError("Unable to determine the format of file",
                       MC_NORMAL_COMPLETION);
            sampBool = SAMP_FALSE;
            break;
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
                              int*              A_msgID,
                              TRANSFER_SYNTAX   A_syntax,
                              unsigned long     A_imageBytes )
{
    MC_STATUS       mcStatus;
    SAMP_BOOLEAN    sampBool;
    int             responseMessageID;
    char            serviceName[48];
    char*           responseService;
    MC_COMMAND      responseCommand;
    char            SOPClassUID[UI_LENGTH+2];
    static char     affectedSOPinstance[UI_LENGTH+2];


    /* Get the SOP class UID and set the service */
    mcStatus = MC_Get_Value_To_String(*A_msgID, 
                    MC_ATT_SOP_CLASS_UID,
                    sizeof(SOPClassUID),
                    SOPClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String for SOP Class UID failed", mcStatus);
        MC_Free_Message(A_msgID);
        return(SAMP_TRUE);
    }
    
    mcStatus = MC_Get_MergeCOM_Service(SOPClassUID, serviceName, sizeof(serviceName));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_MergeCOM_Service failed", mcStatus);
        MC_Free_Message(A_msgID);
        return ( SAMP_TRUE );
    }            
                 
    mcStatus = MC_Set_Service_Command(*A_msgID, serviceName, C_STORE_RQ);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Service_Command failed", mcStatus);
        MC_Free_Message(A_msgID);
        return(SAMP_TRUE);
    }
            
    /* Get and set affected SOP Instance UID */
    mcStatus = MC_Get_Value_To_String(*A_msgID, 
                    MC_ATT_SOP_INSTANCE_UID,
                    sizeof(affectedSOPinstance),
                    affectedSOPinstance);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String for SOP Instance UID failed", mcStatus);
        MC_Free_Message(A_msgID);
        return(SAMP_TRUE);
    }
            
    
    mcStatus = MC_Set_Value_From_String(*A_msgID, 
                      MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                      affectedSOPinstance);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed for affected SOP Instance UID", mcStatus);
        MC_Free_Message(A_msgID);
        return(SAMP_TRUE);
    }
            
    /*
     *  Send the message
     */
    if (A_options->Verbose)
    {
        printf("Sending (%s) object\n", serviceName);
        printf("  UID: %s\n", affectedSOPinstance);
        printf(" Size: %lu bytes\n", A_imageBytes);
    }
    else
        printf("\tSending %s image\n", serviceName);

    mcStatus = MC_Send_Request_Message(A_associationID, *A_msgID);
    if (mcStatus == MC_ASSOCIATION_ABORTED
     || mcStatus == MC_SYSTEM_ERROR)
    {
        /*
         * At this point, the association has been dropped, or we should
         * drop it in the case of MC_SYSTEM_ERROR.
         */
        PrintError("MC_Send_Request_Message failed", mcStatus);
        MC_Free_Message(A_msgID);
        return(SAMP_FALSE);
    }
    else if (mcStatus != MC_NORMAL_COMPLETION)
    {
        /*
         * This is a failure condition we can continue with
         */
        PrintError("Warning: MC_Send_Request_Message failed", mcStatus);
        MC_Free_Message(A_msgID);
        return(SAMP_TRUE);
    }
    
    mcStatus = MC_Free_Message(A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for request message", mcStatus);
        return(SAMP_FALSE);
    }

    
    /*
     *  Wait for response
     */
    mcStatus = MC_Read_Message(A_associationID, 30, &responseMessageID,
                 &responseService, &responseCommand);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Read_Message failed", mcStatus);
        return(SAMP_FALSE);
    }
    
    sampBool = CheckResponseMessage ( responseMessageID );
    if (!sampBool)
    {
        MC_Free_Message(&responseMessageID);
        return(SAMP_FALSE);
    }

    mcStatus = MC_Free_Message(&responseMessageID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for response message", mcStatus);
        return(SAMP_FALSE);
    }

    return(SAMP_TRUE);
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
static SAMP_BOOLEAN CheckResponseMessage ( int A_responseMsgID )
{
    MC_STATUS mcStatus;
    SAMP_BOOLEAN returnBool = SAMP_TRUE;
    unsigned int statval;

    mcStatus = MC_Get_Value_To_UInt ( A_responseMsgID,
                                      MC_ATT_STATUS,
                                      &statval );
    if ( mcStatus != MC_NORMAL_COMPLETION )
    {
        /* Problem with MC_Get_Value_To_UInt */
        PrintError ( "MC_Get_Value_To_UInt for response status failed", mcStatus );
        return SAMP_FALSE;
    }

    /* MC_Get_Value_To_UInt worked.  Check the response status */

    if ( statval != C_STORE_SUCCESS )
    {
        printf ( "\n\nResponse message was not C_STORE_SUCCESS:\n" );

        switch ( statval )
        {
            /* Warnings.  Continue execution. */

            case C_STORE_WARNING_ELEMENT_COERCION:
                printf ( "Warning: Element Coersion... Continuing.\n" );
                break;

            case C_STORE_WARNING_INVALID_DATASET:
                printf ( "Warning: Invalid Dataset... Continuing.\n" );
                break;

            case C_STORE_WARNING_ELEMENTS_DISCARDED:
                printf ( "Warning: Elements Discarded... Continuing.\n" );
                break;

            /* Errors.  Abort execution. */

            case C_STORE_FAILURE_REFUSED_NO_RESOURCES:
                printf ( "ERROR: REFUSED, NO RESOURCES.  ASSOCIATION ABORTING.\n" );
                returnBool = SAMP_FALSE;
                break;

            case C_STORE_FAILURE_INVALID_DATASET:
                printf ( "ERROR: INVALID_DATASET.  ASSOCIATION ABORTING.\n" );
                returnBool = SAMP_FALSE;
                break;

            case C_STORE_FAILURE_CANNOT_UNDERSTAND:
                printf ( "ERROR: CANNOT UNDERSTAND.  ASSOCIATION ABORTING.\n" );
                returnBool = SAMP_FALSE;
                break;

            case C_STORE_FAILURE_PROCESSING_FAILURE:
                printf ( "ERROR: PROCESSING FAILURE.  ASSOCIATION ABORTING.\n" );
                returnBool = SAMP_FALSE;
                break;
                
            default:
                printf ( "Warning: Unknown status (0x%04x)... Continuing.\n", statval );
                break;
        }
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
    CBinfo      callbackInfo;
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
            case PRIVATE_SYNTAX_1:
            case PRIVATE_SYNTAX_2:
                printf("Warning: Encapsulated transfer syntax (%s) image specified\n", 
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

