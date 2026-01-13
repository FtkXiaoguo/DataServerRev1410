// TstDicomBase.cpp: CTstDicomBase クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TstDicomBase.h"

#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#else
#include "VLIDicomImage.h"
#endif

#include <mbstring.h>
#include <vector>
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
TRLogger CTstDicomBase::m_Logger;

CTstDicomBase::CTstDicomBase()
{
	m_outFile = 0;
}

CTstDicomBase::~CTstDicomBase()
{
	closeOutFile();
}
bool CTstDicomBase::initDcmTk()
{
	if(!TRDICOMUtil::InitialDICOM("DBA6-5B453") )
		return false;

	return true;
}
bool CTstDicomBase::releaseDcmTk()
{
	TRDICOMUtil::ReleaseDICOM();
	

	return true;
}
bool CTstDicomBase::openOutFile(const char *fileName)
{
	closeOutFile();
	resetLineCount();
	m_messageBuff[0] = 0;
	m_outFile = fopen(fileName,"wt");
	
	outputMessage(" \n");
	return true;
}
void CTstDicomBase::closeOutFile()
{
	if(m_outFile){
		fclose(m_outFile);
		m_outFile = 0;
	}
}
bool CTstDicomBase::isLF(const char *formatstr)
{
	bool ret_val = false;
	int str_size = strlen(formatstr);
	if(str_size <1) return false;

	const char *str_ptr = formatstr;
	str_ptr += (str_size-1); // to end
	for(int i=0;i<str_size;i++){
		if((*str_ptr--)=='\n'){
			ret_val = true;
			break;
		}
	}
	return ret_val;
}
void  CTstDicomBase::outputMessage(char *format,...)
{
	va_list ap;

	va_start( ap, format );


	bool new_line_flag = (m_messageBuff[0] != 0);

	bool LF_flag = isLF(format);

	if(m_outFile){
		if(new_line_flag){
			fprintf(m_outFile,"%s",m_messageBuff);
			m_messageBuff[0] = 0;
		}
 		vfprintf(m_outFile,format, ap );
		 
		fflush(m_outFile);
	}else{
		printf("%s",m_messageBuff);
 		vprintf(format, ap );
	}

	//for next
	if(LF_flag){
		sprintf(m_messageBuff,"line:%08d: ",m_lineCount++);
	};

	va_end( ap );
}
void CTstDicomBase::ConvertSJISToJIS(string& org, string& conv)
{

	//
	char char_buff[4];
	int ilimit = org.size();
	conv = org;

	string conv_sjis_temp;
	string conv_ascii_temp;
	string conv_ret_val;
	conv_ret_val.erase();

	string sjis_str_begin	= "\033$B";
	string sjis_str_end		= "\033(B";

	bool sjis_cnv_flag = false;
 
	conv_sjis_temp.erase();
	conv_ascii_temp.erase();

	for (int ipos = 0;ipos < ilimit;ipos++) {
		if ( _ismbblead(org.at(ipos)) != 0 && _ismbbtrail(org.at(ipos + 1)) != 0 ) {

			if(!sjis_cnv_flag){
			//sjis start
				conv_ret_val = conv_ret_val + conv_ascii_temp;//previous
				conv_ascii_temp.erase();
				conv_ret_val = conv_ret_val + sjis_str_begin;
				conv_sjis_temp.erase();
				sjis_cnv_flag = true;
			}

			union {
			 unsigned int mbc;
			  struct {
				char ch;
				char cl;
			  } words;
			} sjtoj;
			
			sjtoj.words.cl = org.at(ipos);
			sjtoj.words.ch = org.at(ipos + 1);
			sjtoj.mbc = _mbcjmstojis( sjtoj.mbc );

//			conv.SetAt(ipos, sjtoj.words.cl);
			char_buff[0]= sjtoj.words.cl;
			char_buff[1]=0;
			conv_sjis_temp.append(char_buff);
//			conv.replace(ipos, 1, char_buff);
//			conv.SetAt(ipos + 1, sjtoj.words.ch);
			char_buff[0]= sjtoj.words.ch;
			char_buff[1]=0;
//			conv.replace(ipos + 1, 1, char_buff);
			conv_sjis_temp.append(char_buff);
			ipos++;
		} else {
			if(sjis_cnv_flag){
				//sjis end
				if(conv_sjis_temp.size()>0){
					conv_ret_val = conv_ret_val + conv_sjis_temp; 
					conv_ret_val = conv_ret_val + sjis_str_end;
					conv_sjis_temp.erase();
				}
				//prepare for ASCII　start
				conv_ascii_temp.erase();
				sjis_cnv_flag = false;
			}
			
//			conv.SetAt(ipos, org.at(ipos));
			char_buff[0]= org.at(ipos);
			char_buff[1]=0;
	//		conv.replace(ipos, 1, char_buff);
			conv_ascii_temp.append(char_buff);
		}
	}
	//last
	if(conv_sjis_temp.size()>0){
		conv_ret_val = conv_ret_val + conv_sjis_temp; 
		conv_ret_val = conv_ret_val + sjis_str_end;
		conv_sjis_temp.erase();
	}
	if(conv_ascii_temp.size()>0){
		conv_ret_val = conv_ret_val + conv_ascii_temp; 
		conv_ascii_temp.erase();
	}
	conv = conv_ret_val;
}