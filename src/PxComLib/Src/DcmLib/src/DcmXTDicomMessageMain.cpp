//  
//
//////////////////////////////////////////////////////////////////////



#pragma warning (disable: 4244)

#include "DcmXTDicomMessageMain.h"

#include "DcmXTDataSetMain.h"
#include "DcmXTUtilMain.h"
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
#include "XDcmTk/XDcdatset.h"

#include "CheckMemoryLeak.h"

OFBool
readDumpFile(DcmMetaInfo * metaheader, DcmDataset * dataset,
         FILE * infile, const char * ifname, const OFBool stopOnErrors,
         const unsigned long maxLineLength);

const unsigned int DCM_DumpMaxLineSize = 4096;


 DcmXTDicomMessageMain::DcmXTDicomMessageMain()
{

	 m_maxReadLength = 4096;//2012/03/23 

	m_fileXfer = EXS_Unknown;
	m_DcmMetaHeader = 0;
	m_DcmDataset = 0;

	m_MC_command = INVALID_COMMAND;
	

	m_PixelDataCallback = 0;

	m_WriteFile_PixelDataOffset = 0;
	m_WriteFile_FileLength = 0;
	 
}
DcmXTDicomMessageMain::~DcmXTDicomMessageMain()
{
	 
}
void DcmXTDicomMessageMain::Delete()
{
	destroy();
	delete this;
}
void DcmXTDicomMessageMain::destroy()
{
	
	DcmXTDataBase::destroy();
	
	m_WriteFile_PixelDataOffset = 0;
	m_WriteFile_FileLength = 0;
	
}
void DcmXTDicomMessageMain::makeFromDataset(DcmXTDataSetMain *dataset)
{
	m_DcmMetaHeader = 0;
	m_DcmDataset = dataset;
}
bool DcmXTDicomMessageMain::openMessage(const char * serviceName,	MC_COMMAND command)
{
	m_fileXfer = EXS_Unknown;
	m_serviceName = serviceName;
	m_MC_command = command;
 
	bool ret_b = createDataset();
	if(!ret_b) return false;
	ret_b = createMetaHeader();
	return ret_b;
 
}
bool DcmXTDicomMessageMain::open()
{
	m_fileXfer = EXS_Unknown;
	bool ret_b = createDataset();
	if(!ret_b) return false;
	ret_b = createMetaHeader();
	return ret_b;;
}
bool DcmXTDicomMessageMain::close()
{
	m_fileXfer = EXS_Unknown;
	return true;
}

bool DcmXTDicomMessageMain::readFile(const char * fileName)
{
	destroy();
	;
 
	E_FileReadMode readMode = ERM_autoDetect;
	OFCmdUnsignedInt maxReadLength = m_maxReadLength;//4096; // default is 4 KB
	E_TransferSyntax xfer = EXS_Unknown;

#if 1
	m_DcmFile = new XDcmFileFormat;
	
 
	DcmFileFormat *fileformat=m_DcmFile;
#else
	DcmFileFormat fileformat_temp;

	DcmFileFormat *fileformat=0;
	if(m_DcmFile){
		fileformat = m_DcmFile;
	}else{
		fileformat = &fileformat_temp;
	}
#endif
	DcmObject *dset = fileformat->getDataset();

	DcmMetaInfo * metaheader = fileformat->getMetaInfo();

//	SetDebugLevel(5);

	const char *ifname = fileName;//stringTochar(fileName);
	
//    if (readMode == ERM_dataset) dset = dfile.getDataset();
    OFCondition cond = fileformat->loadFile(ifname, xfer, EGL_noChange, maxReadLength, readMode);
    if (! cond.good())
    {
         return false;
    }
//	DIMSE_debug(1);

//	dset->loadAllDataIntoMemory();

//	dset->print(COUT);

//	COUT << ">>>>>====" <<endl;
//	dset->print(COUT);
//

#if 1
	createDataset();
	createMetaHeader();
#else
	if(!m_DcmFile){
		if(m_DcmDataset)  m_DcmDataset->Delete();
		m_DcmDataset = new DcmXTDataSetMain((DcmDataset*)dset);

		if(m_DcmMetaHeader) m_DcmMetaHeader->Delete();
		m_DcmMetaHeader = new DcmXTMetaHeaderMain(metaheader);
	}
#endif

	//proccess pixeldata
	unsigned long PixelTag = 0x7FE00010;
	if(m_PixelDataCallback){
		DcmElement *pixeldata_ele = m_DcmDataset->searchTagKey(PixelTag,
																false  /*searchIntoSub*/   //#24 2012/06/07 K.Ko
																);
		if(pixeldata_ele){
			cond = pixeldata_ele->loadAllDataIntoMemory();
			if (cond.bad())
			{
				return false;
			}

			unsigned long pixel_length = pixeldata_ele->getLength();
			if(pixel_length>0){
				m_PixelDataCallback->readPixelData(m_MC_MsgID,DcmXTMessageCallback::CB_DataLength,&pixel_length,0,0,0);

				unsigned char *_read_buff =0;
				OFCondition cond = pixeldata_ele->getUint8Array(_read_buff);

				if (cond.good())
				{
					int isFirst=1;
					int isLastPtr=1;
					m_PixelDataCallback->readPixelData(m_MC_MsgID,DcmXTMessageCallback::CB_Data,&pixel_length,(void**)(&_read_buff),isFirst,&isLastPtr);

				}
				 
			}
		//	m_PixelDataCallback-
		}
		 
//		int row_size = dset->
	}
	return true;
};

