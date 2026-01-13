/*************************************************************************
 *
 *       System: MergeCOM-3 Advanced Media Extensions Sample Application
 *
 *    $Workfile: med_fsu.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *  Description: This is a sample media application using a DICOMDIR. 
 *               It behaves as a storage service class SCP, and places
 *               information about received images into a DICOMDIR.
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
    !defined(VXWORKS)        && !defined(MACH_NEXT) && !defined(_MACINTOSH)
#define UNIX            1
#endif

/*
 * Standard 'C' includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Platform specific includes
 */
#if defined(_MACINTOSH)
#include <Types.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <time.h>
#include <console.h>
#include <SIOUX.h>
#endif


#if defined(_MSDOS)  || defined(_WIN32) || defined(INTEL_WCC) || defined(__OS2__)
#if defined(_PHAR_LAP)
int _getch( void );
int _kbhit( void );
#else
#include <conio.h>
#endif
#include <fcntl.h>
#include <io.h>
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
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif


#include "mergecom.h"
#include "diction.h"
#include "mc3media.h"
#include "mc3msg.h"


/*
 * Constant definitions for opening files
 */
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


#if defined(_MSDOS) || defined(_WIN32) || defined(INTEL_WCC) || defined(__OS2__)
#define CURRENT_DIRECTORY ".\\"
#define DIRECTORY_SLASH_CHAR '\\'
#define DIRECTORY_SLASH_STRING "\\"
#elif defined(_RMX3)
#define CURRENT_DIRECTORY ""
#define DIRECTORY_SLASH_CHAR '/'
#define DIRECTORY_SLASH_STRING "/"
#elif defined(_MACINTOSH)
#define CURRENT_DIRECTORY ""
#define DIRECTORY_SLASH_CHAR ':'
#define DIRECTORY_SLASH_STRING ":"
#else
#define CURRENT_DIRECTORY "./"
#define DIRECTORY_SLASH_CHAR '/'
#define DIRECTORY_SLASH_STRING "/"
#endif

#define FSUAETITLE "MERGE_MEDIA"     /* AE title when writting to DICOM media */
#define SCPAETITLE "MERGE_MEDIA_FSU" /* AE title when associating for storage */


/*
 *  Structure passed to our callback functions for reading in data.
 */
typedef struct CALLBACKINFO
{        
   FILE*  stream;
   int    messageID;
   unsigned char buffer[8*1024];
} CBinfo;


/*
 *  Enum used when filling in attributes within a directory record.  The
 *  enum specifies value types of the attributes being set in the DICOMDIR
 *  to know if a failure has occured.
 */
typedef enum
{
    Type_1 = 1,
    Type_2,
    Type_3
} DICOM_TYPE;


/*
 *  Local definitions
 */
#define FAIL 0
#define SUCCESS 1
#define PATH_LENGTH 100  /* Maximum length of pathname to place DICOMDIR into */
#define UI_LENGTH   64   /* Maximum length of UI VR DICOM attributes + padding chars */


/*
 *  Module variables
 */
typedef struct dicomdirinfo
{
    int   dirID;                    /* ID of the DICOMDIR object */
    int   entityID;                 /* ID of the current directory entity */
    int   recID;                    /* ID of the current directory record */
    int   nextRecID;                /* ID of the next directory record */
    int   lowerRecID;    /* ID of the first record in the lower level entity */
    char* type;          /* pointer to a  string with the current record type */
    int   direxists;     /* TRUE if the dicomdir already exists on media */
    int   memory;                   /* TRUE if dicomdir saved in memory */
    char  pathname[PATH_LENGTH];    /* path the dicomdir is to be located */
    char  dirname[PATH_LENGTH+10];  /* length of DICOMDIR name + slash */
    int   fsuappID;                 /* ID of the application */
    int   scpappID;                 /* ID of the application */
    int   count;                    /* Number of elements in a DICOMDIR */
    int   entityCount;              /* Number of elements in an entity */
} DicomDirInfo;


#ifdef UNIX
typedef struct procinfo
{
    pid_t  child_save_PID;          /* PID of the child process */
    pid_t  child_PID;               /* PID of the child process */
    int    terminate;               /* bool for when program is terminating */
    int*   fsuappID;                /* ID of the application */
    int*   scpappID;                /* ID of the application */
} ProcInfo;
#endif

/*
 *  Module function prototypes
 */
static void      initialize                 ( DicomDirInfo*     A_DDInfo );
static void      print_cmdline              ( void );
static MC_STATUS test_cmdline               ( int               A_argc,
                                              char*             A_argv[],
                                              DicomDirInfo*     A_DDInfo ); 
static void      PrintError                 ( char*             A_string, 
                                              MC_STATUS         A_mcStatus );
static void      GotoRootEntity             ( DicomDirInfo*     A_DDInfo,
                                              int               *isLast );
static void      DisplayCurrentRecord       ( int               A_recID );
static void      DisplayCurrentEntity       ( DicomDirInfo*     A_DDInfo );
static MC_STATUS SetValue                   ( int               A_msgID, 
                                              int               A_recID, 
                                              unsigned long     A_tag,
                                              DICOM_TYPE        A_type );
static MC_STATUS FillPatientRecord          ( int               A_msgID, 
                                              int               A_recID );
static MC_STATUS FillStudyRecord            ( int               A_msgID,
                                              int               A_recID );
static MC_STATUS FillSeriesRecord           ( int               A_msgID, 
                                              int               A_recID );
static MC_STATUS FillImageRecord            ( int               A_msgID, 
                                              int               A_recID,
                                              char*             A_fileName );
static MC_STATUS AddMessageToDICOMDIR       ( int               A_msgID, 
                                              char*             A_serviceName,
                                              char*             A_fileName,
                                              DicomDirInfo*     A_DDInfo );
static int       FileExists                 ( char*             A_pathname );
static MC_STATUS AddImageGroupTwoElements   ( int               A_fileID );
static MC_STATUS AddDICOMDIRGroupTwoElements( DicomDirInfo*     A_DDInfo );
static MC_STATUS SetFileNameAttribute       ( int               A_msgID, 
                                              unsigned long     A_tag, 
                                              char*             A_fileName );
static MC_STATUS GetFileNameAttribute       ( int               A_msgID, 
                                              unsigned long     A_tag, 
                                              int               A_strLen,
                                              char*             A_fileName );
static MC_STATUS Handle_Association         ( int*              A_associationID,
                                              DicomDirInfo*     A_DDInfo );
static void      Wait_For_Association       ( DicomDirInfo*     A_DDInfo );
static MC_STATUS FileToMedia                ( char*             A_filename, 
                                              void*             A_userInfo,
                                              int               A_dataSize,
                                              void*             A_dataBuffer,
                                              int               A_isFirst, 
                                              int               A_isLast );
static MC_STATUS MediaToFile                ( char*             A_filename,
                                              void*             A_userInfo,
                                              int*              A_dataSize,
                                              void**            A_dataBuffer,
                                              int               A_isFirst,
                                              int*              A_isLast );
static MC_STATUS FileMetaInfoVersionCallback( int               A_msgID, 
                                              unsigned long     A_tag, 
                                              int               A_first,
                                              void*             A_info,
                                              int*              A_dataSize,
                                              void**            A_dataBufferPtr,
                                              int*              A_isLastPtr ); 
static char*     Create_Inst_UID            ( void );

#ifdef UNIX

static void      Start_Association_Process  ( DicomDirInfo*      A_DDInfo );
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void shandler( int A_signo );
#else
static int       shandler                   ( int                A_signo,
                                              long               A_code,
                                              struct sigcontext* A_scp );
#endif

#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void sigint_routine( int A_signo );
#else
static void      sigint_routine             ( int                A_signo,
                                              long               A_code,
                                              struct sigcontext* A_scp );
#endif

#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void reaper( int A_signo );
#else
static void      reaper                     ( int                A_signo,
                                              long               A_code,
                                              struct sigcontext* A_scp );
#endif


/*
 *  Static structure for the process info for signal routines.
 */
static ProcInfo S_PInfo;

/* 
 *  Static pointer to the dicomdir structure for the signal routines.
 */
static DicomDirInfo *S_DDInfo;
#endif



/****************************************************************************
 *
 *  Function    :   main
 *
 *  Description :   Loops waiting for associations or quit key press
 *
 ****************************************************************************/
#ifdef VXWORKS
int  medfsu(int argc, char** argv);
int medfsu(int argc, char** argv)
#else
int  main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    MC_STATUS     mcStatus;         /* return value from MergeCOM-3 calls */
    CBinfo        callbackInfo;   /* Calback structure */
    int           done = 0;       /* TRUE when exit from program is requested */
    char          user_input[128];/* user input string */
    char          argument[128];  /* user input string */
    int           isLast;         /* variable used for MC_Dir...  calls */
    int           saveEntityID;   /* saved ID of entity for .. */
    int           belowCount;     /* returned count for the number of nodes */
    DicomDirInfo  DDInfo;
    char          errorString[128];
    char          entered[256];

#ifdef UNIX
    int           Return;

    S_DDInfo = &DDInfo;
    S_PInfo.terminate = 0;
    S_PInfo.fsuappID = &DDInfo.fsuappID; 
    S_PInfo.scpappID = &DDInfo.scpappID; 
#endif

#if defined(_MACINTOSH)
    EventRecord    asEvent;
    Boolean        aqQuit = false;

    SIOUXSettings.initializeTB = true;
    SIOUXSettings.standalone = true;
    SIOUXSettings.setupmenus = true;
    SIOUXSettings.autocloseonquit = false;
    SIOUXSettings.asktosaveonclose = true;
    SIOUXSettings.showstatusline = true;
    argc = ccommand(&argv);
