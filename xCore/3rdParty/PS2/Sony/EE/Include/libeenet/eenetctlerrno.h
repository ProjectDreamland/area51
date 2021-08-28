/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <eenetctl - eenetctlerrno.h>
 *                   <header for eenetctl error code>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Jul,24,2002     komaki      first version
 */

#ifndef _EENETCTLERRNO_H_
#define _EENETCTLERRNO_H_

/* General */
#define sceEENETCTL_E_NoMem		-1	/* No more memory */
#define sceEENETCTL_E_Sema		-2	/* Semaphore error */
#define sceEENETCTL_E_Thread		-3	/* Thread error */
#define sceEENETCTL_E_Socket		-4	/* Socket operation error */
#define sceEENETCTL_E_Bpf		-5	/* BPF operation error */
#define sceEENETCTL_E_Ifconfig		-6	/* Interface configuration error */
#define sceEENETCTL_E_DnsRoute		-7	/* DNS & Route configuration error */
#define sceEENETCTL_E_GetStatus		-8	/* Can't get status */
#define sceEENETCTL_E_Local		-9	/* Local operation error */
#define sceEENETCTL_E_Fatal		-10	/* Fatal system error */

#define sceEENETCTL_E_Busy			-23 /* Busy */
#define sceEENETCTL_E_DevBusy		-24 /* Device Busy */
#define sceEENETCTL_E_Exist			-25 /* Already Exist */
#define sceEENETCTL_E_NoEnt			-28 /* there is no entry */
#define sceEENETCTL_E_Invalid		-29	/* invalid argument */
#define sceEENETCTL_E_INTR			-30	/* interrupted */

/* DHCP */
#define sceEENETCTL_E_DhcpNoServer	-201	/* No DHCP server is available */
#define sceEENETCTL_E_DhcpNoAckNak	-202	/* Get neither DHCPACK nor DHCPNAK */
#define sceEENETCTL_E_DhcpGetNak		-203	/* Get DHCPNACK in RENEWING or REBINDING state */
#define sceEENETCTL_E_DhcpSentDecline	-204	/* Sent DHCPDECLINE */
#define sceEENETCTL_E_DhcpLeaseTime	-205	/* Lease time was expired */
#define sceEENETCTL_E_DhcpMalformedPkt	-206	/* Get malformed packet */

/* PPP & PPPoE */
#define sceEENETCTL_E_PppLinkEstab	-301	/* Link control protcol failure */
#define sceEENETCTL_E_PppAuth		-302	/* Authentication failure */
#define sceEENETCTL_E_PppNetwork		-303	/* Network control protocol failure */
#define	sceEENETCTL_E_PppTerminate	-304	/* Link terminatin was received */

#define sceEENETCTL_EE_ModemCannotConnect -401	/* Get NO ANSWER or NO DIAL TONE */
#define sceEENETCTL_EE_ModemHangUp	-402	/* Get NO CARRIER */
#define sceEENETCTL_EE_ModemInit		-403	/* Initialize error */
#define sceEENETCTL_EE_ModemBusy	-404	/* Get BUSY */
#define sceEENETCTL_EE_ModemUnknown	-499	/* unknown error */

#define sceEENETCTL_EE_PppoeServiceName	-501	/* Service name error */
#define sceEENETCTL_EE_PppoeACSystem	-502	/* AC system error */
#define sceEENETCTL_EE_PppoeGeneric	-503	/* Generic error */
#define sceEENETCTL_EE_PppoeNoPADO	-504	/* Get no PADO */
#define sceEENETCTL_EE_PppoeNoPADS	-505	/* Get No PADS */
#define sceEENETCTL_EE_PppoeGetPADT	-506	/* PADT was received */
#define sceEENETCTL_EE_PppoeInit		-507	/* Initialize error */
#define sceEENETCTL_EE_PppoeUnknown	-599	/* unknown error */

/* Stateless Address Autoconfiguration */
#define sceEENETCTL_E_AutoconfNoRA	-601	/* No Router Advertisement */
#define sceEENETCTL_E_AutoconfDAD	-602	/* DAD failed */

/* Other */
#define sceEENETCTL_EE_NotAvail	0	/* sub error code is invalid */

#endif /* _EENETCTLERRNO_H_ */
