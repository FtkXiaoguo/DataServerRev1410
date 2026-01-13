#============================================================================
#
#                       ==== MergeCOM-3 System Profile ===
#
#       The location of this file is provided in the MERGECOM_3_PROFILE
#          parameter of the [MergeCOM3] section of the MERGE.INI file
#
#============================================================================
#
#  Contains the following sections:
#    [ASSOC_PARMS]      - MergeCOM association parameters
#    [DIMSE_PARMS]      - DIMSE Layer parameters
#    [DUL_PARMS]        - Upper Layer parameters
#    [TRANSPORT_PARMS]  - Transport Layer parameters
#    [MESSAGE_PARMS]    - Message parameters
#


#============================================================================
#                    MergeCOM-3 KERNEL CONFIGURATION SECTION
#============================================================================
[ASSOC_PARMS]
    LICENSE                      = 
    IMPLEMENTATION_CLASS_UID     = 2.16.840.1.113669.2.1.1 # Insert your Implementation Class UID
    IMPLEMENTATION_VERSION       = MergeCOM3_310   # 16 characters maximum
    LOCAL_APPL_CONTEXT_NAME      = 1.2.840.10008.3.1.1.1
    ACCEPT_ANY_CONTEXT_NAME         = Yes          # (must be ours if no)
    ACCEPT_ANY_APPLICATION_TITLE    = No           # as long as some application is
                                                   # registered
    ACCEPT_DIFFERENT_IC_UID         = Yes          # (must be ours if no)
    ACCEPT_DIFFERENT_VERSION        = Yes          # (if received)
    ACCEPT_ANY_PROTOCOL_VERSION     = No           # (if received)
    ACCEPT_ANY_PRESENTATION_CONTEXT = Yes          # presentation context negotiated 
                                                   # for service must match context 
                                                   # used with message
    ACCEPT_MULTIPLE_PRES_CONTEXTS   = Yes          # Allow more than one 
                                                   # presentation context per 
                                                   # service to be negotiated
    ACCEPT_ANY_HOSTNAME             = Yes          # Hosts connecting must have entry in 
                                                   # hosts file or domain name server.
    AUTO_ECHO_SUPPORT               = Yes          # Default: Yes
    HARD_CLOSE_TCP_IP_CONNECTION    = No           # Default: No
                                                   # When set to No, connections 
                                                   # are closed with a FIN packet.  
                                                   # When set to Yes, connections 
                                                   # are closed with a RST packet.
    PDU_MAXIMUM_LENGTH              = 28672        # Max size of Protocol Data Units
                                                   # that can be received by this
                                                   # MergeCOM-3 implementation
                                                   # Must be 4K, 8K, or 16K or >16K
                                                   # This value will also place a limit on
                                                   # how large PDU values being sent can 
                                                   # be.  
                                                   # Note also to see the 
                                                   # TCPIP_SEND_BUFFER_SIZE and 
                                                   # TCPIP_RECEIVE_BUFFER_SIZE 
                                                   # configuration values for 
                                                   # improving performance.
                           
    
    # Transfer Syntax UIDs   - Use <none> to prevent using a given syntax
    #                          (implicit little endian ALWAYS used)     
    IMPLICIT_LITTLE_ENDIAN_SYNTAX        = 1.2.840.10008.1.2
    IMPLICIT_BIG_ENDIAN_SYNTAX           = <none>
    EXPLICIT_LITTLE_ENDIAN_SYNTAX        = 1.2.840.10008.1.2.1
    EXPLICIT_BIG_ENDIAN_SYNTAX           = 1.2.840.10008.1.2.2
    
    # Encapsulated transfer syntax UIDs
    RLE_SYNTAX                           = 1.2.840.10008.1.2.5    
    JPEG_BASELINE_SYNTAX                 = 1.2.840.10008.1.2.4.50    
    JPEG_EXTENDED_2_4_SYNTAX             = 1.2.840.10008.1.2.4.51    
    JPEG_EXTENDED_3_5_SYNTAX             = 1.2.840.10008.1.2.4.52    
    JPEG_SPEC_NON_HIER_6_8_SYNTAX        = 1.2.840.10008.1.2.4.53    
    JPEG_SPEC_NON_HIER_7_9_SYNTAX        = 1.2.840.10008.1.2.4.54    
    JPEG_FULL_PROG_NON_HIER_10_12_SYNTAX = 1.2.840.10008.1.2.4.55    
    JPEG_FULL_PROG_NON_HIER_11_13_SYNTAX = 1.2.840.10008.1.2.4.56    
    JPEG_LOSSLESS_NON_HIER_14_SYNTAX     = 1.2.840.10008.1.2.4.57    
    JPEG_LOSSLESS_NON_HIER_15_SYNTAX     = 1.2.840.10008.1.2.4.58    
    JPEG_EXTENDED_HIER_16_18_SYNTAX      = 1.2.840.10008.1.2.4.59    
    JPEG_EXTENDED_HIER_17_19_SYNTAX      = 1.2.840.10008.1.2.4.60    
    JPEG_SPEC_HIER_20_22_SYNTAX          = 1.2.840.10008.1.2.4.61    
    JPEG_SPEC_HIER_21_23_SYNTAX          = 1.2.840.10008.1.2.4.62    
    JPEG_FULL_PROG_HIER_24_26_SYNTAX     = 1.2.840.10008.1.2.4.63    
    JPEG_FULL_PROG_HIER_25_27_SYNTAX     = 1.2.840.10008.1.2.4.64    
    JPEG_LOSSLESS_HIER_28_SYNTAX         = 1.2.840.10008.1.2.4.65    
    JPEG_LOSSLESS_HIER_29_SYNTAX         = 1.2.840.10008.1.2.4.66    
    JPEG_LOSSLESS_HIER_14_SYNTAX         = 1.2.840.10008.1.2.4.70    

    # Private Transfer Syntaxes
    PRIVATE_SYNTAX_1_SYNTAX              = <none>  # UID to be used to identify
                                                   # private transfer syntax 1
    PRIVATE_SYNTAX_1_LITTLE_ENDIAN       = Yes     # If Yes, syntax 1 is little
                                                   # endian, if No, syntax is 
                                                   # big endian
    PRIVATE_SYNTAX_1_EXPLICIT_VR         = Yes     # If Yes, syntax 1 is explicit VR
    PRIVATE_SYNTAX_1_ENCAPSULATED        = No      # If Yes, transfer syntax is 
                                                   # encapsulated and encapsulated 
                                                   # rules must be followed for
                                                   # pixel data (7fe0,0010).
    PRIVATE_SYNTAX_2_SYNTAX              = <none>  # UID to be used to identify
                                                   # private transfer syntax 2
    PRIVATE_SYNTAX_2_LITTLE_ENDIAN       = Yes     # If Yes, syntax 2 is little 
                                                   # endian, if No, syntax is 
                                                   # big endian
    PRIVATE_SYNTAX_2_EXPLICIT_VR         = Yes     # If Yes, syntax 2 is explicit VR
    PRIVATE_SYNTAX_2_ENCAPSULATED        = No      # If Yes, transfer syntax is 
                                                   # encapsulated and encapsulated
                                                   # rules must be followed for
                                                   # pixel data (7fe0,0010).