#endif 
 
    DDInfo.direxists = DDInfo.memory = 0;
    DDInfo.nextRecID = 0;
    DDInfo.lowerRecID = 0;
    printf("%s:  Media Sample FSU Application\n\n", argv[0]);


    /*
     * Formulate dicomdir pathname
     */ 
    if (test_cmdline(argc, argv, &DDInfo) != MC_NORMAL_COMPLETION)
        exit(EXIT_FAILURE);
    initialize(&DDInfo);


   /*
    * Loop for handling menu selections
    */
   while ( done != 1 )
   {
#if defined(_MACINTOSH)
        if (WaitNextEvent(mDownMask | mUpMask | keyDownMask |
            keyUpMask | autoKeyMask, &asEvent, 10, NULL)) 
        {
            if (asEvent.what == keyDown) 
            {
                unsigned char    abChar = asEvent.message & charCodeMask;
                Boolean          aqCmnd = ((asEvent.modifiers & cmdKey) != 0);

                if ((abChar == 'Q') || (abChar == 'q') ||
                    (aqCmnd && (abChar == '.')))
                    aqQuit = true;
            } 
            else if (asEvent.what == mouseDown) 
            {
                WindowPtr apWindow;
                short     awWhere;

                awWhere = FindWindow(asEvent.where, &apWindow);
                if (awWhere == inMenuBar) 
                {
                    MenuSelect(asEvent.where);
                    HiliteMenu(0);
                } 
                else if (awWhere == inSysWindow)
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
      user_input[0] = 0;
      printf ( "\n" );
      printf ( "-------------------------------| Main ");
      printf ( "|-----------------------------------\n");
      printf ( "\n" );
      printf ( "[1] Reload DICOMDIR                   ");
      printf ( "[2] Display current entity          \n");
      printf ( "[3] List DICOMDIR with MC_List_File() ");
      printf ( "[4] List record with MC_List_Item() \n");
      printf ( "[5] Go to root entity                 ");
      printf ( "[6] Go to lower level entity        \n");
      printf ( "[7] Go to first record                ");
      printf ( "[8] Go to next record               \n");
#ifndef UNIX
      printf ( "[9] Wait for an association         \n");
#endif
      printf ( "[C] Perform a count from the current record\n" );
      printf ( "[D] Delete the current referenced entity/record\n" );
      printf ( "[X] Exit\n" );
      printf ( "\n" );
      printf ( "     Current record: ");
      DisplayCurrentRecord(DDInfo.recID);
      if (DDInfo.nextRecID)
          printf("        Record Info: next record exists,");
      else
          printf("        Record Info: no next record,"); 
      if (DDInfo.lowerRecID)
          printf(" lower record exists\n");
      else
          printf(" no lower record\n");

      printf("     DICOMDIR count: ");
      mcStatus = MC_Dir_Root_Count( DDInfo.dirID, &DDInfo.count );
      mcStatus == MC_NORMAL_COMPLETION ? 
          printf("%d\n", DDInfo.count ):
          printf("error\n");
      printf("       Entity count: ");
      mcStatus = MC_Dir_Entity_Count( DDInfo.dirID,
                                      DDInfo.entityID,
                                      &DDInfo.entityCount );
      mcStatus == MC_NORMAL_COMPLETION ?
          printf("%d\n", DDInfo.entityCount ):
          printf("error\n");
      printf ( "\n" );
      printf ( "==> " );
      fflush ( stdout );
      fgets ( user_input,sizeof(user_input),stdin );
      fflush ( stdout );

      if ( user_input[2] != '\0' )
         strcpy ( argument, &user_input[2] );

      if ( !FileExists(DDInfo.dirname) && user_input[0] != 'x'
                                       && user_input[0] != 'X'
                                       && user_input[0] != '9')
      {
          printf("\tThe DICOM Directory (%s) doesn't exist.\n", DDInfo.dirname);
          continue;
      }

      switch ( user_input[0] )
      {
          case '1': /* reloading DICOMDIR */
              printf("\nReloading DICOMDIR.\n");
        
              /*
               * first free the file, this could also be a 
               * MC_Empty_File function call, then the following
               * MC_Create_File call would not be needed
               */
              mcStatus = MC_Free_File(&DDInfo.dirID); 
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to free file object",mcStatus);
                  exit ( EXIT_FAILURE );
              }
        
              /*
               * Check if DICOMDIR exists, if so, load it in
               */
              mcStatus = MC_Create_File(&DDInfo.dirID, DDInfo.dirname, "DICOMDIR",
                                      C_STORE_RQ);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to create file object",mcStatus);
                  exit ( EXIT_FAILURE );
              }
    
              mcStatus = MC_Open_File(DDInfo.fsuappID, DDInfo.dirID,
                                    &callbackInfo, MediaToFile);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to read file from media",mcStatus);
                  exit ( EXIT_FAILURE );
              }
       
              /*
               * get the internal variables for keeping track of the
               * location within the DICOMDIR
               */ 
              mcStatus = MC_Dir_Root_Entity(DDInfo.dirID, &DDInfo.entityID, 
                                          &DDInfo.recID, &DDInfo.type, &isLast);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to get root entity of DICOMDIR",mcStatus);
                  exit ( EXIT_FAILURE );
              }
              if (DDInfo.recID)
              {
                  mcStatus = MC_Get_Value_To_Int(DDInfo.recID, 
                               MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD,
                               &DDInfo.nextRecID);
                  if (mcStatus != MC_NORMAL_COMPLETION)
                  {
                      PrintError("Unable to get offset of the next directory record from DICOMDIR",mcStatus);
                      exit ( EXIT_FAILURE );
                  }
                  mcStatus = MC_Get_Value_To_Int(DDInfo.recID, 
                             MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                             &DDInfo.lowerRecID);
                  if (mcStatus != MC_NORMAL_COMPLETION)
                  {
                      PrintError("Unable to get offset of referenced lower level directory entity of DICOMDIR",mcStatus);
                      exit ( EXIT_FAILURE );
                  }
              }
          break;

          case '2':
              DisplayCurrentEntity(&DDInfo);
              mcStatus = MC_Dir_First_Record(DDInfo.dirID, DDInfo.entityID, 
                           &DDInfo.recID, &DDInfo.type ,&isLast);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to get first record within current entity",mcStatus);
                  exit ( EXIT_FAILURE );
              }
          break;

          case '3':
              MC_List_File(DDInfo.dirID,NULL);
          break;
        
          case '4':
              MC_List_Item(DDInfo.recID,NULL);
          break;
  
          case '5':
              GotoRootEntity( &DDInfo, &isLast );
          break;
 
          case '6':
              if (!DDInfo.lowerRecID)
              {
                  printf("\tLower level entity does not exist.\n");
                  continue;
              }    
              saveEntityID = DDInfo.entityID; 
              mcStatus = MC_Dir_Next_Entity(DDInfo.dirID, DDInfo.recID, 
                                    &DDInfo.entityID, &DDInfo.recID, 
                                    &DDInfo.type ,&isLast);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to get next directory entity",mcStatus);
                  DDInfo.entityID = saveEntityID;
                  (void)MC_Dir_First_Record(DDInfo.dirID, DDInfo.entityID, 
                                      &DDInfo.recID, &DDInfo.type ,&isLast);
              }
              else
              {
                  mcStatus = MC_Get_Value_To_Int(DDInfo.recID,
                               MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD, 
                               &DDInfo.nextRecID);
                  if (mcStatus != MC_NORMAL_COMPLETION)
                  {
                      PrintError("Unable to get next record within current entity",mcStatus);
                      exit ( EXIT_FAILURE );
                  }
                  mcStatus = MC_Get_Value_To_Int(DDInfo.recID, 
                       MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                       &DDInfo.lowerRecID);
                  if (mcStatus != MC_NORMAL_COMPLETION)
                  {
                      PrintError("Unable to get lower entity of current record",mcStatus);
                      exit ( EXIT_FAILURE );
                  }
              }
          break;
       
          case '7':
              mcStatus = MC_Dir_First_Record(DDInfo.dirID, DDInfo.entityID, 
                                     &DDInfo.recID, &DDInfo.type ,&isLast);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to get first directory record in entity",mcStatus);
              }
              else
              {
                  (void) MC_Get_Value_To_Int(DDInfo.recID, 
                             MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD, 
                             &DDInfo.nextRecID);
                  (void) MC_Get_Value_To_Int(DDInfo.recID, 
                       MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                       &DDInfo.lowerRecID);
              }
          break;

          case '8':
              if (!DDInfo.nextRecID)
              {
                  printf("\tNext directory record does not exist.\n");
                  continue;
              }    
              mcStatus = MC_Dir_Next_Record(DDInfo.dirID, DDInfo.entityID, 
                                    &DDInfo.recID, &DDInfo.type ,&isLast);
              if (mcStatus != MC_NORMAL_COMPLETION)
              {
                  PrintError("Unable to get next directory record",mcStatus);
                  (void)MC_Dir_First_Record(DDInfo.dirID, DDInfo.entityID, 
                                      &DDInfo.recID, &DDInfo.type ,&isLast);
              }
              else
              {
                  (void) MC_Get_Value_To_Int(DDInfo.recID, 
                             MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD, 
                             &DDInfo.nextRecID);
                  (void) MC_Get_Value_To_Int(DDInfo.recID, 
                       MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                       &DDInfo.lowerRecID);
              }
          break;

#ifndef UNIX
         case '9':
             printf("\n\tPress 'Q' or Esc to cancel server and\n");
             printf("\tserver will stop when session ends.\n");
             Wait_For_Association(&DDInfo);
         break;
#endif

         case 'c':
         case 'C':
             belowCount = 0;
             mcStatus = MC_Dir_Item_Count( DDInfo.dirID,
                                             DDInfo.recID,
                                             &belowCount );
             mcStatus == MC_NORMAL_COMPLETION ?
                 printf("Node Count:  %d\n", belowCount ):
                 printf("error\n");
             printf( "Press <return> to continue.\n" );
             fgets( entered, sizeof( entered ), stdin );

         break;

         case 'd':
         case 'D':
             mcStatus = MC_Dir_Delete_Record( DDInfo.dirID, DDInfo.recID );
             if (mcStatus != MC_NORMAL_COMPLETION)
             {
                 sprintf( errorString, "Unable to delete record:  %d\n", 
                 DDInfo.recID );
                 PrintError( errorString, mcStatus);
             }
             DDInfo.recID=0;
             DDInfo.lowerRecID=0;
             /*
             ** After we do a delete, this program will automatically go to
             ** the root of the DICOMDIR.  A real application probable doesn't
             ** want to do this.  This was chosen in order to keep this sample
             ** simple.
             */
             GotoRootEntity( &DDInfo, &isLast );
             break;

         case 'x':
         case 'X':
             done = 1;
         break;

      } /* switch */

   } /* while */

    /* ---------------------------------------------------------------- */
    /* We're done.  Close everything (somewhat) gracefully and clean up */
    /* ---------------------------------------------------------------- */

    mcStatus = MC_Release_Application(&DDInfo.scpappID);
    if (mcStatus != MC_NORMAL_COMPLETION)
        printf("Unable to release SCP application.\n");

    mcStatus = MC_Release_Application(&DDInfo.fsuappID);
    if (mcStatus != MC_NORMAL_COMPLETION)
        printf("Unable to release media application.\n");

