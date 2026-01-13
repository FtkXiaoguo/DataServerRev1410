// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

 
#ifdef USE_NEW_LIB

#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "api_logger.h"

#if 0
class MyLogger : public DcmLibApiLogger 
{
public:
	//
	virtual void Logger(int id,const char *str)
	{
		printf(" %d : %s \n",id,str);
	}
};
#endif

extern MyLogger _MyLogger_;

#if 1
typedef struct CALLBACKINFO
{
	FILE* fp;
	int messageID;
	char* serviceName;
	char prefix[30];
	unsigned long dataSize;

	CALLBACKINFO() { fp = 0; dataSize = 0; } // 09/26/2002 T.C. Zhao
	~CALLBACKINFO() { if (fp) { fclose(fp); fp = 0; }}
} CBinfo;

#if 0
 MC_STATUS AqFileObjToMedia( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    size_t     count;
    CBinfo*    cbInfo = (CBinfo*)A_userInfo;

 	// T.C. Zhao 2004.04.21. 
	// Added try/catch  for extra protection against bad data
	try  
	{
	   if (A_isFirst)
			cbInfo->fp = fopen(A_filename, "wb");
			
		//	Rob Lewis - 2004.04.27
		//	Added check for datasize.  If we are being called with something that is
		//		obviously too big, just fail.  NOTE:  the caller should set datasize
		//		before invoking this callback, otherwise this check will not happen.
		if (!cbInfo->fp || (cbInfo->dataSize > 0 && A_dataSize > cbInfo->dataSize))
			return MC_CANNOT_COMPLY;

		if(A_dataSize > 0)
		{
			extern int errno;
			errno = 0;
			count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfo->fp);
			if (count != (size_t)A_dataSize)
			{
				printf("fwrite error: %d",errno);
				return MC_CANNOT_COMPLY;
			}
		}

		if (A_isLast)
		{
			/*
			 * NULL ->fp so that the routine calling MC_Write file knows
			 * not to close the stream.
			 */
			fclose(cbInfo->fp);
			cbInfo->fp = NULL;
		}

		return MC_NORMAL_COMPLETION;
	}
	catch (...)
	{
		 printf("*** error\n");
		return MC_CANNOT_COMPLY;
	}

} /* FileObjToMedia() */
#endif

#endif
 int processCMove(int associationID,int messageID);
