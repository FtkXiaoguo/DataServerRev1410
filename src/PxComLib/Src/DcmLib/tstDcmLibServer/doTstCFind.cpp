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
  
int processCFind(int associationID,int messageID)
{
	
	MC_STATUS mcStatus;

	char  *ServiceName;
	MC_COMMAND    Command;
	/*
	*
	*/
	char		queryLevelStr[64];

	//	Get m_serviceName so we can send it out in response messages
	mcStatus = MC_Get_Message_Service(messageID, &ServiceName, &Command);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf("ERROR: (%d) - CFind::Process() - Merge error (%d,%s) - Couldn't get Message Service\n", associationID, mcStatus, MC_Error_Message(mcStatus));
		return(EXIT_FAILURE);
	}

	//	PATIENT || STUDY || SERIES || IMAGE
	mcStatus = MC_Get_Value_To_String(messageID, MC_ATT_QUERY_RETRIEVE_LEVEL, 64, queryLevelStr);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
 		printf("%d, %d, Couldn't get Query Level \n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

	printf("--- queryLevel [ %s ]   \n",queryLevelStr);

	/*
     *  Acquire a response message object, send it, and free it
     */
	int         rspMsgID;

	//	Message to be used in responses
	mcStatus = MC_Open_Empty_Message(&rspMsgID);

  //  mcStatus = MC_Open_Message (&rspMsgID, ServiceName, C_STORE_RSP);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Open_Message failed", mcStatus);
        return(EXIT_FAILURE);
    }

	mcStatus = MC_Set_Value_From_String(rspMsgID, MC_ATT_QUERY_RETRIEVE_LEVEL, queryLevelStr);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
 		printf(" %d, %d Couldn't set Query Level \n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

	mcStatus = MC_Set_Value_From_String(rspMsgID, MC_ATT_RETRIEVE_AE_TITLE, "myAE");
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf(" %d, %d Couldn't set Retrieve AE Title\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

	int numberOfMatches = 2;
	char _str_buff[128];
	for(int i=0;i<numberOfMatches;i++){
		sprintf(_str_buff,"%d",i);
		std::string number_str;
		number_str = _str_buff;
		if (!strcmp(queryLevelStr, "PATIENT"))
		{

			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENTS_NAME, (number_str+"patientsName").c_str());
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENT_ID, (number_str+"0023444").c_str());
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENTS_BIRTH_DATE,"2001/11/12");
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENTS_SEX, "F");
			 
		} else if (!strcmp(queryLevelStr, "STUDY"))
		{
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENTS_NAME, (number_str+"patientsName").c_str());
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENT_ID, (number_str+"0023444").c_str());
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENTS_BIRTH_DATE, "2001/11/12");
			MC_Set_Value_From_String (rspMsgID, MC_ATT_PATIENTS_SEX, "F");
			MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_DATE, "2011/11/12");
			MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_TIME, "12:00:33");
			MC_Set_Value_From_String (rspMsgID, MC_ATT_ACCESSION_NUMBER, number_str.c_str());

			MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_ID,	"555");
			MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_INSTANCE_UID, (number_str+"4445.555").c_str());
#if 0
			MC_Set_Value_From_String (rspMsgID, MC_ATT_MODALITIES_IN_STUDY, m_studyData[i].m_modalitiesInStudy);
			MC_Set_Value_From_String (rspMsgID, MC_ATT_REFERRING_PHYSICIANS_NAME, m_studyData[i].m_referringPhysiciansName);
			MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_DESCRIPTION, m_studyData[i].m_studyDescription);
			MC_Set_Value_From_LongInt(rspMsgID, MC_ATT_NUMBER_OF_STUDY_RELATED_SERIES, m_studyData[i].m_numberOfStudyRelatedSeries);
			MC_Set_Value_From_LongInt(rspMsgID, MC_ATT_NUMBER_OF_STUDY_RELATED_INSTANCES, m_studyData[i].m_numberOfStudyRelatedInstances);
			MC_Set_Value_From_String (rspMsgID, MC_ATT_SPECIFIC_CHARACTER_SET, m_studyData[i].m_characterSet);