#ifdef UNIX
    /*
     * Kill child process 
     */
    S_PInfo.terminate = 1;
    kill(S_PInfo.child_save_PID, SIGKILL);
    printf("Killed child PID (%d)\n", S_PInfo.child_save_PID);
#endif 
 
    if (MC_Library_Release() != MC_NORMAL_COMPLETION)
        printf("Error releasing the library.\n");
    printf("End of program.\n");
    return(EXIT_SUCCESS);

}   /* END main() */




/*************************************************************************
 *
 *  Function    :   initialize()
 *
 *  Parameters  :   DicomDirInfo *A_DDInfo 
 *
 *  Returns     :   nothing
 *
 *  Description :   Initializes the DICOM Directory, and initializes
 *                  the library.  Sets up signals and forks a process to 
 *                  handle the associations if defined UNIX.
 *
 *************************************************************************/
static void initialize(DicomDirInfo* A_DDInfo)
{
    MC_STATUS     mcStatus;
    int           isLast;         /* variable used for MC_Dir...  calls */
    CBinfo        callbackInfo;   /* Calback structure */


    /*
     * Formulate dicomdir pathname
     */
    if (A_DDInfo->pathname[0])
    {
        strcpy(A_DDInfo->dirname,A_DDInfo->pathname);
        strcat(A_DDInfo->dirname,DIRECTORY_SLASH_STRING);
        strcat(A_DDInfo->dirname,"DICOMDIR");
    }
    else
    {
        strcpy(A_DDInfo->dirname,"DICOMDIR");
    }


    /*
     * Check if specified dicomdir exists
     */
    if (FileExists(A_DDInfo->dirname))
    {
        printf("Reading DICOM Directory from %s. \n", A_DDInfo->dirname);
        A_DDInfo->direxists = 1;
    }
    else
        printf("Creating DICOM Directory at %s. \n", A_DDInfo->dirname);

    printf("\n");


    /* ------------------------------------------------------- */
    /* This call MUST be the first call made to the library!!! */
    /* ------------------------------------------------------- */
    mcStatus = MC_Library_Initialization ( NULL, NULL, NULL );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("\tUnable to initialize library:\n");
        printf("\t\t%s\n", MC_Error_Message(mcStatus));
        exit ( EXIT_FAILURE );
    }

    /*
     *  Register this DICOM application MERGE_MEDIA is used to write
     *  to the DICOM storage media.
     */
    mcStatus = MC_Register_Application(&A_DDInfo->fsuappID,
                                     FSUAETITLE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("\tUnable to register \"%s\":\n", FSUAETITLE);
        printf("\t\t%s\n", MC_Error_Message(mcStatus));
        exit(EXIT_FAILURE);
    }

    /*
     *  Register this DICOM application MERGE_MEDIA_FSU is used to
     *  connect to a DICOM remote storage service class user.
     */
    mcStatus = MC_Register_Application(&A_DDInfo->scpappID,
                                     SCPAETITLE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("\tUnable to register \"%s\":\n", SCPAETITLE);
        printf("\t\t%s\n", MC_Error_Message(mcStatus));
        exit(EXIT_FAILURE);
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

#endif
    /*
     * Create new DICOMDIR object
     */
    mcStatus = MC_Create_File(&A_DDInfo->dirID, A_DDInfo->dirname, "DICOMDIR",
                            C_STORE_RQ);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to create file object",mcStatus);
        exit ( EXIT_FAILURE );
    }
   
    /*
     * Check if DICOMDIR exists, if so, load it in
     */
    if (A_DDInfo->direxists)
    {
        mcStatus = MC_Open_File(A_DDInfo->fsuappID, A_DDInfo->dirID,
                              &callbackInfo, MediaToFile);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("UUnable to read file from media",mcStatus);
            exit ( EXIT_FAILURE );
        }
    }
    else
    {
        /*
         * Intialize DICOMDIR's Attributes
         */
        mcStatus = AddDICOMDIRGroupTwoElements(A_DDInfo);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to add group 2 elements to DICOMDIR",mcStatus);
            exit ( EXIT_FAILURE );
        }

    }

    /*
     * Initialize internal variables that keep track of the current
     * location within the DICOMDIR
     */
    mcStatus = MC_Dir_Root_Entity(A_DDInfo->dirID, &A_DDInfo->entityID,
                                &A_DDInfo->recID, &A_DDInfo->type, &isLast);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get root entity of DICOMDIR",mcStatus);
        exit ( EXIT_FAILURE );
    }
    if (A_DDInfo->recID)
    {
        mcStatus = MC_Get_Value_To_Int(A_DDInfo->recID,
                                     MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD,
                                     &A_DDInfo->nextRecID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError(
              "Unable to get offset of the next directory record from DICOMDIR",
              mcStatus);
            exit ( EXIT_FAILURE );
        }
        mcStatus = MC_Get_Value_To_Int(A_DDInfo->recID,
                     MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                     &A_DDInfo->lowerRecID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError(
             "Unable to get offset of lower level directory entity of DICOMDIR",
             mcStatus);
            exit ( EXIT_FAILURE );
        }
    }

#ifdef UNIX
    /*
     * Start single child process to handle any incoming associations
     */
    Start_Association_Process(A_DDInfo);
#endif

} /* initialize() */




/*************************************************************************
 *
 *  Function    :   print_cmdline
 *
 *  Parameters  :   none
 *
 *  Returns     :   nothing
 *
 *  Description :   Prints program usage
 *
 *************************************************************************/
static void print_cmdline(void)
{
    printf("\nUsage: med_fsu <filename> [-t <syntax>]\n\n");
    printf("    -d <pathname> = Optional specify location of DICOMDIR.\n");
    printf("                    <pathname> = pathname to store DICOMDIR in\n");
    printf("    -m            = Optional don't write DICOMDIR to media\n"); 
    printf("Example: med_fsu -m -d /DEV/MERGECOM\n\n");
} /* end print_cmdline() */




/*************************************************************************
 *
 *  Function    :   test_cmdline
 *
 *  Purpose     :   Test Command Line
 *
 *  Arguements  :   int A_argc,              # of command line arguments
 *                  char *A_argv[]           Array of cmd line arguments
 *                  DicomDirInfo *A_DDInfo   structure of dicomdir info
 *
 *  Return value:   MC_NORMAL_COMPLETION
 *                  MC_CANNOT_COMPLY
 *
 *  Description :   Test command line for valid arguements and the
 *                  existence of the input file.  If problems are found
 *                  display a message and return MC_CANNOT_COMPLY
 *
 *************************************************************************/
static MC_STATUS test_cmdline( int A_argc, char *A_argv[],
                               DicomDirInfo *A_DDInfo)
{
    MC_STATUS return_value = MC_NORMAL_COMPLETION;  /* Default to Success */
    int i;
    
    /*
    ** Initialize the pathname component to be NULL, such that, if we
    ** are given a cmd line argument, only then will we fill it in.
    */
    A_DDInfo->pathname[0] = '\0';

    for (i = 1; i < A_argc; i++)
    {
        if ( !strcmp(A_argv[i], "-h") || !strcmp(A_argv[i], "/h") ||
             !strcmp(A_argv[i], "-H") || !strcmp(A_argv[i], "/H") ||
             !strcmp(A_argv[i], "-?") || !strcmp(A_argv[i], "/?"))
        {
            print_cmdline();
            return_value = MC_CANNOT_COMPLY; 
        }
        else if ( !strcmp(A_argv[i], "-d") || !strcmp(A_argv[i], "-D"))
        {
            i++;
            strncpy(A_DDInfo->pathname,A_argv[i],PATH_LENGTH); 
        } 
        else if ( !strcmp(A_argv[i], "-m") || !strcmp(A_argv[i], "-M"))
        {
            A_DDInfo->memory = 1;
        }
    }
    return return_value;

}/* test_cmd_line() */



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
#ifdef UNIX    
    char        prefix[20] = {0};
    sprintf(prefix, "PID %d:", getpid() );
    printf("\t%s%s:\n",prefix,A_string);
    printf("\t%s\t%s\n",prefix,MC_Error_Message(A_mcStatus));
#else
    printf("\t%s:\n",A_string);
    printf("\t\t%s\n", MC_Error_Message(A_mcStatus));
#endif
}


/****************************************************************************
 *
 *  Function    :   GotoRootEntity
 *
 *  Description :   
 *
 ****************************************************************************/
static void
GotoRootEntity(
               DicomDirInfo *A_DDInfo,
               int          *isLast
              )
{
    MC_STATUS     mcStatus;   /* return value from MergeCOM-3 calls */

    mcStatus = MC_Dir_Root_Entity(A_DDInfo->dirID, &A_DDInfo->entityID, 
                                  &A_DDInfo->recID, &A_DDInfo->type ,isLast);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get root entity",mcStatus);
        (void)MC_Dir_First_Record(A_DDInfo->dirID, A_DDInfo->entityID, 
                                  &A_DDInfo->recID, &A_DDInfo->type ,isLast);
    }
    else
    {
        if ( !A_DDInfo->recID )
        {
            printf("\tNo records in this entity.\n");
            return;
        }
        mcStatus = MC_Get_Value_To_Int(A_DDInfo->recID, 
                                  MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD, 
                                       &A_DDInfo->nextRecID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to get next record within current entity",
                       mcStatus);
            exit ( EXIT_FAILURE );
        }
        mcStatus = MC_Get_Value_To_Int(A_DDInfo->recID,
                      MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                                       &A_DDInfo->lowerRecID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to get lower entity of current record",mcStatus);
            exit ( EXIT_FAILURE );
        }
    }
} /* end of function */


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
 *                  Note:  This function is not thread safe!  If adapting to
 *                  a threaded environment, semaphores should be used to
 *                  restrict access the static variables used.  Also, 
 *                  the function should be modified to pass in the variable
 *                  to store the UID.
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
 *  Function    :   DisplayCurrentRecord
 *
 *  Parameters  :   The ID of the directory record to display
 *                  
 *  Returns     :   none
 *
 *  Description :   Display the current directory record
 *
 ****************************************************************************/
