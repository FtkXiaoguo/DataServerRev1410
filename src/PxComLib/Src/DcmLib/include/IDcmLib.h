// IDcmLib.h: DcmLib クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_H__TYTT_)
#define AFX_IDICOM_LIB_H__TYTT_
 

#include "IDcmLibCom.h"
#include "IDcmLibMsg.h"
#include  "diction.h"

#ifdef MakeIDcmLib 
	#define IDcmLibDefDllCls __declspec(dllexport)
	 
#else 
	#define IDcmLibDefDllCls __declspec(dllimport)
	 
#endif


#pragma warning (disable : 4231)
#pragma warning (disable : 4275)
#pragma warning (disable : 4251)

//#include <string>
//#include <map>

 

/////////////////////////////////////////////////////////
/// for string STL

IDcmLibDefDllCls  void*   DllAllocate(size_t size);
IDcmLibDefDllCls  void    DllFree(void* ptr);


template < class T > struct DllAllocator : public std::allocator<T>
{
    template <class U> struct rebind    { typedef DllAllocator<U> other; };
    DllAllocator () throw() {}
 //   DllAllocator (const DllAllocator&) throw () {}
    template <class U> DllAllocator(const DllAllocator<U>&) throw() {}
    template <class U> DllAllocator& operator=(const DllAllocator<U>&) throw()  {}
    // allocate
    pointer allocate(size_type c, const void* hint = 0)
    {
        return static_cast<pointer>(DllAllocate(sizeof(T) * c));
    }
    // deallocate
    void deallocate(pointer p, size_type n)
    {
        DllFree(p);
    }
};

//typedef std::basic_string<char, std::char_traits<char>, DllAllocator<char> >    dllString;
 
//typedef std::vector<int, DllAllocator<int> >    IntVector;

//#define dcm_string dllString
///////////////////////////////////////////////////
// please use 
//  dllString in getXXXXX method to return string 
// DO NOT USE std::string !
//
///////////////////////////////////////////////////

namespace XTDcmLib
{
	
/* !
* 共通インタフェース
*/

typedef enum {
    DcmXT_EXS_Unknown = -1,
    DcmXT_EXS_LittleEndianImplicit = 0,
    DcmXT_EXS_BigEndianImplicit = 1,
    DcmXT_EXS_LittleEndianExplicit = 2,
    DcmXT_EXS_BigEndianExplicit = 3,
    DcmXT_EXS_JPEGProcess1TransferSyntax = 4,
    DcmXT_EXS_JPEGProcess2_4TransferSyntax = 5,
    DcmXT_EXS_JPEGProcess3_5TransferSyntax = 6,
    DcmXT_EXS_JPEGProcess6_8TransferSyntax = 7,
    DcmXT_EXS_JPEGProcess7_9TransferSyntax = 8,
    DcmXT_EXS_JPEGProcess10_12TransferSyntax = 9,
    DcmXT_EXS_JPEGProcess11_13TransferSyntax = 10,
    DcmXT_EXS_JPEGProcess14TransferSyntax = 11,
    DcmXT_EXS_JPEGProcess15TransferSyntax = 12,
    DcmXT_EXS_JPEGProcess16_18TransferSyntax = 13,
    DcmXT_EXS_JPEGProcess17_19TransferSyntax = 14,
    DcmXT_EXS_JPEGProcess20_22TransferSyntax = 15,
    DcmXT_EXS_JPEGProcess21_23TransferSyntax = 16,
    DcmXT_EXS_JPEGProcess24_26TransferSyntax = 17,
    DcmXT_EXS_JPEGProcess25_27TransferSyntax = 18,
    DcmXT_EXS_JPEGProcess28TransferSyntax = 19,
    DcmXT_EXS_JPEGProcess29TransferSyntax = 20,
    DcmXT_EXS_JPEGProcess14SV1TransferSyntax = 21,
    DcmXT_EXS_RLELossless = 22,
    DcmXT_EXS_JPEGLSLossless = 23,
    DcmXT_EXS_JPEGLSLossy = 24,
    DcmXT_EXS_DeflatedLittleEndianExplicit = 25,
    DcmXT_EXS_JPEG2000LosslessOnly = 26,
    DcmXT_EXS_JPEG2000 = 27,
    DcmXT_EXS_MPEG2MainProfileAtMainLevel = 28,
    DcmXT_EXS_JPEG2000MulticomponentLosslessOnly = 29,
    DcmXT_EXS_JPEG2000Multicomponent = 30
} DcmXT_TransferSyntax;

 
/* ======================================================================== *
 *               Association Rejection Reasons Enumerated Type              *
 * ======================================================================== *
 *  Calls to MC_Reject_Association must use this enumerated type to         *
 *   specify the reason why the association is being rejected.  Note that   *
 *   not all of the potential rejection reasons are listed because they are *
 *   automatically handled by the tool kit.                                 */
typedef enum {
        DcmXt_PERMANENT_NO_REASON_GIVEN,
        DcmXt_TRANSIENT_NO_REASON_GIVEN, 
        DcmXt_PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED,
        DcmXt_TRANSIENT_TEMPORARY_CONGESTION,
        DcmXt_TRANSIENT_LOCAL_LIMIT_EXCEEDED
} DcmXT_Reject_Reason;  


typedef enum {
	DcmXtErr_Unknown=-1,
	DcmXtErr_Normal,
	DcmXtErr_Timeout,
	DcmXtErr_AssociatioinClosed, 
	DcmXtErr_AssociatioinAborted, 
	DcmXtErr_AssociatioinRejected,
	DcmXtErr_NewWorkShutDown, 
} DcmXtError;

class DcmXTMessageCallback
{
public:
	enum CallbackType {
		CB_DataLength,
		CB_Data,
		CB_OffsetTable,
		CB_MediaDataLength,
		CB_RequestForDataLength,
		CB_RequestForData,
	};
virtual bool readPixelData(int msgID, CallbackType CBtype,
										unsigned long* dataSizePtr,void** dataBufferPtr,
										int isFirst,int* isLastPtr) = 0;
};

class DcmXTComInterface
{
public:
	
