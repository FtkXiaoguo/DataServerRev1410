// 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDICOM_LIB_API_GET_VALUE_H_)
#define AFX_IDICOM_LIB_API_GET_VALUE_H_
 

#define good_status(status) ( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) )
#define assign_value(status,ret_val,val_temp) if( (status) == MC_NORMAL_COMPLETION ) (ret_val) = (val_temp) 

template <class T> MC_STATUS get_val_gen(int  AmsgID,unsigned long   Atag, T *Abuffer)
{
	DEC_MESSAGE(AmsgID);
	 
	T val_temp;
	 
	MC_STATUS status = messagePtr->Get_Value(Atag, val_temp);
		 
	if(good_status(status)) 
	{
		assign_value(status,*Abuffer,val_temp);
		return status;
	}else{
		return MC_INVALID_TAG;
	}
};


template <class T> MC_STATUS get_next_val_gen_flat(int AmsgID,unsigned long    Atag , T *Abuffer) { 
	char _str_buff_[256];
	MC_STATUS status = MC_Get_Next_Value_To_String(AmsgID,Atag,256,_str_buff_);
	if(good_status(status)) 
	{
		*Abuffer = atof(_str_buff_);
		return status;
	}
	return MC_ERROR;
}

template <class T> MC_STATUS get_next_val_gen_integer(int AmsgID,unsigned long    Atag , T *Abuffer) { 
	char _str_buff_[256];
	MC_STATUS status = MC_Get_Next_Value_To_String(AmsgID,Atag,256,_str_buff_);
	if(good_status(status)) 
	{
		*Abuffer = atoi(_str_buff_);
		return status;
	}
	return MC_ERROR;
}
 
inline MC_STATUS  MC_Get_Value_To_Function(int            AmsgID,
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
	if(IDcmLibApi::Get_Value_To_Function( AmsgID, Atag, AuserInfo, AuserFunction))
	{
		return MC_NORMAL_COMPLETION;
	}else{
	 
		return MC_ERROR;
	}
}



///
///
inline MC_STATUS  MC_Get_Value_Length   (int            AmsgID,
                                                 unsigned long  Atag,
                                                 int            AvalueNumber,
                                                 unsigned long* Alength)
{
	DEC_MESSAGE(AmsgID);

	return messagePtr->Get_ValueLength(Atag, AvalueNumber, *Alength)  ;
}
///



///
///
///

inline MC_STATUS   MC_Get_Value_To_String(int            AmsgID,
                                                 unsigned long  Atag,
                                                 int            AbufferSize,
                                                 char*          Abuffer)
{
	DEC_MESSAGE(AmsgID);

//	DcmXTDataSet * DataSetPtr = messagePtr->getDcmXTDataSet();
//	if(!DataSetPtr) return MC_ERROR;

#if 0
	dllString str_temp;
	MC_STATUS status = messagePtr->Get_Value(Atag, str_temp);
	if(good_status(status)) 
	{
		IDcmLibApi::myStrcpy(str_temp,	Abuffer,	AbufferSize);
		return status;
	}else{
		return MC_INVALID_TAG;
	}
#else
	MC_STATUS status = messagePtr->Get_Value(Atag, Abuffer,AbufferSize);
	if(good_status(status)) 
	{
		return status;
	}else{
		return MC_INVALID_TAG;
	}
#endif
}

inline MC_STATUS   MC_Get_Value_To_UInt  (int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned int*  Abuffer)
{
	DEC_MESSAGE(AmsgID);

 
	 
	unsigned int uint_temp;
	 
	MC_STATUS status = MC_INVALID_TAG;
	switch(Atag){
		default:
			{
				status = messagePtr->Get_Value(Atag, uint_temp);
			}
			break;
		case MC_ATT_MESSAGE_ID_BEING_RESPONDED_TO:
			{
				if(messagePtr->getMessageIDFromRsp(uint_temp)){
					status = MC_NORMAL_COMPLETION;
				}
			}
			break;
		case MC_ATT_STATUS:
			{
				if(messagePtr->getStatusFromRsp(uint_temp)){
					status = MC_NORMAL_COMPLETION;
				}
			}
			break;
	};
	if(good_status(status)) 
	{
		assign_value(status,*Abuffer,uint_temp);
		 
		return status;
	}else{
		return MC_INVALID_TAG;
	}
}

inline MC_STATUS   MC_Get_Value_To_Double(int            AmsgID,
                                                 unsigned long  Atag,
                                                 double*        Abuffer)
{
 
	return get_val_gen(  AmsgID, Atag,  Abuffer);

}

