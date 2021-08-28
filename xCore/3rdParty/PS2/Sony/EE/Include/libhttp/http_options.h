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
 *                    <libhttp[s] - http_options.h>
 *                     <SetOption/GetOptino codes>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Nov,05,2001     komaki      first version
 */

typedef enum {
	sceHTTPO_ClientName,
	sceHTTPO_HTTPRevision,
	sceHTTPO_Method,
	sceHTTPO_ParsedURI,
	sceHTTPO_ProxyURI,
	sceHTTPO_RequestHeaders,
	sceHTTPO_RequestEntity,
	sceHTTPO_ResponseTimeout,
	sceHTTPO_TransferTimeout,
	sceHTTPO_BlockingMode,
	sceHTTPO_EndOfTransactionCB,
	sceHTTPO_ReceiveChunkCB,
	sceHTTPO_ThreadValue,
	sceHTTPO_KeepAlive,
	sceHTTPO_SSLFlags,
	sceHTTPO_HashCB,
	sceHTTPO_HashSize,
	sceHTTPO_LocalFile,
	sceHTTPO_FileWriteFunction,
	sceHTTPO_DelRequestHeader,
	sceHTTPO_DefaultHeaders,
	sceHTTPO_OpenTimeout,
	sceHTTPO_BlockingOpenMode,
	sceHTTPO_EndOfOpenCB,
	sceHTTPO_FileWrite4Function
} sceHTTPOption_t;
