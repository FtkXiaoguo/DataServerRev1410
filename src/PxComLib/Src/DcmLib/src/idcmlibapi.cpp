// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable: 4786)

 
#include "IDcmLibApi.h"
 
#include "DcmXTDataSetMain.h"
//#include "DcmXTDicomMessageMain.h"
#include "DcmXTAssociationMain.h"
#include "DcmXTAssociationClientMain.h"
#include "DcmXTAssociationServerMain.h"
#include "DcmXTAssociationListenerMain.h"

#include "DcmXTDicomMessageMain.h"

#include "InstanceIDMan.h"

#include "DcmXTUtilMain.h"

#include "CheckMemoryLeak.h"

using namespace XTDcmLib;

//#10 2012/03/22 K.Ko
#define MyRootUID "1.2.392.200036.9163"   //PreXion DICOM root UID
#define MyDicomLibVersionName "PXDcmLibV30"

// static std::map<std::string , char*> _ServiceNameList;

 
static ConfigMap _Config_;

 

int IDcmLibApi::m_CurApplicationID=0;
int IDcmLibApi::m_CurAssociationID=0;
int IDcmLibApi::m_CurMessageID=0;


const char *IDcmLibApi::Get_Version_String()
{
	return "PxDcmAPI Ver1.0.0";
}
////////////


#define StringMapKey(key) (100+key) 
#define BoolMapKey(key) (200+key) 
#define LongMapKey(key) (300+key) 
#define IntMapKey(key) (400+key) 
void IDcmLibApi::Set_String_Config_Value(StringParm     Aparm, const char*          Avalue)
{
	theConfigPair new_item(StringMapKey(Aparm), ConfigEntry(string(Avalue)));
	_Config_.insert(new_item);
}

void IDcmLibApi::Set_Bool_Config_Value(BoolParm       Aparm, int            Avalue)
{
	theConfigPair new_item(BoolMapKey(Aparm), ConfigEntry(  Avalue!=0));
	_Config_.insert(new_item);
}

void IDcmLibApi::Set_Long_Config_Value( LongParm       Aparm, long int   Avalue)
{
	theConfigPair new_item(LongMapKey(Aparm), ConfigEntry((long int) Avalue));
	_Config_.insert(new_item);
}

void IDcmLibApi::Set_Int_Config_Value(IntParm        Aparm,	int            Avalue)
{
	theConfigPair new_item(IntMapKey(Aparm), ConfigEntry(( int) Avalue));
	_Config_.insert(new_item);

}
 
bool findConfigEntry(int Aparm,ConfigEntry &ret_entry)
{
	bool ret_b = false;
	 
	ConfigMap::iterator it = _Config_.begin();
	while(it != _Config_.end()){

		if((*it).first == Aparm){
			ret_entry = (*it).second;
			ret_b = true;
			break;
		}
		it++;
	}
	return ret_b;
}
bool IDcmLibApi::Get_String_Config_Value(StringParm     Aparm,int AbufferSize,   char*  Avalue)
{
	ConfigEntry ret_entry;
	if(findConfigEntry(StringMapKey(Aparm),ret_entry)){
		strncpy(Avalue,ret_entry.m_string_Config.c_str(),AbufferSize);
		return true;
	}
	return false;
}

bool IDcmLibApi::Get_Bool_Config_Value(BoolParm Aparm,int *Avalue)
{
	ConfigEntry ret_entry;
	if(findConfigEntry(BoolMapKey(Aparm),ret_entry)){
		*Avalue = ret_entry.m_bool_Config;
		return true;
	}
	return false;
}

 bool IDcmLibApi::Get_Long_Config_Value(LongParm Aparm,long int *Avalue)
{
	ConfigEntry ret_entry;
	if(findConfigEntry(LongMapKey(Aparm),ret_entry)){
		*Avalue = ret_entry.m_long_Config;
		return true;
	}
	return false;
}

  bool IDcmLibApi::Get_Int_Config_Value(IntParm Aparm, int *Avalue)
{
	ConfigEntry ret_entry;
	if(findConfigEntry(IntMapKey(Aparm),ret_entry)){
		*Avalue = ret_entry.m_int_Config;
		return true;
	}
	return false;
}
  
  

//////////////////////////////
class ApplicationTitleTbl
{
public:
	void Delete() { };
//IDcmLib		*m_DcmLibInstance;
char m_Application_Title[128] ;
int			m_id;
IDcmLibMessageCallback m_MessageCallback;
};
#define APTStrCpy_ApplicationTitle(dest,src) strncpy(dest,src,128)


typedef InstanceIDManage<ApplicationTitleTbl *> ApplicationTblMan;
static ApplicationTblMan _ApplicationTblMan;
///
typedef InstanceIDManage<DcmXTAssociation *> DcmAssociationMan;
static DcmAssociationMan _AssociationMan;

///
typedef InstanceIDManage<DcmXTDicomMessageAndFile *> DcmMessageAndFileMan;
static DcmMessageAndFileMan _DcmMessageAndFileMan;
////////////
#include <map>

int IDcmLibApi::genApplicationID()
{
	DcmLibMTLock lock;

	int start_id = m_CurApplicationID;
	do {
		m_CurApplicationID++;
		if(m_CurApplicationID<1) m_CurApplicationID=1;

		if(start_id == m_CurApplicationID){
			//error
			return 0;
		}
	}while(_ApplicationTblMan.getInstance(m_CurApplicationID)!=0) ;

	return m_CurApplicationID;
}
int IDcmLibApi::genAssociationID()
{
	DcmLibMTLock lock;

	int start_id = m_CurAssociationID;
	do{
		m_CurAssociationID++;
		if(m_CurAssociationID<1) m_CurAssociationID=1;

		if(start_id == m_CurAssociationID){
			//error
			return 0;
		}

	}while(get_Association(m_CurAssociationID)!=0) ;

	return m_CurAssociationID;
}
 
int IDcmLibApi::genMessageID()
{
	DcmLibMTLock lock;

 
	int start_id = m_CurMessageID;
	do{
		m_CurMessageID++;
		if(m_CurMessageID<1) m_CurMessageID=1;

		if(start_id == m_CurMessageID){
			//error
			return 0;
		}

	}while(get_DcmMessage(m_CurMessageID)!=0) ;

	return m_CurMessageID;
}
 
/////////

//////////////////

int findApplicationID(const dcm_string &ap_title)
{
	 
	int retID = -1;

	int list_size = _ApplicationTblMan.getSize();
	for(int i=0;i<list_size;i++){
		ApplicationTitleTbl *app_entry_temp = _ApplicationTblMan.getItem(i);
		if(app_entry_temp->m_Application_Title ==  ap_title )
		{
			retID = app_entry_temp->m_id;
			break;
		}
	}
	 
	return retID;
	 
 
}
//
void deleteAllApplicationID()
{
#if 1
	_ApplicationTblMan.removeAllInstance();
#else
	int list_size = _ApplicationTblMan.getSize();
	for(int i=0;i<list_size;i++){
		ApplicationTitleTbl *app_entry_temp = _ApplicationTblMan.removeInstance(i+1);
		if(app_entry_temp){
			delete app_entry_temp;
		}
	}
	_ApplicationTblMan.clearAll();
#endif
}
//
void deleteAllAssociationID()
{
#if 1
	_AssociationMan.removeAllInstance();
#else
	int list_size = _AssociationMan.getSize();
	for(int i=0;i<list_size;i++){
		DcmXTAssociation *association_temp = _AssociationMan.removeInstance(i+1);
		if(association_temp){
			association_temp->Delete();
		}
	}
	_AssociationMan.clearAll();
#endif
}
//
void deleteAllDcmMessageAndFileID()
{
#if 1
	_DcmMessageAndFileMan.removeAllInstance();
#else
	int list_size = _DcmMessageAndFileMan.getSize();
	for(int i=0;i<list_size;i++){
		DcmXTDicomMessageAndFile * item  = _DcmMessageAndFileMan.removeInstance(i+1);
		if(item){
			item->destroy();
		}
	}
	_DcmMessageAndFileMan.clearAll();
#endif
}

