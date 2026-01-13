// tstDcmLib.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#ifdef USE_NEW_LIB

#include "IDcmLib.h"
#include "IDcmLibApi.h"
#include "IDcmLibDefUID.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif


#ifdef USE_NEW_LIB

const char *LocalAE = "TestQR" ;
const char *RemoteAE = "MONE_AE";
//const char *RemoteAE = "FCOLDMOON_AE";

 char    *RemoteHostname = "127.0.0.1"; 
//	char    *RemoteHostname = "172.17.23.21"; 

int     RemotePort = 6103;
char    *ServiceList = "testSeriveList"; 
//char	*dicom_fileName= "C:\\temp\\dx_dicom\\kodak_H299.dcm";
//char	*dicom_fileName= "C:\\temp\\dx_dicom\\ge_IM1.dcm";
char	*dicom_fileName= "C:\\ext_data\\temp\\impXml.dcm";
//	char	*dicom_fileName= "testdata.dcm"

int applicationID;
int initLib()
{
 
	int mcStatus = MC_Library_Initialization ( NULL, NULL, NULL ); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to initialize library", mcStatus);
        return ( EXIT_FAILURE );
    }

//	IDcmLibApi::openLogger("dcmlibAPI.log",IDcmLib::LOGLEVEL_TRACE );

    /*
     *  Register this DICOM application
     */
    mcStatus = MC_Register_Application(&applicationID, LocalAE);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to register \"%s\":\n", LocalAE);
   //     printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }

	//
	IDcmLibApi::getDcmXTUtil()->createServiceList(true /*isPropose*/,ServiceList);
		
	IDcmLibApi::getDcmXTUtil()->setNetworkTransferSyntax(true/* isPropose*/,IDcmLibApi::ApiTransferSyntaxToDcmLib(IMPLICIT_LITTLE_ENDIAN),ServiceList);

	IDcmLibApi::getDcmXTUtil()->addSOPClassUID(true/*isPropose*/, UID_VerificationSOPClass, ServiceList); //for send Echo-SCU
 
	return 0;

}
void releaseLib()
{
	MC_Library_Release();
}
void tstDcmDataSet()
{
	

	IDcmLib *dcmlib_instance = IDcmLib::createInstance();

//	DcmDataSet *dataset_instance = dcmlib_instance->createDcmDataSet();
//	dataset_instance->readFile("E:\\temp\\testdata_out.dcm");

 
	DcmXTDicomMessage *dcm_file_instance = dcmlib_instance->createDicomMessage();

	dcm_file_instance->setMaxReadLength(16);

//	dcm_file_instance->readFile("E:\\temp\\testdata_out.dcm");
	dcm_file_instance->readFile("E:\\temp\\testdata1.dcm");

//	dcm_file_instance->readFromDumpFile("E:\\temp\\testdata1.dcm.txt");
//	dcm_file_instance->readFromDumpFile("E:\\temp\\QR_Image.dcm.txt");


	DcmXTDataSet *dataset = dcm_file_instance->getDcmXTDataSet();

	DcmXTComInterface *comInterface = (DcmXTComInterface *)dataset;
	comInterface->Set_Value(0x00080022,"tttttttttttt");
	comInterface->Set_Value(0x00280010,256);

	char string_val[1024];

	dcm_file_instance->Get_Value(0x00200037,string_val,1024);

	dcm_file_instance->Get_Value(0x00080020,string_val,1024);

	dcm_file_instance->Get_Value(0x00020010,string_val,1024);

	dcm_file_instance->writeToDumpFile("test_dcm.txt");
}

#endif


 
int tstStoreSCU()
{

	

#if 1
	 

	MC_STATUS mcStatus;
 
	int fileID;
	int messageID;
	 
	int associationID;
 
	{
#if 1
		mcStatus = MC_Create_Empty_File(&fileID,dicom_fileName);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf("MC_Create_Empty_File erro \n");
	   
			return(EXIT_FAILURE);
		}
		//
		mcStatus = MC_Open_File(applicationID,fileID,0,0);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf("MC_Open_File erro \n");
	   
			return(EXIT_FAILURE);
		}
	 	
		//
		mcStatus = MC_File_To_Message(fileID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf("MC_File_To_Message erro \n");
	   
			return(EXIT_FAILURE);
		}
		messageID = fileID;
