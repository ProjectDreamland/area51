/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *			   i.LINK Library
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                             ilink.h
 *                     header file for i.LINK
 *
 *      Version         Date            Design  Log
 *  --------------------------------------------------------------------
 *	1.0		2000/04/07	hig	v1.7
 *	3.11		2000/06/13	hig	v3.11
 *	3.13		2000/07/28	hig	v3.13
 *	3.13.1		2000/11/08	sim	v3.13.1
 */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

enum Sce1394ErrorCode {
	SCE1394ERR_OK			=  0,
	SCE1394ERR_ERROR		= -1,

	SCE1394ERR_NOT_INITIALIZED	= -1000,
	SCE1394ERR_NOT_SUPPORTED	= -1001,
	SCE1394ERR_NO_MEMORY		= -1002,
	SCE1394ERR_RESOURCE_UNAVAILABLE	= -1003,
	SCE1394ERR_INVALID_ID		= -1004,
	SCE1394ERR_INVALID_REQUEST	= -1005,
	SCE1394ERR_INVALID_ARGUMENT	= -1006,
	SCE1394ERR_INVALID_SIZE		= -1007,
	SCE1394ERR_INVALID_ADDRESS	= -1008,

	SCE1394ERR_TRANSACTION_ERROR	= -1020,
	SCE1394ERR_RESET_DETECTED	= -1021,
	SCE1394ERR_REQUEST_DISABLED	= -1022,
	SCE1394ERR_FAILED_RESPONSE	= -1023,
	SCE1394ERR_TIMEOUT		= -1024,
	SCE1394ERR_ACK_MISSING		= -1025,
	SCE1394ERR_RETRY_LIMIT		= -1026,
	SCE1394ERR_DATA_ERROR		= -1027,
	SCE1394ERR_INVALID_PARAMETER	= -1028,

	SCE1394ERR_RESPONSE_DATA_ERROR	= -1041,
	SCE1394ERR_RESPONSE_FORMAT_ERROR = -1042,
	SCE1394ERR_REQUEST_DATA_ERROR	= -1043,
	SCE1394ERR_RESPONSE_ACK_MISSING	= -1044,
	SCE1394ERR_UNSOLICITED_RESPONSE	= -1045,
	SCE1394ERR_RESPONSE_RETRY_LIMIT	= -1046,

	SCE1394ERR_HEADER_CRC_ERROR	= -1061,
	SCE1394ERR_UNKNOWN_TCODE	= -1062,
	SCE1394ERR_CYCLE_TOO_LONG	= -1063,

	SCE1394ERR_NO_MANAGER		= -1081,

};

typedef enum Sce1394ErrorCode	Sce1394ErrorCode;

/* ================================================================ */

extern int	sce1394Initialize(void *arg);
extern int	sce1394Destroy();
extern int	sce1394ConfGet(int member, ...);
extern int	sce1394ConfSet(int member, int value);
extern int	sce1394ChangeThreadPriority(int hi, int lo);

#define SCE1394CF_PRIORITY_HI		1
#define SCE1394CF_PRIORITY_LO		2
/* maximum transaction payload size */
#define SCE1394CF_TRSIZE_MASTER		3
#define SCE1394CF_TRSIZE_SLAVE		4
#define SCE1394CF_NODE_CAPABILITY	5
#define SCE1394CF_NODE_IRMC		(1<<7)
#define SCE1394CF_NODE_CMC		(1<<6)
#define SCE1394CF_NODE_ISC		(1<<5)
#define SCE1394CF_NODE_BMC		(1<<4)
#define SCE1394CF_NODE_PMC		(1<<3)


extern int		sce1394UnitAdd(int nquad, u_int *data);
extern Sce1394ErrorCode	sce1394UnitDelete(int id);

/* ================================================================ */

/* SB_CONTROL.request */

