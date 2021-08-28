/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 * Emotion Engine Library
 *
 * Copyright (C) 2002-2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * netcnfif - netcnfif.h
 * Netcnf Interface Library
 *
 *      Date            Design      Log
 *  ----------------------------------------------------
 *      2002.01.28      tetsu       First version
 *      2002.02.10      tetsu       Add SCE_NETCNFIF_CHECK_ADDITIONAL_AT
 *                                  Add sceNETCNFIF_TOO_LONG_STR
 *      2002.03.23      tetsu       Add sceNETCNFIF_INVALID_ARG
 *      2002.05.05      tetsu       Add sceNETCNFIF_AOL_CONFIGURATION
 *                                  Add SCE_NETCNFIF_CHECK_SPECIAL_PROVIDER
 *      2002.10.19      tetsu       Integrate sce/ee/include/netcnfif.h
 *      2002.10.31      ksh         Add sceNetcnfifLoadEntryAuto()
 *      2003.01.14      ksh         Add callback & Remake
 *      2003.02.26      ksh         sync added
 *      2003.04.10      ksh         char * to const char *
 *      2003.07.18      ksh         Padding in sceNetcnfifList_t
 *      2004.01.20      ksh         Add sceNETCNFIF_LOAD_ATTACH_ERROR
 */

#ifndef _SCE_NETCNFIF_H
#define _SCE_NETCNFIF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SCE_NETCNFIF_PAD_BASE(x, base) (((x) + ((base) - 1)) & ~((base) - 1))
#define SCE_NETCNFIF_PAD(x)            (SCE_NETCNFIF_PAD_BASE(( x ), 64))

/* Sifrpc 用 */
#define SCE_NETCNFIF_SSIZE                  (4096)
#define SCE_NETCNFIF_INTERFACE              (0x80001101)

/* Sifrpc サービス関数用番号 */
#define SCE_NETCNFIF_GET_COUNT              (0)
#define SCE_NETCNFIF_GET_LIST               (1)
#define SCE_NETCNFIF_LOAD_ENTRY             (2)
#define SCE_NETCNFIF_ADD_ENTRY              (3)
#define SCE_NETCNFIF_EDIT_ENTRY             (4)
#define SCE_NETCNFIF_DELETE_ENTRY           (5)
#define SCE_NETCNFIF_SET_LATEST_ENTRY       (6)
#define SCE_NETCNFIF_DELETE_ALL             (7)
#define SCE_NETCNFIF_CHECK_CAPACITY         (8)
#define SCE_NETCNFIF_CHECK_ADDITIONAL_AT    (9)
#define SCE_NETCNFIF_CHECK_SPECIAL_PROVIDER (10)
#define SCE_NETCNFIF_CALLBACK               (11)
#define SCE_NETCNFIF_CALLBACK_OPEN          (12)
#define SCE_NETCNFIF_CALLBACK_READ          (13)
#define SCE_NETCNFIF_CALLBACK_CLOSE         (14)
#define SCE_NETCNFIF_CALLBACK_END           (15)

#define SCE_NETCNFIF_GET_ADDR               (100)
#define SCE_NETCNFIF_ALLOC_WORKAREA         (101)
#define SCE_NETCNFIF_FREE_WORKAREA          (102)
#define SCE_NETCNFIF_SET_ENV                (103)

/* エラーコード(-20 までは netcnf.irx と同等) */
#define sceNETCNFIF_OK                      (0)
#define sceNETCNFIF_NG                      (-1)
#define sceNETCNFIF_ALLOC_ERROR             (-2)
#define sceNETCNFIF_OPEN_ERROR              (-3)
#define sceNETCNFIF_READ_ERROR              (-4)
#define sceNETCNFIF_WRITE_ERROR             (-5)
#define sceNETCNFIF_SEEK_ERROR              (-6)
#define sceNETCNFIF_REMOVE_ERROR            (-7)
#define sceNETCNFIF_ENTRY_NOT_FOUND         (-8)
#define sceNETCNFIF_INVALID_FNAME           (-9)
#define sceNETCNFIF_INVALID_TYPE            (-10)
#define sceNETCNFIF_INVALID_USR_NAME        (-11)
#define sceNETCNFIF_TOO_MANY_ENTRIES        (-12)
#define sceNETCNFIF_ID_ERROR                (-13)
#define sceNETCNFIF_SYNTAX_ERROR            (-14)
#define sceNETCNFIF_MAGIC_ERROR             (-15)
#define sceNETCNFIF_CAPACITY_ERROR          (-16)
#define sceNETCNFIF_UNKNOWN_DEVICE          (-17)
#define sceNETCNFIF_IO_ERROR                (-18)
#define sceNETCNFIF_TOO_LONG_STR            (-19)
#define sceNETCNFIF_AOL_CONFIGURATION       (-20)
#define sceNETCNFIF_LOAD_ATTACH_ERROR       (-21)

