// DcmXTDataFileMain
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_IDICOM_DATA_FILE_MAIN_H_)
#define AFX_IDICOM_DATA_FILE_MAIN_H_
 

#include "IDcmLib.h"

using namespace XTDcmLib;

#include "DcmXTDataBase.h"

#if 0 // giveup
///
class DcmItem;
class DcmXTMetaHeaderMain;
class DcmXTDataSetMain;
class DcmXTDataFileMain : public DcmXTDataFile  , DcmXTDataBase
{
public:
	DcmXTDataFileMain();
	~DcmXTDataFileMain();
	virtual void Delete() ;
	virtual bool open(const dcm_string & fileName){ m_fileName = fileName; return true;};
	virtual void setMaxReadLength( long maxLen){ m_maxReadLength = maxLen;};
	virtual bool readFile(){return readFile(m_fileName) ;};
	virtual bool readFile(const dcm_string & fileName) ;

	virtual bool readFromDumpFile(const dcm_string & dumpFileName);
	virtual bool writeToDumpFile(const dcm_string & dumpFileName);

	virtual DcmXTMetaHeader *getDcmXTMetaHeader();
	virtual DcmXTDataSet *getDcmXTDataSet();
//	virtual DcmComInterface *getComInterface();
	// DcmComInterface
	virtual bool Set_Value(unsigned long  tag,	int val) ;
	virtual bool Set_Value(unsigned long  tag,	const dcm_string &val) ;
	//
	virtual bool Get_Value(unsigned long  tag,	long int &val) ;
	virtual bool Get_Value(unsigned long  tag,	dcm_string &val) ;
	//
protected:
	dcm_string m_fileName;
	void destroy();

	long m_maxReadLength ;
};
#endif
 
#endif // !defined(AFX_IDICOM_DATA_FILE_MAIN_H_)
