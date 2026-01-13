/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Tool Kit Print SCP Sample App
 *
 *    $Workfile: prnt_scp.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is a sample service class provider application
 *               for the Print Management Service Class
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
    !defined(VXWORKS) && !defined (_MACINTOSH)
#define UNIX            1
#endif

#if defined(MACH_NEXT)
#undef UNIX
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

#if defined(INTEL_WCC)
#ifdef NO_EXT_KEYS
#undef NO_EXT_KEYS
#endif
#endif

#if defined(_MSDOS) || defined(_WIN32) || defined(INTEL_WCC)
#if !defined(_PHAR_LAP)
#include <conio.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(UNIX)
#ifdef LYNX_OS
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#endif

#include "mergecom.h"
#include "mcstatus.h"
#include "prnt_svc.h"
#include "diction.h"
#include "mc3msg.h"


/*
 *  Local constant definitions
 */
#define FAIL 0
#define SUCCESS 1
#define UID_LENGTH 100

/*
 *  Structure passed to our callback functions
 */
typedef struct CALLBACKINFO
{        
    FILE*  stream;
    int    messageID;
    char*  serviceName;
    char   prefix[30];
} CBinfo;

typedef struct datatable_struct
{
    unsigned long Tag;
    char*         Value;
} DataTable;

typedef struct printjob_struct
{
    char       jobUID[UID_LENGTH];            /* The UID of the print job for this record */
    char       executionStatusInfo[50]; 
    char       executionStatus[50]; 
    char       printPriority[64+1];
    char       creationDate[64+1];
    char       creationTime[64+1];
    char       printerName[64+1];
    char       originatorAE[16+1];

    struct printjob_struct* next;   /* pointer to the next thread structure in the list. */

} printjob_t;

typedef struct pj_static_struct
{    
    printjob_t* head;               /* the head of the print job list */
    printjob_t* tail;               /* the tail of the print job list */
    int       jobCount;        /* the count of the number of printjobs */
} pj_static_t;

pj_static_t S_pj;


#define TRUE 1
#define FALSE 0

/*
 *  Module static variables
 */
static FILM_SESSION *Ssession_head;
static FILM_BOX     *Sfilm_head;
static IMAGE_BOX    *Simage_head;
static char          Sprefix[40];
static int           SassociationID;
static int           S_printJob_Negotiated = FALSE; 

#ifdef UNIX
#define MAX_CHILD_SERVERS 5
static pid_t    Schild_PID[MAX_CHILD_SERVERS] = {0};
static int    server_quota = MAX_CHILD_SERVERS;  /* Max children */    
#endif


/*
 *  Module function prototypes
 */
static int  Get_Options(
                int              argc, 
                char**           argv);
               
static void Handle_Association(
                int*             A_associationID);

static printjob_t* CreatePrintJobRecord(
                char*            A_jobUID);

static printjob_t* GetPrintJobRecord(
                char*            A_jobUID);

static MC_STATUS PrintJob_N_GET(
                int              A_associationID, 
                int              A_messageID);

static MC_STATUS Printer_N_GET(
                int              A_associationID,
                int              A_messageID);

static short Print_Film(
                FILM_BOX*        A_fbp);

static short Delete_Film(
                FILM_BOX*        A_fbp);
                
static IMAGE_BOX* Image_Create(
                char*            A_Film_UID, 
                short            A_image_position);
                
static short Image_Delete(
                IMAGE_BOX*       A_ibp);

static short Image_Print(
                IMAGE_BOX*       A_ibp);
                
static void PrintError(
                char*            A_string, 
                MC_STATUS        A_status);

static char *Create_Inst_UID( void );
                           

#ifdef UNIX
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static int shandler( int Asigno );
#else
static int shandler (
               int              Asigno,
               long             Acode,
               struct sigcontext *Ascp);
#endif

#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void sigint_routine( int Asigno );
#else
static void sigint_routine  (  
               int              Asigno,
               long             Acode,
               struct sigcontext *Ascp);
#endif

#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void reaper( int Asigno );
#else
static void reaper (
               int              Asigno,
               long             Acode,
               struct sigcontext *Ascp);
#endif
#endif


/****************************************************************************
 *
 *  Function    :   main
 *
 *  Parameters  :   argc - the number of commandline arguments
 *                  argv - the command line arguments
 *
 *  Returns     :   EXIT_FAILURE
 *                  EXIT_SUCCESS
 *
 *  Description :   Loops waiting for associations or quit key press
 *
 ****************************************************************************/
#ifdef VXWORKS
int prntscp(int argc, char** argv);
int prntscp(int argc, char** argv)
#else
int  main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    int            applicationID;
    int            calledApplicationID;
    MC_STATUS      status;
    
#if defined(_MSDOS)    || defined(_WIN32)   || defined(_RMX3) || \
    defined(INTEL_WCC) || defined(OS9_68K) || defined(__OS2__)|| \
    defined(MACH_NEXT) || defined(OS9_PPC) || defined(VXWORKS)
    int            quit;
#elif defined(_MACINTOSH)
    EventRecord    asEvent;
    Boolean        aqQuit = false;    
#else
    static int     quota_message = 1;
    int            child;
    pid_t          childs_pid;
#endif    
    
#if defined(_MACINTOSH) && defined(__MWERKS__)
	SIOUXSettings.initializeTB = true;
	SIOUXSettings.standalone = true;
	SIOUXSettings.setupmenus = true;
	SIOUXSettings.autocloseonquit = false;
	SIOUXSettings.asktosaveonclose = true;
	SIOUXSettings.showstatusline = true;
	argc = ccommand(&argv);
#endif
    
    printf("prnt_scp:\n");
    
    if (Get_Options(argc, argv) != SUCCESS)
        exit( EXIT_FAILURE );
    
    printf("\n");
#if defined(_MSDOS)    || defined(_WIN32)  || defined(_RMX3)   || \
    defined(INTEL_WCC) || defined(OS9_68K) || defined(__OS2__) || \
	defined(VXWORKS)

    printf("\tPress 'Q' or Esc to cancel server and\n");
    printf("\tserver will stop when session ends.\n");
    quit = 0;
#elif defined (_MACINTOSH)
    printf("\tPress Q, q, or CMD-. to cancel server\n");
#else
    printf("\tPress Control-C to cancel server\n");
#endif        


    /*
     *  Initialize the MergeCOM-3 library
     */
    status = MC_Library_Initialization ( NULL, NULL, NULL );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to initialize the library!",status);
        exit( EXIT_FAILURE );
    }

    /*
     *  Register this DICOM application
     */
    status = MC_Register_Application(&applicationID, "MERGE_PRINT_SCP");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to register \"MERGE_PRINT_SCP\"",status);
        exit( EXIT_FAILURE );
    }


#ifdef UNIX    
    /*
      *  Handle change in states of our children
      */
    signal(SIGCHLD, (void (*)())reaper);


    /*
     *  Parent will handle shutdown
     */
    signal(SIGINT, (void (*)())sigint_routine);


    /*
     *  Handle situation of a process continuing to send data on a closed
     *  connection.
     */
    signal(SIGPIPE, (void (*)())shandler);


    /*
     *  Go back to the proper ui so we don't mess with things we don't own.
     */
    setuid(getuid());


#endif    /* UNIX only */

    /*
     *  Loop, handling associations - waiting for quit keypress
     */
    for (;;)
    {        
#if defined(_MSDOS) || defined(_WIN32)
#if defined(INTEL_BCC)
        if (kbhit())
#else
        if (_kbhit())
#endif
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
#elif defined(INTEL_WCC)
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
#elif defined(_MACINTOSH)
        if (WaitNextEvent(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask,
								&asEvent, 10, NULL)) 
	    {
            if (asEvent.what == keyDown) {
                unsigned char    abChar = asEvent.message & charCodeMask;
                Boolean            aqCmnd = ((asEvent.modifiers & cmdKey) != 0);
                
                if ((abChar == 'Q') || (abChar == 'q') || (aqCmnd && (abChar == '.')))
                    aqQuit = true;
            } else if (asEvent.what == mouseDown) {
                WindowPtr    apWindow;
                short            awWhere;
                
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
        status = MC_Wait_For_Association("Print_Service_List", 0,
                        &calledApplicationID,
                        &SassociationID);
        if (status == MC_TIMEOUT
         || status == MC_NEGOTIATION_ABORTED )
            continue;
            
            
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Error on MC_Wait_For_Association",status);
            printf("\t\tProgram aborted.\n");
            break;
        }
        
        if (calledApplicationID != applicationID)
        {
            printf("\tUnexpected application identifier on MC_Wait_For_Association.\n");
            printf("\t\tProgram aborted.\n");
            break;
        }

#ifndef UNIX    /* no forking necessary */
        Handle_Association(&SassociationID);

#else        /* fork child to process the association */
        for (child = 0; child < server_quota; child++)
        {
            if (!Schild_PID[child])
                break;     /* We found an available slot */
        }
        
        if (child >= server_quota)
        {
            (void)MC_Reject_Association(SassociationID, TRANSIENT_LOCAL_LIMIT_EXCEEDED);
            if (quota_message)
            {
                quota_message = 0;
                printf ("Server Quota reached at least once");
            }
            continue;    /* Wait for next association request */
        }
        
        /*
         *  Now fork a child process to handle the association
         */
        childs_pid = fork();
        
        switch (childs_pid)
        {
            case    0:      /* child */
                /*
                 *  Children will ignore interrupts
                 */
                signal(SIGINT, SIG_IGN);
                signal(SIGCHLD, SIG_DFL);
                
                Handle_Association(&SassociationID);
                exit(EXIT_SUCCESS);         /* Children go away when they are done */
                break;
            
            case    -1:     /* error */
                printf("\tUnable to fork child process\n");
                printf("\t\tProgram aborted.\n");
                exit(EXIT_FAILURE);
            
            default :       /* parent */
                Schild_PID[child] = childs_pid;
                break;
        }
        
        /*
         *  Release parent's association resources
         */
        (void)MC_Release_Parent_Association(&SassociationID);
#endif

#if defined (VXWORKS)
        if ( quit == 1 )
            break;
#endif
       
    }    /* Loop till quit requested or error */

    (void)MC_Release_Application(&applicationID);
    
    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");

    printf("End of program\n");
    return EXIT_SUCCESS;
}   /* END main() */