inline MC_STATUS   MC_Get_Value_To_Float (int            AmsgID,
                                                 unsigned long  Atag,
                                                 float*         Abuffer)
{
	return get_val_gen(  AmsgID, Atag,  Abuffer);

}

inline MC_STATUS   MC_Get_Value_To_Int   (int            AmsgID,
                                                 unsigned long  Atag,
                                                 int*           Abuffer)
{
	DEC_MESSAGE(AmsgID);

 
	MC_STATUS status = messagePtr->Get_Value(Atag, *Abuffer);
	 
	return status;
	  
}

inline MC_STATUS   MC_Get_Value_To_LongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 long int*      Abuffer)
{
	return get_val_gen(  AmsgID, Atag,  Abuffer); 
	 
}

inline MC_STATUS   MC_Get_Value_To_ShortInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 short int*     Abuffer)
{
	return get_val_gen(  AmsgID, Atag,  Abuffer);
}



inline MC_STATUS   MC_Get_Value_To_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long* Abuffer)
{
	return get_val_gen(  AmsgID, Atag,  Abuffer);
}

inline MC_STATUS   MC_Get_Value_To_UShortInt(int             AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned short* Abuffer)
{
	return get_val_gen(  AmsgID, Atag,  Abuffer);
}


inline MC_STATUS   MC_Get_Value_Count    (int            AmsgID,
                                                 unsigned long  Atag,
                                                 int*           Abuffer)
{
//	return get_val_gen(  AmsgID, Atag,  Abuffer);
	DEC_MESSAGE(AmsgID);

	return messagePtr->Get_ValueCount(Atag, *Abuffer); 
}

////////////////
inline MC_STATUS   MC_Get_Next_Value_To_String(int            AmsgID,
                                                 unsigned long  Atag,
                                                 int            bufferSize,
                                                 char*          Abuffer)
{
	DEC_MESSAGE(AmsgID);
#if 0
	dllString str_temp;
	MC_STATUS status = messagePtr->Get_ValueNext(Atag, str_temp); 
	if(good_status(status)) 
	{
		 
		IDcmLibApi::myStrcpy(str_temp,	Abuffer,	bufferSize);

		return status;
	}else{
		return MC_INVALID_TAG;
	}
#else
	MC_STATUS status = messagePtr->Get_ValueNext(Atag, Abuffer, bufferSize); 
	if(good_status(status)) 
	{
		return status;
	}else{
		return MC_INVALID_TAG;
	}
#endif
}

 


//////
inline MC_STATUS   MC_Get_Next_Value_To_Double(int            AmsgID,
                                                 unsigned long  Atag,
                                                 double*        Abuffer)
{
	return get_next_val_gen_flat(AmsgID,Atag,Abuffer);
}

inline MC_STATUS   MC_Get_Next_Value_To_Float(int            AmsgID,
                                                 unsigned long  Atag,
                                                 float*         Abuffer)
{
	return get_next_val_gen_flat(AmsgID,Atag,Abuffer);
}

inline MC_STATUS   MC_Get_Next_Value_To_Int(int            AmsgID,
                                                 unsigned long  Atag,
                                                 int*           Abuffer)
{
	return get_next_val_gen_integer(AmsgID,Atag,Abuffer);
}

inline MC_STATUS   MC_Get_Next_Value_To_LongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 long int*      Abuffer)
{
	return get_next_val_gen_integer(AmsgID,Atag,Abuffer);
}

inline MC_STATUS   MC_Get_Next_Value_To_ShortInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 short int*     Abuffer)
{
	return get_next_val_gen_integer(AmsgID,Atag,Abuffer);
}



inline MC_STATUS   MC_Get_Next_Value_To_UInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned int*  Abuffer)
{
	return get_next_val_gen_integer(AmsgID,Atag,Abuffer);
}

inline MC_STATUS   MC_Get_Next_Value_To_ULongInt(int            AmsgID,
                                                 unsigned long  Atag,
                                                 unsigned long* Abuffer)
{
	return get_next_val_gen_integer(AmsgID,Atag,Abuffer);
}

inline MC_STATUS   MC_Get_Next_Value_To_UShortInt(int             AmsgID,
                                                 unsigned long   Atag,
                                                 unsigned short* Abuffer)
{
	return get_next_val_gen_integer(AmsgID,Atag,Abuffer);
}


/* ======================================================================== *
 *              Functions to retrieve PRIVATE attribute values              *
 * ======================================================================== */
