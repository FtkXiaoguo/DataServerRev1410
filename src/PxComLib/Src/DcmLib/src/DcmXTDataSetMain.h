// DcmDataSetMain
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_IDICOM_DATA_SET_MAIN_H_)
#define AFX_IDICOM_DATA_SET_MAIN_H_
 

#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#include "IDcmLib.h"

#include "DcmLocalString.h" 

#include <map>
using namespace XTDcmLib;

 
#include "dcmtk/ofstd/oftypes.h"
 
typedef std::map<unsigned long , int> MultiStringProcQ ;

#include "dcmtk/dcmdata/dcdicent.h"
typedef std::map<unsigned long, DcmDictEntry> PrivateTagMap;


class   DcmItem;
class   DcmElement;
class DcmItemBase
{
public:
	DcmItemBase();
	
	virtual void Delete() ;
	DcmItem *getDcmDataPtr() const { return m_DcmObject;};
	DcmElement *DcmItemBase::searchTagKey(unsigned long  tag,bool searchIntoSub=true ) const ; //#24 2012/06/07 K.Ko

	//void addPrivateTagDictEntry(Uint16 g, Uint16 e, DcmVR vr,const char* nam);
protected:
	 template<class T> bool Set_ValueIn(unsigned long  tag,	T val,const char *pcreator=0) 
	 {
		if(!m_DcmObject){
			return false;
		}

		OFCondition cond;

		DcmTag new_tag  = GenDcmTagKeyEx(tag,pcreator);

		DcmElement *new_elem=0;
		 
		 
		cond = newDicomElement(new_elem,new_tag);
		if(cond.bad()){
			return false;
		}

		if(!setupElementValue(new_elem,val)){
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
	 //
	

//	 bool Set_ValueStrIn(unsigned long  tag,	const dcm_string &val) ;
	 bool Set_ArrayValueIn(unsigned long  tag,const void*dataBuff,int dataSize) ;
	//
	 template<class T> MC_STATUS Get_ValueIn(unsigned long  tag,	T &val,bool Sequence=false)
	{
		bool ret_flag = false;
		MC_STATUS ret_status = MC_INVALID_TAG;
		
		if(!m_DcmObject){
			return ret_status;
		}
		OFCondition cond;

		DcmTag find_tag  = GenDcmTagKey(tag);
		 
		 
		T val_temp;

		DcmElement * target_element;
		cond = m_DcmObject->findAndGetElement(find_tag,target_element, OFTrue /*searchIntoSub*/);
		if(cond.good()){
	 
			//there is TAG
			ret_status = MC_NULL_VALUE;

 			ret_flag = getValue(target_element,val_temp);
			if(ret_flag){
				val = val_temp;
				
				resetProcQueuCurPostion(tag);
				 
			} 
		}else{
			ret_status = MC_INVALID_TAG;
		}
		if(ret_flag && (!Sequence)){
			val = procSequenceData(tag,val_temp);
			 
		} 

		if(ret_flag){
			ret_status = MC_NORMAL_COMPLETION;
		}
		return ret_status;
	};
	//
	 //
	bool DeleteTag(unsigned long  tag);

	bool Set_ValueToNullIn(unsigned long  tag );
	//##35 ##35 2012/09/16 K.Ko
	 bool Begin_SequenceIn(unsigned long  tag,const char *pcreator=0) ;
	 bool End_SequenceIn(unsigned long  tag);

	 //
	 MC_STATUS Get_ValueCountIn(unsigned long  tag,	int&val);
	 // change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_ValueNextIn(unsigned long  tag,	dcm_string &val); //use it inside
	// virtual MC_STATUS Get_ValueNextIn(unsigned long  tag,	char *str_buff, int buff_len); //for Val_1\Val_2\Val_3...

	 MC_STATUS Get_ValueLengthIn(unsigned long  tag, int num,	unsigned long &val) ;
	 //
	 MC_STATUS Get_AttributeInfoIn(unsigned long  Atag,MC_VR &Avr,int &Avalues);
///
	 int getMultiStringCount(unsigned long  tag,const dcm_string &str);
	 dcm_string getMultiStringStart(unsigned long  tag,const dcm_string &str);
	 dcm_string getMultiStringNext(unsigned long  tag,const dcm_string &str);	
	 //
	 short procSequenceData(unsigned long  tag,				const short input);
	 unsigned short procSequenceData(unsigned long  tag,	const unsigned short input);
	 int  procSequenceData(unsigned long  tag,				const int input);
	 unsigned int procSequenceData(unsigned long  tag,		const unsigned int input);
	 long procSequenceData(unsigned long  tag,				const long input);
	 unsigned long procSequenceData(unsigned long  tag ,	const unsigned long input);
	 float procSequenceData(unsigned long  tag,				const float input);
	 double procSequenceData(unsigned long  tag,			const double input);
	 //
	 dcm_string procSequenceData(unsigned long  tag,		const dcm_string input);
	 //
	// dcm_string procSequenceData(unsigned long  tag,			const dcm_string input);

	 //
	 void resetProcQueuCurPostion(unsigned long  tag){
		 m_MultiStringProcQueu[tag] = 0;
	 }
	 MultiStringProcQ m_MultiStringProcQueu;
///
	DcmItem *m_DcmObject;
	bool	m_delDcmObjectFlag;
	//
	PrivateTagMap m_privateTagMap;

	//##35 2012/09/16 K.Ko
	DcmItem *m_CurSequenceDcmObject;
	void *m_SeqStack;
};
class DcmXTMetaHeaderMain : public DcmItemBase,  public DcmXTMetaHeader
{
public:
	DcmXTMetaHeaderMain();
	DcmXTMetaHeaderMain(DcmItem *meta ,bool attach=false);
	
	virtual void Delete() ;
//	virtual bool readFile(const dcm_string & fileName) ;

	// DcmComInterface
	virtual bool DeleteTag(unsigned long  tag);

	virtual bool Set_Value(unsigned long  tag,	unsigned short val,bool append=false,const char *privateTag=0)		;
	virtual bool Set_Value(unsigned long  tag,	short val,bool append=false,const char *privateTag=0)				;
	//
	virtual bool Set_Value(unsigned long  tag,	unsigned int val,bool append=false,const char *privateTag=0)		;
	virtual bool Set_Value(unsigned long  tag,	int val,bool append=false,const char *privateTag=0)				;
	//
	virtual bool Set_Value(unsigned long  tag,	unsigned long val,bool append=false,const char *privateTag=0)		;
	virtual bool Set_Value(unsigned long  tag,	long val,bool append=false,const char *privateTag=0)				;
	//
	virtual bool Set_Value(unsigned long  tag,	float val,bool append=false,const char *privateTag=0)				;
	virtual bool Set_Value(unsigned long  tag,	double val,bool append=false,const char *privateTag=0)				;
	//
	// change string to char * 2012/02/16 K.Ko
	//virtual bool Set_Value(unsigned long  tag,	const dcm_string &val,bool append=false,const char *privateTag=0)	;
	virtual bool Set_Value(unsigned long  tag,	const char *str_val,bool append=false,const char *privateTag=0)	;
	
	//
	virtual bool Set_ArrayValue(unsigned long  tag,const void*dataBuff,int dataSize);
	//
	virtual bool Set_ValueToNull(unsigned long  tag ) ;
	///
	//##35 2012/09/16 K.Ko
	//##35 2012/09/16 K.Ko
	virtual bool Begin_Sequence(unsigned long  tag ) ;
	virtual bool End_Sequence(unsigned long  tag ) ;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned short &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	short &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned int &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	int &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned long &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	long &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	float &val)				;
	virtual MC_STATUS Get_Value(unsigned long  tag,	double &val)				;
	//
	// change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_Value(unsigned long  tag,	dcm_string &val,bool Sequence=false) ;//use it inside
	virtual MC_STATUS Get_Value(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence=false) ;
	virtual MC_STATUS Get_ValueCount(unsigned long  tag,	int&val);
	// change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_ValueNext(unsigned long  tag,	dcm_string &val);//use it inside
	virtual MC_STATUS Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len);
	virtual MC_STATUS Get_ValueLength(unsigned long  tag, int num,	unsigned long &val) ;
	//
	virtual MC_STATUS Get_PixelOffset(unsigned long &val){ return MC_ERROR;};
	virtual MC_STATUS Get_FileLength(unsigned long &val){ return MC_ERROR;}; 
	//
	virtual MC_STATUS Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues);
