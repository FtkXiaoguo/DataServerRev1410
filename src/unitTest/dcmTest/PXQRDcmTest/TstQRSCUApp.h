// TstStoreSCU.h: CTstStoreSCU クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTSTORESCU_APP_H__B30YYYYY1F5D4AF2__INCLUDED_)
#define AFX_TSTSTORESCU_APP_H__B30YYYYY1F5D4AF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "TstDicomBase.h"
#include "TstQRSCU.h"

#include "XSTRING"

using namespace std;


 
class CTstVLIDicomImage;
class CTstQRSCUApp    : public CTstQRSCU
{
public:
	CTstQRSCUApp();
	virtual ~CTstQRSCUApp();

 
	virtual void updateStudyData() ;

	bool closeAll();

	bool loadOption(const char *fileName);

protected:	
#if 0
static bool SetValue ( int A_messageid, unsigned long A_tag,
                            const char *A_value, const char *A_default, 
                            bool A_required );
static bool GetValue ( int A_messageid, unsigned long A_tag, 
                            char *A_value, int A_size, char *A_default );
#endif


protected:
	///
	int m_beforeTime;
	int m_timeSpan;
	//


};

#endif // !defined(AFX_TSTSTORESCU_APP_H__B30YYYYY1F5D4AF2__INCLUDED_)