inline MC_STATUS  MC_Get_pValue_To_Function(int            AmsgID,
                                                 const char*     AprivateCode,
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem,
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
	if(IDcmLibApi::Get_Value_To_Function( AmsgID, _GenPrivateTag_(Agroup,Aelem), AuserInfo, AuserFunction))
	{
		return MC_NORMAL_COMPLETION;
	}else{
	 
		return MC_ERROR;
	}
}


inline MC_STATUS   MC_Get_pValue_Count   (int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 int*           Acount)
{
	DEC_MESSAGE(AmsgID);

	return messagePtr->Get_ValueCount(_GenPrivateTag_(Agroup,Aelem), *Acount);
	 
}
inline MC_STATUS MC_Get_pValue_To_String(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 int            AbufferSize,
                                                 char*          Abuffer)
{
	return MC_Get_Value_To_String(AmsgID,_GenPrivateTag_(Agroup,Aelem),AbufferSize,Abuffer);
}
//
inline MC_STATUS   MC_Get_pValue_Length  (int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 int            AvalueNumber,
                                                 unsigned long* Alength)
{
	DEC_MESSAGE(AmsgID);

	return messagePtr->Get_ValueLength(_GenPrivateTag_(Agroup,Aelem), AvalueNumber,*Alength) ;
	 
 
}


inline MC_STATUS  MC_Get_pValue_To_ULongInt(int            AmsgID,
                                                 const char*    AprivateCode,
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem,
                                                 unsigned long* Abuffer)
{
	return MC_Get_Value_To_ULongInt(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
}

inline MC_STATUS  MC_Get_pValue_To_LongInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 long int*      Abuffer)
{
	return MC_Get_Value_To_LongInt(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
}

inline MC_STATUS  MC_Get_pValue_To_Int  (int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 int*           Abuffer)
{
	return MC_Get_Value_To_Int(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
}


inline MC_STATUS MC_Get_pValue_To_UShortInt(int             AmsgID,
                                                 const char*     AprivateCode,
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem,
                                                 unsigned short* Abuffer)
{
 
	return MC_Get_Value_To_UShortInt(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
 
}

inline MC_STATUS MC_Get_pValue_To_ShortInt(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 short int*     Abuffer)
{
	return MC_Get_Value_To_ShortInt(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
}

inline MC_STATUS MC_Get_pValue_To_Double(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 double*        Abuffer)
{
	return MC_Get_Value_To_Double(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
}

inline MC_STATUS MC_Get_pValue_To_Float(int            AmsgID,
                                                 const char*    AprivateCode, 
                                                 unsigned short Agroup,
                                                 unsigned char  Aelem, 
                                                 float*         Abuffer)
{
	return MC_Get_Value_To_Float(AmsgID,_GenPrivateTag_(Agroup,Aelem),Abuffer);
}
////////////////////////////
inline MC_STATUS MC_Get_Next_pValue_To_String(int            AmsgID,
                                                 const char*     AprivateCode,
                                                 unsigned short  Agroup,
                                                 unsigned char   Aelem,
                                                 int             AbufferSize,
                                                 char*           Abuffer)
{
	return MC_Get_Next_Value_To_String(AmsgID,_GenPrivateTag_(Agroup,Aelem),AbufferSize,Abuffer);
}
 

inline MC_STATUS   MC_Get_File_Length 
                                       (int             AfileID,
                                        unsigned long*  AmsgLength)
{
	DEC_MESSAGE(AfileID);

	return messagePtr->Get_FileLength(*AmsgLength)  ;
}

///////////
inline MC_STATUS  MC_Get_Attribute_Info (int            AmsgID,
                                                 unsigned long  Atag,
                                                 MC_VR*         Avr,
                                                 int*           Avalues)
{
	DEC_MESSAGE(AmsgID);
	 
 
	MC_VR vr_temp;
	int values_temp;
	 
	MC_STATUS status = messagePtr->Get_AttributeInfo(Atag, vr_temp,values_temp);
		 
	if(good_status(status)) 
	{
		if(Avr){
			*Avr = vr_temp;
		}
		if(Avalues){
			*Avalues = values_temp;
		}
		 
		return status;
	}else{
		return MC_INVALID_TAG;
	}
}




/////////////////
//// “ÆŽ©‚ÌAPI
inline MC_STATUS  MC_Get_PixelOffset   (int            AmsgID,
                                          
                                                 unsigned long* Alength)
{
	DEC_MESSAGE(AmsgID);

	return messagePtr->Get_PixelOffset(*Alength)  ;
}
///


#endif // !defined(AFX_IDICOM_LIB_API_GET_VALUE_H_)

