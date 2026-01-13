/*.T PIC.H  Include file for Pegasus Routines
 * $Header$
 * $Nokeywords: $
 */

/***************************************************************************\
*       Copyright (c) 1996-1998, Pegasus Imaging Corporation                *
*       All rights reserved.                                                *
*****************************************************************************
*       Revision History:                                                   *
* modified 2/16/96 -- jweber                                                *
*       -- changed Eof in QUEUE to a dword of flags                         *
*       -- changed names of OP80, 81, 82 structures to D2F, F2D, UTL        *
* modified 2/2/96 -- jweber                                                 *
*       -- added QBIT_COMMENT, QBIT_PALETTE, BI_RLE4, BI_RLE8               *
* modified 1/26/96 -- jweber                                                *
*       -- added YieldEvery to opcode 81                                    *
*       -- modified QBIT_IMAGESIZE comment                                  *
* modified 12/26/95+ -- jweber                                              *
*       -- added PicFlags to opcode 80                                      *
*       -- added BI_PCX, BI_TIF, BI_TGA (generic) definitions               *
*       -- added PF_MultiImage flag                                         *
*       -- added PicFlags to opcode 82                                      *
*       -- renamed Histogram/ColorMap to ptr/ptr2, opcode 82                *
*       -- added NumColors to opcode 82; removed references to subcode 7    *
*       -- added NumImages and ImageNum to PEGQUERY, and associated         *
*          QBIT_NUMIMAGES                                                   *
*       -- changed PEGQUERY to inidividual expansion DWORDs                 *
* modified 2/22/96 -- smann                                                 *
*       -- support reverse queue                                            *
*       -- ParmVer is now 11, ParmVerMinor is now 1                         *
* modified 3/12/96 -- jweber                                                *
*       -- opcode 82: "Reserverd3" changed to "ptr3"                        *
* modified 3/27/96 -- jweber                                                *
*       -- incorporates John's changes for opcode 15 (D2J)                  *
* modified 4/4/96 -- jweber                                                 *
*       -- more of John's changes                                           *
* modified 4/17/96 -- jweber                                                *
*       -- QUEUE structure changed to new pointers                          *
* modified 4/21/96 -- SSM                                                   *
*       -- LOSSLESS and LAYER structures added as well as support for the   *
*          LIME and LIMP functions.                                         *
* modified 4/23/96 -- jrb                                                   *
*       -- ParmVer is 13                                                    *
* modified 5/9/96 -- jweber                                                 *
*       -- Added BI_TIFJ, QBIT_SOIMARKER, and SOIMarker field to query      *
*          private u structure in support for TIFF Jpeg                     *
* modified 8/31/96 -- Els                                                   *
*       -- Added support for ZOOM operation                                 *
* modified 10/16/96 -- SSM                                                  *
*       -- Added support for cropping                                       *
* modified 3/2/96 -- jweber                                                 *
*       -- Reversing optimization                                           *
* modified 3/10/97 -- jweber                                                *
*       -- Several changes to D2F_STRUC to support Printhouse requirements  *
*          (32-bit "DIB"s; CMYK indication; DotRange support; Moto byte     *
*          order; X & Y resolutions                                         *
* modified 6/12/97 -- SSM                                                   *
*       -- Changes to allow LIP/LIE support                                 *
* modified 8/2/97  -- Jim                                                   *
*       -- John and Charles changes so pc/mac use a single set of includes  *
* modified 9/18/97 -- SSM                                                   *
*       -- Changes to allow LL3 and PIC2List support                        *
* modified 12/8/97 -- SSM                                                   *
*       -- Changes to allow Regions                                         *
* modified 4/28/98 -- SSM                                                   *
*       -- Changes to allow support for JPEG restart markers                *
* modified 9/14/98 -- SSM                                                   *
*       -- Added WSQ support, YIELD flag to correspond to RES_YIELD         *
\***************************************************************************/

#if !defined(_PIC)
#define _PIC

#define CURRENT_PARMVER         20

/* DLG added to detect Windows environment from compiler directives */
  #if !defined(WINDOWS)
    #if defined(_Windows) || defined(_WINDOWS) || defined(_WINDOWS_) || \
        defined(__WINDOWS_386__) || defined(_WINDOWS_16_)
            #define WINDOWS
    #endif
  #endif

  #if !defined(__FLAT__) && defined(WIN32)
    #define __FLAT__
  #endif


#include    "stdtypes.h"
#include    "pcd.h"

#if !defined(USEPICSETJMP)
#include    <setjmp.h>
#endif

/* DLG - only include if not doing windows */
  #if !defined(WINDOWS) && !defined(PIC_DONT_DEFINE_STDTYPES)
        #include  "bmp.h"
  #endif

#if defined(PIC_INTERNAL)
#include "internal.h"
#endif

#ifdef __MWERKS__
    /*#if PRAGMA_ALIGN_SUPPORTED*/
    #pragma options align=packed
    /*#endif*/
#elif defined(__GNUC__)
    #pragma pack(1)
#elif defined(__unix__)
    #pragma pack(1)
#elif defined(__BORLANDC__)
    #pragma option -a1
#else /* assume microsoft or watcom */
    #if defined(__FLAT__) || defined(__WATCOMC__)
        #pragma pack(push)
    #endif
    /* microsoft 16-bit doesn't support pack(push) */
    #pragma pack(1)
#endif

/*#### must be packed from here on ####*/

#if !defined(BI_BITFIELDS)
/* this is needed in 16-bit windows where bmp.h isn't
    included, but windows.h doesn't define BI_BITFIELDS */
#define BI_BITFIELDS    (3L)
#endif

/*.P*/
/***************************************************************************\
*   The following typedefs declare extensions for the file types and for    *
*   PIC1 and PIC4 compression types similar to JPEGINFOHEADER.              *
\***************************************************************************/
#define FC( ch0, ch1, ch2, ch3 )                                \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define BI_picJPEG FC('J','P','E','G') /* 'JPEG' compressed (raw ) */
#if !defined(BI_JPEG)
    /* MS started defining BI_JPEG in the header files */
#define BI_JPEG BI_picJPEG          /* 'JPEG' compressed (raw ) */
#endif
#define BI_JPGE FC('J','P','G','E') /* 'JPEG-ELS' compressed  */
#define BI_JPGL FC('J','P','G','L') /* 'JPEG Lossless */
#define BI_LJPG FC('L','J','P','G') /* PIC2 JPEG Lossless */
#define BI_PJPG FC('P','J','P','G') /* 'PJPG' progressive compressed (raw) */
#define BI_TGA1 FC('T','G','A','1') /* 'TGA1' Targa type 1 (cm) uncompressed */
#define BI_TGA2 FC('T','G','A','2') /* 'TGA2' Targa type 2 (bgr) uncompressed */
#define BI_TGA3 FC('T','G','A','3') /* 'TGA3' Targa type 3 (gray) uncompressed */
#define BI_TGA9 FC('T','G','A','9') /* 'TGA9' Targa type 9 (cm) RLE */
#define BI_TGAA FC('T','G','A','A') /* 'TGAA' Targa type 10 (bgr) RLE */
#define BI_TGAB FC('T','G','A','B') /* 'TGAB' Targa type 11 (gray) RLE */
#define BI_PICJ FC('P','I','C','J') /* 'PICJ' compressed (PIC JPEG) */
#define BI_PICL FC('P','I','C','L') /* 'PICL' compressed */
#define BI_PICP FC('P','I','C','P') /* 'PICP' compressed (PIC prog. JPEG) */
#define BI_PIC1 FC('P','I','C','1') /* 'PIC1' compressed */
#define BI_PIC4 FC('P','I','C','4') /* 'PIC4' compressed */
#define BI_PIC8 FC('P','I','C','8') /* 'PIC8' compressed */
#define BI_PICG FC('P','I','C','G') /* 'PICG' compressed */
#define BI_PICX FC('P','I','C','X') /* 'PICX' compressed */
#define BI_PC2J FC('P','C','2','J') /* 'PC2J' compressed (PIC2 JPEG) */
#define BI_PC2E FC('P','C','2','E') /* 'PC2E' compressed (PIC2 JPEG Els) */
#define BI_PC2L FC('P','C','2','L') /* 'PC2L' compressed */
#define BI_PC2P FC('P','C','2','P') /* 'PC2P' compressed (PIC2 prog. JPEG) */
#define BI_PC2N FC('P','C','2','N') /* 'PC2N' compressed (PIC2 prog. JPEG Els) */
#define BI_PC21 FC('P','C','2','1') /* 'PC21' compressed */
#define BI_PC24 FC('P','C','2','4') /* 'PC24' compressed */
#define BI_PC28 FC('P','C','2','8') /* 'PC28' compressed */
#define BI_PC2G FC('P','C','2','G') /* 'PC2G' compressed */
#define BI_PC2X FC('P','C','2','X') /* 'PC2X' compressed */
/* note BI_RGB is defined in bmp.h */
#define BI_RGB565 FC('R','5','6','5')   /* to recognize rgb 565 format */
#define BI_BMPO FC('B','M','P','O') /* 'BMPO' old-style BMP uncompressed */
                                    /* (OS/2 1.1 and 1.2) */
#define BI_BMPR FC('B','M','P','R') /* 'BMPR' new-style BMP RLE */
#define BI_PCX1 FC('P','C','X','1') /* 'PCX1' PCX 1-bit */
#define BI_PCX2 FC('P','C','X','2') /* 'PCX2' PCX 2-bit ('CGA') (cm) */
#define BI_PCX3 FC('P','C','X','3') /* 'PCX3' PCX 3-bit ('EGA') (cm) */
#define BI_PCX4 FC('P','C','X','4') /* 'PCX4' PCX 4-bit (cm) */
#define BI_PCX8 FC('P','C','X','8') /* 'PCX8' PCX 8-bit (cm) */
#define BI_PCXT FC('P','C','X','T') /* 'PCXT' PCX 24-bit */
#define BI_DCXZ FC('D','C','X','Z') /* 'DCXZ' DCX multi-image */
#define BI_TIF1 FC('T','I','F','1') /* 'TIF1' TIFF 1-bit */
#define BI_TIFM FC('T','I','F','M') /* 'TIFM' TIFF color-map (up to 8-bit) */
#define BI_TIFG FC('T','I','F','G') /* 'TIFG' TIFF gray-scale (up to 8-bit) */
#define BI_TIFC FC('T','I','F','C') /* 'TIFC' TIFF rgb (16,24,32-bit) */
#define BI_TIFZ FC('T','I','F','Z') /* 'TIFZ' TIFF multi-image */
#define BI_TIFJ FC('T','I','F','J') /* 'TIFJ' TIFF Jpeg */
#define BI_TIFL FC('T','I','F','L') /* 'TIFL' TIFF LZW */
#define BI_TIFK FC('T','I','F','K') /* 'TIFK' TIFF CMYK */
#define BI_TIFu FC('T','I','F','u') /* 'TIFu' TIFF unknown */
#define BI_GIFN FC('G','I','F','N') /* 'GIFN' GIF non-interlaced */
#define BI_GIFI FC('G','I','F','I') /* 'GIFI' GIF interlaced */
#define BI_GIFu FC('G','I','F','u') /* 'GIFu' GIF unknown */
#define BI_picPNG FC('P','N','G',' ') /* 'PNG ' PNG all flavors */
#if !defined(BI_PNG)
    /* MS started defining BI_PNG in the header files */
#define BI_PNG  BI_picPNG           /* 'PNG ' PNG all flavors */
#endif
#define BI_OS2  FC('O','S','2','U') /* 'OS2U' OS2 2.x+ uncompressed */
#define BI_TGA  FC('T','G','A',' ') /* 'TGA ' Targa generic */
#define BI_PCX  FC('P','C','X',' ') /* 'PCX ' PCX generic */
#define BI_TIF  FC('T','I','F',' ') /* 'TIF ' TIFF generic */
#define BI_PCD  FC('P','C','D',' ') /* 'PCD ' PhotoCD generic */
#define BI_G3   FC('G','3',' ',' ') /* 'G3  ' Raw G3 fax -- 1D */
#define BI_G32D FC('G','3','2','D') /* 'G32D' Raw G3 fax -- 2D */
#define BI_UYVY FC('U','Y','V','Y') /* 'UYVY' microsoft uyvy video subtype */
#define BI_YUY2 FC('Y','U','Y','2') /* 'YUY2' microsoft yuy2 video subtype */
#define BI_CMYK FC('C','M','Y','K') /* 'CMYK' 4-byte interlaced */
#define BI_WAVE FC('W','A','V','E') /* 'WAVE' wavelet */
#define BI_WAVP FC('W','A','V','P') /* 'WAVP' progressive wavelet */
#define BI_GR12 FC('G','R','1','2') /* 'GR12' 12-bit gray uncompressed,low-order bits in 16-bit field */
#define BI_DJVU FC('D','J','V','U') /* 'DJVU' AT&T DJVU(tm) */
#define BI_WSQ  FC('W','S','Q',' ') /* 'WSQ ' */
#define BI_IOCA FC('I','O','C','A') /* 'IOCA' */
#define BI_MDCA FC('M','D','C','A') /* 'MDCA' is MODCA */
#define BI_CALS FC('C','A','L','S') /* 'CALS' */
#define BI_PDF  FC('P','D','F',' ') /* PDF wrapper around G4 image */
#define BI_J2K  FC('J','2','K',' ') /* 'J2K ' JPEG 2000 */


/***************************************************************************\
*   This structure is used to convert to 16-bit DIBs.  Only r5:g5:b5 and    *
*   r5:g6:b5 are supported.                                                 *
\***************************************************************************/

#if 0
The following will not work in 32-bit environment because UINT is 4bytes
rather than the two bytes required. This is intended to be a two-byte
field for 16-bit video modes. If we use WORD here rather than UINT, we are
not ANSI compatible.  So for now we are just going to make RGB555 and
RGB565 be simply words rather than bitfields.
typedef struct {
        UINT    blue  : 5;      /* 00000000 000bbbbb */
        UINT    green : 5;      /* 000000gg ggg00000 */
        UINT    red   : 5;      /* 0rrrrr00 00000000 */
    } RGB555;

typedef struct {
        UINT    blue  : 5;      /* 00000000 000bbbbb */
        UINT    green : 6;      /* 00000ggg ggg00000 */
        UINT    red   : 5;      /* rrrrr000 00000000 */
    } RGB565;
#endif
typedef WORD    RGB555;
typedef WORD    RGB565;
typedef struct {
        BYTE    C;
        BYTE    M;
        BYTE    Y;
        BYTE    K;
    } CMYKQUAD;

/*.P*/
/* PixType determines physical bits per pixel. */
typedef BYTE    PIXEL_TYPE;
/* standard definition:  Sig = 0 */
#define PT_NONE     0x00        /* undefined or compressed pixels */
#define PT_CM       0x01        /* color mapped up to 256 colors */
#define PT_GRAY     0x02        /* gray scale up to 16 bits (Intel format) */
#define PT_GRAYM    0x03        /* gray scale 9 to 16 bits (Motorola format */
#define PT_RGB      0x04        /* RGB 24 or 48 bit (blue is low, red is high) */
#define PT_RGBM     0x05        /* RGB 48 bit (blue is low, red is high) (Motorola) */
#define PT_RGB555   0x06        /* RGB 16 bit (xrrrrrgggggbbbbb) */
#define PT_RGB565   0x07        /* RGB 16 bit (rrrrrggggggbbbbb) */
#define PT_CMYK     0x08        /* CMYK 32 bit (cyan is low, black is high) */
#define PT_GRAYA    0x12        /* gray scale up to 16 bits with alpha (Intel) */
#define PT_GRAYAM   0x13        /* gray scale 9 to 16 bits with alpha (Motorola) */
#define PT_RGBA     0x14        /* RGBA 32 bit (blue is low, alpha is high) */
#define PT_RGBAM    0x15        /* RGBA 64 bit (blue is low, alpha is high) (Motorola) */

#define PT_YUV      0x09        /* YUV 24 bit (Y is low byte, V is high bit) */
#define PT_YUYV     0x0A        /* 4 8-bit samples, U&V subsampled 2:1 */
#define PT_UYVY     0x0B        /* 4 8-bit samples, U&V subsampled 2:1 order differs from PT_YUYV */
#define PT_INT      0x0C        /* 32-bit signed 2's complement integer */
#define PT_FLOAT    0x0D        /* 32-bit IEEE floating point value */
#define PT_YCBCR    0x0E        /* YCbCr 24 bit (Y is low byte, Cr is high byte) */
                                /* 16 <= Y <= 235,  16 <= U,V <= 240 */
#define PT_RGBI     0x16        /* RGB 48 bit (blue is low, red is high) (Intel = little endian) */

#if 0
/* alternate definition:  Sig = 1 */
/* PIXEL_TYPE bits 0..3 represent the number of components-1, i.e., 1..16 */
/* all samples have the same bits/sample */
#define PT_CMAP     0x10        /* treat pixel as an index into the RGB color table */
#define PT_2BYTE    0x20        /* Use 2-byte field for samples, ignored if > 8 bits/sample */
#define PT_BIG_ENDIAN  0x40     /* bits 4 set if 2 bytes/sample and big-endian byte order */
#define PT_SIGNED   0x80        /* samples are signed values */
#endif