/****************************************************************************
 *
 *  Function    :   Get_Options
 *
 *  Parameters   :  argc - The number of command line arguments
 *                  argv - An array of command line arguments
 *                  
 *  Returns      :  FAIL
 *                  SUCCESS
 * 
 *  Description :   Decodes command line arguments
 *
 ****************************************************************************/
static int Get_Options(int argc, char** argv)
{
    if(argc > 1)
    {
        printf("\tUsage:  prnt_scp\n");
        return(FAIL);
    }

    return(SUCCESS);
}


/****************************************************************************
 *
 *  Function    :   Parse_Service
 *
 *  Parameters   :  serviceID - A string containing a MergeCOM-3 service 
 *                              name
 *                  
 *  Returns      :  PRNT_PRINTER
 *                  PRNT_BASIC_GRAYSCALE_IMAGE_BOX
 *                  PRNT_BASIC_FILM_SESSION
 *                  PRNT_BASIC_FILM_BOX
 *                  PRNT_PRINT_JOB
 * 
 *  Description :   Processes a service ID and returns a short identifier
 *
 ****************************************************************************/
static short Parse_Service(char* serviceID)
{
    if(strcmp(serviceID, "PRINTER") == 0)
    {
        return(PRNT_PRINTER);
    }
    else
    if(strcmp(serviceID, "BASIC_GRAYSCALE_IMAGE_BOX") == 0)
    {
        return(PRNT_BASIC_GRAYSCALE_IMAGE_BOX);
    }
    else
    if(strcmp(serviceID, "BASIC_FILM_SESSION") == 0)
    {
        return(PRNT_BASIC_FILM_SESSION);
    }
    else
    if(strcmp(serviceID, "BASIC_FILM_BOX") == 0)
    {
        return(PRNT_BASIC_FILM_BOX);
    }
    else
    if(strcmp(serviceID, "PRINT_JOB") == 0)
    {
        return(PRNT_PRINT_JOB);
    }

    return(-1);
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
 *                  for creating UIDs within DICOM.  
 *
 ****************************************************************************/
static char * Create_Inst_UID()
{
    static short PRINT_UID_CNTR = 0;
    static char  Sprnt_uid[68];
    
    sprintf(Sprnt_uid, "2.16.840.1.999999.%d", PRINT_UID_CNTR++);
    return(Sprnt_uid);
}



/****************************************************************************
 *
 *  Function    :   Send_Empty_Response
 *
 *  Parameters  :  A_associationID - The association over which to send the
 *                                   reply message
 *                 A_serviceName   - The service name for which to send a
 *                                   reply message
 *                 A_command       - The DICOM command type to send a reply
 *                                   message for
 *                 A_responseStatus - The response status to send for the
 *                                    reply
 *
 *  Returns     :  MC_CANNOT_COMPLY
 *                 MC_NORMAL_COMPLETION
 *                  
 *  Description :   Sends an empty response message to the SCU.
 *
 ****************************************************************************/
static MC_STATUS Send_Empty_Response(
            int         A_associationID,
            char*       A_serviceName,
            MC_COMMAND  A_command,
            RESP_STATUS A_responseStatus)
{
    MC_STATUS status;
    int       messageID;
     
    /* 
     * Acquire a response message object 
     */
    status = MC_Open_Message (&messageID, A_serviceName, A_command);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Send_Response_Message(A_associationID,
                                      A_responseStatus,
                                      messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&messageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}



/**************************************************************************
 *
 *  Function     :  Printer_N_GET
 * 
 *  Parameters   :  A_associationID - The association over which the 
 *                                    the N_GET_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *
 *  Returns      :  MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description  :  This function handles when an SCU requests a printer's
 *                  status
 *
 ****************************************************************************/
static MC_STATUS Printer_N_GET(int A_associationID,
                               int A_messageID)
{
    DataTable   data[] = { {MC_ATT_PRINTER_STATUS, "NORMAL"},
                           {MC_ATT_PRINTER_STATUS_INFO, "NONE"},
                           {MC_ATT_PRINTER_NAME, "MERGE_PRINTER"},
                           {MC_ATT_MANUFACTURER, "MERGE EFILM"},
                           {MC_ATT_MANUFACTURERS_MODEL_NAME, "MERGE DEMO PRINTER"},
                           {MC_ATT_DEVICE_SERIAL_NUMBER, "00001"},
                           {MC_ATT_SOFTWARE_VERSIONS, "Version 1.0"},
                           {MC_ATT_DATE_OF_LAST_CALIBRATION,"19960101" },
                           {MC_ATT_TIME_OF_LAST_CALIBRATION,"120000" }   };
                           
    int         responseMessageID;
    MC_STATUS   status; 
    int         i;
    unsigned long tag;
    
     
    /*
     *  Acquire a response message object
     */
    status = MC_Open_Message (&responseMessageID, "PRINTER", N_GET_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    /*
     * Get the attribute identifier list, and copy the values for these fields into
     * the response message
     */
    status = MC_Get_Value_To_ULongInt(A_messageID,MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,&tag);
    while (status == MC_NORMAL_COMPLETION)
    {
        /*
         * Search for the tag in our tag list
         */
        for (i=0;i< (sizeof(data) / sizeof(DataTable));i++)
        {
            if (tag == data[i].Tag)
            {
                status = MC_Set_Value_From_String(responseMessageID, tag, data[i].Value);
                if (status != MC_NORMAL_COMPLETION)
                {
                    PrintError("MC_Set_Value_From_String error", status);
                    /*
                     * Empty out the response, and sent failure processing
                     */
                    MC_Empty_Message(responseMessageID);
                    
                    status = MC_Send_Response_Message(A_associationID,
                                                      N_GET_PROCESSING_FAILURE, responseMessageID);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("MC_Send_Response_Message error", status);
                    }
                     
                    MC_Free_Message(&responseMessageID); /* Free the response object */ 
                    return MC_CANNOT_COMPLY;
                }
                break;
            }
        }
        if (i >= sizeof(data) / sizeof(DataTable))
        {
            /*
             * Unknown tag encountered, this should be a failure
             */
            MC_Empty_Message(responseMessageID);
                
            printf("\nERROR: Unknown tag encountered in attribute list: %lx\n",tag); 
                               
            status = MC_Send_Response_Message(A_associationID,
                                              N_GET_ATTRIBUTE_LIST_ERROR, responseMessageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Send_Response_Message error", status);
            }
            MC_Free_Message(&responseMessageID); /* Free the response object */ 
                 
            return MC_CANNOT_COMPLY;
        } 
        status = MC_Get_Next_Value_To_ULongInt(A_messageID,MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,&tag);
    }
    
    if (status != MC_NORMAL_COMPLETION && status != MC_NO_MORE_VALUES)
    {
        MC_Empty_Message(responseMessageID);

        PrintError("MC_Get_Value_To_ULongInt for attribute identifier list error", status);
                
        status = MC_Send_Response_Message(A_associationID,
                                          N_GET_ATTRIBUTE_LIST_ERROR, responseMessageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", status);
        }
                 
        MC_Free_Message(&responseMessageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    } 
    
    status = MC_Send_Response_Message(A_associationID,
                                      N_GET_SUCCESS, responseMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&responseMessageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Free_Message(&responseMessageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}




/**************************************************************************
 *
 *  Function     :  PrintJob_N_GET
 * 
 *  Parameters   :  A_associationID - The association over which the 
 *                                    the N_GET_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *
 *  Returns      :  MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description  :  This function retrieves a print job's status
 *
 ****************************************************************************/
static MC_STATUS PrintJob_N_GET(
            int         A_associationID, 
            int         A_messageID)
{
    char        uidBuffer[68];
    printjob_t* ptr;
    DataTable   data[] = { {MC_ATT_EXECUTION_STATUS, NULL},
                           {MC_ATT_EXECUTION_STATUS_INFO, NULL},
                           {MC_ATT_PRINT_PRIORITY, NULL},
                           {MC_ATT_CREATION_DATE, NULL},
                           {MC_ATT_CREATION_TIME, NULL},
                           {MC_ATT_PRINTER_NAME, NULL},
                           {MC_ATT_ORIGINATOR, NULL} }; 
    int       responseMessageID;
    MC_STATUS status; 
    int       i;
    unsigned long tag;
   
     
    /*
     *  Acquire a response message object
     */
    status = MC_Open_Message (&responseMessageID, "PRINT_JOB", N_GET_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }


    /* get affected SOP instance UID */
    status = MC_Get_Value_To_String(A_messageID, 
                                    MC_ATT_REQUESTED_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),
                                    uidBuffer);
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintError("MC_Get_Value_To_String failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Set_Value_From_String(responseMessageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID,
               uidBuffer);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String failed",status);
        MC_Free_Message(&responseMessageID);
        return MC_CANNOT_COMPLY;
    }
    
    
    ptr = GetPrintJobRecord(uidBuffer); 
    if (ptr)
    {
        /*
         * Assign pointers to the various status fields for the print job.
         * This is done so that we can generically assign the values
         * based on the loop below.
         */
        data[0].Value = ptr->executionStatus; 
        data[1].Value = ptr->executionStatusInfo; 
        data[2].Value = ptr->printPriority;
        data[3].Value = ptr->creationDate;
        data[4].Value = ptr->creationTime;
        data[5].Value = ptr->printerName;
        data[6].Value = ptr->originatorAE;     
    }
    else
    {
        /*
         * A matching print job was not found, send a failure response
         * message.
         */
        status = MC_Send_Response_Message(A_associationID,
                                          N_GET_PROCESSING_FAILURE, responseMessageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", status);
        } 
        MC_Free_Message(&responseMessageID); /* Free the response object */ 
    
        return MC_CANNOT_COMPLY;
    } 
    
    /*
     * Get the attribute identifier list, and copy the values for these fields into
     * the response message
     */
    status = MC_Get_Value_To_ULongInt(A_messageID,MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,&tag);
    while (status == MC_NORMAL_COMPLETION)
    {
        /*
         * Search for the tag in our tag list
         */
        for (i=0;i< (sizeof(data) / sizeof(DataTable));i++)
        {
            if (tag == data[i].Tag)
            {
                status = MC_Set_Value_From_String(responseMessageID,  tag,data[i].Value);
                if (status != MC_NORMAL_COMPLETION)
                {
                    PrintError("MC_Set_Value_From_String error", status);
                    /*
                     * Empty out the response, and sent failure processing
                     */
                    MC_Empty_Message(responseMessageID);
                    
                    status = MC_Send_Response_Message(A_associationID,
                                                      N_GET_PROCESSING_FAILURE, responseMessageID);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("MC_Send_Response_Message error", status);
                    }
                     
                    MC_Free_Message(&responseMessageID); /* Free the response object */ 
                    return MC_CANNOT_COMPLY;
                }
                break;
            }
        }
        if (i >= sizeof(data) / sizeof(DataTable))
        {
            /*
             * Unknown tag encountered, this should be a failure
             */
            MC_Empty_Message(responseMessageID);
            
            printf("\nERROR: Unknown tag encountered in attribute list: %lx\n",tag); 
                           
            status = MC_Send_Response_Message(A_associationID,
                                              N_GET_ATTRIBUTE_LIST_ERROR, responseMessageID);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("MC_Send_Response_Message error", status);
            }
             
            MC_Free_Message(&responseMessageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        } 
        status = MC_Get_Next_Value_To_ULongInt(A_messageID,MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,&tag);
    }
    
    if (status != MC_NORMAL_COMPLETION && status != MC_NO_MORE_VALUES)
    {
        /*
         * No value set to attribute identifier list, this is a failure.
         */
        PrintError("MC_Get_Value_To_ULongInt for attribute identifier list error", status);

        MC_Empty_Message(responseMessageID);
        
        status = MC_Send_Response_Message(A_associationID,
                                          N_GET_ATTRIBUTE_LIST_ERROR, responseMessageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", status);
        }

        MC_Free_Message(&responseMessageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    } 
    
    
    status = MC_Send_Response_Message(A_associationID,
                                      N_GET_SUCCESS, responseMessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&responseMessageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Free_Message(&responseMessageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error", status);
        return MC_CANNOT_COMPLY;
    }


    return MC_NORMAL_COMPLETION;
}



/****************************************************************************
 *
 *  Function    :   Print_Session
 *
 *  Parameters   :  A_fsp - A pointer to the film session record to be 
 *                          printed
 *
 *  Returns      :  SUCCESS
 *                  FAIL
 *
 *  Description :   This function would actually do the printing of a film
 *                  session.  It currently just displays the film session
 *                  information on the screen.
 *
 ****************************************************************************/
static short Print_Session(FILM_SESSION *A_fsp)
{
    printf("Basic Film Session    %s\n", A_fsp->UID);
    switch(A_fsp->priority)
    {
        case PRNT_HIGH_PRI:
            printf("\tHigh Priority Print\n");
            break;
        case PRNT_MED_PRI:
            printf("\tMedium Priority Print\n");
            break;
        case PRNT_LOW_PRI:
            printf("\tLow Priority Print\n");
            break;
    }
    printf("\tCopies             %d\n", A_fsp->copies);
    printf("\tMedium Type        %s\n", A_fsp->medium_type);
    printf("\tFilm Destination   %s\n", A_fsp->film_destination);
    printf("\tFilm Session Label %s\n", A_fsp->film_session_label);
    printf("\tMemory Allocation  %ld\n", A_fsp->memory_allocation);
    return (SUCCESS);
}


/****************************************************************************
 *
 *  Function    :   FilmSession_N_CREATE
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_CREATE_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 service name of the 
 *                                    request message received
 * 
 *  Returns    :    MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This processes a Film Session N-CREATE-RQ message
 *                  when it is received.
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_CREATE(
            int         A_associationID, 
            int         A_messageID, 
            char*       A_serviceName)
{
    FILM_SESSION* fsp;
    MC_STATUS     status;
    int           messageID;
    char          tmp_buf[68];
    
    fsp = (FILM_SESSION*)calloc(1, sizeof(FILM_SESSION));


    /* 
     * Check if the SCU has already created a UID for this film session.
     * If not, create our own UID.
     */
    if(MC_Get_Value_To_String(A_messageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID,
              sizeof(fsp->UID), fsp->UID) != MC_NORMAL_COMPLETION)
    {
        strcpy(fsp->UID, Create_Inst_UID());
    }

    if((status = MC_Get_Value_To_ShortInt(A_messageID, MC_ATT_NUMBER_OF_COPIES,
              &fsp->copies)) != MC_NORMAL_COMPLETION)
    {
        PrintError("defaulting copy value", status);
        fsp->copies = 1;
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_PRINT_PRIORITY,
              sizeof(tmp_buf), tmp_buf) != MC_NORMAL_COMPLETION)
    {
        printf("defaulting priority value\n");
        fsp->priority = PRNT_MED_PRI;
    }
    else
    {
        if(strcmp(tmp_buf, "HIGH") == 0)
        {
            fsp->priority = PRNT_HIGH_PRI;
        }
        else if(strcmp(tmp_buf, "LOW ") == 0)
        {
            fsp->priority = PRNT_LOW_PRI;
        }
        else
        {
            fsp->priority = PRNT_MED_PRI;
        }
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_MEDIUM_TYPE,
       sizeof(fsp->medium_type), fsp->medium_type) != MC_NORMAL_COMPLETION)
    {
        strcpy(fsp->medium_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_FILM_DESTINATION,
       sizeof(fsp->film_destination), fsp->film_destination) !=
       MC_NORMAL_COMPLETION)
    {
        strcpy(fsp->film_destination, "DEFAULT");
    }


    if( (status  = MC_Get_Value_To_String(A_messageID, MC_ATT_FILM_SESSION_LABEL,
       sizeof(fsp->film_session_label), fsp->film_session_label)) != MC_NORMAL_COMPLETION)
    {
        strcpy(fsp->film_session_label, "DEFAULT");
    }

    if(MC_Get_Value_To_LongInt(A_messageID, MC_ATT_MEMORY_ALLOCATION,
       &fsp->memory_allocation) != MC_NORMAL_COMPLETION)
    {
        fsp->memory_allocation = -1;
    }

    fsp->next = Ssession_head;
    Ssession_head = fsp;

    /* Acquire a response message object */
    status = MC_Open_Message (&messageID, A_serviceName, N_CREATE_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                      fsp->UID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String for accected sop instance error", status);
        MC_Free_Message(&messageID);  
        return MC_CANNOT_COMPLY;
    }

    status = MC_Send_Response_Message(A_associationID,
                                      N_CREATE_SUCCESS, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&messageID);  
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error",status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}


/****************************************************************************
 *
 *  Function    :   FilmSession_N_SET
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_SET_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                    for the request message received
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function process an N-SET-RQ message received for
 *                  at the film session level.
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_SET(
            int         A_associationID, 
            int         A_messageID,
            char*       A_serviceName)
{
   
    FILM_SESSION *fsp;
    MC_STATUS     status;
    char          tmp_buf[68];
    
    fsp = Ssession_head;

    if(!fsp)
    {
        /* reply with failure, no such SOP Instance */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_SET_RSP,
                                      N_SET_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;
    }

    if(MC_Get_Value_To_ShortInt(A_messageID, MC_ATT_NUMBER_OF_COPIES,
              &fsp->copies) == MC_NULL_VALUE)
    {
        fsp->copies = 1;
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_PRINT_PRIORITY,
              sizeof(tmp_buf), tmp_buf) == MC_NULL_VALUE)
    {
        fsp->priority = PRNT_MED_PRI;
    }
    else
    {
        if(strcmp(tmp_buf, "HIGH") == 0)
        {
            fsp->priority = PRNT_HIGH_PRI;
        }
        else if(strcmp(tmp_buf, "LOW") == 0)
        {
            fsp->priority = PRNT_LOW_PRI;
        }
        else
        {
            fsp->priority = PRNT_MED_PRI;
        }
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_MEDIUM_TYPE,
       sizeof(fsp->medium_type), fsp->medium_type) == MC_NULL_VALUE)
    {
        strcpy(fsp->medium_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_FILM_DESTINATION,
       sizeof(fsp->film_destination), fsp->film_destination) == MC_NULL_VALUE)
    {
        strcpy(fsp->film_destination, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_FILM_SESSION_LABEL,
       sizeof(fsp->film_session_label), fsp->film_session_label) == MC_NULL_VALUE)
    {
        strcpy(fsp->film_session_label, "DEFAULT");
    }

    if(MC_Get_Value_To_LongInt(A_messageID, MC_ATT_MEMORY_ALLOCATION,
       &fsp->memory_allocation) == MC_NULL_VALUE)
    {
        fsp->memory_allocation = -1;
    }

    status = Send_Empty_Response( A_associationID,
                                  A_serviceName,
                                  N_SET_RSP,
                                  N_SET_SUCCESS);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Send_Empty_Response error", status);
        return MC_CANNOT_COMPLY;
    }    

    return MC_NORMAL_COMPLETION;
}



