// IXtMfcLib.cpp: IXtMfcLib クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4244)
#pragma warning (disable: 4267)
 
#include "DcmXTDataSetMain.h"

using namespace std;

//////////////////
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"
//#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcistrmz.h"    /* for dcmZlibExpectRFC1950Encoding */


#include "dcmtk/ofstd/ofstdinc.h"

#include "XDcmTk/XDcDatset.h"


#include "DcmTkBase.h"

#include "CheckMemoryLeak.h"
 
#include "DcmXTUtilMain.h"
#include <stack>
 
 
#define MySeqStack ((std::stack<DcmItem*> *)m_SeqStack)
DcmItemBase::DcmItemBase():
m_CurSequenceDcmObject(0)
,m_SeqStack(0)
{
	
	m_DcmObject = 0;
	m_delDcmObjectFlag = true;
}
 
void DcmItemBase::Delete()
{
	if(MySeqStack) delete MySeqStack;//##35 2012/09/16 K.Ko

	if(m_DcmObject){
		if(m_delDcmObjectFlag){
			delete m_DcmObject;
		}
		m_DcmObject =0;
		m_delDcmObjectFlag = true;
	}
}

 
#if 0
bool DcmItemBase::Set_ValueStrIn(unsigned long  tag,	const dcm_string &val)
{
 
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag new_tag  = GenDcmTagKey(tag);

	DcmElement *new_elem=0;

	cond = newDicomElement(new_elem,new_tag);
	if(cond.bad()){
		return false;
	}
	cond = new_elem->putString( val.c_str());
	if(cond.bad()){
		if(new_elem){
			delete new_elem;
		}
		return false;
	}

	cond = m_DcmObject->insert(new_elem,true/*replaceOld*/);
	if(cond.bad()){
		
		return false;
	}
 
	return true;
}
#endif
bool DcmItemBase::Set_ArrayValueIn(unsigned long  tag,const void*dataBuff,int dataSize)
{
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag new_tag  = GenDcmTagKey(tag);

	DcmElement *new_elem=0;

	cond = DcmItem::newDicomElement(new_elem,new_tag);
	if(cond.bad()){
		return false;
	}

	cond = new_elem->putUint8Array( (const Uint8 *)dataBuff,dataSize);
	if(cond.bad()){
		if(new_elem){
			delete new_elem;
		}
		return false;
	}

	cond = m_DcmObject->insert(new_elem,true/*replaceOld*/);
	if(cond.bad()){
		return false;
	}
 
	return true;
}

bool DcmItemBase::Set_ValueToNullIn(unsigned long  tag)
{
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag new_tag  = GenDcmTagKey(tag);

	DcmElement *new_elem=0;

	cond = DcmItem::newDicomElement(new_elem,new_tag);
	if(cond.bad()){
		return false;
	}

//	cond = new_elem->putUint8Array(  );
	if(cond.bad()){
		if(new_elem){
			delete new_elem;
		}
		return false;
	}

	if(m_CurSequenceDcmObject){//##35 2012/09/16 K.Ko
		cond = m_CurSequenceDcmObject->insert(new_elem,true/*replaceOld*/);
	}else{
		cond = m_DcmObject->insert(new_elem,true/*replaceOld*/);
	}
	if(cond.bad()){
		return false;
	}
 
	return true;
}
 //##35 2012/09/16 K.Ko
