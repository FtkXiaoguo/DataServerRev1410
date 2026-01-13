//------------------------------------------------------------------
//------------------------------------------------------------------

#pragma warning (disable: 4530)

#include "AqCore/TRPlatform.h"
#include "PxDICOMUtil.h"
//#include "rtvMergeToolKit.h"
#include "IDcmLibApi.h"

#include "AqCore/AqCore.h"

#include "AuxData.h"

#include "CheckMemoryLeak.h"

using namespace XTDcmLib;

//-----------------------------------------------------------------------------

#include <assert.h>
#include "PxDicomMessage.h"
#include "ProgressAPI.h"

const char* kImplicitLittleEndianTransferSyntaxUID = "1.2.840.10008.1.2.1";
const char* kMediaStorageSOPClassUID = "1.2.840.10008.1.3.10";
const char* kDicomDirFileName = "DICOMDIR";
const char* kDicomDirServiceName = "DICOMDIR";



PxDicomStatus initializeDirInfo(TRDicomDirInfo* in_DDInfo, std::string iRoorDirPath);

PxDicomStatus AddDICOMDIRGroupTwoElements(TRDicomDirInfo* in_DDInfo );

PxDicomStatus AddMessageToDICOMDIR(const char* oldFileName, const char* newFileName, TRDicomDirInfo* in_DDInfo);

PxDicomStatus FillPatientRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID);

PxDicomStatus FillStudyRecord (CPxDicomMessage* in_VLIDCMMsg, int A_recID);

PxDicomStatus FillSeriesRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID);

PxDicomStatus FillImageRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID, const char* oldFileName, 
									const char* newFileName);

PxDicomStatus SetValue(CPxDicomMessage* in_VLIDCMMsg, int A_recID, unsigned long A_tag, DICOM_TYPE A_type, 
									unsigned long CopyFromTag = 0);

PxDicomStatus SetFileNameAttribute( CPxDicomMessage* in_VLIDCMMsg, int A_recID, unsigned long A_tag,
										const char* A_fileName);

PxDicomStatus CreateDCMFileName(std::string iRootDirPath, std::string iDICOMDirPath, 
									std::map<std::string, std::string>& mangledFileNameMap);

bool   CreatePatientDir(std::string iDICOMDirOutPutPath, int *outDirNameIntVal);

int    GetRandNumber();

//change the logger
// #4 2012/02/21 K.Ko
class MyDcmLibApiLoggerLogger : public DcmLibApiLogger
{
public:
	MyDcmLibApiLoggerLogger(){
		m_pDcmApiLogger=0;
	}
	void setupLogger(TRLogger *logger) { m_pDcmApiLogger = logger;};
	//
	virtual void LoggerError(const char *format,...);
	virtual void LoggerWarn(const char *format,...);
	virtual void LoggerDebug(const char *format,...);
	virtual void LoggerTrace(const char *format,...);
protected:
	TRLogger *m_pDcmApiLogger;

	void LoggerOutput(char *type, const char* ifmt, va_list arguments );
	bool isLoggerLevel(IDcmLib::LOG_LEVEL log_level){
		if(m_pDcmApiLogger==0) return false;
		bool ret_level_f = false;
		int file_log_level = m_pDcmApiLogger->GetLogLevel();
		switch(file_log_level){
		case kErrorOnly:
			if(log_level <=IDcmLib::LOGLEVEL_ERROR){
				ret_level_f = true;
			}
			break;
		case kWarning:
			if(log_level <=IDcmLib::LOGLEVEL_WARN){
				ret_level_f = true;
			}
			break;
		case kInfo:
			if(log_level <=IDcmLib::LOGLEVEL_INFO){
				ret_level_f = true;
			}
			break;
		case kDebug:
			if(log_level <=IDcmLib::LOGLEVEL_DEBUG){
				ret_level_f = true;
			}
			break;
		case kTrace:
			if(log_level <=IDcmLib::LOGLEVEL_TRACE){
				ret_level_f = true;
			}
		}
		return ret_level_f;

	}
};

void MyDcmLibApiLoggerLogger::LoggerError(const char *format,...){
		if(!isLoggerLevel(IDcmLib::LOGLEVEL_ERROR)) return ;

	 	va_list args;
		va_start(args, format);
		LoggerOutput("[Error]",format, args);
		va_end(args);
	}
void MyDcmLibApiLoggerLogger::LoggerWarn(const char *format,...){
		if(!isLoggerLevel(IDcmLib::LOGLEVEL_WARN)) return ;

		va_list args;
		va_start(args, format);
		LoggerOutput("[Warn]",format, args);
		va_end(args);
	}
void MyDcmLibApiLoggerLogger::LoggerDebug(const char *format,...){
		if(!isLoggerLevel(IDcmLib::LOGLEVEL_TRACE)) return ;

		va_list args;
		va_start(args, format);
		LoggerOutput("[Debug]",format, args);
		va_end(args);
	}
void MyDcmLibApiLoggerLogger::LoggerTrace(const char *format,...){
		if(!isLoggerLevel(IDcmLib::LOGLEVEL_DEBUG)) return ;

		va_list args;
		va_start(args, format);
		LoggerOutput("[Trace]",format, args);
		va_end(args);
	}
void MyDcmLibApiLoggerLogger::LoggerOutput(char *type, const char* ifmt, va_list arguments )
	{
	//	m_pDcmApiLogger->LogMessage("[%s]: ",type);
		m_pDcmApiLogger->WriteLogMessage(ifmt, arguments,type);
//		m_pDcmApiLogger->LogMessage("\n");
		m_pDcmApiLogger->FlushLog();
		 
	};
 
static MyDcmLibApiLoggerLogger _myDcmAPI_Logger_;
//------------------------------------------------------------------------------------------------------
#if 0 
//change the logger
// #4 2012/02/21 K.Ko
bool TRDICOMUtil::InitialDICOM(const char* iLicense, int iLogLevel/*=0*/, const char* iLogFile/*=0*/,
#else
bool TRDICOMUtil::InitialDICOM(const char* iLicense, int iLogLevel/*=0*/, TRLogger *LoggerFile/*=0*/,
#endif
int iLogMaxSize, const char* iCaptureFile/*=0*/, int iNumCaptureFiles/*=2*/)
{

	int status;
	//	Initialize the toolkit
	status = MC_Library_Initialization(MC_Config_Values, MC_Dictionary_Values, NULL);
	
	//  moved up
	if ( status == MC_LIBRARY_ALREADY_INITIALIZED )
		return true;

	if (status != MC_NORMAL_COMPLETION )
	{
		GetAqLogger()->LogMessage("Error: MergeCOM Init failed: status = %d\n", status);
		GetAqLogger()->FlushLog();
		return false;
	}
 
	//
//	IDcmLibApi::openLogger("dcmlibAPI.log",IDcmLib::LOGLEVEL_TRACE );
//	IDcmLibApi::openLogger("dcmlibAPI.log",IDcmLib::LOGLEVEL_INFO );
// 	IDcmLibApi::openLogger("dcmlibAPI.log",IDcmLib::LOGLEVEL_WARN);//LOGLEVEL_INFO );
//

//	if ( status == MC_LIBRARY_ALREADY_INITIALIZED )
//		return true;

#if 0
	//	Set the Merge License - 
	if(!iLicense)
		iLicense = "";

	status = MC_Set_String_Config_Value(LICENSE, (char*)iLicense);
	if (status != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage("Error(%d) on setting Merge license:%d\n", iLicense, status);
		GetAqLogger()->FlushLog();
		return false;
	}
#endif

#if 1
//change the logger
// #4 2012/02/21 K.Ko
/*
# DICOM information level (merge log level)
# 0 -> Errors
# 1 -> Errors + Warnings
# 2 -> Errors + Warnings + Info
# 3 -> Errors + Warnings + Info + Trans
# 4 -> Errors + Warnings + Info + Trans + Debug
# (Default 0)
*/
	_myDcmAPI_Logger_.setupLogger(LoggerFile);
	
	IDcmLib::LOG_LEVEL dcmLig_log_level = IDcmLib::LOGLEVEL_OFF_LOG;
	switch(iLogLevel){
	case 0:
		dcmLig_log_level = IDcmLib::LOGLEVEL_ERROR;
		break;
	case 1:
		dcmLig_log_level = IDcmLib::LOGLEVEL_WARN;
		break;
	case 2:
		dcmLig_log_level = IDcmLib::LOGLEVEL_INFO;
		break;
	case 3:
		dcmLig_log_level = IDcmLib::LOGLEVEL_DEBUG;
		break;
	case 4:
		dcmLig_log_level = IDcmLib::LOGLEVEL_TRACE;
		break;
	}
	IDcmLibApi::setupLoger(&_myDcmAPI_Logger_,dcmLig_log_level);

#else
	//	Set the merge logging bits according to the verbose config
	SetLogBits(iLogLevel);

	if(iLogFile && iLogFile[0])
	{
		//	Set the Merge Log File 
		status = MC_Set_String_Config_Value(LOG_FILE, (char*)iLogFile);
		if (status != MC_NORMAL_COMPLETION)
			GetAqLogger()->LogMessage(kWarning,"Warning: unable to set Merge Log File %s: status = %d\n", iLogFile, status);

		//	Set the Merge Log File Size
		if(iLogMaxSize > 0)
		{
			status = MC_Set_Int_Config_Value(LOG_FILE_SIZE, (iLogMaxSize * 1024) / 80);
			if (status != MC_NORMAL_COMPLETION)
				GetAqLogger()->LogMessage(kWarning,"Warning: unable to set Merge Log Size (%d): status = %d\n", 
				iLogMaxSize, status);
		}
	}
#endif

	//	Enable Merge Network Capture file
	if (iCaptureFile && iCaptureFile[0]){
		TRDICOMUtil::EnableNetworkCapture((char*)iCaptureFile, false, iNumCaptureFiles);
	}

	{
		//add Private Tag to Global Dictionary

		DcmXTUtil		*dcmUtil= IDcmLibApi::getDcmXTUtil();

		
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_ORIGINAL_SERIES_AND_STUDY_INSTANCE_UID,		LO,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_REFERENCED_VOLUME_ID,						LO,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_BINARY_DATA_NAME,							LO,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_NUMBER_OF_SERIES,							CS,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_NUMBER_OF_BINARY_DATA,						US,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_BINARY_DATA_TYPE,							CS,	TR_CREATOR);
//		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_BINARY_DATA_SIZE,							CS,	TR_CREATOR);
		//#58 2013/06/03 K.Ko  CS->ULに変更
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_BINARY_DATA_SIZE,							UL,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_ADDITIONAL_INFORMATION,						LO,	TR_CREATOR);
		//
		for(int i=0;i<16;i++){
			dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_FIRST_BINARY_DATA+1,						OB,	TR_CREATOR);
		}
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_FIRST_THUMBNAIL,								OB,	TR_CREATOR);
		
		//
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_VOLUME_ID,									UI,	TR_CREATOR);
		dcmUtil->Add_PrivateTag(TR_GROUP,	TR_ATT_VOLUME_ID,									UI,	TR_CREATOR);
	}


	return true;
}


void TRDICOMUtil::ReleaseDICOM()
{
	MC_Library_Release();
}


MC_STATUS AqFileMetaInfoVersion( int           A_msgID,
							unsigned long A_tag,
							int           A_isFirst,
							 void*         A_info,
							 int*          A_dataSize,
							void**        A_dataBufferPtr,
							 int*          A_isLastPtr)


