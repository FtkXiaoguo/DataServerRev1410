#============================================================================
#                      MERGECOM-3 Application Configurations
#============================================================================
#
#                   ==== MergeCOM-3 Application Profiles ===
#
#      The location of this file is provided in the MERGECOM_3_APPLICATIONS
#          parameter of the [MergeCOM3] section of the MERGE.INI file
# .
#============================================================================
#
#  Contains the following sections:
#    [<remote_application_title>]- Section describing a remote DICOM application
#                                  <remote_application_title> names must be
#                                  1 to 16 bytes in length with no embedded 
#                                  spaces.  This section is only used by 
#                                  SCU applications to configure systems they 
#                                  will connect with.
#                                 
#    [<service_list_name>]      - List[s] of DICOM services (referenced by
#                                 entries in the [<remote_application_title>] 
#                                 sections.  These section names are also used
#                                 by the MC_Wait_For_Association function to
#                                 specify an SCP's services supported.
#                                 <service_list_name> names must be
#                                 1 to 33 bytes in length with no embedded 
#                                 spaces.  
#                                 
#    [<syntax_list_name>]       - List[s] of DICOM transfer syntaxes (referenced 
#                                 by optional entries in the [<service_list_name>]
#                                 sections. 
#                                 <syntax_list_name> names must be
#                                 1 to 33 bytes in length with no embedded 
#                                 spaces.
#


# Each [<remote_application_title>] section is of this format:
#       PORT_NUMBER     = <port>          is the TCP/IP port on which the 
#                                         remote DICOM system listens for 
#                                         connections.  The commonly used port 
#                                         number is 104. This default value may
#                                         be overriden by the 
#                                         MC_Open_Association() function call.
#       HOST_NAME       = <TCP/IP name>   is the name of the remote host as it
#                                         is known to your TCP/IP system.  This
#                                         default value may be overriden by 
#                                         the MC_Open_Association() function 
#                                         call.
#                                         <TCP/IP name> must be 1 to 19 bytes 
#                                         in length with no embedded spaces.
#       SERVICE_LIST    = <list name>     is the name of another section in 
#                                         this file which provides a list of 
#                                         services local applications will 
#                                         negotiate for when attempting to
#                                         establish an association.  This is a
#                                         default list; another list may be 
#                                         specified in the 
#                                         MC_Open_Association() call.
#                                         <list name> names must be 1 to 33 
#                                         bytes in length with no embedded 
#                                         spaces.
#
#[MERGE_STORE_SCP]
#    PORT_NUMBER     = 104        # port 104 is the standard DICOM port and is
#                                      priviledged
#    HOST_NAME       = myhost     # Put the remote machine name here
#    SERVICE_LIST    = Storage_SCU_Service_List
#
#[MERGE_STORE_SCU]
#    PORT_NUMBER     = 104        # port 104 is the standard DICOM port and is
#                                      priviledged.  Note that if the storage
#                                      commitment SCU and SCP are running on
#                                      the same machine, a new port number
#                                      must be placed here so both applications
#                                      can listen on different ports.
#    HOST_NAME       = myhost     # Put the remote machine name here
#    SERVICE_LIST    = Storage_Commitment_SCP_Service_List
#
#[MERGE_MEDIA_FSU]
#   PORT_NUMBER     = 104        # port 104 is the standard DICOM port and is
#                                      priviledged
#    HOST_NAME       = myhost     # Put the remote machine name here
#    SERVICE_LIST    = Storage_SCP_Service_List 
#   
#[MERGE_QR_SCP]
#    PORT_NUMBER     = 104        # port 104 is the standard DICOM port and is
#                                      priviledged
#    HOST_NAME       = myhost     # Put the remote machine name here
#    SERVICE_LIST    = Query_SCU_Service_List
#    
#[MERGE_QR_SCU]
#    PORT_NUMBER     = 1115        # port 104 is the standard DICOM port and is
#                                      priviledged.  This example has a
#                                      different port to prevent conflicts when
#                                      testing both applications on the same
#                                      machine.  Both the Q/R SCU and SCP samples
#                                      SCP samples listen for associations.
#    HOST_NAME       = myhost      # Put the remote machine name here
#    SERVICE_LIST    = Storage_SCP_Service_List    
#
#[MERGE_PRINT_SCP]
#    PORT_NUMBER     = 104        # port 104 is the standard DICOM port and is
#                                      priviledged
#    HOST_NAME       = myhost     # Put the remote machine name here
#    SERVICE_LIST    = Print_Service_List
#
#[MERGE_WORK_SCP]
#    PORT_NUMBER     = 104        # port 104 is the standard DICOM port and is
#                                      priviledged
#   HOST_NAME       = myhost     # Put the remote machine name here
#    SERVICE_LIST    = Worklist_Service_List


# --------------------------------------------------------------------
# TERARECON SERVICE LISTS
# --------------------------------------------------------------------

[PRINT_SCP]
	PORT_NUMBER = 3100
	HOST_NAME = localhost
	SERVICE_LIST = Print_Service_List

[DICOMDIR]
    SERVICES_SUPPORTED      = 1    
    SERVICE_1               = C_STORE_RQ