bool DcmXTDicomMessageMain::readFromDumpFile(const char * dumpFileName)
{
	 
	return DcmXTDataBase::readFromDumpFile(dumpFileName);
}

bool DcmXTDicomMessageMain::writeToDumpFile(const char * dumpFileName)
{
	 
 
	return DcmXTDataBase::writeToDumpFile(dumpFileName);
}

DcmXTDataSet *DcmXTDicomMessageMain::getDcmXTDataSet() const
{
	DcmXTDataSet *ret_dataset=m_DcmDataset;

	 

	return ret_dataset;
}
DcmXTMetaHeader *DcmXTDicomMessageMain::getDcmXTMetaHeader() const
{
	DcmXTMetaHeader *ret_dcmMeta=m_DcmMetaHeader;


	return ret_dcmMeta;
}
 
#if 0
DcmComInterface *DcmXTDicomMessageMain::getComInterface()
{
	return this;
}
#endif
	
#if 0
#define return MsgSetValue(tag,	val) { \
	bool insertToHeader = ((tag&(0xffff0000)) == (0x00020000));\
	return  DcmXTDataBase::Set_ValueBase( tag,	val,insertToHeader); \
}
#endif



#define MsgSetValueNull(tag ) { \
	bool insertToHeader = ((tag&(0xffff0000)) == (0x00020000));\
	return  DcmXTDataBase::Set_ValueToNullBase( tag,insertToHeader); \
}
//
bool DcmXTDicomMessageMain::DeleteTag(unsigned long  tag)
{
	bool fromHeader = ((tag&(0xffff0000)) == (0x00020000)); 

	bool ret_b;
	if(fromHeader){
		ret_b = m_DcmMetaHeader->DeleteTag( tag );
	}else{
		ret_b = m_DcmDataset->DeleteTag( tag );
	}

	return  ret_b;

}
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	int val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
	 
}
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	unsigned int val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
//
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	short val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	unsigned short val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
//
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	long  val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	unsigned long val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
//
//
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	float  val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
//

bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	double val,bool append,const char *privateTa)
{
	return MsgSetValue(tag,	val);
}
//
	// change string to char * 2012/02/16 K.Ko
