//==============================================================================
//
//  hud_Health.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "hud_Health.hpp"
#include "HudObject.hpp"
#include "NetworkMgr\Networkmgr.hpp"
#include "StringMgr\StringMgr.hpp"

#ifndef X_EDITOR
#include "../../Apps/GameApp/Config.hpp"
#endif

//==============================================================================
//  STORAGE
//==============================================================================

rhandle<xbitmap>            hud_health::m_InnerBmp;
rhandle<xbitmap>            hud_health::m_OuterBmp;
rhandle<xbitmap>            hud_health::m_MutationBmp;
s32                         hud_health::m_Instances=0;

#define HEALTH_STUN_TIME    0.30f
#define NUM_CONTROL_POINTS  5
#define HEALTH_FALL_OFF_SIZE    11
#define MUTAGEN_FALL_OFF_SIZE   8

#define NUM_HEALTH_UNITS        19
#define NUM_MUTATION_UNITS      16

xcolor  g_HealthColors[ NUM_CONTROL_POINTS ] = 
    {   xcolor( 100, 255, 148 ),
        xcolor( 100, 255, 148 ),
        xcolor( 240, 150,  40 ),
        xcolor( 230, 40,   40 ),
        xcolor( 255, 40,   40 ) };

xcolor  g_White             ( 255, 207, 207, 255 );
xcolor  g_Blue              ( 127,   0,   0, 255 );
xcolor  g_MutagenLow        ( 120,  99,  50, 255 );
xcolor  g_MutagenHi         ( 244, 198, 101, 255 );

xcolor  g_MutantBarOutline  (  95,  85,  15, 200  );

//static s32    EKG_LENGTH   =  NUM_DOTS;

//==============================================================================

hud_health::hud_health ( void )
{
    m_HudInited = FALSE;
    m_MultiPlayerHud = FALSE;

    //
    // Initialize all bars to default values.
    //
    for( s32 i = 0; i < NUM_BARS; i++ )
    {
        m_Bars[ i ].m_CurrentVal    = 0.0f; 
        m_Bars[ i ].m_TargetVal     = 0.0f; 
        m_Bars[ i ].m_StartVal      = 0.0f;  
        m_Bars[ i ].m_StunTime      = 0.0f;          

        m_Bars[ i ].m_CurrentPhase  = 0.0f;
        m_Bars[ i ].m_PhaseSign     = 1.0f;

        m_Bars[ i ].m_OuterColor    = XCOLOR_PURPLE;
        m_Bars[ i ].m_InnerColor    = XCOLOR_PURPLE;

        m_Bars[ i ].m_MaxVal        = 1.0f;

        m_Bars[ i ].m_CurrentDisplayPercentage = 0.0f;

        m_Bars[ i ].m_FirstPass = TRUE;
    }

    //
    // Initialize the health bar.
    //
    {
        m_Bars[ HEALTH ].m_OuterColor = g_HudColor;
        m_Bars[ HEALTH ].m_bFlip      = FALSE;

        for( s32 c = 0 ; c < NUM_HEALTH_UNITS ; c++ )
        {
            m_Cells[ HEALTH ][c].timeslice = 0;
            m_Cells[ HEALTH ][c].ypos = 0;
            m_Cells[ HEALTH ][c].active = FALSE;
        }
    } 
    
    //
    // Initialize the mutagen bar.
    //
    {
        m_Bars[ MUTAGEN ].m_OuterColor = g_HudColor;
        m_Bars[ MUTAGEN ].m_bFlip      = FALSE;

        for( s32 c = 0 ; c < NUM_MUTATION_UNITS ; c++ )
        {
            m_Cells[ MUTAGEN ][c].timeslice = 0;
            m_Cells[ MUTAGEN ][c].ypos = 0;
            m_Cells[ MUTAGEN ][c].active = FALSE;
        }
    }


    //
    // Initialize the flashlight bar.
    //
    //m_FlashLightBmp.SetName( PRELOAD_FILE("HUD_Campaign_flashlight_on.xbmp") );

    //
    // Volume scanner (EQ) (polish)
    //
#ifdef TARGET_PS2
    extern void EnableAudioLevels( xbool IsEnabled );
    EnableAudioLevels(TRUE);
#endif // TARGET_PS2
    m_VolScanBmp.SetName( PRELOAD_FILE("HUD_Campaign_volume_scan.xbmp") );

    //-- EKG Init
    m_EKGSpeed = 1.0f;
    m_BlipCount = 0;
    m_LineColor = XCOLOR_WHITE;

    m_StaticBmp1.SetName( PRELOAD_FILE("HUD_Campaign_heart_bkd_static1.xbmp") );
    m_StaticBmp2.SetName( PRELOAD_FILE("HUD_Campaign_heart_bkd_static2.xbmp") );
    m_StaticBmp3.SetName( PRELOAD_FILE("HUD_Campaign_heart_bkd_static3.xbmp") );


    for( s32 i = 0 ; i < NUM_DOTS ; i ++ )
    {
        m_EKGArray[i] = vector2((f32)m_XPos,(f32)m_YPos);
    }

    m_BeatArray = 1;
    m_CountAtSwitch = -1.0f;
    m_OldBeatArray = 1;
    m_FlashLightActive = FALSE;
    m_staticFrame = 0;

    // EQ Bars ( left side )
    m_EQBarUpdate = 0.0f;
    m_EQBarPeaks[0] = 0;
    m_EQBarPeaks[1] = 0;
    m_EQBarPeaks[2] = 0;
    m_EQBar1_Value = 0;
    m_EQBar2_Value = 0;
    m_EQBar3_Value = 0;
    m_EQBarPeakUpdate = TRUE;

    // Mutation Take over
    m_bPlayHudMutation    = FALSE; // this will be set from player inventory...
    m_MutationState  = MUTATION_START;
    m_Instances++;
}

