#include "StdAfx.h"
#include "TstWrite.h"


//////////////////////////////
	

//------------------------------------------------------------------------------------------
//
MC_STATUS SetBlankPixelData(int A_msgID, unsigned long A_tag, int A_isFirst, void* A_info, 
							int* A_dataSize, void** A_dataBufferPtr, int* A_isLastPtr);

MC_STATUS SetDataBuffer(int messageID, unsigned long tag, int isFirst, void* userInfo, 
							   int* dataSizePtr, void** dataBufferPtr, int* isLastPtr);


////
CTstWrite::CTstWrite(void)
{
	m_messageID = 0;
}

CTstWrite::~CTstWrite(void)
{
	if(m_messageID){
		MC_Free_Message(&m_messageID);
	}
	release();
}
void CTstWrite::doTestVector()
{
	int status;

	{
		int Acount = 0;
		float data_pos[] = {
			1.0,
			1.1,
			1.2,
		};
		status = MC_Set_Value_From_Float(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,data_pos[Acount++]);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
		//
		status = MC_Set_Next_Value_From_Float(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,data_pos[Acount++]);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
 		status = MC_Set_Next_Value_From_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,data_pos[Acount++]);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
		///////
		int           Acount_ret = 0;
		status = MC_Get_Value_Count(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,&Acount_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_Count error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(Acount != Acount_ret){
			printf("MC_Set_Next_Value_From_Float MC_Get_Value_Count diff error\n");
		}
		//
		double pos_ret =0;
		// ˆê‰ñ–Ú
		status = MC_Get_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(DiffFloat(pos_ret,data_pos[0])){
			printf("MC_Set_Next_Value_From_Float MC_Get_Next_Value_To_Double diff error\n");
		}

		//
		//“ñ‰ñ‚©‚çNext
		status = MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(DiffFloat(pos_ret,data_pos[1])){
			printf("MC_Set_Next_Value_From_Float MC_Get_Next_Value_To_Double diff error\n");
		}
		//
		status = MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(DiffFloat(pos_ret,data_pos[2])){
			printf("MC_Set_Next_Value_From_Float MC_Get_Next_Value_To_Double diff error\n");
		}
		//
		status = MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_ORIENTATION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}

		  
	}
	//
	{
		int Acount = 0;
		float data_pos[] = {
			-34.0,
			-34.1,
			-34.2,
		};
		status = MC_Set_Value_From_Float(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,data_pos[Acount++]);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
		//
		status = MC_Set_Next_Value_From_Float(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,data_pos[Acount++]);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
 		status = MC_Set_Next_Value_From_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,data_pos[Acount++]);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
		///////
		int           Acount_ret = 0;
		status = MC_Get_Value_Count(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,&Acount_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_Count error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(Acount != Acount_ret){
			printf("MC_Set_Next_Value_From_Float MC_Get_Value_Count diff error\n");
		}
		//
		double pos_ret =0;
		// ˆê‰ñ–Ú
		status = MC_Get_Value_To_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(DiffFloat(pos_ret,data_pos[0])){
			printf("MC_Set_Next_Value_From_Float MC_Get_Next_Value_To_Double diff error\n");
		}

		//
		//“ñ‰ñ‚©‚çNext
		status = MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(DiffFloat(pos_ret,data_pos[1])){
			printf("MC_Set_Next_Value_From_Float MC_Get_Next_Value_To_Double diff error\n");
		}
		//
		status = MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(DiffFloat(pos_ret,data_pos[2])){
			printf("MC_Set_Next_Value_From_Float MC_Get_Next_Value_To_Double diff error\n");
		}
		//
		status = MC_Get_Next_Value_To_Double(m_messageID, MC_ATT_IMAGE_POSITION_PATIENT,&pos_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}

		  
	}
}
void CTstWrite::doTest()
{
#define STR_SIZE 128
	char  str_buff[STR_SIZE];

	std::string test_str;
	int status;

	 MC_VR	Avr;
	 int	Avalues;
//	init();

	MC_Open_Empty_Message(&m_messageID);


	status = MC_Set_Message_Transfer_Syntax(m_messageID, IMPLICIT_LITTLE_ENDIAN);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Message_Transfer_Syntax error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	//

	status = MC_Set_Value_To_Empty(m_messageID, MC_ATT_SCAN_OPTIONS);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_To_Empty error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
 
	status = MC_Set_Value_From_String(m_messageID, MC_ATT_SCAN_OPTIONS,"will be deleted");
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_From_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}


	//delete this TAG
 	status = MC_Set_Value_To_Empty(m_messageID, MC_ATT_SCAN_OPTIONS);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_To_Empty error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
 

	
	test_str = "testTitle";
	status = MC_Set_Value_From_String(m_messageID, MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,test_str.c_str());
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_From_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	//

	//
	status = MC_Get_Value_To_String(m_messageID, MC_ATT_SOURCE_APPLICATION_ENTITY_TITLE,STR_SIZE,str_buff);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Get_Value_To_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	if(test_str != std::string(str_buff)){
		printf("MC_Set_Value_From_String MC_Get_Value_To_String diff error\n");
	}
	//
	 
	

	//
	test_str = "testComment";
	status = MC_Set_Value_From_String(m_messageID, MC_ATT_PATIENT_COMMENTS,test_str.c_str());
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_From_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	status = MC_Get_Attribute_Info (m_messageID, MC_ATT_PATIENT_COMMENTS,&Avr,&Avalues);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Get_Attribute_Info error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}


	status = MC_Get_Value_To_String(m_messageID, MC_ATT_PATIENT_COMMENTS,STR_SIZE,str_buff);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Get_Value_To_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	if(test_str != std::string(str_buff)){
		printf("MC_Set_Value_From_String MC_Get_Value_To_String diff error\n");
	}


	status = MC_Set_Value_From_Int(m_messageID, MC_ATT_KVP,-165);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_From_Int error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}


	{
		unsigned int test_data = 65;
		status = MC_Set_Value_From_UInt(m_messageID, MC_ATT_RECONSTRUCTION_DIAMETER,test_data);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_UInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		unsigned int test_data_ret;
		status = MC_Get_Value_To_UInt(m_messageID, MC_ATT_RECONSTRUCTION_DIAMETER,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_UInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_UInt MC_Get_Value_To_UInt diff error\n");
		}
	}

	////////
	{
		short test_data = 16;
		status = MC_Set_Value_From_ShortInt(m_messageID, MC_ATT_BITS_ALLOCATED,test_data);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_ShortInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		short test_data_ret;
		status = MC_Get_Value_To_ShortInt(m_messageID, MC_ATT_BITS_ALLOCATED,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_ShortInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_ShortInt MC_Get_Value_To_ShortInt diff error\n");
		}
	}
	/////////

	{
		unsigned short test_data = 12;
		status = MC_Set_Value_From_UShortInt(m_messageID, MC_ATT_BITS_STORED,test_data);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_UShortInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		unsigned short test_data_ret;
		status = MC_Get_Value_To_UShortInt(m_messageID, MC_ATT_BITS_STORED,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_UShortInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_UShortInt MC_Get_Value_To_UShortInt diff error\n");
		}
	}
	/////////
  
	//
	{
		long test_data  = 12340;
		status = MC_Set_Value_From_LongInt(m_messageID, MC_ATT_EXPOSURE_TIME,test_data);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_LongInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		long test_data_ret;
		status = MC_Get_Value_To_LongInt(m_messageID, MC_ATT_EXPOSURE_TIME,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_LongInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_LongInt MC_Get_Value_To_LongInt diff error\n");
		}

	}
	{
		unsigned long test_data  = 3560;
		status = MC_Set_Value_From_ULongInt(m_messageID, MC_ATT_WINDOW_CENTER,3560);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_ULongInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		unsigned long test_data_ret;
		status = MC_Get_Value_To_ULongInt(m_messageID, MC_ATT_WINDOW_CENTER,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_ULongInt error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_ULongInt MC_Get_Value_To_ULongInt diff error\n");
		}
	
	}


	//
	{
		double test_data  = 0.11;
		status = MC_Set_Value_From_Double(m_messageID, MC_ATT_SLICE_THICKNESS,test_data);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_Double error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		double test_data_ret;
		status = MC_Get_Value_To_Double(m_messageID, MC_ATT_SLICE_THICKNESS,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_Double error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_Double MC_Get_Value_To_Double diff error\n");
		}
	}

	{
		float test_data  = 1.1;
 		status = MC_Set_Value_From_Float(m_messageID, MC_ATT_RESCALE_SLOPE,1.1);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Value_From_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		float test_data_ret;
		status = MC_Get_Value_To_Float(m_messageID, MC_ATT_RESCALE_SLOPE,&test_data_ret);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Get_Value_To_Float error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		if(test_data != test_data_ret){
			printf("MC_Set_Value_From_Float MC_Get_Value_To_Float diff error\n");
		}
	}
  
	

	//
	status = MC_Set_Value_From_String(m_messageID, MC_ATT_OPERATORS_NAME,"opt name");
	//
	status = MC_Set_Value_To_NULL(m_messageID, MC_ATT_OPERATORS_NAME );
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_To_NULL error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	status = MC_Get_Attribute_Info (m_messageID, MC_ATT_OPERATORS_NAME,&Avr,&Avalues);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Get_Attribute_Info error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}

	
	////////////////
	status =  MC_Set_Value_From_Function(m_messageID, MC_ATT_PIXEL_DATA, 0, (SetBlankPixelData));
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_From_Function error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}

	//////////////////
	if(0){
		int evenBufferSize = 16;
		unsigned char *evenBuffer = new unsigned char[evenBufferSize];

		for(int i=0;i<evenBufferSize;i++){
			evenBuffer[i] = i;
		}
		CBDataStruct cbdata;
		cbdata.m_buf = evenBuffer;
		cbdata.m_bufsize = evenBufferSize;

		status =  MC_Set_Value_From_Function(m_messageID, MC_ATT_PIXEL_DATA, &cbdata, SetDataBuffer);

		if (status != MC_NORMAL_COMPLETION)
		{
			delete [] evenBuffer;

			printf("MC_Set_Value_From_Function error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
	 
		delete [] evenBuffer;
	 
	}

	

	//////////////////
	
	status = MC_Set_Value_From_String(m_messageID, MC_ATT_IMAGE_TYPE,"DERIVED");
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Value_From_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	//
	status = MC_Set_Next_Value_From_String(m_messageID, MC_ATT_IMAGE_TYPE,"SECONDARY");
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Next_Value_From_String error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}

	status = MC_Get_Attribute_Info (m_messageID, MC_ATT_IMAGE_TYPE,&Avr,&Avalues);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Get_Attribute_Info error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}

	////////////
	doTestVector();
	
	///
	doTestConfig();


	{
		status = MC_Add_Private_Block(m_messageID,"testPrivateTag",0x0077);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Add_Private_Block error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
#if 1
		status = MC_Add_Private_Attribute(m_messageID,"testPrivateTag",0x0077,0x20,DS);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Add_Private_Attribute error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}

		 //
		 status = MC_Set_pValue_From_String(m_messageID,"testPrivateTag", 0x0077,0x20,"uuuuuuuuuuu");
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_String error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		/////
		status = MC_Add_Private_Attribute(m_messageID,"testPrivateTag",0x0077,0x40,US);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Add_Private_Attribute error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}

		status = MC_Set_pValue_From_Int(m_messageID,"testPrivateTag", 0x0077,0x40,5);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_pValue_From_Int error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}

		/////
		status = MC_Add_Private_Attribute(m_messageID,"testPrivateTag",0x0077,0x30,DS);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Add_Private_Attribute error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}

		status = MC_Set_Next_pValue_From_Int(m_messageID,"testPrivateTag", 0x0077,0x30,1);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_String error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		status = MC_Set_Next_pValue_From_Int(m_messageID,"testPrivateTag", 0x0077,0x30,2);
		if (status != MC_NORMAL_COMPLETION)
		{
			printf("MC_Set_Next_Value_From_String error\n");
			MC_Free_Message(&m_messageID);
			return  ;  
		}
		 
