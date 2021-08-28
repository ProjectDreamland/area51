//=========================================================================
//
//  ui_win.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"

#include "ui_win.hpp"
#include "ui_manager.hpp"
#include "ui_font.hpp"

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  ui_win
//=========================================================================

ui_win::ui_win( void )
{
    m_ID    = -1;
    m_Flags = 0;
    m_Font  = 0;
    m_LabelColor = XCOLOR_WHITE;
}

//=========================================================================

ui_win::~ui_win( void )
{
    Destroy();
}

//=========================================================================

xbool ui_win::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success = TRUE;

    m_UserID         = UserID;
    m_pManager       = pManager;
    m_pParent        = pParent;
    m_CreatePosition = Position;
    m_Position       = Position;
    m_Flags          = Flags;
    m_LabelFlags     = ui_font::h_center|ui_font::v_center;
    m_LabelColor     = XCOLOR_WHITE;

    // Add child entry to parent window if we have a parent
    if( m_pParent )
        m_pParent->m_Children.Append() = this;

    return Success;
}

//=========================================================================

void ui_win::Destroy( void )
{
    s32     i;
    s32     iFound = -1;

    // Kill Children
    while( m_Children.GetCount() > 0 )
    {
        ui_win* pChild = m_Children[0];
        delete pChild;
    }

    if( m_pParent )
    {
        // Find in parents child list and remove
        for( i=0 ; i<m_pParent->m_Children.GetCount() ; i++ )
        {
            if( m_pParent->m_Children[i] == this )
                iFound = i;
        }
        if( iFound != -1 )
        {
            m_pParent->m_Children.Delete( iFound );
        }

        // Clear Parent pointer
        m_pParent = NULL;
    }
}

//=========================================================================

void ui_win::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( ox, oy );
        }
    }
}

//=========================================================================

void ui_win::SetPosition( const irect& Position )
{
    m_Position = Position;
}

//=========================================================================

const irect& ui_win::GetPosition( void ) const
{
    return m_Position;
}

//=========================================================================

const irect& ui_win::GetCreatePosition( void ) const
{
    return m_Position;
}

//=========================================================================

s32 ui_win::GetWidth( void ) const
{
    return m_Position.GetWidth();
}

//=========================================================================

s32 ui_win::GetHeight( void ) const
{
    return m_Position.GetHeight();
}

//=========================================================================

ui_win* ui_win::GetWindowAtXY( s32 x, s32 y ) const
{
    ui_win* pFound = NULL;

    // Don't process for STATIC, DISABLED or INVISIBLE windows
    if( !(m_Flags & WF_STATIC) && !(m_Flags & WF_DISABLED) && (m_Flags & WF_VISIBLE) )
    {
        // Check if the coordinates hit our rectangle
        if( ((x >= 0) && (x < m_Position.GetWidth())) &&
            ((y >= 0) && (y < m_Position.GetHeight())) )
        {
            // Loop through all children testing
            for( s32 i=0 ; (i<m_Children.GetCount()) && !pFound ; i++ )
            {
                irect r = m_Children[i]->GetPosition();
                pFound = m_Children[i]->GetWindowAtXY( x - r.l, y - r.t );
            }

            // If no child found then return this window
            if( pFound == NULL )
                pFound = (ui_win*)this;
        }
    }

    // Return window found
    return pFound;
}

//=========================================================================

void ui_win::SetFlags( s32 Flags )
{
    m_Flags = Flags;
}

//=========================================================================

s32 ui_win::GetFlags( void ) const
{
    return m_Flags;
}

//=========================================================================

void ui_win::SetFlag( s32 Flag, s32 State )
{
    if( State )
        m_Flags |= Flag;
    else
        m_Flags &= ~Flag;
}

//=========================================================================

s32 ui_win::GetFlags( s32 Flag )
{
    return m_Flags & Flag;
}

//=========================================================================

void ui_win::SetLabel( const xwstring& Text )
{
    m_Label = Text;
}

//=========================================================================

void ui_win::SetLabel( const xwchar* Text )
{
    m_Label = Text;
}

//=========================================================================

const xwstring& ui_win::GetLabel( void ) const
{
    return m_Label;
}

//=========================================================================

void ui_win::SetLabelFlags( u32 Flags )
{
    m_LabelFlags = Flags;
}

//=========================================================================

void ui_win::SetControlID( s32 ID )
{
    m_ID = ID;
}

//=========================================================================

s32 ui_win::GetControlID( void ) const
{
    return m_ID;
}

//=========================================================================
void ui_win::SetLabelColor(const xcolor& color)
{
    m_LabelColor = color;
}

//=========================================================================
const xcolor& ui_win::GetLabelColor(void) const
{
    return m_LabelColor;
}

/*
//=========================================================================

void ui_win::SetText( const xstring& Text )
{
    m_Label = Text;
}

//=========================================================================

void ui_win::SetText( const char* Text )
{
    m_Label = Text;
}

//=========================================================================

const xstring& ui_win::GetText( void ) const
{
    return m_Label;
}

//=========================================================================

void ui_win::SetTextFlags( u32 Flags )
{
    m_LabelFlags = Flags;
}

*/
//=========================================================================

void ui_win::SetParent( ui_win* pParent )
{
    s32     i;
    s32     iFound = -1;

    // Remove from previous parent
    if( m_pParent )
    {
        // Find in parents child list and remove
        for( i=0 ; i<m_pParent->m_Children.GetCount() ; i++ )
        {
            if( m_pParent->m_Children[i] == this )
                iFound = i;
        }
        if( iFound != -1 )
        {
            m_pParent->m_Children.Delete( iFound );
        }
    }

    // Add to new parent
    m_pParent = pParent;
    m_pParent->m_Children.Append() = this;
}

