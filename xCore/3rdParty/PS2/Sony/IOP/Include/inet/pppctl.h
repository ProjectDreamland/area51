/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.01
 *                           Shift-JIS
 *
 *      Copyright (C) 2000-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - ppp.h
 *                        PPP Control I/F
 *
 * $Id: pppctl.h,v 1.9 2002/03/27 05:11:13 xokano Exp $
 */

#ifndef _PPPCTL_H
#define _PPPCTL_H

/* PPP configuration code */

#define _PPPCC_BASECODE		0x90000000
#define _PPPCC_NETDEV		(1 << 31)
#define _PPPCC_MODEM		(1 << 30)
#define _PPPCC_READ		(1 << 15)
#define _PPPCC_WRITE		(1 << 14)
#define	_PPPCC(rw, n)		(_PPPCC_BASECODE + (rw) + n)

#define	scePPPCC_GetChatDial		_PPPCC(_PPPCC_READ, 1)
#define	scePPPCC_GetChatLogin		_PPPCC(_PPPCC_READ, 2)
#define	scePPPCC_GetPhoneNumber		_PPPCC(_PPPCC_READ, 3)
#define	scePPPCC_GetAuthName		_PPPCC(_PPPCC_READ, 4)
#define	scePPPCC_GetAuthKey		_PPPCC(_PPPCC_READ, 5)
#define	scePPPCC_GetPeerName		_PPPCC(_PPPCC_READ, 6)
#define	scePPPCC_GetPeerKey		_PPPCC(_PPPCC_READ, 7)
#define	scePPPCC_GetLcpTimeout		_PPPCC(_PPPCC_READ, 8)
#define	scePPPCC_GetIpcpTimeout		_PPPCC(_PPPCC_READ, 9)
#define	scePPPCC_GetIdleTimeout		_PPPCC(_PPPCC_READ, 10)
#define	scePPPCC_GetWantMruNego		_PPPCC(_PPPCC_READ, 11)
#define	scePPPCC_GetWantAccmNego	_PPPCC(_PPPCC_READ, 12)
#define	scePPPCC_GetWantMagicNego	_PPPCC(_PPPCC_READ, 13)
#define	scePPPCC_GetWantPrcNego		_PPPCC(_PPPCC_READ, 14)
#define	scePPPCC_GetWantAccNego		_PPPCC(_PPPCC_READ, 15)
#define	scePPPCC_GetWantAddressNego	_PPPCC(_PPPCC_READ, 16)
#define	scePPPCC_GetWantVjcompNego	_PPPCC(_PPPCC_READ, 17)
#define	scePPPCC_GetWantMruValue	_PPPCC(_PPPCC_READ, 18)
#define	scePPPCC_GetWantAccmValue	_PPPCC(_PPPCC_READ, 19)
#define	scePPPCC_GetWantAuth		_PPPCC(_PPPCC_READ, 20)
#define	scePPPCC_GetWantIpAddress	_PPPCC(_PPPCC_READ, 21)
#define	scePPPCC_GetWantIpMask		_PPPCC(_PPPCC_READ, 22)
#define	scePPPCC_GetAllowMruNego	_PPPCC(_PPPCC_READ, 23)
#define	scePPPCC_GetAllowAccmNego	_PPPCC(_PPPCC_READ, 24)
#define	scePPPCC_GetAllowMagicNego	_PPPCC(_PPPCC_READ, 25)
#define	scePPPCC_GetAllowPrcNego	_PPPCC(_PPPCC_READ, 26)
#define	scePPPCC_GetAllowAccNego	_PPPCC(_PPPCC_READ, 27)
#define	scePPPCC_GetAllowAddressNego	_PPPCC(_PPPCC_READ, 28)
#define	scePPPCC_GetAllowVjcompNego	_PPPCC(_PPPCC_READ, 29)
#define	scePPPCC_GetAllowMruValue	_PPPCC(_PPPCC_READ, 30)
#define	scePPPCC_GetAllowAccmValue	_PPPCC(_PPPCC_READ, 31)
#define	scePPPCC_GetAllowAuth		_PPPCC(_PPPCC_READ, 32)
#define	scePPPCC_GetAllowIpAddress	_PPPCC(_PPPCC_READ, 33)
#define	scePPPCC_GetAllowIpMask		_PPPCC(_PPPCC_READ, 34)
#define	scePPPCC_GetWantDNS1Nego	_PPPCC(_PPPCC_READ, 35)
#define	scePPPCC_GetWantDNS2Nego	_PPPCC(_PPPCC_READ, 36)
#define	scePPPCC_GetWantDNS1		_PPPCC(_PPPCC_READ, 37)
#define	scePPPCC_GetWantDNS2		_PPPCC(_PPPCC_READ, 38)
#define	scePPPCC_GetAllowDNS1Nego	_PPPCC(_PPPCC_READ, 39)
#define	scePPPCC_GetAllowDNS2Nego	_PPPCC(_PPPCC_READ, 40)
#define	scePPPCC_GetAllowDNS1		_PPPCC(_PPPCC_READ, 41)
#define	scePPPCC_GetAllowDNS2		_PPPCC(_PPPCC_READ, 42)
#define	scePPPCC_GetConnectTimeout	_PPPCC(_PPPCC_READ, 43)
#define	scePPPCC_GetDialingType		_PPPCC(_PPPCC_READ, 44)
#if 0
#define	scePPPCC_GetZeroPrefix		_PPPCC(_PPPCC_READ, 45)
#endif
#define	scePPPCC_GetToneDial		_PPPCC(_PPPCC_READ, 46)
#define	scePPPCC_GetPulseDial		_PPPCC(_PPPCC_READ, 47)
#define	scePPPCC_GetAnyDial		_PPPCC(_PPPCC_READ, 48)
#if 0
#define	scePPPCC_GetZeroPrefixOff	_PPPCC(_PPPCC_READ, 49)
#define	scePPPCC_GetZeroPrefixOn	_PPPCC(_PPPCC_READ, 50)
#endif
#define	scePPPCC_GetLogFlags		_PPPCC(_PPPCC_READ, 51)
#define	scePPPCC_GetHisAddress          _PPPCC(_PPPCC_READ, 52)
#define	scePPPCC_GetHisMask             _PPPCC(_PPPCC_READ, 53)
#define	scePPPCC_GetOmitEmptyFrame	_PPPCC(_PPPCC_READ, 54)
#define	scePPPCC_GetChapType		_PPPCC(_PPPCC_READ, 55)
#define	scePPPCC_GetCurrentStatus	_PPPCC(_PPPCC_READ, 56)
#define	scePPPCC_GetLcpEchoTimeout	_PPPCC(_PPPCC_READ, 57)
/* 58 */
#define	scePPPCC_GetAuthTimeout		_PPPCC(_PPPCC_READ, 59)
#define	scePPPCC_GetLcpMaxTerminate	_PPPCC(_PPPCC_READ, 60)
#define	scePPPCC_GetIpcpMaxTerminate	_PPPCC(_PPPCC_READ, 61)
#define	scePPPCC_GetLcpMaxConfigure	_PPPCC(_PPPCC_READ, 62)
#define	scePPPCC_GetIpcpMaxConfigure	_PPPCC(_PPPCC_READ, 63)
#define	scePPPCC_GetLcpMaxFailure	_PPPCC(_PPPCC_READ, 64)
#define	scePPPCC_GetAuthMaxFailure	_PPPCC(_PPPCC_READ, 65)
#define	scePPPCC_GetLastResultString	_PPPCC(_PPPCC_READ, 66)
#define	scePPPCC_GetAuthMessage		_PPPCC(_PPPCC_READ, 67)

