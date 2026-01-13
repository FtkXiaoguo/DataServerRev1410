// XDcItem
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_XDCMFILEFORAMT__H_)
#define AFX_XDCMFILEFORAMT__H_
 
#include "dcmtk/dcmdata/dcfilefo.h"

 
class DcmOutputFileStream;
class DcmOutputBufferStream;
class XDcmFileFormat : public DcmFileFormat  
{
public:
	XDcmFileFormat();
 	 /** destructor
     */
	/** constructor
     *  @param dataset to be copied (!) into the new DcmFileFormat object
     */
    XDcmFileFormat(DcmDataset *dataset);

    XDcmFileFormat(const XDcmFileFormat &old);

    virtual ~XDcmFileFormat();

 
 
	unsigned long  getFileLength() const;

	 /** write object to a stream (abstract)
     *  @param outStream DICOM output stream
     *  @param oxfer output transfer syntax
     *  @param enctype encoding types (undefined or explicit length)
     *  @param wcache pointer to write cache object, may be NULL
     *  @param glenc group length encoding
     *  @param padenc dataset trailing padding encoding
     *  @param padlen padding structure size for complete file
     *  @param subPadlen padding structure set for sequence items
     *  @param instanceLength number of extra bytes added to the item/dataset
     *    length used when computing the padding. This parameter is for instance
     *    used to pass the length of the file meta information header from the
     *    DcmFileFormat to the DcmDataset object.
     *  @param writeMode write file with or without meta header. Also allows for
     *    updating the information in the file meta information header.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition write(DcmOutputStream &outStream,
                              const E_TransferSyntax oxfer,
                              const E_EncodingType enctype,
                              DcmWriteCache *wcache,
                              const E_GrpLenEncoding glenc,
                              const E_PaddingEncoding padenc = EPD_noChange,
                              const Uint32 padlen = 0,
                              const Uint32 subPadlen = 0,
                              Uint32 instanceLength = 0,
                              const E_FileWriteMode writeMode = EWM_fileformat);

	virtual OFCondition writeWithBuffer(DcmOutputFileStream *outputFile,
							  DcmOutputBufferStream *outStream, //memory buffer
                              const E_TransferSyntax oxfer,
                              const E_EncodingType enctype,
                              DcmWriteCache *wcache,
                              const E_GrpLenEncoding glenc,
                              const E_PaddingEncoding padenc = EPD_noChange,
                              const Uint32 padlen = 0,
                              const Uint32 subPadlen = 0,
                              Uint32 instanceLength = 0,
                              const E_FileWriteMode writeMode = EWM_fileformat);

	//
	 /** save object to a DICOM file.
     *  @param fileName name of the file to save
     *  @param writeXfer transfer syntax used to write the data (EXS_Unknown means use current)
     *  @param encodingType flag, specifying the encoding with undefined or explicit length
     *  @param groupLength flag, specifying how to handle the group length tags
     *  @param padEncoding flag, specifying how to handle the padding tags
     *  @param padLength number of bytes used for the dataset padding (has to be an even number)
     *  @param subPadLength number of bytes used for the item padding (has to be an even number)
     *  @param writeMode write file with or without meta header. Also allows for updating the
     *    information in the file meta information header.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition saveFile(const char *fileName,
                                 const E_TransferSyntax writeXfer = EXS_Unknown,
                                 const E_EncodingType encodingType = EET_UndefinedLength,
                                 const E_GrpLenEncoding groupLength = EGL_recalcGL,
                                 const E_PaddingEncoding padEncoding = EPD_noChange,
                                 const Uint32 padLength = 0,
                                 const Uint32 subPadLength = 0,
                                 const E_FileWriteMode writeMode = EWM_fileformat);

	//
	OFCondition  saveFileWithBuffer(
								unsigned long bufLen,const char *fileName,
								const E_TransferSyntax writeXfer = EXS_Unknown,
                                 const E_EncodingType encodingType = EET_UndefinedLength,
                                 const E_GrpLenEncoding groupLength = EGL_recalcGL,
                                 const E_PaddingEncoding padEncoding = EPD_noChange,
                                 const Uint32 padLength = 0,
                                 const Uint32 subPadLength = 0,
                                 const E_FileWriteMode writeMode = EWM_fileformat);

	 /** make sure that all data elements of the file meta information header are existent
     *  in metainfo and contain correct values.
     *  @param oxfer the transfer syntax which shall be used
     *  @param writeMode flag indicating whether to update the file meta information or not
     */
	//#10 2012/03/22 K.Ko
    virtual OFCondition validateMetaInfo(const E_TransferSyntax oxfer,
                                         const E_FileWriteMode writeMode = EWM_fileformat);
protected:
	 

	unsigned long m_FileLength;
};
 
 

#endif // !defined(AFX_XDCMFILEFORAMT__H_)
