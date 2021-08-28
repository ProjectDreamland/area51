//=========================================================================
//
//  ui_bitmap.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_bitmap.hpp"
#include "ui_manager.hpp"

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
//  Factory function
//=========================================================================

ui_win* ui_bitmap_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_bitmap* pBitmap = new ui_bitmap;
    pBitmap->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pBitmap;
}

//=========================================================================
//  ui_bitmap
//=========================================================================

ui_bitmap::ui_bitmap( void )
{
    m_LabelColor = XCOLOR_WHITE;
}

//=========================================================================

ui_bitmap::~ui_bitmap( void )
{
    Destroy();
}

//=========================================================================

xbool ui_bitmap::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_BitmapID    = -1;
    m_bIsElement  = FALSE;
    m_RenderState = 0;

    return Success;
}

//=========================================================================

void ui_bitmap::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // render the bitmap
        if( m_BitmapID != -1 )
        {
            if( m_bIsElement )
            {
                g_UiMgr->RenderElement( m_BitmapID, r, m_RenderState, GetLabelColor() );
            }
            else
            {
                g_UiMgr->RenderBitmap( m_BitmapID, r, GetLabelColor() );
            }
        }

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_bitmap::OnUpdate( f32 DeltaTime )
{
    (void)DeltaTime;
}

//=========================================================================

void ui_bitmap::SetBitmap( s32 BitmapID, xbool bIsElement, s32 State )
{
    m_BitmapID    = BitmapID; 
    m_bIsElement  = bIsElement;
    m_RenderState = State;
}

//=========================================================================
														 