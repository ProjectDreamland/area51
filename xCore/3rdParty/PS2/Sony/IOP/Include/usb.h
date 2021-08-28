/* SCE CONFIDENTIAL
"PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*	Copyright (C) 1999-2000 Sony Computer Entertainment Inc.
 *			All Right Reserved
 *
 * usb.h - USB Specification
 *
 * $Id: usb.h,v 1.10 2002/03/27 05:10:07 xokano Exp $
 */

#if !defined(_USB_H)
#define _USB_H

typedef struct {	/* Format of Setup Data */
	u_char bmRequestType;
	u_char bRequest;
	u_short wValue;
	u_short wIndex;
	u_short wLength;
} UsbDeviceRequest;

#define USB_REQTYPE_DIR_BITS			0x80
#define USB_REQTYPE_DIR_TO_DEVICE		0x00
#define USB_REQTYPE_DIR_TO_HOST			0x80
#define USB_REQTYPE_TYPE_BITS			0x60
#define USB_REQTYPE_TYPE_STANDARD		0x00
#define USB_REQTYPE_TYPE_CLASS			0x20
#define USB_REQTYPE_TYPE_VENDOR			0x40
#define USB_REQTYPE_TYPE_RESERVED		0x60
#define USB_REQTYPE_RECIP_BITS			0x1f
#define USB_REQTYPE_RECIP_DEVICE		0x00
#define USB_REQTYPE_RECIP_INTERFACE		0x01
#define USB_REQTYPE_RECIP_ENDPOINT		0x02
#define USB_REQTYPE_RECIP_OTHER			0x03

#define USB_REQUEST_GET_STATUS			0x00
#define USB_REQUEST_CLEAR_FEATURE		0x01
#define USB_REQUEST_SET_FEATURE			0x03
#define USB_REQUEST_SET_ADDRESS			0x05
#define USB_REQUEST_GET_DESCRIPTOR		0x06
#define USB_REQUEST_GET_CONFIGURATION		0x08
#define USB_REQUEST_SET_CONFIGURATION		0x09
#define USB_REQUEST_GET_INTERFACE		0x0a
#define USB_REQUEST_SET_INTERFACE		0x0b
#define USB_REQUEST_SYNCH_FRAME			0x0c

#define USB_DESCRIPTOR_TYPE_DEVICE		0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION	0x02
#define USB_DESCRIPTOR_TYPE_STRING		0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE		0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT		0x05
#define USB_DESCRIPTOR_TYPE_HID                 0x21
#define USB_DESCRIPTOR_TYPE_REPORT		0x22

#define USB_FEATURE_ENDPOINT_HALT		0x00
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP	0x01

#define USB_CLASS_PER_INTERFACE			0x00
#define USB_CLASS_AUDIO				0x01
#define USB_CLASS_COMMUNICATIONS		0x02
#define USB_CLASS_HID				0x03
#define USB_CLASS_MONITOR			0x04
#define USB_CLASS_PHYSICAL			0x05
#define USB_CLASS_POWER				0x06
#define USB_CLASS_PRINTER			0x07
#define USB_CLASS_STORAGE			0x08
#define USB_CLASS_HUB				0x09
#define USB_CLASS_DATA				0x0a
#define USB_CLASS_VENDOR_SPECIFIC		0xff

typedef struct {
	u_char bLength;
	u_char bDescriptorType;
	u_short bcdUSB;
	u_char bDeviceClass;
	u_char bDeviceSubClass;
	u_char bDeviceProtocol;
	u_char bMaxPacketSize0;
	u_short idVendor;
	u_short idProduct;
	u_short bcdDevice;
	u_char iManufacturer;
	u_char iProduct;
	u_char iSerialNumber;
	u_char bNumConfigurations;
} UsbDeviceDescriptor;

typedef struct {
	u_char bLength;
	u_char bDescriptorType;
	u_char wTotalLength0;
	u_char wTotalLength1;
	u_char bNumInterfaces;
	u_char bConfigurationValue;
	u_char iConfiguration;
	u_char bmAttribute;
	u_char MaxPower;
} UsbConfigurationDescriptor;

#define USB_CONFIGURATION_RESERVED_ZERO		0x1f
#define USB_CONFIGURATION_REMOTE_WAKEUP		0x20
#define USB_CONFIGURATION_SELF_POWERED		0x40
#define USB_CONFIGURATION_RESERVED_ONE		0x80

typedef struct {
	u_char bLength;
	u_char bDescriptorType;
	u_char bInterfaceNumber;
	u_char bAlternateSetting;
	u_char bNumEndpoints;
	u_char bInterfaceClass;
	u_char bInterfaceSubClass;
	u_char bInterfaceProtocol;
	u_char iInterface;
} UsbInterfaceDescriptor;

typedef struct {
	u_char bLength;
	u_char bDescriptorType;
	u_char bEndpointAddress;
	u_char bmAttribute;
	u_char wMaxPacketSize0;
	u_char wMaxPacketSize1;
	u_char bInterval;
} UsbEndpointDescriptor;

/* bmAttribute */
#define USB_ENDPOINT_TRANSFER_TYPE_BITS		0x03
#define USB_ENDPOINT_TRANSFER_TYPE_SHIFT	0
#define USB_ENDPOINT_TRANSFER_TYPE_CONTROL	0x00
#define USB_ENDPOINT_TRANSFER_TYPE_ISOCHRONOUS	0x01
#define USB_ENDPOINT_TRANSFER_TYPE_BULK		0x02
#define USB_ENDPOINT_TRANSFER_TYPE_INTERRUPT	0x03

/* bEndpointAddress */
#define USB_ENDPOINT_NUMBER_BITS		0x1f
#define USB_ENDPOINT_NUMBER_SHIFT		0
#define USB_ENDPOINT_DIRECTION_BITS		0x80
#define USB_ENDPOINT_DIRECTION_SHIFT		7
#define USB_ENDPOINT_DIRECTION_OUT		0x00
#define USB_ENDPOINT_DIRECTION_IN		0x80

typedef struct {
	u_char bLength;
	u_char bDescriptorType;
	u_char bString[0];
} UsbStringDescriptor;


/* HID Descriptor (Class Specific Descriptor) */
typedef struct {
  u_char bDescriptorType;
  u_char wDescriptorLength0;
  u_char wDescriptorLength1;
} UsbHidSubDescriptorInfo;

typedef struct {
  u_char bLength;
  u_char bDescriptorType;
  u_char bcdHID0;
  u_char bcdHID1;
  u_char bCountryCode;
  u_char bNumDescriptors;  /* Number of SubDescriptor */
  UsbHidSubDescriptorInfo SubDescriptorInfo[0];
} UsbHidDescriptor;

#define USB_MAX_LS_CONTROL_PACKET_SIZE		8	/* low speed */
#define USB_MAX_FS_CONTROL_PACKET_SIZE		64	/* full speed */
#define USB_MAX_ISOCHRONOUS_PACKET_SIZE		1023	/* full speed only */
#define USB_MAX_LS_INTERRUPT_PACKET_SIZE	8	/* low speed */
#define USB_MAX_FS_INTERRUPT_PACKET_SIZE	64	/* full speed */
#define USB_MAX_BULK_PACKET_SIZE		64	/* full speed only */

#endif	/* !_USB_H */
