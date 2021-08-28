//==============================================================================
//
//  hud_Object.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "hudObject.hpp"
#include "e_Draw.hpp"
#include "e_View.hpp"
#include "Entropy.hpp"
#include "x_math.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Objects\WeaponSniper.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Ui\ui_font.hpp"

#ifndef X_EDITOR
#include "StateMgr\StateMgr.hpp"
#endif

//=========================================================================
// GLOBALS
//=========================================================================

// Screen margins given in pixels
#if defined(TARGET_PS2) || defined( TARGET_PC ) || defined( X_EDITOR )
#define LEFTMARGIN              16
#define TOPMARGIN               16
#define RIGHTMARGIN             16
#define BOTTOMMARGIN            16
#endif

#ifdef TARGET_XBOX
extern u32 g_PhysW;
extern u32 g_PhysH;
extern u32 g_LEdge;
extern u32 g_TEdge;
extern u32 g_REdge;
extern u32 g_BEdge;

#define LEFTMARGIN              g_LEdge
#define TOPMARGIN               g_TEdge
#define RIGHTMARGIN             (g_PhysW-g_REdge)
#define BOTTOMMARGIN            (g_PhysH-g_BEdge)
#endif

s32     hud_object::m_PulseAlpha;
f32     hud_object::m_PulseRate;

#if defined(X_DEBUG)
xbool g_RenderFrameRateInfo = TRUE;
#else
xbool g_RenderFrameRateInfo = FALSE;
#endif

xcolor g_HudColor( 80, 150, 150, 255);

extern xbool g_first_person;
extern xbool g_game_running;


//==============================================================================
// OBJECT DESCRIPTION
//==============================================================================

//==============================================================================
static struct hud_object_desc : public object_desc
{
    hud_object_desc( void ) : object_desc( 
        object::TYPE_HUD_OBJECT, 
        "Hud Object", 
        "HUD",

        object::ATTR_DRAW_2D                     |
        object::ATTR_NEEDS_LOGIC_TIME            |
        object::ATTR_RENDERABLE,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC  ) {}         

        //---------------------------------------------------------------------

        virtual object* Create          ( void )
        {
            return new hud_object;
        }

        //---------------------------------------------------------------------

} s_HudObject_Desc;

//==============================================================================

const object_desc&  hud_object::GetTypeDesc( void ) const
{
    return s_HudObject_Desc;
}

//==============================================================================

const object_desc&  hud_object::GetObjectType( void )
{
    return s_HudObject_Desc;
}


//==============================================================================
// FUNCTIONS
//==============================================================================

hud_object::hud_object( void ) 
{
    x_DebugMsg( "Initializing HUD\n" );

    m_LogicRunning  = FALSE;
    m_Initialized   = FALSE;
    m_PulseAlpha    = 255;
    m_PulseRate     = 512.0f;
    m_NumHuds       = 0;

    m_FPSCount15        = 0;
    m_FPSCount20        = 0;
    m_FPSCount30        = 0;
    m_Below30ImageCount = 0;

    m_bLetterBoxOn       = FALSE;
    m_LetterBoxCurrTime  = 1.0f;
    m_LetterBoxTotalTime = 1.0f;

    m_ViewDimensions.Clear();

    // No point in waiting for InitHud to be called, since the array and the
    // components exist regardless of how many players there are.  Also,
    // this makes message creation before everything is completely set up less
    // problematic.
    s32 i; 
    for( i = 0; i < NET_MAX_PER_CLIENT; i++ )
    {
        // Initialize component pointer array.
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_MUTANT_VISION     ]  = &m_PlayerHuds[ i ].m_MutantVision;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_CONTAGIOUS_VISION ]  = &m_PlayerHuds[ i ].m_ContagiousVision;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_HEALTH_BAR        ]  = &m_PlayerHuds[ i ].m_Health; 
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_AMMO_BAR          ]  = &m_PlayerHuds[ i ].m_Ammo;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_RETICLE           ]  = &m_PlayerHuds[ i ].m_Reticle;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_DAMAGE            ]  = &m_PlayerHuds[ i ].m_Damage;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_SNIPER            ]  = &m_PlayerHuds[ i ].m_Sniper;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_ICON              ]  = &m_PlayerHuds[ i ].m_Icon;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_INFO_BOX          ]  = &m_PlayerHuds[ i ].m_InfoBox;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_TEXT_BOX          ]  = &m_PlayerHuds[ i ].m_Text;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_VOTE              ]  = &m_PlayerHuds[ i ].m_Vote;
        m_PlayerHuds[ i ].m_HudComponents[ HUD_ELEMENT_SCANNER           ]  = &m_PlayerHuds[ i ].m_Scanner;
    
        m_PlayerHuds[ i ].m_Active = FALSE;

        // Initialize element pulse.
        for( s32 j = 0; j < HUD_ELEMENT_NUM_ELEMENTS; j++ )
        {
            m_PlayerHuds[ i ].m_HudComponents[ j ]->m_bPulsing = FALSE;
        }    
    }

    // Timer
    m_TimerTriggerGuid = 0;
    m_RenderTimer = FALSE;
    m_TimerActive = FALSE;
    m_TimerTime   = 30.0f;
    m_TimerAdd    = 0.0f;
    m_TimerSub    = 0.0f;
    m_TimerWarning = FALSE;
    m_TimerCritical = FALSE;
    m_TimerCriticalStarted = FALSE;
    m_TimerWarningStarted = FALSE;

    // Objective Strings
    m_ObjectiveTableNameIndex=-1;
    m_ObjectiveTitleStringIndex=-1;
    m_ObjectiveTime = 0.0f;
}

