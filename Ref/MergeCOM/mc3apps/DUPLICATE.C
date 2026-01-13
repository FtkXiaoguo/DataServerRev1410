/*************************************************************************
 *
 *       System: MergeCOM-3 - Advanced Tool Kit
 *
 *    $Workfile: $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This is a sample compression/decompression application
 *
 *************************************************************************
 *
 *		(c) 2002 Merge Technologies  Incorporated (d/b/a Merge eFilm)
 *                     Milwaukee, Wisconsin  53214
 *
 *                     -- ALL RIGHTS RESERVED --
 *
 *  This software is furnished under license and may be used and copied
 *  only in accordance with the terms of such license and with the
 *  inclusion of the above copyright notice.  This software or any other
 *  copies thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of the software is hereby
 *  transferred.
 *
 ************************************************************************/

#include "mc3media.h"
#include "mc3msg.h"
#include "mcstatus.h"
#include "mergecom.h"
#include "diction.h"
#include <time.h>

/*
 * Module constants
 */

/* DICOM VR Lengths */
#define UI_LENGTH 64


#define BINARY_READ "rb"
#define BINARY_WRITE "wb"
#define BINARY_APPEND "rb+"

/*
 * Module type declarations
 */

/*
 * Boolean used to handle return values from functions
 */
typedef enum
{
    SAMP_TRUE = 1,
    SAMP_FALSE = 0
} SAMP_BOOLEAN;


/*
 * CBinfo is used to callback functions when reading/writing in stream objects
 * and Part 10 format objects.
 */
typedef struct CALLBACKINFO
{
    FILE*         fp;
	char		  filename[128];
    char          buffer[16*1024];
    unsigned long bytesRead;
} CBInfo;


/*
 * Structure to store local application information 
 */
typedef struct usepeg_options
{
  SAMP_BOOLEAN verbose;
  SAMP_BOOLEAN breakout;
  TRANSFER_SYNTAX srcFormat;
  TRANSFER_SYNTAX dstFormat;
  char filename[128];
  SAMP_BOOLEAN mediaFormat;  
} PROGRAM_OPTIONS;


/*****************************************************************************/
/**CALLBACK FUNCTIONS*********************************************************/
/*****************************************************************************/

/****************************************************************************
 *
 *  Function    :   FileObjToMedia
 *
 *  Parameters  :   A_filename   - Filename to write to
 *                  A_userInfo   - Information to store between calls to this
 *                                 function
 *                  A_dataSize   - Size of A_dataBuffer
 *                  A_dataBuffer - Buffer where write data is stored
 *                  A_isFirst    - Flag to tell if this is the first call
 *                  A_isLast     - Flag to tell if this is the last call
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success, any other MC_STATUS
 *                  value on failure.
 *
 *  Description :   Callback function used to write DICOM file object to
 *                  media.  A pointer to this function is passed to 
 *                  MC_Write_File
 *
 ****************************************************************************/
static MC_STATUS FileObjToMedia( char*    A_filename,
                                 void*    A_userInfo,
                                 int      A_dataSize,
                                 void*    A_dataBuffer,
                                 int      A_isFirst,
                                 int      A_isLast)
{
    size_t     count;
    CBInfo*    cbInfoPtr = (CBInfo*)A_userInfo;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

    if (A_isFirst)
        cbInfoPtr->fp = fopen(A_filename, BINARY_WRITE);

    if (!cbInfoPtr->fp)
    {
        printf("\n Unable to open output file %s\n", cbInfoPtr->filename);
        return MC_CANNOT_COMPLY;
    }

    count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfoPtr->fp);
    if (count != (size_t)A_dataSize)
    {
        printf("\n fwrite error\n");
        fclose(cbInfoPtr->fp);
        return MC_CANNOT_COMPLY;
    }

    if (A_isLast)
        fclose(cbInfoPtr->fp);

    return MC_NORMAL_COMPLETION;

} /* FileObjToMedia() */




/*************************************************************************
 *
 *  Function    :   MsgObjToFile
 *
 *  Parameters  :   A_msgID      - Message id of message being written
 *                  A_userInfo   - Information to store between calls to
 *                                 this function
 *                  A_dataSize   - Size of A_dataBuffer
 *                  A_dataBuffer - Buffer where write data is stored
 *                  A_isFirst    - Flag to tell if this is the first call
 *                  A_isLast     - Flag to tell if this is the last call
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success, any other MC_STATUS
 *                  value on failure.
 *
 *  Description :   This function is used to write data in the "Stream"
 *                  format.  A pointer to the function is passed as a 
 *                  parameter to MC_Message_To_Stream.
 *
 **************************************************************************/
static MC_STATUS MsgObjToFile (int      A_msgID,
                               void*    A_userInfo,
                               int      A_dataSize,
                               void*    A_dataBuffer,
                               int      A_isFirst,
                               int      A_isLast)
{
    CBInfo*     cbInfoPtr = (CBInfo*)A_userInfo;
    size_t      count;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

	if(A_isFirst)
		cbInfoPtr->fp = fopen(cbInfoPtr->filename, BINARY_WRITE);

    if (!cbInfoPtr->fp)
    {
        printf("\n Unable to open output file %s\n", cbInfoPtr->filename);
        return MC_CANNOT_COMPLY;
    }

    count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfoPtr->fp);
    if (count != (size_t)A_dataSize)
    {
        printf("\n fwrite error\n");
        fclose(cbInfoPtr->fp);
        return MC_CANNOT_COMPLY;
    }

    if (A_isLast)
        fclose(cbInfoPtr->fp);

    return MC_NORMAL_COMPLETION;
} /* MsgObjToFile() */

/*************************************************************************
 *
 *  Function    :   ToFunction
 *
 *  Parameters  :   A_msgID      - Message id of message being written
 *					A_tag		 - Tag of message that this data is for
 *                  A_userInfo   - Information to store between calls to
 *                                 this function
 *                  A_dataSize   - Size of A_dataBuffer
 *                  A_dataBuffer - Buffer where write data is stored
 *                  A_isFirst    - Flag to tell if this is the first call
 *                  A_isLast     - Flag to tell if this is the last call
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success, any other MC_STATUS
 *                  value on failure.
 *
 *  Description :   This function is used to write the data of a particular
 *					tag in it's own file.  It will not contain any dicom
 *					information.  A pointer to the function is passed as a 
 *                  parameter to MC_Get_Encapsulated_Value_To_Function.
 *
 **************************************************************************/