typedef BYTE    REGION_FLAGS;
/* standard definition:  Sig = 0 */
#define RF_TopDown  0x01        /* set if image stored top line first, bottom last */
#define RF_NonInter 0x02        /* set if image is non-interleaved (obsolete) */
/*  If PixType is PT_RGB or RF_CM2RGB set then the following apply */
#define RF_MakeGray 0x04        /* set if region to be treated as gray scale */
#define RF_SwapRB   0x08        /* set if Red and Blue components to be exchanged (RGB only)) */
/*  If PixType is PT_GRAY or PT_GRAYM or RF_MakeGray set then the following apply */
#define RF_Signed   0x10        /* set if bits in pixel are to be treated as signed int */
#define RF_2Byte    0x20        /* use 2-byte field for samples, ignored if > 8 bits/sample */
/*  If PixType is PT_CM then the following apply */
#define RF_CM2RGB   0x10        /* color mapped pixels are to be converted to RGB pixels */
#define RF_Channel  0x20        /* set if a single color channel is to be used */
#define RF_ChLo     0x40        /* if RF_Channel is set, ChHi,ChLo are two bits whose value, */
#define RF_ChHi     0x80        /* 0..3, indicates the color channel.  Channel 0 is the index.
                                   Channels 1..3 are the first..third ColorTable members. */
#if 0
/* alternate defineition:  Sig = 1 */
/* REGION_FLAGS determines the component transformation to be done */
#define RF_NONE     0x00        /* no transformation */
#define RF_RGB2GRAY 0x01        /* treat first 3 components as RGB, convert to Gray */
#define RF_RGB2YCbCr 0x02       /* treat first 3 components as RGB, convert to YCbCr,
                                    conversely if decompressing */
#define RF_RGB2YUV  0x03        /* treat first 3 components as RGB, convert to YUV
                                    conversely if decompressing */
#define RF_Sample   0xF0        /* Ignore all channels except (RF_Sample & 0x0F) */
#endif

/*  This structure is used to reference general-purpose image regions. */
typedef struct {
        BYTE         Sig;       /* 0 for standard definition, 1 for alternate definition (reserved) */
        BYTE         Interlace; /* 0 => None, 1 => PNG, 2 => Gif, 3 => line */
                                /* If PixType is PT_GRAY or PT_GRAYM then this also specifies */
                                /* subsampling; bits 7:5 specify horizontal and bits 4:2 */
                                /* specify vertical subsampling counts.  000..111 corresponds to */
                                /* skipping 0...7 samples.  I.e., 001 corresponds to 2:1 subsampling. */
                                /* This subsampling is ignored if any is implied by transformations. */
        BYTE         BitErr;    /* low-order bits to be dropped or treated as 0 (Grayscale only) */
        REGION_FLAGS Flags;     /* attributes of region */
        PIXEL_TYPE   PixType;   /* type of pixel which comprise this region */
        BYTE         Bpp;       /* meaningful Bits per pixel.  If bpp <= 8 then one byte per
                                   sample, else 2 bytes per sample (see RF_2Byte) */
        WORD         Width;     /* width of region in pixels > 0 */
        WORD         Height;    /* height of region in lines > 0 */
        SHORT        Stride;    /* width of area containing the region in bytes */
        DWORD        Offset;    /* byte offset of logical first line of region */
    } REGION;

/* A pointer to this structure is passed to PegasusLoadFromRes or set in
    PIC_PARM.LoadPath in order to load the opcode SSM from a list of
    buffered SSM's,
    Signature points to the string "\..\PicSsmList",
    Count is the number of PICSSM structures
    Ssms points to an array of PICSSM structures */
#define PICSSM_SIGNATURE    "\\..\\PicSsmList"
typedef struct
{
    char* Name;
    unsigned char* Buffer;
    unsigned long Length;
} PICSSM;
typedef struct
{
    char Signature[16];
    unsigned long Count;
    PICSSM* Ssms;
    char* LoadPath;
} PICSSMLIST;

/***************************************************************************\
*   These typedefs are used to represent non-callback states.               *
\***************************************************************************/

typedef LONG    OPERATION;
/* OPCODE offsets 0..9 are reserved for std opcodes */
/* OPCODE itself will be offset 10 */
/* MKOP allows the DLL loader to take opcode/1000 to construct a dll name */
#define STD_OPCODE_OFFSET ( 10 )
#define MKOP(x) ( (x) * 1000L + STD_OPCODE_OFFSET )

#define OP_SHELL   (MKOP(01))   /* opcode shell */

#define OP_D2S     (MKOP(10))   /* DIB to a Huffman Sequential JPEG */
#define OP_S2D     (MKOP(11))   /* Huffman Sequential JPEG to a DIB */
#define OP_D2SE    (MKOP(12))   /* DIB to Huffman or Els Sequential JPEG */
#define OP_P2D     (MKOP(13))   /* Huffman Sequential or Progressive JPEG to DIB */
#define OP_S2P     (MKOP(14))   /* Transform Seq. JPEG to Prog. JPEG */
#define OP_D2J     (MKOP(15))   /* DIB to Huffman Progressive or Sequential JPEG */
#define OP_P2S     (MKOP(16))   /* Transform Prog. JPEG to Seq. JPEG */
#define OP_D2JE    (MKOP(17))   /* DIB to Huffman or Els Progressive or Sequential JPEG */
#define OP_SE2D    (MKOP(18))   /* Huffman or Els Sequential JPEG to DIB */
#define OP_JE2D    (MKOP(19))   /* Huffman or Els Sequential or Progressive JPEG to DIB */

#define OP_EXP4    (MKOP(21))   /* IM4 - Expand IM4 to a DIB */
#define OP_ROR     (MKOP(22))   /* ROR - Reorient/requantize to Huffman JPEG */
#define OP_RORE    (MKOP(23))   /* RORE - Reorient/requantize to (ELS or Huffman) JPEG */
#define OP_REQUANT (MKOP(24))   /* REQUANT _ ROR, but requantize only */

#define OP_LIMP    (MKOP(42))   /* Lossless IMage Pack */
#define OP_LIME    (MKOP(43))   /* Lossless IMage Expand */
#define OP_LIP     (MKOP(44))   /* Lossless IMage Pack (Ver 2)*/
#define OP_LIE     (MKOP(45))   /* Lossless IMage Expand (Ver 2)*/
#define OP_LIP3    (MKOP(46))   /* Lossless IMage Pack (Ver 3) */
#define OP_LIE3    (MKOP(47))   /* Lossless Image Expand (Ver 3) */
#define OP_PNGP    (MKOP(48))   /* Portable Network Graphics Pack */
#define OP_PNGE    (MKOP(49))   /* Portable Network Graphics Expand */

#define OP_CLEAN   (MKOP(50))   /* various cleanup operations */
#define OP_TIDP    (MKOP(52))   /* IDP Transmitter */
#define OP_RIDP    (MKOP(53))   /* IDP Receiver */


#define OP_LIP3PLUS (MKOP(62))  /* Lossless Image Pack (Ver 3) + 9-16 bit lossless JPEG */
#define OP_LIE3PLUS (MKOP(63))  /* Lossless Image Expand (Ver 3) + 9-16 bit lossless JPEG */
#define OP_D2LJ    (MKOP(62))   /* temporary pack lossless jpeg */
#define OP_LJ2D    (MKOP(63))   /* temporary expand lossless jpeg */
#define OP_D2SEPLUS (MKOP(64))  /* pack lossy jpeg 8/12-bit gray or 24-bit rgb */
#define OP_SE2DPLUS (MKOP(65))  /* expand lossy jpeg 8/12-bit gray or 24-bit rgb */
#define OP_J2KP     (MKOP(68))  /* pack JPEG 2000 */
#define OP_J2KE     (MKOP(69))  /* expand JPEG 2000 */


#define OP_D2DJVU  (MKOP(70))   /* AT&T DJVU(tm) Pack */
#define OP_DJVU2D  (MKOP(71))   /* AT&T DJVU(tm) Expand */
#define OP_D2MDCA  (MKOP(72))   /* MODCA and CALS Pack */
#define OP_MDCA2D  (MKOP(73))   /* MODCA and CALS Expand */

#define OP_D2F     (MKOP(80))   /* Convert a DIB to supported file format */
#define OP_F2D     (MKOP(81))   /* Convert a supported file format to DIB */
#define OP_UTL     (MKOP(82))   /* Utility functions (dib, palette, histogram) */
#define OP_TIFEDIT (MKOP(83))   /* multi-image TIF editing features */
#define OP_D2FPLUS (MKOP(84))   /* OP_D2F + GIF */
#define OP_F2DPLUS (MKOP(85))   /* OP_F2D + GIF + LZW/TIFF */
#define OP_ZOOM2   (MKOP(88))   /* Change image dimensions */
#define OP_ZOOM    (MKOP(89))   /* Change image dimensions */

#define OP_D2W     (MKOP(90))   /* compress DIB to wavelet */
#define OP_W2D     (MKOP(91))   /* expand wavelet to DIB */
#define OP_WSQP    (MKOP(92))   /* Wavelet Scalar Quantization Pack */
#define OP_WSQE    (MKOP(93))   /* Wavelet Scalar Quantization Expand */
#define OP_D2WV    (MKOP(94))   /* compress DIB to wavelet video */
#define OP_WV2D    (MKOP(95))   /* expand wavelet video to DIB */
#define OP_D2WOLD  (MKOP(96))   /* 2.00.66 version using fwrz 1.08f lib */

#define OP_DEBUG   (MKOP(100))  /* tracing/logging functions */

/* reserved >> */
#define OP_DISPATCH (MKOP(0))
#define OP_QRY   (MKOP( 1))     /* Query a buffer for extensive information */
#define OP_CMAP  (MKOP( 2))     /* Create a Color Map from a buffer */
#define OP_PACKJ (MKOP(10))     /* IMJ - Pack a DIB to Huffman Sequential JPEG */
#define OP_EXPJ  (MKOP(11))     /* IMJ - Expand Huffman Sequential JPEG to a DIB */
#define OP_PACK4 (MKOP(20))     /* IM4 - Pack a DIB to IM4 */
#define OP_PACK1 (MKOP(30))     /* IM1 - Pack a DIB to IM1 */
#define OP_EXP1  (MKOP(31))     /* IM1 - Expand IM1 to a DIB */
#define OP_PACK8 (MKOP(40))     /* IM8 - Pack a DIB to IM8 */
#define OP_EXP8  (MKOP(41))     /* IM8 - Expand IM8 to a DIB */

/*.P*/
/***************************************************************************\
*                                                                           *
*   This structure represents the visual orientation of the image.  It      *
*   is independent of the file's storage orientation (bottom-to-top         *
*   with the bottom of the image stored first in the file or top-to-        *
*   bottom with the top of the image stored first in the file).  Let        *
*   TOP_DOWN be TRUE iff the image is stored top-to-bottom, i.e., the       *
*   biHeight of the image is < 0.  Let VO be the Visual Orientation as      *
*   represented below.  Finally let FO be the File Orientation - the        *
*   orientation field as stored within the PIC file (FO should equal        *
*   VO but for historical reasons it actually indicates whether the         *
*   file should be inverted or not).  Then have:  FO = VO ^ O_inverted.     *
*                                                                           *
*   ÚÄÄÂÄÄÂÄÄÂÄÄÂÄÄÂÄÄÂÄÄÂÄÄ¿                                               *
*   ³ 0³ 0³ x³ x³ 0³ x³ x³ x³    visual ORIENTATION values                  *
*   ÀÄÄÁÄÄÁÄÄÁÄÄÁÄÄÁÄÄÁÄÄÁÄÄÙ                                               *
*     7  6  5  4  3  2  1  0                                                *
*           ³  ³     ³  ³  ÀÄ set if reflected across horizontal line       *
*           ³  ³     ³  ÀÄÄÄÄ set if rotated right 90ø                      *
*           ³  ³     ÀÄÄÄÄÄÄÄ set if rotated right 180ø                     *
*           ³  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄ set if appears white on black                 *
*           ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ set if bit order is reversed                  *
*   Any rotating is done before the reflection.                             *
*                                                                           *
*   If Õ is the standard image then the visual orientations are:            *
*     Õ   Ô   ·   ½   ¾   ¸   Ó   Ö   (if displayed without adjustment)     *
*                                                                           *
*   This structure is also used to indicate the visual characteristics      *
*   of 1-bit images, i.e., IMG (Group 3 and Group 4 fax).                   *
*                                                                           *
\***************************************************************************/
typedef LONG    ORIENTATION;
#define O_normal    0
#define O_inverted  1
#define O_r90       2
#define O_r90_in    3
#define O_r180      4
#define O_r180_in   5
#define O_r270      6
#define O_r270_in   7
#define O_w_on_b    0x10
#define O_bit_rev   0x20


typedef LONG    SUBSAMPLING;
#define SS_111      0
#define SS_211      1   /* Cb and Cr are 2-to-1 subsamp. horiz., not vert. */
#define SS_411      2   /* Cb and Cr are 2-to-1 subsamp. horiz and vert. */
#define SS_211v     3   /* Cb and Cr are 2-to-1 subsampled vert., not horiz. */
                        /* For all other cases subsampling is defined as: */
                        /* HY<<27 | VY<<22 | HCb<<17 | VCb<<12 | HCr<<7 | VCr<<2, */
                        /* where H and V are the subsampling factors defined */
                        /* in JPEG spec. */

typedef DWORD   PICFLAGS;

#define PF_IsGray                   0x00000001L
        /* (Pack) Set if image is gray-scale */
#define PF_TopDown                  0x00000002L
        /* (Exp and Pack) Set if image is top_down (dib buffers then run backwards) */
#define PF_OptimizeHuff             0x00000004L
        /* (Pack, not Els coded) Set if huff codes are to be optimized (meaningless if ELS coded */
#define PF_IsTransparency           0x00000004L
        /* (GIF) image has a transparent color index */
#define PF_IncludeBMPHead           0x00000008L
        /* Include BITMPAINFOHEADER and palette in output stream if appropriate */
#define PF_ReturnYIfFirst           0x00000010L
        /* (Seq Exp) If image is multiscan sequential color lossy jpeg and */
        /* first scan is Y only, return gray image first and defer(RES_Y_DONE_FIRST). */
        /* App can then reset Put buffer before continuing to get color image. */
#define PF_YuvOutput                0x00000020L
        /* (Seq Expand). Causes uyvy or yuy2 output (type chosen in DIB_OUTPUT below) */
#define PF_BigEndian                0x00000040L
        /* (ROR/D2J Exif Motorola byte-ordered tags instead of Intel */
#define PF_ExpandThumbnail          0x00000080L
        /* (J2D) expand exif thumbnail instead of primary image */
#define PF_NoImageChanges           0x00000080L
        /* (ROR) changing Exif tags only -- no change to other image data */
#define PF_ConvertGray              0x00000100L
        /* (Exp)  Set if image is to be gray-scale */
#define PF_IsProtected              0x00000100L
        /* set if PegasusQuery detects that the file is protected */
#define PF_NoDibPad                 0x00000200L
        /* (Exp and Pack)  Set if DIB is not (or not to be) DWORD aligned */
#define PF_Dither                   0x00000400L
        /* (Exp)  Set if image is to be dithered */
#define PF_MakeColors               0x00000800L
        /* (Exp and Pack)  Set if palette is to be made */
#define PF_CreateDibWithMadeColors  0x00001000L
        /* (Exp)  Set if a dib is to be filled with made colors */
#define PF_OnlyUseMadeColors        0x00002000L
        /* (Exp)  Set if ONLY made colors are to be used (no dib output on "first */
        /* pass" using image or user colors) */
#define PF_App1Pal                  0x00004000L
        /* (Pack) Set if palette is to be in APP1 marker code */
#define PF_DibKnown                 0x00008000L    /* obs. name */
#define PF_WidthPadKnown            0x00008000L
        /* (exp and pack) Set if WidthPad is set by app before */
        /* init, NOT to be calc. by level 2 during init. */
#define PF_Yield                    0x00010000L
        /* (WSQ)  Set if Yield should be called during operation */
#define PF_YieldGet                 0x00010000L
        /* (P&E)  Set if Yield during data get (CB only) */
#define PF_YieldPut                 0x00020000L
        /* (P&E)  Set if Yield during data put (CB only) */
#define PF_Poll                     0x00040000L
        /* (Comm) Set if return instantly when nothing received */
#define PF_NoGets                   0x00080000L
        /* (Exp prog.) Set if no data to be read - just use what is in buffer now. */
#define PF_EOIFound                 0x00100000L
        /* (Exp prog.) Set if JPEG EOI marker reached in progressive. */
#define PF_NonInterleaved           0x00100000L
        /* (LL) The image should be planar packed: all reds, then greens, then blues */
#define PF_NoCrossBlockSmoothing    0x00200000L
        /* (Exp prog. and seq) */
#define PF_AutoIgnoreBadSuffix      0x00080000L
        /* (LL) Determine if invalid suffix after -32768, set PF_IgnoreBadSuffix */