	virtual bool DeleteTag(unsigned long  tag) = 0;
	virtual bool Set_Value(unsigned long  tag,	unsigned short val,bool append=false,const char *privateTag=0)		=0;
	virtual bool Set_Value(unsigned long  tag,	short val,bool append=false,const char *privateTag=0)				=0;
	//
	virtual bool Set_Value(unsigned long  tag,	unsigned int val,bool append=false,const char *privateTag=0)		=0;
	virtual bool Set_Value(unsigned long  tag,	int val,bool append=false,const char *privateTag=0)				=0;
	//
	virtual bool Set_Value(unsigned long  tag,	unsigned long val,bool append=false,const char *privateTag=0)		=0;
	virtual bool Set_Value(unsigned long  tag,	long val,bool append=false,const char *privateTag=0)				=0;
	//
	virtual bool Set_Value(unsigned long  tag,	float val,bool append=false,const char *privateTag=0)				=0;
	virtual bool Set_Value(unsigned long  tag,	double val,bool append=false,const char *privateTag=0)				=0;
	//
	// change string to char * 2012/02/16 K.Ko
	//virtual bool Set_Value(unsigned long  tag,	const dcm_string &val,bool append=false,const char *privateTag=0)=0;
	virtual bool Set_Value(unsigned long  tag,	const char *str_val,bool append=false,const char *privateTag=0)=0;
	
	//
	virtual bool Set_ArrayValue(unsigned long  tag,const void*dataBuff,int dataSize)	=0;
	//
	virtual bool Set_ValueToNull(unsigned long  tag )	=0;
	///
	//##35 2012/09/16 K.Ko
	virtual bool Begin_Sequence(unsigned long  tag )	=0;
	virtual bool End_Sequence(unsigned long  tag )	=0;

	/////
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned short &val)	=0;
	virtual MC_STATUS Get_Value(unsigned long  tag,	short &val)				=0;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned int &val)		=0;
	virtual MC_STATUS Get_Value(unsigned long  tag,	int &val)				=0;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned long &val)		=0;
	virtual MC_STATUS Get_Value(unsigned long  tag,	long &val)				=0;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	float &val)				=0;
	virtual MC_STATUS Get_Value(unsigned long  tag,	double &val)			=0;
	// change string to char * 2012/02/16 K.Ko
//	virtual MC_STATUS Get_Value(unsigned long  tag,	dllString &val,bool Sequence=false)=0;
	virtual MC_STATUS Get_Value(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence=false)=0;

	virtual MC_STATUS Get_ValueCount(unsigned long  tag,	int&val)=0; //for Val_1\Val_2\Val_3...
	// change string to char * 2012/02/16 K.Ko
