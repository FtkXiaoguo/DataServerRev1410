/***********************************************************************
 * $Id: RTVDiCOMService.cpp 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes RTVDiCOMService Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */


#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#include "PxDBData.h"
#include "RTVDiCOMService.h"

#include "AqCore/TRPlatform.h"
#if defined(_WIN32) && !defined (vsnprintf)
#define vsnprintf _vsnprintf
#endif

//-----------------------------------------------------------------------------

RTVDiCOMService::RTVDiCOMService(DiCOMConnectionInfo& connectInfo, int iMessageID):
	m_connectInfo(connectInfo), m_messageID(iMessageID), m_serviceName(0), m_command(0)
{
}

//-----------------------------------------------------------------------------

int RTVDiCOMService::SendResponseMessage(int resp_status, int type, const char* reason)
{
	//
	//	Open response message
	//
	int rspMsgID;
	int associationID = m_connectInfo.AssociationID;
	MC_STATUS status =  MC_Open_Empty_Message(&rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: (%d) - SendResponseMessage() - DcmLib error %d on MC_Open_Empty_Message() - for type = %d\n", associationID, status, type);
		return status;
	}

	status =  MC_Set_Service_Command(rspMsgID, m_serviceName, type);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: (%d) - SendResponseMessage() - DcmLib error %d on MC_Open_Message() - for type = %d\n", associationID, status, type);
		MC_Free_Message(&rspMsgID);
		return status;
	}

	if (reason)
	{
		MC_Set_Value_From_String(rspMsgID, MC_ATT_ERROR_COMMENT, reason);
	}

	//
	//	Send response message
	//
	status = MC_Send_Response_Message(associationID, resp_status, rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage( "ERROR: (%d) - SendResponseMessage() - DcmLib error %d on MC_Send_Response_Message() - for type = %d\n", associationID, status, type);
		MC_Free_Message(&rspMsgID);
		return status;
	}

	//
	//	Free response message
	//
	status = MC_Free_Message(&rspMsgID);
	if (status != MC_NORMAL_COMPLETION)
	{
		LogMessage(kWarning,"WARNING: SendResponseMessage() - DcmLib error %d on MC_Send_Response_Message() - for type = %d\n", associationID, status, type);
		return status;
	}
	return MC_NORMAL_COMPLETION;
}

//	-- - 01/08/03 - This overloads rtvbase so we can prepend AssocID and MessageID into all log messages.
#include "AqCore/TRPlatform.h"

void RTVDiCOMService::LogMessage(const char *fmt, ...)
{
	//	Prepend (AssocID, MsgID) to log message
	char mbuf[100];
	sprintf(mbuf, "(%d, %d): ", m_connectInfo.AssociationID, m_messageID);

	try
	{
		va_list args;
		va_start(args, fmt);
		GetAqLogger()->WriteLogMessage(fmt, args, mbuf);
	}
	catch (...)
	{
		GetAqLogger()->LogMessage("** ERROR in log format detected (%s)\n", fmt);
	}
}

void RTVDiCOMService::LogMessage(int iLevel, const char *fmt, ...)
{
	if (GetAqLogger()->GetLogLevel() < iLevel)
		return;

	//	Prepend (AssocID, MsgID) to log message
	char mbuf[100];
	sprintf(mbuf, "(%d, %d): ", m_connectInfo.AssociationID, m_messageID);

	try
	{
		va_list args;
		va_start(args, fmt);
		GetAqLogger()->WriteLogMessage(fmt, args, mbuf);
	}
	catch (...)
	{
		GetAqLogger()->LogMessage("** ERROR in log format detected (%s)\n", fmt);
	}
}

void RTVDiCOMService::FlushLog(void)
{
	GetAqLogger()->FlushLog();
}

//-------------------------------------------------------------------------
//
void RTVDiCOMService::SetServiceList(char* iServiceListName)
{
	ASTRNCPY(m_serviceListName, iServiceListName);
}
