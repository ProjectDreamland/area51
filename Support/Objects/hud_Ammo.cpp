//==============================================================================
//
//  hud_Ammo.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_Ammo.hpp"
#include "HudObject.hpp"
#include "GameLib\RenderContext.hpp"
#include "ui\ui_font.hpp"

//==============================================================================
// STORAGE
//==============================================================================

xcolor  s_AmmoColor         (  0, 255, 255, 255 );

extern xcolor  s_ShadowColor;//       ( 49,  49,  49, 255 );
extern s32     s_ShadowOffsetX;//     = 2;
extern s32     s_ShadowOffsetY;//     = 2;

xcolor                      hud_ammo::m_FragGrenadeColor;
xcolor                      hud_ammo::m_GravGrenadeColor;

static s32 LMAG = 64;
static s32 TMAG = 22;
static s32 LRES = 27;
static s32 TRES = 19;

//==============================================================================
// FUNCTIONS
//==============================================================================
hud_ammo::hud_ammo( void )
{
    m_AmmoHudInited             = FALSE;
    m_CurrentWeaponAmmo         = 0;
    m_GrenadeAmmoJBean          = 0;
    m_GrenadeAmmoFrag           = 0;        

    m_MultiPlayerHud            = FALSE;


    for( s32 a = 0 ; a < NUM_SHADOW_AMMO_RENDERS ; a ++ )
        m_WeaponAmmoQAmounts[a] = 0;

    // initialize ammo hud color
    m_AmmoHudColor              = g_HudColor;

    m_WarningClip = FALSE;
    m_WarningResv = FALSE;
    m_WarningNads = FALSE;
    m_CriticalClip = FALSE;
    m_CriticalResv = FALSE;
    m_CriticalNads = FALSE;
    m_WarningClipALPHA = 0;
    m_WarningResvALPHA = 0;
    m_WarningNadsALPHA = 0;
    m_CriticalSoundID = -1;
}

//==============================================================================

void hud_ammo::Init( void )
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

    // Load Art
    if( m_MultiPlayerHud )
    {
        m_AmmoBoxBMP.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_Q.xbmp"));     // frame
        m_AmmoBoxBMP_Clip.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_000_Q.xbmp"));
        m_AmmoBoxBMP_Resv.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_002_Q.xbmp"));
        m_AmmoBoxBMP_Nads.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_001_Q.xbmp"));

        // initialize nade icons
        m_FragAmmoIcon.SetName  (PRELOAD_FILE("HUD_Campaign_grenade_frag.xbmp"));    
        m_JBeanAmmoIcon.SetName (PRELOAD_FILE("HUD_Campaign_grenade_jellybean.xbmp"));
        m_JBeanXAmmoIcon.SetName(PRELOAD_FILE("HUD_Campaign_grenade_jellybeanx.xbmp"));
    }
    else
    {    
        m_AmmoBoxBMP.SetName(PRELOAD_FILE("HUD_Campaign_ammobox.xbmp"));     // frame
        m_AmmoBoxBMP_Clip.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_000.xbmp"));
        m_AmmoBoxBMP_Resv.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_002.xbmp"));
        m_AmmoBoxBMP_Nads.SetName(PRELOAD_FILE("HUD_Campaign_ammobox_001.xbmp"));

        // initialize nade icons
        m_FragAmmoIcon.SetName  (PRELOAD_FILE("HUD_Campaign_grenade_frag.xbmp"));    
        m_JBeanAmmoIcon.SetName (PRELOAD_FILE("HUD_Campaign_grenade_jellybean.xbmp"));
        m_JBeanXAmmoIcon.SetName(PRELOAD_FILE("HUD_Campaign_grenade_jellybeanx.xbmp"));
    }

    // Set the offsets for the ammo text to appear.
    if( m_MultiPlayerHud )
    {    
        m_LeftMag       = LMAG; 
        m_TopMag        = TMAG;                          
        m_LeftReserve   = LRES; 
        m_TopReserve    = TRES;
        m_LeftNade      = 0;
        m_TopNade       = 0;
    }
    else
    {
        m_LeftMag       = 81; 
        m_TopMag        = 24;
        m_LeftReserve   = 32; 
        m_TopReserve    = 19;
        m_LeftNade      = 71;
        m_TopNade       = 18;
    }

    m_AmmoHudInited = TRUE;
}