#else
		mcStatus =  MC_Open_Empty_Message(&messageID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
		 
			return false;
		}

		//	Added by RL & VS 12/20/01 - We think this should be here.because Open_Empty_Message doesn't set it?
		mcStatus =  MC_Set_Value_From_Int(messageID, MC_ATT_MESSAGE_ID, messageID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			return false;
		}
#endif

		 
		MC_Set_Value_From_String(messageID ,0x00080018,"1.2.392.200183.2006.001.00000000.20061102111430.000.1.5");

		MC_Set_Value_From_String(messageID ,0x00100010,"test test test");
		MC_Set_Value_From_String(messageID ,0x00100020,"11100") ;

		MC_Set_Value_From_String(messageID ,0x0020000d,"1.2.392.200183.2006.001.00000000.20061102111158.000.2");//] #  54, 1 StudyInstanceUID
		MC_Set_Value_From_String(messageID ,0x0020000e,"1.2.392.200183.2006.001.00000000.20061102111430.000.100.3");// #  58, 1 SeriesInstanceUID

	//	MC_Set_Value_From_String(messageID ,0x0018a001,"0");
	//	
		if(0){
			/*
			* send scene message
			*/
			DcmXTDicomMessage *dcm_file_instance = IDcmLibApi::get_DcmMessage(fileID);
			dcm_file_instance->readFile("AqnetScene.dcm");

			
 
		}
		{
			DcmXTDicomMessage *dcm_file_instance = IDcmLibApi::get_DcmMessage(fileID);
	//		dcm_file_instance->readFile("testdata.dcm");
	//		dcm_file_instance->readFile("C:\\temp\\pano_dcm_cnvt\\K8000_cnvt.dcm");
			dcm_file_instance->writeToDumpFile("test_dcm.txt");
		}
			
		if(0){
			DcmXTDicomMessage *dcm_file_instance = IDcmLibApi::get_DcmMessage(fileID);
			dcm_file_instance->writeFile("C:\\temp\\dbg_send.dcm");

		}
 
		//test setting  IMPLEMENTATION_CLASS_UID and IMPLEMENTATION_VERSION
		if(0){
			char      uidBuffer[65];
			strncpy(uidBuffer, "1.2.840.10008.5.1.4.1.1.7", sizeof uidBuffer);
			uidBuffer[sizeof(uidBuffer)-1] =0;

			mcStatus = MC_Set_String_Config_Value(IMPLEMENTATION_CLASS_UID,  uidBuffer);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Set_String_Config_Value error\n");
			}

			mcStatus = MC_Get_String_Config_Value(IMPLEMENTATION_CLASS_UID, sizeof(uidBuffer), uidBuffer);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Get_String_Config_Value error\n");
			}
			//
			mcStatus = MC_Set_String_Config_Value(IMPLEMENTATION_VERSION,  "PXDicomLib");
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Set_String_Config_Value error\n");
			}

			mcStatus = MC_Get_String_Config_Value(IMPLEMENTATION_VERSION, sizeof(uidBuffer), uidBuffer);
			if (mcStatus != MC_NORMAL_COMPLETION)
			{
				printf("MC_Get_String_Config_Value error\n");
			}
		}
		

	//	dcm_file_instance->writeToDumpFile("test_dcm.txt");
	}

	/*
     *   Open association and override hostname & port parameters if 
     *   they were supplied on the command line.
     */
    mcStatus = MC_Open_Association( applicationID, &associationID,
                                    RemoteAE,
                                    &RemotePort, 
                                    RemoteHostname,
                                    ServiceList );
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n", RemoteAE);
  //      printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }

	AssocInfo               asscInfo;
	mcStatus = MC_Get_Association_Info( associationID, &asscInfo); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Get_Association_Info failed %d \n", mcStatus);
    }

   
    if(1){
        printf("Connecting to Remote Application:\n");
        printf("  Remote AE Title:          %s\n", asscInfo.RemoteApplicationTitle);
        printf("  Local AE Title:           %s\n", asscInfo.LocalApplicationTitle);
        printf("  Host name:                %s\n", asscInfo.RemoteHostName);
        printf("  IP Address:               %s\n", asscInfo.RemoteIPAddress);
        printf("  Local Max PDU Size:       %d\n", asscInfo.LocalMaximumPDUSize);
        printf("  Remote Max PDU Size:      %d\n", asscInfo.RemoteMaximumPDUSize);
        printf("  Max operations invoked:   %d\n", asscInfo.MaxOperationsInvoked);
        printf("  Max operations performed: %d\n", asscInfo.MaxOperationsPerformed);
        printf("  Implementation Version:   %s\n", asscInfo.RemoteImplementationVersion);
        printf("  Implementation Class UID: %s\n\n\n", asscInfo.RemoteImplementationClassUID);
        
        printf("Services and transfer syntaxes negotiated:\n");
        
        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are 
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
		 ServiceInfo             servInfo;
        mcStatus = MC_Get_First_Acceptable_Service(associationID,&servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
 //           printf("  %-30s: %s\n",servInfo.ServiceName, 
//                              GetSyntaxDescription(servInfo.SyntaxType));

#ifdef USE_NEW_LIB
				printf("  %-30s: %s\n",servInfo.ServiceName, 
						 GetSyntaxDescription(servInfo.SyntaxType) );
#else
				printf("  %-30s: %d\n",servInfo.ServiceName, servInfo.SyntaxType  );
#endif
            
        
            mcStatus = MC_Get_Next_Acceptable_Service(associationID,&servInfo);
        }
            
        if (mcStatus != MC_END_OF_LIST)
        {
            printf("Warning: Unable to get service info %d ",mcStatus);
        }
        
        printf("\n\n");
    }

	//
	 char   SOPClassUID[64+2];    /* SOP Class UID of the file */
    char   serviceName[48];             /* MergeCOM-3 service name for SOP Class */
    char   SOPInstanceUID[64+2]; /* SOP Instance UID of the file */

	mcStatus = MC_Get_Value_To_String(messageID, 
                        MC_ATT_SOP_CLASS_UID,
                        sizeof(SOPClassUID),
                        SOPClassUID);

    /* Get the SOP class UID and set the service */
    mcStatus = MC_Get_MergeCOM_Service(SOPClassUID, serviceName, sizeof(serviceName));
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
         return false;
    }            
                 
    mcStatus = MC_Set_Service_Command(messageID, serviceName, C_STORE_RQ);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
       return false;
    }

	 

	//
	mcStatus = MC_Send_Request_Message(  associationID, messageID);
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Send_Request_Message error \n");
  //      printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }

	 


	mcStatus = MC_Free_Message(&messageID);
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Free_Message error \n");
  //      printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }


	/*
     *  Wait for response
     */
	int               A_timeout = 30; /*Sec*/
	int             responseMessageID;
	char*           responseService;
    MC_COMMAND      responseCommand;
    mcStatus = MC_Read_Message(associationID, A_timeout, &responseMessageID,
                 &responseService, &responseCommand);
    if (mcStatus == MC_TIMEOUT){
		printf("*** CTstStoreSCU::SendImage MC_Read_Message error MC_TIMEOUT [%d]\n",mcStatus );
		 
		return false;
	}else{
		printf(" === Wait for response OK \n");
	}
      
    


	///
	MC_Close_Association( &associationID);

