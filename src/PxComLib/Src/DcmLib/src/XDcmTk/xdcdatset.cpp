//  
//
//////////////////////////////////////////////////////////////////////


 
#pragma warning (disable: 4616)
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/ofstd/ofstack.h"
#include "dcmtk/ofstd/ofstd.h"

#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcxfer.h"
#include "dcmtk/dcmdata/dcvrus.h"
#include "dcmtk/dcmdata/dcpixel.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcostrma.h"    /* for class DcmOutputStream */
#include "dcmtk/dcmdata/dcostrmf.h"    /* for class DcmOutputFileStream */
#include "dcmtk/dcmdata/dcistrma.h"    /* for class DcmInputStream */
#include "dcmtk/dcmdata/dcistrmf.h"    /* for class DcmInputFileStream */
#include "dcmtk/dcmdata/dcwcache.h"    /* for class DcmWriteCache */




#include "XDcdatset.h"

 

 
#include "CheckMemoryLeak.h"

XDcmDataset::XDcmDataset()
{
m_PixelDataOffset = 0;
 
}

/** copy constructor
     *  @param old dataset to be copied
     */
 XDcmDataset::XDcmDataset(const XDcmDataset &old)
	 :DcmDataset(old)
 {
	 m_PixelDataOffset = old.m_PixelDataOffset;
 }

    /** destructor
     */
 XDcmDataset::~XDcmDataset()
 {
 }

    /** assignment operator
     *  @param obj the dataset to be copied
     */

 XDcmDataset&  XDcmDataset::operator=(const XDcmDataset& obj)
 {
	 
 if (this != &obj)
  {
    // copy parent's member variables
    DcmDataset::operator=(obj);
    
  }
 m_PixelDataOffset = obj.m_PixelDataOffset;
  return *this;
 }

OFCondition XDcmDataset::copyFrom(const XDcmDataset& rhs)
{
  if (this != &rhs)
  {
    if (rhs.ident() != ident()) return EC_IllegalCall;
    *this = OFstatic_cast(const XDcmDataset &, rhs);

	 m_PixelDataOffset = rhs.m_PixelDataOffset;
  }
  return EC_Normal;
}


OFCondition XDcmDataset::write(DcmOutputStream &outStream,
                              const E_TransferSyntax oxfer,
                              const E_EncodingType enctype,
                              DcmWriteCache *wcache,
                              const E_GrpLenEncoding glenc,
                              const E_PaddingEncoding padenc,
                              const Uint32 padlen,
                              const Uint32 subPadlen,
                              Uint32 instanceLength)
{
  /* if the transfer state of this is not initialized, this is an illegal call */
  if (getTransferState() == ERW_notInitialized)
    errorFlag = EC_IllegalCall;
  else
  {
    /* check if the stream reported an error so far; if not, we can go ahead and write some data to it */
    errorFlag = outStream.status();

    if (errorFlag.good() && getTransferState() != ERW_ready)
    {
      /* Determine the transfer syntax which shall be used. Either we use the one which was passed, */
      /* or (if it's an unknown tranfer syntax) we use the one which is contained in this->Xfer. */
      E_TransferSyntax newXfer = oxfer;
      if (newXfer == EXS_Unknown)
        newXfer = DcmDataset::getOriginalXfer();//Xfer;

      /* if this function was called for the first time for the dataset object, the transferState is still */
      /* set to ERW_init. In this case, we need to take care of group length and padding elements according */
      /* to the strategies which are specified in glenc and padenc. Additionally, we need to set the element */
      /* list pointer of this data set to the fist element and we need to set the transfer state to ERW_inWork */
      /* so that this scenario will only be executed once for this data set object. */
      if (getTransferState() == ERW_init)
      {

        /* Check stream compression for this transfer syntax */
        DcmXfer xf(newXfer);
        E_StreamCompression sc = xf.getStreamCompression();
        switch (sc)
        {
          case ESC_none:
            // nothing to do
            break;
          case ESC_unsupported:
            // stream compressed transfer syntax that we cannot create; bail out.
            if (errorFlag.good())
              errorFlag = EC_UnsupportedEncoding;
            break;
          default:
            // supported stream compressed transfer syntax, install filter
            errorFlag = outStream.installCompressionFilter(sc);
            break;
        }

        /* take care of group length and padding elements, according to what is specified in glenc and padenc */
        computeGroupLengthAndPadding(glenc, padenc, newXfer, enctype, padlen, subPadlen, instanceLength);
        elementList->seek(ELP_first);
        setTransferState(ERW_inWork);
      }

      /* if the transfer state is set to ERW_inWork, we need to write the information which */
      /* is included in this data set's element list into the buffer which was passed. */
      if (getTransferState() == ERW_inWork)
      {
        // Remember that elementList->get() can be NULL if buffer was full after
        // writing the last item but before writing the sequence delimitation.
        if (!elementList->empty() && (elementList->get() != NULL))
        {
          /* as long as everything is ok, go through all elements of this data */
          /* set and write the corresponding information to the buffer */
          DcmObject *dO;
          do
          {
            dO = elementList->get();
			writeBeforeCallback(dO,outStream,newXfer);
            errorFlag = dO->write(outStream, newXfer, enctype, wcache);
			writeAfterCallback(dO,outStream,newXfer);
          } while (errorFlag.good() && elementList->seek(ELP_next));
        }

        /* if all the information in this has been written to the */
        /* buffer set this data set's transfer state to ERW_ready */
        if (errorFlag.good())
          setTransferState(ERW_ready);
      }
    }
  }

  /* return the corresponding result value */
  return errorFlag;
}

void XDcmDataset::writeBeforeCallback(const DcmObject *obj,const DcmOutputStream &outStream,E_TransferSyntax Xfer)
{
#if 0
	if(!obj) return;
 
	unsigned int tag = (obj->getGTag()<<16) + obj->getETag();
	if(0x7FE00010 == tag){
		m_PixelDataOffset = outStream.tell();
	}
#endif
}
void XDcmDataset::writeAfterCallback(const DcmObject *obj,const DcmOutputStream &outStream,E_TransferSyntax Xfer)
{
	if(!obj) return;
	if( (0x7FE0 == obj->getGTag()) && 
		(0x0010 == obj->getETag()) )
	{
		m_PixelDataOffset = outStream.tell() ;
		Uint32 val_len = obj->getLengthField();
		m_PixelDataOffset = m_PixelDataOffset - val_len;
	}
}

OFCondition XDcmDataset::read(DcmInputStream &inStream,
                             const E_TransferSyntax xfer,
                             const E_GrpLenEncoding glenc,
                             const Uint32 maxReadLength)
{
	return  DcmDataset::read(inStream,xfer, glenc,maxReadLength);
}