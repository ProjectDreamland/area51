//==============================================================================
//
//  KeyBarKey.h
//
//	A keyframe structure that holds the data that the KeyBar needs to edit it
//
//==============================================================================

//----------------------+
//	Includes			|
//----------------------+

#include <stdafx.h>
#include "MouseMgr.h"

//===========================================================================

MouseMgr::MouseMgr( void )
 :  m_pCWnd         ( NULL ),
 
    m_Pos           ( 0, 0 ),
    m_ClickPos      ( 0, 0 ),
    m_DragPos       ( 0, 0 ),

    m_Wheel         ( 0 ),

    m_Button_Left   ( false ),
    m_Button_Middle ( false ),
    m_Button_Right  ( false ),

    m_Key_Shift     ( false ),
    m_Key_Control   ( false ),
    m_Key_Alt       ( false )
{

}

//===========================================================================

MouseMgr::~MouseMgr()
{

}

//===========================================================================

void    MouseMgr::AttachToWindow( CWnd* pCWnd )
{
    ASSERT( pCWnd );    // Make sure it's actually a window
    m_pCWnd = pCWnd;
}

//===========================================================================

void    MouseMgr::OnButtonDown( UINT nFlags, CPoint point )
{
    ASSERT( m_pCWnd );  // Must attach MouseMgr to a window before you can use it!!

    //m_pCWnd->SetFocus();    // Set Focus on this window...so we get all further keyboard input
    //m_pCWnd->SetCapture();  // Capture the mouse so this window gets updates until we let go of the button

    ::SetFocus  ( m_pCWnd->GetSafeHwnd() ); // Set Focus on this window...so we get all further keyboard input
    ::SetCapture( m_pCWnd->GetSafeHwnd() ); // Capture the mouse so this window gets updates until we let go

    GetMouseButtons( nFlags );
    GetModifierKeys( nFlags );

    // Update Position Info
    m_Pos       = point;
    m_ClickPos  = point;
    m_DragPos.x = 0;
    m_DragPos.y = 0;
}

//===========================================================================

void    MouseMgr::OnButtonUp( UINT nFlags, CPoint point )
{
    ASSERT( m_pCWnd );  // Must attach MouseMgr to a window before you can use it!!

    //if( m_pCWnd->GetCapture() == m_pCWnd )
    if( ::GetCapture() == m_pCWnd->GetSafeHwnd() )
    {
        ::ReleaseCapture();

        GetMouseButtons( nFlags );
        GetModifierKeys( nFlags );

        // Update Position Info
        m_Pos           = point;
        m_DragPos.x     = m_Pos.x - m_ClickPos.x;
        m_DragPos.y     = m_Pos.y - m_ClickPos.y;
    }
}

//===========================================================================

void    MouseMgr::OnMove( UINT nFlags, CPoint point )
{
    ASSERT( m_pCWnd );  // Must attach MouseMgr to a window before you can use it!!

    GetMouseButtons( nFlags );
    GetModifierKeys( nFlags );

    // Update Mouse Position Info
    m_Pos           = point;
    m_DragPos.x     = m_Pos.x - m_ClickPos.x;
    m_DragPos.y     = m_Pos.y - m_ClickPos.y;
}

//===========================================================================

void    MouseMgr::OnWheel( UINT nFlags, CPoint point, short zDelta )
{
    ASSERT( m_pCWnd );          // Must attach MouseMgr to a window before you can use it!!

    m_Wheel += (zDelta / 120);  // Set to +/- 1...Windows works in resolution of 120 for each wheel click

    GetMouseButtons( nFlags );
    GetModifierKeys( nFlags );
}

//===========================================================================

const CPoint&   MouseMgr::GetPos( void ) const
{
    return m_Pos;
}

//===========================================================================

const CPoint&   MouseMgr::GetClickPos( void ) const
{
    return m_ClickPos;
}

//===========================================================================

const CPoint&   MouseMgr::GetDragPos( void ) const
{
    return m_DragPos;
}

//===========================================================================

const xbool&    MouseMgr::GetButton_Left( void ) const
{
    return m_Button_Left;
}

//===========================================================================

const xbool&    MouseMgr::GetButton_Middle( void ) const
{
    return m_Button_Middle;
}

//===========================================================================

const xbool&    MouseMgr::GetButton_Right( void ) const
{
    return m_Button_Right;
}

//===========================================================================

const xbool&    MouseMgr::GetKey_Shift( void ) const
{
    return m_Key_Shift;
}

//===========================================================================

const xbool&    MouseMgr::GetKey_Control( void ) const
{
    return m_Key_Control;
}

//===========================================================================

const xbool&    MouseMgr::GetKey_Alt( void ) const
{
    return m_Key_Alt;
}

//===========================================================================

void    MouseMgr::GetMouseButtons( UINT nFlags )
{
    m_Button_Left   = ( nFlags & MK_LBUTTON ) ? true : false;
    m_Button_Middle = ( nFlags & MK_MBUTTON ) ? true : false;
    m_Button_Right  = ( nFlags & MK_RBUTTON ) ? true : false;
}

//===========================================================================

void    MouseMgr::GetModifierKeys( UINT nFlags )
{
    m_Key_Shift     = ( nFlags & MK_SHIFT )             ? true : false;
    m_Key_Control   = ( nFlags & MK_CONTROL )           ? true : false;
    m_Key_Alt       = ( GetKeyState(VK_MENU) & 0x8000 ) ? true : false;
}