{
    static char version[] = {0x00,0x01};

	assert(A_tag == MC_ATT_FILE_META_INFORMATION_VERSION);
	assert(A_isFirst != 0);

	if (A_isFirst)
	{
		*A_dataSize = 2;
		*A_dataBufferPtr = version;
		*A_isLastPtr = 1;
	}

    return MC_NORMAL_COMPLETION;

} /* FileMetaInfoVersion() */

static int AddGroup2Elements(
	int               A_fileID,
	TRANSFER_SYNTAX   A_transSyntax = IMPLICIT_LITTLE_ENDIAN, 
	const char*		  localAE = 0);
//-----------------------------------------------------------------------------
// AddGroup2Elements - Add all gr 2 elements including implementation UID & Version
int TRDICOMUtil::AddGroup2Elements(int A_fileID, int A_transSyntax, const char* localAE)
{
    int mcStatus;
    char      uidBuffer[kVR_UI];
    char      syntaxUID[kVR_UI];

    /*
     * Get the correct UID for the new transfer syntax
     */
    mcStatus = MC_Get_Transfer_Syntax_From_Enum((TRANSFER_SYNTAX)A_transSyntax,
                                                syntaxUID,
                                                sizeof(syntaxUID));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        fprintf(stderr,"Unable to get transfer syntax UID", mcStatus);
        return mcStatus;
    }

    /*
     * Set the new transfer syntax for this message
     */
    mcStatus = MC_Set_Value_From_String(A_fileID,
                                        MC_ATT_TRANSFER_SYNTAX_UID,
                                        syntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        fprintf(stderr,"Unable to set transfer syntax UID in group 2 elements", mcStatus);
        return mcStatus;
    }

    /*
     * Set other media group 2 elements
     */
    mcStatus = MC_Set_Value_From_Function( A_fileID,
                                           MC_ATT_FILE_META_INFORMATION_VERSION,
                                           NULL,
                                           AqFileMetaInfoVersion );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        fprintf(stderr,"Unable to add file meta information version", mcStatus);
        return mcStatus;
    }


    mcStatus = MC_Get_Value_To_String( A_fileID,
                                       MC_ATT_SOP_CLASS_UID,
                                       sizeof(uidBuffer),
                                       uidBuffer );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
		//	Couldn't get SOP Class UID - default to Standard Secondary Capture
		strncpy(uidBuffer, "1.2.840.10008.5.1.4.1.1.7", sizeof uidBuffer);
        uidBuffer[sizeof(uidBuffer)-1] =0;
    }


    mcStatus = MC_Set_Value_From_String( A_fileID,
                                         MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID,
                                         uidBuffer);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        fprintf(stderr,"Unable to add media storage SOP Class UID", mcStatus);
        return mcStatus;
    }

    mcStatus = MC_Get_Value_To_String(A_fileID,
                                      MC_ATT_SOP_INSTANCE_UID,
                                      sizeof(uidBuffer),
                                      uidBuffer);
    if (mcStatus == MC_NORMAL_COMPLETION)
    {
		MC_Set_Value_From_String( A_fileID,
								  MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID,
								  uidBuffer);
    }
	
	//Add implementation version
	mcStatus = MC_Get_String_Config_Value(IMPLEMENTATION_CLASS_UID, sizeof(uidBuffer), uidBuffer);
	if (mcStatus == MC_NORMAL_COMPLETION)
    {
		MC_Set_Value_From_String( A_fileID,
								  MC_ATT_IMPLEMENTATION_CLASS_UID,
								  uidBuffer);
	}
	
	//Add Implementation version
	mcStatus = MC_Get_String_Config_Value(IMPLEMENTATION_VERSION, sizeof(uidBuffer), uidBuffer);
	if (mcStatus == MC_NORMAL_COMPLETION)
	{
		MC_Set_Value_From_String( A_fileID,
								  MC_ATT_IMPLEMENTATION_VERSION_NAME,
								  uidBuffer);
	}

 	//	Add command group for part 10 compliance
	if (!localAE)
	{
		localAE = "";
	}

	mcStatus = MC_Set_Value_From_String( A_fileID,
                                         MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,
                                         localAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        fprintf(stderr,"Unable to add source application entity title",mcStatus);
        return mcStatus;
    }

    return MC_NORMAL_COMPLETION;

} /* AddGroup2Elements() */

//-----------------------------------------------------------------------------
//
#include <assert.h>
MC_STATUS AqFileObjToMedia( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    size_t     count;
    CBinfo*    cbInfo = (CBinfo*)A_userInfo;

 	//  
	// Added try/catch  for extra protection against bad data
	try  
	{
	   if (A_isFirst)
			cbInfo->fp = fopen(A_filename, "wb");
			
		
		//	Added check for datasize.  If we are being called with something that is
		//		obviously too big, just fail.  NOTE:  the caller should set datasize
		//		before invoking this callback, otherwise this check will not happen.
		if (!cbInfo->fp || (cbInfo->dataSize > 0 && A_dataSize > cbInfo->dataSize))
			return MC_CANNOT_COMPLY;

		if(A_dataSize > 0)
		{
			extern int errno;
			errno = 0;
			count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfo->fp);
			if (count != (size_t)A_dataSize)
			{
				printf("fwrite error: %d",errno);
				return MC_CANNOT_COMPLY;
			}
		}

		if (A_isLast)
		{
			/*
			 * NULL ->fp so that the routine calling MC_Write file knows
			 * not to close the stream.
			 */
			fclose(cbInfo->fp);
			cbInfo->fp = NULL;
		}

		return MC_NORMAL_COMPLETION;
	}
	catch (...)
	{
		assert(0);
		return MC_CANNOT_COMPLY;
	}

} /* FileObjToMedia() */


MC_STATUS FileObjToMemory( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    MemoryCBinfo*    cbInfo = (MemoryCBinfo*)A_userInfo;

	try  
	{
	   if (A_isFirst)
			cbInfo->bytesRead = 0;

		if ((cbInfo->dataSize > 0 && A_dataSize > cbInfo->dataSize))
			return MC_CANNOT_COMPLY;

		if(A_dataSize > 0)
		{
			unsigned long leftBufferSize = cbInfo->dataSize - cbInfo->bytesRead;
			if ( leftBufferSize < A_dataSize )
				return MC_CANNOT_COMPLY; // do not have enough buffer

			memcpy(cbInfo->memoryPointerForWrite+cbInfo->bytesRead,
				A_dataBuffer,A_dataSize);

			cbInfo->bytesRead += A_dataSize;

		}

		return MC_NORMAL_COMPLETION;
	}
	catch (...)
	{
		assert(0);
		return MC_CANNOT_COMPLY;
	}

} /* FileObjToMemory() */


//--------------------------------------------------------------------
//  Function    :   MediaToFileObj
//  Parameters  :   A_fileName   - Filename to open for reading
//                  A_userInfo   - Pointer to an object used to preserve
//                                 data between calls to this function.
//                  A_dataSize   - Number of bytes read
//                  A_dataBuffer - Pointer to buffer of data read
//                  A_isFirst    - Set to non-zero value on first call
//                  A_isLast     - Set to 1 when file has been completely 
//                                 read
//
//  Returns     :   MC_NORMAL_COMPLETION on success
//                  any other PxDicomStatus value on failure.
//
//  Description :   Callback function used by MC_Open_File to read a file
//                  in the DICOM Part 10 (media) format.
//
//--------------------------------------------------------------------
MC_STATUS AqMediaToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast)
{

    MediaCBinfo*    callbackInfo = (MediaCBinfo*)A_userInfo;
    size_t          bytes_read;

    if (!A_userInfo)
	{
		callbackInfo->errCode |= kCBERRBadArg;
        return  MC_CANNOT_COMPLY;
	}

    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
        callbackInfo->fp = fopen(A_filename, BINARY_READ);
    }
    
    if (!callbackInfo->fp)
	{
		callbackInfo->errCode |= kCBERROpenFile;
		return MC_CANNOT_COMPLY;
	}

    bytes_read = fread(callbackInfo->buffer, 1, sizeof(callbackInfo->buffer),
                       callbackInfo->fp);
    if (ferror(callbackInfo->fp))
	{
		callbackInfo->errCode |= kCBERRReadFile;
        return MC_CANNOT_COMPLY;
	}

    if (feof(callbackInfo->fp))
    {
        *A_isLast = 1;
        fclose(callbackInfo->fp);
        callbackInfo->fp = NULL;
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = callbackInfo->buffer;
    *A_dataSize = (int)bytes_read;
    callbackInfo->bytesRead += bytes_read;
    return  MC_NORMAL_COMPLETION;
    
} /* MediaToFileObj() */


MC_STATUS  AqMemoryToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast)
{

    MemoryCBinfo*    callbackInfo = (MemoryCBinfo*)A_userInfo;

    if (!A_userInfo)
        return  MC_CANNOT_COMPLY;

    if (A_isFirst)
    {
        callbackInfo->bytesRead = 0;
    }
    
	unsigned long leftBytes = callbackInfo->dataSize - callbackInfo->bytesRead;
	int bytesToCopy = (int)(leftBytes > sizeof(callbackInfo->buffer) ? sizeof(callbackInfo->buffer) : leftBytes);
	if ( bytesToCopy > 0 )
	{
		memcpy(callbackInfo->buffer,
			callbackInfo->memoryPointerForRead+callbackInfo->bytesRead,
			bytesToCopy);
	}
	callbackInfo->bytesRead += bytesToCopy;
	*A_dataBuffer = callbackInfo->buffer;
	*A_dataSize = bytesToCopy;

	if ( leftBytes <= sizeof(callbackInfo->buffer) )
	{
        *A_isLast = 1;
	}

    return  MC_NORMAL_COMPLETION;
    
} /* MemoryToFileObj() */


