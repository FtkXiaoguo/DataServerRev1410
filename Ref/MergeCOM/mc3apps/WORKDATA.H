/*************************************************************************
 *
 *       System: MergeCOM-3 Worklist Database Sample Applicatin Include File
 *
 *    $Workfile: workdata.h $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is the include file for the worklist samples.
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
/* $NoKeywords: $ */

#ifndef MTI_WORKDATA_H
#define MTI_WORKDATA_H

#ifdef __cplusplus
extern "C" {
#endif


/* Structure definitions for modality worklist database */
typedef struct _PATIENT_REC
{
    char patientID[ LO_LEN ];        /* 0010,0020 */ /* The "key" */
    char patientName[ PN_LEN ];      /* 0010,0010 */
    char patientBirthDate[ DA_LEN ]; /* 0010,0030 */
    char patientSex[ CS_LEN ];       /* 0010,0040 */
    char patientWeight[ DS_LEN ];    /* 0010,1030 */

    LIST *requestedProcedureList;
} PATIENT_REC;

typedef struct _REQ_PROC_REC
{
    /*
    ** Note the following fields are part of the imaging service
    ** request, they are included in this structure to make
    ** the sample easier
    */
    char accessionNumber[ SH_LEN ];          /* 0008,0050 */
    char requestingPhysician[ PN_LEN ];      /* 0032,1032 */
    char referringPhysician[ PN_LEN ];       /* 0008,0090 */

    /*
    ** requested procedure attributes
    */
    char requestedProcID[ SH_LEN ];          /* 0040,1001 */ /* Key attribute */
    char requestedProcDescription[ LO_LEN ]; /* 0032,1060 */
    char requestedProcPriority[ SH_LEN ];    /* 0040,1003 */

    char studyID[ SH_LEN ];                  /* 0020,0010 */
    char studyInstanceUID[ UI_LEN ];         /* 0020,000D */
    LIST *scheduledProcedureStepList;
    LIST *performedProcedureStepList;

} REQ_PROC_REC;

typedef struct _SCH_PROC_STEP_REC
{

    char scheduledAETitle[ AE_LEN ];          /* 0040,0001 */
    char scheduledProcStepLocation[ SH_LEN ]; /* 0040,0010 */
    char scheduledStartDate[ DA_LEN ];        /* 0040,0002 */
    char scheduledStartTime[ TM_LEN ];        /* 0040,0003 */
    char modality[ CS_LEN ];                  /* 0008,0060 */
    char scheduledPerformPhysicianName[ PN_LEN ]; /* 0040,0006 */
    char scheduledProcStepDescription[ LO_LEN ];  /* 0040,0005 */
    char scheduledProcStepID[ SH_LEN ];       /* 0040,0009 */

    int complete;   /* set to TRUE if procedure step has been 
                        completed */

    /* 
     * In the MPPS N-CREATE message, the scheduledProcStepID
     * attribute is type 2.  If the value is set, we will match
     * the MPPS to a specific scheduled step.  If not, we will
     * place the MPPS in the list in the REQ_PROC_REC structure
     *
     * Note, we could place these records in both lists, but when
     * their memory is freed, it would be taken from the 
     * RQ_PROC_REC structure.  This would aid in simplifying
     * searching.
     */
    LIST *performedProcedureStepList;

} SCH_PROC_STEP_REC;

typedef struct _PERF_PROC_STEP_REC
{
    /* 
    ** The performed procedure step UID is the key 
    ** identifying an MPPS instance.  The UID is 
    ** contained in the group 0 elements of the N-CREATE-RQ,
    ** or if not contained there, is returned in the 
    ** N-CREATE-RSP for this MPPS data.  This UID is
    ** subsequently used in N-SET, N-GET, and N-EVENT-REPORT
    ** messages to identify the instance being worked on.
    */
    char performedProcStepUID[ UI_LEN ]; 

    char performedProcStepID[ CS_LEN ];          /* 0040,0253 */
    char performedAETitle[ AE_LEN ];             /* 0040,0241 */
    char performedStationName[ SH_LEN ];         /* 0040,0242 */
    char performedLocation[ SH_LEN ];            /* 0040,0243 */
    char performedProcStepStartDate[ DA_LEN ];   /* 0040,0244 */
    char performedProcStepStartTime[ TM_LEN ];   /* 0040,0245 */
    char performedProcStepStatus[ CS_LEN ];      /* 0040,0252 */
    char performedProcStepDescription[ LO_LEN ]; /* 0040,0254 */
    char performedProcTypeDescription[ LO_LEN ]; /* 0040,0255 */
    char performedProcStepEndDate[ DA_LEN ];     /* 0040,0250 */
    char performedProcStepEndTime[ TM_LEN ];     /* 0040,0251 */

    char modality[ CS_LEN ];                /* 0008,0060 */

    LIST *performedSeriesList;

    /* 
     * Scheduled step attributes that may or may not match to 
     * a specific scheduled step stored.
     */
    char scheduledProcStepDescription[ LO_LEN ];  /* 0040,0005 */
    char scheduledProcStepID[ SH_LEN ];           /* 0040,0009 */

    LIST *imageList;

} PERF_PROC_STEP_REC;

typedef struct _PERF_SERIES_REC
{
    /* image acquisition results */
    char performingPhysicianName[ PN_LEN ]; /* 0008,1050 */
    char protocolName[ LO_LEN ];            /* 0018,1030 */
    char operatorName[ PN_LEN ];            /* 0008,1070 */
    char seriesInstanceUID[ UI_LEN ];       /* 0020,000E */
    char seriesDescription[ LO_LEN ];       /* 0008,103E */
    char retrieveAETitle[ AE_LEN ];         /* 0008,0054 */
} PERF_SERIES_REC;

/*
** The image node of the list
*/
typedef struct _IMAGE_REC
{
    char sopInstanceUID[ UI_LEN ];    /* The "key" to this type of node */
    char sopClassUID[ UI_LEN ]; 
} IMAGE_REC;

/*
** A structure that contains attribute info.  Used to build a linked list
** of attributes (and their info) sent in a worklist search.
*/
typedef struct _ATTRIBUTE_REC
{
    int      msgORitemID;
    unsigned long tag;
    MC_VR    valueRep;
    int      numValues;
    char     value[ PN_LEN ]; /* A string big enough to hold ALL values */
    int      nullFlag;
} ATTRIBUTE_REC;

/*
** A structure that contains ALL of the attributes that make up a modality
** worklist entry.  IE, this is all of the information taken from the complete
** database that makes up a single worklist entry.
*/
typedef struct _WORKLIST_REC
{
    char    patientID[ LO_LEN ];        /* Attributes from the PATIENT_REC */
    char    patientName[ PN_LEN ];
    char    patientBirthDate[ DA_LEN ];
    char    patientSex[ CS_LEN ];
    char    patientWeight[ DS_LEN ];
    char    accessionNumber[ SH_LEN ];  /* Attributes from REQ_PROC_REC   */
    char    requestingPhysician[ PN_LEN ];
    char    referringPhysician[ PN_LEN ];
    char    requestedProcID[ SH_LEN ];
    char    requestedProcPriority[ SH_LEN ];
    char    studyInstanceUID[ UI_LEN ];
    char    requestedProcDescription[ LO_LEN ];
    char    scheduledAETitle[ SH_LEN ]; /*Attributes from SCH_PROC_STEP_REC*/
    char    scheduledStartDate[ DA_LEN*2 ];
    char    scheduledStartTime[ TM_LEN ];
    char    scheduledProcStepID[ SH_LEN ];
    char    modality[ CS_LEN ];
    char    scheduledPerformPhysicianName[ PN_LEN ];
    char    scheduledProcStepDescription[ LO_LEN ];
    char    scheduledProcStepLocation[ SH_LEN ];
} WORKLIST_REC;

/*
** This tri-state type is used to decide if an atribute was sent, not sent, or
** null, when received by this program, from a worklist SCU.
*/
typedef enum
{
    SENT=0,
    NOT_SENT,
    IS_NULL
} VAL_SENT;

/*
** This enum defines the different DICOM attribute types
*/
typedef enum
{
    TYPE_1,
    TYPE_2,
    TYPE_3

} RETURN_TYPE;

/*
** Function prototypes defined in workdata.c
*/
extern int       InsertPatient( PATIENT_REC *patientRec);
extern int InsertRequestedProc( REQ_PROC_REC *requestedProc,
                                char *patientID );
extern int InsertScheduledProc( SCH_PROC_STEP_REC *schProcStepRec,
                                char *patientID,
                                char *requestedProcID );
extern int      InsertPerfProc( PERF_PROC_STEP_REC *perfProcStepRec,
                                char *patientID,
                                char *requestedProcID );
extern int         InsertImage( IMAGE_REC *imageRec,
                                char *patientID,
                                char *requestedProcID,
                                char *performedProcStepID );
extern int    InsertPerfSeries( PERF_SERIES_REC *perfSeriesRec,
                                char *patientID,
                                char *requestedProcID,
                                char *performedProcStepID );
extern void CreateDummyPatient( int  A_messageID,
                                char *A_ID );
extern void           DumpTree( void );
extern void           FreeTree( void );
extern char            *Readln( FILE *infile, char *line );
extern int             ReadDat( char *datFile );
extern PATIENT_REC *ObtainPatientPtr( char *patientID );
extern REQ_PROC_REC *ObtainReqProcPtr( char *patientID,
                                       char *reqProcRec );
extern SCH_PROC_STEP_REC *ObtainSchProcStepPtr( char *patientID, 
                                                char *reqProcRec,
                                                char *schProcStepRec );


#ifdef __cplusplus
}
#endif

#endif /* End of MTI_WORKDATA_H */