bool DcmXTDicomMessageMain::Set_Value(unsigned long  tag,	const char *str_val,bool append ,const char *privateTa  )
{
 
 
	if(append){
		dcm_string new_val = str_val;
	 	dcm_string old_val;
		 
	 	if(Get_Value(tag,old_val,true/*allprocess*/)){
		 	if(old_val.size()>0){
		 		new_val = dcm_string(old_val.c_str())+"\\"+dcm_string(str_val);
			}
		}
//		return MsgSetValue(tag,	new_val  );
		return DcmXTDataBase::Set_ValueString(tag,	new_val,false,privateTa  );
	}else{
//		return MsgSetValue(tag,	val  );
		bool insertToHeader = ((tag&(0xffff0000)) == (0x00020000));
		return DcmXTDataBase::Set_ValueString(tag,	str_val,insertToHeader,privateTa  );
	}
 
}

///
bool DcmXTDicomMessageMain::Set_ArrayValue(unsigned long  tag,	const void*dataBuff,int dataSize)
{
	bool insertToHeader = ((tag&(0xffff0000)) == (0x00020000));
	return DcmXTDataBase::Set_ArrayValue(tag,	 dataBuff, dataSize,insertToHeader);
#if 0
	if(!m_DcmDataset) {
		return false;
	}
	
	return m_DcmDataset->Set_ArrayValue( tag,dataBuff,dataSize) ;   
#endif
	 
}
bool DcmXTDicomMessageMain::Set_ValueToNull(unsigned long  tag )
{
	MsgSetValueNull(tag);  
}
 
//##35 2012/09/16 K.Ko
bool DcmXTDicomMessageMain::Begin_Sequence(unsigned long  tag )
{
	return DcmXTDataBase::Begin_SequenceBase( tag );
}
bool DcmXTDicomMessageMain::End_Sequence(unsigned long  tag )
{
	return DcmXTDataBase::End_SequenceBase( tag );
}
//
////
//
//
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	unsigned short &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	short &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
//
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	unsigned int &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	int &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
//
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	unsigned long &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	long &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
//
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	float &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	double &val)
{
	return DcmXTDataBase::Get_ValueBase( tag,	val);
}
///////

// change string to char * 2012/02/16 K.Ko
//use it inside
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	dcm_string &val,bool Sequence )
{
	 
	return DcmXTDataBase::Get_ValueString( tag,	val ,Sequence);
 
}
 
MC_STATUS DcmXTDicomMessageMain::Get_Value(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence)
{
	return DcmXTDataBase::Get_ValueString( tag,	str_buff,  buff_len ,Sequence);
}

 

MC_STATUS DcmXTDicomMessageMain::Get_ValueCount(unsigned long  tag,	int &val)
{
	 
	return DcmXTDataBase::Get_ValueCount( tag,	val);
 
}
// change string to char * 2012/02/16 K.Ko
//use it inside
MC_STATUS DcmXTDicomMessageMain::Get_ValueNext(unsigned long  tag,	dcm_string &val)
{
	 
	return DcmXTDataBase::Get_ValueNext( tag,	val);
 
}
 
MC_STATUS DcmXTDicomMessageMain::Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len)
{
	 
	return DcmXTDataBase::Get_ValueNext( tag,	str_buff, buff_len);
 
}
 

MC_STATUS DcmXTDicomMessageMain::Get_ValueLength(unsigned long  tag, int num,	unsigned long &val)
{
	 
	return DcmXTDataBase::Get_ValueLength( tag,  num, val);
 
}
MC_STATUS DcmXTDicomMessageMain::Get_PixelOffset(unsigned long &val)
{
	val = m_WriteFile_PixelDataOffset;
	return MC_NORMAL_COMPLETION;
}
MC_STATUS DcmXTDicomMessageMain::Get_FileLength(unsigned long &val)
{ 
	val = m_WriteFile_FileLength;
	return MC_NORMAL_COMPLETION;
};
////
MC_STATUS DcmXTDicomMessageMain::Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues)
{
	return DcmXTDataBase::Get_AttributeInfo( Atag, Avr,Avalues);
}

 template <class T> class ClsAutoDelete
 {
 public :
	 ClsAutoDelete(T *clsInstPtr) {
		 m_instancePtr = clsInstPtr;
	 }
	 ClsAutoDelete() {
		 m_instancePtr = 0;
	 }
	 void setupClsInstance(T *clsInstPtr) {
		 m_instancePtr = clsInstPtr;
	 }
	 ~ClsAutoDelete() {
		 if(m_instancePtr){
			 delete m_instancePtr;
		 } 
	 }
 protected:
	 T *m_instancePtr;
 };