/****************************************************************************
 *
 *  Function    :   FilmSession_N_ACTION
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_GET_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                   for the request message received
 *                  A_remoteAE      - The AE title of the remote machine
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function process an N-ACTION-RQ message on the 
 *                  film session level when it is received from an SCU
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_ACTION(
                int         A_associationID, 
                int         A_messageID, 
                char*       A_serviceName,
                char*       A_remoteAE )
{
    FILM_SESSION *fsp;
    MC_STATUS     status;
    int           messageID;
    int           itemID;
    int           requestmessageID;
    char          printJobUID[68];
    short         i;
    printjob_t*   printJob;
    time_t        timeReturn;
    struct tm*    timePtr;
    
    fsp = Ssession_head;

    if(!fsp)
    {
        /* 
         * reply with failure, no such SOP Instance 
         */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_ACTION_RSP,
                                      N_ACTION_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;
    }

    Print_Session(fsp);
    for(i = 0; fsp->film_boxes[i]; i++)
    {
        Print_Film(fsp->film_boxes[i]);
    }

    /* Create instance UID */
    strcpy(printJobUID, Create_Inst_UID());

    /* Acquire a response message object */
    status = MC_Open_Message (&messageID, A_serviceName, N_ACTION_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Open_Item(&itemID, "REF_PRINT_JOB");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Item error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Add_Standard_Attribute(messageID, MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Add_Standard_Attribute error", status);
        MC_Free_Message(&messageID);
        MC_Free_Item(&itemID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_Int(messageID, MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE, itemID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int error", status);
        MC_Free_Message(&messageID);
        MC_Free_Item(&itemID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, printJobUID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_CLASS_UID, "1.2.840.10008.5.1.1.14");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }
    

    status = MC_Send_Response_Message(A_associationID,
                                      N_ACTION_SUCCESS, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error:\n", status);
        return MC_CANNOT_COMPLY;
    }

    /*
     * Only do this if print job was negotiated!
     */
    if( S_printJob_Negotiated )
    {
        /* Add UID to List */
        printJob = CreatePrintJobRecord( printJobUID );
        if (!printJob)
        {
            printf("Unable to add UID to print job list\n");
        }
        else
        {
            strcpy( printJob->executionStatus, "DONE");
            strcpy( printJob->executionStatusInfo, "QUEUED"); 

            switch(fsp->priority)
            {
                case PRNT_HIGH_PRI:
                    strcpy( printJob->printPriority, "HIGH");
                    break;
                case PRNT_MED_PRI:
                    strcpy( printJob->printPriority, "MEDIUM");
                    break;
                case PRNT_LOW_PRI:
                    strcpy( printJob->printPriority, "LOW");
                    break;
            }

            timeReturn = time(NULL);
            timePtr = localtime(&timeReturn);
            sprintf(printJob->creationDate, "%04d%02d%02d",
                   (timePtr->tm_year + 1900),
                   (timePtr->tm_mon + 1),
                    timePtr->tm_mday);
            sprintf(printJob->creationTime, "%02d%02d%02d",
                    timePtr->tm_hour,
                    timePtr->tm_min,
                    timePtr->tm_sec);
            strcpy( printJob->printerName, "MERGE_PRINT_SCP");
            strcpy( printJob->originatorAE, A_remoteAE);     
        }
     

        /* Acquire a request message object */
        status = MC_Open_Message (&requestmessageID, "PRINT_JOB", N_EVENT_REPORT_RQ);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error", status);
            return MC_CANNOT_COMPLY;
        }

        /* Set UID in message*/
        status = MC_Set_Value_From_String( requestmessageID,
                                            MC_ATT_AFFECTED_SOP_INSTANCE_UID,  
                                            printJobUID);
        if( status != MC_NORMAL_COMPLETION )
        {
           PrintError("Unable to set MC_ATT_AFFECTED_SOP_INSTANCE_UID", status);
           MC_Free_Message(&requestmessageID);
           return  MC_CANNOT_COMPLY;
        }

        /* Set EVENT_TYPE to DONE */
        status = MC_Set_Value_From_Int( requestmessageID,
                                        MC_ATT_EVENT_TYPE_ID, 3);  
        if( status != MC_NORMAL_COMPLETION )
         {
           PrintError("Unable to set MC_ATT_EVENT_TYPE_ID",status);
           MC_Free_Message(&requestmessageID);
           return MC_CANNOT_COMPLY;
         }

        /* Send request message*/
        
        status = MC_Send_Request_Message(A_associationID, requestmessageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Request_Message error", status);
            MC_Free_Message(&requestmessageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        }
        
        status = MC_Free_Message(&requestmessageID); /* Free the response object */ 
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message error", status);
            return MC_CANNOT_COMPLY;
        }
    }
    return MC_NORMAL_COMPLETION;
}



/****************************************************************************
 *
 *  Function    :  FilmSession_N_DELETE
 *
 *  Parameters  :  A_associationID - The association over which the 
 *                                   the N_DELETE_RQ message has come over
 *                 A_messageID     - The ID of the request message
 *                                   received
 *                 A_serviceName   - The MergeCOM-3 assigned service name
 *                                   for the request message received
 *
 *  Returns     :  MC_CANNOT_COMPLY
 *                 MC_NORMAL_COMPLETION
 * 
 *  Description :  This function processes an N-DELETE-RQ message at the
 *                 film session level
 *
 ****************************************************************************/
static MC_STATUS FilmSession_N_DELETE(
            int         A_associationID, 
            int         A_messageID, 
            char*       A_serviceName)
{
    FILM_SESSION *fsp;
    MC_STATUS     status;
    int           i;
    
    fsp = Ssession_head;

    if(!fsp)
    {
        /* reply with failure, no such SOP Instance */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_DELETE_RSP,
                                      N_DELETE_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;
    }
    else
    {
        /* delete this instance and all objects it references */
        for(i = 0; fsp->film_boxes[i]; i++)
        {
            Delete_Film(fsp->film_boxes[i]);
        }
        Ssession_head = fsp->next;
        free(fsp);
    }

    status = Send_Empty_Response( A_associationID,
                                  A_serviceName,
                                  N_DELETE_RSP,
                                  N_DELETE_SUCCESS);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Send_Empty_Response error", status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}




/****************************************************************************
 *
 *  Function    :   Print_Film
 *
 *  Parameters  :   A_fbp - A pointer to a film box record 
 *
 *  Returns     :   SUCCESS
 * 
 *  Description :   This function would normally handle when a request to
 *                  print is made at the film box level.  In this case, it
 *                  only prints out the current print options.
 *
 ****************************************************************************/
static short Print_Film(FILM_BOX *A_fbp)
{
    int i;

    printf("\tBasic Film Box %s\n", A_fbp->UID);
    printf("\t\tImage Display Format         %s",
                                          A_fbp->image_display_format[0]);
    if(A_fbp->image_display_format[1][0])
        printf("\\%s", A_fbp->image_display_format[1]);
    printf("\n");
    printf("\t\tAnnotation Display Format ID %s\n",
                                          A_fbp->annotation_display_format_id);
    printf("\t\tFilm Orientation             %s\n", A_fbp->film_orientation);
    printf("\t\tFilm Size ID                 %s\n", A_fbp->film_size_id);
    printf("\t\tMagnification Type           %s\n", A_fbp->magnification_type);
    printf("\t\tSmoothing Type               %s\n", A_fbp->smoothing_type);
    printf("\t\tBorder Density               %s\n", A_fbp->border_density);
    printf("\t\tEmpty Image Density          %s\n", A_fbp->empty_image_density);
    printf("\t\tMin Density                  %d\n", A_fbp->min_density);
    printf("\t\tMax Density                  %d\n", A_fbp->max_density);
    printf("\t\tTrim                         %s\n", A_fbp->trim);
    printf("\t\tConfiguration Info           %s\n", A_fbp->configuration_info);

    for(i = 0; A_fbp->image_boxes[i]; i++)
    {
        printf("\t\tPrinting image #%d\n", i+1);
        Image_Print(A_fbp->image_boxes[i]);
    }
    return SUCCESS;
}


/****************************************************************************
 *
 *  Function    :   Delete_Film
 *
 *  Parameters   :  A_fbp - A pointer to the film box record to be deleted
 *                  
 *  Returns      :  SUCCESS
 *                  FAIL 
 *                  
 *  Description :   Deletes an film box record from the linked list 
 *                  containing all of the film boxes
 *
 ****************************************************************************/
static short Delete_Film(FILM_BOX* A_fbp)
{
    FILM_BOX   *fbp;
    IMAGE_BOX  *ibp;
    int         i;

    if (!A_fbp)
        return 0;
        
    if(Sfilm_head == A_fbp)
        fbp = NULL;
    else
        fbp = Sfilm_head;

    while((fbp) && (fbp->next != A_fbp))
        fbp = fbp->next;

    if(!fbp)
    {
        /* 
         * reply with failure, no such SOP Instance 
         */
        return FAIL;
    }
    else
    {
        /* 
         * delete this instance and all objects it references 
         */
        for(i = 0; (ibp = fbp->image_boxes[i]); i++)
        {
            Image_Delete(ibp);
        }
        if(fbp == NULL)
            Sfilm_head = A_fbp->next;
        else
            fbp->next = A_fbp->next;
        free(fbp);
    }
    return SUCCESS;
}



/****************************************************************************
 *
 *  Function    :   FilmBox_N_CREATE
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_CREATE_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                    for the request message received
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function handles when an N-CREATE-RQ message is
 *                  received at the film box level
 *
 ****************************************************************************/
static MC_STATUS FilmBox_N_CREATE(
            int         A_associationID, 
            int         A_messageID, 
            char*       A_serviceName)
{
    FILM_BOX     *fbp;
    MC_STATUS     status;
    int           messageID;
    int           itemID;
    char          tmp_buf[68];
    char          config_val[1024];
    char         *index;
    short         rows, columns, images;
    short         i;
   
    /*
     * Create a new record
     */ 
    fbp = (FILM_BOX*)calloc(1, sizeof(FILM_BOX));
    
    /*
     * Check if the SCU has filled in the SOP Instance UID for the 
     * new film box.  If not, we create one ourself.
     */
    if(MC_Get_Value_To_String(A_messageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID,
              sizeof(fbp->UID), fbp->UID) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->UID, Create_Inst_UID());
    }

    /* Associate the film box to the desired film session */

    if(MC_Get_Value_To_Int(A_messageID, MC_ATT_REFERENCED_FILM_SESSION_SEQUENCE,
                           &itemID) != MC_NORMAL_COMPLETION)
    {
        /* Acquire a response message object */
        status = MC_Open_Message (&messageID, A_serviceName, N_CREATE_RSP);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error", status);
            return MC_CANNOT_COMPLY;
        }

        status = MC_Set_Value_From_LongInt(messageID, 
                              MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,
                              MC_ATT_REFERENCED_FILM_SESSION_SEQUENCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_Int error", status);
            MC_Free_Message(&messageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        }
    
        status = MC_Send_Response_Message(A_associationID,
                                          N_CREATE_MISSING_ATTRIBUTE,
                                          messageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", status);
            MC_Free_Message(&messageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        }

        status = MC_Free_Message(&messageID); /* Free the response object */ 
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free-Message error", status);
            return MC_CANNOT_COMPLY;
        }

        return MC_NORMAL_COMPLETION;
    }

    if(MC_Get_Value_To_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID,
              sizeof(fbp->ref_film_session), fbp->ref_film_session) !=
       MC_NORMAL_COMPLETION)
    {
        /* Acquire a response message object */
        status = MC_Open_Message (&messageID, A_serviceName, N_CREATE_RSP);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error", status);
            return MC_CANNOT_COMPLY;
        }
    
        status = MC_Set_Value_From_Int(messageID, MC_ATT_ATTRIBUTE_IDENTIFIER_LIST,
                              MC_ATT_REFERENCED_SOP_INSTANCE_UID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_Int error", status);
            MC_Free_Message(&messageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        }
    
        status = MC_Send_Response_Message(A_associationID,
                                          N_CREATE_MISSING_ATTRIBUTE,
                                          messageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", status);
            MC_Free_Message(&messageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        }

        status = MC_Free_Message(&messageID); /* Free the response object */ 
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message error", status);
            return MC_CANNOT_COMPLY;
        }

        return MC_NORMAL_COMPLETION;
    }

    if(strcmp(Ssession_head->UID, fbp->ref_film_session) == 0)
    {
        Ssession_head->film_boxes[Ssession_head->films++] = fbp;
    }
    else
    {
        /* Acquire a response message object */
        status = MC_Open_Message (&messageID, A_serviceName, N_CREATE_RSP);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error", status);
            return MC_CANNOT_COMPLY;
        }
    
        status = MC_Set_Value_From_String(messageID, MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                 fbp->ref_film_session);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_String error", status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }
    
        status = MC_Send_Response_Message(A_associationID,
                                          N_CREATE_INVALID_ATTRIBUTE_VALUE,
                                          messageID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", status);
            MC_Free_Message(&messageID); /* Free the response object */ 
            return MC_CANNOT_COMPLY;
        }

        status = MC_Free_Message(&messageID); /* Free the response object */ 
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message error", status);
            return MC_CANNOT_COMPLY;
        }

        return MC_NORMAL_COMPLETION;
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_IMAGE_DISPLAY_FORMAT,
                              sizeof(tmp_buf), tmp_buf) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->image_display_format[0], "DEFAULT");
    }
    else
    {
        strcpy(fbp->image_display_format[0], strtok(tmp_buf, "\\"));
        index = strtok(NULL, "\\");
        if(index)
        {
            strcpy(fbp->image_display_format[1], index);
        }
    }

    if(strcmp(fbp->image_display_format[0], "STANDARD") == 0)
    {
        index = fbp->image_display_format[1];
        rows = (short)atoi(index);
        index = strchr(index, ',');
        columns = (short)atoi(index + 1);
        images = rows * columns;
    }
    else
    if(strcmp(fbp->image_display_format[0], "ROW") == 0)
    {
        images = rows = columns = 0;
        index = fbp->image_display_format[1];
        if(*index)
        {
            do
            {
                rows++;
                images += (short)atoi(index);
                index = strchr(index, ',');
                if (index && (*index == ',')) index++;
            }
            while(index && *index);
        }
    }
    else
    if(strcmp(fbp->image_display_format[0], "COL") == 0)
    {
        images = rows = columns = 0;
        index = fbp->image_display_format[1];
        if(*index)
        {
            do
            {
                columns++;
                images += (short)atoi(index);
                index = strchr(index, ',');
                if (index && (*index == ',')) index++;
            }
            while(index && *index);
        }
    }
    else
    if(strcmp(fbp->image_display_format[0], "SLIDE") == 0)
    {
        images = 35;
        rows = 0;
        columns = 0;
    }
    else
    if(strcmp(fbp->image_display_format[0], "SUPERSLIDE") == 0)
    {
        images = 40;
        rows = 0;
        columns = 0;
    }
    else
    {
        images = 0;
    }

    if(images == 0)
    {
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_CREATE_RSP,
                                      N_CREATE_INVALID_ATTRIBUTE_VALUE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;
    }

    for(i = 0; i < images; i++)
    {
        fbp->image_boxes[i] = Image_Create(fbp->UID, (short)(i+1));
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_ANNOTATION_DISPLAY_FORMAT_ID,
                              sizeof(fbp->annotation_display_format_id),
                              fbp->annotation_display_format_id) !=
       MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->annotation_display_format_id, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_FILM_ORIENTATION,
                              sizeof(fbp->film_orientation),
                              fbp->film_orientation) !=
       MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->film_orientation, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID,MC_ATT_FILM_SIZE_ID, sizeof(fbp->film_size_id),
                              fbp->film_size_id) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->film_size_id, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_MAGNIFICATION_TYPE,
                              sizeof(fbp->magnification_type),
                              fbp->magnification_type) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->magnification_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_SMOOTHING_TYPE,
                              sizeof(fbp->smoothing_type),
                              fbp->smoothing_type) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->smoothing_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_BORDER_DENSITY,
                              sizeof(fbp->border_density),
                              fbp->border_density) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->border_density, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_EMPTY_IMAGE_DENSITY,
                              sizeof(fbp->empty_image_density),
                              fbp->empty_image_density) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->empty_image_density, "DEFAULT");
    }

    if(MC_Get_Value_To_Int(A_messageID, MC_ATT_MIN_DENSITY,
                                &fbp->min_density) != MC_NORMAL_COMPLETION)
    {
        fbp->min_density = -1;
    }

    if(MC_Get_Value_To_Int(A_messageID, MC_ATT_MAX_DENSITY,
                                &fbp->max_density) != MC_NORMAL_COMPLETION)
    {
        fbp->max_density = -1;
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_TRIM,
                              sizeof(fbp->trim),
                              fbp->trim) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->trim, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_CONFIGURATION_INFORMATION,
                              sizeof(config_val),
                              config_val) != MC_NORMAL_COMPLETION)
    {
        strcpy(fbp->configuration_info,"DEFAULT");
    }
    else
    {
        strcpy(fbp->configuration_info,config_val);
    }

    /* Acquire a response message object */
    status = MC_Open_Message (&messageID, A_serviceName, N_CREATE_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Get_Value_To_String(A_messageID, MC_ATT_AFFECTED_SOP_CLASS_UID,
                           sizeof(tmp_buf), tmp_buf);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    for(i = 0; i < images; i++)
    {
        /* Acquire a sequence item object */
        status = MC_Open_Item (&itemID, "REF_IMAGE_BOX");
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Item error", status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }
        
        /* Add the sequence to the message */
        status = MC_Set_Next_Value_From_Int(messageID,
                                            MC_ATT_REFERENCED_IMAGE_BOX_SEQUENCE, 
                                            itemID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Next_Value_From_Int error", status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }

        status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_CLASS_UID,
                                          tmp_buf);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_String error", status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        }
        
        status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID,
                                          fbp->image_boxes[i]->UID);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Set_Value_From_String error", status);
            MC_Free_Message(&messageID);
            return MC_CANNOT_COMPLY;
        } 
    }

    status = MC_Set_Value_From_String(messageID, MC_ATT_AFFECTED_SOP_INSTANCE_UID,
                                      fbp->UID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    } 

    status = MC_Send_Response_Message(A_associationID,
                                      N_CREATE_SUCCESS, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&messageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    }

    status = MC_Free_Message(&messageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error", status);
        return MC_CANNOT_COMPLY;
    }


    fbp->next = Sfilm_head;
    Sfilm_head = fbp;
    return MC_NORMAL_COMPLETION;
}