//	virtual MC_STATUS Get_ValueNext(unsigned long  tag,	dllString &val)=0; //for Val_1\Val_2\Val_3...
	virtual MC_STATUS Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len)=0; //for Val_1\Val_2\Val_3...


	virtual MC_STATUS Get_ValueLength(unsigned long  tag, int num,	unsigned long &val)=0;
	//
	virtual MC_STATUS Get_PixelOffset(unsigned long &val)=0;
	virtual MC_STATUS Get_FileLength(unsigned long &val)=0;
	//
	virtual MC_STATUS Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues)=0;
 
};
///
class DcmXTSequence : public DcmXTComInterface
{
public:
	virtual void Delete() = 0;
//	virtual bool readFile(const dcm_string & fileName) = 0;
//
	 
};

class DcmXTMetaHeader : public DcmXTComInterface
{
public:
	virtual void Delete() = 0;
//	virtual bool readFile(const dcm_string & fileName) = 0;
//

};
class DcmXTDataSet : public DcmXTComInterface
{
public:
	virtual void Delete() = 0;
//	virtual bool readFile(const dcm_string & fileName) = 0;
//
	virtual DcmXTDataSet* getFirstSeqDataSet(unsigned long  tag) = 0;
	virtual DcmXTDataSet* getNextSeqDataSet() const = 0;
};
//
class DcmXTDicomMessage : public DcmXTComInterface
{
public:
	virtual void Delete() = 0;
	virtual DcmXTDicomMessage *clone()=0;
	// change string to char * 2012/02/16 K.Ko
	virtual bool openFile(const char * fileNam)=0;
	virtual void setMaxReadLength( long maxLen) = 0;
	virtual bool readFile()=0;
	virtual bool readFile(const char * fileName) = 0;
	virtual bool writeFile(const char *fileName) = 0;
	virtual bool writeFile( ) = 0;
	virtual void setFileName(const char * fileName) = 0;
	virtual bool readFromDumpFile(const char * dumpFileName)	= 0;
	virtual bool writeToDumpFile(const char * dumpFileName)	= 0;
//	virtual bool readFromXMLFile(const dcm_string & XMLFileName) = 0;
//
	virtual bool setID(unsigned int id)=0;
	virtual bool getID(unsigned int &id) const =0;

	//special tag for response message
	virtual bool getMessageIDFromRsp(unsigned int &msgID) = 0;
	virtual bool getStatusFromRsp(unsigned int &status) = 0;
//
	virtual bool close() =0 ;
	virtual bool openMessage(const char *  serviceName,	MC_COMMAND command=INVALID_COMMAND) = 0;
	virtual bool setupCommand(const char *  serviceName,	MC_COMMAND command ) = 0;
	virtual bool open()=0;
//
	virtual bool Set_ArrayValue(unsigned long  tag,	const void*dataBuff,int dataSize)=0;
//
	virtual bool Set_Callback( DcmXTMessageCallback *callback)=0;
	virtual bool Read_OBData( DcmXTMessageCallback *callback,unsigned long tag=0x7FE00010)=0;
//
	virtual bool Set_TransferSyntax(DcmXT_TransferSyntax Xfer)=0;
	virtual bool Get_TransferSyntax(DcmXT_TransferSyntax &Xfer)=0;
	//
	virtual DcmXTMetaHeader *getDcmXTMetaHeader() const = 0;
	virtual DcmXTDataSet *getDcmXTDataSet() const = 0;
	//
	virtual bool Set_ServiceName(const char *AserviceUID, MC_COMMAND serviceCm) = 0 ;
//	virtual bool Get_ServiceName(dllString  &AserviceUID, MC_COMMAND &serviceCm)  const = 0;
	virtual bool Get_ServiceName(char *AserviceUID_Buff,int bufLen, MC_COMMAND &serviceCm)  const = 0;
//

	//
	virtual bool Add_PrivateBlock(unsigned short ,const char*    AprivateCode)  = 0; 
	virtual bool Add_PrivateTag(unsigned short g,unsigned short e, MC_VR Avr,const char*    AprivateCode)  = 0; 
};



