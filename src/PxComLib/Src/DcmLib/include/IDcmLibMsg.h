// IDcmLibMsg.h 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_MSG_H_)
#define AFX_IDICOM_LIB_MSG_H_


#include <string>

/////////////////////

/* ======================================================================== *
 *                    MC3MSG Service Return Status values                   *
 * ======================================================================== */
typedef enum {
	MC_ERROR = -1,
    MC_NORMAL_COMPLETION        = 1,
        
    MC_ALREADY_REGISTERED       = 4000,  /* NEVER CHANGE */
    MC_ASSOCIATION_ABORTED,
    MC_ASSOCIATION_CLOSED,
    MC_ASSOCIATION_REJECTED,
    MC_ATTRIBUTE_HAS_VALUES,
    MC_BUFFER_TOO_SMALL,
    MC_CALLBACK_CANNOT_COMPLY,
    MC_CALLBACK_DATA_SIZE_NEGATIVE,
    MC_CALLBACK_DATA_SIZE_UNEVEN,
    MC_CALLBACK_PARM_ERROR,
    MC_CALLBACK_REGISTERED,
    MC_CANNOT_COMPLY,
    MC_CANT_ACCESS_PROFILE,
    MC_CONFIG_INFO_ERROR,
    MC_CONFIG_INFO_MISSING,
    MC_DDFILE_ERROR,
    MC_DOES_NOT_VALIDATE,
    MC_EMPTY_VALUE,
    MC_END_OF_DATA,
    MC_EXT_INFO_UNAVAILABLE,
    MC_FOUND,
    MC_FUNCTION_UNAVAILABLE,
    MC_INCOMPATIBLE_VR,        
    MC_INCOMPATIBLE_VALUE,
    MC_INVALID_APPLICATION_ID,
    MC_INVALID_APPLICATION_TITLE,
    MC_INVALID_ASSOC_ID,
    MC_INVALID_CHARS_IN_VALUE,        
    MC_INVALID_COMMAND,
    MC_INVALID_DATA_TYPE,
    MC_END_OF_LIST,
    MC_INVALID_GROUP,
    MC_INVALID_HOST_NAME,
    MC_INVALID_ITEM_ID,
    MC_INVALID_LENGTH_FOR_TITLE,
    MC_INVALID_LENGTH_FOR_VR,
    MC_INVALID_LICENSE,
    MC_INVALID_MESSAGE_ID,
    MC_INVALID_MESSAGE_RECEIVED,
    MC_INVALID_PARAMETER_NAME,
    MC_INVALID_PORT_NUMBER,
    MC_INVALID_PRIVATE_CODE,
    MC_INVALID_SERVICE_LIST_NAME,
    MC_INVALID_TAG,
    MC_INVALID_TRANSFER_SYNTAX,
    MC_INVALID_VALUE_FOR_VR,
    MC_INVALID_VALUE_NUMBER,
    MC_INVALID_VR_CODE,
    MC_LOG_EMPTY,
    MC_MESSAGE_EMPTY,
    MC_MESSAGE_VALIDATES,
    MC_MISSING_CONFIG_PARM,
    MC_MSGFILE_ERROR,
    MC_MUST_BE_POSITIVE,
    MC_NETWORK_SHUT_DOWN,
    MC_NO_APPLICATIONS_REGISTERED,
    MC_NO_CALLBACK,
    MC_NO_CONDITION_FUNCTION,
    MC_NO_FILE_SYSTEM,
    MC_NO_INFO_REGISTERED,
    MC_NO_LICENSE,
    MC_NO_MERGE_INI,
    MC_NO_MORE_ATTRIBUTES,
    MC_NO_MORE_VALUES,
    MC_NO_PROFILE,
    MC_NO_REQUEST_PENDING,
    MC_NON_SERVICE_ATTRIBUTE,
    MC_NOT_FOUND,
    MC_NOT_ONE_OF_ENUMERATED_VALUES,
    MC_NOT_ONE_OF_DEFINED_TERMS,
    MC_NULL_POINTER_PARM,
    MC_NULL_VALUE,
    MC_PROTOCOL_ERROR,
    MC_REQUIRED_ATTRIBUTE_MISSING,
    MC_REQUIRED_DATASET_MISSING,
    MC_REQUIRED_VALUE_MISSING,
    MC_STATE_VIOLATION,
    MC_SYSTEM_CALL_INTERRUPTED,
    MC_SYSTEM_ERROR,
    MC_TAG_ALREADY_EXISTS,
    MC_TEMP_FILE_ERROR,
    MC_TIMEOUT,
    MC_TOO_FEW_VALUES,
    MC_TOO_MANY_BLOCKS,
    MC_TOO_MANY_VALUES,
    MC_UNABLE_TO_CHECK_CONDITION,
    MC_UNACCEPTABLE_SERVICE,
    MC_UNEXPECTED_EOD,
    MC_UNKNOWN_ITEM,
    MC_UNKNOWN_SERVICE,
    MC_VALUE_MAY_NOT_BE_NULL,
    MC_VALUE_NOT_ALLOWED,
    MC_VALUE_OUT_OF_RANGE,
    MC_VALUE_TOO_LARGE,
    MC_VR_ALREADY_VALID,
    MC_LIBRARY_ALREADY_INITIALIZED, 
    MC_LIBRARY_NOT_INITIALIZED,
    MC_INVALID_DIRECTORY_RECORD_OFFSET,
    MC_INVALID_FILE_ID, 
    MC_INVALID_DICOMDIR_ID, 
    MC_INVALID_ENTITY_ID,
    MC_INVALID_MRDR_ID,
    MC_UNABLE_TO_GET_ITEM_ID,
    MC_INVALID_PAD,
    MC_ENTITY_ALREADY_EXISTS,
    MC_INVALID_LOWER_DIR_RECORD,
    MC_BAD_DIR_RECORD_TYPE,
    MC_UNKNOWN_HOST_CONNECTED,
    MC_INACTIVITY_TIMEOUT,
    MC_INVALID_SOP_CLASS_UID,
    MC_INVALID_VERSION,
    MC_OUT_OF_ORDER_TAG,
    MC_CONNECTION_FAILED,
    MC_UNKNOWN_HOST_NAME,
    MC_INVALID_FILE,
    MC_NEGOTIATION_ABORTED,
    MC_INVALID_SR_ID,
    MC_UNABLE_TO_GET_SR_ID,
	MC_DUPLICATE_NAME,
	MC_DUPLICATE_SYNTAX,
	MC_EMPTY_LIST,
	MC_MISSING_NAME,
    MC_INVALID_SERVICE_NAME,
	MC_SERVICE_IN_USE,
	MC_INVALID_SYNTAX_NAME,
	MC_SYNTAX_IN_USE,
	MC_NO_CONTEXT,
	MC_OFFSET_TABLE_TOO_SHORT,
	MC_MISSING_DELIMITER,
	MC_COMPRESSION_FAILURE,
	MC_END_OF_FRAME,
	MC_MUST_CONTINUE_BEFORE_READING,
	MC_COMPRESSOR_REQUIRED,
	MC_DECOMPRESSOR_REQUIRED,
	MC_DATA_AVAILABLE,
	MC_ZLIB_ERROR,
	MC_NOT_META_SOP,
	MC_INVALID_ITEM_TRANSFER_SYNTAX,
	MC_LICENSE_ERROR,
    MC_MAX_OPERATIONS_EXCEEDED
} MC_STATUS;
      
 
/////////////////////


