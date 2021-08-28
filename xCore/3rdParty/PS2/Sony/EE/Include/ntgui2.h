/* SCE CONFIDENTIAL
 * "PlayStation 2" Programmer Tool Runtime Library NTGUI package (Release 3.0 version)
 */
/*
 *	Network Setting Application Library 2
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *	                 All Rights Reserved.
 *
 *                          ntgui2.h
 *
 *        Version       Date            Design     Log
 *  --------------------------------------------------------------------
 *        0.10
 */

#ifndef __sceNtgui2_H_
#define __sceNtgui2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <libpkt.h>
#include <libvu0.h>

#define SCE_NTGUI2_STR_SIZE (256)
#define SCE_NTGUI2_DEFAULT_COLOR {{{1,1,1,0},{1,1,1,0},{1,1,1,0},{1,1,1,0}}}

#define SCENTGUI2_MSG_MAX               334


/* マウスの動作ポイント */
enum
{
    SCE_NTGUI2_MOUSE_MESSAGE_TYPE__PRESS = 0,
    SCE_NTGUI2_MOUSE_MESSAGE_TYPE__RELEASE,
    SCE_NTGUI2_MOUSE_MESSAGE_TYPE__MOVE,
};

/* ソフトウェアキーボードのキー状態設定タイプ */
enum
{
    SCE_NTGUI2_ENABLE_KEY_TYPE_ENABLE_LISTED_AND_DISABLE_NOTLISTED = 0,
    SCE_NTGUI2_ENABLE_KEY_TYPE_ENABLE_ALL,
    SCE_NTGUI2_ENABLE_KEY_TYPE_DISABLE_LISTED,
};

/* ソフトウェアキーボードの入力欄タイプ */
enum
{
    SCE_NTGUI2_SKB_CONFIG_NAME=0,
    SCE_NTGUI2_SKB_USER_NAME,
    SCE_NTGUI2_SKB_PASSWORD,
    SCE_NTGUI2_SKB_DHCP_HOSTNAME,

    SCE_NTGUI2_SKB_IP_ADDRESS,
    SCE_NTGUI2_SKB_NETMASK,
    SCE_NTGUI2_SKB_GATEWAY,
    SCE_NTGUI2_SKB_PRIMARY_DNS,
    SCE_NTGUI2_SKB_SECONDARY_DNS,

    SCE_NTGUI2_SKB_AT_COMMAND,
    SCE_NTGUI2_SKB_OUTSIDE_LINE,
    SCE_NTGUI2_SKB_TIMEOUT,

    SCE_NTGUI2_SKB_TEL1,
    SCE_NTGUI2_SKB_TEL2,
    SCE_NTGUI2_SKB_TEL3,
};

/* マウスのボタン状態 */
enum
{
    SCE_NTGUI2_MOUSE_BUTTON_LEFT   = 1<<0,
    SCE_NTGUI2_MOUSE_BUTTON_RIGHT  = 1<<1,
    SCE_NTGUI2_MOUSE_BUTTON_MIDDLE = 1<<2,
};

/* 起動オプション */
enum
{
    SCE_NTGUI2_FLAG_USE_HDD           = 1<<0,
    SCE_NTGUI2_FLAG_USE_USB_MOUSE     = 1<<1,
    SCE_NTGUI2_FLAG_USE_USB_KB        = 1<<2,
    SCE_NTGUI2_FLAG_PAD_REVERSE       = 1<<3,
    SCE_NTGUI2_FLAG_MC_SLOT1_ONLY     = 1<<4,
    SCE_NTGUI2_FLAG_MENU_BTN_SWING    = 1<<5,
};

/* dialing_type */
enum
{
    SCE_NTGUI2_DIALINGTYPE_TONE = 0,
    SCE_NTGUI2_DIALINGTYPE_PULSE,
};

/* type */
enum
{
    SCE_NTGUI2_TYPE_ETH = 1,
    SCE_NTGUI2_TYPE_PPP,
    SCE_NTGUI2_TYPE_NIC,
};

/* phy_config */
enum
{
    SCE_NTGUI2_PHYCONFIG_AUTO  = 1,
    SCE_NTGUI2_PHYCONFIG_10    = 2,
    SCE_NTGUI2_PHYCONFIG_10_FD = 3,
    SCE_NTGUI2_PHYCONFIG_TX    = 5,
    SCE_NTGUI2_PHYCONFIG_TX_FD = 6,
};