//-----------------------------------------------------------------------------
//
int TRDICOMUtil::SendResponseMessage(int iRespStatus, int iAssociationID)
{
	//
	//	Open response message
	//
	int rspMsgID = -1;
	char* serviceName = 0;

	int status =  MC_Open_Empty_Message(&rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status =  MC_Set_Service_Command(rspMsgID, serviceName, C_STORE_RSP);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	//
	//	Send response message
	//
	status = MC_Send_Response_Message(iAssociationID, iRespStatus, rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	//
	//	Free response message
	//
	MC_Free_Message(&rspMsgID);

	return MC_NORMAL_COMPLETION;
}


//-----------------------------------------------------------------------------------------
//

//	iLogLevel has the following meaning:
//		0 -> Errors
//		1 -> Errors + Warnings
//		2 -> Errors + Warnings + Info
//		3 -> Errors + Warnings + Info + Transactions
//		4 -> Errors + Warnings + Info + Transactions + Debug
void TRDICOMUtil::SetLogBits(int iLogLevel, int iRotateLogs, int iNumLogs)
{
	//	Turn off all logging
	MC_Set_Log_Destination(ERROR_DESTINATIONS,   Bitbucket_Destination);
	MC_Set_Log_Destination(WARNING_DESTINATIONS, Bitbucket_Destination);
	MC_Set_Log_Destination(INFO_DESTINATIONS,    Bitbucket_Destination);
	MC_Set_Log_Destination(T1_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T2_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T3_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T4_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T5_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T6_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T7_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T8_DESTINATIONS,      Bitbucket_Destination);
	MC_Set_Log_Destination(T9_DESTINATIONS,      Bitbucket_Destination);

	//
	//	Turn on logging for the level requested
	//

	MC_Set_Bool_Config_Value(LOG_FILE_BACKUP, iRotateLogs);
	MC_Set_Int_Config_Value(NUM_HISTORICAL_LOG_FILES, iNumLogs);

	//	Errors
	if (iLogLevel >= 0)
	{
		MC_Set_Log_Destination(ERROR_DESTINATIONS,   File_Destination);
	}

	//	Warnings
	if (iLogLevel >= 1)
	{
		MC_Set_Log_Destination(WARNING_DESTINATIONS, File_Destination);
	}

	//	Info
	if (iLogLevel >= 2)
	{
		MC_Set_Log_Destination(INFO_DESTINATIONS,    File_Destination);
	}

	//	Debug
	if (iLogLevel >= 3)
	{
		MC_Set_Log_Destination(T3_DESTINATIONS,      File_Destination);
		MC_Set_Log_Destination(T4_DESTINATIONS,      File_Destination);
		MC_Set_Log_Destination(T7_DESTINATIONS,      File_Destination);
	}

	//	Trace
	if (iLogLevel >= 4)
	{
		MC_Set_Log_Destination(T2_DESTINATIONS,      File_Destination);
		MC_Set_Log_Destination(T6_DESTINATIONS,      File_Destination);
		MC_Set_Log_Destination(T8_DESTINATIONS,      File_Destination);
	}
}

//-----------------------------------------------------------------------------------------
//
bool TRDICOMUtil::AssociationIsDead(int iStatus)
{
	return (iStatus == MC_ASSOCIATION_CLOSED ||
			iStatus == MC_ASSOCIATION_ABORTED ||
			iStatus == MC_NETWORK_SHUT_DOWN ||
			iStatus == MC_INACTIVITY_TIMEOUT ||
			iStatus == MC_CONFIG_INFO_ERROR ||
			iStatus == MC_INVALID_MESSAGE_RECEIVED);
}

struct StreamCBData
{
	FILE* m_fp;
//	~StreamCBData() { if (m_fp) fclose(m_fp); }
};

//-----------------------------------------------------------------------------------------
//
MC_STATUS MsgToStreamCB(int messageID, void* userInfo, int dataSize, void* buf, int isFirst, int isLast)
{
	StreamCBData* cbInfo = (StreamCBData*) userInfo;
	size_t n = fwrite(buf, (size_t) dataSize, (size_t) 1, cbInfo->m_fp);
//	if (n != 1)
//		return MC_CALLBACK_CANNOT_COMPLY;

	return MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------------------
//
void TRDICOMUtil::DumpMessage(int iMsgID, const char* iService, int iCommand, const char* iRemoteAE)
{
	int status;
	TRANSFER_SYNTAX syntax;
	std::string cmdString;

	//	Convert command to a string for the filename
	switch(iCommand)
	{
		case C_STORE_RQ:		cmdString = "C_STORE_RQ";			break;
		case C_STORE_RSP:		cmdString = "C_STORE_RSP";			break;
		case C_GET_RQ:			cmdString = "C_GET_RQ";				break;
		case C_GET_RSP:			cmdString = "C_GET_RSP";			break;
		case C_CANCEL_RQ:		cmdString = "C_CANCEL_RQ";			break;
		case C_FIND_RQ:			cmdString = "C_FIND_RQ";			break;
		case C_FIND_RSP:		cmdString = "C_FIND_RSP";			break;
		case C_MOVE_RQ:			cmdString = "C_MOVE_RQ";			break;
		case C_MOVE_RSP:		cmdString = "C_MOVE_RSP";			break;
		case C_ECHO_RQ:			cmdString = "C_ECHO_RQ";			break;
		case C_ECHO_RSP:		cmdString = "C_ECHO_RSP";			break;
		case N_EVENT_REPORT_RQ:	cmdString = "N_EVENT_REPORT_RQ";	break;
		case N_EVENT_REPORT_RSP:cmdString = "N_EVENT_REPORT_RSP";	break;
		case N_GET_RQ:			cmdString = "N_GET_RQ";				break;
		case N_GET_RSP:			cmdString = "N_GET_RSP";			break;
		case N_SET_RQ:			cmdString = "N_SET_RQ";				break;
		case N_SET_RSP:			cmdString = "N_SET_RSP";			break;
		case N_ACTION_RQ:		cmdString = "N_ACTION_RQ";			break;
		case N_ACTION_RSP:		cmdString = "N_ACTION_RSP";			break;
		case N_CREATE_RQ:		cmdString = "N_CREATE_RQ";			break;
		case N_CREATE_RSP:		cmdString = "N_CREATE_RSP";			break;
		case N_DELETE_RQ:		cmdString = "N_DELETE_RQ";			break;
		case N_DELETE_RSP:		cmdString = "N_DELETE_RSP";			break;
		default:				cmdString = "CMD_UNKNOWN";			break;
	};

	//	What is the transfer syntax of the incoming message?  Preserve it if possible
	status = MC_Get_Message_Transfer_Syntax(iMsgID, &syntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		syntax = EXPLICIT_LITTLE_ENDIAN;
	}

	char path[MAX_PATH];

	strcpy(path, "C:/AQNetLog/MessageDump/");
	if (iRemoteAE)
	{
		strcat(path, iRemoteAE);
		strcat(path, "/");
	}

	//	Generate the filename
	TRPlatform::MakeDirIfNeedTo(path);

	char fname[MAX_PATH];
	int idx = 0;
	_snprintf(fname, sizeof fname, "%s%s.%s.%d.%d.dcm", path, iService, cmdString.c_str(), iMsgID, idx); 

	struct _stat statBuf;
	while(!::_stat(fname, &statBuf))
	{
		++idx;
		_snprintf(fname, sizeof fname, "%s%s.%s.%d.%d.dcm", path, iService, cmdString.c_str(), iMsgID, idx); 
	}

	//	If it's a C_STORE_RQ, write it out as part 10
	if (iCommand == C_STORE_RQ)
	{
		int newMsgID = -1;
		MediaCBinfo cbinfo;
		status = MC_Duplicate_Message(iMsgID, &newMsgID, syntax, 0, 0);//MC_Standard_Compressor, MC_Standard_Decompressor);
		status = MC_Message_To_File(newMsgID, fname);
		if (status != MC_NORMAL_COMPLETION)
		{
			return;  
		}
		int fileID = newMsgID;

		status = TRDICOMUtil::AddGroup2Elements(fileID, syntax, "");
		if (status != MC_NORMAL_COMPLETION)
		{
			return;
		}
		
		
		//	So the callback can check if it's passed too much data
		status = MC_Get_File_Length(fileID, &cbinfo.dataSize);
		if (status != MC_NORMAL_COMPLETION)
			cbinfo.dataSize = 0;

		//	Write the file to disk
		status = MC_Write_File(fileID, 0, &cbinfo, AqFileObjToMedia);
		if (status != MC_NORMAL_COMPLETION)
		{
			return;
		}
	}
	//	Otherwise, stream it as is
	else
	{
#if 0
		FILE* outfp = fopen(fname, "w");
		if (!outfp)
		{
			return;
		}

		StreamCBData cbinfo;
		cbinfo.m_fp = outfp;

		status = MC_Message_To_Stream(iMsgID, 0x00000000, 0xFFffFFff, syntax, 
			(void*)&cbinfo, MsgToStreamCB);

		if (cbinfo.m_fp) fclose(cbinfo.m_fp); 
#else
 
		CPxDicomMessage dumpMsg(iMsgID);
		dumpMsg.HandoverID();
		dumpMsg.Save(fname);
#endif

	}
}

//-----------------------------------------------------------------------------
//
int TRDICOMUtil::EnableNetworkCapture(char* iCapFile, bool iRewrite, int iNumFiles, long iMaxSize)
{
	int status;

	iRewrite = iNumFiles ? true : iRewrite;

	//	Turn on Merge Network Capture
	status = MC_Set_String_Config_Value(CAPTURE_FILE, iCapFile);
	status = MC_Set_Bool_Config_Value(NETWORK_CAPTURE, 1);
	status = MC_Set_Bool_Config_Value(REWRITE_CAPTURE_FILES, iRewrite);
	status = MC_Set_Long_Config_Value(CAPTURE_FILE_SIZE, iMaxSize);
	status = MC_Set_Int_Config_Value(NUMBER_OF_CAP_FILES, iNumFiles);

	return status;
}	

//-----------------------------------------------------------------------------
//
int TRDICOMUtil::DisableNetworkCapture()
{
	return MC_Set_Bool_Config_Value(NETWORK_CAPTURE, 0);
}	

//-----------------------------------------------------------------------------
//
bool TRDICOMUtil::IsMainLevelObject(int iMessageID)
{
	int status;
	char SOPInstanceUID[kVR_UI];
	
	status = MC_Get_Value_To_String(iMessageID, kVLISopInstanceUid, kVR_UI, SOPInstanceUID); 
	return (status == MC_NORMAL_COMPLETION);
}


//-----------------------------------------------------------------------------
//
int TRDICOMUtil::ExtractTransferSyntax(int iMsgID, int& oTransferSyntax, bool& oIsCompressed)
{
	int status;

	char syntaxUID[65];
 	TRANSFER_SYNTAX syntax;

	oIsCompressed = false;
	oTransferSyntax = IMPLICIT_LITTLE_ENDIAN;

	status = (PxDicomStatus) MC_Get_Message_Transfer_Syntax(iMsgID, &syntax);
	if (status != kNormalCompletion)
	{
		status = (PxDicomStatus) MC_Get_Value_To_String(iMsgID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
		if (status != kNormalCompletion)
		{
			return status;  
		}

		status = (PxDicomStatus) MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != kNormalCompletion)
		{
			return status;  
		}
	}

	oTransferSyntax = (int) syntax;
	oIsCompressed = (syntax == IMPLICIT_LITTLE_ENDIAN || syntax == EXPLICIT_BIG_ENDIAN ||
		syntax == EXPLICIT_LITTLE_ENDIAN || syntax == IMPLICIT_BIG_ENDIAN);

	return kNormalCompletion;
}

//-----------------------------------------------------------------------------
//
int TRDICOMUtil::GetPixelDataFileOffset(int iMessageID, const char* iFilename, int iSyntax, unsigned long& oPixelDataSize, unsigned long& oOffset)
{
	int status;


	status = MC_Get_Value_Length(iMessageID, MC_ATT_PIXEL_DATA, 1, &oPixelDataSize);
	if (status != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage("ERROR: TRDICOMUtil::GetPixelDataFileOffset Merge status: %d, msgID: %d, filename: %s\n ", 
			status, iMessageID, iFilename);

		return status;
	}

#ifdef UseXTDcmlib
	status = MC_Get_PixelOffset(iMessageID,&oOffset);
	if (status != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage("ERROR: TRDICOMUtil::GetPixelDataFileOffset Merge status: %d, msgID: %d, iSyntax: %d, filename: %s\n ", 
			status, iMessageID, iSyntax, iFilename);

		return status;
	}
#else
	unsigned long post;
	status = MC_Get_Stream_Length(iMessageID, 0x7fe00011, 0xffffffff, &post, (TRANSFER_SYNTAX)iSyntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage("ERROR: TRDICOMUtil::GetPixelDataFileOffset Merge status: %d, msgID: %d, iSyntax: %d, filename: %s\n ", 
			status, iMessageID, iSyntax, iFilename);

		return status;
	}

	unsigned long filesize;
	status = MC_Get_File_Length(iMessageID, &filesize);
	if (status != MC_NORMAL_COMPLETION)
	{

		filesize = TRPlatform::GetFileSize(iFilename);
		if (filesize <= 0)
		{
			GetAqLogger()->LogMessage("ERROR: TRDICOMUtil::GetPixelDataFileOffset size error(%d) on filename: %s\n ", 
				GetLastError(), iFilename);
		}
	}

	oOffset = (unsigned long) filesize - (oPixelDataSize + post);
#endif
	return MC_NORMAL_COMPLETION;

}

//------------------------------------------------------------------------------------------
//
//
int TRDICOMUtil::GetPixelDataFileOffset(const char* iFileName, unsigned long& oPixelOffset, unsigned long& oPixelSize, bool& isCompressed)
{
	if(!iFileName || !*iFileName)
		return MC_INVALID_FILE;

	//Read the DICOM file
	CPxDicomMessage aMsg;
	int readHeaderOnly = 0, readAsFileObj = 1;
	PxDicomStatus vliStatus = aMsg.Load(iFileName, readHeaderOnly, readAsFileObj); //TODO: Add a check to fail non part 10 file
	if(vliStatus != kNormalCompletion)
	{
		return vliStatus; //Not a DICOM file
	}
	int msgID = aMsg.GetID();
	if(msgID <= 0)
		return false;
	
	return GetPixelDataFileOffset(msgID, oPixelOffset, oPixelSize, isCompressed);
}

//------------------------------------------------------------------------------------------
//
//
int TRDICOMUtil::GetPixelDataFileOffset(int iMessageID, unsigned long& oPixelOffset, unsigned long& oPixelSize, bool& isCompressed)
{
	int status;
	oPixelOffset = 0; 	oPixelSize = 0; isCompressed = false;
	
	//Check for multi-frame image
	unsigned short numberOfFrames = 0;
	status = MC_Get_Value_To_UShortInt(iMessageID, MC_ATT_NUMBER_OF_FRAMES,&numberOfFrames); 
    if (status == MC_NORMAL_COMPLETION && numberOfFrames > 1 && numberOfFrames < 32767)
    {
        return MC_CANNOT_COMPLY; //multi-frame is not supported
    }
	//Get original Transfer syntax
	int  originalTransferSyntax;
	bool notCompress;
	//Check if data is compressed
	status = ExtractTransferSyntax(iMessageID, originalTransferSyntax, notCompress);
	if (status != MC_NORMAL_COMPLETION)
		return  status;
	isCompressed = !notCompress;

	TRANSFER_SYNTAX transferSyntax = (TRANSFER_SYNTAX)originalTransferSyntax;
	TRANSFER_SYNTAX metaHeaderTransferSyntax = EXPLICIT_LITTLE_ENDIAN;
	unsigned long preambleSize = 132, metaHeaderSize, offsetPixelInMsg;
	
	
#ifdef UseXTDcmlib
	status = MC_Get_PixelOffset(iMessageID,&oPixelOffset);
	if (status != MC_NORMAL_COMPLETION)
	{
		GetAqLogger()->LogMessage("ERROR: TRDICOMUtil::GetPixelDataFileOffset error\n ");

		return status;
	}
#else
	// Get DICOM header size, if pixel data exists
	status = MC_Get_Stream_Length(iMessageID, 0x00000000, MC_ATT_PRIVATE_INFORMATION, &metaHeaderSize, metaHeaderTransferSyntax);
	if(status == kNormalCompletion)
		status = MC_Get_Stream_Length(iMessageID, 0x00030000, MC_ATT_GROUP_7FE0_LENGTH, &offsetPixelInMsg, transferSyntax);	
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}
	oPixelOffset =  preambleSize + metaHeaderSize + offsetPixelInMsg;
	// adding 4 bytes for group and element
	// group 2 bytes 0x7FE0
	// element 2 bytes 0x0010
	oPixelOffset += 4;

	// adding 4 bytes for length
	oPixelOffset += 4;

	// for explicit transfersyntaxes, 4 more bytes are added for
	// OB/OW typecode and 2 bytes reserved
	if ( ( transferSyntax != IMPLICIT_LITTLE_ENDIAN ) && ( transferSyntax != IMPLICIT_BIG_ENDIAN ) )
	{
		oPixelOffset += 4;
	}
#endif
	//Calculate pixel size
	status = MC_Get_Value_Length(iMessageID, MC_ATT_PIXEL_DATA, 1, &oPixelSize);
	if (status != MC_NORMAL_COMPLETION)
		return  status;

	return MC_NORMAL_COMPLETION;
}
//--------------------------------------------------------------------------------
// Output is: m_serviceListName
// 
int TRDICOMUtil::GenerateServiceList(const char* iSOPClassUID, int iStoredSyntax, int iProposedSyntax, std::string& oServiceListName)
{
	int status = MC_NORMAL_COMPLETION;
	iProposedSyntax = (iProposedSyntax == iStoredSyntax) ? 0 : iProposedSyntax;

	static int serviceCounter;

	++serviceCounter;

	char proposedSyntaxListName[64];   
	char proposedServiceName[64];      

	char storedSyntaxListName[64];   
	char storedServiceName[64];      

	char serviceListName[64];  

	_snprintf(proposedSyntaxListName, sizeof proposedSyntaxListName, "ProposedSyntaxList_%d", serviceCounter);
	_snprintf(proposedServiceName, sizeof proposedServiceName, "ProposedService_%d", serviceCounter);

	_snprintf(storedSyntaxListName, sizeof storedSyntaxListName, "StoredSyntaxList_%d", serviceCounter);
	_snprintf(storedServiceName, sizeof storedServiceName, "StoredService_%d", serviceCounter);

	_snprintf(serviceListName, sizeof serviceListName, "ServiceList_%d", serviceCounter);

	//	Set up syntax lists
	TRANSFER_SYNTAX syntaxArray[2];

	if (iProposedSyntax > 0)
	{
		syntaxArray[0] = (TRANSFER_SYNTAX) iProposedSyntax;
		syntaxArray[1] = (TRANSFER_SYNTAX) 0;
		status = MC_NewSyntaxList(proposedSyntaxListName, syntaxArray);
		if (status != MC_NORMAL_COMPLETION)
			return status;
	}

	syntaxArray[0] = (TRANSFER_SYNTAX) iStoredSyntax;
	syntaxArray[1] = (TRANSFER_SYNTAX) 0;
	status = MC_NewSyntaxList(storedSyntaxListName, syntaxArray);
	if (status != MC_NORMAL_COMPLETION)
		return status;

	//	Set up services
	if (iProposedSyntax > 0)
	{
		status = MC_NewServiceFromUID(proposedServiceName, (char*) iSOPClassUID, proposedSyntaxListName, 1, 0);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_FreeSyntaxList(proposedSyntaxListName);
			return status;
		}
	}

	status = MC_NewServiceFromUID(storedServiceName, (char*) iSOPClassUID, storedSyntaxListName, 1, 0);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_FreeSyntaxList(storedSyntaxListName);
		return status;
	}
			
	char* serviceNameArray[3];

	if (iProposedSyntax > 0)
	{
		serviceNameArray[0] = proposedServiceName;
		serviceNameArray[1] = storedServiceName;
		serviceNameArray[2] = 0;
	}
	else
	{
		serviceNameArray[0] = storedServiceName;
		serviceNameArray[1] = 0;
	}

	//	Create the service list
	status = MC_NewProposedServiceList(serviceListName, serviceNameArray);
	if (status != MC_NORMAL_COMPLETION)
	{
		if (iProposedSyntax > 0) 
		{
			MC_FreeSyntaxList(proposedSyntaxListName);
			MC_FreeService(proposedServiceName);
		}

		MC_FreeSyntaxList(storedSyntaxListName);
		MC_FreeService(storedServiceName);
		return status;
	}	

	char transferSyntax1Char[64];
	char transferSyntax2Char[64];

	if (iProposedSyntax > 0)
		MC_Get_Transfer_Syntax_From_Enum((TRANSFER_SYNTAX) iProposedSyntax, transferSyntax1Char, sizeof transferSyntax1Char);

	MC_Get_Transfer_Syntax_From_Enum((TRANSFER_SYNTAX) iStoredSyntax, transferSyntax2Char, sizeof transferSyntax2Char);

	oServiceListName = serviceListName;
	return status;
}

