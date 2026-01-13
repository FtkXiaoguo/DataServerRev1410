/***********************************************************************
 *  VLIDicomConfig.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		A wrapper on top of MergeCOM config parameters
 *
 *	AUTHOR(S):  Rob Lewis, June 2001
 *   
 *-------------------------------------------------------------------
 */ 
#ifndef VLI_DICOM_CONFIG_H
#define VLI_DICOM_CONFIG_H

enum
{
	kUnset = 0,
	kDoNothing,
	kPopulate,
	
};

const int kMaxConfigLen = 256;

const int kConfigStringParmOffset = 0;
const int kConfigIntParmOffset = 100;
const int kConfigBoolParmOffset = 200;
const int kConfigLongParmOffset = 300;
const int kConfigVLIParmOffset = 400;


enum ConfigVLIParm {
	kcLocalAeTitle = kConfigVLIParmOffset,
	kcLocalCache,
	kcAutoCache,
    kcTISendSopClassUid,
    kcTISendSopInstanceUid,
    kcTISendMsgIdResponse,
    kcTISendResponsePriority,
	kcTimeoutOnQuery,
	kcRetrieveStoresOriginals,
	kcRemoveDuplicateQueryResults,
	kcNumberOfRetrieveThreads,
	kcForceTransferSyntax,
	kcMaxDICOMQueryResults,
	kcConnectCMoveBySeriesUID,
	kcTimeoutOnRetrieve
};

enum ConfigStringParm {
    kcLogFile = kConfigStringParmOffset,
    kcLicense,
    kcImplementationClassUid,
    kcImplementationVersion,
    kcLocalApplContextName,
    kcImplicitLittleEndianSyntax,
    kcImplicitBigEndianSyntax,
    kcExplicitLittleEndianSyntax,
    kcExplicitBigEndianSyntax,
    kcRleSyntax,                         
    kcJpegBaselineSyntax,               
    kcJpegExtended24Syntax,           
    kcJpegExtended35Syntax,           
    kcJpegSpecNonHier68Syntax,      
    kcJpegSpecNonHier79Syntax,      
    kcJpegFullProgNonHier1012Syntax,    
    kcJpegFullProgNonHier1113Syntax,    
    kcJpegLosslessNonHier14Syntax,        
    kcJpegLosslessNonHier15Syntax,        
    kcJpegExtendedHier1618Syntax,         
    kcJpegExtendedHier1719Syntax,        
    kcJpegSpecHier2022Syntax,             
    kcJpegSpecHier2123Syntax,             
    kcJpegFullProgHier2426Syntax,        
    kcJpegFullProgHier2527Syntax,        
    kcJpegLosslessHier28Syntax,       
    kcJpegLosslessHier29Syntax,       
    kcJpegLosslessHier14Syntax, 
    kcInitiatorName,
    kcReceiverName,
    kcDictionaryFile,
    kcMsgInfoFile,
    kcTempFileDirectory,
    kcLargeDataStore,
    kcDictionaryAccess,
    kcUnknownVrCode,
    kcPrivateSyntax1Syntax,
    kcPrivateSyntax2Syntax
};

enum ConfigIntParm {
    kcLogFileSize = kConfigIntParmOffset,
    kcLogMemorySize,
    kcArtimTimeout,
    kcAssocReplyTimeout,
    kcReleaseTimeout,
    kcWriteTimeout,
    kcConnectTimeout,
    kcInactivityTimeout,
    kcLargeDataSize,
    kcObowBufferSize,
    kcWorkBufferSize,
    kcTcpipListenPort,
    kcMaxPendingConnections,
    kcTcpipSendBufferSize,
    kcTcpipReceiveBufferSize,
    kcNumHistoricalLogFiles,
    kcLogFileLineLength
};

enum ConfigBoolParm {
    kcLogFileBackup = kConfigBoolParmOffset,
    kcAcceptAnyContextName,
    kcAcceptAnyApplicationTitle,
    kcAcceptAnyProtocolVersion,
    kcAcceptDifferentIcUid,
    kcAcceptDifferentVersion,
    kcAutoEchoSupport,
    kcSendSopClassUid,
    kcSendSopInstanceUid,
    kcSendLengthToEnd,
    kcSendRecognitionCode,
    kcSendMsgIdResponse,
    kcSendEchoPriority,
    kcSendResponsePriority,
    kcInsureEvenUidLength,  
    kcBlankFillLogFile,
    kcEliminateItemReferences,
    kcHardCloseTcpIpConnection,
    kcForceDicmInPrefix, 
    kcAcceptAnyPresentationContext,
    kcAcceptAnyHostname,
    kcAllowOutOfOrderTags,
    kcEmptyPrivateCreatorCodes,
    kcForceOpenEmptyItem,
    kcAcceptMultiplePresContexts,
    kcAllowInvalidPrivateCreatorCodes,
    kcRemovePaddingChars,
    kcPrivateSyntax1LittleEndian,
    kcPrivateSyntax1ExplicitVr,
    kcPrivateSyntax1Encapsulated,
    kcPrivateSyntax2LittleEndian,
    kcPrivateSyntax2ExplicitVr,
    kcPrivateSyntax2Encapsulated,
    kcExportUnVrToMedia,
    kcExportUnVrToNetwork,
    kcExportPrivateAttributesToMedia,
    kcExportPrivateAttributesToNetwork,
    kcAllowInvalidPrivateAttributes
};

enum ConfigLongParm {
    kcPduMaximumLength = kConfigLongParmOffset,   
    kcCallbackMinDataSize
};

typedef enum {
	MTI_FALSE = 0,
	MTI_TRUE = 1
} MTI_BOOLEAN;

typedef struct {
	char*                 Version;
	char                  A1[2];
	char*                 A2[39];
	MTI_BOOLEAN           A3[39];
	int                   A4[30];
	MTI_BOOLEAN           A5[41];
	unsigned long         A6[2];
	int                   I01;
	const char* const *   P01;
	const char* const *   P02;
	const unsigned long*  P03;
	const int*            P04;
	const int*            P05;
	const int*            P06;
	const int* const *    P07;
	int                   I02;
	const char* const *   P08;
	const int*            P09;
	const char* const *   P10;
} MergeCfg;

#endif