bool DcmXTDicomMessageMain::writeFile(const char * fileName)
{
	if(!m_DcmDataset) return false;

	//changeMetalInfo();

	 
#if 0
	
 	XDcmFileFormat dfileTemp(m_DcmDataset->getDcmDataPtr());

	XDcmFileFormat *write_dfile;
	
	if(m_DcmFile){
 		write_dfile = m_DcmFile;
	}else{
 		write_dfile = &dfileTemp;
		 
	}
#else
	ClsAutoDelete<XDcmFileFormat>  cls_deleter ;
	
//	XDcmFileFormat dfileTemp(m_DcmDataset->getDcmDataPtr());

	XDcmFileFormat *write_dfile;
	
	if(m_DcmFile){
 		write_dfile = m_DcmFile;
	}else{
//		write_dfile = &dfileTemp;
		XDcmFileFormat *dfileTemp = new XDcmFileFormat(m_DcmDataset->getDcmDataPtr());
		cls_deleter.setupClsInstance(dfileTemp);
		write_dfile = dfileTemp;
	}
#endif
 
	E_TransferSyntax writeXfer = EXS_Unknown;

#if 1
	DcmXT_TransferSyntax Xfer_temp;
	Get_TransferSyntax(Xfer_temp);
	writeXfer = DcmXTUtilMain::DcmXT2E_TransferSyntax(Xfer_temp);

#else
	if(m_DcmMetaHeader){
		DcmMetaInfo *MetaObj = (DcmMetaInfo*)(m_DcmMetaHeader->getDcmDataPtr());

	 
		if(MetaObj){
			 
			DcmMetaInfo *old_file_metainfo = write_dfile->getMetaInfo();

		 
			writeXfer = MetaObj->getOriginalXfer(); 
			if(m_fileXfer != EXS_Unknown){
				writeXfer = m_fileXfer;
			}else{
				dcm_string str_temp;
				//try to take TransferSyntaxUID from meteInformatioin header
				if(m_DcmMetaHeader->Get_Value(0x00020010,str_temp)){
					writeXfer = (E_TransferSyntax)DcmXfer(stringTochar(str_temp)).getXfer();
				}
			}
		}
	}
#endif
//	OFCondition cond = write_dfile->saveFile(fileName,writeXfer);
	//#18 2012/05/17 K.Ko use memory stream
	int write_buffer_size = DcmXTUtilMain::getWriteBufferLen();
	OFCondition cond = write_dfile->saveFileWithBuffer(write_buffer_size,fileName,writeXfer);

 	XDcmDataset *xdcm_dataset = (XDcmDataset *)(write_dfile->getDataset());
//	XDcmDataset *xdcm_dataset = dynamic_cast<XDcmDataset *>(write_dfile->getDataset());
	if(xdcm_dataset){
		m_WriteFile_PixelDataOffset = xdcm_dataset->getPixelDataOffset();
	}
	m_WriteFile_FileLength = write_dfile->getFileLength();

	 
	return !cond.bad();
}

bool DcmXTDicomMessageMain::writeFile()
{
	if(m_fileName.size()<1) return false;

	//
//	this->writeToDumpFile("dcm_dump.txt");
	//
	return writeFile(m_fileName.c_str());
}
void DcmXTDicomMessageMain::setFileName(const char * fileName){
	m_fileName =fileName ;
}

bool DcmXTDicomMessageMain::setupCommand(const char * serviceName,	MC_COMMAND command )
{

	m_serviceName = serviceName;
	m_MC_command = command;
	return true;
}

