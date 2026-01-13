/**************************************************************************
 * TRCryptor.cpp
 *
 **************************************************************************/
#include "TRCryptor.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <wincrypt.h>
#undef _WIN32_WINNT
#endif


#include "AqCore.h"
#include "TRPlatform.h"
#include <algorithm>

#ifdef _DEBUG
#include <stdio.h>
#endif

// defaults - MD5&RC4 should be good enough
static char* sContainerName = "nvrServerCipher";
static char* sProvider		= MS_DEF_PROV;
static DWORD sProviderType	= PROV_RSA_FULL;
static ALG_ID sAlgoID		= CALG_RC4;
static ALG_ID sHashAlgo		= CALG_MD5;

//--------------------------------------------------------------------------------------
static void inline ReportError(const char*what)
{
	char msg[128];
	TRPlatform::GetSysErrorText(msg,sizeof msg);
	GetAqLogger()->LogMessage("ERROR %s: %s\n", what, msg);
#ifdef _DEBUG
	fprintf(stderr,"%s: %s\n", what, msg);
#endif;
}


class TRCryptorInternal
{
public:
	TRCryptorInternal(void)
	{
		if (!CryptAcquireContext( &m_CryptProv,sContainerName, sProvider ,sProviderType,CRYPT_MACHINE_KEYSET ))
		{
			ReportError("AcquireContext-Implemenation");
			m_CryptProv = 0;
		}
		
		m_hashObject = 0;
		m_key = 0;
	}
	
	~TRCryptorInternal(void)
	{
		Cleanup();
		if (m_CryptProv)
			CryptReleaseContext(m_CryptProv,0);
	}
	
	void Cleanup(void)
	{
		if (m_hashObject)
			CryptDestroyHash(m_hashObject);

		m_hashObject = 0;
		
		if (m_key)
			CryptDestroyKey(m_key);

		m_key = 0;
	}
	
	HCRYPTPROV m_CryptProv;
	HCRYPTHASH m_hashObject; 
	HCRYPTKEY  m_key;
};


//--------------------------------------------------------------------------------------
TRCryptor::TRCryptor()
{
	HCRYPTPROV hCryptProv = 0; 
	static int initialized = 0;

	m_endAddData = false;
	m_addedDataCount = 0;
	
	if (!initialized)
	{
#if 1
		if (!CryptAcquireContext( &hCryptProv,sContainerName, sProvider ,sProviderType,CRYPT_MACHINE_KEYSET ))
		{
			hCryptProv = 0; 
			if (GetLastError() == NTE_BAD_KEYSET)
			{
				if (!CryptAcquireContext( &hCryptProv,sContainerName, sProvider ,sProviderType,CRYPT_NEWKEYSET|CRYPT_MACHINE_KEYSET))
				{
					if (!hCryptProv && NTE_EXISTS != GetLastError())
					{
						ReportError("AcquireContext-Create");
					}
					hCryptProv = 0; 
				}
				else
				{
					// make the key accessable to every one
					// declare and initialize a security descriptor
					SECURITY_DESCRIPTOR SD;
					if ( InitializeSecurityDescriptor( &SD, SECURITY_DESCRIPTOR_REVISION ) )
					{
						// give the security descriptor a Null Dacl
						// done using the  "TRUE, (PACL)NULL" here
						if ( SetSecurityDescriptorDacl( &SD,TRUE, (PACL)NULL, FALSE ) )
						{
							CryptSetProvParam(hCryptProv, PP_KEYSET_SEC_DESCR, (BYTE*)&SD, DACL_SECURITY_INFORMATION);
						}
					}
				}
			}
		}
			
#else
		if (CryptAcquireContext( &hCryptProv,sContainerName, sProvider ,sProviderType,0) && hCryptProv)
		{	
			;
		}
		// make sure we have the key container
		else if (!CryptAcquireContext( &hCryptProv,sContainerName, sProvider ,sProviderType,CRYPT_NEWKEYSET))
		{
			if (!hCryptProv && NTE_EXISTS != GetLastError())
			{
				ReportError("AcquireContext-Create");
			}
		}
#endif
		
		if (hCryptProv)
		{
			CryptReleaseContext(hCryptProv,0);
			initialized = 1;
		}
	}
	
	m_implementation = new TRCryptorInternal;
}

//--------------------------------------------------------------------------------------
TRCryptor::~TRCryptor() 
{
	if (m_implementation)
		delete (TRCryptorInternal*)m_implementation;
}

// weed out chars DB does not like
static inline bool IsWeirdChar(unsigned int c) 
{
 	return (c=='\'' || c=='"' || c=='`' || c=='\\');
}