extern Sce1394ErrorCode	sce1394SbEui64(u_int *buff);
extern Sce1394ErrorCode	sce1394SbEnable();
extern Sce1394ErrorCode	sce1394SbDisable(int doReset);
extern Sce1394ErrorCode	sce1394SbReset(int flag);
extern int		sce1394SbGenNumber();
extern int		sce1394SbNodeId();
extern int		sce1394SbNodeCount();
extern int		sce1394SbSelfId(int nquad, u_int *buff);
extern Sce1394ErrorCode	sce1394SbPhyPacket(int request, u_int arg);
extern u_int		sce1394SbCycleTime(u_int *bustime);

/* ================================================================ */

/* SB_EVENT.indication */

extern int		sce1394EvAlloc();
extern Sce1394ErrorCode	sce1394EvFree(int id);
extern int		sce1394EvWait(int id, int eventMask);
extern int		sce1394EvPoll(int id, int eventMask);

#define SCE1394EV_BUS_RESET_START		1
#define SCE1394EV_BUS_RESET_COMPLETE		2
#define SCE1394EV_SELF_ID			3
#define SCE1394EV_COMMAND_RESET			4

#define SCE1394EV_BUS_OCCUPANCY_VIOLATION	8
#define SCE1394EV_CYCLE_TOO_LONG		9
#define SCE1394EV_CABLE_POWER_FAIL		10
#define SCE1394EV_DUPLICATE_CHANNEL		11
#define SCE1394EV_HEADER_CRC_ERROR		12
#define SCE1394EV_REQUEST_DATA_ERROR		13
#define SCE1394EV_RESPONSE_ACK_MISSING		14
#define SCE1394EV_RESPONSE_DATA_ERROR		15
#define SCE1394EV_RESPONSE_FORMAT_ERROR		16
#define SCE1394EV_RESPONSE_RETRY_FAILED		17
#define SCE1394EV_UNEXPECTED_CHANNEL		18
#define SCE1394EV_UNKNOWN_TCODE			19
#define SCE1394EV_UNSOLICITED_RESPONSE		20

/* ================================================================ */

/* TR_DATA.indication */

typedef int	(*sce1394TrDataIndProc)(int pb, void *arg);

extern int	sce1394TrDataInd(
				int offH, u_int offL, u_int size,
				sce1394TrDataIndProc wFunc, void *wArg,
				sce1394TrDataIndProc rFunc, void *rArg,
				sce1394TrDataIndProc lFunc, void *lArg
				);

extern Sce1394ErrorCode	sce1394TrDataUnInd(int id);

/* ================================================================ */

extern int sce1394PbAlloc(int size, int mode);
extern int sce1394PbFree(int pb);
extern int sce1394PbGet(int pb, int member, ...);
extern int sce1394PbSet(int pb, int member, u_int value);

#define SCE1394PB_BUFFER	0x10
#define SCE1394PB_GENNUMBER	0x11
#define SCE1394PB_SIZE		0x12
#define SCE1394PB_SPEED		0x13
#define SCE1394PB_ACK		0x14
#define SCE1394PB_STATUS	0x15
#define SCE1394PB_EXTENSION	0x16


#define SCE1394PB_TCODE		0x20
#define SCE1394PB_TLABEL	0x21
#define SCE1394PB_DEST		0x22
#define SCE1394PB_SOURCE	0x23
#define SCE1394PB_OFFH		0x24
#define SCE1394PB_OFFL		0x25
#define SCE1394PB_LENGTH	0x26
#define SCE1394PB_EXTCODE	0x27
#define SCE1394PB_RCODE		0x28
#define SCE1394PB_QUAD_PAYLOAD	0x29
#define SCE1394PB_BLOCK_PAYLOAD	0x2a

#define sce1394PbGetBuffer(pb)		sce1394PbGet(pb, SCE1394PB_BUFFER)
#define sce1394PbGetGenNumber(pb)	sce1394PbGet(pb, SCE1394PB_GENNUMBER)
#define sce1394PbGetSize(pb)		sce1394PbGet(pb, SCE1394PB_SIZE)
#define sce1394PbGetSpeed(pb)		sce1394PbGet(pb, SCE1394PB_SPEED)
#define sce1394PbGetAckCode(pb)		sce1394PbGet(pb, SCE1394PB_ACK)
#define sce1394PbGetStatus(pb)		sce1394PbGet(pb, SCE1394PB_STATUS)

