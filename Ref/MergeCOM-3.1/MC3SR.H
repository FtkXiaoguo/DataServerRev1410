/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Integrator's Tool Kit
 *
 *    $Workfile: mc3sr.h $
 *
 *    $Revision: 6914 $
 *
 *        $Date: 2002-12-14 03:41:22 +0900 (土, 14 12 2002) $
 *
 *       Author: Merge Technologies, Inc.
 *
 *  Description: This module contains public definitions for using the
 *               MergeCOM-3 Advanced Toolkit Library's Media routines.
 *
 *
 *************************************************************************
 *
 *                       COPYRIGHT (C) 2000
 *                   Merge Technologies Incorporated
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



#ifndef MTI_MC3SR_H
#define MTI_MC3SR_H

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
 *             Functions to navigate through DICOMDIR structure             *
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

