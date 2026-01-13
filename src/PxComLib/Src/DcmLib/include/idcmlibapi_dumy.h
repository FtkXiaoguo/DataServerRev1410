// 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_API_DUMY_H_)
#define AFX_IDICOM_LIB_API_DUMY_H_
 
////////////////////////
// the following , not implemate ye
////////////////////////
////////////////////////

#define API_DUMY_ERROR(api_name) { \
	DcmXTUtil *util = IDcmLibApi::getDcmXTUtil();\
	if(util){ util->DcmAPIError(api_name); } \
	}

inline MC_STATUS MC_Create_File
                                       (int*            AfileID, 
                                        const char*     AfileName, 
                                        const char*     AserviceName, 
                                        MC_COMMAND      Acommand){
	API_DUMY_ERROR("MC_Create_File");
	return MC_ERROR;
} 


inline MC_STATUS   MC_Set_pValue_From_Function(
                                                 int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 void*          AuserInfo,
                                                 MC_STATUS      (NOEXP_FUNC *AuserFunction)
                                                    (int            CBmsgID,
                                                     unsigned long  ATag,
                                                     int            CBfirstCall,
                                                     void*          CBuserInfo,
                                                     int*           CBdataLen,
                                                     void**         CBdataBuffer,
                                                     int*           CBisLast))
{
	API_DUMY_ERROR("MC_Set_pValue_From_Function");
	return MC_ERROR;
}


inline MC_STATUS   MC_Set_Next_Value_To_NULL(int            AmsgID,
                                                 unsigned long  Atag)
{
	API_DUMY_ERROR("MC_Set_Next_Value_To_NULL");
	return MC_ERROR;
}
inline MC_STATUS   MC_Get_Value_To_Buffer(int            AmsgID,
                                                 unsigned long  Atag,
                                                 int            AbufferSize,
                                                 void*          Abuffer,
                                                 int*           AvalueSize)
{
	API_DUMY_ERROR("MC_Get_Value_To_Buffer");
	return MC_ERROR;
}




inline MC_STATUS MC_Free_File(int*            AfileID)
{
	API_DUMY_ERROR("MC_Free_File");
	return MC_ERROR;
}

////////////


inline MC_STATUS  MC_Delete_Attribute   (int            AmsgID,
                                                 unsigned long  Atag)
{
	API_DUMY_ERROR("MC_Delete_Attribute");
	return MC_ERROR;
}




//
inline MC_STATUS  MC_Open_Item  (int*           AitemID,
                                                 const char*    AitemName)
{
	API_DUMY_ERROR("MC_Open_Item");
	return MC_ERROR;
}
inline MC_STATUS  MC_Free_Item  (int*           AitemID)
{
	API_DUMY_ERROR("MC_Free_Item");
	return MC_ERROR;
}

//
inline MC_STATUS  MC_Get_First_Attribute(int            AmsgID,
                                                 unsigned long* Atag,
                                                 MC_VR*         Avr,
                                                 int*           Avalues)
{
	API_DUMY_ERROR("MC_Get_First_Attribute");
	return MC_ERROR;
}

inline MC_STATUS  MC_Get_Next_Attribute (int            AmsgID,
                                                 unsigned long* Atag,
                                                 MC_VR*         Avr,
                                                 int*           Avalues)
{
	API_DUMY_ERROR("MC_Get_Next_Attribute");
	return MC_ERROR;
}



//
inline MC_STATUS  MC_Message_To_Stream( 
                                            int             AmsgID,
                                            unsigned long   AstartTag,
                                            unsigned long   AstopTag,
                                            TRANSFER_SYNTAX Asyntax,
                                            void*           AuserInfo, 
                                            MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int        CBmsgID,
                                                     void*      CBuserInfo,
                                                     int        CBdataSize,
                                                     void*      CBdataBuffer,
                                                     int        CBisFirst, 
                                                     int        CBisLast))
{
	API_DUMY_ERROR("MC_Message_To_Stream");
	return MC_ERROR;
} 
inline MC_STATUS  MC_Get_Stream_Length  (int            AmsgID,
                                                 unsigned long  AstartTag,
                                                 unsigned long  AstopTag, 
                                                 unsigned long* AmsgLength,
                                                 TRANSFER_SYNTAX Asyntax)
{
	API_DUMY_ERROR("MC_Get_Stream_Length");
	return MC_ERROR;
}              