#define sce1394PbGetTCode(pb)		sce1394PbGet(pb, SCE1394PB_TCODE)
#define sce1394PbGetTLabel(pb)		sce1394PbGet(pb, SCE1394PB_TLABEL)
#define sce1394PbGetDest(pb)		sce1394PbGet(pb, SCE1394PB_DEST)
#define sce1394PbGetSource(pb)		sce1394PbGet(pb, SCE1394PB_SOURCE)
#define sce1394PbGetOffsetH(pb)		sce1394PbGet(pb, SCE1394PB_OFFH)
#define sce1394PbGetOffsetL(pb, offL)	sce1394PbGet(pb, SCE1394PB_OFFL, offL)
#define sce1394PbGetLength(pb)		sce1394PbGet(pb, SCE1394PB_LENGTH)
#define sce1394PbGetExTCode(pb)		sce1394PbGet(pb, SCE1394PB_EXTCODE)
#define sce1394PbGetRCode(pb)		sce1394PbGet(pb, SCE1394PB_RCODE)
#define sce1394PbGetQuadPayload(pb, payload)	\
			sce1394PbGet(pb, SCE1394PB_QUAD_PAYLOAD, payload)
#define sce1394PbGetBlockPtr(pb, pointer)	\
			sce1394PbGet(pb, SCE1394PB_BLOCK_PAYLOAD, pointer)


#define sce1394PbSetGenNumber(pb, genNumber)	\
		sce1394PbSet(pb, SCE1394PB_GENNUMBER,	genNumber)
#define sce1394PbSetSize(pb, size)		\
		sce1394PbSet(pb, SCE1394PB_SIZE,	size)
#define sce1394PbSetSpeed(pb, speed)		\
		sce1394PbSet(pb, SCE1394PB_SPEED,	speed)

#define sce1394PbSetTCode(pb, tCode)	\
		sce1394PbSet(pb, SCE1394PB_TCODE,	tCode)
#define sce1394PbSetTLabel(pb, tLabel)	\
		sce1394PbSet(pb, SCE1394PB_TLABEL,	tLabel)
#define sce1394PbSetDest(pb, dest)	\
		sce1394PbSet(pb, SCE1394PB_DEST,	dest)
#define sce1394PbSetSource(pb, source)	\
		sce1394PbSet(pb, SCE1394PB_SOURCE,	source)
#define sce1394PbSetOffsetH(pb, offH)	\
		sce1394PbSet(pb, SCE1394PB_OFFH,	offH)
#define sce1394PbSetOffsetL(pb, offL)	\
		sce1394PbSet(pb, SCE1394PB_OFFL,	offL)
#define sce1394PbSetLength(pb, length)	\
		sce1394PbSet(pb, SCE1394PB_LENGTH,	length)
#define sce1394PbSetExTCode(pb, extCode)	\
		sce1394PbSet(pb, SCE1394PB_EXTCODE,	extCode)
#define sce1394PbSetRCode(pb, rCode)	\
		sce1394PbSet(pb, SCE1394PB_RCODE,	rCode)
#define sce1394PbSetQuadPayload(pb, payload)	\
		sce1394PbSet(pb, SCE1394PB_QUAD_PAYLOAD,	payload)

/* ================================================================ */

/* TR_DATA.request */
/* TR_DATA.confirmation */

extern int		sce1394TrAlloc(int nodeId, int mode);
extern Sce1394ErrorCode	sce1394TrFree(int tc);
    extern int		sce1394TrGet(int tc, int member, ...); /* System used */
extern int		sce1394TrSet(int tc, int member, int value); /* System used */
#define SCE1394TR_GENNUMBER	1
#define SCE1394TR_DEST		2
#define SCE1394TR_SPEED		3
#define SCE1394TR_BSIZE		4
#define SCE1394TR_MODE		5
#define SCE1394TR_STATUS	6
#define SCE1394TR_EXTENSION	7

