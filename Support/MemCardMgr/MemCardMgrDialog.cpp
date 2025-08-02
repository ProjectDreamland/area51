///////////////////////////////////////////////////////////////////////////////

// 
// MemCardMgrDialog.cpp
// Wed Feb 26 11:43:28 2003
// 
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
///////////////////////////////////////////////////////////////////////////////

#include "MemCardMgr.hpp"
#include "e_Memcard.hpp"
#include "StringMgr/StringMgr.hpp"
#include "Dialogs/dlg_MCMessage.hpp"


///////////////////////////////////////////////////////////////////////////////
//
// Methods
//
///////////////////////////////////////////////////////////////////////////////



//==---------------------------------------------------------------------------

void MemCardMgr::WarningBox( const char* pTitle,
                             const char* pMessage,
                             xbool       AllowPrematureExit )
{
    (void)pTitle;
    (void)pMessage;
    (void)AllowPrematureExit;
    
    const xwchar* pDone;

    //if( g_StateMgr.GetState() == SM_PLAYING_GAME )
    if( g_StateMgr.DisplayMemcardDialogs() == FALSE )
    {
        // No memcard dialogs when game is in progress.
        // We should only be getting here if an autosave is in progress!
        m_MessageResult = DLG_MCMESSAGE_YES;
        return;
    }

    ASSERTS( pMessage,"String Missing for WarningBox" );

#ifndef TARGET_XBOX
    if( AllowPrematureExit )
        pDone = g_StringTableMgr( "ui","IDS_DONE" );
    else           
#endif
        pDone = NULL; 

    if( m_pMessage && ( m_MessageResult == DLG_MCMESSAGE_IDLE ) )
    {
        g_UiMgr->EndDialog( g_UiUserID,TRUE );
        m_pMessage = NULL;
        m_DialogsAllocated--;

#ifdef TARGET_PC
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        
        irect dialogRect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        );
        
        dialogRect.Translate(
            (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
            (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
        );
        
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            dialogRect,
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_USE_ABSOLUTE
        );
#else
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            irect(
                0,
                0,
                MC_TEXT_BOX_POPUP_SIZE_W,
                MC_TEXT_BOX_POPUP_SIZE_H
            ), 
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_DLG_CENTER
        );
#endif
        
        m_DialogsAllocated++;
        ASSERT( m_pMessage );
        m_pMessage->Configure( xwstring( pTitle ),
                                NULL,
                                pDone,
                                xwstring( pMessage ),
                                XCOLOR_WHITE,
                                &m_MessageResult,
                                TRUE,
                                AllowPrematureExit,
                                AllowPrematureExit
                            );
    }
    else
    {
#ifdef TARGET_PC
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        
        irect dialogRect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        );
        
        dialogRect.Translate(
            (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
            (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
        );
        
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            dialogRect,
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_USE_ABSOLUTE
        );
#else
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            irect(
                0,
                0,
                MC_TEXT_BOX_POPUP_SIZE_W,
                MC_TEXT_BOX_POPUP_SIZE_H
            ), 
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_DLG_CENTER
        );
#endif

        m_DialogsAllocated++;
        ASSERT( m_pMessage );

        m_pMessage->Configure( xwstring( pTitle ),
            NULL,
            pDone,
            xwstring( pMessage ),
            XCOLOR_WHITE,
            &m_MessageResult,
            TRUE,
            AllowPrematureExit,
            AllowPrematureExit
        );
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::WarningBox( const xwchar* pTitle,
                             const xwchar* pMessage,
                             xbool         AllowPrematureExit )
{
    (void)pTitle;
    (void)pMessage;
    (void)AllowPrematureExit;

    const xwchar* pDone;

    //if( g_StateMgr.GetState() == SM_PLAYING_GAME )
    if( g_StateMgr.DisplayMemcardDialogs() == FALSE )
    {        
        // No memcard dialogs when game is in progress.
        // We should only be getting here if an autosave is in progress!
        m_MessageResult = DLG_MCMESSAGE_YES;
        return;
    }

    ASSERTS( pMessage,"String Missing for WarningBox" );

#ifndef TARGET_XBOX
    if( AllowPrematureExit )  
        pDone = g_StringTableMgr( "ui","IDS_DONE" );
    else                      
#endif
        pDone = NULL; 

    if( m_pMessage && ( m_MessageResult == DLG_MCMESSAGE_IDLE ) )
    {
        g_UiMgr->EndDialog( g_UiUserID,TRUE );
        m_pMessage = NULL;
        m_DialogsAllocated--;
        
#ifdef TARGET_PC
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        
        irect dialogRect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        );
        
        dialogRect.Translate(
            (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
            (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
        );
        
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            dialogRect,
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_USE_ABSOLUTE
        );
#else
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            irect(
                0,
                0,
                MC_TEXT_BOX_POPUP_SIZE_W,
                MC_TEXT_BOX_POPUP_SIZE_H
            ),
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_DLG_CENTER
        );
#endif

        m_DialogsAllocated++;
        ASSERT( m_pMessage );

        m_pMessage->Configure( xwstring( pTitle ),
            NULL,
            pDone,
            xwstring( pMessage ),
            XCOLOR_WHITE,
            &m_MessageResult,
            TRUE,
            AllowPrematureExit,
            AllowPrematureExit
        );
    }
    else
    {
#ifdef TARGET_PC
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        
        irect dialogRect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        );
        
        dialogRect.Translate(
            (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
            (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
        );
        
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            dialogRect,
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_USE_ABSOLUTE
        );
#else
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            irect(
                0,
                0,
                MC_TEXT_BOX_POPUP_SIZE_W,
                MC_TEXT_BOX_POPUP_SIZE_H
            ),
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_DLG_CENTER
        );
