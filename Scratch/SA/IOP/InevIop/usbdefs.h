#ifndef __USB_COMMON_H
#define __USB_COMMON_H

#ifdef TARGET_PS2_IOP
#include "ioptypes.h"
#else
#include "x_types.hpp"
#endif

enum {
    USBCMD_BASE = 0x00030000,
    USBCMD_INIT,
    USBCMD_KILL,
    USBCMD_READKEYBOARD,
    USBCMD_READMOUSE,
};

#define MAX_KEYCODES    (16)

typedef struct s_usb_mouse_response
{
    s32 ButtonState;
    s32 X;
    s32 Y;
    s32 ScrollButton;
} usb_mouse_response;

typedef struct s_usb_keyboard_response
{
    u8  shift_state;
    u8  led_state;
    u32 pressed[256/sizeof(u32)];
} usb_keyboard_response;

#endif
