/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: bnnetcnf.h,v 1.8 2003/04/11 10:07:06 ksh Exp $
 */
/* 
 * Emotion Engine Library
 *
 * Copyright (C) 2002 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * bnnetcnf - bnnetcnf.h
 *	"PlayStation BB Navigator" Network Configuration Read Library
 *	header
 *
 *      Date            Design      Log
 *  ----------------------------------------------------
 *      2002-07-17      morita      first version
 *      2002-10-07      tetsu       rewrite bnnetcnf.h
 *      2002-10-12      tetsu       correspond to style-guide
 *      2002-10-15      tetsu       change path of sceerrno.h
 *      2002-10-19      tetsu       correspond to code-review
 */

#ifndef _SCE_BNNETCNF_H
#define _SCE_BNNETCNF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sceerrno.h>

#define SCEBNNETCNF_STR_SIZE_MAX (256)

enum
{
    SCEBNNETCNF_DATA_TYPE_NO_SET = -1, /* 設定しない */
    SCEBNNETCNF_DATA_TYPE_ETH = 1,     /* type eth */
    SCEBNNETCNF_DATA_TYPE_PPP,         /* type ppp */
    SCEBNNETCNF_DATA_TYPE_NIC          /* type nic */
};

enum
{
    SCEBNNETCNF_DATA_MTU_NO_SET = -1,   /* 設定しない */
    SCEBNNETCNF_DATA_MTU_DEFAULT = 1454 /* mtu 1454 */
};

enum
{
    SCEBNNETCNF_DATA_IDLE_TIMEOUT_NO_SET = -1 /* 設定しない */
};

enum
{
    SCEBNNETCNF_DATA_PHY_CONFIG_NO_SET = -1, /* 設定しない */
    SCEBNNETCNF_DATA_PHY_CONFIG_AUTO = 1,    /* phy_config auto */
    SCEBNNETCNF_DATA_PHY_CONFIG_10,          /* phy_config 10 */
    SCEBNNETCNF_DATA_PHY_CONFIG_10_FD,       /* phy_config 10_fd */
    SCEBNNETCNF_DATA_PHY_CONFIG_TX = 5,      /* phy_config tx */
    SCEBNNETCNF_DATA_PHY_CONFIG_TX_FD        /* phy_config tx_fd */
};

enum
{
    SCEBNNETCNF_DATA_DIALING_TYPE_NO_SET = -1, /* 設定しない */
    SCEBNNETCNF_DATA_DIALING_TYPE_TONE = 0,    /* dialing_type tone */
    SCEBNNETCNF_DATA_DIALING_TYPE_PULSE        /* dialing_type pulse */
};

enum
{
    SCEBNNETCNF_DATA_DHCP_NO_SET = 0xff, /* 設定しない */
    SCEBNNETCNF_DATA_DHCP_NO_USE = 0,    /* -dhcp */
    SCEBNNETCNF_DATA_DHCP_USE            /* dhcp */
};

enum
{
    SCEBNNETCNF_DATA_DNS_NEGO_NO_SET = 0xff, /* 設定しない */
    SCEBNNETCNF_DATA_DNS_NEGO_ON = 1         /* want.dns1_nego/want.dns2_nego */
};

enum
{
    SCEBNNETCNF_DATA_F_AUTH_OFF, /* allow.auth chap/pap を設定しない */
    SCEBNNETCNF_DATA_F_AUTH_ON   /* allow.auth chap/pap を設定する */
};

enum
{
    SCEBNNETCNF_DATA_AUTH_CHAP_PAP = 4 /* allow.auth chap/pap */
};

enum
{
    SCEBNNETCNF_DATA_PPPOE_NO_SET = 0xff, /* 設定しない */
    SCEBNNETCNF_DATA_PPPOE_USE = 1        /* pppoe */
};

enum
{
    SCEBNNETCNF_DATA_PRC_NEGO_NO_SET = 0xff, /* 設定しない */
    SCEBNNETCNF_DATA_PRC_NEGO_OFF = 0        /* -want.prc_nego */
};

enum
{
    SCEBNNETCNF_DATA_ACC_NEGO_NO_SET = 0xff, /* 設定しない */
    SCEBNNETCNF_DATA_ACC_NEGO_OFF = 0        /* -want.acc_nego */
};

enum
{
    SCEBNNETCNF_DATA_ACCM_NEGO_NO_SET = 0xff, /* 設定しない */
    SCEBNNETCNF_DATA_ACCM_NEGO_OFF = 0        /* -want.accm_nego */
};