#endif

        m_DialogsAllocated++;
        ASSERT( m_pMessage );

        m_pMessage->Configure( xwstring( pTitle ),
            NULL,
            pDone,
            xwstring( pMessage ),
            XCOLOR_WHITE,
            &m_MessageResult,
            TRUE,
            AllowPrematureExit,
            AllowPrematureExit
        );
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::OptionBox( const char*   pTitle,
                            const char*   pMessage,
                            const xwchar *pYes,
                            const xwchar *pNo,
                            const xbool   DefaultToNo,
                            const xbool   AllowCancel )
{
    (void)pTitle;
    (void)pMessage;
    (void)pYes;
    (void)pNo;
    (void)DefaultToNo;
    (void)AllowCancel;

    ASSERTS( pMessage,"String Missing for OptionBox" );
	
    //if( g_StateMgr.GetState() == SM_PLAYING_GAME )
    if( g_StateMgr.DisplayMemcardDialogs() == FALSE )
    {
        // No memcard dialogs when game is in progress.
        // We should only be getting here if an autosave is in progress!
        m_MessageResult = DLG_MCMESSAGE_YES;
        return;
    }

    if( pYes == NULL )
    {
        pYes = g_StringTableMgr( "ui","IDS_YES" );
    }
    if( pNo == NULL )
    {
        pNo = g_StringTableMgr( "ui","IDS_NO" );
    }

    if( m_pMessage && ( m_MessageResult == DLG_MCMESSAGE_IDLE ) )
    {
        g_UiMgr->EndDialog( g_UiUserID,TRUE );
        m_DialogsAllocated--;
        m_pMessage = NULL;

#ifdef TARGET_PC
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        
        irect dialogRect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        );
        
        dialogRect.Translate(
            (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
            (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
        );
        
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            dialogRect,
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_USE_ABSOLUTE
        );
#else
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            irect(
                0,
                0,
                MC_TEXT_BOX_POPUP_SIZE_W,
                MC_TEXT_BOX_POPUP_SIZE_H
            ),
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_DLG_CENTER
        );
#endif

        m_DialogsAllocated++;
        ASSERT( m_pMessage );
        m_pMessage->Configure( xwstring( pTitle ),
                               pYes,
                               pNo,
                               xwstring( pMessage ),
                               XCOLOR_WHITE,
                               &m_MessageResult,
                               DefaultToNo,
                               AllowCancel
                           );
    }
    else
    {
#ifdef TARGET_PC
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        
        irect dialogRect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        );
        
        dialogRect.Translate(
            (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
            (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
        );
        
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            dialogRect,
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_USE_ABSOLUTE
        );
#else
        m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
            g_UiUserID,
            "mcmessage",
            irect(
                0,
                0,
                MC_TEXT_BOX_POPUP_SIZE_W,
                MC_TEXT_BOX_POPUP_SIZE_H
            ),
            NULL,
            ui_win::WF_VISIBLE
            | ui_win::WF_BORDER
            | ui_win::WF_DLG_CENTER
        );