/****************************************************************************
 *
 *  Function    :   FilmBox_N_SET
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_SET_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                    for the request message received
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function handles an N-SET-RQ received on the film
 *                  box level.
 *
 ****************************************************************************/
static MC_STATUS FilmBox_N_SET(
            int         A_associationID, 
            int         A_messageID, 
            char*       A_serviceName)
{
    FILM_BOX     *fbp;
    MC_STATUS     status;
    
    fbp = Sfilm_head;

    if(!fbp)
    {
        /* 
         * reply with failure, no such SOP Instance 
         */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_SET_RSP,
                                      N_SET_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;      
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_MAGNIFICATION_TYPE,
                              sizeof(fbp->magnification_type),
                              fbp->magnification_type) == MC_NULL_VALUE)
    {
        strcpy(fbp->magnification_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_SMOOTHING_TYPE,
                              sizeof(fbp->smoothing_type),
                              fbp->smoothing_type) == MC_NULL_VALUE)
    {
        strcpy(fbp->smoothing_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_BORDER_DENSITY,
                              sizeof(fbp->border_density),
                              fbp->border_density) == MC_NULL_VALUE)
    {
        strcpy(fbp->border_density, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_EMPTY_IMAGE_DENSITY,
                              sizeof(fbp->empty_image_density),
                              fbp->empty_image_density) == MC_NULL_VALUE)
    {
        strcpy(fbp->empty_image_density, "DEFAULT");
    }

    if(MC_Get_Value_To_Int(A_messageID, MC_ATT_MIN_DENSITY,
                                &fbp->min_density) == MC_NULL_VALUE)
    {
        fbp->min_density = -1;
    }

    if(MC_Get_Value_To_Int(A_messageID, MC_ATT_MAX_DENSITY,
                                &fbp->max_density) == MC_NULL_VALUE)
    {
        fbp->max_density = -1;
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_TRIM,
                              sizeof(fbp->trim),
                              fbp->trim) == MC_NULL_VALUE)
    {
        strcpy(fbp->trim, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_CONFIGURATION_INFORMATION,
                              sizeof(fbp->configuration_info),
                              fbp->configuration_info) == MC_NULL_VALUE)
    {
        strcpy(fbp->configuration_info, "DEFAULT");
    }
    status = Send_Empty_Response( A_associationID,
                                  A_serviceName,
                                  N_SET_RSP,
                                  N_SET_SUCCESS);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Send_Empty_Response error", status);
        return MC_CANNOT_COMPLY;
    }    

    return MC_NORMAL_COMPLETION;
}


