#ifndef __IOP_USB_H
#define __IOP_USB_H

#include "ioptypes.h"
#include "ps2/iop/usbdefs.hpp"

#define USBDEV_UNKNOWN  (0)
#define USBDEV_MOUSE    (1)
#define USBDEV_KEYBOARD (2)


typedef struct s_usb_mouse_unit
{
    u32 unit_number;
    s32 x,y;
    s32 scroll;
    s32 buttons;
} usb_mouse_unit;

typedef struct s_usb_keyboard_unit
{
    u32 unit_number;
    u8  shift_state;
    u8  led_state;
    u8  last_led_state;
    u8  country_code;
    u32 pressed[256/sizeof(u32)];
} usb_keyboard_unit;

typedef struct s_usb_unit
{
    struct s_usb_unit *pNext;
    s32 device_id;
    s32 location;
    s32 number;
    s32 type;
    s32 control_pipe;
    s32 data_pipe;
    s32 payload_size;
    s32 interface_number;
    s32 alternate_setting;
    union
    {
        usb_mouse_unit      mouse;
        usb_keyboard_unit   keyboard;
    } device;
    u8  data[0];
} usb_unit;

typedef struct s_IopUsbVars
{
    usb_devices             DeviceState;
    s32                     WatcherThreadId;
    usb_unit                *pUnits;
} t_IopUsbVars;

void *usb_Dispatch(u32 Command,void *Data,s32 Size);

extern t_IopUsbVars *g_pUsb;

#endif // __IOP_USB_H