//==============================================================================

void hud_health::Init( void )
{
    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
        object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
        hud_object& Hud = hud_object::GetSafeType( *pObj );

        if( Hud.m_Initialized )
        {
            if( Hud.GetNumPlayers() > 2 )
                m_MultiPlayerHud = TRUE;
            else
                m_MultiPlayerHud = FALSE;
        }
    }

    if( m_MultiPlayerHud )
        m_OuterBmp.SetName (PRELOAD_FILE("HUD_Campaign_healthbox_Q.xbmp") );
    else
        m_OuterBmp.SetName (PRELOAD_FILE("HUD_Campaign_healthbox.xbmp") );

    if( m_MultiPlayerHud )
        m_MutationBmp.SetName( PRELOAD_FILE( "HUD_Campaign_mutation_bar_Q.xbmp" ) );
    else
        m_MutationBmp.SetName( PRELOAD_FILE( "HUD_Campaign_mutation_bar.xbmp" ) );

    m_HudInited = TRUE;
}

//==============================================================================
// Evil evil code! The way this is initialized is just abominable. The first time
// it is rendered is when it gets initialized. For some reason, it cannot be initialized
// on construction. Why oh why?
void hud_health::Kill( void )
{
    m_OuterBmp.Destroy();
    m_MutationBmp.Destroy();
}

//==============================================================================

hud_health::~hud_health( void )
{
    ASSERT( m_Instances>0 );
    m_Instances--;
    if( m_Instances==0 )
    {
        Kill();
    }
}

//==============================================================================

inline xcolor Interpolate( xcolor Color1, xcolor Color2, f32 Percentage )
{
    xcolor TmpColor( 
                (u8)(Color2.R * Percentage + Color1.R * (1.0f - Percentage)),
                (u8)(Color2.G * Percentage + Color1.G * (1.0f - Percentage)),
                (u8)(Color2.B * Percentage + Color1.B * (1.0f - Percentage)),
                (u8)(Color2.A * Percentage + Color1.A * (1.0f - Percentage))
                );
    return TmpColor;
}

//==============================================================================

void hud_health::OnRender( player* pPlayer )
{
    // Render Health Bar
    RenderBar( HEALTH );

    // Render Mutatin Bar
    if( pPlayer->GetInventory2().HasItem( INVEN_WEAPON_MUTATION ) || pPlayer->IsMutated() )
    {
        if( m_bPlayHudMutation == TRUE )
        {
            // Run the first time mutation (thing)
            RenderMutationTakeOver( );
        }
        else
        {
            RenderBar( MUTAGEN ); 
        }
    }

    // Render EKG Unit    
    RenderEKG( );
}

//==============================================================================

void hud_health::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    // First pass we need to setup some stuff based on the number
    // of players.
    if( !m_HudInited )
        Init();

    AdvanceBar( pPlayer, DeltaTime, HEALTH );

    if( pPlayer->GetInventory2().HasItem( INVEN_WEAPON_MUTATION ) || pPlayer->IsMutated() )
    {
        if( m_bPlayHudMutation == TRUE )
        {
            UpdateMutationTakeOver(pPlayer,DeltaTime);
        }
        else
            AdvanceBar( pPlayer, DeltaTime, MUTAGEN );
    }

    AdvanceEKG( pPlayer, DeltaTime );

    UpdateEQBar( pPlayer, DeltaTime );
}

