// IDcmLibCom.h 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_COM_H_)
#define AFX_IDICOM_LIB_COM_H_

#if 0
#include <string>
#define dcm_string std::string
#define stringTochar(dcm_string) dcm_string.c_str() 
#define charTostring(c) std::string(c) 
#define dcm_string_size(dcm_string) dcm_string.size()
#endif

#include "IDcmLibMsg.h"

/////////////////////
/* ======================================================================== *
 *                    Association Information Structure                     *
 * ======================================================================== *
 *  Calls to MC_Get_Association_Info must pass the address of a variable    *
 *   of this type.  The structure will be filled in by that function.       */
typedef struct MC_Assoc_Info {
    int     NumberOfProposedServices;        /* From service list */
    int     NumberOfAcceptableServices;      /* Acceptable to both sides */
    char    RemoteApplicationTitle[32];      /* 16-characters max */
    char    RemoteHostName[128];              /* Network node name
                                                64-characters max*/
    int     Tcp_socket;                      /* TCP Socket used for
                                                association */
    char    RemoteIPAddress[128];             /* Network IP Address */
    char    LocalApplicationTitle[32];       /* 16-characters max */
    char    RemoteImplementationClassUID[128];/* 64-characters max */
    char    RemoteImplementationVersion[32]; /* 16-characters max */
	//
		
	unsigned long LocalMaximumPDUSize;   
	unsigned long RemoteMaximumPDUSize;
	unsigned short MaxOperationsInvoked;     /* Negotiated Max operations
                                              * invoked by the assoc requestor */
	unsigned short MaxOperationsPerformed;   /* Negotiated Max operations
                                              * performed by the assoc requestor */
} AssocInfo;

#define ASInfoStrCpy_RemoteApplicationTitle(dest,src) strncpy(dest,src,32)
#define ASInfoStrCpy_LocalApplicationTitle(dest,src) strncpy(dest,src,32)
#define ASInfoStrCpy_RemoteHostName(dest,src) strncpy(dest,src,128)
#define ASInfoStrCpy_RemoteIPAddress(dest,src) strncpy(dest,src,128)
#define ASInfoStrCpy_RemoteImplementationClassUID(dest,src) strncpy(dest,src,128)
#define ASInfoStrCpy_RemoteImplementationVersion(dest,src) strncpy(dest,src,32)


/* ======================================================================== *
 *                      Message Response Status Codes                       *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         *
 * Definitions for some of the more common DICOM Service Class status codes */
 
#define RESP_STATUS unsigned short


/* ======================================================================== *
 *                  Storage Service Class: C_STORE_RSP Codes                *
 * ======================================================================== */
/* SUCCESS code */
#define C_STORE_SUCCESS                         ((RESP_STATUS)0x0000)

/* WARNING codes */
#define C_STORE_WARNING_ELEMENT_COERCION        ((RESP_STATUS)0xB000)
#define C_STORE_WARNING_INVALID_DATASET         ((RESP_STATUS)0xB007)
#define C_STORE_WARNING_ELEMENTS_DISCARDED      ((RESP_STATUS)0xB006)

/* FAILURE codes */
#define C_STORE_FAILURE_REFUSED_NO_RESOURCES    ((RESP_STATUS)0xA700)
#define C_STORE_FAILURE_INVALID_DATASET         ((RESP_STATUS)0xA900)
#define C_STORE_FAILURE_CANNOT_UNDERSTAND       ((RESP_STATUS)0xC000)
#define C_STORE_FAILURE_PROCESSING_FAILURE      ((RESP_STATUS)0x0110)


/* ======================================================================== *
 *        Study Content NotificationService Class: C_STORE_RSP Codes        *
 * ======================================================================== */
/* SUCCESS codes */
#define C_STORE_SUCCESS_COMPLETE_STUDY_EXISTS   ((RESP_STATUS)0x0000)
#define C_STORE_SUCCESS_PARTIAL_STUDY_EXISTS    ((RESP_STATUS)0x0001)
#define C_STORE_SUCCESS_STUDY_DOES_NOT_EXIST    ((RESP_STATUS)0x0002)
#define C_STORE_SUCCESS_UNKNOWN_EXISTENCE       ((RESP_STATUS)0x0003)

/* FAILURE code */
#define C_STORE_FAILED_OPERATION                ((RESP_STATUS)0xC002)


/* ======================================================================== *
 *                Verification Service Class: C_ECHO_RSP Codes              *
 * ======================================================================== */
