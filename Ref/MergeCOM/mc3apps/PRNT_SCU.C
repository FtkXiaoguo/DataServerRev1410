/*************************************************************************
 *
 *       System: MergeCOM-3 Advanced Tool Kit Print SCU Sample App
 *
 *    $Workfile: prnt_scu.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is a sample service class user application
 *               for the Print Service Class
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

#ifdef INTEL_WCC
#ifdef NO_EXT_KEYS
#undef NO_EXT_KEYS
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_MACINTOSH)
#include <Types.h>
#include <console.h>
#include <SIOUX.h>
#endif

#include "mc3msg.h"
#include "mcstatus.h"
#include "mergecom.h"
#include "diction.h"

#include "prnt_svc.h"

/*
 * Module constants
 */
#define transfer_syntax IMPLICIT_LITTLE_ENDIAN
#define UID_LENGTH 66
#define AE_LENGTH  32

#define FALSE 0
#define TRUE 1



/*
 * Module type declarations
 */
typedef struct CALLBACKINFO
{        
    FILE*   stream;
    int     messageID;
} CBinfo;

typedef struct printjob_struct
{
    char       jobUID[UID_LENGTH];  /* The UID of the print job for this record */
    int        status;              /* The status of this print job */
    struct printjob_struct* next;   /* pointer to the next structure in the list. */

} printjob_t;

typedef struct pj_static_struct
{    
    printjob_t* head;               /* the head of the print job list */
    printjob_t* tail;               /* the tail of the print job list */
    int       jobCount;             /* the count of the number of printjobs */
} pj_static_t;



/* 
 *  Module variables
 */
static char         S_fname[128];
static char         S_session_uid[UID_LENGTH];
static char         S_session_sop[UID_LENGTH];
static int          S_printJob_Negotiated;
static pj_static_t  S_pj;

static struct
{
    char    film_uid[65];
    char    image_uids[40][65];
    int     images;
} Films[10];


/* 
 *  Local Function prototypes
 */
static void print_cmdline(void);

static MC_STATUS test_cmd_line(  /* Test Command Line */
                    int                Aargc,      
                    char*              Aargv[],    
                    char*              A_printLevel, 
                    char*              A_app_ent, 
                    char*              A_priority, 
                    int*               A_copies, 
                    int*               A_format, 
                    int*               A_image_start, 
                    int*               A_image_stop, 
                    int*               A_loop_count );

static MC_STATUS StreamToMessageFunction(
                    int                msgID,
                    void*              CBinformation,
                    int                firstCall,
                    int*               dataLen,
                    void**             dataBuffer,
                    int*               isLast);

static MC_STATUS Local_Read_Message(
                    int                A_applicationID,
                    int                A_associationID,
                    int                A_timeOut,
                    int*               A_messageID,
                    char**             A_serviceName,
                    MC_COMMAND*        A_command);

static MC_STATUS Printer_N_GET( 
                    int                A_applicationID,
                    int                A_associationID);

static MC_STATUS FilmSession_N_CREATE( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_priority,
                    int                A_copies);
                    
static MC_STATUS FilmBox_N_CREATE( 
                    int                A_applicationID,
                    int                A_associationID,
                    int                A_format,
                    int                A_page);
                    
static MC_STATUS ImageBox_N_SET( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_imageBoxUID,
                    int                A_image_position,
                    int                A_image_current);

static MC_STATUS FilmBox_N_ACTION( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_filmUID);

static MC_STATUS FilmSession_N_ACTION( 
                    int                A_applicationID,
                    int                A_associationID);
                    
static MC_STATUS FilmSession_N_DELETE( 
                    int                A_applicationID,
                    int                A_associationID);

static int DeletePrintJobRecord(
                    char*              A_jobUID);

static int CreatePrintJobRecord(
                    char*              A_jobUID,
                    int                A_status);

static int SetPrintJobStatus(
                    char*              A_jobUID, 
                    int                A_status);

static int CheckForPrintJob(
                    char*              A_jobUID);

static void PrintError(
                    char*              A_string, 
                    MC_STATUS          A_status);


/****************************************************************************
 *
 *  Function    :   Main
 *
 *  Description :   Mainline 
 *
 ****************************************************************************/
