// DcmXTAssociationMain
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DICOM_ASSOCIATION_H_)
#define AFX_DICOM_ASSOCIATION_H_
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#include "IDcmLib.h"
#include "DcmLocalString.h" 

using namespace XTDcmLib;


struct T_ASC_Network;
struct T_ASC_Parameters;
struct T_ASC_Association;
struct T_DIMSE_Message;
class AssociationHelpClient;
class AssociationHelpServer;

class DcmXTSCPResponseParam;
class DcmXTAssociationMain 
{
public:
	DcmXTAssociationMain();
	~DcmXTAssociationMain();
	
	 
	virtual bool destroy();
	bool abort();
 
	 
	 
//	virtual bool abortAssociation();
//	virtual bool releaseAssociation();
protected:
	bool initNetwork();
	 dcm_string	m_LocalAE;
	
//	T_ASC_Parameters *m_ASC_Params;
	int m_opt_acse_timeout;// = 30;

//	T_ASC_Association *m_association; //2012/02/13 K.Ko

//	T_DIMSE_Message *m_ReceivedMsg;

//	AssociationHelpClient	*m_clientHelper;
//	AssociationHelpServer	*m_serverHelper;

//	bool m_isServer;
static bool m_initWinSoket;
//
	DcmXTSCPResponseParam *m_ResponseParam;
	int m_AcceptableServiceIterator;
	int m_AcceptableServiceCounter;
//

	T_DIMSE_Message *m_ReceivedMsg;
};
 
#endif // !defined(AFX_DICOM_ASSOCIATION_H_)