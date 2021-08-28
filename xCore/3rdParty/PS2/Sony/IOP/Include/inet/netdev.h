/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.05
 *                           Shift-JIS
 *
 *      Copyright (C) 2000-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - netdev.h
 *                           NETDEV I/F
 *
 * $Id: netdev.h,v 1.12 2003/10/07 02:20:45 ksh Exp $
 */

#if !defined(_NETDEV_H)
#define _NETDEV_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(__LIBNET__)
typedef struct sceInetPkt {
	struct sceInetPkt *forw, *back;
	int reserved[2];
	u_char *rp, *wp;
} sceInetPkt_t;

typedef struct sceInetPktQ {
	struct sceInetPkt *head, *tail;
} sceInetPktQ_t;

typedef struct sceInetDevOps {
	struct sceInetDevOps *forw, *back;	/* links for INET layer */
	char interface[8 + 1];			/* interface name */
	char *module_name;			/* module name */
	char *vendor_name;			/* vendor name of device */
	char *device_name;			/* device name of device */
	u_char bus_type;			/* bus type */
	u_char bus_loc[31];			/* bus location */
	u_short prot_ver;			/* protocol version */
	u_short impl_ver;			/* implement version */
	void *priv;				/* private for NETDEV layer */
	int flags;				/* various flags */
	int evfid;				/* event flag ID */
	struct sceInetPktQ rcvq;		/* receive queue */
	struct sceInetPktQ sndq;		/* send queue */
	int (*start)(void *priv, int flags);	/* start function */
	int (*stop)(void *priv, int flags);	/* stop function */
	int (*xmit)(void *priv, int flags);	/* transmit function */
	int (*control)(void *priv, int code, void *ptr, int len);
						/* control function */
	u_long ip_addr;				/* IP address */
	u_long ip_mask;				/* IP subnet mask */
	u_long broad_addr;			/* IP broadcast address */
	u_long gw_addr;				/* gateway address */
	u_long ns_addr1;			/* primary DNS address */
	int mtu;				/* MTU */
	u_char hw_addr[16];			/* hardware address */
	u_char dhcp_hostname[256];		/* host name for DHCP */
	int dhcp_hostname_len;			/* length of host name */
	int dhcp_flags;				/* flags for DHCP */
	void *reserved[4];			/* reserved */
	u_long ns_addr2;			/* secondary DNS address */
	void *pppoe_priv;			/* private for PPPoE */
} sceInetDevOps_t;
#endif	/* __LIBNET__ */

/* Bus type */
#define sceInetBus_Unknown	0
#define sceInetBus_USB		1
#define sceInetBus_1394		2	/* -> reserved only */
#define sceInetBus_PCMCIA	3	/* -> reserved only */
#define sceInetBus_PSEUDO	4
#define sceInetBus_NIC		5

/* prot_ver */
#define sceInetDevProtVer	2	/* current protocol version */
	/* 0->1: add dhcp_hostname,dhcp_hostname_len */
	/* 1->2: rename ns_addr to ns_addr1, add ns_addr2, dhcp_flags,
		 pppoe_priv */

/* flags */
#define sceInetDevF_Up		0x0001	/* interface is up */
#define sceInetDevF_Running	0x0002	/* interface is running */
#define sceInetDevF_Broadcast	0x0004	/* broadcast address is valid */
#define sceInetDevF_ARP		0x0010	/* ARP */
#define sceInetDevF_DHCP	0x0020	/* DHCP */
#define sceInetDevF_PPP		0x0040	/* PPP */
#define sceInetDevF_NIC		0x0080	/* NIC */
#define sceInetDevF_Error	0x0100	/* Error */
#define sceInetDevF_PPPoE	0x0200	/* PPP over Ether */
#define sceInetDevF_Multicast	0x0400	/* supports multicast */

/* Event Flags Patterns */
#define sceInetDevEFP_StartDone	0x00000001	/* start process complete */
#define sceInetDevEFP_PlugOut	0x00000002	/* device is plug-out */
#define sceInetDevEFP_Recv	0x00000004	/* packet received */
#define sceInetDevEFP_Error	0x00000010	/* error */
#define sceInetDevEFP_TimeOut	0x00000020	/* timeout error */
#define sceInetDevEFP_InetUse	0xffff0000	/* used by INET layer */

/* DHCP flags (dhcp_flags) */
#define sceInetDevDHCP_RelOnStop	0x00000001	/* release on stop */

#if !defined(__LIBNET__)
/* for NETDEV layer */
int sceInetRegisterNetDevice(sceInetDevOps_t *ops);
int sceInetUnregisterNetDevice(sceInetDevOps_t *ops);
void *sceInetAllocMem(sceInetDevOps_t *ops, int siz);
void sceInetFreeMem(sceInetDevOps_t *ops, void *ptr);
sceInetPkt_t *sceInetAllocPkt(sceInetDevOps_t *ops, int siz);
void sceInetFreePkt(sceInetDevOps_t *ops, sceInetPkt_t *pkt);
void sceInetPktEnQ(sceInetPktQ_t *que, sceInetPkt_t *pkt);
sceInetPkt_t *sceInetPktDeQ(sceInetPktQ_t *que);
u_int sceInetRand(void);
int sceInetPrintf(const char *fmt, ...);
#endif	/* __LIBNET__ */

