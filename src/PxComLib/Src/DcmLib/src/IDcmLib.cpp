// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

 
#include "IDcmLib.h"
 

#include "DcmXTDataSetMain.h"

#include "DcmXTAssociationClientMain.h"
#include "DcmXTAssociationServerMain.h"
#include "DcmXTAssociationListenerMain.h"

#include "DcmXTDicomMessageMain.h"

#include "InstanceIDMan.h"

#include "DcmXTUtilMain.h"
#include "DcmXTApplicationInstanceMain.h"

using namespace XTDcmLib;

#include "CheckMemoryLeak.h"
 
IDcmLibDefDllCls void*   DllAllocate(size_t size)
{
    return malloc(size);
}
IDcmLibDefDllCls void    DllFree(void* ptr)
{
    free(ptr);
}





#define DcmXTUtilInstancePtr ((DcmXTUtilMain*)m_UtilInstance)

#define DcmXTApplicationInstancePtr ((DcmXTApplicationInstanceMain*)m_AppInstance)

#define DcmXTAssociationListenerPtr ((DcmXTAssociationListenerMain*)m_AssociationListenerHnd)

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////
 
void testIDMan();

IDcmLib::IDcmLib()
{
//	 testIDMan();
	
}

IDcmLib::~IDcmLib()
{
 
}

IDcmLib* IDcmLib::createInstance()
{
	IDcmLib *dll= new IDcmLib;
 
 	dll->m_UtilInstance = new DcmXTUtilMain;
	dll->m_AppInstance  = new DcmXTApplicationInstanceMain;

	dll->m_AssociationListenerHnd = new DcmXTAssociationListenerMain;
	//
	DCMDATA_TRACE("createInstance \n ");

	return ((IDcmLib*)dll);
}


void IDcmLib::destroy()
{
 	 DcmXTUtilInstancePtr->Delete();
	 DcmXTApplicationInstancePtr->Delete();

	 DcmXTAssociationListenerPtr->Delete();

	 delete this;


}
 
#ifdef _MemoryLeakCheck 	
_CrtMemState _CurStats; 
#endif
 
void test_dataset();
unsigned int IDcmLib::CheckMemory()
{
	unsigned int usedSize=0;
#ifdef _MemoryLeakCheck 
	_CrtMemState s1,ss;
	_CrtMemCheckpoint(&s1);
	if(_CrtMemDifference(&ss,&_CurStats,&s1)){
		//
		bool dump_all = false;
		if(dump_all){
			_CrtMemDumpAllObjectsSince(&_CurStats);
		}
		_CrtMemDumpStatistics( &ss );
		_CrtMemCheckpoint(&_CurStats);
		//
		bool dump_flag = false;
		if(dump_flag){
			_CrtDumpMemoryLeaks();
		}
	}
#endif

	
//	test_dataset();

	return usedSize;
}

DcmXTUtil			*IDcmLib::getUtil()
{
	return DcmXTUtilInstancePtr;
}

static DcmXTMutexMain _MultiThread_Mutex_;
DcmXTMutex			*IDcmLib::getMutex()
{
	return &_MultiThread_Mutex_;
}

DcmXTDicomMessage		*IDcmLib::createDicomMessage()
{
	DcmXTDicomMessage *retP = new DcmXTDicomMessageMain;

	return retP;
}

DcmXTDataSet *IDcmLib::createDataSet()
{
	DcmXTDataSet *retP = new DcmXTDataSetMain;

	return retP;
}

DcmXTAssociationClient *IDcmLib::createDcmAssociationClient()
{
	DcmXTAssociationClient *retP = 0;

	retP = new DcmXTAssociationClientMain;

	return retP;
}
DcmXTAssociationServer *IDcmLib::createDcmAssociationServer(DcmXTAssociationListener *wait_aso)
{
	DcmXTAssociationServerMain *retP = 0;

	DcmXTAssociationListenerMain *AssociationListenerMain = (DcmXTAssociationListenerMain *)wait_aso;
	retP = new DcmXTAssociationServerMain(AssociationListenerMain->getAssociationTemp());
               
	retP->setApplicationInstance(DcmXTApplicationInstancePtr);
	return retP;
}
DcmXTAssociationListener *IDcmLib::getAssociationListener()
{
	DcmXTAssociationListenerPtr->setApplicationInstance(DcmXTApplicationInstancePtr);
	return DcmXTAssociationListenerPtr;
	 
}
 
DcmXTApplicationInstance *IDcmLib::getApplicationInstance()
{
	
	return DcmXTApplicationInstancePtr;
}
 
#if 0
void IDcmLib::openLogger(const char *fileName,LOG_LEVEL log_level)
{
	DcmXTUtilInstancePtr->openLogger( fileName, log_level);
}
#endif
void IDcmLib::setupLoger(XTDcmLib::DcmLibApiLogger *logger,IDcmLib::LOG_LEVEL log_level)
{
	DcmXTUtilInstancePtr->setupApiLogger( logger,log_level);
}
//#18 2012/05/17 K.Ko
 void  IDcmLib::setupWriteBufferLen(int len)
{
	 
	DcmXTUtilInstancePtr->setupWriteBufferLen(len);
	 
}
void  IDcmLib::setupReadeBufferLen(int len)
{
	 
	DcmXTUtilInstancePtr->setupReadeBufferLen(len);
	 
}