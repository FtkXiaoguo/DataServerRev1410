/***********************************************************************
 * RTVDiCOMService.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2002, All rights reserved.
 *
 *	PURPOSE:
 *		Processes RTVDiCOMService Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef RTVDiCOMSerive_H
#define RTVDiCOMSerive_H

//-----------------------------------------------------------------------------
#include "RTVThread.h"
#include "AqCore/TRLogger.h"

struct DiCOMConnectionInfo 
{
	DiCOMConnectionInfo() {Clear();};
	~DiCOMConnectionInfo(){Clear();};
	void Clear() {memset(this, 0, sizeof(*this));};

    int		ApplicationID;
	int		AssociationID;
	char    RemoteApplicationTitle[20];      // 16-characters max 
    char    RemoteHostName[66];              // Network node name 64-characters max
    int     Tcp_socket;                      // TCP Socket used for association
    char    RemoteIPAddress[66];             // Network IP Address 
    char    LocalApplicationTitle[20];       // 16-characters max
	int		RemoteAEAqObjectID;				 //	used for audit trail
	int		LocalAEAqObjectID;				 //	used for audit trail
};



//-----------------------------------------------------------------------------
class RTVDiCOMService : public iRTVThreadProcess
{
public:
	
	RTVDiCOMService (DiCOMConnectionInfo& connectInfo, int iMessageID);
	virtual ~RTVDiCOMService() {};
	
	virtual void LogProcessStatus(void) = 0;
	DiCOMConnectionInfo* ConnectInfo() { return &m_connectInfo;};
	int GetMessageID(void) const { return m_messageID; }

	// custom log functions
	virtual void LogMessage(const char *fmt, ...);
	virtual void LogMessage(int iLevel, const char *fmt, ...);
	virtual void FlushLog(void);

protected:
	int SendResponseMessage(int resp_status, int type, const char* reason = 0);
	void SetServiceList(char* iServiceListName);
	
	DiCOMConnectionInfo m_connectInfo;
	int m_messageID;
	char* m_serviceName;
	unsigned short	m_command;
	char m_serviceListName[64];
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // RTVDiCOMSerive_H