#define PF_IgnoreBadSuffix          0x00200000L
        /* (LL) Ignore invalid suffix after -32768  */
#define PF_LIPUseYCbCrForColors     0x00200000L
        /* (Pack only) Use YCbCr space if making colors - often chooses better
             colors. If not making colors, it is ignored. This flag is not
             used by JPEG expand, as it makes colors from YCbCr always. */
#define PF_DecodeToDataEnd          0x00400000L
        /* (Exp prog.) */
#define PF_SynchRestart             0x00400000L
        /* (LL) If a restart error occures, resynch at next marker */
#define PF_MultiImage               0x00800000L
        /* (OP81) -- At RES_DONE, if PF_EOIFound is 0, then: 0 means next dib */
        /* is a progressive image; 1 means next dib is a new image from the */
        /* file (e.g., multi-image TIFF or GIF files) */
#define PF_JPEGFileOnly             0x00800000L
        /* (LL) Do not make PIC2 file for mode J, make JPEG only,
           (F2D) for TIFF/JPEG input, output JPEG image instead of DIB */
#define PF_ElsCoder                 0x00800000L
        /* (Pack and ROR and Exp) Use ELS rather than Huffman entropy coding */
        /* For Pack and ROR, this is set by app to determine output. For Exp, it is merely returned by Pegasus describing input. */
#define PF_SwapRB                   0x01000000L
                /* Exp and Pack. Swap RB in uncompressed out and in, if color > 8 bpp */
#define PF_ApplyTransparency        0x01000000L
        /* (LL) Don't write transparent color index pixels */
#define PF_Quickview12              0x02000000L
                /* Seq.Exp . Decode a 12-bit per component jpeg by keeping only high 8 bits. */
#define PF_UserDelay                0x02000000L
        /* (LL) Mouse or keyboard allows image replacement, also F2D/D2F GIF User Input Flag in graphics extension block */
#define PF_OptimalFilter            0x02000000L
        /* (PNG) the best of filters 0..4 is used for each line */
#define PF_ReverseInputByteOrder    0x02000000L
        /* (jpeg pack, 12-bit grayscale case only)Use this if the word input is in big-endian (Motorola) order. */
#define PF_AllocateComment          0x04000000L
        /* (LL) RES_ALLOCATE_COMMENT_BUF on every comment */
        /* (LL) RES_ALLOCATE_APP2_BUF on every app */
        /* (LL) RES_ALLOCATE_OVERTEXT_BUF on every overtext */
#define PF_IsBMP                    0x04000000L
        /* (PNG) the source file is BMP */
#define PF_ColorTableProvided       0x08000000L
        /* (LL) Use provided ColorTable, not one in file */
#define PF_UsedMMX                  0x08000000L
        /* (SJPEG) Expand MMX was detected and used */
#define PF_MakeBMP                  0x08000000L
        /* (PNG) convert output to a BMP image (set PF_SwapColors, */
        /*  do 16->8 bit conversion, DWORD padding, etc.) */
#define PF_ZoomToSize               0x10000000L
        /* (LL) Zoom all layers to MinLayerWanted size */
#define PF_SwapColors               0x10000000L
        /* (PNG) convert RGB to BGR for 24-, 32, 48-, and 64-bit colors */
#define PF_ConvertToColor           0x10000000L
        /* (ROR) convert Grayscale to Color (for lossy jpegs) */
#define PF_ContextInterpolation     0x20000000L
        /* (LL) Use best guess for interpolation */
#define PF_DidPolish                0x20000000L
        /* (LL) Able to do all PIC2 forward updates */
#define PF_BlendWithBuffer          0x20000000L
        /* (PNG) if alpha or transparency, blend color with buffer vs background color */
#define PF_WordQtbl                 0x20000000L
        /* (jpeg pack, 12-bit grayscale case only)The user-inputted qtbl is words, values up to 32767.  */
#define PF_SkipHuffman              0x40000000L
        /* (LL) Don't calculate optimal Huffman table */
#define PF_HaveColorMap             0x40000000L
        /* (P&E) Must have ColorMap != NULL and colors in color table */
#define PF_HaveTransparency         0x40000000L
        /* (PNG) Transparency value is available */
#define PF_Log		                0x40000000L
        /* (WSQP-PIC internal use only) Create a log file for tracing */
#define PF_UseYCbCrForColors        0x80000000L
        /* (Pack only) Use YCbCr space if making colors - often chooses better
             colors. If not making colors, it is ignored. This flag is not
             used by JPEG expand, as it makes colors from YCbCr always. */
#define PF_HaveWatermark            0x80000000L
        /* (LL) OverText points to watermark not CT */
#define PF_HaveBackground           0x80000000L
        /* (PNG) Background value is available */

/* following PF2_ flag values reserved (can't be overloaded by any opcode) */
/*#define PF2_P2AndP3OptDisable   (0x20L)   \\ disable P2 optimizations */
/*#define PF2_P3OptDisable        (0x40L)   \\ disable P3 optimizations */
/*#define PF2_UseResPoke          (0x80L)   \\ disable RES_POKE */
/*#define PF2_MMXDisable         (0x100L)   \\ disable MMX, P2 and P3 optimizations */

typedef DWORD   THUMBNAIL;
#define THUMB_NONE      0
#define THUMB_4         1
#define THUMB_16        2
#define THUMB_64        3
#define THUMB_4F        4 /* use all dct coeffs in making 1/4 area thumbnail */
#define THUMB_256       4

/***************************************************************************\
*  This typedef designates the currently supported input file types.        *
\***************************************************************************/
typedef LONG    JPEG_TYPE;
#define JT_PIC          0
#define JT_BMP          1
#define JT_RAW          2
#define JT_PIC2         3
#define JT_EXIF         4

typedef LONG    REQUEST;
#define REQ_INIT        1
#define REQ_EXEC        2
#define REQ_CONT        3
#define REQ_TERM        4

/*.P*/
typedef LONG    RESPONSE;
#define RES_DONE                    1
#define RES_EXEC_DONE               1
#define RES_ERR                     2
#define RES_GET_DATA_YIELD          3
#define RES_PUT_DATA_YIELD          4
#define RES_YIELD                   5
#define RES_COLORS_MADE             6
#define RES_PUT_NEED_SPACE          7
#define RES_GET_NEED_DATA           8
#define RES_NULL_PICPARM_PTR        9
#define RES_SEEK                    10
#define RES_AUX_NEEDED              11
#define RES_ALLOCATE_APP2_BUF       12
#define RES_ALLOCATE_COMMENT_BUF    13
#define RES_ALLOCATE_OVERTEXT_BUF   14
#define RES_HAVE_COMMENT            15
#define RES_POKE                    16
#define RES_EXTEND_PIC2LIST         17
#define RES_Y_DONE_FIRST            18
#define RES_PUTQ_GET_NEED_DATA      19
#define RES_INIT_DONE               20
#define RES_PIC_PARM_INIT_DONE      21
#define RES_EXTEND_RCVRSTATE        22



/***************************************************************************\
*   Instance specific work area used by level 1 and level 2.                *
\***************************************************************************/
#define SIZE_PIC_STACK  8192

#ifdef __cplusplus
  typedef struct {
        BYTE        reserved;
  } WORK_AREA;
#else
  typedef struct {
        jmp_buf     AppState;
        jmp_buf     PicState;
        BYTE        PicStack[SIZE_PIC_STACK];
        BYTE        InstState[564];
        BYTE        Padding[4];
        BYTE        WorkArea[1];
            /* other stuff - should never allocate sizeof(WORK_AREA) bytes! */
  } WORK_AREA;
#endif

typedef struct {
        BYTE PICHUGE *FrontEnd; /* Auxiliary, points just beyond contiguous data */
        BYTE PICHUGE *Start;    /* Points to start of queue */
        BYTE PICHUGE *Front;    /* Points to start of data in the queue */
        BYTE PICHUGE *Rear;     /* Points to start of empty space in the queue */
        BYTE PICHUGE *End;      /* Points just beyond queue buffer */
        BYTE PICHUGE *RearEnd;  /* Auxiliary, points just beyond contiguous space */
        DWORD        QFlags;    /* Flags for queue operation */
    } QUEUE;

#define Q_EOF       1           /* Set iff End-of-file reached on input */
#define Q_REVERSE   2           /* Set iff queue is reversed */
#define Q_IO_ERR    4           /* Set iff input or output error occurred */
#define Q_READ_WRAP 8           /* Set iff the Rear pointer has wrapped to Start */
#define Q_DID_WRITE 16          /* Set for Put iff RES_PUT_NEED_SPACE defer has been issued */
#define Q_DID_READ  16          /* Set for Get iff RES_GET_NEED_DATA defer has been issued */
#define Q_READING   32          /* set while level 2 is reading from Put queue, else clear */
#define Q_INIT      64          /* indicates initialization should be done in the queue */

typedef struct {
        LONG         Width;     /* Width of image in pixels for given layer */
        LONG         Height;    /* Height of image in pixels for given layer */
        LONG         Stride;    /* Width of image buffer in bytes for given layer */
        LONG         Size;      /* Compressed size of layer (BYTES) */
        BYTE PICFAR *Handle;    /* Level 2 pointer to a buffer */
    } LAYER;

/*.P*/
/***************************************************************************\
*   Variables to/from PegasusQuery                                          *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*NOTE: this is used by PegasusQuery -- there is NO OPCODE for query */
#if defined(_MSC_VER)
        CDInfo PICHUGE *CDInfo;     /* PCD Information */
#else
        CDInfo PICHUGE* CDInfoData; /* GNU doesn't seem to like the name space collision
                                        (neither do we but we can't break the MSC compile) */
#endif
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* Only PF_IsGray is relevant */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         NumOfPages;    /* # of pages in document (G3/G4) */
        LONG         PageNum;       /* Page # of 1st (or only) page (G3/G4) */
        DWORD        BitFlagsReq;   /* Bitmapped flags to indicate which */
                                    /*   values PegasusQuery should try to */
                                    /*   ascertain.  Set by the user. See */
                                    /* QBITxxx below. */
        DWORD        BitFlagsAck;   /* Bitmapped flags to indicate which */
                                    /*   of the requested values PegasusQuery */
                                    /*   was able to ascertain.  Note that */
                                    /*   some fields may be determined, and */
                                    /*   the resulting bits set, even if not */
                                    /*   specifically requested. */
                                    /* See QBITxxx below. */
        DWORD        ImageSize;     /* Size of image.  See QBIT_IMAGESIZE below. */
        DWORD        AuxSize;       /* Size required to hold all auxillary */
                                    /*   data blocks. */
        DWORD        ImageNum;      /* Image number we're requesting query on */
                                    /*   (1 is 1st image) -- this is ignored */
                                    /*   for files that only contain 1 image */
                                    /*   (bmp, pcx, etc and single-image */
                                    /*   tif, gif, etc) */
        DWORD        NumImages;     /* Number of images in file, if known */
        DWORD        SOIMarker;     /* Offset in file of SOI marker if tiff jpeg */
        SUBSAMPLING  SubSampling;   /* if determinable and JPEG */
        REGION       Region;
        BYTE         LumFactor;     /* if present and JPEG */
        BYTE         ChromFactor;   /* if present and JPEG */
        BYTE         AllowedBitErr; /* where significant */
        BYTE         Padding1;
        WORD         ClusterHeaderVersion;
        WORD         DjVuImageType; /* see ImageType in DJVU_UNION */
    } PEGQUERY;

#define PF2_IsExif                  0x00000001L
        /* (PegasusQuery) file has APP1 marker with Exif signature */

#define QBIT_BISIZE 0x1
        /* Head.biSize -- this is the size of the bitmap info header.  It */
        /* will generally be "sizeof (BITMAPINFOHEADER)", except in the */
        /* case of .BMP files, where it may be larger. */
#define QBIT_BIWIDTH 0x2
        /* Head.biWidth -- this is the width of the image in pixels. */
#define QBIT_BIHEIGHT 0x4
        /* Head.biHeight -- this is the height of the image in pixels. */
        /* This value may be negative.  If so, it indicates that if the */
        /* image is converted to a DIB via opcode 81, it will be upside */
        /* down.  (Opcode 81 may generate upside-down DIBS.) */
#define QBIT_BIPLANES 0x8
        /* Head.biPlanes -- this will always be 1. */
#define QBIT_BIBITCOUNT 0x10
        /* Head.biBitCount -- total number of bits per pixel for the */
        /* image.  May be wierd (e.g., 7).  When opcode 81 generates a */
        /* DIB, it will create only 1, 4, 8, 16, or 24 bpp images. */
#define QBIT_BICOMPRESSION 0x20
        /* Head.biCompression -- this is the basic file type.  See the */
        /* BI_???? manifest constants near the top of this file. */
#define QBIT_BISIZEIMAGE 0x40
        /* Head.biSizeImage -- this is the size of the image data if */
        /* converted to a DIB.  Pixels will be padded out to 1, 4, 8, */
        /* 16, or 24 bpp, and scanlines will be padded out to a 4-byte */
        /* boundary. */
#define QBIT_BIXPELSPERMETER 0x80
        /* Head.biXPelsPerMeter -- generally of no use */
#define QBIT_BIYPELSPERMETER 0x100
        /*Head.biYPelsPerMeter -- generally of no use */
#define QBIT_BICLRUSED 0x200
        /* Head.biClrUsed -- a RGB image may have any value here, or */
        /* 0 (if non-zero, this is presumably some sort of "suggested" */
        /* palette).  For palettized images, this is the number of colors */
        /* in the palette and MUST be >=2 and <= (1 << bpp). */
#define QBIT_BICLRIMPORTANT 0x400
        /* Head.biClrImportant -- generally of no use. */
#define QBIT_IMAGESIZE 0x800
        /* ImageSize -- size of the input "file".  This is the number of */
        /* bytes, starting from the beginning of the "file", which must be */
        /* accessed in order to get all header information and image data */
        /* in order to display an image.  Therefore, other auxillary */
        /* information that may be present in the file, but not required */
        /* for image display (e.g., author, comments, etc) will not be */
        /* included in this size.  Note that for 'random' formats such as */
        /* TIFF, it is possible that this space could contain information */
        /* for other images. */
#define QBIT_AUXSIZE 0x1000
        /* AuxSize -- size of the buffer required to read in all */
        /* 'auxillary' information (comments, gamma curves, etc.) in */
        /* preparation for an opcode 81 call.  Note that opcode 81 does */
        /* not require that this be known in advance, but things are more */
        /* efficient if it is known. */
#define QBIT_NUMIMAGES 0x2000
        /* NumImages -- number of images in the file.  Most often, 1. */
        /* This count may not be obtainable with from a small amout of data. */
#define QBIT_COMMENT 0x4000
        /* Iff Comment field set in PIC_PARM */
#define QBIT_PALETTE 0x08000L
        /* Set this if you want the palette returned in ColorTable */
        /* if possible. */
#define QBIT_SOIMARKER 0x10000L
                /* Set if we found SOIMarker in a tiff jpeg file */
#define QBIT_PICJPEG 0x20000L
#define QBIT_REGION  0x40000L
        /* set if u.QRY.Region is filled in */
#define QBIT_PCDINFO 0x80000L
        /* Located and recorded PCD Information */


/* for progressive JPEG */
typedef struct {
        /* NOTE: Co = 3 (interleaved yCbCr) is allowed ONLY for dc scans. */
        /* Also, for now, the dc scans MUST be interleaved for color image) */
        LONG   Co; /* Component indicator(0=y, 1=Cb, 2=Cr, 3=interleaved YCbCr) */
        LONG   Ss; /* dct coeff. index of start of scan */
        LONG   Se; /* dct coeff. index of end of scan ( 0 indicates dc) */
        LONG   Ah; /* (see JPEG spec) */
        LONG   Al; /* (see JPEG spec) */
        } SCAN_PARM;