[TIDICOM_SCU_Service_List]
    SERVICES_SUPPORTED  = 52
    SERVICE_1		= STANDARD_CT
    SYNTAX_LIST_1       = IMPLICIT_L_E_Syntax_List

    SERVICE_2		= STANDARD_MR
    SYNTAX_LIST_2       = IMPLICIT_L_E_Syntax_List

    SERVICE_3		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3       = IMPLICIT_L_E_Syntax_List 
    SERVICE_4		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_4       = JPEG_LS_H_14_Syntax_List
    SERVICE_5		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_5       = JPEG_E_2_4_Syntax_List
    SERVICE_6		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_6       = JPEG_BL_Syntax_List

    SERVICE_7		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_7       = IMPLICIT_L_E_Syntax_List 
    SERVICE_8		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_8       = JPEG_LS_H_14_Syntax_List
    SERVICE_9		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_9       = JPEG_E_2_4_Syntax_List
    SERVICE_10		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_10      = JPEG_BL_Syntax_List

    SERVICE_11		= STANDARD_XRAY_RF
    SYNTAX_LIST_11      = IMPLICIT_L_E_Syntax_List 
    SERVICE_12		= STANDARD_XRAY_RF
    SYNTAX_LIST_12      = JPEG_LS_H_14_Syntax_List
    SERVICE_13		= STANDARD_XRAY_RF
    SYNTAX_LIST_13      = JPEG_E_2_4_Syntax_List
    SERVICE_14		= STANDARD_XRAY_RF
    SYNTAX_LIST_14      = JPEG_BL_Syntax_List

    SERVICE_15		= STANDARD_CR
    SYNTAX_LIST_15      = IMPLICIT_L_E_Syntax_List 
    SERVICE_16	      = STANDARD_CR
    SYNTAX_LIST_16      = JPEG_LS_H_14_Syntax_List
    SERVICE_17		= STANDARD_CR
    SYNTAX_LIST_17      = JPEG_E_2_4_Syntax_List
    SERVICE_18		= STANDARD_CR
    SYNTAX_LIST_18      = JPEG_BL_Syntax_List

    SERVICE_19		= STANDARD_SEC_CAPTURE
    SYNTAX_LIST_19      = IMPLICIT_L_E_Syntax_List

    SERVICE_20		= STANDARD_US
    SYNTAX_LIST_20      = IMPLICIT_L_E_Syntax_List 
    SERVICE_21		= STANDARD_US
    SYNTAX_LIST_21      = JPEG_BL_Syntax_List

    SERVICE_22		= STANDARD_US_MF
    SYNTAX_LIST_22      = IMPLICIT_L_E_Syntax_List 
    SERVICE_23		= STANDARD_US_MF
    SYNTAX_LIST_23      = JPEG_BL_Syntax_List

    SERVICE_24		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_24      = IMPLICIT_L_E_Syntax_List 
    SERVICE_25		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_25      = JPEG_BL_Syntax_List

    SERVICE_26		= STANDARD_US_RETIRED
    SYNTAX_LIST_26      = IMPLICIT_L_E_Syntax_List 
    SERVICE_27		= STANDARD_US_RETIRED
    SYNTAX_LIST_27      = JPEG_BL_Syntax_List

    SERVICE_28		= STANDARD_DX_PROCESS
    SYNTAX_LIST_28      = IMPLICIT_L_E_Syntax_List 
    SERVICE_29		= STANDARD_DX_PROCESS
    SYNTAX_LIST_29      = JPEG_BL_Syntax_List

    SERVICE_30		= STANDARD_ECHO
    SYNTAX_LIST_30      = IMPLICIT_L_E_Syntax_List
    SERVICE_31		= STUDY_ROOT_QR_FIND
    SYNTAX_LIST_31      = IMPLICIT_L_E_Syntax_List
    SERVICE_32		= STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_32      = IMPLICIT_L_E_Syntax_List
    SERVICE_33		= PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_33      = IMPLICIT_L_E_Syntax_List
    SERVICE_34		= PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_34      = IMPLICIT_L_E_Syntax_List

    SERVICE_35		= STANDARD_CT
    SYNTAX_LIST_35      = JPEG_LS_H_14_Syntax_List
    SERVICE_36		= STANDARD_CT
    SYNTAX_LIST_36      = JPEG_E_2_4_Syntax_List
    SERVICE_37		= STANDARD_CT
    SYNTAX_LIST_37      = JPEG_BL_Syntax_List

    SERVICE_38		= STANDARD_MR
    SYNTAX_LIST_38      = JPEG_LS_H_14_Syntax_List
    SERVICE_39		= STANDARD_MR
    SYNTAX_LIST_39      = JPEG_E_2_4_Syntax_List
    SERVICE_40		= STANDARD_MR
    SYNTAX_LIST_40      = JPEG_BL_Syntax_List

    SERVICE_41		= STANDARD_US
    SYNTAX_LIST_41      = RLE_Syntax_List
    SERVICE_42		= STANDARD_US_MF
    SYNTAX_LIST_42      = RLE_Syntax_List 
    SERVICE_43		= STANDARD_US_RETIRED
    SYNTAX_LIST_43      = RLE_Syntax_List 
    SERVICE_44		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_44      = RLE_Syntax_List

    SERVICE_45		  = STANDARD_NM
    SYNTAX_LIST_45	  = Uncompressed_Syntax_List
    SERVICE_46		  = STANDARD_PET
    SYNTAX_LIST_46	  = Uncompressed_Syntax_List

    SERVICE_47		= STANDARD_DX_PRESENT
    SYNTAX_LIST_47      = IMPLICIT_L_E_Syntax_List 
    SERVICE_48		= STANDARD_DX_PRESENT
    SYNTAX_LIST_48      = JPEG_BL_Syntax_List

    SERVICE_49		= STANDARD_CT
    SYNTAX_LIST_49      = J2K_LOSSLESS_Syntax_List
    SERVICE_50		= STANDARD_CT
    SYNTAX_LIST_50      = J2K_LOSSY_Syntax_List

    SERVICE_51		= STANDARD_MR
    SYNTAX_LIST_51      = J2K_LOSSLESS_Syntax_List
    SERVICE_52		= STANDARD_MR
    SYNTAX_LIST_52      = J2K_LOSSY_Syntax_List

[TIDICOM_SCU_Implicit_LE_Service_List]
    SERVICES_SUPPORTED  = 42
    SERVICE_1		= STANDARD_CT
    SYNTAX_LIST_1       = IMPLICIT_L_E_Syntax_List

    SERVICE_2		= STANDARD_MR
    SYNTAX_LIST_2       = IMPLICIT_L_E_Syntax_List

    SERVICE_3		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3       = IMPLICIT_L_E_Syntax_List 
    SERVICE_4		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_4       = JPEG_LS_H_14_Syntax_List
    SERVICE_5		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_5       = JPEG_E_2_4_Syntax_List
    SERVICE_6		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_6       = JPEG_BL_Syntax_List

    SERVICE_7		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_7       = IMPLICIT_L_E_Syntax_List 
    SERVICE_8		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_8       = JPEG_LS_H_14_Syntax_List
    SERVICE_9		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_9       = JPEG_E_2_4_Syntax_List
    SERVICE_10		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_10      = JPEG_BL_Syntax_List

    SERVICE_11		= STANDARD_XRAY_RF
    SYNTAX_LIST_11      = IMPLICIT_L_E_Syntax_List 
    SERVICE_12		= STANDARD_XRAY_RF
    SYNTAX_LIST_12      = JPEG_LS_H_14_Syntax_List
    SERVICE_13		= STANDARD_XRAY_RF
    SYNTAX_LIST_13      = JPEG_E_2_4_Syntax_List
    SERVICE_14		= STANDARD_XRAY_RF
    SYNTAX_LIST_14      = JPEG_BL_Syntax_List

    SERVICE_15		= STANDARD_CR
    SYNTAX_LIST_15      = IMPLICIT_L_E_Syntax_List 
    SERVICE_16	      = STANDARD_CR
    SYNTAX_LIST_16      = JPEG_LS_H_14_Syntax_List
    SERVICE_17		= STANDARD_CR
    SYNTAX_LIST_17      = JPEG_E_2_4_Syntax_List
    SERVICE_18		= STANDARD_CR
    SYNTAX_LIST_18      = JPEG_BL_Syntax_List

    SERVICE_19		= STANDARD_SEC_CAPTURE
    SYNTAX_LIST_19      = IMPLICIT_L_E_Syntax_List

    SERVICE_20		= STANDARD_US
    SYNTAX_LIST_20      = IMPLICIT_L_E_Syntax_List 
    SERVICE_21		= STANDARD_US
    SYNTAX_LIST_21      = JPEG_BL_Syntax_List

    SERVICE_22		= STANDARD_US_MF
    SYNTAX_LIST_22      = IMPLICIT_L_E_Syntax_List 
    SERVICE_23		= STANDARD_US_MF
    SYNTAX_LIST_23      = JPEG_BL_Syntax_List

    SERVICE_24		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_24      = IMPLICIT_L_E_Syntax_List 
    SERVICE_25		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_25      = JPEG_BL_Syntax_List

    SERVICE_26		= STANDARD_US_RETIRED
    SYNTAX_LIST_26      = IMPLICIT_L_E_Syntax_List 
    SERVICE_27		= STANDARD_US_RETIRED
    SYNTAX_LIST_27      = JPEG_BL_Syntax_List

    SERVICE_28		= STANDARD_DX_PROCESS
    SYNTAX_LIST_28      = IMPLICIT_L_E_Syntax_List 
    SERVICE_29		= STANDARD_DX_PROCESS
    SYNTAX_LIST_29      = JPEG_BL_Syntax_List

    SERVICE_30		= STANDARD_ECHO
    SYNTAX_LIST_30      = IMPLICIT_L_E_Syntax_List
    SERVICE_31		= STUDY_ROOT_QR_FIND
    SYNTAX_LIST_31      = IMPLICIT_L_E_Syntax_List
    SERVICE_32		= STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_32      = IMPLICIT_L_E_Syntax_List
    SERVICE_33		= PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_33      = IMPLICIT_L_E_Syntax_List
    SERVICE_34		= PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_34      = IMPLICIT_L_E_Syntax_List

    SERVICE_35		= STANDARD_US
    SYNTAX_LIST_35      = RLE_Syntax_List
    SERVICE_36		= STANDARD_US_MF
    SYNTAX_LIST_36      = RLE_Syntax_List 
    SERVICE_37		= STANDARD_US_RETIRED
    SYNTAX_LIST_37      = RLE_Syntax_List 
    SERVICE_38		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_38      = RLE_Syntax_List

    SERVICE_39		  = STANDARD_NM
    SYNTAX_LIST_39	  = IMPLICIT_L_E_Syntax_List 
    SERVICE_40		  = STANDARD_PET
    SYNTAX_LIST_40	  = IMPLICIT_L_E_Syntax_List 

    SERVICE_41		= STANDARD_DX_PRESENT
    SYNTAX_LIST_41      = IMPLICIT_L_E_Syntax_List 
    SERVICE_42		= STANDARD_DX_PRESENT
    SYNTAX_LIST_42      = JPEG_BL_Syntax_List