//----------------------------------------------------------------------------------------
//
const char* TRDICOMUtil::GetVersionString(void)
{
	static char buf[128];
	
	char mergeVersion[64];
	int vLength = sizeof mergeVersion;
	
	if (!buf[0])
	{
		int mcStatus = MC_Get_Version_String(vLength, mergeVersion);
		if (mcStatus == MC_NORMAL_COMPLETION)
		{
			strncpy(buf, mergeVersion, sizeof buf);
		}
	}
	return buf;
}


//----------------------------------------------------------------------------------------
//
bool TRDICOMUtil::GetTagName(unsigned long iTag, std::string& oName)
{

//	InitialDICOM("DBA6-5B453");

#if 0
	char name[1024];

	int status = MC_Get_Tag_Info(iTag, name, sizeof(name));
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}
	oName = name;
	return MC_NORMAL_COMPLETION;
#else
	DcmXTUtil	*pDcmXtUtil = IDcmLibApi::getDcmXTUtil();
	if(!pDcmXtUtil){
		return false;
	}
	 
	const char *tag_name = pDcmXtUtil->getTagName(iTag);
	if(tag_name){
		oName = tag_name;
		return true;
	}else{
		return false;
	}
#endif

	
}

//--------------------------------------------------------------------
//
std::string TRDICOMUtil::CalculateLocalAETitle(void)
{
	char localAE[17];
	std::string localAEStr = TRPlatform::GetMyNameLowercase() + std::string("-AQNET");

	strncpy(localAE, localAEStr.c_str(), sizeof(localAE));
	localAE[sizeof(localAE)-1] = 0;

	return localAE;
}

//--------------------------------------------------------------------
//
std::string TRDICOMUtil::CalculateInboundLocalAETitle(void)
{
	char localAE[17];
	std::string localAEStr = TRPlatform::GetMyNameUppercase() + std::string("_AE");

	strncpy(localAE, localAEStr.c_str(), sizeof(localAE));
	localAE[sizeof(localAE)-1] = 0;

	return localAE;
}

//--------------------------------------------------------------------



