// TstCMoveSCU.cpp: CTstCMoveSCU クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)

#include <ctime>

#include "TstQRSCUApp.h"

#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#include "IDcmLibApi.h "
using namespace XTDcmLib;
#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif


#if 0

#include "rtvloadoption.h"
 
bool CTstQRSCUApp::sendCMoveCmd()
{
	MC_STATUS            status;
    int                  messageID;
    int                  responseMessageID;

	char                 *model = "STUDY_ROOT_QR_MOVE" ;


	status = MC_Open_Message ( &messageID, model, C_MOVE_RQ );     
    if ( status != MC_NORMAL_COMPLETION )
    {
         
        return false;
    }

	/*
     * Set the Query/Retrieve Level
     */
    status = MC_Set_Value_From_String ( messageID,
                                        MC_ATT_QUERY_RETRIEVE_LEVEL,
                                        "STUDY" );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("MC_Set_Value_From_String %d NULL", status);
        return ( false );
    }

	/* 
     * Set move destination.  This is the AE title where the 
     * Query SCP will store the images that are requested to
     * be moved.  The default is to sent back to this AE.  
     * You can also place the AE title of another system in 
     * this field.
     */
    status = MC_Set_Value_From_String (messageID,
                                       MC_ATT_MOVE_DESTINATION,
                                       m_CMoveDestAE.c_str());
    if ( status != MC_NORMAL_COMPLETION )
    {
       
        return ( false );
    }

	status = MC_Set_Value_From_String (messageID,
										MC_ATT_QUERY_RETRIEVE_LEVEL,
                                       "STUDY" );
    if ( status != MC_NORMAL_COMPLETION )
    {
         
        return ( false );
    }

	if ( SetValue ( messageID, MC_ATT_STUDY_INSTANCE_UID,
            m_curStudyInstanceUID.c_str(), NULL, 1 ) == false )
            return ( false );

	/*
     * Send off the message 
     */
    status = MC_Send_Request_Message ( m_associationID,
                                       messageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &messageID );
        
        return ( false );
    }

    status = MC_Free_Message ( &messageID );
    if (status != MC_NORMAL_COMPLETION )
    {
        ;
    }
	return true;
}
bool CTstQRSCUApp::waitCMoveRSP()
{
	MC_STATUS            status;
 
    int                  responseMessageID;

	MC_COMMAND           command;
	char                 *serviceName;

	unsigned int         response;
	 /*
     * Wait for the C-MOVE-RSP messages and activity over
     * the storage associatin such as the association being
     * opened or messages received.
     */
    bool continueLoop = true;
	int time_counter = 0;
    while ( continueLoop  )
    {
        /* 
         * Do not wait for a C-MOVE-RSP message.
         */
        status = MC_Read_Message ( m_associationID, 20/*Sec*/,
            &responseMessageID, &serviceName, &command );
        switch ( status )
        {
            case MC_TIMEOUT:
                /*
                 * Nothin received, now we should poll the store association
                 */
				if(time_counter++ > 2){
					printf(" long time out..., stop it.\n");
					continueLoop = false;
				}
				
                break;
            case MC_NORMAL_COMPLETION:

				printf("-");
      //          ValMessage      (moveResponseMessageID, "C-MOVE-RSP");
        //        WriteToListFile (moveResponseMessageID, "cmoversp.msg");

                status = MC_Get_Value_To_UInt(responseMessageID,
                    MC_ATT_STATUS, &response);
                if (status != MC_NORMAL_COMPLETION) {
					;;
				}
                    
    
                status = MC_Free_Message ( &responseMessageID );
                 
                
                /*
                 * Check the status returned in the response message.
                 */
                if ( response == C_MOVE_SUCCESS_NO_FAILURES )
				{
                    /*
                     * The move has completed!
                     */
					 printf("\n");
					 printf("end\n");
                    continueLoop = false;
				}
                else if( response != C_MOVE_PENDING_MORE_SUB_OPERATIONS )
                {
                    printf("Response for C_MOVE  Error in CMOVE operation \n");
                   return false;
                } 
    
                break;
            default:
                MC_Free_Message ( &responseMessageID );
                printf("MC_Read_Message for move response  error %d ", status );
                return ( false );

        } /* switch status of move response read message */

#if 0
	//donot use CSTOREAssociation here
       // qrStatus = ProcessCSTOREAssociation(A_root_data->level, A_myConfig);
     //   if ( qrStatus != QR_SUCCESS )
       //     return ( qrStatus );
		if(!ProcessCSTOREAssociation()){
			return true;
		}
#endif

    } /* continueLoop == TRUE */

	return true;
}
bool CTstQRSCUApp::ProcessCSTOREAssociation()
{
	// not use it here!

	char str_buffer[256];
	static int            S_storeAssociationID = -1;
    static int            S_numImages = -1;
    MC_COMMAND            command;
    MC_STATUS             status;
    int                   storeMessageID;
    int                   calledApplicationID;
    int                   responseMessageID;
    int                   waitStoreAssociationID = -1;
    char*                 serviceName;
    CBinfo                callbackInfo;
    char                  fileName[256];
    char                  listFileName[256];
 
    static char           S_prefix[] = "ProcessCSTOREAssociation";


	if ( S_storeAssociationID == -1)
    {
        /* 
         * Check if any associations have been requested
         */
        status = MC_Wait_For_Association ( "TIDICOMServer_Service_List",
                        0,
                        &calledApplicationID,
                        &waitStoreAssociationID );
        switch ( status )
        {
            case MC_NORMAL_COMPLETION:
                printf ( " Successful connection from remote.\n" );
				printf ( " C-MOVE Receive DICOM ########################################## \n" );

                S_storeAssociationID = waitStoreAssociationID;
                status = MC_Accept_Association ( S_storeAssociationID );
                if ( status != MC_NORMAL_COMPLETION )
                {
                    printf("MC_Accept_Association  error %d \n",status );
                    return ( false );
                }
                break;
    
            case MC_NEGOTIATION_ABORTED:
                printf("Association aborted during negotiation, continuing\n");
                break;
                
            case MC_TIMEOUT:
                break;
            default:
                 printf( "MC_Wait_For_Association  error  %d \n",status);
                 return ( false );
        } /* switch status of MC_Wait_For_Association for store association */
    }

	/*
     * If we do not have an association open, return from this function,
     * or else look for a message on currently open association.
     */
    if (S_storeAssociationID == -1)
        return ( true );

	//
	for (;;)
    {
        /*
         * A for loop is used here incase activity is "queued up"
         * on this association.  If a timeout occurs, or the 
         * association is dropped, we will abort out of this loop.
         * Most of the time is spent here
         */
        status = MC_Read_Message ( S_storeAssociationID, 0,
                                   &storeMessageID, 
                                   &serviceName, &command );
        if (status == MC_TIMEOUT)
            return ( true );
        else if (status == MC_ASSOCIATION_CLOSED
              || status == MC_ASSOCIATION_ABORTED
              || status == MC_NETWORK_SHUT_DOWN)
        {
            printf("Storage association closed\n");
            S_storeAssociationID = -1;
            return ( true );
        }
        else if (status != MC_NORMAL_COMPLETION)
        {
            printf("Aborting storage association\n");
            MC_Abort_Association ( &S_storeAssociationID );
            printf("MC_Read_Message %d \n", status);
            S_storeAssociationID = -1;
            return ( false );
        }
        else
        {         
            /*
             * We have successfully received a C-STORE 
             * message.  It will now be saved to disk.
             */   
            S_numImages++;

            /* 
             * Make a file name to hold the image 
             */
         

       //     ValMessage      (storeMessageID, "C-STORE-RQ");
       //     WriteToListFile (storeMessageID, listFileName );

            /* 
             * Stream the message into a file 
             */
            
			printf("C-MOVE: Reived DICOM Message : %d\n",S_numImages);
 
			if ( GetValue ( storeMessageID, MC_ATT_PATIENTS_NAME, 
                str_buffer,
				sizeof ( str_buffer ),
				"") == false )
				return ( false );
				printf("Patient Name %s ",str_buffer);
			//
			if ( GetValue ( storeMessageID, MC_ATT_PATIENT_ID, 
                str_buffer,
				sizeof ( str_buffer ),
				"") == false )
				return ( false );
				printf("ID [ %s ]  ",str_buffer);

			//
			if ( GetValue ( storeMessageID, MC_ATT_STUDY_ID,
				str_buffer,
				sizeof ( str_buffer ),
				"") == false )
				return ( false );
			 printf("Study ID %s",str_buffer);
			 //
			 if ( GetValue ( storeMessageID, MC_ATT_SERIES_INSTANCE_UID,
				str_buffer,
				sizeof ( str_buffer ),
				"") == false )
				return ( false );
			 printf("SeriesInstanceUID %s \n",str_buffer);

            /* 
             * Send a successful response 
             */
            status = MC_Open_Message ( &responseMessageID,
                                       serviceName,
                                       C_STORE_RSP );
            if ( status != MC_NORMAL_COMPLETION )
            {
                printf("MC_Open_Message error %d \n",status);
                break;
            }

            status = MC_Send_Response_Message ( S_storeAssociationID,
                                                C_STORE_SUCCESS,
                                                responseMessageID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                MC_Free_Message ( &responseMessageID );
                printf("MC_Send_Response_Message error %d \n",status);
                  
                break;
            }

            status = MC_Free_Message ( &responseMessageID );
            if ( status != MC_NORMAL_COMPLETION )
            {
                 ;
                break;
            }

            status = MC_Free_Message (&storeMessageID);
            if (status != MC_NORMAL_COMPLETION )
            {
                 ;
                break;        
            }
        }
    } /* for (;;) */

    if (status != MC_NORMAL_COMPLETION)
    {
        printf("Storage association aborted\n");
        MC_Free_Message ( &storeMessageID );
        MC_Abort_Association ( &S_storeAssociationID );
        S_storeAssociationID = -1;
        return ( false );
    }
	return true;
}

#endif