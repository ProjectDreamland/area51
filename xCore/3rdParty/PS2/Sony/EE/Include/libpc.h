/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.05
 *                           Shift-JIS
 *
 *         Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                        libpc - libpc.h 
 *              Performance Counter access functions
 *
 *      Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.00           06/21/1999      koji        first version
 *      0.01           12/08/1999      koji        bug fix, TLB to ITLB
 *      0.02           01/23/2001      akiyuki     addition of prototype
 *                                                  declaration and __asm__
 *      0.03           04/16/2003      hana        add scePcGetErxEntries
 *      0.04           07/24/2003      hana        add _LANGUAGE_ASSEMBLY
 *      0.05           09/26/2003      hana        change comment
 */

#ifndef _LIBPC_H
#define _LIBPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Bitfields in the CCR register */
#define SCE_PC_EXL0    (1  <<  1)
#define SCE_PC_K0      (1  <<  2)
#define SCE_PC_S0      (1  <<  3)
#define SCE_PC_U0      (1  <<  4)
#define SCE_PC_EVENT0  (31 <<  5)
#define SCE_PC_EXL1    (1  << 11)
#define SCE_PC_K1      (1  << 12)
#define SCE_PC_S1      (1  << 13)
#define SCE_PC_U1      (1  << 14)
#define SCE_PC_EVENT1  (31 << 15)
#define SCE_PC_CTE     (1  << 31)

/* Performance Counter Events */
#define SCE_PC0_RESERVED            (0  <<  5)
#define SCE_PC0_CPU_CYCLE           (1  <<  5) /* Processor cycle */
#define SCE_PC0_SINGLE_ISSUE        (2  <<  5) /* Single instructions issue */
#define SCE_PC0_BRANCH_ISSUED       (3  <<  5) /* Branch issued */
#define SCE_PC0_BTAC_MISS           (4  <<  5) /* BTAC miss */
#define SCE_PC0_ITLB_MISS           (5  <<  5) /* ITLB miss */
#define SCE_PC0_ICACHE_MISS         (6  <<  5) /* Instruction cache miss */
#define SCE_PC0_DTLB_ACCESSED       (7  <<  5) /* DTLB accessed */
#define SCE_PC0_NONBLOCK_LOAD       (8  <<  5) /* Non-blocking load */
#define SCE_PC0_WBB_SINGLE_REQ      (9  <<  5) /* WBB single request */
#define SCE_PC0_WBB_BURST_REQ       (10 <<  5) /* WBB burst request */
#define SCE_PC0_ADDR_BUS_BUSY       (11 <<  5) /* CPU address bus busy */
#define SCE_PC0_INST_COMP           (12 <<  5) /* Instruction completed */
#define SCE_PC0_NON_BDS_COMP        (13 <<  5) /* Non-BDS instruction completed */
#define SCE_PC0_COP2_COMP           (14 <<  5) /* COP2 instruction completed */
#define SCE_PC0_LOAD_COMP           (15 <<  5) /* Load completed */
#define SCE_PC0_NO_EVENT            (16 <<  5) /* No event */

#define SCE_PC1_LOW_BRANCH_ISSUED   (0  << 15) /* Low-order branch issued */
#define SCE_PC1_CPU_CYCLE           (1  << 15) /* Processor cycle */
#define SCE_PC1_DUAL_ISSUE          (2  << 15) /* Dual instructions issue */
#define SCE_PC1_BRANCH_MISS_PREDICT (3  << 15) /* Branch miss-predicted */
#define SCE_PC1_TLB_MISS            (4  << 15) /* TLB miss */
#define SCE_PC1_DTLB_MISS           (5  << 15) /* DTLB miss */
#define SCE_PC1_DCACHE_MISS         (6  << 15) /* Data cache miss */
#define SCE_PC1_WBB_SINGLE_UNAVAIL  (7  << 15) /* WBB single request unavailable */
#define SCE_PC1_WBB_BURST_UNAVAIL   (8  << 15) /* WBB burst request unavailable */
#define SCE_PC1_WBB_BURST_ALMOST    (9  << 15) /* WBB burst request almost full */
#define SCE_PC1_WBB_BURST_FULL      (10 << 15) /* WBB burst request full */
#define SCE_PC1_DATA_BUS_BUSY       (11 << 15) /* CPU data bus busy */
#define SCE_PC1_INST_COMP           (12 << 15) /* Instruction completed */
#define SCE_PC1_NON_BDS_COMP        (13 << 15) /* Non-BDS instruction completed */
#define SCE_PC1_COP1_COMP           (14 << 15) /* COP1 instruction completed */
#define SCE_PC1_STORE_COMP          (15 << 15) /* Store completed */
#define SCE_PC1_NO_EVENT            (16 << 15) /* No event */

#if !defined(_LANGUAGE_ASSEMBLY)

void scePcStart(int control, int counter0, int counter1);
void scePcStop(void);
void *scePcGetErxEntries(void);

/* select inline function strategy */
#ifndef INLINE
#define INLINE  extern __inline__
#endif

INLINE int scePcGetCounter0(void);
INLINE int scePcGetCounter0(void) {
    register int ctr0;
    __asm__ volatile ("mfpc %0, 0": "=r" (ctr0));
    return ctr0;
}

INLINE int scePcGetCounter1(void);
INLINE int scePcGetCounter1(void) {
    register int ctr1;
    __asm__ volatile ("mfpc %0, 1": "=r" (ctr1));
    return ctr1;
}

#endif /* !defined(_LANGUAGE_ASSEMBLY) */

#ifdef __cplusplus
}
#endif

#endif /* _LIBPC_H */

