//  
//
//////////////////////////////////////////////////////////////////////


#pragma warning (disable: 4819)
#pragma warning (disable: 4244)
//////////////////
 

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */


#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcitem.h"
#include "dcmtk/dcmdata/dcxfer.h"
#include "dcmtk/dcmdata/dcvrobow.h"
#include "dcmtk/dcmdata/dcvrui.h"
#include "dcmtk/dcmdata/dcvrul.h"
#include "dcmtk/dcmdata/dcvrus.h"
#include "dcmtk/dcmdata/dcvrae.h"
#include "dcmtk/dcmdata/dcvrsh.h"
#include "dcmtk/dcmdata/dcmetinf.h"

#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcostrma.h"    /* for class DcmOutputStream */
#include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */
#include "dcmtk/dcmdata/dcistrma.h"    /* for class DcmInputStream */
#include "dcmtk/dcmdata/dcistrmf.h"    /* for class DcmInputFileStream */
#include "dcmtk/dcmdata/dcostrmb.h"    /* for class DcmOutputBufferStream */

#include "dcmtk/dcmdata/dcwcache.h"    /* for class DcmWriteCache */


#include "xdcfilefo.h"
#include "XDcdatset.h"
 
#include "CheckMemoryLeak.h"

 XDcmFileFormat::XDcmFileFormat()
{
 //DcmSequenceOfItems::itemList->remove();
DcmSequenceOfItems::itemList->deleteAllElements();

//ì‚è’¼‚µ
	DcmMetaInfo *MetaInfo = new DcmMetaInfo();
	DcmSequenceOfItems::itemList->insert(MetaInfo);
//
	XDcmDataset *Dataset = new XDcmDataset();
	DcmSequenceOfItems::itemList->insert(Dataset);

  m_FileLength = 0;
}

 XDcmFileFormat::XDcmFileFormat(DcmDataset *dataset)
	 :DcmFileFormat(dataset)
 {
	m_FileLength = 0;
 }

 XDcmFileFormat::XDcmFileFormat(const XDcmFileFormat &old)
  : DcmFileFormat(old)
{
	m_FileLength = 0;
}

XDcmFileFormat::~XDcmFileFormat()
{
//•s—v
//	XDcmDataset *Dataset = (XDcmDataset *)DcmSequenceOfItems::itemList->remove();
//	delete Dataset;
}
 
//#10 2012/03/22 K.Ko
OFCondition XDcmFileFormat::validateMetaInfo(const E_TransferSyntax oxfer,
                                            const E_FileWriteMode writeMode)
    /*
     * This function makes sure that all data elements of the meta header information are existent
     * in metainfo and contain correct values.
     *
     * Parameters:
     *   oxfer     - [in] The transfer syntax which shall be used.
     *   writeMode - [in] Flag indicating whether to update the file meta information or not.
     */
{
    /* initialize some variables */
    OFCondition l_error = EC_Normal;
    DcmMetaInfo *metinf = getMetaInfo();
 

#if 1
	OFString outImplementationClassUID;
	outImplementationClassUID.clear();
	 
	metinf->findAndGetOFString(DCM_ImplementationClassUID,outImplementationClassUID);

	OFString outImplementationVersionName;
	outImplementationVersionName.clear();

	metinf->findAndGetOFString(DCM_ImplementationVersionName,outImplementationVersionName);
 
	l_error = DcmFileFormat::validateMetaInfo( oxfer, writeMode);
	

	if(!outImplementationClassUID.empty()){
		metinf->putAndInsertString(DCM_ImplementationClassUID,outImplementationClassUID.c_str(),OFTrue/*replaceOld */);
	}
	//
	if(!outImplementationVersionName.empty()){
		metinf->putAndInsertString(DCM_ImplementationVersionName,outImplementationVersionName.c_str(),OFTrue/*replaceOld */);
	}
	 /* calculate new GroupLength for meta header */
    if (metinf->computeGroupLengthAndPadding(EGL_withGL, EPD_noChange,
        META_HEADER_DEFAULT_TRANSFERSYNTAX, EET_UndefinedLength).bad())
    {
        DCMDATA_ERROR("DcmFileFormat::validateMetaInfo() Group length of Meta Information Header not adapted");
    }

#else
	l_error = DcmFileFormat::validateMetaInfo( oxfer, writeMode);
#endif
    return l_error;
}


