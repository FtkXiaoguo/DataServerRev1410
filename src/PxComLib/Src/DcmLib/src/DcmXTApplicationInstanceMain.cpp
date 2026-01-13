//  
//
//////////////////////////////////////////////////////////////////////

#include "DcmXTApplicationInstanceMain.h"

//////////////////
 #include "dcmtk/ofstd/ofstdinc.h"

#include "AssociationHelp.h"

#include "CheckMemoryLeak.h"

 DcmXTApplicationInstanceMain::DcmXTApplicationInstanceMain()
{
	 m_portNum = -1;
m_SCP_ASC_NetWork = 0;
m_modifiedFlag = false;
}
DcmXTApplicationInstanceMain::~DcmXTApplicationInstanceMain()
{
}
void DcmXTApplicationInstanceMain::Delete()
{
	destroy();
	delete this;
} 
void  DcmXTApplicationInstanceMain::destroy()
{
	DcmLibMTLock lock;

	if(m_SCP_ASC_NetWork) {
		ASC_dropNetwork(&m_SCP_ASC_NetWork);
		m_SCP_ASC_NetWork = 0;
	}
 
	 dcmDataDict.clear() ;

	 //
#if 0
	 m_AEList.clear();
#endif
	 
}

bool DcmXTApplicationInstanceMain::destroyNetWork()
{
	DcmLibMTLock lock;

	if(m_SCP_ASC_NetWork) {
		ASC_dropNetwork(&m_SCP_ASC_NetWork);
		m_SCP_ASC_NetWork = 0;
	}
	m_modifiedFlag = false;
	return true;
}

T_ASC_Network *DcmXTApplicationInstanceMain::getSCPAscNetwork() 
{ 
	DcmLibMTLock lock;

	openSCPAscNetwork();
	return m_SCP_ASC_NetWork;
};
bool DcmXTApplicationInstanceMain::createNetWork( int portNum, int timeout )
{
	DcmLibMTLock lock;

	if(portNum>0){
		if(m_portNum != portNum){
			m_portNum	= portNum;
			m_modifiedFlag = true;
		}
	}
	m_timeout	= timeout;
//	m_modifiedFlag = true;

	return true;
}
bool DcmXTApplicationInstanceMain::openSCPAscNetwork()
{
	DcmLibMTLock lock;

	if(m_modifiedFlag){
		OFCondition cond = ECC_Normal;
		if(m_SCP_ASC_NetWork) {
			cond = ASC_dropNetwork(&m_SCP_ASC_NetWork);
			m_SCP_ASC_NetWork = 0;
		}

		
		cond = ASC_initializeNetwork(NET_ACCEPTOR, OFstatic_cast(int, m_portNum), m_timeout, &m_SCP_ASC_NetWork);
		if (cond.bad())
		{
			DimseCondition::dump(cond);
			return false;
		}
		
		m_modifiedFlag = false;
	}

	return true;

}

 bool DcmXTApplicationInstanceMain::addAE(const char *ae)
{
#if 0
	// if you want to use m_AEList , you have to use lock / unlock for multithread
	 m_AEList.push_back(ae);
#endif
	 return true;
}
 bool DcmXTApplicationInstanceMain::removeAE(const char *ae)
{
#if 0
	DcmXTAEList::iterator it = m_AEList.begin();
	while(it!=m_AEList.end()){
		if((*it)==ae ){
			m_AEList.erase(it);
			break;
		}
		it++;
	}
#endif
	 return true;
}