//=========================================================================
//
// GenericDialog.cpp
//
//=========================================================================
//#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include <stdio.h>  // for printf
#include <richedit.h>

#include <io.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "GenericDialog.hpp"
#include "x_files.hpp"
#include "x_bytestream.hpp"

//=========================================================================

#define BASIC_DLG_BW  70
#define BASIC_DLG_BH  15
#define BASIC_DLG_S   8
#define BASIC_DLG_H   250
#define BASIC_DLG_MIN_WIDTH 500

//=========================================================================

static const generic_dialog* s_pDialog = NULL;

//=========================================================================

generic_dialog::generic_dialog( void )
{
    Clear();
}

//=========================================================================

generic_dialog::~generic_dialog( void )
{
    Clear();
}

//=========================================================================

void generic_dialog::Clear( void )
{
    if( s_pDialog == this )
        s_pDialog = NULL;

    m_pTitle        = NULL;
    m_pMessage      = NULL;
    m_nButtons      = 0;
    x_memset( m_pButtonTitle, 0, sizeof(m_pButtonTitle) );
    m_Result        = -1;
    m_iButton_OK    = -1;
    m_iButton_CANCEL= -1;
}

//=========================================================================

void generic_dialog::SetTitle( const char* pTitle )
{
    m_pTitle = pTitle;
}

//=========================================================================

void generic_dialog::SetMessage( const char* pMessage )
{
    m_pMessage = pMessage;
}

//=========================================================================

void generic_dialog::AppendButton( const char* pButtonTitle )
{
    ASSERT( m_nButtons < 8 );

    if( x_stricmp(pButtonTitle,"OK")==0 )
        m_iButton_OK = m_nButtons;

    if( x_stricmp(pButtonTitle,"CANCEL")==0 )
        m_iButton_CANCEL = m_nButtons;

    m_pButtonTitle[ m_nButtons ] = pButtonTitle;
    m_nButtons++;
}

//=========================================================================

s32 generic_dialog::GetResult( void ) const
{
    return m_Result;
}

//=========================================================================

s32 generic_dialog::GetNButtons( void ) const
{
    return m_nButtons;
}

//=========================================================================

const char* generic_dialog::GetTitle( void ) const
{
    return m_pTitle;
}

//=========================================================================

const char* generic_dialog::GetMessage( void ) const
{
    return m_pMessage;
}

//=========================================================================

const char* generic_dialog::GetButtonTitle( s32 iButton ) const
{
    ASSERT( (iButton>=0) && (iButton<m_nButtons) );
    return m_pButtonTitle[iButton];
}

//=========================================================================

//static 
INT_PTR CALLBACK DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( s_pDialog == NULL )
        return FALSE;

    switch( uMsg )
    {
        case WM_INITDIALOG:
            {
                // Set Dialog title
                ::SetWindowText( hWnd, s_pDialog->GetTitle() );

                // Find edit control and set message
                HWND hWndEdit = ::GetDlgItem( hWnd, s_pDialog->GetNButtons()+100 );
                if( hWndEdit )
                {
                    ::SendMessage( hWndEdit, EM_SETBKGNDCOLOR, 0, ::GetSysColor( COLOR_BTNFACE ) );
                    ::SetWindowText( hWndEdit, s_pDialog->GetMessage() );
                }
            }
            break;

        case WM_CLOSE:
            s_pDialog = NULL;
            ::EndDialog( hWnd, IDCANCEL );
            return TRUE;
            break;

        case WM_KEYDOWN:
            {
                if( wParam == VK_ESCAPE )
                {
                    s_pDialog = NULL;
                    ::EndDialog( hWnd, IDCANCEL );
                    return TRUE;
                }
            }
            break;

        case WM_COMMAND:
            {
                if( (wParam>=100) && (wParam<=(u32)(100+s_pDialog->GetNButtons())) )
                {
                    s_pDialog = NULL;
                    ::EndDialog( hWnd, wParam );
                    return TRUE;
                }
            }
            break;
    }

    return FALSE;
}

//=========================================================================

