/***********************************************************************
 * FXDicomStatus.h
 *---------------------------------------------------------------------
 *   
 *-------------------------------------------------------------------
 */
#ifndef PX_FX_DICOM_STATUS_H
#define PX_FX_DICOM_STATUS_H

 

typedef enum
{
    kNormalCompletion        = 1,
    kEmptyServerList,
    kUnknownServer,
	kCannotOpenVoxFile,
	kDirectoryDoesNotExist,
	kCannotGetCurrentWorkingDirectory,
	kMissingRequiredQueryAttribute,
	kInvalidQueryLevel,
	kNotAllImagesFromSameSeries,
	kInvalidDataSource,			// 10
	kInvalidArguments,
	kRetrieveError,
	kQueryCacheDisabled,
	kCouldNotAllocateMemory,
	kInvalidRetrieveLevel,
	kRetrieveAborted,
	kFindAborted,
	kSortFailed,
	kSocketSelectError,
	kCFindFailed,				// 20
	kRetrievePartialFailure,
	kTooManyRetries,
	kInvalidImagePixels,

    kAlreadyRegistered       = 4000,  /* NEVER CHANGE */
    kAssociationAborted,
    kAssociationClosed,
    kAssociationRejected,
    kAttributeHasValues,
    kBufferTooSmall,
    kCallbackCannotComply,
    kCallbackDataSizeNegative,
    kCallbackDataSizeUneven,
    kCallbackParmError,
    kCallbackRegistered,  // 10
    kCannotComply,
    kCantAccessProfile,
    kConfigInfoError,
    kConfigInfoMissing,
    kDdfileError,
    kDoesNotValidate,
    kEmptyValue,
    kEndOfData,
    kExtInfoUnavailable,
    kFound,					// 20
    kFunctionUnavailable,
    kIncompatibleVr,        
    kIncompatibleValue,
    kInvalidApplicationId,
    kInvalidApplicationTitle,
    kInvalidAssocId,
    kInvalidCharsInValue,        
    kInvalidCommand,
    kInvalidDataType,        
    kEndOfList,                // 30
    kInvalidGroup,
    kInvalidHostName,
    kInvalidItemId,
    kInvalidLengthForTitle,
    kInvalidLengthForVr,
    kInvalidLicense,
    kInvalidMessageId,
    kInvalidMessageReceived,
    kInvalidParameterName,
    kInvalidPortNumber,			// 40
    kInvalidPrivateCode,
    kInvalidServiceListName,
    kInvalidTag,
    kInvalidTransferSyntax,
    kInvalidValueForVr,
    kInvalidValueNumber,
    kInvalidVrCode,
    kLogEmpty,
    kMessageEmpty,
    kMessageValidates,           // 50
    kMissingConfigParm,         
    kMsgfileError,
    kMustBePositive,
    kNetworkShutDown,
    kNoApplicationsRegistered,
    kNoCallback,
    kNoConditionFunction,
    kNoFileSystem,
    kNoInfoRegistered,
    kNoLicense,                   // 60
    kNoMergeIni,                
    kNoMoreAttributes,
    kNoMoreValues,
    kNoProfile,
    kNoRequestPending,
    kNonServiceAttribute,
    kNotFound,
    kNotOneOfEnumeratedValues,
    kNotOneOfDefinedTerms,
    kNullPointerParm,            // 70
    kNullValue,                
    kProtocolError,
    kRequiredAttributeMissing,
    kRequiredDatasetMissing,
    kRequiredValueMissing,
    kStateViolation,
    kSystemCallInterrupted,
    kSystemError,
    kTagAlreadyExists,
    kTempFileError,              // 80
    kTimeout,
    kTooFewValues,                
    kTooManyBlocks,
    kTooManyValues,
    kUnableToCheckCondition,
    kUnacceptableService,
    kUnexpectedEod,
    kUnknownItem,
    kUnknownService,
    kValueMayNotBeNull,         // 90
    kValueNotAllowed,
    kValueOutOfRange,
    kValueTooLarge,
    kVrAlreadyValid,
    kLibraryAlreadyInitialized, 
    kLibraryNotInitialized,
    kInvalidDirectoryRecordOffset,
    kInvalidFileId, 
    kInvalidDicomdirId, 
    kInvalidEntityId,           // 100
    kInvalidMrdrId,
    kUnableToGetItemId,
    kInvalidPad,
    kEntityAlreadyExists,
    kInvalidLowerDirRecord,
    kBadDirRecordType,
    kUnknownHostConnected,
    kInactivityTimeout,
    kInvalidSopClassUid,
    kInvalidVersion,           // 110
    kOutOfOrderTag,
    kConnectionFailed,
    kUnknownHostName,
    kInvalidFile,
    kNegotiationAborted,
    kInvalidSrId,
    kUnableToGetSrId,
	kDuplicateName,
	kDuplicateSyntax,
	kEmptyList,					// 120
	kMissingName,
    kInvalidServiceName,
	kServiceInUse,
	kInvalidSyntaxName,
	kSyntaxInUse,
	kNoContext,
	kOffsetTableTooShort,
	kMissingDelimiter,
	kCompressionFailure,
	kEndOfFrame,				// 130
	kMustContinueBeforeReading,

	/* Added 05/14/2003 T.C. Zhao
	 * new error code  from Merge 
	 */
	kCompressorRequired,	//MC_COMPRESSOR_REQUIRED,
	kDecompressorRequired,	//MC_DECOMPRESSOR_REQUIRED,
	kDataAvilable,			//MC_DATA_AVAILABLE,
	kZlipError,				//MC_ZLIB_ERROR,
	kNoMetaSOP				//MC_NOT_META_SOP
} PxDicomStatus;



#endif	//PX_FX_DICOM_STATUS_H



