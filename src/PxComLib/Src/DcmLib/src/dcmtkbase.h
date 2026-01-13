// DcmTkLibBase.h
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_DCMTK_LIB_BASE_H_)
#define AFX_DCMTK_LIB_BASE_H_
 
#include "DcmLocalString.h" 

inline DcmTagKey GenDcmTagKey(unsigned long tag)
{
	Uint16 tag_g = (tag&(0xffff0000) ) >>16;
	Uint16 tag_e = (tag&(0x0000ffff) );

	return DcmTagKey(tag_g,tag_e);;
}

inline DcmTag GenDcmTagKeyEx(unsigned long tag,const char* pcreator=0)
{
	Uint16 tag_g = (tag&(0xffff0000) ) >>16;
	Uint16 tag_e = (tag&(0x0000ffff) );

	DcmTag tag_key =  DcmTagKey(tag_g,tag_e);
	if(pcreator){
		tag_key.setPrivateCreator(pcreator);
		tag_key.lookupVRinDictionary();
	}
	return tag_key;

}
 
inline void removeItemFromeDataSet(DcmDataset *dataset,const DcmTagKey &tag)
{
	 
	DcmElement *ele = dataset->remove(tag);
	if(ele) delete ele;
}
 
inline void cnvtVal2Str(unsigned short val,		char *_buff,	int buff_size)
{
	sprintf(_buff,"%d",val);
}

inline void cnvtVal2Str( short val,				char *_buff,	int buff_size)
{	
	sprintf(_buff,"%d",val);
}

inline void cnvtVal2Str(unsigned int val,		char *_buff,	int buff_size)
{
	sprintf(_buff,"%d",val);
}

inline void cnvtVal2Str( int val,				char *_buff,	int buff_size)
{
	sprintf(_buff,"%d",val);
}

inline void cnvtVal2Str(unsigned long val,		char *_buff,	int buff_size)
{
	sprintf(_buff,"%d",val);
}

inline void cnvtVal2Str( long val,				char *_buff,	int buff_size)
{
	sprintf(_buff,"%d",val);
}

inline void cnvtVal2Str( float val,				char *_buff,	int buff_size)
{
	sprintf(_buff,"%f",val);
}

inline void cnvtVal2Str( double val,			char *_buff,	int buff_size)
{
	sprintf(_buff,"%f",val);
}

inline void cnvtVal2Str(const std::string  &val,	char *_buff,	int buff_size)
{
	strncpy(_buff,val.c_str(),buff_size);
}

template<class T> bool setupElementValue(DcmElement *new_elem,const T val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	OFCondition cond=EC_InvalidVR;
 
	char _val_str_buff[4096];
#if 1
	cnvtVal2Str(val,_val_str_buff,1024);
	cond = new_elem->putString(_val_str_buff);
#else
	switch (evr)
    {
		case EVR_IS :
			{
				sprintf(_val_str_buff,"%d",(int)(val));
				new_elem->putString(_val_str_buff);
					 
			}
			break;
        case EVR_SS :
			cond = new_elem->putSint16(val);
            break;
		case EVR_CS :
			cond = new_elem->putSint16(val);
            break;
        case EVR_xs : // according to Dicom-Standard V3.0
        case EVR_US :
            cond = new_elem->putUint16(val);
            break;
        case EVR_SL :
            cond = new_elem->putSint32(val);
            break;
        case EVR_up : // for (0004,eeee) according to Dicom-Standard V3.0
        case EVR_UL :
			cond = new_elem->putUint32(val);
			break;
		case EVR_DS :
			{
				sprintf(_val_str_buff,"%d", val);
				new_elem->putString(_val_str_buff);		 
			}
			break;
        case EVR_FL:
            cond = new_elem->putFloat32(val);
            break;
        case EVR_FD :
            cond = new_elem->putFloat64(val);
	}
#endif
	if(cond.bad()){
		return false;
	}
	return true;

}