/* =========================================================================*
 *                            Valid Command Codes                           *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         */
#define MC_COMMAND unsigned short


#define C_CANCEL_RQ         ((MC_COMMAND)0x0FFF)
/*
 *  Composite Commands
 */
#define C_STORE_RQ          ((MC_COMMAND)0x0001)
#define C_STORE_RSP         ((MC_COMMAND)0x8001)
#define C_GET_RQ            ((MC_COMMAND)0x0010)
#define C_GET_RSP           ((MC_COMMAND)0x8010)
#define C_CANCEL_GET_RQ     C_CANCEL_RQ
#define C_FIND_RQ           ((MC_COMMAND)0x0020)
#define C_FIND_RSP          ((MC_COMMAND)0x8020)
#define C_CANCEL_FIND_RQ    C_CANCEL_RQ
#define C_MOVE_RQ           ((MC_COMMAND)0x0021)
#define C_MOVE_RSP          ((MC_COMMAND)0x8021)
#define C_CANCEL_MOVE_RQ    C_CANCEL_RQ
#define C_ECHO_RQ           ((MC_COMMAND)0x0030)
#define C_ECHO_RSP          ((MC_COMMAND)0x8030)

/*
 *  Normalized Commands
 */
#define N_EVENT_REPORT_RQ   ((MC_COMMAND)0x0100)
#define N_EVENT_REPORT_RSP  ((MC_COMMAND)0x8100)
#define N_GET_RQ            ((MC_COMMAND)0x0110)
#define N_GET_RSP           ((MC_COMMAND)0x8110)
#define N_SET_RQ            ((MC_COMMAND)0x0120)
#define N_SET_RSP           ((MC_COMMAND)0x8120)
#define N_ACTION_RQ         ((MC_COMMAND)0x0130)
#define N_ACTION_RSP        ((MC_COMMAND)0x8130)
#define N_CREATE_RQ         ((MC_COMMAND)0x0140)
#define N_CREATE_RSP        ((MC_COMMAND)0x8140)
#define N_DELETE_RQ         ((MC_COMMAND)0x0150)
#define N_DELETE_RSP        ((MC_COMMAND)0x8150)

