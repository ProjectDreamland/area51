#include "entropy.hpp"
#include "ps2/iop/iop.hpp"
#include "usb.hpp"
//
// Will be moved if USB code is migrated in to main codebase
//
#include "ps2/iop/usbdefs.hpp"

usb_devices device_buffer;

void usb_Init(void)
{
    iop_SendSyncRequest(USBCMD_INIT);
}

void usb_Kill(void)
{
    iop_SendSyncRequest(USBCMD_KILL);
}

void usb_ReadDevices(usb_devices &devices)
{
    iop_SendSyncRequest(USBCMD_READDEVICES,NULL,0,&device_buffer,sizeof(device_buffer));
    x_memcpy(&devices,&device_buffer,sizeof(usb_devices));
}