////
//////////
////
inline MC_STATUS  MC_Add_Standard_Attribute(int            AmsgID,
                                                 unsigned long  Atag)
{
	API_DUMY_ERROR("MC_Add_Standard_Attribute");
	return MC_ERROR;
}



inline MC_STATUS  MC_Get_pAttribute_Info(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 MC_VR*         Avr,
                                                 int*           Avalues)
{
	API_DUMY_ERROR("MC_Get_pAttribute_Info");
	return MC_ERROR;
}
////////
///
/////////



inline MC_STATUS    MC_NewSyntaxList           (char*          A_name, 
                                                     TRANSFER_SYNTAX A_syntax_ids[])
{
	API_DUMY_ERROR("MC_NewSyntaxList");
	return MC_ERROR;
}

inline MC_STATUS    MC_NewServiceFromUID       (char*          AserviceName, 
                                                     char*          ASOPClassUID, 
                                                     char*          ASyntaxListName, 
                                                     int            ASCURole, 
                                                     int            ASCPRole)
{
	API_DUMY_ERROR("MC_NewServiceFromUID");
	return MC_ERROR;
}

inline MC_STATUS  MC_FreeSyntaxList          (char*          ASyntaxList)
{
	API_DUMY_ERROR("MC_FreeSyntaxList");
	return MC_ERROR;
}
inline MC_STATUS   MC_FreeService             (char*          AServiceName)
{
	API_DUMY_ERROR("MC_FreeService");
	return MC_ERROR;
}
inline MC_STATUS   MC_NewProposedServiceList  (char*          ServiceListName, 
                                                     char*          ServiceNameArray[])
{
	API_DUMY_ERROR("MC_NewProposedServiceList");
	return MC_ERROR;
}


/* ======================================================================== *
 *                  Functions to get configuration values                   *
 * ======================================================================== */


inline MC_STATUS     MC_Get_Log_Destination(
                                                 LogParm        Aparm,
                                                 int*           Avalue)
{
	API_DUMY_ERROR("MC_Get_Log_Destination");
	return MC_ERROR;
}
//

inline MC_STATUS     MC_Get_Tag_Info (unsigned long  Atag,
                                                 char*          Aname,
                                                 int            Alength)
{
	API_DUMY_ERROR("MC_Get_Version_String");
	return MC_ERROR;
}
///////
//////////////////////////
////
///////////


inline MC_STATUS    MC_Stream_To_Message(
                                            int             AmsgID,
                                            unsigned long   AstartTag,
                                            unsigned long   AstopTag,
                                            TRANSFER_SYNTAX Asyntax,
                                            unsigned long*  AerrorTag,
                                            void*           AuserInfo,
                                            MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                                    (int        CBmsgID,
                                                     void*      CBuserInfo,
                                                     int        CBfirstCall,
                                                     int*       CBdataLen,
                                                     void**     CBdataBuffer,
                                                     int*       CBisLast))
{
	API_DUMY_ERROR("MC_Stream_To_Message");
	return MC_ERROR;
}


inline MC_STATUS  MC_Get_File_Preamble  
                                       (int             AfileID,
                                        char*           Apreamble)
{
	API_DUMY_ERROR("MC_Get_File_Preamble");
	return MC_ERROR;
}
inline MC_STATUS  MC_Set_File_Preamble  
                                       (int             AfileID,
                                        const char*     Apreamble)
{
	API_DUMY_ERROR("MC_Set_File_Preamble");
	return MC_ERROR;
}


inline MC_STATUS   MC_Open_File_Bypass_OBOW
                                       (int             AapplicationID,
                                        int             AfileID,  
                                        void*           AuserInfo, 
                                        MC_STATUS       (NOEXP_FUNC *AuserFunction)
                                               (char*       CBfilename,
                                                void*       CBuserInfo,
                                                int*        CBdataSize,
                                                void**      CBdataBuffer,
                                                int         CBisFirst,
                                                int*        CBisLast))
{
	API_DUMY_ERROR("MC_Open_File_Bypass_OBOW");
	return MC_ERROR;
} 
 

///////////////////
///
////////////////////

inline MC_STATUS 	MC_Cleanup_Memory (int Asec)
{
	API_DUMY_ERROR("MC_Cleanup_Memory");
	return MC_ERROR;
}  
////////
/* ======================================================================== *
 *             Functions to navigate through DICOMDIR structure             *
 * ======================================================================== */