static MC_STATUS ToFunction(	int            A_msgID,
								unsigned long  A_tag,
								void*          A_userInfo,
								int            A_dataSize,
								void*          A_dataBuffer,
								int            A_isFirst,
								int            A_isLast)
{
	CBInfo* cbInfoPtr = (CBInfo*) A_userInfo;
    size_t      count;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

	if(A_isFirst)
		cbInfoPtr->fp = fopen(cbInfoPtr->filename, BINARY_WRITE);

    if (!cbInfoPtr->fp)
    {
        printf("\n Unable to open output file %s\n", cbInfoPtr->filename);
        return MC_CANNOT_COMPLY;
    }

	count = fwrite(A_dataBuffer, 1, A_dataSize, cbInfoPtr->fp);
    if (count != (size_t)A_dataSize)
    {
        printf("\n fwrite error\n");
        fclose(cbInfoPtr->fp);
        return MC_CANNOT_COMPLY;
    }

	if(A_isLast)
		fclose(cbInfoPtr->fp);
   
    return MC_NORMAL_COMPLETION;
} /* ToFunction() */


/****************************************************************************
 *
 *  Function    :   MediaToFileObj
 *
 *  Parameters  :   A_fileName   - Filename to open for reading
 *                  A_userInfo   - Pointer to an object used to preserve
 *                                 data between calls to this function.
 *                  A_dataSize   - Number of bytes read
 *                  A_dataBuffer - Pointer to buffer of data read
 *                  A_isFirst    - Set to non-zero value on first call
 *                  A_isLast     - Set to 1 when file has been completely 
 *                                 read
 *
 *  Returns     :   MC_NORMAL_COMPLETION on success
 *                  any other MC_STATUS value on failure.
 *
 *  Description :   Callback function used by MC_Open_File to read a file
 *                  in the DICOM Part 10 (media) format.
 *
 ****************************************************************************/