OFCondition XDcmFileFormat::write(DcmOutputStream &outStream,
                                 const E_TransferSyntax oxfer,
                                 const E_EncodingType enctype,
                                 DcmWriteCache *wcache,
                                 const E_GrpLenEncoding glenc,
                                 const E_PaddingEncoding padenc,
                                 const Uint32 padlen,
                                 const Uint32 subPadlen,
                                 Uint32 instanceLength,
                                 const E_FileWriteMode writeMode)
    /*
     * This function writes data values which are contained in this to the stream which is
     * passed as first argument. With regard to the writing of information, the other parameters
     * which are passed are accounted for. The function will return EC_Normal, if the information
     * from all elements of this data set has been written to the buffer, and it will return some
     * other (error) value if there was an error.
     *
     * Parameters:
     *   outStream      - [inout] The stream that the information will be written to.
     *   oxfer          - [in] The transfer syntax which shall be used.
     *   enctype        - [in] Encoding type for sequences. Specifies how sequences will be handled.
     *   glenc          - [in] Encoding type for group length. Specifies what will be done with group length tags.
     *   padenc         - [in] Encoding type for padding. Specifies what will be done with padding tags.
     *   padlen         - [in] The length up to which the dataset shall be padded, if padding is desired.
     *   subPadlen      - [in] For sequences (i.e. sub elements), the length up to which item shall be padded,
     *                         if padding is desired.
     *   instanceLength - [in] Number of extra bytes added to the item/dataset length used when computing the
     *                         padding. This parameter is for instance used to pass the length of the file meta
     *                         header from the DcmFileFormat to the DcmDataset object.
     *   writeMode      - [in] Write file with or without meta header. Also allows for updating the information
     *                         in the file meta information header.
     */
{
    /* if the transfer state of this is not initialized, this is an illegal call */
    if (getTransferState() == ERW_notInitialized)
        errorFlag = EC_IllegalCall;
    else
    {
        /* if this is not an illegal call, do something */

        /* assign data set and the meta information header to local variables */
        DcmDataset *dataset = getDataset();
        DcmMetaInfo *metainfo = getMetaInfo();
        /* Determine the transfer syntax which shall be used. Either we use the one which was passed, */
        /* or (if it's an unknown tranfer syntax) we use the data set's original transfer syntax. */
        E_TransferSyntax outxfer = oxfer;
        if (outxfer == EXS_Unknown && dataset)
            outxfer = dataset->getOriginalXfer();
        /* check if the stream reported an error so far */
        errorFlag = outStream.status();
        /* check if we can actually write data to the stream; in certain cases we cannot. */
        if (outxfer == EXS_Unknown || outxfer == EXS_BigEndianImplicit)
            errorFlag = EC_IllegalCall;
        else if (itemList->empty())
            errorFlag = EC_CorruptedData;
        else if (errorFlag.good() && getTransferState() != ERW_ready)
        {
            /* in this case we can write data to the stream */

            /* if this function was called for the first time for the dataset object, the transferState is */
            /* still set to ERW_init. In this case, we need to validate the meta header information, set the */
            /* item list pointer to the fist element and we need to set the transfer state to ERW_inWork. */
            if (getTransferState() == ERW_init)
            {
                validateMetaInfo(outxfer, writeMode);
                itemList->seek(ELP_first);
                setTransferState(ERW_inWork);
            }
            /* if the transfer state is set to ERW_inWork, we need to write the */
            /* information which is included in this to the buffer which was passed. */
            if (getTransferState() == ERW_inWork)
            {
                /* write meta header information */
                errorFlag = metainfo->write(outStream, outxfer, enctype, wcache);
                /* recalculate the instance length */
                instanceLength += metainfo->calcElementLength(outxfer, enctype);
                /* if everything is ok, write the data set */
                if (errorFlag.good())
                    errorFlag = dataset->write(outStream, outxfer, enctype, wcache, glenc, padenc, padlen,
                                               subPadlen, instanceLength);
                /* if everything is ok, set the transfer state to ERW_ready */
                if (errorFlag.good())
                    setTransferState(ERW_ready);
            }
        }
        /* in case the transfer syntax which shall be used is indeed the */
        /* BigEndianImplicit transfer syntax dump some error information */
        if (outxfer == EXS_BigEndianImplicit)
            DCMDATA_ERROR("DcmFileFormat::write() Illegal TransferSyntax (BigEndianImplicit) used");
    }
    /* return result value */
    return errorFlag;
}


