/* SCE CONFIDENTIAL
"PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*	Copyright (C) 1999-2001 Sony Computer Entertainment Inc.
 *			All Right Reserved
 *
 * usbd.h - USBD (USB Driver) library
 *
 * $Id: usbd.h,v 1.11.40.1 2003/10/08 10:38:53 masae Exp $
 */

#if !defined(_LIBUSBD_H)
#define _LIBUSBD_H

/* sceUsbd_MULTI_ISOCH_TRANSFERS enables isochronous transfers with 
 * multiple packets per TD -> performance enhancement
 */
#define sceUsbd_MULTI_ISOCH_TRANSFERS

#ifdef sceUsbd_MULTI_ISOCH_TRANSFERS

/* OHCI hardware can handle a maximum of 8 packets together
 * in one transfer - OHCI spec Table 4-3, field FC 
 */
#define sceUsbd_MAX_ISOCH_PACKETS 8

/* The length of the packet, and the status after transfer 
 * OHCI spec 4.3.2.4
 * used for:
 * - submission of multiple packet transfers
 * - completion code reporting of multiple packet transfers
 */
typedef struct _sceUsbdIsochronousPswLen {
	u_short len:11;
	u_short reserved:1;
	u_short PSW:4;
} sceUsbdIsochronousPswLen;

/* The description of a multiple packet isochronous transfer */

typedef struct _sceUsbdMultiIsochronousRequest {
	void *buffer_base;	   /* in: base address of buffer */
	int relative_start_frame;  /* in: relative starting frame number */
	int num_packets;           /* in: number of packets described in Packets[] */
	/* in/out: on input, this describes the individual packets' lengths,
	 *          on output, the status and length (for IN)
	 */
	sceUsbdIsochronousPswLen Packets[sceUsbd_MAX_ISOCH_PACKETS];
} sceUsbdMultiIsochronousRequest;
#endif

/* Logical Device Driver Operations */

typedef struct _sceUsbdLddOps {
	struct _sceUsbdLddOps *forw, *back;
	char *name;
	int (*probe)(int dev_id);
	int (*attach)(int dev_id);
	int (*detach)(int dev_id);
	void *reserved[5];
	u_int gp;
} sceUsbdLddOps;

typedef	void (*sceUsbdDoneCallback)(int result, int count, void *arg);

#ifdef sceUsbd_MULTI_ISOCH_TRANSFERS
typedef	void (*sceUsbdMultiIsochronousDoneCallback)(int result,
	sceUsbdMultiIsochronousRequest *req, void *arg);
#endif

int sceUsbdRegisterLdd(sceUsbdLddOps *lddops);

int sceUsbdUnregisterLdd(sceUsbdLddOps *lddops);

void *sceUsbdScanStaticDescriptor(int dev_id, void *ptr, u_char type);

int sceUsbdGetDeviceLocation(int dev_id, u_char *locations);

int sceUsbdSetPrivateData(int dev_id, void *priv);

void *sceUsbdGetPrivateData(int dev_id);

int sceUsbdOpenPipe(int dev_id, UsbEndpointDescriptor *edesc);

int sceUsbdOpenPipeAligned(int dev_id, UsbEndpointDescriptor *edesc);

int sceUsbdClosePipe(int pipe_id);

int sceUsbdTransferPipe(int pipe_id, void *ptr, int len, void *option,
	sceUsbdDoneCallback done_cb, void *arg); 

#ifdef sceUsbd_MULTI_ISOCH_TRANSFERS
int sceUsbdMultiIsochronousTransfer(int pipe_id,
	sceUsbdMultiIsochronousRequest *req,
	sceUsbdMultiIsochronousDoneCallback done_cb, void *arg);
#endif

int sceUsbdChangeThreadPriority(int prio1, int prio2);

int sceUsbdGetReportDescriptor(int dev_id, int cfgnum, int ifnum, void **desc, int *len);

/* result code for sceUsbdDoneCallback */

