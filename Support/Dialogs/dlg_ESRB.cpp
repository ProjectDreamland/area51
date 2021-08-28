// 
// 
// 
// dlg_esrb.cpp
// 
// 

//
// Includes
//

#include "entropy.hpp"

#include "ui\ui_manager.hpp"

#include "dlg_ESRB.hpp"   
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "stateMgr/StateMgr.hpp"
#include "Gamelib\LevelLoader.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

//=========================================================================
//  Esrb Dialog
//=========================================================================

enum controls
{
    IDC_ESRB_MESSAGE,
};

ui_manager::control_tem Esrb_Controls[] =
{
    { IDC_ESRB_MESSAGE, "IDS_ESRB_NOTICE", "text", 0, 200, 480,  30, 0, 0, 1, 1, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
};

ui_manager::dialog_tem Esrb_Dialog =
{
    "IDS_NULL",
    1, 9,
    sizeof(Esrb_Controls)/sizeof(ui_manager::control_tem),
    &Esrb_Controls[0],
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

void dlg_esrb_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "ESRB", &Esrb_Dialog, &dlg_esrb_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_esrb_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_esrb* pDialog = new dlg_esrb;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_esrb
//=========================================================================

dlg_esrb::dlg_esrb( void )
{
}

//=========================================================================

dlg_esrb::~dlg_esrb( void )
{
    Destroy();
}


//=========================================================================

xbool dlg_esrb::Create( s32                        UserID,
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

    // initialize esrb text
    m_pESRBText	= (ui_text*)   FindChildByID( IDC_ESRB_MESSAGE );
    m_pESRBText ->SetFlag(ui_win::WF_VISIBLE, TRUE);
    m_pESRBText ->SetLabelColor( xcolor(230, 230, 230, 255) );
	GotoControl( (ui_control*)m_pESRBText );

#if (defined(bwatson)||defined(jpcossigny)) && defined(X_DEBUG)
    m_WaitTime = 0.5f;
#else
#ifdef TARGET_XBOX
    m_WaitTime = 0.1f;
#else
    m_WaitTime = 5.0f;
#endif
#endif

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_esrb::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_esrb::Render( s32 ox, s32 oy )
{
    // Don't render the ESRB screen outside of America
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
        return;
#endif

	ui_dialog::Render( ox, oy );
}

//=========================================================================

void dlg_esrb::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================
void dlg_esrb::OnPadHelp( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void dlg_esrb::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    if( m_WaitTime > 0.0f )
    {
        if( m_WaitTime < DeltaTime )
        {
            m_WaitTime = 0.0f;
            m_State = DIALOG_STATE_SELECT;
            m_pESRBText->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
        }
        else
        {
            m_WaitTime -= DeltaTime;
        }
    }
}

//=========================================================================