OFCondition XDcmFileFormat::saveFile(const char *fileName,
                                    const E_TransferSyntax writeXfer,
                                    const E_EncodingType encodingType,
                                    const E_GrpLenEncoding groupLength,
                                    const E_PaddingEncoding padEncoding,
                                    const Uint32 padLength,
                                    const Uint32 subPadLength,
                                    const E_FileWriteMode writeMode)
{

	m_FileLength = 0;
    if (writeMode == EWM_dataset)
    {
        return getDataset()->saveFile(fileName, writeXfer, encodingType, groupLength,
            padEncoding, padLength, subPadLength);
    }
    OFCondition l_error = EC_IllegalParameter;
    /* check parameters first */
    if ((fileName != NULL) && (strlen(fileName) > 0))
    {
        DcmWriteCache wcache;

        /* open file for output */
        DcmOutputFileStream fileStream(fileName);

        /* check stream status */
        l_error = fileStream.status();
        if (l_error.good())
        {
            /* write data to file */
            transferInit();
            l_error = write(fileStream, writeXfer, encodingType, &wcache, groupLength,
                padEncoding, padLength, subPadLength, 0 /*instanceLength*/, writeMode);
            transferEnd();
			//
			m_FileLength = fileStream.tell();
        }
    }
    return l_error;
}
 
 
unsigned long  XDcmFileFormat::getFileLength() const
{
	return m_FileLength;
}


OFCondition XDcmFileFormat::saveFileWithBuffer(
									unsigned long bufLen,
									const char *fileName,
                                    const E_TransferSyntax writeXfer,
                                    const E_EncodingType encodingType,
                                    const E_GrpLenEncoding groupLength,
                                    const E_PaddingEncoding padEncoding,
                                    const Uint32 padLength,
                                    const Uint32 subPadLength,
                                    const E_FileWriteMode writeMode)
{



	m_FileLength = 0;
    if (writeMode == EWM_dataset)
    {
        return getDataset()->saveFile(fileName, writeXfer, encodingType, groupLength,
            padEncoding, padLength, subPadLength);
    }
    OFCondition l_error = EC_IllegalParameter;
    /* check parameters first */
    if ((fileName != NULL) && (strlen(fileName) > 0))
    {
        DcmWriteCache wcache;

		bufLen = (bufLen/2)*2;
		unsigned char *write_buf = new unsigned char[bufLen];
		try
		{
        /* open file for output */
			 DcmOutputFileStream fileStream(fileName);
				 
			 DcmOutputBufferStream outBufStream(write_buf, bufLen);

 
			/* check stream status */
			l_error = fileStream.status();
			if (l_error.good())
			{
				/* write data to file */
				transferInit();
				l_error = writeWithBuffer(
					&fileStream,
					&outBufStream, writeXfer, encodingType, &wcache, groupLength,
					padEncoding, padLength, subPadLength, 0 /*instanceLength*/, writeMode);
				transferEnd();
				//
	 			m_FileLength = fileStream.tell();
				 
			}
		 
		}catch(...)
		{
			;//
		}

		delete [] write_buf;
    }
    return l_error;
}
 
