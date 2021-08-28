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
 *                        inet - dns.h
 *                     Domain Name Service
 *
 * $Id: dns.h,v 1.6 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_DNS_H)
#define _DNS_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	u_short id;		/* identifier */
#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
	u_char qr	: 1;	/* reponse/~query */
	u_char opcode	: 4;	/* kind of query */
	u_char aa	: 1;	/* authoritive answer */
	u_char tc	: 1;	/* trancation */
	u_char rd	: 1;	/* recursion desired */
	u_char ra	: 1;	/* recursion available */
	u_char z	: 3;	/* reserved for future use */
	u_char rcode	: 4;	/* response code */
#else	/* BIG_ENDIAN */
	u_char rd	: 1;	/* recursion desired */
	u_char tc	: 1;	/* trancation */
	u_char aa	: 1;	/* authoritive answer */
	u_char opcode	: 4;	/* kind of query */
	u_char qr	: 1;	/* reponse/~query */
	u_char rcode	: 4;	/* response code */
	u_char z	: 3;	/* reserved for future use */
	u_char ra	: 1;	/* recursion available */
#endif	/* BIG_ENDIAN */
	u_short qdcount;	/* # of entries in the question section */
	u_short ancount;	/* # of RR in the answer section */
	u_short nscount;	/* # of RR in the authority record section */
	u_short arcount;	/* # of RR in the addtional record section */
} DNS_HDR;

/* kind of query */
#define DNS_OPCODE_QUERY	0	/* a standard query */
#define DNS_OPCODE_IQUERY	1	/* an inverse query */
#define DNS_OPCODE_STATUS	2	/* a server status reqeust */

/* response codes */
#define DNS_RCODE_NOERR		0	/* no error condition */
#define DNS_RCODE_FORMAT_ERR	1	/* format error */
#define DNS_RCODE_SERVER_FAIL	2	/* server failure */
#define DNS_RCODE_NAME_ERR	3	/* name error */
#define DNS_RCODE_NOT_IMPL	4	/* not implemented */
#define DNS_RCODE_REFUSED	5	/* refused */

/* RR TYPE */
#define RR_TYPE_A	1	/* a host address */
#define RR_TYPE_NS	2	/* an authoritative name server */
#define RR_TYPE_MD	3	/* a mail destination (Obsolete - use MX) */
#define RR_TYPE_MF	4	/* a mail forwader (Obsolete - use MX) */
#define RR_TYPE_CNAME	5	/* the canonical name for an alias */
#define RR_TYPE_SOA	6	/* marks the start of a zone of authority */
#define RR_TYPE_MB	7	/* a mailbox domain name (EXPERIMENTAL) */
#define RR_TYPE_MG	8	/* a mailbox group name (EXPERIMENTAL) */
#define RR_TYPE_MR	9	/* a mail rename domain name (EXPERIMENTAL) */
#define RR_TYPE_NULL	10	/* a null RR (EXPERIMENTAL) */
#define RR_TYPE_WRK	11	/* a well known service domain */
#define RR_TYPE_PTR	12	/* a domain name pointer */
#define RR_TYPE_HINFO	13	/* host information */
#define RR_TYPE_MINFO	14	/* mailbox or mail list information */
#define RR_TYPE_MX	15	/* mail exchange */
#define RR_TYPE_TXT	16	/* text strings */
/* addional for QTYPE */
#define RR_TYPE_AXFR	252	/* a request for a transfer of an entire zone */
#define RR_TYPE_MAILB	253	/* a request for mailbox-related records */
#define RR_TYPE_MAILA	254	/* a request for mail agent RRs (Obsolete) */
#define RR_TYPE_ANY	255	/* a request for all records */

/* RR CLASS */
#define RR_CLASS_IN	1	/* the Internet */
#define RR_CLASS_CS	2	/* the CSNET class (Obsolete) */
#define RR_CLASS_CH	3	/* the CHAOS class */
#define RR_CLASS_HS	4	/* Hesiod */
/* addional for QCLASS */
#define RR_CLASS_ANY	255	/* any class */

#define PACKET_SZ	512
#define MAX_DOMAIN_SZ	256
#define MAX_LABEL_SZ	63

#define DNS_INDIR_MSK	0xc0
#define DNS_INDIR	0xc0

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define dns_ntoh(p)
#define dns_hton(p)
#else

#define dns_ntoh(p)	((p)->id = ntohs((p)->id), \
			 (p)->qdcount = ntohs((p)->qdcount), \
			 (p)->ancount = ntohs((p)->ancount), \
			 (p)->nscount = ntohs((p)->nscount), \
			 (p)->arcount = ntohs((p)->arcount))

#define dns_hton(p)	((p)->id = htons((p)->id), \
			 (p)->qdcount = htons((p)->qdcount), \
			 (p)->ancount = htons((p)->ancount), \
			 (p)->nscount = htons((p)->nscount), \
			 (p)->arcount = htons((p)->arcount))
#endif

#define RES_TIMEOUT	(6 * 1000)	/* 6 sec */
#define RES_RETRY	4

#if defined(__cplusplus)
}
#endif

#endif	/* _DNS_H */