inline MC_STATUS MC_Dir_Root_Entity
                                       (int             AdirID,
                                        int*            AentityID, 
                                        int*            AitemID, 
                                        char**          AitemType,
                                        int*            AisLast)
{
	API_DUMY_ERROR("MC_Dir_Root_Entity");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Next_Entity
                                       (int             AdirID,
                                        int             AitemID, 
                                        int*            AnextEntityID, 
                                        int*            AnextItemID, 
                                        char**          AnextItemType,
                                        int*            AisLast)
{
	API_DUMY_ERROR("MC_Dir_Next_Entity");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_First_Record
                                       (int             AdirID,
                                        int             AentityID, 
                                        int*            AitemID, 
                                        char**          AitemType, 
                                        int*            AisLast)
{
	API_DUMY_ERROR("MC_Dir_First_Record");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Next_Record
                                       (int             AdirID,
                                        int             AentityID, 
                                        int*            AitemID, 
                                        char**          AitemType, 
                                        int*            AisLast)
{
	API_DUMY_ERROR("MC_Dir_Next_Record");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Add_Entity     
                                       (int             AdirID, 
                                        int             AitemID,
                                        const char*     AnewItemType,
                                        int*            AnewEntityID, 
                                        int*            AnewItemID)
{
	API_DUMY_ERROR("MC_Dir_Add_Entity");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Delete_Referenced_Entity
                                       (int    AdirID,
                                        int             AitemID)
{
	API_DUMY_ERROR("MC_Dir_Delete_Referenced_Entity");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Add_Record     
                                       (int             AdirID, 
                                        int             AentityID, 
                                        const char*     AnewItemType,
                                        int*            AnewItemID)
{
	API_DUMY_ERROR("MC_Dir_Add_Record");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Delete_Record  
                                       (int             AdirID,
                                        int             AitemID)
{
	API_DUMY_ERROR("MC_Dir_Delete_Record");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Root_Count
                                       (int    AdirID,
                                        int*   Acount)
{
	API_DUMY_ERROR("MC_Dir_Root_Count");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Entity_Count
                                       (int    AdirID,
                                        int    AentityID,
                                        int*   Acount)
{
	API_DUMY_ERROR("MC_Dir_Entity_Count");
	return MC_ERROR;
}  

inline MC_STATUS MC_Dir_Item_Count
                                       (int    AdirID,
                                        int    AitemID,
                                        int*   Acount)
{
	API_DUMY_ERROR("MC_Dir_Item_Count");
	return MC_ERROR;
}  


inline MC_STATUS MC_Standard_Compressor     (int            AmsgID,
                                                     void**         Context, 
                                                     unsigned long  AinputLength, 
                                                     void*          AinputBuffer,
                                                     unsigned long* AoutputLength,
                                                     void**         AoutputBuffer,
                                                     int            AisFirst,
                                                     int            AisLast, 
                                                     int            Arelease)
{
	API_DUMY_ERROR("MC_Standard_Compressor");
	return MC_ERROR;
}  

inline MC_STATUS  MC_Standard_Decompressor(int AmsgID,void **Context, 
                                 unsigned long AinputLength, void *AinputBuffer,
                                 unsigned long *AoutputLength,void **AoutputBuffer,
                                 int AisFirst,int AisLast,int Arelease)
{
	API_DUMY_ERROR("MC_Standard_Decompressor");
	return MC_ERROR;
}  
inline MC_STATUS  MC_Register_Compression_Callbacks(
                                                 int            AmsgID,
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
                                                        int             CBrelease)
                                                 )
{
	API_DUMY_ERROR("MC_Register_Compression_Callbacks");
	return MC_ERROR;
}  

inline MC_STATUS MC_Get_Encapsulated_Value_To_Function(int            AmsgID,
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
	API_DUMY_ERROR("MC_Get_Encapsulated_Value_To_Function");
	return MC_ERROR;
}  

inline MC_STATUS MC_Get_Next_Encapsulated_Value_To_Function(int            AmsgID,
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
	API_DUMY_ERROR("MC_Get_Next_Encapsulated_Value_To_Function");
	return MC_ERROR;
}  

///////////////////
///
////////////////////

#endif // !defined(AFX_IDICOM_LIB_API_DUMY_H_)