/***************************************************************************\
*   Variables for reading DIBS (Operation D2S)                              *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Information prior to REQ_INIT */
        BYTE PICHUGE *AppField;     /* points to user info of AppLen bytes */
        BYTE PICHUGE *QTable;       /* NULL or points to Q-table values;
                                     Not in zig_zag order! There only need be   64 bytes for
                                     grayscale input.  For color,   first 64 bytes are for
                                     luminance(Y) and the   next 64 for Cb. If this is
                                     followed by a zero byte, the same 64 are used for Cr
                                     as were for Cb.  If not, it is assumed that the 64
                                     bytes following the 64 Cr quant.   bytes are used for Cb.
                                     Thus in the color case, this table must be at least 129
                                     bytes, and 192 bytes if there are 3 different quant. tables. */
        SCAN_PARM PICHUGE *ScanParms;    /* For progressive JPEG */
        BYTE PICHUGE *ExifThumbnail; /* pointer to Jfif-format thumbnail image for Exif output if
                                     desired.  Error unless unless JpegType == JT_EXIF &&
                                     ExifThumbnailToMake == THUMB_NONE */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* PF_IsGray, PF_IsNotDibPad, PF_OptimizeHuff,etc */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         PrimClrToMake; /* Desired number of colors in primary palette */
        LONG         SecClrToMake;  /* Desired number of colors in secondary palette */
        LONG         LumFactor;     /* Luminance compression factor 0..255.
                                    Ignored if QTable above is not NULL. */
        LONG         ChromFactor;   /* Chrominance compression factor 0..255 */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling */
        JPEG_TYPE    JpegType;      /* JT_ fields, desired PIC/BMP/RAW/PIC2 JPEG file type. JT_BMP not supported. */
        LONG         AppFieldSize;  /* Size of AppField in bytes. */
        LONG         AppFieldLen;   /* 0 or length of data within AppField */
        LONG         NumOfPages;    /* Number of pages in document */
        LONG         PageNum;       /* Page Number of first (or only) page */
        LONG         Context;       /* number of context pts (IM1, IM4, & IM8) */
        /*  Derived information about the Uncompressed Image after REQ_INIT */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip */
        LONG         WidthPad;      /* Actual byte size of UI line (with any pad) */
        LONG         NumProgScans;  /* for progressive JPEG */
        LONG         ExifAppsToKeep;
            /* bit 0 to output JFIF App0 marker
               bit 1 to output PIC App1 maker (not recommended) */
        DWORD     ExifThumbnailLen;
            /* length of jiff thumbnail provided at ExifThumbail.  Error unless
                JpegType == JT_EXIF && ExifThumbnailToMake == THUMB_NONE */
        THUMBNAIL ExifThumbnailToMake;
            /* != THUMB_NONE to create a thumbnail in Exif output.
                (requires and uses OP_S2D or OP_SE2D and OP_D2S or OP_D2SE)
                error unless JpegType == JT_EXIF && ExifThumbnailLen == 0 */
        LONG         AppsToOmit;
            /* bit 0 on ->  do not put a JFIF App0 marker in JPEG output stream.
                 bit 1 on -> do not put a PIC App1 marker in JPEG output stream. */
        LONG         RightShift12; /* only used for compressing 2 byte-per-pixel gray
                images to 12-bit JPEG format.  This is how much the data needs to
                be right-shifted so that the high 4 bits of the word are guaranteed to
                be 0. It can take values from   0 through 4. For example if the
                data were left-justified in the word, this parameter would
                be set to 4. */
        DWORD        ResolutionUnit;
                                    /* 0 = none (X/Y specify aspect ratio if != 0)
                                        else if X or Y are 0 then biX/YPelsPerMeter
                                        in PIC_PARM.Head are used */
        DWORD        XResolution;   /* horizontal pixels/ResolutionUnit */
        DWORD        YResolution;   /* vertical pixels/ResolutionUnit */
        } DIB_INPUT;


/* defined above */
/* #define PF2_Cosited                 0x00000001L */
		/* (jpeg pack and exp)Applies for now to 211 (horizontal 2 to 1) Chrom. sampling only.  */
		/* For exp, must also set PF2_SmoothUpsample to have any effect. */
#define PF2_UseResPoke  (0x00000080L)   /* for PF_MakeColors, Sequential JPEG pack used to require that
                                        you set the colors in the JPEG header yourself following
                                        compression.  If your Put queue is not large enough to hold
                                        the entire compressed image, setting this flag allows JPEG
                                        pack to use RES_POKE responses to prompt you through setting
                                        these colors.  The Put queue must be at least
                                        (PrimClrToMake+SecClrToMake)*sizeof(RGBTRIPLE) bytes.
                                        For a PIC2 file, you will get a RES_POKE even if this flag
                                        is not set and your Put queue < compressed size */
/* defined for ROR */
/* #define PF2_OmitISOStandardHuffmanTbls 0x00000008L */
    /* If PF_OptimizeHuff and PF_ElsCoder are clear, then do not write
        the Huffman code markers (FFC4) to the output jpeg stream; this is primarily
        for MJPEG streams because the result doesn't conform to the still-image jpeg standard */
#define PF2_CompressedYUV           (0x00000010L)
		/* (jpeg pack and exp) yuv is compressed to 16-235 lum and 16-240 chrom.  */

/*.P*/
/***************************************************************************\
*   Variables for writing DIBs (Operations S2D, P2D)                   *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Information prior to REQ_INIT */
        BYTE PICHUGE *AppField;     /* points to user info of AppLen bytes */
        BYTE PICHUGE *MadeColorTable;
        BYTE PICHUGE *PrecisionReq; /* PJPEG - 192 byte array of precision requested */
        BYTE PICHUGE *ColorMap;     /* points to pre-allocated 32K area for ColorMap */
        BYTE PICHUGE *ExifThumbnail;/* exif thumbnail buffer of ExifThumbnailLen */
        BYTE PICHUGE *GrayMap12Bit; /* If not NULL, Remaps 12-bit grayscale jpgs; will be assumed to be
             words rather than bytes is DibSize is 16 rather than 8.*/
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* PF_MakeGray, PF_NoDibPad, PF_Dither etc*/
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         DibSize;       /* 1/4/8/16/24/32, may change if gray image */
        THUMBNAIL    Thumbnail;     /* 0/1/2/3 => none / 1/4 / 1/16 / 1/64 thumbnail */
        LONG         NumScansReq;   /* PJPEG User's requested number of scans */
                                    /*   (0 means do all) */
        LONG         NumScansDone;  /* PJPEG Scans actually done upon return */
        LONG         NumBytesDone;  /* PJPEG Bytes comprising the scans returned */
        LONG         GraysToMake;   /* if > 0 then number of gray levels to make */
        LONG         PrimClrToMake; /* Desired number of colors in primary palette */
        LONG         SecClrToMake;  /* Desired number of colors in secondary palette */
        /*  Derived information about the Uncompressed Image after REQ_INIT */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip */
        LONG         WidthPad;      /* Actual byte size of UI line (with any pad) */
        LONG         LumFactor;     /* Luminance compression factor 0..255 */
        LONG         ChromFactor;   /* Chrominance compression factor 0..255 */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling */
                                    /*   (returned by Pegasus) */
        JPEG_TYPE    JpegType;      /* (returned by Pegasus). JT_ fields, PIC/BMP/RAW/PIC2 JPEG file type (JT_BMP not supported)*/
        LONG         AppFieldSize;  /* Size of AppField in bytes. */
        LONG         AppFieldLen;   /* 0 or logical length of Application data */
        LONG         NumOfPages;    /* Number of pages in document */
        LONG         PageNum;       /* Page Number of first (or only) page */
        LONG         Context;       /* number of context pts (IM1, IM4, & IM8) */
        LONG         DitherType;    /* 0 = Floyd-Steinberg, 1 = fast-error-diffusion */
        LONG         YuvOutputType; /* BI_UYVY or BI_YUY2.  Ignored unless PF_YuvOutput is set. */
        LONG         ExifThumbnailLen; /* size of ExifThumbnail buffer */
        DWORD        ResolutionUnit;
                                    /* 0 = none (X/Y specify aspect ratio if != 0)
                                        else if X or Y are 0 then biX/YPelsPerMeter
                                        in PIC_PARM.Head are used */
        DWORD        XResolution;   /* horizontal pixels/ResolutionUnit */
        DWORD        YResolution;   /* vertical pixels/ResolutionUnit */
        } DIB_OUTPUT;

#define PF2_Cosited                 0x00000001L
		/* (jpeg pack and exp)Applies for now to 211 (horizontal 2 to 1) Chrom. sampling only.  */
		/* For exp, must also set PF2_SmoothUpsample to have any effect. */
#define PF2_SmoothUpsample          0x00000002L
		/* (jpeg exp)Applies for now to 211 (horizontal 2 to 1) Chrom. subsampling only.  */
#define PF2_565Output               0x00000004L
		/* (Seq Expand). Set for 565 rather than 555 rgb output if 16-bit output is chosen for color images. */
/* The following was defined above after DIB_INPUT, used by both jpeg pack and expand */
/* #define PF2_CompressedYUV           (0x00000010L)  */
		/* (jpeg pack and exp) yuv is compressed to 16-235 lum and 16-240 chrom.  */
/* following used by lossy JPEG for 12-bit images, */
/*#define PF2_P2AndP3OptDisable   (0x20L)     \\ disable P2 optimizations */
/*#define PF2_P3OptDisable        (0x40L)     \\ disable P3 optimizations */


/******************  For Wizard ***********************************/

#define MAX_REGIONS 16
/*This structure is used for regions in wizard (see REORIENT structure). */
typedef struct {
                WORD         LumFactor; /* 0-255 gives quality as in compress, can go beyond 255 here. */
                WORD         ChromFactor;
                LONG         NumTbls; /* Number of quant. tables to be supplied, or a code.
                                         255-> do not change quantization from existing,
                                         0->use luminance and chrominance values instead of supplying q-tables,
                                         1->grayscale,2 or 3 -> color. */
                WORD         QTbls[1]; /* If NumTbls != 0 or 255, there should actually be
                                         64*NumTbls words here. The 1 is a dummy. */
        } REGION_INFO;


/***************************************************************************\
*   Variables for re-orienting JPEG files (Operation ROR)                   *
\***************************************************************************/
typedef struct {
        DWORD         Reserved0;
		BYTE PICHUGE *RegionInfo; /* Null unless doing regions.
									 Put in data according to REGION_INFO structure above,
									 for each of NumRegions regions (concatenated). After that, put in
									 RegionMap, at offset RegionMapOffset from beginning of RegionInfo.
									 RegionMap is an array of bytes, corresponding to the 8x8 blocks in
									 the image.  The value in each byte,in row scan order, tells
									 which region number the corresponding 8x8 block is in.  The
									 region numbers are from 0 through NumRegions - 1. RegionMap
									 has width and height given below in RegionMapWidth and
									 RegionMapHeight. */
		BYTE PICHUGE *QTableReq;    /* (optional) Ignored unless Requantize
									 below is set to 1, or JoinFlags & JF_DoJoin != 0 and
									 JoinFlags & JF_UseRequestedQuantization != 0. Allocated
									 and set by user.	Points to Q-table values, if used.
									 Not in zig_zag order! There only need be	64 bytes for
									 grayscale input.  For color,	first 64 bytes are for
									 luminance(Y) and the	next 64 for Cb. If this is followed
									 by a	zero byte, the same 64 are used for Cr as	were for Cb.
									 If not, it is assumed that the 64 bytes following the 64
									 Cr quant. bytes are used for Cb.  Thus in the color case,
									 this table must be at least 129 bytes, and 192 bytes if
									 there are 3 different quant. tables. */
		BYTE PICHUGE *ExifThumbnail; /* pointer to Jfif-format thumbnail image for Exif output if
                                     desired.  Error unless unless JpegType == JT_EXIF &&
                                     ExifThumbnailToMake == THUMB_NONE */
        BYTE PICHUGE *MapY; /* If not NULL, Remaps Y component(used for gamma correction,takes precedence over Yscale and Yshift). */
        BYTE PICHUGE *MapCb; /* If not NULL, Remaps Cb component(used for gamma correction,takes precedence over Yscale and Yshift). */
        BYTE PICHUGE *MapCr; /* If not NULL, Remaps Cr component(used for gamma correction,takes precedence over Yscale and Yshift). */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
		PICFLAGS     PicFlags;
		PICFLAGS     PicFlags2;
        LONG         Pad;           /* 1-> pad to fill mcu if needed, 0->trim ,
                                     2-> pad width only, 3-> pad height only(in case of rotation,
                                     width and height here refer to original image). */
		LONG         KeepColors;    /* 1->put colors from pic file in APP1,0->discard */
		LONG         LumFactorReq;  /* (optional) Ignored unless QTableReq is
									 NULL and: Requantize	below is set to 1, or	 JoinFlags &
									 JF_DoJoin != 0 and	JoinFlags & JF_UseRequestedQuantization
									 != 0. Luminance compression factor 0..255 */
		LONG         ChromFactorReq; /* Same comment as above. Chrominance compression factor 0..255 */
		LONG         Requantize;    /* Set by User. 1-> change the quantization interval size
									for each DCT coefficient to the odd multiple
									of the existing value which is nearest to
									the value in QTableReq if this is present,
									or to the one implied by LumFactorReq and
									ChromFactorReq otherwise. This produces the
									same quantized DCT coefficients that would
									have been obtained starting from the raw
									image using this quantization.
									2-> for each DCT coefficient, use requested
									quantization value when larger than one in
									input file, otherwise use one in input file.
									NOTE: this does not produce same jpeg file
									as if we had started with the raw image
									using this quantiztion! Use at your own risk.
									3-> for each DCT coefficient, use requested
									quantization value, period.
									0-> do not requantize  */
		LONG         RequantizationDone;  /* set by Pegasus if Requantize is
									1. 1->some change was made to quantization,
									0-> no change was made. */
		LONG         AppsToKeep;    /* The default is to remove all App data segments
									except JFIF and PIC. The bit positions on in
									AppsToKeep, numbered from least significant to
									most, tell which App data segments to keep,
									other than JFIF and PIC, for positions 0,1,...15.
									For example, if AppsToKeep = 0xC (1100 binary),
									this means keep App2 and App3 data segments.
									JFIF is removed only if bit position 16 is ON,
									and PIC is removed only if bit position 17 is ON. */
		LONG         RemoveComments; /* if the 0th bit is on, remove the existing
									comments (if not, keep all comments).
									If the 1st bit is on, do NOT add the comment
									Pegasus Imaging Corp (if not, add this
									comment, even if bit 0 is on, unless a comment
									whose first 15 characters are Pegasus Imaging
									already remains in the file). */
		JPEG_TYPE    JpegType;      /* Output type(set by application). Only JT_RAW and JT_PIC2 are supported */
		BYTE         OutputKeyField[8];
				/* ^ Output encode key -- currently only for JpegType == JT_PIC2 &&
								if OutputKeyField[0] != 0.  Eventually also for JpegType ==
								JT_RAW && OutputKeyField[N] != 0 for any N = 0, .., 7 */
		LONG    JoinOffset; /* offset from Get.Start to the start of
					a second image in Get buffer to be joined to first (see JoinFlags
					below). If this is 0,	Pegasus will assume offset is unknown and
					will scan to attempt to find it. The Get buffer must be large
					enough to hold ALL of the	first image and up through the first scan
					header of the second image. */
		DWORD   JoinFlags;  /* These flags are set to cause two images to be joined
					 in manner desired. See the defined values for JoinFlags below. */
		DWORD   InsertTransparencyLum; /* A value between 0 and 256, inclusive, which
					determines how much of the luminance comes from the pixels of the original
					image when inserting an image.  A value of 0 means the INSERTED
					image determines the luminance in its	region.  A value of 256 means the
					the original image completely determines the luminance.
					This has meaning only when JF_Insert is set (see below). */
		DWORD   InsertTransparencyChrom; /* A value between 0 and 256, inclusive, which
					determines how much of the chrominance comes from the pixels of the original
					image when inserting an image. See above.
					This has meaning only when JF_Insert is set (see below). */
		LONG	NumRegions; /* 1 or greater to have effect; <= MAX_REGIONS */
		DWORD	RegionMapOffset; /* offset from RegionInfo to RegionMap. */
		WORD	RegionMapWidth; /* Must be >= (image_width + 7)/8.  Any extra columns skipped over */
		WORD	RegionMapHeight; /* Must be >= (image_height + 7)/8. Any extra rows ignored */
		/* The following color space transforms are not allowed if doing requantize,join, or regions. */
		SHORT   YShift; /* 0 if not transforming luminance (Y); -256 ... 255 otherwise. */
		/* YShift represents increase(decrease if < 0) in brightness. */
		SHORT	YScale; /* 0 if not transforming luminance (Y); -128 ... 127 otherwise. */
		/* YScale represents increase(decrease if <0) in contrast.  Ususally the
		contrast needs to be increased if the brightness is increased, so these two
		are often used together. */
		SHORT   CbShift; /* 0 if not transforming chrominance; -256 ... 255 otherwise. */
		SHORT	CbScale; /* 0 if not transforming chrominance; -128 ... 127 otherwise. */
		SHORT   CrShift; /* 0 if not transforming chrominance; -256 ... 255 otherwise. */
		SHORT	CrScale; /* 0 if not transforming chrominance; -128 ... 127 otherwise. */
        DWORD     ExifThumbnailLen;
            /* length of jiff thumbnail provided at ExifThumbail.  Error unless
                JpegType == JT_EXIF && ExifThumbnailToMake == THUMB_NONE */
        THUMBNAIL ExifThumbnailToMake;
            /* != THUMB_NONE to create a thumbnail in Exif output.
                (requires and uses OP_S2D or OP_SE2D and OP_D2S or OP_D2SE)
                error unless JpegType == JT_EXIF && ExifThumbnailTomake == THUMB_NONE */
        LONG    MaxRqtblSize; /* only applies to Flash */
		} REORIENT;

		/* settings for JoinFlags for join operation above( these are "or"ed with JoinFlags). */
#define JF_DoJoin    0x00000001  /* Set if two images are to be joined. All
		other join flags are ignored if this is not set.  This is NOT allowed
		unless: Requantize is zero, F_Crop is NOT set in PicParm Flags, and
		VisualOrient is zero in PicParm. */
#define JF_LeftRight 0x00000002  /* Set if images are to be joined left-right
		rather than top-bottom(the default is top-bottom) */