bool DcmItemBase::Begin_SequenceIn(unsigned long  tag,const char *pcreator ) 
 {
	if(!m_DcmObject){
		return false;
	}

	if(!m_SeqStack){
	//Notice: initial timing
		m_SeqStack = new std::stack<DcmItem*> ;//##35 2012/09/16 K.Ko
	}
 

	OFCondition cond;

	DcmTag new_tag  = GenDcmTagKeyEx(tag,pcreator);

	DcmElement *new_elem=0;
	 
	 
	cond = DcmItem::newDicomElement(new_elem,new_tag);

	//DcmSequenceOfItems

	if(cond.bad()){
		return false;
	}

	DcmItem *working_DcmItem = m_DcmObject;

	if(m_CurSequenceDcmObject){
		working_DcmItem = m_CurSequenceDcmObject;
		//for sub-sequence
		MySeqStack->push( m_CurSequenceDcmObject);
		 
	} 
		
 	cond = working_DcmItem->insert(new_elem,true/*replaceOld*/);
	 

	if(cond.bad()){
		return false;
	}
 
	DcmTag item_new_tag  = GenDcmTagKeyEx(0xfffee000,pcreator);
	DcmItem *item_new_elem  = new DcmItem(item_new_tag);
	 
	if(item_new_elem == 0){
		return false;
	}

	DcmSequenceOfItems *seq_obj = (DcmSequenceOfItems*)new_elem;
	if(!seq_obj){
		return false;
	}

	 
	cond = seq_obj->insert(item_new_elem,true/*replaceOld*/);

	if(cond.bad()){
		return false;
	}
	
	cond = working_DcmItem->findAndGetSequenceItem(new_tag,m_CurSequenceDcmObject );

	if(cond.bad()){
		return false;
	}

	return true;
	 
}
bool DcmItemBase:: End_SequenceIn(unsigned long  tag)
{
	 
	//for sub-sequence
	if(MySeqStack->size()>0){
 		m_CurSequenceDcmObject = MySeqStack->top() ;
		MySeqStack->pop();
	}else{
		m_CurSequenceDcmObject =  0;
	}
	return true;
}

 short DcmItemBase::procSequenceData(unsigned long  tag,				const short input)
 {
	 return input;
 }
 unsigned short DcmItemBase::procSequenceData(unsigned long  tag,	const unsigned short input)
 {
	 return input;
 }
 int  DcmItemBase::procSequenceData(unsigned long  tag,				const int input)
 {
	 return input;
 }
 unsigned int DcmItemBase::procSequenceData(unsigned long  tag,		const unsigned int input)
 {
	 return input;
 }
 long DcmItemBase::procSequenceData(unsigned long  tag,				const long input)
 {
	 return input;
 }
 unsigned long DcmItemBase::procSequenceData(unsigned long  tag ,	const unsigned long input)
 {
	 return input;
 }
 float DcmItemBase::procSequenceData(unsigned long  tag,				const float input)
 {
	 return input;
 }
 double DcmItemBase::procSequenceData(unsigned long  tag,			const double input)
 {
	 return input;
 }
//
 dcm_string DcmItemBase::procSequenceData(unsigned long  tag,		const dcm_string input)
 {
	 dcm_string val;
	if(getMultiStringCount( tag,input)>1){
		val = getMultiStringStart(tag,input);
	}else{
		val = input;
	};
	return val;
 }
#if 0
 dllString DcmItemBase::procSequenceData(unsigned long  tag,		const dllString input)
 {
	 dcm_string input_temp = input.c_str();
	 dcm_string ret_temp = procSequenceData(tag,input_temp);
	 return ret_temp.c_str();
 }
#endif

#if 0
bool DcmItemBase::Get_ValueIn(unsigned long  tag,	long int &val)
{
	bool ret_flag = false;
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
	 	ret_flag = getElementValueInt(target_element,val);
	}

	return ret_flag;
}
bool DcmItemBase::Get_ValueIn(unsigned long  tag,	 int &val)
{
	bool ret_flag = false;
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
	 	ret_flag = getElementValueInt(target_element,val);
	}

	return ret_flag;
}

bool DcmItemBase::Get_ValueIn(unsigned long  tag,	float &val)
{
	bool ret_flag = false;
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
	 	ret_flag = getElementValueInt(target_element,val);
	}

	return ret_flag;
}
//
bool DcmItemBase::Get_ValueIn(unsigned long  tag,	unsigned short &val)
{
	bool ret_flag = false;
	if(!m_DcmObject){
		return false;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
	 	ret_flag = getElementValueInt(target_element,val);
	}

	return ret_flag;
}


bool DcmItemBase::Get_ValueIn(unsigned long  tag,	dcm_string &val)
{
bool ret_flag = false;
	if(!m_DcmObject){
		return ret_flag;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
 	dcm_string  str_temp;



	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
	 	ret_flag = getElementValueString(target_element,str_temp);
	//  	val.assign(str_value.c_str());
 
		if(!ret_flag){
			char *_str_temp=0;
 			cond = target_element->getString(_str_temp);

			if(cond.good()){
				if(_str_temp){
					str_temp =  _str_temp ;
					ret_flag = true;
				}
			}
		}
 
	}
	if(ret_flag){
		if(getMultiStringCount( tag,str_temp)>1){
			val = getMultiStringStart(tag,str_temp);
		}else{
			val = str_temp;
		};
	}

	return ret_flag;
}
 