//=========================================================================

ui_win* ui_win::GetParent( void ) const
{
    return m_pParent;
}

/*
//=========================================================================

ui_win* ui_win::FindChildByLabel( const xstring& Label ) const
{
    for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
    {
        ui_win* pChild = m_Children[i];
        if( pChild->GetLabel() == Label )
            return pChild;
    }

    return NULL;
}

//=========================================================================

ui_win* ui_win::FindChildByLabel( const char* Label ) const
{
    return FindChildByLabel( xstring( Label ) );
}
*/
//=========================================================================

ui_win* ui_win::FindChildByID( s32 ID ) const
{
    for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
    {
        ui_win* pChild = m_Children[i];
        if( pChild->m_ID == ID )
            return pChild;
    }

    // Failed to find the child
    ASSERT( 0 );

    return NULL;
}

//=========================================================================

xbool ui_win::IsChildOf( ui_win* pParent ) const
{
    if( m_pParent )
    {
        if( m_pParent == pParent )
            return TRUE;
        else
            return m_pParent->IsChildOf( pParent );
    }

    return FALSE;
}

//=========================================================================

void ui_win::LocalToScreen( s32& x, s32& y ) const
{
    x += m_Position.l;
    y += m_Position.t;
    if( m_pParent )
    {
        m_pParent->LocalToScreen( x, y );
    }
}

//=========================================================================

void ui_win::ScreenToLocal( s32& x, s32& y ) const
{
    x -= m_Position.l;
    y -= m_Position.t;
    if( m_pParent )
    {
        m_pParent->ScreenToLocal( x, y );
    }
}

//=========================================================================

void ui_win::LocalToScreen( irect& r ) const
{
    r.l += m_Position.l;
    r.r += m_Position.l;
    r.t += m_Position.t;
    r.b += m_Position.t;
    if( m_pParent )
    {
        m_pParent->LocalToScreen( r );
    }
}

//=========================================================================

void ui_win::ScreenToLocal( irect& r ) const
{
    r.l -= m_Position.l;
    r.r -= m_Position.l;
    r.t -= m_Position.t;
    r.b -= m_Position.t;
    if( m_pParent )
    {
        m_pParent->ScreenToLocal( r );
    }
}

//=========================================================================

void ui_win::LocalToScreenCreate( irect& r ) const
{
    irect r2 = GetCreatePosition();
    r.l += r2.l;
    r.r += r2.l;
    r.t += r2.t;
    r.b += r2.t;
    if( m_pParent )
    {
        m_pParent->LocalToScreenCreate( r );
    }
}

//=========================================================================

void ui_win::ScreenToLocalCreate( irect& r ) const
{
    irect r2 = GetCreatePosition();
    r.l -= r2.l;
    r.r -= r2.l;
    r.t -= r2.t;
    r.b -= r2.t;
    if( m_pParent )
    {
        m_pParent->ScreenToLocalCreate( r );
    }
}

//=========================================================================
//=========================================================================
//  Message Handler Functions
//=========================================================================
//=========================================================================

void ui_win::OnUpdate( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // Render children
    for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
    {
        m_Children[i]->OnUpdate( m_Children[i], DeltaTime );
    }
}

//=========================================================================

void ui_win::OnNotify( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnNotify( pWin, pSender, Command, pData );
}

//=========================================================================

void ui_win::OnLBDown( ui_win* pWin )
{
    (void)pWin;
    
    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadSelect( pWin );
}

//=========================================================================

void ui_win::OnLBUp( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void ui_win::OnMBDown( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void ui_win::OnMBUp( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void ui_win::OnRBDown( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void ui_win::OnRBUp( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void ui_win::OnCursorMove( ui_win* pWin, s32 x, s32 y )
{
    (void)pWin;
    (void)x;
    (void)y;
}

//=========================================================================

void ui_win::OnCursorEnter( ui_win* pWin )
{
    (void)pWin;
    m_Flags |= WF_HIGHLIGHT;

#ifndef TARGET_PC
//    audio_Play( SFX_FRONTEND_CURSOR_MOVE_01,AUDFLAG_CHANNELSAVER );	//-- Jhowa
#endif
}

//=========================================================================

void ui_win::OnCursorExit( ui_win* pWin )
{
    (void)pWin;
    m_Flags &= ~WF_HIGHLIGHT;
}

//=========================================================================

void ui_win::OnKeyDown( ui_win* pWin, s32 Key )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnKeyDown( pWin, Key );
}

//=========================================================================

void ui_win::OnKeyUp( ui_win* pWin, s32 Key )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnKeyUp( pWin, Key );
}

//=========================================================================

void ui_win::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void ui_win::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadSelect( pWin );
}

//=========================================================================

void ui_win::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadBack( pWin );
}

//=========================================================================

void ui_win::OnPadDelete( ui_win* pWin )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadDelete( pWin );
}

//=========================================================================

void ui_win::OnPadHelp( ui_win* pWin )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadHelp( pWin );
}

//=========================================================================

void ui_win::OnPadActivate( ui_win* pWin )
{
    (void)pWin;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadActivate( pWin );
}

//=========================================================================

void ui_win::OnPadShoulder( ui_win* pWin, s32 Direction )
{
    (void)pWin;
    (void)Direction;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadShoulder( pWin, Direction );
}

//=========================================================================

void ui_win::OnPadShoulder2( ui_win* pWin, s32 Direction )
{
    (void)pWin;
    (void)Direction;

    // Pass up chain to parent
    if( m_pParent )
        m_pParent->OnPadShoulder2( pWin, Direction );
}

//=========================================================================