#define INVALID_COMMAND     ((MC_COMMAND)0xFFFF)



/* ======================================================================== *
 *                Message Command Group Attribute Definitions               *
 * ======================================================================== */
#define CMND_GROUP_LENGTH               0x00000000
#define CMND_AFFECTED_SOP_CLASS_UID     0x00000002
#define CMND_REQUESTED_SOP_CLASS_UID    0x00000003
#define CMND_COMMAND                    0x00000100
#define CMND_MESSAGE_ID                 0x00000110
#define CMND_RESPONDING_TO_MESSAGE_ID   0x00000120
#define CMND_MOVE_DESTINATION           0x00000600
#define CMND_PRIORITY                   0x00000700
#define CMND_DATASET_TYPE               0x00000800
#define CMND_STATUS                     0x00000900
#define CMND_OFFENDING_ELEMENT          0x00000901
#define CMND_ERROR_COMMENT              0x00000902
#define CMND_ERROR_ID                   0x00000903
#define CMND_AFFECTED_SOP_INSTANCE_UID  0x00001000
#define CMND_REQUESTED_SOP_INSTANCE_UID 0x00001001
#define CMND_EVENT_TYPE_ID              0x00001002
#define CMND_ATTRIBUTE_IDENTIFIER_LIST  0x00001005
#define CMND_ACTION_TYPE_ID             0x00001008
#define CMND_REMAINING_SUB_OPERATIONS   0x00001020
#define CMND_COMPLETED_SUB_OPERATIONS   0x00001021
#define CMND_FAILED_SUB_OPERATIONS      0x00001022
#define CMND_WARNING_SUB_OPERATIONS     0x00001023
#define CMND_MOVE_ORIGINATOR_AE_TITLE   0x00001030
#define CMND_MOVE_ORIGINATOR_MESSAGE_ID 0x00001031



/* =========================================================================*
 *                   Valid Value Representation(VR) Codes                   *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         *

 *           If compiling VxWorks for the Hitachi SH processor, this test   *
 *           would have to be more specific                                 */
#if defined (VX_COLDFIRE)|| defined (VXWORKS)
#undef SH
#endif

typedef enum {
     AE = 00,
     AS, CS, DA, DS, DT, IS, LO, LT, PN, SH, ST, TM, UT,
     UI, SS, US, AT, SL, UL, FL, FD, UNKNOWN_VR, OB, OW, OL, OF, SQ
} MC_VR;



/* =========================================================================*
 *                        Types of logged messages                          *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         */
typedef enum {
    MC_ERROR_MESSAGE = 1, MC_WARNING_MESSAGE, MC_INFO_MESSAGE, MC_TRACE_MESSAGE,
    /* The types below are only passed to enhanced message log callbacks */
    MC_T1_MESSAGE, MC_T2_MESSAGE, MC_T3_MESSAGE, MC_T4_MESSAGE, MC_T5_MESSAGE,
    MC_T6_MESSAGE, MC_T7_MESSAGE, MC_T8_MESSAGE, MC_T9_MESSAGE, MC_OTHER_MESSAGE
} MsgLogType;



