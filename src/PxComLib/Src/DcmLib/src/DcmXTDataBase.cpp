//  
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable: 4244)
 

#include "DcmXTDataBase.h"

#include "DcmXTDataSetMain.h"

//////////////////
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"
//#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcistrmz.h"    /* for dcmZlibExpectRFC1950Encoding */

#include "dcmtk/dcmnet/dimse.h"

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTRING
#include "dcmtk/ofstd/ofstdinc.h"

#include "DcmTkBase.h"
#include "XDcmTk/xdcfilefo.h"

#include "CheckMemoryLeak.h"

#if 0
OFBool
readDumpFile(DcmMetaInfo * metaheader, DcmDataset * dataset,
         FILE * infile, const char * ifname, const OFBool stopOnErrors,
         const unsigned long maxLineLength);
#else
OFBool
readDumpFile(DcmMetaInfo *metaheader, DcmDataset *dataset,
         FILE *infile, const char *ifname, E_TransferSyntax &xfer,
         const OFBool stopOnErrors, const unsigned long maxLineLength);
#endif

const unsigned int DCM_DumpMaxLineSize = 4096;


DcmXTDataBase::DcmXTDataBase()
{

	m_DcmFile = new XDcmFileFormat;
	m_DcmMetaHeader = 0;
	m_DcmDataset = 0;

}
DcmXTDataBase::~DcmXTDataBase()
{
	 
}
void DcmXTDataBase::Delete()
{
	destroy();
	delete this;
}
void DcmXTDataBase::destroy()
{
	
 
	if(m_DcmMetaHeader){
		m_DcmMetaHeader->Delete();
		m_DcmMetaHeader = 0;
	}
	if(m_DcmDataset){
		m_DcmDataset->Delete();
		m_DcmDataset = 0;
	}
	
	if(m_DcmFile){
		delete m_DcmFile;
		m_DcmFile = 0;
		
	}
}
bool DcmXTDataBase::createDataset()
{
 
	if(m_DcmDataset){
		m_DcmDataset->Delete();
		m_DcmDataset = 0;
	}
	if(m_DcmFile){
		m_DcmDataset = new DcmXTDataSetMain(m_DcmFile->getDataset(),true/*attach*/);
	}else{
		m_DcmDataset = new DcmXTDataSetMain;
	}
	 
	return true;
}
bool DcmXTDataBase::createMetaHeader()
{
	if(m_DcmMetaHeader){
		m_DcmMetaHeader->Delete();
		m_DcmMetaHeader = 0;
	}
	if(m_DcmFile){
		m_DcmMetaHeader = new DcmXTMetaHeaderMain(m_DcmFile->getMetaInfo(),true/*attach*/);
	}else{
		m_DcmMetaHeader = new DcmXTMetaHeaderMain();
	}
	return true;
}


bool DcmXTDataBase::readFromDumpFile(const dcm_string & dumpFileName)
{
	destroy();

    DcmFileFormat fileformat_temp;

	DcmFileFormat *fileformat=0;
	if(m_DcmFile){
		fileformat = m_DcmFile;
	}else{
		fileformat = &fileformat_temp;
	}

    DcmMetaInfo * metaheader = fileformat->getMetaInfo();
    DcmDataset * dataset = fileformat->getDataset();

 //   SetDebugLevel((opt_debugMode));

	bool error_flag = false;
	 
	FILE * dumpfile = 0;
	do 
	{

		/* make sure data dictionary is loaded */
		if (!dcmDataDict.isDictionaryLoaded()) {
			error_flag = true;
			break;
		}

		const char *ifname = stringTochar(dumpFileName);
	  
		 dumpfile = fopen(ifname, "r");
		if (!dumpfile)
		{
			error_flag = true;
			break;
	        
		}

		OFBool stopOnErrors = OFTrue;
		OFCmdUnsignedInt opt_linelength = DCM_DumpMaxLineSize;

		E_TransferSyntax xfer;
		// read dump file into metaheader and dataset
		if (readDumpFile(metaheader, dataset, dumpfile, ifname, xfer,stopOnErrors,
			OFstatic_cast(unsigned long, opt_linelength)))
		{
			// write into file format or dataset
		//	error_flag = true;
		//	break;
		}
	}while(false) ;//just do it once

	if(dumpfile)
	{
		fclose(dumpfile);
	}
	if(error_flag){
		 return false;
	}else{
		
		if(!m_DcmFile){
		//	if(metaheader->error().good()){
				if(m_DcmMetaHeader) m_DcmMetaHeader->Delete();
				m_DcmMetaHeader = new DcmXTMetaHeaderMain(metaheader);
				 
		//	}
		 
		//	if(dataset->error().good()){
				if(m_DcmDataset) m_DcmDataset->Delete();
				m_DcmDataset = new DcmXTDataSetMain(dataset);
		//	}
		}
	//	m_OrginalXfer = new DcmXfer(dataset->getOriginalXfer());
	}
	return true;
}