//--------------------------------------------------------------------
// CreateDICOMDIR is called ti create DICOMDIR
// User shall provide following parameters:
// iRootDirPath (DICOM files root directory)
// iDICOMDirOPPath (DICOMDIR output folder)
// mangledFileNameMap (map of dicom files)
//--------------------------------------------------------------------
//
//Pass Directory and this funtion will create DICOMDIR for that Directory
int TRDICOMUtil::CreateDICOMDIR(std::string iRootDirPath, std::string iDICOMDirOPPath, std::map<std::string, std::string> mangledFileNameMap)
{
	PxDicomStatus aVliStatus;	
	CBinfo cbinfo;			
	TRDicomDirInfo  in_DDInfo;	//Structure to save DICOMDIR information
	
	//If empaty map is passed iterate through root directory to get list of files.
	if (mangledFileNameMap.size() == 0)
	{
		if (iDICOMDirOPPath.at(iDICOMDirOPPath.size()-1) != '\\')
		{
			iDICOMDirOPPath = iDICOMDirOPPath +'\\';
		}

		CreateDCMFileName(iRootDirPath, iDICOMDirOPPath, mangledFileNameMap);
	}//end of if

	//Initialize the TRDicomDirInfo Structure
	aVliStatus = initializeDirInfo(&in_DDInfo, iDICOMDirOPPath);
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Error on initializeDirInfo -- %d\n ", aVliStatus);
		return aVliStatus;
	}

	std::map<std::string, std::string>::iterator iter;
	int modifyDICOMDIR = 0;

	//Add Patient\Study\Series\Image to DICOMDIR Object
	for(iter = mangledFileNameMap.begin(); iter != mangledFileNameMap.end(); iter++)
	{
		//For each file, read necessary information and add it to DICOMDIR object
		aVliStatus = AddMessageToDICOMDIR(iter->first.c_str(),iter->second.c_str(), &in_DDInfo);
		if (aVliStatus != kNormalCompletion)
		{
			modifyDICOMDIR = 0;
			break;
		}
		else
			modifyDICOMDIR =1;

	}
	//If DICOMDIR creation is successful, write the DICOMDIR object to file on disk
	if (modifyDICOMDIR)
    {        
		aVliStatus = (PxDicomStatus)MC_Write_File(in_DDInfo.dirID, 0, &cbinfo, AqFileObjToMedia);
        if (aVliStatus != kNormalCompletion)
			{
				GetAqLogger()->LogMessage("ERROR: Error on MC_Write_File -- %d\n ", aVliStatus);
				return aVliStatus;
			}
	}
	else
	{
		GetAqLogger()->LogMessage("ERROR: Creation of DICOMDIR failed -- %d\n ", aVliStatus);
		return aVliStatus;
	}
	//Release Message
	aVliStatus = (PxDicomStatus)MC_Free_File(&in_DDInfo.dirID); 
	if (aVliStatus != kNormalCompletion)
	{
	  GetAqLogger()->LogMessage("ERROR: Unable to free file object -- %d\n ", aVliStatus);
	  return aVliStatus;
	}
	
	//To release the buffer taken by Merge memory maneger, we must call this function 
	
    MC_Cleanup_Memory(5);  
	return kNormalCompletion;
}
//--------------------------------------------------------------------
// If empty map is provied, this function is called to get all dicom files
// This feature is used for DICOMEditor
//--------------------------------------------------------------------

PxDicomStatus CreateDCMFileName(std::string iRootDirPath, std::string iDICOMDirOPPath, std::map<std::string, std::string>& mangledFileNameMap)
{
		std::vector<TRFile> allFiles;
		std::vector<TRFile>::iterator iter, delIter;
		int aRandDirName = 0, size=0;
		int aRandDFileName = 0;
		char aTempbuf[MAX_PATH];
		BOOL aStatus=0;
		std::string aOutputFIleName;
		std::string aFileName;
		unsigned long aDummy1, aDummy2;
		PxDicomStatus aVliStatus;	

		// Get lists of all the Directory & Files by iterative search under directory iRootDirPath
		iGetDirectoryListR(iRootDirPath.c_str(), allFiles, 0);

		CreatePatientDir(iDICOMDirOPPath, &aRandDirName);
		aRandDFileName = 0;

		//Filter all the Directory and update vector to keep all the files only
		for(iter = allFiles.begin(); iter != allFiles.end(); iter++)
		{
			
			CPxDicomMessage aVLIDicomMsg;
			if(iter->IsDirectory())
			{
				
				delIter = iter;
				iter--;			//Move iterator to previous entry before deletion and next one is delIter
				allFiles.erase(delIter);
			}
			else
			{
				aFileName = iter->GetName();
				size = aFileName.size() - 8;
				if(aFileName.compare(size, 8,"DICOMDIR") == 0)
				{
					delIter = iter;
					iter--;			//Move iterator to previous entry before deletion and next one is delIter
					allFiles.erase(delIter);
				}
				aVliStatus = aVLIDicomMsg.LoadHeader(iter->GetName(), aDummy1, aDummy2);
				//Delete non-DICOM files from Vector
				if (aVliStatus != kNormalCompletion)
				{
					delIter = iter;
					iter--;			//Move iterator to previous entry before deletion and next one is delIter
					allFiles.erase(delIter);
				}
				else
				{
					_snprintf(aTempbuf, MAX_PATH-1, "%d%s%dDCM", 
										aRandDirName, 
										"/", 
										++aRandDFileName);
					mangledFileNameMap[iter->GetName()] = aTempbuf;
					aOutputFIleName = iDICOMDirOPPath + aTempbuf;
					::CopyFile((LPCTSTR)iter->GetName(), (LPCTSTR)aOutputFIleName.c_str(), aStatus);
				}
			}
			
			
		}
		return kNormalCompletion;
}
/*************************************************************************
 *
 *  Function    :   initializeDirInfo()
 *
 *  Parameters  :   TRDicomDirInfo *in_DDInfo 
 *
 *  Returns     :   nothing
 *
 *  Description :   Initializes the DICOM Directory, and initializes
 *                  the library.  Sets up signals and forks a process to 
 *                  handle the associations if defined UNIX.
 *
 *************************************************************************/
PxDicomStatus initializeDirInfo(TRDicomDirInfo* in_DDInfo, std::string iRootDirPath)
{
    PxDicomStatus		aVliStatus;
    int					isLast;         /* variable used for MC_Dir...  calls */

	
	in_DDInfo->direxists = in_DDInfo->memory = 0;
    in_DDInfo->nextRecID = 0;
    in_DDInfo->lowerRecID = 0;
	
	//Set DIDOCMDIR PATH here
	strcpy(in_DDInfo->dirname,iRootDirPath.c_str());
	strcat(in_DDInfo->dirname,kDicomDirFileName);

	//Check if DICOMDIR exists
	DWORD dwAttr = GetFileAttributes(in_DDInfo->dirname);
	DWORD dwError = GetLastError();
	if(!((dwError == ERROR_FILE_NOT_FOUND) || (dwError == ERROR_PATH_NOT_FOUND)))
	{
		  if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
			  in_DDInfo->direxists = 0;// Directory found, then set it to as not found and recreate it again
	}
	   
    //  Register this DICOM application MERGE_MEDIA is used to write
    //  to the DICOM storage media.
    aVliStatus = (PxDicomStatus)MC_Register_Application(&in_DDInfo->fsuappID,FSUAETITLE);
    if (aVliStatus == kAlreadyRegistered)
		GetAqLogger()->LogMessage("Information: FSUAETITLE already registered -- %d\n ", aVliStatus);
	else if (aVliStatus != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Failed MC_Register_Application for FSUAETITLE -- %d\n ", aVliStatus);
		return aVliStatus;
    }

     //Create new DICOMDIR object by registering with DICOMDIR service defined within service configuration file
    aVliStatus = (PxDicomStatus)MC_Create_File(&in_DDInfo->dirID, in_DDInfo->dirname, kDicomDirServiceName, C_STORE_RQ);
     if (aVliStatus != kNormalCompletion)
    {
        GetAqLogger()->LogMessage("ERROR: MC_Create_File failed -- %d\n ", aVliStatus);
		return  aVliStatus;
    }

	CBinfo	cbinfo;
	//Check if DICOMDIR exists, if so, load it in -- verify with Rob
   if (in_DDInfo->direxists)
    {
        aVliStatus = (PxDicomStatus)MC_Open_File(in_DDInfo->fsuappID, in_DDInfo->dirID, &cbinfo, AqMediaToFileObj);
		if (aVliStatus != kNormalCompletion)
        {
            GetAqLogger()->LogMessage("ERROR: Failed MC_Open_File call - Unable to read file from media -- %d\n ", aVliStatus);
			return  aVliStatus;
        }
    }
    else
    {
        //Intialize DICOMDIR's Attributes if DICOMDIR doesn't exists
        aVliStatus = AddDICOMDIRGroupTwoElements(in_DDInfo);
        if (aVliStatus != kNormalCompletion)
        {
            GetAqLogger()->LogMessage("ERROR: Failed AddDICOMDIRGroupTwoElements (Unable to add group 2 elements to DICOMDIR) -- %d\n ", aVliStatus);
			return aVliStatus;
        }
    }

    //Initialize internal variables that keep track of the current
    //location within the DICOMDIR
     
    aVliStatus = (PxDicomStatus)MC_Dir_Root_Entity(in_DDInfo->dirID, &in_DDInfo->entityID,
                                &in_DDInfo->recID, &in_DDInfo->type, &isLast);
    if (aVliStatus != kNormalCompletion)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to get root entity of DICOMDIR -- %d\n",aVliStatus);
		return  kNormalCompletion;
    }

    if (in_DDInfo->recID)
    {
        aVliStatus = (PxDicomStatus)MC_Get_Value_To_Int(in_DDInfo->recID,
                                     MC_ATT_OFFSET_OF_THE_NEXT_DIRECTORY_RECORD,
                                     &in_DDInfo->nextRecID);
        if (aVliStatus != kNormalCompletion)
        {
            GetAqLogger()->LogMessage("ERROR: Unable to get offset of the next directory record from DICOMDIR - %d\n", aVliStatus);
            return  aVliStatus;
        }
        aVliStatus = (PxDicomStatus)MC_Get_Value_To_Int(in_DDInfo->recID,
                     MC_ATT_OFFSET_OF_REFERENCED_LOWER_LEVEL_DIRECTORY_ENTITY,
                     &in_DDInfo->lowerRecID);
        if (aVliStatus != kNormalCompletion)
        {
            GetAqLogger()->LogMessage("ERROR: Unable to get offset of lower level directory entity of DICOMDIR -- %d\n", aVliStatus);
            return  aVliStatus;
        }
    }

	return kNormalCompletion;
} /* initializeDirInfo() */

/****************************************************************************
 *
 *  Function    :   AddDICOMDIRGroupTwoElements
 *
 *  Description :   Adds the required group two elements to a DIOCMDIR 
 *                  object.  Note that the implementation class UID and
 *                  implementation version name are automatically filled in
 *                  by the tool kit based on the mergecom.pro config file.
 *
 ****************************************************************************/
PxDicomStatus AddDICOMDIRGroupTwoElements( TRDicomDirInfo *in_DDInfo )
{
    MC_STATUS mcStatus;

    mcStatus = MC_Set_Value_From_String(
                        in_DDInfo->dirID,
                        MC_ATT_TRANSFER_SYNTAX_UID,
                        kImplicitLittleEndianTransferSyntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to add transfer syntax UID -- %d\n", mcStatus);
		return (PxDicomStatus) mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_Function(
                        in_DDInfo->dirID,
                        MC_ATT_FILE_META_INFORMATION_VERSION,
                        NULL,
                        AqFileMetaInfoVersion);
	
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
       GetAqLogger()->LogMessage("ERROR: Unable to add file meta information version -- %d\n", mcStatus);
       return (PxDicomStatus) mcStatus;
    }
     
    mcStatus = MC_Set_Value_From_String(
                        in_DDInfo->dirID,
                        MC_ATT_MEDIA_STORAGE_SOP_CLASS_UID,
                        kMediaStorageSOPClassUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to add media storage SOP Class UID -- %d\n", mcStatus);
        return (PxDicomStatus) mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(
                        in_DDInfo->dirID,
                        MC_ATT_MEDIA_STORAGE_SOP_INSTANCE_UID,
                        TRPlatform::GenerateUID().c_str()/*Create_Inst_UID()*/);

	if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to add media storage SOP instance UID -- %d\n", mcStatus);
        return (PxDicomStatus) mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(
                        in_DDInfo->dirID,
                        MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,
                        TRDICOMUtil::CalculateLocalAETitle().c_str());
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to add media storage SOP instance UID -- %d\n", mcStatus);
        return (PxDicomStatus) mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_String(in_DDInfo->dirID, MC_ATT_FILE_SET_ID, FSUAETITLE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to add file set ID -- %d\n", mcStatus); 
		return (PxDicomStatus) mcStatus;
    }
    
    mcStatus = MC_Set_Value_From_UInt(in_DDInfo->dirID, MC_ATT_FILE_SET_CONSISTENCY_FLAG,0x0000);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        GetAqLogger()->LogMessage("ERROR: Unable to add file set consistency flag -- %d\n", mcStatus);
        return (PxDicomStatus) mcStatus;
    }

    return (PxDicomStatus) MC_NORMAL_COMPLETION; 
    
} /* End AddDICOMDIRGroupTwoElements() */