static void DisplayCurrentRecord(int A_recID)
{
MC_STATUS   mcStatus;
char        string[50];

    mcStatus = MC_Get_Value_To_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                    sizeof(string),string);
    if (mcStatus != MC_NORMAL_COMPLETION) return;
    if (!strncmp("PATIENT",string,7))
    {
        /* Print PATIENT record */
        printf("Patient record\n");
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_PATIENTS_NAME,
                                      sizeof(string),string);
        printf("       Patient name: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_PATIENT_ID,
                                      sizeof(string),string);
        printf("         Patient ID: %s\n",string);
    }
    else if (!strncmp("STUDY",string,5))
    {
        /* Print STUDY record */
        printf("Study record\n");
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_STUDY_DESCRIPTION,
                                      sizeof(string),string);
        printf("  Study Description: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_STUDY_ID,
                                      sizeof(string),string);
        printf("           Study ID: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_STUDY_INSTANCE_UID,
                                      sizeof(string),string);
        printf(" Study Instance UID: %s\n",string);
    }
    else if (!strncmp("SERIES",string,6))
    {
        /* Print SERIES record */
        printf("Series record\n");
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_SERIES_NUMBER,
                                      sizeof(string),string);
        printf("      Series Number: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_SERIES_INSTANCE_UID,
                                      sizeof(string),string);
        printf("Series Instance UID: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_MODALITY,
                                      sizeof(string),string);
        printf("           Modality: %s\n",string);
    }
    else if (!strncmp("IMAGE",string,5)
        ||   !strncmp("STORED PRINT",string,12) 
        ||   !strncmp("RT DOSE",string,7) 
        ||   !strncmp("RT STRUCTURE SET",string,16) 
        ||   !strncmp("RT PLAN",string,7) 
        ||   !strncmp("RT TREAT RECORD",string,15))
    {
        /* Print IMAGE record */
        printf("%s record\n", string);
        string[0] = '\0';
        (void) GetFileNameAttribute(A_recID, MC_ATT_REFERENCED_FILE_ID,
                                    sizeof(string),string);
        printf("            File ID: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_IMAGE_NUMBER,
                                      sizeof(string),string);
        printf("       Image Number: %s\n",string);
        string[0] = '\0';
        (void) MC_Get_Value_To_String(A_recID, MC_ATT_IMAGE_TYPE,
                                      sizeof(string),string);
        printf("         Image Type: %s\n",string);
    }
    else
    {
        /* Print other record */
        printf("%s record\n",string);
    
    }
} /* End DisplayCurrentRecord() */



/****************************************************************************
 *
 *  Function    :   DisplayCurrentEntity
 *
 *  Description :   Display the current directory entity
 *
 ****************************************************************************/
static void DisplayCurrentEntity(DicomDirInfo *A_DDInfo)
{
    MC_STATUS   mcStatus;
    int         itemID;
    int         isLast;
    char*       itemType;

    mcStatus = MC_Dir_First_Record(A_DDInfo->dirID, A_DDInfo->entityID, &itemID, &itemType, &isLast);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get first record in entity",mcStatus);
        return;
    }
    
    DisplayCurrentRecord(itemID);
    printf("\n"); 
    while (!isLast)
    {
        mcStatus = MC_Dir_Next_Record(A_DDInfo->dirID, A_DDInfo->entityID, &itemID, &itemType, &isLast);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to get record in entity",mcStatus);
            return;
        }
    
        DisplayCurrentRecord(itemID); 
        printf("\n"); 
    }
} /* End DisplayCurrentEntity() */


/****************************************************************************
 *
 *  Function    :   SetValue
 *
 *  Description :   Used to copy the attribute Atag from the message A_msgID
 *                  into the directory record ArecID.  This routine is 
 *                  specifically written not to give an error if Atag does
 *                  not exist in A_msgID. 
 *
 ****************************************************************************/
static MC_STATUS SetValue( int           A_msgID, 
                           int           A_recID, 
                           unsigned long A_tag,
                           DICOM_TYPE    A_type )
{
    MC_STATUS   mcStatus;
    char        string[1024];
    char        errorString[128];

    mcStatus = MC_Get_Value_To_String(A_msgID,A_tag,sizeof(string),string);
    if (mcStatus == MC_NORMAL_COMPLETION)
    {
        mcStatus = MC_Set_Value_From_String(A_recID,A_tag,string);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            sprintf(errorString,"Error Setting tag from string: %lx",A_tag);
            PrintError(errorString,mcStatus);
            return mcStatus;
        }
    }
    else if (mcStatus == MC_NULL_VALUE)
    {
        if (A_type == Type_2)
        {
            mcStatus = MC_Set_Value_To_NULL(A_recID, A_tag);
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                sprintf(errorString,"Error setting tag (%lx) to NULL",A_tag);
                PrintError(errorString,mcStatus);
                return mcStatus;
            }
        }
        else if (A_type == Type_1)
        {
            sprintf(errorString,"Value for tag (%lx) set to NULL, expecting value",A_tag);
            PrintError(errorString,mcStatus);
            return mcStatus;
        
        }
    }
    else if (A_type == Type_2)
    {
        mcStatus = MC_Set_Value_To_NULL(A_recID, A_tag);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            sprintf(errorString,"Error setting tag (%lx) to NULL",A_tag);
            PrintError(errorString,mcStatus);
            return mcStatus;
        }
    }
    else if (A_type == Type_1)
    {
        sprintf(errorString, 
                "Error with tag (%lx), expecting value in message", 
                A_tag);
        PrintError(errorString,mcStatus);
        return mcStatus;
    }
    
    return MC_NORMAL_COMPLETION;
    
} /* End SetValue() */


/****************************************************************************
 *
 *  Function    :   FillPatientRecord
 *
 *  Description :   Fills the correct attributes of a patient directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillPatientRecord(int A_msgID, int A_recID)
{
    MC_STATUS mcStatus;

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "PATIENT");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;
    
    mcStatus = MC_Set_Value_From_UInt(A_recID, MC_ATT_RECORD_IN_USE_FLAG, 0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_PATIENTS_NAME, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_PATIENT_ID, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_PATIENTS_BIRTH_DATE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_PATIENTS_SEX, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_PATIENT_COMMENTS, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FilePatientRecord() */



/****************************************************************************
 *
 *  Function    :   FillStudyRecord
 *
 *  Description :   Fills the correct attributes of a study directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillStudyRecord(int A_msgID, int A_recID)
{
    MC_STATUS   mcStatus;

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "STUDY");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STUDY_DATE, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STUDY_TIME, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STUDY_DESCRIPTION, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STUDY_INSTANCE_UID, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STUDY_ID, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_ACCESSION_NUMBER, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillStudyRecord() */




/****************************************************************************
 *
 *  Function    :   FillSeriesRecord
 *
 *  Description :   Fills the correct attributes of a series directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillSeriesRecord(int A_msgID, int A_recID)
{
    MC_STATUS   mcStatus;

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "SERIES");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_MODALITY, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_SERIES_INSTANCE_UID, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_SERIES_NUMBER, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_SERIES_DATE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_SERIES_TIME, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_SERIES_DESCRIPTION, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillSeriesRecord() */




/****************************************************************************
 *
 *  Function    :   FillStoredPrintRecord
 *
 *  Description :   Fills the correct attributes of a stored print directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillStoredPrintRecord(int A_msgID, int A_recID, char* A_fileName)
{
    MC_STATUS   mcStatus;
    char        uidBuffer[UI_LENGTH+2];


    /*
     * Get the UID from the file to determine what directory type should be 
     * used.
     */
    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                     "STORED PRINT");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;


    /*
     * Set the SOP Class and Instance UIDs in the directory record
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Class UID in file",mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Instance UID in file",mcStatus);
        return mcStatus;
    }
    

    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_NUMBER, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_DATE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_TIME, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_TYPE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetFileNameAttribute(A_recID,MC_ATT_REFERENCED_FILE_ID,A_fileName);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillStoredPrintRecord() */




/****************************************************************************
 *
 *  Function    :   FillRTDoseRecord
 *
 *  Description :   Fills the correct attributes of a image directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillRTDoseRecord(int A_msgID, int A_recID, char* A_fileName)
{
    MC_STATUS   mcStatus;
    char        uidBuffer[UI_LENGTH+2];
    

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "RT DOSE");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;


    /*
     * Set the SOP Class and Instance UIDs in the directory record
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Class UID in file",mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Instance UID in file",mcStatus);
        return mcStatus;
    }
    

    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_NUMBER, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;
    
    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_DOSE_SUMMATION_TYPE, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_DOSE_COMMENT, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;
    
    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_DATE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_TIME, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_TYPE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetFileNameAttribute(A_recID,MC_ATT_REFERENCED_FILE_ID,A_fileName);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillRTDoseRecord() */




/****************************************************************************
 *
 *  Function    :   FillRTPlanRecord
 *
 *  Description :   Fills the correct attributes of a image directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillRTPlanRecord(int A_msgID, int A_recID, char* A_fileName)
{
    MC_STATUS   mcStatus;
    char        uidBuffer[UI_LENGTH+2];
    

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "RT PLAN");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;


    /*
     * Set the SOP Class and Instance UIDs in the directory record
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Class UID in file",mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Instance UID in file",mcStatus);
        return mcStatus;
    }


    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_NUMBER, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_RT_PLAN_LABEL, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_RT_PLAN_DATE, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_RT_PLAN_TIME, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetFileNameAttribute(A_recID,MC_ATT_REFERENCED_FILE_ID,A_fileName);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillRTPlanRecord() */




/****************************************************************************
 *
 *  Function    :   FillRTStructureSetRecord
 *
 *  Description :   Fills the correct attributes of a image directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillRTStructureSetRecord(int A_msgID, int A_recID, char* A_fileName)
{
    MC_STATUS   mcStatus;
    char        uidBuffer[UI_LENGTH+2];
    

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "RT STRUCTURE SET");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;


    /*
     * Set the SOP Class and Instance UIDs in the directory record
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Class UID in file",mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Instance UID in file",mcStatus);
        return mcStatus;
    }


    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_NUMBER, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STRUCTURE_SET_LABEL, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STRUCTURE_SET_DATE, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_STRUCTURE_SET_TIME, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetFileNameAttribute(A_recID,MC_ATT_REFERENCED_FILE_ID,A_fileName);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillRTStructureSetRecord() */



/****************************************************************************
 *
 *  Function    :   FillRTTreatRecord
 *
 *  Description :   Fills the correct attributes of a RT treatment record 
 *                  directory record to match with the image pointed to by 
 *                  A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillRTTreatRecord(int A_msgID, int A_recID, char* A_fileName)
{
    MC_STATUS   mcStatus;
    char        uidBuffer[UI_LENGTH+2];
    

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "RT TREAT RECORD");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;
    
   
    /*
     * Set the SOP Class and Instance UIDs in the directory record
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Class UID in file",mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Instance UID in file",mcStatus);
        return mcStatus;
    }
       

    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_NUMBER, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_TREATMENT_DATE, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_TREATMENT_TIME, Type_2);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;


    mcStatus = SetFileNameAttribute(A_recID,MC_ATT_REFERENCED_FILE_ID,A_fileName);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillRTTreatRecord() */