//==============================================================================

hud_object::~hud_object( void ) 
{
}

void hud_object::InitHud( void )
{
#ifdef X_EDITOR
    m_NumHuds = 1;
#else
    m_NumHuds = g_NetworkMgr.GetLocalPlayerCount();
#endif

    ResetFrameRateInfo();

    s32 i = 0;
    slot_id PlayerSlot = g_ObjMgr.GetFirst( object::TYPE_PLAYER );

    // If a player hasn't been created yet, abort.
    if( PlayerSlot == SLOT_NULL )
        return;

    while( PlayerSlot != SLOT_NULL )
    {
        ASSERT( i < m_NumHuds );

        // Get the players view port.
        player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( PlayerSlot );

        if( 
            (pPlayer != NULL)                     && // Is the player valid?
            (pPlayer->GetLocalSlot() != -1) )         // Is the player local?
        {
            player_hud& PlayerHud   = m_PlayerHuds[ i ]; 
            PlayerHud.m_PlayerSlot  = PlayerSlot;
            PlayerHud.m_LocalSlot   = pPlayer->GetLocalSlot();
#ifndef X_EDITOR
            PlayerHud.m_NetSlot     = pPlayer->net_GetSlot();            
#else
            PlayerHud.m_NetSlot     = 0;
#endif
            PlayerHud.m_Active      = TRUE;

            // Find out what portion of the screen the player owns.
            //rect m_ViewDimensions;
            view& rView = pPlayer->GetView();
            rView.GetViewport( m_ViewDimensions );

            // Set Hud dimensions!
            switch( m_NumHuds )
            {
                // One player.
                case 1:
                    PlayerHud.m_XPos    = LEFTMARGIN    + 2.0f;
                    PlayerHud.m_YPos    = TOPMARGIN     + 2.0f;
                    PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  - LEFTMARGIN - RIGHTMARGIN - 4.0f;
                    PlayerHud.m_Height  = m_ViewDimensions.GetHeight() - TOPMARGIN  - BOTTOMMARGIN - 4.0f;
                    break;


                // Two players.
                case 2:
                    switch( PlayerHud.m_LocalSlot )
                    {
                        // Top Player.
                        case 0:
                            PlayerHud.m_XPos    = m_ViewDimensions.Min.X + LEFTMARGIN    + 2.0f;
                            PlayerHud.m_YPos    = m_ViewDimensions.Min.Y + TOPMARGIN     + 2.0f;
                            PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  - LEFTMARGIN - RIGHTMARGIN - 4.0f;
                            PlayerHud.m_Height  = m_ViewDimensions.GetHeight() - TOPMARGIN  - 4.0f;
                            break;

                        // Bottom Player.
                        case 1:
                            PlayerHud.m_XPos    = m_ViewDimensions.Min.X + LEFTMARGIN    + 2.0f;
                            PlayerHud.m_YPos    = m_ViewDimensions.Min.Y + 2.0f;
                            PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  - LEFTMARGIN - RIGHTMARGIN - 4.0f;
                            PlayerHud.m_Height  = m_ViewDimensions.GetHeight() - BOTTOMMARGIN - 4.0f;
                            break;

                        default:
                            break;
                    }
                    break;

                // Three/Four players.
                case 3:
                {
                    switch( PlayerHud.m_LocalSlot )
                    {
                        // Top-left player.
                    case 0:
                        PlayerHud.m_XPos    = m_ViewDimensions.Min.X       + LEFTMARGIN + 2.0f;
                        PlayerHud.m_YPos    = m_ViewDimensions.Min.Y       +  TOPMARGIN + 2.0f;
                        PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  - LEFTMARGIN - 4.0f;
                        PlayerHud.m_Height  = m_ViewDimensions.GetHeight() -  TOPMARGIN - 4.0f;
                        break;

                        // Top-right player.
                    case 1:
                        PlayerHud.m_XPos    = m_ViewDimensions.Min.X                      + 2.0f;
                        PlayerHud.m_YPos    = m_ViewDimensions.Min.Y       +    TOPMARGIN + 2.0f;
                        PlayerHud.m_Width   = m_ViewDimensions.GetWidth () -  RIGHTMARGIN - 4.0f;
                        PlayerHud.m_Height  = m_ViewDimensions.GetHeight() -    TOPMARGIN - 4.0f;
                        break;

                        // Bottom player.
                    case 2:
                        PlayerHud.m_XPos    = m_ViewDimensions.Min.X + LEFTMARGIN    + 2.0f;
                        PlayerHud.m_YPos    = m_ViewDimensions.Min.Y + 2.0f;
                        PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  - LEFTMARGIN - RIGHTMARGIN - 4.0f;
                        PlayerHud.m_Height  = m_ViewDimensions.GetHeight() - BOTTOMMARGIN - 4.0f;
                        break;                        
                    default:
                        break;
                    }
                    break;
                }
                case 4:
                {
                    //f32 X    = m_ViewDimensions.Min.X;
                    //f32 Y    = m_ViewDimensions.Min.Y;
                    //f32 MidX = m_ViewDimensions.Min.X + ((m_ViewDimensions.Min.X + m_ViewDimensions.Max.X)/2.0f);
                    //f32 MidY = m_ViewDimensions.Min.Y + ((m_ViewDimensions.Min.Y + m_ViewDimensions.Max.Y)/2.0f);

                    switch( PlayerHud.m_LocalSlot )
                    {
                        // Top-left player.
                        case 0:
                            PlayerHud.m_XPos    = m_ViewDimensions.Min.X       + LEFTMARGIN + 2.0f;
                            PlayerHud.m_YPos    = m_ViewDimensions.Min.Y       +  TOPMARGIN + 2.0f;
                            PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  - LEFTMARGIN - 4.0f;
                            PlayerHud.m_Height  = m_ViewDimensions.GetHeight() -  TOPMARGIN - 4.0f;
                            break;

                        // Top-right player.
                        case 1:
                            PlayerHud.m_XPos    = m_ViewDimensions.Min.X                      + 2.0f;
                            PlayerHud.m_YPos    = m_ViewDimensions.Min.Y       +    TOPMARGIN + 2.0f;
                            PlayerHud.m_Width   = m_ViewDimensions.GetWidth () -  RIGHTMARGIN - 4.0f;
                            PlayerHud.m_Height  = m_ViewDimensions.GetHeight() -    TOPMARGIN - 4.0f;
                            break;

                        // Bottom-left player.
                        case 2:
                            PlayerHud.m_XPos    = m_ViewDimensions.Min.X       +   LEFTMARGIN + 2.0f;
                            PlayerHud.m_YPos    = m_ViewDimensions.Min.Y                      + 2.0f;
                            PlayerHud.m_Width   = m_ViewDimensions.GetWidth()  -   LEFTMARGIN - 4.0f;
                            PlayerHud.m_Height  = m_ViewDimensions.GetHeight() - BOTTOMMARGIN - 4.0f;
                            break;

                        // Bottom-right player (or not).
                        case 3:
                            PlayerHud.m_XPos    = m_ViewDimensions.Min.X                      + 2.0f;
                            PlayerHud.m_YPos    = m_ViewDimensions.Min.Y                      + 2.0f;
                            PlayerHud.m_Width   = m_ViewDimensions.GetWidth () -  RIGHTMARGIN - 4.0f;
                            PlayerHud.m_Height  = m_ViewDimensions.GetHeight() - BOTTOMMARGIN - 4.0f;
                            break;

                        default:
                            break;
                    }
                    break;
                }

                default:
                    ASSERT( FALSE );
                    break;
            }

            PlayerHud.m_CenterX = ((f32)m_ViewDimensions.Min.X+(m_ViewDimensions.GetWidth())/2.0f);
            PlayerHud.m_CenterY = ((f32)m_ViewDimensions.Min.Y+(m_ViewDimensions.GetHeight())/2.0f);

            // Ok, now lets initialize the hud elements.
            {
                // Centered.
                PlayerHud.m_Reticle.m_XPos  = PlayerHud.m_CenterX;
                PlayerHud.m_Reticle.m_YPos  = PlayerHud.m_CenterY;

                PlayerHud.m_Damage.m_XPos   = PlayerHud.m_CenterX;
                PlayerHud.m_Damage.m_YPos   = PlayerHud.m_CenterY;

                PlayerHud.m_Icon.m_XPos     = PlayerHud.m_CenterX;
                PlayerHud.m_Icon.m_YPos     = PlayerHud.m_CenterY;

                // Top left.
                PlayerHud.m_Text.m_XPos     = PlayerHud.m_XPos;
                PlayerHud.m_Text.m_YPos     = PlayerHud.m_YPos;               

                // Bottom left.
                PlayerHud.m_Health.m_XPos   = PlayerHud.m_XPos;
                PlayerHud.m_Health.m_YPos   = PlayerHud.m_Height + PlayerHud.m_YPos;

                PlayerHud.m_Vote.m_XPos     = 210;
                PlayerHud.m_Vote.m_YPos     = 332;

                // Bottom right.
                PlayerHud.m_Ammo.m_XPos     = PlayerHud.m_XPos + PlayerHud.m_Width;
                PlayerHud.m_Ammo.m_YPos     = PlayerHud.m_YPos + PlayerHud.m_Height;

                PlayerHud.m_Scanner.m_XPos  = PlayerHud.m_XPos + PlayerHud.m_Width;
                PlayerHud.m_Scanner.m_YPos  = PlayerHud.m_YPos + PlayerHud.m_Height;

                // Top right.
                PlayerHud.m_InfoBox.m_XPos  = PlayerHud.m_XPos + PlayerHud.m_Width;
                PlayerHud.m_InfoBox.m_YPos  = PlayerHud.m_YPos;

                // Sniper
                PlayerHud.m_Sniper.m_ViewDimensions = m_ViewDimensions;

                PlayerHud.m_Text.SetMaxWidth( s32(PlayerHud.m_Width) - 4 );
            }


            i++;
        }

        // Get the next player.
        PlayerSlot = g_ObjMgr.GetNext( PlayerSlot ) ;
    }

    m_Initialized = TRUE;
}

