//=========================================================================
//
//  dlg_avatar_select.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"

#include "dlg_AvatarSelect.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
	IDC_AVATAR_SELECT_COMBO,
    IDC_AVATAR_SELECT_NAV_TEXT,
};


ui_manager::control_tem AvatarSelectControls[] = 
{
    // Frames.
    { IDC_AVATAR_SELECT_COMBO,     "IDS_NULL",  "combo",     71,  50, 128, 256, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AVATAR_SELECT_NAV_TEXT,  "IDS_NULL",  "text",       0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem AvatarSelectDialog =
{
    "IDS_AVATAR_MENU",
    1, 9,
    sizeof(AvatarSelectControls)/sizeof(ui_manager::control_tem),
    &AvatarSelectControls[0],
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

void dlg_avatar_select_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "avatar select", &AvatarSelectDialog, &dlg_avatar_select_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_avatar_select_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_avatar_select* pDialog = new dlg_avatar_select;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_avatar_select::dlg_avatar_select( void )
{
}

//=========================================================================

dlg_avatar_select::~dlg_avatar_select( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_avatar_select::Create( s32                        UserID,
                             ui_manager*                pManager,
                             ui_manager::dialog_tem*    pDialogTem,
                             const irect&               Position,
                             ui_win*                    pParent,
                             s32                        Flags,
                             void*                      pUserData )
{
          xbool   Success  = FALSE;
    const xwchar* pNullStr = g_StringTableMgr( "ui", "IDS_NULL" );

    (void)pUserData;

    ASSERT( pManager );

    // Do dialog creation
	Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    // find controls
    m_pAvatarSelect = (ui_combo*)   FindChildByID( IDC_AVATAR_SELECT_COMBO    );
    m_pNavText      = (ui_text*)    FindChildByID( IDC_AVATAR_SELECT_NAV_TEXT );

    // hide them
    m_pAvatarSelect->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_ACCEPT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_CYCLE_AVATAR" );
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // check profile to see if alien avatars are unlocked
    player_profile& Profile = g_StateMgr.GetPendingProfile();

    // load avatars
    s32 ID0  = g_UiMgr->LoadBitmap( "avatar0",  "UI_Avatar_Haz_1.xbmp"  );
    s32 ID1  = g_UiMgr->LoadBitmap( "avatar1",  "UI_Avatar_Haz_2.xbmp"  );
    s32 ID2  = g_UiMgr->LoadBitmap( "avatar2",  "UI_Avatar_Haz_3.xbmp"  );
    s32 ID3  = g_UiMgr->LoadBitmap( "avatar3",  "UI_Avatar_Haz_4.xbmp"  );
    s32 ID4  = g_UiMgr->LoadBitmap( "avatar4",  "UI_Avatar_Spec_1.xbmp" );
    s32 ID5  = g_UiMgr->LoadBitmap( "avatar5",  "UI_Avatar_Spec_2.xbmp" );
    s32 ID6  = g_UiMgr->LoadBitmap( "avatar6",  "UI_Avatar_Spec_3.xbmp" );
    s32 ID7  = g_UiMgr->LoadBitmap( "avatar7",  "UI_Avatar_Spec_4.xbmp" );
    s32 ID8  = g_UiMgr->LoadBitmap( "avatar8",  "UI_Avatar_Tech_1.xbmp" );
    s32 ID9  = g_UiMgr->LoadBitmap( "avatar9",  "UI_Avatar_Tech_2.xbmp" );
    s32 ID10 = g_UiMgr->LoadBitmap( "avatar10", "UI_Avatar_Tech_3.xbmp" );
    s32 ID11 = g_UiMgr->LoadBitmap( "avatar11", "UI_Avatar_Tech_4.xbmp" );
    s32 ID12 = g_UiMgr->LoadBitmap( "avatar12", "UI_Avatar_Sci_1.xbmp"  );
    s32 ID13 = g_UiMgr->LoadBitmap( "avatar13", "UI_Avatar_Sci_2.xbmp"  );
    s32 ID14 = g_UiMgr->LoadBitmap( "avatar14", "UI_Avatar_Sci_3.xbmp"  );
    s32 ID15 = g_UiMgr->LoadBitmap( "avatar15", "UI_Avatar_Sci_4.xbmp"  ); 

    // set up selections
    m_pAvatarSelect->DeleteAllItems();
    m_pAvatarSelect->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAvatarSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV);

    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_HAZMAT_0  );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_HAZMAT_1  );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_HAZMAT_2  );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_HAZMAT_3  );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_SPECFOR_0 );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_SPECFOR_1 );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_SPECFOR_2 );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_SPECFOR_3 );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_TECH_0    );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_TECH_1    );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_TECH_2    );
    m_pAvatarSelect->AddItem  ( pNullStr, SKIN_TECH_3    );

    if( Profile.m_bAlienAvatarsOn )
    {
        m_pAvatarSelect->AddItem  ( pNullStr, SKIN_GREY_0    );
        m_pAvatarSelect->AddItem  ( pNullStr, SKIN_GREY_1    );
        m_pAvatarSelect->AddItem  ( pNullStr, SKIN_GREY_2    );
        m_pAvatarSelect->AddItem  ( pNullStr, SKIN_GREY_3    );
    }

    m_pAvatarSelect->SetItemBitmap(  0, ID0  ); 
    m_pAvatarSelect->SetItemBitmap(  1, ID1  );
    m_pAvatarSelect->SetItemBitmap(  2, ID2  );
    m_pAvatarSelect->SetItemBitmap(  3, ID3  );
    m_pAvatarSelect->SetItemBitmap(  4, ID4  );
    m_pAvatarSelect->SetItemBitmap(  5, ID5  );
    m_pAvatarSelect->SetItemBitmap(  6, ID6  );
    m_pAvatarSelect->SetItemBitmap(  7, ID7  );
    m_pAvatarSelect->SetItemBitmap(  8, ID8  );
    m_pAvatarSelect->SetItemBitmap(  9, ID9  );
    m_pAvatarSelect->SetItemBitmap( 10, ID10 );
    m_pAvatarSelect->SetItemBitmap( 11, ID11 );

    if( Profile.m_bAlienAvatarsOn )
    {
        m_pAvatarSelect->SetItemBitmap( 12, ID12 );
        m_pAvatarSelect->SetItemBitmap( 13, ID13 );
        m_pAvatarSelect->SetItemBitmap( 14, ID14 );
        m_pAvatarSelect->SetItemBitmap( 15, ID15 );
    }

    // get the pending profile
    player_profile& CurrentProfile = g_StateMgr.GetPendingProfile();
    s32 AvatarID = CurrentProfile.GetAvatarID();
    ASSERTS( AvatarID < SM_MAX_AVATARS, "AvatarID out of range!" );
    // find the avatar in the list
    for( s32 i=0; i<SM_MAX_AVATARS; i++ )
    {
        if( m_pAvatarSelect->GetItemData(i) == AvatarID )
        {
            m_pAvatarSelect->SetSelection( i ); 
            break;
        }
    }

    // set initial focus
    m_CurrHL = 0;
    GotoControl( (ui_control*)m_pAvatarSelect );
    m_CurrentControl = IDC_AVATAR_SELECT_COMBO;
    m_bRenderBlackout = FALSE;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_avatar_select::Destroy( void )
{
    ui_dialog::Destroy();

#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmap
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif

    // unload avatars
    g_UiMgr->UnloadBitmap( "avatar0"  );
    g_UiMgr->UnloadBitmap( "avatar1"  );
    g_UiMgr->UnloadBitmap( "avatar2"  );
    g_UiMgr->UnloadBitmap( "avatar3"  );
    g_UiMgr->UnloadBitmap( "avatar4"  );
    g_UiMgr->UnloadBitmap( "avatar5"  );
    g_UiMgr->UnloadBitmap( "avatar6"  );
    g_UiMgr->UnloadBitmap( "avatar7"  );
    g_UiMgr->UnloadBitmap( "avatar8"  );
    g_UiMgr->UnloadBitmap( "avatar9"  );
    g_UiMgr->UnloadBitmap( "avatar10" );
    g_UiMgr->UnloadBitmap( "avatar11" );
    g_UiMgr->UnloadBitmap( "avatar12" );
    g_UiMgr->UnloadBitmap( "avatar13" );
    g_UiMgr->UnloadBitmap( "avatar14" );
    g_UiMgr->UnloadBitmap( "avatar15" );

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_avatar_select::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;

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

    // render the avatar
    // do this here, or do it in the UI somewhere.... TBD

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}
//=========================================================================

void dlg_avatar_select::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // check for changing sort key
        switch( Code )
        {
            case ui_manager::NAV_LEFT:
                // get the previous avatar
                break;
            case ui_manager::NAV_RIGHT:
                // get the next avatar
                break;
        }
    }
}

//=========================================================================

void dlg_avatar_select::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // cancel
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_avatar_select::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // accept
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // get the pending profile
        player_profile& CurrentProfile = g_StateMgr.GetPendingProfile();
        // store the avatar ID in the profile data
        CurrentProfile.SetAvatarID( m_pAvatarSelect->GetSelectedItemData( 0 ) ); 

        g_AudioMgr.Play("Select_Norm");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_avatar_select::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pAvatarSelect ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            GotoControl( (ui_control*)m_pAvatarSelect );
            g_UiMgr->SetScreenHighlight( m_pAvatarSelect->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================