/* SUCCESS code */
#define C_ECHO_SUCCESS                          ((RESP_STATUS)0x0000)


/* ======================================================================== *
 *               Query/Retrieve Service Class: C_FIND_RSP Codes             *
 * ======================================================================== */
/* SUCCESS code */
#define C_FIND_SUCCESS                          ((RESP_STATUS)0x0000)

/* PENDING codes */                         
#define C_FIND_PENDING                          ((RESP_STATUS)0xFF00)
#define C_FIND_PENDING_NO_OPTIONAL_KEY_SUPPORT  ((RESP_STATUS)0xFF01)

/* FAILURE codes */
#define C_FIND_FAILURE_REFUSED_NO_RESOURCES     ((RESP_STATUS)0xA700)
#define C_FIND_FAILURE_INVALID_DATASET          ((RESP_STATUS)0xA900)
#define C_FIND_FAILURE_UNABLE_TO_PROCESS        ((RESP_STATUS)0xC001)
#define C_FIND_FAILURE_RPI_UNABLE_TO_PROCESS    ((RESP_STATUS)0xC000)
#define C_FIND_FAILURE_MORE_THAN_ONE_MATCH_FOUND ((RESP_STATUS)0xC100)
#define C_FIND_FAILURE_UNABLE_TO_SUPPORT_REQUESTED_TEMPLATE ((RESP_STATUS) 0xC200)

/* CANCEL code */
#define C_FIND_CANCEL_REQUEST_RECEIVED          ((RESP_STATUS)0xFE00)


/* ======================================================================== *
 *               Query/Retrieve Service Class: C_MOVE_RSP Codes             *
 * ======================================================================== */
/* SUCCESS code */
#define C_MOVE_SUCCESS_NO_FAILURES              ((RESP_STATUS)0x0000)

/* WARNING code */
#define C_MOVE_WARNING_ONE_OR_MORE_FAILURES     ((RESP_STATUS)0xB000)

/* PENDING code */
#define C_MOVE_PENDING_MORE_SUB_OPERATIONS      ((RESP_STATUS)0xFF00)

/* FAILURE codes */
#define C_MOVE_FAILURE_REFUSED_CANT_CALCULATE   ((RESP_STATUS)0xA701)
#define C_MOVE_FAILURE_REFUSED_CANT_PERFORM     ((RESP_STATUS)0xA702)
#define C_MOVE_FAILURE_REFUSED_DEST_UNKNOWN     ((RESP_STATUS)0xA801)
#define C_MOVE_FAILURE_INVALID_DATASET          ((RESP_STATUS)0xA900)
#define C_MOVE_FAILURE_UNABLE_TO_PROCESS        ((RESP_STATUS)0xC001 )

/* CANCEL code */
#define C_MOVE_CANCEL_REQUEST_RECEIVED          ((RESP_STATUS)0xFE00)


/* ======================================================================== *
 *               Query/Retrieve Service Class: C_GET_RSP Codes              *
 * ======================================================================== */
/* SUCCESS code */
#define C_GET_SUCCESS_NO_FAILURES_OR_WARNINGS   ((RESP_STATUS)0x0000)

/* WARNING code */
#define C_GET_WARNING_SOME_FAILURES_WARNINGS    ((RESP_STATUS)0xB000)

/* PENDING code */
#define C_GET_PENDING_MORE_SUB_OPERATIONS       ((RESP_STATUS)0xFF00)

/* FAILURE codes */
#define C_GET_FAILURE_REFUSED_CANT_CALC_MATCHES ((RESP_STATUS)0xA701)
#define C_GET_FAILURE_REFUSED_CANT_PERFORM      ((RESP_STATUS)0xA702)
#define C_GET_FAILURE_INVALID_DATASET           ((RESP_STATUS)0xA900)
#define C_GET_FAILURE_UNABLE_TO_PROCESS         ((RESP_STATUS)0xC001)

/* CANCEL code */
#define C_GET_CANCEL_REQUEST_RECEIVED           ((RESP_STATUS)0xFE00)


/* ======================================================================== *
 *                      N_EVENT_REPORT_RSP Response Codes                   *
 * ======================================================================== */
/* SUCCESS code */
#define N_EVENT_SUCCESS                         ((RESP_STATUS)0x0000)

