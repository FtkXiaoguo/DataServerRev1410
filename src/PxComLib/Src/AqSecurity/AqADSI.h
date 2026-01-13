/***********************************************************************
 * AqADSI.h
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


#ifndef AQADSI_H_
#define AQADSI_H_


#include "AqCore/AqString.h"
#include <vector>

class AqADSI
{
public:
	AqADSI() {};
	virtual ~AqADSI() {};

	static bool GetADGroups(const wchar_t* iDomainLDAPStr, std::vector<AqMixStringKV>& oGroups);
	static bool CheckDomain(const wchar_t* iDomain);
	static bool GetADUserInfo(const wchar_t* iUsername, const wchar_t* iDomainLDAPStr, AqUString& oFName, AqUString& oLName, AqUString& oEmail);

	//if iDomain is nul pointer or empty, search in RootDSE
	static bool GetDomainGroups(const wchar_t* iDomain, std::vector<AqMixStringKV>& oGroups);
	static bool GetDomainUserInfo(const wchar_t* iUsername, const wchar_t* iDomain, AqUString& oFName, AqUString& oLName, AqUString& oEmail);
};

#endif //AQADSI