[TIDICOM_SCU_Explicit_LE_Service_List]
    SERVICES_SUPPORTED  = 42
    SERVICE_1		= STANDARD_CT
    SYNTAX_LIST_1       = EXPLICIT_L_E_Syntax_List

    SERVICE_2		= STANDARD_MR
    SYNTAX_LIST_2       = EXPLICIT_L_E_Syntax_List

    SERVICE_3		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3       = EXPLICIT_L_E_Syntax_List
    SERVICE_4		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_4       = JPEG_LS_H_14_Syntax_List
    SERVICE_5		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_5       = JPEG_E_2_4_Syntax_List
    SERVICE_6		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_6       = JPEG_BL_Syntax_List

    SERVICE_7		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_7       = EXPLICIT_L_E_Syntax_List 
    SERVICE_8		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_8       = JPEG_LS_H_14_Syntax_List
    SERVICE_9		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_9       = JPEG_E_2_4_Syntax_List
    SERVICE_10		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_10      = JPEG_BL_Syntax_List

    SERVICE_11		= STANDARD_XRAY_RF
    SYNTAX_LIST_11      = EXPLICIT_L_E_Syntax_List 
    SERVICE_12		= STANDARD_XRAY_RF
    SYNTAX_LIST_12      = JPEG_LS_H_14_Syntax_List
    SERVICE_13		= STANDARD_XRAY_RF
    SYNTAX_LIST_13      = JPEG_E_2_4_Syntax_List
    SERVICE_14		= STANDARD_XRAY_RF
    SYNTAX_LIST_14      = JPEG_BL_Syntax_List

    SERVICE_15		= STANDARD_CR
    SYNTAX_LIST_15      = EXPLICIT_L_E_Syntax_List 
    SERVICE_16	      = STANDARD_CR
    SYNTAX_LIST_16      = JPEG_LS_H_14_Syntax_List
    SERVICE_17		= STANDARD_CR
    SYNTAX_LIST_17      = JPEG_E_2_4_Syntax_List
    SERVICE_18		= STANDARD_CR
    SYNTAX_LIST_18      = JPEG_BL_Syntax_List

    SERVICE_19		= STANDARD_SEC_CAPTURE
    SYNTAX_LIST_19      = EXPLICIT_L_E_Syntax_List

    SERVICE_20		= STANDARD_US
    SYNTAX_LIST_20      = EXPLICIT_L_E_Syntax_List 
    SERVICE_21		= STANDARD_US
    SYNTAX_LIST_21      = JPEG_BL_Syntax_List

    SERVICE_22		= STANDARD_US_MF
    SYNTAX_LIST_22      = EXPLICIT_L_E_Syntax_List 
    SERVICE_23		= STANDARD_US_MF
    SYNTAX_LIST_23      = JPEG_BL_Syntax_List

    SERVICE_24		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_24      = EXPLICIT_L_E_Syntax_List 
    SERVICE_25		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_25      = JPEG_BL_Syntax_List

    SERVICE_26		= STANDARD_US_RETIRED
    SYNTAX_LIST_26      = EXPLICIT_L_E_Syntax_List 
    SERVICE_27		= STANDARD_US_RETIRED
    SYNTAX_LIST_27      = JPEG_BL_Syntax_List

    SERVICE_28		= STANDARD_DX_PROCESS
    SYNTAX_LIST_28      = EXPLICIT_L_E_Syntax_List 
    SERVICE_29		= STANDARD_DX_PROCESS
    SYNTAX_LIST_29      = JPEG_BL_Syntax_List

    SERVICE_30		= STANDARD_ECHO
    SYNTAX_LIST_30      = EXPLICIT_L_E_Syntax_List
    SERVICE_31		= STUDY_ROOT_QR_FIND
    SYNTAX_LIST_31      = EXPLICIT_L_E_Syntax_List
    SERVICE_32		= STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_32      = EXPLICIT_L_E_Syntax_List
    SERVICE_33		= PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_33      = EXPLICIT_L_E_Syntax_List
    SERVICE_34		= PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_34      = EXPLICIT_L_E_Syntax_List

    SERVICE_35		= STANDARD_US
    SYNTAX_LIST_35      = RLE_Syntax_List
    SERVICE_36		= STANDARD_US_MF
    SYNTAX_LIST_36      = RLE_Syntax_List 
    SERVICE_37		= STANDARD_US_RETIRED
    SYNTAX_LIST_37      = RLE_Syntax_List 
    SERVICE_38		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_38      = RLE_Syntax_List

    SERVICE_39		  = STANDARD_NM
    SYNTAX_LIST_39	  = EXPLICIT_L_E_Syntax_List 
    SERVICE_40		  = STANDARD_PET
    SYNTAX_LIST_40	  = EXPLICIT_L_E_Syntax_List 

    SERVICE_41		= STANDARD_DX_PRESENT
    SYNTAX_LIST_41      = EXPLICIT_L_E_Syntax_List
    SERVICE_42		= STANDARD_DX_PRESENT
    SYNTAX_LIST_42      = JPEG_BL_Syntax_List