//==============================================================================
static u8 EKG_BACK_ALPHA = 255;
void hud_health::RenderEKG( void )
{
    // If this is 3 or 4 player then we dont need EKG
    if( m_MultiPlayerHud )
        return;

    //
    // Draw the background.
    //
    {
        xbitmap* staticBMP = NULL;
        if( m_staticFrame == 0 )
            staticBMP = m_StaticBmp1.GetPointer();
        else if ( m_staticFrame == 1 )
            staticBMP = m_StaticBmp2.GetPointer();
        else if ( m_staticFrame == 2 )
            staticBMP = m_StaticBmp3.GetPointer();

        if( staticBMP )
        {        
            draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
            draw_SetTexture(*staticBMP);
            draw_DisableBilinear();
            static xcolor EKGBackColor = xcolor( 0, 150, 0, EKG_BACK_ALPHA );

            player* pPlayer = SMP_UTIL_GetActivePlayer();
            if( pPlayer->IsMutated() )
                EKGBackColor = xcolor( 150, 0, 0, EKG_BACK_ALPHA);
            else
                EKGBackColor = xcolor( 0, 150, 0, EKG_BACK_ALPHA);

            static f32 STATIC_X = 3;
            static f32 STATIC_Y = -42;
            draw_Sprite( vector3(m_XPos + STATIC_X,m_YPos + STATIC_Y,0), vector2((f32)m_StaticBmp1.GetPointer()->GetWidth(), (f32)m_StaticBmp1.GetPointer()->GetHeight()), EKGBackColor);
            draw_End();
        }
    }

    //
    // Render Line (ekg)
    //
    {
        draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
        s32 W = 1;
        s32 H = 1;
        u8 AlphaStep = (255 / NUM_DOTS);

        s32 Type1 = 0;
        s32 Type2 = 0;
        s32 Type3 = 0;
        f32 Transition = 0;

        // Render all the points (dots)
        for( s32 i = 0; i < NUM_DOTS; i++ )
        {
            static s32 EKG_X_TWEEK = 7;
            static s32 EKG_Y_TWEEK = 9;

            // X Start Pos.
            s32 HeartBeatX = (s32)m_XPos      + EKG_X_TWEEK;
            s32 HeartBeatY = (s32)m_YPos - 10 - EKG_Y_TWEEK;
            
            // Array that tells how the blip looks.
            s32 HeartBeatArrayY[ 3 ][ NUM_DOTS ] = 
            {
                { 0, -1, -3, -1,  2, -1,  3, 5, -2, -6,  1, -10, -6, -3, -2, 6, 3, 0,  4,  2, -3, -4, 0, 2, 3, 0, 0 },
                { 0,  0,  0, -1, -2, -1,  0, 0,  0,  0, -5, -10, -6, -2,  2, 6, 3, 0, -1, -2, -3, -4, 0, 0, 0, 0, 0 },
                { 0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,   0,  0,  0,  0, 0, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 }
            };

            f32 SpotX2 = (f32)HeartBeatX + (((s32)m_BlipCount + i)     % NUM_DOTS);
            f32 SpotX1 = (f32)HeartBeatX + (((s32)m_BlipCount + i + 1) % NUM_DOTS);

            f32 SpotY1 = (f32)HeartBeatY + HeartBeatArrayY[ m_BeatArray ][ ((s32)m_BlipCount + i + 1)    % NUM_DOTS ];
            f32 SpotY2 = (f32)HeartBeatY + HeartBeatArrayY[ m_BeatArray ][ ((s32)m_BlipCount + i)        % NUM_DOTS ];

            if( m_CountAtSwitch > (i + 1) )
            {
                SpotY1 = (f32)HeartBeatY + HeartBeatArrayY[ m_OldBeatArray ][ ((s32)m_BlipCount + i + 1) % NUM_DOTS ];
                SpotY2 = (f32)HeartBeatY + HeartBeatArrayY[ m_OldBeatArray ][ ((s32)m_BlipCount + i)     % NUM_DOTS ];
                Type2++;
            }
            else if( m_CountAtSwitch > i )
            {
                SpotY2 = (f32)HeartBeatY + HeartBeatArrayY[ m_OldBeatArray ][ ((s32)m_BlipCount + i)     % NUM_DOTS ];
                Transition = m_CountAtSwitch - i;
                Type3++;
            }
            else
            {
                Type1++;
            }

            xcolor c = m_LineColor;

            if( i == (NUM_DOTS - 1) )
            {
                draw_Color( xcolor( c.R, c.G, c.B, 255 ) );
            }
            else
            {
                s32 Alpha = x_max( 0, (AlphaStep * i) - 55 );
                draw_Color( xcolor( c.R, c.G, c.B, Alpha ) );
            }

            if( SpotX2 < SpotX1 )
            {
                if( SpotY1 == SpotY2 )
                {
                    draw_Vertex( SpotX1, SpotY1+H, 0.001f ); 
                    draw_Vertex( SpotX1, SpotY1-H, 0.001f ); 
                    draw_Vertex( SpotX2, SpotY2+H, 0.001f ); 
                    draw_Vertex( SpotX2, SpotY2-H, 0.001f ); 
                }
                else if ( SpotY1 > SpotY2 )
                {
                    draw_Vertex( SpotX1 + W, SpotY1 - H, 0.001f ); 
                    draw_Vertex( SpotX1 - W, SpotY1 + H, 0.001f ); 
                    draw_Vertex( SpotX2 - W, SpotY2 + H, 0.001f ); 
                    draw_Vertex( SpotX2 + W, SpotY2 - H, 0.001f ); 
                }
                else // SpotY1 < SpotY2
                {
                    draw_Vertex( SpotX1 - W, SpotY1 - H, 0.001f ); 
                    draw_Vertex( SpotX1 + W, SpotY1 + H, 0.001f ); 
                    draw_Vertex( SpotX2 + W, SpotY2 + H, 0.001f ); 
                    draw_Vertex( SpotX2 - W, SpotY2 - H, 0.001f ); 
                }
            }
        }
        draw_End();
    }
}

//=========================================================================

void hud_health::AdvanceEKG( player* pPlayer, f32 DeltaTime )
{
    if(m_MultiPlayerHud)
        return;

    player& Player = *(player*)pPlayer;

    f32 CurrentHealth = MINMAX( 0.0f, Player.GetHealth() / Player.GetMaxHealth(), 1.0f );

    m_EKGSpeed   = (2.0f - CurrentHealth) * 2.0f; 
    m_LineColor  = Interpolate( XCOLOR_RED, XCOLOR_WHITE, CurrentHealth );

    // Update blip pos using speed.
    m_BlipCount     += ((DeltaTime * m_EKGSpeed) * 10.0f);
    m_CountAtSwitch -= ((DeltaTime * m_EKGSpeed) * 10.0f);

    if( m_BlipCount >= (NUM_DOTS * 5000) )
    {
        m_BlipCount = 0;
    }

    m_CountAtSwitch = x_max( -1.0f, m_CountAtSwitch );

    s32 BeatArray;

    // Mutating, show a flatline.
    if( pPlayer->IsMutated() && !pPlayer->IsMutantVisionOn() )
    {
       BeatArray = 2;
    }

    // Mutated, show mutant ekg.
    else if( pPlayer->IsMutated() )
    {
        BeatArray = 0;
    }

    // Human, show normal ekg.
    else
    {
        BeatArray = 1;
    }

    if( BeatArray != m_BeatArray )
    {
        m_OldBeatArray  = m_BeatArray;
        m_BeatArray     = BeatArray;
        m_CountAtSwitch = NUM_DOTS + fmod( m_BlipCount, 1.0f ); 
    }
}
//==============================================================================
// Helper function for drawing a shifted box ( used for the health bar )
//==============================================================================
void Draw_ShiftCube( f32 X, f32 Y, f32 W, f32 H, f32 S, xcolor tColor, xcolor bColor )
{
    draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
    draw_Color( tColor );
    draw_Vertex( X, Y, 0.001f ); 
    draw_Color( bColor );
    draw_Vertex( X+S, Y+H, 0.001f ); 
    draw_Vertex( X+W+S, Y+H, 0.001f );
    draw_Color( tColor );
    draw_Vertex( X+W, Y, 0.001f ); 
    draw_End();
}

