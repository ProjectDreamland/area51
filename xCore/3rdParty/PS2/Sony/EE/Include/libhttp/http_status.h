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
 *                    <libhttp[s] - http_status.h>
 *                          <http status>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Nov,05,2001     komaki      first version
 */

typedef enum {
	/* Informational 1xx */
	sceHTTPC_Continue			= 100,
	sceHTTPC_SwitchProtocols		= 101,
	sceHTTPC_Processing			= 102,

	/* Successfull 2xx */
	sceHTTPC_OK				= 200,
	sceHTTPC_Created			= 201,
	sceHTTPC_Accepted			= 202,
	sceHTTPC_NonAuthoritativeInfo		= 203,
	sceHTTPC_NoContent			= 204,
	sceHTTPC_ResetContent			= 205,
	sceHTTPC_PartialContent			= 206,
	sceHTTPC_MultiStatus			= 207,

	/* Redirection 3xx */
	sceHTTPC_MultipleChoices		= 300,
	sceHTTPC_MovedPermanentry		= 301,
	sceHTTPC_Found				= 302,
	sceHTTPC_SeeOther			= 303,
	sceHTTPC_NotModified			= 304,
	sceHTTPC_UseProxy			= 305,
	sceHTTPC_TemporaryRedirect		= 307,

	/* Client error 4xx */
	sceHTTPC_BadRequest			= 400,
	sceHTTPC_Unauthorized			= 401,
	sceHTTPC_PaymentRequired		= 402,
	sceHTTPC_Forbidden			= 403,
	sceHTTPC_NotFound			= 404,
	sceHTTPC_MethodNotAllowed		= 405,
	sceHTTPC_NotAcceptable			= 406,
	sceHTTPC_ProxyAuthenticationRequired	= 407,
	sceHTTPC_RequestTimeout			= 408,
	sceHTTPC_Conflict			= 409,
	sceHTTPC_Gone				= 410,
	sceHTTPC_LengthRequired			= 411,
	sceHTTPC_PreconditionFailed		= 412,
	sceHTTPC_RequestEntityTooLarge		= 413,
	sceHTTPC_RequestURITooLarge		= 414,
	sceHTTPC_UnsupportedMediaType		= 415,
	sceHTTPC_RequestedRangeNotSatisfiable	= 416,
	sceHTTPC_ExceptionFailed		= 417,
	sceHTTPC_UnprocessableEntity		= 422,
	sceHTTPC_Locked				= 423,
	sceHTTPC_FailedDependency		= 424,

	/* Server error 5xx */
	sceHTTPC_InternalServerError		= 500,
	sceHTTPC_NotImplemented			= 501,
	sceHTTPC_BadGateway			= 502,
	sceHTTPC_ServiceUnavailable		= 503,
	sceHTTPC_GatewayTimeout			= 504,
	sceHTTPC_HTTPVersionNotSupported	= 505,
	sceHTTPC_InsufficientStorage		= 507
} sceHTTPStatusCode_t;
