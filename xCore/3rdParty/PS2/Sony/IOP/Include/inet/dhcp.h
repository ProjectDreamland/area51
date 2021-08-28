/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.03
 *                           Shift-JIS
 *
 *      Copyright (C) 1995-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - dhcp.h
 *              Dynamic Host Configuration Protocol
 *
 * $Id: dhcp.h,v 1.6 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_DHCP_H)
#define _DHCP_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	u_char op;		/* operation code */
	u_char htype;		/* hardware address type */
	u_char hlen;		/* hardware address length */
	u_char hops;		/* gateway hops */
	u_long xid;		/* transaction id */
	u_short secs;		/* seconds since boot began */
	u_short flags;		/* (unused) */
	u_long ciaddr;		/* client IP address */
	u_long yiaddr;		/* your IP address */
	u_long siaddr;		/* server IP address */
	u_long giaddr;		/* gateway IP address */
	u_char chaddr[16];	/* client hardware address */
	char sname[64];		/* server host name */
	char file[128];		/* boot file name */
	u_char options[0];	/* options */
} DHCP_HDR;

#define DHCP_OP_REQUEST		0x01
#define DHCP_OP_REPLY		0x02

#define DHCP_FLAGS_BROADCAST	0x8000

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define dhcp_ntoh(p)
#define dhcp_hton(p)
#else

#if defined(__mips__)
#define dhcp_ntoh(p)		( \
	put4u(&(p)->xid, ntohl(get4u(&(p)->xid))), \
	(p)->secs = ntohs((p)->secs), \
	(p)->flags = ntohs((p)->flags), \
	put4u(&(p)->ciaddr, ntohl(get4u(&(p)->ciaddr))), \
	put4u(&(p)->yiaddr, ntohl(get4u(&(p)->yiaddr))), \
	put4u(&(p)->siaddr, ntohl(get4u(&(p)->siaddr))), \
	put4u(&(p)->giaddr, ntohl(get4u(&(p)->giaddr))))

#define dhcp_hton(p)		( \
	put4u(&(p)->xid, htonl(get4u(&(p)->xid))), \
	(p)->secs = htons((p)->secs), \
	(p)->flags = htons((p)->flags), \
	put4u(&(p)->ciaddr, htonl(get4u(&(p)->ciaddr))), \
	put4u(&(p)->yiaddr, htonl(get4u(&(p)->yiaddr))), \
	put4u(&(p)->siaddr, htonl(get4u(&(p)->siaddr))), \
	put4u(&(p)->giaddr, htonl(get4u(&(p)->giaddr))))
#else
#define dhcp_ntoh(p)		((p)->xid = ntohl((p)->xid), \
				 (p)->secs = ntohs((p)->secs), \
				 (p)->flags = ntohs((p)->flags), \
				 (p)->ciaddr = ntohl((p)->ciaddr), \
				 (p)->yiaddr = ntohl((p)->yiaddr), \
				 (p)->siaddr = ntohl((p)->siaddr), \
				 (p)->giaddr = ntohl((p)->giaddr))

#define dhcp_hton(p)		((p)->xid = htonl((p)->xid), \
				 (p)->secs = htons((p)->secs), \
				 (p)->flags = htons((p)->flags), \
				 (p)->ciaddr = htonl((p)->ciaddr), \
				 (p)->yiaddr = htonl((p)->yiaddr), \
				 (p)->siaddr = htonl((p)->siaddr), \
				 (p)->giaddr = htonl((p)->giaddr))
#endif

#endif

#define DHCP_MAGIC		99, 130, 83, 99

/*
 * RFC 2132 "DHCP options and BOOTP Vender Externsions"
 */

/* RFC 1497 vender externsions */
#define OPT_SUBNET_MASK		1
#define OPT_TIME_OFFSET		2
#define OPT_ROUTER		3
#define OPT_TIME_SERVER		4
#define OPT_NAME_SERVER		5
#define OPT_DNS_SERVER		6
#define OPT_LOG_SERVER		7
#define OPT_COOKIE_SERVER	8
#define OPT_LPR_SERVER		9
#define OPT_IMPRESS_SERVER	10
#define OPT_RL_SERVER		11
#define OPT_HOST_NAME		12
#define OPT_BOOT_FILE_SIZE	13
#define OPT_MERIT_DUMP		14
#define OPT_DOMAIN_NAME		15
#define OPT_SWAP_SERVER		16
#define OPT_ROOT_PATH		17
#define OPT_EXTENTIONS_PATH	18
/* IP layer parameters */
#define OPT_IP_FORWARDING	19
#define OPT_NON_LOCAL_SOURCE	20
#define OPT_POLICY_FILTER	21
#define OPT_MAX_DG_REASSEMBLY	22
#define OPT_DEFAULT_TTL		23
#define OPT_PATH_MTU_TIMEOUT	24
#define OPT_PATH_MTU_PLATEAU	25
#define OPT_INTERFACE_MTU	26
#define OPT_ALL_SUBNETS_LOCAL	27
#define OPT_BROADCAST_ADDRESS	28
#define OPT_MASK_DISCOVERY	29
#define OPT_MASK_SUPPLIER	30
#define OPT_ROUTER_DISCOVERY	31
#define OPT_ROUTER_SOLICITATION	32
#define OPT_STATIC_ROUTE	33
/* Link layer parameters */
#define OPT_TRAILER_ENCAPS	34
#define OPT_ARP_CACHE_TIMEOUT	35
#define OPT_ETHER_ENCAPS	36
/* TCP parameters */
#define OPT_TCP_DEFAULT_TTL	37
#define OPT_TCP_KEEPALIVE	38
#define OPT_TCP_KA_GARBAGE	39
/* Application and service parameters */
#define OPT_NIS_DOMAIN		40
#define OPT_NIS_SERVER		41
#define OPT_NTP_SERVER		42
#define OPT_VENDER_SPECIFIC	43
#define OPT_NETBIOS_NS		44
#define OPT_NETBIOS_DD		45
#define OPT_NETBIOS_NODE_TYPE	46
#define OPT_NETBIOS_SCOPE	47
#define OPT_X_FONT_SERVER	48
#define OPT_X_DISPLAY_MANAGER	49
#define OPT_NISPLUS_DOMAIN	64
#define OPT_NISPLUS_SERVER	65
#define OPT_NISPLUS_DOMAIN	64
#define OPT_MOBILE_IP_HA	68
#define OPT_SMTP_SERVER		69
#define OPT_POP3_SERVER		70
#define OPT_NNTP_SERVER		71
#define OPT_WWW_SERVER		72
#define OPT_FINGER_SERVER	73
#define OPT_IRC_SERVER		74
#define OPT_STREETTALK_SERVER	75
#define OPT_STDA_SERVER		76
/* DHCP externsions */
#define OPT_REQUESTED_IPADR	50
#define OPT_IPADR_LEASETIME	51
#define OPT_OVERLOAD		52
#define OPT_MESSAGE_TYPE	53
#define OPT_SERVER_ID		54
#define OPT_PARAM_REQUEST_LIST	55
#define OPT_MESSAGE		56
#define OPT_MAX_MESSAGE_SIZE	57
#define OPT_T1_VALUE		58
#define OPT_T2_VALUE		59
#define OPT_VENDER_CLASS_ID	60
#define OPT_CLIENT_ID		61

#define OPT_TFTP_SERVER_NAME	66
#define OPT_BOOTFILE_NAME	67

/* Message types.  */
#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNACK		6
#define DHCPRELEASE		7
#define DHCPINFORM		8

#if defined(__cplusplus)
}
#endif

#endif	/* _DHCP_H */
