#ifdef DEBUG_KEYBOARD
#ifndef XKBD_H
#define XKBD_H
/************************************************************************
*                                                                       *
*   Xkbd.h -- This module defines the Xbox Keyboard API support         *
*                                                                       *
*   IMPORTANT:                                                          *
*   These API's are for use on development platform only to be used as  *
*   a debugging aid.  At this time, there are no plans for keyboard     *
*   support  with shipping Xbox titles.                                 *
*                                                                       *
*   Copyright (c) 2000 - 2001 Microsoft Corp. All rights reserved.      *
*                                                                       *
************************************************************************/

extern      XPP_DEVICE_TYPE XDEVICE_TYPE_DEBUG_KEYBOARD_TABLE;
#define     XDEVICE_TYPE_DEBUG_KEYBOARD (&XDEVICE_TYPE_DEBUG_KEYBOARD_TABLE)
#define     XINPUT_DEVSUBTYPE_KBD_STANDARD  0

// XTL is compiled SINGLE_KEYBOARD_ONLY at this time, therefore it is a required
// definition
#define SINGLE_KEYBOARD_ONLY


//
//  XINPUT_DEBUG_KEYSTROKE
//  Structure filled in by XInputGetKeystroke
//
//      VirtualKey - the virtual key that was pressed.
//      Ascii      - the corresponding ascii value, accounting
//                   for the state of modifier keys and the various
//                   key locks, NULL, if there is no corresponding ascii.
//      Flags      - flags are defined below.  They are a combination of
//                   flags needed to interrupt the key (key-up or key-down,
//                   and key repeat) and modifier keys that may have been
//                   down at the time.
//
typedef struct _XINPUT_DEBUG_KEYSTROKE
{
    BYTE VirtualKey;
    CHAR Ascii;
    BYTE Flags;
} XINPUT_DEBUG_KEYSTROKE, *PXINPUT_DEBUG_KEYSTROKE;

//
//  Flags reported in XINPUT_DEBUG_KEYSTROKE::Flags
//
#define XINPUT_DEBUG_KEYSTROKE_FLAG_CTRL       0x01   //Set if the left or right CTRL key was down
#define XINPUT_DEBUG_KEYSTROKE_FLAG_SHIFT      0x02   //Set if the left or right shift key was down
#define XINPUT_DEBUG_KEYSTROKE_FLAG_ALT        0x04   //Set if the left or right ALT key was down
#define XINPUT_DEBUG_KEYSTROKE_FLAG_CAPSLOCK   0x08   //Set if Caps Lock was set
#define XINPUT_DEBUG_KEYSTROKE_FLAG_NUMLOCK    0x10   //Set if Num Lock was set
#define XINPUT_DEBUG_KEYSTROKE_FLAG_SCROLLLOCK 0x20   //Set if Scroll Lock was set
#define XINPUT_DEBUG_KEYSTROKE_FLAG_KEYUP      0x40   //Set for a key up, clear for a keydown.
#define XINPUT_DEBUG_KEYSTROKE_FLAG_REPEAT     0x80   //Set if this a repeat of key that is held.

//
// XINPUT_DEBUG_KEYQUEUE_PARAMETERS  
//
//  Structure passed to XInputInitKeyboardQueue.
//
//      dwFlags          - see XINPUT_DEBUG_KEYQUEUE_FLAG_XXX
//      dwQueueSize      - size of the queue in keystrokes.
//      dwRepeatDelay    - initial delay (in ms) before repeating a key held down.
//      dwRepeatInterval - delay (in ms) between repeat keystrokes of a key held down.
//
typedef struct _XINPUT_DEBUG_KEYQUEUE_PARAMETERS
{
    DWORD dwFlags;
    DWORD dwQueueSize;
    DWORD dwRepeatDelay;
    DWORD dwRepeatInterval;
} XINPUT_DEBUG_KEYQUEUE_PARAMETERS, *PXINPUT_DEBUG_KEYQUEUE_PARAMETERS;

#define XINPUT_DEBUG_KEYQUEUE_FLAG_KEYDOWN      0x00000001  //Queue keydown events
#define XINPUT_DEBUG_KEYQUEUE_FLAG_KEYREPEAT    0x00000002  //Queue keydown repeat events
#define XINPUT_DEBUG_KEYQUEUE_FLAG_KEYUP        0x00000004  //Queue keyup events
#define XINPUT_DEBUG_KEYQUEUE_FLAG_ASCII_ONLY   0x00000010  //Queue only keys with legitimate ASCII values
#ifdef SINGLE_KEYBOARD_ONLY
#define XINPUT_DEBUG_KEYQUEUE_FLAG_ONE_QUEUE  0x00000020  //Create one queue for all keyboards
                                                      //(the handle passed to XInputGetKeystroke
                                                      // will be ignored).
