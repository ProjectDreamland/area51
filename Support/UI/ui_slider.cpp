//=========================================================================
//
//  ui_slider.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"

#include "ui_slider.hpp"
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
//  Factory function
//=========================================================================

ui_win* ui_slider_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_slider* pButton = new ui_slider;
    pButton->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pButton;
}

//=========================================================================
//  ui_slider
//=========================================================================

ui_slider::ui_slider( void )
{
}

//=========================================================================

ui_slider::~ui_slider( void )
{
    Destroy();
}

//=========================================================================

xbool ui_slider::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_iElementBar   = m_pManager->FindElement( "slider_bar"   );
    m_iElementThumb = m_pManager->FindElement( "slider_thumb" );
    ASSERT( m_iElementBar   != -1 );
    ASSERT( m_iElementThumb != -1 );
    m_Min   = 0;
    m_Max   = 100;
    m_Value = 0;
    m_ValueParametric = 0.0f;
    m_IsParametric = FALSE;
    m_Step  = 5;
    m_StepScaler    = 1;
    m_StepScalerMax = 1;
    m_UseSound = TRUE;

#ifdef TARGET_PC
    m_MouseDown = FALSE;
#endif

    return Success;
}

//=========================================================================

void ui_slider::Render( s32 ox, s32 oy )
{
    s32     State = ui_manager::CS_NORMAL;

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  TextColor1 = XCOLOR_WHITE;
        xcolor  TextColor2 = XCOLOR_BLACK;

        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

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
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        else
        {
            State = ui_manager::CS_NORMAL;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }

        // Determine Bar & Thumb positions and render
        irect r2 = r;
        r2.t += 7; 
        //r2.t = (r.t+r.b)/2 - 2;
        //r2.b = r2.t + 4;
        if( m_Max > m_Min )
            r.l += (r.r-r.l-5) * (m_Value-m_Min) / (m_Max-m_Min);
        r.r = r.l + 6;
        m_pManager->RenderElement( m_iElementBar,   r2, 0     );
        m_pManager->RenderElement( m_iElementThumb, r , State );

#ifdef TARGET_PC
        m_Thumb = r;
#endif
        // Add Highlight to list
        if( m_Flags & WF_HIGHLIGHT )
            m_pManager->AddHighlight( m_UserID, r );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_slider::OnUpdate( f32 DeltaTime )
{
    (void)DeltaTime;
}

//=========================================================================

void ui_slider::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    xbool       Processed = FALSE;
    s32         dx = 0;
    static s32  ScaleCounter = 0;

    // Determine movement required
    switch( Code )
    {
    case ui_manager::NAV_LEFT:
        {
            // Reset Scaler on Press
            if( Presses != 0 )
            {
                ScaleCounter = 0;
                m_StepScaler = 1;
            }
            dx = -m_Step;
        }
        break;
    case ui_manager::NAV_RIGHT:
        {
            // Reset Scaler on Press
            if( Presses != 0 )
            {
                ScaleCounter = 0;
                m_StepScaler = 1;
            }
            dx =  m_Step;
        }
    }

    // Check for scaling movement
    if( Presses == 0 )
    {
//        ScaleCounter++;
        if( ScaleCounter >= 10 )
        {
            m_StepScaler *= 10;
            if( m_StepScaler > m_StepScalerMax )
                m_StepScaler = m_StepScalerMax;
            ScaleCounter = 0;
        }
    }

    // Apply movement
    if( dx != 0 )
    {
        s32 OldValue = m_Value;

        m_Value += dx * m_StepScaler;
        if( m_Value <  m_Min )
        {
            m_Value = m_Min;
            g_AudioMgr.Play( "InvalidEntry" );
        }
        else if( m_Value >= m_Max )
        {
            m_Value = m_Max;
            g_AudioMgr.Play( "InvalidEntry" );
        }
        else
        {
            if( m_UseSound )
                g_AudioMgr.Play( "Slider" );
        }

        if( (m_Value != OldValue) && m_pParent )
        {
            if( m_Max > m_Min )
                m_ValueParametric = (f32)(m_Value - m_Min) / (f32)(m_Max-m_Min);

            m_pParent->OnNotify( m_pParent, this, WN_SLIDER_CHANGE, (void*)m_Value );
        }

        Processed = TRUE;
    }

    // Pass up chain if not processed
    if( !Processed )
    {
        if( m_pParent )
            m_pParent->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
    }
}

//=========================================================================

