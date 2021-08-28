//=========================================================================
//
//  ui_dlg_vkeyboard.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"
#include "StateMgr/StateMgr.hpp"
#include "StringMgr\StringMgr.hpp"

#include "ui_dlg_vkeyboard.hpp"
#include "ui_manager.hpp"
#include "ui_control.hpp"
#include "ui_font.hpp"
#include "ui_frame.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//=========================================================================
//  Defines
//=========================================================================

#define KEYW    14
#define KEYH    18
#define NCOLS   17
#define NROWS   5

enum notifications
{
    WN_CHARACTER    = ui_win::WN_USER,
    WN_REFRESH,
};

static u8 Keys[17*5] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '%', '$', '^', '#', '/', ':', '\'',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', '@', '(', '+', ')',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '?', '<', '=', '>', 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', '~', '[', '-', ']',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '.', '{', '|', '}',
};

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Virtual Keyboard Dialog
//=========================================================================

enum controls
{
    IDC_CANCEL,
    IDC_ACCEPT,
    IDC_FRAME
};

ui_manager::control_tem vkeyboardControls[] =
{
//  { IDC_CANCEL,   "IDS_CANCEL", "button",     9, 162+22, 100,  24, 0, 8, 9, 1, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
//  { IDC_ACCEPT,   "IDS_OK",     "button",   162, 162+22, 100,  24, 9, 8, 8, 1, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },

    { IDC_FRAME,    "IDS_NULL",   "frame",      8,  36+22, 254, 122, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem vkeyboardDialog =
{
    "IDS_VIRTUAL_KEYBOARD_TITLE",
    17, 9,
    sizeof(vkeyboardControls)/sizeof(ui_manager::control_tem),
    &vkeyboardControls[0],
    0
};

//=========================================================================
//  vkey Window
//=========================================================================

class ui_vkey : public ui_control
{
public:
    virtual void    Render              ( s32 ox=0, s32 oy=0 );
    virtual void    OnPadSelect         ( ui_win* pWin );
    virtual void    OnLBDown            ( ui_win* pWin );
    virtual void    OnCursorEnter       ( ui_win* pWin );
protected:
};

//=========================================================================

void ui_vkey::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Render placeholder rectangle
        xcolor  Color       = XCOLOR_BLACK;
        xcolor  TextColor1  = XCOLOR_WHITE;
        xcolor  TextColor2  = XCOLOR_BLACK;
        //xbool   ForceRect   = FALSE;

        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        xwstring Label;
        Label = m_Label;
        xwchar Char = Label[0];

        switch( Char )
        {
        case '\010':
            //ForceRect = TRUE;
            Label = g_StringTableMgr( "ui", "IDS_DELETE" );	
            break;
        case '\020':
            //ForceRect = TRUE;
            Label = g_StringTableMgr( "ui", "IDS_SPACE" );	
            break;
        case '\030':
            //ForceRect = TRUE;
            Label = g_StringTableMgr( "ui", "IDS_CANCEL" );	
            break;
        case '\040':
            //ForceRect = TRUE;
            Label = g_StringTableMgr( "ui", "IDS_OK" );	
            break;
        }

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            Color      = xcolor(0,0,0,0);
            TextColor1 = XCOLOR_GREY;
            TextColor2 = xcolor(0,0,0,0);
        }
        else if( m_Flags & (WF_HIGHLIGHT|WF_SELECTED) )
        {
            s32 alpha = 128 + (g_UiMgr->GetHighlightAlpha(8) * 8); // 128<->192
            Color      = xcolor(79,214,60,alpha); //xcolor( 121, 199, 213 );
            TextColor1 = xcolor(0,0,0,255);
            TextColor2 = xcolor(0,0,0,0);

            m_pManager->AddHighlight( m_UserID, r );
        }
        else
        {
            Color      = xcolor(0,0,0,0);
            TextColor1 = xcolor(255,252,204,255); //XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }

        //if( ForceRect )
        //    Color = xcolor(79,214,60,255); //xcolor( 121, 199, 213 );

        r.Inflate( 1, 1 );
        m_pManager->RenderRect( r, Color, FALSE );

        r.Translate(  2, -2 );
        m_pManager->RenderText( 1, r, ui_font::h_center|ui_font::v_center, TextColor2, Label );
        r.Translate( -1, -1 );
        m_pManager->RenderText( 1, r, ui_font::h_center|ui_font::v_center, TextColor1, Label );
    }
}

//=========================================================================

void ui_vkey::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
    m_pParent->OnNotify( m_pParent, this, WN_CHARACTER, &m_Label );
    g_AudioMgr.Play( "Select_VKB" );
}

//=========================================================================