[TIDICOM_SCU_Explicit_BE_Service_List]
    SERVICES_SUPPORTED  = 42
    SERVICE_1		= STANDARD_CT
    SYNTAX_LIST_1       = EXPLICIT_B_E_Syntax_List

    SERVICE_2		= STANDARD_MR
    SYNTAX_LIST_2       = EXPLICIT_B_E_Syntax_List

    SERVICE_3		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3       = EXPLICIT_B_E_Syntax_List 
    SERVICE_4		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_4       = JPEG_LS_H_14_Syntax_List
    SERVICE_5		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_5       = JPEG_E_2_4_Syntax_List
    SERVICE_6		= STANDARD_XRAY_ANGIO
    SYNTAX_LIST_6       = JPEG_BL_Syntax_List

    SERVICE_7		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_7       = EXPLICIT_B_E_Syntax_List 
    SERVICE_8		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_8       = JPEG_LS_H_14_Syntax_List
    SERVICE_9		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_9       = JPEG_E_2_4_Syntax_List
    SERVICE_10		= STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_10      = JPEG_BL_Syntax_List

    SERVICE_11		= STANDARD_XRAY_RF
    SYNTAX_LIST_11      = EXPLICIT_B_E_Syntax_List 
    SERVICE_12		= STANDARD_XRAY_RF
    SYNTAX_LIST_12      = JPEG_LS_H_14_Syntax_List
    SERVICE_13		= STANDARD_XRAY_RF
    SYNTAX_LIST_13      = JPEG_E_2_4_Syntax_List
    SERVICE_14		= STANDARD_XRAY_RF
    SYNTAX_LIST_14      = JPEG_BL_Syntax_List

    SERVICE_15		= STANDARD_CR
    SYNTAX_LIST_15      = EXPLICIT_B_E_Syntax_List 
    SERVICE_16	      = STANDARD_CR
    SYNTAX_LIST_16      = JPEG_LS_H_14_Syntax_List
    SERVICE_17		= STANDARD_CR
    SYNTAX_LIST_17      = JPEG_E_2_4_Syntax_List
    SERVICE_18		= STANDARD_CR
    SYNTAX_LIST_18      = JPEG_BL_Syntax_List

    SERVICE_19		= STANDARD_SEC_CAPTURE
    SYNTAX_LIST_19      = EXPLICIT_B_E_Syntax_List

    SERVICE_20		= STANDARD_US
    SYNTAX_LIST_20      = EXPLICIT_B_E_Syntax_List 
    SERVICE_21		= STANDARD_US
    SYNTAX_LIST_21      = JPEG_BL_Syntax_List

    SERVICE_22		= STANDARD_US_MF
    SYNTAX_LIST_22      = EXPLICIT_B_E_Syntax_List 
    SERVICE_23		= STANDARD_US_MF
    SYNTAX_LIST_23      = JPEG_BL_Syntax_List

    SERVICE_24		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_24      = EXPLICIT_B_E_Syntax_List 
    SERVICE_25		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_25      = JPEG_BL_Syntax_List

    SERVICE_26		= STANDARD_US_RETIRED
    SYNTAX_LIST_26      = EXPLICIT_B_E_Syntax_List 
    SERVICE_27		= STANDARD_US_RETIRED
    SYNTAX_LIST_27      = JPEG_BL_Syntax_List

    SERVICE_28		= STANDARD_DX_PROCESS
    SYNTAX_LIST_28      = EXPLICIT_B_E_Syntax_List 
    SERVICE_29		= STANDARD_DX_PROCESS
    SYNTAX_LIST_29      = JPEG_BL_Syntax_List

    SERVICE_30		= STANDARD_ECHO
    SYNTAX_LIST_30      = EXPLICIT_B_E_Syntax_List
    SERVICE_31		= STUDY_ROOT_QR_FIND
    SYNTAX_LIST_31      = EXPLICIT_B_E_Syntax_List
    SERVICE_32		= STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_32      = EXPLICIT_B_E_Syntax_List
    SERVICE_33		= PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_33      = EXPLICIT_B_E_Syntax_List
    SERVICE_34		= PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_34      = EXPLICIT_B_E_Syntax_List

    SERVICE_35		= STANDARD_US
    SYNTAX_LIST_35      = RLE_Syntax_List
    SERVICE_36		= STANDARD_US_MF
    SYNTAX_LIST_36      = RLE_Syntax_List 
    SERVICE_37		= STANDARD_US_RETIRED
    SYNTAX_LIST_37      = RLE_Syntax_List 
    SERVICE_38		= STANDARD_US_MF_RETIRED
    SYNTAX_LIST_38      = RLE_Syntax_List

    SERVICE_39		  = STANDARD_NM
    SYNTAX_LIST_39	  = EXPLICIT_B_E_Syntax_List 
    SERVICE_40		  = STANDARD_PET
    SYNTAX_LIST_40	  = EXPLICIT_B_E_Syntax_List 

    SERVICE_41		= STANDARD_DX_PRESENT
    SYNTAX_LIST_41      = EXPLICIT_B_E_Syntax_List
    SERVICE_42		= STANDARD_DX_PRESENT
    SYNTAX_LIST_42      = JPEG_BL_Syntax_List

[TIDICOMServer_Service_List]
    SERVICES_SUPPORTED	  = 20
    SERVICE_1		  = STANDARD_CT
    SYNTAX_LIST_1         = Full_Syntax_List
    SERVICE_2		  = STANDARD_MR
    SYNTAX_LIST_2         = Full_Syntax_List
    SERVICE_3             = STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3         = Full_Syntax_List
    SERVICE_4             = STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_4         = Full_Syntax_List
    SERVICE_5             = STANDARD_XRAY_RF
    SYNTAX_LIST_5         = Full_Syntax_List
    SERVICE_6             = STANDARD_CR
    SYNTAX_LIST_6         = Full_Syntax_List
    SERVICE_7		  = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_7         = Full_Syntax_List
    SERVICE_8		  = STANDARD_US
    SYNTAX_LIST_8	        = Full_Syntax_List
    SERVICE_9		  = STANDARD_US_MF
    SYNTAX_LIST_9	        = Full_Syntax_List
    SERVICE_10		  = STANDARD_US_MF_RETIRED
    SYNTAX_LIST_10	  = Full_Syntax_List
    SERVICE_11		  = STANDARD_US_RETIRED
    SYNTAX_LIST_11	  = Full_Syntax_List
    SERVICE_12		  = STANDARD_NM
    SYNTAX_LIST_12	  = Uncompressed_Syntax_List
    SERVICE_13		  = STANDARD_PET
    SYNTAX_LIST_13	  = Uncompressed_Syntax_List
    SERVICE_14		  = STANDARD_ECHO
    SYNTAX_LIST_14	  = Uncompressed_Syntax_List
    SERVICE_15            = STUDY_ROOT_QR_FIND
    SYNTAX_LIST_15        = Uncompressed_Syntax_List
    SERVICE_16            = STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_16        = Uncompressed_Syntax_List
    SERVICE_17            = PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_17        = Uncompressed_Syntax_List
    SERVICE_18            = PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_18        = Uncompressed_Syntax_List
    SERVICE_19            = STANDARD_DX_PRESENT
    SYNTAX_LIST_19        = Full_Syntax_List
    SERVICE_20            = STANDARD_DX_PROCESS
    SYNTAX_LIST_20        = Full_Syntax_List

