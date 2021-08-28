/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library "DNAS" package (Release 3.0 version)
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       <libdnas2 - dnas2.h>
 *                          <DNAS header>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      1.00            May,10,2001     komaki      first version
 */

#ifndef _DNAS2_H_
#define _DNAS2_H_

#include <com_error.h>

#define INTERNET_MAX_DOWNLEN 32768

#if defined(__R5900__) || defined(__ee__)
#ifndef _eetypes_h_
#include <eetypes.h>
#endif
#else
#include <sys/types.h>
#endif

/* Generic data storage structure */
typedef struct sceDNAS2Data {
	void *ptr;
	int size;
} sceDNAS2Data_t;

/* structure for DNAS Authentication */
typedef struct sceDNAS2TitleAuthInfo {
	int line_type;
	sceDNAS2Data_t pass_phrase;
	sceDNAS2Data_t auth_data;
} sceDNAS2TitleAuthInfo_t;

/* Structure for time management in DNAS */
typedef struct sceDNAS2LastDate {
	u_char year;
	u_char month;
	u_char date;
	u_char hour;
	u_char min;
	u_char sec;
} sceDNAS2LastDate_t;

/* Structure used for unique ID acquisition */
typedef struct sceDNAS2UniqueID {
	unsigned int category;
	void *ptr;
	int *sizep;
	int result;
} sceDNAS2UniqueID_t;

/* Structure used for data record service */
#define RECORDDATA_LEN 128
typedef struct sceDNAS2RecordData {
	char data[RECORDDATA_LEN];
} sceDNAS2RecordData_t;

/* Structure to store DNAS Status */
typedef struct sceDNAS2Status {
	int code;
	int sub_code;
	int progress;
	void *optional;
} sceDNAS2Status_t;

/* Structure to store DNAS Timeout information */
typedef struct sceDNAS2Timeout_info {
	int timeout;
	int priority;
} sceDNAS2TimeoutInfo_t;

/* Network connection type */
#define sceDNAS2_T_INET		1

/* Return values for sceDNAS2Init() */
#define sceDNAS2_MALLOC_FAILED		-1
#define sceDNAS2_MEMALIGN_FAILED	-2
#define sceDNAS2_NOT_SHUTDOWNED 	-7
#define sceDNAS2_INVALID_NG_TYPE 	-9
#define sceDNAS2_INVALID_TIMEOUT	-10

/* sceDNAS2Auth[Install(), DataDownload(), DataDownload2(), GetUniqueID()]
*/
/* Return values for sceDNAS2Auth[NetStart(), RecordData()] */
#define sceDNAS2_NOT_READY			-3

/* Return values for sceDNAS2Auth[NetStart(), RecordData()]
** sceDNAS2Auth[Install(), DataDownload(), DataDownload2(), GetUniqueID()]
** sceDNAS2[Abort(), GetStatus(), SetProxy(), GetProxy()]
*/
#define sceDNAS2_NOT_INITIALIZED 	-8

/* Return values for sceDNAS2AuthData[Download(), Download2()] */
#define sceDNAS2_INVALID_IDSLOT		-4
#define sceDNAS2_INVALID_SIZE		-5

/* Return values for sceDNAS2Abort() */
#define sceDNAS2_NOT_IN_COMM        -6

/* Return values for sceDNAS2GetStatus() */
#define sceDNAS2_OK           0  /* Normal completion */
#define sceDNAS2_NG           -1 /* Abnormal completion */

/* Common return value for all DNAS API */
#define sceDNAS2_INVALID_ARG        -11 /* Invalid argument was given */

/* To be set in code field of sceDNAS2Status_t */
#define sceDNAS2_S_DORMANT    1  /* Uninitialized */
#define sceDNAS2_S_INIT       2  /* Initialization completed */
#define sceDNAS2_S_INST       3  /* Authentication in progress */
#define sceDNAS2_S_DATA       4  /* Data exchange in progress */
#define sceDNAS2_S_END        5  /* Normal completion */
#define sceDNAS2_S_NG         6  /* Abnormal completion */
#define sceDNAS2_S_COM_ERROR  7  /* Communication Error */
#define sceDNAS2_S_DATA_DONE  8  /* Data exchange error(after transaction) */
#define sceDNAS2_S_GETID      9  /* unique ID acquistion in progress */
#define sceDNAS2_S_RECORDDATA 10 /* Data record service in progress */
#define sceDNAS2_S_NETSTART   11 /* Net start authentication in progress */

/* To be set in progress of sceDNAS2Status_t */
#define sceDNAS2_P_NOPROGRESS          0 /* Uninitialized */
#define sceDNAS2_P_CONNECT             1 /* Connection to server */
#define sceDNAS2_P_AUTH                2 /* Authentication in progress */
#define sceDNAS2_P_AUTH_DONE           3 /* Authentication done */
#define sceDNAS2_P_RECORDDATA          4 /* In data record service */
#define sceDNAS2_P_DOWN                5 /* Downloading data */
#define sceDNAS2_P_DOWN_DONE           7 /* Downloading completed */
#define sceDNAS2_P_ID                  8 /* Acquiring unique ID */

/* To be set in result of sceDNAS2UniqueID_t */
#define sceDNAS2_UNIQUE_R_FIRST             1 /* First timer to get unique ID */
#define sceDNAS2_UNIQUE_R_EXIST             2 /* Not a first timer to get unique ID */