bool DcmXTDataBase::writeToDumpFile(const dcm_string & dumpFileName)
{
	bool ret_flag = false;

	
	/* make sure data dictionary is loaded */
	if (!dcmDataDict.isDictionaryLoaded()) {
		return false;
	}

	const char *dump_fname = stringTochar(dumpFileName);
    
	STD_NAMESPACE ofstream out_file(dump_fname);
	 
	size_t printFlags = DCMTypes::PF_shortenLongTagValues /*| DCMTypes::PF_showTreeStructure*/;

	if( m_DcmMetaHeader){
		m_DcmMetaHeader->getDcmDataPtr()->print(out_file, printFlags);
		ret_flag = true;
	}

    
	 if(m_DcmDataset){
		 DcmDataset *dataset_ptr = m_DcmDataset->getDcmDataPtr();
		 if(dataset_ptr){
			dataset_ptr->print(out_file, printFlags);
		}
		ret_flag = true;
	}


	 out_file<< STD_NAMESPACE endl;
	 out_file.close();
 
	return true;
}

DcmXTDataSet *DcmXTDataBase::getDcmXTDataSet()
{
	DcmXTDataSet *ret_dataset=m_DcmDataset;

	 

	return ret_dataset;
}
DcmXTMetaHeader *DcmXTDataBase::getDcmXTMetaHeader()
{
	DcmXTMetaHeader *ret_dcmMeta=m_DcmMetaHeader;
	return ret_dcmMeta;
}
 
#if 0
DcmComInterface *DcmXTDataBase::getComInterface()
{
	return this;
}
#endif
	
// DcmComInterface
#define UPDATE_DCM_VALUE(tag, val) { \
	if(searchTagKeyHeader(tag) != 0) { m_DcmMetaHeader->Set_Value( tag,val); return true; }\
	if(searchTagKeyDataset(tag) != 0){ m_DcmDataset->Set_Value( tag,val) ;   return true; }\
}
#define SET_DCM_VALUE(tag, val, insertToHeader ) { \
	if(insertToHeader){ \
		return m_DcmMetaHeader->Set_Value( tag,	val); \
	}else{ \
		return m_DcmDataset->Set_Value( tag,	val); \
	} \
}

 

DcmItem *DcmXTDataBase::getInsertDest(bool insertToHeader) const
{
	DcmItem *item_dest=0;
	if(insertToHeader){
		if(!m_DcmMetaHeader) return false;
		item_dest = m_DcmMetaHeader->getDcmDataPtr();
	}else{
		if(!m_DcmDataset) return false;
		item_dest = m_DcmDataset->getDcmDataPtr();
	}
	 
	return item_dest;

}

//
MC_STATUS DcmXTDataBase::Get_ValueCount(unsigned long  tag,	int &val)
{
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_ValueCount( tag,	val)  ;
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}
	} 
	if(m_DcmDataset){		
		return m_DcmDataset->Get_ValueCount( tag,	val) ;		
			 
	} 

	return MC_INVALID_TAG;

}
// change string to char * 2012/02/16 K.Ko
 
