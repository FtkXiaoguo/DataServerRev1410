/**************************************************************************
 * TRCryptor.h
 *
 **************************************************************************/


#ifndef TRCRYPTOR_H_
#define TRCRYPTOR_H_

#include "AqString.h"
#include <vector>

class TRCryptorInternal;

class TRCryptor
{
public:

	TRCryptor(void);
	~TRCryptor(void);

	TRCryptor(const TRCryptor& iE);
	TRCryptor& operator=(const TRCryptor& iE);

	/* Take a null-terminated password, and output an null-terminated 
	 * encrypted password (MD5 hashing). Encryption here is NOT reversible.
	 * In case of error, an empty string is returned.
	 * The encrypted password will have a max of 20 chars.
	 */
	const char* EncryptPassword(const char* iPassword, const char* salt="");


	/* Set the encryption/decryption key. Returns 0 for success */
	bool		SetKey(const char* iKey);

	/* take a piece of text and generate an encrypted (MD5 & RC4) version of it
	 * reversible with the same key. f oBuf is zero, the encryption/decryption will 
	 * be done in place, overwriting the input buffer iBuf.  If oBuf is not zero, 
	 * it specifies the output buffer, and oLen on input
	 * specifies the size of the output buffer. On output, oLen will be
	 * the length of the output. oLen >= iLen must hold.
	 * On error, function returns a negative number otherwise the size
	 * of the output.
	 */
	int		Encrypt(char* iBuf, int iLen,  char* oBuf=0, int*oLen=0);

	/* Decrypt a previously encrypted text */
	int		Decrypt(char *iBuf, int iLen,  char* oBuf=0, int*oLen=0);


	//GL 4-20-2006 Add Checksum for indentify the subseries by list of SOPInstanceUID
	bool InitMD5();
	bool AddData(const void* iData, int iDataLen);
	bool AddData(const std::vector<const char*>& iList, bool iSort=false);
	bool AddData(const AqStrList& iList, bool iSort=false);

	bool GetChecksum(AqString& oHash, bool iPrefixCount=false);
	bool GetCountAndHashHex(AqString& oHash) {return GetChecksum(oHash, true);};
	
	bool GetCountAndHashHex(const std::vector<const char*>& iList, AqString& oHash)
		{return (InitMD5() && AddData(iList, true) && GetCountAndHashHex(oHash));}
	
	bool GetCountAndHashHex(const AqStrList& iList, AqString& oHash)
		{return (InitMD5() && AddData(iList, true) && GetCountAndHashHex(oHash));}
	

private:
	bool AddStrings(const std::vector<const char*>& iList);
	TRCryptorInternal*		m_implementation;
	bool					m_endAddData;
	int						m_addedDataCount;
};



#endif