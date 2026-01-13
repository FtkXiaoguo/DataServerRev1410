// TstCMoveSCU.h: CTstCMoveSCU クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTCMOVESCU_H__5288F36C_D0ED_4DFF_831D_9E76F6422145__INCLUDED_)
#define AFX_TSTCMOVESCU_H__5288F36C_D0ED_4DFF_831D_9E76F6422145__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstQRSCU.h"

class CTstCMoveSCU : public CTstQRSCU  
{
public:
	CTstCMoveSCU();
	virtual ~CTstCMoveSCU();

	bool sendCMoveCmd();
};

#endif // !defined(AFX_TSTCMOVESCU_H__5288F36C_D0ED_4DFF_831D_9E76F6422145__INCLUDED_)
