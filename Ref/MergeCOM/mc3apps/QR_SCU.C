
/*************************************************************************
 *
 *       System: MergeCOM-3 Integrator's Tool Kit
 *
 *    $Workfile: qr_scu.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: 
 *               Q/R Sample Service Class User Application.
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

#include "mergecom.h" 
#include "mc3msg.h"
#include "mcstatus.h"
#include "diction.h"
#include "qr.h"


/*
 * Callback structure used to stream in and out messages
 */
typedef struct CALLBACKINFO
{
    FILE*   stream;
    int     messageID;
    char    prefix[30];
} CBinfo;


/* 
 * Command line and configuration options 
 */
typedef struct app_config
{
    int     timeOut;
    char    applicationTitle[AE_LEN];      /* my application title */
    int     applicationID;
    int     associationID;
    int     maxQueryResponses;
    char    remoteApplicationTitle[AE_LEN];/* title we are quering with */
    char    moveDestinationTitle[AE_LEN];  /* location to move image to */
    char    serviceList1[SERVICELIST_LEN];
    char    serviceList2[SERVICELIST_LEN];
    char    imageType[FILENAME_LEN];
} AppConfig;


/* 
 * Information returned structure 
 */
typedef struct ret_data
{
    char     model[SERVICENAME_LEN+1];
    char     level[CS_LEN+1];
    int      numData;
    PRPL     *patient_level;
    PRSTL    *study_level;
    PRSL     *series_level;
    PRIL     *image_level; 
} RetData;


/* 
 * Local Function prototypes 
 */
static void PrintUsage               ( void );
static QR_STATUS EditQuery           ( RetData*      A_root_level_info,
                                       RetData*      A_list,
                                       AppConfig*    A_myConfig );
static QR_STATUS CMOVEOption         ( AppConfig*    A_myConfig,
                                       RetData*      A_root_level_info );
static QR_STATUS CFINDOption         ( AppConfig*    A_myConfig,
                                       RetData*      A_root_level_info,
                                       RetData*      A_list );
static char      MainMenu            ( RetData*      A_data );
static char      NextMenu            ( RetData*      A_data,
                                       int           A_final );
static void      SetOptions          ( AppConfig*    A_myConfig );
static void      ShowOptions         ( AppConfig*    A_myConfig );
static void      ChooseModel         ( AppConfig*    A_myConfig,
                                       RetData*      A_data);
static int       ChangeAheadLevel    ( RetData*      A_data );
static void      ResetQueryData      ( RetData*      A_data,
                                       RetData*      A_list );
static int       GetLevelData        ( char*         A_level,
                                       RetData*      A_data );
static QR_STATUS SelectRecord        ( RetData*      A_data,
                                       RetData*      A_list );
static QR_STATUS SendCFINDMessage    ( RetData*      A_data,
                                       AppConfig*    A_myConfig,
                                       RetData*      A_list );
static QR_STATUS AddToList           ( char*         A_level,
                                       RetData*      A_list,
                                       RetData*      A_data );
static QR_STATUS EmptyList           ( RetData*      A_list );
static QR_STATUS SendCMOVEMessage    ( RetData*      A_root_data,
                                       AppConfig*    A_myConfig );
static QR_STATUS ProcessCSTOREAssociation
                                     ( char*         A_level,
                                       AppConfig*    A_myConfig );
static QR_STATUS CancelCMOVERQ       ( int*          A_associationID,
                                       char*         A_model );
static QR_STATUS CancelCFINDRQ       ( int*          A_associationID,
                                       char*         A_model );
static MC_STATUS MessageToFile       ( int           A_msgID,
                                       void*         A_CBinformation,
                                       int           A_dataSize,
                                       void*         A_dataBuffer,
                                       int           A_isFirst,
                                       int           A_isLast );
static QR_STATUS GetOptions          ( int           argc,
                                       char**        argv,
                                       AppConfig*    A_myConfig );
static QR_STATUS SetProgramDefaults  ( AppConfig*    A_myConfig,
                                       RetData*      A_root_level_info );
static QR_STATUS BuildCFINDMessage   ( RetData*      A_data, 
                                       int           A_messageid );
static QR_STATUS BuildCMOVEMessage   ( RetData*      A_data,
                                       int           A_messageid,
                                       AppConfig*    A_myConfig,
                                       char*         A_moveLevel );
static QR_STATUS ReadCFINDMessage    ( RetData*      A_data,
                                       RetData*      A_root_level_info,
                                       int           A_messageid,
                                       AppConfig*    A_myConfig );
static QR_STATUS SetValue            ( int           A_messageid, 
                                       unsigned long A_tag,
                                       char*         A_value,
                                       char*         A_default,
                                       int           A_required );
static QR_STATUS GetValue            ( int           A_messageid,
                                       unsigned long A_tag,
                                       char*         A_value,
                                       int           A_size, 
                                       char*         A_default );
static void      PrintCFINDResults   ( RetData*      A_list );
static short     OktoMove            ( RetData*      A_data );



/****************************************************************************
 *
 * NAME
 *    main - Query/Retrieve user main program
 *
 * SYNOPSIS
 *    qr_scu REMOTE_APPLICATION_TITLE [options]
 *
 ****************************************************************************/
#ifdef VXWORKS
int  qrscu(int argc, char** argv);
int qrscu(int argc, char** argv)
#else
int main(int argc, char** argv);
int main(int argc, char** argv)
#endif
{
    int           quit = FALSE;
    RetData       root_level_info;
    AppConfig     myConfig;
    RetData       list;
    MC_STATUS     status;
    QR_STATUS     qrStatus;
    PRPL          patient_level;
    PRSTL         study_level;
    PRSL          series_level;
    PRIL          image_level;
 
#if defined(_MACINTOSH) && defined(__MWERKS__)
    SIOUXSettings.initializeTB = true;
    SIOUXSettings.standalone = true;
    SIOUXSettings.setupmenus = true;
    SIOUXSettings.autocloseonquit = false;
    SIOUXSettings.asktosaveonclose = true;
    SIOUXSettings.showstatusline = true;
    argc = ccommand(&argv);
#endif 

    root_level_info.patient_level = &patient_level;
    root_level_info.study_level = &study_level;
    root_level_info.series_level = &series_level;
    root_level_info.image_level = &image_level;
    list.numData = 0;
    list.patient_level = NULL;

    printf ( "\n" );
    printf ( "QR_SCU - Query/Retrieve Service Class User\n" );
	printf ( "(c) 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)\n");
	printf ( "All rights reserved\n\n");


    /*
     * Set the defaults configuration values for the application and
     * parse the command line options 
     */
    memset((void*)&myConfig, 0, sizeof(myConfig));
    qrStatus = SetProgramDefaults ( &myConfig, &root_level_info );
    if ( qrStatus == QR_FAILURE )
        return ( EXIT_FAILURE );
    qrStatus = GetOptions ( argc, argv, &myConfig );
    if ( qrStatus == QR_FAILURE )
        return ( EXIT_FAILURE );

    /*
     * This is for the menu system, other menus depend on where you came from 
     */
    while ( quit != TRUE )
    {
        switch ( MainMenu( &root_level_info ) )
        {
            case '1':
                if (EditQuery(&root_level_info, &list, &myConfig) == QR_FAILURE)
                    printf("ERROR: editing query\n");
                break;

            case '2':
                ChooseModel( &myConfig, &root_level_info );
                break;

            case '3':
                ShowOptions ( &myConfig );
                break;

            case '4':
                printf("\nSelect an option from this menu\n");
                break;

            case 'q':
            case 'Q':
            case 'x':
            case 'X':
            case '5':
                quit = TRUE;
                break;

            default:
                continue;

        } /* End of switch */
    } /* End of while */


    /* 
     * We're done.  Close everything gracefully and clean up 
     */
    status = MC_Release_Application ( &myConfig.applicationID );
    if ( status != MC_NORMAL_COMPLETION )
        PrintErrorMessage ( "main", "MC_Release_Application", status, NULL );
    
    status = MC_Library_Release ( );
    if ( status != MC_NORMAL_COMPLETION )
        PrintErrorMessage ( "main", "MC_Library_Release", status, NULL );
        
    (void)EmptyList ( &list ); 
    return ( EXIT_SUCCESS );

} /* main() */



/*****************************************************************************
 *
 * NAME
 *    PrintUsage - Prints out the command line options.
 *
 * ARGUMENTS
 *    none.
 *
 * DESCRIPTION
 *    Prints out the options for this program.
 *
 * RETURNS
 *    none.
 *
 * SEE ALSO
 *    none.
 *
 ***************************************************************************/
static void PrintUsage(void)
{
    printf("\n" );
    printf("   <remote_ae_title> The remote application entity to connect\n");
    printf("\n" );
    printf("Options:\n" );
    printf("   -h                Print this help page\n" );
    printf("   -a <ae_title>     Local application entity\n" );
    printf("   -d <dest>         Store destination\n" );
    printf("   -o <timeout>      Time out value\n" );
    printf("   -t <type>         Image Type\n" );
    printf("   -1 <slist>        Storage Service List 1\n" );
    printf("   -2 <slist>        Q/R Service List 2\n" );
    printf("\n" );

} /* PrintUsage() */




/*****************************************************************************
 *
 * NAME
 *    GetOptions - Get the command line options.
 *
 * ARGUMENTS
 *    Aargc         int            Number of command line arguments
 *    Aargv         char **        The command line.
 *    A_myConfig    AppConfig *    Config parameters for AE
 *
 * DESCRIPTION
 *    GetOptions is the routine that parses the command line and sets the
 *    variables associated with each parameter that are contained in the
 *    A_myConfig structure.
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    PrintErrorMessage
 *
 ****************************************************************************/
static QR_STATUS GetOptions ( int Aargc, char **Aargv, AppConfig *A_myConfig )
{
    int          i;
    MC_STATUS    status;



    if ( Aargc < 2 )
    {
        printf("ERROR: Wrong number of arguments specified\n");
        printf( "Usage: %s <remote_ae_title> [options]\n", Aargv[0] );
        PrintUsage();
        return QR_FAILURE;
    }

    for (i=1; i<Aargc; i++)
    {
        if (Aargv[i][0] == '-' && (i+1) < Aargc)
        {
            switch(Aargv[i][1])
            {
                case 'a':
                    i++;
                    strcpy(A_myConfig->applicationTitle, &(Aargv[i][0]));
                    strcpy(A_myConfig->moveDestinationTitle, &(Aargv[i][0])); 
                break;
 
                case 'd':
                    strcpy(A_myConfig->moveDestinationTitle, &(Aargv[++i][0]));
                break;
 
                case 'o':
                    A_myConfig->timeOut = atoi(&(Aargv[++i][0]));
                break;
 
                case 't':
                    strcpy(A_myConfig->imageType, &(Aargv[++i][0]));
                break;
 
                case '1':
                    strcpy(A_myConfig->serviceList1, &(Aargv[++i][0]));
                break;
 
                case '2':
                    strcpy(A_myConfig->serviceList2, &(Aargv[++i][0]));
                break;
 
                default:
                    printf("ERROR: Invalid argument specified\n");
                    printf("Usage: %s <remote_ae_title> [options]\n", Aargv[0]);
                    PrintUsage();
                    return QR_FAILURE;

            }
        }
        else if (Aargv[i][0] != '-')
            strcpy(A_myConfig->remoteApplicationTitle, &(Aargv[i][0]));
        else
        {
            printf("ERROR: Wrong number of arguments specified\n");
            printf("Usage: %s <remote_ae_title> [options]\n", Aargv[0]);
            PrintUsage();
            return QR_FAILURE;
        }
    }

    if (A_myConfig->remoteApplicationTitle[0] == '\0')
    {
        printf("ERROR: Remote AE not specified\n");
        printf("Usage: %s <remote_ae_title> [options]\n", Aargv[0]);
        PrintUsage();
        return QR_FAILURE;
    }
 
    /*
     * This call MUST be the first call made to the library!!! 
     */
    status = MC_Library_Initialization ( NULL, NULL, NULL );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintErrorMessage (A_myConfig->applicationTitle, 
                           "Unable to initialize library", status,NULL);
        return QR_FAILURE;
    }

    /*
     * Register this DICOM application 
     */
    status = MC_Register_Application ( &(A_myConfig->applicationID),
        A_myConfig->applicationTitle );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( A_myConfig->applicationTitle, 
            "MC_Register_Application", status, NULL );
        return QR_FAILURE;
    }
    return QR_SUCCESS;
} /* GetOptions() */



/*****************************************************************************
 *
 * NAME
 *    SetProgramDefaults - Initializes the global variables.
 *
 * ARGUMENTS
 *    A_myConfig           AppConfig *    Config struct for defaults    
 *    A_root_level_info    RetData *      Query level information
 *
 * DESCRIPTION
 *    This routine initializes the global variables to their default values.
 *    These variables are contained in the A_myConfig structure.
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static QR_STATUS SetProgramDefaults ( AppConfig *A_myConfig, 
                                      RetData *A_root_level_info )
{
    A_myConfig->timeOut = 3000;
    strcpy( A_myConfig->applicationTitle, "MERGE_QR_SCU" );
    strcpy( A_myConfig->serviceList1, "Storage_SCP_Service_List" );
    strcpy( A_myConfig->serviceList2, "Query_SCU_Service_List" );
    strcpy( A_myConfig->imageType, "CT" );
    A_myConfig->maxQueryResponses = 100;

    A_myConfig->remoteApplicationTitle[0] = '\0'; /* required parameter */
    strcpy( A_myConfig->moveDestinationTitle, A_myConfig->applicationTitle ); 

    memset((void*)(A_root_level_info->patient_level), 0, sizeof(PRPL));
    memset((void*)(A_root_level_info->study_level), 0, sizeof(PRSTL));
    memset((void*)(A_root_level_info->series_level), 0, sizeof(PRSL));
    memset((void*)(A_root_level_info->image_level), 0, sizeof(PRIL));

    strcpy(A_root_level_info->model, PATIENT_STUDY_ONLY_MODEL);
    strcpy(A_root_level_info->level, PATIENT_LEVEL);

    return ( QR_SUCCESS );
} /* SetProgramDefaults() */