#define JF_SecondOnTopLeftInsert 0x00000004  /* Set if second image in Get buffer
	 goes on top or left or inserted; default is first is top or left or inserted. */
#define JF_UseSecondSubsampling 0x00000008  /* Set if subsampling is to be
		taken from second image in Get buffer; default is from first image.
		WARNING: If the two images do not	have the same subsampling, some inverse
		and forward DCT operations are required	on the chrominance components. */
#define JF_UseRequestedQuantization 0x00000010  /* Set if quantization of
		joined image is to be input by app, either by supplying a non-null
		pointer for	QTableReq above; if that is null, Pegasus will use
		LumFactorReq and ChromFactorReq above. */
#define JF_UseSecondQuantization 0x00000020  /* This is ignored if
		JoinFlags & JF_UseRequestedQuantization != 0.  Set this if quantization
		is to be taken from second image in get buffer; the default is to take
		quantization from the first. */

		/* WARNING: if the joined image quantization is different from the input
		quantization for either image, some new error may be introduced, larger
		than would have been expected from the indicated quantization table.
		Even when an output quantization interval is larger than the input one,
		the error in a coefficient is no longer guaranteed to be no more than half
		of the output quantization interval size.	This has to do with rounding. */

#define JF_Insert    0x00000040  /* Set if one image is to be inserted into
		the other rather than placed side-by-side. NOTE: This is considered a
		version of joining two images, so JF_DoJoin must also be set in order for
		this to have meaning.  The first image in the Get	buffer will be
		inserted in the second image unless the flag JF_SecondOnTopLeftInsert
		is set.  If JF_Insert is set, then CropXoff and CropYoff in the PIC_PARM
		stucture must be set;  this gives the location of where to put the upper
		left corner of the inserted image.  This will be rounded down to the nearest
		multiple of MCU width and height of the surrounding image.  The width and
		height of the inserted image will be rounded down to a multiple of MCU width
		and height, and made to fit in the surrounding image if too large. Also
		if JF_Insert is set, InsertTransparencyLum and InsertTransparencyChrom
		must be given a value between 0 and 256 inclusive (see above). */

		/* Settings for PicFlags2 above */
#define PF2_ClipDc	0x00000001L  /* not operational at this time */
#define PF2_ClipAc	0x00000002L  /* not operational at this time */
#define PF2_UseISOStandardHuffmanTbls    0x00000004L  /* (ROR)The default is to compute optimal huffman codes */
#define PF2_OmitISOStandardHuffmanTbls 0x00000008L
    /* (ROR) If PF2_USEISOStandardHuffmanTbls is also set, then do not write
    the Huffman code markers (FFC4) to the output jpeg stream; this is primarily
    for MJPEG streams because the result doesn't conform to the still-image jpeg
    standard */

/***************************************************************************\
*   Variables for converting Seq to Progressive JPEG (Operations S2P)       *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Buffer information */
        SCAN_PARM PICHUGE *ScanParms; /* For progressive JPEG */
        BYTE PICHUGE *Reserved2;    /* Must be NULL */
        BYTE PICHUGE *Reserved3;    /* Must be NULL */
        BYTE PICHUGE *Reserved4;    /* Must be NULL */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         NumProgScans;  /* for progressive JPEG */
        } TRANS2P;

/***************************************************************************\
*   Variables for converting Progressive to Seq JPEG (Operation P2S)        *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        /*  Buffer information */
        BYTE PICHUGE *Reserved1;    /* Must be NULL! */
        BYTE PICHUGE *Reserved2;    /* Must be NULL */
        BYTE PICHUGE *Reserved3;    /* Must be NULL */
        BYTE PICHUGE *Reserved4;    /* Must be NULL */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         NumScansReq;   /* num progressive JPEG scans to decode
                                        (0 means do all) */
        LONG         NumScansDone;  /* PJPEG Scans actually done upon return */
        LONG         NumBytesDone;  /* PJPEG Bytes comprising the scans returned */
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling
                                        (returned by Pegasus) */
    } TRANP2S;

/*.P*/
/***************************************************************************\
*   Info is already in the PIC_PARM structure                               *
\***************************************************************************/
/* INTERNAL must be the largest union variant */
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *ReservedPtrs[8]; /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;
        DWORD        Reserveds[64];
    } INTERNAL;

/*.P*/
/***************************************************************************\
*   Converting supported file formats to DIBs (F2D) (opcode 81)             *
\***************************************************************************/

#define OP8X_NPOINTERS  20          /*way more than enough */
#define OP8X_NVARS      20          /*way more than enough */
typedef struct {
        BYTE PICFAR *ptr8X[OP8X_NPOINTERS];
        DWORD vars[OP8X_NVARS];
        } OP8X_WORK_AREA;
/* This is the work area (in p->Reserved) for opcodes 80,81,82.  It basically */
/* consists of a number of malloc'ed pointers that all get released */
/* at PegasusTerm time.  Currently, vars[0] is a flag which, if set, */
/* tells REQ_EXEC to do a REQ_CONT (but only for opcode 81) -- this is so */
/* the app that calls Pegasus can call REQ_EXEC for multiple dibs, but */
/* REQ_CONT will actually be executed.  The area must obviously initially */
/* be nulled out. */

typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *AuxSpace;
                /* If auxillary data (e.g., Author, etc) is to be retained, this is a */
                /* pointer to the area to contain these chunks.  If the application */
                /* is not interested in these chunks, this pointer must be NULL.  The */
                /* application is responsible for allocating this space.  If there is */
                /* not enough room to hold the data, the response RES_SPACE_NEEDED */
                /* will result, which means that AuxNeeded (below) additional bytes */
                /* need to be allocated.  The current allocated size of this buffer */
                /* must be set in AuxSize below, and maintained each time it is */
                /* increased by a response to RES_SPACE_NEEDED. */
        BYTE PICHUGE *Ptr2;    /* Must be NULL!, except for PhotoCD, in which */
                /* case it must point to the filename for the PhotoCD file */
        BYTE PICHUGE *GIFHead;      /* -> 781 byte buffer for GIF header if you
                                        want to process animated GIF's */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
                /*NOTE: if any of the following fields are not applicable, */
                /* they should be set to 0 */
        PICFLAGS     PicFlags;      /*PF_EOIFound, PF_IsGray, PF_IncludeBMPHead */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        DWORD        AllocType;
                /* 0 if input is to be processed in chunks, 1 if input will be */
                /* available in its entirety (avoids possible need for seeking) */
        DWORD        AuxSize;
                /* current allocated size of AuxSpace; set by the application */
        DWORD        AuxUsed;
                /* how many bytes of AuxSpace are used up.  Should be set to 0 by */
                /* the application before the opcode is "called". */
        DWORD        AuxNeeded;
                /* needed additional space; set by opcode before returning with */
                /* RES_AUX_NEEDED.  If the application continues (REQ_CONT) and */
                /* AuxSize is unchanged, it is assumed that the application requests */
                /* that any other auxillary chunks be tossed. */
        DWORD        ApplyResponse;
                /* ignored unless PNG or TIFF.  0=don't apply gamma correction or */
                /* gray response to resulting DIB; 1=do */
        DWORD        ProgressiveMode;
                /* only applies if input file is progressive: */
                /* 0=create increasingly larger DIBs */
                /* 1=create same size DIBS, increasing clarity (just pixel */
                /*   replication) -- with only data from current pass */
                /* 2=return only final DIB */
                /* 3=create same size DIBS, increasing clarity (just pixel replica- */
                /*   tion) -- merging data from each pass.  Seeking of the output */
                /*   queue may be required if the output queue is not large enough */
                /*   to hold the entire image. */
                /* 4=create same size DIBS, smoothing, with only data from current pass */
                /* 5 = create same size DIBS, smoothing, merging data from each pass. */
                /*   Seeking of the output queue may be required if the output queue */
                /*   is not large enough to hold the entire image. */
        BITMAPINFOHEADER BiOut;     /* this is the bi header of the output dib */
        DWORD        YieldEvery;    /* If PF_Yield is set, yield after putting this many scanlines in the
                                        output queue */
        DWORD        PhotoCDResolution; /* unused and should be 0, except for PhotoCD,
                                        in which case it is: 0: 64x96; 1: 128x192; 2: 256x384; 3: 512x768;
                                        4: 1024x1536; 5: 2048x3072; 6: 4096x6144 */
        LONG         Compression;   /* TIFF (0, 1-none, 2-Modified G3, 3-G31D, -3-G32D, 4-G4, 5-LZW, 32773-PackBits) */
        BYTE         TransparentColorIndex;/* if GIF && PF_IsTransparency */
        BYTE         RawG3FillOrder; /* != 1 for raw fax TIFF byte FillOrder == 2, otherwise FillOrder == 1 */
        BYTE         RawG3PhotometricInterpretation; /* != 0 for raw fax BlackIsZero, otherwise WhiteIsZero */
        BYTE         DisposalMethod;/* GIF graphics extension field */
        WORD         DelayTime;     /* GIF graphics extension field */
        BYTE         BackgroundColor;/* GIF header background color index */
        BYTE         AspectRatio;   /* GIF header aspect ratio = floor(((pixelwidth/pixelheight)+15)/64) */
        DWORD        WidthPad;      /* currently only used for GIF -- set PF_WidthPadKnown to override DIB padding */
        WORD         LogicalScreenWidth;    /* from GIF header */
        WORD         LogicalScreenHeight;   /* from GIF header */
        WORD         ImageLeftPosition;     /* from GIF image header */
        WORD         ImageTopPosition;      /* from GIF image header */
        DWORD        Expansion8;    /* unused and should be 0 */
        DWORD        Expansion9;    /* unused and should be 0 */
        DWORD        Expansion10;   /* unused and should be 0 */
    } F2D_STRUC;

#define PF2_IsInterlacedGIF ( 0x00000001L )
#define PF2_GIFColorsSorted ( 0x00000002L )
    /* set if input GIF is interlaced */


/*.P*/
/***************************************************************************\
*   Converting DIBs to supported file formats (D2F) (opcode 80)             *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *AuxSpace;
                /* If auxillary data (e.g., Author, etc) has been retained from an */
                /* opcode 81 call, this is a pointer to the area that contains these */
                /* chunks.  The size of this buffer must be set in AuxSize below. */
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
                /* NOTE: if any of the following fields are not applicable, they */
                /* should be set to 0 */
        PICFLAGS     PicFlags;      /*only PF_IncludeBMPHead */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        DWORD        AllocType;
                /* 0 if input is to be processed in chunks, 1 if input will be */
                /* available in its entirety (avoids possible need for seeking) */
        DWORD        AuxSize;
                /* allocated size of AuxSpace; set by the application */
        DWORD        ImageType;
                /* output image type requested: must be in one of these families: */
                /* BI_BMPx, BI_TGAx, BI_PCXx, BI_TIFx, BI_PNGx */
        DWORD        Compression;
                /* type of compression:
                    0=none (BMP, TGA, TIFF)
                    1=rle (BMP, PCX) none TIFF)
                    2=lzw (PNG), Modified G3 (TIFF)
                    3=G3 (TIFF) 1-D & 2-D
                    4= G4 (TIFF),
                    6= jpeg */
        DWORD        PNG_Progressive;
                /* PNG only: 0=sequential, 1=adam7 progressive */
        DWORD        PNG_Filter;
                /* PNG filter option: 0=none, 1=sub, 2=up, 3=average, 4=paeth, */
                /* 1000="optimal" (computed optimally for each scanline) */
        DWORD        ApplyAux;
                /* 0=discard AuxSpace data; 1=map chunks only if they map exactly to */
                /* the output file; 2=map chunks to most reasonable output chunks */
        DWORD        OutBpp;
                /* only used when creating Targa files and the input dib size is 24; */
                /* in that case, OutBpp may be either 16 or 24 */
        DWORD        Expansion1;    /* unused and should be 0, except for TIF
                                        ( Expansion1 & 0xff000000 ) == 0 for intel byte order output
                                        ( Expansion1 & 0xff000000 ) == 1 for motoroloa byte order output
                                        if the input is 32-bpp CMYK:
                                            ( Expansion1 & 0xff ) == 0 for CMYK output
                                            ( Expansion1 & 0xff ) == 1 for K output only from input CMYK
                                        if the input is 32-bpp CMYK and ( Expansion1 & 0xff ) == 0
                                            ( Expansion1 & 0x0000ff00 ) is output as DotRange[0]
                                            ( Expansion1 & 0x00ff0000 ) is output as DotRange[1] */
        DWORD        Expansion2;     /* unused and should be 0, except for TIF
                                        ( Expansion & 0xff ) == 0 don't output X/Y resolution tags
                                                             == 1 output in inches
                                                             == 2 output in centimeters */
        DWORD        MultiImageSize; /* for TIF, optional input image size for appending to an existing image */
        BYTE         TransparentColorIndex; /* if GIF && PF_ApplyTransparency */
        BYTE         NegateImage;   /* for TIF, G3 or G4, output complement of image if != 0 */
        BYTE         Dimension;     /* for TIF with Compression=3, 0=1-D, 2=1-D */
        BYTE         DisposalMethod;/* GIF graphics extension block field */
        DWORD        RowsPerStrip;  /* # rows in each strip, 0 or >= abs(biHeight) for 1 strip with all rows */
        LONG         LumFactor;     /* Luminance compression factor 0..255 if TIFF Jpeg */
        LONG         ChromFactor;   /* Chrominance compression factor 0..255 if TIFF Jpeg*/
        SUBSAMPLING  SubSampling;   /* SS_field, 111/211/211v/411 subsampling if TIFF Jpeg*/
        LONG         Predictor;     /* Predictor to be used for TIFF/LZW 0,1=no prediction, 2=TIFF Horizontal differencing */
        WORD         DelayTime;     /* GIF graphics extension block field */
        BYTE         BackgroundColor;/* GIF header background color index */
        BYTE         AspectRatio;   /* GIF header aspect ratio = floor(((pixelwidth/pixelheight)+15)/64) */
        DWORD        WidthPad;      /* currently only used for GIF -- set PF_WidthPadKnown to override DIB padding */
        WORD         LogicalScreenWidth;    /* if != 0, GIF logical screen width for GIF header */
        WORD         LogicalScreenHeight;   /* if != 0, GIF logical screen height for GIF header */
        WORD         ImageLeftPosition;     /* from GIF image header */
        WORD         ImageTopPosition;      /* from GIF image header */
    } D2F_STRUC;

#define PF2_MakeCorrectTiffJPEG ( 0x00000001L )
    /* else it makes a Tiff JPEG which is incorrect according to the TIFF
        spec but which Wang Imaging can read */
#define PF2_OmitGIFTrailer      ( 0x00000002L )
    /* set for all frames except the last frame of a multi-image GIF */
#define PF2_LocalGIFColors      ( 0x00000004L )
    /* if PF_MultiImage is set, then input colors are written to output as a local color table */

/*.P*/
/***************************************************************************\
*   Utiltiy functions (DIBs, colormaps, histograms) (opcode 82)             *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *ptr1;
                /* subcode 0,3,4,5,6 -- unreferenced */
                /* subcode 1,2 -- pointer to 32k/64K WORD histogram buffer */
        BYTE PICHUGE *ptr2;
                /* subcode 3,4 -- pointer to 32K BYTE colormap buffer */
                /* subcode 0,1,2,5,6 -- unreferenced */
        BYTE PICHUGE *ptr3;         /* subcode 8 -- external LOGPALETTE */
        BYTE PICHUGE *Reserved4;    /* must be null */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
                /*NOTE: if a subcode does not use one of the following members, */
                /* it should be set to 0. */
        PICFLAGS     PicFlags;      /* PF_IncludeBMPHead, PF_Dither */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        DWORD        AllocType;
                /* 0 if input is to be processed in chunks, 1 if input will be */
                /* available in its entirety (avoids possible need for seeking) */
        DWORD        Subcode;       /* 0,1,2,3,4,5,6 */
        DWORD        HistogramSize;
                /* subcode 0,4,5,6 -- unused */
                /* subcode 1,2 -- size of histogram buffer (Histogram) in WORDS -- */
                /* must be either 32768 or 65536 */
        BITMAPINFOHEADER BiOut;     /* bi header for output dibs */
        DWORD        RetainPalette;
                /* subcode 0 -- 1 to retain palette info, 0 to discard */
                /* subcodes 1,2,3,4,5,6 -- unused */
        DWORD        Orient;
                /* subcode 0,1,2,3,4,6 -- unused */
                /* subcode 5 -- 3 lsbs indicate: */
                /*   bit 0: reflect horizontally */
                /*   bit 1: rotate right 90 */
                /*   bit 2: rotate 180 */
                /* all other bits should be 0 rotations are performed first */
        DWORD        OutBpp;
                /* subcode 0,4: output bits per pixel */
                /* subcode 1,2,3,5,6: unused */
        DWORD        NumColors;
                /* subcode 2,3: number of colors */
                /* subcode 0,1,4,5,6: unused */
        DWORD        OutWidthPad;   /* used to pass in output width on Mac */
        DWORD        TransparentColorIndex; /* if PF_ApplyTransparency is set and <= 8 bit:
                                                subcode 1: this index is ignored when constructing the color histogram
                                                subcode 2: optimum palette of NumColors-1 created for opaque input
                                                            colors and the last output palette entry is reserved as
                                                            the new transparent color
                                                subcode 3: color map won't map to last palette entry
                                                subcode 4: input transparent pixels are mapped to last palette entry and
                                                            opaque pixels aren't mapped to last palette entry
                                            */
        DWORD        ReduceError;   /* unused and should be 0 */
        BYTE         BackgroundColorIndex; /* subcode 4: input as input palette index, output as re-mapped index
                                                in output palette */
        BYTE         DitherType;    /* 0 for default dither, 1 for faster, less accurate dithering */
        WORD         Expansion4c;   /* unused and should be 0 */
        DWORD        Expansion5;    /* unused and should be 0 */
        DWORD        Expansion6;    /* unused and should be 0 */
        DWORD        Expansion7;    /* unused and should be 0 */
        DWORD        Expansion8;    /* unused and should be 0 */
        DWORD        Expansion9;    /* unused and should be 0 */
        DWORD        Expansion10;   /* unused and should be 0 */
    } UTL_STRUC;

