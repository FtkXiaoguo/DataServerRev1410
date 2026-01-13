// XDcItem
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_XDCITEM__H_)
#define AFX_XDCITEM__H_
 
 

 
class XDcItem : public DcmItem  
{
public:
	XDcItem();
 	 /** destructor
     */
    virtual ~XDcItem();

#if 0
	  /** constructor.
     *  Create new item from given tag and length.
     *  @param tag DICOM tag for the new element
     *  @param len value length for the new element
     */
    XDcItem(const DcmTag &tag,
            const Uint32 len = 0);

    /** copy constructor
     *  @param old item to be copied
     */
     XDcItem(const XDcItem &old);

  
    /** clone method
     *  @return deep copy of this object
     */
    virtual DcmObject *clone() const
    {
      return new XDcItem(*this);
    }
#endif
	 /** This function reads the information of all attributes which
     *  are captured in the input stream and captures this information
     *  in elementList. Each attribute is represented as an element
     *  in this list. If not all information for an attribute could be
     *  read from the stream, the function returns EC_StreamNotifyClient.
     *  @param inStream      The stream which contains the information.
     *  @param ixfer         The transfer syntax which was used to encode
     *                       the information in inStream.
     *  @param glenc         Encoding type for group length; specifies
     *                       what will be done with group length tags.
     *  @param maxReadLength Maximum read length for reading an attribute value.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition read(DcmInputStream &inStream,
                             const E_TransferSyntax ixfer,
                             const E_GrpLenEncoding glenc = EGL_noChange,
                             const Uint32 maxReadLength = DCM_MaxReadLength);
protected:
	
};
 
#endif // !defined(AFX_XDCITEM__H_)
