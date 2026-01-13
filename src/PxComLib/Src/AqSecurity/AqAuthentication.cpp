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
#include "AqAuthentication.h"

#define SECURITY_WIN32
#include <Security.h>
#include <sspi.h>
#include <sddl.h>

#define SEC_SUCCESS(Status) ((Status) >= 0)
#define NT_DLL_NAME           "secur32.dll"

static TCHAR gPackageName[256];
bool  AqAuthentication::c_fUseConfidentiality = false;
bool  AqAuthentication::c_fUseIntegrity = false;

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#include <stdio.h>
#endif

#pragma comment( lib, "secur32.lib")
#pragma warning (disable: 4786)



// structure storing the state of the authentication sequence
//
struct AUTH_SEQ 
{
	AUTH_SEQ()
	{
		_fNewConversation = true;
		_fHaveCredHandle = false;
		_fHaveCtxtHandle = false;
	}

	~AUTH_SEQ()
	{
		if (_fHaveCtxtHandle)
			DeleteSecurityContext (&_hctxt), _fHaveCtxtHandle=false;
		
		if (_fHaveCredHandle)
			FreeCredentialsHandle (&_hcred), _fHaveCredHandle=false;
	}
	
	//states
    bool _fNewConversation;
    CredHandle _hcred;
    bool _fHaveCredHandle;
    bool _fHaveCtxtHandle;

	//class globle varaibles
	SECURITY_STATUS   _ss;
	TimeStamp         _Lifetime;
	SecBufferDesc     _OutBuffDesc;
	SecBuffer         _OutSecBuff;
	SecBufferDesc     _InBuffDesc;
	SecBuffer         _InSecBuff;
	ULONG             _ContextAttributes;

	// security handle information
    SecHandle  _hctxt;
    ULONG cbMaxSignature;
    ULONG cbSecurityTrailer;
};

typedef AUTH_SEQ *PAUTH_SEQ;

AqAuthentication::AqAuthentication() : m_pAuthStates(0), m_hLogonToken(0)
{
	InitSession();
}

AqAuthentication::~AqAuthentication() 
{
	TermSession ();
}

bool AqAuthentication::InitPackage (const char* iPackageName)
{
	if(stricmp(gPackageName, iPackageName) == 0)
		return true;
	
	strncpy(gPackageName, iPackageName, sizeof gPackageName);

	PSecPkgInfo pkgInfo;
	
	// Query for the package we're interested in
	m_pAuthStates->_ss = QuerySecurityPackageInfo (gPackageName, &pkgInfo);
	
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
#ifdef _DEBUG
		fprintf (stderr, "Couldn't query package info for %s, error 0x%08x\n", iPackageName, m_pAuthStates->_ss);
#endif
		gPackageName[0] = 0;
		return(false);
	}
	
	FreeContextBuffer (pkgInfo);
#ifdef _DEBUG	
	printf ("Using package: %s\n", gPackageName);
#endif	
	return true;
}

bool AqAuthentication::InitSession()
{
	if(m_pAuthStates)
		delete m_pAuthStates;

	m_pAuthStates = new AUTH_SEQ;
	if(m_hLogonToken > 0)
		CloseHandle(m_hLogonToken), m_hLogonToken=0;
	return(true);
}

bool AqAuthentication::TermSession ()
{
	if(m_pAuthStates)
		delete m_pAuthStates, m_pAuthStates=0;
	
	return(true);
}  


/*

  Routine Description:
  
	Optionally takes an input buffer coming from the server and returns
	a buffer of information to send back to the server.  Also returns
	an indication of whether or not the context is complete.
	
	  Return Value:
	  
		Returns true is successful; otherwise false is returned.
		
*/

bool AqAuthentication::GenContext (BYTE *inBuffers, DWORD nInBuffer,
							   BYTE **pOutBuffers, DWORD& nOutBuffer,
							   bool& oDone, ULONG& oAttribs, CHAR *iTarget)
							   
