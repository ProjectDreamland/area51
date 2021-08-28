//=========================================================================
//
//  dlg_profile_name.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_ProfileName.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"



//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_PROFILE_NAME_NAV_TEXT,
};


ui_manager::control_tem ProfileNameControls[] = 
{
    // Frames.
    { IDC_PROFILE_NAME_NAV_TEXT,    "IDS_NULL",    "text",   0,  0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem ProfileNameDialog =
{
    "IDS_PROFILE_NAME",
    1, 9,
    sizeof(ProfileNameControls)/sizeof(ui_manager::control_tem),
    &ProfileNameControls[0],
    0
};

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
//  Registration function
//=========================================================================

void dlg_profile_name_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "profile name", &ProfileNameDialog, &dlg_profile_name_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_profile_name_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_profile_name* pDialog = new dlg_profile_name;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_profile_name::dlg_profile_name( void )
{
}

//=========================================================================

dlg_profile_name::~dlg_profile_name( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_profile_name::Create( s32                        UserID,
                             ui_manager*                pManager,
                             ui_manager::dialog_tem*    pDialogTem,
                             const irect&               Position,
                             ui_win*                    pParent,
                             s32                        Flags,
                             void*                      pUserData )
{
    xbool   Success = FALSE;

    (void)pUserData;

    ASSERT( pManager );

    // Do dialog creation
	Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    // initialize nav text
    m_pNavText = (ui_text*)FindChildByID( IDC_PROFILE_NAME_NAV_TEXT );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );

    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    
    // get profile data
    player_profile& Profile = g_StateMgr.GetPendingProfile();

    m_ProfileName = xwstring(Profile.GetProfileName());
    m_ProfileEntered = FALSE;
    m_ProfileOK = FALSE;
    m_bRenderBlackout = FALSE;
    m_pVKeyboard = NULL;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_profile_name::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_profile_name::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;
    
    // render background filter
    if( m_bRenderBlackout )
    {
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
#ifdef TARGET_PS2
        // Nasty hack to force PS2 to draw to rb.l = 0
        rb.Set( -1, 0, XRes, YRes );
#else
        rb.Set( 0, 0, XRes, YRes );
#endif
        g_UiMgr->RenderGouraudRect(rb, xcolor(0,0,0,180),
            xcolor(0,0,0,180),
            xcolor(0,0,0,180),
            xcolor(0,0,0,180),FALSE);
    }

    // render transparent screen
    rb.l = m_CurrPos.l + 22;
    rb.t = m_CurrPos.t;
    rb.r = m_CurrPos.r - 23;
    rb.b = m_CurrPos.b;

    g_UiMgr->RenderGouraudRect(rb, xcolor(56,115,58,64),
                                   xcolor(56,115,58,64),
                                   xcolor(56,115,58,64),
                                   xcolor(56,115,58,64),FALSE);


    // render the screen bars
    s32 y = rb.t + offset;    

    while (y < rb.b)
    {
        irect bar;

        if ((y+width) > rb.b)
        {
            bar.Set(rb.l, y, rb.r, rb.b);
        }
        else
        {
            bar.Set(rb.l, y, rb.r, y+width);
        }

        // draw the bar
        g_UiMgr->RenderGouraudRect(bar, xcolor(56,115,58,30),
                                        xcolor(56,115,58,30),
                                        xcolor(56,115,58,30),
                                        xcolor(56,115,58,30),FALSE);

        y+=gap;
    }

    // increment the offset
    if (++offset > 9)
        offset = 0;

    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_profile_name::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_profile_name::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
       
}

//=========================================================================

void dlg_profile_name::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        //g_AudioMgr.Play("OptionBack");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_profile_name::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the nav text
            m_pNavText->SetFlag( ui_win::WF_VISIBLE, TRUE );

            // open a VK to enter the profile name
            m_pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", m_Position, this, ui_win::WF_VISIBLE|ui_win::WF_USE_ABSOLUTE );
            m_pVKeyboard->Configure( TRUE );
            m_pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_NAME_EDIT" ) );
            m_pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
            m_pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOK );

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
            m_pNavText->SetLabel( navText );

            // set the highlight
            g_UiMgr->SetScreenHighlight( m_pVKeyboard->GetPosition() );
        }
    }

    if( m_pVKeyboard )
        m_pVKeyboard->OnUpdate(pWin, DeltaTime);

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // check for profile name entry
    if( m_ProfileEntered )
    {
        m_pVKeyboard = NULL;

        m_ProfileEntered = FALSE;

        if( m_ProfileOK )
        {
            m_ProfileOK = FALSE;

            // store the new profile name
            player_profile& Profile = g_StateMgr.GetPendingProfile();
            Profile.SetProfileName( xstring(m_ProfileName) );
        }

        // return to previous screen
        //g_AudioMgr.Play("OptionBack");
        m_State = DIALOG_STATE_BACK;
    }


}

//=========================================================================
