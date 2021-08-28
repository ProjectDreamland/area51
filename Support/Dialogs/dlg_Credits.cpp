//==============================================================================
//
//  dlg_Credits.cpp
//
//  Copyright (c) 2005 Midway West. All rights reserved.
//
//  define HERE
//
//==============================================================================

//==========================================================================
// INCLUDE
//==========================================================================
#include "entropy.hpp"

#include "ui\ui_manager.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_bitmap.hpp"

#include "dlg_Credits.hpp"  
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "memcardmgr\memcardmgr.hpp" 
#include "stateMgr/StateMgr.hpp"
#include "ui\ui_font.hpp"
#include "MoviePlayer/MoviePlayer.hpp"

#include "ui/ui_manager.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

extern xstring SelectBestClip( const char* pName );

typedef struct _Sweep
{
    f32 StartTime,EndTime,StartValue,EndValue;
}Sweep;

#define SWEEP_FRAME_MAX     30.0f
#define FINAL_PIXEL_COLOR   140.0f
Sweep Sweeps[2] = 
{
    {5.0f,SWEEP_FRAME_MAX,     255.0f,FINAL_PIXEL_COLOR},
    {0.0f  ,5.0f            ,    0.0f,255.0f}
};

//==========================================================================
// DEFINES
//==========================================================================
#define DEFAULT_LINE_TIME       (8.0f)
#define FADE_BOUNDS_OFFSETS     (80)
#define FADE_MIN_START          (0.5f)
#define FADE_MAX_START          (2.5f)
//==========================================================================
// GLOBAL
//==========================================================================
enum controls
{
    IDC_CREDIT_STRING_1,
};