/****************************************************************************
 *
 *  Function    :   FillImageRecord
 *
 *  Description :   Fills the correct attributes of a image directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/
static MC_STATUS FillImageRecord(int A_msgID, int A_recID, char* A_fileName)
{
    MC_STATUS   mcStatus;
    char        uidBuffer[UI_LENGTH+2];
    

    mcStatus = MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,
                                      "IMAGE");
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    /*
     * Set the SOP Class and Instance UIDs in the directory record
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Class UID in file",mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from message",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to set Referenced SOP Instance UID in file",mcStatus);
        return mcStatus;
    }
    
    
    mcStatus = MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_NUMBER, Type_1);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_DATE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_TIME, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetValue(A_msgID,A_recID,MC_ATT_IMAGE_TYPE, Type_3);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    mcStatus = SetFileNameAttribute(A_recID,MC_ATT_REFERENCED_FILE_ID,A_fileName);
    if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;

    return MC_NORMAL_COMPLETION;
    
} /* End FillImageRecord() */



/****************************************************************************
 *
 *  Function    :   AddMessageToDICOMDIR
 *
 *  Description :   Adds a directory record to the DICOMDIR dirID for the
 *                  message A_msgID.  If records do not exist for the patient,
 *                  series, and study for the message, these records will
 *                  also be created.
 *
 ****************************************************************************/
static MC_STATUS AddMessageToDICOMDIR(int A_msgID, char* A_serviceName,
                                      char* A_fileName, 
                                      DicomDirInfo* A_DDInfo )
{
    MC_STATUS mcStatus;
    char    patientID[100];     /* These 3 buffers are the patient,    */
    char    studyUID[UI_LENGTH+1]; /* series, and study identifiers found */
    char    seriesUID[UI_LENGTH+1];/* in the message A_msgID.              */
    
    char    dirPatientID[100];        /* These 3 buffers are used to get the    */
    char    dirStudyUID[UI_LENGTH+1]; /* patient, series, and study identifiers */
    char    dirSeriesUID[UI_LENGTH+1];/* while traversing the DICOMDIR          */

    int     entityID;   /* to keep track of entity while traversing DICOMDIR */
    int     recID;      /* to keep track of records while traversing DICOMDIR */
    int     isLast;     /* used while traversing DICOMDIR */
    char*   itemType;   /* type of directory record */
    char*   recTypeToCreate;   /* type of directory record to create */
    int     first;

    int     createPatient = 1;     /* These 6 boolean variables are used to  */
    int     createStudy = 1;       /* identify which records need to be      */
    int     createSeries = 1;      /* created in the DICOMDIR to place the   */
    int     createStudyEntity = 1; /* image A_msgID into the the DICOMDIR.    */
    int     createSeriesEntity = 1;/* They also tell whether entities        */
    int     createImageEntity = 1; /* have to be created.                    */


    /*
     * Get the patient ID, study instance UID, and series instance UID
     * identifiers from the message object.  These strings are used to 
     * compare with records in the DICOMDIR to see if the patient,
     * study, or series already appear in the DICOMDIR.
     */
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_PATIENT_ID,
                                    sizeof(patientID),patientID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get Patient ID from message",mcStatus);
        return mcStatus;
    }
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_STUDY_INSTANCE_UID,
                                    sizeof(studyUID), studyUID);
    if (mcStatus != MC_NORMAL_COMPLETION) 
    {
        PrintError("Unable to get study instance UID from message",mcStatus);
        return mcStatus;
    }
    mcStatus = MC_Get_Value_To_String(A_msgID, MC_ATT_SERIES_INSTANCE_UID,
                                    sizeof(seriesUID), seriesUID);
    if (mcStatus != MC_NORMAL_COMPLETION) 
    {
        PrintError("Unable to get series instance UID from message",mcStatus);
        return mcStatus;
    }


    /*
     * The following loop scans to see if the root entity contains a record
     * for the same Patient ID as in A_msgID.  If this is the case, it continues
     * on searching the referenced entity of this record to see if the study
     * already appears for the patient.  If the study exists, it searches its
     * lower level entity to see if the series already exists for this image.
     * If this is the case, it places a new image record in the lower
     * level entity of the series record.
     */
    isLast = 0;
    first = 1;
    while (!isLast)
    {
        if (first)
        {
            mcStatus = MC_Dir_Root_Entity(A_DDInfo->dirID, &entityID, &recID,
                                        &itemType, &isLast);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to get root entity record from DICOMDIR", 
                           mcStatus);
                return mcStatus;
            }

            if (recID == 0)
            {
                createPatient = 1;
                break; 
            } 
            first = 0;
        }
        else
        {
            mcStatus = MC_Dir_Next_Record(A_DDInfo->dirID, entityID, &recID,
                                        &itemType, &isLast);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to get root entity record from DICOMDIR",
                           mcStatus);
                return mcStatus;
            }
        } 

        mcStatus = MC_Get_Value_To_String(recID, MC_ATT_PATIENT_ID,
                                        sizeof(dirPatientID), dirPatientID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            PrintError("Unable to get patient ID from root directory entity",
                       mcStatus);
            return mcStatus;
        } 
            
        if (!strcmp(patientID,dirPatientID))
        {
            /*
             * At this point a matching patient ID has been found
             */
            createPatient = 0;

            mcStatus = MC_Dir_Next_Entity(A_DDInfo->dirID, recID, &entityID,
                                        &recID, &itemType, &isLast); 
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to get next entity below patient record",
                           mcStatus);
                return mcStatus;
            }
            if (entityID == 0)
            {
                createStudyEntity = 1;
                break; /* patient ID is found, but no study entity found */
            }
            else
                createStudyEntity = 0;

            /*
             * Scan if study directory record ready
             */        
            isLast = 0;
            first = 1;

            while (!isLast)
            {
                if (first)
                {
                    mcStatus = MC_Dir_First_Record(A_DDInfo->dirID, entityID,
                                                 &recID, &itemType, &isLast);
                    first = 0;
                }
                else
                {
                    mcStatus = MC_Dir_Next_Record(A_DDInfo->dirID, entityID,
                                                &recID, &itemType, &isLast);
                } 
                if (mcStatus != MC_NORMAL_COMPLETION) 
                {
                    PrintError("Unable to get directory record from study entity",mcStatus);
                    return mcStatus;
                }

                mcStatus = MC_Get_Value_To_String(recID, 
                                                MC_ATT_STUDY_INSTANCE_UID,
                                                sizeof(dirStudyUID),
                                                dirStudyUID);
                if (mcStatus != MC_NORMAL_COMPLETION) 
                {
                    PrintError("Unable to get study instance UID from directory record",mcStatus);
                    return mcStatus;
                }
                 
                if (!strcmp(studyUID,dirStudyUID))
                {
                    /*
                     * At this point a matching patient ID and study UID 
                     * has been found in the DICOMDIR
                     */
                    createStudy = 0;

                    mcStatus = MC_Dir_Next_Entity(A_DDInfo->dirID, recID,
                                                &entityID, &recID, &itemType,
                                                &isLast); 
                    if (mcStatus != MC_NORMAL_COMPLETION) 
                    {
                        PrintError("Unable to get next entity below study record",mcStatus);
                        return mcStatus;
                    }

                    if (entityID == 0)
                    {
                        /* patient ID is found, study found, but no series
                         *  entity found
                         */
                        createSeriesEntity = 1;
                        break; 
                    }
                    else
                        createSeriesEntity = 0;
                
                    /*
                     * Scan if series directory record ready
                     */        
                    isLast = 0;
                    first = 1;
                   
                    while (!isLast)
                    {
                        if (first)
                        {
                            mcStatus = MC_Dir_First_Record(A_DDInfo->dirID, 
                                                         entityID, &recID,
                                                         &itemType, &isLast);
                            first = 0;
                        }
                        else
                        {
                            mcStatus = MC_Dir_Next_Record(A_DDInfo->dirID,
                                                        entityID, &recID,
                                                        &itemType, &isLast);
                            
                        } 
                        if (mcStatus != MC_NORMAL_COMPLETION) 
                        {
                            PrintError("Unable to get series record from directory record",mcStatus);
                            return mcStatus;
                        }

                        mcStatus = MC_Get_Value_To_String(recID,
                                      MC_ATT_SERIES_INSTANCE_UID,
                                      sizeof(dirSeriesUID),dirSeriesUID);
                        if (mcStatus != MC_NORMAL_COMPLETION) 
                        {
                            PrintError("Unable to get series instance UID from message",mcStatus);
                            return mcStatus;
                        } 
                        if (!strcmp(seriesUID,dirSeriesUID))
                        {
                            /*
                             * At this point a matching patient ID, study UID,
                             * and series UID has been found in the DICOMDIR,
                             * only the image record needs to be created
                             */ 
                            createSeries = 0;

                            mcStatus = MC_Dir_Next_Entity(A_DDInfo->dirID, recID,
                                          &entityID,&recID,&itemType,&isLast); 
                            if (mcStatus != MC_NORMAL_COMPLETION) 
                            {
                                PrintError("Unable to get next entity below series record",mcStatus);
                                return mcStatus;
                            } 
                            if (entityID == 0)
                                createImageEntity = 1;
                            else 
                                /*
                                 * Note, at this point we always automatically
                                 * insert an image record.  We should at this 
                                 * point search the image entity to see if we've
                                 * already received this record, and reject
                                 * the C-STORE-RQ if we have.  We don't do this
                                 * here for simplicity sake.
                                 */
                                createImageEntity = 0;
                            
                            /* patient, study, and series already in DICOMDIR */
                            break;
                        } /* end if (!strcmp(seriesUID,dirSeriesUID)) */ 
                    }
                    break;
                } /* end if (!strcmp(studyUID,dirStudyUID)) */ 
            } 
            break;
        } /* end if (!strcmp(patientID,dirPatientID)) */

    } 


    /*
     * Note, the below tests assume that the variables recID, and entityID 
     * are set correctly, along with the values for createPatient, 
     * createStudy, createSeries, createStudyEntity, createSeriesEntity,
     * and createImageEntity to properly create the correct records.
     */
    if (createPatient)
    {
        /*
         * Create Patient directory record
         */  
        mcStatus = MC_Dir_Add_Record(A_DDInfo->dirID, entityID, 
                                   "DIR_REC_PATIENT", &recID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            PrintError("Unable to add patient directory record",mcStatus);
            return mcStatus;
        } 
        mcStatus = FillPatientRecord(A_msgID,recID); 
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            MC_Dir_Delete_Record(A_DDInfo->dirID, recID);
            PrintError("Unable to fill patient directory record",mcStatus);
            return mcStatus;
        }
        createStudyEntity = 1;
    }
    
    if (createStudy)
    {
        /*
         * Create Study directory record
         */
        if (createStudyEntity)
        {
            mcStatus = MC_Dir_Add_Entity(A_DDInfo->dirID, recID, 
                                       "DIR_REC_STUDY", &entityID, &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to add study directory entity",mcStatus);
                return mcStatus;
            } 
            createSeriesEntity = 1;
        }
        else
        {
            mcStatus = MC_Dir_Add_Record(A_DDInfo->dirID, entityID, 
                                       "DIR_REC_STUDY", &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to add study directory record",mcStatus);
                return mcStatus;
            }
        }
        mcStatus = FillStudyRecord(A_msgID,recID); 
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            MC_Dir_Delete_Record(A_DDInfo->dirID, recID);
            PrintError("Unable to fill study directory record",mcStatus);
            return mcStatus;
        }
            
    }

    if (createSeries)
    {
        /*
         * Create Series directory record
         */  
        if (createSeriesEntity)
        {
            mcStatus = MC_Dir_Add_Entity(A_DDInfo->dirID, recID, 
                                       "DIR_REC_SERIES", &entityID, &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to add series directory entity",mcStatus);
                return mcStatus;
            } 
            createImageEntity = 1;
        }
        else
        {
            mcStatus = MC_Dir_Add_Record(A_DDInfo->dirID, entityID, 
                                       "DIR_REC_SERIES", &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                PrintError("Unable to add series directory record",mcStatus);
                return mcStatus;
            }
        } 
        mcStatus = FillSeriesRecord(A_msgID,recID); 
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            MC_Dir_Delete_Record(A_DDInfo->dirID, recID);
            PrintError("Unable to fill series directory record",mcStatus);
            return mcStatus;
        }
    }     

    
    /*
     * Assign the proper directory record type to use.
     */
    if (!strcmp(A_serviceName, "STANDARD_STORED_PRINT"))
    {
        recTypeToCreate = "DIR_REC_STORED_PRINT";
    }
    else if (!strcmp(A_serviceName, "STANDARD_RT_DOSE"))
    {
        recTypeToCreate = "DIR_REC_RT_DOSE";
    }
    else if (!strcmp(A_serviceName, "STANDARD_RT_PLAN"))
    {
        recTypeToCreate = "DIR_REC_RT_PLAN";
    }
    else if (!strcmp(A_serviceName, "STANDARD_RT_STRUCTURE_SET"))
    {
        recTypeToCreate = "DIR_REC_RT_STRUCTURE_SET";
    }
    else if (!strcmp(A_serviceName, "STANDARD_RT_BEAMS_TREAT")
          || !strcmp(A_serviceName, "STANDARD_RT_BRACHY_TREAT")
          || !strcmp(A_serviceName, "STANDARD_RT_TREAT_SUM"))
    {
        recTypeToCreate = "DIR_REC_RT_TREAT_RECORD";
    }
    else
    {
        recTypeToCreate = "DIR_REC_IMAGE";
    }


    /*
     * Create Image directory record always
     */ 
    if (createImageEntity)
    {
        mcStatus = MC_Dir_Add_Entity(A_DDInfo->dirID, recID, 
                                   recTypeToCreate, &entityID, &recID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            PrintError("Unable to add image directory entity",mcStatus);
            return mcStatus;
        }
    }
    else
    {
        mcStatus = MC_Dir_Add_Record(A_DDInfo->dirID, entityID, 
                                   recTypeToCreate, &recID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            PrintError("Unable to add image directory record",mcStatus);
            return mcStatus;
        }
    }


    if (!strcmp(recTypeToCreate, "DIR_REC_STORED_PRINT"))
        mcStatus = FillStoredPrintRecord(A_msgID,recID,A_fileName); 
    else if (!strcmp( recTypeToCreate, "DIR_REC_RT_DOSE" ))
        mcStatus = FillRTDoseRecord(A_msgID,recID,A_fileName); 
    else if (!strcmp( recTypeToCreate, "DIR_REC_RT_PLAN" ))
        mcStatus = FillRTPlanRecord(A_msgID,recID,A_fileName); 
    else if (!strcmp( recTypeToCreate, "DIR_REC_RT_STRUCTURE_SET" ))
        mcStatus = FillRTStructureSetRecord(A_msgID,recID,A_fileName); 
    else if (!strcmp( recTypeToCreate, "DIR_REC_RT_TREAT_RECORD" ))
        mcStatus = FillRTTreatRecord(A_msgID,recID,A_fileName); 
    else
        mcStatus = FillImageRecord(A_msgID,recID,A_fileName); 
    
    if (mcStatus != MC_NORMAL_COMPLETION) 
    {
        MC_Dir_Delete_Record(A_DDInfo->dirID, recID);
        PrintError("Unable to fill directory record",mcStatus);
    }
    
    return mcStatus;
} /* End AddMessageToDICOMDIR() */



