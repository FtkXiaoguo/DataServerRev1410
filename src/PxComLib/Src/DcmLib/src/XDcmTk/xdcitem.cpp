//  
//
//////////////////////////////////////////////////////////////////////




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


#include "xdcitem.h"


#include "DcmTkBase.h"

 


 XDcItem::XDcItem()
{

 
}
XDcItem::~XDcItem()
{
}
OFCondition XDcItem::read(DcmInputStream & inStream,
                          const E_TransferSyntax xfer,
                          const E_GrpLenEncoding glenc,
                          const Uint32 maxReadLength)
{
#if 1
    /* check if this is an illegal call; if so set the error flag and do nothing, else go ahead */
    if (fTransferState == ERW_notInitialized)
        errorFlag = EC_IllegalCall;
    else
    {
        /* figure out if the stream reported an error */
        errorFlag = inStream.status();
        /* if the stream reported an error or if it is the end of the */
        /* stream, set the error flag correspondingly; else go ahead */
        if (errorFlag.good() && inStream.eos())
            errorFlag = EC_EndOfStream;
        else if (errorFlag.good() && fTransferState != ERW_ready)
        {
            /* if the transfer state of this item is ERW_init, get its start */
            /* position in the stream and set the transfer state to ERW_inWork */
            if (fTransferState == ERW_init)
            {
                fStartPosition = inStream.tell();  // start position of this item
                fTransferState = ERW_inWork;
            }
            DcmTag newTag;
            /* start a loop in order to read all elements (attributes) which are contained in the inStream */
            while (inStream.good() && (fTransferredBytes < Length || !lastElementComplete))
            {
                /* initialize variables */
                Uint32 newValueLength = 0;
                Uint32 bytes_tagAndLen = 0;
                /* if the reading of the last element was complete, go ahead and read the next element */
                if (lastElementComplete)
                {
                    /* read this element's tag and length information (and */
                    /* possibly also VR information) from the inStream */
                    errorFlag = readTagAndLength(inStream, xfer, newTag, newValueLength, bytes_tagAndLen);
                    /* increase counter correpsondingly */
                    fTransferredBytes += bytes_tagAndLen;

                    /* if there was an error while we were reading from the stream, terminate the while-loop */
                    /* (note that if the last element had been read from the instream in the last iteration, */
                    /* another iteration will be started, and of course then readTagAndLength(...) above will */
                    /* return that it encountered the end of the stream. It is only then (and here) when the */
                    /* while loop will be terminated.) */
                    if (errorFlag.bad())
                        break;
                    /* If we get to this point, we just started reading the first part */
                    /* of an element; hence, lastElementComplete is not longer true */
                    lastElementComplete = OFFalse;
                    /* read the actual data value which belongs to this element */
                    /* (attribute) and insert this information into the elementList */
                    errorFlag = readSubElement(inStream, newTag, newValueLength, xfer, glenc, maxReadLength);
                    /* if reading was successful, we read the entire data value information */
                    /* for this element; hence lastElementComplete is true again */
                    if (errorFlag.good())
                        lastElementComplete = OFTrue;
                } else {
                    /* if lastElementComplete is false, we have only read the current element's */
                    /* tag and length (and possibly VR) information as well as maybe some data */
                    /* data value information. We need to continue reading the data value */
                    /* information for this particular element. */
                    errorFlag = elementList->get()->read(inStream, xfer, glenc, maxReadLength);
                    /* if reading was successful, we read the entire information */
                    /* for this element; hence lastElementComplete is true */
                    if (errorFlag.good())
                        lastElementComplete = OFTrue;
                }
                /* remember how many bytes were read */
                fTransferredBytes = inStream.tell() - fStartPosition;
                if (errorFlag.good())
                {
                    // If we completed one element, update the private tag cache.
					//koko
       //             if (lastElementComplete)
      //                  privateCreatorCache.updateCache(elementList->get());
                } else
                    break; // if some error was encountered terminate the while-loop
            } //while

            /* determine an appropriate result value; note that if the above called read function */
            /* encountered the end of the stream before all information for this element could be */
            /* read from the stream, the errorFlag has already been set to EC_StreamNotifyClient. */
            if ((fTransferredBytes < Length || !lastElementComplete) && errorFlag.good())
                errorFlag = EC_StreamNotifyClient;
            if (errorFlag.good() && inStream.eos())
                errorFlag = EC_EndOfStream;
        } // else errorFlag
        /* modify the result value: two kinds of special error codes do not count as an error */
        if (errorFlag == EC_ItemEnd || errorFlag == EC_EndOfStream)
            errorFlag = EC_Normal;
        /* if at this point the error flag indicates success, the item has */
        /* been read completely; hence, set the transfer state to ERW_ready. */
        /* Note that all information for this element could be read from the */
        /* stream, the errorFlag is still set to EC_StreamNotifyClient. */
        if (errorFlag.good())
            fTransferState = ERW_ready;
    }
#endif
    /* return result value */
    return errorFlag;


} // DcmItem::read()