[TIDICOMServer_Implicit_LE_Service_List]
    SERVICES_SUPPORTED	  = 20
    SERVICE_1		  = STANDARD_CT
    SYNTAX_LIST_1         = IMPLICIT_L_E_Syntax_List
    SERVICE_2		  = STANDARD_MR
    SYNTAX_LIST_2         = IMPLICIT_L_E_Syntax_List
    SERVICE_3             = STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3         = Full_Syntax_List
    SERVICE_4             = STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_4         = Full_Syntax_List
    SERVICE_5             = STANDARD_XRAY_RF
    SYNTAX_LIST_5         = Full_Syntax_List
    SERVICE_6             = STANDARD_CR
    SYNTAX_LIST_6         = IMPLICIT_L_E_Syntax_List
    SERVICE_7		  = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_7         = IMPLICIT_L_E_Syntax_List
    SERVICE_8		  = STANDARD_US
    SYNTAX_LIST_8	        = Full_Syntax_List
    SERVICE_9		  = STANDARD_US_MF
    SYNTAX_LIST_9	        = Full_Syntax_List
    SERVICE_10		  = STANDARD_US_MF_RETIRED
    SYNTAX_LIST_10	  = Full_Syntax_List
    SERVICE_11		  = STANDARD_US_RETIRED
    SYNTAX_LIST_11	  = Full_Syntax_List
    SERVICE_12		  = STANDARD_NM
    SYNTAX_LIST_12	  = IMPLICIT_L_E_Syntax_List
    SERVICE_13		  = STANDARD_PET
    SYNTAX_LIST_13	  = IMPLICIT_L_E_Syntax_List
    SERVICE_14		  = STANDARD_ECHO
    SYNTAX_LIST_14	  = Uncompressed_Syntax_List
    SERVICE_15            = STUDY_ROOT_QR_FIND
    SYNTAX_LIST_15        = Uncompressed_Syntax_List
    SERVICE_16            = STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_16        = Uncompressed_Syntax_List
    SERVICE_17            = PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_17        = Uncompressed_Syntax_List
    SERVICE_18            = PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_18        = Uncompressed_Syntax_List
    SERVICE_19            = STANDARD_DX_PRESENT
    SYNTAX_LIST_19        = IMPLICIT_L_E_Syntax_List
    SERVICE_20            = STANDARD_DX_PROCESS
    SYNTAX_LIST_20        = IMPLICIT_L_E_Syntax_List

[TIDICOMServer_Explicit_LE_Service_List]
    SERVICES_SUPPORTED	  = 20
    SERVICE_1		  = STANDARD_CT
    SYNTAX_LIST_1         = EXPLICIT_L_E_Syntax_List
    SERVICE_2		  = STANDARD_MR
    SYNTAX_LIST_2         = EXPLICIT_L_E_Syntax_List
    SERVICE_3             = STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3         = Full_Syntax_List
    SERVICE_4             = STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_4         = Full_Syntax_List
    SERVICE_5             = STANDARD_XRAY_RF
    SYNTAX_LIST_5         = Full_Syntax_List
    SERVICE_6             = STANDARD_CR
    SYNTAX_LIST_6         = EXPLICIT_L_E_Syntax_List
    SERVICE_7		  = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_7         = EXPLICIT_L_E_Syntax_List
    SERVICE_8		  = STANDARD_US
    SYNTAX_LIST_8	        = Full_Syntax_List
    SERVICE_9		  = STANDARD_US_MF
    SYNTAX_LIST_9	        = Full_Syntax_List
    SERVICE_10		  = STANDARD_US_MF_RETIRED
    SYNTAX_LIST_10	  = Full_Syntax_List
    SERVICE_11		  = STANDARD_US_RETIRED
    SYNTAX_LIST_11	  = Full_Syntax_List
    SERVICE_12		  = STANDARD_NM
    SYNTAX_LIST_12	  = EXPLICIT_L_E_Syntax_List
    SERVICE_13		  = STANDARD_PET
    SYNTAX_LIST_13	  = EXPLICIT_L_E_Syntax_List
    SERVICE_14		  = STANDARD_ECHO
    SYNTAX_LIST_14	  = Uncompressed_Syntax_List
    SERVICE_15            = STUDY_ROOT_QR_FIND
    SYNTAX_LIST_15        = Uncompressed_Syntax_List
    SERVICE_16            = STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_16        = Uncompressed_Syntax_List
    SERVICE_17            = PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_17        = Uncompressed_Syntax_List
    SERVICE_18            = PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_18        = Uncompressed_Syntax_List
    SERVICE_19            = STANDARD_DX_PRESENT
    SYNTAX_LIST_19        = EXPLICIT_L_E_Syntax_List
    SERVICE_20            = STANDARD_DX_PROCESS
    SYNTAX_LIST_20        = EXPLICIT_L_E_Syntax_List

[TIDICOMServer_Explicit_BE_Service_List]
    SERVICES_SUPPORTED	  = 20
    SERVICE_1		  = STANDARD_CT
    SYNTAX_LIST_1         = EXPLICIT_B_E_Syntax_List
    SERVICE_2		  = STANDARD_MR
    SYNTAX_LIST_2         = EXPLICIT_B_E_Syntax_List
    SERVICE_3             = STANDARD_XRAY_ANGIO
    SYNTAX_LIST_3         = Full_Syntax_List
    SERVICE_4             = STANDARD_XRAY_ANGIO_BIPLANE
    SYNTAX_LIST_4         = Full_Syntax_List
    SERVICE_5             = STANDARD_XRAY_RF
    SYNTAX_LIST_5         = Full_Syntax_List
    SERVICE_6             = STANDARD_CR
    SYNTAX_LIST_6         = EXPLICIT_B_E_Syntax_List
    SERVICE_7		  = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_7         = EXPLICIT_B_E_Syntax_List
    SERVICE_8		  = STANDARD_US
    SYNTAX_LIST_8	        = Full_Syntax_List
    SERVICE_9		  = STANDARD_US_MF
    SYNTAX_LIST_9	        = Full_Syntax_List
    SERVICE_10		  = STANDARD_US_MF_RETIRED
    SYNTAX_LIST_10	  = Full_Syntax_List
    SERVICE_11		  = STANDARD_US_RETIRED
    SYNTAX_LIST_11	  = Full_Syntax_List
    SERVICE_12		  = STANDARD_NM
    SYNTAX_LIST_12	  = EXPLICIT_B_E_Syntax_List
    SERVICE_13		  = STANDARD_PET
    SYNTAX_LIST_13	  = EXPLICIT_B_E_Syntax_List
    SERVICE_14		  = STANDARD_ECHO
    SYNTAX_LIST_14	  = Uncompressed_Syntax_List
    SERVICE_15            = STUDY_ROOT_QR_FIND
    SYNTAX_LIST_15        = Uncompressed_Syntax_List
    SERVICE_16            = STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_16        = Uncompressed_Syntax_List
    SERVICE_17            = PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_17        = Uncompressed_Syntax_List
    SERVICE_18            = PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_18        = Uncompressed_Syntax_List
    SERVICE_19            = STANDARD_DX_PRESENT
    SYNTAX_LIST_19        = EXPLICIT_B_E_Syntax_List
    SERVICE_20            = STANDARD_DX_PROCESS
    SYNTAX_LIST_20        = EXPLICIT_B_E_Syntax_List

[AqNETSonic_Service_List]
    SERVICES_SUPPORTED	  = 10
    SERVICE_1		  = STANDARD_US
    SYNTAX_LIST_1	        = Full_Syntax_List
    SERVICE_2		  = STANDARD_US_MF
    SYNTAX_LIST_2	        = Full_Syntax_List
    SERVICE_3		  = STANDARD_US_MF_RETIRED
    SYNTAX_LIST_3	 	  = Full_Syntax_List
    SERVICE_4		  = STANDARD_US_RETIRED
    SYNTAX_LIST_4	  	  = Full_Syntax_List
    SERVICE_5		  = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_5	  	  = Full_Syntax_List
    SERVICE_6		  = STANDARD_ECHO
    SYNTAX_LIST_6	  	  = Uncompressed_Syntax_List
    SERVICE_7             = STUDY_ROOT_QR_FIND
    SYNTAX_LIST_7         = Uncompressed_Syntax_List
    SERVICE_8             = STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_8         = Uncompressed_Syntax_List
    SERVICE_9             = PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_9         = Uncompressed_Syntax_List
    SERVICE_10            = PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_10        = Uncompressed_Syntax_List

