/* SCEI CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library libexcep Version 1.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.0
 *                           S-JIS
 *
 *      Copyright (C) 1998-2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <libexcep - libexcep.h>
 *                       <Exception library> 
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *         1.0       Oct.6.2000      kumagae     First release
 */

#ifndef _LIBEXCEP_H
#define _LIBEXCEP_H

#include <sif.h>
#include <sifcmd.h>

typedef struct {
  int reg[45];
  int version;
  int offset;
  char module[32];
}sceExcepIOPExceptionData  __attribute__((aligned(16)));

//EE,IOP Common GPR  
#define GPR_at 1
#define GPR_v0 2
#define GPR_v1 3
#define GPR_a0 4
#define GPR_a1 5
#define GPR_a2 6
#define GPR_a3 7
#define GPR_t0 8
#define GPR_t1 9
#define GPR_t2 10
#define GPR_t3 11
#define GPR_t4 12
#define GPR_t5 13
#define GPR_t6 14
#define GPR_t7 15
#define GPR_s0 16
#define GPR_s1 17
#define GPR_s2 18
#define GPR_s3 19
#define GPR_s4 20
#define GPR_s5 21
#define GPR_s6 22
#define GPR_s7 23
#define GPR_t8 24
#define GPR_t9 25
#define GPR_k0 26
#define GPR_k1 27
#define GPR_gp 28
#define GPR_sp 29
#define GPR_fp 30
#define GPR_ra 31

//IOP Additional Register
#define IOP_HI            (32)
#define IOP_LO            (33)
#define IOP_SR            (34)
#define IOP_EPC           (35)
#define IOP_IEIDI         (36)
#define IOP_CAUSE         (37)
#define IOP_TAR          (38)
#define IOP_BADVADDR     (39)
#define IOP_DCIC         (40)
#define IOP_BPC          (41)
#define IOP_BPCM         (42)
#define IOP_BDA          (43)
#define IOP_BDAM         (44)


/*
 *	Prototypes
 */
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

void sceExcepConsOpen(u_int gs_x, u_int gs_y, u_int ch_w, u_int ch_h);
void sceExcepConsPrintf(const char* str, ...);
void sceExcepConsLocate(u_int x, u_int y);
void sceExcepConsClose( void );

void sceExcepSetDebugIOPHandler(char *panicsys,  sceSifCmdHandler IOPExceptionHandler ,sceExcepIOPExceptionData  *pIOPExceptionData);
void sceExcepSetDebugEEHandler( void (*handler)(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa,  u_long128 *gpr));

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _LIBEXCEP_H */
