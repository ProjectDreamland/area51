// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__6A5C273E_38D1_4C2A_876A_DB8CE9B90DB7__INCLUDED_)
#define AFX_STDAFX_H__6A5C273E_38D1_4C2A_876A_DB8CE9B90DB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Build for Windows XP
#if _MSC_VER >= 1300
#define _WIN32_WINNT 0x0501
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//
// --- X_Files ---
//
#include "x_files.hpp"
#undef ASSERT
#undef VERIFY

#include <afxwin.h>         // MFC core and standard components
#include <winsock2.h>
#include <afxext.h>         // MFC extensions
#if _MSC_VER >= 1200
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions

#include <afxmt.h>          // MFC multi-threaded header

#include <XTToolKit.h>      // Xtreme Toolkit MFC Extensions

#include "Resource.h"

#include <shlwapi.h>
#include <imm.h>

#include <uxtheme.h>
#include <TmSchema.h>

#include "StringHelpers.h"

#include <gl/gl.h>
#include <gl/glu.h>

#define MEMORY_HEADER_SIZE  0 //32
#define MEMORY_GRANULARITY  1 //32

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__6A5C273E_38D1_4C2A_876A_DB8CE9B90DB7__INCLUDED_)