static MC_STATUS MediaToFileObj( char*     A_filename,
                                 void*     A_userInfo,
                                 int*      A_dataSize,
                                 void**    A_dataBuffer,
                                 int       A_isFirst,
                                 int*      A_isLast)
{

    CBInfo*         cbInfoPtr = (CBInfo*)A_userInfo;
    size_t          bytes_read;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

    if (A_isFirst)
    {
        cbInfoPtr->bytesRead = 0;
        cbInfoPtr->fp = fopen(A_filename, BINARY_READ);
    }
    
    if (!cbInfoPtr->fp)
    {
        printf("\n Unable to open input file %s\n", cbInfoPtr->filename);
        return MC_CANNOT_COMPLY;
    }

    bytes_read = fread(cbInfoPtr->buffer, 1, sizeof(cbInfoPtr->buffer), cbInfoPtr->fp);

    if (ferror(cbInfoPtr->fp))
	{
        printf("\n fread error\n");
        fclose(cbInfoPtr->fp);
        return MC_CANNOT_COMPLY;
	}

    if (feof(cbInfoPtr->fp))
    {
        *A_isLast = 1;
        fclose(cbInfoPtr->fp);
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = cbInfoPtr->buffer;
    *A_dataSize = (int)bytes_read;

    cbInfoPtr->bytesRead += bytes_read;

    return MC_NORMAL_COMPLETION;
    
} /* MediaToFileObj() */

/*************************************************************************
 *
 *  Function    :  StreamToMsgObj
 *
 *  Parameters  :  A_msgID         - Message ID of message being read
 *                 A_CBinformation - user information passwd to callback
 *                 A_isFirst       - flag to tell if this is the first call
 *                 A_dataSize      - length of data read
 *                 A_dataBuffer    - buffer where read data is stored
 *                 A_isLast        - flag to tell if this is the last call
 *
 *  Returns     :  MC_NORMAL_COMPLETION on success
 *                 any other return value on failure.
 *
 *  Description :  Reads input stream data from a file, and places the data
 *                 into buffer to be used by the MC_Stream_To_Message function.
 *
 **************************************************************************/
static MC_STATUS StreamToMsgObj( int        A_msgID,
                                 void*      A_userInfo,
                                 int        A_isFirst,
                                 int*       A_dataSize,
                                 void**     A_dataBuffer,
                                 int*       A_isLast)
{
    size_t          bytesRead;
    CBInfo*         cbInfoPtr = (CBInfo*)A_userInfo;

    if (!A_userInfo)
        return MC_CANNOT_COMPLY;

    if (A_isFirst)
	{
        cbInfoPtr->bytesRead = 0L;
		cbInfoPtr->fp = fopen(cbInfoPtr->filename, BINARY_READ);
	}
        
    if (!cbInfoPtr->fp)
    {
        printf("\n Unable to open input file %s\n", cbInfoPtr->filename);
        return MC_CANNOT_COMPLY;
    }

    bytesRead = fread(cbInfoPtr->buffer, 1, sizeof(cbInfoPtr->buffer), cbInfoPtr->fp);
    if (ferror(cbInfoPtr->fp))
    {
        printf("\n fread error\n");
        fclose(cbInfoPtr->fp);
        return MC_CANNOT_COMPLY;
    }

    if (feof(cbInfoPtr->fp))
    {
        *A_isLast = 1;
        fclose(cbInfoPtr->fp);
    }
    else
        *A_isLast = 0;

    *A_dataBuffer = cbInfoPtr->buffer;
    *A_dataSize = (int)bytesRead;

    cbInfoPtr->bytesRead += bytesRead;

    return MC_NORMAL_COMPLETION;
} /* StreamToMsgObj() */

/*****************************************************************************/
/**OTHER LOCAL FUNCTIONS******************************************************/
/*****************************************************************************/

/**************************************************************************
 *
 *  Function    :   MySplitPath
 *
 *  Parameters  :   A_path   - Buffer containing the full path name
 *                  A_drive  - Contains the drive letter followed by a colon
 *                             (:) if a drive is specified in <A_path>.
 *                  A_dir    - Contains the path of subdirectories, if any,
 *                             including the trailing slash. Forward slashes
 *                             (/), backslashes (\), or both may be present
 *                             in <A_path>.
 *                  A_fname  - Contains the base filename without any
 *                             extensions.
 *                  A_ext    - Contains the filename extension, if any,
 *                             including the leading period (.).
 *
 *  Returns     :   Nothing
 *
 *  Description:
 *    The MySplitPath routine breaks a full path name into its four
 *    components. The <A_path> argument points to a buffer containing the
 *    complete path name. The maximum size necessary for each buffer
 *    is specified by the _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, and
 *    _MAX_EXT manifest constants (defined in
 *    in merge.h for other operating systems) .
 *
 *    The return parameters contain empty strings for any path-name
 *    components not found in <A_path>. You can pass a null pointer to
 *    MySplitPath for any component you don't wish to receive.
 *
 **************************************************************************/
void MySplitPath( char *A_path, char *A_drive, char *A_dir,
                    char *A_fname, char *A_ext )
{
#if defined(_WIN32)
    _splitpath( A_path, A_drive, A_dir, A_fname, A_ext );
#else
    char       *colon, *ptr, *ptr2, *dot, *slash, *last_slash;
    int         length;

    if (A_drive)
        *A_drive = '\0';
    if (A_dir)
        *A_dir = '\0';
    if (A_fname)
        *A_fname = '\0';
    if (A_ext)
        *A_ext = '\0';

    ptr = A_path;
    colon = (char*)strchr (ptr, ':');
    if (colon)
    {
        length = (int)(colon - ptr + 1);
        if (length > 1)
        {
            if (A_drive)
            {
                memcpy (A_drive, ptr, length);
                A_drive[length] = '\0';
            }
            ptr = colon + 1;
        }
    }

    dot = (char*)strrchr (ptr, '.');
    if (dot)
    {
        if (A_ext)
            strcpy (A_ext, dot);
        *dot = '\0';
    }

    last_slash = NULL;
    ptr2 = ptr;
    for (;;)
    {
        slash = (char*)strpbrk (ptr2, "/\\");
        if (!slash)
            break;
        last_slash = slash;
        ptr2 = slash + 1;
    }

    if (last_slash)
    {
        length = (int)(last_slash - ptr + 1);
        if (A_dir)
        {
            memcpy (A_dir, ptr, length);
            A_dir[length] = '\0';
        }
        ptr = last_slash + 1;
    }

    if (A_fname)
        strcpy (A_fname, ptr);

    if (dot)
        *dot = '.';
#endif
}

/****************************************************************************
 *
 *  Function    :   Create_Inst_UID
 *
 *  Parameters  :   none
 *                  
 *  Returns     :   A pointer to a new UID
 *
 *  Description :   This function creates a new UID for use within this 
 *                  application.  Note that this is not a valid method
 *                  for creating UIDs within DICOM because the "base UID"
 *                  is not valid.  
 *                  UID Format:
 *                  <baseuid>.<deviceidentifier>.<serial number>.<process id>
 *                       .<current date>.<current time>.<counter>
 *
 ****************************************************************************/
char *
Create_Inst_UID()
{
    static short UID_CNTR = 0;
    static char  deviceType[] = "1";
    static char  serial[] = "1";
    static char  Sprnt_uid[68];
    char         creationDate[68];
    char         creationTime[68];
    time_t       timeReturn;
    struct tm*   timePtr;
#ifdef UNIX
    unsigned long pid = getpid();
#endif


    timeReturn = time(NULL);
    timePtr = localtime(&timeReturn);
    sprintf(creationDate, "%d%d%d",
           (timePtr->tm_year + 1900),
           (timePtr->tm_mon + 1),
            timePtr->tm_mday);
    sprintf(creationTime, "%d%d%d",
            timePtr->tm_hour,
            timePtr->tm_min,
            timePtr->tm_sec);

#ifdef UNIX    
    sprintf(Sprnt_uid, "2.16.840.1.999999.%s.%s.%d.%s.%s.%d", 
                       deviceType,
                       serial,
                       pid,
                       creationDate,
                       creationTime,
                       UID_CNTR++);
#else
    sprintf(Sprnt_uid, "2.16.840.1.999999.%s.%s.%s.%s.%d", 
                       deviceType,
                       serial,
                       creationDate,
                       creationTime,
                       UID_CNTR++);
#endif

    return(Sprnt_uid);
}


/****************************************************************************
 *
 *  Function    :   GetSyntaxDescription
 *
 *  Parameters  :   A_syntax  - Transfer Syntax to get description of
 *
 *  Returns     :   char* - pointer to character string description
 *
 *  Description :   Return a text description of a DICOM transfer syntax.
 *                  This is used for display purposes.
 *
 ****************************************************************************/
static char* GetSyntaxDescription(TRANSFER_SYNTAX A_syntax)
{
    char* ptr;
    
    switch (A_syntax)
    {
    case IMPLICIT_LITTLE_ENDIAN: ptr = "Implicit VR Little Endian"; break;
    case EXPLICIT_LITTLE_ENDIAN: ptr = "Explicit VR Little Endian"; break;
    case EXPLICIT_BIG_ENDIAN:    ptr = "Explicit VR Big Endian"; break;
    case IMPLICIT_BIG_ENDIAN:    ptr = "Implicit VR Big Endian"; break;
    case DEFLATED_EXPLICIT_LITTLE_ENDIAN: ptr = "Deflated Explicit VR Little Endian"; break;
    case RLE:                    ptr = "RLE"; break;
    case JPEG_BASELINE:          ptr = "JPEG Baseline (Process 1)"; break;
    case JPEG_EXTENDED_2_4:      ptr = "JPEG Extended (Process 2 & 4)"; break;
    case JPEG_EXTENDED_3_5:      ptr = "JPEG Extended (Process 3 & 5)"; break;
    case JPEG_SPEC_NON_HIER_6_8: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)"; break;
    case JPEG_SPEC_NON_HIER_7_9: ptr = "JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)"; break;
    case JPEG_FULL_PROG_NON_HIER_10_12: ptr = "JPEG Full Progression, Non-Hierarchical (Process 10 & 12)"; break;
    case JPEG_FULL_PROG_NON_HIER_11_13: ptr = "JPEG Full Progression, Non-Hierarchical (Process 11 & 13)"; break;
    case JPEG_LOSSLESS_NON_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical (Process 14)"; break;
    case JPEG_LOSSLESS_NON_HIER_15: ptr = "JPEG Lossless, Non-Hierarchical (Process 15)"; break;
    case JPEG_EXTENDED_HIER_16_18: ptr = "JPEG Extended, Hierarchical (Process 16 & 18)"; break;
    case JPEG_EXTENDED_HIER_17_19: ptr = "JPEG Extended, Hierarchical (Process 17 & 19)"; break;
    case JPEG_SPEC_HIER_20_22: ptr = "JPEG Spectral Selection Hierarchical (Process 20 & 22)"; break;
    case JPEG_SPEC_HIER_21_23: ptr = "JPEG Spectral Selection Hierarchical (Process 21 & 23)"; break;
    case JPEG_FULL_PROG_HIER_24_26: ptr = "JPEG Full Progression, Hierarchical (Process 24 & 26)"; break;
    case JPEG_FULL_PROG_HIER_25_27: ptr = "JPEG Full Progression, Hierarchical (Process 25 & 27)"; break;
    case JPEG_LOSSLESS_HIER_28: ptr = "JPEG Lossless, Hierarchical (Process 28)"; break;
    case JPEG_LOSSLESS_HIER_29: ptr = "JPEG Lossless, Hierarchical (Process 29)"; break;
    case JPEG_LOSSLESS_HIER_14: ptr = "JPEG Lossless, Non-Hierarchical, First-Order Prediction"; break;
	case JPEG_2000_LOSSLESS_ONLY: ptr = "JPEG 2000 Image Compression (Lossless Only)"; break;
	case JPEG_2000:				ptr = "JPEG 2000 Image Compression"; break;
    case PRIVATE_SYNTAX_1:      ptr = "Private Syntax 1"; break;
    case PRIVATE_SYNTAX_2:      ptr = "Private Syntax 2"; break;
    }
    return ptr;
}