protected:

	virtual ~DcmXTMetaHeaderMain();
	
};

class DcmDataset;
class DcmXTDataSetMain : public DcmItemBase,  public  DcmXTDataSet
{
public:
	DcmXTDataSetMain();
	DcmXTDataSetMain(DcmDataset *data,bool attach=false);

	DcmDataset *getDcmDataPtr() const { return (DcmDataset*)m_DcmObject;};
	virtual void Delete() ;
//	virtual bool readFile(const dcm_string & fileName) ;

	// DcmComInterface
	virtual bool DeleteTag(unsigned long  tag);

	virtual bool Set_Value(unsigned long  tag,	unsigned short val,bool append=false,const char *privateTag=0)		;
	virtual bool Set_Value(unsigned long  tag,	short val,bool append=false,const char *privateTag=0)				;
	//
	virtual bool Set_Value(unsigned long  tag,	unsigned int val,bool append=false,const char *privateTag=0)		;
	virtual bool Set_Value(unsigned long  tag,	int val,bool append=false,const char *privateTag=0)				;
	//
	virtual bool Set_Value(unsigned long  tag,	unsigned long val,bool append=false,const char *privateTag=0)		;
	virtual bool Set_Value(unsigned long  tag,	long val,bool append=false,const char *privateTag=0)				;
	//
	virtual bool Set_Value(unsigned long  tag,	float val,bool append=false,const char *privateTag=0)				;
	virtual bool Set_Value(unsigned long  tag,	double val,bool append=false,const char *privateTag=0)				;
	//
	// change string to char * 2012/02/16 K.Ko
	//virtual bool Set_Value(unsigned long  tag,	const dcm_string &val,bool append=false,const char *privateTag=0)	;
	virtual bool Set_Value(unsigned long  tag,	const char *str_val,bool append=false,const char *privateTag=0)	;
	
