// AqObjectBase.h: interface for the AqObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_AQ_OBJECT_BASE_H_)
#define _AQ_OBJECT_BASE_H_ "_AQ_OBJECT_BASE_H_"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AqBaseConstants.h"

class AqObjectBase  
{
public:
	AqObjectBase();
	virtual ~AqObjectBase();

	void AddRef(AqObjectBase *iDataOwner=0);   // Increase the reference counter on the data owner.

protected:
	virtual bool Release();						// Decrease the reference counter on the data owner. If 0, release buffer.	
private:
	// Data owner:
	AqObjectBase			*m_dataOwner;  // Points to the data owner who is going to release the buffer. Or NULL if this object owns the data.
	int						m_refCounter;  // Reference counter (used by data owner only).	
	
};

#endif // !defined(_AQ_OBJECT_BASE_H_)