/****************************************************************************
 *
 *  Function    :   WeSupportThisFormat
 *
 *  Parameters  :   A_syntax  - Transfer Syntax to check
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   This program only supports these Transfer Syntaxs.  Verify
 *					 that the syntax passed in is one of them.
 *
 ****************************************************************************/
static SAMP_BOOLEAN WeSupportThisFormat(TRANSFER_SYNTAX A_syntax)
{
	if( A_syntax == IMPLICIT_LITTLE_ENDIAN	||
		A_syntax == IMPLICIT_BIG_ENDIAN		||
		A_syntax == EXPLICIT_LITTLE_ENDIAN	||
		A_syntax == EXPLICIT_BIG_ENDIAN		||
		A_syntax == DEFLATED_EXPLICIT_LITTLE_ENDIAN	||
		A_syntax == JPEG_BASELINE			||
		A_syntax == JPEG_EXTENDED_2_4		||
		A_syntax == JPEG_LOSSLESS_HIER_14	||
		A_syntax == JPEG_2000				||
		A_syntax == JPEG_2000_LOSSLESS_ONLY)

		return SAMP_TRUE;
	else
		return SAMP_FALSE;
}

/****************************************************************************
 *
 *  Function    :   PrintError
 *
 *  Parameters  :   A_string - Error string to display
 *					A_status - Merge Toolkit error status
 *
 *  Description :   Display a text string on one line and the error message
 *                  for a given error on the next line.
 *
 ****************************************************************************/
static void PrintError(char* A_string, MC_STATUS A_status, int A_ln)
{
    char        prefix[30] = {0};

    /*
     *  Need process ID number for messages
     */
#ifdef UNIX    
    sprintf(prefix, "PID %d", getpid() );
#endif 
    if (A_status == -1)
    {
        printf("\n %s\tLine %d: %s\n",prefix, A_ln, A_string);
    }
    else
    {
        printf("\n %s\tLine %d: %s:\n",prefix, A_ln, A_string);
        printf("\n %s\t\t%s\n", prefix,MC_Error_Message(A_status));
    }
}

/********************************************************************
 *
 *  Function    :   PrintCmdLine
 *
 *  Parameters  :   none
 *
 *  Returns     :   nothing
 *
 *  Description :   Prints program usage
 *
 ********************************************************************/
static void PrintCmdLine(void)
{
    printf("\n Usage duplicate -f filename <-s source_format> <-d dest_format> <-v> <-b> \n\n");
    printf("\tfilename	part 10 DICOM file, or message \"stream\" file \n");
    printf("\tsource_format	Format of <filename> above \n");
	printf("\t		Do not use if part 10, otherwise\n");
    printf("\t		IMPLICIT_LITTLE_ENDIAN \n");
    printf("\t		EXPLICIT_LITTLE_ENDIAN \n");
    printf("\t		IMPLICIT_BIG_ENDIAN \n");
    printf("\t		EXPLICIT_BIG_ENDIAN \n");
    printf("\t		DEFLATED_EXPLICIT_LITTLE_ENDIAN \n");
    printf("\t		JPEG_BASELINE \n");
    printf("\t		JPEG_EXTENDED_2_4 \n");
    printf("\t		JPEG_LOSSLESS_HIER_14 \n");
    printf("\t		JPEG_2000_LOSSLESS_ONLY \n");
    printf("\t		JPEG_2000 \n");
	printf("\t		Default IMPLICIT_LITTLE_ENDIAN \n");
    printf("\tdest_format	Format to create (one of the above formats)\n");
	printf("\t		If input was part 10, output will be part 10,\n");
	printf("\t		otherwise \"stream\" format.\n");
	printf("\t		Default JPEG_BASELINE \n");
    printf("\t-v		execute in verbose mode \n");
	printf("\t		Default is off \n");
    printf("\t-b		Breakout option.  Copy each frame's pixel data encapsulated in the\n");
	printf("\t		destination file into separate compressed image files.  Default is off \n");
	printf("\tAll arguments are optional EXCEPT FILENAME, but they're only optional\n");
	printf("\t if the defaults are acceptable for source_format, dest_format, verbose\n");
	printf("\tIf source_format = dest_format, then an output file is not created, but\n");
	printf("\t breakout will occur if the flag is specified. \n");



} /* end PrintCmdLine() */