//==============================================================================
void hud_health::RenderBar( bar_type BarNum )
{
    status_bar& Bar = m_Bars[ BarNum ];

    xbitmap* pBitmap = NULL;

    if( BarNum == HEALTH  )
        pBitmap = m_OuterBmp.GetPointer();
    else
        pBitmap = m_MutationBmp.GetPointer();

    //
    // Draw the border.
    //
    if( pBitmap )
    {
        vector2 WH;
        vector3 Pos;
        xcolor DisplayColor;

        if( BarNum == HEALTH )
        {
            static s32 HUD_HEALTH_X = 0;
            static s32 HUD_HEALTH_Y = -44;
            WH( (f32)m_OuterBmp.GetPointer()->GetWidth(), (f32)m_OuterBmp.GetPointer()->GetHeight() );
            Pos.GetX()  = m_XPos + HUD_HEALTH_X;
            Pos.GetY()  = m_YPos + HUD_HEALTH_Y;
            DisplayColor = g_HudColor;
        }
        else
        {
            WH( (f32)m_MutationBmp.GetPointer()->GetWidth(), (f32)m_MutationBmp.GetPointer()->GetHeight() );
            static s32 HUD_MUTANT_X = 0;
            static s32 HUD_MUTANT_Y = -44;
            Pos.GetX()  = m_XPos + HUD_MUTANT_X;
            Pos.GetY()  = m_YPos + HUD_MUTANT_Y;
            DisplayColor = g_HudColor;
        }

        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
        draw_SetTexture( *pBitmap );
        draw_DisableBilinear();
        vector2 TL;
        vector2 BR;
        TL = vector2( 0.0f, 0.0f );
        BR = vector2( 1.0f, 1.0f );   
        draw_SpriteUV( Pos, WH, TL, BR, DisplayColor );
        draw_End(); 
    }

    //
    // FlashLight Indicator
    //
    //if( BarNum == HEALTH && m_FlashLightActive )
    //{
    //    xbitmap* flashLightBMP;
    //    flashLightBMP = m_FlashLightBmp.GetPointer();

    //    draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
    //    draw_SetTexture(*flashLightBMP);
    //    draw_DisableBilinear();

    //    vector2 WH( (f32)m_FlashLightBmp.GetPointer()->GetWidth(), (f32)m_FlashLightBmp.GetPointer()->GetHeight() );
    //    vector3 Pos;
    //    vector2 TL;
    //    vector2 BR;

    //    TL = vector2( 0.0f, 0.0f );
    //    BR = vector2( 1.0f, 1.0f );

    //    static s32 HUD_FL_X = 8;
    //    static s32 HUD_FL_Y = -55;

    //    Pos.GetX() = m_XPos + HUD_FL_X;
    //    Pos.GetY() = m_YPos + HUD_FL_Y;

    //    draw_SpriteUV( Pos, WH, TL, BR, xcolor( 200,200,0,255 ));
    //    draw_End();
    //}

    s32 X;
    s32 Y;
    s32 NUM_BUBS;
    s32 BUB_SPACEING;
    s32 BUB_X;
    s32 BUB_Y;
    s32 BUB_H;
    s32 BUB_W;

    // Mutagen bar ( BUBs )
    static s32 NUM_BUBS_M       = NUM_MUTATION_UNITS;
    static s32 BUB_SPACEING_M   = 4;
    static s32 BUB_X_M          = 46;
#ifdef TARGET_XBOX
    static s32 BUB_Y_M          = -26;
#else
    static s32 BUB_Y_M          = -25;
#endif
    static s32 BUB_H_M          = 8;
#ifdef TARGET_XBOX 
    static s32 BUB_W_M          = 5;
#else
    static s32 BUB_W_M          = 5;
#endif

    // Health bar ( BUBs )
    static s32 NUM_BUBS_H       = NUM_HEALTH_UNITS;
    static s32 BUB_SPACEING_H   = 4;
    static s32 BUB_X_H          = 45;
    static s32 BUB_Y_H          = -15;
    static s32 BUB_H_H          = 11;
#ifdef TARGET_XBOX  
    static s32 BUB_W_H          = 5;
#else
    static s32 BUB_W_H          = 5;
#endif

    // If we are in a Multi Player game then we need to move the health bars to fit the smaller hud.
    if( m_MultiPlayerHud )
    {
        static s32 MULTI_HUD_HEALTH_X = 4;
        BUB_X_M = MULTI_HUD_HEALTH_X;
        BUB_X_H = MULTI_HUD_HEALTH_X;
        BUB_Y_M = -26;
    }
    else
    {
        BUB_X_M = 46;
        BUB_X_H = 45;
#ifdef TARGET_XBOX  
        BUB_Y_M = -26;
#else
        BUB_Y_M = -25;
#endif
    }

    if( BarNum == MUTAGEN )
    {     
        NUM_BUBS        =NUM_BUBS_M;
        BUB_SPACEING    =BUB_SPACEING_M;
        BUB_X           =BUB_X_M;
        BUB_Y           =BUB_Y_M;
        BUB_H           =BUB_H_M;
        BUB_W           =BUB_W_M;
    }
    else
    {
        NUM_BUBS        =NUM_BUBS_H;
        BUB_SPACEING    =BUB_SPACEING_H;
        BUB_X           =BUB_X_H;
        BUB_Y           =BUB_Y_H;
        BUB_H           =BUB_H_H;
        BUB_W           =BUB_W_H;
    }

    X = (s32)m_XPos + BUB_X;
    Y = (s32)m_YPos + BUB_Y;

    static xcolor HUD_HEALTH_HIGH = xcolor(0,150,0,255);
    static xcolor HUD_HEALTH_LOW  = xcolor(0,80,0,255);
    static xcolor HUD_HEALTH_BACK = xcolor(0,80,0,200);
    static xcolor HUD_HEALTH_RED  = xcolor(100,50,0,255 );

    static xcolor HUD_MUTANT_HIGH = xcolor(120,120,40,255);
    static xcolor HUD_MUTANT_LOW  = xcolor(80,80,30,255 );
    static xcolor HUD_MUTANT_BACK = xcolor(80,80,30,200);
    static xcolor HUD_MUTANT_RED  = xcolor(100,50,0,255 );

    f32 BUB_SEGMENT_SIZE = (Bar.m_MaxVal / NUM_BUBS);

    for( s32 bub = 0 ; bub < NUM_BUBS ; bub++ )
    {
        irect bubble;
        bubble.Set( X+(BUB_SPACEING*bub), Y,(X+(BUB_SPACEING*bub))+BUB_W, Y+BUB_H);

        vector4 box;
        box.GetX() = (f32)X+(BUB_SPACEING*bub);
        box.GetY() = (f32)Y;
        box.GetZ() = (f32)BUB_W;
        box.GetW() = (f32)BUB_H;
        
        static f32 SHIFT = 10;

        // Current HEALTH
        if( Bar.m_TargetVal >= (bub*BUB_SEGMENT_SIZE) )
        {
            if(BarNum == HEALTH)
                Draw_ShiftCube( box.GetX(),box.GetY(),box.GetZ(),box.GetW(),SHIFT,HUD_HEALTH_HIGH,HUD_HEALTH_LOW);
            else
                Draw_ShiftCube( box.GetX(),box.GetY(),box.GetZ(),box.GetW(),SHIFT,HUD_MUTANT_HIGH,HUD_MUTANT_LOW);
        } 
        // Render Current.
        else if( m_Cells[ BarNum ][bub].active )
        {
            // render back cell
            if(BarNum == HEALTH)
                Draw_ShiftCube( box.GetX(),box.GetY(),box.GetZ(),box.GetW(),SHIFT,HUD_HEALTH_BACK,HUD_HEALTH_BACK);
            else
                Draw_ShiftCube( box.GetX(),box.GetY(),box.GetZ(),box.GetW(),SHIFT,HUD_MUTANT_BACK,HUD_MUTANT_BACK);

            // render Sub section of cell (red)
            if(BarNum == HEALTH)
                Draw_ShiftCube( box.GetX(),box.GetY()+m_Cells[ BarNum ][bub].ypos,box.GetZ(),box.GetW()-m_Cells[ BarNum ][bub].ypos,SHIFT,HUD_HEALTH_RED,HUD_HEALTH_RED);
            else
                Draw_ShiftCube( box.GetX(),box.GetY()+m_Cells[ BarNum ][bub].ypos,box.GetZ(),box.GetW()-m_Cells[ BarNum ][bub].ypos,SHIFT,HUD_MUTANT_RED,HUD_MUTANT_RED);
        }
        else // Cell is missing.
        {
            if(BarNum == HEALTH)
                Draw_ShiftCube( box.GetX(),box.GetY(),box.GetZ(),box.GetW(),SHIFT,HUD_HEALTH_BACK,HUD_HEALTH_BACK);
            else
                Draw_ShiftCube( box.GetX(),box.GetY(),box.GetZ(),box.GetW(),SHIFT,HUD_MUTANT_BACK,HUD_MUTANT_BACK);
        }
    }
}