void deleteConfigMap()
{
	_Config_.clear();
	 
}
 
////////////////


void testIDMan()
{
 ApplicationTblMan  ApplicationTblMan_temp;

	ApplicationTblMan_temp.clearAll();

	 ApplicationTitleTbl *new_app_entry = new ApplicationTitleTbl;
	 strcpy(new_app_entry->m_Application_Title,"test1");
	 int id1 = 1;

	ApplicationTblMan_temp.registerInstance(id1,new_app_entry);

	 new_app_entry = new ApplicationTitleTbl;
	 strcpy(new_app_entry->m_Application_Title,"test2");

	 int id2 = 2;
	 ApplicationTblMan_temp.registerInstance(id2,new_app_entry);

	 ApplicationTitleTbl *app_entry_temp = ApplicationTblMan_temp.removeInstance(id2);
	 delete app_entry_temp ;

	 new_app_entry = new ApplicationTitleTbl;
	 strcpy(new_app_entry->m_Application_Title ,"test3");

	 int id3 = 30;
	 ApplicationTblMan_temp.registerInstance(id3,new_app_entry);

	 printf("id3 %d \n",id3);
}



static IDcmLib *m_stock_DcmLibInstance = 0;

#define CurrenctDcmLibInstance m_stock_DcmLibInstance
inline bool checkDcmLibInstance() { return (m_stock_DcmLibInstance != 0);};
bool IDcmLibApi::DcmLibInitialization(	void*(*AcfgFunction)(void),
                                    void*(*AdictFunction)(void),
                                    void*(*AinfoFunction)(void))
 {
	 _Config_.clear();

	 m_stock_DcmLibInstance = IDcmLib::createInstance();

 
	//#10 2012/03/22 K.Ko
	 IDcmLibApi::Set_String_Config_Value(IMPLEMENTATION_CLASS_UID, MyRootUID) ;
	 IDcmLibApi::Set_String_Config_Value(IMPLEMENTATION_VERSION, MyDicomLibVersionName) ;
		 

	 return true;
 }
bool IDcmLibApi::DcmLibRelease()
{
	bool ret_b = false;
	if(checkDcmLibInstance()) 
	{

		m_stock_DcmLibInstance->destroy();

		deleteAllApplicationID();
	 
		deleteAllAssociationID();
	 
		deleteAllDcmMessageAndFileID();

		deleteConfigMap();

		ret_b = true;
	}

	 _Config_.clear();

#ifdef MemoryLeakCheck 
	bool dump_flag = false;
	if(dump_flag){
		_CrtDumpMemoryLeaks();
	}
#endif
	return ret_b;
}
#if 0
void IDcmLibApi::openLogger(const char *fileName,IDcmLib::LOG_LEVEL log_level)
{
	if(checkDcmLibInstance()) 
	{
		m_stock_DcmLibInstance->openLogger( fileName, log_level);
	}
}
#endif
void IDcmLibApi::setupLoger(DcmLibApiLogger *logger,IDcmLib::LOG_LEVEL log_level)
{
	if(checkDcmLibInstance()) 
	{
		m_stock_DcmLibInstance->setupLoger( logger,log_level);
	}
}
//#18 2012/05/17 K.Ko
 void  IDcmLibApi::setupWriteBufferLen(int len)
{
	if(checkDcmLibInstance()) 
	{
		m_stock_DcmLibInstance->setupWriteBufferLen(len);
	}
}
void  IDcmLibApi::setupReadeBufferLen(int len)
{
	if(checkDcmLibInstance()) 
	{
		m_stock_DcmLibInstance->setupReadeBufferLen(len);
	}
}


 unsigned int IDcmLibApi::CheckMemory(int del_tbl )
{
	if(checkDcmLibInstance()) 
	{
		int rest_msg_size = _DcmMessageAndFileMan.getSize();
		if(del_tbl){
			deleteConfigMap();
		}
		return m_stock_DcmLibInstance->CheckMemory();
	}else{
		return 0;
	}
}
 bool  IDcmLibApi::Register_Application(int* ApplicationID, const char* ApplicationTitle)
 {

	 if(!checkDcmLibInstance()) return false;
 
	 *ApplicationID = genApplicationID();
	 if(*ApplicationID<=0) {
		 return false;
	 }

	ApplicationTitleTbl *new_app_entry = new ApplicationTitleTbl;
 
	APTStrCpy_ApplicationTitle(new_app_entry->m_Application_Title, ApplicationTitle);
	
	if(new_app_entry){
		 m_stock_DcmLibInstance->getApplicationInstance()->addAE(new_app_entry->m_Application_Title );
	 }


//	IDcmLib *app_lib_instance = IDcmLib::createInstance();
//	new_app_entry->m_DcmLibInstance = app_lib_instance;



	new_app_entry->m_id = *ApplicationID;
	
	return _ApplicationTblMan.registerInstance(*ApplicationID,new_app_entry);
	 
 
 }
 bool IDcmLibApi::Release_Application(int* ApplicationID)
 {
	 if(!checkDcmLibInstance()) return false;


	 ApplicationTitleTbl *app_entry = _ApplicationTblMan.removeInstance(*ApplicationID);

	 if(app_entry){
		 m_stock_DcmLibInstance->getApplicationInstance()->removeAE(app_entry->m_Application_Title );

		 delete app_entry;
	 }
	 return true;
 }
 bool IDcmLibApi::Close_Association(int	 AssociationID)
 {
	 if(!checkDcmLibInstance()) return false;

	 DcmXTAssociation *association_temp = _AssociationMan.removeInstance(AssociationID);
	
	 if(association_temp==0){
		 return false;
	 }
 
	 if(association_temp->isServer()){
		 DcmXTAssociationServer *server_aso = (DcmXTAssociationServer *)association_temp;
		 server_aso->Delete();
		 //not supported
	//	 return false;
	 }else{
 
	 
		 DcmXTAssociationClient *client_aso = (DcmXTAssociationClient *)association_temp;
		 client_aso->close();
		client_aso->Delete();
	}

	 return true;

 }
  bool IDcmLibApi::Abort_Association(int	 AssociationID)
 {
	 if(!checkDcmLibInstance()) return false;

	 DcmXTAssociation *association_temp = _AssociationMan.removeInstance(AssociationID);

	 if(association_temp==0){
		 return false;
	 }
	 association_temp->abort();

	 association_temp->Delete();

	 return true;

 }
    
 bool IDcmLibApi::Reject_Association(int	 AssociationID,REJECT_REASON  Areason)
 {
	 if(!checkDcmLibInstance()) return false;

	 DcmXTAssociation *association_temp = _AssociationMan.getInstance(AssociationID);

	 if(association_temp==0){
		 return false;
	 }
	 if(!association_temp->isServer()){
		 //not supported;
		 return false;
	 }
 
	 association_temp = _AssociationMan.removeInstance(AssociationID);
	 DcmXTAssociationServer *server_aso = (DcmXTAssociationServer *)association_temp;

	 server_aso->reject(IDcmLibApi::RejectReasonMCToDcmXT(Areason));

	 server_aso->Delete();

	 return true;

 }
 
 DcmXtError  IDcmLibApi::Open_Association(	int            ApplicationID,
						int*           AssociationID,
						const char*    RemoteApplicationTitle,
						int*           RemoteHostPortNumber,
						char*          RemoteHostTCPIPName,
						char*          ServiceList)
 {

	 DcmXtError  errorCode = DcmXtErr_Unknown;
	  
	if(!checkDcmLibInstance()) return errorCode;

	*AssociationID = -1;
	
	 
	 ApplicationTitleTbl *app_entry = _ApplicationTblMan.getInstance(ApplicationID);
	 if(!app_entry){
		 return errorCode;
	 }
	 
	 *AssociationID=  genAssociationID();
	 if(*AssociationID<=0) {
		 return errorCode;
	 }

	 DcmXTAssociationClient *new_assoiation = CurrenctDcmLibInstance->createDcmAssociationClient();

	 if(!new_assoiation){
		 
		return errorCode ;
	 } 

 
	 DcmXTUtilMain	*dcm_utilPtr = (DcmXTUtilMain*)(m_stock_DcmLibInstance->getUtil());
	 
	DcmServiceListEntry *serviceListEntry = dcm_utilPtr->getServiceList(ServiceList,true/*propose*/);

	DcmXTAssociationClientMain *AssociationClientMain = (DcmXTAssociationClientMain *)new_assoiation;
	AssociationClientMain->setProposeServiceList(serviceListEntry);



	 new_assoiation->setLocalAE(app_entry->m_Application_Title);
	 if(!new_assoiation->open( RemoteApplicationTitle,
								*RemoteHostPortNumber,
								RemoteHostTCPIPName,
								ServiceList,
								errorCode))
	 {
			new_assoiation->Delete();
			return errorCode;
	 }else{
		 if(!_AssociationMan.registerInstance(*AssociationID,new_assoiation))
		 {
		 	 errorCode = DcmXtErr_Unknown;
		 };

		 return errorCode;
	 }

 }
 DcmXtError IDcmLibApi::Wait_For_Association(	const char*    ServiceList,
                                     int            Timeout,
                                     int*           ApplicationID,
                                     int*           AssociationID 
									 )
 {
	 /*
	 *  TCPIP_LISTEN_PORT は1つしかない。
	 *  この関数はマルチスレッド対応していない。基本的に１つアプリに1スレッド
	 *
	 */
	 DcmXtError  errorCode = DcmXtErr_Unknown;
	 if(!checkDcmLibInstance()) return errorCode;

	*ApplicationID = -1;
	*AssociationID = -1;
 
 
	DcmXTAssociationListenerMain *AssociationListener = (DcmXTAssociationListenerMain *)CurrenctDcmLibInstance->getAssociationListener();
	

	int portNum = 100;
	  
 	 portNum = _Config_[IntMapKey(TCPIP_LISTEN_PORT)].m_int_Config;
//	 dllString calledAE ;//= app_entry->m_Application_Title;
	 char calledAE_Temp[128];
	 if(!AssociationListener->waiting( ServiceList,
								portNum,
								Timeout,
								calledAE_Temp,128,
								errorCode))
	 {
			 
			return errorCode;
	 }

	 //to create new association

	  
	  *AssociationID=  genAssociationID();
	 if(*AssociationID<=0) {
		 return DcmXtErr_Unknown;
	 }


	 DcmXTAssociationServerMain *new_assoiation = 
		 (DcmXTAssociationServerMain *)CurrenctDcmLibInstance->createDcmAssociationServer(AssociationListener);

	 if(!new_assoiation){
	
		return DcmXtErr_Unknown ;
	 } 

//	  new_assoiation->setAssociation(AssociationListener->getAssociation());

	 //
	*ApplicationID = findApplicationID(calledAE_Temp);
	if(!_AssociationMan.registerInstance(*AssociationID,new_assoiation)){
		errorCode = DcmXtErr_Unknown;
	}else{
		DcmXTUtilMain	*dcm_utilPtr = (DcmXTUtilMain*)(m_stock_DcmLibInstance->getUtil());
	 
		DcmServiceListEntry *serviceListEntry = dcm_utilPtr->getServiceList(ServiceList,false/*propose*/);

	
		new_assoiation->setAcceptServiceList(serviceListEntry);
	};
	 
		 
	return errorCode;
	  

 }
