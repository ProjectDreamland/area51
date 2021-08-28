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

//=========================================================================
// DECLARING EDITORS
//=========================================================================
// Use the macro.
//=========================================================================
struct reg_editor
{
    reg_editor( const char* pEditorName, const char* pTypeAssociation ) :
        m_pEditorName       ( pEditorName ),
        m_pTypeAssociation  ( pTypeAssociation )
        {
            m_pNext = m_pHead;
            m_pHead = this;
        }

    struct on_open
    {
        char        Title[256];
        void*       pData;
    };

    virtual CMultiDocTemplate* Register( CWinApp* pWinApp )=0;
    CDocument* CreateInstance( const char* pString, void* pData )
    {
        on_open OnOpen;
        x_strcpy( OnOpen.Title, pString );
        OnOpen.pData  = pData;
        return m_pTemplate->OpenDocumentFile( (char*)&OnOpen );
    }

    inline static reg_editor* FindEditorByName( const char* pEditorName )
    {
        ASSERT(pEditorName);
        reg_editor* pEditor = reg_editor::m_pHead;
        while( pEditor )
        {            
            if( x_stricmp( pEditorName, pEditor->m_pEditorName ) == 0 )
                break;
            pEditor = pEditor->m_pNext;
        }
        return pEditor;
    }

    inline static reg_editor* FindEditorByType( const char* pEditorType )
    {
        ASSERT(pEditorType);
        reg_editor* pEditor = reg_editor::m_pHead;
        while( pEditor )
        {            
            if( x_stricmp( pEditorType, pEditor->m_pTypeAssociation ) == 0 )
                break;
            pEditor = pEditor->m_pNext;
        }
        return pEditor;
    }


    reg_editor*             m_pNext;
    const char*             m_pEditorName;
    const char*             m_pTypeAssociation;
    CMultiDocTemplate*      m_pTemplate;
    CWinApp*                m_pWinApp;
    static reg_editor*      m_pHead;
};


#define REG_EDITOR( VAR_NAME, EDITOR_NAME, RSC_TYPE, EDITOR_ID_NAME, EDITOR_DOC, EDITOR_FRAME, EDITOR_VIEW ) \
struct EDITOR_ID_NAME##my_editor : public reg_editor                                               \
{                                                                                                  \
    EDITOR_ID_NAME##my_editor( void ) : reg_editor( EDITOR_NAME, RSC_TYPE ) {}                     \
    virtual CMultiDocTemplate* Register( CWinApp* pWinApp )                                        \
    {                                                                                              \
        m_pWinApp   = pWinApp;                                                                     \
        m_pTemplate = new CMultiDocTemplate( EDITOR_ID_NAME,                                       \
		                                     RUNTIME_CLASS(EDITOR_DOC),                            \
		                                     RUNTIME_CLASS(EDITOR_FRAME),                          \
		                                     RUNTIME_CLASS(EDITOR_VIEW));                          \
        ASSERT( m_pTemplate );                                                                     \
        return m_pTemplate;                                                                        \
    }                                                                                              \
};                                                                                                 \
EDITOR_ID_NAME##my_editor VAR_NAME;                                                                \


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7F20BC63_3C1A_4AEB_AB58_A6DB2750F851__INCLUDED_)