/****************************************************************************
 *
 *  Function    :   FilmBox_N_ACTION
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_ACTION_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                    for the request message received
 *                  A_remoteAE      - The AE title of the remote machine
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function handles an N-ACTION-RQ message received on
 *                  the film box level
 *
 ****************************************************************************/
static MC_STATUS FilmBox_N_ACTION(
            int         A_associationID, 
            int         A_messageID, 
            char*       A_serviceName,
            char*       A_remoteAE )
{
    FILM_BOX     *fbp;
    FILM_SESSION *fsp;
    printjob_t*   printJob;
    MC_STATUS     status;
    int           messageID;
    int           itemID;
    int           requestmessageID;
    char          tmp_buf1[68];
    char          tmp_buf2[68];
    struct tm*    timePtr;
    time_t        timeReturn;
    
    if((status = MC_Get_Value_To_String(A_messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID,
              sizeof(tmp_buf1), tmp_buf1)) == MC_EMPTY_VALUE)
    {
        /* reply with failure, missing attribute value (0000,1000) */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_ACTION_RSP,
                                      N_ACTION_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;   
    }
    else if(status == MC_NULL_VALUE)
    {
        /* reply with failure, invalid Attribute value */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_ACTION_RSP,
                                      N_ACTION_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;
    }
    else
    {
        fbp = Sfilm_head;

        while(fbp)
        {
            if(strcmp(fbp->UID, tmp_buf1) == 0)
            {
                break;
            }
            fbp = fbp->next;
        }

        if(!fbp)
        {
            /* reply with failure, no such SOP Instance */
            status = Send_Empty_Response( A_associationID,
                                          A_serviceName,
                                          N_ACTION_RSP,
                                          N_ACTION_NO_SUCH_SOP_INSTANCE);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("Send_Empty_Response error", status);
                return MC_CANNOT_COMPLY;
            }    

            return MC_NORMAL_COMPLETION;
        }

        fsp = Ssession_head;

        while(fsp)
        {
            if(strcmp(fsp->UID, fbp->ref_film_session) == 0)
            {
                break;
            }
            fsp = fsp->next;
        }

        if(!fsp)
        {
            /* reply with failure, no such SOP Instance */
            status = Send_Empty_Response( A_associationID,
                                          A_serviceName,
                                          N_ACTION_RSP,
                                          N_ACTION_PROCESSING_FAILURE);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("Send_Empty_Response error", status);
                return MC_CANNOT_COMPLY;
            }    

            return MC_NORMAL_COMPLETION;
        }

        Print_Session(fsp);
        Print_Film(fbp);
    }

    /* Create instance UID for the print job*/
    strcpy(tmp_buf2, Create_Inst_UID());

    /* Acquire a response message object */
    status = MC_Open_Message (&messageID, A_serviceName, N_ACTION_RSP);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    /* The following code is necessary only if the PRINT_JOB SOP class 
     * has been negotiated.  
     */

    status = MC_Open_Item(&itemID, "REF_PRINT_JOB");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Item error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Add_Standard_Attribute(messageID, MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Add_Standard_Attribute error", status);
        MC_Free_Message(&messageID);
        MC_Free_Message(&itemID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_Int(messageID, MC_ATT_REFERENCED_PRINT_JOB_SEQUENCE, itemID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_Int error", status);
        MC_Free_Message(&messageID);
        MC_Free_Message(&itemID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(itemID, MC_ATT_REFERENCED_SOP_INSTANCE_UID, tmp_buf2);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Set_Value_From_String(itemID, 
                                      MC_ATT_REFERENCED_SOP_CLASS_UID, 
                                      "1.2.840.10008.5.1.1.14");
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Value_From_String error", status);
        MC_Free_Message(&messageID);
        return MC_CANNOT_COMPLY;
    }

    status = MC_Send_Response_Message(A_associationID,
                                      N_ACTION_SUCCESS, messageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&messageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    }


    status = MC_Free_Message(&messageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    /*
     * If no print job, we can just exit.
     */
    if (!S_printJob_Negotiated)
        return MC_NORMAL_COMPLETION;


    /* Add UID to List */
    printJob = CreatePrintJobRecord( tmp_buf2 );
    if (!printJob)
    {
        printf("Unable to add UID to print job list\n");
    }
    else
    {
        strcpy( printJob->executionStatus, "DONE");
        strcpy( printJob->executionStatusInfo, "QUEUED"); 

        switch(fsp->priority)
        {
            case PRNT_HIGH_PRI:
                strcpy( printJob->printPriority, "HIGH");
                break;
            case PRNT_MED_PRI:
                strcpy( printJob->printPriority, "MEDIUM");
                break;
            case PRNT_LOW_PRI:
                strcpy( printJob->printPriority, "LOW");
                break;
        }

        timeReturn = time(NULL);
        timePtr = localtime(&timeReturn);
        sprintf(printJob->creationDate, "%04d%02d%02d",
               (timePtr->tm_year + 1900),
               (timePtr->tm_mon + 1),
                timePtr->tm_mday);
        sprintf(printJob->creationTime, "%02d%02d%02d",
                timePtr->tm_hour,
                timePtr->tm_min,
                timePtr->tm_sec);
        strcpy( printJob->printerName, "MERGE_PRINT_SCP");
        strcpy( printJob->originatorAE, A_remoteAE);     
    }
    
    /* Acquire a request message object */
    status = MC_Open_Message (&requestmessageID, "PRINT_JOB", N_EVENT_REPORT_RQ);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    /* Set UID in message*/
    status = MC_Set_Value_From_String( requestmessageID,
                                        MC_ATT_AFFECTED_SOP_INSTANCE_UID,  
                                        tmp_buf2);
    if( status != MC_NORMAL_COMPLETION )
     {
       PrintError("Unable to set MC_ATT_AFFECTED_SOP_INSTANCE_UID",status);
       MC_Free_Message(&requestmessageID);
       return MC_CANNOT_COMPLY;
     }

    /* Set EVENT_TYPE to DONE */
    status = MC_Set_Value_From_Int( requestmessageID,
                                    MC_ATT_EVENT_TYPE_ID, 3);  
    if( status != MC_NORMAL_COMPLETION )
     {
       PrintError("Unable to set MC_ATT_EVENT_TYPE_ID",status);
       MC_Free_Message(&requestmessageID);
       return MC_CANNOT_COMPLY;
     }

    /* Send request message*/
    
    status = MC_Send_Request_Message(A_associationID, requestmessageID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Send_Response_Message error", status);
        MC_Free_Message(&requestmessageID); /* Free the response object */ 
        return MC_CANNOT_COMPLY;
    }
    
    status = MC_Free_Message(&requestmessageID); /* Free the response object */ 
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Free_Message error", status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}



        
/****************************************************************************
 *
 *  Function    :   FilmBox_N_DELETE
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_DELETE_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                    for the request message received
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function handles N-DELETE-RQ messages received at
 *                  the film box level.
 *
 ****************************************************************************/
static MC_STATUS FilmBox_N_DELETE(
            int         A_associationID, 
            int         A_messageID, 
            char*       A_serviceName)
{
    MC_STATUS     status;
    short         retStatus;
    RESP_STATUS   respStatus;
     
    /* 
     * Delete this instance and all objects it references 
     */
    retStatus = Delete_Film(Sfilm_head);

    if(retStatus != SUCCESS)
    {
        /* 
         * reply with failure, no such SOP Instance 
         */
        respStatus =  N_DELETE_NO_SUCH_SOP_INSTANCE;
    }
    else
        respStatus =  N_DELETE_SUCCESS;

    status = Send_Empty_Response( A_associationID,
                                  A_serviceName,
                                  N_DELETE_RSP,
                                  respStatus);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Send_Empty_Response error", status);
        return MC_CANNOT_COMPLY;
    }

    return MC_NORMAL_COMPLETION;
}




/****************************************************************************
 *
 *  Function    :   Image_Create
 *
 *  Parameters  :   A_film_UID - The UID for the film box that this image
 *                               box is contained within is passed in here
 *                  A_image_position - This field contains the image position
 *                                     of the newly created image box
 *
 *  Returns     :   A pointer to the new image box record
 * 
 *  Description :   This function creates an Image Box and returns the image
 *                  box record to the caller (should only be called from 
 *                  Film_Create).
 *
 ****************************************************************************/
static IMAGE_BOX *Image_Create(char* A_Film_UID, short A_image_position)
{
    IMAGE_BOX    *ibp;
    
    ibp = (IMAGE_BOX*)calloc(1, sizeof(IMAGE_BOX));
    strcpy(ibp->UID, Create_Inst_UID());

    /* Associate the image box to the desired film box */
    strcpy(ibp->film_box_uid, A_Film_UID);
    ibp->image_position = A_image_position;
    strcpy(ibp->polarity, "DEFAULT");
    strcpy(ibp->requested_image_size, "DEFAULT");
    strcpy(ibp->magnification_type, "DEFAULT");
    strcpy(ibp->smoothing_type, "DEFAULT");
    ibp->pixel_sequence_defined = PRNT_IMAGE_PIXEL_DATA_EMPTY;

    ibp->next = Simage_head;
    Simage_head = ibp;
    return(ibp);
}


/****************************************************************************
 *
 *  Function    :   Image_Delete
 *
 *  Parameters   :  A_ibp - A pointer to the image box record to be deleted
 *                  
 *  Returns      :  SUCCESS
 *                  FAIL
 *                  
 *  Description :   Deletes an image box record from the linked list 
 *                  containing all of the image boxes
 *
 ****************************************************************************/
static short Image_Delete(IMAGE_BOX *A_ibp)
{
    IMAGE_BOX    *ibp;

    if(Simage_head == A_ibp)
        ibp = NULL;
    else
        ibp = Simage_head;

    while((ibp) && (ibp->next != A_ibp))
        ibp = ibp->next;

    if(!ibp)
    {
        /* reply with failure, no such SOP Instance */
        return FAIL;
    }
    else
    {
        /* delete this instance */
        if(ibp == NULL)
            Simage_head = A_ibp->next;
        else
            ibp->next = A_ibp->next;
        free(ibp);
    }
    return SUCCESS;
}


/****************************************************************************
 *
 *  Function    :   Image_Print
 *
 *  Parameters  :   A_ibp - The image box record to print
 *
 *  Returns     :   SUCCESS
 * 
 *  Description :   This function would handle the actual printing of an
 *                  image box.  It currently only displays the options on
 *                  the image box level.
 *
 ****************************************************************************/
static short Image_Print(IMAGE_BOX *A_ibp)
{
    printf("\t\tGrayscale Image Box    %s\n", A_ibp->UID);
    printf("\t\t\tImage Position       %d\n", A_ibp->image_position);
    printf("\t\t\tPolarity             %s\n", A_ibp->polarity);
    printf("\t\t\tRequested Image Size %s\n", A_ibp->requested_image_size);
    printf("\t\t\tMagnification Type   %s\n", A_ibp->magnification_type);
    printf("\t\t\tSmoothing Type       %s\n", A_ibp->smoothing_type);
    if(A_ibp->pixel_sequence_defined == PRNT_IMAGE_PIXEL_DATA_EMPTY)
    {
       printf("\t\t\tImage is EMPTY\n");
    }
    else
    {
        printf("\t\t\tImage has pixel data described by the following\n");
        printf("\t\t\t\tSamples Per Pixel          %d\n",
                                                 A_ibp->samples_per_pixel);
        printf("\t\t\t\tPhotometric Interpretation %s\n",
                                                 A_ibp->photometric_interpretation);
        printf("\t\t\t\tRows                       %d\n",
                                                 A_ibp->rows);
        printf("\t\t\t\tColumns                    %d\n",
                                                 A_ibp->columns);
        printf("\t\t\t\tPixel Aspect Ratio         %s/%s\n",
                                                 A_ibp->pixel_aspect_ratio[0],
                                                 A_ibp->pixel_aspect_ratio[1]);
        printf("\t\t\t\tBits Allocated             %d\n",
                                                 A_ibp->bits_allocated);
        printf("\t\t\t\tBits Stored                %d\n",
                                                 A_ibp->bits_stored);
        printf("\t\t\t\tHigh Bit                   %d\n",
                                                 A_ibp->high_bit);
        printf("\t\t\t\tPixel Representation       %d\n",
                                                 A_ibp->pixel_representation);

        if(A_ibp->pixel_data == PRNT_IMAGE_PIXEL_DATA_PRESENT)
        {
            printf("\t\t\t\tPixel Data                 Defined\n");
        }
        else
        {
            printf("\t\t\t\tPixel Data                 Empty\n");
        }
    }
    return SUCCESS;
}


/****************************************************************************
 *
 *  Function    :   ImageBox_N_SET
 *
 *  Parameters  :   A_associationID - The association over which the 
 *                                    the N_SET_RQ message has come over
 *                  A_messageID     - The ID of the request message
 *                                    received
 *                  A_serviceName   - The MergeCOM-3 assigned service name
 *                                    for the request message received
 *
 *  Returns     :   MC_CANNOT_COMPLY
 *                  MC_NORMAL_COMPLETION
 * 
 *  Description :   This function handles N-SET-RQ message received at the
 *                  image box level. 
 *
 ****************************************************************************/
static MC_STATUS ImageBox_N_SET(int A_associationID, int A_messageID, char *A_serviceName)
{
    IMAGE_BOX    *ibp;
    MC_STATUS     status;
    int           itemID;
    char          tmp_buf[68];

    if((status = MC_Get_Value_To_String(A_messageID, MC_ATT_REQUESTED_SOP_INSTANCE_UID,
              sizeof(tmp_buf), tmp_buf)) == MC_EMPTY_VALUE)
    {
        /* reply with failure, missing attribute value (0000,1000) */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_SET_RSP,
                                      N_SET_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    

        return MC_NORMAL_COMPLETION;
    }
    else if(status == MC_NULL_VALUE)
    {
        /* reply with failure, invalid Attribute value */
        status = Send_Empty_Response( A_associationID,
                                      A_serviceName,
                                      N_SET_RSP,
                                      N_SET_NO_SUCH_SOP_INSTANCE);
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Send_Empty_Response error", status);
            return MC_CANNOT_COMPLY;
        }    
        return MC_NORMAL_COMPLETION;
    }
    else
    {
        ibp = Simage_head;

        while(ibp)
        {
            if(strcmp(ibp->UID, tmp_buf) == 0)
            {
                break;
            }
            ibp = ibp->next;
        }

        if(!ibp)
        {
            /* reply with failure, no such SOP Instance */
            status = Send_Empty_Response( A_associationID,
                                          A_serviceName,
                                          N_SET_RSP,
                                          N_SET_NO_SUCH_SOP_INSTANCE);
            if (status != MC_NORMAL_COMPLETION)
            {
                PrintError("Send_Empty_Response error", status);
                return MC_CANNOT_COMPLY;
            }    

            return MC_NORMAL_COMPLETION;
        }
    }

    if(MC_Get_Value_To_UShortInt(A_messageID, MC_ATT_IMAGE_DISPLAY_FORMAT,
                                 &ibp->image_position) == MC_NULL_VALUE)
    {
        /* default is to not change the position */
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_POLARITY,
                              sizeof(ibp->polarity),
                              ibp->polarity) == MC_NULL_VALUE)
    {
        strcpy(ibp->polarity, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_REQUESTED_IMAGE_SIZE,
                              sizeof(ibp->requested_image_size),
                              ibp->requested_image_size) == MC_NULL_VALUE)
    {
        strcpy(ibp->requested_image_size, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_MAGNIFICATION_TYPE,
                              sizeof(ibp->magnification_type),
                              ibp->magnification_type) == MC_NULL_VALUE)
    {
        strcpy(ibp->magnification_type, "DEFAULT");
    }

    if(MC_Get_Value_To_String(A_messageID, MC_ATT_SMOOTHING_TYPE,
                              sizeof(ibp->smoothing_type),
                              ibp->smoothing_type) == MC_NULL_VALUE)
    {
        strcpy(ibp->smoothing_type, "DEFAULT");
    }

    if(MC_Get_Value_To_Int(A_messageID,
                           MC_ATT_BASIC_GRAYSCALE_IMAGE_SEQUENCE,
                           &itemID) != MC_NORMAL_COMPLETION)
    {
        ibp->pixel_sequence_defined = PRNT_IMAGE_PIXEL_DATA_EMPTY;
    }
    else
    {
        ibp->pixel_sequence_defined = PRNT_IMAGE_PIXEL_DATA_PRESENT;
        if(MC_Get_Value_To_Int(itemID, MC_ATT_SAMPLES_PER_PIXEL,
                                     &ibp->samples_per_pixel) == MC_NULL_VALUE)
        {
            ibp->samples_per_pixel = -1;
        }

        if(MC_Get_Value_To_String(itemID, MC_ATT_PHOTOMETRIC_INTERPRETATION,
                                  sizeof(ibp->photometric_interpretation),
                                  ibp->photometric_interpretation) ==
           MC_NULL_VALUE)
        {
            strcpy(ibp->photometric_interpretation, "DEFAULT");
        }

        if(MC_Get_Value_To_Int(itemID, MC_ATT_ROWS,
                                    &ibp->rows) == MC_NULL_VALUE)
        {
            ibp->rows = -1;
        }

        if(MC_Get_Value_To_Int(itemID, MC_ATT_COLUMNS,
                                    &ibp->columns) == MC_NULL_VALUE)
        {
            ibp->columns = -1;
        }
        
        status = MC_Get_Value_To_String(itemID, MC_ATT_PIXEL_ASPECT_RATIO,
                                   sizeof(ibp->pixel_aspect_ratio[1]),
                                   ibp->pixel_aspect_ratio[1]);
        if ((status == MC_INVALID_TAG) 
         || (status == MC_NULL_VALUE)
         || (status == MC_EMPTY_VALUE))
        {
            strcpy(ibp->pixel_aspect_ratio[0], "1.00");
            strcpy(ibp->pixel_aspect_ratio[1], "1.00");
         
        }
        else if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Get_Value_To_String error", status);
            return MC_CANNOT_COMPLY;
        }

        if(MC_Get_Value_To_UShortInt(itemID, MC_ATT_BITS_ALLOCATED,
                                    &ibp->bits_allocated) == MC_NULL_VALUE)
        {
            ibp->bits_allocated = 8;
        }

        if(MC_Get_Value_To_UShortInt(itemID, MC_ATT_BITS_STORED,
                                    &ibp->bits_stored) == MC_NULL_VALUE)
        {
            ibp->bits_stored = 8;
        }

        if(MC_Get_Value_To_UShortInt(itemID, MC_ATT_HIGH_BIT,
                                    &ibp->high_bit) == MC_NULL_VALUE)
        {
            ibp->high_bit = 7;
        }

        if(MC_Get_Value_To_Int(itemID, MC_ATT_PIXEL_REPRESENTATION,
                                    &ibp->pixel_representation) == MC_NULL_VALUE)
        {
            ibp->pixel_representation = -1;
        }

        if(MC_Get_Value_To_UShortInt(itemID, MC_ATT_PIXEL_DATA,
                                    &ibp->pixel_data) != MC_NORMAL_COMPLETION)
        {
            ibp->pixel_data = PRNT_IMAGE_PIXEL_DATA_PRESENT;
        }
        else
        {
            ibp->pixel_data = PRNT_IMAGE_PIXEL_DATA_EMPTY;
        }
    }

    status = Send_Empty_Response( A_associationID,
                                  A_serviceName,
                                  N_SET_RSP,
                                  N_SET_SUCCESS);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Send_Empty_Response error", status);
        return MC_CANNOT_COMPLY;
    }    

    return MC_NORMAL_COMPLETION;
}