//==============================================================================

void hud_ammo::OnRender( player* pPlayer )
{
    // Don't render any ammo stuff if the player is mutated.
    if( 
        pPlayer->GetCurrentWeapon2() == INVEN_WEAPON_MUTATION ||
        pPlayer->GetCurrentWeapon2() == INVEN_WEAPON_SCANNER || 
        pPlayer->GetCurrentWeapon2() == INVEN_WEAPON_TRA
      )
    {
        // Stop all warning and Critical sounds.
        if (m_CriticalSoundID != -1)
        {
            g_AudioMgr.Release(m_CriticalSoundID,0.0f);
            m_CriticalSoundID = -1;
        }

        return;
    }

    RenderAmmoBar       ( pPlayer );
    RenderNades         ( pPlayer, pPlayer->GetCurrentGrenadeType2() );
    RenderAmmoString    ( pPlayer );
}

//==============================================================================

void hud_ammo::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)DeltaTime;
    (void)pPlayer;

    // First pass we need to setup some stuff based on the number
    // of players.
    if( !m_AmmoHudInited )
        Init();
}

//==============================================================================

xbool hud_ammo::OnProperty( prop_query& rPropQuery )
{
    (void)rPropQuery;
    return FALSE;
}

//==============================================================================

void hud_ammo::OnEnumProp( prop_enum&  List )
{
    (void)List;
}

//==============================================================================
static s32 HUD_AMMO_X = -134;
static s32 HUD_AMMO_Y = -43;
static s32 HUD_AMMO_X_G = -40;
static s32 HUD_AMMO_Y_G = -39;
static s32 HUD_AMMO_X_G_Q = -36;
static s32 HUD_AMMO_Y_G_Q = -39;
void hud_ammo::RenderNades( player* pPlayer, inven_item NadType )
{
    (void)pPlayer;
    (void)NadType;

    xbitmap*    pGrnBitmap = NULL;
    s32         GrnCount    = 0;
    xcolor      GrnColor;

    static xcolor FRAG_GRENADE_COLOR = g_HudColor;
    
    switch( NadType )
    {
        case INVEN_GRENADE_FRAG:
            pGrnBitmap  = m_FragAmmoIcon.GetPointer();
        break;
        case INVEN_GRENADE_JBEAN:
            pGrnBitmap  = m_JBeanAmmoIcon.GetPointer();
        break;
        case INVEN_GRENADE_JBEAN_ENHANCE:
            pGrnBitmap  = m_JBeanXAmmoIcon.GetPointer();
        break;

        default:
            ASSERT(0);
        break;
    }

    GrnCount    = (s32)pPlayer->GetInventory2().GetAmount( NadType );
    GrnColor    = XCOLOR_WHITE;

    if( pGrnBitmap == NULL )
        return;

    // BEGIN DRAW
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER|DRAW_UV_CLAMP );
    draw_SetTexture( *pGrnBitmap );
    vector2 WH( (f32)pGrnBitmap->GetWidth(), (f32)pGrnBitmap->GetHeight() );
    vector3 Pos(0,0,0.01f);
    if( m_MultiPlayerHud )
    {
        Pos.GetX() = m_XPos + HUD_AMMO_X_G_Q;
        Pos.GetY() = m_YPos + HUD_AMMO_Y_G_Q;
    }
    else
    {    
        Pos.GetX() = m_XPos + HUD_AMMO_X_G;
        Pos.GetY() = m_YPos + HUD_AMMO_Y_G;
    }
    draw_Sprite( Pos, WH, GrnColor );
    draw_End();
    // END DRAW

    static xstring GrnCountString;
    GrnCountString = (const char *)xfs("%d", GrnCount);
    irect IRECT;
    s32 SomeR = -12;
    s32 SomeB = -22;

    if( m_MultiPlayerHud )
        SomeR = -9;

    IRECT.Set( (s32)m_XPos, (s32)m_YPos, SomeR+(s32)m_XPos, SomeB+(s32)m_YPos );
    xcolor white = XCOLOR_WHITE;
    RenderLine( (const xwchar*)(xwstring((const char*)GrnCountString)), IRECT, 255, white, 1, ui_font::h_right|ui_font::v_bottom, FALSE );
}

