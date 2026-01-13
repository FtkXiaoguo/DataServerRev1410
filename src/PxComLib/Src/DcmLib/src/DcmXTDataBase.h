// DcmXTDataBase
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_IDICOM_DATA_BASE_H_)
#define AFX_IDICOM_DATA_BASE_H_
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#include "map"

#include "IDcmLib.h"
#include "DcmLocalString.h" 

using namespace XTDcmLib;


class DcmItem;
class DcmXTMetaHeaderMain;
class DcmXTDataSetMain;

class DcmElement;
class DcmTagKey;
class XDcmFileFormat;
class DcmXTDataBase  
{
public:
	DcmXTDataBase();
	~DcmXTDataBase();
	virtual void Delete() ;
	
	bool createDataset();
	bool createMetaHeader();

	virtual bool readFromDumpFile(const dcm_string & dumpFileName);
	virtual bool writeToDumpFile(const dcm_string & dumpFileName);

	virtual DcmXTMetaHeader *getDcmXTMetaHeader();
	virtual DcmXTDataSet *getDcmXTDataSet();
//	virtual DcmComInterface *getComInterface();
	//  
	bool Set_ValueToNullBase(unsigned long  tag, bool insertToHeader=false);

	//##35 2012/09/16 K.Ko
	bool Begin_SequenceBase(unsigned long  tag , bool insertToHeader=false) ;
	bool End_SequenceBase(unsigned long  tag ) ;

	template<class T>  bool Set_ValueBase(unsigned long  tag,	T val, bool insertToHeader=false) 
	{
	 

//	UPDATE_DCM_VALUE(tag,val);
		if(searchTagKeyHeader(tag) != 0) 
		{ 
			m_DcmMetaHeader->Set_Value( tag,val);
			return true; 
		} 
		if(searchTagKeyDataset(tag) != 0)
		{ 
			m_DcmDataset->Set_Value( tag,val) ;   
			return true; 
		} 

	/////
//	SET_DCM_VALUE(tag, val, insertToHeader );
		if(insertToHeader){ 
			if(!m_DcmMetaHeader) return false;
			return m_DcmMetaHeader->Set_Value( tag,	val); 
		}else{  
			return m_DcmDataset->Set_Value( tag,	val);  
		}  

    }

	

	//
	template<class T>  MC_STATUS Get_ValueBase(unsigned long  tag,	T &val ) 
	{
//	GET_DCM_VALUE(tag,val);
		if(m_DcmMetaHeader){	
			MC_STATUS status = m_DcmMetaHeader->Get_Value( tag,	val)  ;	
			if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
				return status;
			} 	 
		} 
		if(m_DcmDataset){		
			return m_DcmDataset->Get_Value( tag,	val) ;
		} 

		return MC_INVALID_TAG;
	}
	//
	//
	// change string to char * 2012/02/16 K.Ko
	 MC_STATUS Get_ValueString(unsigned long  tag,	dcm_string &val,bool Sequence=false ) ;
	 MC_STATUS Get_ValueString(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence=false ) ;

	 bool Set_ValueString(unsigned long  tag,	const dcm_string &val, bool insertToHeader=false,const char *pcreator=0) ;
	
	 bool Set_ArrayValue(unsigned long  tag,	const void*dataBuff,int dataSiz, bool insertToHeader=false,const char *pcreator=0) ;
	  
	//
	
	//
	MC_STATUS Get_ValueCount(unsigned long  tag,	int &val);
	// change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_ValueNext(unsigned long  tag,	dcm_string &val);
	MC_STATUS Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len);
	
	MC_STATUS Get_ValueLength(unsigned long  tag, int num,	unsigned long &val);
	//
	virtual MC_STATUS Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues);
protected:
	
	dcm_string m_fileName;
	virtual void destroy();
	DcmElement *searchTagKey(unsigned long  tag ,const char *pcreator=0) const ;
	DcmElement *searchTagKeyHeader(unsigned long  tag,const char *pcreator=0 ) const ;
	DcmElement *searchTagKeyDataset(unsigned long  tag ,const char *pcreator=0) const ;
	DcmItem *getInsertDest(bool insertToHeader) const;
	//
	 
	 


	DcmXTMetaHeaderMain *m_DcmMetaHeader;
	DcmXTDataSetMain *m_DcmDataset;
	XDcmFileFormat	*m_DcmFile;

	
};
 
#endif // !defined(AFX_IDICOM_DATA_BASE_H_)
