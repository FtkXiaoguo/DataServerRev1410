// XDcItem
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_XDCDATSET__H_)
#define AFX_XDCDATSET__H_
 
 

 
class XDcmDataset : public DcmDataset  
{
public:
	/** default constructor
     */
    XDcmDataset();

    /** copy constructor
     *  @param old dataset to be copied
     */
    XDcmDataset(const XDcmDataset &old);

    /** destructor
     */
    virtual ~XDcmDataset();

    /** assignment operator
     *  @param obj the dataset to be copied
     */
    XDcmDataset& operator=(const XDcmDataset& obj);

    /** clone method
     *  @return deep copy of this object
     */
    virtual DcmObject *clone() const
    {
      return new XDcmDataset(*this);
    }

    /** Virtual object copying. This method can be used for DcmObject
     *  and derived classes to get a deep copy of an object. Internally
     *  the assignment operator is called if the given DcmObject parameter
     *  is of the same type as "this" object instance. If not, an error
     *  is returned. This function permits copying an object by value
     *  in a virtual way which therefore is different to just calling the
     *  assignment operator of DcmElement which could result in slicing
     *  the object.
     *  @param rhs - [in] The instance to copy from. Has to be of the same
     *                class type as "this" object
     *  @return EC_Normal if copying was successful, error otherwise
     */
    virtual OFCondition copyFrom(const XDcmDataset& rhs);

 
	 /** This function reads the information of all attributes which
     *  are captured in the input stream and captures this information
     *  in this->elementList. Each attribute is represented as an
     *  element in this list. Having read all information for this
     *  particular data set or command, this function will also take
     *  care of group length (according to what is specified in glenc)
     *  and padding elements (don't change anything).
     *  @param inStream      The stream which contains the information.
     *  @param xfer          The transfer syntax which was used to encode
     *                       the information in inStream.
     *  @param glenc         Encoding type for group length; specifies what
     *                       will be done with group length tags.
     *  @param maxReadLength Maximum read length for reading an attribute value.
     *  @return status, EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition read(DcmInputStream &inStream,
                             const E_TransferSyntax xfer = EXS_Unknown,
                             const E_GrpLenEncoding glenc = EGL_noChange,
                             const Uint32 maxReadLength = DCM_MaxReadLength);

	/** This function writes data values which are contained in this
     *  DcmDataset object to the stream which is passed as first argument.
     *  With regard to the writing of information, the other parameters
     *  which are passed are accounted for. The function will return
     *  EC_Normal, if the information from all elements of this data
     *  set has been written to the buffer, it will return EC_StreamNotifyClient,
     *  if there is no more space in the buffer and _not_ all elements
     *  have been written to it, and it will return some other (error)
     *  value if there was an error.
     *  @param outStream      The stream that the information will be written to.
     *  @param oxfer          The transfer syntax which shall be used.
     *  @param enctype        Encoding type for sequences; specifies how sequences
     *                        will be handled.
     *  @param wcache pointer to write cache object, may be NULL
     *  @param glenc          Encoding type for group length; specifies what will
     *                        be done with group length tags.
     *  @param padenc         Encoding type for padding. Specifies what will be done
     *                        with padding tags.
     *  @param padlen         The length up to which the dataset shall be padded, if
     *                        padding is desired.
     *  @param subPadlen      For sequences (ie sub elements), the length up to which
     *                        item shall be padded, if padding is desired.
     *  @param instanceLength Number of extra bytes added to the item/dataset length
     *                        used when computing the padding; this parameter is for
     *                        instance used to pass the length of the file meta header
     *                        from the DcmFileFormat to the DcmDataset object.
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
                              Uint32 instanceLength = 0);

	unsigned long getPixelDataOffset() const { return m_PixelDataOffset;};
protected:
	
	void writeBeforeCallback(const DcmObject *obj,const DcmOutputStream &outStream,E_TransferSyntax Xfer);
	void writeAfterCallback(const DcmObject *obj,const DcmOutputStream &outStream,E_TransferSyntax Xfer);

	unsigned long m_PixelDataOffset;
};
 
#endif // !defined(AFX_XDCDATSET__H_)
