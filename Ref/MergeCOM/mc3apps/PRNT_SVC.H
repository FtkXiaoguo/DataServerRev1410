/*************************************************************************
 *
 *       System: MergeCOM-3 Print Sample Application Include File
 *
 *    $Workfile: prnt_svc.h $
 *
 *    $Revision: 20529 $
 *
 *        $Date: 2005-10-26 08:55:17 +0900 (水, 26 10 2005) $
 *
 *       Author: Merge eFilm
 *
 *  Description: This module contains defintions used by applications
 *               which use the MergeCOM-3 Advanced Tool Kit Print Service
 *	             functions.
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
/* $NoKeywords: $ */


#ifndef MTI_PRNT_SVC_H

#ifdef __cplusplus
extern "C" {
#endif 


typedef struct Image_Box
{
    char              UID[68];
    unsigned short    image_position;
    char              polarity[20];
    char              requested_image_size[20];
    char              magnification_type[20];
    char              smoothing_type[20];
    unsigned short    pixel_sequence_defined;
    int               samples_per_pixel;
    char              photometric_interpretation[20];
    int               rows;
    int               columns;
    char              pixel_aspect_ratio[2][20];
    unsigned short    bits_allocated;
    unsigned short    bits_stored;
    unsigned short    high_bit;
    int               pixel_representation;
    unsigned short    pixel_data;
    char              film_box_uid[68];
    unsigned short    fields_set;
    struct Image_Box  *next;
} IMAGE_BOX;

#define PRNT_IMAGE_PIXEL_DATA_EMPTY         0
#define PRNT_IMAGE_PIXEL_DATA_PRESENT       1

#define PRNT_IMAGE_UID_SET                  1
#define PRNT_IMAGE_IMAGE_POSITION_SET       2
#define PRNT_IMAGE_POLARITY_SET             4
#define PRNT_IMAGE_REQUESTED_IMAGE_SIZE_SET 8
#define PRNT_IMAGE_MAGNIFICATION_TYPE_SET  16
#define PRNT_IMAGE_SMOOTHING_TYPE_SET      32
#define PRNT_IMAGE_FILM_BOX_UID_SET        64

typedef struct Film_Box
{
    char              UID[68];
    char              image_display_format[2][32];
    char              annotation_display_format_id[20];
    char              film_orientation[20];
    char              film_size_id[20];
    char              magnification_type[20];
    char              smoothing_type[20];
    char              border_density[20];
    char              empty_image_density[20];
    int               min_density;
    int               max_density;
    char              trim[20];
    char              configuration_info[1024];
    char              ref_film_session[68];
    IMAGE_BOX         *image_boxes[50];
    unsigned short    fields_set;
    struct Film_Box   *next;
} FILM_BOX;

#define PRNT_FILM_UID_SET                      1
#define PRNT_FILM_IMAGE_DISPLAY_FORMAT         2
#define PRNT_FILM_ANNOTATION_DISPLAY_FORMAT_ID 4
#define PRNT_FILM_FILM_ORIENTATION_SET         8
#define PRNT_FILM_FILM_SIZE_ID_SET            16
#define PRNT_FILM_MAGNIFICATION_TYPE_SET      32
#define PRNT_FILM_SMOOTHING_TYPE_SET          64
#define PRNT_FILM_BORDER_DENSITY_SET         128
#define PRNT_FILM_EMPTY_IMAGE_DENSITY_SET    256
#define PRNT_FILM_MIN_DENSITY_SET            512
#define PRNT_FILM_MAX_DENSITY_SET           1024
#define PRNT_FILM_TRIM_SET                  2048
#define PRNT_FILM_CONFIGURATION_INFO_SET    4096
#define PRNT_FILM_REF_FILM_SESSION_SET      8192
#define PRNT_FILM_IMAGE_BOXES_SET          16384

typedef struct Film_Session
{
    char                 UID[68];
    short                copies;
    short                priority;
    char                 medium_type[20];
    char                 film_destination[20];
    char                 film_session_label[20];
    long                 memory_allocation;
    FILM_BOX             *film_boxes[50];
    unsigned short       films;
    unsigned short       fields_set;
    struct Film_Session  *next;
} FILM_SESSION;

#define PRNT_HIGH_PRI                        1
#define PRNT_MED_PRI                         0
#define PRNT_LOW_PRI                        -1

#define PRNT_SESSION_UID_SET                 1
#define PRNT_SESSION_COPIES_SET              2
#define PRNT_SESSION_PRIORITY_SET            4
#define PRNT_SESSION_MEDIUM_TYPE_SET         8
#define PRNT_SESSION_FILM_DESTINATION_SET   16
#define PRNT_SESSION_FILM_SESSION_LABEL     32
#define PRNT_SESSION_MEMORY_ALLOCATION_SET  64
#define PRNT_SESSION_FILM_BOXES_SET        128

#define PRNT_BASIC_FILM_SESSION         1
#define PRNT_BASIC_FILM_BOX             2
#define PRNT_BASIC_GRAYSCALE_IMAGE_BOX  3
#define PRNT_PRINTER                    4
#define PRNT_PRINT_JOB					5


#if defined(_MSDOS)     || defined(__OS2__)   || defined(_WIN32) || \
    defined(_MACINTOSH) || defined(INTEL_WCC) || defined(_RMX3)
#define BINARY_READ "rb"
#define BINARY_WRITE "wb"
#define BINARY_APPEND "rb+"
#define BINARY_READ_APPEND "a+b"
#define BINARY_CREATE "w+b"
#ifdef _MSDOS
#define TEXT_READ "rt"
#define TEXT_WRITE "wt"
#else
#define TEXT_READ "r"
#define TEXT_WRITE "w"
#endif
#else
#define BINARY_READ "r"
#define BINARY_WRITE "w"
#define BINARY_APPEND "r+"
#define BINARY_READ_APPEND "a+"
#define BINARY_CREATE "w+"
#define TEXT_READ "r"
#define TEXT_WRITE "w"
#endif 



#ifdef __cplusplus
}
#endif 

#if defined(_PHAR_LAP)
int _getch( void );
int _kbhit( void );
#endif

#define MTI_MERGECOM_H
#endif