#endif

MC_STATUS DcmItemBase::Get_ValueCountIn(unsigned long  tag,	int&val)
{

	bool ret_flag = false;
	MC_STATUS ret_status = MC_INVALID_TAG;
	
	if(!m_DcmObject){
		return ret_status;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 

	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
		//there is TAG
		ret_status = MC_NULL_VALUE;
		
		if( 0x7FE00010 == tag){ //PixelData
			Uint32  pixel_len = target_element->getLength();
			if(pixel_len > 0){
				if (find_tag.getEVR() == EVR_OW || find_tag.getEVR() == EVR_lt)
				{
					/* get array of 16 bit values */
					 const size_t count = OFstatic_cast(size_t, target_element->getLength() / sizeof(Uint16));
					 val = count;
				}else{
					 const size_t count = OFstatic_cast(size_t, target_element->getLength()) ;
					 val = count;
				}
				ret_flag = true;
			}

			 
			 
		}else{
			char *_str_temp=0;
			cond = target_element->getString(_str_temp);

			if(cond.good()){
				if(_str_temp){
					val =  getMultiStringCount(tag,_str_temp);
					 
					ret_flag = true;
				}
			}else{
				//try to DcmOtherByteOtherWord
				Uint8 *ow_data;
				cond = target_element->getUint8Array(ow_data);
				if(cond.good()){
					_str_temp = (char*)ow_data;
					val =  getMultiStringCount(tag,_str_temp);
						 
					ret_flag = true;
				}
			}
		}
		
	}else{
		ret_status = MC_INVALID_TAG;;
			 
	}
	if(ret_flag){
		ret_status = MC_NORMAL_COMPLETION;
	}

	return ret_status;
}
//
MC_STATUS DcmItemBase::Get_ValueNextIn(unsigned long  tag,	dcm_string& val)
{
	bool ret_flag = false;
	MC_STATUS ret_status = MC_INVALID_TAG;
	
	if(!m_DcmObject){
		return ret_status;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 

	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
		//there is TAG
		ret_status = MC_NULL_VALUE;

		char *_str_temp=0;
		cond = target_element->getString(_str_temp);

		if(cond.good()){
			if(_str_temp){
				val =  getMultiStringNext(tag,_str_temp).c_str();
				 
				ret_flag = true;
			}
		}else{
		}
		
	}else{
		ret_status = MC_INVALID_TAG;;
			 
	}
	if(ret_flag){
		ret_status = MC_NORMAL_COMPLETION;
	}

	return ret_status;
}

 
MC_STATUS DcmItemBase::Get_ValueLengthIn(unsigned long  tag, int num,	unsigned long &val)
{
	bool ret_flag = false;
	MC_STATUS ret_status = MC_INVALID_TAG;
	
	if(!m_DcmObject){
		return ret_status;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
//	dcm_string  str_value;



	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
		//there is TAG
		ret_status = MC_NULL_VALUE;
		
		if( 0x7FE00010 == tag){ //PixelData
			Uint32  pixel_len = target_element->getLength();
			if(pixel_len > 0){
				
				val = pixel_len;
				
				ret_flag = true;
			}

			 
		}else{ //2011/06/21
			Uint32  pixel_len = target_element->getLength();
			if(pixel_len > 0){
				
				val = pixel_len;
				
				ret_flag = true;
			}

		}
		
	}else{
		ret_status = MC_INVALID_TAG;;
			 
	}
	if(ret_flag){
		ret_status = MC_NORMAL_COMPLETION;
	}

	return ret_status;
}

 
MC_STATUS DcmItemBase::Get_AttributeInfoIn(unsigned long  tag,MC_VR &Avr,int &Avalues)
{
	bool ret_flag = false;
	MC_STATUS ret_status = MC_INVALID_TAG;
	Avalues = 0;
	Avr = UNKNOWN_VR;
	DcmEVR dcm_Evr = EVR_UNKNOWN;

	if(!m_DcmObject){
		return ret_status;
	}
	OFCondition cond;

	DcmTag find_tag  = GenDcmTagKey(tag);
	 
	dcm_Evr = find_tag.getEVR();


	DcmElement * target_element;
	cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
	if(cond.good()){
 
		//there is TAG
		ret_status = MC_NULL_VALUE;
		
		Avalues = 1; //MC_NULL_VALUE;
		if( 0x7FE00010 == tag){ //PixelData
			 

			 
		}else{ //2011/06/21
			dcm_string val_temp;
			ret_flag = getValue(target_element,val_temp);
			if(ret_flag){
				if(val_temp.size()>0){
					Avalues = getMultiStringCount(  tag,val_temp);
				}
			}
		}
		
	}else{
		ret_status = MC_INVALID_TAG;;
			 
	}
	if(ret_flag){
		Avr = DcmXTUtilMain::ConvtDcmEVr2MCVr(dcm_Evr);
		ret_status = MC_NORMAL_COMPLETION;
	}

	return ret_status;
}

