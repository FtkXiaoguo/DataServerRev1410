/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Integrator's Tool Kit
 *
 *    $Workfile: mc3msg.h $
 *
 *    $Revision: 6658 $
 *
 *        $Date: 2002-11-14 04:13:11 +0900 (木, 14 11 2002) $
 *
 *  Description:   This module contains public definitions for using the
 *                 MergeCOM-3 Advanced Toolkit Library's Message Manipulation
 *                 routines. 
 *
 *************************************************************************
 *
 *		(c) 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)
 *                     Milwaukee, Wisconsin  53214
 *
 *                      -- ALL RIGHTS RESERVED --
 *
 *  This software is furnished under license and may be used and copied
 *  only in accordance with the terms of such license and with the
 *  inclusion of the above copyright notice.  This software or any other
 *  copies thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of the software is hereby
 *  transferred.
 *
 ************************************************************************/
/* $Nokeywords$ */


#ifndef MTI_MC3MSG_H
#define MTI_MC3MSG_H

#if !defined(EXP_FUNC)
#if defined(WIN311)
#define EXP_FUNC        __far __pascal __export
#define EXP_FUNC_CDECL  __far __export
#define NOEXP_FUNC      __far __pascal
#define EXP_PRE
#elif defined(INTEL_BCC)
#define EXP_FUNC        __export
#define EXP_FUNC_CDECL  __export
#define NOEXP_FUNC 
#define EXP_PRE
#elif defined(INTEL_VB)
#define EXP_FUNC        __stdcall
#define EXP_FUNC_CDECL 
#define NOEXP_FUNC      __stdcall  
#define EXP_PRE         __declspec(dllexport)
#elif defined(_WIN32)
#define EXP_FUNC        __cdecl
#define EXP_FUNC_CDECL  __cdecl
#define NOEXP_FUNC      __cdecl  
#define EXP_PRE         
#else
#define EXP_FUNC
#define EXP_FUNC_CDECL
#define NOEXP_FUNC
#define EXP_PRE
#endif
#endif

#include <stdio.h>
#include "mcstatus.h"
      