#if defined(__cplusplus)
}
#endif

/* code for control() */
/*   (I/F independent) */
#define sceInetNDCC_GET_THPRI		0x80000000	/* get thread prio. */
#define sceInetNDCC_SET_THPRI		0x81000000	/* set thread prio. */
#define sceInetNDCC_GET_IF_TYPE		0x80000100	/* get I/F type */
#define   sceInetNDIFT_GENERIC		  0x00000000	/*   generic */
#define   sceInetNDIFT_ETHERNET		  0x00000001	/*   Ethernet */
#define   sceInetNDIFT_PPP		  0x00000002	/*   PPP */
#define sceInetNDCC_GET_RX_PACKETS	0x80010000	/* rx packets */
#define sceInetNDCC_GET_TX_PACKETS	0x80010001	/* tx packets */
#define sceInetNDCC_GET_RX_BYTES	0x80010002	/* rx bytes */
#define sceInetNDCC_GET_TX_BYTES	0x80010003	/* tx bytes */
#define sceInetNDCC_GET_RX_ERRORS	0x80010004	/* rx errors */
#define sceInetNDCC_GET_TX_ERRORS	0x80010005	/* tx errors */
#define sceInetNDCC_GET_RX_DROPPED	0x80010006	/* rx dropped */
#define sceInetNDCC_GET_TX_DROPPED	0x80010007	/* tx dropped */
#define sceInetNDCC_GET_RX_BROADCAST_PACKETS	0x80010008
#define sceInetNDCC_GET_TX_BROADCAST_PACKETS	0x80010009
#define sceInetNDCC_GET_RX_BROADCAST_BYTES	0x8001000a
#define sceInetNDCC_GET_TX_BROADCAST_BYTES	0x8001000b
#define sceInetNDCC_GET_RX_MULTICAST_PACKETS	0x8001000c
#define sceInetNDCC_GET_TX_MULTICAST_PACKETS	0x8001000d
#define sceInetNDCC_GET_RX_MULTICAST_BYTES	0x8001000e
#define sceInetNDCC_GET_TX_MULTICAST_BYTES	0x8001000f

/*   (Ethernet I/F) */
#define sceInetNDCC_GET_MULTICAST	0x80011000  /* multicast */
#define sceInetNDCC_GET_COLLISIONS	0x80011001  /* collisions */
#define sceInetNDCC_GET_RX_LENGTH_ER	0x80011002  /* rx length errors */
#define sceInetNDCC_GET_RX_OVER_ER	0x80011003  /* rx over errors */
#define sceInetNDCC_GET_RX_CRC_ER	0x80011004  /* rx crc errors */
#define sceInetNDCC_GET_RX_FRAME_ER	0x80011005  /* rx frame errors */
#define sceInetNDCC_GET_RX_FIFO_ER	0x80011006  /* rx fifo errors */
#define sceInetNDCC_GET_RX_MISSED_ER	0x80011007  /* rx missed errors */
#define sceInetNDCC_GET_TX_ABORTED_ER	0x80011008  /* tx aborted errors */
#define sceInetNDCC_GET_TX_CARRIER_ER	0x80011009  /* tx carrier errors */
#define sceInetNDCC_GET_TX_FIFO_ER	0x8001100a  /* tx fifo errors */
#define sceInetNDCC_GET_TX_HEARTBEAT_ER	0x8001100b  /* tx heartbeat errors */
#define sceInetNDCC_GET_TX_WINDOW_ER	0x8001100c  /* tx window errors */
#define sceInetNDCC_GET_NEGO_MODE	0x80020000  /* get nego mode */
#define sceInetNDCC_SET_NEGO_MODE	0x81020000  /* set nego mode */
#define sceInetNDCC_GET_NEGO_STATUS	0x80020001  /* get nego status */
#define sceInetNDCC_GET_LINK_STATUS	0x80030000  /* link status */
#define sceInetNDCC_SET_MULTICAST_LIST	0x81040000  /* set multicast list */

/* Nego mode for Ethernet I/F */
#define sceInetNDNEGO_10	0x0001	/* 10BaseT Half-Duplex */
#define sceInetNDNEGO_10_FD	0x0002	/* 10BaseT Full-Duplex */
#define sceInetNDNEGO_TX	0x0004	/* 100BaseTX Half-Duplex */
#define sceInetNDNEGO_TX_FD	0x0008	/* 100BaseTX Full-Duplex */
#define sceInetNDNEGO_PAUSE	0x0040	/* Enable Flow Control */
#define sceInetNDNEGO_AUTO	0x0080	/* Enable Auto Nego */

#endif	/* _NETDEV_H */
