// AssociationHandler.cpp: CAssociationHandler クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AssociationHandler.h"

#ifdef USE_NEW_LIB
#include "PxDicomImage.h"
#include "IDcmLibApi.h "
using namespace XTDcmLib;
#else
#include "VLIDicomImage.h"
#include "rtvMergeToolKit.h "
#endif

#include "rtvloadoption.h"

 

#include "TstVLIDicomImage.h"

 
 
//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

bool CAssociationHandler::SetValue ( int A_messageid, unsigned long A_tag,
                            const char *A_value, const char *A_default, 
                            bool A_required )
{
    MC_STATUS      status;
    static char    S_prefix[] = "SetValue";

    if ( strlen(A_value) <= (size_t)0 )
    {
        /*
         * The tag we were gonna set, was not given a value
         */
        if ( A_required == false )
        {
            /*
             * It's not a required tag, so we can set it to NULL
             */
            status = MC_Set_Value_To_NULL ( A_messageid, A_tag );
            if ( status != MC_NORMAL_COMPLETION )
            {
              
                return false;
            }
            return true;
        }
        else if ( A_required == true )
        {
            /*
             * The tag is required so we must check to see if there is a
             *  default since we were not given a value for it.
             */
            if ( strncmp( A_default, NULL, sizeof(NULL) ) )
            {
                status = MC_Set_Value_From_String ( A_messageid, A_tag,
                    A_default );
                if ( status != MC_NORMAL_COMPLETION )
                {
                   
                    return ( false );
                }
                return ( true );
            } 
            else
            {
                /*
                 * This is a required tag and no value was given for it.  Ther
                 *  is no default value, so we cannot set it.  Its an error.
                 */
                printf("%s:%lX, Required Parameter not set.\n", S_prefix,A_tag);
                return ( false );
            }
        }
    }

    /*
     * This is just a usual tag that is being set, since we have a value for it
     */
    status = MC_Set_Value_From_String ( A_messageid, A_tag, A_value ); 
    if ( status != MC_NORMAL_COMPLETION )
    {
      
        printf("***          Tag: %lX\n", A_tag);
        return ( false );
    }
    return ( true );

} /* SetValue() */

 
bool CAssociationHandler::GetValue ( int A_messageid, unsigned long A_tag, 
                            char *A_value, int A_size, char *A_default )
{
    MC_STATUS      status;
   
    status = MC_Get_Value_To_String ( A_messageid, A_tag, A_size,
                                      A_value );
    if ( status == MC_NULL_VALUE || status == MC_EMPTY_VALUE  ||
         status == MC_INVALID_TAG )
    {
        if (!A_default)
        {
            A_value[0] = '\0';
            return ( false );
        }
        strcpy ( A_value, A_default );
    }
    else if ( status != MC_NORMAL_COMPLETION )
    {
        printf("MC_Get_Value_To_String %d ", status );
        printf("***          Tag:  %lX\n", A_tag);
        return ( false );
    }
    return ( true );
} /* GetValue() */



CAssociationHandler::CAssociationHandler(int id,TRLogger *log)
{
	 m_Logger = log; 

	m_AssociationID = id;
	m_ImageCount = 0;
	m_checkPixelData = 0;
	m_endFlag = false;

	m_writeFile = false;
	m_prodImgFlag = true;
	//
	 
}

CAssociationHandler::~CAssociationHandler()
{
	clearSeriesManList();

	printf(" delete CAssociationHandler \n");
}
int	CAssociationHandler::PreProcess(void) 
{
	m_processStatus = kToBeStarted; 
	return 0;
}
	

