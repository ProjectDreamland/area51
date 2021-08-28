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
 *                       <libhttp - libhttp.h>
 *                     <header for http library>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Nov,05,2001     komaki      first version
 */

#ifndef _LIBHTTP_H
#define _LIBHTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libhttp/http_status.h>
#include <libhttp/http_methods.h>
#include <libhttp/http_options.h>

#define sceHTTPProt_HTTP	0
#define sceHTTPProt_HTTPS	1

typedef struct sceHTTPHeaderList {
	struct sceHTTPHeaderList *forw, *back;
	char *name;
	char *value;
} sceHTTPHeaderList_t;

#ifdef __GNUC__
typedef int sceHTTPTime_t __attribute__ ((mode (__DI__)));
typedef int sceHTTPSize_t __attribute__ ((mode (__DI__)));
#else
typedef long long sceHTTPTime_t;
typedef long long sceHTTPSize_t;
#endif

typedef struct sceHTTPResponse {
	int http_ver;
	int http_rev;
	sceHTTPStatusCode_t code;	/* server status return	*/
	const char *reason;		/* reason phrase */
	int server_prot;		/* server protocol */
	sceHTTPHeaderList_t *headers;	/* list of returned headers */
	unsigned char *entity;		/* response data */
	sceHTTPSize_t length;		/* length of response data */
	int interrupted;		/* transfer interrupted */
	sceHTTPTime_t date;		/* response date and time */
	sceHTTPSize_t content_length;	/* length of entity */
} sceHTTPResponse_t;

typedef struct sceHTTPClient {
	char *name;			/* user-agent name */
	int http_ver;
	int http_rev;
	int rtimeout;			/* response timeout */
	int ttimeout;			/* transfer timeout */
	int laptime;
	int prot;			/* protocol */
	int state;
	int errnum;
	int net_errno;
	int net_herrno;
	int reloading;
	int keep_alive;
	int keep_count;
	int non_blocking;
	int abort_req;
	int otimeout;
	int c_wait0, c_wait1;
	int c_tcount;
	int c_timer;
	int t_stacksize;
	int t_priority;
	int t_thread;
	void *t_stack;
	int t_rtn;
	void (*t_notify)(struct sceHTTPClient *, int, void *);
	void *t_notify_opt;
	void *t_notify_gp;
	int t_busy;
	void (*t_hash)(struct sceHTTPClient *, sceHTTPSize_t, void *);
	void *t_hash_opt;
	void *t_hash_gp;
	int hashbytes;
	unsigned int t_sent;
	unsigned int max_olength;
	struct sceHTTPParsedURI *proxy;
	int c_stacksize;
	int c_priority;
	int c_thread;
	void *c_stack;
	void (*c_notify)(struct sceHTTPClient *, int, void *);
	void *c_notify_opt;
	void *c_notify_gp;
	int nb_open;
	/* Request */
	sceHTTPMethod_t method;		/* method code */
	struct sceHTTPParsedURI *puri;	/* parsed identifier */
	sceHTTPHeaderList_t *iheaders;	/* list of headers */
	char *idata;			/* input data */
	int ilength;			/* length of idata */
	int iflags;			/* input flags */
	int fd;				/* local file descriptor */
	int (*writef)(int, unsigned char *, unsigned int);
	void *writef_gp;
	int hflags;
	/* Response */
	sceHTTPResponse_t response;
	void (*chunkf)(struct sceHTTPClient *, unsigned char *, unsigned int,
			void *);
	void *chunkf_opt;
	void *chunkf_gp;
	/* io */
	int recv_thread;
	int send_thread;
	void *io_rstack;
	void *io_sstack;
	int io_desc;
	char *io_buf;
	int io_len;
	int io_rtn;
	int io_timer;
	int io_rwait, io_rdone;
	int io_swait, io_sdone;
	int io_flags;
	int io_tcount;
	int reserved00;
	void *reserved0;
	void *reserved1;
	int reserved2;
	int hflags2;
	void *write4_opt;
	int pad[35];
} sceHTTPClient_t;

