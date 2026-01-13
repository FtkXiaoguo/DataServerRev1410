// IDcmLibApi.h: DcmLib クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_API_H_)
#define AFX_IDICOM_LIB_API_H_
 
 
#include "IDcmLib.h"

#define UseXTDcmlib

namespace XTDcmLib
{
	
class DcmXTDicomMessageAndFile
{
public:
	DcmXTDicomMessageAndFile( DcmXTDicomMessage *dicomData)
	{
		m_DcmXTDicomMessage = dicomData;
		m_Type = DcmXTDicomMessageFile_Message;
	}

	 
	DcmXTDicomMessage *getDcmXTDicomMessage() const 
	{
		 
		return m_DcmXTDicomMessage;
	}
	void Delete(){
		destroy();
	}
	void destroy() {
		if(m_DcmXTDicomMessage){
			m_DcmXTDicomMessage->Delete();
			m_DcmXTDicomMessage = 0;
		}
		 
	}
	 
	enum DcmXTDicomMessageFile {
		DcmXTDicomMessageFile_Message,
		DcmXTDicomMessageFile_File
	} ;
	void setType(DcmXTDicomMessageFile type){ m_Type = type;};
	DcmXTDicomMessageFile getType( ) const { return m_Type ;};
protected:
	DcmXTDicomMessage	*m_DcmXTDicomMessage ;
	 
	DcmXTDicomMessageFile m_Type;
};

///////////////


class  IDcmLibMessageCallback : public  DcmXTMessageCallback
{
public:
	IDcmLibMessageCallback(){
		m_PixelCallback = 0;
		m_PixelCallback2 = 0;
	}
virtual bool readPixelData(int msgID, CallbackType CBtype,
										unsigned long* dataSizePtr,void** dataBufferPtr,
										int isFirst,int* isLastPtr) ;

void setPixelCallback( 		int        ApplicationID,
                            unsigned long  Tag,
                            void*          UserInfo,
							MC_STATUS ( NOEXP_FUNC *Acallback)(int            CBmsgID,
                                             unsigned long  CBtag,
                                             void*          CBuserInfo,
                                             CALLBACK_TYPE  CBtype,
                                             unsigned long* CBdataSizePtr,
                                             void**         CBdataBufferPtr,
                                             int            CBisFirst,
                                             int*           CBisLast)  );
void setPixelCallback2( 		int        ApplicationID,
                            unsigned long  Tag,
                            void*          UserInfo,
							MC_STATUS ( NOEXP_FUNC *Acallback)(int            CBmsgID,
                                             unsigned long  CBtag,
                                             void*          CBuserInfo,
											int            CBdataSize,
											 void*          CBdataBuffer,
											 int            CBisFirst,
											 int            CBisLast))  ;
protected:
	 
	 MC_STATUS  (NOEXP_FUNC *m_PixelCallback)(int            CBmsgID,
                                                     unsigned long  CBtag,
                                                     void*          CBuserInfo,
                                                     CALLBACK_TYPE  CBtype,
                                                     unsigned long* CBdataSizePtr,
                                                     void**         CBdataBufferPtr,
                                                     int            CBisFirst,
                                                     int*           CBisLast)  ;
	  MC_STATUS  (NOEXP_FUNC *m_PixelCallback2)(int            CBmsgID,
                                                     unsigned long  CBtag,
                                                     void*          CBuserInfo,
                                                     int            CBdataSize,
													 void*          CBdataBuffer,
													 int            CBisFirst,
													 int            CBisLast)  ;

	unsigned long  m_Tag;
	void*          m_UserInfo;
	int        m_ApplicationID;

};

class DcmLibApiLogger {
public:
	//
//	virtual void Logger(int id,const char *str) = 0;
	virtual void LoggerError(const char *format,...)	= 0;
	virtual void LoggerWarn(const char *format,...)		= 0;
	virtual void LoggerDebug(const char *format,...)	= 0;
	virtual void LoggerTrace(const char *format,...)	= 0;
 
};

class IDcmLibDefDllCls IDcmLibApi  
{
public :
//API
 static DcmXT_Reject_Reason RejectReasonMCToDcmXT(REJECT_REASON MC_reason);
 static char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax);
 static TRANSFER_SYNTAX  DcmLibTransferSyntaxToApi(DcmXT_TransferSyntax dcmLib_syntax);
 static DcmXT_TransferSyntax  ApiTransferSyntaxToDcmLib(TRANSFER_SYNTAX api_syntax);