MC_STATUS DcmXTDataBase::Get_ValueNext(unsigned long  tag,	dcm_string &val)
{
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_ValueNext( tag,	val) ;	
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}  
	} 
	if(m_DcmDataset){		
		return m_DcmDataset->Get_ValueNext( tag,	val) ;
	} 

	return MC_INVALID_TAG;

}
 
MC_STATUS DcmXTDataBase::Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len)
{
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_ValueNext( tag,	str_buff, buff_len) ;	
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}  
	} 
	if(m_DcmDataset){		
		return m_DcmDataset->Get_ValueNext( tag,	str_buff,  buff_len) ;
	} 

	return MC_INVALID_TAG;

}
 
MC_STATUS DcmXTDataBase::Get_ValueLength(unsigned long  tag, int num,	unsigned long &val)
{
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_ValueLength( tag,  num, val);
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}  ; 
	} 
	if(m_DcmDataset){		
		return m_DcmDataset->Get_ValueLength( tag,  num, val);
	} 

	return MC_INVALID_TAG;

}
MC_STATUS DcmXTDataBase::Get_ValueString(unsigned long  tag, dcm_string &val,bool Sequence ) 
{
//	GET_DCM_VALUE(tag,val);
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_Value( tag,	val,Sequence);	
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}  ;  
	}
		  
	if(m_DcmDataset){		
		return m_DcmDataset->Get_Value( tag,	val,Sequence) ;		
	} 

	return MC_INVALID_TAG;
}

MC_STATUS DcmXTDataBase::Get_ValueString(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence ) 
{
//	GET_DCM_VALUE(tag,val);
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_Value( tag,	str_buff,  buff_len,Sequence);	
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}  ;  
	}
		  
	if(m_DcmDataset){		
		return m_DcmDataset->Get_Value( tag,	str_buff,  buff_len,Sequence) ;		
	} 

	return MC_INVALID_TAG;
}
//
MC_STATUS DcmXTDataBase::Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues)
{
	if(m_DcmMetaHeader){	
		MC_STATUS status = m_DcmMetaHeader->Get_AttributeInfo( Atag, Avr, Avalues);	
		if( (status == MC_NULL_VALUE) || (status == MC_NORMAL_COMPLETION) ){
			return status;
		}  ;  
	}
		  
	if(m_DcmDataset){		
		return m_DcmDataset->Get_AttributeInfo( Atag, Avr, Avalues);		
	} 

	return MC_INVALID_TAG;
}

////

 
DcmElement *DcmXTDataBase::searchTagKey(unsigned long  tag,const char *pcreator ) const 
{
	DcmElement *ret_elem = 0;
	
	ret_elem = searchTagKeyHeader(tag,pcreator );
	if(ret_elem) return ret_elem;

	return searchTagKeyDataset(tag );
}

DcmElement *DcmXTDataBase::searchTagKeyHeader(unsigned long  tag,const char *pcreator ) const 
{
	DcmElement *ret_elem = 0;
	DcmTag tag_key  = GenDcmTagKeyEx(tag,pcreator);
	if(pcreator){
		tag_key.setPrivateCreator(pcreator);
		tag_key.lookupVRinDictionary();
	}
	 
	DcmStack resultStack;

	OFCondition cond;

	if(m_DcmMetaHeader){
		DcmItem *item = m_DcmMetaHeader->getDcmDataPtr();
		if(item){
			cond = item->search(tag_key,resultStack,ESM_fromStackTop);
			if(cond.good()){
				ret_elem = OFstatic_cast(DcmElement *, resultStack.top());
			}
		}
	}
	 
	return ret_elem;
}
	 
DcmElement *DcmXTDataBase::searchTagKeyDataset(unsigned long  tag,const char *pcreator) const 
{
	DcmElement *ret_elem = 0;
	DcmTag tag_key  = GenDcmTagKeyEx(tag,pcreator);
	 
	 
	DcmStack resultStack;

	OFCondition cond;

	resultStack.clear();

	if(m_DcmDataset){
		DcmItem *item = m_DcmDataset->getDcmDataPtr();
		if(item){
			cond = item->search(tag_key,resultStack,ESM_fromStackTop);
			if(cond.good()){
				ret_elem = OFstatic_cast(DcmElement *, resultStack.top());
			}
		}
	}
 
	return ret_elem;
	  
}
 
