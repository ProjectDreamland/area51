//=========================================================================
//
//  ui_text.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_text.hpp"
#include "ui_manager.hpp"
#include "ui_font.hpp"

//=========================================================================
//  Defines
//=========================================================================

#ifdef TARGET_XBOX
extern u32 g_TEdge;
#define DIALOG_TOP (g_TEdge)
#define DIALOG_BOTTOM (DIALOG_TOP+(448-72))
#else
#define DIALOG_TOP 24
#define DIALOG_BOTTOM 448-72
#endif

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_text_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_text* pButton = new ui_text;
    pButton->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pButton;
}

//=========================================================================
//  ui_text
//=========================================================================

ui_text::ui_text( void )
{
    m_LabelColor = XCOLOR_WHITE;
}

//=========================================================================

ui_text::~ui_text( void )
{
    Destroy();
}

//=========================================================================

xbool ui_text::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_useSmallText = FALSE;

    return Success;
}

//=========================================================================

void ui_text::Render( s32 ox, s32 oy )
{
    s32     State = ui_manager::CS_NORMAL;
    s32     FontID;

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  TextColor1 = XCOLOR_WHITE;
        xcolor  TextColor2 = XCOLOR_BLACK;

        // Calculate rectangle
        irect    r;
        if( m_LabelFlags & ui_font::is_help_text )
        {
            if( m_LabelFlags & ui_font::set_position )
            {
                r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );
            }
            else
            {
            #ifdef TARGET_XBOX
                s32 Top = DIALOG_TOP+374;
                r.Set( 0, Top, 512, Top+40 );
            #else
                r.Set( 0, 400 * g_UiMgr->GetScaleY(), 512, 440 * g_UiMgr->GetScaleY() );
            #endif
                s32 XRes, YRes;
                eng_GetRes( XRes, YRes );
                    
                s32 midX = XRes>>1;
                //s32 midY = YRes>>1;
                //
                s32 dx = midX - 256;
                //s32 dy = midY - 224;
                //
                //r.Translate( dx, dy );

#if defined( TARGET_PC) && !defined( X_EDITOR )
                r.Set( 0, YRes - (40 * (YRes-8) / 448), XRes, YRes-8 );
                dx = 0;
#endif

                r.Translate( dx, 0 );
            }
        }
        else
        {
            r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );
        }

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            State = ui_manager::CS_DISABLED;
            TextColor1 = XCOLOR_GREY;
            TextColor2 = xcolor(0,0,0,0);
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_HIGHLIGHT )
        {
            State = ui_manager::CS_HIGHLIGHT;
            TextColor1 = GetLabelColor();
            TextColor2 = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            TextColor1 = GetLabelColor();
            TextColor2 = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            TextColor1 = GetLabelColor();
            TextColor2 = XCOLOR_BLACK;
        }
        else
        {
            State = ui_manager::CS_NORMAL;
            TextColor1 = GetLabelColor();
            TextColor2 = XCOLOR_BLACK;
        }

        // Render Text
        if (m_useSmallText)
        {
            FontID = g_UiMgr->FindFont("small");
        }
        else
        {
            FontID = g_UiMgr->FindFont("large");
        }

        // debug!
        //g_UiMgr->RenderRect( r, XCOLOR_GREY, TRUE );

        if( TextColor1.R == 255 && TextColor1.G == 255 && TextColor1.B == 255 )
		{
            //r.Translate( 2, -2 );
            //m_pManager->RenderText( FontID, r, m_LabelFlags, TextColor2, m_Label );
            //r.Translate( -2, -2 );
            m_pManager->RenderText( FontID, r, m_LabelFlags, TextColor1, m_Label );
        }
        else
        {
            //r.Translate( 1, -1 );
            //m_pManager->RenderText( FontID, r, m_LabelFlags, TextColor2, m_Label );
            //r.Translate( -1, -1 );
            m_pManager->RenderText( FontID, r, m_LabelFlags, TextColor1, m_Label );
        }


        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_text::OnUpdate( f32 DeltaTime )
{
    (void)DeltaTime;
}

//=========================================================================
														 