/* ======================================================================== *
 *                          Data Type Codes                             *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         */
typedef enum {
    String_Type = 0,    /* ASCII  null-terminated string */
    Int_Type,           /* Binary integer number */
    UInt_Type,          /* Binary unsigned integer number */
    ShortInt_Type,      /* Binary short integer number */
    UShortInt_Type,     /* Binary unsigned short integer number */
    LongInt_Type,       /* Binary long integer number */
    ULongInt_Type,      /* Binary unsigned long integer number */
    Float_Type,         /* Binary Floating point number */
    Double_Type,        /* Binary double floating point number */
    Buffer_Type         /* Buffer containing value with unknown VR */
} MC_DT;




/* ======================================================================== *
 *             Transfer syntax encoding used for streamed messages          *
 * ======================================================================== */
/*  NOTE:   These values are used as the fourth parameter in
 *      MC_Message_To_Stream() and MC_Stream_To_Message()
 *      function calls.  They specify the transfer syntax
 *      associated with the input or output stream.  The stream
 *      is encoded or decoded differently based on this parameter.
 */
typedef enum {
    INVALID_TRANSFER_SYNTAX = 0,
    IMPLICIT_LITTLE_ENDIAN = 100,
    EXPLICIT_LITTLE_ENDIAN,
    EXPLICIT_BIG_ENDIAN,
    IMPLICIT_BIG_ENDIAN,
    DEFLATED_EXPLICIT_LITTLE_ENDIAN,
    RLE,
    JPEG_BASELINE,
    JPEG_EXTENDED_2_4,
    JPEG_EXTENDED_3_5,
    JPEG_SPEC_NON_HIER_6_8,
    JPEG_SPEC_NON_HIER_7_9,
    JPEG_FULL_PROG_NON_HIER_10_12,
    JPEG_FULL_PROG_NON_HIER_11_13,
    JPEG_LOSSLESS_NON_HIER_14,
    JPEG_LOSSLESS_NON_HIER_15,
    JPEG_EXTENDED_HIER_16_18,
    JPEG_EXTENDED_HIER_17_19,
    JPEG_SPEC_HIER_20_22,
    JPEG_SPEC_HIER_21_23,
    JPEG_FULL_PROG_HIER_24_26,
    JPEG_FULL_PROG_HIER_25_27,
    JPEG_LOSSLESS_HIER_28,
    JPEG_LOSSLESS_HIER_29,
    JPEG_LOSSLESS_HIER_14,
    JPEG_2000_LOSSLESS_ONLY,
    JPEG_2000,
    JPEG_LS_LOSSLESS,
    JPEG_LS_LOSSY,
    MPEG2_MPML,
    PRIVATE_SYNTAX_1,
    PRIVATE_SYNTAX_2,
	JPEG_2000_MC_LOSSLESS_ONLY,
    JPEG_2000_MC

} TRANSFER_SYNTAX;



/* ======================================================================== *
 *                         Validation Error Levels                          *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         */
typedef enum {
    Validation_Level1 = 1,  /* Report only Errors */
    Validation_Level2 = 2,  /* Report Errors and Warnings */
    Validation_Level3 = 3   /* Report Errors, Warnings and Info Messages */
} VAL_LEVEL;




/* ======================================================================== *
 *        Structure containing Message Validation Error Information         *
 * ======================================================================== */
typedef struct ValErr_struct
{
    unsigned long   Tag;            /* Tag with validation error */
    int             MsgItemID;      /* ID of message or item object
                                       containing the tag */
    int             ValueNumber;    /* Value number involved - zero
                                       if error is not value related */
    MC_STATUS       Status;         /* Error status code */
} VAL_ERR;



/* ======================================================================== *
 *                         Condition Return Codes                           *
 * ======================================================================== *
 *           NOTE: NEVER change the values assigned to these codes!         */
#ifndef _MC_COND_
#define _MC_COND_
typedef enum {
    CONDITION_DOES_NOT_EXIST,
    CONDITION_EXISTS,
    CONDITION_UNCERTAIN
} MC_COND;
#endif



/* ======================================================================== *
 *              Structure used to define Validation Functions               *
 * ======================================================================== */
