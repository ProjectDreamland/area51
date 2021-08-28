// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7F20BC63_3C1A_4AEB_AB58_A6DB2750F851__INCLUDED_)
#define AFX_STDAFX_H__7F20BC63_3C1A_4AEB_AB58_A6DB2750F851__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Build for Windows XP
#if _MSC_VER >= 1300
#define _WIN32_WINNT 0x0501
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

//=========================================================================
// X_FILES
#include "x_files.hpp"
#undef ASSERT
#undef VERIFY
// X_FILES
//=========================================================================

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#if _MSC_VER >= 1200
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions

#include <XTToolKit.h>      // Xtreme Toolkit MFC Extensions

//=========================================================================
// X_FILES
#ifdef X_ASSERT
#undef ASSERT
#undef VERIFY
#define ASSERT(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, NULL ) )  BREAK; } while( FALSE )
#define VERIFY(expr)        do{ if( !(expr) && RTFHandler( __FILE__, __LINE__, #expr, NULL ) )  BREAK; } while( FALSE )
#endif
// X_FILES
//=========================================================================

#include "Entropy.hpp"
#include "UserMessage.h"
#include "Parsing/TextIn.hpp"
#include "Parsing/TextOut.hpp"
#include "Parsing/tokenizer.hpp"
#include "memleak.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7F20BC63_3C1A_4AEB_AB58_A6DB2750F851__INCLUDED_)