//==============================================================================
static xcolor AMMO_INSIDE_COLOR = xcolor(0,31,0,204);
void hud_ammo::RenderAmmoBar( player* pPlayer )
{
    (void)pPlayer;
    static xcolor AMMO_FRAME_COLOR = g_HudColor;

    xbitmap* pBitmap = m_AmmoBoxBMP.GetPointer();
    if( pBitmap == NULL )
        return;

    // BEGIN DRAW
    draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );

    s32 BitmapWidth  = m_AmmoBoxBMP.GetPointer()->GetWidth();
    s32 BitmapHeight = m_AmmoBoxBMP.GetPointer()->GetHeight();

    // Draw the ammo bar.
    vector2 WH( (f32)(BitmapWidth), (f32)(BitmapHeight) );

    m_Dimensions = WH;
    vector3 Pos;

    Pos.GetX() = m_XPos+HUD_AMMO_X; 
    Pos.GetY() = m_YPos+HUD_AMMO_Y; 
   
    draw_DisableBilinear();

    // Clip Box
    draw_SetTexture( *m_AmmoBoxBMP_Clip.GetPointer() );
    draw_Sprite( Pos, WH, AMMO_INSIDE_COLOR);
    if( m_WarningClip )
    {
        m_WarningClipALPHA += 16;
        m_WarningClipALPHA = MIN(m_WarningClipALPHA,100);
        xcolor warning = xcolor( 200,50,0,m_WarningClipALPHA);
        if( m_CriticalClip )
        {
            warning.A = hud_object::m_PulseAlpha;
            warning.A = MIN( hud_object::m_PulseAlpha, 100 );
            draw_Sprite( Pos, WH, warning );
        }
        else
            draw_Sprite( Pos, WH, warning );
    }    
    else
    {
        m_WarningClipALPHA -= 16;
        m_WarningClipALPHA = MAX(m_WarningClipALPHA,0);
        xcolor warning = xcolor( 200,50,0,m_WarningClipALPHA);
        draw_Sprite( Pos, WH, warning );
    }

    // Box 2
    draw_SetTexture( *m_AmmoBoxBMP_Resv.GetPointer() );
    draw_Sprite( Pos, WH, AMMO_INSIDE_COLOR );
    if( m_WarningResv )
    {
        m_WarningResvALPHA += 16;
        m_WarningResvALPHA = MIN(m_WarningResvALPHA,100);
        xcolor warning = xcolor( 200,50,0,m_WarningResvALPHA);
        if( m_CriticalResv )
        {
            warning.A = hud_object::m_PulseAlpha;
            warning.A = MIN( hud_object::m_PulseAlpha, 100 );
            draw_Sprite( Pos, WH, warning );
        }
        else
            draw_Sprite( Pos, WH, warning );
    }    
    else
    {
        m_WarningResvALPHA -= 16;
        m_WarningResvALPHA = MAX(m_WarningResvALPHA,0);
        xcolor warning = xcolor( 200,50,0,m_WarningResvALPHA);
        draw_Sprite( Pos, WH, warning );
    }

    // Box 3
    draw_SetTexture( *m_AmmoBoxBMP_Nads.GetPointer() );
    if( m_WarningNads )
        draw_Sprite( Pos, WH, xcolor(180,50,0,100) );
    else
        draw_Sprite( Pos, WH, AMMO_INSIDE_COLOR );


    // Frame
    draw_SetTexture( *m_AmmoBoxBMP.GetPointer() );
    draw_Sprite( Pos, WH, AMMO_FRAME_COLOR );

    draw_EnableBilinear();
    draw_End();

    // END DRAW
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