/* set PF2_ShowReduceError to get color error returned in ReduceError
    and set ptr1 to point to a 32K histogram for input rgb555 colors
    (color-mapped input only) */
#define PF2_ShowReduceError             (0x00000001L)
/* set PF2_ApplyReduceErrorThreshold to use the ReduceError value as
    an error threshold and abort subcode 4 with ERR_THRESHOLD_EXCEEDED
    if applicable */
#define PF2_ApplyReduceErrorThreshold   (0x00000002L)
/* set PF2_ColorMap444 to create/use a 4K RGB444 ColorMap instead of
    a 32K RGB555 ColorMap trading speed for accuracy */
#define PF2_ColorMap444                 (0x00000004L)

/*.P*/
/***************************************************************************\
*   Variables for Lossless DIBS (Operation LIMP, LIME)                      *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        LAYER PICHUGE *Layer;       /* points to an 8-element array of layers */
        BYTE PICHUGE *AppField;     /* points to user info of AppFieldLen bytes */
        BYTE PICHUGE *OverText;     /* NULL or points to overlaying text */
        RGBQUAD PICHUGE *UniversalCT;  /* Universal Color Table, replaces CT in image */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         AppFieldSize;  /* Allocated size of AppField in bytes. */
        LONG         AppFieldLen;   /* 0 or length of data within AppField */
        LONG         OverTextSize;  /* Allocated size of OverText in bytes. */
        LONG         OverTextLen;   /* 0 or length of data within OverText */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip - after */
                                    /* INIT it is the size of the largest strip, upon */
                                    /* RES_PUT_NEED_SPACE it is the size for the */
                                    /* current layer */
        SHORT        NumUC;         /* Number of Universal Colors in UniversalCT */
        SBYTE        MinLayerWanted;/* When expanding, smallest layer needed */
        SBYTE        CurrentLayer;  /* Current layer being packed/expanded */
        BYTE         RestartLo;     /* Low range of restart intervals to be expanded */
        BYTE         RestartHi;     /* High range of restart intervals to be expanded */
        /* the next 15 items (ProgHeight..SymbolSize) should remain in the same order */
        BYTE         ProgHeight;    /* Progression height */
        BYTE         NumRestarts;   /* Number of restart intervals (for level 0) */
        DWORD        RestartOff[4]; /* Offset of restart interval from layer 0 */
        SHORT        NumOfPages;    /* Number of pages/images in document/file */
        SHORT        PageNum;       /* Page/image number of current page/image */
        SHORT        Transparent;   /* Transparent color index, -1 means none */
        SHORT        UserDelay;     /* User delay (seconds/100) before image replacement */
        SHORT        XOff;          /* Image X-offset relative to logical screen */
        SHORT        YOff;          /* Image Y-offset relative to logical screen */
        BYTE         DispMethod;    /* Disposition method */
        BYTE         ErrLimit;      /* Last ErrLimit bits can be in error */
        BYTE         CompMethod;    /* Lossless compression 'L', or 'P', 0 => automatic */
        BYTE         CompOrder;     /* Order of method if method if 'P' (1..4), 0=>3 */
        /* The remaining should be invisible to the app. */
        LONG         TableSize;     /* Hash table size */
        LONG         SymbolSize;    /* Size in bits of Symbol */
        DWORD        Symbol;        /* Symbol to be encoded/decoded */
        DWORD        HashedContext; /* Context for Symbol to be encoded/decoded */
        DWORD        HashStep;      /* Aid to help Els-coder traverse hash table */
                                    /* must be < TableSize */
        DWORD        TotalWritten;  /* Accumulative count of bytes written so far */
        WORD         internal_buffer;   /* for internal use by Els coder only */
        SHORT        jot_count;                     /* " */
        BYTE PICHUGE *context_table;                /* " */
        BYTE PICHUGE *backup_table;                 /* " */
        BYTE PICHUGE *table_end;                    /* " */
        DWORD        minimum;                       /* " */
        LONG         backlog;                       /* " */
    } LOSSLESS;


/*.P*/
/***************************************************************************\
*   Variables for Lossless DIBS (Operation LIP3, LIE3), have ParmVerMin>=2  *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        LAYER PICHUGE *Layer;       /* points to an 8-element array of layers */
        BYTE PICHUGE *AHT;          /* points to optional Abbreviated Huffman Table */
        BYTE PICHUGE *Reserved3;    /* must be null */
        RGBQUAD PICHUGE *UniversalCT;  /* Universal Color Table, replaces CT in image */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* See the PF2_ flags above */
        REGION       Region;        /* Source or Destination region (replaces Head) */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip - after */
                                    /* INIT it is the size of the largest strip, upon */
                                    /* RES_PUT_NEED_SPACE it is the size for the */
                                    /* current layer */
        SHORT        NumUC;         /* superseded by NumUC2 below (must be 0) */
        SBYTE        MinLayerWanted;/* When expanding, smallest layer needed */
        SBYTE        CurrentLayer;  /* Current layer being packed/expanded */
        BYTE         RestartLo;     /* Low range of restart intervals to be expanded */
        BYTE         RestartHi;     /* High range of restart intervals to be expanded */
        /* the next 13 items (ProgHeight..SymbolSize) should remain in the same order */
        BYTE         ProgHeight;    /* Progression height */
        BYTE         NumRestarts;   /* Number of restart intervals (for level 0) */
        DWORD        RestartOff[4]; /* Offset of restart interval from layer 0 */
        SHORT        NumOfPages;    /* Number of pages/images in document/file */
        SHORT        PageNum;       /* Page/image number of current page/image */
        SHORT        Transparent;   /* Transparent color index, -1 means none */
        SHORT        UserDelay;     /* User delay (seconds/100) before image replacement */
        SHORT        XOff;          /* Image X-offset relative to logical screen */
        SHORT        YOff;          /* Image Y-offset relative to logical screen */
        BYTE         DispMethod;    /* Disposition method */
        BYTE         AllowedBitErr; /* Last AllowedBitErr bits can be in error */
        BYTE         CompMethod;    /* Lossless compression (see #define METHOD_xxx below) */
        /* Configuration parameters for PPMD or JPEG method: */
        BYTE         CompOrder;     /* Order of method PPMD=>0..4, JPEG=>1..7 */
        BYTE         PTuning;       /* Index to set of tuned parameters for PPMD method */
        SBYTE        Channel;       /* Color channel to be treated as gray (-1 if composit) */
        PIXEL_TYPE   IOPixType;     /* Convert from/to this pixel type , 0 = default */
        BYTE         NativeBpp;     /* Bpp prior to any conversion from/to IOPixType */
        DWORD        RestartCount;  /* Number of pixels between restart markers */
        DWORD        SetBits;       /* (internal) After UnpkLine this has pixel's used bits set */
        DWORD PICHUGE *PixelMap;     /* (internal) Maps 2^EffectiveBpp values to 2^RegionBpp values, */
                                    /* includes UniversalCT corrections and any byte swap */
        BYTE         EffectiveBpp;  /* (internal) NativeBpp - AllowedBitErr after INIT */
        BYTE         PrecisionUCT;  /* (internal) 0 <= Each element in UniversalCT < 2^PrecisionUCT */
        BYTE         Reserved9[2];
        DWORD        AHTLen;        /* Byte length of AHT buffer (if any) */
        DWORD        AppsToOmit;
        DWORD        NumUC2;        /* Number of Universal Colors in UniversalCT (if NumUC above is 0,
                                        replaces NumUC so up to a 65536-element UniversalCT can be specified)
                                        UniversalCT is ignored if NumUC2 and NumUC are less than 2 */
    } LOSSLESS3;

#define PF2_FoundDicom              0x00000001L
        /* (LL JPEG) file was found to be Dicom, may have multiple images */
#define PF2_NonInterleaved          0x00000002L
        /* (LL JPEG) compressed code is non-interleaved */
#define PF2_FastDecompress          0x00000004L
        /* (LL JPEG) high-speed AVI decompression used */
#define PF2_ReachedEOI              0x00000008L
        /* (LL JPEG) 0xff/0xd9 encountered in image */
#define PF2_AHTCodeChanged          0x00000010L
        /* (LL JPEG pack) AHT specified a single 15-bit huffman code and
            zero, one or two 16-bit huffman codes.  The code(s) were changed
            to avoid having to generate a 0xffff code to code a prefix
            not coded by the AHT. */

#define METHOD_AUTO 0
#define METHOD_LOCO 'J'
#define METHOD_PPMD 'P'
#define METHOD_JPEG 'S'


/***************************************************************************\
*   Variables for Portable Network Graphics, have ParmVerMin>=2             *
\***************************************************************************/

/* Flags for ther PNG filter to say which filters to use.  The flags
 * are chosen so that they don't conflict with real filter types below.
 * These values should NOT be changed.
 */
#define F_PNG_NO_FILTERS     0x00
#define F_PNG_FILTER_NONE    0x08
#define F_PNG_FILTER_SUB     0x10
#define F_PNG_FILTER_UP      0x20
#define F_PNG_FILTER_AVG     0x40
#define F_PNG_FILTER_PAETH   0x80
#define F_PNG_ALL_FILTERS (PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP | \
                         PNG_FILTER_AVG | PNG_FILTER_PAETH)

/* F_PNG_ALL_FILTERS is the typical default 
 * and will be used if PF_OptimalFilter is set.
 */

/* Filter values 
 * These defines should NOT be mixed with filter flags. Either use
 * the flags or 1 of the following filter values.
 * These values should NOT be changed.
 */
#define V_PNG_FILTER_VALUE_NONE  0
#define V_PNG_FILTER_VALUE_SUB   1
#define V_PNG_FILTER_VALUE_UP    2
#define V_PNG_FILTER_VALUE_AVG   3
#define V_PNG_FILTER_VALUE_PAETH 4
#define V_PNG_FILTER_VALUE_LAST  5


#define rgbAlpha     rgbReserved    /* replaces 4th component of RGBQUAD array */
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *Reserved1;    /* must be null */
        BYTE PICHUGE *Reserved2;    /* must be null */
        BYTE PICHUGE *Reserved3;    /* must be null */
        BYTE PICHUGE *Reserved4;    /* must be null */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        REGION       Region;        /* Source or Destination region (replaces Head) */
        LONG         StripSize;     /* Minimum size of buffer to hold a strip - after */
                                    /* INIT it is the size of the largest strip */
        DWORD        FilterSampleSize; /* Byte size for filter offset */
        DWORD        AlphaBpp;      /* Bytes per pixel including any alpha channel */
        DWORD        AlphaStride;   /* Stride including any alpha channel */       
        DWORD        BytesRem;      /* Number of image bytes to be (de)compressed */
        BYTE         Background[6]; /* index, gray value, or color value of background color */
                                    /* colors are RGB, each component is a WORD  */
                                    /* valid if PF_HaveBackground */
        BYTE         Trans[6];      /* gray value, or color RGB value of transparent color (Not BGR)!*/
									/* Supports 16 bit RGB values so each 2 byte is a word */
                                    /* For index trans. the 4th RGBQUAD value is used */
        BYTE         LastTrans;     /* ColorTable indices 0..LastTrans are transparent */
 									/* This sets how many 4th RBGQUAD elements are useful */
									/* 0 - means the first and 255 means all 256 alpha channels  */
									/* are relevant */
		BYTE         Action;        /* internal variable - action to be performed */
        BYTE         Filter;        /* valid if PF_OptimalFilter is not set. Use filter 0..4 but values */
        BYTE         BitDepth;      /* number of bits per component */
        DWORD		 MinReadData;	/* minimum bytes required to satisfy ReadCode request */
		WORD		 Palette_Enteries; /* How many Color_Table Entries to use with PT_CM flag set */
		BYTE		 Reserved9;
		BYTE		 Reserved10;
    } PNG_UNION;

#define PF2_InvertAlpha             0x00000001L
/* PNG Write - turn alpha channel from transparency to opacity (wrt PNG 0 is transparent 255 is opaque) */
#define PF2_IndexedAlpha            0x00000002L
/* PNG Write - RGBQuad Reserved is an Alpha Channel */  



/***************************************************************************\
*   Variables for Zooming (Operation ZOOM)                                  *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE *Reserved1;    /* Must be NULL! */
        BYTE PICHUGE *Reserved2;    /* Must be NULL! */
        BYTE PICHUGE *Reserved3;    /* Must be NULL! */
        BYTE PICHUGE *Reserved4;    /* Must be NULL! */
        BYTE PICHUGE *Reserved5;    /* Must be NULL! */
        BYTE PICHUGE *Reserved6;    /* Must be NULL! */
        BYTE PICHUGE *Reserved7;    /* Must be NULL! */
        BYTE PICHUGE *Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* See the PF_ flags above */
        PICFLAGS     PicFlags2;     /* See the PF_ flags above */
        LONG         NewWidth;      /* width of new image */
        LONG         NewHeight;     /* height of new image */
        WORD         Mode;          /* if 1, then scaling factor is NewWidth/NewHeight */
        WORD         NewBitCount;   /* bitcount for new image */
        void PICHUGE *WorkArea;     /* For internal use */
        BITMAPINFOHEADER BiOut;     /* Header for output image */
        DWORD        Reserved;      /* Should be zero */
    } ZOOM_PARMS;



/***************************************************************************\
*   Variables for wavelet expand/compress (Operations W2D, D2W)             *
\***************************************************************************/
typedef struct {
        WORD          QuantizationThreshold;    /* 0 .. 32767 */
        CHAR          XTransformFilter[9];      /* ASCIIZ file name */
        CHAR          YTransformFilter[9];      /* ASCIIZ file name */
        BYTE          MinTransformLevel;        /* 0 .. 10   */
        BYTE          MaxTransformLevel;        /* 0 .. 10   */
        CHAR          Qthr[6];                  /* QLUTLIB ASCIIZ Quantization table name */
        } WAVECOMPONENT;

typedef struct {
        DWORD        Reserved0;
        /* Information prior to REQ_INIT */
        BYTE PICHUGE* QlutlibPath;
        BYTE PICHUGE* SliceFactors; /* reserved */
        BYTE PICHUGE* UpdatedTiles; /* if <> 0, points to a bitmap of updated tiles in order
                                        ordinarily this would be set by OP_RIDP */
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         Status;
        /* file names below specify files with extension .ASC in the same
            directory (QlutlibPath) as QLUTLIB.ASC */
        WAVECOMPONENT Y;    /* also gray scale */
        WAVECOMPONENT I;
        WAVECOMPONENT Q;
        REGION       Region;
        /* ChunkSize .. ChunkHeight must appear in this order and adjacent
            to each other so op91.c/op90.c can define a pic_parm offset to them */            
        BYTE         ChunkSize;     /* if ChunkType != 3 tile size in megabytes (0 for no tiles),
                                        else ignored if ChunkType == 3 */
        BYTE         ChunkType;     /* 0 horizontal strips, 1 vertical strips, 2 rectangles,
                                        3 to use ChunkWidth/ChunkHeight and ignore
                                        ChunkSize */
        WORD         ChunkWidth;    /* tile width for ChunkType == 3, but at least 4 */
        WORD         ChunkHeight;   /* tile height for ChunkType == 3, but at least 4 */
        BYTE         TolerancePercentOutputSize; /* if PF2_AutoQuantizationThreshold is set and MaxOutputSize is not 0
                                        and not Chunks and TolerancePercentOutputSize is not 0,
                                        then a Quantization Threshold is selected such that the output 
                                        +/-TolerancePercentOutputSize percent (default 5%).  Then if the output size
                                        would be larger than MaxOutputSize, the output is limited to MaxOutputSize */
        BYTE         NumSlices;     /* reserved */
        DWORD        KeyFrameRate;  /* key frame at least once every KeyFrameRate frames */
        DWORD        MaxOutputSize; /* != 0 to limit output size if compression settings would otherwise
                                        result in a larger output */
        BYTE         ChunkProgSlices; /* if >= 2, and there are chunks, then this specifies the rearrangement
                                        to be done for progressive display -- the first 1/ChunkProgSlices'th of each
                                        chunk is output, then the next 1/ChunkProgSlices'th of each chunk is output,
                                        and so on,
                                        if >= 1, and there are chunks, and MaxOutputSize is not 0, then each chunk's
                                        size is limited according to the ratio of each chunk's unlimited size to the
                                        total unlimited size for all chunks
                                        otherwise if < 1 and MaxOutputSize is not 0, then each chunk's size is limited
                                        according to the ratio of the chunk's pixels to the total pixels for all chunks */
        BYTE         Padding1;
        WORD         ClusterHeaderVersion;
        THUMBNAIL    Thumbnail;     /* expand to thumbnail size if != THUMB_NONE */
        THUMBNAIL    Resolution;    /* level of detail to include in decompressed image */
        } WAVELET;