#if 0
	for(int run_i=0;run_i<100;run_i++){
			MC_Open_Association( applicationID, &associationID,
											RemoteAE,
											&RemotePort, 
											RemoteHostname,
											ServiceList );
			MC_Close_Association( &associationID);
	}
#endif


#endif
 
	return 0;
}


int tstEchoSCU()
{
	

#if 1
	 

	MC_STATUS mcStatus;
 
	int fileID;
	int echoMsgID;
	 
	int associationID;
 
	

	/*
     *   Open association and override hostname & port parameters if 
     *   they were supplied on the command line.
     */
    mcStatus = MC_Open_Association( applicationID, &associationID,
                                    RemoteAE,
                                    &RemotePort, 
                                    RemoteHostname,
                                    ServiceList );
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("Unable to open association with \"%s\":\n", RemoteAE);
  //      printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }

	AssocInfo               asscInfo;
	mcStatus = MC_Get_Association_Info( associationID, &asscInfo); 
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Get_Association_Info failed %d \n", mcStatus);
    }

   
    if(1){
        printf("Connecting to Remote Application:\n");
        printf("  Remote AE Title:          %s\n", asscInfo.RemoteApplicationTitle);
        printf("  Local AE Title:           %s\n", asscInfo.LocalApplicationTitle);
        printf("  Host name:                %s\n", asscInfo.RemoteHostName);
        printf("  IP Address:               %s\n", asscInfo.RemoteIPAddress);
        printf("  Local Max PDU Size:       %d\n", asscInfo.LocalMaximumPDUSize);
        printf("  Remote Max PDU Size:      %d\n", asscInfo.RemoteMaximumPDUSize);
        printf("  Max operations invoked:   %d\n", asscInfo.MaxOperationsInvoked);
        printf("  Max operations performed: %d\n", asscInfo.MaxOperationsPerformed);
        printf("  Implementation Version:   %s\n", asscInfo.RemoteImplementationVersion);
        printf("  Implementation Class UID: %s\n\n\n", asscInfo.RemoteImplementationClassUID);
        
        printf("Services and transfer syntaxes negotiated:\n");
        
        /*
         * Go through all of the negotiated services.  If encapsulated /
         * compressed transfer syntaxes are supported, this code should be
         * expanded to save the services & transfer syntaxes that are 
         * negotiated so that they can be matched with the transfer
         * syntaxes of the images being sent.
         */
		 ServiceInfo             servInfo;
        mcStatus = MC_Get_First_Acceptable_Service(associationID,&servInfo);
        while (mcStatus == MC_NORMAL_COMPLETION)
        {
 //           printf("  %-30s: %s\n",servInfo.ServiceName, 
//                              GetSyntaxDescription(servInfo.SyntaxType));

#ifdef USE_NEW_LIB
				printf("  %-30s: %s\n",servInfo.ServiceName, 
						 GetSyntaxDescription(servInfo.SyntaxType) );
#else
				printf("  %-30s: %d\n",servInfo.ServiceName, servInfo.SyntaxType  );
#endif
            
        
            mcStatus = MC_Get_Next_Acceptable_Service(associationID,&servInfo);
        }
            
        if (mcStatus != MC_END_OF_LIST)
        {
            printf("Warning: Unable to get service info %d ",mcStatus);
        }
        
        printf("\n\n");
    }

	{

		echoMsgID = -1;
		mcStatus = MC_Open_Empty_Message(&echoMsgID);
		if(mcStatus != MC_NORMAL_COMPLETION) 
		{
			printf("ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed to open Empty Message for E-ECHO \n",associationID, mcStatus); 
			return mcStatus;
		}

		mcStatus = MC_Set_Service_Command(echoMsgID, "STANDARD_ECHO", C_ECHO_RQ);
		if(mcStatus != MC_NORMAL_COMPLETION) 
		{
			printf( "ERROR:(%d) CMove::Process() - Error (%d,%s) - Failed to set Service Command for E-ECHO\n",associationID, mcStatus); 
			return mcStatus;
		}

	}

	 

	//
	mcStatus = MC_Send_Request_Message(  associationID, echoMsgID);
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Send_Request_Message error \n");
  //      printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }

	 
	////////////
	//	Read the C-ECHO-RSP message
	int echoRsp = -1;
	MC_COMMAND	command;
	char*	serviceName;
	mcStatus =  MC_Read_Message(associationID, 10, &echoRsp, &serviceName, &command);
	if(mcStatus != MC_NORMAL_COMPLETION)
	{
		printf( "ERROR:(%d) CMove::Process() - Error (%d) - Failed on attempt to Read C-ECHO-RSP\n",associationID, mcStatus); 
		return mcStatus;
	} 


	if(command != C_ECHO_RSP)
	{
		printf( "ERROR:(%d) CMove::Process() - Error (%d) - Failed on attempt to Read C-ECHO-RSP\n",associationID, mcStatus); 
		return mcStatus;
	}
	
	unsigned int echoRspStatus = -1;
	mcStatus = MC_Get_Value_To_UInt(echoRsp, MC_ATT_STATUS, &echoRspStatus);
	if(mcStatus != MC_NORMAL_COMPLETION)
	{
		printf( "ERROR:(%d) CMove::Process() - Error (%d) - Failed on attempt to Read C-ECHO-RSP Response Status\n",associationID, mcStatus); 
		return mcStatus;
	} 

	if (echoRspStatus != C_ECHO_SUCCESS)
	{
		printf( "ERROR:(%d) CMove::Process() - Error (%d) - Bad C-ECHO-RSP Response Status %d\n",associationID, mcStatus, echoRspStatus); 
		return mcStatus;
	}


	//////////

	mcStatus = MC_Free_Message(&echoMsgID);
                                    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Free_Message error \n");
  //      printf("\t%s\n", MC_Error_Message(mcStatus));
        return(EXIT_FAILURE);
    }


 
    


	///
	MC_Close_Association( &associationID);

 

#endif
 
	return 0;
}