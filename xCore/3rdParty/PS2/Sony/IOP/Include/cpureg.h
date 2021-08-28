/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: cpureg.h,v 1.4 1999/10/12 09:06:46 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         cpureg.h
 *                         CPU & CP0 registers names
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1998/10/12      tei
 */

#ifndef _CPUREG_H_
#define _CPUREG_H_

#if defined(__LANGUAGE_ASSEMBLY) || defined(__ASSEMBLER__) || defined(LANGUAGE_ASSEMBLY)
#define zero	$0	/* wired zero */
#define AT	$at	/* assembler temp */
#define v0	$2	/* return value */
#define v1	$3
#define a0	$4	/* argument registers */
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8	/* caller saved */
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16	/* callee saved */
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24	/* code generator */
#define t9	$25
#define k0	$26	/* kernel temporary */
#define k1	$27
#define gp	$28	/* global pointer */
#define sp	$29	/* stack pointer */
#define fp	$30	/* frame pointer */
#define ra	$31	/* return address */


#define C0_BPC		$3	/* breakpoint program counter register */
#define C0_BDA		$5	/* breakpoint data address register */
#define C0_TAR		$6	/* target address register (read only) */
#define C0_DCIC 	$7	/* debug and cache invalidate control register */
#define	C0_BADVADDR	$8	/* bad virtual address */
#define C0_BDAM		$9	/* breakpoint data address mask register */

#define C0_BPCM		$11	/* breakpoint program counter mask register*/
#define	C0_SR		$12	/* status register */
#define	C0_CAUSE	$13	/* exception cause */
#define	C0_EPC		$14	/* exception pc */
#define	C0_PRID		$15	/* revision identifier */
#endif

/*
 * Status register
 */
#define	SR_CUMASK	0xf0000000	/* coproc usable bits */

#define	SR_CU3		0x80000000	/* Coprocessor 3 usable */
#define	SR_CU2		0x40000000	/* Coprocessor 2 usable */
#define	SR_CU1		0x20000000	/* Coprocessor 1 usable */
#define	SR_CU0		0x10000000	/* Coprocessor 0 usable */

#define	SR_BEV		0x00400000	/* use boot exception vectors */

/* Cache control bits */
#define	SR_TS		0x00200000	/* TLB shutdown */
#define	SR_PE		0x00100000	/* cache parity error */
#define	SR_CM		0x00080000	/* cache miss */
#define	SR_PZ		0x00040000	/* cache parity zero */
#define	SR_SWC		0x00020000	/* swap cache */
#define	SR_ISC		0x00010000	/* Isolate data cache */

/*
 * Interrupt enable bits
 * (NOTE: bits set to 1 enable the corresponding level interrupt)
 */
#define	SR_IMASK	0x0000ff00	/* Interrupt mask */
#define	SR_IMASK8	0x00000000	/* mask level 8 */
#define	SR_IMASK7	0x00008000	/* mask level 7 */
#define	SR_IMASK6	0x0000c000	/* mask level 6 */
#define	SR_IMASK5	0x0000e000	/* mask level 5 */
#define	SR_IMASK4	0x0000f000	/* mask level 4 */
#define	SR_IMASK3	0x0000f800	/* mask level 3 */
#define	SR_IMASK2	0x0000fc00	/* mask level 2 */
#define	SR_IMASK1	0x0000fe00	/* mask level 1 */
#define	SR_IMASK0	0x0000ff00	/* mask level 0 */

#define	SR_IBIT8	0x00008000	/* bit level 8 */
#define	SR_IBIT7	0x00004000	/* bit level 7 */
#define	SR_IBIT6	0x00002000	/* bit level 6 */
#define	SR_IBIT5	0x00001000	/* bit level 5 */
#define	SR_IBIT4	0x00000800	/* bit level 4 */
#define	SR_IBIT3	0x00000400	/* bit level 3 */
#define	SR_IBIT2	0x00000200	/* bit level 2 */
#define	SR_IBIT1	0x00000100	/* bit level 1 */

#define	SR_KUO		0x00000020	/* old kernel/user, 0 => k, 1 => u */
#define	SR_IEO		0x00000010	/* old interrupt enable, 1 => enable */
#define	SR_KUP		0x00000008	/* prev kernel/user, 0 => k, 1 => u */
#define	SR_IEP		0x00000004	/* prev interrupt enable, 1 => enable */
#define	SR_KUC		0x00000002	/* cur kernel/user, 0 => k, 1 => u */
#define	SR_IEC		0x00000001	/* cur interrupt enable, 1 => enable */

#define	SR_IMASKSHIFT	8

/*
 * Cause Register
 */
#define	CAUSE_BD	0x80000000	/* Branch delay slot */
#define	CAUSE_BT	0x40000000	/* Branch Target */
#define	CAUSE_CEMASK	0x30000000	/* coprocessor error */
#define	CAUSE_CESHIFT	28

/* Interrupt pending bits */
#define	CAUSE_IP8	0x00008000	/* External level 8 pending */
#define	CAUSE_IP7	0x00004000	/* External level 7 pending */
#define	CAUSE_IP6	0x00002000	/* External level 6 pending */
#define	CAUSE_IP5	0x00001000	/* External level 5 pending */
#define	CAUSE_IP4	0x00000800	/* External level 4 pending */
#define	CAUSE_IP3	0x00000400	/* External level 3 pending */
#define	CAUSE_SW2	0x00000200	/* Software level 2 pending */
#define	CAUSE_SW1	0x00000100	/* Software level 1 pending */

#define	CAUSE_IPMASK	0x0000FF00	/* Pending interrupt mask */
#define	CAUSE_IPSHIFT	8

#define	CAUSE_EXCMASK	0x0000003C	/* Cause code bits */
#define	CAUSE_EXCSHIFT	2

/* Cause register exception codes */

#define	EXC_CODE(x)	((x)<<2)

/* Hardware exception codes */
#define	EXC_INT		EXC_CODE(0)	/* interrupt */
#define	EXC_MOD		EXC_CODE(1)	/* TLB mod */
#define	EXC_RMISS	EXC_CODE(2)	/* Read TLB Miss */
#define	EXC_WMISS	EXC_CODE(3)	/* Write TLB Miss */
#define	EXC_RADE	EXC_CODE(4)	/* Read Address Error */
#define	EXC_WADE	EXC_CODE(5)	/* Write Address Error */
#define	EXC_IBE		EXC_CODE(6)	/* Instruction Bus Error */
#define	EXC_DBE		EXC_CODE(7)	/* Data Bus Error */
#define	EXC_SYSCALL	EXC_CODE(8)	/* SYSCALL */
#define	EXC_BREAK	EXC_CODE(9)	/* BREAKpoint */
#define	EXC_II		EXC_CODE(10)	/* Illegal Instruction */
#define	EXC_CPU		EXC_CODE(11)	/* CoProcessor Unusable */
#define	EXC_OV		EXC_CODE(12)	/* OVerflow */

/*
 * Segment base addresses and sizes
 */
#define	K0BASE		0x80000000
#define	K0SIZE		0x20000000
#define	K1BASE		0xA0000000
#define	K1SIZE		0x20000000
#define	K2BASE		0xC0000000
#define	K2SIZE		0x20000000


#endif /* _CPUREG_H_ */
