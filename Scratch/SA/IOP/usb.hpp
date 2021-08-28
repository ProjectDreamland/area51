#ifndef _USB_HPP
#define _USB_HPP

#include "ps2/iop/usbdefs.hpp"

void usb_Init(void);
void usb_Kill(void);
//
// Reads the keyboard from the iop. This will return a bit-array with a bit set
// if the corresponding key is pressed. This will allow us to do a quick test for
// the input_WasPressed and input_IsPressed functions by doing a quick bitscan. When
// We need to convert that from the bit-array to a keycode, we'll have to scan through
// the array although, that will not be done very often and won't actually be that
// slow.
//
void usb_ReadDevices(usb_devices &devices);

#endif // _USB_HPP