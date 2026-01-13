/*************************************************************************
 * $Id: RTVRegistry.cpp 35 2008-08-06 02:57:21Z atsushi $
 *
 *---------------------------------------------------------------------
 *	Copyright (c) PreXion , 2001. All rights reserved.
 *
 *	PURPOSE:
 *		Member of the AQNetConfiguration Class 
 *
 *
 *  
 *
 ************************************************************************/
#pragma warning (disable: 4786)

#if !defined(RTVREGISTRY_H)
#include "RTVRegistry.h"
#endif

#include "AqCore/AqCore.h"

static const int kRTVREGISTRYMaxPath = 1024;
//-----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RTVRegistry::RTVRegistry(HKEY iWhich, const char* iLocation)
{
	m_currentRegistryTopLevelKey = iWhich;
	m_currentLocation = iLocation;
}

//-----------------------------------------------------------------------------

RTVRegistry::~RTVRegistry()
{

}

//-----------------------------------------------------------------------------
// This function writes a key into the registry.
// if the Key is not present then it Creates it and sets the Value
//

int RTVRegistry::SetRegistryKey (const char* iKeyName, const char* iKeyValue)
{
	HKEY	hKey;

	int		retcd;
	DWORD	result;
	
	retcd = RegCreateKeyEx (m_currentRegistryTopLevelKey, m_currentLocation.c_str(), 0, "", REG_OPTION_NON_VOLATILE,
			                KEY_ALL_ACCESS, NULL, &hKey, &result);

	if( retcd != ERROR_SUCCESS )
	{
		return kErrCannotCreateKey;
	}
	
	retcd = RegSetValueEx (hKey, iKeyName, 0, REG_SZ, (BYTE*)iKeyValue, strlen(iKeyValue));

	if( retcd != ERROR_SUCCESS )
	{
		RegCloseKey( hKey );

		return kErrCannotWriteKey;
	}

	RegCloseKey (hKey);
	
	return kSuccess;
}

//-----------------------------------------------------------------------------
// This function writes a key into the registry.
// if the Key is not present then it Creates it and sets the Value
//

int RTVRegistry::SetRegistryKey (const char* iKeyName, int iKeyValue)
{
	HKEY	hKey;

	int		retcd;
	DWORD	result;
	
	retcd = RegCreateKeyEx (m_currentRegistryTopLevelKey, m_currentLocation.c_str(), 0, "", REG_OPTION_NON_VOLATILE,
			                KEY_ALL_ACCESS, NULL, &hKey, &result);

	if( retcd != ERROR_SUCCESS )
	{
		return kErrCannotCreateKey;
	}
	
	retcd = RegSetValueEx (hKey, iKeyName, 0, REG_DWORD, (BYTE*)&iKeyValue, sizeof (iKeyValue));

	if( retcd != ERROR_SUCCESS )
	{
		RegCloseKey( hKey );

		return kErrCannotWriteKey;
	}

	RegCloseKey (hKey);
	
	return kSuccess;
}

//-----------------------------------------------------------------------------
// This function writes a key into the registry.
// if the Key is not present then it Creates it and sets the Value
// SH, change implemenation to save decimal string instead of hex string
// so user can easily understand and change the values

int RTVRegistry::SetRegistryKey (const char* iKeyName, double iKeyValue,
								 const int iNumberOfDigitsAfterDecimalPoint)
{
	
	int  decimal, sign;
	char *buffer = 0;

	buffer = _fcvt( iKeyValue, iNumberOfDigitsAfterDecimalPoint, &decimal, &sign );
	if ( !buffer || buffer[0] == '\0' )
	   return kErrCannotWriteKey;

	if ( decimal < 0 )
	   return kErrCannotWriteKey;

	std::string keyValueString = buffer;
	keyValueString.insert(decimal,".",1);
	if ( decimal == 0 )
		keyValueString.insert(0,"0",1);

	if ( sign )
	   keyValueString.insert(0,"-",1);
	
	return SetRegistryKey(iKeyName,keyValueString.c_str());
}

//-----------------------------------------------------------------------------
// This function gets key set in the registry
int RTVRegistry::GetRegistryKey (const char* iKeyName, std::string& oKeyValue)
{
	HKEY	hKey;
	DWORD	size;
	DWORD	type;
	BYTE	str[kRTVREGISTRYMaxPath] = { 0 };

	int retcd = RegOpenKey (m_currentRegistryTopLevelKey, m_currentLocation.c_str(), &hKey);

	if( retcd == ERROR_SUCCESS )
	{
		retcd = RegQueryValueEx (hKey, iKeyName, NULL, &type, NULL, &size);

		if( retcd == ERROR_SUCCESS )
		{
			if (size < kRTVREGISTRYMaxPath)
				RegQueryValueEx (hKey, iKeyName, NULL, &type, str, &size);
			else
			{
				RegCloseKey (hKey);
				return kErrKeyTooLong;
			}
		}
		else
		{
			RegCloseKey( hKey );
			return kErrCannotFindKey;
		}
		// Abe
		
		RegCloseKey( hKey );
	}
	else
	{
		return kErrCannotFindKey;
	}

	int n;
	
	if (type == REG_DWORD)
	{
		n = *(int *)str;
		sprintf((char *)str,"%d", n);
	}
	
	oKeyValue = (char*)str;
 

	return kSuccess;
}

//-----------------------------------------------------------------------------
// This function gets key set in the registry
int RTVRegistry::GetRegistryKey (const char* iKeyName, int& oKeyValue)
{
	HKEY	hKey;
	DWORD	size;
	DWORD	type;
    DWORD   value = 0;
	int status = kSuccess;
	int retcd = RegOpenKey (m_currentRegistryTopLevelKey, m_currentLocation.c_str(), &hKey);

	if( retcd == ERROR_SUCCESS )
	{
		retcd = RegQueryValueEx (hKey, iKeyName, NULL, &type, NULL, &size);

		if( retcd == ERROR_SUCCESS )
		{
            RegQueryValueEx (hKey, iKeyName, NULL, &type, (BYTE*)&value, &size);
		}
		else
		{
			status =  kErrCannotFindKey;
		}
		RegCloseKey( hKey );
	}
	else
	{
		status =  kErrCannotFindKey;
	}
	
	// modified by shiying hu, 2006-03-17
	// should only assign value when it is successful
	if ( status == kSuccess )
		oKeyValue = value;

	return status;
}

//-----------------------------------------------------------------------------
// This function gets key set in the registry
// SH, change to read regular string instead of Hex string
int RTVRegistry::GetRegistryKey (const char* iKeyName, double& oKeyValue)
{
	std::string valueString;
	int status = GetRegistryKey(iKeyName,valueString);
	if ( status != kSuccess )
		return status;

	double value = atof(valueString.c_str());
	if ( value == 0.0 )
		return kErrCannotFindKey;
	
	oKeyValue = value;

	return kSuccess;
}
//-----------------------------------------------------------------------------

int RTVRegistry::DeleteRegistryLocation (const char* iKeyValue)
{	
	int retcd = RegDeleteKey (m_currentRegistryTopLevelKey, iKeyValue);

	if( retcd != ERROR_SUCCESS )
	{
		return kErrCannotFindKey;
	}

	return kSuccess;
}
//-----------------------------------------------------------------------------