/* iflags */
/* sceHTTPInputF_ESCAPE wants to encode data with URL encoding. */
#define sceHTTPInputF_ESCAPE	1
/* The input data doesn't copied if sceHTTPInputF_LINK is on. */
#define sceHTTPInputF_LINK	2

/* hflags */
#define sceHTTPHeaderF_Accept		1
#define sceHTTPHeaderF_AcceptCharset	2
#define sceHTTPHeaderF_UserAgent	4
#define sceHTTPHeaderF_AcceptEncoding	8
#define sceHTTPHeaderF_HostCase		16

/* errnum */
#define	sceHTTPError_KERNEL	-1001
#define	sceHTTPError_NOMEM	-1002
#define	sceHTTPError_IO		-1003
#define	sceHTTPError_INVAL	-1004
#define	sceHTTPError_TIMEOUT	-1005
#define	sceHTTPError_RESOLV	-1006
#define	sceHTTPError_SOCKET	-1007
#define	sceHTTPError_CONNECT	-1008
#define	sceHTTPError_SSL	-1009
#define	sceHTTPError_NOTYET	-1010
#define	sceHTTPError_INTR	-1011
#define	sceHTTPError_PROXY	-1012
#define	sceHTTPError_BUSY	-1013
#define	sceHTTPError_WRITEF	-1014
#define	sceHTTPError_LENGTH	-1015
#define	sceHTTPError_NETGLUE	-1016

typedef struct sceHTTPParsedURI {
	const char *scheme;
	const char *username;
	const char *password;
	const char *hostname;
	int port;
	const char *filename;
	const char *search;
} sceHTTPParsedURI_t;

#define sceHTTPParseURI_USER		0x02
#define sceHTTPParseURI_FILENAME	0x20
#define sceHTTPParseURI_SEARCHPART	0x40

typedef struct sceHTTPCookie {
	const char *name;
	const char *value;
	const char *domain;
	const char *path;
	sceHTTPTime_t expires;
	int secure;
	int version;
	sceHTTPTime_t maxage;
	const char *comment;
	int cookie2;
	int discard;
	const char *comment_url;
	const char *port;
} sceHTTPCookie_t;

typedef struct sceHTTPCookieList {
	struct sceHTTPCookieList *forw, *back;
	struct sceHTTPCookie cookie;
} sceHTTPCookieList_t;

typedef struct sceHTTPAuth {
	int type;
	const char *realm;
	const char **domains;
	const char *uri;
	const char *nonce;
	const char *opaque;
	int stale;
	int algorithm;
	int qop;
	int proxy;
} sceHTTPAuth_t;

typedef struct sceHTTPAuthList {
	struct sceHTTPAuthList *forw, *back;
	struct sceHTTPAuth auth;
} sceHTTPAuthList_t;

typedef struct sceHTTPAuthInfo {
	const char *nextnonce;
	const char *rspauth;
	const char *cnonce;
	int nc;
	int qop;
	int proxy;
} sceHTTPAuthInfo_t;

typedef struct sceHTTPDigest {
	char *username;
	char *realm;
	char *password;
	char *uri;
	char *nonce;
	char *cnonce;
	char *opaque;
	int algorithm;
	int nc;
	int qop;
	int method;
	char *entity;
	int length;
} sceHTTPDigest_t;

#define sceHTTPAuth_BASIC		0
#define sceHTTPAuth_DIGEST		1
#define sceHTTPDigestAlg_MD5		1
#define sceHTTPDigestAlg_MD5SESS	2
#define sceHTTPDigestQOP_AUTH		1
#define sceHTTPDigestQOP_AUTHINT	2
#define sceHTTPDigestStale_TRUE		1
#define sceHTTPDigestStale_FALSE	2
#define sceHTTPVerifyAuthInfo_NORSPAUTH	1