void ui_slider::OnPadSelect( ui_win* pWin )
{
    // Pass up to parent
    if( m_pParent )
        m_pParent->OnPadSelect( pWin );
}

//=========================================================================

void ui_slider::SetRange( s32 Min, s32 Max )
{
    ASSERT( Max >= Min );
    m_Min = Min;
    m_Max = Max;

    if( m_IsParametric )
    {
        m_Value = m_Min + (s32)((m_Max-m_Min) * m_ValueParametric + 0.25f);
    }
    else
    {
        if( m_Value <  m_Min ) m_Value = m_Min;
        if( m_Value >= m_Max ) m_Value = m_Max;
    }
}

//=========================================================================

void ui_slider::GetRange( s32& Min, s32& Max ) const
{
    Min = m_Min;
    Max = m_Max;
}

//=========================================================================

void ui_slider::SetStep( s32 Step, s32 StepScalerMax )
{
    m_Step          = Step;
    m_StepScaler    = 1;
    m_StepScalerMax = StepScalerMax;
}

//=========================================================================

s32 ui_slider::GetStep( void ) const
{
    return m_Step;
}

//=========================================================================

void ui_slider::SetValue( s32 Value )
{
    if( Value <  m_Min ) Value = m_Min;
    if( Value >= m_Max ) Value = m_Max;

    if( Value != m_Value )
    {
        m_Value = Value;
        if( m_Max > m_Min )
            m_ValueParametric = (f32)(Value - m_Min) / (m_Max-m_Min);
        else
            m_ValueParametric = 0.0f;
        m_pParent->OnNotify( m_pParent, this, WN_SLIDER_CHANGE, (void*)m_Value );
    }
}

//=========================================================================

s32 ui_slider::GetValue( void ) const
{
    return m_Value;
}

//=========================================================================

void ui_slider::OnLBDown( ui_win* pWin )
{
    (void)pWin;

#ifdef TARGET_PC

    xbool       Processed = FALSE;
    s32         dx = 0;
    static s32  ScaleCounter = 0;
 
    // If the cursor is in the thumb then allow the mouse to start dragging it.
    if( m_Thumb.PointInRect( m_MouseX, m_MouseY ) )
    {
        m_MouseDown = TRUE;
    }
#endif
}

//=========================================================================

void ui_slider::OnCursorMove( ui_win* pWin, s32 x, s32 y )
{
    (void) pWin;
    (void)x;
    (void)y;

#ifdef TARGET_PC

    // We are still dragging the thumb.
    if( m_MouseDown )
    {
        s32 OldValue = m_Value;

        ScreenToLocal( x, y );

        // Get the ratio to multily by.
        f32 Value = (f32)x/(f32)m_Position.GetWidth();
        Value *= (f32)m_Max;
        m_Value = (s32)Value;
        
        // Try to minimize the loss from float to int conversion.
        if( Value > (f32)m_Value )
            m_Value += 1;
        
        // Is it in bound.
        if( m_Value <  m_Min ) m_Value = m_Min;
        if( m_Value >= m_Max ) m_Value = m_Max;

        if( (m_Value != OldValue) && m_pParent )
        {
            m_pParent->OnNotify( m_pParent, this, WN_SLIDER_CHANGE, (void*)m_Value );
//            audio_Play( SFX_FRONTEND_CURSOR_MOVE_02,AUDFLAG_CHANNELSAVER );	//-- Jhowa
        }
    }
    
    // The current mouse position.
    m_MouseX = x;
    m_MouseY = y;
#endif
}

//=========================================================================

void ui_slider::OnLBUp( ui_win* pWin )
{
    (void)pWin;

#ifdef TARGET_PC
    // Stop dragging the thumb.
    if( m_MouseDown )
        m_MouseDown = FALSE;
    
    if( m_pParent )
        m_pParent->OnLBUp( pWin );
#endif

}

//=========================================================================

void ui_slider::OnCursorExit ( ui_win* pWin )
{
    (void)pWin;

#ifdef TARGET_PC

    // Stop dragging the thumb.
//    if( m_MouseDown )
//        m_MouseDown = FALSE;

#endif
    // Turn off the high light.
    ui_win::OnCursorExit( pWin );

    if( m_pParent )
    {
        m_pParent->OnCursorExit( pWin );
    }
}

//=========================================================================

void ui_slider::SetParametric( xbool State )
{
    m_IsParametric = State;
}

//=========================================================================