int	CAssociationHandler::Process(void)
{
	printf("rec:\n");
	m_AsoStartTime = ::GetTickCount();

	MC_COMMAND            command;
    MC_STATUS             status;
  
	m_endFlag = false;
int map_len = m_SeriesManList.size();

 bool free_Msg_flag ;
  while(!TerminationRequested())
  {
	 m_MessageID = 0;
	 free_Msg_flag = true;
	 status = MC_Read_Message ( m_AssociationID, 1/*Sec*/,
                                   &m_MessageID, 
                                   &m_serviceName, &command );

//	 ::Sleep(200);
		 if (status == MC_TIMEOUT){
			  continue;
		 }else 
		if (status == MC_ASSOCIATION_CLOSED
			  || status == MC_ASSOCIATION_ABORTED
			  || status == MC_NETWORK_SHUT_DOWN)
		{
			printf("\n Storage association closed\n");
        
			closeAssociation();
			break;
		}
		else if (status != MC_NORMAL_COMPLETION)
		{
       
			break;
		}
		else
		{
			switch(command){
			case C_STORE_RQ:
				{
					free_Msg_flag = false;
					if(!proc_CStore()){
						printf(" proc_CStore error \n");
						continue;
					};
					if(!response_CStore()){
						printf(" response_CStore error \n");
						continue;
					};
				}
				break;
			case C_FIND_RQ:
				{
					printf(" C_FIND_RQ ");
				}
				break;
			default:
				break;
			}
		 
			
		}

	}

	if (free_Msg_flag)
	{
		//proc_CStore 処理されていないため
		if(m_MessageID){
			status = MC_Free_Message ( &m_MessageID );
			if ( status != MC_NORMAL_COMPLETION )
			{
			//	 printf("MC_Free_Message error \n");
			}

			m_MessageID = 0;
		}
	}

//
	{
		printf(" == m_ImageCount %d\n",m_ImageCount);

		if(!proc_end()){
			printf(" proc_end error \n");
		};
	}
  
	closeAssociation();

	return 0;
}

 
bool CAssociationHandler::proc_CStore()
{
	MC_STATUS             status;

	char str_buffer[256];
    
  printf(".");

	
	if ( GetValue ( m_MessageID, MC_ATT_PATIENTS_NAME, 
        str_buffer,
		sizeof ( str_buffer ),
		"") == false ){
			printf(" GetValue %x error \n",MC_ATT_PATIENTS_NAME);
		 
	}
 	//	printf("Patient Name %s ",str_buffer);
	//
	if ( GetValue ( m_MessageID, MC_ATT_PATIENT_ID, 
        str_buffer,
		sizeof ( str_buffer ),
		"") == false ){
			printf(" GetValue %x error \n",MC_ATT_PATIENT_ID);
		 
	};
//		printf("ID [ %s ]  ",str_buffer);

	//
	if ( GetValue ( m_MessageID, MC_ATT_STUDY_ID,
		str_buffer,
		sizeof ( str_buffer ),
		"") == false ){
			printf(" GetValue %x error \n",MC_ATT_STUDY_ID);
		 
	};
//	 printf("Study ID %s",str_buffer);
	 //
	 if ( GetValue ( m_MessageID, MC_ATT_SERIES_INSTANCE_UID,
		str_buffer,
		sizeof ( str_buffer ),
		"") == false ){
			printf(" GetValue %x error \n",MC_ATT_SERIES_INSTANCE_UID);
		 
	};
//	 printf("SeriesInstanceUID %s \n",str_buffer);

	 if(m_prodImgFlag){
			
		 if(!procImage()) {
			 return false;
		 }
	 }else{
	 
	 //if you donot CTstVLIDicomImage, please free MesageID here.
		 status = MC_Free_Message ( &m_MessageID );
		if ( status != MC_NORMAL_COMPLETION )
		{
			 ;
			 return false;
		}
 
	 }
 
	 m_ImageCount++;



	return true;
}
bool CAssociationHandler::response_CStore()
{
	MC_STATUS             status;
	int responseMessageID;
	/* 
     * Send a successful response 
     */
    status = MC_Open_Message ( &responseMessageID,
                               m_serviceName,
                               C_STORE_RSP );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("MC_Open_Message error %d \n",status);
        return false;
    }

    status = MC_Send_Response_Message ( m_AssociationID,
                                        C_STORE_SUCCESS,
                                        responseMessageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        MC_Free_Message ( &responseMessageID );
        printf("MC_Send_Response_Message error %d \n",status);
          
        return false;
    }

    status = MC_Free_Message ( &responseMessageID );
    if ( status != MC_NORMAL_COMPLETION )
    {
         ;
         return false;
    }

	return true;
}
bool CAssociationHandler::proc_end()
{
	unsigned long cur_time = ::GetTickCount();

	printf(" ==== <<< CAssociationHandler::proc_end \n");
	printf(" spent time <<<< %.2f Sec >>>> \n",(cur_time-m_AsoStartTime)/1000.0);



	procVolume();

	clearSeriesManList();

	m_endFlag = true;
 

	return true;
}
bool CAssociationHandler::procImage()
{
	 MC_STATUS status ;
	 
	 unsigned int bits_allocated;
	 status = MC_Get_Value_To_UInt( m_MessageID, MC_ATT_BITS_ALLOCATED,
                                      &bits_allocated );
 
	unsigned int bits_stored;
	 status = MC_Get_Value_To_UInt( m_MessageID, MC_ATT_BITS_STORED,
                                      &bits_stored );

	 unsigned int high_bit;
	 status = MC_Get_Value_To_UInt( m_MessageID, MC_ATT_HIGH_BIT,
                                      &high_bit );

 
	 unsigned int image_sizeX;
	 status = MC_Get_Value_To_UInt( m_MessageID, MC_ATT_COLUMNS,
                                      &image_sizeX );

	 unsigned int image_sizeY;
	 status = MC_Get_Value_To_UInt( m_MessageID, MC_ATT_ROWS,
                                      &image_sizeY );
	 

	 
	 CTstVLIDicomImage *DicomImage = new CTstVLIDicomImage;
	 DicomImage->createFromMessasgeID(m_MessageID);

	 if(m_writeFile){
		 printf("[%d]>>>==: Reived DICOM Message : %d\n", m_AssociationID,m_ImageCount);

		char _file_name[256];
		if(m_homeFolder.size()>0){
			sprintf(_file_name,"%stemp\\recevd_%d.dcm",m_homeFolder.c_str(),m_AssociationID);
		}else{
			sprintf(_file_name,"recevd_%d.dcm",m_AssociationID);
		}
		DicomImage->getDicomImage()->ConvertToFile(_file_name, "MONE_AE");

	 
	}

//	 DicomImage.saveDicom("test.dcm");

// 	 DicomImage->checkImageData(0);

	 unsigned char * pixel_data_ptr = DicomImage->GetImagePixels();
	 if(pixel_data_ptr){
#if 0
		 FILE *fp = fopen("test.bin","wb");
		 fwrite(pixel_data_ptr, 2 , image_sizeX*image_sizeY,fp);
		 fclose(fp);
#endif
	 }
#ifdef USE_NEW_LIB
	 CPxDicomImage* vli_dicomImage  = DicomImage->getDicomImage();
#else
	 VLIDicomImage* vli_dicomImage  = DicomImage->getDicomImage();
#endif


	 std::string SeriesInstanceUID = vli_dicomImage->GetValue(kVLISeriesInstanceUid);

  
	 SeriesManIterator iter = m_SeriesManList.find(SeriesInstanceUID);
	 if(iter == m_SeriesManList.end()){
		 //新series
 		m_SeriesManList[SeriesInstanceUID].push_back(DicomImage);
	 }else{
		 iter->second.push_back(DicomImage);
	 }
 
	 
#if 0
	 //donot free message here , becase used CTstVLIDicomImage
	 status = MC_Free_Message ( &m_MessageID );
	if ( status != MC_NORMAL_COMPLETION )
    {
         ;
         return false;
    }
#endif

 

	 return true;
}

