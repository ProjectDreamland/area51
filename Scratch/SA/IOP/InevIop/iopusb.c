#include "ioptypes.h"
#include "iopusb.h"
#include <kernel.h>
#include <usb.h>
#include <usbd.h>
#include "iopmain.h"

// USB support module. If no keyboard or mouse
// are present, it should do nothing.
// Define this to enable keyboard & Mouse

//#define USB_DEBUG

#ifdef ENABLE_USB_DEVICES

t_IopUsbVars *g_pUsb=NULL;

s32 usb_probe(s32 dev_id);
s32 usb_attach(s32 dev_id);
s32 usb_detach(s32 dev_id);

sceUsbdLddOps MouseLddOps =
{
    NULL,NULL,
    "usbmouse",
    usb_probe,
    usb_attach,
    usb_detach,
};

sceUsbdLddOps KeyboardLddOps =
{
    NULL,NULL,
    "usbkeybd",
    usb_probe,
    usb_attach,
    usb_detach,
};

#include "keyscancodes.h"

void data_transfer(usb_unit *unit);
//-------------------------------------------------
//---- Helper functions
//-------------------------------------------------
//-----------------------------------------------------------------------------
usb_unit *unit_alloc(s32 dev_id,s32 payload_size,s32 interface_number,s32 alternate_setting)
{
    usb_unit *pUnit,*pUnitList,*pUnitLast;
    u8      locations[8];
    s32     i;
    s32     status,location;

    status = sceUsbdGetDeviceLocation(dev_id,locations);
    if (status != sceUsbd_NOERR)
    {
        iop_DebugMsg("usb_attach: Unable to get device location.\n");
        return NULL;
    }

    location = 0;
    
    for (i=0;i<7;i++)
    {
        location = (location << 4) | (locations[i] & 0x0f);
    }

    pUnit = iop_Malloc(sizeof(usb_unit) + payload_size);
    if (!pUnit)
        return NULL;

    memset(pUnit,0,sizeof(usb_unit)+payload_size);

    pUnit->device_id            = dev_id;
    pUnit->payload_size         = payload_size;
    pUnit->interface_number     = interface_number;
    pUnit->alternate_setting    = alternate_setting;
    pUnit->location             = location;
    //
    // Now insert it in to the unit list in order of location
    //

    pUnitLast = NULL;
    pUnitList = g_pUsb->pUnits;

    while (pUnitList)
    {
        if (pUnitList->location > location)
            break;
        pUnitLast = pUnitList;
        pUnitList = pUnitList->pNext;
    }

    if (pUnitLast)
    {
        pUnit->pNext = pUnitLast->pNext;
        pUnitLast->pNext = pUnit;
    }
    else
    {
        pUnit->pNext = NULL;
        g_pUsb->pUnits = pUnit;
    }

    return pUnit;
}

//-----------------------------------------------------------------------------
void unit_free(usb_unit *unit)
{
    usb_unit *pUnit,*pPrev;

    pUnit = g_pUsb->pUnits;
    pPrev = NULL;

    while (pUnit)
    {
        if (pUnit == unit)
            break;
        pPrev = pUnit;
        pUnit = pUnit->pNext;
    }
    if (!pUnit)
    {
        iop_DebugMsg("unit_free[%d]: Attempt to free a unit that was already free.\n",unit->device_id);
        return;
    }

    if (pPrev)
    {
        pPrev->pNext = unit->pNext;
    }
    else
    {
        g_pUsb->pUnits = unit->pNext;
    }
    iop_Free(unit);
}

u32 FindDeviceIndex(usb_unit *unit)
{
    usb_unit    *pUnit;
    u32         index;

    index = 0;
    pUnit = g_pUsb->pUnits;

    while (pUnit)
    {
        if ( (pUnit != unit) && (pUnit->type == unit->type) )
            index++;
        pUnit = pUnit->pNext;
    }
    return index;
}
//-------------------------------------------------
//---- Callbacks
//-------------------------------------------------
//-----------------------------------------------------------------------------
#define SET_KEYBOARD_BIT(unit,keycode) unit->device.keyboard.pressed[keycode / 32] |= 1<<(keycode & 31)