DcmElement *DcmItemBase::searchTagKey(unsigned long  tag ,bool searchIntoSub) const  //#24 2012/06/07 K.Ko
{
	DcmElement *ret_elem = 0;
	DcmTag tag_key  = GenDcmTagKey(tag);
	 
	DcmStack resultStack;

	OFCondition cond;

	 
	DcmItem *item =  getDcmDataPtr();
	if(item){
		cond = item->search(tag_key,resultStack,ESM_fromStackTop,
			searchIntoSub?OFTrue:OFFalse	//#24 2012/06/07 K.Ko
			); 
		if(cond.good()){
			ret_elem = OFstatic_cast(DcmElement *, resultStack.top());
		}
	}
	 
	 
	return ret_elem;
}
////
 int DcmItemBase::getMultiStringCount(unsigned long  tag,const dcm_string &str)
 {
	 if(dcm_string_size(str)<1) return 0;

	 OFString str_str = OFString(stringTochar(str));
	  
	int count =1 ;
	size_t index = string::npos;
	int search_pos = 0;
	do{
		index = str_str.find("\\",search_pos+1);
		if(index != string::npos)
		{
			count++;
			search_pos = index+1;
		}else{
			break;
		}
	}while(true);
	
	return count;
 }
 dcm_string DcmItemBase::getMultiStringStart(unsigned long  tag,const dcm_string &str)
 {
	 dcm_string ret_str ;
	 if(dcm_string_size(str)<1) return ret_str;

	  OFString str_str = OFString(stringTochar(str));

	  int start_pos = 0;//m_MultiStringProcQueu[tag];

	  char _str_buff[256];
	 int index = str_str.find("\\",start_pos);
	 if(index != string::npos)
	 {
		 if(index>0){
			str_str.copy(_str_buff, index,  0);
			_str_buff[index] = 0;
			ret_str = _str_buff;
			m_MultiStringProcQueu[tag] = index+1;
		}
	 }

	 return ret_str;
 }
 dcm_string DcmItemBase::getMultiStringNext(unsigned long  tag,const dcm_string &str)
 {
	 dcm_string ret_str ;

	 int org_str_len = dcm_string_size(str);
	 if(org_str_len<1) return ret_str;

	  OFString str_str = OFString(stringTochar(str));

	  int start_pos = m_MultiStringProcQueu[tag];
//	  if(start_pos >= (org_str_len-1)){ //2013/09/04 K.Ko
	  if(start_pos > (org_str_len-1)){//終了判定 2013/09/06
		  //end;
		  return ret_str;
	  }
	  //
	  // 一回目は普通のGet_Value、二回目がGet_ValueNextとなる。
	  // 
	  if(start_pos==0){
	  //２つ目のデータに移動
		  int index = str_str.find("\\",start_pos);
		  start_pos = index+1;
	  }


	  int str_nn = 0;
//	  char _str_buff[256];
	 int index = str_str.find("\\",start_pos);
	 if(index != string::npos)
	 {
		 if(index>0){
		   str_nn = index-start_pos;
			m_MultiStringProcQueu[tag] = index+1;
		}
	 }else{
		 if(start_pos>0){ //last
			str_nn = org_str_len - start_pos;
	//		m_MultiStringProcQueu[tag] = org_str_len-1;
			m_MultiStringProcQueu[tag] = org_str_len ;//終了判定 2013/09/06
		 }
	 }

//	 if(str_nn>1){ //2013/09/04 K.Ko
	 if(str_nn>0){
#if 0
		str_str.copy(_str_buff, str_nn,  start_pos);
		_str_buff[str_nn] = 0;
		ret_str = _str_buff;
#else
		 ret_str = str_str.substr(start_pos,str_nn).data();
#endif
	 }

	 return ret_str;
 }
	 
 bool DcmItemBase::DeleteTag(unsigned long  tag)
{
	DcmElement *tag_ele =  searchTagKey(  tag );
	if(tag_ele){
		DcmItem *item =  getDcmDataPtr();
		if(item){
			tag_ele = item->remove(tag_ele);
			delete tag_ele;
		}
	}

	return true;
}
////////////

