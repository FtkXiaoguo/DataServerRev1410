// DcmXTDicomMessageMain
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_IDICOM_DICOMMESSAGE_MAIN_H_)
#define AFX_IDICOM_DICOMMESSAGE_MAIN_H_
 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#include "IDcmLib.h"

using namespace XTDcmLib;

#include "DcmXTDataBase.h"
 #include "dcmtk/dcmdata/dcxfer.h"
class DcmItem;
class DcmXTMetaHeaderMain;
class DcmXTDataSetMain;

 
class DcmXTDicomMessageMain : public DcmXTDicomMessage  , DcmXTDataBase
{
public:
	DcmXTDicomMessageMain();
	
	~DcmXTDicomMessageMain();
	void makeFromDataset(DcmXTDataSetMain *dataset);
	virtual void Delete() ;
	//
	virtual DcmXTDicomMessage *clone() ;
	// change string to char * 2012/02/16 K.Ko
	virtual bool openFile(const char * fileName){ m_fileName = fileName; return true;};
	virtual void setMaxReadLength( long maxLen){ m_maxReadLength = maxLen;};
	virtual bool readFile(){return readFile(m_fileName.c_str()) ;};
	virtual bool readFile(const char * fileName) ;
	virtual bool writeFile(const char * fileName) ;
	//
	virtual bool writeFile( ) ;
	virtual void setFileName(const char * fileName) ;

	virtual bool readFromDumpFile(const char * dumpFileName);
	virtual bool writeToDumpFile(const char * dumpFileName);
//
	virtual bool close() ;
	virtual bool openMessage(const char * serviceName,	MC_COMMAND command=INVALID_COMMAND)  ;
	virtual bool setupCommand(const char * serviceName,	MC_COMMAND command ) ;
	virtual bool open() ;
//
	virtual bool Set_ArrayValue(unsigned long  tag,	const void*dataBuff,int dataSize);
	//
	virtual bool Set_ValueToNull(unsigned long  tag ) ;
	//##35 2012/09/16 K.Ko
	virtual bool Begin_Sequence(unsigned long  tag ) ;
	virtual bool End_Sequence(unsigned long  tag ) ;
//
	virtual bool Set_Callback( DcmXTMessageCallback *callback){ m_PixelDataCallback = callback; return true;};
	virtual bool Read_OBData( DcmXTMessageCallback *callback,unsigned long tag=0x7FE00010);
//
	virtual bool Set_TransferSyntax(DcmXT_TransferSyntax Xfer) ;
	virtual bool Get_TransferSyntax(DcmXT_TransferSyntax &Xfer) ;
//
	virtual DcmXTMetaHeader *getDcmXTMetaHeader() const;
	virtual DcmXTDataSet *getDcmXTDataSet() const ;

	virtual bool Add_PrivateBlock(unsigned short ,const char*    AprivateCode) ; 
	virtual bool Add_PrivateTag(unsigned short g,unsigned short e, MC_VR Avr,const char*    AprivateCode);
//	virtual DcmComInterface *getComInterface();
	// DcmComInterface
	bool DeleteTag(unsigned long  tag);

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
	//
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned short &val)	;
	virtual MC_STATUS Get_Value(unsigned long  tag,	short &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned int &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	int &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	unsigned long &val)		;
	virtual MC_STATUS Get_Value(unsigned long  tag,	long &val)				;
	//
	virtual MC_STATUS Get_Value(unsigned long  tag,	float &val)				;
	virtual MC_STATUS Get_Value(unsigned long  tag,	double &val)			;
	//

	// change string to char * 2012/02/16 K.Ko
	MC_STATUS Get_Value(unsigned long  tag,	dcm_string &val,bool Sequence=false) ;//use it inside
	virtual MC_STATUS Get_Value(unsigned long  tag,	char *str_buff, int buff_len,bool Sequence=false);

	virtual MC_STATUS Get_ValueCount(unsigned long  tag,	int &val) ;
	// change string to char * 2012/02/16 K.Ko
	 MC_STATUS Get_ValueNext(unsigned long  tag,	dcm_string &val);//use it inside
	virtual MC_STATUS Get_ValueNext(unsigned long  tag,	char *str_buff, int buff_len); //for Val_1\Val_2\Val_3...

	//
	virtual MC_STATUS Get_ValueLength(unsigned long  tag, int num,	unsigned long &val) ;
	//
	virtual MC_STATUS Get_PixelOffset(unsigned long &val);
	virtual MC_STATUS Get_FileLength(unsigned long &val);
	//
	virtual MC_STATUS Get_AttributeInfo(unsigned long  Atag,MC_VR &Avr,int &Avalues);

	//
	virtual bool Set_ServiceName(const char *AserviceUID, MC_COMMAND  serviceCmd) ;
//	virtual bool Get_ServiceName(dllString  &AserviceUID, MC_COMMAND &serviceCmd) const ;
	virtual bool Get_ServiceName(char *AserviceUID_Buff,int bufLen, MC_COMMAND &serviceCm)  const  ;
//
	///
 	//
	
	//
	virtual bool setID(unsigned int id) {m_MC_MsgID = id; return true;}; 
	virtual bool getID(unsigned int &id) const {id = m_MC_MsgID ;return true;};

	//
	//special tag for response message
	virtual bool getMessageIDFromRsp(unsigned int &msgID) ;
	virtual bool getStatusFromRsp(unsigned int &status) ;

	void setResponseMsg(unsigned int m_msgID,unsigned int status){
			m_rspmessageID	= m_msgID;
			m_rspStatus		= status;
	};

	 
	//
	bool changeMetalInfo();
protected:
	template<class T> bool MsgSetValue(unsigned long tag,T val) {
		bool insertToHeader = ((tag&(0xffff0000)) == (0x00020000));
		return  DcmXTDataBase::Set_ValueBase( tag,	val,insertToHeader);
	}

	dcm_string m_fileName;
	void destroy();

	long m_maxReadLength ;

	dcm_string m_serviceName;
	int	 m_MC_command;
	//
	unsigned int m_MC_MsgID;
	//for response message

	unsigned int  m_rspmessageID;
	unsigned int	 m_rspStatus;
	//
	E_TransferSyntax m_fileXfer;
	//
	DcmXTMessageCallback *m_PixelDataCallback;
	//
	//
	unsigned long m_WriteFile_PixelDataOffset;
	unsigned long m_WriteFile_FileLength;
};
 
#endif // !defined(AFX_IDICOM_DICOMMESSAGE_MAIN_H_)
