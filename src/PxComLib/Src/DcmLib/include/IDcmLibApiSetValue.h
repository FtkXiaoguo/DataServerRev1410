// 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_API_SET_VALUE_H_)
#define AFX_IDICOM_LIB_API_SET_VALUE_H_
 

inline MC_STATUS  MC_Set_Value_To_Empty (int            AmsgID,
                                                 unsigned long   Atag)
{
	DEC_MESSAGE(AmsgID);
 
	if(messagePtr->DeleteTag(Atag ))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
} 

inline MC_STATUS  MC_Set_Value_From_String(int            AmsgID,
                                                 unsigned long   Atag,
                                                 const char*     Avalue)
{
	DEC_MESSAGE(AmsgID);
 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS  MC_Set_Value_From_Int (int            AmsgID,
                                                 unsigned long   Atag,
                                                 int             Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS  MC_Set_Value_From_UInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned int    Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS  MC_Set_Value_From_Double(int            AmsgID,
                                                 unsigned long   Atag,
                                                 double           Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
inline MC_STATUS  MC_Set_Value_From_Float(int            AmsgID,
                                                 unsigned long   Atag,
                                                 float           Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

//
inline MC_STATUS   MC_Set_Value_From_ShortInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 short           Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}
//
inline MC_STATUS  MC_Set_Value_From_UShortInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned short    Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

 //

//
inline MC_STATUS  MC_Set_Value_From_LongInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                  long    Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}


inline MC_STATUS  MC_Set_Value_From_ULongInt(int            AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned long    Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}



struct CBDataStruct
{
	unsigned char* m_buf;
	unsigned long m_bufsize;
};

 
inline MC_STATUS  MC_Set_Value_From_Function(
                                                 int            AmsgID,
                                                 unsigned long  Atag,
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
	DEC_MESSAGE(AmsgID);

	CBDataStruct *cbdata = (CBDataStruct *)AuserInfo;
 
	if(cbdata==0){
		if(AuserFunction){
			int  A_dataSize;
			void*  A_dataBufferPtr;
			int  A_isLastPtr;
			MC_STATUS (*setDataFuc)(int A_msgID, unsigned long A_tag, int A_isFirst, void* A_info, 
							int* A_dataSize, void** A_dataBufferPtr, int* A_isLastPtr) = AuserFunction;
			if(MC_NORMAL_COMPLETION == (*setDataFuc)(AmsgID,Atag,1,0,
				&A_dataSize,&A_dataBufferPtr,&A_isLastPtr)){

				if(messagePtr->Set_ArrayValue(Atag, A_dataBufferPtr,A_dataSize))
				{
					return MC_NORMAL_COMPLETION;
				}else{
					return MC_ERROR;
				}
			}
 
		}
		return MC_ERROR;
	}
	if(messagePtr->Set_ArrayValue(Atag, cbdata->m_buf,cbdata->m_bufsize))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
	 
}
inline MC_STATUS  MC_Set_Value_To_NULL  (int            AmsgID,
                                                 unsigned long  Atag)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_ValueToNull(Atag))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}


///////////////////////
///////////////////////
inline MC_STATUS   MC_Set_Next_Value_From_String(int            AmsgID,
                                                unsigned long   Atag,
                                                const char*     Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(Atag, Avalue,true/*append*/))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
	 
}
///


inline MC_STATUS   MC_Set_Next_Value_From_Double(int            AmsgID,
                                                unsigned long   Atag,
                                                double          Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%f",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_Value_From_Float(int            AmsgID,
                                                unsigned long   Atag,
                                                float           Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%f",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_Value_From_Int(int            AmsgID,
                                                unsigned long   Atag,
                                                int             Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_Value_From_ShortInt(int            AmsgID,
                                                unsigned long   Atag,
                                                short           Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_Value_From_LongInt(int            AmsgID,
                                                unsigned long   Atag,
                                                long            Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}



inline MC_STATUS   MC_Set_Next_Value_From_UInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned int   Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_Value_From_UShortInt(int           AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned short Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_Value_From_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long  Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_Value_From_String(AmsgID,Atag,_str_buff_);
}
///////////////
//////////////////////////////////////////
// for private Tag
///////////////////////////////////////////
#define PrivateElementOffset (0x1000)
#define _GenPrivateTag_(g,e) (((g)<<16)+(e)+PrivateElementOffset)
inline MC_STATUS   MC_Add_Private_Attribute(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  AelementByte,
                                                 MC_VR          Avr)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Add_PrivateTag(Agroup, AelementByte+PrivateElementOffset, Avr,AprivateCode ))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}

	
	return MC_ERROR;
}

inline MC_STATUS   MC_Add_Private_Block  (int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Add_PrivateBlock(Agroup,AprivateCode ))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
	return MC_ERROR;
}

inline MC_STATUS   MC_Set_pValue_From_String(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 const char*    Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(_GenPrivateTag_(Agroup,Aelem), Avalue , false/*append*/,AprivateCode))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS   MC_Set_pValue_From_Double(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 double         Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%f",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_pValue_From_Float(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 float          Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%f",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_pValue_From_Int(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 int            Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_pValue_From_ShortInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 short          Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_pValue_From_LongInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 long           Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}



inline MC_STATUS   MC_Set_pValue_From_UInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned int   Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_pValue_From_UShortInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned short Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_pValue_From_ULongInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned long  Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}


////////////////////
inline MC_STATUS   MC_Set_Next_pValue_From_String (int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 const char*    Avalue)
{
	DEC_MESSAGE(AmsgID);

 
	if(messagePtr->Set_Value(_GenPrivateTag_(Agroup,Aelem), Avalue , true/*append*/,AprivateCode))
	{
		return MC_NORMAL_COMPLETION;
	}else{
		return MC_ERROR;
	}
}

inline MC_STATUS   MC_Set_Next_pValue_From_Double(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 double         Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%f",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_pValue_From_Float(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 float          Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%f",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_pValue_From_Int(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 int            Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_pValue_From_ShortInt(int           AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 short          Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_pValue_From_LongInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 long           Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}



inline MC_STATUS   MC_Set_Next_pValue_From_UInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned int   Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_pValue_From_UShortInt(int          AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned short Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

inline MC_STATUS   MC_Set_Next_pValue_From_ULongInt(int           AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned long  Avalue)
{
	char _str_buff_[256];
	sprintf(_str_buff_,"%d",Avalue);
	return MC_Set_Next_pValue_From_String(AmsgID,AprivateCode,Agroup,Aelem,_str_buff_);
}

#endif // !defined(AFX_IDICOM_LIB_API_SET_VALUE_H_)

