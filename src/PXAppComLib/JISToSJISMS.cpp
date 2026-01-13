/** 
 * @file  JISToSJISMS.cpp
 * @brief CJISToSJISMS Class Implementation
 *
 * @author TeraRecon, Inc. <kuni@terarecon.co.jp>
 * @date 2004-02-03
 * @version $Id: JISToSJISMS.cpp 1707 2011-02-21 07:17:01Z furutsuki $
 *
 * Copyright (C) 2004 TeraRecon, Inc. All rights reserved.
 */

 
#include "JISToSJISMS.h"

#include <mbstring.h>
#include <vector>

using namespace std;

#define BOOL bool
#define TRUE true
#define FALSE false
/*----------------------------------------------------------------------
  CJISToSJISMS
----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  Constructor/Desctructor
----------------------------------------------------------------------*/
CJISToSJISMS::CJISToSJISMS()
{

}

CJISToSJISMS::~CJISToSJISMS()
{

}

/*----------------------------------------------------------------------
  Member Function
----------------------------------------------------------------------*/
/** Conversion Function of JIS to SJIS
 * @param org   Cstring containing "JIS" characters( ASCII also OK )
 * @param conv  Cstring stored by this function with converted "SJIS" characters
 */
void CJISToSJISMS::ConvertJISToSJIS(const std::string &org, std::string &conv)
{
	char *convc = new char[org.size() + 1];

	if ( convc == NULL ) {
		conv = "";	return;
	}

	ConvertJISToSJIS( org.c_str(), convc, org.size() + 1);

	conv = convc;

	delete [] convc;

	return;
}

/** Conversion Function of JIS to SJIS
 * @param org      const char containing "JIS" characters( ASCII also OK )
 * @param conv     char stored by this function with converted "SJIS" characters
 * @param bufsize  buffer size of conv
 */
void CJISToSJISMS::ConvertJISToSJIS(const char *org, char *conv, int bufsize)
{
	unsigned long	i = 0, skip = 0, actualsize;
	BOOL			doubleByte = FALSE;
	BOOL			katakana = FALSE;

	actualsize = strlen(org);

	if ( actualsize > bufsize - 1 ) 
	{
		*conv = 0;	return;
	}
	else
	{
		memset(conv, NULL, bufsize);
	}

	strcpy(conv, org);

	while(i < actualsize)
	{
		if ( strncmp((conv + i), "\033(B", 3) == 0 ) // reg6 ascii
		{
			doubleByte = FALSE;
			katakana = FALSE;
			i+=3;
			skip++;
		}
		if ( strncmp((conv + i), "\033(I", 3) == 0 ) // reg13 JIS X 0201-Katakana
		{
			doubleByte = FALSE;
			katakana = TRUE;
			i+=3;
			skip++;
		}
		if ( strncmp((conv + i), "\033(J", 3) == 0 ) // reg14 JIS X 0201-Roman
		{
			doubleByte = FALSE;
			katakana = FALSE;
			i+=3;
			skip++;
		}
		if ( strncmp((conv + i), "\033$@", 3) == 0 ) // reg42 JIS X 0208-1978
		{
			doubleByte = TRUE;
			katakana = FALSE;
			i+=3;
			skip++;
		}
		if ( strncmp((conv + i), "\033$B", 3) == 0 ) // reg87 JIS X 0208-1983
		{
			doubleByte = TRUE;
			katakana = FALSE;
			i+=3;
			skip++;
		}
		// kunikichi 2006.06.13 found \033)I sequence in real data. is this same as \033(B ?
		if ( strncmp((conv + i), "\033)I", 3) == 0 ) // reg6 ascii
		{
			doubleByte = FALSE;
			katakana = FALSE;
			i+=3;
			skip++;
		}

		// Can't support JIS X 0212 at this time.

		// 2007.06.14 avoid buffer over run. kunikichi
		// 2008.07.08 fix wrong handling if last character is ascii char
		if ( doubleByte && i >= actualsize - 1 ) 
		{
			i = actualsize;
			break;
		}
		else if ( i > actualsize - 1 )  // fix wrong handling if last character is ascii char
		{
			i = actualsize;
			break;
		}

		if ( doubleByte )
		{
			// conversion logic of reg87 JIS X 0208-1983
			unsigned int	jtosj;
			jtosj = (unsigned int)(*(conv+i)<<8) + (unsigned int)*(conv+i+1);
			jtosj = _mbcjistojms( jtosj );
			*(conv+i-skip*3) = (jtosj & 0xFF00) >> 8; 
			*(conv+i+1-skip*3) = jtosj & 0x00FF;
			i++;
		}
		else
		{
			if ( katakana ) {
				// conversion logic of 
				*(conv+i-skip*3) = *(conv+i) + 128;
			} else {
				// as it is reg13 JIS X 0201-Katakana
				*(conv+i-skip*3) = *(conv+i);
			}
		}
		i++;
	}
	if ( skip != 0 )
	{
		*(conv+i-skip*3) = NULL;
	}
	return;
}

