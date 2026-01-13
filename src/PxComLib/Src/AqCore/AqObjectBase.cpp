// AqObjectBase.cpp: implementation of the AqObjectBase class.
//
//////////////////////////////////////////////////////////////////////

#include "AqObjectBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AqObjectBase::AqObjectBase()
{
	m_dataOwner = 0;
	m_refCounter = 1;
}

AqObjectBase::~AqObjectBase()
{
	
}

//================================================================================
void AqObjectBase::AddRef(AqObjectBase* iDataOwner)
{
	if (iDataOwner) {
		m_dataOwner = iDataOwner;	
		m_dataOwner->m_refCounter++;
	}
	else
	{
		m_refCounter++;
	}
}

// protected:


//================================================================================
bool AqObjectBase::Release()
{
	if (m_dataOwner)  // Data is owned by somebody else
	{
		// Announce the release to the owner
		m_dataOwner->Release();
		
	}
	else  // I own the data
	{
		
		// Decrease the counter:
		m_refCounter--;
		if (m_refCounter > 0)
		{
			return 0;
		}
		assert(m_refCounter<=0);
	}
	return (m_refCounter==0);
}