#ifdef VXWORKS
int  prntscu(int argc, char** argv);
int prntscu(int argc, char** argv)
#else
int  main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    int                     applicationID;
    int                     associationID;
    int                     responseMessageID;
    MC_STATUS               status;
    unsigned int            dicomStatus;
    char*                   responseService;
    MC_COMMAND              responseCommand;
    int                     image_start;
    int                     image_stop;
    int                     image_current;
    int                     image_position;
    int                     format;
    int                     loop_count, l;
    int                     page;
    int                     copies;
    char                    app_ent[AE_LENGTH];
    char                    priority[14];
    char                    printLevel[14];
    ServiceInfo             serviceInfo;

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
     * Read in and test the command line arguements
     * to the application
     */
    status = test_cmd_line( argc, 
                            argv, 
                            printLevel, 
                            app_ent, 
                            priority, 
                            &copies, 
                            &format, 
                            &image_start, 
                            &image_stop, 
                            &loop_count );
    if (status != MC_NORMAL_COMPLETION)
        exit(EXIT_FAILURE);   
    
    printf("Printing images %d though %d to %s, %d image%s up\n",
           image_start, image_stop, app_ent, format,
           format > 1 ? "s" : "");
    
    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */
  
    status = MC_Library_Initialization ( NULL, NULL, NULL );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize library",status);
        exit ( EXIT_FAILURE );
    }   

    /* ------------------------------------------------------- */
    /*  Register this DICOM application                        */
    /* ------------------------------------------------------- */

    status = MC_Register_Application(&applicationID, "MERGE_PRINT_SCU");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to register \"MERGE_PRINT_SCU\"",status);
        exit ( EXIT_FAILURE );
    }

    /*
     *   Create the Association
     *   One association per loop count
     */
    for (l = 1; l <= loop_count;  l++) 
    {

        status = MC_Open_Association(applicationID, 
                                     &associationID,
                                     app_ent, 
                                     NULL,
                                     NULL, 
                                     NULL);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to open association:", status);
            exit ( EXIT_FAILURE );
        }
        else
        {
            printf("Connected to remote system [%s]\n", app_ent);
        }

        /*
         * Parse the list of service classes negotiated to determine
         * if the print job SOP class was accepted.
         */ 
        S_printJob_Negotiated = FALSE;
        status = MC_Get_First_Acceptable_Service(associationID, &serviceInfo); 
        if (status == MC_NORMAL_COMPLETION)
        {
            while (status == MC_NORMAL_COMPLETION)
            {
                if( strcmp( serviceInfo.ServiceName, "PRINT_JOB" ) == 0)
                    S_printJob_Negotiated = TRUE;
                printf("Service Negotiated: %s" ,serviceInfo.ServiceName);
                if (status == MC_BUFFER_TOO_SMALL )
                    printf( " (TRUNCATED)" );
                printf("\n");
                status = MC_Get_Next_Acceptable_Service(associationID, &serviceInfo);
             }

        }
        else if( status == MC_END_OF_LIST )
        {
            PrintError("MC_Get_First_Acceptable_Service warning",status);
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            exit ( EXIT_FAILURE );
        }
        else
        {
            PrintError("MC_Get_First_Acceptable_Service failed",status);
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            exit ( EXIT_FAILURE );
        }


        if( S_printJob_Negotiated )
            printf( "Print Job Service Class was negotiated\n");
        else
            printf( "Print Job Service Class was NOT negotiated\n");  
            

        /*
         *   Get printer information
         */
        status = Printer_N_GET( applicationID, associationID );
        if (status != MC_NORMAL_COMPLETION)
        {
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            exit(EXIT_FAILURE); 
        }
                                                                             
     
        /*
         * Create the Film Session with the SCP
         */
        status = FilmSession_N_CREATE(applicationID,associationID,priority,copies);
        if (status != MC_NORMAL_COMPLETION)
        {
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            exit(EXIT_FAILURE); 
        }
        
        
        /*
         *   Send all requested images, send Film Box Creation message at
         *   the beginning of a page.
         */
        for (page = image_position = 0, image_current = image_start;
             image_current <= image_stop && page < 10;
             image_position++, image_current++)
        {
            if((image_position % format) == 0)
            {
                if (printLevel[0] == 'B')
                {
                    if (page)
                    {
                        /*
                         * Print a Film_Box 
                         */
                        printf ("\n\nFilm_Box N_ACTION\n\n" );
                        status = FilmBox_N_ACTION(applicationID,associationID,Films[page-1].film_uid);
                        if (status != MC_NORMAL_COMPLETION)
                        {
                            MC_Abort_Association(&associationID);
                            MC_Release_Application(&applicationID);
                            exit(EXIT_FAILURE);
                        }
                        page--;
                    } /* end of "if(page)" */
                } 
        
                /*
                 * Create the Film Box with the SCP
                 */
                status = FilmBox_N_CREATE(applicationID,associationID,format,page); 
                if (status != MC_NORMAL_COMPLETION)
                {
                    MC_Abort_Association(&associationID);
                    MC_Release_Application(&applicationID);
                    exit(EXIT_FAILURE);
                }

                page++;
                printf("Film Box Created!!\n");
                image_position = 0;
            }
            
            status = ImageBox_N_SET(applicationID,
                                    associationID,
                                    Films[page-1].image_uids[image_position],
                                    image_position,
                                    image_current); 
                                                                               
            if (status != MC_NORMAL_COMPLETION)
            {
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                exit(EXIT_FAILURE);
            }

        }   /* END for loop for each image */


        if( printLevel[0] == 'B' )
        {
            if(image_position)
            {
                   printf ("\n\nFilm_Box N_ACTION\n\n" );
                status = FilmBox_N_ACTION(applicationID,associationID,Films[page-1].film_uid);
                if (status != MC_NORMAL_COMPLETION)
                {
                    MC_Abort_Association(&associationID);
                    MC_Release_Application(&applicationID);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else 
        {
            printf ("\n\nFilm_Session N_ACTION\n\n" );
            status = FilmSession_N_ACTION(applicationID,associationID);
            if (status != MC_NORMAL_COMPLETION)
            {
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                exit(EXIT_FAILURE);
            }
     
        } /* N-ACTION print session */    

        status = FilmSession_N_DELETE( applicationID,associationID );
        if (status != MC_NORMAL_COMPLETION)
        {
            MC_Abort_Association(&associationID);
            MC_Release_Application(&applicationID);
            exit(EXIT_FAILURE);
        }
        
        /*
         *  Wait for response from the FilmSession N-DELETE &
         *  read messages until all print jobs have completed 
         */
        do
        {
            status = Local_Read_Message( applicationID,
                                         associationID, 
                                         30, 
                                         &responseMessageID,
                                         &responseService, 
                                         &responseCommand);
            if (status == MC_TIMEOUT)
                continue;
            else if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("Local_Read_Message failed",status);
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                exit ( EXIT_FAILURE );
            }
            
            status = MC_Get_Value_To_UInt ( responseMessageID, MC_ATT_STATUS, &dicomStatus );
            if ( status != MC_NORMAL_COMPLETION )
            {
                PrintError ( "MC_Get_Value_To_UInt failed for FILM_SESSION,N-DELETE-RSP",status );
                MC_Free_Message(&responseMessageID);
                exit ( EXIT_FAILURE );
            }

            if ( dicomStatus != N_DELETE_SUCCESS )
            {
                printf ( "\n\nResponse message was not N_DELETE_SUCCESS: 0x%x\n", dicomStatus );
                MC_Free_Message(&responseMessageID);
                exit ( EXIT_FAILURE );
            }
    
            status = MC_Free_Message(&responseMessageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message from Local_Read_Message failed",status);
                MC_Abort_Association(&associationID);
                MC_Release_Application(&applicationID);
                exit ( EXIT_FAILURE );
            }

            
        } while (S_pj.jobCount > 0);

        /*
         * Close Association  
         */
        status = MC_Close_Association(&associationID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Close_Association failed",status);
            exit(EXIT_FAILURE);
        }
  
    } /* end of "for (l = 1; l <= loop_count;  l++)" */ 

    MC_Release_Application(&applicationID);

    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");

    printf("\n");
    return(EXIT_SUCCESS);
} /* end of main */


/********************************************************************
 *
 *  Function    :   print_cmdline
 *
 *  Parameters  :   none
 *
 *  Returns     :   nothing
 *
 *  Description :   Prints program usage
 *
 ********************************************************************/
static void print_cmdline(void)
{
    printf("\nUsage prnt_scu -c <copies> -p <priority> -l <level> <application> <format> <start> <stop> <loop_count>\n");
    printf("\tcopies      number of copies to request\n");
    printf("\tpriority    priority level to request - L, M, H\n");
    printf("\tlevel       print level to request - S=session, B=film box\n");
    printf("\tapplication Application Entity Name of remote server\n");
    printf("\tformat      number of images to put on a page\n");
    printf("\tstart       start image number\n");
    printf("\tstop        stop image number\n");
    printf("\tloop_count  Number of times to repeat operation\n");
    printf("\n\tImage files must be in the current directory.\n");
    printf("\n\tImage files must be named 0.img, 1.img, 2.img, etc.\n");
} /* end print_cmdline() */



/*************************************************************************
 *
 *  Function    :   test_cmdline
 *
 *  Parameters  :   Aargc   - Command line arguement count
 *                  Aargv   - Command line arguements
 *                  A_printLevel - The level to print at is returned here
 *                  A_app_ent    - The remote application entity is 
 *                                 returned here
 *                  A_priority   - The priority at which to print s 
 *                                 returned here
 *                  A_copies     - The number of copies to print is
 *                                 returned here
 *                  A_image_start- The image start number is returned here
 *                  A_image_stop - The image stop number is returned here
 *                  A_loop_count - The number of times to loop through the
 *                                 image list is returned here
 *
 *  Return value:   MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 *
 *  Description :   Test command line for valid arguements and the
 *                  existence of the input file.  If problems are found
 *                  display a message and return MC_CANNOT_COMPLY
 *
 *************************************************************************/