#endif //SINGLE_KEYBOARD_ONLY

XBOXAPI
DWORD
WINAPI
XInputDebugInitKeyboardQueue(
    IN PXINPUT_DEBUG_KEYQUEUE_PARAMETERS pParameters OPTIONAL
    );
/*++
    Routine Description:
        Sets up a keyboard queue (or keyboard queues) for
        recording keystrokes.
    Arguments:
        pParameters - Parameters for creating queue.  This is optional.
                      If NULL is passed, default settings are used.
                      
    Return Value:
        ERROR_SUCCESS     - on success.
        ERROR_OUTOFMEMORY - if memory could not be allocated for the queue.

    Default Values for pParamteres:
        dwFlags = XINPUT_DEBUG_KEYQUEUE_FLAG_KEYDOWN | XINPUT_DEBUG_KEYQUEUE_FLAG_KEYREPEAT;
        dwQueueSize = 32;
        dwRepeatDelay = 400;
        dwRepeatInterval = 150;
--*/

XBOXAPI
DWORD
WINAPI
XInputDebugGetKeystroke(
#ifndef SINGLE_KEYBOARD_ONLY
    IN HANDLE hDevice,
#endif //SINGLE_KEYBOARD_ONLY
    OUT PXINPUT_DEBUG_KEYSTROKE pKeystroke
    );
/*++
    Routine Description:
        Retrieves a keystroke for a(the) keyqueue.
    Arguments:
    
    !SINGLE_KEYBOARD_ONLY
        hDevice - if the XINPUT_DEBUG_KEYQUEUE_FLAG_ONE_QUEUE was not specified,
                  this must contain the handle of a valid keyboard opened with
                  XInputOpen.  If XINPUT_DEBUG_KEYQUEUE_FLAG_ONE_QUEUE was specified,
                  this value should be NULL.
    END OF !SINGLE_KEYBOARD_ONLY

        pKeystroke - pointer to caller supplied buffer to retrieve the keystroke
                     information.
                      
    Return Value:
       ERROR_SUCCESS - on success
       ERROR_HANDLE_EOF - if the queue is empty
       On failure a value defined in winerror.h

--*/


/******
******* KEY DEFINITIONS
*******
******/

/*
 * mouse buttons are not supported
 */

//#define VK_LBUTTON        0x01
//#define VK_RBUTTON        0x02

#define VK_CANCEL         0x03

//#define VK_MBUTTON        0x04 /* NOT contiguous with L & RBUTTON */
//#define VK_XBUTTON1       0x05 /* NOT contiguous with L & RBUTTON */
//#define VK_XBUTTON2       0x06 /* NOT contiguous with L & RBUTTON */

/*
 * 0x07 : unassigned
 */

#define VK_BACK           0x08
#define VK_TAB            0x09

 /*
 * 0x0A - 0x0B : reserved
 */

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D
#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15  /* old name - should be here for compatibility */
#define VK_HANGUL         0x15
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

/*
 * 0x5E : reserved
 */

#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

/*
 * 0x88 - 0x8F : unassigned
 */

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

/*
 * NEC PC-9800 kbd definitions
 */
#define VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad

/*
 * Fujitsu/OASYS kbd definitions
 */
#define VK_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

/*
 * 0x97 - 0x9F : unassigned
 */

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7

/*
 * 0xB8 - 0xB9 : reserved
 */

#define VK_OEM_1          0xBA   // ';:' for US
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_COMMA      0xBC   // ',' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
#define VK_OEM_PERIOD     0xBE   // '.' any country
#define VK_OEM_2          0xBF   // '/?' for US
#define VK_OEM_3          0xC0   // '`~' for US

/*
 * 0xC1 - 0xD7 : reserved
 */

/*
 * 0xD8 - 0xDA : unassigned
 */

#define VK_OEM_4          0xDB  //  '[{' for US
#define VK_OEM_5          0xDC  //  '\|' for US
#define VK_OEM_6          0xDD  //  ']}' for US
#define VK_OEM_7          0xDE  //  ''"' for US
#define VK_OEM_8          0xDF

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */
#define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define VK_ICO_HELP       0xE3  //  Help key on ICO
#define VK_ICO_00         0xE4  //  00 key on ICO

#define VK_PROCESSKEY     0xE5

#define VK_ICO_CLEAR      0xE6


#define VK_PACKET         0xE7

/*
 * 0xE8 : unassigned
 */


/*
 * 0xFF : reserved
 */

#endif //XKBD_H
#endif //DEBUG_KEYBOARD