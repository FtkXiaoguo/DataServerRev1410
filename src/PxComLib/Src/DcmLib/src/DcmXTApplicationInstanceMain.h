// DcmXTApplicationInstanceMain
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DICOM_APPLICATION_INSTANCE_MAIN_H_)
#define AFX_DICOM_APPLICATION_INSTANCE_MAIN_H_
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#include <vector>

#include "IDcmLib.h"
#include "DcmLocalString.h" 

using namespace XTDcmLib;
 
typedef std::vector<dcm_string> DcmXTAEList ;
struct T_ASC_Network;

class DcmXTMutexMain;
class DcmXTApplicationInstanceMain  : public DcmXTApplicationInstance  
{
public:
	DcmXTApplicationInstanceMain();
	~DcmXTApplicationInstanceMain();

	virtual bool createNetWork( int portNum, int timeout=0);
	virtual bool destroyNetWork();
 
	virtual bool addAE(const char *ae);
	virtual bool removeAE(const char *ae);
	void Delete();

	T_ASC_Network *getSCPAscNetwork();
protected:

	bool openSCPAscNetwork();
	virtual void destroy();

 
//	DcmXTAEList m_AEList;

	T_ASC_Network	*m_SCP_ASC_NetWork; // for single listener !
	//
	bool m_modifiedFlag;
//	dcm_string m_AE;
	int m_portNum;
	int m_timeout;
//
	DcmXTMutexMain	*m_MultiThread_Mutex ;
};

 
#endif // !defined(AFX_DICOM_APPLICATION_INSTANCE_MAIN_H_)