/*****************************************************************************
 *
 * NAME
 *    MainMenu - Shows the Main Menu options
 *
 * ARGUMENTS
 *    A_data    RetData *         A pointer to data
 *
 * DESCRIPTION
 *    Displays the main menu and waits for the user to select an option
 * 
 * RETURNS
 *    The option selected by the user.
 *
 * SEE ALSO
 *
 ****************************************************************************/
static char MainMenu ( RetData *A_data )
{
    char    user_input[USER_INPUT_LEN];
 
    printf ( "\n" );
    printf ( "-------------------------------| MAIN |---------------"
        "------------------\n" );
    printf ( "\n" );

    printf ( "[1] Begin [%s] Query\n", A_data->model );
    printf ( "[2] Choose Model [%s]\n", A_data->model );
    printf ( "[3] Show Options\n" );
    printf ( "[4] Instructions\n");
    printf ( "[5] Exit\n" );

    printf ( "\n" );
    printf ( "==> " );
    fflush ( stdout );
    fgets ( user_input, sizeof(user_input), stdin );
    fflush ( stdout );

    return ( user_input[0] );
} /* MainMenu() */



/*****************************************************************************
 *
 * NAME
 *    SetOptions - UI to set command line options.
 *
 * ARGUMENTS
 *    A_myConfig    AppConfig *    Program configuration parameters
 *
 * DESCRIPTION
 *    This routine is used by the user to set any command line options
 *    which they may have forgotten to set or change command line options.
 *
 * RETURNS
 *    none.
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static void SetOptions ( AppConfig *A_myConfig )
{
    int           done = FALSE;
    char          user_input[USER_INPUT_LEN];
    char          argument[USER_INPUT_LEN];
    int           i;
    MC_STATUS     status;


    while ( done != TRUE )
    {
        printf("\n" );
        printf("------------------------------| Set Options |--------"
            "-------------------\n" );
        printf("\n" );
        printf("[1] Local Application Title:  = [%s]\n",
            A_myConfig->applicationTitle );
        printf("[2] Service List 1:           = [%s]\n",
            A_myConfig->serviceList1 );
        printf("[3] Service List 2:           = [%s]\n",
            A_myConfig->serviceList2 );
        printf("[4] Image Type:               = [%s]\n",
            A_myConfig->imageType );
        printf("[5] Time Out:                 = [%d]\n", A_myConfig->timeOut );
        printf("[6] Move Destination:         = [%s]\n",
            A_myConfig->moveDestinationTitle );
        printf("[7] Remote Application Title: = [%s]\n",
            A_myConfig->remoteApplicationTitle );
        printf("[8] Maximum Query Responses:  = [%d]\n",
            A_myConfig->maxQueryResponses );
        printf("\n" );
        printf("[R] Return to main\n" );
        printf("\n" );
        printf("Enter one of the above numbers followed by the required\n" );
        printf("input for that field.  For example, if you would like to\n" );
        printf("set the timeout, enter the following:\n" );
        printf("\n" );
        printf("   \"1 3000\".\n" );
        printf("\n\n" );
        printf("EDIT> " );
        fflush(stdout );
        fgets(user_input, sizeof(user_input), stdin );
        fflush(stdout );

        /*
         * Clear the newline characters from the end of the input data so
         * the checks below work properly.
         */
        for (i = strlen(user_input)-1;
             i >= 0 && 
               (user_input[i]==' '||user_input[i]=='\r'||user_input[i]=='\n');
             i--)
            user_input[i] = '\0';


        if ( user_input[2] != '\0' )
            strcpy ( argument, &user_input[2] );

        switch ( user_input[0] )
        {
            case '1':
                status = MC_Release_Application( &(A_myConfig)->applicationID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    PrintErrorMessage ( "main", "MC_Release_Application",
                       status, NULL );
                    return;
                }

                /*
                 * This is the new application title
                 */
                strcpy( A_myConfig->applicationTitle, argument );

                /*
                 * Register this DICOM application
                 */
                status = MC_Register_Application ( &(A_myConfig->applicationID),
                    A_myConfig->applicationTitle );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    PrintErrorMessage ( A_myConfig->applicationTitle,
                        "MC_Register_Application", status, NULL );
                    return;
                }
                break;

            case '2':
                strcpy ( A_myConfig->serviceList1, argument );
                break;

            case '3':
                strcpy ( A_myConfig->serviceList2, argument );
                break;

            case '4':
                strcpy ( A_myConfig->imageType, argument );
                break;

            case '5':
                A_myConfig->timeOut = atoi ( argument );
                break;

            case '6':
                strcpy ( A_myConfig->moveDestinationTitle, argument );
                break;

            case '7':
                strcpy ( A_myConfig->remoteApplicationTitle, argument );
                break;

            case '8':
                A_myConfig->maxQueryResponses = atoi (argument);
                break;

            case 'r':
            case 'R':
                done = TRUE;
                break;

            default:
                continue;

        } /* end of switch */
    } /* end of while */

    return;
} /* SetOptions() */


/*****************************************************************************
 *
 * NAME                 
 *    ShowOptions - Displays the command line options.
 *
 * ARGUMENTS
 *    A_myConfig    AppConfig *    Configuration parameters for program
 *
 * DESCRIPTION
 *    This routine displays the command line options for the user.
 *
 * RETURNS
 *    none.
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static void ShowOptions ( AppConfig *A_myConfig )
{
    char         user_input[USER_INPUT_LEN];

    printf("\n" );
    printf("-----------------------------| Show Options |------"
        "---------------------\n" );
    printf("\n" );
    printf("Options:\n" );
    printf("   Local Application Title:  %s\n", A_myConfig->applicationTitle );
    printf("   Service List 1:           %s\n", A_myConfig->serviceList1 );
    printf("   Service List 2:           %s\n", A_myConfig->serviceList2 );
    printf("   Image Type:               %s\n", A_myConfig->imageType );
    printf("   Time Out:                 %d\n", A_myConfig->timeOut );
    printf("   Move Destination:         %s\n",
        A_myConfig->moveDestinationTitle );
    printf("   Remote Application Title: %s\n",
        A_myConfig->remoteApplicationTitle );
    printf("   Maximum Query Responses:  %d\n", A_myConfig->maxQueryResponses );

    printf("\n\nDo you wish to change any options?\n\n");
    printf("==> ");
    fflush ( stdout );
    fgets ( user_input, sizeof(user_input), stdin );
    fflush ( stdout );

    if (user_input[0] == 'Y' || user_input[0] == 'y')
        SetOptions( A_myConfig );

    return;
} /* ShowOptions() */



/*****************************************************************************
 *
 * NAME
 *    ChooseModel - Allows user to choose model 
 *
 * ARGUMENTS
 *    A_myConfig    AppConfig *    Configuration params for the program
 *    A_data        RetData *      Pointer to model information
 *
 * DESCRIPTION
 *    This function allows the user to choose the query retrieve model to 
 *    be used.
 *
 * RETURNS
 *    none.
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static void  ChooseModel( AppConfig *A_myConfig, RetData *A_data)
{
    static char    S_prefix[] = "ChooseModel";
    MC_STATUS      status;
    char           user_input[USER_INPUT_LEN];
    int            patientStudyOnly = FALSE;
    int            patientRoot = FALSE;
    int            studyRoot = FALSE;
    ServiceInfo    serviceInfo;
    
    
    printf("Opening association to determine which services %s supports...\n",
        A_myConfig->remoteApplicationTitle);

    status = MC_Open_Association ( A_myConfig->applicationID,
                                   &(A_myConfig->associationID), 
                                   A_myConfig->remoteApplicationTitle,
                                   NULL, NULL, NULL);
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Association", status, NULL );
        return; 
    }

    status = MC_Get_First_Acceptable_Service(A_myConfig->associationID, 
        &serviceInfo); 
    if (status == MC_NORMAL_COMPLETION)
    {
        while (status == MC_NORMAL_COMPLETION)
        {
            if (!strcmp(serviceInfo.ServiceName, "PATIENT_STUDY_ONLY_QR_FIND"))
                patientStudyOnly = TRUE;
            else if (!strcmp(serviceInfo.ServiceName, "PATIENT_ROOT_QR_FIND"))
                patientRoot = TRUE;
            else if (!strcmp(serviceInfo.ServiceName, "STUDY_ROOT_QR_FIND"))
                studyRoot = TRUE;

            status = MC_Get_Next_Acceptable_Service(A_myConfig->associationID,
                &serviceInfo);
        }

    }
    else if( status == MC_END_OF_LIST )
    {
        PrintErrorMessage(S_prefix,"MC_Get_First_Acceptable_Service"
           " unnexpectedly returned", status, NULL);
        MC_Abort_Association(&(A_myConfig->associationID));
        return;
    }
    else
    {
        PrintErrorMessage(S_prefix,"MC_Get_First_Acceptable_Service failed",
            status, NULL);
        MC_Abort_Association(&(A_myConfig->associationID));
        return;
    } 

    status = MC_Close_Association ( &(A_myConfig->associationID) );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintErrorMessage ( S_prefix, "MC_Close_Association",
            status, NULL );
        return;
    }

    printf ( "\n" );
    printf ( "-----------| Models supported by %s |------------\n", 
             A_myConfig->remoteApplicationTitle);
    printf ( "\n" );
    if (patientRoot)
        printf ( "[P] Patient Root\n");
    if (studyRoot)
        printf ( "[S] Study Root\n");
    if (patientStudyOnly)
        printf ( "[O] Patient/Study Only\n");

    printf ( "\n" );

    printf ( "==> " );
    fflush ( stdout );
    fgets ( user_input, sizeof(user_input), stdin );
    fflush ( stdout );

    switch (user_input[0])
    {
        case 'P':
        case 'p':
            if (patientRoot)
            {
                strcpy(A_data->model, PATIENT_MODEL);
                strcpy(A_data->level, PATIENT_LEVEL);
            }
            break;
        case 'S':
        case 's':
            if (studyRoot)
            {
                strcpy(A_data->model, STUDY_MODEL);
                strcpy(A_data->level, STUDY_LEVEL);
            }
            break;
        case 'O':
        case 'o':
            if (patientStudyOnly)
            {
                strcpy(A_data->model, PATIENT_STUDY_ONLY_MODEL);
                strcpy(A_data->level, PATIENT_LEVEL);
            }
            break;
        default:
            printf("Invalid option: %c",user_input[0]);
            break;
    }

    return;
} /* ChooseModel() */



/*****************************************************************************
 *
 * NAME
 *    EditQuery - Allows the user to filter throught the matching responses
 *                returned from a C-FIND-RQ query.
 *
 * ARGUMENTS
 *    A_root_level_info    RetData *      Current Query information
 *    A_list               RetData *      List of data returned from the Query
 *    A_myConfig           AppConfig *    Program Config values
 *
 * DESCRIPTION
 *    This is the main control routine used for conducting a query.  It it
 *    used when conducting querys for each of the models.  It will traverse
 *    through the various levels for a query and allow the user to do a move
 *    request at each of the levels.  It is meant to be organized in such a 
 *    way that a GUI interface can be used in place of this function with
 *    the remainder of the functionality still working.
 *
 * RETURNS
 *    QR_STATUS according to appropriate errors
 *
 * SEE ALSO
 *    GetLevelData
 *    ResetQueryData
 *    CFINDOption
 *    NextMenu
 *    PrintCFINDResults
 *    SelectRecord
 *    ChangeAheadLevel    
 *
 ****************************************************************************/
static QR_STATUS EditQuery (RetData*     A_root_level_info, 
                            RetData*     A_list,
                            AppConfig*   A_myConfig )
{
    QR_STATUS qrStatus;
    int       done = FALSE;
    time_t    start_time;
    int       value;

    /*
     * Get input from the user to modify the initial query
     */
    if (GetLevelData (A_root_level_info->level,A_root_level_info))
    {
        ResetQueryData ( A_root_level_info, A_list );
        return ( QR_SUCCESS );
    }
    time(&start_time);

    /*
     * Send the C-FIND Message to see if there are any matches to
     * the intial input.
     */
    qrStatus = CFINDOption( A_myConfig, A_root_level_info, A_list);
    if (qrStatus == QR_FAILURE )
        return ( qrStatus );

    printf("\nElapsed time of Find Operation: %ld seconds\n",
        time((time_t *)NULL) - start_time);

    while ( done != TRUE )
    {
        /*
         * NextMenu displays the appropriate menu based on
         * our level and find status.
         */
        value = NextMenu (A_root_level_info, FALSE);
        if ( value == '1' )
        {
            /*
             * Select a record you want to continue to query on 
             */
            if ( A_list->numData == 0 )
            {
                printf("There are no records available to select.\n");
            }
            else
            {
                if (!strncmp (A_root_level_info->level, A_list->level, 
                    sizeof(A_list->level)))
                {
                    PrintCFINDResults ( A_list );

                    qrStatus =  SelectRecord ( A_root_level_info,  A_list );
                    
                    /*
                     * If we selected a record we will give a menu 
                     *  which will ask if we want to move the record 
                     */
                    if (qrStatus == QR_SUCCESS )
                    {
                        value = NextMenu(A_root_level_info, TRUE);
                        if ( value == '1' )
                        {
                            /*
                             * We chose to do a move request
                             */
                            qrStatus = CMOVEOption( A_myConfig,
                                A_root_level_info );
                            if (qrStatus != QR_SUCCESS )
                            {
                                ResetQueryData (A_root_level_info, A_list);
                                return ( qrStatus );
                            }
                        }
                        else if ( value != 'q' && value != 'Q' )
                        {
                            /*
                             * We choose to go to the next level so we check to
                             *  see if there is a next level to go to
                             */
                            if ( ChangeAheadLevel ( A_root_level_info ))
                            {
                                time(&start_time);

                                /*
                                 * Send the C-FIND Message
                                 */
                                qrStatus = CFINDOption( A_myConfig,
                                    A_root_level_info,  A_list);
                                if (qrStatus == QR_FAILURE )
                                    return ( qrStatus );

                                printf("\nElapsed time of Find Operation: %ld"
                                    " seconds\n", 
                                    time((time_t *)NULL) - start_time);
                                continue;
                            }
                        }

                        /*
                         * There was no level to go to, so we are going to start
                         *  over again, from the beginning
                         */
                        ResetQueryData (A_root_level_info, A_list);
                        done = TRUE;
                    }
                }
                else
                    printf("There are none available.\n");
            }
        }
        /* 
         * This was a choice to quit, so we do
         */
        else if ( value == 'q' || value == 'Q' )
        {
            ResetQueryData ( A_root_level_info, A_list );
            done = TRUE;
        }
    } /* while */

    return ( QR_SUCCESS );
} /* EditQuery() */