///
DcmXTMetaHeaderMain::DcmXTMetaHeaderMain()
{

//	m_DcmObject = new DcmItem;//DcmObject;
	m_DcmObject = new DcmMetaInfo;
	 
}
 
DcmXTMetaHeaderMain::DcmXTMetaHeaderMain(DcmItem *meta,bool attach)
{
 	DcmItemBase::Delete();
	
	if(attach){
		m_DcmObject = meta;
		m_delDcmObjectFlag = false;
	}else{
		m_DcmObject = (DcmMetaInfo*)(meta->clone());
		m_delDcmObjectFlag = true;
	}
	 
//	m_DcmObject->print(COUT);
}

DcmXTMetaHeaderMain::~DcmXTMetaHeaderMain()
{
}
void DcmXTMetaHeaderMain::Delete()
{
 
 	DcmItemBase::Delete(); 
	delete this;
}

// DcmComInterface
bool DcmXTMetaHeaderMain::DeleteTag(unsigned long  tag)
{
	return DcmItemBase::DeleteTag(tag);
}

bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	int val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	unsigned int val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	short val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	unsigned short val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	long val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	unsigned long val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	float val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	double val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
// change string to char * 2012/02/16 K.Ko
bool DcmXTMetaHeaderMain::Set_Value(unsigned long  tag,	const char *str_val,bool append ,const char *privateTag)
{

	return DcmItemBase::Set_ValueIn(tag,	str_val,privateTag);
}
bool DcmXTMetaHeaderMain::Set_ArrayValue(unsigned long  tag,const void*dataBuff,int dataSize)
{
	return DcmItemBase::Set_ArrayValueIn(tag,dataBuff,dataSize);
 
}
bool DcmXTMetaHeaderMain::Set_ValueToNull(unsigned long  tag )
{
	return DcmItemBase::Set_ValueToNullIn(tag );
 
}
////
//##35 2012/09/16 K.Ko
bool DcmXTMetaHeaderMain::Begin_Sequence(unsigned long  tag )
{
	return DcmItemBase::Begin_SequenceIn(tag );
}
 
bool DcmXTMetaHeaderMain::End_Sequence(unsigned long  tag )
{

	return DcmItemBase::End_SequenceIn(tag );;
}

//
//
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	unsigned short &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	short &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
//
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	unsigned int &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	int &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
//
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	unsigned long &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	long &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
//
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	float &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	double &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
///////
MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag, dcm_string &val,bool Sequence)
{
	 
 
	MC_STATUS status = DcmItemBase::Get_ValueIn( tag,	val, Sequence);
	 
	return status;
}

MC_STATUS DcmXTMetaHeaderMain::Get_Value(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence)
{
	// change string to char * 2012/02/16 K.Ko
	std::string val_temp;
	MC_STATUS status = DcmItemBase::Get_ValueIn( tag,	val_temp, Sequence);
	if(status == MC_NORMAL_COMPLETION){
		strncpy(str_buff,val_temp.c_str(),buff_len);
	};
	return status;
}
MC_STATUS DcmXTMetaHeaderMain::Get_ValueCount(unsigned long  tag,	int &val)
{
	return DcmItemBase::Get_ValueCountIn( tag,	 val);
}
MC_STATUS DcmXTMetaHeaderMain::Get_ValueNext(unsigned long  tag,	dcm_string &val)
{
	return DcmItemBase::Get_ValueNextIn( tag,	val);
}

