// DcmXTAssociationMain
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DICOM_ASSOCIATION_LISTENER_H_)
#define AFX_DICOM_ASSOCIATION_LISTENER_H_

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)


#include "IDcmLib.h"
#include "DcmXTAssociationMain.h"
#include "DcmXTUtilMain.h"

using namespace XTDcmLib;


 

class AssociationHelpClient;
class AssociationHelpServer;

class DcmXTSCPResponseParam;

class DcmXTApplicationInstanceMain;

class DcmXTAssociationListenerMain : public DcmXTAssociationListener,  DcmXTAssociationMain
{
public:
	DcmXTAssociationListenerMain();
	~DcmXTAssociationListenerMain();
	 
	virtual void Delete() ;
	//
 
 
	virtual bool abort();
	//
	virtual bool waiting(const char*ServiceList,int portNum,int timeout,char *calledAE_Buff,int bufLen,DcmXtError &errorCode);
	//
	
	 
	//
	void setApplicationInstance(DcmXTApplicationInstanceMain *ap){m_ApplicationInstanceMain = ap;};

	void setAcceptServiceList(const DcmServiceListEntry *seriveEntry);

	T_ASC_Association *getAssociationTemp() { return m_associationTemp;};
protected:
	T_ASC_Association *m_associationTemp; // do not destroy it ,in this class
	 AssociationHelpServer	*m_serverHelper;

//	const DcmServiceListEntry *m_AcceptServiceList;
	 

	DcmXTApplicationInstanceMain *m_ApplicationInstanceMain;
//static T_ASC_Network	*m_ASC_NetWork; // for single listener !
};
 
 
#endif // !defined(AFX_DICOM_ASSOCIATION_LISTENER_H_)