/*****************************************************************************
 *
 * NAME
 *    NextMenu - Shows the Next Menu options
 *
 * ARGUMENTS
 *    A_data     RetData *   A pointer to data
 *    A_final    int         A TRUE or FALSE value indicating whether the 
 *                            final menu or not
 *
 * DESCRIPTION
 *    Gives user main menu.
 *
 * RETURNS
 *    Option selected via user input
 *
 * SEE ALSO
 *
 ****************************************************************************/
static char NextMenu ( RetData *A_data, int A_final )
{
    char    user_input[USER_INPUT_LEN];

    printf ( "\n" );
    printf("-----------------------------------------------------"
        "-------------------\n");
    printf ( "                           Model:  %s\n",  A_data->model );
    printf ( "                           Level:  %s\n", A_data->level );
    printf("-----------------------------------------------------"
        "-------------------\n");

    if ( !strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL)-1) )
    {
        if (A_data->patient_level->retrieveAETitle[0] == '\0')
        {
            printf("%60.80s\n", "OFFLINE"); 
            printf("%50.59s%20.20s\n", "File ID:",
                A_data->patient_level->med_file_id);
            printf("%50.59s%20.20s\n", "File UID:",
                A_data->patient_level->med_file_uid);
        }
        else
            printf("%60.80s\n", "ONLINE"); 
    }
    else if ( !strncmp(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL)-1))
    {
        if (A_data->study_level->retrieveAETitle[0] == '\0')
        {
            printf("%60.80s\n", "OFFLINE"); 
            printf("%50.59s%20.20s\n", "File ID:",
                A_data->study_level->med_file_id);
            printf("%50.59s%20.20s\n", "File UID:",
                A_data->study_level->med_file_uid);
        }
        else
            printf("%60.80s\n", "ONLINE"); 
    }
    else if ( !strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL)-1))
    {
        if (A_data->series_level->retrieveAETitle[0] == '\0')
        {
            printf("%60.80s\n", "OFFLINE"); 
            printf("%50.59s%20.20s\n", "File ID:",
                A_data->series_level->med_file_id);
            printf("%50.59s%20.20s\n", "File UID:",
                A_data->series_level->med_file_uid);
        }
        else
            printf("%60.80s\n", "ONLINE"); 
    }
    else if ( !strncmp(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL)-1) )
    {
        if (A_data->image_level->retrieveAETitle[0] == '\0')
        {
            printf("%60.80s\n", "OFFLINE"); 
            printf("%50.59s%20.20s\n", "File ID:",
                A_data->image_level->med_file_id);
            printf("%50.59s%20.20s\n", "File UID:",
                A_data->image_level->med_file_uid);
        }
        else
            printf("%60.80s\n", "ONLINE"); 
    }

    if ( !strncmp ( A_data->model, STUDY_MODEL, sizeof( STUDY_MODEL ) - 1 ) )
    {
        printf("Patient's Name      = [%s]\n", 
            A_data->study_level->patient_name);
        printf("Patient ID          = [%s]\n", 
            A_data->study_level->patient_id);
    }
    else 
    {
        printf("Patient's Name      = [%s]\n", 
            A_data->patient_level->patient_name);
        printf("Patient ID          = [%s]\n", 
            A_data->patient_level->patient_id);
    }

    printf("Study Instance UID  = [%s]\n",
        A_data->study_level->study_inst_uid);
    printf("Series Instance UID = [%s]\n",
        A_data->series_level->series_inst_uid);
    printf("SOP Instance UID    = [%s]\n", A_data->image_level->sop_inst_uid);
    printf ( "\n" );

    if ( !A_final )
    {
        printf ( "[1] SELECT a [%s] from the query result.\n",
            strtok ( A_data->level, "_" ) );
    }
    else
    {
       printf ( "[1] MOVE the [%s].\n", strtok ( A_data->level, "_") );
       printf ( "[2] Continue with the query.\n");
    }

    printf ( "[Q] Quit this Query.\n" );
    printf ( "\n" );
    printf ( "QUERY> " );
    fflush ( stdout );
    fgets ( user_input, sizeof(user_input), stdin );
    fflush ( stdout );

    return ( user_input[0] );
} /* NextMenu() */




/*****************************************************************************
 *
 * NAME
 *    ChangeAheadLevel - Changes to next level in Query 
 *
 * ARGUMENTS
 *    A_data    RetData *    Data need to determine the next level to go to 
 *
 * DESCRIPTION
 *    Changes the LEVEL in the root_level_info structure to some safe value
 *    based on the model for the current query.
 *
 * RETURNS
 *   TRUE or FALSE as to whether or not there is another level
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static int ChangeAheadLevel(RetData *A_data)
{
    if(!strncmp(A_data->model, PATIENT_MODEL, sizeof(PATIENT_MODEL) - 1))
    {
        if(!strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
            strncpy(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL));
        else if(!strncmp(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL) - 1)) 
            strncpy(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL));
        else if(!strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
            strncpy(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL));
        else
            return ( FALSE );
    }
    else if(!strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
    {
        if(!strncmp(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL) - 1))
            strncpy(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL));
        else if(!strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
            strncpy(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL));
        else
            return ( FALSE );
    }
    else /* patient/study only */
    {
        if(!strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
            strncpy(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL)); 
        else
           return ( FALSE );
    }
    return ( TRUE );
} /* ChangeAheadLevel() */


/*****************************************************************************
 *
 * NAME
 *    ResetQueryData - Changes to next level in Query
 *
 * ARGUMENTS
 *    A_data    RetData *      Current Query information
 *    A_list    RetData *      Data returned from the Query
 *
 * DESCRIPTION
 *    Takes you back to the beginnning clearing out the list of query 
 *    results and zeroing out the root_level_structure   
 *
 * RETURNS
 *    nothing.
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static void ResetQueryData( RetData *A_data, RetData *A_list )
{
    if (EmptyList ( A_list ) != QR_SUCCESS)
        printf("ERROR: freeing query list\n");

    memset((void*)(A_data->patient_level), 0, sizeof(PRPL));
    memset((void*)(A_data->study_level), 0, sizeof(PRSTL));
    memset((void*)(A_data->series_level), 0, sizeof(PRSL));
    memset((void*)(A_data->image_level), 0, sizeof(PRIL));

    if(!strncmp(A_data->model, PATIENT_MODEL, sizeof(PATIENT_MODEL) - 1))
    {
        strncpy(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL));
    }
    else if(!strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
    {
        strncpy(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL));
    }
    else if(!strncmp(A_data->model, PATIENT_STUDY_ONLY_MODEL,
        sizeof(PATIENT_STUDY_ONLY_MODEL)-1)) 
    {
        strncpy(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL));
    }
} /* ResetQueryData() */



/*****************************************************************************
 *
 * NAME
 *    GetLevelData - Allows user to enter level data 
 *
 * ARGUMENTS
 *    A_level    char *       String containing the level 
 *    A_list     RetData *    Data returned from the Query
 *
 * DESCRIPTION
 *    This function allows the user to enter data for a level
 *
 * RETURNS
 *    none
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static int GetLevelData(char *A_level, RetData *A_data)
{
    char user_input[USER_INPUT_LEN];
    int choice = 0;
    int i;

    do
    {
        printf ( "\n" );

        if(!strncmp(A_level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
        {
            printf (
                "****************| Patient Level Editing |****************\n");
            printf ( "\n" );

            printf ( "[1] Patient ID          = [%s]\n", 
                A_data->patient_level->patient_id);
            printf ( "[2] Patient Name        = [%s]\n", 
                A_data->patient_level->patient_name);
            choice = 0;
        }
        else if(!strncmp(A_level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
        {
            printf (
                "****************| Series Level Editing |*****************\n");
            printf ( "\n" );

            printf ( "[1] Modality            = [%s]\n",
                A_data->series_level->modality);
            printf ( "[2] Series Number       = [%s]\n", 
                A_data->series_level->series_num);
            printf ( "[3] Series Instance UID = [%s]\n", 
                A_data->series_level->series_inst_uid);
            printf ( "[4] Study Instance UID  = [%s]\n",
                A_data->study_level->study_inst_uid);
            if (strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL)))
                printf ( "[5] Patient ID          = [%s]\n", 
                    A_data->patient_level->patient_id);
            choice = 1;
        }
        else if(!strncmp(A_level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL) - 1))
        {
            printf (
                "****************| Image Level Editing |******************\n");
            printf ( "\n" );

            printf ( "[1] Image Number        = [%s]\n", 
                A_data->image_level->image_num);
            printf ( "[2] SOP Instance UID    = [%s]\n", 
                A_data->image_level->sop_inst_uid);
            printf ( "[3] Study Instance UID  = [%s]\n", 
                A_data->study_level->study_inst_uid);
            printf ( "[4] Series Instance UID = [%s]\n", 
                A_data->series_level->series_inst_uid);
            if (strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL)))
                printf ( "[5] Patient ID          = [%s]\n", 
                    A_data->patient_level->patient_id);
            choice = 2;
        }
        else if(!strncmp(A_level, STUDY_LEVEL, sizeof(STUDY_LEVEL) - 1))
        {
            printf ( 
                "*****************| Study Level Editing |*****************\n");
            printf ( "\n" );

            printf ( "[1] Study Date          = [%s]\n", 
                A_data->study_level->study_date);
            printf ( "[2] Study Time          = [%s]\n", 
                A_data->study_level->study_time);
            printf ( "[3] Accession Number    = [%s]\n", 
                A_data->study_level->accession_num);
            printf ( "[4] Study ID            = [%s]\n", 
                A_data->study_level->study_id);
            printf ( "[5] Study Instance UID  = [%s]\n", 
                A_data->study_level->study_inst_uid);
            printf ( "[6] Patient ID          = [%s]\n", 
                A_data->patient_level->patient_id);
            if (!strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL)))  
                printf ( "[7] Patient Name        = [%s]\n", 
                    A_data->patient_level->patient_name);
            choice = 3;
        }

        printf( "[D] Done Editing\n" );
        printf( "[Q] Quit this Query\n" );

        printf ( "EDIT> " );
        fflush ( stdout );
        fgets ( user_input, sizeof(user_input), stdin );
        fflush ( stdout );

        /*
         * Clear the newline characters from the end of the input data so
         * the checks below work properly.
         */
        for (i = strlen(user_input)-1; i >= 0 && 
            (user_input[i]==' '||user_input[i]=='\r'||user_input[i]=='\n');
            i--)
            user_input[i] = '\0';

        switch(user_input[0])
        {
            case '1':
                switch (choice)
                {
                    case 0:
                        strcpy(A_data->patient_level->patient_id,
                            &user_input[2]);
                    break;
                    case 1:
                        strcpy(A_data->series_level->modality, &user_input[2]);
                    break;
                    case 2:
                        strcpy(A_data->image_level->image_num, &user_input[2]);
                    break;
                    case 3:
                        strcpy(A_data->study_level->study_date, &user_input[2]);
                    break;
                    default:
                        printf("Invalid choice");
                        break;
                }
            break;

            case '2':
                switch (choice)
                {
                    case 0:
                        strcpy(A_data->patient_level->patient_name,
                            &user_input[2]);
                    break;
                    case 1:
                        strcpy(A_data->series_level->series_num,
                            &user_input[2]);
                    break;
                    case 2:
                        strcpy(A_data->image_level->sop_inst_uid,
                            &user_input[2]);
                    break;
                    case 3:
                        strcpy(A_data->study_level->study_time, &user_input[2]);
                    break;
                    default:
                        printf("Invalid choice");
                        break;
                }
            break;

            case '3':
                switch (choice)
                {
                    case 1:
                        strcpy(A_data->series_level->series_inst_uid,
                            &user_input[2]);
                    break;
                    case 2:
                        strcpy(A_data->study_level->study_inst_uid,
                            &user_input[2]);
                    break;
                    case 3:
                        strcpy(A_data->study_level->accession_num,
                            &user_input[2]);
                    break;
                    default:
                        printf("Invalid choice");
                        break;
                }
            break;

            case '4':
                switch (choice)
                {
                    case 1:
                        strcpy(A_data->study_level->study_inst_uid,
                            &user_input[2]);
                    break;
                    case 2:
                        strcpy(A_data->series_level->series_inst_uid,
                            &user_input[2]);
                    break;
                    case 3:
                        strcpy(A_data->study_level->study_id, &user_input[2]);
                    break;
                }
            break;

            case '5':
                switch (choice)
                {
                    case 1:
                        strcpy(A_data->patient_level->patient_id,
                            &user_input[2]);
                    break;
                    case 2:
                        strcpy(A_data->patient_level->patient_id,
                            &user_input[2]);
                    break;
                    case 3:
                        strcpy(A_data->study_level->study_inst_uid,
                            &user_input[2]);
                    break;
                }
            break;

            case '6':
                if(choice == 3)
                {
                    strcpy(A_data->patient_level->patient_id, &user_input[2]);
                }
            break;

            case '7':
                if(choice == 3)
                {
                    strcpy(A_data->patient_level->patient_name, &user_input[2]);
                }
            break;

            case 'Q':
            case 'q':
                return ( TRUE );

        }  /* end switch */
    } while ( user_input[0] != 'D' && user_input[0] != 'd' );

    return ( FALSE );
} /* GetLevelData() */



