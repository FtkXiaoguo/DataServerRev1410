/***********************************************************************
 *---------------------------------------------------------------------
 *		Copyright(c), TeraRecon, Inc., 2001-2002 All rights reserved.
 *
 *	PURPOSE:		 
 *      API for window user authentication over socket
 *
 *	AUTHOR(S):  Gang Li	12-29-2005
 *   
 *-------------------------------------------------------------------
 */

#ifndef AQAUTHENTICATION_H_
#define AQAUTHENTICATION_H_

#include "AqCore/AqString.h"
#include <vector>

struct AUTH_SEQ;

class AqAuthentication
{
public:
	static bool  c_fUseConfidentiality;
	static bool  c_fUseIntegrity;

	AqAuthentication();
	virtual ~AqAuthentication();
	

	bool InitPackage (const char* iPackageName);
	bool InitSession ();
	virtual bool Authenticate(TCHAR *iTarget=0) = 0;

	bool ImpersonateContext();
	bool ImpersonateUser(const wchar_t* iUsername, const wchar_t* iDomain, const wchar_t* iPassword);
	bool GetUserPrincipalName(AqUString& oUsername, AqUString& oDomain);
	bool GetUserFullName(AqUString& oFullname);
	bool GetUserGroupsSID(std::vector< AqString >& oGroupsSID);
	bool RevertContext();
	

	bool EncryptThis (PBYTE ipMsg, ULONG iMsgSize, PBYTE& pOutput, ULONG& outputSize);
	PBYTE DecryptThis(PBYTE pBuffer, ULONG& ioMsgSize);
	bool SignThis(PBYTE ipMsg, ULONG iMsgSize, PBYTE& pOutput, ULONG& outputSize);
	PBYTE VerifyThis(PBYTE pBuffer, ULONG& ioMsgSize);

	
	
	bool TermSession ();

protected:
	virtual bool GenContext (BYTE *inBuffers, DWORD nInBuffer,
							 BYTE **pOutBuffers, DWORD& nOutBuffer,
							 bool& oDone, ULONG& oAttribs, CHAR *iTarget=0);

	virtual bool AcquireCredHandle() = 0;
	virtual bool FillFirstSecurityContext(ULONG& oAttribs, CHAR *iTarget) = 0;
	virtual bool SendMsg (PBYTE iBuf, DWORD iBufSize) = 0;
	virtual bool ReceiveMsg (PBYTE& oBuf, DWORD &oReadByte) = 0;

	AUTH_SEQ* m_pAuthStates;
	HANDLE m_hLogonToken;
};


class AqAuthClient : public AqAuthentication
{
public:

	AqAuthClient() {};
	virtual ~AqAuthClient() {};
	
	bool Authenticate(TCHAR *iTarget=0);

protected:
	bool AcquireCredHandle();
	bool FillFirstSecurityContext(ULONG& oAttribs, CHAR *iTarget);

};

class AqAuthServer : public AqAuthentication
{
public:

	AqAuthServer() {};
	virtual ~AqAuthServer() {};
	
	bool Authenticate(TCHAR *iTarget=0);

protected:
	bool AcquireCredHandle();
	bool FillFirstSecurityContext(ULONG& oAttribs, CHAR *iTarget);

};


#endif //AQAUTHENTICATION_H_