#endif
	}

 
 	writeFile();


 
	if(m_messageID){
		MC_Free_Message(&m_messageID);
		m_messageID = 0;
	}
	_CrtDumpMemoryLeaks();
}

void CTstWrite::writeFile()
{
	DcmXTDicomMessage		*messageInstance = IDcmLibApi::get_DcmMessage(m_messageID);
	messageInstance->writeFile("test_dcm.dcm");
}


void CTstWrite::doTestConfig()
{
	int status;

	 
	status = MC_Set_String_Config_Value(IMPLEMENTATION_CLASS_UID,"tttttttt");
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_String_Config_Value error\n");
		return  ;  
	}
		 
	char _str_buff[1024];
	status = MC_Get_String_Config_Value(IMPLEMENTATION_CLASS_UID,1024,_str_buff);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_String_Config_Value error\n");
		return  ;  
	}
	//
	status = MC_Get_String_Config_Value(IMPLEMENTATION_VERSION,1024,_str_buff);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_String_Config_Value error\n");
		return  ;  
	}
}

static int
SetNullOrStringValue( 
                int               A_messageID,
                unsigned long     A_tag,
                char*             A_value,
                char*             A_function
            )
{
    MC_STATUS status;
    char      tagDescription[60];

    if ( strlen( A_value ) > 0 )
    {
        status = MC_Set_Value_From_String( A_messageID,
                                           A_tag,
                                           A_value );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
            printf(A_function, __LINE__, status,
                      "Failed to set the tag %s to \"%s\"\n",
                      tagDescription, A_value );
            return  0;
        }
    }
    else
    {
        status = MC_Set_Value_To_NULL( A_messageID,
                                       A_tag );
        if ( status != MC_NORMAL_COMPLETION )
        {
            MC_Get_Tag_Info(A_tag, tagDescription, sizeof(tagDescription));
            printf(A_function, __LINE__, status,
                      "Failed to NULL \"%s\"\n", tagDescription );
            return  0;
        }
    }
    
    return 1 ;
}
void CTstWrite::dotestItem()
{
	int status;

	MC_Open_Empty_Message(&m_messageID);


	status = MC_Set_Message_Transfer_Syntax(m_messageID, IMPLICIT_LITTLE_ENDIAN);
	if (status != MC_NORMAL_COMPLETION)
	{
		printf("MC_Set_Message_Transfer_Syntax error\n");
		MC_Free_Message(&m_messageID);
		return  ;  
	}
	//


	int          procedureStepItemID;
	 status = MC_Open_Item( &procedureStepItemID,
                           "SCHEDULED_PROCEDURE_STEP" );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("SetAndSendWorklistMsg", __LINE__, status,
                  "Unable to open an item for "
                  "SCHEDULED_PROCEDURE_STEP\n" );
        MC_Free_Message( &m_messageID );
        ;
    }

	///
	/*
    ** Now we have a message and we have an item.  The item is the sequence
    ** of procedures.  We need to connect this item to the message.  The
    ** following places the id of the item into the message in place of the
    ** procedure step sequence.
    */
    status = MC_Set_Value_From_Int( m_messageID,
                                   MC_ATT_SCHEDULED_PROCEDURE_STEP_SEQUENCE,
                                   procedureStepItemID );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("SetAndSendWorklistMsg", __LINE__, status,
                  "Unable to set value for MSG_ID:  %d\n"
                  "                           TAG:  %lx\n"
                  "                     TAG VALUE:  %s\n",
                  m_messageID,
                  MC_ATT_SCHEDULED_PROCEDURE_STEP_SEQUENCE,
                  procedureStepItemID );
        MC_Free_Item( &procedureStepItemID );
        MC_Free_Message( &m_messageID );
        return  ;
    }

	////
	  /*
    ** In order to setup the message, we use the MC_Set_Value... functions.
    ** We also include some fields that are part of the IHE requirements.
    ** We set those values to NULL, indicitating that the SCP should give
    ** them to us if it can.
    */

    if ( SetNullOrStringValue( procedureStepItemID, 
                               MC_ATT_MODALITY, 
                               "", 
                               "SetAndSendWorklistMsg" ) != 1) 
    {
        MC_Free_Message( &m_messageID );
        return  ;
    }

    /*
    ** If any of the required fields are still unfilled, then we will
    ** set them to NULL, lest we get invalid tag messages.
    */
    if ( SetNullOrStringValue( procedureStepItemID, 
                        MC_ATT_SCHEDULED_STATION_AE_TITLE,
                        "", 
                        "SetAndSendWorklistMsg" ) != 1 ) 
    {
        MC_Free_Message( &m_messageID );
        return  ;
    }

	///
	status = MC_Set_Value_To_NULL( procedureStepItemID,
                                 MC_ATT_SCHEDULED_PROCEDURE_STEP_START_TIME );
    if ( status != MC_NORMAL_COMPLETION )
    {
        printf("SetAndSendWorklistMsg", __LINE__, status,
                  "Unable to set NULL value for MSG_ID:  %d\n"
                  "                                TAG:  %lx\n"
                  "                          TAG VALUE:  %s\n",
                  procedureStepItemID,
                  MC_ATT_SCHEDULED_PROCEDURE_STEP_START_TIME,
                  "NULL" );
        MC_Free_Message( &m_messageID );
        return  ;
    }


	writeFile();
}