bool CAssociationHandler::procVolume()
{
	SeriesManIterator iter;

	for(iter=m_SeriesManList.begin();iter!=m_SeriesManList.end(); iter++){
		int z_i;
		 
		int sizeZ = iter->second.size();

#if 1
		//check image data

		int error_count = 0;

		int frameNum = -1;
		int previous_frameNum=-1;;

		int ref_dataType = -1;
		int ref_series_uid=-1;
		for( z_i=0;z_i<sizeZ;z_i++){
			CTstVLIDicomImage  *imager_ptr = iter->second.at(z_i);
			if(m_checkPixelData !=0){
				if(!imager_ptr->checkImageData(z_i,ref_dataType,ref_series_uid,frameNum))
				{
					error_count++;

					m_Logger->LogMessage(" CAssociationHandler::checkImageData Pixeldata ERROR\n");
					m_Logger->FlushLog();
					break;
				};
				if(previous_frameNum>0){
					if(frameNum != previous_frameNum){
						error_count++;

						m_Logger->LogMessage(" CAssociationHandler::checkImageData frameNum ERROR\n");
						m_Logger->FlushLog();
						break;
					}
				}
				previous_frameNum = frameNum;
			}
			 
		}
		if(m_checkPixelData !=0){
			if(previous_frameNum>0){
				if(previous_frameNum != sizeZ){
					error_count++;

					m_Logger->LogMessage(" CAssociationHandler::checkImageData previous_frameNum(%d) != sizeZ(%d) ERROR\n",previous_frameNum,sizeZ);
					m_Logger->FlushLog();
				}
			}
			if(error_count<1){
				m_Logger->LogMessage(" CAssociationHandler::checkImageData OK \n");
				m_Logger->FlushLog();
			}
		}
#endif
 
	 
	}

	return true;
}
void CAssociationHandler::clearSeriesManList()
{
	SeriesManIterator iter;

	for(iter=m_SeriesManList.begin();iter!=m_SeriesManList.end(); iter++){
		int sizeZ = iter->second.size();
		//remove all
		for(int z_i=0;z_i<sizeZ;z_i++){
 			CTstVLIDicomImage  *imager_ptr = iter->second.at(z_i);
	 		delete imager_ptr;
			 
		}
 		iter->second.clear();
	}
	m_SeriesManList.clear();
	int map_len = m_SeriesManList.size();
}

bool CAssociationHandler::closeAssociation()
{
	if(m_AssociationID>0){
		MC_Close_Association(&m_AssociationID);
		m_AssociationID = 0;
	}
	return true;
}

 