#define PF2_IsKeyFrame      ( 0x00000001L ) /* returned if packed/expanded frame were a key frame */
#define PF2_ForceKeyFrame   ( 0x00000002L ) /* set to force packed frame to be a key frame */
#define PF2_Hurry           ( 0x00000004L ) /* set to force wavelet delta encoding to hurry up */
#define PF2_AutoQuantizationThreshold (0x8L) /* set to automatically determine a quantization threshold to
                                                achieve MaxOutputSize to a tolerance of ToleranceOutputSize bytes */
#define PF2_FrameByFrameQthr    (0x10L)     /* set to adjust qthr frame by frame */
#define PF2_P2AndP3OptDisable   (0x20L)     /* disable P2 and P3 optimizations */
#define PF2_P3OptDisable        (0x40L)     /* disable P3 optimizations */
#define PF2_MMXDisable          (0x100L)    /* disable MMX, P2 and P3 optimizations */


/***************************************************************************\
*   Variables for Wavelet Scalar Quantization (Operations WSQP, WSQE)       *
\***************************************************************************/
typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE* Reserved1; 
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS     PicFlags;      /* PF_Yield is used */
        PICFLAGS     PicFlags2;     /* PF2_ flags above when implemented */
        LONG         Status;
        REGION       Region;
        LONG         StripSize;     /* Minimum size of buffer to hold a strip */
		LONG		 Black;			/* Scanner black calibration value. */
		LONG		 White;			/* Scanner white calibration value. */
		double		 Quant;			/* Desired total bit rate per pixel. */
    } WSQ_UNION;



/***************************************************************************\
*   Variables for AT&T DjVu(tm) (Operations D2DJVU, DJVU2D)                 *
\***************************************************************************/
#define DJVUTYPE_DOCUMENT       ( 0 )
#define DJVUTYPE_BW             ( 1 )
#define DJVUTYPE_PHOTO_COLOR    ( 2 )
#define DJVUTYPE_PHOTO_GRAY     ( 3 )

#define DJVULAYER_FULL          ( 0 )
#define DJVULAYER_MASK          ( 1 )
#define DJVULAYER_FOREGROUND    ( 2 )
#define DJVULAYER_BACKGROUND    ( 3 )

#define DJVUFILTER_COLOR        ( 1 )
#define DJVUFILTER_SIZE         ( 2 )
#define DJVUFILTER_INVERSION    ( 4 )
#define DJVUFILTER_ALL          ( DJVUFILTER_COLOR | DJVUFILTER_SIZE | DJVUFILTER_INVERSION )

#define DJVUQUALITY_LOSSY       ( 0 )
#define DJVUQUALITY_LOSSLESS    ( 1 )
#define DJVUQUALITY_AGGRESSIVE  ( 2 )

#define DJVUSUBSAMPLING_LESSDETAIL  ( 1 )
#define DJVUSUBSAMPLING_MOREDETAIL  ( 0 )
#define DJVUSUBSAMPLING_FULLDETAIL  ( 2 )

typedef struct {
        DWORD        Reserved0;
        BYTE PICHUGE* StatusText;   /* expand and pack: you set a pointer to a buffer for error text (ERR_DJVU_*) and
                                        status text (RES_PUT_DATA_YIELD) returned by DjVu */
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;     /* expand: PF_YieldGet and/or PF_YieldPut
                                       pack:   PF_YieldPut */
        PICFLAGS      PicFlags2;    /* PF2_ flags (none) */
        DWORD         StatusSize;   /* expand and pack: you set the size of the StatusText buffer */
        DWORD         WidthPad;     /* expand and pack: returned width of one line */
        DWORD         StripSize;    /* expand: returned minimum size for Put queue,
                                       pack:   returned minimum size for Get queue */
        WORD          Version;      /* expand only: returned DjVu encoder version */
        WORD          ImageDpi;     /* expand: returned image dpi
                                       pack:   you set to the overall image dpi (not needed for DJVUTYPE_PHOTO_*)
                                               (25..6000, default according to rounded Head.biXPelsPerMeter
                                                or 200 if Head.biXPelsPerMeter is 0) */
        WORD          GammaX10;     /* expand: returned image target gamma
                                       pack:   you set to target gamma (multiplied by 10) to be stored with DjVu iamge
                                               (4..49, default 22 for gammas 0.4..4.9, default 2.2) */
        BYTE          ImageType;    /* expand: returned DJVUTYPE_* above
                                       pack:   you set to DJVUTYPE_BW if input is bilevel (biBitCount == 1)
                                               otherwise if 24-bit,
                                                   DJVUTYPE_PHOTO_COLOR,
                                                   DJVUTYPE_PHOTO_GRAY converts to gray
                                                   DJVUTYPE_DOCUMENT and DJVUTYPE_BW separate the image into:
                                                        BW mask
                                                        foreground colors (colors under BW mask)
                                                        background image (remainder of image)
                                                   then DJVUTYPE_BW discards foreground colors and background image
                                               otherwise if 8-bit with uniform grayscale palette
                                                   DJVUTYPE_PHOTO_COLOR (invalid),
                                                   DJVUTYPE_PHOTO_GRAY
                                                   DJVUTYPE_DOCUMENT and DJVUTYPE_BW separate the image into:
                                                        BW mask
                                                        foreground gray levels (gray levels under BW mask)
                                                        background image (remainder of image)
                                                   then DJVUTYPE_BW discards foreground gray levels and background image
                                               otherwise don't do that (convert it first to one of the above formats) */
        BYTE          RenderLayer;  /* expand only: you set to DJVULAYER_* above
                                       DJVULAYER_FULL is ok for all ImageTypes
                                       DJVULAYER_MASK is ok for DJVUTYPE_BW or
                                           DJVUTYPE_DOCUMENT
                                       other values are ok for DJVUTYPE_DOCUMENT only
                                       ignored if not ok (same as full) */

        /* additional pack only parameters you set are: */

        /* the next two only matter for DJVUTYPE_BW and DJVU_DOCUMENT */
        BYTE          TextQuality;  /* DJVUQUALITY_LOSSLESS, DJVUQUALITY_LOSSY (default), DJVUQUALITY_AGGRESSIVE */
        BYTE          TextDontThicken; /* (default 0) if 0, the normal character thickening algorithm is used which is
                                        designed to increase the readability of text.  != 0 may improve the
                                        readability of small text (small fonts or low-resolution image) */

        /* the next two matter for DJVUTYPE_PHOTO_COLOR */
        BYTE          PhotoSubsampling; /* control the amount of CbCr detail retained
                                           DJVUSUBSAMPLING_LESSDETAIL  (less color detail)
                                           DJVUSUBSAMPLING_MOREDETAIL  (more color detail)
                                           DJVUSUBSAMPLING_FULLDETAIL  (full color detail)
                                           (default is DJVUSUBSAMPLING_MOREDETAIL) */
        /* this one also matters for DJVUTYPE_PHOTO_GRAY */
        BYTE          PhotoQuality;      /* image compression quality setting (20..100 default is 75) */

        /* the remaining fields only matter for DJVUTYPE_DOCUMENT (TextQuality and TextDontThicken above)
            If you need to be compatible with prior DjVu versions, ImageDpi must be 100, 150 or 300 */
        WORD          DocTextColorDpi; /* (25..6000, default 25) 25-50 recommended */
        WORD          DocBackgroundDpi; /* (25.6000, default 100) 100 recommended
                                        100 if you need to be compatible with prior DjVu decoders */
        BYTE          DocBackgroundQuality; /* background compression quality setting (20..100 default is 75) */
        BYTE          Padding; /* padding to dword boundary */

        /* the remaining fields only matter for DJVUTYPE_DOCUMENT
           **** these fields will ordinarily be left at their default values ****
           they are parameters to DjVu's mask creation during which text-like information is separated
           from the other image content

           If you _really_ need to change these, I suggest you look at the help file which comes with AT&T's
           DjVuShop application or look at AT&T's DjVu SDK documentation
        */
        LONG          MaskFilterLevel;  /* (-200..200, default 0)
                                            SDK says "connected components with a score less than this ... are kept" */
        BYTE          MaskMultiForeBack;/* (0..255, default 0)
                                            if != 0, used as a luminance threshold to divide into two ranges
                                            a mask is determined for each, then the two masks are "stitched"
                                            together into one mask */
        BYTE          MaskDisableFilters;/* turn off one or more of the following mask-separation filters:
                                            DJVUFILTER_COLOR
                                            DJVUFILTER_SIZE
                                            DJVUFILTER_INVERSION
                                            DJVUFILTER_ALL (invalid if MaskFilterLevel != 0) */
    } DJVU_UNION;



/***************************************************************************\
*   Variables for Tiff Edit  (Operations TIFEDIT)             *
\***************************************************************************/

typedef struct {
            DWORD        Reserved0;
            /*  Buffer information */
            BYTE PICHUGE    *Reserved1;    /* Must be NULL */
            BYTE PICHUGE    *Reserved2;    /* Must be NULL */
            BYTE PICHUGE    *Reserved3;    /* Must be NULL */
            BYTE PICHUGE    *Reserved4;    /* Must be NULL */
            BYTE PICHUGE    *Reserved5;    /* Must be NULL */
            BYTE PICHUGE    *Reserved6;    /* Must be NULL */
            BYTE PICHUGE    *Reserved7;    /* Must be NULL */
            BYTE PICHUGE    *Reserved8;    /* Must be NULL */
            PICFLAGS        PicFlags;
            PICFLAGS        PicFlags2;
            LONG            TiffReq_Op;     /* Tiff Command Operation */
            WORD            Get_Page;       /* Requested IFD page in Get Queue */
            WORD            Put_Page;       /* Requested IFD page in Put Queue */
            DWORD           ImageSize;      /* Hopefully set by user and/or OP_CODE */
} TIFF_EDIT;

#define TF_DELETE_PAGE      0x001
#define TF_INSERT_PAGE      0x002
#define TF_REPLACE_PAGE     0x004
#define TF_EXTRACT_PAGE     0x008
#define TF_LOCATE_PAGE      0x010
#define TF_COMPACT_TIFF     0x020
#define TF_EDIT_TAGS        0x100



/***************************************************************************\
*   Variables for MO:DCA/CALS Expand                                        *
\***************************************************************************/

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;     /* expand: PF_YieldPut
                                       pack:   PF_YieldPut */
        PICFLAGS      PicFlags2;    /* PF2_ flags (none) */
        DWORD         WidthPad;     /* expand and pack: returned width of one line */
        DWORD         StripSize;    /* expand: returned minimum size for Put queue,
                                       pack:   returned minimum size for Get queue */
        DWORD         ImageType;    /* pack: BI_CALS or BI_MODCA */
        DWORD         NegateImage;  /* pack: != 0 to compress negative of input image */
    } MODCA_UNION;



/***************************************************************************\
*   Variables for cleanup, such as dust removal(OP_CLEAN)
\***************************************************************************/

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1; 
        BYTE PICHUGE* Reserved2;
        BYTE PICHUGE* Reserved3;
        BYTE PICHUGE* Reserved4;
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;    /* PF2_ flags above when implemented */
        LONG          Subcode;		/* 0 = dust removal(only value for now) */
									/*	16 = red eye removal on JPEG or 24 bit BGR files  */
        LONG          Stride;
        LONG          RectX;  /* x coordinate of upper left corner of rect. */
        LONG          RectY;  /* y coordinate of upper left corner of rect. */
        LONG          RectWidth;
        LONG          RectHeight;
        LONG          ThreshAdjust; /* (for subcode 0) for method 0 only,between -255 and 255 (default 0) */
        LONG          FiltAdjust;   /* (for subcode 0) for method 0 only,between -6 and 2 ( default 0) */
        LONG          IterAdjust;   /* (for subcode 0) for method 0 only,between -5 and 100 (default 0) */
        LONG          Method;       /* (for subcode 0) 0 or 1 */
									/* (for subcode 16) 0 , 1, or 2 (how redeye is found) */
		LONG		  Darkness;		/* (for subcode 16)  - degree of making eye dark */
									/* Last 4 bits for eye darknes 0x3 - light, 0 - normal, 1 dark */
									/* (Next 4 bits) Reduce Eye Glare: 0x10 -slightly reduce, 0x20 Reduce eye glare */
		LONG		  Alignment;	/* (for subcode 16) and JPEG , set none zero */
									/* may try 0 if you know jpeg is 111 SubSampled */
		LONG		  Brightness;	/* (for subcode 4) -100 to +100 with 0 as default */
		LONG		  Contrast;		/* (for subcode 4) -100 to +100 with 0 as default */
		double		  Gamma;		/* (for subcode 4) 0.1 (bright) to 10.0 (dark) with 1.0 as default */
		LONG		  Angle;		/* (for subcode 8) -450 to +450 where 1 = 0.1 degree */
        BITMAPINFOHEADER BiOut;     /* (for subcode 8) Header for output image */
        } CLEAN;

#define PF2_DarkSpeck           (0x00000001L)

/***************************************************************************\
*   Variables for Image Delivery Protocol (OP_RIDP, OP_TIDP)
\***************************************************************************/

typedef struct {
        DWORD         Priority;     /* 1 is lowest, 0 is the same as 1 */
        DWORD         XOfs;         /* enclosing rectangle horizontal pixel offset */
        DWORD         YOfs;         /* enclosing rectangle vertical pixel offset */
        DWORD         Width;        /* enclosing rectangle width in pixels */
        DWORD         Height;       /* enclosing rectangle height in pixels */
        DWORD         XBox;         /* horizontal pixels per bitmap box, 0 is the same as 1 */
        DWORD         YBox;         /* vertical pixels per bitmap box, 0 is the same as 1 */
        BYTE*         BoxBitmap;    /* ROI boxes within enclosing rectangle */
        } IDPREGION;

typedef struct {
        DWORD         Reserved0;
        IDPREGION PICHUGE* RegionsOfInterest;
        BYTE PICHUGE* RcvrState;
        BYTE PICHUGE* UpdatedTiles; /* bitmap of changed tiles for OP_W2D */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS      PicFlags;
        PICFLAGS      PicFlags2;    /* PF2_ flags when implemented */
        LONG          NumRegions;   /* # regions in RegionsOfInterest */
        LONG          RcvrStateSize;/* total size of RcvrState buffer */
        LONG          RcvrStateLen; /* data length in RcvrState buffer */
        THUMBNAIL     OutputResolution; /* don't bother with output at finer resolution than this */
        DWORD         OutputSize;   /* Data length to be output,
                                        0 = unlimited output according to
                                        parameters */
                                        
		LONG          XmitAppData;  /* set bit 0 to transmit comments
                                       set bit 1 to transmit raw data packets
                                       set bit 2 to transmit color table */
        DWORD         StateStamp;   /* output by OP_RIDP to be input to OP_TIDP as a verification
                                        that their states are synchronized */
        DWORD         NumTiles;     /* returned by RIDP after REQ_INIT so DeltaTileBitmap's
                                        size is known so it can be allocated */
        } IDP_UNION;

/* bit flags for XmitAppData field above */
#define IDPXMIT_Comment         ( 1 )
#define IDPXMIT_RawData         ( 2 )
#define IDPXMIT_ColorPalette    ( 4 )

/***************************************************************************\
*   Variables for JPEG 2000 operations (Operations J2KP, J2KE)              *
\***************************************************************************/

#define WT_9_7  0                   /* irreversible wavelet transform (lossy) */
#define WT_5_3  1                   /* reversible wavelet transform (lossless) */

#define TCT_Default     0           /* apply YCbCr <-> RGB transform to first 3 components,
                                        component transform depends on wavelet transform */
#define TCT_None        1           /* do not apply component transform */
#define TCT_YCbCr       2           /* apply YCbCr <-> RGB transform to first 3 components */

#define PO_LRCP         1           /* Layer-Resolution level-Component-Position progression */
#define PO_RLCP         2           /* Resolution level-Layer-Component-Position progression */
#define PO_RPCL         3           /* Resolution level-Position-Component-Layer progression */
#define PO_PCRL         4           /* Position-Component-Resolution level-Layer progression */
#define PO_CPRL         5           /* Component-Position-Resolution level-Layer progression */