void ui_vkey::OnLBDown( ui_win* pWin )
{
    (void)pWin;
    m_pParent->OnNotify( m_pParent, this, WN_CHARACTER, &m_Label );
    g_AudioMgr.Play( "Select_VKB" );
}

//=========================================================================

void ui_vkey::OnCursorEnter( ui_win* pWin )
{
    (void)pWin;
    m_Flags |= WF_HIGHLIGHT;
    //g_AudioMgr.Play( "Cusor_VKB" );
}

//=========================================================================

//=========================================================================
//  vkString Window
//=========================================================================

class ui_vkString : public ui_control
{
public:
    xbool           Create              ( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );
    virtual void    Render              ( s32 ox=0, s32 oy=0 );
    virtual void    OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void    OnPadShoulder       ( ui_win* pWin, s32 Direction );
    virtual void    OnPadDelete         ( ui_win* pWin );

    void            Backspace           ( void );
    void            Character           ( const xstring& String );
    s32             GetCursorPos        ( void );
    void            SetCursorPos        ( s32 Pos );

protected:
    s32             m_iElement;
    s32             m_Cursor;
};

//=========================================================================

xbool ui_vkString::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_iElement = m_pManager->FindElement( "frame2" );
    ASSERT( m_iElement != -1 );
    
    m_Cursor = 0;

    return Success;
}

//=========================================================================

static s32 s_CursorFrame = 0;

void ui_vkString::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Render placeholder rectangle
        xcolor  Color       = XCOLOR_BLACK;
        xcolor  TextColor1  = xcolor(255,252,204,255); //XCOLOR_WHITE;
        xcolor  TextColor2  = XCOLOR_BLACK;

        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            Color      = xcolor(0,0,0,0);
            TextColor1 = XCOLOR_GREY;
            TextColor2 = xcolor(0,0,0,0);
        }
        else if( m_Flags & (WF_HIGHLIGHT|WF_SELECTED) )
        {
            Color      = xcolor (39,117,28,128);//xcolor( 121, 199, 213 );
            TextColor1 = XCOLOR_BLACK;
            TextColor2 = xcolor(255,252,204,255); //XCOLOR_WHITE;
        }
        else
        {
            Color      = xcolor (39,117,28,128); //xcolor(25,79,103,255);
            TextColor1 = xcolor(255,252,204,255); //XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }

        // Render color rectangle and frame
        irect   rb = r;
        rb.Deflate( 1, 1 );
        m_pManager->RenderRect( rb, Color, FALSE );
        m_pManager->RenderElement( m_iElement, r, 0 );

        // Set a clip window to render the text
        rb.Deflate( 4, 1 );

        // render highlight if selected
        if ( m_Flags & (WF_HIGHLIGHT|WF_SELECTED) )
        {
            s32 alpha = 128 + (g_UiMgr->GetHighlightAlpha(8) * 8); // 64<->192
            m_pManager->RenderRect( rb, xcolor(79,214,60,alpha), FALSE );
        }

        if (g_UiMgr->IsWipeActive())
        {
            irect wipePos;
            g_UiMgr->GetWipePos(wipePos);

            if ( wipePos.b > rb.t )
            {
                if ( wipePos.b > rb.b )
                {
#ifdef TARGET_PS2
                    gsreg_Begin( 1 );
                    gsreg_SetScissor( rb.l, rb.t, rb.r, rb.b );
                    gsreg_End();
#endif
                }
                else
                {
#ifdef TARGET_PS2
                    gsreg_Begin( 1 );
                    gsreg_SetScissor( rb.l, rb.t, rb.r, wipePos.b );
                    gsreg_End();
#endif
                }
            }
        }
        else
        {
            m_pManager->PushClipWindow( rb );
        }

        // Render Text
        irect rt = rb;
        rt.l += 1;
        rt.r -= 3;
        rt.Translate(  3, -2 );
        m_pManager->RenderText( 1, rt, ui_font::h_left|ui_font::v_center|ui_font::clip_r_justify, TextColor2, m_Label );
        rt.Translate( -1, -1 );
        m_pManager->RenderText( 1, rt, ui_font::h_left|ui_font::v_center|ui_font::clip_r_justify, TextColor1, m_Label );

        // Render Cursor
        irect Rect;
        m_pManager->TextSize( 1, Rect, m_Label, m_Cursor );
        rb.l = rt.l + Rect.r - 1;
        rb.r = rb.l + 1;
        rb.Deflate( 0, 2 );

        if( !(s_CursorFrame & 0x10) )
            m_pManager->RenderRect( rb, TextColor1, TRUE );
        s_CursorFrame++;

        // Clear the clip window
        if (g_UiMgr->IsWipeActive())
        {
#ifdef TARGET_PS2
            // restore correct scissor
            irect wipePos;
            g_UiMgr->GetWipePos(wipePos);

            irect screen;
            g_UiMgr->GetScreenSize(screen);
            
            gsreg_Begin( 1 );
            gsreg_SetScissor( screen.l, screen.t, screen.r, wipePos.b );
            gsreg_End();
#endif
        }
        else
        {
            m_pManager->PopClipWindow();
        }
    }
}