/****************************************************************************
 *
 *  Function    :   Handle_Association
 *
 *  Parameters  :   A_associationID - The association ID of the new 
 *                                    association
 *
 *  Returns     :   none
 * 
 *  Description :   This function handles associations when they come in. 
 *                  Note that under UNIX systems, this is handles in a new
 *                  child process, but under single tasking systems, only
 *                  one association can be received at once.
 *
 ****************************************************************************/
static void Handle_Association(int* A_associationID)
{
    time_t      start_time, image_start_time;
    CBinfo      callbackInfo;    
    MC_STATUS   status;
    MC_COMMAND  command;
    int         messageID;
    char*       serviceName;
    short       serviceID;
    ServiceInfo serviceInfo;
    AssocInfo   assocInfo;

    /*
     *  Need process ID number for messages
     */
#ifdef UNIX     
    sprintf(Sprefix, "PID %d", getpid() );
#endif
    
    strcpy(callbackInfo.prefix, Sprefix);
        
    /*
     * Parse the list of service classes negotiated to determine
     * if the print job SOP class was accepted.
     */ 
    S_printJob_Negotiated = FALSE;
    status = MC_Get_First_Acceptable_Service(SassociationID, &serviceInfo); 
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
            status = MC_Get_Next_Acceptable_Service(SassociationID, &serviceInfo);
         }
    }
    else
    {
        PrintError("MC_Get_First_Acceptable_Service failed",status);
        MC_Abort_Association(&SassociationID);
        return; 
    }

    if( S_printJob_Negotiated )
        printf( "Print Job Service Class was negotiated\n");
    else
        printf( "Print Job Service Class was NOT negotiated\n");  

    /*
     * Get the association info.  This is needed because the print
     * job service class requires us to manage the AE title from
     * which we received a print job.  This is contained in the
     * AssocInfo structure.
     */
    status = MC_Get_Association_Info(SassociationID, &assocInfo); 
    if (status!= MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Association_Info failed",status);
        MC_Abort_Association(&SassociationID);
        return; 
    }
    
    printf("%s\n\tAssociation Received.\n", Sprefix);
    time (&start_time);

    status = MC_Accept_Association(SassociationID);
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintError("Error on MC_Accept_Association", status);
        return;
    }
    
    for (;;)
    {
        time (&image_start_time);
        status = MC_Read_Message(SassociationID, 30, &messageID,
                     &serviceName, &command);
        if (status != MC_NORMAL_COMPLETION)
        {
            MC_Free_Message(&messageID);
            if (status == MC_ASSOCIATION_CLOSED)
            {       
                printf("%s\tAssociation Closed.\n", Sprefix);
                printf ("%s\t\tElapsed time = %ld seconds\n",
                           Sprefix, (long)(time(NULL) - start_time));
                break;
            }
                
            if (status == MC_NETWORK_SHUT_DOWN
            ||  status == MC_ASSOCIATION_ABORTED
            ||  status == MC_INVALID_MESSAGE_RECEIVED
            ||  status == MC_CONFIG_INFO_ERROR)
            {
                PrintError("Unexpected event", status);
                break;
            }
            
            PrintError("Error on MC_Read_Message", status);
            MC_Abort_Association(&SassociationID);
            break;
        }

        serviceID = Parse_Service(serviceName);

        switch(serviceID)
        {
            case PRNT_PRINTER:
                if ( command == N_GET_RQ )
                {
                    status = Printer_N_GET(SassociationID,messageID);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on Printer_N_GET", status);
                    }
                }
                else
                { 
                    printf("unexpected command received for %s service\n",
                            serviceName);
                    MC_Abort_Association(&SassociationID);
                    return;
                }
                break;
            case PRNT_PRINT_JOB:
                if ( command == N_GET_RQ )
                {
                    status = PrintJob_N_GET(SassociationID,messageID);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on PrintJob_N_GET", status);
                    }
                }
                else if ( command == N_EVENT_REPORT_RSP )
                {
                    printf("N_EVENT_REPORT_RSP received\n"); 
                }
                else 
                {
                    printf("unexpected command received for %s service\n",
                           serviceName);
                    MC_Abort_Association(&SassociationID);
                    return;
                }
                break;

            case PRNT_BASIC_FILM_SESSION:
                if ( command == N_CREATE_RQ )
                {
                    status = FilmSession_N_CREATE(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmSession_N_CREATE", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else if ( command == N_SET_RQ )
                {
                    status = FilmSession_N_SET(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmSession_N_SET", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else if ( command == N_DELETE_RQ )
                {
                    status = FilmSession_N_DELETE(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmSession_N_DELETE", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else if ( command == N_ACTION_RQ )
                {
                    status = FilmSession_N_ACTION(SassociationID, messageID, serviceName, assocInfo.RemoteApplicationTitle);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmSession_N_ACTION", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else 
                {
                    printf("unexpected command received for %s service\n",
                          serviceName);
                    MC_Abort_Association(&SassociationID);
                    return;
                }
                break;
            case PRNT_BASIC_FILM_BOX:
                if ( command == N_CREATE_RQ )
                {
                    status = FilmBox_N_CREATE(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmBox_N_CREATE", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else if ( command == N_SET_RQ )
                {
                    status = FilmBox_N_SET(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmBox_N_SET", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else if ( command == N_DELETE_RQ )
                {
                    status = FilmBox_N_DELETE(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmBox_N_DELETE", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else if ( command == N_ACTION_RQ )
                {
                    status = FilmBox_N_ACTION(SassociationID, messageID, serviceName, assocInfo.RemoteApplicationTitle);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on FilmBox_N_SET", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else
                {
                    printf("unexpected command received for %s service\n",
                           serviceName);
                    MC_Abort_Association(&SassociationID);
                    return;
                }
                break;
            case PRNT_BASIC_GRAYSCALE_IMAGE_BOX:
                if ( command == N_SET_RQ ) 
                {
                    status = ImageBox_N_SET(SassociationID, messageID, serviceName);
                    if (status != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on ImageBox_N_SET", status);
                        MC_Abort_Association(&SassociationID);
                        return;
                    }
                }
                else
                { 
                    printf("unexpected command received for %s service\n",
                          serviceName);
                    MC_Abort_Association(&SassociationID);
                    return;
                }
                break;
            default:
                printf("unexpected message received |%s|\n", serviceName);
                MC_Abort_Association(&SassociationID);
                return;
        }
        
        status = MC_Free_Message(&messageID);    /* Free the received message object */
        if (status != MC_NORMAL_COMPLETION)
        {
            PrintError("Error on MC_Free_Message", status);
            MC_Abort_Association(&SassociationID);
            return;
        }
    }
    return;        
}

 

/**************************************************************************
 *
 *  Function     :  CreatePrintJobRecord
 * 
 *  Parameters   :  A_jobUID, the UID of the print job to create a record for
 *                  
 *  Returns      :  Pointer to the print job structure created
 *                  NULL on failure
 * 
 *  Description  :  This function creates a structure and places it in the 
 *                  list of print jobs so that they can be referenced later.
 *
 ****************************************************************************/
static printjob_t* CreatePrintJobRecord(char* A_jobUID)
{
    printjob_t*  ptr;

    ptr = (printjob_t*)malloc(sizeof(printjob_t));
    
    if (!ptr)
    {
        printf("Unable to malloc memory\n");
        return NULL;
    }


    /*
     * Initialize the new print job record
     */       
    strcpy(ptr->jobUID,A_jobUID);     
    ptr->executionStatusInfo[0] = '\0'; 
    ptr->executionStatus[0] = '\0'; 
    ptr->printPriority[0] = '\0';
    ptr->creationDate[0] = '\0';
    ptr->creationTime[0] = '\0';
    ptr->printerName[0] = '\0';
    ptr->originatorAE[0] = '\0';    
    ptr->next = NULL;

    /*
     * Check if no records in the linked list.  Ie, this is the
     * first time the function is called.
     */       
    if (!S_pj.head)     
    { 
        S_pj.head = ptr;
        S_pj.tail = ptr;
        return ptr;
    }

   
    /*
     * Do a sanity check to be sure the tail node is initialized
     * correctly
     */ 
    if (S_pj.tail->next)
    {
        printf("The applications internal data structures contained an error\n");
        free(ptr);
        return NULL;
    }

    /*
     * Place the new record at the end of the list.
     */    
    S_pj.tail->next = ptr;
    S_pj.tail = ptr;
    
    ++S_pj.jobCount;
    return ptr;
}

#ifndef VXWORKS
/* Local Function Prototype */
static int DeletePrintJobRecord(
                char*            A_jobUID);

/**************************************************************************
 *
 *  Function     :  DeletePrintJobRecord
 * 
 *  Parameters   :  A_jobUID, the ID of the thread to delete the record
 *                  
 *  Returns      :  none
 * 
 *  Description  :  This function removes a thread's record from our records.
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
             * Check if first print job record is the record to delete
             */ 
            if (S_pj.head == ptr)
            {
                S_pj.head = S_pj.head->next;
                if (!S_pj.head->next)
                    S_pj.tail = NULL;
            }
            else
            {
                if (S_pj.tail == ptr)
                    S_pj.tail = prevPtr;
                prevPtr->next = ptr->next;
            }
            
            free(ptr);
            --S_pj.jobCount;
            return SUCCESS;
        }
        prevPtr = ptr;
        ptr = ptr->next;
    }
    return 1;
}
#endif /* ifndef VXWORKS */


/**************************************************************************
 *
 *  Function     :  GetPrintJobRecord
 * 
 *  Parameters   :  A_jobUID, the ID of the print job to get the record
 *                  
 *  Returns      :  none
 * 
 *  Description  :  This function retreieves a print jobs's record from 
 *                  our records.
 *
 ****************************************************************************/
static printjob_t* GetPrintJobRecord(char* A_jobUID)
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
            return ptr;
        }
        prevPtr = ptr;
        ptr = ptr->next;
    }
    return NULL;
}



/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Parameters   :  A_jobUID, the ID of the thread to delete the record
 *                  
 *  Returns      :  none
 * 
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
static void PrintError(char* A_string, MC_STATUS A_status)
{
    printf("%s\t%s:\n",Sprefix,A_string);
    printf("%s\t\t%s\n", Sprefix,MC_Error_Message(A_status));
}



#ifdef UNIX
/****************************************************************************
 *
 *  Function    :   shandler (UNIX)
 *
 *  Parameters   :  Asigno  -
 *                  Acode   -                
 *                  Ascp    -                
 
 *  Returns      :  int
 * 
 *  Description :   The shandler function is used to handle SIGPIPE
 *                  interrupts which occur when a process continues to
 *                  send data on a connection which has closed.
 *
 ****************************************************************************/
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static int shandler( int Asigno )
#else
static int shandler   (int                Asigno,
                       long               Acode,
                       struct sigcontext *Ascp)
#endif
{
    signal(SIGPIPE, (void (*)())shandler);
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
#else
    return -1;
#endif
}


/****************************************************************************
 *
 *  Function    :   sigint_routine (UNIX)
 *
 *  Parameters  :   Asigno  -
 *                  Acode   -                
 *                  Ascp    -                
 *
 *  Returns     :   none
 *
 *  Description :   This is our SIGINT routine (The user wants to shut down)
 *
 ****************************************************************************/
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void sigint_routine( int Asigno )
#else
static void sigint_routine  (int                Asigno,
                             long               Acode,
                             struct sigcontext *Ascp)
#endif
{
    pid_t        pid;
    int        child;
    int        Return = 0;
    
    for (child = 0; child < server_quota; child++)
    {
        pid = Schild_PID[child];
        if (pid)
        {
            /*
               Kill any lingering child processes.
            */
            (void)kill(pid, SIGKILL);
            Return = 1;
            fprintf(stderr, "KILLING CHILD PID (%d)\n", pid);
        }
    }    
    exit(Return);
}


/****************************************************************************
 *
 *  Function    :   reaper (UNIX)
 *
 *  Parameters   :  Asigno  -
 *                  Acode   -                
 *                  Ascp    - 
 *               
 *  Returns      :  none
 * 
 *  Description :   This is our SIGCHLD routine. (One or more of our children
 *                  has changed state)
 *
 ****************************************************************************/
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void reaper( int Asigno )
#else
static void reaper    (int                Asigno,
                       long               Acode,
                       struct sigcontext *Ascp)
#endif
{
    pid_t    pid;
    int    child;
    int    Return;
    
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        for (child = 0; child < server_quota; child++)
        {
            if (pid == Schild_PID[child])
            {
                /*
                   This does NOT kill the process; it only checks if
                   the process exists.
                */
                Return = kill(pid, 0);
                if (Return < 0 && errno == ESRCH)
                    Schild_PID[child] = 0;
                break;
            }
        }
        
        if (child >= server_quota)
            fprintf(stderr, "WAIT3 RETURNED PID=%d (NOT OUR CHILD)\n", pid);
    }
}
#endif  /* UNIX */            




