/***********************************************************************
 *---------------------------------------------------------------------
 *	#2240_for_Japanese_DB_Search
 *-------------------------------------------------------------------
 */

typedef void(__stdcall* CONVERT_TO_CURRENT_ACP)(const char* src, char* dest, const unsigned int bufsize, const char* dicomCharset, UINT acp);
typedef void(__stdcall* CONVERT_FROM_CURRENT_ACP)(const char* src, char* dest, const unsigned int bufsize, const char* dicomCharset, UINT acp);
typedef void(__stdcall* CONVERT_SJISTOJIS_FORSQL)(const char* src, char* dest, const unsigned int bufsize, const char* dicomCharset, UINT acp);
 
#define _Def_CodePage_SJIS_932 (932)
#define _Def_MyCodePage_JIS (65100)
class JISCharacterLib
{
public:
	JISCharacterLib();
	
	~JISCharacterLib();
	std::string convertJIStoSJIS(const std::string &jisStr);
	std::string convertSJIStoJIS(const std::string &jisStr);
	std::string convertSJToJCodeOnlyForSQL(const std::string &org);
	bool loadLib(void);
protected:
	HMODULE						m_hCharacterLib;
	CONVERT_TO_CURRENT_ACP		m_convertToCurrentAcp;
	CONVERT_FROM_CURRENT_ACP	m_convertFromCurrentAcp;
	CONVERT_SJISTOJIS_FORSQL	m_convertSJISToJISForSQL;
 
};
 