void data_transfer_done(s32 result,s32 count,void *pArg)
{
    usb_unit *unit=(usb_unit *)pArg;
    s32 i;
    u8  keycode;
    u8 *pTranslation;

    (void)result;
#ifdef USB_DEBUG
    printf("data_transfer_done[%d]: result=%d, count=%d\n",unit->device_id,result,count);
    printf("data_transfer_done[%d]: Payload = ",unit->device_id);
    for (i=0;i<count;i++)
        printf("%s%d",(i==0)?"":",",unit->data[i]);
    printf("\n");
#endif
    switch (unit->type)
    {
    case USBDEV_KEYBOARD:
        unit->device.keyboard.shift_state = unit->data[0];
        unit->device.keyboard.last_led_state = unit->data[1];
        //
        // Translate shift-state in to actual keycodes
        //
        // Get's set to all 1 if too many keys are pressed so we ignore the data
        if (unit->data[2]==1)
            break;
        //
        // Clear old keypressed state
        //
        memset(unit->device.keyboard.pressed,0,256/sizeof(u32));
        //
        // Form a new keypressed state
        //

        for (i=0;i<8;i++)
        {
            if (unit->device.keyboard.shift_state & (1<<i) )
            {
                SET_KEYBOARD_BIT(unit,Shiftcode_Convert[i]);
            }
        }

        for (i=2;i<count;i++)
        {
            keycode = unit->data[i];
            switch (unit->device.keyboard.country_code)
            {
            case 8:
                pTranslation = International_Convert_FRANCE;
                break;
            case 9:
                pTranslation = International_Convert_GERMANY;
                break;
            default:
                pTranslation = NULL;
            }

            if (pTranslation)
            {
                while (pTranslation[0])
                {
                    if (pTranslation[0] == keycode)
                    {
                        keycode = pTranslation[1];
                        break;
                    }
                    pTranslation+=2;
                }
            }
            keycode = Scancode_Convert[keycode];
            // Set a bit if a key has been pressed
            SET_KEYBOARD_BIT(unit,keycode);
        }
        break;

    case USBDEV_MOUSE:
        unit->device.mouse.buttons = unit->data[0];
        unit->device.mouse.x += (s8)unit->data[1];
        unit->device.mouse.y += (s8)unit->data[2];
        unit->device.mouse.scroll += (s8)unit->data[3];
#ifdef USB_DEBUG
        printf("data_transfer_done[%d]: mouse button=%d,scroll=%d,x=%d,y=%d\n",
                unit->device_id,
                unit->device.mouse.buttons,
                unit->device.mouse.scroll,
                unit->device.mouse.x,
                unit->device.mouse.y);

#endif
        break;
    case USBDEV_UNKNOWN:
    default:
        iop_DebugMsg("data_transfer_done[%d]: Unknown device type when transferring data\n",unit->device_id);
        break;
    }
    data_transfer(unit);

}

//-----------------------------------------------------------------------------
void data_transfer(usb_unit *unit)
{
    s32 status;

    status = sceUsbdInterruptTransfer(unit->data_pipe,
                                      unit->data,
                                      unit->payload_size,
                                      data_transfer_done,
                                      unit);
#ifdef USB_DEBUG
    if (status)
    {
        printf("data_transfer: Transferring %d bytes failed to device %d.\n",unit->payload_size,unit->device_id);
    }
#endif
}

//-----------------------------------------------------------------------------
void set_interface_done(s32 result,s32 count,void *pArg)
{
    usb_unit *unit = (usb_unit *)pArg;

    (void)result;
    (void)count;
    data_transfer(unit);
}

//-----------------------------------------------------------------------------
void set_config_done(s32 result,s32 count,void *pArg)
{
    usb_unit *unit = (usb_unit *)pArg;
    s32 status;

    (void)result;
    (void)count;

    if (unit->alternate_setting >0)
    {
        status = sceUsbdSetInterface(unit->control_pipe,
                                     unit->interface_number,
                                     unit->alternate_setting,
                                     set_interface_done,
                                     unit);
        if (status)
        {
            iop_DebugMsg("set_config_done: Unable to set interface on unit %d.\n",unit->device_id);
        }
    }
    else
    {
        data_transfer(unit);
    }
}

