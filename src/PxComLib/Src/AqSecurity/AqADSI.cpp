/***********************************************************************
 * AqADSI.cpp
 *---------------------------------------------------------------------
 **************	This document contains information that	**************
 **************	   is proprietary to TeraRecon, Inc.	**************
 **************		     All rights reserved.			**************
 **************			Copyright TeraRecon, Inc.		**************
 **************				 2005-2006					**************
 *
 *	PURPOSE:
 *		AqNET active directory interface class for SSO
 *  
 * 
 *	AUTHOR:  Gang Li, DEC. 2005.
 * 
 */


#include "AqADSI.h"

#include "activeds.h"
//#include "Adshlp.h"


#pragma comment( lib, "ActiveDS.Lib" )
#pragma comment( lib, "ADSIID.LIB" ) 

static bool BindSearch(const wchar_t* iDomainLDAPStr, IDirectorySearch* &oDCSearch)
{
    HRESULT hr = E_FAIL;
	
	hr = ADsOpenObject(
		iDomainLDAPStr,
		0,
		0,
		ADS_SECURE_AUTHENTICATION,
		IID_IDirectorySearch,
		(void **)&oDCSearch
    );

	if (FAILED(hr))
	{
		if (oDCSearch)
			oDCSearch->Release();
		AqString domain;
		domain.ConvertUTF8(iDomainLDAPStr);
		GetAqLogger()->LogMessage("Error: BindSearch can't bind to domain:%s\n", domain);
		return false;
	}


   //Search entire subtree from root.
	ADS_SEARCHPREF_INFO arSearchPrefs[2];
	arSearchPrefs[0].dwSearchPref =     ADS_SEARCHPREF_SEARCH_SCOPE; 
    arSearchPrefs[0].vValue.dwType =    ADSTYPE_INTEGER; 
    arSearchPrefs[0].vValue.Integer =   ADS_SCOPE_SUBTREE; 

    arSearchPrefs[1].dwSearchPref =     ADS_SEARCHPREF_PAGESIZE; 
    arSearchPrefs[1].vValue.dwType =    ADSTYPE_INTEGER; 
    arSearchPrefs[1].vValue.Integer =   1000; 

	// Set the search preference
    hr = oDCSearch->SetSearchPreference(arSearchPrefs, 2); 
    if (FAILED(hr))
	{
		if (oDCSearch)
			oDCSearch->Release();
		AqString domain;
		domain.ConvertUTF8(iDomainLDAPStr);
		GetAqLogger()->LogMessage("Error: BindSearch fail to set filter on domain:%s\n", domain);
		return false;
	}


	return true;
}


/*

if not exists (select * from master.dbo.sysservers where srvname = 'ADSI')
	EXEC sp_addlinkedserver 'ADSI', 'Active Directory Services 2.5', 
		'ADSDSOObject', 'adsdatasource'


SELECT * FROM OpenQuery( ADSI, 
'SELECT objectSid, Name FROM ''LDAP://rtviz.com'' 
WHERE objectClass= ''group''') 

insert into UserGroup (DomainID, Privilege, Name, SID)
SELECT 1, 7, Name, objectSid FROM OpenQuery( ADSI, 
'SELECT objectSid, Name FROM ''LDAP://rtviz.com'' 
WHERE objectClass= ''group'' and Name=''Server Operators''
*/

static HRESULT AqGetNextRow(IDirectorySearch *pDSSearch, ADS_SEARCH_HANDLE hSearchHandle)
{
	HRESULT hr = S_OK;
	DWORD dwADsExtError = ERROR_SUCCESS;
	WCHAR szErrorBuf[512];
	WCHAR szNameBuf[128];

	do
	{
		// Clear ADSI extended error
		dwADsExtError = ERROR_SUCCESS;
		ADsSetLastError(ERROR_SUCCESS, NULL, NULL);

		// Next row
		hr = pDSSearch->GetNextRow(hSearchHandle);
		if ( !SUCCEEDED(hr) )
			return hr;

		
		// Check ADSI extend error if we got S_ADS_NOMORE_ROWS
		if (S_ADS_NOMORE_ROWS == hr)
		{
			hr = ADsGetLastError(&dwADsExtError, szErrorBuf, 512, szNameBuf, 128);
			if ( !SUCCEEDED(hr) )
				return hr;

			if (ERROR_MORE_DATA != dwADsExtError)
				// All data received
				return ERROR_NO_DATA;
		}

	} while (ERROR_MORE_DATA == dwADsExtError);

	return ERROR_MORE_DATA;

}