int processCFind(int associationID,int messageID);
int processCStore(int associationID,int messageID);
int tstMCSetServer1()
{
	const char *LocalAE = "MONE_AE" ;
	const char *RemoteAE = "TestMyAE";

	char    *RemoteHostname = "172.17.3.72"; 
    int     RemotePort = 105;
    char    *ServiceList = "testSeriveList"; 
	//

	MC_STATUS mcStatus;
 
	int applicationID;
	int associationID;
 
	if(!IDcmLibApi::DcmLibInitialization(	 NULL, NULL, NULL ))
	{
		printf("Unable to initialize library", mcStatus);
        return ( EXIT_FAILURE );
	}

	IDcmLibApi::setupLoger(&_MyLogger_,IDcmLib::LOGLEVEL_TRACE);

	MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, 20);
	MC_Set_Int_Config_Value(CONNECT_TIMEOUT, 20);
 	MC_Set_Int_Config_Value(WRITE_TIMEOUT, 20);
 	MC_Set_Int_Config_Value(RELEASE_TIMEOUT, 20);
 	MC_Set_Int_Config_Value(INACTIVITY_TIMEOUT, 20);	

    /*
     *  Register this DICOM application
     */
    if(!IDcmLibApi::Register_Application(&applicationID, LocalAE))
    {
        printf("Unable to register \"%s\":\n", LocalAE);
   //     printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }
 


	mcStatus = MC_Set_Int_Config_Value( TCPIP_LISTEN_PORT,105 );
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        return false;
    }


	for(int i=0;i<10000;i++){
		/*
		 *   Open association and override hostname & port parameters if 
		 *   they were supplied on the command line.
		 */
		DcmXtError errorCode = IDcmLibApi::Wait_For_Association(ServiceList,
                                                    1,
                                                    &applicationID,
													&associationID);
	 
		switch(errorCode)
		{
		case DcmXtErr_Normal:
		;//
			break;
		case DcmXtErr_Timeout:
			printf("wait for association with \"%s\": -- timeout \n", RemoteAE);
			continue;
			break;
		default:
			printf("wait for association with \"%s\": -- error\n", RemoteAE);
			return(EXIT_FAILURE);
			break;
		}
                                    
	 
		/*
		*
		*/
		AssocInfo   asscInfo;
		mcStatus = MC_Get_Association_Info( associationID, &asscInfo); 
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Association_Info failed", mcStatus);
		}
		else
		{
        
			{
				printf("Connection from Remote Application:\n");
				printf("   AE Title:                 %s\n", asscInfo.RemoteApplicationTitle);
				printf("   Host name:                %s\n", asscInfo.RemoteHostName);
				printf("   IP Address:               %s\n", asscInfo.RemoteIPAddress);
				printf("   Implementation Version:   %s\n", asscInfo.RemoteImplementationVersion);
				printf("   Implementation Class UID: %s\n", asscInfo.RemoteImplementationClassUID);
				printf("   Requested AE Title:       %s\n\n\n", asscInfo.LocalApplicationTitle);
			}
		}

		 
		mcStatus = MC_Accept_Association(associationID);


		for(int run_read_msg = 0;run_read_msg<100;run_read_msg++){

			int   MessageID;
			 char  *ServiceName=0;
			 MC_COMMAND    Command;
			mcStatus = MC_Read_Message( associationID,
												 50,
												   &MessageID,
												   &ServiceName,
												   &Command);
                                    
			if(mcStatus == MC_ASSOCIATION_CLOSED){
				printf("MC_Read_Message MC_ASSOCIATION_CLOSED:\n" );
				break;
			}

			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Read_Message error:\n" );
   
				break;
			//	return(EXIT_FAILURE);
			}

			char _str_buff[1024];
		//	MC_Get_Value_From_String();

			
 
			CBinfo cbinfo;
			//	Convert to a DICOM Part 10 file object
			 
			int fileID = MessageID;
			mcStatus = MC_Message_To_File(fileID,"write_reved_dat.dcm");
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Message_To_File error:\n" );
  
				return(EXIT_FAILURE);
			}

			 
			 
			mcStatus = MC_Get_File_Length(fileID, &cbinfo.dataSize);
			if (mcStatus != MC_NORMAL_COMPLETION)
				cbinfo.dataSize = 0;


			//	Write the file to disk
			mcStatus = MC_Write_File(fileID, 0, &cbinfo, AqFileObjToMedia);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				 printf("MC_Write_File error:\n" );
  
				return(EXIT_FAILURE);
			}
 
			switch(Command){
			case C_MOVE_RQ:
				printf(">>>> C_MOVE_RQ \n");
				processCMove(associationID,MessageID);
				break;

			case C_STORE_RQ:
				printf(">>>> C_STORE_RQ \n");
				processCStore(associationID,MessageID);
				break;
			case C_FIND_RQ:
				printf(">>>> C_FIND_RQ \n");
				processCFind(associationID,MessageID);
				break;
			}


			mcStatus = MC_Free_Message(&MessageID);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Free_Message for response message failed", mcStatus);
		//        MC_Abort_Association(&associationID);
				return(EXIT_FAILURE);
			}

		}//run_read_msg
			//
		mcStatus = MC_Close_Association(&associationID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf("MC_Close_Association failed", mcStatus);
	//        MC_Abort_Association(&associationID);
	//		return(EXIT_FAILURE);
		}
		//

		

		IDcmLibApi::CheckMemory();

 
	} //loop

	MC_Library_Release();

	_CrtDumpMemoryLeaks();


	return 0;
}

 