DcmXTDicomMessage *DcmXTDicomMessageMain::clone()
{
	DcmXTDicomMessageMain *new_msg = new DcmXTDicomMessageMain;
	 
		
	if(new_msg->m_DcmFile){
		delete new_msg->m_DcmFile;
	} 
 
	*new_msg = *this;
	//
	if(new_msg->m_DcmFile){
		new_msg->m_DcmFile = new XDcmFileFormat(*m_DcmFile);
#if 0 // bug ? 2010/07/16
		m_DcmMetaHeader = new DcmXTMetaHeaderMain(m_DcmFile->getMetaInfo(),true/*attach*/);
		m_DcmDataset	= new DcmXTDataSetMain(m_DcmFile->getDataset(),true/*attach*/);
#else
		new_msg->m_DcmMetaHeader = new DcmXTMetaHeaderMain(new_msg->m_DcmFile->getMetaInfo(),true/*attach*/);
		new_msg->m_DcmDataset	= new DcmXTDataSetMain(new_msg->m_DcmFile->getDataset(),true/*attach*/);
#endif
	}else{
		if(m_DcmMetaHeader){
			new_msg->m_DcmMetaHeader = new DcmXTMetaHeaderMain(m_DcmMetaHeader->getDcmDataPtr());
		 
		}

		if(m_DcmDataset){
			new_msg->m_DcmDataset = new DcmXTDataSetMain(m_DcmDataset->getDcmDataPtr());
		}
	}
	//
//	writeToDumpFile("org_msg.txt");
 //	new_msg->writeToDumpFile("new_msg.txt");

	return new_msg;
}
bool DcmXTDicomMessageMain::getMessageIDFromRsp(unsigned int &msgID)
{
	msgID = m_rspmessageID;
 
	return true;
}
bool DcmXTDicomMessageMain::getStatusFromRsp(unsigned int &status)
{
	status = m_rspStatus;
	return true;
}

bool DcmXTDicomMessageMain::Set_TransferSyntax(DcmXT_TransferSyntax Xfer)
{
//just for dicom file
	m_fileXfer =DcmXTUtilMain::DcmXT2E_TransferSyntax(Xfer);
	 
	
	return true;
}
bool DcmXTDicomMessageMain::Get_TransferSyntax(DcmXT_TransferSyntax &Xfer)
{
	 
	E_TransferSyntax orgXfer = m_fileXfer;
	if(m_fileXfer == EXS_Unknown){
		if(m_DcmMetaHeader){
			DcmMetaInfo *MetaObj = (DcmMetaInfo*)(m_DcmMetaHeader->getDcmDataPtr());
			if(MetaObj){
			//	dllString str_temp;
				char str_buff[2048]; // change string to char * 2012/02/16 K.Ko
				str_buff[0] = 0;
				orgXfer = MetaObj->getOriginalXfer(); 
				//try to take TransferSyntaxUID from meteInformatioin header
				//if(m_DcmMetaHeader->Get_Value(0x00020010,str_temp)){
				//	orgXfer = (E_TransferSyntax)DcmXfer(stringTochar(str_temp)).getXfer();
				if(m_DcmMetaHeader->Get_Value(0x00020010,str_buff,2048)){
				 
					orgXfer = (E_TransferSyntax)DcmXfer(str_buff).getXfer();
				}
			}
		}
	}
	Xfer = DcmXTUtilMain::E2DcmXT_TransferSyntax(orgXfer);
	return true;
}


bool DcmXTDicomMessageMain::Read_OBData( DcmXTMessageCallback *callback,unsigned long tag )
{
	OFCondition cond = ECC_Normal;
//proccess pixeldata
	if(!m_DcmDataset) return false;
	unsigned long PixelTag = tag;//0x7FE00010;
	if(callback){
		DcmElement *pixeldata_ele = m_DcmDataset->searchTagKey(PixelTag);

		if(pixeldata_ele){
			cond = pixeldata_ele->loadAllDataIntoMemory();
			if (cond.bad())
			{
				return false;
			}
			unsigned long pixel_length = pixeldata_ele->getLength();
			if(pixel_length>0){
			//	callback->readPixelData(m_MC_MsgID,DcmXTMessageCallback::CB_DataLength,&pixel_length,0,0,0);

				unsigned char *_read_buff =0;
				OFCondition cond = pixeldata_ele->getUint8Array(_read_buff);

				if (cond.good())
				{
					int isFirst=1;
					int isLastPtr=1;
					callback->readPixelData(m_MC_MsgID,DcmXTMessageCallback::CB_Data,&pixel_length,(void**)(&_read_buff),isFirst,&isLastPtr);

				}
				 
			}
		//	m_PixelDataCallback-
		}else{
			return false;
		}
		 
//		int row_size = dset->
	}
	return true;
};