#define sceNETCNFIF_NO_DATA                 (-100)
#define sceNETCNFIF_INVALID_ARG             (-101)

#define sceNETCNFIF_F_MASK                  (0xffff0000)
#define sceNETCNFIF_F_USE_WAITSEMA          (0x00010000)

typedef struct sceNetcnfifArg{
    int data;
    int f_no_decode;
    int type;
    int addr;
    char fname[256];
    char usr_name[256];
    char new_usr_name[256];
} sceNetcnfifArg_t;

enum
{
    sceNetcnfifArg_f_no_decode_off,
    sceNetcnfifArg_f_no_decode_on
};

enum
{
    sceNetcnfifArg_type_net,
    sceNetcnfifArg_type_ifc,
    sceNetcnfifArg_type_dev
};

typedef struct sceNetcnfifList{
    int type;
    int stat;
    char sys_name[256];
    char usr_name[256];
	int padding[14];
} sceNetcnfifList_t __attribute__((aligned(64)));

typedef struct sceNetcnfifData{
    char attach_ifc[256];
    char attach_dev[256];
    char dhcp_host_name[256];
    char address[256];
    char netmask[256];
    char gateway[256];
    char dns1_address[256];
    char dns2_address[256];
    char phone_numbers1[256];
    char phone_numbers2[256];
    char phone_numbers3[256];
    char auth_name[256];
    char auth_key[256];
    char peer_name[256];
    char vendor[256];
    char product[256];
    char chat_additional[256];
    char outside_number[256];
    char outside_delay[256];
    int ifc_type;
    int mtu;
    int ifc_idle_timeout;
    int dev_type;
    int phy_config;
    int dialing_type;
    int dev_idle_timeout;
    int p0;
    unsigned char dhcp;
    unsigned char dns1_nego;
    unsigned char dns2_nego;
    unsigned char f_auth;
    unsigned char auth;
    unsigned char pppoe;
    unsigned char prc_nego;
    unsigned char acc_nego;
    unsigned char accm_nego;
    unsigned char p1;
    unsigned char p2;
    unsigned char p3;
    int p4[5];
} sceNetcnfifData_t __attribute__((aligned(64)));

enum
{
    sceNetcnfifData_type_no_set = -1,
    sceNetcnfifData_type_eth = 1,
    sceNetcnfifData_type_ppp,
    sceNetcnfifData_type_nic
};

enum
{
    sceNetcnfifData_mtu_no_set = -1,
    sceNetcnfifData_mtu_default = 1454
};

enum
{
    sceNetcnfifData_idle_timeout_no_set = -1
};

enum
{
    sceNetcnfifData_phy_config_no_set = -1,
    sceNetcnfifData_phy_config_auto = 1,
    sceNetcnfifData_phy_config_10,
    sceNetcnfifData_phy_config_10_FD,
    sceNetcnfifData_phy_config_TX = 5,
    sceNetcnfifData_phy_config_TX_FD
};

enum
{
    sceNetcnfifData_dialing_type_no_set = -1,
    sceNetcnfifData_dialing_type_tone = 0,
    sceNetcnfifData_dialing_type_pulse
};

enum
{
    sceNetcnfifData_dhcp_no_set = 0xff,
    sceNetcnfifData_dhcp_no_use = 0,
    sceNetcnfifData_dhcp_use
};

