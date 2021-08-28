/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                       Controller Library 2
 *                          Version 2.01
 *                           Shift-JIS
 *
 *         Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         libpad2 - libpad2.h
 *                     header file of libpad2
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      2.01          2001-03-09        nozomu      the first version
 *
 */

#ifndef _LIBPAD2_H_
#define _LIBPAD2_H_

#define SCE_PAD2_DMA_BUFFER_SIZE	(256)
#define SCE_PAD2_BUTTON_PROFILE_SIZE	(4)
#define SCE_PAD2_BUTTON_DATA_SIZE	(18)
#define SCE_PAD2_STATESTR_SIZE		(16)

#define SCE_PAD2_DMA_BUFFER_MAX		(16)
#define SCE_PAD2_MAX_DEVICE_NAME	(16)

#define SCE_PAD2_PORT_1C		(0)
#define SCE_PAD2_PORT_2C		(1)
#define SCE_PAD2_PORT_USB		(99)

#define SCE_PAD2_SPECIFIC_PORT			( 1 << 1 )
#define SCE_PAD2_SPECIFIC_DRIVER_NUMBER		( 1 << 2 )
#define SCE_PAD2_SPECIFIC_DEVICE_NAME		( 1 << 3 )

#define scePad2StateNoLink	(0)
#define scePad2StateStable	(1)
#define scePad2StateExecCmd	(2)
#define scePad2StateError	(3)

#define SCE_PAD2_SELECT			(0)
#define SCE_PAD2_L3			(1)
#define SCE_PAD2_R3			(2)
#define SCE_PAD2_START			(3)
#define SCE_PAD2_UP			(4)
#define	SCE_PAD2_RIGHT			(5)
#define SCE_PAD2_DOWN			(6)
#define	SCE_PAD2_LEFT			(7)

#define	SCE_PAD2_L2			(8)
#define	SCE_PAD2_R2			(9)
#define	SCE_PAD2_L1			(10)
#define	SCE_PAD2_R1			(11)
#define	SCE_PAD2_TRIANGLE		(12)
#define	SCE_PAD2_CIRCLE			(13)
#define	SCE_PAD2_CROSS			(14)
#define SCE_PAD2_SQUARE			(15)

#define SCE_PAD2_STICK_RX		(16)	
#define	SCE_PAD2_STICK_RY		(17)
#define	SCE_PAD2_STICK_LX		(18)
#define	SCE_PAD2_STICK_LY		(19)
#define SCE_PAD2_ANALOG_RIGHT		(20)
#define	SCE_PAD2_ANALOG_LEFT		(21)
#define	SCE_PAD2_ANALOG_UP		(22)
#define	SCE_PAD2_ANALOG_DOWN		(23)

#define SCE_PAD2_ANALOG_TRIANGLE	(24)
#define	SCE_PAD2_ANALOG_CIRCLE		(25)
#define SCE_PAD2_ANALOG_CROSS		(26)
#define	SCE_PAD2_ANALOG_SQUARE		(27)
#define	SCE_PAD2_ANALOG_L1		(28)
#define SCE_PAD2_ANALOG_R1		(29)
#define	SCE_PAD2_ANALOG_L2		(30)
#define SCE_PAD2_ANALOG_R2		(31)

typedef struct
{
	unsigned int option;
	int port;
	int slot;
	int number;
	unsigned char name[ SCE_PAD2_MAX_DEVICE_NAME ];
} scePad2SocketParam;

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
int scePad2Init( int );
int scePad2End( void );
int scePad2CreateSocket( scePad2SocketParam*, void* );
int scePad2DeleteSocket( int );
int scePad2Read( int, unsigned char* );
int scePad2GetButtonProfile( int, unsigned char* );
int scePad2GetState( int );
int scePad2GetButtonInfo( int, unsigned char*, int );
void scePad2StateIntToStr( int, unsigned char* );
void *scePad2GetErxEntries(void);
#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _LIBPAD2_H_ */