/****************************************************************************
 *
 *  Function    :   AddDICOMDIRGroupTwoElements
 *
 *  Description :   Adds the required group two elements to a DIOCMDIR 
 *                  object.  Note that the implementation class UID and
 *                  implementation version name are automatically filled in
 *                  by the tool kit based on the mergecom.pro config file.
 *
 ****************************************************************************/
static MC_STATUS AddDICOMDIRGroupTwoElements( DicomDirInfo *A_DDInfo )
{
    MC_STATUS mcStatus;

    /*
     * Set media group 2 elements
     */
    mcStatus = MC_Set_Value_From_String(
                        A_DDInfo->dirID,
                        MC_ATT_TRANSFER_SYNTAX_UID,
                        "1.2.840.10008.1.2.1");
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add transfer syntax UID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_Function(
                        A_DDInfo->dirID,
                        MC_ATT_FILE_META_INFORMATION_VERSION,
                        NULL,
                        FileMetaInfoVersionCallback);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add file meta information version",mcStatus);
        return mcStatus;
    }
     
    mcStatus = MC_Set_Value_From_String(
                        A_DDInfo->dirID,
                        MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID,
                        "1.2.840.10008.1.3.10");
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP Class UID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(
                        A_DDInfo->dirID,
                        MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID,
                        Create_Inst_UID());
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP instance UID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(
                        A_DDInfo->dirID,
                        MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,
                        FSUAETITLE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP instance UID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(A_DDInfo->dirID, MC_ATT_FILE_SET_ID,
                                      FSUAETITLE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add file set ID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_UInt(A_DDInfo->dirID, 
                 MC_ATT_FILE_SET_CONSISTENCY_FLAG,0x0000);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add file set consistency flag",mcStatus);
        return mcStatus;
    }

    return MC_NORMAL_COMPLETION; 
    
} /* End AddDICOMDIRGroupTwoElements() */


/****************************************************************************
 *
 *  Function    :   AddImageGroupTwoElements
 *
 *  Description :   Adds the required group two elements to a file
 *                  object.  Note that the implementation class UID and
 *                  implementation version name are automatically filled in
 *                  by the tool kit based on the mergecom.pro config file.
 *
 ****************************************************************************/
static MC_STATUS AddImageGroupTwoElements(int A_fileID)
{
    MC_STATUS mcStatus;
    char      uidBuffer[UI_LENGTH+2];

    /*
     * Set media group 2 elements
     */
    mcStatus = MC_Set_Value_From_String(
                        A_fileID,
                        MC_ATT_TRANSFER_SYNTAX_UID,
                        "1.2.840.10008.1.2.1");
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add transfer syntax UID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_Function(
                        A_fileID,
                        MC_ATT_FILE_META_INFORMATION_VERSION,
                        NULL,
                        FileMetaInfoVersionCallback);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add file meta information version",mcStatus);
        return mcStatus;
    }
     
    mcStatus = MC_Get_Value_To_String(A_fileID, 
                                    MC_ATT_SOP_CLASS_UID,
                                    sizeof(uidBuffer),
                                    uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Class UID from image file",mcStatus);
        return mcStatus;
    }
                                    
    mcStatus = MC_Set_Value_From_String(
                        A_fileID,
                        MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID,
                        uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP Class UID",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Get_Value_To_String(A_fileID, 
                                    MC_ATT_SOP_INSTANCE_UID,
                                    sizeof(uidBuffer),
                                    uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to get SOP Instance UID from image file",mcStatus);
        return mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(
                        A_fileID,
                        MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID,
                        uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP instance UID",mcStatus);
        return mcStatus;
    }
   
    mcStatus = MC_Set_Value_From_String(
                        A_fileID,
                        MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,
                        FSUAETITLE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to add media storage SOP instance UID",mcStatus);
        return mcStatus;
    }
    
    return MC_NORMAL_COMPLETION; 
    
} /* End AddImageGroupTwoElements() */




/********************************************************************
 *
 *  Function    :   FileMetaInfoVersionCallback
 *
 *  Returns     :   1 - All went well
 *                  0 - Error occurred
 *
 *  Description :   Sets the value of MC_ATT_FILE_META_INFORMATION_VERSION
 *                  
 ********************************************************************/
static MC_STATUS FileMetaInfoVersionCallback( int           A_msgID, 
                                              unsigned long A_tag, 
                                              int           A_first,
                                              void*         A_info,
                                              int*          A_dataSize,
                                              void**        A_dataBufferPtr,
                                              int*          A_isLastPtr) 
{
    static unsigned char version[] = {0x00, 0x01};

    *A_dataSize = 2;
    *A_dataBufferPtr = version;
    *A_isLastPtr = 1;

    return MC_NORMAL_COMPLETION;

}

/****************************************************************************
 *
 *  Function    :   SetFileNameAttribute
 *
 *  Description :   Sets a filename attribute from a string.  This must be 
 *                  done because the DICOM standard dictates that heiarchial
 *                  directorys are listed as seperate values.
 *
 *                  This routine was written for the case where files/images
 *                  are placed in a directory within the root DICOMDIR 
 *                  directory.  This application currently just places
 *                  incoming files in the root directory. (ie, the same
 *                  directory the DICOMDIR is located in.)
 *
 ****************************************************************************/