/* FAILURE codes */
#define N_EVENT_CLASS_INSTANCE_CONFLICT         ((RESP_STATUS)0x0119)
#define N_EVENT_DUPLICATE_INVOCATION            ((RESP_STATUS)0x0210)
#define N_EVENT_INVALID_ARGUMENT_VALUE          ((RESP_STATUS)0x0115)
#define N_EVENT_MISTYPED_ARGUMENT               ((RESP_STATUS)0x0212)
#define N_EVENT_NO_SUCH_ARGUMENT                ((RESP_STATUS)0x0114)
#define N_EVENT_NO_SUCH_EVENT_TYPE              ((RESP_STATUS)0x0113)
#define N_EVENT_NO_SUCH_SOP_CLASS               ((RESP_STATUS)0x0118)
#define N_EVENT_NO_SUCH_SOP_INSTANCE            ((RESP_STATUS)0x0112)
#define N_EVENT_PROCESSING_FAILURE              ((RESP_STATUS)0x0110)
#define N_EVENT_RESOURCE_LIMITATION             ((RESP_STATUS)0x0213)
#define N_EVENT_UNRECOGNIZED_OPERATION          ((RESP_STATUS)0x0211)


/* ======================================================================== *
 *                          N_GET_RSP Response Codes                        *
 * ======================================================================== */
/* SUCCESS code */
#define N_GET_SUCCESS                           ((RESP_STATUS)0x0000)

/* WARNING codes */
#define N_GET_WARNING_OPT_ATTRIB_UNSUPPORTED    ((RESP_STATUS)0x0001)
#define N_GET_ATTRIBUTE_LIST_ERROR              ((RESP_STATUS)0x0107)

/* FAILURE codes */
#define N_GET_CLASS_INSTANCE_CONFLICT           ((RESP_STATUS)0x0119)
#define N_GET_DUPLICATE_INVOCATION              ((RESP_STATUS)0x0210)
#define N_GET_MISTYPED_ARGUMENT                 ((RESP_STATUS)0x0212)
#define N_GET_NO_SUCH_SOP_CLASS                 ((RESP_STATUS)0x0118)
#define N_GET_NO_SUCH_SOP_INSTANCE              ((RESP_STATUS)0x0112)
#define N_GET_PROCESSING_FAILURE                ((RESP_STATUS)0x0110)
#define N_GET_RESOURCE_LIMITATION               ((RESP_STATUS)0x0213)
#define N_GET_UNRECOGNIZED_OPERATION            ((RESP_STATUS)0x0211)


/* ======================================================================== *
 *                          N_SET_RSP Response Codes                        *
 * ======================================================================== */
/* SUCCESS code */
#define N_SET_SUCCESS                           ((RESP_STATUS)0x0000)


/* WARNING codes */
#define N_SET_REQUESTED_MIN_OR_MAX_DENSITY_OUTSIDE_RANGE ((RESP_STATUS)0xB605)
#define N_SET_ATTRIBUTE_VALUE_OUT_OF_RANGE      ((RESP_STATUS)0x0116)
#define N_SET_ATTRIBUTE_LIST_ERROR              ((RESP_STATUS)0x0107)

/* FAILURE codes */
#define N_SET_CLASS_INSTANCE_CONFLICT           ((RESP_STATUS)0x0119)
#define N_SET_DUPLICATE_INVOCATION              ((RESP_STATUS)0x0210)
#define N_SET_INVALID_ATTRIBUTE_VALUE           ((RESP_STATUS)0x0106)
#define N_SET_MISTYPED_ARGUMENT                 ((RESP_STATUS)0x0212)
#define N_SET_INVALID_OBJECT_INSTANCE           ((RESP_STATUS)0x0117)
#define N_SET_MISSING_ATTRIBUTE_VALUE           ((RESP_STATUS)0x0121)
#define N_SET_NO_SUCH_ATTRIBUTE                 ((RESP_STATUS)0x0105)
#define N_SET_NO_SUCH_SOP_CLASS                 ((RESP_STATUS)0x0118)
#define N_SET_NO_SUCH_SOP_INSTANCE              ((RESP_STATUS)0x0112)
#define N_SET_PROCESSING_FAILURE                ((RESP_STATUS)0x0110)
#define N_SET_RESOURCE_LIMITATION               ((RESP_STATUS)0x0213)
#define N_SET_UNRECOGNIZED_OPERATION            ((RESP_STATUS)0x0211)

/* Print Management Service Classes WARNING codes*/
#define N_SET_WARNING_IMAGE_CROPPED_TO_FIT      ((RESP_STATUS)0xB609)
#define N_SET_WARNING_IMAGE_DECIMATED_TO_FIT    ((RESP_STATUS)0xB60A)

