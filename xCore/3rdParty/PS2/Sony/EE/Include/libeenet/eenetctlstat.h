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
 *                     <eenetctl - eenetctlstat.h>
 *              <header for status about interface protocol>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Jul,24,2002     komaki      first version
 */

#ifndef _EENETCTLSTAT_H_
#define _EENETCTLSTAT_H_

#include <netinet/in.h>

/* DHCP */
#define sceEENETCTL_P_DhcpInit		201	/* INIT state */
#define sceEENETCTL_P_DhcpSelecting	202	/* SELECTING state */
#define sceEENETCTL_P_DhcpRequesting	203	/* REQUESTING state */
#define sceEENETCTL_P_DhcpBound		204	/* BOUND state */
#define sceEENETCTL_P_DhcpRenewing	205	/* RENEWING state */
#define sceEENETCTL_P_DhcpRebinding	206	/* REBINDING state */

typedef struct sceEENetCtlStatDhcp {
	int phase;
	int lease_duration;
	struct in_addr server_addr;
} sceEENetCtlStatDhcp_t;


/* PPP & PPPoE */
#define sceEENETCTL_P_PppDead		301	/* Phase dead */
#define sceEENETCTL_P_PppEstablish	302	/* Phase link establishment */
#define sceEENETCTL_P_PppTerminate	303	/* Phase terminate */
#define sceEENETCTL_P_PppAuth		304	/* Phase authentication */
#define sceEENETCTL_P_PppNetwork		305	/* Phase netowrk */

#define sceEENETCTL_PP_ModemDisconnect	401	/* Line is disconnected */
#define sceEENETCTL_PP_ModemConnecting	402	/* Line is now connecting */
#define sceEENETCTL_PP_ModemConnect	403	/* Line is connected */
#define sceEENETCTL_PP_ModemDisconnecting 404	/* Line is now disconnecting */
#define sceEENETCTL_PP_ModemStarting	405	/* Modem initializing */

#define sceEENETCTL_PP_PppoeInit		501	/* Session is not opened */
#define sceEENETCTL_PP_PppoePADISent	502	/* PADI sent, wait PADO */
#define sceEENETCTL_PP_PppoePADRSent	503	/* PADR sent, wait PADS */
#define sceEENETCTL_PP_PppoeSession	504	/* Session is up */
#define sceEENETCTL_PP_PppoeClosing	505	/* Sessing is now closing */

typedef struct sceEENetCtlStatPpp {
	int ppp_phase;
	int modem_phase;
	int redial_count;
	char phone_number[256];
	char status_string[80];
	char last_string[32];
	char auth_message[128];
} sceEENetCtlStatPpp_t;

typedef struct sceEENetCtlStatPppoe {
	int ppp_phase;
	int pppoe_phase;
	char auth_message[128];
} sceEENetCtlStatPppoe_t;


/* Stateless Address Autoconfiguration */
#define sceEENETCTL_P_AutoconfIdle	601	/* Idle */
#define sceEENETCTL_P_AutoconfDelay	602	/* Delay for sending RS */
#define sceEENETCTL_P_AutoconfProbe	603	/* RS sent, wait for RA */
#define sceEENETCTL_P_AutoconfGetRA	604	/* Get valid RA */
#define sceEENETCTL_P_AutoconfTentative	605	/* DAD is now performing */
#define sceEENETCTL_P_AutoconfDuplicate	606	/* Duplicate address was detected, still continue DAD */
#define sceEENETCTL_P_AutoconfDown	607	/* Interface is invalid */

typedef struct sceEENetCtlStatAutoconf {
	int phase;
	struct in6_addr router_addr;
} sceEENetCtlStatAutoconf_t;

#endif /* _EENETCTLSTAT_H_ */
