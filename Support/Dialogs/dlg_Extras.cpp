//=========================================================================
//
//  dlg_extras.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_listbox.hpp"

#include "dlg_Extras.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"
#include "StateMgr/mapList.hpp"
#include "NetworkMgr/GameMgr.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "Configuration/GameConfig.hpp"
#include "MoviePlayer/MoviePlayer.hpp"



//=========================================================================
//  Main Menu Dialog
//=========================================================================
extern xstring SelectBestClip( const char* pName );

enum controls
{
	IDC_EXTRAS_LISTBOX,
    IDC_EXTRAS_NAV_TEXT,
};


ui_manager::control_tem ExtrasControls[] = 
{
    // Frames.
    { IDC_EXTRAS_LISTBOX,   "IDS_NULL",           "listbox",  50, 60, 220, 232, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_EXTRAS_NAV_TEXT,  "IDS_NULL",           "text",      0,  0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem ExtrasDialog =
{
    "IDS_EXTRAS_MENU_TITLE",
    1, 9,
    sizeof(ExtrasControls)/sizeof(ui_manager::control_tem),
    &ExtrasControls[0],
    0
};

//=========================================================================
//  Defines
//=========================================================================
// Movie Names
#define ID_EXTRA_DEMO_NARC_OR_UT    0
#define ID_EXTRA_VOICES_OF_A51      1
#define ID_EXTRA_RAMIREZ            2
#define ID_EXTRA_CRISPY             3
#define ID_EXTRA_MCCANN             4
#define ID_EXTRA_INTRO              5
#define ID_EXTRA_SLIDE_SHOW_1       6
#define ID_EXTRA_SLIDE_SHOW_2       7
#define ID_EXTRA_SLIDE_SHOW_3       8
#define ID_EXTRA_SLIDE_SHOW_4       9
#define ID_EXTRA_SLIDE_SHOW_5       10
#define ID_EXTRA_SLIDE_SHOW_6       11
#define ID_EXTRA_SLIDE_SHOW_7       12
#define ID_EXTRA_SLIDE_SHOW_8       13
#define ID_EXTRA_SLIDE_SHOW_9       14
#define ID_EXTRA_SLIDE_SHOW_10      15
#define ID_EXTRA_SLIDE_SHOW_11      16
#define ID_EXTRA_SLIDE_SHOW_12      17
#define ID_EXTRA_SLIDE_SHOW_13      18 
#define ID_EXTRA_SLIDE_SHOW_14      19
#define ID_EXTRA_SLIDE_SHOW_15      20
#define ID_EXTRA_SLIDE_SHOW_16      21
#define ID_EXTRA_SLIDE_SHOW_17      22
#define ID_EXTRA_SLIDE_SHOW_18      23
#define ID_EXTRA_SLIDE_SHOW_19      24
#define ID_EXTRA_INFECTION          25 
#define ID_EXTRA_EDGAR              26
#define ID_EXTRA_OUTRO              27

#ifdef TARGET_XBOX
#define ID_DEMO_NARC_OR_UT_STRING   {"PromoUnreal2"}
#else
#define ID_DEMO_NARC_OR_UT_STRING   ("PromoNarc")
#endif
#define ID_VOICES_OF_A51_STRING     {"VoicesOfArea51"}
#ifdef TARGET_XBOX
#define ID_RAMIREZ_STRING           {"Secrets_Ramirez"}
#define ID_CRISPY_STRING            {"Secrets_Crispy"}
#define ID_MCCANN_STRING            {"Secrets_McCan"}
#else
#define ID_RAMIREZ_STRING           {"SecretsRamirez"}
#define ID_CRISPY_STRING            {"SecretsCrispy"}
#define ID_MCCANN_STRING            {"SecretsMcCan"}
#endif
#define ID_INTRO_STRING             {"CinemaIntro"}
#define ID_SLIDE_SHOW_1_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_2_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_3_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_4_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_5_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_6_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_7_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_8_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_9_STRING      {"CinemaIntro"}
#define ID_SLIDE_SHOW_10_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_11_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_12_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_13_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_14_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_15_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_16_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_17_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_18_STRING     {"CinemaIntro"}
#define ID_SLIDE_SHOW_19_STRING     {"CinemaIntro"}
#define ID_INFECTION_STRING         {"CinemaInfection"}
#define ID_EDGAR_STRING             {"CinemaEdgar"}
#define ID_OUTRO_STRING             {"CinemaOutro"}
                                      
                                      
char s_MovieNames[30][32] = 
{    
    ID_DEMO_NARC_OR_UT_STRING,   
    ID_VOICES_OF_A51_STRING  ,   
    ID_RAMIREZ_STRING        ,   
    ID_CRISPY_STRING         ,   
    ID_MCCANN_STRING         ,   
    ID_INTRO_STRING          ,   
    ID_SLIDE_SHOW_1_STRING   ,   
    ID_SLIDE_SHOW_2_STRING   ,   
    ID_SLIDE_SHOW_3_STRING   ,   
    ID_SLIDE_SHOW_4_STRING   ,   
    ID_SLIDE_SHOW_5_STRING   ,   
    ID_SLIDE_SHOW_6_STRING   ,   
    ID_SLIDE_SHOW_7_STRING   ,   
    ID_SLIDE_SHOW_8_STRING   ,   
    ID_SLIDE_SHOW_9_STRING   ,   
    ID_SLIDE_SHOW_10_STRING  ,   
    ID_SLIDE_SHOW_11_STRING  ,   
    ID_SLIDE_SHOW_12_STRING  ,   
    ID_SLIDE_SHOW_13_STRING  ,   
    ID_SLIDE_SHOW_14_STRING  ,   
    ID_SLIDE_SHOW_15_STRING  ,   
    ID_SLIDE_SHOW_16_STRING  ,   
    ID_SLIDE_SHOW_17_STRING  ,  
    ID_SLIDE_SHOW_18_STRING  ,   
    ID_SLIDE_SHOW_19_STRING  ,   
    ID_INFECTION_STRING      ,   
    ID_EDGAR_STRING          ,   
    ID_OUTRO_STRING                 
};

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Registration function
//=========================================================================

void dlg_extras_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "extras", &ExtrasDialog, &dlg_extras_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_extras_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_extras* pDialog = new dlg_extras;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_extras
//=========================================================================

dlg_extras::dlg_extras( void )
{
}

//=========================================================================

dlg_extras::~dlg_extras( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_extras::Create( s32                        UserID,
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

    m_pExtrasList   = (ui_listbox*) FindChildByID( IDC_EXTRAS_LISTBOX  );
    m_pNavText      = (ui_text*)    FindChildByID( IDC_EXTRAS_NAV_TEXT );
    
    GotoControl( (ui_control*)m_pExtrasList );
    m_CurrentControl = IDC_EXTRAS_LISTBOX;

    FillExtrasList();
    m_pExtrasList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pExtrasList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pExtrasList->SetLabelFlags( ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis|ui_font::clip_l_justify );
    m_pExtrasList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pExtrasList->DisableFrame();
    m_pExtrasList->SetExitOnSelect(FALSE);
    m_pExtrasList->SetExitOnBack(TRUE);
    m_CurrHL = 0;

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" ); 
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_extras::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_extras::Render( s32 ox, s32 oy )
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

void dlg_extras::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;
}

//=========================================================================

void dlg_extras::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_extras::OnPadSelect( ui_win* pWin )
{
    g_AudioMgr.Play( "Select_Norm" );
    m_pExtrasList->OnPadActivate( pWin );

    // shut down background movie
    g_StateMgr.DisableBackgoundMovie();
#if defined( TARGET_PC )
    // play the selected movie
    PlaySimpleMovie(  SelectBestClip(s_MovieNames[ m_pExtrasList->GetSelectedItemData(0)]) );
#endif
    // start up the background movie
    g_StateMgr.EnableBackgroundMovie();
}

//=========================================================================

void dlg_extras::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_extras::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // complete!  turn on the elements
            m_pExtrasList ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            GotoControl( (ui_control*)m_pExtrasList );
            g_UiMgr->SetScreenHighlight( m_pExtrasList->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================

class checkpoint_sort_compare : public x_compare_functor<const ui_listbox::item&>
{
public:
    s32 operator()( const ui_listbox::item& A, const ui_listbox::item& B )
    {
        const level_check_points& Left = (const level_check_points&)A.Data[0];
        const level_check_points& Right = (const level_check_points&)B.Data[0];
        s32 LeftIndex   = A.Data[1];
        s32 RightIndex  = B.Data[1];

        // We want lowest map id at the top
        if( Left.MapID < Right.MapID )          return -1;
        if( Left.MapID > Right.MapID )          return 1;

        // If map ids are the same, we want the lowest index
        // at the top.
        if( LeftIndex < RightIndex )            return -1;
        if( LeftIndex > RightIndex )            return 1;
        return 0;
    }
};


//=========================================================================

void dlg_extras::FillExtrasList( void )
{  
//  MapID   LevelName
//  1000    "Welcome to Dreamland"
//  1010    "Deep Underground"    
//  1020    "The Hot Zone"        
//  1030    "The Search"          
//  1040    "They Get Bigger"     
//  1050    "The Last Stand"      
//  1060    "One of Them"      // unlock infection   
//  1070    "Internal Changes"    
//  1075    "Life or Death"       
//  1080    "Dr. Cray"            
//  1090    "Hatching Parasites"  
//  1095    "Project: Blue Book"  
//  1100    "Lies of the Past"    
//  1105    "Buried Secrets"      
//  1110    "Now Boarding"        
//  1115    "The Grays"        // unlock edgar         
//  1120    "Descent"             
//  1125    "The Ascension"       
//  1130    "The Last Exit"    // Unlock outro

    // Clear listbox
    m_pExtrasList->DeleteAllItems();

    xwstring DentString;
    DentString = xwstring("     ");

    player_profile& Profile = g_StateMgr.GetActiveProfile(0);

    //Misc
    m_pExtrasList->AddItem(g_StringTableMgr( "ui", "IDS_EXTRAS_DEMO" )                      , ID_EXTRA_DEMO_NARC_OR_UT ,   0, TRUE);
    m_pExtrasList->AddItem(g_StringTableMgr( "ui", "IDS_EXTRAS_VOICES_OF_A51" )             , ID_EXTRA_VOICES_OF_A51   ,   0, TRUE);
    m_pExtrasList->AddItem(g_StringTableMgr( "ui", "IDS_EXTRAS_RAMIREZ" )                   , ID_EXTRA_RAMIREZ         ,   0, TRUE);
    m_pExtrasList->AddItem(g_StringTableMgr( "ui", "IDS_EXTRAS_CRISPY" )                    , ID_EXTRA_CRISPY          ,   0, TRUE);
    m_pExtrasList->AddItem(g_StringTableMgr( "ui", "IDS_EXTRAS_MCCANN" )                    , ID_EXTRA_MCCANN          ,   0, TRUE);

    // Slide shows
    m_pExtrasList->AddItem(            g_StringTableMgr( "ui", "IDS_EXTRAS_CINEMATICS"    ) , 0                        ,9999, FALSE);
    m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_INTRO"         ) , ID_EXTRA_INTRO           ,1000, FALSE); DentString = xwstring("     ");    
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_1"  ) , ID_EXTRA_SLIDE_SHOW_1    ,1000, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_2"  ) , ID_EXTRA_SLIDE_SHOW_2    ,1010, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_3"  ) , ID_EXTRA_SLIDE_SHOW_3    ,1020, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_4"  ) , ID_EXTRA_SLIDE_SHOW_4    ,1030, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_5"  ) , ID_EXTRA_SLIDE_SHOW_5    ,1040, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_6"  ) , ID_EXTRA_SLIDE_SHOW_6    ,1050, FALSE); DentString = xwstring("     ");
    m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_INFECTION"     ) , ID_EXTRA_INFECTION       ,1060, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_7"  ) , ID_EXTRA_SLIDE_SHOW_7    ,1060, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_8"  ) , ID_EXTRA_SLIDE_SHOW_8    ,1070, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_9"  ) , ID_EXTRA_SLIDE_SHOW_9    ,1075, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_10" ) , ID_EXTRA_SLIDE_SHOW_10   ,1080, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_11" ) , ID_EXTRA_SLIDE_SHOW_11   ,1090, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_12" ) , ID_EXTRA_SLIDE_SHOW_12   ,1095, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_13" ) , ID_EXTRA_SLIDE_SHOW_13   ,1100, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_14" ) , ID_EXTRA_SLIDE_SHOW_14   ,1105, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_15" ) , ID_EXTRA_SLIDE_SHOW_15   ,1110, FALSE); DentString = xwstring("     ");
    m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_EDGAR"         ) , ID_EXTRA_EDGAR           ,1115, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_16" ) , ID_EXTRA_SLIDE_SHOW_16   ,1115, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_17" ) , ID_EXTRA_SLIDE_SHOW_17   ,1120, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_18" ) , ID_EXTRA_SLIDE_SHOW_18   ,1125, FALSE); DentString = xwstring("     ");
    //m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_SLIDE_SHOW_19" ) , ID_EXTRA_SLIDE_SHOW_19   ,1130, FALSE); DentString = xwstring("     ");
    m_pExtrasList->AddItem(DentString+=g_StringTableMgr( "ui", "IDS_EXTRAS_OUTRO"         ) , ID_EXTRA_OUTRO           ,9998, Profile.m_bGameFinished); DentString = xwstring("     ");

    for( s32 i=0; i<g_MapList.GetCount(); i++ )
    {
        const map_entry& Entry = *g_MapList.GetByIndex( i );
        s32 MapID = Entry.GetMapID();

        // check if of the correct game type
        if( ( Entry.GetGameType() == GAME_CAMPAIGN ) && ( MapID < 2000 ) )
        {
            for( s32 j=0; j<MAX_SAVED_LEVELS; j++ )
            {
                level_check_points& Checkpoint = Profile.GetCheckpoint(j);

                // check for a match
                if( Checkpoint.MapID == MapID )
                {
                    for(s32 list = 0 ; list < m_pExtrasList->GetItemCount() ; list++ )
                    {
                        if( m_pExtrasList->GetItemData(list,1) <= MapID && m_pExtrasList->GetItemData(list,1) != 9999 )
                        {
                            // Enable this item.
                            m_pExtrasList->EnableItem(list, TRUE);
                        }
                    }
                }
            }
        }
    }

    m_pExtrasList->SetSelection(0);
}

//=========================================================================