/* dhcp */
enum
{
    SCE_NTGUI2_NOUSE_DHCP = 0,
    SCE_NTGUI2_USE_DHCP,
};

/* pppoe */
enum
{
    SCE_NTGUI2_NOUSE_PPPOE = 0,
    SCE_NTGUI2_USE_PPPOE,
};

/* ボタンガイド表示文字列 */
enum
{
	SCE_NTGUI2_BTNGD_STR_NULL=0,
	SCE_NTGUI2_BTNGD_STR_BACK,
	SCE_NTGUI2_BTNGD_STR_ENTER,
	SCE_NTGUI2_BTNGD_STR_INPUT,
	SCE_NTGUI2_BTNGD_STR_DELETE,
	SCE_NTGUI2_BTNGD_STR_OPSETTING,
	SCE_NTGUI2_BTNGD_STR_OPVIEW,
	SCE_NTGUI2_BTNGD_STR_NEXT,
	SCE_NTGUI2_BTNGD_STR_TRANS,
	SCE_NTGUI2_BTNGD_STR_SPACE,
	SCE_NTGUI2_BTNGD_STR_DISPLAY,
};

/* BGのスクロール方向 */
enum
{
	SCE_NTGUI2_BG_DIRECTION_UP    = 0,
	SCE_NTGUI2_BG_DIRECTION_LEFT  = 1,
	SCE_NTGUI2_BG_DIRECTION_DOWN  = 2,
	SCE_NTGUI2_BG_DIRECTION_RIGHT = 3,
};

/* キーボードの入力モード */
enum
{
	SCE_NTGUI2_KEY_MODE_ALPHABET = 0,
	SCE_NTGUI2_KEY_MODE_RHIRAGANA,
	SCE_NTGUI2_KEY_MODE_HIRAGANA,
	SCE_NTGUI2_KEY_MODE_RKATAKANA,
	SCE_NTGUI2_KEY_MODE_KATAKANA,
};

enum
{
	SCE_NTGUI2_VIDEO_MODE_NTSC = 0,
	SCE_NTGUI2_VIDEO_MODE_PAL,
};


typedef void * (*sceNtgui2Callback_Malloc)( size_t size );
typedef void * (*sceNtgui2Callback_Memalign)( size_t align, size_t size );
typedef void * (*sceNtgui2Callback_Realloc)( void * old_ptr, size_t new_size );
typedef void   (*sceNtgui2Callback_Free)( void * ptr );
typedef void   (*sceNtgui2Callback_SKBInit)(void);
typedef void   (*sceNtgui2Callback_SKBDestroy)(void);
typedef void   (*sceNtgui2Callback_SKBOpen)( int input_type, char* str );
typedef void   (*sceNtgui2Callback_SKBClose)( char* str );
typedef void * (*sceNtgui2Callback_SKBGetVif1PktTopAddr)(void);
typedef void   (*sceNtgui2Callback_SKBGetStatus)( int * w, int * h );
typedef int    (*sceNtgui2Callback_SKBSendMouseMessage)( int type, int x, int y );
typedef void   (*sceNtgui2Callback_SKBEnableKey)( int type, unsigned char * keynames[], int keynames_size );
typedef void   (*sceNtgui2Callback_SKBEveryFrame)(void);
typedef void   (*sceNtgui2Callback_UsbKBChangeMode )(int mode);
typedef void   (*sceNtgui2Callback_UsbMouseRead )( int * delta_x, int * delta_y, int * buttons, int * wheel );
typedef void   (*sceNtgui2Callback_UsbKbRead )(void);
typedef void   (*sceNtgui2Callback_PadRead )( unsigned int * paddata );

typedef void   (*sceNtgui2Callback_SOUNDInit)(void);
typedef void   (*sceNtgui2Callback_SOUNDDestroy)(void);
typedef void   (*sceNtgui2Callback_SOUNDEveryFrame)(void);

