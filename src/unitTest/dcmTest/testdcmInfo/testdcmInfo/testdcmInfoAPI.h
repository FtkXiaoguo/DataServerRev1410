
// testdcmInfoAPI.h  
//

#pragma once
#include "afxwin.h"

#include <string>
class CtestdcmInfoAPI  
{
 
public:
	CtestdcmInfoAPI(void);
	virtual ~CtestdcmInfoAPI(void);
	bool loadDicomFile(const CString &file);
	std::wstring m_PatientNameW;
	std::string m_PatientName;
	bool Utf8ToWString(std::wstring& cstrOut, const char* utf8Str);
	bool WStringToChar(std::string& cstrOut, const std::wstring& wStr);
	bool Big5ToWString(std::wstring& cstrOut, const char* utf8Str);
protected:
	const char * cnvToChar(const TCHAR *inStr);
	
	char *m_CharStrBuff;
	WCHAR *m_WCharStrBuff;
	int  m_CharStrBuffSize;
	
};