typedef struct SceBnnetcnfData {
    char attach_ifc[SCEBNNETCNF_STR_SIZE_MAX];      /* 接続プロバイダ設定ファイル名(net) */
    char attach_dev[SCEBNNETCNF_STR_SIZE_MAX];      /* 接続機器設定ファイル名(net) */
    char dhcp_host_name[SCEBNNETCNF_STR_SIZE_MAX];  /* DHCP用ホスト名(ifc) */
    char address[SCEBNNETCNF_STR_SIZE_MAX];         /* IPアドレス(ifc) */
    char netmask[SCEBNNETCNF_STR_SIZE_MAX];         /* ネットマスク(ifc) */
    char gateway[SCEBNNETCNF_STR_SIZE_MAX];         /* デフォルトルータ(ifc) */
    char dns1_address[SCEBNNETCNF_STR_SIZE_MAX];    /* プライマリDNS(ifc) */
    char dns2_address[SCEBNNETCNF_STR_SIZE_MAX];    /* セカンダリDNS(ifc) */
    char phone_numbers1[SCEBNNETCNF_STR_SIZE_MAX];  /* 接続先電話番号1(ifc) */
    char phone_numbers2[SCEBNNETCNF_STR_SIZE_MAX];  /* 接続先電話番号2(ifc) */
    char phone_numbers3[SCEBNNETCNF_STR_SIZE_MAX];  /* 接続先電話番号3(ifc) */
    char auth_name[SCEBNNETCNF_STR_SIZE_MAX];       /* ユーザID(ifc) */
    char auth_key[SCEBNNETCNF_STR_SIZE_MAX];        /* パスワード(ifc) */
    char peer_name[SCEBNNETCNF_STR_SIZE_MAX];       /* 接続先の認証名(ifc) */
    char vendor[SCEBNNETCNF_STR_SIZE_MAX];          /* ベンダ名(dev) */
    char product[SCEBNNETCNF_STR_SIZE_MAX];         /* プロダクト名(dev) */
    char chat_additional[SCEBNNETCNF_STR_SIZE_MAX]; /* 追加ATコマンド(dev) */
    char outside_number[SCEBNNETCNF_STR_SIZE_MAX];  /* 外線発信番号設定(番号設定部分)(dev) */
    char outside_delay[SCEBNNETCNF_STR_SIZE_MAX];   /* 外線発信番号設定(遅延設定部分)(dev) */
    int ifc_type;                                   /* デバイスレイヤの種別(ifc) */
    int mtu;                                        /* MTUの設定(ifc) */
    int ifc_idle_timeout;                           /* 回線切断設定(ifc) */
    int dev_type;                                   /* デバイスレイヤの種別(dev) */
    int phy_config;                                 /* イーサネット接続機器の動作モード(dev) */
    int dialing_type;                               /* ダイアル方法(dev) */
    int dev_idle_timeout;                           /* 回線切断設定(dev) */
    int p0;                                         /* 予約領域0 */
    unsigned char dhcp;                             /* DHCP使用・不使用(ifc) */
    unsigned char dns1_nego;                        /* DNSサーバアドレスを自動取得する・しない(ifc) */
    unsigned char dns2_nego;                        /* DNSサーバアドレスを自動取得する・しない(ifc) */
    unsigned char f_auth;                           /* 認証方法の指定有り(ifc) */
    unsigned char auth;                             /* 認証方法(ifc) */
    unsigned char pppoe;                            /* PPPoE使用・不使用(ifc) */
    unsigned char prc_nego;                         /* PFCネゴシエーションの禁止(ifc) */
    unsigned char acc_nego;                         /* ACFCネゴシエーションの禁止(ifc) */
    unsigned char accm_nego;                        /* ACCMネゴシエーションの禁止(ifc) */
    unsigned char p1;                               /* 予約領域1 */
    unsigned char p2;                               /* 予約領域2 */
    unsigned char p3;                               /* 予約領域3 */
    int p4[5];                                      /* 予約領域4 */
} SceBnnetcnfData __attribute__((aligned(64)));

int sceBnnetcnfRead(const char *fsname, SceBnnetcnfData *data,
		    void *(*func_malloc)(size_t), void (*func_free)(void *));

void *sceBnnetcnfGetErxEntries(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SCE_BNNETCNF_H */