//=========================================================================

void ui_vkString::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    // Which way are we moving
    switch( Code )
    {
    case ui_manager::NAV_LEFT:
        if( m_Cursor > 0 )
        {
            m_Cursor--;
            s_CursorFrame = 0;
            g_AudioMgr.Play( "Cusor_VKB" );
        }
        else
		{
            g_AudioMgr.Play( "InvalidEntry" );
		}
        break;
    case ui_manager::NAV_RIGHT:
        if( m_Cursor < m_Label.GetLength() )
        {
            m_Cursor++;
            s_CursorFrame = 0;
            g_AudioMgr.Play( "Cusor_VKB" );
        }
        else
		{
            g_AudioMgr.Play( "InvalidEntry" );
		}
        break;
    case ui_manager::NAV_UP:
        ui_control::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
        g_AudioMgr.Play( "InvalidEntry" );
        break;
    default:
        ui_control::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
        //g_AudioMgr.Play( "Cusor_VKB" );
        break;
    }
}

//=========================================================================

void ui_vkString::OnPadShoulder( ui_win* pWin, s32 Direction )
{
    // Which way are we moving
    switch( Direction )
    {
    case -1:
        if( m_Cursor > 0 )
        {
            m_Cursor--;
            s_CursorFrame = 0;
            g_AudioMgr.Play( "Cusor_VKB" );
        }
        else
		{
            g_AudioMgr.Play( "InvalidEntry" );
		}
        break;
    case 1:
        if( m_Cursor < m_Label.GetLength() )
        {
            m_Cursor++;
            s_CursorFrame = 0;
            g_AudioMgr.Play( "Cusor_VKB" );
        }
        else
		{
            g_AudioMgr.Play( "InvalidEntry" );
		}
        break;
    default:
        ui_control::OnPadShoulder( pWin, Direction );
    }
}

//=========================================================================

void ui_vkString::OnPadDelete( ui_win* pWin )
{
    (void)pWin;
    if( m_Cursor > 0 )
    {
        Backspace();
        m_pParent->OnNotify( m_pParent, this, WN_REFRESH, NULL );
        g_AudioMgr.Play( "Delete_VKB" );
    }
    else
    {
        g_AudioMgr.Play( "InvalidEntry" );
    }
}

//=========================================================================

void ui_vkString::Backspace( void )
{
    ASSERT( m_Cursor > 0 );

    xstring NewString;
    NewString = m_Label.Left( m_Cursor-1 );
    NewString += m_Label.Right( m_Label.GetLength()-m_Cursor );
    m_Label = NewString;
    m_Cursor--;
    s_CursorFrame = 0;
}

void ui_vkString::Character( const xstring& String )
{
    xstring NewString;
    NewString = m_Label.Left( m_Cursor );
    NewString += String;
    NewString += m_Label.Right( m_Label.GetLength()-m_Cursor );
    m_Label = NewString;
    m_Cursor++;
    s_CursorFrame = 0;
}

s32 ui_vkString::GetCursorPos( void )
{
    return m_Cursor;
}