//--------------------------------------------------------------------------------------
static inline int BinaryToAscii(unsigned int c)
{
	c &= 126;
	c = (c <= 33 ? c + 33 : c);
	if (IsWeirdChar(c))
		c += 1;
	if (IsWeirdChar(c))
		c += 1;
	return c;
}

// this is more or less a one-shot deal and can acutally be a static function.
// Limited re-entrancy
const char* TRCryptor::EncryptPassword(const char* iPassword, const char* salt)
{
	HCRYPTPROV hCryptProv; 
	HCRYPTHASH hHash; 
	DWORD dwBufferLen; 
	// change the length of passwordWithSalt could potentially invalidates all 
	// previously hashed and stored passwords
	char passwordWithSalt[24];   
	static unsigned char sHashed[8][24];
	static int sN;
	unsigned char* hashed = sHashed[ sN = ((sN+1)&7)];
	
	
	if (!iPassword || !*iPassword)
	{
		return "";
	}
	
	if (!CryptAcquireContext( &hCryptProv,sContainerName, sProvider ,sProviderType,CRYPT_MACHINE_KEYSET ))
	{
		ReportError("AcquireContext");
		return "";
	}
	
	// create a hash objct
	if (!CryptCreateHash(hCryptProv,sHashAlgo,0,0,&hHash))
	{
		ReportError("CreateHash");
		return "";
	}

	if (!salt) salt = "";
	int N = snprintf(passwordWithSalt,sizeof passwordWithSalt,"%s%s",iPassword,salt);
    passwordWithSalt[sizeof passwordWithSalt - 1] = '\0';
	if (N < 0) N = strlen(passwordWithSalt);
	
	// now hash it
	if (!CryptHashData(hHash, (BYTE*) passwordWithSalt, N ,0))
	{
		ReportError("HashData");
		return "";
	}
	
	//GL 4-17-2006 change from HP_HASHSIZE to HP_HASHVAL to get required buffer size.
	// Get the hashed data
	dwBufferLen = 0;
	if (!CryptGetHashParam(hHash,HP_HASHVAL,0, &dwBufferLen,0))
	{
		ReportError("GetHashParam SIZE");
		return "";
	}
	
	if (dwBufferLen > 20)
	{
#ifdef _DEBUG
		fprintf(stderr,"dwBufferLen=%d\n", dwBufferLen);
#endif
		return "";// internal error
	}
	
	dwBufferLen = sizeof sHashed[0]; 
	if (!CryptGetHashParam(hHash,HP_HASHVAL, hashed, &dwBufferLen,0))
	{
		ReportError("GetHashParam VAL"); 
		return "";
	}
	
	hashed[dwBufferLen] = '\0';
	
	// turn the hash into ASCII
	for ( int n = dwBufferLen; --n >=0; )
		hashed[n] = BinaryToAscii(hashed[n]);
	
	CryptDestroyHash(hHash);
	CryptReleaseContext(hCryptProv,0);
	
	return (char *)hashed;
}

//GL 4-20-2006 Add Checksum for indentify the subseries by list of SOPInstanceUID
bool TRCryptor::InitMD5()
{
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	
	if (!crypt || !crypt->m_CryptProv)
		return false;
	
	crypt->Cleanup();
	
	
	if (!CryptCreateHash(crypt->m_CryptProv,CALG_MD5,0,0,&crypt->m_hashObject))
	{
		ReportError("CreateHash");
		return false;
	}
	
	m_endAddData = false; // open for add data
	m_addedDataCount = 0;
	return true;

}

bool TRCryptor::AddData(const void* iData, int iDataLen)
{
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	if (!iData || !crypt || m_endAddData || !CryptHashData(crypt->m_hashObject, (BYTE*) iData, iDataLen,0))
	{
		ReportError("AddData void*");
		return false;
	}
	m_addedDataCount++;
	return true;
}

bool TRCryptor::AddStrings(const std::vector<const char*>& iList)
{
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	if(m_endAddData || !crypt)
	{
		ReportError("AddStrings");
		return false;
	}

	int count = iList.size();
	const char* str;
	// now hash it
	for(int i=0; i<count; i++)
	{
		str = iList[i];

		if (!CryptHashData(crypt->m_hashObject, (BYTE*)str, strlen(str),0))
		{
			ReportError("AddStrings");
			return false;

		}
	}
	m_addedDataCount += count;
	return true;

}

static bool SortCString (const char* str0, const char* str1)
{
	return (strcmp (str0, str1) < 0);
}

