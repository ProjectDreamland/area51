/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <eenetctl - eenetctl.h>
 *             <header for handling network configration>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Nov,15,2001     komaki      first version
 */

#ifndef _EE_EENETCTL_H_
#define _EE_EENETCTL_H_

#ifndef ADDR_MAX
#define ADDR_MAX 256
#endif
#define NAME_MAX 256
#define IFNAME_MAX 16

/* eenetctl device type */
#define EENETCTL_IFTYPE_ANY 0
#define EENETCTL_IFTYPE_ETH 1
#define EENETCTL_IFTYPE_MODEM 2
#define EENETCTL_IFTYPE_NIC 3

/* type for event handler of sceEENetCtlRegisterEventHandler() */
#define sceEENETCTL_IEV_Attach   1   /* I/F attach */
#define sceEENETCTL_IEV_Detach   2   /* I/F detaching happened */
#define sceEENETCTL_IEV_Start    3   /* I/F start (Get IP addr)(IPv4) */
#define sceEENETCTL_IEV_Stop     4   /* I/F stop (lose IP addr)(IPv4) */
#define sceEENETCTL_IEV_Error    5   /* I/F error (Error=1) */
#define sceEENETCTL_IEV_Conf     6   /* Configuration found */
#define sceEENETCTL_IEV_NoConf   7   /* Configuration not found */
#define sceEENETCTL_IEV_GetAddr  8   /* get IP addr(IPv6) */
#define sceEENETCTL_IEV_LoseAddr 9  /* lose IP addr(IPv6) */
#define sceEENETCTL_IEV_UpCompleted 10 /* user up request for "ipversion"
                                         completed. */
#define sceEENETCTL_IEV_DownCompleted 11 /* user down request for "ipversion"
                                         completed. */
#define sceEENETCTL_IEV_DetachCompleted 12 /* detach of interface completed. */

/* state code for sceEENetCtlGetState() */
#define sceEENETCTL_S_DETACHED   0   /* Detached */
#define sceEENETCTL_S_STARTING   1   /* Starting */
#define sceEENETCTL_S_RETRYING   2   /* Retrying */
#define sceEENETCTL_S_STARTED    3   /* Started */
#define sceEENETCTL_S_STOPPING   4   /* Stopping */
#define sceEENETCTL_S_STOPPED    5   /* Stopped */
#define sceEENETCTL_S_DETACHING  6   /* Detaching */

#ifdef __cplusplus
extern "C" {
#endif

/* erx export */
void *sceEENetCtlGetErxEntries(void);

int sceEENetCtlInit(int rpcs_stack, int rpcs_prio, int eenetctl_stack,
	int eenetctl_prio, int if_stack, int if_prio, int verbose, int no_auto);
int sceEENetCtlUpInterface(const char *ifname);
int sceEENetCtlDownInterface(const char *ifname);
int sceEENetCtlRegisterEventHandler(void (*func)(const char *ifname, int af, int type));
int sceEENetCtlUnregisterEventHandler(void (*func)(const char *ifname, int af, int type));
int sceEENetCtlGetState(const char *ifname, int af, int *pstate);
int sceEENetCtlGetErrorCode(const char *ifname, int af, int *pcode, int *psubcode);
int sceEENetCtlGetIfStat(const char *ifname, int af, void *pstat, int len);

int sceEENetCtlSetAutoMode(int f_on);
int sceEENetCtlTerm(void);

#ifdef __cplusplus
}
#endif

#endif /* _EE_EENETCTL_H_ */