 static void Set_String_Config_Value(StringParm     Aparm, const char*          Avalue);
 static void Set_Bool_Config_Value(BoolParm       Aparm, int            Avalue);
 static void Set_Long_Config_Value( LongParm       Aparm, long int       Avalue);
 static void Set_Int_Config_Value(IntParm        Aparm,	int            Avalue);
 //
 static bool Get_String_Config_Value(StringParm     Aparm, int AbufferSize, char*  Avalue);
 static bool Get_Bool_Config_Value(BoolParm       Aparm, int *Avalue);
 static bool Get_Long_Config_Value( LongParm       Aparm, long int *Avalue);
 static bool Get_Int_Config_Value(IntParm        Aparm,	int *Avalue);

static const char *Get_Version_String();
static const char * Get_Error_Message(MC_STATUS AstatusCode);

// static void openLogger(const char *fileName,IDcmLib::LOG_LEVEL log_level);
 static void setupLoger(DcmLibApiLogger *logger,IDcmLib::LOG_LEVEL log_level);
 //#18 2012/05/17 K.Ko
 static void setupWriteBufferLen(int len);
 static void setupReadeBufferLen(int len);

  static bool DcmLibInitialization(	void*(*AcfgFunction)(void),
                                    void*(*AdictFunction)(void),
                                    void*(*AinfoFunction)(void)) ;
static bool IDcmLibApi::DcmLibRelease();

  static bool Register_Application(int* ApplicationID, const char* ApplicationTitle);
  static bool Release_Application(int* ApplicationID);

 //
  static bool Get_Transfer_Syntax_From_Enum(	TRANSFER_SYNTAX Asyntax,
												char*          Auid,
												int            AbufferSize );
  static bool Get_Enum_From_Transfer_Syntax(	const char*          Auid,
												TRANSFER_SYNTAX *Asyntax
												 );

  static bool Get_ServiceNameOfUID(const char *uid, char* &namePtr);
 // static char* Chg_ServiceNameUID2Name(const dcm_string &uid );
  static bool Get_ServiceUIDOfName(const char* serviceName, char * &seriveUID );

  
  static bool Register_Callback_Function(int        ApplicationID,
                                                 unsigned long  Tag,
                                                 void*          UserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *Acallback)
                                                    (int            CBmsgID,
                                                     unsigned long  CBtag,
                                                     void*          CBuserInfo,
                                                     CALLBACK_TYPE  CBtype,
                                                     unsigned long* CBdataSizePtr,
                                                     void**         CBdataBufferPtr,
                                                     int            CBisFirst,
                                                     int*           CBisLast));

  static bool Close_Association(int			AssociationID);
  static bool Abort_Association(int			AssociationID);
  static bool Reject_Association(int		AssociationID,REJECT_REASON  Areason);
 
  static DcmXtError  Open_Association(		int				ApplicationID,
									int*			AssociationID,
									const char*		RemoteApplicationTitle,
									int*			RemoteHostPortNumber,
									char*			RemoteHostTCPIPName,
									char*			ServiceList);
//
  static DcmXtError Wait_For_Association(	const char*    ServiceList,
                                     int            Timeout,
                                     int*           ApplicationID,
                                     int*           AssociationID
									 ); 
  static DcmXtError Read_Message(int           AssociationID,
                                         int            Timeout,
                                         int*           MessageID,
                                         char**			ServiceName,
                                         MC_COMMAND*    Command 
										  );

 static bool Get_Value_To_Function(int            AmsgID,
                                                unsigned long   Atag,
                                                void*           AuserInfo,
                                                MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     void*          CBuserInfo,
                                                     int            CBdataSize,
                                                     void*          CBdataBuffer,
                                                     int            CBisFirst,
                                                     int            CBisLast));


  static bool Open_Message(int *MessageID) ;
  static bool Free_Message(int	MessageID);
  

  static bool Create_Empty_File(int*  AfileID, const char*     AfileName);
  static bool Open_File (int  AapplicationID,int  AfileID,  void*           AuserInfo);
  static bool File_To_Message(int AfileID);
  static bool Duplicate_Message(int OldMsgID,int *NewMsgID);

  static bool Message_To_File(int*  AfileID, const char*     AfileName);