static MC_STATUS test_cmd_line(  /* Test Command Line */
        int                 Aargc,   
        char*               Aargv[], 
        char*               A_printLevel, 
        char*               A_app_ent, 
        char*               A_priority, 
        int*                A_copies, 
        int*                A_format, 
        int*                A_image_start, 
        int*                A_image_stop, 
        int*                A_loop_count )
{
    MC_STATUS return_value = MC_NORMAL_COMPLETION;  /* Default to Success */
    int i;
    
    *A_copies = 1;

    /* 
     * Set the default print level to film Session 
     */
    strcpy(A_printLevel, "S");

#ifndef VXWORKS
    if (Aargc < 5)
    {
        print_cmdline();
        return MC_CANNOT_COMPLY;
    }
#else
    strcpy(A_priority,"MED");
    A_printLevel[0] = 'S';
    strcpy(A_app_ent,"MERGE_PRINT_SCP");
    *A_format = 1;
    *A_image_start = 1;
    *A_image_stop = 3;
    *A_loop_count = 1;
    return MC_NORMAL_COMPLETION;
#endif

    /*
     * Loop through each arguement
     */     
    for (i = 1; i < Aargc; i++)
    {
        if ( !strcmp(Aargv[i], "-h") || !strcmp(Aargv[i], "/h") ||
             !strcmp(Aargv[i], "-H") || !strcmp(Aargv[i], "/H") ||
             !strcmp(Aargv[i], "-?") || !strcmp(Aargv[i], "/?"))
        {
            print_cmdline();
            return_value = MC_CANNOT_COMPLY; 
        }
        else if ( !strcmp(Aargv[i], "-c") || !strcmp(Aargv[i], "-C"))
        {
            /*
             * Number of copies
             */
            i++;
            *A_copies = atoi(Aargv[i]);
        } 
        else if ( !strcmp(Aargv[i], "-p") || !strcmp(Aargv[i], "-P"))
        {
            /*
             * Print job priority
             */
            i++;
            strcpy(A_priority,Aargv[i]);
        }
        else if ( !strcmp(Aargv[i], "-l") || !strcmp(Aargv[i], "-L"))
        {
            /*
             * Level at which to print
             */
            i++;
            strcpy(A_printLevel,Aargv[i]);
            if (A_printLevel[0]=='b') 
                A_printLevel[0]='B';
            if (A_printLevel[0]=='s') 
                A_printLevel[0]='S';
        }
        else
        {
            /*
             * We are done with options, parse through the remaining
             * config options below
             */
            break;
        }
    }
    /* 
     * Remote server application entity title
     */
    strcpy(A_app_ent,Aargv[i]);

    /* 
     * Number of images per page 
     */
    i++;
    *A_format = atoi(Aargv[i]);

    /* 
     * Start image 
     */
    i++;
    *A_image_start = atoi(Aargv[i]);

    /* 
     * Stop image 
     */
    i++;
    *A_image_stop  = atoi(Aargv[i]);
         
    /* 
     * Loop count 
     */
    i++;
    if (i < Aargc)
    {
        *A_loop_count = atoi(Aargv[i]);
    }
    else
    {
        /* default loop count to 1 */
        *A_loop_count = 1;
    }

    /* check priority */
    switch(A_priority[0])
    {
        case 'l':
        case 'L': strcpy(A_priority, "LOW");
                  break;
        case 'h':
        case 'H': strcpy(A_priority, "HIGH");
                  break;
        case 'm':
        case 'M': strcpy(A_priority, "MED");
                  break;
        default : strcpy(A_priority, "LOW");

    }

    /* 
     * Check number of images to print on page
     */
    switch(*A_format)
    {
        case 1:
        case 2:
        case 4:
        case 6:
        case 9:
        case 12:
        case 15:
        case 16:
        case 20:
        case 24:
        break;
        default:
            printf("ERROR: Unsupported format used %d-up\n", *A_format);
            print_cmdline();
            return_value = MC_CANNOT_COMPLY;
    }

    if (*A_image_stop < *A_image_start)
    {
        printf("Image stop number must be greater than or equal to image start number.\n");
        print_cmdline();
        return_value = MC_CANNOT_COMPLY;
    }
    
    return return_value;
    
}/* test_cmd_line() */



/****************************************************************************
 *
 *  Function    :   StreamToMessageFunction
 *
 *  Description :   Callback used to read in DICOM storage class images
 *
 ****************************************************************************/
static MC_STATUS StreamToMessageFunction (
                    int             msgID,
                    void*           CBinformation,
                    int             firstCall,
                    int*            dataLen,
                    void**          dataBuffer,
                    int*            isLast) 
{
    static short    buffer[4*1024];
    size_t          bytes_read;
    CBinfo*         callbackInfo = (CBinfo*)CBinformation;
    
    if (firstCall && msgID != callbackInfo->messageID)
    {
        printf("Wrong message ID!\n");
        return MC_CANNOT_COMPLY;
    }
    
    bytes_read = fread (buffer, 1, sizeof(buffer), callbackInfo->stream);
    if (ferror(callbackInfo->stream))
    {
        perror("Read error");
        return MC_CANNOT_COMPLY;
    }
    
    if (feof(callbackInfo->stream))
        *isLast = 1;
    else
        *isLast = 0;
            
    *dataBuffer = buffer;
    *dataLen = (int)bytes_read;
    return MC_NORMAL_COMPLETION;
}



/**************************************************************************
 *
 *  Function     :  Local_Read_Mesaage
 * 
 *  Parameters   :  A_applicationID  - The MERGE_PRINT_SCU application ID
 *                  A_associationID  - The current assocation ID
 *                  A_timeOut        - The timeout for calling MC_Read_Message()
 *                  A_messageID      - The message ID is returned here
 *                  A_serviceName    - The service name of the message read
 *                                     is returned here
 *                  A_command        - The command of the message read is
 *                                     returned here
 *                  
 *  Returns      :     MC_STATUS values returned by MC_Read_Message()
 * 
 *  Description  :  This function reads the message sent from the SCP.  It
 *                  is implemented such that N-EVENT-REPORT-RQ events
 *                  asynchronously sent from the print SCP are intercepted
 *                  and processed.
 *
 ****************************************************************************/  