{
	PAUTH_SEQ         pAS;
	SecPkgContext_Sizes SecPkgContextSizes;
	SecPkgContext_NegotiationInfo SecPkgNegInfo;
		
	*pOutBuffers = 0;

	if(!m_pAuthStates)
		InitSession();
	pAS = m_pAuthStates;	
	
	if (pAS->_fNewConversation)  
	{
		if(!AcquireCredHandle())
			return false;
	}
	
	// prepare output buffer
	//
	pAS->_OutBuffDesc.ulVersion = 0;
	pAS->_OutBuffDesc.cBuffers = 1;
	pAS->_OutBuffDesc.pBuffers = &pAS->_OutSecBuff;
	
	pAS->_OutSecBuff.cbBuffer = 0;
	pAS->_OutSecBuff.BufferType = SECBUFFER_TOKEN;
	pAS->_OutSecBuff.pvBuffer = NULL;
	
	// prepare input buffer
	//
	pAS->_InBuffDesc.ulVersion = 0;
	pAS->_InBuffDesc.cBuffers = 1;
	pAS->_InBuffDesc.pBuffers = &pAS->_InSecBuff;
		
	pAS->_InSecBuff.cbBuffer = nInBuffer;
	pAS->_InSecBuff.BufferType = SECBUFFER_TOKEN;
	pAS->_InSecBuff.pvBuffer = inBuffers;
		
#ifdef DEBUG_PRINT
	if(g_fVerbose)
	{
		printf ("token buffer recieved(%lu bytes):\n", InSecBuff.cbBuffer);
		PrintHexDump (InSecBuff.cbBuffer, (PBYTE)InSecBuff.pvBuffer);
	}
#endif		
	
	// Always have the package allocate the memory
	oAttribs |= ISC_REQ_ALLOCATE_MEMORY;
	
	if(!FillFirstSecurityContext(oAttribs, iTarget))
		return false;

	pAS->_fHaveCtxtHandle = true;
	
	// Complete token -- if applicable
	//
	if ((SEC_I_COMPLETE_NEEDED == m_pAuthStates->_ss) || (SEC_I_COMPLETE_AND_CONTINUE == m_pAuthStates->_ss))  
	{
		m_pAuthStates->_ss = CompleteAuthToken (&pAS->_hctxt, &pAS->_OutBuffDesc);
		if (!SEC_SUCCESS(m_pAuthStates->_ss))  
		{
			//fprintf (stderr, "complete failed: 0x%08x\n", m_pAuthStates->_ss);
			return false;
		}
	}
	
	nOutBuffer = pAS->_OutSecBuff.cbBuffer;
	*pOutBuffers = (PBYTE)pAS->_OutSecBuff.pvBuffer;
	
	if (pAS->_fNewConversation)
		pAS->_fNewConversation = false;
	
	oDone = !((SEC_I_CONTINUE_NEEDED == m_pAuthStates->_ss) ||
		(SEC_I_COMPLETE_AND_CONTINUE == m_pAuthStates->_ss));
	
#ifdef DEBUG_PRINT
	if(g_fVerbose)
	{
		printf ("token buffer generated (%lu bytes):\n", OutSecBuff.cbBuffer);
		PrintHexDump (OutSecBuff.cbBuffer, (PBYTE)OutSecBuff.pvBuffer);
	}
#endif	
	
	if(oDone) 
	{
		
		// find size of signature block
		//
		m_pAuthStates->_ss = QueryContextAttributes(
			&pAS->_hctxt,
			SECPKG_ATTR_SIZES,
			&SecPkgContextSizes );
		
		if (!SEC_SUCCESS(m_pAuthStates->_ss))  
		{
			//fprintf (stderr, "QueryContextAttributes failed: 0x%08x\n", m_pAuthStates->_ss);
			//printf("here");
			if(pOutBuffers)
				FreeContextBuffer(pOutBuffers);
			return false;
		}
		
		// these values are used for encryption and signing
		//
		pAS->cbMaxSignature = SecPkgContextSizes.cbMaxSignature;
		pAS->cbSecurityTrailer = SecPkgContextSizes.cbSecurityTrailer;
		
		// return the attributes we ended up with
		//
		oAttribs = pAS->_ContextAttributes;
		
		// find out what package was negotiated
		//
		m_pAuthStates->_ss = QueryContextAttributes(
			&pAS->_hctxt,
			SECPKG_ATTR_NEGOTIATION_INFO,
			&SecPkgNegInfo );
		
		if (!SEC_SUCCESS(m_pAuthStates->_ss))  
		{
			//fprintf (stderr, "QueryContextAttributes failed: 0x%08x\n", m_pAuthStates->_ss);
			if(pOutBuffers)
				FreeContextBuffer(pOutBuffers);
			return false;
		}
		else
		{
			//printf("Package Name: %s\n", SecPkgNegInfo.PackageInfo->Name);
			// free up the allocated buffer
			// 
			FreeContextBuffer(SecPkgNegInfo.PackageInfo);
			
		}
#ifdef DEBUG_PRINT
		// find out what flags are really used
		//
		SecPkgContext_Flags SecPkgFlags;
		m_pAuthStates->_ss = QueryContextAttributes(
			&pAS->_hctxt,
			SECPKG_ATTR_FLAGS,
			&SecPkgFlags );
		
		if (!SEC_SUCCESS(m_pAuthStates->_ss))  
		{
			fprintf (stderr, "QueryContextAttributes failed: 0x%08x\n", m_pAuthStates->_ss);
			if(pOutBuffers)
				FreeContextBuffer(pOutBuffers);
			return false;
		}
		else
		{
			printf("FLags: %x\n", SecPkgFlags.Flags);
			
		}
#endif		
	}
	
	return true;
}