void ui_vkString::SetCursorPos( s32 Pos )
{
    if( Pos < 0 )
        Pos = 0;
    if( Pos > m_Label.GetLength() )
        Pos = m_Label.GetLength();
    m_Cursor = Pos;
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_dlg_vkeyboard_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    ui_dlg_vkeyboard* pDialog = new ui_dlg_vkeyboard;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  ui_dlg_vkeyboard
//=========================================================================

ui_dlg_vkeyboard::ui_dlg_vkeyboard( void )
{
    m_pResultDone = NULL;
    m_pResultOk   = NULL;
    m_bName       = TRUE;
    m_pPopUp      = NULL;
#ifdef TARGET_XBOX
    m_bVerifyXBL  = TRUE;
#endif
}

//=========================================================================

ui_dlg_vkeyboard::~ui_dlg_vkeyboard( void )
{
    Destroy();
}

//=========================================================================

xbool ui_dlg_vkeyboard::Create( s32                        UserID,
                                ui_manager*                pManager,
                                ui_manager::dialog_tem*    pDialogTem,
                                const irect&               Position,
                                ui_win*                    pParent,
                                s32                        Flags,
                                void*                      pUserData )
{
    xbool   Success = FALSE;
    s32     x,y;


    (void)pDialogTem;
    (void)pUserData;

    ASSERT( pManager );

    // Set Size of window
    irect r( 0, 0, 270, 166+22 );

    // scale x pos and x size
    r.l = (s32)( (f32)r.l * g_UiMgr->GetScaleX() );
    r.r = (s32)( (f32)r.r * g_UiMgr->GetScaleX() );
    r.t = (s32)( (f32)r.t * g_UiMgr->GetScaleY() );
    r.b = (s32)( (f32)r.b * g_UiMgr->GetScaleY() );

    // center it
    r.Translate( Position.GetWidth()/2-r.GetWidth()/2, Position.GetHeight()/2-r.GetHeight()/2 );

    // Do dialog creation
    Success = ui_dialog::Create( UserID, pManager, &vkeyboardDialog, r, pParent, Flags );

    // Setup Frame
    ((ui_frame*)m_Children[0])->SetBackgroundColor( xcolor (39,117,28,128) ); //xcolor(25,79,103,255) );
    ((ui_frame*)m_Children[0])->ChangeElement("frame2");

    // Add custom controls to Dialog
    {
        s32 i = 0;
        for( y=0 ; y<NROWS ; y++ )
        {
            for( x=0 ; x<NCOLS ; x++ )
            {
                // Create each control
                irect Pos;
                Pos.Set( 16+x*KEYW, 22+42+y*KEYH, 16+x*KEYW+KEYW, 22+42+y*KEYH+KEYH );

                // scale x pos and x size
                Pos.l = (s32)( (f32)Pos.l * g_UiMgr->GetScaleX() );
                Pos.r = (s32)( (f32)Pos.r * g_UiMgr->GetScaleX() );
                Pos.t = (s32)( (f32)Pos.t * g_UiMgr->GetScaleY() );
                Pos.b = (s32)( (f32)Pos.b * g_UiMgr->GetScaleY() );

                ui_vkey* pVKey = new ui_vkey;
                ASSERT( pVKey );
                pVKey->Create( m_UserID, m_pManager, Pos, this, ui_win::WF_VISIBLE );

                // Configure the control
                pVKey->SetLabel( xwstring(xfs("%c",Keys[i])) );
                pVKey->SetNavPos( irect( x, y+1, x+1, y+2 ) );
                m_NavGraph[x+(y+1)*NCOLS] = pVKey;
                i++;
            }
        }
    }

    // These are the standard button widths that were in the code previously.
    // I guess these were based from english text.
    s32 ui_button_width[4] = { 58, 58, 58, 58 };

    // Override the button widths for foreign languages.
    // Make sure the total width remains at 232 (58*4)
    switch( x_GetLocale() )
    {
    case XL_LANG_FRENCH:
        ui_button_width[0] = 72;
        ui_button_width[1] = 56;
        ui_button_width[3] = 46;
        break;
    case XL_LANG_SPANISH:
        ui_button_width[2] = 66;
        ui_button_width[3] = 50;
        break;
    case XL_LANG_GERMAN:
        ui_button_width[0] = 58;
        ui_button_width[1] = 70;
        ui_button_width[2] = 74;
        ui_button_width[3] = 30;
        break;
    }
        
    // Add backspace
    {
        s32 x=0;
        s32 y=5;
        irect Pos;
        //Pos.Set( 16+x*KEYW, 22+4+42+y*KEYH, 16+(x+6)*KEYW+KEYW, 22+4+42+y*KEYH+KEYH );
        Pos.Set( 16, 22+4+42+y*KEYH, 16+ui_button_width[0], 22+4+42+y*KEYH+KEYH );

        // scale x pos and x size
        Pos.l = (s32)( (f32)Pos.l * g_UiMgr->GetScaleX() );
        Pos.r = (s32)( (f32)Pos.r * g_UiMgr->GetScaleX() );
        Pos.t = (s32)( (f32)Pos.t * g_UiMgr->GetScaleY() );
        Pos.b = (s32)( (f32)Pos.b * g_UiMgr->GetScaleY() );

        ui_vkey* pVKey = new ui_vkey;
        ASSERT( pVKey );
        pVKey->Create( m_UserID, m_pManager, Pos, this, ui_win::WF_VISIBLE );
        pVKey->SetLabel( "\010" );
        pVKey->SetNavPos( irect( x, y+1, x+4, y+2 ) );
        for( x=0 ; x<4 ; x++ )
            m_NavGraph[x+(y+1)*NCOLS] = pVKey;
    }

    // Add space
    {
        s32 x=4;
        s32 y=5;
        irect Pos;
        //Pos.Set( 16+x*KEYW, 22+4+42+y*KEYH, 16+(x+6)*KEYW+KEYW, 22+4+42+y*KEYH+KEYH );
        Pos.Set( 16+ui_button_width[0]+2, 22+4+42+y*KEYH, 16+ui_button_width[0]+2+ui_button_width[1], 22+4+42+y*KEYH+KEYH );

        // scale x pos and x size
        Pos.l = (s32)( (f32)Pos.l * g_UiMgr->GetScaleX() );
        Pos.r = (s32)( (f32)Pos.r * g_UiMgr->GetScaleX() );
        Pos.t = (s32)( (f32)Pos.t * g_UiMgr->GetScaleY() );
        Pos.b = (s32)( (f32)Pos.b * g_UiMgr->GetScaleY() );

        ui_vkey* pVKey = new ui_vkey;
        ASSERT( pVKey );
        pVKey->Create( m_UserID, m_pManager, Pos, this, ui_win::WF_VISIBLE );
        pVKey->SetLabel( "\020" );
        pVKey->SetNavPos( irect( x, y+1, x+4, y+2 ) );
        for( x=4 ; x<8 ; x++ )
            m_NavGraph[x+(y+1)*NCOLS] = pVKey;
    }

    // add cancel
    {
        s32 x=8;
        s32 y=5;
        irect Pos;
        Pos.Set( 16+ui_button_width[0]+2+ui_button_width[1]+2, 22+4+42+y*KEYH, 16+ui_button_width[0]+2+ui_button_width[1]+2+ui_button_width[2], 22+4+42+y*KEYH+KEYH );

        // scale x pos and x size
        Pos.l = (s32)( (f32)Pos.l * g_UiMgr->GetScaleX() );
        Pos.r = (s32)( (f32)Pos.r * g_UiMgr->GetScaleX() );
        Pos.t = (s32)( (f32)Pos.t * g_UiMgr->GetScaleY() );
        Pos.b = (s32)( (f32)Pos.b * g_UiMgr->GetScaleY() );

        ui_vkey* pVKey = new ui_vkey;
        ASSERT( pVKey );
        pVKey->Create( m_UserID, m_pManager, Pos, this, ui_win::WF_VISIBLE );
        pVKey->SetLabel( "\030" );
        pVKey->SetNavPos( irect( x, y+1, x+4, y+2 ) );
        for( x=8 ; x<12 ; x++ )
            m_NavGraph[x+(y+1)*NCOLS] = pVKey;
    }

    // add OK
    {
        s32 x=12;
        s32 y=5;
        irect Pos;
        Pos.Set( 16+ui_button_width[0]+2+ui_button_width[1]+2+ui_button_width[2]+2, 22+4+42+y*KEYH, 16+ui_button_width[0]+2+ui_button_width[1]+2+ui_button_width[2]+2+ui_button_width[3], 22+4+42+y*KEYH+KEYH );

        // scale x pos and x size
        Pos.l = (s32)( (f32)Pos.l * g_UiMgr->GetScaleX() );
        Pos.r = (s32)( (f32)Pos.r * g_UiMgr->GetScaleX() );
        Pos.t = (s32)( (f32)Pos.t * g_UiMgr->GetScaleY() );
        Pos.b = (s32)( (f32)Pos.b * g_UiMgr->GetScaleY() );

        ui_vkey* pVKey = new ui_vkey;
        ASSERT( pVKey );
        pVKey->Create( m_UserID, m_pManager, Pos, this, ui_win::WF_VISIBLE );
        pVKey->SetLabel( "\040" );
        pVKey->SetNavPos( irect( x, y+1, x+5, y+2 ) );
        for( x=12 ; x<17 ; x++ )
            m_NavGraph[x+(y+1)*NCOLS] = pVKey;

        GotoControl((ui_control*)pVKey);
    }

    // Add String control to dialog
    m_pStringCtrl = new ui_vkString;
    ASSERT( m_pStringCtrl );
    irect Pos( 8, 22+8, 8+254, 22+8+24 );

    // scale x pos and x size
    Pos.l = (s32)( (f32)Pos.l * g_UiMgr->GetScaleX() );
    Pos.r = (s32)( (f32)Pos.r * g_UiMgr->GetScaleX() );
    Pos.t = (s32)( (f32)Pos.t * g_UiMgr->GetScaleY() );
    Pos.b = (s32)( (f32)Pos.b * g_UiMgr->GetScaleY() );

    m_pStringCtrl->Create( m_UserID, m_pManager, Pos, this, ui_win::WF_VISIBLE );
    for( x=0 ; x<m_NavW ; x++ )
        m_NavGraph[x] = m_pStringCtrl;

    // Initialize Data
    m_iElement = m_pManager->FindElement( "frame2" );
    ASSERT( m_iElement != -1 );
    m_BackgroundColor   = xcolor (19,59,14,255); //xcolor(0, 20, 30,255);//FECOL_DIALOG2; //-- Jhowa
    m_pString       = NULL;
    m_MaxCharacters = -1;

    // play create sound
    g_AudioMgr.Play( "Select_VKB" );

    // Return success code
    return Success;
}

//=========================================================================

void ui_dlg_vkeyboard::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  Color       = XCOLOR_WHITE;

        // Set color if highlighted or selected or disabled
        if( m_Flags & WF_DISABLED )
            Color = XCOLOR_GREY;
        if( m_Flags & (WF_HIGHLIGHT|WF_SELECTED) )
            Color = XCOLOR_RED;

        // Get window rectangle
        irect   r;
        r.Set( m_Position.l+ox, m_Position.t+oy, m_Position.r+ox, m_Position.b+oy );

        // Render background color
        if( m_BackgroundColor.A > 0 )
        {
            irect   rb = r;
            rb.Deflate( 1, 1 );
            m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );
/*
            irect   r1 = rb;
            irect   r2 = rb;
            irect   r3 = rb;
            s32     s = MIN( 128, r1.GetHeight()/2 );
            r1.b = r1.t + s;
            r2.t = r1.b;
            r3.t = r3.b - s;
            r2.b = r3.t;
            xcolor  c1 = m_BackgroundColor;
            xcolor  c2 = m_BackgroundColor;
            c1.A = 224;
            c2.A = 128;
            m_pManager->RenderGouraudRect( r1, c1,c2,c2,c1, FALSE );
            m_pManager->RenderGouraudRect( r2, c2,c2,c2,c2, FALSE );
            m_pManager->RenderGouraudRect( r3, c2,c1,c1,c2, FALSE );
*/
        }

        // Render Title
        irect rect = r;
        rect.Deflate( 1, 1 );
        rect.SetHeight( 22 );
        xcolor c1 = xcolor (25,77,18,255); //xcolor(20,30,40,224);//FECOL_TITLE1; //-- Jhowa
        xcolor c2 = xcolor (25,77,18,255); //xcolor(20,30,40,224);//FECOL_TITLE2; //-- Jhowa
        m_pManager->RenderGouraudRect( rect, c1, c1, c2, c2, FALSE );

        rect.Deflate( 8, 0 );
        rect.Translate( 1, -1 );
        m_pManager->RenderText( 0, rect, ui_font::h_left|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Label );
        rect.Translate( -1, -1 );
        m_pManager->RenderText( 0, rect, ui_font::h_left|ui_font::v_center, xcolor(255,252,204,255), m_Label );

        // Render frame
        m_pManager->RenderElement( m_iElement, r, 0 );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_dlg_vkeyboard::OnUpdate( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    if( m_pPopUp )
    {
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            m_pPopUp = NULL;
        }
    }
}