typedef struct sceHTTPMimeFilter {
	struct sceHTTPMimeFilter *next;
	struct sceHTTPMimeFilter *prev;
	int itype;
	int idesc;
	unsigned char *ibuf;
	unsigned int ibuflen;
	unsigned char *iptr;
	int idesc_eof;
	int otype;
	int odesc;
	unsigned char *obuf;
	unsigned int obuflen;
	unsigned char *optr;
	sceHTTPHeaderList_t *headers;
	int dflags;
	int (*decoder)(const char *, char *, int);
	unsigned char *dbuf;
	int multipart;
	char *boundary;
} sceHTTPMimeFilter_t;

#define sceHTTPMimeFilter_STRING	0
#define sceHTTPMimeFilter_FILE		1

#define	sceHTTPMultipart_MIXED		1
#define	sceHTTPMultipart_BYTERANGES	2
#define	sceHTTPMultipart_ALTERNATIVE	3

/* mode for sceHTTPSetOption with sceHTTPO_RequestHeaders */
#define sceHTTPRequestHeaders_ADD	0
#define sceHTTPRequestHeaders_OVERWRITE	1
#define sceHTTPRequestHeaders_MERGE	2

/* erx export */
void *sceHTTPGetErxEntries(void);

/* Memory function types */
typedef void *(*sceHTTPMallocFunction_t)(size_t size);
typedef void *(*sceHTTPReallocFunction_t)(void *p, size_t);
typedef void (*sceHTTPFreeFunction_t)(void *p);

/* Library initialization */
extern int sceHTTPInit(void);
extern int sceHTTPSetDefaultThreadValue(int stacksize, int priority);

/* Library termination */
extern int sceHTTPTerminate(void);

/* HTTP connection */
extern int sceHTTPOpen(sceHTTPClient_t *client);
extern int sceHTTPClose(sceHTTPClient_t *client);
extern int sceHTTPGetSocketError(sceHTTPClient_t *client);
extern int sceHTTPGetResolveError(sceHTTPClient_t *client);

/* HTTP client transaction */
extern sceHTTPClient_t *sceHTTPCreate(void);
extern int sceHTTPDestroy(sceHTTPClient_t *client);
extern int sceHTTPRequest(sceHTTPClient_t *client);
extern sceHTTPResponse_t *sceHTTPGetResponse(sceHTTPClient_t *client);
extern int sceHTTPCleanUpResponse(sceHTTPClient_t *client);
extern int sceHTTPGetClientError(sceHTTPClient_t *client);
extern int sceHTTPAbortRequest(sceHTTPClient_t *client);
extern sceHTTPSize_t sceHTTPGetTransferedBytes(sceHTTPClient_t *client);
extern sceHTTPSize_t sceHTTPGetPostedBytes(sceHTTPClient_t *p);
extern sceHTTPSize_t sceHTTPGetContentLength(sceHTTPClient_t *client);

/* Abort */
extern int sceHTTPAbort(sceHTTPClient_t *client);

/* Options */
extern int sceHTTPSetOption(sceHTTPClient_t *client, sceHTTPOption_t opt, ...);
extern int sceHTTPGetOption(sceHTTPClient_t *client, sceHTTPOption_t opt, ...);

/* URI */
extern sceHTTPParsedURI_t *sceHTTPParseURI(const char *uri, int parseflag);
extern int sceHTTPFreeURI(sceHTTPParsedURI_t *puri);
extern sceHTTPParsedURI_t *sceHTTPCloneURI(const sceHTTPParsedURI_t *puri);
extern char *sceHTTPUnparseURI(const sceHTTPParsedURI_t *puri);
extern int sceHTTPIsAbsoluteURI(const char *uri);
extern char *sceHTTPFindAbsoluteURI(const char *uri, const char *base);

/* Convenience */
extern const char *sceHTTPErrorString(sceHTTPStatusCode_t error);

/* Header List */
extern sceHTTPHeaderList_t *sceHTTPAddHeaderList(sceHTTPHeaderList_t *p,
			const char *name, const char *value);
extern int sceHTTPFreeHeaderList(sceHTTPHeaderList_t *p);