static MC_STATUS SetFileNameAttribute( int             A_msgID, 
                                       unsigned long   A_tag, 
                                       char*           A_fileName)
{
    MC_STATUS mcStatus;
    char   tempBuffer[PATH_LENGTH+10];
    char*  strTokenPtr;
    int    first = 1;

    strncpy(tempBuffer,A_fileName,PATH_LENGTH+10);
    strTokenPtr = strtok(tempBuffer,DIRECTORY_SLASH_STRING);
    
    while (strTokenPtr)
    {
        if (first)
        {
            first = 0;
            mcStatus = MC_Set_Value_From_String(A_msgID,A_tag,strTokenPtr);
            if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;
        }
        else
        {
            mcStatus = MC_Set_Next_Value_From_String(A_msgID,A_tag,strTokenPtr);
            if (mcStatus != MC_NORMAL_COMPLETION) return mcStatus;
        }

        strTokenPtr = strtok(NULL,DIRECTORY_SLASH_STRING);
    } 

    return MC_NORMAL_COMPLETION;
    
} /* End SetFileNameAttribute() */



/****************************************************************************
 *
 *  Function    :   GetFileNameAttribute
 *
 *  Description :   Gets a filename attribute into a string.  
 * 
 *                  This routine was written for the case where files/images
 *                  are placed in a directory.  This application currently
 *                  just places incoming files in the root directory. (ie,
 *                  the same directory the DICOMDIR is located in.)
 *
 ****************************************************************************/
static MC_STATUS GetFileNameAttribute( int             A_msgID, 
                                       unsigned long   A_tag, 
                                       int             A_strLen,
                                       char*           A_fileName)
{
    MC_STATUS   mcStatus;
    char        tempBuffer[PATH_LENGTH+10];
    char*       bufferPtr;
    int         bufferLength = 0;
    size_t      tempLength;
    int         first=1;

    bufferPtr = tempBuffer; 
    for (;;)
    {
        if (first)
        {
            first = 0;
            mcStatus = MC_Get_Value_To_String( A_msgID, A_tag,
                          (PATH_LENGTH + 10) - bufferLength, bufferPtr);
        }
        else
        {
            mcStatus = MC_Get_Next_Value_To_String( A_msgID, A_tag,
                         (PATH_LENGTH + 10) - bufferLength, bufferPtr);
        }
        if (mcStatus == MC_NO_MORE_VALUES)
            break;
        else if (mcStatus != MC_NORMAL_COMPLETION)
            return mcStatus;
            
        tempLength = strlen(bufferPtr);
        bufferPtr[tempLength] = DIRECTORY_SLASH_CHAR;
        bufferPtr +=  (tempLength + 1);
        bufferLength += (tempLength + 1);
    } 
    if (bufferLength)
        tempBuffer[bufferLength-1] = '\0';
    else 
        tempBuffer[0] = '\0';
    
    strncpy(A_fileName,tempBuffer,A_strLen);
    return MC_NORMAL_COMPLETION;
    
} /* End GetFileNameAttribute() */



/****************************************************************************
 *
 *  Function    :   Wait_For_Association
 *
 *  Description :   Waits for an association.  This routine runs in a child
 *                  process for UNIX systems, or only when requested in the
 *                  MSDOS environment.
 *
 ****************************************************************************/
static void Wait_For_Association( DicomDirInfo *A_DDInfo )
{
    int         associationID;
    int         calledApplicationID;
    MC_STATUS   mcStatus;
#if defined(_MSDOS) || defined(_WIN32) || defined(__OS2__)
    int         quit = 0;
#elif defined(_MACINTOSH)
    EventRecord asEvent;
    Boolean     aqQuit = false;
#endif

    /*
     *  Loop, handling associations - waiting for quit keypress
     */
    for (;;)
    {      
#ifndef UNIX 
#if defined(_MSDOS) || defined(_WIN32)
#if defined(INTEL_BCC)
        if ( kbhit () )
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
#endif                                        
#endif
#if defined(_MACINTOSH)
        if (WaitNextEvent(mDownMask | mUpMask | keyDownMask |
            keyUpMask | autoKeyMask, &asEvent, 10, NULL)) 
        {
            if (asEvent.what == keyDown) 
            {
                unsigned char abChar = asEvent.message & charCodeMask;
                Boolean       aqCmnd = ((asEvent.modifiers & cmdKey) != 0);

                if ((abChar == 'Q') || (abChar == 'q') ||
                    (aqCmnd && (abChar == '.')))
                    aqQuit = true;
            } 
            else if (asEvent.what == mouseDown) 
            {
                WindowPtr apWindow;
                short     awWhere;

                awWhere = FindWindow(asEvent.where, &apWindow);
                if (awWhere == inMenuBar) 
                {
                    MenuSelect(asEvent.where);
                    HiliteMenu(0);
                } 
                else if (awWhere == inSysWindow)
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
   
        mcStatus = MC_Wait_For_Association("Storage_SCP_Service_List", 1,
                                     &calledApplicationID,
                                     &associationID);
        if (mcStatus == MC_TIMEOUT)
            continue;
        else if (mcStatus == MC_NEGOTIATION_ABORTED)
        {
            PrintError("Association negotiation aborted",mcStatus);
            continue;
        }
        else if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Error on MC_Wait_For_Association",mcStatus);
            MC_Abort_Association(&associationID);
            break;
        }
      
        if (calledApplicationID != A_DDInfo->scpappID)
        {
            printf("\tUnexpected application identifier on MC_Wait_For_Association.\n");
            MC_Abort_Association(&associationID);
            break;
        }

        mcStatus = Handle_Association(&associationID, A_DDInfo);
        if (mcStatus == MC_NORMAL_COMPLETION) 
            return;

    }   /* Loop till quit requested or error */
    return;
} /* End Wait_For_Association() */


/**************************************************************************
 *
 *  Function    :   FileExists
 *
 *  Parameters  :   A_pathname   - File name.
 *
 *  Returns     :   1 if the file exists else 0
 *
 *  Description:
 *
 *    The FileExists function returns 1 if the given Apathname exists,
 *    else 0 is returned.  The MSDOS implementation uses the _access
 *    function.
 *
 **************************************************************************/
static int FileExists( char *Apathname)
{
#if defined(_MSDOS) || defined(_WIN32)
#if defined(INTEL_BCC)
    return ( !access (Apathname, 0) );
#else
    return ( !_access (Apathname, 0) );
#endif
#else
    FILE *file;

    file = fopen (Apathname, "r");
    if (file == NULL)
        return 0;
    else
    {
        fclose (file);
        return 1;
    }
#endif

} /* End FileExists() */



/****************************************************************************
 *
 *  Function    :   Handle_Association
 *
 *  Description :   Processes a received association requests
 *
 ****************************************************************************/
static MC_STATUS Handle_Association(int* associationID, DicomDirInfo* A_DDInfo)
{
    char        fname[PATH_LENGTH+10];
    char        fullFname[PATH_LENGTH+10];
    char        modality[10];
    CBinfo      callbackInfo;   
    MC_STATUS   mcStatus;
    MC_COMMAND  command;
    int         messageID;
    char*       serviceName;
    char        prefix[30] = {0};
    int         fileNameCnt = 0;
    int         modifyDICOMDIR = 0;
    RESP_STATUS response = C_STORE_SUCCESS; 

    /*
     *  Need process ID number for messages
     */
#ifdef UNIX    
    sprintf(prefix, "Child PID %d:", getpid() );
#endif
   
    
    printf("\n\t%sAssociation Received.\n", prefix);
   
    mcStatus = MC_Accept_Association(*associationID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Error on MC_Accept_Association", mcStatus);
        MC_Abort_Association(associationID);
        return mcStatus;
    }
   
    for (;;)
    {
        mcStatus = MC_Read_Message(*associationID, 30, &messageID,
                                  &serviceName, &command);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            if (mcStatus == MC_ASSOCIATION_CLOSED)
            {       
                printf("\t%sAssociation Closed.\n", prefix);
                break;
            }
          
            if (mcStatus == MC_NETWORK_SHUT_DOWN
            ||  mcStatus == MC_ASSOCIATION_ABORTED
            ||  mcStatus == MC_INVALID_MESSAGE_RECEIVED
            ||  mcStatus == MC_CONFIG_INFO_ERROR)
            {
                PrintError("Unexpected event", mcStatus);
                break;
            }
            
            if (mcStatus == MC_TIMEOUT)
            {       
                printf("\t%sTimeout in MC_Read_Message, calling again.\n",
                       prefix);
                break;
            }
     
            PrintError("Error on MC_Read_Message", mcStatus);
            MC_Abort_Association(associationID);
            break;
        }
      
        /* use the modality for the filename */
        mcStatus = MC_Get_Value_To_String(messageID, MC_ATT_MODALITY,
                                        sizeof(modality), modality);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            strcpy(modality,"??");
        }
        
        /* add pathname here */
 
        fileNameCnt = 1;
        for (;;)
        {
            /*
             * Set up the filename for the actual path to put it in.
             */
            if (A_DDInfo->pathname[0])
            {
                sprintf(fullFname, "%s%c%.2s%06d", A_DDInfo->pathname,
                        DIRECTORY_SLASH_CHAR, modality, fileNameCnt);
            }
            else
            {
                sprintf(fullFname, "%.2s%06d", modality, fileNameCnt);
            }

            sprintf(fname, "%.2s%06d", modality, fileNameCnt);

            if (!FileExists(fullFname)) 
                break;
            fileNameCnt++;
        }


        mcStatus = AddMessageToDICOMDIR(messageID, serviceName, fname, A_DDInfo);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            /* Free the received message object */
            mcStatus = MC_Free_Message(&messageID);   
            if (mcStatus != MC_NORMAL_COMPLETION)
            {
                PrintError("Error on MC_Free_Message", mcStatus);
            }
            /* give a meaningful C_STORE_RSP message to the scu */
            response = C_STORE_FAILURE_PROCESSING_FAILURE;
        }
        else
        {
            modifyDICOMDIR = 1;
             
            if ( A_DDInfo->memory)
            {
                /* Free the received message object */
                mcStatus = MC_Free_Message(&messageID);   
                if (mcStatus != MC_NORMAL_COMPLETION)
                {
                    PrintError("Error on MC_Free_Message", mcStatus);
                    /* give a meaningful C_STORE_RSP message to the scu */
                    response = C_STORE_FAILURE_PROCESSING_FAILURE;
                }
            }
            else
            {
                mcStatus = MC_Message_To_File(messageID,fullFname);
                if (mcStatus != MC_NORMAL_COMPLETION)
                {
                    PrintError("Error on MC_Message_To_File", mcStatus);
                    /* give a meaningful C_STORE_RSP message to the scu */
                    response = C_STORE_FAILURE_PROCESSING_FAILURE;
                    modifyDICOMDIR = 0;
                    MC_Free_Message(&messageID);
                }
                else
                {
                    mcStatus = AddImageGroupTwoElements(messageID);
                    if (mcStatus != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on AddImageGroupTwoElements", mcStatus);
                        /* give a meaningful C_STORE_RSP message to the scu */
                        response = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
                        modifyDICOMDIR = 0;
                    }
                    else
                    {
                        printf("\t%sWriting %s image to file \"%s\".\n", prefix,
                                               modality, fname);
 
                        mcStatus = MC_Write_File(messageID, 2, &callbackInfo, 
                                               FileToMedia);        
                        if (mcStatus == MC_CALLBACK_CANNOT_COMPLY)
                        {
                            PrintError("Error on MC_Write_File", mcStatus);
                            /* give a meaningful C_STORE_RSP message to the scu */
                            response = C_STORE_FAILURE_REFUSED_NO_RESOURCES;
                            modifyDICOMDIR = 0;
                        }
                        else if (mcStatus == MC_INVALID_TRANSFER_SYNTAX)
                        {
                            PrintError("Error on MC_Write_File", mcStatus);
                            /* give a meaningful C_STORE_RSP message to the scu */
                            response = C_STORE_FAILURE_CANNOT_UNDERSTAND;
                            modifyDICOMDIR = 0;
                        }
                        else if (mcStatus != MC_NORMAL_COMPLETION)
                        {
                            PrintError("Error on MC_Write_File", mcStatus);
                            /* give a meaningful C_STORE_RSP message to the scu */
                            response = C_STORE_FAILURE_PROCESSING_FAILURE;
                            modifyDICOMDIR = 0;
                        }
                    }
                        
                    /* Free the received message object */
                    mcStatus = MC_Free_File(&messageID); 
                    if (mcStatus != MC_NORMAL_COMPLETION)
                    {
                        PrintError("Error on MC_Free_File", mcStatus);
                        /* give a meaningful C_STORE_RSP message to the scu */
                        if (response != C_STORE_SUCCESS)
                            response = C_STORE_FAILURE_PROCESSING_FAILURE;
                        modifyDICOMDIR = 0;
                    }
                }
            }
        } 
      
        /*
         *  Acquire a response message object
         */
        mcStatus = MC_Open_Message (&messageID, serviceName, C_STORE_RSP);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Open_Message error",mcStatus);
            MC_Abort_Association(associationID);
            return mcStatus;
        }
      
        mcStatus = MC_Send_Response_Message(*associationID, response,
                                           messageID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Send_Response_Message error", mcStatus);
            MC_Abort_Association(associationID);
            MC_Free_Message(&messageID);
            return mcStatus;
        }
      
        /* Free the response message object */ 
        mcStatus = MC_Free_Message(&messageID); 
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("MC_Free_Message error", mcStatus);
            MC_Abort_Association(associationID);
            return mcStatus;
        }
    }
    
    if (modifyDICOMDIR)
    {        
        mcStatus = MC_Write_File(A_DDInfo->dirID, 2, &callbackInfo, FileToMedia);
        if (mcStatus != MC_NORMAL_COMPLETION)
            PrintError("Error on MC_Write_File", mcStatus);
        return mcStatus;      
    }
    return MC_CANNOT_COMPLY;
} /* End Handle_Association() */



