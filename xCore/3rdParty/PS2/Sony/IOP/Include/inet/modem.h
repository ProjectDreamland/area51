/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.04
 *                           Shift-JIS
 *
 *      Copyright (C) 2000-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - modem.h
 *                           Modem I/F
 *
 * $Id: modem.h,v 1.8 2003/09/16 08:08:28 masae Exp $
 */

#if !defined(_MODEM_H)
#define _MODEM_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct sceModemOps {
	struct sceModemOps *forw, *back;	/* links for upper layer */
	char *module_name;			/* module name */
	char *vendor_name;			/* vendor name of device */
	char *device_name;			/* device name of device */
	u_char bus_type;			/* bus type */
	u_char bus_loc[31];			/* bus location */
	u_short prot_ver;			/* protocol version */
	u_short impl_ver;			/* implement version */
	void *priv;				/* private for modem layer */
	int evfid;				/* event flag ID */
	int rcv_len;				/* receive length */
	int snd_len;				/* send length */
	int (*start)(void *priv, int flags);		/* start function */
	int (*stop)(void *priv, int flags);		/* stop function */
	int (*recv)(void *priv, void *ptr, int len);	/* receive function */
	int (*send)(void *priv, void *ptr, int len);	/* send function */
	int (*control)(void *priv, int code, void *ptr, int len);
						/* control function */
	void *reserved[4];			/* reserved */
} sceModemOps_t;

/* Bus type */
#define sceModemBus_Unknown	0
#define sceModemBus_USB		1
#define sceModemBus_1394	2	/* -> reserved only */
#define sceModemBus_PCMCIA	3	/* -> reserved only */
#define sceModemBus_PSEUDO	4	/* -> reserved only */

/* prot_ver */
#define sceModemProtVer		0		/* current protocol version */
#define sceModemProtVerMask	0x00ff		/* version mask */
#define sceModemProtSkipDLL	0x0100		/* no need DLL coding */
#define sceModemProtOverEther	0x0100		/* PPP over Ethernet */
#define sceModemProtOverATM	0x0300		/* PPP over ATM */

/* Event Flags Patterns */
#define sceModemEFP_StartDone	0x00000001	/* start process completed */
#define sceModemEFP_PlugOut	0x00000002	/* plug-out (device) */
#define sceModemEFP_Connect	0x00000010	/* connect (circuit) */
#define sceModemEFP_Disconnect	0x00000020	/* disconnect (circuit) */
#define sceModemEFP_Ring	0x00000040	/* ring ring ring ... */
#define sceModemEFP_Recv	0x00000100	/* rcv_len is incremented */
#define sceModemEFP_Send	0x00000200	/* snd_len is incremented */
#define sceModemEFP_UpperUse	0xffff0000	/* used by upper layer */

int sceModemRegisterDevice(struct sceModemOps *ops);
int sceModemUnregisterDevice(struct sceModemOps *ops);

#if defined(__cplusplus)
}
#endif

/* code for control() */
/*   (I/F independent) */
#define sceModemCC_GET_THPRI	0xc0000000	/* set thread priority */
#define sceModemCC_SET_THPRI	0xc1000000	/* set thread priority */
#define sceModemCC_GET_IF_TYPE	0xc0000100	/* get I/F type */
#define   sceModemIFT_GENERIC	  0x00000000	/*   generic I/F */
#define   sceModemIFT_SERIAL	  0x00000001	/*   serial I/F */
#define sceModemCC_FLUSH_RXBUF	0xc0000110	/* flush rx buffer */
#define sceModemCC_FLUSH_TXBUF	0xc0000111	/* flush tx buffer */
#define sceModemCC_GET_DIALCONF	0xc0000200	/* get DIAL.CNF path */
#define sceModemCC_GET_RX_COUNT	0xc0010000	/* rx bytes */
#define sceModemCC_GET_TX_COUNT	0xc0010001	/* tx bytes */

/*   (Serial I/F) */
#define sceModemCC_GET_OE_COUNT	0xc0010002	/* overrun error count */
#define sceModemCC_GET_PE_COUNT	0xc0010003	/* parity error count */
#define sceModemCC_GET_FE_COUNT	0xc0010004	/* framing error count */
#define sceModemCC_GET_BO_COUNT	0xc0010005	/* buffer overflow count */
#define sceModemCC_GET_PARAM	0xc0020000	/* get serial parameters */
#define sceModemCC_SET_PARAM	0xc1020000	/* set serial parameters */
#define   sceModemPARAM_SPEED	  0x003fffff	/*   speed [bps] */
#define   sceModemPARAM_RESERVED  0x00400000	/*   (reserved) */
#define   sceModemPARAM_XON	  0x00800000	/*   XON/OFF input control */
#define   sceModemPARAM_XOFF	  0x01000000	/*   XON/OFF output control */
#define   sceModemPARAM_RTSCTS	  0x02000000	/*   enable RTS/CTS control */
#define   sceModemPARAM_STOPS	  0x0c000000	/*   stop bits */
#define   sceModemPARAM_STOP0	  0x00000000	/*     0   bits */
#define   sceModemPARAM_STOP1	  0x04000000	/*     1   bits */
#define   sceModemPARAM_STOP1H	  0x08000000	/*     1.5 bits */
#define   sceModemPARAM_STOP2	  0x0c000000	/*     2   bits */
#define   sceModemPARAM_CSIZE	  0x30000000	/*   character size */
#define   sceModemPARAM_CS5	  0x00000000	/*     5 bit/char */
#define   sceModemPARAM_CS6	  0x10000000	/*     6 bit/char */
#define   sceModemPARAM_CS7	  0x20000000	/*     7 bit/char */
#define   sceModemPARAM_CS8	  0x30000000	/*     8 bit/char */
#define   sceModemPARAM_PARODD	  0x40000000	/*   odd parity */
#define   sceModemPARAM_PARENB	  0x80000000	/*   parity enable */
#define sceModemCC_GET_LINE	0xc0030000	/* get signal pin status */
#define sceModemCC_SET_LINE	0xc1030000	/* set signal pin */ 
#define   sceModemLINE_CTS	  0x00000001	/*   CTS (in) */
#define   sceModemLINE_DSR	  0x00000002	/*   DCD (in) */
#define   sceModemLINE_RI  	  0x00000004	/*   RI  (in) */
#define   sceModemLINE_DCD	  0x00000008	/*   DCD (in) */
#define   sceModemLINE_DTR	  0x00000010	/*   DTR (out) */
#define   sceModemLINE_RTS	  0x00000020	/*   RTS (out) */
#define sceModemCC_SET_BREAK	0xc1040000	/* force break condition */

#endif	/* _MODEM_H */