//==============================================================================

void hud_object::OnAdvanceLogic( f32 DeltaTime )
{
    // Currently, the HUD needs to initialize before it can do the logic,
    // and it needs to attempt to do the logic before it can initialize.
    // This is due to the orders in which the game and the editor create the
    // objects. It should only take a couple of frames to work itself out.
    m_LogicRunning = TRUE;
    if( !m_Initialized )
    {
        return;
    }

    // Timer
    if( m_TimerWarning && m_TimerWarningStarted == FALSE )
    {
        g_AudioMgr.Play("Objective_Timer_Reset", TRUE );
        m_TimerWarningStarted = TRUE;
    }
    else if( m_TimerWarning == FALSE)
        m_TimerWarningStarted = FALSE;
        
    if( m_TimerCritical && m_TimerCriticalStarted == FALSE )
    {
        g_AudioMgr.Play("Objective_Timer_Reset", TRUE );
        m_TimerCriticalStarted = TRUE;
    }
    else if( m_TimerCritical == FALSE )
        m_TimerCriticalStarted = FALSE;

    if( m_TimerAdd > 0.0f )
    {
        m_TimerTime += m_TimerAdd;
        m_TimerAdd = 0.0f;
    }

    if( m_TimerSub < 0.0f )
    {
        m_TimerTime -= m_TimerSub;
        m_TimerSub = 0.0f;
    }

    if(m_TimerActive)
    {
        if(m_TimerTime<=0.0f)
        {
            // Trigger Guid
            object* pObject = NULL;
            pObject = g_ObjMgr.GetObjectByGuid(m_TimerTriggerGuid);
            if( pObject )
            {                
                pObject->OnActivate(TRUE);               
            }

            // Time is at zero stop the clock.
            m_TimerActive = FALSE;
        }
        m_TimerTime-=DeltaTime;
    }

    // Objective Text update
    if( m_ObjectiveTime > 0.0f )
        m_ObjectiveTime-=DeltaTime;

    // Update pulsing elements.
    m_PulseAlpha += (s32)( m_PulseRate * DeltaTime );

    if( m_PulseAlpha > 255 )
    {
        m_PulseAlpha = 255;
        m_PulseRate = -m_PulseRate;
    }

    if( m_PulseAlpha < 64 )
    {
        m_PulseAlpha = 64;
        m_PulseRate = -m_PulseRate;
    }

    // Do logic for all components.
    s32 i;
    for( i = 0; i < m_NumHuds; i++ )
    {
        m_PlayerHuds[ i ].OnAdvanceLogic( DeltaTime );
    }

    // Update the mutant vision effect manually. It is a special case because it uses
    // a single static effect for all the player huds.
    // We also do the contagious vision at the same time.
    hud_mutant_vision::UpdateEffects( DeltaTime );
    hud_contagious_vision::UpdateEffects( DeltaTime );

    // Do the widescreen bars.
    m_LetterBoxCurrTime = MINMAX(0.0f , m_LetterBoxCurrTime + DeltaTime, m_LetterBoxTotalTime );

    // Note that the timings are exactly 30fps, that's only an estimate, so
    // we'll give a smallish amount of leeway.
#if (defined(TARGET_DEV) || defined(X_QA)) && defined(X_OPTIMIZED) && !defined(CONFIG_PROFILE)
    f32 FPS = eng_GetFPS();
    if ( FPS < 19.0f )      m_FPSCount15++;
    else if ( FPS < 29.0f ) m_FPSCount20++;
    else                    m_FPSCount30++;

    if ( FPS < 29.0f )
        m_Below30ImageCount = 1;
#endif // (TARGET_DEV || X_QA) && X_OPTIMIZED
}

