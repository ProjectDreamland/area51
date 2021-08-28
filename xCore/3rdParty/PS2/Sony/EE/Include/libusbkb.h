#ifndef INCLUDE_LIBUSBKB_H
#define INCLUDE_LIBUSBKB_H

/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *      USB Keyboard Library (for EE)
 *
 *                          Version 1.30
 *                          Shift-JIS
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                              libusbkb.h
 *
 *        Version       Date            Design     Log
 *  --------------------------------------------------------------------
 *      0.10            Dec,11,2000     fukunaga   USB Keyboard Library
 *      1.00            Feb,5,2001      fukunaga   First release version
 *      1.01            Mar,29,2001     fukunaga   Bug fix (Ring buffer)
 *      1.10            Apr,9,2001      fukunaga   sceUsbKbSetReadMode()
 *      1.20            June,21,2001    fukunaga   sceUsbKbClearRbuf()
 *      1.21            June,28,2001    fukunaga   Bug fix (sceUsbKbEnd)
 *      1.22            Feb,18,2002     fukunaga   printf -> scePrintf
 *      1.23            May,22,2002     fukunaga   sceUsbKbCnvRawCode() CAPS+Shift
 *	1.30		June,18,2002	fukunaga   Remove malloc()
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef _eetypes_h_
#include <sys/types.h>
#endif


/*----- Structure -----*/

#define USBKB_MAX_STATUS 127

typedef struct {
  int max_connect;
  int now_connect;
  u_char status[USBKB_MAX_STATUS];
} USBKBINFO_t;

#define USBKB_MAX_KEYCODES 62

typedef struct {
  u_int   led;
  u_int   mkey;
  int     len;
  u_short keycode[USBKB_MAX_KEYCODES];
} USBKBDATA_t;


/*----- MACRO -----*/

#define USBKB_OK     0
#define USBKB_NG     (-1)
#define USBKB_E_SIF  (-2)   /* SIF error */
#define USBKB_E_PAR1 (-11)  /* first parameter error */
#define USBKB_E_PAR2 (-12)  /* second parameter error */

/* sceUsbKbSync関数 */
#define USBKB_WAIT     0
#define USBKB_NO_WAIT  1
#define USBKB_DONE     0
#define USBKB_EXEC     1

/* USBKBDATA_t の led で使用 */
#define USBKB_LED_NUM_LOCK    (1U<<0)  /* 0:OFF 1:ON */
#define USBKB_LED_CAPS_LOCK   (1U<<1)  /* 0:OFF 1:ON */
#define USBKB_LED_SCROLL_LOCK (1U<<2)  /* 0:OFF 1:ON */
#define USBKB_LED_COMPOSE     (1U<<3)  /* 0:OFF 1:ON */
#define USBKB_LED_KANA        (1U<<4)  /* 0:OFF 1:ON */

/* USBKBDATA_t の mkey で使用 */
#define USBKB_MKEY_L_CTRL   (1U<<0)  /* 0:Release 1:Push */
#define USBKB_MKEY_L_SHIFT  (1U<<1)  /* 0:Release 1:Push */
#define USBKB_MKEY_L_ALT    (1U<<2)  /* 0:Release 1:Push */
#define USBKB_MKEY_L_WIN    (1U<<3)  /* 0:Release 1:Push */
#define USBKB_MKEY_R_CTRL   (1U<<4)  /* 0:Release 1:Push */
#define USBKB_MKEY_R_SHIFT  (1U<<5)  /* 0:Release 1:Push */
#define USBKB_MKEY_R_ALT    (1U<<6)  /* 0:Release 1:Push */
#define USBKB_MKEY_R_WIN    (1U<<7)  /* 0:Release 1:Push */
/*
   ※Macintosh用キーボードの場合、ALT と WIN は、
   それぞれ OPTION と APPLE になります。
*/

/* sceUsbKbSetLEDMode() で使用するマクロ */
#define USBKB_LED_MODE_MANUAL 0
#define USBKB_LED_MODE_AUTO1  1
#define USBKB_LED_MODE_AUTO2  2