/****************************************************************************
 *
 *  Function    :   AddMessageToDICOMDIR
 *
 *  Description :   Adds a directory record to the DICOMDIR dirID for the
 *                  message A_msgID.  If records do not exist for the patient,
 *                  study, and series for the message, these records will
 *                  also be created.
 *
 ****************************************************************************/
PxDicomStatus AddMessageToDICOMDIR(const char* oldFileName, const char* newFileName, TRDicomDirInfo* in_DDInfo )
{
    //char* A_serviceName;
	MC_STATUS mcStatus;
	PxDicomStatus mVliStatus;
    char    patientID[100];     /* These 3 buffers are the patient,    */
    char    studyUID[UI_LENGTH+1]; /* series, and study identifiers found */
    char    seriesUID[UI_LENGTH+1];/* in the message A_msgID.              */
    
    char    dirPatientID[100];        /* These 3 buffers are used to get the    */
    char    dirStudyUID[UI_LENGTH+1]; /* patient, series, and study identifiers */
    char    dirSeriesUID[UI_LENGTH+1];/* while traversing the DICOMDIR          */

    int     entityID;   /* to keep track of entity while traversing DICOMDIR */
    int     recID;      /* to keep track of records while traversing DICOMDIR */
    int     isLast;     /* used while traversing DICOMDIR */
    char*   itemType;   /* type of directory record */
    char*   recTypeToCreate;   /* type of directory record to create */
    int     first;

    int     createPatient = 1;     /* These 6 boolean variables are used to  */
    int     createStudy = 1;       /* identify which records need to be      */
    int     createSeries = 1;      /* created in the DICOMDIR to place the   */
    int     createStudyEntity = 1; /* image A_msgID into the the DICOMDIR.    */
    int     createSeriesEntity = 1;/* They also tell whether entities        */
    int     createImageEntity = 1; /* have to be created.                    */


    /*
     * Get the patient ID, study instance UID, and series instance UID
     * identifiers from the message object.  These strings are used to 
     * compare with records in the DICOMDIR to see if the patient,
     * study, or series already appear in the DICOMDIR.*/

	unsigned long aDummy1, aDummy2;
	CPxDicomMessage aVLIDicomMsg;
	PxDicomStatus status;

	status = aVLIDicomMsg.LoadHeader(oldFileName, aDummy1, aDummy2);
	if ( status != kNormalCompletion )
	{
		GetAqLogger()->LogMessage("ERROR: CPxDicomMessage LoadHeader Failed-- %d\n", status); 
        return (PxDicomStatus)kAqErrLoadDicomFileFailed;
    }
		     
    status = aVLIDicomMsg.GetValue(kVLIPatientId, patientID);
    if (status != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Unable to get Patient ID from message -- %d\n", status); 
        return (PxDicomStatus) MC_INVALID_MESSAGE_ID;
    }
    
	status = aVLIDicomMsg.GetValue(kVLIStudyInstanceUid, studyUID);
    if (status != MC_NORMAL_COMPLETION) 
    {
       	GetAqLogger()->LogMessage("ERROR: Unable to get study instance UID from message -- %d\n", status); 
        return (PxDicomStatus) MC_INVALID_MESSAGE_ID;
    }
    
	status = aVLIDicomMsg.GetValue(kVLISeriesInstanceUid, seriesUID);
    if (status != MC_NORMAL_COMPLETION) 
    {
        GetAqLogger()->LogMessage("ERROR: Unable to get series instance UID from message -- %d\n", status); 
        return (PxDicomStatus) MC_INVALID_MESSAGE_ID;
    }
    /*
     * The following loop scans to see if the root entity contains a record
     * for the same Patient ID as in A_msgID.  If this is the case, it continues
     * on searching the referenced entity of this record to see if the study
     * already appears for the patient.  If the study exists, it searches its
     * lower level entity to see if the series already exists for this image.
     * If this is the case, it places a new image record in the lower
     * level entity of the series record.
     */
    isLast = 0;
    first = 1;
    while (!isLast)
    {
        if (first)
        {
            mcStatus = MC_Dir_Root_Entity(in_DDInfo->dirID, &entityID, &recID, &itemType, &isLast);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                GetAqLogger()->LogMessage("ERROR: Unable to get root entity record from DICOMDIR -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            }

            if (recID == 0)
            {
                createPatient = 1;
                break; 
            } 
            first = 0;
        }
        else
        {
            mcStatus = MC_Dir_Next_Record(in_DDInfo->dirID, entityID, &recID, &itemType, &isLast);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
               	GetAqLogger()->LogMessage("ERROR: Unable to get root entity record from DICOMDIR -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            }
        } 

        mcStatus = MC_Get_Value_To_String(recID, MC_ATT_PATIENT_ID, sizeof(dirPatientID), dirPatientID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            GetAqLogger()->LogMessage("ERROR: Unable to get patient ID from root directory entity -- %d\n", mcStatus); 
            return (PxDicomStatus) mcStatus;
        } 
            
        if (!strcmp(patientID,dirPatientID))
        {
            /*
             * At this point a matching patient ID has been found
             */
            createPatient = 0;

            mcStatus = MC_Dir_Next_Entity(in_DDInfo->dirID, recID, &entityID, &recID, &itemType, &isLast); 
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
             	GetAqLogger()->LogMessage("ERROR: Unable to get next entity below patient record -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            }
            if (entityID == 0)
            {
                createStudyEntity = 1;
                break; /* patient ID is found, but no study entity found */
            }
            else
                createStudyEntity = 0;

            /*
             * Scan if study directory record ready
             */        
            isLast = 0;
            first = 1;

            while (!isLast)
            {
                if (first)
                {
                    mcStatus = MC_Dir_First_Record(in_DDInfo->dirID, entityID,
                                                 &recID, &itemType, &isLast);
                    first = 0;
                }
                else
                {
                    mcStatus = MC_Dir_Next_Record(in_DDInfo->dirID, entityID,
                                                &recID, &itemType, &isLast);
                } 
                if (mcStatus != MC_NORMAL_COMPLETION) 
                {
                    GetAqLogger()->LogMessage("ERROR: Unable to get directory record from study entity -- %d\n", mcStatus); 
                    return (PxDicomStatus) mcStatus;
                }

                mcStatus = MC_Get_Value_To_String(recID, 
                                                MC_ATT_STUDY_INSTANCE_UID,
                                                sizeof(dirStudyUID),
                                                dirStudyUID);
                if (mcStatus != MC_NORMAL_COMPLETION) 
                {
                   	GetAqLogger()->LogMessage("ERROR: Unable to get study instance UID from directory record -- %d\n", mcStatus); 
                    return (PxDicomStatus) mcStatus;
                }
                 
                if (!strcmp(studyUID,dirStudyUID))
                {
                    /*
                     * At this point a matching patient ID and study UID 
                     * has been found in the DICOMDIR
                     */
                    createStudy = 0;

                    mcStatus = MC_Dir_Next_Entity(in_DDInfo->dirID, recID,
                                                &entityID, &recID, &itemType,
                                                &isLast); 
                    if (mcStatus != MC_NORMAL_COMPLETION) 
                    {
                       	GetAqLogger()->LogMessage("ERROR: Unable to get next entity below study record -- %d\n", mcStatus); 
                        return (PxDicomStatus) mcStatus;
                    }

                    if (entityID == 0)
                    {
                        /* patient ID is found, study found, but no series
                         *  entity found
                         */
                        createSeriesEntity = 1;
                        break; 
                    }
                    else
                        createSeriesEntity = 0;
                
                    /*
                     * Scan if series directory record ready
                     */        
                    isLast = 0;
                    first = 1;
                   
                    while (!isLast)
                    {
                        if (first)
                        {
                            mcStatus = MC_Dir_First_Record(in_DDInfo->dirID, 
                                                         entityID, &recID,
                                                         &itemType, &isLast);
                            first = 0;
                        }
                        else
                        {
                            mcStatus = MC_Dir_Next_Record(in_DDInfo->dirID,
                                                        entityID, &recID,
                                                        &itemType, &isLast);
                            
                        } 
                        if (mcStatus != MC_NORMAL_COMPLETION) 
                        {
                           	GetAqLogger()->LogMessage("ERROR: Unable to get series record from directory record -- %d\n", mcStatus); 
                            return (PxDicomStatus) mcStatus;
                        }

                        mcStatus = MC_Get_Value_To_String(recID,
                                      MC_ATT_SERIES_INSTANCE_UID,
                                      sizeof(dirSeriesUID),dirSeriesUID);
                        if (mcStatus != MC_NORMAL_COMPLETION) 
                        {
							GetAqLogger()->LogMessage("ERROR: Unable to get series instance UID from message -- %d\n", mcStatus); 
                            return (PxDicomStatus) mcStatus;
                        } 
                        if (!strcmp(seriesUID,dirSeriesUID))
                        {
                            /*
                             * At this point a matching patient ID, study UID,
                             * and series UID has been found in the DICOMDIR,
                             * only the image record needs to be created
                             */ 
                            createSeries = 0;

                            mcStatus = MC_Dir_Next_Entity(in_DDInfo->dirID, recID,
                                          &entityID,&recID,&itemType,&isLast); 
                            if (mcStatus != MC_NORMAL_COMPLETION) 
                            {
                              	GetAqLogger()->LogMessage("ERROR: Unable to get next entity below series record -- %d\n", mcStatus); 
                                return (PxDicomStatus) mcStatus;
                            } 
                            if (entityID == 0)
                                createImageEntity = 1;
                            else 
                                /*
                                 * Note, at this point we always automatically
                                 * insert an image record.  We should at this 
                                 * point search the image entity to see if we've
                                 * already received this record, and reject
                                 * the C-STORE-RQ if we have.  We don't do this
                                 * here for simplicity sake.
                                 */
                                createImageEntity = 0;
                            
                            /* patient, study, and series already in DICOMDIR */
                            break;
                        } /* end if (!strcmp(seriesUID,dirSeriesUID)) */ 
                    }
                    break;
                } /* end if (!strcmp(studyUID,dirStudyUID)) */ 
            } 
            break;
        } /* end if (!strcmp(patientID,dirPatientID)) */

    } 
	 /*
     * Note, the below tests assume that the variables recID, and entityID 
     * are set correctly, along with the values for createPatient, 
     * createStudy, createSeries, createStudyEntity, createSeriesEntity,
     * and createImageEntity to properly create the correct records.
     */
    if (createPatient)
    {
        /*
         * Create Patient directory record
         */  
        mcStatus = MC_Dir_Add_Record(in_DDInfo->dirID, entityID, 
                                   "DIR_REC_PATIENT", &recID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            GetAqLogger()->LogMessage("ERROR: Unable to add patient directory record -- %d\n", mcStatus); 
            return (PxDicomStatus) mcStatus;
        } 
        mVliStatus = FillPatientRecord(&aVLIDicomMsg,recID); 
        if (mVliStatus != kNormalCompletion) 
        {
            MC_Dir_Delete_Record(in_DDInfo->dirID, recID);
            GetAqLogger()->LogMessage("ERROR: Unable to fill patient directory record -- %d\n", mcStatus); 
			return mVliStatus;
        }
        createStudyEntity = 1;
    }
    
    if (createStudy)
    {
        /*
         * Create Study directory record
         */
        if (createStudyEntity)
        {
            mcStatus = MC_Dir_Add_Entity(in_DDInfo->dirID, recID, 
                                       "DIR_REC_STUDY", &entityID, &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
               	GetAqLogger()->LogMessage("ERROR: Unable to add study directory entity -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            } 
            createSeriesEntity = 1;
        }
        else
        {
            mcStatus = MC_Dir_Add_Record(in_DDInfo->dirID, entityID, 
                                       "DIR_REC_STUDY", &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                GetAqLogger()->LogMessage("ERROR: Unable to add study directory record -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            }
        }
        mVliStatus = FillStudyRecord(&aVLIDicomMsg,recID); 
        if (mVliStatus != kNormalCompletion)
        {
            MC_Dir_Delete_Record(in_DDInfo->dirID, recID);
            GetAqLogger()->LogMessage("ERROR: Unable to fill study directory record -- %d\n", mcStatus); 
			return mVliStatus;
        }
            
    }

    if (createSeries)
    {
        /*
         * Create Series directory record
         */  
        if (createSeriesEntity)
        {
            mcStatus = MC_Dir_Add_Entity(in_DDInfo->dirID, recID, 
                                       "DIR_REC_SERIES", &entityID, &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                GetAqLogger()->LogMessage("ERROR: Unable to add series directory entity -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            } 
            createImageEntity = 1;
        }
        else
        {
            mcStatus = MC_Dir_Add_Record(in_DDInfo->dirID, entityID, 
                                       "DIR_REC_SERIES", &recID);
            if (mcStatus != MC_NORMAL_COMPLETION) 
            {
                GetAqLogger()->LogMessage("ERROR: Unable to add series directory record -- %d\n", mcStatus); 
                return (PxDicomStatus) mcStatus;
            }
        } 
        mVliStatus = FillSeriesRecord(&aVLIDicomMsg,recID);
        if (mcStatus != MC_NORMAL_COMPLETION)
        {
            MC_Dir_Delete_Record(in_DDInfo->dirID, recID);
            GetAqLogger()->LogMessage("ERROR: Unable to fill series directory record -- %d\n", mcStatus);
			return mVliStatus;
        }
    }     

    
    
    // TODO Create Image directory record always -- In future this might change, so using this variable
	recTypeToCreate = "DIR_REC_IMAGE";
    if (createImageEntity)
    {
        mcStatus = MC_Dir_Add_Entity(in_DDInfo->dirID, recID, 
                                   recTypeToCreate, &entityID, &recID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            GetAqLogger()->LogMessage("ERROR: Unable to add image directory entity -- %d\n", mcStatus); 
            return (PxDicomStatus) mcStatus;
        }
    }
    else
    {
        mcStatus = MC_Dir_Add_Record(in_DDInfo->dirID, entityID, 
                                   recTypeToCreate, &recID);
        if (mcStatus != MC_NORMAL_COMPLETION) 
        {
            GetAqLogger()->LogMessage("ERROR: Unable to add image directory record -- %d\n", mcStatus); 
			return (PxDicomStatus) mcStatus;
        }
    }

	//if (!strcmp(recTypeToCreate, "DIR_REC_IMAGE"));
	mVliStatus = FillImageRecord(&aVLIDicomMsg,recID,oldFileName, newFileName); 
    
    if (mVliStatus != kNormalCompletion) 
    {
        MC_Dir_Delete_Record(in_DDInfo->dirID, recID);
        GetAqLogger()->LogMessage("ERROR: Unable to fill directory record -- %d\n", mcStatus); 
    }

   
    return (PxDicomStatus) mcStatus;
} /* End AddMessageToDICOMDIR() */


/****************************************************************************
 *
 *  Function    :   (PxDicomStatus) FillPatientRecord
 *
 *  Description :   (PxDicomStatus) Fills the correct attributes of a patient directory 
 *                  record to match with the image pointed to by A_msgID.
 *	Error Handling	If Tyep 1 fail, then DICOMDIR creation fails. If Type to fails, 
 *					set to null and continue. Type 3 don't care
 ****************************************************************************/
PxDicomStatus  FillPatientRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID)
{
    PxDicomStatus aVliStatus;
	//MC_ATT_DIRECTORY_RECORD_TYPE
    aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE, "PATIENT");
    if (aVliStatus != kNormalCompletion) return  aVliStatus;
    
    aVliStatus = (PxDicomStatus)MC_Set_Value_From_UInt(A_recID, MC_ATT_RECORD_IN_USE_FLAG, 0xFFFF);
    if (aVliStatus != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Set of Patient RECORD IN USE FLAG failed - %d.\n", aVliStatus);
		return  aVliStatus;
    }
	//MC_ATT_PATIENTS_NAME
    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_PATIENTS_NAME, Type_2);
    if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_PATIENTS_NAME);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Patient Name to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}
	//MC_ATT_PATIENT_ID
    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_PATIENT_ID, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_PATIENT_ID);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Patient ID to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}  
	//MC_ATT_PATIENTS_BIRTH_DATE
    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_PATIENTS_BIRTH_DATE, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_PATIENTS_BIRTH_DATE);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Patient Birth Date to null failed - %d .\n", aVliStatus);
			return  aVliStatus;
		} 
	}
	//MC_ATT_PATIENTS_SEX
    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_PATIENTS_SEX, Type_2);
    if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_PATIENTS_SEX);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Patient Sex to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}
 
	//Don't care about error logging for Type 3
    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_PATIENT_COMMENTS, Type_3);
	
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SPECIFIC_CHARACTER_SET, Type_3);

    return kNormalCompletion;
    
} /* End FilePatientRecord() */