//==============================================================================

void hud_health::AdvanceBar( player* pPlayer, f32 DeltaTime, bar_type BarNum )
{
    (void)DeltaTime;

    f32 PlayerValue = 0.0f;

    s32 NumCells = NUM_HEALTH_UNITS;
    if( BarNum == MUTAGEN )
        NumCells = NUM_MUTATION_UNITS;

    m_FlashLightActive = pPlayer->IsFlashlightActive();

    status_bar& Bar = m_Bars[ BarNum ];

    switch( BarNum )
    {
    case HEALTH:
        //-- if this is the first time we have updated the bars then
        if( Bar.m_FirstPass )
        {
            Bar.m_MaxVal = pPlayer->GetMaxHealth();
            Bar.m_TargetVal = pPlayer->GetHealth();       
        }

        Bar.m_MaxVal = pPlayer->GetMaxHealth();
        PlayerValue = pPlayer->GetHealth();   

        m_staticFrame++;
        if( m_staticFrame>2 )
            m_staticFrame=0;
    break;

    case MUTAGEN:

        //-- if this is the first time we have updated the bars then
        if( Bar.m_FirstPass )
        {
            Bar.m_MaxVal = pPlayer->GetMaxMutagen();
            Bar.m_TargetVal = pPlayer->GetMutagen();       
        }

        Bar.m_MaxVal = pPlayer->GetMaxMutagen();
        PlayerValue = pPlayer->GetMutagen();            
    break;

    default:
    break;
    }

    // Health or Mut has been restored. (reset the cells)
    if( PlayerValue > Bar.m_TargetVal)
    {
        f32 CELL_SEGMENT = (Bar.m_MaxVal / NumCells);
        for( s32 cells = 0 ; cells < NumCells ; cells++)
        {
            if(PlayerValue >= cells*CELL_SEGMENT)
            {
                m_Cells[BarNum][cells].active = FALSE;
                m_Cells[BarNum][cells].ypos = 0;
                m_Cells[BarNum][cells].timeslice = 0;
            }
        }
    }

    // This is first pass after restart of a new game.
    // So we need to setup the data for the fall off sections
    // and make sure that the start val is the target.
    if (Bar.m_FirstPass)
    {
        Bar.m_StartVal   = Bar.m_TargetVal;

        f32 CELL_SEGMENT_SIZE = (Bar.m_MaxVal / NumCells);
        for( s32 c = NumCells-1 ; c > 0 ; c-- )
        {
            if( Bar.m_TargetVal <= c*CELL_SEGMENT_SIZE && m_Cells[BarNum][c].active != TRUE )
            {
                m_Cells[BarNum][c].active = TRUE;

                f32 YSize = HEALTH_FALL_OFF_SIZE;
                if( BarNum == MUTAGEN )
                    YSize = MUTAGEN_FALL_OFF_SIZE;

                m_Cells[BarNum][c].ypos = YSize;
                m_Cells[BarNum][c].timeslice = 0.0f;
            }
        }
    }

    // First, look for damage or health increases and set bar stun time.
    if( PlayerValue != Bar.m_TargetVal )
    {
        // We only want to pause the fade if the health wasn't already fading.
        if( Bar.m_CurrentVal <= Bar.m_TargetVal )
            Bar.m_StunTime = HEALTH_STUN_TIME;

        // before we update the Target and start values.. lets see if we will
        // need to be ghosting the bar.. ( loss of life or mutegen )
        xbool ghostBar = FALSE;
        if( PlayerValue < Bar.m_TargetVal )
            ghostBar = TRUE;

        Bar.m_TargetVal  = PlayerValue;
        Bar.m_StartVal   = Bar.m_CurrentVal;

        // we have taken damage or used mutagen.. setup cells to show the loss.
        if( ghostBar )
        {            
            f32 CELL_SEGMENT_SIZE = (Bar.m_MaxVal / NumCells);
            for( s32 c = NumCells-1 ; c > 0 ; c-- )
            {
                if( Bar.m_TargetVal <= c*CELL_SEGMENT_SIZE && m_Cells[BarNum][c].active != TRUE )
                {
                    m_Cells[BarNum][c].active = TRUE;
                    m_Cells[BarNum][c].ypos = 0;
                    m_Cells[BarNum][c].timeslice = 0.125f * (NumCells - c);
                }
            }
        }
    }

    Bar.m_StunTime -= DeltaTime;

    // Now, advance towards the target value.
    if( (Bar.m_StunTime < 0.0f) && (Bar.m_CurrentVal > Bar.m_TargetVal) )
    {
        for(s32 c = 0 ; c < NumCells ; c++)
        {
            if( m_Cells[BarNum][c].active)
            {
                m_Cells[BarNum][c].timeslice -= DeltaTime;
            }
            if( m_Cells[BarNum][c].timeslice < 0.0f )
            {
                m_Cells[BarNum][c].ypos += 1.25f;

                f32 YSize = HEALTH_FALL_OFF_SIZE;
                if( BarNum == MUTAGEN )
                    YSize = MUTAGEN_FALL_OFF_SIZE;
                    
                if( m_Cells[BarNum][c].ypos > YSize )
                {
                    m_Cells[BarNum][c].ypos = YSize;
                }
            }
        }
    }

    Bar.m_CurrentVal = x_max( Bar.m_CurrentVal, Bar.m_TargetVal );
    Bar.m_StartVal   = x_max( Bar.m_CurrentVal, Bar.m_StartVal  ); 
    Bar.m_FirstPass = FALSE;
}