/*************************************************************************
 *
 *  Function    :   TestCmdLine
 *
 *  Parameters  :   Aargc   - Command line arguement count
 *                  Aargv   - Command line arguements
 *                  A_options - Local application options read in.
 *
 *  Return value:   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Test command line for valid arguements.  If problems
 *                  are found, display a message and return SAMP_FALSE
 *
 *************************************************************************/
static SAMP_BOOLEAN TestCmdLine(  /* Test Command Line */
        int                 A_argc,   
        char*               A_argv[], 
        PROGRAM_OPTIONS*    A_options )
{
    int       i;
    int       argCount=0;
	SAMP_BOOLEAN fileProvided = SAMP_FALSE;
    
    if (A_argc < 1)
    {
        PrintCmdLine();
        return SAMP_FALSE;
    }
   
    /*
     * Set default values
     */
    A_options->verbose = SAMP_FALSE;
	A_options->srcFormat = IMPLICIT_LITTLE_ENDIAN;
	A_options->dstFormat = JPEG_BASELINE;
	A_options->mediaFormat = SAMP_TRUE;
	A_options->breakout = SAMP_FALSE;
	
    /*
     * Loop through each arguement
     */     
    for (i = 1; i < A_argc; i++)
    {
        if ( !strcmp(A_argv[i], "-h") || !strcmp(A_argv[i], "/h") ||
             !strcmp(A_argv[i], "-H") || !strcmp(A_argv[i], "/H") ||
             !strcmp(A_argv[i], "-?") || !strcmp(A_argv[i], "/?"))
        {
            PrintCmdLine();
            return SAMP_FALSE; 
        }
        else if ( !strcmp(A_argv[i], "-s") || !strcmp(A_argv[i], "-S"))
        {
            /*
             * Set the srcFormat
             */
            i++;
			if (!strcmp("IMPLICIT_LITTLE_ENDIAN", A_argv[i]) )
				A_options->srcFormat = IMPLICIT_LITTLE_ENDIAN;

			else if (!strcmp("EXPLICIT_LITTLE_ENDIAN", A_argv[i]) )
				A_options->srcFormat = EXPLICIT_LITTLE_ENDIAN;

			else if (!strcmp("IMPLICIT_BIG_ENDIAN", A_argv[i]) )
				A_options->srcFormat = IMPLICIT_BIG_ENDIAN;

			else if (!strcmp("EXPLICIT_BIG_ENDIAN", A_argv[i]) )
				A_options->srcFormat = EXPLICIT_BIG_ENDIAN;

			else if (!strcmp("DEFLATED_EXPLICIT_LITTLE_ENDIAN", A_argv[i]) )
				A_options->srcFormat = DEFLATED_EXPLICIT_LITTLE_ENDIAN;

			else if (!strcmp("JPEG_BASELINE", A_argv[i]) )
				A_options->srcFormat = JPEG_BASELINE;

			else if (!strcmp("JPEG_EXTENDED_2_4", A_argv[i]) )
				A_options->srcFormat = JPEG_EXTENDED_2_4;

			else if (!strcmp("JPEG_LOSSLESS_HIER_14", A_argv[i]) )
				A_options->srcFormat = JPEG_LOSSLESS_HIER_14;

			else if (!strcmp("JPEG_2000_LOSSLESS_ONLY", A_argv[i]) )
				A_options->srcFormat = JPEG_2000_LOSSLESS_ONLY;

			else if (!strcmp("JPEG_2000", A_argv[i]) )
				A_options->srcFormat = JPEG_2000;

			else
				A_options->srcFormat = IMPLICIT_LITTLE_ENDIAN;

			A_options->mediaFormat = SAMP_FALSE;
	
        } 
        else if ( !strcmp(A_argv[i], "-d") || !strcmp(A_argv[i], "-D"))
        {
            /*
             * Set the dstFormat
             */
            i++;
			if (!strcmp("IMPLICIT_LITTLE_ENDIAN", A_argv[i]) )
				A_options->dstFormat = IMPLICIT_LITTLE_ENDIAN;

			else if (!strcmp("EXPLICIT_LITTLE_ENDIAN", A_argv[i]) )
				A_options->dstFormat = EXPLICIT_LITTLE_ENDIAN;

			else if (!strcmp("IMPLICIT_BIG_ENDIAN", A_argv[i]) )
				A_options->dstFormat = IMPLICIT_BIG_ENDIAN;

			else if (!strcmp("EXPLICIT_BIG_ENDIAN", A_argv[i]) )
				A_options->dstFormat = EXPLICIT_BIG_ENDIAN;

			else if (!strcmp("DEFLATED_EXPLICIT_LITTLE_ENDIAN", A_argv[i]) )
				A_options->dstFormat = DEFLATED_EXPLICIT_LITTLE_ENDIAN;

			else if (!strcmp("JPEG_BASELINE", A_argv[i]) )
				A_options->dstFormat = JPEG_BASELINE;

			else if (!strcmp("JPEG_EXTENDED_2_4", A_argv[i]) )
				A_options->dstFormat = JPEG_EXTENDED_2_4;

			else if (!strcmp("JPEG_LOSSLESS_HIER_14", A_argv[i]) )
				A_options->dstFormat = JPEG_LOSSLESS_HIER_14;

			else if (!strcmp("JPEG_2000_LOSSLESS_ONLY", A_argv[i]) )
				A_options->dstFormat = JPEG_2000_LOSSLESS_ONLY;

			else if (!strcmp("JPEG_2000", A_argv[i]) )
				A_options->dstFormat = JPEG_2000;

			else
				A_options->dstFormat = IMPLICIT_LITTLE_ENDIAN;
        }
        else if ( !strcmp(A_argv[i], "-f") || !strcmp(A_argv[i], "-F"))
        {
            /*
             * Set the source Filename
             */
            i++;
            strcpy(A_options->filename, A_argv[i]);
			fileProvided = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-v") || !strcmp(A_argv[i], "-V"))
        {
            /*
             * verbose mode
             */
            A_options->verbose = SAMP_TRUE;
        }
        else if ( !strcmp(A_argv[i], "-b") || !strcmp(A_argv[i], "-B"))
        {
            /*
             * breakout mode
             */
            A_options->breakout = SAMP_TRUE;
        }
   }

	if(!fileProvided)
	{
		printf("\n Must provide a file\n");
		PrintCmdLine();
		return SAMP_FALSE;
	}

	if(A_options->breakout && (	A_options->dstFormat != JPEG_BASELINE && 
								A_options->dstFormat != JPEG_EXTENDED_2_4 && 
								A_options->dstFormat != JPEG_LOSSLESS_HIER_14 &&
								A_options->dstFormat != JPEG_2000_LOSSLESS_ONLY &&
								A_options->dstFormat != JPEG_2000))
	{
		printf("\n Can't breakout a non-encapsulated message or file.  Ignoring flag");
		A_options->breakout = SAMP_FALSE;
	}

    return SAMP_TRUE;
    
}/* TestCmdLine() */



 
 
/****************************************************************************
 *
 *  Function    :   ReadFileFromMedia
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *
 *  Returns     :   SAMP_TRUE on success
 *                  SAMP_FALSE on failure to read the object
 *
 *  Description :   This function reads a file in the DICOM Part 10 (media)
 *                  file format.  Before returning, it determines the
 *                  transfer syntax the file was encoded as, AND converts to
 *					a message!
 *
 *                  When this function returns failure, the caller should
 *                  not do any cleanup, A_msgID will not contain a valid
 *                  message ID.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadFileFromMedia(	PROGRAM_OPTIONS*	A_options,
										int					A_appID,
										int*				A_msgID)
{
    CBInfo		cbInfo;
    MC_STATUS   mcStatus;
    char        transferSyntaxUID[UI_LENGTH+2];

    if (A_options->verbose)
    {
        printf("\n Reading %s in DICOM Part 10 format", A_options->filename);
    }

    /*
     * Create new File object 
     */
    mcStatus = MC_Create_Empty_File(A_msgID, A_options->filename);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to create file object",mcStatus, __LINE__);
        return( SAMP_FALSE );
    }

	/* Don't have to set the callback filename because it will be passed to callback */
	
    /*
     * Read the file off of disk
     */
    mcStatus = MC_Open_File(A_appID,
                           *A_msgID,
                            &cbInfo,
                            MediaToFileObj);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Open_File failed, unable to read file from media", mcStatus, __LINE__);
        MC_Free_File(A_msgID);
        return( SAMP_FALSE );
    }
    
    /*
     * Get the transfer syntax UID from the file so we can determine srcFormat
     */
    mcStatus = MC_Get_Value_To_String(*A_msgID, 
                            MC_ATT_TRANSFER_SYNTAX_UID,
                            sizeof(transferSyntaxUID),
                            transferSyntaxUID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Get_Value_To_String failed for transfer syntax UID", mcStatus, __LINE__);
        MC_Free_File(A_msgID);
        return SAMP_FALSE;
    }

    mcStatus = MC_Get_Enum_From_Transfer_Syntax( transferSyntaxUID, &A_options->srcFormat);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        printf("\n Invalid transfer syntax UID contained in the file: %s\n", transferSyntaxUID);
        MC_Free_File(A_msgID);
        return SAMP_FALSE; 
    } 

	/* Check if we support this xfr syntax */
	if(WeSupportThisFormat(A_options->srcFormat) == SAMP_FALSE)
	{
            printf("\n Error: The Source transfer syntax (%s) is not supported\n", 
                   GetSyntaxDescription(A_options->srcFormat));
            MC_Free_File(A_msgID);
            return SAMP_FALSE; 
    }


    if (A_options->verbose)
        printf("\n Reading DICOM Part 10 format file in %s: \n %s", 
               GetSyntaxDescription(A_options->srcFormat),
               A_options->filename);

	/* Convert it because we can only duplicate messages */
    mcStatus = MC_File_To_Message(*A_msgID);    
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_File_To_Message failed", mcStatus, __LINE__);
        MC_Free_File(A_msgID);
        return SAMP_FALSE;
    }

	mcStatus = MC_Set_Message_Transfer_Syntax(*A_msgID, A_options->srcFormat);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Message_Transfer_Syntax error", mcStatus, __LINE__);
        MC_Free_Message(A_msgID);
        return SAMP_FALSE;
    }

    return SAMP_TRUE;
    
} /* ReadFileFromMedia() */




