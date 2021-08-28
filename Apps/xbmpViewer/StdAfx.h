// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__57266317_F083_4EF4_B381_D8E3E56D7560__INCLUDED_)
#define AFX_STDAFX_H__57266317_F083_4EF4_B381_D8E3E56D7560__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//
// --- X_Files ---
//
#include "x_files.hpp"
#undef ASSERT
#undef VERIFY

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#if _MSC_VER >= 1200
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <XTToolKit.h>      // Xtreme Toolkit MFC Extensions

// Custom Message
enum CustomMessage
{
    NM_DIRCHANGED = WM_USER,
    NM_POPULATELIST,
    NM_REFRESHLIST,
    NM_NEWBITMAP,
    NM_NEWMIPLEVEL,
};

// Resource defines
#include "resource.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__57266317_F083_4EF4_B381_D8E3E56D7560__INCLUDED_)
