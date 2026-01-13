// TstDicomBase.h: CTstDicomBase クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTDICOMBASE_H__E4AC002E_9160_4DDF_996D_E5ED90CF5B6F__INCLUDED_)
#define AFX_TSTDICOMBASE_H__E4AC002E_9160_4DDF_996D_E5ED90CF5B6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "AqCore\TRLogger.h"
#include "XSTRING"
using namespace std;

class CTstDicomBase  
{
public:
	CTstDicomBase();
	virtual ~CTstDicomBase();


	bool initDcmTk();
	bool releaseDcmTk();

	bool openOutFile(const char *fileName);
	void closeOutFile();
static void ConvertSJISToJIS(string& iOrg, string& oConv);
protected:
	void outputMessage(char *format,...);
	void resetLineCount(){ m_lineCount = 0;};
	bool isLF(const char *formatstr);

	char m_messageBuff[1024];
	FILE *m_outFile;
	unsigned long m_lineCount;

	static TRLogger  m_Logger;
	
};

#endif // !defined(AFX_TSTDICOMBASE_H__E4AC002E_9160_4DDF_996D_E5ED90CF5B6F__INCLUDED_)