/****************************************************************************
 *
 *  Function    :   ReadMessageFromFile
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *
 *  Returns     :   SAMP_TRUE  on success
 *                  SAMP_FALSE on failure to open the file
 *
 *  Description :   This function reads a file in the DICOM "stream" format.
 *                  This format contains no DICOM part 10 header information.
 *                  The transfer syntax of the object is contained in the 
 *                  A_options->srcFormat parameter.  
 *
 *                  When this function returns failure, the caller should
 *                  not do any cleanup, A_msgID will not contain a valid
 *                  message ID.
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadMessageFromFile(	PROGRAM_OPTIONS*	A_options,
											int*				A_msgID)
{
    MC_STATUS               mcStatus;
    unsigned long           errorTag;
    CBInfo                  cbInfo;  

	if(WeSupportThisFormat(A_options->srcFormat) == SAMP_FALSE)
	{
            printf("\n Error: The Source transfer syntax (%s) is not supported\n", 
                   GetSyntaxDescription(A_options->srcFormat));
            return SAMP_FALSE; 
    }

    if (A_options->verbose)
        printf("\n Reading DICOM \"stream\" format file in %s: \n %s", 
               GetSyntaxDescription(A_options->srcFormat),
               A_options->filename);

    /*
     * Open an empty message object to load the image into
     */
    mcStatus = MC_Open_Empty_Message(A_msgID);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("Unable to open empty message", mcStatus, __LINE__);
        return SAMP_FALSE;
    }

	/* Set the filename to be opened */
	strcpy(cbInfo.filename,A_options->filename);

    mcStatus = MC_Stream_To_Message(*A_msgID,
                                    MC_ATT_GROUP_0008_LENGTH, 
                                    0xffffFFFF,
                                    A_options->srcFormat,
                                    &errorTag,
                                    (void*) &cbInfo, /* data for StreamToMsgObj */
                                    StreamToMsgObj);

    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Stream_To_Message error", mcStatus, __LINE__);
        MC_Free_Message(A_msgID);
        return SAMP_FALSE;
    }
	
	mcStatus = MC_Set_Message_Transfer_Syntax(*A_msgID, A_options->srcFormat);
    if (mcStatus != MC_NORMAL_COMPLETION)
    {
        PrintError("MC_Set_Message_Transfer_Syntax error", mcStatus, __LINE__);
        MC_Free_Message(A_msgID);
        return SAMP_FALSE;
    }

    return SAMP_TRUE;

} /* ReadMessageFromFile() */
  
  
/****************************************************************************
 *
 *  Function    :   ReadInFile
 *
 *  Parameters  :   A_options  - Pointer to structure containing input
 *                               parameters to the application
 *                  A_appID    - Application ID registered 
 *                  A_msgID    - The message ID of the message to be opened
 *                               returned here.
 *
 *  Returns     :   SAMP_TRUE
 *                  SAMP_FALSE
 *
 *  Description :   Determine the format of a DICOM file and read it into
 *                  memory.  NOTE, either function returns a msgID, not a fileID
 *
 ****************************************************************************/