OFCondition XDcmFileFormat::writeWithBuffer(
								DcmOutputFileStream *outputFile,//write to file
								DcmOutputBufferStream *outBufferStream,  //memory buffer
                                 const E_TransferSyntax oxfer,
                                 const E_EncodingType enctype,
                                 DcmWriteCache *wcache,
                                 const E_GrpLenEncoding glenc,
                                 const E_PaddingEncoding padenc,
                                 const Uint32 padlen,
                                 const Uint32 subPadlen,
                                 Uint32 instanceLength,
                                 const E_FileWriteMode writeMode)
    /*
     * This function writes data values which are contained in this to the stream which is
     * passed as first argument. With regard to the writing of information, the other parameters
     * which are passed are accounted for. The function will return EC_Normal, if the information
     * from all elements of this data set has been written to the buffer, and it will return some
     * other (error) value if there was an error.
     *
     * Parameters:
     *   outStream      - [inout] The stream that the information will be written to.
     *   oxfer          - [in] The transfer syntax which shall be used.
     *   enctype        - [in] Encoding type for sequences. Specifies how sequences will be handled.
     *   glenc          - [in] Encoding type for group length. Specifies what will be done with group length tags.
     *   padenc         - [in] Encoding type for padding. Specifies what will be done with padding tags.
     *   padlen         - [in] The length up to which the dataset shall be padded, if padding is desired.
     *   subPadlen      - [in] For sequences (i.e. sub elements), the length up to which item shall be padded,
     *                         if padding is desired.
     *   instanceLength - [in] Number of extra bytes added to the item/dataset length used when computing the
     *                         padding. This parameter is for instance used to pass the length of the file meta
     *                         header from the DcmFileFormat to the DcmDataset object.
     *   writeMode      - [in] Write file with or without meta header. Also allows for updating the information
     *                         in the file meta information header.
     */
{
	

    /* if the transfer state of this is not initialized, this is an illegal call */
    if (getTransferState() == ERW_notInitialized)
        errorFlag = EC_IllegalCall;
    else
    {
        /* if this is not an illegal call, do something */

        /* assign data set and the meta information header to local variables */
        DcmDataset *dataset = getDataset();
        DcmMetaInfo *metainfo = getMetaInfo();
        /* Determine the transfer syntax which shall be used. Either we use the one which was passed, */
        /* or (if it's an unknown tranfer syntax) we use the data set's original transfer syntax. */
        E_TransferSyntax outxfer = oxfer;
        if (outxfer == EXS_Unknown && dataset)
            outxfer = dataset->getOriginalXfer();
        /* check if the stream reported an error so far */
        errorFlag = outBufferStream->status();
        /* check if we can actually write data to the stream; in certain cases we cannot. */
        if (outxfer == EXS_Unknown || outxfer == EXS_BigEndianImplicit)
            errorFlag = EC_IllegalCall;
        else if (itemList->empty())
            errorFlag = EC_CorruptedData;
        else if (errorFlag.good() && getTransferState() != ERW_ready)
        {
            /* in this case we can write data to the stream */

            /* if this function was called for the first time for the dataset object, the transferState is */
            /* still set to ERW_init. In this case, we need to validate the meta header information, set the */
            /* item list pointer to the fist element and we need to set the transfer state to ERW_inWork. */
            if (getTransferState() == ERW_init)
            {
                validateMetaInfo(outxfer, writeMode);
                itemList->seek(ELP_first);
                setTransferState(ERW_inWork);
            }
            /* if the transfer state is set to ERW_inWork, we need to write the */
            /* information which is included in this to the buffer which was passed. */
            if (getTransferState() == ERW_inWork)
            {
#if 0
                /* write meta header information */
                errorFlag = metainfo->write(outStream, outxfer, enctype, wcache);
                /* recalculate the instance length */
                instanceLength += metainfo->calcElementLength(outxfer, enctype);
                /* if everything is ok, write the data set */
                if (errorFlag.good())
                    errorFlag = dataset->write(outStream, outxfer, enctype, wcache, glenc, padenc, padlen,
                                               subPadlen, instanceLength);
                /* if everything is ok, set the transfer state to ERW_ready */
                if (errorFlag.good())
                    setTransferState(ERW_ready);
#else
				//////////////////////////////////
				//write meta header information 
				{
					OFCondition econd = EC_Normal;

					OFBool last = OFFalse ;
					OFBool written = OFFalse;
					///
					offile_off_t rtnLength;
					///
					while (!last)
					 {
						 if (! written)
						{
						  econd = metainfo->write(*outBufferStream, outxfer, enctype, wcache);
						  if (econd == EC_Normal)                   /* all contents have been written to the buffer */
						  {
							  written = OFTrue;
						  }
						  else if (econd == EC_StreamNotifyClient)  /* no more space in buffer, _not_ all elements have been written to it */
						  {
							  // nothing to do
							  // just go on
						  }
						  else                                      /* some error has occurred */
						  {
							  errorFlag = econd;
							  return econd;
						  }
						}
					  
						if (written) outBufferStream->flush(); // flush stream including embedded compression codec.

						/* get buffer and its length, assign to local variable */
						void *fullBuf = NULL;
						outBufferStream->flushBuffer(fullBuf, rtnLength);

						last = written && outBufferStream->isFlushed();

						/* if the buffer is not empty, do something with its contents */
						if (rtnLength > 0)
						{
							/* rtnLength could be odd */
							//if (!(rtnLength & 1))
							{
								//int x =0 ;
					//			return EC_IllegalParameter;
								//not so !
							}
							 

							/* since the block size is always even, block size must be larger
							* than rtnLength, so we can safely add a pad byte (and hope that
							* the pad byte will not confuse the receiver's decompressor).
							*/
							if(rtnLength != outputFile->write(fullBuf,rtnLength)){
								errorFlag = EC_IllegalCall;
								return errorFlag;
							};
						}
					}
					//
					metainfo->transferEnd();
				}
				////////////
				/* recalculate the instance length */
                instanceLength += metainfo->calcElementLength(outxfer, enctype);

				//////////////////////////////////
				//write write the data set 
				{
					OFCondition econd = EC_Normal;

					OFBool last = OFFalse ;
					OFBool written = OFFalse;
					///
					offile_off_t rtnLength;
					///
					while (!last)
					 {
						 if (! written)
						{
						  econd = dataset->write(*outBufferStream, outxfer, enctype, wcache, glenc, padenc, padlen,
												   subPadlen, instanceLength);
						  if (econd == EC_Normal)                   /* all contents have been written to the buffer */
						  {
							  written = OFTrue;
						  }
						  else if (econd == EC_StreamNotifyClient)  /* no more space in buffer, _not_ all elements have been written to it */
						  {
							  // nothing to do
							  // just go on
						  }
						  else                                      /* some error has occurred */
						  {
							  errorFlag = econd;
							  return econd;
						  }
						}
					  
						if (written) outBufferStream->flush(); // flush stream including embedded compression codec.

						/* get buffer and its length, assign to local variable */
						void *fullBuf = NULL;
						outBufferStream->flushBuffer(fullBuf, rtnLength);

						last = written && outBufferStream->isFlushed();

						/* if the buffer is not empty, do something with its contents */
						if (rtnLength > 0)
						{
							/* rtnLength could be odd */
							//if (!(rtnLength & 1))
							{
								//int x =0;
								//return EC_IllegalParameter;
								//not so !
							}
							 

							/* since the block size is always even, block size must be larger
							* than rtnLength, so we can safely add a pad byte (and hope that
							* the pad byte will not confuse the receiver's decompressor).
							*/
							if(rtnLength != outputFile->write(fullBuf,rtnLength)){
								errorFlag = EC_IllegalCall;
								return errorFlag;
							};

						}
					}
					//
					dataset->transferEnd();
					 
				}
				//everything is ok
				setTransferState(ERW_ready);
#endif
            }
        }
        /* in case the transfer syntax which shall be used is indeed the */
        /* BigEndianImplicit transfer syntax dump some error information */
        if (outxfer == EXS_BigEndianImplicit)
            DCMDATA_ERROR("DcmFileFormat::write() Illegal TransferSyntax (BigEndianImplicit) used");
    }
    /* return result value */
    return errorFlag;
}