//-------------------------------------------------
//-- Mouse device driver
//-------------------------------------------------
//-----------------------------------------------------------------------------
s32 usb_probe(s32 dev_id)
{
    UsbDeviceDescriptor *device_desc;
    UsbInterfaceDescriptor *interface_desc;

    iop_DebugMsg("usb_probe[%d]: Attempting to probe for mouse or keyboard..\n",dev_id);

    device_desc = sceUsbdScanStaticDescriptor(dev_id,
                                              NULL,
                                              USB_DESCRIPTOR_TYPE_DEVICE);
    if (!device_desc)
    {
        iop_DebugMsg("usb_probe[%d]: Attempt to scan device descriptor failed\n",dev_id);
        return 0;
    }

    interface_desc = sceUsbdScanStaticDescriptor(dev_id,
                                                 (UsbInterfaceDescriptor *)device_desc,
                                                 USB_DESCRIPTOR_TYPE_INTERFACE);
    if (!interface_desc)
    {
        iop_DebugMsg("usb_probe[%d]: Attempt to scan device interface failed\n",dev_id);
        return 0;
    }


    if ( (interface_desc->bInterfaceClass    == 3) &&
         (interface_desc->bInterfaceSubClass == 1) &&
         (interface_desc->bInterfaceProtocol == 2) )
    {
        return 1;
    }

    if ( (interface_desc->bInterfaceClass    == 3) &&
         (interface_desc->bInterfaceSubClass == 1) &&
         (interface_desc->bInterfaceProtocol == 1) )
    {
        return 1;
    }

    iop_DebugMsg("usb_probe[%d]: Probe completed - unknown device.\n",dev_id);

    return 1;
}

//-----------------------------------------------------------------------------
s32 usb_attach(s32 dev_id)
{
    UsbConfigurationDescriptor  *config_desc;
    UsbInterfaceDescriptor      *interface_desc;
    UsbEndpointDescriptor       *endpoint_desc;
    UsbHidDescriptor            *hid_desc;
    s32                         payload_size;
    usb_unit                    *unit;
    s32                         status;
 
    iop_DebugMsg("usb_attach[%d]: Attempting to attach mouse or keyboard..\n",dev_id);

    config_desc = sceUsbdScanStaticDescriptor(dev_id,NULL,USB_DESCRIPTOR_TYPE_CONFIGURATION);
    if (!config_desc)
    {

        iop_DebugMsg("usb_attach[%d]: Attempt to scan configuration failed.\n",dev_id);
        return -1;
    }

    interface_desc = sceUsbdScanStaticDescriptor(dev_id,
                                                 (UsbInterfaceDescriptor *)config_desc,
                                                 USB_DESCRIPTOR_TYPE_INTERFACE);
    if (!interface_desc)
    {
        iop_DebugMsg("usb_attach[%d]: Attempt to scan interface failed.\n",dev_id);
        return -1;
    }

    if (interface_desc->bNumEndpoints != 1)
    {
        iop_DebugMsg("usb_attach[%d]: No endpoints?\n",dev_id);
        return -1;
    }

    endpoint_desc = sceUsbdScanStaticDescriptor(dev_id,
                                                (UsbInterfaceDescriptor *)config_desc,
                                                USB_DESCRIPTOR_TYPE_ENDPOINT);
    if (!endpoint_desc)
    {
        iop_DebugMsg("usb_attach[%d]: Attempt to scan endpoint failed.\n",dev_id);
        return -1;
    }

    if ( ((endpoint_desc->bEndpointAddress & USB_ENDPOINT_DIRECTION_BITS) != USB_ENDPOINT_DIRECTION_IN) ||
         ((endpoint_desc->bmAttribute & USB_ENDPOINT_TRANSFER_TYPE_BITS) != USB_ENDPOINT_TRANSFER_TYPE_INTERRUPT) )
    {
        iop_DebugMsg("usb_attach[%d]: Endpoint direction or transfer type invalid.\n",dev_id);
        return -1;
    }


    payload_size = endpoint_desc->wMaxPacketSize0 | (endpoint_desc->wMaxPacketSize1 << 8);


    unit = unit_alloc(dev_id,payload_size,
                      interface_desc->bInterfaceNumber,
                      interface_desc->bAlternateSetting);

    if (!unit)
    {
        iop_DebugMsg("usb_attach[%d]: Unable to allocate unit (payload size=%d).\n",dev_id,payload_size);
        return -1;
    }

    unit->control_pipe = sceUsbdOpenPipe(dev_id,NULL);
    if (unit->control_pipe <0)
    {
        iop_DebugMsg("usb_attach[%d]: Unable to open control pipe.\n",dev_id);
        unit_free(unit);
        return -1;
    }
    unit->data_pipe = sceUsbdOpenPipe(dev_id,endpoint_desc);
    if (unit->data_pipe <0)
    {
        iop_DebugMsg("usb_attach[%d]: Unable to open data pipe.\n",dev_id);
        unit_free(unit);
        return -1;
    }
    sceUsbdSetPrivateData(dev_id,unit);
    status = sceUsbdSetConfiguration(unit->control_pipe, 
                                     config_desc->bConfigurationValue,
                                     set_config_done,
                                     unit);
    if (status != sceUsbd_NOERR)
    {
        iop_DebugMsg("usb_attach[%d]: Unable to set configuration.\n",dev_id);
        unit_free(unit);
        return -1;
    }
    if ( (interface_desc->bInterfaceClass    == 3) &&
         (interface_desc->bInterfaceSubClass == 1) &&
         (interface_desc->bInterfaceProtocol == 2) )
    {
        iop_DebugMsg("usb_attach[%d]: Device type MOUSE\n",unit->device_id);
        unit->type = USBDEV_MOUSE;
    }
    else if ( (interface_desc->bInterfaceClass    == 3) &&
              (interface_desc->bInterfaceSubClass == 1) &&
              (interface_desc->bInterfaceProtocol == 1) )
    {
        iop_DebugMsg("usb_attach[%d]: Device type KEYBOARD\n",unit->device_id);
        unit->type = USBDEV_KEYBOARD;

        hid_desc = sceUsbdScanStaticDescriptor(dev_id,
                                                (UsbInterfaceDescriptor *)config_desc,
                                                USB_DESCRIPTOR_TYPE_HID);
        if (hid_desc)
        {
            iop_DebugMsg("Country code from keyboard %d\n",hid_desc->bCountryCode);
            unit->device.keyboard.country_code = hid_desc->bCountryCode;
        }
        else
        {
            unit->device.keyboard.country_code = 0;
        }

    }
    else
    {
        iop_DebugMsg("usb_attach[%d]: Device type UNKNOWN\n",unit->device_id);
        unit->type = USBDEV_UNKNOWN;
    }


    if (unit->type == USBDEV_KEYBOARD)
    {
        unit->device.keyboard.unit_number = FindDeviceIndex(unit);
    }
    else
    {
        unit->device.mouse.unit_number = FindDeviceIndex(unit);
    }

    iop_DebugMsg("usb_attach[%d]: Attach completed (location = %08x)\n",unit->device_id,unit->location);
    return 0;

}