#define	scePPPCC_SetChatDial		_PPPCC(_PPPCC_WRITE, 1)
#define	scePPPCC_SetChatLogin		_PPPCC(_PPPCC_WRITE, 2)
#define	scePPPCC_SetPhoneNumber		_PPPCC(_PPPCC_WRITE, 3)
#define	scePPPCC_SetAuthName		_PPPCC(_PPPCC_WRITE, 4)
#define	scePPPCC_SetAuthKey		_PPPCC(_PPPCC_WRITE, 5)
#define	scePPPCC_SetPeerName		_PPPCC(_PPPCC_WRITE, 6)
#define	scePPPCC_SetPeerKey		_PPPCC(_PPPCC_WRITE, 7)
#define	scePPPCC_SetLcpTimeout		_PPPCC(_PPPCC_WRITE, 8)
#define	scePPPCC_SetIpcpTimeout		_PPPCC(_PPPCC_WRITE, 9)
#define	scePPPCC_SetIdleTimeout		_PPPCC(_PPPCC_WRITE, 10)
#define	scePPPCC_SetWantMruNego		_PPPCC(_PPPCC_WRITE, 11)
#define	scePPPCC_SetWantAccmNego	_PPPCC(_PPPCC_WRITE, 12)
#define	scePPPCC_SetWantMagicNego	_PPPCC(_PPPCC_WRITE, 13)
#define	scePPPCC_SetWantPrcNego		_PPPCC(_PPPCC_WRITE, 14)
#define	scePPPCC_SetWantAccNego		_PPPCC(_PPPCC_WRITE, 15)
#define	scePPPCC_SetWantAddressNego	_PPPCC(_PPPCC_WRITE, 16)
#define	scePPPCC_SetWantVjcompNego	_PPPCC(_PPPCC_WRITE, 17)
#define	scePPPCC_SetWantMruValue	_PPPCC(_PPPCC_WRITE, 18)
#define	scePPPCC_SetWantAccmValue	_PPPCC(_PPPCC_WRITE, 19)
#define	scePPPCC_SetWantAuth		_PPPCC(_PPPCC_WRITE, 20)
#define	scePPPCC_SetWantIpAddress	_PPPCC(_PPPCC_WRITE, 21)
#define	scePPPCC_SetWantIpMask		_PPPCC(_PPPCC_WRITE, 22)
#define	scePPPCC_SetAllowMruNego	_PPPCC(_PPPCC_WRITE, 23)
#define	scePPPCC_SetAllowAccmNego	_PPPCC(_PPPCC_WRITE, 24)
#define	scePPPCC_SetAllowMagicNego	_PPPCC(_PPPCC_WRITE, 25)
#define	scePPPCC_SetAllowPrcNego	_PPPCC(_PPPCC_WRITE, 26)
#define	scePPPCC_SetAllowAccNego	_PPPCC(_PPPCC_WRITE, 27)
#define	scePPPCC_SetAllowAddressNego	_PPPCC(_PPPCC_WRITE, 28)
#define	scePPPCC_SetAllowVjcompNego	_PPPCC(_PPPCC_WRITE, 29)
#define	scePPPCC_SetAllowMruValue	_PPPCC(_PPPCC_WRITE, 30)
#define	scePPPCC_SetAllowAccmValue	_PPPCC(_PPPCC_WRITE, 31)
#define	scePPPCC_SetAllowAuth		_PPPCC(_PPPCC_WRITE, 32)
#define	scePPPCC_SetAllowIpAddress	_PPPCC(_PPPCC_WRITE, 33)
#define	scePPPCC_SetAllowIpMask		_PPPCC(_PPPCC_WRITE, 34)
#define	scePPPCC_SetWantDNS1Nego	_PPPCC(_PPPCC_WRITE, 35)
#define	scePPPCC_SetWantDNS2Nego	_PPPCC(_PPPCC_WRITE, 36)
#define	scePPPCC_SetWantDNS1		_PPPCC(_PPPCC_WRITE, 37)
#define	scePPPCC_SetWantDNS2		_PPPCC(_PPPCC_WRITE, 38)
#define	scePPPCC_SetAllowDNS1Nego	_PPPCC(_PPPCC_WRITE, 39)
#define	scePPPCC_SetAllowDNS2Nego	_PPPCC(_PPPCC_WRITE, 40)
#define	scePPPCC_SetAllowDNS1		_PPPCC(_PPPCC_WRITE, 41)
#define	scePPPCC_SetAllowDNS2		_PPPCC(_PPPCC_WRITE, 42)
#define	scePPPCC_SetConnectTimeout	_PPPCC(_PPPCC_WRITE, 43)
#define	scePPPCC_SetDialingType		_PPPCC(_PPPCC_WRITE, 44)
#if 0
#define	scePPPCC_SetZeroPrefix		_PPPCC(_PPPCC_WRITE, 45)
#endif
#define	scePPPCC_SetToneDial		_PPPCC(_PPPCC_WRITE, 46)
#define	scePPPCC_SetPulseDial		_PPPCC(_PPPCC_WRITE, 47)
#define	scePPPCC_SetAnyDial		_PPPCC(_PPPCC_WRITE, 48)
#if 0
#define	scePPPCC_SetZeroPrefixOff	_PPPCC(_PPPCC_WRITE, 49)
#define	scePPPCC_SetZeroPrefixOn	_PPPCC(_PPPCC_WRITE, 50)
#endif
#define	scePPPCC_SetLogFlags		_PPPCC(_PPPCC_WRITE, 51)
/* 52 */
/* 53 */
#define	scePPPCC_SetOmitEmptyFrame	_PPPCC(_PPPCC_WRITE, 54)
#define	scePPPCC_SetChapType		_PPPCC(_PPPCC_WRITE, 55)
/* 56 */
#define	scePPPCC_SetLcpEchoTimeout	_PPPCC(_PPPCC_WRITE, 57)
#define	scePPPCC_SetDefaultMTU		_PPPCC(_PPPCC_WRITE, 58)
#define	scePPPCC_SetAuthTimeout		_PPPCC(_PPPCC_WRITE, 59)
#define	scePPPCC_SetLcpMaxTerminate	_PPPCC(_PPPCC_WRITE, 60)
#define	scePPPCC_SetIpcpMaxTerminate	_PPPCC(_PPPCC_WRITE, 61)
#define	scePPPCC_SetLcpMaxConfigure	_PPPCC(_PPPCC_WRITE, 62)
#define	scePPPCC_SetIpcpMaxConfigure	_PPPCC(_PPPCC_WRITE, 63)
#define	scePPPCC_SetLcpMaxFailure	_PPPCC(_PPPCC_WRITE, 64)
#define	scePPPCC_SetAuthMaxFailure	_PPPCC(_PPPCC_WRITE, 65)
/* 66 */

#endif