/*****************************************************************************
 *
 * NAME
 *    SelectRecord - Selects a record to query on
 *
 * ARGUMENTS
 *    A_data       RetData *    A pointer to data
 *    A_list       RetData *    A pointer to data returned from association
 *
 * DESCRIPTION
 *    Allows the selection of a record to continue to query on.  
 *  
 * RETURNS
 *    QR_SUCCESS
 *    QR_FAILURE
 *
 * SEE ALSO
 *
 ****************************************************************************/
static QR_STATUS SelectRecord ( RetData *A_data, RetData *A_list )
{
    char   user_input[USER_INPUT_LEN];
    int    i;
    PRPL   *prpl;
    PRSTL  *prstl;
    PRSL   *prsl;
    PRIL   *pril;

    printf("\nWhich one, there are %d.\n", A_list->numData);
    printf("==> ");
    fflush ( stdout );
    fgets ( user_input, sizeof(user_input), stdin );
    fflush ( stdout );

    if (( atoi(user_input) > 0 ) && (atoi(user_input) <= A_list->numData ))
    {
        if ( !strncmp(A_data->level,PATIENT_LEVEL, sizeof(PATIENT_LEVEL)-1) )
        {
            prpl = A_list->patient_level;
            for ( i = 1; i < atoi(user_input); i++)
                prpl = prpl->next;

            memcpy ( (void*) A_data->patient_level, (void*) prpl,
                sizeof(PRPL) );
        }

        else if ( !strncmp(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL)-1))
        {
            prstl = A_list->study_level;
            for ( i = 1; i < atoi(user_input); i++)
                prstl = prstl->next;

            memcpy ( (void*) A_data->study_level, (void*) prstl,
                sizeof(PRSTL) );
        }

        else if ( !strncmp(A_data->level,SERIES_LEVEL,sizeof(SERIES_LEVEL)-1))
        {
            prsl = A_list->series_level;
            for ( i = 1; i < atoi(user_input); i++)
                prsl = prsl->next;

            memcpy ( (void*) A_data->series_level, (void*) prsl, 
                sizeof(PRSL) ); 
        }

        else if ( !strncmp(A_data->level,IMAGE_LEVEL, sizeof(IMAGE_LEVEL)-1) )
        {
            pril = A_list->image_level;
            for ( i = 1; i < atoi(user_input); i++)
                pril = pril->next;

            memcpy ( (void*)A_data->image_level, (void*) pril,
                sizeof(PRIL) );
        }

        if (EmptyList ( A_list ) != QR_SUCCESS)
            printf("ERROR: freeing record list\n");
    }
    else
        return ( QR_FAILURE );

    return ( QR_SUCCESS );
} /* SelectRecord() */




/*****************************************************************************
 *
 * NAME
 *    AddToList - Add query records to list of query results
 *
 * ARGUMENTS
 *    A_level    char *       A pointer to the type of query 
 *    A_list     RetData *    A pointer to the list of data returned
 *    A_data     RetData *    A pointer to the new data to add to list
 *
 * DESCRIPTION
 *    Addes an element to the list of query results.
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    PrintErrorMessage
 *
 ****************************************************************************/
static QR_STATUS AddToList ( char *A_level, RetData *A_list,  RetData *A_data )
{
    PRPL           *prpl;
    PRSTL          *prstl;
    PRSL           *prsl;
    PRIL           *pril;
    static char    S_prefix[] = "AddToList";

    strcpy ( A_list->level, A_level );
    strcpy ( A_list->model, A_data->model );
    if ( !strncmp ( A_level, PATIENT_LEVEL, sizeof( PATIENT_LEVEL ) - 1 ) )
    {
        prpl = (void*) malloc (sizeof(PRPL));
        if ( prpl == NULL )
        {
            PrintErrorMessage ( S_prefix, "malloc", -1, 
                "Error allocating memory" );
            return ( QR_FAILURE );
        } 
        memcpy( (void*) prpl, (void*) A_data->patient_level, sizeof(PRPL) );
        prpl->next = A_list->patient_level;
        A_list->patient_level = prpl;
    }

    else if ( !strncmp ( A_level, STUDY_LEVEL, sizeof( STUDY_LEVEL ) - 1 ) )
    {
        prstl = (void*) malloc (sizeof(PRSTL));
        if ( prstl == NULL )
        {
            PrintErrorMessage ( S_prefix, "malloc", -1,
                "Error allocating memory" );
            return ( QR_FAILURE );
        }
        memcpy( (void*) prstl, (void*) A_data->study_level, sizeof(PRSTL) );
        prstl->next = A_list->study_level;
        A_list->study_level = prstl;
    }

    else if ( !strncmp ( A_level, SERIES_LEVEL, sizeof( SERIES_LEVEL ) - 1 ) )
    {
        prsl = (void*) malloc (sizeof(PRSL));
        if ( prsl == NULL )
        {
            PrintErrorMessage ( S_prefix, "malloc", -1,
                "Error allocating memory" );
            return ( QR_FAILURE );
        }
        memcpy( (void*) prsl, (void*) A_data->series_level, sizeof(PRSL) );
        prsl->next = A_list->series_level;
        A_list->series_level = prsl;
    }

    else if ( !strncmp ( A_level, IMAGE_LEVEL, sizeof( IMAGE_LEVEL ) -1 ) )
    {
        pril = (void*) malloc (sizeof(PRIL));
        if ( pril == NULL )
        {
            PrintErrorMessage ( S_prefix, "malloc", -1,
                "Error allocating memory" );
            return ( QR_FAILURE );
        }
        memcpy( (void*) pril, (void*) A_data->image_level, sizeof(PRIL) );
        pril->next = A_list->image_level;
        A_list->image_level = pril;
    }

    A_list->numData = A_list->numData + 1;

    return ( QR_SUCCESS );
} /* AddToList() */



/*****************************************************************************
 *
 * NAME
 *    EmptyList - empties the list of query results
 *
 * ARGUMENTS
 *    A_list     RetData *    A pointer to the list of data returned
 *
 * DESCRIPTION
 *    Frees the list of query results copied into these data structures
 *    from C-FIND-RSP messages.
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    PrintErrorMessage
 *
 ****************************************************************************/
static QR_STATUS EmptyList ( RetData *A_list )
{
    int     i;
    char    *level;
    PRPL    *prpl;
    PRSTL   *prstl;
    PRSL    *prsl;
    PRIL    *pril;

    level = A_list->level;
    
    for (i=0; i<A_list->numData; i++)
    {
        if (!strncmp ( level, PATIENT_LEVEL, sizeof( PATIENT_LEVEL )-1 ) )
        {
            prpl = A_list->patient_level;
            if ( prpl != NULL )
            {
                A_list->patient_level = prpl->next;
                free ( prpl );
            }
        }

        else if (!strncmp ( level, STUDY_LEVEL, sizeof( STUDY_LEVEL )-1 ) )
        {
            prstl = A_list->study_level;
            if ( prstl != NULL )
            {
                A_list->study_level = prstl->next;
                free ( prstl );
            }
        }

        else if (!strncmp (level, SERIES_LEVEL, sizeof( SERIES_LEVEL )-1 ) )
        {
            prsl = A_list->series_level;
            if ( prsl != NULL )
            {
                A_list->series_level = prsl->next;
                free ( prsl );
            }
        }

        else if (!strncmp ( level, IMAGE_LEVEL, sizeof( IMAGE_LEVEL ) -1 ) )
        {
            pril = A_list->image_level;
            if ( pril != NULL )
            {
                A_list->image_level = pril->next;
                free ( pril );
            }
        }
    }
    A_list->numData = 0;
    A_list->patient_level = NULL;

    return ( QR_SUCCESS );
} /* EmptyList() */



/*****************************************************************************
 *
 * NAME
 *    OktoMove - Ask user if ok to move base on CFIND results
 *
 * ARGUEMENTS
 *    A_data    RetData *    Data to be moved, prompt user with it 
 *
 * DESCRIPTION
 *    This function asks user if ok to move SOP Instance based on CFIND.
 *    
 *
 * RETURNS
 *    1 for TRUE or 0 for FALSE
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static short OktoMove(RetData *A_data)
{
    char user_input[USER_INPUT_LEN];

    printf("\nOK to move ");
    if(!strncmp(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL) - 1))
    {
        printf("Image Number = [%s]\t", A_data->image_level->image_num);
        if (!strncmp (A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
            printf("Patinet ID = [%s]\n", A_data->study_level->patient_id);
        else
            printf("Patinet ID = [%s]\n", A_data->patient_level->patient_id);
    }
    else if(!strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
    {
        printf("Series Number = [%s]\n", A_data->series_level->series_num);
    }
    else if (!strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
    {
        printf("Patient ID = [%s]\n", A_data->patient_level->patient_id);
    }
    else /* study level */
    {
        printf("Study ID = [%s]\n", A_data->study_level->study_id);
    }

    printf ( "==> " );
    fflush ( stdout );
    fgets ( user_input, sizeof(user_input), stdin );
    fflush ( stdout );

    if(user_input[0] == 'Y' || user_input[0] == 'y')
        return 1;
    else
        return 0;

} /* OktoMove() */



/*****************************************************************************
*
 * NAME
 *    CFINDOption - Starts the C-FIND Process
 *
 * ARGUMENTS
 *    A_myConfig           AppConfig *    Program Config values
 *    A_root_level_info    RetData *      Current Query information
 *    A_list               RetData *      Data returned from the Query  
 *
 * DESCRIPTION
 *    Option used for the C-FIND
 *
 * RETURNS
 *    QR_STATUS set according to the error
 *
 * SEE ALSO
 *    MC_Close_Association
 *    MC_Open_Association
 *    SendCFINDMessage    
 *
 ****************************************************************************/
static QR_STATUS CFINDOption ( AppConfig *A_myConfig, 
                               RetData *A_root_level_info,
                               RetData *A_list )
{
    MC_STATUS      status;
    QR_STATUS      qrStatus;
    static char    S_prefix[] = "CFINDOption";

    status = MC_Open_Association ( A_myConfig->applicationID,
                                   &(A_myConfig->associationID), 
                                   A_myConfig->remoteApplicationTitle,
                                   NULL, NULL, NULL);
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Association",
           status, NULL );
        return ( QR_FAILURE );
    }
 
    /* 
     * Make sure there is nothing left in the list before starting           
     */

    if (EmptyList ( A_list ) != QR_SUCCESS)
        printf("ERROR: freeing found list\n");

    printf ( "%s: Sending the C-FIND request.\n", S_prefix );
    qrStatus = SendCFINDMessage ( A_root_level_info,
                                  A_myConfig, 
                                  A_list );
    if (qrStatus != QR_SUCCESS)
    {
        PrintErrorMessage ( S_prefix, "SendCFINDMessage",
            -1, "Error returned, continueing." );
    }
    
    status = MC_Close_Association ( &(A_myConfig->associationID) );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintErrorMessage ( S_prefix, "MC_Close_Association",
            status, NULL );
        return ( QR_FAILURE );
    }
    
    return ( qrStatus );
} /* CFINDOption() */



/*****************************************************************************
 *
 * NAME
 *    SendCFINDMessage - Send a C-FIND message to the remote application
 *
 * ARGUMENTS
 *    A_data            RetData *      A pointer to input data for the FIND
 *    A_myConfig        AppConfig *    A pointer to configuration data
 *    A_list            RetData *      A pointer to data returned from assoc
 *
 * DESCRIPTION
 *    SendCFINDMessage sends a C-FIND message to the remote application.
 *    After opening a new message and filling in all required fields,
 *    SendCFINDMessage sends the newly created and populated message by
 *    calling MC_Send_Request_Message.  The function then waits for 
 *    responses from the remote application.  When it gets the responses,
 *    the routine continues to put the Patient IDs in an array until a
 *    C_FIND_SUCCESS is received.
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    PrintErrorMessage
 *    MC_Free_Message
 *    MC_Get_Value_To_UInt
 *    MC_Get_Value_To_String
 *    MC_Open_Message
 *    MC_Read_Message
 *    MC_Set_Value_From_String
 *    MC_Set_Value_To_NULL
 *    CancelCFINDRQ
 *
 ****************************************************************************/