/****************************************************************************
 *
 *  Function    :   MediaToFile
 *
 *  Description :   Callback function used to read DICOM file from media
 *
 ****************************************************************************/
static MC_STATUS MediaToFile( char*       Afilename,
                              void*       AuserInfo,
                              int*        AdataSize,
                              void**      AdataBuffer,
                              int         AisFirst,
                              int*        AisLast)
{
    CBinfo*         callbackInfo = (CBinfo*)AuserInfo;   
    size_t          bytes_read;

    if (AisFirst)
    {
        callbackInfo->stream = fopen(Afilename, BINARY_READ);
    }    
    if (!callbackInfo->stream)
       return MC_CANNOT_COMPLY;

    bytes_read = fread (callbackInfo->buffer, 1, sizeof(callbackInfo->buffer),
                        callbackInfo->stream);
    if (ferror(callbackInfo->stream))
        return MC_CANNOT_COMPLY;

    if (feof(callbackInfo->stream))
    {
        *AisLast = 1;
        fclose(callbackInfo->stream);
    }
    else
        *AisLast = 0;
 
    *AdataBuffer = callbackInfo->buffer;
    *AdataSize = (int)bytes_read;
    return MC_NORMAL_COMPLETION;
    
} /* End MediaToFile() */


/****************************************************************************
 *
 *  Function    :   FileToMedia 
 *
 *  Description :   Callback function used to write DICOM file object to
 *                  media
 *
 ****************************************************************************/ 
static MC_STATUS FileToMedia(char*      Afilename,
                            void*       AuserInfo,
                            int         AdataSize,
                            void*       AdataBuffer,
                            int         AisFirst, 
                            int         AisLast)
{
    CBinfo*     callbackInfo = (CBinfo*)AuserInfo;   
    size_t      count;

    if (AisFirst)
    {    
        callbackInfo->stream = fopen(Afilename, BINARY_WRITE);
    }
    if (!callbackInfo->stream)
        return MC_CANNOT_COMPLY;
    
    count = fwrite(AdataBuffer, 1, AdataSize, callbackInfo->stream);
    if (count != (size_t)AdataSize)
        return MC_CANNOT_COMPLY;
        
    if (AisLast)
        fclose(callbackInfo->stream);
        
    return MC_NORMAL_COMPLETION;   
    
} /* End FileToMedia() */


#if defined(UNIX)

/****************************************************************************
 *
 *  Function    :   Start_Association_Process (UNIX)
 *
 *  Description :   Starts a process to wait for an association
 *
 ****************************************************************************/
static void Start_Association_Process( DicomDirInfo *A_DDInfo )
{
    int         child;
    pid_t       childs_pid;

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
           
            Wait_For_Association( A_DDInfo );
            printf("Child is EXITing\n");
             
            exit(EXIT_SUCCESS);    /* Children go away when they are done */
            break;
     
        case    -1:     /* error */
            printf("\tUnable to fork child process\n");
            printf("\t\tProgram aborted.\n");
            exit(EXIT_FAILURE);
     
        default :       /* parent */
            S_PInfo.child_save_PID = childs_pid;
            printf("\tChild PID %d: Starting process\n",childs_pid); 
            return;
    }

} /* End Start_Association_Process() */



/****************************************************************************
 *
 *  Function    :   shandler (UNIX)
 *
 *  Description :   The shandler function is used to handle SIGPIPE
 *                  interrupts which occur when a process continues to
 *                  send data on a connection which has closed.
 *
 ****************************************************************************/
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void shandler( int A_signo )
#else
static int shandler( int A_signo, long A_code,
                     struct sigcontext *A_scp)
#endif
{
    signal(SIGPIPE, (void (*)())shandler);
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
#else
    return -1;
#endif
} /* shandler() */


/****************************************************************************
 *
 *  Function    :   sigint_routine (UNIX)
 *
 *  Description :   This is our SIGINT routine (The user wants to shut down)
 *
 ****************************************************************************/
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void sigint_routine( int A_signo )
#else
static void sigint_routine  (int A_signo, long A_code,
                             struct sigcontext *A_scp)
#endif
{
    pid_t    pid;
    int      child;
    int      Return = EXIT_SUCCESS;
   
    pid = S_PInfo.child_PID;
    if (pid)
    {
        /*
         * Kill any lingering child processes.
         */
        (void)kill(pid, SIGKILL);
        Return = EXIT_FAILURE;
        fprintf(stderr, "KILLING CHILD PID (%d)\n", pid);
    }
    exit(Return);
} /* sigint_routine() */


/****************************************************************************
 *
 *  Function    :   reaper (UNIX)
 *
 *  Description :   This is our SIGCHLD routine. (One or more of our children
 *                  has changed state)
 *
 ****************************************************************************/
#if defined(SUN_SOLNAT) || defined(SUN_SOL8)
static void reaper( int A_signo )
#else
static void reaper ( int A_signo, long A_code,
                     struct sigcontext *A_scp)
#endif
{
    /* 
       Notice we RESET the signal handler before we exit
       this handler.  If we don't SYSTEM V will lose further
       signals.          
     */

    pid_t    pid;
    int      child;
    int      Return;
    int      isLast;
    int      newProcess = 0;
    MC_STATUS mcStatus;
    CBinfo   callbackInfo;
    
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("\tParent:Accpeting death of Child PID %d.\n", pid);

        newProcess = 1;

       if (pid == S_PInfo.child_PID)
        {
            /*
             * This does NOT kill the process; it only checks if
             * the process exists.
             */
            Return = kill(pid, 0);
            if (Return < 0 && errno == ESRCH)
            {
                S_PInfo.child_PID = 0;
            }
            signal(SIGCHLD, (void (*)())reaper);
            break;
        }
      
    }
    signal(SIGCHLD, (void (*)())reaper);


    /*
     * If the application is not being terminated, reload
     * the DICOMDIR here.
     */
    if (!S_PInfo.terminate)
    {
        printf("\tParent:Freeing DICOMDIR in memory.\n");
        (void) MC_Free_File(&S_DDInfo->dirID); 

        /*
         * Check if DICOMDIR exists, if so, load it in
         */
        mcStatus = MC_Create_File(&S_DDInfo->dirID, S_DDInfo->dirname,
                     "DICOMDIR", C_STORE_RQ);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to create file object",mcStatus);
            exit ( EXIT_FAILURE );
        }
    
        printf("\tParent:Reading DICOMDIR from media.\n");

        mcStatus = MC_Open_File(S_DDInfo->fsuappID, S_DDInfo->dirID,
                     &callbackInfo, MediaToFile);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to read file from media",mcStatus);
            exit ( EXIT_FAILURE );
        }
        
        mcStatus = MC_Dir_Root_Entity(S_DDInfo->dirID, &S_DDInfo->entityID, 
                      &S_DDInfo->recID, &S_DDInfo->type, &isLast);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            PrintError("Unable to get root entity of DICOMDIR",mcStatus);
            exit ( EXIT_FAILURE );
        }
        if (S_DDInfo->recID)
        {
            (void) MC_Get_Value_To_Int(S_DDInfo->recID, 
                      MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD, 
                      &S_DDInfo->nextRecID);
            (void) MC_Get_Value_To_Int(S_DDInfo->recID, 
                      MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY, 
                      &S_DDInfo->lowerRecID);
        }
        /*
         * Flag set above to start a new process
         */
        if (newProcess)
            Start_Association_Process(S_DDInfo);
    }    
} /* reaper() */
#endif  /* UNIX */