#============================================================================
#                     DIMSE LAYER CONFIGURATION SECTIONS
#============================================================================
#
#  Default DICOM DIMSE Layer Parameters
#
[DIMSE_PARMS]
    SEND_SOP_CLASS_UID         =  YES        # Version 3 command field
    SEND_SOP_INSTANCE_UID      =  YES        # Version 3 command field

    INITIATOR_NAME             =  <NONE>     # Retired   command field
    RECEIVER_NAME              =  <NONE>     # Retired   command field
    SEND_LENGTH_TO_END         =  NO         # Retired   command field
    SEND_RECOGNITION_CODE      =  NO         # Retired   command field 
    SEND_MSG_ID_RESPONSE       =  NO         # Version 2 command field  
    SEND_ECHO_PRIORITY         =  NO         # Version 2 command field
    SEND_RESPONSE_PRIORITY     =  NO         # Version 2 command field 
    



#============================================================================
#                     UPPER LAYER CONFIGURATION SECTION
#============================================================================
#
#  DICOM Upper Layer Parameters
#    The Upper Layer ALWAYS uses reads this section for its parameters
#
[DUL_PARMS]               # Dicom Upper Layer Parameters
    ARTIM_TIMEOUT             = 30  # The number of seconds to use as a
                                    #     timeout waiting for association
                                    #     request or waiting for the peer
                                    #     to shut down an association.
    ASSOC_REPLY_TIMEOUT       = 15  # The number of seconds to wait
                                    #     for reply to associate request.
    RELEASE_TIMEOUT           = 15  # The number of seconds to wait
                                    #     for reply to associate release.
    WRITE_TIMEOUT             = 15  # The number of seconds to wait
                                    #     for a network write to be accepted.
    CONNECT_TIMEOUT           = 15  # The number of seconds to wait
                                    #     for a network connect to be accepted.
    INACTIVITY_TIMEOUT        = 15  # The number of seconds to wait for data
                                    #     between TCP/IP packets on a call to
                                    #     MC_Read_Message().


