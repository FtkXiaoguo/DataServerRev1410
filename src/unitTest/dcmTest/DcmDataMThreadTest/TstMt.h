// TstStoreSCU.h: CTstStoreSCU クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
#define AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstDicomBase.h"

#include "rtvthread.h"

#include "MtThread.h"


#include "XSTRING"

using namespace std;


 
class CTstVLIDicomImage;
class CTstMt   :   public RTVThreadManager<CMtHandler>
{
public:
	CTstMt();
	virtual ~CTstMt();

//	void setupAE();

 
	void start(int id);

protected:	

	 
 

};

#endif // !defined(AFX_TSTSTORESCU_H__B30118BE_41D4_4425_A9D3_66371F5D4AF2__INCLUDED_)