typedef struct ValFunction_struct
{
    char*       ValidationName;     /* Tag with validation error */
    MC_COND     (*ValFunction)(int AmsgID, unsigned long Atag);
} VAL_FUNC;


/* ======================================================================== *
 *          Structure used to provide message log information               *
 *          to a callback function registered with                          *
 *          MC_Register_Enhanced_MemoryLog_Function                         *
 * ======================================================================== */
typedef struct LogTime_struct {
        int hour;    /* hours since midnight - [0,23] */
        int min;     /* minutes after the hour - [0,59] */
        int sec;     /* seconds after the minute - [0,59] */
        int hun;     /* hundreths of a sec after the sec */
        int day;     /* day of the month - [1,31] */
        int mon;     /* months since January - [0,11] */
        int year;    /* years since 1900 */
} LogTime;

typedef struct LogInfo_struct
{
    MsgLogType  typeCode;
    LogTime     timeValues;
    char*       type;
    char*       prefix;     /* May be empty */
    char*       processID;
    char*       timeStamp;
    char*       function;   /* May be empty */
    char*       message;
} LogInfo;


/* ======================================================================== *
 *                  Valid Character Configuration Parameters                *
 * ======================================================================== *
 *  These enumerated values may be used with MC_Set_String_Config_Value()   *
 *  function to change the value of character string configuration values.  *
 *  The names are the same as the parameter names in configuration files    */
typedef enum {
    LOG_FILE,
    LICENSE,
    IMPLEMENTATION_CLASS_UID,
    IMPLEMENTATION_VERSION,
    LOCAL_APPL_CONTEXT_NAME,
    IMPLICIT_LITTLE_ENDIAN_SYNTAX,
    IMPLICIT_BIG_ENDIAN_SYNTAX,
    EXPLICIT_LITTLE_ENDIAN_SYNTAX,
    EXPLICIT_BIG_ENDIAN_SYNTAX,
    DEFLATED_EXPLICIT_LITTLE_ENDIAN_SYNTAX,
    RLE_SYNTAX,
    JPEG_BASELINE_SYNTAX,
    JPEG_EXTENDED_2_4_SYNTAX,
    JPEG_EXTENDED_3_5_SYNTAX,
    JPEG_SPEC_NON_HIER_6_8_SYNTAX,
    JPEG_SPEC_NON_HIER_7_9_SYNTAX,
    JPEG_FULL_PROG_NON_HIER_10_12_SYNTAX,
    JPEG_FULL_PROG_NON_HIER_11_13_SYNTAX,
    JPEG_LOSSLESS_NON_HIER_14_SYNTAX,
    JPEG_LOSSLESS_NON_HIER_15_SYNTAX,
    JPEG_EXTENDED_HIER_16_18_SYNTAX,
    JPEG_EXTENDED_HIER_17_19_SYNTAX,
    JPEG_SPEC_HIER_20_22_SYNTAX,
    JPEG_SPEC_HIER_21_23_SYNTAX,
    JPEG_FULL_PROG_HIER_24_26_SYNTAX,
    JPEG_FULL_PROG_HIER_25_27_SYNTAX,
    JPEG_LOSSLESS_HIER_28_SYNTAX,
    JPEG_LOSSLESS_HIER_29_SYNTAX,
    JPEG_LOSSLESS_HIER_14_SYNTAX,
    JPEG_2000_LOSSLESS_ONLY_SYNTAX,
    JPEG_2000_SYNTAX,
    INITIATOR_NAME,
    RECEIVER_NAME,
    DICTIONARY_FILE,
    MSG_INFO_FILE,
    TEMP_FILE_DIRECTORY,
    UNKNOWN_VR_CODE,
    PRIVATE_SYNTAX_1_SYNTAX,
    PRIVATE_SYNTAX_2_SYNTAX,
    CAPTURE_FILE,
	PEGASUS_OP_LIP3PLUS_NAME,
	PEGASUS_OP_LIE3PLUS_NAME,
	PEGASUS_OP_D2SEPLUS_NAME,
	PEGASUS_OP_SE2DPLUS_NAME,
	PEGASUS_OP_J2KP_NAME,
	PEGASUS_OP_J2KE_NAME,
	PEGASUS_OP_LIP3PLUS_REGISTRATION,
	PEGASUS_OP_LIE3PLUS_REGISTRATION,
	PEGASUS_OP_D2SEPLUS_REGISTRATION,
	PEGASUS_OP_SE2DPLUS_REGISTRATION,
	PEGASUS_OP_J2KP_REGISTRATION,
	PEGASUS_OP_J2KE_REGISTRATION,
	COMPRESSION_RGB_TRANSFORM_FORMAT,
	JPEG_LS_LOSSLESS_SYNTAX,
	JPEG_LS_LOSSY_SYNTAX,
	MPEG2_MPML_SYNTAX,

	/* char parms */
    LARGE_DATA_STORE,
    DICTIONARY_ACCESS,
    NULL_TYPE3_VALIDATION
} StringParm;


