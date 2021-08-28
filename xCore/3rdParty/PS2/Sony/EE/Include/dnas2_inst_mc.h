/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library "DNAS" package (Release 3.0 version)
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                 <libdnas2 - dnas2_inst_nohdd.h>
 *                         <DNAS header>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      1.00            Sep,28,2002     munekis     first version
 */


#ifndef __SCE_DNAS2_INST_MC_H__
#define __SCE_DNAS2_INST_MC_H__

/*********************

  function prototypes

***********************/

#ifdef __cplusplus
extern "C" {
#endif

/* to rename API */
#define sceDNAS2InstInit_mc						sceDNAS2InstInit_nohdd
#define sceDNAS2InstPersonalizeDataLength_mc 	sceDNAS2InstPersonalizeDataLength_nohdd
#define sceDNAS2InstPersonalizeData_mc			sceDNAS2InstPersonalizeData_nohdd
#define sceDNAS2InstExtractDataLength_mc		sceDNAS2InstExtractDataLength_nohdd
#define sceDNAS2InstExtractData_mc				sceDNAS2InstExtractData_nohdd
#define sceDNAS2InstShutdown_mc					sceDNAS2InstShutdown_nohdd

#define sceDNAS2InstConvertError_mc				sceDNAS2InstConvertError_nohdd





/* API */
int sceDNAS2InstInit_nohdd(void);

int sceDNAS2InstPersonalizeDataLength_nohdd( u_int src_length,
                               u_char *src_ptr,
                               u_int *dst_length);

int sceDNAS2InstPersonalizeData_nohdd( u_int src_length,
                         u_int dst_length,
                         u_char *src_ptr);

int sceDNAS2InstExtractDataLength_nohdd( u_int src_length,
                               u_char *src_ptr,
                               u_int *dst_length);

int sceDNAS2InstExtractData_nohdd( u_int src_length,
                         u_int dst_length,
                         u_char *src_ptr);

int sceDNAS2InstShutdown_nohdd(void);




/* function to translate DNAS error code to general one */
/* (used by ERX libraries, internally) */

typedef enum SceDNAS2InstNohddFunc {
    SCE_DNAS2_INST_NOHDD_FUNC_ANYTHING
} SceDNAS2InstNohddFunc;

int sceDNAS2InstConvertError_nohdd(SceDNAS2InstNohddFunc func, int error);



/* erx */
void *sceDNAS2InstGetErxEntries(void);

#ifdef __cplusplus
}
#endif


/*
  error codes.
*/

/* DNAS2 error code (success) */
#define sceDNAS2_OK                         0    /* no problem. */



#endif /* __SCE_DNAS2_INST_MC_H__ */