/* sceUsbdKbCodeType() で使用するマクロ */
#define USBKB_CODETYPE_RAW    0
#define USBKB_CODETYPE_ASCII  1

/* sceUsbKbSetArrangement() で使用するマクロ */
#define USBKB_ARRANGEMENT_101      0
#define USBKB_ARRANGEMENT_106      1
#define USBKB_ARRANGEMENT_106_KANA 2

/* sceUsbKbSetReadMode() で使用するマクロ */
#define USBKB_RMODE_INPUTCHAR 0
#define USBKB_RMODE_PACKET    1

/* Keycode flag */
#define USBKB_RAWDAT 0x8000U  /* ASCIIコードに変換できないコード */
#define USBKB_KEYPAD 0x4000U  /* キーパッドコード */

/* ******* ASCII Code に変換できないキーコード ********/
/*
  ※1 USBKB_RAWDAT ビットが立っているときに以下のようなコードが来ます。
  ※2 下記以外のコードが来ることもあります。
*/
#define USBKEYC_NO_EVENT    0x00
#define USBKEYC_E_ROLLOVER  0x01
#define USBKEYC_E_POSTFAIL  0x02
#define USBKEYC_E_UNDEF     0x03
#define USBKEYC_ESCAPE      0x29
#define USBKEYC_106_KANJI   0x35  /* 半角／全角 漢字 */
#define USBKEYC_CAPS_LOCK   0x39
#define USBKEYC_F1          0x3a
#define USBKEYC_F2          0x3b
#define USBKEYC_F3          0x3c
#define USBKEYC_F4          0x3d
#define USBKEYC_F5          0x3e
#define USBKEYC_F6          0x3f
#define USBKEYC_F7          0x40
#define USBKEYC_F8          0x41
#define USBKEYC_F9          0x42
#define USBKEYC_F10         0x43
#define USBKEYC_F11         0x44
#define USBKEYC_F12         0x45
#define USBKEYC_PRINTSCREEN 0x46
#define USBKEYC_SCROLL_LOCK 0x47
#define USBKEYC_PAUSE       0x48
#define USBKEYC_INSERT      0x49
#define USBKEYC_HOME        0x4a
#define USBKEYC_PAGE_UP     0x4b
#define USBKEYC_DELETE      0x4c
#define USBKEYC_END         0x4d
#define USBKEYC_PAGE_DOWN   0x4e
#define USBKEYC_RIGHT_ARROW 0x4f
#define USBKEYC_LEFT_ARROW  0x50
#define USBKEYC_DOWN_ARROW  0x51
#define USBKEYC_UP_ARROW    0x52
#define USBKEYC_NUM_LOCK    0x53
#define USBKEYC_APPLICATION 0x65  /* アプリケーションキー */
#define USBKEYC_KANA        0x88  /* カタカナ／ひらがな／ローマ字 */
#define USBKEYC_HENKAN      0x8a  /* 前候補／変換／全候補 */
#define USBKEYC_MUHENKAN    0x8b  /* 無変換 */