/** Conversion Function of SJIS to JIS
 * @param org      const char containing "SJIS" characters( ASCII also OK )
 * @param conv     char stored by this function with converted "JIS" characters
 */
void CJISToSJISMS::ConvertSJISToJIS(const std::string &org, std::string &conv)
{
	return;
}

/** Conversion Function of SJIS to JIS
 * @param org      const char containing "SJIS" characters( ASCII also OK )
 * @param conv     char stored by this function with converted "JIS" characters
 * @param bufsize  buffer size of conv (warn, JIS length will be longer than SJIS!)
 */
void CJISToSJISMS::ConvertSJISToJIS(const char *org, char *conv, int bufsize)
{
	return;
}

//#62 2013/07/30
bool isSQLSpecChar(char ch)
{
	bool ret_b = false;
	switch(ch){
//	case 0x27:	// "'"(シングルクォート) //->PxDB ToSQLString 払: J'  //
	case 0x25:	// "%"(パーセント)                棚: C*    //
	case 0x5F:	//"_"(アンダースコア)             敷: I_    //
	case 0x5B:	//"["(開き大カッコ)               布: I[    //
//	case 0x5D:	//"]"(閉じ大カッコ) //不要        怖: I]    //
//	case 0x5E:	//"^"(サーカムフレックス)//不要   扶: I^    //
//	case 0x5C:	//"\"(エンまたはバックスラッシュ：デフォルトエスケープ文字)//不要   府: I\ //
////
	case 0x2A:	//"*"                             奪: C%    //
	case 0x3F:	//"?"                             豹: I?    //

	ret_b = true;
	break;
	}
	return ret_b ;
}
/** Conversion Function of JIS to SJIS
 * @param org      const char containing "SJIS" characters( ASCII also OK )
 * @param conv     char stored by this function with converted "JIS" characters
 */
void CJISToSJISMS::ConvertSJToJCodeOnly(const std::string &org, std::string &conv)
{
	int ilimit = org.size();
	conv = org;

	for (int ipos = 0;ipos < ilimit;ipos++) {
		if ( _ismbblead(org[ipos]) != 0 && _ismbbtrail(org[ipos + 1]) != 0 ) {
			union {
			 unsigned int mbc;
			  struct {
				char ch;
				char cl;
			  } words;
			} sjtoj;
			
			sjtoj.words.cl = org[ipos];
			sjtoj.words.ch = org[ipos + 1];
			sjtoj.mbc = _mbcjmstojis( sjtoj.mbc );
			conv[ipos] = sjtoj.words.cl;
			conv[ipos + 1]=sjtoj.words.ch;
			ipos++;
		} else {
			conv[ipos]= org[ipos];
		}
	}

	return;
}

//#62 2013/07/30
void CJISToSJISMS::ConvertSJToJCodeOnlyForSQL(const std::string &org, std::string &conv)
{
	int ilimit = org.size();
	conv = org;

	std::string out_str_temp;
 
	for (int ipos = 0;ipos < ilimit;ipos++) {
		if ( _ismbblead(org[ipos]) != 0 && _ismbbtrail(org[ipos + 1]) != 0 ) {
			union {
			 unsigned int mbc;
			  struct {
				char ch;
				char cl;
			  } words;
			} sjtoj;
			
			sjtoj.words.cl = org[ipos];
			sjtoj.words.ch = org[ipos + 1];
			sjtoj.mbc = _mbcjmstojis( sjtoj.mbc );
			//
			if(isSQLSpecChar(sjtoj.words.cl)){
				out_str_temp += '[';
				out_str_temp += sjtoj.words.cl;
				out_str_temp += ']';
			}else{
		//		conv[ipos] = sjtoj.words.cl;
				out_str_temp += sjtoj.words.cl;
			}
			if(isSQLSpecChar(sjtoj.words.ch)){
				out_str_temp += '[';
				out_str_temp += sjtoj.words.ch;
				out_str_temp += ']';
			}else{
		//		conv[ipos + 1]=sjtoj.words.ch;
				out_str_temp += sjtoj.words.ch;
			}
			ipos++;
		} else {
	//		conv[ipos]= org[ipos];
			out_str_temp += org[ipos];
		}
	}

	conv = out_str_temp;

	return;
}