s32 generic_dialog::Execute( void )
{
    xwstring ClassName;
    xwstring Name;

    // Load the RichEdit control dll
    HMODULE hMod = LoadLibrary("Riched20.dll");

    // Fill out values not setup by user
    {
        if( !m_pTitle   )   m_pTitle    = "NO TITLE";
        if( !m_pMessage )   m_pMessage  = "NO MESSAGE";
        if( !m_nButtons )
        {
            m_nButtons = 1;
            m_pButtonTitle[0] = "OK";
            m_iButton_OK = 0;
        }
    }

    // Compute dialog width based on buttons
    s32 DialogW = (BASIC_DLG_S + (BASIC_DLG_BW + BASIC_DLG_S) * m_nButtons);
    DialogW = MAX( DialogW, BASIC_DLG_MIN_WIDTH );
    s32 DialogWCenter = (DialogW/2) - (((BASIC_DLG_BW + BASIC_DLG_S) * m_nButtons)/2);

    xbytestream b;

    // Create the dialog box template
    /* dlgVer */    b << (u16)1;
    /* signature */ b << (u16)0xFFFF;
    /* helpID */    b << (u32)0;
    /* exStyle */   b << (u32)0;
    /* dwStyle */   b << (u32)( DS_MODALFRAME | DS_CENTER | DS_SHELLFONT | WS_POPUP | WS_CAPTION | WS_VISIBLE | WS_SYSMENU );
    /* cDlgItems */ b << (u16)(m_nButtons+1);
    /* x */         b << (u16)0;
    /* y */         b << (u16)0;
    /* cx */        b << (u16)DialogW;
    /* cy */        b << (u16)BASIC_DLG_H;
    /* menu */      b << (u16)0;
    /* class */     b << (u16)0;
    /* title */     b << (u16)0;
    /* pointsize */ b << (u16)8;
    /* weight */    b << (u16)400;
    /* italic */    b << (u8)0;
    /* charset */   b << (u8)1;
                    Name = xstring("MS Shell Dlg");
    /* font */      b.Append( (byte*)(const xwchar*)Name, Name.GetLength()*2+2 );

    // Control ID
    s32 ID = 100;

    // Add buttons
    for( s32 i=0 ; i<m_nButtons ; i++ )
    {
        // Align to DWORD boundry
        while( (b.GetLength() & 3) != 0 )
            b.Append( (byte)0 );

        // Get button name
        Name = xwstring( m_pButtonTitle[i] );

        /* helpID */    b << (u32)0;
        /* exStyle */   b << (u32)0;
        /* style */     b << (u32)( BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | ((i==0)?BS_DEFPUSHBUTTON:0) );
        /* x */         b << (u16)(DialogWCenter + (BASIC_DLG_BW + BASIC_DLG_S)*i);//(u16)(DialogW - (BASIC_DLG_BW + BASIC_DLG_S)*(m_nButtons-i));
        /* y */         b << (u16)(BASIC_DLG_H - BASIC_DLG_S - BASIC_DLG_BH);
        /* cx */        b << (u16)BASIC_DLG_BW;
        /* cy */        b << (u16)BASIC_DLG_BH;
        /* id */        b << (u32)ID++;
                        ClassName = xstring("BUTTON");
        /* class */     b.Append( (const byte*)(const xwchar*)ClassName, ClassName.GetLength()*2+2 );
        /* class */     //b << (u16)0xffff;
        /* class */     //b << (u16)0x080;
        /* title */     b.Append( (const byte*)(const xwchar*)Name, Name.GetLength()*2+2 );
        /* create */    b << (u16)0;
    }

    // Align to DWORD boundry
    while( (b.GetLength() & 3) != 0 )
        b.Append( (byte)0 );

    /* helpID */    b << (u32)0;
    /* exStyle */   b << (u32)( /*WS_EX_TRANSPARENT |*/ WS_EX_CLIENTEDGE );
    /* style */     b << (u32)( WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | WS_VSCROLL | ES_READONLY | WS_TABSTOP );
    /* x */         b << (u16)BASIC_DLG_S;
    /* y */         b << (u16)BASIC_DLG_S;
    /* cx */        b << (u16)(DialogW - BASIC_DLG_S*2);
    /* cy */        b << (u16)(BASIC_DLG_H - BASIC_DLG_S*3 - BASIC_DLG_BH);
    /* id */        b << (u32)ID++;
                    ClassName = xstring("RichEdit20A");
//                    ClassName = xstring("EDIT");
    /* class */     b.Append( (const byte*)(const xwchar*)ClassName, ClassName.GetLength()*2+2 );
                    Name = xstring("");
    /* title */     b.Append( (const byte*)(const xwchar*)Name, Name.GetLength()*2+2 );
    /* create */    b << (u16)0;

    // Set this dialog up as the current active
//    ASSERT( s_pDialog == NULL );
    s_pDialog = this;

    // Execute the dialog
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle( NULL );
    s32 Result = ::DialogBoxIndirect( hInstance, (LPCDLGTEMPLATE)b.GetBuffer(), NULL, DialogProc );

    m_Result = -1;
    if( Result >= 100       ) m_Result = Result % 100;
    if( Result == IDOK      ) m_Result = m_iButton_OK;
    if( Result == IDCANCEL  ) m_Result = m_iButton_CANCEL;
        
    s_pDialog = NULL;

    // Return the dialog result
    return m_Result;
}

//=========================================================================

s32 generic_dialog::Execute_OK( const char* pTitle, const char* pMessage )
{
    Clear();
    SetTitle( pTitle );
    SetMessage( pMessage );
    AppendButton( "OK" );
    return Execute();
}

//=========================================================================

s32 generic_dialog::Execute_OK_CANCEL( const char* pTitle, const char* pMessage )
{
    Clear();
    SetTitle( pTitle );
    SetMessage( pMessage );
    AppendButton( "OK" );
    AppendButton( "CANCEL" );
    return Execute();
}

//=========================================================================