////////////
// get value
///////////
#define tryGetUint8(new_elem,ret_val) {\
	OFCondition cond=EC_InvalidVR;\
	Uint8 data_temp;\
	cond = new_elem->getUint8(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetSint16(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Sint16 data_temp;\
	cond = new_elem->getSint16(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetUint16(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Uint16 data_temp;\
	cond = new_elem->getUint16(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetUint16(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Uint16 data_temp;\
	cond = new_elem->getUint16(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetSint32(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Sint32 data_temp;\
	cond = new_elem->getSint32(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetUint32(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Uint32 data_temp;\
	cond = new_elem->getUint32(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetFloat32(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Float32 data_temp;\
	cond = new_elem->getFloat32(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
//
#define tryGetFloat64(new_elem,ret_val){\
	OFCondition cond=EC_InvalidVR;\
	Float64 data_temp;\
	cond = new_elem->getFloat64(data_temp);\
	if(cond.good()){\
		ret_val = data_temp;\
		return true;\
	}\
}
inline bool getValue(DcmElement *new_elem,unsigned char &val)  
{
	DcmTag tag = new_elem->getTag();
	

	tryGetUint8(new_elem,val);

	//
	tryGetSint16(new_elem, val);
	tryGetUint16(new_elem, val);
	//
	tryGetSint32(new_elem, val);
	tryGetUint32(new_elem, val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);


	return  false ;
}

inline bool getValue(DcmElement *new_elem,short &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();

	tryGetSint16(new_elem, val);

	//
//	tryGetSint16(new_elem, val);
	tryGetUint16(new_elem, val);
	//
	tryGetSint32(new_elem, val);
	tryGetUint32(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);

	return  false ;
}
inline bool getValue(DcmElement *new_elem,unsigned short &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	 
	tryGetUint16(new_elem, val);

	//
	tryGetSint16(new_elem, val);
//	tryGetUint16(new_elem, val);
	//
	tryGetSint32(new_elem, val);
	tryGetUint32(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);
	//
	return  false ;
}
////
inline bool getValue(DcmElement *new_elem,int &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();

	tryGetSint32(new_elem, val);

	//
	//
	tryGetSint16(new_elem, val);
 	tryGetUint16(new_elem, val);
	//
//	tryGetSint32(new_elem, val);
	tryGetUint32(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);
	//
 
	return  false ;
}
inline bool getValue(DcmElement *new_elem,unsigned int &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	
	tryGetUint32(new_elem, val);
	//
	//
	tryGetSint16(new_elem, val);
 	tryGetUint16(new_elem, val);
	//
	tryGetSint32(new_elem, val);
//	tryGetUint32(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);
	//
	 
	return  false ;
}
////
inline bool getValue(DcmElement *new_elem,long &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	 
	tryGetSint32(new_elem, val);

	//
	//
	tryGetSint16(new_elem, val);
 	tryGetUint16(new_elem, val);
	//
//	tryGetSint32(new_elem, val);
	tryGetUint32(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);
	//
	return  false ;
}
inline bool getValue(DcmElement *new_elem,unsigned long &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	
	tryGetUint32(new_elem, val);
	//
	//
	tryGetSint16(new_elem, val);
 	tryGetUint16(new_elem, val);
	//
	tryGetSint32(new_elem, val);
//	tryGetUint32(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);
	//
	return  false ;
}
////
inline bool getValue(DcmElement *new_elem,float &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();

	tryGetFloat32(new_elem, val);
	//
	//
//	tryGetFloat32(new_elem, val);
	tryGetFloat64(new_elem, val);
//
	tryGetSint32(new_elem, val);
 	tryGetUint32(new_elem, val);
	//	 
	tryGetSint16(new_elem, val);
 	tryGetUint16(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//

	return  false ;
}
inline bool getValue(DcmElement *new_elem,double &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	
	tryGetFloat64(new_elem, val);
//
//
	tryGetFloat32(new_elem, val);
//	tryGetFloat64(new_elem, val);
//
	tryGetSint32(new_elem, val);
 	tryGetUint32(new_elem, val);
	//	 
	tryGetSint16(new_elem, val);
 	tryGetUint16(new_elem, val);
	//
	tryGetUint8(new_elem,val);
	//
	return  false ;
}
///////
inline bool getValue(DcmElement *new_elem,std::string &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	OFCondition cond=EC_InvalidVR;

	char *data_temp;
	cond = new_elem->getString(data_temp);
	if(cond.good()){
		if(data_temp){
			val = data_temp;
		}else{
			val = "";
		}
	}else{
		if((tag.getGTag()!=0x7FE0) || (tag.getETag()!=0x0010 )){
			//try to DcmOtherByteOtherWord
			Uint8 *ow_data=0;
			cond = new_elem->getUint8Array(ow_data);
			if(cond.good()){
				if(ow_data){
					data_temp = (char*)ow_data;
					val =  data_temp;
				}
			}
		}
			 
	}
	return  cond.good() ;
}
 
#if 0
inline bool getValue(DcmElement *new_elem,dcm_string &val)  
{
	std::string str_temp;
	if(getValue( new_elem,str_temp) ){
		val = str_temp.c_str();
		return true;
	}else{
		return false;
	}

	 
}
#endif


template<class T> bool getElementValueInt(DcmElement *new_elem,T &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	OFCondition cond=EC_InvalidVR;
 
	bool have_val = true;
	switch (evr)
    {
        case EVR_SS :
			{
				Sint16 data_temp;
				cond = new_elem->getSint16(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_xs : // according to Dicom-Standard V3.0
        case EVR_US :
			{
				Uint16 data_temp;
				cond = new_elem->getUint16(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
		case EVR_IS :
        case EVR_SL :
			{
				Sint32 data_temp;
				cond = new_elem->getSint32(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_up : // for (0004,eeee) according to Dicom-Standard V3.0
        case EVR_UL :
			{
				Uint32 data_temp;
				cond = new_elem->getUint32(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
			break;
        case EVR_FL:
			{
				Float32 data_temp;
				cond = new_elem->getFloat32(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_FD :
			{
				Float64 data_temp;
				cond = new_elem->getFloat64(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
			break;
		case EVR_DS :
			{
				have_val = false;
			}
			break;
	}
 
	if(cond.bad()){
		return false;
	}
	return have_val;

}

template<class T> bool getElementValueString(DcmElement *new_elem,T &val)  
{
	DcmTag tag = new_elem->getTag();
	DcmEVR evr=tag.getEVR();
	OFCondition cond=EC_InvalidVR;
 
	
	switch (evr)
    {
        case EVR_SS :
			{
				Sint16 data_temp;
				cond = new_elem->getSint16(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_xs : // according to Dicom-Standard V3.0
        case EVR_US :
			{
				Uint16 data_temp;
				cond = new_elem->getUint16(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_SL :
			{
				Sint32 data_temp;
				cond = new_elem->getSint32(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_up : // for (0004,eeee) according to Dicom-Standard V3.0
        case EVR_UL :
			{
				Uint32 data_temp;
				cond = new_elem->getUint32(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
			break;
        case EVR_FL:
			{
				Float32 data_temp;
				cond = new_elem->getFloat32(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
            break;
        case EVR_FD :
			{
				Float64 data_temp;
				cond = new_elem->getFloat64(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
			break;
		case EVR_DS :
		case EVR_UI :
			{
			 	char *data_temp;
			 	cond = new_elem->getString(data_temp);
			//	OFString data_temp;
			//	cond = new_elem->getStringValue(data_temp);
				if(cond.good()){
					val = data_temp;
				}
			}
			break;
	}
 
	if(cond.bad()){
		return false;
	}
	return true;

}

#endif // !defined(AFX_DCMTK_LIB_BASE_H_)