/** Reformat CString for DICOM Japanese PersonName Display standards
 * @param name      CString containing SJIS characters and converted
 *                  by this function with DICOM standards support 
 *                  algorithm. ( ASCII also OK )
 * @param type      Format Type( ex. "mike=東=ひがし" > "東[ひがし]" or "東" or "mike" )
 *                  not implemented.
 */
void CJISToSJISMS::ReformatPatientName(std::string &name, PNFormatType type) // for SJIS!
{
	int ipos = 0;
	bool bBreak = false;
	vector<std::string> vecElements;
	std::string strbyte, strdisp, strcall;

	// Tokenize
	while ( !bBreak ) {
		int inext = name.find("=", ipos);
		if ( inext == -1 ) {
			std::string kk = name.substr(ipos);
			vecElements.push_back(name.substr(ipos));			
			bBreak = true;
		} else {
			std::string ll = name.substr(ipos, inext - ipos);
			vecElements.push_back(name.substr(ipos, inext - ipos));
		}
		ipos = inext + 1;
	}
	
	// Check Type
	vector<std::string>::iterator itr = vecElements.begin();
	while( itr != vecElements.end() ) {
		unsigned int	sjtoj = 0;
		bool			b2byteproced = false;
		if ( itr->size() > 0 ) {
			if ( itr->size() > 1 ) {
				union {
				 unsigned int mbc;
				  struct {
					char ch;
					char cl;
				  } words;
				} sjtoj;
				
				sjtoj.words.cl = (*itr)[0];
				sjtoj.words.ch = (*itr)[1];
				sjtoj.mbc = _mbcjmstojis( sjtoj.mbc );

				// 2006.06.13 kunikichi  detection for kanji was wrong. fixed.
				if ( (0x2330 < sjtoj.mbc && sjtoj.mbc < 0x2576 ) || // ０ -> ヶ
					 (0x2A20 < sjtoj.mbc && sjtoj.mbc < 0x2A5F ) ) // ｡ -> ﾟ
				{ // It's Kanji
					strcall = *itr;
					b2byteproced = true;
				} else if ( sjtoj.mbc != 0 ) {
					strdisp = *itr;
					b2byteproced = true;
				// 2006.06.13 kunikichi  detection for kanji was wrong. fixed.
				} else if ('｡' < (*itr)[0] && (*itr)[0] < 'ﾟ' ) {
					strcall = *itr;
				}
			}
			if ( !b2byteproced ) {
//			if ( sjtoj == 0 && itr->GetAt(0) > 160 && itr->GetAt(0) < 224 ) {
				strbyte = *itr;
			}
		}
		itr++;
	}

	// Format Presentation
	name = "";
	if ( strdisp.size() > 0 ) {
		if ( strcall.size() > 0 ) {
			name = strdisp + "[" + strcall + "]";
		} else {
			name = strdisp;
		}
	} else if ( strcall.size() > 0 ) {
		name = strcall;
	} else {
		name = strbyte;
	}

	return;
}

/* 指定した文字データがShift-JISで記述された２バイト文字の１バイト目かどうか判定するマクロ（Builder-MLより借用） */

#define IsKanji(c)      ( (unsigned char)((int)((unsigned char)(c) ^ 0x20) - 0x0A1) < 0x3C )


 
//#1059 2011/02/21 By K.Ko
//
/** check the string whether is hakaku kataka
*   refuse the Reg13
 *   code:  0xA1 ,..., 0xDF.
 */
 
bool CJISToSJISMS::isHankakuKatakana(const char *sjis_str,int bufsize)//check Reg13	
{
	bool ret_b = false;

	bool sjis_2Byte_flag = false;
	for (int ipos = 0;ipos < bufsize;ipos++) {

		if(ipos <(bufsize-1)){
			//check the 2Bytes of SJIS
			if ( _ismbblead(sjis_str[ipos]) != 0 && _ismbbtrail(sjis_str[ipos + 1]) != 0 ) {

				sjis_2Byte_flag = true;
				ipos++;
			}else{
				sjis_2Byte_flag = false;
			}
		}else{
			sjis_2Byte_flag = false;
		}
		if(!sjis_2Byte_flag){
			//check the hankaku katakana
			unsigned char char_v = sjis_str[ipos];
			if( ( char_v >=0xA1 ) && ( char_v <= 0xDF ) ){
				ret_b = true;
				break;
			}
		}
	}
	return ret_b;
}