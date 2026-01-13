
// testdcmInfoAPI.cpp : ŽÀ‘•ƒtƒ@ƒCƒ‹
//

#include "stdafx.h"
#include "testdcmInfo.h"
#include "testdcmInfoAPI.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "IDcmLib.h"


using namespace XTDcmLib;


CtestdcmInfoAPI::CtestdcmInfoAPI(void) 
{
	m_CharStrBuffSize = 2048;
	m_CharStrBuff = new char[m_CharStrBuffSize];
	m_WCharStrBuff = new WCHAR[m_CharStrBuffSize];
}
CtestdcmInfoAPI::~CtestdcmInfoAPI()
{
	if (m_CharStrBuff != nullptr){
		delete[] m_CharStrBuff;
	}
	if (m_WCharStrBuff != nullptr){
		delete[] m_WCharStrBuff;
	}
}
const char * CtestdcmInfoAPI::cnvToChar(const TCHAR *inStr)
{
#ifdef UNICODE	 
	WideCharToMultiByte(CP_ACP, 0, inStr, -1, m_CharStrBuff, m_CharStrBuffSize, NULL, NULL);
	return m_CharStrBuff;
#else
	return inStr;
#endif
}

bool CtestdcmInfoAPI::loadDicomFile(const CString &file)
{
	IDcmLib *dcmlib_instance = IDcmLib::createInstance();

	DcmXTDicomMessage *dcm_file_instance = dcmlib_instance->createDicomMessage();
	dcm_file_instance->setMaxReadLength(16);
	bool rdSts = dcm_file_instance->readFile(cnvToChar(file));
	//DcmXTDataSet *dataset = dcm_file_instance->getDcmXTDataSet();
	char string_val[1024];
	dcm_file_instance->Get_Value(0x00100010, string_val, 1024);

	std::wstring strTemp;
	Utf8ToWString(strTemp, string_val);
	m_PatientNameW = strTemp;
	WStringToChar(m_PatientName, m_PatientNameW);
	dcmlib_instance->destroy();
	return true;
}
bool CtestdcmInfoAPI::Utf8ToWString(std::wstring& cstrOut, const char* utf8Str)
{
	size_t utf8StrLen = strlen(utf8Str);

	if (utf8StrLen == 0)
	{
		cstrOut.clear();
		return true;
	}


	// CString is UNICODE string so we decode
	int newLen = MultiByteToWideChar(
		CP_UTF8, 0,
		utf8Str, utf8StrLen, m_WCharStrBuff, m_CharStrBuffSize
		);
	if (!newLen)
	{
		cstrOut.clear();
		return false;
	}
	m_WCharStrBuff[newLen] = 0;


	cstrOut = m_WCharStrBuff;
	return true;
}
bool CtestdcmInfoAPI::Big5ToWString(std::wstring& cstrOut, const char* utf8Str)
{
	size_t utf8StrLen = strlen(utf8Str);

	if (utf8StrLen == 0)
	{
		cstrOut.clear();
		return true;
	}


	// CString is UNICODE string so we decode
	int newLen = MultiByteToWideChar(
		CTRY_TAIWAN, 0,
		utf8Str, utf8StrLen, m_WCharStrBuff, m_CharStrBuffSize
		);
	if (!newLen)
	{
		cstrOut.clear();
		return false;
	}
	m_WCharStrBuff[newLen] = 0;


	cstrOut = m_WCharStrBuff;
	return true;
}
bool CtestdcmInfoAPI::WStringToChar(std::string& cstrOut, const std::wstring& wStr)
{


	int newLen = WideCharToMultiByte(
		CP_ACP, 0,
		wStr.c_str(), wStr.size(),
		m_CharStrBuff, m_CharStrBuffSize,
		NULL,
		NULL);
	if (!newLen)
	{
		cstrOut.clear();

		return false;
	}
	m_CharStrBuff[newLen] = 0;
	cstrOut = m_CharStrBuff;
	return true;
}