bool IDcmLibApi::Open_Message(int *MessageID)
 {
	 if(!m_stock_DcmLibInstance) return false;

	 *MessageID=  genMessageID();
	 if(*MessageID<=0) {
		 return false;
	 }

  	 DcmXTDicomMessage *new_messsage = CurrenctDcmLibInstance->createDicomMessage();


	 DcmXTDicomMessageAndFile *new_item = new DcmXTDicomMessageAndFile(new_messsage);
	 new_item->setType(DcmXTDicomMessageAndFile::DcmXTDicomMessageFile_Message);

	if(!new_messsage->open()){
		return false;
	}
	 
	  
 	  new_messsage->setID(*MessageID);
	 return _DcmMessageAndFileMan.registerInstance(*MessageID,new_item); 
 }
 
bool IDcmLibApi::Free_Message(int	MessageID)
{
	if(!checkDcmLibInstance()) return false;

	 DcmXTDicomMessageAndFile *message_item = _DcmMessageAndFileMan.removeInstance(MessageID);

	 if(message_item==0){
		 return false;
	 }
	 message_item->destroy();
	 delete message_item;

	 return true;
}
bool IDcmLibApi::Create_Empty_File(int*  AfileID, const char*     AfileName)
{
	if(!m_stock_DcmLibInstance) return false;
	 
	*AfileID=  genMessageID();
	 if(*AfileID<=0) {
		 return false;
	 }

	 DcmXTDicomMessage *new_messsage = CurrenctDcmLibInstance->createDicomMessage();
	
	 DcmXTDicomMessageAndFile *new_item = new DcmXTDicomMessageAndFile(new_messsage);
	 new_item->setType(DcmXTDicomMessageAndFile::DcmXTDicomMessageFile_File);

	
	 if(!new_messsage->openFile(AfileName)){
		 return false;
	 }

	  new_messsage->setID(*AfileID);
	 return _DcmMessageAndFileMan.registerInstance(*AfileID,new_item); 

	 
}
 
bool IDcmLibApi::Open_File (int  AapplicationID,int  AfileID,  void*  AuserInfo)
{
	 
	DcmXTDicomMessage *file =  get_DcmMessage(  AfileID);
	if(!file) return false;

 
	 ApplicationTitleTbl *app_entry = _ApplicationTblMan.getInstance(AapplicationID);
	 if(app_entry){
		  file->Set_Callback(&(app_entry->m_MessageCallback));
	 }
	return file->readFile();
}
bool IDcmLibApi::File_To_Message(int AfileID)
{
	DcmXTDicomMessageAndFile *dcmFile_item = _DcmMessageAndFileMan.getInstance(AfileID);
	if(!dcmFile_item) return false;

	

	/*
	* change File to Message
	*/

	dcmFile_item->setType(DcmXTDicomMessageAndFile::DcmXTDicomMessageFile_Message); 

	
	
	return true;
}
bool IDcmLibApi::Duplicate_Message(int OldMsgID,int *NewMsgID)
{
	DcmXTDicomMessageAndFile *dcmFile_item = _DcmMessageAndFileMan.getInstance(OldMsgID);

	if(!dcmFile_item) return false;
	 

	*NewMsgID=  genMessageID();
	 if(*NewMsgID<=0) {
		 return false;
	 }

	DcmXTDicomMessage *org_msg = dcmFile_item->getDcmXTDicomMessage();
	if(!org_msg){
		return false;
	}
	DcmXTDicomMessage *new_msg = org_msg->clone();
	if(!new_msg){
		return false;
	}

	 DcmXTDicomMessageAndFile *new_item = new DcmXTDicomMessageAndFile(new_msg);
	 new_item->setType(DcmXTDicomMessageAndFile::DcmXTDicomMessageFile_Message);


	 new_msg->setID(*NewMsgID);

	return _DcmMessageAndFileMan.registerInstance(*NewMsgID,new_item); 
}
  

 DcmXTAssociation *IDcmLibApi::get_Association(int AssociationID)
 {
	 return _AssociationMan.getInstance(AssociationID);
 }