/* Print Management Service Classes FAILURE codes*/
#define N_SET_INSUFFICIENT_PRINTER_MEMORY       ((RESP_STATUS)0xC605)
#define N_SET_MORE_THAN_ONE_VOID_LUT_BOX        ((RESP_STATUS)0xC606)
#define N_SET_FAILURE_IMAGE_LARGER_THAN_BOX     ((RESP_STATUS)0xC603)
#define N_SET_FAILURE_COMBINED_LARGER_THAN_BOX  ((RESP_STATUS)0xC613)

/* Modality Performed Procedure Step FAILURE codes*/
#define N_SET_OBJECT_MAY_NO_LONGER_BE_UPDATED   ((RESP_STATUS)0x0110)

/* Modify GP Performed Procedure Step REFUSED codes */
#define N_SET_REFUSED_PPS_NOT_IN_PROGRESS            ((RESP_STATUS)0xA506)


/* ======================================================================== *
 *                        N_ACTION_RSP Response Codes                       *
 * ======================================================================== */
/* SUCCESS code */
#define N_ACTION_SUCCESS                        ((RESP_STATUS)0x0000)

/* FAILURE codes */
#define N_ACTION_CLASS_INSTANCE_CONFLICT        ((RESP_STATUS)0x0119)
#define N_ACTION_DUPLICATE_INVOCATION           ((RESP_STATUS)0x0210)
#define N_ACTION_INVALID_ARGUMENT_VALUE         ((RESP_STATUS)0x0115)
#define N_ACTION_MISTYPED_ARGUMENT              ((RESP_STATUS)0x0212)
#define N_ACTION_NO_SUCH_ARGUMENT               ((RESP_STATUS)0x0114)
#define N_ACTION_NO_SUCH_SOP_CLASS              ((RESP_STATUS)0x0118)
#define N_ACTION_NO_SUCH_SOP_INSTANCE           ((RESP_STATUS)0x0112)
#define N_ACTION_PROCESSING_FAILURE             ((RESP_STATUS)0x0110)
#define N_ACTION_RESOURCE_LIMITATION            ((RESP_STATUS)0x0213)
#define N_ACTION_UNRECOGNIZED_OPERATION         ((RESP_STATUS)0x0211)
#define N_ACTION_NO_SUCH_ACTION_TYPE            ((RESP_STATUS)0x0123)

/* Print Management Service Classes WARNING codes */
#define N_ACTION_WARNING_NO_COLLATION_SUPPORT   ((RESP_STATUS)0xB601)
#define N_ACTION_WARNING_SESSION_EMPTY_PAGE     ((RESP_STATUS)0xB602)
#define N_ACTION_WARNING_BOX_EMPTY_PAGE         ((RESP_STATUS)0xB603)
#define N_ACTION_WARNING_IMAGE_CROPPED_TO_FIT   ((RESP_STATUS)0xB609)
#define N_ACTION_WARNING_IMAGE_DECIMATED_TO_FIT ((RESP_STATUS)0xB60A)

/* Print Management Service Classes FAILURE codes */
#define N_ACTION_FAILURE_NO_FILM_SOP_INSTANCES  ((RESP_STATUS)0xC600)
#define N_ACTION_FAILURE_SESSION_PRINT_Q_FULL   ((RESP_STATUS)0xC601)
#define N_ACTION_FAILURE_BOX_PRINT_Q_FULL       ((RESP_STATUS)0xC602)
#define N_ACTION_FAILURE_SIZE_LARGER_THAN_BOX   ((RESP_STATUS)0xC603)
#define N_ACTION_FAILURE_IMAGE_POS_COLLISION    ((RESP_STATUS)0xC604)
#define N_ACTION_FAILURE_COMBINED_SIZE_LARGER_THAN_BOX ((RESP_STATUS)0xC613)

/* Print Queue Management Service Classes FAILURE codes */
#define N_ACTION_FAILURE_PRINT_QUEUE_IS_HALTED  ((RESP_STATUS)0xC651)
#define N_ACTION_FAILURE_MISMATCH_OF_OWNER_IDS  ((RESP_STATUS)0xC652)
#define N_ACTION_FAILURE_JOB_IN_PROCESS         ((RESP_STATUS)0xC653)

