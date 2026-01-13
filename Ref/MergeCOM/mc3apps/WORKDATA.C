/*************************************************************************
 *
 *       System: MergeCOM-3 Integrator's Tool Kit
 *
 *    $Workfile: workdata.c $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: 
 *               Database functions for Modality Worklist and Modality
 *               Performed Procedure Step SCP "database"
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
    !defined(_ALPHA_OPENVMS) && !defined(OS9_68K) && !defined(UNIX) 
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "mergecom.h" 
#include "mc3msg.h"
#include "mcstatus.h"
#include "diction.h"
#include "qr.h"
#include "workdata.h"

/* Global variables */
LIST G_patientList; /* The complete database tree */

/*****************************************************************************
**
** NAME
**    InsertPatient
**
** SYNOPSIS
**    This function takes a pointer to a patient record, and then inserts a
**    node containing this patient's data onto the patient list.  Also, since
**    this is a newly created patient, a study list is allocated and a pointer
**    to this patient's study list is placed onto this patient's node in the
**    patient list.
**
*****************************************************************************/
int
InsertPatient(
                 PATIENT_REC *A_patientRec
             )
{
    LIST *requestedProcedureList;  /* A pointer to this patient's */
                                   /* requested procedure list.   */

    /*
    ** Obtain a pointer to a list head structure, for creating a 
    ** requested procedure list.
    */
    requestedProcedureList = (LIST*)malloc( sizeof( LIST ) );
    if ( requestedProcedureList == NULL )
    {
        LogError( ERROR_LVL_1, "InsertPatient",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to allocate memory for a requested\n"
                  "procedure list for patientID:  %s\n",
                  A_patientRec->patientID );
        return( FAILURE );
    }

    /*
    ** Now, we want to tell the list management function what size of node
    ** to place on this list.
    */
    if ( LLCreate( requestedProcedureList,
                   sizeof( REQ_PROC_REC ) ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertPatient",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to create head node for a requested "
                  "procedure list for patientID:  %s\n",
                  A_patientRec->patientID );
        return( FAILURE );
    }

    /*
    ** Since, we've now created the requested procedure list for this patient,
    ** we need to place a pointer to it within this patient's patient record.
    */
    A_patientRec->requestedProcedureList = requestedProcedureList;

    /*
    ** Finally, insert the completed node onto the patient list, for this
    ** particular patient.
    */

    LLLast( &G_patientList );
    if ( LLInsert( &G_patientList, A_patientRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertPatient",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to insert a patient record for\n" 
                  "patientID:  %s\n",
                  A_patientRec->patientID );
        return( FAILURE );
    }

    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**    InsertRequestedProc
**
** SYNOPSIS
**    This function finds the patient that this requested procedure is
**    to be inserted for, allocates a scheduled procedure list and a
**    performed procedure list for this patient/requested-procedure-step
**    combniation, and then inserts the data for this node into the
**    patient's requested procedure list.  The patient ID that this
**    requested procedure is for is, of course, passed in as a search
**    parameter.
**
*****************************************************************************/
int
InsertRequestedProc(
                     REQ_PROC_REC *A_requestedProc,
                     char         *A_patientID 
                   )
{
    PATIENT_REC patientRec;
    LIST        *scheduledProcedureStepList;/* A pointer to this requested */
                                            /* procedure's scheduled       */
                                            /* procedure step list.        */
    LIST        *performedProcedureStepList;/* A pointer to this requested */
                                            /* procedure's performed       */
                                            /* procedure step list.        */
    int         found = FALSE;

    /*
    ** First things first:  we need to find the patient that this particular
    ** study is being created for.  If the patient doesn't exist in the patient
    ** list, then we have problems.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    ** We match on patient ID, since it is the key to the patient record.
    */
    while( LLPopNode( &G_patientList, &patientRec )  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "insertStudy",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  "patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Obtain a pointer to a list head structure, for creating a scheduled
    ** procedure step list.
    */
    scheduledProcedureStepList = (LIST*)malloc( sizeof( LIST ) );
    if ( scheduledProcedureStepList == NULL )
    {
        LogError( ERROR_LVL_1, "insertStudy",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to allocate memory for a scheduled "
                  "procedure step list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Now, we want to tell the list management function what size of node
    ** to place on this list.
    */
    if ( LLCreate( scheduledProcedureStepList,
                   sizeof( SCH_PROC_STEP_REC ) ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "insertStudy",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to create a head node for a scheduled "
                  "procedure step list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Now, we need to create the head list stuff for the performed procedure
    ** step list.
    */
    performedProcedureStepList = (LIST*)malloc( sizeof( LIST ) );
    if ( performedProcedureStepList == NULL )
    {
        LogError( ERROR_LVL_1, "insertStudy",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to allocate memory for a performed "
                  "procedure step list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Now, we want to tell the list management function what size of node
    ** to place on this list.
    */
    if ( LLCreate( performedProcedureStepList,
                   sizeof( PERF_PROC_STEP_REC ) ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "insertStudy",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to create a head node for a performed "
                  "procedure step list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Since, we've now created both scheduled and performed procedure step
    ** lists for this patient/requested-procedure-step, we need to place the
    ** pointers to them within this patient's patient/requested-procedure-step
    ** record.
    */
    A_requestedProc->scheduledProcedureStepList = scheduledProcedureStepList;
    A_requestedProc->performedProcedureStepList = performedProcedureStepList;

    /*
    ** Finally, insert the completed node onto the patient list, for this
    ** particular patient.
    */

    LLLast( patientRec.requestedProcedureList );
    if ( LLInsert( patientRec.requestedProcedureList,
                   A_requestedProc ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "insertStudy",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to insert a requested procedure "
                  "record for\n"
                  "patient ID:  %s, requested procedure:  %s\n", A_patientID,
                  A_requestedProc->requestedProcID );
        return( FAILURE );
    }

    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**    InsertScheduledProc
**
** SYNOPSIS
**    This function finds the patient and requested-procedure combination
**    that this scheduled procedure is to be inserted for and then inserts
**    the data for this node into the patient's requested-procedure-step's
**    scheduled-procedure-step list.  The patient ID and requested procedure
**    ID are key values that are passed into this function as search criteria.
**
*****************************************************************************/
int
InsertScheduledProc(
                  SCH_PROC_STEP_REC *A_schProcStepRec,
                  char              *A_patientID,
                  char              *A_requestedProcID
            )
{
    PATIENT_REC  patientRec;
    REQ_PROC_REC reqProcRec;
    int         found = FALSE;

    /*
    ** First things first:  we need to find the patient and the requested
    ** procedure ID that this particular scheduled procedure is being
    ** created for.  If the patient/requested-procedure-ID doesn't exist in
    ** the patient list, then we have aproblem.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    */
    while( LLPopNode( &G_patientList, &patientRec )  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertScheduledProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  " patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    found = FALSE; /* Reset our found "flag" */
    /*
    ** We've found the first part of the key, now we need to find the
    ** correct requested procedure ID.
    */
    LLRewind( patientRec.requestedProcedureList );
    while( LLPopNode( patientRec.requestedProcedureList,
                      &reqProcRec ) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( reqProcRec.requestedProcID, A_requestedProcID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertScheduledProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a requested procedure "
                  "step record for\n"
                  " patient ID:  %s, requested procedure ID:  %s\n",
                  A_patientID, A_requestedProcID );
        return( FAILURE );
    }

    /*
    ** OK, now we've found the patient and the corresponding requested
    ** procedure ID.  Now we need to insert the scheduled procedure
    ** information onto the list.  Since this is a "leaf-node" and the head
    ** structure of the scheduled procedure step list was created when the
    ** requested procedure node was created for a patient, we just need
    ** to insert this node into the structure.
    */
    LLLast( reqProcRec.scheduledProcedureStepList );
    if ( LLInsert( reqProcRec.scheduledProcedureStepList,
                   A_schProcStepRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertScheduledProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to insert a scheduled "
                  "procedure step record\n"
                  "for patient ID:  %s\n"
                  "study:  %s\n",
                  "scheduled procedure step ID:  %s\n",
                  A_patientID, A_requestedProcID,
                  A_schProcStepRec->scheduledProcStepID );
        return( FAILURE );
    }

    return( SUCCESS );

}


/*****************************************************************************
**
** NAME
**    InsertPerfProc
**
** SYNOPSIS
**    This function finds the patient/requested-procedure-step combination
**    that this performed procedure step is to be inserted for, and then
**    inserts the data for this node into the patient's
**    requested-procedure-step's performed-procedure step list.  The patient
**    ID and requested procedure ID are the key values and need to be
**    passed in, as search criteria.
**
*****************************************************************************/
int
InsertPerfProc(
                 PERF_PROC_STEP_REC *perfProcStepRec,
                 char               *A_patientID,
                 char               *A_requestedProcID
              )
{
    PATIENT_REC  patientRec;
    REQ_PROC_REC reqProcRec;
    LIST         *performedSeriesList;
    LIST         *imageList;
    int          found = FALSE;

    /*
    ** First things first:  we need to find the patient/requested-procedure-
    ** step that this particular performed procedure step is being created for.
    ** If the patient/requested-procedure-step doesn't exist in the patient
    ** list, then we have problems.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    */
    while( LLPopNode( &G_patientList, &patientRec )  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  " patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    found = FALSE; /* reset our "found" flag */
    /*
    ** We've found the first part of the key, now we need to find the
    ** correct requested procedure ID.
    */
    LLRewind( patientRec.requestedProcedureList );
    while( LLPopNode( patientRec.requestedProcedureList,
                      &reqProcRec ) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( reqProcRec.requestedProcID, A_requestedProcID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a requested procdure step "
                  "record for\n"
                  " patient ID:  %s, requested procedure ID:  %s\n",
                  A_patientID, A_requestedProcID );
        return( FAILURE );
    }

    /*
    ** A performed procedure step node contains two additional lists.
    ** a performed series list, and an image list.  We need to allocate
    ** those particular lists for this performed procedure step node.
    */
    performedSeriesList = (LIST*)malloc( sizeof( LIST ) );
    if ( performedSeriesList == NULL )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to allocate memory for a performed "
                  "series step list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Now, we want to tell the list management function what size of node
    ** to place on this list.
    */
    if ( LLCreate( performedSeriesList,
                   sizeof( PERF_SERIES_REC ) ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to create a head node for a performed "
                  "series list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** and, then initialize the image list...
    */
    imageList = (LIST*)malloc( sizeof( LIST ) );
    if ( imageList == NULL )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to allocate memory for an "
                  "image list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Now, we want to tell the list management function what size of node
    ** to place on this list.
    */
    if ( LLCreate( imageList, sizeof( IMAGE_REC ) ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to create a head node for an image "
                  "list\n"
                  "for patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    /*
    ** Put the pointers to the newly created, empty lists into the performed
    ** procedure step structure node.
    */
    perfProcStepRec->performedSeriesList = performedSeriesList;
    perfProcStepRec->imageList = imageList;

    /*
    ** Finally, insert the completed node onto the requested procedure list,
    ** for this particular patient/requested-procedure-step.
    */
    LLLast( reqProcRec.performedProcedureStepList );
    if ( LLInsert( reqProcRec.performedProcedureStepList,
                   perfProcStepRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertPerfProc",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to insert a performed procedure ste "
                  "for\n"
                  " patient ID:  %s, requested procedure ID:  %s\n",
                  A_patientID, A_requestedProcID );
        return( FAILURE );
    }

    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**    InsertImage
**
** SYNOPSIS
**    This function finds the patient/requested-procedure-step/
**    performed-procedure-step combination that this image is to be inserted
**    for, and then inserts the data for this node into the patient's
**    requested-procedure-step's performed-procedure-step's image list.
**    The patient name, requested procedure ID, and performed procedure ID
**    are key values that are passed into this function as search criteria.
**
*****************************************************************************/
int
InsertImage(
                 IMAGE_REC *imageRec,
                 char      *A_patientID,
                 char      *A_requestedProcID,
                 char      *performedProcStepUID
           )
{
    PATIENT_REC        patientRec;
    REQ_PROC_REC       reqProcRec;
    PERF_PROC_STEP_REC perfProcStepRec;
    int                found = FALSE;

    /*
    ** First things first:  we need to find the patient/requested-procedure-
    ** step/performed-procedure-step that this particular image is being
    ** created for.  If the patient/requested-procedure-step/performed-
    ** procedure-step doesn't exist in the patient list, then we have
    ** a problem.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    */
    while( LLPopNode( &G_patientList, &patientRec )  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertImage",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  " patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    found = FALSE; /* reset our "found" flag */
    /*
    ** We've found the first part of the key, now we need to find the
    ** next part.
    */
    LLRewind( patientRec.requestedProcedureList );
    while( LLPopNode( patientRec.requestedProcedureList,
                      &reqProcRec ) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( reqProcRec.requestedProcID, A_requestedProcID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertImage",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a requested procedure "
                  "record for\n"
                  "patient:  %s, requested procedure ID:  %s\n",
                  A_patientID,
                  A_requestedProcID );
        return( FAILURE );
    }

    found = FALSE; /* reset our "found" flag */
    /*
    ** Finally, we need to find the last part of the key.
    */
    LLRewind( reqProcRec.performedProcedureStepList );
    while( LLPopNode( reqProcRec.performedProcedureStepList,
                      &perfProcStepRec ) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( perfProcStepRec.performedProcStepUID,
                     performedProcStepUID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertImage",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a performed procedure step "
                  "record for\n"
                  "patient:  %s\n"
                  "requested procedure step ID:  %s\n"
                  "performed procedure step ID:  %s\n",
                  A_patientID, A_requestedProcID, performedProcStepUID );
        return( FAILURE );
    }

    found = FALSE; /* reset our "found" flag */
    /*
    ** Finally, insert the completed node onto the image list, for this
    ** particular patient/requested-procedure-step/performed-procedure-step.
    */
    LLLast( perfProcStepRec.imageList );
    if ( LLInsert( perfProcStepRec.imageList, imageRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertImage",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to insert an image record for\n"
                  "patient:  %s\n"
                  "requested procedure step ID:  %s\n"
                  "performed procedure step ID:  %s\n",
                  A_patientID, A_requestedProcID, performedProcStepUID );
        return( FAILURE );
    }

    return( SUCCESS );

}


/*****************************************************************************
**
** NAME
**    InsertPerfSeries
**
** SYNOPSIS
**    This function finds the patient/requested-procedure-step/
**    performed-procedure-step combination that this series is to be
**    inserted for, and then inserts the data for this node into the
**    patient's requested-procedure-step's performed-procedure-step's
**    performed series list.  The patient name, requested procedure ID, and
**    performed procedure ID are key values that are passed into this function
**    as search criteria.
**
*****************************************************************************/
int
InsertPerfSeries(
                   PERF_SERIES_REC *A_perfSeriesRec,
                   char            *A_patientID,
                   char            *A_requestedProcID,
                   char            *A_performedProcStepUID
                )
{
    PATIENT_REC        patientRec;
    REQ_PROC_REC       reqProcRec;
    PERF_PROC_STEP_REC perfProcStepRec;
    int                found = FALSE;

    /*
    ** First things first:  we need to find the patient/requested-procedure-
    ** step/performed-procedure-step that this particular performed series
    ** is being created for.  If the patient/requested-procedure-step/
    ** performed-procedure-step doesn't exist in the patient list, then we have
    ** a problem.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    */
    while( LLPopNode( &G_patientList, &patientRec )  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertPerfSeries",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  " patient ID:  %s\n", A_patientID );
        return( FAILURE );
    }

    found = FALSE; /* reset our "found" flag */
    /*
    ** We've found the first part of the key, now we need to find the
    ** next part.
    */
    LLRewind( patientRec.requestedProcedureList );
    while( LLPopNode( patientRec.requestedProcedureList,
                      &reqProcRec ) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( reqProcRec.requestedProcID, A_requestedProcID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertPerfSeries",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a requested procedure "
                  "record for\n"
                  "patient ID:  %s, requested procedure ID:  %s\n",
                  A_patientID,
                  A_requestedProcID );
        return( FAILURE );
    }

    found = FALSE; /* reset our "found" flag */
    /*
    ** Finally, we need to find the last part of the key.
    */
    LLRewind( reqProcRec.performedProcedureStepList );
    while( LLPopNode( reqProcRec.performedProcedureStepList,
                      &perfProcStepRec ) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( perfProcStepRec.performedProcStepUID,
                     A_performedProcStepUID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "InsertPerfSeries",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a performed procedure step "
                  "record for\n"
                  "patient ID:  %s\n"
                  "requested procedure step ID:  %s\n"
                  "performed procedure step ID:  %s\n",
                  A_patientID, A_requestedProcID, A_performedProcStepUID );
        return( FAILURE );
    }

    /*
    ** Finally, insert the completed node onto the image list, for this
    ** particular patient/requested-procedure-step/performed-procedure-step.
    */
    LLLast( perfProcStepRec.performedSeriesList );
    if ( LLInsert( perfProcStepRec.performedSeriesList,
                   A_perfSeriesRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "InsertPerfSeries",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to insert a performed series "
                  " record for\n"
                  "patient ID:  %s\n"
                  "requested procedure step ID:  %s\n"
                  "performed procedure step ID:  %s\n",
                  A_patientID, A_requestedProcID, A_performedProcStepUID );
        return( FAILURE );
    }

    return( SUCCESS );

}


/*****************************************************************************
**
** NAME
**     CreateDummyPatient
**
** SYNOPSIS
**     Creates "dummy" entries in the "database" for a patient who didn't
**     have a worklist created for their imaging procedure.
**
*****************************************************************************/
void
CreateDummyPatient(
                      int  A_messageID,
                      char *A_ID
                  )
{
    PATIENT_REC       patientRec;
    REQ_PROC_REC      reqProcRec;
    SCH_PROC_STEP_REC schProcStepRec;

    /*
    ** We can only depend on type 1 values being present in the N_CREATE
    ** request message.  Therefore, create the dummy patient, requested
    ** procedure, and scheduled procedure based only on type 1 attributes.
    ** There are no type 1 attributes that are part of PATIENT_REC.
    */
    sprintf( patientRec.patientID, "%s", A_ID );
    sprintf( patientRec.patientName, "DUMMY%s", A_ID );
    strcpy( patientRec.patientBirthDate, "" );
    strcpy( patientRec.patientSex, "" );
    strcpy( patientRec.patientWeight, "" );
    patientRec.requestedProcedureList = NULL;

    /*
    ** There are no type 1 attributes that are part of REQ_PROC_REC.
    */
    sprintf( reqProcRec.requestedProcID, "%s", A_ID );
    sprintf( reqProcRec.requestedProcDescription, "DUMMY%s", A_ID );
    strcpy( reqProcRec.accessionNumber, "" );
    strcpy( reqProcRec.requestingPhysician, "" );
    strcpy( reqProcRec.referringPhysician, "" );
    strcpy( reqProcRec.requestedProcPriority, "" );
    strcpy( reqProcRec.studyID, "" );
    strcpy( reqProcRec.studyInstanceUID, "" );
    reqProcRec.scheduledProcedureStepList = NULL;
    reqProcRec.performedProcedureStepList = NULL;

    /*
    ** There are no type 1 attributes that are part of SCH_PROC_STEP_REC.
    */
    sprintf( schProcStepRec.scheduledProcStepID, "%s", A_ID );
    sprintf( schProcStepRec.scheduledProcStepDescription, "DUMMY%s", A_ID );
    strcpy( schProcStepRec.scheduledAETitle, "" );
    strcpy( schProcStepRec.scheduledProcStepLocation, "" );
    strcpy( schProcStepRec.scheduledStartDate, "" );
    strcpy( schProcStepRec.scheduledStartTime, "" );
    strcpy( schProcStepRec.modality, "" );
    strcpy( schProcStepRec.scheduledPerformPhysicianName, "" );
    schProcStepRec.complete = FALSE;
    schProcStepRec.performedProcedureStepList = NULL;

    /*
    ** ...and then insert our dummy structures into the database.
    */
    if ( InsertPatient( &patientRec ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "CreateDummyPatient", __LINE__,
                  MC_NORMAL_COMPLETION,
                  "Unable to insert a dummy patient record into the "
                  "database.\n" );
    }

    if ( InsertRequestedProc( &reqProcRec, patientRec.patientID ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "CreateDummyPatient", __LINE__,
                  MC_NORMAL_COMPLETION,
                  "Unable to insert a dummy requested procedure record into "
                  "the database.\n" );
    }

    if ( InsertScheduledProc( &schProcStepRec,
                              patientRec.patientID,
                              reqProcRec.requestedProcID ) != SUCCESS )
    {
        LogError( ERROR_LVL_1, "CreateDummyPatient", __LINE__,
                  MC_NORMAL_COMPLETION,
                  "Unable to insert a dummy scheduled procedure record into "
                  "the database.\n" );
    }
}


/*****************************************************************************
**
** NAME
**    dumpTree
**
** SYNOPSIS
**    This function will trapse through every node on every imbedded list
**    to print out the key values that may be placed there.  This is mostly
**    a debugging tool that is useful to use when the contents of the entire
**    linked-list "database" needs to be displayed.
**    This function is used via the "-u" command line paramater, which will
**    dump the contents of the database after every MPPS operation.
**
**    The search is accomplished in a deph-first manner.
**
*****************************************************************************/
void
DumpTree(
        )
{
    PATIENT_REC        patientRec;
    REQ_PROC_REC       reqProcRec;
    SCH_PROC_STEP_REC  schProcStepRec;
    PERF_PROC_STEP_REC perfProcStepRec;
    PERF_SERIES_REC    perfSeriesRec;
    IMAGE_REC          imageRec;

    /*
    ** The first step in this process is to rewind the patient list so that
    ** we are starting at the root of the structure.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, dig into the requested
    ** procedure record.
    */
    while( LLPopNode( &G_patientList, &patientRec ) != NULL )
    {
        /*
        ** We've found a node, so we echo out the key information.
        */
        printf( "Patient ID:  %s\n", patientRec.patientID );

        /*
        ** ...and then trapse through the requested procedures for this
        ** particular patient.
        */
        LLRewind( patientRec.requestedProcedureList );
        while( LLPopNode( patientRec.requestedProcedureList,
                          &reqProcRec ) != NULL )
        {
            printf( "    Requested Procedure ID:  %s\n",
                    reqProcRec.requestedProcID );

            /*
            ** ...and then trapse through the performed procedure for this
            ** particular patient/requested-procedure
            */
            LLRewind( reqProcRec.performedProcedureStepList );
            while( LLPopNode( reqProcRec.performedProcedureStepList,
                              &perfProcStepRec ) != NULL )
            {
                printf( "        Performed Procedure Step UID:  %s\n",
                        perfProcStepRec.performedProcStepUID );

                /*
                ** ...and then trapse through the image components for this
                ** particular patient/requested-procedure-step/performed-
                ** procedure-step instances.
                */
                LLRewind( perfProcStepRec.imageList );
                while( LLPopNode( perfProcStepRec.imageList,
                                  &imageRec ) != NULL )
                {
                    printf( "            SOP instance UID:  %s\n",
                            imageRec.sopInstanceUID );
                } /* end of image while loop */

                /*
                ** Since performed procedure step's also have performed
                ** series lists at the same level as image lists, we also
                ** list the performed series' key values out.
                */
                LLRewind( perfProcStepRec.performedSeriesList );
                while( LLPopNode( perfProcStepRec.performedSeriesList,
                                  &perfSeriesRec ) != NULL )
                {
                    printf( "            Series instance UID:  %s\n",
                            perfSeriesRec.seriesInstanceUID );
                } /* end of performed series while loop */

            } /* end of performed procedure step while loop */

            /*
            ** Since scheduled procedure step is at the same level as
            ** performed procedure step, we do that now.
            */
            LLRewind( reqProcRec.scheduledProcedureStepList );
            while( LLPopNode( reqProcRec.scheduledProcedureStepList,
                              &schProcStepRec ) != NULL )
            {
                printf( "        Scheduled Procedure Step ID:  %s\n",
                        schProcStepRec.scheduledProcStepID );
            } /* end of scheduled procedure step while loop */

        } /* end of requested procedure while loop */

    } /* end of patient while loop */
}


/*****************************************************************************
**
** NAME
**    FreeTree
**
** SYNOPSIS
**    This function will search through the entire linked-list database
**    structure in a depth-first manner, and free the memory that has been
**    allocated for each node of the linked list.  This is important because
**    the normal "LLDestroy" linked list function WILL NOT free memory that
**    may have been allocated and "placed onto" a node.  Keep in mind that
**    some of the nodes in the "database" contain LIST's of other nodes.
**
*****************************************************************************/
void
FreeTree(
        )
{
    PATIENT_REC        patientRec;
    REQ_PROC_REC       reqProcRec;
    PERF_PROC_STEP_REC perfProcStepRec;

    /*
    ** The first step in this process is to rewind the patient list so that
    ** we are starting at the root of the structure.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, dig into the requested
    ** procedure record.
    */
    while( LLPopNode( &G_patientList, &patientRec ) != NULL )
    {
        /*
        ** ...and then trapse through the requested procedures for this
        ** particular patient.
        */
        LLRewind( patientRec.requestedProcedureList );
        while( LLPopNode( patientRec.requestedProcedureList,
                          &reqProcRec ) != NULL )
        {
            /*
            ** ...and then trapse through the performed procedure for this
            ** particular patient/requested-procedure
            */
            LLRewind( reqProcRec.performedProcedureStepList );
            while( LLPopNode( reqProcRec.performedProcedureStepList,
                              &perfProcStepRec ) != NULL )
            {
                /*
                ** Finally, we can destroy the image list, since it
                ** doesn't contain any other embedded linked lists.
                */
                LLDestroy( perfProcStepRec.imageList );

                /*
                ** And, since the "Insert" functions also created the head
                ** node of the image list, we must free it.
                */
                free( perfProcStepRec.imageList );

                /*
                ** Since performed procedure step's also have performed
                ** series lists at the same level as image lists, we also
                ** free and destroy the performed series list
                */
                LLDestroy( perfProcStepRec.performedSeriesList );
                free( perfProcStepRec.performedSeriesList );

            } /* end of performed procedure step while loop */

            /*
            ** Now that the lists contained within the performed procedure
            ** step linked list have been freed, we can destroy and free the
            ** performed procedure list, itself.
            */
            LLDestroy( reqProcRec.performedProcedureStepList );
            free( reqProcRec.performedProcedureStepList );

            /*
            ** Since scheduled procedure step is at the same level as
            ** performed procedure step, we free and destroy it now.
            */
            LLDestroy( reqProcRec.scheduledProcedureStepList );
            free( reqProcRec.scheduledProcedureStepList );

        } /* end of requested procedure while loop */

        /*
        ** Once the requested procedure list has had all of its contained
        ** linked lists destroyed and freed, we can destroy and free it.
        */
        LLDestroy( patientRec.requestedProcedureList );
        free( patientRec.requestedProcedureList );

    } /* end of patient while loop */

    /*
    ** And finally, we can destroy the patient list, but we can't free the
    ** head node structure.  It wasn't allocated, but rather was declared as
    ** a global!
    */
    LLDestroy( &G_patientList );
}

/* The following functions are used to parse the work.dat data file */
/*****************************************************************************
**
** NAME
**    readDat
**
** SYNOPSIS
**    This function reads the worklist data file, and then calls the various
**    "Insert" functions to place the worklist information into the linked-
**    list database.  Since the entire database used for the work_scp is
**    contained within memory, it is built from the datafile upon each
**    successive run of the work_scp.
**
*****************************************************************************/
int
ReadDat(
           char *A_datFile
       )
{
    FILE *infile = NULL;
    char line[80];
    char read = '\0';
    int  x = 0;
    PATIENT_REC   patientRec;        /* A structure containing patient       */
                                     /* information                          */
    REQ_PROC_REC  reqProcRec;        /* A structure containing requested     */
                                     /* procedure step information           */
    SCH_PROC_STEP_REC schProcStepRec;/* A structure containing scheduled     */
                                     /* procedure step information           */
    PERF_PROC_STEP_REC perfProcStepRec; /* A structure containing performed  */
                                        /* procedure step information        */
    PERF_SERIES_REC perfSeriesRec;   /* A structure containing performed     */
                                     /* series information                   */
    IMAGE_REC     imageRec;          /* A structure containing image         */
                                     /* information                          */
    /*
    ** The following variables hold the keys to each of the individual
    ** structures that items are being inserted for.  If a key isn't
    ** defined yet, the user will be told of their error.
    */
    char currentPatientID[ LO_LEN ]; /* The ID of the "current" patient      */
    char currentReqProcID[ SH_LEN ]; /* The ID of the "current" requested    */
                                     /* procedure                            */
    char currentSchProcID[ SH_LEN ]; /* The ID of the "current" scheduled    */
                                     /* procedure step                       */
    char currentPerProUID[ UI_LEN ]; /* The UID of the "current" performed   */
                                     /* procedure step                       */

    /*
    ** We set the working keys to a value that means that each key hasn't
    ** been set yet.  This forms a primitive error check on the key value
    ** so that a user can't insert a record for a non-existant key.
    */
    strcpy( currentPatientID, "BOGUS-KEY" );
    strcpy( currentReqProcID, "BOGUS-KEY" );
    strcpy( currentSchProcID, "BOGUS-KEY" );
    strcpy( currentPerProUID, "BOGUS-KEY" );

    infile = fopen( A_datFile, "r" );

    if ( infile == NULL )
        return( FAILURE );
    
    while( !feof( infile ) )
    {
        Readln( infile, line );
        if ( feof( infile ) )
            break;
        if ( line[0] == '#' )
        {
            /*
            ** Found a comment line:  ignore it.
            */
            read = '\0';
            x=0;
        }
        else if ( line[0] == '\0' )
        {
            /*
            ** Found a blank line:  ignore it.
            */
            read = '\0';
            x=0;
        }
        else
        {
            if ( strcmp( line, "PATIENT" ) == 0 )
            {
                /*
                ** Since we've found a PATIENT keyword, the next 5 lines
                ** in the file must be part of a PATIENT structure.
                */
                strcpy( patientRec.patientID, Readln( infile, line ) );
                strcpy( patientRec.patientName, Readln( infile, line ) );
                strcpy( patientRec.patientBirthDate, Readln( infile, line ) );
                strcpy( patientRec.patientSex, Readln( infile, line ) );
                strcpy( patientRec.patientWeight, Readln( infile, line ) );

                /*
                ** Since we've found a patient record, apply all following
                ** inserts to this patient, 'till another patient record is
                ** found.
                */
                strcpy( currentPatientID, patientRec.patientID );

                /*
                ** And then attempt to insert the patient record.
                */
                if ( InsertPatient( &patientRec ) != SUCCESS )
                {
                    LogError( ERROR_LVL_1, "readDat",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Couldn't insert patient ID:  %s\n",
                              patientRec.patientID );
                    return( FAILURE );
                }
            }
            else if ( strcmp( line, "REQPROC" ) == 0 )
            {
                /*
                ** Since we've found a REQPROC keyword, the next 8 lines
                ** in the file must be part of a REQPROC structure.
                */
                strcpy( reqProcRec.accessionNumber, Readln( infile, line ) );
                strcpy( reqProcRec.requestingPhysician,
                        Readln( infile, line ) );
                strcpy( reqProcRec.referringPhysician, Readln( infile, line ) );
                strcpy( reqProcRec.requestedProcID, Readln( infile, line ) );
                strcpy( reqProcRec.requestedProcDescription,
                        Readln( infile, line ) );
                strcpy( reqProcRec.requestedProcPriority,
                        Readln( infile, line ) );
                strcpy( reqProcRec.studyID, Readln( infile, line ) );
                strcpy( reqProcRec.studyInstanceUID, Readln( infile, line ) );

                /*
                ** Check to see that we have entered a patient that this
                ** requested procedure is for.
                */
                if ( strcmp( currentPatientID, "BOGUS-KEY" ) == 0 )
                {
                    LogError( ERROR_LVL_1, "readDat",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Attempted to insert a requested procedure ID:  "
                              "%s\nwhen a patient hasn't been inserted yet\n",
                              reqProcRec.requestedProcID );
                    return( FAILURE );
                }

                /*
                ** Since we've validated that a patient has already been
                ** inserted, we now set the current requested procedure ID
                ** to match the one for this record.
                */
                strcpy( currentReqProcID, reqProcRec.requestedProcID );

                /*
                ** ...and then attempt to insert this requested procedure.
                */
                if ( InsertRequestedProc( &reqProcRec,
                                          currentPatientID ) != SUCCESS )
                {
                    LogError( ERROR_LVL_1, "readDat",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Couldn't insert requested procedure ID:  %s\n",
                              reqProcRec.requestedProcID );
                    return( FAILURE );
                }
            }
            else if ( strcmp( line, "SCHPROC" ) == 0 )
            {
                /*
                ** Since we've found a SCHPROC keyword, the next 9 lines
                ** in the file must be part of a SCHPROC structure.
                */
                strcpy( schProcStepRec.scheduledAETitle,
                        Readln( infile, line ) );
                strcpy( schProcStepRec.scheduledProcStepLocation,
                        Readln( infile, line ) );
                strcpy( schProcStepRec.scheduledStartDate,
                        Readln( infile, line ) );
                strcpy( schProcStepRec.scheduledStartTime,
                        Readln( infile, line ) );
                strcpy( schProcStepRec.modality, Readln( infile, line ) );
                strcpy( schProcStepRec.scheduledPerformPhysicianName,
                        Readln( infile, line ) );
                strcpy( schProcStepRec.scheduledProcStepDescription,
                        Readln( infile, line ) );
                strcpy( schProcStepRec.scheduledProcStepID,
                        Readln( infile, line ) );
                schProcStepRec.complete = strtol( Readln( infile, line ),
                                                  NULL, 10 );

                /*
                ** We must validate that a patient and a requested procedure
                ** step have been inserted.
                */
                if ( ( strcmp( currentPatientID, "BOGUS-KEY" ) == 0 ) ||
                     ( strcmp( currentReqProcID, "BOGUS-KEY" ) == 0 ) )
                {
                    LogError( ERROR_LVL_1, "readDat",
                              __LINE__, MC_NORMAL_COMPLETION,
                             "Attempted to insert scheduled procedure ID:  %s\n"
                             "when a patient or a requested procedure hasn't "
                             "been inserted yet.\n",
                             schProcStepRec.scheduledProcStepID );
                    return( FAILURE );
                }

                /*
                ** Since we've validated that key values for this insert have
                ** been met, we can also define this current record for future
                ** inserts...
                */
                strcpy( currentSchProcID, schProcStepRec.scheduledProcStepID );

                /*
                ** ...and then attempt to insert this record.
                */
                if ( InsertScheduledProc( &schProcStepRec,
                                          currentPatientID,
                                          currentReqProcID ) != SUCCESS )
                {
                    LogError( ERROR_LVL_1, "readDat",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Couldn't insert scheduled procedure ID:  %s\n",
                             schProcStepRec.scheduledProcStepID );
                    return( FAILURE );
                }
            }
            else if ( strcmp( line, "PERPROC" ) == 0 )
            {
                /*
                ** Since we've found a PERPROC keyword, the next 15 lines
                ** in the file must be part of a PERPROC structure.
                */
                strcpy( perfProcStepRec.performedProcStepUID,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepID,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedAETitle,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedStationName,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedLocation,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepStartDate,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepStartTime,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepStatus,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepDescription,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcTypeDescription,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepEndDate,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.performedProcStepEndTime,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.modality, Readln( infile, line ) );
                strcpy( perfProcStepRec.scheduledProcStepDescription,
                        Readln( infile, line ) );
                strcpy( perfProcStepRec.scheduledProcStepID,
                        Readln( infile, line ) );

                /*
                ** We must validate that a patient and a requested procedure
                ** step have been inserted.
                */
                if ( ( strcmp( currentPatientID, "BOGUS-KEY" ) == 0 ) ||
                     ( strcmp( currentReqProcID, "BOGUS-KEY" ) == 0 ) )
                {
                    LogError( ERROR_LVL_1, "InsertPerfSeries",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Attempted to insert performed "
                              "procedure UID:  %s\n"
                              "when a patient or a requested procedure hasn't "
                              "been inserted yet.\n",
                              perfProcStepRec.performedProcStepUID );
                    return( FAILURE );
                }

                /*
                ** Since we've validated that key values for this insert have
                ** been met, we can also define this current record for future
                ** inserts...
                */
                strcpy( currentPerProUID,
                        perfProcStepRec.performedProcStepUID );

                /*
                ** ...and then attempt to insert this record.
                */
                if ( InsertPerfProc( &perfProcStepRec,
                                     currentPatientID,
                                     currentReqProcID ) != SUCCESS )
                {
                    LogError( ERROR_LVL_1, "InsertPerfSeries",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Couldn't insert performed procedure UID:  %s\n",
                              perfProcStepRec.performedProcStepUID );
                    return( FAILURE );
                }
            }
            else if ( strcmp( line, "PERSERI" ) == 0 )
            {
                /*
                ** Since we've found a PERSERI keyword, the next 6 lines
                ** in the file must be part of a PERSERI structure.
                */
                strcpy( perfSeriesRec.performingPhysicianName,
                        Readln( infile, line ) );
                strcpy( perfSeriesRec.protocolName, Readln( infile, line ) );
                strcpy( perfSeriesRec.operatorName, Readln( infile, line ) );
                strcpy( perfSeriesRec.seriesInstanceUID,
                        Readln( infile, line ) );
                strcpy( perfSeriesRec.seriesDescription,
                        Readln( infile, line ) );
                strcpy( perfSeriesRec.retrieveAETitle, Readln( infile, line ) );

                /*
                ** We must validate that a patient, requested procedure step,
                ** and performed procedure step have been inserted.
                */
                if ( ( strcmp( currentPatientID, "BOGUS-KEY" ) == 0 ) ||
                     ( strcmp( currentReqProcID, "BOGUS-KEY" ) == 0 ) ||
                     ( strcmp( currentPerProUID, "BOGUS-KEY" ) == 0 ) )
                {
                    LogError( ERROR_LVL_1, "InsertPerfSeries",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Attempted to insert performed "
                              "series UID:  %s\n"
                              "when a patient, requested procedure, or "
                              "performed procedure step UID hasn't "
                              "been inserted yet.\n",
                              perfSeriesRec.seriesInstanceUID );
                    return( FAILURE );
                }

                /*
                ** Since we've validated that key values for this insert have
                ** been met, we can insert this structure, since it's a leaf
                ** node.
                */
                if ( InsertPerfSeries( &perfSeriesRec,
                                       currentPatientID,
                                       currentReqProcID,
                                       currentPerProUID ) != SUCCESS )
                {
                    LogError( ERROR_LVL_1, "InsertPerfSeries",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Couldn't insert performed series "
                              "UID:  %s\n",
                              perfSeriesRec.seriesInstanceUID );
                    return( FAILURE );
                }
            }
            else if ( strcmp( line, "IMAGERE" ) == 0 )
            {
                /*
                ** Since we've found a IMAGERE keyword, the next 2 lines
                ** in the file must be part of a IMAGERE structure.
                */
                strcpy( imageRec.sopInstanceUID, Readln( infile, line ) );
                strcpy( imageRec.sopClassUID, Readln( infile, line ) );

                /*
                ** We must validate that a patient, requested procedure step,
                ** and performed procedure step have been inserted.
                */
                if ( ( strcmp( currentPatientID, "BOGUS-KEY" ) == 0 ) ||
                     ( strcmp( currentReqProcID, "BOGUS-KEY" ) == 0 ) ||
                     ( strcmp( currentPerProUID, "BOGUS-KEY" ) == 0 ) )
                {
                    LogError( ERROR_LVL_1, "InsertPerfSeries",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Attempted to issert an image SOP "
                              "UID:  %s\n"
                              "when a patient, requested procedure, or "
                              "performed procedure step UID hasn't "
                              "been inserted yet.\n",
                              imageRec.sopInstanceUID );
                    return( FAILURE );
                }

                /*
                ** Since we've validated that key values for this insert have
                ** been met, we can insert this structure, since it's a leaf
                ** node.
                */
                if ( InsertImage( &imageRec,
                                  currentPatientID,
                                  currentReqProcID,
                                  currentPerProUID ) != SUCCESS )
                {
                    LogError( ERROR_LVL_1, "InsertPerfSeries",
                              __LINE__, MC_NORMAL_COMPLETION,
                              "Couldn't insert an image SOP UID:  %s\n",
                              imageRec.sopInstanceUID );
                    return( FAILURE );
                }
            }
            else
            {
                LogError( ERROR_LVL_1, "InsertPerfSeries",
                          __LINE__, MC_NORMAL_COMPLETION,
                          "Found an invalid keyword:  %s\n", line );
            }

            /*
            ** Clear out what we've read, and start again.
            */
            read = '\0';
            x=0;
        } /* end of else [we've found a keyword] */
    } /* end of while loop */

    fclose( infile );
    return( SUCCESS );
}


/*****************************************************************************
**
** NAME
**    Readln
**
** SYNOPSIS
**    This function reads a single line from the input file, placing its
**    contents into the string pointed to by "line".  Also, the "line" is
**    NULL terminated.  The function also returns a pointer to this "line".
**
*****************************************************************************/
char *
Readln(
          FILE *infile,
          char *line
      )
{
    char read = '\0';
    int  x = 0;

    while ( read != '\n' )
    {
        read = fgetc( infile );
        if ( feof( infile ) )
            break;
        if ( read == '\n' )
        {
            line[x] = '\0';
            break;
        }
        line[x] = read;
        x++;
    }

    return( line );

}


/*****************************************************************************
**
** NAME
**     ObtainPatientPtr
**
** SYNOPSIS
**     Given a patient ID, this function will return a pointer to the
**     place where the actual data is contained within the database.
**     This is used to modify the database's values.
**
*****************************************************************************/
PATIENT_REC *
ObtainPatientPtr(
                    char *A_patientID
                )

{
    int               found = FALSE;
    NODE              *nodePtr;
    void              *dataPtr;
    PATIENT_REC       patientRec;

    /*
    ** First, we search for a matching patient.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    ** We match on patient ID, since it is the key to the patient record.
    */
    while( (nodePtr = LLPopNode( &G_patientList, &patientRec ))  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    /*
    ** If we couldn't find a matching patient, then we must fail.
    */
    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "ObtainPatientPtr",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  "patient id:  %s\n", A_patientID );
        return( NULL );
    }

    /*
    ** The only way that we could have gotten here is if we found a matching
    ** patient.  Therefore, save the pointer to the patient data.
    */
    dataPtr = LLGetDataPtr( nodePtr );
    return( (PATIENT_REC *)dataPtr );
}


/*****************************************************************************
**
** NAME
**     ObtainReqProcPtr
**
** SYNOPSIS
**     Given a patient ID, and a requested procedure step ID,
**     this function will return a pointer to the
**     place where the actual data is contained within the database.
**     This is used to modify the database's values.
**
*****************************************************************************/
REQ_PROC_REC *
ObtainReqProcPtr(
                    char *A_patientID,
                    char *A_requestedProcID
                )

{
    int               found = FALSE;
    NODE              *nodePtr;
    void              *dataPtr;
    PATIENT_REC       patientRec;
    REQ_PROC_REC      reqProcRec;

    /*
    ** First, we search for a matching patient.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    ** We match on patient ID, since it is the key to the patient record.
    */
    while( (nodePtr = LLPopNode( &G_patientList, &patientRec ))  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    /*
    ** If we couldn't find a matching patient, then we must fail.
    */
    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "ObtainReqProcPtr",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  "patient id:  %s\n", A_patientID );
        return( NULL );
    }

    /*
    ** The only way that we could have gotten here is if we found a matching
    ** patient.  Therefore, save the pointer to the patient data, and continue
    ** with the other keys.
    */
    found = FALSE; /* reset this flag for later */

    /*
    ** Now, attempt to find the requested procedure that is associated with
    ** this patient ID.
    */
    LLRewind( patientRec.requestedProcedureList );
    while( (nodePtr = LLPopNode( patientRec.requestedProcedureList,
                                &reqProcRec )) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( reqProcRec.requestedProcID, A_requestedProcID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    /*
    ** If we couldn't find a matching patient, then we must fail.
    */
    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "ObtainReqProcPtr",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a requested Procedure record for\n"
                  "requested procedure id:  %s\n", A_requestedProcID );
        return( NULL );
    }

    dataPtr = LLGetDataPtr( nodePtr );
    return( (REQ_PROC_REC *)dataPtr );
}


/*****************************************************************************
**
** NAME
**     ObtainSchProcStepPtr
**
** SYNOPSIS
**     Given a patient ID, and a requested procedure step ID, and a
**     scheduled procedure step ID, this function will return a pointer
**     to the place where the actual data is contained within the database.
**     This is used to modify the database's values.
**
*****************************************************************************/
SCH_PROC_STEP_REC *
ObtainSchProcStepPtr(
                        char *A_patientID,
                        char *A_requestedProcID,
                        char *A_scheduledProcStepID
                    )

{
    int               found = FALSE;
    NODE              *nodePtr;
    void              *dataPtr;
    PATIENT_REC       patientRec;
    REQ_PROC_REC      reqProcRec;
    SCH_PROC_STEP_REC schProcStepRec;

    /*
    ** First, we search for a matching patient.
    */
    LLRewind( &G_patientList );

    /*
    ** Now, for each patient node on the list, look for a matching patient.
    ** We match on patient ID, since it is the key to the patient record.
    */
    while( (nodePtr = LLPopNode( &G_patientList, &patientRec ))  != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( patientRec.patientID, A_patientID ) == 0 )
        {
            found = TRUE;
            break;
        }
    } /* end of patient search loop */

    /*
    ** If we couldn't find a matching patient, then we must fail.
    */
    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "ObtainSchProcStepPtr",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a patient record for\n"
                  "patient id:  %s\n", A_patientID );
        return( NULL );
    }

    /*
    ** The only way that we could have gotten here is if we found a matching
    ** patient.  Therefore, save the pointer to the patient data, and continue
    ** with the other keys.
    */
    found = FALSE; /* reset this flag for later */

    /*
    ** Now, attempt to find the requested procedure that is associated with
    ** this patient ID.
    */
    LLRewind( patientRec.requestedProcedureList );
    while( (nodePtr = LLPopNode( patientRec.requestedProcedureList,
                                &reqProcRec )) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( reqProcRec.requestedProcID, A_requestedProcID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    /*
    ** If we couldn't find a matching patient, then we must fail.
    */
    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "ProcessNSETRQ",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a requested Procedure record for\n"
                  "requested procedure id:  %s\n", A_requestedProcID );
        return( NULL );
    }

    /*
    ** Now, attempt to find the scheduled procedure that is associated with
    ** this patient ID, and requested procedure.
    */
    LLRewind( reqProcRec.scheduledProcedureStepList );
    while( (nodePtr = LLPopNode( reqProcRec.scheduledProcedureStepList,
                                 &schProcStepRec )) != NULL )
    {
        /*
        ** We've found a node, so we exit from the search loop.
        */
        if ( strcmp( schProcStepRec.scheduledProcStepID,
                     A_scheduledProcStepID ) == 0 )
        {
            found = TRUE;
            break;
        }
    }

    /*
    ** If we couldn't find a matching patient, then we must fail.
    */
    if ( found != TRUE )
    {
        LogError( ERROR_LVL_1, "ObtainReqProcPtr",
                  __LINE__, MC_NORMAL_COMPLETION,
                  "Unable to find a scheduled procedure record for\n"
                  "requested procedure id:  %s\n", A_scheduledProcStepID );
        return( NULL );
    }

    dataPtr = LLGetDataPtr( nodePtr );
    return( (SCH_PROC_STEP_REC *)dataPtr );
}