DcmXTAssociationClient 	*IDcmLibApi::get_AssociationClient(int AssociationID)
{
	DcmXTAssociation *ssociation_temp = _AssociationMan.getInstance(AssociationID);
//	DcmXTAssociationClient *client_association_temp = dynamic_cast<DcmXTAssociationClientMain*>(ssociation_temp);
	DcmXTAssociationClient *client_association_temp = (DcmXTAssociationClientMain*)(ssociation_temp);
	return client_association_temp;
}
DcmXTAssociationServer 	*IDcmLibApi::get_AssociationServer(int AssociationID)
{
	DcmXTAssociation *ssociation_temp = _AssociationMan.getInstance(AssociationID);
	DcmXTAssociationServerMain *server_association_temp = (DcmXTAssociationServerMain*)(ssociation_temp);
	return server_association_temp;
//	return dynamic_cast<DcmXTAssociationServerMain*>(_AssociationMan.getInstance(AssociationID));
}

 DcmXTDicomMessage *IDcmLibApi::get_DcmMessage(int MessageID)
 {
	 DcmXTDicomMessageAndFile *message_item = _DcmMessageAndFileMan.getInstance(MessageID);
	 if(!message_item) return 0;
	 return  message_item->getDcmXTDicomMessage();
 }
#if 0
 DcmXTDataFile *IDcmLibApi::get_DcmXTDataFile(int AfileID)
 {
	 DcmXTDicomMessageAndFile *dcmFile_item = _DcmMessageAndFileMan.getInstance(AfileID);
	 if(!dcmFile_item) return 0;
	 return  dcmFile_item->getDcmXTDataFile();
 }
#endif

 static char *_error_cmd_string = "UnknownServiceName";
 DcmXtError IDcmLibApi::Read_Message(int            AssociationID,
                                             int            Timeout,
                                             int*           MessageID,
                                             char**         ServiceName,
                                             MC_COMMAND*    Command
											)
{
	DcmXtError  errorCode = DcmXtErr_Unknown;
	if(!checkDcmLibInstance()) return errorCode;

	 

//	 DcmXTAssociationServer *association_temp = get_AssociationServer(AssociationID);
	DcmXTAssociation *association_temp = get_Association(AssociationID);

	 if(association_temp==0){
		 return errorCode;
	 }

//	 dllString serviceNameUID;
	 char serviceNameUID_temp[128]; //DICOM ->64 

	 DcmXTDicomMessage *read_msg = association_temp->readMessage(serviceNameUID_temp,128,*Command,errorCode,Timeout);
	 if(!read_msg) {
		 if(association_temp->isServer()){
			switch(errorCode)
			{
			case DcmXtErr_AssociatioinClosed:
			case DcmXtErr_AssociatioinAborted: 
				{
				    
					DcmXTAssociation *association_temp = _AssociationMan.removeInstance(AssociationID);
					// closed alreadly
					//just delete my association instance
					if(association_temp){
						association_temp->Delete();
					}

				}
				break;
			default:
				 
				break;
			}
		 }
		 return errorCode;
	 }

	 *MessageID=  genMessageID();
	 if(*MessageID<=0) {
		 return errorCode;
	 }

	 char *serviceName_temp = 0;
	  
	 Get_ServiceNameOfUID(serviceNameUID_temp,serviceName_temp);

	 DcmXTDicomMessageAndFile *new_item = new DcmXTDicomMessageAndFile(read_msg);
	 new_item->setType(DcmXTDicomMessageAndFile::DcmXTDicomMessageFile_Message);


	 read_msg->setID(*MessageID);

	 *ServiceName = serviceName_temp;

	_DcmMessageAndFileMan.registerInstance(*MessageID,new_item); ;
	return errorCode;
}



 /****************************************************************************
 *
 *  Function    :   GetSyntaxDescription
 *
 *  Description :   Return a text description of a DICOM transfer syntax.
 *                  This is used for display purposes.
 *
 ****************************************************************************/
char* IDcmLibApi::GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr = NULL;
    
    switch (A_syntax)
    {
    case IMPLICIT_LITTLE_ENDIAN: ptr = "Implicit VR Little Endian"; break;
    case EXPLICIT_LITTLE_ENDIAN: ptr = "Explicit VR Little Endian"; break;
    case EXPLICIT_BIG_ENDIAN:    ptr = "Explicit VR Big Endian"; break;
    case IMPLICIT_BIG_ENDIAN:    ptr = "Implicit VR Big Endian"; break;
    case DEFLATED_EXPLICIT_LITTLE_ENDIAN: ptr = "Deflated Explicit VR Little Endian"; break;
    case RLE:                    ptr = "RLE"; break;
    case JPEG_BASELINE:          ptr = "JPEG Baseline (Process 1)"; break;
    case JPEG_EXTENDED_2_4:      ptr = "JPEG Extended (Process 2 & 4)"; break;
    case JPEG_EXTENDED_3_5:      ptr = "JPEG Extended (Process 3 & 5)"; break;
    case JPEG_SPEC_NON_HIER_6_8: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)"; break;
    case JPEG_SPEC_NON_HIER_7_9: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)"; break;
    case JPEG_FULL_PROG_NON_HIER_10_12: ptr = "JPEG Full Progression, Non-Hierarchical (Process 10 & 12)"; break;
    case JPEG_FULL_PROG_NON_HIER_11_13: ptr = "JPEG Full Progression, Non-Hierarchical (Process 11 & 13)"; break;
    case JPEG_LOSSLESS_NON_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical (Process 14)"; break;
    case JPEG_LOSSLESS_NON_HIER_15: ptr = "JPEG Lossless, Non-Hierarchical (Process 15)"; break;
    case JPEG_EXTENDED_HIER_16_18: ptr = "JPEG Extended, Hierarchical (Process 16 & 18)"; break;
    case JPEG_EXTENDED_HIER_17_19: ptr = "JPEG Extended, Hierarchical (Process 17 & 19)"; break;
    case JPEG_SPEC_HIER_20_22:   ptr = "JPEG Spectral Selection Hierarchical (Process 20 & 22)"; break;
    case JPEG_SPEC_HIER_21_23:   ptr = "JPEG Spectral Selection Hierarchical (Process 21 & 23)"; break;
    case JPEG_FULL_PROG_HIER_24_26: ptr = "JPEG Full Progression, Hierarchical (Process 24 & 26)"; break;
    case JPEG_FULL_PROG_HIER_25_27: ptr = "JPEG Full Progression, Hierarchical (Process 25 & 27)"; break;
    case JPEG_LOSSLESS_HIER_28:  ptr = "JPEG Lossless, Hierarchical (Process 28)"; break;
    case JPEG_LOSSLESS_HIER_29:  ptr = "JPEG Lossless, Hierarchical (Process 29)"; break;
    case JPEG_LOSSLESS_HIER_14:  ptr = "JPEG Lossless, Non-Hierarchical, First-Order Prediction"; break;
    case JPEG_2000_LOSSLESS_ONLY:ptr = "JPEG 2000 Lossless Only"; break;
    case JPEG_2000:              ptr = "JPEG 2000"; break;
    case JPEG_LS_LOSSLESS:       ptr = "JPEG-LS Lossless"; break;
    case JPEG_LS_LOSSY:          ptr = "JPEG-LS Lossy (Near Lossless)"; break;
    case MPEG2_MPML:             ptr = "MPEG2 Main Profile @ Main Level"; break;
    case PRIVATE_SYNTAX_1:       ptr = "Private Syntax 1"; break;
    case PRIVATE_SYNTAX_2:       ptr = "Private Syntax 2"; break;
    case INVALID_TRANSFER_SYNTAX:ptr = "Invalid Transfer Syntax"; break;
    }
    return ptr;
}

class DcmXt_Api_TransferSyntax_Pair
{
public:
DcmXT_TransferSyntax	DcmXt_xfer;
TRANSFER_SYNTAX			Api_xfer;
};