#endif

		} else if (!strcmp(queryLevelStr, "SERIES")) 
		{
			 MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_INSTANCE_UID, number_str.c_str());
			 MC_Set_Value_From_String (rspMsgID, MC_ATT_SERIES_INSTANCE_UID, (number_str+"4445.555").c_str());
			 MC_Set_Value_From_LongInt (rspMsgID, MC_ATT_SERIES_NUMBER, 122);
#if 0
			 MC_Set_Value_From_String (rspMsgID, MC_ATT_SERIES_DESCRIPTION, m_seriesData[i].m_seriesDescription);
			 MC_Set_Value_From_String (rspMsgID, MC_ATT_MODALITY, m_seriesData[i].m_modality);
			 MC_Set_Value_From_LongInt(rspMsgID, MC_ATT_NUMBER_OF_SERIES_RELATED_INSTANCES, m_seriesData[i].m_numberOfSeriesRelatedInstances);
			 MC_Set_Value_From_String( rspMsgID, MC_ATT_BODY_PART_EXAMINED,m_seriesData[i].m_bodyPartExamined);
			 MC_Set_Value_From_String(rspMsgID, MC_ATT_SERIES_DATE, m_seriesData[i].m_seriesDate);
			 MC_Set_Value_From_String(rspMsgID, MC_ATT_SERIES_TIME, m_seriesData[i].m_seriesTime);
#endif
		} else if (!strcmp(queryLevelStr, "IMAGE"))
		{

			 MC_Set_Value_From_String (rspMsgID, MC_ATT_STUDY_INSTANCE_UID, (number_str+".22.4445.555.44").c_str());
			 MC_Set_Value_From_String (rspMsgID, MC_ATT_SERIES_INSTANCE_UID, (number_str+".22.4445.555.44.5").c_str());
			 MC_Set_Value_From_String (rspMsgID, MC_ATT_SOP_INSTANCE_UID, (number_str+".22.4445.555.44.5.4").c_str());
			 MC_Set_Value_From_LongInt (rspMsgID, MC_ATT_INSTANCE_NUMBER, i);
				
		}
	 

		//	Send out the fully populated response
		mcStatus = MC_Set_Service_Command(rspMsgID, ServiceName, C_FIND_RSP);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf(" %d, %d on MC_Set_Service_Command()\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
			return(EXIT_FAILURE);
		}

 		
		mcStatus = MC_Send_Response_Message(associationID, C_FIND_PENDING, rspMsgID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf("MC_Send_Response_Message failed", mcStatus);
		 //   MC_Abort_Association(&associationID);
			MC_Free_Message(&rspMsgID);
		   return(EXIT_FAILURE);
		}

		//	Reset the Merge message object for re-use
		mcStatus = MC_Empty_Message(rspMsgID);
		if (mcStatus != MC_NORMAL_COMPLETION)
		{
			printf(" %d, %d C_FIND_FAILURE_UNABLE_TO_PROCESS\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
			return(EXIT_FAILURE);
		}

	}

	//
	mcStatus = MC_Set_Service_Command(rspMsgID, ServiceName, C_FIND_RSP);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf(" %d, %d on MC_Set_Service_Command()\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

	mcStatus = MC_Send_Response_Message(associationID, C_FIND_SUCCESS, rspMsgID);
	if (mcStatus != MC_NORMAL_COMPLETION)
	{
		printf(" %d, %d on MC_Send_Response_Message()\n",mcStatus, C_FIND_FAILURE_UNABLE_TO_PROCESS);
		return(EXIT_FAILURE);
	}

  
    mcStatus = MC_Free_Message(&rspMsgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("MC_Free_Message for response message failed", mcStatus);
//        MC_Abort_Association(&associationID);
        return(EXIT_FAILURE);
    }

}
