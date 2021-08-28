//=========================================================================
//
//  dlg_submenu.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "stringmgr\stringmgr.hpp"

#include "dlg_SubMenu.hpp"    // Include Header file 

//==============================================================================
// dlg_submenu
//==============================================================================

enum controls
{
    IDC_BUTTON_ONE,
    IDC_BUTTON_TWO,
    IDC_BUTTON_THREE,
    IDC_BUTTON_FOUR,
    IDC_BUTTON_FIVE,
};

ui_manager::control_tem SubMenuControls[] =
{
    { IDC_BUTTON_ONE,    "IDS_NULL",    "button", 3,  60, 294, 20, 0,  0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_BUTTON_TWO,    "IDS_NULL",    "button", 3,  85, 294, 20, 0,  1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_BUTTON_THREE,  "IDS_NULL",    "button", 3, 110, 294, 20, 0,  2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_BUTTON_FOUR,   "IDS_NULL",    "button", 3, 135, 294, 20, 0,  3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_BUTTON_FIVE,   "IDS_NULL",    "button", 3, 160, 294, 20, 0,  4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem SubMenuDialog =
{
    "IDS_NULL",
    2, 5,
    sizeof(SubMenuControls)/sizeof(ui_manager::control_tem),
    &SubMenuControls[0],
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

void dlg_submenu_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "submenu", &SubMenuDialog, &dlg_submenu_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_submenu_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_submenu* pDialog = new dlg_submenu;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_submenu
//=========================================================================

dlg_submenu::dlg_submenu( void )
{
}

//=========================================================================

dlg_submenu::~dlg_submenu( void )
{
    Destroy();
}

//=========================================================================

void dlg_submenu::Configure( const xwchar* Title, const xwchar* ItemOne, const xwchar* ItemTwo, const xwchar* ItemThree, const xwchar* Message, s32 *pResult )
{
    Configure(Title,ItemOne,ItemTwo,ItemThree,NULL,NULL,Message,pResult);
}

//=========================================================================

void dlg_submenu::Configure( const xwchar* Title, const xwchar* ItemOne, const xwchar* ItemTwo, const xwchar* ItemThree, const xwchar* ItemFour, const xwchar* Message, s32 *pResult )
{
    Configure(Title,ItemOne,ItemTwo,ItemThree,ItemFour,NULL,Message,pResult);
}

//=========================================================================

void dlg_submenu::Configure( const xwchar* Title, const xwchar* ItemOne, const xwchar* ItemTwo, const xwchar* ItemThree, const xwchar* ItemFour, const xwchar* ItemFive, const xwchar* Message, s32 *pResult )
{
    xbool   bOne   = TRUE;
    xbool   bTwo   = TRUE;
    xbool   bThree = TRUE;
    xbool   bFour  = TRUE;
    xbool   bFive  = TRUE;
    xwchar  Empty = 0;

    SetLabel( Title );

    if( !ItemOne   )    ItemOne   = &Empty;
    if( !ItemTwo   )    ItemTwo   = &Empty;
    if( !ItemThree )    ItemThree = &Empty;
    if( !ItemFour  )    ItemFour  = &Empty;
    if( !ItemFive  )    ItemFive  = &Empty;

    m_pButtonOne   ->SetLabel( ItemOne   );
    m_pButtonTwo   ->SetLabel( ItemTwo   );
    m_pButtonThree ->SetLabel( ItemThree );
    m_pButtonFour  ->SetLabel( ItemFour  );
    m_pButtonFive  ->SetLabel( ItemFive  );

    if( x_wstrlen(ItemOne) == 0 )
    {
        m_pButtonOne->SetFlags( (m_pButtonOne->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bOne = FALSE;
    }
	else
	{
        m_pButtonOne->SetFlags( (m_pButtonOne->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
		bOne = TRUE;
	}

    if( x_wstrlen(ItemTwo) == 0 )
    {
        m_pButtonTwo ->SetFlags( (m_pButtonTwo ->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bTwo = FALSE;
    }
	else
	{
        m_pButtonTwo ->SetFlags( (m_pButtonTwo ->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
		bTwo = TRUE;
	}

    if( x_wstrlen(ItemThree) == 0 )
    {
        m_pButtonThree ->SetFlags( (m_pButtonThree ->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bThree = FALSE;
    }
	else
	{
        m_pButtonThree ->SetFlags( (m_pButtonThree ->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
		bThree = TRUE;
	}
    if( x_wstrlen(ItemFour) == 0 )
    {
        m_pButtonFour ->SetFlags( (m_pButtonFour ->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bFour = FALSE;
    }
    else
    {
        m_pButtonFour ->SetFlags( (m_pButtonFour ->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
        bFour = TRUE;
    }
    if( x_wstrlen(ItemFive) == 0 )
    {
        m_pButtonFive ->SetFlags( (m_pButtonFive ->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bFive = FALSE;
    }
    else
    {
        m_pButtonFive ->SetFlags( (m_pButtonFive ->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
        bFive = TRUE;
    }

    m_Message       = Message;
    m_pResult       = pResult;

    if( m_pResult )
    {
        *m_pResult = DLG_SUBMESSAGE_IDLE;
    }

    GotoControl( m_pButtonOne );
    m_CurrHL = 0;
}

//=========================================================================

void dlg_submenu::Close( void )
{
    g_UiMgr->EndDialog(m_UserID,TRUE);
}

//=========================================================================

xbool dlg_submenu::Create( s32                        UserID,
                                ui_manager*                pManager,
                                ui_manager::dialog_tem*    pDialogTem,
                                const irect&               Position,
                                ui_win*                    pParent,
                                s32                        Flags,
                                void*                      pUserData)
{
    xbool   Success = FALSE;

    (void)pUserData;

    ASSERT( pManager );

    // Make it input modal
    Flags |= WF_INPUTMODAL;

    // Do dialog creation
    Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    m_bDoBlackout       = TRUE;
    m_BackgroundColor   = xcolor (19,59,14,255); 

    m_iElement = m_pManager->FindElement( "frame2" );
    ASSERT( m_iElement != -1 );

    m_pButtonOne    = (ui_button*) FindChildByID( IDC_BUTTON_ONE   );
    m_pButtonTwo    = (ui_button*) FindChildByID( IDC_BUTTON_TWO   );
    m_pButtonThree  = (ui_button*) FindChildByID( IDC_BUTTON_THREE );
    m_pButtonFour   = (ui_button*) FindChildByID( IDC_BUTTON_FOUR  );
    m_pButtonFive   = (ui_button*) FindChildByID( IDC_BUTTON_FIVE  );

    m_pButtonOne   ->UseSmallText( TRUE );
    m_pButtonTwo   ->UseSmallText( TRUE );
    m_pButtonThree ->UseSmallText( TRUE );
    m_pButtonFour  ->UseSmallText( TRUE );
    m_pButtonFive  ->UseSmallText( TRUE );

    // Clear pointer to result code
    m_pResult = NULL;
    m_CurrHL = -1;

    // play create sound
    g_AudioMgr.Play( "Select_Norm" );

    // Return success code
    return Success;
}

//=========================================================================

void dlg_submenu::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================
void dlg_submenu::Render( s32 ox, s32 oy )
{
	// Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // dim the background dialog
        if( m_bDoBlackout )
        {
	        s32 XRes, YRes;
            eng_GetRes(XRes, YRes);
    #ifdef TARGET_PS2
            // Nasty hack to force PS2 to draw to rb.l = 0
            irect rb( -1, 0, XRes, YRes );
    #else
            irect rb( 0, 0, XRes, YRes );
    #endif
            g_UiMgr->RenderGouraudRect(rb, xcolor(0,0,0,180),
                                           xcolor(0,0,0,180),
                                           xcolor(0,0,0,180),
                                           xcolor(0,0,0,180),FALSE);
        }

        // Get window rectangle
        irect   r;
        r.Set( m_Position.l+ox, m_Position.t+oy, m_Position.r+ox, m_Position.b+oy );
        irect   rb = r;
        rb.Deflate( 1, 1 );

        // Render background color
        if( m_BackgroundColor.A > 0 )
        {
            m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );
        }

        // Render Title
        irect rect = r;
        rect.Deflate( 1, 1 );
        rect.SetHeight( 22 );
        xcolor c1 = xcolor (25,77,18,255); 
        m_pManager->RenderRect( rect, c1, FALSE );

        rect.Deflate( 8, 0 );
        rect.Translate( 1, -1 );
        m_pManager->RenderText( g_UiMgr->FindFont("large"), rect, ui_font::h_left|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Label );
        rect.Translate( -1, -1 );
        m_pManager->RenderText( g_UiMgr->FindFont("large"), rect, ui_font::h_left|ui_font::v_center, xcolor(255,252,204,255), m_Label );

        // Render frame
        m_pManager->RenderElement( m_iElement, r, 0 );

        // Render Message
        //r.Set( m_Position.l+ox, m_Position.t+oy, m_Position.r+ox, m_Position.b+oy );
        //r.Deflate( 15, 100 );
        //r.t += -40;
        //r.b += 15;
		//r.l += 8;
		//r.r -= 8;
        //r.Translate( 2, -2 );
        //g_UiMgr->RenderText_Wrap( g_UiMgr->FindFont("small"), r, ui_font::h_center|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Message );
        //r.Translate( -2, -2 );
        //g_UiMgr->RenderText_Wrap( g_UiMgr->FindFont("small"), r, ui_font::h_center|ui_font::v_center, xcolor(255,252,204,255), m_Message );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void dlg_submenu::OnUpdate(ui_win *pWin, f32 DeltaTime)
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    if( m_CurrHL != -1 )
    {
        if( m_pButtonOne->GetFlags(WF_HIGHLIGHT) )
            highLight = 0;
        else if( m_pButtonTwo->GetFlags(WF_HIGHLIGHT) )
            highLight = 1;
        else if( m_pButtonThree->GetFlags(WF_HIGHLIGHT) )
            highLight = 2;
        else if( m_pButtonFour->GetFlags(WF_HIGHLIGHT) )
            highLight = 4;


        if( highLight != m_CurrHL )
        {
            if( highLight != -1 )
                g_AudioMgr.Play("Cusor_Norm");

            m_CurrHL = highLight;
        }
    }
}

//=========================================================================

void dlg_submenu::OnPadSelect( ui_win* pWin )
{
    if( pWin == (ui_win*)m_pButtonOne )
    {
        // Close dialog and step back
        if( m_pResult )
            *m_pResult = DLG_SUBMESSAGE_ONE;
        
        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
    else if( pWin == (ui_win*)m_pButtonTwo )
    {
        if( m_pResult )
            *m_pResult = DLG_SUBMESSAGE_TWO;

        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog(m_UserID,TRUE);
    }
    else if( pWin == (ui_win*)m_pButtonThree )
    {
        if( m_pResult )
            *m_pResult = DLG_SUBMESSAGE_THREE;

        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog(m_UserID,TRUE);
    }
    else if( pWin == (ui_win*)m_pButtonFour )
    {
        if( m_pResult )
            *m_pResult = DLG_SUBMESSAGE_FOUR;

        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog(m_UserID,TRUE);
    }
    else if( pWin == (ui_win*)m_pButtonFive )
    {
        if( m_pResult )
            *m_pResult = DLG_SUBMESSAGE_FIVE;

        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog(m_UserID,TRUE);
    }
}

//=========================================================================
void dlg_submenu::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_pResult )
        *m_pResult = DLG_SUBMESSAGE_BACK;

    g_AudioMgr.Play("Backup");
    g_UiMgr->EndDialog(m_UserID,TRUE);
}

//=========================================================================