bool TRCryptor::AddData(const std::vector<const char*>& iList, bool iSort)
{
	std::vector<const char*> mylist;
	const char* str;
	for(int i=0; i<iList.size(); i++)
	{
		str = iList[i];
		if(!str || strlen(str) > 10000000)
		{
			ReportError("AddData vector<const char*> invalid string");
			return false;
		}
		mylist.push_back(str);
	}

	if(iSort)
	{
		std::stable_sort (mylist.begin (), mylist.end (), SortCString);
	}

	return AddStrings(mylist);
}

bool TRCryptor::AddData(const AqStrList& iList, bool iSort)
{

	std::vector<const char*> mylist;
	const char* str;
	for(int i=0; i<iList.Size(); i++)
	{
		str = iList[i];
		if(!str || strlen(str) > 10000000)
		{
			ReportError("AddData vector<AqString> invalid string");
			return false;
		}
		mylist.push_back(str);
	}

	if(iSort)
	{
		std::stable_sort (mylist.begin (), mylist.end (), SortCString);
	}

	return AddStrings(mylist);
}

bool TRCryptor::GetChecksum(AqString& oHash, bool iPrefixCount)
{
	DWORD dwBufferLen; 
	unsigned char hashID[28];
	unsigned char* hashed = hashID+4;

	// Get the hashed data
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	if (!crypt)
		return false;

	dwBufferLen = 0;
	m_endAddData = true; // no more data add after CryptGetHashParam
	if (!CryptGetHashParam(crypt->m_hashObject, HP_HASHVAL, 0, &dwBufferLen,0))
	{
		ReportError("GetHashParam SIZE");
		return false;
	}
	
	if (dwBufferLen > 20)
	{
		return false;
	}
	
	dwBufferLen = 21;
	if (!CryptGetHashParam(crypt->m_hashObject, HP_HASHVAL, hashed, &dwBufferLen,0))
	{
		return false;
	}
	
	hashed[dwBufferLen] = '\0';
	if(!iPrefixCount)
	{
		AqBinToHex(hashed, dwBufferLen, oHash);
	}
	else
	{
		memcpy(hashID, &m_addedDataCount, 4); // prefix with count
		AqBinToHex(hashID, dwBufferLen+4, oHash);
	}
	return true;

}


//--------------------------------------------------------------------------------------
bool TRCryptor::SetKey(const char* iKey)
{
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	
	if (!crypt->m_CryptProv)
		return false;
	
	if (!iKey || !*iKey)
		iKey = "nvrServerCryptor";
	
	crypt->Cleanup();
	
	if (!CryptCreateHash(crypt->m_CryptProv,CALG_MD5,0,0,&crypt->m_hashObject))
	{
		ReportError("CreateHash");
		return false;
	}
	
	if (!CryptHashData(crypt->m_hashObject, (BYTE*) iKey, strlen(iKey),0))
	{
		ReportError("HashData");
		return false;
	}
	
	if(!CryptDeriveKey(crypt->m_CryptProv,sAlgoID,crypt->m_hashObject,CRYPT_EXPORTABLE,&crypt->m_key))
	{
        ReportError("DeriveKey");
		crypt->m_key = 0;
		return false;
	}
	
	return true;
}

//--------------------------------------------------------------------------------------
int TRCryptor::Encrypt(char *iBuf, int iLen, char* oBuf, int* oLen)
{
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	
	if (!crypt || !crypt->m_key)
	{
		return 0;
	}

	if (!iBuf || iLen <= 0)
		return 0;
	
	if (oBuf && !oLen)
		return 0;
	
	if (oLen && *oLen < iLen)
		return 0;
	
	DWORD inOutLen =  iLen;
	int	  bufLen = oLen ? *oLen : iLen;;
	if (oBuf)
	{
		memcpy(oBuf, iBuf, iLen);
		iBuf = oBuf;
	}
	
	if (!CryptEncrypt(crypt->m_key, 0,true, 0,(BYTE*)iBuf, &inOutLen, bufLen))
	{
		ReportError("Encrypt");
		return 0;
	}
	
	if (oLen)
		*oBuf = inOutLen;
	
	return inOutLen;
}

//--------------------------------------------------------------------------------------
int TRCryptor::Decrypt(char *iBuf, int iLen, char* oBuf, int* oLen)
{
	TRCryptorInternal *crypt = (TRCryptorInternal*)this->m_implementation;
	
	if (!crypt || !crypt->m_key)
	{
		return 0;
	}
	
	DWORD len = iLen;
	
	if (oBuf)
	{
		memcpy(oBuf,iBuf,iLen);
		iBuf = oBuf;
	}
	
	if (!CryptDecrypt(crypt->m_key,0,true,0, (BYTE*) iBuf,&len))
	{
		ReportError("Decrypt");
		return 0;
	}
	
	if (oLen)
		*oLen = len;
	
	return len;
}