static DcmXTAssociation 		*get_Association(int AssociationID);
static DcmXTAssociationClient 	*get_AssociationClient(int AssociationID);
static DcmXTAssociationServer 	*get_AssociationServer(int AssociationID);

static DcmXTDicomMessage		*get_DcmMessage(int MessageID);
//static DcmXTDataFile	*IDcmLibApi::get_DcmXTDataFile(int AfileID);

static DcmXTUtil	*getDcmXTUtil();
 ////
 static  void myStrcpy(const char *src_str, char *dest_str, int dest_len);
//static  void myStrcpy(const dllString &src_str, char *dest_str, int dest_len);
 //
static  unsigned int CheckMemory(int del_tbl=0);
//
private:
static int genApplicationID();
static int genAssociationID();
static int genMessageID();
static int m_CurApplicationID;
static int m_CurAssociationID;
static int m_CurMessageID;

 

};
 

#define DEC_ASSOSIATION_COM(AssociationID)			DcmXTAssociation	*associationComPtr		= IDcmLibApi::get_Association(AssociationID);		if(!associationComPtr)		return MC_INVALID_ASSOC_ID;
#define DEC_ASSOSIATION_CLIENT(AssociationID)	 DcmXTAssociationClient	*associationClientPtr	= IDcmLibApi::get_AssociationClient(AssociationID); if(!associationClientPtr)	return MC_INVALID_ASSOC_ID;
#define DEC_ASSOSIATION_SERVER(AssociationID)	 DcmXTAssociationServer	*associationServerPtr	= IDcmLibApi::get_AssociationServer(AssociationID); if(!associationServerPtr)	return MC_INVALID_ASSOC_ID;

#define DEC_MESSAGE(MessageID)			DcmXTDicomMessage	*messagePtr = IDcmLibApi::get_DcmMessage((MessageID)); if(!messagePtr) return MC_INVALID_MESSAGE_ID;