typedef void   (*sceNtgui2Callback_SOUNDPlayLaunch)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlaySlide)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayGoSub)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayRetSub)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayPushBtn)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayFocusBtn)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayMsgboxOpen_Normal)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayMsgboxClose_Normal)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayMsgboxOpen_Wait)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayMsgboxClose_Wait)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayMsgboxOpen_Warning)(void);
typedef void   (*sceNtgui2Callback_SOUNDPlayMsgboxClose_Warning)(void);

typedef struct sceNtgui2_Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} sceNtgui2_Color_t;

typedef struct sceNtgui2_Color4 {
    sceNtgui2_Color_t aColor[4];
} sceNtgui2_Color4_t;

typedef struct sceNtgui2DefaultData {
    char address[SCE_NTGUI2_STR_SIZE];         /* IPアドレス                       */
    char netmask[SCE_NTGUI2_STR_SIZE];         /* ネットマスク                     */
    char gateway[SCE_NTGUI2_STR_SIZE];         /* デフォルトルータアドレス         */
    char dns1_address[SCE_NTGUI2_STR_SIZE];    /* プライマリDNSサーバアドレス      */
    char dns2_address[SCE_NTGUI2_STR_SIZE];    /* セカンダリDNSサーバアドレス      */
    char phone_numbers1[SCE_NTGUI2_STR_SIZE];  /* 接続先電話番号1                  */
    char phone_numbers2[SCE_NTGUI2_STR_SIZE];  /* 接続先電話番号2                  */
    char phone_numbers3[SCE_NTGUI2_STR_SIZE];  /* 接続先電話番号3                  */
    char auth_name[SCE_NTGUI2_STR_SIZE];       /* ユーザID                         */
    char auth_key[SCE_NTGUI2_STR_SIZE];        /* パスワード                       */
    char chat_additional[SCE_NTGUI2_STR_SIZE]; /* 追加ATコマンド                   */
    char outside_number[SCE_NTGUI2_STR_SIZE];  /* 外線発信番号設定                 */
    char outside_delay[SCE_NTGUI2_STR_SIZE];   /* 外線発信番号設定                 */
    char dhcp_host_name[SCE_NTGUI2_STR_SIZE];  /* DHCP用ホスト名                   */
    int dialing_type;                          /* ダイアル方法                     */
    int phy_config;                            /* イーサネット接続機器の動作モード */
    int idle_timeout;                          /* 回線切断設定                     */
    unsigned char dhcp;                        /* DHCP使用・不使用                 */
    unsigned char pppoe;                       /* PPPoE使用・不使用                */
} sceNtgui2DefaultData_t;

typedef struct  sceNtgui2FontInfo {
	char  name[64];
	short max_ascent16;
	short max_descent16;
} sceNtgui2FontInfo_t;

typedef struct sceNtgui2FontColor1 {
	unsigned char r, g, b, a;
} sceNtgui2FontColor1_t;

typedef struct sceNtgui2FontColor4{
	sceNtgui2FontColor1_t aColor[4];
} sceNtgui2FontColor4_t;

typedef struct sceNtgui2FontRect{
	float top;
	float bottom;
	float left;
	float right;
} sceNtgui2FontRect_t;

typedef struct sceNtgui2FontCharInfo
{
	short base_x;
	short base_y;
	short l_bearing;
	short r_bearing;
	short ascent;
	short descent;
	short width;
	short kerning;
} sceNtgui2FontCharInfo_t;

typedef struct sceNtgui2FontCallback {
	float (*GetRatio)( void );
	void (*Create)( void );
	void (*Destroy)( void );
	int  (*Open)( void );
	void (*Close)( void );
	int (*GetInfo)( sceNtgui2FontInfo_t* );
	int (*GetCharInfo)( unsigned int , sceNtgui2FontCharInfo_t*  );
	int (*GetRect)( float, int , const unsigned char*, sceNtgui2FontRect_t* );
	void (*SetLocate)( int, int );
	int  (*GetLocateX)(void);
	int  (*GetLocateY)(void);
	void (*SetRGBA4)( sceNtgui2FontColor4_t* );
	void (*Flip)( void );
	sceVif1Packet* 
	(*DrawText)( float fScale, int X16, int Y16, int Pitch16,
				 int Start, int End,
				 const unsigned char *pString );
} sceNtgui2FontCallback_t;