//=========================================================================

void ui_dlg_vkeyboard::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    (void)WrapX;
    (void)WrapY;
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, TRUE, FALSE );
    g_AudioMgr.Play( "Cusor_VKB" );
}

//=========================================================================

void ui_dlg_vkeyboard::OnPadShoulder( ui_win* pWin, s32 Direction )
{
    m_pStringCtrl->OnPadShoulder( pWin, Direction );
}

//=========================================================================

void ui_dlg_vkeyboard::OnPadDelete( ui_win* pWin )
{
    m_pStringCtrl->OnPadDelete( pWin );
}

//=========================================================================

void ui_dlg_vkeyboard::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void ui_dlg_vkeyboard::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_pPopUp )
        return;

    // Undo changes
    if( m_pString )
        *m_pString = m_BackupString;

    if( m_pResultDone )
        *m_pResultDone = TRUE;

    // Close dialog
    if ( m_Flags & WF_INPUTMODAL )
    {
        m_pManager->EndDialog( m_UserID, TRUE );
    }

    g_AudioMgr.Play( "Backup" );
}

//=========================================================================

s32 ui_dlg_vkeyboard::IsValid( const xwstring* pString, xbool bIsName )
{
    if( bIsName )
    {
        s32 Length = pString->GetLength();                                

        //
        // Length Filter.
        //
        if( Length < 3 )
        {
            return 2;
        }

#ifdef TARGET_XBOX
        if( m_bVerifyXBL )
        {
            // MUST be online to do this!
            ASSERT( g_NetworkMgr.IsOnline() );

            // run xbox obscenity check
            XONLINETASK_HANDLE Task;
            XOnlineStringVerify(
                1,                              // number of strings
                (LPCWSTR*)pString,              // array of strings
                XGetLanguage(),                 // language to check
                NULL,
                &Task
                );

            HRESULT Result;
            // wait for check to complete
            while( ( Result = XOnlineTaskContinue( Task ) ) == XONLINETASK_S_RUNNING )
            {
                x_DelayThread( 1 );
            }

            if( Result != XONLINETASK_S_SUCCESS )
            {
                // check failed
                return 1;
            }
        }
#endif

        // Convert it to lower case.
        xwstring PotentialName = *pString;
        for( s32 i = 0; i < Length; i++ )
        {
            if( IN_RANGE( (xwchar)'A', PotentialName.GetAt( i ), (xwchar)'Z' ) )
            {
                PotentialName.SetAt( i, PotentialName.GetAt( i ) - ('A' - 'a') );
            }
        }

        //
        // Obscenity Filter.
        //
        g_StringTableMgr.LoadTable( "Obscenities", xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "ENG_obscenities.stringbin" ) );
        xwstring Words[ 100 ];

        s32 NumWords = -1;
        do {
            NumWords++;
            Words[ NumWords ] = g_StringTableMgr( "Obscenities", xfs( "Word_%02d", NumWords ) );
        } while( x_wstrcmp( (const xwchar*)Words[ NumWords ], 
                            (const xwchar*)xwstring( "<null>" ) ) != 0 );

        g_StringTableMgr.UnloadTable( "Obscenities" );
        
        s32 NameCharacters[ NET_SERVER_NAME_LENGTH ];
        x_memset( NameCharacters, 0, sizeof(NameCharacters) );

        for( s32 i = 0; i < PotentialName.GetLength(); i++ )
        {
            if( i>= NET_SERVER_NAME_LENGTH-1 )
            {
                break;
            }
            if( IN_RANGE( (xwchar)'a', PotentialName.GetAt( i ), (xwchar)'z' ) )
            {
                NameCharacters[ i ] = 2;
            }    
            else if( IN_RANGE( (xwchar)'0', PotentialName.GetAt( i ), (xwchar)'9' ) )
            {
                NameCharacters[ i ] = 1;
            }
        }
        
        const xwchar* pNameStr = (const xwchar*)PotentialName;

        // This probably could be more efficient, but it already runs instantly, 
        // and it's only ever run when the user tries to accept his profile name.
        for( s32 i = 0; i < NumWords; i++ )
        {
            // Look for this particular obscene word.
            xwchar* pObscenePos;
            const xwchar* pLastPos = pNameStr;

            while( (pObscenePos = x_wstrstr( pLastPos, (const xwchar*)Words[ i ]) ) )
            {
                // If the word is sufficiently small it has a chance to redeem itself.
                if( Words[ i ].GetLength() < 4 )
                {
                    s32 Position = pObscenePos - pNameStr;
                    for( s32 j = Position; j < Position + Words[ i ].GetLength(); j++ )
                    {
                        // Mark the characters bad.
                        NameCharacters[ j ] = -1;
                    }

                    pLastPos = pObscenePos + 1;
                }

                // Just go ahead and throw anything with a four or more letter curse word out.
                else
                {
                    return 1;
                }
            }
        }

        xbool bHasAlphaNumeric    = FALSE;
        xbool bCurseRedeemer      = FALSE;
        xbool bInCurse            = FALSE;

        // Here is the code that makes the world safe
        // for assassins everywhere.
        for( s32 i = 0; i < NET_SERVER_NAME_LENGTH; i++ )
        {
            if( (NameCharacters[ i ] == 1) ||
                (NameCharacters[ i ] == 2) )
            {
                bHasAlphaNumeric = TRUE;
            }

            if( (NameCharacters[ i ] == 2) )
            {
                bCurseRedeemer = TRUE;
            }
            else if( NameCharacters[ i ] != -1 )
            {
                bCurseRedeemer = FALSE;
            }

            if( (NameCharacters[ i ] == -1) &&
                !bCurseRedeemer )
            {
                bInCurse = TRUE;
            }
            else if( bInCurse )
            {
                if( !bCurseRedeemer )
                {
                    // Looks like we've found an isolated curse word! Hooray!
                    return FALSE;
                }
                else
                {
                    bCurseRedeemer = FALSE;
                    bInCurse       = FALSE;
                }
            }
        }

        if( !bHasAlphaNumeric )
        {
            return 2;
        }
    }
    
    return 0;
}