bool AqADSI::GetADGroups(const wchar_t* iDomainLDAPStr, std::vector<AqMixStringKV>& oGroups)
{
    HRESULT hr = E_FAIL;
    IDirectorySearch *pGCSearch = 0;

	oGroups.clear();
	if(!BindSearch(iDomainLDAPStr, pGCSearch))
		return false;

	AqString domain;
	domain.ConvertUTF8(iDomainLDAPStr);

	//Create search filter
	wchar_t* pszSearchFilter = L"((objectClass=group))";
	
	// Set attributes to return
	const DWORD dwAttrNameSize = 2;
	wchar_t* pszAttribute[dwAttrNameSize] = {L"cn",L"objectSid"};
	

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;

    // Execute the search
    hr = pGCSearch->ExecuteSearch(pszSearchFilter, pszAttribute, dwAttrNameSize, &hSearch);
	if ( !SUCCEEDED(hr) )
	{
		if (pGCSearch)
			pGCSearch->Release();
		
		AqString domain;
		domain.ConvertUTF8(iDomainLDAPStr);
		GetAqLogger()->LogMessage("Error: AqADSI::GetADGroups can't group in domain:%s\n", domain);

		return false;

	}

	// COL for iterations
	ADS_SEARCH_COLUMN col;
	// Call IDirectorySearch::GetNextRow() to retrieve the next row 
	//of data
	int rows = 0;
	bool success = true;
	PSID pObjectSID = 0;
	AqMixStringKV addGroup;
	oGroups.clear();
    //while( pGCSearch->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
	while( AqGetNextRow(pGCSearch, hSearch) == ERROR_MORE_DATA)
	{
		
		// Get the data for this column
        hr = pGCSearch->GetColumn( hSearch, pszAttribute[0], &col );
		if ( !SUCCEEDED(hr) || col.dwADsType != ADSTYPE_CASE_IGNORE_STRING )
		{
			//success = false;
			if(SUCCEEDED(hr))
				pGCSearch->FreeColumn( &col );
			GetAqLogger()->LogMessage("Warning: GetADGroups stopped at group:%d on domain(%x):%s\n", rows, hr, domain);
			break;
		}
		
		addGroup.Key = col.pADsValues->CaseIgnoreString;
		pGCSearch->FreeColumn( &col );
		
        hr = pGCSearch->GetColumn( hSearch, pszAttribute[1], &col );
		if ( !SUCCEEDED(hr) || col.dwADsType != ADSTYPE_OCTET_STRING )
		{
			//success = false;
			if(SUCCEEDED(hr))
				pGCSearch->FreeColumn( &col );

			GetAqLogger()->LogMessage("Warning: GetADGroups 2 stopped at group:%d on domain(%x):%s\n", rows, hr, domain);

			break;
		}

		pObjectSID = (PSID)(col.pADsValues->OctetString.lpValue);
		if(IsValidSid(pObjectSID))
		{
			AqBinToHex(pObjectSID, GetLengthSid(pObjectSID), addGroup.Value);
		}
		else
		{
			//success = false;
			pGCSearch->FreeColumn( &col );

			GetAqLogger()->LogMessage("Warning: GetADGroups encounter invalid SID at group:%d on domain:%s\n", rows, domain);
			break;
		}

		rows++;
		oGroups.push_back(addGroup);
		pGCSearch->FreeColumn( &col );

	}
		
	// Close the search handle to clean up
    pGCSearch->CloseSearchHandle(hSearch);
	if (pGCSearch)
		pGCSearch->Release();

	GetAqLogger()->LogMessage("Info: GetADGroups received %d groups on domain:%s\n", rows, domain);
    return success;
}

