/***********************************************************************
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */
#include "PxDB.h"

#include <string>

#include "PxDBCharacterLib.h"


JISCharacterLib::JISCharacterLib(){
		m_hCharacterLib = nullptr;
		m_convertToCurrentAcp = nullptr;
		m_convertFromCurrentAcp = nullptr;
		m_convertSJISToJISForSQL = nullptr;
	}
	
JISCharacterLib::~JISCharacterLib()
	{
		if (m_hCharacterLib != nullptr)
			FreeLibrary(m_hCharacterLib);
	}

bool JISCharacterLib::loadLib(void){
#ifdef _DEBUG
	const char *characterLibName = "PxCharacterD.dll";
#else
	const char *characterLibName = "PxCharacter.dll";
#endif
	m_hCharacterLib = LoadLibrary(characterLibName);

	if (m_hCharacterLib != NULL) {
		m_convertToCurrentAcp = (CONVERT_TO_CURRENT_ACP)GetProcAddress(m_hCharacterLib, "ConvertToCurrentACP");
		if (m_convertToCurrentAcp == nullptr){
			return false;
		}
		m_convertFromCurrentAcp = (CONVERT_FROM_CURRENT_ACP)GetProcAddress(m_hCharacterLib, "ConvertFromCurrentACP");
		if (m_convertFromCurrentAcp == nullptr){
			return false;
		}
		m_convertSJISToJISForSQL = (CONVERT_SJISTOJIS_FORSQL)GetProcAddress(m_hCharacterLib, "ConvertSJISToJISForSQL");
		if (m_convertSJISToJISForSQL == nullptr){
			return false;
		}
	}
	else{
		return false;
	}
	return true;
}

std::string JISCharacterLib::convertJIStoSJIS(const std::string &jisStr)
	{
		unsigned int length = MAX_PATH * 2;
		if (length == 0)
			return jisStr;
		char* strTemp = new char[length + 1];
		unsigned int page_code = _Def_CodePage_SJIS_932; // shift-jis;
		m_convertToCurrentAcp(jisStr.c_str(), strTemp, length + 1, "CP932"/*ignored*/, page_code);
		 
		std::string retStr = strTemp;
		delete[] strTemp;
		return retStr;
	 
	}

std::string JISCharacterLib::convertSJIStoJIS(const std::string &SjisStr)
	{
		unsigned int length = MAX_PATH * 2;
		char* strTemp = new char[length];
		unsigned int page_code = _Def_CodePage_SJIS_932; // shift-jis;
		m_convertFromCurrentAcp(SjisStr.c_str(), strTemp, length, "CP932", page_code);
		std::string retStr = strTemp;
		delete[] strTemp;
		return retStr;
	};

std::string JISCharacterLib::convertSJToJCodeOnlyForSQL(const std::string &SjisStr)
{
	unsigned int length = MAX_PATH * 2;
	if (length == 0)
		return SjisStr;
	char* strTemp = new char[length + 1];
	unsigned int page_code = _Def_CodePage_SJIS_932; // shift-jis;
	m_convertSJISToJISForSQL(SjisStr.c_str(), strTemp, length + 1, "CP932"/*ignored*/, page_code);

	std::string retStr = strTemp;
	delete[] strTemp;
	return retStr;
}
	 