DcmXt_Api_TransferSyntax_Pair _DcmXt_Api_TransferSyntax_Table[] = 
{
/*
*   DcmXT_TransferSyntax,				TRANSFER_SYNTAX
*/

	DcmXT_EXS_Unknown,					INVALID_TRANSFER_SYNTAX	,
	DcmXT_EXS_LittleEndianImplicit,		IMPLICIT_LITTLE_ENDIAN	,
	DcmXT_EXS_BigEndianImplicit,		IMPLICIT_BIG_ENDIAN		,
	//
	DcmXT_EXS_LittleEndianExplicit,		EXPLICIT_LITTLE_ENDIAN	,
	DcmXT_EXS_BigEndianExplicit,		EXPLICIT_BIG_ENDIAN	,
	//
};
 TRANSFER_SYNTAX IDcmLibApi::DcmLibTransferSyntaxToApi(DcmXT_TransferSyntax dcmLib_syntax) 
{

	TRANSFER_SYNTAX ret_syntax = INVALID_TRANSFER_SYNTAX;
	int tbl_size = sizeof(_DcmXt_Api_TransferSyntax_Table)/sizeof(DcmXt_Api_TransferSyntax_Pair);
	int i;
	for(i=0;i<tbl_size;i++){
		if(_DcmXt_Api_TransferSyntax_Table[i].DcmXt_xfer == dcmLib_syntax){
			ret_syntax = _DcmXt_Api_TransferSyntax_Table[i].Api_xfer;
		}
	}
	return ret_syntax;
#if 0
	switch(dcmLib_syntax){
	default:
	case DcmXT_EXS_Unknown:
		break;
	case DcmXT_EXS_LittleEndianImplicit:
		ret_syntax = IMPLICIT_LITTLE_ENDIAN; break;
    case DcmXT_EXS_BigEndianImplicit:
		ret_syntax = IMPLICIT_BIG_ENDIAN; break;
	//
    case DcmXT_EXS_LittleEndianExplicit:
		ret_syntax = EXPLICIT_LITTLE_ENDIAN; break;
    case DcmXT_EXS_BigEndianExplicit:
		ret_syntax = EXPLICIT_BIG_ENDIAN; break;
	//
	//
    case DcmXT_EXS_JPEGProcess1TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess2_4TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess3_5TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess6_8TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess7_9TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess10_12TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess11_13TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess14TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess15TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess16_18TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess17_19TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess20_22TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess21_23TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess24_26TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess25_27TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess28TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess29TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGProcess14SV1TransferSyntax:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_RLELossless:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGLSLossless:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEGLSLossy:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_DeflatedLittleEndianExplicit:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEG2000LosslessOnly:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEG2000:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_MPEG2MainProfileAtMainLevel:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEG2000MulticomponentLosslessOnly:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
    case DcmXT_EXS_JPEG2000Multicomponent:
		ret_syntax = INVALID_TRANSFER_SYNTAX; break;
	}

	return ret_syntax;
#endif

}  
//

DcmXT_TransferSyntax IDcmLibApi::ApiTransferSyntaxToDcmLib(TRANSFER_SYNTAX api_syntax) 
{

	 DcmXT_TransferSyntax ret_syntax = DcmXT_EXS_Unknown;
	int tbl_size = sizeof(_DcmXt_Api_TransferSyntax_Table)/sizeof(DcmXt_Api_TransferSyntax_Pair);
	int i;
	for(i=0;i<tbl_size;i++){
		if(_DcmXt_Api_TransferSyntax_Table[i].Api_xfer == api_syntax){
			ret_syntax = _DcmXt_Api_TransferSyntax_Table[i].DcmXt_xfer;
			break;
		}
	}
	return ret_syntax;

 }

 bool IDcmLibApi::Get_Transfer_Syntax_From_Enum(	TRANSFER_SYNTAX Asyntax,
												char*          Auid,
												int            AbufferSize )
 {
#if 0
	 char* str_temp = IDcmLibApi::GetSyntaxDescription(Asyntax);
	 if(str_temp == 0) return false;
	 IDcmLibApi::myStrcpy(str_temp, Auid, AbufferSize);
#else
	 checkDcmLibInstance();
	 DcmXTUtil	*dcm_utilPtr = m_stock_DcmLibInstance->getUtil();
	 DcmXTUtil::XferNames XferName;

	 DcmXT_TransferSyntax dcmxt_xfer = ApiTransferSyntaxToDcmLib(Asyntax);
	 if(dcm_utilPtr->getTansferSyntaxBySyntax(dcmxt_xfer,XferName)){
		IDcmLibApi::myStrcpy(XferName.xferID, Auid, AbufferSize);
		return true;
	 }else{
		 return false;
	 }
	 ;
#endif
	 return true;
 }
 //
 
 bool IDcmLibApi::Get_Enum_From_Transfer_Syntax(const char*          Auid,	
												TRANSFER_SYNTAX *Asyntax )
 {
#if 0
	 char* str_temp = IDcmLibApi::GetSyntaxDescription(Asyntax);
	 if(str_temp == 0) return false;
	 IDcmLibApi::myStrcpy(str_temp, Auid, AbufferSize);
#else
	 checkDcmLibInstance();
	 DcmXTUtil	*dcm_utilPtr = m_stock_DcmLibInstance->getUtil();
	 DcmXTUtil::XferNames XferName;

//	 DcmXT_TransferSyntax dcmxt_xfer = ApiTransferSyntaxToDcmLib(Asyntax);
	 if(dcm_utilPtr->getTansferSyntaxByName(Auid,XferName)){
		 *Asyntax = DcmLibTransferSyntaxToApi(XferName.xfer);
		 
		return true;
	 }else{
		 return false;
	 }
	 ;
#endif
	 return true;
 }

 


 bool IDcmLibApi::Get_ServiceNameOfUID(const char *uid,  char* &uid_name)
 {
	 checkDcmLibInstance();
	 DcmXTUtil	*dcm_utilPtr = m_stock_DcmLibInstance->getUtil();
	  
	 uid_name = dcm_utilPtr->getNameOfUID( uid);
	 
	 if(uid_name){

	//	strncpy(nameBuf,uid_name,AbufferSize);
		return true;
	 }else{
		 return false;
	 }
 }
#if 0
 char* IDcmLibApi::Chg_ServiceNameUID2Name(const dcm_string &serviceNameUID )
 {
	 if(serviceNameUID.size()<1) return 0;

 	char *serviceName_temp = _ServiceNameList[serviceNameUID];
	if(serviceName_temp == 0){
		 char _str_buff[512];
		 _str_buff[0] = 0;
		 Get_NameOfUID(stringTochar(serviceNameUID), _str_buff, 128 );

	 	  
		 int str_len = strlen(_str_buff);
		 if(str_len >0){
			 serviceName_temp = new char[str_len+1];
			 strcpy(serviceName_temp,_str_buff);
			 _ServiceNameList[serviceNameUID] = serviceName_temp;
		//	 delete [] serviceName_temp;
		 }

	}
	return serviceName_temp;
 }
#endif

  
 bool IDcmLibApi::Get_ServiceUIDOfName(const char* serviceName, char * &seriveUID )
 {
	 dcm_string ret_str ;

	 checkDcmLibInstance();
	 DcmXTUtil	*dcm_utilPtr = m_stock_DcmLibInstance->getUtil();
	  
	  char*uid_name = dcm_utilPtr->getUIDOfName( serviceName);
	 
	 if(uid_name){

		seriveUID = uid_name;
		 
		return true;
	 }else{
		 return false;
	 }
#if 0
	 std::map<std::string , char*>::iterator it = _ServiceNameList.begin();
	 while(it != _ServiceNameList.end()){
		 if( strcmp(it->second,serviceName) == 0){
			 ret_str = it->first;
			 break;
		 }
		 it++;
	 }
#else
#endif

	 
 }