ui_manager::control_tem CreditsControls[] =
{
    { IDC_CREDIT_STRING_1, "IDS_PRESS_START_TEXT", "text",     0, 308, 480,  30, 0, 0, 1, 1, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
};

ui_manager::dialog_tem CreditsDialog =
{
        "IDS_CREIDTS_SCREEN",
        1, 9,
        sizeof(CreditsControls)/sizeof(ui_manager::control_tem),
        &CreditsControls[0],
        0
};

//==========================================================================
// FUNTIONS
//==========================================================================

//=========================================================================
//  Registration function
//=========================================================================

void dlg_credits_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "credits screen", &CreditsDialog, &dlg_credits_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_credits_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_credits* pDialog = new dlg_credits;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_credits
//=========================================================================

dlg_credits::dlg_credits( void )
{
}

//=========================================================================

dlg_credits::~dlg_credits( void )
{
}

//=========================================================================

xbool dlg_credits::Create( s32                        UserID,
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

    // Load Credits String Table
    g_StringTableMgr.LoadTable( "credits", xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "ENG_Credits_strings.stringbin" ) );

    // init
    m_LineIndex = 0;
    m_PageLineCount = 0;
    m_CreditsDone = FALSE;
    InitCreditLines();
    
    // Build First Page
    BuildPage();

    // disable the highlight
    g_UiMgr->DisableScreenHighlight();

    // diable the background movie
    //DisableBackgoundMovie();

    // start up the start movie
    //g_StateMgr.PlayMovie( "MenuBackGround", TRUE, TRUE );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_credits::Destroy( void )
{
    // unload credits strings
    g_StringTableMgr.UnloadTable("credits");

    // enable the highlight
    g_UiMgr->EnableScreenHighlight();

    ui_dialog::Destroy();
}

//=========================================================================

void dlg_credits::Render( s32 ox, s32 oy )
{
    for( s32 index = 0 ; index < m_PageLineCount ; index ++)
    {
        g_UiMgr->GetFont(m_CreditLines[index].m_Font)->RenderStateControlledText(
                             m_CreditLines[index].m_Rect, 
                             m_CreditLines[index].m_RenderFlags,                            
                             m_CreditLines[index].m_Color, 
                             m_CreditLines[index].m_String,
                             (void*)m_CreditLines[index].m_CustomRenderStruct);
    }

    // finally render all the normal dialog stuff
    ui_dialog::Render( ox, oy );
}

//=========================================================================

void dlg_credits::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void dlg_credits::OnPadBack( ui_win* pWin )
{
    (void)pWin;
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // Credits EXIT
        
        // close the credits background movie
        //g_StateMgr.CloseMovie();
        // renable the menu background
        //g_StateMgr.EnableBackgroundMovie();
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================


void dlg_credits::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // Total Page Time
    m_CurrentPageTime += DeltaTime;
    
    if( m_CurrentPageTime >= m_PageTimer )
    {
        // if this page is done then fade the chars out.
        xbool TurnPage = TRUE;

        for( s32 index = 0 ; index < m_PageLineCount ; index ++)
        {       
            for( s32 i = 0 ; i < m_CreditLines[index].m_CharCount ; i ++ )
            {
                // Fade All Chars
                if( m_CreditLines[index].m_CustomRenderStruct[i].m_Value > 0.0f )
                {
                    m_CreditLines[index].m_CustomRenderStruct[i].m_Value -= 8;
                    if( m_CreditLines[index].m_CustomRenderStruct[i].m_Value < 0.0f )
                        m_CreditLines[index].m_CustomRenderStruct[i].m_Value = 0.0f;

                    TurnPage = FALSE;
                }
            }
        }

        // Next page.
        if( TurnPage )
        { 
            if( BuildPage() ) 
            {
                // EXIT here ..
                m_State = DIALOG_STATE_BACK;

                // Use this for looping.
                //m_LineIndex = 0;
                //m_CreditsDone = FALSE;
            }
        }
    }
    else
    {
        // handle effects of chars poping in.

        for( s32 index = 0 ; index < m_PageLineCount ; index ++)
        {       
            for( s32 i = 0 ; i < m_CreditLines[index].m_CharCount ; i ++ )
            {
                m_CreditLines[index].m_FadeDelay[i]-=DeltaTime;

                if( (m_CreditLines[index].m_FadeDelay[i] <= 0.0f) && m_CreditLines[index].m_CustomRenderStruct[i].m_Frame == 0)
                {
                    m_CreditLines[index].m_CustomRenderStruct[i].m_State = ui_font::s_render;
                    m_CreditLines[index].m_CustomRenderStruct[i].m_Frame++;
                    m_CreditLines[index].m_CustomRenderStruct[i].m_Value = 0;
                }
                else if( m_CreditLines[index].m_CustomRenderStruct[i].m_State == ui_font::s_render )
                {
                    m_CreditLines[index].m_CustomRenderStruct[i].m_Frame++;

                    // Find what alpha we are in.
                    {
                        if(m_CreditLines[index].m_CustomRenderStruct[i].m_Frame < SWEEP_FRAME_MAX)
                        {
                            for( s32 s = 0 ; s < 2 ; s++ )
                            {
                                if( m_CreditLines[index].m_CustomRenderStruct[i].m_Frame <= Sweeps[s].EndTime && m_CreditLines[index].m_CustomRenderStruct[i].m_Frame >= Sweeps[s].StartTime )
                                {
                                    m_CreditLines[index].m_CustomRenderStruct[i].m_Value = ((((m_CreditLines[index].m_CustomRenderStruct[i].m_Frame - Sweeps[s].StartTime) / (Sweeps[s].EndTime - Sweeps[s].StartTime)) * (Sweeps[s].EndValue-Sweeps[s].StartValue)) + Sweeps[s].StartValue);
                                }
                            }
                        }
                        else
                        {
                            m_CreditLines[index].m_CustomRenderStruct[i].m_Value = FINAL_PIXEL_COLOR;
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void dlg_credits::InitCreditLines( void )
{
    for( s32 index = 0 ; index < MAX_CREDIT_LINES_PER_PAGE ; index++ )
    {
        m_CreditLines[index].m_Type = LINE_TYPE_NORMAL;
        m_CreditLines[index].m_Color = xcolor(255,255,255,255);
        x_strcpy( m_CreditLines[index].m_Font, "large" );
        m_CreditLines[index].m_Rect = irect(0,0,0,0);
        m_CreditLines[index].m_String[0] = 0;
        m_CreditLines[index].m_RenderFlags = 0;
        m_CreditLines[index].m_StringLength= 0;
    }
}

//=========================================================================

xbool dlg_credits::BuildPage( void )
{
    InitCreditLines();

    s32   CurrentLine = 0;
    char  LookupID[32];
    
    // Build first ID to find.
    x_sprintf(LookupID, "IDS_CREDIT_LINE_%03i",m_LineIndex);

    // Check for end of FILE.
    if( m_CreditsDone )
        return( TRUE );
    
    // Loop untill new page is found.
    while( 
            CheckLine( g_StringTableMgr( "credits", LookupID )) != CREDITS_NEW_PAGE && 
            CheckLine( g_StringTableMgr( "credits", LookupID )) != CREDITS_DONE
         )
    {
        // Build lines    
        x_wstrcpy(m_CreditLines[CurrentLine].m_String, g_StringTableMgr("credits",LookupID));

        
        m_CreditLines[CurrentLine].m_CharCount = x_wstrlen( m_CreditLines[CurrentLine].m_String );
        for( s32 i = 0 ; i < m_CreditLines[CurrentLine].m_CharCount ; i++ )
        {
            m_CreditLines[CurrentLine].m_CustomRenderStruct[i].m_State  = ui_font::s_no_render;
            m_CreditLines[CurrentLine].m_CustomRenderStruct[i].m_Value  = 0.0f;
            m_CreditLines[CurrentLine].m_CustomRenderStruct[i].m_Frame  = 0;
            m_CreditLines[CurrentLine].m_FadeDelay[i]                   = x_frand(FADE_MIN_START,FADE_MAX_START);
        }

        // Setup Line Types
        if( CheckLine(g_StringTableMgr( "credits", LookupID )) == CREDITS_TITLE_CODE )
        {
            x_strcpy( m_CreditLines[CurrentLine].m_Font, "loadscr" );
            m_CreditLines[CurrentLine].m_Type = LINE_TYPE_TITLE;
            m_CreditLines[CurrentLine].m_RenderFlags = (ui_font::h_center|ui_font::v_center);
        }
        else
        {
            //m_CreditLines[CurrentLine].m_MoveDelay = (0.25f * CurrentLine+1);
            m_CreditLines[CurrentLine].m_RenderFlags = (ui_font::h_center|ui_font::v_center);
        }

        // done.. next line.
        x_sprintf(LookupID, "IDS_CREDIT_LINE_%03i",++m_LineIndex);
        CurrentLine++;
    }

    // Is this the last page?
    if ( CheckLine( g_StringTableMgr( "credits", LookupID )) == CREDITS_DONE )
        m_CreditsDone = TRUE;

    // Set number of Lines this page.
    ASSERT( CurrentLine < MAX_CREDIT_LINES_PER_PAGE );
    m_PageLineCount = CurrentLine;

    // Given number of lines.. build the rects for rendering.
    s32 line;
    s32 TotalPageHeight = 0;
    s32 LineOffset = 0;

    // Get Total line height in font pixels
    for( line = 0 ; line < m_PageLineCount ; line++ )
    {
        TotalPageHeight += g_UiMgr->GetLineHeight( g_UiMgr->FindFont(m_CreditLines[line].m_Font) );

        irect TextRect;
        g_UiMgr->TextSize( g_UiMgr->FindFont(m_CreditLines[line].m_Font), TextRect, m_CreditLines[line].m_String, -1);
        m_CreditLines[line].m_StringLength = TextRect.GetWidth();
    }

    // Setup spaceing 
    for( line = 0 ; line < m_PageLineCount ; line++ )
    {
        s32 PageCenter = g_UiMgr->GetUserBounds(g_UiUserID).b / 2;
       
        m_CreditLines[line].m_Rect.l = 0;
        m_CreditLines[line].m_Rect.r = g_UiMgr->GetUserBounds(g_UiUserID).r;

        m_CreditLines[line].m_Rect.t = (PageCenter - (TotalPageHeight / 2)) +  LineOffset;
        m_CreditLines[line].m_Rect.b = m_CreditLines[line].m_Rect.t + g_UiMgr->GetLineHeight( g_UiMgr->FindFont(m_CreditLines[line].m_Font) ); 

        LineOffset += g_UiMgr->GetLineHeight( g_UiMgr->FindFont(m_CreditLines[line].m_Font) );
    }

    m_PageTimer =  DEFAULT_LINE_TIME;
    m_CurrentPageTime = 0.0f;
    m_LineIndex++;

    return( FALSE );
}

//=========================================================================

s32 dlg_credits::CheckLine( const xwchar* pString )
{
    // This function checks for Credit specific codes in the string that 
    // only apply to the credit screen.
    while( *pString )
    {
        xwchar c = *pString++;

        if( c == 0xAB ) // '«'
        {
            s32 ButtonCode = g_UiMgr->LookUpButtonCode( pString, 0 );
            if( ButtonCode != -1 )
            {
                // for the start button, double it... 
                if( ButtonCode == NEW_CREDIT_PAGE)
                {
                    return( CREDITS_NEW_PAGE );
                }
                else if ( ButtonCode == CREDIT_TITLE_LINE )
                {
                    return( CREDITS_TITLE_CODE );
                }
                else if ( ButtonCode == CREDIT_END )
                {
                    return( CREDITS_DONE );
                }
            }
        }
    }
    return ( CREDITS_NO_CODE );
}