inline char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
	return IDcmLibApi::GetSyntaxDescription(A_syntax);
}
////////
inline MC_STATUS MC_Library_Initialization(	void*(*AcfgFunction)(void),
                                         void*(*AdictFunction)(void),
                                         void*(*AinfoFunction)(void))
{
	if( IDcmLibApi::DcmLibInitialization(	 AcfgFunction ,
								AdictFunction ,
								AinfoFunction) )
	{

		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
 
}

inline MC_STATUS MC_Library_Release(void)
{
	if( IDcmLibApi::DcmLibRelease())
	{

		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}


 
inline MC_STATUS  MC_Register_Application(	int*           ApplicationID,
											const char*    ApplicationTitle)
{
	if(IDcmLibApi::Register_Application(ApplicationID,
							ApplicationTitle
							))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Release_Application
                                                    (int*           ApplicationID)
{
	
	if(IDcmLibApi::Release_Application(ApplicationID))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}

}

inline MC_STATUS MC_Register_Callback_Function (int        ApplicationID,
                                                 unsigned long  Tag,
                                                 void*          UserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *Acallback)
                                                    (int            CBmsgID,
                                                     unsigned long  CBtag,
                                                     void*          CBuserInfo,
                                                     CALLBACK_TYPE  CBtype,
                                                     unsigned long* CBdataSizePtr,
                                                     void**         CBdataBufferPtr,
                                                     int            CBisFirst,
                                                     int*           CBisLast))
{
	if(IDcmLibApi::Register_Callback_Function(ApplicationID,Tag,UserInfo,
							Acallback
							))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}


inline MC_STATUS  MC_Open_Association(		int				ApplicationID,
									int*			AssociationID,
									const char*		RemoteApplicationTitle,
									int*			RemoteHostPortNumber,
									char*			RemoteHostTCPIPName,
									char*			ServiceList)
{
	DcmXtError errorCode = IDcmLibApi::Open_Association(		ApplicationID,
							AssociationID,
							RemoteApplicationTitle,
							RemoteHostPortNumber,
							RemoteHostTCPIPName,
							ServiceList);
	switch(errorCode)
	{
	case DcmXtErr_Normal:
		return MC_NORMAL_COMPLETION;
		break;
	case DcmXtErr_Timeout:
		return MC_TIMEOUT;
		break;
	case DcmXtErr_AssociatioinClosed:
		return MC_ASSOCIATION_CLOSED;
		break;
	case DcmXtErr_AssociatioinAborted: 
		return MC_ASSOCIATION_ABORTED;
		break;
	case DcmXtErr_AssociatioinRejected:
		return MC_ASSOCIATION_REJECTED;
		break;
	case DcmXtErr_NewWorkShutDown:
		return MC_NETWORK_SHUT_DOWN;
		break;
	default:
		return MC_ERROR;
		break;
	}
 

}

inline MC_STATUS  MC_Wait_For_Association(const char*    ServiceList,
                                                     int            Timeout,
                                                     int*           ApplicationID,
                                                     int*           AssociationID)
{

	DcmXtError errorCode = IDcmLibApi::Wait_For_Association(ServiceList,
                                                    Timeout,
                                                    ApplicationID,
                                                    AssociationID);
	switch(errorCode)
	{
	case DcmXtErr_Normal:
		return MC_NORMAL_COMPLETION;
		break;
	case DcmXtErr_Timeout:
		return MC_TIMEOUT;
		break;
	default:
		return MC_ASSOCIATION_ABORTED;
		break;
	}
}

inline MC_STATUS  MC_Close_Association(int*           AssociationID)
{
	if(IDcmLibApi::Close_Association(*AssociationID))
	{
		*AssociationID = 0;
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Abort_Association(int*  AssociationID)
{
	if(IDcmLibApi::Abort_Association(*AssociationID))
	{
		*AssociationID = 0;
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS  MC_Accept_Association(int AssociationID)
{
	DEC_ASSOSIATION_SERVER( AssociationID);
 	 
	if(associationServerPtr->accept( ))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
	 
}
inline MC_STATUS  MC_Reject_Association(int   AssociationID, REJECT_REASON  Areason)
{
	if(IDcmLibApi::Reject_Association(AssociationID,  Areason))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}


inline MC_STATUS MC_Open_Empty_Message(int* AmessageID)
{
	if(IDcmLibApi::Open_Message(AmessageID))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}

}
inline MC_STATUS  MC_Open_Message (int*  AmessageID,
                                                 const char*    AserviceName,
                                                 MC_COMMAND     Acommand)
{
	if(IDcmLibApi::Open_Message(AmessageID))
	{
		DEC_MESSAGE(*AmessageID);
	//	dcm_string  serviceName_temp ;
	//	if(AserviceName) serviceName_temp = AserviceName;
		const char *serviceName_temp = AserviceName;
		if(serviceName_temp == 0){
			return MC_ERROR;
		}
		if(strlen(serviceName_temp)<1){
			return MC_ERROR;
		}
		//change service name of string to UID 2012/03/27 K.Ko
		char * serviceUID;
		if(!IDcmLibApi::Get_ServiceUIDOfName(AserviceName,serviceUID)){
			return MC_ERROR;
		}

		messagePtr->setupCommand(serviceUID,Acommand);
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Free_Message (int*  MessageID)
{
	 
	if(IDcmLibApi::Free_Message(*MessageID))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Read_Message   (int            AssociationID,
                                                     int            Timeout,
                                                     int*           MessageID,
                                                     char**         ServiceName,
                                                     MC_COMMAND*    Command)
{
	DcmXtError errorCode = IDcmLibApi::Read_Message(		AssociationID,
										Timeout,
										MessageID,
										ServiceName,
										Command);
	switch(errorCode)
	{
	case DcmXtErr_Normal:
		return MC_NORMAL_COMPLETION;
		break;
	case DcmXtErr_Timeout:
		return MC_TIMEOUT;
		break;
	case DcmXtErr_AssociatioinClosed:
		return MC_ASSOCIATION_CLOSED;
		break;
	case DcmXtErr_AssociatioinAborted: 
		return MC_ASSOCIATION_ABORTED;
		break;
	case DcmXtErr_NewWorkShutDown:
		return MC_NETWORK_SHUT_DOWN;
		break;
	default:
		return MC_ERROR;
		break;
	}
}

///
 
inline MC_STATUS  MC_Create_Empty_File(int*            AfileID,
                                        const char*     AfileName)
{
	if(IDcmLibApi::Create_Empty_File(AfileID ,AfileName))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS  MC_Open_File (int  AapplicationID,
                                int  AfileID,  
                                void*           AuserInfo, 
                                MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                       (char*       CBfilename,
                                        void*       CBuserInfo,
                                        int*        CBdataSize,
                                        void**      CBdataBuffer,
                                        int         CBisFirst,
                                        int*        CBisLast))
{
	if(IDcmLibApi::Open_File(AapplicationID, AfileID, AuserInfo))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS    MC_Open_File_Upto_Tag 
                                       (int             AapplicationID,
                                        int             AfileID,  
                                        void*           AuserInfo, 
                                        unsigned long   Atag,
                                        long*           Aoffset,
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int*        CBdataSize,
                                                void**      CBdataBuffer,
                                                int         CBisFirst,
                                                int*        CBisLast))
{
	if(IDcmLibApi::Open_File(AapplicationID, AfileID, AuserInfo))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

 
inline MC_STATUS  MC_File_To_Message(int AfileID)
{
	if(IDcmLibApi::File_To_Message(AfileID))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
///

inline MC_STATUS  MC_Duplicate_Message (int AsourceID, int* AdestID,
                                                        TRANSFER_SYNTAX AdestTransferSyntax,
                                                        MC_STATUS   (NOEXP_FUNC *Acompression_Callback)(
                                                                    int             CBmsgID,
                                                                    void**          CBContext,
                                                                    unsigned long   CBdataLen,
                                                                    void*           CBdataValue,
                                                                    unsigned long*  CBoutdataLen,
                                                                    void**          CBoutdataValue,
                                                                    int             CBisFirst,
                                                                    int             CBisLast,
                                                                    int             CBrelease),
                                                        MC_STATUS   (NOEXP_FUNC *Adecompression_Callback)(
                                                                    int             CBmsgID,
                                                                    void**          CBContext,
                                                                    unsigned long   CBdataLen,
                                                                    void*           CBdataValue,
                                                                    unsigned long*  CBoutdataLen,
                                                                    void**          CBoutdataValue,
                                                                    int             CBisFirst,
                                                                    int             CBisLast,
                                                                    int             CBrelease) )
{
	if(IDcmLibApi::Duplicate_Message(AsourceID,AdestID))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
	 
	 
}  


inline MC_STATUS  MC_Get_Association_Info(int            AssociationID,
                                                     AssocInfo*     Ainfo)
{
	DEC_ASSOSIATION_COM( AssociationID);
	 
	DcmXTAssociationInfo AssoTemp;
 
	if(associationComPtr->getInfo(AssoTemp))
	{

		if(!Ainfo) return MC_ERROR;
/*
#define ASInfoStrCpy_RemoteApplicationTitle(dest,src) strncpy(dest,src,32)
#define ASInfoStrCpy_LocalApplicationTitle(dest,src) strncpy(dest,src,32)
#define ASInfoStrCpy_RemoteHostName(dest,src) strncpy(dest,src,128)
#define ASInfoStrCpy_RemoteIPAddress(dest,src) strncpy(dest,src,128)
#define ASInfoStrCpy_RemoteImplementationClassUID(dest,src) strncpy(dest,src,128)
#define ASInfoStrCpy_RemoteImplementationVersion(dest,src) strncpy(dest,src,32)
*/
 
		Ainfo->NumberOfProposedServices		= AssoTemp.NumberOfProposedServices;
		Ainfo->NumberOfAcceptableServices	= AssoTemp.NumberOfAcceptableServices;
		ASInfoStrCpy_RemoteApplicationTitle(Ainfo->RemoteApplicationTitle,AssoTemp.RemoteApplicationTitle) ;

		ASInfoStrCpy_RemoteHostName(Ainfo->RemoteHostName,AssoTemp.RemoteHostName);

		Ainfo->Tcp_socket					= AssoTemp.Tcp_socket;

		ASInfoStrCpy_RemoteIPAddress(Ainfo->RemoteIPAddress,AssoTemp.RemoteIPAddress);

		ASInfoStrCpy_LocalApplicationTitle(Ainfo->LocalApplicationTitle,AssoTemp.LocalApplicationTitle);

		ASInfoStrCpy_RemoteImplementationClassUID(Ainfo->RemoteImplementationClassUID,AssoTemp.RemoteImplementationClassUID);

		ASInfoStrCpy_RemoteImplementationVersion(Ainfo->RemoteImplementationVersion,AssoTemp.RemoteImplementationVersion);
		//


		
		Ainfo->LocalMaximumPDUSize					= AssoTemp.LocalMaximumPDUSize;
		Ainfo->RemoteMaximumPDUSize					= AssoTemp.RemoteMaximumPDUSize;
		Ainfo->MaxOperationsInvoked					= AssoTemp.MaxOperationsInvoked;
		Ainfo->MaxOperationsPerformed				= AssoTemp.MaxOperationsPerformed;


		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
 
////
	 
}
inline MC_STATUS  MC_Get_First_Acceptable_Service(int            AsessionID,
                                                     ServiceInfo*   AservInfo)
{
	DEC_ASSOSIATION_CLIENT( AsessionID);
	 
	DcmXTAssociationServiceInfo AservInfoTemp;
	bool endList;
	if(associationClientPtr->getFirstAcceptableService(AservInfoTemp,endList))
	{
		if(!AservInfo) return MC_ERROR;


		TRANSFER_SYNTAX sytax_temp;
		IDcmLibApi::Get_Enum_From_Transfer_Syntax( AservInfoTemp.SyntaxTypeUID ,&sytax_temp);
	 
	 	AservInfo->SyntaxType = sytax_temp;//IMPLICIT_LITTLE_ENDIAN;
		strcpy(AservInfo->ServiceName,AservInfoTemp.ServiceName );
		AservInfo->PresentationContextID = AservInfoTemp.PresentationContextID;

		return MC_NORMAL_COMPLETION;
	}else{
		if(endList) return MC_END_OF_LIST;
		return MC_ERROR;
	}
}
inline MC_STATUS  MC_Get_Next_Acceptable_Service(int            AsessionID,
                                                     ServiceInfo*   AservInfo)
{
	DEC_ASSOSIATION_CLIENT(AsessionID);
	 
	DcmXTAssociationServiceInfo AservInfoTemp;
	bool endList;
	if(associationClientPtr->getNextAcceptableService(AservInfoTemp,endList))
	{
		if(!AservInfo) return MC_ERROR;

		AservInfo->SyntaxType = IMPLICIT_LITTLE_ENDIAN;
		strcpy(AservInfo->ServiceName,AservInfoTemp.ServiceName );
		AservInfo->PresentationContextID = AservInfoTemp.PresentationContextID;
	 
		 
		return MC_NORMAL_COMPLETION;
	}else{
		if(endList) return MC_END_OF_LIST;
		return MC_ERROR;
	}
}


inline MC_STATUS  MC_Send_Response_Message(int  AssociationID,
                                                     RESP_STATUS    ResponseStatus,
                                                     int            ResponseMessageID)
{
	DEC_ASSOSIATION_SERVER( AssociationID);
	DEC_MESSAGE(ResponseMessageID);
	 
	if(associationServerPtr->sendResponseMessage(ResponseStatus,messagePtr))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Send_Request_Message(int	AssociationID,	int	MessageID)
{
 

	DEC_ASSOSIATION_CLIENT( AssociationID);
	DEC_MESSAGE(MessageID);
	 
	if(associationClientPtr->sendRequestMessage(*messagePtr))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}

}

inline MC_STATUS  MC_Empty_Message      (int            AmsgID)
{
 
	DEC_MESSAGE(AmsgID);

	if(messagePtr->close()){
		if(messagePtr->open()){
			return MC_NORMAL_COMPLETION;
		}else{
			return MC_ERROR;
		}
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS MC_Message_To_File 
                                       (int             AmsgID,
                                        const char*     AfileName)
{
	DEC_MESSAGE(AmsgID);

	messagePtr->setFileName(AfileName);
	 
	return MC_NORMAL_COMPLETION;
	 
	 
}

inline MC_STATUS   MC_Write_File(int             AfileID, 
                                        int             AnumBytes, 
                                        void*           AuserInfo, 
                                        MC_STATUS       ( NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int         CBdataSize,
                                                void*       CBdataBuffer,
                                                int         CBisFirst, 
                                                int         CBisLast))
{
	DEC_MESSAGE(AfileID);

	if(messagePtr->writeFile())
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
} 

inline MC_STATUS    MC_Get_Transfer_Syntax_From_Enum(
                                                 TRANSFER_SYNTAX Asyntax,
                                                 char*          Auid,
                                                 int            AbufferSize )
{
	if(IDcmLibApi::Get_Transfer_Syntax_From_Enum( Asyntax,Auid,AbufferSize))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
//
inline MC_STATUS    MC_Get_Enum_From_Transfer_Syntax(
                                                 const char*    Auid,
                                                 TRANSFER_SYNTAX* Asyntax )
{
	
	if(IDcmLibApi::Get_Enum_From_Transfer_Syntax( Auid,Asyntax ))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
	 
}
//
/*
	virtual bool Set_TransferSyntax(DcmXT_TransferSyntax Xfer)=0;
	virtual bool Get_TransferSyntax(DcmXT_TransferSyntax &Xfer)=0;
	*/
inline MC_STATUS   MC_Set_Message_Transfer_Syntax
                                                    (int            AmsgID,
                                                     TRANSFER_SYNTAX Asyntax)
{
	DEC_MESSAGE(AmsgID);

	DcmXT_TransferSyntax new_syntax = IDcmLibApi::ApiTransferSyntaxToDcmLib(Asyntax);
	if(messagePtr->Set_TransferSyntax(new_syntax))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS    MC_Get_Message_Transfer_Syntax
                                                    (int            AmsgID,
                                                     TRANSFER_SYNTAX* Asyntax)
{
	DEC_MESSAGE(AmsgID);
	DcmXT_TransferSyntax dcmlib_Xfer;
	if(messagePtr->Get_TransferSyntax(dcmlib_Xfer))
	{
		*Asyntax = IDcmLibApi::DcmLibTransferSyntaxToApi(dcmlib_Xfer);
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

//

inline MC_STATUS MC_Get_UID_From_MergeCOM_Service(
                                                 const char*    AserviceName,
                                                 char*          Auid,
                                                 int            AbufferSize )
{
	 char * serviceUID;
	if(IDcmLibApi::Get_ServiceUIDOfName(AserviceName,serviceUID)){
		
		strncpy(Auid,serviceUID,AbufferSize);
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
		
	}
	 
}

//
inline MC_STATUS MC_Get_MergeCOM_Service(const char*   Auid,
                                                 char*          Aname,
                                                 int            Alength)
{
	 char * str_temp;
	if(IDcmLibApi::Get_ServiceNameOfUID( Auid,str_temp))
	{
		strncpy(Aname,str_temp,Alength);
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
 
}
inline MC_STATUS MC_Set_Service_Command(int      AmsgID,
                                                 const char*    AserviceName,
                                                 MC_COMMAND     Acommand)
{
	DEC_MESSAGE(AmsgID);
	 
	 char * serviceUID;
	if(!IDcmLibApi::Get_ServiceUIDOfName(AserviceName,serviceUID)){
		return MC_ERROR;
	}
	if(messagePtr->Set_ServiceName(serviceUID,Acommand))
	{
		return MC_NORMAL_COMPLETION;
		 
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Get_Message_Service(int    AmsgID, 
                                                 char**         AserviceName, 
                                                 MC_COMMAND*    Acommand)
{
	DEC_MESSAGE(AmsgID);
	 
//	dllString serviceUID;
	
//	if(messagePtr->Get_ServiceName(serviceUID,*Acommand))
	char serviceUID_buff[128]; //DICOM ->64 
	if(messagePtr->Get_ServiceName(serviceUID_buff,128,*Acommand))
	{
		 
		char* name_temp=0;
		 
			
		if(!IDcmLibApi::Get_ServiceNameOfUID(serviceUID_buff,name_temp)){
				return MC_ERROR;
		}
		*AserviceName = name_temp;
		if(AserviceName == 0){
			return MC_ERROR;
		}else{
			return MC_NORMAL_COMPLETION;
		}
	}else{
		return MC_ERROR;
	}
	 
} 

//

///////////////////////
///////////////////
/* ======================================================================== *
 *                  Functions to set configuration values                   *
 * ======================================================================== */
inline MC_STATUS    MC_Set_String_Config_Value(
                                                 StringParm     Aparm,
                                                 char*          Avalue)
{
	IDcmLibApi::Set_String_Config_Value(Aparm, Avalue);
	return MC_NORMAL_COMPLETION;
}
inline MC_STATUS    MC_Set_Bool_Config_Value(
                                                 BoolParm       Aparm,
                                                 int            Avalue)
{
	IDcmLibApi::Set_Bool_Config_Value(Aparm, Avalue);
	return MC_NORMAL_COMPLETION;
}
inline MC_STATUS     MC_Set_Long_Config_Value(
                                                 LongParm       Aparm,
                                                 long int       Avalue)
{
	IDcmLibApi::Set_Long_Config_Value(Aparm, Avalue);
	return MC_NORMAL_COMPLETION;
}
inline MC_STATUS     MC_Set_Int_Config_Value(
                                                 IntParm        Aparm,
                                                 int            Avalue)
{
	IDcmLibApi::Set_Int_Config_Value(Aparm, Avalue);
	return MC_NORMAL_COMPLETION;
}
inline MC_STATUS     MC_Set_Log_Destination(
                                                 LogParm        Aparm,
                                                 int            Avalue)
{
	return MC_NORMAL_COMPLETION;
}

////////////
inline MC_STATUS MC_Get_String_Config_Value(
                                                 StringParm     Aparm,
                                                 int            AbufferSize,
                                                 char*          Abuffer)
{
	if(IDcmLibApi::Get_String_Config_Value(Aparm,AbufferSize,Abuffer)){
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS MC_Get_Bool_Config_Value(
                                                 BoolParm       Aparm,
                                                 int*           Avalue)
{
	if(IDcmLibApi::Get_Bool_Config_Value(Aparm,Avalue)){
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS MC_Get_Long_Config_Value(
                                                 LongParm       Aparm,
                                                 long int*           Avalue)
{
	if(IDcmLibApi::Get_Long_Config_Value(Aparm,Avalue)){
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
 
inline MC_STATUS MC_Get_Int_Config_Value(
                                                 IntParm       Aparm,
                                                  int*           Avalue)
{
	if(IDcmLibApi::Get_Int_Config_Value(Aparm,Avalue)){
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}


inline MC_STATUS     MC_Get_Version_String(
                                                 int            AbufferSize,
                                                 char*          Abuffer)
{
	const char * ver_str = IDcmLibApi::Get_Version_String();
	strncpy(Abuffer,ver_str,AbufferSize);
 
	return MC_NORMAL_COMPLETION;
}

inline const char*  MC_Error_Message (MC_STATUS AstatusCode)
{
	const char * error_msg_str = IDcmLibApi::Get_Error_Message(AstatusCode);
	 
	return error_msg_str;
}
 
////////////////////////
// the following , not implemate ye
////////////////////////
#include "IDcmLibApi_dumy.h"

#include "IDcmLibApiSetValue.h"
#include "IDcmLibApiGetValue.h"

}

#endif // !defined(AFX_IDICOM_LIB_API_H_)

//////////////////////
extern MC_STATUS    AqMediaToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast);

extern  MC_STATUS    AqMemoryToFileObj( char*     A_filename,
                           void*     A_userInfo,
                           int*      A_dataSize,
                           void**    A_dataBuffer,
                           int       A_isFirst,
                           int*      A_isLast);


extern MC_STATUS    AqFileObjToMedia( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast);
								
/* ======================================================================== *
 *                Default function names from source generators             *
 * ======================================================================== */
extern void* MC_Config_Values(void) ;
extern void* MC_Dictionary_Values(void) ;
extern void* MC_MsgInfo_Values(void) ;