/****************************************************************************
 *
 *  Function    :   (PxDicomStatus) FillStudyRecord
 *
 *  Description :   (PxDicomStatus) Fills the correct attributes of a study directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 *	Error Handling	If Tyep 1 fail, then DICOMDIR creation fails. If Type to fails, 
 *					set to null and continue. Type 3 don't care
 ****************************************************************************/
PxDicomStatus  FillStudyRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID)
{
    PxDicomStatus   aVliStatus;

    aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE, "STUDY");
	if (aVliStatus != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Study Directory Record Creation Failed, error code - %d .\n", aVliStatus);
		return aVliStatus;
    } 

    aVliStatus = (PxDicomStatus) MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
	if (aVliStatus != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Set of Study RECORD IN USE FLAG failed - %d.\n", aVliStatus);
		return aVliStatus;
    }

    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_STUDY_DATE, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_STUDY_DATE);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Study Date to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}    


    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_STUDY_TIME, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_STUDY_TIME);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Study Time to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}    


    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_STUDY_DESCRIPTION, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_STUDY_DESCRIPTION);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Study description to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}    


    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_STUDY_INSTANCE_UID, Type_1);
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Set STUDY INSTANCE UID failed - %d.\n", aVliStatus);
		return  aVliStatus;
	}    


    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_STUDY_ID, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_STUDY_ID);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Study ID to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}    


    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_ACCESSION_NUMBER, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_ACCESSION_NUMBER);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Accesstion number to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		}
	}    
	
	aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SPECIFIC_CHARACTER_SET, Type_3);
    if (aVliStatus != kNormalCompletion) return  aVliStatus;

    return kNormalCompletion;
    
} /* End (PxDicomStatus) FillStudyRecord() */




/****************************************************************************
 *
 *  Function    :   (PxDicomStatus) FillSeriesRecord
 *
 *  Description :   (PxDicomStatus) Fills the correct attributes of a series directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 *	Error Handling	If Tyep 1 fail, then DICOMDIR creation fails. If Type to fails, 
 *					set to null and continue. Type 3 don't care
 ****************************************************************************/
PxDicomStatus  FillSeriesRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID)
{
    PxDicomStatus   aVliStatus;

    aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE,"SERIES");
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Set of Directory Record type failed - %d.\n", aVliStatus);
		return  aVliStatus;
	}

    aVliStatus = (PxDicomStatus)MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Set Series Record in Use failed - %d.\n", aVliStatus);
		return  aVliStatus;
	}

    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_MODALITY, Type_1);
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Set Modality failed - %d.\n", aVliStatus);
		return  aVliStatus;
	} 

    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SERIES_INSTANCE_UID, Type_1);
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Set Series INSTANCE UID failed - %d.\n", aVliStatus);
		return  aVliStatus;
	} 

    aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SERIES_NUMBER, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_SERIES_NUMBER);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set Series Number to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	} 
	
	aVliStatus = (PxDicomStatus)SetValue(in_VLIDCMMsg,A_recID,MC_ATT_INSTITUTION_NAME, Type_2);
	if (aVliStatus == kNullValue)
	{
		aVliStatus = (PxDicomStatus)MC_Set_Value_To_NULL(A_recID, MC_ATT_INSTITUTION_NAME);
		if (aVliStatus != kNormalCompletion)
		{
			GetAqLogger()->LogMessage("ERROR: Set INSTITUTION NAME to null failed - %d.\n", aVliStatus);
			return  aVliStatus;
		} 
	}  

	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SERIES_DATE, Type_3);

    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SERIES_TIME, Type_3);
    
    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SERIES_DESCRIPTION, Type_3);
    	
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SPECIFIC_CHARACTER_SET, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_INSTITUTION_ADDRESS, Type_3);

    return kNormalCompletion;
    
} /* End (PxDicomStatus) FillSeriesRecord() */


/****************************************************************************
 *
 *  Function    :   (PxDicomStatus) FillImageRecord
 *
 *  Description :   (PxDicomStatus) Fills the correct attributes of a image directory 
 *                  record to match with the image pointed to by A_msgID.
 *
 ****************************************************************************/