typedef struct sceNtgui2_Arg
{
	int size;
	const char *pMessageList[SCENTGUI2_MSG_MAX];
	/* initial thread priority */
	int threadPrio;
	int videomode;
    /* 起動オプション */
    int flag;

    /* v-blank 開始を待つセマフォ */
    int _sema_vsync;

	/* semaphore used to wait for dma transfer to finish */
	int _sema_dmawait;

    /* 追加時のデフォルトデータへのポインタ */
    sceNtgui2DefaultData_t *default_env_data;

    /* メモリアロケーション コールバック関数ポインタ */
    sceNtgui2Callback_Malloc               cb_malloc;
    sceNtgui2Callback_Memalign             cb_memalign;
    sceNtgui2Callback_Realloc              cb_realloc;
    sceNtgui2Callback_Free                 cb_free;

    /* ソフトウェアキーボード コールバック関数ポインタ */
    sceNtgui2Callback_SKBInit              cb_skb_init;
    sceNtgui2Callback_SKBDestroy           cb_skb_destroy;
    sceNtgui2Callback_SKBOpen              cb_skb_open;
    sceNtgui2Callback_SKBClose             cb_skb_close;
    sceNtgui2Callback_SKBGetVif1PktTopAddr cb_skb_getvif1pkttopaddr;
    sceNtgui2Callback_SKBGetStatus         cb_skb_getstatus;
    sceNtgui2Callback_SKBSendMouseMessage  cb_skb_sendmousemessage;
    sceNtgui2Callback_SKBEnableKey         cb_skb_enablekey;
    sceNtgui2Callback_SKBEveryFrame        cb_skb_everyframe;

    /* キーボード 入力モード切換えコールバック関数ポインタ */
    sceNtgui2Callback_UsbKBChangeMode      cb_kb_changemode;

    /* USBマウスの 入力を受ける関数へのポインタ */
    sceNtgui2Callback_UsbMouseRead         cb_mouse_read;

    /* パッドの 入力を受ける関数へのポインタ */
    sceNtgui2Callback_PadRead              cb_pad_read;

    /* USBキーボードの入力を受ける関数へのポインタ */
    sceNtgui2Callback_UsbKbRead            cb_kb_read;

	/* システムのアスペクト比の設定を使用しない場合のアスペクト比の値 */
	int set_aspect;

	/* 言語の設定 */
	int set_language;

	/* Sound関係のコールバック関数へのポインタ */
    sceNtgui2Callback_SOUNDInit                    cb_sound_init;
	sceNtgui2Callback_SOUNDDestroy                 cb_sound_destroy;
    sceNtgui2Callback_SOUNDEveryFrame              cb_sound_everyframe;

	/* Sound */
	/* 起動 */
	sceNtgui2Callback_SOUNDPlayLaunch              cb_sound_play_launch;
	/* ページスライド(ガイドモード) */
	sceNtgui2Callback_SOUNDPlaySlide               cb_sound_play_slide;
	/* ページチェンジ(詳細画面などに移るとき) */
	sceNtgui2Callback_SOUNDPlayGoSub               cb_sound_play_gosub;
	/* ページチェンジ(詳細画面などから戻るとき) */
	sceNtgui2Callback_SOUNDPlayRetSub              cb_sound_play_retsub;
	/* プッシュ ボタン */
	sceNtgui2Callback_SOUNDPlayPushBtn             cb_sound_play_push_btn;
	/* フォーカス移動 */
	sceNtgui2Callback_SOUNDPlayFocusBtn            cb_sound_play_focus_btn;
	/* メッセージボックス オープン クローズ */
	sceNtgui2Callback_SOUNDPlayMsgboxOpen_Normal   cb_sound_play_msgbox_open_normal;
	sceNtgui2Callback_SOUNDPlayMsgboxClose_Normal  cb_sound_play_msgbox_close_normal;
	sceNtgui2Callback_SOUNDPlayMsgboxOpen_Wait     cb_sound_play_msgbox_open_wait;
	sceNtgui2Callback_SOUNDPlayMsgboxClose_Wait    cb_sound_play_msgbox_close_wait;
	sceNtgui2Callback_SOUNDPlayMsgboxOpen_Warning  cb_sound_play_msgbox_open_warning;
	sceNtgui2Callback_SOUNDPlayMsgboxClose_Warning cb_sound_play_msgbox_close_warning;

    /* 背景をファイルからから読み込むためのパス */
    char * str_path_bg;
    char * str_path_bg2;

	/* 背景のアルファの有無 */
	int bg_has_alpha;
	int bg_has_alpha2;

	/* 背景スクロールの有無 */
	int bg_scroll_do_flag;
	int bg_scroll_do_flag2;
	/* 背景のスクロールする方向 */
	int bg_scroll_direction;
	int bg_scroll_direction2;
	/* 背景スクロールの速度 */
	int bg_scroll_speed;
	int bg_scroll_speed2;

    /* 色 */
    sceNtgui2_Color4_t       color_pointer;                         /* ポインタ */
    sceNtgui2_Color4_t       color_pointer_center;                  /* ポインタの中央 */
    sceNtgui2_Color4_t       color_pointer_select;                  /* ポインタの選択部分 */
    sceNtgui2_Color4_t       color_select_cursor;                   /* 選択カーソル */
    sceNtgui2_Color4_t       color_decision_cursor;                 /* 決定カーソル */
    sceNtgui2_Color4_t       color_font;                            /* フォント */
    sceNtgui2_Color4_t       color_scrollbar_slider;                /* スクロールバーのスライダー */
    sceNtgui2_Color4_t       color_msgbox_back;                     /* ダイアログ表示時の背景 */
    sceNtgui2_Color4_t       color_msgbox_normal_title;             /* 正常ダイアログのタイトル */
    sceNtgui2_Color4_t       color_msgbox_normal_button;            /* 正常ダイアログのボタン */
    sceNtgui2_Color4_t       color_msgbox_normal_select;            /* 正常ダイアログの選択ボタン */
    sceNtgui2_Color4_t       color_msgbox_normal_bg;                /* 正常ダイアログの背景 */
    sceNtgui2_Color4_t       color_msgbox_warning_title;            /* エラーダイアログのタイトル */
    sceNtgui2_Color4_t       color_msgbox_warning_button;           /* エラーダイアログのボタン */
    sceNtgui2_Color4_t       color_msgbox_warning_select;           /* エラーダイアログの選択ボタン */
    sceNtgui2_Color4_t       color_msgbox_warning_bg;               /* エラーダイアログの背景 */
    sceNtgui2_Color4_t       color_msgbox_wait_title;               /* 処理中ダイアログのタイトル */
    sceNtgui2_Color4_t       color_msgbox_wait_button;              /* 処理中ダイアログのボタン */
    sceNtgui2_Color4_t       color_msgbox_wait_select;              /* 処理中ダイアログの選択ボタン */
    sceNtgui2_Color4_t       color_msgbox_wait_bg;                  /* 処理中ダイアログの背景 */
    sceNtgui2_Color4_t       color_softkb_bg;                       /* ソフトウェアキーボードの背景 */
    int                      color_transparency;                    /* 半透明パーツの透明度(-5～5 0:default) */
    sceNtgui2FontCallback_t  font_callback;                         /* フォントコールバック構造体 */
	u_char reserved[120];
} sceNtgui2_Arg_t;

void *sceNtgui2GetErxEntries(void);

int sceNtgui2_Do( sceNtgui2_Arg_t * arg );

/* SendKBMessage の 要因 */
enum
{
    SCE_NTGUI2_KBMSG_TYPE_SOFTKB,
    SCE_NTGUI2_KBMSG_TYPE_HARDKB,
    SCE_NTGUI2_KBMSG_TYPE_KBCMD=4,
};

/*  SoftKB */
void sceNtgui2_SendKBMessage( int type, unsigned char * keyname );

void sceNtgui2_SKB_GetClosePos(int* x, int* y);
void sceNtgui2_SKB_SetClosePos(int x, int y);
void sceNtgui2_SKB_GetOpenPos(int* x, int* y);
void sceNtgui2_SKB_SetOpenPos(int x, int y);

/*  Pointer */
void sceNtgui2_SetPointerAlpha(int alpha);
void sceNtgui2_ResetPointerAlpha( void );

void sceNtgui2_SetBtnGuide(int code4kaku, int codeBatsu, int codeMaru, int code3kaku);
void sceNtgui2_ResetBtnGuide(void);



#ifdef __cplusplus
}
#endif

#endif /* __sceNtgui2_H_ */