/* Pull Print Request WARNING codes */
#define N_ACTION_MEMORY_ALLOCATION_NOT_SUPP     ((RESP_STATUS)0xB600)
#define N_ACTION_WARNING_ANNOTATION_NOT_SUPP    ((RESP_STATUS)0xB604)
#define N_ACTION_WARNING_IMAGE_OVERLAY_NOT_SUPP ((RESP_STATUS)0xB605)
#define N_ACTION_WARNING_PRES_LUT_NOT_SUPP      ((RESP_STATUS)0xB606)
#define N_ACTION_WARNING_IMAGE_BOX_LUT_NOT_SUPP ((RESP_STATUS)0xB608)

/* Pull Print Request FAILURE codes */
#define N_ACTION_FAILURE_INSUFFICIENT_MEMORY    ((RESP_STATUS)0xC605)
#define N_ACTION_FAILURE_NO_STORED_PRINT_AT_AE  ((RESP_STATUS)0xC607)
#define N_ACTION_FAILURE_NO_IMAGE_AT_AE         ((RESP_STATUS)0xC608)
#define N_ACTION_FAILURE_RETRIEVING_STORED_PRINT ((RESP_STATUS)0xC609)
#define N_ACTION_FAILURE_RETRIEVING_IMAGE       ((RESP_STATUS)0xC60A)
#define N_ACTION_FAILURE_UNKNOWN_RETREIVE_AE    ((RESP_STATUS)0xC60B)
#define N_ACTION_FAILURE_PRINTER_CANNOT_ACCEPT_COLOR ((RESP_STATUS)0xC60C)
#define N_ACTION_FAILURE_EMPTY_PAGE             ((RESP_STATUS)0xC60D)
#define N_ACTION_FAILURE_ANNOTATION_NOT_SUPP    ((RESP_STATUS)0xC60E)
#define N_ACTION_FAILURE_IMAGE_OVERLAY_NOT_SUPP ((RESP_STATUS)0xC60F)
#define N_ACTION_FAILURE_PRES_LUT_NOT_SUPP      ((RESP_STATUS)0xC610)
#define N_ACTION_FAILURE_IMAGE_BOX_LUT_NOT_SUPP ((RESP_STATUS)0xC614)
#define N_ACTION_FAILURE_UNABLE_TO_OPEN_ASSOC   ((RESP_STATUS)0xC615)

/* Modify GP Scheduled Procedure Step REFUSED codes */
#define N_ACTION_REFUSED_MAY_NO_LONGER_BE_UPDATED    ((RESP_STATUS)0xA501)
#define N_ACTION_REFUSED_WRONG_TRANSACTION_ID        ((RESP_STATUS)0xA502)
#define N_ACTION_REFUSED_ALREADY_IN_PROGRESS         ((RESP_STATUS)0xA503)

/* Media Creation Management FAILURE codes */
#define N_ACTION_MEDIA_CREATION_RQ_COMPLETED    ((RESP_STATUS)0xC201)
#define N_ACTION_MEDIA_CREATION_RQ_IN_PROGRESS  ((RESP_STATUS)0xC202)
#define N_ACTION_CANCELLATION_DENIED            ((RESP_STATUS)0xC203)


/* ======================================================================== *
 *                        N_CREATE_RSP Response Codes                       *
 * ======================================================================== */
/* SUCCESS code */
#define N_CREATE_SUCCESS                        ((RESP_STATUS)0x0000)

/* FAILURE codes */
#define N_CREATE_CLASS_INSTANCE_CONFLICT        ((RESP_STATUS)0x0119)
#define N_CREATE_DUPLICATE_INVOCATION           ((RESP_STATUS)0x0210)
#define N_CREATE_DUPLICATE_SOP_INSTANCE         ((RESP_STATUS)0x0111)
#define N_CREATE_INVALID_ATTRIBUTE_VALUE        ((RESP_STATUS)0x0106)
#define N_CREATE_MISSING_ATTRIBUTE              ((RESP_STATUS)0x0120)
#define N_CREATE_MISSING_ATTRIBUTE_VALUE        ((RESP_STATUS)0x0121)
#define N_CREATE_MISTYPED_ARGUMENT              ((RESP_STATUS)0x0212)
#define N_CREATE_NO_SUCH_ATTRIBUTE              ((RESP_STATUS)0x0105)
#define N_CREATE_NO_SUCH_SOP_CLASS              ((RESP_STATUS)0x0118)
#define N_CREATE_NO_SUCH_SOP_INSTANCE           ((RESP_STATUS)0x0112)
#define N_CREATE_PROCESSING_FAILURE             ((RESP_STATUS)0x0110)
#define N_CREATE_RESOURCE_LIMITATION            ((RESP_STATUS)0x0213)
#define N_CREATE_UNRECOGNIZED_OPERATION         ((RESP_STATUS)0x0211)