#define sceUsbd_NOERR		0x000	/* No Error */
#define sceUsbd_CRC		0x001	/* CRC */
#define sceUsbd_BFV		0x002	/* Bit Stuffing Violation */
#define sceUsbd_DTM		0x003	/* Data Toggle Mismatch */
#define sceUsbd_STALL		0x004	/* Stall */
#define sceUsbd_NOTRESP		0x005	/* Device Not Responding */
#define sceUsbd_PIDCF		0x006	/* PID Check Failure */
#define sceUsbd_UEPID		0x007	/* Unexpected PID */
#define sceUsbd_DOR		0x008	/* Data Overrun */
#define sceUsbd_DUR		0x009	/* Data Underrun */
#define sceUsbd_RSVDA		0x00a	/* (reserved) */
#define sceUsbd_RSVDB		0x00b	/* (reserved) */
#define sceUsbd_BOR		0x00c	/* Buffer Overrun */
#define sceUsbd_BUR		0x00d	/* Buffer Underrun */
#define sceUsbd_NOTACC1		0x00e	/* (not accessed) */
#define sceUsbd_NOTACC2		0x00f	/* (not accessed) */

#define sceUsbd_PSW_BITS	0x0f0	/* PSW.CC (Isochronous) */
#define sceUsbd_PSW_SHIFT	4

#define sceUsbd_INVAL_DEVICE	0x101	/* Invalid device id */
#define sceUsbd_INVAL_PIPE	0x102	/* Invalid pipe id */
#define sceUsbd_INVAL_LENGTH	0x103	/* Invalid length */
#define sceUsbd_INVAL_LDDOPS	0x104	/* Invalid LDD ops */
#define sceUsbd_INVAL_CONTEXT	0x105	/* Invalid context */
#define sceUsbd_INVAL_ALIGN	0x106	/* Invalid alignment */
#define sceUsbd_INVAL_HUB_DEPTH	0x107	/* Invalid hub Depth */
#define sceUsbd_NO_ED		0x111	/* No space (ED) */
#define sceUsbd_NO_IOREQ	0x112	/* No space (IOREQ) */
#define sceUsbd_NO_OPTION	0x113	/* No Option */
#define sceUsbd_BUSY		0x121	/* Busy */
#define sceUsbd_ABORTED		0x122	/* Aborted */
#define sceUsbd_NOT_IMP		0x131	/* Not yet implemented */
#define sceUsbd_ERROR		0x132	/* Error (unknown reason) */

/* Control,Isochronous,Bulk,Interrupt transfer */

#define sceUsbdControlTransfer(pid, rt, r, v, i, l, ptr, cb, arg)	({ \
	UsbDeviceRequest _dr; \
	_dr.bmRequestType = (rt); \
	_dr.bRequest = (r); \
	_dr.wValue = (v); \
	_dr.wIndex = (i); \
	_dr.wLength = (l); \
	sceUsbdTransferPipe((pid), (ptr), _dr.wLength, &_dr, (cb), (arg)); })

#define sceUsbdIsochronousTransfer(pid, ptr, len, delta, cb, arg)	({ \
	sceUsbdTransferPipe((pid), (ptr), (len), (void *)(delta), \
		(cb), (arg)); })

#define sceUsbdBulkTransfer(pid, ptr, len, cb, arg) ({ \
	sceUsbdTransferPipe((pid), (ptr), (len), NULL, (cb), (arg)); })

#define sceUsbdInterruptTransfer(pid, ptr, len, cb, arg) ({ \
	sceUsbdTransferPipe((pid), (ptr), (len), NULL, (cb), (arg)); })

/* Standard Control transfer */

#define sceUsbdClearDeviceFeature(pid, fs, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x00, USB_REQUEST_CLEAR_FEATURE, \
		(fs), 0, 0, NULL, (cb), (arg))

#define sceUsbdClearInterfaceFeature(pid, fs, interface, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x01, USB_REQUEST_CLEAR_FEATURE, \
		(fs), (interface), 0, NULL, (cb), (arg))

#define sceUsbdClearEndpointFeature(pid, fs, endpoint, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x02, USB_REQUEST_CLEAR_FEATURE, \
		(fs), (endpoint), 0, NULL, (cb), (arg))