/* ******* 代表的な生コード *******/
/*
  ※1 生コードでよく使われるものを定義しています。
      上記の「ASCII Code に変換できないキーコード」と併せてお使いください。
  ※2 下記以外のコードが来ることもあります。
*/
#define USBKEYC_A           0x04
#define USBKEYC_B           0x05
#define USBKEYC_C           0x06
#define USBKEYC_D           0x07
#define USBKEYC_E           0x08
#define USBKEYC_F           0x09
#define USBKEYC_G           0x0A
#define USBKEYC_H           0x0B
#define USBKEYC_I           0x0C
#define USBKEYC_J           0x0D
#define USBKEYC_K           0x0E
#define USBKEYC_L           0x0F
#define USBKEYC_M           0x10
#define USBKEYC_N           0x11
#define USBKEYC_O           0x12
#define USBKEYC_P           0x13
#define USBKEYC_Q           0x14
#define USBKEYC_R           0x15
#define USBKEYC_S           0x16
#define USBKEYC_T           0x17
#define USBKEYC_U           0x18
#define USBKEYC_V           0x19
#define USBKEYC_W           0x1A
#define USBKEYC_X           0x1B
#define USBKEYC_Y           0x1C
#define USBKEYC_Z           0x1D
#define USBKEYC_1           0x1E
#define USBKEYC_2           0x1F
#define USBKEYC_3           0x20
#define USBKEYC_4           0x21
#define USBKEYC_5           0x22
#define USBKEYC_6           0x23
#define USBKEYC_7           0x24
#define USBKEYC_8           0x25
#define USBKEYC_9           0x26
#define USBKEYC_0           0x27
#define USBKEYC_ENTER       0x28
#define USBKEYC_ESC         0x29
#define USBKEYC_BS          0x2A
#define USBKEYC_TAB         0x2B
#define USBKEYC_SPACE       0x2C
#define USBKEYC_MINUS       0x2D
#define USBKEYC_EQUAL_101             0x2E  /* = and + */
#define USBKEYC_ACCENT_CIRCONFLEX_106 0x2E  /* ^ and ~ */
#define USBKEYC_LEFT_BRACKET_101      0x2F  /* [ */
#define USBKEYC_ATMARK_106            0x2F  /* @ */
#define USBKEYC_RIGHT_BRACKET_101     0x30  /* ] */
#define USBKEYC_LEFT_BRACKET_106      0x30  /* [ */
#define USBKEYC_BACKSLASH_101         0x31  /* \ and | */
#define USBKEYC_RIGHT_BRACKET_106     0x32  /* ] */
#define USBKEYC_SEMICOLON             0x33  /* ; */
#define USBKEYC_QUOTATION_101         0x34  /* ' and " */
#define USBKEYC_COLON_106             0x34  /* : and * */
#define USBKEYC_COMMA       0x36
#define USBKEYC_PERIOD      0x37
#define USBKEYC_SLASH       0x38
#define USBKEYC_CAPS_LOCK   0x39
#define USBKEYC_KPAD_NUMLOCK   0x53
#define USBKEYC_KPAD_SLASH     0x54
#define USBKEYC_KPAD_ASTERISK  0x55
#define USBKEYC_KPAD_MINUS     0x56
#define USBKEYC_KPAD_PLUS      0x57
#define USBKEYC_KPAD_ENTER     0x58
#define USBKEYC_KPAD_1         0x59
#define USBKEYC_KPAD_2         0x5A
#define USBKEYC_KPAD_3         0x5B
#define USBKEYC_KPAD_4         0x5C
#define USBKEYC_KPAD_5         0x5D
#define USBKEYC_KPAD_6         0x5E
#define USBKEYC_KPAD_7         0x5F
#define USBKEYC_KPAD_8         0x60
#define USBKEYC_KPAD_9         0x61
#define USBKEYC_KPAD_0         0x62
#define USBKEYC_KPAD_PERIOD    0x63
#define USBKEYC_BACKSLASH_106  0x87
#define USBKEYC_YEN_106        0x89


/*----- Prototype -----*/

extern int sceUsbKbInit(int *max_connect);
extern int sceUsbKbEnd(void);
extern int sceUsbKbGetInfo(USBKBINFO_t *info);
extern int sceUsbKbRead(u_int no,USBKBDATA_t *data);
extern int sceUsbKbGetLocation(int no,u_char *location);
extern int sceUsbKbSetLEDStatus(int no, u_char led);
extern int sceUsbKbSetLEDMode(int no, int mode);
extern int sceUsbKbSetRepeat(int no, int sta_time, int interval);
extern int sceUsbKbSetCodeType(int no, int type);
extern int sceUsbKbSetArrangement(int no, int arrange);
extern int sceUsbKbSync(int mode, int *result);
extern u_short sceUsbKbCnvRawCode(int arrange, u_int mkey, u_int led, u_short rawcode);
extern int sceUsbKbSetReadMode(int no, int rmode);
extern int sceUsbKbClearRbuf(u_int no);

void *sceUsbKbGetErxEntries(void);

#ifdef __cplusplus
              }
#endif
#endif