static QR_STATUS SendCFINDMessage ( RetData*   A_data, 
                                    AppConfig *A_myConfig, 
                                    RetData *A_list )
{
    static char          S_prefix[] = "SendCFINDMessage";
    static int           S_once = FALSE;
    int                  done = FALSE;
    QR_STATUS            qrStatus;
    MC_COMMAND           command;
    MC_STATUS            status;
    int                  messageID;
    int                  responseMessageID;
    char                 *serviceName;
    char                 model[SERVICENAME_LEN+1];
    unsigned int         response;
    RetData              data;
    PRPL                 patient_level;
    PRSTL                study_level;
    PRSL                 series_level;
    PRIL                 image_level;

    memcpy((void*)&data, (void*)A_data, sizeof(RetData));

    /*
     * Fill in some local data structures used to specify the C-FIND-RQ
     * messages. This is done so that the input data is not modified.
     */
    if (!strncmp ( A_data->level, PATIENT_LEVEL, sizeof( PATIENT_LEVEL )-1 ))
    {
        memcpy((void*)&patient_level, (void*)A_data->patient_level,
            sizeof(PRPL));
        data.patient_level = &patient_level;    
    }
    else if (!strncmp ( A_data->level, STUDY_LEVEL, sizeof( STUDY_LEVEL )-1 ))
    {
        memcpy((void*)&study_level, (void*)A_data->study_level, sizeof(PRSTL));
        data.study_level = &study_level;
    }
    else if (!strncmp ( A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL)-1 ))
    {
        memcpy((void*)&series_level, (void*)(A_data->series_level),
            sizeof(PRSL));
        data.series_level = &series_level;
    }
    else if ( !strncmp ( A_data->level, IMAGE_LEVEL, sizeof( IMAGE_LEVEL ) -1 ))
    {
        memcpy((void*)&image_level, (void*)(A_data->image_level), sizeof(PRIL));
        data.image_level = &image_level;
    }


    /*
     * Open a new message 
     */

    strcpy(model, data.model);
    strcat(model, "_QR_FIND");

    status = MC_Open_Message ( &messageID, model, C_FIND_RQ );     
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Message", status, NULL );
        return ( QR_FAILURE );
    }


    /*
     * Fill in the message with the required fields
     * depending on the level the find is being sent at 
     */
    qrStatus = BuildCFINDMessage(&data, messageID);
    if ( qrStatus != QR_SUCCESS )
    {
        MC_Free_Message ( &responseMessageID );
        return ( qrStatus );
    }

    /*
     * Validate and list the message
     */
    ValMessage      (messageID, "C-FIND-RQ");
    WriteToListFile (messageID, "cfindrq.msg");

    /*
     * Send off the message 
     */
    status = MC_Send_Request_Message ( A_myConfig->associationID, messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &messageID );
        PrintErrorMessage ( S_prefix, "MC_Send_Request_Message",
            status, NULL );
        return ( QR_FAILURE );
    }

    status = MC_Free_Message ( &messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Free_Message",
            status, NULL );
        return ( QR_FAILURE );
    }


    /* 
     * A single response message is sent for each
     * match to the find request.  Wait for all of these
     * response messages here.  This loop is exited when
     * the status contained in a response message
     * is equal to a failure, or C_FIND_SUCCESS.
     */
    while (done != TRUE)
    {
        status = MC_Read_Message ( A_myConfig->associationID,
                                   A_myConfig->timeOut,
                                   &responseMessageID,
                                   &serviceName,
                                   &command );
        if ( status == MC_TIMEOUT )
        {
            printf("Timed out in MC_Read_Message.  Calling again.\n");
            continue;
        }
        else if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Read_Message",
                status, NULL );
            return ( QR_FAILURE );
        }

        /* 
         * The status of the find response is contained in
         * the DICOM group 0 elements.  Retreive this status
         * and examine it here.
         */
        status = MC_Get_Value_To_UInt ( responseMessageID,
                                        MC_ATT_STATUS,
                                        &response );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &responseMessageID );
            PrintErrorMessage ( S_prefix, "MC_Get_Value_To_UInt",
                status, NULL );
            return ( QR_FAILURE );
        }                                             

        /* 
         * If you are thinking that this error section could be better handled
         *  using a switch statement, you are right.  However under the OS9000
         *  compiler it did not work as a switch.  The solution was to write
         *  it as an if-else.
         */
        if ( response == C_FIND_SUCCESS )
        {
            /*
             * If we get a success, we are finished 
             */
            printf ( "%s: Response is C_FIND_SUCCESS\n", S_prefix );
            S_once = FALSE;
            done = TRUE;
        }
        else if ( response == C_FIND_CANCEL_REQUEST_RECEIVED ) 
        {
            /* 
             * If we get a cancel,  we are finished 
             */
            printf ("%s: Response C_FIND_CANCEL_REQUEST_RECEIVED\n", S_prefix );
            S_once = FALSE;
            done = TRUE;
        }
        else if ( response == C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT || 
                  response == C_FIND_PENDING )
        {
            if ( response == C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT )
            {
                printf ( "%s: C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT\n",
                         S_prefix );
                printf ( "\t\twas received...\n" );
            }
            /*
             * This means we got a real message with data 
             */
    
            /*
             * Hold off on Validation of CFIND until we know it is not  
             *   a C_FIND_SUCCESS since C_FIND_SUCCESS will not validate 
             *   because it is an empty message.                         
             */
            ValMessage      (responseMessageID, "C-FIND-RSP");
            WriteToListFile (responseMessageID, "cfindrsp.msg");

            qrStatus = ReadCFINDMessage(&data, A_data, responseMessageID, 
                A_myConfig); 
            if ( qrStatus == QR_FAILURE )
            {
                MC_Free_Message(&responseMessageID);
                return ( QR_FAILURE );
            }

            if(A_list->numData < A_myConfig->maxQueryResponses )
            {
                /* 
                 * Add it to the list of responses to the query
                 */
                qrStatus = AddToList ( A_data->level, A_list, &data );
                if ( qrStatus == QR_FAILURE )
                {
                    MC_Free_Message(&responseMessageID);
                    return ( QR_FAILURE );
                }
            }
            else if ( S_once != TRUE ) 
            {  
                /* 
                 * We have filled up our buffer of C-FIND-RSP messages we
                 * can receive, we now send a cancel to the server to stop
                 * sending these response messages.  Note that because of
                 * a delay in when the SCP receives the C-CANCEL, several
                 * responses may continue to come in.
                 */
                if (CancelCFINDRQ(&A_myConfig->associationID, model) ==
                    QR_FAILURE)
                    printf("ERROR: sending C-FIND-RSP,  C-CANCEL\n");
                S_once = TRUE;
            }
        }
        else
        {
            /* 
             * Some other kind of error message 
             */
            printf("%s: Response is Unknown Error:  %X.\n", S_prefix, response);
            S_once = FALSE;
            done = TRUE;
        }

        /*
         * Free the valid response message and continue 
         * waiting for another response.
         */
        status = MC_Free_Message ( &responseMessageID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Free_Message",
                status, NULL );
            return ( QR_FAILURE );
        }
    } /* while(done != TRUE) */


    printf ("Found %d matches.\n", A_list->numData );
    return ( QR_SUCCESS );
} /* SendCFINDMessage() */





/*****************************************************************************
 *
 * NAME
 *    CMOVEOption - Starts the C-MOVE Process
 *
 * ARGUMENTS
 *    A_myConfig           AppConfig *    Program Config values
 *    A_rootInfo           RetData *      Current Query information
 *
 * DESCRIPTION
 *    This funciton handles sending a C-MOVE-RQ message.  The association is
 *    opened and the C-MOVE-RQ message is sent based on user input
 *    and the data contained in the A_root_level_info structure.
 *
 * RETURNS
 *    QR_STATUS according to appropriate error conditions
 *
 * SEE ALSO
 *    MC_Open_Association
 *    SendCMOVEMessage
 *    MC_Abort_Association
 *    MC_Close_Association    
 *
 ****************************************************************************/
static QR_STATUS CMOVEOption ( AppConfig *A_myConfig,
                               RetData *A_rootInfo )
{
    MC_STATUS     status;
    QR_STATUS     qrStatus;
    static char   S_prefix[] = "CMOVEOption";
    char          *retrieveAE = "";


    if ( !strncmp(A_rootInfo->level,PATIENT_LEVEL, sizeof(PATIENT_LEVEL)-1) )
        retrieveAE = A_rootInfo->patient_level->retrieveAETitle;
    else if ( !strncmp(A_rootInfo->level, STUDY_LEVEL, sizeof(STUDY_LEVEL)-1))
        retrieveAE = A_rootInfo->study_level->retrieveAETitle;
    else if ( !strncmp(A_rootInfo->level,SERIES_LEVEL,sizeof(SERIES_LEVEL)-1))
        retrieveAE = A_rootInfo->series_level->retrieveAETitle;
    else if ( !strncmp(A_rootInfo->level,IMAGE_LEVEL, sizeof(IMAGE_LEVEL)-1) )
        retrieveAE = A_rootInfo->image_level->retrieveAETitle;

    if (!strcmp(retrieveAE, ""))
    {
        printf("The location of the image was not given by the archive,\nso it"
            " is either on offline storage, or its location is not known.\n");
        return QR_OFFLINE;
    }

    /* 
     * Establish an association 
     */
    status = MC_Open_Association ( A_myConfig->applicationID,
                                   &(A_myConfig->associationID),
                                   retrieveAE,
                                   NULL,
                                   NULL,
                                   NULL );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Association", status, NULL );
        return ( QR_FAILURE );
    }

    /*
     * Send the C-MOVE Message 
     */
    printf ( "%s: Sending the C-MOVE request.\n", S_prefix );

    qrStatus = SendCMOVEMessage( A_rootInfo, A_myConfig );
    if ( qrStatus != QR_SUCCESS )
    {
        MC_Abort_Association ( &(A_myConfig->associationID) );
        PrintErrorMessage ( S_prefix, "SendCMOVEMessage",
            -1, "SendCMOVEMessage returned QR_FAILURE" );
        return ( qrStatus );
    }
    
    status = MC_Close_Association ( &(A_myConfig->associationID) );
    if (status != MC_NORMAL_COMPLETION)
    {
        PrintErrorMessage ( S_prefix, "MC_Close_Association",
            status, NULL );
    }

    return ( QR_SUCCESS );
} /* CMOVEOption() */



/*****************************************************************************
 *
 * NAME
 *    SendCMOVEMessage - Processes a C-MOVE message
 *
 * ARGUMENTS
 *    A_root_data       RetData *      A pointer to data structure with info.
 *    A_myConfig        AppConfig *    A pointer to configuration data.
 *
 * DESCRIPTION
 *    SendCMOVEMessage handles a C-MOVE message.  It begins by calling
 *    ProcessCSTOREAssociation.  This initial initial call gets the 
 *    listen socket started to handle the incoming store association.  
 *    This avoids a possible problem with timing.
 *
 *    Then, for every selected record in the A_root_data structure, a C-MOVE 
 *    request message is generated and sent to the remote application.  Once 
 *    this happens, three events can occur: 1) a message from the first (move) 
 *    association may arrive, 2) activity may occur on the store association 
 *    such as the association being opened or the C-STORE-RQ messages being
 *    received.  When a C-MOVE-RSP  message is received with a status
 *    signifying the move has completed or failed, the routine is exited.
 *    Note that the destination AE title filled in the C-MOVE-RQ may be 
 *    different than this application.  (You can move to another Storage Service
 *    Class SCP.)  The ProcessCSTOREAssociation() function will still operate 
 *    properly if this is the case.
 *
 *    If you are modifying this application so that the move destination is not
 *    going to be this application (it may be a seperate process running our
 *    stor_scp application), the calls to ProcessCSTOREAssociation can be 
 *    eliminated from this routine.
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    PrintErrorMessage
 *    ProcessCSTOREAssociation
 *    MC_Abort_Association
 *    MC_Accept_Association
 *    MC_Free_Message
 *    MC_Open_Message
 *    MC_Read_Message
 *    MC_Reject_Association
 *    MC_Send_Request_Message
 *    MC_Set_Value_From_String
 *    MC_Wait_For_Association
 *
 ****************************************************************************/