bool AqAuthentication::ImpersonateContext ()
/*++

  Routine Description:
  
    Impersonates the client whose context is associated with the
	supplied key.
	
	  Return Value:
	  
		Returns true is successful; otherwise false is returned.
		
		  --*/
{
	m_pAuthStates->_ss = ImpersonateSecurityContext (&m_pAuthStates->_hctxt);
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
#ifdef _DEBUG
		fprintf (stderr, "Impersonate failed: 0x%08x\n", m_pAuthStates->_ss);
#endif
		return(false);
	}
	
	return(true);
}


bool AqAuthentication::ImpersonateUser(const wchar_t* iUsername, const wchar_t* iDomain, const wchar_t* iPassword)
{
	InitSession();

	if(!LogonUserW( iUsername, iDomain, iPassword,
		LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_WINNT50, &m_hLogonToken ))
		return false;

	if(!ImpersonateLoggedOnUser(m_hLogonToken))
		return false;
	
	return true;
}

bool AqAuthentication::GetUserPrincipalName(AqUString& oUsername, AqUString& oDomain)
{
	ULONG msgSize=128;
	wchar_t* p = oUsername.GetBuffer(128);

	BOOL ret = GetUserNameExW(NameUserPrincipal, p, &msgSize);
	if(!ret)
	{
#ifdef _DEBUG
		printf("GetUserNameEx error: %x\n", GetLastError());
#endif
		return false;
	}


#ifdef _DEBUG
	printf("GetUserNameEx -> %S\n", oUsername);
#endif
	wchar_t* nameTail = wcschr(oUsername, L'@');
	if(nameTail == 0)
		return false;
	
	oDomain = (nameTail+1);
	*nameTail = 0;

	return(true);
}


bool AqAuthentication::GetUserFullName(AqUString& oFullname)
{
	ULONG msgSize=128;
	wchar_t* p = oFullname.GetBuffer(128);

	BOOL ret = GetUserNameExW(NameDisplay, p, &msgSize);
	if(!ret)
	{
#ifdef _DEBUG
		printf("GetUserNameEx error: %x\n", GetLastError());
#endif
		return false;
	}


#ifdef _DEBUG
	printf("GetUserNameEx -> %S\n", oFullname);
#endif

	return(true);
}

bool AqAuthentication::GetUserGroupsSID(std::vector< AqString >& oGroupsSID)
{
	oGroupsSID.clear();
	if( m_hLogonToken == 0)
	{
		m_pAuthStates->_ss = QuerySecurityContextToken(&m_pAuthStates->_hctxt, &m_hLogonToken);
		if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
		{
#ifdef _DEBUG
			fprintf (stderr, "Couldn't Get User Groups SID, error 0x%08x\n", m_pAuthStates->_ss);
#endif
			return(false);
		}
	}
	

	DWORD ReturnLength = 0; 
	//get size of required buffer
	GetTokenInformation(m_hLogonToken, TokenGroups, 0, 0, &ReturnLength);

	AqBuffer aBuffer;
	aBuffer.Allocate(ReturnLength, false);
	LPTSTR pBuffer = aBuffer.GetData();

	BOOL ret = GetTokenInformation(m_hLogonToken, TokenGroups, pBuffer, ReturnLength, &ReturnLength);
	if(!ret)
	{
#ifdef _DEBUG
		printf("GetTokenInformation error: %x\n", GetLastError());
#endif
		return false;
	}
	
	PTOKEN_GROUPS groupsSID = (TOKEN_GROUPS*)pBuffer;
	LPTSTR szSID=0;
	_SID* psid = 0;
	BOOL IsMember = 0;
#ifdef DEBUG_PRINT
	for(int i=0; i<groupsSID->GroupCount; i++)
	{
		psid = (_SID*)groupsSID->Groups[i].Sid;
		ret =  CheckTokenMembership(m_hLogonToken, psid, &IsMember);
		if(ret && IsMember&& ConvertSidToStringSid(psid, &szSID))
		{
			oGroupsSID.push_back(szSID);
			LocalFree(szSID);
		}
		
	}
#else	
	char buf[128];
	for(int i=0; i<groupsSID->GroupCount; i++)
	{
		psid = (_SID*)groupsSID->Groups[i].Sid;
		ret =  CheckTokenMembership(m_hLogonToken, psid, &IsMember);
		if(ret && IsMember)
		{
			AqBinToHex(psid, GetLengthSid(psid), buf);
			oGroupsSID.push_back(buf);
		}
		
	}
#endif

#ifdef DEBUG_PRINT
	
	if(pBuffer)
	{
		PTOKEN_GROUPS groupsSID = (TOKEN_GROUPS*)pBuffer;
		 LPTSTR szSID=0;
		 _SID* psid = 0;
		 BOOL IsMember = 0;
		 char buf[128];
		for(int i=0; i<groupsSID->GroupCount; i++)
		{
			psid = (_SID*)groupsSID->Groups[i].Sid;
			ret = ConvertSidToStringSid(psid, &szSID);
			//LookupAccountSid
			if(!ret)
			{
				printf("ConvertSidToStringSid error: %x\n", GetLastError());
				continue;
			}
			ret =  CheckTokenMembership(m_hLogonToken, psid, &IsMember);
			if(!ret)
				printf("CheckTokenMembership failed on %s\n", szSID);
			else
				printf("#%d group enable:%d; flag:0x%08X; SID : %s\n", i, IsMember, 
					groupsSID->Groups[i].Attributes, szSID);
			
			
			printf("#%d group enable:%d; flag:0x%08X; Hex: %s\n", i, IsMember, 
					groupsSID->Groups[i].Attributes, AqBinToHex(psid, GetLengthSid(psid), buf));
			
			//psid
			LocalFree(szSID);

		}

	//ret = GetTokenInformation(m_hLogonToken, TokenPrimaryGroup, pBuffer, ioMsgSize, &ReturnLength);
	//TOKEN_PRIMARY_GROUP* pPrimaryGroup = (TOKEN_PRIMARY_GROUP*)pBuffer;
	//psid = (_SID*)pPrimaryGroup->PrimaryGroup;
	//ret = ConvertSidToStringSid(psid, &szSID);
	//printf("PrimaryGroup sid -> %s\n", szSID);
		
	}
#endif
//CheckTokenMembership();

	return(true);

}