#ifdef __cplusplus
extern "C" {
#endif 

#ifdef _MACINTOSH
#pragma enumsalwaysint on
#pragma options align=mac68k
#endif

#ifdef __BORLANDC__
#pragma option -b
#endif

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
#if defined (VX_COLDFIRE)
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
    PRIVATE_SYNTAX_1,
    PRIVATE_SYNTAX_2

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
    WORK_BUFFER_SIZE,
    TCPIP_LISTEN_PORT,
    MAX_PENDING_CONNECTIONS,
    TCPIP_SEND_BUFFER_SIZE,
    TCPIP_RECEIVE_BUFFER_SIZE,
    NUM_HISTORICAL_LOG_FILES,
    LOG_FILE_LINE_LENGTH,
    NUMBER_OF_CAP_FILES
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
	COMPRESSION_ALLOW_FRAGS,
    NETWORK_CAPTURE,
    REWRITE_CAPTURE_FILES
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




/* ======================================================================== *
 *                  Functions to set configuration values                   *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_String_Config_Value(
                                                 StringParm     Aparm,
                                                 char*          Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Bool_Config_Value(  
                                                 BoolParm       Aparm,
                                                 int            Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Long_Config_Value(  
                                                 LongParm       Aparm,
                                                 long int       Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Int_Config_Value(   
                                                 IntParm        Aparm,
                                                 int            Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Log_Destination(
                                                 LogParm        Aparm,
                                                 int            Avalue);


/* ======================================================================== *
 *                  Functions to get configuration values                   *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_String_Config_Value(
                                                 StringParm     Aparm,
                                                 int            AbufferSize,
                                                 char*          Abuffer);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Bool_Config_Value(
                                                 BoolParm       Aparm,
                                                 int*           Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Long_Config_Value(
                                                 LongParm       Aparm,
                                                 long int*      Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Int_Config_Value(
                                                 IntParm        Aparm,
                                                 int*           Avalue);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Log_Destination(
                                                 LogParm        Aparm,
                                                 int*           Avalue);


/* ======================================================================== *
 *                  Functions to (re-)initialize the library                *
 * ======================================================================== */
EXP_PRE extern MC_STATUS    EXP_FUNC MC_Library_Initialization(
                                                 void*(*AcfgFunction)(void),
                                                 void*(*AdictFunction)(void),
                                                 void*(*AinfoFunction)(void));
EXP_PRE extern MC_STATUS    EXP_FUNC MC_Library_Reset    (void);
EXP_PRE extern MC_STATUS    EXP_FUNC MC_Library_Release  (void);

#ifdef SINGLE_MEMORY_BUFFER
EXP_PRE extern MC_STATUS    EXP_FUNC MC_Memory_Initialization( 
                                                 unsigned long AbufferSize);
#endif
EXP_PRE extern MC_STATUS    EXP_FUNC MC_Set_MergeINI     ( char *Afilename );

EXP_PRE extern MC_STATUS    EXP_FUNC MC_Get_Version_String( 
                                                 int            AbufferSize,
                                                 char*          Abuffer);



/* ======================================================================== *
 *                Default function names from source generators             *
 * ======================================================================== */
extern void* MC_Config_Values(void);
extern void* MC_Dictionary_Values(void);
extern void* MC_MsgInfo_Values(void);



/* ======================================================================== *
 *                      Functions to open or free messages                  *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Free_Item  (int*           AitemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Free_Message (int*           AmessageID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Open_Empty_Message (int*           AmessageID);
                                             
EXP_PRE extern MC_STATUS EXP_FUNC MC_Open_Item  (int*           AitemID,
                                                 const char*    AitemName);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Open_Message (int*           AmessageID,
                                                 const char*    AserviceName,
                                                 MC_COMMAND     Acommand);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Service_Command(int            AmessageID,
                                                 const char*    AserviceName,
                                                 MC_COMMAND     Acommand);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Duplicate_Message (int AsourceID, int* AdestID, 
                                                        TRANSFER_SYNTAX AdestTransferSyntax, 
                                                        MC_STATUS   (NOEXP_FUNC *Acompression_Callback)(
                                                                    int             CBmsgID, 
                                                                    void**          CBContext, 
                                                                    unsigned long   CBdataLen,
                                                                    void*           CBdataValue,
                                                                    unsigned long*  CBoutdataLen,
                                                                    void**          CBoutdataValue,
                                                                    int             CBisFirst,
                                                                    int             CBisLast,
                                                                    int             CBrelease),
                                                        MC_STATUS   (NOEXP_FUNC *Adecompression_Callback)(
                                                                    int             CBmsgID, 
                                                                    void**          CBContext, 
                                                                    unsigned long   CBdataLen,
                                                                    void*           CBdataValue,
                                                                    unsigned long*  CBoutdataLen,
                                                                    void**          CBoutdataValue,
                                                                    int             CBisFirst,
                                                                    int             CBisLast,
                                                                    int             CBrelease) );
                                            

/* ======================================================================== *
 *                Functions to add or remove message attributes             *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Add_Standard_Attribute(int            AmsgID,
                                                 unsigned long  Atag);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Add_Nonstandard_Attribute(int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_VR          Avr);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Add_Private_Attribute(int            AmsgID, 
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  AelementByte,
                                                 MC_VR          Avr);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Add_Private_Block  (int            AmsgID, 
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup);
                         
                           
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Delete_Attribute   (int            AmsgID,
                                                 unsigned long  Atag);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Delete_Range       (int            AmsgID,
                                                 unsigned long  AfirstTag,  
                                                 unsigned long  AlastTag);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Delete_Current_Value(int            AmsgID,
                                                 unsigned long  Atag);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Delete_Private_Attribute(int            AmsgID, 
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  AelementByte);  

EXP_PRE extern MC_STATUS EXP_FUNC MC_Delete_Private_Block(int            AmsgID, 
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup);  


/* ======================================================================== *
 *              Functions to retrieve message attribute values              *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_Length   (int            AmsgID,
                                                 unsigned long  Atag,
                                                 int            AvalueNumber, 
                                                 unsigned long* Alength);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value          (int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_DT          AdataType, 
                                                 int            AbufferSize,
                                                 void*          Abuffer,
                                                 int*           AvalueSize);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_Count    (int            AmsgID, 
                                                 unsigned long  Atag, 
                                                 int*           Acount);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_Buffer(int            AmsgID, 
                                                 unsigned long  Atag,
                                                 int            AbufferSize, 
                                                 void*          Abuffer,
                                                 int*           AvalueSize);
                             
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_Double(int            AmsgID,
                                                 unsigned long  Atag,
                                                 double*        Abuffer);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_Float (int            AmsgID,
                                                 unsigned long  Atag,
                                                 float*         Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_Int   (int            AmsgID,
                                                 unsigned long  Atag,
                                                 int*           Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_LongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 long int*      Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_ShortInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 short int*     Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_String(int            AmsgID, 
                                                 unsigned long  Atag,
                                                 int            AbufferSize, 
                                                 char*          Abuffer);
                    
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_UInt  (int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned int*  Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long* Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_UShortInt(int             AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned short* Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Value_To_Function(int            AmsgID,
                                                unsigned long   Atag,
                                                void*           AuserInfo, 
                                                MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     void*          CBuserInfo,
                                                     int            CBdataSize,
                                                     void*          CBdataBuffer,
                                                     int            CBisFirst,
                                                     int            CBisLast)); 
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Encapsulated_Value_To_Function(int            AmsgID,
                                                unsigned long   Atag,
                                                void*           AuserInfo, 
                                                MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     void*          CBuserInfo,
                                                     int            CBdataSize,
                                                     void*          CBdataBuffer,
                                                     int            CBisFirst,
                                                     int            CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Encapsulated_Value_To_Function(int            AmsgID,
                                                unsigned long   Atag,
                                                void*           AuserInfo, 
                                                MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     void*          CBuserInfo,
                                                     int            CBdataSize,
                                                     void*          CBdataBuffer,
                                                     int            CBisFirst,
                                                     int            CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Byte_Swap_OBOW    (int            AmsgID,
                                                unsigned long   Atag); 
                                                                     
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value     (int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_DT          AdataType, 
                                                 int            AbufferSize,
                                                 void*          Abuffer,
                                                 int*           AvalueSize);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_Double(int            AmsgID,
                                                 unsigned long  Atag,
                                                 double*        Abuffer);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_Float(int            AmsgID,
                                                 unsigned long  Atag,
                                                 float*         Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_Int(int            AmsgID,
                                                 unsigned long  Atag,
                                                 int*           Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_LongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 long int*      Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_ShortInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 short int*     Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_String(int            AmsgID, 
                                                 unsigned long  Atag,
                                                 int            bufferSize, 
                                                 char*          Abuffer);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_UInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned int*  Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long* Abuffer);
                                                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Value_To_UShortInt(int             AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned short* Abuffer);
                        

/* ======================================================================== *
 *              Functions to retrieve PRIVATE attribute values              *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_Length  (int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int            AvalueNumber, 
                                                 unsigned long* Alength);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue         (int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 MC_DT          AdataType, 
                                                 int            AbufferSize,
                                                 void*          Abuffer,
                                                 int*           AvalueSize);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_Count   (int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int*           Acount);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_Buffer(int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int            AbufferSize, 
                                                 void*          Abuffer,
                                                 int*           AvalueSize);
                             
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_Double(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 double*        Abuffer);
                                                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_Float(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 float*         Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_Int  (int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int*           Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_ShortInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 short int*     Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_LongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 long int*      Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_String(int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int            AbufferSize, 
                                                 char*          Abuffer);
                    
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_UInt (int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned int*  Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_UShortInt(int             AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 unsigned short* Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_ULongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned long* Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pValue_To_Function(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 void*           AuserInfo, 
                                                 MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     void*          CBuserInfo,
                                                     int            CBdataSize,
                                                     void*          CBdataBuffer,
                                                     int            CBisFirst,
                                                     int            CBisLast)); 
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue    (int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 MC_DT           AdataType, 
                                                 int             AbufferSize,
                                                 void*           Abuffer,
                                                 int*            AvalueSize);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_Double(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem,
                                                 double*         Abuffer);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_Float(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 float*          Abuffer);
                                                                 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_Int(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 int*            Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_ShortInt(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 short int*      Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_LongInt(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 long int*       Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_String(int            AmsgID, 
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 int             AbufferSize, 
                                                 char*           Abuffer);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_UInt(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 unsigned int*   Abuffer);
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_ULongInt(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 unsigned long*  Abuffer);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_pValue_To_UShortInt(int            AmsgID,
                                                 const char*     AprivateCode, 
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem, 
                                                 unsigned short* Abuffer);
                        


/* ======================================================================== *
 *              Functions to supply message attribute values                *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Empty_Message      (int            AmsgID);
                                                                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Empty_Item         (int            AitemID);
 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_To_Empty (int            AmsgID, 
                                                 unsigned long   Atag);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_To_NULL  (int            AmsgID, 
                                                 unsigned long  Atag);

                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value          (int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_DT          AdataType,
                                                 void*          Avalue);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_Double(int            AmsgID,
                                                 unsigned long   Atag,
                                                 double          Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_Float(int            AmsgID,
                                                 unsigned long   Atag,
                                                 float           Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_Int (int            AmsgID,
                                                 unsigned long   Atag,
                                                 int             Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_ShortInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 short           Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_LongInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 long            Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_String(int            AmsgID,
                                                 unsigned long   Atag,
                                                 const char*     Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_UInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned int    Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_UShortInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned short Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long  Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_From_Function(
                                                 int            AmsgID,
                                                 unsigned long  Atag,
                                                 void*          AuserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     int            CBfirstCall,
                                                     void*          CBuserInfo,
                                                     int*           CBdataLen,
                                                     void**         CBdataBuffer,
                                                     int*           CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Encapsulated_Value_From_Function(
                                                 int            AmsgID,
                                                 unsigned long  Atag,
                                                 void*          AuserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     int            CBfirstCall,
                                                     void*          CBuserInfo,
                                                     int*           CBdataLen,
                                                     void**         CBdataBuffer,
                                                     int*           CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Encapsulated_Value_From_Function(
                                                 int            AmsgID,
                                                 unsigned long  Atag,
                                                 void*          AuserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     int            CBfirstCall,
                                                     void*          CBuserInfo,
                                                     int*           CBdataLen,
                                                     void**         CBdataBuffer,
                                                     int*           CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Close_Encapsulated_Value (  
                                        int                             AmsgID,
                                        unsigned long   Atag);



EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_To_NULL(int            AmsgID, 
                                                 unsigned long  Atag);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value     (int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_DT          AdataType,
                                                 void*          Avalue);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_Double(int            AmsgID,
                                                unsigned long   Atag,
                                                double          Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_Float(int            AmsgID,
                                                unsigned long   Atag,
                                                float           Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_Int(int            AmsgID,
                                                unsigned long   Atag,
                                                int             Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_ShortInt(int            AmsgID,
                                                unsigned long   Atag,
                                                short           Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_LongInt(int            AmsgID,
                                                unsigned long   Atag,
                                                long            Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_String(int            AmsgID,
                                                unsigned long   Atag,
                                                const char*     Avalue);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_UInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned int   Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_UShortInt(int           AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned short Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_Value_From_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long  Avalue);
                        


/* ======================================================================== *
 *              Functions to supply PRIVATE attribute values                *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_To_Empty(int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_To_NULL (int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem);


                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue (int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 MC_DT          AdataType,
                                                 void*          Avalue);
                                                                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_Double(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 double         Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_Float(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 float          Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_Int(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int            Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_ShortInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 short          Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_LongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 long           Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_String(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 const char*    Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_UInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned int   Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_UShortInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned short Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_ULongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned long  Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_From_Function(
                                                 int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 void*          AuserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     int            CBfirstCall,
                                                     void*          CBuserInfo,
                                                     int*           CBdataLen,
                                                     void**         CBdataBuffer,
                                                     int*           CBisLast)); 


EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_To_NULL(int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 MC_DT          AdataType,
                                                 void*          Avalue);
                                                 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_Double(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 double         Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_Float(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 float          Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_Int(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int            Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_ShortInt(int           AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 short          Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_LongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 long           Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_String (int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 const char*    Avalue);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_UInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned int   Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_UShortInt(int          AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned short Avalue);
                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Next_pValue_From_ULongInt(int           AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 unsigned long  Avalue);
                        

/* ======================================================================== *
 *            Functions for implementing registered callbacks               *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Register_Callback_Function (int        ApplicationID,
                                                 unsigned long  Tag,
                                                 void*          UserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *Acallback)  
                                                    (int            CBmsgID, 
                                                     unsigned long  CBtag,
                                                     void*          CBuserInfo, 
                                                     CALLBACK_TYPE  CBtype, 
                                                     unsigned long* CBdataSizePtr,
                                                     void**         CBdataBufferPtr,
                                                     int            CBisFirst,
                                                     int*           CBisLast)); 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Release_Callback_Function(
                                                 int            ApplicationID,
                                                 unsigned long  Atag);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Register_pCallback_Function(
                                                 int            ApplicationID,
                                                 char*          AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  AelementByte,
                                                 void*          AuserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *Acallback)  
                                                     (int           CBmsgID, 
                                                      char*         CBprivateCode,
                                                      unsigned short CBgroup,
                                                      unsigned char CBelementByte,
                                                      void*         CBuserInfo, 
                                                      CALLBACK_TYPE CBtype, 
                                                      unsigned long* CBdataSizePtr,
                                                      void**        CBdataBufferPtr,
                                                      int           CBisFirst,
                                                      int*          CBisLast));
EXP_PRE extern MC_STATUS EXP_FUNC MC_Release_pCallback_Function(
                                                 int            ApplicationID,
                                                 char*          AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  AelementByte);
                                                  
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Message_Callbacks(     
                                                 int            ApplicationID,
                                                 int            AmsgID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Register_Compression_Callbacks(
                                                 int            AmsgID,
                                                MC_STATUS   (NOEXP_FUNC *Acompression_Callback)(
                                                        int             CBmsgID, 
                                                        void**          CBContext, 
                                                        unsigned long   CBdataLen,
                                                        void*           CBdataValue,
                                                        unsigned long*  CBoutdataLen,
                                                        void**          CBoutdataValue,
                                                        int             CBisFirst,
                                                        int             CBisLast,
                                                        int             CBrelease),
                                                MC_STATUS   (NOEXP_FUNC *Adecompression_Callback)(    
                                                        int             CBmsgID, 
                                                        void**          CBContext, 
                                                        unsigned long   CBdataLen,
                                                        void*           CBdataValue,
                                                        unsigned long*  CBoutdataLen,
                                                        void**          CBoutdataValue,
                                                        int             CBisFirst,
                                                        int             CBisLast,
                                                        int             CBrelease)
                                                 );

/* ======================================================================== *
 *     Function to provide streaming of message data to/from the message    *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Stream_To_Message(
                                            int             AmsgID,
                                            unsigned long   AstartTag,
                                            unsigned long   AstopTag, 
                                            TRANSFER_SYNTAX Asyntax,
                                            unsigned long*  AerrorTag, 
                                            void*           AuserInfo,
                                            MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int        CBmsgID,
                                                     void*      CBuserInfo,
                                                     int        CBfirstCall,
                                                     int*       CBdataLen,
                                                     void**     CBdataBuffer,
                                                     int*       CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Stream_To_Message_With_Offset(
                                            int             AmsgID,
                                            unsigned long   AstartTag,
                                            unsigned long   AstopTag, 
                                            TRANSFER_SYNTAX Asyntax,
                                            unsigned long*  AerrorTag, 
                                            unsigned long*  Aoffset, 
                                            void*           AuserInfo,
                                            MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int        CBmsgID,
                                                     void*      CBuserInfo,
                                                     int        CBfirstCall,
                                                     int*       CBdataLen,
                                                     void**     CBdataBuffer,
                                                     int*       CBisLast));  

EXP_PRE extern MC_STATUS EXP_FUNC MC_Message_To_Stream( 
                                            int             AmsgID,
                                            unsigned long   AstartTag,
                                            unsigned long   AstopTag,
                                            TRANSFER_SYNTAX Asyntax,
                                            void*           AuserInfo, 
                                            MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int        CBmsgID,
                                                     void*      CBuserInfo,
                                                     int        CBdataSize,
                                                     void*      CBdataBuffer,
                                                     int        CBisFirst, 
                                                     int        CBisLast)); 
                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Stream_Length  (int            AmsgID,
                                                 unsigned long  AstartTag,
                                                 unsigned long  AstopTag, 
                                                 unsigned long* AmsgLength,
                                                 TRANSFER_SYNTAX Asyntax);

/* ======================================================================== *
 *      Function to retrieve a message's service name and command           *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Message_Service(int    AmsgID, 
                                                 char**         AserviceName, 
                                                 MC_COMMAND*    Acommand);

EXP_PRE extern MC_STATUS MC_Get_UID_From_MergeCOM_Service(
                                                 const char*    AserviceName,
                                                 char*          Auid,
                                                 int            AbufferSize );

EXP_PRE extern MC_STATUS MC_Get_Transfer_Syntax_From_Enum(
                                                 TRANSFER_SYNTAX Asyntax,
                                                 char*          Auid,
                                                 int            AbufferSize );

EXP_PRE extern MC_STATUS MC_Get_Enum_From_Transfer_Syntax(
                                                 const char*    Auid,
                                                 TRANSFER_SYNTAX* Asyntax );


/* ======================================================================== *
 *    Function to retrieve the MergeCOM Service name for a SOP Class UID    *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_MergeCOM_Service(const char*   Auid, 
                                                 char*          Aname, 
                                                 int            Alength);

/* ======================================================================== *
 *  Functions to retrieve a string describing a DICOM or private attribute  *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Tag_Info (unsigned long  Atag,
                                                 char*          Aname,
                                                 int            Alength);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pTag_Info (const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 char*          Aname,
                                                 int            Alength);
                                                 
/* ======================================================================== *
 *        Functions to retrieve info about a message's attributes           *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Attribute_Info (int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_VR*         Avr,
                                                 int*           Avalues);
                         
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_pAttribute_Info(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 MC_VR*         Avr,
                                                 int*           Avalues);
                                                 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_First_Attribute(int            AmsgID, 
                                                 unsigned long* Atag,
                                                 MC_VR*         Avr,
                                                 int*           Avalues);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Attribute (int            AmsgID, 
                                                 unsigned long* Atag,
                                                 MC_VR*         Avr,
                                                 int*           Avalues);


/* ======================================================================== *
 *        Functions to set an attribute's Value Representation Code         *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_Value_Representation(int            AmsgID, 
                                                 unsigned long  Atag,
                                                 MC_VR          Avr);
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_pValue_Representation(int            AmsgID, 
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 MC_VR          Avr);

#ifndef NO_FILE_SYSTEM
/* ======================================================================== *
 *          Function to display the contents of a message or item object    *
 * ======================================================================== */
#if defined (__OS2__) || defined(_WIN32)
EXP_PRE extern void       EXP_FUNC MC_List_Message (int            AmsgID, 
                                                 const char*    AfileName);
EXP_PRE extern void       EXP_FUNC MC_List_Item (int            AitemID, 
                                                 const char*    AfileName);
#else
EXP_PRE extern void       EXP_FUNC MC_List_Message (int            AmsgID, 
                                                 FILE*          Afp);
EXP_PRE extern void       EXP_FUNC MC_List_Item (int            AitemID, 
                                                 FILE*          Afp);
#endif
#endif

/* ======================================================================== *
 *                       Message Validation Functions                       *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_Validate_Message   (int            AmsgID,
                                                 VAL_ERR**      AerrInfo,
                                                 VAL_LEVEL      AerrLevel);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Validate_Attribute (int           AmsgID,
                                                 unsigned long  Atag,
                                                 VAL_ERR**      AerrInfo,
                                                 VAL_LEVEL      AerrLevel );
                                           
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Next_Validate_Error(int            AmsgID,
                                                 VAL_ERR**      AerrInfo);


/* ======================================================================== *
 *                       Status message log functions                       *
 * ======================================================================== */
EXP_PRE extern MC_STATUS EXP_FUNC MC_MemoryLog_To_Function(int            AclearLog,
                                                 MC_STATUS(*AuserFunction)
                                                            (char*  LogLine,
                                                             int    CBisFirst,
                                                             int    CBisLast));

EXP_PRE extern void EXP_FUNC MC_Register_MemoryLog_Function(void (*AmemLogFunction)
                                                            (MsgLogType Arg1,
                                                             char*      Arg2));
EXP_PRE extern void EXP_FUNC MC_Register_Enhanced_MemoryLog_Function(
                                                            void (*AmemLogFunction)
                                                            (LogInfo* Arg1));

EXP_PRE extern void EXP_FUNC MC_Set_Log_Prefix  (const char*    Aprefix);

#ifdef __BORLANDC__
#pragma option -b.
#endif

#ifdef _MACINTOSH
#pragma enumsalwaysint reset
#pragma options align=reset
#endif
                        
#ifdef __cplusplus
}
#endif 

#endif