# --------------------------------------------------------------------
# END OF TERARECON SERVICE LISTS
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# TERARECON SYNTAX LISTS
# --------------------------------------------------------------------


[Full_Syntax_List]
    SYNTAXES_SUPPORTED      = 10       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_LOSSLESS_HIER_14 
    SYNTAX_2                = JPEG_BASELINE   
    SYNTAX_3                = JPEG_EXTENDED_2_4 
    SYNTAX_4		    = JPEG_LOSSLESS_NON_HIER_14
    SYNTAX_5		    = JPEG_2000_LOSSLESS_ONLY
    SYNTAX_6		    = JPEG_2000
    SYNTAX_7		    = RLE   
    SYNTAX_8                = IMPLICIT_LITTLE_ENDIAN 
    SYNTAX_9                = EXPLICIT_LITTLE_ENDIAN 
    SYNTAX_10               = EXPLICIT_BIG_ENDIAN 

[Baseline_Syntax_List]
    SYNTAXES_SUPPORTED      = 4       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_BASELINE 
    SYNTAX_2                = IMPLICIT_LITTLE_ENDIAN
    SYNTAX_3                = EXPLICIT_LITTLE_ENDIAN 
    SYNTAX_4                = EXPLICIT_BIG_ENDIAN

[Uncompressed_Syntax_List]
    SYNTAXES_SUPPORTED      = 3       # Number of Syntaxes in list
    SYNTAX_1                = IMPLICIT_LITTLE_ENDIAN 
    SYNTAX_2                = EXPLICIT_LITTLE_ENDIAN
    SYNTAX_3                = EXPLICIT_BIG_ENDIAN

# The following two syntax lists are supplied as an example of how to 
# define syntax lists for compressed transfer syntaxes.  Only the default
# lossless and lossy compression syntaxes have been included.
[Compressed_Syntax_List]
    SYNTAXES_SUPPORTED      = 6       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_LOSSLESS_HIER_14
    SYNTAX_2                = JPEG_BASELINE  
    SYNTAX_3                = JPEG_EXTENDED_2_4
    SYNTAX_4		    = JPEG_2000_LOSSLESS_ONLY
    SYNTAX_5		    = JPEG_2000
    SYNTAX_6		    = RLE
    
    
# The Following is for the AQNetDICOMSCU we need to list all of these
# Multiple times for each Transfer Syntax that is handled by the SCU