//-----------------------------------------------------------------------------
s32 usb_detach(s32 dev_id)
{
    usb_unit *pUnit;

    iop_DebugMsg("usb_detach[%d]: Attempting to detach..\n",dev_id);

    pUnit = sceUsbdGetPrivateData(dev_id);
    if (!pUnit)
    {
        iop_DebugMsg("usb_detach[%d]: Unit was free?\n",dev_id);
        return 0;
    }
    unit_free(pUnit);
    iop_DebugMsg("usb_detach[%d]: Detach completed.\n",dev_id);
    return 1;
}

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

//-----------------------------------------------------------------------------
void *usb_Init(void)
{
    s32 status;

    ASSERT(g_pUsb==NULL);

    iop_DebugMsg("USB Init, %d bytes free before\n",iop_MemFree());
    g_pUsb = iop_Malloc(sizeof(t_IopUsbVars));
    ASSERT(g_pUsb);
    g_pUsb->pUnits = 0;
    status = sceUsbdRegisterLdd(&MouseLddOps);
    if (status)
    {
        iop_DebugMsg("UsbInit: Unable to register usb mouse driver\n");
    }
    status = sceUsbdRegisterLdd(&KeyboardLddOps);
    if (status)
    {
        iop_DebugMsg("UsbInit: Unable to register usb keyboard driver\n");
    }
    iop_DebugMsg("%d bytes free after usb init\n",iop_MemFree());
    return 0;
}

//-----------------------------------------------------------------------------
void *usb_Kill(void)
{
    ASSERT(g_pUsb);
    sceUsbdUnregisterLdd(&MouseLddOps);
    sceUsbdUnregisterLdd(&KeyboardLddOps);
    iop_Free(g_pUsb);
    g_pUsb=NULL;
    return 0;
}