static MC_STATUS Local_Read_Message(
                    int             A_applicationID,
                    int             A_associationID,
                    int             A_timeOut,
                    int*            A_messageID,
                    char**          A_serviceName,
                    MC_COMMAND*     A_command)
{
    MC_STATUS      status;
    unsigned short eventType;
    char           uidBuffer[UID_LENGTH];
    char           tmpstr[80];
    int            rsp_status;
    int            rspMessageID = -1;


    for(;;)
    {
        status = MC_Read_Message( A_associationID, 
                                  A_timeOut, 
                                  A_messageID, 
                                  A_serviceName, 
                                  A_command );
        /*
         * Abort out if an error is encountered
         */
        if( status != MC_NORMAL_COMPLETION ) return status;
        
        /*
         * If a response message is encountered, print out the
         * response here, because we usually do not handle it 
         * when this function is called.
         */
        if( ( *A_command == C_STORE_RSP ) ||
            ( *A_command == C_ECHO_RSP ) ||
            ( *A_command == C_FIND_RSP ) ||
            ( *A_command == C_GET_RSP ) ||
            ( *A_command == C_MOVE_RSP ) ||
            ( *A_command == N_EVENT_REPORT_RSP ) ||
            ( *A_command == N_GET_RSP ) ||
            ( *A_command == N_SET_RSP ) ||
            ( *A_command == N_ACTION_RSP ) ||
            ( *A_command == N_CREATE_RSP ) ||
            ( *A_command == N_DELETE_RSP ))
        {
            status = MC_Get_Value_To_Int( *A_messageID, MC_ATT_STATUS, &rsp_status );
            if( status != MC_NORMAL_COMPLETION ) return MC_NORMAL_COMPLETION;
            if( rsp_status )
            {
                printf( "\n\n*** Reponse Error ***\n");
                printf( "reponse = 0x%x\n\n", rsp_status);
            }
        }
        
        /*
         * Exit for any service except N_EVENT_REPORT_RQ
         */
        if( *A_command != N_EVENT_REPORT_RQ )  return status;

        if( strcmp( *A_serviceName, "PRINTER" ) == 0)
        {
            status = MC_Get_Value_To_String( *A_messageID, 
                                             MC_ATT_AFFECTED_SOP_INSTANCE_UID, 
                                             sizeof(uidBuffer), 
                                             uidBuffer);
            if (status != MC_NORMAL_COMPLETION)
                PrintError("MC_Get_Value_To_String for printer SOP Instance UID failed",status );

            status = MC_Get_Value_To_UShortInt( *A_messageID, 
                                                MC_ATT_EVENT_TYPE_ID, 
                                                &eventType );
            if (status != MC_NORMAL_COMPLETION)
                eventType = 0;

            switch( eventType )
            {
                case 1: /* NORMAL */
                    printf ( "\nprinter uid %s NORMAL event\n\n", uidBuffer );  
                    break;
                case 2: /* WARNING */
                    printf ( "\nprinter uid %s WARNING event\n", uidBuffer );  
                    status = MC_Get_Value_To_String( *A_messageID, 
                                                     MC_ATT_PRINTER_NAME, 
                                                     sizeof( tmpstr ), 
                                                     tmpstr );
                    if (status == MC_NORMAL_COMPLETION)
                    {
                        printf( "\tPrinter Name: %s\n", tmpstr);
                    }
                    status = MC_Get_Value_To_String( *A_messageID, 
                                                     MC_ATT_PRINTER_STATUS_INFO, 
                                                     sizeof( tmpstr ), 
                                                     tmpstr );
                    if (status == MC_NORMAL_COMPLETION)
                    {
                        printf( "\tPrinter Status Info: %s\n", tmpstr);
                    }

                    break;
                case 3: /* ERROR */
                    printf ( "\nprinter uid %s ERROR event\n", uidBuffer );  
                    status = MC_Get_Value_To_String( *A_messageID, 
                            MC_ATT_PRINTER_NAME, sizeof( tmpstr ), tmpstr );
                    if (status == MC_NORMAL_COMPLETION)
                    {
                        printf( "\tPrinter Name: %s\n", tmpstr);
                    }
                    status = MC_Get_Value_To_String( *A_messageID, 
                            MC_ATT_PRINTER_STATUS_INFO, sizeof( tmpstr ), tmpstr );
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        printf( "\tPrinter Status Info: %s\n", tmpstr);
                    }
                    break;
                default:
                    printf( "printer n_event_report_rq; unknown event type %d\n", eventType );
            }

            status = MC_Open_Message (&rspMessageID, *A_serviceName, N_EVENT_REPORT_RSP);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Open_Message error",status);
                MC_Free_Message(A_messageID);
                return ( MC_SYSTEM_ERROR ); 
            }

            status = MC_Send_Response_Message(A_associationID, N_EVENT_SUCCESS, rspMessageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Send_Response_Message for N_EVENT_REPORT_RSP error",status);
                MC_Free_Message(A_messageID);
                return( MC_SYSTEM_ERROR ); 
            }

            status = MC_Free_Message(A_messageID); 
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message of PRINTER,N_EVENT_REPORT_RQ error",status);
                MC_Free_Message(&rspMessageID);
                return ( MC_SYSTEM_ERROR ); 
            }

            status = MC_Free_Message(&rspMessageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message of PRINTER,N_EVENT_REPORT_RSP error",status);
                return ( MC_SYSTEM_ERROR ); 
            }

        }
        else if(!strcmp( *A_serviceName, "PRINT_JOB" ))
        {
            status = MC_Get_Value_To_String( *A_messageID, 
                        MC_ATT_AFFECTED_SOP_INSTANCE_UID, 
                        sizeof(uidBuffer), uidBuffer);
            if (status == MC_NORMAL_COMPLETION)
            {
                if( CheckForPrintJob(uidBuffer) )
                {
                    status = MC_Get_Value_To_UShortInt( *A_messageID, 
                                MC_ATT_EVENT_TYPE_ID, &eventType );
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        eventType = 0;
                    }

                    switch( eventType )
                    {
                        case 1: /* PENDING */
                            printf ( "\nPrint Job UID: %s is PENDING\n", uidBuffer );
                            SetPrintJobStatus(uidBuffer, eventType);  
                            break;
                        case 2: /* PRINTING */
                            printf ( "\nPrint Job UID: %s is PRINTING\n", uidBuffer );
                            SetPrintJobStatus(uidBuffer, eventType);  
                            break;
                        case 3: /* DONE */
                            printf ( "\nPrint Job UID: %s has completed printing \n", uidBuffer );  
                            DeletePrintJobRecord(uidBuffer);  
                            break;
                        case 4: /* FAILURE */
                            printf ( "\nPrint Job UID: %s has FAILED\n", uidBuffer );  
                            DeletePrintJobRecord(uidBuffer);  
                            break;
                        default:
                            printf("print job: UID: %s event_report invalid event type %d\n",
                                    uidBuffer, eventType );
                    }
                    status = MC_Get_Value_To_String( *A_messageID, 
                                                     MC_ATT_PRINTER_NAME,  
                                                     sizeof( tmpstr ), tmpstr );
                    if (status == MC_NORMAL_COMPLETION)
                    {
                        printf( "\tPrinter Name: %s\n", tmpstr);
                    }
                    status = MC_Get_Value_To_String( *A_messageID, 
                                                     MC_ATT_FILM_SESSION_LABEL,
                                                     sizeof( tmpstr ), tmpstr );
                    if (status == MC_NORMAL_COMPLETION)
                    {
                        printf( "\tFilm Session Label: %s\n", tmpstr);
                    }
                    status = MC_Get_Value_To_String( *A_messageID, 
                                                     MC_ATT_EXECUTION_STATUS_INFO,
                                                     sizeof( tmpstr ), tmpstr );
                    if (status == MC_NORMAL_COMPLETION)
                    {
                        printf( "\tExecution Status Info: %s\n", tmpstr);
                    }
                }
            }

            status = MC_Open_Message (&rspMessageID, *A_serviceName, N_EVENT_REPORT_RSP);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Open_Message error",status);
                MC_Free_Message(A_messageID);
                return MC_SYSTEM_ERROR; 
            }

            status = MC_Send_Response_Message(A_associationID, N_EVENT_SUCCESS, rspMessageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Send_Response_Message error",status);
                MC_Free_Message(A_messageID);
                return MC_SYSTEM_ERROR; 
            }

            status = MC_Free_Message(A_messageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message of PRINT_JOB,N_EVENT_REPORT_RQ error",status);
                MC_Free_Message(&rspMessageID); 
                return MC_SYSTEM_ERROR; 
            }

            status = MC_Free_Message(&rspMessageID); 
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Free_Message of PRINT_JOB,N_EVENT_REPORT_RSP error",status);
                return MC_SYSTEM_ERROR; 
            }
        }
        else
        {
            return status;
        }
    }
}

 
/**************************************************************************
 *
 *  Function     :  Printer_N_GET
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  
 *  Returns      :  MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION 
 * 
 *  Description  :  This function gets the current printer status.
 *
 ****************************************************************************/
