/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.02
 *                           S-JIS
 *
 *      Copyright (C) 1998-2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <libscf - libscf.h>
 *                  <system configuration library> 
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.0            Jul.31.2000     kumagae     First release
 *       1.01           Apr,16,2003     hana        add sceScfGetErxEntries
 *       1.02           Sep,26,2003     hana        change comment
 */

#ifndef _LIBSCF_H
#define _LIBSCF_H

#ifndef _LIBCDVD_H
#include <libcdvd.h>
#endif


/* ASPECT */
#define SCE_ASPECT_43   0
#define SCE_ASPECT_FULL 1
#define SCE_ASPECT_169  2

/* DATE NOTATION */
#define SCE_DATE_YYYYMMDD   0
#define SCE_DATE_MMDDYYYY   1
#define SCE_DATE_DDMMYYYY   2

/* LANGUAGE */
#define SCE_JAPANESE_LANGUAGE   0
#define SCE_ENGLISH_LANGUAGE    1
#define SCE_FRENCH_LANGUAGE     2
#define SCE_SPANISH_LANGUAGE    3
#define SCE_GERMAN_LANGUAGE     4
#define SCE_ITALIAN_LANGUAGE    5
#define SCE_DUTCH_LANGUAGE      6
#define SCE_PORTUGUESE_LANGUAGE 7 

/* SPDIF */
#define SCE_SPDIF_ON        0
#define SCE_SPDIF_OFF       1

/* SUMMER TIME */
#define SCE_SUMMERTIME_OFF  0
#define SCE_SUMMERTIME_ON   1

/* TIME NOTATION */
#define SCE_TIME_24HOUR     0
#define SCE_TIME_12HOUR     1

typedef struct {
    short  TimeZone;
    u_char Aspect; 
    u_char DateNotation;
    u_char Language;
    u_char Spdif;
    u_char SummerTime;
    u_char TimeNotation;
} sceScfT10kConfig;
 

/*
 *  Prototypes
 */
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

void sceScfSetT10kConfig(sceScfT10kConfig *config);
int sceScfGetLanguage(void);
int sceScfGetAspect(void);
int sceScfGetSpdif(void);
int sceScfGetTimeZone(void);
int sceScfGetDateNotation(void);
int sceScfGetSummerTime(void);
int sceScfGetTimeNotation(void);
void sceScfGetLocalTimefromRTC(sceCdCLOCK *rtc);
void sceScfGetGMTfromRTC(sceCdCLOCK *rtc);
void *sceScfGetErxEntries(void);
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif


#endif /* _LIBSCF_H */