MC_STATUS DcmXTMetaHeaderMain::Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len)
{
	dcm_string str_temp;
	MC_STATUS status = DcmItemBase::Get_ValueNextIn( tag,	str_temp);
	if(status == MC_NORMAL_COMPLETION){
		strncpy(str_buff,str_temp.c_str(),buff_len);
	};
	return status;
	 
}
 
MC_STATUS DcmXTMetaHeaderMain::Get_ValueLength(unsigned long  tag, int num,	unsigned long &val)
{
	return DcmItemBase::Get_ValueLengthIn( tag,  num, val);
}
 
MC_STATUS DcmXTMetaHeaderMain::Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues)
{
	return DcmItemBase::Get_AttributeInfoIn( Atag, Avr, Avalues);
}
 
/////

//////
DcmXTDataSetMain::DcmXTDataSetMain()
{
//	m_DcmObject = new DcmItem;//DcmObject;
	m_DcmObject = new XDcmDataset;
}
DcmXTDataSetMain::DcmXTDataSetMain(DcmDataset *data,bool attach)
{
	DcmItemBase::Delete();

	if(attach){
		m_DcmObject = data;
		m_delDcmObjectFlag = false;
	}else{
		m_DcmObject = (DcmDataset*)(data->clone());
		m_delDcmObjectFlag = true;
	}
 
}

DcmXTDataSetMain::~DcmXTDataSetMain()
{
}
void DcmXTDataSetMain::Delete()
{
	DcmItemBase::Delete();
	delete this;
}

#if 0
bool DcmXTDataSetMain::readFile(const dcm_string & fileName)
{
	E_FileReadMode readMode = ERM_autoDetect;
	OFCmdUnsignedInt maxReadLength = 4096; // default is 4 KB
	E_TransferSyntax xfer = EXS_Unknown;

	 DcmFileFormat dfile;
    DcmObject *dset = &dfile;


	SetDebugLevel(5);

	const char *ifname = stringTochar(fileName);

    if (readMode == ERM_dataset) dset = dfile.getDataset();
    OFCondition cond = dfile.loadFile(ifname, xfer, EGL_noChange, maxReadLength, readMode);
    if (! cond.good())
    {
         return false;
    }
//	dset->print(COUT);

	return true;
};
#endif

// DcmComInterface
 


bool DcmXTDataSetMain::DeleteTag(unsigned long  tag)
{
	return DcmItemBase::DeleteTag(tag);
}
//
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	int val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	unsigned int val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	short val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	unsigned short val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	long val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	unsigned long val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
//
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	float val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	double val,bool append,const char *privateTa)
{
	return DcmItemBase::Set_ValueIn(tag,	val);
 
}

// change string to char * 2012/02/16 K.Ko
bool DcmXTDataSetMain::Set_Value(unsigned long  tag,	const char *str_val,bool append ,const char *privateTag)
{
	return DcmItemBase::Set_ValueIn(tag,	str_val,privateTag);
 
}

bool DcmXTDataSetMain::Set_ArrayValue(unsigned long  tag,const void*dataBuff,int dataSize)
{
	return DcmItemBase::Set_ArrayValueIn(tag,dataBuff,dataSize);
 
}
bool DcmXTDataSetMain::Set_ValueToNull(unsigned long  tag )
{
	return DcmItemBase::Set_ValueToNullIn(tag );
 
}
	///
////
//##35 2012/09/16 K.Ko
bool DcmXTDataSetMain::Begin_Sequence(unsigned long  tag )
{
	return DcmItemBase::Begin_SequenceIn(tag );
}
 
bool DcmXTDataSetMain::End_Sequence(unsigned long  tag )
{

	return DcmItemBase::End_SequenceIn(tag );
}
////
//
//
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	unsigned short &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	short &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
//
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	unsigned int &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	int &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
//
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	unsigned long &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	long &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
//
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	float &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag,	double &val)
{
	return DcmItemBase::Get_ValueIn( tag,	 val);
}
///////
MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag, dcm_string &val,bool Sequence)
{
	 
	MC_STATUS status = DcmItemBase::Get_ValueIn( tag,	val, Sequence);
	 
	return status;
}