#============================================================================
#                    MergeCOM-3 MESSAGE CONFIGURATION SECTION
#============================================================================
[MESSAGE_PARMS]
    LARGE_DATA_STORE            = MEM    #  MEM | FILE   Default = MEM
    LARGE_DATA_SIZE             = 4096
    DICTIONARY_ACCESS           = MEM    #  MEM | FILE   Default = MEM
    DICTIONARY_FILE             =C:\Program Files\Merge\Toolkit v3.1.0\mc3msg\mrgcom3.dct
    MSG_INFO_FILE               =C:\Program Files\Merge\Toolkit v3.1.0\mc3msg\mrgcom3.msg
    TEMP_FILE_DIRECTORY         = .      # Note that on Windows 200 platforms,
                                         # this value can be changes to %TEMP%
                                         # to specify the system's configured
                                         # temporary directory

    OBOW_BUFFER_SIZE            = 16384  # Used when LARGE_DATA_STORE is set
                                         # to FILE.  This setting configures
                                         # the size chunks that OB or OW data
                                         # is spooled to the temporary files in.
    WORK_BUFFER_SIZE            = 28672  # Recommended value: 28K (28672 bytes)
                                         # Buffer size used when streaming in
                                         # or out messages/files.  Using larger
                                         # values for this option can improve
                                         # performance
    ELIMINATE_ITEM_REFERENCES   = No     # When set to Yes, the  MC_Free_Item()
                                         # will check all other open messages
                                         # for references to this item. Setting
                                         # this value to No will greatly improve
                                         # the performance of free'ing DICOMDIRs
    FORCE_OPEN_EMPTY_ITEM       = No     # Force the MC_Open_Item() function to
                                         # behave similar to the 
                                         # MC_Open_Empty_Message() function.
                                         # This will reduce the amount of 
                                         # memory required to store items.
    EMPTY_PRIVATE_CREATOR_CODES = Yes    # MC_Empty_Message() will empty private
                                         # creator codes when this is enabled
    UNKNOWN_VR_CODE             = UN     # Valid values: UN, OB.
                                         # VR Code to use for unknown VRs, ie    
                                         # private attributes that are not in the
                                         # data dictionary which are received in
                                         # the implicit VR little endian transfer
                                         # syntax.  'UN' is the official DICOM
                                         # VR, however, 'OB' may need to be used
                                         # if an implementation does not 
                                         # understand 'UN' and the private 
                                         # attributes are to be preserved.
    ALLOW_INVALID_PRIVATE_ATTRIBUTES = No # When set to 'No', private attributes 
                                         # that are not encoded according to 
                                         # DICOM will be ignored by the library.
                                         # IE, if a private attribute does not
                                         # have a creator code associated with   
                                         # it.  When set to 'Yes', these 
                                         # attributes are treated as "garbage
                                         # in, garbage out".
    ALLOW_INVALID_PRIVATE_CREATOR_CODES = No  # When set to 'No', private 
                                         # creator codes that are not encoded 
                                         # according to DICOM will be ignored
                                         # by the library.  Private elements
                                         # associated with these creator codes
                                         # will also be ignored.
    CALLBACK_MIN_DATA_SIZE      = 1      # Minimum size OB/OW tag for which to
                                         # use a callback function registered 
                                         # with MC_Register_Callback_Funciton().
                                         # This is useful to ignore small pixel
                                         # data in a message such as an Icon.
    REMOVE_PADDING_CHARS        = No     # When set to Yes, the tool kit will
                                         # remove space padding characters from
                                         # all text based attributes.  This
                                         # removal will occur when the attribute
                                         # is encoded with one of the MC_Set_Value
                                         # functions, or when the attributes are
                                         # read into the tool kit with one of
                                         # the streaming or network read
                                         # functions.
    EXPORT_UN_VR_TO_NETWORK     = Yes    # When set to No, do not export UN VR 
                                         # attributes while using the 
                                         # MC_Send_Request_Message() and 
                                         # MC_Send_Response_Message() functions
    EXPORT_PRIVATE_ATTRIBUTES_TO_NETWORK = Yes # When set to no, do not export 
                                         # private attributes while using the 
                                         # MC_Send_Request_Message() and 
                                         # MC_Send_Response_Message() functions
    ALLOW_COMMA_IN_DS_FL_FD_STRINGS = No # If Yes, a comma or a period will be allowed 
                                         # in the value passed to MC_Set_Value_From_String
                                         # for attributes with a VR of DS or FL or FD.
                                         # If No, only a period will be acceptable
                                         # as the radix character.  (In any case,
                                         # the tool kit will insure that DS attributes
                                         # use the period character when streaming
                                         # to the network or to a file, regardless
                                         # of the current locale settings.)
    RETURN_COMMA_IN_DS_FL_FD_STRINGS = No # If Yes, the tool kit will return a comma
                                         # in the value when MC_Get_Value_To_String
                                         # is called for an attribute with a VR of DS, FL or FD.
                                         # If No, the value will be returned with
                                         # a period used as the radix character.
                                         # (In any case, the tool kit will properly
                                         # decode DS values regardless of the current
                                         # locale settings.)

