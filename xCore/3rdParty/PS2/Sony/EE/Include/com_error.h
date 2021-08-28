/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library "DNAS" package (Release 3.0 version)
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                      <libdnas2 - com_error.h>
 *                  <communication error denifition>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      1.00            Sep,19,2001     komaki      first version
 */

#ifndef _DNAS2_COM_ERROR_H_
#define _DNAS2_COM_ERROR_H_

/* com error (-600 ... -699)
*/

#define sceDNAS2_ERR_GLUE_ABORT                      -601
#define sceDNAS2_ERR_NET_PROXY                       -602 /* Proxy error */
#define sceDNAS2_ERR_NET_TIMEOUT                     -603 /* Timeout set in sceDNAS2Init occurred */
#define sceDNAS2_ERR_NET_SSL                         -610 /* SSL error */
#define sceDNAS2_ERR_NET_DNS_HOST_NOT_FOUND          -611 /* unknown hostname */
#define sceDNAS2_ERR_NET_DNS_TRY_AGAIN               -612 /* cannot find DNS server */
#define sceDNAS2_ERR_NET_DNS_NO_RECOVERY             -613 /* invalid DNS response */
#define sceDNAS2_ERR_NET_DNS_NO_DATA                 -614 /* there is no address corresponding to the hostname */
#define sceDNAS2_ERR_NET_DNS_OTHERS                  -615
#define sceDNAS2_ERR_NET_EISCONN                     -616 /* already connected */
#define sceDNAS2_ERR_NET_ETIMEOUT                    -617 /* timeout */
#define sceDNAS2_ERR_NET_ECONNREFUSED                -618 /* connection refused */
#define sceDNAS2_ERR_NET_ENETUNREACH                 -619 /* target is unreachable */
#define sceDNAS2_ERR_NET_ENOTCONN                    -620 /* socket is not connected */
#define sceDNAS2_ERR_NET_ENOBUFS                     -621 /* not enough memory */
#define sceDNAS2_ERR_NET_EMFILE                      -622 /* there is no available socket */
#define sceDNAS2_ERR_NET_EBADF                       -623 /* bad socket descriptor */
#define sceDNAS2_ERR_NET_EINVAL                      -624 /* invalid argument */
#define sceDNAS2_ERR_NET_OTHERS                      -625
#define sceDNAS2_ERR_NET_ECONNRESET                  -626 /* connection reseted. */

#endif /* _DNAS2_COM_ERROR_H_ */