bool DcmXTDicomMessageMain::Set_ServiceName(const  char * AserviceName, MC_COMMAND serviceCmd)
{
	this->setupCommand(AserviceName,  serviceCmd);
	 
	return true;
}

//bool DcmXTDicomMessageMain::Get_ServiceName(dllString  &AserviceUID, MC_COMMAND &serviceCmd) const
bool DcmXTDicomMessageMain::Get_ServiceName(char *AserviceUID_Buff,int bufLen, MC_COMMAND &serviceCmd)  const 
{
//	AserviceUID = this->m_serviceName.c_str();
	strncpy(AserviceUID_Buff,m_serviceName.c_str(),bufLen);
	serviceCmd	= m_MC_command;
	return true;
}
 
bool DcmXTDicomMessageMain::Add_PrivateBlock(unsigned short g,const char*    AprivateCode)
{
	unsigned short e = 0x0010;
	if(!DcmXTUtilMain::AddPrivateCreator2GDict( g, AprivateCode,e)){
		return false;
	}

	return DcmXTDicomMessageMain::Set_Value((g<<16)+e,AprivateCode,false/*append*/ ,AprivateCode );

}
bool DcmXTDicomMessageMain::Add_PrivateTag(unsigned short g,unsigned short e, MC_VR  Avr,const char*    AprivateCode)
{
	return  DcmXTUtilMain::AddPrivateTag2GDict( g, e,Avr,AprivateCode);
}

bool DcmXTDicomMessageMain::changeMetalInfo()
{
	
#if 0
	/*
	DIMSE_C_STORE_RQ ‚Ìê‡
	DataSet‚©‚çMetaInfo‚ÉˆÚ‚·.

#define DCM_FileMetaInformationGroupLength       DcmTagKey(0x0002, 0x0000)
#define DCM_FileMetaInformationVersion           DcmTagKey(0x0002, 0x0001)
#define DCM_MediaStorageSOPClassUID              DcmTagKey(0x0002, 0x0002)
#define DCM_MediaStorageSOPInstanceUID           DcmTagKey(0x0002, 0x0003)
#define DCM_TransferSyntaxUID                    DcmTagKey(0x0002, 0x0010)
#define DCM_ImplementationClassUID               DcmTagKey(0x0002, 0x0012)
#define DCM_ImplementationVersionName            DcmTagKey(0x0002, 0x0013)
#define DCM_SourceApplicationEntityTitle         DcmTagKey(0x0002, 0x0016)
#define DCM_PrivateInformationCreatorUID         DcmTagKey(0x0002, 0x0100)
#define DCM_PrivateInformation                   DcmTagKey(0x0002, 0x0102)
	
	*/

#endif
	DcmDataset *dataset_ptr = m_DcmDataset->getDcmDataPtr();

	if(dataset_ptr){
		removeItemFromeDataSet(dataset_ptr,DCM_FileMetaInformationGroupLength);
		removeItemFromeDataSet(dataset_ptr,DCM_FileMetaInformationVersion);
		removeItemFromeDataSet(dataset_ptr,DCM_MediaStorageSOPClassUID);
		removeItemFromeDataSet(dataset_ptr,DCM_MediaStorageSOPInstanceUID);
		//
		removeItemFromeDataSet(dataset_ptr,DCM_TransferSyntaxUID);
		removeItemFromeDataSet(dataset_ptr,DCM_ImplementationClassUID);
		removeItemFromeDataSet(dataset_ptr,DCM_ImplementationVersionName);
		removeItemFromeDataSet(dataset_ptr,DCM_SourceApplicationEntityTitle);
		removeItemFromeDataSet(dataset_ptr,DCM_PrivateInformationCreatorUID);
		removeItemFromeDataSet(dataset_ptr,DCM_PrivateInformation);
	}


	return true;
}