//==============================================================================
void hud_health::RenderEQBar( s32 EQBar, s32 AmountPercent )
{
    irect R;
    static s32 HUD_VS_X = 7;
    static s32 HUD_VS_Y = -69;
    static s32 HUD_VS_W = 8;
#ifdef TARGET_XBOX
    static s32 HUD_VS_H =  80;
#else
    static s32 HUD_VS_H = 100;
#endif
    s32 X = (s32)m_XPos + HUD_VS_X;
    s32 Y = (s32)m_YPos + HUD_VS_Y;

    s32 WorkingX = (s32)(X + ( EQBar * 10 ));
    s32 BarLength = (HUD_VS_H - x_min( AmountPercent,HUD_VS_H ));

    // Render Bar
    R.Set(
        WorkingX, 
        x_max( 0,Y ),
        WorkingX+HUD_VS_W, 
        x_max( 0,Y-(HUD_VS_H-(BarLength)))
    );

    static xcolor EQColor = xcolor(80, 150, 150, 30);
    static xcolor EQColorHigh = xcolor(80, 150, 150, 50);
    draw_GouraudRect(R, EQColorHigh, EQColorHigh, EQColor, EQColor, FALSE );

    // Has the bar went up? if so update the peak
    if( Y-(HUD_VS_H-(BarLength)) < m_EQBarPeaks[EQBar] )
        m_EQBarPeaks[EQBar] = (s32)(Y-(HUD_VS_H-(BarLength)));

    // Are we in a Update Peak mode?
    if( m_EQBarPeakUpdate ) // do we need to update?
    {
        m_EQBarPeaks[EQBar] = (s32)(Y-(HUD_VS_H-(BarLength)));
        if( EQBar == 2 ) // EEK UGLY
            m_EQBarPeakUpdate = FALSE;
    }

     // Render Peak
    R.Set(
        WorkingX,
        x_max( 0,m_EQBarPeaks[EQBar] ),
        WorkingX+HUD_VS_W,
        x_max( 0,m_EQBarPeaks[EQBar]-2 )
    );

    draw_Rect(R, g_HudColor, FALSE);
}