#endif

        m_DialogsAllocated++;
        ASSERT( m_pMessage );

        m_pMessage->Configure( xwstring( pTitle ),
            pYes,
            pNo,
            xwstring( pMessage ),
            XCOLOR_WHITE,
            &m_MessageResult,
            DefaultToNo,
            AllowCancel
        );
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::OptionBox( const xwchar* pTitle,
                            const xwchar* pMessage,
                            const xwchar *pYes,
                            const xwchar *pNo,
                            const xwchar *pMaybe,
                            const xbool   DefaultToNo,
                            const xbool   AllowCancel )
{
    (void)pTitle;
    (void)pMessage;
    (void)pMaybe;
    (void)pYes;
    (void)pNo;
    (void)DefaultToNo;
    (void)AllowCancel;
	
    //if( g_StateMgr.GetState() == SM_PLAYING_GAME )
    if( g_StateMgr.DisplayMemcardDialogs() == FALSE )
    {
        // No memcard dialogs when game is in progress.
        // We should only be getting here if an autosave is in progress!
        m_MessageResult = DLG_MCMESSAGE_YES;
        return;
    }

    ASSERTS( pMessage,"String Missing for OptionBox" );

    if( ! pYes )
          pYes = g_StringTableMgr( "ui","IDS_YES" );

    if( ! pNo )
          pNo = g_StringTableMgr( "ui","IDS_NO" );

    if( m_pMessage && ( m_MessageResult == DLG_MCMESSAGE_IDLE ) )
    {
        g_UiMgr->EndDialog( g_UiUserID,TRUE );
        m_DialogsAllocated--;
        m_pMessage = NULL;
    }
    
#ifdef TARGET_PC
    s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
    
    irect dialogRect(
        0,
        0,
        MC_TEXT_BOX_POPUP_SIZE_W,
        MC_TEXT_BOX_POPUP_SIZE_H
    );
    
    dialogRect.Translate(
        (XRes - MC_TEXT_BOX_POPUP_SIZE_W) / 2,
        (YRes - MC_TEXT_BOX_POPUP_SIZE_H) / 2
    );
    
    m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
        g_UiUserID,
        "mcmessage",
        dialogRect,
        NULL,
        ui_win::WF_VISIBLE
        | ui_win::WF_BORDER
        | ui_win::WF_USE_ABSOLUTE
    );
#else
    m_pMessage=( dlg_mcmessage* )g_UiMgr->OpenDialog(
        g_UiUserID,
        "mcmessage",
        irect(
            0,
            0,
            MC_TEXT_BOX_POPUP_SIZE_W,
            MC_TEXT_BOX_POPUP_SIZE_H
        ),
        NULL,
        ui_win::WF_VISIBLE
        | ui_win::WF_BORDER
        | ui_win::WF_DLG_CENTER
    );
#endif

    m_DialogsAllocated++;
    ASSERT( m_pMessage );

    m_pMessage->Configure( xwstring( pTitle ),
        pYes,
        pNo,
        pMaybe,
        xwstring( pMessage ),
        XCOLOR_WHITE,
        &m_MessageResult,
        DefaultToNo,
        AllowCancel
    );
}

//==---------------------------------------------------------------------------

void MemCardMgr::PopUpBox( const xwchar* pTitle,const xwchar* pMessage, const xwchar* pNavText, const xbool bYes, const xbool bNo, const xbool bMaybe )
{
    (void)pTitle;
    (void)pMessage;
    (void)pNavText;
    (void)bYes;
    (void)bNo;
    (void)bMaybe;

    ASSERTS( pMessage,"String Missing for PopUpBox" );

    if( m_pMessage && ( m_MessageResult == DLG_MCMESSAGE_IDLE ) )
    {
        g_UiMgr->EndDialog( g_UiUserID,TRUE );
        m_DialogsAllocated--;
        m_pMessage = NULL;
    }

    dlg_popup *pPopUp;

#ifdef TARGET_PC
    s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
    
    irect r(0, 0, XRes, YRes);
#else
    irect r = g_UiMgr->GetUserBounds(g_UiUserID);
#endif

    pPopUp = (dlg_popup*)g_UiMgr->OpenDialog(
        g_UiUserID, 
        "popup", 
        r, 
        NULL, 
        ui_win::WF_VISIBLE | ui_win::WF_BORDER | ui_win::WF_DLG_CENTER | ui_win::WF_INPUTMODAL | ui_win::WF_USE_ABSOLUTE
    );

#ifdef TARGET_PC
    irect Size(0, 0, 400, 240);
#else
    irect Size(0, 0, 400, 240);
#endif

    pPopUp->Configure( 
        Size,
        pTitle, 
        bYes, 
        bNo, 
        bMaybe, 
        pMessage,
        pNavText,
        &m_MessageResult
    );

    m_pMessage = (dlg_mcmessage*)pPopUp; 

    m_DialogsAllocated++;
}

//==---------------------------------------------------------------------------

void MemCardMgr::EnableProgress( xbool Enabled )
{
    if( m_pMessage )
    {
        m_pMessage->EnableProgress( Enabled, 200 );
    }
}
//==---------------------------------------------------------------------------

void MemCardMgr::UpdateProgress( f32 Progress )
{
    if( m_pMessage )
    {
        m_pMessage->SetProgress( Progress );
    }
}