MC_STATUS DcmXTDataSetMain::Get_Value(unsigned long  tag, char *str_buff, int buff_len,bool Sequence)
{
	// change string to char * 2012/02/16 K.Ko
	std::string val_temp;
	MC_STATUS status = DcmItemBase::Get_ValueIn( tag,	val_temp, Sequence);
	if(status == MC_NORMAL_COMPLETION){
		strncpy(str_buff,val_temp.c_str(),buff_len);
	};
	return status;

//	return DcmItemBase::Get_ValueIn( tag,	 str_buff,  buff_len, Sequence);
}
MC_STATUS DcmXTDataSetMain::Get_ValueCount(unsigned long  tag,	int &val)
{
	return DcmItemBase::Get_ValueCountIn( tag,	 val);
}
MC_STATUS DcmXTDataSetMain::Get_ValueNext(unsigned long  tag,	dcm_string &val)
{
	return DcmItemBase::Get_ValueNextIn( tag,	val);
}

MC_STATUS DcmXTDataSetMain::Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len)
{
	dcm_string str_temp;
	MC_STATUS status = DcmItemBase::Get_ValueNextIn( tag,	str_temp);
	if(status == MC_NORMAL_COMPLETION){
		strncpy(str_buff,str_temp.c_str(),buff_len);
	};
	return status;
}

MC_STATUS DcmXTDataSetMain::Get_ValueLength(unsigned long  tag, int num,	unsigned long &val)
{
	return DcmItemBase::Get_ValueLengthIn( tag,  num, val);
}
 
MC_STATUS DcmXTDataSetMain::Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues)
{
	return DcmItemBase::Get_AttributeInfoIn( Atag, Avr, Avalues);
}
DcmItem* DcmItemBase::getFirstSeqDataSetIn(unsigned long  tag) 
{
	if (!m_SeqStack) {
		//Notice: initial timing
		m_SeqStack = new std::stack<DcmItem*>;//##35 2012/09/16 K.Ko
	}
	//MySeqStack->empty();
	//2026/01/02
	while (!MySeqStack->empty()) {
		MySeqStack->pop();
	}
	auto stack_size = MySeqStack->size();
	DcmItem* pFirstItem = nullptr;
	unsigned long dcm_dir_tag = tag;
	DcmTag dcm_dir_tag_key = GenDcmTagKey(dcm_dir_tag);
	int top_num = m_DcmObject->getNumberOfValues();
	for (int i = 0; i < top_num; i++) {
		DcmElement* ele = m_DcmObject->getElement(i);
		auto ele_tag = ele->getTag();
		if (ele_tag == dcm_dir_tag_key) {
			DcmSequenceOfItems* seq_item = (DcmSequenceOfItems*)ele;
			if (seq_item == nullptr) return nullptr;
			int seq_num = seq_item->getNumberOfValues();
			for (int ii = (seq_num - 1); ii >=0 ; ii--) {
				DcmItem* sub_item = seq_item->getItem(ii);
				auto sub_tag = sub_item->getTag();
				MySeqStack->push(sub_item);
			}
			stack_size = MySeqStack->size();
		}
	}
	if(MySeqStack->size()>0) {
		pFirstItem = MySeqStack->top();
		MySeqStack->pop();
	}
	stack_size = MySeqStack->size();
	return pFirstItem;
}
DcmItem* DcmItemBase::getNextSeqDataSetIn() const
{
	if (m_SeqStack == nullptr) {
		return nullptr;
	}
	DcmItem* pItem = nullptr;
	if (MySeqStack->size() > 0) {
		pItem = MySeqStack->top();
		MySeqStack->pop();
	}
	auto stack_size = MySeqStack->size();
	return pItem;
}
DcmXTDataSet* DcmXTDataSetMain::getFirstSeqDataSet(unsigned long  tag) 
{
	DcmXTDataSet* retDatst = nullptr;
	DcmItem* item = DcmItemBase::getFirstSeqDataSetIn(tag);
	if (item != nullptr) {
		bool bAttach = true;
		retDatst = new DcmXTDataSetMain((DcmDataset*)item, bAttach);
	}
	return retDatst;
}
DcmXTDataSet* DcmXTDataSetMain::getNextSeqDataSet() const
{
	DcmXTDataSet* retDatst = nullptr;
	DcmItem* item = DcmItemBase::getNextSeqDataSetIn();
	if (item != nullptr) {
		bool bAttach = true;
		retDatst = new DcmXTDataSetMain((DcmDataset*)item, bAttach);
	 
	}
	return retDatst;
	 
}
/////