bool AqAuthentication::RevertContext ()
/*++

  Routine Description:
  
    Reverts to the original server context.
	
	  Return Value:
	  
		Returns true is successful; otherwise false is returned.
		
		  --*/
{
	if(!m_pAuthStates->_fHaveCtxtHandle && m_hLogonToken)
		return RevertToSelf()?true:false;

	m_pAuthStates->_ss = RevertSecurityContext (&m_pAuthStates->_hctxt);
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
		//fprintf (stderr, "Revert failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	
	return(true);
}

bool AqAuthentication::EncryptThis (PBYTE ipMsg, ULONG iMsgSize, PBYTE& pOutput, ULONG& outputSize)
				  /*++
				  
					Routine Description:
					
					  Encrypts a message
					  
						Return Value:
						
						  Returns true is successful; otherwise false is returned.
						  
							--*/
{
	PAUTH_SEQ         pAS = m_pAuthStates;
	SecBufferDesc     BuffDesc;
	SecBuffer         SecBuff[2];
	ULONG             ulQop = 0;
	ULONG             SigBufferSize;
	
	
	
	// the size of the trailer (signature + padding) block is 
	// determined from this value
	//
	SigBufferSize = pAS->cbSecurityTrailer;
	
	//if(g_fVerbose)
	//	printf ("data before encryption: %s\n", pMessage);
	
	// allocate a buffer big enough to hold the 
	// signature + encrypted data + a dword that 
	// will tell the other side how big the trailer block is
	//
	pOutput = (PBYTE) malloc (SigBufferSize + iMsgSize + sizeof(DWORD));
	
	// prepare buffers
	//
	BuffDesc.ulVersion = 0;
	BuffDesc.cBuffers = 2;
	BuffDesc.pBuffers = SecBuff;
	
	SecBuff[0].cbBuffer = SigBufferSize;
	SecBuff[0].BufferType = SECBUFFER_TOKEN;
	SecBuff[0].pvBuffer = pOutput + sizeof(DWORD);
	
	SecBuff[1].cbBuffer = iMsgSize;
	SecBuff[1].BufferType = SECBUFFER_DATA;
	SecBuff[1].pvBuffer = ipMsg;
	
	
	m_pAuthStates->_ss = EncryptMessage(&pAS->_hctxt, ulQop, &BuffDesc,	0);
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
		//fprintf (stderr, "EncryptMessage failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	
	// indicate in the first DWORD of our output buffer how big 
	// the trailer block is
	//
	*((DWORD *) pOutput) = SecBuff[0].cbBuffer;
	
	// Here we append the encrypted data to our trailer block
	// to form a single block. And yes, it's confusing that we put the trailer
	// in the beginning of our data to send to the client, but it worked out 
	// better that way.
	//
	memcpy (pOutput+SecBuff[0].cbBuffer+sizeof(DWORD), ipMsg, iMsgSize);
	
	outputSize = iMsgSize + SecBuff[0].cbBuffer + sizeof(DWORD);
	
	//if(g_fVerbose)
	//{
	//	printf ("data after encryption including trailer (%lu bytes):\n", *pcbOutput);
	//	PrintHexDump (*pcbOutput, *ppOutput);
	//}
	
	return true;
	
}


