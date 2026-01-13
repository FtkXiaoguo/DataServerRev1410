// MyDicomFile.cpp: CMyDicomFile クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "MyDicomFile.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CMyDicomFile::CMyDicomFile()
{

}

CMyDicomFile::~CMyDicomFile()
{

}
int CMyDicomFile::loadDicomHeader(const char *iFilePath)
{
#if 1
	int status = Load(iFilePath,1 /*iHeaderOnly*/);
#else
	int status;

	m_filePath = iFilePath;

 
//		status = LoadHeader(iFilePath, 0);

#if 0
	 	status = LoadHeaderEx(iFilePath, 1);
#else

		status = LoadHeaderEx(iFilePath, m_pixelOffset,m_pixelSize);
#endif

	 	if (status != kNormalCompletion)
		{
			status = LoadHeaderFromStream(iFilePath, 0);
			if (status != kNormalCompletion)
			{
				return status;
			}
		}
	
		PopulateFromMessage(m_messageID);
	
		status =  kNormalCompletion;
	 
#endif
	return status;
}

#if 0
#include "rtvMergeToolKit.h"
#include "rtvsutil.h"
#else
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
#endif

#include "PxDicomMessage.h"

//
 //#include "AqCore/TRPlatform.h"
 




 ///////////////
 MC_STATUS MyAqMediaToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast)
{

    MyMediaCBinfo*    callbackInfo = (MyMediaCBinfo*)A_userInfo;
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
    }else{
		if(callbackInfo->cancelFlag){
			*A_isLast = 1;
			*A_dataSize = 0;
	
			fclose(callbackInfo->fp);
			callbackInfo->fp = NULL;
			return  MC_NORMAL_COMPLETION;
		}
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

#include <sys/stat.h>
//-----------------------------------------------------------------------------
//
static MC_STATUS My_GetPixelOffsetCB(int m_messageID,unsigned long tag,void* userInfo,CALLBACK_TYPE CBtype, 
					 unsigned long* dataSizePtr,void** dataBufferPtr,int isFirst,int* isLastPtr)
{
	if(!userInfo)
		return MC_CALLBACK_CANNOT_COMPLY;
	CMyDicomFile* pMessage = (CMyDicomFile*) userInfo;

	pMessage->getMediaCBinfo().cancelFlag  = true;
	return (MC_STATUS) pMessage->ProcessGetPixelOffset(CBtype, dataSizePtr, dataBufferPtr);
}

int CMyDicomFile::LoadHeaderEx(const char* iFilePath, int iKeepAsFile)
{
	MC_STATUS status;
	int fileID;
	char			syntaxUID[65];
	TRANSFER_SYNTAX syntax;
  //  MyMediaCBinfo	mcbInfo;

	// don't need this - we have to do it on the fly (TC Zhao Jan 2002)
//	struct _stat statbuf;
	struct stat statbuf;
	
//	if (_stat(iFilePath, &statbuf) != 0)
	if (::stat(iFilePath, &statbuf) != 0)
	{
		return kInvalidArguments;
	}
	
	//	Avoid pixel-data callbacks
	if (!msgApplicationID)
	{
		status = MC_Register_Application(&msgApplicationID, "CPxDicomMessage");
		if (status != kNormalCompletion)
		{
			return (PxDicomStatus) status;
		}
	 
	}
	


    status = MC_Create_Empty_File(&fileID, iFilePath); 
    if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;
    }

    long offset = -1;

	m_myMediaCBinfo.init();
    status = MC_Open_File_Upto_Tag(msgApplicationID, fileID, &m_myMediaCBinfo, 
        MC_ATT_GROUP_7FE0_LENGTH, &offset, MyAqMediaToFileObj); 
    if (status != kNormalCompletion)
    {
        MC_Free_File(&fileID);
        return (PxDicomStatus) status;
    }


	status = MC_Get_Value_To_String(fileID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status; 
		}
	}

	if (!iKeepAsFile)
	{
		status = MC_File_To_Message(fileID); 
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status; 
		}

		status = MC_Set_Message_Transfer_Syntax(fileID, syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status;  
		}
	}

 

	MC_Free_Message(&m_messageID);
	m_messageID = fileID; 

	return kNormalCompletion;
}


