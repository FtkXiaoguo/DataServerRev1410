/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Integrator's Tool Kit
 *
 *    $Workfile: mc3media.h $
 *
 *    $Revision: 6658 $
 *
 *        $Date: 2002-11-14 04:13:11 +0900 (木, 14 11 2002) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This module contains public definitions for using the
 *               MergeCOM-3 Advanced Toolkit Library's Media routines.
 *
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



#ifndef MTI_MC3MEDIA_H
#define MTI_MC3MEDIA_H

#include <stdio.h>
#include "mcstatus.h"
#include "mc3msg.h"
      
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

/* ======================================================================== *
 *                Message Media Group Attribute Definitions                 *
 * ======================================================================== */
#define MEDIA_GROUP_LENGTH               0x00020000
#define MEDIA_FILE_META_INFO_VER         0x00020001
#define MEDIA_STORAGE_SOP_CLASS_UID      0x00020002
#define MEDIA_STORAGE_SOP_INSTANCE_UID   0x00020003
#define MEDIA_TRANSFER_SYNTAX_UID        0x00020010
#define MEDIA_IMPLEMENTATION_CLASS_UID   0x00020012
#define MEDIA_IMPLEMENTATION_VER_NAME    0x00020013
#define MEDIA_SOURCE_APPLICATION_ENTITY  0x00020016
#define MEDIA_PRIVATE_INFO_CREATOR_UID   0x00020100
#define MEDIA_PRIVATE_INFO               0x00020102




/* ======================================================================== *
 *                   Functions to create or free media files                *
 * ======================================================================== */

                                            
EXP_PRE extern MC_STATUS EXP_FUNC MC_Create_File
                                       (int*            AfileID, 
                                        const char*     AfileName, 
                                        const char*     AserviceName, 
                                        MC_COMMAND      Acommand); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Create_Empty_File  
                                       (int*            AfileID,
                                        const char*     AfileName);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Empty_File
                                       (int             AfileID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Free_File
                                       (int*            AfileID);


/* ======================================================================== *
 *                 Functions to stream data to or from a file               *
 * ======================================================================== */


EXP_PRE extern MC_STATUS EXP_FUNC MC_Write_File(int             AfileID, 
                                        int             AnumBytes, 
                                        void*           AuserInfo, 
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int         CBdataSize,
                                                void*       CBdataBuffer,
                                                int         CBisFirst, 
                                                int         CBisLast)); 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Write_File_By_Callback 
                                       (int             AapplicationID,
                                        int             AfileID, 
                                        int             AnumBytes, 
                                        void*           AuserInfo, 
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int         CBdataSize,
                                                void*       CBdataBuffer,
                                                int         CBisFirst, 
                                                int         CBisLast)); 
                                                
EXP_PRE extern MC_STATUS EXP_FUNC MC_Open_File (int             AapplicationID,
                                        int             AfileID,  
                                        void*           AuserInfo, 
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int*        CBdataSize,
                                                void**      CBdataBuffer,
                                                int         CBisFirst,
                                                int*        CBisLast)); 

EXP_PRE extern MC_STATUS EXP_FUNC MC_Open_File_Upto_Tag 
                                       (int             AapplicationID,
                                        int             AfileID,  
                                        void*           AuserInfo, 
                                        unsigned long   Atag,
                                        long*           Aoffset,
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int*        CBdataSize,
                                                void**      CBdataBuffer,
                                                int         CBisFirst,
                                                int*        CBisLast)); 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Open_File_Bypass_OBOW
                                       (int             AapplicationID,
                                        int             AfileID,  
                                        void*           AuserInfo, 
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int*        CBdataSize,
                                                void**      CBdataBuffer,
                                                int         CBisFirst,
                                                int*        CBisLast)); 



/* ======================================================================== *
 *                    Functions for general use of files                    *
 * ======================================================================== */

#ifndef NO_FILE_SYSTEM
#if defined (__OS2__) || defined(_WIN32)
EXP_PRE extern void      EXP_FUNC MC_List_File
                                       (int             AfileID,  
                                        const char*     AfileName);
#else
EXP_PRE extern void      EXP_FUNC MC_List_File
                                       (int             AfileID,  
                                        FILE*           Afp);