static SAMP_BOOLEAN ReadInFile( PROGRAM_OPTIONS*  A_options,
							  int				A_appID,
                              int*              A_msgID)
{
    SAMP_BOOLEAN            sampBool;


	if(A_options->mediaFormat)
            sampBool = ReadFileFromMedia( A_options, 
                                          A_appID, 
                                          A_msgID);
	else
            sampBool = ReadMessageFromFile( A_options, 
                                            A_msgID);

    return sampBool;
}    




 
/*****************************************************************************/
/** MAIN *********************************************************************/
/*****************************************************************************/
int main(int argc, char** argv)
{
	int srcID, dstID, appID, breakOutID, frameNum;
	MC_STATUS status;
	PROGRAM_OPTIONS options;
	char outFile[128];
	char outExt[6];
	CBInfo cbInfo;
	char dstXfrSyntaxUID[65];

	/* Always start with initializing the library */
	status = MC_Library_Initialization(NULL, NULL, NULL);
	if(status != MC_NORMAL_COMPLETION)
	{
		PrintError("MC_Library_Initialization failed", status, __LINE__);
	    return(0);
	}

	/* Register our application */
	status = MC_Register_Application(&appID, "USE_PEG_LIB");
	if(status != MC_NORMAL_COMPLETION)
	{
		PrintError("MC_Register_Application failed", status, __LINE__);
	    return(0);
	}

    /*
     * Test the command line parameters, and populate the options
     * structure with these parameters
     */
    if ( !TestCmdLine( argc, argv, &options ) )
    {
        return(0);
    }

	/* Check what the user would like to create before doing anything else */
	if(WeSupportThisFormat(options.dstFormat) == SAMP_FALSE)
	{
            printf("\n Error: The Destination transfer syntax (%s) is not supported\n", 
                   GetSyntaxDescription(options.dstFormat));
	        return(0);
    }
	
	/* Determine if Part 10 file, or message file, and create srcID which will be a msg */
	if( !ReadInFile(&options, appID, &srcID) )
	{
		printf("\n There was a problem reading the file\n");
        return(0);
	}

	breakOutID = srcID;

	/* now we have msg ID with valid message, are we changing xfr syntaxs? */
	if(options.srcFormat != options.dstFormat)
	{
		if(options.dstFormat == JPEG_2000)
		{
			if(options.verbose)
				printf("\n JPEG_2000 specified.  Default = Lossy");

			/* We will treat this as lossy compression */
			MC_Set_Bool_Config_Value(COMPRESSION_WHEN_J2K_USE_LOSSY, 1);
		}

		if(options.verbose)
			printf("\n Change %s\n to %s", GetSyntaxDescription(options.srcFormat),
												GetSyntaxDescription(options.dstFormat));
		/* 
		 * Since we are changing Xfr syntaxs, register compression callbacks for the source msg
		 * if the source msg is one of the supported encapsulated types
		 */
		if( options.srcFormat == JPEG_BASELINE ||
			options.srcFormat == JPEG_EXTENDED_2_4 ||
			options.srcFormat == JPEG_LOSSLESS_HIER_14 ||
			options.srcFormat == JPEG_2000_LOSSLESS_ONLY ||
			options.srcFormat == JPEG_2000)
		{
			status = MC_Register_Compression_Callbacks(srcID, MC_Standard_Compressor, MC_Standard_Decompressor);
			if(status != MC_NORMAL_COMPLETION)
			{
				PrintError("MC_Register_Compression_Callbacks failed", status, __LINE__);
				MC_Free_Message(&srcID);
				return(0);
			}
		}

		/* 
		 * If the destination is encapsulated, MC_Duplicate_Message will automatically register
		 *  the compression/decompression callbacks when called.  If it's not encapsulated, we
		 *  don't have to pass compression/decompression callbacks to it.
		 */
		if( options.dstFormat == JPEG_BASELINE ||
			options.dstFormat == JPEG_EXTENDED_2_4 ||
			options.dstFormat == JPEG_LOSSLESS_HIER_14 ||
			options.dstFormat == JPEG_2000_LOSSLESS_ONLY ||
			options.dstFormat == JPEG_2000)

			status = MC_Duplicate_Message(srcID, &dstID, options.dstFormat, MC_Standard_Compressor, MC_Standard_Decompressor);
		else
			status = MC_Duplicate_Message(srcID, &dstID, options.dstFormat, NULL, NULL);

		/* We don't need the source msg anymore, so free it */
		MC_Free_Message(&srcID);

		if(status != MC_NORMAL_COMPLETION)
		{
			PrintError("MC_Duplicate_Message failed", status, __LINE__);
	        return(0);
		}

		/* Now that we have an output message, set appropriate attributes */
		if( options.dstFormat == JPEG_BASELINE ||
			options.dstFormat == JPEG_EXTENDED_2_4 ||
			options.dstFormat == JPEG_2000)
		{
			/* The following must be set when duplicating to a lossy compression */

			/* Set Lossy Image Compression (0028,2110) to "01" */
			status = MC_Set_Value_From_Int(dstID, 0x00282110, 1);

			/* Set Attribute Image Type (0008,0008) Value 1 to DERIVED */
			status = MC_Set_Value_From_String(dstID, 0x00080008, "DERIVED");

			/* Need to set a new SOP Instance UID.  NOTE, Create_Inst_UID 
			 *  is not a valid method for creating UIDs within DICOM because the "base UID"
			 *  is not valid.  This should be replaced with YOUR UID creator
			 */
			status = MC_Set_Value_From_String(dstID, 0x00080018, Create_Inst_UID());

			printf("\n New SOP Instance UID has been assigned, but is invalid.");
		}

		/* create output file/msg */

		/* set up the output filename */
		MySplitPath(options.filename,NULL,NULL,&outFile[0],&outExt[0]);
		sprintf(cbInfo.filename,"%s_out%s",&outFile[0], &outExt[0]);

		/* 
		 * If the source msg was a file, then the output should be a file.  We would like to
		 *  keep the output the same format as the input.
		 */
		if(options.mediaFormat)
		{
			/* write a file */
			status = MC_Message_To_File(dstID, cbInfo.filename);
			if(status != MC_NORMAL_COMPLETION)
			{
				PrintError("MC_Message_To_File failed", status, __LINE__);
				MC_Free_Message(&dstID);
				return(0);
			}
			status = MC_Get_Transfer_Syntax_From_Enum(options.dstFormat, &dstXfrSyntaxUID[0], 65);
			if(status != MC_NORMAL_COMPLETION)
			{
				PrintError("MC_Get_Transfer_Syntax_From_Enum failed", status, __LINE__);
				MC_Free_Message(&dstID);
				return(0);
			}

		    status = MC_Set_Value_From_String(dstID,0x00020010, &dstXfrSyntaxUID[0]);
			if(status != MC_NORMAL_COMPLETION)
			{
				PrintError("MC_Set_Value_From_String failed", status, __LINE__);
				MC_Free_Message(&dstID);
				return(0);
			}

			status = MC_Write_File(dstID, 0, &cbInfo, FileObjToMedia);
			if(status != MC_NORMAL_COMPLETION)
			{
				PrintError("MC_Write_File failed", status, __LINE__);
				MC_Free_Message(&dstID);
				return(0);
			}
		}
		else
		{
			/* write a stream */
			status = MC_Message_To_Stream(	dstID, 
											0x00000000, 0xFFFFFFFF, 
											options.dstFormat, 
											&cbInfo, 
											MsgObjToFile);

			if(status != MC_NORMAL_COMPLETION)
			{
				PrintError("MC_Message_To_Stream failed", status, __LINE__);
				MC_Free_Message(&dstID);
				return(0);
			}
		}

		if(options.verbose)
			printf("\n Output file created: %s\n", cbInfo.filename);

		breakOutID = dstID;

	}

	/*
	 * The user would like us to create separate compressed image files if options.breakout
	 *  was specified
	 */
	if(options.breakout)
	{
		/*
		 * We don't want to uncompress the compressed images, but we do want to
		 *  unencapsulate them into separate files.  We do this by NULLing out the
		 *  compression/decompression callbacks, and using MC_Get_(Next_)Encap_Value.
		 *  If we wanted to decompress them, we would simply use:
		 *  MC_Register_Compression_Callbacks(breakOutID, MC_Standard_Compressor, MC_Standard_Decompressor);
		 */
		status = MC_Register_Compression_Callbacks(breakOutID, NULL, NULL);
		if(status != MC_NORMAL_COMPLETION)
		{
			if(status == MC_INVALID_TRANSFER_SYNTAX)
				printf("\n The message wasn't encapsulated and we can't break it out.\n");
			else
				PrintError("MC_Register_Compression_Callbacks failed.", status, __LINE__);

			MC_Free_Message(&breakOutID);
			return(0);
		}

		frameNum = 1;

		MySplitPath(options.filename,NULL,NULL,&outFile[0],NULL);

		/* 
		 * Either this is the destination message, or it is the source message
		 *  whos xfr syntax is the same as the destination anyway
		 */
		if(options.dstFormat == JPEG_LOSSLESS_HIER_14)
			sprintf(cbInfo.filename, "%s_frame%d.ljp",&outFile[0],frameNum++);
		else if(options.dstFormat == JPEG_BASELINE || options.dstFormat == JPEG_EXTENDED_2_4)
			sprintf(cbInfo.filename, "%s_frame%d.jpg",&outFile[0],frameNum++);
		else
			sprintf(cbInfo.filename, "%s_frame%d.jp2",&outFile[0],frameNum++);

		/* Break out breakOutID */

		/* First frame sent to callback function "ToFunction" */
		status = MC_Get_Encapsulated_Value_To_Function(breakOutID, 0x7fe00010, &cbInfo, ToFunction);

		while(status == MC_END_OF_FRAME)
		{
			if(options.dstFormat == JPEG_LOSSLESS_HIER_14)
				sprintf(cbInfo.filename, "%s_frame%d.ljp",&outFile[0],frameNum++);
			else if(options.dstFormat == JPEG_BASELINE || options.dstFormat == JPEG_EXTENDED_2_4)
				sprintf(cbInfo.filename, "%s_frame%d.jpg",&outFile[0],frameNum++);
			else
				sprintf(cbInfo.filename, "%s_frame%d.jp2",&outFile[0],frameNum++);

			/* Get next frame to callback function "ToFunction" */
			status = MC_Get_Next_Encapsulated_Value_To_Function(breakOutID, 0x7fe00010, &cbInfo, ToFunction);

		}

		if(status != MC_NORMAL_COMPLETION)
		{
			PrintError("Breaking out the images failed", status, __LINE__);
			MC_Free_Message(&breakOutID);
			return(0);
		}
		else
		{
			if(options.verbose)
				printf("\n Message was split into %d image files\n", frameNum-1);
		}
	}

	/* 
	 * Free the output of our duplicate (which was a message or a file depending on what 
	 *  the source was), or the source message/file because there wasn't a change of xfr
	 *  syntaxs.  Either way, we know what it is by the source file's format, specified in
	 *  options.mediaFormat
	 */  
	if(options.mediaFormat)
		MC_Free_File(&breakOutID);
	else
		MC_Free_Message(&breakOutID);

	/* Release the library */
	MC_Library_Release();

	return(1);
}


