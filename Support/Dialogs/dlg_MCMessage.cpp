// 
// 
// 
// dlg_MCMessage.cpp
// Wed Jan 08 10:00:30 2003
// 
// 

//
// Includes
//

#include "entropy.hpp"

#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "stringmgr\stringmgr.hpp"

#include "dlg_MCMessage.hpp"    // Include Header file 

//==============================================================================
// Online Dialog
//==============================================================================

enum controls
{
    IDC_MCMSG_NO,
    IDC_MCMSG_YES,
    IDC_MCMSG_MAYBE
};

ui_manager::control_tem MCMessageControls[] =
{
    { IDC_MCMSG_YES,  "IDS_YES",    "button", 0, -80+(20*1), 80, 20, 0,  0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MCMSG_NO,   "IDS_NO",     "button", 0, -80+(20*2), 80, 20, 0,  1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MCMSG_MAYBE,"IDS_YES",    "button", 0, -80+(20*3), 80, 20, 0,  2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem MCMessageDialog =
{
    "IDS_MESSAGE",
    2, 4,
    sizeof(MCMessageControls)/sizeof(ui_manager::control_tem),
    &MCMessageControls[0],
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

void dlg_mcmessage_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "mcmessage", &MCMessageDialog, &dlg_mcmessage_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_mcmessage_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_mcmessage* pDialog = new dlg_mcmessage;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_mcmessage
//=========================================================================

dlg_mcmessage::dlg_mcmessage( void )
{
}

//=========================================================================

dlg_mcmessage::~dlg_mcmessage( void )
{
    Destroy();
}

//=========================================================================
void dlg_mcmessage::Configure( const xwchar* Title, const xwchar* Yes, const xwchar* No, const xwchar* Message, const xcolor MessageColor, s32 *pResult, const xbool DefaultToNo, const xbool AllowCancel, const xbool AllowSelect )
{
    Configure( Title, Yes, No, NULL, Message, MessageColor, pResult, DefaultToNo, AllowCancel, AllowSelect );
}

//=========================================================================
void dlg_mcmessage::Configure( const xwchar* Title, const xwchar* Yes, const xwchar* No, const xwchar* Maybe, const xwchar* Message, const xcolor MessageColor, s32 *pResult, const xbool DefaultToNo, const xbool AllowCancel, const xbool AllowSelect )
{
    xbool   bYes  = TRUE;
    xbool   bNo   = TRUE;
    xbool   bMaybe= TRUE;
    xwchar  Empty = 0;

    SetLabel( Title );
	m_AllowCancel = AllowCancel;
    m_AllowSelect = AllowSelect;

    if( !Yes )        Yes = &Empty;
    if( !No  )        No  = &Empty;
    if( !Maybe  )     Maybe = &Empty;

    m_pYes->SetLabel( Yes );
    m_pNo ->SetLabel( No  );
    m_pMaybe ->SetLabel( Maybe  );

    if( x_wstrlen(Yes) == 0 )
    {
        m_pYes->SetFlags( (m_pYes->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bYes = FALSE;
    }
	else
	{
        m_pYes->SetFlags( (m_pYes->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
		bYes = TRUE;

        // reset width
        s32 diff = g_UiMgr->TextWidth( 1, Yes ) - 70;
        if( diff > 0 )
        {
            irect cp;
            cp = m_pYes->GetPosition();
            cp.l -= diff/2;
            cp.r += diff/2;
            m_pYes->SetPosition( cp );
        }       
	}

    if( x_wstrlen(No) == 0 )
    {
        m_pNo ->SetFlags( (m_pNo ->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bNo = FALSE;
    }
	else
	{
        m_pNo ->SetFlags( (m_pNo ->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
		bNo = TRUE;

        // reset width
        s32 diff = g_UiMgr->TextWidth( 1, No ) - 70;
        if( diff > 0 )
        {
            irect cp;
            cp = m_pNo->GetPosition();
            cp.l -= diff/2;
            cp.r += diff/2;
            m_pNo->SetPosition( cp );
        }       
    }

    if( x_wstrlen(Maybe) == 0 )
    {
        m_pMaybe ->SetFlags( (m_pMaybe ->GetFlags() & ~WF_VISIBLE) | WF_DISABLED );
        bMaybe = FALSE;
    }
	else
	{
        m_pMaybe ->SetFlags( (m_pMaybe ->GetFlags() & ~WF_DISABLED) | WF_VISIBLE );
		bMaybe = TRUE;

        // reset width
        s32 diff = g_UiMgr->TextWidth( 1, Maybe ) - 70;
        if( diff > 0 )
        {
            irect cp;
            cp = m_pMaybe->GetPosition();
            cp.l -= diff/2;
            cp.r += diff/2;
            m_pMaybe->SetPosition( cp );
        }       
	}

    // Configure for a single button
    if( bNo && !bYes && !bMaybe )
    {
        irect rw = GetPosition();
        irect rb = m_pNo->GetPosition();

//        m_pNo->SetFlags( m_pNo->GetFlags() ^ WF_BUTTON_RIGHT );

        rb.l = rw.GetWidth() / 2 - 70;
        rb.r = rw.GetWidth() / 2 + 70;

        m_pNo->SetPosition( rb );
        m_CurrHL = 1;
    }

    m_Message       = Message;
    m_MessageColor  = MessageColor;

    m_pResult       = pResult;
    m_Timeout       = -1;

    if( m_pResult )
    {
        *m_pResult = DLG_MCMESSAGE_IDLE;
    }

    // Activate appropriate control
    if ( bNo && DefaultToNo )
    {
        GotoControl( m_pNo );
        m_CurrHL = 1;
    }
    else if( bYes )
    {
        GotoControl( m_pYes );
        m_CurrHL = 0;
    }
    else
    {
        m_CurrHL = -1;
    }
}

//=========================================================================
void dlg_mcmessage::SetTimeout          (f32 timeout )
{
    m_Timeout   = timeout;
}

//=========================================================================
void dlg_mcmessage::Close( void )
{
    g_UiMgr->EndDialog(m_UserID,TRUE);
}

//=========================================================================

xbool dlg_mcmessage::Create( s32                        UserID,
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

    m_pYes    = (ui_button*) FindChildByID( IDC_MCMSG_YES   );
    m_pNo     = (ui_button*) FindChildByID( IDC_MCMSG_NO    );
    m_pMaybe  = (ui_button*) FindChildByID( IDC_MCMSG_MAYBE );
	m_AllowCancel = FALSE;
    m_AllowSelect = TRUE;

    m_pYes   ->UseSmallText( TRUE );
    m_pNo    ->UseSmallText( TRUE );
    m_pMaybe ->UseSmallText( TRUE );

    s32 w = Position.GetWidth()/2;
    s32 h = Position.GetHeight();
    irect cp;

    // Move No button
    cp = m_pNo->GetPosition();
    cp.Translate( w-(cp.GetWidth()/2), h-cp.GetHeight() );
    m_pNo->SetPosition( cp );

    // Move Yes button
    cp = m_pYes->GetPosition();
    cp.Translate( w-(cp.GetWidth()/2), h-cp.GetHeight() );
    m_pYes->SetPosition( cp );

    // Move MayBe button
    cp = m_pMaybe->GetPosition();
    cp.Translate( w-(cp.GetWidth()/2), h-cp.GetHeight() );
    m_pMaybe->SetPosition( cp );

    // Clear pointer to result code
    m_pResult = NULL;
    m_Timeout = 8; //-1
    m_CurrHL = -1;

    // play create sound
    g_AudioMgr.Play( "Select_Norm" );
    EnableProgress( FALSE );

    // Return success code
    return Success;
}

//=========================================================================

void dlg_mcmessage::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================
void dlg_mcmessage::Render( s32 ox, s32 oy )
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
        m_pManager->RenderText( g_UiMgr->FindFont("small"), rect, ui_font::h_left|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Label );
        rect.Translate( -1, -1 );
        m_pManager->RenderText( g_UiMgr->FindFont("small"), rect, ui_font::h_left|ui_font::v_center, xcolor(255,252,204,255), m_Label );

        // Render frame
        m_pManager->RenderElement( m_iElement, r, 0 );

        // Render progress bar if needed
        if( m_EnableProgress )
        {
            irect r;
            s32 Center;
            Center = (m_Position.GetWidth() - m_ProgressWidth)/2;

            r.Set( m_Position.l + Center, m_Position.b - 20, m_Position.r - Center, m_Position.b );

            m_pManager->RenderRect( r, XCOLOR_BLACK, FALSE );
            r.Set( m_Position.l + Center, m_Position.b - 20, m_Position.r - (s32)(m_Progress*m_ProgressWidth), m_Position.b );

            m_pManager->RenderRect( r, XCOLOR_GREEN, FALSE );
        }
        // Render Message
        r.Set( m_Position.l+ox, m_Position.t+oy, m_Position.r+ox, m_Position.b+oy );
        r.Deflate( 15, 100 );
        r.t += -60;
        r.b += 15;
		r.l += 8;
		r.r -= 8;
        r.Translate( 2, -2 );
        g_UiMgr->RenderText_Wrap( g_UiMgr->FindFont("small"), r, ui_font::h_center|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Message );
        r.Translate( -2, -2 );
        //g_UiMgr->RenderText_Wrap( g_UiMgr->FindFont("small"), r, ui_font::h_center|ui_font::v_center, m_MessageColor, m_Message );
        g_UiMgr->RenderText_Wrap( g_UiMgr->FindFont("small"), r, ui_font::h_center|ui_font::v_center, xcolor(255,252,204,255), m_Message );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================
void dlg_mcmessage::OnUpdate(ui_win *pWin,f32 DeltaTime)
{
    (void)pWin;

    if (m_Timeout >=0.0f)
    {
        m_Timeout -= DeltaTime;
        if (m_Timeout <=0.0f)
        {
            m_pManager->EndDialog(m_UserID,TRUE);
        }
    } 


    s32 highLight = -1;

    if( m_CurrHL != -1 )
    {
        if( m_pYes->GetFlags(WF_HIGHLIGHT) )
            highLight = 0;
        else if( m_pNo->GetFlags(WF_HIGHLIGHT) )
            highLight = 1;
        else if( m_pMaybe->GetFlags(WF_HIGHLIGHT) )
            highLight = 2;

        if( highLight != m_CurrHL )
        {
            if( highLight != -1 )
                g_AudioMgr.Play("Cusor_Norm");

            m_CurrHL = highLight;
        }
    }
}

//=========================================================================
void dlg_mcmessage::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================
void dlg_mcmessage::OnPadSelect( ui_win* pWin )
{
    if( !m_AllowSelect )
        return;

    if( pWin == (ui_win*)m_pYes )
    {
        if( m_pResult )
            *m_pResult = DLG_MCMESSAGE_YES;       

        // Close dialog and step back
        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
    else if( pWin == (ui_win*)m_pNo )
    {
        if( m_pResult )
            *m_pResult = DLG_MCMESSAGE_NO;

        // Close dialog and step back
        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
    else if( pWin == (ui_win*)m_pMaybe )
    {
        if( m_pResult )
            *m_pResult = DLG_MCMESSAGE_MAYBE;

        // Close dialog and step back
        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
#ifdef TARGET_XBOX
    else
    {
        // default to yes
        if( m_pResult )
            *m_pResult = DLG_MCMESSAGE_YES;

        // Close dialog and step back
        g_AudioMgr.Play("Select_Norm");
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
#endif
}

//=========================================================================
void dlg_mcmessage::OnPadBack( ui_win* pWin )
{
    (void)pWin;

	if (m_AllowCancel)
	{
        if( m_pResult )
            *m_pResult = DLG_MCMESSAGE_NO;

        g_AudioMgr.Play("Backup");
        g_UiMgr->EndDialog(m_UserID,TRUE);
	}
}

//=========================================================================

void dlg_mcmessage::OnNotify( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;
}

//=========================================================================

void dlg_mcmessage::UpdateControls( void )
{
}

//=========================================================================

void dlg_mcmessage::EnableProgress( xbool Enabled, s32 MaxWidth )
{
    m_EnableProgress = Enabled;
    m_Progress = 0.0f;
    m_ProgressWidth = MaxWidth;
}

//=========================================================================

void dlg_mcmessage::SetProgress( f32 Progress )
{
    m_Progress = Progress;
}