//=========================================================================

void ui_dlg_vkeyboard::OnNotify( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;

    switch( Command )
    {
    case WN_CHARACTER:
        {
            ASSERT( pData );

            // Update String
            xstring*    pString = (xstring*)pData;
            if( pString->GetAt(0) == '\010' )
            {
                if( m_pStringCtrl->GetCursorPos() > 0 )
                    m_pStringCtrl->Backspace();
            }
            else if ( pString->GetAt(0) == '\020' )
            {
                // space
                if( (m_MaxCharacters == -1) || (m_pStringCtrl->GetLabel().GetLength() < m_MaxCharacters) )
                {
                    //                  m_pStringCtrl->SetLabel( m_pStringCtrl->GetLabel() + *pString );
                    m_pStringCtrl->Character( ' ' );
                }                             
                else
                {
                    g_AudioMgr.Play( "InvalidEntry" );
                }
                break;
            }
            else if ( pString->GetAt(0) == '\030' )
            {
                // cancel
                if( m_pResultOk )
                    *m_pResultOk = FALSE;
                OnPadBack( pWin );

                break;
            }
            else if ( pString->GetAt(0) == '\040' )
            {
                // accept
                // check for NULL string
                s32 ValidCode = IsValid( m_pString, m_bName );
                if( ValidCode != 0 )
                {
                    g_AudioMgr.Play( "InvalidEntry" );

                    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                    m_pPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));

                    const xwchar* pNavtext = (const xwchar*)navText;                 

                    if( ValidCode == 1 )
                    {
                        m_pPopUp->Configure( g_StringTableMgr( "ui", "IDS_OBSCENITY_FILTER" ), 
                            TRUE, 
                            TRUE, 
                            FALSE,
    #ifdef TARGET_XBOX 
                            g_StringTableMgr( "ui", "IDS_OBSCENE_PROFILE_XBOX" ),
    #else
                            g_StringTableMgr( "ui", "IDS_OBSCENE_PROFILE_PS2" ),
    #endif
                            pNavtext,
                            &m_PopUpResult );
                    }
                    else 
                    {
                        m_pPopUp->Configure( g_StringTableMgr( "ui", "IDS_INVALID_NAME" ), 
                            TRUE, 
                            TRUE, 
                            FALSE,
                            g_StringTableMgr( "ui", "IDS_INVALID_NAME_TEXT" ),
                            pNavtext,
                            &m_PopUpResult );
                    }
                }
                else
                {
                    if( m_pResultOk )
                        *m_pResultOk   = TRUE;
                    if( m_pResultDone )
                        *m_pResultDone = TRUE;

                    // Close dialog
                    if ( m_Flags & WF_INPUTMODAL )
                    {
                        m_pManager->EndDialog( m_UserID, TRUE );
                    }

                    g_AudioMgr.Play( "Select_Norm" );

                    break;
                }
            }
            else
            {
                if( (m_MaxCharacters == -1) || (m_pStringCtrl->GetLabel().GetLength() < m_MaxCharacters) )
                {
//                  m_pStringCtrl->SetLabel( m_pStringCtrl->GetLabel() + *pString );
                    m_pStringCtrl->Character( *pString );
                }
                else
                {
                    g_AudioMgr.Play( "InvalidEntry" );
                }
            }

            // Update connected string
            if( m_pString )
                *m_pString = m_pStringCtrl->GetLabel();
        }
        break;
    case WN_REFRESH:
        {
            // Update connected string
            if( m_pString )
                *m_pString = m_pStringCtrl->GetLabel();
        }
    }
}