#define sceUsbdGetConfiguration(pid, ptr, cb, arg) \
	sceUsbdControlTransfer((pid), 0x80, USB_REQUEST_GET_CONFIGURATION, \
		0, 0, 1, (ptr), (cb), (arg))

#define sceUsbdGetDescriptor(pid, type, index, lang_id, ptr, len, cb, arg) \
	sceUsbdControlTransfer((pid), 0x80, USB_REQUEST_GET_DESCRIPTOR, \
		((type) << 8) | (index), (lang_id), (len), (ptr), (cb), (arg))

#define sceUsbdGetInterface(pid, interface, ptr, cb, arg) \
	sceUsbdControlTransfer((pid), 0x81, USB_REQUEST_GET_INTERFACE, \
		0, (interface), 1, (ptr), (cb), (arg))

#define sceUsbdGetDeviceStatus(pid, ptr, cb, arg) \
	sceUsbdControlTransfer((pid), 0x80, USB_REQUEST_GET_STATUS, \
		0, 0, 2, (ptr), (cb), (arg))

#define sceUsbdGetInterfaceStatus(pid, interface, ptr, cb, arg) \
	sceUsbdControlTransfer((pid), 0x81, USB_REQUEST_GET_STATUS, \
		0, (interface), 2, (ptr), (cb), (arg))

#define sceUsbdGetEndpointStatus(pid, endpoint, ptr, cb, arg) \
	sceUsbdControlTransfer((pid), 0x82, USB_REQUEST_GET_STATUS, \
		0, (endpoint), 2, (ptr), (cb), (arg))

#define sceUsbdSetAddress(pid, address, cb, arg) \
	sceUsbdControlTransfer((pid), 0x00, USB_REQUEST_SET_ADDRESS, \
		(address), 0, 0, NULL, (cb), (arg))

#define sceUsbdSetConfiguration(pid, config, cb, arg) \
	sceUsbdControlTransfer((pid), 0x00, USB_REQUEST_SET_CONFIGURATION, \
		(config), 0, 0, NULL, (cb), (arg))

#define sceUsbdSetDeviceDescriptor(pid, type, index, lang_id, ptr, len, \
		cb, arg) \
	sceUsbdControlTransfer((pid), 0x00, USB_REQUEST_SET_DESCRIPTOR, \
		((type) << 8) | (index), (lang_id), (len), (ptr), (cb), (arg))

#define sceUsbdSetInterfaceDescriptor(pid, type, index, lang_id, ptr, len, \
		cb, arg) \
	sceUsbdControlTransfer((pid), 0x01, USB_REQUEST_SET_DESCRIPTOR, \
		((type) << 8) | (index), (lang_id), (len), (ptr), (cb), (arg))

#define sceUsbdSetEndpointDescriptor(pid, type, index, lang_id, ptr, len, \
		cb, arg) \
	sceUsbdControlTransfer((pid), 0x02, USB_REQUEST_SET_DESCRIPTOR, \
		((type) << 8) | (index), (lang_id), (len), (ptr), (cb), (arg))

#define sceUsbdSetDeviceFeature(pid, fs, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x00, USB_REQUEST_SET_FEATURE, \
		(fs), 0, 0, NULL, (cb), (arg))

#define sceUsbdSetInterfaceFeature(pid, fs, interface, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x01, USB_REQUEST_SET_FEATURE, \
		(fs), (interface), 0, NULL, (cb), (arg))

#define sceUsbdSetEndpointFeature(pid, fs, endpoint, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x02, USB_REQUEST_SET_FEATURE, \
		(fs), (endpoint), 0, NULL, (cb), (arg))

#define sceUsbdSetInterface(pid, interface, alt_setting, cb, arg) \
	sceUsbdControlTransfer((pid), 0x01, USB_REQUEST_SET_INTERFACE, \
		(alt_setting), (interface), 0, NULL, (cb), (arg))

#define sceUsbdSynchFrame(pid, endpoint, pfn, cb, arg)	\
	sceUsbdControlTransfer((pid), 0x82, USB_REQUEST_SYNCH_FRAME, \
		0, (endpoint), 2, (pfn), (cb), (arg))

#endif	/* !_LIBUSBD_H */