	//
	virtual bool Set_ArrayValue(unsigned long  tag,const void*dataBuff,int dataSize) ;
	//
	virtual bool Set_ValueToNull(unsigned long  tag ) ;
	//##35 2012/09/16 K.Ko
	virtual bool Begin_Sequence(unsigned long  tag ) ;
	virtual bool End_Sequence(unsigned long  tag ) ;
 
	//
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned short &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	short &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned int &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	int &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned long &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	long &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	float &val)				;
	virtual MC_STATUS Get_Value(unsigned long  tag,	double &val)				;
	//

	// change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_Value(unsigned long  tag,	dcm_string &val,bool Sequence=false) ; //use it inside
	virtual MC_STATUS Get_Value(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence=false);

	virtual MC_STATUS Get_ValueCount(unsigned long  tag,	int&val);
	// change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_ValueNext(unsigned long  tag,	dcm_string &val);//use it inside
	virtual MC_STATUS Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len);
	virtual MC_STATUS Get_ValueLength(unsigned long  tag, int num,	unsigned long &val) ;

	//
	virtual MC_STATUS Get_PixelOffset(unsigned long &val){ return MC_ERROR;};
	virtual MC_STATUS Get_FileLength(unsigned long &val){ return MC_ERROR;};
	//
	virtual MC_STATUS Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues);

protected :
	 virtual ~DcmXTDataSetMain();
};
 
#endif // !defined(AFX_IDICOM_DATA_SET_MAIN_H_)