#============================================================================
#                    MergeCOM-3 MEDIA CONFIGURATION SECTION
#============================================================================
[MEDIA_PARMS]
    EXPORT_UN_VR_TO_MEDIA       = Yes    # When set to no, do not write UN VR 
                                         # attributes when using the 
                                         # MC_Write_File() and 
                                         # MC_Write_File_By_Callback() functions
    EXPORT_PRIVATE_ATTRIBUTES_TO_MEDIA = Yes # When set to no, do not write 
                                         # private attributes when using the 
                                         # MC_Write_File() and
                                         # MC_Write_File_By_Callback() functions
                                         


#============================================================================
#                    MergeCOM-3 Transport Layer Parameters
#============================================================================
[TRANSPORT_PARMS]                  
        # Note that the listen port potentially has to be changed to a new 
        # value when this file is used by the Q/R SCU sample application where 
        # the Q/R SCP is running on the same machine.
#    TCPIP_LISTEN_PORT                       = 1115 
    TCPIP_LISTEN_PORT                       = 104 
    
    # Max number of open listen channels.  Note that this is not the 
    # "Maximum number of simmultaneous associations", but is rather
    # a TCP/IP level configuraton value used when waiting for connections. 
    MAX_PENDING_CONNECTIONS                 = 5

    # Set the send and receive OS network buffer sizes.  Note that the maximum
    # values for these constants are operating system dependant.  If the
    # values of these options are set too high, a message will be logged
    # to the tool kit's log files, although no errors will be returned 
    # through the tool kit's API.  Larger values for these constants will 
    # greatly improve network performance on networks with minimal network
    # activity.  Note that for optimum performance, these values should be at 
    # least slightly larger than the PDU_MAXIMUM_LENGTH configuration value.
    TCPIP_SEND_BUFFER_SIZE                  = 29696 
    TCPIP_RECEIVE_BUFFER_SIZE               = 29696 

    # Network Capture parameters
    NETWORK_CAPTURE=No           # Capture network data to a file that
                                 # can be examined using the MergeDPM product.
                                 # Default: No
    CAPTURE_FILE_SIZE=0          # The maximum size in kilobytes of capture files
                                 # Default: 0 (no limit to file size)
                                 # Capture stops when file would exceed nK 
                                 # (zero means no limit)
                                 # (If 1 thru 31, 32K is used)
    CAPTURE_FILE=./merge.cap     # Default: merge.cap in current directory
                                 # If NUMBER_OF_CAP_FILES > 1, nnn will be
                                 # appended to the base file name 
                                 # (e.g. merge001.cap)
    NUMBER_OF_CAP_FILES=1        # Max number of capture files to create
                                 # Default: 1
                                 # If 1, data from all associations is captured 
                                 # in the one file; if > 1 each association is
                                 # captured in its own file.
                                 # If < 1, 1 will be assumed.
                                 # (Ignored if CAPTURE_FILE_SIZE is <= 0)
    REWRITE_CAPTURE_FILES=Yes    # Default: Yes
                                 # When all capture files have been written, the 
                                 # oldest capture file will be rewritten if this is Yes,
                                 # otherwise no more data will be captured.
                                 # (Ignored if CAPTURE_FILE_SIZE is <= 0)

