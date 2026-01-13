// DcmXTUtilMain
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DICOM_UTIL_MAIN_H_)
#define AFX_DICOM_UTIL_MAIN_H_
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#include "IDcmLibApi.h"
#include "IDcmLib.h"

using namespace XTDcmLib;

#include "dcmtk/dcmdata/dcxfer.h"

/////////////////
#include <vector>
#include <map>

#if 0
typedef struct {
//	std::vector<std::string> SOPClassUIDs;
#define SOPClassUIDsMax (256)
	char *SOPClassUIDs[SOPClassUIDsMax];
	int SOPClassUIDsSize;
	E_TransferSyntax networkTransferSyntax;
	int	maxPDUSize;
} DcmServiceListEntry;
#else
class DcmServiceListEntry
{
//	std::vector<std::string> SOPClassUIDs;
#define SOPClassUIDsMax (256)
public:
	DcmServiceListEntry();
	~DcmServiceListEntry();
///
	char* *SOPClassUIDs;
	int SOPClassUIDsSize;
	E_TransferSyntax networkTransferSyntax;
	int	maxPDUSize;
};
#endif

typedef std::map <std::string , DcmServiceListEntry*> DcmServiceListMap;
///////////////

class DcmObject;
class DcmItem;
class DcmXfer;
class DcmElement;
class DcmTagKey;
 
class DcmXTUtilMain : public DcmXTUtil  
{
public:
	DcmXTUtilMain();
	~DcmXTUtilMain();
	 void Delete() ;
 
#if 0
static  void LoggerError(const char *format,...);
static  void LoggerWarn(const char *format,...);
static  void LoggerDebug(const char *format,...);
static  void LoggerTrace(const char *format,...);
#endif

static E_TransferSyntax  DcmXT2E_TransferSyntax(DcmXT_TransferSyntax in);
static DcmXT_TransferSyntax E2DcmXT_TransferSyntax(E_TransferSyntax in);
static DcmEVR ConvtMCVr2DcmEVr(MC_VR mc_vr);
static MC_VR ConvtDcmEVr2MCVr( DcmEVR vr);

virtual bool  getTansferSyntaxByName(const char*xferName,	DcmXTUtil::XferNames &ret) ;
virtual bool  getTansferSyntaxBySyntax(const DcmXT_TransferSyntax xfer,	DcmXTUtil::XferNames &ret) ;
virtual  char *getNameOfUID(const char* uid) ;
virtual  char *getUIDOfName(const char* name);
virtual const char *getTagName(unsigned long  tag);//2012/03/09 K.Ko
virtual bool Add_PrivateTag(unsigned short g,unsigned short e, MC_VR Avr,const char* AprivateCode);
virtual bool createServiceList(bool isPropose,const char *ServiceName);
virtual bool clearServiceList(bool isPropose,const char *ServiceName);
virtual bool setNetworkTransferSyntax(bool isPropose,DcmXT_TransferSyntax xfer,const char *ServiceName);
virtual bool addSOPClassUID(bool isPropose,const char * sopUID,const char *ServiceName);
virtual bool setMaxPDUSize(bool isPropose,int size,const char *ServiceName) ;//2012/03/09 K.Ko
//
//virtual bool createAcceptServiceList(const char *ServiceName);
//virtual bool addAcceptSOPClassUID(const char * sopUID,const char *ServiceName);

virtual void  DcmAPIError(const char* name);

static bool AddPrivateCreator2GDict(unsigned short g,const char*    AprivateCode,unsigned short e = 0x0010);
static  bool AddPrivateTag2GDict(unsigned short g,unsigned short e, MC_VR Avr,const char* AprivateCode);
//
DcmServiceListEntry *getServiceList(std::string name,bool Propose);


//void openLogger(const char *fileName,IDcmLib::LOG_LEVEL log_level);
static void setupApiLogger(XTDcmLib::DcmLibApiLogger *logger,IDcmLib::LOG_LEVEL log_level);
static XTDcmLib::DcmLibApiLogger *getApiLogger() ;
//#18 2012/05/17 K.Ko
 static void setupWriteBufferLen(int len);
 static void setupReadeBufferLen(int len);
 static int getWriteBufferLen();
 static int getReadeBufferLen();

static IDcmLib::LOG_LEVEL getLogLevel()  { return m_logLevel;};
#if 0
static std::string getLogPrefix();
static bool isEnableAPILog();
#endif

protected:

void destroy();

void initDefaultServiceList(const std::string &newName,bool Propose);

DcmServiceListMap m_ProposeServiceListMap;
DcmServiceListMap m_AcceptServiceListMap;
//
static IDcmLib::LOG_LEVEL m_logLevel;
 
};

class OFMutex;
class DcmXTMutexMain : public DcmXTMutex
{
public:
	DcmXTMutexMain();
	~DcmXTMutexMain();
virtual int lock()  ;
virtual int unlock() ;
protected:
  OFMutex *m_mutex;
};
 
#endif // !defined(AFX_DICOM_UTIL_MAIN_H_)