//=========================================================================

void ui_dlg_vkeyboard::ConnectString( xwstring* pString, s32 BufferSize )
{
    ASSERT( pString );

    // Initialize all strings and pointers
    m_pString = pString;
    m_BackupString = *pString;
    m_pStringCtrl->SetLabel( *pString );
    m_pStringCtrl->SetCursorPos( pString->GetLength() );
    m_MaxCharacters = BufferSize - 1;
}

//=========================================================================

void ui_dlg_vkeyboard::SetReturn( xbool* pDone, xbool* pOk )
{
    m_pResultDone = pDone;
    m_pResultOk   = pOk;
}

//=========================================================================
#ifdef TARGET_XBOX
void ui_dlg_vkeyboard::ConfigureForProfile( void )
{
    // Disable the ? / + < = > | : controls
    s32 i = 0;
    s32 x;
    s32 y;
    for( y = 0 ; y < NROWS ; y++ )
    {
        for( x = 0 ; x < NCOLS ; x++ )
        {
            ui_vkey* pVKey = (ui_vkey*)m_NavGraph[x + (y + 1) * NCOLS];

            if ( pVKey )
            {
                if (   (pVKey->GetLabel() == xwstring( "?" ))
                    || (pVKey->GetLabel() == xwstring( "/" ))
                    || (pVKey->GetLabel() == xwstring( "+" ))
                    || (pVKey->GetLabel() == xwstring( "<" ))
                    || (pVKey->GetLabel() == xwstring( "=" ))
                    || (pVKey->GetLabel() == xwstring( ">" ))
                    || (pVKey->GetLabel() == xwstring( "|" ))
                    || (pVKey->GetLabel() == xwstring( ":" ))
                    )
                {
                    pVKey->SetFlag( WF_DISABLED, TRUE  );
                }
            }
        }
    }

    // we don't need to verify profile names
    m_bVerifyXBL = FALSE;
}
#endif
//=========================================================================