[EXPLICIT_L_E_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = EXPLICIT_LITTLE_ENDIAN

[IMPLICIT_L_E_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = IMPLICIT_LITTLE_ENDIAN

[EXPLICIT_B_E_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = EXPLICIT_BIG_ENDIAN


[JPEG_BL_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_BASELINE

[JPEG_LS_H_14_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_LOSSLESS_HIER_14

[JPEG_E_2_4_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_EXTENDED_2_4

[RLE_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = RLE

[JPEG_LS_N_H_14_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_LOSSLESS_NON_HIER_14

[J2K_LOSSLESS_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1		    = JPEG_2000_LOSSLESS_ONLY

[J2K_LOSSY_Syntax_List]
    SYNTAXES_SUPPORTED      = 1       # Number of Syntaxes in list
    SYNTAX_1		    = JPEG_2000

# --------------------------------------------------------------------
# END OF TERARECON SYNTAX LISTS
# --------------------------------------------------------------------

# --------------------------------------------------------------------
# BELOW THIS LINE IS THE SET OF EXAMPLES THAT CAME WITH MERGE
# --------------------------------------------------------------------




# Each [<service_list_name>] section must contain a SERVICES_SUPPORTED
#       parameter to specify the number of services in the list.  Optionally,
#       if an application supports DICOM asynchronous communications, the 
#       maximum operations invoked and performed by the application can be
#       specified in the service list.  MAX_OPERATIONS_INVOKED specifies
#       the maximum number of requests messages an application will send
#       before processing a response message.  MAX_OPERATIONS_PERFORMED
#       specifies how many requests messages an application can recieve
#       before sending a response message.  Setting a value of '0' means
#       unlimited requests can be invoked or performed.  Leaving these 
#       options out of the service list will cause this not to be negotiated
#       and synchronous communication will be used over the association.
#
#       MAX_OPERATIONS_INVOKED = 10
#       MAX_OPERATIONS_PERFORMED = 10
#
#       Finally, a service list has four parameters for each service, of 
#       the following format:
#
#       SERVICE_n = <service name>
#       SYNTAX_LIST_n = <syntax_list_name>  (this parameter is optional)
#       ROLE_n = <role type>
#       EXT_NEG_INFO_n = <neg info>
#
#       where:
#               SERVICE_n       the character string "SERVICE_n", with n 
#                               replaced by the number of the service being 
#                               named.
#               ROLE_n          the character string "ROLE_n", with n replaced
#                               by the number of the role being set.  The only
#                               values which may be set are SCU, SCP, or BOTH.
#                               This section is optional.  Not specifying a
#                               role will default to a requestor = SCU, and
#                               acceptor = SCP.
#               SYNTAX_LIST_n   the character string "SYNTAX_LIST_n", with n 
#                               replaced by the number of the service for 
#                               which a list of transfer syntaxes to support
#                               is specified.
#               EXT_NEG_INFO_n  the character string "EXT_NEG_INFO_n", with n 
#                               replaced by the number of the service for 
#                               which extended negotiation is specified.  Note
#                               that this extended negotation information is 
#                               only used by the SCU.
#               <service name>  is the name of one of the services supported by
#                               your MergeCOM-3 system.  It must match one of 
#                               the service names described in the MergeCOM-3 
#                               Services Profile (named by the 
#                               MERGECOM_3_SERVICES parameter in the merge.ini 
#                               file).
#               <syntax_list_name>  is the name of a section that contains a 
#                               list of transfer syntaxes to support.  This
#                               parameter is optional.
#               <neg info>      is a string of hex characters signaling the 
#                               DICOM extended negotiation info to be used by
#                               an SCU when proposing an association.  The 
#                               string would be of the format 0x01 0x00 0x01, 
#                               etc.
#
# If a transfer syntax list is not specified, the tool kit will use the three
# non-encapsulated transfer syntaxes defined in the mergecom.pro configuration
# file (implicit VR little endian, explicit VR little endian and explicit VR big
# endian).


# Service list used by the Storage and Storage Commitment SCP application.
# This is a seperate service list because the SCU does not want to include
# the storage commitment push service class its association request.
[Storage_SCP_Service_List]
    SERVICES_SUPPORTED      = 69       # Number of Services in list
    SERVICE_1               = STORAGE_COMMITMENT_PUSH
    SERVICE_2               = CHEST_CAD_SR
    SERVICE_3               = ENHANCED_CT_IMAGE
    SERVICE_4               = ENHANCED_MR_IMAGE
    SERVICE_5               = KEY_OBJECT_SELECTION_DOC
    SERVICE_6               = MAMMOGRAPHY_CAD_SR
    SERVICE_7               = MR_SPECTROSCOPY
    SERVICE_8               = PROCEDURE_LOG
    SERVICE_9               = SC_MULTIFRAME_GRAYSCALE_BYTE    
    SERVICE_10              = SC_MULTIFRAME_GRAYSCALE_WORD
    SERVICE_11              = SC_MULTIFRAME_SINGLE_BIT
    SERVICE_12              = SC_MULTIFRAME_TRUE_COLOR
    SERVICE_13              = SPATIAL_FIDUCIALS
    SERVICE_14              = SPATIAL_REGISTRATION
    SERVICE_15              = STANDARD_BASIC_TEXT_SR
    SERVICE_16              = STANDARD_COMPREHENSIVE_SR
    SERVICE_17              = STANDARD_CR
    SERVICE_18              = STANDARD_CT
    SYNTAX_LIST_18          = Uncompressed_Syntax_List    # This is provided as an example and can be removed
    SERVICE_19              = STANDARD_CURVE
    SERVICE_20              = STANDARD_DX_PRESENT
    SERVICE_21              = STANDARD_DX_PROCESS
    SERVICE_22              = STANDARD_ENHANCED_SR
    SERVICE_23              = STANDARD_GRAYSCALE_SOFTCOPY_PS
    SERVICE_24              = STANDARD_HARDCOPY_COLOR
    SERVICE_25              = STANDARD_HARDCOPY_GRAYSCALE
    SERVICE_26              = STANDARD_IO_PRESENT
    SERVICE_27              = STANDARD_IO_PROCESS
    SERVICE_28              = STANDARD_MG_PRESENT
    SERVICE_29              = STANDARD_MG_PROCESS
    SERVICE_30              = STANDARD_MODALITY_LUT
    SERVICE_31              = STANDARD_MR
    SERVICE_32              = STANDARD_NM
    SERVICE_33              = STANDARD_NM_RETIRED
    SERVICE_34              = STANDARD_OPHTHALMIC_16_BIT
    SERVICE_35              = STANDARD_OPHTHALMIC_8_BIT
    SERVICE_36              = STANDARD_OVERLAY
    SERVICE_37              = STANDARD_PET
    SERVICE_38              = STANDARD_PET_CURVE
    SERVICE_39              = STANDARD_PRINT_STORAGE
    SERVICE_40              = STANDARD_RT_BEAMS_TREAT
    SERVICE_41              = STANDARD_RT_BRACHY_TREAT
    SERVICE_42              = STANDARD_RT_DOSE
    SERVICE_43              = STANDARD_RT_IMAGE
    SERVICE_44              = STANDARD_RT_PLAN
    SERVICE_45              = STANDARD_RT_STRUCTURE_SET
    SERVICE_46              = STANDARD_RT_TREAT_SUM
    SERVICE_47              = STANDARD_SEC_CAPTURE
    SERVICE_48              = STANDARD_US
    SERVICE_49              = STANDARD_US_MF
    SERVICE_50              = STANDARD_US_MF_RETIRED
    SERVICE_51              = STANDARD_US_RETIRED
    SERVICE_52              = STANDARD_VIDEO_ENDOSCOPIC 
    SERVICE_53              = STANDARD_VIDEO_MICROSCOPIC
    SERVICE_54              = STANDARD_VIDEO_PHOTOGRAPHIC
    SERVICE_55              = STANDARD_VL_ENDOSCOPIC
    SERVICE_56              = STANDARD_VL_MICROSCOPIC
    SERVICE_57              = STANDARD_VL_PHOTOGRAPHIC
    SERVICE_58              = STANDARD_VL_SLIDE_MICROSCOPIC
    SERVICE_59              = STANDARD_VOI_LUT
    SERVICE_60              = STANDARD_WAVEFORM_12_LEAD_ECG
    SERVICE_61              = STANDARD_WAVEFORM_AMBULATORY_ECG
    SERVICE_62              = STANDARD_WAVEFORM_BASIC_VOICE_AU
    SERVICE_63              = STANDARD_WAVEFORM_CARDIAC_EP
    SERVICE_64              = STANDARD_WAVEFORM_GENERAL_ECG
    SERVICE_65              = STANDARD_WAVEFORM_HEMODYNAMIC
    SERVICE_66              = STANDARD_XRAY_ANGIO
    SERVICE_67              = STANDARD_XRAY_ANGIO_BIPLANE
    SERVICE_68              = STANDARD_XRAY_RF
    SERVICE_69              = STEREOMETRIC_RELATIONSHIP


# Storage SCU service list.  This includes all of the standard image types.
[Storage_SCU_Service_List]
    SERVICES_SUPPORTED      = 39       # Number of Services in list 
    MAX_OPERATIONS_INVOKED  = 1
    MAX_OPERATIONS_PERFORMED = 1
    SERVICE_1               = STANDARD_MR
    SERVICE_2               = STANDARD_CR
    SERVICE_3               = STANDARD_CT
    SYNTAX_LIST_3           = Uncompressed_Syntax_List    # This is provided as an example and can be removed
    SERVICE_4               = STANDARD_CURVE
    SERVICE_5               = STANDARD_MODALITY_LUT
    SERVICE_6               = STANDARD_OVERLAY
    SERVICE_7               = STANDARD_SEC_CAPTURE
    SERVICE_8               = STANDARD_US
    SERVICE_9               = STANDARD_US_RETIRED
    SERVICE_10              = STANDARD_US_MF
    SERVICE_11              = STANDARD_US_MF_RETIRED
    SERVICE_12              = STANDARD_VOI_LUT
    SERVICE_13              = STANDARD_NM
    SERVICE_14              = STANDARD_XRAY_ANGIO
    SERVICE_15              = STANDARD_XRAY_ANGIO_BIPLANE
    SERVICE_16              = STANDARD_XRAY_RF
    SERVICE_17              = STANDARD_RT_DOSE
    SERVICE_18              = STANDARD_RT_IMAGE
    SERVICE_19              = STANDARD_RT_PLAN
    SERVICE_20              = STANDARD_RT_STRUCTURE_SET
    SERVICE_21              = STANDARD_RT_BEAMS_TREAT
    SERVICE_22              = STANDARD_RT_BRACHY_TREAT
    SERVICE_23              = STANDARD_RT_TREAT_SUM
    SERVICE_24              = STANDARD_PET
    SERVICE_25              = STANDARD_PET_CURVE
    SERVICE_26              = STANDARD_HARDCOPY_COLOR
    SERVICE_27              = STANDARD_HARDCOPY_GRAYSCALE
    SERVICE_28              = STANDARD_PRINT_STORAGE
    SERVICE_29              = STANDARD_DX_PRESENT
    SERVICE_30              = STANDARD_DX_PROCESS
    SERVICE_31              = STANDARD_IO_PRESENT
    SERVICE_32              = STANDARD_IO_PROCESS
    SERVICE_33              = STANDARD_MG_PRESENT
    SERVICE_34              = STANDARD_MG_PROCESS
    SERVICE_35              = STANDARD_VL_ENDOSCOPIC
    SERVICE_36              = STANDARD_VL_MICROSCOPIC
    SERVICE_37              = STANDARD_VL_PHOTOGRAPHIC
    SERVICE_38              = STANDARD_VL_SLIDE_MICROSCOPIC
    SERVICE_39              = STANDARD_GRAYSCALE_SOFTCOPY_PS



# The following 2 service lists are provided as an example of how to define
# a service list that supports compressed transfer syntaxes.  They are not
# used by any sample application.  The SCU_Enc_... service list is a typical 
# SCU's service list.  It tries to negotiation both an uncompressed and a 
# compressed transfer syntax for each service.  The SCP_Enc_... service list 
# simply defines all of the transfer syntaxes supported for each DICOM service.
[SCU_Enc_Storage_Service_List]
    SERVICES_SUPPORTED      = 4       # Number of Services in list    
    SERVICE_1               = STANDARD_MR
    SYNTAX_LIST_1           = Uncompressed_Syntax_List
    SERVICE_2               = STANDARD_MR
    SYNTAX_LIST_2           = Compressed_Syntax_List
    SERVICE_3               = STANDARD_CT
    SYNTAX_LIST_3           = Uncompressed_Syntax_List
    SERVICE_4               = STANDARD_CT
    SYNTAX_LIST_4           = Compressed_Syntax_List
    
[SCP_Enc_Storage_Service_List]
    SERVICES_SUPPORTED      = 2       # Number of Services in list    
    SERVICE_1               = STANDARD_MR
    SYNTAX_LIST_1           = Full_Syntax_List
    SERVICE_2               = STANDARD_CT
    SYNTAX_LIST_2           = Full_Syntax_List


# This is used by the storage SCU sample to open an association with 
# the storage SCP.  Note that the standard "Storage_SCU_Service_List" 
# above for the MERGE_STORE_SCP application is overridden with the
# following service list.
[Storage_Commit_SCU_Service_List]
    SERVICES_SUPPORTED      = 1       # Number of Services in list    
    SERVICE_1               = STORAGE_COMMITMENT_PUSH
    ROLE_1                  = SCU


# This service list is used by the storage SCP sample application to 
# open an assocation up with the storage SCU.  This association is only
# used to send the Storage Commitment N-EVENT-REPORT-RQ message.
[Storage_Commit_SCP_Service_List]
    SERVICES_SUPPORTED      = 1       # Number of Services in list    
    SERVICE_1               = STORAGE_COMMITMENT_PUSH
    ROLE_1                  = SCP


[Query_SCP_Service_List]
    SERVICES_SUPPORTED      = 2      # Number of Services in list
    SERVICE_1               = PATIENT_STUDY_ONLY_QR_FIND
    SERVICE_2               = PATIENT_STUDY_ONLY_QR_MOVE

# used by QRscp sample Java application
[Query_SCP_Service_List1]
    SERVICES_SUPPORTED      = 6      # Number of Services in list
    SERVICE_1               = PATIENT_STUDY_ONLY_QR_FIND
    SERVICE_2               = PATIENT_STUDY_ONLY_QR_MOVE
    SERVICE_3               = STUDY_ROOT_QR_FIND
    SERVICE_4               = STUDY_ROOT_QR_MOVE
    SERVICE_5               = PATIENT_ROOT_QR_FIND
    SERVICE_6               = PATIENT_ROOT_QR_MOVE

[Query_SCU_Service_List]
    SERVICES_SUPPORTED      = 6      # Number of Services in list
    SERVICE_1               = PATIENT_STUDY_ONLY_QR_FIND
    SERVICE_2               = PATIENT_STUDY_ONLY_QR_MOVE
    SERVICE_3               = STUDY_ROOT_QR_FIND
#   EXT_NEG_INFO_3          = 0x01   # This example would propose relational queries in the DICOM extended negotiation for Study Root QR Finds
    SERVICE_4               = STUDY_ROOT_QR_MOVE
    SERVICE_5               = PATIENT_ROOT_QR_FIND
    SERVICE_6               = PATIENT_ROOT_QR_MOVE

[Print_Service_List]
    SERVICES_SUPPORTED      = 2       # Number of Services in list
    SERVICE_1               = BASIC_GRAYSCALE_PRINT_MANAGEMENT
    SERVICE_2               = PRINT_JOB

[Worklist_Service_List]
    SERVICES_SUPPORTED      = 2       # Number of Services in list
    SERVICE_1               = MODALITY_WORKLIST_FIND
    SERVICE_2               = PERFORMED_PROCEDURE_STEP

[Verification_Service_List]
    SERVICES_SUPPORTED      = 1
    SERVICE_1               = STANDARD_ECHO
 


# Each [<syntax_list_name>] section must contain a SYNTAXS_SUPPORTED
#       parameter to specify the number of transfer syntaxes in the 
#       list; plus one parameter for each syntax, of the following format:
#
#       SYNTAX_n = <transfer syntax name>
#
#       where:
#               SYNTAX_n        the character string "SYNTAX_n", with n 
#                               replaced by the number of the transfer 
#                               syntaxes being named.
#               <transfer syntax name>  is the name of one of the transfer
#                               syntaxes supported by your MergeCOM-3 system.  
#                               It must match one of the following DICOM transfer 
#                               syntaxes:
#                       IMPLICIT_LITTLE_ENDIAN
#                       IMPLICIT_BIG_ENDIAN
#                       EXPLICIT_LITTLE_ENDIAN
#                       EXPLICIT_BIG_ENDIAN
#                       RLE
#                       JPEG_BASELINE
#                       JPEG_EXTENDED_2_4
#                       JPEG_EXTENDED_3_5
#                       JPEG_SPEC_NON_HIER_6_8
#                       JPEG_SPEC_NON_HIER_7_9
#                       JPEG_FULL_PROG_NON_HIER_10_12
#                       JPEG_FULL_PROG_NON_HIER_11_13
#                       JPEG_LOSSLESS_NON_HIER_14
#                       JPEG_LOSSLESS_NON_HIER_15
#                       JPEG_EXTENDED_HIER_16_18
#                       JPEG_EXTENDED_HIER_17_19
#                       JPEG_SPEC_HIER_20_22
#                       JPEG_SPEC_HIER_21_23
#                       JPEG_FULL_PROG_HIER_24_26
#                       JPEG_FULL_PROG_HIER_25_27
#                       JPEG_LOSSLESS_HIER_28
#                       JPEG_LOSSLESS_HIER_29
#                       JPEG_LOSSLESS_HIER_14
#                       JPEG_LS_LOSSLESS
#                       JPEG_LS_LOSSY
#                       JPEG_2000
#                       JPEG_2000_LOSSLESS_ONLY
#                       MPEG2_MPML
#                       PRIVATE_SYNTAX_1
#                       PRIVATE_SYNTAX_2
#
# Note that the order that these transfer syntaxes are listed
# defines how SCP applications using the tool kit will select 
# them during association negotiation.  The tool kit will place 
# the highest priority on the first syntax and decreasing 
# priority on following syntaxes in the transfer syntax list.

# Example syntax list used by the Storage_Service_List above.

[Uncompressed_Syntax_List]
    SYNTAXES_SUPPORTED      = 3       # Number of Syntaxes in list
    SYNTAX_1                = EXPLICIT_LITTLE_ENDIAN
    SYNTAX_2                = EXPLICIT_BIG_ENDIAN
    SYNTAX_3                = IMPLICIT_LITTLE_ENDIAN


# The following two syntax lists are supplied as an example of how to 
# define syntax lists for compressed transfer syntaxes.  Only the default
# lossless and lossy compression syntaxes have been included.
[Compressed_Syntax_List]
    SYNTAXES_SUPPORTED      = 3       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_LOSSLESS_HIER_14
    SYNTAX_2                = JPEG_BASELINE  
    SYNTAX_3                = JPEG_EXTENDED_2_4      
    
    
[Full_Syntax_List]
    SYNTAXES_SUPPORTED      = 6       # Number of Syntaxes in list
    SYNTAX_1                = JPEG_LOSSLESS_HIER_14
    SYNTAX_2                = JPEG_BASELINE  
    SYNTAX_3                = JPEG_EXTENDED_2_4      
    SYNTAX_4                = EXPLICIT_LITTLE_ENDIAN
    SYNTAX_5                = EXPLICIT_BIG_ENDIAN
    SYNTAX_6                = IMPLICIT_LITTLE_ENDIAN