/* ======================================================================== *
 *                   Valid Logging Configuration Parameters                 *
 * ======================================================================== *
 *  These enumerated values may be used with the MC_Set_Log_Destination()   *
 *  function to change the value of message logging destination values.     *
 *  The names are the same as the parameter names in configuration files    */
typedef enum {
    ERROR_DESTINATIONS,
    WARNING_DESTINATIONS,
    INFO_DESTINATIONS,
    T1_DESTINATIONS,
    T2_DESTINATIONS,
    T3_DESTINATIONS,
    T4_DESTINATIONS,
    T5_DESTINATIONS,
    T6_DESTINATIONS,
    T7_DESTINATIONS,
    T8_DESTINATIONS,
    T9_DESTINATIONS
} LogParm;


/*
 *  Bit Masks for destination flags for the different message types
 *                         (DO NOT CHANGE!)
 */
#define File_Destination         ( (1 << 0) )
#define Memory_Destination       ( (1 << 1) )
#define Screen_Destination       ( (1 << 2) )
#define Bitbucket_Destination    ( (1 << 4) )




/* ======================================================================== *
 *                   Valid Integer Configuration Parameters                 *
 * ======================================================================== *
 *  These enumerated values may be used with the MC_Set_Int_Config_Value()  *
 *  function to change the value of integer configuration values.           *
 *  The names are the same as the parameter names in configuration files    */
typedef enum {
    LOG_FILE_SIZE,
    LOG_MEMORY_SIZE,
    ARTIM_TIMEOUT,
    ASSOC_REPLY_TIMEOUT,
    RELEASE_TIMEOUT,
    WRITE_TIMEOUT,
    CONNECT_TIMEOUT,
    INACTIVITY_TIMEOUT,
    LARGE_DATA_SIZE,
    DESIRED_LAST_PDU_SIZE,
    OBOW_BUFFER_SIZE,
	FLATE_GROW_OUTPUT_BUF_SIZE,
	DEFLATE_COMPRESSION_LEVEL,
	COMPRESSION_LUM_FACTOR,
	COMPRESSION_CHROM_FACTOR,
	COMPRESSION_J2K_LOSSY_RATIO,
	COMPRESSION_J2K_LOSSY_QUALITY,
    WORK_BUFFER_SIZE,
    TCPIP_LISTEN_PORT,
    MAX_PENDING_CONNECTIONS,
    TCPIP_SEND_BUFFER_SIZE,
    TCPIP_RECEIVE_BUFFER_SIZE,
    NUM_HISTORICAL_LOG_FILES,
    LOG_FILE_LINE_LENGTH,
    NUMBER_OF_CAP_FILES,
    IGNORE_JPEG_BAD_SUFFIX,
	//2012/06/01 K.KO add openAssociation retry number
	RETRY_NUMBER_OPEN_ASSOC,
	OPEN_ASSOC_TIMEOUT
} IntParm;



/* ======================================================================== *
 *                   Valid Boolean Configuration Parameters                 *
 * ======================================================================== *
 *  These enumerated values may be used with the MC_Set_Bool_Config_Value() *
 *  function to change the value of boolean configuration values.           *
 *  The names are the same as the parameter names in configuration files    */