//==============================================================================
void hud_health::UpdateEQBar( player* pPlayer, f32 DeltaTime )
{
    m_EQBarUpdate -= DeltaTime;

    xcolor XC = pPlayer->GetFloorProperties().GetColor();

    // Light around player
    vector3 V;
    V.GetX() = (f32)(XC.R)/255.0f;
    V.GetY() = (f32)(XC.G)/255.0f;
    V.GetZ() = (f32)(XC.B)/255.0f;
    f32 I = V.Length()*100.0f;
    f32 Rand;
    Rand = x_frand( -3.0f, 3.0f );
    m_EQBar1_Value = MIN( (s32)(I+Rand), 100);

    // Sound player is hearing
    Rand = x_frand( -3.0f, 3.0f );
    extern s32 GetAudioLevel( void );
    s32 AudioLevel = (GetAudioLevel() / 256);
    m_EQBar2_Value = MIN( (s32)(fabs(AudioLevel+Rand)), 100);

    // Movement.. not good..
    Rand = x_frand( -3.0f, 3.0f );
    s32 v = m_EQBar1_Value + m_EQBar2_Value;
    s32 adv = v / 2;
    m_EQBar3_Value = MIN( (s32)fabs(adv+Rand), 100 );

    if( m_EQBarUpdate <= 0.0f )
    {
        m_EQBarUpdate = 3.2f;
        m_EQBarPeakUpdate = TRUE;
    }
}

//==============================================================================
void hud_health::RenderVolScanner( void )
{
    xbitmap* volScannerBMP = NULL;
    volScannerBMP = m_VolScanBmp.GetPointer();

    if( volScannerBMP )
    {        
        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
        draw_SetTexture(*volScannerBMP);
        draw_DisableBilinear();
        static xcolor HUD_VOL_SCAN_COLOR = xcolor( 80,150,150,180 );
        static f32 HUD_VOL_SCAN_X = -8;
        static f32 HUD_VOL_SCAN_Y = -176;
        draw_Sprite( vector3(m_XPos + HUD_VOL_SCAN_X,m_YPos + HUD_VOL_SCAN_Y,0), vector2((f32)m_VolScanBmp.GetPointer()->GetWidth(), (f32)m_VolScanBmp.GetPointer()->GetHeight()), HUD_VOL_SCAN_COLOR);
        draw_End();

        RenderEQBar( 0, m_EQBar1_Value );
        RenderEQBar( 1, m_EQBar2_Value );
        RenderEQBar( 2, m_EQBar3_Value );
    }
}

//==============================================================================

void hud_health::RenderMutationTakeOver( void )
{
#ifndef X_EDITOR
    switch( m_MutationState )
    {
    case MUTATION_START:

        break;
    
    case MUTATION_SUIT_BREACH:
        {
            irect box;
            box.Set(0,180,512,448);
            RenderLine( g_StringTableMgr("ui", "IDS_MUTATION_SUIT_BREACH"), box, 255, g_HudColor, 1, ui_font::h_center|ui_font::v_top, FALSE );
        }
        break;
    case MUTATION_MUTATION_DETECTED_1:
        {
            irect box;
            box.Set(0,180,512,448);
            RenderLine( g_StringTableMgr("ui", "IDS_MUTATION_MUTATION_DETECTED_1"), box, 255, g_HudColor, 1, ui_font::h_center|ui_font::v_top, FALSE );
        }
        break;
    case MUTATION_MUTATION_DETECTED_2:        
        {
            irect box;
            box.Set(0,180,512,448);
            RenderLine( g_StringTableMgr("ui", "IDS_MUTATION_MUTATION_DETECTED_2"), box, 255, g_HudColor, 1, ui_font::h_center|ui_font::v_top, FALSE );
        }
        break;
    case MUTATION_MUTATION_PROTOCOL_INIT:
        {
            irect box;
            box.Set(0,180,512,448);
            RenderLine( g_StringTableMgr("ui", "IDS_MUTATION_MUTATION_PROTOCOL_INIT"), box, 255, g_HudColor, 1, ui_font::h_center|ui_font::v_top, FALSE );
        }
        break;
    case MUTATION_MUTATION_SCREEN_FX:
        break;
    case MUTATION_HEALTH_RAMP:        
        break;

    case MUTATION_FADE_BAR_IN:
        {    
            xbitmap* pBitmap = m_MutationBmp.GetPointer();
            if( pBitmap )
            {
                vector2 WH;
                vector3 Pos;
                xcolor DisplayColor;
                WH( (f32)m_MutationBmp.GetPointer()->GetWidth(), (f32)m_MutationBmp.GetPointer()->GetHeight() );
                Pos.GetX()  = m_XPos + 0; //HUD_MUTANT_X;
                Pos.GetY()  = m_YPos + -44;//HUD_MUTANT_Y;
                DisplayColor = g_HudColor;// HUD_MUTANT_BAR_COLOR;

                DisplayColor.A = m_MutationBarAlpha;

                draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
                draw_SetTexture( *pBitmap );
                draw_DisableBilinear();
                vector2 TL;
                vector2 BR;
                TL = vector2( 0.0f, 0.0f );
                BR = vector2( 1.0f, 1.0f );   
                draw_SpriteUV( Pos, WH, TL, BR, DisplayColor );
                draw_End();
            }
        }
        break;

    case MUTATION_BAR_FILL:       
         RenderBar( MUTAGEN );
        break;

    case MUTATION_LIFESIGN_STABILIZED:
        {
            irect box;
            box.Set(0,180,512,448);
            RenderLine( g_StringTableMgr("ui", "IDS_MUTATION_LIFESIGN_STABILIZED"), box, 255, g_HudColor, 1, ui_font::h_center|ui_font::v_top, FALSE );
            RenderBar( MUTAGEN );
        }
        break;

    case MUTATION_END:
        RenderBar( MUTAGEN );
        break;

    default:
        break;
    }
#endif // X_EDITOR
}

