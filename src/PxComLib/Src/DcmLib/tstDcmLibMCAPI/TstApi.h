#pragma once

#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;

 
#define DiffFloat(d1,d2) ( ((d1-d2)*(d1-d2))>0.0000001)
class CTstApi
{
public:
	CTstApi(void);
	~CTstApi(void);

	void init();
	void release();
protected:
	IDcmLib *m_dcmlib_instance;

	int m_messageID;
};