#if 0
class DcmXTAssociationInfo
{
public:	
    int     NumberOfProposedServices;        /* From service list */
    int     NumberOfAcceptableServices;      /* Acceptable to both sides */
    char    RemoteApplicationTitle[32+2] ;      /* 16-characters max */
    char	RemoteHostName[128+2] ;              /* Network node name
                                                64-characters max*/
    int     Tcp_socket;                      /* TCP Socket used for
                                                association */
    char	RemoteIPAddress[128+2] ;             /* Network IP Address */
    char	LocalApplicationTitle[32+2] ;       /* 16-characters max */
    char	RemoteImplementationClassUID[128+2] ;/* 64-characters max */
    char	RemoteImplementationVersion[32+2] ; /* 16-characters max */
//
	unsigned long LocalMaximumPDUSize;   
	unsigned long RemoteMaximumPDUSize;
	unsigned short MaxOperationsInvoked;     /* Negotiated Max operations
                                              * invoked by the assoc requestor */
	unsigned short MaxOperationsPerformed;   /* Negotiated Max operations
                                              * performed by the assoc requestor */
}  ;
#else
typedef struct MC_Assoc_Info DcmXTAssociationInfo ;
#endif


//ref: MC_Service_Info
class DcmXTAssociationServiceInfo 
{
public:
    char  ServiceName[64+2];        /* MergeCOM-3 Service Name */
    char  SyntaxTypeUID[128+2];        /* Transfer syntax negotiated 
                                            for the service */
    int       RoleNegotiated;         /* The role negotiated for the 
                                               service */
	int		 PresentationContextID;
} ;
#define ASServiceInfo_ServiceName(dest,src) strncpy(dest,src,64)
#define ASServiceInfo_SyntaxTypeUID(dest,src) strncpy(dest,src,128)

class DcmXTMutex
{
public:
	virtual int lock() = 0;
	virtual int unlock() = 0;
};

class DcmXTAssociation 
{
public:
	virtual bool isServer() = 0;
	virtual void Delete()	= 0;
	virtual bool abort()	= 0;
	virtual bool getInfo(DcmXTAssociationInfo &info) = 0;
	virtual DcmXTDicomMessage * readMessage( char *serviceNameBuff,int buffLen,unsigned short &command,DcmXtError &errorCode,int timeout=-1/*sec*/)=0;
};
class DcmXTAssociationClient  : public DcmXTAssociation
{
public:
 
	virtual void setLocalAE(const char *myAE) = 0;
	 
	virtual bool open(const char *RemoteApplicationTitle,
						int          RemoteHostPortNumber,
						const char *RemoteHostTCPIPName,
						const char *ServiceList,
						DcmXtError &errorCode ) = 0;
	//
	virtual bool close()	= 0;
	virtual bool sendRequestMessage( DcmXTDicomMessage &message)=0;
	//
 
	virtual bool getFirstAcceptableService(DcmXTAssociationServiceInfo &serviceInfo,bool &endList) = 0;
	virtual bool getNextAcceptableService(DcmXTAssociationServiceInfo &serviceInfo,bool &endList) = 0;
	 
};
//
class DcmXTAssociationServer  : public DcmXTAssociation
{
public:
 
//chg	virtual bool waiting(const dcm_string &ServiceList,int portNum,int timeout,dllString &calledAE,DcmXtError &errorCode)=0;
	 
	virtual bool sendResponseMessage(RESP_STATUS    ResponseStatus,const DcmXTDicomMessage *message)=0;
	//
	//
	virtual bool reject( DcmXT_Reject_Reason reason) = 0;
	virtual bool accept() = 0;
 
};

class DcmXTAssociationListener  
{
public:
 
	virtual bool waiting(const char *ServiceList,int portNum,int timeout,char *calledAE_Buff,int bufLen,DcmXtError &errorCode)=0;
	 
	virtual void Delete()=0;
	//
	virtual bool abort()=0;

#if 0
	virtual bool sendResponseMessage(RESP_STATUS    ResponseStatus,const DcmXTDicomMessage *message)=0;
	//
	//
	virtual bool reject( DcmXT_Reject_Reason reason) = 0;
	virtual bool accept() = 0;
#endif
 
};

class DcmXTApplicationInstance
{
public:

	virtual bool createNetWork( int portNum, int timeout=0)=0;
	virtual bool destroyNetWork()=0;
	virtual bool addAE(const char *ae)=0;
	virtual bool removeAE(const char *ae)=0;
};