void hud_ammo::RenderAmmoString( player* pPlayer )
{
    new_weapon* pWeapon = pPlayer->GetCurrentWeaponPtr();
    if( pWeapon == NULL  )
        return;

    s32 AmmoMax;
    s32 ClipMax;
    s32 AmmoResv;
    s32 AmmoClip;
    pWeapon->GetAmmoState(new_weapon::AMMO_PRIMARY,m_CurrentWeaponAmmo,AmmoMax,ClipMax,m_WeaponAmmoQAmounts[0]);

    AmmoResv = m_CurrentWeaponAmmo - m_WeaponAmmoQAmounts[0];
    AmmoClip = m_WeaponAmmoQAmounts[0];

    xbool WarningsOverRide = FALSE;

    if( AmmoMax == ClipMax ) 
        WarningsOverRide = TRUE;

    // If this is the BBG then never render Red in only has 1 shot.
    if( pPlayer->GetCurrentWeapon2() == INVEN_WEAPON_MESON_CANNON )
        WarningsOverRide = TRUE;

    // check clip warning and critical range.
    f32 clip = (f32)ClipMax;
    f32 resvMax = (f32)AmmoMax;

    s32 percent     = (s32)(0.25f * clip);
    s32 resvPercent = (s32)(0.25f * resvMax);

    percent = MAX(4,percent);

    if( AmmoClip < percent && m_WarningClip == FALSE )
    {
        g_AudioMgr.Play("Ammo_Low",TRUE);
        m_WarningClip = TRUE;
        m_WarningClipALPHA = 0;
    }
    else if( AmmoClip >= percent )
    {
        m_WarningClip = FALSE;     
        m_CriticalClip = FALSE;
    }

    // check clip warning and criticl range.
    if( AmmoResv == 0 && !WarningsOverRide)
        m_CriticalResv = TRUE;    

    // don't warn reserves for BBG or Meson cannon
    if( WarningsOverRide )
    {
        m_CriticalResv = FALSE;
        m_WarningResv = FALSE;        
    }

    if( AmmoResv < resvPercent && m_WarningResv == FALSE && !WarningsOverRide)
    {
        g_AudioMgr.Play("Ammo_Low",TRUE);
        m_WarningResv = TRUE;
        m_WarningResvALPHA = 0;
    }
    else if( AmmoResv >= resvPercent )
    {
        m_WarningResv= FALSE;     
        m_CriticalResv= FALSE;
    }

    // Critical
    if( AmmoClip == 0 && AmmoResv == 0 )
    {
        if( m_CriticalSoundID == -1 )
            m_CriticalSoundID = g_AudioMgr.Play("Ammo_Out_Loop", TRUE );

        m_CriticalClip = TRUE;
    }
    else
    {
        if (m_CriticalSoundID != -1)
        {
            g_AudioMgr.Release(m_CriticalSoundID,0.0f);
            m_CriticalSoundID = -1;
        }

        m_CriticalClip = FALSE;
    }

    
    //
    // Render the ammo counts.
    //
#ifndef X_EDITOR
    {
        static xstring AmmoReserveString;
        static xstring AmmoMagazineString;
        AmmoReserveString  = (const char *)xfs( "%d", AmmoResv );
        AmmoMagazineString = (const char *)xfs( "%d", AmmoClip );

        // Current loaded ammo.
        m_AmmoRect.l = (s32)m_XPos - m_LeftMag;
        m_AmmoRect.t = (s32)m_YPos - m_TopMag;
        m_AmmoRect.SetHeight( (s32)   50 );
        m_AmmoRect.SetWidth ( (s32)   20 ); 

        xcolor cAmmoColor = g_HudColor;     
        if( m_MultiPlayerHud )
            RenderLine( (const xwchar*)(xwstring((const char*)AmmoMagazineString)), m_AmmoRect, 255, cAmmoColor, 1, ui_font::h_right|ui_font::v_top, FALSE );
        else
            RenderLine( (const xwchar*)(xwstring((const char*)AmmoMagazineString)), m_AmmoRect, 255, cAmmoColor, 2, ui_font::h_right|ui_font::v_top, FALSE );

        // Ammo in reserve.
        m_AmmoRect.l = (s32)m_XPos - m_LeftReserve;
        m_AmmoRect.t = (s32)m_YPos - m_TopReserve;
        m_AmmoRect.SetHeight( (s32)   50 );
        m_AmmoRect.SetWidth ( (s32)   20 );
        RenderLine( (const xwchar*)(xwstring((const char*)AmmoReserveString)), m_AmmoRect, 255, g_HudColor, 1, ui_font::h_right|ui_font::v_top, FALSE );
    }
#endif
}

//==============================================================================

void hud_ammo::RenderFlashlight( player* pPlayer )
{
    (void)pPlayer;
}

//==============================================================================