bool AqADSI::GetADUserInfo(const wchar_t* iUsername, const wchar_t* iDomainLDAPStr, AqUString& oFName, AqUString& oLName, AqUString& oEmail)
{
    HRESULT hr = E_FAIL;
    IDirectorySearch *pGCSearch = 0;

	oFName = L""; oLName=L""; oEmail=L"";
	if(!BindSearch(iDomainLDAPStr, pGCSearch))
	{
		AqString user, domain;
		user.ConvertUTF8(iUsername);
		domain.ConvertUTF8(iDomainLDAPStr);

		GetAqLogger()->LogMessage("Error: AqADSI::GetADUserInfo fail on user: %s in domain:%s\n", user, domain);
		return false;
	}
	//Create search filter
	AqUString filter;
	filter.Format(L"(&(objectCategory=person)(objectClass=user)(sAMAccountName=%s))", iUsername);

	// Set attributes to return
	const DWORD dwAttrNameSize = 3;
    wchar_t* pszAttribute[dwAttrNameSize] = {L"givenName",L"sn", L"mail"};

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;

    // Execute the search
    hr = pGCSearch->ExecuteSearch(filter.GetBuffer(), pszAttribute, dwAttrNameSize, &hSearch);
	if ( !SUCCEEDED(hr) )
	{
		if (pGCSearch)
			pGCSearch->Release();
		
		AqString user, domain;
		user.ConvertUTF8(iUsername);
		domain.ConvertUTF8(iDomainLDAPStr);

		GetAqLogger()->LogMessage("Error: AqADSI::GetADUserInfo fail on user: %s in domain:%s\n", user, domain);

		return false;

	}

	// COL for iterations
	ADS_SEARCH_COLUMN col;
	// Call IDirectorySearch::GetNextRow() to retrieve the next row 
	//of data
	bool success = false;
	if( pGCSearch->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
	{
		success = true; // it is allow some user not have last name , first name or email
		// Get the data for this column
        hr = pGCSearch->GetColumn( hSearch, pszAttribute[0], &col );
		if ( SUCCEEDED(hr) || col.dwADsType == ADSTYPE_CASE_IGNORE_STRING )
		{
			oFName = col.pADsValues->CaseIgnoreString;
		}
		if(SUCCEEDED(hr))
			pGCSearch->FreeColumn( &col );
		
		hr = pGCSearch->GetColumn( hSearch, pszAttribute[1], &col );
		if ( SUCCEEDED(hr) && col.dwADsType == ADSTYPE_CASE_IGNORE_STRING )
		{
			oLName = col.pADsValues->CaseIgnoreString;
		}
		if(SUCCEEDED(hr))
			pGCSearch->FreeColumn( &col );


        hr = pGCSearch->GetColumn( hSearch, pszAttribute[2], &col );
		if ( SUCCEEDED(hr) && col.dwADsType == ADSTYPE_CASE_IGNORE_STRING )
		{
			oEmail = col.pADsValues->CaseIgnoreString;
		}
		if(SUCCEEDED(hr))
			pGCSearch->FreeColumn( &col );

	}
		
	// Close the search handle to clean up
    pGCSearch->CloseSearchHandle(hSearch);
	if (pGCSearch)
		pGCSearch->Release();
    return success;
}


bool AqADSI::CheckDomain(const wchar_t* iDomain)
{
	AqUString cstr = L"LDAP://";
	// get LDAP search string
	cstr += L"DC=";
	cstr += iDomain;
	cstr.Replace(L".", L",DC=");

		
	IDirectorySearch *pGCSearch = 0;

	if(!BindSearch(cstr, pGCSearch))
		return false;
	else
	{
		if (pGCSearch)
			pGCSearch->Release();
		return true;
	}

}


bool AqADSI::GetDomainGroups(const wchar_t* iDomain, std::vector<AqMixStringKV>& oGroups)
{
	AqUString cstr = L"LDAP://";
	if(!iDomain || !iDomain[0])
	{
		// get default root domain
		HRESULT hr;
		IADs *pRoot=NULL;
    
		hr = ADsGetObject(L"LDAP://RootDSE", IID_IADs,(void**)&pRoot);
		if (!SUCCEEDED(hr))
		{
			GetAqLogger()->LogMessage("Error: AqADSI::GetDomainUserInfo can't get default domain\n");
			return false;
		}
	
		VARIANT varDSRoot;
		hr = pRoot->Get(L"defaultNamingContext",&varDSRoot);
		cstr += varDSRoot.bstrVal;
		//_tprintf(TEXT("\nDomain Name is :%s\n"),varDSRoot.bstrVal);
	    pRoot->Release();
	}
	else
	{
		// get LDAP search string
		cstr += L"DC=";
		cstr += iDomain;
		cstr.Replace(L".", L",DC=");
	}

	//cstr = L"GC:";
	return GetADGroups( cstr, oGroups);
}



bool AqADSI::GetDomainUserInfo(const wchar_t* iUsername, const wchar_t* iDomain, AqUString& oFName, AqUString& oLName, AqUString& oEmail)
{
	if(!iUsername || !iUsername[0])
		return false;

	AqUString cstr = L"LDAP://";
	if(!iDomain || !iDomain[0])
	{
		// get default root domain
		HRESULT hr;
		IADs *pRoot=NULL;
    
		hr = ADsGetObject(L"LDAP://RootDSE", IID_IADs,(void**)&pRoot);
		if (!SUCCEEDED(hr))
		{
			GetAqLogger()->LogMessage("Error: AqADSI::GetDomainUserInfo can't get default domain\n");
			return false;
		}
	
		VARIANT varDSRoot;
		hr = pRoot->Get(L"defaultNamingContext",&varDSRoot);
		cstr += varDSRoot.bstrVal;
		//_tprintf(TEXT("\nDomain Name is :%s\n"),varDSRoot.bstrVal);
	    pRoot->Release();
	}
	else
	{
		// get LDAP search string
		cstr += L"DC=";
		cstr += iDomain;
		cstr.Replace(L".", L",DC=");
	}

	return GetADUserInfo(iUsername, cstr, oFName, oLName, oEmail);
}