#endif
#endif

EXP_PRE extern MC_STATUS EXP_FUNC MC_Validate_File
                                       (int             AfileID,  
                                        VAL_ERR**       AerrInfo, 
                                        VAL_LEVEL       AerrLevel);
 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_File_Preamble  
                                       (int             AfileID,
                                        char*           Apreamble);
 
EXP_PRE extern MC_STATUS EXP_FUNC MC_Set_File_Preamble  
                                       (int             AfileID,
                                        const char*     Apreamble);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Reset_Filename     
                                       (int             AfileID, 
                                        const char*     AnewFileName);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_Filename 
                                       (int             AfileID, 
                                        char*  	        AfileName,
                                        int             AbufferSize);

EXP_PRE extern MC_STATUS EXP_FUNC MC_File_To_Message 
                                       (int             AfileID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Message_To_File 
                                       (int             AmsgID,
                                        const char*     AfileName);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Get_File_Length 
                                       (int             AfileID,
                                        unsigned long*  AmsgLength);

/* ======================================================================== *
 *             Functions to navigate through DICOMDIR structure             *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Root_Entity
                                       (int             AdirID,
                                        int*            AentityID, 
                                        int*            AitemID, 
                                        char**          AitemType,
                                        int*            AisLast);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Next_Entity
                                       (int             AdirID,
                                        int             AitemID, 
                                        int*            AnextEntityID, 
                                        int*            AnextItemID, 
                                        char**          AnextItemType,
                                        int*            AisLast);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_First_Record
                                       (int             AdirID,
                                        int             AentityID, 
                                        int*            AitemID, 
                                        char**          AitemType, 
                                        int*            AisLast);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Next_Record
                                       (int             AdirID,
                                        int             AentityID, 
                                        int*            AitemID, 
                                        char**          AitemType, 
                                        int*            AisLast);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Add_Entity     
                                       (int             AdirID, 
                                        int             AitemID,
                                        const char*     AnewItemType,
                                        int*            AnewEntityID, 
                                        int*            AnewItemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Delete_Referenced_Entity
                                       (int    AdirID,
                                        int             AitemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Add_Record     
                                       (int             AdirID, 
                                        int             AentityID, 
                                        const char*     AnewItemType,
                                        int*            AnewItemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Delete_Record  
                                       (int             AdirID,
                                        int             AitemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Root_Count
                                       (int    AdirID,
                                        int*   Acount);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Entity_Count
                                       (int    AdirID,
                                        int    AentityID,
                                        int*   Acount);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Item_Count
                                       (int    AdirID,
                                        int    AitemID,
                                        int*   Acount);


/* ======================================================================== *
 *             Functions manipulate MRDR records                            *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Open_MRDR      
                                       (int             AdirID,
                                        int             ArefItemID,
                                        const char*     AnewMrdrType,
                                        int*            AnewMrdrID);
                                        
EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Reference_MRDR 
                                       (int             AdirID,
                                        int             ArefItemID,
                                        int             AmrdrID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_Dir_Remove_Ref_MRDR
                                       (int             AdirID,
                                        int             ArefItemID);
                                        
/* ======================================================================== *
 *             Functions for SR									            *
 * ======================================================================== */

EXP_PRE extern MC_STATUS EXP_FUNC MC_Message_To_SR
                                       (int             AmsgID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_To_Message
                                       (int             AsrID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Get_Root
                                       (int             AsrID,
                                        int*            AitemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Add_Child
                                       (int             AsrID,
                                        int             AitemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Add_Root
                                       (int             AsrID,
                                        int             *AitemID);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Get_First_Child
                                       (int             AsrID,
                                        int*            AitemID,
                                        int*            AisLast);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Get_Next_Child
                                       (int             AsrID,
                                        int*            AnextChildID,
                                        int*            AisLast);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Get_Location
                                       (int             AsrID,
                                        int             AsizeOfLocation,
                                        char*           Alocation);

EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Delete_Child
                                       (int             AsrID);


EXP_PRE extern MC_STATUS EXP_FUNC MC_SR_Get_Child_Count
                                        ( int  AsrID,
                                          int  *AchildCount);


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

