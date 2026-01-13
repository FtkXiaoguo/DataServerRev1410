// TstVLIDicomMessage.h: CTstVLIDicomMessage クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTVLIDICOMMESSAGE_H__D4E1C3FC_77D8_4B13_92DF_2C1BBA2CC63D__INCLUDED_)
#define AFX_TSTVLIDICOMMESSAGE_H__D4E1C3FC_77D8_4B13_92DF_2C1BBA2CC63D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TstDicomBase.h"

#ifdef USE_NEW_LIB
class CPxDicomMessage;
#else
class VLIDicomMessage;
#endif

class CTstVLIDicomMessage   : public CTstDicomBase
{
public:
	CTstVLIDicomMessage();
	virtual ~CTstVLIDicomMessage();

	bool loadDicom(const char *filename,bool bHeaderOnly=0 );
	void saveDicom(const char *filename);
protected:
#ifdef USE_NEW_LIB
 
	CPxDicomMessage *m_DicomImage;
#else
	VLIDicomMessage *m_DicomImage;
 
#endif
};

#endif // !defined(AFX_TSTVLIDICOMMESSAGE_H__D4E1C3FC_77D8_4B13_92DF_2C1BBA2CC63D__INCLUDED_)