/*
*  2011/04/05 再登録したパノラマーが表示できない
*  m_pixelOffset, m_dataSize  0 となっている。
*  それの対応版
*/
static int pixelOffsetApplicationID=0;
int CMyDicomFile::LoadHeaderEx(const char* iFilePath, unsigned long& oOffset, unsigned long& oSize)
{
	MC_STATUS status;
	char syntaxUID[65];
	TRANSFER_SYNTAX syntax;
	

	if (!pixelOffsetApplicationID)
	{
		status = MC_Register_Application(&pixelOffsetApplicationID, "PIXELOFFSET");
		if (status != MC_NORMAL_COMPLETION)
		{
			return (PxDicomStatus) status;
		}
	

	}
	status = MC_Register_Callback_Function(pixelOffsetApplicationID, MC_ATT_PIXEL_DATA, this, My_GetPixelOffsetCB);
	if (status != MC_NORMAL_COMPLETION)
	{
		return  (PxDicomStatus) status;
	}
	 
	int fileID = -1;
	status = MC_Create_Empty_File(&fileID, iFilePath);
	if (status != MC_NORMAL_COMPLETION)
    {
        return (PxDicomStatus) status;  
    }

//	MyMediaCBinfo	cbinfo;
	m_myMediaCBinfo.init();
	status = MC_Open_File_Bypass_OBOW(pixelOffsetApplicationID, fileID, &m_myMediaCBinfo, MyAqMediaToFileObj);
	
	if ((status != MC_NORMAL_COMPLETION)  && (m_myMediaCBinfo.cancelFlag==false))
    {
        return (PxDicomStatus) status;  
    }

	oOffset = m_pixelDataOffset;
	oSize = m_pixelDataSize;   

	status = MC_Get_Value_To_String(fileID, MC_ATT_TRANSFER_SYNTAX_UID, 65, syntaxUID);
	if (status == MC_NORMAL_COMPLETION)
	{
		status = MC_Get_Enum_From_Transfer_Syntax(syntaxUID, &syntax);
		if (status != MC_NORMAL_COMPLETION)
		{
			MC_Free_File(&fileID);
			return (PxDicomStatus) status; 
		}
	}

	status = MC_File_To_Message(fileID); 
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_File(&fileID);
		return (PxDicomStatus) status; 
	}


	status = MC_Set_Message_Transfer_Syntax(fileID, syntax);
	if (status != MC_NORMAL_COMPLETION)
	{
		MC_Free_File(&fileID);
		return (PxDicomStatus) status;  
	}
	m_transferSyntax = syntax;

	MC_Free_Message(&m_messageID);
	m_messageID = fileID; 

	return kNormalCompletion;
}

bool CMyDicomFile::initRegisterApp()
{
	MC_STATUS status;
	if (!msgApplicationID)
	{
		status = MC_Register_Application(&msgApplicationID, "VLIDicomMessage");
		if (status != kNormalCompletion)
		{
			return false;
		}
	}
	if (!pixelOffsetApplicationID)
	{
		status = MC_Register_Application(&pixelOffsetApplicationID, "PIXELOFFSET");
		if (status != MC_NORMAL_COMPLETION)
		{
			return false;
		}
	

	}
	return true;
}

bool CMyDicomFile::getSimpleDicomInfo(SimpleDicomInfo & dicomInfo)
{
 	
	std::string study_date_str = this->GetValue(  MC_ATT_STUDY_DATE );

	if(study_date_str.size()>=8){
		std::string::size_type index = study_date_str.find( "/" ); 
		
		if( index == std::string::npos )   
		{
			sscanf(study_date_str.c_str(),"%04d%2d%2d",&dicomInfo.m_studyYear, &dicomInfo.m_studyMon, &dicomInfo.m_studyDay);
		}else{
			sscanf(study_date_str.c_str(),"%d/%d/%d",&dicomInfo.m_studyYear, &dicomInfo.m_studyMon, &dicomInfo.m_studyDay);
		}
	}else{
		dicomInfo.m_studyYear	= 0;
		dicomInfo.m_studyMon	= 0;
		dicomInfo.m_studyDay	= 0;
	}

	//MC_ATT_STUDY_ID
	dicomInfo.m_studyUID	= GetValue(  MC_ATT_STUDY_ID );
	//MC_ATT_PATIENTS_NAME
	dicomInfo.m_patientName = GetValue(  MC_ATT_PATIENTS_NAME );
	//MC_ATT_PATIENT_ID
	dicomInfo.m_patientID	= GetValue(  MC_ATT_PATIENT_ID );
	//
	dicomInfo.m_seriesUID	= GetValue(  MC_ATT_SERIES_INSTANCE_UID );
	return true;
}