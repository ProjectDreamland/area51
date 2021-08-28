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
 *                    <libhttp[s] - http_methods.h>
 *                          <http methods>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Nov,05,2001     komaki      first version
 */

typedef enum {
	sceHTTPM_OPTIONS,
	sceHTTPM_GET,
	sceHTTPM_HEAD,
	sceHTTPM_POST,
	sceHTTPM_PUT,
	sceHTTPM_DELETE,
	sceHTTPM_TRACE,
	sceHTTPM_CONNECT,
	sceHTTPM_PROPFIND,
	sceHTTPM_PROPPATCH,
	sceHTTPM_MKCOL,
	sceHTTPM_COPY,
	sceHTTPM_MOVE,
	sceHTTPM_LOCK,
	sceHTTPM_UNLOCK
} sceHTTPMethod_t;