//-----------------------------------------------------------------------------
void usb_UpdateMouseState(void)
{
    usb_unit *pUnit;
    u32         buttons,newbuttons;
    s32 index;

    pUnit = g_pUsb->pUnits;
    index = 0;

    memset(g_pUsb->DeviceState.Mouse,0,sizeof(g_pUsb->DeviceState.Mouse));

    while (pUnit)
    {
        if (pUnit->type == USBDEV_MOUSE)
        {

            newbuttons=0;
            buttons = pUnit->device.mouse.buttons;
            g_pUsb->DeviceState.Mouse[index].is_present = TRUE;

            if (buttons & 0x01) newbuttons |= 1<<(INPUT_MOUSE_BTN_L-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x02) newbuttons |= 1<<(INPUT_MOUSE_BTN_R-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x04) newbuttons |= 1<<(INPUT_MOUSE_BTN_C-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x08) newbuttons |= 1<<(INPUT_MOUSE_BTN_0-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x10) newbuttons |= 1<<(INPUT_MOUSE_BTN_1-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x20) newbuttons |= 1<<(INPUT_MOUSE_BTN_2-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x40) newbuttons |= 1<<(INPUT_MOUSE_BTN_3-INPUT_MOUSE__DIGITAL);
            if (buttons & 0x80) newbuttons |= 1<<(INPUT_MOUSE_BTN_4-INPUT_MOUSE__DIGITAL);

            g_pUsb->DeviceState.Mouse[index].Buttons = newbuttons;
            g_pUsb->DeviceState.Mouse[index].Wheel = pUnit->device.mouse.scroll;
            g_pUsb->DeviceState.Mouse[index].X = pUnit->device.mouse.x;
            g_pUsb->DeviceState.Mouse[index].Y = pUnit->device.mouse.y;

            index++;
            if (index>=MAX_USB_MOUSE)
                return;
        }
        pUnit=pUnit->pNext;
    }

//    pUnit->device.mouse.x = 0;
//    pUnit->device.mouse.y = 0;
//    pUnit->device.mouse.scroll = 0;
}

//-----------------------------------------------------------------------------
void usb_UpdateKeyState(void)
{
    usb_unit *pUnit;
    s32 index;

    memset(g_pUsb->DeviceState.Keyboard,0,sizeof(g_pUsb->DeviceState.Keyboard));

    pUnit = g_pUsb->pUnits;
    index = 0;

    while (pUnit)
    {
        if (pUnit->type == USBDEV_KEYBOARD)
        {
            g_pUsb->DeviceState.Keyboard[index].is_present = TRUE;
            g_pUsb->DeviceState.Keyboard[index].led_state = pUnit->device.keyboard.led_state;
            g_pUsb->DeviceState.Keyboard[index].shift_state = pUnit->device.keyboard.shift_state;
            memcpy( g_pUsb->DeviceState.Keyboard[index].pressed,
                    pUnit->device.keyboard.pressed,
                    sizeof(pUnit->device.keyboard.pressed));
            index++;
            if (index >= MAX_USB_KEYBOARD)
            {
                return;
            }

        }
        pUnit=pUnit->pNext;
    }
}

//-----------------------------------------------------------------------------
void *usb_Dispatch(u32 Command,void *Data,s32 Size)
{
    s32 *pData;
    pData = (s32 *)Data;

    (void)Size;
    switch(Command)
    {
//-------------------------------------------------
    case USBCMD_INIT:
        return usb_Init();
        break;
//-------------------------------------------------
    case USBCMD_KILL:
        return usb_Kill();

        break;
//-------------------------------------------------
    case USBCMD_READDEVICES:
        ASSERT(g_pUsb);
        usb_UpdateKeyState();
        usb_UpdateMouseState();
        return &g_pUsb->DeviceState;
        break;
//-------------------------------------------------
    default:
        iop_DebugMsg("UsbDispatch: Invalid USB dispatch code 0x%08x\n",Command);
        break;
    }
    return NULL;
}

#else // ENABLE_USB_DEVICES

//-----------------------------------------------------------------------------
void *usb_Init(void)
{
    return NULL;
}

//-----------------------------------------------------------------------------
void *usb_Kill(void)
{
    return NULL;
}

//-----------------------------------------------------------------------------
void *usb_Dispatch(u32 Command,void *Data,s32 Size)
{
    (void)Command;
    (void)Data;
    (void)Size;
    ASSERTS(FALSE,"usb: Request received for keyboard/mouse update when it's been disabled");
    return NULL;
}

#endif