void IDcmLibApi::myStrcpy(const char *src_str, char *dest_str, int dest_len)
{
	 if(dest_len<1){
		 dest_len = strlen(src_str);
	 }
	 strncpy(dest_str,src_str,dest_len);
}
#if 0
void IDcmLibApi::myStrcpy(const dllString &src_str, char *dest_str, int dest_len)
{
	if(src_str.size()>0){
		IDcmLibApi::myStrcpy(src_str.c_str(), dest_str, dest_len);
	}
}
#endif
bool IDcmLibApi::Register_Callback_Function(int        ApplicationID,
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
	if(!checkDcmLibInstance()) return false;

	 ApplicationTitleTbl *app_entry = _ApplicationTblMan.getInstance(ApplicationID);
	 if(!app_entry){
		 return false;
	 }

	app_entry->m_MessageCallback.setPixelCallback(ApplicationID, Tag,  UserInfo,Acallback);
	return true;
};


  bool IDcmLibMessageCallback::readPixelData(int msgID, CallbackType CBtype,
										unsigned long* dataSizePtr,void** dataBufferPtr,
										int isFirst,int* isLastPtr) 
  {
	  if(m_PixelCallback){
			CALLBACK_TYPE  CM_CBtype = PROVIDING_DATA_LENGTH;
			
			if(CBtype == DcmXTMessageCallback::CB_Data){
				CM_CBtype = PROVIDING_DATA;
			}
			unsigned long  CBtag = 0x7FE00010;
		  (*m_PixelCallback)(msgID,CBtag,m_UserInfo,CM_CBtype,
			  dataSizePtr, dataBufferPtr,
			  isFirst, isLastPtr
			  );
	  }
	  if(m_PixelCallback2){
			CALLBACK_TYPE  CM_CBtype = PROVIDING_DATA_LENGTH;
			
			if(CBtype == DcmXTMessageCallback::CB_Data){
				CM_CBtype = PROVIDING_DATA;
			}
			unsigned long  CBtag = 0x7FE00010;
			int isLast_temp = 0;
			if(isLastPtr) {
				isLast_temp = *isLastPtr;
			}
			int dataSize_temp = 0;
			if(dataSizePtr){
				dataSize_temp = *dataSizePtr;
			}

		  (*m_PixelCallback2)(msgID,CBtag,m_UserInfo,
			  dataSize_temp, *dataBufferPtr,
			  isFirst, isLast_temp
			  );
	  }
	  return true;
  }
 
 void IDcmLibMessageCallback::setPixelCallback(	int        ApplicationID,
									unsigned long  Tag,
									void*          UserInfo,
									MC_STATUS ( NOEXP_FUNC *Acallback)(int            CBmsgID,
													 unsigned long  CBtag,
													 void*          CBuserInfo,
													 CALLBACK_TYPE  CBtype,
													 unsigned long* CBdataSizePtr,
													 void**         CBdataBufferPtr,
													 int            CBisFirst,
													 int*           CBisLast)   )
  {
	m_PixelCallback = Acallback;
	//
	m_ApplicationID	= ApplicationID;
	m_Tag			= Tag;
	m_UserInfo		= UserInfo;
	
	  
  }
  void IDcmLibMessageCallback::setPixelCallback2(	int        ApplicationID,
									unsigned long  Tag,
									void*          UserInfo,
									MC_STATUS ( NOEXP_FUNC *Acallback)(int            CBmsgID,
													 unsigned long  CBtag,
													 void*          CBuserInfo,
													 int            CBdataSize,
														 void*          CBdataBuffer,
														 int            CBisFirst,
														 int            CBisLast))
  {
	m_PixelCallback2 = Acallback;
	//
	m_ApplicationID	= ApplicationID;
	m_Tag			= Tag;
	m_UserInfo		= UserInfo;
	
  }
 
  bool IDcmLibApi::Get_Value_To_Function(int            AmsgID,
													unsigned long   Atag,
													void*           AuserInfo,
													MC_STATUS       (NOEXP_FUNC *AuserFunction)
														(int            CBmsgID,
														 unsigned long  ATag,
														 void*          CBuserInfo,
														 int            CBdataSize,
														 void*          CBdataBuffer,
														 int            CBisFirst,
														 int            CBisLast))
  {
	  DcmXTDicomMessage *mess_ptr  =  get_DcmMessage(  AmsgID);
	if(!mess_ptr) return false;

	IDcmLibMessageCallback DcmLibMessageCallback_temp;
	 DcmLibMessageCallback_temp.setPixelCallback2(0/*ApplicationID*/,Atag,AuserInfo,AuserFunction);

	 bool ret_val = mess_ptr->Read_OBData(&DcmLibMessageCallback_temp,Atag);

	  return ret_val;
  }

  

  DcmXT_Reject_Reason IDcmLibApi::RejectReasonMCToDcmXT(REJECT_REASON MC_reason)
  {
	  DcmXT_Reject_Reason ret_val;

	  switch(MC_reason){
	    case PERMANENT_NO_REASON_GIVEN:
		  ret_val = XTDcmLib::DcmXt_PERMANENT_NO_REASON_GIVEN;
		  break;
		case TRANSIENT_NO_REASON_GIVEN:
		  ret_val = XTDcmLib::DcmXt_TRANSIENT_NO_REASON_GIVEN;
		  break;
		case PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED:
		  ret_val = XTDcmLib::DcmXt_PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED;
		  break;
		case TRANSIENT_TEMPORARY_CONGESTION:
		  ret_val = XTDcmLib::DcmXt_TRANSIENT_TEMPORARY_CONGESTION;
		  break;
		case TRANSIENT_LOCAL_LIMIT_EXCEEDED:
		  ret_val = XTDcmLib::DcmXt_TRANSIENT_LOCAL_LIMIT_EXCEEDED;
		  break;
		break;
	  }
	  return ret_val;
  }

DcmXTUtil	*IDcmLibApi::getDcmXTUtil()
{
	if(!checkDcmLibInstance()){
		return NULL;
	}
	 DcmXTUtil	*dcm_utilPtr = m_stock_DcmLibInstance->getUtil();
	 return dcm_utilPtr;
}

///////////////
typedef struct {
	int m_MC_Status;
	const char* m_MsgName;
} MC_StatusMsg;

