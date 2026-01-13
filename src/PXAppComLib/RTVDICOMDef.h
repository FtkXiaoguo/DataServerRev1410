/***********************************************************************
 * $Id: RTVDICOMDef.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Some common DICOM definations
 *
 *	
 *   
 *-------------------------------------------------------------------
 */
#ifndef RTVDICOMDEF_H
#define RTVDICOMDEF_H

//-----------------------------------------------------------------------------
//
typedef enum 
{
	kDicomImage = 1,
	kInteractiveReport,
	kSetFile,
	kCaScore,
	kSecondaryCapture,
	kUnknownObjectType
} DicomObjectType;

//
//	Some max lengths for DICOM Value Representations - See DICOM PS3.5 table 6.2-1
//
const int kVR_AE = 16 + 1; 
const int kVR_AS = 4 + 1;
const int kVR_AT = 4 + 1;
const int kVR_CS = 16 + 1;
const int kVR_DA = 17 + 1;
const int kVR_DS = 16 + 1;
const int kVR_DT = 26 + 1;
const int kVR_FL = 4 + 1;
const int kVR_FD = 8 + 1;
const int kVR_IS = 12 + 1;
const int kVR_LO = 64 + 1;
const int kVR_LT = 10240 + 1;
const int kVR_PN = 320 + 1;
const int kVR_SH = 16 + 1;
const int kVR_SL = 4 + 1;
const int kVR_SS = 2 + 1;
const int kVR_ST = 1024 + 1;
const int kVR_TM = 16 + 1;
const int kVR_UI = 64 + 1;
const int kVR_UL = 4 + 1;
const int kVR_US = 2 + 1;

const int kMediaLabel = 16+1;

#define C_STORE_FAILURE_DUPLICATE_SOP      0x0111

#endif // RTVDICOMDEF_H