typedef enum {
    LOG_FILE_BACKUP,
    ACCEPT_ANY_CONTEXT_NAME,
    ACCEPT_ANY_APPLICATION_TITLE,
    ACCEPT_ANY_PROTOCOL_VERSION,
    ACCEPT_DIFFERENT_IC_UID,
    ACCEPT_DIFFERENT_VERSION,
    AUTO_ECHO_SUPPORT,
    SEND_SOP_CLASS_UID,
    SEND_SOP_INSTANCE_UID,
    SEND_LENGTH_TO_END,
    SEND_RECOGNITION_CODE,
    SEND_MSG_ID_RESPONSE,
    SEND_ECHO_PRIORITY,
    SEND_RESPONSE_PRIORITY,
    INSURE_EVEN_UID_LENGTH,
    BLANK_FILL_LOG_FILE,
    ELIMINATE_ITEM_REFERENCES,
    HARD_CLOSE_TCP_IP_CONNECTION,
    FORCE_DICM_IN_PREFIX,
    ACCEPT_ANY_PRESENTATION_CONTEXT,
    ACCEPT_ANY_HOSTNAME,
    ALLOW_OUT_OF_ORDER_TAGS,
    EMPTY_PRIVATE_CREATOR_CODES,
    FORCE_OPEN_EMPTY_ITEM,
    FORCE_JAVA_BIG_ENDIAN,
    ACCEPT_MULTIPLE_PRES_CONTEXTS,
    ALLOW_INVALID_PRIVATE_CREATOR_CODES,
    REMOVE_PADDING_CHARS,
	REMOVE_SINGLE_TRAILING_SPACE,
    PRIVATE_SYNTAX_1_LITTLE_ENDIAN,
    PRIVATE_SYNTAX_1_EXPLICIT_VR,
    PRIVATE_SYNTAX_1_ENCAPSULATED,
    PRIVATE_SYNTAX_2_LITTLE_ENDIAN,
    PRIVATE_SYNTAX_2_EXPLICIT_VR,
    PRIVATE_SYNTAX_2_ENCAPSULATED,
    EXPORT_UN_VR_TO_MEDIA,
    EXPORT_UN_VR_TO_NETWORK,
    EXPORT_PRIVATE_ATTRIBUTES_TO_MEDIA,
    EXPORT_PRIVATE_ATTRIBUTES_TO_NETWORK,
    ALLOW_INVALID_PRIVATE_ATTRIBUTES,
    RETURN_COMMA_IN_DS_FL_FD_STRINGS,
    ALLOW_COMMA_IN_DS_FL_FD_STRINGS,
	DEFLATE_ALLOW_FLUSH,
	REMOVE_SQ,
	COMPRESSION_ALLOW_FRAGS,
    NETWORK_CAPTURE,
	DUPLICATE_ENCAPSULATED_ICON,
	COMPRESSION_WHEN_J2K_USE_LOSSY,
	COMPRESSION_J2K_LOSSY_USE_QUALITY,
	TCPIP_DISABLE_NAGLE,
	COMBINE_DATA_WITH_HEADER,
	EFILM_OPEN_FILE,
	MSG_FILE_ITEM_OBJ_TRACE,
    REWRITE_CAPTURE_FILES,
    EXPORT_UNDEFINED_LENGTH_SQ,
    EXPORT_UNDEFINED_LENGTH_SQ_IN_DICOMDIR,
    EXPORT_GROUP_LENGTHS_TO_NETWORK,
    EXPORT_GROUP_LENGTHS_TO_MEDIA
} BoolParm;



/* ======================================================================== *
 *                    Valid Long Configuration Parameters                   *
 * ======================================================================== *
 *  These enumerated values may be used with the MC_Set_Long_Config_Value() *
 *  function to change the value of long int configuration values.          *
 *  The names are the same as the parameter names in configuration files    */
typedef enum {
    PDU_MAXIMUM_LENGTH,
    CALLBACK_MIN_DATA_SIZE,
    CAPTURE_FILE_SIZE
} LongParm;



/* ======================================================================== *
 *          Codes used to identify the type of a callback function          *
 * ======================================================================== */
typedef enum {
    REQUEST_FOR_DATA = 300,
    REQUEST_FOR_DATA_LENGTH,
    PROVIDING_DATA,
    PROVIDING_DATA_LENGTH,
    PROVIDING_MEDIA_DATA_LENGTH,
    PROVIDING_OFFSET_TABLE
} CALLBACK_TYPE;


#if !defined(EXP_FUNC)
#define NOEXP_FUNC      __cdecl
#endif

///


#endif // !defined(AFX_IDICOM_LIB_MSG_H_)
 