PxDicomStatus  FillImageRecord(CPxDicomMessage* in_VLIDCMMsg, int A_recID,const char* oldFileName ,const char* newFileName)
{
    PxDicomStatus   aVliStatus;
    char        uidBuffer[UI_LENGTH+2];
    

    aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID, MC_ATT_DIRECTORY_RECORD_TYPE, "IMAGE");
	if (aVliStatus != kNormalCompletion)
	{
		GetAqLogger()->LogMessage("ERROR: Set of Image Record type failed - %d.\n", aVliStatus);
		return  aVliStatus;
	}

    aVliStatus = (PxDicomStatus)MC_Add_Standard_Attribute(A_recID, MC_ATT_FRAME_OF_REFERENCE_UID);
	    
	//Set the SOP Class and Instance UIDs in the directory record
    aVliStatus = in_VLIDCMMsg->GetValue(MC_ATT_SOP_CLASS_UID, uidBuffer);
    if (aVliStatus != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Unable to get SOP Class UID from message -- %d\n", aVliStatus); 
        return  aVliStatus;
    }
    
    aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_CLASS_UID_IN_FILE,uidBuffer);
    if (aVliStatus != kNormalCompletion)
    {
       	GetAqLogger()->LogMessage("ERROR: Unable to set Referenced SOP Class UID in file -- %d\n", aVliStatus);
		return  aVliStatus;
    }

    aVliStatus = in_VLIDCMMsg->GetValue(MC_ATT_SOP_INSTANCE_UID, uidBuffer);
    if (aVliStatus != kNormalCompletion)
    {
       	GetAqLogger()->LogMessage("ERROR: Unable to get SOP Instance UID from message -- %d\n", aVliStatus); 
        return  aVliStatus;
    }
    
    aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID,MC_ATT_REFERENCED_SOP_INSTANCE_UID_IN_FILE,uidBuffer);
    if (aVliStatus != kNormalCompletion)
    {
       	GetAqLogger()->LogMessage("ERROR: Unable to set Referenced SOP Instance UID in file -- %d\n", aVliStatus);
		return  aVliStatus;
    }
    
    
    aVliStatus = (PxDicomStatus)MC_Set_Value_From_UInt(A_recID,MC_ATT_RECORD_IN_USE_FLAG,0xFFFF);
    if (aVliStatus != kNormalCompletion)
    {
		GetAqLogger()->LogMessage("ERROR: Unable to get SOP Class UID from message -- %d\n", aVliStatus); 
        return  aVliStatus;
    }


    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_IMAGE_NUMBER, Type_2);
    if (aVliStatus != kNormalCompletion) return  aVliStatus;

    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_IMAGE_DATE, Type_3);
    
    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_IMAGE_TIME, Type_3);
    
    aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_IMAGE_TYPE, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SPECIFIC_CHARACTER_SET, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_REFERENCED_TRANSFER_SYNTAX_UID_IN_FILE, Type_3, MC_ATT_TRANSFER_SYNTAX_UID);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SOP_CLASS_UID, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_SOP_INSTANCE_UID, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_IMAGE_POSITION_PATIENT, Type_3);

	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_IMAGE_ORIENTATION_PATIENT, Type_3);
    	
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_FRAME_OF_REFERENCE_UID, Type_3);
    	
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_ROWS, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_COLUMNS, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_PIXEL_SPACING, Type_3);
    
	aVliStatus = SetValue(in_VLIDCMMsg,A_recID,MC_ATT_CALIBRATION_IMAGE, Type_3);
    	
    aVliStatus = (PxDicomStatus)SetFileNameAttribute(in_VLIDCMMsg,A_recID, MC_ATT_REFERENCED_FILE_ID, newFileName);
	if (aVliStatus != MC_NORMAL_COMPLETION) 
	{
		GetAqLogger()->LogMessage("ERROR: SetFileNameAttribute Failed (Invalid Characters) -- %d\n", aVliStatus); 
		return (PxDicomStatus) aVliStatus;
	}

    return kNormalCompletion;
    
} 
/* End (PxDicomStatus) FillImageRecord() */
/****************************************************************************
 *
 *  Function    :   SetValue
 *
 *  Description :   Used to copy the attribute Atag from the message A_msgID
 *                  into the directory record ArecID.  This routine is 
 *                  specifically written not to give an error if Atag does
 *                  not exist in A_msgID. 
 *
 ****************************************************************************/
PxDicomStatus SetValue(CPxDicomMessage* in_VLIDCMMsg, int A_recID, unsigned long A_tag, DICOM_TYPE A_type, unsigned long CopyFromTag )
										   
{
    MC_STATUS		mcStatus;
	PxDicomStatus	aVlistatus;
    char			string[1024];
    char			errorString[128];
	int				aCount = 0;
	unsigned long	sourceTag = (CopyFromTag) ? CopyFromTag : A_tag;
	
	in_VLIDCMMsg->GetValueCount(sourceTag, &aCount);
	
	if (sourceTag == MC_ATT_TRANSFER_SYNTAX_UID)
	{
		TRANSFER_SYNTAX transferSyntax;
		MC_Get_Message_Transfer_Syntax(in_VLIDCMMsg->GetID(), &transferSyntax);
		MC_Get_Transfer_Syntax_From_Enum(transferSyntax, string, 64);
	}
	else
	{
		aVlistatus = in_VLIDCMMsg->GetValue(sourceTag, string);
	}

	if (aVlistatus == kNormalCompletion)
	{
		mcStatus = MC_Set_Value_From_String(A_recID,A_tag,string);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			if (A_type == Type_1)
			{
				sprintf(errorString,"Type 1 attribute missing for Tag (%lx)",A_tag);
				GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, mcStatus); 
				return (PxDicomStatus) mcStatus;
			}
		}
	}
	else if (aVlistatus == kNullValue)
	{
		if (A_type == Type_2) //if null exist, set value to null for tyep2
		{
			mcStatus = MC_Set_Value_To_NULL(A_recID, A_tag);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				sprintf(errorString,"Setting to null for Type 2 for Tag(%lx) failed",A_tag);
               	GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, mcStatus); 
				return (PxDicomStatus) mcStatus;
			}
		}
		else if (A_type == Type_1) //if null exist, error out for type1
		{
			sprintf(errorString,"Value for tag (%lx) set to NULL, expecting value",A_tag);
			GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, aVlistatus); 
			return aVlistatus;
    
		}
	}
	else if (A_type == Type_2)  //If error occur, set value to null for type2
	{
		mcStatus = MC_Set_Value_To_NULL(A_recID, A_tag);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			sprintf(errorString,"Error setting for tag (%lx) to NULL",A_tag);
           	GetAqLogger()->LogMessage("ERROR: %s, eoror code -> %d\n",errorString, mcStatus); 
			return (PxDicomStatus) mcStatus;
		}
	}
	else if (A_type == Type_1) //if any other error for type1, error out
	{
		sprintf(errorString, 
				"Error with tag (%lx), expecting value in message", 
				A_tag);
       	GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, aVlistatus); 
		return aVlistatus;
	}
	//For multiple VR
	for (int i= 2; i <= aCount; i++)
	{
		aVlistatus = in_VLIDCMMsg->GetNextValue(sourceTag, string);

		if (aVlistatus == kNormalCompletion)
		{
			mcStatus = MC_Set_Next_Value_From_String(A_recID,A_tag,string);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				if (A_type == Type_1)
				{

					sprintf(errorString,"Setting value to tag (%lx) failed (Type 1)",A_tag);
					GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, mcStatus); 
					return (PxDicomStatus) mcStatus;
				}
			}
		}
		else if (aVlistatus == kNullValue)
		{
			if (A_type == Type_2) //if null exist, set value to null for tyep2
			{
				mcStatus = MC_Set_Next_Value_To_NULL(A_recID, A_tag);
				if (mcStatus != MC_NORMAL_COMPLETION)
				{
					sprintf(errorString,"Error setting tag (%lx) to NULL for Type 2",A_tag);
               		GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, mcStatus); 
					return (PxDicomStatus) mcStatus;
				}
			}
			else if (A_type == Type_1) //if null exist, error out for type1
			{
				sprintf(errorString,"Value for tag (%lx) set to NULL for Type1, expecting value",A_tag);
				GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, aVlistatus); 
				return aVlistatus;
    
			}
		}
		else if (A_type == Type_2)  //If error occur, set value to null for type2
		{
			mcStatus = MC_Set_Next_Value_To_NULL(A_recID, A_tag);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				sprintf(errorString,"Error setting tag (%lx) to NULL for Type2",A_tag);
           		GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, mcStatus); 
				return (PxDicomStatus) mcStatus;
			}
		}
		else if (A_type == Type_1) //if any other error for type1, error out
		{
			sprintf(errorString, "Error with tag (%lx), expecting value in message for Type1", A_tag);
       		GetAqLogger()->LogMessage("ERROR: %s, error code -> %d\n",errorString, aVlistatus); 
			return aVlistatus;
		}
    }
    return kNormalCompletion;
    
} /* End SetValue() */

/****************************************************************************
 *
 *  Function    :   SetFileNameAttribute
 *
 *  Description :   Sets a filename attribute from a string.  This must be 
 *                  done because the DICOM standard dictates that heiarchial
 *                  directorys are listed as seperate values.
 *
 *                  This routine was written for the case where files/images
 *                  are placed in a directory within the root DICOMDIR 
 *                  directory.  This application currently just places
 *                  incoming files in the root directory. (ie, the same
 *                  directory the DICOMDIR is located in.)
 *
 ****************************************************************************/
PxDicomStatus SetFileNameAttribute( CPxDicomMessage* in_VLIDCMMsg, int A_recID, unsigned long A_tag, const char* newFileName)
{
    PxDicomStatus aVliStatus;
    char   aTempBuffer[PATH_LENGTH+10];
    char*  aStrTokenPtr;
    int    first = 1;
	int aLen =0;
	std::string aStr1, aStr2;

	aStr1 = newFileName;
	strcpy(aTempBuffer,aStr1.c_str());
    aStrTokenPtr = strtok(aTempBuffer,"/");
    
    while (aStrTokenPtr)
    {
        if (first)
        {
            first = 0;
            aVliStatus = (PxDicomStatus)MC_Set_Value_From_String(A_recID,A_tag,aStrTokenPtr);
            if (aVliStatus != kNormalCompletion) return aVliStatus;
        }
        else
        {
            aVliStatus = (PxDicomStatus)MC_Set_Next_Value_From_String(A_recID,A_tag,aStrTokenPtr);
            if (aVliStatus != kNormalCompletion) return aVliStatus;
        }

        aStrTokenPtr = strtok(NULL,"/");
    } 

    return kNormalCompletion;
    
} /* End SetFileNameAttribute() */

//------------------------------------------------------------------------------------------------

bool CreatePatientDir(std::string iDICOMDirOPPath, int *outDirNameIntVal)
{
	char aTempBuf[8];
	int aReturnStatus;
	std::string outDirname;
	
	//Try creating four times
	for (int i = 0 ; i< 8 ; i++)
	{
		*outDirNameIntVal = GetRandNumber();
		sprintf(aTempBuf,"%d",*outDirNameIntVal);
		outDirname = iDICOMDirOPPath + aTempBuf;
		aReturnStatus = ::CreateDirectory(outDirname.c_str(), NULL);
		if (aReturnStatus == ERROR_ALREADY_EXISTS)
		{
			continue;
		}
		else if (aReturnStatus == ERROR_PATH_NOT_FOUND)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------

int GetRandNumber()
{
	//Random Number generation
	int aRandNo = 0;
	srand ( time(NULL) );
	aRandNo = rand() % 9999;
	return aRandNo;
		
}

//--------------------------------------------------------------------
bool TRDICOMUtil::IsDICOMDirMessage( int iMessageID )
{
	unsigned int nOffsetofFirstDirRecoftheRootDirEntity = 0;
	if ( MC_Get_Value_To_UInt( iMessageID, 0x00041200, &nOffsetofFirstDirRecoftheRootDirEntity ) != MC_NORMAL_COMPLETION )
		return false;

	if ( nOffsetofFirstDirRecoftheRootDirEntity == 0 )
		return false;

	return true;
}

void* MC_Config_Values(void) 
{
	return 0;
}
 void* MC_Dictionary_Values(void) 
{
	return 0;
} 
void* MC_MsgInfo_Values(void)  
{
	return 0;
}

///////////
//#8 2012/03/09 K.KO
	//ServiceList指定仕組みの追加
bool TRDICOMUtil::initServiceList(const char *ServiceName,bool isPropose)
{
	DcmXTUtil	*pDcmXtUtil = IDcmLibApi::getDcmXTUtil();
	if(!pDcmXtUtil){
		return false;
	}
 
	bool ret_b =  pDcmXtUtil->createServiceList(isPropose/*isPropose*/,ServiceName);

	if(!ret_b) return ret_b;

	ret_b = pDcmXtUtil->clearServiceList(isPropose/*isPropose*/,ServiceName);
	return ret_b;
 
}
bool TRDICOMUtil::addSOPClassUID(const char * sopUID,const char *ServiceName,bool isPropose)
{
	DcmXTUtil	*pDcmXtUtil = IDcmLibApi::getDcmXTUtil();
	if(!pDcmXtUtil){
		return false;
	}
	return pDcmXtUtil->addSOPClassUID(isPropose/*isPropose*/,sopUID, ServiceName);
 
}