typedef struct {
        DWORD    ComponentNumber;   /* numbered 0..M as implied by partition ordering */
        DWORD    WaveletTransform;  /* WT_9_7 (default, irreversible) or WT_5_3 (reversible) */
        DWORD    CompressionProfile;  /* Best quality = 0 <= CompressionProfile <= 10 = best compression */
                                      /* ignored if CompFileSize (below) is not 0. */
                                      /* (it's currently just mapped to Rate presets, so it's not very useful yet */
        DWORD    DecompositionLevels; /* (0 = default) actual wavelet decomposition levels (up to 10) */
        DWORD    SkippedLevels;     /* # highest-res levels skipped (Total = Skipped+Decomposition) */
        DWORD    Reserved[8];
    } TILECOMP;

typedef struct {
        DWORD    TileNumber;        /* numbered 0..N in left-right/top-down order */
        DWORD    TileCompTransform; /* TCT_Default, TCT_None, TCT_YCbCr */
        TILECOMP DftTileComp;       /* these settings apply to all this tile's components not in OtherTileComps array */
        DWORD    NumOtherTileComps;
        TILECOMP PICHUGE* OtherTileComps;
        DWORD    NumLayers;         /* 0 = default used (which is 1) bit slice layers */
        DWORD    ProgressionOrder;  /* 0 = default used (which is PO_LRCP) */
        DWORD    Reserved[8];
    } TILE;

typedef struct {
        QUEUE    Queue;
        DWORD    StripSize;         /* Minimum size of buffer to hold a strip */
        REGION   Region;
    } PARTITION;

typedef struct {
        DWORD         Reserved0;
        BYTE PICHUGE* Reserved1;    /* Must be NULL! */
        BYTE PICHUGE* Reserved2;    /* Must be NULL! */
        BYTE PICHUGE* Reserved3;    /* Must be NULL! */
        BYTE PICHUGE* Reserved4;    /* Must be NULL! */
        BYTE PICHUGE* Reserved5;    /* Must be NULL! */
        BYTE PICHUGE* Reserved6;    /* Must be NULL! */
        BYTE PICHUGE* Reserved7;    /* Must be NULL! */
        BYTE PICHUGE* Reserved8;    /* Must be NULL! */
        PICFLAGS    PicFlags;
        PICFLAGS    PicFlags2;      /* PF2_SkipJP2Header, PF2_ForceLossless */

        /* following in reference grid coordinates */
        DWORD       ImageWidth;     /* Xsiz - XOsiz */
        DWORD       ImageHeight;    /* Ysiz - YOsiz */
        DWORD       ImageXOff;      /* XOsiz */
        DWORD       ImageYOff;      /* YOsiz */

        DWORD       StripSize;      /* Minimum size of buffer to hold a strip */
        REGION      Region;
        DWORD       NumOtherPartitions;
        PARTITION PICHUGE* OtherPartitions;
        DWORD       PartitionNum;   /* Used by Defer to determine which partition */

        /* following in reference grid coordinates */
        DWORD       TileWidth;      /* XTsiz, 0 is the same as TileWidth = ImageWidth */
        DWORD       TileHeight;     /* YTsiz, 0 is the same as TileHeight = ImageHeight */
        DWORD       TileXOff;       /* XTOsiz, 0th tile origin X/Y offset from reference grid origin */
        DWORD       TileYOff;       /* YTOsiz */

        TILE        DftTile;        /* these settings apply to all components for all tiles not in OtherTiles array */
        DWORD       NumOtherTiles;
        TILE PICHUGE* OtherTiles;

        DWORD       Rate;           /* Bits per pixel * 1000, or compressed size */
        THUMBNAIL   Thumbnail;      /* (expand only) power of 2 reduced size */
        DWORD       Resolution;     /* (expand only) limit expansion by discarding higher detail */
        DWORD       CompFileSize;   /* Desired compressed file size; if 0, use Compression profile instead. */
    } J2K_UNION;

#define PF2_SkipJP2Header           0x00000001L
        /* (JPEG 2000) create minimal JPEG 2000 stream file, not full JP2 file */
#define PF2_ForceLossless           0x00000002L
        /* (JPEG 2000) Override CompressionProfile, Rate, and Transforms to ensure lossless. */
#define PF2_KeepColorTable          0x00000004L
        /* (JPEG 2000) For Color Mapped files only, optionally keep the table with the
            compressed image.  This option is ignored if PF2_SkipJP2Header is set. */

/*.P*/
/***************************************************************************\
*  This structure is allocated by the user and its address passed to        *
*  the Pegasus routines and to any callback routines.                       *
*  The values which need to be specified in the PIC_PARM structure vary     *
*  depending on which operation is performed, and on whether the            *
*  REQ_INIT or REQ_EXEC request is made.  See the documentation for         *
*  specific details.                                                        *
\***************************************************************************/

typedef struct PIC_PARM_TAG {
        DWORD        Reserved0;
        /*  Elements shared by most operations */
        LONG         ParmSize;      /* Size of this structure (bytes) */
        BYTE         ParmVer;       /* Version of parameters for given Op (11..99 ) */
        BYTE         ParmVerMinor;  /* Minor version number of parameter structure */
        WORD         Reserver1;     /* should be 0 for now */
        LONG         Status;        /* 0 or error code (see ERRORS.H) */
        OPERATION    Op;            /* OP_ fields, specific operation to be performed */
        BITMAPINFOHEADER Head;      /* Uncompressed Image (UI) Width, Height, etc. */
        RGBQUAD      ColorTable[272]; /* Holds primary (ó256) & secondary (ó16) palettes */
#if 0
#if defined(MACINTOSH)
        CTabHandle  MacCTable;      /* Color Table used on the Macintosh */
#endif
#endif
        LONG         PicVer;        /* PIC image version */
        ORIENTATION  VisualOrient;  /* O_ fields, displayed orientation of UI */
        LONG         CommentSize;   /* Size of comment buffer (bytes) */
        LONG         CommentLen;    /* Length of comment (0 if no comment) */
        CHAR PICHUGE *Comment;      /* Points to comment up to CommentSize bytes */
        LPARAM       App;           /* any user info needed by the callbacks */
        LONG         PercentDone;   /* Strip progress monitor, 100 iff done */
        WORK_AREA PICHUGE *Reserved;/* Allocated by Pegasus routines, init NULL */
        /*  Buffer information. Can be used as circular buffers. */
        QUEUE        Get;
        QUEUE        Put;
        BYTE         KeyField[8];   /* Used to hold encode/decode key */
        /* The following function is called in the non-coroutine case by Pegasus */
        /* to ask for more data, space, etc. from the application.  DeferFn is an */
        /* application function that returns non-zero if Pegasus is to terminate */
        LONG         (PICFAR *DeferFn)(struct PIC_PARM_TAG PICFAR *, RESPONSE);
        DWORD        Flags;         /* See F_ defines */
        DWORD        Flags2;        /* See F2_ defines when implemented */
        WORD         CropWidth;     /* These next four fields have meaning only */
        WORD         CropHeight;    /* if the Flags F_Crop bit is set.  The */
        WORD         CropXoff;      /* pixel position are relative to the */
        WORD         CropYoff;      /* image's logical upper-left corner */
        DWORD        ImageNumber;   /* Which image of a multi-image file (0 = next) */
        DWORD        PacketType;    /* Type of PIC2 packet for which space is to be reserved */
        DWORD        SeekInfo;      /* see components of SeekInfo below */

        CHAR PICHUGE* PIC2List;
        LONG         PIC2ListSize;
        LONG         PIC2ListLen;
        REGION       RegionIn;
        REGION       RegionOut;
        WORD         OpVersion0;    /* lsw of 64-bit opcode FileVersion */
        WORD         OpVersion1;    /* word 1 of 64-bit opcode FileVersion */
        WORD         OpVersion2;    /* word 2 of 64-bit opcode FileVersion */
        WORD         OpVersion3;    /* msw of 64-bit opcode FileVersion */

        BYTE PICHUGE* ReservedPtr1;
        BYTE PICHUGE* ReservedPtr2;
        BYTE PICHUGE* ReservedPtr3;
        BYTE PICHUGE* ReservedPtr4;
        BYTE PICHUGE* ReservedPtr5;
        BYTE PICHUGE* ReservedPtr6;
        BYTE PICHUGE* ReservedPtr7;
        BYTE PICHUGE* ReservedPtr8;

        CHAR PICHUGE* LoadPath;     /* Path to directory with opcode DLL */
        DWORD         LoadResInstance; /* Module instance handle of an
                                       EXE or DLL from whose resource data
                                       the opcode DLL is to be loaded */
            /* if LoadPath == 0 && LoadResInstance == 0
                    then the opcode DLL is loaded from the same directory as
                    the dispatcher DLL.  If not found there then the opcode
                    DLL is loaded using the directory order Windows uses
                    when loading DLLs
               if LoadPath != 0 && LoadResInstance == 0,
                    then the opcode DLL is loaded from the specified directory
               if LoadPath == 0 && LoadResInstance != 0
                    then the opcode DLL is loaded from the specified module's
                    resource data
               if LoadPath != 0 && LoadResInstance != 0
                    then the opcode DLL is loaded from the specified module's
                    resource data.  If not found there, then the opcode DLL
                    is loaded from the specified path. If LoadPath is "",
                    and the opcode DLL is not found in the specified module's
                    resource data, then the opcode DLL is loaded as though
                    LoadPath == 0 && LoadResInstance == 0
            */
        struct PIC_PARM_TAG PICHUGE* NestPP;    /* reserved picnclib.c */
        REQUEST       NestReq;                  /* reserved picnclib.c */
        DWORD         tlsReserved;              /* reserved picdisp.c */
        DWORD         tlsReserved2;             /* reserved picdisp.c */
        DWORD         memReserved;              /* reserved opmem.c */
        DWORD         wrapReserved;             /* reserved opmain.c */
        DWORD         wrapReserved2;            /* reserved opmain.c */
        WORD          DispVersion0;    /* lsw of 64-bit dispatcher FileVersion */
        WORD          DispVersion1;    /* word 1 of 64-bit dispatcher FileVersion */
        WORD          DispVersion2;    /* word 2 of 64-bit dispatcher FileVersion */
        WORD          DispVersion3;    /* msw of 64-bit dispatcher FileVersion */
        DWORD         DispExports;     /* reserved picnclib.c, picmacos.c, picunixos.c, etc. */
        DWORD         tlsReserved3;             /* reserved picdisp.c */
        DWORD         Reserveds[99];
        /* see also: Internal Use Only comments in internal.h */
        /*  Operator specific information */
        union {
            PEGQUERY    QRY;        /* PegasusQuery (NOT opcode) */
            DIB_INPUT   D2J;        /* OP_D2J (progressive or sequential) */
            DIB_OUTPUT  J2D;        /* OP_J2D, OP_JE2D, OP_S2D, OP_SE2D */
            TRANS2P     S2P;        /* OP_S2P */
            TRANP2S     P2S;        /* OP_P2S */
            D2F_STRUC   D2F;        /* OP_D2F */
            F2D_STRUC   F2D;        /* OP_F2D */
            UTL_STRUC   UTL;        /* OP_UTL */
            LOSSLESS    LL;         /* OP_LIMP, OP_LIME */
            LOSSLESS3   LL3;        /* OP_LIP3, OP_LIE3 */
            REORIENT    ROR;        /* OP_ROR */
            ZOOM_PARMS  ZOOM;       /* OP_ZOOM */
            WAVELET     WAVE;       /* OP_W2D, OP_D2W */
            PNG_UNION   PNG;        /* Portable Network Graphics */
            WSQ_UNION   WSQ;        /* Wavelet Scalar Quantization */
            DJVU_UNION  DJVU;       /* OP_DJVU2D, OP_D2DJVU */
            DIB_INPUT   D2S;        /* please use D2J instead */
            DIB_OUTPUT  S2D;        /* please use J2D instead */
            DIB_OUTPUT  P2D;        /* please use J2D instead */
            TIFF_EDIT   TED;        /* Tiff Edit - OP_TIFEDIT */
            MODCA_UNION Modca;      /* Modca/CALS */
            CLEAN       CLN;        /* OP_CLEAN */
            IDP_UNION   IDP;        /* OP_RIDP, OP_TIDP */
            J2K_UNION   J2K;        /* OP_J2KP or OP_J2KE */
            INTERNAL    Reserved;   /*Always reserved as the largest variant */
        } u;
    } PIC_PARM;

        /*the components of SeekInfo */
#define SEEK_OFFSET     0x3FFFFFFFL  /* bits 0-29: unsigned offset in file */
#define SEEK_FILE       0x40000000L  /* bit 30: 0/1 => seek input/output queue */
#define SEEK_DIRECTION  0x80000000L  /* bit 31: 0/1 => seek from beg./end of file */

        /* Flag bit fields for general image information */
#define F_Crop          0x00000001  /* Set if image is to be cropped */
#define F_Raw           0x00000002  /* Set if pack source or unpack dest. is RAW */
#define F_Bmp           0x00000004  /* Set if pack source or unpack dest. is BMP */
#define F_XOut          0x00000008  /* reserved, for internal use only */
#define F_XIn           0x00000010  /* reserved, for internal use only */
#define F_DoReqExecNext 0x00000020  /* used by KSAPI */
#define F_DidReqExec    0x00000040  /* used by KSAPI */
#define F_AsyncAbort    0x00000080  /* used by MTAPI */
#define F_UseDeferFn    0x00000100  /* allows use of DeferFn when DeferFn != 0
                                        and opcode would otherwise return the
                                        response from Pegasus */

/*.P*/
/***************************************************************************\
*   void  Verify (BOOL x, PIC_PARM *p, LONG err);                           *
*                                                                           *
*   This is a macro which tests the value of x.  Iff x is false, Verify     *
*   will return control back to the application from any nested depth.      *
*   p->status is set to err, the operation is terminated, the Pegasus       *
*   routine is terminated, and the Reserved is freed.                       *
*                                                                           *
*   void  ChkDefer (BOOL x, PIC_PARM *p);                                   *
*                                                                           *
*   This is a macro which tests the value of x.  Iff x is false, it         *
*   will return control back to the application from any nested depth.      *
*   The Pegasus routine is terminated, and the Reserved is freed.           *
*                                                                           *
*   void  Defer (PIC_PARM *p, RESPONSE res);                                *
*                                                                           *
*   This is a macro which will pass control back to the application         *
*   with the result res.  It is assumed that control will resume after      *
*   the Defer statement when the application continues with the next        *
*   call to the Pegasus routine. Note that res should not equal RES_ERR.    *
\***************************************************************************/
#define Verify(x,p,err)     while (!(x)) (p)->Status = (err), \
                                longjmp((p)->Reserved->AppState, RES_ERR)

#define VerifyFALSE(p,err)  (p)->Status = (err), longjmp((p)->Reserved->AppState, RES_ERR)

/***************************************************************************\
*  Prototypes for Pegasus functions.                                        *
\***************************************************************************/
#ifdef __cplusplus
    extern "C" {
#endif

PUBLIC  BOOL DLLEXPORTWINAPI PegasusQuery (PIC_PARM PICHUGE *p);

#if !defined(MACINTOSH)
PUBLIC  BOOL DLLEXPORTWINAPI PegasusCB (PIC_PARM PICHUGE *p,
                 void (PICFAR *GetData) (PIC_PARM PICHUGE *p),
                 void (PICFAR *PutData) (PIC_PARM PICHUGE *p) );
#endif

PUBLIC  RESPONSE DLLEXPORTWINAPI Pegasus (PIC_PARM PICHUGE *p, REQUEST req);

PUBLIC  LONG DLLEXPORTWINAPI PegasusLoad(
    OPERATION Op,
    LONG ParmVer,
    char PICFAR *Path);

PUBLIC  LONG DLLEXPORTWINAPI PegasusLoadFromRes(
    OPERATION Op,
    LONG ParmVer,
    char PICFAR *Path,
    DWORD hInstance);

PUBLIC  void DLLEXPORTWINAPI PegasusUnload (OPERATION Op, LONG ParmVer);

PUBLIC  void DLLEXPORTWINAPI PegasusTrace (const char PICFAR *pszMsg);

PUBLIC  BOOL DLLEXPORTWINAPI PegasusLibInit (DWORD hInstance);

PUBLIC  void DLLEXPORTWINAPI PegasusLibTerm (void);

PUBLIC  BOOL DLLEXPORTWINAPI PegasusLibInitNest (DWORD hInstance);

PUBLIC  void DLLEXPORTWINAPI PegasusLibTermNest (void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadInit (void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadTerm (void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadInitNest (void);

PUBLIC  void DLLEXPORTWINAPI PegasusLibThreadTermNest (void);

#ifdef __cplusplus
    }
#endif

#ifdef __MWERKS__
    /*#### find out how to save/restore the alignment */
#elif defined(__GNUC__)
        #pragma pack()
#elif defined(__unix__)
        #pragma pack()
#elif defined(__BORLANDC__)
    /*#### find out how to save/restore the alignment
            "#pragma option -a" doesn't do it in BC 4.5 */
#else /* assume microsoft or watcom */
    #if defined(__FLAT__) || defined(__WATCOMC__)
        #pragma pack(pop)
    #else
        /* in MS 16-bit, best we can do is to restore to the command-line state */
        #pragma pack()
    #endif
#endif

#endif  /* #if !defined(_PIC) */

