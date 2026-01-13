// DcmXTAssociationClientMain
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DICOM_ASSOCIATION_CLIENT_H_)
#define AFX_DICOM_ASSOCIATION_CLIENT_H_
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)


#include "IDcmLib.h"
#include "DcmXTAssociationMain.h"
#include "DcmXTUtilMain.h"

using namespace XTDcmLib;


struct  T_ASC_Network;

class AssociationHelpClient;
class AssociationHelpServer;

class DcmXTSCPResponseParam;
 
class DcmXTAssociationClientMain : public DcmXTAssociationClient, DcmXTAssociationMain
{
public:
	DcmXTAssociationClientMain();
	~DcmXTAssociationClientMain();
	virtual bool isServer(){ return false;}
	virtual void Delete() ;
	//
	virtual void setLocalAE(const char *myAE) { m_LocalAE = myAE;};
	//
	virtual bool close();
	//
	virtual bool destroy();
	virtual bool abort();
	virtual bool open(const char *RemoteApplicationTitle,
						int          RemoteHostPortNumber,
						const char *RemoteHostTCPIPName,
						const char *ServiceList,
						DcmXtError &errorCode)  ;
	//
	
	virtual bool sendRequestMessage( DcmXTDicomMessage &message) ;
	
	//
	virtual DcmXTDicomMessage * readMessage(char *serviceNameBuff,int buffLen,unsigned short &command,DcmXtError &errorCode,int timeout=-1/*sec*/) ;
	 
	virtual bool getInfo(DcmXTAssociationInfo &info);
	virtual bool getFirstAcceptableService(DcmXTAssociationServiceInfo &serviceInfo,bool &endList);
	virtual bool getNextAcceptableService(DcmXTAssociationServiceInfo &serviceInfo,bool &endList);
	//
//	virtual bool abortAssociation();
//	virtual bool releaseAssociation();
	
	//
	void setProposeServiceList(const DcmServiceListEntry *seriveEntry){ m_ProposeServiceList = seriveEntry;};
	 
protected:
	T_ASC_Association *m_association; //2012/02/13 K.Ko

	AssociationHelpClient	*m_clientHelper;

	T_ASC_Parameters *m_ASC_Params;
	T_ASC_Network	*m_ASC_NetWork;
	unsigned int m_sentOrgMsgID;

	const DcmServiceListEntry *m_ProposeServiceList;
};
 
#endif // !defined(AFX_DICOM_ASSOCIATION_CLIENT_H_)