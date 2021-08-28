//=========================================================================
//
//  ui_button.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_button.hpp"
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

// define the global button colors
xcolor   ui_button::m_TextColorNormal;
xcolor   ui_button::m_TextColorHighlight;
xcolor   ui_button::m_TextColorDisabled;
xcolor   ui_button::m_TextColorShadow;

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_button_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_button* pButton = new ui_button;
    pButton->Create( UserID, pManager, Position, pParent, Flags );
    return (ui_win*)pButton;
}

//=========================================================================
//  ui_button
//=========================================================================

ui_button::ui_button( void )
{
}

//=========================================================================

ui_button::~ui_button( void )
{
    Destroy();
}

//=========================================================================

xbool ui_button::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_iElement = m_pManager->FindElement( "button" );
    ASSERT( m_iElement != -1 );
    
    // Initialize flags
    m_useSmallText   = FALSE;
    m_useNativeColor = FALSE;
    m_bPulseOn       = FALSE;
    m_bPulseUp       = FALSE;
    m_PulseRate      = 1024.0f;
    m_PulseValue     = 255.0f;

    // Initialize bitmap ID
    m_BitmapID = -1;

    // clear data
    m_Data = 0;

    return Success;
}

//=========================================================================

void ui_button::Render( s32 ox, s32 oy )
{
    s32     State = ui_manager::CS_NORMAL;
    s32     FontID;

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  TextColor1 = m_TextColorNormal;
        xcolor  TextColor2 = m_TextColorShadow;
      
        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            State = ui_manager::CS_DISABLED;
            TextColor1 = m_TextColorDisabled; 
            TextColor2 = xcolor(0,0,0,0);
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_HIGHLIGHT )
        {
            State = ui_manager::CS_HIGHLIGHT;
            TextColor1 = m_TextColorHighlight;
            TextColor2 = m_TextColorShadow;   
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            TextColor1 = m_TextColorHighlight; 
            TextColor2 = m_TextColorShadow;    
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            TextColor1 = m_TextColorHighlight; 
            TextColor2 = m_TextColorShadow;    
        }
        else						
        {
            State = ui_manager::CS_NORMAL;
            TextColor1 = m_TextColorNormal;    
            TextColor2 = m_TextColorShadow;    
        }

        // check to see if we should render the button artwork
        if (m_Flags & WF_BORDER)
        {
            m_pManager->RenderElement( m_iElement, r, State );

            // Add Highlight to list
            if( m_Flags & WF_HIGHLIGHT )
                m_pManager->AddHighlight( m_UserID, r );
        }
        else if( m_useSmallText )
        {
            if ( m_Flags & WF_HIGHLIGHT )
            {
                TextColor1 = xcolor(0,0,0,255);
                TextColor2 = xcolor(0,0,0,0);

                s32 alpha = 128 + (g_UiMgr->GetHighlightAlpha(8) * 8); // 64<->192
                m_pManager->RenderRect( r, xcolor(79,214,60,alpha), FALSE );
            }
        }
        else
        {
            if( m_BitmapID != -1 )
            {
                if( m_Flags & WF_HIGHLIGHT )
                {
                    irect r2 = r;
                    r2.t -= 2;
                    r2.l -= 2;
                    r2.b += 2;
                    r2.r += 2;

                    m_pManager->RenderRect( r2, TextColor1, FALSE );
                }

                // render the button bitmap
                if( m_useNativeColor )
                {
                    m_pManager->RenderBitmap( m_BitmapID, r, XCOLOR_WHITE );
                }
                else
                {
                    m_pManager->RenderBitmap( m_BitmapID, r, TextColor1 );
                }
            }
        }

        // Render Text
        s32 justFlags=0;
        if( m_Flags & WF_BUTTON_LEFT )
            justFlags = (ui_font::h_left|ui_font::v_center);
        else if( m_Flags & WF_BUTTON_RIGHT )
            justFlags = (ui_font::h_right|ui_font::v_center);
        else
            justFlags = (ui_font::h_center|ui_font::v_center);


        // Render Text
        if( m_useSmallText )
        {
            FontID = g_UiMgr->FindFont("small");
        }
        else
        {
            FontID = g_UiMgr->FindFont("large");
        }

        // check for pulsing
        if( m_bPulseOn )
        {
            TextColor1.A = (u8)m_PulseValue;
        }

        if( m_Flags & WF_HIGHLIGHT )
		{
			r.Translate( 1, -1 );
			m_pManager->RenderText( FontID, r, justFlags, TextColor2, m_Label );
			r.Translate( -1, -1 );
			m_pManager->RenderText( FontID, r, justFlags, TextColor1, m_Label );
		}
		else
		{
			r.Translate( 1, -1 );
			m_pManager->RenderText( FontID, r, justFlags, TextColor2, m_Label );
			r.Translate( -1, -1 );
			m_pManager->RenderText( FontID, r, justFlags, TextColor1, m_Label );
		}


        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_button::OnUpdate( f32 DeltaTime )
{
    (void)DeltaTime;

    // update color pulsing
    if( m_bPulseOn )
    {
        if( m_bPulseUp )
        {
            m_PulseValue += DeltaTime * m_PulseRate;

            if( m_PulseValue >= 255.0f )
            {
                m_PulseValue = 255.0f;
                m_bPulseUp = FALSE;
            }
        }
        else
        {
            m_PulseValue -= DeltaTime * m_PulseRate;

            if( m_PulseValue <= 128.0f )
            {
                m_PulseValue = 128.0f;
                m_bPulseUp = TRUE;
            }
        }
    }
}

//=========================================================================