//==============================================================================

xbool hud_object::IsLetterBoxOn( void ) const
{
    // Draw the cinematic bars?
    if( ( ( m_LetterBoxCurrTime > 0.0f ) && m_bLetterBoxOn ) ||
        ( ( m_LetterBoxCurrTime < m_LetterBoxTotalTime ) && !m_bLetterBoxOn ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//==============================================================================

f32 hud_object::GetLetterBoxAmount( void ) const
{
    f32 Amount = m_LetterBoxCurrTime / m_LetterBoxTotalTime;
    if( !m_bLetterBoxOn )
    {
        Amount = 1.0f - Amount;
    }
    
    return Amount;
}

//==============================================================================

void hud_object::RenderLetterBox( const rect& VP, f32 Amount )
{
    // Compute percentage of bar coverage
    f32 Near      = 0.001f;
    f32 BarHeight = (f32)VP.GetHeight() * 0.2f * Amount;

    // Compute top bar
    irect TopRect( (s32)( VP.Min.X - 1.0f ), 
        (s32)( VP.Min.Y - 1.0f ), 
        (s32)( VP.Max.X ), 
        (s32)( VP.Min.Y + BarHeight ) );

    // Compute bottom bar
    irect BottomRect( (s32)( VP.Min.X - 1.0f ), 
        (s32)( VP.Max.Y - BarHeight ),
        (s32)( VP.Max.X ),
        (s32)( VP.Max.Y ) );

    // Render bars
    draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_NO_ZBUFFER);
    
#ifdef X_EDITOR    
    draw_Color( XCOLOR_PURPLE );
#else
    draw_Color( XCOLOR_BLACK );
#endif

    // Top bar.
    draw_Vertex( (f32)TopRect.l, (f32)TopRect.t, Near );
    draw_Vertex( (f32)TopRect.l, (f32)TopRect.b, Near );
    draw_Vertex( (f32)TopRect.r, (f32)TopRect.b, Near );
    draw_Vertex( (f32)TopRect.r, (f32)TopRect.t, Near );

    // Bottom bar.
    draw_Vertex( (f32)BottomRect.l, (f32)BottomRect.t, Near );
    draw_Vertex( (f32)BottomRect.l, (f32)BottomRect.b, Near );
    draw_Vertex( (f32)BottomRect.r, (f32)BottomRect.b, Near );
    draw_Vertex( (f32)BottomRect.r, (f32)BottomRect.t, Near );

    draw_End( );
}

//==============================================================================
xbool g_RenderHUD = TRUE;
void hud_object::OnRender( void )
{
    // If _Debug_ tirgger is set dont render any HUD.   
#if !defined(X_RETAIL) || defined(X_QA)
    if( g_RenderHUD == FALSE )
        return;
#endif

// SB: Do not render if the game isn't running in the editor otherwise the editor
//     will crash when a new player is created and placed.
#ifdef X_EDITOR
    if( !g_game_running )
        return;
#endif

// SB: I commented this out for the editor so that the letter box bars
//     are drawn when you pause the game in the editor.
#ifndef X_EDITOR
    if( m_LogicRunning == FALSE )
        return;
#endif

    // Try to initialize the hud every frame until it works.
    if( !m_Initialized )
    {
        InitHud();

        // Check and see if it worked, if not, come back later.
        if( !m_Initialized )
        {
            return;
        }
    }

#ifdef X_EDITOR
    InitHud();
#endif

    // Draw the cinematic bars?
    if( ( g_first_person ) && ( IsLetterBoxOn() ) )
    {
        RenderLetterBox( m_ViewDimensions, GetLetterBoxAmount() );
        ((hud_renderable*)(GetPlayerHud( g_RenderContext.LocalPlayerIndex ).m_HudComponents[ HUD_ELEMENT_TEXT_BOX ]))->OnRender( SMP_UTIL_GetActivePlayer() );
    }
    else
    {
#ifndef X_EDITOR
        if( g_StateMgr.GetState() != SM_PLAYING_GAME )
            return;
#endif

        GetPlayerHud( g_RenderContext.LocalPlayerIndex ).OnRender( );

        // Timer
        if( m_RenderTimer )
            RenderTimer();
    }

    // Clear any icons that might have accumulated this frame.
    ((hud_icon*)(GetPlayerHud( g_RenderContext.LocalPlayerIndex ).m_HudComponents[ HUD_ELEMENT_ICON ]))->m_NumActiveIcons = 0;

    RenderFrameRateInfo();

#ifdef X_EDITOR
    m_LogicRunning = FALSE;
#endif // X_EDITOR
}
//==============================================================================
static irect TimerRect = irect(0,300,512,448);
void hud_object::RenderTimer( void )
{
    //irect TimerRect;
    xwchar TimeStr[32];

    s32 Seconds1 = ((s32)(m_TimerTime) % 60) / 10;
    s32 Seconds2 = ((s32)(m_TimerTime) % 60) % 10;
    s32 Minutes1 = (((s32)(m_TimerTime)) / 60) / 10;
    s32 Minutes2 = (((s32)(m_TimerTime)) / 60) % 10;

    s32 upper1 = (s32)(m_TimerTime);
    f32 strip = m_TimerTime - (f32)upper1;
    s32 HthsSeconds2 = (s32)((strip /  10)*1000);

    Minutes1 = MAX( Minutes1, 0 );
    Minutes2 = MAX( Minutes2, 0 );
    Seconds1 = MAX( Seconds1, 0 );
    Seconds2 = MAX( Seconds2, 0 );
    HthsSeconds2 = MAX( HthsSeconds2, 0 );

    TimerRect.r = (s32)m_ViewDimensions.GetWidth();
    TimerRect.b = (s32)m_ViewDimensions.GetHeight();

    xcolor TextColor;
    TextColor = XCOLOR_WHITE;
    x_wstrcpy( TimeStr, (const xwchar*)((xwstring)xfs( "%d%d:%d%d:%02d", Minutes1, Minutes2, Seconds1, Seconds2, HthsSeconds2)) );  

#ifndef X_EDITOR
    if( m_TimerWarning )
        TextColor = xcolor( 200,0,0,255);

    if( m_TimerCritical )
    {
        TextColor = xcolor( 200,0,0,255 );
        RenderLine( TimeStr, TimerRect, m_PulseAlpha, TextColor, 2, ui_font::h_center|ui_font::v_center, TRUE );
    }
    else
        RenderLine( TimeStr, TimerRect, 255, TextColor, 2, ui_font::h_center|ui_font::v_center, TRUE );
#else
    x_printfxy( 1, 5, "Timer\n%d%d:%d%d:%02d", Minutes1, Minutes2, Seconds1, Seconds2, HthsSeconds2 );
#endif
}

//==============================================================================
void hud_object::OnEnumProp( prop_enum&  List )
{
    object::OnEnumProp      ( List );

    List.PropEnumHeader( "Hud", "Stats for this item", PROP_TYPE_HEADER );

    // Enum all components.
    s32 j;
    for( j = 0; j < HUD_ELEMENT_NUM_ELEMENTS; j++ )
    {
        m_PlayerHuds[ 0 ].m_HudComponents[ j ]->OnEnumProp( List );
    }


    //----------------------------------------------------------------------
    // Binary text resource.
    //----------------------------------------------------------------------
    List.PropEnumHeader( "Hud\\Binary Text Resource", "The binary text resource loader.", 0 );
    s32 i;
    for( i = 0; i < MAX_BIN_TXT_RSC; i++ )
        List.PropEnumExternal( xfs("Hud\\Binary Text Resource\\Binary Text Rsc [%d]", i ), 
        "Resource\0stringbin\0",
        "Loads the binary text resource which is used "
        "for localizing all of our text resources", 0 );


    //----------------------------------------------------------------------
    // Timer
    //----------------------------------------------------------------------
    List.PropEnumHeader ("Hud\\Timer",                  "Properties for setting up and useing a on screen timer",   0 );
    List.PropEnumGuid   ("Hud\\Timer\\Trigger Guid",    "Guid that is activated when the timer hits Zero.",         PROP_TYPE_EXPOSE|PROP_TYPE_MUST_ENUM);
    List.PropEnumFloat  ("Hud\\Timer\\Time",            "Time in seconds",                                          PROP_TYPE_EXPOSE);
    List.PropEnumBool   ("Hud\\Timer\\Render Timer",    "Render the Timer",                                         PROP_TYPE_EXPOSE);
    List.PropEnumBool   ("Hud\\Timer\\Start Timer",     "true = Start, False = Stop",                               PROP_TYPE_EXPOSE);
    List.PropEnumFloat  ("Hud\\Timer\\Add Time",        "Add Time",                                                 PROP_TYPE_EXPOSE|PROP_TYPE_DONT_SHOW);
    List.PropEnumFloat  ("Hud\\Timer\\Sub Time",        "Sub Time",                                                 PROP_TYPE_EXPOSE|PROP_TYPE_DONT_SHOW);
    List.PropEnumBool   ("Hud\\Timer\\Timer In Warning State", "Set Timer to a Warning State. Turn Timer Text RED",                     PROP_TYPE_EXPOSE);
    List.PropEnumBool   ("Hud\\Timer\\Timer In Critical State","Set Timer to a Warning State. Flash Timer Text and render it RED",      PROP_TYPE_EXPOSE);
}

//==============================================================================

xbool hud_object::OnProperty( prop_query& rPropQuery )
{
    //----------------------------------------------------------------------
    // Base Object.
    //----------------------------------------------------------------------
    if( object::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    // Cycle through all elements.

    s32 i;
    for( i = 0; i < HUD_ELEMENT_NUM_ELEMENTS; i++ )
    {
        if( m_PlayerHuds[ 0 ].m_HudComponents[ i ]->OnProperty( rPropQuery ) )
        {
            return TRUE;
        }
    }

    //----------------------------------------------------------------------
    // Binary text resource.    
    //----------------------------------------------------------------------
    if( rPropQuery.IsVar( "Hud\\Binary Text Resource\\Binary Text Rsc []" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hBinaryTextRsc[ rPropQuery.GetIndex(0) ].GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            const char* pStr = rPropQuery.GetVarExternal();

            if( x_strlen( pStr ) )
            {
                xstring FileName( pStr );

                if( FileName.Find( '.' ) == -1 )
                    FileName += ".bin";

                m_hBinaryTextRsc[ rPropQuery.GetIndex(0) ].SetName( (const char*)FileName );

                // Load the binary text resource.
                if( m_hBinaryTextRsc[ rPropQuery.GetIndex(0) ].IsLoaded() == FALSE )
                    m_hBinaryTextRsc[ rPropQuery.GetIndex(0) ].GetPointer();
            }
        }

        return TRUE;
    }

    //----------------------------------------------------------------------
    // Timer
    //----------------------------------------------------------------------
    if( rPropQuery.VarGUID("Hud\\Timer\\Trigger Guid",      m_TimerTriggerGuid) )
        return TRUE;

    if( rPropQuery.VarFloat("Hud\\Timer\\Time",             m_TimerTime)        )
        return TRUE;

    if( rPropQuery.VarBool("Hud\\Timer\\Render Timer",      m_RenderTimer)      )
        return TRUE;

    if( rPropQuery.VarBool("Hud\\Timer\\Start Timer",       m_TimerActive)      )
        return TRUE;

    if( rPropQuery.VarFloat("Hud\\Timer\\Add Time",         m_TimerAdd )        )
        return TRUE;

    if( rPropQuery.VarFloat("Hud\\Timer\\Sub Time",         m_TimerSub )        )
        return TRUE;

    if( rPropQuery.VarBool("Hud\\Timer\\Timer In Warning State", m_TimerWarning ))
        return TRUE;
    if( rPropQuery.VarBool("Hud\\Timer\\Timer In Critical State", m_TimerCritical ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

void hud_object::GetBinaryResourceName( xstring& String )
{
    for( s32 i = 0; i < MAX_BIN_TXT_RSC; i++ )
    {
        if( x_strlen( m_hBinaryTextRsc[i].GetName() ) )
        {
            xstring tStr( m_hBinaryTextRsc[i].GetName() );

            // Get rid of the the extension.
            s32 DotIndex = tStr.Find('.');
            if( DotIndex != -1 )
                tStr = tStr.Left( DotIndex );

            String += tStr;
            String += '\0';
        }
    }

    String += '\0';
}

//==============================================================================

// Start or stop a HUD element pulsing 
void hud_object::SetElementPulseState( s32 ElementID, xbool DoPulse )
{
    ASSERTS( IN_RANGE( 0, ElementID, HUD_ELEMENT_NUM_ELEMENTS - 1), "Element ID out of range" );

    // This can only be used on the first HUD.
    m_PlayerHuds[ 0 ].m_HudComponents[ ElementID ]->m_bPulsing = DoPulse;
}                   

//==============================================================================

// Just because the editor wants this.
bbox hud_object::GetLocalBBox( void ) const
{
    return bbox( GetPosition(), 50.0f );
}

//==============================================================================

void hud_object::ResetFrameRateInfo( void )
{
    m_FPSCount15        = 0;
    m_FPSCount20        = 0;
    m_FPSCount30        = 0;
    m_Below30ImageCount = 0;
}

//==============================================================================

void hud_object::RenderFrameRateInfo( void )
{
#ifdef jhowa
    g_RenderFrameRateInfo = FALSE;
#endif

    if ( g_RenderFrameRateInfo )
    {
#if (defined(TARGET_DEV) || defined(X_QA)) && defined(X_OPTIMIZED)
        {
            s32 TotalCount = m_FPSCount15+
                m_FPSCount20+
                m_FPSCount30;
#if defined(X_EDITOR)
            {
                x_printfxy( 0, 2, ">=30:%03d%%(%d)", (s32)(100.0f*(f32)m_FPSCount30/(f32)TotalCount), m_FPSCount30 );
                x_printfxy( 0, 3, "  20:%03d%%(%d)", (s32)(100.0f*(f32)m_FPSCount20/(f32)TotalCount), m_FPSCount20 );
                x_printfxy( 0, 4, "<=15:%03d%%(%d)", (s32)(100.0f*(f32)m_FPSCount15/(f32)TotalCount), m_FPSCount15 );
            }
#else
            {
                irect Rect;
                s32 XRes,YRes;
                eng_GetRes(XRes,YRes);

                s32 x = 150;
                s32 y = YRes - 60;
                s32 font = g_UiMgr->FindFont("small");

                Rect.Set( x, y, x + 160, y + (g_UiMgr->GetLineHeight(font) * 3) );
                draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

                xwstring Text1 = (const char *)xfs( ">=30:%03d%%(%d)", (s32)(100.0f*(f32)m_FPSCount30/(f32)TotalCount), m_FPSCount30 );
                g_UiMgr->TextSize( font, Rect, Text1, Text1.GetLength());
                Rect.Translate(x, y);
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text1 );
                y += g_UiMgr->GetLineHeight(font);

                xwstring Text2 = (const char *)xfs( "   20:%03d%%(%d)", (s32)(100.0f*(f32)m_FPSCount20/(f32)TotalCount), m_FPSCount20 );
                g_UiMgr->TextSize( font, Rect, Text2, Text2.GetLength());
                Rect.Translate(x, y);
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text2 );
                y += g_UiMgr->GetLineHeight(font);

                xwstring Text3 = (const char *)xfs( "<=15:%03d%%(%d)", (s32)(100.0f*(f32)m_FPSCount15/(f32)TotalCount), m_FPSCount15 );
                g_UiMgr->TextSize( font, Rect, Text3, Text3.GetLength());
                Rect.Translate(x, y);
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text3 );
            }
#endif //!defined X_EDITOR

            if ( m_Below30ImageCount )
            {
                m_Below30ImageCount--;

                s32 XRes,YRes;
                eng_GetRes(XRes,YRes);

                // draw the bad frame rate image
                draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER );

                rhandle<xbitmap> m_ScreenEdgeBmp;
                m_ScreenEdgeBmp.SetName( "HUD_30fps.xbmp" );

                xbitmap* pBitmap = m_ScreenEdgeBmp.GetPointer();
                if( pBitmap != NULL )
                {
                    draw_SetTexture( *pBitmap );
                    draw_Sprite( vector3((f32)XRes-200.0f,YRes-64.0f,0.0f), vector2((f32)64.0f, (f32)64.0f), xcolor(255,255,255,255) );
                }

                draw_End();
            }

            //eng_End();
        }
#endif
    }
}

//==============================================================================

player_hud& hud_object::GetPlayerHud( s32 LocalSlot )
{
    s32 i = 0;
    for( i = 0; i < m_NumHuds; i++ )
    {
        if( m_PlayerHuds[ i ].m_LocalSlot == LocalSlot )
        {
            return m_PlayerHuds[ i ];
        }
    }

    ASSERTS( FALSE, "No valid hud for player found!" );
    return m_PlayerHuds[ 0 ];
}

//==============================================================================

void    hud_object::SetupLetterBox( xbool On, f32 SlideTime )
{
    // We don't want any nasty division by zero bugs!
    if( SlideTime == 0.0f )
    {
        m_LetterBoxCurrTime  = 1.0f;
        m_LetterBoxTotalTime = 1.0f;
    }

    f32 CurrPercentage = m_LetterBoxCurrTime / m_LetterBoxTotalTime;

    // If its on already, just change the effective speed of sliding.
    if( On == m_bLetterBoxOn )
    {
        m_LetterBoxCurrTime  = SlideTime * CurrPercentage;
        m_LetterBoxTotalTime = SlideTime;
    }

    // Looks like we have to reverse its direction too.
    else 
    {
        CurrPercentage = 1.0f - CurrPercentage;
    }

    m_LetterBoxCurrTime  = SlideTime * CurrPercentage;
    m_LetterBoxTotalTime = SlideTime;
    m_bLetterBoxOn = On;
}

//==============================================================================

void hud_object::SetObjectiveText( s32 TableNameIndex, s32 TitleStringIndex )
{
    m_ObjectiveTableNameIndex = TableNameIndex;
    m_ObjectiveTitleStringIndex = TitleStringIndex;
}

//==============================================================================

void hud_object::RenderObjectiveText( void )
{
    if( m_ObjectiveTime > 0.0f )
        return;

    if( m_ObjectiveTableNameIndex != -1 && m_ObjectiveTitleStringIndex != -1 )
    {
        g_GameTextMgr.DisplayMessage( g_StringMgr.GetString( m_ObjectiveTableNameIndex ), g_StringMgr.GetString( m_ObjectiveTitleStringIndex ) );   
        m_ObjectiveTime = 6.0f;
    }
}

//==============================================================================