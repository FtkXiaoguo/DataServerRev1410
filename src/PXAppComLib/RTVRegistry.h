/*************************************************************************
 * $Id: RTVRegistry.h 35 2008-08-06 02:57:21Z atsushi $
 *
 *---------------------------------------------------------------------
 *	Copyright (c) TeraRecon, Inc., 2001. All rights reserved.
 *
 *	PURPOSE:
 *		This class has function to put and get keys in the regigtry
 *
 *
 *  AUTHOR(S):
 *		Vikram Simha
 *
 ************************************************************************/

#if !defined(RTVREGISTRY_H)
#define RTVREGISTRY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//-----------------------------------------------------------------------------

#include <stdio.h>
#include <windows.h>
#include <string>



//-----------------------------------------------------------------------------

class RTVRegistry  
{
	public:
		enum eErrorCodes
		{
			kSuccess = 0,
			kErrCannotCreateKey,
			kErrCannotWriteKey,
			kErrCannotFindKey,
			kErrKeyTooLong
		};

		// iWhich can be one of predefined
		// HKEY_CLASSES_ROOT 
		// HKEY_CURRENT_USER 
		// HKEY_LOCAL_MACHINE 
		// HKEY_USERS 
		// HKEY_CURRENT_CONFIG
		// HKEY_PERFORMANCE_DATA
		// HKEY_DYN_DATA
		// or a new one
		RTVRegistry (HKEY iWhich, const char* iLocation);
		virtual ~RTVRegistry();

		int RTVRegistry::SetRegistryKey (const char* iKeyName, const char* iKeyValue);
        int RTVRegistry::SetRegistryKey (const char* iKeyName, int iKeyValue);

		// added by shiying hu, 2006-02-14
		int RTVRegistry::SetRegistryKey (const char* iKeyName, double iKeyValue,
			const int iNumberOfDigitsAfterDecimalPoint = 5);
        
        int RTVRegistry::GetRegistryKey (const char* iKeyName, std::string& oKeyValue);
        int RTVRegistry::GetRegistryKey (const char* iKeyName, int& oKeyValue);

		// added by shiying hu, 2006-02-14
		// we use atof to convert string to double
		// be aware that return value 0.0 indicates error.
		// therefore, we have to avoid setting registry value to 0.0
        int RTVRegistry::GetRegistryKey (const char* iKeyName, double& oKeyValue);

		// Deletes all the stuff in the m_currentLocation and removes it from the
		// Registry
		int RTVRegistry::DeleteRegistryLocation (const char* iKeyValue);

	protected:		
		// Handle to one of the current registry keys
		HKEY	      m_currentRegistryTopLevelKey;

		// The current Top Level Key in the Registry
		std::string   m_currentLocation;

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif // !defined(RTVREGISTRY_H)