PBYTE AqAuthentication::DecryptThis(PBYTE pBuffer, ULONG& ioMsgSize)
/*++

Routine Description:

  Decrypts a message
  
	Return Value:
	
	  Returns true is successful; otherwise false is returned.
	  
		--*/
{
	PAUTH_SEQ         pAS = m_pAuthStates;
	SecBufferDesc     BuffDesc;
	SecBuffer         SecBuff[2];
	ULONG             ulQop = 0;
	PBYTE             pSigBuffer;
	PBYTE             pDataBuffer;
	DWORD             SigBufferSize;
	
	
	// When the server encrypted the message it set the size
	// of the trailer block to be just what it needed. We have to
	// tell DecryptMessage how big the trailer block turned out to be,
	// so we sent the size of the trailer as the first dword of the 
	// message. 
	//
	SigBufferSize = *((DWORD *) pBuffer);
	
	//if(g_fVerbose)
	//{
	//	printf ("data before decryption including trailer (%lu bytes):\n", *pcbMessage);
	//	PrintHexDump (*pcbMessage, (PBYTE) pBuffer);
	//}
	
	// we know the trailer is at the beginning of the blob
	// but after the trailer size dword
	//
	pSigBuffer = pBuffer + sizeof(DWORD);
	
	// and that the data is after the trailer
	//
	pDataBuffer = pSigBuffer + SigBufferSize;
	
	// reset the size of the data to reflect just the encrypted blob
	//
	ioMsgSize = ioMsgSize- SigBufferSize - sizeof(DWORD);
	
	// prepare buffer
	//
	BuffDesc.ulVersion = 0;
	BuffDesc.cBuffers = 2;
	BuffDesc.pBuffers = SecBuff;
	
	SecBuff[0].cbBuffer = SigBufferSize;
	SecBuff[0].BufferType = SECBUFFER_TOKEN;
	SecBuff[0].pvBuffer = pSigBuffer;
	
	SecBuff[1].cbBuffer = ioMsgSize;
	SecBuff[1].BufferType = SECBUFFER_DATA;
	SecBuff[1].pvBuffer = pDataBuffer;
	
	m_pAuthStates->_ss = DecryptMessage(
		&pAS->_hctxt,
		&BuffDesc,
		0,
		&ulQop
		);
	
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
		//fprintf (stderr, "DecryptMessage failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	
	// only return a pointer to the data which we decrypted - 
	// discard the trailer data
	return pDataBuffer;
	
}

bool AqAuthentication::SignThis (PBYTE ipMsg, ULONG iMsgSize, PBYTE& pOutput, ULONG& outputSize)
/*++

 Routine Description:
 
   Signs a message
   
	 Return Value:
	 
	   Returns true is successful; otherwise false is returned.
	   
		 --*/
{
	PAUTH_SEQ         pAS = m_pAuthStates;
	SecBufferDesc     BuffDesc;
	SecBuffer         SecBuff[2];
	ULONG             ulQop = 0;
	PBYTE             pSigBuffer;
	DWORD             SigBufferSize;
	
	
	// the size of the signature block is determined from this value
	//
	SigBufferSize = pAS->cbMaxSignature;
	
	// for reasons apparent later, we are going to allocate
	// a buffer big enough to hold the signature + signed data
	pSigBuffer = (PBYTE) malloc (SigBufferSize + iMsgSize);
	
	// prepare buffers
	//
	BuffDesc.ulVersion = 0;
	BuffDesc.cBuffers = 2;
	BuffDesc.pBuffers = SecBuff;
	
	SecBuff[0].cbBuffer = SigBufferSize;
	SecBuff[0].BufferType = SECBUFFER_TOKEN;
	SecBuff[0].pvBuffer = pSigBuffer;
	
	SecBuff[1].cbBuffer = iMsgSize;
	SecBuff[1].BufferType = SECBUFFER_DATA;
	SecBuff[1].pvBuffer = ipMsg;
	
	m_pAuthStates->_ss = MakeSignature(
		&pAS->_hctxt,
		ulQop,
		&BuffDesc,
		0
		);
	
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
		//fprintf (stderr, "MakeSignature failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	
	// here we append the signed data to our signature block
	// and also reducing the size of our signature buffer if
	// the package did not use the size that we provided
	//
	memcpy (pSigBuffer+SecBuff[0].cbBuffer, ipMsg, iMsgSize);
	
	// point the data buffer to our new blob and reset the size
	//
	pOutput = pSigBuffer;
	
	outputSize = iMsgSize + SecBuff[0].cbBuffer;
	
	//if(g_fVerbose)
	//{
	//	printf ("data after signing including signature (%lu bytes):\n", *pcbOutput);
	//	PrintHexDump (*pcbOutput, *ppOutput);
	//}
	
	return true;
	
}


