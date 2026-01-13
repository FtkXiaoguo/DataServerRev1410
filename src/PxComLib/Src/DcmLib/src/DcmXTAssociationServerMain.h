// DcmXTAssociationServerMain
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DICOM_ASSOCIATION_SERVER_H_)
#define AFX_DICOM_ASSOCIATION_SERVER_H_
 
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
class DcmXTAssociationServerMain : public DcmXTAssociationServer,  DcmXTAssociationMain
{
public:
	DcmXTAssociationServerMain(T_ASC_Association *aso);
	~DcmXTAssociationServerMain();

	 

	virtual bool isServer(){ return true;};
	virtual void Delete() ;
	//
	virtual void setLocalAE(const char *myAE) { m_LocalAE = myAE;};
	//
 
	virtual bool destroy();
	virtual bool abort();
	//
//chg	virtual bool waiting(const dcm_string &ServiceList,int portNum,int timeout,dllString &calledAE,DcmXtError &errorCode);
	//
	
	virtual bool sendResponseMessage(RESP_STATUS    ResponseStatus,const DcmXTDicomMessage *message) ;
	//
	virtual DcmXTDicomMessage * readMessage(char *serviceNameBuff,int buffLen,unsigned short &command,DcmXtError &errorCode,int timeout=-1/*sec*/) ;
	 
	virtual bool getInfo(DcmXTAssociationInfo &info);
	virtual bool reject( DcmXT_Reject_Reason reason) ;
	virtual bool accept()  ;
	//
//	virtual bool abortAssociation();
//	virtual bool releaseAssociation();
	//
	void setApplicationInstance(DcmXTApplicationInstanceMain *ap){m_ApplicationInstanceMain = ap;};

	void setAcceptServiceList(const DcmServiceListEntry *seriveEntry);
protected:
	T_ASC_Association *m_association; //2012/02/13 K.Ko

	AssociationHelpServer	*m_serverHelper;
	 
 	const DcmServiceListEntry *m_AcceptServiceList;
//	const char ** m_CurStorageSOPClassUID;
//	int m_CurStorageSOPClassUIDSize;
//	void destroyCurStorageSOPClassUID();

	DcmXTApplicationInstanceMain *m_ApplicationInstanceMain;
//static T_ASC_Network	*m_ASC_NetWork; // for single listener !
};
 
#endif // !defined(AFX_DICOM_ASSOCIATION_SERVER_H_)