static __inline__ sceHTTPHeaderList_t *sceHTTPNextHeader(const sceHTTPHeaderList_t *p){
	return((p)? p->forw : 0);
}

/* Cookies */
extern sceHTTPCookieList_t *sceHTTPAddCookieList(sceHTTPCookieList_t *p,
			const sceHTTPCookie_t *cp);
extern sceHTTPCookieList_t *sceHTTPDeleteCookieListEntry(
			sceHTTPCookieList_t *p,	sceHTTPCookieList_t *ep);
extern sceHTTPCookieList_t *sceHTTPFilterCookieList(const sceHTTPCookieList_t *list,
			int (*filter)(sceHTTPCookie_t *, void *), void *arg);
extern int sceHTTPFreeCookieList(sceHTTPCookieList_t *p);
extern sceHTTPCookieList_t *sceHTTPParseCookie(sceHTTPClient_t *client,
			const sceHTTPResponse_t *rp);
extern int sceHTTPSetCookie(sceHTTPClient_t *client,
			const sceHTTPCookieList_t *p);

/* Authentication */
extern int sceHTTPFreeAuthList(sceHTTPAuthList_t *p);
extern sceHTTPAuthList_t *sceHTTPParseAuth(const sceHTTPResponse_t *rp);
extern int sceHTTPSetBasicAuth(sceHTTPClient_t *client,
			const char *user, const char *passwd, int proxy);
extern int sceHTTPSetDigestAuth(sceHTTPClient_t *client,
			const sceHTTPDigest_t *dp,
			int proxy);
extern sceHTTPAuthInfo_t *sceHTTPParseAuthInfo(const sceHTTPResponse_t *rp);
extern int sceHTTPFreeAuthInfo(sceHTTPAuthInfo_t *ip);
extern int sceHTTPVerifyAuthInfo(sceHTTPClient_t *client,
			const sceHTTPAuthInfo_t *ip, const sceHTTPDigest_t *dp,
			int flags);
/* Redirection */
extern const char **sceHTTPParseLocations(const sceHTTPResponse_t *rp);
extern int sceHTTPFreeLocations(const char **vec);
extern int sceHTTPSetRedirection(sceHTTPClient_t *client,
			const sceHTTPParsedURI_t *uri, int proxy);
/* Mime */
extern sceHTTPMimeFilter_t *sceHTTPMimeFilterCreate(int itype, void *iarg,
		int ilen, int otype, void *oarg);
extern int sceHTTPMimeFilterFree(sceHTTPMimeFilter_t *p);
extern int sceHTTPMimeFilterParseHeaders(sceHTTPMimeFilter_t *p);
extern int sceHTTPMimeFilterApply(sceHTTPMimeFilter_t *p, int *closep);
extern int sceHTTPMimeFilterGetMultipartType(sceHTTPMimeFilter_t *p);
extern int sceHTTPMimeFilterChangeOutput(sceHTTPMimeFilter_t *p, int otype,
		void *arg);
extern int sceHTTPMimeFilterGetStringOutput(sceHTTPMimeFilter_t *p,
		char **odatap, int *olengthp);
extern sceHTTPHeaderList_t *sceHTTPMimeFilterGetHeaderList(
		sceHTTPMimeFilter_t *p);

/* Memory functions */
extern int sceHTTPInitMemory(void *pool, unsigned int poolsize);
extern void sceHTTPFreeMemory(void *p);
extern void sceHTTPSetMallocFunction(sceHTTPMallocFunction_t mallocp);
extern void sceHTTPSetReallocFunction(sceHTTPReallocFunction_t reallocp);
extern void sceHTTPSetFreeFunction(sceHTTPFreeFunction_t freep);
extern int sceHTTPGetMemoryStatus(unsigned int *curp, unsigned int *minp);

/* Version */
extern const char *sceHTTPGetLibVersion(void);

/* url escape */
extern unsigned char *sceURLEscape(unsigned const char *in);
extern unsigned char *sceURLUnescape(unsigned const char *in);

#ifdef __cplusplus
}
#endif

#endif /* _LIBHTTP_H */