/* Print Management Service Classes FAILURE codes */
#define N_CREATE_NO_MEMORY_ALLOCATION_POSSIBLE  ((RESP_STATUS)0x0106)
#define N_CREATE_TEMPORARILY_NO_MEMORY          ((RESP_STATUS)0x0213)

/* Print Management Service Classes WARNING codes */
#define N_CREATE_MEMORY_ALLOCATION_NOT_SUPPORTED ((RESP_STATUS)0xB600)
#define N_CREATE_REQUESTED_MIN_OR_MAX_DENSITY_OUTSIDE_RANGE ((RESP_STATUS)0xB605)
#define N_CREATE_ATTRIBUTE_VALUE_OUT_OF_RANGE    ((RESP_STATUS)0x0116)
#define N_CREATE_ATTRIBUTE_LIST_ERROR            ((RESP_STATUS)0x0107)

/* Basic Print Image Overlay Box FAILURE codes */
#define N_CREATE_FAILURE_COMBINED_REQUIRES_CROPPING  ((RESP_STATUS)0xC616)

/* Modify GP Performed Procedure Step REFUSED codes */
#define N_CREATE_REFUSED_SPS_NOT_IN_PROGRESS         ((RESP_STATUS)0xA504)
#define N_CREATE_REFUSED_UNMATCHED_TRANSACTION_ID    ((RESP_STATUS)0xA505)


/* ======================================================================== *
 *                        N_DELETE_RSP Response Codes                       *
 * ======================================================================== */
/* SUCCESS code */
#define N_DELETE_SUCCESS                        ((RESP_STATUS)0x0000)

/* FAILURE codes */
#define N_DELETE_CLASS_INSTANCE_CONFLICT    ((RESP_STATUS)0x0119)
#define N_DELETE_DUPLICATE_INVOCATION       ((RESP_STATUS)0x0210)
#define N_DELETE_MISTYPED_ARGUMENT          ((RESP_STATUS)0x0212)
#define N_DELETE_NO_SUCH_SOP_CLASS          ((RESP_STATUS)0x0118)
#define N_DELETE_NO_SUCH_SOP_INSTANCE       ((RESP_STATUS)0x0112)
#define N_DELETE_PROCESSING_FAILURE         ((RESP_STATUS)0x0110)
#define N_DELETE_RESOURCE_LIMITATION        ((RESP_STATUS)0x0213)
#define N_DELETE_UNRECOGNIZED_OPERATION     ((RESP_STATUS)0x0211)

/* ======================================================================== *
 *                     SCU and SCP Role information                         *
 * ======================================================================== */
typedef enum {
        SCU_ROLE,
        SCP_ROLE,
        BOTH_ROLES
} ROLE_TYPE;



/* ======================================================================== *
 *                      Service Information Structure                       *
 * ======================================================================== *
 *  Calls to MC_Get_First_Acceptable_Service and                            *
 *   MC_Get_Next_Acceptable_Service must pass the address of a variable of  *
 *   this type.  The structure will be filled in by that function.          */
typedef struct MC_Service_Info {
    char            ServiceName[64+2];        /* MergeCOM-3 Service Name */
    TRANSFER_SYNTAX SyntaxType;             /* Transfer syntax negotiated 
                                               for the service */
    ROLE_TYPE       RoleNegotiated;         /* The role negotiated for the 
                                               service */
	int				PresentationContextID;
} ServiceInfo;

/* ======================================================================== *
 *               Association Rejection Reasons Enumerated Type              *
 * ======================================================================== *
 *  Calls to MC_Reject_Association must use this enumerated type to         *
 *   specify the reason why the association is being rejected.  Note that   *
 *   not all of the potential rejection reasons are listed because they are *
 *   automatically handled by the tool kit.                                 */
typedef enum {
        PERMANENT_NO_REASON_GIVEN,
        TRANSIENT_NO_REASON_GIVEN, 
        PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED,
        TRANSIENT_TEMPORARY_CONGESTION,
        TRANSIENT_LOCAL_LIMIT_EXCEEDED
} REJECT_REASON;

#endif // !defined(AFX_IDICOM_LIB_COM_H_)
 