//==============================================================================

void hud_health::UpdateMutationTakeOver( player* pPlayer, f32 DeltaTime )
{
    (void)pPlayer;
    (void)DeltaTime;

    switch( m_MutationState )
    {
    case MUTATION_START:
        // init all variables
        m_MutationBarAlpha = 0;
        m_MutationFadeInStep = 8;
        m_MutationFadeResetChance = 40;
        m_MutationDisplayTime = 0.0f;
        m_MutationState++;
        break;

    case MUTATION_SUIT_BREACH:
        m_MutationDisplayTime += DeltaTime;

        if(m_MutationDisplayTime > 2.0f)
        {
            m_MutationState++;
            m_MutationDisplayTime = 0.0f;
        }
        break;
    case MUTATION_MUTATION_DETECTED_1:
        m_MutationDisplayTime += DeltaTime;

        if(m_MutationDisplayTime > 2.0f)
        {
            m_MutationState++;
            m_MutationDisplayTime = 0.0f;
        }
        break;
    case MUTATION_MUTATION_DETECTED_2:
        m_MutationDisplayTime += DeltaTime;

        if(m_MutationDisplayTime > 2.0f)
        {
            m_MutationState++;
            m_MutationDisplayTime = 0.0f;
        }
        break;

    case MUTATION_MUTATION_PROTOCOL_INIT:
        m_MutationDisplayTime += DeltaTime;

        if(m_MutationDisplayTime > 2.0f)
        {
            m_MutationState++;
            m_MutationDisplayTime = 0.0f;
        }
        break;
    case MUTATION_MUTATION_SCREEN_FX:
        //pPlayer->ForceMutationChange( TRUE );
        m_MutationState++;
        break;
    case MUTATION_HEALTH_RAMP:
        m_MutationState++;
        break;

    case MUTATION_FADE_BAR_IN:
        m_MutationBarAlpha += m_MutationFadeInStep;
        m_MutationFadeResetChance += 1;

        if( x_rand()%100 > m_MutationFadeResetChance )
            m_MutationBarAlpha -= 32;

        m_MutationBarAlpha = MAX(0, m_MutationBarAlpha);
        m_MutationBarAlpha = MIN(255,m_MutationBarAlpha);

        if( m_MutationBarAlpha >= 255 )
        {
            pPlayer->SetMutagen(10.0f);
            m_Bars[MUTAGEN].m_FirstPass = TRUE;
            m_MutationState++;
        }
        break;

    case MUTATION_BAR_FILL:
        AdvanceBar(pPlayer,DeltaTime, MUTAGEN );
        pPlayer->SetMutagen(pPlayer->GetMutagen()+1.0f);
        
        if( pPlayer->GetMutagen() >= pPlayer->GetMaxMutagen() )
        {
            pPlayer->SetMutagen( pPlayer->GetMaxMutagen() );
            m_MutationState++;
        }
        break;
    case MUTATION_LIFESIGN_STABILIZED:
        m_MutationDisplayTime += DeltaTime;

        if(m_MutationDisplayTime > 2.0f)
        {
            m_MutationState++;
            m_MutationDisplayTime = 0.0f;
        }
        break;

    case MUTATION_END:
        m_bPlayHudMutation = FALSE;
        m_MutationState = MUTATION_START;
        break;

    default:
        break;
    }
}

//==============================================================================

xbool hud_health::OnProperty( prop_query& rPropQuery )
{   
    (void)rPropQuery;

    if( rPropQuery.VarBool( "Hud\\Mutation\\Play Hud Mutation", m_bPlayHudMutation ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

void hud_health::OnEnumProp( prop_enum&  List )
{
    (void)List;
    List.PropEnumHeader( "Hud\\Mutation", "Damage state variables", 0 );
    List.PropEnumBool( "Hud\\Mutation\\Play Hud Mutation",  "Set to True to trigger the first time mutation HUD.", PROP_TYPE_EXPOSE|PROP_TYPE_MUST_ENUM  );

}

//==============================================================================