static MC_STATUS Printer_N_GET( 
                    int              A_applicationID,
                    int              A_associationID)
{
    int             messageID, responseMessageID;
    MC_STATUS       status;
    unsigned int    dicomStatus;
    char*           responseService;
    MC_COMMAND      responseCommand;
    char            tmp_buf[128];
    int             i;
    char            printerSOP[UID_LENGTH] = "1.2.840.10008.5.1.1.17";
    char            printerUID[UID_LENGTH] = "1.2.840.10008.5.1.1.16"; 
    unsigned long   tags[] ={ MC_ATT_PRINTER_STATUS,
                              MC_ATT_PRINTER_STATUS_INFO,
                              MC_ATT_PRINTER_NAME,
                              MC_ATT_MANUFACTURER,
                              MC_ATT_MANUFACTURERS_MODEL_NAME,
                              MC_ATT_DEVICE_SERIAL_NUMBER,
                              MC_ATT_SOFTWARE_VERSIONS,
                              MC_ATT_DATE_OF_LAST_CALIBRATION,
                              MC_ATT_TIME_OF_LAST_CALIBRATION}; 
                              
    /*
     *   Send the N-get Printer message
     */                                                                                                   
    status = MC_Open_Message(&messageID, "PRINTER", N_GET_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request PRINTER message",status);
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Set_Value_From_String(messageID, MC_ATT_REQUESTED_SOP_CLASS_UID, printerUID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for RQ_SOP_CLASS_UID failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID, printerSOP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for requested SOP_INSTANCE_UID failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    /* 
     * Create the attribute identifer list of all tags
     */
    for (i=0;i<sizeof(tags)/ sizeof(unsigned long);i++)
    {
        status = MC_Set_Next_Value_From_ULongInt(messageID,MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,tags[i]);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_ULongInt for attribute identifier list failed",status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }
    }
 
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message failed for PRINTER,N_GET_RQ",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID);  
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed for PRINTER,N_GET_RQ",status);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 30, &responseMessageID,
                                 &responseService, &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Local_Read_Message failed for PRINTER,N-GET-RSP",status);
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Get_Value_To_UInt ( responseMessageID, MC_ATT_STATUS, &dicomStatus );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintError ( "MC_Get_Value_To_UInt failed for PRINTER,N-GET-RSP",status );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    if ( dicomStatus != N_GET_SUCCESS 
      && dicomStatus != N_GET_WARNING_OPT_ATTRIB_UNSUPPORTED
      && dicomStatus != N_GET_ATTRIBUTE_LIST_ERROR)
    {
        printf ( "\n\nResponse message was not N_GET_SUCCESS: 0x%x\n", dicomStatus );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }
    
    printf("Printer N-GET status:\n");

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_PRINTER_STATUS,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tPrinter Status:           %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_PRINTER_STATUS_INFO,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
         printf("\tPrinter Status Info:      %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_PRINTER_NAME,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tPrinter Name:             %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_MANUFACTURER,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tManufacturer:             %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_MANUFACTURERS_MODEL_NAME,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tManufacturer Model Name:  %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_DEVICE_SERIAL_NUMBER,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tDevice Serial Number:     %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_SOFTWARE_VERSIONS,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tSoftware Version:         %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_DATE_OF_LAST_CALIBRATION,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tDate Last Calibration:    %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_TIME_OF_LAST_CALIBRATION,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tLast Clibration:          %s\n",tmp_buf);
        
    status = MC_Free_Message( &responseMessageID );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message PRINTER,N_GET_RSP failed",status);
    return ( MC_CANNOT_COMPLY );
    }

    return MC_NORMAL_COMPLETION;

}
    
    
#ifndef VXWORKS
/* Local Function Prototype */
static MC_STATUS PrintJob_N_GET( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_printeriJobUID );

/**************************************************************************
 *
 *  Function     :  PrintJob_N_GET
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  A_printJobUID   - The UID of the print job to get the
 *                                    status for.
 *                  
 *  Returns      :  MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION 
 * 
 *  Description  :  This function gets the status of a print job.  Note that
 *                  this function currently is not used by this sample 
 *                  application.  It was left in as an example.
 *
 ****************************************************************************/
static MC_STATUS PrintJob_N_GET( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_printJobUID )
{
    int             messageID, 
                    responseMessageID;
    MC_STATUS       status;
    char*           responseService;
    MC_COMMAND      responseCommand;
    char            tmp_buf[128];
    int             i;
    unsigned int    dicomStatus;
    unsigned long   tags[] ={ MC_ATT_EXECUTION_STATUS,
                              MC_ATT_EXECUTION_STATUS_INFO,
                              MC_ATT_PRINT_PRIORITY,
                              MC_ATT_CREATION_DATE,
                              MC_ATT_CREATION_TIME,
                              MC_ATT_PRINTER_NAME,
                              MC_ATT_ORIGINATOR }; 

    /*
     *   Send the N-get Print Job message
     */                                                                                                   
    status = MC_Open_Message(&messageID, "PRINT_JOB", N_GET_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message",status);
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Set_Value_From_String(messageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID, A_printJobUID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for AFF_SOP_INSTANCE_UID failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
    
    /* 
     * Create the attribute identifer list of all tags
     */
    for (i=0;i<sizeof(tags)/ sizeof(unsigned long);i++)
    {
        status = MC_Set_Next_Value_From_ULongInt(messageID,MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,tags[i]);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_ULongInt for attribute identifier list failed",status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }
    }
     
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message for PRINT_JOB,N_GET_RQ failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID);  
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message for PRINT_JOB,N_GET_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }


    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 
                                 30, 
                                 &responseMessageID,
                                 &responseService, 
                                 &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Local_Read_Message failed for PRINT_JOB,N-GET-RSP",status);
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Get_Value_To_UInt ( responseMessageID, MC_ATT_STATUS, &dicomStatus );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintError ( "MC_Get_Value_To_UInt failed for PRINTER,N-GET-RSP",status );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    if ( dicomStatus != N_GET_SUCCESS 
      && dicomStatus != N_GET_WARNING_OPT_ATTRIB_UNSUPPORTED
      && dicomStatus != N_GET_ATTRIBUTE_LIST_ERROR)
    {
        printf ( "\n\nResponse message was not N_GET_SUCCESS: 0x%x\n", dicomStatus );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }
    
    printf( "\nN_GET Print Job Status:%s\n", A_printJobUID );

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_EXECUTION_STATUS,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tExecution Status:         %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_EXECUTION_STATUS_INFO,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
         printf("\tExecution Status Info:    %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_PRINT_PRIORITY,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tPrint Priority:            %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_CREATION_DATE,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tCreation Date:            %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_CREATION_TIME,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tCreation Time:            %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_PRINTER_NAME,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tPrinter Name:             %s\n",tmp_buf);

    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_ORIGINATOR,
                                    sizeof(tmp_buf), tmp_buf);
    if (status == MC_NORMAL_COMPLETION)
        printf("\tOriginator:               %s\n",tmp_buf);


    status = MC_Free_Message( &responseMessageID );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message for PRINT_JOB,N_GET_RSP failed",status);
        return MC_CANNOT_COMPLY;
    }
    return MC_NORMAL_COMPLETION;

}
#endif /* ifndef VXWORKS */

/**************************************************************************
 *
 *  Function     :  FilmSession_N_CREATE
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  A_priority      - The priority for the film session                  
 *                  A_copies        - The number of copies of the file 
 *                                    session                  
 *                  
 *  Returns      :  MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 * 
 *  Description  :  This function sends an N_CREATE_RQ on the film session 
 *                  level
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_CREATE( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_priority,
                    int                A_copies)
{
    int             messageID,
                    responseMessageID;
    MC_STATUS       status;
    unsigned int    dicomStatus;
    char*           responseService;
    MC_COMMAND      responseCommand;
    VAL_ERR*        errPtr;
    
    /*
     *   Open the Film Session Creation message
     */
    status = MC_Open_Message(&messageID, "BASIC_FILM_SESSION", N_CREATE_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message:",status);
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Set_Value_From_Int(messageID, 
                                   MC_ATT_NUMBER_OF_COPIES,
                                   A_copies);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int for copies failed:",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    if ( A_priority[0] )
    {
        status = MC_Set_Value_From_String(messageID, MC_ATT_PRINT_PRIORITY,
                          A_priority);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_String for priority failed",status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }
    }



    /********************** PLEASE NOTE **********************/
    /***** The following set should not need to be done. *****/
    /***** It is a "bug" in the DICOM standard.  A data  *****/ 
    /***** set is required in a create message even      *****/
    /***** though the all values in a film session are   *****/
    /***** optional, so something has to be set.         *****/
    /***                                                   ***/

    strcpy(S_session_sop,"1.2.840.10008.5.1.1.1");
    status = MC_Set_Value_From_String(messageID, MC_ATT_AFFECTED_SOP_CLASS_UID,
                      S_session_sop);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for affected SOP_CLASS_UID failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Validate_Message(messageID, &errPtr, Validation_Level2);
    if (status != MC_MESSAGE_VALIDATES)
    {
        printf("MC_Validate_Message failed: %s\n", MC_Error_Message(status));
        printf("\t\tTag=         %ld\n", errPtr->Tag);
        printf("\t\tMsgItemID=   %d\n", errPtr->MsgItemID);
        printf("\t\tValueNumber= %d\n", errPtr->ValueNumber);
        printf("\t\tstatus=      %s\n", MC_Error_Message(errPtr->Status));
    }

 
    /* 
     *  Send the Film Session creation message
     */
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message for FILM_SESSION, N_CREATE_RQ failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
 
    status = MC_Get_Value_To_String(messageID, MC_ATT_AFFECTED_SOP_CLASS_UID,
                                    sizeof(S_session_sop), S_session_sop);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String of SOP for Film Session failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of FILM_SESSION,N_CREATE_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 30, &responseMessageID,
                                 &responseService, &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Local_Read_Message failed for FILM_SESSION,N-CREATE-RSP",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Get_Value_To_UInt ( responseMessageID, MC_ATT_STATUS, &dicomStatus );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintError ( "MC_Get_Value_To_UInt failed for FILM_SESSION,N-CREATE-RSP",status );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    if ( dicomStatus != N_CREATE_SUCCESS )
    {
        printf ( "\n\nResponse message was not N_CREATE_SUCCESS: 0x%x\n", dicomStatus );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    /* 
     * When the instance SOP is created on the Provider, this is necessary 
     * to determine the returned SOP
     */
    status = MC_Get_Value_To_String(responseMessageID, 
                                    MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                    sizeof(S_session_uid), S_session_uid);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String of SOP Instance for Film Session failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&responseMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message failed",status);
        return MC_CANNOT_COMPLY;
    }

    printf("Film Session Created!!\n");
 
    return MC_NORMAL_COMPLETION;
}