static QR_STATUS SendCMOVEMessage ( RetData *A_root_data,
                                    AppConfig *A_myConfig )
{
    static char           S_prefix[] = "SendCMOVEMessage";
    QR_STATUS             qrStatus;
    MC_COMMAND            command;
    MC_STATUS             status;
    int                   messageID;
    int                   moveResponseMessageID;
    int                   moveAssociationID = A_myConfig->associationID;
    int                   continueLoop = TRUE;
    char                 *serviceName;
    char                  model[SERVICENAME_LEN+1];
    unsigned int          response;
    time_t                start_time;
    RetData              *tmpData;
 

    /* 
     * Post a "wait" immediately.  This gets the listen socket started.
     * This is only needed because this routine polls between waiting
     * for an association and reading messages.  
     */
    qrStatus = ProcessCSTOREAssociation(A_root_data->level, A_myConfig);
    if ( qrStatus != QR_SUCCESS )
    {
        return ( QR_FAILURE );
    }

    /*
     * There is nothing left in the list so we are gonna take the 
     * one that was selected.  Its contents are in the A_root_data var
     */
    tmpData = A_root_data;
    
    if ( OktoMove( tmpData ) )
    {
        time(&start_time);       /* lets see how long this takes */

        /*
         * Open a new message at the proper level, depending on where
         * the move request was done.
         */
        strcpy(model, A_root_data->model);
        strcat(model, "_QR_MOVE");

        status = MC_Open_Message ( &messageID,
                                   model,
                                   C_MOVE_RQ );
        if ( status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Open_Message", status, NULL );
            return ( QR_FAILURE );
        }

        /*
         * Build the C-MOVE-RQ message with required fields based
         * on the level where the move is being requested.
         */
        qrStatus = BuildCMOVEMessage( tmpData, messageID, A_myConfig,
            A_root_data->level );
        if (qrStatus != QR_SUCCESS)
        {
            MC_Free_Message ( &messageID );
            PrintErrorMessage ( S_prefix, "BuildCMOVEMessage", -1, "Failure" );
            return ( qrStatus );
        }
        
        /*
         * Validate and list the message
         */
        ValMessage       (messageID, "C-MOVE-RQ");
        WriteToListFile (messageID, "cmoverq.msg");

        /*
         * Send off the message 
         */
        status = MC_Send_Request_Message ( moveAssociationID,
                                           messageID );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Free_Message ( &messageID );
            PrintErrorMessage ( S_prefix, "MC_Send_Request_Message",
                status, NULL );
            return ( QR_FAILURE );
        }

        status = MC_Free_Message ( &messageID );
        if (status != MC_NORMAL_COMPLETION )
        {
            PrintErrorMessage ( S_prefix, "MC_Free_Message",
                status, NULL );
        }


        /*
         * Wait for the C-MOVE-RSP messages and activity over
         * the storage associatin such as the association being
         * opened or messages received.
         */
        continueLoop = TRUE;
        while ( continueLoop == TRUE )
        {
            /* 
             * Do not wait for a C-MOVE-RSP message.
             */
            status = MC_Read_Message ( moveAssociationID, 0,
                &moveResponseMessageID, &serviceName, &command );
            switch ( status )
            {
                case MC_TIMEOUT:
                    /*
                     * Nothin received, now we should poll the store association
                     */
                    break;
                case MC_NORMAL_COMPLETION:

                    ValMessage      (moveResponseMessageID, "C-MOVE-RSP");
                    WriteToListFile (moveResponseMessageID, "cmoversp.msg");

                    status = MC_Get_Value_To_UInt(moveResponseMessageID,
                        MC_ATT_STATUS, &response);
                    if (status != MC_NORMAL_COMPLETION)
                        PrintErrorMessage(S_prefix, "MC_Get_Value_To_UInt",
                            status, NULL );
        
                    status = MC_Free_Message ( &moveResponseMessageID );
                    if (status != MC_NORMAL_COMPLETION)
                        PrintErrorMessage(S_prefix, "MC_Free_Message",
                            status, NULL);
                    
                    /*
                     * Check the status returned in the response message.
                     */
                    if ( response == C_MOVE_SUCCESS_NO_FAILURES )
                        /*
                         * The move has completed!
                         */
                        continueLoop = FALSE;
                    else if( response != C_MOVE_PENDING_MORE_SUB_OPERATIONS )
                    {
                        PrintErrorMessage(S_prefix, "Response for C_MOVE",
                                          -1, "Error in CMOVE operation");
                        MC_Free_Message ( &moveResponseMessageID );
                        return( QR_FAILURE );
                    } 
        
                    break;
                default:
                    MC_Free_Message ( &moveResponseMessageID );
                    PrintErrorMessage ( S_prefix, 
                        "MC_Read_Message for move response", status, NULL );
                    return ( QR_FAILURE );

            } /* switch status of move response read message */


            qrStatus = ProcessCSTOREAssociation(A_root_data->level, A_myConfig);
            if ( qrStatus != QR_SUCCESS )
                return ( qrStatus );

        } /* continueLoop == TRUE */

        printf("\nElapsed time of Move Operation: %ld seconds\n",
            time((time_t *)NULL) - start_time);
    } /* if OktoMove  */

    qrStatus = ProcessCSTOREAssociation(A_root_data->level, A_myConfig);
    if ( qrStatus != QR_SUCCESS )
        return ( qrStatus );

    return ( QR_SUCCESS );
} /* SendCMOVEMessage() */



/*****************************************************************************
 *
 * NAME
 *    ProcessCSTOREAssociation - Processes a C-Store Association
 *
 * ARGUMENTS
 *    A_level       RetData *      String containing the level.
 *    A_myConfig    AppConfig *    A pointer to configuration data.
 *
 * DESCRIPTION
 *    ProcessCSTOREAssociation contains the code for a simple storage 
 *    service class SCP.  It is intended to handle the C-STORE operation
 *    conducted by the Q/R SCP after a C-MOVE-RQ message has been sent.
 *    This routine is not intended to be a "final" solution for a 
 *    Storage SCP when doing Q/R.  A typical application would have 
 *    another thread or process acting as the storage SCP when a move is 
 *    requested apart from the Q/R application.  
 *
 *    This routine will only allow a single association at a time.  
 *    It is meant to be polled repeatedly to handle any activity over this
 *    association.  Note that because of this polling, it may miss the store
 *    association being  closed.   
 *
 * RETURNS
 *    QR_SUCCESS if the routine finishes properly.
 *    QR_FAILURE if the routine detects an error.
 *
 * SEE ALSO
 *    MC_Wait_For_Association
 *    MC_Read_Message
 *    MC_Abort_Association
 *    ValMessage
 *    WriteToListFile
 *    MC_Message_To_Stream
 *    CancelCMOVERQ       
 *    MC_Open_Message
 *    MC_Send_Response_Message
 *    MC_Free_Message
 *    PrintErrorMessage
 *
 ****************************************************************************/
static QR_STATUS ProcessCSTOREAssociation ( char *A_level,
                                            AppConfig *A_myConfig )
{
    static int            S_storeAssociationID = -1;
    static int            S_numImages = -1;
    MC_COMMAND            command;
    MC_STATUS             status;
    int                   storeMessageID;
    int                   calledApplicationID;
    int                   responseMessageID;
    int                   waitStoreAssociationID = -1;
    char*                 serviceName;
    CBinfo                callbackInfo;
    char                  fileName[256];
    char                  listFileName[256];
 
    static char           S_prefix[] = "ProcessCSTOREAssociation";

    strcpy (callbackInfo.prefix, "MessageToStream");

    if ( S_storeAssociationID == -1)
    {
        /* 
         * Check if any associations have been requested
         */
        status = MC_Wait_For_Association ( A_myConfig->serviceList1,
                        0,
                        &calledApplicationID,
                        &waitStoreAssociationID );
        switch ( status )
        {
            case MC_NORMAL_COMPLETION:
                printf ( "%s: Successful connection from remote.\n",
                         S_prefix );
                S_storeAssociationID = waitStoreAssociationID;
                status = MC_Accept_Association ( S_storeAssociationID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    PrintErrorMessage ( S_prefix, "MC_Accept_Association",
                                        status, NULL );
                    return ( QR_FAILURE );
                }
                break;
    
            case MC_NEGOTIATION_ABORTED:
                printf("Association aborted during negotiation, continuing\n");
                break;
                
            case MC_TIMEOUT:
                break;
            default:
                 PrintErrorMessage ( S_prefix, "MC_Wait_For_Association",
                     status, NULL );
                 return ( QR_FAILURE );
        } /* switch status of MC_Wait_For_Association for store association */
    }

    /*
     * If we do not have an association open, return from this function,
     * or else look for a message on currently open association.
     */
    if (S_storeAssociationID == -1)
        return ( QR_SUCCESS );


    for (;;)
    {
        /*
         * A for loop is used here incase activity is "queued up"
         * on this association.  If a timeout occurs, or the 
         * association is dropped, we will abort out of this loop.
         * Most of the time is spent here
         */
        status = MC_Read_Message ( S_storeAssociationID, 0,
                                   &storeMessageID, 
                                   &serviceName, &command );
        if (status == MC_TIMEOUT)
            return ( QR_SUCCESS );
        else if (status == MC_ASSOCIATION_CLOSED
              || status == MC_ASSOCIATION_ABORTED
              || status == MC_NETWORK_SHUT_DOWN)
        {
            printf("Storage association closed\n");
            S_storeAssociationID = -1;
            return ( QR_SUCCESS );
        }
        else if (status != MC_NORMAL_COMPLETION)
        {
            printf("Aborting storage association\n");
            MC_Abort_Association ( &S_storeAssociationID );
            PrintErrorMessage ( S_prefix, "MC_Read_Message", status, NULL );
            S_storeAssociationID = -1;
            return ( QR_FAILURE );
        }
        else
        {         
            /*
             * We have successfully received a C-STORE 
             * message.  It will now be saved to disk.
             */   
            S_numImages++;

            /* 
             * Make a file name to hold the image 
             */
            sprintf ( fileName, "QR_%d", S_numImages );
            sprintf ( listFileName, "QR_%d", S_numImages );
            strcat ( fileName, ".img" );
            strcat ( listFileName, ".msg" );

            ValMessage      (storeMessageID, "C-STORE-RQ");
            WriteToListFile (storeMessageID, listFileName );

            /* 
             * Stream the message into a file 
             */
            printf ( "%s: Streaming message to file.\n", S_prefix );
            if (NULL == (callbackInfo.stream = fopen(fileName, BINARY_WRITE)))
            {
                PrintErrorMessage(S_prefix, "fopen", -1, "File open failed.");
                break;
            }

            callbackInfo.messageID = storeMessageID;
            status = MC_Message_To_Stream(  storeMessageID,
                                            MC_ATT_GROUP_0008_LENGTH,
                                            MC_ATT_PIXEL_DATA,
                                            IMPLICIT_LITTLE_ENDIAN,
                                            (void*)&callbackInfo,
                                            MessageToFile );
            fclose ( callbackInfo.stream );

            if (status != MC_NORMAL_COMPLETION)
            {
                PrintErrorMessage ( S_prefix, "MC_Message_To_Stream", status,
                    NULL );
                if (CancelCMOVERQ(&A_myConfig->associationID, A_level) !=
                    QR_SUCCESS)
                    printf("ERROR: sending C-MOVE-RSP, C-CANCEL\n");;
                break;
            }

            /* 
             * Send a successful response 
             */
            status = MC_Open_Message ( &responseMessageID,
                                       serviceName,
                                       C_STORE_RSP );
            if ( status != MC_NORMAL_COMPLETION )
            {
                PrintErrorMessage ( S_prefix, "MC_Open_Message",
                    status, NULL );
                break;
            }

            status = MC_Send_Response_Message ( S_storeAssociationID,
                                                C_STORE_SUCCESS,
                                                responseMessageID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                MC_Free_Message ( &responseMessageID );
                PrintErrorMessage ( S_prefix, "MC_Send_Response_Message",
                    status, NULL );
                break;
            }

            status = MC_Free_Message ( &responseMessageID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                PrintErrorMessage ( S_prefix, "MC_Free_Message for C-STORE-RSP",
                    status, NULL );
                break;
            }

            status = MC_Free_Message (&storeMessageID);
            if (status != MC_NORMAL_COMPLETION )
            {
                PrintErrorMessage ( S_prefix, "MC_Free_Message for C-STORE-RQ",
                    status, NULL);
                break;        
            }
        }
    } /* for (;;) */

    if (status != MC_NORMAL_COMPLETION)
    {
        printf("Storage association aborted\n");
        MC_Free_Message ( &storeMessageID );
        MC_Abort_Association ( &S_storeAssociationID );
        S_storeAssociationID = -1;
        return ( QR_FAILURE );
    }
    
    return ( QR_SUCCESS ); 
} /* ProcessCSTOREAssociation() */



/*****************************************************************************
 *
 * NAME
 *    CancelCMOVERQ - Cancel a C-MOVE request.
 *
 * ARGUMENTS
 *    A_associationID    int *     Association ID of AE getting the CANCEL
 *    A_model            char *    Model name for which we are canceling
 *
 * DESCRIPTION
 *    Sends a C_CANCEL_MOVE_RQ message to a remote AE
 *
 * RETURNS
 *    QR_STATUS with appropriate error for current condition
 *
 * SEE ALSO
 *    MC_Open_Message
 *    MC_Send_Request_Message
 *    MC_Free_Message
 *
 ****************************************************************************/
static QR_STATUS CancelCMOVERQ ( int *A_associationID, char *A_model )
{
    static char   S_prefix[]="CancelCMOVERQ";
    int           status;
    int           messageID;

    status = MC_Open_Message ( &messageID, A_model, C_CANCEL_MOVE_RQ );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Message",
               status, NULL );
        return ( QR_FAILURE );
    }

    ValMessage      ( messageID, "C-CANCEL-MOVE-RQ" );        
    WriteToListFile( messageID, "ccancelq.msg" );

    status = MC_Send_Request_Message ( *A_associationID,
                                        messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &messageID );
        PrintErrorMessage ( S_prefix, "MC_Send_Response_Message",
               status, NULL );
        return ( QR_FAILURE );
    }

    status = MC_Free_Message ( &messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Free_Message",
               status, NULL );
        return ( QR_FAILURE );
    }

    return ( QR_SUCCESS );
} /* CancelCMOVERQ() */



/*****************************************************************************
 *
 * NAME
 *    CancelCFINDRQ - Cancel a C-FIND request.
 *
 * ARGUMENTS
 *    A_associationID    int *     Association ID of AE getting the CANCEL
 *    A_model            char *    Model name for which we are canceling
 *
 * DESCRIPTION
 *    Sends a C_FIND_CANCEL_RQ message to a remote AE
 *
 * RETURNS
 *    QR_STATUS with appropriate error for current condition
 *
 * SEE ALSO
 *    MC_Open_Message
 *    MC_Send_Request_Message
 *    MC_Free_Message
 *
 ****************************************************************************/
static QR_STATUS CancelCFINDRQ ( int *A_associationID, char *A_model )
{
    int           status;
    int           messageID;
    static char   S_prefix[]="CancelCFINDRQ";

    status = MC_Open_Message ( &messageID, A_model, C_CANCEL_FIND_RQ );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Open_Message", status, NULL );
        return ( QR_FAILURE );
    }

    ValMessage     ( messageID, "C-CANCEL-FIND-RQ" );        
    WriteToListFile( messageID, "ccancelq.msg" );

    status = MC_Send_Request_Message ( *A_associationID, messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage(S_prefix, "MC_Send_Response_Message", status, NULL);
        return ( QR_FAILURE );
    }

    status = MC_Free_Message ( &messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Free_Message", status, NULL );
        return ( QR_FAILURE );
    }

    return ( QR_SUCCESS );
} /* CancelCFINDRQ() */





