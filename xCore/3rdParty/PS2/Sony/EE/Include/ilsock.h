/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                         i.LINK Library
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                             ilsock.h
 *                     header file of socket for i.LINK
 *
 *	Version		Date		Design	Log
 *  --------------------------------------------------------------------
 *	0.9		1999/12/27      hig	v1.4
 *	0.92		2000/03/30	hig	v1.6
 *	0.93		2000/05/10      sim	Add sceGetCycleTimeV
 *	0.94		2000/05/12      sim	Rename sceILsock...
 */

#ifndef _SCEILSOCK_H_
#define _SCEILSOCK_H_

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#ifndef SCEILSOCK_AF_UNSPEC
#define SCEILSOCK_AF_UNSPEC	0
#endif /* !SCEILSOCK_AF_UNSPEC */
#ifndef SCEILSOCK_DGRAM
#define SCEILSOCK_DGRAM	2
#endif /* !SCEILSOCK_DGRAM */

#ifndef SCEILSOCK_MSG_PEEK
#define SCEILSOCK_MSG_PEEK	0x2
#endif /* !SCEILSOCK_MSG_PEEK */

#define SCEILSOCK_AF	60
#define SCEILSOCK_PF	SCEILSOCK_AF

#define SCEILSOCK_ADDR_ANY_HI		0
#define SCEILSOCK_ADDR_ANY_LO		0
#define SCEILSOCK_ADDR_BROADCAST_HI	0xffffffff
#define SCEILSOCK_ADDR_BROADCAST_LO	0xffffffff

#define SCEILSOCK_PORT_ANONMIN		49152
#define SCEILSOCK_PORT_ANONMAX		65535


#ifdef SCEILSOCK_NO_PACKET_SIZE_EXTENSION
#define SCEILSOCK_MAX_PACKET_SIZE	64
#else /* SCEILSOCK_NO_PACKET_SIZE_EXTENSION */
#define SCEILSOCK_MAX_PACKET_SIZE	512
#endif /* SCEILSOCK_NO_PACKET_SIZE_EXTENSION */
#define SCEILSOCK_MAX_PAYLOAD_SIZE	(SCEILSOCK_MAX_PACKET_SIZE - 24)
						/* 20: sizeof(dp_header) */
#define SCEILSOCK_MAX_PAYLOAD_SIZE_DEFAULT	(64 - 24)


typedef unsigned int	sceILsock_addr_t;
typedef unsigned short	sceILsock_port_t;

typedef struct sceILsock_addr {
	unsigned char			sock_len;
	unsigned char			sock_family;
	sceILsock_port_t		sock_port;
	struct eui64 {
		sceILsock_addr_t	eui64_hi;
		sceILsock_addr_t	eui64_lo;
	} sock_addr;
	char				sock_zero[4]; 
} sceILsock_addr;

enum SceILsockErrorCode {
	SCEILSOCKERR_OK				=  0,
	SCEILSOCKERR_ERROR			= -1,
	SCEILSOCKERR_NOT_INITIALIZED		= -2000,
	SCEILSOCKERR_NOT_SUPPORTED		= -2001,
	SCEILSOCKERR_NO_MEMORY			= -2002,
	SCEILSOCKERR_RESOURCE_UNAVAILABLE	= -2003,
	SCEILSOCKERR_INVALID_ID			= -2004,
	SCEILSOCKERR_INVALID_REQUEST		= -2005,
	SCEILSOCKERR_INVALID_ARGUMENT		= -2006,
	SCEILSOCKERR_INVALID_SIZE		= -2007,
	SCEILSOCKERR_INVALID_ADDRESS		= -2008,
	SCEILSOCKERR_NO_SUCH_NODE		= -2021,
};


extern enum SceILsockErrorCode	sceILsockInit(int maxsock, int maxsize);

extern void	sceILsockReset(void);

extern int	sceILsockOpen(int domain, int type, int protocol);

extern enum SceILsockErrorCode	sceILsockClose(int sock);

extern enum SceILsockErrorCode	sceILsockBind(int sock,
				struct sceILsock_addr *name, int namelen);

extern enum SceILsockErrorCode	sceILsockConnect(int sock,
				struct sceILsock_addr *name, int namelen);

extern int	sceILsockSend(int sock, char *buf, int len, int flags);
extern int	sceILsockSendTo(int sock, char *buf, int len, int flags,
				struct sceILsock_addr *to, int tolen);

extern int	sceILsockRecv(int sock, char *buf, int len, int flags);
extern int	sceILsockRecvFrom(int sock, char *buf, int len, int flags,
				struct sceILsock_addr *from, int *fromlen);

#ifndef sceILsockHtoNl
extern sceILsock_addr_t	sceILsockHtoNl(sceILsock_addr_t host32);
#ifdef __MIPSEL__
#define sceILsockHtoNl(host32)	\
	((((host32) >> 8) & 0xff00) | (((host32) & 0xff00) << 8) | \
	((host32) << 24) | ((sceILsock_addr_t)(host32) >> 24))
#endif /* __MIPSEL__ */
#ifdef __MIPSEB__
#define sceILsockHtoNl(host32)	(host32)
#endif /* __MIPSEB__ */
#endif /* !sceILsockHtoNl */

#ifndef sceILsockNtoHl
extern sceILsock_addr_t	sceILsockNtoHl(sceILsock_addr_t net32);
#define sceILsockNtoHl(net32)	sceILsockHtoNl(net32)
#endif /* !sceILsockNtoHl */

#ifndef sceILsockHtoNs
extern sceILsock_port_t	sceILsockHtoNs(sceILsock_port_t host16);
#ifdef __MIPSEL__
#define sceILsockHtoNs(host16)	\
	((((host16 >> 8) & 0xff) | (host16 << 8)) & 0xffff)
#endif /* __MIPSEL__ */
#ifdef __MIPSEB__
#define sceILsockHtoNs(host16)	(host16)
#endif /* __MIPSEB__ */
#endif /* !sceILsockHtoNs */

#ifndef sceILsockNtoHs
extern sceILsock_port_t	sceILsockNtoHs(sceILsock_port_t net16);
#define sceILsockNtoHs(net16)	sceILsockHtoNs(net16)
#endif /* ! sceILsockNtoHs */

extern void sce1394GetCycleTimeV(void* vars);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !_SCEILSOCK_H_ */