#define sce1394TrGetGenNumber(tc)	sce1394TrGet(tc, SCE1394TR_GENNUMBER)
#define sce1394TrGetDest(tc)		sce1394TrGet(tc, SCE1394TR_DEST)
#define sce1394TrGetSpeed(tc)		sce1394TrGet(tc, SCE1394TR_SPEED)
#define sce1394TrGetBlockSize(tc)	sce1394TrGet(tc, SCE1394TR_BSIZE)
#define sce1394TrGetMode(tc)		sce1394TrGet(tc, SCE1394TR_MODE) /* System used */
#define sce1394TrGetStatus(tc, rCode)	\
			sce1394TrGet(tc, SCE1394TR_STATUS,	(int *)rCode) /* System used */
#define sce1394TrStatus			sce1394TrGetStatus
#define sce1394TrGetExtension(tc, ext)	\
			sce1394TrGet(tc, SCE1394TR_EXTENSION,	(void **)ext)

#define sce1394TrSetGenNumber(tc, genNumber)	\
			sce1394TrSet(tc, SCE1394TR_GENNUMBER,	genNumber)
#define sce1394TrSetDest(tc, destNode)		\
			sce1394TrSet(tc, SCE1394TR_DEST,	destNode)
#define sce1394TrSetSpeed(tc, speed)		\
			sce1394TrSet(tc, SCE1394TR_SPEED,	speed)
#define sce1394TrSetBlockSize(tc, size)		\
			sce1394TrSet(tc, SCE1394TR_BSIZE,	size)
#define sce1394TrSetMode(tc, mode)		\
			sce1394TrSet(tc, SCE1394TR_MODE,	mode) /* System used */
#define sce1394TrSetExtension(tc, ext)		\
			sce1394TrSet(tc, SCE1394TR_EXTENSION,	(int)ext)


extern int sce1394TrWrite(int tc, int offH, u_int offL, int len, void *buff);
extern int sce1394TrRead(int tc, int offH, u_int offL, int len, void *buff);
extern int sce1394TrLock(int tc, int offH, u_int offL, int len, void *buff, int ex_tCode);

typedef struct sce1394Iov {
	int	iov_len;
	void	*iov_base;
} sce1394Iov;

extern int sce1394TrWriteV(int tc, int offH, u_int offL, int cnt, sce1394Iov *iov);
extern int sce1394TrReadV(int tc, int offH, u_int offL, int cnt, sce1394Iov *iov);

/* ================================================================ */

extern int sce1394CrGenNumber();
extern int sce1394CrInvalidate();
extern int sce1394CrEui64(int nodeId, u_int *eui64);
extern int sce1394CrMaxRec(int nodeId);
extern int sce1394CrMaxSpeed(int nodeId);
extern int sce1394CrCapability(int nodeId);
extern int sce1394CrRead(int nodeId, u_int off, int nquad, u_int *buff);

extern int sce1394CrFindNode(u_int *eui64);
extern int sce1394CrFindUnit(int unit, int specId, int version, u_int *offset);

/* ================================================================ */

#define SCE1394_RC_COMP		0x0
#define SCE1394_RC_CERR		0x4
#define SCE1394_RC_DERR		0x5
#define SCE1394_RC_TERR		0x6
#define SCE1394_RC_AERR		0x7

#define SCE1394_BUS_LOCAL	0x3ff
#define SCE1394_SPEED_100M	0
#define SCE1394_SPEED_200M	1
#define SCE1394_SPEED_400M	2

#ifndef sce1394HtoN32
#ifdef __MIPSEL__
#define sce1394HtoN32(host32) \
	((((host32) >> 8) & 0xff00) | (((host32) & 0xff00) << 8) | \
	 ((host32) << 24) | ((unsigned)(host32) >> 24))
#endif /* __MIPSEL__ */
#ifdef __MIPSEB__
#define sce1394HtoN32(host32)	(host32)
#endif /* __MIPSEB__ */
#endif /* !sce1394HtoN32 */
#ifndef sce1394NtoH32
#define sce1394NtoH32(net32)	sce1394HtoN32(net32)
#endif /* !sce1394NtoH32 */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

/* ================================================================ */