/**************************************************************************
 *
 *  Function     :  FilmBox_N_CREATE
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  A_format        - The format for the film box       
 *                  A_page          - The page number we're currently on.
 *                                    Note that this is for internal use.
 *                  
 *  Returns      :  MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 * 
 *  Description  :  This function sends an N_CREATE_RQ on the filmbox level
 *
 ****************************************************************************/
static MC_STATUS FilmBox_N_CREATE( 
                    int                A_applicationID,
                    int                A_associationID,
                    int                A_format,
                    int                A_page)
{
    int             messageID,
                    responseMessageID,
                    itemID;
    MC_STATUS       status;
    unsigned int    dicomStatus;
    char*           responseService;
    MC_COMMAND      responseCommand;
    char            tmp_buf[104];
    int             j;
    
        
    status = MC_Open_Message(&messageID, "BASIC_FILM_BOX", N_CREATE_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message",status);
        return MC_CANNOT_COMPLY;
    }
            
    sprintf(tmp_buf, "STANDARD\\");

    switch(A_format)
    {
        case 1:
                strcat(tmp_buf, "1,1");
                break;
        case 2:
                strcat(tmp_buf, "1,2");
                break;
        case 4:
                strcat(tmp_buf, "2,2");
                break;
        case 6:
                strcat(tmp_buf, "2,3");
                break;
        case 9:
                strcat(tmp_buf, "3,3");
                break;
        case 12:
                strcat(tmp_buf, "3,4");
                break;
        case 15:
                strcat(tmp_buf, "3,5");
                break;
        case 16:
                strcat(tmp_buf, "4,4");
                break;
        case 20:
                strcat(tmp_buf, "4,5");
                break;
        case 24:
                strcat(tmp_buf, "4,6");
                break;
        default:
                printf("Unsupported format used %d-up\n", A_format);
                return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_IMAGE_DISPLAY_FORMAT, tmp_buf);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for image display format failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Open_Item(&itemID, "REF_FILM_SESSION");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_Int(messageID, MC_ATT_REFERENCED_FILM_SESSION_SEQUENCE,
                                   itemID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int for the referenced film session sequence failed",status);
        MC_Free_Item(&itemID);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }    

    
    status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, S_session_sop);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for referenced SOP Class UID failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, S_session_uid);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for referenced SOP Instance UID failed",status);
        MC_Free_Message(&itemID);
        return MC_CANNOT_COMPLY;
    }
    
    /*
     * Send film box create message 
     */
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
             
    status = MC_Free_Message(&messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of FILM_BOX,N_CREATE_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }
            
    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 
                                 30, 
                                 &responseMessageID,
                                 &responseService, 
                                 &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Local_Read_Message failed for FILM_BOX,N-CREATE-RSP",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Get_Value_To_UInt ( responseMessageID, MC_ATT_STATUS, &dicomStatus );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintError ( "MC_Get_Value_To_UInt failed for FILM_BOX,N-CREATE-RSP",status );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    if ( dicomStatus != N_CREATE_SUCCESS )
    {
        printf ( "\n\nResponse message was not N_CREATE_SUCCESS: 0x%x\n", dicomStatus );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    } 
           
    status = MC_Get_Value_To_String(responseMessageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                    sizeof(tmp_buf), tmp_buf);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String of SOP instance of FILM BOX failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }
    strcpy(Films[A_page].film_uid, tmp_buf);
           
           
    /*
     * Retrieve the UIDs of each image box out of the response message.
     * These are used to do N-SET's for each image box later.
     */ 
    j = 0;
    while(MC_Get_Next_Value_To_Int(responseMessageID, MC_ATT_REFERENCED_IMAGE_BOX_SEQUENCE,
                                   &itemID) == MC_NORMAL_COMPLETION)
    {
        status = MC_Get_Value_To_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                        sizeof(tmp_buf), tmp_buf);
        if (status != MC_NORMAL_COMPLETION)
        {
            break;
        }

        strcpy(Films[A_page].image_uids[j++],tmp_buf);
    }
    Films[A_page].images = j;
                
    status = MC_Free_Message(&responseMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
    PrintError("MC_Free_Message of FILM_BOX,N_CREATE_RSP failed",status);
        return ( MC_CANNOT_COMPLY );
    }
    return MC_NORMAL_COMPLETION;
}


/**************************************************************************
 *
 *  Function     :  ImageBox_N_SET
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  A_imageBoxUID   - The UID of the image box to set       
 *                  A_image_position- The position on the film of this image
 *                                    box.
 *                  A_image_current - The current image number to set       
 *                  
 *  Returns      :  MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 * 
 *  Description  :  This function sends an N_SET_RQ on the imagebox level
 *
 ****************************************************************************/
