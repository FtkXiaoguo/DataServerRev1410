//  
//
//////////////////////////////////////////////////////////////////////



#include "DcmXTDataFileMain.h"

#include "DcmXTDataSetMain.h"

//////////////////
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcistrmz.h"    /* for dcmZlibExpectRFC1950Encoding */

#include "dcmtk/dcmnet/dimse.h"

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTRING
#include "dcmtk/ofstd/ofstdinc.h"

#include "DcmTkBase.h"

OFBool
readDumpFile(DcmMetaInfo * metaheader, DcmDataset * dataset,
         FILE * infile, const char * ifname, const OFBool stopOnErrors,
         const unsigned long maxLineLength);

const unsigned int DCM_DumpMaxLineSize = 4096;


 DcmXTDataFileMain::DcmXTDataFileMain()
{

	m_DcmMetaHeader = 0;
	m_DcmDataset = 0;

	m_maxReadLength = 4096;//2012/03/23 
}
DcmXTDataFileMain::~DcmXTDataFileMain()
{
}
void DcmXTDataFileMain::Delete()
{
	destroy();
	delete this;
}
void DcmXTDataFileMain::destroy()
{
	
	DcmXTDataBase::destroy();
	
}
bool DcmXTDataFileMain::readFile(const dcm_string & fileName)
{
	destroy();
 
	E_FileReadMode readMode = ERM_autoDetect;
	OFCmdUnsignedInt maxReadLength = m_maxReadLength;//4096; // default is 4 KB
	E_TransferSyntax xfer = EXS_Unknown;

	 DcmFileFormat dfile;
    DcmObject *dset = dfile.getDataset();
	 
	DcmMetaInfo * metaheader = dfile.getMetaInfo();

 

	SetDebugLevel(5);

	const char *ifname = stringTochar(fileName);

//    if (readMode == ERM_dataset) dset = dfile.getDataset();
    OFCondition cond = dfile.loadFile(ifname, xfer, EGL_noChange, maxReadLength, readMode);
    if (! cond.good())
    {
         return false;
    }
//	DIMSE_debug(1);

//	dset->loadAllDataIntoMemory();

	dset->print(COUT);

//	COUT << ">>>>>====" <<endl;
//	dset->print(COUT);
//

	m_DcmDataset = new DcmXTDataSetMain((DcmDataset*)dset);
	 
	m_DcmMetaHeader = new DcmXTMetaHeaderMain(metaheader);

	return true;
};

bool DcmXTDataFileMain::readFromDumpFile(const dcm_string & dumpFileName)
{
	 
	return DcmXTDataBase::readFromDumpFile(dumpFileName);
}

bool DcmXTDataFileMain::writeToDumpFile(const dcm_string & dumpFileName)
{
	 
 
	return DcmXTDataBase::writeToDumpFile(dumpFileName);
}

DcmXTDataSet *DcmXTDataFileMain::getDcmXTDataSet()
{
	DcmXTDataSet *ret_dataset=m_DcmDataset;

	 

	return ret_dataset;
}
DcmXTMetaHeader *DcmXTDataFileMain::getDcmXTMetaHeader()
{
	DcmXTMetaHeader *ret_dcmMeta=m_DcmMetaHeader;


	return ret_dcmMeta;
}
 
#if 0
DcmComInterface *DcmXTDataFileMain::getComInterface()
{
	return this;
}
#endif
	
// DcmComInterface
#define SET_DCM_VALUE(tag, val) { \
	if(m_DcmMetaHeader){	if( m_DcmMetaHeader->Set_Value( tag,	val) )	return true;  } \
	if(m_DcmDataset){		if(!m_DcmDataset->Set_Value( tag,	val) )		return false; } \
}
#define GET_DCM_VALUE(tag, val) { \
	if(m_DcmMetaHeader){	if( m_DcmMetaHeader->Get_Value( tag,	val) )	return true;  } \
	if(m_DcmDataset){		if(!m_DcmDataset->Get_Value( tag,	val) )		return false; } \
}

bool DcmXTDataFileMain::Set_Value(unsigned long  tag,	int val)
{
	SET_DCM_VALUE(tag,val);
	return true;
}
bool DcmXTDataFileMain::Set_Value(unsigned long  tag,	const dcm_string &val)
{
	SET_DCM_VALUE(tag,val);
	return true;
}
	//
bool DcmXTDataFileMain::Get_Value(unsigned long  tag,	long int &val)
{
	GET_DCM_VALUE(tag,val);
	return true;
}
bool DcmXTDataFileMain::Get_Value(unsigned long  tag,	dcm_string &val)
{
	GET_DCM_VALUE(tag,val);
	return true;
}