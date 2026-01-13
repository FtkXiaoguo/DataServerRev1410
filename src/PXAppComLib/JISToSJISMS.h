/** 
 * @file  JISToSJISMS.h
 * @brief CJISToSJISMS Class Definition
 *
 * @author TeraRecon, Inc. <kuni@terarecon.co.jp>
 * @date 2004-02-03
 * @version $Id: JISToSJISMS.h 1707 2011-02-21 07:17:01Z furutsuki $
 *
 * Copyright (C) 2004 TeraRecon, Inc. All rights reserved.
 */
#if !defined(AFX_JISTOSJISMS_H__549FD47C_C014_442D_BF29_BD3A672E1C1E__INCLUDED_)
#define AFX_JISTOSJISMS_H__549FD47C_C014_442D_BF29_BD3A672E1C1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*----------------------------------------------------------------------
  Enum Definition
----------------------------------------------------------------------*/
typedef enum PNFormatType
{
	cPNStandard=1, 
	cPNShort,
	cPNAvoid2Byte
};
/*----------------------------------------------------------------------
  Class Definition
----------------------------------------------------------------------*/
/** 
 * @class CJISToSJISMS
 * @brief CJISToSJISMS Handling "JIS->SJIS" conversion, PatName formatting
 *
 * CJISToSJISMSクラスは
 * 1.JIS->SJISの変換
 * 2.DICOM PN規約（日本語患者名定義）の表現展開
 * を処理する
 */
#include <string>
class CJISToSJISMS  
{
public:
	// Constructor
	CJISToSJISMS();
	// Destuctor
	virtual ~CJISToSJISMS();
	// CString Version of JIS to SJIS Conversion
	static void ConvertJISToSJIS(const std::string &org, std::string &conv);
	// char Version of JIS to SJIS Conversion
	static void ConvertJISToSJIS(const char *org, char *conv, int bufsize);
	// CString Version of SJIS to JIS Conversion
	static void ConvertSJISToJIS(const std::string &org, std::string &conv);
	// char Version of SJIS to JIS Conversion
	static void ConvertSJISToJIS(const char *org, char *conv, int bufsize);
	// SJIS to JIS Conversion for 
	static void ConvertSJToJCodeOnly(const std::string &org, std::string &conv);
	// DICOM Standards Japanese PatientName formatting
	static void ReformatPatientName(std::string &name, PNFormatType type);
	//#1059 2011/02/21 By K.Ko
	static bool isHankakuKatakana(const char *sjis_str,int bufsize); //check Reg13

	//#62 2013/07/30
	static void ConvertSJToJCodeOnlyForSQL(const std::string &org, std::string &conv);
};

#endif // !defined(AFX_JISTOSJISMS_H__549FD47C_C014_442D_BF29_BD3A672E1C1E__INCLUDED_)