/*****************************************************************************
 *
 * NAME
 *    BuildCFINDMessage - Builds CFIND message  
 *
 * ARGUMENTS
 *    A_data         RetData *    Current Query information
 *    A_messageid    int          Id of message to build
 *
 * DESCRIPTION
 *    This function builds a CFIND message based on user input
 *
 * RETURN
 *    QR_SUCCESS or QR_FAILURE
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static QR_STATUS BuildCFINDMessage(RetData *A_data, int A_messageid)
{
    MC_STATUS status;
    static char S_prefix[] = "BuildCFINDMessage";

    status = MC_Set_Value_From_String (A_messageid,
        MC_ATT_QUERY_RETRIEVE_LEVEL, A_data->level );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, 
            "QUERY_RETRIEVE_LEVEL, MC_Set_Value_From_String", status, NULL );
        return ( QR_FAILURE );
    }
    
    /*
     * Set values that you want returned from a query to NULL or they will
     *  not be sent back.  A wild card "*" will work for some values, but
     *  not all of them.
     */
    printf("We have a(n) %s Model, %s Level Query.\n", A_data->model,
        A_data->level);

    /* 
     * Fields needed for PATIENT_LEVEL (Patient Root, Patient/Study Root) 
     */
    if (!strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
    {
        if ( SetValue ( A_messageid, MC_ATT_PATIENTS_NAME, 
            A_data->patient_level->patient_name, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_PATIENT_ID, 
            A_data->patient_level->patient_id, "*", FALSE) == QR_FAILURE )
            return ( QR_FAILURE );
    } /* end of PATIENT_LEVEL */


    /* 
     * Fields needed for STUDY_LEVEL (All Models) 
     */
    else if (!strncmp(A_data->level, STUDY_LEVEL, sizeof(STUDY_LEVEL) - 1))
    {
        if ( SetValue ( A_messageid, MC_ATT_STUDY_DATE,
            A_data->study_level->study_date, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_TIME,
            A_data->study_level->study_time, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_ACCESSION_NUMBER,
            A_data->study_level->accession_num, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_ID,
            A_data->study_level->study_id, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );


        /* 
         * Fields needed by Study Root Model, and other Models   
         */
        if(!strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
        {
            if ( SetValue ( A_messageid, MC_ATT_PATIENTS_NAME,
                A_data->patient_level->patient_name, NULL,FALSE) == QR_FAILURE )
                return ( QR_FAILURE );

            if ( SetValue ( A_messageid, MC_ATT_PATIENT_ID,
                A_data->patient_level->patient_id, "*", FALSE) == QR_FAILURE )
                return ( QR_FAILURE );
        } 
        else /* PATIENT MODEL and PATIENT/STUDY ONLY MODEL do this */
        {
            if ( SetValue ( A_messageid, MC_ATT_PATIENT_ID,
                A_data->patient_level->patient_id, "*", FALSE) == QR_FAILURE )
                return ( QR_FAILURE );
        }

    } /* end of STUDY_LEVEL */

    /*
     * Fields needed for SERIES_LEVEL (Patient Root, Study Root) 
     */
    else if(!strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
    {
        if ( SetValue ( A_messageid, MC_ATT_MODALITY,
            A_data->series_level->modality, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );
 
        if ( SetValue ( A_messageid, MC_ATT_SERIES_NUMBER,
            A_data->series_level->series_num, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_SERIES_INSTANCE_UID,
            A_data->series_level->series_inst_uid, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        /* 
         * Set the Fields of the other levels (Patient, Study)  
         */
        if (!strncmp(A_data->model, STUDY_MODEL, sizeof ( STUDY_MODEL ) -1 ))
        {
            if ( SetValue ( A_messageid, MC_ATT_PATIENTS_NAME,
                "", NULL, FALSE) == QR_FAILURE )
                return ( QR_FAILURE );
        }

        if ( SetValue ( A_messageid, MC_ATT_PATIENT_ID,
            A_data->patient_level->patient_id, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid, NULL, TRUE) == QR_FAILURE )
            return ( QR_FAILURE );

    } /* End of SERIES_LEVEL */


    /* 
     * Fields needed for IMAGE_LEVEL (Patient Root, Study Root Models ) 
     */
    else if(!strncmp(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL) - 1))
    {
        if ( SetValue ( A_messageid, MC_ATT_IMAGE_NUMBER,
            A_data->image_level->image_num, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_SOP_INSTANCE_UID,
            A_data->image_level->sop_inst_uid, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );


        /*
         * Set the Fields of the other levels (Patient, Study, Series) 
         */
        if (!strncmp(A_data->model, STUDY_MODEL, sizeof ( STUDY_MODEL ) -1 ))
        {
            if ( SetValue ( A_messageid, MC_ATT_PATIENTS_NAME,
                "", NULL, FALSE) == QR_FAILURE )
                return ( QR_FAILURE );
        }

        if ( SetValue ( A_messageid, MC_ATT_PATIENT_ID,
            A_data->patient_level->patient_id, NULL, FALSE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid, NULL, TRUE) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageid, MC_ATT_SERIES_INSTANCE_UID,
            A_data->series_level->series_inst_uid, NULL, TRUE) == QR_FAILURE )
            return ( QR_FAILURE );

    } /* End of IMAGE_LEVEL */

    return(QR_SUCCESS);
} /* BuildCFINDMessage() */



/*****************************************************************************
 *
 * NAME
 *    SetValue - Sets values for a message
 *
 * ARGUMENTS
 *    A_messageid    int              Id of message the value is being set in
 *    A_tag          unsigned long    Tag of message that is to be set
 *    A_value        char *           Value of tag that is to be set
 *    A_default      char *           Default value of tag, if A_value is NULL 
 *    A_required     int              Flag telling if the tag is required 
 *
 * DESCRIPTION
 *    This function sets the values for a tag in a message. If the value  
 *    given is NULL and the tag is not required, the tag will be set to NULL 
 *    using MC_Set_Value_To_NULL().  If the tag value is not required, and the 
 *    value given is NULL you can specify a default value. The default value
 *    will then be used and set using MC_Set_Value_From_String(). 
 *
 * RETURNS
 *    QR_SUCCESS or QR_FAILURE
 *
 * SEE ALSO
 *    PrintErrorMessage()
 *    MC_Set_Value_From_String()
 *    MC_Set_Value_To_NULL() 
 *
 ****************************************************************************/
static QR_STATUS SetValue ( int A_messageid, unsigned long A_tag,
                            char *A_value, char *A_default, 
                            int A_required )
{
    MC_STATUS      status;
    static char    S_prefix[] = "SetValue";

    if ( strlen(A_value) <= (size_t)0 )
    {
        /*
         * The tag we were gonna set, was not given a value
         */
        if ( A_required == FALSE )
        {
            /*
             * It's not a required tag, so we can set it to NULL
             */
            status = MC_Set_Value_To_NULL ( A_messageid, A_tag );
            if ( status != MC_NORMAL_COMPLETION )
            {
                PrintErrorMessage(S_prefix,"MC_Set_Value_To_NULL",status, NULL);
                return ( QR_FAILURE );
            }
            return ( QR_SUCCESS );
        }
        else if ( A_required == TRUE )
        {
            /*
             * The tag is required so we must check to see if there is a
             *  default since we were not given a value for it.
             */
            if ( strncmp( A_default, NULL, sizeof(NULL) ) )
            {
                status = MC_Set_Value_From_String ( A_messageid, A_tag,
                    A_default );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    PrintErrorMessage (S_prefix,"MC_Set_Value_From_String,"
                        " default", status, NULL);
                    printf("***          Tag: %lX\n", A_tag);
                    return ( QR_FAILURE );
                }
                return ( QR_SUCCESS );
            } 
            else
            {
                /*
                 * This is a required tag and no value was given for it.  Ther
                 *  is no default value, so we cannot set it.  Its an error.
                 */
                printf("%s:%lX, Required Parameter not set.\n", S_prefix,A_tag);
                return ( QR_FAILURE );
            }
        }
    }

    /*
     * This is just a usual tag that is being set, since we have a value for it
     */
    status = MC_Set_Value_From_String ( A_messageid, A_tag, A_value ); 
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage( S_prefix, "MC_Set_Value_From_String", status, NULL );
        printf("***          Tag: %lX\n", A_tag);
        return ( QR_FAILURE );
    }
    return ( QR_SUCCESS );

} /* SetValue() */



/*****************************************************************************
 *
 * NAME
 *    BuildCMOVEMessage - Builds CMOVE message  
 *
 * ARGUMENTS
 *    A_data         RetData *      Information needed for the message
 *    A_messageID    int            Id of message being built 
 *    A_myConfig     AppConfig *    Program defaults structure
 *    A_moveLevel    char *         Name of level being moved
 *
 * DESCRIPTION
 *    This function builds a CMOVE message based on user input that has
 *    been filled into the A_data structure.
 *
 * RETURNS
 *    QR_SUCCESS or QR_FAILURE
 *
 * SEE ALSO
 *    MC_Set_Value_From_String
 *    PrintErrorMessage
 *    SetVal
 *
 ****************************************************************************/
static QR_STATUS BuildCMOVEMessage( RetData*   A_data, 
                                    int        A_messageID, 
                                    AppConfig* A_myConfig,  
                                    char*      A_moveLevel ) 
{
    MC_STATUS status;
    static char S_prefix[] = "BuildCMOVEMessage";

    /*
     * Set the Query/Retrieve Level
     */
    status = MC_Set_Value_From_String ( A_messageID,
                                        MC_ATT_QUERY_RETRIEVE_LEVEL,
                                        A_moveLevel );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage( S_prefix, "MC_Set_Value_From_String", status, NULL );
        return ( QR_FAILURE );
    }

    /* 
     * Set move destination.  This is the AE title where the 
     * Query SCP will store the images that are requested to
     * be moved.  The default is to sent back to this AE.  
     * You can also place the AE title of another system in 
     * this field.
     */
    status = MC_Set_Value_From_String (A_messageID,
                                       MC_ATT_MOVE_DESTINATION,
                                       A_myConfig->moveDestinationTitle);
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "Move_Destination, SVFS", status, NULL );
        return ( QR_FAILURE );
    }

    status = MC_Set_Value_From_String (A_messageID,
                                       MC_ATT_QUERY_RETRIEVE_LEVEL,
                                       A_data->level );
    if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage(S_prefix,"Query_Retrieve_level, SVFS", status, NULL );
        return ( QR_FAILURE );
    }

    if(!strncmp(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL) - 1))
    {
        /* check and set required attributes */
        if ( SetValue ( A_messageID, MC_ATT_SOP_INSTANCE_UID,
            A_data->image_level->sop_inst_uid, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageID, MC_ATT_SERIES_INSTANCE_UID,
            A_data->series_level->series_inst_uid, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageID, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );

        /*
         * If this is a STUDY_ROOT Model, then we don't set the patient id
         */
        if (strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1 ))
        {
            if ( SetValue ( A_messageID, MC_ATT_PATIENT_ID,
                A_data->patient_level->patient_id, NULL, TRUE ) == QR_FAILURE )
                return ( QR_FAILURE );
        }
    }
    else if (!strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
    {
        if ( SetValue ( A_messageID, MC_ATT_PATIENT_ID,
            A_data->patient_level->patient_id, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );
    }
    else if(!strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
    {
        if ( SetValue ( A_messageID, MC_ATT_SERIES_INSTANCE_UID,
            A_data->series_level->series_inst_uid, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( SetValue ( A_messageID, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );

        /*
         * If this is a STUDY_ROOT Model, then we don't set the patient id
         */
        if ( strncmp (A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) ) )
        {
            if ( SetValue ( A_messageID, MC_ATT_PATIENT_ID,
                A_data->patient_level->patient_id, NULL, TRUE ) == QR_FAILURE )
                return ( QR_FAILURE );
        }
    }
    else /* study level */
    {
        if ( SetValue ( A_messageID, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid, NULL, TRUE ) == QR_FAILURE )
            return ( QR_FAILURE );

        /*
         * If this is a STUDY_ROOT Model, then we don't set the patient id
         */
        if ( strncmp (A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) ) )
        { 
            if ( SetValue ( A_messageID, MC_ATT_PATIENT_ID,
                A_data->patient_level->patient_id, NULL, TRUE ) == QR_FAILURE )
                return ( QR_FAILURE );
        }
    }

    return(QR_SUCCESS);
} /* BuildCMOVEMessage() */



/*****************************************************************************
 *
 * NAME
 *    ReadCFINDMessage - Read a CFIND message  
 *
 * ARGUMENTS
 *    A_data               RetData *      Message found to be added to a list
 *    A_root_level_info    RetData *      Current query information
 *    A_messageid          int            Id of message that is being read
 *    A_myConfig           AppConfig *    Application configuration information
 *
 * DESCRIPTION
 *    This function reads a CFIND message and copies the appropriate 
 *    information into the A_data structure.
 *
 * RETURNS
 *    QR_SUCCESS or QR_FAILURE
 *
 * SEE ALSO
 *    GetValue 
 *
 ****************************************************************************/
static QR_STATUS ReadCFINDMessage( RetData *A_data, RetData *A_root_level_info,
                                   int A_messageid, AppConfig *A_myConfig )
{

    /* 
     * required fields for IMAGE_LEVEL
     */
    if(!strncmp(A_data->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL) - 1))
    {
        if ( GetValue ( A_messageid, MC_ATT_IMAGE_NUMBER,
            A_data->image_level->image_num,
            sizeof ( A_data->image_level->image_num ),
            A_root_level_info->image_level->image_num ) == QR_FAILURE )
            return ( QR_FAILURE ); 

        if ( GetValue ( A_messageid, MC_ATT_SOP_INSTANCE_UID,
            A_data->image_level->sop_inst_uid,
            sizeof ( A_data->image_level->sop_inst_uid ),
            A_root_level_info->image_level->sop_inst_uid ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_RETRIEVE_AE_TITLE, 
            A_data->image_level->retrieveAETitle, 
            sizeof (A_data->image_level->retrieveAETitle), NULL) == QR_NOTAG )
        {
            if (GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_ID,
                A_data->image_level->med_file_id,
                sizeof(A_data->image_level->med_file_id), NULL) != QR_SUCCESS)
                return QR_FAILURE;
            if (GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_UID,
                A_data->image_level->med_file_uid,
                sizeof(A_data->image_level->med_file_uid), NULL) != QR_SUCCESS)
                return QR_FAILURE;
        }
    }

    /* 
     * required fields for SERIES_LEVEL
     */
    else if(!strncmp(A_data->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
    {
        if ( GetValue ( A_messageid, MC_ATT_MODALITY,
            A_data->series_level->modality,
            sizeof ( A_data->series_level->modality ),
            A_root_level_info->series_level->modality ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_SERIES_INSTANCE_UID,
            A_data->series_level->series_inst_uid,
            sizeof ( A_data->series_level->series_inst_uid ),
            A_root_level_info->series_level->series_inst_uid ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_SERIES_NUMBER,
            A_data->series_level->series_num,
            sizeof ( A_data->series_level->series_num ),
            A_root_level_info->series_level->series_num ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_RETRIEVE_AE_TITLE, 
            A_data->series_level->retrieveAETitle, 
            sizeof(A_data->series_level->retrieveAETitle), NULL) == QR_NOTAG)
        {
            GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_ID,
                     A_data->series_level->med_file_id,
                     sizeof(A_data->series_level->med_file_id), NULL);
            GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_UID,
                     A_data->series_level->med_file_uid,
                     sizeof(A_data->series_level->med_file_uid), NULL);
        }
    }

    /*
     * required fields for PATIENT_LEVEL
     */
    else if (!strncmp(A_data->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
    {
        if ( GetValue ( A_messageid, MC_ATT_PATIENTS_NAME,
            A_data->patient_level->patient_name,
            sizeof ( A_data->patient_level->patient_name),
            A_root_level_info->patient_level->patient_name ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_PATIENT_ID, 
            A_data->patient_level->patient_id,
            sizeof ( A_data->patient_level->patient_id ),
            A_root_level_info->patient_level->patient_id ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_RETRIEVE_AE_TITLE, 
            A_data->patient_level->retrieveAETitle, 
            sizeof(A_data->patient_level->retrieveAETitle), NULL) == QR_NOTAG)
        {
            GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_ID,
                     A_data->patient_level->med_file_id,
                     sizeof(A_data->patient_level->med_file_id), NULL);
            GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_UID,
                     A_data->patient_level->med_file_uid,
                     sizeof(A_data->patient_level->med_file_uid),NULL);
        }
    }

    /*
     * required fields for STUDY_LEVEL 
     */
    else /* study level */
    {
        if ( GetValue ( A_messageid, MC_ATT_STUDY_DATE, 
            A_data->study_level->study_date,
            sizeof ( A_data->study_level->study_date ),
            A_root_level_info->study_level->study_date ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_STUDY_TIME, 
            A_data->study_level->study_time,
            sizeof ( A_data->study_level->study_time ),
            A_root_level_info->study_level->study_time ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_ACCESSION_NUMBER, 
            A_data->study_level->accession_num,
            sizeof ( A_data->study_level->accession_num ),
            A_root_level_info->study_level->accession_num ) == QR_FAILURE )
            return ( QR_FAILURE );

        if (strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
        {
            /* 
             * If it is not a STUDY_MODEL query, then the patient's name
             *  goes with the patient level structure
             */
            if ( GetValue ( A_messageid, MC_ATT_PATIENTS_NAME, 
                A_data->patient_level->patient_name,
                sizeof ( A_data->patient_level->patient_name ),
                A_root_level_info->patient_level->patient_name ) == QR_FAILURE )
                return ( QR_FAILURE );

            if ( GetValue ( A_messageid, MC_ATT_PATIENT_ID,
                A_data->patient_level->patient_id,
                sizeof ( A_data->patient_level->patient_id ),
                A_root_level_info->patient_level->patient_id ) == QR_FAILURE )
                return ( QR_FAILURE );
        }

        /*
         * The study model requires that the study_level has the patient name
         *  and patient id
         */
        else if (!strncmp(A_data->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
        {
            if ( GetValue ( A_messageid, MC_ATT_PATIENTS_NAME,
                A_data->study_level->patient_name,
                sizeof ( A_data->study_level->patient_name ),
                A_root_level_info->study_level->patient_name ) == QR_FAILURE )
                return ( QR_FAILURE );

            if ( GetValue ( A_messageid, MC_ATT_PATIENT_ID,
                A_data->study_level->patient_id,
                sizeof ( A_data->study_level->patient_id ),
                A_root_level_info->study_level->patient_id ) == QR_FAILURE )
                return ( QR_FAILURE );
 
        }

        if ( GetValue ( A_messageid, MC_ATT_STUDY_ID,
            A_data->study_level->study_id,
            sizeof ( A_data->study_level->study_id ),
            A_root_level_info->study_level->study_id ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_STUDY_INSTANCE_UID,
            A_data->study_level->study_inst_uid,
            sizeof ( A_data->study_level->study_inst_uid ),
            A_root_level_info->study_level->study_inst_uid ) == QR_FAILURE )
            return ( QR_FAILURE );

        if ( GetValue ( A_messageid, MC_ATT_RETRIEVE_AE_TITLE, 
            A_data->study_level->retrieveAETitle, 
            sizeof(A_data->study_level->retrieveAETitle), NULL) == QR_NOTAG )
        {
            GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_ID,
                     A_data->study_level->med_file_id,
                     sizeof(A_data->study_level->med_file_id), NULL);
            GetValue(A_messageid, MC_ATT_STORAGE_MEDIA_FILE_SET_UID,
                     A_data->study_level->med_file_uid,
                     sizeof(A_data->study_level->med_file_uid), NULL);
        }

    } /* End of STUDY_LEVEL */

    return ( QR_SUCCESS );
} /* ReadCFINDMessage() */


/*****************************************************************************
 *
 * NAME
 *    GetValue - Gets the value from a message for specific tags 
 *
 * ARGUMENTS
 *    A_messageid    int              Id of message the value is being set in
 *    A_tag          unsigned long    Tag of message that is to be set
 *    A_value        char *           Value of tag that is to be set
 *    A_size         int              Size of the buffer to hold the tag value
 *    A_default      char *           Default value of tag, if A_value is NULL
 *
 * DESCRIPTION
 *    Gets the value of a tag from a given message id.
 *
 * RETURNS
 *    QR_SUCCESS or QR_FAILURE
 *
 * SEE ALSO
 *
 ****************************************************************************/
static QR_STATUS GetValue ( int A_messageid, unsigned long A_tag, 
                            char *A_value, int A_size, char *A_default )
{
    MC_STATUS      status;
    static char    S_prefix[] = "GetValue";

    status = MC_Get_Value_To_String ( A_messageid, A_tag, A_size,
                                      A_value );
    if ( status == MC_NULL_VALUE || status == MC_EMPTY_VALUE  ||
         status == MC_INVALID_TAG )
    {
        if (!A_default)
        {
            A_value[0] = '\0';
            return ( QR_NOTAG );
        }
        strcpy ( A_value, A_default );
    }
    else if ( status != MC_NORMAL_COMPLETION )
    {
        PrintErrorMessage ( S_prefix, "MC_Get_Value_To_String", status, NULL );
        printf("***          Tag:  %lX\n", A_tag);
        return ( QR_FAILURE );
    }
    return ( QR_SUCCESS );
} /* GetValue() */



/*****************************************************************************
 *
 * NAME
 *    PrintCFINDResults - Print contents of a CFIND Message  
 *
 * ARGUEMENTS
 *    A_list    RetData *    List of find results to be displayed
 *   
 * DESCRIPTION
 *    This function prints the contents of a data structure with info
 *    from a CFIND Message.  This function is used to display the query
 *    results from a given level when selecting a record.
 *
 * RETURNS
 *    none
 *
 * SEE ALSO
 *    none
 *
 ****************************************************************************/
static void PrintCFINDResults( RetData *A_list )
{
    int    i;
    PRPL   *prpl;
    PRSTL  *prstl;
    PRSL   *prsl;
    PRIL   *pril;

    if (!strncmp(A_list->model, STUDY_MODEL, sizeof(STUDY_MODEL) - 1))
    {
        if (!strncmp(A_list->level, STUDY_LEVEL, sizeof(STUDY_LEVEL) - 1))
        {
            printf("Patient's Name      = [%s]\n",
                A_list->study_level->patient_name);
            printf("Patient ID          = [%s]\n",
                A_list->study_level->patient_id);
        }
    }
    else if(!strncmp(A_list->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
    {
        printf("Patient's Name      = [%s]\n",
            A_list->patient_level->patient_name);
        printf("Patient ID          = [%s]\n",
            A_list->patient_level->patient_id);
    }

    if(!strncmp(A_list->level, IMAGE_LEVEL, sizeof(IMAGE_LEVEL) - 1))
    {
        pril = A_list->image_level;
        printf("      %-33.33s%-20.20s%s\n", "Image Number",
            "SOP Instance UID", "Image Status");
        printf("-----------------------------------------------------"
            "-------------------\n");
        for (i=0; i<A_list->numData; i++)
        {
            printf("%5d: %-32.32s %-20.20s %s\n", i+1,
                pril->image_num, pril->sop_inst_uid, 
                (pril->retrieveAETitle[0] != '\0' ? "ONLINE":"OFFLINE") ); 
            pril = pril->next;
        }
    }
    else if(!strncmp(A_list->level, SERIES_LEVEL, sizeof(SERIES_LEVEL) - 1))
    {
        prsl = A_list->series_level;
        printf("      %-33.33s%-20.20s%s\n", "Modality", "Series Number",
            "Image Status");
        printf("-----------------------------------------------------"
            "-------------------\n");
        for (i=0; i<A_list->numData; i++)
        {
            printf("%5d: %-32.32s %-20.20s %s\n", i+1,
                prsl->modality, prsl->series_num,
                (prsl->retrieveAETitle[0] != '\0' ? "ONLINE":"OFFLINE") ); 
            prsl = prsl->next;
        }
    }
    else if (!strncmp(A_list->level, STUDY_LEVEL, sizeof(STUDY_LEVEL)- 1))
    {
        prstl = A_list->study_level;
        printf("      %-33.33s%-20.20s%s\n", "Study Date", "Study ID", 
            "Image Status");
        printf("-----------------------------------------------------"
            "-------------------\n");
        for (i=0; i<A_list->numData; i++)
        {
            printf("%5d: %-32.32s %-20.20s %s\n", i+1,
                prstl->study_date, prstl->study_id, 
                (prstl->retrieveAETitle[0] != '\0' ? "ONLINE":"OFFLINE") ); 
            prstl = prstl->next;
        }
    }
    else if (!strncmp(A_list->level, PATIENT_LEVEL, sizeof(PATIENT_LEVEL) - 1))
    {
        prpl = A_list->patient_level;
        printf("\n");
        printf("      %-33.33s%-20.20s%s\n", "Patient Name", "Patient ID",
            "Image Status");
        printf("-----------------------------------------------------"
            "-------------------\n");
        for (i=0; i<A_list->numData; i++)
        {
            printf("%5d: %-32.32s %-20.20s %s\n", i+1,
                prpl->patient_name, prpl->patient_id, 
                (prpl->retrieveAETitle[0] != '\0' ? "ONLINE":"OFFLINE") ); 
            prpl = prpl->next;
        }
    }
    else 
        printf("The specified level is not supported,"
               " so the data was not shown.\n");

} /* PrintCFINDResults() */



/*****************************************************************************
 *
 * NAME
 *    MessageToFile - The callback routine for MC_Stream_To_Message
 *
 * ARGUMENTS
 *    A_msgID             int     The message ID
 *    A_CBinformation     void *  The data passed to this function
 *    A_dataSize          int     The data size of the stream data
 *    A_dataBuffer        void *  The data buffer containing the stream data
 *    A_isFirst           int     TRUE when MC3 is providing first data
 *    A_isLast            int     TRUE when MC3 is providing last data
 *
 * DESCRIPTION
 *    MessageToFile is the function which will be called repeatedly to
 *    provide blocks of streamed DICOM message data.  It is the callback
 *    function called by MC_Message_To_Stream.
 *
 * RETURNS
 *    MC_CANNOT_COMPLY
 *    MC_NORMAL_COMPLETION
 *
 * SEE ALSO
 *    none.
 *
 ****************************************************************************/
static MC_STATUS MessageToFile ( int A_msgID, void *A_CBinformation,
                                 int A_dataSize, void *A_dataBuffer,
                                 int A_isFirst, int A_isLast )
{
    CBinfo*      callbackInfo = (CBinfo*)A_CBinformation;
    size_t       count;

    if ( A_isFirst && A_msgID != callbackInfo->messageID )
    {
        printf ( "%s:  Wrong message ID!\n", callbackInfo->prefix );
        return ( MC_CANNOT_COMPLY );
    }

    count = fwrite ( A_dataBuffer, 1, A_dataSize, callbackInfo->stream );

    if ( count != (size_t)A_dataSize )
    {
        printf ( "%s:  fwrite error\n", callbackInfo->prefix );
        return ( MC_CANNOT_COMPLY );
    }

    return ( MC_NORMAL_COMPLETION );
} /* MessageToFile() */