static MC_STATUS ImageBox_N_SET( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_imageBoxUID,
                    int                A_image_position,
                    int                A_image_current)
{
    int             messageID,
                    responseMessageID,
                    itemID;
    MC_STATUS       status;
    unsigned int    dicomStatus;
    char*           responseService;
    MC_COMMAND      responseCommand;
    time_t          start_time;
    CBinfo          callbackInfo;
    unsigned long   errorTag;
    
     
    /*
     *  Open request message object
     */
    status = MC_Open_Message(&messageID, "BASIC_GRAYSCALE_IMAGE_BOX",
                     N_SET_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message",status);
        return MC_CANNOT_COMPLY;
    }
            
    time(&start_time);
            
    sprintf(S_fname, "%d.img", A_image_current);
            
    callbackInfo.stream = fopen(S_fname, BINARY_READ);
    if (!callbackInfo.stream)
    {
        printf("Cannot open image file [%s]\n", S_fname);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Open_Item(&itemID, "PREFORMATTED_GRAYSCALE_IMAGE");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_Int(messageID,
                                   MC_ATT_BASIC_GRAYSCALE_IMAGE_SEQUENCE, 
                                   itemID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed",status);
        MC_Free_Item(&itemID);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(itemID, MC_ATT_PIXEL_ASPECT_RATIO, "1");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed for the x pixel aspect ratio",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
            
    status = MC_Set_Next_Value_From_String(itemID, MC_ATT_PIXEL_ASPECT_RATIO, "1");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed for y pixel aspect ratio",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
            
    callbackInfo.messageID = itemID;

    /*
     *  Populate the message from the file with the group 
     *  0x0028 attributes from the message.
     */
    status = MC_Stream_To_Message( itemID,
                                   MC_ATT_GROUP_0028_LENGTH, MC_ATT_PIXEL_REPRESENTATION,
                                   transfer_syntax,
                                   &errorTag, (void*)&callbackInfo,
                                   StreamToMessageFunction);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Stream to message error:\n",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
    fclose(callbackInfo.stream);
    

    callbackInfo.stream = fopen(S_fname, BINARY_READ);
    if (!callbackInfo.stream)
    {
        printf("Cannot open image file [%s]\n", S_fname);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    /*
     * Add the window center to the image box.  Note that although
     * this is not required by DICOM it is very useful to 
     * print SCPs
     */
    status = MC_Stream_To_Message( itemID,
                                   MC_ATT_WINDOW_CENTER, 
                                   MC_ATT_WINDOW_CENTER_WIDTH_EXPLANATION,
                                   transfer_syntax,
                                   &errorTag, 
                                   (void*)&callbackInfo,
                                   StreamToMessageFunction);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Stream to message error",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
    fclose(callbackInfo.stream);
            
    callbackInfo.stream = fopen(S_fname, BINARY_READ);
    if (!callbackInfo.stream)
    {
        printf("Cannot open image file [%s]\n", S_fname);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Stream_To_Message( itemID,
                                   MC_ATT_PIXEL_DATA, 
                                   MC_ATT_PIXEL_DATA,
                                   transfer_syntax,
                                   &errorTag, 
                                   (void*)&callbackInfo,
                                   StreamToMessageFunction);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Stream to message error:\n",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
    
    fclose(callbackInfo.stream);
            
        
    status = MC_Set_Value_From_Int(messageID, MC_ATT_IMAGE_POSITION,
                                   A_image_position + 1);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int for image position failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
        
            
    status = MC_Set_Value_From_String(messageID,
                                      MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                      A_imageBoxUID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for the image box sop instance uid failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
        
    /*
     *  Send the message
     */
    printf("\tSending %s (%s):", S_fname, "BASIC_GRAYSCALE_IMAGE_BOX");

    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
        
    status = MC_Free_Message(&messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_GRAYSCALE_IMAGE_BOX,N_SET_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 
                                 30, 
                                 &responseMessageID,
                                 &responseService, 
                                 &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("\nMC_Local_Read_Message failed for IMAGE_BOX,N-SET-RSP",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Get_Value_To_UInt ( responseMessageID, MC_ATT_STATUS, &dicomStatus );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintError ( "MC_Get_Value_To_UInt failed for IMAGE_BOX,N-SET-RSP",status );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    if ( dicomStatus != N_SET_SUCCESS )
    {
        printf ( "\n\nResponse message was not N_SET_SUCCESS: 0x%x\n", dicomStatus );
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    } 
           
    /*
     *  How long did it take?
     */
    printf("Elapsed time: %d seconds\n",
            (int)(time((time_t *)NULL) - start_time));

    status = MC_Free_Message(&responseMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_GRAYSCALE_IMAGE_BOX,N_SET_RSP failed",status);
        return MC_CANNOT_COMPLY;
    }


    return MC_NORMAL_COMPLETION;
}



/**************************************************************************
 *
 *  Function     :  FilmBox_N_ACTION
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  A_filmUID       - The UID of the film box to print       
 *                  
 *  Returns      :  MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 * 
 *  Description  :  This function sends an N_ACTION_RQ on the filmbox level
 *
 ****************************************************************************/
static MC_STATUS FilmBox_N_ACTION( 
                    int                A_applicationID,
                    int                A_associationID,
                    char*              A_filmUID)
{
    int             messageID,
                    responseMessageID,
                    itemID;
    MC_STATUS       status;
    char*           responseService;
    MC_COMMAND      responseCommand;
    int             responseStatus;
    char            tmp_buf[104];


    status = MC_Open_Message(&messageID, "BASIC_FILM_BOX",
                             N_ACTION_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open BASIC_FILM_BOX request message",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                      A_filmUID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for requested sop instance uid failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_Int(messageID, MC_ATT_ACTION_TYPE_ID,1);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int for action type id failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Send the message
     */
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message for BASIC_FILM_BOX failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_FILM_BOX,N_ACTION_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 
                                 30, 
                                 &responseMessageID,
                                 &responseService, 
                                 &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Local_Read_Message failed for FILM_BOX,N-ACTION-RSP",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Get_Value_To_Int(responseMessageID,MC_ATT_STATUS,&responseStatus);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_Int for FILM_BOX,N-ACTION-RSP status failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    switch (responseStatus)
    {
        case N_ACTION_NO_SUCH_SOP_INSTANCE:
            printf("FILM_BOX, N-ACTION-RSP failed because of invalid UID\n");
            MC_Free_Message(&responseMessageID);
            return MC_CANNOT_COMPLY;
        case N_ACTION_PROCESSING_FAILURE:
            printf("FILM_BOX, N-ACTION-RSP failed because of processing failure\n");
            MC_Free_Message(&responseMessageID);
            return MC_CANNOT_COMPLY;
        case N_ACTION_SUCCESS:
            break;
        default:
            printf("FILM_BOX, N-ACTION-RSP failure, status=%x\n",responseStatus);
            MC_Free_Message(&responseMessageID);
            return MC_CANNOT_COMPLY;
    }

    if( S_printJob_Negotiated ) 
    {
        status = MC_Get_Value_To_Int( responseMessageID,
                                      MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE, 
                                      &itemID);
        while( status == MC_NORMAL_COMPLETION )
        {
            status = MC_Get_Value_To_String( itemID,
                                             MC_ATT_REFERENCED_SOP_INSTANCE_UID,  
                                             sizeof(tmp_buf), 
                                             tmp_buf);
            if( status != MC_NORMAL_COMPLETION )
            {
                PrintError("Unable to get referenced print job uid",status);
                MC_Free_Message(&responseMessageID);
                return MC_CANNOT_COMPLY;
            }
            
            /*
             *  Create a print job record to track the progress
             *  of the print job created by the SCP
             */
            if( !CreatePrintJobRecord( tmp_buf, 1 ) )
            {
                printf("Unable to add UID to print job pending list\n");
                MC_Free_Message(&responseMessageID);
                return MC_CANNOT_COMPLY;
            }
            status = MC_Get_Next_Value_To_Int( responseMessageID,
                                               MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE, 
                                               &itemID);
        }
    }


    status = MC_Free_Message(&responseMessageID); 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_FILM_BOX,N_ACTION_RSP failed",status);
        return MC_CANNOT_COMPLY;
    }


    return MC_NORMAL_COMPLETION;            
}            



/**************************************************************************
 *
 *  Function     :  FilmSession_N_ACTION
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  
 *  Returns      :  MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 * 
 *  Description  :  This function sends an N_ACTION_RQ on the film
 *                  session evel
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_ACTION( 
                    int                A_applicationID,
                    int                A_associationID)
{
    int             messageID,
                    responseMessageID,
                    itemID;
    MC_STATUS       status;
    char*           responseService;
    MC_COMMAND      responseCommand;
    char            tmp_buf[104];
    int             responseStatus;


    status = MC_Open_Message(&messageID, "BASIC_FILM_SESSION",
                             N_ACTION_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message for BASIC_FILM_SESSION",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                  S_session_uid);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed for requested sop instance uid",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_Int(messageID, MC_ATT_ACTION_TYPE_ID,
                                   1);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int failed for action type id",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Send the message
     */
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message for BASIC_FILM_SESSION failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_FILM_SESSION,N_ACTION_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Wait for response
     */
    status = Local_Read_Message( A_applicationID,
                                 A_associationID, 
                                 30, 
                                 &responseMessageID,
                                 &responseService, 
                                 &responseCommand);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Local_Read_Message failed for BASIC_FILM_SESSION,N_ACTION_RSP",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Get_Value_To_Int(responseMessageID,MC_ATT_STATUS,&responseStatus);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_Int for FILM_BOX,N-ACTION-RSP status failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }

    switch (responseStatus)
    {
        case N_ACTION_NO_SUCH_SOP_INSTANCE:
            printf("FILM_SESSION, N-ACTION-RSP failed because of invalid UID\n");
            MC_Free_Message(&responseMessageID);
            return MC_CANNOT_COMPLY;
        case N_ACTION_PROCESSING_FAILURE:
            printf("FILM_SESSION, N-ACTION-RSP failed because of processing failure\n");
            MC_Free_Message(&responseMessageID);
            return MC_CANNOT_COMPLY;
        case N_ACTION_SUCCESS:
            break;
        default:
            printf("FILM_SESSION, N-ACTION-RSP failure, status=%x\n",responseStatus);
            MC_Free_Message(&responseMessageID);
            return MC_CANNOT_COMPLY;
    }
    
    
    if( S_printJob_Negotiated ) 
    {
        status = MC_Get_Next_Value_To_Int( responseMessageID,
                                           MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE, &itemID);
        while( status == MC_NORMAL_COMPLETION )
        {
            status = MC_Get_Value_To_String( itemID,
                                             MC_ATT_REFERENCED_SOP_INSTANCE_UID,  
                                             sizeof(tmp_buf), tmp_buf);
            if( status != MC_NORMAL_COMPLETION )
            {
                PrintError("Unable to get print job uid from N-ACTION-RQ message",status);
                MC_Free_Message(&responseMessageID);
                return MC_CANNOT_COMPLY;
            }
            /*
             *  Create a print job record to track the progress
                *  of the print job created by the SCP
             */
            if( !CreatePrintJobRecord( tmp_buf, 1 ) )
            {
                printf("Unable to add UID to print job pending list\n");
                MC_Free_Message(&responseMessageID);
                return MC_CANNOT_COMPLY;
            }
            status = MC_Get_Next_Value_To_Int( responseMessageID,
                                               MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE, 
                                               &itemID);
        }
    }

    status = MC_Free_Message(&responseMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_FILM_SESSION,N_ACTION_RSP failed",status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}



/**************************************************************************
 *
 *  Function     :  FilmSession_N_DELETE
 * 
 *  Parameters   :  A_applicationID - The application ID of MERGE_PRINT_SCU
 *                  A_associationID - The current association ID
 *                  
 *  Returns      :  MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 * 
 *  Description  :  This function sends an N_DELETE_RQ on the film session
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_DELETE( 
                    int                A_applicationID,
                    int                A_associationID)
{
    int             messageID;
    MC_STATUS       status;


    status = MC_Open_Message(&messageID, "BASIC_FILM_SESSION",
                             N_DELETE_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open request message",status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                      S_session_uid);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for sop instance uid failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    /*
     *  Send the message
     */
    status = MC_Send_Request_Message(A_associationID, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Request_Message failed",status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message of BASIC_FILM_SESSION,N_DELETE_RQ failed",status);
        return MC_CANNOT_COMPLY;
    }

    /*
     * Note that the N-DELETE-RSP will be handled outside of this function
     */
    return MC_NORMAL_COMPLETION;
}



/**************************************************************************
 *
 *  Function     : CreatePrintJobRecord
 * 
 *  Parameters   :  A_jobUID, the ID of the print job to create a record for
 *                  A_state, the state to set the thread to
 *                  
 *  Returns      :  0 if failure
 *                  1 if success
 * 
 *  Description  :  This function creates the an internal
 *                  record for a print job and sets the state for the record.
 *
 ****************************************************************************/
static int CreatePrintJobRecord(char* A_jobUID, int A_status)
{
    printjob_t*  ptr;

    ptr = (printjob_t*)malloc(sizeof(printjob_t));
    
    if (!ptr)
    {
        printf("Unable to malloc memory\n");
        return FALSE;
    }


    /*
     * Initialize the new  record
     */       
    strcpy(ptr->jobUID,A_jobUID);     
    ptr->status = A_status;        
    ptr->next = NULL;
    
    printf("Print Job created: %s\n",ptr->jobUID); 

    /*
     * Check if no records in the linked list.  ie, this is the
     * first time the function is called.
     */       
    if (!S_pj.head)     
    { 
        S_pj.head = ptr;
        S_pj.tail = ptr;
        ++S_pj.jobCount;
        return TRUE;
    }

   
    /*
     * Do a sanity check to be sure the tail node is initialized
     * correctly
     */ 
    if (S_pj.tail->next)
    {
        printf("The applications internal data structures contained an error\n");
        free(ptr);
        return FALSE;
    }

    /*
     * Place the new record at the end of the list.
     */    
    S_pj.tail->next = ptr;
    S_pj.tail = ptr;
    
    ++S_pj.jobCount;
    
    return TRUE;
}


/**************************************************************************
 *
 *  Function     :  DeletePrintJobRecord
 * 
 *  Parameters   :  A_jobUID, the ID of the print job to delete the record
 *                  
 *  Returns      :  0 if failure
 *                  1 if success
 * 
 *  Description  :  This function removes a print job record from our records.
 *                   
 *
 ****************************************************************************/
static int DeletePrintJobRecord(char* A_jobUID)
{
    printjob_t*  ptr = S_pj.head;
    printjob_t*  prevPtr = S_pj.head;

    /*
     * Traverse through the linked list to find the record
     * (by printjob UID)
     */    
    while (ptr)
    {
        if (!strcmp(ptr->jobUID,A_jobUID))
        {
            /*
             * Check if first record is the print job record to delete
             */ 
            if (S_pj.head == ptr)
            {
                S_pj.head = S_pj.head->next;
                if (S_pj.head)
                {
                   if (!S_pj.head->next)
                       S_pj.tail = NULL;
                }
            }
            else
            {
                if (S_pj.tail == ptr)
                    S_pj.tail = prevPtr;
                prevPtr->next = ptr->next;
            }
            
            free(ptr);
            --S_pj.jobCount;
            return TRUE;
        }
        prevPtr = ptr;
        ptr = ptr->next;
    }
    return TRUE;
}


/**************************************************************************
 *
 *  Function     :  SetPrintJobStatus
 * 
 *  Parameters   :  A_jobUID, the UID of the print job to change the state
 *                  A_status, the state to set the print job to
 *                  
 *  Returns      :  0 if failure
 *                  1 if success
 * 
 *  Description  :  This function sets the application's internal
 *                  state for a print job.
 *
 ****************************************************************************/  
static int SetPrintJobStatus(char*     A_jobUID, 
                              int      A_status) 
{
    printjob_t*  ptr;
    
    /*
     * Traverse through the list to find the print job record.
     */ 
    ptr = S_pj.head;
    
    while (ptr)
    {
        if (!strcmp(ptr->jobUID,A_jobUID))
        {
            ptr->status = A_status;
            return (TRUE); 
        }
        ptr = ptr->next;
    }
    return (FALSE);
}

/**************************************************************************
 *
 *  Function     :  CheckForPrintJob
 * 
 *  Parameters   :  A_jobUID, the UID of the print job to look for
 *                  
 *  Returns      :  0 if failure
 *                  1 if success
 * 
 *  Description  :  This function looks for a record for a given print job
 *
 ****************************************************************************/  
static int CheckForPrintJob(char*     A_jobUID)
{
    printjob_t*  ptr;
    
    /*
     * Traverse through the list to find the print job record.
     */ 
    ptr = S_pj.head;
    
    while (ptr)
    {
        if (!strcmp(ptr->jobUID,A_jobUID))
        {
            return (TRUE); 
        }
        ptr = ptr->next;
    }
    return (FALSE);
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
    printf("\t%s:\n",A_string);
    printf("\t\t%s\n", MC_Error_Message(A_status));
}