bool DcmXTDataBase::Set_ValueToNullBase(unsigned long  tag, bool insertToHeader )
{
	if(searchTagKeyHeader(tag) != 0) 
	{ 
		m_DcmMetaHeader->Set_ValueToNull( tag );
		return true; 
	} 
	if(searchTagKeyDataset(tag) != 0)
	{ 
		m_DcmDataset->Set_ValueToNull( tag ) ;   
		return true; 
	} 

	/////
 
	if(insertToHeader){ 
		if(!m_DcmMetaHeader) return false;
		return m_DcmMetaHeader->Set_ValueToNull( tag ); 
	}else{  
		return m_DcmDataset->Set_ValueToNull( tag );  
	}  

	return true;
}

//##35 2012/09/16 K.Ko
bool DcmXTDataBase::Begin_SequenceBase(unsigned long  tag, bool insertToHeader )
{
	
 
 	if(insertToHeader){ 
		if(!m_DcmMetaHeader) return false;
		return m_DcmMetaHeader->Begin_Sequence( tag); 
	}else{  
		return m_DcmDataset->Begin_Sequence( tag);  
	}  

 

}
bool DcmXTDataBase::End_SequenceBase(unsigned long  tag )
{
	if(searchTagKeyHeader(tag ) != 0) 
	{ 
		m_DcmMetaHeader->End_Sequence( tag);
		return true; 
	} 
	if(searchTagKeyDataset(tag) != 0)
	{ 
		m_DcmDataset->End_Sequence( tag) ;   
		return true; 
	} 
	return true;
}


bool DcmXTDataBase::Set_ValueString(unsigned long  tag,	const dcm_string &val, bool insertToHeader ,const char *pcreator )
{
		 
	 

//	UPDATE_DCM_VALUE(tag,val);
		if(searchTagKeyHeader(tag,pcreator) != 0) 
		{ 
			m_DcmMetaHeader->Set_Value( tag,val.c_str(),false/*apend*/,pcreator);
			return true; 
		} 
		if(searchTagKeyDataset(tag,pcreator) != 0)
		{ 
			m_DcmDataset->Set_Value( tag,val.c_str(),false/*apend*/,pcreator) ;   
			return true; 
		} 

	/////
//	SET_DCM_VALUE(tag, val, insertToHeader );

		if(insertToHeader){ 
			if(!m_DcmMetaHeader) return false;
			return m_DcmMetaHeader->Set_Value( tag,	val.c_str(),false/*apend*/,pcreator); 
		}else{  
			return m_DcmDataset->Set_Value( tag,	val.c_str(),false/*apend*/,pcreator);  
		}  
 
 
}
	
bool DcmXTDataBase::Set_ArrayValue(unsigned long  tag,	const void*dataBuff,int dataSize, bool insertToHeader,const char *pcreator)
{
		 
	 

//	UPDATE_DCM_VALUE(tag,val);
		if(searchTagKeyHeader(tag,pcreator) != 0) 
		{ 
			return m_DcmMetaHeader->Set_ArrayValue( tag,dataBuff,dataSize);
			 
		} 
		if(searchTagKeyDataset(tag,pcreator) != 0)
		{ 
			return m_DcmDataset->Set_ArrayValue( tag,dataBuff,dataSize) ;   
			 
		} 

	/////
//	SET_DCM_VALUE(tag, val, insertToHeader );
		if(insertToHeader){ 
			if(!m_DcmMetaHeader) return false;
			return m_DcmMetaHeader->Set_ArrayValue( tag,dataBuff,dataSize); 
		}else{  
			return m_DcmDataset->Set_ArrayValue( tag,dataBuff,dataSize) ;  
		}  
 
}