PBYTE AqAuthentication::VerifyThis(PBYTE pBuffer, ULONG& ioMsgSize)
/*++

Routine Description:

 Decrypts a message
 
   Return Value:
   
	 Returns true is successful; otherwise false is returned.
	 
	   --*/
{
	PAUTH_SEQ         pAS = m_pAuthStates;
	SecBufferDesc     BuffDesc;
	SecBuffer         SecBuff[2];
	ULONG             ulQop = 0;
	PBYTE             pSigBuffer;
	PBYTE             pDataBuffer;
	DWORD             SigBufferSize;
	
	
	// the size of the signature block is determined from this value
	//
	SigBufferSize = pAS->cbMaxSignature;
	
	//if(g_fVerbose)
	//{
	//	printf ("data before verifying (including signature):\n");
	//	PrintHexDump (*pcbMessage, pBuffer);
	//}

	// we know the signature is at the beginning of the blob
	//
	pSigBuffer = pBuffer;
	
	// and that the data is after the signature
	//
	pDataBuffer = pBuffer + SigBufferSize;
	
	// reset the size of the data to reflect just the data
	//
	ioMsgSize = ioMsgSize - (SigBufferSize);
	
	// prepare buffer
	//
	BuffDesc.ulVersion = 0;
	BuffDesc.cBuffers = 2;
	BuffDesc.pBuffers = SecBuff;
	
	SecBuff[0].cbBuffer = SigBufferSize;
	SecBuff[0].BufferType = SECBUFFER_TOKEN;
	SecBuff[0].pvBuffer = pSigBuffer;
	
	SecBuff[1].cbBuffer = ioMsgSize;
	SecBuff[1].BufferType = SECBUFFER_DATA;
	SecBuff[1].pvBuffer = pDataBuffer;
	
	m_pAuthStates->_ss = VerifySignature(
		&pAS->_hctxt,
		&BuffDesc,
		0,
		&ulQop
		);
	
	if (!SEC_SUCCESS(m_pAuthStates->_ss)) 
	{
		//fprintf (stderr, "VerifyMessage failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	
	return pDataBuffer;
	
}

//AqAuthClient functions

bool AqAuthClient::AcquireCredHandle()
{
	PAUTH_SEQ         pAS;
	pAS = m_pAuthStates;	

	m_pAuthStates->_ss = AcquireCredentialsHandle (
		NULL, // principal
		gPackageName,
		SECPKG_CRED_OUTBOUND,
		NULL, // LOGON id
		NULL, // auth data
		NULL, // get key fn
		NULL, // get key arg
		&pAS->_hcred,
		&pAS->_Lifetime
		);
	if (SEC_SUCCESS (m_pAuthStates->_ss))
		pAS->_fHaveCredHandle = true;
	else 
	{
		//fprintf (stderr, "AcquireCreds failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	return true;
}

bool AqAuthClient::FillFirstSecurityContext(ULONG& oAttribs, CHAR *iTarget)
{
	PAUTH_SEQ         pAS;
	pAS = m_pAuthStates;	
		
	
	// Always have the package allocate the memory
	oAttribs |= ISC_REQ_ALLOCATE_MEMORY;
	
	m_pAuthStates->_ss = InitializeSecurityContext (
		&pAS->_hcred,
		pAS->_fNewConversation ? 0:&pAS->_hctxt,
		iTarget,
		oAttribs, 
		0, // reserved1
		SECURITY_NATIVE_DREP,
		pAS->_fNewConversation ? 0:&pAS->_InBuffDesc,
		0, // reserved2
		&pAS->_hctxt,
		&pAS->_OutBuffDesc,
		&pAS->_ContextAttributes,
		&pAS->_Lifetime
		);
	
	if (!SEC_SUCCESS (m_pAuthStates->_ss))  
	{
		//fprintf (stderr, "InitializeSecurityContext failed: 0x%08x\n", m_pAuthStates->_ss);
		return false;
	}
	return true;
}


bool AqAuthClient::Authenticate(TCHAR *iTarget)
/*++

  Routine Description:
  
    Manges the authentication conversation with the server via the
    supplied socket handle.
	
	  Return Value:
	  
		Returns true is successful; otherwise false is returned.
		
		  --*/
{
	bool stat, done;
	DWORD cbOut = 0;
	DWORD cbIn = 0;
	ULONG attrib = 0;
	PBYTE pOutBuf = NULL;
	PBYTE pInBuf = NULL;
	
	
	if (c_fUseConfidentiality)
		attrib |= ISC_REQ_CONFIDENTIALITY;
	
	if (c_fUseIntegrity)
		attrib |= ISC_REQ_INTEGRITY;
	
	cbOut = 0;
	
	if (!GenContext (NULL, 0, &pOutBuf, cbOut, done, attrib, iTarget) || pOutBuf == 0)
		return(false);
	
	stat = SendMsg (pOutBuf, cbOut);
	
	FreeContextBuffer(pOutBuf); // done with package allocated buffer - free it

	if (!stat)
		return(false);
		
	done = false;
	while (!done) 
	{
		if (!ReceiveMsg (pInBuf, cbIn))
			return(false);
		
		cbOut = 0;
		pOutBuf = 0;
		if (!GenContext (pInBuf, cbIn, &pOutBuf, cbOut, done, attrib, iTarget))
			return(false);
		
		stat = SendMsg (pOutBuf, cbOut);
		FreeContextBuffer(pOutBuf); // done with package allocated buffer - free it
		if (!stat)
			return(false);
	}
	
	//if(g_fVerbose)
	//	printf("Context Attributes = 0x%08x\n", attrib);
	
	// check the context flags and make sure we got what we want
	//
	if (c_fUseConfidentiality && !(attrib & ISC_RET_CONFIDENTIALITY))
	{
		//fprintf (stderr, "** confidentiality flags not set on context\n");
		//
		// the application can decide to either terminate the conversation or continue.
		// we are going to let the client make these decisions and continue
		//
		c_fUseConfidentiality = false;
	}
	
	if (c_fUseIntegrity && !(attrib & ISC_REQ_INTEGRITY))
	{
		//fprintf (stderr, " ** integrity flags not set on context\n");
		//
		// the application can decide to either terminate the conversation or continue.
		// we are going to let the client make these decisions and continue
		//
		c_fUseIntegrity = false;
	}
	
	return(true);
}

//AqAuthServer functions

//!!!DsWriteAccountSpn

bool AqAuthServer::AcquireCredHandle()
{
	PAUTH_SEQ         pAS;
	pAS = m_pAuthStates;	

	m_pAuthStates->_ss = AcquireCredentialsHandle (
		NULL, // principal
		gPackageName,
		SECPKG_CRED_INBOUND,
		NULL, // LOGON id
		NULL, // auth data
		NULL, // get key fn
		NULL, // get key arg
		&pAS->_hcred,
		&pAS->_Lifetime
		);
	if (SEC_SUCCESS (m_pAuthStates->_ss))
		pAS->_fHaveCredHandle = true;
	else 
	{
		//fprintf (stderr, "AcquireCreds failed: 0x%08x\n", m_pAuthStates->_ss);
		return(false);
	}
	return true;
}

bool AqAuthServer::FillFirstSecurityContext(ULONG& oAttribs, CHAR *iTarget)
							   
					   /*++
					   
						 Routine Description:
						 
						   Takes an input buffer coming from the client and returns a buffer
						   to be sent to the client.  Also returns an indication of whether or
						   not the context is complete.
						   
							 Return Value:
							 
							   Returns true is successful; otherwise false is returned.
							   
								 --*/
{
	PAUTH_SEQ         pAS;
	pAS = m_pAuthStates;	
		
	// Always have the package allocate the memory
	oAttribs |= ISC_REQ_ALLOCATE_MEMORY;
	
	m_pAuthStates->_ss = AcceptSecurityContext (
		&pAS->_hcred,
		pAS->_fNewConversation ? 0:&pAS->_hctxt,
		&pAS->_InBuffDesc,
		oAttribs, // context requirements
		SECURITY_NATIVE_DREP,
		&pAS->_hctxt,
		&pAS->_OutBuffDesc,
		&pAS->_ContextAttributes,
		&pAS->_Lifetime
		);

	if (!SEC_SUCCESS (m_pAuthStates->_ss))  
	{
		//fprintf (stderr, "AcceptSecurityContext failed: 0x%08x\n", m_pAuthStates->_ss);
		return false;
	}
	
	return true;


}

bool AqAuthServer::Authenticate(TCHAR *iTarget)
/*++

 Routine Description:

    Manages the authentication conversation with the client via the
    supplied socket handle.

 Return Value:

    Returns true is successful; otherwise false is returned.

--*/
{
	bool stat, done;
   	DWORD cbOut = 0;
	DWORD cbIn = 0;
	ULONG attrib = 0;
	PBYTE pOutBuf = NULL;
	PBYTE pInBuf = NULL;
	
	if (c_fUseConfidentiality)
		attrib |= ASC_REQ_CONFIDENTIALITY;
	
	if (c_fUseIntegrity)
		attrib |= ASC_REQ_INTEGRITY;
	
	do 
	{
		if (!ReceiveMsg(pInBuf, cbIn))
			return(false);
		
		pOutBuf = 0;
		if (!GenContext (pInBuf, cbIn, &pOutBuf, cbOut, done, attrib, iTarget))
			return(false);
		
		stat = SendMsg (pOutBuf, cbOut);
		
		FreeContextBuffer(pOutBuf); // done with package allocated buffer - free it
		
		if (!stat)
			return(false);
		
	} while(!done);
	
	//if(g_fVerbose)
	//	printf("Context Attributes = 0x%08x\n", attrib);
	
	// check the context flags and see if we got what we want
	//
	if (c_fUseConfidentiality && !(attrib & ASC_RET_CONFIDENTIALITY))
	{
		//fprintf (stderr, "** confidentiality flags not set on context\n");
		
		// The application can decide to either terminate the conversation 
		// or continue. We are going to let the client make these decisions 
		// and continue.
		//
		c_fUseConfidentiality = false;
	}
	
	if (c_fUseIntegrity && !(attrib & ASC_RET_INTEGRITY))
	{
		//fprintf (stderr, " ** integrity flags not set on context\n");
		
		// The application can decide to either terminate the conversation 
		// or continue. We are going to let the client make these decisions 
		// and continue.
		//
		c_fUseIntegrity = false;
	}
	
	// or the client could have requested the context flags and the package obliged
	//
	c_fUseConfidentiality = ((attrib & ASC_RET_CONFIDENTIALITY) != 0);
	
	c_fUseIntegrity = ((attrib & ASC_RET_INTEGRITY) != 0);
	
	
	return(true);
}  


#if 0

#include <Ntdsapi.h>
#include <DsGetDC.h>

// SpnRegister
// Register or unregister the SPNs under the service's account.
//
// The pszServiceAcctDN parameter is the distinguished name of the
// logon account for this instance of the service. 
//
// If the service runs in LocalSystem account, pszServiceAcctDN is the 
// distinguished name of the local computer account.
DWORD
SpnRegister(
			TCHAR *pszServiceAcctDN,    // DN of the service's logon account
			TCHAR **pspn,               // Array of SPNs to register
			unsigned long ulSpn,        // Number of SPNs in array
			DS_SPN_WRITE_OP Operation)  // Add, replace, or delete SPNs
{
	
	DWORD dwStatus;
	HANDLE hDs;
	//TCHAR szSamName[512];
	//unsigned short szSamName[512];
	char szSamName[512];
	DWORD dwSize = sizeof(szSamName);
	WCHAR *pWhack = NULL;
	PDOMAIN_CONTROLLER_INFO pDcInfo;

#ifdef DEBUG_PRINT	
	_tprintf(TEXT("SPN is:%s\n"), pspn[0]);
	if (Operation == DS_SPN_ADD_SPN_OP)
		_tprintf(TEXT("SPN will be set for %s\n"), pszServiceAcctDN);
	else
		_tprintf(TEXT("SPN will be removed from %s\n"), pszServiceAcctDN);
#endif
	
	// Bind to a domain controller. 
	// Get the domain for the current user.
	if ( GetUserNameEx( NameSamCompatible, szSamName, &dwSize ) )    
	{
		pWhack = wcschr( szSamName, L'\\' );
		if ( pWhack )
			*pWhack = L'\0';
	} else 
	{
		_tprintf(TEXT("GetUserNameEx failed - %d\n"), GetLastError());
		return GetLastError() ;
	}
	
	// Get the name of a domain controller in that domain.
	dwStatus = DsGetDcName(
		NULL,
		szSamName,
		NULL,
		NULL,
		DS_IS_FLAT_NAME |
		DS_RETURN_DNS_NAME |
		DS_DIRECTORY_SERVICE_REQUIRED,
		&pDcInfo );
	if ( dwStatus != 0 ) 
	{
		_tprintf(TEXT("DsGetDcName failed - %d\n"), dwStatus);
		return dwStatus;
	}
	
	// Bind to the domain controller.
	dwStatus = DsBind( pDcInfo->DomainControllerName, NULL, &hDs );
	
	// Free the DOMAIN_CONTROLLER_INFO buffer.
	NetApiBufferFree( pDcInfo );
	if ( dwStatus != 0 ) 
	{
		_tprintf(TEXT("DsBind failed - %d\n"), dwStatus);
		return dwStatus;
	}
	
	// Write the SPNs to the service account or computer account.
	dwStatus = DsWriteAccountSpn(
        hDs,         // handle to the directory
        Operation,   // Add or remove SPN from account's existing SPNs
        pszServiceAcctDN,        // DN of service account or computer account
        ulSpn,                   // Number of SPNs to add
        (const TCHAR **)pspn);   // Array of SPNs
	if (dwStatus != NO_ERROR) 
		_tprintf(TEXT("Failed to write SPN: Error was %X\n"),dwStatus);
	
	// Unbind the DS in any case.
	DsUnBind(&hDs);
	
	return(dwStatus);
}
#endif