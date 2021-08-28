#ifndef __USB_COMMON_H
#define __USB_COMMON_H

#ifdef TARGET_PS2_IOP
#include "ioptypes.h"
#else
#include "x_types.hpp"
#endif

#define MAX_USB_MOUSE       2
#define MAX_USB_KEYBOARD    2
enum {
    USBCMD_BASE = 0x00030000,
    USBCMD_INIT,
    USBCMD_KILL,
    USBCMD_READDEVICES,
};

typedef struct s_usb_mouse_response
{
    u8  is_present;
    s32 Buttons;
    s32 X;
    s32 Y;
    s32 Wheel;
} usb_mouse;

typedef struct s_usb_keyboard_response
{
    u8  is_present;
    u8  shift_state;
    u8  led_state;
    u32 pressed[256/sizeof(u32)];
} usb_keyboard;

typedef struct s_usb_devices
{
    usb_mouse       Mouse[MAX_USB_MOUSE];
    usb_keyboard    Keyboard[MAX_USB_KEYBOARD];
} usb_devices;

#endif