enum
{
    sceNetcnfifData_dns_nego_no_set = 0xff,
    sceNetcnfifData_dns_nego_on = 1
};

enum
{
    sceNetcnfifData_f_auth_off,
    sceNetcnfifData_f_auth_on
};

enum
{
    sceNetcnfifData_auth_chap_pap = 4
};

enum
{
    sceNetcnfifData_pppoe_no_set = 0xff,
    sceNetcnfifData_pppoe_use = 1
};

enum
{
    sceNetcnfifData_prc_nego_no_set = 0xff,
    sceNetcnfifData_prc_nego_off = 0
};

enum
{
    sceNetcnfifData_acc_nego_no_set = 0xff,
    sceNetcnfifData_acc_nego_off = 0
};

enum
{
    sceNetcnfifData_accm_nego_no_set = 0xff,
    sceNetcnfifData_accm_nego_off = 0
};

/*** callback ***/
typedef int (*sceNetcnfifOpenFunction)(const char *device, const char *pathname, int flags, int mode, int *filesize);
typedef int (*sceNetcnfifReadFunction)(int fd, const char *device, const char *pathname, void *buf, int offset, int size);
typedef int (*sceNetcnfifCloseFunction)(int fd);

typedef	struct sceNetcnfifCallback {
	int type;
	sceNetcnfifOpenFunction open;
	sceNetcnfifReadFunction read;
	sceNetcnfifCloseFunction close;
} sceNetcnfifCallback_t;

#define	sceNetcnfifCallback_type_any	(0)
#define	sceNetcnfifCallback_type_mc	(1)

int sceNetcnfifCreateCallbackThread(sceNetcnfifCallback_t *pcallback, char *stack, int size, int priority);
int sceNetcnfifDeleteCallbackThread(int tid);
int sceNetcnfifCreateMc2Thread(int priority);
int sceNetcnfifDeleteMc2Thread(int tid);
/*** callback end ***/

/*** erx ***/
void *sceNetcnfifGetErxEntries(void);
void *sceNetifmc2GetErxEntries(void);

/* EE/IOP */
int sceNetcnfifDmaCheck(int id);
void sceNetcnfifDataInit(sceNetcnfifData_t *data);
void sceNetcnfifDataDump(sceNetcnfifData_t *data);

/* EE */
int sceNetcnfifSetup(void);
int sceNetcnfifInit(void);
int sceNetcnfifSync(int mode);
int sceNetcnfifCheck(void);
int sceNetcnfifGetResult(sceNetcnfifArg_t *result);
int sceNetcnfifSetFNoDecode(int f_no_decode);
int sceNetcnfifGetCount(const char *fname, int type);
int sceNetcnfifGetList(const char *fname, int type, sceNetcnfifList_t *addr, int size);
int sceNetcnfifLoadEntry(const char *fname, int type, const char *usr_name, sceNetcnfifData_t *addr);
int sceNetcnfifLoadEntryAuto(const char *fname, int type_with_flags, const char *usr_name, sceNetcnfifData_t *addr);
int sceNetcnfifAddEntry(const char *fname, int type, const char *usr_name);
int sceNetcnfifEditEntry(const char *fname, int type, const char *usr_name, const char *new_usr_name);
int sceNetcnfifDeleteEntry(const char *fname, int type, const char *usr_name);
int sceNetcnfifSetLatestEntry(const char *fname, int type, const char *usr_name);
int sceNetcnfifDeleteAll(const char *fname);
int sceNetcnfifCheckCapacity(const char *fname);
int sceNetcnfifCheckAdditionalAT(const char *additional_at);
int sceNetcnfifCheckSpecialProvider(const char *fname, int type, const char *usr_name);
int sceNetcnfifGetAddr(void);
int sceNetcnfifAllocWorkarea(int size);
int sceNetcnfifFreeWorkarea(void);
int sceNetcnfifSetEnv(int type);
int sceNetcnfifSendIOP(unsigned int data, unsigned int addr, unsigned int size);
int sceNetcnfifConvAuthname(const char *src, char *dst, int dst_size);
int sceNetcnfifTerm(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SCE_NETCNFIF_H */