class DcmXTUtil   
{
public:
	class XferNames{
	public:
		char xferID[256];
		char xferName[256];
		DcmXT_TransferSyntax xfer;
	};
//

virtual bool  getTansferSyntaxByName(const char*xferName,	DcmXTUtil::XferNames &ret) = 0;
virtual bool  getTansferSyntaxBySyntax(const DcmXT_TransferSyntax xfer,	DcmXTUtil::XferNames &ret) = 0;
virtual char *getNameOfUID(const char* uid)=0;
virtual char *getUIDOfName(const char* name)=0;
virtual bool Add_PrivateTag(unsigned short g,unsigned short e, MC_VR Avr,const char* AprivateCode)=0;
virtual const char  *getTagName(unsigned long  tag)=0;//2012/03/09 K.Ko
virtual void  DcmAPIError(const char* name)=0;

//
virtual bool createServiceList(bool isPropose,const char *ServiceName)=0;
virtual bool clearServiceList(bool isPropose,const char *ServiceName)=0;
virtual bool setNetworkTransferSyntax(bool isPropose,DcmXT_TransferSyntax xfer,const char *ServiceName)=0;
virtual bool addSOPClassUID(bool isPropose,const char * sopUID,const char *ServiceName)=0;
virtual bool setMaxPDUSize(bool isPropose,int size,const char *ServiceName) = 0;//2012/03/09 K.Ko
//
//virtual bool createAcceptServiceList(const char *ServiceName)=0;
//virtual bool addAcceptSOPClassUID(const char * sopUID,const char *ServiceName)=0;
};
///
// dcmlib main instance
class DcmLibApiLogger;
class IDcmLibDefDllCls IDcmLib  
{
public:
	enum LOG_LEVEL {
				LOGLEVEL_OFF_LOG      =  0,
				LOGLEVEL_FALTAL,
				LOGLEVEL_ERROR,
				LOGLEVEL_WARN,
				LOGLEVEL_INFO,
				LOGLEVEL_DEBUG,
				LOGLEVEL_TRACE,
				LOGLEVEL_ALL,
				LOGLEVEL_NOT_SET,
				 
	};

static IDcmLib *createInstance();
void destroy();
 
DcmXTDicomMessage			*createDicomMessage();
DcmXTDataSet				*createDataSet();
DcmXTAssociationClient		*createDcmAssociationClient();
DcmXTAssociationServer		*createDcmAssociationServer(DcmXTAssociationListener *wait_aso);
//
DcmXTAssociationListener   *getAssociationListener();
//
DcmXTUtil					*getUtil();
static DcmXTMutex			*getMutex();
DcmXTApplicationInstance	*getApplicationInstance();
 
//
unsigned int				CheckMemory();
//
//void						openLogger(const char *fileName,LOG_LEVEL log_level);
void						setupLoger(DcmLibApiLogger *logger,IDcmLib::LOG_LEVEL log_level);
//#18 2012/05/17 K.Ko
void setupWriteBufferLen(int len);
void setupReadeBufferLen(int len);
private:
	virtual ~IDcmLib();
	IDcmLib();
   void *m_UtilInstance;
   void *m_AppInstance;
	int m_data;
	//
	void *m_AssociationListenerHnd;
};

// for multi-thread lock
class DcmLibMTLock
{
public:
	DcmLibMTLock() 
	{ 
		IDcmLib::getMutex()->lock();
		m_locked = 1;
	}

	~DcmLibMTLock(void)				  
	{ 
		IDcmLib::getMutex()->unlock();
		m_locked = 0;
	}
private:
	int		m_locked;
};

//////////
template<class T>
struct deleter_DcmLib {
	void operator()(T* ptr_) {
		ptr_->Delete();
	}
};

}
#define SmartPtr_DcmXTDicomMessage  std::unique_ptr < XTDcmLib::DcmXTDicomMessage, XTDcmLib::deleter_DcmLib<XTDcmLib::DcmXTDicomMessage >>
#define SmartPtr_DcmXTDataSet   std::unique_ptr < XTDcmLib::DcmXTDataSet, XTDcmLib::deleter_DcmLib<XTDcmLib::DcmXTDataSet >>

#endif // !defined(AFX_IOCTVTK_LIB_H__01260_4AVFYYA__INCLUDED_)

