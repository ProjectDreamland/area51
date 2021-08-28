//=========================================================================
//
//  dlg_online_eula.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_textbox.hpp"

#include "dlg_OnlineEULA.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"


//=========================================================================
//  Online EULA Dialog
//=========================================================================

enum controls
{
	IDC_EULA_TEXTBOX,
    IDC_EULA_NAV_TEXT,
};


ui_manager::control_tem OnlineEulaControls[] = 
{
    // Frames.
    { IDC_EULA_TEXTBOX,     "IDS_NULL", "textbox",  45, 60, 350, 248, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_EULA_NAV_TEXT,    "IDS_NULL", "text",      0,  0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem OnlineEulaDialog =
{
    "IDS_EULA_TITLE",
    1, 9,
    sizeof(OnlineEulaControls)/sizeof(ui_manager::control_tem),
    &OnlineEulaControls[0],
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

void dlg_online_eula_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "dnas eula", &OnlineEulaDialog, &dlg_online_eula_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_eula_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_eula* pDialog = new dlg_online_eula;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_eula
//=========================================================================

dlg_online_eula::dlg_online_eula( void )
{
}

//=========================================================================

dlg_online_eula::~dlg_online_eula( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_online_eula::Create( s32                        UserID,
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

    m_pEULATextBox  = (ui_textbox*) FindChildByID( IDC_EULA_TEXTBOX  );
    m_pNavText      = (ui_text*)    FindChildByID( IDC_EULA_NAV_TEXT );
    
    GotoControl( (ui_control*)m_pEULATextBox );
    m_CurrentControl = IDC_EULA_TEXTBOX;

    // set up textbox
    m_pEULATextBox->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pEULATextBox->SetFlag( ui_win::WF_SELECTED, TRUE );
    m_pEULATextBox->SetExitOnSelect( TRUE );
    m_pEULATextBox->SetExitOnBack( TRUE );
    m_pEULATextBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pEULATextBox->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pEULATextBox->SetLabel( g_StringTableMgr( "EULA", "IDS_EULA_AGREEMENT" ) );
    m_CurrHL = 0;

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_AGREE" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_DISAGREE" ); 
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // initialize timeout
    m_Timeout = 2.0f;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_online_eula::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_eula::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;
        
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

void dlg_online_eula::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // no input allowed until the EULA is fully visible
    if( m_Timeout > 0 )
        return;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // agreement accepted
        g_AudioMgr.Play( "Select_Norm" );
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_online_eula::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // no input allowed until the EULA is fully visible
    if( m_Timeout > 0 )
        return;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // agreement not accepted
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_online_eula::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // complete!  turn on the elements
            m_pEULATextBox  ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            GotoControl( (ui_control*)m_pEULATextBox );
            g_UiMgr->SetScreenHighlight( m_pEULATextBox->GetPosition() );
        }
    }

    // update timeout
    if( m_Timeout > 0 )
    {
        if( m_Timeout > DeltaTime )
        {
            m_Timeout -= DeltaTime;
        }
        else
        {
            // show nav text
            m_Timeout = 0;
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================