/* 
** Post DNAS service operation (in case of code=sceDNAS2_S_END)
*/

/* Macros defined for to get each field in sub_code */
#define sceDNAS2Status_SubCode_TYPE(status) \
           (status.sub_code & 0x7f000000) >> 24
#define sceDNAS2Status_SubCode_INST_RESULT(status) \
           (status.sub_code & 0x0000000c) >> 2

/* sub_code */
#define sceDNAS2_SS_NOSUBCODE          0 /* Uninitialized */

/* Values to be set in type field of sub_code */
#define sceDNAS2_SS_INST_OK            1 /* Installable */
#define sceDNAS2_SS_RECORDDATA_OK      2 /* Data record completed */
#define sceDNAS2_SS_DOWNLOAD_OK        3 /* Download completed */
#define sceDNAS2_SS_GETID_OK           4 /* unique ID acquisition completed */
#define sceDNAS2_SS_DRDATA_OK          5 /* DR DATA acquisition completed */

/* Values to be set in inst_result field of sub_code */
#define sceDNAS2_SC_InstResult_EXIST       0
#define sceDNAS2_SC_InstResult_OK          1

/*
** Errors related to DNAS Service(code=sceDNAS2_S_NG)
**     to be set in error_code
*/
/* Common errors between INSTALL, DOWNLOAD, Unique ID */
#define sceDNAS2_SS_SERVER_BUSY        -101 /* Server is busy */
#define sceDNAS2_SS_BEFORE_SERVICE     -102 /* Title Service yet to be activated */
#define sceDNAS2_SS_OUT_OF_SERVICE     -103 /* Title Service already expired */
#define sceDNAS2_SS_END_OF_SERVICE     -104 /* DNAS Service expired */
#define sceDNAS2_SS_SESSION_TIME_OUT   -105 /* Session Time out */
#define sceDNAS2_SS_INVALID_SERVER     -106 /* Connection to invalid server */
#define sceDNAS2_SS_INTERNAL_ERROR     -107 /* DNAS client library internal error */
#define sceDNAS2_SS_EXTERNAL_ERROR     -108 /* Other server side error */


/* Errors set by DOWNLOAD service */
#define sceDNAS2_SS_DL_NODATA          -201 /* No downloadable data */
#define sceDNAS2_SS_DL_BEFORE_SERVICE  -202 /* Before data exchange service activation */
#define sceDNAS2_SS_DL_OUT_OF_SERVICE  -203 /* Data exchange service expired */
#define sceDNAS2_SS_DL_NOT_UPDATED     -204 /* No new downloadable data */

/* Errors set at client local */
#define sceDNAS2_SS_INVALID_PS2         -401 /* Failed to access machine information */
#define sceDNAS2_SS_INVALID_MEDIA       -402 /* Failed to access local media */
#define sceDNAS2_SS_INVALID_AUTHDATA    -403 /* Something wrong with auth_data */
#define sceDNAS2_SS_INVALID_HDD_BINDING -404 /* PS2 not bound properly to HDD */

/* Other errors  */
#define sceDNAS2_SS_MISMATCH_AUTHDATA -501 /* unmatched auth_data */

/* Errors set by Unique ID service */
#define sceDNAS2_SS_ID_NOUSE           -701 /* Not registerd to unique ID service */
#define sceDNAS2_SS_ID_NOT_JOIN_TO_CAT -703 /* Title not joinged to category */
#define sceDNAS2_SS_ID_INTERNAL_ERROR  -704 /* Internal Unique ID Error */

/*
** Communication Error(code=sceDNAS2_S_COM_ERROR)
**     error value to be set in error_code
 */
/* i-mode communication error 1-99 (please refer netglue_imode.h) */

/* Internet communication error: -500 - -599(please refer netglue_net.h) */

/* reserved error number: -600 - -699 */

/*
** Miscellaneous Error
**
** Error value beyond -800 are reserved for unknown NG error
 */


/* File name of IOP replace image */
#define DNAS_IMAGE_FILE     "DNAS300.IMG"
#define DNAS_IMAGE_file     "dnas300.img"

#ifdef __cplusplus
extern "C" {
#endif

int sceDNAS2Init(sceDNAS2TitleAuthInfo_t *auth_info, int priority, int debug_flag, int ng_type, sceDNAS2TimeoutInfo_t *timeout_info);
int sceDNAS2InitNoHDD(sceDNAS2TitleAuthInfo_t *auth_info, int priority, int debug_flag, int ng_type, sceDNAS2TimeoutInfo_t *timeout_info);
int sceDNAS2Shutdown(void);
int sceDNAS2AuthInstall(void);
int sceDNAS2AuthGetUniqueID(sceDNAS2UniqueID_t *data);
int sceDNAS2AuthNetStart(void);
int sceDNAS2GetStatus(sceDNAS2Status_t *status);
int sceDNAS2Abort(void);

/* system reserved */
int sceDNAS2AuthRecordData(sceDNAS2RecordData_t *data);

int sceDNAS2SetProxy(int activate_flag, const char *new_proxy_host, u_short new_proxy_port);
int sceDNAS2GetProxy(int *activate_flag, char *proxy_host, int len, u_short *proxy_port);

/* erx */
void *sceDNAS2NetGetErxEntries(void);

#ifdef __cplusplus
}
#endif

#endif /* _DNAS2_H_ */