MC_StatusMsg StatusMsgTbl[] = 
{
	MC_ERROR  ,				"DcmAPI_ERROR",
    MC_NORMAL_COMPLETION  ,	"DcmAPI_NORMAL_COMPLETION",
 //
    MC_ALREADY_REGISTERED                         ,          "DcmAPI_ALREADY_REGISTERED         "  ,       
    MC_ASSOCIATION_ABORTED                       ,           "DcmAPI_ASSOCIATION_ABORTED        "  ,   
    MC_ASSOCIATION_CLOSED                       ,            "DcmAPI_ASSOCIATION_CLOSED         "  ,    
    MC_ASSOCIATION_REJECTED                       ,          "DcmAPI_ASSOCIATION_REJECTED       "  ,   
    MC_ATTRIBUTE_HAS_VALUES                       ,          "DcmAPI_ATTRIBUTE_HAS_VALUES       "  ,  
    MC_BUFFER_TOO_SMALL                       ,              "DcmAPI_BUFFER_TOO_SMALL           "  , 
    MC_CALLBACK_CANNOT_COMPLY                       ,        "DcmAPI_CALLBACK_CANNOT_COMPLY     "   ,
    MC_CALLBACK_DATA_SIZE_NEGATIVE                       ,   "DcmAPI_CALLBACK_DATA_SIZE_NEGATIVE"   ,
    MC_CALLBACK_DATA_SIZE_UNEVEN                       ,     "DcmAPI_CALLBACK_DATA_SIZE_UNEVEN  "   ,
    MC_CALLBACK_PARM_ERROR                       ,           "DcmAPI_CALLBACK_PARM_ERROR        "   ,
    MC_CALLBACK_REGISTERED                       ,           "DcmAPI_CALLBACK_REGISTERED        "   ,
    MC_CANNOT_COMPLY                       ,                 "DcmAPI_CANNOT_COMPLY              "   ,
    MC_CANT_ACCESS_PROFILE                       ,           "DcmAPI_CANT_ACCESS_PROFILE        "   ,
    MC_CONFIG_INFO_ERROR                       ,             "DcmAPI_CONFIG_INFO_ERROR          "   ,
    MC_CONFIG_INFO_MISSING                       ,           "DcmAPI_CONFIG_INFO_MISSING        "   ,
    MC_DDFILE_ERROR                       ,                  "DcmAPI_DDFILE_ERROR               "   ,
    MC_DOES_NOT_VALIDATE                       ,             "DcmAPI_DOES_NOT_VALIDATE          "   ,
    MC_EMPTY_VALUE                       ,                   "DcmAPI_EMPTY_VALUE                "   ,
	MC_END_OF_DATA                       ,                   "DcmAPI_END_OF_DATA                " ,
    MC_EXT_INFO_UNAVAILABLE                         ,        "DcmAPI_EXT_INFO_UNAVAILABLE       "  ,
//
    MC_FOUND                                    ,             "DcmAPI_FOUND                      " ,
    MC_FUNCTION_UNAVAILABLE                         ,         "DcmAPI_FUNCTION_UNAVAILABLE       " ,
    MC_INCOMPATIBLE_VR                         ,              "DcmAPI_INCOMPATIBLE_VR            " ,
    MC_INCOMPATIBLE_VALUE                         ,           "DcmAPI_INCOMPATIBLE_VALUE         " ,
    MC_INVALID_APPLICATION_ID                         ,       "DcmAPI_INVALID_APPLICATION_ID     " ,
    MC_INVALID_APPLICATION_TITLE                         ,    "DcmAPI_INVALID_APPLICATION_TITLE  " ,
    MC_INVALID_ASSOC_ID                         ,             "DcmAPI_INVALID_ASSOC_ID           " ,
    MC_INVALID_CHARS_IN_VALUE                         ,       "DcmAPI_INVALID_CHARS_IN_VALUE     " ,
    MC_INVALID_COMMAND                         ,              "DcmAPI_INVALID_COMMAND            " ,
    MC_INVALID_DATA_TYPE                         ,            "DcmAPI_INVALID_DATA_TYPE          " ,
    MC_END_OF_LIST                         ,                  "DcmAPI_END_OF_LIST                " ,
    MC_INVALID_GROUP                         ,                "DcmAPI_INVALID_GROUP              " ,
    MC_INVALID_HOST_NAME                         ,            "DcmAPI_INVALID_HOST_NAME          " ,
    MC_INVALID_ITEM_ID                         ,              "DcmAPI_INVALID_ITEM_ID            " ,
    MC_INVALID_LENGTH_FOR_TITLE                         ,     "DcmAPI_INVALID_LENGTH_FOR_TITLE   " ,
    MC_INVALID_LENGTH_FOR_VR                         ,        "DcmAPI_INVALID_LENGTH_FOR_VR      " ,
    MC_INVALID_LICENSE                         ,              "DcmAPI_INVALID_LICENSE            " ,
    MC_INVALID_MESSAGE_ID                         ,           "DcmAPI_INVALID_MESSAGE_ID         " ,
    MC_INVALID_MESSAGE_RECEIVED                         ,     "DcmAPI_INVALID_MESSAGE_RECEIVED   " ,
    MC_INVALID_PARAMETER_NAME                         ,       "DcmAPI_INVALID_PARAMETER_NAME     " ,
//
    MC_INVALID_PORT_NUMBER                  ,           "DcmAPI_INVALID_PORT_NUMBER        "  ,
    MC_INVALID_PRIVATE_CODE                  ,          "DcmAPI_INVALID_PRIVATE_CODE       "  ,
    MC_INVALID_SERVICE_LIST_NAME                  ,     "DcmAPI_INVALID_SERVICE_LIST_NAME  "  ,
    MC_INVALID_TAG                             ,        "DcmAPI_INVALID_TAG                "  ,
	MC_INVALID_TRANSFER_SYNTAX           ,              "DcmAPI_INVALID_TRANSFER_SYNTAX    "    ,
    MC_INVALID_VALUE_FOR_VR                      ,      "DcmAPI_INVALID_VALUE_FOR_VR       "   ,  
    MC_INVALID_VALUE_NUMBER                      ,      "DcmAPI_INVALID_VALUE_NUMBER       "   , 
    MC_INVALID_VR_CODE                      ,           "DcmAPI_INVALID_VR_CODE            "   ,
    MC_LOG_EMPTY                      ,                 "DcmAPI_LOG_EMPTY                  "   ,
    MC_MESSAGE_EMPTY                      ,             "DcmAPI_MESSAGE_EMPTY              "   ,
    MC_MESSAGE_VALIDATES                      ,         "DcmAPI_MESSAGE_VALIDATES          "   ,
    MC_MISSING_CONFIG_PARM                      ,       "DcmAPI_MISSING_CONFIG_PARM        "   ,
    MC_MSGFILE_ERROR                      ,             "DcmAPI_MSGFILE_ERROR              "   ,
    MC_MUST_BE_POSITIVE                      ,          "DcmAPI_MUST_BE_POSITIVE           "   ,
    MC_NETWORK_SHUT_DOWN                      ,         "DcmAPI_NETWORK_SHUT_DOWN          "   ,
    MC_NO_APPLICATIONS_REGISTERED                      ,"DcmAPI_NO_APPLICATIONS_REGISTERED "   ,
    MC_NO_CALLBACK                      ,               "DcmAPI_NO_CALLBACK                "   ,
    MC_NO_CONDITION_FUNCTION                      ,     "DcmAPI_NO_CONDITION_FUNCTION      "   ,
    MC_NO_FILE_SYSTEM                      ,            "DcmAPI_NO_FILE_SYSTEM             "   ,
    MC_NO_INFO_REGISTERED                      ,        "DcmAPI_NO_INFO_REGISTERED         "   ,
//
    MC_NO_LICENSE                      ,                   "DcmAPI_NO_LICENSE                   " ,
    MC_NO_MERGE_INI                      ,                 "DcmAPI_NO_MERGE_INI                 " ,
    MC_NO_MORE_ATTRIBUTES                      ,           "DcmAPI_NO_MORE_ATTRIBUTES           " ,
    MC_NO_MORE_VALUES                      ,               "DcmAPI_NO_MORE_VALUES               " ,
    MC_NO_PROFILE                      ,                   "DcmAPI_NO_PROFILE                   " ,
    MC_NO_REQUEST_PENDING                      ,           "DcmAPI_NO_REQUEST_PENDING           " ,
    MC_NON_SERVICE_ATTRIBUTE                      ,        "DcmAPI_NON_SERVICE_ATTRIBUTE        " ,
    MC_NOT_FOUND                                    ,      "DcmAPI_NOT_FOUND                    "  ,                  
	MC_NOT_ONE_OF_ENUMERATED_VALUES                  ,     "DcmAPI_NOT_ONE_OF_ENUMERATED_VALUES ",
    MC_NOT_ONE_OF_DEFINED_TERMS                       ,    "DcmAPI_NOT_ONE_OF_DEFINED_TERMS     ",
    MC_NULL_POINTER_PARM                          ,        "DcmAPI_NULL_POINTER_PARM            ",
    MC_NULL_VALUE                          ,               "DcmAPI_NULL_VALUE                   ",
    MC_PROTOCOL_ERROR                          ,           "DcmAPI_PROTOCOL_ERROR               ",
    MC_REQUIRED_ATTRIBUTE_MISSING                   ,	   "DcmAPI_REQUIRED_ATTRIBUTE_MISSING   ",
    MC_REQUIRED_DATASET_MISSING                    ,       "DcmAPI_REQUIRED_DATASET_MISSING     ",
    MC_REQUIRED_VALUE_MISSING                      ,       "DcmAPI_REQUIRED_VALUE_MISSING       ",
    MC_STATE_VIOLATION                          ,          "DcmAPI_STATE_VIOLATION              ",
    MC_SYSTEM_CALL_INTERRUPTED                     ,       "DcmAPI_SYSTEM_CALL_INTERRUPTED      ",
    MC_SYSTEM_ERROR                          ,             "DcmAPI_SYSTEM_ERROR                 ",
    MC_TAG_ALREADY_EXISTS                          ,       "DcmAPI_TAG_ALREADY_EXISTS           ",
//
    MC_TEMP_FILE_ERROR                          ,          "DcmAPI_TEMP_FILE_ERROR                 " ,
    MC_TIMEOUT                          ,                  "DcmAPI_TIMEOUT                         " ,
    MC_TOO_FEW_VALUES                          ,           "DcmAPI_TOO_FEW_VALUES                  " ,
    MC_TOO_MANY_BLOCKS                          ,          "DcmAPI_TOO_MANY_BLOCKS                 " ,
    MC_TOO_MANY_VALUES                          ,          "DcmAPI_TOO_MANY_VALUES                 " ,
    MC_UNABLE_TO_CHECK_CONDITION               ,           "DcmAPI_UNABLE_TO_CHECK_CONDITION       " ,
    MC_UNACCEPTABLE_SERVICE                     ,          "DcmAPI_UNACCEPTABLE_SERVICE            " ,
    MC_UNEXPECTED_EOD                          ,           "DcmAPI_UNEXPECTED_EOD                  " ,
    MC_UNKNOWN_ITEM                          ,             "DcmAPI_UNKNOWN_ITEM                    " ,
    MC_UNKNOWN_SERVICE                          ,          "DcmAPI_UNKNOWN_SERVICE                 " ,
    MC_VALUE_MAY_NOT_BE_NULL                      ,        "DcmAPI_VALUE_MAY_NOT_BE_NULL           " ,
    MC_VALUE_NOT_ALLOWED                          ,        "DcmAPI_VALUE_NOT_ALLOWED               " ,
    MC_VALUE_OUT_OF_RANGE                          ,       "DcmAPI_VALUE_OUT_OF_RANGE              " ,
    MC_VALUE_TOO_LARGE                          ,          "DcmAPI_VALUE_TOO_LARGE                 " ,
	MC_VR_ALREADY_VALID                        ,           "DcmAPI_VR_ALREADY_VALID                ",
    MC_LIBRARY_ALREADY_INITIALIZED                     ,   "DcmAPI_LIBRARY_ALREADY_INITIALIZED     ",
    MC_LIBRARY_NOT_INITIALIZED                     ,       "DcmAPI_LIBRARY_NOT_INITIALIZED         ",
    MC_INVALID_DIRECTORY_RECORD_OFFSET            ,		   "DcmAPI_INVALID_DIRECTORY_RECORD_OFFSET ",
    MC_INVALID_FILE_ID                        ,            "DcmAPI_INVALID_FILE_ID                 ",
    MC_INVALID_DICOMDIR_ID                        ,        "DcmAPI_INVALID_DICOMDIR_ID             ",
//

    MC_INVALID_ENTITY_ID                        ,             "DcmAPI_INVALID_ENTITY_ID           "    ,
    MC_INVALID_MRDR_ID                        ,               "DcmAPI_INVALID_MRDR_ID             "    ,
    MC_UNABLE_TO_GET_ITEM_ID                        ,         "DcmAPI_UNABLE_TO_GET_ITEM_ID       "    ,
    MC_INVALID_PAD                        ,                   "DcmAPI_INVALID_PAD                 "    ,
    MC_ENTITY_ALREADY_EXISTS                        ,         "DcmAPI_ENTITY_ALREADY_EXISTS       "    ,
    MC_INVALID_LOWER_DIR_RECORD                        ,      "DcmAPI_INVALID_LOWER_DIR_RECORD    "    ,
    MC_BAD_DIR_RECORD_TYPE                        ,           "DcmAPI_BAD_DIR_RECORD_TYPE         "    ,
    MC_UNKNOWN_HOST_CONNECTED                        ,        "DcmAPI_UNKNOWN_HOST_CONNECTED      "    ,
    MC_INACTIVITY_TIMEOUT                        ,            "DcmAPI_INACTIVITY_TIMEOUT          "    ,
    MC_INVALID_SOP_CLASS_UID                        ,         "DcmAPI_INVALID_SOP_CLASS_UID       "    ,
    MC_INVALID_VERSION                        ,               "DcmAPI_INVALID_VERSION             "    ,
    MC_OUT_OF_ORDER_TAG                        ,              "DcmAPI_OUT_OF_ORDER_TAG            "    ,
    MC_CONNECTION_FAILED                        ,             "DcmAPI_CONNECTION_FAILED           "    ,
    MC_UNKNOWN_HOST_NAME                        ,             "DcmAPI_UNKNOWN_HOST_NAME           "    ,
    MC_INVALID_FILE                        ,                  "DcmAPI_INVALID_FILE                "    ,
	MC_NEGOTIATION_ABORTED                         ,          "DcmAPI_NEGOTIATION_ABORTED         "   ,
    MC_INVALID_SR_ID                         ,                "DcmAPI_INVALID_SR_ID               "   ,
    MC_UNABLE_TO_GET_SR_ID                         ,          "DcmAPI_UNABLE_TO_GET_SR_ID         "   ,
	MC_DUPLICATE_NAME                         ,               "DcmAPI_DUPLICATE_NAME              "   ,
	MC_DUPLICATE_SYNTAX                         ,             "DcmAPI_DUPLICATE_SYNTAX            "   ,
//
	MC_EMPTY_LIST                         ,                   "DcmAPI_EMPTY_LIST                   "  ,
	MC_MISSING_NAME                         ,                 "DcmAPI_MISSING_NAME                 "  ,
    MC_INVALID_SERVICE_NAME                         ,         "DcmAPI_INVALID_SERVICE_NAME         "  ,
	MC_SERVICE_IN_USE                         ,               "DcmAPI_SERVICE_IN_USE               "  ,
	MC_INVALID_SYNTAX_NAME                         ,          "DcmAPI_INVALID_SYNTAX_NAME          "  ,
	MC_SYNTAX_IN_USE                         ,                "DcmAPI_SYNTAX_IN_USE                "  ,
	MC_NO_CONTEXT                         ,                   "DcmAPI_NO_CONTEXT                   "  ,
	MC_OFFSET_TABLE_TOO_SHORT                         ,       "DcmAPI_OFFSET_TABLE_TOO_SHORT       "  ,
	MC_MISSING_DELIMITER                         ,            "DcmAPI_MISSING_DELIMITER            "  ,
	MC_COMPRESSION_FAILURE                         ,          "DcmAPI_COMPRESSION_FAILURE          "  ,
	MC_END_OF_FRAME                         ,                 "DcmAPI_END_OF_FRAME                 "  ,
	MC_MUST_CONTINUE_BEFORE_READING                         , "DcmAPI_MUST_CONTINUE_BEFORE_READING "  ,
	MC_COMPRESSOR_REQUIRED                         ,          "DcmAPI_COMPRESSOR_REQUIRED          "  ,
	MC_DECOMPRESSOR_REQUIRED                         ,        "DcmAPI_DECOMPRESSOR_REQUIRED        "  ,
	MC_DATA_AVAILABLE                         ,               "DcmAPI_DATA_AVAILABLE               "  ,
	MC_ZLIB_ERROR                         ,                   "DcmAPI_ZLIB_ERROR                   "  ,
	MC_NOT_META_SOP                         ,                 "DcmAPI_NOT_META_SOP                 "  ,
	MC_INVALID_ITEM_TRANSFER_SYNTAX                         , "DcmAPI_INVALID_ITEM_TRANSFER_SYNTAX "  ,
	MC_LICENSE_ERROR                         ,                "DcmAPI_LICENSE_ERROR                "  , 
    MC_MAX_OPERATIONS_EXCEEDED           ,                    "DcmAPI_MAX_OPERATIONS_EXCEEDED      "  ,
}  ;
 
const char *IDcmLibApi::Get_Error_Message(MC_STATUS AstatusCode)
{
	const char *ret_str = " ";
	int entry_size = sizeof(StatusMsgTbl)/sizeof(MC_StatusMsg);
	for(int i=0;i<entry_size;i++){
		if(StatusMsgTbl[i].m_MC_Status == AstatusCode){
			ret_str = StatusMsgTbl[i].m_MsgName;
		}
	}

	return ret_str;
}
